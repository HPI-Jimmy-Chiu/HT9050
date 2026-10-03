// AI(W906-S17D) 20261003 [W906] St01: offline selftest for fix D of card S-17 (web/page/dialog-bridge.js inspectClose, page side
//   of contract 1.3.1).  C++ (tools/wb_serve.cpp W906_DialogCloseRequest) keeps the latest close at the top level of the single-slot
//   Dialog-close-request file and adds "recent": up to 8 close entries, oldest first, the latest included.  The page must:
//     - process every entry with seq > lastSeq, in order (two closes within one 100 ms poll both take effect);
//     - close a box that is still QUEUED (not shown yet) by taking it out of the queue, so it never renders;
//     - still accept an old-format file (no recent[]) = [the top-level request];
//     - ignore entries already processed (seq <= lastSeq).
//   Loads the REAL dialog-bridge.js in a node vm with a minimal fake window / document / fetch / setInterval and a fake
//   window.HTDialogHost that records what would go to C++.  No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/s17_dialog_close_selftest.cjs [<web/page dir>]
//   Control run: W906_DIALOG_BRIDGE_JS at the pre-change file -> must go red.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_DIALOG_BRIDGE_JS || path.join(PAGE, 'dialog-bridge.js');
const code = fs.readFileSync(SRC, 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }
async function flush() { for (let i = 0; i < 6; i++) await new Promise((r) => setImmediate(r)); }

// ---- a DOM just big enough for installView / render / closeView / renderNonStop / closeNonStop --------------------------------
function makeDom() {
  const byId = {};
  const iframes = {};
  function classList() {
    const set = new Set();
    return {
      add(...c) { c.forEach((x) => set.add(x)); }, remove(...c) { c.forEach((x) => set.delete(x)); },
      toggle(c, f) { const on = f === undefined ? !set.has(c) : !!f; if (on) set.add(c); else set.delete(c); return on; },
      contains(c) { return set.has(c); },
    };
  }
  function matches(el, sel) {                         // tag, .class, [data-kind="x"] -- one simple selector
    const m = /^(\w+)?(?:\.([\w-]+))?(?:\[([\w-]+)="([^"]*)"\])?$/.exec(sel);
    if (!m) return false;
    if (m[1] && el.tagName !== m[1].toUpperCase()) return false;
    if (m[2] && !el.classList.contains(m[2])) return false;
    if (m[3] && el.attrs[m[3]] !== m[4]) return false;
    return true;
  }
  class El {
    constructor(tag, attrs) {
      this.tagName = String(tag).toUpperCase(); this.attrs = attrs || {}; this.dataset = {}; this.desc = [];
      this.classList = classList(); this.style = {}; this.textContent = ''; this._cls = '';
      for (const k of Object.keys(this.attrs)) if (k.indexOf('data-') === 0) this.dataset[k.slice(5)] = this.attrs[k];
      if (this.attrs.class) this.attrs.class.split(/\s+/).forEach((c) => this.classList.add(c));
      if (this.tagName === 'IFRAME') { const self = this; this.posted = []; this.contentWindow = { postMessage(m) { self.posted.push(m); } }; }
    }
    set id(v) { this._id = v; byId[v] = this; }
    get id() { return this._id; }
    set className(v) { this._cls = v; }
    get className() { return this._cls; }
    setAttribute(k, v) { this.attrs[k] = v; }
    appendChild(c) { return c; }
    remove() {}
    addEventListener() {}
    set innerHTML(html) {
      this.desc = [];
      const re = /<(\w+)([^>]*)>/g; let m;
      while ((m = re.exec(html))) {
        const attrs = {}; const ar = /([\w-]+)="([^"]*)"/g; let a;
        while ((a = ar.exec(m[2]))) attrs[a[1]] = a[2];
        const el = new El(m[1], attrs);
        this.desc.push(el);
        if (el.tagName === 'IFRAME') iframes[attrs['data-kind']] = el;
      }
    }
    querySelectorAll(sel) { return this.desc.filter((e) => matches(e, sel)); }
    querySelector(sel) { return this.querySelectorAll(sel)[0] || null; }
  }
  const document = {
    readyState: 'complete', head: new El('head'), body: new El('body'),
    createElement(tag) { return new El(tag); },
    getElementById(id) { return byId[id] || null; },
    querySelector() { return new El('div'); },        // status() / authMsg() targets: a throwaway element is enough
    addEventListener() {},
  };
  return { document, iframes };
}

