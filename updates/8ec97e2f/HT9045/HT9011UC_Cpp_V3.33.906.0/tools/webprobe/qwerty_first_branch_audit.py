# -*- coding: utf-8 -*-
r"""qwerty_first_branch_audit.py -- READ-ONLY audit of the generated on-screen keyboard tables.

AI(W906-QFB-AUDIT) 20261002 (St02-E helper) -- laptop INBOX 140 (gen_wire.py first-branch bug).

Background
  web/page/ht9045_wire_*.js carry   kb: { <controlId>: ['<TYPE>', dp, checkRange, min, max], ... }
  produced by .claude/skills/ht9045-html-version/scripts/extract_page_v2.py (+ gen_wire.py).
  extract_page_v2.py sqk_by_handler() keeps the FIRST ShowQwertyKey line of each handler
  (out.setdefault), so when the handler branches (customer code, CosFunction, IniConfig,
  machine model, Sender dispatch) the generated entry can be a customer branch instead of
  golden's generic (final else) branch.  The hand-written *_c.js C-route scripts copied those
  tables ("var kb = {...}"), so they are audited too.

What it does, for every kb entry (wire files, the *_c.js "var kb" copies, the
ht9045_contact_wire.js / ht9045_hotplate_wire.js "var KB" tables; "var GOLDEN_KB" override
tables such as ht9045_testerif_c_wire.js block (8) only mark the entries they replace):
  1. picks the control's golden form (.dfm; the wire header names it), the control's
     OnClick / OnMouseDown / OnDblClick / OnEnter handler (same event set as the extractor),
  2. parses the handler body in the form's .cpp (comments, #if 0, nested if / else if /
     else / switch / return) and lists every live ShowQwertyKey call with its conditions,
  3. evaluates golden's GENERIC branch for a plain machine (ASSUMPTIONS below), per control
     (Sender / Ptr==ed... / ->Name dispatch is resolved per control; ->Tag comes from the .dfm),
  4. compares the generated entry with the generic call and grades the impact,
  5. works out which kb table each page really uses (the last HT9045Wire.register() in the
     page's script order wins; HTML comments are ignored).

ASSUMPTIONS (plain machine)
  CUSTOMER_CODE==CC_x -> false, != -> true (no specific customer; CC_HONPREC_QC = 0 build)
  Barcode_Reader(x)   == 2 (BarcodeReader.cpp:415-444: 0/1 only for CC_KYEC_LEE / KYEC_XILINX)
  CosFunction.b*      -> false (set only by the per-customer FUNC_CC_*() in CosFunction.cpp)
  IniConfig.b*        -> false (option flags at their default "off")
  MachineTypeChoice   == Type_HT9045 (HT9045 standard model)
  anything else (USE_* / HOT_PLATE_POSITION / ATC type / runtime page state) -> unknown:
  every side is explored; the REPRESENTATIVE generic branch is the path that takes the else
  side of every unknown condition ("all else"), the other variants are listed with their atoms.

Grades (generated vs generic, as the web page behaves: qwerty.js clamps with checkRange
and rounds DOUBLE to dp, INTEGER to an integer)
  HIGH  a value typed on a plain machine is clamped / rounded differently: narrower range,
        higher min / lower max, a range where golden has none, INTEGER where golden is
        DOUBLE, fewer decimals, numeric pad where golden is a text keyboard
  MED   wider range / no range where golden has one, DOUBLE where golden is INTEGER,
        display-only flags (PASSWORD / NO_SPACE ...), golden shows no keyboard
  LOW   same values; or the id is not a text input on the page; or the input is hidden on the
        page and Visible=False in the golden .dfm
  cause "branch" = the generator took another branch; "norm" = same branch, the
  difference comes from gen_wire's own normalisation (negative / variable range dropped).

Output: a summary on stdout; --md writes the tables (branch rows, per page, ownership, and the
"要重產的檔 → 欄位" per-wire-file list); --json / --tsv for tools.  Nothing is written unless
one of them is given.  Uses `git log` (read-only) for the ownership table unless --no-git.
A full run reads the whole golden tree twice (906 + 912) and takes about 80 s.

Usage (STEVEN-NB3, from the repo root):
  D:\HT9045\.venv\Scripts\python.exe HT9011UC_Cpp_V3.33.906.0\tools\webprobe\qwerty_first_branch_audit.py
      [--golden DIR] [--golden2 DIR] [--repo DIR] [--page DIR]
      [--md OUT.md] [--json OUT.json] [--tsv OUT.tsv] [--no-git] [--quiet]
"""
from __future__ import print_function

import argparse
import collections
import glob
import io
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
DEF_GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven'
DEF_GOLDEN2 = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
DEF_PAGE = os.path.join(REPO, 'web', 'page')

NUMERIC_FLAGS = {'INTEGER', 'DOUBLE', 'PORT', 'IP_ADDR'}
CLAMP_FLAGS_GOLDEN = {'INTEGER', 'DOUBLE'}          # myQwertyKeyBoard.cpp:285 clamps only these
EVENTS = ('OnMouseDown', 'OnClick', 'OnEnter', 'OnDblClick')   # extract_page_v2.py:38 RE_ONMD
SEV_RANK = {'LOW': 0, 'MED': 1, 'HIGH': 2}


# ---------------------------------------------------------------------------------------------
# text helpers
# ---------------------------------------------------------------------------------------------
def read_text(path):
    """Golden sources are Big5 (cp950); repo files are UTF-8."""
    with open(path, 'rb') as fh:
        raw = fh.read()
    if raw.startswith(b'\xef\xbb\xbf'):
        raw = raw[3:]
    try:
        return raw.decode('utf-8')
    except UnicodeDecodeError:
        return raw.decode('cp950', errors='replace')


def num(s):
    """'60' / '0.0' / '3000.0f' -> float, anything else (variables, casts) -> None."""
    if s is None:
        return None
    if isinstance(s, (int, float)):
        return float(s)
    t = str(s).strip()
    t = re.sub(r'[fFlLuU]+$', '', t)
    try:
        return float(t)
    except ValueError:
        return None


def fmt_num(v):
    if v is None:
        return '-'
    if isinstance(v, str):
        return v
    if float(v).is_integer():
        return str(int(v))
    return ('%g' % v)


# ---------------------------------------------------------------------------------------------
# C++ lexer (comments dropped, #if 0 regions dropped, directives recorded)
# ---------------------------------------------------------------------------------------------
TOK_RE = re.compile(r'''
   (?P<nl>\n)
  |(?P<ws>[ \t\r\f\v]+)
  |(?P<lc>//[^\n]*)
  |(?P<bc>/\*.*?\*/)
  |(?P<str>L?"(?:\\.|[^"\\\n])*")
  |(?P<chr>L?'(?:\\.|[^'\\\n])*')
  |(?P<num>0[xX][0-9a-fA-F]+[uUlL]*|(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?[uUlLfF]*)
  |(?P<id>[A-Za-z_]\w*)
  |(?P<op>->\*|<<=|>>=|\.\.\.|->|==|!=|<=|>=|&&|\|\||::|\+\+|--|\+=|-=|\*=|/=|%=|&=|\|=|\^=|<<|>>|[^\s])
''', re.S | re.X)


def _bc_state_after(line, in_bc):
    """Rough block-comment state tracking so '#' inside /* */ is not taken as a directive."""
    j, n = 0, len(line)
    while j < n:
        if in_bc:
            k = line.find('*/', j)
            if k < 0:
                return True
            in_bc, j = False, k + 2
            continue
        c = line[j]
        if c in '"\'':
            q, j = c, j + 1
            while j < n and line[j] != q:
                j += 2 if line[j] == '\\' else 1
            j += 1
        elif line.startswith('//', j):
            return False
        elif line.startswith('/*', j):
            in_bc, j = True, j + 2
        else:
            j += 1
    return in_bc


def split_directives(text):
    lines = text.split('\n')
    out, pp, in_bc, i = [], [], False, 0
    while i < len(lines):
        ln = lines[i]
        if not in_bc and ln.lstrip().startswith('#'):
            start, buf, blank = i + 1, ln, ['']
            while buf.rstrip().endswith('\\') and i + 1 < len(lines):
                i += 1
                buf = buf.rstrip()[:-1] + ' ' + lines[i]
                blank.append('')
            pp.append((start, buf.strip()))
            out.extend(blank)
        else:
            out.append(ln)
            in_bc = _bc_state_after(ln, in_bc)
        i += 1
    return '\n'.join(out), pp


def pp_regions(pp):
    """-> [(start_line, end_line, cond_text, dead)] for #if/#ifdef/#elif/#else/#endif."""
    regions, stack = [], []
    for line, d in pp:
        d2 = re.sub(r'/\*.*?\*/', '', re.sub(r'//.*', '', d)).strip()
        m = re.match(r'#\s*(\w+)\s*(.*)$', d2)
        if not m:
            continue
        kw, rest = m.group(1), m.group(2).strip()
        if kw in ('if', 'ifdef', 'ifndef'):
            cond = rest if kw == 'if' else ('defined(%s)' % rest if kw == 'ifdef' else '!defined(%s)' % rest)
            const = rest if (kw == 'if' and rest in ('0', '1')) else None
            stack.append(dict(cond='#' + kw + ' ' + rest, start=line, dead=(const == '0'), seen_true=(const == '1')))
        elif kw == 'elif' and stack:
            top = stack[-1]
            regions.append((top['start'], line, top['cond'], top['dead']))
            top.update(cond='#elif ' + rest, start=line, dead=top['seen_true'] or rest == '0')
            if rest == '1':
                top['seen_true'] = True
        elif kw == 'else' and stack:
            top = stack[-1]
            regions.append((top['start'], line, top['cond'], top['dead']))
            top.update(cond='#else of ' + top['cond'], start=line, dead=top['seen_true'])
        elif kw == 'endif' and stack:
            top = stack.pop()
            regions.append((top['start'], line, top['cond'], top['dead']))
    return regions


def lex(text):
    toks, line = [], 1
    for m in TOK_RE.finditer(text):
        k = m.lastgroup
        v = m.group(k)
        if k == 'nl':
            line += 1
        elif k in ('ws', 'lc'):
            continue
        elif k == 'bc':
            line += v.count('\n')
        else:
            toks.append((k, v, line))
    return toks


