# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/q41_open_gate_probe.py -- C 路開頁（WS editlist.get）／存檔（WS editlist.save）的 golden 開窗閘重查。
#
#  AI(W906-FRW-S158) 20260927 [W906]  NOT in golden。Q41 盤點 C-1／C-2
#  （D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md）；C++ 本體 FileRW/_EditPage.cpp
#  OpenGateRefused／kOpenGates，tools/wb_serve.cpp editlist.get（:5207）與 editlist.save（:5303）同一行插入。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve）。
#
#  驗什麼（不用瀏覽器，直接走 WS）：
#    L  機台停著（auth.mode systemStart=false）：
#       L0 切到 Operator（下拉選單模式 auth.select 0；密碼本模式 auth.logout —— golden 登出＝AccessLevel 0）
#       L1 每個等級（Operator 一定跑；其他等級要 --login 給帳密）逐頁 editlist.get，照 levelset.dat 算的預期比：
#            擋 → error 以 "not-authorized:" 開頭、帶「目前登入」；工具選單擋的帶「工具選單需要等級 0」、
#                 設定選單擋的帶「設定選單需要等級 1」；Handler System 帶「Handler System 要」
#            過 → ok（--no-open 時不送，只驗擋的；過的頁會跑 golden FormShow，讀檔）
#       L2 預期擋的頁送 editlist.save（value 是空的 widgets）→ 同一個碼、帶「存檔前重查」。
#            ⚠ 預期會過的頁**絕不送** save（會真的跑 golden 存檔流程寫檔）。
#       L3 BarCode 的第二條路：主路（工具選單＋88）不通、Teach 那條（Motion View 86＋87）通 → 照 golden 放行
#          （golden V912 uteach.cpp:4603-4607 Teach 頁的 sbBarCode 不查 88）。
#       最後照 --login 回到開始的等級；沒給帳密就停在 Operator（會印警告）。
#    R  機台運轉中（auth.mode systemStart=true；--expect-running 時不是運轉中算失敗）：等級這時不能換（golden btLoginClick
#       運轉中不理），用當下的等級驗：
#       R1 工具選單頁（Ld_UldDelayTime）、設定選單頁（TTLCfg）、Teach、ShuttleMove、HSys → "running:"（golden 選單鈕
#          :29033／:29012 與各鈕自己的 if(SystemStart) return;）
#       R2 Offset_File（golden sbOffsetClick 不查 SystemStart）、AOAOffset（AOA 頁籤不查）→ 開頁不被 "running:" 擋
#          （--no-open 時跳過）；Speed 只在 SystemStart && iHome 時擋 → 兩種都收，印出來
#       R3 editlist.save 任一頁 → 先被 R0927-7 擋（「機台運轉中（SystemStart）不能從網頁存設定」，在本閘之前）
#
#  levelset.dat：預設讀 GET /api/system/levelset；wb_serve 用 W906_LEVELSET_PATH 指了測試檔時，加 --levelset <同一個檔>。
#  建議的測試檔（四個等級各有頁被擋，也驗得到 BarCode 的第二條路）：
#      python tools\webprobe\q41_open_gate_probe.py make-levelset %TEMP%\q41_levelset.dat
#      （[0]=1 [1]=1 [3]=1 [24]=2 [25]=2 [31]=2 [39]=2 [86]=1 [87]=2 [88]=3，其他 0；四階制 Operator0／Engineer1／
#        Supervisor2／HonPrec3）
#
#  用法：
#      set W906_LEVELSET_PATH=%TEMP%\q41_levelset.dat
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046   # ⛔ 20260927 更正：--dry 已取消（帶了 wb_serve 直接結束，tools/wb_serve.cpp:3683-3684）；在 Steven01 怎麼跑見 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\wbserve-sandbox-run.md
#      python tools\webprobe\q41_open_gate_probe.py --port 8046 --levelset %TEMP%\q41_levelset.dat ^
#             [--login 1=ENGUSER:ENGPW --login 2=SUPUSER:SUPPW --login 3=HPUSER:HPPW] [--no-open] [--expect-running]
#        --login L=帳號:密碼：密碼本模式用 auth.login；下拉選單模式帳號不用，只用密碼（auth.select L）
#  ⚠ 允許開的頁會跑 golden FormShow（讀檔；IniConfig 會把 lastdata.dat 讀回 LastSet）。要完全不碰檔案就加 --no-open。
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import struct
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
GENERAL_INI = r'D:\HT9045\system\Gerneral.ini'
CC_SCS, CC_SPIL_CHINA_SUZHOU, CC_ASE_KaohSiung, CC_ASE_KaohSiung_K12 = 944, 912, 936, 929   # MachineType.h
TEST_LEVELS = {0: 1, 1: 1, 3: 1, 24: 2, 25: 2, 31: 2, 39: 2, 86: 1, 87: 2, 88: 3}
EMPTY_SAVE = json.dumps({'widgets': {}, 'answers': {}})


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def info(what):
    print('  INFO  ' + what)


