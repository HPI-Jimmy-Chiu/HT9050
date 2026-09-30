# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/formevent_cc_probe.py -- WS form.event 的 e2e 探針：Config.Configuration（Q41 盤點 CC-E2 D46 上下鍵、CC-E7 勾選連動顯示）。
#
#  AI(W906-FRW-S158) 20260927 [W906]  NOT in golden。Steven ★ Q40＝A（RULINGS_20260926 S157）；Q41 盤點
#  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.19 CC-E2／CC-E7。
#  C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.cpp 檔尾（kEvPage 別名頁 "Config.Configuration"、g_evreg、IC_EvBeforeApply、
#       IC_EvKeepShown）＋ D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\IniConfig.gen.inc 檔尾 kIC_Events。
#  golden：D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp :6094 udD46Click、:6125 cbE30Click、:6369 cbE39Click、
#          :6551 cbD36Click、:6025 Timer1Timer（顯示段 :6032-6044）。
#  共用的 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code／sha；失敗清單同一份）。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見 .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗什麼（直接走 WS，不用瀏覽器）：
#    C0 editlist.get IniConfig → "events" 有 18 個控制項（udD46＋17 個勾選框）、"eventTag"＝"Config.Configuration"、
#       udD46 的 events＝["btNext","btPrev"]
#       AI(W906-Q57-TRIAGE) 20260930：⛔ 更正 —— 現在是 34 個：＋14 個 B4（875d3499：golden V912 cConfiguration.cpp :6062 btD47Click、
#       :6691 btnRecordJamRateByTimeClearClick、:6257 btResumeClick、:6453 cbA09Click（cbA09／chkA09_1／cbA14）、8 支 tbD25_*／tbD60_* OnChange）
#       ＋2 個 B10b（f14484e1：:6101 PageControl1Change、:6116 pcConfigChange）。見下面 CHECKS_B4／PAGES_B10B。
#    C1 udD46（點得到才驗）btNext → ok、route C、changed.udD46.position＝min(原值+1, 15)、changed.edD46.text＝新值；
#       btPrev 回原值；"click" → no-handler（沒有方向）
#    C2 cbE30 點一下 → changed.palE30.visible＝新的勾選；再點回
#    C3 cbE39 點一下 → cbE39_1 visible＝新的勾選（取消時 cbE39_1 checked＝false）；再點回
#    C4 cbD21 點一下 → edD21_mm visible＝新的勾選（Timer1 那一拍）；再點回
#    C5 開頁值快照（不寫檔，存檔在 FormClose 之前就被擋）：送 cbE30 事件、state 帶 cbA27＝反值 → editlist.save 送 cbA27＝反值
#       → 400 且訊息提到 A27（golden :6824 cbA27Click 要密碼；IC_OpenChecked 看開頁快照，不是被 state 改過的替身）
#    C6（--allow-save 才跑）存檔補重播：重開頁 → editlist.save 送開頁值、但 cbE30 反過來（沒送 form.event）、answers 不答（＝NO，
#       config.ini 不寫；golden FormClose 仍會照 golden 補寫 Gerneral.ini 缺鍵 iMagazineCheckZPos、跑關窗尾段）
#       → ok、ack.events 含 cbE30、saved=false；接著立刻送 cbE30 事件 → handler-failed（"reload page"：存檔＝golden FormClose 之後）
#    最後 editlist.get IniConfig 一次（golden FormShow：ReadLastSetIni 把替身換回檔案值）。
#
#  ⚠ D46：form.event 只改畫面（udD46／edD46），不改記憶體 IniConfig.iD46WaitIndexDestroyTime（偏離 golden :6098，見 IniConfig.cpp 檔尾 (3)）。
#  ⚠ 權限：pal_D5（udD46）等依 AccessLevel 停用（golden :5317-5330 等級 30／67～79）→ 帶 --user／--password。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\formevent_cc_probe.py --port 8046 [--user U --password P] [--allow-save] [--gap 0.6]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import formevent_probe as fp   # noqa: E402  Ws／check／err_code／sha／FAILS

