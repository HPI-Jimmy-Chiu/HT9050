# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/q41_speed_ts7_probe.py -- Q41 Setup.Speed（SP-1～SP-5）＋ Setup.Temp_Set（TS-7）的 C++ 半邊 e2e 探針。
#
#  AI(W906-FRW-S158) 20260927 [W906]  NOT in golden。Q41 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md
#  §3.5 SP-1～SP-5、§3.9 TS-7；St02 FROM_STEVEN §4 20260927 17:21 (a)(b)。
#  C++：FileRW/ArmSpeed_File.cpp（BeforeApply 滑桿重播、Reload、g_evreg）＋ArmSpeed_File.gen.inc 檔尾 kSP_Events；
#       FileRW/Temperature.cpp（檔尾 BasePointReplay）＋Temperature.gen.inc（rb1PointClick／rgBasePointClick、DoIniDataToForm 的
#       VCL 隱含 OnClick、檔尾 kTS_Events）。共用的 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code／sha）。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見 .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗什麼（直接走 WS；頁面送出點是 Jimmy 在 ht9045_wire_engine.js 加的，不在這裡驗）：
#    S  Setup.Speed（C 路 ArmSpeed_File，golden cSpeed.cpp）
#       S0 editlist.get ArmSpeed_File → events 13 個（9 個軸勾選框＋spbSelectAll／spbSetToDef／spbSpeedAdd／spbSpeedDec），
#          proxies 有 tbAllSpeed／tbAccSpeed 的 position
#       S1 第一個點得到的軸勾選框點一下（反轉）→ ok、route C、golden "cSpeed.cpp:1426"；之後 tbAllSpeed／spbSpeedAdd／spbSpeedDec
#          enabled（changed 有 enabled:true，或本來就開著）。再點回原值。
#       S2 spbSpeedAdd（state 帶 tbAllSpeed position）→ changed 的 edAllSpeed.text ＝ 新 position（golden tbAllSpeedChange :1284）；
#          spbSpeedDec 同。spbSelectAll → 九個勾選框（看得見的）checked。
#       S3 錯誤碼：tbAllSpeed change → no-handler（滑桿不在事件表：form.event 帶不了 position）
#       S4（--allow-save 才跑，會寫 <recipe>\ArmCondition.Data、跑關窗尾段）存檔重播：重開頁 → editlist.save 送開頁值、但
#          tbAllSpeed／tbAccSpeed 各 -10（不小於 1）→ saved、ack.events 有 tbAllSpeed／tbAccSpeed；重開頁 → edAllSpeed／edAllAccSpeed
#          ＝新 position。再存一次原值還原。
#    T  Setup.Temp_Set（C 路 Temperature，golden uTemp_Set.cpp）
#       T0 editlist.get Temperature → events 有 rb1Point..rb6Point（5 顆）＋rgBasePoint＋TS-1 兩個
#       T1 點另一顆基準點數（優先 1→3 或 3→1）→ ok、golden "uTemp_Set.cpp:421"；changed 裡原本那顆 checked=false（TurnSiblingsOff）；
#          有 myTempPal<i>_edBase 的 visible 變了（1 Points 藏 Base，3 Points 顯示）。再點回原值。
#       T2 rgBasePoint（點得到才驗，CosFunction.bTemp5PointKitOffset）換一項 → ok、golden "uTemp_Set.cpp:6384"；再換回。
#       T3 錯誤碼：btnSort click → no-handler（沒轉，見 tools/editlist/Temperature.py 'events' 註解）
#       T4 存檔重播（**不寫檔**）：重開頁 → editlist.save 送開頁值、但換基準點數＋改一格「換之後才看得見」的 edBase、
#          同時 rgIndexHeatMode 換一項（沒送 form.event ⇒ FileRW/Temperature.cpp SaveFlow (1) 拒存，什麼都不寫）→
#          saved=false、ack.events 有 5 顆基準點鈕、那一格不在 ack.ignored（＝重播後照「新點數」判可改）。rgIndexHeatMode 點不到就跳過。
#    最後兩頁各再 editlist.get 一次（golden FormShow：ReadFile／ReadTempFile(true) 把替身換回檔案值）。
#
#  ⚠ 會碰檔案（照 golden，不是 form.event 自己寫）：editlist.get（FormShow）與 T4 的 Reload 跑 ReadTempFile(true)，會照 golden 補寫缺鍵、
#    [ATC] Chiller Temp、DefineTemp 變體檔；--recipe-dir 整個資料夾與 --watch 列的檔前後比 SHA256，有變就列出來（不自動還原）。
#  ⚠ 權限：兩頁依 AccessLevel 停用（Speed 等級 3、Temp_Set 等級 52 管 gbBasePoint）→ 帶 --user／--password。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\q41_speed_ts7_probe.py --port 8046 [--user U --password P] [--recipe-dir D:\HT9045\IniData\Data\<recipe>]
#             [--allow-save] [--gap 0.6] [--only S|T]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import formevent_probe as fp   # noqa: E402  Ws／check／err_code／sha／FAILS

