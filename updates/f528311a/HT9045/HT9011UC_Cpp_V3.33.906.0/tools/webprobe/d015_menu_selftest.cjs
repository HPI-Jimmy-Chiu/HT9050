// AI(W906-D015-A01b) 20260930 [W906] St01: offline selftest for the page half of D-015 A01b (ctest D015_A01MenuPage).
//   The main screen's Tools / Config drop-downs (golden palSetup / palConfig, web/page/main.html #palSetup / #palConfig) reach C++ as
//   one WS command act.main.menuVisible {"setup":bool,"config":bool} (web/page/ht9045_main_st01_ev.js initMenuReport; C++
//   FileRW/Main_A01AutoLogout.cpp W906_A01MenuVisibleOp -> golden V912 main.cpp:25965 Timer3Timer resets the [A01] idle count), and the
//   page closes both when tag auth.level drops from >0 to 0 (golden ChangeLevelAttr main.cpp:12928-12935 + :13182-13190).
//   Loads the REAL ht9045_main_st01_ev.js and ht9045_busy_util.js in a node vm with a fake DOM (style.display setter -> MutationObserver),
//   a fake HT9045Recipe (scripted acks) and a fake HT9045Tags, on a fake clock. Also reads main.html / ht9xxx.css (read-only) to ratchet
//   the assumption the script is built on: main.html shows / hides the two menus ONLY through the inline style.display.
//   No socket, no wb_serve, no file written.  Usage: node tools/webprobe/d015_menu_selftest.cjs [<web/page dir>]
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_MAIN_ST01_EV || path.join(PAGE, 'ht9045_main_st01_ev.js');   // control run: point at the pre-A01b file -> must go red
const code = fs.readFileSync(SRC, 'utf8');
const busyCode = fs.readFileSync(path.join(PAGE, 'ht9045_busy_util.js'), 'utf8');
const mainHtml = fs.readFileSync(path.join(PAGE, 'main.html'), 'utf8');
const css = fs.readFileSync(path.join(PAGE, 'ht9xxx.css'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock
let clock = 0, timers = [], tid = 1;
function fakeSetTimeout(fn, ms) { const id = tid++; timers.push({ id, due: clock + Math.max(0, ms | 0), fn }); return id; }
function fakeClearTimeout(id) { timers = timers.filter((t) => t.id !== id); }
async function settle() { for (let i = 0; i < 12; i++) await new Promise((r) => setImmediate(r)); }
async function advance(ms) {
  const end = clock + ms;
  await settle();
  for (;;) {
    timers.sort((a, b) => a.due - b.due || a.id - b.id);
    const t = timers[0];
    if (!t || t.due > end) break;
    timers.shift();
    clock = t.due;
    t.fn();
    await settle();
  }
  clock = end;
  await settle();
}

// ---------------------------------------------------------------- fake page
function makePage(opts) {
  const els = {};
  function el(id) {
    const e = { id, listeners: {}, __mos: [] };
    e.style = {};
    ['display', 'left'].forEach((prop) => {                        // toggleMenu() writes both; each write changes the style attribute
      let v0 = '';
      Object.defineProperty(e.style, prop, {
        enumerable: true,
        get() { return v0; },
        set(v) { v0 = String(v); e.__mos.forEach((mo) => mo.__notify(e)); },
      });
    });
    e.addEventListener = (t, f) => { (e.listeners[t] = e.listeners[t] || []).push(f); };
    e.contains = () => false;
    els[id] = e;
    return e;
  }
  ['palSetup', 'palConfig', 'palDebug', 'SitePanel'].forEach(el);
  if (opts.main !== false) el('palMainStatus');
  class FakeMO {
    constructor(cb) { this.cb = cb; this.q = false; }
    observe(target, o) { this.opts = o; target.__mos.push(this); }
    disconnect() {}
    __notify(target) {
      if (!this.opts || !this.opts.attributes) return;
      if (this.q) return;
      this.q = true;
      Promise.resolve().then(() => { this.q = false; this.cb([{ type: 'attributes', attributeName: 'style', target }], this); });   // microtask, as the real one
    }
  }
  const srv = { cmds: [], log: [], holds: false, script: [], held: null };
  const R = {
    status() { return { connected: true, holdsToken: srv.holds }; },
    keepAlive() { srv.log.push('control.acquire'); srv.holds = true; return Promise.resolve(); },
    release() { srv.log.push('control.release'); srv.holds = false; return Promise.resolve(); },
    rawCmd(name, extra) {
      srv.cmds.push({ name, value: extra && extra.value, at: clock });
      srv.log.push(name);
      const next = srv.script.length ? srv.script.shift() : 'ok';
      if (next === 'hold') return new Promise((res, rej) => { srv.held = { res, rej }; });
      if (next === 'ok') return Promise.resolve({ type: 'ack', ok: true, executed: true });
      return Promise.reject(new Error(next));
    },
  };
  const tagFns = {};
  const Tags = {
    on(tag, fn) { (tagFns[tag] = tagFns[tag] || []).push(fn); return () => {}; },
    emit(tag, v) { (tagFns[tag] || []).forEach((f) => f(v, tag)); },
  };
  const says = [];
  const sb = {
    console: { info() {}, warn() {}, error(...a) { console.error(...a); }, log() {} },
    setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    MutationObserver: opts.noMO ? undefined : FakeMO,
    HT9045Recipe: R, HT9045Tags: Tags,
    HT9045Wire: { say(msg) { says.push(msg); } },
    getComputedStyle(e) { return { display: e.style.display || 'none' }; },   // ht9xxx.css .submenu{display:none}
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
  };
  sb.window = sb;
  sb.document = {
    readyState: 'complete',
    getElementById: (id) => els[id] || null,
    querySelector: () => null,
    querySelectorAll: () => [],
    addEventListener() {},
    createElement: () => el('x' + tid++),
    body: { appendChild() {} },
  };
  vm.createContext(sb);
  vm.runInContext(busyCode, sb, { filename: 'ht9045_busy_util.js' });
  vm.runInContext(code, sb, { filename: 'ht9045_main_st01_ev.js' });
  const menu = () => srv.cmds.filter((c) => c.name === 'act.main.menuVisible');
  const val = (c) => { try { return JSON.parse(c.value); } catch (e) { return null; } };
  return { els, srv, Tags, says, sb, menu, val };
}
const V = (s, c) => JSON.stringify({ setup: s, config: c });

(async () => {
  console.log('d015_menu_selftest -- ' + SRC);

  console.log('[0] main.html / ht9xxx.css: the menus are shown / hidden only through the inline style.display (read-only)');
  check(/<div class="submenu" id="palSetup"/.test(mainHtml) && /<div class="submenu" id="palConfig"/.test(mainHtml),
        'main.html has <div class="submenu" id="palSetup"> and id="palConfig"');
  check(mainHtml.indexOf("target.style.display=(target.style.display==='grid')?'none':'grid';") >= 0 &&
        mainHtml.indexOf("ALL_MENUS.forEach(function(m){if(m!==target)m.style.display='none';});") >= 0,
        'toggleMenu() toggles with style.display (grid / none) and hides the others the same way');
  check(mainHtml.indexOf("mi.closest('.submenu').style.display='none';") >= 0 &&
        mainHtml.indexOf("ALL_MENUS.forEach(function(m){m.style.display='none';});") >= 0,
        'a menu item and a click outside close with style.display = none');
  check(!/pal(Setup|Config)\.classList|ALL_MENUS\.forEach\(function\(m\)\{m\.classList/.test(mainHtml) && /\.submenu\{display:none/.test(css),
        'no classList toggling of the two menus; ht9xxx.css .submenu{display:none} is the closed default');
  check(/<script src="ht9045_main_st01_ev\.js"><\/script>/.test(mainHtml) && /id="palMainStatus"/.test(mainHtml) && /id="SitePanel"/.test(mainHtml),
        'main.html loads ht9045_main_st01_ev.js and has #palMainStatus / #SitePanel (the script\'s main-page test)');
  check(mainHtml.indexOf('act.main.menuVisible') < 0, 'main.html itself is not touched (the logic lives in the St01 script)');

  console.log('[1] reporting (golden palSetup->Visible / palConfig->Visible, main.cpp:25965)');
  const P = makePage({});
  const { els, srv } = P;
  await advance(0);
  check(P.menu().length === 1 && P.menu()[0].value === V(false, false), 'page load -> one act.main.menuVisible {setup:false,config:false}');
  check(srv.log.slice(0, 3).join(',') === 'control.acquire,act.main.menuVisible,control.release',
        'token: acquired for the report and given back right after (main.html, like send()) -- ' + srv.log.slice(0, 3).join(','));
  els.palSetup.style.display = 'grid';                              // toggleMenu(palSetup)
  await advance(0);
  check(P.menu().length === 2 && P.menu()[1].value === V(true, false), 'Tools opened -> {setup:true,config:false}');
  els.palSetup.style.display = 'none'; els.palConfig.style.display = 'grid';   // toggleMenu(palConfig) in one click handler
  els.palConfig.style.left = '10px';
  await advance(0);
  check(P.menu().length === 3 && P.menu()[2].value === V(false, true), 'switch Tools -> Config in one click -> ONE report {setup:false,config:true}');
  els.palConfig.style.display = 'none';                             // item click / click outside
  await advance(0);
  check(P.menu().length === 4 && P.menu()[3].value === V(false, false), 'closed -> {setup:false,config:false}');
  els.palConfig.style.display = 'none'; els.palSetup.style.display = 'none'; els.palDebug.style.display = 'grid';
  await advance(5000);
  check(P.menu().length === 4, 'no change (hide again, or only the web-only Debug menu) -> nothing sent; no traffic in 5 s of steady state');
  els.palDebug.style.display = 'none';

  console.log('[2] retries only after a failure; the latest state wins');
  srv.script.push('busy: same command in progress or just done (act.main.menuVisible, 12 ms ago)');
  const t0 = clock;
  els.palSetup.style.display = 'grid';
  await advance(0);
  check(P.menu().length === 5, 'busy: first try refused');
  await advance(399);
  check(P.menu().length === 5, 'busy: not retried before the 400 ms window');
  await advance(100);
  check(P.menu().length === 6 && P.menu()[5].value === V(true, false) && P.menu()[5].at - t0 === 420,
        'busy: retried once at coolMs+20 = 420 ms (got ' + (P.menu()[5] && P.menu()[5].at - t0) + ')');
  srv.script.push('modal-pending: a blocking box is open', 'not-operator', 'ok');
  const t1 = clock;
  els.palSetup.style.display = 'none';
  await advance(0);
  await advance(999);
  const beforeFirstRetry = P.menu().length;
  await advance(1);
  const at1 = P.menu()[P.menu().length - 1].at - t1;
  await advance(2000);
  const at2 = P.menu()[P.menu().length - 1].at - t1;
  check(beforeFirstRetry === 7 && P.menu().length === 9 && at1 === 1000 && at2 === 3000 && P.menu()[8].value === V(false, false),
        'modal-pending / not-operator -> retried after 1 s, then 2 s, delivered the third time (at ' + at1 + ' / ' + at2 + ' ms)');
  await advance(20000);
  check(P.menu().length === 9, 'delivered -> no more retries');
  srv.script.push('control-held', 'ok');
  const t2 = clock;
  els.palConfig.style.display = 'grid';
  await advance(500);
  els.palConfig.style.left = '12px';                                // style writes that do not change open / closed (toggleMenu's left,
  els.palSetup.style.display = 'none';                              //   a click outside hiding an already hidden menu)
  await advance(499);
  const noEarly = P.menu().length === 10;
  await advance(1);
  check(noEarly && P.menu().length === 11 && P.menu()[10].at - t2 === 1000 && P.menu()[10].value === V(false, true),
        'a failure backs off 1 s; style writes that do not open / close a menu do not cut the back-off short');
  els.palConfig.style.display = 'none';
  await advance(0);

  srv.script.push('hold');
  els.palConfig.style.display = 'grid';
  await advance(0);
  const nHold = P.menu().length;
  els.palConfig.style.display = 'none';
  await advance(0);
  check(P.menu().length === nHold && P.menu()[nHold - 1].value === V(false, true), 'in flight: a change is not sent on top (one command at a time)');
  srv.held.res({ ok: true });
  await advance(0);
  check(P.menu().length === nHold + 1 && P.menu()[nHold].value === V(false, false),
        'first ack -> the newer state {false,false} is sent right after (latest wins)');
  srv.script.push('hold');
  els.palSetup.style.display = 'grid';
  await advance(0);
  els.palSetup.style.display = 'none';
  const nHold2 = P.menu().length;
  srv.held.rej(new Error('socket closed before ack'));
  await advance(0);
  check(P.menu().length === nHold2, 'refused in flight but the state went back to the acknowledged one -> nothing to resend');
  await advance(15000);
  check(P.menu().length === nHold2, '... and no retry later either');

  console.log('[3] closing on a level drop to 0 (golden ChangeLevelAttr :12928-12935 + :13182-13190)');
  els.palSetup.style.display = 'grid';
  els.palDebug.style.display = 'grid';
  await advance(0);
  const n3 = P.menu().length;
  P.Tags.emit('auth.level', 2);                                     // first frame: only recorded (golden iOldAccessLevel=-999)
  P.Tags.emit('auth.level', 1);                                     // dropped, but not to 0
  P.Tags.emit('auth.level', null);                                  // not loaded / disconnected: ignored
  await advance(0);
  check(els.palSetup.style.display === 'grid' && P.menu().length === n3, 'first frame 2, then 1, then null -> Tools stays open, nothing sent');
  P.Tags.emit('auth.level', 0);
  await advance(0);
  check(els.palSetup.style.display === 'none' && els.palConfig.style.display === 'none' && els.palDebug.style.display === 'grid',
        '1 -> 0 -> Tools closed (Config stays closed); the web-only Debug menu is left alone');
  check(P.menu().length === n3 + 1 && P.menu()[n3].value === V(false, false) && P.says.some((s) => /選單已收起/.test(s)),
        '... reported {false,false} and said so in the status line');
  els.palConfig.style.display = 'grid';
  await advance(0);
  const n4 = P.menu().length, s4 = P.says.length;
  P.Tags.emit('auth.level', 0);
  await advance(0);
  check(els.palConfig.style.display === 'grid' && P.menu().length === n4 && P.says.length === s4, '0 -> 0 -> nothing closed (no drop)');
  P.Tags.emit('auth.level', 3); P.Tags.emit('auth.level', 0);
  await advance(0);
  check(els.palConfig.style.display === 'none' && P.menu()[P.menu().length - 1].value === V(false, false), '0 -> 3 -> 0 -> Config closed and reported');
  els.palDebug.style.display = 'none';

  const Q = makePage({});
  await advance(0);
  Q.els.palSetup.style.display = 'grid';
  await advance(0);
  Q.Tags.emit('auth.level', 0);
  await advance(0);
  check(Q.els.palSetup.style.display === 'grid', 'first frame already 0 -> not closed (golden static iOldAccessLevel=-999: the first call never closes)');

  console.log('[4] an old wb_serve (unknown cmd) -> stop, say it once');
  const U = makePage({});
  U.srv.script.push("unknown cmd 'act.main.menuVisible' (no branch in tools/wb_serve.cpp's dispatch)");
  await advance(0);
  U.els.palSetup.style.display = 'grid';
  await advance(30000);
  check(U.menu().length === 1 && U.says.filter((s) => /不認得 act\.main\.menuVisible/.test(s)).length === 1,
        'unknown cmd -> no further reports (menus changed, 30 s passed), one status line');

  console.log('[5] other pages that load the same script (no #palMainStatus) send nothing');
  const O = makePage({ main: false });
  await advance(0);
  O.els.palSetup.style.display = 'grid';
  await advance(1000);
  check(O.menu().length === 0, 'not the main page -> no act.main.menuVisible');

  console.log('[6] no MutationObserver (very old browser) -> only the load report, no crash');
  const N = makePage({ noMO: true });
  await advance(0);
  N.els.palSetup.style.display = 'grid';
  await advance(1000);
  check(N.menu().length === 1 && N.menu()[0].value === V(false, false), 'load report only');

  console.log('d015_menu_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
