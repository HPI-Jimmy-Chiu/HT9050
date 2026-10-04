'use strict';
// =============================================================================
//  tools/webprobe/qwerty_golden_selftest.cjs -- ctest QwertyGoldenKeypad.  AI(W906-KB-GOLDEN) 20261004.
//
//  web/page/qwerty.js against golden TfQwertyKey (HT9011UC_Code_V3.33.906.0_20260618 myQwertyKeyBoard.cpp / .dfm; V912 has the same
//  bytes).  Machine report 20261004: numeric input on the Teach screen was wrong.  The old keypad appended every key to the shown value
//  (-72700, then 7 2 8 0 0 -> "-7270072800"), appended '-' instead of toggling the sign, divided by 100 on '%', and lost the typed
//  value on a stray tap outside.  Offline: a minimal fake DOM, the REAL qwerty.js, and the REAL physicalKeys() cut out of
//  web/page/ht9045_wire_engine.js.  What golden does, and what is checked:
//    1. the API: HTQwerty.show / N; no physical-key API in qwerty.js (the user's 20260915 Rule B keeps physical keys in the engine).
//    2. FormShow :137-143 + TEdit AutoSelect (dfm:84-100 has no AutoSelect line = VCL default True): the text opens selected, and the
//       first key replaces it (spbKeyClick :320-339) -- on the numpad, on the QWERTY keys and on the space bar.
//    3. '-' toggles a leading minus on the whole text (spbMinusClick :421-431); BS drops the last char of the old text (:341-345);
//       Del clears (:352-355).
//    4. '%' only cycles the step captions (spbPercentClick :368-377, ChangeDecimalPoint :379-418; an INTEGER keypad goes 0 -> 1 -> 0).
//    5. the steps add atof(caption) (spbAdd1Click :433-449): INTEGER int() truncation, DOUBLE "%1.6f"; dp sets the captions at
//       every show (:185-187).
//    6. '.' only for DOUBLE / IP_ADDR, once (spbDPClick :451-458).
//    7. OK and Abort run the same post-step (:285-301): CheckRange (MachineType.h:1519-1540, min / max in either order) only for
//       INTEGER|DOUBLE with checkRange, the text = AnsiString(double); Abort restores the original first (:362-366), and writes /
//       calls onCommit only when the result differs; onAbort is always called on Abort, never on OK; onCommit once on OK.
//    8. N_PORT (:249-257): Maximum / Minimum 0..65535 shown (also when the caller gives no min / max: golden's defaults are 0,
//       myQwertyKeyBoard.h:153), no clamp unless INTEGER|DOUBLE; no steps / '%' / '-' (:194-202).
//    9. modal (:283 ShowModal, dfm:4 BorderIcons=[]): no X, a click on the dimmed overlay does nothing.
//   10. physical keys through the REAL engine (Rule B, docs/web-client/REPLICATE.md §2.5): a key = the keypad button with the same
//       label (so golden's on-screen semantics), Escape = Abort, a key that is not on the keypad is refused.
//       AI(W906-KB-GOLDEN) 20261004 (2/2): the engine's PHYS_MAP also names the numeric pad's buttons -- Backspace -> BS, Delete ->
//       Del, Enter -> OK (golden KeyDown :463-466 closes on Enter too) -- Space only where the pad shows a space bar, and a refused key
//       is swallowed while a keypad is open (golden ShowModal: no key reaches the form behind; F1-F12 / Ctrl / Alt / Meta combos /
//       lone modifiers still go to the browser).  10i-10l are new; 10b / 10c / 10e / 10g changed with that.
//   11. a second show while one is open stacks over it (:171-178 fQwertyKey2); a third is ignored with onAbort (:180-181); the same
//       field asked twice (two binders on one input, e.g. Setup.Contact.html) is replaced as before, not stacked.
//       AI(W906-KB-GOLDEN) 20261004 (2/2): an overlay removed by hand is dropped at the next key / OK / Abort / show (settle()) and
//       its onAbort is called once (11b changed, 11d new: A under B, B removed by hand -> A's digit / OK / Abort work at once).
//   12. a dp no case of the switch matches keeps the form's captions (:384; golden calls N_DOUBLE dp 4 cContact.cpp:18557 and dp 6
//       cConfiguration.cpp:5985), design-time captions first (dfm:437-520); INTEGER dp 15 (cSpeed.cpp:2235) -> dp 0 (:381-382);
//       the S98 Tray / HP cell as golden opens it (N_DOUBLE, 2, true, 0, 1000: V912 cConfiguration.cpp:6977 / :7113).
//   13. kept: PASSWORD masking, NO_SYMBOL / NO_SPACE / UPPERCASE keys, the server rangeHook (W906-FW-KBRANGE), the .qkDisp value.
//  argv[2] = web/page.  CONTROL: W906_QWERTY_JS pointing at the qwerty.js before this change must go red
//  (W906_WIRE_ENGINE_JS overrides the engine file the same way).
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const qwFile = process.env.W906_QWERTY_JS || path.join(pageDir, 'qwerty.js');
const engFile = process.env.W906_WIRE_ENGINE_JS || path.join(pageDir, 'ht9045_wire_engine.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
function scenario(name, fn) {
  try { fn(); } catch (e) { check(false, name + ' threw: ' + (e && e.message || e)); }
}

// ---- fake DOM --------------------------------------------------------------
class El {
  constructor(tag) {
    this.tagName = String(tag || 'div').toUpperCase(); this.children = []; this.parentNode = null; this.listeners = [];
    this.className = ''; this.id = ''; this.textContent = ''; this.value = ''; this.style = {}; this.disabled = false;
    this.readOnly = false; this._html = ''; this._stubs = null;
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
  appendChild(c) { if (c.parentNode) c.parentNode.removeChild(c); c.parentNode = this; this.children.push(c); return c; }
  insertBefore(c, ref) {
    if (c.parentNode) c.parentNode.removeChild(c);
    const i = this.children.indexOf(ref); c.parentNode = this;
    if (i < 0) this.children.push(c); else this.children.splice(i, 0, c);
    return c;
  }
  removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; return c; }
  remove() { if (this.parentNode) this.parentNode.removeChild(this); }
  replaceWith(n) {
    const p = this.parentNode; if (!p) return;
    if (n.parentNode) n.parentNode.removeChild(n);
    p.children[p.children.indexOf(this)] = n; n.parentNode = p; this.parentNode = null;
  }
  addEventListener(t, fn, cap) { this.listeners.push({ t, fn, cap: !!cap }); }
  dispatch(t, extra) {                       // target phase only: capture listeners first, then the others
    const ev = Object.assign({ type: t, target: this, preventDefault() {}, stopPropagation() {} }, extra || {});
    this.listeners.filter(l => l.t === t && l.cap).concat(this.listeners.filter(l => l.t === t && !l.cap)).forEach(l => l.fn(ev));
    return ev;
  }
  click() { if (this.disabled) return; this.dispatch('click'); }      // as HTMLElement.click(): nothing on a disabled button
  all() { const out = []; (function walk(e) { e.children.forEach(k => { out.push(k); walk(k); }); })(this); return out; }
  html() { return [this._html].concat(this.all().map(e => e._html)).join(''); }
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

function makeWorld(qwerty, engineCut) {
  const head = new El('head'), body = new El('body'), docListeners = [];
  const document = {
    head, body,
    createElement: t => new El(t),
    getElementById: id => head.all().concat(body.all()).find(e => e.id === id) || null,
    querySelector: sel => body.querySelector(sel),
    querySelectorAll: sel => body.querySelectorAll(sel),
    addEventListener(t, fn, cap) { docListeners.push({ t, fn, cap: !!cap }); },
  };
  const sb = { document, console: { log() {}, warn() {}, info() {}, error() {} } };
  sb.window = sb;
  vm.createContext(sb);
  let loadErr = null;
  if (qwerty !== null) {
    try { vm.runInContext(qwerty, sb, { filename: path.basename(qwFile) }); } catch (e) { loadErr = e; }
  }
  if (engineCut) vm.runInContext(engineCut + '\nphysicalKeys();\n', sb, { filename: 'ht9045_wire_engine.js#physicalKeys' });
  const w = { sb, document, body, head, docListeners, loadErr };
  Object.defineProperty(w, 'Q', { get: () => sb.HTQwerty });
  return w;
}
function fireKey(w, k, mods) {             // a keydown on document, as the browser delivers it to the engine's capture listener
  const ev = Object.assign({ type: 'keydown', key: k, prevented: false,
    preventDefault() { this.prevented = true; }, stopPropagation() {} }, mods || {});
  w.docListeners.filter(l => l.t === 'keydown').forEach(l => l.fn(ev));
  return ev;
}

// ---- helpers ---------------------------------------------------------------
const I = 0x0001, D = 0x0002, NSYM = 0x0004, PW = 0x0008, NSP = 0x0010, PORT = 0x0080, IP = 0x0100;
function open(w, value, flags, opt) {
  const tgt = w.document.createElement('input'); tgt.value = value;
  const log = { commit: [], abort: 0, order: [] };
  const o = Object.assign({}, opt || {}, {
    onCommit(v) { log.commit.push(v); log.order.push('commit'); },
    onAbort() { log.abort++; log.order.push('abort'); },
  });
  w.Q.show(tgt, flags, o);
  return { tgt, log, o };
}
function top(w) { return w.document.querySelector('.qkOv'); }   // first overlay in document order = the keypad on top
function btn(w, label) {
  const ov = top(w); if (!ov) return null;
  return ov.all().find(e => e.tagName === 'BUTTON' && e.textContent === label) || null;
}
function press(w) {                       // press(w, '1', '2', ...): a missing key throws, so the scenario fails loudly
  for (let i = 1; i < arguments.length; i++) {
    const b = btn(w, arguments[i]);
    if (!b) throw new Error('no key ' + JSON.stringify(arguments[i]));
    b.click();
  }
}
function disp(w) { const ov = top(w); const d = ov && ov.querySelector('.qkDisp'); return d ? d.value : null; }
function selOn(w) { const ov = top(w); const d = ov && ov.querySelector('.qkDisp'); return !!(d && d.hasClass('qkSel')); }
function pad(w) { const ov = top(w); return ov ? ov.querySelector('.qkNp') : null; }
function steps(w) {                        // [+a, +b, +c, -a, -b, -c] = spbAdd1, spbAdd10, spbAdd100, spbMinus1, spbMinus10, spbMinus100
  const np = pad(w); if (!np) return [];
  return [3, 8, 13, 4, 9, 14].map(i => np.children[i] ? np.children[i].textContent : '?');
}
function padKeys(w) { const np = pad(w); return np ? np.all().filter(e => e.tagName === 'BUTTON').map(e => e.textContent) : []; }
const J = x => JSON.stringify(x);
const DP0 = ['+10', '+100', '+1000', '-10', '-100', '-1000'], DP1 = ['+1', '+10', '+100', '-1', '-10', '-100'];
const DP2 = ['+1.0', '+0.1', '+0.01', '-1.0', '-0.1', '-0.01'], DP3 = ['+0.1', '+0.01', '+0.001', '-0.1', '-0.01', '-0.001'];
const DESIGN = ['+1', '+10', '+100', '-1', '-10', '-100'];

// ---- the engine's physicalKeys(), cut out of the real file --------------------
const engine = fs.readFileSync(engFile, 'utf8');
const cut0 = engine.indexOf('  var PHYS_BOUND = false;'), cut1 = engine.indexOf('  function attachKeyboards()');
const engineCut = (cut0 >= 0 && cut1 > cut0) ? engine.slice(cut0, cut1) : '';
const qwerty = fs.readFileSync(qwFile, 'utf8');

console.log('-- 1. API');
let w = makeWorld(qwerty);
check(!w.loadErr && w.Q && typeof w.Q.show === 'function' && w.Q.N && w.Q.N.INTEGER === 1 && w.Q.N.DOUBLE === 2 && w.Q.N.PORT === 0x80,
      'qwerty.js loads and exports HTQwerty.show / N' + (w.loadErr ? ' [' + w.loadErr.message + ']' : ''));
check(w.Q && w.Q.key === undefined, 'no physical-key API in qwerty.js: Rule B (REPLICATE.md §2.5, user 20260915) keeps physical keys in the engine');
check(!!engineCut && /Escape:\s*\['Abort'\]/.test(engineCut) && engineCut.indexOf('HTQwerty.key') < 0,
      'the engine physicalKeys() is the Rule B label map (PHYS_MAP Escape -> Abort), not a hand-off to qwerty.js');

console.log('-- 2. the text opens selected; the first key replaces it (FormShow :137-143, spbKeyClick :320-339)');
scenario('2a', () => {
  const k = open(w, '-72700', I, { dp: 0, checkRange: false, min: 0, max: 0 });
  check(disp(w) === '-72700' && selOn(w), 'Teach field -72700 opens shown and selected  [' + disp(w) + ' sel=' + selOn(w) + ']');
  press(w, '1', '2', '3');
  check(disp(w) === '123' && !selOn(w), '1 2 3 -> "123" (was "-72700123")  [' + disp(w) + ']');
  press(w, 'OK');
  check(k.tgt.value === '123' && J(k.log.commit) === '["123"]' && k.log.abort === 0 && !top(w),
        'OK writes "123", onCommit once with "123", no onAbort, keypad closed  [' + k.tgt.value + ' ' + J(k.log) + ']');
});
scenario('2b', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '7', '2', '8', '0', '0', 'OK');
  check(k.tgt.value === '72800', 'the machine report: -72700, 7 2 8 0 0, OK -> "72800" (was "-7270072800")  [' + k.tgt.value + ']');
});
scenario('2c', () => {
  const k = open(w, '500', I, { dp: 0 });
  press(w, '6', '0', '0', 'OK');
  check(k.tgt.value === '600', '500, 6 0 0, OK -> "600" (was "500600")  [' + k.tgt.value + ']');
});
scenario('2d', () => {
  const k = open(w, '', I, { dp: 0 });
  check(!selOn(w), 'an empty field opens with nothing selected');
  press(w, '5', 'OK');
  check(k.tgt.value === '5', 'empty, 5, OK -> "5"  [' + k.tgt.value + ']');
});

