# -*- coding: utf-8 -*-
r"""FShow_Audit：全樹「畫面開著沒」讀取點的棘輪（ratchet）。

AI(W906-PAGETAB-Q51) 20260928 [W906] St01（Steven 團隊）。
設計 D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-state-array.md §6 T3。
由 tools/p6b_fshow_audit.py（只看 WebStart.cpp）擴成全樹；p6b 那支照舊留著，不動。

## 白話

golden BCB 的表單自己帶「我現在顯示著」旗標（fShow／bShow），別的程式直接讀
`fContact->fShow`。網頁版的畫面在瀏覽器裡，C++ 的那個成員沒人設成真 ⇒ 直接讀它
永遠是 false。Steven 20260928（Q51、第一優先）要所有讀取改問頁面表的單一函式：

    golden:  if(fContact->fShow==false)
    移植:    if(W906_FormShowing("fContact", fContact->fShow)==false)

這支工具數每個檔還有幾個「直接讀成員」（bare）的，並跟基準檔
tools/fshow_audit_baseline.json 比：**任何一個檔的數目只能減、不能增**。
新翻進來的 golden 程式（或解開 `#if 0` 閘的人）只要寫了一個直接讀，ctest 就紅。

## 算什麼

* 讀 `X->fShow`、`X->bShow`（X 是任何識別字；寫入 `X->fShow = ...` 不算）。
* 讀 `X->Visible`／`X->Showing`，但只限 X 是頁面表上的表單名
  （從 WebPageTable.cpp 的 kRows 解析；例 `MyMessageBox->Visible`）。
* `#define` 裡把 `->fShow` 包起來的那一行也算裸的（使用點看不到，逼人來看定義）。
* 已經包在 `W906_FormShowing("X", X->fShow)`（或舊名 `W906_FormFShow`／`W906_FShow`）
  裡的算「已接」。
* 落在 `#if 0` 堆疊裡的算「閘住」（批 7；解閘的人照這個函式寫，解開當下這支工具就會抓）。
* 註解（`//`、`/* */`）與字串常值裡的不算。

另外一條：除了 WebWindowRegistry.cpp 自己和 WebTeachLeave.cpp 的「沒安裝」退路，
不准有人直接呼叫 `WebWindowRegistryFShowPolicy(`／`WebWindowRegistryDiagnosticsOpen(`
（那是「過期＝當成開著」的舊政策；要問就問頁面表）。

## 範圍

`git ls-files -co --exclude-standard`（取不到就走目錄）的 .cpp／.h／.hpp／.inc／.c，
排除 tests/、docs/、tools/dfm2rc/、third_party/、build*、scratchpad/。

## 用法

    python tools/fshow_audit.py                   # 棘輪檢查：有檔變多 ⇒ 回 1
    python tools/fshow_audit.py --list [子字串]    # 列出裸的讀取點（可用檔名子字串過濾）
    python tools/fshow_audit.py --write-baseline  # 數目變少之後收緊基準（只准變少；變多要加 --force）
    python tools/fshow_audit.py --selftest        # 驗稽核器自己會不會假綠
"""
import io
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BASELINE = os.path.join(HERE, 'fshow_audit_baseline.json')
PAGETABLE = os.path.join(ROOT, 'WebPageTable.cpp')

EXTS = ('.cpp', '.h', '.hpp', '.inc', '.c')
EXCLUDE_PREFIX = ('tests/', 'docs/', 'tools/dfm2rc/', 'third_party/', 'scratchpad/', 'build', 'C:/')

