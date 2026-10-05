'use strict';
// =============================================================================
//  tools/webprobe/teach_kb_golden_selftest.cjs -- ctest TeachKbGolden.  AI(W906-KB-GOLDEN) 20261004 (KB-GOLDEN 2/2, the web side
//  of the Teach follow-ups to d1e77b4f "KB-GOLDEN 1/2").
//
//  Offline: a minimal fake DOM, the REAL web/page/qwerty.js, the REAL attachKeyboards() cut out of web/page/ht9045_wire_engine.js,
//  the REAL kb table of web/page/ht9045_wire_hwteach.js, the REAL web/JSON/teach-access.json and the REAL functions cut out of
//  web/page/HW.teach.html (teachFieldValue, teachBindTechButtons, teachOn, teachBindMotionButtons, teachRuntimeById, teachNum,
//  teachSyncHome, teachFillSetToOffset).  Golden = HT9011UC_Code_V3.33.906.0_20260618 (uteach.cpp / uteach.dfm / HTEdit.cpp).
//    1. kb rows: EditSh1Speed / EditSh2Speed INTEGER (dfm:5992 / :5936 OnClick=EditSh1SpeedClick -> uteach.cpp:4716-4719 N_INTEGER, 0),
//       the 8 elTeach ECDouble fields DOUBLE dp 3 (uteach.cpp:3210-3217 -> HTEdit.cpp:72 EditClick, :216-223), the 8 elTeach
//       edtInAlignPitchX* without a dfm OnClick INTEGER (uteach.cpp:3222-3233), edtSpeed INTEGER (the web's ScrollBar stand-in), and
//       null for the 50 edits golden opens no keypad on (no OnClick / OnMouseDown, not in elTeach, or ReadOnly); no range anywhere
//       new (user ruling #51 = A); every bound input of HW.teach.html has a row except the two W906-only CCD Y fields.
//    2. attachKeyboards: null -> readonly, no keypad on mousedown, the title says why, listed in HT9045KbAudit.noKeypad; INTEGER /
//       DOUBLE dp 3 open the golden numeric pad (dp 0 / dp 3 step captions, '.' only for DOUBLE); a missing row = generic QWERTY.
//    3. D6, typed only when changed (golden TControl.SetText: no OnChange for the same text, controls.pas:3723-3726): a static or
//       cpp value opened and OKed unchanged keeps its data-src (teachFieldValue does not send a static one); a changed value
//       becomes 'typed' and is sent; retyping the same digits is no change.
//    4. edtSetToOffset (golden Timer1Timer :1395 / :1402, btnSetToOffsetClick :2200-2203): filled from the runtime cur.lastHomePos
//       when the HOME job ends with HomeFlag 1; not on a failed home (HomeFlag 2) or while the job runs; SetToOffset with an empty
//       box copies "" as golden :2202 and warns (refusing it is NIGHT_REPORT s0 #103); with a value it is copied into EditPtr (cpp-pos) and sent.
//  argv[2] = web/page.  CONTROL: W906_TEACH_HTML / W906_WIRE_HWTEACH_JS / W906_WIRE_ENGINE_JS pointing at the HEAD copies (before
//  this change) must go red (W906_QWERTY_JS overrides qwerty.js the same way).
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const htmlFile = process.env.W906_TEACH_HTML || path.join(pageDir, 'HW.teach.html');
const wireFile = process.env.W906_WIRE_HWTEACH_JS || path.join(pageDir, 'ht9045_wire_hwteach.js');
const engFile = process.env.W906_WIRE_ENGINE_JS || path.join(pageDir, 'ht9045_wire_engine.js');
const qwFile = process.env.W906_QWERTY_JS || path.join(pageDir, 'qwerty.js');
const accessFile = path.join(pageDir, '..', 'JSON', 'teach-access.json');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
function scenario(name, fn) { try { fn(); } catch (e) { check(false, name + ' threw: ' + (e && e.stack || e)); } }
const J = x => JSON.stringify(x);

// ---- golden facts (verified 20261004 against uteach.dfm / uteach.cpp of the 906 golden tree) ----------------------------
const ECDOUBLE = ['edtInArmCCDXResolution', 'edtInArmCCDYResolution', 'edtInArmCCDXRadian', 'edtInArmCCDYRadian',
                  'edtOutArmCCDXResolution', 'edtOutArmCCDYResolution', 'edtOutArmCCDXRadian', 'edtOutArmCCDYRadian'];
