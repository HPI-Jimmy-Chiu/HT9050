# -*- coding: utf-8 -*-
# =============================================================================
#  tools/webprobe/teach_kb_e2e_probe.py -- Teach page numeric keypad + read / write, end to end in a real browser.
#
#  AI(W906-TEACH-KB-E2E) 20261004: new file. Jimmy 1004 14:1x: a Teach field's numeric input went wrong on the machine --
#    "confirm it and truly verify numeric input and read/write". w5_teach_probe.py proves editlist.get / editlist.save at the
#    WebSocket level against the real teach.ini; this probe drives the SAME page an operator uses: headless Edge opens
#    /page/HW.teach.html on a real wb_serve, the keypad (web/page/qwerty.js, bound by ht9045_wire_engine.js attachKeyboards
#    to the field's mousedown) is driven with trusted mouse events (CDP Input.dispatchMouseEvent; --clicks dom = synthetic
#    dispatchEvent / click()), and the page's own Save button and its "Sure to Save?" confirm box are used.
#
#  Golden reference: myQwertyKeyBoard.cpp (V912 / 906 tree, same code)
#    * FormShow focuses edQwertyContent (TEdit AutoSelect) -> the old value is selected; spbKeyClick: SelLength>0 -> Text=""
#      first, so the FIRST digit REPLACES the old value.                 The pre-fix web keypad appends.
#    * spbMinusClick toggles a leading '-'.                              The pre-fix web appends '-'.
#    * spbPercentClick only cycles iDecimalPoint -> ChangeDecimalPoint relabels the six step buttons; the text is untouched.
#                                                                        The pre-fix web divides the value by 100.
#    * dp 0 step captions are +10/+100/+1000 and -10/-100/-1000;        the pre-fix web shows +1/+10/+100.
#    * spbAdd1Click adds atof(Caption); OK with bCheckRange -> CheckRange(atof(Text), min, max) as AnsiString(double).
#
#  Sequences (in this order on each picked field; the keypad is opened by a mousedown on the field each time; expected
#  values come from two models, LegacyPad = the pre-fix qwerty.js, GoldenPad = myQwertyKeyBoard.cpp, both printed):
#    S1  open on v, 1 2 3, OK          legacy v+"123"        golden "123"
#    S2  open, Del, 4 5 6, -, OK       legacy "456-"         golden "-456"
#    S3  open, %, OK                   legacy v/100 (rounded) golden v unchanged (the step captions change: checked too)
#    S4  open, Del, -, 7 8 9, OK       both "-789"           (the documented workaround)
#    S5  open, read the six step captions, press the smallest '+' step, OK      (TECH_PARA field only)
#    After each: the keypad closed, input / change reached the field, data-src = "typed" on a teach-point edit
#    (HW.teach.html AI(W906-TEACH-TYPED): its Go button then sends what was typed), else unchanged.
#    Fields (visible, enabled, not covered; a tab is clicked open when needed): a TECH_PARA int field, an elTeach int field
#    and a field with a wire range (the OK clamp applies). Fields saved to ONE teach.ini key are preferred -- golden binds
#    some TEdits to TECH_PARA and elTeach at once (pick_fields) -- and for the elTeach one, one without a C++ range.
#  Save (both modes): reload the page, type A = -789 (TECH_PARA), B = 4321 (elTeach; a value inside B's C++ range when 4321
#    is not, e.g. 321) and, when the page has one, C = above the C++ range of another elTeach field, all with the S4
#    workaround; press the page's Save, answer YES in its confirm box (window.confirm, answered over DevTools), wait for the
#    ack and the engine's re-read, then
#      (a) teach.ini: every key A / B is saved to holds the typed value    (b) the reopened page shows them (data-src=cpp)
#      (c) a fresh WS editlist.get Teach returns them
#      (d) Gerneral.ini changed only [Shuttle] iInShtZRange (golden atoi('') quirk, docs/W5_PROGRESS.md section 4-5)
#      (e) no other teach.ini key changed value
#      (f) C is NOT written and comes back unchanged: HTEditList's save skips an out-of-range value and golden says nothing
#          (cxx_el_ranges) -- the same as golden; the page itself does not know these ranges.
#
#  Real files: unless --attach, the probe runs `realfile_guard.py snap <tag>` BEFORE it starts wb_serve (retrying while a
#    ctest.exe runs -- never --ignore-ctest) and, after stopping wb_serve (normal close via its quit event), runs check ->
#    restore -> check (must be 0 changed) -> drop. --keep skips restore / drop (the backup stays for you). wb_serve also
#    writes its dialog mailbox under <web root>\JSON\runtime (gitignored in this tree).
#  Usage (from the tree root):
#    python HT9011UC_Cpp_V3.33.906.0/tools/webprobe/teach_kb_e2e_probe.py --expect legacy --web <export of the pre-fix web>
#    python HT9011UC_Cpp_V3.33.906.0/tools/webprobe/teach_kb_e2e_probe.py --expect golden     (after the qwerty.js fix)
#  Not a ctest (needs Edge, a real wb_serve and the real machine files). Exit 0 = every expectation of --expect met.
# =============================================================================
import argparse, ctypes, json, math, os, re, shutil, socket, subprocess, sys, time, urllib.error, urllib.request

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True   # AI(W906-KB-GOLDEN) 20261004 review: the imports below must not leave __pycache__ in the repo
from s12_form_probe import Cdp, launch_edge                       # noqa: E402  headless Edge + DevTools (also sets stdout utf-8)
from w4_motor_probe import Conn, check, FAILS, FrameReader         # noqa: E402  WS acks, PASS/FAIL lines
from w5_teach_probe import read_ini, entries, TEACH_INI, Q_SAVE    # noqa: E402  teach.ini reader, editlist list access
from cmd_probe import ws_handshake, send_text                      # noqa: E402

TREE = os.path.normpath(os.path.join(HERE, '..', '..', '..'))
GUARD = os.path.normpath(os.path.join(HERE, '..', 'realfile_guard.py'))
GERNERAL_INI = r'D:\HT9045\system\Gerneral.ini'
DEFAULT_WB = os.path.join(TREE, 'Obj', 'V906', 'build_sim', 'wb_serve.exe')   # review: THIS tree's sim build (build.bat / the gate), not another worktree's
PAGE = 'HW.teach.html'
QUIRK = {('shuttle', 'iinshtzrange')}            # Gerneral.ini keys a Teach save may change (golden btnSaveClick :2280)
STEP_RE = re.compile(r'^[+-](\d+(\.\d*)?|\.\d+)$')
INFO = []                                         # observations printed at the end


def note(msg):
    INFO.append(msg)
    print('  NOTE: ' + msg)


# ----------------------------------------------------------------------------- keypad models (expected values)
_NUM = re.compile(r'^\s*([+-]?(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?)')


def js_parse_float(s):
    """JS parseFloat: the leading decimal literal, else NaN (None)."""
    m = _NUM.match(s or '')
    return float(m.group(1)) if m else None


def c_atof(s):
    """C atof: same prefix rule, 0.0 when nothing converts."""
    v = js_parse_float(s)
    return 0.0 if v is None else v


def js_round(x):
    return math.floor(x + 0.5)                    # Math.round: half up (toward +inf), unlike Python round()


def num_text(x):
    """JS String(number) / VCL AnsiString(double) for the integral values these fields take."""
    x = float(x)
    return str(int(x)) if x == int(x) else repr(x)


