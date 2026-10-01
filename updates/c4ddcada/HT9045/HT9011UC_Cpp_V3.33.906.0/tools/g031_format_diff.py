# -*- coding: utf-8 -*-
# =============================================================================
#  tools/g031_format_diff.py  --  G-031 step 1 (TO_IFOR I-02): old -> new format difference list
#
#  AI(W906-I02) 20261001 (Ifor01): new file. Read-only analysis tool; nothing in the build uses it.
#  The curated list built from its output is docs/G031_FORMAT_DIFF.md; its raw output is docs/G031_FORMAT_DIFF_DETAIL.md.
#
#  What the BCB6 production program (old: V912 / V899) and the C++ port (new: this tree) read and write in the machine's
#  recipe and settings files, so that a converter for upgrading old machines can be built (the converter itself is
#  St02's, TO_IFOR §4 12:0x).  Three independent measurements, each rerunnable:
#
#    1. STATIC  -- every INI access in each tree's sources, as (file, section, key, default, read/write, file:line):
#                  the path-taking helpers (ReadIniData / CheckAndReadIniData / WriteIniData / ReadWriteIni / ...),
#                  the Gerneral.ini helpers (*General), FormSysTools sessions, and HTEditList ->Add registrations.
#                  The file is attributed from the path argument (literal, global path variable, or the nearest
#                  assignment inside the function); what cannot be attributed is listed, never guessed.
#    2. BINARY  -- struct layouts written whole to system\*.dat (tech.dat, lastdata.dat, machinerecord.dat, ...), via
#                  tools/nb2_assist/struct_layout_across_trees.py.
#    3. DATA    -- presence counts of every (file, section, key) over real recipe folders, read with Win32 INI rules
#                  (case-insensitive, first duplicate section wins, keys and values trimmed).
#
#  Read-only: it reads the trees and the data folders and writes only the --md / --json files you name.
#  Comments and `#if 0` blocks are stripped first (the port's FileRW/*.gen.inc keep gated golden text in `#if 0`).
#  Sections and keys compare case-insensitively (Win32 INI semantics).
#
#  usage:
#    python tools/g031_format_diff.py [--old NAME=PATH ...] [--new PATH] [--recipes DIR ...]
#                                     [--md OUT.md] [--json OUT.json] [--no-binary]
#  defaults: --old V912=... and V899=... (the two production trees under D:/HT9045, when present)  --new <this tree>
#            --recipes every D:/HT9045/IniData/Data* folder that exists
#  Accuracy notes (read before trusting a single row):
#    * attribution is heuristic; whatever cannot be attributed lands under "?" with its call site (V912 ~1.7 %, port ~1.6 %);
#    * a key registered in several places (often under different customer / feature conditions) shows every default;
#      "default sets differ (subset)" usually means one extra registration, not a changed default;
#    * a computed key that cannot be turned into a pattern (`<expr>`) is treated as matching any key in the data check,
#      which makes "unknown to the port" an under-count, never an over-count.
# =============================================================================
import argparse
import collections
import json
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.abspath(os.path.join(HERE, '..'))
DEFAULT_OLDS = ['V912=D:/HT9045/HT9011UC_Code_V3.33.912.0_20260908_Jimmy',
                'V899=D:/HT9045/HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422']
DEFAULT_RECIPE_PARENT = 'D:/HT9045/IniData'

SKIP_DIRS = {'.svn', '.git', 'tests', 'tools', 'vclcompat', 'backup', 'Backup', '__history', 'Obj', 'Obj912', 'Out912',
             'node_modules', 'web', 'docs', 'machines'}
SRC_EXT = ('.cpp', '.h', '.hpp', '.inc')

# ---------------------------------------------------------------------------------------------------------------------
#  source loading + stripping
# ---------------------------------------------------------------------------------------------------------------------


def read_source(path):
    b = open(path, 'rb').read()
    try:
        return b.decode('utf-8')
    except UnicodeDecodeError:
        return b.decode('cp950', 'replace')