const ALIGN = ['edtInAlignPitchXAd', 'edtInAlignPitchXAe', 'edtInAlignPitchXAf', 'edtInAlignPitchXAg',
               'edtInAlignPitchXBd', 'edtInAlignPitchXBe', 'edtInAlignPitchXBf', 'edtInAlignPitchXBg'];
const NOKB = ['edtNowPosition', 'edtSetToOffset',                                                     // ReadOnly (dfm:1229 / :1243)
  'seteditContactZ1Relative', 'seteditContactZ2Relative', 'setEditZ2G', 'setEditZ2E', 'setEditZ2C', 'setEditZ2A', 'setEditZ2H',
  'setEditZ2F', 'setEditZ2D', 'setEditZ2B', 'setEditNGBinBoxX', 'setEditNGBinBoxY', 'edtBtBlowY', 'edtBtBlowX', 'SetEditPlaceNGBinBoxZ',
  'SetEditLDRear', 'edtOutArmWaitX', 'edtOutArmWaitY', 'SetEditAuto2Front', 'SetEditAuto2FrontBack', 'SetEditAuto2Rear',
  'SetEditAuto2RearBack', 'SetEditAuto2CassetteZStart', 'edLeftTops65X', 'edLeftTops65Y', 'edRightBottoms65X', 'edRightBottoms65Y',
  'edLeftTopl65X', 'edLeftTopl65Y', 'edRightTopl65X', 'edRightTopl65Y', 'edLeftBottoml65X', 'edRightBottoml65X', 'edLeftBottoml65Y',
  'edRightBottoml65Y', 'edTopBtnCenterX', 'edTopBtnCenterY', 'edTopBtnRotate0', 'setEditLoadPort4Z', 'setEditLoadPort3Z',
  'setEditLoadPort2Z', 'setEditLoadPort1Z', 'setEditLoadPortBufferZ', 'edtEditMagZTray1', 'edtEditMagZStandby', 'setEditCatchMagRear',
  'setEditCatchMagFront', 'EdtTemp'];
const KEEP = { SetEditAuto1Front: 'INTEGER', setEditZ1A: 'INTEGER', SetEditLDRearBack: 'INTEGER', edtInAlignPitchXAa: 'INTEGER',
               edtMoveTo: 'INTEGER' };                                        // golden DOES give these a keypad: must not become null
const DP0 = ['+10', '+100', '+1000', '-10', '-100', '-1000'], DP3 = ['+0.1', '+0.01', '+0.001', '-0.1', '-0.01', '-0.001'];

