# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/formevent_b3_lane2_probe.py -- WS form.event 的 e2e 探針：事件移植批次 B3 第 2 線
#    Setup.BarCode（BC-1、BC-3）、Setup.TrayForm（TF-4）、Setup.Temp_Set（TS-2）、Setup.OffSet（OS-1）。
#
#  AI(W906-EVB3) 20260928 [W906]  NOT in golden。批次 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B3。
#  C++：FileRW/TestIF_File_BarCode.cpp、FileRW/UserDefForm_File.cpp、FileRW/Temperature.cpp、FileRW/Offset_File.cpp 各自檔尾
#       ＋同名 .gen.inc 檔尾的 k<P>_Events。共用的 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code／FAILS）。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見 .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗什麼：
#    B  BarCode：B0 editlist.get TestIF_File_BarCode → events 有 chkMulti2DID／rgMulti2DType／btResetBarCodeCount；
#       B1 chkMulti2DID 點一下 → changed.tsMulti2DID.tabVisible＝新的勾選（golden :8825）；點回原值。
#       B2 rgMulti2DType（點得到才驗）換一項 → cbAa 的 items 是 ["---","1","2"(,"3","4")]、itemIndex 0（golden SetMulti2DMap :8830）；換回。
#       B3 btResetBarCodeCount：點得到（模擬版＋等級夠）→ ok；點不到 → bad-payload（golden gbManualTest 看不見）。
#    F  TrayForm：F0 events 有 btnBinBoxReset；F1（--allow-reset 才跑：會清掉機台記憶體裡 Fix3 盤的格子資料）點一下 → ok、
#       changed.edtBinBoxNow.text＝"0"（沒變＝本來就是 0）。
#    T  Temp_Set：T0 events 有 rbATCActiveOn／rbATC70ActiveOn／cbATCReferTempSensor（＋三格 ATC 7.0 勾選）；
#       T1 cbATCReferTempSensor（點得到才驗）取消勾選 → changed.cbUseTC2Offset.visible=false（golden :7073）；點回。
#       T2 rbATCActiveOn（點得到才驗）勾起 → changed.rbATC70ActiveOn 取消／停用（golden :5430）；點回。
#       最後 editlist.get 一次（golden FormShow 把替身換回檔案值）。
#    O  Offset：O0 editlist.get Offset_File → 有 eventTag "Setup.OffSet"、events 有 4 顆微調鈕＋部位鈕；
#       O1 沒先選部位就點微調 → handler-failed「select-first」；
#       O2（--allow-save 才跑：會寫 Position Offset*.Data）選第一個點得到的部位 → 點 sb_AutoOffsetUp（state 帶 edArmY）→ ok、
#          changed.edArmY.text＝原值+0.10；再點 sb_AutoOffsetDown 還原；重開頁 edArmY＝原值。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\formevent_b3_lane2_probe.py --port 8046 [--user U --password P] [--allow-save] [--allow-reset] [--only B|F|T|O]
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


def ok_c(r):
    return bool(r and r.get('ok')) and r.get('route') == 'C'


def short(r, n=260):
    return json.dumps(r, ensure_ascii=False)[:n]