check, err_code = fp.check, fp.err_code
TAG, F = 'Config.Configuration', 'TfConfiguration'
VALUE_KEYS = ('checked', 'itemIndex', 'text', 'position', 'dateTime', 'cells', 'tag')
CHECKS = ['cbE30', 'cbE31', 'cbE31_1', 'cbE31_2', 'cbE32', 'cbE32_1', 'cbE32_2', 'cbE33', 'cbE39',
          'cbD36', 'cbD37', 'cbD38', 'cbD36_1', 'cbD36_2', 'cbD21', 'cbD47', 'cbF05']
# AI(W906-Q57-TRIAGE) 20260930: Q57 r2／r3c FAIL（回應 34 個）＝這支探針寫在 B4／B10b 之前。tools/editlist/IniConfig.py 'events' 之後加了：
#   875d3499 B4（CC-E1/E3/E4/E6/E8）：golden V912 cConfiguration.cpp :6062 btD47Click、:6691 btnRecordJamRateByTimeClearClick、
#     :6257 btResumeClick、:6453 cbA09Click（綁 cbA09／chkA09_1／cbA14 三格，golden 怪處）、8 支滑桿 OnChange（:6271／:6303／:6336／
#     :6560／:6591／:6622／:6652／:6669）；
#   f14484e1 B10b（CC-L1／X-5）：:6101 PageControl1Change、:6116 pcConfigChange（TPageControl OnChange）。
CHECKS_B4 = ['btD47', 'btnRecordJamRateByTimeClear', 'btResume', 'cbA09', 'chkA09_1', 'cbA14',
             'tbD25_Index30mm', 'tbD25_Index30mm_NS', 'tbD25_Index40mm', 'tbD25_Index40mm_NS',
             'tbD25_Index60mm', 'tbD25_Index60mm_NS', 'tbD60_Index56mm', 'tbD60_Index56mm_NS']
PAGES_B10B = ['PageControl1', 'pcConfig']


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


def vis_after(r, prox, name):
    return ((r or {}).get('changed') or {}).get(name, {}).get('visible', (prox.get(name) or {}).get('visible'))


