'use strict';
// =============================================================================
//  tools/webprobe/qwerty_p3_selftest.cjs -- ctest QWERTY_P3.  AI(W906-TIF-P3) 20261002 (St02-E).
//
//  docs/TESTERIF_FIELD_AUDIT_20261002.md P3: web/page/qwerty.js commit() with a range must leave a DOUBLE value as golden
//  906_0625_Steven myQwertyKeyBoard.cpp:290 does -- edQwertyContent->Text=AnsiString(CheckRange(d, min, max)): the clamped
//  number as is; the decimals argument of ShowQwertyKey only sets the +/- step (:379-418).  qwerty.js:71 used toFixed(dp),
//  which rounded 10.5 to "11" at dp 0 and padded 10 to "10.00" at dp 2.
//  Offline: a minimal fake DOM; the REAL qwerty.js; the keyboard opens on a target whose value is the "typed" text, then
//  its own OK button commits.  Checks: DOUBLE dp 0 keeps 10.5; DOUBLE dp 2 gives "10" for 10; the clamp still applies (both
//  ends, min / max given in either order); INTEGER still rounds; no range -> the text untouched; onCommit gets the value.
//  argv[2] = web/page.  CONTROL: W906_QWERTY pointing at qwerty.js before :71 changed must turn the DOUBLE checks red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const file = process.env.W906_QWERTY || path.join(pageDir, 'qwerty.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }

class El {
  constructor(tag) {
    this.tagName = String(tag).toUpperCase(); this.children = []; this.listeners = {}; this.parent = null;
    this.className = ''; this.textContent = ''; this.value = ''; this.style = {}; this._html = ''; this._stubs = {};
    const self = this;
    this.classList = { add(c) { self.className += ' ' + c; } };
  }
  appendChild(c) { c.parent = this; this.children.push(c); return c; }
  addEventListener(t, fn) { (this.listeners[t] = this.listeners[t] || []).push(fn); }
  remove() { if (this.parent) this.parent.children = this.parent.children.filter(x => x !== this); this.parent = null; }
  get innerHTML() { return this._html; }
  set innerHTML(h) { this._html = String(h); }
  hasClass(c) { return (' ' + this.className + ' ').indexOf(' ' + c + ' ') >= 0; }
  find(pred) {
    for (const k of this.children) { if (pred(k)) return k; const r = k.find(pred); if (r) return r; }
    return null;
  }
  querySelector(sel) {
    const cls = sel.charAt(0) === '.' ? sel.slice(1) : null;
    const hit = cls ? this.find(k => k.hasClass(cls)) : null;
    if (hit) return hit;
    if (cls && this._html.indexOf('class="' + cls + '"') >= 0) {     // made through innerHTML: a stand-in element
      if (!this._stubs[cls]) { this._stubs[cls] = new El('span'); this._stubs[cls].className = cls; }
      return this._stubs[cls];
    }
    return null;
  }
}
const head = new El('head'), body = new El('body');
const document = { head, body, createElement: t => new El(t), getElementById: () => null };
const sb = { document, console: { log() {}, warn() {}, info() {}, error() {} } };
sb.window = sb;
vm.createContext(sb);
vm.runInContext(fs.readFileSync(file, 'utf8'), sb, { filename: path.basename(file) });
const Q = sb.HTQwerty;
check(Q && typeof Q.show === 'function' && Q.N && Q.N.DOUBLE === 2 && Q.N.INTEGER === 1, 'qwerty.js exports HTQwerty.show / N');

function ok(typed, flags, opt) {
  const tgt = new El('input'); tgt.value = typed;
  let got = null;
  const o = Object.assign({ onCommit(v) { got = v; } }, opt);
  Q.show(tgt, flags, o);
  const btn = body.find(k => k.tagName === 'BUTTON' && k.textContent === 'OK');
  if (!btn || !btn.listeners.click) return { value: '(no OK button)', got };
  btn.listeners.click[0]();
  return { value: tgt.value, got };
}
const D = Q ? Q.N.DOUBLE : 2, I = Q ? Q.N.INTEGER : 1;
let r;
console.log('-- 1. DOUBLE with a range: the clamped value as is (golden :290)');
r = ok('10.5', D, { dp: 0, checkRange: true, min: 0, max: 100 });
check(r.value === '10.5' && r.got === '10.5', 'DOUBLE dp 0: 10.5 stays "10.5" (was "11")  [' + r.value + ']');
r = ok('10', D, { dp: 2, checkRange: true, min: 0, max: 15000 });
check(r.value === '10', 'DOUBLE dp 2: 10 stays "10" (was "10.00")  [' + r.value + ']');
r = ok('12.345', D, { dp: 2, checkRange: true, min: 0, max: 1000 });
check(r.value === '12.345', 'DOUBLE dp 2: 12.345 keeps its decimals  [' + r.value + ']');
console.log('-- 2. the clamp still applies');
r = ok('200', D, { dp: 2, checkRange: true, min: 0, max: 100 });
check(r.value === '100', 'DOUBLE above max -> max  [' + r.value + ']');
r = ok('5', D, { dp: 2, checkRange: true, min: 50, max: 10 });
check(r.value === '10', 'DOUBLE below min, min / max given reversed (golden swaps) -> 10  [' + r.value + ']');
r = ok('-3.5', D, { dp: 1, checkRange: true, min: -20, max: 0.1 });
check(r.value === '-3.5', 'DOUBLE negative in range stays  [' + r.value + ']');
console.log('-- 3. INTEGER and no range unchanged');
r = ok('7', I, { dp: 0, checkRange: true, min: 0, max: 64 });
check(r.value === '7', 'INTEGER in range  [' + r.value + ']');
r = ok('99', I, { dp: 0, checkRange: true, min: 0, max: 64 });
check(r.value === '64', 'INTEGER clamped  [' + r.value + ']');
r = ok('3.25', D, { dp: 2, checkRange: false });
check(r.value === '3.25', 'DOUBLE without a range: the text untouched  [' + r.value + ']');

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
