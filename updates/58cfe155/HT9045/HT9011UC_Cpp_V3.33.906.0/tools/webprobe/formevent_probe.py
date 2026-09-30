# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/formevent_probe.py -- WS form.event（golden 表單控制項事件）的 e2e 探針。
#
#  AI(W906-FRW-S157) 20260927 [W906]  NOT in golden。Steven ★ Q40＝A（RULINGS_20260926 S157）；
#  格式 FROM_STEVEN 20260927 10:15；C++ 本體 FileRW/_FormEvent.cpp、JsonBridge/FormBridge.cpp RunEvent、
#  FileRW/_EditPage.cpp RunPageEvent。**寫好、還沒跑過**（Steven01 這台不跑 wb_serve，見
#  .claude/skills/ht9045-html-json/references/wbserve-conventions.md §6）。
#
#  驗三頁（不用瀏覽器，直接走 WS；頁面送出點是 Jimmy 在 ht9045_wire_engine.js 加的，不在這裡驗）：
#    A  Setup.HotPlate（A 形狀，golden cHotPlate.cpp:412 cbSelectHPFromDBChange）
#       A1 選第 0 項（表頭 "Package Type"）→ ok、changed 空（golden :426 index<1 return）
#       A2 選第 --hp-index 項 → ok、changed 恰好 7 個欄位＝PlateForm.csv 那一列 Cells[0..6].Trim()
#       A3 同一包立刻再送 → "busy:"（WebCmdGuard；W906_CMDGUARD_MS=0 時跳過）
#       A4 錯誤碼：unknown-page、unknown-control、no-handler、bad-payload（itemIndex 超出、文字對不上、form 不符、value 不是 JSON）
#    T  Setup.TrayForm（C 路 UserDefForm_File，golden cTrayForm.cpp:570 cbTrayType1Change → :580 ShowTypePage）
#       T1 editlist.get UserDefForm_File → 回應有 events.cbTrayType1..3（items＝TrayForm.csv 非空名稱列）
#          ＋btnBinBoxReset（AI(W906-Q57-TRIAGE) 20260930：b08ae6ad B3 lane 2 TF-4，golden V912 cTrayForm.cpp:729 btnBinBoxResetClick）
#       T2 選 cbTrayType<--tray-type+1> 第 --tray-index 項 → ok；開頁值疊上 changed ＝ golden ShowTypePage 填的 10 格
#    C  Setup.Cleaning（C 路 TestIF_File_Cleaning，golden uCleaning.cpp:2322 cbbSelectTrayChange）
#       C1 editlist.get → events.cbbSelectTray.operable == false（golden DFM Enabled=False，uCleaning.dfm:906，沒人打開）
#       C2 form.event → bad-payload（照 golden：使用者點不到）
#
#  ⚠ 會碰檔案（照 golden，不是 form.event 自己寫）：A 的每一次 form.event 先跑 golden FormShow（ReadFile 可能補寫
#    <recipe>\HotPlate.Data 的 bTrayHotplateCheck，SetArmHotPlateYPitch）；C 的 editlist.get 跑 golden FormShow →
#    LoadAutoCleanData（會寫 HandlerCondition.Data，見 FileRW/TestIF_File_Cleaning.cpp 檔頭）。
#    --watch 列的檔（預設 D:\HT9045\System\PlateForm.csv、TrayForm.csv；--recipe-dir 整個資料夾）前後比 SHA256，
#    有變就列出來（不自動還原；跑之前自己備份）。
#  ⚠ 權限：HotPlate 的 GroupBox2 Enabled＝fSecurity->Insufficient(15)（HotPlateForm_File.cpp FormShow），等級不夠時 A2 會回
#    bad-payload（golden 點不到）→ 帶 --user／--password（測試密碼簿，wb_serve 以 W906_PWBOOK_PATH 啟動）。
#
#  用法：
#      build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\formevent_probe.py --port 8046 [--user S12TEST --password S12PW]
#             [--hp-index 3] [--tray-type 0] [--tray-index 1] [--recipe-dir D:\HT9045\IniData\Data\<recipe>]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import hashlib
import json
import os
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cmd_probe import ws_handshake, send_text, read_frames   # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PLATE_CSV = r'D:\HT9045\System\PlateForm.csv'
TRAY_CSV = r'D:\HT9045\System\TrayForm.csv'


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def golden_grid(path):
    """golden sbtReloadTrayClick／sbtReloadHPClick（FileRW/CfgTrayPlate.cpp）：每列拿掉 "，逗號前的每一段是一格
    （最後一個逗號後面那段不算）。回 cells[row][col]（未 Trim）。"""
    rows = []
    for ln in open(path, 'rb').read().decode('cp950', 'replace').splitlines():
        s1, cells = ln.replace('"', ''), []
        while ',' in s1:
            i = s1.index(',')
            cells.append(s1[:i])
            s1 = s1[i + 1:]
        rows.append(cells)
    return rows


