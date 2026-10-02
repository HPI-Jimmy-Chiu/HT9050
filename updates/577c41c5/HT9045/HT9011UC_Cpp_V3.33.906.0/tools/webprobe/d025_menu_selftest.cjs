// AI(W906-D025) 20261001 [W906] St01: offline selftest for todo D-025, the page half (ctest D025_MenuOpenPage).
//   golden V912 main.cpp:29030-29049 sbSettingClick / :29009-29028 sbConfigClick: if(SystemStart) return; -> Insufficient(0|1) ->
//   ProceeToolBar + palX->Visible=true -> (Config) SetWorkParameter / UpdateMainOperateMode -> SECS EnterTool / EnterConfig. No toggle:
//   a second click reruns the body and the menu stays open.
//   web/page/ht9045_main_st01_ev.js initMenuOpen catches a click on #sbSetting / #sbConfig in the WINDOW CAPTURE phase, before main.html's
//   own listener on the button, sends WS act.main.menuOpen {"menu":"setup"|"config"} (C++ FileRW/Main_D025MenuOpen.cpp) and only on
//   allow lets main.html's original handler run (toggleMenu); already open -> left open; deny -> nothing, a status line.
//   The D-015 A01b MutationObserver (act.main.menuVisible) must keep reporting what is visible.
//   Loads, into ONE node vm, main.html's REAL menu block (toggleMenu + the three button listeners + the click-outside closer, cut out of
//   main.html by exact text -- read only), then the REAL ht9045_busy_util.js and ht9045_main_st01_ev.js, with a fake DOM that dispatches
//   click events through capture / target / bubble, a fake HT9045Recipe (scripted acks), fake HT9045Tags and a fake clock.
//   No socket, no wb_serve, no file written.  Usage: node tools/webprobe/d025_menu_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_MAIN_ST01_EV=<the pre-D-025 ht9045_main_st01_ev.js>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_MAIN_ST01_EV || path.join(PAGE, 'ht9045_main_st01_ev.js');
const code = fs.readFileSync(SRC, 'utf8');
const busyCode = fs.readFileSync(path.join(PAGE, 'ht9045_busy_util.js'), 'utf8');
const mainHtml = fs.readFileSync(path.join(PAGE, 'main.html'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- main.html's menu code, cut out by exact text (read only)
const BLOCK_START = '// Tools/Config/Debug 子選單：互斥顯示';
const BLOCK_END = "document.getElementById('sbDebug').addEventListener('click',function(e){\n  e.stopPropagation(); toggleMenu(palDebug,this);\n});";
const OUTSIDE = "document.addEventListener('click',function(){\n  ALL_MENUS.forEach(function(m){m.style.display='none';});\n});";
const html = mainHtml.replace(/\r\n/g, '\n');
const i0 = html.indexOf(BLOCK_START), i1 = html.indexOf(BLOCK_END);
const menuBlock = (i0 >= 0 && i1 > i0) ? html.slice(i0, i1 + BLOCK_END.length) : '';
const outsideBlock = html.indexOf(OUTSIDE) >= 0 ? OUTSIDE : '';

// ---------------------------------------------------------------- fake clock (same as d015_menu_selftest.cjs)
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

// ---------------------------------------------------------------- fake DOM with capture / target / bubble
function listeners() { return { cap: {}, bub: {} }; }
function addL(node, t, f, opt) { const ph = (opt === true || (opt && opt.capture)) ? 'cap' : 'bub'; (node.__l[ph][t] = node.__l[ph][t] || []).push(f); }
let WIN = null, DOC = null;
function dispatch(target, type, trusted) {
  const ev = {
    type, target, isTrusted: !!trusted, _stop: false, _imm: false, defaultPrevented: false,
    stopPropagation() { this._stop = true; }, stopImmediatePropagation() { this._stop = true; this._imm = true; }, preventDefault() { this.defaultPrevented = true; },
  };
  const chain = [];
  for (let n = target; n && n !== DOC; n = n.parentNode) chain.unshift(n);
  const path = [WIN, DOC].concat(chain);
  function run(n, ph) { for (const f of (n.__l[ph][type] || []).slice()) { f.call(n, ev); if (ev._imm) return; } }
  for (const n of path.slice(0, -1)) { run(n, 'cap'); if (ev._stop) return ev; }
  run(target, 'cap'); if (ev._stop) return ev;
  run(target, 'bub'); if (ev._stop) return ev;
  for (const n of path.slice(0, -1).reverse()) { run(n, 'bub'); if (ev._stop) return ev; }
  return ev;
}
const byId = {};
function el(id, parent) {
  const e = { id, tagName: 'DIV', __l: listeners(), __mos: [], parentNode: parent || null, offsetLeft: 100, offsetWidth: 300, children: [] };
  e.style = {};
  ['display', 'left'].forEach((prop) => {
    let v0 = '';
    Object.defineProperty(e.style, prop, { enumerable: true, get() { return v0; }, set(v) { v0 = String(v); e.__mos.forEach((mo) => mo.__notify(e)); } });
  });
  e.addEventListener = (t, f, o) => addL(e, t, f, o);
  e.contains = () => false;
  e.click = () => dispatch(e, 'click', false);             // HTMLElement.click(): a synthetic (untrusted) click through the whole path
  e.setAttribute = () => {}; e.getAttribute = () => null;
  if (parent) parent.children.push(e);
  if (id) byId[id] = e;
  return e;
}

function makePage() {
  Object.keys(byId).forEach((k) => delete byId[k]);
  DOC = { __l: listeners(), readyState: 'complete', documentElement: { clientWidth: 1280 } };
  const body = el('', null); body.parentNode = DOC;
  const bar = el('', body);
  const sbSetting = el('sbSetting', bar), sbConfig = el('sbConfig', bar), sbDebug = el('sbDebug', bar);
  const icSetting = el('', sbSetting);                     // <span class="ic"> inside the button (a real click lands here)
  ['palSetup', 'palConfig', 'palDebug', 'palMainStatus', 'SitePanel'].forEach((id) => el(id, body));
  DOC.getElementById = (id) => byId[id] || null;
  DOC.querySelector = () => null;
  DOC.querySelectorAll = () => [];
  DOC.addEventListener = (t, f, o) => addL(DOC, t, f, o);
  DOC.createElement = () => el('', null);
  DOC.body = body;
  body.appendChild = () => {};
  class FakeMO {
    constructor(cb) { this.cb = cb; this.q = false; }
    observe(target, o) { this.opts = o; target.__mos.push(this); }
    disconnect() {}
    __notify(target) {
      if (!this.opts || !this.opts.attributes || this.q) return;
      this.q = true;
      Promise.resolve().then(() => { this.q = false; this.cb([{ type: 'attributes', attributeName: 'style', target }], this); });
    }
  }
  const srv = { cmds: [], log: [], holds: true, script: [], held: null };
  const R = {
    status() { return { connected: true, holdsToken: srv.holds }; },
    keepAlive() { srv.log.push('control.acquire'); srv.holds = true; return Promise.resolve(); },
    release() { srv.log.push('control.release'); srv.holds = false; return Promise.resolve(); },
    rawCmd(name, extra) {
      srv.cmds.push({ name, value: extra && extra.value, at: clock });
      srv.log.push(name);
      if (name !== 'act.main.menuOpen') return Promise.resolve({ type: 'ack', ok: true, executed: true });
      const next = srv.script.length ? srv.script.shift() : 'allow';
      const menu = (() => { try { return JSON.parse(extra.value).menu; } catch (e) { return '?'; } })();
      if (next === 'allow') return Promise.resolve({ type: 'ack', ok: true, executed: true, allow: true, menu, ran: [], golden: 'golden V912 main.cpp' });
      if (next === 'hold') return new Promise((res, rej) => { srv.held = { res: () => res({ type: 'ack', ok: true, allow: true, menu }), rej }; });
      return Promise.reject(new Error(next));
    },
  };
  const says = [];
  WIN = {
    __l: listeners(),
    console: { info() {}, warn() {}, error(...a) { console.error(...a); }, log() {} },
    setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    MutationObserver: FakeMO,
    HT9045Recipe: R,
    HT9045Tags: { on() { return () => {}; } },
    HT9045Wire: { say(msg) { says.push(msg); } },
    getComputedStyle(e) { return { display: e.style.display || 'none' }; },   // ht9xxx.css .submenu{display:none}
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
    document: DOC,
  };
  WIN.window = WIN;
  WIN.addEventListener = (t, f, o) => addL(WIN, t, f, o);
  vm.createContext(WIN);
  vm.runInContext(menuBlock + '\n' + outsideBlock, WIN, { filename: 'main.html (menu block)' });
  let toggles = 0;
  const realToggle = WIN.toggleMenu;
  WIN.toggleMenu = function (t, a) { toggles++; return realToggle(t, a); };   // the button listeners look toggleMenu up by name at call time
  vm.runInContext(busyCode, WIN, { filename: 'ht9045_busy_util.js' });
  vm.runInContext(code, WIN, { filename: 'ht9045_main_st01_ev.js' });
  const open = () => srv.cmds.filter((c) => c.name === 'act.main.menuOpen');
  const vis = () => srv.cmds.filter((c) => c.name === 'act.main.menuVisible');
  return { srv, says, WIN, open, vis, toggles: () => toggles, sbSetting, sbConfig, sbDebug, icSetting,
           pal: { setup: byId.palSetup, config: byId.palConfig, debug: byId.palDebug }, body };
}
const V = (s, c) => JSON.stringify({ setup: s, config: c });
const shown = (e) => e.style.display === 'grid';

(async () => {
  console.log('d025_menu_selftest -- ' + SRC);

  console.log('[0] main.html (read-only): the menu block the interception is built on');
  check(!!menuBlock && menuBlock.indexOf("document.getElementById('sbSetting').addEventListener('click',function(e){\n  e.stopPropagation(); toggleMenu(palSetup,this);\n});") >= 0 &&
        menuBlock.indexOf("document.getElementById('sbConfig').addEventListener('click',function(e){\n  e.stopPropagation(); toggleMenu(palConfig,this);\n});") >= 0,
        'main.html: #sbSetting / #sbConfig click listeners (bubble, on the button) call toggleMenu');
  check(!!outsideBlock, 'main.html: the click-outside closer on document');
  check(html.indexOf('<script src="ht9045_main_st01_ev.js"></script>') > i1, 'ht9045_main_st01_ev.js loads after the inline menu script');
  check(html.indexOf('act.main.menuOpen') < 0, 'main.html itself is not touched');

  console.log('[1] allow -> main.html\'s original handler opens the menu; D-015 A01b reports it');
  let P = makePage();
  await advance(0);
  const vis0 = P.vis().length;
  check(vis0 === 1 && P.vis()[0].value === V(false, false), 'load: D-015 A01b report {false,false} (unchanged)');
  P.srv.script.push('hold');                               // keep the answer back to look at the moment in between
  dispatch(P.icSetting, 'click', true);                    // a real click on the icon inside Tools
  await advance(0);
  check(P.open().length === 1 && P.open()[0].value === JSON.stringify({ menu: 'setup' }) && !shown(P.pal.setup) && P.toggles() === 0,
        'click -> act.main.menuOpen {"menu":"setup"} sent, main.html handler NOT run yet, menu still closed');
  P.srv.held.res();
  await advance(0);
  check(shown(P.pal.setup) && P.toggles() === 1, 'allow -> main.html handler ran toggleMenu once -> palSetup open');
  check(P.vis().length === 2 && P.vis()[1].value === V(true, false) && P.srv.cmds.findIndex((c) => c.name === 'act.main.menuVisible' && c.value === V(true, false)) >
        P.srv.cmds.findIndex((c) => c.name === 'act.main.menuOpen'), 'the MutationObserver then reports {setup:true,config:false} (after menuOpen)');

  console.log('[2] second click while open: golden reruns the body, the menu stays open');
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(P.open().length === 2 && shown(P.pal.setup) && P.toggles() === 1 && P.vis().length === 2,
        'menuOpen sent again, toggleMenu NOT called (it would close it), still open, no extra report');

  console.log('[3] Tools open -> click Config');
  await advance(500);
  dispatch(P.sbConfig, 'click', true);
  await advance(0);
  check(P.open().length === 3 && P.open()[2].value === JSON.stringify({ menu: 'config' }) && shown(P.pal.config) && !shown(P.pal.setup) && P.toggles() === 2,
        'allow -> toggleMenu(palConfig): Config open, Tools closed (golden ProceeToolBar :25093-25094)');
  check(P.vis().length === 3 && P.vis()[2].value === V(false, true), 'one report {false,true}');

  console.log('[4] deny -> nothing opens, the reason on the status line');
  dispatch(P.body, 'click', true);                          // click outside: main.html closes all (unchanged)
  await advance(0);
  check(!shown(P.pal.config) && P.vis().length === 4 && P.vis()[3].value === V(false, false), 'click outside still closes (main.html), reported {false,false}');
  P.srv.script.push(JSON.stringify({ executed: false, allow: false, menu: 'setup', guard: 'running', detail: '機台運轉中（SystemStart）不能開工具選單',
                                     golden: 'golden V912 main.cpp:29033-29034 sbSettingClick if(SystemStart) return;' }));
  const says0 = P.says.length;
  await advance(500);
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(P.open().length === 4 && !shown(P.pal.setup) && P.toggles() === 2 && P.vis().length === 4, 'denied: no toggleMenu, menu closed, no report');
  check(P.says.length > says0 && /Tools/.test(P.says[P.says.length - 1]) && /running/.test(P.says[P.says.length - 1]) && /29033/.test(P.says[P.says.length - 1]),
        'status line: Tools + running + the golden line (' + (P.says[P.says.length - 1] || '').slice(0, 60) + ')');
  P.srv.script.push(JSON.stringify({ executed: false, allow: false, menu: 'config', guard: 'not-authorized', detail: '等級不足', golden: 'golden V912 main.cpp:29015' }));
  await advance(500);
  dispatch(P.sbConfig, 'click', true);
  await advance(0);
  check(!shown(P.pal.config) && P.toggles() === 2 && /not-authorized/.test(P.says[P.says.length - 1]), 'Config not-authorized: not opened');

  console.log('[5] busy: and in flight');
  P.srv.script.push('busy: same command in progress or just done (act.main.menuOpen, 12 ms ago)');
  await advance(500);
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(!shown(P.pal.setup) && P.toggles() === 2 && P.says[P.says.length - 1].indexOf(P.WIN.HT9045Busy.NOTE) >= 0, 'busy: not opened, busy note shown');
  P.srv.script.push('hold');
  await advance(500);
  const n5 = P.open().length;
  dispatch(P.sbSetting, 'click', true);
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(P.open().length === n5 + 1 && /略過/.test(P.says[P.says.length - 1]), 'second click while the first is in flight: dropped (one request)');
  P.srv.held.res();
  await advance(0);
  check(shown(P.pal.setup) && P.toggles() === 3, 'the held allow arrives -> opened once');

  console.log('[6] token: taken for the click, given back');
  dispatch(P.body, 'click', true);
  await advance(500);
  P.srv.holds = false; P.srv.log.length = 0;
  dispatch(P.sbConfig, 'click', true);
  await advance(0);
  check(P.srv.log.slice(0, 3).join(',') === 'control.acquire,act.main.menuOpen,control.release', 'acquire, menuOpen, release (' + P.srv.log.slice(0, 3).join(',') + ')');
  check(shown(P.pal.config), 'and it opened');

  console.log('[7] Debug ▾ (web-only) is not intercepted');
  const n7 = P.open().length;
  dispatch(P.sbDebug, 'click', true);
  await advance(0);
  check(P.open().length === n7 && shown(P.pal.debug), 'Debug opens as before, nothing sent');

  console.log('[8] old wb_serve (unknown cmd): as before');
  P = makePage();
  await advance(0);
  P.srv.script.push('unknown cmd: act.main.menuOpen');
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(shown(P.pal.setup) && P.toggles() === 1 && /舊版/.test(P.says[P.says.length - 1]), 'unknown cmd -> opened the old way, said once');
  dispatch(P.body, 'click', true);
  await advance(500);
  dispatch(P.sbSetting, 'click', true);
  await advance(0);
  check(P.open().length === 1 && shown(P.pal.setup) && P.toggles() === 2, 'after that: opened directly, not asked again');

  console.log('d025_menu_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