check, err_code, sha = fp.check, fp.err_code, fp.sha
VALUE_KEYS = ('checked', 'itemIndex', 'text', 'position', 'dateTime', 'cells', 'tag')
AXES = ['cbIndexArm', 'cbInArm', 'cbOutArm', 'cbShuttle', 'cbTrayArm', 'cbInArmZ', 'cbOutArmZ', 'cbInRotate', 'cbOutRotate']
RBS = ['rb1Point', 'rb2Point', 'rb3Point', 'rb5Point', 'rb6Point']


def widgets_from(get_ack):
    """editlist.get 的 proxies → editlist.save 的 widgets（同頁面引擎 gbSave：每個有值的替身都送目前值）"""
    out = {}
    for name, p in ((get_ack or {}).get('proxies') or {}).items():
        v = {k: p[k] for k in VALUE_KEYS if k in p}
        if 'itemIndex' in v and 'text' in p:
            v['text'] = p['text']
        if v:
            out[name] = v
    return out


def save(ws, tag, widgets):
    return ws.cmd('editlist.save', tag, json.dumps({'widgets': widgets, 'answers': {}}))


def msgs(ack):
    return json.dumps((ack or {}).get('messages') or ((ack or {}).get('session') or {}).get('messages') or [], ensure_ascii=False)


def ok_c(r):
    return bool(r and r.get('ok')) and r.get('route') == 'C'


def short(r, n=240):
    return json.dumps(r, ensure_ascii=False)[:n]


