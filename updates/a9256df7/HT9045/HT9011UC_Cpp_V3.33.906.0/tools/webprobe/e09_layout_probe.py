# -*- coding: utf-8 -*-
"""tools/webprobe/e09_layout_probe.py -- AI(W906-ST02-E09) 20261004 (St02-E): card E-09 (TO_ES02.md section 3, EastSun via
Jimmy, St02 since 1004 20:3x) -- "on the HT9050 screen, list every window / page that is covered or cut off".  Written on
STEVEN-NB3 where it is NOT run (St02 compiles only, runs no exe -- Edge included); St01 proxy-runs it.

What it does: serves <tree>/web from a stub HTTP server, opens web/background.html in headless Edge with the viewport of the
HMI shell's work area (--screen, default 1920x1032 = 1920x1080 minus a 48 px taskbar) and the main screen's zoom (--zoom,
default 110 = main.html's selector; background.html reads ?zoom=), then opens every window of its WINDOWS table one by one
(openWin(id, true) -- the program path: no operator guard / alert) and measures, per window:
  cut      how far the window's frame is outside the visible desktop (left, top, right, bottom; css px on screen)
  doc      the page's scroll size vs its viewport, and whether body / html can scroll (overflow not hidden / clip)
  clipped  visible controls cut off by an ancestor with overflow hidden / clip (unreachable: no scrolling shows them)
  covered  visible controls whose centre point belongs to some OTHER element (document.elementFromPoint) -- a tip, a banner,
           a floating box, another control -- with the coverer's id / class
  wincov   sample points of the window frame that show something else in background.html (another window, an overlay)
Every tab sheet of every page control (.pcTabs > .tab) is activated in turn and measured, so controls on tabs other than the
first are measured too (the C12 click probe left those "not visible").  The window is closed again afterwards.
SAFETY: nothing reaches a real wb_serve or the machine -- an injected prelude rewrites every WebSocket / fetch / XHR URL to the
stub's port (the c12_click_probe.py way; the stub acks commands ok:false and answers /api/* with 404), alert / confirm / prompt
are no-ops.  No machine files are read or written.  Not a ctest (needs Edge).
SELF-TEST first (exit 2 if it fails): /__e09_selftest.html has a button covered by a fixed box, a label clipped by an
overflow:hidden box and a plain button; the measurement must find exactly the first two.
Usage: python e09_layout_probe.py --out <result.tsv> [--md <report.md>] [--tree <repo root>] [--screen 1920x1032] [--zoom 110]
       [--windows id1,id2] [--settle-ms 1200] [--dbg-port 9342]
       python e09_layout_probe.py --dry [--tree ...]      (no Edge: parse the WINDOWS table and print the plan)
Exit 0 = every window measured; 1 = Edge / a window failed (rows so far still written); 2 = the self-test failed."""
import argparse, io, json, os, re, sys, threading, time
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
TAB, NL = chr(9), chr(10)


# ------------------------------------------------------------------------------------------------ the WINDOWS table (dry)
def windows_of(tree):
    bg = io.open(os.path.join(tree, 'web', 'background.html'), encoding='utf-8').read()
    s = bg.index('var WINDOWS=[')
    e = bg.index('];', s)
    out = []
    for m in re.finditer(r"\{id:'([^']+)'(.*?)\}", bg[s:e]):
        src = re.search(r"src:'([^']*)'", m.group(2))
        out.append((m.group(1), src.group(1) if src else '', 'hidden:true' in m.group(2)))
    return out


# ------------------------------------------------------------------------------------------------ stub server
SELFTEST_PAGE = r"""<!DOCTYPE html><html><head><meta charset="utf-8"><title>e09 selftest</title>
<style>body{margin:0;width:600px;height:400px;position:relative}</style></head><body>
<button id="e09Covered" style="position:absolute;left:20px;top:20px;width:100px;height:30px">covered</button>
<div id="e09Cover" class="tipBox" style="position:fixed;left:10px;top:10px;width:150px;height:60px;background:#ffd;z-index:50">tip</div>
<div id="e09Box" style="position:absolute;left:20px;top:120px;width:120px;height:40px;overflow:hidden">
  <label id="e09Clipped" style="position:absolute;left:200px;top:5px;width:80px;height:20px">clipped</label></div>
<button id="e09Fine" style="position:absolute;left:300px;top:20px;width:100px;height:30px">fine</button>
<div id="e09Pane" style="position:absolute;left:300px;top:120px;width:250px;height:100px;overflow:hidden">
  <div id="e09Scroll" style="position:absolute;left:0px;top:0px;width:120px;height:60px;overflow:auto">
    <input id="e09Scrolled" style="position:absolute;left:150px;top:10px;width:60px;height:20px"></div>
  <div id="e09Beside" style="position:absolute;left:125px;top:0px;width:120px;height:90px;background:#eef"></div></div>
</body></html>"""


