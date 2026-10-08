# -*- coding: utf-8 -*-
"""tools/webprobe/c12_click_probe.py -- AI(W906-ST02-C12) 20261003 (St02-E): the "what does a click actually send" half of the
ST02-C12 button census (TO_STEVEN.md section 3; the static half is c12_button_census.py).  Written on STEVEN-NB3 where it is
NOT run (St02 builds only); St01 proxy-runs it and the result is merged with
    python c12_button_census.py --merge <out.tsv> --out-tsv ... --out-md ...

What it does: serves <tree>/web from a stub HTTP server, opens every page of the census in headless Edge (top level, no frame),
and for every row the census did not drop (dev page / statically hidden) it activates the row's tab sheet, measures the
element (missing / not visible / greyed + reason), clicks it, waits, and records everything the click produced:
  ws    every WebSocket frame the page sent (cmd, tag, short value)          <- the main column
  http  fetch / XHR other than GET polling seen in the 1.5 s before the click
  frame postMessage / window.open / window.close calls (window-level UI actions, no C++)
  dlg   alert / confirm / prompt texts (confirm and prompt answer "OK" so the send after them is seen)
  dom   number of DOM mutations during the click window (a purely local UI action shows up here only)
  err   uncaught JS errors in the click window
  inp   input / change events during the click window                      (v2)
  val   form controls whose .value / .checked changed in the click window    (v2)
v2 -- AI(W906-ST02-C12T) 20261005 (St02-E; St02-M 04:4x): the v1 click (el.click(), isTrusted false) read two kinds of WIRED
buttons as dead: handlers that only accept a user's click (e.isTrusted; e.g. ht9045_dio_delete.js spbDelete) and handlers that
only change input values (no frame, no DOM mutation; e.g. Setup.Temp_Set btClearAll).  Now the element is scrolled into view,
a point of it that is really on top is found (centre first, then four inset points; the E-09 covered rule), and the click is
a real mouse click from DevTools (Input.dispatchMouseEvent: isTrusted true).  No such point -> state "covered" (the element
under it is in "why"; nothing is clicked); a covered row is retried once on a freshly loaded page (an overlay a previous click
left open).  inp / val count the value-only kind.  The stub server and the prelude are unchanged.
SAFETY: nothing can reach a real wb_serve or the machine.  Before any page script runs, an injected prelude rewrites EVERY
WebSocket URL (some pages hard-code ws://127.0.0.1:9045/...) and every absolute fetch / XHR URL to the stub's own port; the
stub acks every command ok:false ("c12 probe stub") and answers /api/* with 404 JSON.  No machine files are read or written.
Not a ctest (needs Edge).
Usage: python c12_click_probe.py --census <census.tsv> --out <result.tsv> [--tree <repo root holding web/>] [--pages A.html,B.html]
       [--wait-ms 700] [--dbg-port 9341]
       python c12_click_probe.py --dry [--census <census.tsv>]   (no Edge, no server: the plan, the JS bracket balance, the selftest page)
Output columns: page, id, state (clicked / missing / not-visible / greyed / covered), sent (ws cmds ; joined), http, frame, dlg,
dom, err, why, inp, val (the last two are v2; c12_button_census.py --merge reads both widths).
Exit 0 when every page was opened; 1 if Edge or a page failed to load (the rows done so far are still written); 2 if the selftest
fails (nothing is measured)."""
import argparse, base64, hashlib, json, os, struct, sys, threading, time
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from s12_form_probe import Cdp, launch_edge   # noqa: E402
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'
LOG = []                      # (t, kind, text) seen by the stub server
LOCK = threading.Lock()


def note(kind, text):
    with LOCK:
        LOG.append((time.monotonic(), kind, text))


def ws_send(wfile, text):
    b = text.encode('utf-8')
    n = len(b)
    hdr = bytearray([0x81])
    if n < 126:
        hdr.append(n)
    elif n < 65536:
        hdr.append(126)
        hdr += struct.pack('>H', n)
    else:
        hdr.append(127)
        hdr += struct.pack('>Q', n)
    wfile.write(bytes(hdr) + b)
    wfile.flush()


