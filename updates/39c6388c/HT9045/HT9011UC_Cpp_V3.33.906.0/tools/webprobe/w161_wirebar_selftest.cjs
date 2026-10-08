'use strict';
/* AI(W906-ES02-W161) 20261008: W-161 (EastSun 1007「不要蓋住畫面」) -- the save result on the real Setup.Contact page, in a headless
 * Edge, through the real Save button and the real wire engine; only the server is replaced (HT9045Recipe.editlistGet /
 * editlistSave answer in the page: written, 65 fields kept, two golden steps not done -- what EastSun saw):
 *   release   the box says ✔ 已寫入 + ⓘ 65 個欄位… and nothing of the developer's (no "golden 還有沒做到的步驟", no "— 重讀 —"),
 *             and hides by itself (6 s); it covers no control (theme.js picks its place)
 *   debug     (?mode=debug) the developer's lines are there and the box stays (sticky)
 * SAFETY: a stub HTTP server serves <tree>/web read-only; ws / wss and the HMI ports are blocked; nothing reaches wb_serve.
 * Usage: node w161_wirebar_selftest.cjs [--tree <repo root>]     Exit 0 = all pass, 1 = a check failed, 2 = Edge / setup. */
const fs = require('fs'), path = require('path'), os = require('os'), cp = require('child_process'), http = require('http');
const arg = (k, d) => { const i = process.argv.indexOf('--' + k); return i > 0 ? process.argv[i + 1] : d; };
const TREE = path.resolve(arg('tree', path.join(__dirname, '..', '..', '..')));
const WEB = path.join(TREE, 'web');
const MIME = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.css': 'text/css', '.json': 'application/json', '.png': 'image/png', '.bmp': 'image/bmp', '.jpg': 'image/jpeg', '.gif': 'image/gif', '.svg': 'image/svg+xml' };
const server = http.createServer((req, res) => {
  const u = decodeURIComponent(req.url.split('?')[0]);
  const f = path.resolve(WEB, '.' + u);
  if (req.method !== 'GET' || !f.startsWith(WEB) || !fs.existsSync(f) || !fs.statSync(f).isFile()) { res.writeHead(404); return res.end(); }
  res.writeHead(200, { 'content-type': MIME[path.extname(f).toLowerCase()] || 'application/octet-stream' });
  fs.createReadStream(f).pipe(res);
});
const sleep = ms => new Promise(r => setTimeout(r, ms));
// in the page: the server's answers replaced, the page read again, the real Save button pressed
const PREP = `(function(){
  window.confirm = function(){ return true; };
  window.HT9045Recipe.editlistGet = function(){ return Promise.resolve({ lists:{}, proxies:{}, mustSend:[] }); };
  window.HT9045Recipe.editlistSave = function(){ return Promise.resolve({ saved:true, ignored:new Array(65).fill('x'),
    session:{ messages:[], asked:[], todo:['ContactForce_Calibration（golden :812）','SaveDeviceCSV（golden :901）'] } }); };
  return window.HT9045Wire.reload().then(function(){ return true; });
})()`;
const BAR = `(function(){ var b=document.getElementById('ht9045WireBar'); if(!b) return null; var r=b.getBoundingClientRect();
  var shown = b.style.display !== 'none' && r.width > 0;
  var cov = [];
  if (shown) document.querySelectorAll('.form button, .form input:not([type=hidden]), .form select').forEach(function(el){
    var e = el.getBoundingClientRect(); if (e.width < 2 || e.height < 2 || e.top >= innerHeight) return;
    var mx = (e.left + e.right) / 2, my = (e.top + e.bottom) / 2;
    if (mx > r.left && mx < r.right && my > r.top && my < r.bottom) cov.push(el.id || el.tagName); });
  return { text: b.textContent.replace(/×$/, ''), shown: shown, place: b.getAttribute('data-place') || '', covers: cov }; })()`;