// ---- fake DOM --------------------------------------------------------------
class El {
  constructor(tag, id) {
    this.tagName = String(tag || 'div').toUpperCase(); this.id = id || ''; this.attrs = {}; this.children = []; this.parentNode = null;
    this.listeners = []; this.className = ''; this.textContent = ''; this.value = ''; this.style = {}; this.disabled = false;
    this.readOnly = false; this.title = ''; this._html = ''; this._stubs = null;
    const self = this;
    this.classList = {
      add(c) { if (!self.hasClass(c)) self.className = (self.className ? self.className + ' ' : '') + c; },
      remove(c) { self.className = self.className.split(/\s+/).filter(x => x && x !== c).join(' '); },
      contains(c) { return self.hasClass(c); },
      toggle(c, on) { if (on === undefined) on = !self.hasClass(c); if (on) this.add(c); else this.remove(c); return on; },
    };
  }
  get innerHTML() { return this._html; }
  set innerHTML(h) { this._html = String(h); this._stubs = null; }
  hasClass(c) { return (' ' + this.className + ' ').indexOf(' ' + c + ' ') >= 0; }
  getAttribute(k) { return Object.prototype.hasOwnProperty.call(this.attrs, k) ? this.attrs[k] : null; }
  setAttribute(k, v) { this.attrs[k] = String(v); if (k === 'readonly') this.readOnly = true; }
  removeAttribute(k) { delete this.attrs[k]; }
  appendChild(c) { if (c.parentNode) c.parentNode.removeChild(c); c.parentNode = this; this.children.push(c); return c; }
  insertBefore(c, ref) {
    if (c.parentNode) c.parentNode.removeChild(c);
    const i = this.children.indexOf(ref); c.parentNode = this;
    if (i < 0) this.children.push(c); else this.children.splice(i, 0, c);
    return c;
  }
  removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; return c; }
  remove() { if (this.parentNode) this.parentNode.removeChild(this); }
  replaceWith(n) { const p = this.parentNode; if (!p) return; if (n.parentNode) n.parentNode.removeChild(n); p.children[p.children.indexOf(this)] = n; n.parentNode = p; this.parentNode = null; }
  addEventListener(t, fn, cap) { this.listeners.push({ t, fn, cap: !!(cap === true || (cap && cap.capture)) }); }
  removeEventListener(t, fn) { this.listeners = this.listeners.filter(l => !(l.t === t && l.fn === fn)); }
  dispatchEvent(ev) {                         // target phase only: capture listeners first, then the others, in registration order
    if (!ev.target) ev.target = this;
    const ls = this.listeners.filter(l => l.t === ev.type && l.cap).concat(this.listeners.filter(l => l.t === ev.type && !l.cap));
    ls.forEach(l => l.fn(ev));
    return !ev.defaultPrevented;
  }
  click() { if (this.disabled) return; this.dispatchEvent({ type: 'click', preventDefault() {}, stopPropagation() {}, stopImmediatePropagation() {} }); }
  all() { const out = []; (function walk(e) { e.children.forEach(k => { out.push(k); walk(k); }); })(this); return out; }
  querySelectorAll(sel) {
    const cls = sel.charAt(0) === '.' ? sel.slice(1) : null;
    return this.all().filter(e => cls ? e.hasClass(cls) : e.tagName === sel.toUpperCase());
  }
  querySelector(sel) {
    const r = this.querySelectorAll(sel);
    if (r.length) return r[0];
    const cls = sel.charAt(0) === '.' ? sel.slice(1) : null;          // made through innerHTML: a stand-in element
    if (!cls) return null;
    const holder = [this].concat(this.all()).find(e => e._html.indexOf('class="' + cls + '"') >= 0);
    if (!holder) return null;
    holder._stubs = holder._stubs || {};
    if (!holder._stubs[cls]) { holder._stubs[cls] = new El('span'); holder._stubs[cls].className = cls; }
    return holder._stubs[cls];
  }
}
class FakeEvent {
  constructor(type, o) { this.type = type; this.bubbles = !!(o && o.bubbles); this.defaultPrevented = false; this.target = null; }
  preventDefault() { this.defaultPrevented = true; }
  stopPropagation() {}
}

