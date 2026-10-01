'use strict';
// =============================================================================
//  tools/webprobe/tif_kb_selftest.cjs -- ctest TIF_KbGolden.  AI(W906-TIF-KB) 20261002 (St02-E).
//
//  docs/TESTERIF_FIELD_AUDIT_20261002.md P1 / P2: Setup.TesterIF's keyboard limits for 9 fields must be golden
//  906_0625_Steven cTesterIF.cpp's generic branch, not the customer branch the generated wire data took:
//    the six max-test-time fields   DOUBLE, 2, true, 0..15000   (:1401; generated INTEGER 60..9999 = CC_ASE_M :1384)
//    the three Initial Start Delay  DOUBLE, 2, false            (:1415; generated 30..3000 = HiSilicon + ASE_KaohSiung :1413)
//  Offline, no browser, no server: a fake window / document; the REAL ht9045_wire_setuptesterif.js registers its cfg
//  through a stub HT9045Wire.register (the engine's own register sets window.HT9045Page.cfg to that object), then the
//  REAL ht9045_testerif_c_wire.js runs (without HT9045Recipe its main block returns at once; block (8) does not need it).
//  Checks: the 9 entries are golden's; every other kb entry is untouched; nothing is applied (and a warning is logged) once
//  the keyboards are already attached; Setup.TesterIF.html loads the c_wire file after the wire data (read only).
//  VTEST (golden :1395-1398, AI(W906-TIF-KB) 20261002): with HT9045TesterIfC.vtest() true, a mousedown on a max-test-time
//  field opens the keyboard with DOUBLE 2, 0..36000 (and stops the engine's own handler); false -> untouched (the engine opens
//  it with the generic entry); other fields untouched; OK fires input + change; the export reads extra.vtest and the C++
//  line sends it (both pinned in the source, read only).
//  argv[2] = web/page.  CONTROL: W906_TIF_C_WIRE pointing at the file before (8) must turn the 9 checks red; at the file
//  before the VTEST half (c1e7f765) the VTEST checks must be red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const cwire = process.env.W906_TIF_C_WIRE || path.join(pageDir, 'ht9045_testerif_c_wire.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
const same = (a, b) => JSON.stringify(a) === JSON.stringify(b);

function load(attached) {
  const warns = [], listeners = [];
  const sb = {
    console: { log() {}, info() {}, warn(m) { warns.push(String(m)); }, error() {} },
    document: { readyState: 'loading', addEventListener(t, fn, cap) { listeners.push({ t, fn, cap: !!cap }); }, getElementById() { return null; } }
  };
  sb.Event = function (type, init) { this.type = type; this.bubbles = !!(init && init.bubbles); };
  sb.window = sb;
  let cfg = null;
  sb.HT9045Wire = { register(c) { cfg = c; sb.HT9045Page = { cfg: c }; }, say() {} };
  if (attached) sb.HT9045KbAudit = { page: 'Setup.TesterIF', bound: 1, unmapped: [] };
  vm.createContext(sb);
  vm.runInContext(fs.readFileSync(path.join(pageDir, 'ht9045_wire_setuptesterif.js'), 'utf8'), sb,
                  { filename: 'ht9045_wire_setuptesterif.js' });
  const before = cfg && cfg.kb ? JSON.parse(JSON.stringify(cfg.kb)) : null;
  vm.runInContext(fs.readFileSync(cwire, 'utf8'), sb, { filename: path.basename(cwire) });
  return { sb, cfg, before, warns, listeners };
}

const MAX = ['edMaxTestTime', 'edMaxTestTime_RT', 'edMaxTestTime_EQC', 'edInitialMaxTest', 'edInitialMaxTest_RT', 'edInitialMaxTest_EQC'];
const DLY = ['edtInitStartDelay', 'edtInitStartDelay_RT', 'edtInitStartDelay_EQC'];

console.log('-- 1. the 9 entries are golden cTesterIF.cpp\'s generic branch');
const a = load(false);
check(a.cfg && a.cfg.kb && a.before, 'the generated wire data registered a kb table');
const kb = (a.cfg && a.cfg.kb) || {};
for (const id of MAX) check(same(kb[id], ['DOUBLE', 2, true, 0, 15000]), id + ': DOUBLE, 2 decimals, 0..15000 (golden :1401), not INTEGER 60..9999 (:1384)');
for (const id of DLY) check(Array.isArray(kb[id]) && kb[id][0] === 'DOUBLE' && kb[id][1] === 2 && kb[id][2] === false,
                            id + ': DOUBLE, 2 decimals, no range (golden :1415), not 30..3000 (:1413)');

