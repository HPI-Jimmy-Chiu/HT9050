# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/evb3_formevent_probe.py -- WS form.event 的 e2e 探針：批次 B3 lane 1（Yield／Cleaning／Contact）。
#
#  AI(W906-EVB3) 20260928 [W906]  NOT in golden。Steven 20260928「任何畫面的事件, 都是我們做」；
#  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 批次 B3：YM-2、CL-1、CL-2、CL-3、CL-5、CL-7、CT-1、CT-L1。
#  C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_YieldMonitoring.cpp 檔尾、FileRW\TestIF_File_Cleaning.cpp 檔尾、
#       FileRW\DeviceForm_File.cpp 檔尾（事件表跳板、存檔重播基準、Contact 的機台記憶體 session）。
#  頁面：D:\HT9045\web\page\ht9045_yield_ev.js、ht9045_cleaning_ev.js、ht9045_contact_ev.js。
#  共用的 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code）。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見 .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#  ⚠ 每一次 editlist.get 都是 golden FormShow：Cleaning 的 LoadAutoCleanData 本身就會寫檔（FileRW/TestIF_File_Cleaning.cpp 檔頭），
#    Contact 開頁會跑主畫面尾段（DoStructUnitConvert、SetWorkParameter）—— 跑之前照 tools/webprobe/wbrun_guard.py 備份。
#
#  驗什麼（直接走 WS，不用瀏覽器）：
#    Y0 editlist.get TestIF_File_YieldMonitoring → events 有 btnResetInterval（event click）
#    Y1 btnResetInterval click（點得到才驗）→ ok、route C、changed 是空的（golden 只改全域 dAdaptiveStardardYield）
#    C0 editlist.get TestIF_File_Cleaning → events 83 個：btInclude／btnResetCleanCount／btnResetInterval／sbTrayAssign／rgCleanKitType／
#       cbbSelectTray＋77 個小鍵盤格（event 都是 click，cbbSelectTray 是 change）
#    C1 XCT1（Kit 頁，點得到才驗）送 text "7" → golden XCT1Click 夾 1..100 → OnChange XCT1Change（:2183-2187 4..7 → 4）→ changed.XCT1.text＝"4"；
#       再送回原值
#    C2 btInclude（點得到才驗）→ ok；changed 帶 XCT2 或 YCT2（golden :2273／:2276；YCT2 一定是 "2"）
#    C3 sbTrayAssign（點得到才驗）→ ok、todo 有 "main:AutoCleanStringGrid"
#    C4（--allow-write 才跑）btnResetCleanCount → ok（寫清潔計數檔、SECS 事件）；btnResetInterval → ok（Smart AC 開著時寫配方 -1）
#    K0 editlist.get DeviceForm_File → events 8 個
#    K1 rgKitDiameter 換一個選項（點得到才驗）→ ok、changed 帶 edAirForce 或 lblMinForce；再點回原選項
#    K2 cbContactMode 換一個選項 → ok；再換回
#    K3 pnlSensorAdj（點得到才驗）→ ok；--level109 時 todo 要有 "open:cclink"，否則要沒有（golden 等級 109，不足跳 WAR1676）
#    最後各頁 editlist.get 一次（替身回檔案值）。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\evb3_formevent_probe.py --port 8046 [--user U --password P] [--allow-write] [--level109] [--gap 0.6]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import formevent_probe as fp   # noqa: E402  Ws／check／err_code／FAILS

check, err_code = fp.check, fp.err_code
YM, CL, DF = 'TestIF_File_YieldMonitoring', 'TestIF_File_Cleaning', 'DeviceForm_File'


def ok_c(r):
    return bool(r and r.get('ok')) and r.get('route') == 'C'


def dump(r):
    return json.dumps(r, ensure_ascii=False)[:300]


def get(ws, tag):
    g = ws.cmd('editlist.get', tag)
    return g if g and g.get('ok') else None


def operable(evs, ctl):
    return bool((evs.get(ctl) or {}).get('operable'))


