# -*- coding: utf-8 -*-
"""tools/webprobe/e09_static_layout.py -- AI(W906-ST02-E09) 20261004 (St02-E): card E-09 static layout pass (python only --
no browser, no exe); the runtime half is e09_layout_probe.py (St01 runs it).
For every window in web/background.html's WINDOWS table:
  W  the window rect vs the screen canvas (work area / zoom) for a few screen sizes -> cut off by the screen?
  P  the page's root <div class="form" style="width;height"> vs the window client (w-2, h-26) -> content larger than the window?
  E  visible absolutely placed elements whose box (offsets accumulated through positioned ancestors) leaves the form root
  F  floating elements: position:fixed, or z-index >= 100, or a class / id that reads as a tip / toast / banner / overlay
  O  windows open at start (not hidden) that overlap each other
Usage: e09_static.py <repo root> <out.tsv> [zoom=1.10] [taskbar=48]
"""
import html.parser, io, os, re, sys

ROOT, OUT = sys.argv[1], sys.argv[2]
ZOOM = float(sys.argv[3]) if len(sys.argv) > 3 else 1.10
TASKBAR = int(sys.argv[4]) if len(sys.argv) > 4 else 48
SCREENS = [(1920, 1080), (1280, 1024), (1366, 768), (1024, 768)]
TAB, NL = chr(9), chr(10)

def rd(p):
    raw = io.open(p, 'rb').read()
    for enc in ('utf-8', 'cp950'):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            pass
    return raw.decode('utf-8', 'replace')

# ---------------------------------------------------------------- WINDOWS table
bg = rd(os.path.join(ROOT, 'web', 'background.html'))
start = bg.index('var WINDOWS=[')
end = bg.index('];', start)
WIN = []
for m in re.finditer(r'\{id:\'([^\']+)\'(.*?)\}', bg[start:end]):
    body = m.group(2)
    def num(k):
        mm = re.search(r'\b' + k + r':\s*(-?\d+)', body)
        return int(mm.group(1)) if mm else None
    src = re.search(r"src:'([^']*)'", body)
    w = dict(id=m.group(1), src=src.group(1) if src else '', x=num('x'), y=num('y'), w=num('w'), h=num('h'),
             hidden='hidden:true' in body, fixed='fixed:true' in body, locked='locked:true' in body,
             line=bg[:start + m.start()].count(NL) + 1)
    if None not in (w['x'], w['y'], w['w'], w['h']):
        WIN.append(w)

# ---------------------------------------------------------------- page parser
def style_of(attrs):
    d = {}
    for part in (attrs.get('style') or '').split(';'):
        if ':' in part:
            k, v = part.split(':', 1)
            d[k.strip().lower()] = v.strip().lower()
    return d

def px(v):
    mm = re.match(r'(-?\d+(?:\.\d+)?)px', v or '')
    return float(mm.group(1)) if mm else None

FLOAT_WORDS = re.compile(r'tip|toast|banner|overlay|float|hint|popup|bubble|badge|ribbon', re.I)
VOID = {'br', 'img', 'input', 'meta', 'link', 'hr', 'area', 'base', 'col', 'embed', 'source', 'track', 'wbr', 'param'}

