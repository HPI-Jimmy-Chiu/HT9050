// AI(W906-D019) 20260930 / AI(W906-D020) 20261001 [W906] St01: offline selftest for todo D-019 and D-020 (ctest D019_D020_PageSelftest).
//   D-019  web/page/ht9045_contact_slk.js section 18 (Setup.Contact.html). golden V912 cContact.cpp TfContact::FormShow (:1094) runs
//          scrbSLKChange (:1186 -> DutCount :1988 / ShowArmAndDeviceForce :1925, both read LastSet.bUseTestSocket) on EVERY Show();
//          background.html only hides a closed window's iframe, so the page now recomputes on the HT_WIN closed -> open edge, and once
//          more if a site.arm{1,2}.s{n} tag changes within 2 s of that edge (stage F: the post-open synthetic snapshot has no ordering
//          guarantee against HT_WIN). Minimized is open (no edge); a page that never got HT_WIN (standalone) behaves as before.
//   D-020  web/page/ht9045_lotinfo_wire.js tabVis (Data.LotInfo.html). golden V912 uLotInfo.cpp pgLotinfoChange (:7340) is wired only
//          as uLotInfo.dfm:98 OnChange (a user tab click); nothing calls it and VCL does not fire it on a programmatic ActivePage /
//          TabIndex / TabVisible change. So hiding the ACTIVE tab now switches only the view (same DOM walk as tabs.js) and sends no
//          lotinfo.op (tab_tsSelection -> selection.get, tab_tsLotID -> lotEnd.state); a real user click still sends them.
//   Loads the REAL ht9045_contact_slk.js, ht9045_lotinfo_wire.js and tabs.js in node vm contexts with a fake DOM, fake HT9045Tags /
//   HT9045System / HT9045Recipe, and a fake clock. Reads background.html (read-only) to ratchet the HT_WIN `open` contract.
//   No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/d019_d020_selftest.cjs [<web/page dir>]
//   Control runs (must go red): W906_CONTACT_SLK=<pre-D-019 ht9045_contact_slk.js>, W906_LOTINFO_WIRE=<pre-D-020 ht9045_lotinfo_wire.js>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SLK = process.env.W906_CONTACT_SLK || path.join(PAGE, 'ht9045_contact_slk.js');
const LOT = process.env.W906_LOTINFO_WIRE || path.join(PAGE, 'ht9045_lotinfo_wire.js');
const slkCode = fs.readFileSync(SLK, 'utf8');
const lotCode = fs.readFileSync(LOT, 'utf8');
const tabsCode = fs.readFileSync(path.join(PAGE, 'tabs.js'), 'utf8');
const bgHtml = fs.readFileSync(path.join(PAGE, '..', 'background.html'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock
let clock = 0, timers = [], tid = 1;
function fakeSetTimeout(fn, ms) { const id = tid++; timers.push({ id, due: clock + Math.max(0, ms | 0), fn }); return id; }
function fakeClearTimeout(id) { timers = timers.filter((t) => t.id !== id); }
async function settle() { for (let i = 0; i < 16; i++) await new Promise((r) => setImmediate(r)); }
async function advance(ms) {
  const end = clock + ms;
  await settle();
  for (;;) {
    timers.sort((a, b) => a.due - b.due || a.id - b.id);
    const t = timers[0];
    if (!t || t.due > end) break;
    timers.shift();
    clock = t.due;
    t.fn();
    await settle();
  }
  clock = end;
  await settle();
}
const quiet = { info() {}, warn() {}, log() {}, error(...a) { console.error(...a); } };

// =====================================================================================================================
// D-019: Setup.Contact
// =====================================================================================================================
function siteTags(off) {                       // site.arm{1,2}.s{1..16}; `off` = list of tags that are false
  const o = {};
  for (let a = 1; a <= 2; a++) for (let s = 1; s <= 16; s++) o['site.arm' + a + '.s' + s] = true;
  (off || []).forEach((k) => { o[k] = false; });
  return o;
}
const S1_OFF = ['site.arm1.s1', 'site.arm2.s1'];   // golden DutCount :2001-2002: s1 closed on both arms -> dDutCount=1

function makeTags(initial) {                   // same notify rules as ht9045_recipe_client.js HT9045Tags (changed keys only; subscribe replays)
  const state = Object.assign({}, initial || {});
  const subs = [];
  return {
    has: (t) => t in state,
    get: (t) => (t in state ? state[t] : null),
    all: () => Object.assign({}, state),
    subscribe(fn) {
      subs.push(fn);
      if (Object.keys(state).length) fn(Object.assign({}, state), state);
      return () => { const i = subs.indexOf(fn); if (i >= 0) subs.splice(i, 1); };
    },
    on(tag, fn) { return this.subscribe((ch) => { if (tag in ch) fn(ch[tag], tag); }); },
    frame(data) {                              // one snapshot / patch frame
      const ch = {};
      Object.keys(data).forEach((k) => { if (!(k in state) || state[k] !== data[k]) { state[k] = data[k]; ch[k] = data[k]; } });
      if (Object.keys(ch).length) subs.slice().forEach((f) => f(ch, state));
    },
    nSubs: () => subs.length,
  };
}

function makeContact(opts) {
  opts = opts || {};
  const els = {};
  let airNWrites = 0;
  function plain(id) {
    const e = { id, style: {}, title: '', textContent: '', listeners: {} };
    e.addEventListener = (t, f) => { (e.listeners[t] = e.listeners[t] || []).push(f); };
    e.setAttribute = (k, v) => { e['@' + k] = String(v); };
    e.getAttribute = (k) => (('@' + k) in e ? e['@' + k] : null);
    e.appendChild = (c) => c;
    e.getBoundingClientRect = () => ({ top: 0, height: 0 });
    e.focus = () => {};
    e.clientHeight = 0; e.offsetHeight = 0;
    if (id) els[id] = e;
    return e;
  }
  function input(id, v) { const e = plain(id); e.value = v; return e; }
  input('edPinCount', '100');
  input('edForcePerPinN', '0.2940');
  input('edForcePerPinG', '30');
  input('edDieForcePerPinG', '0');
  ['edDieForcePerPinN', 'edAirForce', 'edForcePerDeviceKG', 'edForcePerDeviceN', 'edAirKPA', 'edSetKg',
   'edtPinOfDie', 'edDoubleForce', 'edTransfer'].forEach((id) => input(id, ''));
  const airN = plain('edAirForceN');                                 // written once per ShowArmAndDeviceForce (golden :1946)
  let airNv = '';
  Object.defineProperty(airN, 'value', { enumerable: true, get() { return airNv; }, set(v) { airNv = String(v); airNWrites++; } });
  plain('lblMaxForcePerIC'); plain('lblMinForce'); plain('imgSLK'); plain('scrbSLK');
  const rg = plain('rgKitDiameter');
  const radios = ['30', '40', '60'].map(() => ({ checked: false }));
  const labels = ['30', '40', '60'].map((t) => ({ textContent: t }));
  rg.querySelector = () => null;
  rg.querySelectorAll = (sel) => (sel.indexOf('radio') >= 0 ? radios : labels);

  const msgListeners = [];
  const tags = opts.noTags ? undefined : makeTags(opts.tags);
  let release = null;
  const gate = opts.holdLoads ? new Promise((r) => { release = r; }) : Promise.resolve();
  const doc = (sections) => gate.then(() => ({ sections }));
  const SYS = {
    gerneral: { Version: { Model: 'HT-9045' }, System: { CUSTOMER_CODE: '0', INDEX_PRESS_TYPE: '0', EP_MAXKPA: '0', INSTALL_DOUBLE_EP: '0' } },
    config: { Index: { UseSingleSite85kg: '1', bD28MaxForceLimitByDiameter: '0' }, 'Contact Force': { bD04MinForceByFile: '0' } },
    contactInfo: { 'SLK Type': { Type: '30,40,60', Visible: '1,1,1' } },
  };
  const REC = {
    handlerCondition: { Configuration: { 'Test Mode': '2-Site' } },   // DualSite: golden DutCount :1996-2011 reads the site states
    contact: { Mode: { 'Head Device Mode': '2', 'Kit Diameter': '3.0' } },
  };
  const sb = {
    console: quiet, setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout,
    Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp, isFinite, isNaN, parseFloat, parseInt,
    HT9045System: { read: (n) => doc(SYS[n]) },
    HT9045Recipe: { read: (n) => doc(REC[n]) },
    addEventListener(t, f) { if (t === 'message') msgListeners.push(f); },
  };
  if (tags) sb.HT9045Tags = tags;
  sb.window = sb;
  const FRAME = { name: 'background.html' };
  sb.parent = opts.standalone ? sb : FRAME;
  sb.document = {
    readyState: 'complete',
    getElementById: (id) => els[id] || null,
    createElement: () => plain(''),
    createTextNode: (t) => ({ textContent: t }),
    addEventListener() {},
  };
  vm.createContext(sb);
  vm.runInContext(slkCode, sb, { filename: 'ht9045_contact_slk.js' });
  const P = {
    sb, els, tags, FRAME,
    writes: () => airNWrites,
    air: () => els.edAirForce.value,
    api: () => sb.HT9045ContactSLK,
    reopen: () => (sb.HT9045ContactSLK && sb.HT9045ContactSLK.reopen ? sb.HT9045ContactSLK.reopen() : {}),
    post(data, source) { msgListeners.forEach((f) => f({ data, source: source === undefined ? FRAME : source })); },
    win(open, state, initial) { P.post({ type: 'HT_WIN', id: 'contact', open, state, initial: !!initial }); },
    release: () => release && release(),
  };
  return P;
}

// =====================================================================================================================
// D-020: Data.LotInfo
// =====================================================================================================================
class FNode {
  constructor(doc, id) {
    this.doc = doc; this.id = id || ''; this.children = []; this.parentElement = null; this.listeners = {};
    this.style = {}; this.dataset = {}; this.title = ''; this.textContent = ''; this._cls = new Set();
    const self = this;
    this.classList = { add: (c) => self._cls.add(c), remove: (c) => self._cls.delete(c), contains: (c) => self._cls.has(c) };
    if (id) doc.els[id] = this;
  }
  append(c) { c.parentElement = this; this.children.push(c); return c; }
  addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); }
  matches(sel) { return sel[0] === '.' && sel.slice(1).split('.').every((c) => this._cls.has(c)); }   // '.tab' / '.tab.act'
  closest(sel) { for (let n = this; n; n = n.parentElement) if (n.matches(sel)) return n; return null; }
  querySelectorAll(sel) { const out = []; const walk = (n) => n.children.forEach((c) => { if (c.matches(sel)) out.push(c); walk(c); }); walk(this); return out; }
  querySelector(sel) { return this.querySelectorAll(sel)[0] || null; }
  click() {                                    // a real click: target, bubble to parents, then document (tabs.js delegate)
    this.doc.clicks.push(this.id);
    const ev = { type: 'click', target: this, stopPropagation() {}, preventDefault() {} };
    for (let n = this; n; n = n.parentElement) (n.listeners.click || []).forEach((f) => f(ev));
    (this.doc.listeners.click || []).forEach((f) => f(ev));
  }
}
function makeLot() {
  const D = { els: {}, listeners: {}, clicks: [] };
  const cmds = [];
  let cfg = null;
  const sb = {
    console: quiet, setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout,
    Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    HT9045Wire: { register(c) { cfg = c; }, say() {} },
    HT9045Recipe: {
      rawCmd(name, extra) {
        cmds.push({ name, value: extra && extra.value });
        return Promise.resolve({ executed: true, boxes: {} });
      },
    },
    confirm: () => false,
  };
  sb.window = sb;
  sb.document = {
    readyState: 'complete',
    getElementById: (id) => D.els[id] || null,
    querySelector: (sel) => (scope ? scope.querySelector(sel) : null),
    querySelectorAll: (sel) => (scope ? (sel === '.tabs' ? [tabsEl] : scope.querySelectorAll(sel)) : []),
    addEventListener(t, f) { (D.listeners[t] = D.listeners[t] || []).push(f); },
  };
  let scope = null, tabsEl = null;
  // the DOM of Data.LotInfo.html: <div class="tabs" id="pgLotinfo"><span class="tab" data-tab=.. id="tab_<dfm name>">..
  //   and the panes <div class="tabPane" data-pane=..> as siblings of .tabs (tabs.js: scope = tabs.parentElement)
  scope = new FNode(D, 'scope');
  tabsEl = scope.append(new FNode(D, 'pgLotinfo'));
  tabsEl.classList.add('tabs');
  vm.createContext(sb);
  // tab list = the script's own TABS (HT9045LotInfo.tabs); build the DOM after a dry load, then load for real
  const dry = { window: {}, document: { readyState: 'complete', getElementById: () => null, querySelector: () => null, addEventListener() {} },
                HT9045Wire: { register() {} }, console: quiet, Promise, JSON, Date, Math, Object, Array, String };
  dry.window = dry;
  vm.createContext(dry);
  vm.runInContext(lotCode, dry, { filename: 'ht9045_lotinfo_wire.js (dry)' });
  const TABS = dry.HT9045LotInfo.tabs;
  TABS.forEach((n) => {
    const t = tabsEl.append(new FNode(D, 'tab_' + n));
    t.classList.add('tab'); t.dataset.tab = n.toLowerCase(); t.title = n;
    const p = scope.append(new FNode(D, 'pane_' + n));
    p.classList.add('tabPane'); p.dataset.pane = n.toLowerCase();
  });
  D.els.tab_tsRTCFullViewImg.classList.add('act');                  // Data.LotInfo.html:38 <span class="tab act" ... tsRTCFullViewImg>
  vm.runInContext(tabsCode, sb, { filename: 'tabs.js' });
  (D.listeners.DOMContentLoaded || []).forEach((f) => f());       // tabs.js: only the .act pane is shown
  vm.runInContext(lotCode, sb, { filename: 'ht9045_lotinfo_wire.js' });
  const L = {
    D, cmds, TABS, sb,
    tag(name, v) { const e = cfg.tags['lot.tab.' + name]; e[1](v, D.els[e[0]]); },
    act: () => TABS.filter((n) => D.els['tab_' + n].classList.contains('act')),
    shownPanes: () => TABS.filter((n) => D.els['pane_' + n].style.display === ''),
    ops: (from) => cmds.slice(from || 0).filter((c) => c.name === 'lotinfo.op').map((c) => { try { return JSON.parse(c.value).op; } catch (e) { return '?'; } }),
  };
  return L;
}