console.log('-- 3. minus toggles, BS / Del (spbMinusClick :421-431, spbBackSpaceClick :341-345, spbClearClick :352-355)');
scenario('3a', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '-');
  check(disp(w) === '72700' && !selOn(w), "'-' on -72700 -> \"72700\", selection gone  [" + disp(w) + ']');
  press(w, '-');
  check(disp(w) === '-72700', "'-' again -> \"-72700\"  [" + disp(w) + ']');
  press(w, 'OK');
  check(k.tgt.value === '-72700', 'OK -> "-72700"  [' + k.tgt.value + ']');
});
scenario('3b', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '-', '5', 'OK');
  check(k.tgt.value === '727005', "'-' then 5 on a selected -72700 -> \"727005\" (golden: '-' deselects and keeps the text)  [" + k.tgt.value + ']');
});
scenario('3c', () => {
  const k = open(w, '0', I, { dp: 0 });
  press(w, '1', '5', '0', '0', '-', 'OK');
  check(k.tgt.value === '-1500', "0, 1 5 0 0 -, OK -> \"-1500\" (was \"01500-\")  [" + k.tgt.value + ']');
});
scenario('3d', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, 'Del', '7', '2', '8', '0', '0', '-', 'OK');
  check(k.tgt.value === '-72800', "Del 7 2 8 0 0 -, OK -> \"-72800\" (was \"72800-\")  [" + k.tgt.value + ']');
});
scenario('3e', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, 'BS');
  check(disp(w) === '-7270' && !selOn(w), 'BS on a selected -72700 -> "-7270" (the old text loses its last char)  [' + disp(w) + ']');
  press(w, '5', 'OK');
  check(k.tgt.value === '-72705', 'then 5, OK -> "-72705"  [' + k.tgt.value + ']');
});
scenario('3f', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, 'Del');
  check(disp(w) === '' && !selOn(w), 'Del -> ""  [' + disp(w) + ']');
  press(w, '5', '0', '0', 'OK');
  check(k.tgt.value === '500', 'then 5 0 0, OK -> "500"  [' + k.tgt.value + ']');
});