def match(toks, i, limit=None):
    """Index of the bracket closing toks[i], or None."""
    o = toks[i][1]
    c = {'(': ')', '{': '}', '[': ']'}[o]
    d, end = 0, len(toks) if limit is None else limit
    for j in range(i, end):
        if toks[j][0] != 'op':
            continue
        v = toks[j][1]
        if v == o:
            d += 1
        elif v == c:
            d -= 1
            if d == 0:
                return j
    return None


def tj(toks):
    """Join tokens: a space only between two word-like tokens."""
    s, prev = [], None
    for k, v, _ln in toks:
        if prev in ('id', 'num') and k in ('id', 'num'):
            s.append(' ')
        s.append(v)
        prev = k
    return ''.join(s)


class CppFile(object):
    def __init__(self, path):
        self.path = path
        self.text = read_text(path)
        self.lines = self.text.split('\n')
        body, pp = split_directives(self.text)
        self.regions = pp_regions(pp)
        dead = set()
        for a, b, _c, d in self.regions:
            if d:
                dead.update(range(a + 1, b))
        self.dead = dead
        self.toks = [t for t in lex(body) if t[2] not in dead]
        self.funcs = self._find_funcs()

    def _find_funcs(self):
        out = collections.defaultdict(list)
        t, i, n = self.toks, 0, len(self.toks)
        while i < n - 3:
            if (t[i][0] == 'id' and t[i + 1][1] == '::' and t[i + 2][0] == 'id' and t[i + 3][1] == '('
                    and i > 0 and t[i - 1][1] == '__fastcall'):
                rp = match(t, i + 3)
                if rp is not None and rp + 1 < n and t[rp + 1][1] == '{':
                    end = match(t, rp + 1)
                    if end is not None:
                        out[t[i + 2][1]].append(dict(cls=t[i][1], name=t[i + 2][1], line=t[i][2],
                                                     a=rp + 2, b=end, end_line=t[end][2],
                                                     params=t[i + 4:rp]))
                        i = end + 1
                        continue
            i += 1
        return out

    def live_regions(self, line):
        return [(a, c) for a, b, c, d in self.regions if not d and a < line < b]


# ---------------------------------------------------------------------------------------------
# statement parser -> small AST
# ---------------------------------------------------------------------------------------------
class Parser(object):
    def __init__(self, toks):
        self.t = toks
        self.chain = 0

    def stmts(self, a, b):
        out, i = [], a
        while i < b:
            s, j = self.stmt(i, b)
            if j <= i:
                j = i + 1
            if s is not None:
                out.append(s)
            i = j
        return out

    def _close(self, i, b):
        j = match(self.t, i, b)
        return j if j is not None else b - 1

    def stmt(self, i, b, chain=None):
        t = self.t
        k, v, ln = t[i]
        if k == 'op' and v == '{':
            j = self._close(i, b)
            return ('block', self.stmts(i + 1, j), ln), j + 1
        if k == 'id' and v == 'if' and i + 1 < b and t[i + 1][1] == '(':
            rp = self._close(i + 1, b)
            cond = t[i + 2:rp]
            if chain is None:
                self.chain += 1
                chain = self.chain
            then, j = self.stmt(rp + 1, b) if rp + 1 < b else (None, b)
            els, eln = None, None
            if j < b and t[j][0] == 'id' and t[j][1] == 'else':
                eln = t[j][2]
                if j + 1 < b and t[j + 1][1] == 'if':
                    els, j = self.stmt(j + 1, b, chain)          # else-if: same chain
                elif j + 1 < b:
                    els, j = self.stmt(j + 1, b)
                else:
                    j += 1
            return ('if', cond, ln, then, els, eln, chain), j
        if k == 'id' and v == 'switch' and i + 1 < b and t[i + 1][1] == '(':
            rp = self._close(i + 1, b)
            body, j = self.stmt(rp + 1, b) if rp + 1 < b else (None, b)
            return ('switch', t[i + 2:rp], ln, body), j
        if k == 'id' and v in ('for', 'while') and i + 1 < b and t[i + 1][1] == '(':
            rp = self._close(i + 1, b)
            body, j = self.stmt(rp + 1, b) if rp + 1 < b else (None, b)
            return ('loop', v, t[i + 2:rp], ln, body), j
        if k == 'id' and v == 'do':
            body, j = self.stmt(i + 1, b)
            cond = []
            if j < b and t[j][1] == 'while' and j + 1 < b and t[j + 1][1] == '(':
                rp = self._close(j + 1, b)
                cond, j = t[j + 2:rp], rp + 1
                if j < b and t[j][1] == ';':
                    j += 1
            return ('loop', 'do', cond, ln, body), j
        if k == 'id' and v in ('try', '__try'):
            body, j = self.stmt(i + 1, b)
            hs = []
            while j < b and t[j][1] in ('catch', '__except', '__finally'):
                if t[j][1] in ('catch', '__except') and j + 1 < b and t[j + 1][1] == '(':
                    j = self._close(j + 1, b) + 1
                else:
                    j += 1
                if j < b:
                    h, j = self.stmt(j, b)
                    hs.append(h)
            return ('try', body, hs, ln), j
        if k == 'id' and v == 'case':
            j, d = i + 1, 0
            while j < b:
                vv = t[j][1]
                if vv in ('(', '['):
                    d += 1
                elif vv in (')', ']'):
                    d -= 1
                elif vv == ':' and d == 0:
                    break
                j += 1
            return ('case', t[i + 1:j], ln), j + 1
        if k == 'id' and v == 'default' and i + 1 < b and t[i + 1][1] == ':':
            return ('default', ln), i + 2
        if k == 'op' and v == ';':
            return None, i + 1
        j, d = i, 0
        while j < b:
            kk, vv = t[j][0], t[j][1]
            if kk == 'op':
                if vv in ('(', '[', '{'):
                    d += 1
                elif vv in (')', ']', '}'):
                    d -= 1
                    if d < 0:
                        break
                elif vv == ';' and d == 0:
                    break
            j += 1
        kind = v if (k == 'id' and v in ('return', 'break', 'continue', 'goto', 'throw')) else 'expr'
        nxt = j + 1 if (j < b and t[j][1] == ';') else j
        return (kind, t[i:j], ln), max(nxt, i + 1)


def split_args(toks):
    parts, cur, d = [], [], 0
    for tk in toks:
        v = tk[1]
        if tk[0] == 'op':
            if v in ('(', '[', '{'):
                d += 1
            elif v in (')', ']', '}'):
                d -= 1
            elif v == ',' and d == 0:
                parts.append(cur)
                cur = []
                continue
        cur.append(tk)
    if cur or parts:
        parts.append(cur)
    return parts


def find_calls(toks):
    out = []
    for i, tk in enumerate(toks):
        if tk[0] == 'id' and tk[1] == 'ShowQwertyKey' and i + 1 < len(toks) and toks[i + 1][1] == '(':
            rp = match(toks, i + 1)
            if rp is None:
                continue
            args = [tj(a) for a in split_args(toks[i + 2:rp])]
            out.append(dict(line=tk[2], args=args))
    return out


# ---------------------------------------------------------------------------------------------
# conditions: 3-valued evaluation
# ---------------------------------------------------------------------------------------------
def split_top(toks, op):
    parts, cur, d = [], [], 0
    for tk in toks:
        v = tk[1]
        if tk[0] == 'op':
            if v in ('(', '['):
                d += 1
            elif v in (')', ']'):
                d -= 1
            elif v == op and d == 0:
                parts.append(cur)
                cur = []
                continue
        cur.append(tk)
    parts.append(cur)
    return parts


def parse_cond(toks):
    ors = split_top(toks, '||')
    if len(ors) > 1:
        return ('or', [parse_cond(p) for p in ors])
    ands = split_top(toks, '&&')
    if len(ands) > 1:
        return ('and', [parse_cond(p) for p in ands])
    if toks and toks[0][0] == 'op' and toks[0][1] == '!':
        return ('not', parse_cond(toks[1:]))
    if toks and toks[0][1] == '(' and match(toks, 0) == len(toks) - 1:
        return parse_cond(toks[1:-1])
    return ('atom', toks)


def norm_atom(text, aliases):
    s = text
    s = re.sub(r'dynamic_cast<\w+\*>\((\w+)\)', r'\1', s)
    s = re.sub(r'static_cast<\w+\*>\((\w+)\)', r'\1', s)
    s = re.sub(r'\(\w+\*\)', '', s)
    prev = None
    while prev != s:                                  # (x) -> x, but never a call's f(x)
        prev = s
        s = re.sub(r'(?<![\w\]])\((\w+)\)', r'\1', s)
    return s


def eval_atom(toks, ctx):
    """-> (value True/False/None, category, label)."""
    s = norm_atom(tj(toks), ctx['aliases'])
    al = '|'.join(re.escape(a) for a in ctx['aliases'])
    if s in ('true', '1'):
        return True, 'const', s
    if s in ('false', '0'):
        return False, 'const', s
    # Sender dispatch (always evaluated: it is per control, not a machine assumption)
    m = re.fullmatch(r'(%s)(==|!=)(\w+)' % al, s) or re.fullmatch(r'(\w+)(==|!=)(%s)' % al, s)
    if m:
        a, op, b = m.groups()
        other = b if a in ctx['aliases'] else a
        if other in ('NULL', '0', 'nullptr'):
            v = False
        else:
            v = (other == ctx['ctl'])
        return (v if op == '==' else not v), 'sender', s
    m = re.fullmatch(r'(%s)->Name(==|!=)(?:AnsiString\()?"([^"]*)"\)?' % al, s)
    if m:
        v = (m.group(3) == ctx['ctl'])
        return (v if m.group(2) == '==' else not v), 'sender', s
    m = re.fullmatch(r'(%s)->Tag(==|!=|<=|>=|<|>)(-?\d+)' % al, s)
    if m:
        tag, op, n = ctx['tag'], m.group(2), int(m.group(3))
        v = {'==': tag == n, '!=': tag != n, '<': tag < n, '>': tag > n, '<=': tag <= n, '>=': tag >= n}[op]
        return v, 'sender', s
    if re.fullmatch(r'(%s)' % al, s):
        return True, 'sender', s
    if ctx['mode'] == 'sender_only':
        return None, 'other', s
    # plain-machine assumptions
    m = re.fullmatch(r'Barcode_Reader\(\w+\)(==|!=)(\d+)', s)
    if m:
        # BarcodeReader.cpp:415-444: returns 2 unless USE_BARCODE_AS_KEYBOARD and CC_KYEC_LEE / CC_KYEC_XILINX
        v = (2 == int(m.group(2)))
        return (v if m.group(1) == '==' else not v), 'customer', s
    if 'CUSTOMER_CODE' in s:
        m = re.fullmatch(r'CUSTOMER_CODE(==|!=)(\w+)', s) or re.fullmatch(r'(\w+)(==|!=)CUSTOMER_CODE', s)
        if m:
            op = m.group(1) if m.group(1) in ('==', '!=') else m.group(2)
            return (op == '!='), 'customer', s
        return None, 'customer', s
    m = re.fullmatch(r'(CosFunction|IniConfig)\.(b\w+)(?:(==|!=)(true|false|1|0))?', s)
    if m:
        base = False
        op, lit = m.group(3), m.group(4)
        if op:
            want = lit in ('true', '1')
            v = (base == want) if op == '==' else (base != want)
        else:
            v = base
        return v, ('cos' if m.group(1) == 'CosFunction' else 'ini'), s
    if s.startswith('CosFunction.'):
        return None, 'cos', s
    if s.startswith('IniConfig.'):
        return None, 'ini', s
    m = re.fullmatch(r'MachineTypeChoice(==|!=)(\w+)', s)
    if m:
        v = (m.group(2) == 'Type_HT9045')
        return (v if m.group(1) == '==' else not v), 'model', s
    if 'MachineTypeChoice' in s:
        return None, 'model', s
    if re.match(r'[A-Z][A-Z0-9_]*(?![a-z])', s):
        return None, 'mech', s
    return None, 'runtime', s


