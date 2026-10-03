'use strict';
// =============================================================================
//  tools/webprobe/teach_st02_selftest.cjs -- ctest ST02C9_TeachSt02Page.  AI(W906-ST02-C9-G2) 20261002 (St02-E helper).
//
//  web/page/ht9045_teach_st02.js (St02 card ST02-C9 G2) offline: a minimal fake DOM, the REAL file, and the REAL TEACH_UNWIRED
//  grey code cut out of web/page/HW.teach.html (from `var TEACH_UNWIRED_CSS=` to its load-time call).  Checks:
//   1. the 40 G2 buttons (the 36 on golden's two rotate handlers + Go Latch 1 / 2 + Set All In / Out Z) are bound
//      (data-st02 / data-acc) and a click sends HTMotorAccess.send('St02Teach*', {btn, fields}); the fields are golden atoi
//      of the page fields (backlashIn / backlashOut for all 36, shtSpeed; blank = 0; '30abc' = 30).
//   2. LOAD ORDER PIN: HW.teach.html loads ht9045_teach_st02.js BEFORE the inline TEACH_UNWIRED script, and that script
//      greys once at load (one teachMarkUnwired() call, at DOMContentLoaded or immediately; batch 38's TEACH_UNWIRED_B38 is
//      applied inside it and nowhere else) -- the order this file relies on.
//   3. the buttons that code greys (TEACH_UNWIRED + TEACH_UNWIRED_B38 as HW.teach.html has them today; AI(W906-ST02-C9)
//      20261002: on main 2dd90ef3 TEACH-UNWIRED-2 already lists btnSetAllInArmZ_Move / the rotate buttons, so the test pushes no
//      ids of its own any more): after DOMContentLoaded the grey runs first, then this file's deferred ungrey removes class +
//      attribute from exactly the greyed ones this file handles, and a click still sends (the grey's capture listener does not
//      answer); every id of B38's C9 第二組 group is one this file handles, its 第一組 (G1) / 第三組 (G3) are not.
//   4. the state query at window load: {action:'teachSt02', kind:'control', params:{btn:'St02State', query:true}} straight
//      to HT9045Recipe.motorAccess; shown[] -> display; a refusal -> dashed outline + a retry; HT_WIN open -> asks again;
//      AI(W906-ST02-C9-G2) 20261002: shown.tsRotate hides / shows the tab header (selecting another tab when it was the
//      selected one), the 16 grbIn/OutR* by id; an answer without those keys outlines them and hides nothing.
//   5. AI(W906-ST02-C9) 20261002: G3 dropped (the machine wired BtnPanelLane1-3 / btnArm1Y / Arm2Y / Z1 / Z2Servo on main and
//      greyed the TTL tab) -- this file binds none of them: no data-st02, no St02 listener, a click sends nothing on St02Teach*,
//      and spTTLReset / cbEnableTTLButtonUse keep the page's grey.
//   6. G1 (the pitch family) is not in this file yet: the twelve pitch buttons are not bound and keep the page's grey.
//  `await settle()` after every load / resolve (the KB_MachineSetting lesson, TO_STEVEN 1002 06:1x).
//  argv[2] = web/page.  CONTROL: W906_TEACH_ST02_JS pointing at a copy without the capture listener / ungrey must go red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const jsFile = process.env.W906_TEACH_ST02_JS || path.join(pageDir, 'ht9045_teach_st02.js');
const htmlFile = path.join(pageDir, 'HW.teach.html');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
const settle = () => new Promise(r => setImmediate(r));

