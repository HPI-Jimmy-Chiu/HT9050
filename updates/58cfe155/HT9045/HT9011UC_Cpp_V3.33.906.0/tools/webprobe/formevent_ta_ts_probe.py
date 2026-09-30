# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/formevent_ta_ts_probe.py -- WS form.event 的 e2e 探針：Setup.TrayAssignment（TA-1～TA-4）＋ Setup.Temp_Set（TS-1）。
#
#  AI(W906-FRW-S158) 20260927 [W906]  NOT in golden。Steven ★ Q40＝A（RULINGS_20260926 S157）；Q41 盤點
#  D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md §3.4 TA-1～TA-4、§3.9 TS-1。
#  C++：FileRW/TrayForm.cpp（g_evreg、BeforeApply、TA_EvOnTab）＋TrayForm.gen.inc 檔尾 kTA_Events；
#       FileRW/Temperature.cpp（g_evreg、BeforeApply、SaveFlow (1)）＋Temperature.gen.inc 檔尾 kTS_Events。
#  共用的 WS 小工具借 tools/webprobe/formevent_probe.py（Ws／check／err_code／sha；失敗清單同一份）。
#  **寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見 .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗什麼（不用瀏覽器，直接走 WS；頁面送出點是 Jimmy 在 ht9045_wire_engine.js 加的，不在這裡驗）：
#    A  Setup.TrayAssignment（C 路 TrayForm，golden cTrayAssignment.cpp）
#       A0 editlist.get TrayForm → events 有 32 個控制項（13 張圖＋19 個選單／下拉）；13 張圖的 proxies 都帶 tag 0..8（＝方向圖號，
#          不是 golden DFM Tag 索引 —— 見 tools/editlist/TrayForm.py 的 _IMG 註解）
#          AI(W906-Q57-TRIAGE) 20260930：⛔ 更正 —— 33 個：＋cbEnableAMR_KYEC（f14484e1 B10b X-3，golden V912 cTrayAssignment.dfm
#          cbEnableAMR_KYEC OnClick＝cbEnableAMR_KYECClick、cTrayAssignment.cpp:1805）。見 TA_EVENTS。
#       A1 第一張「點得到」的圖連點 8 下（每下隔 --gap 秒，避開 WebCmdGuard 400 ms）：每下 changed.<圖>.tag＝golden imgLoaderClick
#          的下一號（dir+1，>=8 回 0）；8 下後（原本 0..7 時）回到原號
#       A2 RGLoader（tsNormalTestGroup 分頁）點 0 → ok、route C；之後 cbEmpty 停用（golden RGLoaderClick :1238-1249 FT 連動，要
#          TA_EvOnTab 把分頁暫設成 0 才會發生）。再點回原值。（圖像模式 bTrayAssignUseGraphic 時這頁的分頁看不見 → 跳過）
#       A3 rgFixTrayMode（點得到才驗）點另一項 → ok；「任一 labFix1..6 顯示（Fix 盤有 IC）」⇔「itemIndex 被改回」（golden :1182-1194）
#          AI(W906-Q57-TRIAGE) 20260930：「改回」＝changed.rgFixTrayMode.itemIndex 在、而且不是剛點的值。changed 的前快照在 RunPageEvent
#          第 5 步（先照 VCL 設好點的值）之後才拍（FileRW/_EditPage.cpp:534-570），沒改回＝rgFixTrayMode 不會出現在 changed。
#       A4 RGAuto2 點 2（Bin）→ ok；訊息有 "already has bin" ⇔ 某一組被改回 0（golden BinTrayDetect :1753-1765）。再點回原值。
#       A5 錯誤碼：unknown-control（noSuchWidget）、no-handler（edAuto1Type）、bad-payload（RGAuto1 itemIndex 超出）
#       A6（--allow-save 才跑，會寫 <recipe>\Tray.Data）存檔重播：重開頁 → editlist.save 送開頁值、但第一張可點的圖 tag＋3 →
#          ack.events 有那張圖、saved；重開頁 → tag＝新值。再存一次原值還原。
#    T  Setup.Temp_Set（C 路 Temperature，golden uTemp_Set.cpp）
#       T0 editlist.get Temperature → events 有 rgIndexHeatMode、chkTempCalByRecipe（operable 照機台設定）
#          AI(W906-Q57-TRIAGE) 20260930：⛔ 更正 —— 現在是 16 個（見 TS_EVENTS）：＋TS-7 基準點 6 個（70aa17e8）、＋TS-2 ATC 6 個（b08ae6ad）、
#          ＋sbtExit（ff497e5d B10a TS-10）、＋pgcTempOffset（f14484e1 B10b X-5）；都是 golden V912 uTemp_Set.dfm 的 OnClick／OnChange。
#       T1 存檔防線（不寫檔）：重開頁 → editlist.save 送開頁值、但 rgIndexHeatMode 換一項（沒送 form.event）→ saved=false、
#          訊息含 "without the click event"（FileRW/Temperature.cpp SaveFlow (1)）
#       T2 rgIndexHeatMode 點另一項 → ok、route C、golden "uTemp_Set.cpp:4232"（changed 有哪些 myTempPal<i>_* 印出來；兩個模式
#          用同一份 DefineTemp 表時 changed 可以是空的 —— 只有 HeadChamber 與其他模式分兩份表，:2900-2907）
#       T3 chkTempCalByRecipe（點得到才驗）點一下 → ok；再點回
#       T4（--allow-save 才跑，會寫 Temperature.Data／Tester.Data／DefineTemp\*.Data／Config\ATC.ini）T2 之後存檔 → 不被 SaveFlow (1)
#          擋（訊息不含 "without the click event"）。之後自己用 golden 頁面改回原模式。
#    最後兩頁各再 editlist.get 一次（golden FormShow：ReadTempFile(true)／ReadFile 把 T2／A1 改過的機台記憶體換回檔案值）。
#
#  ⚠ 機台記憶體：golden 點 rgIndexHeatMode／chkTempCalByRecipe 當下就把全域 Temperature.fTempOffSet[][] 換成另一份補償表
#    （bthermo.cpp:282 ConvertTempOffset 算加熱設定值讀它）。本探針結尾的 editlist.get 會換回檔案值；中途中斷就手動重開一次頁。
#  ⚠ 會碰檔案（照 golden，不是 form.event 自己寫）：editlist.get（FormShow）與 T2／T3 的 ReadTempFile(false) 會照 golden 補寫缺鍵、
#    [ATC] Chiller Temp、DefineTemp 變體檔；--recipe-dir 整個資料夾與 --watch 列的檔前後比 SHA256，有變就列出來（不自動還原）。
#  ⚠ 權限：兩頁的元件多數依 AccessLevel 停用（TrayForm 權限、Temp_Set 等級 17／57…）→ 帶 --user／--password。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\formevent_ta_ts_probe.py --port 8046 [--user U --password P] [--recipe-dir D:\HT9045\IniData\Data\<recipe>]
#             [--allow-save] [--gap 0.6] [--only A|T]
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
IMGS = ['imgLoader'] + ['imgAuto%d' % i for i in range(1, 7)] + ['imgFix%d' % i for i in range(1, 7)]
VALUE_KEYS = ('checked', 'itemIndex', 'text', 'position', 'dateTime', 'cells', 'tag')
# AI(W906-Q57-TRIAGE) 20260930: Q57 r3a／r3b 的 A0（回應 33 個）與 T0（回應 16 個）FAIL＝事件表之後長大了，這支探針沒跟上。
#   期望改成「剛好這些名字」（tools/editlist/TrayForm.py、Temperature.py 'events' 的控制項；每一個都對得到 golden V912 的 dfm 綁定）：
#   TrayAssignment：13 張圖＋19 個選單／下拉（4e74e8b4 TA-1～TA-4）＋cbEnableAMR_KYEC（f14484e1 B10b X-3，cTrayAssignment.cpp:1805）
TA_EVENTS = IMGS + ['rgLoaderType', 'cbLoader', 'RGLoader', 'rgLoad_RT', 'rgFixTrayMode', 'cbColor', 'cbEmpty'] + \
    ['RGAuto%d' % i for i in range(1, 7)] + ['rgAuto%d_RT' % i for i in range(1, 7)] + ['cbEnableAMR_KYEC']
