# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_testcategory_probe.py -- Data.TestCategory.html（golden V912 TfTestCategory，
#  cTestCategory.cpp／cTestCategory.dfm）接 C++ tcat.* tag 的 e2e probe。
#
#  Steven 團隊 20260925 (Data.TestCategory).
#
#  用真的瀏覽器（headless Edge ＋ DevTools）開頁面，驗：
#    F  HTML 原檔：格子全是 "---"、沒有綠底 td.g、沒有寫死的 "2"；載入手寫的 ht9045_testcategory_wire.js
#    T  176 個 tcat.* 都在線上快照（4 個表單狀態 ＋ 2 張格子 ×（5 個版面 ＋ 3x9 格 × 文字／bg／bold））
#    L  liveness：tcat.show 不是 null —— 否則是 wb_serve 沒有呼叫 W906_BootTestCategory()（整合步驟），
#       這時驗「全部 null、畫面全是 "---"」後結束
#    S  tag＝來源（探針自己重算 golden，不讀 C++ 的算法）：
#         tcat.show／cateByArm   ＝ D:\HT9045\config\config.ini [Visible] bShowTestCate／iShowCateByArm（cprod.cpp:2858/:2862）
#         tcat.height            ＝ 192（By Arm）／105（golden SetShowCateMode :511-525，DEBUG_WIN7_FULL_HD 未定義）
#         版面                   ＝ golden AdjFormData（:42-143）以 GET /api/struct/testIF.live 的 iTestMode、
#                                  /api/struct/testIF.file 的 iTestMode（IsNNMode，cinitial.cpp:7417）重算；
#                                  bShowTestCate=0 時 golden 沒有 FormShow → AdjFormData 沒跑 → dfm 的 3x3／80／24
#         標題格                 ＝ golden sgArm1DrawCell :155-210（"Socket n"／"Arm1"／"Arm2"、a..h、A/B 或 NN 的 B/C/D）
#                                  底色 clBtnFace（0x8000000F）；左上角不粗、其餘粗
#         內容格                 ＝ 開機沒測過時 InitCateCell（:496-509）→ -1／clWhite → 文字 "" 或 "X"（bCloseSiteByIndexArm）、
#                                  底色 clWhite、粗體（沿用列標題的 fsBold）；測過時文字只能是 golden 的形狀（數字／E／Err／Error／H）、
#                                  底色只能是 golden 用過的 clGreen／clRed／clSilver／clYellow／clWhite
#                                  NN_1Row 的第 5 欄以後 golden 不畫 → VCL 預設（""、clWindow、不粗）
#         超出 RowCount／ColCount 的格子 ＝ null
#         InitialOK==false（spine pump 沒 armed）→ golden :148 整支 return，全部是 VCL 預設繪製，另外驗這個形狀
#    D  畫面＝tag：表格顯示＝tcat.<g>.visible；格數＝rowCount×colCount；每格文字＝tag（null → "---"）；
#       有明確底色的格子 computed background＝TColor 的 RGB
#
#  不寫任何檔、不送任何指令（只讀 tag 與 GET /api/struct），不需要登入、不需要控制權杖。
#  ⚠ 沒辦法從網頁觸發一次測試循環，所以「測過之後」的形狀只在機台真的測過時才驗得到（探針會印 INFO 說明走的是哪一段）。
#    setter → 格子的路徑另有 C++ 單元測試 tests/test_testcategory_paint.cpp（整合者註冊進 ctest）。
#
#  用法：
#      python tools\webprobe\data_testcategory_probe.py --port 8046
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import re
import shutil
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge                    # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
TREE = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PAGE_HTML = os.path.normpath(os.path.join(TREE, '..', 'web', 'page', 'Data.TestCategory.html'))
CONFIG = r'D:\HT9045\config\config.ini'

GRIDS = ['sgArm1', 'sgArm2']                 # Tag 0 / Tag 1（MyStringGD[0]／[1]）
R, C = 3, 9
MAX_SOCKET_ROW = 4                           # MachineType.h:485
CL_BTNFACE = -2147483633                     # (int)0x8000000F
CL_WINDOW = -2147483643                      # (int)0x80000005
CL_WHITE, CL_GREEN, CL_RED, CL_SILVER, CL_YELLOW = 0xFFFFFF, 0x008000, 0x0000FF, 0xC0C0C0, 0x00FFFF
GOLDEN_COLORS = {CL_WHITE, CL_GREEN, CL_RED, CL_SILVER, CL_YELLOW}

