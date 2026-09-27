# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_sortct_probe.py -- Data.SortCT.html（golden V912 TfSortCT，cSortCT.cpp／cSortCT.dfm）
#  接 C++ sort.* tag 與 Clear Count 鈕的 e2e probe。
#
#  Steven 團隊 20260925 (Data.SortCT).
#
#  用真的瀏覽器（headless Edge ＋ DevTools）開頁面，驗：
#    T  頁面接線檔註冊的 tag＝預期的 178 個；全部在線上快照裡
#    S  tag＝來源：自己讀 D:\HT9045\system\lastdata.dat（golden LastSet 的原始傾印，cprod.cpp ReadLastDataFile），
#       欄位偏移由本探針用同一支 MinGW g++ 編一個小程式 offsetof(LAST_GENERAL_SET, …) 取得（檔案大小＝sizeof 才算數）：
#         sort.loading           ＝ SendCT[0]
#         sort.<站>.count        ＝ BinCT[0][iTo3Unload[i]]（golden main.cpp:2003-2035 的對照表，本檔寫死）
#         sort.total             ＝ 有設定的站的數量總和（golden ShowSortIC :352-357；「有設定」＝該站 count tag 非 null）
#         sort.<站>.yield        ＝ golden ChangeToPercentage：Sum==0 → "0.00%"，否則 "%0.2f%%"（MachineType.h:1713）
#         sort.art.*             ＝ BinCT_ART／SendCT_ART，ART 總數只在 sort.art.active 時累加（:358-363）
#    V  列可見度＝golden UpForm（:718-790）裡只看 Gerneral.ini 的那幾條（AUTO_EMPTY_COLOR／AUTO3_IS_MAGAZINE／
#       USE_Scanner_AOI_Inspection）；Fix4-6 共用同一條件（iFixTrayMode），三個必須一致；ART 分頁可見度＝FormShow :196-204
#    D  畫面＝tag：178 個 tag 綁的格子文字＝tag（null→"---"）；列／pnlYield／分頁的顯示狀態＝可見度 tag
#    F  沒有假數字：HTML 原檔所有值格都是 "---"；舊展示值（312／298／92.28%…）掃不到
#    C  Clear Count（golden btnClearCountClick :585-708）：
#         分派沒接（act.sortCT.clearCount 回 unknown-action）→ SKIP，並驗「按了不會假裝清了」（頁面訊息、lastdata.dat 不變）
#         有接 → (1) 詢問段取消：走到確認框、沒有清、lastdata.dat 不變；(2) 確認段：executed、lastdata.dat 的
#                SendCT[0..3]／BinCT[0][*]／BinCT[2][*]／BinCT[3][*]／iIndexInputOutPut[0..3] 歸零（Clarn_Data(8) 的
#                ctLoadingCounts／ctTraySortCount／ctIndexCount）、tag 跟著歸零、QtyData 本行程第一次 Clarn_Data 不寫檔
#
#  ⚠ C 段會寫 D:\HT9045\system\lastdata.dat／lastdata_backup.dat（restore 範圍）與建立 D:\HT9045_Log\QtyData\YYYYMM\（目錄）。
#    請用 scratchpad/run_sortct.sh 跑（鎖＋restore＋SHA256），不要單獨對真檔跑。
#
#  用法：
#      python tools\webprobe\data_sortct_probe.py --port 8046 --user S12TEST --password S12PW [--no-clear]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
TREE = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
LASTDATA = r'D:\HT9045\system\lastdata.dat'
GENERAL = r'D:\HT9045\system\Gerneral.ini'
PAGE_HTML = os.path.normpath(os.path.join(TREE, '..', 'web', 'page', 'Data.SortCT.html'))
QTY = r'D:\HT9045_Log\QtyData'

# golden e6TrayName 順序（MachineType.h:1191-1226）與 iTo3Unload（golden V912 main.cpp:2003-2035）
ST = (['auto%d' % i for i in range(1, 7)] + ['fix%d' % i for i in range(1, 13)] + ['bulkbox'] +
      ['mag%d' % i for i in range(1, 15)])
