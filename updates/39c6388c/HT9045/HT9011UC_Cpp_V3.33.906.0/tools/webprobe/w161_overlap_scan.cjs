'use strict';
/* AI(W906-ES02-W161) 20261008: W-161 OVERLAP (EastSun 1007 via machine cpp 0297「請派工jimmy 偵測所有畫面是否有擋到」;
 * docs/REQUEST_JIMMY_OVERLAP_20261007.md). Every page of web/page in a headless Edge, for both machines (HT9045 default and
 * ?machine=HT9050), every tab sheet of every page control activated in turn:
 *   covered   a visible control (button / input / select / textarea / check box / link / onclick) whose centre point
 *             belongs to some OTHER element (document.elementFromPoint) -- with the coverer's id / class
 *   barCover  the controls the save-result box (#ht9045WireBar, the message box of the wire engine and of the pages' own
 *             *_c.js) covers when it shows the message EastSun saw on Setup.Contact (✔ 已寫入 + ⓘ 65 個欄位… + ⚠ golden 還有
 *             沒做到的步驟 …) -- through HT9045Wire.say when the page has the engine, else a box made the engine's way
 * The viewport is the page's own form (.form: the window's client area in background.html), at most 1920x1032.
 * SAFETY: the pages are served by a stub HTTP server from <tree>/web (read only); ws:// / wss:// and the HMI ports
 * (8045 / 8046 / 8055) are blocked in the browser; alert / confirm / prompt are dismissed. Nothing of the machine is read or
 * written. Not a ctest (needs Edge).
 * SELF-TEST first (exit 2 when it fails): a page with a button under a fixed box and a plain button -- exactly the first
 * one must be found covered, and the injected box must cover a button under it.
 * Usage: node w161_overlap_scan.cjs --out <result.json> [--md <report.md>] [--tree <repo root>] [--pages A,B] [--settle 700]
 *        [--zoom 110] [--msg long|short]
 * Exit 0 = every page measured; 1 = Edge / a page failed (what was measured is still written); 2 = the self-test failed. */
const fs = require('fs'), path = require('path'), os = require('os'), cp = require('child_process'), http = require('http');

const arg = (k, d) => { const i = process.argv.indexOf('--' + k); return i > 0 ? process.argv[i + 1] : d; };
const TREE = path.resolve(arg('tree', path.join(__dirname, '..', '..', '..')));
const WEB = path.join(TREE, 'web');
const OUT = arg('out', path.join(os.tmpdir(), 'w161_overlap.json'));
const MD = arg('md', null);
const SETTLE = +arg('settle', 700);
const ONLY = arg('pages', '') ? arg('pages', '').split(',') : null;
const SCREEN = { w: 1920, h: 1032 };
// (the window of a page in background.html at the main screen's zoom: at most the desktop / zoom, its title bar off -- a page taller
//  than that scrolls inside its window, and a box fixed to the window's bottom sits on what is shown there)
const ZOOM = +arg('zoom', 110) / 100;
const CAP = { w: Math.floor(SCREEN.w / ZOOM), h: Math.floor((SCREEN.h - 30) / ZOOM) };

const LONG_MSG = '✔ 已寫入（golden DeviceForm_File）\nⓘ 65 個欄位目前權限不能改，沿用原值\n⚠ golden 還有沒做到的步驟：\n' +
  '  ContactForce_Calibration（golden :812 MessageDlg 確認後重算）\n  Contact Height 換算（golden :845）\n  SaveDeviceCSV（golden :901）\n' +
  '  ReloadRecipeAll（golden :933）\n  寫入 LOG（golden :960）\n— 重讀 —\n✔ 讀取完成（C 路，golden DeviceForm_File）：套用 412 筆清單值與 88 個元件狀態';

// (--msg short: what the operator sees of that save after W-161 -- release mode drops the developer lines, see ht9045_wire_engine.js gbSave)
const SHORT_MSG = '✔ 已寫入（golden DeviceForm_File）\nⓘ 65 個欄位目前權限不能改，沿用原值';
const MSG = arg('msg', 'long') === 'short' ? SHORT_MSG : LONG_MSG;