function makeWorld() {
  const head = new El('head'), body = new El('body');
  const infos = [], sends = [], greys = [];
  const document = {
    head, body, readyState: 'complete',
    createElement: t => new El(t),
    createEvent: () => new FakeEvent(''),
    getElementById: id => head.all().concat(body.all()).find(e => e.id === id) || null,
    querySelector: sel => body.querySelector(sel),
    querySelectorAll(sel) {
      if (sel === 'input[type="text"], input:not([type])')
        return body.all().filter(e => e.tagName === 'INPUT' && (e.getAttribute('type') === null || e.getAttribute('type') === 'text'));
      if (sel === '[title],[data-htitle]') return [];
      return body.querySelectorAll(sel);
    },
    addEventListener() {},
  };
  const sb = {
    document, Event: FakeEvent, console: { log() {}, warn() {}, info() {}, error() {} },
    setTimeout(fn) { return 1; }, clearTimeout() {}, setInterval() { return 1; }, clearInterval() {},
    addEventListener() {}, location: { protocol: 'file:', search: '' },
    teachSetInfo(m, c) { infos.push([m, c]); },
    teachMarkUnwiredEl(el, why) { greys.push([el && el.id, why]); },
    teachBindGoHints() {}, teachHidePointsWithoutAxes() {},
    teachHTitle(el) { return el.getAttribute('title') || el.getAttribute('data-htitle') || ''; },
    teachSpeedBarSet() {},
    HTMotorAccess: { send(btn, p, m) { sends.push({ btn, p: JSON.parse(JSON.stringify(p || {})) }); return { state: 'requested' }; },
                     release() {}, releaseHeld() {} },
    // page globals the cut functions read / write (declared with var in HW.teach.html)
    teachAccessPoints: null, teachFieldsByBtn: {}, teachMotorRuntime: null, teachHomeMotor: null, teachHomeGone: 0, teachEditPtr: null,
    teachRuntimeLive: true, teachActiveMotorId: null, teachSpeedPending: null,
  };
  sb.window = sb;
  vm.createContext(sb);
  const mk = (tag, id, parent) => { const e = new El(tag, id); (parent || body).appendChild(e); return e; };
  return { sb, document, body, head, infos, sends, greys, mk };
}
function topOv(w) { return w.document.querySelector('.qkOv'); }
function btn(w, label) { const ov = topOv(w); return ov ? ov.all().find(e => e.tagName === 'BUTTON' && e.textContent === label) || null : null; }
function press(w) {
  for (let i = 1; i < arguments.length; i++) {
    const b = btn(w, arguments[i]);
    if (!b) throw new Error('no key ' + J(arguments[i]));
    b.click();
  }
}
function steps(w) {
  const ov = topOv(w), np = ov && ov.querySelector('.qkNp'); if (!np) return [];
  return [3, 8, 13, 4, 9, 14].map(i => np.children[i] ? np.children[i].textContent : '?');
}
function mousedown(el) { const ev = new FakeEvent('mousedown'); el.dispatchEvent(ev); return ev; }
function cutFn(src, name) {                   // a top-level `function NAME(` .. the first `}` at column 0 after it
  const a = src.indexOf('\nfunction ' + name + '(');
  if (a < 0) return '';
  const b = src.indexOf('\n}', a + 1);
  return b < 0 ? '' : src.slice(a + 1, b + 2) + '\n';
}

const html = fs.readFileSync(htmlFile, 'utf8').replace(/\r\n/g, '\n');
const wire = fs.readFileSync(wireFile, 'utf8');
const engine = fs.readFileSync(engFile, 'utf8').replace(/\r\n/g, '\n');
const qwerty = fs.readFileSync(qwFile, 'utf8');
const access = JSON.parse(fs.readFileSync(accessFile, 'utf8'));
const k0 = engine.indexOf('  function attachKeyboards() {'), k1 = k0 >= 0 ? engine.indexOf('\n  }\n', k0) : -1;
const attachCut = (k0 >= 0 && k1 > k0) ? engine.slice(k0, k1 + 4) : '';

// the REAL kb table
let CFG = null;
{
  const s = { HT9045Wire: { register(c) { CFG = c; } }, console: { log() {}, warn() {} } };
  vm.createContext(s);
  vm.runInContext(wire, s, { filename: path.basename(wireFile) });
}
const kb = (CFG && CFG.kb) || {};
function newWorld() {
  const w = makeWorld();
  w.sb.CFG = { page: 'HW.teach.html', kb: kb };
  w.sb.serverRange = function () { return null; };
  vm.runInContext(qwerty, w.sb, { filename: 'qwerty.js' });
  if (attachCut) vm.runInContext(attachCut, w.sb, { filename: 'ht9045_wire_engine.js#attachKeyboards' });
  return w;
}