// ---- fake DOM --------------------------------------------------------------
class El {
  constructor(id, tag) {
    this.id = id || ''; this.tagName = String(tag || 'div').toUpperCase(); this.attrs = {}; this.style = {}; this.value = '';
    this.disabled = false; this.listeners = []; this.children = []; this.textContent = '';
    const cls = new Set();
    this.classList = { add(c) { cls.add(c); }, remove(c) { cls.delete(c); }, contains(c) { return cls.has(c); }, toggle(c, on) { if (on) cls.add(c); else cls.delete(c); } };
  }
  getAttribute(k) { return Object.prototype.hasOwnProperty.call(this.attrs, k) ? this.attrs[k] : null; }
  setAttribute(k, v) { this.attrs[k] = String(v); }
  removeAttribute(k) { delete this.attrs[k]; }
  addEventListener(t, fn, cap) { this.listeners.push({ t, fn, cap: !!cap }); }
  removeEventListener(t, fn, cap) { this.listeners = this.listeners.filter(l => !(l.t === t && l.fn === fn && l.cap === !!cap)); }
  appendChild(c) { this.children.push(c); return c; }
  querySelector(sel) { const t = String(sel).toUpperCase(); return this.children.find(k => k.tagName === t) || null; }   // AI(W906-ST02-C9-G3): 'input' only
  fire(type) { this.listeners.filter(l => l.t === type).forEach(l => l.fn({ type, target: this, stopImmediatePropagation() {}, stopPropagation() {}, preventDefault() {} })); }
  click() {                                   // target phase: capture listeners first, then the others, in registration order
    if (this.disabled) return;
    let stop = false;
    const ev = { type: 'click', target: this, stopImmediatePropagation() { stop = true; }, stopPropagation() {}, preventDefault() {} };
    const order = this.listeners.filter(l => l.t === 'click' && l.cap).concat(this.listeners.filter(l => l.t === 'click' && !l.cap));
    for (const l of order) { if (stop) break; l.fn(ev); }
  }
}
const ids = {};
function mk(id, tag) { ids[id] = new El(id, tag); return ids[id]; }
// AI(W906-ST02-C9-G2) 20261002: the 36 buttons of golden's two rotate handlers (uteach.dfm), then the other four
const GRB = ['InRA', 'InRB', 'InRC', 'InRD', 'InRE', 'InRF', 'InRG', 'InRH', 'OutRA', 'OutRB', 'OutRC', 'OutRD', 'OutRE', 'OutRF', 'OutRG', 'OutRH'];
const ROT = ['SpeedButtonInRotatePos90', 'SpeedButtonInRotateNeg90', 'SpeedButtonOutRotatepPos90', 'SpeedButtonOutRotatepNeg90'];
GRB.forEach(x => { ROT.push('btnPos90' + x); ROT.push('btnNeg90' + x); });
const G2 = ROT.concat(['btnSht1GoLatch', 'btnSht2GoLatch', 'btnSetAllInArmZ_Move', 'btnSetAllOutArmZ_Move']);
G2.forEach(id => mk(id, 'button'));
['grpRotate_Kit', 'pnlInArmZ', 'pnlOutArmZ'].forEach(id => mk(id));
GRB.forEach(x => mk('grb' + x, 'fieldset'));
// the tsRotate tab header (no id: <div class="tab" data-t="10" title="tsRotate : TTabSheet">) and the tab golden selects instead
const tabRotate = new El('', 'div');
tabRotate.setAttribute('title', 'tsRotate : TTabSheet');
tabRotate.classList.add('tab');
const tabAxle = new El('', 'div');
tabAxle.clicks = 0;
tabAxle.click = function () { this.clicks++; };
tabRotate.parentElement = { querySelector: sel => (/:not\(\[style\*="display: none"\]\)/.test(sel) ? tabAxle : null) };
mk('grpShtSensor').style.display = 'none';                        // the dfm's Visible = False (HW.teach.html has display:none)
mk('edtEditRotateInBacklash', 'input').value = '30abc';
mk('edtEditRotateOutBacklash', 'input').value = '';
mk('EditSh1Speed', 'input').value = ' 55';
mk('EditSh2Speed', 'input').value = '-7';
// AI(W906-ST02-C9) 20261002: the G3 elements (lanes are .btnpanel divs, the TTL box a label holding its input) -- present so that
//   section 5 can show this file leaves them alone (the machine's on main)
const G3 = ['BtnPanelLane1', 'BtnPanelLane2', 'BtnPanelLane3', 'btnArm1YServo', 'btnArm2YServo', 'btnZ1Servo', 'btnZ2Servo', 'spTTLReset', 'cbEnableTTLButtonUse'];
['BtnPanelLane1', 'BtnPanelLane2', 'BtnPanelLane3'].forEach(id => mk(id, 'div'));
['btnArm1YServo', 'btnArm2YServo', 'btnZ1Servo', 'btnZ2Servo', 'spTTLReset'].forEach(id => { mk(id, 'button').textContent = 'Server ON'; });
const ttlInput = new El('', 'input');
mk('cbEnableTTLButtonUse', 'label').appendChild(ttlInput);
// the pitch family's twelve buttons (G1, not in this file yet) -- present so that section 6 can show they are left alone
const PITCH_HOME = ['btn_InPX_13Home', 'btn_InPX_24Home', 'btn_OutPX_13Home', 'btn_OutPX_24Home'];
const G1 = ['btnInClosePitch', 'btnInOpenPitch', 'btnPitchTest', 'btn_InPitchX_13', 'btn_InPitchX_24', 'btn_OutPitch_13', 'btn_OutPitch_24']
  .concat(PITCH_HOME).concat(['btn_PitchX1_3_Loop']);
