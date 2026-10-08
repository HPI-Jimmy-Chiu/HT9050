// AI(W906-W178) 20261008 [W906] St02-E: offline selftest for W-178 part 2 (web/page/dialog-bridge.js inspectRecent, page side of
//   contract 1.3.2).  C++ (tools/wb_dialog_mailbox.h AlarmPost) keeps the newest request at the top level of the single-slot
//   Alarm-dialog-request file and, only when 2 or more requests are outstanding (e.g. the W-175 low-yield notices of one One Cycle
//   finish, posted in one tick), adds "recent": the requests since the slot was last idle, at most 8, oldest first, the newest included.
//   The page must enqueue every entry newer than lastSeq, oldest first, in one poll -- so they show one at a time -- and must:
//     - still take an old / single file (no recent[]) as before;
//     - not enqueue a request twice when the same file is read again;
//     - skip entries it already queued (seq <= lastSeq).
//   The harness (fake window / document / fetch / setInterval, a fake HTDialogHost) is S-17D's, copied from
//   s17_dialog_close_selftest.cjs.  No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/w178_alarm_recent_selftest.cjs [<web/page dir>]
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

function alarmFile(entries, oldFormat) {               // top level = the newest; recent[] when 2+ (what AlarmPost writes)
  const latest = entries[entries.length - 1];
  const f = Object.assign({}, latest);
  if (!oldFormat && entries.length > 1) f.recent = entries.map((e) => Object.assign({}, e));
  return f;
}
async function settle(h) { await h.poll(); for (let i = 0; i < 4; i++) await flush(); }

(async function main() {
  console.log('W178_AlarmRecentPage  source: ' + SRC);
  let h;
  const A = alarmReq(1, '301'), B = alarmReq(2, '302'), C = alarmReq(3, '303');

  // ---- 0. a single request (no recent[]) works as before ---------------------------------------------------------------------
  h = makeBridge();
  h.files['Alarm-dialog-request'] = alarmFile([A]);
  await settle(h);
  check(h.activeId() === '301' && h.rendered().join() === '301', '0: a single request shows as before');

  // ---- 1. three requests posted in one tick: ONE file read queues all three, oldest first --------------------------------------
  h = makeBridge();
  h.files['Alarm-dialog-request'] = alarmFile([A, B, C]);
  await settle(h);
  check(h.activeId() === '301', '1: the oldest (301) shows first (active: ' + h.activeId() + ')');
  check(h.stopQueue().join() === '302,303', '1: 302 and 303 wait behind it, in order (queue: ' + h.stopQueue().join() + ')');

  // ---- 2. the same file read again adds nothing ----------------------------------------------------------------------------------
  await settle(h);
  await settle(h);
  check(h.stopQueue().join() === '302,303' && h.rendered().join() === '301', '2: re-reading the same file queues nothing twice');

  // ---- 3. the page already had A: a later file [A, B, C] adds only B and C -----------------------------------------------------
  h = makeBridge();
  h.files['Alarm-dialog-request'] = alarmFile([A]);
  await settle(h);
  h.files['Alarm-dialog-request'] = alarmFile([A, B, C]);
  await settle(h);
  check(h.activeId() === '301' && h.stopQueue().join() === '302,303', '3: only the newer two are added (queue: ' + h.stopQueue().join() + ')');

  // ---- 4. an old-format file (no recent[]) with the newest only: today's behaviour (the older ones are lost) --------------------
  h = makeBridge();
  h.files['Alarm-dialog-request'] = alarmFile([A, B, C], true);
  await settle(h);
  check(h.activeId() === '303' && h.stopQueue().length === 0, '4: without recent[] only the newest arrives (why C++ now sends recent[])');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL: exception ' + (e && e.stack)); process.exit(1); });
