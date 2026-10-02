# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。驗證「移除注入的 Save to recipe / Reload 浮動鈕」之後，
# 沒有頁面靜默失去存檔能力。
# 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""savebtn_smoke.py -- 每一頁跑一次，量三件事（從跑過 JS 的 DOM）：

  1. 沒有注入的浮動鈕（#ht9045WireSave、或文字是 'Save to recipe' / 'Reload'
     且帶 position:fixed 的按鈕）
  2. 有可讀寫欄位的頁面，狀態列**不**出現「沒有可用的存檔鈕」
  3. 狀態列沒有紅字（#f88 = 拒接/拒寫）

為什麼要量第 2 項：拿掉注入鈕最大的風險不是畫面，是**靜默唯讀**——
頁面看起來一切正常，只是按不到存檔。單純檢查「按鈕不見了」會 100% 通過
而完全沒有鑑別力。

用法: py -3 savebtn_smoke.py [--port 8045]
"""
import io
import json
import os
import re
import subprocess
import sys

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
PAGE_DIR = r'D:\HT9045\web\page'
CLIENT = r'D:\HT9045\client'
PORT = 8045
for _i, _a in enumerate(sys.argv):
    if _a == '--port' and _i + 1 < len(sys.argv):
        PORT = int(sys.argv[_i + 1])

# 只接 tag、沒有可讀寫欄位的頁面本來就不需要存檔鈕
TAG_ONLY = {'main.html'}


def wired_pages():
    """{page: wirefile}，只取有接線檔的頁面。"""
    out = {}
    for fn in sorted(os.listdir(CLIENT)):
        if not fn.startswith('ht9045_wire_') or not fn.endswith('.js'):
            continue
        if fn == 'ht9045_wire_engine.js':
            continue
        t = open(os.path.join(CLIENT, fn), encoding='utf-8', errors='replace').read()
        m = re.search(r"page:\s*'([^']+)'", t)
        if m:
            out[m.group(1)] = fn
    return out


def dump(page):
    cmd = [EDGE, '--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check',
           '--user-data-dir=' + os.path.join(os.environ.get('TEMP', '.'), 'wb_edge_btn'),
           '--virtual-time-budget=12000', '--dump-dom',
           'http://127.0.0.1:%d/page/%s' % (PORT, page)]
    return subprocess.run(cmd, capture_output=True, timeout=120).stdout.decode('utf-8', 'replace')


def main():
    pages = wired_pages()
    fails = []
    print('%-32s %-8s %-10s %s' % ('page', '注入鈕', '存檔', '狀態列'))
    for page in sorted(pages):
        if not os.path.isfile(os.path.join(PAGE_DIR, page)):
            continue
        dom = dump(page)
        injected = ('id="ht9045WireSave"' in dom) or ('>Save to recipe<' in dom)
        # 引擎注入的 Reload 是 position:fixed 的裸 button
        injected = injected or bool(re.search(r'<button[^>]*position:\s*fixed[^>]*>Reload</button>', dom))
        m = re.search(r'id="ht9045WireBar".*?">(.*?)<span', dom, re.S)
        bar = re.sub(r'<[^>]+>', '', m.group(1)).strip().replace('\n', ' | ') if m else '(無狀態列)'
        nosave = '沒有可用的存檔鈕' in bar
        red = 'rgb(255, 136, 136)' in (m.group(0) if m else '')

        ok_inject = not injected
        ok_save = (not nosave) or (page in TAG_ONLY)
        if not ok_inject:
            fails.append(page + '：仍有注入的浮動鈕')
        if not ok_save:
            fails.append(page + '：靜默唯讀（找不到存檔鈕）')
        if red:
            fails.append(page + '：狀態列紅字 -> ' + bar[:80])
        print('%-32s %-8s %-10s %s' % (
            page,
            'OK' if ok_inject else '**有**',
            ('唯讀' if nosave else 'OK') if not (page in TAG_ONLY) else 'n/a',
            bar[:70]))

    print('')
    if fails:
        print('FAILED %d:' % len(fails))
        for f in fails:
            print('  ' + f)
        return 1
    print('ALL PASS（%d 頁）' % len(pages))
    return 0


if __name__ == '__main__':
    sys.exit(main())
