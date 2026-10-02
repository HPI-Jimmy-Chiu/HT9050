# -*- coding: utf-8 -*-
# Steven 20260915
# ----------------------------------------------------------------------
# 把 wire js 與 <script> 行佈署到頁面。（保存版）
# 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
# ----------------------------------------------------------------------

"""deploy_wire.py -- 把產生的接線資料檔部署到 web/page 並接上 HTML。

AI(W906-FW-GEN) 20260914。

做三件事，每一件都可重入（跑第二次不會重複加）：
  1. web/page/ht9045_wire_*.js 就地（AI(W906-P5B) 20260923：原本是 client/ -> web/page/ 的複製；
     P5 裁決甲後 gen_wire.py 直接產到 web/page，來源＝目的時不複製）
  2. 每個頁面的 </body> 之前插入四行 <script>，順序固定：
       qwerty.js -> ht9045_recipe_client.js -> ht9045_wire_engine.js -> ht9045_wire_<slug>.js
  3. 把新增的 js 加進 tools/sync_web.py 的 OURS

原檔備份到 scratchpad/html_backup/。
"""
import os, re, glob, shutil, io, sys

CLIENT  = r'D:\HT9045\web\page'   # AI(W906-P5B) 20260923：P5 裁決甲，原本是 D:\HT9045\client
WEBPAGE = r'D:\HT9045\web\page'
SYNC    = r'D:\HT9045\tools\sync_web.py'
BACKUP  = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'html_backup')

TAGS = ['qwerty.js', 'ht9045_recipe_client.js', 'ht9045_wire_engine.js']


def _copy_if_different(src, dst):
    # AI(W906-P5B) 20260923：CLIENT 改指 web/page 之後來源常常就是目的地；shutil.copy2 對同一個檔會丟 SameFileError。
    if os.path.normcase(os.path.abspath(src)) != os.path.normcase(os.path.abspath(dst)):
        shutil.copy2(src, dst)


def main():
    os.makedirs(BACKUP, exist_ok=True)
    done, skipped, ours = [], [], []

    for src in sorted(glob.glob(os.path.join(CLIENT, 'ht9045_wire_*.js'))):
        base = os.path.basename(src)
        if base == 'ht9045_wire_engine.js':
            _copy_if_different(src, os.path.join(WEBPAGE, base))
            ours.append('page/' + base)
            continue

        text = open(src, encoding='utf-8').read()
        m = re.search(r"page:\s*'([^']+)'", text)
        if not m:
            skipped.append((base, '找不到 page:'))
            continue
        page = m.group(1)
        html = os.path.join(WEBPAGE, page)
        if not os.path.isfile(html):
            skipped.append((base, '找不到 ' + page))
            continue

        _copy_if_different(src, os.path.join(WEBPAGE, base))
        ours.append('page/' + base)

        s = open(html, encoding='utf-8').read()
        if base in s:
            done.append((page, base, '已接線，略過'))
            continue
        if s.count('</body>') != 1:
            skipped.append((base, '%s 有 %d 個 </body>' % (page, s.count('</body>'))))
            continue

        shutil.copy2(html, os.path.join(BACKUP, page))
        lines = ['<!-- AI(W906-FW-GEN) 20260914: recipe read/write + on-screen keyboard.',
                 '     載入順序不可顛倒：qwerty -> recipe client -> engine -> page data.',
                 '     資料檔由 scratchpad/gen_wire.py 從 golden 機械產生。 -->']
        for t in TAGS:
            if t not in s:
                lines.append('<script src="%s"></script>' % t)
        lines.append('<script src="%s"></script>' % base)
        block = '\n'.join(lines) + '\n'
        open(html, 'w', encoding='utf-8', newline='').write(s.replace('</body>', block + '</body>'))
        done.append((page, base, '已接線'))

    # --- sync_web.py OURS ---
    sy = open(SYNC, encoding='utf-8').read()
    add = [o for o in sorted(set(ours)) if '"%s"' % o not in sy]
    if add:
        i = sy.index('OURS = {')
        j = sy.index(chr(10) + '}', i) + 1      # OURS 區塊收尾的 '}'
        block = ''.join('    "%s",' % o + chr(10) for o in add)
        open(SYNC, 'w', encoding='utf-8', newline='').write(sy[:j] + block + sy[j:])

    print('%-28s %-34s %s' % ('page', 'data file', 'result'))
    for p, b, r in sorted(done):
        print('%-28s %-34s %s' % (p, b, r))
    for b, r in skipped:
        print('SKIP %-23s %s' % (b, r))
    print()
    print('加進 OURS 的項目: %d' % len(add))


main()