def strip_comments(t):
    """Comments -> spaces (newlines kept), strings and char literals untouched."""
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c == '/' and i + 1 < n and t[i + 1] == '/':
            j = t.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i))
            i = j
        elif c == '/' and i + 1 < n and t[i + 1] == '*':
            j = t.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(''.join('\n' if ch == '\n' else ' ' for ch in t[i:j]))
            i = j
        elif c in '"\'':
            j = i + 1
            while j < n and t[j] != c and t[j] != '\n':
                j += 2 if t[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(t[i:j])
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


_PP = re.compile(r'^[ \t]*#[ \t]*(if|ifdef|ifndef|elif|else|endif)\b(.*)$')


def strip_if0(t):
    """Blank `#if 0` regions (and the #else of `#if 1`); other conditionals are kept (both arms count)."""
    lines = t.split('\n')
    stack = []          # per level: [this_branch_dead, some_branch_taken_for_constant_ifs, constant]
    out = []
    for ln in lines:
        m = _PP.match(ln)
        dead_outer = any(s[0] for s in stack)
        if m:
            kw, rest = m.group(1), m.group(2).strip()
            if kw in ('if', 'ifdef', 'ifndef'):
                if kw == 'if' and re.match(r'^0\b', rest):
                    stack.append([True, False, True])
                elif kw == 'if' and re.match(r'^1\b', rest):
                    stack.append([False, True, True])
                else:
                    stack.append([False, False, False])
            elif kw == 'elif' and stack:
                s = stack[-1]
                if s[2]:
                    s[0] = s[1]                 # constant #if: elif live only if nothing taken yet (treat as live then)
                    s[1] = True
            elif kw == 'else' and stack:
                s = stack[-1]
                if s[2]:
                    s[0] = s[1]
                    s[1] = True
            elif kw == 'endif' and stack:
                stack.pop()
            out.append('')
            continue
        out.append('' if (dead_outer or (stack and stack[-1][0])) else ln)
    return '\n'.join(out)


def blank_strings(t):
    """Same length; string contents -> spaces (for brace / paren structure)."""
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c in '"\'':
            j = i + 1
            while j < n and t[j] != c and t[j] != '\n':
                j += 2 if t[j] == '\\' else 1
            j = min(j + 1, n)
            out.append(c + ' ' * max(0, j - i - 2) + (t[j - 1] if j - 1 > i else ''))
            i = j
        else:
            out.append(c)
            i += 1
    return ''.join(out)


class Source:
    def __init__(self, tree, rel, text):
        self.tree, self.rel = tree, rel
        self.code = strip_if0(strip_comments(text))
        self.blank = blank_strings(self.code)
        self._nl = [i for i, ch in enumerate(self.code) if ch == '\n']
        self._func = None

    def line(self, pos):
        lo, hi = 0, len(self._nl)
        while lo < hi:
            mid = (lo + hi) // 2
            if self._nl[mid] < pos:
                lo = mid + 1
            else:
                hi = mid
        return lo + 1

    def func_span(self, pos):
        """(start, end) of the top-level brace block around pos (a function body), or the whole file."""
        if self._func is None:
            spans, depth, start = [], 0, None
            for i, ch in enumerate(self.blank):
                if ch == '{':
                    if depth == 0:
                        start = i
                    depth += 1
                elif ch == '}' and depth > 0:
                    depth -= 1
                    if depth == 0 and start is not None:
                        spans.append((start, i))
            self._func = spans
        for a, b in self._func:
            if a <= pos <= b:
                # extend back to the signature line
                sig = self.code.rfind('\n', 0, max(0, self.code.rfind(')', 0, a)))
                return max(0, sig), b
        return 0, len(self.code)


def load_tree(name, root):
    out = []
    for dp, dn, fn in os.walk(root):
        dn[:] = [d for d in dn if d not in SKIP_DIRS and not d.lower().startswith('build')]
        for f in fn:
            if f.lower().endswith(SRC_EXT):
                p = os.path.join(dp, f)
                out.append(Source(name, os.path.relpath(p, root).replace('\\', '/'), read_source(p)))
    return out

# ---------------------------------------------------------------------------------------------------------------------
#  call parsing
# ---------------------------------------------------------------------------------------------------------------------


def split_args(code, blank, open_paren):
    """Arguments of the call whose '(' is at open_paren -> ([arg text...], close_index)."""
    depth, args, cur_start = 0, [], open_paren + 1
    i = open_paren
    while i < len(blank):
        ch = blank[i]
        if ch in '([{':
            depth += 1
        elif ch in ')]}':
            depth -= 1
            if depth == 0:
                args.append(code[cur_start:i].strip())
                return args, i
        elif ch == ',' and depth == 1:
            args.append(code[cur_start:i].strip())
            cur_start = i + 1
        i += 1
    return args, len(blank)


_STR = re.compile(r'"((?:[^"\\]|\\.)*)"')


def literal(arg):
    """Value of an argument that is only string literal(s) (optionally wrapped in AnsiString(...)); else None."""
    a = arg.strip()
    m = re.fullmatch(r'(?:AnsiString|String)\s*\((.*)\)', a, re.S)
    if m:
        a = m.group(1).strip()
    if not a.startswith('"'):
        return None
    parts = _STR.findall(a)
    rest = _STR.sub('', a).strip()
    if rest:
        return None
    return ''.join(p.replace('\\\\', '\\').replace('\\"', '"') for p in parts)


def _num(s):
    try:
        f = float(s.rstrip('fF'))
    except ValueError:
        return None
    return str(int(f)) if f == int(f) else repr(f)


def norm_default(arg):
    """Effective default as the file would see it: "0" / 0 / false -> 0, "1" / 1 / true -> 1, "0.0" -> 0."""
    if arg is None:
        return None
    m = re.fullmatch(r'\s*(?:AnsiString|String|IntToStr|FloatToStr)\s*\((.*)\)\s*', arg, re.S)
    if m and literal(arg) is None:
        return norm_default(m.group(1))
    v = literal(arg)
    if v is not None:
        n = _num(v.strip()) if re.fullmatch(r'\s*[-+]?\d+\.?\d*\s*', v) else None
        return n if n is not None else '"%s"' % v
    a = re.sub(r'\s+', '', arg)
    if a in ('false', 'FALSE'):
        return '0'
    if a in ('true', 'TRUE'):
        return '1'
    if re.fullmatch(r'[-+]?\d+\.?\d*[fF]?', a):
        n = _num(a)
        return n if n is not None else a
    return a


def key_pattern(arg, src, pos):
    """Computed section / key -> a regex-ish pattern for display and data matching, or None."""
    a = arg.strip()
    m = re.match(r'^(?:AnsiString\s*\(\s*)?"((?:[^"\\]|\\.)*)"\s*\)?\s*\+', a)
    if m:
        return re.escape(m.group(1)) + '.*'
    m = re.fullmatch(r'[A-Za-z_]\w*', a)
    if m:
        f0, _ = src.func_span(pos)
        body = src.code[f0:pos]
        sp = list(re.finditer(r'\b%s\s*\.\s*(?:s?printf|cat_sprintf)\s*\(\s*"((?:[^"\\]|\\.)*)"' % re.escape(a), body))
        if sp:
            fmt = sp[-1].group(1)
            parts = re.split(r'(%[-+ 0#]*\d*(?:\.\d+)?[lh]?[duisfx])', fmt)
            conv = {'d': r'\d+', 'u': r'\d+', 'i': r'\d+', 'x': r'[0-9a-fA-F]+', 's': r'.*', 'f': r'[-0-9.]+'}
            rx = ''.join(conv[p[-1]] if (p.startswith('%') and len(p) > 1 and p[-1] in conv) else re.escape(p) for p in parts)
            lit = ''.join(p for p in parts if not p.startswith('%'))
            if len(lit.strip()) >= 3:
                return rx
    return None

# ---------------------------------------------------------------------------------------------------------------------
#  file attribution
# ---------------------------------------------------------------------------------------------------------------------


_FILE_LIT = re.compile(r'(?<![%\w\-])([A-Za-z0-9_\-]+\.(?:Data|DATA|data|ini|INI|Ini|dat|DAT|txt|TXT|csv|CSV|inf|INF|def|DEF))(?![\w])')
GLOBAL_PATHS = {
    'asGeneralPath': 'Gerneral.ini', 'asTeachPath': 'teach.ini', 'asLastSetIniPath': 'LastSet.ini',
}
LIST_FILE = {
    'elConfig': 'config.ini', 'cbLastSet': 'LastSet.ini', 'elConfig_byRecipe': 'configByRecipe.ini',
    'elTrayForm': 'Tray.Data', 'elUdUld': 'UdUld.Data', 'elParameter': 'AOI.Data', 'elLaser': 'HandlerCondition.Data',
    'elVacuumUnit': 'HandlerCondition.Data', 'elData': 'AutoCalSuckZ.Data', 'elTeach': 'teach.ini',
}


PREFERRED = {}          # lower -> first spelling seen (Windows file names are case-insensitive)
BINASGN = 'Binasgn*.Data (7 files)'


def canon(fname):
    f = fname.strip().lstrip('\\/')
    low = f.lower()
    if low in ('gerneral.ini', 'general.ini'):
        return 'Gerneral.ini'
    if low.startswith('binasgn') and low.endswith('.data'):
        return BINASGN          # BinasgnOff / Binasgn / -Line / _ART / _MRT / _MRT_RT: one loop over SavePath[] reads them all
    return PREFERRED.setdefault(low, f)


def file_from_expr(expr):
    if re.search(r'\bk?SavePath\s*\[', expr):
        return BINASGN
    lits = _FILE_LIT.findall(' '.join(_STR.findall(expr)) if '"' in expr else '')
    if lits:
        return canon(lits[-1])
    for g, f in GLOBAL_PATHS.items():
        if re.search(r'\b%s\b' % g, expr):
            return f
    return None


GPIB_INI = 'GPIB general.ini (D:/GPIB9045/system, separate program)'


_COMPUTED = re.compile(r'%[-+ 0#]*\d*(?:\.\d+)?[lh]?s\.(Data|ini|txt|dat|csv|def)\b', re.I)
TREE_INDEX = {}         # tree name -> {identifier: set(files)} from initialisers / assignments anywhere in the tree
TREE_SOURCES = {}       # tree name -> [Source]
# names too common to look up tree-wide (a different variable of the same name elsewhere says nothing about this one)
COMMON_NAMES = {'szDir', 'szDir2', 'szDir3', 'FileName', 'filename', 'fileName', 'sPath', 'asPath', 'aPath', 'Path', 'path',
                'sFile', 'asFile', 'aFile', 'file', 'fname', 'sFileName', 'asFileName', 'aFileName', 'IniFileName', 'sIniFile',
                'asIniFile', 'strFile', 'strPath', 'setupIni', 'testerData', 'S', 's', 'str', 'Str', 'tmp', 'sTmp'}
_CALLER_CACHE = {}


def func_info(src, pos):
    """(qualified name, short name, [param names], definition start) of the function around pos, or None."""
    s, e = src.func_span(pos)
    if (s, e) == (0, len(src.code)):
        return None
    brace = src.blank.find('{', s)
    close = src.blank.rfind(')', 0, brace)
    if close < 0 or src.blank[close + 1:brace].strip() not in ('', 'const'):
        return None
    depth, i = 0, close
    while i >= 0:
        ch = src.blank[i]
        if ch == ')':
            depth += 1
        elif ch == '(':
            depth -= 1
            if depth == 0:
                break
        i -= 1
    if i < 0:
        return None
    m = re.search(r'([A-Za-z_][\w:~]*)\s*$', src.code[max(0, i - 120):i])
    if not m:
        return None
    qual = m.group(1)
    params, _ = split_args(src.code, src.blank, i)
    names = []
    for prm in params:
        prm = prm.split('=')[0].strip()
        mm = re.search(r'([A-Za-z_]\w*)\s*(?:\[\s*\w*\s*\])?$', prm)
        names.append(mm.group(1) if mm else '')
    return qual, qual.split('::')[-1], names, i


def via_callers(src, pos, var, depth):
    fi = func_info(src, pos)
    if not fi or var not in fi[2]:
        return None
    qual, short, names, defpos = fi
    idx = names.index(var)
    key = (src.tree, src.rel, qual, idx)
    if key in _CALLER_CACHE:
        return _CALLER_CACHE[key]
    _CALLER_CACHE[key] = None            # recursion guard
    files = set()
    call_rx = re.compile(r'(?<![\w~])%s\s*\(' % re.escape(short))
    scopes = [[src]] + [[s for s in TREE_SOURCES.get(src.tree, []) if s is not src]]
    for scope in scopes:
        for s2 in scope:
            if short not in s2.code:
                continue
            for m in call_rx.finditer(s2.blank):
                if s2 is src and abs(m.start() - (defpos - len(short))) < 200:
                    continue                  # the definition itself
                head = s2.code[max(0, m.start() - 40):m.start()]
                if re.search(r'(void|int|bool|AnsiString|double|__fastcall)\s*(\w+::)?\s*$', head):
                    continue                  # another definition / declaration
                args, _ = split_args(s2.code, s2.blank, m.end() - 1)
                if len(args) <= idx:
                    continue
                f, _how = attribute_path(args[idx], s2, m.start(), depth + 1)
                if f != '?':
                    files.add(f)
        if files:
            break
    res = None
    if len(files) == 1:
        res = (next(iter(files)), 'via callers of %s' % qual)
    elif 1 < len(files) <= 3:
        res = (' / '.join(sorted(files)), 'one of several files (callers of %s)' % qual)
    _CALLER_CACHE[key] = res
    return res


def build_index(tree, sources):
    idx = collections.defaultdict(set)
    rx = re.compile(r'\b([A-Za-z_]\w*)\s*(?:\+?=(?!=)|\(|\.\s*(?:s?printf|cat_sprintf)\s*\()\s*([^;]*?)\)?\s*;')
    for s in sources:
        for m in rx.finditer(s.code):
            name, e = m.group(1), m.group(2)
            if name in ('if', 'while', 'for', 'switch', 'return', 'sizeof'):
                continue
            f = file_from_expr(e)
            if f:
                idx[name].add(f)
    TREE_INDEX[tree] = idx
    TREE_SOURCES[tree] = sources
    for k in [k for k in _CALLER_CACHE if k[0] == tree]:
        del _CALLER_CACHE[k]          # the same tree name is reused for each --old tree


def computed_name(expr):
    m = _COMPUTED.search(' '.join(_STR.findall(expr)))
    return '(name built at run time).%s' % m.group(1) if m else None


def attribute_path(expr, src, pos, depth=0):
    """File kind for a path argument, or ('?', reason)."""
    if src.tree == 'new' and src.rel.startswith('TesterComm/') and re.search(r'\b(asGeneralPath|gpibGeneralIni)\b', expr):
        return GPIB_INI, 'TesterComm (gpibbridge::asGeneralPath, GpibBridge.h:230)'
    f = file_from_expr(expr)
    if f:
        return f, 'literal'
    ids = re.findall(r'\b([A-Za-z_]\w*)\b', expr)
    var = ids[-1] if ids else None
    m = re.fullmatch(r'\s*([A-Za-z_][\w\.\->]*?)\s*(?:\.c_str\(\))?\s*', expr)
    if m:
        var = re.split(r'->|\.', m.group(1))[-1]
    if not var:
        return '?', 'no identifier'
    f0, _ = src.func_span(pos)
    body = src.code[f0:pos]
    pat = re.compile(r'\b%s\b\s*(\+?=)(?!=)\s*([^;]+);|\b%s\s*\.\s*(?:s?printf|cat_sprintf)\s*\(([^;]+)\)\s*;|'
                     r'\bAnsiString\s+%s\s*\(([^;]+)\)\s*;' % ((re.escape(var),) * 3))
    hits = list(pat.finditer(body))
    for h in reversed(hits):
        e = h.group(2) or h.group(3) or h.group(4) or ''
        f = file_from_expr(e)
        if f:
            return f, 'assigned in function'
        c = computed_name(e)
        if c:
            return c, 'file name built at run time'
        if h.group(1) == '=' or h.group(3) or h.group(4):
            # an assignment without a file name stops the walk: follow its last identifier once more (S, sATCIniPath, ...)
            if depth < 2:
                ids2 = [x for x in re.findall(r'\b([A-Za-z_]\w*)\b', e)
                        if x not in ('AnsiString', 'String', 'c_str', 'GetRecipePath', 'GetRecipeFileName', 'AuthPath', 'DataPath')]
                if ids2:
                    f2, how2 = attribute_path(ids2[-1], src, h.start() + f0, depth + 1)
                    if f2 != '?':
                        return f2, 'via %s' % ids2[-1]
            break
    if depth < 3:
        vc = via_callers(src, pos, var, depth)
        if vc:
            return vc
    # file-wide: exactly one file name ever assigned to this identifier in this source file
    fw = set()
    for h in re.finditer(r'\b%s\b\s*\+?=(?!=)\s*([^;]+);|\b%s\s*\.\s*(?:s?printf|cat_sprintf)\s*\(([^;]+)\)\s*;'
                         % (re.escape(var), re.escape(var)), src.code):
        f = file_from_expr(h.group(1) or h.group(2) or '')
        if f:
            fw.add(f)
    if len(fw) == 1:
        return fw.pop(), 'assigned elsewhere in file'
    tw = TREE_INDEX.get(src.tree, {}).get(var, set()) if (len(var) >= 6 and var not in COMMON_NAMES) else set()
    if len(tw) == 1:
        return next(iter(tw)), 'assigned elsewhere in the tree'
    if len(tw) > 1 and len(tw) <= 3:
        return ' / '.join(sorted(tw)), 'one of several files (variable reused)'
    return '?', 'path %s' % expr.strip()[:80]

# ---------------------------------------------------------------------------------------------------------------------
#  extraction
# ---------------------------------------------------------------------------------------------------------------------


PATH_CALLS = {  # name: (mode, default arg index or None)
    'ReadIniData': ('r', 3), 'ReadIniDataMem': ('r', 3), 'CheckAndReadIniData': ('r+', 3),
    'WriteIniData': ('w', None), 'WriteIniData1': ('w', None), 'WriteIniDataNoLog': ('w', None),
    'ReadWriteIni': ('rw', 4), 'CheckIniData': ('x', None), 'CheckKeyExist': ('x', None),
}
GEN_CALLS = {'CheckAndReadIniDataGeneral': ('r+', 2), 'WriteIniDataGeneral': ('w', None), 'ReadIniDataGeneral': ('r', 2)}
_CALL = re.compile(r'(?<![\w>.:])(?:(FormSysTools)\s*->\s*)?(' + '|'.join(sorted(PATH_CALLS, key=len, reverse=True)) +
                   r')\s*\(')
_GEN = re.compile(r'(?<![\w>.:])(' + '|'.join(sorted(GEN_CALLS, key=len, reverse=True)) + r')\s*\(')
_ADD = re.compile(r'\b(\w+)\s*->\s*Add\s*\(')
_OPEN = re.compile(r'FormSysTools\s*->\s*(?:OpenFormData|OpenIniFile)\s*\(')


_DECL = re.compile(r'^(?:const\s+)?(?:AnsiString|String|int|bool|double|float|TDateTime|unsigned\s+long|unsigned|char\s*\*|'
                   r'std::string)\s*&?\s*\w+(?:\s*=.*)?$')
_HELPERS = set(PATH_CALLS) | set(GEN_CALLS) | {'ReadWriteIniGeneral', 'CheckSectionExist'}


def in_helper(src, pos):
    """True inside the body of one of the INI helpers themselves (common.cpp etc.)."""
    s, _ = src.func_span(pos)
    head = src.code[s:s + 400]
    m = re.search(r'\b(\w+)\s*\(', head)
    return bool(m and m.group(1) in _HELPERS)


class Rec:
    __slots__ = ('tree', 'file', 'section', 'key', 'sec_pat', 'key_pat', 'mode', 'default', 'loc', 'layer', 'how')

    def __init__(self, **kw):
        for k in self.__slots__:
            setattr(self, k, kw.get(k))


def layer_of(src):
    if src.tree == 'new':
        if src.rel.startswith('FileRW/') and src.rel.endswith('.gen.inc'):
            return 'page'
        return 'engine'
    return 'old'


def extract(src):
    recs = []
    code, blank = src.code, src.blank
    if '/TesterComm/' in '/' + src.rel and src.tree == 'new':
        gen_file = GPIB_INI
    else:
        gen_file = 'Gerneral.ini'
    # path-taking helpers (and FormSysTools sessions)
    for m in _CALL.finditer(blank):
        name = m.group(2)
        args, _ = split_args(code, blank, m.end() - 1)
        mode, di = PATH_CALLS[name]
        if m.group(1):                      # FormSysTools->X(section, key, value): path from the open session
            if len(args) < 2:
                continue
            opens = [o for o in _OPEN.finditer(blank, src.func_span(m.start())[0], m.start())]
            if opens:
                oargs, _ = split_args(code, blank, opens[-1].end() - 1)
                f, how = attribute_path(oargs[0] if oargs else '', src, opens[-1].start())
            elif re.search(r'(^|/)(cBinSel|cShowBinSelect|BinSelect)', src.rel):
                f, how = BINASGN, 'FormSysTools session opened by a caller (BinSel)'
            else:
                f, how = '?', 'FormSysTools without OpenFormData in function'
            sec_a, key_a = args[0], args[1]
            d = args[di - 1] if (di is not None and len(args) > di - 1) else None
        else:
            if len(args) < 3:
                continue
            if _DECL.match(args[0]) or in_helper(src, m.start()):
                continue
            f, how = attribute_path(args[0], src, m.start())
            sec_a, key_a = args[1], args[2]
            d = args[di] if (di is not None and len(args) > di) else None
            if name == 'ReadWriteIni' and len(args) > 5:
                rd = args[5].strip()
                mode = 'r+' if rd == 'true' else ('w' if rd == 'false' else 'rw')
        recs.append(Rec(tree=src.tree, file=f, section=literal(sec_a), key=literal(key_a),
                        sec_pat=key_pattern(sec_a, src, m.start()) if literal(sec_a) is None else None,
                        key_pat=key_pattern(key_a, src, m.start()) if literal(key_a) is None else None,
                        mode=mode, default=norm_default(d), loc='%s:%d' % (src.rel, src.line(m.start())),
                        layer=layer_of(src), how=how if f != '?' else how))
        if literal(sec_a) is None:
            recs[-1].section = None
        if literal(key_a) is None:
            recs[-1].key = None
        recs[-1].how = how
        if recs[-1].section is None:
            recs[-1].sec_pat = recs[-1].sec_pat or ('<%s>' % sec_a.strip()[:60])
        if recs[-1].key is None:
            recs[-1].key_pat = recs[-1].key_pat or ('<%s>' % key_a.strip()[:60])
    # Gerneral.ini helpers
    for m in _GEN.finditer(blank):
        name = m.group(1)
        args, _ = split_args(code, blank, m.end() - 1)
        if len(args) < 2:
            continue
        mode, di = GEN_CALLS[name]
        r = Rec(tree=src.tree, file=gen_file, section=literal(args[0]), key=literal(args[1]),
                mode=mode, default=norm_default(args[di]) if (di is not None and len(args) > di) else None,
                loc='%s:%d' % (src.rel, src.line(m.start())), layer=layer_of(src), how='*General')
        if r.section is None:
            r.sec_pat = key_pattern(args[0], src, m.start()) or '<%s>' % args[0].strip()[:60]
        if r.key is None:
            r.key_pat = key_pattern(args[1], src, m.start()) or '<%s>' % args[1].strip()[:60]
        recs.append(r)
    # HTEditList registrations
    for m in _ADD.finditer(blank):
        lst = m.group(1)
        if lst not in LIST_FILE and not re.match(r'^(el|cb)[A-Z]\w*$', lst):
            continue
        args, _ = split_args(code, blank, m.end() - 1)
        if len(args) < 5:
            continue
        f = LIST_FILE.get(lst, '?')
        r = Rec(tree=src.tree, file=f, section=literal(args[3]), key=literal(args[4]), mode='list',
                default=norm_default(args[8]) if len(args) > 8 else None,
                loc='%s:%d' % (src.rel, src.line(m.start())), layer=layer_of(src),
                how='list %s' % lst if f != '?' else 'list %s (file unknown)' % lst)
        if r.section is None:
            r.sec_pat = key_pattern(args[3], src, m.start()) or '<%s>' % args[3].strip()[:60]
        if r.key is None:
            r.key_pat = key_pattern(args[4], src, m.start()) or '<%s>' % args[4].strip()[:60]
        recs.append(r)
    return recs

# ---------------------------------------------------------------------------------------------------------------------
#  data (real recipe folders)
# ---------------------------------------------------------------------------------------------------------------------


def parse_ini(path):
    """Win32 GetPrivateProfileString view: {section_lower: {key_lower: value}}; first duplicate section wins."""
    out, cur, seen = {}, None, set()
    try:
        text = open(path, 'rb').read().decode('cp950', 'replace')
    except OSError:
        return out
    for ln in text.splitlines():
        t = ln.strip()
        if t.startswith('[') and ']' in t:
            name = t[1:t.index(']')].strip().lower()
            if name in seen:
                cur = None                      # later duplicate: invisible to the Win32 reader
            else:
                seen.add(name)
                cur = out.setdefault(name, {})
        elif cur is not None and '=' in t and not t.startswith(';'):
            k, v = t.split('=', 1)
            k = k.strip().lower()
            if k not in cur:
                cur[k] = v.strip()
    return out


def scan_recipes(roots):
    """{file_lower: {'n': folders having the file, 'keys': Counter((sec, key))}}, folders scanned."""
    stats = collections.defaultdict(lambda: {'n': 0, 'keys': collections.Counter(), 'name': None})
    folders = 0
    for root in roots:
        if not os.path.isdir(root):
            continue
        for d in sorted(os.listdir(root)):
            p = os.path.join(root, d)
            if not os.path.isdir(p):
                continue
            files = [f for f in os.listdir(p) if f.lower().endswith(('.data', '.ini'))]
            if not files:
                continue
            folders += 1
            for f in files:
                s = stats[f.lower()]
                s['n'] += 1
                s['name'] = s['name'] or f
                for sec, kv in parse_ini(os.path.join(p, f)).items():
                    for k in kv:
                        s['keys'][(sec, k)] += 1
    return stats, folders

# ---------------------------------------------------------------------------------------------------------------------
#  binary layouts
# ---------------------------------------------------------------------------------------------------------------------


BINARY = [  # (struct, header, file it is dumped to)
    ('TECH', 'LastSet.h', 'system\\tech.dat'),
    ('LAST_GENERAL_SET', 'LastSet.h', 'system\\lastdata.dat (+ _backup, _backup2)'),
    ('MachRec', 'cinitial.cpp', 'system\\machinerecord.dat (+ machinerecordRealCCD.dat)'),
    ('PASS_WORD', 'cprod.h', 'system\\login.dat'),
    ('LAST_LEVEL_SET', 'cprod.h', 'system\\levelset.dat'),
    ('AUTOTEACH_POINT', 'cprod.h', 'system\\AutoTeach_*.dat'),
]


def binary_layouts(port_root, golden_root):
    """struct_layout_across_trees.py compares its own four trees (golden906 via $NB2_GOLDEN_UTF8, port906 = the tree it
    lives in, V899, V912); it decodes cp950 when UTF-8 fails, so the original golden folder works as-is."""
    tool = os.path.join(port_root, 'tools', 'nb2_assist', 'struct_layout_across_trees.py')
    env = dict(os.environ, PYTHONIOENCODING='utf-8')
    if golden_root and os.path.isdir(golden_root):
        env['NB2_GOLDEN_UTF8'] = golden_root
    out = []
    for struct, header, dat in BINARY:
        r = subprocess.run([sys.executable, '-B', tool, '--struct', struct, '--header', header], capture_output=True, env=env)
        out.append((struct, header, dat, r.returncode, (r.stdout + r.stderr).decode('utf-8', 'replace')))
    return out

# ---------------------------------------------------------------------------------------------------------------------
#  comparison + report
# ---------------------------------------------------------------------------------------------------------------------


def ident(r):
    sec = r.section.lower().strip() if r.section is not None else ('~' + (r.sec_pat or '?'))
    key = r.key.lower().strip() if r.key is not None else ('~' + (r.key_pat or '?'))
    return sec, key


def group(recs):
    g = collections.defaultdict(lambda: collections.defaultdict(list))
    for r in recs:
        g[r.file][ident(r)].append(r)
    return g


def shown(rs):
    r = rs[0]
    sec = r.section if r.section is not None else r.sec_pat
    key = r.key if r.key is not None else r.key_pat
    return '[%s] %s' % (sec, key)


def modes(rs):
    return ''.join(sorted(set(x.mode for x in rs)))


def defaults(rs):
    return sorted(set(x.default for x in rs if x.default is not None))


def locs(rs, n=3):
    v = sorted(set(x.loc for x in rs))
    return ', '.join(v[:n]) + (' …(+%d)' % (len(v) - n) if len(v) > n else '')


def md_escape(s):
    return str(s).replace('|', '\\|')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--old', action='append', default=[], help='NAME=PATH of an old BCB6 tree (repeatable)')
    ap.add_argument('--new', default=PORT)
    ap.add_argument('--recipes', action='append', default=[])
    ap.add_argument('--md')
    ap.add_argument('--json')
    ap.add_argument('--no-binary', action='store_true')
    a = ap.parse_args()
    olds = [tuple(x.split('=', 1)) for x in (a.old or [d for d in DEFAULT_OLDS if os.path.isdir(d.split('=', 1)[1])])]
    recipe_roots = a.recipes or sorted(os.path.join(DEFAULT_RECIPE_PARENT, d).replace('\\', '/')
                                       for d in (os.listdir(DEFAULT_RECIPE_PARENT) if os.path.isdir(DEFAULT_RECIPE_PARENT) else [])
                                       if d.startswith('Data') and os.path.isdir(os.path.join(DEFAULT_RECIPE_PARENT, d)))

    new_src = load_tree('new', a.new)
    build_index('new', new_src)
    new_recs = [r for s in new_src for r in extract(s)]
    G_new = group(new_recs)
    report = {'new': a.new, 'olds': [], 'data': {}, 'binary': []}
    old_groups = []
    import datetime
    md = ['# G-031 format difference list -- raw output of tools/g031_format_diff.py', '',
          'Generated %s with `python %s`. The curated, checked list is `docs/G031_FORMAT_DIFF.md`; this file is the '
          'tool output, row by row, regenerated by rerunning the command.' % (
              datetime.datetime.now().strftime('%Y-%m-%d %H:%M'), ' '.join(['tools/g031_format_diff.py'] + sys.argv[1:])), '',
          'new = `%s` (%d sources, %d INI accesses)' % (a.new, len(new_src), len(new_recs)), '']

    stats, nfold = scan_recipes(recipe_roots)
    report['data'] = {'roots': recipe_roots, 'folders': nfold}

    for oname, oroot in olds:
        old_src = load_tree('old', oroot)
        build_index('old', old_src)
        old_recs = [r for s in old_src for r in extract(s)]
        G_old = group(old_recs)
        old_groups.append((oname, {f.lower(): v for f, v in G_old.items()}))
        md += ['## %s -> new' % oname, '', 'old = `%s` (%d sources, %d INI accesses)' % (oroot, len(old_src), len(old_recs)), '']
        files = sorted(set(G_old) | set(G_new), key=lambda f: (f == '?', f.lower()))
        summary = []
        per_file = {}
        for f in files:
            o, n = G_old.get(f, {}), G_new.get(f, {})
            only_o = sorted(set(o) - set(n))
            only_n = sorted(set(n) - set(o))
            both = sorted(set(o) & set(n))
            ddiff = [k for k in both if defaults(o[k]) and defaults(n[k]) and defaults(o[k]) != defaults(n[k])]
            dconf = [k for k in ddiff if not (set(defaults(o[k])) <= set(defaults(n[k])) or set(defaults(n[k])) <= set(defaults(o[k])))]
            dsub = [k for k in ddiff if k not in dconf]
            summary.append((f, len(o), len(n), len(only_o), len(only_n), len(dconf), len(dsub)))
            per_file[f] = {
                'only_old': [{'key': shown(o[k]), 'modes': modes(o[k]), 'defaults': defaults(o[k]), 'where': locs(o[k], 6)} for k in only_o],
                'only_new': [{'key': shown(n[k]), 'modes': modes(n[k]), 'defaults': defaults(n[k]), 'where': locs(n[k], 6),
                              'layers': sorted(set(x.layer for x in n[k]))} for k in only_n],
                'default_diff': [{'key': shown(o[k]), 'old': defaults(o[k]), 'new': defaults(n[k]),
                                  'old_where': locs(o[k]), 'new_where': locs(n[k])} for k in dconf],
                'default_subset': [{'key': shown(o[k]), 'old': defaults(o[k]), 'new': defaults(n[k]),
                                    'old_where': locs(o[k]), 'new_where': locs(n[k])} for k in dsub],
            }
        md += ['| file | old keys | new keys | only old | only new | default conflicts | default sets differ (subset) |',
               '|---|---:|---:|---:|---:|---:|---:|']
        md += ['| %s | %d | %d | %d | %d | %d | %d |' % s for s in summary]
        md += ['']
        for f in files:
            pf = per_file[f]
            if not (pf['only_old'] or pf['only_new'] or pf['default_diff'] or pf['default_subset']):
                continue
            md += ['### %s' % ('(file not attributed)' if f == '?' else f), '']
            if pf['only_old']:
                md += ['**Only %s reads/writes it** (%d):' % (oname, len(pf['only_old'])), '',
                       '| key | mode | default(s) | where |', '|---|---|---|---|']
                md += ['| %s | %s | %s | %s |' % (md_escape(x['key']), x['modes'], md_escape(', '.join(x['defaults'])), md_escape(x['where']))
                       for x in pf['only_old']]
                md += ['']
            if pf['only_new']:
                md += ['**Only the port reads/writes it** (%d):' % len(pf['only_new']), '',
                       '| key | mode | default(s) | layer | where |', '|---|---|---|---|---|']
                md += ['| %s | %s | %s | %s | %s |' % (md_escape(x['key']), x['modes'], md_escape(', '.join(x['defaults'])),
                                                        '/'.join(x['layers']), md_escape(x['where'])) for x in pf['only_new']]
                md += ['']
            if pf['default_diff']:
                md += ['**Same key, different default (conflict)** (%d):' % len(pf['default_diff']), '',
                       '| key | %s default | port default | %s where | port where |' % (oname, oname), '|---|---|---|---|---|']
                md += ['| %s | %s | %s | %s | %s |' % (md_escape(x['key']), md_escape(', '.join(x['old'])), md_escape(', '.join(x['new'])),
                                                        md_escape(x['old_where']), md_escape(x['new_where'])) for x in pf['default_diff']]
                md += ['']
            if pf['default_subset']:
                md += ['**Default sets differ only by an extra registration on one side** (%d) -- the key is registered in several '
                       'places with different defaults; check whether the extra place is reached:' % len(pf['default_subset']), '',
                       '| key | %s defaults | port defaults | %s where | port where |' % (oname, oname), '|---|---|---|---|---|']
                md += ['| %s | %s | %s | %s | %s |' % (md_escape(x['key']), md_escape(', '.join(x['old'])), md_escape(', '.join(x['new'])),
                                                        md_escape(x['old_where']), md_escape(x['new_where'])) for x in pf['default_subset']]
                md += ['']
        unres = [r for r in old_recs if r.file == '?']
        report['olds'].append({'name': oname, 'root': oroot, 'sources': len(old_src), 'accesses': len(old_recs),
                               'unattributed': len(unres), 'summary': summary, 'files': per_file})

    # data presence vs the port's literal keys
    md += ['## Real recipe folders vs the port', '',
           '%d recipe folders under %s. "unknown to the port" = present in the data, never named by the port '
           '(literal or computed pattern); the converter can drop or must map these. "missing in data" = the port '
           'reads it but N folders lack it (the port then uses its default; CheckAndRead* also writes it back).' % (nfold, ', '.join(recipe_roots)), '']
    new_by_file = {f.lower(): v for f, v in G_new.items()}

    def matcher(keys):
        lit = {k for k in keys if not k[0].startswith('~') and not k[1].startswith('~')}
        pats = [k for k in keys if k[0].startswith('~') or k[1].startswith('~')]

        def known(sec, key):
            if (sec, key) in lit:
                return True
            for ps, pk in pats:
                okS = (not ps.startswith('~') and ps == sec) or ps.startswith('~<') or \
                      (ps.startswith('~') and bool(re.fullmatch(ps[1:], sec, re.I)))
                okK = (not pk.startswith('~') and pk == key) or pk.startswith('~<') or \
                      (pk.startswith('~') and bool(re.fullmatch(pk[1:], key, re.I)))
                if okS and okK:
                    return True
            return False
        return known

    for fl in sorted(stats):
        s = stats[fl]
        cf = canon(s['name']).lower()
        nkeys = new_by_file.get(cf, {})
        lit = {k for k in nkeys if not k[0].startswith('~') and not k[1].startswith('~')}
        known = matcher(nkeys)
        old_known = [(on, matcher(og.get(cf, {}))) for on, og in old_groups]
        unknown = sorted(((sec, key), c) for (sec, key), c in s['keys'].items() if not known(sec, key))
        unk_old = {k: [on for on, kn in old_known if kn(*k)] for k, _c in unknown}
        missing = sorted((k, s['n'] - s['keys'].get(k, 0)) for k in lit
                         if any(x.mode in ('r', 'r+', 'rw', 'list') for x in nkeys[k]) and s['keys'].get(k, 0) < s['n'])
        report['data'][s['name']] = {'folders': s['n'], 'file': canon(s['name']), 'unknown_to_port': unknown,
                                     'unknown_known_by_old': {'[%s] %s' % k: v for k, v in unk_old.items()}, 'missing_in_data': missing}
        md += ['### %s (%d folders)' % (s['name'], s['n']), '']
        if not nkeys:
            md += ['The port names no key of this file (not read by the port).', '']
            continue
        legacy = [(k, c) for k, c in unknown if not unk_old[k]]
        gap = [(k, c) for k, c in unknown if unk_old[k]]
        md += ['- unknown to the port, but an old tree names it (unported feature or port gap): %d key(s)%s' % (
                   len(gap), (': ' + ', '.join('[%s] %s ×%d (%s)' % (k[0], k[1], c, '/'.join(unk_old[k])) for k, c in gap[:40]) +
                              (' …' if len(gap) > 40 else '')) if gap else ''),
               '- unknown to every tree (legacy keys written by older versions; harmless to keep): %d key(s)%s' % (
                   len(legacy), (': ' + ', '.join('[%s] %s ×%d' % (k[0], k[1], c) for k, c in legacy[:25]) +
                                 (' …' if len(legacy) > 25 else '')) if legacy else ''),
               '- read by the port but missing in some folders: %d key(s)%s' % (
                   len(missing), (': ' + ', '.join('[%s] %s (missing in %d)' % (k[0], k[1], c) for k, c in missing[:40]) +
                                  (' …' if len(missing) > 40 else '')) if missing else ''), '']

    if not a.no_binary:
        g906 = 'D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618'
        md += ['## Binary structs dumped whole to system files', '']
        for struct, header, dat, rc, out in binary_layouts(a.new, g906):
            report['binary'].append({'struct': struct, 'header': header, 'file': dat, 'rc': rc, 'output': out})
            md += ['### %s (`%s`) -> %s' % (struct, header, dat), '', '```', out.strip(), '```', '']

    text = '\n'.join(md) + '\n'
    if a.md:
        open(a.md, 'w', encoding='utf-8', newline='\n').write(text)
    if a.json:
        json.dump(report, open(a.json, 'w', encoding='utf-8'), ensure_ascii=False, indent=1, default=str)
    if not a.md:
        sys.stdout.reconfigure(encoding='utf-8')
        print(text)
    else:
        for o in report['olds']:
            print('%s: %d sources, %d accesses, %d not attributed' % (o['name'], o['sources'], o['accesses'], o['unattributed']))
        print('new: %d sources, %d accesses, %d not attributed' % (len(new_src), len(new_recs), sum(1 for r in new_recs if r.file == '?')))
        print('data: %d recipe folders' % nfold)


if __name__ == '__main__':
    main()
