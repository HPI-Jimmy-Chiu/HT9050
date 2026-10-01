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
//  argv[2] = web/page.  CONTROL: W906_TIF_C_WIRE pointing at the file before (8) must turn the 9 checks red.
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
  const warns = [];
  const sb = {
    console: { log() {}, info() {}, warn(m) { warns.push(String(m)); }, error() {} },
    document: { readyState: 'loading', addEventListener() {}, getElementById() { return null; } }
  };
  sb.window = sb;
  let cfg = null;
  sb.HT9045Wire = { register(c) { cfg = c; sb.HT9045Page = { cfg: c }; }, say() {} };
  if (attached) sb.HT9045KbAudit = { page: 'Setup.TesterIF', bound: 1, unmapped: [] };
  vm.createContext(sb);
  vm.runInContext(fs.readFileSync(path.join(pageDir, 'ht9045_wire_setuptesterif.js'), 'utf8'), sb,
                  { filename: 'ht9045_wire_setuptesterif.js' });
  const before = cfg && cfg.kb ? JSON.parse(JSON.stringify(cfg.kb)) : null;
  vm.runInContext(fs.readFileSync(cwire, 'utf8'), sb, { filename: path.basename(cwire) });
  return { sb, cfg, before, warns };
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

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