console.log("-- 4. '%' cycles the step captions only (spbPercentClick :368-377, ChangeDecimalPoint :379-418)");
scenario('4a', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  check(J(steps(w)) === J(DP0), 'INTEGER dp 0 steps +10 +100 +1000 / -10 -100 -1000 (was +1 +10 +100)  ' + J(steps(w)));
  press(w, '%');
  check(J(steps(w)) === J(DP1) && disp(w) === '-72700' && selOn(w), "'%' -> dp 1 captions, value and selection unchanged  " + J(steps(w)) + ' [' + disp(w) + ']');
  press(w, '%');
  check(J(steps(w)) === J(DP0), "'%' again -> back to dp 0 (an INTEGER keypad wraps dp > 1 to 0)  " + J(steps(w)));
  press(w, '7', 'OK');
  check(k.tgt.value === '7', 'the selection survived the two %: 7, OK -> "7"  [' + k.tgt.value + ']');
});
scenario('4b', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '%', 'OK');
  check(k.tgt.value === '-72700', "-72700, %, OK -> \"-72700\" (was \"-727\": divide by 100)  [" + k.tgt.value + ']');
});
scenario('4c', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '%', '%', '+10', 'OK');
  check(k.tgt.value === '-72690', '-72700, % % +10, OK -> "-72690" (was "3")  [' + k.tgt.value + ']');
});
scenario('4d', () => {
  open(w, '1.5', D, { dp: 0 });
  const seen = [J(steps(w))];
  for (let i = 0; i < 4; i++) { press(w, '%'); seen.push(J(steps(w))); }
  check(J(seen) === J([J(DP0), J(DP1), J(DP2), J(DP3), J(DP0)]) && disp(w) === '1.5',
        'DOUBLE: % cycles dp 0 -> 1 -> 2 -> 3 -> 0 (+1.0 / +0.1 / +0.01 at dp 2, +0.1 / +0.01 / +0.001 at dp 3), value unchanged');
  press(w, 'Abort');
});
scenario('4e', () => {
  open(w, '5', I, { dp: 3 });
  check(J(steps(w)) === J(DP0), 'INTEGER opened with dp 3 -> dp 0 captions (:381-382)  ' + J(steps(w)));
  press(w, 'Abort');
});

console.log('-- 5. steps add atof(caption): INTEGER int(), DOUBLE "%1.6f" (spbAdd1Click :433-449)');
scenario('5a', () => {
  const k = open(w, '-72700', I, { dp: 1 });
  press(w, '+100');
  check(disp(w) === '-72600' && !selOn(w), 'dp 1, +100 on -72700 -> "-72600", selection gone  [' + disp(w) + ']');
  press(w, '5', 'OK');
  check(k.tgt.value === '-726005', 'a key after a step appends: 5, OK -> "-726005"  [' + k.tgt.value + ']');
});
scenario('5b', () => {
  const k = open(w, '-72700', I, { dp: 0 });
  press(w, '+10');
  const a = disp(w);
  press(w, '+1000');
  const b = disp(w);
  press(w, '-100', 'OK');
  check(a === '-72690' && b === '-71690' && k.tgt.value === '-71790', 'dp 0: +10 -> -72690, +1000 -> -71690, -100 -> -71790  [' + [a, b, k.tgt.value] + ']');
});
scenario('5c', () => {
  const k1 = open(w, '12.7', I, { dp: 0 }); press(w, '+10', 'OK');
  const k2 = open(w, '-12.7', I, { dp: 0 }); press(w, '+10', 'OK');
  check(k1.tgt.value === '22' && k2.tgt.value === '-2', 'INTEGER int() truncates toward 0: 12.7 +10 -> "22", -12.7 +10 -> "-2" (was 23 / -3)  [' + k1.tgt.value + ' ' + k2.tgt.value + ']');
});
scenario('5d', () => {
  const k1 = open(w, '25.5', D, { dp: 1 }); press(w, '+1', 'OK');
  const k2 = open(w, '1', D, { dp: 3 }); press(w, '+0.001', 'OK');
  const k3 = open(w, 'abc', D, { dp: 2 }); press(w, '+0.1', 'OK');
  check(k1.tgt.value === '26.500000' && k2.tgt.value === '1.001000' && k3.tgt.value === '0.100000',
        'DOUBLE "%1.6f": 25.5 +1 -> 26.500000, 1 +0.001 -> 1.001000, abc +0.1 -> 0.100000  [' + [k1.tgt.value, k2.tgt.value, k3.tgt.value] + ']');
});