const SELFTEST = '<!DOCTYPE html><html><head><meta charset="utf-8"></head><body style="margin:0">' +
  '<div class="form" style="position:relative;width:900px;height:400px">' +
  '<button id="stCovered" style="position:absolute;left:20px;top:20px;width:100px;height:30px">covered</button>' +
  '<div id="stCover" style="position:fixed;left:10px;top:10px;width:150px;height:60px;background:#ffd;z-index:50">tip</div>' +
  '<button id="stFine" style="position:absolute;left:300px;top:200px;width:100px;height:30px">fine</button>' +
  '<button id="stUnderBar" style="position:absolute;left:700px;top:12px;width:100px;height:30px">bar</button>' +
  '</div></body></html>';

// ---------------------------------------------------------------------------------------------------- stub server
const MIME = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.css': 'text/css', '.json': 'application/json',
  '.png': 'image/png', '.jpg': 'image/jpeg', '.gif': 'image/gif', '.svg': 'image/svg+xml', '.bmp': 'image/bmp', '.ico': 'image/x-icon', '.woff2': 'font/woff2' };
const server = http.createServer((req, res) => {
  const u = decodeURIComponent(req.url.split('?')[0]);
  if (u === '/__w161_selftest.html') { res.writeHead(200, { 'content-type': MIME['.html'] }); return res.end(SELFTEST); }
  if (req.method !== 'GET' || u.startsWith('/api/')) { res.writeHead(404); return res.end(); }
  const f = path.resolve(WEB, '.' + u);
  if (!f.startsWith(WEB) || !fs.existsSync(f) || !fs.statSync(f).isFile()) { res.writeHead(404); return res.end(); }
  res.writeHead(200, { 'content-type': MIME[path.extname(f).toLowerCase()] || 'application/octet-stream' });
  fs.createReadStream(f).pipe(res);
});

