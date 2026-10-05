# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。把一整頁的接線對照表送 dryRun，驗證每個鍵都寫得進去。
# 當日完整變更紀錄：<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""wire_probe.py -- 拿接線檔本身當測資，對 wb_serve 驗證整頁可寫。

AI(W906-FW-SYSWIRE) 20260916。

做法：讀 ht9045_wire_<slug>.js 的 sysFields / sysEnums，對每個 (檔,區段,鍵)
先從 /api/system/<檔> 取「目前的值」，再把同一批值以 dryRun 送 system.file.put。

預期結果（兩者同時成立才算過）：
  notFound = 0   對照表裡每個鍵伺服器都認得 —— 這是引擎規則 2 的關卡，
                 只要有一個 notFound，整頁會被拒寫。
  changed  = 0   送的是目前值，所以不該有任何變更。若這裡冒出 changed，
                 代表讀回來的值再送出去會被改寫（型別/格式往返不一致），
                 那會在操作員只按了存檔、什麼都沒改的情況下偷偷改機台設定。

用法: py wire_probe.py <ht9045_wire_xxx.js> [...]
"""
import io, json, os, re, sys, urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

WEBPAGE = r'D:\HT9045\web\page'
# Steven 20260916（審查 D）：埠可用環境變數 WB_PORT 指定，第二個實例／審查者才能重用。
PORT = int(os.environ.get('WB_PORT', '8045'))
BASE = 'http://127.0.0.1:%d/api/system/' % PORT

# 重用 ws_probe 的手工 WebSocket 客戶端。
# ⚠ ws_probe.py 在模組層直接呼叫 main()，一 import 就會自己連線送一筆 put，
#   所以不能用 `from ws_probe import ...`。這裡把那一行拆掉再 exec。
_wsp = {}
with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ws_probe.py'),
          encoding='utf-8') as _fh:
    exec(compile(_fh.read().replace('\nmain()', '\n# main() -- 由 wire_probe 停用'),
                 'ws_probe.py', 'exec'), _wsp)
_wsp['PORT'] = PORT                                        # Steven 20260916（D）
connect, send, recv = _wsp['connect'], _wsp['send'], _wsp['recv']

_cache = {}


def api(f):
    if f not in _cache:
        _cache[f] = json.load(urllib.request.urlopen(BASE + f))
    return _cache[f]


def parse_block(text, name):
    """抓 `<name>: { ... }` 區塊裡的 id: [..] 行。"""
    m = re.search(name + r':\s*\{(.*?)\n  \}', text, re.S)
    if not m:
        return []
    out = []
    for line in m.group(1).splitlines():
        mm = re.match(r"\s*(\w+):\s*\[(.*?)\],?\s*$", line)
        if mm:
            parts = [p.strip().strip("'") for p in mm.group(2).split(',')]
            out.append((mm.group(1), parts))
    return out


def current(f, sec, key):
    """一律回 raw。

    Steven 20260916：cell 長成 {"value":0,"type":"float","raw":"0.000000"}。
    寫入端只認 raw（wb_serve.cpp 說明：只有 raw 是無損的）。拿 value 送回去，
    "0.000000" 就變成 "0" —— 讀進來再原樣送出竟然會 changed，這個探針就是
    這樣抓到引擎在 cell.value / raw 之間用錯欄位的。
    """
    cell = ((api(f).get('sections') or {}).get(sec) or {}).get(key)
    if cell is None:
        return None
    if not isinstance(cell, dict):
        return cell
    return cell.get('raw') if cell.get('raw') is not None else cell.get('value')


def cnt(v):
    if isinstance(v, bool):
        return 0
    if isinstance(v, int):
        return v
    return len(v or [])


def put_csv(s, cid, f, rows_payload):
    send(s, {'type': 'cmd', 'id': cid, 'cmd': 'system.file.put', 'tag': f,
             'value': json.dumps({'rows': rows_payload, 'dryRun': True})})
    return recv(s, cid, timeout=60) or {}


def probe_grid(s, cid, fn, text):
    """Steven 20260916：表格頁（sysGrid）的驗收。

    1. 整張表讀回來、每一格原樣送 dryRun：notFound 必須 0、changed 必須 0。
       這是抓「讀進來原樣送出竟然會變」那一類 bug 的測試，格數以千計時
       identical 應該等於送出格數。
    2. 用「非鍵欄的值」當 rowKey（例如 ioTable 的 IOType 'Sensor'）：必須 notFound。
       20260916 之前 CsvApplyEdits 一律拿第 0 欄比對，這種鍵會命中第一列並回 changed=1。
    3. 空 rowKey：必須 notFound（否則會命中 IO_Table 的空白佔位列）。
    """
    m = re.search(r"sysGrid:\s*\{[^}]*?file:\s*'([^']+)'", text, re.S)
    if not m:
        return cid, 0
    f = m.group(1)
    d = api(f)
    cols, kc, rows = d.get('columns') or [], d.get('keyColumn'), d.get('rows') or []
    print('=== %s  表格 %s：%d 列 × %d 欄，keyColumn=%r' % (fn, f, len(rows), len(cols), kc))
    rc = 0

    # WebBridgeServer.cpp:258 kMaxWsMessage = 64 KB：超過就直接斷線（WinError 10053），
    # ioTable 整張 9,002 格約 300 KB 一定爆。分批送，每批控制在 ~40 KB 內。
    # 操作員實際存檔只送改過的格子，遠低於這個上限；這只是探針要整張回送才會碰到。
    LIMIT = 40 * 1024
    batches, cur, size, n = [], {}, 2, 0
    for r in rows:
        k = r.get(kc, '')
        if not k:
            continue
        cells = {}
        for c in cols:
            if c == kc:
                continue
            cells[c] = str(r.get(c, ''))
            n += 1
        piece = len(json.dumps({k: cells}))
        if cur and size + piece > LIMIT:
            batches.append(cur); cur, size = {}, 2
        cur[k] = cells; size += piece
    if cur:
        batches.append(cur)

    tot_nf = tot_ch = tot_id = 0
    has_all = True; ok_all = True
    for b in batches:
        cid += 1
        a = put_csv(s, cid, f, b)
        if not a:
            ok_all = False; has_all = False
            print('        批次 %d 列：沒有 ack（連線被斷？）' % len(b)); break
        has_all &= ('notFound' in a) and ('changed' in a)
        ok_all &= bool(a.get('ok'))
        tot_nf += cnt(a.get('notFound')); tot_ch += cnt(a.get('changed')); tot_id += cnt(a.get('identical'))
        if a.get('error'):
            print('        error:', a['error'])
    good = ok_all and has_all and tot_nf == 0 and tot_ch == 0 and tot_id == n
    rc |= (0 if good else 1)
    print('    [1] 原樣回送 %-5d 格（%d 批） ok=%-5s notFound=%-4d identical=%-5d changed=%-4d  %s%s'
          % (n, len(batches), ok_all, tot_nf, tot_id, tot_ch, 'PASS' if good else 'FAIL',
             '' if has_all else '  <- ack 沒有計數欄位，驗證無效'))
    payload = {}
    for b in batches:
        payload.update(b)

    # 2. 非鍵欄的值當 rowKey
    other = [c for c in cols if c != kc]
    if other and rows:
        badkey = str(rows[0].get(other[0], ''))
        if badkey and badkey not in payload:
            cid += 1
            a = put_csv(s, cid, f, {badkey: {other[-1]: 'x'}})
            good = bool(a.get('ok')) and cnt(a.get('notFound')) == 1 and cnt(a.get('changed')) == 0
            rc |= (0 if good else 1)
            print('    [2] 用 %s=%r 當 rowKey    notFound=%d changed=%d  %s'
                  % (other[0], badkey, cnt(a.get('notFound')), cnt(a.get('changed')), 'PASS' if good else 'FAIL'))
    # 3. 空 rowKey
    cid += 1
    a = put_csv(s, cid, f, {'': {other[-1] if other else cols[-1]: 'x'}})
    good = bool(a.get('ok')) and cnt(a.get('notFound')) == 1 and cnt(a.get('changed')) == 0
    rc |= (0 if good else 1)
    print('    [3] 空 rowKey                     notFound=%d changed=%d  %s'
          % (cnt(a.get('notFound')), cnt(a.get('changed')), 'PASS' if good else 'FAIL'))

    # Steven 20260916：system.csv.rows（整列新增／刪除）的 dryRun 契約
    def rows_cmd(body):
        nonlocal_cid[0] += 1
        send(s, {'type': 'cmd', 'id': nonlocal_cid[0], 'cmd': 'system.csv.rows', 'tag': f,
                 'value': json.dumps(dict(body, dryRun=True))})
        return recv(s, nonlocal_cid[0], timeout=30) or {}
    nonlocal_cid = [cid]
    existing = rows[0][kc]
    # Steven 20260916（審查 R6）：motTable 的 Motorname 伺服器端限 M##（golden 以 M%02d 對馬達 enum）。
    # 測試鍵要符合格式且不存在（M00..M43 已用，M98 空著）；不符合格式的另測一條必 notFound。
    newkey = 'M98' if kc == 'Motorname' else 'WBPROBE_TMP'
    tests = [
        ('[4] 新增新鍵 %s' % newkey,         {'add': [{kc: newkey}]},               dict(added=1, deleted=0, notFound=0)),
    ] + ([('[4b] 新增不符 M## 的鍵 WBPROBE_TMP', {'add': [{kc: 'WBPROBE_TMP'}]},  dict(added=0, deleted=0, notFound=1))]
         if kc == 'Motorname' else []) + [
        ('[5] 新增已存在的鍵 %r' % existing, {'add': [{kc: existing}]},            dict(added=0, deleted=0, notFound=1)),
        ('[6] 新增鍵值含逗號',              {'add': [{kc: 'A,B'}]},                dict(added=0, deleted=0, notFound=1)),
        ('[7] 刪除不存在的鍵',              {'delete': ['NO_SUCH_KEY_WB']},         dict(added=0, deleted=0, notFound=1)),
        ('[8] 刪除現有鍵 %r（dryRun）' % existing, {'delete': [existing]},         dict(added=0, deleted=1, notFound=0)),
    ]
    for name, body, want in tests:
        a = rows_cmd(body)
        got = dict(added=cnt(a.get('added')), deleted=cnt(a.get('deleted')), notFound=cnt(a.get('notFound')))
        has = all(k in a for k in ('added', 'deleted', 'notFound'))
        good = bool(a.get('ok')) and has and got == want
        rc |= (0 if good else 1)
        print('    %-40s %s  %s%s' % (name, json.dumps(got), 'PASS' if good else 'FAIL',
                                     '' if has else '  <- ack 沒有計數欄位，驗證無效'))
        if a.get('error'):
            print('        error:', a['error'])
    cid = nonlocal_cid[0]
    return cid, rc


def main():
    files = sys.argv[1:] or ['ht9045_wire_hwteach.js', 'ht9045_wire_hwhandlersys.js',
                             'ht9045_wire_hwmotortest.js', 'ht9045_wire_hwiosetview.js']
    s = connect()
    send(s, {'type': 'cmd', 'id': 1, 'cmd': 'control.acquire'})
    recv(s, 1)
    cid = 1
    rc = 0
    for fn in files:
        text = open(os.path.join(WEBPAGE, fn), encoding='utf-8').read()
        if 'sysGrid:' in text:                                   # Steven 20260916
            cid, grc = probe_grid(s, cid, fn, text)
            rc |= grc
        rows = parse_block(text, 'sysFields') + parse_block(text, 'sysEnums')
        if not rows:
            continue
        byfile = {}
        unread = []
        for wid, p in rows:
            f, sec, key = p[0], p[1], p[2]
            v = current(f, sec, key)
            if v is None:
                unread.append((wid, f, sec, key))
                continue
            byfile.setdefault(f, {}).setdefault(sec, {})[key] = {'raw': str(v)}
        print('=== %s  對照 %d 筆，涵蓋 %s' % (fn, len(rows), '、'.join(sorted(byfile))))
        if unread:
            print('    [!] 讀不到值 %d 筆（不送出）：%s'
                  % (len(unread), ', '.join(x[0] for x in unread[:6])))
        for f, sections in sorted(byfile.items()):
            n = sum(len(v) for v in sections.values())
            # Steven 20260916（審查 D）：ini 也要有「錯區段／錯鍵／含換行 必 notFound」的測試，
            # 之前只有 csv 有 [2]/[3]。三筆各自送、各自看。
            for name, body, want in (
                    ('錯區段',   {'__NO_SUCH_SECTION__': {'x': {'raw': '1'}}}, 1),
                    ('錯鍵',     {next(iter(sections)): {'__NO_SUCH_KEY__': {'raw': '1'}}}, 1),
                    ('值含換行', {next(iter(sections)): {next(iter(sections[next(iter(sections))])): {'raw': '1\n[X]'}}}, 1)):
                cid += 1
                send(s, {'type': 'cmd', 'id': cid, 'cmd': 'system.file.put', 'tag': f,
                         'value': json.dumps({'sections': body, 'dryRun': True})})
                a2 = recv(s, cid, timeout=20) or {}
                ok2 = bool(a2.get('ok')) and ('notFound' in a2) and cnt(a2.get('notFound')) == want and cnt(a2.get('changed')) == 0
                rc |= (0 if ok2 else 1)
                print('    %-10s [%s] notFound=%d changed=%d  %s' % (f, name, cnt(a2.get('notFound')), cnt(a2.get('changed')), 'PASS' if ok2 else 'FAIL'))
            cid += 1
            send(s, {'type': 'cmd', 'id': cid, 'cmd': 'system.file.put', 'tag': f,
                     'value': json.dumps({'sections': sections, 'dryRun': True})})
            ack = recv(s, cid, timeout=20) or {}
            # Steven 20260916
            # ack 的 changed/identical/notFound 是「計數」不是陣列。
            # ⚠ 在 20260916 修好 WebBridgeServer.cpp 的 AckJson 之前，這三個欄位
            #   根本不會出現在 ack 裡，`ack.get('notFound') or []` 會一律得到空的
            #   —— 也就是說「notFound=0 所以 PASS」是假的通過。用 cnt() 之外還要
            #   檢查欄位是否存在，否則對著舊版伺服器跑會再騙自己一次。
            # Steven 20260916：cnt() 用模組層的那個（原本這裡又定義一次，讓上面的 ini 測試撞到 UnboundLocalError）
            has = ('notFound' in ack) and ('changed' in ack)
            nf, ch, ident = cnt(ack.get('notFound')), cnt(ack.get('changed')), cnt(ack.get('identical'))
            ok = ack.get('ok')
            good = bool(ok) and has and nf == 0 and ch == 0
            if not good:
                rc = 1
            why = ''
            if not has:
                why = '  <- ack 沒有 notFound/changed 欄位，這次驗證無效'
            print('    %-10s 送出 %-4d  ok=%-5s notFound=%-4d identical=%-4d changed=%-4d  %s%s'
                  % (f, n, ok, nf, ident, ch, 'PASS' if good else 'FAIL', why))
            if ack.get('error'):
                print('        error:', ack['error'])
    sys.exit(rc)


main()
