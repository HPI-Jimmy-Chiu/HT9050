# -*- coding: utf-8 -*-
"""tools/webprobe/c12_button_census.py -- AI(W906-ST02-C12) 20261003 (St02-E): ST02-C12 "visible but does nothing" button census,
STATIC half (TO_STEVEN.md section 3 ST02-C12; St02-M 1003 pick A).  Measure and classify only -- this script changes nothing.

For every button on every operator page of web/background.html's WINDOWS table (plus the Alert overlays):
  web     the element (id, golden type from the generator's title="<id> : <TType>", caption), statically hidden / greyed,
          and the binding evidence in the page's own scripts (inline + every <script src>): file:line of each reference
          to the id, data-ht-event, onclick=, plus the command literals found next to those references (a GUESS);
  C++     whether the port knows that command: string literals of tools/wb_serve.cpp, WebBridge/, JsonBridge/, FileRW/,
          forms/ (not tests/), prefix dispatchers (cmd.compare(0, n, "act.xxx.")), and the form.event tables
          (FileRW/*.gen.inc PageEvent rows {"control","event","<golden>",...}, JsonBridge FormBridge events);
  golden  906 0618 (RULINGS_20261003 #2; 0625_Steven is a comparison only -- pass it with --golden): the dfm OnClick
          handler, its body (file:line, statement count), Visible / Enabled in the dfm, the code lines that set the
          component's Visible / Enabled, and whether the handler body moves a motor / writes an output (D);
  machine docs/WORKLOG_MACHINE.md section 4 mentions the id or the page (E).
Category (card ST02-C12): A web only / B C++ not translated / C golden hides it or does nothing here / D moves the machine or
writes outputs (for Jimmy, not dispatched) / E machine side is on it; plus OK-cmd / OK-ui (statically bound) and ? (needs the
click run).  The "sent" column is UNVERIFIED until tools/webprobe/c12_click_probe.py (headless Edge, stub server) is run
(St01 proxy run) and merged with --merge.
Usage:
  python c12_button_census.py [--golden <906 tree>] [--out-tsv <tsv>] [--out-md <md>] [--merge <click_result.tsv>]
Exit 0 always (a census, not a gate).  Read-only: reads the repo and the golden tree, writes only --out-tsv / --out-md."""
import argparse, collections, html.parser, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
TREE = os.path.normpath(os.path.join(HERE, '..', '..'))            # HT9011UC_Cpp_V3.33.906.0
REPO = os.path.normpath(os.path.join(TREE, '..'))
WEB = os.path.join(REPO, 'web')
PAGE = os.path.join(WEB, 'page')
GOLDEN_DEFAULT = 'D:/HT9045/HT9011UC_Code_V3.33.906.0_20260618'   # AI(W906-ST02-C12) 20261004 (St02-E): 0618 is the basis (RULINGS_20261003 #2)
BS = chr(92)
BTN_TYPES = ('TButton', 'TBitBtn', 'TSpeedButton')


def rd(path, enc='utf-8'):
    with open(path, 'rb') as f:
        b = f.read()
    try:
        return b.decode(enc)
    except UnicodeDecodeError:
        return b.decode('cp950' if enc == 'utf-8' else 'utf-8', 'replace')


def relp(path, root):
    return os.path.relpath(path, root).replace(BS, '/')


# ============================================================================ web: pages in scope
def window_pages():
    """(window id, golden form object, page file) from background.html WINDOWS + the Alert overlays."""
    txt = rd(os.path.join(WEB, 'background.html'))
    out = []
    for m in re.finditer(r"\{id:'([^']+)',\s*form:(?:'([^']*)'|null)[^}]*?src:'page/([^']+\.html)'", txt):
        out.append((m.group(1), m.group(2) or '', m.group(3)))
    seen = set(p for _, _, p in out)
    for f in sorted(os.listdir(PAGE)):
        if f.startswith('Alert.') and f.endswith('.html') and f not in seen:
            out.append(('(overlay)', '', f))
    return out


DEV_PAGE = re.compile(r'^(IDE\.|ScreenShots)')


