// AI(W906-LI9-F1) 20261002 (St02-E helper): offline selftest for card LI-9 F1's two page scripts (ctest LI9_FtpClientPage).
//   web/page/ht9045_lotinfo_ftp.js  -- the Lot Info FTP tab buttons -> WS act.lotInfoFtp.open {tag} / state; opened:true asks the
//                                      frame to open the 'ftpclient' window; a refusal (JSON in the ack error) is shown, not resent.
//   web/page/ht9045_ftpclient.js    -- Data.FTPClient.html: nothing sent until the frame says the window is open (HT_WIN), then
//                                      state; draws the reply state; filter 200 ms after the last key; double click = pick; Load
//                                      from HD; HT_WIN open:false while open = close; closeMe when C++ closed the dialog; one
//                                      request at a time; busy: is a note, not resent.
//   The REAL scripts run in a node vm with a fake DOM, a fake HT9045Recipe and a fake clock (setTimeout / setInterval / Date.now).
//   Data.FTPClient.html is read only (ids + load order).  No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/li9_ftpclient_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_LI9_FTPCLIENT_JS / W906_LI9_LOTINFO_FTP_JS = an empty file.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
function readOr(p) { try { return fs.readFileSync(p, 'utf8'); } catch (e) { return ''; } }
const SRC_FC = process.env.W906_LI9_FTPCLIENT_JS || path.join(PAGE, 'ht9045_ftpclient.js');
const SRC_LI = process.env.W906_LI9_LOTINFO_FTP_JS || path.join(PAGE, 'ht9045_lotinfo_ftp.js');
const codeFC = readOr(SRC_FC), codeLI = readOr(SRC_LI);
const html = readOr(path.join(PAGE, 'Data.FTPClient.html'));
const lotHtml = readOr(path.join(PAGE, 'Data.LotInfo.html'));

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock
let clock = 1000000, timers = [], tid = 1;
function addTimer(fn, ms, every) { const id = tid++; timers.push({ id, due: clock + Math.max(0, ms | 0), fn, every: every ? Math.max(1, ms | 0) : 0 }); return id; }
function clearTimer(id) { timers = timers.filter((t) => t.id !== id); }
async function settle() { for (let i = 0; i < 16; i++) await new Promise((r) => setImmediate(r)); }
async function advance(ms) {
  const end = clock + ms;
  await settle();
  for (;;) {
    timers.sort((a, b) => a.due - b.due || a.id - b.id);
    const t = timers[0];
    if (!t || t.due > end) break;
    clock = t.due;
    if (t.every) t.due += t.every; else timers.shift();
    t.fn();
    await settle();
  }
  clock = end;
  await settle();
}
const FakeDate = { now: () => clock };

// ---------------------------------------------------------------- fake DOM
function makeDom() {
  const byId = {}, byName = {};
  class El {
    constructor(tag, id) {
      this.tagName = tag; this.children = []; this.parentNode = null; this.style = {}; this.attrs = {}; this.listeners = {};
      this._text = ''; this.className = ''; this.disabled = false; this.hidden = false; this.readOnly = false; this.value = '';
      this.checked = false; this.scrollTop = 0; this.scrollHeight = 0;
      if (id) { this.id = id; byId[id] = this; }
    }
    get firstChild() { return this.children[0] || null; }
    get nextSibling() { const p = this.parentNode; if (!p) return null; const i = p.children.indexOf(this); return p.children[i + 1] || null; }
    get textContent() { return this.children.length ? this.children.map((c) => c.textContent).join('') : this._text; }
    set textContent(v) { this.children = []; this._text = String(v); }
    appendChild(c) { if (c.parentNode) c.parentNode.removeChild(c); c.parentNode = this; this.children.push(c); if (c.id) byId[c.id] = c; return c; }
    removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; return c; }
    setAttribute(k, v) { this.attrs[k] = String(v); }
    getAttribute(k) { return k in this.attrs ? this.attrs[k] : null; }
    addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); }
  }
  function fire(target, type, props) {
    const ev = Object.assign({ type, target, isTrusted: true, preventDefault() {}, stopPropagation() { this.stopped = true; } }, props || {});
    for (let n = target; n && !ev.stopped; n = n.parentNode) (n.listeners[type] || []).slice().forEach((f) => { ev.currentTarget = n; f(ev); });
    return ev;
  }
  const head = new El('HEAD');
  const document = {
    readyState: 'complete', hidden: false, head, documentElement: head, activeElement: null,
    getElementById: (id) => byId[id] || null,
    getElementsByName: (n) => byName[n] || [],
    querySelector: () => null, querySelectorAll: () => [],
    createElement: (t) => new El(t.toUpperCase()),
    addEventListener() {},
  };
  return { El, fire, byId, byName, document, body: new El('BODY') };
}