async function main() {
  await new Promise(r => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const edgeExe = ['C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'].find(p => fs.existsSync(p));
  if (!edgeExe) { console.log('msedge.exe not found'); process.exit(2); }
  const prof = fs.mkdtempSync(path.join(os.tmpdir(), 'w161_bar_'));
  const edge = cp.spawn(edgeExe, ['--headless=new', '--disable-gpu', '--no-first-run', '--user-data-dir=' + prof, '--remote-debugging-port=0', '--window-size=1019,912', 'about:blank'], { stdio: 'ignore' });
  let pass = 0, fail = 0;
  const ok = (c, name, x) => { if (c) pass++; else fail++; console.log((c ? 'PASS  ' : 'FAIL  ') + name + (x ? '   ' + x : '')); };
  try {
    let dport = 0;
    for (let i = 0; i < 150 && !dport; i++) { await sleep(100); try { dport = +fs.readFileSync(path.join(prof, 'DevToolsActivePort'), 'utf8').split('\n')[0]; } catch (e) { } }
    if (!dport) throw new Error('no DevToolsActivePort');
    const t = await (await fetch('http://127.0.0.1:' + dport + '/json/new?about:blank', { method: 'PUT' })).json();
    const ws = new WebSocket(t.webSocketDebuggerUrl);
    await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
    let id = 0; const wait = new Map(); const evs = [];
    ws.onmessage = ev => { const m = JSON.parse(ev.data); if (m.id && wait.has(m.id)) { wait.get(m.id)(m); wait.delete(m.id); } else if (m.method) evs.push(m.method); };
    const call = (method, params) => new Promise(res => { const i = ++id; wait.set(i, res); ws.send(JSON.stringify({ id: i, method, params: params || {} })); });
    const ev = async (expr, ap) => { const r = await call('Runtime.evaluate', { expression: expr, returnByValue: true, awaitPromise: !!ap }); if (r.result && r.result.exceptionDetails) throw new Error(JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result.result.value; };
    await call('Page.enable'); await call('Network.enable');
    await call('Network.setBlockedURLs', { urls: ['ws://*', 'wss://*', '*:8045*', '*:8046*', '*:8055*'] });
    await call('Emulation.setDeviceMetricsOverride', { width: 1019, height: 912, deviceScaleFactor: 1, mobile: false });
    for (const mode of ['release', 'debug']) {
      evs.length = 0;
      await call('Page.navigate', { url: 'http://127.0.0.1:' + port + '/page/Setup.Contact.html?mode=' + mode });
      for (let i = 0; i < 100 && !evs.includes('Page.loadEventFired'); i++) await sleep(100);
      await sleep(1200);
      if (!(await ev('!!(window.HT9045Wire && window.HT9045Recipe)'))) { ok(false, mode + ': the page has the wire engine'); continue; }
      await ev(PREP, true);
      await sleep(300);
      await ev("document.getElementById('spbSave').click(), true");
      await sleep(900);
      const a = await ev(BAR);
      await sleep(6600);
      const b = await ev(BAR);
      const dev = a && /golden 還有沒做到的步驟|— 重讀 —|讀取完成/.test(a.text);
      if (mode === 'release') {
        ok(!!a && a.shown && /✔ 已寫入/.test(a.text) && /65 個欄位/.test(a.text) && !dev, 'release: the save result says 已寫入 + 65 欄位, none of the developer lines', JSON.stringify(a && a.text));
        ok(!!b && !b.shown, 'release: the clean save result hides by itself (6 s)', JSON.stringify(b && { shown: b.shown }));
        ok(!!a && a.covers.length === 0, 'release: the box covers no control (theme.js put it at ' + (a && a.place) + ')', JSON.stringify(a && a.covers));
      } else {
        ok(!!a && a.shown && dev, 'debug: the developer lines are still there (golden 還有沒做到的步驟 / 重讀)', JSON.stringify(a && a.text.slice(0, 160)));
        ok(!!b && b.shown, 'debug: the box with the developer lines stays (sticky)');
      }
    }
  } catch (e) { console.log('ERROR ' + (e.stack || e)); fail++; }
  try { edge.kill(); } catch (e) { }
  server.close();
  await sleep(300); try { fs.rmSync(prof, { recursive: true, force: true }); } catch (e) { }
  console.log('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  process.exit(fail ? 1 : 0);
}
main();