// ---------------------------------------------------------------------------------------------------- in-page code
const MEASURE = String(function (barMode, longMsg) {
  var out = { covered: [], barCover: [], controls: 0, tabs: [], bar: null };
  var form = document.querySelector('.form') || document.body;
  function vis(el) {
    // (inside a closed <details> -- a drop-down not opened -- it is not shown; Chrome still gives it a box: counted as covered)
    var dt = el.closest && el.closest('details:not([open])');
    if (dt && !(el.closest('summary') && el.closest('summary').parentElement === dt)) return false;
    for (var p = el; p && p.nodeType === 1; p = p.parentElement) {
      var cs = getComputedStyle(p);
      if (cs.display === 'none' || cs.visibility === 'hidden' || +cs.opacity === 0) return false;
    }
    var r = el.getBoundingClientRect();
    return r.width > 2 && r.height > 2;
  }
  function name(el) {
    if (!el) return '';
    if (el.id) return '#' + el.id;
    for (var p = el.parentElement; p; p = p.parentElement) if (p.id) return el.tagName.toLowerCase() + (el.className && typeof el.className === 'string' ? '.' + el.className.split(/\s+/)[0] : '') + ' in #' + p.id;
    return el.tagName.toLowerCase();
  }
  function label(el) { var t = (el.value || el.textContent || '').replace(/\s+/g, ' ').trim(); return t.slice(0, 30); }
  // the controls: a checkbox / radio inside a <label> counts as its label (the clickable thing)
  var sel = 'button, input:not([type=hidden]), select, textarea, a[href], [onclick], .btn3d';
  var all = Array.prototype.slice.call(form.querySelectorAll(sel)).map(function (el) {
    if (el.tagName === 'INPUT' && /checkbox|radio/i.test(el.type) && el.parentElement && el.parentElement.tagName === 'LABEL') return el.parentElement;
    return el;
  }).filter(function (el, i, a) { return a.indexOf(el) === i; });
  function scan(tag) {
    var bar = document.getElementById('ht9045WireBar');
    var barOn = bar && vis(bar);
    all.forEach(function (el) {
      if (!vis(el)) return;
      var r = el.getBoundingClientRect();
      var cx = r.left + r.width / 2, cy = r.top + r.height / 2;
      if (cx < 0 || cy < 0 || cx >= innerWidth || cy >= innerHeight) return;
      // (inside a scrolled-away part of a scrolling box: not reachable here, not "covered")
      for (var p = el.parentElement; p && p !== document.body; p = p.parentElement) {
        var cs = getComputedStyle(p);
        if (/(auto|scroll|hidden|clip)/.test(cs.overflow + cs.overflowX + cs.overflowY)) {
          var pr = p.getBoundingClientRect();
          if (cx < pr.left || cx > pr.right || cy < pr.top || cy > pr.bottom) return;
        }
      }
      out.controls++;
      var hit = document.elementFromPoint(cx, cy);
      if (!hit || hit === el || el.contains(hit)) return;
      if (hit.contains(el) && getComputedStyle(el).pointerEvents === 'none') return;   // (a decoration that lets clicks through to its holder)
      var row = { tab: tag, id: name(el), text: label(el), by: name(hit), byText: label(hit) };
      if (barOn && (hit === bar || bar.contains(hit))) out.barCover.push(row); else out.covered.push(row);
    });
  }
  function tabsOf() { return Array.prototype.slice.call(document.querySelectorAll('.pcTabs > .tab')).filter(vis); }
  function each(fn) {
    var tabs = tabsOf();
    fn('');
    for (var i = 0; i < tabs.length; i++) { try { tabs[i].click(); } catch (e) {} fn(label(tabs[i]) || ('tab' + i)); }
    if (tabs.length) { try { tabs[0].click(); } catch (e) {} }
    out.tabs = tabs.map(label);
  }
  if (!barMode) { each(scan); return out; }
  // the save-result box: the engine's own say(), else one made the engine's way (the 7 bar() copies are the same)
  var made = false;
  if (window.HT9045Wire && typeof HT9045Wire.say === 'function') HT9045Wire.say(longMsg, '#ffcc66', 'sticky');
  else {
    var b = document.getElementById('ht9045WireBar');
    if (!b) {
      b = document.createElement('div'); b.id = 'ht9045WireBar'; made = true;
      b.style.cssText = 'position:fixed;left:614px;right:8px;top:8px;z-index:99999;font:12px/1.5 monospace;padding:6px 24px 6px 10px;background:#222;color:#ddd;border:1px solid #555;border-radius:4px;max-height:60vh;overflow:auto;white-space:pre-wrap;min-width:260px';
      document.body.appendChild(b);
    }
    b.style.display = ''; b.textContent = longMsg;
  }
  return new Promise(function (res) {
    setTimeout(function () {
      out.covered = [];
      each(scan);
      out.covered = [];   // (the static pass reports those)
      var bb = document.getElementById('ht9045WireBar');
      if (bb) { var r = bb.getBoundingClientRect(); out.bar = { x: Math.round(r.left), y: Math.round(r.top), w: Math.round(r.width), h: Math.round(r.height), via: made ? 'made' : 'engine', shown: vis(bb), place: bb.getAttribute('data-place') || '' }; }
      // (the controls of the first tab the box overlaps at all -- partly hidden, even when their centre is free)
      out.barOverlap = []; out.barCover = [];   // (geometric: the box lets clicks through -- pointer-events:none -- so elementFromPoint never sees it; a control whose centre is under it cannot be SEEN)
      if (bb && vis(bb)) { var q = bb.getBoundingClientRect(); all.forEach(function (el) { if (!vis(el)) return; var e = el.getBoundingClientRect(); if (e.left >= innerWidth || e.top >= innerHeight) return; if (e.right > q.left && e.left < q.right && e.bottom > q.top && e.top < q.bottom) out.barOverlap.push(name(el) + (label(el) ? '「' + label(el) + '」' : '')); var mx = (e.left + e.right) / 2, my = (e.top + e.bottom) / 2; if (mx > q.left && mx < q.right && my > q.top && my < q.bottom) out.barCover.push({ tab: '', id: name(el), text: label(el), by: '#ht9045WireBar' }); }); }
      res(out);
    }, 400);
  });
});

