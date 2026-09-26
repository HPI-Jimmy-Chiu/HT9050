# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/data_contactct_probe.py -- Data.ContactCT.html（golden V912 cContactCT.cpp TfContactCT）
#  的 e2e probe。
#
#  Steven 團隊 20260925.
#
#  用真的瀏覽器（headless Edge ＋ DevTools 協定）開頁面，驗：
#    F  頁面沒有舊的假資料（inline var DATA、96.76% 等字樣）
#    G  畫面格子（文字／bg／fg）＝ contactct.get 回應；rgYieldType 選項＝回應，開頁選在 golden FormShow 的 3（Kind(%)）
#    V  回應的數值＝直接照 golden 公式從 D:\HT9045\system\Arm*.dat／ArmHis*.dat 算出來的值
#       （TArm::ReadFile cSocket.cpp:537 的讀法 ＋ ReturnSiteData golden :317-518 ＋ sgYieldDrawCell :148-307）
#    S  切 rgYieldType 會重取（loads 增加、itemIndex 跟著變、值換成該模式）
#    C  Count Clear 按了只顯示「尚未接」；整段期間 Arm*.dat SHA256 不變
#
#  只讀，不寫任何檔。
#  用法：
#      set W906_PWBOOK_PATH=<scratch book>  &  build\wb_serve.exe --allow-cmd --root D:\HT9045\web --port 8046
#      python tools\webprobe\data_contactct_probe.py --port 8046 [--user S12TEST --password S12PW]
#  Exit 0 = 全部通過；非 0 = 失敗項目數。
# =============================================================================
import argparse
import glob
import hashlib
import json
import os
import shutil
import struct
import sys
import time
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from s12_form_probe import Cdp, launch_edge, ws_login          # noqa: E402
from s12c_config_probe import wait_js                           # noqa: E402

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
FAILS = []
PAGE = 'Data.ContactCT.html'
SYSDIR = r'D:\HT9045\system'
MAX_ROW, MAX_COL = 4, 8        # MachineType.h:485-486 MAX_SOCKET_ROW／MAX_SOCKET_COL

# MachineType.h:561 eTestMode
(SingleSite, DualSite, TriSite1X3, QualSite1X4, DualSite2x1, QualSite2X2, QualSite2X2N, _6Site2X3,
 _6Site2X3N, _8Site2X4, _8Site2X4N, _10Site2X5, _12Site2X6, _16Site2X8, _16Site4X4, _32Site4X8N,
 _32Site4X8M, _8Site1X4) = range(18)


def check(ok, what):
    print(('  PASS  ' if ok else '  FAIL  ') + what)
    if not ok:
        FAILS.append(what)


def sha_arm():
    out = {}
    for p in sorted(glob.glob(os.path.join(SYSDIR, 'Arm*.dat'))):
        out[os.path.basename(p)] = hashlib.sha256(open(p, 'rb').read()).hexdigest()
    return out


