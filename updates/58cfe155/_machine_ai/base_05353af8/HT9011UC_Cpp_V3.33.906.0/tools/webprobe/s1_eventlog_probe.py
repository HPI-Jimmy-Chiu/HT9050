# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/s1_eventlog_probe.py -- JSON 橋接層 S0～S3 的 e2e probe。
#
#  名字留著 s1_ 是因為它一開始只盯 S1；現在範圍是
#    S0 machine.defines / cfg.resync
#    S1 log.event / log.tail
#    S2 deviceForm.file / .live 與 schema
#    S3 levelSet（二進位，用陣列處理）
#
#  AI(W906-JSONBRIDGE-S1) 20260923: 新檔。走完整條
#  瀏覽器 -> ws -> CommandQueue -> tick-drain -> HandleLogEvent -> golden 入口
#  -> ring -> GET /api/struct/log.tail 的來回，以及 S0 的 cfg.resync。
#
#  用 cmd_probe.py 的握手／訊框工具（同目錄，AI(W906-FW-W1) 20260819），
#  不另寫一份 —— 兩份 RFC6455 遮罩實作就是兩個會漂的地方。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --seconds 25 --port 8192
#      python tools\webprobe\s1_eventlog_probe.py --port 8192
#
#  Exit 0 = 每一項期望都達成；非 0 = 第一個失敗的項目（看輸出）。
# =============================================================================
import argparse
import json
import os
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

# ⚠ 不要用 cmd_probe.wait_ack()：它每次呼叫都拿同一份 `leftover` 重建一個
#   read_frames 產生器。單次探測沒問題，但連續送多個指令時，第一次呼叫已經把
#   socket 上的資料吃進它自己的緩衝，第二次重建的產生器看到的是**過期的**
#   leftover，於是從第二個 ack 起全部收不到。
#   （20260923 實測：第一筆 PASS、後面四筆全 FAIL，看起來像伺服器只處理了
#   一個指令 —— 但伺服器是對的。）
#   這裡改成整場只建一個產生器，所有 ack 都從它拉。


