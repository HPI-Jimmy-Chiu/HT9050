// AI(W906-D034) 20261002 [W906] St01: offline selftest for the page half of todo D-034's DoPassword_MBox (ctest D034_IniConfigFifoPage).
//   web/page/ht9045_iniconfig_auth_c.js wraps HT9045Recipe.editlistSave for IniConfig: when [I37_1] FIFO is turned on (golden V912
//   cConfiguration.cpp:7353-7369 CheckConfigurationBeforeSave -> MyMessageBox->DoPassword_MBox, mymessbox.cpp:1228-1286) it opens the
//   login keypad and sends the answer as editlist.save's extra.reauth {point:"i37_1", ...}; with [M01] changed too golden asks twice,
//   so the page asks M01 first, then FIFO, and sends an array.  The C++ half (the compare, the level, the logout) is
//   tests/test_weblogin_reauth.cpp (WebLogin_Reauth) step 6b.
//   Loads the REAL page script in a node vm with a fake window / document / HTQwerty / HT9045Recipe.  No socket, no wb_serve, no file
//   written.  Usage: node tools/webprobe/d034_iniconfig_fifo_selftest.cjs [<web/page dir>]
//   Control run: W906_INICONFIG_AUTH_JS at the pre-change file -> must go red.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_INICONFIG_AUTH_JS || path.join(PAGE, 'ht9045_iniconfig_auth_c.js');
const code = fs.readFileSync(SRC, 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }
function rnd(prefix) { return prefix + Math.random().toString(36).slice(2, 10) + Date.now().toString(36).slice(-3); }
const SECRET = rnd('pw'), SECRET2 = rnd('pq'), USER = 'u' + rnd('');
const LOGS = [];

function makePage(opts) {
  opts = opts || {};
  const keypadQueue = [];                       // scripted keypad results: string = commit, null = abort
  const keypadCalls = [];
  const saves = [];
  let nextAck = null;
  const raised = [];
  const banners = [];
  const doc = {
    body: { appendChild(el) { el._in = true; } },
    head: { appendChild() {} }, documentElement: { appendChild() {} },
    getElementById() { return null; },
    createElement() {
      const el = { style: {}, _text: '', remove() { this._in = false; } };
      Object.defineProperty(el, 'textContent', { get() { return this._text; }, set(v) { this._text = v; banners.push(String(v)); } });
      return el;
    },
  };
  const win = {};
  win.window = win;
  win.HT9045NonStop = { raise(o) { raised.push(o); } };
  const R = {
    editlistGet() { return Promise.resolve(opts.get); },
    editlistSave(st, widgets, answers, extra) {
      saves.push({ st, extra: extra ? JSON.parse(JSON.stringify(extra)) : extra, live: extra });
      const a = nextAck || { saved: true };
      nextAck = null;
      return Promise.resolve(JSON.parse(JSON.stringify(a)));
    },
  };
  win.HT9045Recipe = R;
  const HTQwerty = {
    N: { NO_SYMBOL: 4, PASSWORD: 8, NO_SPACE: 16 },
    show(el, flags, cb) {
      keypadCalls.push(flags);
      const v = keypadQueue.length ? keypadQueue.shift() : null;
      if (v === null) cb.onAbort(); else cb.onCommit(v);
    },
  };
  const fakeConsole = { log: (...a) => LOGS.push(a.join(' ')), warn: (...a) => LOGS.push(a.join(' ')), error: (...a) => LOGS.push(a.join(' ')), info: (...a) => LOGS.push(a.join(' ')) };
  const ctx = { window: win, document: doc, HTQwerty, console: fakeConsole, setTimeout, clearTimeout, Promise, JSON, Object, String, Number, Array, Date, Error, Math };
  vm.createContext(ctx);
  vm.runInContext(code, ctx, { filename: SRC });
  return {
    R, win, saves, keypadCalls, raised, banners,
    keys(...v) { keypadQueue.push(...v); },
    ack(a) { nextAck = a; },
    probe: win.HT9045IniConfigAuth,
  };
}

const M01 = ['cbM01'].concat(Array.from({ length: 15 }, (_, i) => 'cbM01_' + String(i + 1).padStart(2, '0')));
function getData(mode, fifoPoint, openFifo, fifoEditable) {
  const proxies = { cbI37_1: { checked: !!openFifo } };
  if (fifoEditable === false) proxies.cbI37_1.editable = false;
  M01.forEach((id) => { proxies[id] = { checked: false }; });
  const points = [{ id: 'm01', controls: M01, kind: 'relogin', armed: true, levelItem: 92, level: 2, logoutAfter: mode === 'book' }];
  if (fifoPoint) points.push(fifoPoint);
  points.push({ id: 'c12', controls: ['cbC12'], kind: 'vendor', armed: false, why: 'x' });
  return { proxies, extra: { auth: { mode, realTimeCcd: true, points } } };
}
function fifoPt(over) {
  return Object.assign({ id: 'i37_1', controls: ['cbI37_1'], kind: 'mbox-password', armed: true, turnOnOnly: true, levelItem: 35, level: 2,
                         levelZeroSkips: false, logoutAfter: true, golden: 'x' }, over || {});
}

