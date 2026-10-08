// AI(W906-W169) 20261008 (NB2-1): W-169 page half -- web/page/ht9045_contact_ev.js, the block between "// W169-BEGIN" and
//   "// W169-END" (W-157 ruling, Steven 1008 07:5x, RULINGS_20261008 #1 (2): F5 / reload is not a close).  The C++ half is ctest
//   NB2_W169ContactClose (tests/test_nb2_w169_contactclose.cpp).
//   How: that block alone in a node vm with a fake window (addEventListener), HT9045Tags (on) and HT9045Wire (say):
//     [1] the block is there, the file has one EOL style and no BOM
//     [2] tag contact.running never seen / 0: F5, Ctrl+R and beforeunload are left alone
//     [3] contact.running 1: F5 / Ctrl+R / Ctrl+Shift+R / Ctrl+F5 / keyCode 116 are swallowed (preventDefault, stopPropagation, a message);
//         a plain key is not; beforeunload asks (preventDefault + returnValue '')
//     [4] back to 0: nothing is swallowed again
//   Control: W906_W169_PAGE_DIR -> the page before W-169 (no block) must be red.  Reads one .js under web\page.
//   Use: only through ctest (NB2_W169ContactPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_W169_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const FILE = path.join(PAGE_DIR, 'ht9045_contact_ev.js');

let failed = 0, passed = 0;
function check(name, fn) {
  try { fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }

const raw = fs.readFileSync(FILE);
const text = raw.toString('utf8');
const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
const b = text.indexOf('// W169-BEGIN'), e = text.indexOf('// W169-END');

check('[1] block present (W169-BEGIN / W169-END), one EOL style, no BOM', () => {
  ok(b >= 0 && e > b, 'W169-BEGIN / W169-END markers not found in ' + FILE);
  ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf);
  ok(!(raw[0] === 0xEF && raw[1] === 0xBB && raw[2] === 0xBF), 'BOM');
});

const listeners = {}, tagCb = {}, said = [];
const win = {
  addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
  HT9045Tags: { on: (name, cb) => { tagCb[name] = cb; } },
  HT9045Wire: { say: m => { said.push(m); } },
};
win.window = win;
let loaded = false;
if (b >= 0 && e > b) {
  try { vm.runInNewContext(text.slice(b, e), win, { filename: 'ht9045_contact_ev.js#W169' }); loaded = true; }
  catch (x) { console.log('vm error: ' + (x && x.message || x)); }
}

function key(k, code, mods) {
  const ev = Object.assign({ key: k, keyCode: code, ctrlKey: false, metaKey: false, shiftKey: false, prevented: false, stopped: false }, mods || {});
  ev.preventDefault = () => { ev.prevented = true; };
  ev.stopPropagation = () => { ev.stopped = true; };
  (listeners.keydown || []).forEach(fn => fn(ev));
  return ev;
}
function unload() {
  const ev = { prevented: false, returnValue: undefined };
  ev.preventDefault = () => { ev.prevented = true; };
  (listeners.beforeunload || []).forEach(fn => { const r = fn(ev); if (r !== undefined) ev.ret = r; });
  return ev;
}

check('[2] not running: F5 / Ctrl+R / beforeunload left alone; the block subscribed to contact.running', () => {
  ok(loaded, 'block did not load');
  ok(typeof tagCb['contact.running'] === 'function', 'no subscription to tag contact.running');
  ok(!key('F5', 116).prevented, 'F5 swallowed before any tag');
  tagCb['contact.running'](0);
  ok(!key('F5', 116).prevented && !key('r', 82, { ctrlKey: true }).prevented, 'swallowed while contact.running 0');
  const u = unload();
  ok(!u.prevented && u.returnValue === undefined, 'beforeunload asked while not running');
});

check('[3] contact.running 1: reload keys swallowed with a message, a plain key not, beforeunload asks', () => {
  ok(loaded, 'block did not load');
  tagCb['contact.running'](1);
  const n0 = said.length;
  const f5 = key('F5', 116);
  ok(f5.prevented && f5.stopped, 'F5 not swallowed');
  ok(key('r', 82, { ctrlKey: true }).prevented, 'Ctrl+R not swallowed');
  ok(key('R', 82, { ctrlKey: true, shiftKey: true }).prevented, 'Ctrl+Shift+R not swallowed');
  ok(key('F5', 116, { ctrlKey: true }).prevented, 'Ctrl+F5 not swallowed');
  ok(key('', 116).prevented, 'keyCode 116 without key not swallowed');
  ok(!key('a', 65).prevented, 'a plain key was swallowed');
  ok(said.length > n0 && /Auto Height/.test(said[said.length - 1]), 'no message said');
  const u = unload();
  ok(u.prevented && u.returnValue === '', 'beforeunload did not ask');
});

check('[4] back to 0: nothing swallowed', () => {
  ok(loaded, 'block did not load');
  tagCb['contact.running'](0);
  ok(!key('F5', 116).prevented, 'F5 still swallowed');
  ok(!unload().prevented, 'beforeunload still asks');
});

check('[5] a window without addEventListener (other page tests run the whole file that way): the block does not throw', () => {
  ok(b >= 0 && e > b, 'block missing');
  const w2 = { HT9045Tags: { on: () => {} } };
  w2.window = w2;
  vm.runInNewContext(text.slice(b, e), w2, { filename: 'ht9045_contact_ev.js#W169-noAEL' });
});

console.log('\n' + passed + ' / ' + (passed + failed) + ' passed');
process.exit(failed === 0 ? 0 : 1);