def toggle_check(ws, a, prox, evs, ctl, watch, label):
    if not (evs.get(ctl) or {}).get('operable'):
        print('  SKIP  %s %s 點不到（權限或客戶碼）' % (label, ctl))
        return
    c0 = bool((prox.get(ctl) or {}).get('checked'))
    r = ws.event(TAG, F, ctl, 'click', checked=not c0)
    check(ok_c(r) and vis_after(r, prox, watch) is (not c0), '%s %s=%s → %s visible=%s：%s' % (label, ctl, not c0, watch, not c0, dump(r)))
    if ctl == 'cbE39' and c0:   # 取消 E39：golden :6378-6379 cbE39_1 看不見且取消勾
        check(((r.get('changed') or {}).get('cbE39_1') or {}).get('checked', (prox.get('cbE39_1') or {}).get('checked')) is False,
              '%s cbE39 取消 → cbE39_1 checked=false' % label)
    time.sleep(a.gap)
    ws.event(TAG, F, ctl, 'click', checked=c0)
    time.sleep(a.gap)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--allow-save', action='store_true', help='C6：真的跑 golden FormClose（答 NO，不寫 config.ini）')
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
    check(bool(g), 'C0 editlist.get IniConfig')
    if not g:
        return len(fp.FAILS)
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    want_ev = ['udD46'] + CHECKS + CHECKS_B4 + PAGES_B10B   # AI(W906-Q57-TRIAGE) 20260930: 18 → 34（875d3499 B4＋f14484e1 B10b，見 CHECKS_B4）
    check(sorted(evs) == sorted(want_ev), 'C0 events %d 個（S158 18＋B4 14＋B10b 2）：多 %s／缺 %s' % (
        len(want_ev), sorted(set(evs) - set(want_ev)), sorted(set(want_ev) - set(evs))))
    check(g.get('eventTag') == TAG, 'C0 eventTag＝%s：%s' % (TAG, g.get('eventTag')))
    check((evs.get('udD46') or {}).get('events') == ['btNext', 'btPrev'], 'C0 udD46 events：%s' % (evs.get('udD46') or {}).get('events'))

    # C1
    if (evs.get('udD46') or {}).get('operable'):
        p0 = int((prox.get('udD46') or {}).get('position', 5))
        r = ws.event(TAG, F, 'udD46', 'btNext')
        ch = (r or {}).get('changed') or {}
        want = min(p0 + 1, 15)
        got = (ch.get('udD46') or {}).get('position', p0)
        txt = (ch.get('edD46') or {}).get('text', (prox.get('edD46') or {}).get('text'))
        check(ok_c(r) and got == want and txt == str(want), 'C1 udD46 btNext %d → %d、edD46=%s：%s' % (p0, want, txt, dump(r)))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'udD46', 'btPrev')
        got2 = ((r or {}).get('changed') or {}).get('udD46', {}).get('position', got)
        check(ok_c(r) and got2 == max(want - 1, 5), 'C1 udD46 btPrev → %d：%s' % (max(want - 1, 5), dump(r)))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'udD46', 'click')
        check(bool(r) and not r.get('ok') and err_code(r) == 'no-handler', 'C1 udD46 "click" → no-handler：%s' % dump(r))
        time.sleep(a.gap)
    else:
        print('  SKIP  C1 udD46 點不到（pal_D5 依等級 30／69 停用）')

    toggle_check(ws, a, prox, evs, 'cbE30', 'palE30', 'C2')
    toggle_check(ws, a, prox, evs, 'cbE39', 'cbE39_1', 'C3')
    toggle_check(ws, a, prox, evs, 'cbD21', 'edD21_mm', 'C4')

    # C5（不寫檔）
    g = get(ws)
    prox = (g or {}).get('proxies') or {}
    if (evs.get('cbE30') or {}).get('operable') and (prox.get('cbA27') or {}).get('editable'):
        a27 = bool((prox.get('cbA27') or {}).get('checked'))
        e30 = bool((prox.get('cbE30') or {}).get('checked'))
        r = ws.event(TAG, F, 'cbE30', 'click', checked=e30, state={'cbA27': {'checked': not a27}})
        check(ok_c(r), 'C5 cbE30 事件（state 帶 cbA27=%s）：%s' % (not a27, dump(r)))
        time.sleep(a.gap)
        w = widgets_from(g)
        w['cbA27'] = {'checked': not a27}
        s = ws.cmd('editlist.save', 'IniConfig', json.dumps({'widgets': w, 'answers': {}}))
        check(bool(s) and not s.get('ok') and 'A27' in (s.get('error') or ''), 'C5 存檔 → 400 密碼守衛（看開頁快照）：%s' % dump(s))
        get(ws)   # 把 state 套進去的 cbA27 換回檔案值
    else:
        print('  SKIP  C5 cbE30 或 cbA27 點不到')

    # C6
    if a.allow_save and (evs.get('cbE30') or {}).get('operable'):
        g = get(ws)
        w = widgets_from(g)
        e30 = bool(((g or {}).get('proxies') or {}).get('cbE30', {}).get('checked'))
        w['cbE30'] = {'checked': not e30}
        s = ws.cmd('editlist.save', 'IniConfig', json.dumps({'widgets': w, 'answers': {}}))
        check(bool(s and s.get('ok')) and 'cbE30' in (s.get('events') or []) and s.get('saved') is False,
              'C6 沒送 form.event 就改 E30 存檔（答 NO）→ ack.events 含 cbE30：%s' % dump(s))
        time.sleep(a.gap)
        r = ws.event(TAG, F, 'cbE30', 'click', checked=e30)
        check(bool(r) and not r.get('ok') and 'reload page' in (r.get('error') or ''), 'C6 存檔後沒重開頁就送事件 → reload page：%s' % dump(r))
    get(ws)
    ws.cmd('control.release')
    print('失敗 %d 項' % len(fp.FAILS))
    return len(fp.FAILS)


if __name__ == '__main__':
    sys.exit(main())
