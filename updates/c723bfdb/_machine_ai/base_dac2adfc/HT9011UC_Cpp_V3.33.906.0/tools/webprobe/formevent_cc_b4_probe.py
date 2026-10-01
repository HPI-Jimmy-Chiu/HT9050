# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/formevent_cc_b4_probe.py -- WS form.event 的 e2e 探針：Config.Configuration 批次 B4
#  （CC-E1 btD47、CC-E3 btnRecordJamRateByTimeClear、CC-E4 btResume、CC-E6 cbA09 群組、CC-E8 8 條 EP 滑桿）。
#
#  AI(W906-EVB4) 20260928 [W906]  NOT in golden。Steven 20260928 事件派工（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
#  第三節 B4）。C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp 檔尾 (6)（IC_EvA09Recheck、IC_EvSlide、IC_EvCreateProxies）＋
#  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc 檔尾 kIC_Events（設定 tools\editlist\IniConfig.py 'events'）。
#  golden：D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp :6062 btD47Click、:6691 btnRecordJamRateByTimeClearClick、
#          :6257 btResumeClick、:6453 cbA09Click、:6271-:6684 tbD25_*／tbD60_* Change、:6025 Timer1Timer（每個事件後補一拍）。
#  共用 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code；失敗清單同一份）。前一批（CC-E2／CC-E7）見 formevent_cc_probe.py。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，.claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗什麼（直接走 WS，不用瀏覽器）：
#    B0 editlist.get IniConfig → "events" 32 個控制項（前一批 18＋這一批 14），B4 那 14 個的 event 名對；eventTag＝"Config.Configuration"
#    B1 tbD25_Index60mm（點得到才驗）change position＝原值±1 → ok、route C、changed.labD25_1.caption 帶新值；送回原值
#       position 999 → changed.tbD25_Index60mm.position＝Max（115）＋todo 提到 "outside Min..Max"；再送回原值
#       "click" → no-handler；btD47 帶 position → bad-payload（position 只給 ELTrackBar 的 change）
#    B2 cbA09 點一下（SIM 機台通常沒有料 ⇒ 不會改回）→ ok；changed 沒有 cbA09（沒被改回）；再點回
#       chkA09_1 點一下 → ok（golden 三格綁 cbA09Click，只看 cbA09）；再點回
#    --allow-actions 才跑（會改機台記憶體，但都不寫檔）：
#    B3 btD47 → ok、changed.edD47_3.text＝"0"（LastSet.iD47SocketTestedCount 歸零；存檔才寫 LastSet.ini）——btD47 看 cbD47（Timer1 一拍），點不到就 SKIP
#    B4 btnRecordJamRateByTimeClear → ok（兩個 Jam 率計數歸零、bRecordJamRateByTime_Clear=true）
#    B5 btResume（只有 CC_MTI 看得到）→ ok、messages 有 "Resume handler manually!"（bLockByServer=false）
#    B6 tbD60_Index56mm change → ok、changed.labD60.caption 帶新值（golden :6652 當場 RecordProcess 記一筆 EP log ⇒ 會寫事件紀錄）
#    --allow-save 才跑：
#    B7 存檔補重查 A09：重開頁 → editlist.save 送開頁值、cbA09 反過來（沒送 form.event）、answers 不答（＝NO，config.ini 不寫；
#       golden FormClose 仍照 golden 補寫缺鍵、跑關窗尾段，同 formevent_cc_probe.py C6）→ ok、ack.events 含 cbA09；
#       機台沒料 ⇒ ack.ignored 沒有 cbA09。有料時（實機）⇒ ack.ignored 有 cbA09（頁面值不收，IC_EvA09Recheck）
#    最後 editlist.get IniConfig 一次（golden FormShow：ReadLastSetIni 把替身換回檔案值）。
#
#  ⚠ 沒辦法在 SIM 造「機台內有 IC」：cbA09 被改回（MES1645／MES1646）那一支只能實機驗，這裡只驗沒料的路。
#    實機有料時跑 B2 會照 golden 跳 MES1645／MES1646 告警（ShowErrorMessage 先 StopAllMotor，golden note.cpp:795-801 同）。
#  ⚠ D25 滑桿事件會把 bEP*DataChange 設成 true（記憶體）⇒ 之後任何一次真的存檔會多寫一筆 EP 變更 log（golden 同）。
#  ⚠ 權限：gbD25／grpD47／gbN04 依 AccessLevel 停用 → 帶 --user／--password。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\formevent_cc_b4_probe.py --port 8046 [--user U --password P] [--allow-actions] [--allow-save] [--gap 0.6]
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
TAG, F = 'Config.Configuration', 'TfConfiguration'
VALUE_KEYS = ('checked', 'itemIndex', 'text', 'position', 'dateTime', 'cells', 'tag')
OLD = ['udD46', 'cbE30', 'cbE31', 'cbE31_1', 'cbE31_2', 'cbE32', 'cbE32_1', 'cbE32_2', 'cbE33', 'cbE39',
       'cbD36', 'cbD37', 'cbD38', 'cbD36_1', 'cbD36_2', 'cbD21', 'cbD47', 'cbF05']
