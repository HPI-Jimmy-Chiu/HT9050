# -*- coding: utf-8 -*-
r"""掃描 D:\HT9045\IMG\ScreenShot：針對「尚未 dfm 轉換」的每個表單各產生一頁
page/shot/<Form>.html（截圖＋variant 頁籤＋警示），並輸出
page/shot_windows.js（window.SHOT_WINDOWS，桌面視窗定義）供 background.html 併入"""
import os, re, shutil, json, struct

SRC = r'D:\HT9045\IMG\ScreenShot'
MAN = r'D:\HT9045'
IMG = os.path.join(MAN, 'page', 'img', 'shot')
SHOT = os.path.join(MAN, 'page', 'shot')
os.makedirs(IMG, exist_ok=True)
os.makedirs(SHOT, exist_ok=True)

# 已有 HTML 模擬頁的表單（跳過）
SKIP = {'fMain', 'MainForm', 'MotionView', 'MotorView', 'fOffSet', 'fSpeed', 'HandlerSystem', 'HandlerSys',
        'fContactCT', 'fObserver', 'fLotInfo', 'fTestCategory', 'fBinSel', 'SortCount',
        'fCounterSel', 'fCounterClear', 'fBuilder', 'fDIOFrom', 'fLtcSensor',
        'fTowerLight', 'fOmron', 'fQAMode', 'fBarCode', 'fCCLink', 'fCleaning', 'fContact',
        'FTestIF', 'fGroundMan', 'fLd_ULd', 'fSecurity', 'fTrayForm', 'fSCKART',
        'fYieldMonitoring', 'fHotPlate',
        'fSetup', 'fSmartDiagnostic', 'fStartCondition', 'fTemp_Set', 'fTeach', 'fMotorTest', 'fHome', 'fTrayAssignment'}
# 主畫面下方分頁的零散截圖 → 收成一組
MAINTABS = {'Tab_UPH', 'tsCategoryInfo', 'tsIndex', 'tsLotID', 'tsTestBin'}


def img_size(path):
    """讀 PNG IHDR / JPEG SOF 取得像素尺寸（不依賴 PIL）"""
    with open(path, 'rb') as f:
        head = f.read(26)
        if head[:8] == b'\x89PNG\r\n\x1a\n':
            w, h = struct.unpack('>II', head[16:24])
            return w, h
        f.seek(2)
        while True:
            b = f.read(1)
            if not b: return 0, 0
            if b != b'\xff': continue
            marker = f.read(1)
            if marker in (b'\xc0', b'\xc1', b'\xc2', b'\xc3'):
                f.read(3)
                h, w = struct.unpack('>HH', f.read(4))
                return w, h
            if marker in (b'\xd8', b'\x01') or b'\xd0' <= marker <= b'\xd7': continue
            ln = struct.unpack('>H', f.read(2))[0]
            f.seek(ln - 2, 1)


groups = {}   # slug -> {'title':..., 'items':[{'file','label','w','h'}]}
for fn in sorted(os.listdir(SRC)):
    stem, ext = os.path.splitext(fn)
    if ext.lower() not in ('.png', '.jpg'):
        continue
    form, _, tab = stem.partition('.')
    if form in SKIP:
        continue
    if form in MAINTABS:
        slug, title, label = 'MainTabs', 'Main 下方分頁', stem
    else:
        slug, title, label = re.sub(r'[^A-Za-z0-9_]', '_', form), form, (tab or '(主畫面)')
    w, h = img_size(os.path.join(SRC, fn))
    groups.setdefault(slug, {'title': title, 'items': []})['items'].append(
        {'file': fn, 'label': label, 'w': w, 'h': h})
    shutil.copy2(os.path.join(SRC, fn), os.path.join(IMG, fn))

