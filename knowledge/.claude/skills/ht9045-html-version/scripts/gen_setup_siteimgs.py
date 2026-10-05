# -*- coding: utf-8 -*-
r"""gen_setup_siteimgs.py -- Set Up 畫面 Site Mode 示意圖 bmp -> png

//Steven 20260921
當日完整變更紀錄：<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260921_Steven.md

golden cSetUp.cpp 的 Image1 是**執行期**換圖的：
    Image1->Picture->LoadFromFile(BmpPath + TestSiteFileName[1][iCheckPos]);
（BmpPath = D:\HT9045\IMG\BMP\），外加 chkOffCenterkitClick /
cbQualSite2X2ShiftClick 的 4 張偏心圖。

_gen_dfm_abs.py 的 IMG_MAP 一個 TImage 只能配一張圖（它配的是
8siteCenterX.bmp，等於「機台目前是 8-Site」那一張），所以換模式要用到的
另外 21 張根本沒被轉出來。這支把它們補齊。

檔名規則刻意與 _gen_dfm_abs.py::img_html 完全一致：
    'dfm_' + bmp 主檔名（空白換底線） + '.png'
這樣兩邊產出的檔名不會打架，將來若把某張搬進 IMG_MAP 也不必改 HTML。

⚠ 輸出目錄是 web\page\img（wb_serve 的 WEBROOT 底下那棵），
  不是 _gen_dfm_abs.py 預設的 D:\HT9045\page —— 後者已退場。

重跑：
  <python314> D:\HT9045\HT9011UC_Cpp_V3.33.906.0\scratchpad\gen_setup_siteimgs.py
"""
import os
import sys

from PIL import Image

BMP = r'D:\HT9045\IMG\BMP'
OUT = os.environ.get('HT9045_SITEIMG_OUT') or r'D:\HT9045\web\page\img'

# golden cmydef.cpp TestSiteFileName[1][] 的 18 張（依 eTestMode 順序），
# 加上 cSetUp.cpp 裡另外 LoadFromFile 的 4 張。
FILES = [
    # TestSiteFileName[1][SingleSite .. _8Site1X4]
    '1site.bmp', '2site.bmp', '3site.bmp', '4site.bmp', '2site2x1.bmp',
    '4siteRow.bmp', '2x2siteRow_NN.bmp', '6Site.bmp', '2x3site_NN.bmp',
    '8Site.bmp', '2x4site_NN.bmp', '10Site.bmp', '12Site.bmp', '16Site.bmp',
    '16Site4X4.bmp', '32SiteN.bmp', '32SiteM.bmp', '8-Site Pop.bmp',
    # chkOffCenterkitClick / cbQualSite2X2ShiftClick
    '1siteOffCentre.bmp', '2siteOffCentre.bmp', '4siteOffCentre.bmp',
    '4siteRowOffCentre.bmp',
    # chkOffCenterkitClick（b2x4SupportCenterPitch 那條）
    '8siteCenterX.bmp',
]


def resolve(name):
    """Windows 檔名不分大小寫，golden 的表和實體檔常常對不起來
    （表寫 16Site.bmp、磁碟上是 16site.bmp）。這裡自己做一次不分大小寫比對，
    免得在大小寫敏感的環境重跑時整批失敗而沒人發現。"""
    p = os.path.join(BMP, name)
    if os.path.exists(p):
        return p
    low = name.lower()
    for f in os.listdir(BMP):
        if f.lower() == low:
            return os.path.join(BMP, f)
    return None


def main():
    os.makedirs(OUT, exist_ok=True)
    made, skipped = 0, []
    for name in FILES:
        src = resolve(name)
        if not src:
            skipped.append(name)
            continue
        png = 'dfm_' + os.path.splitext(name)[0].replace(' ', '_') + '.png'
        dst = os.path.join(OUT, png)
        Image.open(src).convert('RGBA').save(dst)
        made += 1
        print('  %-26s -> %s' % (name, png))
    print('wrote %d png into %s' % (made, OUT))
    if skipped:
        # 不當成錯誤：golden 的表列了機種不一定裝的圖（例如海思 8-Site Pop）。
        # 少一張的後果是該模式沿用前一張圖，頁面仍可用 —— 但要講出來。
        print('NOT FOUND in %s (該模式會沿用上一張圖): %s' % (BMP, ', '.join(skipped)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