#   Temp_Set：TS-1 rgIndexHeatMode（uTemp_Set.cpp:4232）／chkTempCalByRecipe（:6333）（4e74e8b4）；TS-7 rb1..6Point（rb1PointClick :421）
#   ／rgBasePoint（:6384）（70aa17e8）；TS-2 rbATCActiveOn（:5430）／rbATC70ActiveOn 與 golden 另外三個綁 rbATC70ActiveOnClick（:5407）的勾選框
#   ／cbATCReferTempSensor（:7073）（b08ae6ad）；TS-10 sbtExit（:5175，ff497e5d B10a）；X-5 pgcTempOffset（:5224 OnChange，f14484e1 B10b）
TS_EVENTS = ['rgIndexHeatMode', 'chkTempCalByRecipe', 'rb1Point', 'rb2Point', 'rb3Point', 'rb5Point', 'rb6Point', 'rgBasePoint',
             'rbATCActiveOn', 'rbATC70ActiveOn', 'cbEnableATCConsFailOffset', 'cbEnableATCQAModeOffset', 'cbATCTestTimeOffset',
             'cbATCReferTempSensor', 'sbtExit', 'pgcTempOffset']


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


def ev(ws, tag, form, control, event, **kw):
    return ws.event(tag, form, control, event, **kw)