TO3 = ([0, 1, 2, 24, 25, 26] + [3, 4, 5, 6, 7, 8] + [27, 28, 29, 30, 31, 32] + [9] + list(range(10, 24)))
assert len(ST) == 33 and len(TO3) == 33 and sorted(TO3) == list(range(33))


def sfx(slug):
    if slug == 'bulkbox':
        return 'BinBox'
    m = re.match(r'^([a-z]+)(\d+)$', slug)
    return m.group(1).capitalize() + m.group(2)


FIXED = ['sort.loading', 'sort.total', 'sort.yield', 'sort.yield.visible', 'sort.art.loading', 'sort.art.total',
         'sort.art.yield', 'sort.art.tabVisible', 'sort.art.tabCaption', 'sort.ic.tabVisible', 'sort.ic.load',
         'sort.ic.hp1', 'sort.ic.hp2']
PER = []
for s in ST:
    PER += ['sort.%s.count' % s, 'sort.%s.yield' % s, 'sort.%s.visible' % s, 'sort.art.%s.count' % s, 'sort.art.%s.yield' % s]
ALL_TAGS = FIXED + PER                                         # 13 + 165 = 178
UNBOUND = ['sort.art.active']                                 # C++ 有送、頁面不綁（golden ShowSortIC :358-360 的 ART 計數條件；探針算 Sum_ART 用）
# 文字格：tag -> 元件 id（與 web/page/ht9045_sortct_wire.js 一致）
TEXT = {'sort.loading': 'pnlLoader', 'sort.total': 'pnlTotal', 'sort.yield': 'pnlYield',
        'sort.art.loading': 'pnlLoadingART', 'sort.art.total': 'pnlTotalART', 'sort.art.yield': 'pnlYieldART',
        'sort.ic.load': 'pnlLoad', 'sort.ic.hp1': 'pnlHP1', 'sort.ic.hp2': 'pnlHP2'}
for s in ST:
    TEXT['sort.%s.count' % s] = 'pnl' + sfx(s)
    TEXT['sort.%s.yield' % s] = 'pnl' + sfx(s) + 'Yield'
    TEXT['sort.art.%s.count' % s] = 'pnART' + sfx(s)
    TEXT['sort.art.%s.yield' % s] = 'pnART' + sfx(s) + 'Yield'
INT_TAGS = {'sort.loading', 'sort.total', 'sort.art.loading', 'sort.art.total'} | \
           {'sort.%s.count' % s for s in ST} | {'sort.art.%s.count' % s for s in ST}

FAKE = [r'\b312\b', r'\b298\b', r'92\.28%', r'6\.71%', r'1\.01%', r'\b275\b']

LAYOUT_FIELDS = ['SendCT', 'SendCT_ART', 'BinCT', 'BinCT_ART', 'iIndexInputOutPut', 'LastOpenFilename']
# 只編譯成組語、不執行（這台機器從 %TEMP% 執行新編的 exe 會被擋：WinError 5）—— 常數值直接從 .s 讀
LAYOUT_CPP = ('#include <cstddef>\n#include "LastSet.h"\n'
              'extern const unsigned W906L_size = sizeof(LAST_GENERAL_SET);\n'
              'extern const unsigned W906L_sizeofLong = sizeof(long);\n' +
              ''.join('extern const unsigned W906L_%s = offsetof(LAST_GENERAL_SET, %s);\n' % (f, f) for f in LAYOUT_FIELDS))


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def layout():
    """用建 wb_serve 的同一支 MinGW g++ 取 LAST_GENERAL_SET 的 sizeof／offsetof（不猜偏移）。"""
    d = tempfile.mkdtemp(prefix='sortct_layout_')
    try:
        src, asm = os.path.join(d, 'l.cpp'), os.path.join(d, 'l.s')
        open(src, 'w').write(LAYOUT_CPP)
        gxx = shutil.which('g++') or r'C:\MinGW\bin\g++.exe'
        subprocess.check_call([gxx, '-std=c++1z', '-S', '-I', TREE, src, '-o', asm],
                              env=dict(os.environ, PATH=r'C:\MinGW\bin;' + os.environ.get('PATH', '')))
        s = open(asm, encoding='utf-8', errors='replace').read()
        out = {}
        for name in ['size', 'sizeofLong'] + LAYOUT_FIELDS:
            m = re.search(r'_?W906L_%s:\s*\n\s*\.(long\s+(\d+)|space\s+4)' % name, s)   # 值為 0 時 g++ 放 .space 4（例 LastOpenFilename 偏移 0）
            if not m:
                raise SystemExit('layout: %s not found in g++ -S output' % name)
            out[name] = int(m.group(2)) if m.group(2) else 0
        return out
    finally:
        shutil.rmtree(d, ignore_errors=True)