console.log("-- 6. '.' (spbDPClick :451-458)");
scenario('6a', () => {
  open(w, '25', I, { dp: 0 });
  check(btn(w, '.') === null && padKeys(w).indexOf('.') < 0, "INTEGER: no '.' key (golden spbDP Visible=false, :200), not even a disabled one");
  press(w, 'Abort');
});
scenario('6b', () => {
  const k = open(w, '25', D, { dp: 2 });
  press(w, '.');
  const a = disp(w), s = selOn(w);
  press(w, '.');
  const b = disp(w);
  press(w, '5', 'OK');
  check(a === '25.' && !s && b === '25.' && k.tgt.value === '25.5', "DOUBLE: selected 25, '.' -> \"25.\" (appends, deselects), '.' again -> no second dot, 5 -> \"25.5\"  [" + [a, b, k.tgt.value] + ']');
});
scenario('6c', () => {
  const k = open(w, '25.5', D, { dp: 2 });
  press(w, '.', '1', 'OK');
  check(k.tgt.value === '25.51', "DOUBLE: selected 25.5, '.' (already has one) still deselects, 1 -> \"25.51\"  [" + k.tgt.value + ']');
});

console.log('-- 7. OK / Abort post-step (:285-301, CheckRange MachineType.h:1519-1540, spbCancelClick :362-366)');
scenario('7a', () => {
  const k1 = open(w, '0', I, { dp: 0, checkRange: true, min: 0, max: 500 }); press(w, 'Del', '8', '0', '0', 'OK');
  const k2 = open(w, '0', I, { dp: 0, checkRange: true, min: 500, max: 0 }); press(w, 'Del', '8', '0', '0', 'OK');
  const k3 = open(w, '0', I, { dp: 0, checkRange: true, min: 500, max: 0 }); press(w, 'Del', '-', '5', 'OK');
  const k4 = open(w, '7', I, { dp: 0, checkRange: true, min: 3, max: 3 }); press(w, 'OK');
  check(k1.tgt.value === '500' && k2.tgt.value === '500' && k3.tgt.value === '0' && k4.tgt.value === '3',
        'OK clamps: 800 in 0..500 -> 500; min / max reversed 800 -> 500, -5 -> 0; min == max -> 3  [' + [k1.tgt.value, k2.tgt.value, k3.tgt.value, k4.tgt.value] + ']');
});
scenario('7b', () => {
  const k1 = open(w, '0050', I, { dp: 0, checkRange: true, min: 0, max: 100 }); press(w, 'OK');
  const k2 = open(w, '0050', I, { dp: 0 }); press(w, 'OK');
  const k3 = open(w, '1.5', D, { dp: 2, checkRange: true, min: 0, max: 1 }); press(w, 'OK');
  const k4 = open(w, '0.30000000000000004', D, { dp: 2, checkRange: true, min: 0, max: 1 }); press(w, 'OK');
  const k5 = open(w, '1', D, { dp: 2, checkRange: true, min: 0, max: 0.00001 }); press(w, 'OK');
  check(k1.tgt.value === '50' && k2.tgt.value === '0050' && k3.tgt.value === '1' && k4.tgt.value === '0.3' && k5.tgt.value === '1E-5',
        'the text = AnsiString(double) only with a range: 0050 -> 50 (no range: 0050), 1.5 -> 1, 0.30000000000000004 -> 0.3, 1 in 0..0.00001 -> 1E-5  [' +
        [k1.tgt.value, k2.tgt.value, k3.tgt.value, k4.tgt.value, k5.tgt.value] + ']');
});
scenario('7c', () => {
  const k1 = open(w, '-72700', I, { dp: 0, checkRange: true, min: -70000, max: -74000 }); press(w, '7', '2', '8', '0', '0', 'OK');
  const k2 = open(w, '-72700', I, { dp: 0, checkRange: true, min: -70000, max: -74000 }); press(w, '7', '2', '8', '0', '0', '-', 'OK');
  check(k1.tgt.value === '-70000' && k2.tgt.value === '-72800',
        "machine TEACH-KB range -70000..-74000: 7 2 8 0 0 OK -> -70000 (as golden), 7 2 8 0 0 - OK -> -72800 (was -74000)  [" + k1.tgt.value + ' ' + k2.tgt.value + ']');
});
scenario('7d', () => {
  const k = open(w, '-80000', I, { dp: 0, checkRange: false, min: 0, max: 0 }); press(w, 'OK');
  check(k.tgt.value === '-80000', 'no range (main wire data, ruling #51 A): -80000 stays  [' + k.tgt.value + ']');
});
scenario('7e', () => {
  const k = open(w, '500', I, { dp: 0 }); press(w, '6', '0', '0', 'Abort');
  check(k.tgt.value === '500' && J(k.log.commit) === '[]' && k.log.abort === 1 && !top(w),
        'Abort after typing: field unchanged, onAbort once, no onCommit  [' + k.tgt.value + ' ' + J(k.log) + ']');
});
scenario('7f', () => {
  const k = open(w, '800', I, { dp: 0, checkRange: true, min: 0, max: 500 }); press(w, '1', 'Abort');
  check(k.tgt.value === '500' && J(k.log.commit) === '["500"]' && k.log.abort === 1 && J(k.log.order) === '["abort","commit"]',
        'Abort on an out-of-range original with a range: restored then clamped -> "500", onAbort then onCommit("500")  [' + k.tgt.value + ' ' + J(k.log) + ']');
});
scenario('7g', () => {
  const k1 = open(w, '800', I, { dp: 0 }); press(w, 'Abort');
  const k2 = open(w, '0050', I, { dp: 0, checkRange: true, min: 0, max: 100 }); press(w, 'Abort');
  check(k1.tgt.value === '800' && J(k1.log.commit) === '[]' && k2.tgt.value === '50' && J(k2.log.commit) === '["50"]',
        'Abort without a range leaves 800 (no onCommit); with a range 0050 becomes 50 (onCommit)  [' + k1.tgt.value + ' ' + k2.tgt.value + ']');
});
scenario('7h', () => {
  const k = open(w, '120', I, { dp: 0 }); press(w, 'OK');
  check(k.tgt.value === '120' && J(k.log.commit) === '["120"]' && k.log.abort === 0, 'OK with no change still calls onCommit once (the engine fires input / change; Temp_Set TS-8 relies on it)');
});
scenario('7i', () => {
  const k = open(w, '50', I, { dp: 0, checkRange: true }); press(w, 'OK');
  check(k.tgt.value === '0', 'checkRange without min / max: golden defaults min = max = 0 (myQwertyKeyBoard.h:153) -> "0"  [' + k.tgt.value + ']');
});