function makeBridge(opts) {
  const dom = makeDom();
  const listeners = {};
  const subs = [];                                    // what HTDialogHost.submitResponse got: {file, resp}
  const logs = [];
  const files = {
    'Alarm-dialog-request': { state: 'idle', seq: 0, requestId: '' },
    'Message-dialog-request': { state: 'idle', seq: 0, requestId: '' },
    'Dialog-close-request': { state: 'idle', seq: 0, closeRequestId: '' },
  };
  let tick = null;
  const win = {
    addEventListener(t, f) { (listeners[t] = listeners[t] || []).push(f); },
    dispatchEvent() { return true; },
    postMessage() {},
    console: { log() {}, warn: (...a) => logs.push(a.join(' ')), error: (...a) => logs.push(a.join(' ')), info() {} },
  };
  win.window = win;
  win.HTDialogHost = { submitResponse(file, resp) { subs.push({ file, resp }); return Promise.resolve('ok'); } };
  if (opts && opts.nonStopRouter) {
    win.HT9045NonStop = { route(kind, req) { return Promise.resolve({ kind: req.ns ? (kind === 'message' ? 'messageNonStop' : 'alarmNonStop') : kind }); } };
  }
  function fetch(url) {
    const m = /JSON\/([^/?]+)\.json/.exec(url);
    const name = m && m[1];
    if (!name || !(name in files)) return Promise.resolve({ ok: false, status: 404 });
    const body = JSON.stringify(files[name]);
    return Promise.resolve({ ok: true, status: 200, json() { return Promise.resolve(JSON.parse(body)); } });
  }
  class CustomEvent { constructor(type, init) { this.type = type; this.detail = init && init.detail; } }
  const ctx = {
    window: win, document: dom.document, location: { pathname: '/background.html', protocol: 'http:', search: '' },
    fetch, setInterval(f) { tick = f; return 1; }, clearInterval() {}, setTimeout, clearTimeout,
    console: win.console, CustomEvent, Promise, JSON, Object, String, Number, Date, Error, Math,
  };
  vm.createContext(ctx);
  vm.runInContext(code, ctx, { filename: SRC });
  // every dialog page says HT_DIALOG_READY (as the real iframes do on load)
  ['alarm', 'message', 'alarmNonStop', 'messageNonStop'].forEach((kind) => {
    (listeners.message || []).forEach((f) => f({ data: { type: 'HT_DIALOG_READY', kind }, source: dom.iframes[kind].contentWindow }));
  });
  const B = win.HTDialogBridge;
  return {
    B, files, subs, logs,
    async poll(n) { for (let i = 0; i < (n || 1); i++) { tick(); await flush(); } },
    rendered(kind) { return dom.iframes[kind || 'alarm'].posted.filter((m) => m.type === 'HT_DIALOG_REQUEST').map((m) => m.request.requestId); },
    activeId() { const a = B.active(); return a ? a.request.requestId : null; },
    activeNSId() { const a = B.activeNS(); return a ? a.request.requestId : null; },
    stopQueue() { return B.queues().stop.map((q) => q.requestId); },
    nsQueue() { return B.queues().nonStop.map((q) => q.requestId); },
    answers(rid) { return subs.filter((s) => /^Alarm-dialog-response/.test(s.file) && s.resp.requestId === rid); },
    closeResps(rid) { return subs.filter((s) => /^Dialog-close-response/.test(s.file) && s.resp.target && s.resp.target.requestId === rid); },
  };
}

function alarmReq(seq, rid, ns) {
  const r = { schemaVersion: '1.0.0', channel: 'show-error-message', seq, requestId: rid, state: 'pending', blocking: !ns,
              arguments: { code: 'WAR' + rid, kCode: 1 } };
  if (ns) r.ns = true;
  return r;
}
function closeEntry(seq, req, action) {
  return { closeRequestId: 'close-' + seq, seq, target: { channel: req.channel, requestId: req.requestId, requestSeq: req.seq },
           resolvedAction: action || { name: 'RETRY', code: 1 }, closeReason: 'external-io',
           trigger: { source: 'io', inputName: 'K_PAUSE', detectedAt: 'x' } };
}
function closeFile(entries, oldFormat) {                // top level = the latest entry; recent[] unless oldFormat
  const latest = entries[entries.length - 1];
  const f = Object.assign({ schemaVersion: '1.0.0', channel: 'dialog-close', state: 'pending', requestedAt: 'x', error: null }, latest);
  if (!oldFormat) f.recent = entries;
  return f;
}
// A shown, B and C queued behind it (stop queue never drops)
async function abc(h) {
  const A = alarmReq(1, '101'), Bq = alarmReq(2, '102'), C = alarmReq(3, '103');
  h.files['Alarm-dialog-request'] = A; await h.poll();
  h.files['Alarm-dialog-request'] = Bq; await h.poll();
  h.files['Alarm-dialog-request'] = C; await h.poll();
  return { A, B: Bq, C };
}