def make_server(tree):
    from c12_click_probe import ws_send, ws_recv, WS_GUID     # the c12 stub's WebSocket framing (same directory)
    import base64, hashlib

    class H(SimpleHTTPRequestHandler):
        protocol_version = 'HTTP/1.1'

        def __init__(self, *a, **k):
            super().__init__(*a, directory=os.path.join(tree, 'web'), **k)

        def log_message(self, *a):
            pass

        def _send(self, code, ctype, body):
            self.send_response(code)
            self.send_header('Content-Type', ctype)
            self.send_header('Content-Length', str(len(body)))
            self.send_header('Cache-Control', 'no-store')
            self.end_headers()
            self.wfile.write(body)

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
                if isinstance(m, dict) and 'id' in m:
                    ws_send(self.wfile, json.dumps({'type': 'ack', 'id': m['id'], 'ok': False, 'error': 'e09 probe stub'}))
            self.close_connection = True

        def do_GET(self):
            p = self.path.split('?', 1)[0]
            if self.headers.get('Upgrade', '').lower() == 'websocket':
                return self._ws()
            if p == '/__e09_selftest.html':
                return self._send(200, 'text/html; charset=utf-8', SELFTEST_PAGE.encode('utf-8'))
            if p.startswith('/api/'):
                return self._send(404, 'application/json; charset=utf-8', b'{"ok":false,"error":"e09 probe stub"}')
            return super().do_GET()

        def do_POST(self):
            n = int(self.headers.get('Content-Length') or 0)
            if n:
                self.rfile.read(n)
            self._send(404, 'application/json; charset=utf-8', b'{"ok":false,"error":"e09 probe stub"}')

        do_PUT = do_POST

    srv = ThreadingHTTPServer(('127.0.0.1', 0), H)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv


# Injected before any page script, in every frame (Page.addScriptToEvaluateOnNewDocument).  %PORT% = the stub's port.
PRELUDE = r"""
(function(){
  var P = '%PORT%';
  function toStub(u){ try { var x = new URL(String(u), location.href); if (x.hostname !== '127.0.0.1' || x.port !== P) { x.hostname = '127.0.0.1'; x.port = P; }
                         return x.href; } catch(e){ return u; } }
  var WS = window.WebSocket;
  function W(u, p){ return p === undefined ? new WS(toStub(String(u).replace(/^http/, 'ws'))) : new WS(toStub(String(u).replace(/^http/, 'ws')), p); }
  W.prototype = WS.prototype; W.CONNECTING = 0; W.OPEN = 1; W.CLOSING = 2; W.CLOSED = 3; window.WebSocket = W;
  var F = window.fetch; if (F) window.fetch = function(u, o){ return F(toStub(u), o); };
  var XO = XMLHttpRequest.prototype.open; XMLHttpRequest.prototype.open = function(m, u){ var a = Array.prototype.slice.call(arguments); a[1] = toStub(u); return XO.apply(this, a); };
  window.alert = function(){}; window.confirm = function(){ return true; }; window.prompt = function(t, d){ return d || ''; };
  window.open = function(){ return null; };
})();
"""