class Ws(object):
    def __init__(self, port):
        self.sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
        self.frames = read_frames(self.sock, left, time.monotonic() + 900)
        self.n = 100
        self.sent = {}

    def cmd(self, name, tag=None, value=None):
        key = (name, tag, value)                   # WebCmdGuard：同 cmd＋tag＋value 400 ms 內重複回 busy:
        dt = time.monotonic() - self.sent.get(key, -9)
        if dt < 0.5:
            time.sleep(0.5 - dt)
        self.sent[key] = time.monotonic()
        self.n += 1
        m = {'type': 'cmd', 'id': self.n, 'cmd': name}
        if tag is not None:
            m['tag'] = tag
        if value is not None:
            m['value'] = value
        send_text(self.sock, json.dumps(m))
        for op, p in self.frames:
            if op != 1:
                continue
            try:
                d = json.loads(p.decode('utf-8', 'replace'))
            except ValueError:
                continue
            if d.get('type') == 'ack' and d.get('id') == self.n:
                return d
        return None


def err(ack):
    return (ack or {}).get('error') or ''


def short(ack):
    return json.dumps(ack, ensure_ascii=False)[:300]


def read_levels(a):
    if a.levelset:
        b = open(a.levelset, 'rb').read()
        return list(struct.unpack('<%di' % (len(b) // 4), b[:len(b) // 4 * 4]))
    j = json.loads(urllib.request.urlopen('http://127.0.0.1:%d/api/system/levelset' % a.port, timeout=10)
                   .read().decode('utf-8'))
    return list(j.get('values') or [])


def customer_code(a):
    if a.customer_code is not None:
        return a.customer_code
    try:
        for line in open(GENERAL_INI, encoding='cp950', errors='replace'):
            if line.strip().upper().startswith('CUSTOMER_CODE='):
                return int(line.split('=', 1)[1].strip())
    except (OSError, ValueError):
        pass
    return -1


def expectations(lv, LS, cc, honprec):
    """照 FileRW/_EditPage.cpp kOpenGates 算（只挑條件全是等級、算得出來的頁）。回 {tag: (allowed, 必含字串 or None)}；
    allowed=None ＝ 算不出來（要看 bResetMNet／bAnyLevelCanGetStateRecode），只印不判。"""
    ok = lambda i: LS[i] <= lv                     # golden Insufficient：AccessLevel<LevelSet.AccessLevel[i] ⇒ 不夠
    tools, config = ok(0), ok(1)
    e = {}
    e['Ld_UldDelayTime'] = (tools, None if tools else '工具選單需要等級 0')
    e['TestIF_File_YieldMonitoring'] = (tools and ok(39), None if tools and ok(39) else
                                        ('工具選單需要等級 0' if not tools else 'Yield Monitoring需要等級 39'))
    e['TTLCfg'] = (config and ok(31), None if config and ok(31) else ('設定選單需要等級 1' if not config else 'DIO Setting需要等級 31'))
    e['StartCondition'] = (config and ok(24), None if config and ok(24) else ('設定選單需要等級 1' if not config else 'Start Mode需要等級 24'))
    e['IniConfig_CounterSel'] = (config and ok(25), None if config and ok(25) else ('設定選單需要等級 1' if not config else 'Counter Select需要等級 25'))
    if cc == CC_SCS:
        e['IniConfig'] = (config and ok(30), None if config and ok(30) else ('設定選單需要等級 1' if not config else 'Configuration（CC_SCS）需要等級 30'))
    else:
        e['IniConfig'] = (config, None if config else '設定選單需要等級 1')
    if cc == CC_SPIL_CHINA_SUZHOU:
        e['ArmSpeed_File'] = (lv != 0, None if lv != 0 else 'CC_SPIL_CHINA_SUZHOU')
    else:
        e['ArmSpeed_File'] = (ok(3), None if ok(3) else 'Speed需要等級 3')
    if cc in (CC_ASE_KaohSiung, CC_ASE_KaohSiung_K12):
        e['HSys'] = (False, 'ASE 高雄')
    else:
        e['HSys'] = (lv >= honprec, None if lv >= honprec else 'Handler System 要')
    e['Offset_File'] = (True, None)
    main = tools and ok(88)
    if main:
        e['TestIF_File_BarCode'] = (True, None)
    elif not ok(87):                               # Teach 那條一定不通（87 不夠）
        e['TestIF_File_BarCode'] = (False, '工具選單需要等級 0' if not tools else 'Bar Code需要等級 88')
    elif ok(86):                                   # Teach 那條：Motion View 86 夠、87 夠（假設 bResetMNet=false）
        e['TestIF_File_BarCode'] = (True, 'alt')
    else:
        e['TestIF_File_BarCode'] = (None, None)    # 要看 bAnyLevelCanGetStateRecode
    return e


def set_level(ws, target, creds):
    st = ws.cmd('auth.mode') or {}
    if st.get('level') == target:
        return True
    if target == 0:
        r = ws.cmd('auth.select', '0') if st.get('mode') == 'select' else ws.cmd('auth.logout')
    else:
        if target not in creds:
            return None
        user, pw = creds[target]
        if st.get('mode') == 'select':
            r = ws.cmd('auth.select', str(target), pw)
        else:
            if st.get('btLogin') == 'Logout':
                ws.cmd('auth.logout')
            r = ws.cmd('auth.login', user, pw)
    st = ws.cmd('auth.mode') or {}
    if st.get('level') != target:
        info('切等級 %d 失敗：%s／%s' % (target, short(r), short(st)))
    return st.get('level') == target


def run_level(ws, a, lv, LS, cc):
    print('== L  等級 %d（levelset：[0]=%d [1]=%d [3]=%d [24]=%d [25]=%d [31]=%d [39]=%d [86]=%d [87]=%d [88]=%d）' % (
        lv, LS[0], LS[1], LS[3], LS[24], LS[25], LS[31], LS[39], LS[86], LS[87], LS[88]))
    for tag, (allowed, must) in expectations(lv, LS, cc, a.honprec_level).items():
        if allowed is None:
            if not a.no_open:
                r = ws.cmd('editlist.get', tag)
                info('%s 算不出來（看 bAnyLevelCanGetStateRecode）：%s' % (tag, short(r)))
            continue
        if allowed:
            if a.no_open:
                info('%s 預期可開（--no-open，不送）' % tag)
                continue
            r = ws.cmd('editlist.get', tag)
            e = err(r)
            gate = e.split(':', 1)[0] in ('not-authorized', 'running', 'hidden', 'not-ready', 'no-gate')
            check(bool(r) and not gate, 'L1 %s 等級 %d 可開%s → 不被開窗閘擋：%s' % (
                tag, lv, '（BarCode 第二條路 Teach）' if must == 'alt' else '', short(r)))
            continue
        r = ws.cmd('editlist.get', tag)
        e = err(r)
        check(bool(r) and not r.get('ok') and e.startswith('not-authorized:' if tag != 'HSys' or cc not in (
            CC_ASE_KaohSiung, CC_ASE_KaohSiung_K12) else 'hidden:') and (must or '') in e and
              ('目前登入' in e or 'ASE' in e) and '開頁重查' in e,
              'L1 %s 等級 %d 擋、帶「%s」：%s' % (tag, lv, must, e[:260]))
        s = ws.cmd('editlist.save', tag, EMPTY_SAVE)
        es = err(s)
        check(bool(s) and not s.get('ok') and es.split(':', 1)[0] == e.split(':', 1)[0] and '存檔前重查' in es,
              'L2 %s 等級 %d 存檔也擋（存檔前重查）：%s' % (tag, lv, es[:200]))


def run_running(ws, a):
    st = ws.cmd('auth.mode') or {}
    print('== R  運轉中（等級 %s）' % st.get('level'))
    for tag in ('Ld_UldDelayTime', 'TTLCfg', 'Teach', 'ShuttleMove', 'HSys'):
        r = ws.cmd('editlist.get', tag)
        e = err(r)
        ok = bool(r) and not r.get('ok') and (e.startswith('running:') or
                                                (tag in ('Teach', 'ShuttleMove') and e.startswith('not-authorized:')) or
                                                (tag == 'HSys' and e.startswith('hidden:')))
        check(ok, 'R1 %s 運轉中開頁被擋：%s' % (tag, e[:220]))
    if not a.no_open:
        for tag in ('Offset_File', 'AOAOffset'):
            r = ws.cmd('editlist.get', tag)
            check(bool(r) and not err(r).startswith('running:'), 'R2 %s 運轉中開頁不被 running 擋（golden 不查）：%s' % (tag, short(r)))
        r = ws.cmd('editlist.get', 'ArmSpeed_File')
        info('R2 ArmSpeed_File（golden 只在 SystemStart && iHome 時擋）：%s' % short(r))
    s = ws.cmd('editlist.save', 'Ld_UldDelayTime', EMPTY_SAVE)
    check(bool(s) and not s.get('ok') and '機台運轉中（SystemStart）不能從網頁存設定' in err(s),
          'R3 運轉中存檔先被 R0927-7 擋：%s' % err(s)[:200])


def make_levelset(path):
    v = [0] * 256
    for k, x in TEST_LEVELS.items():
        v[k] = x
    open(path, 'wb').write(struct.pack('<256i', *v))
    print('wrote %s (%s)' % (path, ' '.join('[%d]=%d' % kv for kv in sorted(TEST_LEVELS.items()))))


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == 'make-levelset':
        make_levelset(sys.argv[2])
        return 0
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--levelset', help='wb_serve 的 W906_LEVELSET_PATH 指的同一個檔（不給＝GET /api/system/levelset）')
    ap.add_argument('--login', action='append', default=[], help='L=帳號:密碼（可重複）')
    ap.add_argument('--customer-code', type=int, help='不給＝讀 D:\\HT9045\\system\\Gerneral.ini CUSTOMER_CODE')
    ap.add_argument('--honprec-level', type=int, default=3, help='iDefHonPrecLevel（移植樹 cmydef.cpp:3865 = 3）')
    ap.add_argument('--no-open', action='store_true', help='預期可開的頁不送 editlist.get（不跑 golden FormShow）')
    ap.add_argument('--expect-running', action='store_true')
    a = ap.parse_args()
    creds = {}
    for s in a.login:
        lv, rest = s.split('=', 1)
        user, pw = rest.split(':', 1) if ':' in rest else ('', rest)
        creds[int(lv)] = (user, pw)

    ws = Ws(a.port)
    r = ws.cmd('control.acquire')
    check(bool(r and r.get('ok')), 'control.acquire')
    st0 = ws.cmd('auth.mode') or {}
    print('auth.mode：%s' % short(st0))
    LS = read_levels(a)
    check(len(LS) >= 256, 'levelset 讀到 %d 格' % len(LS))
    if len(LS) < 256:
        return len(FAILS)
    cc = customer_code(a)
    info('CUSTOMER_CODE=%d' % cc)

    if st0.get('systemStart'):
        run_running(ws, a)
    else:
        check(not a.expect_running, '--expect-running 但機台沒在運轉（auth.mode systemStart=false）')
        start = st0.get('level')
        ok0 = set_level(ws, 0, creds)
        check(bool(ok0), 'L0 切到 Operator（等級 0）')
        if ok0:
            run_level(ws, a, 0, LS, cc)
        for lv in sorted(k for k in creds if k > 0):
            if set_level(ws, lv, creds):
                run_level(ws, a, lv, LS, cc)
            else:
                check(False, '切到等級 %d（--login 的帳密）' % lv)
        if start is not None and start != 0:
            back = set_level(ws, start, creds)
            if back is None:
                print('  WARN  等級停在 Operator：沒有等級 %d 的 --login，請自己登入回來' % start)
            else:
                check(bool(back), '回到開始的等級 %d' % start)
    ws.cmd('control.release')
    print('FAILS: %d' % len(FAILS))
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
