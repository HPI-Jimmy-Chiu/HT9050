// AI(W906-S16) 20261003 [W906] (St01 ST01-E2): ctest S16_LazyBoot -- laptop card S-16 (RULINGS_20261001 #32): D:\HT9045\web\background.html
//   loads a hidden window's page only when that window is first opened (iframe data-defer-src), so the start-up loader waits for the
//   boot-visible windows and the dialog overlays only. Offline in headless Edge, like HSys_TablePage: the page is copied into a temp
//   folder with <base href> pointing back at web\ and a probe script; WebSocket is stubbed in the frame (nothing leaves the page);
//   the probe's JSON is read from `msedge --headless --dump-dom`.
//   [A] background.html: no BOM, one EOL style; the builder has the defer rule; the loader skips data-defer-src
//   [B] after 'ht-loaded': HT_LOAD_INFO.total <= 16 (was ~72: every window + overlay), reason complete; no hidden window has a src;
//       lazy:true windows (pci1203 / eventlog / testercomm) still have none either
//   [C] openWin('teach'): the page loads, data-defer-src becomes data-base-src, and the iframe gets HT_WIN initial (postWinState(id,true));
//       closeWin('teach') only hides it (src kept, state closed)
//   [D] applyGpibMachine('9050GPIB') on the never-opened Motion View: data-defer-src becomes the 9050 page, no src / data-base-src
//       (reloadAllIframesWithMode must not preload it)
//   Control: the same probe on a copy without the defer rule in the builder must be red ([B] total).
//   Exit 77 (ctest SKIP) when Edge is not installed. Use: only through ctest (S16_LazyBoot); W906_EDGE overrides the Edge path.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');
const url = require('url');
const child = require('child_process');