def ws_recv(rfile):
    h = rfile.read(2)
    if len(h) < 2:
        return None
    op, masked, n = h[0] & 0x0f, h[1] & 0x80, h[1] & 0x7f
    if n == 126:
        n = struct.unpack('>H', rfile.read(2))[0]
    elif n == 127:
        n = struct.unpack('>Q', rfile.read(8))[0]
    mask = rfile.read(4) if masked else b''
    data = bytearray(rfile.read(n))
    if masked:
        for i in range(len(data)):
            data[i] ^= mask[i % 4]
    return op, bytes(data)


def make_server(root):
    class H(SimpleHTTPRequestHandler):
        protocol_version = 'HTTP/1.1'

        def __init__(self, *a, **k):
            super().__init__(*a, directory=os.path.join(root, 'web'), **k)

        def log_message(self, *a):
            pass

        def _json(self, code, obj):
            b = json.dumps(obj).encode('utf-8')
            self.send_response(code)
            self.send_header('Content-Type', 'application/json; charset=utf-8')
            self.send_header('Content-Length', str(len(b)))
            self.send_header('Cache-Control', 'no-store')
            self.end_headers()
            self.wfile.write(b)

        def _ws(self):
            key = self.headers.get('Sec-WebSocket-Key', '')
            acc = base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode()
            self.send_response(101)
            self.send_header('Upgrade', 'websocket')
            self.send_header('Connection', 'Upgrade')
            self.send_header('Sec-WebSocket-Accept', acc)
            self.end_headers()
            ws_send(self.wfile, json.dumps({'type': 'snapshot', 'seq': 1, 'generation': 1, 'data': {}}))
            while True:
                fr = ws_recv(self.rfile)
                if fr is None or fr[0] == 8:
                    break
                if fr[0] == 9:
                    self.wfile.write(b'\x8a\x00')
                    self.wfile.flush()
                    continue
                try:
                    m = json.loads(fr[1].decode('utf-8'))
                except Exception:
                    continue
                if isinstance(m, dict):
                    note('WS', str(m.get('cmd') or m.get('type') or '?'))
                if isinstance(m, dict) and 'id' in m:
                    ws_send(self.wfile, json.dumps({'type': 'ack', 'id': m['id'], 'ok': False, 'error': 'c12 probe stub'}))
            self.close_connection = True

        def do_GET(self):
            p = self.path.split('?', 1)[0]
            if self.headers.get('Upgrade', '').lower() == 'websocket':
                return self._ws()
            if p == '/__c12_selftest.html':
                b = SELFTEST_PAGE.encode('utf-8')
                self.send_response(200)
                self.send_header('Content-Type', 'text/html; charset=utf-8')
                self.send_header('Content-Length', str(len(b)))
                self.end_headers()
                self.wfile.write(b)
                return
            if p.startswith('/api/'):
                note('GET', p)
                return self._json(404, {'error': 'c12 probe stub: ' + p})
            return super().do_GET()

        def do_POST(self):
            n = int(self.headers.get('Content-Length') or 0)
            if n:
                self.rfile.read(n)
            note('POST', self.path.split('?', 1)[0])
            return self._json(404, {'error': 'c12 probe stub: POST ' + self.path})

        do_PUT = do_POST

    class S(ThreadingHTTPServer):
        daemon_threads = True

        def handle_error(self, request, client_address):
            if not isinstance(sys.exc_info()[1], (ConnectionAbortedError, ConnectionResetError, BrokenPipeError)):
                super().handle_error(request, client_address)

    srv = S(('127.0.0.1', 0), H)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv


# Injected before any page script (Page.addScriptToEvaluateOnNewDocument).  %PORT% = the stub's port.
PRELUDE = r"""
(function(){
  var P = '%PORT%', R = window.__c12 = {ws:[], http:[], frame:[], dlg:[], err:[], dom:0};
  function toStub(u){ try { var x = new URL(String(u), location.href); if (x.hostname !== '127.0.0.1' || x.port !== P) { x.hostname = '127.0.0.1'; x.port = P; }
                         return x.href; } catch(e){ return u; } }
  var WS = window.WebSocket;
  function W(u, p){ var s = p === undefined ? new WS(toStub(String(u).replace(/^http/, 'ws'))) : new WS(toStub(String(u).replace(/^http/, 'ws')), p);
    var send = s.send.bind(s);
    s.send = function(d){ try { var m = JSON.parse(d); R.ws.push([m.cmd || m.type || '?', m.tag || '', (typeof m.value === 'string' ? m.value : JSON.stringify(m.value || '')).slice(0, 120)]); }
                          catch(e){ R.ws.push(['(raw)', '', String(d).slice(0, 120)]); } return send(d); };
    return s; }
  W.prototype = WS.prototype; W.CONNECTING = 0; W.OPEN = 1; W.CLOSING = 2; W.CLOSED = 3; window.WebSocket = W;
  var F = window.fetch; if (F) window.fetch = function(u, o){ var m = (o && o.method) || 'GET'; R.http.push(m + ' ' + String(u).split('?')[0]); return F(toStub(u), o); };
  var XO = XMLHttpRequest.prototype.open; XMLHttpRequest.prototype.open = function(m, u){ R.http.push(m + ' ' + String(u).split('?')[0]);
    var a = Array.prototype.slice.call(arguments); a[1] = toStub(u); return XO.apply(this, a); };
  var PM = window.postMessage.bind(window);
  window.postMessage = function(d, o){ try { R.frame.push('postMessage ' + JSON.stringify(d).slice(0, 100)); } catch(e){ R.frame.push('postMessage'); } };
  try { Object.defineProperty(window, 'parent', {get: function(){ return window; }}); } catch(e){}
  window.open = function(u){ R.frame.push('open ' + u); return null; };
  window.close = function(){ R.frame.push('close'); };
  window.alert = function(t){ R.dlg.push('alert ' + String(t).slice(0, 80)); };
  window.confirm = function(t){ R.dlg.push('confirm ' + String(t).slice(0, 80)); return true; };
  window.prompt = function(t, d){ R.dlg.push('prompt ' + String(t).slice(0, 80)); return d || ''; };
  window.addEventListener('error', function(e){ R.err.push(String(e.message).slice(0, 100)); });
  window.addEventListener('unhandledrejection', function(e){ R.err.push('rejection ' + String(e.reason && e.reason.message || e.reason).slice(0, 100)); });
  Object.defineProperty(Document.prototype, 'hidden', {get: function(){ return false; }, configurable: true});
  Object.defineProperty(Document.prototype, 'visibilityState', {get: function(){ return 'visible'; }, configurable: true});
  document.addEventListener('DOMContentLoaded', function(){ new MutationObserver(function(l){ R.dom += l.length; })
    .observe(document.documentElement, {subtree: true, childList: true, attributes: true, characterData: true}); });
})();
"""