class PageParser(html.parser.HTMLParser):
    VOID = {'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'param', 'source', 'track', 'wbr'}

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.stack = []          # [tag, attrs, hidden_here, rec_or_None]
        self.buttons = []
        self.scripts = []        # src list
        self.inline = []         # (start line, text)
        self._in_script = None
        self.title = ''
        self._in_title = False

    def _hidden(self, a):
        st = (a.get('style') or '').replace(' ', '').lower()
        t = a.get('title') or ''
        cls = (a.get('class') or '').split()
        if ': TTabSheet' in t or 'pane' in cls or 'pcPane' in cls or 'data-tab' in a or 'data-pane' in a:
            return False                                   # an inactive tab sheet (div.pcPane data-tab=...) is reachable by its tab
        return 'display:none' in st or 'visibility:hidden' in st or 'hidden' in a

    def handle_starttag(self, tag, attrs):
        a = dict((k, v if v is not None else '') for k, v in attrs)
        if tag == 'script':
            if a.get('src'):
                self.scripts.append(a['src'])
            self._in_script = [self.getpos()[0], []]
        if tag == 'title':
            self._in_title = True
        rec = None
        t = a.get('title') or ''
        m = re.match(r'^([A-Za-z_]\w*) : (T\w+)', t)          # "spbExit : TSpeedButton（統一 exit 樣式，原 TPanel）" too
        typ = m.group(2) if m else ''
        is_btn = (typ in BTN_TYPES) or tag == 'button' or (tag == 'input' and (a.get('type') or '').lower() in ('button', 'submit'))
        if is_btn:
            anc_hidden = any(s[2] for s in self.stack)
            sheet = ''
            for s in reversed(self.stack):
                if ': TTabSheet' in (s[1].get('title') or ''):
                    sheet = s[1].get('id') or ''
                    break
            rec = {'id': a.get('id') or '', 'tag': tag, 'gtype': typ, 'golden_name': m.group(1) if m else '',
                   'line': self.getpos()[0], 'caption': (a.get('value') or '') if tag == 'input' else '',
                   'hidden': self._hidden(a) or anc_hidden, 'disabled': ('disabled' in a) or a.get('aria-disabled') == 'true'
                   or 'disabled' in (a.get('class') or '').split(), 'cls': a.get('class') or '',
                   'onclick': a.get('onclick') or '', 'data': dict((k, v) for k, v in a.items() if k.startswith('data-')),
                   'sheet': sheet}
            self.buttons.append(rec)
        if tag not in self.VOID:
            self.stack.append([tag, a, self._hidden(a), rec])

    def handle_endtag(self, tag):
        if tag == 'script' and self._in_script:
            self.inline.append((self._in_script[0], ''.join(self._in_script[1])))
            self._in_script = None
        if tag == 'title':
            self._in_title = False
        for i in range(len(self.stack) - 1, -1, -1):
            if self.stack[i][0] == tag:
                del self.stack[i:]
                break

    def handle_data(self, data):
        if self._in_script is not None:
            self._in_script[1].append(data)
            return
        if self._in_title:
            self.title += data
        for s in reversed(self.stack):
            if s[3] is not None:
                if len(s[3]['caption']) < 60:
                    s[3]['caption'] += data.strip()
                break


# ============================================================================ JS text: strip comments, keep strings
def strip_js_comments(t):
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c in '"\'`':
            j = i + 1
            while j < n and t[j] != c:
                if t[j] == BS:
                    j += 1
                elif t[j] == '\n' and c != '`':
                    break
                j += 1
            out.append(t[i:j + 1])
            i = j + 1
            continue
        if c == '/' and i + 1 < n and t[i + 1] == '/':
            j = t.find('\n', i)
            j = n if j < 0 else j
            i = j
            continue
        if c == '/' and i + 1 < n and t[i + 1] == '*':
            j = t.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append('\n' * t.count('\n', i, j))           # keep line numbers
            i = j
            continue
        out.append(c)
        i += 1
    return ''.join(out)


JS_CACHE = {}


def js_text(path):
    if path not in JS_CACHE:
        try:
            JS_CACHE[path] = strip_js_comments(rd(path))
        except OSError:
            JS_CACHE[path] = None
    return JS_CACHE[path]


CMD_LIT = re.compile(r'''(?:rawCmd|sendCmd|wsCmd|callCmd|\.cmd)\(\s*['"]([A-Za-z][\w.]*)['"]|\bcmd\s*[:=]\s*['"]([A-Za-z][\w.]*)['"]'''
                     r'''|['"]((?:act|main|start|pause|motor|io|dialog|form|editlist|recipe|system|security|lotinfo|towerlight|counterclear|'''
                     r'''counter|smartdiag|builder|observer|contactct|ttlcfg|cfgtrayplate|auth|ui|sim|log|lot|sys|struct|hw|cfg|modal)\.[\w.]+)['"]''')
UI_ACT = re.compile(r'\b(openWin|closeWin|HT_WIN|postMessage|window\.close|htClose|closeSelf|showTab|selectTab|switchTab|'
                    r'printPage|window\.print|download|exportCsv|toggle)\b')


def refs_in(text, ident):
    """line numbers (1-based) where ident appears as a string literal / #selector / object key."""
    pat = re.compile(r'''(['"`])%s\1|#%s(?![\w-])|(?<![\w$.])%s\s*:(?!:)|\[id=['"]?%s['"\]]''' % ((re.escape(ident),) * 4))
    out = []
    for m in pat.finditer(text):
        out.append(text.count('\n', 0, m.start()) + 1)
    return out


def near_cmds(text, line, span=40):
    ls = text.split('\n')
    seg = '\n'.join(ls[max(0, line - 3):line + span])
    cmds = []
    for m in CMD_LIT.finditer(seg):
        c = m.group(1) or m.group(2) or m.group(3)
        if c and c not in cmds:
            cmds.append(c)
    ui = sorted(set(m.group(1) for m in UI_ACT.finditer(seg)))
    return cmds, ui


# ============================================================================ C++: what the port dispatches
def cpp_known():
    lits, prefixes, events = set(), set(), set()
    skip = ('tests', 'build', 'build_ship', '.git')
    for root, dirs, files in os.walk(TREE):
        dirs[:] = [d for d in dirs if d not in skip and not d.startswith('build')]
        for f in files:
            if not f.endswith(('.cpp', '.h', '.inc')):
                continue
            p = os.path.join(root, f)
            t = rd(p)
            for m in re.finditer(r'"([A-Za-z][\w]*(?:\.[\w]+)+\.?)"', t):
                lits.add(m.group(1))
            for m in re.finditer(r'compare\(\s*0\s*,\s*\d+\s*,\s*"([\w.]+\.)"\s*\)\s*==\s*0', t):
                prefixes.add(m.group(1))
            for m in re.finditer(r'\{\s*"(\w+)"\s*,\s*"(click|change|dblclick)"\s*,\s*"[^"]*?(T\w+)::(\w+)"', t):
                events.add((m.group(3), m.group(1), m.group(2)))
    return lits, prefixes, events


def port_handler_words():
    """{word: set(rel files)} for every identifier containing Click / Change in the port's C++ code (comments stripped)."""
    idx = collections.defaultdict(set)
    skip = ('tests', '.git')
    for root, dirs, files in os.walk(TREE):
        dirs[:] = [d for d in dirs if d not in skip and not d.startswith('build')]
        for f in files:
            if not f.endswith(('.cpp', '.h', '.inc')):
                continue
            p = os.path.join(root, f)
            st = strip_cpp(rd(p))
            for w in set(re.findall(r'\w*(?:Click|Change|DblClick)\w*', st)):
                parts = w.split('_')
                for i in range(len(parts)):                    # SP_Ev_cbIndexArmClick also counts for cbIndexArmClick
                    idx['_'.join(parts[i:])].add(relp(p, TREE))
    return idx


def port_has(idx, name):
    return sorted(idx.get(name, ()))[:4] if name else []


SHIM_DIR = os.path.join(WEB, 'JSON', 'js')


def expand_scripts(srcs, texts):
    """add scripts a page loads at run time: string literals ending in .js that exist next to the page, and JSON shims
    (web/JSON/js/<name>.js) whose <name> appears as a literal ('teach-access', '../JSON/teach-access.json')."""
    shims = set(f[:-3] for f in os.listdir(SHIM_DIR) if f.endswith('.js')) if os.path.isdir(SHIM_DIR) else set()
    out = list(srcs)
    for t in texts:
        if not t:
            continue
        for m in re.finditer(r'''['"]([\w./-]+\.js)['"]''', t):
            for base in (PAGE, WEB):
                p = os.path.normpath(os.path.join(base, m.group(1)))
                if os.path.exists(p) and p not in out:
                    out.append(p)
                    break
        for m in re.finditer(r'''['"](?:[\w./-]*/)?([\w-]+)(?:\.json)?['"]''', t):
            n = m.group(1)
            if n in shims and 'i18n' not in n.lower():
                p = os.path.join(SHIM_DIR, n + '.js')
                if p not in out:
                    out.append(p)
    return out


UNWIRED_CTX = re.compile(r'UNWIRED|[Uu]nwired|teach-unwired|data-unwired|還沒接|沒接功能|未接')
GENERIC_CLS = {'btn3d', 'btn', 'button', 'ed', 'pnl', 'tab', 'act', 'down', 'up', 'flat', 'small', 'big', 'wide', 'grid'}
UNWIRED_FILE = re.compile(r'unwired|unported')            # ht9045_unported_form_c.js, ht9045_iosetview_unwired_c.js: grey + reason
NONBIND_FILE = re.compile(r'layout|i18n|theme|style|lang', re.I)   # offsets / captions / colours, never a click binding


def cmd_known(c, lits, prefixes):
    if c in lits:
        return 'yes'
    for p in prefixes:
        if c.startswith(p):
            suf = c[len(p):]
            return 'prefix(%s)%s' % (p, '' if not suf or suf in lits or any(l.endswith('.' + suf) for l in lits) else ' suffix?')
    return 'no'


# ============================================================================ golden 906 (0618; 0625_Steven via --golden)
OBJ = re.compile(r'^\s*(object|inherited|inline)\s+(\w+)\s*:\s*(\w+)')
PROP = re.compile(r'^\s*([\w.]+)\s*=\s*(.*)$')


def parse_dfm(path):
    lines = rd(path, 'cp950').replace('\r\n', '\n').split('\n')
    comps, stack = {}, []
    i, n = 0, len(lines)
    while i < n:
        l = lines[i]
        m = OBJ.match(l)
        if m:
            node = {'name': m.group(2), 'type': m.group(3), 'line': i + 1, 'props': {},
                    'anc': [s['name'] for s in stack]}
            stack.append(node)
            comps.setdefault(m.group(2), node)
            comps.setdefault('__root__', node)              # the first object = the form (fCleaning: TfCleaning)
            i += 1
            continue
        if l.strip() == 'end' and stack:
            stack.pop()
            i += 1
            continue
        if stack:
            pm = PROP.match(l)
            if pm:
                k, v = pm.group(1), pm.group(2).strip()
                if v and v[-1] in '({<' and not v.startswith("'"):
                    closer = {'(': ')', '{': '}', '<': '>'}[v[-1]]
                    j = i + 1
                    while j < n and not lines[j].rstrip().endswith(closer):
                        j += 1
                    i = j + 1
                    continue
                stack[-1]['props'][k] = v
        i += 1
    return comps


def strip_cpp(t):
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c == '"' or c == "'":
            j = i + 1
            while j < n and t[j] != c and t[j] != '\n':
                if t[j] == BS:
                    j += 1
                j += 1
            out.append(c + ' ' * max(0, j - i - 1) + c)
            i = j + 1
            continue
        if c == '/' and i + 1 < n and t[i + 1] == '/':
            j = t.find('\n', i)
            j = n if j < 0 else j
            out.append(' ' * (j - i))
            i = j
            continue
        if c == '/' and i + 1 < n and t[i + 1] == '*':
            j = t.find('*/', i + 2)
            j = n if j < 0 else j + 2
            out.append(''.join('\n' if ch == '\n' else ' ' for ch in t[i:j]))
            i = j
            continue
        out.append(c)
        i += 1
    return ''.join(out)


#   D = the handler body itself moves a motor / switches an output / starts the machine (only the body, calls are not followed).
#   Reading MOT[i] (a teach Set reads the position) is NOT D.
MOVE_M = re.compile(r'^(?!Is|Get|Check|Read|Wait|Can)\w*(Mov|Jog|Home|Servo|Stop|Brake|SetPosition|SetCmdPos)\w*$')   # Gali_MotMove, MotorMove, ServoOnOff
DANGER = re.compile(r'(?:\bMOT|\bMotor\w*)\s*\[[^\]]*\]\s*(?:\.|->)\s*(\w+)\s*\(|'
                    r'\b(SW|Cylinder|Valve|Suck)\s*\[[^\]]*\]\s*(?:\.|->)\s*(On|Off|Set\w*|Toggle|Open|Close|Up|Down|Do\w*)\s*\(|'
                    r'\b(SetOutput\w*|OutPortData|WriteOutput\w*|DoOutput\w*|SetDO\w*|SetOutBit\w*|WriteDO\w*|ADAM_Write\w*|'
                    r'StopAllMotor|ProcessMotorHome|DoAllMotorHome\w*|ServoOnAll\w*|ServoOffAll\w*)\s*\(|'
                    r'\bfMain\s*->\s*(Start|Reset|Pause|BtnStartClick|BtnHomeClick)\s*\(|\b(SystemStart)\s*=\s*true')


def danger_of(body):
    out = set()
    for m in DANGER.finditer(body):
        if m.group(1):
            if MOVE_M.match(m.group(1)):
                out.add('MOT[].' + m.group(1))
        elif m.group(2):
            out.add('%s[].%s' % (m.group(2), m.group(3)))
        elif m.group(4):
            out.add(m.group(4))
        elif m.group(5):
            out.add('fMain->' + m.group(5))
        elif m.group(6):
            out.add('SystemStart=true')
    return sorted(out)[:6]


class Golden(object):
    def __init__(self, root):
        self.root = root
        self.cpp = {}          # rel -> (raw lines, stripped text)
        self.defs = {}         # (Class, Method) -> (rel, line, start_off, end_off)
        if not os.path.isdir(root):
            print('golden tree not found: ' + root, file=sys.stderr)
            return
        for r, ds, fs in os.walk(root):
            for f in fs:
                if not f.lower().endswith('.cpp') or f.startswith('~') or '.~' in f:
                    continue
                p = os.path.join(r, f)
                rel = relp(p, root)
                raw = rd(p, 'cp950').replace('\r\n', '\n').replace('\r', '\n')
                st = strip_cpp(raw)
                self.cpp[rel] = (raw.split('\n'), st)
                for m in re.finditer(r'(?m)^[ \t]*(?:[\w<>\*&]+[ \t]+)*?__fastcall[ \t]+(\w+)::(~?\w+)[ \t]*\(', st):
                    b = self._body(st, m.end())
                    if b:
                        self.defs.setdefault((m.group(1), m.group(2)), (rel, st.count('\n', 0, m.start()) + 1, m.start(), b[0], b[1]))

    @staticmethod
    def _body(st, pos):
        pd, i, n = 1, pos, len(st)
        while i < n and pd:
            if st[i] == '(':
                pd += 1
            elif st[i] == ')':
                pd -= 1
            i += 1
        while i < n and st[i] in ' \t\r\n':
            i += 1
        if i >= n or st[i] != '{':
            return None
        d, j = 0, i
        while j < n:
            if st[j] == '{':
                d += 1
            elif st[j] == '}':
                d -= 1
                if d == 0:
                    return (i, j)
            j += 1
        return None

    def handler(self, cls, meth):
        d = self.defs.get((cls, meth))
        if not d:
            return None
        rel, line, defoff, a, b = d
        body = self.cpp[rel][1][a + 1:b]
        stmts = len([x for x in re.split(r'[;{}]', body) if x.strip()])
        danger = danger_of(body)
        end_line = line + self.cpp[rel][1].count('\n', defoff, b)
        return {'loc': '%s:%d-%d' % (rel, line, end_line), 'stmts': stmts, 'danger': danger,
                'closes': bool(re.search(r'(?<![\w>.])Close\s*\(\s*\)', body)),
                'opens': sorted(set(re.findall(r'\b(f\w+|F\w+)\s*->\s*Show(?:Modal)?\s*\(', body)))[:4]}

    VIS = re.compile(r'(?:\b(\w+)\s*->\s*)?(?<![\w])(\w+)\s*->\s*(Visible|Enabled|TabVisible)\s*=\s*([^;]+);')

    def _vis_index(self):
        self.vis = collections.defaultdict(list)       # comp -> [(rel, line, qualifier, prop, value)]
        for rel, (raw, st) in self.cpp.items():
            for m in self.VIS.finditer(st):
                self.vis[m.group(2)].append((rel, st.count('\n', 0, m.start()) + 1, m.group(1) or '', m.group(3),
                                             re.sub(r'\s+', '', m.group(4))[:30]))

    def vis_sites(self, comp, form_obj, own_cpp):
        """code lines that set comp->Visible / ->Enabled: in the form's own cpp (bare or this->) or anywhere qualified by the form object."""
        if not hasattr(self, 'vis'):
            self._vis_index()
        out = []
        for rel, ln, q, prop, val in self.vis.get(comp, []):
            if (q == form_obj and form_obj) or (rel in own_cpp and q in ('', 'this')):
                out.append('%s:%d %s=%s' % (rel, ln, prop, val))
        return out


# ============================================================================ main census
def machine_section4():
    p = os.path.join(TREE, 'docs', 'WORKLOG_MACHINE.md')
    try:
        t = rd(p)
    except OSError:
        return ''
    m = re.search(r'\n## 4\..*?(?=\n## 5\.|\Z)', t, re.S)
    return m.group(0) if m else ''


def census(golden_root):
    lits, prefixes, fevents = cpp_known()
    CPP_LITS[0], CPP_LITS[1] = lits, prefixes
    pidx = port_handler_words()
    G = Golden(golden_root)
    sec4 = machine_section4()
    rows = []
    pages = window_pages()
    for win, form_obj, page in pages:
        path = os.path.join(PAGE, page)
        if not os.path.exists(path):
            continue
        pp = PageParser()
        pp.feed(rd(path))
        # title forms seen: "（AutoClean/uCleaning.dfm / fCleaning : TfCleaning）", "（cContactCT.dfm）", "（main.dfm tsMotorView）",
        # "（uShowMessage.cpp / fShowMessage）" -> the dfm; the form object / class come from the dfm's root object below
        tm = re.search(r'([\w/' + re.escape(BS) + r'.-]+)\.(?:dfm|cpp)\b', pp.title)
        dfm_rel = (tm.group(1).strip().replace(BS, '/') + '.dfm') if tm else ''
        gform_obj = form_obj
        gclass = 'T' + gform_obj[1:] if gform_obj and gform_obj[0] == 'f' else gform_obj
        if not dfm_rel and page.startswith('Main.'):
            dfm_rel = 'main.dfm'
        comps = {}
        dfm_path = os.path.join(golden_root, dfm_rel) if dfm_rel else ''
        if dfm_rel and not os.path.exists(dfm_path):
            base = os.path.basename(dfm_rel).lower()
            for r, ds, fs in os.walk(golden_root):
                hit = [f for f in fs if f.lower() == base]
                if hit:
                    dfm_path = os.path.join(r, hit[0])
                    dfm_rel = relp(dfm_path, golden_root)
                    break
        if dfm_path and os.path.exists(dfm_path):
            comps = parse_dfm(dfm_path)
            root = comps.get('__root__')
            if root:
                gform_obj, gclass = root['name'], root['type']
        own_cpp = set()
        if dfm_rel:
            stem = dfm_rel[:-4]
            own_cpp = set(k for k in G.cpp if k[:-4].lower() == stem.lower())
        texts = [('(inline)', pp.inline)]
        srcs = []
        for s in pp.scripts:
            sp = os.path.normpath(os.path.join(PAGE, s.split('?')[0]))
            srcs.append(sp)
        srcs = expand_scripts(srcs, [strip_js_comments(t) for _, t in pp.inline] + [js_text(s) for s in srcs])
        sec4_page = any(k and k in sec4 for k in (page[:-5], gclass, gform_obj))
        for b in pp.buttons:
            ident = b['id']
            r = {'window': win, 'page': page, 'dev': bool(DEV_PAGE.match(page)), 'id': ident or '(no id)', 'html_line': b['line'],
                 'gtype': b['gtype'] or '(web-only)', 'caption': re.sub(r'\s+', ' ', b['caption'])[:40], 'sheet': b['sheet'],
                 'hidden': b['hidden'], 'greyed': b['disabled'] or 'unwired' in b['cls'] or 'data-unwired' in b['data'],
                 'refs': [], 'cmds': [], 'ui': [], 'htevent': b['data'].get('data-ht-event', ''), 'onclick': b['onclick'][:60]}
            r['unwired'] = ''
            if ident:
                sources = [(page, start - 1, strip_js_comments(txt)) for start, txt in pp.inline] + \
                          [(relp(sp, WEB), 0, js_text(sp)) for sp in srcs]
                for name, base, t2 in sources:
                    if not t2 or NONBIND_FILE.search(name):
                        continue
                    tl = None
                    for ln in refs_in(t2, ident):
                        if tl is None:
                            tl = t2.split('\n')
                        ctx = '\n'.join(tl[max(0, ln - 40):ln])
                        if UNWIRED_FILE.search(name) or UNWIRED_CTX.search(tl[ln - 1]) or \
                                (UNWIRED_CTX.search(ctx) and re.search(r'^\s*\[', tl[ln - 1])):
                            r['unwired'] = r['unwired'] or '%s:%d' % (name, base + ln)     # greyed at run time with a reason
                            continue
                        r['refs'].append('%s:%d' % (name, base + ln))
                        c, u = near_cmds(t2, ln)
                        r['cmds'] += [x for x in c if x not in r['cmds']]
                        r['ui'] += [x for x in u if x not in r['ui']]
                    for cl in b['cls'].split():                 # bound by class: '.exitbtn' (the unified Exit), classList.contains('x')
                        if cl in GENERIC_CLS or len(cl) < 4:
                            continue
                        for mm in re.finditer(r'''\.%s(?![\w-])|['"]%s['"]''' % (re.escape(cl), re.escape(cl)), t2):
                            ln = t2.count('\n', 0, mm.start()) + 1
                            r['refs'].append('cls.%s@%s:%d' % (cl, name, base + ln))
                            c, u = near_cmds(t2, ln)
                            r['cmds'] += [x for x in c if x not in r['cmds']]
                            r['ui'] += [x for x in u if x not in r['ui']]
                            break
            if r['onclick']:
                c, u = near_cmds(r['onclick'], 1, 1)
                r['cmds'] += c
                r['ui'] += u
            if r['htevent']:
                r['cmds'].append('form.event')
            # C++
            ck = []
            for c in r['cmds']:
                ck.append('%s=%s' % (c, cmd_known(c, lits, prefixes)))
            r['cpp'] = ck
            r['fe_row'] = (gclass, b['golden_name'] or ident, 'click') in fevents
            # golden
            gn = b['golden_name'] or ident
            comp = comps.get(gn) if gn else None
            r['g_dfm'] = '%s:%d' % (dfm_rel, comp['line']) if comp else ''
            r['g_onclick'] = comp['props'].get('OnClick', '') if comp else ''
            r['g_visible'] = comp['props'].get('Visible', '') if comp else ''
            r['g_enabled'] = comp['props'].get('Enabled', '') if comp else ''
            h = G.handler(gclass, r['g_onclick']) if r['g_onclick'] else None
            r['g_handler'] = h
            r['port_handler'] = port_has(pidx, r['g_onclick']) if r['g_onclick'] else []
            r['g_vis_sites'] = G.vis_sites(gn, gform_obj, own_cpp) if comp else []
            r['sec4'] = bool(ident and len(ident) > 3 and re.search(r'(?<![\w])%s(?![\w])' % re.escape(ident), sec4))
            r['sec4_page'] = sec4_page
            r['cat'] = classify(r)
            rows.append(r)
    return rows


def dead_cat(r):
    """a visible, enabled button whose click produced nothing (probe) / has no binding (static): why, by the golden + port side."""
    h = r['g_handler']
    golden = r['gtype'] != '(web-only)'
    if golden and r['g_dfm'] and not r['g_onclick']:
        base = 'C0'
    elif golden and h is not None and h['stmts'] == 0:
        base = 'C0'
    elif golden and r['g_visible'] == 'False' and not any('Visible=' in s and 'false' not in s.lower() for s in r['g_vis_sites']):
        base = 'C'
    elif h and h['danger']:
        base = 'D'
    elif r['fe_row'] or r['port_handler']:
        base = 'A'
    elif golden and r['g_onclick']:
        base = 'B'
    else:
        base = '?'
    if base in ('A', 'B', 'D', '?') and (r['sec4'] or r['sec4_page']):
        base += '+E?'
    return base


def classify_probe(r, lits, prefixes):
    p = r['probe']
    st = p['state']
    if st in ('missing', 'not-visible', 'greyed'):
        return {'missing': 'missing(probe)', 'not-visible': 'hidden(probe)', 'greyed': 'greyed(probe)'}[st]
    if st != 'clicked':
        return 'probe-' + st
    if p['ws']:
        names = [x.split('(')[0].split(' ')[0] for x in p['ws'] if x]
        names = [n for n in names if n not in ('control.acquire', 'control.release', 'control.takeover', 'ui.windows.put')] or names
        unknown = [n for n in names if cmd_known(n, lits, prefixes) == 'no']
        return 'B(web-sends-unknown-cmd)' if unknown else 'OK-cmd'
    if any(x.split(' ')[0] in ('POST', 'PUT') for x in p['http']):
        return 'OK-http'
    if p['frame'] or p['dlg'] or p['dom'] > 2:
        return 'OK-ui'
    return 'dead/' + dead_cat(r)


def classify(r):
    if r['dev']:
        return 'dev-page'
    if r['hidden']:
        return 'hidden(static)'
    if r.get('probe'):
        return classify_probe(r, CPP_LITS[0], CPP_LITS[1])
    if r['greyed']:
        return 'greyed(static)'
    if r['unwired'] and not r['refs']:
        return 'greyed(runtime-list)'
    golden = r['gtype'] != '(web-only)'
    # STATIC (provisional) -- prefixed "s:" so it is never mistaken for a measured result
    if golden and r['g_dfm'] and not r['g_onclick']:
        return 's:C0'                                         # golden has no OnClick: no reaction is the golden behaviour
    if golden and r['g_visible'] == 'False' and not any('Visible=' in s and 'false' not in s.lower() for s in r['g_vis_sites']):
        return 's:C'                                          # golden never shows it
    if golden and r['g_enabled'] == 'False' and not any('Enabled=' in s and 'false' not in s.lower() for s in r['g_vis_sites']):
        return 's:C'
    if r['cmds']:
        unknown = [c for c in r['cpp'] if c.endswith('=no')]
        return 's:B(web-sends-unknown-cmd)' if unknown else 's:bound-cmd'
    if r['refs'] or r['onclick']:
        if r['ui']:
            return 's:bound-ui'
        if all('JSON/js/' in x for x in r['refs']):
            return 's:bound-data'                             # listed in a data table (e.g. teach-access): the engine builds the command
        return 's:referenced'                                 # the id is used by a script; what a click does needs the probe
    return 's:unbound/' + dead_cat(r)


CPP_LITS = [set(), set()]


TSV_COLS = ['page', 'window', 'id', 'gtype', 'caption', 'sheet', 'html_line', 'cat', 'hidden', 'greyed', 'unwired_list', 'refs',
            'sent_static_guess', 'cpp_knows', 'formevent_row', 'port_handler', 'golden_dfm', 'golden_onclick', 'golden_handler',
            'golden_stmts', 'golden_danger', 'golden_visible', 'golden_enabled', 'golden_vis_sites', 'machine_s4', 'sent_probe']


def tsv_row(r):
    h = r['g_handler'] or {}
    return [r['page'], r['window'], r['id'], r['gtype'], r['caption'], r['sheet'], str(r['html_line']), r['cat'],
            '1' if r['hidden'] else '', '1' if r['greyed'] else '', r['unwired'],
            ' '.join(r['refs'][:6]) + (' +%d' % (len(r['refs']) - 6) if len(r['refs']) > 6 else ''),
            ' '.join(r['cmds'] + ['ui:' + u for u in r['ui']]), ' '.join(r['cpp']), '1' if r['fe_row'] else '',
            ' '.join(r['port_handler']), r['g_dfm'],
            r['g_onclick'], h.get('loc', ''), str(h.get('stmts', '')) if h else '', ' '.join(h.get('danger', [])) if h else '',
            r['g_visible'], r['g_enabled'], ' | '.join(r['g_vis_sites'][:4]) + (' +%d' % (len(r['g_vis_sites']) - 4) if len(r['g_vis_sites']) > 4 else ''),
            ('id' if r['sec4'] else '') + ('page' if r['sec4_page'] else ''), r.get('sent_probe', 'UNVERIFIED')]


def clean(s):
    return str(s).replace('\t', ' ').replace('\n', ' ')


def merge_probe(rows, path):
    """c12_click_probe.py output: page, id, state, sent, http, frame, dlg, dom, err, why -> r['probe'], re-classified."""
    got = {}
    for l in rd(path).split('\n')[1:]:
        c = l.rstrip('\r').split('\t')
        if len(c) >= 10:
            sp = lambda s: [x.strip() for x in s.split(' ; ') if x.strip()]
            got[(c[0], c[1])] = {'state': c[2], 'ws': sp(c[3]), 'http': sp(c[4]), 'frame': sp(c[5]), 'dlg': sp(c[6]),
                                 'dom': int(c[7]) if c[7].isdigit() else 0, 'err': sp(c[8]), 'why': c[9]}
    n = 0
    for r in rows:
        p = got.get((r['page'], r['id']))
        if p:
            r['probe'] = p
            parts = [p['state']] + p['ws'] + ['http:' + x for x in p['http']] + ['frame:' + x for x in p['frame']] + \
                    ['dlg:' + x for x in p['dlg']] + (['dom:%d' % p['dom']] if p['dom'] else []) + ['err:' + x for x in p['err']]
            r['sent_probe'] = ' ; '.join(parts)
            r['cat'] = classify(r)
            n += 1
    return n


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--golden', default=GOLDEN_DEFAULT)
    ap.add_argument('--out-tsv', default='')
    ap.add_argument('--out-md', default='')
    ap.add_argument('--merge', default='')
    a = ap.parse_args()
    rows = census(a.golden)
    if a.merge:
        merge_probe(rows, a.merge)
    if a.out_tsv:
        lines = ['\t'.join(TSV_COLS)] + ['\t'.join(clean(x) for x in tsv_row(r)) for r in rows]
        with open(a.out_tsv, 'wb') as f:
            f.write(('\n'.join(lines) + '\n').encode('utf-8'))
    cnt = collections.Counter(r['cat'] for r in rows)
    sys.stdout.buffer.write(('rows %d\n' % len(rows) + '\n'.join('%6d %s' % (v, k) for k, v in cnt.most_common()) + '\n').encode('utf-8'))
    if a.out_md:
        from c12_census_md import write_md      # report writer (same directory)
        write_md(rows, a.out_md, a.golden)


if __name__ == '__main__':
    main()