console.log('-- 8. N_PORT / IP_ADDR (:249-257, :194-202)');
scenario('8a', () => {
  const k = open(w, '23', PORT, { dp: 0, checkRange: false, min: 0, max: 0 });
  const lim = top(w).html();
  check(btn(w, '+10') === null && btn(w, '%') === null && btn(w, '-') === null && btn(w, '.') === null,
        "PORT: no steps / '%' / '-' / '.' keys  " + J(padKeys(w)));
  check(lim.indexOf('value="65535"') >= 0 && lim.indexOf('value="0"') >= 0, 'PORT with 0 / 0: Maximum 65535 / Minimun 0 shown');
  press(w, 'Del', '7', '0', '0', '0', '0', 'OK');
  check(k.tgt.value === '70000', 'PORT only: 70000 is not clamped (the clamp needs INTEGER|DOUBLE, :285)  [' + k.tgt.value + ']');
});
scenario('8b', () => {
  const k = open(w, '23', PORT | I, { dp: 0, checkRange: false, min: -1, max: 0 });
  press(w, 'Del', '7', '0', '0', '0', '0', 'OK');
  check(k.tgt.value === '65535', 'PORT|INTEGER with -1 / 0: forced 0..65535 -> "65535"  [' + k.tgt.value + ']');
});
scenario('8c', () => {
  open(w, '10.0.0.1', IP, { dp: 0 });
  check(btn(w, '.') !== null && btn(w, '+10') === null && btn(w, '+1') === null, "IP_ADDR: '.' key, no steps  " + J(padKeys(w)));
  press(w, 'Abort');
});
scenario('8d', () => {
  const k = open(w, '23', PORT, {});
  const lim = top(w).html();
  check(lim.indexOf('value="65535"') >= 0 && lim.indexOf('value="0"') >= 0 && lim.indexOf('NAN') < 0,
        'PORT opened without min / max: golden defaults 0 / 0 -> Maximum 65535 / Minimun 0 (was "NAN")');
  press(w, 'Abort');
  check(k.tgt.value === '23' && k.log.abort === 1 && J(k.log.commit) === '[]', 'Abort: field unchanged  [' + k.tgt.value + ']');
});

console.log('-- 9. modal: no X, the dimmed overlay ignores clicks (:283 ShowModal, dfm:4 BorderIcons=[])');
scenario('9a', () => {
  const k = open(w, '500', I, { dp: 0 });
  const ov = top(w);
  check(ov.html().indexOf('✕') < 0 && ov.html().indexOf('qkx') < 0, 'no X in the title bar');
  press(w, '6', '0', '0');
  ov.dispatch('mousedown', { target: ov }); ov.dispatch('click', { target: ov });
  check(top(w) === ov && disp(w) === '600' && k.log.abort === 0, 'a tap on the dimmed area keeps the keypad open with "600" (was: aborted)');
  press(w, 'OK');
  check(k.tgt.value === '600', 'OK still stores "600"  [' + k.tgt.value + ']');
});
scenario('9b', () => {
  open(w, '5', I, { dp: 0 });
  const pc = btn(w, '%'), okb = btn(w, 'OK');
  check(pc && okb && pc.hasClass('w2') && okb.hasClass('w2'), "'%' and OK are two columns wide (dfm:325-329 / :283-296, Width 104)");
  press(w, 'Abort');
});

