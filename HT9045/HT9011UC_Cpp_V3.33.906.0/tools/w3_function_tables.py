# -*- coding: utf-8 -*-
# ===========================================================================
#  tools/w3_function_tables.py  --  AI(W906-W3-AUDIT) 20260926
#
#  W3 稽核第 ⑴ 項：mykitsuck／myTimer／MyTempPanel 的「被引用函式 → 移植樹位置 → 本體內閘數 → 測試」表。
#  * 被引用：某個非測試 .obj 把它列為未定義（與 tools/w3_caller_census.py 同一規則，nm 量，不 grep）
#  * 測試：tests/ 底下的測試 TU（.obj 路徑不含 __，即不是編進測試的產品檔）把它列為未定義 ⇒ 那支 ctest 直接呼叫它；
#    經產品碼間接走到的不算（所以「—」不等於沒被測到，只是沒有測試直接叫它）
#  * 本體內 `#if 0`：定義行到下一個頂層定義之間的 `#if 0` 行數
#  ⚠ 這是「符號存在」三級裡的第一級（能編、能連）；有沒有執行期呼叫者要另外量。
#  ⚠ Bash 裡跑要先把 C:\MinGW\bin 放進 PATH（nm／c++filt 本身不需要，但同一台機器上 g++ 沒 PATH 會不出聲地失敗）。
#
#  用法：python tools/w3_function_tables.py <build_dir> <輸出.md>
#  例：  python tools/w3_function_tables.py ../Obj/V906/build_ship docs/_w3_tables.md
# ===========================================================================
import os, re, subprocess, sys
from collections import defaultdict
BD = os.path.abspath(sys.argv[1])
SRC = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NM = r'C:\MinGW\bin\nm.exe'
TARGETS = {'mykitsuck': 'ht9045_sm', 'myTimer': 'ht9045_globals', 'MyTempPanel': 'ht9045_sm', 'myio': 'ht9045_io'}


def nm(args, obj):
    r = subprocess.run([NM] + args + [obj], capture_output=True)
    return [l.strip() for l in r.stdout.decode('utf-8', 'replace').splitlines() if l.strip()]


objs, tobjs = [], []
for r, _, fs in os.walk(BD):
    if 'CMakeFiles' not in r:
        continue
    for f in fs:
        if f.endswith('.obj'):
            (tobjs if os.sep + 'tests' + os.sep in r + os.sep else objs).append(os.path.join(r, f))
undef, tundef = defaultdict(set), defaultdict(set)
for o in objs:
    for l in nm(['--undefined-only'], o):
        undef[l.split()[-1]].add(os.path.basename(o).replace('.cpp.obj', ''))
for o in tobjs:
    if os.sep + '__' + os.sep in o:   # a production .cpp compiled into a test target, not a test TU
        continue
    for l in nm(['--undefined-only'], o):
        tundef[l.split()[-1]].add([c for c in o.split(os.sep) if c.endswith('.dir')][-1][:-4])

out = []
for name, tgt in TARGETS.items():
    obj = os.path.join(BD, 'CMakeFiles', tgt + '.dir', name + '.cpp.obj')
    src = open(os.path.join(SRC, name + '.cpp'), 'rb').read().decode('utf-8').splitlines()
    # definition lines: "Type Class::Func(" or "Class::Class(" at column 0
    defs = [(i, l) for i, l in enumerate(src, 1) if re.match(r'^[A-Za-z_][\w\s\*&<>:,]*\b\w+::~?\w+\s*\(', l) or re.match(r'^\w+::~?\w+\s*\(', l)]
    # free functions at column 0: "Type Name(" whose line is not a prototype (no trailing ';')
    defs += [(i, l) for i, l in enumerate(src, 1)
             if re.match(r'^(static\s+|inline\s+)*(void|bool|int|byte|BYTE|DWORD|double|unsigned\s+\w+|AnsiString)\s*[\*&]?\s*\w+\s*\(', l)
             and not l.split('//')[0].rstrip().endswith(';') and (i, l) not in defs]
    defs.sort()
    rows = []
    for l in nm(['--defined-only'], obj):
        p = l.split()
        if len(p) < 3 or p[1] not in 'Tt':
            continue
        mang = p[2]
        users = undef.get(mang, set()) - {name}
        if not users:
            continue
        m2 = re.match(r'^@(_Z.*)@\d+$', mang)   # stdcall-decorated ctor symbol, e.g. @_ZN10TQPF_TimerC1Ev@4
        dem = subprocess.run([r'C:\MinGW\bin\c++filt.exe', m2.group(1) if m2 else mang], capture_output=True).stdout.decode().strip()
        if dem.startswith('_Z'):   # mingw32 c++filt wants the target's extra leading underscore
            dem = subprocess.run([r'C:\MinGW\bin\c++filt.exe', '_' + dem], capture_output=True).stdout.decode().strip()
        base = re.sub(r'\(.*', '', dem).split('::')[-1]
        cls = dem.split('::')[0] if '::' in dem else ''
        # find definition line: match Class::base( ... with same arg count
        cand = [i for i, l in defs if re.search(r'\b%s\s*\(' % re.escape((cls + '::' if cls else '') + base), l)]
        nargs = 0 if dem.endswith('()') else dem.count(',') + 1
        best = None
        for i in cand:
            sig = src[i - 1]
            j = i
            while ')' not in sig and j < len(src):
                sig += src[j]; j += 1
            a = sig[sig.index('(') + 1: sig.index(')')].strip()
            n = 0 if a in ('', 'void') else a.count(',') + 1
            if n == nargs:
                best = i; break
        if best is None and cand:
            best = cand[0]
        gates = '?'
        if best:
            nxt = [i for i, _ in defs if i > best]
            end = nxt[0] - 1 if nxt else len(src)
            gates = sum(1 for k in range(best - 1, end) if re.match(r'\s*#if 0\b', src[k]))
        tests = sorted(tundef.get(mang, set()))
        rows.append((len(users), dem, best, gates, tests))
    rows.sort(key=lambda r: (-r[0], r[1]))
    out.append('### %s（%d 個被引用）' % (name, len(rows)))
    out.append('')
    out.append('| # | 函式 | 移植樹 | 本體內 `#if 0` | 引用它的產品檔數 | 測試 TU 直接呼叫它的 ctest（經產品碼間接走到的不算） |')
    out.append('|---|---|---|---|---|---|')
    for k, (n, dem, best, gates, tests) in enumerate(rows, 1):
        dem = dem.replace('vclcompat::', '').replace('|', '\\|')
        t = '、'.join(tests[:4]) + ('…（%d）' % len(tests) if len(tests) > 4 else '') if tests else '—'
        out.append('| %d | `%s` | `%s.cpp:%s` | %s | %d | %s |' % (k, dem, name, best, gates, n, t))
    out.append('')
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write('\n'.join(out))
print('\n'.join(l for l in out if l.startswith('###')))