FIXED = ['tcat.show', 'tcat.cateByArm', 'tcat.width', 'tcat.height']
PER = []
for _g in GRIDS:
    PER += ['tcat.%s.%s' % (_g, k) for k in ('visible', 'rowCount', 'colCount', 'colWidths', 'rowHeights')]
    for _r in range(R):
        for _c in range(C):
            base = 'tcat.%s.r%d.c%d' % (_g, _r, _c)
            PER += [base, base + '.bg', base + '.bold']
ALL_TAGS = FIXED + PER
assert len(ALL_TAGS) == 176


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def http_json(port, path):
    return json.loads(urllib.request.urlopen('http://127.0.0.1:%d%s' % (port, path), timeout=10).read().decode('utf-8'))


def read_ini(path, section):
    """同一鍵出現多次取第一個（與 GetPrivateProfileString／TIniFile 一致）。"""
    try:
        txt = open(path, 'rb').read().decode('cp950', errors='replace')
    except OSError:
        return {}
    sec, vals = None, {}
    for line in txt.splitlines():
        s = line.strip()
        if s.startswith('[') and s.endswith(']'):
            sec = s[1:-1]
            continue
        if sec == section and '=' in s:
            k, v = s.split('=', 1)
            vals.setdefault(k.strip(), v.strip())
    return vals


def ini_bool(v):
    if v is None:
        return None
    return v.strip().lower() in ('1', 'true', 'yes')


# ---- golden 重算 --------------------------------------------------------------
def is_nn(file_mode):
    """golden cinitial.cpp:15111 IsNNMode（移植 cinitial.cpp:7417）：0 None_NN／1 NN_1Row／2 NN_2Row。"""
    if file_mode in (6, 8, 10):              # QualSite2X2N／_6Site2X3N／_8Site2X4N
        return 1
    if file_mode in (15, 14):                # _32Site4X8N／_16Site4X4
        return 2
    return 0


def adj_form_data(mode, nn):
    """golden AdjFormData :42-143 → (rows, colWidths, width)。mode＝TestIF.iTestMode（live）。"""
    rows = 2 if ((mode <= 3 or mode == 17 or nn == 1) and mode != 4) else 3
    width = 269
    if mode == 0:
        cw = [80, 120]
    elif mode in (2, 8, 7):
        cw = [80] + [40] * 3
    elif mode in (3, 9, 17, 14, 10):
        cw = [80] + [40] * 4
    elif mode == 11:
        cw, width = [80] + [40] * 5, 309
    elif mode == 12:
        cw, width = [80] + [40] * 6, 349
    elif mode in (13, 15, 16):
        cw, width = [80] + [40] * 8, 429
    else:
        cw = [80, 80, 80]
    return rows, cw, width


def header(tag, r, c, by_arm, nn):
    """golden sgArm1DrawCell :155-210 的標題格 → (text, bold) 或 None（golden 不畫）。"""
    if c == 0 and r == 0:
        return (('Socket %d' % tag) if not by_arm else ['Arm1', 'Arm2'][tag]), False
    if r == 0:
        return (chr(ord('a') + c - 1), True) if 1 <= c <= 8 else None
    if c == 0:
        if nn == 2:
            return (chr((ord('C') if tag == 0 else ord('A')) + r - 1), True) if r < MAX_SOCKET_ROW + 1 else None
        if nn == 1:
            return (chr((ord('B') if tag == 0 else ord('A')) + r - 1), True) if r < MAX_SOCKET_ROW + 1 else None
        return (chr(ord('A') + r - 1), True) if r in (1, 2) else None
    return None


def content_drawn(r, c, nn):
    """golden 會不會畫這個內容格（:211-358）。"""
    if nn == 2:
        return 1 <= c <= 8
    if nn == 1:
        return 1 <= c <= 4
    return r in (1, 2) and 1 <= c <= 8


BIN_TEXT = re.compile(r'^(-?\d+|E|Err|Error|H|X|)$')


def rgb(v):
    return 'rgb(%d, %d, %d)' % (v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF)


def tags_and_dom(cdp):
    js = ("(function(){var T=%s,o={tags:{},has:{},dom:{}};"
          "T.forEach(function(t){o.has[t]=HT9045Tags.has(t);o.tags[t]=HT9045Tags.get(t);});"
          "%s.forEach(function(g){var tb=document.getElementById(g);var d={shown:tb?(getComputedStyle(tb).display!=='none'):null,rows:[]};"
          "if(tb){Array.prototype.forEach.call(tb.rows,function(tr){var row=[];Array.prototype.forEach.call(tr.cells,function(td){"
          "row.push({text:td.textContent.replace(/\\u00a0/g,''),bg:getComputedStyle(td).backgroundColor,"
          "fw:getComputedStyle(td).fontWeight,unk:td.classList.contains('tcatUnknown')});});d.rows.push(row);});}"
          "o.dom[g]=d;});o.status=(document.getElementById('tcatStatus')||{}).textContent||'';"
          "o.renders=(window.__tcat||{}).renders;return JSON.stringify(o);})()") % (json.dumps(ALL_TAGS), json.dumps(GRIDS))
    return json.loads(cdp.eval(js, timeout=30))