def ok_c(r):
    return bool(r and r.get('ok')) and r.get('route') == 'C'


def trayform(ws, a):
    print('== A  Setup.TrayAssignment（C 路 TrayForm，golden cTrayAssignment.cpp）')
    g = ws.cmd('editlist.get', 'TrayForm')
    check(bool(g and g.get('ok')), 'A0 editlist.get TrayForm：%s' % ('' if g and g.get('ok') else json.dumps(g, ensure_ascii=False)[:240]))
    if not (g and g.get('ok')):
        return
    evs = g.get('events') or {}
    prox = g.get('proxies') or {}
    # AI(W906-Q57-TRIAGE) 20260930: 32 → 33（f14484e1 加 cbEnableAMR_KYEC），而且要剛好是 TA_EVENTS 這些名字
    check(sorted(evs) == sorted(TA_EVENTS), 'A0 events %d 個（13 張圖＋19 個選單／下拉＋cbEnableAMR_KYEC）：%d 個，多 %s／缺 %s' % (
        len(TA_EVENTS), len(evs), sorted(set(evs) - set(TA_EVENTS)), sorted(set(TA_EVENTS) - set(evs))))
    tags = {n: (prox.get(n) or {}).get('tag') for n in IMGS}
    check(all(isinstance(t, int) and 0 <= t <= 8 for t in tags.values()), 'A0 13 張圖的 tag＝方向圖號 0..8：%s' % tags)
    F = 'TfTrayAssignment'
    # A1
    img = next((n for n in IMGS if (evs.get(n) or {}).get('operable')), None)
    if img:
        cur, first = tags[img], tags[img]
        for k in range(8):
            r = ev(ws, 'Setup.TrayAssignment', F, img, 'click')
            want = 0 if cur + 1 >= 8 else cur + 1
            got = ((r or {}).get('changed') or {}).get(img, {}).get('tag', cur)
            check(ok_c(r) and got == want, 'A1 %s 第 %d 下：%d → %d（golden imgLoaderClick）：%s' % (img, k + 1, cur, want, json.dumps(r, ensure_ascii=False)[:200]))
            cur = got
            time.sleep(a.gap)
        if first <= 7:
            check(cur == first, 'A1 %s 點 8 下回到原號 %d：%d' % (img, first, cur))
    else:
        print('  SKIP  A1 沒有點得到的方向圖（權限或 rgLoaderType 讓容器看不見）')
    # A2
    if (evs.get('RGLoader') or {}).get('operable'):
        orig = (prox.get('RGLoader') or {}).get('itemIndex', 0)
        r = ev(ws, 'Setup.TrayAssignment', F, 'RGLoader', 'click', itemIndex=0)   # 不重開頁（FormShow 會把狀態蓋回去）
        ch = (r or {}).get('changed') or {}
        cbE = (ch.get('cbEmpty') or {}).get('enabled', (prox.get('cbEmpty') or {}).get('enabled'))   # 沒變＝本來就停用
        check(ok_c(r) and cbE is False, 'A2 RGLoader=0 → cbEmpty 停用（FT 連動，分頁暫設 tsNormalTestGroup）：%s' % json.dumps(r, ensure_ascii=False)[:300])
        time.sleep(a.gap)
        if orig != 0:
            ev(ws, 'Setup.TrayAssignment', F, 'RGLoader', 'click', itemIndex=orig)
            time.sleep(a.gap)
    else:
        print('  SKIP  A2 RGLoader 點不到（圖像模式或權限）')
    # A3
    if (evs.get('rgFixTrayMode') or {}).get('operable'):
        orig = (prox.get('rgFixTrayMode') or {}).get('itemIndex', 0)
        new = 1 - orig if orig in (0, 1) else 0
        r = ev(ws, 'Setup.TrayAssignment', F, 'rgFixTrayMode', 'click', itemIndex=new)
        ch = (r or {}).get('changed') or {}
        lab = any(((ch.get('labFix%d' % i) or {}).get('visible', (prox.get('labFix%d' % i) or {}).get('visible')) is True) for i in range(1, 7))
        # AI(W906-Q57-TRIAGE) 20260930: Q57 r3a／r3b FAIL「Fix 盤有 IC(False) ⇔ 改回(True)」是探針自己算錯：舊寫法把「changed 裡沒有
        #   rgFixTrayMode」(None) 當成改回。RunPageEvent 先照 VCL 把點的值設進替身、之後才拍 changed 的前快照（FileRW/_EditPage.cpp:534-570），
        #   所以 golden 走 ShowCompnet（沒 IC、不改回，cTrayAssignment.cpp:1189）時 rgFixTrayMode 不會出現在 changed；只有 golden
        #   :1193 `rgFixTrayMode->ItemIndex=TrayForm.iFixTrayMode` 把它改掉時才會出現，而且值不是剛點的 new。那一輪的回應（ckUseFix1..6
        #   visible＝true、labFix 都藏著）就是 golden「沒 IC → 保留新值」。
        got_ix = (ch.get('rgFixTrayMode') or {}).get('itemIndex', None)
        reverted = got_ix is not None and got_ix != new
        check(ok_c(r) and lab == reverted, 'A3 rgFixTrayMode：Fix 盤有 IC(%s) ⇔ 改回(%s)：%s' % (lab, reverted, json.dumps(r, ensure_ascii=False)[:300]))
        time.sleep(a.gap)
        if not reverted:
            ev(ws, 'Setup.TrayAssignment', F, 'rgFixTrayMode', 'click', itemIndex=orig)
            time.sleep(a.gap)
    else:
        print('  SKIP  A3 rgFixTrayMode 點不到（CosFunction.bUseTrayUpDownSet 關或 rgLoaderType=0）')
    # A4
    if (evs.get('RGAuto2') or {}).get('operable') and len((evs.get('RGAuto2') or {}).get('items') or []) >= 3:
        orig = (prox.get('RGAuto2') or {}).get('itemIndex', 0)
        r = ev(ws, 'Setup.TrayAssignment', F, 'RGAuto2', 'click', itemIndex=2)
        ch = (r or {}).get('changed') or {}
        bin_msg = 'already has bin' in msgs(r)
        back = any((ch.get(n) or {}).get('itemIndex') == 0 for n in ('RGLoader', 'RGAuto2', 'rgLoad_RT', 'rgAuto2_RT'))
        check(ok_c(r) and bin_msg == back, 'A4 RGAuto2=2：bin 訊息(%s) ⇔ 有一組改回 0(%s)：%s' % (bin_msg, back, json.dumps(r, ensure_ascii=False)[:300]))
        time.sleep(a.gap)
        ev(ws, 'Setup.TrayAssignment', F, 'RGAuto2', 'click', itemIndex=orig)
        time.sleep(a.gap)
    else:
        print('  SKIP  A4 RGAuto2 點不到或不到 3 項')
    # A5
    for name, code, kw in [('A5a', 'unknown-control', dict(control='noSuchWidget', event='click')),
                           ('A5b', 'no-handler', dict(control='edAuto1Type', event='change', text='x')),
                           ('A5c', 'bad-payload', dict(control='RGAuto1', event='click', itemIndex=99))]:
        c, e = kw.pop('control'), kw.pop('event')
        r = ev(ws, 'Setup.TrayAssignment', F, c, e, **kw)
        check(bool(r) and not r.get('ok') and err_code(r) == code, '%s → %s：%s' % (name, code, json.dumps(r, ensure_ascii=False)[:200]))
        time.sleep(a.gap)
    # A6
    if a.allow_save and img:
        g = ws.cmd('editlist.get', 'TrayForm')
        w = widgets_from(g)
        old = (w.get(img) or {}).get('tag', 0)
        w[img] = {'tag': (old + 3) % 8}
        s = save(ws, 'TrayForm', w)
        check(bool(s and s.get('ok')) and s.get('saved') and img in (s.get('events') or []),
              'A6 存檔重播 %s tag %d→%d：%s' % (img, old, (old + 3) % 8, json.dumps(s, ensure_ascii=False)[:300]))
        g = ws.cmd('editlist.get', 'TrayForm')
        check(((g or {}).get('proxies') or {}).get(img, {}).get('tag') == (old + 3) % 8, 'A6 重開頁 %s tag＝新值' % img)
        w = widgets_from(g)
        w[img] = {'tag': old}
        s = save(ws, 'TrayForm', w)
        check(bool(s and s.get('ok')) and s.get('saved'), 'A6 還原 %s tag＝%d' % (img, old))
    ws.cmd('editlist.get', 'TrayForm')   # golden FormShow：iTrayDirect／替身換回檔案值