console.log('-- 1. the kb rows of ht9045_wire_hwteach.js (golden uteach.dfm / uteach.cpp / HTEdit.cpp)');
scenario('1', () => {
  const I0 = J(['INTEGER', 0, false, 0, 0]);
  check(J(kb.EditSh1Speed) === I0 && J(kb.EditSh2Speed) === I0,
        'EditSh1Speed / EditSh2Speed = INTEGER dp 0, no range (golden EditSh1SpeedClick N_INTEGER, 0; range dropped by ruling #51 A)  ' + J([kb.EditSh1Speed, kb.EditSh2Speed]));
  check(ECDOUBLE.every(id => J(kb[id]) === J(['DOUBLE', 3, false, 0, 0])), 'the 8 elTeach ECDouble fields = DOUBLE dp 3, no range (HTEdit.cpp:223)  ' +
        J(ECDOUBLE.filter(id => J(kb[id]) !== J(['DOUBLE', 3, false, 0, 0]))));
  check(ALIGN.every(id => J(kb[id]) === I0), 'edtInAlignPitchX Ad..Ag / Bd..Bg (no dfm OnClick, but elTeach EditClick) = INTEGER dp 0, not null');
  check(J(kb.edtSpeed) === I0, 'edtSpeed (golden ReadOnly; the web ScrollBar stand-in) keeps a keypad, INTEGER  ' + J(kb.edtSpeed));
  const notNull = NOKB.filter(id => !(id in kb) || kb[id] !== null);
  check(NOKB.length === 50 && notNull.length === 0, 'the 50 edits golden opens no keypad on are null  ' + J(notNull));
  check(Object.keys(KEEP).every(id => Array.isArray(kb[id]) && kb[id][0] === KEEP[id]), 'golden keypads stay: ' + Object.keys(KEEP).join(' / ') + ' INTEGER');
  const ranged = Object.keys(kb).filter(id => kb[id] && kb[id][2] === true).sort();
  check(J(ranged) === J(['InSHZDownRange', 'edShtCheckRange', 'setEditInZSafeHeight', 'setEditOutZSafeHeight']),
        'no new range (ruling #51 A): the checkRange rows are the four the generator wrote  ' + J(ranged));
  // every input attachKeyboards binds on the page has a row, except the W906-only CCD Y points (not in golden)
  //   AI(W906-TEACH-INDEXZ-OUTSHT) 20261005 (laptop, batch 69 gate b69a): and the W906-only Index Z1 Out Shuttle Z (setEditIndex1ToOutSht1Z, machine dispatch 1005; not in golden either)
  const ids = [];
  html.replace(/<input\b[^>]*>/g, t => {
    const ty = /\btype="([^"]*)"/.exec(t), id = /\bid="([^"]*)"/.exec(t);
    if (id && (!ty || ty[1] === 'text')) ids.push(id[1]);
    return t;
  });
  const missing = ids.filter(id => !(id in kb));
  check(ids.length >= 584 && J(missing) === J(['setEditIndex1ToOutSht1Z', 'setEditCCDYSite1x1', 'setEditCCDYCal1x1']),
        'HW.teach.html: ' + ids.length + ' bound inputs; without a row only the W906 points (Index Z1 Out Shuttle Z, CCD Y)  ' + J(missing));
});

console.log('-- 2. attachKeyboards (the REAL engine function) on null / INTEGER / DOUBLE / missing rows');
scenario('2', () => {
  if (!attachCut) { check(false, 'attachKeyboards() found in ' + path.basename(engFile)); return; }
  const w = newWorld();
  const z2a = w.mk('input', 'setEditZ2A'), now = w.mk('input', 'edtNowPosition'), sh1 = w.mk('input', 'EditSh1Speed');
  const rad = w.mk('input', 'edtInArmCCDXRadian'), ccd = w.mk('input', 'setEditCCDYSite1x1');
  z2a.value = '10'; now.value = '—'; sh1.value = '50'; rad.value = '1.5'; ccd.value = '7';
  const n = w.sb.attachKeyboards();
  const audit = w.sb.HT9045KbAudit || {};
  check(n === 3 && J(audit.noKeypad) === J(['setEditZ2A', 'edtNowPosition']) && J(audit.unmapped) === J(['setEditCCDYSite1x1']),
        'bound 3; noKeypad = setEditZ2A / edtNowPosition; unmapped = the CCD Y point  ' + J([n, audit.noKeypad, audit.unmapped]));
  check(z2a.getAttribute('readonly') !== null && /無小鍵盤/.test(z2a.title) && z2a.listeners.length === 0,
        'null row: readonly, the title says why, no mousedown listener  [' + z2a.title + ']');
  mousedown(z2a); mousedown(now);
  check(!topOv(w) && z2a.value === '10', 'null row: a tap opens no keypad (golden: no OnClick / ReadOnly)');
  mousedown(sh1);
  check(topOv(w) && J(steps(w)) === J(DP0) && btn(w, '.') === null && btn(w, 'OK') !== null,
        'EditSh1Speed: the numeric pad, INTEGER dp 0 captions, no "." key  ' + J(steps(w)));
  press(w, 'Abort');
  mousedown(rad);
  check(topOv(w) && J(steps(w)) === J(DP3) && btn(w, '.') !== null, 'edtInArmCCDXRadian: DOUBLE dp 3 captions (+0.1 / +0.01 / +0.001), "." key  ' + J(steps(w)));
  press(w, '+0.001', 'OK');
  check(rad.value === '1.501000', 'DOUBLE step: 1.5 +0.001 -> "1.501000" (golden %1.6f; no range, ruling #51 A)  [' + rad.value + ']');
  mousedown(ccd);
  check(topOv(w) && btn(w, '文A') !== null && btn(w, 'Enter') !== null, 'a missing row is still the generic QWERTY (no golden basis)');
  press(w, 'Abort');
});