# The instrument's own control page (run first; a wrong reading on any of them stops the census, exit 2):
#   c12Ws      opens a WebSocket to the HARD-CODED ws://127.0.0.1:9045/ht9045-json/ and sends {cmd:"c12.selftest"} -> the frame must be
#              recorded in the page AND arrive at the stub (proves the URL rewrite: nothing reaches a real port 9045)
#   c12Dom     only changes the page -> dom > 0, no ws        c12Dead  no handler -> nothing at all (no inp / val either)
#   v2 (AI(W906-ST02-C12T) 20261005):
#   c12Trusted reacts only to a user's click (e.isTrusted), and sits 2400 px down the page -> clicked after scrolling, dom > 0
#              (the v1 el.click() read this one as dead)
#   c12Value   only sets an input's .value (no event, no attribute) -> val > 0, dom == 0 (v1 read this one as dead too)
#   c12Covered lies under a box that takes the click -> state "covered", nothing clicked (the box's handler would write #out)
#   v3 (AI(W906-ST02-C12-TAB) 20261008): c12Tab sits on the second pane of a generator-style page control (.tab[data-t] / .pcPane[data-p],
#              pane hidden) -> the probe clicks its tab first, then the button: clicked, dom > 0
#   c12TabHid  sits on a pane whose tab is itself display:none -> state "tab-hidden", nothing clicked (its handler would write #out3)
SELFTEST_PAGE = r"""<!DOCTYPE html><html><head><meta charset="utf-8"><title>c12 selftest</title></head><body style="margin:0">
<button id="c12Ws">ws</button><button id="c12Dom">dom</button><button id="c12Dead">dead</button><div id="out"></div>
<input id="c12In" value=""><button id="c12Value">value</button>
<div style="position:relative;width:200px;height:40px"><button id="c12Covered" style="position:absolute;left:0;top:0;width:120px;height:30px">covered</button>
<div id="c12Lid" style="position:absolute;left:0;top:0;width:200px;height:40px;z-index:5;background:rgba(0,0,0,.1)"></div></div>
<div class="pcWrap"><div class="tabs pcTabs"><div class="tab act" data-t="0">t0</div><div class="tab" data-t="1">t1</div><div class="tab" data-t="2" style="display:none">t2</div></div>
<div class="pcPane" data-p="0" title="tsA">pane 0</div><div class="pcPane" data-p="1" title="tsB" style="display:none;"><button id="c12Tab">tab</button></div>
<div class="pcPane" data-p="2" title="tsC" style="display:none;"><button id="c12TabHid">tabhid</button></div></div><div id="out3"></div>
<div style="height:2400px"></div><button id="c12Trusted">trusted</button><div id="out2"></div>
<script>
document.getElementById('c12Ws').addEventListener('click', function () {
  var s = new WebSocket('ws://127.0.0.1:9045/ht9045-json/');
  s.onopen = function () { s.send(JSON.stringify({type: 'cmd', id: 1, cmd: 'c12.selftest', tag: 'probe'})); };
});
document.getElementById('c12Dom').addEventListener('click', function () { document.getElementById('out').textContent = 'clicked'; });
document.getElementById('c12Trusted').addEventListener('click', function (e) { if (e.isTrusted) document.getElementById('out2').textContent = 'trusted'; });
document.getElementById('c12Value').addEventListener('click', function () { document.getElementById('c12In').value = 'v' + Date.now(); });
document.getElementById('c12Lid').addEventListener('click', function () { document.getElementById('out').textContent = 'lid'; });
Array.prototype.forEach.call(document.querySelectorAll('.pcWrap .tab'), function (t) { t.addEventListener('click', function () {
  Array.prototype.forEach.call(document.querySelectorAll('.pcWrap .pcPane'), function (p) { p.style.display = p.getAttribute('data-p') === t.getAttribute('data-t') ? '' : 'none'; }); }); });
document.getElementById('c12Tab').addEventListener('click', function () { document.getElementById('out3').textContent = 'tab'; });
document.getElementById('c12TabHid').addEventListener('click', function () { document.getElementById('out3').textContent = 'tabhid'; });
</script></body></html>"""
SELFTEST_IDS = ('c12Ws', 'c12Dom', 'c12Dead', 'c12Trusted', 'c12Value', 'c12Covered', 'c12Tab', 'c12TabHid')   # + v3 c12Tab / c12TabHid


def selftest(cdp, port, wait_ms):
    cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/__c12_selftest.html' % port})
    for _ in range(40):
        try:
            if cdp.eval("document.readyState") == 'complete':
                break
        except Exception:
            pass
        time.sleep(0.25)
    res = {}
    for ident in SELFTEST_IDS:
        res[ident] = click_row(cdp, ident, '', max(wait_ms, 1200))
    with LOCK:
        stub_ws = [t for _, k, t in LOG if k == 'WS']
    nothing = lambda r: not r.get('ws') and not r.get('http') and r.get('dom', 0) == 0 and not r.get('inp') and not r.get('val')
    checks = [
        ('ws', res['c12Ws'].get('ws') == ['c12.selftest(probe)'] and 'c12.selftest' in stub_ws),
        ('dom', res['c12Dom'].get('state') == 'clicked' and not res['c12Dom'].get('ws') and res['c12Dom'].get('dom', 0) > 0),
        ('dead', res['c12Dead'].get('state') == 'clicked' and nothing(res['c12Dead'])),
        ('trusted', res['c12Trusted'].get('state') == 'clicked' and res['c12Trusted'].get('dom', 0) > 0),
        ('value', res['c12Value'].get('state') == 'clicked' and res['c12Value'].get('val', 0) > 0 and res['c12Value'].get('dom', 0) == 0),
        ('covered', res['c12Covered'].get('state') == 'covered' and 'c12Lid' in res['c12Covered'].get('why', '')),
        ('tab', res['c12Tab'].get('state') == 'clicked' and res['c12Tab'].get('dom', 0) > 0),                    # v3
        ('tabhid', res['c12TabHid'].get('state') == 'tab-hidden' and res['c12TabHid'].get('why', '') == 'tsC'),  # v3
    ]
    ok = all(v for _, v in checks)
    print('%s selftest: %s (stub saw %s)' % ('PASS' if ok else 'FAIL', ' '.join('%s=%s' % (k, 'ok' if v else 'WRONG') for k, v in checks),
                                            stub_ws))
    if not ok:
        for ident in SELFTEST_IDS:
            print('   %-11s %s' % (ident, json.dumps(res[ident], ensure_ascii=False)[:300]))
    return ok


