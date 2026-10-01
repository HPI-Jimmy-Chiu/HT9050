// AI(W906-TA5) 20261001 [W906] St01: offline selftest for TA-5, the page half (ctest TA5_ScrollPage).
//   Setup.TrayAssignment.html's two graphic-mode scroll bars (golden V912 cTrayAssignment.cpp:1639 sbNormalTestChange -> GraphicToRadio,
//   :1674 sbNormalTest_RTChange -> GraphicToRadio_RT; DFM cTrayAssignment.dfm:569 / :592 TScrollBar Min 0 Max 15, OnChange) were static
//   divs. web/page/ht9045_trayassign_ev.js (TA-5 section) makes them operable scroll bars that send
//   form.event {"form":"TfTrayAssignment","control":"sbNormalTest","event":"change","position":n,"state":{...}} (X-2) on every step,
//   applies ack.changed, puts the position back on a refusal, and turns #sbNormalTest.value into an accessor (a read = the live position
//   for the engine's Save / the event state; a write = C++'s Position from an engine load or an ack, which only moves the thumb).
//   Loads the REAL ht9045_trayassign_ev.js into a node vm with a fake DOM, a fake HT9045Recipe (editlist.get / form.event scripted),
//   a fake HT9045Page and a fake clock. Setup.TrayAssignment.html is read only (ids + load-order ratchet). No socket, no wb_serve,
//   no file written.  Usage: node tools/webprobe/ta5_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_TRAYASSIGN_EV=<the pre-TA-5 ht9045_trayassign_ev.js>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_TRAYASSIGN_EV || path.join(PAGE, 'ht9045_trayassign_ev.js');
const code = fs.readFileSync(SRC, 'utf8');
const html = fs.readFileSync(path.join(PAGE, 'Setup.TrayAssignment.html'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock (same as d015_menu_selftest.cjs)
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

// ---------------------------------------------------------------- fake DOM
const byId = {};
class El {
  constructor(tag, id) {
    this.tagName = tag; this.children = []; this.parentNode = null; this.parentElement = null;
    this.style = {}; this.attrs = {}; this.listeners = {}; this.textContent = ''; this.title = '';
    this.clientHeight = 89; this.offsetHeight = 5;          // the DFM height of both bars (cTrayAssignment.dfm:573 / :596)
    this.qs = {};                                           // selector -> result for querySelector (per test)
    this.qsa = {};                                          // selector -> array for querySelectorAll
    if (id) { this.id = id; byId[id] = this; }
  }
  get firstChild() { return this.children[0] || null; }
  appendChild(c) { c.parentNode = this; c.parentElement = this; this.children.push(c); return c; }
  removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; c.parentElement = null; return c; }
  setAttribute(k, v) { this.attrs[k] = String(v); }
  getAttribute(k) { return k in this.attrs ? this.attrs[k] : null; }
  removeAttribute(k) { delete this.attrs[k]; }
  addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); }
  removeEventListener(t, f) { this.listeners[t] = (this.listeners[t] || []).filter((x) => x !== f); }
  querySelector(sel) { return this.qs[sel] !== undefined ? this.qs[sel] : null; }
  querySelectorAll(sel) { return this.qsa[sel] || []; }
  getBoundingClientRect() { return { top: 0, height: this.clientHeight }; }
  focus() {}
  get classList() { const self = this; return { contains(c) { return (self.className || '').split(/\s+/).indexOf(c) >= 0; }, add() {}, remove() {} }; }
}
function fire(target, type, props) {                        // bubble from the target up (no capture listeners are used here)
  const ev = Object.assign({ type, target, isTrusted: true, preventDefault() { this.defaultPrevented = true; }, stopPropagation() { this.stopped = true; } }, props || {});
  for (let n = target; n && !ev.stopped; n = n.parentNode) (n.listeners[type] || []).slice().forEach((f) => f(ev));
  return ev;
}
function radioGroup(id, n) {
  const g = new El('FIELDSET', id);
  const rs = [];
  for (let i = 0; i < n; i++) { const r = new El('INPUT'); r.type = 'radio'; r.checked = false; g.appendChild(r); rs.push(r); }
  g.qsa['input[type="radio"]'] = rs;
  return g;
}
const docListeners = {};
const document = {
  readyState: 'complete',
  getElementById: (id) => byId[id] || null,
  querySelector: () => null,
  querySelectorAll: () => [],
  createElement: (t) => new El(t.toUpperCase()),
  addEventListener(t, f) { (docListeners[t] = docListeners[t] || []).push(f); },
  removeEventListener(t, f) { docListeners[t] = (docListeners[t] || []).filter((x) => x !== f); },
};
// the page: the two scroll bars as Setup.TrayAssignment.html:56 has them (a div with one static thumb child), four groups, pgRunMode
const sbA = new El('DIV', 'sbNormalTest'); sbA.appendChild(new El('DIV'));
const sbB = new El('DIV', 'sbNormalTest_RT'); sbB.appendChild(new El('DIV'));
['RGAuto3', 'RGAuto2', 'RGAuto1', 'RGLoader', 'rgAuto3_RT', 'rgLoad_RT'].forEach((id) => radioGroup(id, 2));
const pc = new El('DIV', 'pgRunMode');
const tabAct = new El('DIV'); tabAct.attrs['data-t'] = '2'; tabAct.className = 'tab act';
pc.qs[':scope > .pcTabs > .tab.act'] = tabAct;
const tab1 = new El('DIV'); tab1.attrs['data-t'] = '1'; tab1.className = 'tab'; let tab1Clicks = 0; tab1.click = () => { tab1Clicks++; };
pc.qs[':scope > .pcTabs > .tab[data-t="1"]'] = tab1;