console.log('-- 3. D6: typed only when the keypad changed the text (golden SetText, controls.pas:3723-3726)');
const fvCut = cutFn(html, 'teachFieldValue'), btCut = cutFn(html, 'teachBindTechButtons');
scenario('3', () => {
  if (!attachCut || !fvCut || !btCut) { check(false, 'cut teachFieldValue / teachBindTechButtons out of HW.teach.html'); return; }
  const w = newWorld();
  vm.runInContext(fvCut + btCut, w.sb, { filename: 'HW.teach.html#teach' });
  const loader = w.mk('input', 'SetEditPickLoader'), rear = w.mk('input', 'SetEditLDRearBack'), auto2 = w.mk('input', 'SetEditAuto2Front');
  loader.value = '-500'; loader.setAttribute('data-src', 'static');            // another machine's snapshot (Teach-config.json)
  rear.value = '100'; rear.setAttribute('data-src', 'cpp');                   // loaded from C++ (C route)
  auto2.value = '300'; auto2.setAttribute('data-src', 'static');
  w.sb.attachKeyboards();                                                     // the engine binds first (attach at load) ...
  w.sb.teachAccessPoints = access;
  w.sb.teachBindTechButtons();                                                // ... the page's typed hook after teach-access.json
  const fv = id => w.sb.teachFieldValue(id);
  check(loader.getAttribute('data-typed-hook') === '1' && rear.getAttribute('data-typed-hook') === '1', 'the typed hook is on the teach-point edits');
  mousedown(loader); press(w, 'OK');
  check(loader.value === '-500' && loader.getAttribute('data-src') === 'static' && fv('SetEditPickLoader') === null,
        "static -500, keypad OK unchanged -> stays 'static', not sent (was 'typed' and sent: D6)  [" + loader.getAttribute('data-src') + ']');
  mousedown(loader); press(w, '7', '2', '8', '0', '0', '-', 'OK');
  check(loader.value === '-72800' && loader.getAttribute('data-src') === 'typed' && fv('SetEditPickLoader') === -72800,
        "then 7 2 8 0 0 -, OK -> '-72800', 'typed', sent as -72800  [" + loader.value + ' ' + loader.getAttribute('data-src') + ']');
  mousedown(loader); press(w, 'OK');
  check(loader.getAttribute('data-src') === 'typed' && fv('SetEditPickLoader') === -72800, "OK again unchanged: stays 'typed'");
  mousedown(rear); press(w, 'OK');
  check(rear.getAttribute('data-src') === 'cpp' && fv('SetEditLDRearBack') === 100, "cpp 100, OK unchanged -> stays 'cpp' (sent as the C++ value)");
  mousedown(rear); press(w, 'Del', '1', '0', '0', 'OK');
  check(rear.value === '100' && rear.getAttribute('data-src') === 'cpp', "cpp 100, Del 1 0 0 OK (the same text) -> no change, stays 'cpp' (VCL: no OnChange)");
  mousedown(rear); press(w, '1', '0', '1', 'Abort');
  check(rear.value === '100' && rear.getAttribute('data-src') === 'cpp', "Abort after typing: unchanged, stays 'cpp'");
  mousedown(auto2);
  check(!topOv(w) && auto2.getAttribute('data-src') === 'static' && fv('SetEditAuto2Front') === null,
        "SetEditAuto2Front (null row, golden no OnClick): no keypad, stays 'static', not sent");
});