# One click, step 1 (in the page): the row's tab sheet, visible / greyed, scroll into view, a point of the element that is
# really on top (centre, then four inset points), a snapshot of every form control's value, then {state:'ready', x, y}.
# A point counts only when elementFromPoint gives the element or something inside it (an ancestor would take the click).
PREP = r"""
(async function(id, sheet){
  var R = window.__c12; if (!R) return {state:'no-prelude'};
  var el = document.getElementById(id);
  if (!el) return {state:'missing'};
  if (sheet) { var s1 = sheet.split('>').pop(), t = document.querySelector('.tab[data-tab="' + s1 + '"], [data-tab="' + s1 + '"]:not(.pcPane)');
               if (t && t !== el) { try { t.click(); } catch(e){} await new Promise(function(r){ setTimeout(r, 150); }); } }
  // AI(W906-ST02-C12-TAB) 20261008 (St02-E) v3: the page generator's tabs are .tab[data-t=N] and its panes .pcPane[data-p=N] in one
  // .pcWrap (no data-tab) -- v1 / v2 never switched them, so every button off the default tab read not-visible (INBOX 155: 1058 rows).
  // Every hidden pane above the element: the tab with the same index in the same .pcWrap, clicked outermost first; a tab that is itself
  // hidden (golden dfm TabVisible=False, or the page's own rule) -> state "tab-hidden", nothing clicked.
  var tabs = [], q = el;
  function wrapOf(n){ var w = n.parentElement; while (w && !(w.classList && w.classList.contains('pcWrap'))) w = w.parentElement; return w; }
  while (q && q !== document.body) {
    if (q.classList && q.classList.contains('pcPane') && getComputedStyle(q).display === 'none') {
      var wq = wrapOf(q), np = q.getAttribute('data-p'), tb = null;
      if (wq) Array.prototype.some.call(wq.querySelectorAll('.tab[data-t="' + np + '"]'), function(t2){ if (wrapOf(t2) === wq) { tb = t2; return true; } return false; });
      if (!tb) return {state:'not-visible', why:'no tab for pane ' + (q.getAttribute('title') || np)};
      if (getComputedStyle(tb).display === 'none' || getComputedStyle(tb).visibility === 'hidden') return {state:'tab-hidden', why:(q.getAttribute('title') || 'p' + np)};
      tabs.unshift(tb);
    }
    q = q.parentElement;
  }
  for (var ti = 0; ti < tabs.length; ti++) { try { tabs[ti].click(); } catch(e){} await new Promise(function(r){ setTimeout(r, 150); }); }
  var cs = getComputedStyle(el), vis = el.getClientRects().length > 0 && cs.visibility !== 'hidden';
  var p = el; while (vis && p && p !== document.body) { if (getComputedStyle(p).display === 'none') vis = false; p = p.parentElement; }
  if (!vis) return {state:'not-visible'};
  var why = el.getAttribute('data-unwired') || '';
  if (el.disabled || el.getAttribute('aria-disabled') === 'true' || el.classList.contains('teach-unwired') || why)
    return {state:'greyed', why: why.slice(0, 80)};
  try { el.scrollIntoView({block:'center', inline:'center'}); } catch(e){ try { el.scrollIntoView(); } catch(e2){} }
  await new Promise(function(r){ setTimeout(r, 120); });
  function desc(n){ return !n ? '(none)' : n.id ? n.tagName.toLowerCase() + '#' + n.id : n.tagName.toLowerCase() + '.' + String(n.className || '').split(' ')[0]; }
  var r = el.getBoundingClientRect(), vw = window.innerWidth, vh = window.innerHeight, hit = null, under = null;
  [[0.5, 0.5], [0.25, 0.25], [0.75, 0.25], [0.25, 0.75], [0.75, 0.75]].some(function(f){
    var x = r.left + r.width * f[0], y = r.top + r.height * f[1];
    if (x < 0 || y < 0 || x >= vw || y >= vh) return false;
    var t = document.elementFromPoint(x, y);
    if (t && (t === el || el.contains(t))) { hit = [x, y]; return true; }
    if (!under) under = t;
    return false;
  });
  if (!hit) return {state:'covered', why: (under ? 'under ' + desc(under) : 'off screen') + ' (' + [r.left, r.top, r.width, r.height].map(Math.round).join(',') + ')'};
  R.ws.length = 0; R.http.length = 0; R.frame.length = 0; R.dlg.length = 0; R.err.length = 0;
  if (!window.__c12inp) { window.__c12inp = true; R.inp = 0;
    ['input', 'change'].forEach(function(k){ document.addEventListener(k, function(){ R.inp++; }, true); }); }
  await new Promise(function(r2){ setTimeout(r2, 300); });
  R.baseHttp = R.http.slice(); R.http.length = 0; R.dom = 0; R.inp = 0;
  R.vals = Array.prototype.map.call(document.querySelectorAll('input, select, textarea'), function(c){ return [c, c.value, !!c.checked]; });
  return {state:'ready', x: hit[0], y: hit[1]};
})(%s, %s)
"""

