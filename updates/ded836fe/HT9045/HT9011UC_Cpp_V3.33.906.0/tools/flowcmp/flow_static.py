"""Static half of the flow comparison (RULINGS_20260927 第 5 條): for every task that MOVED in the reference StateRecord, find the
state machine(s) that drive its variable (`int &Task=<var>` / `switch(<var>)`) in golden 906 and in the port, collect the live
`case N:` labels, and report the reference steps the port's machine does not have (untranslated or gated branches).
usage: python flow_static.py > static_report.md"""
import json, os, re, subprocess, sys
_TREE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # <repo>/HT9011UC_Cpp_V3.33.906.0
_REPO = os.path.dirname(_TREE)
_OUT = os.environ.get('FLOW_OUT') or os.path.join(os.environ.get('TEMP', _REPO), 'ht9045_flowcmp')   # outputs never go into the tree
BS = chr(92)
FL = os.path.dirname(os.path.abspath(__file__))
GOLD = 'D:' + BS + 'HT9045' + BS + 'HT9011UC_Code_V3.33.906.0_20260618'
PORTREPO = _REPO
PORT = PORTREPO + BS + 'HT9011UC_Cpp_V3.33.906.0'
sys.path.insert(0, FL)
from flow_ref import parse

ref = parse(os.path.join('D:' + BS, 'HT9045', 'Staterecord', '2025-12-11 17_47_57', 'Task_ListWithTime.csv'))
reg = {}
for l in open(os.path.join(FL, 'golden_tasklist_block.txt'), encoding='utf-8').read().split('\n'):
    m = re.search(r'SetAliasAndTask\("([^"]+)"\s*,\s*&([^)]+)\)', l)
    if m and not l.split('\t', 1)[1].lstrip().startswith('//'):
        reg[m.group(1)] = m.group(2).strip()


def strip_if0(lines):
    """blank out #if 0 ... #endif (nested #if counted) and // comments; keep line count"""
    out, stack = [], []
    for s in lines:
        t = s.strip()
        if t.startswith('#if'):
            stack.append(t.startswith('#if 0') or (stack and stack[-1]))
            out.append(''); continue
        if t.startswith('#else') and stack:
            prev = stack[-2] if len(stack) > 1 else False
            stack[-1] = (not stack[-1]) if not prev else True
            out.append(''); continue
        if t.startswith('#endif') and stack:
            stack.pop(); out.append(''); continue
        if stack and stack[-1]:
            out.append(''); continue
        out.append(s.split('//')[0])
    return out


_cache = {}


def load(path, enc):
    if path not in _cache:
        raw = open(path, 'rb').read().decode(enc, 'replace').replace('\r\n', '\n').split('\n')
        _cache[path] = (raw, strip_if0(raw))
    return _cache[path]


def machines(root, var, enc, gold):
    """-> list of (file, line, set(case labels)) for each `&Task=var` / `switch(var)` site"""
    leaf = var.split('->')[-1].split('.')[-1]
    leaf = re.sub(r'\[.*', '', leaf)
    pat = r'&\s*Task\s*=\s*[\w>\-\.]*' + re.escape(leaf) + r'\b|switch\s*\(\s*[\w>\-\.]*' + re.escape(leaf) + r'\b'
    if gold:
        cmd = ['git', 'grep', '--no-index', '-n', '-E', pat, '--', '*.cpp']
        cwd = root
    else:
        cmd = ['git', '-C', PORTREPO, 'grep', '-n', '-E', pat, '--', 'HT9011UC_Cpp_V3.33.906.0/*.cpp', 'HT9011UC_Cpp_V3.33.906.0/**/*.cpp',
               ':!HT9011UC_Cpp_V3.33.906.0/tests/*']
        cwd = None
    o = subprocess.run(cmd, capture_output=True, cwd=cwd).stdout.decode('utf-8', 'replace').splitlines()
    res = []
    for x in o:
        f, ln = x.split(':', 2)[:2]
        path = os.path.join(root if gold else PORTREPO, f.replace('/', BS))
        raw, live = load(path, enc)
        i = int(ln) - 1
        if not live[i].strip():
            continue                                  # the site itself is gated / commented
        depth, started, cases = 0, False, set()
        for k in range(i, min(len(live), i + 6000)):
            s = live[k]
            for m in re.finditer(r'\bcase\s+(-?\d+)\s*:|\bTask\s*==\s*(-?\d+)\b', s):
                cases.add(int(m.group(1) or m.group(2)))
                continue
            for m in re.finditer(r'\b' + re.escape(leaf) + r'\s*==\s*(-?\d+)\b', s):
                cases.add(int(m.group(1)))
            depth += s.count('{') - s.count('}')
            if s.count('{'):
                started = True
            if depth < 0:                             # the enclosing function (or block) closed
                break
        res.append((f.split('/')[-1], int(ln), cases))
    return res


print('# 靜態：真機走過的步序，移植樹的狀態機有沒有\n')
print('參考：`D:\\HT9045\\Staterecord\\2025-12-11 17_47_57`（HT9046_LS，FT005054）；golden 906；移植樹 m0925。\n')
print('| task | 變數 | 真機步序 | golden 狀態機 | 移植樹狀態機（活的） | 真機走過、移植樹沒有的步序 |')
print('|---|---|---|---|---|---|')
out = {}
for alias in sorted(k for k, v in ref.items() if v['ran'] and len(v['seq']) > 1):
    base = alias.split('#')[0]
    var = reg.get(base)
    steps = sorted(set(ref[alias]['seq']) - {0})
    if not var:
        print('| %s | ? | %s | | | (golden 沒登錄) |' % (alias, steps)); continue
    g = machines(GOLD, var, 'cp950', True)
    p = machines(PORT, var, 'utf-8', False)
    gc = set().union(*[c for _, _, c in g]) if g else set()
    pc = set().union(*[c for _, _, c in p]) if p else set()
    miss = [s for s in steps if s not in pc and s != 1 or (s == 1 and not p)]
    miss = [s for s in steps if s not in pc]
    gs = ', '.join('%s:%d(%d)' % (f, l, len(c)) for f, l, c in g[:3]) + (' …' if len(g) > 3 else '')
    ps = ', '.join('%s:%d(%d)' % (f, l, len(c)) for f, l, c in p[:3]) + (' …' if len(p) > 3 else '')
    print('| %s | `%s` | %s | %s | %s | %s |' % (alias, var, steps[:14], gs or '—', ps or '**無**', miss if miss else '✅'))
    out[alias] = {'var': var, 'steps': steps, 'gold': [(f, l, sorted(c)) for f, l, c in g], 'port': [(f, l, sorted(c)) for f, l, c in p], 'missing': miss}
os.makedirs(_OUT, exist_ok=True)
json.dump(out, open(os.path.join(_OUT, 'flow_static.json'), 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
