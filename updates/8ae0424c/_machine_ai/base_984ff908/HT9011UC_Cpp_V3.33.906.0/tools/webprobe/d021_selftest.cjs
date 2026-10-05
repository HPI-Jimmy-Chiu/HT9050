// AI(W906-D021) 20261001 [W906] St01: offline selftest for todo D-021 (ctest D021_ContactScrollSync).
//   Setup.Contact.html scrbSLK. golden 906 cContact.cpp (V912 in （）; AI(W906-E030-CITE) 20261003) has ONE TScrollBar Position: FormShow :1169（V912 :1185） sets it from
//   DeviceForm_File.iHeadDeviceCT and :1170（V912 :1186） calls scrbSLKChange; spbSaveClick :14072（V912 :14179） -> SaveSetupFile :14296（V912 :14404） writes
//   WriteIniData("Mode","Head Device Mode",scrbSLK->Position). On the web the engine (ht9045_wire_engine.js, the laptop's file) writes
//   the C++ Position into #scrbSLK.value on every editlist.get (gbApply :1163; C++ runs golden FormShow for each get,
//   FileRW/_EditPage.cpp:58) and reads parseInt(#scrbSLK.value) at Save (gbValue :1250); ht9045_contact_ev.js reads it for the
//   form.event state (:146) and writes it from an ack (:237). ht9045_contact_slk.js section 19 makes #scrbSLK.value an accessor
//   over SB.pos: a read is the live position, a write is adopted (golden :1169（V912 :1185）) and followed by ONE scrbSLKChange (golden :1170（V912 :1186）)
//   after the writer's batch. Checked here:
//     a scroll -> #scrbSLK.value follows, the engine's real Save sends it, the form.event state carries it;
//     an engine load (boot, reopen, post-save reload, reload) or an ack -> SB.pos follows and the recompute runs once, after the batch;
//     no loop (page moves never write #scrbSLK.value); the D-019 reopen edge still recomputes at once.
//   AI(W906-D022) 20261001: contact_slk section 20. golden TfContact::FormShow (:1078（V912 :1094）) runs only when the form is shown
//     (906 main.cpp:27312（V912 :28302） sbContactClick -> :27324（V912 :28314） fContact->Show()). The page used to call HT9045Page.load() at boot even while its
//     (non-lazy, hidden) window was closed = one editlist.get = C++ FormShow + Enter at site start, around the engine's H4 open-only
//     read. Now, with the engine's own H4 rule (same HT_WIN, same 3 s): hosted + closed at boot -> 0 editlist.get at boot, the engine's
//     H4 read on the first open is the only one, and section 17 (fShow, :1169（V912 :1185） Position, :1170（V912 :1186） scrbSLKChange) runs after it;
//     hosted + open at boot, hosted without any HT_WIN (3 s), standalone -> as before.
//   AI(W906-D023) 20261001: contact_slk sections 5 / 15 / 17. golden FormShow rebuilds rgKitDiameter->Items from the bShow SLKClass
//     only (:1108-1128（V912 :1124-1144）) and DoIniDataToForm (:843-911（V912 :851-919）) sets ItemIndex; nothing changes it before :1170（V912 :1186）. The page listed the hidden
//     sizes too and re-selected from the recipe file after the C++ load. Now: options = the bShow sizes, section 17 adopts the C++
//     ItemIndex (HT9045Page.golden().page), and falls back to the recipe file only when C++ gave none (or one out of range).
//   Loads the REAL ht9045_wire_engine.js, ht9045_wire_setupcontact.js, ht9045_contact_ev.js and ht9045_contact_slk.js into ONE node vm
//   context in Setup.Contact.html's order, with a fake DOM, a fake HT9045Recipe (editlist.get / editlist.save / form.event), fake
//   HT9045System / HT9045Tags and a fake clock. Setup.Contact.html and background.html are read only (ratchets). No socket, no wb_serve,
//   no file written.
//   Usage: node tools/webprobe/d021_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_CONTACT_SLK=<the pre-D-021 ht9045_contact_slk.js> (D-021), or <the pre-D-022/D-023 one>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SLK = process.env.W906_CONTACT_SLK || path.join(PAGE, 'ht9045_contact_slk.js');
const rd = (f) => fs.readFileSync(f, 'utf8');
const engineCode = rd(path.join(PAGE, 'ht9045_wire_engine.js'));
const wireCode = rd(path.join(PAGE, 'ht9045_wire_setupcontact.js'));
const evCode = rd(path.join(PAGE, 'ht9045_contact_ev.js'));
const slkCode = rd(SLK);
const html = rd(path.join(PAGE, 'Setup.Contact.html'));
const bgHtml = rd(path.join(PAGE, '..', 'background.html'));

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock (same as d019_d020_selftest.cjs)
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

// ---------------------------------------------------------------- the page: fake DOM + fake C++ + the four real scripts
// Air force figures for the inputs below (DualSite, all sites in use -> dDutCount 2; 10 pins x 30 gf = 0.3 kg per device):
//   golden CalculateTotalAirForce: per-head load 0.6 / (2 x fComplianceUnit) is under the kit's minimum -> 2 x fComplianceUnit x minimum.
//   30 mm (min 1.5): pos 2 -> 3.0000, 3 -> 1.5000, 4 -> 0.7500, 5 -> 6.0000.   40 mm (min 4.0): pos 3 -> 4.0000, 4 -> 2.0000.
//   HT9046 (Max 6), 30 mm, pos 6 (x0.125): 0.6 / 0.25 = 2.4 kg per head, above the minimum -> 0.6000.
//   AI(W906-D022) 20261001: site 1 closed on both arms with [D27] on -> DutCount :1972-1973（V912 :2001-2002） dDutCount 1 and CalculateTotalAirForce n=1:
//     30 mm pos 3: 0.3 / 0.5 = 0.6 kg per head, under 1.5 -> 1.5 x 0.5 = 0.7500 (GetMaxIndexForceLimit gives 0 = no clamp for 30 mm).
//   AI(W906-D023) 20261001: 80 mm (min 15.0, GetMinForce :18997（V912 :19288）), pos 2 (x1.0), 2 duts: 0.6 / 2 = 0.3 under 15 -> 15 x 2 = 30.0000.
const AIR = { '30:2': '3.0000', '30:3': '1.5000', '30:4': '0.7500', '30:5': '6.0000', '40:3': '4.0000', '40:4': '2.0000', '30:6': '0.6000',
              '30:3:1dut': '0.7500', '80:2': '30.0000' };