MEMBER = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*->\s*(fShow|bShow|Visible|Showing)\b')
WRAPPED = re.compile(r'\bW906_(?:FormShowing|FormFShow|FShow)\s*\(\s*"([A-Za-z_][A-Za-z0-9_]*)"\s*,')
MACRO = re.compile(r'^\s*#\s*define\b.*->\s*(fShow|bShow)\b')
IF0 = re.compile(r'^\s*#\s*if\s*\(?\s*0\s*\)?(\s|$)')
POLICY = re.compile(r'\b(?:ht9045::)?(WebWindowRegistryFShowPolicy|WebWindowRegistryDiagnosticsOpen)\s*\(')
POLICY_ALLOWED = ('WebWindowRegistry.cpp', 'WebWindowRegistry.h', 'WebTeachLeave.cpp')
ROWFORM = re.compile(r'^\s*\{\s*"([A-Za-z_][A-Za-z0-9_]*)"\s*,\s*"')


def out(s):
    sys.stdout.write(s)


def load_forms():
    """頁面表的表單名（Visible／Showing 只對這些算）。讀不到 ⇒ 空集合（Visible／Showing 不算）。"""
    forms = set()
    try:
        with io.open(PAGETABLE, encoding='utf-8', newline='') as fh:
            for line in fh:
                m = ROWFORM.match(line)
                if m:
                    forms.add(m.group(1))
    except IOError:
        pass
    return forms


def strip_lines(text):
    """回傳 [(code, masked)]，一行一組：code ＝ 去掉註解（字串留著），masked ＝ 再把字串內容換成空白（長度不變）。"""
    res = []
    in_block = False
    for raw in text.split('\n'):
        raw = raw.rstrip('\r')
        code = []
        masked = []
        i = 0
        n = len(raw)
        while i < n:
            c = raw[i]
            if in_block:
                if raw.startswith('*/', i):
                    in_block = False
                    code.append('  ')
                    masked.append('  ')
                    i += 2
                else:
                    code.append(' ')
                    masked.append(' ')
                    i += 1
                continue
            if raw.startswith('//', i):
                break
            if raw.startswith('/*', i):
                in_block = True
                code.append('  ')
                masked.append('  ')
                i += 2
                continue
            if c == '"' or c == "'":
                q = c
                j = i + 1
                while j < n and raw[j] != q:
                    if raw[j] == '\\':
                        j += 1
                    j += 1
                j = min(j, n - 1)
                seg = raw[i:j + 1]
                code.append(seg)
                masked.append(q + ' ' * max(0, len(seg) - 2) + (q if len(seg) >= 2 else ''))
                i = j + 1
                continue
            code.append(c)
            masked.append(c)
            i += 1
        res.append((''.join(code), ''.join(masked)))
    return res


def is_write(masked, end):
    """`X->fShow = ...`（不是 ==）＝ 寫入。"""
    k = end
    while k < len(masked) and masked[k] in ' \t':
        k += 1
    return k < len(masked) and masked[k] == '=' and not masked.startswith('==', k)


def scan(text, forms, path=''):
    """回傳 dict：wired／bare／gated／policy，各是 [(行號, 物件, 成員)]。"""
    r = {'wired': [], 'bare': [], 'gated': [], 'policy': []}
    stack = []
    base = path.replace('\\', '/').rsplit('/', 1)[-1]
    for lineno, (code, masked) in enumerate(strip_lines(text), 1):
        s = masked.lstrip()
        if s.startswith('#'):
            d = re.sub(r'^#\s*', '#', s)
            if d.startswith('#if'):
                stack.append(bool(IF0.match(s)))
            elif d.startswith('#elif'):
                if stack:
                    stack[-1] = False
            elif d.startswith('#else'):
                if stack:
                    stack[-1] = not stack[-1]
            elif d.startswith('#endif'):
                if stack:
                    stack.pop()
            if MACRO.match(masked) and not any(stack):
                r['bare'].append((lineno, u'#define', 'fShow'))
            continue
        dead = any(stack)
        if not dead and base not in POLICY_ALLOWED:
            for m in POLICY.finditer(masked):
                r['policy'].append((lineno, m.group(1), ''))
        if '->' not in masked:
            continue
        # 包好的 ＝ 落在 W906_FormShowing("X", ...) 的第二個引數裡（成員那一格可以是 `fTeach != 0 && fTeach->fShow==true`
        #   這種運算式，csystem.cpp:30098 就是）；括號配對只在同一行找，找不到就算到行尾。
        spans = []
        for m in WRAPPED.finditer(code):
            depth = 1
            k = m.end()
            while k < len(masked) and depth > 0:
                if masked[k] == '(':
                    depth += 1
                elif masked[k] == ')':
                    depth -= 1
                k += 1
            spans.append((m.end(), k, m.group(1)))
        for m in MEMBER.finditer(masked):
            obj, mem = m.group(1), m.group(2)
            if mem in ('Visible', 'Showing') and obj not in forms:
                continue
            if is_write(masked, m.end()):
                continue
            if any(a <= m.start() and m.end() <= b for a, b, _ in spans):
                if not dead:
                    r['wired'].append((lineno, obj, mem))
                continue
            if dead:
                r['gated'].append((lineno, obj, mem))
            else:
                r['bare'].append((lineno, obj, mem))
    return r


