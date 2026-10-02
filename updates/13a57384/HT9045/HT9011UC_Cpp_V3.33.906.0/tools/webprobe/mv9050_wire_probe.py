# -*- coding: utf-8 -*-
"""tools/webprobe/mv9050_wire_probe.py -- AI(W906-MV9050-WIRE) 20261002: the Motion View window on HT9050 (headless Edge, a stub server).
  F  the main screen (web/background.html) follows the machine.gpibModel tag (NB2 MR !124, AI(W906-MV-GPIB)): with 9050GPIB the
     motionview window's page becomes page/Main.MotionView9050.html, its title names HT9050, and the loaded iframes are reloaded
     with &machine=HT9050 (background.html appends &machine=<MACHINE_ID> to every iframe, and the child pages' HT9045Live.machine()
     takes the URL first -- so this is the only place the switch can be made; an in-page redirect never sees HT9050 in the frame).
  A  with that window opened (openWin), ht9045_mv9050_axes.js fills the 9050 page's axis table from /api/struct/motor/runtime:
     cmdPos as is, encPos marked（enc）when there is no cmdPos, a motor without a position and the cylinder row keep "–",
     axis-disabled rows keep "–".
  N  an HT9045-family gpibModel (9046_32GPIB) changes nothing: the window keeps Main.MotionView.html and &machine=HT9045.
  P  the 9050 page opened on its own (no frame, Model HT-9050) fills the table too; 3 failed polls turn filled cells into "—".
  CONTROL: --control <tree> runs F against another tree (e.g. HEAD's web/ via git archive, without MR !124) and expects it to FAIL.
The stub server serves <tree>/web, answers /api/system/gerneral (Model HT-9045W = the HT9050 machine's own Gerneral.ini, or
HT-9050 for P), /api/recipe/, /api/recipe/<doc>, /api/struct/motor/runtime, and a WebSocket at /ht9045 that sends one tag
snapshot {machine.gpibModel} and acks every command with ok:false.  Read-only: no wb_serve, no machine files.  Not a ctest (needs Edge).
Usage: python tools/webprobe/mv9050_wire_probe.py [<tree holding web/>] [--control <tree>]   Exit 0 = all pass."""
import base64, hashlib, json, os, struct, sys, threading, time
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler, SimpleHTTPRequestHandler
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from s12_form_probe import Cdp, launch_edge   # noqa: E402
sys.stdout.reconfigure(encoding='utf-8', errors='replace')

args = [a for a in sys.argv[1:]]
CONTROL = None
if '--control' in args:
    i = args.index('--control'); CONTROL = args[i + 1]; del args[i:i + 2]
TREE = args[0] if args else os.path.normpath(os.path.join(HERE, '..', '..', '..'))
FAILS = []
STATE = {'model': 'HT-9045W', 'gpib': '9050GPIB', 'motor_ok': True}
MOTORS = {'motors': [
    {'motorId': 'MInArmX', 'position': {'cmdPos': 12345, 'encPos': 12340}},
    {'motorId': 'MTestZ1', 'position': {'encPos': -77}},
    {'motorId': 'MOutArmX', 'position': {}},
]}
WS_GUID = '258EAFA5-E914-47DA-95CA-C5AB0DC85B11'


def check(ok, what):
    print(('PASS ' if ok else 'FAIL ') + what)
    if not ok:
        FAILS.append(what)