const S1_OFF = { 'site.arm1.s1': false, 'site.arm2.s1': false };

function makePage(opts) {
  opts = opts || {};
  const byId = {};
  const docListeners = {};
  class El {
    constructor(tag, id) {
      this.tagName = tag; this.nodeType = 1; this._id = ''; this.children = []; this.parentNode = null;
      this.style = {}; this.attrs = {}; this.listeners = {}; this.textContent = ''; this.title = '';
      if (id) this.id = id;
    }
    get id() { return this._id; }
    set id(v) { this._id = v; if (v) byId[v] = this; }
    get firstChild() { return this.children[0] || null; }
    get clientHeight() { return 0; }
    get offsetHeight() { return 0; }
    appendChild(c) { c.parentNode = this; this.children.push(c); return c; }
    insertBefore(c, ref) { const i = this.children.indexOf(ref); c.parentNode = this; if (i < 0) this.children.push(c); else this.children.splice(i, 0, c); return c; }
    removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); return c; }
    setAttribute(k, v) { this.attrs[k] = String(v); }
    getAttribute(k) { return k in this.attrs ? this.attrs[k] : null; }
    removeAttribute(k) { delete this.attrs[k]; }
    addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); }
    querySelectorAll() { return []; }
    querySelector() { return null; }
    closest() { return this.id ? this : null; }
    getBoundingClientRect() { return { top: 0, height: 0 }; }
    focus() {}
    fire(t, ev) { (this.listeners[t] || []).slice().forEach((f) => f(Object.assign({ type: t, target: this, preventDefault() {}, stopPropagation() {} }, ev || {}))); }
  }
  const els = byId;
  function input(id, v) { const e = new El('INPUT', id); e.type = 'text'; e.value = v; return e; }
  input('edPinCount', '10');
  input('edForcePerPinN', '0.2940');
  input('edForcePerPinG', '30');
  input('edDieForcePerPinG', '0');
  ['edDieForcePerPinN', 'edAirForce', 'edForcePerDeviceKG', 'edForcePerDeviceN', 'edAirKPA', 'edSetKg',
   'edtPinOfDie', 'edDoubleForce', 'edTransfer'].forEach((id) => input(id, ''));
  let airNWrites = 0, airNv = '';
  const airN = new El('INPUT', 'edAirForceN');                       // written once per ShowArmAndDeviceForce (golden :1917（V912 :1946）)
  Object.defineProperty(airN, 'value', { enumerable: true, get() { return airNv; }, set(v) { airNv = String(v); airNWrites++; } });
  ['lblMaxForcePerIC', 'lblMinForce'].forEach((id) => new El('SPAN', id));
  new El('IMG', 'imgSLK');
  const slk = new El('DIV', 'scrbSLK');
  const rg = new El('FIELDSET', 'rgKitDiameter');
  let radios;
  if (opts.kit) {
    // AI(W906-D023) 20261001: Setup.Contact.html's real shape <fieldset id=rgKitDiameter><div class="cli"><label class="rgi"><input
    //   type=radio>30</label>.. (the dfm's 30 / 40 / 60, 30 checked), so contact_slk's section 5 rebuildKitDiameter really rebuilds it
    const cli = new El('DIV'); cli.className = 'cli';
    Object.defineProperty(cli, 'innerHTML', { configurable: true, get() { return ''; }, set() { cli.children = []; } });
    rg.appendChild(cli);
    ['30', '40', '60'].forEach((t, i) => {
      const lab = new El('LABEL'); lab.className = 'rgi';
      const inp = new El('INPUT'); inp.type = 'radio'; inp.checked = (i === 0);
      lab.appendChild(inp); lab.appendChild({ nodeType: 3, textContent: t }); cli.appendChild(lab);
    });
    const labs = () => cli.children.filter((c) => c.tagName === 'LABEL');
    rg.querySelector = (sel) => (sel === '.cli' ? cli : null);
    rg.querySelectorAll = (sel) => (sel.indexOf('radio') >= 0 ? labs().map((l) => l.children.find((c) => c.tagName === 'INPUT'))
      : sel.indexOf('rgi') >= 0 ? labs().map((l) => ({ textContent: l.children.filter((c) => c.nodeType === 3).map((c) => c.textContent).join('') })) : []);
  } else {
    radios = ['30', '40', '60'].map(() => ({ tagName: 'INPUT', type: 'radio', checked: false }));
    const labels = ['30', '40', '60'].map((t) => ({ textContent: t }));
    rg.querySelectorAll = (sel) => (sel.indexOf('radio') >= 0 ? radios : sel.indexOf('rgi') >= 0 ? labels : []);
  }
  const save = new El('BUTTON', 'spbSave');                          // cContact.dfm Save; the engine captures its click
  const body = new El('BODY'), docEl = new El('HTML');

  // ---- fake C++ (FileRW/DeviceForm_File.cpp behind WS editlist.get / editlist.save / form.event) ----
  const CPP = { pos: opts.cppPos === undefined ? 3 : opts.cppPos, kit: opts.cppKit === undefined ? 0 : opts.cppKit,
                gets: 0, saves: [], cmds: [], ack: null, failNext: !!opts.failFirstGet };
  const clone = (o) => JSON.parse(JSON.stringify(o));
  const tags = (() => {
    const state = {};
    for (let a = 1; a <= 2; a++) for (let s = 1; s <= 16; s++) state['site.arm' + a + '.s' + s] = true;
    const subs = [];
    return {
      has: (t) => t in state, get: (t) => (t in state ? state[t] : null), all: () => Object.assign({}, state),
      subscribe(fn) { subs.push(fn); fn(Object.assign({}, state), state); return () => {}; },
      on(tag, fn) { return this.subscribe((ch) => { if (tag in ch) fn(ch[tag], tag); }); },
      connect: () => Promise.resolve(),
      frame(ch) { Object.assign(state, ch); subs.slice().forEach((f) => f(Object.assign({}, ch), state)); },   // a tag frame / snapshot
    };
  })();
  const SYS = {
    gerneral: { Version: { Model: opts.model || 'HT-9045' }, System: { CUSTOMER_CODE: '0', INDEX_PRESS_TYPE: '0', EP_MAXKPA: '0', INSTALL_DOUBLE_EP: '0' } },
    config: { Index: { UseSingleSite85kg: '1', bD28MaxForceLimitByDiameter: '0' }, 'Contact Force': { bD04MinForceByFile: '0' } },
    contactInfo: { 'SLK Type': opts.kit ? { Type: opts.kit.types, Visible: opts.kit.visible } : { Type: '30,40,60', Visible: '1,1,1' } },
  };
  const RECDOC = {
    handlerCondition: { Configuration: { 'Test Mode': '2-Site' } },
    contact: { Mode: { 'Head Device Mode': String(opts.filePos === undefined ? 2 : opts.filePos), 'Kit Diameter': opts.kit ? opts.kit.file : '3.0' } },
  };
  const R = {
    read: (n) => Promise.resolve({ sections: clone(RECDOC[n]) }),
    editlistGet(st) {
      CPP.gets++;
      if (CPP.failNext) { CPP.failNext = false; return Promise.reject(new Error('fake C++: editlist.get failed')); }
      const proxies = {                                              // scrbSLK FIRST: the recompute must wait for the rest of the batch
        scrbSLK: { position: CPP.pos },
        rgKitDiameter: { itemIndex: CPP.kit },
        edPinCount: { text: '10' }, edForcePerPinN: { text: '0.2940' }, edForcePerPinG: { text: '30' },
      };
      if (opts.noKitProxy) delete proxies.rgKitDiameter;
      return Promise.resolve({
        struct: st, form: 'TfContact', booted: true,
        lists: { elContact: { entries: [] } },
        proxies,
        mustSend: ['scrbSLK'],
        events: { rgKitDiameter: { event: 'click', operable: true } },
        session: {},
      });
    },
    editlistSave(st, widgets) {
      CPP.saves.push({ st, widgets: clone(widgets) });
      if (widgets.scrbSLK) CPP.pos = widgets.scrbSLK.position;        // golden SaveSetupFile :14296（V912 :14404） writes the proxy's Position
      if (widgets.rgKitDiameter) CPP.kit = widgets.rgKitDiameter.itemIndex;
      return Promise.resolve({ saved: true, session: {} });
    },
    rawCmd(name, extra) {
      const v = JSON.parse(extra.value);
      CPP.cmds.push({ name, tag: extra.tag, v });
      const a = CPP.ack || { changed: {} };
      CPP.ack = null;
      return Promise.resolve({ value: JSON.stringify(a) });
    },
  };

  const msgListeners = [];
  let releaseCaps = null;
  const capsGate = new Promise((r) => { releaseCaps = r; });
  const sb = {
    console: quiet, setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout,
    Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp, isFinite, isNaN, parseFloat, parseInt, Event: undefined,
    HT9045Recipe: R, HT9045Tags: tags,
    // opts.holdCaps: ContactInfo.ini (section 15's option rebuild) answers only on P.releaseCaps() -- a slow file read
    HT9045System: { read: (n) => (opts.holdCaps && n === 'contactInfo' ? capsGate : Promise.resolve()).then(() => ({ sections: clone(SYS[n]) })) },
    confirm: () => true,
    getComputedStyle: () => ({ display: '', visibility: '' }),
    addEventListener(t, f) { if (t === 'message') msgListeners.push(f); },
  };
  sb.window = sb;
  const FRAME = { name: 'background.html' };
  sb.parent = opts.standalone ? sb : FRAME;
  sb.document = {
    readyState: 'complete', body, documentElement: docEl,
    getElementById: (id) => byId[id] || null,
    createElement: (t) => new El(String(t).toUpperCase()),
    createTextNode: (t) => ({ nodeType: 3, nodeValue: t, textContent: t }),
    querySelectorAll: () => [], querySelector: () => null,
    addEventListener(t, f) { (docListeners[t] = docListeners[t] || []).push(f); },
  };
  vm.createContext(sb);
  // Setup.Contact.html order (ratcheted in [0]): recipe client (fake) .. engine, wire data, contact_ev, contact_slk
  vm.runInContext(engineCode, sb, { filename: 'ht9045_wire_engine.js' });
  vm.runInContext(wireCode, sb, { filename: 'ht9045_wire_setupcontact.js' });
  vm.runInContext(evCode, sb, { filename: 'ht9045_contact_ev.js' });
  vm.runInContext(slkCode, sb, { filename: 'ht9045_contact_slk.js' });
  const kitRadios = () => rg.querySelectorAll('input[type="radio"]');
  const P = {
    sb, els, CPP, radios, tags,
    computes: () => airNWrites,
    air: () => els.edAirForce.value,
    api: () => sb.HT9045ContactSLK,
    pos: () => sb.HT9045ContactSLK.position(),
    sync: () => (sb.HT9045ContactSLK && sb.HT9045ContactSLK.sync ? sb.HT9045ContactSLK.sync() : {}),
    reopen: () => (sb.HT9045ContactSLK && sb.HT9045ContactSLK.reopen ? sb.HT9045ContactSLK.reopen() : {}),
    boot: () => (sb.HT9045ContactSLK && sb.HT9045ContactSLK.boot ? sb.HT9045ContactSLK.boot() : {}),
    evState: () => sb.HT9045EvB3Contact.state(),
    key: (k) => slk.fire('keydown', { key: k }),                     // section 14: ArrowDown = +1, ArrowUp = -1
    clickSave() {                                                    // the engine's document capture handler (a real operator click)
      const ev = { type: 'click', target: save, stopPropagation() {}, preventDefault() {} };
      (docListeners.click || []).slice().forEach((f) => f(ev));
    },
    pickKit(i) { const rs = kitRadios(); rs.forEach((r, j) => { r.checked = (j === i); }); rg.fire('change', { isTrusted: true, target: rs[i] }); },
    pickKitText(t) { P.pickKit(P.kitItems().indexOf(t)); },           // the operator clicks the radio labelled t
    kitItems: () => rg.querySelectorAll('label.rgi').map((l) => l.textContent),
    kitIndex: () => { const rs = kitRadios(); for (let i = 0; i < rs.length; i++) if (rs[i].checked) return i; return -1; },
    kitText: () => { const i = P.kitIndex(); return i < 0 ? '' : P.kitItems()[i]; },
    minForce: () => els.lblMinForce.textContent,
    lastSave: () => CPP.saves[CPP.saves.length - 1],
    releaseCaps: () => releaseCaps(),
    post(data, source) { msgListeners.forEach((f) => f({ data, source: source === undefined ? FRAME : source })); },
    win(open, state, initial) { P.post({ type: 'HT_WIN', id: 'contact', open, state, initial: !!initial }); },
  };
  return P;
}