def verify_dom(o, label):
    v = o['tags']
    for gi, g in enumerate(GRIDS):
        d = o['dom'][g]
        vis = v['tcat.%s.visible' % g]
        check(d['shown'] == (vis is not False), 'D  %s：%s 顯示=%r ＝ visible tag %r' % (label, g, d['shown'], vis))
        rows = v['tcat.%s.rowCount' % g]
        cols = v['tcat.%s.colCount' % g]
        er = min(rows, R) if isinstance(rows, int) else 3
        ec = min(cols, C) if isinstance(cols, int) else 3
        shape = [len(x) for x in d['rows']]
        check(len(d['rows']) == er and all(n == ec for n in shape),
              'D  %s：%s 畫出 %d 列 × %s 欄 ＝ tag %r×%r（null 時 dfm 3x3）' % (label, g, len(d['rows']), shape, rows, cols))
        bad, badbg = [], []
        for r in range(min(er, len(d['rows']))):
            for c in range(min(ec, len(d['rows'][r]))):
                base = 'tcat.%s.r%d.c%d' % (g, r, c)
                t, bg = v[base], v[base + '.bg']
                want = '---' if t is None else t
                got = d['rows'][r][c]['text']
                if got != want:
                    bad.append('%s DOM=%r tag=%r' % (base, got, t))
                if t is not None and isinstance(bg, int) and bg >= 0 and d['rows'][r][c]['bg'] != rgb(bg):
                    badbg.append('%s bg DOM=%s tag=%s' % (base, d['rows'][r][c]['bg'], rgb(bg)))
        check(not bad, 'D  %s：%s 每格文字＝tag（null→"---"；不符 %d：%s）' % (label, g, len(bad), bad[:4]))
        check(not badbg, 'D  %s：%s 明確底色格的 computed background＝TColor RGB（不符：%s）' % (label, g, badbg[:4]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9341)
    a = ap.parse_args()

    # ---- F：HTML 原檔 ------------------------------------------------------------
    html = open(PAGE_HTML, encoding='utf-8').read()
    body = re.sub(r'<!--.*?-->', '', html, flags=re.S)
    cells = re.findall(r'<t[hd][^>]*>([^<]*)</t[hd]>', body)
    notdash = [x for x in cells if x.strip() != '---']
    check(len(cells) == 18 and not notdash, 'F  Data.TestCategory.html 原檔 %d 格（dfm 3x3 ×2）全是 "---"（非 ---：%s）' % (len(cells), notdash[:5]))
    check('class="g"' not in body and not re.search(r'<td[^>]*>\s*2\s*</td>', body), 'F  原檔沒有綠底 td.g、沒有寫死的 "2"')
    check('ht9045_recipe_client.js' in body and 'ht9045_testcategory_wire.js' in body,
          'F  頁面載入 ht9045_recipe_client.js ＋ 手寫的 ht9045_testcategory_wire.js')

    # ---- 來源 ---------------------------------------------------------------------
    vis_ini = read_ini(CONFIG, 'Visible')
    fn_ini = read_ini(CONFIG, 'Function')
    ini_show = ini_bool(vis_ini.get('bShowTestCate'))
    ini_byarm = vis_ini.get('iShowCateByArm')
    ini_byarm = (int(ini_byarm) != 0) if ini_byarm not in (None, '') and re.match(r'^-?\d+$', ini_byarm) else None
    ini_a09 = ini_bool(fn_ini.get('bCloseSiteByIndexArm'))
    print('  INFO  config.ini [Visible] bShowTestCate=%r iShowCateByArm=%r；[Function] bCloseSiteByIndexArm=%r' % (
        vis_ini.get('bShowTestCate'), vis_ini.get('iShowCateByArm'), fn_ini.get('bCloseSiteByIndexArm')))
    try:
        live = http_json(a.port, '/api/struct/testIF.live')['values']
        filev = http_json(a.port, '/api/struct/testIF.file')['values']
        mode, fmode = live.get('iTestMode'), filev.get('iTestMode')
        print('  INFO  /api/struct：TestIF.iTestMode=%r TestIF_File.iTestMode=%r iShuttleMode=%r iShuttle_Sel=%r' % (
            mode, fmode, live.get('iShuttleMode'), live.get('iShuttle_Sel')))
    except Exception as e:                       # noqa: BLE001
        mode = fmode = None
        live = {}
        check(False, 'S  GET /api/struct/testIF.live|file（%s）' % e)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/Data.TestCategory.html' % a.port})
        time.sleep(0.5)
        ok = wait_js(cdp, "window.HT9045Tags && HT9045Tags.status().connected && HT9045Tags.has('tcat.sgArm2.r2.c8.bold')"
                          " && window.HT9045TestCategory", 60)
        check(bool(ok), 'T  開頁、tag 串流已連上且收到 tcat.sgArm2.r2.c8.bold')
        if not ok:
            return len(FAILS)
        time.sleep(1.2)                                          # 等一個完整快照（500 ms 一拍）＋ rAF 重畫
        o = tags_and_dom(cdp)
        v = o['tags']
        missing = [k for k in ALL_TAGS if not o['has'][k]]
        check(not missing, 'T  176 個 tcat.* 都在線上快照（缺 %d：%s）' % (len(missing), missing[:6]))

        # ---- L：liveness ----------------------------------------------------------
        if v['tcat.show'] is None:
            nonnull = [k for k in ALL_TAGS if v[k] is not None]
            check(not nonnull, 'L  tcat.show 是 null 時其餘 tcat.* 也全是 null（沒有一半真一半假；非 null：%s）' % nonnull[:5])
            verify_dom(o, '未開機')
            check(False, 'L  tcat.show=null —— wb_serve 沒有呼叫 W906_BootTestCategory()（golden main.cpp:9882／:10594），'
                         '或 config 未載入。整合步驟見 cTestCategory.cpp 檔頭／交件報告')
            return len(FAILS)
        check(isinstance(v['tcat.show'], bool) and isinstance(v['tcat.cateByArm'], bool),
              'L  tcat.show=%r、tcat.cateByArm=%r 有值' % (v['tcat.show'], v['tcat.cateByArm']))

        # ---- S：表單狀態 ------------------------------------------------------------
        show, by_arm = v['tcat.show'], v['tcat.cateByArm']
        if ini_show is not None:
            check(show == ini_show, 'S  tcat.show=%r ＝ config.ini bShowTestCate=%r（golden DoShowUserDefFrom :9170-9178）' % (show, ini_show))
        else:
            print('  INFO  config.ini 沒有 bShowTestCate：golden 用 LastSet.ShowTestCate 當預設，本探針不驗 tcat.show 的值')
        if ini_byarm is not None:
            check(by_arm == ini_byarm, 'S  tcat.cateByArm=%r ＝ config.ini iShowCateByArm!=0（%r）' % (by_arm, ini_byarm))
        check(v['tcat.height'] == (192 if by_arm else 105), 'S  tcat.height=%r ＝ golden SetShowCateMode（%s）' % (
            v['tcat.height'], 192 if by_arm else 105))
        check(v['tcat.sgArm1.visible'] is True and v['tcat.sgArm2.visible'] == by_arm,
              'S  visible：sgArm1=%r（恆 true）sgArm2=%r（＝By Arm %r）' % (v['tcat.sgArm1.visible'], v['tcat.sgArm2.visible'], by_arm))

        # ---- S：版面 ----------------------------------------------------------------
        nn = is_nn(fmode) if isinstance(fmode, int) else None
        if show and isinstance(mode, int) and nn is not None:
            rows_e, cw_e, width_e = adj_form_data(mode, nn)
            src = 'golden AdjFormData（iTestMode=%d，IsNNMode=%d）' % (mode, nn)
        elif show is False:
            rows_e, cw_e, width_e = 3, [80, 80, 80], None
            src = 'dfm（bShowTestCate=0 → 沒有 FormShow → AdjFormData 沒跑）'
        else:
            rows_e = cw_e = width_e = None
            src = None
        if src:
            print('  INFO  預期版面來自 %s：rows=%d colWidths=%s width=%r' % (src, rows_e, cw_e, width_e))
            for g in GRIDS:
                check(v['tcat.%s.rowCount' % g] == rows_e and v['tcat.%s.colCount' % g] == len(cw_e),
                      'S  %s rowCount/colCount=%r/%r ＝ %d/%d' % (g, v['tcat.%s.rowCount' % g], v['tcat.%s.colCount' % g], rows_e, len(cw_e)))
                check(v['tcat.%s.colWidths' % g] == ','.join(str(x) for x in cw_e),
                      'S  %s colWidths=%r ＝ %r' % (g, v['tcat.%s.colWidths' % g], ','.join(str(x) for x in cw_e)))
                check(v['tcat.%s.rowHeights' % g] == ','.join(['24'] * rows_e),
                      'S  %s rowHeights=%r ＝ 全 24（EdgeHeight／golden :50-70）' % (g, v['tcat.%s.rowHeights' % g]))
            check(v['tcat.width'] == width_e, 'S  tcat.width=%r ＝ %r' % (v['tcat.width'], width_e))

        # ---- S：格子內容 ------------------------------------------------------------
        corner = v['tcat.sgArm1.r0.c0']
        initial_ok = corner not in ('', None)
        if not initial_ok:
            print('  INFO  左上角是空字串：InitialOK==false（spine pump 沒 armed）→ golden :148 整支 return，驗 VCL 預設繪製')
        nn_draw = nn if nn is not None else 0
        idle = True
        bad_hdr, bad_cell, bad_null, bad_def = [], [], [], []
        for tag_i, g in enumerate(GRIDS):
            rows, cols = v['tcat.%s.rowCount' % g], v['tcat.%s.colCount' % g]
            for r in range(R):
                for c in range(C):
                    base = 'tcat.%s.r%d.c%d' % (g, r, c)
                    t, bg, bold = v[base], v[base + '.bg'], v[base + '.bold']
                    if not (isinstance(rows, int) and isinstance(cols, int) and r < rows and c < cols):
                        if t is not None or bg is not None or bold is not None:
                            bad_null.append(base)
                        continue
                    fixed = (r == 0 or c == 0)
                    if not initial_ok:
                        if (t, bg, bold) != ('', CL_BTNFACE if fixed else CL_WINDOW, False):
                            bad_def.append('%s=%r' % (base, (t, bg, bold)))
                        continue
                    h = header(tag_i, r, c, by_arm, nn_draw) if fixed else None
                    if fixed:
                        want = (h[0], CL_BTNFACE, h[1]) if h else ('', CL_BTNFACE, False)
                        if (t, bg, bold) != want:
                            bad_hdr.append('%s=%r 預期 %r' % (base, (t, bg, bold), want))
                    elif not content_drawn(r, c, nn_draw):
                        if (t, bg, bold) != ('', CL_WINDOW, False):
                            bad_cell.append('%s golden 不畫 → 應為 VCL 預設，實際 %r' % (base, (t, bg, bold)))
                    else:
                        if t not in ('', 'X') or bg != CL_WHITE:
                            idle = False
                        if not isinstance(t, str) or not BIN_TEXT.match(t):
                            bad_cell.append('%s 文字 %r 不是 golden 的形狀' % (base, t))
                        if bg not in GOLDEN_COLORS:
                            bad_cell.append('%s 底色 %r 不是 golden 用過的顏色' % (base, bg))
                        if bold is not True:
                            bad_cell.append('%s bold=%r（應沿用列標題的 fsBold）' % (base, bold))
                        if t == 'X' and ini_a09 is False:
                            bad_cell.append('%s="X" 但 bCloseSiteByIndexArm=0（golden :229/:277/:322 只在 A09 時畫 X）' % base)
                        # 註：空白格可以是黃底 —— golden ProcessStartTestData（atester_ProcessCount.cpp:2012）
                        #     SetTestingCateCell 只改 ColorPtr=clYellow、不動 TestResult（仍是 -1）。
        check(not bad_null, 'S  超出 RowCount×ColCount 的格子全是 null（不符：%s）' % bad_null[:4])
        if not initial_ok:
            check(not bad_def, 'S  InitialOK==false：每格都是 VCL 預設（""、固定格 clBtnFace／其餘 clWindow、不粗；不符：%s）' % bad_def[:4])
        else:
            check(not bad_hdr, 'S  標題格＝golden :155-210（"Socket n"／Arm1／Arm2、a..h、列字母；clBtnFace；左上不粗；不符：%s）' % bad_hdr[:4])
            check(not bad_cell, 'S  內容格＝golden 形狀（不符 %d：%s）' % (len(bad_cell), bad_cell[:4]))
            print('  INFO  內容格狀態：%s' % ('開機後還沒測過（全部 "" 或 "X" 且白底，InitCateCell 的 -1／clWhite）' if idle
                                          else '機台測過或測試中（有 bin 值或非白底）—— 已驗文字／底色屬於 golden 的形狀'))

        verify_dom(o, '開機')
        check(bool(o['status'].strip()), 'D  說明列有內容：%r' % o['status'][:80])
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('data_testcategory_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