class LegacyPad(object):
    """web/page/qwerty.js as committed before the golden fix (buildNumpad / put / adj / pct / commit)."""
    name = 'legacy'

    def __init__(self, text, rng):
        self.val, self.rng = str(text).strip(), rng

    def out(self):
        return self.val

    def captions(self):
        return sorted(['+1', '+10', '+100', '-1', '-10', '-100'])   # fixed; '%' does not relabel them

    def key(self, k):
        if k.isdigit():
            self.val += k
        elif k == 'Del':
            self.val = ''
        elif k == '-':
            self.val += '-'                                          # put('-')
        elif k == '%':
            self.val = num_text(js_round((js_parse_float(self.val) or 0.0) / 100))   # pct(), INTEGER
        elif STEP_RE.match(k):
            self.val = num_text(js_round((js_parse_float(self.val) or 0.0) + float(k)))  # adj(n)
        elif k == 'OK' and self.rng:
            v = js_parse_float(self.val) or 0.0                      # commit(): parseFloat || 0, clamp, Math.round
            self.val = num_text(js_round(min(max(v, min(self.rng)), max(self.rng))))
        return self.val


class GoldenPad(object):
    """myQwertyKeyBoard.cpp: ShowQwertyKey / spbKeyClick / spbMinusClick / spbPercentClick / spbAdd1Click / OK."""
    name = 'golden'
    CAPS = {0: ['+10', '+100', '+1000', '-10', '-100', '-1000'], 1: ['+1', '+10', '+100', '-1', '-10', '-100'],
            2: ['+1.0', '+0.1', '+0.01', '-1.0', '-0.1', '-0.01'], 3: ['+0.1', '+0.01', '+0.001', '-0.1', '-0.01', '-0.001']}

    def __init__(self, text, rng, dp=0):
        self.text, self.rng, self.dp = str(text), rng, dp
        self.sel = len(self.text) > 0                                # FormShow SetFocus: TEdit AutoSelect selects the old value
        self._fix()

    def _fix(self):
        if self.dp > 1:                                              # ChangeDecimalPoint: bIntegerOnly && iDecimalPoint>1 -> 0
            self.dp = 0

    def out(self):
        return self.text

    def captions(self):
        return sorted(self.CAPS[self.dp])

    def key(self, k):
        if k.isdigit():
            if self.sel:
                self.text = ''                                       # SelLength>0 -> Text=""
            self.text += k
            self.sel = False                                         # SelStart=Length()
        elif k == 'Del':
            self.text, self.sel = '', False
        elif k == '-':
            self.text = self.text[1:] if self.text.startswith('-') else '-' + self.text
            self.sel = False
        elif k == '%':
            self.dp = 0 if self.dp + 1 > 3 else self.dp + 1          # text untouched, selection kept (TSpeedButton takes no focus)
            self._fix()
        elif STEP_RE.match(k):
            self.text, self.sel = str(int(c_atof(self.text) + float(k))), False   # bIntegerOnly: AnsiString(int(dResult))
        elif k == 'OK' and self.rng:
            d = c_atof(self.text)
            self.text = num_text(min(max(d, min(self.rng)), max(self.rng)))      # CheckRange (either argument order)
        return self.text


MODELS = {'legacy': LegacyPad, 'golden': GoldenPad}


# ----------------------------------------------------------------------------- real-file guard / server / browser plumbing
def port_open(port):
    try:
        socket.create_connection(('127.0.0.1', port), timeout=1.0).close()
        return True
    except OSError:
        return False


def guard(cmd, tag):
    env = dict(os.environ, PYTHONIOENCODING='utf-8:replace')
    r = subprocess.run([sys.executable, GUARD, cmd, tag], capture_output=True, env=env)
    out = (r.stdout + r.stderr).decode('utf-8', 'replace')
    print('  [realfile_guard %s %s] exit %d' % (cmd, tag, r.returncode))
    for ln in out.splitlines():                                      # binary .dat diffs come out as one huge "line": cut them
        ln = re.sub(r'[\x00-\x08\x0b-\x1f]', '.', ln).rstrip()
        print('    | ' + (ln[:200] + ' ...(%d chars)' % len(ln) if len(ln) > 200 else ln))
    return r.returncode, out


def guard_snap(tag, wait_s):
    t0 = time.monotonic()
    while True:
        rc, _ = guard('snap', tag)
        if rc != 3 or time.monotonic() - t0 > wait_s:
            return rc
        print('  a ctest.exe is running (another gate shares the real files) -- retrying the snap in 30 s (no --ignore-ctest)')
        time.sleep(30)