def read_lastdata(L):
    b = open(LASTDATA, 'rb').read()
    lf = 'i' if L['sizeofLong'] == 4 else 'q'
    r = {'size': len(b), 'raw': b}
    r['SendCT'] = list(struct.unpack_from('<4' + lf, b, L['SendCT']))
    r['SendCT_ART'] = list(struct.unpack_from('<4' + lf, b, L['SendCT_ART']))
    r['BinCT'] = [list(struct.unpack_from('<256I', b, L['BinCT'] + k * 1024)) for k in range(4)]
    r['BinCT_ART'] = [list(struct.unpack_from('<256I', b, L['BinCT_ART'] + k * 1024)) for k in range(4)]
    r['iIndexInputOutPut'] = list(struct.unpack_from('<4' + lf, b, L['iIndexInputOutPut']))
    r['LastOpenFilename'] = b[L['LastOpenFilename']:L['LastOpenFilename'] + 64].split(b'\0')[0]
    return r


def read_general():
    vals = {}
    for line in open(GENERAL, 'rb').read().decode('cp950', errors='replace').splitlines():
        if '=' in line and not line.strip().startswith('['):
            k, v = line.split('=', 1)
            vals.setdefault(k.strip(), v.strip())
    return vals


def pct(n, d):
    """golden ChangeToPercentage（MachineType.h:1713-1719）。"""
    return '0.00%' if d == 0 else '%0.2f%%' % (float(n) / float(d) * 100.0)


def fmt(tag, v):
    if v is None:
        return '---'
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if tag in INT_TAGS and isinstance(v, (int, float)):
        return '%d' % int(round(v))
    return str(v)


def tags(cdp, names):
    js = "JSON.stringify(%s.map(function(t){return [t, HT9045Tags.has(t), HT9045Tags.get(t)];}))" % json.dumps(names)
    return {t: (h, v) for t, h, v in json.loads(cdp.eval(js, timeout=30))}


def dom_state(cdp):
    js = ("(function(){var o={text:{},rows:{},artrows:{}};var T=%s;"
          "Object.keys(T).forEach(function(t){var e=document.getElementById(T[t]);o.text[t]=e?e.textContent:null;});"
          "%s.forEach(function(s){var r=document.getElementById('row_'+s),a=document.getElementById('artrow_'+s);"
          "o.rows[s]=r?(r.style.display!=='none'):null;o.artrows[s]=a?(a.style.display!=='none'):null;});"
          "var y=document.getElementById('pnlYield');o.yieldShown=y?(y.style.visibility!=='hidden'):null;"
          "var ta=document.getElementById('tabARTSortCount'),ti=document.getElementById('tabICCount');"
          "o.artTab=ta?(ta.style.display!=='none'):null;o.artTabText=ta?ta.textContent:null;"
          "o.icTab=ti?(ti.style.display!=='none'):null;"
          "o.status=(document.getElementById('sortStatus')||{}).textContent||'';"
          "return JSON.stringify(o);})()") % (json.dumps(TEXT), json.dumps(ST))
    return json.loads(cdp.eval(js, timeout=30))