// ---------------------------------------------------------------- fake C++ (HT9045Recipe) and engine (HT9045Page)
const cmds = [], says = [];
const script = [];                                          // next rawCmd answers: 'ok:<changed json>' | 'err:<message>'
const R = {
  status() { return { connected: true, holdsToken: true }; },
  keepAlive() { return Promise.resolve(); },
  editlistGet(st) { cmds.push({ name: 'editlist.get', st }); return Promise.resolve(JSON.parse(JSON.stringify(PAGEDATA))); },
  editlistSave() { return Promise.resolve({}); },
  rawCmd(name, extra) {
    cmds.push({ name, tag: extra && extra.tag, value: extra && extra.value, at: clock });
    const next = script.length ? script.shift() : 'ok:{}';
    if (next.indexOf('err:') === 0) return Promise.reject(new Error(next.slice(4)));
    return Promise.resolve(Object.assign({ type: 'ack', ok: true }, { changed: JSON.parse(next.slice(3)) }));
  },
};
const PAGEDATA = {
  struct: 'TrayForm', form: 'TfTrayAssignment',
  proxies: { pgRunMode: { activePageIndex: 1 }, sbNormalTest: { position: 7 }, sbNormalTest_RT: { position: 0 } },
  events: {
    sbNormalTest: { event: 'change', golden: 'cTrayAssignment.cpp:1639 TfTrayAssignment::sbNormalTestChange', operable: true },
    sbNormalTest_RT: { event: 'change', golden: 'cTrayAssignment.cpp:1674 TfTrayAssignment::sbNormalTest_RTChange', operable: false },
    RGAuto3: { event: 'click', golden: 'cTrayAssignment.cpp:1197 TfTrayAssignment::cbEmptyChange', operable: true },
  },
  session: { messages: [] },
};
const sb = {
  console: { info() {}, warn() {}, log() {}, error(...a) { console.error(...a); } },
  setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp, isNaN, parseInt,
  HT9045Recipe: R,
  HT9045Page: { golden() { return { kinds: { sbNormalTest: 'position', sbNormalTest_RT: 'position', RGAuto1: 'itemIndex', RGAuto3: 'itemIndex' } }; } },
  HT9045Wire: { say(msg) { says.push(msg); } },
  document,
};
sb.window = sb;
vm.createContext(sb);
vm.runInContext(code, sb, { filename: 'ht9045_trayassign_ev.js' });