console.log('-- 4. edtSetToOffset: filled at the HOME end (Timer1Timer :1395 / :1402), SetToOffset copies an empty box as golden and warns');
const cuts4 = ['teachOn', 'teachNum', 'teachRuntimeById', 'teachSyncHome', 'teachFillSetToOffset', 'teachBindMotionButtons'].map(n => [n, cutFn(html, n)]);
scenario('4', () => {
  const missing = cuts4.filter(c => !c[1]).map(c => c[0]);
  if (missing.length) { check(false, 'cut out of HW.teach.html: ' + missing.join(', ')); return; }
  const w = newWorld();
  vm.runInContext(cuts4.map(c => c[1]).join('\n'), w.sb, { filename: 'HW.teach.html#motion' });
  const off = w.mk('input', 'edtSetToOffset'), home = w.mk('button', 'btnHome'), sto = w.mk('button', 'btnSetToOffset');
  const tgt = w.mk('input', 'SetEditPickLoader');
  tgt.value = '-500'; tgt.setAttribute('data-src', 'static');
  w.sb.attachKeyboards();
  check(off.getAttribute('readonly') !== null && off.listeners.length === 0, 'edtSetToOffset: readonly, no keypad (golden ReadOnly, dfm:1243-1256)');
  w.sb.teachBindMotionButtons({ commands: [] });
  w.sb.teachEditPtr = 'SetEditPickLoader';
  const S = w.sb;
  const run = (homeJob, flag, last) => {
    S.teachMotorRuntime = { motors: [{ motorId: 'MInArmX', motion: { homeJob: homeJob }, cur: { lastHomePos: last, homeFlag: flag }, diag: { homeFlag: flag } }] };
    S.teachSyncHome();
  };
  // SetToOffset before any HOME: golden copies the empty text (btnSetToOffsetClick :2202) -- kept as golden (NIGHT_REPORT s0 #103),
  //   with a warning on the status line; the next Save would store atoi('')=0
  sto.click();
  check(tgt.value === '' && tgt.getAttribute('data-src') === 'cpp-pos' && w.sends.length === 1 && w.sends[0].btn === 'btnSetToOffset' &&
        w.infos.some(x => x[1] === 'warn' && /SetToOffset/.test(x[0])),
        'empty box: SetToOffset copies "" into EditPtr as golden :2202 (cpp-pos), sends, and warns  ' + J(w.infos.slice(-1)));
  tgt.value = '-500'; tgt.setAttribute('data-src', 'static'); w.sends.length = 0;
  // HOME running, then done (HomeFlag 1)
  S.teachHomeMotor = 'MInArmX'; S.teachActiveMotorId = 'MInArmX'; home.classList.add('down');
  run(true, 0, 0); run(true, 0, 0);
  check(off.value === '' && home.hasClass('down'), 'while the HOME job runs: nothing filled, btnHome stays down');
  run(false, 1, -1234);
  const one = off.value;
  run(false, 1, -1234);
  check(one === '' && off.value === '-1234' && !home.hasClass('down'),
        'HOME done (job gone twice, HomeFlag 1): btnHome up and edtSetToOffset = cur.lastHomePos "-1234"  [' + off.value + ']');
  // a failed HOME fills nothing
  off.value = ''; home.classList.add('down'); S.teachHomeGone = 0;
  run(false, 2, -999); run(false, 2, -999);
  check(off.value === '' && !home.hasClass('down'), 'a failed HOME (HomeFlag 2): btnHome up, nothing filled (golden ProcessSingleMotorHome returned false)');
  // the stop path (ack homeActive false lifts the button first): no fill
  off.value = '';
  run(false, 1, -555); run(false, 1, -555);
  check(off.value === '', 'btnHome not down (stopped / closed): teachSyncHome fills nothing');
  // a real fill, then SetToOffset copies it
  home.classList.add('down'); S.teachHomeGone = 0;
  run(false, 1, 4321); run(false, 1, 4321);
  sto.click();
  const last = w.sends[w.sends.length - 1];
  check(off.value === '4321' && tgt.value === '4321' && tgt.getAttribute('data-src') === 'cpp-pos' && last && last.btn === 'btnSetToOffset' && last.p.offset === 4321,
        'filled 4321: SetToOffset copies it into EditPtr (cpp-pos) and sends {offset: 4321} (golden :2202)  ' + J(last));
});

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