console.log('-- 2. nothing else changes');
const others = Object.keys(a.before || {}).filter(k => !MAX.includes(k) && !DLY.includes(k));
check(others.length > 20 && others.every(k => same(kb[k], a.before[k])), 'the other ' + others.length + ' kb entries are untouched');
check(a.sb.HT9045TesterIfKb && same(a.sb.HT9045TesterIfKb.applied.slice().sort(), MAX.concat(DLY).sort()), 'exactly the 9 ids applied');
check(a.warns.length === 0, 'no warning while the page is still parsing');

console.log('-- 3. too late once the keyboards are attached');
const b = load(true);
check(b.sb.HT9045TesterIfKb && b.sb.HT9045TesterIfKb.applied.length === 0, 'attached already: nothing applied');
check(b.warns.some(w => w.indexOf('already attached') >= 0), 'attached already: a warning is logged');

console.log('-- 4. load order (Setup.TesterIF.html, read only)');
const html = fs.readFileSync(path.join(pageDir, 'Setup.TesterIF.html'), 'utf8');
const iE = html.indexOf('src="ht9045_wire_engine.js"'), iW = html.indexOf('src="ht9045_wire_setuptesterif.js"'), iC = html.indexOf('src="ht9045_testerif_c_wire.js"');
check(iE >= 0 && iW > iE && iC > iW, 'engine, then the wire data, then ht9045_testerif_c_wire.js');

console.log('-- 5. VTEST (golden :1395-1398): 0..36000 when the server says extra.vtest');
{
  const v = load(false);
  const md = v.listeners.filter(l => l.t === 'mousedown' && l.cap);
  check(md.length === 1, 'one capture-phase mousedown listener');
  const shows = [];
  v.sb.HTQwerty = { N: { INTEGER: 1, DOUBLE: 2 }, show(el, flags, o) { shows.push({ id: el.id, flags, o }); } };
  let vt = false;
  v.sb.HT9045TesterIfC = { vtest() { return vt; } };
  function fire(id) {
    const fired = [];
    const el = { id, disabled: false, dispatchEvent(e) { fired.push(e.type); } };
    const ev = { target: el, pd: false, sp: false, preventDefault() { this.pd = true; }, stopPropagation() { this.sp = true; } };
    if (md[0]) md[0].fn(ev);
    return { ev, fired };
  }
  let r = fire('edMaxTestTime');
  check(shows.length === 0 && !r.ev.pd && !r.ev.sp, 'vtest false: the engine opens the keyboard (generic entry), the listener does nothing');
  vt = true;
  for (const id of MAX) {
    shows.length = 0;
    r = fire(id);
    const s0 = shows[0];
    check(shows.length === 1 && s0.flags === 2 && s0.o.dp === 2 && s0.o.checkRange === true && s0.o.min === 0 && s0.o.max === 36000 && r.ev.pd && r.ev.sp,
          id + ': vtest true -> DOUBLE, 2 decimals, 0..36000, the engine handler stopped');
  }
  shows.length = 0;
  r = fire('edtInitStartDelay');
  check(shows.length === 0 && !r.ev.pd, 'vtest true: Initial Start Delay untouched (golden :1415 has no VTEST branch)');
  r = fire('edOverSec');
  check(shows.length === 0 && !r.ev.pd, 'vtest true: other fields untouched');
  r = fire('edMaxTestTime');
  if (shows[0] && shows[0].o.onCommit) shows[0].o.onCommit();
  check(same(r.fired, ['input', 'change']), 'OK fires input + change on the field (as the engine)');
  const cw = fs.readFileSync(cwire, 'utf8');
  check(cw.indexOf('vtest: function () { return !!(LAST && LAST.extra && LAST.extra.vtest); }') >= 0, 'HT9045TesterIfC.vtest reads the last editlist.get extra.vtest');
  const cpp = fs.readFileSync(path.join(pageDir, '..', '..', 'HT9011UC_Cpp_V3.33.906.0', 'FileRW', 'TestIF_File_TesterIF.cpp'), 'utf8');
  const lineOf = cpp.split(/\r?\n/).find(l => l.indexOf('w.Key("vtest")') >= 0) || '';
  check(lineOf.indexOf('w.Key("vtest").Bool(IniConfig.bVTESTFunction != 0);') >= 0 &&
        lineOf.indexOf('w.Key("vtest")') < (lineOf.indexOf('//') < 0 ? Infinity : lineOf.indexOf('//')),
        'FileRW/TestIF_File_TesterIF.cpp sends extra.vtest = IniConfig.bVTESTFunction, as code before the comment');
}

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