G1.forEach(id => mk(id, 'button'));
const fetches = [];
const docListeners = [];
const head = new El('head', 'head');
const document = {
  readyState: 'loading', head, body: new El('body', 'body'),
  getElementById: id => ids[id] || null,
  // only the tsRotate tab, and only by a selector that also matches the release theme's data-htitle
  querySelector: sel => (/\.tab\[title\^="tsRotate "\]/.test(sel) && /\.tab\[data-htitle\^="tsRotate "\]/.test(sel)) ? tabRotate : null,
  createElement: t => new El('', t),
  addEventListener(t, fn) { docListeners.push({ t, fn }); },
};
const winListeners = [];
const timers = [];
const infos = [];
const sends = [];
const queries = [];
let answer = null;                                                // what the fake C++ answers the state query with
const sb = {
  document, console: { log() {}, warn() {}, info() {}, error() {} }, location: { search: '' },
  setTimeout(fn, ms) { timers.push({ fn, ms: ms || 0 }); return timers.length; },
  clearTimeout() {},
  addEventListener(t, fn) { winListeners.push({ t, fn }); },
  teachSetInfo(m, c) { infos.push([m, c]); },
  HTMotorAccess: { send(btn, p) { sends.push({ btn, p: JSON.parse(JSON.stringify(p || {})) }); return {}; } },
  HT9045Recipe: { motorAccess(req) { queries.push(JSON.parse(JSON.stringify(req))); return answer ? answer(req) : Promise.reject(new Error('no answer')); } },
  // AI(W906-ST02-C9) 20261002: G3's /api/struct/io reads are gone; any fetch is recorded (section 5: none)
  fetch(url) {
    fetches.push(String(url));
    return Promise.resolve({ ok: false, status: 503, json() { return Promise.resolve({}); } });
  },
};
sb.window = sb;
vm.createContext(sb);
function runTimers(maxMs) {                                       // run (and drop) the queued timers with ms <= maxMs
  for (let i = 0; i < timers.length; ) { if (timers[i].ms <= maxMs) { const t = timers.splice(i, 1)[0]; t.fn(); } else ++i; }
}
function fireDoc(t) { docListeners.filter(l => l.t === t).forEach(l => l.fn({ type: t })); }
function fireWin(t, ev) { winListeners.filter(l => l.t === t).forEach(l => l.fn(ev || { type: t })); }