def barcode(ws, a):
    print('== B  Setup.BarCode（C 路 TestIF_File_BarCode，golden BarCode\\BarCode.cpp）')
    F, T = 'TfBarCode', 'Setup.BarCode'
    g = ws.cmd('editlist.get', 'TestIF_File_BarCode')
    check(bool(g and g.get('ok')), 'B0 editlist.get：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    check(all(n in evs for n in ('chkMulti2DID', 'rgMulti2DType', 'btResetBarCodeCount')), 'B0 events：%s' % sorted(evs))
    if (evs.get('chkMulti2DID') or {}).get('operable'):
        c0 = bool((prox.get('chkMulti2DID') or {}).get('checked'))
        r = ws.event(T, F, 'chkMulti2DID', 'click', checked=not c0)
        tv = ((r or {}).get('changed') or {}).get('tsMulti2DID', {}).get('tabVisible', c0)
        check(ok_c(r) and tv == (not c0), 'B1 chkMulti2DID=%s → tsMulti2DID.tabVisible=%s：%s' % (not c0, not c0, short(r)))
        time.sleep(a.gap)
        ws.event(T, F, 'chkMulti2DID', 'click', checked=c0)
        time.sleep(a.gap)
    else:
        print('  SKIP  B1 chkMulti2DID 點不到（CosFunction.bEnableMulti2D 關）')
    if (evs.get('rgMulti2DType') or {}).get('operable'):
        i0 = (prox.get('rgMulti2DType') or {}).get('itemIndex', 0)
        other = 3 if i0 != 3 else 0
        r = ws.event(T, F, 'rgMulti2DType', 'click', itemIndex=other)
        cb = ((r or {}).get('changed') or {}).get('cbAa', {})
        want = ['---', '1', '2', '3', '4'] if other >= 3 else ['---', '1', '2']
        check(ok_c(r) and cb.get('items', want) == want and cb.get('itemIndex', 0) == 0, 'B2 rgMulti2DType=%d → cbAa items %s：%s' % (other, want, short(r)))
        time.sleep(a.gap)
        ws.event(T, F, 'rgMulti2DType', 'click', itemIndex=i0)
        time.sleep(a.gap)
    else:
        print('  SKIP  B2 rgMulti2DType 點不到（tsMulti2DID 看不見）')
    op = (evs.get('btResetBarCodeCount') or {}).get('operable')
    r = ws.event(T, F, 'btResetBarCodeCount', 'click')
    if op:
        check(ok_c(r) and 'BarCode.cpp:2424' in (r or {}).get('golden', ''), 'B3 btResetBarCodeCount → ok：%s' % short(r))
    else:
        check(bool(r) and not r.get('ok') and err_code(r) == 'bad-payload', 'B3 btResetBarCodeCount 點不到 → bad-payload：%s' % short(r))
    time.sleep(a.gap)
    ws.cmd('editlist.get', 'TestIF_File_BarCode')


def trayform(ws, a):
    print('== F  Setup.TrayForm（C 路 UserDefForm_File，golden cTrayForm.cpp）')
    g = ws.cmd('editlist.get', 'UserDefForm_File')
    check(bool(g and g.get('ok')), 'F0 editlist.get：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    evs = g.get('events') or {}
    check('btnBinBoxReset' in evs, 'F0 events 有 btnBinBoxReset：%s' % sorted(evs))
    if not a.allow_reset:
        print('  SKIP  F1（--allow-reset 才跑：golden 會清 MOT[MManualTray3] 的格子資料）')
        return
    if (evs.get('btnBinBoxReset') or {}).get('operable'):
        r = ws.event('Setup.TrayForm', 'TfTrayForm', 'btnBinBoxReset', 'click')
        t = ((r or {}).get('changed') or {}).get('edtBinBoxNow', {}).get('text', '0')
        check(ok_c(r) and t == '0', 'F1 btnBinBoxReset → edtBinBoxNow "0"：%s' % short(r))
    else:
        print('  SKIP  F1 btnBinBoxReset 點不到（IniConfig.bBinBox 關）')


def temperature(ws, a):
    print('== T  Setup.Temp_Set（C 路 Temperature，golden uTemp_Set.cpp）')
    F, T = 'TfTemp_Set', 'Setup.Temp_Set'
    g = ws.cmd('editlist.get', 'Temperature')
    check(bool(g and g.get('ok')), 'T0 editlist.get：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    need = ['rbATCActiveOn', 'rbATC70ActiveOn', 'cbEnableATCConsFailOffset', 'cbEnableATCQAModeOffset', 'cbATCTestTimeOffset', 'cbATCReferTempSensor']
    check(all(n in evs for n in need), 'T0 events 有 TS-2 六列：%s' % sorted(evs))
    if (evs.get('cbATCReferTempSensor') or {}).get('operable') and (prox.get('cbATCReferTempSensor') or {}).get('checked'):
        r = ws.event(T, F, 'cbATCReferTempSensor', 'click', checked=False)
        v = ((r or {}).get('changed') or {}).get('cbUseTC2Offset', {}).get('visible', (prox.get('cbUseTC2Offset') or {}).get('visible'))
        check(ok_c(r) and v is False, 'T1 取消參考感測器 → cbUseTC2Offset 藏起來：%s' % short(r))
        time.sleep(a.gap)
        ws.event(T, F, 'cbATCReferTempSensor', 'click', checked=True)
        time.sleep(a.gap)
    else:
        print('  SKIP  T1 cbATCReferTempSensor 點不到或本來沒勾')
    if (evs.get('rbATCActiveOn') or {}).get('operable') and not (prox.get('rbATCActiveOn') or {}).get('checked'):
        r = ws.event(T, F, 'rbATCActiveOn', 'click', checked=True)
        ch = ((r or {}).get('changed') or {}).get('rbATC70ActiveOn', {})
        check(ok_c(r) and ch.get('checked', False) is False, 'T2 勾 ATC Active → ATC 7.0 取消：%s' % short(r))
        time.sleep(a.gap)
        ws.event(T, F, 'rbATCActiveOn', 'click', checked=False)
        time.sleep(a.gap)
    else:
        print('  SKIP  T2 rbATCActiveOn 點不到（ATC 沒裝／等級 158）或本來就勾著')
    ws.cmd('editlist.get', 'Temperature')


def offset(ws, a):
    print('== O  Setup.OffSet（Offset_File，golden cOffSet.cpp；form.event 別名頁 Setup.OffSet）')
    F = 'TfOffSet'
    g = ws.cmd('editlist.get', 'Offset_File')
    check(bool(g and g.get('ok')), 'O0 editlist.get：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    tag, evs = g.get('eventTag'), g.get('events') or {}
    check(tag == 'Setup.OffSet' and all(n in evs for n in ('sb_AutoOffsetUp', 'sb_AutoOffsetDown', 'sb_AutoOffsetRight', 'sb_AutoOffsetLeft', 'sbLoader')),
          'O0 eventTag %r、events %d 個' % (tag, len(evs)))
    if not (evs.get('sb_AutoOffsetUp') or {}).get('operable'):
        print('  SKIP  O1／O2 微調鈕點不到（IniConfig.bUseAutoOffsetFunction 關 → pan_AutoOffsetMove 看不見）')
        return
    r = ws.event(tag, F, 'sb_AutoOffsetUp', 'click')
    check(bool(r) and not r.get('ok') and 'select-first' in (r.get('error') or ''), 'O1 沒先選部位 → select-first：%s' % short(r))
    time.sleep(a.gap)
    if not a.allow_save:
        print('  SKIP  O2（--allow-save 才跑：會寫 Position Offset*.Data）')
        return
    key = next((k for k, v in sorted((g.get('offsets') or {}).items()) if ':' not in k and v.get('clickable') and not v.get('refused')
                and (evs.get(v.get('button')) or {}).get('operable') and 'edArmY' in (v.get('widgets') or {})), None)
    if key is None:
        print('  SKIP  O2 沒有點得到、又有 edArmY 的部位')
        return
    grp = g['offsets'][key]
    y0 = (grp['widgets']['edArmY'] or {}).get('text', '0')
    for btn, want in (('sb_AutoOffsetUp', '%.2f' % (float(y0 or 0) + 0.1)), ('sb_AutoOffsetDown', '%.2f' % float(y0 or 0))):
        r1 = ws.event(tag, F, grp['button'], 'click')
        time.sleep(a.gap)
        cur = y0 if btn == 'sb_AutoOffsetUp' else '%.2f' % (float(y0 or 0) + 0.1)
        r2 = ws.event(tag, F, btn, 'click', state={'edArmY': {'text': cur}})
        got = ((r2 or {}).get('changed') or {}).get('edArmY', {}).get('text')
        check(ok_c(r1) and ok_c(r2) and got == want, 'O2 %s（%s）edArmY %s → %s：%s' % (btn, grp['button'], cur, want, short(r2)))
        time.sleep(a.gap)
    g2 = ws.cmd('editlist.get', 'Offset_File')
    y1 = (((g2 or {}).get('offsets') or {}).get(key, {}).get('widgets', {}).get('edArmY') or {}).get('text')
    check(y1 is not None and abs(float(y1 or 0) - float(y0 or 0)) < 0.005, 'O2 重開頁 edArmY 回到 %s：%s' % (y0, y1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--allow-save', action='store_true', help='O2：真的跑 golden spbSaveClick（會寫 Position Offset*.Data）')
    ap.add_argument('--allow-reset', action='store_true', help='F1：golden btnBinBoxResetClick（清 Fix3 盤的格子資料）')
    ap.add_argument('--gap', type=float, default=0.6, help='事件之間的間隔（WebCmdGuard 400 ms）')
    ap.add_argument('--only', choices=('B', 'F', 'T', 'O'))
    a = ap.parse_args()
    ws = fp.Ws(a.port)
    r = ws.cmd('control.acquire')
    check(bool(r and r.get('ok')), 'control.acquire（form.event 要權杖）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)
    for k, fn in (('B', barcode), ('F', trayform), ('T', temperature), ('O', offset)):
        if a.only in (None, k):
            fn(ws, a)
    ws.cmd('control.release')
    print('失敗 %d 項' % len(fp.FAILS))
    return len(fp.FAILS)


if __name__ == '__main__':
    sys.exit(main())