def ws_send(wfile, text):
    b = text.encode('utf-8'); n = len(b)
    hdr = bytearray([0x81])
    if n < 126:
        hdr.append(n)
    elif n < 65536:
        hdr.append(126); hdr += struct.pack('>H', n)
    else:
        hdr.append(127); hdr += struct.pack('>Q', n)
    wfile.write(bytes(hdr) + b); wfile.flush()


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
            STATE['ws'] = STATE.get('ws', 0) + 1
            key = self.headers.get('Sec-WebSocket-Key', '')
            acc = base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode()
            self.send_response(101)
            self.send_header('Upgrade', 'websocket')
            self.send_header('Connection', 'Upgrade')
            self.send_header('Sec-WebSocket-Accept', acc)
            self.end_headers()
            ws_send(self.wfile, json.dumps({'type': 'snapshot', 'seq': 1, 'generation': 1,
                                            'data': {'machine.gpibModel': STATE['gpib']}}))
            while True:
                fr = ws_recv(self.rfile)
                if fr is None or fr[0] == 8:
                    break
                if fr[0] == 9:                       # ping -> pong
                    self.wfile.write(b'\x8a\x00'); self.wfile.flush(); continue
                try:
                    m = json.loads(fr[1].decode('utf-8'))
                except Exception:
                    continue
                if isinstance(m, dict) and 'id' in m:
                    ws_send(self.wfile, json.dumps({'type': 'ack', 'id': m['id'], 'ok': False, 'error': 'probe stub'}))
            self.close_connection = True

        def do_GET(self):
            p = self.path.split('?', 1)[0]
            if p == '/ht9045' and self.headers.get('Upgrade', '').lower() == 'websocket':
                return self._ws()
            if p == '/api/system/gerneral':
                return self._json(200, {'available': True, 'sections': {'Version': {'Model': {'value': STATE['model']}}}})
            if p == '/api/recipe/':
                return self._json(200, {'recipe': 'PROBE_RECIPE'})
            if p in ('/api/recipe/tray', '/api/recipe/hotPlate'):
                return self._json(200, {'available': True, 'sections': {}})
            if p == '/api/struct/motor/runtime':
                if not STATE['motor_ok']:
                    return self._json(503, {'error': 'probe: server gone'})
                return self._json(200, MOTORS)
            if p.startswith('/api/'):
                return self._json(404, {'error': 'probe stub: ' + p})
            return super().do_GET()

        def do_POST(self):
            n = int(self.headers.get('Content-Length') or 0)
            if n:
                self.rfile.read(n)
            return self._json(404, {'error': 'probe stub: POST ' + self.path})

    class S(ThreadingHTTPServer):
        daemon_threads = True

        def handle_error(self, request, client_address):   # the browser aborting a request mid-navigation is not a finding
            if not isinstance(sys.exc_info()[1], (ConnectionAbortedError, ConnectionResetError, BrokenPipeError)):
                super().handle_error(request, client_address)

    srv = S(('127.0.0.1', 0), H)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv


def wait(cdp, expr, secs):
    end = time.monotonic() + secs
    while time.monotonic() < end:
        try:
            v = cdp.eval(expr)
            if v:
                return v
        except Exception:
            pass
        time.sleep(0.25)
    return None


MVF = "document.querySelector('#win-motionview iframe')"
ROWS = ("(function(d){ if(!d) return ''; return JSON.stringify(Array.prototype.map.call(d.querySelectorAll('#axTable tbody tr'),"
        "function(tr){ return [tr.cells[0].textContent.trim(), tr.cells[2] ? tr.cells[2].textContent : '', tr.className]; })); })(%s)")


def axes_by_name(js):
    rows = json.loads(js) if js else []
    by = {}
    for name, val, cls in rows:
        by[name.split()[-1] if name.split() else name] = (val, cls)
    return by


def check_axes(by, tag):
    check(by.get('MInArmX', ('',))[0] == '12345', tag + ': MInArmX shows cmdPos 12345 (got %r)' % (by.get('MInArmX'),))
    check(by.get('MTestZ1', ('',))[0] == '-77（enc）', tag + ': MTestZ1 shows encPos -77（enc） (got %r)' % (by.get('MTestZ1'),))
    check(by.get('MOutArmX', ('',))[0] == '–', tag + ': MOutArmX (no position) keeps "–" (got %r)' % (by.get('MOutArmX'),))
    cyl = [v for k, v in by.items() if k.startswith('C_')]
    check(bool(cyl) and all(v[0] == '–' for v in cyl), tag + ': the cylinder row keeps "–" (%r)' % (cyl,))
    dis = [v for v in by.values() if 'axis-disabled' in (v[1] or '')]
    check(bool(dis) and all(v[0] == '–' for v in dis), tag + ': %d axis-disabled rows keep "–"' % len(dis))