def list_files():
    files = None
    try:
        p = subprocess.run(['git', '-C', ROOT, 'ls-files', '-co', '--exclude-standard', '-z'],
                           stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=60)
        if p.returncode == 0:
            files = [f for f in p.stdout.decode('utf-8', 'replace').split('\0') if f]
    except Exception:
        files = None
    if files is None:
        files = []
        for dp, dn, fn in os.walk(ROOT):
            rel = os.path.relpath(dp, ROOT).replace('\\', '/')
            rel = '' if rel == '.' else rel + '/'
            dn[:] = [d for d in dn if not (rel + d + '/').startswith(EXCLUDE_PREFIX) and not d.startswith('.')]
            for f in fn:
                files.append(rel + f)
    res = []
    for f in files:
        f = f.replace('\\', '/')
        if not f.endswith(EXTS):
            continue
        if f.startswith(EXCLUDE_PREFIX):
            continue
        if os.path.isfile(os.path.join(ROOT, f)):
            res.append(f)
    return sorted(set(res))


def scan_tree():
    forms = load_forms()
    per = {}
    for f in list_files():
        try:
            with io.open(os.path.join(ROOT, f), encoding='utf-8', errors='replace', newline='') as fh:
                text = fh.read()
        except IOError:
            continue
        if '->' not in text and 'WebWindowRegistry' not in text:
            continue
        r = scan(text, forms, f)
        if r['bare'] or r['wired'] or r['gated'] or r['policy']:
            per[f] = r
    return per, forms


def load_baseline():
    try:
        with io.open(BASELINE, encoding='utf-8') as fh:
            return json.load(fh)
    except (IOError, ValueError):
        return None


def totals(per):
    t = {'wired': 0, 'bare': 0, 'gated': 0, 'policy': 0}
    for r in per.values():
        for k in t:
            t[k] += len(r[k])
    return t


def cmd_check(args):
    per, forms = scan_tree()
    base = load_baseline()
    t = totals(per)
    out(u'FShow_Audit: %d files, wired %d, bare %d, gated(#if 0) %d; page-table forms %d\n'
        % (len(per), t['wired'], t['bare'], t['gated'], len(forms)))
    if not forms:
        out(u'FAIL cannot read the form list from %s (kRows)\n' % PAGETABLE)
        return 2
    if base is None:
        out(u'FAIL no baseline %s -- run with --write-baseline once\n' % BASELINE)
        return 2
    bmap = base.get('bare', {})
    bad = 0
    tighter = []
    for f in sorted(set(list(per.keys()) + list(bmap.keys()))):
        n = len(per[f]['bare']) if f in per else 0
        b = int(bmap.get(f, 0))
        if n > b:
            bad += 1
            out(u'\nFAIL %s: %d bare reads (baseline %d) -- a new direct read of a form\'s show-state member:\n' % (f, n, b))
            for lineno, obj, mem in per[f]['bare']:
                out(u'  %s:%d  %s->%s\n' % (f, lineno, obj, mem))
        elif n < b:
            tighter.append((f, n, b))
    pol = [(f, x) for f, r in per.items() for x in r['policy']]
    for f, (lineno, name, _) in sorted(pol):
        bad += 1
        out(u'FAIL %s:%d calls %s directly -- ask the page table (W906_FormShowing / W906_PageFormAnswer) instead\n'
            % (f, lineno, name))
    if bad:
        out(u'\nFix: write `W906_FormShowing("X", X->fShow)` (same line; W906FormShowing.h or csystem.h).\n'
            u'If a read must stay direct, say why on that line and raise the baseline with --write-baseline --force\n'
            u'in the same commit -- do not teach the auditor to stop asking.\n')
        return 1
    if tighter:
        out(u'NOTE %d file(s) went down; tighten the ratchet with --write-baseline:\n' % len(tighter))
        for f, n, b in tighter:
            out(u'  %s %d -> %d\n' % (f, b, n))
    out(u'PASS no file has more bare reads than the baseline (total baseline %d, now %d)\n'
        % (sum(int(v) for v in bmap.values()), t['bare']))
    return 0


