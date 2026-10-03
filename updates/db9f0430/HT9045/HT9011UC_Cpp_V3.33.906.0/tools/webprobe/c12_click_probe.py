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
SAFETY: nothing can reach a real wb_serve or the machine.  Before any page script runs, an injected prelude rewrites EVERY
WebSocket URL (some pages hard-code ws://127.0.0.1:9045/...) and every absolute fetch / XHR URL to the stub's own port; the
stub acks every command ok:false ("c12 probe stub") and answers /api/* with 404 JSON.  No machine files are read or written.
Not a ctest (needs Edge).
Usage: python c12_click_probe.py --census <census.tsv> --out <result.tsv> [--tree <repo root holding web/>] [--pages A.html,B.html]
       [--wait-ms 700] [--dbg-port 9341]
Output columns: page, id, state (clicked / missing / not-visible / greyed), sent (ws cmds ; joined), http, frame, dlg, dom, err, why.
Exit 0 when every page was opened; 1 if Edge or a page failed to load (the rows done so far are still written)."""
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

# The instrument's own control page (run first; a wrong reading on any of the three stops the census, exit 2):
#   c12Ws   opens a WebSocket to the HARD-CODED ws://127.0.0.1:9045/ht9045-json/ and sends {cmd:"c12.selftest"} -> the frame must be
#           recorded in the page AND arrive at the stub (proves the URL rewrite: nothing reaches a real port 9045)
#   c12Dom  only changes the page -> dom > 0, no ws        c12Dead  no handler -> nothing at all
SELFTEST_PAGE = r"""<!DOCTYPE html><html><head><meta charset="utf-8"><title>c12 selftest</title></head><body>
<button id="c12Ws">ws</button><button id="c12Dom">dom</button><button id="c12Dead">dead</button><div id="out"></div>
<script>
document.getElementById('c12Ws').addEventListener('click', function () {
  var s = new WebSocket('ws://127.0.0.1:9045/ht9045-json/');
  s.onopen = function () { s.send(JSON.stringify({type: 'cmd', id: 1, cmd: 'c12.selftest', tag: 'probe'})); };
});
document.getElementById('c12Dom').addEventListener('click', function () { document.getElementById('out').textContent = 'clicked'; });
</script></body></html>"""


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
    for ident in ('c12Ws', 'c12Dom', 'c12Dead'):
        res[ident] = cdp.eval(CLICK % (json.dumps(ident), json.dumps(''), max(wait_ms, 1200)), timeout=30) or {}
    with LOCK:
        stub_ws = [t for _, k, t in LOG if k == 'WS']
    ok = (res['c12Ws'].get('ws') == ['c12.selftest(probe)'] and 'c12.selftest' in stub_ws and
          not res['c12Dom'].get('ws') and res['c12Dom'].get('dom', 0) > 0 and
          not res['c12Dead'].get('ws') and not res['c12Dead'].get('http') and res['c12Dead'].get('dom', 0) == 0)
    print('%s selftest: ws=%s (stub saw %s) dom=%s dead=%s' % ('PASS' if ok else 'FAIL', res['c12Ws'].get('ws'), stub_ws,
                                                              res['c12Dom'].get('dom'), res['c12Dead']))
    return ok


# One click, in the page.  Returns {state, why, ws, http, frame, dlg, dom, err}.
CLICK = r"""
(async function(id, sheet, waitMs){
  var R = window.__c12; if (!R) return {state:'no-prelude'};
  var el = document.getElementById(id);
  if (!el) return {state:'missing'};
  if (sheet) { var t = document.querySelector('.tab[data-tab="' + sheet + '"], [data-tab="' + sheet + '"]:not(.pcPane)');
               if (t && t !== el) { try { t.click(); } catch(e){} await new Promise(function(r){ setTimeout(r, 150); }); } }
  var cs = getComputedStyle(el), vis = el.getClientRects().length > 0 && cs.visibility !== 'hidden';
  var p = el; while (vis && p && p !== document.body) { if (getComputedStyle(p).display === 'none') vis = false; p = p.parentElement; }
  if (!vis) return {state:'not-visible'};
  var why = el.getAttribute('data-unwired') || '';
  if (el.disabled || el.getAttribute('aria-disabled') === 'true' || el.classList.contains('teach-unwired') || why)
    return {state:'greyed', why: why.slice(0, 80)};
  R.ws.length = 0; R.http.length = 0; R.frame.length = 0; R.dlg.length = 0; R.err.length = 0;
  await new Promise(function(r){ setTimeout(r, 300); });
  var baseHttp = R.http.slice(); R.http.length = 0; R.dom = 0;
  ['pointerdown', 'mousedown', 'pointerup', 'mouseup'].forEach(function(k){ try { el.dispatchEvent(new MouseEvent(k, {bubbles:true, cancelable:true, view:window})); } catch(e){} });
  try { el.click(); } catch(e) { R.err.push('click threw ' + e.message); }
  await new Promise(function(r){ setTimeout(r, waitMs); });
  var http = R.http.filter(function(h){ return baseHttp.indexOf(h) < 0; });
  var out = {state:'clicked', ws: R.ws.map(function(x){ return x[0] + (x[1] ? '(' + x[1] + ')' : '') + (x[0] === 'form.event' ? ' ' + x[2] : ''); }),
             http: http, frame: R.frame.slice(), dlg: R.dlg.slice(), dom: R.dom, err: R.err.slice()};
  try { document.dispatchEvent(new KeyboardEvent('keydown', {key:'Escape', bubbles:true})); } catch(e){}
  return out;
})(%s, %s, %d)
"""


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


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--census', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--tree', default=os.path.normpath(os.path.join(HERE, '..', '..', '..')))
    ap.add_argument('--pages', default='')
    ap.add_argument('--wait-ms', type=int, default=700)
    ap.add_argument('--dbg-port', type=int, default=9341)
    a = ap.parse_args()
    pages = set(x for x in a.pages.split(',') if x)
    rows = read_census(a.census, pages)
    byp = {}
    for p, i, s in rows:
        byp.setdefault(p, []).append((i, s))
    srv = make_server(a.tree)
    port = srv.server_address[1]
    proc, prof, wsurl = launch_edge(a.dbg_port)
    out = [['page', 'id', 'state', 'sent', 'http', 'frame', 'dlg', 'dom', 'err', 'why']]
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
                    r = cdp.eval(CLICK % (json.dumps(ident), json.dumps(sheet), a.wait_ms), timeout=30) or {}
                except Exception as e:
                    r = {'state': 'probe-error', 'err': [str(e)[:120]]}
                out.append([page, ident, r.get('state', ''), ' ; '.join(r.get('ws', [])), ' ; '.join(r.get('http', [])),
                            ' ; '.join(r.get('frame', [])), ' ; '.join(r.get('dlg', [])), str(r.get('dom', '')),
                            ' ; '.join(r.get('err', [])), r.get('why', '')])
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
