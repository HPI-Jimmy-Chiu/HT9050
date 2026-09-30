// AI(W906-STREAM-S2E-MV) 20260930: stage 2E for Motion View's motor poll -- web/page/ht9045_mv_motor.js must not fetch
//   /api/struct/motor/runtime while its window is closed (RULINGS_20260930 #12 「有開的網頁才能更新資料」; golden forms poll only
//   while fShow). Main.MotionView.html adds the script AFTER the page loaded (:2983), so it missed the frame's one-time HT_WIN of
//   the iframe load and polled 2x/s with its window never opened (laptop measurement 20260930 23:5x: 80 polls / 3.7 MB in 40 s).
//   The fix reads the frame's WIN_STATE once at start (background.html `var WIN_STATE`, id from the .win around the iframe).
//   Runs the real file in a node vm with a fake window / frameElement / parent / setInterval / fetch.
//   Control: W906_S2E_PAGE_DIR (or argv[2]) -> a directory holding the pre-change ht9045_mv_motor.js must be red.
//   Offline: no wb_serve, no machine files (reads the one .js). Usage: through ctest (Stream2E_MvMotor).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_S2E_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const CODE = fs.readFileSync(path.join(PAGE_DIR, 'ht9045_mv_motor.js'), 'utf8');

let passed = 0, failed = 0;
function check(name, cond, detail) {
  if (cond) { passed++; console.log('PASS ' + name); }
  else { failed++; console.log('FAIL ' + name + (detail ? ' -- ' + detail : '')); }
}
const flush = async () => { for (let i = 0; i < 6; i++) await new Promise((r) => setImmediate(r)); };

// frame: undefined = standalone page (no frameElement); 'throw' = parent not readable; else the WIN_STATE value of 'motionview'
async function start(frame) {
  const timers = [], listeners = [];
  const env = { fetches: 0 };
  const ctx = {
    console, Date, Math, Promise, JSON, Error, String, Number, Object, Array, RegExp,
    location: { protocol: 'http:' },
    document: { hidden: false },
    setTimeout(fn) { fn(); return 1; },
    setInterval(fn) { timers.push(fn); return timers.length; },
    fetch() { env.fetches++; return Promise.resolve({ ok: true, json: () => Promise.resolve({ motors: [{ motorId: 'M1', position: { cmdPos: 1 } }] }) }); },
    applyRuntimeState() {},
    addEventListener(type, fn) { if (type === 'message') listeners.push(fn); },
  };
  ctx.window = ctx;
  if (frame === 'throw') {
    Object.defineProperty(ctx, 'frameElement', { get() { throw new Error('SecurityError: cross-origin'); } });
  } else if (frame !== undefined) {
    const win = { id: 'win-motionview' };
    ctx.frameElement = { closest: (sel) => (sel === '.win' ? win : null) };
    ctx.parent = { WIN_STATE: { motionview: frame } };
  }
  vm.createContext(ctx);
  vm.runInContext(CODE, ctx, { filename: 'ht9045_mv_motor.js' });
  await flush();
  env.tick = async (n) => { for (let i = 0; i < n; i++) { timers.forEach((fn) => fn()); await flush(); } };
  env.post = async (open) => { listeners.forEach((fn) => fn({ data: { type: 'HT_WIN', id: 'motionview', open, state: open ? 'open' : 'closed' } })); await flush(); };
  return env;
}

(async () => {
  let e = await start('never');
  await e.tick(10);
  check('window never opened (WIN_STATE never): no fetch at start or on 10 ticks', e.fetches === 0, 'fetches=' + e.fetches);
  await e.post(true);
  check('HT_WIN open edge: one fetch at once', e.fetches === 1, 'fetches=' + e.fetches);
  await e.tick(4);
  check('open: fetches on every tick', e.fetches === 5, 'fetches=' + e.fetches);
  await e.post(false);
  const n = e.fetches;
  await e.tick(6);
  check('HT_WIN close edge: no more fetches', e.fetches === n, 'fetches ' + n + ' -> ' + e.fetches);

  e = await start('closed');
  await e.tick(5);
  check('WIN_STATE closed at start: no fetch', e.fetches === 0, 'fetches=' + e.fetches);

  e = await start('open');
  check('WIN_STATE open at start: fetches at start', e.fetches === 1, 'fetches=' + e.fetches);
  await e.tick(3);
  check('WIN_STATE open: fetches on every tick', e.fetches === 4, 'fetches=' + e.fetches);

  e = await start('minimized');
  await e.tick(3);
  check('WIN_STATE minimized = open (golden keeps fShow): fetches', e.fetches === 4, 'fetches=' + e.fetches);

  e = await start(undefined);
  await e.tick(3);
  check('standalone page (no frame): polls as before', e.fetches === 4, 'fetches=' + e.fetches);

  e = await start('throw');
  await e.tick(3);
  check('parent not readable: polls as before (nothing throws)', e.fetches === 4, 'fetches=' + e.fetches);

  check('the file decodes as UTF-8 without U+FFFD', CODE.indexOf(String.fromCharCode(0xFFFD)) < 0);
  console.log('\n' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})().catch((err) => { console.log('FAIL exception: ' + (err && err.stack || err)); process.exit(1); });