console.log('-- 10. physical keys: the REAL engine physicalKeys(), Rule B (REPLICATE.md §2.5) -> the same-label keypad button');
const we = makeWorld(qwerty, engineCut || null);
scenario('10a', () => {
  if (!engineCut) { check(false, 'physicalKeys() found in ' + path.basename(engFile)); return; }
  const k = open(we, '500', I, { dp: 0 });
  const evs = ['6', '0', '0'].map(c => fireKey(we, c));
  check(evs.every(e => e.prevented) && disp(we) === '600', 'INTEGER: 6 0 0 on a selected 500 -> "600" (the digit buttons replace the selection)  [' + disp(we) + ']');
  const esc = fireKey(we, 'Escape');
  check(esc.prevented && !top(we) && k.tgt.value === '500' && k.log.abort === 1 && J(k.log.commit) === '[]',
        'Escape -> Abort: field stays "500", onAbort once, no onCommit  [' + k.tgt.value + ' ' + J(k.log) + ']');
});
scenario('10b', () => {
  if (!engineCut) return;
  open(we, '500', I, { dp: 0 });
  const m = fireKey(we, '-');
  check(m.prevented && disp(we) === '-500', "'-' toggles the sign (spbMinus)  [" + disp(we) + ']');
  const p = fireKey(we, '%');
  check(p.prevented && J(steps(we)) === J(DP1) && disp(we) === '-500', "'%' cycles the captions to dp 1 (spbPercent), value unchanged  " + J(steps(we)));
  const a = fireKey(we, 'a'), dot = fireKey(we, '.'), plus = fireKey(we, '+');
  check(a.prevented && dot.prevented && plus.prevented && disp(we) === '-500' && J(steps(we)) === J(DP1) && top(we),
        "INTEGER: 'a', '.', '+' are not on the pad -> refused: nothing typed, and swallowed (golden ShowModal; was: left to the page)  [" + disp(we) + ']');
  press(we, 'Abort');
});
scenario('10c', () => {
  if (!engineCut) return;
  // AI(W906-KB-GOLDEN) 20261004 (2/2): the numeric pad's buttons are BS / Del / OK -- Backspace / Delete / Enter now reach them
  const k = open(we, '-72700', I, { dp: 0 });
  const bs = fireKey(we, 'Backspace'); const a = disp(we), sa = selOn(we);
  const d1 = fireKey(we, '5'); const b = disp(we);
  const del = fireKey(we, 'Delete'); const c = disp(we);
  fireKey(we, '7'); fireKey(we, '2');
  const ent = fireKey(we, 'Enter');
  check(bs.prevented && a === '-7270' && !sa && d1.prevented && b === '-72705' && del.prevented && c === '' &&
        ent.prevented && !top(we) && k.tgt.value === '72' && J(k.log.commit) === '["72"]' && k.log.abort === 0,
        "numeric pad: Backspace = BS (-72700 -> -7270, as the on-screen BS), 5, Delete = Del (clear), 7 2, Enter = OK -> \"72\" (was: no effect)  " +
        J([a, b, c, k.tgt.value]));
});
scenario('10d', () => {
  if (!engineCut) return;
  const k = open(we, '2', D, { dp: 2 });
  fireKey(we, '.'); const a = disp(we);
  fireKey(we, '5'); fireKey(we, '.'); const b = disp(we);
  press(we, 'OK');
  check(a === '2.' && b === '2.5' && k.tgt.value === '2.5', "DOUBLE: '.' = spbDP (appends to the selected 2), 5, a second '.' adds nothing  [" + J([a, b, k.tgt.value]) + ']');
});
scenario('10e', () => {
  if (!engineCut) return;
  const k = open(we, 'abc', 0, {});
  fireKey(we, 'X'); const a = disp(we);
  fireKey(we, 'y'); const b = disp(we);
  fireKey(we, 'Backspace'); const c = disp(we);
  fireKey(we, ' '); const d = disp(we);
  fireKey(we, 'Delete'); fireKey(we, 'q');
  const bang = fireKey(we, '!');
  const ent = fireKey(we, 'Enter');
  check(a === 'x' && b === 'xy' && c === 'x' && d === 'x ' && bang.prevented && ent.prevented && k.tgt.value === 'q' && !top(we),
        "QWERTY: X -> 'x' replaces abc (case map), y, Backspace -> '⌫', space, Delete, q; '!' is not on the lower-case keys (swallowed, nothing typed); Enter commits  [" + J([a, b, c, d, k.tgt.value]) + ']');
});
scenario('10f', () => {
  if (!engineCut) return;
  const k = open(we, 'ab', NSYM, {});
  const semi = fireKey(we, ';');
  check(semi.prevented && disp(we) === 'ab' && selOn(we), "NO_SYMBOL: ';' hits the disabled key -> nothing typed, selection kept");
  fireKey(we, 'Escape');
  check(!top(we) && k.log.abort === 1 && k.tgt.value === 'ab', 'Escape aborts the text keypad');
});
scenario('10g', () => {
  if (!engineCut) return;
  open(we, '500', I, { dp: 0 });
  const f5 = fireKey(we, 'F5'), tab = fireKey(we, 'Tab'), arrow = fireKey(we, 'ArrowLeft');
  check(!f5.prevented && tab.prevented && arrow.prevented && disp(we) === '500' && selOn(we),
        'F5 (a browser key) is left to the browser; Tab / ArrowLeft are not keypad keys -> swallowed (focus cannot move behind the modal keypad), nothing changes');
  press(we, 'Abort');
  check(!fireKey(we, '5').prevented && !fireKey(we, ' ').prevented && !fireKey(we, 'Tab').prevented, 'no keypad open -> keys untouched');
});
// AI(W906-KB-GOLDEN) 20261004 (2/2): 10i-10l -- Space, browser keys, stacking with a hand-removed top, every row of the table
scenario('10i', () => {
  if (!engineCut) return;
  const k = open(we, '500', I, { dp: 0 });
  const sp = fireKey(we, ' ');
  check(sp.prevented && disp(we) === '500' && selOn(we) && top(we),
        'numeric pad: Space has no button -> swallowed (a focused page button behind the keypad is NOT activated), nothing typed');
  press(we, 'Abort');
  const k2 = open(we, 'abc', 0, {});
  fireKey(we, ' '); fireKey(we, 'Enter');
  const k3 = open(we, 'abc', NSP, {});
  const sp3 = fireKey(we, ' ');
  const d3 = disp(we);
  fireKey(we, 'Escape');
  check(k.log.abort === 1 && k2.tgt.value === ' ' && sp3.prevented && d3 === 'abc' && k3.tgt.value === 'abc' && k3.log.abort === 1,
        'QWERTY: Space = the space bar (replaces the selection); N_NO_SPACE (no space bar): Space swallowed, nothing typed  ' + J([k2.tgt.value, d3]));
});
scenario('10j', () => {
  if (!engineCut) return;
  const k = open(we, 'abc', 0, {});
  const cv = fireKey(we, 'v', { ctrlKey: true }), sh = fireKey(we, 'Shift'), alt = fireKey(we, 'a', { altKey: true });
  check(!cv.prevented && !sh.prevented && !alt.prevented && disp(we) === 'abc' && selOn(we),
        'Ctrl / Alt combos and a lone Shift go to the browser and type nothing (Ctrl+V is not the v key)');
  fireKey(we, 'Escape');
  check(k.log.abort === 1 && k.tgt.value === 'abc', 'Escape -> Abort');
});
scenario('10k', () => {
  if (!engineCut) return;
  const a = open(we, '500', I, { dp: 0 });
  const b = open(we, '7', I, { dp: 0 });
  top(we).remove();                                 // page code removed B's overlay by hand
  fireKey(we, '6'); fireKey(we, '0'); fireKey(we, '0');
  const d = disp(we);
  fireKey(we, 'Enter');
  check(d === '600' && a.tgt.value === '600' && J(a.log.commit) === '["600"]' && b.log.abort === 1 && J(b.log.commit) === '[]' && b.tgt.value === '7' && !top(we),
        'B removed by hand: the physical keys reach A at once (6 0 0 Enter -> "600"), B gets onAbort once, B\'s field untouched  ' + J([d, a.tgt.value, b.log]));
});
scenario('10l', () => {
  if (!engineCut) return;
  // every row of the Rule B table on the pad that has it (REPLICATE.md §2.5)
  const n = open(we, '12', D, { dp: 2 });
  fireKey(we, '%'); const cap = J(steps(we));
  fireKey(we, '-'); fireKey(we, '.'); fireKey(we, 'Backspace'); fireKey(we, '.'); fireKey(we, '5');
  const nd = disp(we);
  fireKey(we, 'Enter');
  const q = open(we, 'ab', 0, {});
  fireKey(we, 'Delete'); fireKey(we, 'Q'); fireKey(we, '1'); fireKey(we, ' '); fireKey(we, 'Backspace');
  const qd = disp(we);
  fireKey(we, 'Escape');
  check(cap === J(DP3) && nd === '-12.5' && n.tgt.value === '-12.5' && qd === 'q1' && q.tgt.value === 'ab' && q.log.abort === 1,
        "numeric DOUBLE: % (dp 2 -> 3 captions), - . Backspace . 5, Enter -> \"-12.5\"; QWERTY: Delete Q 1 Space Backspace -> \"q1\", Escape restores \"ab\"  " + J([cap, nd, qd]));
});
scenario('10h', () => {
  if (!engineCut) return;
  const a = open(we, '500', I, { dp: 0 });
  fireKey(we, '6');
  const b = open(we, 'abc', 0, {});
  fireKey(we, 'x'); fireKey(we, 'Enter');
  check(b.tgt.value === 'x' && top(we) && disp(we) === '6', 'stacked: the physical keys go to the keypad on top (B gets x + Enter), then A is back with "6"  [' + b.tgt.value + ' ' + disp(we) + ']');
  fireKey(we, '0'); fireKey(we, '0');
  press(we, 'OK');
  check(a.tgt.value === '600', 'A then takes 0 0 and OK -> "600"  [' + a.tgt.value + ']');
});