# __e09doc(doc, win, sheet): one document's controls -- clipped by an overflow:hidden / clip ancestor, or covered at their centre.
DOC_JS = r"""
window.__e09doc = function(d, w, sheet, acc){
  function desc(el){ if(!el) return '?'; var s = el.tagName.toLowerCase(); if(el.id) s += '#' + el.id;
    var c = (typeof el.className === 'string' ? el.className : '').trim().split(/\s+/)[0]; if(c) s += '.' + c;
    var t = (el.textContent || el.value || '').trim().replace(/\s+/g, ' ').slice(0, 16); if(t) s += '"' + t + '"'; return s; }
  function key(el){ return el.id ? '#' + el.id : desc(el) + '@' + Math.round(el.getBoundingClientRect().left) + ',' + Math.round(el.getBoundingClientRect().top); }
  var vw = d.documentElement.clientWidth, vh = d.documentElement.clientHeight;
  var els = d.querySelectorAll('button, input, select, textarea, a[href], [onclick], label, .lb, .btn, .spb');
  for (var i = 0; i < els.length; i++){
    var el = els[i];
    if(!el.getClientRects().length) continue;
    var st = w.getComputedStyle(el); if(st.visibility !== 'visible' || st.display === 'none' || parseFloat(st.opacity) === 0) continue;
    if(el.type === 'hidden') continue;
    var r = el.getBoundingClientRect(); if(r.width < 2 || r.height < 2) continue;
    var k = key(el); if(acc.seen[k]) continue; acc.seen[k] = 1; acc.n++;
    // AI(W906-ST02-E09) 20261004 (St02-E): v2 -- a scroll container (overflow auto / scroll, e.g. a golden TScrollBox) makes what
    //   it holds reachable: the box being tested climbs to the scroll container and the walk goes on from there (v1 reported
    //   HW.teach's scrlbxInArmXY / HW.OmronEJ1N's ScrollBox1 content as clipped by the tab pane further up -- not reachable is
    //   only "leaves an overflow hidden / clip ancestor before any scroll container").
    var box = r, who = el, cut = null, visible = true;
    for (var p = el.parentElement; p && p !== d.body && p !== d.documentElement; p = p.parentElement){
      var ps = w.getComputedStyle(p), pr = p.getBoundingClientRect();
      var scrolls = /auto|scroll/.test(ps.overflowX + ' ' + ps.overflowY);
      var hx = /hidden|clip|auto|scroll/.test(ps.overflowX), hy = /hidden|clip|auto|scroll/.test(ps.overflowY);
      var outX = hx && (box.right > pr.right + 2 || box.left < pr.left - 2);
      var outY = hy && (box.bottom > pr.bottom + 2 || box.top < pr.top - 2);
      if(outX || outY) visible = false;                     // not on screen right now (scrolled away or cut)
      if(scrolls){ box = pr; who = p; continue; }           // reachable inside it: from here on test the scroll container itself
      if((outX || outY) && /hidden|clip/.test(ps.overflowX + ' ' + ps.overflowY)){ cut = p; break; }
    }
    if(cut){
      // AI(W906-ST02-E09) 20261005 (St02-E): v3 -- is any TEXT cut, or only the control's box (a VCL TCheckBox is often wider than
      //   its caption: Setup.SetUp grpIndexOption's 275 px boxes overhang the group by a few px)?  The text's own line boxes
      //   (Range over the control's contents) against the clipping ancestor's rect.
      var tag = ' [box only]';
      try {
        var rg = d.createRange(); rg.selectNodeContents(el); var cr = cut.getBoundingClientRect(), rs = rg.getClientRects();
        for (var q = 0; q < rs.length; q++){ var t2 = rs[q]; if(t2.width < 1) continue;
          if(t2.right > cr.right + 1 || t2.left < cr.left - 1 || t2.bottom > cr.bottom + 1 || t2.top < cr.top - 1){ tag = ' [TEXT CUT]'; break; } }
        if(!rs.length) tag = ' [no text]';
      } catch(e){ tag = ' [text ?]'; }
      var kr = cut.getBoundingClientRect();         // v3: the clipping box too, so a fix can see why it is narrower than golden's
      acc.clipped.push((sheet ? '[' + sheet + '] ' : '') + desc(el) + ' by ' + desc(cut) + (who !== el ? ' (via ' + desc(who) + ')' : '') + ' (' + Math.round(r.left) + ',' + Math.round(r.top) + ' ' + Math.round(r.width) + 'x' + Math.round(r.height) + ' in ' + Math.round(kr.left) + ',' + Math.round(kr.top) + ' ' + Math.round(kr.width) + 'x' + Math.round(kr.height) + ')' + tag);
    }
    // covered: only for a control that is on screen now (inside every clipping / scrolling ancestor and the viewport)
    var cx = r.left + r.width / 2, cy = r.top + r.height / 2;
    if(!visible || cx < 0 || cy < 0 || cx > vw || cy > vh){ acc.offview++; continue; }
    var t = d.elementFromPoint(cx, cy);
    if(t && t !== el && !el.contains(t) && !t.contains(el) && !(el.htmlFor && t.id === el.htmlFor) && !(el.labels && Array.prototype.indexOf.call(el.labels, t) >= 0))
      acc.covered.push((sheet ? '[' + sheet + '] ' : '') + desc(el) + ' under ' + desc(t));
  }
  return acc;
};
"""