const evs = () => cmds.filter((c) => c.name === 'form.event');
const val = (c) => { try { return JSON.parse(c.value); } catch (e) { return null; } };
const arrows = (bar) => ({ up: bar.children[0], down: bar.children[1], thumb: bar.children[2] });
const radio = (id) => byId[id].qsa['input[type="radio"]'].findIndex((r) => r.checked);

(async () => {
  console.log('ta5_selftest -- ' + SRC);

  console.log('[0] Setup.TrayAssignment.html (read-only): the two TScrollBar divs, the script after the engine');
  check(/<div id="sbNormalTest" title="sbNormalTest : TScrollBar"/.test(html) && /<div id="sbNormalTest_RT" title="sbNormalTest_RT : TScrollBar"/.test(html),
        'html has div#sbNormalTest / div#sbNormalTest_RT (TScrollBar)');
  const iEngine = html.indexOf('<script src="ht9045_wire_engine.js">'), iEv = html.indexOf('<script src="ht9045_trayassign_ev.js">');
  check(iEngine >= 0 && iEv > iEngine, 'ht9045_trayassign_ev.js loads after ht9045_wire_engine.js');

  console.log('[1] built at load: operable scroll bars (golden DFM Min 0 Max 15), value accessor');
  const A = arrows(sbA);
  check(sbA.children.length === 3 && A.up.textContent === '▲' && A.down.textContent === '▼' && sbA.getAttribute('role') === 'slider' &&
        sbA.getAttribute('aria-valuemax') === '15', 'sbNormalTest: static thumb replaced by up / down / thumb, role=slider, max 15');
  check(sbB.children.length === 3, 'sbNormalTest_RT built the same way');
  const d = Object.getOwnPropertyDescriptor(sbA, 'value');
  check(!!d && typeof d.get === 'function' && typeof d.set === 'function', '#sbNormalTest.value is an accessor');

  console.log('[2] editlist.get: the engine writes C++\'s Position -> thumb only, no event');
  const p0 = R.editlistGet('TrayForm');                     // the wrapped get (the engine calls this one)
  const got = await p0;
  sbA.value = got.proxies.sbNormalTest.position;            // engine gbApply: el.value = v.position
  sbB.value = got.proxies.sbNormalTest_RT.position;
  await advance(0);                                         // the script's setTimeout(0): hook / tab / refreshKnown
  check(sb.HT9045EvB2TrayAssign.scroll('sbNormalTest').pos === 7 && sbA.value === '7', 'Position 7 adopted (read back "7")');
  check(evs().length === 0, 'no form.event for C++\'s own Position (golden FormShow already ran RadioToGraphic :670)');
  check(tab1Clicks === 1, 'open tab from proxies.pgRunMode.activePageIndex (existing B2 behaviour, still there)');

  console.log('[3] user step -> form.event with position (X-2), state without the scroll bar itself');
  script.push('ok:' + JSON.stringify({ RGAuto3: { itemIndex: 1 }, RGAuto2: { itemIndex: 0 }, RGAuto1: { itemIndex: 0 }, RGLoader: { itemIndex: 1 } }));
  fire(A.down, 'mousedown', { clientY: 80 });
  await advance(0);
  check(evs().length === 1, 'one step down -> one form.event');
  const v1 = evs()[0] && val(evs()[0]);
  check(evs()[0] && evs()[0].tag === 'Setup.TrayAssignment' && v1 && v1.form === 'TfTrayAssignment' && v1.control === 'sbNormalTest' &&
        v1.event === 'change' && v1.position === 8, 'payload {form TfTrayAssignment, control sbNormalTest, event change, position 8} to tag Setup.TrayAssignment');
  check(v1 && v1.state && !('sbNormalTest' in v1.state) && !('RGAuto3' in v1.state) && v1.state.pgRunMode && v1.state.pgRunMode.activePageIndex === 2,
        'state: no sbNormalTest / RGAuto3 (own events), carries pgRunMode activePageIndex 2 (the tab on screen)');
  check(radio('RGAuto3') === 1 && radio('RGAuto2') === 0 && radio('RGLoader') === 1, 'ack.changed applied to the radio groups (C++ ran golden GraphicToRadio)');
  check(evs().length === 1, 'applying the ack sent nothing more');
  check(sbA.value === '8', 'engine Save would read the live position "8" (gbValue parseInt(el.value))');

  console.log('[4] ack writes the position (golden handler moved it) -> thumb only');
  script.push('ok:' + JSON.stringify({ sbNormalTest: { position: 15 } }));
  fire(sbA, 'keydown', { key: 'ArrowUp' });
  await advance(0);
  check(evs().length === 2 && val(evs()[1]).position === 7, 'ArrowUp -> position 7 sent');
  check(sb.HT9045EvB2TrayAssign.scroll('sbNormalTest').pos === 15 && evs().length === 2, 'ack position 15 adopted, no extra event');

  console.log('[5] refusal -> put back');
  script.push('err:bad-payload: sbNormalTest cannot be operated now (it or a container is disabled or hidden after golden FormShow / DFM) -- reload the page');
  const saysBefore = says.length;
  fire(A.up, 'mousedown', { clientY: 1 });
  await advance(0);
  check(evs().length === 3 && val(evs()[2]).position === 14, 'Up -> position 14 sent');
  check(sb.HT9045EvB2TrayAssign.scroll('sbNormalTest').pos === 15, 'refused -> back to 15 (golden: no handler ran, the page cannot stay on the new value)');
  check(says.length > saysBefore && /sbNormalTest/.test(says[says.length - 1]) && /改回/.test(says[says.length - 1]), 'status line says why');

  console.log('[6] not operable (events.operable=false) -> nothing sent, put back');
  const B = arrows(sbB);
  fire(B.down, 'mousedown', { clientY: 80 });
  await advance(0);
  check(evs().length === 3 && sb.HT9045EvB2TrayAssign.scroll('sbNormalTest_RT').pos === 0, 'sbNormalTest_RT operable=false: no form.event, position stays 0');
  script.push('ok:' + JSON.stringify({ tsReTestGraph: { tabVisible: true } }));
  fire(A.down, 'mousedown', { clientY: 80 });                // (15 is the max: this step does not change anything -> nothing sent)
  await advance(0);
  check(evs().length === 3, 'at Max 15 a step down changes nothing -> nothing sent (VCL: no OnChange without a change)');
  fire(sbA, 'keydown', { key: 'Home' });
  await advance(0);
  check(evs().length === 4 && val(evs()[3]).position === 0, 'Home -> position 0 sent; its ack shows tsReTestGraph (golden ShowCompnet :1067-1068)');
  script.push('ok:{}');
  fire(B.down, 'mousedown', { clientY: 80 });
  await advance(0);
  check(evs().length === 5 && val(evs()[4]).control === 'sbNormalTest_RT' && val(evs()[4]).position === 1,
        'after the tab became visible the RT bar is operable -> position 1 sent (the server rechecks anyway)');

  console.log('[7] wheel, and the engine value write while idle');
  script.push('ok:{}');
  fire(sbA, 'wheel', { deltaY: 100 });
  await advance(0);
  check(evs().length === 6 && val(evs()[5]).position === 1, 'wheel down -> position 1 sent');
  sbA.value = 'x';
  check(sbA.value === '1', 'a non-number write is ignored');
  sbA.value = 99;
  check(sbA.value === '15' && evs().length === 6, 'an out-of-range write is clamped to 15 (VCL), no event');

  console.log('ta5_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