def cmd_list(args):
    per, _ = scan_tree()
    flt = [a for a in args if not a.startswith('--')]
    kinds = ['bare']
    if '--gated' in args:
        kinds.append('gated')
    if '--wired' in args:
        kinds.append('wired')
    for f in sorted(per):
        if flt and not any(x.replace('\\', '/') in f for x in flt):
            continue
        for k in kinds:
            for lineno, obj, mem in per[f][k]:
                out(u'%-6s %s:%d  %s->%s\n' % (k, f, lineno, obj, mem))
    return 0


def cmd_write(args):
    per, forms = scan_tree()
    if not forms:
        out(u'FAIL cannot read the form list from %s\n' % PAGETABLE)
        return 2
    new = dict((f, len(r['bare'])) for f, r in per.items() if r['bare'])
    old = load_baseline()
    if old is not None and '--force' not in args:
        up = [(f, n, int(old.get('bare', {}).get(f, 0))) for f, n in new.items()
              if n > int(old.get('bare', {}).get(f, 0))]
        if up:
            out(u'FAIL refusing to raise the baseline without --force:\n')
            for f, n, b in up:
                out(u'  %s %d -> %d\n' % (f, b, n))
            return 1
    data = {
        'about': u'tools/fshow_audit.py ratchet: bare reads of a form show-state member per file; '
                 u'may only go down (AI(W906-PAGETAB-Q51) 20260928)',
        'total': sum(new.values()),
        'bare': dict(sorted(new.items())),
    }
    with io.open(BASELINE, 'w', encoding='utf-8', newline='\n') as fh:
        fh.write(json.dumps(data, ensure_ascii=False, indent=1, sort_keys=False) + '\n')
    out(u'wrote %s: %d files, %d bare reads\n' % (BASELINE, len(new), data['total']))
    return 0