class Page(html.parser.HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.stack = []          # (tag, abs_x, abs_y, positioned, hidden, in_scroll)
        self.form = None
        self.elems, self.floats = [], []
    def handle_starttag(self, tag, a):
        a = dict(a)
        st = style_of(a)
        par = self.stack[-1] if self.stack else ('', 0.0, 0.0, False, False, False)
        scroll = par[5] or bool(re.search(r'auto|scroll', st.get('overflow', '') + ' ' + st.get('overflow-y', '') + ' ' + st.get('overflow-x', '')))
        hidden = par[4] or st.get('display') == 'none' or st.get('visibility') == 'hidden' or 'hidden' in a
        cls = a.get('class') or ''
        if self.form is None and tag == 'div' and re.search(r'(^|\s)form(\s|$)', cls) and px(st.get('width')) is not None:
            self.form = (px(st.get('width')), px(st.get('height')) or 0.0, self.getpos()[0])
            node = (tag, 0.0, 0.0, True, hidden, False)
        else:
            pos = st.get('position', '')
            l, t = px(st.get('left')), px(st.get('top'))
            ax, ay = par[1], par[2]
            positioned = pos in ('absolute', 'relative', 'fixed')
            if pos == 'absolute' and l is not None and t is not None:
                ax, ay = par[1] + l, par[2] + t
                w, h = px(st.get('width')), px(st.get('height'))
                if self.form is not None and w is not None and h is not None and not hidden and not par[5]:
                    self.elems.append(dict(id=a.get('id', ''), tag=tag, x=ax, y=ay, w=w, h=h, line=self.getpos()[0],
                                           title=(a.get('title') or '')[:40]))
            zi = st.get('z-index', '')
            ident = (a.get('id') or '') + ' ' + cls
            if pos == 'fixed' or (zi.lstrip('-').isdigit() and int(zi) >= 100) or FLOAT_WORDS.search(ident):
                self.floats.append(dict(id=a.get('id', ''), cls=cls[:40], pos=pos, z=zi, line=self.getpos()[0], hidden=hidden))
            node = (tag, ax, ay, positioned, hidden, scroll)
        if tag not in VOID:
            self.stack.append(node)
    def handle_endtag(self, tag):
        for i in range(len(self.stack) - 1, -1, -1):
            if self.stack[i][0] == tag:
                del self.stack[i:]
                break

rows = []
def add(kind, win, what, detail, line=''):
    rows.append([kind, win['id'], win['src'], what, detail, str(line)])

# W: window vs screen
for win in WIN:
    for sw, sh in SCREENS:
        cw, ch = (sw) / ZOOM, (sh - TASKBAR) / ZOOM
        ox, oy = win['x'] + win['w'] - cw, win['y'] + win['h'] - ch
        if ox > 0.5 or oy > 0.5:
            add('W', win, 'screen %dx%d @%d%%' % (sw, sh, round(ZOOM * 100)),
                'window %d,%d %dx%d ends at %d,%d; canvas %dx%d -> cut %s%s' % (
                    win['x'], win['y'], win['w'], win['h'], win['x'] + win['w'], win['y'] + win['h'], cw, ch,
                    ('right %dpx ' % ox) if ox > 0.5 else '', ('bottom %dpx' % oy) if oy > 0.5 else ''), win['line'])

# P / E / F: pages
for win in WIN:
    if not win['src'].startswith('page/'):
        continue
    p = os.path.join(ROOT, 'web', win['src'].replace('/', os.sep))
    if not os.path.exists(p):
        add('P', win, 'page missing', p)
        continue
    pg = Page()
    try:
        pg.feed(rd(p))
    except Exception as e:
        add('P', win, 'parse error', str(e)[:80])
        continue
    if pg.form:
        fw, fh, fl = pg.form
        cw, ch = win['w'] - 2, win['h'] - 26
        if fw > cw + 0.5 or fh > ch + 0.5:
            add('P', win, 'form larger than window client',
                'form %dx%d vs client %dx%d (window %dx%d): %s%s' % (fw, fh, cw, ch, win['w'], win['h'],
                ('wider by %d ' % (fw - cw)) if fw > cw + 0.5 else '', ('taller by %d' % (fh - ch)) if fh > ch + 0.5 else ''), fl)
        for e in pg.elems:
            ox, oy = e['x'] + e['w'] - fw, e['y'] + e['h'] - fh
            if ox > 1 or oy > 1 or e['x'] < -1 or e['y'] < -1:
                add('E', win, 'element outside the form', '%s <%s> at %d,%d %dx%d; form %dx%d -> %s%s%s' % (
                    e['id'] or '(no id)', e['tag'], e['x'], e['y'], e['w'], e['h'], fw, fh,
                    ('right +%d ' % ox) if ox > 1 else '', ('bottom +%d ' % oy) if oy > 1 else '',
                    'negative origin' if (e['x'] < -1 or e['y'] < -1) else ''), e['line'])
    else:
        add('P', win, 'no <div class="form"> root', 'generated geometry not found (hand-made page?)')
    for f in pg.floats:
        if not f['hidden']:
            add('F', win, 'floating element', '%s class="%s" position:%s z-index:%s' % (f['id'] or '(no id)', f['cls'], f['pos'] or '-', f['z'] or '-'), f['line'])

# O: start-up windows overlapping each other
vis = [w for w in WIN if not w['hidden']]
for i in range(len(vis)):
    for j in range(i + 1, len(vis)):
        a, b = vis[i], vis[j]
        ix = min(a['x'] + a['w'], b['x'] + b['w']) - max(a['x'], b['x'])
        iy = min(a['y'] + a['h'], b['y'] + b['h']) - max(a['y'], b['y'])
        if ix > 0 and iy > 0:
            add('O', a, 'overlaps window %s' % b['id'], 'overlap %dx%d (%s %d,%d %dx%d / %s %d,%d %dx%d)' % (
                ix, iy, a['id'], a['x'], a['y'], a['w'], a['h'], b['id'], b['x'], b['y'], b['w'], b['h']), a['line'])

hdr = ['kind', 'window', 'page', 'what', 'detail', 'line']
io.open(OUT, 'w', encoding='utf-8', newline=NL).write(TAB.join(hdr) + NL + NL.join(TAB.join(r) for r in rows) + NL)
from collections import Counter
c = Counter(r[0] for r in rows)
print('windows %d (start-up %d); rows %d: %s' % (len(WIN), len(vis), len(rows), dict(sorted(c.items()))))