PAGE = '''<!DOCTYPE html>
<html lang="zh-Hant">
<head>
<meta charset="UTF-8">
<title>{title}（實機截圖・尚未進行 dfm 轉換）</title>
<link rel="stylesheet" href="../theme.css">
<style>
  html,body{{margin:0;height:100%;background:var(--panel,#ece9d8);color:var(--text,#222);
    font-family:var(--font-ui,"Microsoft JhengHei",sans-serif);font-size:12px;}}
  body{{display:flex;flex-direction:column;}}
  .hdr{{flex:none;display:flex;align-items:center;gap:10px;padding:4px 10px;
    background:var(--panel-dk,#d4d0c8);border-bottom:1px solid var(--border,#808080);}}
  .hdr b{{color:var(--navy,#000080);}}
  .badge{{background:#fff3cd;color:#7a4b00;border:1px solid #d9b24c;border-radius:3px;
    padding:1px 8px;font-weight:bold;}}
  .tabs{{flex:none;display:flex;flex-wrap:wrap;gap:3px;padding:4px 8px 3px;
    border-bottom:1px solid var(--gbx-border,#99aaaa);}}
  .vt{{padding:2px 9px;border:1px solid var(--gbx-border,#99aaaa);border-radius:3px 3px 0 0;
    background:var(--tab-bg,#d4d0c8);cursor:pointer;font-size:11px;}}
  .vt.act{{background:var(--tab-act-bg,#fff);font-weight:bold;color:var(--navy,#000080);}}
  .imgbox{{flex:1;overflow:auto;background:#8a97a5;padding:6px;}}
  .imgbox img{{display:block;border:1px solid #345;box-shadow:2px 2px 8px #0005;background:#fff;}}
</style>
</head>
<body>
<div class="hdr"><b>{title}</b><span class="badge">⚠ 尚未進行 dfm 轉換</span>
  <span id="cap" style="color:var(--text-dim,#666);"></span></div>
{tabs}
<div class="imgbox"><img id="img" alt="{title} 實機截圖"></div>
<script src="../theme.js"></script>
<script>
var ITEMS={items};
function sel(i){{
  document.querySelectorAll('.vt').forEach(function(e,j){{e.classList.toggle('act',j===i);}});
  document.getElementById('img').src='../img/shot/'+ITEMS[i].file;
  document.getElementById('cap').textContent=ITEMS[i].label+'　（'+ITEMS[i].file+'）';
}}
document.querySelectorAll('.vt').forEach(function(e,i){{e.addEventListener('click',function(){{sel(i);}});}});
sel(0);
</script>
</body>
</html>
'''

wins = []
n = 0
for slug in sorted(groups):
    g = groups[slug]
    items = g['items']
    tabs = ''
    if len(items) > 1:
        tabs = ('<div class="tabs">' +
                ''.join(f'<span class="vt" title="{it["file"]}">{it["label"]}</span>' for it in items) +
                '</div>')
    html = PAGE.format(title=g['title'], tabs=tabs,
                       items=json.dumps(items, ensure_ascii=False))
    with open(os.path.join(SHOT, slug + '.html'), 'w', encoding='utf-8') as f:
        f.write(html)
    maxw = max(it['w'] for it in items)
    maxh = max(it['h'] for it in items)
    # 視窗＝26 標題 + 26 頁首 + (頁籤列 27) + 圖高/寬 + 捲軸餘裕；上限依 1920×1032 規範
    w = min(maxw + 32, 1560)
    h = min(26 + 26 + (27 if len(items) > 1 else 0) + maxh + 30, 990)
    wins.append({'id': 'shot_' + slug, 'title': g['title'] + '（實機截圖・未轉換）',
                 'src': 'page/shot/' + slug + '.html',
                 'x': 150 + (n % 5) * 40, 'y': 8 + (n % 6) * 14,
                 'w': w, 'h': h, 'hidden': True, 'noTask': True,
                 'form': g['title'], 'count': len(items), 'thumb': items[0]['file']})
    n += 1

with open(os.path.join(MAN, 'page', 'shot_windows.js'), 'w', encoding='utf-8') as f:
    f.write('/* 由 _gen_screenshot_pages.py 產生：未 dfm 轉換表單的截圖視窗定義 */\n')
    f.write('window.SHOT_WINDOWS = ')
    f.write(json.dumps(wins, ensure_ascii=False, indent=1))
    f.write(';\n')

total = sum(len(g['items']) for g in groups.values())
print(f'{len(groups)} pages, {total} images')
for w in wins:
    print(f"  {w['id']}: {w['count']} img, win {w['w']}x{w['h']}")