def eval_node(node, ctx, atoms):
    k = node[0]
    if k == 'atom':
        v, cat, label = eval_atom(node[1], ctx)
        atoms.append((label, cat, v))
        return v
    if k == 'not':
        v = eval_node(node[1], ctx, atoms)
        return None if v is None else (not v)
    vals = [eval_node(n, ctx, atoms) for n in node[1]]
    if k == 'and':
        if any(v is False for v in vals):
            return False
        return True if all(v is True for v in vals) else None
    if any(v is True for v in vals):
        return True
    return False if all(v is False for v in vals) else None


def eval_cond(toks, ctx):
    atoms = []
    v = eval_node(parse_cond(toks), ctx, atoms)
    return v, atoms


# ---------------------------------------------------------------------------------------------
# handler analysis
# ---------------------------------------------------------------------------------------------
class Handler(object):
    def __init__(self, cpp, fdef):
        self.cpp = cpp
        self.fdef = fdef
        toks = cpp.toks
        self.body = toks[fdef['a']:fdef['b']]
        p = Parser(self.body)
        self.ast = p.stmts(0, len(self.body))
        self.calls = []                                   # live calls with static paths
        self._collect(self.ast, [])
        for c in self.calls:                              # #if regions around the call
            for a, cond in cpp.live_regions(c['line']):
                if a > fdef['line']:
                    c['path'] = [('pp', cond, a, True, 0)] + c['path']
        self.aliases = self._aliases()
        self.commented = self._commented_calls()
        self.extractor_first = self._extractor_first()

    def _aliases(self):
        names = {'Sender'}
        prm = tj(self.fdef['params'])
        m = re.search(r'TObject\*(\w+)', prm)
        if m:
            names.add(m.group(1))
        body = tj(self.body)
        for pat in (r'(\w+)=(?:\(\w+\*\))?(?:%s)\b' % '|'.join(names),
                    r'(\w+)=(?:dynamic_cast|static_cast)<\w+\*>\((?:%s)\)' % '|'.join(names)):
            for m in re.finditer(pat, body):
                if m.group(1) not in ('if', 'return'):
                    names.add(m.group(1))
        return sorted(names)

    def _collect(self, stmts, path):
        for s in stmts:
            if s is None:
                continue
            k = s[0]
            if k == 'block':
                self._collect(s[1], path)
            elif k == 'if':
                _, cond, ln, then, els, eln, chain = s
                if then is not None:
                    self._collect([then], path + [('if', cond, ln, True, chain)])
                if els is not None:
                    self._collect([els], path + [('if', cond, ln, False, chain)])
            elif k == 'switch':
                _, expr, ln, body = s
                items = body[1] if (body is not None and body[0] == 'block') else [body]
                labels, last_was_label = [], False
                for it in items:
                    if it is None:
                        continue
                    if it[0] in ('case', 'default'):
                        lab = 'default' if it[0] == 'default' else tj(it[1])
                        labels = (labels + [lab]) if last_was_label else [lab]
                        last_was_label = True
                        continue
                    last_was_label = False
                    self._collect([it], path + [('case', expr, ln, list(labels), 0)])
            elif k == 'loop':
                self._collect([s[4]], path + [('loop', s[2], s[3], True, 0)])
            elif k == 'try':
                self._collect([s[1]], path)
                for h in s[2]:
                    self._collect([h], path + [('catch', [], s[3], True, 0)])
            elif k in ('expr', 'return', 'throw'):
                for c in find_calls(s[1]):
                    c['path'] = list(path)
                    self.calls.append(c)

    def _commented_calls(self):
        """ShowQwertyKey text inside comments / dead #if 0 (the extractor reads raw lines)."""
        out = []
        live = {c['line'] for c in self.calls}
        for ln in range(self.fdef['line'], self.fdef['end_line'] + 1):
            raw = self.cpp.lines[ln - 1] if ln - 1 < len(self.cpp.lines) else ''
            if 'ShowQwertyKey' in raw and ln not in live:
                out.append(ln)
        return out

    def _extractor_first(self):
        """Emulate extract_page_v2.py sqk_by_handler on this function's raw lines (first match)."""
        for ln in range(self.fdef['line'], self.fdef['end_line'] + 1):
            raw = self.cpp.lines[ln - 1] if ln - 1 < len(self.cpp.lines) else ''
            m = RE_SQK_X.search(raw)
            if m:
                return ln, (extractor_norm(m, True), extractor_norm(m, False))
        return None, (None, None)

    # ---- per-control views ---------------------------------------------------------------
    def reachable(self, ctl, tag):
        ctx = dict(ctl=ctl, tag=tag, aliases=self.aliases, mode='sender_only')
        out = []
        for c in self.calls:
            ok, kept = True, []
            for e in c['path']:
                kind = e[0]
                if kind == 'if':
                    v, atoms = eval_cond(e[1], ctx)
                    if v is not None and v != e[3]:
                        ok = False
                        break
                    if v is not None and all(a[1] in ('sender', 'const') for a in atoms):
                        continue                          # the control's own dispatch: implied
                    kept.append(e)
                elif kind == 'case':
                    sv = switch_value(e[1], ctx)
                    if sv is not None and 'default' not in e[3]:
                        if not case_hits(sv, e[3]):
                            ok = False
                            break
                        continue
                    kept.append(e)
                else:
                    kept.append(e)
            if ok:
                out.append(dict(c, kept=kept))
        return out

    def generic(self, ctl, tag):
        """Abstract execution under the plain-machine assumptions -> variants."""
        ctx = dict(ctl=ctl, tag=tag, aliases=self.aliases, mode='generic')
        states = run_list(self.ast, [State()], ctx)
        variants = collections.OrderedDict()
        for st in states:
            first = st.calls[0] if st.calls else None
            key = first['line'] if first else None
            variants.setdefault(key, dict(call=first, asms=[]))
            variants[key]['asms'].append(st.asms)
        return list(variants.values())


class State(object):
    __slots__ = ('calls', 'status', 'asms')

    def __init__(self, calls=(), status='run', asms=()):
        self.calls, self.status, self.asms = tuple(calls), status, tuple(asms)

    def with_(self, calls=None, status=None, asm=None):
        return State(self.calls if calls is None else calls,
                     self.status if status is None else status,
                     self.asms + ((asm,) if asm is not None else ()))


MAX_STATES = 4096


def switch_value(expr, ctx):
    s = norm_atom(tj(expr), ctx['aliases'])
    al = '|'.join(re.escape(a) for a in ctx['aliases'])
    if re.fullmatch(r'(%s)->Tag' % al, s):
        return ctx['tag']
    if ctx['mode'] == 'generic' and s == 'MachineTypeChoice':
        return 'Type_HT9045'
    return None


def case_hits(val, labels):
    for lab in labels:
        if lab == 'default':
            continue
        if isinstance(val, int):
            if num(lab) is not None and int(num(lab)) == val:
                return True
        elif lab == val:
            return True
    return False


def run_list(stmts, states, ctx):
    for s in stmts:
        if s is None:
            continue
        nxt = []
        for st in states:
            if st.status != 'run':
                nxt.append(st)
            else:
                nxt.extend(run_stmt(s, st, ctx))
        states = nxt[:MAX_STATES]
    return states


def run_stmt(s, st, ctx):
    k = s[0]
    if k == 'block':
        return run_list(s[1], [st], ctx)
    if k == 'if':
        _, cond, ln, then, els, eln, chain = s
        v, atoms = eval_cond(cond, ctx)
        unk = [a[0] for a in atoms if a[2] is None]
        out = []
        if v is True or v is None:
            s1 = st.with_(asm=(tj(cond), ln, True, tuple(unk))) if v is None else st
            out.extend(run_list([then], [s1], ctx) if then is not None else [s1])
        if v is False or v is None:
            s2 = st.with_(asm=(tj(cond), ln, False, tuple(unk))) if v is None else st
            out.extend(run_list([els], [s2], ctx) if els is not None else [s2])
        return out
    if k == 'switch':
        _, expr, ln, body = s
        items = [it for it in (body[1] if (body is not None and body[0] == 'block') else [body]) if it is not None]
        sv = switch_value(expr, ctx)
        starts = []
        for idx, it in enumerate(items):
            if it[0] == 'case':
                starts.append((idx, [tj(it[1])]))
            elif it[0] == 'default':
                starts.append((idx, ['default']))
        entries = []
        if sv is not None:
            hit = None
            for idx, labs in starts:
                if labs != ['default'] and case_hits(sv, labs):
                    hit = idx
                    break
            if hit is None:
                hit = next((idx for idx, labs in starts if labs == ['default']), None)
            entries.append((hit, None))
        else:
            for idx, labs in starts:
                entries.append((idx, ('switch(%s) case %s' % (tj(expr), labs[0]), ln, True, (tj(expr),))))
            if not any(labs == ['default'] for _i, labs in starts):
                entries.append((None, ('switch(%s) no case' % tj(expr), ln, True, (tj(expr),))))
        out = []
        for idx, asm in entries:
            s2 = st.with_(asm=asm) if asm else st
            if idx is None:
                out.append(s2)
                continue
            rest = [it for it in items[idx:] if it[0] not in ('case', 'default')]
            for x in run_list(rest, [s2], ctx):
                out.append(x.with_(status='run') if x.status == 'brk' else x)
        return out
    if k == 'loop':
        res = run_list([s[4]], [st], ctx) if s[4] is not None else [st]
        return [x.with_(status='run') if x.status in ('brk', 'cont') else x for x in res]
    if k == 'try':
        return run_list([s[1]], [st], ctx) if s[1] is not None else [st]
    if k in ('expr', 'return', 'throw'):
        calls = find_calls(s[1])
        st2 = st.with_(calls=st.calls + tuple(calls)) if calls else st
        if k in ('return', 'throw'):
            return [st2.with_(status='ret')]
        return [st2]
    if k == 'break':
        return [st.with_(status='brk')]
    if k == 'continue':
        return [st.with_(status='cont')]
    return [st]


