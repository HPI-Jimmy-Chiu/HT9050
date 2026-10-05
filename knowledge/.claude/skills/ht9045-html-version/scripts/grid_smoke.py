# -*- coding: utf-8 -*-
# Steven 20260916
# ----------------------------------------------------------------------
# 新檔。用 Edge 無頭模式把表格頁真的載入一次，檢查 sysGrid 有沒有畫出來。
# 當日完整變更紀錄：<入口網站 repo>\public\Docs\ChangeLog\Steven\CHANGES_20260916_Steven.md
# ----------------------------------------------------------------------

"""grid_smoke.py -- 沒有 Playwright，就用 msedge --headless --dump-dom。

AI(W906-FW-SYSWIRE) 20260916。

對每個表格頁檢查四件事（都是從真正跑過 JS 的 DOM 量出來，不是看原始碼）：
  1. 我們的容器 #<host>__wb 存在，裡面 .wbGrid 的資料列數 == /api/system/<file> 的 rows 數
  2. legacy 容器 #<host> 被藏起來（display:none）——它會被過期 JSON 快照畫進去
  3. 表頭欄數 == API 的 columns 數
  4. 鍵欄的格子都有 class="ro"（唯讀）

伺服器要先跑著（--allow-cmd 即可，這裡只讀；AI(W906-NODRY) 20260924：--dry 已退場，傳了會被拒絕）。
用法: py grid_smoke.py
"""
import io, json, os, re, subprocess, sys, urllib.request

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
EDGE = r'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe'
BASE = 'http://127.0.0.1:8045'
PAGES = [('HW.MotorTest.html', 'motTable', 'strngrdMotorData'),
         ('HW.IoSetView.html', 'ioTable', 'strngrdIoTable')]


def dump(url):
    # --virtual-time-budget 讓頁面的非同步載入（fetch API、WS）有時間跑完再 dump。
    cmd = [EDGE, '--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check',
           '--user-data-dir=' + os.path.join(os.environ.get('TEMP', '.'), 'wb_edge_profile'),
           '--virtual-time-budget=8000', '--dump-dom', url]
    out = subprocess.run(cmd, capture_output=True, timeout=90)
    return out.stdout.decode('utf-8', 'replace')


def main():
    rc = 0
    for page, f, host in PAGES:
        api = json.load(urllib.request.urlopen(BASE + '/api/system/' + f))
        n_rows, n_cols, kc = len(api['rows']), len(api['columns']), api['keyColumn']
        dom = dump(BASE + '/page/' + page)
        print('=== %s  (DOM %d bytes)' % (page, len(dom)))

        m = re.search(r'<div[^>]*id="%s__wb"[^>]*>(.*?)</div>\s*<div[^>]*id="%s"' % (host, host), dom, re.S)
        mine = m.group(0) if m else ''
        tbody_rows = len(re.findall(r'<tr data-k=', mine))
        ths = len(re.findall(r'<th[\s>]', mine))   # 不要連 <thead> 一起數進去（第一版就是這樣多了 1）
        legacy = re.search(r'<div[^>]*id="%s"[^>]*style="([^"]*)"' % host, dom)
        legacy_hidden = bool(legacy and 'display: none' in legacy.group(1).replace(':none', ': none'))
        key_ro = len(re.findall(r'<td data-c="%s" class="[^"]*\bro\b' % re.escape(kc), mine))

        checks = [
            ('我們的表格列數 == API rows', tbody_rows == n_rows, '%d vs %d' % (tbody_rows, n_rows)),
            ('legacy 容器已藏起來', legacy_hidden, legacy.group(1)[:60] if legacy else '找不到 legacy 容器'),
            ('表頭欄數 == API columns', ths == n_cols, '%d vs %d' % (ths, n_cols)),
            ('鍵欄 %s 全部唯讀' % kc, key_ro == n_rows, '%d vs %d' % (key_ro, n_rows)),
        ]
        for name, ok, detail in checks:
            rc |= 0 if ok else 1
            print('    %-30s %s  (%s)' % (name, 'PASS' if ok else 'FAIL', detail))
    sys.exit(rc)


main()