def temperature(ws, a):
    print('== T  Setup.Temp_Set（C 路 Temperature，golden uTemp_Set.cpp）')
    F = 'TfTemp_Set'
    g = ws.cmd('editlist.get', 'Temperature')
    check(bool(g and g.get('ok')), 'T0 editlist.get Temperature：%s' % ('' if g and g.get('ok') else json.dumps(g, ensure_ascii=False)[:240]))
    if not (g and g.get('ok')):
        return
    evs = g.get('events') or {}
    # AI(W906-Q57-TRIAGE) 20260930: 2 → 16（70aa17e8 TS-7、b08ae6ad TS-2、ff497e5d TS-10、f14484e1 X-5），見 TS_EVENTS
    check(sorted(evs) == sorted(TS_EVENTS), 'T0 events %d 個：%d 個，多 %s／缺 %s' % (
        len(TS_EVENTS), len(evs), sorted(set(evs) - set(TS_EVENTS)), sorted(set(TS_EVENTS) - set(evs))))
    prox = g.get('proxies') or {}
    rg = evs.get('rgIndexHeatMode') or {}
    orig = (prox.get('rgIndexHeatMode') or {}).get('itemIndex', 0)
    n = len(rg.get('items') or [])
    other = (2 if orig != 2 else 0) if n > 2 else (1 - orig if n == 2 else None)   # 優先換 HeadChamber↔其他（兩份表）
    if rg.get('operable') and other is not None:
        # T1（不寫檔）
        w = widgets_from(g)
        w['rgIndexHeatMode'] = {'itemIndex': other}
        s = save(ws, 'Temperature', w)
        check(bool(s and s.get('ok')) and s.get('saved') is False and 'without the click event' in msgs(s),
              'T1 沒送 form.event 就換模式存檔 → 拒存（SaveFlow (1)）：%s' % json.dumps(s, ensure_ascii=False)[:300])
        g = ws.cmd('editlist.get', 'Temperature')
        # T2
        r = ev(ws, 'Setup.Temp_Set', F, 'rgIndexHeatMode', 'click', itemIndex=other)
        check(ok_c(r) and 'uTemp_Set.cpp:4232' in (r or {}).get('golden', ''), 'T2 rgIndexHeatMode=%d → ok：changed %d 個 %s' % (
            other, len((r or {}).get('changed') or {}), sorted(((r or {}).get('changed') or {}).keys())[:12]))
        time.sleep(a.gap)
        if a.allow_save and ok_c(r):
            w = widgets_from(g)
            for k, v in ((r.get('changed') or {}).items()):
                for kk in VALUE_KEYS:
                    if kk in v:
                        w.setdefault(k, {})[kk] = v[kk]
            w['rgIndexHeatMode'] = {'itemIndex': other}
            s = save(ws, 'Temperature', w)
            check(bool(s and s.get('ok')) and 'without the click event' not in msgs(s),
                  'T4 送過 form.event 再存 → 不被 SaveFlow (1) 擋：%s' % json.dumps(s, ensure_ascii=False)[:300])
    else:
        print('  SKIP  T1／T2／T4 rgIndexHeatMode 點不到（LastSet.iTemperature=Hot、等級 57、ATC 機種）或只有一項')
    ck = evs.get('chkTempCalByRecipe') or {}
    if ck.get('operable'):
        c0 = bool((prox.get('chkTempCalByRecipe') or {}).get('checked'))
        r = ev(ws, 'Setup.Temp_Set', F, 'chkTempCalByRecipe', 'click', checked=not c0)
        check(ok_c(r) and 'uTemp_Set.cpp:6333' in (r or {}).get('golden', ''), 'T3 chkTempCalByRecipe=%s → ok：changed %d 個' % (not c0, len((r or {}).get('changed') or {})))
        time.sleep(a.gap)
        ev(ws, 'Setup.Temp_Set', F, 'chkTempCalByRecipe', 'click', checked=c0)
        time.sleep(a.gap)
    else:
        print('  SKIP  T3 chkTempCalByRecipe 點不到（CosFunction.bTempCalByRecipe 關）')
    ws.cmd('editlist.get', 'Temperature')   # golden FormShow：ReadTempFile(true) 把 T2／T3 換掉的補償表換回檔案值


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--recipe-dir')
    ap.add_argument('--watch', action='append', default=[])
    ap.add_argument('--allow-save', action='store_true', help='A6／T4：真的跑 golden 存檔（會寫配方檔）')
    ap.add_argument('--gap', type=float, default=0.6, help='同一包事件之間的間隔（WebCmdGuard 400 ms）')
    ap.add_argument('--only', choices=('A', 'T'))
    a = ap.parse_args()

    watched = list(a.watch)
    if a.recipe_dir and os.path.isdir(a.recipe_dir):
        watched += [os.path.join(a.recipe_dir, f) for f in sorted(os.listdir(a.recipe_dir))
                    if os.path.isfile(os.path.join(a.recipe_dir, f))]
    before = {p: sha(p) for p in watched}
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
    if a.only in (None, 'A'):
        trayform(ws, a)
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
