// AI(W906-GEARRATIO) 20261002 [W906]: offline selftest for the Motor Test "Gear Ratio" tab, the page half (ctest MT_GearRatioPage).
//   RULINGS_20261002 #22, NB2 spec §4 / §8 (webprobe list: the 6th tab switches and hides, mtGear* is not caught by the generic
//   binders, the keypad is DOUBLE, file:// greys the whole tab) + the flow (Start = take-up, arrival read from the runtime only,
//   readings signed with the direction, Preview -> Save with the preview key, the second confirmation, STOP, timeout, refusals).
//   Loads the REAL web/page/ht9045_mt_gearratio.js into a node vm with a fake DOM (PageControl1 with the page's 5 tabs, the page's
//   own tab binder as HW.MotorTest.html :89-97 has it), a fake HTMtPageApi / HTMotorAccess / HTQwerty / HT9045Recipe / HT9045Page,
//   a fake runtime (the C++ /api/struct/motor/runtime shape with gearCal) and a fake clock. HW.MotorTest.html is read only (hooks
//   ratchet). No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/mt_gearratio_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_MT_GEAR_JS=<a broken copy of ht9045_mt_gearratio.js>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_MT_GEAR_JS || path.join(PAGE, 'ht9045_mt_gearratio.js');
const CODE = fs.readFileSync(SRC, 'utf8');
const HTML = fs.readFileSync(path.join(PAGE, 'HW.MotorTest.html'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- HW.MotorTest.html: the four same-line hooks (read only)
{
  const lines = HTML.split(/\r?\n/);
  const tagLine = lines.findIndex((l) => l.indexOf('<script src="ht9045_mt_gearratio.js"></script>') >= 0);
  const wireLine = lines.findIndex((l) => l.indexOf('<script src="ht9045_wire_hwmotortest.js"></script>') >= 0);
  check(tagLine >= 0 && tagLine === wireLine && lines[tagLine].indexOf('ht9045_wire_hwmotortest.js') < lines[tagLine].indexOf('ht9045_mt_gearratio.js'),
        'page: the gear script is loaded on the wire-data script line, after it (same line: no line number moves)');
  check(/onAck:function\(req, ack, err\)\{ if\(window\.HTMtGear&&HTMtGear\.onAck\(req,ack,err\)\)return;/.test(HTML), 'page: onAck hands every reply to HTMtGear.onAck first');
  check(/onFinish:function\(was\)\{ if\(window\.HTMtGear&&HTMtGear\.onFinish\)HTMtGear\.onFinish\(was\);/.test(HTML), 'page: onFinish calls HTMtGear.onFinish');
  const apiLine = lines.find((l) => l.indexOf('window.HTMtPageApi=') >= 0) || '';
  check(apiLine.indexOf('window.HTMtPageApi=') < apiLine.indexOf('loadAndBind();') && /selectMotor:selectMotor/.test(apiLine) && /dbReload:dbReload/.test(apiLine) &&
        /runtime:function\(\)\{return motorRuntime;\}/.test(apiLine) && /source:function\(\)\{return modeSource;\}/.test(apiLine),
        'page: HTMtPageApi (selectMotor, runtime, dbReload, source) is defined on the loadAndBind line, before the call');
  check(HTML.indexOf("p.style.display=(p.dataset.p===tb.dataset.t)?'block':'none';") >= 0, 'page: the tab binder this test emulates is still the page\'s (:89-97)');
  check(!/HTMotorAccess\.init/.test(CODE.replace(/\/\/.*$/gm, '').replace(/\/\*[\s\S]*?\*\//g, '')), 'the gear script never calls HTMotorAccess.init (it would replace the page\'s whole configuration)');
  check(!/btnReloadMotorData|reloadMotorData/.test(CODE.replace(/\/\/.*$/gm, '').replace(/\/\*[\s\S]*?\*\//g, '')), 'the gear script never sends reloadMotorData (it zeroes every 1203 axis and clears every HomeFlag)');
}

// ---------------------------------------------------------------- fake clock
let clock = 1000000, timers = [], tid = 1;
function addTimer(fn, ms, every) { const id = tid++; timers.push({ id, due: clock + Math.max(0, ms | 0), fn, every: every ? Math.max(1, ms | 0) : 0 }); return id; }
function clearTimer(id) { timers = timers.filter((t) => t.id !== id); }
function advance(ms) {
  const end = clock + ms;
  for (;;) {
    timers.sort((a, b) => a.due - b.due || a.id - b.id);
    const t = timers[0];
    if (!t || t.due > end) break;
    clock = t.due;
    if (t.every) t.due += t.every; else timers.shift();
    t.fn();
  }
  clock = end;
}

// ---------------------------------------------------------------- fake DOM
let curReg = {};                                            // the id registry of the world being built
class El {
  constructor(tag) {
    this.reg = curReg;
    this.tagName = String(tag).toUpperCase(); this.children = []; this.parentNode = null;
    this.style = {}; this.attrs = {}; this.listeners = []; this._text = ''; this.className = '';
  }
  get firstChild() { return this.children[0] || null; }
  get id() { return this.attrs.id || ''; }
  appendChild(c) { if (c.parentNode) c.parentNode.removeChild(c); c.parentNode = this; this.children.push(c); return c; }
  removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; return c; }
  setAttribute(k, v) {
    v = String(v);
    if (k === 'class') this.className = v; else this.attrs[k] = v;
    if (k === 'id') this.reg[v] = this;
    if (k === 'style') v.split(';').forEach((d) => { const i = d.indexOf(':'); if (i > 0) this.style[d.slice(0, i).trim()] = d.slice(i + 1).trim(); });   // the browser parses it
  }
  getAttribute(k) { if (k === 'class') return this.className; return k in this.attrs ? this.attrs[k] : null; }
  removeAttribute(k) { delete this.attrs[k]; }
  get dataset() { const d = {}; for (const k of Object.keys(this.attrs)) if (k.indexOf('data-') === 0) d[k.slice(5)] = this.attrs[k]; return d; }
  get textContent() { return this._text + this.children.map((c) => c.textContent).join(''); }
  set textContent(v) { this.children.forEach((c) => { c.parentNode = null; }); this.children = []; this._text = String(v); }
  set innerHTML(v) { if (v !== '') throw new Error('fake DOM: innerHTML with markup is not supported (' + String(v).slice(0, 40) + ')'); this.textContent = ''; }
  get classList() {
    const self = this;
    const list = () => self.className.split(/\s+/).filter(Boolean);
    return {
      contains: (c) => list().indexOf(c) >= 0,
      add: (c) => { if (list().indexOf(c) < 0) self.className = list().concat([c]).join(' '); },
      remove: (c) => { self.className = list().filter((x) => x !== c).join(' '); },
      toggle: (c, force) => { const on = force === undefined ? list().indexOf(c) < 0 : !!force; if (on) self.classList.add(c); else self.classList.remove(c); return on; },
    };
  }
  addEventListener(type, fn, capture) { this.listeners.push({ type, fn, capture: !!capture }); }
  removeEventListener(type, fn) { this.listeners = this.listeners.filter((l) => !(l.type === type && l.fn === fn)); }
  querySelector(sel) { return this.querySelectorAll(sel)[0] || null; }
  querySelectorAll(sel) {
    const m = /^:scope > \.([\w-]+)$/.exec(sel);
    if (m) return this.children.filter((c) => c.classList.contains(m[1]));
    const k = /^\.([\w-]+)$/.exec(sel);
    if (k) { const out = []; (function walk(n) { n.children.forEach((c) => { if (c.classList.contains(k[1])) out.push(c); walk(c); }); })(this); return out; }
    throw new Error('fake DOM: selector not supported: ' + sel);
  }
  all() { const out = []; (function walk(n) { n.children.forEach((c) => { out.push(c); walk(c); }); })(this); return out; }
}
function click(el) {
  const ev = { type: 'click', target: el, defaultPrevented: false, stopped: false, stoppedNow: false,
               preventDefault() { this.defaultPrevented = true; }, stopPropagation() { this.stopped = true; }, stopImmediatePropagation() { this.stopped = true; this.stoppedNow = true; } };
  const chain = []; for (let n = el; n; n = n.parentNode) chain.push(n);
  for (let i = chain.length - 1; i >= 1 && !ev.stopped; i--) chain[i].listeners.filter((l) => l.type === 'click' && l.capture).forEach((l) => { if (!ev.stoppedNow) l.fn(ev); });
  if (!ev.stopped) for (const l of el.listeners.filter((x) => x.type === 'click')) { if (ev.stoppedNow) break; l.fn(ev); }
  for (let i = 1; i < chain.length && !ev.stopped; i++) chain[i].listeners.filter((l) => l.type === 'click' && !l.capture).forEach((l) => { if (!ev.stoppedNow) l.fn(ev); });
  return ev;
}

// ---------------------------------------------------------------- one world = one page load
const N = { INTEGER: 0x0001, DOUBLE: 0x0002 };
function world(opt) {
  opt = opt || {};
  const reg = {}; curReg = reg;
  const W = { sends: [], kb: [], selects: [], confirms: [], confirmAnswers: [], dbReloads: 0, pageLoads: 0, keepAlives: 0, holds: [], infos: [],
              cur: null, rt: null, dirty: null, winListeners: [], docListeners: {} };
  const docEl = new El('html'); if (opt.release) docEl.setAttribute('data-mode', 'release');
  const head = new El('head'), body = new El('body');
  const doc = {
    readyState: 'loading', documentElement: docEl, head, body,
    getElementById: (id) => { const e = reg[id]; return e && isAttached(e) ? e : null; },
    createElement: (t) => { const e = new El(t); e.reg = reg; return e; },
    querySelector: (sel) => { if (sel === '.wbGrid td.dirty') return W.dirty; throw new Error('fake document: selector not supported: ' + sel); },
    addEventListener: (t, f) => { (W.docListeners[t] = W.docListeners[t] || []).push(f); },
  };
  function isAttached(e) { for (let n = e; n; n = n.parentNode) if (n === body || n === head) return true; return false; }
  // PageControl1 as HW.MotorTest.html :70 has it: .pcTabs with 5 tabs (data-t 0..4), .pcBody with 5 panes (data-p 0..4)
  const pc = new El('div'); pc.setAttribute('class', 'pcWrap'); pc.setAttribute('id', 'PageControl1'); body.appendChild(pc);
  const tabs = new El('div'); tabs.setAttribute('class', 'tabs pcTabs'); pc.appendChild(tabs);
  const pcBody = new El('div'); pcBody.setAttribute('class', 'pcBody'); pc.appendChild(pcBody);
  const caps = ['Motor Test', 'Preasure', 'Light Scale', 'Light Scale Data', 'Motor Database'];
  for (let i = 0; i < 5; i++) {
    const t = new El('div'); t.setAttribute('class', i === 0 ? 'tab act' : 'tab'); t.setAttribute('data-t', String(i)); t.textContent = caps[i]; tabs.appendChild(t);
    const p = new El('div'); p.setAttribute('class', 'pcPane'); p.setAttribute('data-p', String(i)); p.style.display = i === 0 ? 'block' : 'none'; pcBody.appendChild(p);
  }
  // the page's binder (HW.MotorTest.html :89-97): binds the tabs that exist when it runs
  tabs.querySelectorAll(':scope > .tab').forEach((tb) => tb.addEventListener('click', () => {
    tabs.querySelectorAll(':scope > .tab').forEach((x) => x.classList.remove('act'));
    tb.classList.add('act');
    pcBody.querySelectorAll(':scope > .pcPane').forEach((p) => { p.style.display = (p.dataset.p === tb.dataset.t) ? 'block' : 'none'; });
  }));
  W.tabs = tabs; W.pcBody = pcBody; W.doc = doc; W.reg = reg;
  // the engine (ht9045_wire_engine.js) attaches its keypads on DOMContentLoaded, before this script's boot
  W.engineSaw = null;
  doc.addEventListener('DOMContentLoaded', () => { W.engineSaw = Object.keys(reg).filter((k) => k.indexOf('mtGear') === 0 && doc.getElementById(k)); });
  let seq = 0;
  const sb = {
    console: { log() {}, info() {}, warn() {}, error(...a) { console.error(...a); } },
    setTimeout: (f, ms) => addTimer(f, ms, false), clearTimeout: clearTimer, setInterval: (f, ms) => addTimer(f, ms, true), clearInterval: clearTimer,
    Date: { now: () => clock }, Promise, JSON, Math, Object, Array, String, Number, Error, RegExp, isNaN, isFinite, parseInt, parseFloat,
    document: doc, location: { protocol: opt.file ? 'file:' : 'http:' },
    confirm: (msg) => { W.confirms.push(msg); return W.confirmAnswers.length ? W.confirmAnswers.shift() : true; },
    addEventListener: (t, f) => { if (t === 'message') W.winListeners.push(f); },
    HTQwerty: { N, show: (el, flags, o) => { W.kb.push({ el, flags, o }); } },
    HTMotorAccess: { send: (button, params, motors) => {
      if (W.busy) return null;
      const req = { seq: ++seq, button, action: { mtGearMove: 'gearCalMove', mtGearPreview: 'gearRatioPreview', mtGearSave: 'gearRatioSave', btnStop: 'stop' }[button], params, motors };
      W.sends.push(req); return req;
    } },
    HT9045Recipe: { setTokenHold: (fn) => W.holds.push(fn), keepAlive: () => { W.keepAlives++; return Promise.resolve(); } },
    HT9045Page: { load: () => { W.pageLoads++; return Promise.resolve(); } },
  };
  sb.window = sb;
  sb.parent = { postMessage() {} };                         // in the HMI the page is an iframe of background.html
  if (!opt.noApi) {
    sb.HTMtPageApi = {
      selectMotor: (mi) => { W.selects.push(mi); const a = AXES.find((x) => x.mi === mi); if (!a) return false; W.cur = a.motor; return true; },
      curMotorId: () => W.cur, runtime: () => W.rt, setInfo: (m, c) => W.infos.push([m, c]), dbReload: () => { W.dbReloads++; },
      source: () => opt.source || 'C++',
    };
  }
  W.sb = sb;
  vm.createContext(sb);
  vm.runInContext(CODE, sb, { filename: 'ht9045_mt_gearratio.js' });
  W.beforeDcl = !!doc.getElementById('mtGearPane');
  doc.readyState = 'interactive';
  (W.docListeners.DOMContentLoaded || []).forEach((f) => f());
  advance(0);
  W.G = sb.HTMtGear;
  W.S = () => sb.HTMtGear.state();
  W.$ = (id) => doc.getElementById(id);
  W.status = () => (W.$('mtGearStatus') || { textContent: '' }).textContent;
  W.grey = (id) => { const e = W.$(id); return !!(e && e.classList.contains('teach-unwired') && e.getAttribute('data-unwired')); };
  W.lastSend = () => W.sends[W.sends.length - 1];
  return W;
}

// ---------------------------------------------------------------- the C++ side (runtime + acks), the way WebMotorAccess.cpp shapes them
const AXES = [
  { motor: 'MInArmX', mi: 13, eligible: true, kind: 'linear', why: '', gearRatio: 1, softP: 30000, softN: -30000, placeholder: false, homeFlag: 1 },
  { motor: 'MInArmY', mi: 14, eligible: true, kind: 'linear', why: '', gearRatio: 0.071425, softP: 999999, softN: -999999, placeholder: true, homeFlag: 1 },
  { motor: 'MInRotate', mi: 20, eligible: false, kind: 'rotate', why: 'MInRotate：旋轉軸（單位是角度不是 mm）—— Gear Ratio 分頁只校直線軸', gearRatio: 1, softP: 999999, softN: -999999, placeholder: true, homeFlag: 1 },
];
const LIMITS = { takeupMaxMm: 5, stepMaxMm: 50, spanMaxMm: 100, speedMaxPct: 20 };
function rtOf(motor, o) {
  const ax = AXES.find((a) => a.motor === motor) || AXES[0];
  const motors = AXES.map((a) => ({ motorId: a.motor, position: { cmdPos: a.motor === motor ? o.cmdPos : 0 }, motion: { busy: a.motor === motor ? !!o.busy : false },
                                  state: { inPos: a.motor === motor ? o.inPos !== false : true }, cur: { gearRatio: a.gearRatio, homeFlag: 1 } }));
  return { motors, gearCal: o.noGear ? undefined : { session: o.session === undefined ? null : o.session, axes: AXES, limits: LIMITS }, ax };
}
// a poll: a NEW runtime object, then one tick period
function poll(W, motor, o) { W.rt = rtOf(motor, o); advance(250); }
function sess(id, extra) { return Object.assign({ id, active: true, motor: 'MInArmX', arrived: false, fresh: true, why: '' }, extra || {}); }

// a scripted C++ for one measurement: ack every gearCalMove, then show "moving" and "arrived"
function cpp(W, motor, id) {
  const st = { pos: 0, id };
  st.ack = (req) => {
    const d = Math.round(req.params.distanceMm * 100);
    st.pos += d;
    W.G.onAck(req, { result: 'moving', target: st.pos, cardTarget: st.pos, arriveCmdPos: st.pos, begin: !!req.params.begin, session: sess(id) }, null);
  };
  st.arrive = () => {
    poll(W, motor, { cmdPos: st.pos - 10, busy: true, inPos: false, session: sess(id) });
    poll(W, motor, { cmdPos: st.pos, busy: false, inPos: true, session: sess(id, { arrived: true }) });
    poll(W, motor, { cmdPos: st.pos, busy: false, inPos: true, session: sess(id, { arrived: true }) });
  };
  return st;
}
function typeReading(W, i, text) {
  click(W.$('mtGearRead' + i));
  const k = W.kb[W.kb.length - 1];
  k.el.textContent = text; k.o.onCommit(text);
  return k;
}

// ================================================================= A. structure, the 6th tab, the binders
console.log('[A] the tab, the pane, the generic binders');
{
  const W = world();
  check(!W.beforeDcl && W.engineSaw && W.engineSaw.length === 0, 'built after DOMContentLoaded (+ one turn): the engine\'s keypad attach saw no mtGear element');
  const tab = W.$('mtGearTab'), pane = W.$('mtGearPane');
  check(tab && tab.getAttribute('data-t') === '5' && tab.textContent === 'Gear Ratio' && tab.getAttribute('title') === 'tsGearRatio : TTabSheet' && pane && pane.getAttribute('data-p') === '5',
        'the 6th tab: data-t 5, caption "Gear Ratio", title tsGearRatio : TTabSheet; its pane data-p 5');
  check(pane.style.display === 'none', 'the pane starts hidden (tab 0 is active)');
  click(tab);
  check(pane.style.display === 'block' && tab.classList.contains('act') && W.pcBody.children.filter((p) => p !== pane).every((p) => p.style.display === 'none'),
        'clicking Gear Ratio shows its pane and hides the other five');
  click(W.tabs.children[0]);
  check(pane.style.display === 'none' && !tab.classList.contains('act') && W.pcBody.children[0].style.display === 'block', 'clicking Motor Test (the page\'s own binder) hides the Gear Ratio pane again');
  const ids = Object.keys(W.reg).filter((k) => W.$(k) && W.$(k).id === k && pane.all().indexOf(W.$(k)) >= 0);
  const forbiddenIds = ['sbUpdate', 'sbtReload', 'btnAddMotor', 'btnDeleteMotor', 'spbSave', 'btSave', 'btnSave', 'btOK', 'btApply'];
  const badIds = ids.filter((k) => k.indexOf('mtGear') !== 0 || forbiddenIds.indexOf(k) >= 0 || /^(labName|edPos1_|edPos2_|cbUsing)/.test(k));
  const badCls = pane.all().filter((e) => e.classList.contains('exitbtn') || e.classList.contains('btnpanel'));
  check(ids.length >= 20 && badIds.length === 0 && badCls.length === 0, 'every id in the pane is mtGear* (' + ids.length + '), none of the page\'s bound names / prefixes, no .exitbtn / .btnpanel');
  const inputs = pane.all().filter((e) => e.tagName === 'INPUT' || e.tagName === 'SELECT' || e.tagName === 'TEXTAREA');
  check(inputs.length === 0, 'no <input>/<select> in the pane (the mm fields are divs with their own keypad; motor-access lockAll cannot strand them)');
  const stop = W.$('mtGearStop');
  check(stop && stop.tagName === 'DIV', 'STOP is a div (lockAll disables button / input / select only -> STOP always works)');
  const W2 = world({ release: true });
  check(W2.$('mtGearTab').getAttribute('title') === null && W2.$('mtGearTab').getAttribute('data-htitle') === 'tsGearRatio : TTabSheet', 'release mode: titles go to data-htitle (theme.js already ran)');
}

// ================================================================= B. greying of the whole tab
console.log('[B] the whole tab greys when there is no C++');
{
  const W = world({ file: true });
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  const all = ['mtGearDirP', 'mtGearDirN', 'mtGearStart', 'mtGearNext', 'mtGearPreview', 'mtGearSave', 'mtGearSpeed', 'mtGearTakeup', 'mtGearF1', 'mtGearF2', 'mtGearF3'];
  check(all.every((id) => W.grey(id)) && /file:\/\//.test(W.$('mtGearPane').getAttribute('data-grey') || ''), 'file:// -> every control greyed (teach-unwired + data-unwired), the reason names file://');
  click(W.$('mtGearStart'));
  check(W.sends.length === 0 && /file:\/\//.test(W.status()), 'file://: clicking Start sends nothing and says why');
  const W2 = world({ noApi: true }); poll(W2, 'MInArmX', { cmdPos: 0 });
  check(all.every((id) => W2.grey(id)) && /HTMtPageApi/.test(W2.$('mtGearPane').getAttribute('data-grey') || ''), 'no HTMtPageApi (old page) -> greyed');
  const W3 = world({ source: 'online' }); poll(W3, 'MInArmX', { cmdPos: 0 });
  check(all.every((id) => W3.grey(id)) && /online/.test(W3.$('mtGearPane').getAttribute('data-grey') || ''), 'the page reads a static snapshot (source online) -> greyed');
  const W4 = world(); poll(W4, 'MInArmX', { cmdPos: 0, noGear: true });
  check(all.every((id) => W4.grey(id)) && /gearCal/.test(W4.$('mtGearPane').getAttribute('data-grey') || ''), 'a C++ runtime without gearCal (old wb_serve) -> greyed');
  poll(W4, 'MInArmX', { cmdPos: 0, session: null });
  check(!W4.$('mtGearPane').getAttribute('data-grey') && W4.grey('mtGearStart') && /先選一軸/.test(W4.$('mtGearStart').getAttribute('data-unwired')),
        'greying is reversible: gearCal appears -> the tab un-greys, Start still asks for an axis');
}

// ================================================================= C. the axis list
console.log('[C] the axis list');
{
  const W = world();
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  const rot = W.$('mtGearAxis_MInRotate'), x = W.$('mtGearAxis_MInArmX');
  check(rot && W.grey('mtGearAxis_MInRotate') && x && !W.grey('mtGearAxis_MInArmX'), 'C++ axes listed; the rotary one greyed with C++\'s reason');
  click(rot);
  check(W.selects.length === 0 && /旋轉軸/.test(W.status()), 'clicking the greyed rotary axis: not selected, the reason shown');
  click(W.$('mtGearAxis_MInArmX'));
  check(W.selects.length === 1 && W.selects[0] === 13 && W.S().motor === 'MInArmX' && W.$('mtGearAxis_MInArmX').classList.contains('mtGearSel'),
        'clicking MInArmX: the page\'s own selectMotor(13) (C++ selectedMotor / locks / getMotors agree), marked selected');
  check(!W.grey('mtGearStart') && W.grey('mtGearNext') && /Start/.test(W.$('mtGearNext').getAttribute('data-unwired')), 'after the pick: Start open, Next waits for Start');
  W.cur = 'MInArmY'; poll(W, 'MInArmX', { cmdPos: 0, session: null });
  check(W.grey('mtGearStart') && /主分頁選的軸是 MInArmY/.test(W.$('mtGearStart').getAttribute('data-unwired')), 'the main tab selected another axis -> Start greyed with why');
}

// ================================================================= D. the keypads
console.log('[D] the keypads');
{
  const W = world();
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  click(W.$('mtGearAxis_MInArmX'));
  for (const id of ['mtGearTakeup', 'mtGearF1', 'mtGearF2', 'mtGearF3']) {
    const n0 = W.kb.length; click(W.$(id));
    check(W.kb.length === n0 + 1 && W.kb[n0].flags === N.DOUBLE && W.kb[n0].el === W.$(id), id + ': HTQwerty.show(el, N.DOUBLE)');
  }
  const n1 = W.kb.length; click(W.$('mtGearSpeed'));
  check(W.kb.length === n1 + 1 && W.kb[n1].flags === N.INTEGER, 'mtGearSpeed (a %): HTQwerty.show(el, N.INTEGER)');
  let k = W.kb[W.kb.length - 1]; k.o.onCommit('12');
  click(W.$('mtGearF1')); k = W.kb[W.kb.length - 1]; k.el.textContent = '60'; k.o.onCommit('60');
  check(W.S().pts.map((p) => p.mag).join(',') === '60,100,200,100,60,0', 'F1 = 60 -> the plan is 60 / 100 / 200 forward, 100 / 60 / 0 back');
  click(W.$('mtGearF3')); k = W.kb[W.kb.length - 1]; k.el.textContent = ''; k.o.onCommit('');
  check(W.$('mtGearF3').textContent === '' && W.S().pts.map((p) => p.mag).join(',') === '60,100,60,0', 'F3 left empty -> 2 forward points (60 / 100), back 60 / 0');
  click(W.$('mtGearF1')); k = W.kb[W.kb.length - 1]; k.el.textContent = 'abc'; k.o.onCommit('abc');
  check(W.$('mtGearF1').textContent === '60' && /不是數字/.test(W.status()), 'a non-number keeps the old value and says so');
}

// ================================================================= E. the whole measurement, arrival from the runtime only
console.log('[E] Start, arrival, readings, Preview, Save');
{
  const W = world();
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  click(W.$('mtGearTab'));
  click(W.$('mtGearAxis_MInArmX'));
  const C = cpp(W, 'MInArmX', 7);
  click(W.$('mtGearStart'));
  let s = W.lastSend();
  check(s && s.button === 'mtGearMove' && s.action === 'gearCalMove' && s.params.distanceMm === 2 && s.params.speedPct === 10 && s.params.begin === true &&
        JSON.stringify(s.motors) === '["MInArmX"]', 'Start: motor.access gearCalMove {distanceMm 2, speedPct 10, begin true} on [MInArmX]');
  check(W.S().moving && W.grey('mtGearStart') && W.grey('mtGearNext') && W.grey('mtGearPreview'), 'while moving: Start / Next / Preview do nothing');
  click(W.$('mtGearStart'));
  check(W.sends.length === 1, 'Start clicked again while moving: nothing sent');
  poll(W, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(7, { arrived: true }) });
  poll(W, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(7, { arrived: true }) });
  check(W.S().moving, 'no ack yet: a runtime that looks arrived is not taken (arrival needs the ack\'s arriveCmdPos)');
  C.ack(s);
  check(W.S().moving && W.S().target === 200 && W.S().sessionId === 7, 'the ack: target 200, session 7, still moving (accepted is not arrived)');
  poll(W, 'MInArmX', { cmdPos: 0, busy: false, inPos: true, session: null });
  check(W.S().moving && W.S().sessionId === 7 && !/中止/.test(W.status()), 'a runtime read fetched before the session existed (session null) right after the ack: not taken as an abort');
  // controls: each condition alone keeps it "moving"
  const bad = [['inPos false', { cmdPos: 200, busy: false, inPos: false, session: sess(7, { arrived: true }) }],
               ['busy', { cmdPos: 200, busy: true, inPos: true, session: sess(7, { arrived: true }) }],
               ['cmdPos 199', { cmdPos: 199, busy: false, inPos: true, session: sess(7, { arrived: true }) }],
               ['C++ session not arrived', { cmdPos: 200, busy: false, inPos: true, session: sess(7, { arrived: false }) }]];
  for (const [what, o] of bad) { poll(W, 'MInArmX', o); poll(W, 'MInArmX', o); poll(W, 'MInArmX', o); check(W.S().moving, 'not arrived while ' + what + ' (3 reads)'); }
  poll(W, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(7, { arrived: true }) });
  check(W.S().moving, 'one good read is not enough');
  poll(W, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(7, { arrived: true }) });
  check(!W.S().moving && /消背隙到位/.test(W.status()), 'two consecutive good reads -> the take-up arrived: "zero the gauge"');
  check(!W.grey('mtGearNext') && W.grey('mtGearDirP') && W.grey('mtGearF1'), 'in the session: Next open; direction and the points locked');
  // the 6 points of the spec's §3.2 example
  const legs = [50, 50, 100, -100, -50, -50], reads = ['49.62', '99.21', '198.43', '99.25', '49.66', '0.04'];
  for (let i = 0; i < 6; i++) {
    if (i > 0) {
      click(W.$('mtGearNext'));
      check(W.sends.length === i + 1, 'Next before point ' + i + '\'s reading: nothing sent (' + (W.$('mtGearNext').getAttribute('data-unwired') || '') + ')');
      typeReading(W, i - 1, reads[i - 1]);
    }
    click(W.$('mtGearNext'));
    s = W.lastSend();
    check(s.params.distanceMm === legs[i] && s.params.begin === false, 'point ' + (i + 1) + ': gearCalMove ' + legs[i] + ' mm (begin false)');
    C.ack(s); C.arrive();
    check(!W.S().moving && W.S().pts[i].state === 'arrived', 'point ' + (i + 1) + ' arrived from the runtime');
  }
  const kr = typeReading(W, 5, reads[5]);
  check(kr.flags === N.DOUBLE, 'the reading keypad is DOUBLE');
  check(W.grey('mtGearNext') && !W.grey('mtGearPreview') && W.grey('mtGearSave'), 'all six read: Next done, Preview open, Save waits for the preview');
  click(W.$('mtGearPreview'));
  s = W.lastSend();
  check(s.action === 'gearRatioPreview' && s.params.oldRatio === 1 && JSON.stringify(s.params.measC) === '[50,100,200,100,50,0]' &&
        JSON.stringify(s.params.measA) === '[49.62,99.21,198.43,99.25,49.66,0.04]' && JSON.stringify(s.params.measDir) === '[1,1,1,-1,-1,-1]',
        'Preview: oldRatio = the runtime cur.gearRatio, measC / measA / measDir as the C++ session recorded them');
  W.G.onAck(s, { fit: { level: 'ok', why: '', k: 0.9921524, newRatioText: '0.9921524', oldRatio: 1, deviationPct: -0.78476, consistencyPct: 0.0302, nForward: 3, nBack: 3, maxSpanMm: 200, backlashMm: 0.04 },
                 plan: { ok: true, why: '', previewKey: '0123456789abcdef', softP: { old: 30000, new: 29765, placeholder: false }, softN: { old: -30000, new: -29765, placeholder: false },
                         teach: [{ where: 'MInArmX setEditInArmLoadX', old: 10000, new: 9922, slots: ['a', 'b'] }], changedCount: 1, zeroSkipped: 0, manual: [] },
                 notes: ['n1'], canSave: true, saveWhy: '', needConfirm: false, sessionWhy: '' }, null);
  check(!W.grey('mtGearSave') && /0\.9921524/.test(W.$('mtGearResult').textContent) && /10000 → 9922/.test(W.$('mtGearResult').textContent),
        'the preview shows k / the new ratio / old -> new of each teach value; Save opens');
  W.confirmAnswers = [false];
  const nSave = W.sends.length;
  click(W.$('mtGearSave'));
  check(W.sends.length === nSave && W.confirms.length === 1, 'Save, first confirmation answered No: nothing sent');
  click(W.$('mtGearSave'));
  s = W.lastSend();
  check(s.action === 'gearRatioSave' && s.params.previewKey === '0123456789abcdef' && s.params.confirmLarge === undefined && JSON.stringify(s.params.measC) === '[50,100,200,100,50,0]',
        'Save (Yes): gearRatioSave with the preview key and the same measurements, no confirmLarge below 2 %');
  W.G.onAck(s, { message: '已存檔', gearRatio: { old: 1, new: 0.9921524, newText: '0.9921524' }, fit: {}, plan: {}, next: '請重新回原點' }, null);
  check(W.dbReloads === 1 && W.pageLoads === 1 && /已存檔/.test(W.status()) && W.S().sessionId === 0, 'after the save: the page\'s dbReload + HT9045Page.load (never reloadMotorData), "re-home" shown');
  check(W.holds.length === 1 && W.holds[0]() === false, 'the token hold is off once the session is used up');
}

// ================================================================= F. the minus direction signs measC and measA; the second confirmation; dirty grid
console.log('[F] minus direction, the 2..10 % confirmation, a dirty Motor Database grid');
{
  const W = world();
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  click(W.$('mtGearAxis_MInArmX'));
  click(W.$('mtGearDirN'));
  const C = cpp(W, 'MInArmX', 9);
  click(W.$('mtGearStart'));
  let s = W.lastSend();
  check(s.params.distanceMm === -2 && s.params.begin === true, 'direction -: the take-up is -2 mm');
  C.ack(s); C.arrive();
  const reads = ['50.1', '100.2', '200.4', '100.3', '50.2', '0.05'];
  for (let i = 0; i < 6; i++) { if (i > 0) typeReading(W, i - 1, reads[i - 1]); click(W.$('mtGearNext')); s = W.lastSend(); C.ack(s); C.arrive(); }
  typeReading(W, 5, reads[5]);
  click(W.$('mtGearPreview'));
  s = W.lastSend();
  check(JSON.stringify(s.params.measC) === '[-50,-100,-200,-100,-50,0]' && JSON.stringify(s.params.measA) === '[-50.1,-100.2,-200.4,-100.3,-50.2,-0.05]' &&
        JSON.stringify(s.params.measDir) === '[1,1,1,-1,-1,-1]', 'direction -: measC and measA signed with the direction (C++ cNom), measDir still +1 forward / -1 back');
  W.G.onAck(s, { fit: { level: 'confirm', why: '差 4 %：先問 EastSun', k: 1.04, newRatioText: '1.04', oldRatio: 1, deviationPct: 4 }, plan: { ok: true, previewKey: 'k2', changedCount: 0 }, canSave: true, needConfirm: true }, null);
  W.confirmAnswers = [true, false];
  click(W.$('mtGearSave'));
  check(W.lastSend().action === 'gearRatioPreview' && W.confirms.length === 2 && /先問 EastSun/.test(W.confirms[1]), '2..10 %: a second confirmation; No -> nothing sent');
  W.confirmAnswers = [true, true];
  click(W.$('mtGearSave'));
  s = W.lastSend();
  check(s.action === 'gearRatioSave' && s.params.confirmLarge === true && s.params.previewKey === 'k2', 'both Yes -> gearRatioSave with confirmLarge true');
  W.dirty = new El('td');
  W.G.onAck(s, { message: 'ok', gearRatio: { newText: '1.04' } }, null);
  check(W.dbReloads === 1 && W.pageLoads === 0 && /還沒存的格子/.test(W.status()), 'a dirty Motor Database cell: dbReload yes, HT9045Page.load skipped (the unsaved edit kept), said so');
}

// ================================================================= G. STOP, refusals, timeout, session end, placeholder plan, other acks, window close
console.log('[G] STOP, a refused move, the timeout, the session ending, the placeholder plan, other acks, the window');
{
  const W = world();
  poll(W, 'MInArmX', { cmdPos: 0, session: null });
  click(W.$('mtGearAxis_MInArmX'));
  const C = cpp(W, 'MInArmX', 11);
  click(W.$('mtGearStart')); C.ack(W.lastSend());
  click(W.$('mtGearStop'));
  const s = W.lastSend();
  check(s.button === 'btnStop' && JSON.stringify(s.motors) === '["MInArmX"]' && !W.S().moving && W.S().sessionId === 0 && /STOP/.test(W.status()),
        'STOP (a div): the page\'s btnStop row on [MInArmX]; the page drops the session');
  poll(W, 'MInArmX', { cmdPos: 100, session: sess(11, { active: false, why: 'STOP' }) });
  check(W.grey('mtGearNext') && /Start/.test(W.$('mtGearNext').getAttribute('data-unwired')), 'after STOP: Next asks for a new Start');
  // a refused move
  click(W.$('mtGearStart')); C.ack(W.lastSend()); C.arrive();
  click(W.$('mtGearNext'));
  W.G.onAck(W.lastSend(), null, new Error('gearCalMove MInArmX: 驅動器 ALM'));
  check(!W.S().moving && W.S().next === 0 && W.S().pts[0].state === 'todo' && /拒絕移動.*ALM/.test(W.status()), 'a refused move: back to the same point, the C++ reason shown');
  // timeout: 60 s with no progress
  click(W.$('mtGearNext')); C.ack(W.lastSend());
  for (let i = 0; i < 230; i++) poll(W, 'MInArmX', { cmdPos: 200, busy: true, inPos: false, session: sess(11) });
  check(W.S().moving, '57.5 s without progress: still waiting');
  for (let i = 0; i < 20; i++) poll(W, 'MInArmX', { cmdPos: 200, busy: true, inPos: false, session: sess(11) });
  check(!W.S().moving && /到位逾時/.test(W.status()) && /INP/.test(W.status()), '> 60 s without progress -> timeout, the runtime values and the INP hint shown');
  // progress resets the timeout
  const W2 = world();
  poll(W2, 'MInArmX', { cmdPos: 0, session: null });
  click(W2.$('mtGearAxis_MInArmX'));
  const C2 = cpp(W2, 'MInArmX', 12);
  click(W2.$('mtGearStart')); C2.ack(W2.lastSend());
  for (let i = 0; i < 400; i++) poll(W2, 'MInArmX', { cmdPos: i, busy: true, inPos: false, session: sess(12) });
  check(W2.S().moving, '100 s of a slow move that keeps progressing: no timeout');
  // the C++ session ends during a move (an alarm, STOP from another page ...): two reads -> aborted with C++'s reason
  const W5 = world();
  poll(W5, 'MInArmX', { cmdPos: 0, session: null });
  click(W5.$('mtGearAxis_MInArmX'));
  const C5 = cpp(W5, 'MInArmX', 14);
  click(W5.$('mtGearStart')); C5.ack(W5.lastSend());
  poll(W5, 'MInArmX', { cmdPos: 100, busy: true, inPos: false, session: sess(14, { active: false, why: '馬達警報' }) });
  poll(W5, 'MInArmX', { cmdPos: 100, busy: false, inPos: false, session: sess(14, { active: false, why: '馬達警報' }) });
  check(!W5.S().moving && /量測被中止：馬達警報/.test(W5.status()), 'the C++ session ended during the move -> aborted with C++\'s reason');
  // the session ends while idle
  poll(W2, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(12, { arrived: true }) });
  poll(W2, 'MInArmX', { cmdPos: 200, busy: false, inPos: true, session: sess(12, { arrived: true }) });
  check(!W2.S().moving && W2.S().sessionId === 12, 'arrived');
  poll(W2, 'MInArmX', { cmdPos: 200, session: sess(12, { active: false, why: 'Motor Test 換軸' }) });
  check(W2.S().sessionId === 0 && /量測已結束：Motor Test 換軸/.test(W2.status()), 'the C++ session ended while idle (another axis, alarm, operator gone ...): said, Start needed');
  // the placeholder axis: 50 / 100 only
  const W3 = world();
  poll(W3, 'MInArmY', { cmdPos: 0, session: null });
  click(W3.$('mtGearAxis_MInArmY'));
  check(W3.$('mtGearF3').textContent === '' && W3.S().pts.map((p) => p.mag).join(',') === '50,100,50,0' && !W3.grey('mtGearStart'),
        'a ±999999 axis: point 3 emptied -> 50 / 100 (<= 100 mm from the zero, <= 50 mm a leg)');
  click(W3.$('mtGearF3')); let k = W3.kb[W3.kb.length - 1]; k.el.textContent = '200'; k.o.onCommit('200');
  check(W3.grey('mtGearStart') && /佔位值/.test(W3.$('mtGearStart').getAttribute('data-unwired')), 'a ±999999 axis with a 200 mm point: Start greyed (the C++ caps), says why');
  click(W3.$('mtGearStart'));
  check(W3.sends.length === 0, 'and clicking it sends nothing');
  // other actions are the page's
  check(W.G.onAck({ action: 'saveMotTable' }, {}, null) === false && W.G.onAck({ action: 'home' }, null, new Error('x')) === false, 'onAck: other actions -> false (the page handles them)');
  // the window closes (HT_WIN): the session is dropped
  const W4 = world();
  poll(W4, 'MInArmX', { cmdPos: 0, session: null });
  click(W4.$('mtGearAxis_MInArmX'));
  const C4 = cpp(W4, 'MInArmX', 13);
  click(W4.$('mtGearStart')); C4.ack(W4.lastSend());
  W4.winListeners.forEach((f) => f({ data: { type: 'HT_WIN', open: false }, source: {} }));           // not from the parent: ignored
  check(W4.S().moving && W4.S().sessionId === 13, 'an HT_WIN that does not come from the parent frame is ignored');
  W4.winListeners.forEach((f) => f({ data: { type: 'HT_WIN', open: false }, source: W4.sb.parent }));
  check(!W4.S().moving && W4.S().sessionId === 0 && W4.S().winClosed, 'HT_WIN closed: moving / session dropped (the page\'s formClose ends the C++ one)');
  check(W4.holds.length === 1 && W4.holds[0]() === false, 'closed window: the token hold lets go');
}

console.log(pass + ' passed, ' + fail + ' failed');
process.exit(fail ? 1 : 0);