(async () => {
  console.log('d019_d020_selftest -- ' + SLK + ' | ' + LOT);

  console.log('[0] background.html (read-only): HT_WIN open = open or minimized (the edge the page keys on)');
  check(bgHtml.indexOf("{type:'HT_WIN',id:id,open:(st==='open'||st==='minimized'),state:st,initial:!!initial}") >= 0,
        "postWinState posts {type:'HT_WIN', id, open:(st==='open'||st==='minimized'), state, initial}");

  // --------------------------------------------------------------------------------------------------- D-019
  console.log('[1] D-019 hosted, loaded while closed; tags keep flowing while closed (review6, no stage F)');
  const A = makeContact({ tags: siteTags() });
  A.win(false, 'closed', true);                                     // background.html iframe load: initial state
  await advance(0);
  check(A.writes() === 1 && A.air() === '6.0000', 'boot: the section 17 FormShow compute ran once, 2 duts x 3 kg = 6.0000 (got ' + A.writes() + ' / ' + A.air() + ')');
  A.tags.frame({ 'site.arm1.s1': false, 'site.arm2.s1': false });
  await advance(100);
  check(A.writes() === 1 && A.air() === '6.0000', 'closed: a site change does not recompute (golden FormClose :1847 fShow=false; no timer)');
  A.win(true, 'open');
  await advance(0);
  check(A.writes() === 2 && A.air() === '3.0000', 'closed -> open edge: ONE recompute right away from the current values (1 dut -> 3.0000; got ' + A.writes() + ' / ' + A.air() + ')');
  check(A.reopen().winOpen === true && A.reopen().shows === 1 && A.reopen().follow === true, 'reopen(): winOpen, shows=1, follow window armed');
  await advance(500);
  A.tags.frame({ 'site.arm1.s1': true });
  await advance(0);
  check(A.writes() === 3 && A.air() === '6.0000' && A.reopen().follows === 1, 'a site.arm tag changes 0.5 s after the edge -> a second recompute (6.0000)');
  A.tags.frame({ 'site.arm1.s1': false });
  await advance(0);
  check(A.writes() === 3 && A.reopen().follow === false, '... only once: the next change inside the 2 s does not recompute (golden does not recompute while shown)');
  A.win(true, 'minimized');
  await advance(0);
  A.win(true, 'open');
  await advance(0);
  check(A.writes() === 3 && A.reopen().shows === 1, 'open -> minimized -> open: minimized is open, no edge, no recompute');
  A.win(false, 'closed');
  await advance(0);
  check(A.writes() === 3, 'open -> closed: nothing computed');
  A.win(true, 'open');
  await advance(0);
  check(A.writes() === 4 && A.air() === '3.0000' && A.reopen().shows === 2, 'second closed -> open edge: one more recompute (3.0000)');
  await advance(2001);
  check(A.reopen().follow === false, 'the follow window ends 2 s after the edge');
  A.tags.frame({ 'site.arm1.s1': true, 'site.arm2.s1': true });
  await advance(0);
  check(A.writes() === 4 && A.air() === '3.0000', 'a site change long (2 s+) after the edge -> no recompute');
  A.win(false, 'closed'); await advance(0);
  A.win(true, 'open'); await advance(0);
  const w5 = A.writes();
  A.tags.frame({ 'machine.state': 'RUN', 'auth.level': 2 });
  await advance(0);
  check(w5 === 5 && A.writes() === 5, 'inside the window a non-site tag change -> no recompute');
  A.win(false, 'closed'); await advance(0);
  await advance(300);
  A.tags.frame({ 'site.arm1.s1': false, 'site.arm2.s1': false });
  await advance(0);
  check(A.writes() === 5, 'closed again inside the 2 s -> the follow is dropped (no recompute while closed)');
  A.post({ type: 'HT_WIN', id: 'contact', open: true, state: 'open' }, { name: 'some other frame' });
  await advance(0);
  check(A.writes() === 5 && A.reopen().winOpen === false, 'HT_WIN from a source other than window.parent is ignored');
  A.post({ type: 'HT_OTHER', open: true });
  await advance(0);
  check(A.writes() === 5, 'other message types are ignored');

  console.log('[2] D-019 stage F: no tag frames while closed, the synthetic snapshot lands AFTER HT_WIN');
  const F = makeContact({ tags: siteTags() });                      // BOOT_TAGS at socket open: all sites on
  F.win(false, 'closed', true);
  await advance(0);
  check(F.writes() === 1 && F.air() === '6.0000', 'boot on BOOT_TAGS: 6.0000');
  F.win(true, 'open');
  await advance(0);
  check(F.writes() === 2 && F.air() === '6.0000', 'edge: recomputed at once, but get() still has the values from before the window closed');
  await advance(30);
  F.tags.frame(Object.assign(siteTags(S1_OFF), { 'machine.state': 'IDLE' }));   // the full synthetic snapshot (ht9045_link.js Hub.windowState)
  await advance(0);
  check(F.writes() === 3 && F.air() === '3.0000' && F.reopen().follows === 1, 'the snapshot 30 ms later carries the closed sites -> recomputed again (3.0000)');

  console.log('[3] D-019 first HT_WIN already open (lazy window / refresh while open): not an edge');
  const Z = makeContact({ tags: siteTags() });
  Z.win(true, 'open', true);
  await advance(0);
  check(Z.writes() === 1 && Z.reopen().shows === 0, 'never-told -> open: the boot compute is this FormShow, no second one');
  Z.tags.frame({ 'site.arm1.s1': false, 'site.arm2.s1': false });
  await advance(0);
  check(Z.writes() === 1 && Z.air() === '6.0000', '... and no follow window is armed');

  console.log('[4] D-019 the edge arrives before the boot compute finished (fShow still false)');
  const E = makeContact({ tags: siteTags(), holdLoads: true });
  E.win(false, 'closed', true);
  E.win(true, 'open');
  await advance(0);
  check(E.writes() === 0 && E.reopen().shows === 0 && E.reopen().follow === true, 'edge before boot: skipped (fShow false), follow window armed');
  E.release();
  await advance(0);
  check(E.writes() === 1, 'boot compute runs once with the values of that moment');
  E.tags.frame({ 'site.arm1.s1': false, 'site.arm2.s1': false });
  await advance(0);
  check(E.writes() === 2 && E.air() === '3.0000', 'a site change still inside the 2 s -> recomputed once');

  console.log('[5] D-019 standalone page (window.parent === window) and no HT9045Tags: unchanged');
  const S = makeContact({ tags: siteTags(), standalone: true });
  await advance(0);
  check(S.writes() === 1 && S.air() === '6.0000', 'standalone boot: one compute');
  S.post({ type: 'HT_WIN', id: 'contact', open: false, state: 'closed' }, S.sb);
  S.post({ type: 'HT_WIN', id: 'contact', open: true, state: 'open' }, S.sb);
  S.tags.frame({ 'site.arm1.s1': false, 'site.arm2.s1': false });
  await advance(3000);
  check(S.writes() === 1 && S.air() === '6.0000' && S.tags.nSubs() === 0, 'HT_WIN-shaped messages from itself are ignored, tags never subscribed, nothing recomputed');
  const N = makeContact({ noTags: true });
  N.win(false, 'closed', true);
  await advance(0);
  N.win(true, 'open');
  await advance(0);
  check(N.writes() === 2 && N.air() === '6.0000', 'hosted without HT9045Tags: the edge still recomputes (sites default to in use, as section 4)');

  // --------------------------------------------------------------------------------------------------- D-020
  console.log('[6] D-020 hiding the ACTIVE tab switches only the view');
  const L = makeLot();
  check(L.act().join() === 'tsRTCFullViewImg' && L.shownPanes().join() === 'tsRTCFullViewImg', 'start: tsRTCFullViewImg active, only its pane shown (tabs.js)');
  L.tag('tsDeviceInfo', false);
  await advance(0);
  check(L.act().join() === 'tsRTCFullViewImg' && L.cmds.length === 0, 'hiding a tab that is not active: no switch, no command');
  L.tag('tsRTCFullViewImg', false);
  await advance(0);
  check(L.act().join() === 'tsLotID' && L.shownPanes().join() === 'tsLotID',
        'hiding the active tab -> the first visible tab (tsLotID) is active and its pane shown (got ' + L.act().join() + ' / ' + L.shownPanes().join() + ')');
  check(L.cmds.length === 0 && L.D.clicks.length === 0,
        'no click and no command: no control.acquire, no lotinfo.op lotEnd.state (golden: no pgLotinfoChange) -- sent ' + L.cmds.map((c) => c.name).join(','));
  ['tsLotID', 'tsFTP', 'tsATC', 'ts_OCRInterface', 'ts_SocketInterface'].forEach((n) => { if (n !== 'tsLotID') L.tag(n, false); });
  L.tag('tsLotID', false);
  await advance(0);
  check(L.act().join() === 'tsSelection' && L.shownPanes().join() === 'tsSelection' && L.ops().indexOf('selection.get') < 0 && L.cmds.length === 0,
        'hiding the active tsLotID with tsSelection first visible -> view on Selection, no selection.get (no Security_new.def read / write)');
  L.tag('tsSelection', null);
  await advance(0);
  check(L.act().join() === 'tsSelection' && L.D.els.tab_tsSelection.style.display === '', 'TabVisible unknown (null) keeps the tab shown and active');

  console.log('[7] D-020 a real user tab click still runs the page command (golden OnChange = user click)');
  const c7 = L.cmds.length;                                         // [7] counts only its own commands
  L.tag('tsBarCode', true);
  L.D.els.tab_tsBarCode.click();
  await advance(0);
  check(L.act().join() === 'tsBarCode' && L.cmds.length === c7, 'user click on BarCode: view switched by tabs.js, BarCode has no page command');
  L.D.els.tab_tsSelection.click();
  await advance(0);
  check(L.act().join() === 'tsSelection' && L.ops(c7).join() === 'selection.get' && L.cmds[c7] && L.cmds[c7].name === 'control.acquire',
        'user click on Selection: control.acquire + lotinfo.op selection.get (golden pgLotinfoChange :7343-7382)');
  L.tag('tsLotID', true);
  L.D.els.tab_tsLotID.click();
  await advance(0);
  check(L.act().join() === 'tsLotID' && L.ops(c7).join() === 'selection.get,lotEnd.state', 'user click on Lot: lotinfo.op lotEnd.state as before');

  console.log('[8] D-020 source ratchets');
  const tabVisSrc = ((lotCode.match(/function tabVis\(v, el\) \{[\s\S]*?\n  \}/) || [''])[0]).replace(/\/\/.*$/gm, '');   // code only
  check(tabVisSrc !== '' && tabVisSrc.indexOf('.click(') < 0 && tabVisSrc.indexOf('showTabOnly(f)') >= 0,
        'tabVis switches with showTabOnly, not .click() (comments stripped before the check)');
  check(tabsCode.indexOf("tabs.querySelectorAll('.tab').forEach(function (t) { t.classList.remove('act'); });") >= 0 &&
        tabsCode.indexOf("p.style.display = (p.dataset.pane === tab.dataset.tab) ? '' : 'none';") >= 0,
        'tabs.js still switches with .tab.act + sibling .tabPane[data-pane] (the walk showTabOnly copies)');

  console.log('d019_d020_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