class Server(object):
    """One wb_serve on --port. Started with --seconds (no browser auto-open, ends by itself if the probe dies);
    stopped with its own quit event (normal close: the same path as --seconds expiring), killed only as a fallback."""

    def __init__(self, exe, web, port, seconds, rundir):
        self.exe, self.web, self.port, self.seconds, self.rundir = exe, web, port, seconds, rundir
        self.p, self.log = None, os.path.join(rundir, 'wb_serve_%d.log' % port)

    def start(self):
        env = dict(os.environ, W906_NO_BROWSER_WAKE='1')
        self.logf = open(self.log, 'wb')
        self.p = subprocess.Popen([self.exe, '--root', self.web, '--port', str(self.port), '--seconds', str(self.seconds)],
                                  cwd=self.rundir, stdout=self.logf, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                                  env=env, creationflags=subprocess.CREATE_NEW_PROCESS_GROUP)
        print('  wb_serve pid %d, log %s' % (self.p.pid, self.log))

    def wait_ready(self, timeout):
        t0 = time.monotonic()
        base = 'http://127.0.0.1:%d' % self.port
        while time.monotonic() - t0 < timeout:
            if self.p.poll() is not None:
                return 'wb_serve exited with %s' % self.p.returncode
            try:
                urllib.request.urlopen(base + '/page/' + PAGE, timeout=3).read()
                try:                                                 # C++ answers /api/form (TeachFormShow_File): the server side is up
                    urllib.request.urlopen(base + '/api/form/' + PAGE, timeout=5).read()
                except urllib.error.HTTPError:
                    pass                                             # an answer all the same (an older build without the endpoint)
                print('  wb_serve ready after %.1f s' % (time.monotonic() - t0))
                return ''
            except Exception:
                time.sleep(0.5)
        return 'not ready within %d s' % timeout

    def stop(self, wait_s=45):
        if not self.p or self.p.poll() is not None:
            return
        k32 = ctypes.windll.kernel32
        k32.OpenEventW.restype = ctypes.c_void_p
        k32.OpenEventW.argtypes = [ctypes.c_uint32, ctypes.c_int, ctypes.c_wchar_p]
        k32.SetEvent.argtypes = [ctypes.c_void_p]
        k32.CloseHandle.argtypes = [ctypes.c_void_p]
        h = k32.OpenEventW(0x0002, 0, 'Local\\HT9045_wb_serve_quit_%d' % self.p.pid)   # EVENT_MODIFY_STATE
        if h:
            k32.SetEvent(h)
            k32.CloseHandle(h)
            print('  wb_serve: normal close requested (quit event)')
        try:
            self.p.wait(wait_s)
            print('  wb_serve exited %s' % self.p.returncode)
        except subprocess.TimeoutExpired:
            print('  wb_serve did not end within %d s -> taskkill' % wait_s)
            subprocess.run(['taskkill', '/PID', str(self.p.pid), '/T', '/F'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            self.p.wait(10)
        self.logf.close()


class TeachCdp(Cdp):
    """s12_form_probe.Cdp with a buffered frame reader (w4 FrameReader: a deadline per call, nothing lost between calls),
    DevTools events handled while a call waits -- a JS confirm() box is recorded and answered (Page.handleJavaScriptDialog)
    -- and trusted mouse clicks."""

    def __init__(self, ws_url):                                      # replaces Cdp.__init__ (its generator has no per-call deadline)
        hostport, path = ws_url[len('ws://'):].split('/', 1)
        host, port = hostport.split(':')
        self.sock, left = ws_handshake(host, int(port), '/' + path, time.monotonic() + 10)
        self.rd = FrameReader(self.sock, left)
        self.nid = 0
        self.dialogs, self.errors = [], []
        self.accept = True

    def _event(self, m):
        meth, p = m.get('method'), m.get('params') or {}
        if meth == 'Page.javascriptDialogOpening':
            self.dialogs.append({'type': p.get('type'), 'message': p.get('message', ''), 'accepted': self.accept})
            self.nid += 1
            send_text(self.sock, json.dumps({'id': self.nid, 'method': 'Page.handleJavaScriptDialog',
                                             'params': {'accept': self.accept}}))
        elif meth == 'Runtime.exceptionThrown':
            d = p.get('exceptionDetails') or {}
            self.errors.append(((d.get('exception') or {}).get('description') or d.get('text') or '?')[:200])

    def call(self, method, params=None, timeout=20):
        self.nid += 1
        my = self.nid
        send_text(self.sock, json.dumps({'id': my, 'method': method, 'params': params or {}}))
        end = time.monotonic() + timeout
        while True:
            t = self.rd.next_text(end)
            if t is None:
                raise RuntimeError('CDP %s: no reply within %s s' % (method, timeout))
            m = json.loads(t)
            if m.get('id') == my:
                return m
            if 'method' in m:
                self._event(m)

    def click(self, x, y):
        for typ, extra in (('mouseMoved', {'button': 'none'}),
                           ('mousePressed', {'button': 'left', 'buttons': 1, 'clickCount': 1}),
                           ('mouseReleased', {'button': 'left', 'buttons': 0, 'clickCount': 1})):
            d = {'type': typ, 'x': x, 'y': y}
            d.update(extra)
            self.call('Input.dispatchMouseEvent', d, timeout=30)


# ----------------------------------------------------------------------------- page side
# Installed after every page load (window.__tkp). Nothing here changes the page except tab clicks, scrolling and the
# event counters on the fields the probe types into.
PAGE_JS = r"""
(function () {
  if (window.__tkp) return 'present';
  var T = window.__tkp = { ev: {} };
  function hider(el) {                        // the element (self or ancestor) that hides el, or null
    for (var p = el; p && p.nodeType === 1; p = p.parentElement) {
      var cs = getComputedStyle(p);
      if (cs.display === 'none' || cs.visibility === 'hidden') return p;
    }
    return null;
  }
  function vis(el) { return !!el && el.getClientRects().length > 0 && !hider(el); }
  function tabsOf(el) {                       // the PageControl tab headers that must be active for el, outermost first
    var out = [];
    for (var p = el.parentElement; p; p = p.parentElement) {
      if (!p.classList || !p.classList.contains('pcPane')) continue;
      var wrap = p.parentElement && p.parentElement.parentElement, tab = null;
      if (wrap && wrap.classList.contains('pcWrap')) {
        var tabs = wrap.querySelector(':scope > .pcTabs');
        if (tabs) tab = tabs.querySelector(':scope > .tab[data-t="' + p.getAttribute('data-p') + '"]');
      }
      out.unshift({ tab: tab, pane: p });
    }
    return out;
  }
  function name(t) { return (t.getAttribute('title') || t.getAttribute('data-htitle') || t.textContent || '').split(' :')[0].trim(); }
  function center(el) { var r = el.getBoundingClientRect(); return { x: r.left + r.width / 2, y: r.top + r.height / 2, w: r.width, h: r.height }; }
  function hitIs(el, c) { var h = document.elementFromPoint(c.x, c.y); return { ok: !!h && (h === el || el.contains(h)), what: h ? (h.id || String(h.className) || h.tagName) : null }; }
  T.reach = function (id, activate) {
    var el = document.getElementById(id);
    if (!el) return { id: id, missing: true };
    var chain = tabsOf(el), why = '';
    chain.forEach(function (c) {
      if (why) return;
      if (!c.tab) { why = 'no tab header for pane ' + c.pane.getAttribute('title'); return; }
      if (!vis(c.tab) || c.tab.classList.contains('rule-dim') || c.tab.classList.contains('rule-hide') ||
          getComputedStyle(c.tab).pointerEvents === 'none') { why = 'tab ' + name(c.tab) + ' hidden / dimmed'; return; }
      if (activate && !c.tab.classList.contains('act')) c.tab.click();
    });
    if (activate && !why) el.scrollIntoView({ block: 'center', inline: 'center' });
    var c = center(el), h = hitIs(el, c), hd = hider(el);
    if (!why && hd) why = 'hidden by ' + (hd === el ? 'itself' : (hd.id || hd.getAttribute('title') || hd.tagName)) +
                          (String(hd.className || '') ? ' (' + hd.className + ')' : '');
    return { id: id, visible: vis(el), disabled: !!el.disabled, ariaDisabled: !!(el.closest && el.closest('[aria-disabled="true"]')),
             readOnly: !!el.readOnly, src: el.getAttribute('data-src'), value: el.value, x: c.x, y: c.y, hitOk: h.ok, hit: h.what,
             why: why, tabs: chain.map(function (c) { return c.tab ? name(c.tab) : '?'; }) };
  };
  T.track = function (id) {
    var el = document.getElementById(id);
    if (!T.ev[id]) {
      T.ev[id] = { input: 0, change: 0 };
      el.addEventListener('input', function () { T.ev[id].input++; });
      el.addEventListener('change', function () { T.ev[id].change++; });
    }
    T.ev[id].input = 0; T.ev[id].change = 0;
    return true;
  };
  T.field = function (id) {
    var el = document.getElementById(id);
    return { value: el.value, src: el.getAttribute('data-src'), typedHook: el.getAttribute('data-typed-hook') === '1',
             ev: T.ev[id] || null, kbOpen: !!document.querySelector('.qkOv') };
  };
  function keys() { var ov = document.querySelector('.qkOv'); return ov ? Array.prototype.slice.call(ov.querySelectorAll('button')) : []; }
  function lab(b) { return (b.textContent || '').trim(); }
  T.kb = function () {
    var ov = document.querySelector('.qkOv');
    if (!ov) return { open: false };
    var d = ov.querySelector('.qkDisp'), ae = document.activeElement;
    var steps = keys().filter(function (b) { return vis(b) && !b.disabled && /^[+-](\d|\.)/.test(lab(b)); }).map(lab);
    return { open: true, disp: d ? d.value : null, dispSel: d ? [d.selectionStart, d.selectionEnd] : null, dispClass: d ? d.className : null,
             active: ae ? (ae.tagName + (ae.className ? '.' + String(ae.className).split(' ').join('.') : '')) : null,
             steps: steps, labels: keys().filter(vis).map(lab) };
  };
  T.key = function (label, domClick) {        // a visible keypad button by its label; domClick -> b.click() here
    var all = keys().filter(function (b) { return lab(b) === label && vis(b); });
    if (!all.length) return { err: 'no visible keypad button labelled "' + label + '"' };
    var b = all.filter(function (x) { return !x.disabled; })[0] || all[0];
    var c = center(b), h = hitIs(b, c);
    if (domClick && !b.disabled) b.click();
    return { x: c.x, y: c.y, disabled: !!b.disabled, hitOk: h.ok, hit: h.what, n: all.length };
  };
  T.domDown = function (id) {                  // synthetic mousedown, what ht9045_wire_engine.js attachKeyboards listens to
    var el = document.getElementById(id);
    ['pointerdown', 'mousedown'].forEach(function (k) {
      el.dispatchEvent(new MouseEvent(k, { bubbles: true, cancelable: true, view: window, button: 0 }));
    });
    return !!document.querySelector('.qkOv');
  };
  return 'installed';
})()
"""

WAIT_LOAD_JS = r"""
(async function () {
  for (var i = 0; i < 600; i++) {
    var P = window.HT9045Page, g = P && P.golden && P.golden();
    var bar = document.getElementById('ht9045WireBar'), bt = bar ? bar.textContent : '';
    if (g && g.page) return { ok: true, struct: g.struct, cpp: document.querySelectorAll('[data-src="cpp"]').length, ms: i * 100 };
    if (/C 路讀取失敗|reload page|control-held/.test(bt)) return { ok: false, error: bt.slice(0, 400) };
    await new Promise(function (r) { setTimeout(r, 100); });
  }
  var b2 = document.getElementById('ht9045WireBar');
  return { ok: false, error: 'no C-route answer in 60 s', bar: b2 ? b2.textContent.slice(0, 400) : '' };
})()
"""

# What the probe needs from the C-route answer: every list entry with an id, plus the page's keypad table.
FIELDS_JS = r"""
(function () {
  var g = HT9045Page.golden(), lists = (g.page && g.page.lists) || {}, kb = (HT9045Page.cfg && HT9045Page.cfg.kb) || {}, out = [];
  Object.keys(lists).forEach(function (ln) {
    ((lists[ln] || {}).entries || []).forEach(function (e) {
      if (!e.id) return;
      var el = document.getElementById(e.id), o = {};
      Object.keys(e).forEach(function (k) { if (typeof e[k] !== 'object') o[k] = e[k]; });
      o.list = ln; o.kb = kb[e.id] || null; o.dom = !!el; o.src = el ? el.getAttribute('data-src') : null;
      o.domValue = el && 'value' in el ? el.value : null;
      out.push(o);
    });
  });
  return { entries: out, kbBound: (window.HT9045KbAudit || {}).bound, kbUnmapped: ((window.HT9045KbAudit || {}).unmapped || []).length };
})()
"""


class Page(object):
    def __init__(self, cdp, base, clicks):
        self.cdp, self.base, self.clicks = cdp, base, clicks

    def ev(self, expr, timeout=20):
        return self.cdp.eval(expr, timeout)

    def load(self):
        self.cdp.call('Page.navigate', {'url': self.base + '/page/' + PAGE})
        for _ in range(120):
            try:
                if self.ev('document.readyState') == 'complete':
                    break
            except RuntimeError:
                pass
            time.sleep(0.25)
        r = self.ev(WAIT_LOAD_JS, timeout=75) or {}
        if r.get('ok'):
            time.sleep(1.5)                                          # FormShow supplement (/api/form) and the other page scripts
            self.ev(PAGE_JS)
        return r

    def js(self, fn, *args):
        return self.ev('window.__tkp.%s(%s)' % (fn, ', '.join(json.dumps(a) for a in args)))

    def open_kb(self, fid):
        self.js('track', fid)
        r = self.js('reach', fid, True)
        if self.clicks == 'cdp':
            self.cdp.click(r['x'], r['y'])
        else:
            self.js('domDown', fid)
        time.sleep(0.05)
        return self.js('kb')

    def press(self, label):
        r = self.js('key', label, self.clicks == 'dom')
        if 'err' in r:
            return r
        if r.get('disabled'):
            r['err'] = 'button "%s" is disabled' % label
            return r
        if self.clicks == 'cdp':
            if not r.get('hitOk'):
                r['err'] = 'button "%s" is covered by %s' % (label, r.get('hit'))
                return r
            self.cdp.click(r['x'], r['y'])
        time.sleep(0.03)
        return r


# ----------------------------------------------------------------------------- field choice
PREFER = {'para': ['setEditOutRG', 'SetEditAuto1Front'], 'el': ['SetEditAutoClean'],
          'range': ['setEditInZSafeHeight', 'setEditOutZSafeHeight', 'edShtCheckRange', 'InSHZDownRange'], 'elrange': []}
INT_RE = re.compile(r'^-?\d+$')
SHUTTLE_GROUP = re.compile(r'^MIn(Shutte|Shuttle)[12]$')          # two spellings in teach.ini (w5_teach_probe.py, TECH_PARA::ReadFromFile)
GEN_INC = os.path.normpath(os.path.join(HERE, '..', '..', 'FileRW', 'Teach.gen.inc'))
_EL_ADD = re.compile(r'^\s*elTeach->Add\(filerw::EL<\w+>\("\w+",\s*"(\w+)"\)\s*,([^)]*)\);')


def cxx_el_ranges():
    """widget id -> (lo, hi) of its elTeach HTEditList range, or None (no range); None overall if the file is unreadable.
    The C-route list does not carry these, and the save enforces them: HTEditList::SaveEditTextToFile skips a key whose value
    is outside [Min, Max] (Public/HTEditList.cpp, golden HTEditList.cpp :574-1007) and golden TfTeach::SaveFile ignores the
    false it returns (uteach.cpp:4958); bAlarmLimitation is never set, so nothing is said -- the field just reverts.
    Args after the control: &var, Content, group, key, bShow, bEnable, bReadFromFile, DefValue, bDisableEventOverlap
    [, Min, Max]; HTEditList::Add swaps an inverted pair (e.g. "4000, 0" -> 0..4000)."""
    try:
        lines = open(GEN_INC, encoding='utf-8', errors='replace').read().splitlines()
    except OSError:
        return None
    out = {}
    for ln in lines:
        m = _EL_ADD.match(ln)
        if not m:
            continue
        args = [x.strip() for x in m.group(2).split(',')]
        try:
            out[m.group(1)] = (min(float(args[9]), float(args[10])), max(float(args[9]), float(args[10]))) if len(args) >= 11 else None
        except ValueError:
            out[m.group(1)] = None
    return out


def pick_fields(page, overrides, ini, ranges):
    """ini = teach.ini now. Each pick carries every teach.ini key its TEdit is saved to (binds): the C-route list row, plus
    the TECH_PARA twin the list leaves out -- FileRW/Teach.cpp FileRW_Teach_Page skips a TECH_PARA row whose widget is in
    elTeach ("elTeach 那一份清單會帶"), but golden SaveFile still writes both (setEdLoadCellZ1: uteach.cpp:402 TechPara
    [MTestZ1] setEdLoadCellZ1 and :3149 elTeach [Index] iLoadCellZ1Down, one variable). A TECH_PARA key is the widget's
    name (FileRW/Teach.gen.inc key->widget table: 1 of 361 differs, editsetEditZ3A), so the twin is any teach.ini key
    spelled like the widget id."""
    info = page.ev(FIELDS_JS)
    ents = info['entries']
    print('  C-route lists: %s; keypads bound %s (%s without a golden flag)' % (
        ', '.join('%s %d' % (ln, sum(1 for e in ents if e['list'] == ln)) for ln in sorted(set(e['list'] for e in ents))),
        info.get('kbBound'), info.get('kbUnmapped')))
    lists_of = {}
    for e in ents:
        lists_of.setdefault(e['id'], set()).add(e['list'])
    want_list = {'para': 'techPara', 'el': 'elTeach', 'elrange': 'elTeach'}
    ranges = ranges or {}

    def binds_of(fid):
        b = set((x['list'], x['group'], x['key']) for x in ents if x['id'] == fid and x.get('group') and x.get('key'))
        known = set((g.lower(), k.lower()) for _, g, k in b)
        for sec, kv in ini.items():
            if fid.lower() in kv and (sec, fid.lower()) not in known:
                b.add(('TECH_PARA twin (not in the C-route list)', sec, fid))
        return sorted(b)

    def entry_for(fid, kind):                                        # the entry of the list this kind is about
        es = [e for e in ents if e['id'] == fid]
        return ([e for e in es if e['list'] == want_list.get(kind)] or es or [None])[0]

    def ok_kind(e, kind):
        kb = e.get('kb')
        if not e['dom'] or not kb or kb[0] != 'INTEGER' or e.get('src') != 'cpp' or not INT_RE.match(str(e.get('domValue') or '')):
            return False
        if not e.get('group') or not e.get('key') or SHUTTLE_GROUP.match(e['group']):
            return False                                             # the teach.ini checks need one [group] key spelling
        if kind == 'elrange':                                        # (f): a C++ range the page does not know about, one key
            return e['list'] == 'elTeach' and not kb[2] and bool(ranges.get(e['id'])) and len(binds_of(e['id'])) == 1
        if kind in want_list:
            return e['list'] == want_list[kind] and not kb[2]
        return bool(kb[2])

    out, skipped = {}, []
    for kind in ('para', 'el', 'range', 'elrange'):
        okk = [e['id'] for e in ents if ok_kind(e, kind)]
        # golden binds some TEdits to TECH_PARA and elTeach at once (see the docstring): prefer a field saved to one teach.ini
        #   key only, so each list's write path is seen on its own; a two-key field is the fallback. For the elTeach pick also
        #   prefer one without a C++ range (cxx_el_ranges), so the typed value is not silently refused.
        order = ([overrides[kind]] if kind in overrides else []) + PREFER[kind] + \
                sorted(okk, key=lambda i: (len(binds_of(i)) > 1, bool(ranges.get(i))))
        seen = set(f['id'] for f in out.values())                    # never the same field twice
        for fid in order:
            if fid in seen or fid not in lists_of:
                continue
            seen.add(fid)
            e = entry_for(fid, kind)
            if not ok_kind(e, kind):
                skipped.append('%s %s (list %s, kb %s, src %s, value %r)' % (kind, fid, e['list'], e.get('kb'), e.get('src'), e.get('domValue')))
                continue
            r = page.js('reach', fid, True)
            if not r.get('visible') or r.get('disabled') or r.get('ariaDisabled') or not r.get('hitOk') or r.get('why'):
                skipped.append('%s %s (visible %s, disabled %s/%s, hit %s, %s)' % (kind, fid, r.get('visible'), r.get('disabled'),
                                                                                r.get('ariaDisabled'), r.get('hit'), r.get('why')))
                continue
            kb = e['kb']
            out[kind] = {'id': fid, 'list': e['list'], 'group': e.get('group'), 'key': e.get('key'), 'text': e.get('text'),
                         'value': r['value'], 'rng': (kb[3], kb[4]) if kb[2] else None, 'dp': kb[1], 'tabs': r['tabs'],
                         'binds': binds_of(fid), 'cxx': ranges.get(fid)}
            break
    for s in skipped[:12]:
        print('    skipped %s' % s)
    if len(skipped) > 12:
        print('    ... %d more skipped' % (len(skipped) - 12))
    for kind in ('para', 'el', 'range', 'elrange'):
        f = out.get(kind)
        if f:
            print('  field %-7s %-24s = %-8r tabs %-10s%s%s  teach.ini %s' % (
                kind, f['id'], f['value'], ' > '.join(f['tabs']), ('  wire range %s..%s' % f['rng']) if f['rng'] else '',
                ('  C++ elTeach range %g..%g' % f['cxx']) if f['cxx'] else '', '; '.join('%s [%s] %s' % b for b in f['binds'])))
    return out


def in_range_value(f, wanted, avoid):
    """wanted if the field's C++ elTeach range takes it, else a distinctive value inside the range (never `avoid`)."""
    lo, hi = f['cxx'] if f.get('cxx') else (-1e18, 1e18)
    for c in [wanted, '321', '21', '7', '-321', '-21', '-7', num_text(lo + 1), num_text(hi - 1)]:
        if lo <= float(c) <= hi and c != avoid:
            return c
    return wanted


# ----------------------------------------------------------------------------- the keypad sequences
SEQS = [('S1', ['1', '2', '3', 'OK'], 'open on v, 1 2 3, OK'),
        ('S2', ['Del', '4', '5', '6', '-', 'OK'], 'open, Del, 4 5 6, -, OK'),
        ('S3', ['%', 'OK'], 'open, %, OK'),
        ('S4', ['Del', '-', '7', '8', '9', 'OK'], 'open, Del, -, 7 8 9, OK (workaround)')]
RESULTS = []


def run_seq(page, mode, f, name, keys, what, caps_after=None):
    """Type `keys` into field f through the keypad; check the result against the --expect model and report the other."""
    f0 = page.js('field', f['id'])
    start = f0['value']
    models = dict((m, MODELS[m](start, f['rng'])) for m in MODELS)
    print('  %s %s on %s (start %r): %s' % (name, what, f['id'], start, ' '.join(keys)))
    st = page.open_kb(f['id'])
    if not st.get('open'):
        check(False, '%s %s: the keypad opened on the field\'s mousedown' % (name, f['id']), json.dumps(st)[:200])
        return None
    if name == 'S1':
        note('%s keypad shown: display %r (class "%s", input selection %s), focus on %s' % (
            f['id'], st.get('disp'), st.get('dispClass'), st.get('dispSel'), st.get('active')))
    caps = None
    for k in keys:
        if k == 'OK' and caps_after is not None:
            caps = sorted(page.js('kb').get('steps') or [])
        r = page.press(k)
        if 'err' in r:
            check(False, '%s %s: press "%s"' % (name, f['id'], k), r['err'])
            if page.js('kb').get('open'):
                page.press('Abort')
            return None
        for m in models.values():
            m.key(k)
    fld = page.js('field', f['id'])
    other = 'golden' if mode == 'legacy' else 'legacy'
    got, exp, oexp = fld['value'], models[mode].out(), models[other].out()
    check(got == exp, '%s %s: field shows %r (%s expects %r; %s would give %r)' % (name, f['id'], got, mode, exp, other, oexp))
    check(not fld['kbOpen'], '%s %s: keypad closed after OK' % (name, f['id']))
    ev = fld.get('ev') or {}
    # HW.teach.html AI(W906-TEACH-TYPED) 20261001: a teach-point edit (data-typed-hook) becomes data-src=typed on 'input', so its
    #   Go button sends what the operator typed; any other field keeps what the C route set ("cpp").
    #   AI(W906-KB-GOLDEN) 20261004 (2/2, D6): the golden-mode page marks it typed only when the committed text differs from the text
    #   at open (VCL SetText fires no OnChange for the same text); the legacy page marked any input.
    if mode == 'legacy':
        want_src = 'typed' if (fld.get('typedHook') and ev.get('input', 0) > 0) else f0['src']
    else:
        want_src = 'typed' if (fld.get('typedHook') and (got != start or f0['src'] == 'typed')) else f0['src']
    check(fld['src'] == want_src, '%s %s: data-src %r after the commit (expects %r: typed-hook %s, was %r)' % (
        name, f['id'], fld['src'], want_src, fld.get('typedHook'), f0['src']))
    if got != start:
        check(ev.get('input', 0) >= 1 and ev.get('change', 0) >= 1,
              '%s %s: input / change fired for the new value (input %s, change %s)' % (name, f['id'], ev.get('input'), ev.get('change')))
    else:
        print('    value unchanged; input %s change %s' % (ev.get('input'), ev.get('change')))
    if caps_after is not None:
        exp_caps = models[mode].captions()
        check(caps == exp_caps, '%s %s: step captions after "%%": %s (%s expects %s)' % (name, f['id'], ' '.join(caps or []), mode, ' '.join(exp_caps)))
    RESULTS.append((name, f['id'], start, ' '.join(keys), got, models['legacy'].out(), models['golden'].out()))
    return got


def run_s5(page, mode, f):
    start = page.js('field', f['id'])['value']
    print('  S5 open, read the step captions, press the smallest "+" step, OK on %s (start %r)' % (f['id'], start))
    st = page.open_kb(f['id'])
    if not st.get('open'):
        check(False, 'S5 %s: the keypad opened' % f['id'])
        return
    caps = sorted(st.get('steps') or [])
    models = dict((m, MODELS[m](start, f['rng'])) for m in MODELS)
    exp_caps = models[mode].captions()
    check(caps == exp_caps, 'S5 %s: six step captions %s (%s expects %s; %s shows %s)' % (
        f['id'], ' '.join(caps), mode, ' '.join(exp_caps), 'golden' if mode == 'legacy' else 'legacy',
        ' '.join(models['golden' if mode == 'legacy' else 'legacy'].captions())))
    plus = sorted([c for c in caps if c.startswith('+')], key=lambda c: float(c))
    if not plus:
        check(False, 'S5 %s: a "+" step button exists' % f['id'])
        page.press('Abort')
        return
    for k in (plus[0], 'OK'):
        r = page.press(k)
        if 'err' in r:
            check(False, 'S5 %s: press "%s"' % (f['id'], k), r['err'])
            return
    got = page.js('field', f['id'])['value']
    exp = num_text(int(c_atof(start) + float(plus[0])))               # both models: value + the caption's number
    small = dict((m, sorted([c for c in MODELS[m](start, f['rng']).captions() if c.startswith('+')], key=float)[0]) for m in MODELS)
    check(got == exp, 'S5 %s: "%s" then OK -> %r (expects start %s %s = %r)' % (f['id'], plus[0], got, start, plus[0], exp))
    RESULTS.append(('S5', f['id'], start, plus[0] + ' OK', got,
                    '%s (%s)' % (num_text(int(c_atof(start) + float(small['legacy']))), small['legacy']),
                    '%s (%s)' % (num_text(int(c_atof(start) + float(small['golden']))), small['golden'])))


def type_value(page, f, value):
    """The S4-style workaround (Del first, then '-' if negative, then the digits): same result in both modes."""
    keys = ['Del'] + (['-'] if value.startswith('-') else []) + list(value.lstrip('-')) + ['OK']
    st = page.open_kb(f['id'])
    if not st.get('open'):
        return None, 'keypad did not open'
    for k in keys:
        r = page.press(k)
        if 'err' in r:
            return None, r['err']
    return page.js('field', f['id'])['value'], ''


# ----------------------------------------------------------------------------- the save path
def diff_ini(a, b):
    """{(section, key): (before, after)} for values that differ (numbers compared as numbers)."""
    out = {}
    for sec in set(a) | set(b):
        for k in set(a.get(sec, {})) | set(b.get(sec, {})):
            x, y = a.get(sec, {}).get(k), b.get(sec, {}).get(k)
            if x == y:
                continue
            try:
                if x is not None and y is not None and float(x) == float(y):
                    continue
            except ValueError:
                pass
            out[(sec, k)] = (x, y)
    return out


def files_now():
    return {'teach.ini': read_ini(TEACH_INI), 'Gerneral.ini': read_ini(GERNERAL_INI)}


def note_changes(label, before, after):
    """Attribution only (a NOTE, not a check): which teach.ini / Gerneral.ini values a phase changed."""
    for fn in ('teach.ini', 'Gerneral.ini'):
        d = diff_ini(before[fn], after[fn])
        if d:
            items = sorted(d.items())
            note('%s changed %s (%d): %s%s' % (label, fn, len(d), ', '.join(
                '[%s] %s %r->%r' % (k[0], k[1], v[0], v[1]) for k, v in items[:8]), ' ...' if len(d) > 8 else ''))


def save_path(page, cdp, a, mode, fa, fb, fc):
    """fa TECH_PARA, fb elTeach (value kept inside its C++ range), fc (optional) an elTeach field with a C++ range that gets a
    value OUTSIDE it: golden-faithful silent refusal, check (f)."""
    print('\n[SAVE] reload, type the fields, page Save, YES')
    r = page.load()
    check(r.get('ok') is True, 'page reloaded (C route) for the save', json.dumps(r, ensure_ascii=False)[:300])
    if not r.get('ok'):
        return
    va = '-789' if fa['value'] != '-789' else '-788'
    vb = in_range_value(fb, '4321' if fb['value'] != '4321' else '4322', fb['value'])
    typed = [(fa, va), (fb, vb)]
    vc = None
    if fc:
        cur = page.js('field', fc['id'])['value']
        vc = num_text(fc['cxx'][1] + 321) if fc['cxx'][1] + 321 != float(cur) else num_text(fc['cxx'][1] + 322)
        typed.append((fc, vc))
    before_t, before_g = read_ini(TEACH_INI), read_ini(GERNERAL_INI)
    for f, v in typed:
        got, err = type_value(page, f, v)
        check(got == v, 'typed %s = %s through the keypad (Del %s, OK) -> field shows %r%s' % (
            f['id'], v, '- ' + ' '.join(v[1:]) if v.startswith('-') else ' '.join(v), got,
            '  [outside its C++ range %g..%g on purpose]' % f['cxx'] if f is fc else ''), err)
    page.ev('window.__tkp.gb0 = HT9045Page.golden().page; true')
    nd = len(cdp.dialogs)
    save = page.js('reach', 'btnSave', False)
    check(bool(save.get('visible')) and save.get('hitOk'), 'Save button btnSave visible and clickable', json.dumps(save)[:200])
    if page.clicks == 'cdp':
        cdp.click(save['x'], save['y'])
    else:
        page.ev("document.getElementById('btnSave').click(); true", timeout=30)
    done = None
    for _ in range(400):                                             # 40 s: confirm -> takeover -> editlist.save -> ack -> reload
        done = page.ev('(function(){var g=HT9045Page.golden();return {last:g.lastSave,reloaded:g.page!==window.__tkp.gb0&&!!g.page};})()')
        if done.get('last') and done.get('reloaded'):
            break
        time.sleep(0.1)
    dl = cdp.dialogs[nd:]
    check(len(dl) == 1 and dl[0]['type'] == 'confirm' and Q_SAVE in dl[0]['message'],
          'one confirm box carrying "%s" (answered YES)' % Q_SAVE, json.dumps(dl, ensure_ascii=False)[:300])
    for d in dl:
        note('save confirm box [%s]: %s' % (d['type'], d['message'].replace('\n', ' / ')))
    last = (done or {}).get('last') or {}
    sess = last.get('session') or {}
    asked = [(q.get('en'), q.get('answer')) for q in sess.get('asked') or []]
    check(last.get('saved') is True, 'editlist.save ack: saved', json.dumps(last, ensure_ascii=False)[:400])
    check((Q_SAVE, 1) in asked or not asked, 'golden asked "%s" and got YES (asked %s)' % (Q_SAVE, asked))
    check(bool((done or {}).get('reloaded')), 'the page re-read the C route after the save (engine rule 3)')
    if sess.get('todo'):
        note('golden steps still not ported (session.todo): ' + ' | '.join(sess['todo']))
    for m in sess.get('messages') or []:
        note('save message: %s' % (m.get('zh') or m.get('en')))
    if last.get('ignored'):
        note('save ack "ignored" (not editable now, old value kept): %s' % ', '.join(str(x) for x in last['ignored']))
    after_t, after_g = read_ini(TEACH_INI), read_ini(GERNERAL_INI)
    mine = set()
    for f, v in ((fa, va), (fb, vb)):
        for lst, grp, key in f['binds']:                             # every teach.ini key golden binds to this TEdit
            mine.add((grp.lower(), key.lower()))
            fv = after_t.get(grp.lower(), {}).get(key.lower())
            check(fv == v, '(a) teach.ini [%s] %s = %r (%s %s typed %s; was %r)' % (
                grp, key, fv, lst, f['id'], v, before_t.get(grp.lower(), {}).get(key.lower())))
    if fc:
        for lst, grp, key in fc['binds']:
            mine.add((grp.lower(), key.lower()))
            b4, fv = before_t.get(grp.lower(), {}).get(key.lower()), after_t.get(grp.lower(), {}).get(key.lower())
            check(fv == b4, '(f) out-of-range %s = %s NOT written: teach.ini [%s] %s stays %r (now %r) -- HTEditList range %g..%g, '
                  'golden SaveFile ignores the refusal (uteach.cpp:4958), nothing tells the operator' % (
                      fc['id'], vc, grp, key, b4, fv, fc['cxx'][0], fc['cxx'][1]))
    dt = diff_ini(before_t, after_t)
    other = dict((k, v) for k, v in dt.items() if k not in mine)
    check(not other, '(e) no other teach.ini key changed (%d other)' % len(other), repr(sorted(other.items())[:8]))
    dg = diff_ini(before_g, after_g)
    check(set(dg) <= QUIRK, '(d) Gerneral.ini changed only the known quirk: %s' % (
        ', '.join('[%s] %s %r->%r' % (k[0], k[1], v[0], v[1]) for k, v in sorted(dg.items())) or 'nothing'))
    print('  (b) reopen %s' % PAGE)
    r = page.load()
    check(r.get('ok') is True, '(b) page reopened (C route)', json.dumps(r, ensure_ascii=False)[:300])
    old_c = fc and before_t.get(fc['group'].lower(), {}).get(fc['key'].lower())
    want = [(fa, va), (fb, vb)] + ([(fc, old_c)] if fc else [])      # fc: the OLD value comes back
    if r.get('ok'):
        for f, v in want:
            x = page.js('field', f['id'])
            check(x['value'] == v and x['src'] == 'cpp', '(%s) reopened page: %s = %r (expects %r), data-src %s' % (
                'f' if f is fc else 'b', f['id'], x['value'], v, x['src']))
    print('  (c) fresh WS editlist.get Teach')
    c = Conn('127.0.0.1', a.port, '/ht9045', 30)
    rr = c.cmd('control.takeover')
    check(rr.get('ok') is True, '(c) control.takeover', json.dumps(rr, ensure_ascii=False)[:200])
    pg = c.cmd('editlist.get', 'Teach')
    check(pg.get('ok') is True, '(c) editlist.get Teach', json.dumps(pg, ensure_ascii=False)[:200])
    for f, v in want:
        y = [e for e in entries(pg, f['list']) if e.get('id') == f['id']]
        check(bool(y) and y[0].get('text') == v, '(%s) WS %s %s text = %r (expects %r)' % (
            'f' if f is fc else 'c', f['list'], f['id'], y[0].get('text') if y else None, v))
    try:
        c.cmd('control.release')
        c.sock.close()
    except Exception:
        pass


# ----------------------------------------------------------------------------- main
def foreign_wb_serve():
    """PIDs of wb_serve.exe processes already running (review: wbrun_guard.py aborts on one too)."""
    try:
        o = subprocess.run(['tasklist', '/FI', 'IMAGENAME eq wb_serve.exe', '/FO', 'CSV', '/NH'], stdout=subprocess.PIPE,
                           stderr=subprocess.DEVNULL, timeout=60).stdout.decode('mbcs', 'replace')
    except Exception:   # noqa: BLE001
        return []
    return [r.split('","')[1] for r in o.splitlines() if r.lower().startswith('"wb_serve.exe"') and '","' in r]


def sim_check(port):
    """True / False = machine.defines SOFT_SIMULTE on / off, None = unknown (same call as wbrun_guard.py sim_check)."""
    try:
        opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))
        j = json.loads(opener.open('http://127.0.0.1:%d/api/struct/machine.defines?all=1' % port, timeout=15).read().decode('utf-8', 'replace'))
        d = (j.get('defines') or {}).get('SOFT_SIMULTE')
        return None if d is None else bool(d.get('on'))
    except Exception:   # noqa: BLE001
        return None