# Runs in background.html: open one window, measure the frame and every tab sheet of the page, close it.
MEASURE = r"""
(async function(id, settle){
  function sleep(ms){ return new Promise(function(r){ setTimeout(r, ms); }); }
  var o = {id: id, state: '', win: '', view: '', cut: '', doc: '', scroll: '', n: 0, offview: 0, clipped: [], covered: [], wincov: [], tabs: 0, err: ''};
  var win = document.getElementById('win-' + id);
  if(!win){ o.state = 'no-window'; return o; }
  var wasHidden = win.style.display === 'none';     // start-up windows (main, the right-hand panes ...) stay open afterwards
  var opened = false;
  try { opened = openWin(id, true); } catch(e){ o.err = 'openWin: ' + e.message; }
  if(!opened || win.style.display === 'none'){ o.state = 'not-shown'; try { o.err = (ruleOf(id) || {}).reason || o.err; } catch(e){} return o; }
  var fr = win.querySelector('iframe');
  for (var i = 0; i < 60; i++){ try { if(fr && fr.contentDocument && fr.contentDocument.readyState === 'complete' && fr.contentDocument.body && fr.contentDocument.body.childElementCount) break; } catch(e){} await sleep(150); }
  await sleep(settle);
  var wrap = document.getElementById('desktopWrap'), a = wrap.getBoundingClientRect(), r = win.getBoundingClientRect();
  o.state = win.classList.contains('htFull') ? 'open-fullscreen' : 'open';
  o.win = [r.left, r.top, r.width, r.height].map(Math.round).join(',');
  o.view = [a.left, a.top, a.width, a.height].map(Math.round).join(',');
  o.cut = [a.left - r.left, a.top - r.top, r.right - a.right, r.bottom - a.bottom].map(function(v){ return Math.max(0, Math.round(v)); }).join(',');
  [0.05, 0.5, 0.95].forEach(function(fx){ [0.05, 0.5, 0.95].forEach(function(fy){
    var x = r.left + r.width * fx, y = r.top + r.height * fy;
    if(x < a.left || x > a.right || y < a.top || y > a.bottom) return;
    var t = document.elementFromPoint(x, y);
    if(t && !win.contains(t)) o.wincov.push(Math.round(fx * 100) + '%,' + Math.round(fy * 100) + '% ' + (t.id ? t.tagName.toLowerCase() + '#' + t.id : t.tagName.toLowerCase() + '.' + String(t.className).split(' ')[0]));
  }); });
  var d = null, w = null; try { d = fr && fr.contentDocument; w = fr && fr.contentWindow; } catch(e){ o.err = 'frame: ' + e.message; }
  if(d && d.documentElement && w){
    try { w.eval(window.__e09src); } catch(e){ o.err = 'inject: ' + e.message; }
    var de = d.documentElement, b = d.body, cb = w.getComputedStyle(b), ch = w.getComputedStyle(de);
    o.doc = de.scrollWidth + 'x' + de.scrollHeight + ' in ' + de.clientWidth + 'x' + de.clientHeight;
    var hid = function(v){ return /hidden|clip/.test(v); };
    var needX = de.scrollWidth > de.clientWidth + 1, needY = de.scrollHeight > de.clientHeight + 1;
    o.scroll = (needX ? (hid(cb.overflowX) || hid(ch.overflowX) ? 'X-CUT ' : 'x-scroll ') : '') + (needY ? (hid(cb.overflowY) || hid(ch.overflowY) ? 'Y-CUT' : 'y-scroll') : '');
    var acc = {seen: {}, n: 0, offview: 0, clipped: [], covered: []};
    var tabs = d.querySelectorAll('.pcTabs > .tab');
    o.tabs = tabs.length;
    if(!tabs.length){ w.__e09doc(d, w, '', acc); }
    else {
      for (var t = 0; t < tabs.length && t < 40; t++){
        var tb = tabs[t]; if(tb.style.display === 'none') continue;
        try { tb.click(); } catch(e){}
        await sleep(200);
        w.__e09doc(d, w, (tb.textContent || '').trim().slice(0, 14) || String(t), acc);
      }
      try { tabs[0].click(); } catch(e){}
    }
    o.n = acc.n; o.offview = acc.offview; o.clipped = acc.clipped; o.covered = acc.covered;
  }
  if(wasHidden){ try { closeWin(id); } catch(e){ o.err += ' close: ' + e.message; } }
  await sleep(150);
  return o;
})(%ID%, %SETTLE%)
"""