// ---------------------------------------------------------------------------------------------------- driver
const sleep = ms => new Promise(r => setTimeout(r, ms));
async function main() {
  await new Promise(r => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const edgeExe = ['C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'].find(p => fs.existsSync(p));
  if (!edgeExe) throw new Error('msedge.exe not found');
  const prof = fs.mkdtempSync(path.join(os.tmpdir(), 'w161_edge_'));
  const edge = cp.spawn(edgeExe, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--user-data-dir=' + prof,
    '--remote-debugging-port=0', '--window-size=' + SCREEN.w + ',' + SCREEN.h, 'about:blank'], { stdio: 'ignore' });
  const R = { msg: arg('msg', 'long'), when: new Date().toISOString(), tree: TREE, screen: SCREEN, zoom: ZOOM, cap: CAP, pages: [], selftest: null, errors: [] };
  const write = () => {
    fs.writeFileSync(OUT, JSON.stringify(R, null, 1));
    if (MD) fs.writeFileSync(MD, report(R));
  };
  try {
    let dport = 0;
    for (let i = 0; i < 150 && !dport; i++) { await sleep(100); try { dport = +fs.readFileSync(path.join(prof, 'DevToolsActivePort'), 'utf8').split('\n')[0]; } catch (e) { } }
    if (!dport) throw new Error('no DevToolsActivePort');
    const t = await (await fetch('http://127.0.0.1:' + dport + '/json/new?about:blank', { method: 'PUT' })).json();
    const ws = new WebSocket(t.webSocketDebuggerUrl);
    await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
    let id = 0; const wait = new Map(); const evs = [];
    ws.onmessage = ev => {
      const m = JSON.parse(ev.data);
      if (m.id && wait.has(m.id)) { wait.get(m.id)(m); wait.delete(m.id); return; }
      if (m.method === 'Page.javascriptDialogOpening') call('Page.handleJavaScriptDialog', { accept: false });
      if (m.method) evs.push(m.method);
    };
    const call = (method, params) => new Promise(res => { const i = ++id; wait.set(i, res); ws.send(JSON.stringify({ id: i, method, params: params || {} })); });
    await call('Page.enable'); await call('Network.enable'); await call('Runtime.enable');
    await call('Network.setBlockedURLs', { urls: ['ws://*', 'wss://*', '*:8045/*', '*:8046/*', '*:8055/*', 'http://*:8045*', 'http://*:8046*', 'http://*:8055*'] });
    const ev = async (expr, awaitP) => { const r = await call('Runtime.evaluate', { expression: expr, returnByValue: true, awaitPromise: !!awaitP }); if (r.result && r.result.exceptionDetails) throw new Error(JSON.stringify(r.result.exceptionDetails).slice(0, 300)); return r.result && r.result.result ? r.result.result.value : null; };
    const open = async url => {
      await call('Emulation.setDeviceMetricsOverride', { width: SCREEN.w, height: SCREEN.h, deviceScaleFactor: 1, mobile: false });
      evs.length = 0;
      await call('Page.navigate', { url });
      for (let i = 0; i < 100 && !evs.includes('Page.loadEventFired'); i++) await sleep(100);
      await sleep(SETTLE);
      // the window's client area = the form
      const sz = await ev('(function(){var f=document.querySelector(".form");if(!f)return null;var r=f.getBoundingClientRect();return {w:Math.ceil(r.right),h:Math.ceil(r.bottom)};})()');
      if (sz && sz.w > 50 && sz.h > 50) await call('Emulation.setDeviceMetricsOverride', { width: Math.min(CAP.w, sz.w), height: Math.min(CAP.h, sz.h), deviceScaleFactor: 1, mobile: false });
      await sleep(250);
      return sz;
    };
    const measure = async () => {
      const a = await ev('(' + MEASURE + ')(false)');
      const b = await ev('(' + MEASURE + ')(true,' + JSON.stringify(MSG) + ')', true);
      return { covered: a.covered, controls: a.controls, tabs: a.tabs, barCover: b.barCover, bar: b.bar, barOverlap: b.barOverlap };
    };
    // self-test
    await open('http://127.0.0.1:' + port + '/__w161_selftest.html');
    const st = await measure();
    R.selftest = { covered: st.covered.map(x => x.id + '<' + x.by), barCover: st.barCover.map(x => x.id) };
    const stOk = st.covered.length === 1 && st.covered[0].id === '#stCovered' && st.covered[0].by === '#stCover' && st.barCover.some(x => x.id === '#stUnderBar') && !st.barCover.some(x => x.id === '#stFine');
    R.selftest.ok = stOk;
    if (!stOk) { write(); console.log('SELF-TEST FAILED ' + JSON.stringify(R.selftest)); edge.kill(); server.close(); process.exit(2); }
    console.log('self-test ok');
    const pages = fs.readdirSync(path.join(WEB, 'page')).filter(f => /^(Setup|HW|Status|Main|Data|Alert|Config)\..*\.html$/.test(f) && (!ONLY || ONLY.includes(f.replace(/\.html$/, ''))));
    for (const f of pages) {
      for (const mach of ['', 'HT9050']) {
        const url = 'http://127.0.0.1:' + port + '/page/' + f + (mach ? '?machine=' + mach : '');
        const row = { page: f.replace(/\.html$/, ''), machine: mach || 'HT9045' };
        try {
          const sz = await open(url);
          if (!sz) { row.skip = 'no .form'; R.pages.push(row); continue; }
          row.size = sz;
          Object.assign(row, await measure());
        } catch (e) { row.error = String(e.message || e).slice(0, 300); R.errors.push(row.page + ' ' + row.machine + ': ' + row.error); }
        R.pages.push(row);
        process.stdout.write(row.page + ' ' + row.machine + ': ' + (row.error ? 'ERROR' : row.skip ? row.skip : (row.covered || []).length + ' covered, bar covers ' + (row.barCover || []).length) + '\n');
        write();
      }
    }
    write();
    edge.kill(); server.close();
    try { await sleep(400); fs.rmSync(prof, { recursive: true, force: true }); } catch (e) { }
    process.exit(R.errors.length ? 1 : 0);
  } catch (e) {
    R.errors.push(String(e.stack || e)); write();
    try { edge.kill(); } catch (x) { } server.close();
    console.log('ERROR ' + e.message); process.exit(1);
  }
}