def verify_sources(t, ld, label):
    v = {k: t[k][1] for k in t}
    check(v['sort.loading'] == ld['SendCT'][0], 'S  %s：sort.loading=%r ＝ lastdata SendCT[0]=%r' % (label, v['sort.loading'], ld['SendCT'][0]))
    live = [i for i, s in enumerate(ST) if v['sort.%s.count' % s] is not None]
    bad = ['%s=%r≠%r' % (ST[i], v['sort.%s.count' % ST[i]], ld['BinCT'][0][TO3[i]]) for i in live
           if v['sort.%s.count' % ST[i]] != ld['BinCT'][0][TO3[i]]]
    check(not bad, 'S  %s：%d 個有設定的站 count＝lastdata BinCT[0][iTo3Unload[i]]（不符：%s）' % (label, len(live), bad))
    total = sum(ld['BinCT'][0][TO3[i]] for i in live)
    check(v['sort.total'] == total, 'S  %s：sort.total=%r ＝ golden Sum（有設定的 %d 站）=%r' % (label, v['sort.total'], len(live), total))
    six = sum(ld['BinCT'][0][TO3[i]] for i in live if ST[i] in ('auto1', 'auto2', 'auto3', 'fix1', 'fix2', 'fix3'))
    print('  INFO  %s：有設定的站 %s；舊六站公式會得 %d（golden 33 站 %d）' % (label, [ST[i] for i in live], six, total))
    bad = ['%s=%r≠%r' % (ST[i], v['sort.%s.yield' % ST[i]], pct(ld['BinCT'][0][TO3[i]], total)) for i in live
           if v['sort.%s.yield' % ST[i]] != pct(ld['BinCT'][0][TO3[i]], total)]
    check(not bad, 'S  %s：各站 yield＝ChangeToPercentage(count, Sum)（不符：%s）' % (label, bad))
    nul = [s for i, s in enumerate(ST) if i not in live and (v['sort.%s.yield' % s] is not None or v['sort.art.%s.count' % s] is not None)]
    check(not nul, 'S  %s：沒設定的站 yield／ART 也是 null（golden 不寫；不符：%s）' % (label, nul))
    # ART
    check(v['sort.art.loading'] == ld['SendCT_ART'][0], 'S  %s：sort.art.loading=%r ＝ SendCT_ART[0]=%r' % (label, v['sort.art.loading'], ld['SendCT_ART'][0]))
    bad = [s for i, s in enumerate(ST) if i in live and v['sort.art.%s.count' % s] != ld['BinCT_ART'][0][TO3[i]]]
    check(not bad, 'S  %s：ART count＝BinCT_ART[0][iTo3Unload[i]]（不符：%s）' % (label, bad))
    art_total = sum(ld['BinCT_ART'][0][TO3[i]] for i in live) if v['sort.art.active'] else 0
    check(v['sort.art.total'] == art_total, 'S  %s：sort.art.total=%r ＝ golden Sum_ART=%r（sort.art.active=%r）' % (label, v['sort.art.total'], art_total, v['sort.art.active']))
    bad = [s for i, s in enumerate(ST) if i in live and v['sort.art.%s.yield' % s] != pct(ld['BinCT_ART'][0][TO3[i]], art_total)]
    check(not bad, 'S  %s：ART yield＝ChangeToPercentage(count, Sum_ART)（不符：%s）' % (label, bad))
    # 總良率：只在 bLowYieldAlarmByBin（sort.yield.visible）時有值
    yv, y = v['sort.yield.visible'], v['sort.yield']
    check((yv is False and y is None) or (yv is True and isinstance(y, str) and y.endswith('%')),
          'S  %s：總良率 sort.yield=%r 只在 sort.yield.visible=%r 時有值（golden :430-437）' % (label, y, yv))
    return live