# ---------------------------------------------------------------------------------------------
# generator emulation (extract_page_v2.py:43 + :163-184, gen_wire.py:302-318)
# ---------------------------------------------------------------------------------------------
RE_SQK_X = re.compile(r'ShowQwertyKey\s*\(\s*[^,]+,\s*N_(\w+)((?:\s*,\s*[^,()]+)*)\s*\)')
NUM_X = re.compile(r'^-?\d+(\.\d+)?$')


def gen_wire_norm(flags, dp, cr, mn, mx, drop_negative=True):
    """gen_wire.py:304-318.  drop_negative=False = the 20260915 files written before that rule."""
    dp = dp if dp not in (None, '') else 0
    mn = mn if mn not in (None, '') else 0
    mx = mx if mx not in (None, '') else 0
    try:
        if drop_negative and cr and (float(mn) < 0 or float(mx) <= 0):
            cr, mn, mx = False, 0, 0
    except (TypeError, ValueError):
        cr, mn, mx = False, 0, 0
    return (flags, float(dp), bool(cr), float(mn), float(mx))


def extractor_norm(m, drop_negative=True):
    rest = [a.strip() for a in m.group(2).split(',') if a.strip()]
    dp = rest[0] if rest else None
    cr = rest[1] if len(rest) > 1 else 'false'
    mn = rest[2] if len(rest) > 2 else None
    mx = rest[3] if len(rest) > 3 else None
    if not (mn and mx and NUM_X.match(mn) and NUM_X.match(mx)):
        cr, mn, mx = 'false', None, None
    if dp is not None and not NUM_X.match(dp):
        dp = None
    return gen_wire_norm(m.group(1), dp, cr == 'true', mn, mx, drop_negative)


def call_norm(call, drop_negative=True):
    """What the generator would have emitted for this call (None: the extractor cannot read it)."""
    text = 'ShowQwertyKey(' + ', '.join(call['args']) + ')'
    m = RE_SQK_X.search(text)
    return extractor_norm(m, drop_negative) if m else None


def _same(a, b):
    """Same kb entry as the page behaves: min/max order does not matter (golden CheckRange and
    qwerty.js both sort them) and min/max are ignored when checkRange is false."""
    if a is None or b is None:
        return False
    if a[0] != b[0] or a[2] != b[2] or abs(a[1] - b[1]) > 1e-9:
        return False
    if not a[2]:
        return True
    ra, rb = sorted((a[3], a[4])), sorted((b[3], b[4]))
    return all(abs(x - y) < 1e-9 for x, y in zip(ra, rb))


def same_entry(a, b):
    return _same(a, b)


def call_matches(call, ent):
    """The generated entry equals this call as gen_wire emits it (with or without the negative-range rule)."""
    return _same(call_norm(call, True), ent) or _same(call_norm(call, False), ent)


def call_text(call):
    if call is None:
        return '(無小鍵盤)'
    a = call['args'][1:]
    return ', '.join(a) + ' :%d' % call['line']


def golden_eff(call):
    a = call['args']
    flags = set(f.strip()[2:] if f.strip().startswith('N_') else f.strip() for f in (a[1] if len(a) > 1 else '').split('|'))
    dp = num(a[2]) if len(a) > 2 else 0.0
    if len(a) > 2 and dp is None:
        dp = 'runtime'
    cr = a[3] if len(a) > 3 else 'false'
    mn = num(a[4]) if len(a) > 4 else 0.0
    mx = num(a[5]) if len(a) > 5 else 0.0
    crv = True if cr == 'true' else (False if cr == 'false' else None)
    rng = None
    if flags & CLAMP_FLAGS_GOLDEN:
        if crv is True:
            rng = 'runtime' if (mn is None or mx is None) else (min(mn, mx), max(mn, mx))
        elif crv is None:
            rng = 'runtime'
    return dict(flags=flags, dp=dp, rng=rng)


def gen_eff(entry):
    flags = set(entry[0].split('|'))
    rng = (min(entry[3], entry[4]), max(entry[3], entry[4])) if (entry[2] and flags & NUMERIC_FLAGS) else None
    return dict(flags=flags, dp=entry[1], rng=rng)


def compare(gen, gold):
    """-> (severity, [diff texts])."""
    sev, diffs = 'LOW', []

    def bump(s):
        nonlocal sev
        if SEV_RANK[s] > SEV_RANK[sev]:
            sev = s

    gf, of = gen['flags'], gold['flags']
    gnum, onum = bool(gf & NUMERIC_FLAGS), bool(of & NUMERIC_FLAGS)
    if gf != of:
        diffs.append('型別 %s≠%s' % ('|'.join(sorted(gf)), '|'.join(sorted(of))))
        if 'INTEGER' in gf and 'DOUBLE' in of:
            bump('HIGH')
        elif gnum and not onum:
            bump('HIGH')
        else:
            bump('MED')
    if 'DOUBLE' in gf and 'DOUBLE' in of and gold['dp'] != gen['dp'] and gen['rng'] is None:
        # qwerty.js:68-71 rounds to dp only inside the checkRange branch
        diffs.append('小數位 %s≠%s（頁面沒夾限就不四捨五入，無作用）' % (fmt_num(gen['dp']), fmt_num(gold['dp'])))
    elif 'DOUBLE' in gf and 'DOUBLE' in of and gold['dp'] != gen['dp']:
        if gold['dp'] == 'runtime':
            diffs.append('小數位 %s≠(變數)' % fmt_num(gen['dp']))
            bump('MED')
        else:
            diffs.append('小數位 %s≠%s' % (fmt_num(gen['dp']), fmt_num(gold['dp'])))
            bump('HIGH' if gen['dp'] < gold['dp'] else 'MED')
    gr, orr = gen['rng'], gold['rng']
    if orr == 'runtime':
        if gr is None:
            diffs.append('範圍 無≠(golden 執行期變數)')
            bump('MED')
        else:
            diffs.append('範圍 %s~%s≠(golden 執行期變數)' % (fmt_num(gr[0]), fmt_num(gr[1])))
            bump('HIGH')
    elif gr is None and orr is not None:
        diffs.append('範圍 無≠%s~%s' % (fmt_num(orr[0]), fmt_num(orr[1])))
        bump('MED')
    elif gr is not None and orr is None:
        diffs.append('範圍 %s~%s≠無' % (fmt_num(gr[0]), fmt_num(gr[1])))
        bump('HIGH')
    elif gr is not None and orr is not None and gr != orr:
        diffs.append('範圍 %s~%s≠%s~%s' % (fmt_num(gr[0]), fmt_num(gr[1]), fmt_num(orr[0]), fmt_num(orr[1])))
        bump('HIGH' if (gr[0] > orr[0] or gr[1] < orr[1]) else 'MED')
    return sev, diffs


# ---------------------------------------------------------------------------------------------
# path rendering
# ---------------------------------------------------------------------------------------------
def render_path(entries):
    """Static path -> 'cond(:L) > elif cond(:L) > else of cond(:L)'."""
    parts, i = [], 0
    while i < len(entries):
        e = entries[i]
        kind = e[0]
        if kind == 'if' and e[3] is False:
            chain, j = e[4], i
            while j < len(entries) and entries[j][0] == 'if' and entries[j][4] == chain and entries[j][3] is False:
                j += 1
            nxt = entries[j] if j < len(entries) else None
            if nxt is not None and nxt[0] == 'if' and nxt[4] == chain and nxt[3] is True:
                parts.append('elif %s(:%d)' % (tj(nxt[1]), nxt[2]))
                i = j + 1
            else:
                last = entries[j - 1]
                if j - i == 1:
                    parts.append('else of %s(:%d)' % (tj(last[1]), last[2]))
                else:
                    parts.append('else(:%d)' % last[2])
                i = j
            continue
        if kind == 'if':
            parts.append('%s(:%d)' % (tj(e[1]), e[2]))
        elif kind == 'case':
            parts.append('switch(%s) case %s(:%d)' % (tj(e[1]), '/'.join(e[3]), e[2]))
        elif kind == 'pp':
            parts.append('%s(:%d)' % (e[1], e[2]))
        elif kind == 'loop':
            parts.append('loop(%s)(:%d)' % (tj(e[1]), e[2]))
        else:
            parts.append(kind)
        i += 1
    return ' > '.join(parts) if parts else '無條件'


# ---------------------------------------------------------------------------------------------
# golden index
# ---------------------------------------------------------------------------------------------
RE_DFM_OBJ = re.compile(r'^\s*(object|inherited|inline)\s+([A-Za-z_]\w*)\s*:\s*(\w+)')
RE_DFM_EV = re.compile(r'^\s*(On\w+)\s*=\s*([A-Za-z_]\w*)\s*$')
RE_DFM_TAG = re.compile(r'^\s*Tag\s*=\s*(-?\d+)\s*$')
RE_DFM_VIS = re.compile(r'^\s*Visible\s*=\s*(True|False)\s*$')