(async () => {
  console.log('d021_selftest -- ' + SLK);

  console.log('[0] ratchets (read only): the page load order, the engine / contact_ev el.value contract, the engine H4 contract and C++ response the fix relies on');
  const at = (s) => html.indexOf(s);
  check(at('src="ht9045_wire_engine.js"') > 0 && at('src="ht9045_wire_engine.js"') < at('src="ht9045_wire_setupcontact.js"') &&
        at('src="ht9045_wire_setupcontact.js"') < at('src="ht9045_contact_ev.js"') && at('src="ht9045_contact_ev.js"') < at('src="ht9045_contact_slk.js"'),
        'Setup.Contact.html loads engine < wire data < contact_ev < contact_slk (the order this test runs them in)');
  check(engineCode.indexOf("if (v.position !== undefined) { el.value = v.position; GB_KIND[id] = 'position'; }") >= 0 &&
        engineCode.indexOf("if (k === 'position') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }") >= 0,
        'engine gbApply writes el.value = v.position, gbValue reads parseInt(el.value) at Save');
  check(evCode.indexOf("if (k === 'position') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }") >= 0 &&
        evCode.indexOf('if (v.position !== undefined) el.value = v.position;') >= 0,
        'contact_ev reads parseInt(el.value) for the form.event state and writes el.value from an ack');
  // AI(W906-D022) 20261001: the engine's H4 (open-only read) that section 20 mirrors, and the response section 17 reads for D-023
  check(engineCode.indexOf('if (m.open && !H4.open) { H4.open = true; if (H4.waiting) load(); }') >= 0 &&
        engineCode.indexOf('if (gbStruct() && window.parent && window.parent !== window) {') >= 0 &&
        engineCode.indexOf('if (!H4.hosted && !H4.timer) H4.timer = setTimeout(function () { H4.timer = null; if (!H4.hosted) load(); }, 3000);') >= 0,
        'engine H4: hosted C-route page reads on the HT_WIN open edge, falls back to a read after 3 s without any HT_WIN (ht9045_wire_engine.js :2064-2072 / :2137-2140)');
  check(engineCode.indexOf('golden: function () { return { struct: gbStruct(), page: GB, kinds: GB_KIND, lastSave: GB_LAST }; }') >= 0 &&
        engineCode.indexOf('return HT9045Recipe.editlistGet(st).then(function (d) {') >= 0 && engineCode.indexOf('GB = d; GB_KIND = {};') >= 0,
        'engine: gbLoad calls HT9045Recipe.editlistGet(st) and keeps the response as HT9045Page.golden().page');
  check(/\{id:'contact',[^}]*src:'page\/Setup\.Contact\.html'[^}]*hidden:true[^}]*\}/.test(bgHtml) &&
        !/\{id:'contact',[^}]*lazy:true/.test(bgHtml),
        'background.html: the contact window is hidden and NOT lazy (its iframe is loaded closed at site start)');

  console.log('[1] D-022 boot, hosted, window closed: no editlist.get until the first open; that read is golden FormShow');
  const A = makePage({ cppPos: 3, filePos: 2 });
  await advance(0);
  check(A.CPP.gets === 0 && A.boot().state === 'wait-win', 'before the first HT_WIN: no editlist.get, the page waits like the engine H4 (got gets ' + A.CPP.gets + ', state ' + A.boot().state + ')');
  A.win(false, 'closed', true);                                     // background.html iframe load: initial state
  await advance(0);
  check(A.CPP.gets === 0 && A.boot().mode === 'closed', 'hosted, closed at boot: 0 editlist.get (golden FormShow runs only on Show; was 1: C++ FormShow + Enter at site start)');
  check(A.computes() === 0 && A.api().state().fShow === false && A.sync().writes === 0,
        'no section 17 compute while closed: fShow false (golden fShow is false until FormShow :1167 (V912 :1183)) (got computes ' + A.computes() + ')');
  await advance(5000);
  check(A.CPP.gets === 0 && A.computes() === 0, '5 s later still nothing (an HT_WIN arrived, so no 3 s fallback read)');
  A.win(true, 'open');                                              // the first open
  await advance(0);
  check(A.CPP.gets === 1, 'first open: exactly ONE editlist.get, the engine H4 read (contact_slk adds none) (got ' + A.CPP.gets + ')');
  check(A.computes() === 1 && A.api().state().fShow === true && A.pos() === 3 && A.els.scrbSLK.value === '3' && A.air() === AIR['30:3'],
        'section 17 ran once after that read = golden FormShow :1167-1170 (V912 :1183-1186): Position 3 from C++ (the recipe file says 2), ' + AIR['30:3'] + ' (got ' + A.computes() + ' / ' + A.pos() + ' / ' + A.air() + ')');
  check(A.api().state().dDutCount === 2 && A.api().state().iHeadDeviceCT === 3 && A.minForce() === 'Min force per compliance: 1.50kg',
        'the section 17 figures: dDutCount 2 (DualSite, all sites), DeviceForm.iHeadDeviceCT 3 (DutCount :2068 (V912 :2097)), min 1.50 kg (30 mm)');
  check(A.sync().cpp === 3 && A.sync().writes === 1 && A.sync().recalcs === 0 && A.reopen().shows === 0 && A.boot().state === 'done',
        'the C++ Position was adopted before fShow -> no extra :1170 (V912 :1186); the D-019 edge recompute was skipped (fShow false at the edge; section 17 is this FormShow)');

  console.log('[2] the operator scrolls; Save and form.event carry the live Position');
  A.key('ArrowDown');
  await advance(0);
  check(A.pos() === 4 && A.els.scrbSLK.value === '4' && A.air() === AIR['30:4'] && A.computes() === 2,
        'scroll down: SB.pos 4, #scrbSLK.value "4", OnChange recomputed once (' + AIR['30:4'] + ') (got ' + A.els.scrbSLK.value + ' / ' + A.air() + ')');
  check(A.sync().writes === 1 && A.sync().recalcs === 0 && A.sync().pending === false, 'no loop: the page move wrote nothing back through #scrbSLK.value and queued no extra recompute');
  check(A.evState().scrbSLK && A.evState().scrbSLK.position === 4, 'contact_ev form.event state carries position 4 (got ' + JSON.stringify(A.evState().scrbSLK) + ')');
  A.clickSave();
  await advance(0);
  const s1 = A.lastSave();
  check(s1 && s1.st === 'DeviceForm_File' && s1.widgets.scrbSLK && s1.widgets.scrbSLK.position === 4,
        'Save (the engine gbSave, via the spbSave click) sends scrbSLK {position: 4} -> C++ ch["scrbSLK"] (DF_DeriveBeforeSave :141) and SaveSetupFile :14296 (V912 :14404) (got ' + JSON.stringify(s1 && s1.widgets.scrbSLK) + ')');
  check(A.sync().writes === 2 && A.sync().recalcs === 1 && A.pos() === 4 && A.air() === AIR['30:4'],
        'the post-save re-read (golden :14154-14155 (V912 :14262-14263); C++ runs FormShow for every get) is adopted and recomputed once (writes 2, recalcs 1)');

  console.log('[3] an unsaved scroll is dropped when the window is closed and opened again (golden FormClose :1847-1849, FormShow :1185-1186)');
  A.key('ArrowDown');
  await advance(0);
  check(A.pos() === 5 && A.air() === AIR['30:5'], 'scroll down again, not saved: 5 (' + AIR['30:5'] + ')');
  const w3 = A.sync().writes, r3 = A.sync().recalcs, sh3 = A.reopen().shows, g3 = A.CPP.gets;
  A.win(false, 'closed');
  await advance(0);
  A.win(true, 'open');
  await advance(0);
  check(A.reopen().shows === sh3 + 1 && A.CPP.gets === g3 + 1, 'the D-019 edge recompute ran, and the engine H4 read once more');
  check(A.pos() === 4 && A.els.scrbSLK.value === '4' && A.air() === AIR['30:4'] && A.api().state().iHeadDeviceCT === 4,
        'reopen: SB.pos back to the file value 4, recomputed (' + AIR['30:4'] + ', iHeadDeviceCT 4) (got ' + A.pos() + ' / ' + A.air() + ')');
  check(A.sync().writes === w3 + 1 && A.sync().recalcs === r3 + 1, 'one write, one recompute after it');
  A.clickSave();
  await advance(0);
  const s2 = A.lastSave();
  check(s2 && s2.widgets.scrbSLK.position === A.pos(), 'what the screen shows is what Save sends (' + A.pos() + ' / ' + (s2 && s2.widgets.scrbSLK.position) + ')');

  console.log('[4] a C++ load that changes only rgKitDiameter: golden :1186 runs AFTER the whole batch (scrbSLK is applied first)');
  A.CPP.kit = 1;
  const r4 = A.sync().recalcs;
  A.sb.HT9045Page.load();                                           // the reload path (same as the post-save re-read)
  await advance(0);
  check(A.pos() === 4 && A.air() === AIR['40:4'] && A.minForce() === 'Min force per compliance: 4.00kg',
        'recomputed with the new 40 mm kit: ' + AIR['40:4'] + ', min 4.00 kg (got ' + A.air() + ' / ' + A.minForce() + ')');
  check(A.sync().recalcs === r4 + 1 && A.sync().pending === false, 'exactly one recompute for the batch');

  console.log('[5] form.event: the state carries the live Position; an ack that moves Position is adopted');
  A.key('ArrowUp');
  await advance(0);
  check(A.pos() === 3 && A.air() === AIR['40:3'], 'scroll up: 3 (' + AIR['40:3'] + ')');
  A.CPP.ack = { changed: { scrbSLK: { position: 2 } } };             // C++ moved Position (e.g. golden DutCount :2090-2095 single-site clamp)
  const w5 = A.sync().writes, r5 = A.sync().recalcs;
  A.pickKit(0);
  await advance(0);
  const cmd = A.CPP.cmds[A.CPP.cmds.length - 1];
  check(cmd && cmd.name === 'form.event' && cmd.v.control === 'rgKitDiameter' && cmd.v.state.scrbSLK && cmd.v.state.scrbSLK.position === 3,
        'rgKitDiameter click -> form.event with state.scrbSLK.position 3 (got ' + JSON.stringify(cmd && cmd.v.state.scrbSLK) + ')');
  check(A.pos() === 2 && A.els.scrbSLK.value === '2' && A.air() === AIR['30:2'] && A.sync().writes === w5 + 1 && A.sync().recalcs === r5 + 1,
        'the ack position 2 is adopted and recomputed once (30 mm, ' + AIR['30:2'] + ') (got ' + A.pos() + ' / ' + A.air() + ')');

  console.log('[6] no loop, one recompute per batch, bad values ignored');
  const w6 = A.sync().writes, r6 = A.sync().recalcs, c6 = A.computes();
  A.els.scrbSLK.value = 5; A.els.scrbSLK.value = 5; A.els.scrbSLK.value = 4;
  check(A.pos() === 4 && A.sync().writes === w6 + 3 && A.sync().pending === true, 'three writes in one batch: adopted at once (4), one recompute queued');
  await advance(0);
  check(A.sync().recalcs === r6 + 1 && A.computes() === c6 + 1 && A.air() === AIR['30:4'], '... it runs once (' + AIR['30:4'] + ')');
  A.els.scrbSLK.value = 'abc';
  await advance(0);
  check(A.pos() === 4 && A.sync().writes === w6 + 3 && A.sync().recalcs === r6 + 1, 'a non-integer write is ignored');
  A.els.scrbSLK.value = 9;
  await advance(0);
  check(A.pos() === 5 && A.els.scrbSLK.value === '5', 'a write above Max is clamped to Max 5 (VCL / ELTrackBar SetPosition)');
  const w6b = A.sync().writes, r6b = A.sync().recalcs, c6b = A.computes();
  A.key('ArrowUp'); A.key('ArrowUp'); A.key('Home');
  await advance(10);
  check(A.pos() === 2 && A.computes() === c6b + 3 && A.sync().writes === w6b && A.sync().recalcs === r6b,
        'three page moves: three OnChange recomputes, nothing written back, nothing queued');

  console.log('[7] minimized is open: no engine re-read, no edge, no recompute');
  const g7 = A.CPP.gets, sh7 = A.reopen().shows, r7 = A.sync().recalcs, c7 = A.computes();
  A.win(true, 'minimized'); await advance(0);
  A.win(true, 'open'); await advance(0);
  check(A.CPP.gets === g7 && A.reopen().shows === sh7 && A.sync().recalcs === r7 && A.computes() === c7, 'open -> minimized -> open changes nothing');

  console.log('[8] HT9046 (Max 6), window open at boot: the C++ Position 6 survives boot (Max is set in section 17 before the clamp)');
  const B = makePage({ model: 'HT-9046', cppPos: 6, filePos: 2 });
  B.win(true, 'open', true);                                        // lazy window / refresh while open: the first HT_WIN is open
  await advance(0);
  check(B.pos() === 6 && B.air() === AIR['30:6'] && B.computes() === 1, 'SB.pos 6 (x0.125), ' + AIR['30:6'] + ', one boot compute (got ' + B.pos() + ' / ' + B.air() + ' / ' + B.computes() + ')');

  console.log('[9] standalone page (no background.html): the engine reads at once; still one boot compute with the C++ Position');
  const S = makePage({ standalone: true, cppPos: 5, filePos: 2 });
  await advance(0);
  check(S.pos() === 5 && S.air() === AIR['30:5'] && S.computes() === 1 && S.sync().recalcs === 0,
        'SB.pos 5, ' + AIR['30:5'] + ', one compute (got ' + S.pos() + ' / ' + S.air() + ' / ' + S.computes() + ' / recalcs ' + S.sync().recalcs + ')');
  check(S.CPP.gets === 2 && S.boot().mode === 'page', 'D-022 standalone = as before: the engine attach read + the page boot read = 2 editlist.get (got ' + S.CPP.gets + ')');
  S.key('ArrowUp');
  await advance(0);
  S.clickSave();
  await advance(0);
  const s9 = S.lastSave();
  check(s9 && s9.widgets.scrbSLK.position === 4 && S.pos() === 4, 'scroll + Save sends 4');

  console.log('[10] D-022 hosted, window already open at boot (lazy first open / refresh while open / minimized): as before');
  const O = makePage({ cppPos: 4 });
  O.win(true, 'open', true);
  await advance(0);
  check(O.CPP.gets === 2 && O.boot().mode === 'open', 'the engine H4 read + the page boot read = 2 editlist.get, as before (got ' + O.CPP.gets + ')');
  check(O.computes() === 1 && O.pos() === 4 && O.air() === AIR['30:4'] && O.reopen().shows === 0,
        'one section 17 compute with the C++ Position 4 (' + AIR['30:4'] + '); never-told -> open is not a D-019 edge');
  const M = makePage({ cppPos: 4 });
  M.win(true, 'minimized', true);
  await advance(0);
  check(M.CPP.gets === 2 && M.computes() === 1 && M.air() === AIR['30:4'], 'minimized at boot counts as open: 2 reads, one compute');

  console.log('[11] D-022 hosted but no HT_WIN at all (old frame / embedded by another page): the engine H4 3 s fallback, the page reads with it');
  const N = makePage({ cppPos: 3 });
  await advance(2999);
  check(N.CPP.gets === 0 && N.computes() === 0, 'before 3 s: no read (the engine waits the same 3 s) (got ' + N.CPP.gets + ')');
  await advance(2);
  check(N.CPP.gets === 2 && N.boot().mode === 'no-ht-win' && N.computes() === 1 && N.pos() === 3 && N.air() === AIR['30:3'],
        'at 3 s: the engine read + the page read, one compute with Position 3 (got ' + N.CPP.gets + ' / ' + N.computes() + ' / ' + N.air() + ')');

  console.log('[12] D-022 the first-open figures use the site states of that moment');
  const T = makePage({ cppPos: 3 });
  T.win(false, 'closed', true);
  await advance(0);
  T.tags.frame(S1_OFF);                                             // review6 (no stage F): tags keep flowing while closed
  await advance(0);
  check(T.computes() === 0, 'closed: a site change computes nothing (fShow false)');
  T.win(true, 'open');
  await advance(0);
  check(T.CPP.gets === 1 && T.computes() === 1 && T.api().state().dDutCount === 1 && T.air() === AIR['30:3:1dut'],
        'first open: DutCount :2001-2002 sees site 1 closed on both arms -> dDutCount 1, ' + AIR['30:3:1dut'] + ' (got ' + T.api().state().dDutCount + ' / ' + T.air() + ')');
  const U = makePage({ cppPos: 3 });                               // stage F: no frames while closed, the snapshot lands after the read
  U.win(false, 'closed', true);
  await advance(0);
  U.win(true, 'open');
  await advance(0);
  check(U.computes() === 1 && U.air() === AIR['30:3'], 'first open before the snapshot: section 17 with BOOT_TAGS (all sites) -> ' + AIR['30:3']);
  await advance(30);
  U.tags.frame(S1_OFF);                                             // the synthetic snapshot (ht9045_link.js Hub.windowState)
  await advance(0);
  check(U.computes() === 2 && U.air() === AIR['30:3:1dut'] && U.reopen().follows === 1,
        'the snapshot 30 ms later -> the D-019 follow recomputes once (' + AIR['30:3:1dut'] + ') (got ' + U.computes() + ' / ' + U.air() + ')');

  console.log('[13] D-022 the first-open read fails: section 17 still runs (as the old load().catch), with the recipe file values');
  const F = makePage({ cppPos: 3, filePos: 2, failFirstGet: true });
  F.win(false, 'closed', true);
  await advance(0);
  F.win(true, 'open');
  await advance(0);
  check(F.CPP.gets === 1 && F.computes() === 1 && F.api().state().fShow === true && F.pos() === 2 && F.air() === AIR['30:2'],
        'one failed read, one compute with the file Position 2 (' + AIR['30:2'] + ') (got ' + F.CPP.gets + ' / ' + F.computes() + ' / ' + F.pos() + ')');
  F.win(false, 'closed'); await advance(0);
  F.win(true, 'open'); await advance(0);
  check(F.CPP.gets === 2 && F.pos() === 3 && F.air() === AIR['30:3'], 'the next open reads C++ and adopts Position 3 (' + AIR['30:3'] + ')');

  // ------------------------------------------------------------------------------------------------------------------- D-023
  const MACHINE = { types: '30,40,60,56,80', visible: '1,1,1,0,1' };  // D:\HT9045\system\ContactInfo.ini [SLK Type] on this machine
  console.log('[14] D-023 options = the bShow sizes (golden FormShow :1124-1144); 80 mm recipe -> the C++ ItemIndex (DoIniDataToForm :896-903)');
  const K = makePage({ cppPos: 2, filePos: 2, kit: Object.assign({ file: '8.0' }, MACHINE), cppKit: 3 });   // golden list 30,40,60,80: 80 = 3
  K.win(false, 'closed', true);
  await advance(0);
  check(K.kitItems().join(',') === '30,40,60,80', 'rgKitDiameter rebuilt at boot to 30,40,60,80: the Visible=0 size 56 is not listed (got ' + K.kitItems().join(',') + ')');
  K.win(true, 'open');
  await advance(0);
  check(K.kitIndex() === 3 && K.kitText() === '80' && K.boot().kit === 'cpp',
        'first open: ItemIndex 3 = "80", the C++ value (got ' + K.kitIndex() + ' "' + K.kitText() + '" / ' + K.boot().kit + ')');
  check(K.minForce() === 'Min force per compliance: 15.00kg' && K.air() === AIR['80:2'],
        'figures with 80 mm: min 15.00 kg (GetMinForce), ' + AIR['80:2'] + ' (got ' + K.minForce() + ' / ' + K.air() + ')');
  K.clickSave();
  await advance(0);
  const k1 = K.lastSave();
  check(k1 && k1.widgets.rgKitDiameter && k1.widgets.rgKitDiameter.itemIndex === 3 && K.kitText() === '80',
        'Save sends rgKitDiameter {itemIndex: 3} = "80" in the C++ list, and the screen shows "80" (the old page list 30,40,60,56,80 had "80" at 4, past the 4-item C++ list) (got ' +
        JSON.stringify(k1 && k1.widgets.rgKitDiameter) + ' / "' + K.kitText() + '")');
  K.pickKitText('60');
  await advance(0);
  K.pickKitText('80');
  await advance(0);
  const kc = K.CPP.cmds[K.CPP.cmds.length - 1];
  check(kc && kc.name === 'form.event' && kc.v.control === 'rgKitDiameter' && kc.v.itemIndex === 3,
        'clicking "80" sends form.event itemIndex 3 (got ' + JSON.stringify(kc && { control: kc.v.control, itemIndex: kc.v.itemIndex }) + ')');

  console.log('[15] D-023 a recipe size that is not listed (5.6 with 56 hidden): golden DoIniDataToForm :915-917 picks item 0, the page follows C++');
  const H = makePage({ cppPos: 2, kit: Object.assign({ file: '5.6' }, MACHINE), cppKit: 0 });
  H.win(false, 'closed', true);
  await advance(0);
  H.win(true, 'open');
  await advance(0);
  check(H.kitIndex() === 0 && H.kitText() === '30' && H.minForce() === 'Min force per compliance: 1.50kg' && H.air() === AIR['30:2'],
        '"30" as C++ / golden, min 1.50 kg, ' + AIR['30:2'] + ' (was "56" from the recipe file) (got "' + H.kitText() + '" / ' + H.minForce() + ')');

  console.log('[16] D-023 no usable C++ value: the recipe-file mapping as before');
  const X = makePage({ cppPos: 2, kit: Object.assign({ file: '8.0' }, MACHINE), noKitProxy: true });
  X.win(false, 'closed', true);
  await advance(0);
  X.win(true, 'open');
  await advance(0);
  check(X.kitText() === '80' && X.kitIndex() === 3 && X.boot().kit === 'file', 'no rgKitDiameter in the C++ response: "80" from [Mode] Kit Diameter 8.0, source file');
  const Y = makePage({ cppPos: 2, kit: Object.assign({ file: '4.0' }, MACHINE), cppKit: 7 });
  Y.win(false, 'closed', true);
  await advance(0);
  Y.win(true, 'open');
  await advance(0);
  check(Y.kitText() === '40' && Y.boot().kit === 'file', 'a C++ ItemIndex out of the page options (7): "40" from the recipe file, source file');

  console.log('[17] D-023 standalone: the engine applies the C++ ItemIndex to the dfm options before section 15 rebuilds them; section 17 puts it back');
  const Z = makePage({ standalone: true, cppPos: 2, kit: Object.assign({ file: '8.0' }, MACHINE), cppKit: 3 });
  await advance(0);
  check(Z.kitItems().join(',') === '30,40,60,80' && Z.kitIndex() === 3 && Z.kitText() === '80' && Z.boot().kit === 'cpp' && Z.air() === AIR['80:2'],
        '"80" (ItemIndex 3 from C++) after the rebuild, ' + AIR['80:2'] + ' (got ' + Z.kitItems().join(',') + ' / ' + Z.kitIndex() + ' / ' + Z.air() + ')');
  Z.clickSave();
  await advance(0);
  const z1 = Z.lastSave();
  check(z1 && z1.widgets.rgKitDiameter && z1.widgets.rgKitDiameter.itemIndex === 3,
        'Save right after boot goes through with itemIndex 3: the page boot read runs after loadCaps, so its itemIndex lands on the rebuilt options ' +
        '(was: refused, the engine had applied 3 to the 3 dfm radios -> GB_UNFILL) (got ' + JSON.stringify(z1 && z1.widgets.rgKitDiameter) + ')');
  const Z2 = makePage({ standalone: true, cppPos: 2, kit: Object.assign({ file: '8.0' }, MACHINE), cppKit: 3, holdCaps: true });
  await advance(0);
  check(Z2.CPP.gets === 1 && Z2.kitItems().join(',') === '30,40,60',
        'a slow ContactInfo read: only the engine attach read so far (it met the 3 dfm radios); the page boot read waits for loadCaps (got ' + Z2.CPP.gets + ')');
  Z2.releaseCaps();
  await advance(0);
  Z2.clickSave();
  await advance(0);
  const z2 = Z2.lastSave();
  check(Z2.CPP.gets === 3 && Z2.kitText() === '80' && z2 && z2.widgets.rgKitDiameter && z2.widgets.rgKitDiameter.itemIndex === 3,
        '... then the page read lands on the rebuilt options: "80", and Save (not refused) sends itemIndex 3 (got ' + Z2.CPP.gets + ' / "' + Z2.kitText() + '" / ' +
        JSON.stringify(z2 && z2.widgets.rgKitDiameter) + ')');
  const Q = makePage({ cppPos: 2, kit: Object.assign({ file: '8.0' }, MACHINE), cppKit: 3 });   // hosted, open at boot (refresh while open)
  Q.win(true, 'open', true);
  await advance(0);
  const qg = Q.CPP.gets, qk = Q.kitText();
  Q.clickSave();                                                    // (its post-save re-read is one more editlist.get)
  await advance(0);
  const q1 = Q.lastSave();
  check(qg === 2 && qk === '80' && q1 && q1.widgets.rgKitDiameter && q1.widgets.rgKitDiameter.itemIndex === 3,
        'hosted, open at boot: 2 reads at boot as before, "80" shown, Save sends itemIndex 3 (got ' + qg + ' / "' + qk + '" / ' + JSON.stringify(q1 && q1.widgets.rgKitDiameter) + ')');

  console.log('d021_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