def selftest(cdp, port):
    cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/__e09_selftest.html' % port})
    for _ in range(40):
        try:
            if cdp.eval('document.readyState') == 'complete':
                break
        except Exception:
            pass
        time.sleep(0.25)
    cdp.eval(DOC_JS)
    acc = cdp.eval("JSON.stringify(window.__e09doc(document, window, '', {seen:{}, n:0, offview:0, clipped:[], covered:[]}))")
    acc = json.loads(acc or '{}')
    cov = ' '.join(acc.get('covered', []))
    clip = ' '.join(acc.get('clipped', []))
    ok = ('#e09Covered' in cov and '#e09Cover' in cov and '#e09Clipped' in clip and '#e09Fine' not in cov + clip
          and '[TEXT CUT]' in clip                              # v3: the self-test label's text lies outside its overflow:hidden box
          and '#e09Scrolled' not in cov + clip                 # v2: scrolled out of a scroll box, a pane beside it -- neither
          and len(acc.get('covered', [])) == 1 and len(acc.get('clipped', [])) == 1)
    print('SELFTEST %s  covered=%s  clipped=%s' % ('PASS' if ok else 'FAIL', acc.get('covered'), acc.get('clipped')))
    return ok


COLS = ['window', 'page', 'hidden', 'state', 'win', 'view', 'cut', 'doc', 'scroll', 'tabs', 'controls', 'offview',
        'n_clipped', 'clipped', 'n_covered', 'covered', 'wincov', 'err']