def main():
    ap = argparse.ArgumentParser(description='Teach page keypad + read/write browser e2e probe')
    ap.add_argument('--expect', required=True, choices=['legacy', 'golden'], help='legacy = pre-fix qwerty.js, golden = myQwertyKeyBoard.cpp')
    ap.add_argument('--web', default=os.path.join(TREE, 'web'), help='web root to serve (holds page/)')
    ap.add_argument('--port', type=int, default=8045)
    ap.add_argument('--dbg-port', type=int, default=9351)
    ap.add_argument('--wb', default=DEFAULT_WB, help='wb_serve.exe (sim build: SOFT_SIMULTE on)')
    ap.add_argument('--seconds', type=int, default=600, help='wb_serve --seconds (it is stopped earlier by the probe)')
    ap.add_argument('--attach', action='store_true', help='use a wb_serve already on --port (the caller owns the guard)')
    ap.add_argument('--keep', action='store_true', help='do not restore / drop the guard backup')
    ap.add_argument('--tag', default='', help='realfile_guard tag (default teachkb_<timestamp>)')
    ap.add_argument('--clicks', choices=['cdp', 'dom'], default='cdp', help='cdp = trusted mouse events; dom = dispatchEvent / click()')
    ap.add_argument('--fields', default='', help='override the picks: para=ID,el=ID,range=ID')
    ap.add_argument('--keypad-only', action='store_true', help='skip the save path')
    ap.add_argument('--ctest-wait', type=int, default=1800, help='seconds to keep retrying the snap while ctest.exe runs')
    ap.add_argument('--rundir', default='', help='where wb_serve runs and logs (default: a temp dir)')
    a = ap.parse_args()
    tag = a.tag or time.strftime('teachkb_%Y%m%d_%H%M%S')
    rundir = a.rundir or os.path.join(os.environ.get('TEMP', '.'), 'claude', 'teachkb_' + time.strftime('%H%M%S'))
    os.makedirs(rundir, exist_ok=True)
    overrides = dict(x.split('=', 1) for x in a.fields.split(',') if '=' in x)
    print('teach_kb_e2e_probe: expect %s, clicks %s, web %s, port %d, %s' % (
        a.expect, a.clicks, a.web, a.port, 'attach' if a.attach else 'wb_serve ' + a.wb))
    if not os.path.isfile(os.path.join(a.web, 'page', PAGE)):
        print('FAIL: %s has no page/%s' % (a.web, PAGE))
        return 2
    srv, snapped, edge = None, False, None
    if a.attach:
        if not port_open(a.port):
            print('FAIL: --attach but nothing listens on %d' % a.port)
            return 2
        print('  --attach: the caller owns realfile_guard (snap before it started wb_serve, restore after)')
    else:
        if port_open(a.port):
            print('FAIL: something already listens on %d -- not starting a second server' % a.port)
            return 2
        if not os.path.isfile(a.wb):
            print('FAIL: no wb_serve at %s -- build this tree (build.bat / the gate) or pass --wb' % a.wb)
            return 2
        foreign = foreign_wb_serve()
        if foreign:
            print('FAIL: another wb_serve.exe is running (pid %s: F5 or the HMI?) -- its writes would be undone by the restore; stop it first' % ', '.join(foreign))
            return 2
        print('[guard] snap %s (before wb_serve starts)' % tag)
        rc = guard_snap(tag, a.ctest_wait)
        if rc != 0:
            print('FAIL: realfile_guard snap exit %d -- not starting wb_serve' % rc)
            return 2
        snapped = True
    files0 = files_now()
    try:
        if not a.attach:
            srv = Server(a.wb, a.web, a.port, a.seconds, rundir)
            srv.start()
            why = srv.wait_ready(180)
            check(not why, 'wb_serve up on %d' % a.port, why)
            if why:
                return 1
            sim = sim_check(a.port)
            check(sim is True, 'wb_serve is a SIM build (machine.defines SOFT_SIMULTE on)', 'SOFT_SIMULTE %r' % sim)
            if sim is not True:
                return 1
        files1 = files_now()
        note_changes('wb_serve boot' if not a.attach else 'nothing (attach)', files0, files1)
        if port_open(a.dbg_port):
            print('FAIL: the DevTools port %d is busy -- another browser would be driven; pass --dbg-port' % a.dbg_port)
            return 2
        edge, prof, wsurl = launch_edge(a.dbg_port)
        cdp = TeachCdp(wsurl)
        cdp.call('Page.enable')
        cdp.call('Runtime.enable')
        cdp.call('Emulation.setDeviceMetricsOverride', {'width': 1280, 'height': 960, 'deviceScaleFactor': 1, 'mobile': False})
        page = Page(cdp, 'http://127.0.0.1:%d' % a.port, a.clicks)

        print('\n[LOAD] %s' % PAGE)
        r = page.load()
        check(r.get('ok') is True and r.get('struct') == 'Teach', 'C-route load finished (struct Teach, %s fields data-src=cpp, %s ms)' % (
            r.get('cpp'), r.get('ms')), json.dumps(r, ensure_ascii=False)[:400])
        if not r.get('ok'):
            return 1
        note_changes('the first page open (editlist.get = golden FormShow)', files1, files_now())
        ver = page.ev("(function(){var s=[].slice.call(document.scripts).filter(function(x){return /qwerty\\.js/.test(x.src);});return s.length?s[0].src:'';})()")
        note('keypad script %s' % ver)
        ranges = cxx_el_ranges()
        if ranges is None:
            note('cannot read %s -- elTeach C++ ranges unknown; the typed elTeach value may be refused silently' % GEN_INC)
        fields = pick_fields(page, overrides, read_ini(TEACH_INI), ranges)
        check('para' in fields and 'el' in fields, 'a visible, enabled TECH_PARA field and elTeach field were found')
        if 'range' not in fields:
            note('no visible field with a wire range on this machine -- range sequences skipped')
        if 'para' not in fields or 'el' not in fields:
            return 1
        for f in (fields['para'], fields['el']):
            ini = read_ini(TEACH_INI).get(str(f['group']).lower(), {}).get(str(f['key']).lower())
            check(ini is not None and js_parse_float(ini) is not None and float(f['value']) == js_parse_float(ini),
                  'start value %s = %r equals teach.ini [%s] %s = %r' % (f['id'], f['value'], f['group'], f['key'], ini))

        print('\n[KEYPAD] expect %s' % a.expect)
        for kind in ('para', 'el', 'range'):
            f = fields.get(kind)
            if not f:
                continue
            for name, keys, what in SEQS:
                run_seq(page, a.expect, f, name, keys, what, caps_after=True if name == 'S3' else None)
        run_s5(page, a.expect, fields['para'])
        if cdp.errors:
            note('page JS errors during the keypad part: %s' % ' | '.join(cdp.errors[:5]))

        if not a.keypad_only:
            if 'elrange' not in fields:
                note('no visible elTeach field with a C++ range -- check (f) skipped')
            save_path(page, cdp, a, a.expect, fields['para'], fields['el'], fields.get('elrange'))
        if cdp.errors:
            note('page JS errors (all): %d, first: %s' % (len(cdp.errors), ' | '.join(cdp.errors[:5])))
        try:
            cdp.call('Page.navigate', {'url': 'about:blank'})
        except Exception:
            pass
    finally:
        if edge:
            edge.kill()
            shutil.rmtree(prof, ignore_errors=True)
        if srv:
            print('\n[STOP] wb_serve')
            try:
                srv.stop()
            except Exception as e:   # noqa: BLE001 -- review: still check / restore / drop below
                print('  WARN: wb_serve stop raised %r -- continuing with the guard steps' % (e,))
        if snapped:
            print('\n[guard] check %s (what this run changed)' % tag)
            guard('check', tag)
            if a.keep:
                print('  --keep: NOT restored; the backup stays (%s). Restore later: python %s restore %s' % (tag, GUARD, tag))
            else:
                guard('restore', tag)
                rc2, _ = guard('check', tag)
                check(rc2 == 0, 'realfile_guard check after restore: 0 changed')
                if rc2 == 0:
                    guard('drop', tag)
                else:
                    print('  the backup is KEPT (%s) -- restore it by hand' % tag)

    print('\n[SUMMARY] expect %s' % a.expect)
    print('  %-3s %-24s %-12s %-22s %-14s %-14s %-14s' % ('seq', 'field', 'start', 'keys', 'got', 'legacy model', 'golden model'))
    for row in RESULTS:
        print('  %-3s %-24s %-12s %-22s %-14s %-14s %-14s' % tuple(str(x) for x in row))
    for i in INFO:
        print('  NOTE: ' + i)
    print('\n%d failed' % len(FAILS))
    for f in FAILS:
        print('   - ' + f)
    return 1 if FAILS else 0


if __name__ == '__main__':
    sys.exit(main())