def speed(ws, a):
    print('== S  Setup.Speed（C 路 ArmSpeed_File，golden cSpeed.cpp）')
    F = 'TfSpeed'
    g = ws.cmd('editlist.get', 'ArmSpeed_File')
    check(bool(g and g.get('ok')), 'S0 editlist.get ArmSpeed_File：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    want = set(AXES) | {'spbSelectAll', 'spbSetToDef', 'spbSpeedAdd', 'spbSpeedDec'}
    check(set(evs) == want, 'S0 events 13 個：%s' % sorted(evs))
    p0 = (prox.get('tbAllSpeed') or {}).get('position')
    check(isinstance(p0, int), 'S0 tbAllSpeed position：%r' % p0)
    # S1
    ax = next((n for n in AXES if (evs.get(n) or {}).get('operable')), None)
    if ax:
        c0 = bool((prox.get(ax) or {}).get('checked'))
        r = ws.event('Setup.Speed', F, ax, 'click', checked=not c0)
        ch = (r or {}).get('changed') or {}
        en = (ch.get('tbAllSpeed') or {}).get('enabled', (prox.get('tbAllSpeed') or {}).get('enabled'))
        check(ok_c(r) and 'cSpeed.cpp:1426' in (r or {}).get('golden', '') and en is True,
              'S1 %s 點一下 → tbAllSpeed 打開（golden cbIndexArmClick）：%s' % (ax, short(r, 300)))
        time.sleep(a.gap)
        ws.event('Setup.Speed', F, ax, 'click', checked=c0)
        time.sleep(a.gap)
    else:
        print('  SKIP  S1 九個軸勾選框都點不到（等級 3 不夠或權限）')
    # S2
    if ax and isinstance(p0, int):
        for btn, sign in (('spbSpeedDec', -1), ('spbSpeedAdd', +1)):
            r = ws.event('Setup.Speed', F, btn, 'click', state={'tbAllSpeed': {'position': p0}})
            ch = (r or {}).get('changed') or {}
            pos = (ch.get('tbAllSpeed') or {}).get('position', p0)
            txt = (ch.get('edAllSpeed') or {}).get('text', (prox.get('edAllSpeed') or {}).get('text'))
            check(ok_c(r) and str(pos) == str(txt), 'S2 %s（position %d → %s）edAllSpeed＝%s：%s' % (btn, p0, pos, txt, short(r, 300)))
            time.sleep(a.gap)
        r = ws.event('Setup.Speed', F, 'spbSelectAll', 'click')
        ch = (r or {}).get('changed') or {}
        vis = [n for n in AXES if (prox.get(n) or {}).get('visible', True)]
        allc = all((ch.get(n) or {}).get('checked', (prox.get(n) or {}).get('checked')) is True for n in vis)
        check(ok_c(r) and allc, 'S2 spbSelectAll → 看得見的軸勾選框全勾：%s' % short(r, 300))
        time.sleep(a.gap)
    # S3
    r = ws.event('Setup.Speed', F, 'tbAllSpeed', 'change')
    check(bool(r) and not r.get('ok') and err_code(r) == 'no-handler', 'S3 tbAllSpeed change → no-handler：%s' % short(r, 200))
    time.sleep(a.gap)
    # S4
    if a.allow_save and isinstance(p0, int):
        g = ws.cmd('editlist.get', 'ArmSpeed_File')
        w = widgets_from(g)
        pa = (w.get('tbAccSpeed') or {}).get('position', 1)
        n_all, n_acc = max(1, p0 - 10), max(1, pa - 10)
        w['tbAllSpeed'] = {'position': n_all}
        w['tbAccSpeed'] = {'position': n_acc}
        s = save(ws, 'ArmSpeed_File', w)
        ev = (s or {}).get('events') or []
        check(bool(s and s.get('ok')) and s.get('saved') and (n_all == p0 or 'tbAllSpeed' in ev or 'tbAllSpeed' in (s.get('ignored') or [])),
              'S4 存檔重播 tbAllSpeed %d→%d、tbAccSpeed %d→%d：%s' % (p0, n_all, pa, n_acc, short(s, 300)))
        g = ws.cmd('editlist.get', 'ArmSpeed_File')
        pr = (g or {}).get('proxies') or {}
        if 'tbAllSpeed' in ev:
            check((pr.get('edAllSpeed') or {}).get('text') == str(n_all), 'S4 重開頁 edAllSpeed＝%d：%s' % (n_all, pr.get('edAllSpeed')))
        check((pr.get('edAllAccSpeed') or {}).get('text') == str(n_acc), 'S4 重開頁 edAllAccSpeed＝%d：%s' % (n_acc, pr.get('edAllAccSpeed')))
        w = widgets_from(g)
        w['tbAllSpeed'] = {'position': p0}
        w['tbAccSpeed'] = {'position': pa}
        s = save(ws, 'ArmSpeed_File', w)
        check(bool(s and s.get('ok')) and s.get('saved'), 'S4 還原 tbAllSpeed＝%d、tbAccSpeed＝%d' % (p0, pa))
    ws.cmd('editlist.get', 'ArmSpeed_File')   # golden FormShow：ReadFile＋DoIniDataToForm 把替身換回檔案值


def temperature(ws, a):
    print('== T  Setup.Temp_Set（C 路 Temperature，golden uTemp_Set.cpp）')
    F = 'TfTemp_Set'
    g = ws.cmd('editlist.get', 'Temperature')
    check(bool(g and g.get('ok')), 'T0 editlist.get Temperature：%s' % ('' if g and g.get('ok') else short(g)))
    if not (g and g.get('ok')):
        return
    evs, prox = g.get('events') or {}, g.get('proxies') or {}
    check(set(RBS + ['rgBasePoint', 'rgIndexHeatMode', 'chkTempCalByRecipe']) == set(evs), 'T0 events：%s' % sorted(evs))
    cur = next((n for n in RBS if (prox.get(n) or {}).get('checked')), None)
    pref = {'rb1Point': 'rb3Point', 'rb3Point': 'rb1Point'}.get(cur)
    other = pref if pref and (evs.get(pref) or {}).get('operable') else next(
        (n for n in RBS if n != cur and (evs.get(n) or {}).get('operable')), None)
    # T1
    became = []   # 1→3 Points 時「換之後才看得見」的 edBase（T4 用）
    if cur and other:
        r = ws.event('Setup.Temp_Set', F, other, 'click', checked=True)
        ch = (r or {}).get('changed') or {}
        sib = (ch.get(cur) or {}).get('checked') is False
        base = [n for n, v in ch.items() if n.startswith('myTempPal') and n.endswith('_edBase') and 'visible' in v]
        became = [n for n in base if (ch.get(n) or {}).get('visible') is True and (ch.get(n) or {}).get('editable') is True]
        check(ok_c(r) and 'uTemp_Set.cpp:421' in (r or {}).get('golden', '') and sib,
              'T1 %s → %s：原本那顆取消勾選(%s)，edBase visible 變了 %d 格：%s' % (cur, other, sib, len(base), short(r, 300)))
        time.sleep(a.gap)
        ws.event('Setup.Temp_Set', F, cur, 'click', checked=True)
        time.sleep(a.gap)
    else:
        print('  SKIP  T1 基準點數點不到（等級 52 不夠）或只有一顆看得見')
    # T2
    rgb = evs.get('rgBasePoint') or {}
    if rgb.get('operable') and len(rgb.get('items') or []) >= 2:
        i0 = (prox.get('rgBasePoint') or {}).get('itemIndex', 0)
        r = ws.event('Setup.Temp_Set', F, 'rgBasePoint', 'click', itemIndex=1 - i0 if i0 in (0, 1) else 0)
        check(ok_c(r) and 'uTemp_Set.cpp:6384' in (r or {}).get('golden', ''), 'T2 rgBasePoint 換一項 → ok：changed %d 個' % len((r or {}).get('changed') or {}))
        time.sleep(a.gap)
        ws.event('Setup.Temp_Set', F, 'rgBasePoint', 'click', itemIndex=i0)
        time.sleep(a.gap)
    else:
        print('  SKIP  T2 rgBasePoint 點不到（CosFunction.bTemp5PointKitOffset 關）')
    # T3
    r = ws.event('Setup.Temp_Set', F, 'btnSort', 'click')
    check(bool(r) and not r.get('ok') and err_code(r) in ('no-handler', 'unknown-control'), 'T3 btnSort click → no-handler：%s' % short(r, 200))
    time.sleep(a.gap)
    # T4（不寫檔：同時沒送 form.event 就換 rgIndexHeatMode ⇒ SaveFlow (1) 拒存）
    g = ws.cmd('editlist.get', 'Temperature')
    evs, prox = (g or {}).get('events') or {}, (g or {}).get('proxies') or {}
    rg = evs.get('rgIndexHeatMode') or {}
    hm = (prox.get('rgIndexHeatMode') or {}).get('itemIndex', 0)
    n = len(rg.get('items') or [])
    hm2 = (hm + 1) % n if n >= 2 else None
    if cur == 'rb1Point' and other == 'rb3Point' and rg.get('operable') and hm2 is not None:
        hidden = became[0] if became else None
        w = widgets_from(g)
        for rb in RBS:
            w[rb] = {'checked': rb == other}
        if hidden:
            w[hidden] = {'text': w.get(hidden, {}).get('text', '0')}
        w['rgIndexHeatMode'] = {'itemIndex': hm2}
        s = save(ws, 'Temperature', w)
        ev = (s or {}).get('events') or []
        check(bool(s and s.get('ok')) and s.get('saved') is False and 'without the click event' in msgs(s) and all(rb in ev for rb in RBS)
              and (not hidden or hidden not in ((s or {}).get('ignored') or [])),
              'T4 換基準點數 %s→%s（沒送 form.event）存檔 → 重播（events 有 5 顆）、%s 不在 ignored；同時換加熱模式 → 拒存不寫檔：%s'
              % (cur, other, hidden, short(s, 400)))
    else:
        print('  SKIP  T4 要「目前 1 Points、3 Points 點得到、rgIndexHeatMode 點得到且至少兩項」才能不寫檔地驗（T1 那一格：%s）' % (became[:1],))
    ws.cmd('editlist.get', 'Temperature')   # golden FormShow：ReadTempFile(true) 把替身與看得見換回檔案值


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--recipe-dir')
    ap.add_argument('--watch', action='append', default=[])
    ap.add_argument('--allow-save', action='store_true', help='S4：真的跑 golden Speed 存檔鈕（會寫 ArmCondition.Data、跑關窗尾段）')
    ap.add_argument('--gap', type=float, default=0.6, help='同一包事件之間的間隔（WebCmdGuard 400 ms）')
    ap.add_argument('--only', choices=('S', 'T'))
    a = ap.parse_args()

    watched = list(a.watch)
    if a.recipe_dir and os.path.isdir(a.recipe_dir):
        watched += [os.path.join(a.recipe_dir, f) for f in sorted(os.listdir(a.recipe_dir))
                    if os.path.isfile(os.path.join(a.recipe_dir, f))]
    before = {p: sha(p) for p in watched}
    ws = fp.Ws(a.port)
    r = ws.cmd('control.takeover')  # AI(W906-SCREEN-TOKEN) 20261001: takeover, not acquire -- an HMI screen now holds the token while it is connected (RULINGS_20261001, Jimmy 1001 14:3x)
    check(bool(r and r.get('ok')), 'control.takeover（form.event／editlist.save 要權杖）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)
    if a.only in (None, 'S'):
        speed(ws, a)
    if a.only in (None, 'T'):
        temperature(ws, a)
    ws.cmd('control.release')
    after = {p: sha(p) for p in watched}
    moved = [p for p in watched if before[p] != after[p]]
    print('== 檔案（SHA256 前後）：%d 個有變%s' % (len(moved), '' if not moved else '（form.event 不存檔；這是 golden 讀檔的補寫或 --allow-save 的存檔）'))
    for p in moved:
        print('     ' + p)
    print('失敗 %d 項' % len(fp.FAILS))
    return len(fp.FAILS)


if __name__ == '__main__':
    sys.exit(main())