function report(R) {
  const L = ['# W-161 畫面遮擋掃描（' + R.when + '）', '', '視窗＝各頁 .form 的大小（最大 ' + R.cap.w + 'x' + R.cap.h + ' css px＝' + R.screen.w + 'x' + R.screen.h + ' 桌面、縮放 ' + Math.round(R.zoom * 100) + '%、扣標題列）；每個分頁都切過；HT9045 與 ?machine=HT9050 各一次。', ''];
  const pages = R.pages.filter(p => !p.skip);
  const cov = pages.filter(p => (p.covered || []).length), bar = pages.filter(p => (p.barCover || []).length || (p.barOverlap || []).length);
  L.push('- 頁數：' + pages.length / 2 + '（× 2 機種）；有控制項被擋：' + cov.length + ' 次；存檔結果框會擋到控制項：' + bar.length + ' 次', '');
  L.push('## 1. 靜態：控制項被別的元素擋到', '', '| 頁 | 機種 | 分頁 | 控制項 | 被誰擋 |', '|---|---|---|---|---|');
  for (const p of cov) for (const c of p.covered) L.push('| ' + p.page + ' | ' + p.machine + ' | ' + (c.tab || '-') + ' | ' + c.id + (c.text ? '「' + c.text + '」' : '') + ' | ' + c.by + (c.byText ? '「' + c.byText + '」' : '') + ' |');
  L.push('', '## 2. 存檔結果框（#ht9045WireBar）擋到的控制項 —— 訊息：' + (R.msg === 'short' ? '修正後操作員實際看到的兩行' : 'EastSun 在 Contact 看到的那段長訊息'), '', '| 頁 | 機種 | 框的位置 | 中心被擋（點不到） | 部分壓到 |', '|---|---|---|---|---|');
  for (const p of bar) L.push('| ' + p.page + ' | ' + p.machine + ' | ' + (p.bar ? p.bar.x + ',' + p.bar.y + ' ' + p.bar.w + 'x' + p.bar.h + ' (' + p.bar.via + (p.bar.place ? ', ' + p.bar.place : '') + ')' : '-') + ' | ' + p.barCover.length + (p.barCover.length ? '：' + p.barCover.slice(0, 6).map(c => c.id + (c.text ? '「' + c.text + '」' : '')).join('、') + (p.barCover.length > 6 ? '…' : '') : '') + ' | ' + (p.barOverlap || []).length + ((p.barOverlap || []).length ? '：' + p.barOverlap.slice(0, 6).join('、') + (p.barOverlap.length > 6 ? '…' : '') : '') + ' |');
  if (R.errors.length) L.push('', '## 錯誤', '', ...R.errors.map(e => '- ' + e));
  return L.join('\n') + '\n';
}

main();