class Dfm(object):
    def __init__(self, path):
        self.path = path
        self.objs = {}
        self.form_class = None
        stack = []
        for ln_no, line in enumerate(read_text(path).split('\n'), 1):
            s = line.strip()
            m = RE_DFM_OBJ.match(line)
            if m:
                name, typ = m.group(2), m.group(3)
                if self.form_class is None:
                    self.form_class = typ
                self.objs.setdefault(name, dict(type=typ, line=ln_no, events=[], tag=0, visible=True))
                stack.append(name)
                continue
            if s == 'item':
                stack.append(None)
                continue
            if s == 'end' or s.startswith('end>'):
                if stack:
                    stack.pop()
                continue
            cur = next((x for x in reversed(stack) if x), None)
            if not cur:
                continue
            m = RE_DFM_EV.match(line)
            if m:
                self.objs[cur]['events'].append((m.group(1), m.group(2), ln_no))
                continue
            if stack and stack[-1] == cur:
                m = RE_DFM_TAG.match(line)
                if m:
                    self.objs[cur]['tag'] = int(m.group(1))
                m = RE_DFM_VIS.match(line)
                if m:
                    self.objs[cur]['visible'] = (m.group(1) == 'True')


class Golden(object):
    def __init__(self, root):
        self.root = root
        self.dfms = {}
        self.funcidx = collections.defaultdict(list)
        self._cpp = {}
        self._handlers = {}
        for dp, dn, fn in os.walk(root):
            if '.svn' in dp.split(os.sep):
                continue
            for f in fn:
                p = os.path.join(dp, f)
                fl = f.lower()
                if fl.endswith('.dfm'):
                    try:
                        self.dfms[p] = Dfm(p)
                    except (OSError, UnicodeError):
                        pass
                elif fl.endswith('.cpp'):
                    try:
                        txt = read_text(p)
                    except OSError:
                        continue
                    for m in re.finditer(r'__fastcall\s+(\w+)\s*::\s*(\w+)\s*\(', txt):
                        self.funcidx[(m.group(1), m.group(2))].append(p)

    def rel(self, p):
        return os.path.relpath(p, self.root).replace(os.sep, '/')

    def cpp(self, p):
        if p not in self._cpp:
            self._cpp[p] = CppFile(p)
        return self._cpp[p]

    def pick_form(self, ids, header_form):
        best, score = None, -1
        for p, d in self.dfms.items():
            n = len(ids & set(d.objs))
            if n == 0:
                continue
            # the wire header names the form the generator used: it wins whenever it overlaps
            bonus = 100000 if (header_form and os.path.basename(p).lower() == header_form.lower()) else 0
            if n + bonus > score:
                best, score = p, n + bonus
        return best

    def handler(self, dfm_path, cls, name):
        key = (dfm_path, cls, name)
        if key in self._handlers:
            return self._handlers[key]
        primary = os.path.splitext(dfm_path)[0] + '.cpp'
        paths = list(self.funcidx.get((cls, name), []))
        paths.sort(key=lambda p: 0 if os.path.normcase(p) == os.path.normcase(primary) else 1)
        h = None
        for p in paths:
            cf = self.cpp(p)
            for fd in cf.funcs.get(name, []):
                if fd['cls'] == cls:
                    h = Handler(cf, fd)
                    break
            if h:
                break
        if h is None and os.path.isfile(primary):
            cf = self.cpp(primary)                       # fallback: same name, any class, in the form's .cpp
            if cf.funcs.get(name):
                h = Handler(cf, cf.funcs[name][0])
        self._handlers[key] = h
        return h


# ---------------------------------------------------------------------------------------------
# web side: kb tables, page load order, ownership
# ---------------------------------------------------------------------------------------------
RE_KB_ENTRY = re.compile(r"^\s*([A-Za-z_]\w*)\s*:\s*\[\s*'([A-Z_|]+)'\s*,\s*([^,\]]+?)\s*,\s*(true|false)\s*,"
                         r"\s*([^,\]]+?)\s*,\s*([^,\]]+?)\s*\]")
KB_STARTS = [(re.compile(r'^\s*kb\s*:\s*\{\s*$'), 'wire'),
             (re.compile(r'^\s*var\s+kb\s*=\s*\{'), 'copy'),
             # ht9045_contact_wire.js / ht9045_hotplate_wire.js: own KB table + own mousedown listener
             (re.compile(r'^\s*var\s+KB\s*=\s*\{'), 'parallel'),
             (re.compile(r'^\s*var\s+GOLDEN_KB\s*=\s*\{'), 'override')]


def parse_kb_tables(path):
    """-> list of (kind, start_line, {ctl: (entry tuple, line)})."""
    lines = read_text(path).split('\n')
    out, i = [], 0
    while i < len(lines):
        kind = None
        for rx, kd in KB_STARTS:
            if rx.match(lines[i]):
                kind = kd
                break
        if not kind:
            i += 1
            continue
        start, tab, j = i + 1, collections.OrderedDict(), i
        if lines[i].rstrip().endswith('{') is False and '}' in lines[i]:
            i += 1
            continue
        j = i + 1
        while j < len(lines) and not re.match(r'^\s*\}', lines[j]):
            m = RE_KB_ENTRY.match(lines[j])
            if m:
                ctl, ty, dp, cr, mn, mx = m.groups()
                tab[ctl] = ((ty, float(num(dp) or 0), cr == 'true', float(num(mn) or 0), float(num(mx) or 0)), j + 1)
            j += 1
        if tab:
            out.append((kind, start, tab))
        i = j + 1
    return out


def header_info(path):
    txt = read_text(path)
    form = None
    m = re.search(r'來源表單\s+(\S+\.dfm)', txt)
    if m:
        form = m.group(1)
    page = None
    m = re.search(r"^\s*page\s*:\s*'([^']+)'", txt, re.M)
    if m:
        page = m.group(1)
    base = os.path.basename(path)
    if base.startswith('ht9045_wire_') and 'AI(W906-FW-GEN)' in txt:
        gen = 'gen_wire'
    elif base.startswith('ht9045_wire_') and 'merge_wire' in txt:
        gen = 'merge_wire'
    elif 'gen_wire' in txt:
        gen = 'hand (kb copied from gen_wire)'
    else:
        gen = 'hand'
    return dict(form=form, page=page, gen=gen, register=('HT9045Wire.register(' in txt))


def page_scripts(html_path):
    txt = re.sub(r'<!--.*?-->', '', read_text(html_path), flags=re.S)
    return re.findall(r'<script[^>]*\bsrc\s*=\s*"([^"]+)"', txt, re.I)


def page_text_inputs(html_path):
    """ids the engine can bind: input[type="text"], input:not([type]) (ht9045_wire_engine.js attachKeyboards).
    -> {id: hidden_at_load}; hidden = display:none on the input or on its dfm2web wrapper tag."""
    txt = re.sub(r'<!--.*?-->', '', read_text(html_path), flags=re.S)
    ids = {}
    for m in re.finditer(r'<\s*input\b([^>]*)>', txt, re.I | re.S):
        attrs = m.group(1)
        t = re.search(r'\btype\s*=\s*["\']([^"\']+)["\']', attrs, re.I)
        if t and t.group(1).lower() != 'text':
            continue
        i = re.search(r'\bid\s*=\s*["\']([^"\']+)["\']', attrs, re.I)
        if not i:
            continue
        hidden = bool(re.search(r'display\s*:\s*none', attrs, re.I))
        before = txt[max(0, m.start() - 400):m.start()]
        k = max(before.rfind('<span'), before.rfind('<div'))
        if k >= 0 and not re.search(r'</(span|div)>', before[k:]):
            tag = before[k:before.find('>', k) + 1]
            hidden = hidden or bool(re.search(r'display\s*:\s*none', tag, re.I))
        ids[i.group(1)] = hidden
    return ids


def git_log(path, n=5):
    try:
        out = subprocess.check_output(['git', '-C', REPO, 'log', '-%d' % n, '--format=%h|%an|%ad|%s',
                                       '--date=short', '--', path], stderr=subprocess.STDOUT)
        return [ln.split('|', 3) for ln in out.decode('utf-8', 'replace').splitlines() if ln.strip()]
    except (OSError, subprocess.CalledProcessError):
        return []


def owner_of(commits):
    tags = []
    for h, an, ad, s in commits:
        if re.search(r'\bSt02\b|St02-E|St02-M', s):
            t = 'St02'
        elif re.search(r'\bSt01\b|st01-|ST01', s):
            t = 'St01'
        elif an.startswith('jimmychiu'):
            t = '筆電(jimmychiu)'
        elif an.startswith('HT9045 Machine') or an.startswith('EastSun'):
            t = '機台端(EastSun)'
        else:
            t = 'Steven(未標)'
        if t not in tags:
            tags.append(t)
    return tags


# ---------------------------------------------------------------------------------------------
# main audit
# ---------------------------------------------------------------------------------------------
def audit(args):
    G = Golden(args.golden)
    G2 = Golden(args.golden2) if (args.golden2 and os.path.isdir(args.golden2)) else None
    page_dir = args.page

    # load order -> which register wins on each page
    html_scripts, html_inputs = {}, {}
    for h in sorted(glob.glob(os.path.join(page_dir, '*.html'))):
        html_scripts[os.path.basename(h)] = page_scripts(h)
        html_inputs[os.path.basename(h)] = page_text_inputs(h)
    js_info = {}
    for js in sorted(glob.glob(os.path.join(page_dir, '*.js'))):
        b = os.path.basename(js)
        if b == 'ht9045_wire_engine.js':
            continue
        tabs = parse_kb_tables(js)
        hi = header_info(js)
        if tabs or b.startswith('ht9045_wire_'):
            js_info[b] = dict(path=js, tabs=tabs, **hi)
    loaded_by = collections.defaultdict(list)
    for h, scripts in html_scripts.items():
        regs = [s for s in scripts if s in js_info and js_info[s]['register']]
        for s in scripts:
            if s in js_info:
                st = 'loaded'
                if js_info[s]['register']:
                    st = 'effective' if (regs and s == regs[-1]) else 'shadowed'
                loaded_by[s].append((h, st))

    # overrides (tables applied after the last register, e.g. testerif (8))
    overrides = collections.defaultdict(dict)       # page -> ctl -> (file, line, entry)
    for b, ji in js_info.items():
        for kind, start, tab in ji['tabs']:
            if kind != 'override':
                continue
            for h, _st in loaded_by.get(b, []):
                for ctl, (ent, ln) in tab.items():
                    overrides[h][ctl] = (b, ln, ent)

    # pick the form per page from the wire header(s)
    page_form = {}
    for b, ji in js_info.items():
        pages = [h for h, _ in loaded_by.get(b, [])] or ([ji['page']] if ji['page'] else [])
        for pg in pages:
            if ji['form'] and pg not in page_form:
                page_form[pg] = ji['form']

    entries = []
    for b, ji in sorted(js_info.items()):
        for kind, start, tab in ji['tabs']:
            if kind == 'override':
                continue
            pages = loaded_by.get(b) or [(ji['page'] or '?', 'not-loaded')]
            ids = set(tab)
            for pg, status in pages:
                if kind == 'parallel' and status == 'loaded':
                    # both listeners fire on mousedown; qwerty.js show() closes the open keypad first,
                    # and the engine attached later (its DOMContentLoaded listener is added later) -> engine wins
                    regs = [s for s in html_scripts.get(pg, []) if s in js_info and js_info[s]['register']]
                    status = 'shadowed' if regs else 'effective'
                hdr = ji['form'] or page_form.get(pg)
                dfm_path = G.pick_form(ids, hdr)
                for ctl, (ent, ln) in tab.items():
                    entries.append(audit_entry(G, G2, b, ji, kind, pg, status, dfm_path, ctl, ent, ln,
                                               overrides.get(pg, {}).get(ctl), html_inputs.get(pg)))
    return G, js_info, loaded_by, html_scripts, entries