def run(tree, dbg_port, control=False):
    srv = make_server(tree)
    base = 'http://127.0.0.1:%d' % srv.server_address[1]
    proc, prof, ws = launch_edge(dbg_port)
    try:
        cdp = Cdp(ws)
        cdp.call('Page.enable')
        # headless Edge reports document.hidden=true at random (measured 1002 21:4x: the same page polled in one run and not in
        # the next) and both axis feeds stop polling while hidden -- force 'visible' in every frame, before any page script runs
        cdp.call('Page.addScriptToEvaluateOnNewDocument', {'source': "Object.defineProperty(Document.prototype,'hidden',{get:function(){return false;},configurable:true});"
                  "Object.defineProperty(Document.prototype,'visibilityState',{get:function(){return 'visible';},configurable:true});"})
        # F: 9050GPIB -> the motionview window switches in the main screen
        STATE.update(model='HT-9045W', gpib='9050GPIB', motor_ok=True)
        cdp.call('Page.navigate', {'url': base + '/background.html?mode=debug'})
        got = wait(cdp, "(function(f){ return f && f.getAttribute('data-base-src')==='page/Main.MotionView9050.html' && /machine=HT9050/.test(f.src) ? f.src : ''; })(" + MVF + ")", 40)
        if control:
            check(not got, 'CONTROL: the tree without MR !124 does NOT switch the motionview window on 9050GPIB (got %r)' % (got,))
            return
        check(bool(got), 'F: 9050GPIB -> motionview window = page/Main.MotionView9050.html with &machine=HT9050 (%r)' % (got,))
        ttl = cdp.eval("(function(){ var t=document.querySelector('#win-motionview .tbar .ttl'); return t ? t.textContent : ''; })()")
        wt = cdp.eval("(function(){ var w=WINDOWS.filter(function(x){ return x.id==='motionview'; })[0]; return w ? w.title : ''; })()")
        check(ttl == 'Motion View' and 'HT9050' in (wt or ''), 'F: the bar shows "Motion View" (dispTitle drops the note, no stray "）") and the window entry names HT9050 (%r / %r)' % (ttl, wt))
        others = cdp.eval("JSON.stringify(Array.prototype.map.call(document.querySelectorAll('.win iframe[data-base-src]'), function(f){ return f.src; }))")
        others = json.loads(others or '[]')
        check(len(others) > 1 and all('machine=HT9050' in s for s in others),
              'F: all %d loaded iframes now carry &machine=HT9050 (%d do not)' % (len(others), sum(1 for s in others if 'machine=HT9050' not in s)))
        # A: open the window, the 9050 page in it fills the axis table
        cdp.eval("openWin('motionview'), true")
        filled = wait(cdp, "(function(){ var f=" + MVF + ", d=f&&f.contentDocument; var r=" + (ROWS % 'd') +
                      "; return r && JSON.parse(r).some(function(x){ return /MInArmX$/.test(x[0]) && x[1]==='12345'; }) ? r : ''; })()", 30)
        if not filled:
            filled = cdp.eval("(function(){ var f=" + MVF + "; return " + (ROWS % 'f&&f.contentDocument') + "; })()")
        check_axes(axes_by_name(filled), 'A (in the main screen)')
        # N: an HT9045-family gpibModel changes nothing
        STATE.update(gpib='9046_32GPIB')
        cdp.call('Page.navigate', {'url': base + '/background.html?mode=debug'})
        wait(cdp, "document.readyState==='complete' && !!" + MVF, 20)
        time.sleep(8)
        st = cdp.eval("(function(f){ return f ? f.getAttribute('data-base-src')+' | '+f.src : ''; })(" + MVF + ")")
        check(bool(st) and st.startswith('page/Main.MotionView.html |') and 'machine=HT9045' in st and 'Main.MotionView9050' not in st,
              'N: 9046_32GPIB keeps Main.MotionView.html with &machine=HT9045 (%r)' % (st,))
        # P: the 9050 page on its own
        STATE.update(model='HT-9050', gpib='', motor_ok=True)
        cdp.call('Page.navigate', {'url': base + '/page/Main.MotionView9050.html?mode=debug'})
        filled = wait(cdp, "(function(){ var r=" + (ROWS % 'document') + "; return r && JSON.parse(r).some(function(x){ return /MInArmX$/.test(x[0]) && x[1]==='12345'; }) ? r : ''; })()", 20)
        check(not cdp.eval("document.body.classList.contains('gated')"), 'P: the page on its own (Model HT-9050) is not gated')
        check_axes(axes_by_name(filled or cdp.eval(ROWS % 'document')), 'P (page on its own)')
        STATE['motor_ok'] = False
        gone = wait(cdp, "(function(){ var r=JSON.parse(" + (ROWS % 'document') + "); return r.some(function(x){ return /MInArmX$/.test(x[0]) && x[1]==='—'; }); })()", 8)
        check(bool(gone), 'P: server gone (3 failed polls) -> the filled MInArmX cell turns "—"')
    finally:
        proc.kill()
        srv.shutdown()


if __name__ == '__main__':
    run(TREE, 9341)
    if CONTROL:
        run(CONTROL, 9342, control=True)
    print('%s (%d failed)' % ('ALL PASS' if not FAILS else 'FAILED', len(FAILS)))
    sys.exit(1 if FAILS else 0)