def cell(rows, r, c):
    return rows[r][c] if r < len(rows) and c < len(rows[r]) else ''


def golden_items(rows):
    """golden FormShow：Cells[0][i] 不是 "" 也不是 " " 的列依序加進下拉（ItemIndex 直接當列號用 —— golden 怪處，照留）"""
    return [row[0] if row else '' for row in rows if (row[0] if row else '') not in ('', ' ')]


def sha(path):
    return hashlib.sha256(open(path, 'rb').read()).hexdigest() if os.path.isfile(path) else None


def watch_list(a):
    out = [PLATE_CSV, TRAY_CSV]
    if a.recipe_dir and os.path.isdir(a.recipe_dir):
        out += [os.path.join(a.recipe_dir, f) for f in sorted(os.listdir(a.recipe_dir))
                if os.path.isfile(os.path.join(a.recipe_dir, f))]
    return out


class Ws(object):
    def __init__(self, port):
        self.sock, left = ws_handshake('127.0.0.1', port, '/ht9045', time.monotonic() + 5)
        self.frames = read_frames(self.sock, left, time.monotonic() + 600)
        self.n = 100

    def cmd(self, name, tag=None, value=None):
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

    def event(self, tag, form, control, event='change', **kw):
        v = {'form': form, 'control': control, 'event': event, 'itemIndex': None, 'text': None, 'checked': None}
        v.update(kw)
        return self.cmd('form.event', tag, json.dumps(v))