SELFTEST_CASES = [
    # (原始碼, wired, bare, gated, policy, 說明)
    ('if (fContact->fShow == false)\n', 0, 1, 0, 0, u'裸的讀取點要被抓到'),
    ('if (W906_FormShowing("fContact", fContact->fShow) == false)\n', 1, 0, 0, 0, u'包好的不算裸的'),
    ('if (W906_FShow("fContact", fContact->fShow))\n', 1, 0, 0, 0, u'舊名 W906_FShow 也算包好'),
    ('fContact->fShow = false;\n', 0, 0, 0, 0, u'寫入不算讀'),
    ('fContact->fShow=true;  x = (fContact->fShow == true);\n', 0, 1, 0, 0, u'同一行一寫一讀：只數讀'),
    ('#if 0\nif (fContact->fShow == false)\n#endif\n', 0, 0, 1, 0, u'#if 0 裡的算閘住'),
    ('#if 0 // GATE\n#if 1\nif (fContact->fShow)\n#endif\n#endif\n', 0, 0, 1, 0, u'#if 0 裡的 #if 1 仍然是死的'),
    ('#if 0\n#else\nif (fContact->fShow)\n#endif\n', 0, 1, 0, 0, u'#if 0 的 #else 是活的'),
    ('#if 0\n#endif\nif (fContact->fShow)\n', 0, 1, 0, 0, u'#endif 之後恢復成活的'),
    ('// fContact->fShow is the golden form\n', 0, 0, 0, 0, u'// 註解裡的不算'),
    ('x = 1; /* fContact->fShow */ y = 2;\n', 0, 0, 0, 0, u'/* */ 註解裡的不算'),
    ('/* start\n if (fContact->fShow)\n end */\n', 0, 0, 0, 0, u'跨行的 /* */ 註解裡的不算'),
    ('Log("fContact->fShow");\n', 0, 0, 0, 0, u'字串裡的不算'),
    ('x = a->fShow; y = W906_FormShowing("b", b->fShow);\n', 1, 1, 0, 0, u'同一行一裸一包'),
    ('if(W906_FormFShow("fTeach", fTeach != 0 && fTeach->fShow==true) || fMotorTest->fShow)\n', 1, 1, 0, 0,
     u'成員那一格是運算式也算包好；括號外的照樣是裸的'),
    ('#define TEACHING (fTeach->fShow)\n', 0, 1, 0, 0, u'包裝巨集的定義算裸的'),
    ('if (MyMessageBox->Visible)\n', 0, 1, 0, 0, u'頁面表表單的 Visible 算'),
    ('if (Panel2->Visible)\n', 0, 0, 0, 0, u'不是表單的 Visible 不算'),
    ('return ht9045::WebWindowRegistryFShowPolicy("fTeach");\n', 0, 0, 0, 1, u'直接呼叫舊政策要抓'),
    ('// WebWindowRegistryFShowPolicy("fTeach")\n', 0, 0, 0, 0, u'註解裡提到舊政策不算'),
]


def selftest():
    forms = set(['fContact', 'fTeach', 'MyMessageBox', 'fNote'])
    bad = 0
    for src, ew, eb, eg, ep, why in SELFTEST_CASES:
        r = scan(src, forms, 'x.cpp')
        got = (len(r['wired']), len(r['bare']), len(r['gated']), len(r['policy']))
        ok = got == (ew, eb, eg, ep)
        if not ok:
            bad += 1
        out(u'  %s %s  (wired=%d/%d bare=%d/%d gated=%d/%d policy=%d/%d)\n' % (
            u'PASS' if ok else u'FAIL', why, got[0], ew, got[1], eb, got[2], eg, got[3], ep))
    r = scan('return WebWindowRegistryFShowPolicy(form);\n', forms, 'WebTeachLeave.cpp')
    ok = len(r['policy']) == 0
    bad += 0 if ok else 1
    out(u'  %s WebTeachLeave.cpp 的沒安裝退路可以呼叫舊政策\n' % (u'PASS' if ok else u'FAIL'))
    f = load_forms()
    ok = 'fContact' in f and 'fNote' in f and 'Zteach' in f and len(f) >= 60
    bad += 0 if ok else 1
    out(u'  %s 從 WebPageTable.cpp 讀到頁面表的表單名（%d 個）\n' % (u'PASS' if ok else u'FAIL', len(f)))
    return bad


def main():
    if hasattr(sys.stdout, 'reconfigure'):
        try:
            sys.stdout.reconfigure(encoding='utf-8')
        except Exception:
            pass
    args = sys.argv[1:]
    if '--selftest' in args:
        out(u'== fshow_audit selftest ==\n')
        bad = selftest()
        out(u'== %d failed ==\n' % bad)
        return 1 if bad else 0
    if '--list' in args:
        return cmd_list([a for a in args if a != '--list'])
    if '--write-baseline' in args:
        return cmd_write(args)
    return cmd_check(args)


if __name__ == '__main__':
    sys.exit(main())