def write_md(rows, path, screen, zoom, tree):
    bad = [r for r in rows if r['state'].startswith('open') and (r['cut'] not in ('', '0,0,0,0') or 'CUT' in r['scroll']
           or r['n_clipped'] != '0' or r['n_covered'] != '0' or r['wincov'])]
    L = ['# E-09 機台畫面：被遮住或被截斷的視窗／頁面（版面實測）', '',
         '> 量測：`tools/webprobe/e09_layout_probe.py`，無頭 Edge 開 background.html，視窗大小 %s、畫面縮放 %s%%（main.html 🔍）。'
         '每個視窗用 openWin(id, true) 開一次、每個分頁都切過，量完關掉。假伺服器，碰不到 wb_serve 或機台檔。' % (screen, zoom),
         '> 樹：`%s`。' % tree, '',
         '## 有問題的視窗（%d／%d 個開得起來的視窗）' % (len(bad), sum(1 for r in rows if r['state'].startswith('open'))), '',
         '| 視窗 | 頁 | 框超出可見範圍（左,上,右,下 px） | 內容 vs 視窗 | 捲動 | 被裁掉的控制項 | 被蓋住的控制項 | 視窗本身被蓋 |',
         '|---|---|---|---|---|---:|---:|---|']
    for r in bad:
        L.append('| %s | %s | %s | %s | %s | %s | %s | %s |' % (r['window'], r['page'].replace('page/', ''), r['cut'], r['doc'],
                 r['scroll'] or '-', r['n_clipped'], r['n_covered'], r['wincov'] or '-'))
    L += ['', '## 明細（每個視窗最多列 12 個）', '']
    for r in bad:
        L.append('### %s（%s）' % (r['window'], r['page']))
        for k, lab in (('clipped', '被裁掉'), ('covered', '被蓋住')):
            items = [x for x in r[k].split(' ; ') if x]
            if items:
                L.append('- %s %d：' % (lab, len(items)) + '；'.join(items[:12]) + ('…' if len(items) > 12 else ''))
        if r['wincov']:
            L.append('- 視窗框的取樣點看到別的東西：' + r['wincov'])
        L.append('')
    other = [r for r in rows if not r['state'].startswith('open')]
    if other:
        L += ['## 沒開起來的視窗', ''] + ['- %s（%s）：%s %s' % (r['window'], r['page'], r['state'], r['err']) for r in other] + ['']
    io.open(path, 'w', encoding='utf-8', newline=NL).write(NL.join(L) + NL)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out', default='')
    ap.add_argument('--md', default='')
    ap.add_argument('--tree', default=os.path.normpath(os.path.join(HERE, '..', '..', '..')))
    ap.add_argument('--screen', default='1920x1032')
    ap.add_argument('--zoom', type=int, default=110)
    ap.add_argument('--windows', default='')
    ap.add_argument('--settle-ms', type=int, default=1200)
    ap.add_argument('--dbg-port', type=int, default=9342)
    ap.add_argument('--dry', action='store_true')
    a = ap.parse_args()
    wins = windows_of(a.tree)
    only = set(x for x in a.windows.split(',') if x)
    if only:
        wins = [w for w in wins if w[0] in only]
    sw, sh = [int(x) for x in a.screen.lower().split('x')]
    if a.dry:
        print('DRY: %d windows from web/background.html; screen %dx%d, zoom %d%%; nothing launched' % (len(wins), sw, sh, a.zoom))
        for wid, src, hidden in wins:
            print('  %-14s %-40s %s' % (wid, src, 'hidden' if hidden else 'start-up'))
        for js in (DOC_JS, MEASURE, PRELUDE):
            assert js.count('(') == js.count(')') and js.count('{') == js.count('}') and js.count('[') == js.count(']')
        print('JS bracket balance OK (DOC_JS / MEASURE / PRELUDE)')
        return
    if not a.out:
        raise SystemExit('--out is required (or use --dry)')
    from s12_form_probe import Cdp, launch_edge      # Edge only from here on
    srv = make_server(a.tree)
    port = srv.server_address[1]
    proc, prof, wsurl = launch_edge(a.dbg_port)
    rows, bad = [], 0
    try:
        cdp = Cdp(wsurl)
        cdp.call('Page.enable')
        cdp.call('Emulation.setDeviceMetricsOverride', {'width': sw, 'height': sh, 'deviceScaleFactor': 1, 'mobile': False})
        cdp.call('Page.addScriptToEvaluateOnNewDocument', {'source': PRELUDE.replace('%PORT%', str(port))})
        if not selftest(cdp, port):
            print('the instrument does not read the self-test page correctly -- E-09 NOT measured')
            sys.exit(2)
        cdp.call('Page.navigate', {'url': 'http://127.0.0.1:%d/background.html?zoom=%d' % (port, a.zoom)})
        for _ in range(120):
            try:
                if cdp.eval('document.readyState') == 'complete':
                    break
            except Exception:
                pass
            time.sleep(0.25)
        time.sleep(4.0)                                   # the start-up windows load their pages
        cdp.eval('window.__e09src = %s; eval(window.__e09src); 1' % json.dumps(DOC_JS))
        for wid, src, hidden in wins:
            try:
                r = cdp.eval(MEASURE.replace('%ID%', json.dumps(wid)).replace('%SETTLE%', str(a.settle_ms)), timeout=120) or {}
            except Exception as e:
                bad += 1
                r = {'state': 'probe-error', 'err': str(e)[:160]}
            row = {'window': wid, 'page': src, 'hidden': '1' if hidden else '', 'state': r.get('state', ''), 'win': r.get('win', ''),
                   'view': r.get('view', ''), 'cut': r.get('cut', ''), 'doc': r.get('doc', ''), 'scroll': r.get('scroll', ''),
                   'tabs': str(r.get('tabs', '')), 'controls': str(r.get('n', '')), 'offview': str(r.get('offview', '')),
                   'n_clipped': str(len(r.get('clipped', []))), 'clipped': ' ; '.join(r.get('clipped', [])),
                   'n_covered': str(len(r.get('covered', []))), 'covered': ' ; '.join(r.get('covered', [])),
                   'wincov': ' ; '.join(r.get('wincov', [])), 'err': r.get('err', '')}
            rows.append(row)
            print('%-14s %-12s cut=%-12s clipped=%-3s covered=%-3s %s' % (wid, row['state'], row['cut'], row['n_clipped'], row['n_covered'], row['scroll']))
    finally:
        try:
            proc.kill()
        except Exception:
            pass
        clean = lambda v: str(v).replace(TAB, ' ').replace(NL, ' ')
        io.open(a.out, 'w', encoding='utf-8', newline=NL).write(
            TAB.join(COLS) + NL + NL.join(TAB.join(clean(r.get(c, '')) for c in COLS) for r in rows) + NL)
        print('wrote %s (%d windows)' % (a.out, len(rows)))
        if a.md and rows:
            write_md(rows, a.md, a.screen, a.zoom, a.tree)
            print('wrote %s' % a.md)
    sys.exit(1 if bad else 0)


if __name__ == '__main__':
    main()