def audit_entry(G, G2, src, ji, kind, page, status, dfm_path, ctl, ent, ln, ovr, inputs=None):
    e = dict(page=page, src=src, src_kind=kind, gen=ji['gen'], status=status, ctl=ctl, line=ln,
             generated=ent, override=ovr, form=(G.rel(dfm_path) if dfm_path else None))
    if not dfm_path:
        e['verdict'] = 'no-form'
        return e
    d = G.dfms[dfm_path]
    obj = d.objs.get(ctl)
    if obj is None:
        for p, dd in G.dfms.items():
            if ctl in dd.objs and any(ev in EVENTS for ev, _h, _l in dd.objs[ctl]['events']):
                d, obj, dfm_path = dd, dd.objs[ctl], p
                e['form'] = G.rel(p) + ' (控制項不在主表單)'
                break
    if obj is None:
        e['verdict'] = 'no-control'
        return e
    hs = []
    for ev, hn, evl in obj['events']:
        if ev not in EVENTS:
            continue
        h = G.handler(dfm_path, d.form_class, hn)
        if h is not None and h.calls:
            hs.append((ev, hn, evl, h))
    if not hs:
        e['verdict'] = 'no-handler'
        e['events'] = ['%s=%s(:%d)' % (ev, hn, evl) for ev, hn, evl in obj['events']]
        return e
    ev, hn, evl, h = hs[0]
    e['other_handlers'] = ['%s=%s' % (x[0], x[1]) for x in hs[1:]]
    e['event'] = '%s %s=%s' % ('%s:%d' % (G.rel(dfm_path), evl), ev, hn)
    e['handler'] = '%s:%d %s' % (G.rel(h.cpp.path), h.fdef['line'], hn)
    e['handler_key'] = (G.rel(h.cpp.path), h.fdef['line'], hn)
    e['n_calls'] = len(h.calls)
    e['conditional'] = (len(h.calls) > 1) or any(c['path'] for c in h.calls)
    reach = h.reachable(ctl, obj.get('tag', 0))
    e['reach'] = [dict(line=c['line'], text=call_text(c), cond=render_path(c['kept'])) for c in reach]
    e['n_reach'] = len(reach)
    # which call(s) the generated entry equals
    hits = [c['line'] for c in h.calls if call_matches(c, ent)]
    e['gen_hits'] = hits
    fl, fnorm = h.extractor_first
    e['extractor_first'] = fl
    e['extractor_first_same'] = _same(fnorm[0], ent) or _same(fnorm[1], ent)
    e['commented'] = h.commented
    e['is_input'] = (inputs is None) or (ctl in inputs)
    e['hidden'] = bool(inputs) and bool(inputs.get(ctl))
    e['golden_hidden'] = not obj.get('visible', True)
    if not hits and G2 is not None:
        e['g2'] = golden2_hits(G, G2, h, ent)
    # generic branch: per control, plain-machine assumptions; conditions that stay unknown
    # (mechanism config / page state) -> the representative is the path that takes the
    # else side of every one of them ("all else"); the other variants are listed.
    variants = h.generic(ctl, obj.get('tag', 0))
    e['generic'] = []
    rep = None
    for v in variants:
        c = v['call']
        unk = sorted(set(a for asms in v['asms'] for asm in asms for a in asm[3]))
        item = dict(text=call_text(c), line=(c['line'] if c else None), unknown=unk,
                    args=(c['args'] if c else None),
                    all_else=any(all(is_else(a) for a in asms) for asms in v['asms']))
        if c is None:
            item['sev'], item['diffs'] = 'MED', ['golden 一般機不開小鍵盤']
            item['cause'] = 'branch'
        else:
            item['sev'], item['diffs'] = compare(gen_eff(ent), golden_eff(c))
            if item['diffs']:
                item['cause'] = 'norm' if call_matches(c, ent) else 'branch'
        e['generic'].append(item)
        if rep is None and item['all_else']:
            rep = item
    if rep is None:
        rep = max(e['generic'], key=lambda x: SEV_RANK[x['sev']])
    e['generic'].sort(key=lambda x: 0 if x is rep else 1)
    sev_all, diffs_all = rep['sev'], list(rep['diffs'])
    causes = {rep['cause']} if rep.get('cause') else set()
    others = [x for x in e['generic'] if x is not rep]
    e['sev_other'] = max([x['sev'] for x in others], key=lambda s: SEV_RANK[s]) if others else None
    if not e['is_input'] and diffs_all:
        sev_all = 'LOW'                                   # the engine only binds <input type=text>
        diffs_all = diffs_all + ['頁面上不是文字框，引擎不掛小鍵盤（無作用）']
    elif e['hidden'] and e['golden_hidden'] and diffs_all:
        sev_all = 'LOW'                                   # e.g. iosetview edtTemp: a hidden temp editor
        diffs_all = diffs_all + ['頁面與 golden .dfm 都隱藏（Visible=False），點不到']
    e['sev'] = sev_all
    e['diffs'] = diffs_all
    e['cause'] = 'branch' if 'branch' in causes else ('norm' if causes else '')
    e['norm_kind'] = norm_kind(rep.get('args'))
    e['gold_var'] = bool(rep.get('args')) and golden_eff(dict(args=rep['args']))['rng'] == 'runtime'
    e['varies'] = len(variants) > 1
    e['verdict'] = 'ok'
    return e


def norm_kind(args):
    """Why gen_wire would lose this golden call's range: 'neg' (gen_wire.py:309-315 drops min<0 /
    max<=0 -- golden myQwertyKeyBoard.cpp:249-257 does that only for N_PORT), 'var' (bounds or dp
    are C++ expressions), '' (nothing lost)."""
    if not args or len(args) < 4 or args[3] != 'true':
        return ''
    mn = num(args[4]) if len(args) > 4 else 0.0
    mx = num(args[5]) if len(args) > 5 else 0.0
    if mn is None or mx is None or (len(args) > 2 and num(args[2]) is None):
        return 'var'
    if mn < 0 or mx <= 0:
        return 'neg'
    return ''


def is_else(asm):
    """An assumption taken on the 'else' side (if false / switch default / no case)."""
    text, _ln, pol, _unk = asm
    if text.startswith('switch('):
        return text.endswith(' case default') or text.endswith(' no case')
    return pol is False


def golden2_hits(G, G2, h, ent):
    rel = G.rel(h.cpp.path)
    p2 = os.path.join(G2.root, rel.replace('/', os.sep))
    if not os.path.isfile(p2):
        return None
    cf = G2.cpp(p2)
    for fd in cf.funcs.get(h.fdef['name'], []):
        if fd['cls'] == h.fdef['cls']:
            h2 = Handler(cf, fd)
            hits = [c['line'] for c in h2.calls if call_matches(c, ent)]
            fl, fn = h2.extractor_first
            return dict(hits=hits, first=fl, first_same=(_same(fn[0], ent) or _same(fn[1], ent)), line=fd['line'])
    return None


# ---------------------------------------------------------------------------------------------
# output
# ---------------------------------------------------------------------------------------------
def ent_text(ent):
    return '%s %s %s %s~%s' % (ent[0], fmt_num(ent[1]), 'true' if ent[2] else 'false', fmt_num(ent[3]), fmt_num(ent[4]))


def gen_which(e):
    hits = e.get('gen_hits') or []
    fl = e.get('extractor_first')
    if hits:
        if fl in hits:
            more = len(hits) - 1
            return '＝第一個呼叫 :%d%s' % (fl, ('（另 %d 個呼叫經產生器正規化後同值）' % more) if more else '')
        if fl in (e.get('commented') or []) and e.get('extractor_first_same'):
            return '＝:%s（產生器其實抄到註解掉的 :%d，值碰巧相同）' % ('/:'.join(str(x) for x in hits[:3]), fl)
        return '＝:%s' % '/:'.join(str(x) for x in hits[:3])
    g2 = e.get('g2')
    if g2 and g2.get('hits'):
        return '906 無對應；＝912 :%s（912 第一個呼叫）' % '/:'.join(str(x) for x in g2['hits'][:3]) \
            if g2.get('first') in g2['hits'] else '906 無對應；＝912 :%s' % '/:'.join(str(x) for x in g2['hits'][:3])
    if e.get('extractor_first_same'):
        return '＝註解掉的 :%d' % fl
    return '906／912 都無對應'


def short_unknown(u, n=3):
    return '、'.join(u[:n]) + ('…等 %d 個' % len(u) if len(u) > n else '')


def generic_text(e):
    gs = e.get('generic') or []
    if len(gs) == 1:
        return gs[0]['text']
    out = ['%s（未知條件全取 else）' % gs[0]['text']]
    for g in gs[1:]:
        out.append('其他：%s［依 %s］' % (g['text'], short_unknown(g['unknown']) or '?'))
    return '<br>'.join(out)


def status_text(st):
    return {'effective': '生效', 'shadowed': '被後載蓋掉', 'not-loaded': '沒有頁面載入', 'loaded': '載入'}.get(st, st)