class AckReader(object):
    def __init__(self, sock, leftover, deadline):
        self._gen = read_frames(sock, leftover, deadline)
        self._pending = []          # 先收到、但不是現在要等的 ack

    def wait(self, want_id):
        for j in self._pending:
            if j.get('id') == want_id:
                self._pending.remove(j)
                return j
        for op, payload in self._gen:
            if op != 1:
                continue
            try:
                j = json.loads(payload.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if j.get('type') != 'ack':
                continue            # snapshot / patch 訊框，不是我們要的
            if j.get('id') == want_id:
                return j
            self._pending.append(j)
        return None

FAIL = []


def check(cond, what):
    print(('  PASS  ' if cond else '  FAIL  ') + what)
    if not cond:
        FAIL.append(what)


def http_json(port, path):
    url = 'http://127.0.0.1:%d%s' % (port, path)
    with urllib.request.urlopen(url, timeout=5) as r:
        return json.loads(r.read().decode('utf-8'))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8192)
    a = ap.parse_args()
    port = a.port

    # -- S0：開機配置一定要在，而且 SOFT_SIMULTE 要說得出口 ------------------
    print('[S0] GET /api/struct/machine.defines')
    md = http_json(port, '/api/struct/machine.defines')
    check(md.get('count', 0) > 0, 'web 組不是空的（count=%s）' % md.get('count'))
    check(md.get('total', 0) >= md.get('count', 0), 'total >= count')
    check(md.get('cfgVer', 0) >= 1, 'cfgVer 從 1 起跳（=%s）' % md.get('cfgVer'))
    d = md.get('defines', {})
    check('SOFT_SIMULTE' in d, 'SOFT_SIMULTE 在表裡')
    if 'SOFT_SIMULTE' in d:
        # 這顆 exe 是哪一種組態，probe 不預設；它只要求「說得出來」。
        print('        SOFT_SIMULTE = %s' % json.dumps(d['SOFT_SIMULTE']))
        check(isinstance(d['SOFT_SIMULTE'].get('on'), bool), 'on 是布林，不是字串')
    check(not any(k.startswith('CC_') for k in d), 'CC_* 客戶碼常數沒有上線')

    ad = http_json(port, '/api/struct/machine.defines?all=1')
    check(any(k.startswith('CC_') for k in ad.get('defines', {})),
          '?all=1 看得到 CC_*')

    # -- 留痕：先記下起點 ----------------------------------------------------
    print('[S1] GET /api/struct/log.tail（起點）')
    t0 = http_json(port, '/api/struct/log.tail')
    base_seq = t0.get('seq', 0)
    check('entries' in t0, 'log.tail 有 entries')
    check(t0.get('capacity', 0) > 0, 'capacity > 0（=%s）' % t0.get('capacity'))
    check(t0.get('db') is False, 'db 誠實回報未開啟')

    # -- WS：送三筆 log.event 與一次 cfg.resync ------------------------------
    print('[S1] WS log.event x3 + cfg.resync')
    deadline = time.monotonic() + 20.0
    sock, rest = ws_handshake('127.0.0.1', port, '/ht9045', deadline)
    reader = AckReader(sock, rest, deadline)

    def cmd(cid, name, value, tag=None):
        """送一個指令，回一個**已正規化**的 ack。

        ⚠ 伺服器的回應形狀在成功與失敗時**不一樣**（AckJson，
          WebBridgeServer.cpp:1262）：
            ok=true  -> handler 回的 JSON 物件被去掉大括號**原地拼接**進 ack
            ok=false -> 整包被 QuoteString **字串化**塞進 ack["error"]
          所以「handler 說了什麼」在兩種情況下要用不同方式取。這裡統一攤平，
          後面的斷言才不用each自己判斷。
        """
        msg = {'type': 'cmd', 'id': cid, 'cmd': name, 'value': value}
        if tag is not None:
            msg['tag'] = tag
        send_text(sock, json.dumps(msg))
        a = reader.wait(cid)
        if a is None:
            return None
        err = a.get('error')
        if isinstance(err, str) and err.startswith('{'):
            try:
                inner = json.loads(err)
                merged = dict(inner)
                merged['ok'] = a.get('ok')
                merged['id'] = a.get('id')
                return merged
            except ValueError:
                pass
        return a

    a1 = cmd(9001, 'log.event', json.dumps({
        'kind': 'process', 'msg': 'Setup.Contact opened pressed',
        'page': 'Setup.Contact.html', 'ctrl': 'tabContact'}))
    check(a1 is not None and a1.get("ok") is True, 'kind=process 被接受')
    if a1:
        print('        ack: %s' % json.dumps(a1.get('error') or a1)[:200])

    a2 = cmd(9002, 'log.event', json.dumps({
        'kind': 'change', 'msg': 'X Pitch change Value', 'debug': '26.66==>26.70'}))
    check(a2 is not None and a2.get('ok') is True, 'kind=change 被接受')

    # 壞的 payload 一定要被拒，不能因為「收下了」就回成功。
    a3 = cmd(9003, 'log.event', json.dumps({'kind': 'alarm', 'msg': 'no code'}))
    check(a3 is not None and a3.get('ok') is False, 'kind=alarm 缺 alarmCode -> 拒絕')

    a4 = cmd(9004, 'log.event', json.dumps({'kind': 'process', 'msg': ''}))
    check(a4 is not None and a4.get('ok') is False, '空 msg -> 拒絕')

    a5 = cmd(9005, 'log.event', 'not json at all')
    check(a5 is not None and a5.get('ok') is False, '非 JSON payload -> 拒絕')

    # S0 的 cfg.resync：same -> unchanged、0 -> 完整內容
    ver = md.get('cfgVer', 1)
    # ⚠ CompleteCommand 的第三個參數不是「錯誤字串」——當它是一段合法 JSON 物件時，
    #   伺服器把它**併進 ack 物件本身**（實測 log.event 的 ack：
    #   {"type":"ack","id":9001,"ok":true,"seq":1,"sinks":[...],"db":false}）。
    #   所以回傳欄位要直接在 ack 上取，不是 ack["error"]。
    a6 = cmd(9006, 'cfg.resync', ver)
    check(a6 is not None and a6.get('ok') is True, 'cfg.resync(same) ok')
    check(a6 is not None and a6.get('unchanged') is True,
          'cfg.resync(same) 回 unchanged=true')
    check(a6 is not None and a6.get('ver') == ver,
          'cfg.resync 回的 ver 與 /api 一致（=%s）' % (a6 or {}).get('ver'))
    check(a6 is not None and 'defines' not in a6,
          'cfg.resync(same) 不夾帶內容（沒變就不要送）')

    a7 = cmd(9007, 'cfg.resync', 0)
    check(a7 is not None and a7.get('unchanged') is False,
          'cfg.resync(0) 回 unchanged=false')
    check(a7 is not None and isinstance(a7.get('defines'), dict)
          and 'SOFT_SIMULTE' in a7['defines'].get('defines', a7['defines']),
          'cfg.resync(0) 夾帶完整內容且含 SOFT_SIMULTE')

    # -- S6：JSON -> 結構（今天只到 dryRun）---------------------------------
    print('[S6] WS struct.put（dryRun）')
    # ⚠ struct.put **不在**權杖豁免名單裡，這是刻意的：它會改機台設定，
    #   跟 cfg.resync（純讀）／log.event（回報）是不同性質的東西。
    #   所以要先 control.acquire —— 先驗證擋得住，再拿權杖。
    a9 = cmd(9009, 'struct.put', json.dumps({'values': {}, 'dryRun': True}),
             tag='testIF.file')
    check(a9 is not None and a9.get('ok') is False and
          'not-operator' in json.dumps(a9),
          '沒有權杖時 struct.put 被擋（設定寫入不該豁免）')

    acq = cmd(9008, 'control.acquire', 1)
    check(acq is not None and acq.get('ok') is True, 'control.acquire 拿到權杖')

    # Steven 20260924: 原本寫死 9。ca4e903 之後 TestIF_File 是配方真值
    #   （QPM5577_8 = 13），寫死會變成「剛好等於現值」才會過。改成先讀現值。
    cur_tm = http_json(port, '/api/struct/testIF.file').get('values', {}).get('iTestMode')
    a10 = cmd(9010, 'struct.put', json.dumps({
        'values': {'iTestMode': cur_tm}, 'dryRun': True}), tag='testIF.file')
    check(a10 is not None and a10.get('applied') is True,
          '純量欄位通過驗證')
    check(a10 is not None and a10.get('changed') == 0,
          '送進去的值跟現值相同 -> changed=0（=%s）' % (a10 or {}).get('changed'))

    a11 = cmd(9011, 'struct.put', json.dumps({
        'values': {'iTestMode': 12345}, 'dryRun': True}), tag='testIF.file')
    check(a11 is not None and a11.get('changed') == 1,
          '改一個值 -> changed=1（=%s）' % (a11 or {}).get('changed'))

    a12 = cmd(9012, 'struct.put', json.dumps({
        'values': {'iTestMode': 'nine'}, 'dryRun': True}), tag='testIF.file')
    check(a12 is not None and a12.get('applied') is False,
          '型別不符 -> 拒絕（字串不會被當成數字）')

    a13 = cmd(9013, 'struct.put', json.dumps({
        'values': {'noSuchField': 1}, 'dryRun': True}), tag='testIF.file')
    check(a13 is not None and a13.get('applied') is False,
          '不存在的欄位 -> 拒絕')

    # 陣列要稀疏索引物件（使用者裁決：GET 稠密、PUT 稀疏）
    a14 = cmd(9014, 'struct.put', json.dumps({
        'values': {'IndexArmPick': {'0': 1.5}}, 'dryRun': True}),
        tag='deviceForm.file')
    check(a14 is not None and a14.get('applied') is True,
          '一維陣列收稀疏索引物件 {"0": 1.5}')
    a15 = cmd(9015, 'struct.put', json.dumps({
        'values': {'IndexArmPick': [1.5, 2.5]}, 'dryRun': True}),
        tag='deviceForm.file')
    check(a15 is not None and a15.get('applied') is False,
          '陣列給稠密陣列 -> 拒絕（PUT 收稀疏）')
    a16 = cmd(9016, 'struct.put', json.dumps({
        'values': {'IndexArmPick': {'99': 1.5}}, 'dryRun': True}),
        tag='deviceForm.file')
    check(a16 is not None and a16.get('applied') is False,
          '索引越界 -> 拒絕')

    # ⚠ 最重要的一條：沒有 Clamp* 之前，dryRun=false 一定要被拒。
    a17 = cmd(9017, 'struct.put', json.dumps({
        'values': {'iTestMode': 9}, 'dryRun': False}), tag='testIF.file')
    check(a17 is not None and a17.get('applied') is False,
          'dryRun=false 誠實拒絕（缺 Clamp* 與 ini 鍵對照表）')
    # ⚠ 欄位叫 reason 不叫 error —— error 是 ack 外層的鍵，撞名會被覆蓋。
    check(a17 is not None and 'Clamp' in (a17.get('reason') or ''),
          '拒絕理由講得出缺什麼，不是含糊的「尚未支援」')

    a18 = cmd(9018, 'struct.put', json.dumps({
        'values': {'iTestMode': 9}, 'dryRun': True}), tag='no.such.binding')
    check(a18 is not None and a18.get('ok') is False, '未知綁定 -> 拒絕')

    sock.close()

    # -- 留痕：終點，只有被接受的那兩筆該進去 --------------------------------
    print('[S1] GET /api/struct/log.tail（終點）')
    t1 = http_json(port, '/api/struct/log.tail?since=%d' % base_seq)
    got = t1.get('entries', [])
    check(t1.get('seq', 0) == base_seq + 2,
          '只增加 2 筆（被拒的 3 筆不進 ring），seq %s -> %s'
          % (base_seq, t1.get('seq')))
    check(len(got) == 2, 'since 過濾正確，回 %d 筆' % len(got))
    if len(got) == 2:
        e1, e2 = got[0], got[1]
        check(e1.get('kind') == 'process' and e1.get('table') == 'Process',
              '第一筆 kind/table 正確')
        check('[Setup.Contact.html/tabContact]' in (e1.get('debug') or ''),
              'page/ctrl 接在 debug 尾端')
        check('golden' in (e1.get('sinks') or []), 'process 有送進 golden 入口')
        check('stdout' in (e1.get('sinks') or []),
              'process 的替身會印 stdout（canary_support.cpp:116）')
        check(e2.get('kind') == 'change' and e2.get('table') == 'ChangeLog',
              '第二筆落到 ChangeLog 表')
        check(e1.get('origin') == 'html', 'origin 標成 html')
        check('db' not in (e1.get('sinks') or []),
              'db 沒被謊報（wb_serve 不呼叫 MyDBOpenDB）')

    # -- S2/S3：結構綁定 ----------------------------------------------------
    print('[S2/S3] GET /api/struct（綁定清單）')
    idx = http_json(port, '/api/struct')
    names = [b['name'] for b in idx.get('bindings', [])]
    for want in ('machine.defines', 'log.tail', 'deviceForm.file',
                 'deviceForm.live', 'levelSet'):
        check(want in names, '綁定 %s 在清單裡' % want)

    print('[S2] GET /api/struct/deviceForm.file')
    df = http_json(port, '/api/struct/deviceForm.file')
    check(df.get('type') == 'SYSTEM_DEVICE_FORM', 'type 正確')
    check(df.get('count') == 66, '66 個欄位（=%s）' % df.get('count'))
    check(df.get('writable') is False, 'S2 只做讀方向，writable=false')
    # ⚠ 這一條是這次最重要的斷言：旗標必須與「填它的 golden 函式在不在」一致，
    #   因為 0 在這棵樹上是合法的座標／模式／壓力，旗標錯了就等於騙人。
    # Steven 20260924: False → True。JerryYang ca4e903 已移植 TfContact::ReadFile
    #   （forms/fContact.cpp:1475）並接上 wb_serve 開機序列。
    check(df.get('sourcePorted') is True,
          'sourcePorted 回報 TfContact::ReadFile 已移植')
    check('TfContact::ReadFile' in (df.get('filledBy') or ''),
          'filledBy 指出是誰該填它')
    v = df.get('values', {})
    check(isinstance(v.get('IndexArmPick'), list) and len(v['IndexArmPick']) == 2,
          '一維陣列回稠密 [2]')
    check('CalCCDIP' in v and isinstance(v['CalCCDIP'], str),
          'AnsiString 欄位回字串')
    check(set(df.get('unsupported', [])) == {'iOffsetX', 'iOffsetY', 'iOffsetR'},
          '三維陣列誠實列進 unsupported 而不是消失')
    for u in df.get('unsupported', []):
        check(v.get(u, 'MISSING') is None, '%s 在 values 裡是 null 不是省略' % u)

    print('[S2] GET /api/struct/deviceForm.file/schema')
    sc = http_json(port, '/api/struct/deviceForm.file/schema')
    check(sc.get('structSize', 0) > 0, 'structSize 由 sizeof 算（=%s）'
          % sc.get('structSize'))
    f = sc.get('fields', {})
    check(len(f) == 66, 'schema 66 個欄位')
    check(f.get('IndexArmPick', {}).get('off') == 0,
          '第一個欄位 offset=0（offsetof 由編譯器算）')
    offs = sorted(x.get('off', -1) for x in f.values())
    check(len(set(offs)) == len(offs), '沒有兩個欄位共用同一個 offset')
    check(max(offs) < sc.get('structSize', 0), '最大 offset 落在 structSize 內')

    # S6 的 ini 鍵對照（寫方向要用）
    check(sc.get('iniMapped', 0) > 0,
          'schema 帶 ini 鍵對照（%s 欄）' % sc.get('iniMapped'))
    check(sc.get('iniMultiSource', -1) >= 0,
          '多來源欄位數有回報（=%s）' % sc.get('iniMultiSource'))
    ti_sc = http_json(port, '/api/struct/testIF.file/schema')
    xp = ti_sc.get('fields', {}).get('dSiteXPitch', {}).get('ini', {})
    check(xp.get('section') == 'Configuration' and xp.get('key') == 'X Pitch',
          'dSiteXPitch 對到 [Configuration] X Pitch（JerryYang 翻的那一組）')
    check(ti_sc.get('iniMapped', 0) > 600,
          'testIF.file 對到 %s 欄 ini 鍵' % ti_sc.get('iniMapped'))

    print('[S3] GET /api/struct/levelSet')
    ls = http_json(port, '/api/struct/levelSet')
    acc = ls.get('values', {}).get('AccessLevel')
    check(isinstance(acc, list), 'AccessLevel 是陣列（使用者裁決：二進位用陣列處理）')
    check(isinstance(acc, list) and len(acc) == 256, '長度 256（=%s）'
          % (len(acc) if isinstance(acc, list) else '?'))
    check(ls.get('sourcePorted') is True, 'GetLevelSet 有移植')

    # -- S4/S5：大結構，而且要真的有資料 ------------------------------------
    print('[S4] GET /api/struct/testIF.file')
    ti = http_json(port, '/api/struct/testIF.file')
    # ⚠ 788 不是 351。351 是反向大括號配對咬到 cprod.h:2156 註解裡的 `{`
    #   之後量到的尾巴。這條斷言就是為了讓那個誤判不會再回來。
    check(ti.get('count') == 788, 'SYSTEM_TEST_IF 是 788 欄（=%s）' % ti.get('count'))
    check(ti.get('sourcePorted') is True,
          'TfSetup::ReadFile 已移植（JerryYang 20260922）')
    tv = ti.get('values', {})
    # ⚠ 20260923（第四輪審查）：這裡原本斷言
    #       tv['dSiteXPitch'] != 0 and tv['dSiteYPitch'] != 0
    #   理由是「開機時 wb_serve 自己印 X Pitch=30.000, Y Pitch=60.000，
    #   這裡要對得上」。那條**相依於磁碟上那一份配方** —— 換一台機器、
    #   換一個 recipe，pitch 合法地是 0（單 site，或還沒教導）時這支 probe
    #   會報假失敗。假失敗會訓練人忽略整支 probe，那比少一條斷言更糟。
    #
    #   改成兩條與配方值無關的：
    #     (a) 欄位在、而且型別是數。這守的是 FieldDesc 把 double 欄位讀成
    #         數值這件事（回字串、回 null、或整個欄位消失，都表示讀表那一段
    #         壞了），不是某一份配方的內容。
    #     (b) 「資料層真的載進來了」交給下面那條彙總斷言（非零欄位數 > 20），
    #         它不綁任何單一欄位的值，換配方不會翻。
    #   pitch 的實際值照印出來給人看，但不作為成敗依據。
    for _pitch in ('dSiteXPitch', 'dSiteYPitch'):
        _v = tv.get(_pitch, '(MISSING)')
        check(isinstance(_v, (int, float)) and not isinstance(_v, bool),
              '%s 回數值型別（=%r）' % (_pitch, _v))
    print('    參考值（不作判定）: X Pitch=%r  Y Pitch=%r'
          % (tv.get('dSiteXPitch'), tv.get('dSiteYPitch')))
    check(isinstance(tv.get('iSiteMap'), list) and
          any(any(c for c in row) for row in tv['iSiteMap'] if isinstance(row, list)),
          'iSiteMap 是二維陣列且不是全 0')
    nonzero = sum(1 for x in tv.values() if x not in (0, False, '', None, []))
    check(nonzero > 20, '非零欄位數 %d > 20（資料層真的載進來了）' % nonzero)

    print('[S5] 其餘結構綁定')
    for name, want, ported in (('temperature', 223, True),
                               ('trayForm', 44, True),
                               ('testMode', 5, True),
                               ('testIF.live', 788, True)):     # Steven 20260924: False→True，DoStructUnitConvert 已上 wb_serve 開機路徑
        d2 = http_json(port, '/api/struct/' + name)
        check(d2.get('count') == want,
              '%s 有 %d 欄（=%s）' % (name, want, d2.get('count')))
        check(d2.get('sourcePorted') is ported,
              '%s sourcePorted=%s' % (name, ported))

    print('[S2] 404 / 405 / HEAD')
    try:
        http_json(port, '/api/struct/no.such.binding')
        check(False, '未知綁定要回 404')
    except Exception as ex:
        check('404' in str(ex), '未知綁定回 404（%s）' % str(ex)[:40])

    print('')
    if FAIL:
        print('s1_eventlog_probe: %d FAILURE(S)' % len(FAIL))
        for f in FAIL:
            print('   - ' + f)
        return 1
    print('s1_eventlog_probe: OK')
    return 0


if __name__ == '__main__':
    sys.exit(main())