console.log('-- 11. a second keypad stacks (:171-178 fQwertyKey2); a third is ignored (:180-181)');
scenario('11a', () => {
  const a = open(w, '500', I, { dp: 0 });
  press(w, '6');
  const ovA = top(w);
  const b = open(w, 'abc', 0, {});
  const ovB = top(w);
  check(ovB && ovB !== ovA && ovA.parentNode === w.body && w.body.children.indexOf(ovB) < w.body.children.indexOf(ovA) && String(ovB.style.zIndex) === '100000',
        'B opens over A (A stays, B first in document order, z-index 100000)');
  check(disp(w) === 'abc' && selOn(w) && a.log.abort === 0, 'B shows its own text selected; A was not aborted');
  const c = open(w, 'zzz', 0, {});
  check(c.log.abort === 1 && top(w) === ovB, 'a third request is ignored and gets onAbort');
  press(w, 'x', 'Enter');
  check(b.tgt.value === 'x' && J(b.log.commit) === '["x"]' && top(w) === ovA && disp(w) === '6', 'B commits "x"; A is back on top with "6"  [' + disp(w) + ']');
  press(w, '0', '0', 'OK');
  check(a.tgt.value === '600' && J(a.log.commit) === '["600"]' && !top(w), 'A then commits "600"  [' + a.tgt.value + ']');
});
scenario('11c', () => {
  // two binders on one input (Setup.Contact.html: ht9045_contact_wire.js + the engine both bind mousedown) -> one tap = two show()s
  const tgt = w.document.createElement('input'); tgt.value = '500';
  const l1 = { commit: [], abort: 0 }, l2 = { commit: [], abort: 0 };
  w.Q.show(tgt, I, { dp: 0, onCommit(v) { l1.commit.push(v); }, onAbort() { l1.abort++; } });
  w.Q.show(tgt, I, { dp: 0, onCommit(v) { l2.commit.push(v); }, onAbort() { l2.abort++; } });
  const n = w.body.children.filter(e => e.hasClass('qkOv')).length;
  press(w, '6', '0', '0', 'OK');
  check(n === 1 && l1.abort === 1 && J(l1.commit) === '[]' && J(l2.commit) === '["600"]' && tgt.value === '600' && !top(w),
        'the same field asked twice: the later keypad replaces the first (no stacking, no leftover keypad with the old value)  [' + n + ' ' + tgt.value + ']');
});
scenario('11b', () => {
  const a = open(w, '1', I, { dp: 0 });
  top(w).remove();                                  // page code removed the overlay by hand (hsys_table_selftest does this)
  const b = open(w, '2', I, { dp: 0 });
  check(top(w) && w.body.children.length === 1 && disp(w) === '2' && !top(w).style.zIndex, 'an overlay removed by hand counts as closed (the next one is not stacked)');
  press(w, 'OK');
  // AI(W906-KB-GOLDEN) 20261004 (2/2): the dropped keypad now gets onAbort once (was 0: a promise-style caller waited forever)
  check(b.tgt.value === '2' && a.log.abort === 1 && J(a.log.commit) === '[]' && a.tgt.value === '1',
        'and works; the keypad removed by hand got onAbort once, its field untouched  [' + b.tgt.value + ' ' + J(a.log) + ']');
});
scenario('11d', () => {
  // AI(W906-KB-GOLDEN) 20261004 (2/2), adversarial review round 2: stack A + B, page code removes B's overlay by hand -> A's keys,
  //   OK and Abort used to do nothing until the next show() (the S === cur gates); now the first key / OK / Abort drops B (settle()).
  const a = open(w, '500', I, { dp: 0 });
  const b = open(w, 'abc', 0, {});
  const ovB = top(w);
  ovB.remove();
  press(w, '6');                                    // A's on-screen digit
  const d1 = disp(w);
  press(w, '0', '0', 'OK');
  check(d1 === '6' && a.tgt.value === '600' && J(a.log.commit) === '["600"]' && a.log.abort === 0,
        "B removed by hand: A's digit works at once (\"6\"), then 0 0 OK -> \"600\"  " + J([d1, a.tgt.value]));
  check(b.log.abort === 1 && J(b.log.commit) === '[]' && b.tgt.value === 'abc' && !top(w), "B's onAbort is called exactly once, its field untouched, nothing left open  " + J(b.log));
  const a2 = open(w, '500', I, { dp: 0 });
  const b2 = open(w, '9', I, { dp: 0 });
  top(w).remove();
  press(w, 'Abort');                                // A's Abort
  check(!top(w) && a2.log.abort === 1 && b2.log.abort === 1 && a2.tgt.value === '500', "A's Abort works at once too (A and B each get onAbort once)  " + J([a2.log, b2.log]));
  const a3 = open(w, '1', I, { dp: 0 });
  const b3 = open(w, '2', I, { dp: 0 });
  top(w).remove();
  press(w, 'OK');                                   // A's OK with nothing typed
  check(!top(w) && J(a3.log.commit) === '["1"]' && b3.log.abort === 1 && a3.log.abort === 0, "A's OK works at once  " + J([a3.log, b3.log]));
});