(async function main() {
  console.log('D034_IniConfigFifoPage  source: ' + SRC);
  let pg = makePage({ get: getData('book', fifoPt(), false) });
  check(pg.R.__iniconfigAuthWrapped === true && pg.probe, 'the script wrapped editlistSave and exposes its probe');
  await pg.R.editlistGet('IniConfig');
  check(pg.probe && typeof pg.probe.fifo === 'function' && pg.probe.state().fifoArmed === true, 'probe: the [I37_1] point is read from extra.auth');

  // ---- 1. nothing changed: no keypad, no reauth ----------------------------------------------------------------------
  await pg.R.editlistSave('IniConfig', { cbI37_1: { checked: false } }, {}, null);
  check(pg.keypadCalls.length === 0 && pg.saves.length === 1 && !(pg.saves[0].extra && pg.saves[0].extra.reauth), 'FIFO left off: no keypad, no reauth');

  // ---- 2. FIFO turned on, book mode: user + password, then {point:"i37_1"} ------------------------------------------
  pg.keys(USER, SECRET);
  pg.ack({ saved: true, reauth: { point: 'i37_1', answered: true, asked: true, handled: true, passed: true, loggedOut: true, reason: 'password book login; level 2 >= 2', login: { userCaption: 'Operator', level: 0 } } });
  let a = await pg.R.editlistSave('IniConfig', { cbI37_1: { checked: true } }, {}, null);
  let s = pg.saves[pg.saves.length - 1];
  check(pg.keypadCalls.length === 2 && (pg.keypadCalls[1] & 8) === 8, 'FIFO on: two keypad steps (user, then password with N_PASSWORD)');
  check(s.extra && s.extra.reauth && !Array.isArray(s.extra.reauth) && s.extra.reauth.point === 'i37_1' && s.extra.reauth.userId === USER &&
        s.extra.reauth.password === SECRET, 'extra.reauth = {point:"i37_1", userId, password} as typed');
  check(s.live && s.live.reauth === null, 'the reauth object is dropped from extra after the reply');
  const msgs = (a && a.session && a.session.messages) || [];
  check(msgs.some((m) => /\[I37_1\] FIFO 密碼通過/.test(m.zh)) && msgs.some((m) => /DoPassword_MBox/.test(m.en)), 'the reply becomes a FIFO status line');
  check(pg.banners.some((b) => /DoPassword_MBox/.test(b) && /第 35 項/.test(b)), 'the keypad banner names golden DoPassword_MBox and levelset item 35');

  // ---- 3. M01 changed and FIFO on: M01 first, then FIFO, sent as an array; reauthAll reported ----------------------
  pg = makePage({ get: getData('book', fifoPt(), false) });
  await pg.R.editlistGet('IniConfig');
  pg.keys(USER, SECRET, USER, SECRET2);
  pg.ack({ saved: true, reauth: { point: 'm01', asked: true, handled: true, passed: true, reason: 'm01 ok', login: { level: 0 } },
           reauthAll: [{ point: 'm01', asked: true, handled: true, passed: true, reason: 'm01 ok', login: { level: 0 } },
                       { point: 'i37_1', asked: true, handled: true, passed: false, alarm: 'WAR1677', reason: 'wrong', reverted: ['cbI37_1'], login: { level: 0 } }] });
  a = await pg.R.editlistSave('IniConfig', { cbI37_1: { checked: true }, cbM01_03: { checked: true } }, {}, null);
  s = pg.saves[pg.saves.length - 1];
  check(pg.keypadCalls.length === 4, 'two boxes: four keypad steps');
  check(s.extra && Array.isArray(s.extra.reauth) && s.extra.reauth.length === 2 && s.extra.reauth[0].point === 'm01' && s.extra.reauth[1].point === 'i37_1' &&
        s.extra.reauth[0].password === SECRET && s.extra.reauth[1].password === SECRET2, 'extra.reauth = [m01, i37_1] in golden order');
  const ms3 = (a && a.session && a.session.messages) || [];
  check(ms3.some((m) => /重新登入通過/.test(m.zh)) && ms3.some((m) => /\[I37_1\] FIFO 密碼沒有通過/.test(m.zh) && /WAR1677/.test(m.zh)), 'reauthAll: one line per box');
  check(pg.raised.length === 1 && pg.raised[0].code === 'WAR1677' && /FIFO/.test(pg.raised[0].messageZh), 'WAR1677 raised once, as the FIFO box');
  check(s.live && Array.isArray(s.live.reauth) === false, 'the array is dropped from extra after the reply');

  // ---- 4. Abort = cancelled (golden blank = wrong) ------------------------------------------------------------------
  pg = makePage({ get: getData('book', fifoPt(), false) });
  await pg.R.editlistGet('IniConfig');
  pg.keys(USER, null);
  await pg.R.editlistSave('IniConfig', { cbI37_1: { checked: true } }, {}, null);
  s = pg.saves[pg.saves.length - 1];
  check(s.extra && s.extra.reauth && s.extra.reauth.point === 'i37_1' && s.extra.reauth.cancelled === true && !('password' in s.extra.reauth),
        'Abort sends {point:"i37_1", cancelled:true}, no password field');

  // ---- 5. drop-down mode: password only -------------------------------------------------------------------------------
  pg = makePage({ get: getData('select', fifoPt({ logoutAfter: false }), false) });
  await pg.R.editlistGet('IniConfig');
  pg.keys(SECRET);
  await pg.R.editlistSave('IniConfig', { cbI37_1: { checked: true } }, {}, null);
  s = pg.saves[pg.saves.length - 1];
  check(pg.keypadCalls.length === 1 && s.extra.reauth.point === 'i37_1' && !('userId' in s.extra.reauth) && s.extra.reauth.password === SECRET,
        'drop-down mode: one keypad step, no userId');

  // ---- 6. no ask when golden would not ask ---------------------------------------------------------------------------
  for (const [what, data, widgets] of [
    ['FIFO turned off (golden asks only on)', getData('book', fifoPt(), true), { cbI37_1: { checked: false } }],
    ['FIFO already on (no change)', getData('book', fifoPt(), true), { cbI37_1: { checked: true } }],
    ['CC_PTI vendor point (armed:false, not offered)', getData('book', fifoPt({ kind: 'vendor', armed: false }), false), { cbI37_1: { checked: true } }],
    ['the cell is not editable (dropped by the server)', getData('book', fifoPt(), false, false), { cbI37_1: { checked: true } }],
    ['an older C++ without the point', getData('book', null, false), { cbI37_1: { checked: true } }],
  ]) {
    pg = makePage({ get: data });
    await pg.R.editlistGet('IniConfig');
    await pg.R.editlistSave('IniConfig', widgets, {}, null);
    s = pg.saves[pg.saves.length - 1];
    check(pg.keypadCalls.length === 0 && !(s.extra && s.extra.reauth), what + ': no keypad, no reauth');
  }

  // ---- 7. M01 alone is unchanged (B5): a single object ---------------------------------------------------------------
  pg = makePage({ get: getData('book', fifoPt(), false) });
  await pg.R.editlistGet('IniConfig');
  pg.keys(USER, SECRET);
  await pg.R.editlistSave('IniConfig', { cbM01_01: { checked: true }, cbI37_1: { checked: false } }, {}, null);
  s = pg.saves[pg.saves.length - 1];
  check(s.extra && s.extra.reauth && !Array.isArray(s.extra.reauth) && s.extra.reauth.point === 'm01', 'M01 alone: {point:"m01"} as before');

  // ---- 8. the server reports "no answer" / "not reached" for FIFO ------------------------------------------------------
  pg = makePage({ get: getData('book', fifoPt(), false) });
  await pg.R.editlistGet('IniConfig');
  pg.ack({ saved: true, reauth: { point: 'i37_1', answered: false, asked: true, handled: false, passed: false, reason: 'no answer', reverted: ['cbI37_1'], login: {} } });
  a = await pg.R.editlistSave('IniConfig', {}, {}, null);
  check(((a.session || {}).messages || []).some((m) => /存檔沒有帶帳號密碼/.test(m.zh) && /FIFO 維持關閉/.test(m.zh)), 'handled:false -> "FIFO stays off, save again"');

  // ---- 9. no password in any log, banner or status line --------------------------------------------------------------
  const all = LOGS.join('\n');
  check(all.indexOf(SECRET) < 0 && all.indexOf(SECRET2) < 0, 'no console line carries a password');
  check(!/console\.(log|warn|error|info)\([^)]*(password|pw\b)/i.test(code), 'the page never logs a password field');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL: exception ' + (e && e.stack)); process.exit(1); });