def err_code(ack):
    e = (ack or {}).get('error') or ''
    return e.split(':', 1)[0] if ':' in e else e


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--hp-index', type=int, default=3)
    ap.add_argument('--tray-type', type=int, default=0, choices=(0, 1, 2))
    ap.add_argument('--tray-index', type=int, default=1)
    ap.add_argument('--recipe-dir')
    ap.add_argument('--skip-busy', action='store_true', help='wb_serve 以 W906_CMDGUARD_MS=0 啟動時用')
    a = ap.parse_args()

    watched = watch_list(a)
    before = {p: sha(p) for p in watched}
    ws = Ws(a.port)
    r = ws.cmd('control.acquire')
    check(bool(r and r.get('ok')), 'control.acquire（form.event 要權杖，同 form.save）')
    if a.user:
        r = ws.cmd('auth.login', a.user, a.password or '')
        if r and not r.get('ok') and 'already logged in' in (r.get('error') or ''):
            ws.cmd('auth.logout')
            time.sleep(0.5)   # WebCmdGuard: a second auth.login within 400 ms of the first one is refused as busy (Q50 20260928)
            r = ws.cmd('auth.login', a.user, a.password or '')
        check(bool(r and r.get('ok')), 'auth.login %s' % a.user)

    # ---------------- A：Setup.HotPlate ----------------
    print('== A  Setup.HotPlate（golden cHotPlate.cpp:412 cbSelectHPFromDBChange）')
    plate = golden_grid(PLATE_CSV)
    form = json.loads(urllib.request.urlopen('http://127.0.0.1:%d/api/form/Setup.HotPlate.html' % a.port, timeout=10)
                      .read().decode('utf-8'))
    items = ((form.get('widgets') or {}).get('cbSelectHPFromDB') or {}).get('items') or []
    check(items == golden_items(plate), 'A0 /api/form 的 cbSelectHPFromDB 清單＝PlateForm.csv 非空名稱列（%d 項）' % len(items))
    if len(items) > max(a.hp_index, 0):
        r = ws.event('Setup.HotPlate', 'TfHotPlate', 'cbSelectHPFromDB', itemIndex=0, text=items[0])
        check(bool(r and r.get('ok')) and r.get('changed') == {}, 'A1 第 0 項（表頭）→ ok、changed 空：%s' % json.dumps(r, ensure_ascii=False)[:300])
        r = ws.event('Setup.HotPlate', 'TfHotPlate', 'cbSelectHPFromDB', itemIndex=a.hp_index, text=items[a.hp_index])
        want = {n: {'text': cell(plate, a.hp_index, c).strip()} for c, n in
                enumerate(['HotPlateName', 'XST1', 'YST1', 'XPitch1', 'YPitch1', 'XCT1', 'YCT1'])}
        ok = bool(r and r.get('ok'))
        check(ok and r.get('route') == 'A' and r.get('changed') == want,
              'A2 第 %d 項 → changed＝%s；實際 %s' % (a.hp_index, json.dumps(want, ensure_ascii=False),
                                                  json.dumps(r, ensure_ascii=False)[:600]))
        if not a.skip_busy:
            r2 = ws.event('Setup.HotPlate', 'TfHotPlate', 'cbSelectHPFromDB', itemIndex=a.hp_index, text=items[a.hp_index])
            check(bool(r2) and not r2.get('ok') and (r2.get('error') or '').startswith('busy:'),
                  'A3 同一包 400 ms 內再送 → busy:（WebCmdGuard）：%s' % json.dumps(r2, ensure_ascii=False)[:200])
            time.sleep(0.6)
        cases = [
            ('A4a', 'unknown-page', dict(tag='Setup.NoSuchPage', control='cbSelectHPFromDB', itemIndex=1)),
            ('A4b', 'unknown-control', dict(tag='Setup.HotPlate', control='noSuchWidget', itemIndex=1)),
            ('A4c', 'no-handler', dict(tag='Setup.HotPlate', control='XST1', itemIndex=None, text='1')),
            ('A4d', 'bad-payload', dict(tag='Setup.HotPlate', control='cbSelectHPFromDB', itemIndex=len(items) + 5)),
            ('A4e', 'bad-payload', dict(tag='Setup.HotPlate', control='cbSelectHPFromDB', itemIndex=a.hp_index, text='NOT-IN-LIST')),
            ('A4f', 'bad-payload', dict(tag='Setup.HotPlate', control='cbSelectHPFromDB', itemIndex=1, form='TfWrong')),
        ]
        for name, code, kw in cases:
            tag = kw.pop('tag')
            f = kw.pop('form', 'TfHotPlate')
            r = ws.event(tag, f, kw.pop('control'), **kw)
            check(bool(r) and not r.get('ok') and err_code(r) == code, '%s → %s：%s' % (name, code, json.dumps(r, ensure_ascii=False)[:240]))
        r = ws.cmd('form.event', 'Setup.HotPlate', 'not json')
        check(bool(r) and not r.get('ok') and err_code(r) == 'bad-payload', 'A4g value 不是 JSON → bad-payload')

    # ---------------- T：Setup.TrayForm（C 路 UserDefForm_File）----------------
    print('== T  Setup.TrayForm（golden cTrayForm.cpp:570 cbTrayType1Change → :580 ShowTypePage）')
    tray = golden_grid(TRAY_CSV)
    ctl = 'cbTrayType%d' % (a.tray_type + 1)
    g = ws.cmd('editlist.get', 'UserDefForm_File')
    check(bool(g and g.get('ok')), 'T1 editlist.get UserDefForm_File')
    evs = (g or {}).get('events') or {}
    titems = (evs.get(ctl) or {}).get('items') or []
    # AI(W906-Q57-TRIAGE) 20260930: Q57 r2 FAIL（回應 4 個）＝舊期望寫在 TF-4 之前。b08ae6ad（B3 lane 2）把 golden V912
    #   cTrayForm.cpp:729 btnBinBoxResetClick（dfm OnClick，Bin Box 分頁 Reset 鈕）加進 tools/editlist/UserDefForm_File.py 'events'。
    #   現在要剛好這 4 個，而且各自接到 golden 的處理器（editlist.get events.<名>.golden，FileRW/_EditPage.cpp EvPageJson）。
    want_ev = {'cbTrayType1': 'cbTrayType1Change', 'cbTrayType2': 'cbTrayType1Change', 'cbTrayType3': 'cbTrayType1Change',
               'btnBinBoxReset': 'btnBinBoxResetClick'}
    check(sorted(evs) == sorted(want_ev) and all(h in ((evs.get(n) or {}).get('golden') or '') for n, h in want_ev.items()),
          'T1 回應的 events＝cbTrayType1..3（cbTrayType1Change）＋btnBinBoxReset（btnBinBoxResetClick）：%s' % sorted(evs))
    check(titems == golden_items(tray), 'T1 events.%s.items＝TrayForm.csv 非空名稱列（%d 項）' % (ctl, len(titems)))
    if len(titems) > a.tray_index:
        proxies = (g or {}).get('proxies') or {}
        r = ws.event('Setup.TrayForm', 'TfTrayForm', ctl, itemIndex=a.tray_index, text=titems[a.tray_index])
        ok = bool(r and r.get('ok'))
        check(ok and r.get('route') == 'C', 'T2 %s 第 %d 項 → ok：%s' % (ctl, a.tray_index, json.dumps(r, ensure_ascii=False)[:300]))
        if ok:
            k, i = a.tray_type + 1, a.tray_index
            names = ['TrayName%d' % k, 'XST%d' % k, 'YST%d' % k, 'XPitch%d' % k, 'YPitch%d' % k, 'Tp%dThick' % k,
                     'XCT%d' % k, 'YCT%d' % k, 'Tp%dTickUp' % k, 'edMemo%d' % k]
            cols = [0, 1, 2, 3, 4, 9, 5, 6, None, 11]
            after = {}
            for n in names:
                after[n] = ((r.get('changed') or {}).get(n) or {}).get('text', (proxies.get(n) or {}).get('text'))
            for n, c in zip(names, cols):
                if c is None:   # golden :597 TrayEdit[iTag][8]->Text = atof(Cells[7])/2.0（AnsiString(double)）
                    try:
                        good = abs(float(after[n]) - float(cell(tray, i, 7).strip() or 0) / 2.0) < 1e-9
                    except (TypeError, ValueError):
                        good = False
                    check(good, 'T2 %s＝X Width/2（%s/2）：%r' % (n, cell(tray, i, 7).strip(), after[n]))
                else:
                    check(after[n] == cell(tray, i, c).strip(), 'T2 %s＝CSV 第 %d 列第 %d 欄 %r：%r' % (n, i, c, cell(tray, i, c).strip(), after[n]))

    # ---------------- C：Setup.Cleaning（C 路 TestIF_File_Cleaning）----------------
    print('== C  Setup.Cleaning（golden uCleaning.cpp:2322 cbbSelectTrayChange；cbbSelectTray DFM Enabled=False）')
    g = ws.cmd('editlist.get', 'TestIF_File_Cleaning')
    check(bool(g and g.get('ok')), 'C1 editlist.get TestIF_File_Cleaning')
    ev = ((g or {}).get('events') or {}).get('cbbSelectTray') or {}
    check(ev.get('operable') is False, 'C1 events.cbbSelectTray.operable == false（golden 沒有地方打開它）：%s' % json.dumps(ev, ensure_ascii=False)[:200])
    r = ws.event('Setup.Cleaning', 'TfCleaning', 'cbbSelectTray', itemIndex=1)
    check(bool(r) and not r.get('ok') and err_code(r) == 'bad-payload', 'C2 form.event → bad-payload（照 golden：點不到）：%s' % json.dumps(r, ensure_ascii=False)[:240])

    ws.cmd('control.release')
    after = {p: sha(p) for p in watched}
    moved = [p for p in watched if before[p] != after[p]]
    print('== 檔案（SHA256 前後）：%d 個有變%s' % (len(moved), '' if not moved else '（form.event 不寫檔；這是 golden FormShow 的讀檔／補鍵，見檔頭）'))
    for p in moved:
        print('     ' + p)
    check(not [p for p in moved if p in (PLATE_CSV, TRAY_CSV)], 'PlateForm.csv／TrayForm.csv 沒被寫（golden 只讀）')
    print('失敗 %d 項' % len(FAILS))
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