def build_rows(entries):
    """Group entries that tell the same story (same page/handler/branches/result) into one row."""
    rows = collections.OrderedDict()
    for e in entries:
        if e.get('verdict') != 'ok' or not e.get('conditional'):
            continue
        key = (e['page'], e['handler'], tuple((r['text'], r['cond']) for r in e['reach']), e['generated'],
               generic_text(e), e['sev'], tuple(e['diffs']), gen_which(e),
               bool(e.get('override')))
        r = rows.get(key)
        if r is None:
            r = rows[key] = dict(page=e['page'], handler=e['handler'], event=e['event'], reach=e['reach'],
                                 generated=e['generated'], gen_which=gen_which(e), generic=generic_text(e),
                                 sev=e['sev'], diffs=e['diffs'], cause=e['cause'], varies=e['varies'],
                                 sev_other=e.get('sev_other'), is_input=e['is_input'],
                                 ctls=[], srcs=collections.OrderedDict(), override=e.get('override'),
                                 n_calls=e['n_calls'])
        if e['ctl'] not in r['ctls']:
            r['ctls'].append(e['ctl'])
        r['srcs'].setdefault((e['src'], e['status']), []).append(e['line'])
    out = list(rows.values())
    for r in out:
        r['live'] = any(st in ('effective', 'loaded') for (_s, st) in r['srcs'])
        r['open'] = bool(r['diffs']) and r['live'] and not r.get('override') and r['sev'] != 'LOW'
    return out


OVERRIDE_LABEL = {'ht9045_testerif_c_wire.js': '已覆寫 (MR !93/!94)'}


def kb_lit(args):
    """golden call args -> the kb entry shape ['TYPE', dp, checkRange, min, max] (expressions kept)."""
    if not args:
        return '—（golden 不開小鍵盤）'
    fl = '|'.join(f.strip()[2:] if f.strip().startswith('N_') else f.strip() for f in args[1].split('|'))
    dp = args[2] if len(args) > 2 else '0'
    cr = args[3] if len(args) > 3 else 'false'
    mn = args[4] if len(args) > 4 else '0'
    mx = args[5] if len(args) > 5 else '0'
    return "['%s', %s, %s, %s, %s]" % (fl, dp, cr, mn, mx)


def regen_tables(entries):
    """Per wire file: the fields whose generated kb differs from golden's generic branch."""
    per = collections.OrderedDict()
    for e in sorted(entries, key=lambda x: (x['src'], x['line'])):
        if not e['src'].startswith('ht9045_wire_') or e.get('verdict') != 'ok' or not e.get('diffs'):
            continue
        f = per.setdefault(e['src'], dict(rows=[], pages=collections.OrderedDict()))
        f['pages'][(e['page'], e['status'])] = True
        if any(r['ctl'] == e['ctl'] for r in f['rows']):
            continue
        cpp = e['handler_key'][0]
        hits = e.get('gen_hits') or []
        fl = e.get('extractor_first')
        if hits:
            taken = ', '.join('`%s:%d`' % (cpp, x) for x in ([fl] if fl in hits else hits[:3]))
            if fl not in hits and len(hits) > 3:
                taken += ' 等'
        elif e.get('g2') and e['g2'].get('hits'):
            taken = '906 無；912 `%s:%d`' % (cpp, e['g2']['hits'][0])
        elif e.get('extractor_first_same'):
            taken = '`%s:%d`（註解掉的那行）' % (cpp, fl)
        else:
            taken = '906／912 都無對應'
        rep = e['generic'][0]
        gline = '`%s:%d`' % (cpp, rep['line']) if rep['line'] else '`%s:%d` 整段（不開小鍵盤）' % (cpp, e['handler_key'][1])
        if e.get('varies'):
            gline += '（未知條件全取 else）'
        imp = e['sev']
        if e['cause'] == 'branch':
            tags = ['分支']
        else:
            tags = ['正規化：' + {'neg': '負值範圍被關', 'var': 'golden 範圍是變數'}.get(e.get('norm_kind'), '其他')]
        if e['cause'] == 'branch' and e.get('gold_var'):
            tags.append('golden 範圍是變數，重產也帶不出')
        if e.get('varies'):
            tags.append('依設定／狀態')
        if not e['is_input']:
            tags.append('不是文字框')
        elif e['hidden'] and e['golden_hidden']:
            tags.append('隱藏')
        if e['status'] in ('shadowed', 'not-loaded'):
            tags.append('此檔未生效')
        ov = e.get('override')
        if ov:
            tags.append(OVERRIDE_LABEL.get(ov[0], '已覆寫（`%s`:%d）' % (ov[0], ov[1])))
        f['rows'].append(dict(ctl=e['ctl'], line=e['line'], gen=ent_text(e['generated']), taken=taken,
                              gline=gline, gval=kb_lit(rep['args']), sev=e['sev'], cause=e['cause'],
                              norm_kind=e.get('norm_kind'), gold_var=e.get('gold_var'),
                              imp='%s（%s）' % (imp, '；'.join(tags)), override=bool(ov),
                              live=e['status'] not in ('shadowed', 'not-loaded')))
    return per


def write_regen(L, per):
    L.append('## 要重產的檔 → 欄位')
    L.append('')
    L.append('每個接線檔一張表，只列「產生的 kb ≠ golden 一般分支」的欄位。影響欄括號：分支＝產生器抄到別的分支（第一分支問題）；'
             '正規化＝分支沒抄錯，差在 gen_wire.py 自己的規則（負值範圍／變數範圍被關掉）。')
    L.append('')
    for src, f in per.items():
        rows = sorted(f['rows'], key=lambda r: (0 if r['cause'] == 'branch' else 1, -SEV_RANK[r['sev']], r['line']))
        nb = sum(1 for r in rows if r['cause'] == 'branch')
        nneg = sum(1 for r in rows if r['cause'] != 'branch' and r['norm_kind'] == 'neg')
        nn = len(rows) - nb
        nh = sum(1 for r in rows if r['sev'] == 'HIGH')
        no = sum(1 for r in rows if r['override'])
        pages = '、'.join('%s（%s）' % (p, status_text(st)) for (p, st) in f['pages'])
        L.append('**`web/page/%s`**（%s）：差異 %d 欄＝分支 %d＋正規化 %d（負值範圍 %d、變數 %d）；HIGH %d%s' % (
            src, pages, len(rows), nb, nn, nneg, nn - nneg, nh, ('，其中已覆寫 %d' % no) if no else ''))
        L.append('')
        L.append('| 控制項 | 接線檔:行 | 產生的值 | 產生器抄到的 golden 分支 | golden 一般分支 | 一般分支的值 [型別, dp, checkRange, min, max] | 影響 |')
        L.append('|---|---|---|---|---|---|---|')
        for r in rows:
            L.append('| `%s` | `%s`:%d | %s | %s | %s | `%s` | %s |' % (
                r['ctl'], src, r['line'], r['gen'], r['taken'], r['gline'], md_escape(r['gval']), r['imp']))
        L.append('')


def md_escape(s):
    """For text inside a `code span` in a table cell: only the cell separator matters."""
    return str(s).replace('|', '¦')


def md_cell(s):
    """Plain table-cell text: no cell separator, no raw < > (C++ like a<b would become a tag); keep <br>."""
    t = str(s).replace('|', '¦').replace('<', '&lt;').replace('>', '&gt;')
    return t.replace('&lt;br&gt;', '<br>')


def row_md(r):
    srcs = '<br>'.join('`%s`:%s（%s）' % (s, ','.join(str(x) for x in sorted(set(lns))), status_text(st))
                       for (s, st), lns in r['srcs'].items())
    br = '<br>'.join('`%s` → `%s`' % (md_escape(x['cond']), md_escape(x['text'])) for x in r['reach'])
    if r['n_calls'] > len(r['reach']):
        br += '<br>（handler 共 %d 個呼叫，其餘屬別的控制項）' % r['n_calls']
    imp = '**%s**' % r['sev'] if r['sev'] == 'HIGH' else r['sev']
    tags = []
    if r['diffs']:
        if r['cause'] == 'norm':
            tags.append('同分支，差在產生器正規化')
        if r['varies']:
            tags.append('依設定／狀態，其他情形最高 %s' % r.get('sev_other'))
    if not r['live']:
        tags.append('此檔未生效')
    if r.get('override'):
        o = r['override']
        tags.append('已覆寫（`%s`:%d → %s）' % (o[0], o[1], ent_text(o[2])))
    if tags:
        imp += '（' + '；'.join(tags) + '）'
    return '| %s | %s | %s | `%s` | %s | `%s` %s | %s | %s | %s |' % (
        r['page'], srcs, '<br>'.join('`%s`' % c for c in r['ctls']), md_escape(r['handler']),
        br, ent_text(r['generated']), md_cell(r['gen_which']), md_cell(r['generic']),
        md_cell('；'.join(r['diffs']) or '相同'), imp)


def summarize(entries, rows, js_info, loaded_by):
    S = collections.OrderedDict()
    wire = [e for e in entries if e['src'].startswith('ht9045_wire_')]
    S['wire_files_with_kb'] = len({e['src'] for e in wire})
    S['wire_kb_entries'] = len({(e['src'], e['ctl']) for e in wire})
    S['copy_files'] = sorted({e['src'] for e in entries if not e['src'].startswith('ht9045_wire_')})
    S['copy_kb_entries'] = len({(e['src'], e['ctl']) for e in entries if not e['src'].startswith('ht9045_wire_')})
    hk = collections.defaultdict(set)
    for e in entries:
        if e.get('verdict') == 'ok':
            hk[e['handler_key']].add(e['conditional'])
    S['handlers'] = len(hk)
    S['handlers_branch'] = sum(1 for k, v in hk.items() if True in v)
    S['handlers_single'] = sum(1 for k, v in hk.items() if True not in v)
    S['rows'] = len(rows)
    S['rows_differ'] = sum(1 for r in rows if r['diffs'])
    S['rows_differ_branch'] = sum(1 for r in rows if r['diffs'] and r['cause'] == 'branch')
    S['rows_differ_norm'] = sum(1 for r in rows if r['diffs'] and r['cause'] == 'norm')
    S['rows_high'] = sum(1 for r in rows if r['sev'] == 'HIGH')
    S['rows_high_open'] = sum(1 for r in rows if r['sev'] == 'HIGH' and r['open'])
    S['rows_high_open_plain'] = sum(1 for r in rows if r['sev'] == 'HIGH' and r['open'] and not r['varies'])
    S['rows_high_overridden'] = sum(1 for r in rows if r['sev'] == 'HIGH' and r.get('override'))
    S['rows_high_not_live'] = sum(1 for r in rows if r['sev'] == 'HIGH' and not r['live'])
    S['rows_med_open'] = sum(1 for r in rows if r['sev'] == 'MED' and r['open'])
    S['entries_high_open'] = sum(len(r['ctls']) for r in rows if r['sev'] == 'HIGH' and r['open'])
    S['verdicts'] = dict(collections.Counter(e.get('verdict') for e in entries))
    return S


