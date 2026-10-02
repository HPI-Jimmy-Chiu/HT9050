'use strict';
// =============================================================================
//  tools/webprobe/nonstop_route_selftest.cjs -- ctest NonStopRoute.  AI(W906-NONSTOP-SEM) 20261002.
//
//  The non-stop alarm window said 「機台未停機，仍在運轉」 for C++ ShowErrorMessage alarms whose code is in
//  web/config/AlarmNonStop.json (WAR1676 = golden cSecurity.cpp:592 Insufficient privileges, WAR1681 ...), although golden
//  ShowErrorMessage always StopAllMotor (note.cpp:795-808) and C++ does it too (W906_AlarmStopLikeGolden).  The REAL
//  web/page/ht9045_nonstop_alarm.js runs in a fake window (XHR serves the REAL web/config/AlarmNonStop.json):
//    C++ show-error-message requests -> the stopping page whatever the table says; web-local (channel nonstop-local,
//    origin html) requests still use the table; requestedSideEffects.stopAllMotor false (the message channel) unchanged.
//  argv[2] = web root.  CONTROL: W906_NONSTOP_JS pointing at the file before 0.5 must turn [1] red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const webRoot = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web');
const jsFile = process.env.W906_NONSTOP_JS || path.join(webRoot, 'page', 'ht9045_nonstop_alarm.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }

function makeSandbox() {
  const sb = { console: { log() {}, info() {}, warn() {}, error() {} }, Promise, Date, JSON, setTimeout, clearTimeout };
  sb.window = sb; sb.self = sb;
  sb.location = { protocol: 'http:', pathname: '/background.html' };
  sb.document = {
    currentScript: { src: 'http://127.0.0.1:8055/page/ht9045_nonstop_alarm.js' },
    getElementsByTagName() { return []; }, createElement() { return {}; }, head: { appendChild() {} },
    addEventListener() {}, readyState: 'complete'
  };
  sb.XMLHttpRequest = function () {
    const x = this; let url = '';
    x.open = function (m, u) { url = u; };
    x.send = function () {
      const p = url.replace(/^https?:\/\/[^/]+\//, '').replace(/\?.*$/, '');
      const f = path.join(webRoot, p);
      setTimeout(function () {
        if (fs.existsSync(f)) { x.status = 200; x.responseText = fs.readFileSync(f, 'utf8'); } else { x.status = 404; x.responseText = ''; }
        x.readyState = 4; if (x.onreadystatechange) x.onreadystatechange();
      }, 0);
    };
  };
  vm.createContext(sb);
  vm.runInContext(fs.readFileSync(jsFile, 'utf8'), sb, { filename: path.basename(jsFile) });
  return sb;
}

(async function () {
  const sb = makeSandbox();
  const NS = sb.HT9045NonStop;
  check(NS && typeof NS.route === 'function', 'ht9045_nonstop_alarm.js exports HT9045NonStop.route');
  const table = JSON.parse(fs.readFileSync(path.join(webRoot, 'config', 'AlarmNonStop.json'), 'utf8'));
  const inTable = (c) => !!(table.codes && Object.prototype.hasOwnProperty.call(table.codes, c));
  check(inTable('WAR1676') || inTable('WAR1681'), 'the table really lists WAR1676 / WAR1681 (the codes that showed wrong)');

  const cpp = (code) => ({ schemaVersion: '1.0.0', channel: 'show-error-message', function: 'ShowErrorMessage', blocking: true,
                           arguments: { code: code, kCode: 0, position: 0 } });
  for (const code of ['WAR1676', 'WAR1681']) {
    const r = await NS.route('alarm', cpp(code));
    check(r && r.info === null && r.kind === 'alarm', '[1] C++ ShowErrorMessage ' + code + ' -> the stopping page (' + (r && r.why) + ')');
  }
  // every code of the table, from C++: none goes to a non-stop page (enumerated, not sampled)
  const codes = Object.keys(table.codes || {});
  let wrong = [];
  for (const code of codes) { const r = await NS.route('alarm', cpp(code)); if (!r || r.info !== null) wrong.push(code); }
  check(codes.length > 0 && wrong.length === 0, '[2] all ' + codes.length + ' table codes raised by C++ ShowErrorMessage -> stopping page; wrong: ' + JSON.stringify(wrong.slice(0, 5)));

  const local = { channel: 'nonstop-local', origin: 'html', arguments: { code: inTable('WAR1676') ? 'WAR1676' : 'WAR1681' } };
  const rl = await NS.route('alarm', local);
  check(rl && rl.info !== null && rl.kind === 'alarmNonStop', '[3] a web-local alarm (nonstop-local, origin html) still uses the table (' + (rl && rl.why) + ')');

  const msg = { channel: 'show-my-message', requestedSideEffects: { stopAllMotor: false }, arguments: { code: local.arguments.code } };
  const rm = await NS.route('message', msg);
  check(rm && rm.kind === 'messageNonStop', '[4] the message channel with stopAllMotor:false is unchanged (' + (rm && rm.why) + ')');
  const ms2 = { channel: 'show-my-message', requestedSideEffects: { stopAllMotor: true }, arguments: { code: local.arguments.code } };
  const rm2 = await NS.route('message', ms2);
  check(rm2 && rm2.info === null, '[5] the message channel with stopAllMotor:true -> stopping page (unchanged)');

  console.log('PASS: ' + pass + '/' + (pass + fail) + ' checks');
  process.exitCode = fail ? 1 : 0;
})().catch(function (e) { console.log('  FAIL threw: ' + e.message); process.exitCode = 1; });