# One click, step 3 (in the page, after the DevTools mouse click): wait, then everything the click produced.
COLLECT = r"""
(async function(waitMs){
  var R = window.__c12; if (!R) return {state:'no-prelude'};
  await new Promise(function(r){ setTimeout(r, waitMs); });
  var base = R.baseHttp || [], http = R.http.filter(function(h){ return base.indexOf(h) < 0; });
  var val = (R.vals || []).filter(function(v){ return v[0].value !== v[1] || !!v[0].checked !== v[2]; }).length;
  var out = {state:'clicked', ws: R.ws.map(function(x){ return x[0] + (x[1] ? '(' + x[1] + ')' : '') + (x[0] === 'form.event' ? ' ' + x[2] : ''); }),
             http: http, frame: R.frame.slice(), dlg: R.dlg.slice(), dom: R.dom, err: R.err.slice(), inp: R.inp || 0, val: val};
  R.vals = null;
  try { document.dispatchEvent(new KeyboardEvent('keydown', {key:'Escape', bubbles:true})); } catch(e){}
  return out;
})(%d)
"""


def click_row(cdp, ident, sheet, wait_ms):
    """PREP in the page, a real mouse click from DevTools (isTrusted true), COLLECT.  Returns the row dict."""
    r = cdp.eval(PREP % (json.dumps(ident), json.dumps(sheet)), timeout=30) or {}
    if r.get('state') != 'ready':
        return r
    x, y = r['x'], r['y']
    for p in ({'type': 'mouseMoved', 'x': x, 'y': y, 'button': 'none', 'buttons': 0},
              {'type': 'mousePressed', 'x': x, 'y': y, 'button': 'left', 'buttons': 1, 'clickCount': 1},
              {'type': 'mouseReleased', 'x': x, 'y': y, 'button': 'left', 'buttons': 0, 'clickCount': 1}):
        m = cdp.call('Input.dispatchMouseEvent', p)
        if 'error' in m:
            raise RuntimeError('Input.dispatchMouseEvent %s: %s' % (p['type'], json.dumps(m['error'])[:120]))
    return cdp.eval(COLLECT % wait_ms, timeout=30) or {}


def read_census(path, pages):
    rows = []
    with open(path, encoding='utf-8') as f:
        hdr = f.readline().rstrip('\n').split('\t')
        ix = dict((k, i) for i, k in enumerate(hdr))
        for l in f:
            c = l.rstrip('\n').split('\t')
            if len(c) < len(hdr):
                continue
            if c[ix['cat']] in ('dev-page', 'hidden(static)') or c[ix['id']] == '(no id)':
                continue
            if pages and c[ix['page']] not in pages:
                continue
            rows.append((c[ix['page']], c[ix['id']], c[ix['sheet']]))
    return rows