(async function main() {
  const html = fs.readFileSync(htmlFile, 'utf8');
  // ---- 2. load-order pin (HW.teach.html) ----
  console.log('-- 2. HW.teach.html: this file loads before the inline grey, and the grey runs once at load');
  const tagAt = html.indexOf('<script src="ht9045_teach_st02.js"></script>');
  const greyAt = html.indexOf('var TEACH_UNWIRED_CSS=');
  check(tagAt >= 0, 'HW.teach.html has <script src="ht9045_teach_st02.js"></script>');
  check(tagAt >= 0 && greyAt > tagAt, 'the tag is before the inline TEACH_UNWIRED script (its capture listener is registered first)');
  const motorAt = html.indexOf('<script src="motor-access.js"></script>');
  check(tagAt >= 0 && motorAt >= 0 && html.slice(motorAt, tagAt).indexOf('\n') < 0, 'the tag sits on the motor-access.js line (:81, same-line claim)');
  const calls = (html.match(/teachMarkUnwired\(\)(?!\s*\{)/g) || []).length;
  check(calls === 1, 'teachMarkUnwired() is called once (the load-time call) -- a re-grey on refresh would need the laptop\'s list claim  [' + calls + ']');
  check(/if\(document\.readyState==='loading'\) document\.addEventListener\('DOMContentLoaded', teachMarkUnwired\); else teachMarkUnwired\(\);/.test(html),
        'the grey is applied at DOMContentLoaded (or at once when already loaded)');
  // AI(W906-ST02-C9-G2) 20261002: batch 38's list is greyed inside that one call and nowhere else
  const fnAt = html.indexOf('function teachMarkUnwired(){'), fnEnd = html.indexOf("if(document.readyState==='loading') document.addEventListener('DOMContentLoaded', teachMarkUnwired)");
  const b38 = (html.match(/TEACH_UNWIRED_B38/g) || []).length;
  check(fnAt >= 0 && fnEnd > fnAt && html.slice(fnAt, fnEnd).indexOf('TEACH_UNWIRED_B38.forEach') >= 0 && b38 === 2,
        'TEACH_UNWIRED_B38: defined once, used once -- inside teachMarkUnwired()  [' + b38 + ' mentions]');
  // the grey code itself, cut from the real page
  const g0 = greyAt, g1 = html.indexOf('teachMarkUnwired();', g0);
  const greyCode = (g0 >= 0 && g1 > g0) ? html.slice(g0, html.indexOf('\n', g1)) : '';
  check(greyCode.indexOf('function teachMarkUnwiredEl(') >= 0, 'cut the real TEACH_UNWIRED grey code out of HW.teach.html');

  // ---- load: this file first (:81), then the page's inline grey ----
  vm.runInContext(fs.readFileSync(jsFile, 'utf8'), sb, { filename: path.basename(jsFile) });
  await settle();
  const S = sb.HT9045TeachSt02;
  console.log('-- 1. binding and the request each button sends');
  check(S && S.CAT_BTN === 'St02Teach*' && S.G2.length === 40 && S.ROT.length === 36 && JSON.stringify(S.ROT) === JSON.stringify(ROT),
        'HT9045TeachSt02 exported, catalog button St02Teach*, 40 G2 buttons (the 36 rotate ones = uteach.dfm\'s list)');
  check(G2.every(id => ids[id].getAttribute('data-st02') === 'G2' && ids[id].getAttribute('data-acc') === '1'), 'every G2 button bound (data-st02 / data-acc)');
  check(G2.every(id => ids[id].listeners.length === 1 && ids[id].listeners[0].cap), 'one capture click listener each, registered before the page\'s scripts');
  check(S.G1 === undefined && G1.every(id => ids[id].getAttribute('data-st02') === null && ids[id].listeners.length === 0),
        'the twelve pitch buttons (G1) are not bound by this file yet');
  if (greyCode) vm.runInContext(greyCode, sb, { filename: 'HW.teach.html#TEACH_UNWIRED' });
  await settle();
  check(Array.isArray(sb.TEACH_UNWIRED) && Array.isArray(sb.TEACH_UNWIRED_B38), 'the inline code defined TEACH_UNWIRED and TEACH_UNWIRED_B38');
  // AI(W906-ST02-C9-G2) 20261002: what the page greys at load, from its own two lists
  const greyed = new Set();
  (sb.TEACH_UNWIRED || []).forEach(r => greyed.add(r[0]));
  (sb.TEACH_UNWIRED_B38 || []).forEach(g => String(g[1]).split(' ').forEach(id => greyed.add(id)));
  const handled = S.G2.slice();                                   // AI(W906-ST02-C9) 20261002: G3 dropped (S.G3 is gone); G1 not yet
  const expectUngrey = handled.filter(id => greyed.has(id) && ids[id]).sort();
  check(greyed.has('btnSetAllInArmZ_Move') && greyed.has('SpeedButtonInRotatePos90'),
        'AI(W906-ST02-C9): the page itself greys btnSetAllInArmZ_Move / SpeedButtonInRotatePos90 (TEACH-UNWIRED-2 + B38 on main)');
  const c9 = [], c9other = [];
  (sb.TEACH_UNWIRED_B38 || []).forEach(g => {
    if (/C9 第二組/.test(g[0])) String(g[1]).split(' ').forEach(id => c9.push(id));
    if (/C9 第[一三]組/.test(g[0])) String(g[1]).split(' ').forEach(id => c9other.push(id));
  });
  check(c9.length === 40 && c9.every(id => handled.indexOf(id) >= 0), 'every id of B38\'s C9 第二組 group is one this file handles  [' + c9.length + ' ids' +
        (c9.filter(id => handled.indexOf(id) < 0).length ? '; not handled: ' + c9.filter(id => handled.indexOf(id) < 0).join(',') : '') + ']');
  check(c9other.length === 13 && c9other.every(id => handled.indexOf(id) < 0),
        'AI(W906-ST02-C9): B38\'s C9 第一組 (G1, not yet) / 第三組 (G3, dropped; the machine\'s) are not handled here  [' + c9other.length + ' ids]');
  document.readyState = 'interactive';
  fireDoc('DOMContentLoaded');                                    // this file's listener first (it defers), then the grey
  check(ids.btnSetAllInArmZ_Move.classList.contains('teach-unwired') && /還沒接/.test(ids.btnSetAllInArmZ_Move.getAttribute('data-unwired') || ''),
        'DOMContentLoaded: the grey ran (class + the page\'s own reason on btnSetAllInArmZ_Move)');
  runTimers(0);                                                   // this file's deferred ungrey
  console.log('-- 3. the ungrey after the grey, and clicks still send');
  check(!ids.btnSetAllInArmZ_Move.classList.contains('teach-unwired') && ids.btnSetAllInArmZ_Move.getAttribute('data-unwired') === null &&
        !ids.SpeedButtonInRotatePos90.classList.contains('teach-unwired'), 'the deferred ungrey removed class and attribute from both greyed buttons');
  check(expectUngrey.length === 40 && JSON.stringify(S.state().ungreyed.slice().sort()) === JSON.stringify(expectUngrey),
        'exactly the greyed ones this file handles were touched (TEACH_UNWIRED + TEACH_UNWIRED_B38 as the page has them; no-op on the rest)  [' +
        S.state().ungreyed.length + ' of ' + expectUngrey.length + ']');
  check(handled.every(id => !ids[id] || (!ids[id].classList.contains('teach-unwired') && ids[id].getAttribute('data-unwired') === null)),
        'no button of this file is left greyed (the 36 rotate ones, Go Latch / Set All Z)');
  infos.length = 0; sends.length = 0;
  ids.btnSetAllInArmZ_Move.click();
  check(sends.length === 1 && sends[0].btn === 'St02Teach*' && sends[0].p.btn === 'btnSetAllInArmZ_Move',
        'a click on the once-greyed button sends St02Teach* {btn:btnSetAllInArmZ_Move}');
  check(!infos.some(x => /還沒接|Rotate Kit/.test(x[0])), 'the grey\'s capture listener did not answer the click');
  ids.SpeedButtonInRotatePos90.click();
  check(sends.length === 2 && sends[1].p.btn === 'SpeedButtonInRotatePos90' && sends[1].p.backlashIn === 30 && sends[1].p.backlashOut === 0,
        'Pos90: {btn, backlashIn:30 (atoi "30abc"), backlashOut:0 (blank)}  [' + JSON.stringify(sends[1] && sends[1].p) + ']');
  ids.SpeedButtonInRotateNeg90.click();
  check(sends.length === 3 && sends[2].p.btn === 'SpeedButtonInRotateNeg90' && sends[2].p.backlashIn === 30, 'Neg90 (greyed by the page): same fields');
  ids.btnSht1GoLatch.click(); ids.btnSht2GoLatch.click(); ids.btnSetAllOutArmZ_Move.click();
  check(sends.length === 6 && sends[3].p.shtSpeed === 55 && sends[4].p.shtSpeed === -7 && sends[5].p.btn === 'btnSetAllOutArmZ_Move' &&
        Object.keys(sends[5].p).length === 1, 'Go Latch 1 / 2: shtSpeed = atoi(EditSh1Speed / EditSh2Speed) (55 / -7, C++ refuses the negative); SetAllOutArmZ: only btn');
  ids.btnSht1GoLatch.disabled = true;                             // motor-access.js lockAll while a motion runs
  ids.btnSht1GoLatch.click();
  check(sends.length === 6, 'a disabled button (motion running) sends nothing');
  ids.btnSht1GoLatch.disabled = false;
  // AI(W906-ST02-C9-G2) 20261002: the other 34 rotate buttons -- the same two fields (C++ picks the side by the motor)
  infos.length = 0;
  ids.SpeedButtonOutRotatepPos90.click(); ids.btnNeg90OutRG.click(); ids.btnPos90InRA.click();
  check(sends.length === 9 && sends[6].p.btn === 'SpeedButtonOutRotatepPos90' && sends[7].p.btn === 'btnNeg90OutRG' && sends[8].p.btn === 'btnPos90InRA' &&
        sends.slice(6).every(s => s.btn === 'St02Teach*' && s.p.backlashIn === 30 && s.p.backlashOut === 0 && Object.keys(s.p).length === 3),
        'Out-kit +90 / a tsRotate Out -90 / a tsRotate In +90: {btn, backlashIn, backlashOut} each  [' + JSON.stringify(sends.slice(6).map(s => s.p)) + ']');
  check(!infos.some(x => /還沒接|Rotate Kit/.test(x[0])), 'the page\'s grey listener did not answer them');
  ROT.forEach(id => ids[id].click());
  check(sends.length === 9 + 36 && sends.slice(9).every((s, k) => s.p.btn === ROT[k] && 'backlashIn' in s.p && 'backlashOut' in s.p),
        'all 36 send their own name with both backlash fields');
  sends.length = 6;

  // ---- 4. state query ----
  console.log('-- 4. the state query and golden visibility');
  answer = () => Promise.resolve({ state: 'done', result: 'st02State',
    shown: { grpRotate_Kit: false, pnlInArmZ: true, pnlOutArmZ: false, grpShtSensor: false } });
  fireWin('load');
  await settle(); await settle();
  const q = queries[queries.length - 1] || {};
  check(queries.length === 1 && q.action === 'teachSt02' && q.source === 'uteach' && q.kind === 'control' && q.params &&
        q.params.btn === 'St02State' && q.params.query === true, 'window load: one query {action teachSt02, kind control, params {btn St02State, query:true}}');
  check(ids.grpRotate_Kit.style.display === 'none' && ids.pnlInArmZ.style.display === '' && ids.pnlOutArmZ.style.display === 'none' &&
        ids.grpShtSensor.style.display === 'none', 'shown[] applied (grpRotate_Kit hidden, pnlInArmZ shown, pnlOutArmZ hidden, grpShtSensor stays hidden)');
  check(ids.pnlInArmZ.style.outline === '' && ids.pnlInArmZ.getAttribute('data-st02-rule') === 'shown', 'a known group has no unknown outline');
  answer = () => Promise.reject(new Error('SystemStart==true'));
  timers.length = 0;
  fireWin('message', { data: { type: 'HT_WIN', open: true } });
  await settle(); await settle();
  check(queries.length === 2, 'HT_WIN open asks again');
  check(/dashed/.test(ids.grpRotate_Kit.style.outline || '') && /SystemStart/.test(ids.grpRotate_Kit.getAttribute('data-st02-rule') || ''),
        'a refused query: dashed outline with the reason, the last display kept');
  check(ids.grpRotate_Kit.style.display === 'none', 'the previous answer\'s display is not undone by a refusal');
  check(timers.some(t => t.ms === 5000) && S.state().retrying, 'a retry is scheduled (5 s)');
  answer = () => Promise.resolve({ shown: { grpRotate_Kit: true, pnlInArmZ: false, pnlOutArmZ: false, grpShtSensor: false } });
  runTimers(5000);
  await settle(); await settle();
  check(queries.length === 3 && ids.grpRotate_Kit.style.display === '' && ids.grpRotate_Kit.style.outline === '' && ids.pnlInArmZ.style.display === 'none',
        'the retry answered: grpRotate_Kit shown again, pnlInArmZ hidden, outline cleared');

  // ---- 4b. AI(W906-ST02-C9-G2) 20261002: the tsRotate tab (TabVisible :1668) and its 16 groups (:1690-1703) ----
  console.log('-- 4b. the tsRotate tab and its 16 groups');
  check(/dashed/.test(tabRotate.style.outline || '') && tabRotate.style.display === undefined && /dashed/.test(ids.grbInRA.style.outline || '') &&
        !ids.grbInRA.style.display, 'an answer without the new keys (an older C++): the tab and the 16 groups outlined as unknown, nothing hidden');
  const base = { grpRotate_Kit: false, pnlInArmZ: false, pnlOutArmZ: false, grpShtSensor: false };
  const e4 = {};
  GRB.forEach(x => { e4['grb' + x] = /^(InRA|InRB|InRE|InRF|OutRA|OutRB|OutRE|OutRF)$/.test(x); });   // C++'s answer on e4MotRotate
  tabRotate.classList.add('act');                                 // the user is on the Rotate tab
  answer = () => Promise.resolve({ state: 'done', shown: Object.assign({ tsRotate: false }, base, e4) });
  fireWin('message', { data: { type: 'HT_WIN', open: true } });
  await settle(); await settle();
  check(tabRotate.style.display === 'none' && tabRotate.style.outline === '' && tabRotate.getAttribute('data-st02-rule') === 'hidden' && tabAxle.clicks === 1,
        'tsRotate false: the tab header hidden; it was the selected tab -> another visible tab selected (VCL)');
  tabRotate.classList.remove('act');
  answer = () => Promise.resolve({ state: 'done', shown: Object.assign({ tsRotate: true }, base, e4) });
  fireWin('message', { data: { type: 'HT_WIN', open: true } });
  await settle(); await settle();
  check(tabRotate.style.display === '' && tabAxle.clicks === 1 && ids.grbInRC.style.display === 'none' && ids.grbOutRH.style.display === 'none' &&
        ids.grbInRB.style.display === '' && ids.grbOutRA.style.display === '' && ids.grbInRA.style.outline === '',
        'e4MotRotate: the tab shown; grbInRC / grbOutRH hidden, grbInRB / grbOutRA shown, no outline left');
  check(S.GROUPS.length === 21 && S.GROUPS.indexOf('tsRotate') >= 0, 'GROUPS = the four G2 containers + tsRotate + the 16 grbIn/OutR*');

  // ---- 5. AI(W906-ST02-C9) 20261002: G3 dropped -- this file leaves the machine's buttons alone ----
  console.log('-- 5. G3 is not here: the lanes / servo / TTL elements are not bound by this file');
  check(G3.every(id => ids[id].getAttribute('data-st02') === null && S.state().bound.indexOf(id) < 0) && S.G3 === undefined,
        'no G3 element bound (no data-st02, not in state().bound, no G3 export)');
  check(['BtnPanelLane1', 'BtnPanelLane2', 'BtnPanelLane3', 'btnArm1YServo', 'btnArm2YServo', 'btnZ1Servo', 'btnZ2Servo'].every(id => ids[id].listeners.length === 0),
        'the lanes and the four servo buttons carry no listener from this file (the page wires them: ht9045_io_do.js / ht9045_teach_zallup_c.js)');
  check(/還沒接/.test(ids.spTTLReset.getAttribute('data-unwired') || '') && /還沒接/.test(ids.cbEnableTTLButtonUse.getAttribute('data-unwired') || ''),
        'spTTLReset / cbEnableTTLButtonUse keep the page\'s grey (TEACH-UNWIRED-2 / TEACH-ALLCOMP), not undone here');
  sends.length = 0;
  const q5 = queries.length;
  G3.forEach(id => ids[id].click());
  ttlInput.fire('change');
  for (let k = 0; k < 6; ++k) await settle();
  check(sends.length === 0 && queries.length === q5 && fetches.length === 0, 'clicking them sends nothing on St02Teach*, asks C++ nothing, reads no IO');

  // ---- 6. G1 (the pitch family) is not in this file yet ----
  console.log('-- 6. G1 is not here yet: the twelve pitch buttons keep the page\'s grey');
  check(G1.every(id => /還沒接/.test(ids[id].getAttribute('data-unwired') || '') && ids[id].classList.contains('teach-unwired')),
        'every pitch button still greyed with the page\'s reason (TEACH-UNWIRED-2 / B38)');
  sends.length = 0;
  const q6 = queries.length;
  G1.forEach(id => ids[id].click());
  for (let k = 0; k < 6; ++k) await settle();
  check(sends.length === 0 && queries.length === q6, 'clicking them sends nothing on St02Teach* and asks C++ nothing');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
  process.exit(fail ? 1 : 0);
})().catch(e => { console.log('FAIL exception: ' + (e && e.stack || e)); process.exit(1); });
