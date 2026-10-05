# -*- coding: utf-8 -*-
"""gen_contact_slk_imgs.py -- Setup.Contact.html 的 imgSLK 換圖素材

Steven 20260921
當日完整變更紀錄：<入口網站 repo>\\public\\Docs\\ChangeLog\\Steven\\CHANGES_20260921_Steven.md

golden cContact.cpp:2080 DutCount()：
    asStr.sprintf("%sContact%d.bmp", BmpPath, scrbSLK->Position);
    imgSLK->Picture->LoadFromFile(asStr);

scrbSLK 的值域是 2..6，所以要 Contact2..Contact6 五張。_gen_dfm_abs.py 的
EXT_IMG 表只掛了 imgSLK 的起始圖 Contact2.bmp（dfm 裡就只有那一張），
其餘四張沒有人轉過 —— 這支補上。

去背規則與 _gen_dfm_abs.py 的 bmp_debg() 完全一樣（左下角像素色轉透明，
即 VCL 的 Transparent 規則），不要各寫一套。

    py -3.14 gen_contact_slk_imgs.py
"""
import os
from PIL import Image

BMP = r'D:\HT9045\IMG\BMP'
# 部署樹。client\ 沒有 img\，圖只住部署樹。見 memory/ht9045-web-deploy-direction
OUT = r'D:\HT9045\web\page\img'

# scrbSLK 的 Min=2、Max=5（HT9046 / HT9045_12Site 是 6），所以 2..6
POSITIONS = (2, 3, 4, 5, 6)


def bmp_debg(im):
    """BMP 去背：左下角像素色轉透明（同 _gen_dfm_abs.py / VCL Transparent 規則）"""
    im = im.convert('RGBA')
    tc = im.getpixel((0, im.height - 1))[:3]
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            if px[x, y][:3] == tc:
                px[x, y] = (0, 0, 0, 0)
    return im


def main():
    os.makedirs(OUT, exist_ok=True)
    missing = []
    for n in POSITIONS:
        src = os.path.join(BMP, 'Contact%d.bmp' % n)
        if not os.path.exists(src):
            missing.append(src)
            continue
        dst = os.path.join(OUT, 'dfm_Contact%d.png' % n)
        im = bmp_debg(Image.open(src))
        im.save(dst)
        print('img:', src, '->', dst, '%dx%d' % im.size)
    if missing:
        # 不靜默跳過 —— 少一張圖，換到那個 Position 就會是破圖
        print('NOT FOUND:')
        for m in missing:
            print('   ', m)


if __name__ == '__main__':
    main()