def verify_visibility(t, gen):
    v = {k: t[k][1] for k in t}
    aec = int(gen.get('AUTO_EMPTY_COLOR', '0') or 0)
    mag = int(gen.get('AUTO3_IS_MAGAZINE', '0') or 0)
    aoi = int(gen.get('USE_Scanner_AOI_Inspection', '0') or 0)
    print('  INFO  Gerneral.ini：AUTO_EMPTY_COLOR=%d AUTO3_IS_MAGAZINE=%d USE_Scanner_AOI_Inspection=%d' % (aec, mag, aoi))
    exp = {}
    for s in ('auto1', 'auto2', 'auto3', 'fix1', 'fix2', 'fix3'):
        exp[s] = True
    if aoi == 2:            # eBtnAOI_TopBottomInstall 的值見下面 INFO；不是 2 時這三站一定顯示
        print('  INFO  USE_Scanner_AOI_Inspection=2：golden 可能藏 Auto3／Fix2／Fix3（視 eBtnAOI_TopBottomInstall），不驗')
        for s in ('auto3', 'fix2', 'fix3'):
            exp.pop(s)
    exp['auto4'] = aec >= 3
    exp['auto5'] = aec >= 3
    exp['auto6'] = aec >= 4
    for i in range(1, 15):
        exp['mag%d' % i] = mag > 0
    if aec < 3:
        for i in range(7, 13):
            exp['fix%d' % i] = False
    bad = ['%s=%r（golden %r）' % (s, v['sort.%s.visible' % s], e) for s, e in exp.items() if v['sort.%s.visible' % s] != e]
    check(not bad, 'V  列可見度＝golden UpForm（:724-766，Gerneral.ini 決定的 %d 站；不符：%s）' % (len(exp), bad))
    f456 = [v['sort.fix%d.visible' % i] for i in (4, 5, 6)]
    check(len(set(f456)) == 1, 'V  Fix4-6 可見度一致（同一個 TrayForm.iFixTrayMode 條件，:776-778）：%s' % f456)
    print('  INFO  Fix4-6 visible=%r、Bulk Box visible=%r（iFixTrayMode／iHWFix_BinBox 來自配方，不在本探針驗）' % (f456[0], v['sort.bulkbox.visible']))
    check(isinstance(v['sort.art.tabVisible'], bool) and isinstance(v['sort.ic.tabVisible'], bool),
          'V  分頁可見度有值：ART=%r（FormShow :196-204）IC Count=%r（:207）caption=%r' % (
              v['sort.art.tabVisible'], v['sort.ic.tabVisible'], v['sort.art.tabCaption']))


def verify_dom(cdp, t, label):
    v = {k: t[k][1] for k in t}
    d = dom_state(cdp)
    bad = ['%s=%r（tag %s=%r）' % (TEXT[k], d['text'].get(k), k, v[k]) for k in TEXT if d['text'].get(k) != fmt(k, v[k])]
    check(not bad, 'D  %s：%d 個文字格＝tag（不符 %d：%s）' % (label, len(TEXT), len(bad), bad[:6]))
    bad = [s for s in ST if d['rows'][s] != (v['sort.%s.visible' % s] is not False) or d['artrows'][s] != d['rows'][s]]
    check(not bad, 'D  %s：33 列（SortCount＋ART）顯示＝sort.<站>.visible（不符：%s）' % (label, bad))
    check(d['yieldShown'] == (v['sort.yield.visible'] is not False), 'D  %s：pnlYield 顯示=%r ＝ sort.yield.visible=%r' % (label, d['yieldShown'], v['sort.yield.visible']))
    check(d['artTab'] == (v['sort.art.tabVisible'] is not False) and d['icTab'] == (v['sort.ic.tabVisible'] is not False),
          'D  %s：ART 分頁顯示=%r（tag %r）、IC Count 分頁顯示=%r（tag %r）' % (label, d['artTab'], v['sort.art.tabVisible'], d['icTab'], v['sort.ic.tabVisible']))
    if isinstance(v['sort.art.tabCaption'], str) and v['sort.art.tabCaption']:
        check(d['artTabText'] == v['sort.art.tabCaption'], 'D  %s：ART 分頁標題=%r ＝ tag %r' % (label, d['artTabText'], v['sort.art.tabCaption']))
    return d