const WEB_DIR = path.resolve(process.env.W906_S16_WEB_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web'));
const PAGE = 'background.html';
const RULE = ": (cfg.hidden && !cfg.eager) ? '<iframe data-defer-src=\"'+cfg.src+'\"></iframe>'";
const LOADER = "querySelectorAll('iframe:not([data-lazy-src]):not([data-defer-src])')";
let failed = 0, passed = 0;
function check(name, fn) {
  try { fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
function eq(a, b, what) { if (JSON.stringify(a) !== JSON.stringify(b)) throw new Error(what + ': got ' + JSON.stringify(a) + ', want ' + JSON.stringify(b)); }

function findEdge() {
  const c = [process.env.W906_EDGE, 'C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'];
  for (const p of c) if (p && fs.existsSync(p)) return p;
  return null;
}

// Offline: no WebSocket leaves the frame (a wb_serve on this PC must never see the probe).
const WS_STUB = String.raw`
<script>
(function () {
  function W() { var s = this; s.readyState = 0; s.send = function () {}; s.close = function () {};
    s.addEventListener = function () {}; s.removeEventListener = function () {};
    setTimeout(function () { s.readyState = 3; try { if (s.onerror) s.onerror({}); } catch (e) {} try { if (s.onclose) s.onclose({ code: 1006, reason: 'probe' }); } catch (e) {} }, 0); }
  W.CONNECTING = 0; W.OPEN = 1; W.CLOSING = 2; W.CLOSED = 3;
  window.WebSocket = W;
})();
</script>
`;

const PROBE = String.raw`
<script>
(function () {
  var out = { errors: [], tests: {} }, T = out.tests, finished = false;
  window.addEventListener('error', function (e) { out.errors.push(String(e.message) + ' @' + e.filename + ':' + e.lineno); });
  function done() { if (finished) return; finished = true; var p = document.createElement('pre'); p.id = '__result'; p.textContent = JSON.stringify(out); document.body.appendChild(p); }
  function step(name, fn) { try { fn(); } catch (e) { out.errors.push(name + ': ' + (e && e.message || e)); } }
  function fr(id) { var w = document.getElementById('win-' + id); return w ? w.querySelector('iframe') : null; }
  function attrs(f) { return f ? { src: f.getAttribute('src') || '', base: f.getAttribute('data-base-src') || '', defer: f.getAttribute('data-defer-src') || '', lazy: f.getAttribute('data-lazy-src') || '' } : null; }
  window.addEventListener('ht-loaded', function (ev) {
    step('boot', function () {
      var info = ev.detail || window.HT_LOAD_INFO || {};
      T.info = { total: info.total, loaded: info.loaded, reason: info.reason };
      var hiddenSrc = [], defer = 0, visible = [];
      document.querySelectorAll('.win').forEach(function (w) {
        var f = w.querySelector('iframe'), id = w.id.replace(/^win-/, '');
        if (!f) return;
        if (f.hasAttribute('data-defer-src')) defer++;
        if (w.style.display === 'none' && f.getAttribute('src')) hiddenSrc.push(id);
        if (w.style.display !== 'none') visible.push(id);
      });
      T.boot = { defer: defer, hiddenWithSrc: hiddenSrc, visible: visible.length,
                 lazySrc: ['pci1203', 'eventlog', 'testercomm'].map(function (id) { var f = fr(id); return f ? (f.getAttribute('src') || '') : 'none'; }) };
    });
    var calls = [];
    step('wrap', function () { var pws = window.postWinState; window.postWinState = function (id, initial) { calls.push(id + ':' + !!initial); return pws.apply(this, arguments); }; });
    var tf = fr('teach');
    if (!tf) { out.errors.push('no #win-teach iframe'); done(); return; }
    T.teachBefore = attrs(tf);
    tf.addEventListener('load', function () {
      if (T.teachLoaded) return;
      step('teachLoaded', function () {
        T.teachLoaded = attrs(tf);
        T.teachUrl = (function () { try { return String(tf.contentWindow.location.pathname).replace(/^.*\//, ''); } catch (e) { return 'n/a'; } })();
        T.initial = calls.indexOf('teach:true') >= 0;
        closeWin('teach');
        T.teachClosed = { src: tf.getAttribute('src') || '', state: (window.WIN_STATE || {}).teach || '' };
      });
      step('gpib', function () {
        var mv = fr('motionview');
        T.mvBefore = attrs(mv);
        applyGpibMachine('9050GPIB');
        T.mvAfter = attrs(mv);
      });
      setTimeout(done, 300);
    });
    step('open', function () { openWin('teach', true); T.teachOpened = attrs(tf); });
    setTimeout(function () { out.errors.push('teach did not load within 20 s'); done(); }, 20000);
  });
  setTimeout(function () { out.errors.push('no ht-loaded within 55 s'); done(); }, 55000);
})();
</script>
`;

function probe(edge, tmp, tag, htmlOverride) {
  let html = htmlOverride || fs.readFileSync(path.join(WEB_DIR, PAGE), 'utf8');
  const base = url.pathToFileURL(WEB_DIR).href.replace(/\/?$/, '/');
  ok(html.includes('<head>'), 'page has no <head>');
  html = html.replace('<head>', '<head>\n<base href="' + base + '">' + WS_STUB);
  ok(html.includes('</body>'), 'page has no </body>');
  html = html.replace('</body>', PROBE + '</body>');
  const file = path.join(tmp, tag + '.html');
  fs.writeFileSync(file, html, 'utf8');
  const r = child.spawnSync(edge, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--disable-extensions',
    '--user-data-dir=' + path.join(tmp, 'profile-' + tag), '--allow-file-access-from-files', '--virtual-time-budget=60000',
    '--dump-dom', url.pathToFileURL(file).href], { encoding: 'utf8', timeout: 150000, maxBuffer: 64 * 1024 * 1024, windowsHide: true });
  const m = /<pre id="__result">([\s\S]*?)<\/pre>/.exec(r.stdout || '');
  ok(m, tag + ': no probe result from Edge (status ' + r.status + (r.error ? ', ' + r.error.message : '') + ')');
  const txt = m[1].replace(/&quot;/g, '"').replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&amp;/g, '&');
  return JSON.parse(txt);
}

function verdicts(d) {        // [name, fn] list so the control run can tell which ones go red
  const T = d.tests;
  return [
    ['[B] the probe finished', () => eq(d.errors.filter(e => /did not load|no ht-loaded|^boot|^wrap|^open|^teachLoaded|^gpib|no #win-teach/.test(e)), [], 'probe errors')],
    ['[B] start-up waits for the visible windows + overlays only (total <= 16)', () => { ok(T.info && T.info.total <= 16 && T.info.total >= T.boot.visible, 'HT_LOAD_INFO.total ' + JSON.stringify(T.info) + ', visible ' + (T.boot && T.boot.visible)); eq(T.info.reason, 'complete', 'reason'); }],
    ['[B] no hidden window has a src at boot', () => { eq(T.boot.hiddenWithSrc, [], 'hidden windows with src'); ok(T.boot.defer >= 40, 'only ' + T.boot.defer + ' deferred iframes'); }],
    ['[B] lazy:true windows still unloaded', () => eq(T.boot.lazySrc, ['', '', ''], 'pci1203 / eventlog / testercomm src')],
    ['[C] first open loads the page and turns it into a normal window', () => {
      eq([T.teachBefore.src, T.teachBefore.defer], ['', 'page/HW.teach.html'], 'before');
      eq([T.teachLoaded.defer, T.teachLoaded.base, T.teachUrl], ['', 'page/HW.teach.html', 'HW.teach.html'], 'after load');
    }],
    ['[C] the loaded page gets HT_WIN initial', () => eq(T.initial, true, 'postWinState(teach, true) after load')],
    ['[C] close only hides it', () => { ok(T.teachClosed.src.indexOf('HW.teach.html') >= 0, 'src after close ' + T.teachClosed.src); eq(T.teachClosed.state, 'closed', 'state'); }],
    ['[D] machine.gpibModel 9050GPIB on a never-opened Motion View loads nothing', () => {
      eq([T.mvBefore.defer, T.mvBefore.src], ['page/Main.MotionView.html', ''], 'before');
      eq([T.mvAfter.defer, T.mvAfter.src, T.mvAfter.base], ['page/Main.MotionView9050.html', '', ''], 'after');
    }],
  ];
}

function main() {
  // [A] the file
  const raw = fs.readFileSync(path.join(WEB_DIR, PAGE)), page = raw.toString('utf8');
  check('[A] background.html: no BOM, one EOL style', () => {
    ok(!(raw[0] === 0xEF && raw[1] === 0xBB && raw[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (page.match(/\r\n/g) || []).length, lf = (page.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  check('[A] builder defers hidden windows; loader skips them', () => {
    ok(page.includes(RULE), 'defer rule not in the iframe builder');
    ok(page.includes(LOADER), 'loader does not skip data-defer-src');
  });

  const edge = findEdge();
  if (!edge) { console.log('SKIP no Microsoft Edge (set W906_EDGE) -- [B]..[D] need a real DOM'); process.exit(failed ? 1 : 77); }
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 's16_lazyboot_'));
  try {
    let d = null;
    check('[B] probe ran in Edge', () => { d = probe(edge, tmp, 'real'); });
    if (d) for (const [name, fn] of verdicts(d)) check(name, fn);
    if (d && d.tests.info) console.log('     boot: HT_LOAD_INFO.total ' + d.tests.info.total + ', visible windows ' + d.tests.boot.visible + ', deferred iframes ' + d.tests.boot.defer);

    // Control: without the builder rule every hidden window loads at start-up again -> [B] total must be red
    check('control: a copy without the defer rule is red', () => {
      ok(page.includes(RULE), 'rule not found (update the control)');
      const b = probe(edge, tmp, 'broken', page.replace(RULE, ''));
      const red = verdicts(b).filter(([, fn]) => { try { fn(); return false; } catch (e) { return true; } }).map(([n]) => n);
      ok(red.some(n => n.indexOf('total <= 16') >= 0), 'the copy without the rule passed the total check (red: ' + JSON.stringify(red) + ')');
      console.log('     control red: ' + red.length + ' check(s), e.g. ' + red[0] + '; its total ' + (b.tests.info && b.tests.info.total));
    });
  } finally {
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { /* Edge may still hold the profile for a moment */ }
  }
  console.log((failed ? 'FAILED ' : 'OK ') + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
}
main();
