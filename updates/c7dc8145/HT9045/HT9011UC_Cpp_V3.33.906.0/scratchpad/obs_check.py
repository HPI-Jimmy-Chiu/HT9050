# -*- coding: utf-8 -*-
"""headless Edge 連跑 N 次 Data.Observer.html，逐格報告「有值 / --- / 殘留假值」。

⚠ 為什麼要跑多次：--dump-dom 有 timing race，太早 dump 會看到還沒填的初始狀態。
  判定一格「沒接上」之前至少要看 3 次一致的結果。
"""
import io, re, subprocess, sys, collections

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
URL = sys.argv[1] if len(sys.argv) > 1 else 'http://127.0.0.1:8045/page/Data.Observer.html'
N = int(sys.argv[2]) if len(sys.argv) > 2 else 3

VERSION = ['labModel', 'labSerialNo', 'labMachineID', 'labFactory', 'labVersion',
           'labReleaseDate', 'pnlGPIBVersion', 'pnlESDVersion', 'pnlATCVersion',
           'pnlTTLRS232Version']
OPER = ['labPowerOnTime', 'labRunningTime', 'labProductTime', 'labLoadingCount',
        'labMUBA', 'labMTBA', 'labMTBF', 'pnlDayJamRate']
OTHER = ['labDeviceName']
IDS = VERSION + OPER + OTHER

# 模擬器當初填的值（web\JSON\Production-runtime.json 實測）。
# 任何一格出現這些，就是 A 路還活著。
# ⚠ 'LLS062' 不列入：Gerneral.ini [Version] Serial No 的**真值**就是 LLS062，
#   模擬器當初抄對了這一格。把真值當成假值會得到一個永遠修不好的假失敗。
#   同理 'KYEC'/'V3.33' 也是檔案裡真的有的字串，不能拿來當殘留指紋。
SIM = {'HT-9046AT', 'HT9046AT-136',
       '0017 days 11:45:27.480', '0020 days 07:49:08.668',
       '0000 days 00:00:00.000', '0 / 1452 unit', '0 / 00:00:00', '11:45:27'}


def dump():
    return subprocess.run([EDGE, '--headless=new', '--disable-gpu',
                           '--virtual-time-budget=12000', '--dump-dom', URL],
                          capture_output=True, text=True, encoding='utf-8',
                          errors='replace', timeout=120).stdout


def cellof(html, wid):
    """<div class="pnl" id=X ...><span class="pnlCap">VALUE</span></div> 的 VALUE。"""
    m = re.search(r'<div[^>]*\bid="' + wid + r'"[^>]*>(.*?)</div>', html, re.S)
    if not m:
        return None
    inner = m.group(1)
    c = re.search(r'<span[^>]*class="[^"]*pnlCap[^"]*"[^>]*>(.*?)</span>', inner, re.S)
    txt = (c.group(1) if c else inner)
    return re.sub(r'<[^>]+>', '', txt).strip()


runs = []
for i in range(N):
    h = dump()
    runs.append(h)
    print('--- run %d：%d bytes ---' % (i + 1, len(h)))

print()
print('%-22s %s' % ('element', ' | '.join('run%d' % (i + 1) for i in range(N))))
bad = []
for wid in IDS:
    vals = [cellof(h, wid) for h in runs]
    shown = ' | '.join(('(找不到元素)' if v is None else (v if v else '(空)')) for v in vals)
    print('%-22s %s' % (wid, shown))
    for v in vals:
        if v in SIM:
            bad.append((wid, v))

print()
# 全頁掃殘留假值
for i, h in enumerate(runs):
    # ⚠ 先剝掉 HTML 註解與 <script> 內容再掃：本頁的說明註解**刻意引用**了
    #   模擬器的舊值當作「移掉的是什麼」的證據，渲染器的 JS 裡也有 '---' 的
    #   字串字面值。不剝掉的話，自己的文件會把自己判成回歸。
    vis = re.sub(r'<!--.*?-->', '', h, flags=re.S)
    vis = re.sub(r'<script\b.*?</script>', '', vis, flags=re.S)
    hit = sorted(set(x for x in SIM if x in vis))
    if hit:
        bad.append(('(全頁) run%d' % (i + 1), ', '.join(hit)))

print('=== 模擬器假值殘留檢查 ===')
if bad:
    for w, v in bad:
        print('  ❌ %s -> %s' % (w, v))
else:
    print('  ✅ 三次 dump 都沒有任何模擬器假值')

print()
print('=== 其他檢查 ===')
h0 = runs[-1]
print('  settings.js script 引用     :', 'STILL THERE ❌' if '<script src="settings.js"></script>' in h0 else '已移除 ✅')
print('  HTSettings. 呼叫            :', 'STILL THERE ❌' if 'HTSettings.' in h0 else '已移除 ✅')
print('  ht9045_wire_dataobserver.js :', '已載入 ✅' if 'ht9045_wire_dataobserver.js' in h0 else 'MISSING ❌')
print('  ccval 寫死的 0              :', h0.count('<td class="ccval">0</td>'), '（應為 0）')
print('  ccval 顯示 ---              :', h0.count('<td class="ccval">---</td>'), '（應為 32）')
print('  evGrid 捏造事件列           :', len(re.findall(r'<tr data-type=', h0)), '（應為 0）')
print('  tcGrid 的 --- 格            :', h0.count('<td class="val">---</td>'), '（應為 80 = 5 列 x 16 欄）')