def per_page(entries, rows):
    P = collections.OrderedDict()
    for e in sorted(entries, key=lambda x: x['page']):
        p = P.setdefault(e['page'], dict(srcs=collections.OrderedDict(), n=0, single=set(), branch=set(),
                                         single_mismatch=[], nohandler=[]))
        p['srcs'][(e['src'], e['status'])] = p['srcs'].get((e['src'], e['status']), 0) + 1
        p['n'] += 1
        if e.get('verdict') != 'ok':
            p['nohandler'].append((e['src'], e['ctl'], e.get('verdict')))
            continue
        if e['conditional']:
            p['branch'].add(e['handler_key'])
        else:
            p['single'].add(e['handler_key'])
            if not e['gen_hits']:
                p['single_mismatch'].append((e['src'], e['ctl'], e['line'], gen_which(e)))
    for r in rows:
        p = P[r['page']]
        p.setdefault('rows', 0)
        p['rows'] += 1
        if r['diffs']:
            p['differ'] = p.get('differ', 0) + 1
        if r['sev'] == 'HIGH' and r['open']:
            p['high'] = p.get('high', 0) + 1
    return P


def write_md(out, S, rows, P, js_info, loaded_by, html_scripts, args, ownership, regen):
    L = []
    L.append('<!-- generated by tools/webprobe/qwerty_first_branch_audit.py; golden=%s -->' % args.golden)
    L.append('')
    L.append('## 附錄 A　腳本總數（與 stdout 同一份）')
    L.append('')
    for k, v in S.items():
        L.append('- `%s`: %s' % (k, md_cell(v)))
    L.append('')
    L.append('## 附錄 B　分支表（handler 有多個呼叫，或唯一呼叫在條件裡）')
    L.append('')
    L.append('同一個 handler、同樣分支、同樣結果的控制項併成一列；兩份接線檔都有的欄位，「接線檔:行」列出兩份與各自狀態。'
             '分支欄已依控制項把 Sender／Ptr==／->Name／->Tag 的分派解掉（只留這個控制項走得到的呼叫）。')
    L.append('')
    L.append('| 頁面 | 接線檔:行 | 控制項 | golden handler | 分支（條件 → 呼叫） | 產生的 ＝哪一支 | golden 一般分支 | 差異 | 影響 |')
    L.append('|---|---|---|---|---|---|---|---|---|')
    order = {'HIGH': 0, 'MED': 1, 'LOW': 2}
    for r in sorted(rows, key=lambda r: (order[r['sev']], 0 if r['open'] else 1, r['page'], r['handler'])):
        L.append(row_md(r))
    L.append('')
    L.append('## 附錄 C　逐頁統計')
    L.append('')
    L.append('「單一無條件 handler」＝只有一個 ShowQwertyKey 且不在任何條件裡的 handler 數（不需列表）；'
             '「HIGH」只算生效檔、沒被覆寫、頁面上點得到的列。')
    L.append('')
    L.append('| 頁面 | kb 來源（狀態：筆數） | kb 筆數 | 單一無條件 handler | 有分支 handler | 列數 | 有差異 | HIGH |')
    L.append('|---|---|---|---|---|---|---|---|')
    for pg, p in P.items():
        srcs = '<br>'.join('`%s`（%s：%d）' % (s, status_text(st), n) for (s, st), n in p['srcs'].items())
        L.append('| %s | %s | %d | %d | %d | %d | %d | %d |' % (pg, srcs, p['n'], len(p['single']), len(p['branch']),
                                                          p.get('rows', 0), p.get('differ', 0), p.get('high', 0)))
    L.append('')
    L.append('## 附錄 D　單一無條件呼叫、但產生的值找不到對應呼叫的 kb 筆')
    L.append('')
    n = 0
    for pg, p in P.items():
        for s, c, ln, why in p['single_mismatch']:
            L.append('- %s `%s`:%d `%s`：%s' % (pg, s, ln, c, md_cell(why)))
            n += 1
    if not n:
        L.append('（無）')
    L.append('')
    L.append('## 附錄 E　找不到 golden handler 的 kb 筆')
    L.append('')
    n = 0
    for pg, p in P.items():
        if p['nohandler']:
            L.append('- %s：%s' % (pg, '、'.join('`%s`/%s(%s)' % x for x in p['nohandler'])))
            n += 1
    if not n:
        L.append('（無）')
    L.append('')
    if ownership:
        L.append('## 附錄 F　擁有者（git log 最近 5 筆，腳本判斷）')
        L.append('')
        L.append('判斷規則：subject 有 St02／St01 標記就照標；作者 jimmychiu＝筆電；HT9045 Machine (V906)／EastSun-machine＝機台端；'
                 '其餘「Steven(未標)」＝Steven 兩台（St01／St02）沒在 subject 標明的 commit。')
        L.append('')
        L.append('| 檔案 | 產生方式 | 最近 commit | 判斷 |')
        L.append('|---|---|---|---|')
        for f, (gen, commits) in ownership.items():
            L.append('| `%s` | %s | %s | %s |' % (f, gen, '<br>'.join('%s %s %s' % (c[0], md_cell(c[1]), md_cell(c[3][:70]))
                                                                    for c in commits), '、'.join(owner_of(commits))))
        L.append('')
    write_regen(L, regen)
    with io.open(out, 'w', encoding='utf-8', newline='\r\n') as fh:
        fh.write('\n'.join(L) + '\n')


def main(argv=None):
    global REPO
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--golden', default=DEF_GOLDEN)
    ap.add_argument('--golden2', default=DEF_GOLDEN2, help='912 tree, only to explain entries 906 cannot match')
    ap.add_argument('--repo', default=REPO, help='repo root (for git log)')
    ap.add_argument('--page', default=None, help='default: <repo>/web/page')
    ap.add_argument('--md')
    ap.add_argument('--json')
    ap.add_argument('--tsv')
    ap.add_argument('--no-git', action='store_true')
    ap.add_argument('--quiet', action='store_true')
    args = ap.parse_args(argv)
    REPO = os.path.normpath(args.repo)
    if not args.page:
        args.page = os.path.join(REPO, 'web', 'page')
    if hasattr(sys.stdout, 'buffer'):
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

    G, js_info, loaded_by, html_scripts, entries = audit(args)
    rows = build_rows(entries)
    S = summarize(entries, rows, js_info, loaded_by)
    P = per_page(entries, rows)
    regen = regen_tables(entries)
    S['regen_files'] = len(regen)
    S['regen_fields'] = sum(len(f['rows']) for f in regen.values())
    S['regen_fields_branch'] = sum(1 for f in regen.values() for r in f['rows'] if r['cause'] == 'branch')
    S['regen_fields_high'] = sum(1 for f in regen.values() for r in f['rows'] if r['sev'] == 'HIGH')
    S['regen_fields_norm_neg'] = sum(1 for f in regen.values() for r in f['rows']
                                     if r['cause'] != 'branch' and r['norm_kind'] == 'neg')
    S['regen_fields_norm_var'] = sum(1 for f in regen.values() for r in f['rows']
                                     if r['cause'] != 'branch' and r['norm_kind'] != 'neg')
    S['regen_fields_live'] = sum(1 for f in regen.values() for r in f['rows'] if r['live'])
    S['regen_fields_branch_live'] = sum(1 for f in regen.values() for r in f['rows'] if r['live'] and r['cause'] == 'branch')

    ownership = collections.OrderedDict()
    if not args.no_git:
        files = set()
        for e in entries:
            files.add(e['src'])
            if e['page'] and e['page'].endswith('.html'):
                files.add(e['page'])
        for f in sorted(files):
            gen = js_info[f]['gen'] if f in js_info else 'page'
            ownership[f] = (gen, git_log(os.path.join(args.page, f)))

    print('== qwerty first-branch audit (golden %s)' % args.golden)
    for k, v in S.items():
        print('  %-22s %s' % (k, v))
    if not args.quiet:
        print('')
        print('== rows (sev | page | controls | handler | generated -> which | generic | diffs)')
        for r in sorted(rows, key=lambda r: (-SEV_RANK[r['sev']], r['page'])):
            if not r['diffs']:
                continue
            print('%s | %s | %s | %s | %s %s | %s | %s%s' % (
                r['sev'], r['page'], ','.join(r['ctls']), r['handler'], ent_text(r['generated']), r['gen_which'],
                r['generic'].replace('<br>', ' / '), '; '.join(r['diffs']),
                ' [override]' if r.get('override') else ''))
    if args.md:
        write_md(args.md, S, rows, P, js_info, loaded_by, html_scripts, args, ownership, regen)
        print('md  -> %s' % args.md)
    if args.json:
        def conv(o):
            if isinstance(o, (set, tuple)):
                return list(o)
            return str(o)
        jrows = []
        for r in rows:
            r2 = dict(r)
            r2['srcs'] = [[s, st, lns] for (s, st), lns in r['srcs'].items()]
            jrows.append(r2)
        with io.open(args.json, 'w', encoding='utf-8', newline='\n') as fh:
            json.dump(dict(summary=S, rows=jrows, entries=entries), fh, ensure_ascii=False, indent=1, default=conv)
        print('json -> %s' % args.json)
    if args.tsv:
        with io.open(args.tsv, 'w', encoding='utf-8', newline='\n') as fh:
            for e in entries:
                fh.write(chr(9).join(str(x) for x in (
                    e['page'], e['src'], e['line'], e['ctl'], e.get('verdict'), e.get('handler'),
                    e.get('n_calls'), e.get('conditional'), ent_text(e['generated']), gen_which(e) if e.get('verdict') == 'ok' else '',
                    generic_text(e) if e.get('verdict') == 'ok' else '', e.get('sev'), '; '.join(e.get('diffs') or []))) + '\n')
        print('tsv -> %s' % args.tsv)
    return 0


if __name__ == '__main__':
    sys.exit(main())