def dry(a, byp):
    """--dry: no Edge, no server.  The plan, the JS bracket balance, the selftest page carries every id selftest() clicks."""
    print('DRY: %d pages, %d rows from %s; nothing launched' % (len(byp), sum(len(v) for v in byp.values()), a.census or '(no --census)'))
    for p in sorted(byp)[:8]:
        print('  %-40s %d rows' % (p, len(byp[p])))
    for name, js in (('PRELUDE', PRELUDE), ('PREP', PREP), ('COLLECT', COLLECT), ('SELFTEST_PAGE', SELFTEST_PAGE)):
        assert js.count('(') == js.count(')') and js.count('{') == js.count('}') and js.count('[') == js.count(']'), name
    print('JS bracket balance OK (PRELUDE / PREP / COLLECT / SELFTEST_PAGE)')
    for ident in SELFTEST_IDS:
        assert ('id="%s"' % ident) in SELFTEST_PAGE, ident
    assert (PREP % ('"x"', '""')).count('%') == 0 and (COLLECT % 700).count('%') == 0
    print('selftest page carries %s; PREP / COLLECT format OK' % ', '.join(SELFTEST_IDS))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--census', default='')
    ap.add_argument('--out', default='')
    ap.add_argument('--tree', default=os.path.normpath(os.path.join(HERE, '..', '..', '..')))
    ap.add_argument('--pages', default='')
    ap.add_argument('--wait-ms', type=int, default=700)
    ap.add_argument('--dbg-port', type=int, default=9341)
    ap.add_argument('--dry', action='store_true')
    a = ap.parse_args()
    pages = set(x for x in a.pages.split(',') if x)
    rows = read_census(a.census, pages) if a.census else []
    byp = {}
    for p, i, s in rows:
        byp.setdefault(p, []).append((i, s))
    if a.dry:
        return dry(a, byp)
    if not a.census or not a.out:
        raise SystemExit('--census and --out are required (or use --dry)')
    srv = make_server(a.tree)
    port = srv.server_address[1]
    proc, prof, wsurl = launch_edge(a.dbg_port)
    out = [['page', 'id', 'state', 'sent', 'http', 'frame', 'dlg', 'dom', 'err', 'why', 'inp', 'val']]
    bad = 0
    try:
        cdp = Cdp(wsurl)
        cdp.call('Page.enable')
        cdp.call('Page.addScriptToEvaluateOnNewDocument', {'source': PRELUDE.replace('%PORT%', str(port))})
        if not selftest(cdp, port, a.wait_ms):
            print('the instrument does not read a known-live / known-dead button correctly -- census NOT run')
            sys.exit(2)
        for page in sorted(byp):
            def load():
                cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/page/%s' % (port, page)})
                for _ in range(80):
                    try:
                        if cdp.eval("document.readyState") == 'complete':
                            break
                    except Exception:
                        pass
                    time.sleep(0.25)
                time.sleep(2.0)                                  # binders that attach after load / after the first snapshot
            try:
                load()
            except Exception as e:
                bad += 1
                print('FAIL load %s: %s' % (page, e))
                continue
            n = 0
            for ident, sheet in byp[page]:
                try:
                    r = click_row(cdp, ident, sheet, a.wait_ms)
                    if r.get('state') == 'covered':              # an overlay a previous click left open? once more on a fresh page
                        load()
                        r = click_row(cdp, ident, sheet, a.wait_ms)
                except Exception as e:
                    r = {'state': 'probe-error', 'err': [str(e)[:120]]}
                out.append([page, ident, r.get('state', ''), ' ; '.join(r.get('ws', [])), ' ; '.join(r.get('http', [])),
                            ' ; '.join(r.get('frame', [])), ' ; '.join(r.get('dlg', [])), str(r.get('dom', '')),
                            ' ; '.join(r.get('err', [])), r.get('why', ''), str(r.get('inp', '')), str(r.get('val', ''))])
                n += 1
                try:                                             # a click may navigate, open a keyboard, or close the window
                    href = cdp.eval("location.pathname")
                    if not str(href).endswith('/' + page) or n % 40 == 0:
                        load()
                except Exception:
                    load()
            print('page %-40s %d rows' % (page, n))
    finally:
        try:
            proc.kill()
        except Exception:
            pass
        with open(a.out, 'wb') as f:
            f.write(('\n'.join('\t'.join(str(x).replace('\t', ' ').replace('\n', ' ') for x in r) for r in out) + '\n').encode('utf-8'))
        print('wrote %s (%d rows)' % (a.out, len(out) - 1))
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