function makeWindow(dom, recipe, parentPosts) {
  const wl = {};
  const sb = {
    console: { info() {}, warn() {}, log() {}, error(...a) { console.error(...a); } },
    setTimeout: (f, ms) => addTimer(f, ms, false), clearTimeout: clearTimer,
    setInterval: (f, ms) => addTimer(f, ms, true), clearInterval: clearTimer,
    Promise, JSON, Math, Object, Array, String, Number, Error, RegExp, isNaN, parseInt, Date: FakeDate,
    HT9045Recipe: recipe,
    HT9045Busy: { is: (x) => /^busy:/.test(String((x && x.message) || x || '')), NOTE: 'BUSY-NOTE', coolMs: () => 400 },
    document: dom.document,
    addEventListener(t, f) { (wl[t] = wl[t] || []).push(f); },
    open(u) { parentPosts.push({ windowOpen: u }); },
  };
  sb.window = sb;
  sb.parent = { postMessage(m) { parentPosts.push(m); } };
  sb.__dispatch = (type, data) => (wl[type] || []).forEach((f) => f({ data }));
  return sb;
}

// a recipe client whose replies a test scripts: queue of functions (cmd, value) -> Promise
function makeRecipe() {
  const cmds = [], script = [];
  let held = [];
  const R = {
    cmds, script,
    status() { return { connected: true, holdsToken: true }; },
    keepAlive() { return Promise.resolve(); },
    rawCmd(name, extra) {
      const v = JSON.parse(extra.value);
      cmds.push({ name, v, at: clock });
      const f = script.length ? script.shift() : null;
      if (f === 'hold') return new Promise((res) => { held.push(res); });
      if (typeof f === 'function') return f(name, v);
      return Promise.resolve({ type: 'ack', ok: true, executed: true, op: name, state: null });
    },
    release(reply) { const r = held.shift(); if (r) r(reply); },
  };
  return R;
}
const ok = (o) => () => Promise.resolve(Object.assign({ type: 'ack', ok: true }, o));
const refuse = (o) => () => Promise.reject(new Error(JSON.stringify(o)));
const errText = (t) => () => Promise.reject(new Error(t));

function stateHD(items, open, extra) {
  return Object.assign({
    open: open, page: open ? 'hd' : '', iHD: 1, barcodeKeys: false,
    tabs: [{ name: 'TabSheet1', visible: false }, { name: 'TabSheet2', visible: false }, { name: 'TabSheet3', visible: true }, { name: 'TabSheet4', visible: false }],
    hd: { edtHDWaferName: { visible: true, enabled: true, text: '' }, lstHDFile: { visible: true, enabled: true, itemIndex: -1, items: items },
          plLoad: { visible: true, enabled: true, caption: 'Load from HD' }, plUnload: { visible: true, enabled: true, caption: 'Upload to Server' },
          plUnloadALL: { visible: false, enabled: true }, labN06_DownloadPath1: { visible: false } },
    server: {}, tester: { rgInputMethod: { itemIndex: 0, enabled: true }, grpTesterMap: { visible: true }, grpTesterName: { enabled: false } },
    memo: ['', 'Process -- Close-----------'],
    lotInfo: { buttons: [] },
  }, extra || {});
}
// POOL-14 MR-A (AI(W906-P14) 20261010 (St02-E)): the Server page as WebLotInfoFtp_St02.cpp writes it
function stateServer(items, text) {
  return stateHD([], true, {
    page: 'server', iHD: 0,
    tabs: [{ name: 'TabSheet1', visible: false }, { name: 'TabSheet2', visible: true }, { name: 'TabSheet3', visible: false }, { name: 'TabSheet4', visible: false }],
    server: { edtServerWaferName: { visible: true, enabled: true, text: text || '' }, lstServerFile: { visible: true, enabled: true, itemIndex: -1, items: items },
              plSLoad: { visible: true, enabled: true, caption: 'Download to Handler' }, Panel15: { visible: true, enabled: true, caption: 'Copy File Name' },
              Panel16: { visible: false, enabled: true, caption: 'Clean File Name' }, downloadPorted: false },
  });
}