(async function main() {
  console.log('S17_DialogClosePage  source: ' + SRC);
  let h, r;

  // ---- 0. the harness itself: A shows, B and C wait -------------------------------------------------------------------------
  h = makeBridge();
  r = await abc(h);
  check(h.activeId() === '101' && h.rendered().join() === '101', '0: A (101) is shown');
  check(h.stopQueue().join() === '102,103', '0: B (102) and C (103) are queued behind it');

  // ---- 1. a close for a QUEUED stop box removes it: it never renders, the next queued one does ------------------------------
  const e10 = closeEntry(10, r.B);
  h.files['Dialog-close-request'] = closeFile([e10]);
  await h.poll();
  check(h.stopQueue().join() === '103', '1: close-10 targeting queued B removes B from the queue');
  check(h.activeId() === '101', '1: A is still the box on screen');
  await h.poll(2);
  check(h.closeResps('102').length === 1 && h.closeResps('102')[0].resp.accepted === true && h.closeResps('102')[0].resp.closeRequestId === 'close-10',
        '1: one accepted close response for B (close-10)');
  check(h.answers('102').length === 0, '1: no Alarm-dialog-response for B (C++ resolved it itself)');
  h.files['Dialog-close-request'] = closeFile([e10, closeEntry(11, r.A)]);
  await h.poll(3);
  check(h.answers('101').length === 1 && h.answers('101')[0].resp.closedBy === 'external-io', '1: close-11 closes A (closedBy external-io)');
  check(h.rendered().join() === '101,103', '1: next shown is C -- B never rendered (shown: ' + h.rendered().join() + ')');
  check(h.activeId() === '103' && h.stopQueue().length === 0, '1: C on screen, queue empty');

  // ---- 2. two closes in one file (A active + B queued) both take effect in ONE poll --------------------------------------------
  h = makeBridge();
  r = await abc(h);
  h.files['Dialog-close-request'] = closeFile([closeEntry(20, r.A), closeEntry(21, r.B)]);
  await h.poll();                                                     // exactly one poll
  check(h.answers('101').length === 1, '2: entry 1 (close-20) closed active A in that poll');
  check(h.activeId() === null, '2: A gone from the screen');
  check(h.stopQueue().join() === '103', '2: entry 2 (close-21) removed queued B in the same poll');
  await h.poll(3);
  check(h.rendered().join() === '101,103' && h.activeId() === '103', '2: then C shows; B never rendered (shown: ' + h.rendered().join() + ')');
  check(h.closeResps('101').length === 1 && h.closeResps('102').length === 1, '2: both close responses reach the host (none dropped while one is in flight)');
  const c101 = h.closeResps('101')[0], c102 = h.closeResps('102')[0];
  check(c101 && c102 && c101.resp.closeRequestId === 'close-20' && c102.resp.closeRequestId === 'close-21',
        '2: each close response carries its own closeRequestId');

  // ---- 3. an old-format close (no recent[]) still closes the active box --------------------------------------------------------
  h = makeBridge();
  r = await abc(h);
  h.files['Dialog-close-request'] = closeFile([closeEntry(5, r.A, { name: 'SKIP', code: 2 })], true);
  await h.poll();
  check(h.answers('101').length === 1 && h.answers('101')[0].resp.selectedAction.name === 'SKIP' &&
        h.answers('101')[0].resp.selectedAction.code === 2, '3: old-format close-5 closes A with C++\'s resolvedAction');
  await h.poll(2);
  check(h.activeId() === '102', '3: B shows next');

  // ---- 4. entries already processed (seq <= lastSeq) are ignored ---------------------------------------------------------------
  //   (continues from 3: lastSeq = 5, B is on screen)
  const before = h.subs.length;
  h.files['Dialog-close-request'] = closeFile([closeEntry(5, r.B), closeEntry(7, alarmReq(99, '999'))]);
  await h.poll(3);
  check(h.activeId() === '102' && h.answers('102').length === 0, '4: an old entry (seq 5 <= lastSeq) targeting B is ignored -- B stays');
  const rej = h.subs.slice(before).filter((s) => /^Dialog-close-response/.test(s.file));
  check(rej.length === 1 && rej[0].resp.closeRequestId === 'close-7' && rej[0].resp.accepted === false,
        '4: only the new entry (close-7, no such box) is processed: rejected');
  const before2 = h.subs.length;
  h.files['Dialog-close-request'] = closeFile([closeEntry(6, r.B)]);
  await h.poll(3);
  check(h.activeId() === '102' && h.subs.length === before2, '4: a whole file with seq <= lastSeq is ignored');
  h.files['Dialog-close-request'] = closeFile([closeEntry(6, r.B), closeEntry(8, r.B)]);
  await h.poll(3);
  check(h.answers('102').length === 1 && h.activeId() === '103', '4: a later new entry (close-8) for B does close it');

  // ---- 5. the non-stop layer: a queued non-stop box is removed too -----------------------------------------------------------
  h = makeBridge({ nonStopRouter: true });
  const N1 = alarmReq(1, '201', true), N2 = alarmReq(2, '202', true);
  h.files['Alarm-dialog-request'] = N1; await h.poll();
  h.files['Alarm-dialog-request'] = N2; await h.poll();
  check(h.activeNSId() === '201' && h.nsQueue().join() === '202', '5: non-stop 201 shown, 202 queued');
  h.files['Dialog-close-request'] = closeFile([closeEntry(30, N2), closeEntry(31, N1)]);
  await h.poll();
  check(h.activeNSId() === null && h.nsQueue().length === 0, '5: one poll closes shown 201 and removes queued 202');
  await h.poll(3);
  check(h.rendered('alarmNonStop').join() === '201', '5: 202 never rendered (shown: ' + h.rendered('alarmNonStop').join() + ')');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL: exception ' + (e && e.stack)); process.exit(1); });