# ---- golden TArm::ReadFile（cSocket.cpp:537-735）只取 Pass／Fail ---------------------------------------
def read_arm(name):
    main = os.path.join(SYSDIR, name + '.dat')
    back = os.path.join(SYSDIR, name + '_backup.dat')
    if not os.path.exists(main):
        return None                                  # bHasFile=false -> 全 0
    data = open(main, 'rb').read()
    if os.path.exists(back) and open(back, 'rb').read() != data:
        data = open(back, 'rb').read()               # golden :572-594 檔案與備份不同 -> 讀備份
    size = len(data)
    if size == 2432:
        per = 19
    elif size == 12800:
        per = 100
    elif size == 13312:
        per = 104
    else:
        per = size // (MAX_ROW * MAX_COL * 4)        # tempMax4[4][8][TEST_MAX_BIN+4]
    v = struct.unpack('<%dI' % (len(data) // 4), data[:len(data) // 4 * 4])
    pf = {}
    for i in range(MAX_ROW):
        for j in range(MAX_COL):
            b = (i * MAX_COL + j) * per
            pf[(i, j)] = (v[b], v[b + 1])            # SetPassCT(temp[..][0]) / SetFailCT(temp[..][1])
    return pf


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


def pca(p, f):
    # TMySocket::GetPCA = ChangeToFloat(Pass, Total) -> float（MachineType.h:1721，回傳型別 float）
    t = p + f
    return f32((p / t) * 100.0) if t else 0.0


def site_ij(mode, arow):
    """ReturnSiteData golden :317-384 的 i/j。"""
    r = arow - 1
    if mode in (DualSite2x1, QualSite2X2, QualSite2X2N):
        return r // 2, r % 2
    if mode in (_6Site2X3, _6Site2X3N):
        return r // 3, r % 3
    if mode in (_16Site4X4, _8Site2X4N):
        return r // 4, r % 4
    if mode == _10Site2X5:
        return r // 5, r % 5
    if mode == _12Site2X6:
        return r // 6, r % 6
    if mode in (_16Site2X8, _32Site4X8N, _32Site4X8M):
        return r // 8, r % 8
    if not (mode <= QualSite1X4 or mode == _8Site1X4):   # TestSite2X2Mode golden :309-315
        return r // 4, r % 4
    return 0, r


def rows_for(mode):
    """ShowFormComp golden :97-129。"""
    return {SingleSite: 2, DualSite: 3, DualSite2x1: 3, QualSite2X2N: 3, TriSite1X3: 4, _6Site2X3N: 4,
            QualSite1X4: 5, QualSite2X2: 5, _8Site1X4: 5, _8Site2X4N: 5, _6Site2X3: 7, _10Site2X5: 11,
            _12Site2X6: 13, _16Site2X8: 17, _16Site4X4: 9, _32Site4X8N: 17, _32Site4X8M: 33}.get(mode, 9)


def expect_text(arms, his, idx, arm, i, j, d):
    """ReturnSiteData golden :386-518，只做 idx 0..4（ByBin 系列要 Arm*.ini 與 bLowYieldAlarmByBin，這裡不驗）。"""
    def get(src, a):
        return (src[a] or {}).get((i, j), (0, 0)) if src[a] is not None else (0, 0)
    if arm == 2:
        if idx in (0, 1, 2):
            src = his if idx == 0 else arms
            if idx == 2:
                return str(get(src, 0)[0] + get(src, 1)[0])
            return str(sum(get(src, 0)) + sum(get(src, 1)))
        a = pca(*get(arms, 0))
        b = pca(*get(arms, 1))
        if d['shuttleMode'] == 0:
            return '%3.2f%%' % ((a + b) / 2)
        return '%3.2f%%' % (a if d['shuttleSel'] == 0 else b)
    if idx == 0:
        return str(sum(get(his, arm)))
    if idx == 1:
        return str(sum(get(arms, arm)))
    if idx == 2:
        return str(get(arms, arm)[0])
    return '%3.2f%%' % pca(*get(arms, arm))       # idx 3 GetPCA／idx 4 GetBySitePCA（開機讀檔後兩者同值，SetPassCT／SetFailCT）


def verify_values(d, arms, his, label):
    idx = d['rgYieldType']['itemIndex']
    mode = d['testMode']
    cells = d['cells']
    if not d.get('initialOK'):
        check(False, '%s：C++ InitialOK=true（否則 golden sgYieldDrawCell 開頭就 return）' % label)
        return
    check(d['rows'] == rows_for(mode) and len(cells) == d['rows'],
          '%s：列數＝golden ShowFormComp（testMode=%d -> %d，回應 %d）' % (label, mode, rows_for(mode), d['rows']))
    if d['twoArm32Site'] or d['nnMode'] != 0:
        print('  NOTE  twoArm32Site／NN 模式的欄位配置不同，本探針只驗一般配置（Arm1｜Arm2｜Sum）')
        return
    hdr = [c['text'] for c in cells[0]]
    check(hdr[1:] == ['Arm 1', 'Arm 2', 'Sum'], '%s：表頭＝golden :186-194（%s）' % (label, hdr))
    bad, shown = [], 0
    for r in range(1, d['rows']):
        i, j = site_ij(mode, r)
        lab = '%c%c' % (ord('A') + int((r - 1) / d['shtCol']), ord('a') + (r - 1) % d['shtCol'])
        if cells[r][0]['text'] != lab:
            bad.append('r%d c0 %r!=%r' % (r, cells[r][0]['text'], lab))
        red = d['showSiteYield'][(0 if mode == _32Site4X8N else r - 1)] if r < 16 else d['showSiteYield'][0]
        for c, arm in ((1, 0), (2, 1), (3, 2)):
            want = expect_text(arms, his, idx, arm, i, j, d)
            cell = cells[r][c]
            if cell['text'] != want:
                bad.append('r%d c%d %r!=%r' % (r, c, cell['text'], want))
            wbg = 'clInfoBk' if c == 3 else 'clWhite'
            wfg = 'clRed' if red else 'clBlack'
            if cell['bg'] != wbg or cell['fg'] != wfg or not cell['drawn']:
                bad.append('r%d c%d colour %s/%s' % (r, c, cell['bg'], cell['fg']))
            if shown < 3 and (i, j) in ((0, 0), (0, 1), (1, 0)) and c in (1, 3):
                src = his if idx == 0 else arms
                pa = (src[0] or {}).get((i, j), (0, 0)) if src[0] is not None else (0, 0)
                pb = (src[1] or {}).get((i, j), (0, 0)) if src[1] is not None else (0, 0)
                print('  手算  %s 列%d（site[%d][%d]）col%d：%s Arm0 P/F=%s Arm1 P/F=%s -> %s（畫面 %s）'
                      % (label, r, i, j, c, 'ArmHis*.dat' if idx == 0 else 'Arm*.dat', pa, pb, want, cell['text']))
                shown += 1
    check(not bad, '%s：%d 格值／顏色＝golden 公式從 .dat 算出%s' % (label, (d['rows'] - 1) * 4, ('' if not bad else '：' + '; '.join(bad[:6]))))


def dom_grid(cdp):
    return json.loads(cdp.eval(
        "(function(){var t=document.getElementById('sgYield'),o=[];"
        "t.querySelectorAll('tr').forEach(function(tr){var r=[];tr.querySelectorAll('td').forEach(function(td){"
        "r.push({text:td.textContent,bg:td.getAttribute('data-bg'),fg:td.getAttribute('data-fg')});});o.push(r);});"
        "var rs=[].map.call(document.querySelectorAll('#rgYieldType label'),function(l){"
        "var i=l.querySelector('input');return {text:l.textContent.trim(),checked:i.checked};});"
        "return JSON.stringify({grid:o,radios:rs,status:document.getElementById('ctStatus').textContent});})()"))


def compare_dom(cdp, d, label):
    g = dom_grid(cdp)
    want = [[{'text': c['text'], 'bg': c['bg'], 'fg': c['fg']} for c in row] for row in d['cells']]
    check(g['grid'] == want, '%s：畫面格子（%d×%d 文字／bg／fg）＝回應' % (label, d['rows'], d['cols']))
    items = d['rgYieldType']['items']
    check([x['text'] for x in g['radios']] == items, '%s：rgYieldType 選項＝回應（%s）' % (label, items))
    chk = [k for k, x in enumerate(g['radios']) if x['checked']]
    check(chk == [d['rgYieldType']['itemIndex']], '%s：選中的選項＝回應 itemIndex %d' % (label, d['rgYieldType']['itemIndex']))
    return g


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--port', type=int, default=8046)
    ap.add_argument('--dbg', type=int, default=9337)
    ap.add_argument('--user')
    ap.add_argument('--password')
    ap.add_argument('--baseline', help='開機前 D:\\HT9045 的 sha256sum 清單（./system/Arm*.dat 那幾行拿來比）')
    a = ap.parse_args()

    sha0 = sha_arm()
    if a.baseline:
        # 開機前的基準（sha256sum 格式，路徑相對 D:\HT9045）：wb_serve 開機（含 ReadCTInfo）沒有改到任何 Arm*.dat
        base = {}
        for line in open(a.baseline, encoding='utf-8', errors='replace'):
            parts = line.rstrip('\r\n').split(None, 1)
            if len(parts) == 2 and parts[1].lstrip('*').replace('\\', '/').startswith('./system/Arm') \
                    and parts[1].endswith('.dat'):
                base[os.path.basename(parts[1].lstrip('*'))] = parts[0]
        check(base == sha0, 'C：開機後 system\\Arm*.dat SHA256 ＝ 開機前基準（%d 檔，基準 %d 檔）' % (len(sha0), len(base)))
    arms =[read_arm('Arm0'), read_arm('Arm1')]
    his = [read_arm('ArmHis0'), read_arm('ArmHis1')]
    nz = sum(1 for s in arms + his if s for v in s.values() if v != (0, 0))
    print('  info  Arm*.dat 檔案數 %d；Arm0/1＋ArmHis0/1 非零 site 數 %d' % (len(sha0), nz))

    if a.user:
        check(bool(ws_login(a.port, a.user, a.password)), '登入 %s' % a.user)

    src = urllib.request.urlopen('http://127.0.0.1:%d/page/%s' % (a.port, PAGE), timeout=10).read().decode('utf-8')
    fake = [s for s in ('var DATA', '96.76%', '99.68%', 'Lot-03', "value=\"kindpct\"") if s in src]
    check(not fake, 'F：頁面原始碼沒有假資料（找到：%s）' % fake)
    check('ht9045_contactct_wire.js' in src and 'ht9045_recipe_client.js' in src, 'F：頁面載入 recipe_client＋contactct_wire')

    edge, prof, ws = launch_edge(a.dbg)
    try:
        cdp = Cdp(ws)
        cdp.call('Runtime.enable')
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (a.port, PAGE)})
        ok = wait_js(cdp, "window.__contactct && window.__contactct.loads>=1 && (window.__contactct.data||window.__contactct.error) ? 1 : 0", 60)
        check(bool(ok), '開頁送出 contactct.get 並收到回應')
        st = json.loads(cdp.eval("JSON.stringify(window.__contactct||{})"))
        if st.get('error') or not st.get('data'):
            check(False, 'contactct.get 成功（%s）' % st.get('error'))
            return len(FAILS)
        d = st['data']
        print('  info  testMode=%d rows=%d cols=%d shtRow=%d shtCol=%d shuttleMode=%d initialOK=%s items=%s'
              % (d['testMode'], d['rows'], d['cols'], d['shtRow'], d['shtCol'], d['shuttleMode'],
                 d['initialOK'], d['rgYieldType']['items']))
        items = d['rgYieldType']['items']
        base = ['History', 'Total', 'Kind', 'Kind(%)', 'ByHeadYield']
        bybin = ['ByBinPass', 'ByBinPass(%)', 'ByBinArmPass', 'ByBinArmPass(%)', 'ByBinSitePass', 'ByBinSitePass(%)']
        rest = items[5:]
        check(items[:5] == base and rest in ([], bybin, bybin + ['IntervalYield(%)'], ['IntervalYield(%)']),
              'G：選項＝golden FormShow :43-72（History…ByHeadYield［＋ByBin 六項］［＋IntervalYield(%)］）')
        check(d['rgYieldType']['itemIndex'] == 3, 'G：開頁 itemIndex＝golden FormShow :73 的 3（Kind(%)）')
        check(not d.get('errors'), 'G：沒有繪製例外（%s）' % d.get('errors'))
        compare_dom(cdp, d, '開頁')
        verify_values(d, arms, his, '開頁 Kind(%)')

        loads = st['loads']
        for idx in (0, 1, 2, 4, 3):
            if idx >= len(items):
                continue
            cdp.eval("(function(){var r=document.querySelectorAll('#rgYieldType input')[%d]; r.click(); return 1;})()" % idx)
            ok = wait_js(cdp, "window.__contactct && window.__contactct.loads>%d && window.__contactct.data "
                              "&& window.__contactct.data.rgYieldType.itemIndex==%d ? 1 : 0" % (loads, idx), 30)
            st = json.loads(cdp.eval("JSON.stringify(window.__contactct||{})"))
            check(bool(ok), 'S：點 %s -> 重取 contactct.get（loads %d -> %s，itemIndex=%s）'
                  % (items[idx], loads, st.get('loads'), (st.get('data') or {}).get('rgYieldType', {}).get('itemIndex')))
            loads = st.get('loads', loads)
            if ok:
                compare_dom(cdp, st['data'], items[idx])
                verify_values(st['data'], arms, his, items[idx])

        cdp.eval("document.getElementById('btClearCount').click()")
        time.sleep(0.5)
        g = dom_grid(cdp)
        check('尚未接' in g['status'], 'C：Count Clear 顯示「尚未接」（%s）' % g['status'][:60])
        st2 = json.loads(cdp.eval("JSON.stringify({loads:(window.__contactct||{}).loads})"))
        check(st2['loads'] == loads, 'C：按 Count Clear 沒有送任何指令（loads 不變）')
    finally:
        edge.kill()
        shutil.rmtree(prof, ignore_errors=True)

    sha1 = sha_arm()
    check(sha0 == sha1, 'C：探針期間 system\\Arm*.dat SHA256 全部不變（%d 檔）' % len(sha0))
    print('\n%s：%d 項失敗' % ('PASS' if not FAILS else 'FAIL', len(FAILS)))
    for f in FAILS:
        print('   - ' + f)
    return len(FAILS)


if __name__ == '__main__':
    sys.exit(main())