(async () => {
  console.log('li9_ftpclient_selftest -- ' + SRC_FC + ' / ' + SRC_LI);

  // ======================================================== [0] html
  console.log('[0] Data.FTPClient.html (read only)');
  ['PageControl1', 'tab_TabSheet2', 'tab_TabSheet3', 'tab_TabSheet4', 'TabSheet2', 'TabSheet3', 'TabSheet4', 'edtHDWaferName', 'lstHDFile', 'plLoad',
    'plUnload', 'cbTesterType', 'cbTesterID', 'cbTasterIp', 'edTesterName', 'btSafeTasterName', 'Button3', 'memoFTP', 'sbFTPStatus', 'ftpClosed']
    .forEach((id) => check(html.indexOf('id="' + id + '"') >= 0, 'html has #' + id));
  const iRc = html.indexOf('<script src="ht9045_recipe_client.js">'), iFc = html.indexOf('<script src="ht9045_ftpclient.js">');
  check(iRc >= 0 && iFc > iRc, 'ht9045_ftpclient.js loads after ht9045_recipe_client.js');
  if (lotHtml.indexOf('ht9045_lotinfo_ftp.js') >= 0) {
    ['btnFtpServer', 'btnFtpHD', 'btnFtpTester', 'btnDataFTPSaveToData', 'lotFtpStatus'].forEach((id) => check(lotHtml.indexOf('id="' + id + '"') >= 0, 'Data.LotInfo.html has #' + id));
  } else {
    console.log('  (Data.LotInfo.html:130-133 claim not applied yet: the Lot Info ids are not checked)');
  }

  // ======================================================== [1] ht9045_lotinfo_ftp.js
  console.log('[1] ht9045_lotinfo_ftp.js');
  {
    const dom = makeDom(), R = makeRecipe(), posts = [];
    const pane = dom.body.appendChild(new dom.El('DIV'));
    pane.setAttribute('data-pane', 'ftp');
    const btn = {};
    [['btnFtpServer', 0], ['btnDataFTPSaveToData', 3], ['btnFtpHD', 1], ['btnFtpTester', 2], ['btnFTPTryConnect', 4]].forEach(([id, t]) => {
      btn[id] = pane.appendChild(new dom.El('BUTTON', id)); btn[id].setAttribute('data-tag', String(t));
    });
    pane.appendChild(new dom.El('SPAN', 'lbFTPStatus'));
    const status = pane.appendChild(new dom.El('DIV', 'lotFtpStatus'));
    const sb = makeWindow(dom, R, posts);
    vm.createContext(sb);
    const lotButtons = [
      { id: 'btnFtpServer', tag: 0, caption: 'Server', visible: true, enabled: false },
      { id: 'btnDataFTPSaveToData', tag: 3, caption: 'DataFTP Save  to  Data', visible: false, enabled: true },
      { id: 'btnFtpHD', tag: 1, caption: 'HD', visible: true, enabled: true },
      { id: 'btnFtpTester', tag: 2, caption: 'Tester Name', visible: true, enabled: true },
      { id: 'btnFTPTryConnect', tag: 4, caption: 'Connection test', visible: false, enabled: true }];
    R.script.push(ok({ executed: true, op: 'state', state: { open: false, lotInfo: { buttons: lotButtons, lbFTPStatus: { visible: false, caption: 'Status' } } } }));
    let err = null;
    try { vm.runInContext(codeLI, sb, { filename: 'ht9045_lotinfo_ftp.js' }); } catch (e) { err = e; }
    check(!err && codeLI.length > 0, 'loads' + (err ? ' (' + err.message + ')' : ''));
    await settle();
    const states = () => R.cmds.filter((c) => c.name === 'act.lotInfoFtp.state');
    check(states().length === 1, 'the FTP pane is shown: one state read at once');
    await settle();
    check(btn.btnFtpServer.disabled === true && btn.btnFtpHD.disabled === false && btn.btnDataFTPSaveToData.hidden === true &&
          btn.btnFtpHD.textContent === 'HD', 'buttons drawn from state.lotInfo (enabled / visible / caption)');
    R.script.push(ok({ executed: true, op: 'open', opened: true, state: { open: true, lotInfo: { buttons: lotButtons } } }));
    dom.fire(btn.btnFtpHD, 'click');
    await settle();
    const opens = () => R.cmds.filter((c) => c.name === 'act.lotInfoFtp.open');
    check(opens().length === 1 && opens()[0].v.tag === 1, 'HD click -> act.lotInfoFtp.open {tag:1}');
    check(posts.some((m) => m.open === 'ftpclient'), 'opened:true -> the frame is asked to open the ftpclient window');
    btn.btnFtpServer.disabled = false;
    R.script.push(refuse({ executed: false, op: 'open', guard: 'server-list-failed', detail: 'not connected', state: { open: false, lotInfo: { buttons: lotButtons } } }));
    const nPosts = posts.length;
    dom.fire(btn.btnFtpServer, 'click');
    await settle();
    check(opens().length === 2 && opens()[1].v.tag === 0 && /列不出檔案/.test(status.textContent) && posts.length === nPosts,
          'Server click -> refusal server-list-failed shown in the status line, no window (POOL-14 MR-A)');
    dom.fire(btn.btnFtpTester, 'click', { isTrusted: false });
    await settle();
    check(opens().length === 2, 'an untrusted click sends nothing');
    btn.btnFtpTester.disabled = true;
    dom.fire(btn.btnFtpTester, 'click');
    await settle();
    check(opens().length === 2, 'a disabled button sends nothing');
    btn.btnFtpTester.disabled = false;
    R.script.push(errText('busy: same command in progress'));
    dom.fire(btn.btnFtpTester, 'click');
    await settle();
    check(opens().length === 3 && status.textContent === 'BUSY-NOTE', 'busy: -> the note, not resent');
    await settle();
    check(opens().length === 3, 'busy: not resent');
    const s0 = states().length;
    await advance(3100);
    check(states().length === s0 + 1, 'state is read again every 3 s while the pane is shown');
    pane.style.display = 'none';
    await advance(10000);
    check(states().length === s0 + 1, 'pane hidden: no more reads');
  }

  // ======================================================== [2] ht9045_ftpclient.js
  console.log('[2] ht9045_ftpclient.js');
  {
    const dom = makeDom(), R = makeRecipe(), posts = [];
    const ids = ['PageControl1', 'tab_TabSheet1', 'tab_TabSheet2', 'tab_TabSheet3', 'tab_TabSheet4', 'TabSheet2', 'TabSheet3', 'TabSheet4', 'edtServerWaferName',
      'lstServerFile', 'edtHDWaferName', 'lstHDFile', 'plLoad', 'plUnload', 'plUnloadALL', 'rowHdPaths', 'rowHdPaths1', 'rowHdPaths2', 'rowHdPaths3',
      'FTP_DownPath1', 'FTP_UpLdPath1', 'edHandlerType', 'edHandlerID', 'grpTesterMap', 'cbTesterType', 'cbTesterID', 'dl_cbTesterType', 'dl_cbTesterID',
      'cbTasterIp', 'grpTesterName', 'edTesterName', 'btSafeTasterName', 'Button3', 'ftpClosed', 'memoFTP', 'sbFTPStatus',
      'plSLoad', 'Panel15', 'Panel16'];                                 // POOL-14 MR-A: the Server page's buttons
    const E = {};
    ids.forEach((id) => { E[id] = dom.body.appendChild(new dom.El('DIV', id)); });
    dom.byName.rgInputMethod = [0, 1].map((v) => { const r = new dom.El('INPUT'); r.value = String(v); return r; });
    const sb = makeWindow(dom, R, posts);
    vm.createContext(sb);
    let err = null;
    try { vm.runInContext(codeFC, sb, { filename: 'ht9045_ftpclient.js' }); } catch (e) { err = e; }
    check(!err && codeFC.length > 0, 'loads' + (err ? ' (' + err.message + ')' : ''));
    await advance(1000);
    check(R.cmds.length === 0, 'hosted: nothing sent before the frame says the window is open');
    R.script.push(ok({ executed: true, op: 'state', state: stateHD(['AAA', 'BBB', 'CCC'], true) }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true, initial: true });
    await settle();
    check(R.cmds.length === 1 && R.cmds[0].name === 'act.lotInfoFtp.state', 'HT_WIN open -> act.lotInfoFtp.state');
    await settle();
    check(E.lstHDFile.children.length === 3 && E.lstHDFile.children[1].textContent === 'BBB', 'lstHDFile drawn from state.hd');
    check(/ on$/.test(E.TabSheet3.className) && E.tab_TabSheet3.hidden === false && E.tab_TabSheet2.hidden === true && E.ftpClosed.hidden === true,
          'the HD page shown, its tab visible, the others hidden');
    check(E.plUnload.disabled === false && E.plLoad.disabled === false, 'Upload to Server (F2-3) and Load from HD enabled as C++ says');
    check(E.memoFTP.value === '\nProcess -- Close-----------', 'memoFTP drawn');

    E.edtHDWaferName.value = 'bbb';
    R.script.push(ok({ executed: true, op: 'filter', state: stateHD(['BBB'], true) }));
    dom.fire(E.edtHDWaferName, 'input');
    await advance(150);
    check(R.cmds.length === 1, 'typing: nothing sent within 200 ms');
    await advance(100);
    check(R.cmds.length === 2 && R.cmds[1].name === 'act.lotInfoFtp.filter' && R.cmds[1].v.text === 'bbb' && R.cmds[1].v.edit === 'hd',
          'typing: filter {edit:hd, text} 200 ms after the last key');
    await settle();
    check(E.lstHDFile.children.length === 1, 'the filtered list drawn');

    R.script.push(ok({ executed: true, op: 'pick', state: stateHD(['BBB'], true) }));
    dom.fire(E.lstHDFile.children[0], 'dblclick');
    await settle();
    check(R.cmds.length === 3 && R.cmds[2].name === 'act.lotInfoFtp.pick' && R.cmds[2].v.index === 0 && R.cmds[2].v.name === 'BBB',
          'double click -> pick {list:hd, index, name}');

    R.script.push('hold');
    dom.fire(E.plLoad, 'click');
    await settle();
    dom.fire(E.plLoad, 'click');
    await settle();
    check(R.cmds.length === 4 && R.cmds[3].name === 'act.lotInfoFtp.loadHD', 'Load from HD -> loadHD; a second click waits (one request at a time)');
    R.script.push(ok({ executed: true, op: 'loadHD', state: stateHD([], false) }));
    R.release(Object.assign({ type: 'ack', ok: true }, { executed: true, op: 'loadHD', state: stateHD(['BBB'], true) }));
    await settle();
    check(R.cmds.length === 5 && R.cmds[4].name === 'act.lotInfoFtp.loadHD', 'the queued click is sent after the first reply');
    await settle();
    check(posts.some((m) => m.closeMe === true) && E.ftpClosed.hidden === false, 'state.open false after an op -> closeMe to the frame, the closed note');

    R.script.push(ok({ executed: true, op: 'state', state: stateHD(['AAA'], true) }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    const nBefore = R.cmds.length;
    R.script.push(refuse({ executed: false, op: 'close', guard: 'cannot-close', detail: 'bCanExit is false', state: stateHD(['AAA'], true) }));
    const nPosts = posts.length;
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    await settle();
    check(R.cmds.length === nBefore + 1 && R.cmds[nBefore].name === 'act.lotInfoFtp.close', 'HT_WIN open:false while open -> close');
    await settle();
    check(posts.slice(nPosts).some((m) => m.open === 'ftpclient') && E.sbFTPStatus.className === 'bad' && /cannot-close/.test(E.sbFTPStatus.textContent),
          'C++ kept it open (golden FormCloseQuery) -> the window is opened again, the refusal shown');

    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    R.script.push(errText('busy: same command in progress'));
    const nB = R.cmds.length;
    dom.fire(E.Button3, 'click');
    await settle();
    check(R.cmds.length === nB + 1 && R.cmds[nB].name === 'act.lotInfoFtp.exit' && E.sbFTPStatus.textContent === 'BUSY-NOTE',
          'Exit -> exit; busy: -> the note, not resent');
    await advance(2000);
    check(R.cmds.length === nB + 1, 'busy: not resent later either');
    dom.fire(E.memoFTP, 'dblclick', { isTrusted: false });
    await settle();
    check(R.cmds.length === nB + 1, 'an untrusted double click on the memo sends nothing');

    // F2-3 (AI(W906-W202) 20261009 (St02-E)): Upload to Server -> act.lotInfoFtp.upload; C++'s plUnload.enabled decides the button
    R.script.push(ok({ executed: true, op: 'state', state: stateHD(['BBB'], true) }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    const nU = R.cmds.length;
    R.script.push(ok({ executed: true, op: 'upload', messages: [{ s1: 'Upload done', s2: '' }], state: stateHD(['BBB'], true) }));
    dom.fire(E.plUnload, 'click');
    await settle();
    check(R.cmds.length === nU + 1 && R.cmds[nU].name === 'act.lotInfoFtp.upload', 'Upload to Server -> act.lotInfoFtp.upload (F2-3)');
    await settle();
    const off = stateHD(['BBB'], true);
    off.hd.plUnload = { visible: true, enabled: false, caption: 'Upload to Server' };
    R.script.push(ok({ executed: true, op: 'state', state: off }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    const nV = R.cmds.length;
    dom.fire(E.plUnload, 'click');
    await settle();
    check(E.plUnload.disabled === true && R.cmds.length === nV, 'C++ plUnload.enabled false -> the button is disabled and a click sends nothing');

    // POOL-14 MR-A (AI(W906-P14) 20261010 (St02-E)): the Server page (golden 913 ShowFTPModal case 0) -- filter / pick on the server
    //   list, Copy File Name, Clean File Name hidden as C++ says, Download to Handler stays disabled (MR-C)
    R.script.push(ok({ executed: true, op: 'state', state: stateServer(['a', 'c_ATC_Recipe'], '') }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    check(/ on$/.test(E.TabSheet2.className) && E.lstServerFile.children.length === 2 && E.lstServerFile.children[1].textContent === 'c_ATC_Recipe',
          'Server page: shown, lstServerFile drawn from state.server');
    check(E.plSLoad.disabled === false && E.Panel15.disabled === false && E.Panel16.hidden === true,
          'Download to Handler and Copy File Name enabled as C++ says (POOL-14 MR-C); Clean File Name hidden');
    const nS = R.cmds.length;
    E.edtServerWaferName.value = 'A';
    R.script.push(ok({ executed: true, op: 'filter', state: stateServer(['a'], 'A') }));
    dom.fire(E.edtServerWaferName, 'input');
    await advance(250);
    check(R.cmds.length === nS + 1 && R.cmds[nS].name === 'act.lotInfoFtp.filter' && R.cmds[nS].v.edit === 'server' && R.cmds[nS].v.text === 'A',
          'typing in the Server name -> filter {edit:server, text} 200 ms after the last key');
    await settle();
    R.script.push(ok({ executed: true, op: 'pick', state: stateServer(['a'], 'a') }));
    dom.fire(E.lstServerFile.children[0], 'dblclick');
    await settle();
    check(R.cmds.length === nS + 2 && R.cmds[nS + 1].name === 'act.lotInfoFtp.pick' && R.cmds[nS + 1].v.list === 'server' && R.cmds[nS + 1].v.name === 'a',
          'double click on the server list -> pick {list:server, index, name}');
    await settle();
    R.script.push(ok({ executed: true, op: 'copyName', state: stateServer(['a'], 'a') }));
    dom.fire(E.Panel15, 'click');
    await settle();
    check(R.cmds.length === nS + 3 && R.cmds[nS + 2].name === 'act.lotInfoFtp.copyName', 'Copy File Name -> copyName');
    await settle();
    R.script.push(ok({ executed: true, op: 'download', state: stateServer(['a'], '') }));
    dom.fire(E.plSLoad, 'click');
    await settle();
    check(R.cmds.length === nS + 4 && R.cmds[nS + 3].name === 'act.lotInfoFtp.download' && E.Panel16.hidden === true,
          'Download to Handler -> act.lotInfoFtp.download (POOL-14 MR-C); Clean File Name stays hidden');
    await settle();
    const offS = stateServer(['a'], 'a');
    offS.server.plSLoad = { visible: true, enabled: false, caption: 'Download to Handler' };
    R.script.push(ok({ executed: true, op: 'state', state: offS }));
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: false });
    sb.__dispatch('message', { type: 'HT_WIN', id: 'ftpclient', open: true });
    await settle();
    const nD = R.cmds.length;
    dom.fire(E.plSLoad, 'click');
    await settle();
    check(E.plSLoad.disabled === true && R.cmds.length === nD, 'C++ plSLoad.enabled false -> disabled, a click sends nothing');
  }

  console.log('li9_ftpclient_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail === 0 ? 0 : 1);
})().catch((e) => { console.error(e); process.exit(1); });