def qty_files():
    out = []
    for root, _, files in os.walk(QTY):
        out += [os.path.join(root, f) for f in files]
    return sorted(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9338)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--no-clear', action='store_true', help='不按 Clear（不寫 lastdata.dat）')
    a = ap.parse_args()

    L = layout()
    ld0 = read_lastdata(L)
    # Steven 20260926 (RULINGS S45)：LAST_GENERAL_SET 補上 V912 檔尾 iBinBaseRT[256]＋bO25_RTBaselined（1032 bytes）。
    #   S45 之前寫的舊檔短 1032 bytes；golden ReadLastDataFile 不看讀到幾 bytes（cprod.cpp:1603-1690），只填前段，
    #   本探針用的欄位都在前段，偏移照樣可信。第一次 WriteLastDataFile 之後檔長才會等於 sizeof。
    check(ld0['size'] in (L['size'], L['size'] - 1032), 'S  lastdata.dat 大小 %d ＝ sizeof(LAST_GENERAL_SET) %d 或 S45 之前的舊檔長（偏移可信）' % (ld0['size'], L['size']))
    print('  INFO  LastSet 偏移：SendCT=%d SendCT_ART=%d BinCT=%d BinCT_ART=%d iIndexInputOutPut=%d（sizeof long=%d）' % (
        L['SendCT'], L['SendCT_ART'], L['BinCT'], L['BinCT_ART'], L['iIndexInputOutPut'], L['sizeofLong']))
    gen = read_general()

    # ---- F：HTML 原檔沒有假數字 ------------------------------------------------
    html = open(PAGE_HTML, encoding='utf-8').read()
    cells = re.findall(r'<span class="[pv]" id="(\w+)">([^<]*)</span>', html)
    notdash = [(i, c) for i, c in cells if c != '---']
    check(len(cells) == 3 + 3 + 3 + 33 * 4 and not notdash,
          'F  Data.SortCT.html 原檔 %d 個值格都是 "---"（非 ---：%s）' % (len(cells), notdash[:5]))
    body = re.sub(r'<!--.*?-->', '', html, flags=re.S)
    hits = [p for p in FAKE if re.search(p, body)]
    check(not hits, 'F  原檔掃不到舊展示值（命中：%s）' % hits)
    check('ht9045_wire_datasortct.js' not in body and 'ht9045_sortct_wire.js' in body,
          'F  頁面載入手寫的 ht9045_sortct_wire.js（不再載入 gen_wire 產生的 8-tag 版）')

    if a.user and a.password:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/Data.SortCT.html' % a.port})
        time.sleep(0.5)
        ok = wait_js(cdp, "window.HT9045Tags && HT9045Tags.status().connected && HT9045Tags.has('sort.mag14.visible')"
                          " && HT9045Tags.has('sort.total') && window.HT9045SortCT", 60)
        check(bool(ok), 'T  開頁、tag 串流已連上且收到 sort.mag14.visible')
        if not ok:
            return len(FAILS)
        cfg = sorted(json.loads(cdp.eval("JSON.stringify(Object.keys(HT9045Page.cfg.tags))")))
        check(cfg == sorted(ALL_TAGS), 'T  接線檔註冊 %d 個 tag ＝ 預期 %d 個（多：%s 少：%s）' % (
            len(cfg), len(ALL_TAGS), sorted(set(cfg) - set(ALL_TAGS))[:5], sorted(set(ALL_TAGS) - set(cfg))[:5]))
        time.sleep(1.2)                                          # 等一個完整快照（500 ms 一拍）
        t0 = tags(cdp, ALL_TAGS + UNBOUND)
        missing = [k for k in ALL_TAGS if not t0[k][0]]
        check(not missing, 'T  178 個 sort.* 都在線上快照（缺：%s）' % missing[:8])

        verify_sources(t0, ld0, '開機')
        verify_visibility(t0, gen)
        verify_dom(cdp, t0, '開機')

        # ---- C：Clear Count --------------------------------------------------
        if a.no_clear:
            print('  SKIP  C：--no-clear')
        else:
            q0 = qty_files()
            r1 = json.loads(cdp.eval("HT9045SortCT.clear({answer:false}).then(function(r){return JSON.stringify(r);})", timeout=60))
            print('  INFO  Clear 詢問段（取消）：%s' % json.dumps({k: r1.get(k) for k in ('executed', 'guard', 'needConfirm', 'asked', 'exitCode', 'confirmReached', 'detail')}, ensure_ascii=False))
            ld1 = read_lastdata(L)
            if r1.get('guard') == 'unknown-action' or 'unknown cmd' in (r1.get('detail') or ''):
                print('  SKIP  C：act.sortCT.clearCount 的伺服器分派還沒接（整合者加一行，見 WebSortCT.cpp 檔頭）')
                st = dom_state(cdp)['status']
                check('分派' in st and '沒有清除' in st, 'C  分派未接時頁面明講沒有清除：%r' % st)
                check(ld1['raw'] == ld0['raw'], 'C  分派未接時 lastdata.dat 一個位元組都沒變')
            elif r1.get('executed'):
                print('  INFO  詢問段就執行完了（golden 不問確認的客戶，例 CC_Greatek :635）—— 取消段不適用')
            else:
                check(r1.get('asked') is True and r1.get('executed') is False,
                      'C  詢問段：走到 golden 的確認框（:637）、按取消 → 沒有執行（guard=%r）' % r1.get('guard'))
                check(ld1['raw'] == ld0['raw'], 'C  取消後 lastdata.dat 一個位元組都沒變')
                time.sleep(0.45)   # AI(W906-CMDGUARD) 20260926: 同一個 phase-1（confirmed:false）payload 在 400 ms 內重送會被 wb_serve WebCmdGuard 回 busy（S107-3）
                r2 = json.loads(cdp.eval("HT9045SortCT.clear({answer:true}).then(function(r){return JSON.stringify(r);})", timeout=60))
                print('  INFO  Clear 確認段：%s' % json.dumps({k: r2.get(k) for k in ('executed', 'guard', 'before', 'after', 'exitCode', 'detail')}, ensure_ascii=False))
                check(r2.get('executed') is True, 'C  確認段 executed（golden btnClearCountClick 跑完；guard=%r）' % r2.get('guard'))
                time.sleep(0.3)
                ld2 = read_lastdata(L)
                # Steven 20260926 (S45)：S45 之前的舊檔（短 1032 bytes）第一次被 WriteLastDataFile 寫就會延長到 sizeof（OPEN_EXISTING 不截斷，golden 同）
                size_ok = ld2['size'] == ld0['size'] or (ld0['size'] == L['size'] - 1032 and ld2['size'] == L['size'])
                check(size_ok and ld2['LastOpenFilename'] == ld0['LastOpenFilename'],
                      'C  lastdata.dat 大小與 LastOpenFilename 不變（%r）' % ld2['LastOpenFilename'])
                z = {'SendCT[0..3]': ld2['SendCT'], 'iIndexInputOutPut[0..3]': ld2['iIndexInputOutPut'],
                     'BinCT[0][0..32]': ld2['BinCT'][0][:33], 'BinCT[2][0..32]': ld2['BinCT'][2][:33],
                     'BinCT[3][0..32]': ld2['BinCT'][3][:33]}
                bad = [k for k, arr in z.items() if any(arr)]
                check(not bad, 'C  lastdata.dat：Clarn_Data(8) 的 ctLoadingCounts／ctTraySortCount／ctIndexCount 欄位歸零（沒歸零：%s）' % bad)
                before_nz = [k for k, arr in {'SendCT': ld0['SendCT'], 'BinCT0': ld0['BinCT'][0][:33]}.items() if any(arr)]
                print('  INFO  清除前非零：%s（全零時「歸零」驗不出差別）' % (before_nz or '無'))
                upd = wait_js(cdp, "HT9045Tags.get('sort.loading')===0 && HT9045Tags.get('sort.total')===0", 15)
                check(bool(upd), 'C  15 秒內 tag 更新：sort.loading=%r sort.total=%r' % (
                    cdp.eval("HT9045Tags.get('sort.loading')"), cdp.eval("HT9045Tags.get('sort.total')")))
                time.sleep(1.2)
                t2 = tags(cdp, ALL_TAGS + UNBOUND)
                verify_sources(t2, ld2, '清除後')
                verify_dom(cdp, t2, '清除後')
                ys = [s for s in ST if t2['sort.%s.count' % s][1] is not None and t2['sort.%s.yield' % s][1] != '0.00%']
                check(not ys, 'C  清除後有設定的站 yield 全是 "0.00%%"（Sum==0，golden ChangeToPercentage；不符：%s）' % ys)
                q2 = qty_files()
                new = sorted(set(q2) - set(q0))
                print('  INFO  QtyData 新檔：%s（golden Clarn_Data 的 static FileName：本行程第一次呼叫不寫）' % (new or '無'))
                check(not new, 'C  本行程第一次 Clarn_Data 不寫 QtyLog（golden main.cpp:15471 FileName=="" 跳過）')
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)
    print('data_sortct_probe: %s' % ('OK' if not FAILS else '%d FAILURE(S)' % len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