def skip(label, ctl):
    print('  SKIP  %s %s 點不到（權限、客戶碼或 golden 顯示條件）' % (label, ctl))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--allow-write', action='store_true', help='C4：真的跑會寫檔的兩顆鈕（Reset Clean Count／Reset Interval）')
    ap.add_argument('--level109', action='store_true', help='K3：登入的等級 >= LevelSet.AccessLevel[109]')
    ap.add_argument('--gap', type=float, default=0.6, help='事件之間的間隔（WebCmdGuard 400 ms）')
    a = ap.parse_args()

    ws = fp.Ws(a.port)
    r = ws.cmd('control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
    check(bool(r and r.get('ok')), 'control.takeover（form.event 要權杖）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)

    # ---- Yield
    g = get(ws, YM)
    check(bool(g), 'Y0 editlist.get %s' % YM)
    evs = (g or {}).get('events') or {}
    check((evs.get('btnResetInterval') or {}).get('event') == 'click', 'Y0 events.btnResetInterval：%s' % evs.get('btnResetInterval'))
    if operable(evs, 'btnResetInterval'):
        r = ws.event(YM, 'TfYieldMonitoring', 'btnResetInterval', 'click')
        check(ok_c(r) and not (r.get('changed') or {}), 'Y1 btnResetInterval → ok、changed 空：%s' % dump(r))
        time.sleep(a.gap)
    else:
        skip('Y1', 'btnResetInterval（CosFunction.bAdaptiveYield）')

    # ---- Cleaning
    g = get(ws, CL)
    check(bool(g), 'C0 editlist.get %s' % CL)
    evs, prox = (g or {}).get('events') or {}, (g or {}).get('proxies') or {}
    check(len(evs) == 83, 'C0 events 83 個：%d' % len(evs))
    for c in ('btInclude', 'btnResetCleanCount', 'btnResetInterval', 'sbTrayAssign', 'rgCleanKitType', 'XCT1'):
        check((evs.get(c) or {}).get('event') == 'click', 'C0 events.%s click：%s' % (c, evs.get(c)))
    check((evs.get('cbbSelectTray') or {}).get('event') == 'change', 'C0 events.cbbSelectTray change')
    if operable(evs, 'XCT1') and ((prox.get('pgCleanType') or {}).get('activePageIndex') == 0):
        t0 = (prox.get('XCT1') or {}).get('text', '')
        r = ws.event(CL, 'TfCleaning', 'XCT1', 'click', text='7', state={'pgCleanType': {'activePageIndex': 0}})
        check(ok_c(r) and ((r.get('changed') or {}).get('XCT1') or {}).get('text') == '4', 'C1 XCT1=7 → golden XCT1Change 夾成 4：%s' % dump(r))
        time.sleep(a.gap)
        r = ws.event(CL, 'TfCleaning', 'XCT1', 'click', text=t0, state={'pgCleanType': {'activePageIndex': 0}})
        check(ok_c(r), 'C1 XCT1 送回原值 %r：%s' % (t0, dump(r)))
        time.sleep(a.gap)
    else:
        skip('C1', 'XCT1（或 pgCleanType 不在 Kit 頁）')
    if operable(evs, 'btInclude'):
        r = ws.event(CL, 'TfCleaning', 'btInclude', 'click')
        ch = (r or {}).get('changed') or {}
        check(ok_c(r) and ('XCT2' in ch or 'YCT2' in ch or (prox.get('YCT2') or {}).get('text') == '2'),
              'C2 btInclude → ok、Clean Tray 欄位照 Loader 填：%s' % dump(r))
        time.sleep(a.gap)
    else:
        skip('C2', 'btInclude')
    if operable(evs, 'sbTrayAssign'):
        r = ws.event(CL, 'TfCleaning', 'sbTrayAssign', 'click')
        check(ok_c(r) and any(str(t).startswith('main:AutoCleanStringGrid') for t in (r.get('todo') or [])),
              'C3 sbTrayAssign → todo main:AutoCleanStringGrid：%s' % dump(r))
        time.sleep(a.gap)
    else:
        skip('C3', 'sbTrayAssign（CosFunction.bAutoCleanAutoSelIndexArm）')
    if a.allow_write:
        for c in ('btnResetCleanCount', 'btnResetInterval'):
            if operable(evs, c):
                r = ws.event(CL, 'TfCleaning', c, 'click')
                check(ok_c(r), 'C4 %s → ok：%s' % (c, dump(r)))
                time.sleep(a.gap)
            else:
                skip('C4', c)
    get(ws, CL)

    # ---- Contact
    g = get(ws, DF)
    check(bool(g), 'K0 editlist.get %s' % DF)
    evs, prox = (g or {}).get('events') or {}, (g or {}).get('proxies') or {}
    check(sorted(evs) == sorted(['cbContactMode', 'rgKitDiameter', 'rgOutKitDiameter', 'rgDieForceKitDiameter', 'chkUseAddWeight',
                                 'cbEnableUK', 'coD41', 'pnlSensorAdj']), 'K0 events 8 個：%s' % sorted(evs))
    for c, lab in (('rgKitDiameter', 'K1'), ('cbContactMode', 'K2')):
        items = (evs.get(c) or {}).get('items') or []
        i0 = (prox.get(c) or {}).get('itemIndex', -1)
        if not operable(evs, c) or len(items) < 2 or i0 < 0:
            skip(lab, c)
            continue
        i1 = 1 if i0 == 0 else 0
        kw = {'itemIndex': i1}
        if c == 'cbContactMode':
            kw['text'] = items[i1]
        r = ws.event(DF, 'TfContact', c, (evs.get(c) or {}).get('event'), **kw)
        ch = (r or {}).get('changed') or {}
        want = ('edAirForce' in ch or 'lblMinForce' in ch or 'edForcePerDeviceKG' in ch) if c == 'rgKitDiameter' else True
        check(ok_c(r) and want, '%s %s %d→%d：%s' % (lab, c, i0, i1, dump(r)))
        time.sleep(a.gap)
        kw['itemIndex'] = i0
        if c == 'cbContactMode':
            kw['text'] = items[i0]
        r = ws.event(DF, 'TfContact', c, (evs.get(c) or {}).get('event'), **kw)
        check(ok_c(r), '%s %s 換回 %d：%s' % (lab, c, i0, dump(r)))
        time.sleep(a.gap)
    if operable(evs, 'pnlSensorAdj'):
        r = ws.event(DF, 'TfContact', 'pnlSensorAdj', 'click')
        has = any(str(t).startswith('open:cclink') for t in ((r or {}).get('todo') or []))
        check(ok_c(r) and has == bool(a.level109), 'K3 pnlSensorAdj → open:cclink %s：%s' % ('有' if a.level109 else '沒有', dump(r)))
        time.sleep(a.gap)
    else:
        skip('K3', 'pnlSensorAdj（SHUTTLE_SENSOR_TYPE）')
    get(ws, DF)
    get(ws, YM)
    ws.cmd('control.release')
    print('失敗 %d 項' % len(fp.FAILS))
    return len(fp.FAILS)


if __name__ == '__main__':
    sys.exit(main())