B4 = {'btD47': 'click', 'btnRecordJamRateByTimeClear': 'click', 'btResume': 'click',
      'cbA09': 'click', 'chkA09_1': 'click', 'cbA14': 'click',
      'tbD25_Index60mm': 'change', 'tbD25_Index30mm': 'change', 'tbD25_Index40mm': 'change',
      'tbD25_Index60mm_NS': 'change', 'tbD25_Index40mm_NS': 'change', 'tbD25_Index30mm_NS': 'change',
      'tbD60_Index56mm': 'change', 'tbD60_Index56mm_NS': 'change'}


def widgets_from(get_ack):
    out = {}
    for name, p in ((get_ack or {}).get('proxies') or {}).items():
        v = {k: p[k] for k in VALUE_KEYS if k in p}
        if 'itemIndex' in v and 'text' in p:
            v['text'] = p['text']
        if v:
            out[name] = v
    return out


def ok_c(r):
    return bool(r and r.get('ok')) and r.get('route') == 'C'


def dump(r):
    return json.dumps(r, ensure_ascii=False)[:300]


def get(ws):
    g = ws.cmd('editlist.get', 'IniConfig')
    return g if g and g.get('ok') else None


def changed(r, name, key, default=None):
    return (((r or {}).get('changed') or {}).get(name) or {}).get(key, default)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--allow-actions', action='store_true', help='B3～B6：真的按 D47 Clear／Jam 率清除／Resume、拖 D60（改記憶體、D60 記 EP log）')
    ap.add_argument('--allow-save', action='store_true', help='B7：真的跑 golden FormClose（答 NO，不寫 config.ini）')
    ap.add_argument('--gap', type=float, default=0.6, help='事件之間的間隔（WebCmdGuard 400 ms）')
    a = ap.parse_args()

    ws = fp.Ws(a.port)
    r = ws.cmd('control.acquire')
    check(bool(r and r.get('ok')), 'control.acquire（form.event／editlist.save 要權杖）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)

    g = get(ws)
    check(bool(g), 'B0 editlist.get IniConfig')
    if not g:
        return len(fp.FAILS)
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    check(sorted(evs) == sorted(OLD + list(B4)), 'B0 events 32 個：%s' % sorted(evs))
    check(all((evs.get(k) or {}).get('event') == v for k, v in B4.items()), 'B0 B4 事件名：%s' % {k: (evs.get(k) or {}).get('event') for k in B4})
    check(g.get('eventTag') == TAG, 'B0 eventTag＝%s：%s' % (TAG, g.get('eventTag')))
    check('btnRecordJamRateByTimeClear' in prox, 'B0 btnRecordJamRateByTimeClear 有替身（IC_EvCreateProxies）')

    # B1 D25 滑桿
    if (evs.get('tbD25_Index60mm') or {}).get('operable'):
        p0 = int((prox.get('tbD25_Index60mm') or {}).get('position', 80))
        p1 = p0 + 1 if p0 < 115 else p0 - 1
        r = ws.event(TAG, F, 'tbD25_Index60mm', 'change', position=p1)
        cap = changed(r, 'labD25_1', 'caption', '')
        check(ok_c(r) and ('%g' % (p1 / 100.0)) in cap, 'B1 tbD25_Index60mm → %d、labD25_1=%r：%s' % (p1, cap, dump(r)))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'tbD25_Index60mm', 'change', position=999)
        # 夾成 115；golden EP 保護（:6289-:6299，old60data<85 時把 Position 拉回 85）也可能接著把它改成 85 ⇒ 兩個都算對
        check(ok_c(r) and changed(r, 'tbD25_Index60mm', 'position') in (115, 85) and
              any('outside Min..Max' in t for t in (r.get('todo') or [])), 'B1 position 999 → 夾成 115（或 EP 保護的 85）＋todo：%s' % dump(r))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'tbD25_Index60mm', 'change', position=p0)
        check(ok_c(r), 'B1 送回原值 %d：%s' % (p0, dump(r)))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'tbD25_Index60mm', 'click')
        check(bool(r) and not r.get('ok') and err_code(r) == 'no-handler', 'B1 滑桿 "click" → no-handler：%s' % dump(r))
        time.sleep(a.gap)
    else:
        print('  SKIP  B1 tbD25_Index60mm 點不到（gbD25 依等級停用）')
    r = ws.event(TAG, F, 'btD47', 'click', position=1)
    check(bool(r) and not r.get('ok') and err_code(r) == 'bad-payload', 'B1 btD47 帶 position → bad-payload：%s' % dump(r))
    time.sleep(a.gap)

    # B2 A09 群組（沒料的路）
    for ctl in ('cbA09', 'chkA09_1'):
        if not (evs.get(ctl) or {}).get('operable'):
            print('  SKIP  B2 %s 點不到（權限或客戶碼）' % ctl)
            continue
        c0 = bool((prox.get(ctl) or {}).get('checked'))
        r = ws.event(TAG, F, ctl, 'click', checked=not c0)
        check(ok_c(r) and changed(r, 'cbA09', 'checked') is None, 'B2 %s=%s → ok、cbA09 沒被改回（SIM 沒料）：%s' % (ctl, not c0, dump(r)))
        time.sleep(a.gap)
        ws.event(TAG, F, ctl, 'click', checked=c0)
        time.sleep(a.gap)

    if a.allow_actions:
        g = get(ws)
        evs, prox = (g or {}).get('events') or {}, (g or {}).get('proxies') or {}
        if (evs.get('btD47') or {}).get('operable'):                                     # B3
            r = ws.event(TAG, F, 'btD47', 'click')
            txt = changed(r, 'edD47_3', 'text', (prox.get('edD47_3') or {}).get('text'))
            check(ok_c(r) and txt == '0', 'B3 btD47 → edD47_3=%r：%s' % (txt, dump(r)))
            time.sleep(a.gap)
        else:
            print('  SKIP  B3 btD47 點不到（cbD47 沒勾：Timer1 顯示段把它藏起來）')
        if (evs.get('btnRecordJamRateByTimeClear') or {}).get('operable'):              # B4
            r = ws.event(TAG, F, 'btnRecordJamRateByTimeClear', 'click')
            check(ok_c(r), 'B4 btnRecordJamRateByTimeClear → ok：%s' % dump(r))
            time.sleep(a.gap)
        else:
            print('  SKIP  B4 btnRecordJamRateByTimeClear 點不到（pal_O2 依等級停用）')
        if (evs.get('btResume') or {}).get('operable'):                                 # B5
            r = ws.event(TAG, F, 'btResume', 'click')
            msgs = ' '.join((m.get('en') or '') for m in ((r or {}).get('messages') or []))
            check(ok_c(r) and 'Resume handler manually!' in msgs, 'B5 btResume → 訊息：%s' % dump(r))
            time.sleep(a.gap)
        else:
            print('  SKIP  B5 btResume 看不到（只有 CC_MTI）')
        if (evs.get('tbD60_Index56mm') or {}).get('operable'):                          # B6
            q0 = int((prox.get('tbD60_Index56mm') or {}).get('position', 80))
            q1 = q0 + 1 if q0 < 150 else q0 - 1
            r = ws.event(TAG, F, 'tbD60_Index56mm', 'change', position=q1)
            check(ok_c(r) and ('%g' % (q1 / 100.0)) in changed(r, 'labD60', 'caption', ''), 'B6 tbD60_Index56mm → %d：%s' % (q1, dump(r)))
            time.sleep(a.gap)
            ws.event(TAG, F, 'tbD60_Index56mm', 'change', position=q0)
            time.sleep(a.gap)
        else:
            print('  SKIP  B6 tbD60_Index56mm 點不到')

    if a.allow_save and (evs.get('cbA09') or {}).get('operable'):                         # B7
        g = get(ws)
        w = widgets_from(g)
        a09 = bool(((g or {}).get('proxies') or {}).get('cbA09', {}).get('checked'))
        w['cbA09'] = {'checked': not a09}
        s = ws.cmd('editlist.save', 'IniConfig', json.dumps({'widgets': w, 'answers': {}}))
        check(bool(s and s.get('ok')) and 'cbA09' in (s.get('events') or []) and s.get('saved') is False,
              'B7 沒送 form.event 就改 A09 存檔（答 NO）→ ack.events 含 cbA09：%s' % dump(s))
        print('  INFO  B7 ack.ignored 含 cbA09＝%s（SIM 沒料應為 False；有料＝True，頁面值不收）' % ('cbA09' in ((s or {}).get('ignored') or [])))
    get(ws)
    ws.cmd('control.release')
    print('失敗 %d 項' % len(fp.FAILS))
    return len(fp.FAILS)


if __name__ == '__main__':
    sys.exit(main())
