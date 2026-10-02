'use strict';
// =============================================================================
//  tools/webprobe/kb_machine_setting_selftest.cjs -- ctest KB_MachineSetting.  AI(W906-C2) 20261002 (St02-E).
//
//  Card C-2 "machine-setting fields": Main.CommView edtSetZ1 / edtSetZ2 keep golden 906_0625_Steven main.cpp:26175-26185's
//  two branches -- INDEX_DRIVER_TYPE==Mitsubishi_DRIVER (1) INTEGER 0..100, else 0..300 -- chosen when the keyboard opens
//  (web/page/ht9045_kb_generic.js, loaded at Main.CommView.html:177 after the wire data).
//  Offline: a fake window / document; the REAL ht9045_wire_maincommview.js registers through a stub HT9045Wire.register,
//  then the REAL ht9045_kb_generic.js runs with a stub HT9045System.read('gerneral').
//  Checks: Panasonic (2) / A4 (0) -> 0..300; Mitsubishi (1) -> 0..100; read failed / no value / not answered yet ->
//  0..100 (the safe side); every time the keyboard is opened here and the engine's handler stopped (independent of the
//  generated kb entry); other fields untouched; the kb table itself unchanged; OK fires input + change; another page -> no
//  listener; the load order in Main.CommView.html.
//  argv[2] = web/page.  CONTROL: W906_KB_GENERIC pointing at a file without the listener (e.g. an empty .js) must be red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const kbFile = process.env.W906_KB_GENERIC || path.join(pageDir, 'ht9045_kb_generic.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
const same = (a, b) => JSON.stringify(a) === JSON.stringify(b);

function load(readImpl, wireFile) {
  const listeners = [];
  const sb = {
    console: { log() {}, info() {}, warn() {}, error() {} },
    document: { readyState: 'loading', addEventListener(t, fn, cap) { listeners.push({ t, fn, cap: !!cap }); }, getElementById() { return null; } },
    Promise
  };
  sb.window = sb;
  sb.Event = function (type, init) { this.type = type; this.bubbles = !!(init && init.bubbles); };
  let cfg = null;
  sb.HT9045Wire = { register(c) { cfg = c; sb.HT9045Page = { cfg: c }; }, say() {} };
  sb.HT9045System = { read: readImpl };
  vm.createContext(sb);
  vm.runInContext(fs.readFileSync(path.join(pageDir, wireFile || 'ht9045_wire_maincommview.js'), 'utf8'), sb, { filename: 'wire' });
  const before = cfg && cfg.kb ? JSON.parse(JSON.stringify(cfg.kb)) : null;
  vm.runInContext(fs.readFileSync(kbFile, 'utf8'), sb, { filename: path.basename(kbFile) });
  return { sb, cfg, before, listeners };
}
function fire(v, id) {
  const shows = [];
  v.sb.HTQwerty = { N: { INTEGER: 1, DOUBLE: 2 }, show(el, flags, o) { shows.push({ id: el.id, flags, o }); } };
  const md = v.listeners.filter(l => l.t === 'mousedown' && l.cap);
  const fired = [];
  const el = { id, disabled: false, dispatchEvent(e) { fired.push(e.type); } };
  const ev = { target: el, pd: false, sp: false, preventDefault() { this.pd = true; }, stopPropagation() { this.sp = true; } };
  md.forEach(l => l.fn(ev));
  return { shows, ev, fired, n: md.length };
}
const settle = () => new Promise(r => setTimeout(r, 0));
const gern = v => () => Promise.resolve({ sections: { IndexDriver: { INDEX_DRIVER_TYPE: { raw: String(v), value: String(v) } } } });

(async () => {
  console.log('-- 1. golden main.cpp:26183: not Mitsubishi -> INTEGER 0..300');
  for (const t of [2, 0]) {
    const v = load(gern(t));
    await settle();
    check(v.sb.HT9045KbGeneric && v.sb.HT9045KbGeneric.driver().known && v.sb.HT9045KbGeneric.driver().type === t, 'INDEX_DRIVER_TYPE ' + t + ' read from Gerneral.ini [IndexDriver]');
    for (const id of ['edtSetZ1', 'edtSetZ2']) {
      const r = fire(v, id), s0 = r.shows[0];
      check(r.n === 1 && r.shows.length === 1 && s0.flags === 1 && s0.o.dp === 0 && s0.o.checkRange === true && s0.o.min === 0 && s0.o.max === 300 && r.ev.pd && r.ev.sp,
            id + ' with driver ' + t + ': INTEGER 0..300, the engine handler stopped');
      if (s0 && s0.o.onCommit) s0.o.onCommit();
      check(same(r.fired, ['input', 'change']), id + ': OK fires input + change');
    }
    const o = fire(v, 'edtReadZ1');
    check(o.shows.length === 0 && !o.ev.pd, 'other fields untouched');
    check(same(v.cfg.kb, v.before), 'the kb table itself is unchanged (the engine keeps the generated 0..100)');
  }
  console.log('-- 2. golden main.cpp:26179 / the safe side: INTEGER 0..100, opened here (not the generated entry)');
  const cases = [['Mitsubishi (1)', gern(1)], ['read rejected', () => Promise.reject(new Error('http 500'))],
                 ['no [IndexDriver] key', () => Promise.resolve({ sections: {} })]];
  for (const [what, impl] of cases) {
    const v = load(impl);
    await settle();
    const r = fire(v, 'edtSetZ1'), s0 = r.shows[0];
    check(r.n === 1 && r.shows.length === 1 && s0.flags === 1 && s0.o.min === 0 && s0.o.max === 100 && r.ev.pd && r.ev.sp,
          what + ': INTEGER 0..100, the engine handler stopped');
  }
  {
    let resolve;
    const v = load(() => new Promise(r => { resolve = r; }));
    await settle();                                 // the read starts one microtask after load (Promise.resolve().then); still pending
    check(typeof resolve === 'function' && !v.sb.HT9045KbGeneric.driver().known, 'the read has started and is still pending');
    const r = fire(v, 'edtSetZ2');
    check(r.shows.length === 1 && r.shows[0].o.max === 100 && r.ev.pd, 'not answered yet: 0..100 (the safe side)');
    resolve({ sections: { IndexDriver: { INDEX_DRIVER_TYPE: { raw: '2' } } } });
    await settle(); await settle();
    const r2 = fire(v, 'edtSetZ2');
    check(r2.shows.length === 1 && r2.shows[0].o.max === 300, 'answered later (2): the next opening gets 0..300');
  }
  console.log('-- 3. other pages, load order');
  const w = load(gern(2), 'ht9045_wire_setupsetup.js');
  check(w.listeners.filter(l => l.t === 'mousedown').length === 0, 'another page (Setup.SetUp): no listener');
  const html = fs.readFileSync(path.join(pageDir, 'Main.CommView.html'), 'utf8');
  const iE = html.indexOf('src="ht9045_wire_engine.js"'), iW = html.indexOf('src="ht9045_wire_maincommview.js"'), iK = html.indexOf('src="ht9045_kb_generic.js"');
  check(iE >= 0 && iW > iE && iK > iW, 'Main.CommView.html: engine, the wire data, then ht9045_kb_generic.js');
  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
  process.exit(fail ? 1 : 0);
})();