console.log("-- 12. dp outside the switch (:384) as golden passes it; the S98 Tray / HP cell as golden opens it");
scenario('12a', () => {
  const wf = makeWorld(qwerty);
  open(wf, '1', D, { dp: 6, checkRange: true, min: 0.95, max: 1.05 });   // golden cConfiguration.cpp:5985 (906) N_DOUBLE, 6
  const a = steps(wf); press(wf, 'Abort');
  open(wf, '1', D, { dp: 2 }); press(wf, 'Abort');
  open(wf, '1', D, { dp: 6, checkRange: true, min: 0.95, max: 1.05 });
  const b = steps(wf);
  press(wf, '%');
  const c = steps(wf); press(wf, 'Abort');
  open(wf, '1', D, { dp: 4 });                                            // golden cContact.cpp:18557 (906) N_DOUBLE, 4
  const d = steps(wf); press(wf, 'Abort');
  check(J(a) === J(DESIGN) && J(b) === J(DP2) && J(c) === J(DP0) && J(d) === J(DP0),
        'DOUBLE dp 6 / dp 4: design-time captions first, then the last ones (dp 2); % -> 7 > 3 -> dp 0  ' + J([a, b, c, d]));
});
scenario('12b', () => {
  const wf = makeWorld(qwerty);
  open(wf, '1', D, { dp: 3 }); press(wf, 'Abort');
  open(wf, '50', I, { dp: 15, checkRange: true, min: 5, max: 200 });     // golden cSpeed.cpp:2235 (906) N_INTEGER, 15
  check(J(steps(wf)) === J(DP0), 'INTEGER dp 15 -> dp 0 captions (bIntegerOnly && dp > 1, :381-382)  ' + J(steps(wf)));
  press(wf, 'Abort');
});
scenario('12c', () => {
  const wf = makeWorld(qwerty);
  open(wf, '3', I, { dp: 0, checkRange: true, min: 0, max: 1000 }); press(wf, 'Abort');          // an INTEGER Tray cell first
  const k = open(wf, '12.50', D, { dp: 2, checkRange: true, min: 0, max: 1000 });               // V912 cConfiguration.cpp:6977 / :7113
  const cap = steps(wf);
  press(wf, '+1.0'); const a = disp(wf);
  press(wf, '+0.01'); const b = disp(wf);
  press(wf, 'OK');
  check(J(cap) === J(DP2) && a === '13.500000' && b === '13.510000' && k.tgt.value === '13.51',
        'Tray / HP DOUBLE cell with golden dp 2 (what the server sends, CfgTrayPlate.cpp:907): +1.0 / +0.1 / +0.01; 12.50 +1.0 +0.01, OK -> 13.51  ' + J([cap, a, b, k.tgt.value]));
});

console.log('-- 13. text, password, kept features');
scenario('13a', () => {
  const k = open(w, 'abc', 0, {});
  check(selOn(w), 'text field opens selected');
  press(w, 'x');
  const a = disp(w);
  press(w, 'y');
  const b = disp(w);
  press(w, '⌫');
  const c = disp(w);
  press(w, 'Delete', 'q', 'Enter');
  check(a === 'x' && b === 'xy' && c === 'x' && k.tgt.value === 'q', 'QWERTY: x replaces "abc", y appends, backspace, Delete, q, Enter -> "q"  [' + J([a, b, c, k.tgt.value]) + ']');
});
scenario('13b', () => {
  const k = open(w, 'abc', 0, {});
  const np = padKeys(w);
  check(J(np.slice().sort()) === J(['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']), 'text mode numpad: only the digits (BS / Del / Abort / OK / steps hidden, :190-202)  ' + J(np));
  const p8 = pad(w) && pad(w).all().find(e => e.tagName === 'BUTTON' && e.textContent === '8');
  if (!p8) throw new Error('no numpad 8');
  p8.click();
  check(disp(w) === '8', 'text mode: a numpad digit replaces the selection too  [' + disp(w) + ']');
  press(w, '7', 'Enter');
  check(k.tgt.value === '87', 'then the QWERTY 7 appends, Enter -> "87"  [' + k.tgt.value + ']');
  open(w, 'abc', 0, {});
  press(w, '文A');
  check(selOn(w) && btn(w, 'X') !== null, 'the case key keeps the selection and switches the keys');
  press(w, 'X', 'Abort');
  check(!top(w), 'closed');
});
scenario('13c', () => {
  const k = open(w, 'abc', 0, {});
  press(w, ' ', 'Enter');
  open(w, 'abc', NSP, {});
  const noSpace = btn(w, ' ') === null;
  press(w, 'Abort');
  check(k.tgt.value === ' ' && noSpace, 'space bar replaces a selection; NO_SPACE has no space bar  [' + J(k.tgt.value) + ']');
});
scenario('13d', () => {
  const k = open(w, '', NSYM | NSP | PW, {});
  check(top(w).querySelector('.qkLim') === null || top(w).html().indexOf('Current Value') < 0, 'PASSWORD: no Current Value');
  press(w, 'a', 'b');
  check(disp(w) === '**', 'PASSWORD masks  [' + disp(w) + ']');
  press(w, 'Enter');
  check(J(k.log.commit) === '["ab"]', 'PASSWORD: onCommit gets the clear text');
  const k2 = open(w, 'secret', PW, {});
  check(disp(w) === '******' && selOn(w), 'a preset password is masked and selected');
  press(w, 'x', 'Enter');
  check(k2.tgt.value === 'x', 'and the first key replaces it  [' + k2.tgt.value + ']');
  open(w, '', NSYM, {});
  const semi = btn(w, ';');
  check(semi && semi.disabled && semi.hasClass('dis'), 'NO_SYMBOL still disables the symbol keys (kept)');
  press(w, 'Abort');
});
scenario('13e', () => {
  const k = open(w, '50', I, { dp: 0, checkRange: false, min: 0, max: 0, rangeHook: () => ({ min: 0, max: 10 }) });
  const src = top(w).html();
  press(w, 'OK');
  const k2 = open(w, '50', I, { dp: 0, checkRange: true, min: 0, max: 100, rangeHook: () => null });
  const src2 = top(w).html();
  press(w, 'OK');
  check(k.tgt.value === '10' && src.indexOf('機台') >= 0 && k2.tgt.value === '50' && src2.indexOf('接線檔') >= 0,
        'server rangeHook still wins (50 -> 10, source = machine); no server range -> the wire range (W906-FW-KBRANGE)  [' + k.tgt.value + ' ' + k2.tgt.value + ']');
});
scenario('13f', () => {
  const k = open(w, '7.25', D, { dp: 2 });
  const d = top(w).querySelector('.qkDisp');
  check(d && d.tagName === 'INPUT' && d.value === '7.25', '.qkDisp is still the input whose value is the shown text (HSys_TablePage reads it)');
  press(w, 'Abort');
  check(k.log.abort === 1, 'closed');
});

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
