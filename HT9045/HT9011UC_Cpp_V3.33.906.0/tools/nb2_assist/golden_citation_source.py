# -*- coding: utf-8 -*-
# ===========================================================================
#  tools/nb2_assist/golden_citation_source.py  --  NB2 輔助 session 20260925
#
#  問題：移植樹註解裡的「golden <檔>:<行>」到底指的是**哪一棵** golden？906_20260618 還是 V912？
#
#  為什麼：使用者 20260925 11:2x 裁決（docs/RULINGS_20260925.md §15）「翻譯對照一律 906」。
#        但 20260914～20260925 之間工具曾把 golden 指向 V912（AI(W906-GOLDEN912)），Steven 的 C 路也照 V912 翻
#        （NB2 R31：a9636d9c 引用的 cContact.cpp:18966 是 V912 的行號，906 是 :18675）。兩棵的行號對不上，
#        引用 912 行號的段落等於「對照基準不是 906」，照 §15 要重新對 906 驗一次。
#
#  做法（第一級，字面）：
#    * 抓 `golden <path>.cpp|.h:<line>[-<line>]`，以及同一行註解裡緊接的錨點識別字（`X::Y(` 或 `Name(`）。
#    * 錨點在 906 鏡像的該行 ±2 行內出現 ⇒ 906；在 V912 的該行 ±2 行內出現 ⇒ 912；兩邊都有 ⇒ both；都沒有 ⇒ ?。
#    * 沒有錨點的引用只計數，不判斷。
#  ⚠ 「912」只代表「這個行號是照 V912 寫的」，不代表內容不同。內容要不要改，要再比該段 906 與 912 的敘述
#    （tu_function_map.py 或 NB2 R31 的做法）。
#
#  用法：python tools/nb2_assist/golden_citation_source.py [檔 ...] [--range A...B] [--v912 DIR] [--json OUT]
#        golden 906 的 UTF-8 鏡像取環境變數 NB2_GOLDEN_UTF8；V912 預設 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy（Big5，當場解碼）。
# ===========================================================================
import argparse
import json
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
TREE = os.path.abspath(os.path.join(HERE, '..', '..'))
RE_CITE = re.compile(r'golden(?:\s+912)?\s+([A-Za-z0-9_./\\-]+\.(?:cpp|h|hpp|dfm))\s*:\s*(\d+)(?:\s*-\s*:?(\d+))?')
RE_ANCHOR = re.compile(r'((?:[A-Za-z_]\w*::)?~?[A-Za-z_]\w*)\s*\(')
SKIP_ANCHOR = {'if', 'for', 'while', 'switch', 'return', 'sizeof', 'golden', 'AI', 'W906'}
_cache = {}


def lines_of(root, rel, big5):
    key = (root, rel.lower())
    if key in _cache:
        return _cache[key]
    p = os.path.join(root, rel.replace('/', os.sep))
    if not os.path.isfile(p):
        # try basename search one level (golden paths are often cited without sub-folder)
        base = os.path.basename(rel)
        hit = None
        for r, ds, fs in os.walk(root):
            ds[:] = [d for d in ds if d != '.svn']
            for f in fs:
                if f.lower() == base.lower():
                    hit = os.path.join(r, f)
                    break
            if hit:
                break
        p = hit
    out = None
    if p and os.path.isfile(p):
        b = open(p, 'rb').read()
        t = b.decode('cp950', 'replace') if big5 else b.decode('utf-8', 'replace')
        out = [l.rstrip('\r') for l in t.split('\n')]
    _cache[key] = out
    return out


def near(lines, ln, anchor):
    if not lines or ln < 1 or ln > len(lines) + 2:
        return False
    tail = anchor.split('::')[-1]
    for k in range(max(0, ln - 3), min(len(lines), ln + 2)):
        if re.search(r'(?<![\w])%s\s*\(' % re.escape(tail), lines[k]):
            return True
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('files', nargs='*')
    ap.add_argument('--range', help='git range; scan files changed in it (e.g. origin/main...origin/v906/steven-cbridge-review6)')
    ap.add_argument('--v912', default=r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy')
    ap.add_argument('--json')
    ap.add_argument('--all', action='store_true', help='scan every tracked .cpp/.h/.inc/.js/.py in the port tree (except tools/nb2_assist)')
    a = ap.parse_args()
    g906 = os.environ.get('NB2_GOLDEN_UTF8')
    if not g906:
        sys.exit('set NB2_GOLDEN_UTF8 to the golden 906 UTF-8 mirror')
    files = list(a.files)
    if a.all:
        out = subprocess.run(['git', '-C', TREE, 'ls-files', '--', '*.cpp', '*.h', '*.inc', '*.js', '*.py'],
                             capture_output=True).stdout.decode('utf-8', 'replace').splitlines()
        files += [f for f in out if not f.startswith('tools/nb2_assist/')]
    if a.range:
        out = subprocess.run(['git', '-C', TREE, 'diff', '--name-only', a.range, '--', '.'], capture_output=True).stdout.decode('utf-8', 'replace').splitlines()
        pre = 'HT9011UC_Cpp_V3.33.906.0/'
        files += [f[len(pre):] if f.startswith(pre) else f for f in out if f.endswith(('.cpp', '.h', '.inc', '.py'))]
    rows = []
    for f in files:
        p = os.path.join(TREE, f)
        if not os.path.isfile(p):
            continue
        for i, l in enumerate(open(p, 'rb').read().decode('utf-8', 'replace').split('\n')):
            for m in RE_CITE.finditer(l):
                rel, ln = m.group(1), int(m.group(2))
                rest = l[m.end():]
                anc = None
                for am in RE_ANCHOR.finditer(rest):
                    if am.group(1).split('::')[-1] not in SKIP_ANCHOR:
                        anc = am.group(1)
                        break
                if not anc:
                    rows.append({'file': f, 'line': i + 1, 'cite': '%s:%d' % (rel, ln), 'anchor': None, 'src': 'no-anchor'})
                    continue
                in906 = near(lines_of(g906, rel, False), ln, anc)
                in912 = near(lines_of(a.v912, rel, True), ln, anc)
                src = 'both' if in906 and in912 else '906' if in906 else '912' if in912 else '?'
                rows.append({'file': f, 'line': i + 1, 'cite': '%s:%d' % (rel, ln), 'anchor': anc, 'src': src})
    per = defaultdict(Counter)
    for r in rows:
        per[r['file']][r['src']] += 1
    print('# golden 行號引用指向哪一棵（906 vs V912）\n')
    print('掃 %d 支檔、%d 個引用。⚠「912」只表示行號照 V912 寫，內容是否不同要另比（R31 做法）。\n' % (len(per), len(rows)))
    print('| 檔 | 906 | 912 | both | ? | 無錨點 |\n|---|---|---|---|---|---|')
    for f, c in sorted(per.items(), key=lambda kv: -kv[1]['912']):
        print('| `%s` | %d | %d | %d | %d | %d |' % (f, c['906'], c['912'], c['both'], c['?'], c['no-anchor']))
    only912 = [r for r in rows if r['src'] == '912']
    print('\n## 只對得上 V912 的引用（%d 個，前 60）\n' % len(only912))
    for r in only912[:60]:
        print('* `%s:%d` → golden `%s` `%s`' % (r['file'], r['line'], r['cite'], r['anchor']))
    if a.json:
        json.dump(rows, open(a.json, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)


if __name__ == '__main__':
    main()
