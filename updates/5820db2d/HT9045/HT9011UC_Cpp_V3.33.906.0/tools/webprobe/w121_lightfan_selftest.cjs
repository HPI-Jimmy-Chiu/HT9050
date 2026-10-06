// w121_lightfan_selftest.cjs -- ctest St02_W121LightFanPage (node, offline): the main page's Light / FAN .bigbtn are wired in
// BOTH page modes.  AI(W906-W121) 20261006 (St02-E): laptop TO_STEVEN s4 18:0x W-121 -- in release mode (main.html's default,
// theme.js:10-11) theme.js stripTitles() (main.html:538, theme.js:15-24) moves every title to data-htitle before
// ht9045_main_st01_ev.js initMain() looks the two buttons up, so a [title="spbLight"] / [title="spbFan"] selector found nothing and
// the buttons never sent act.main.light / act.main.fan.  The fix selects title OR data-htitle (the main.html:352 comp() rule).
// Usage: node w121_lightfan_selftest.cjs <web/page dir>.  Control: W906_MAIN_ST01_EV=<the pre-W-121 file> must make it red.
// Fake DOM rule (St02 workflow skill section 5 item 20): every property the script reads is set when the DOM is built;
// the page pin looks for the full element markup, not a bare name.
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
function check(cond, what) { if (cond) { pass++; console.log('  ok   ' + what); } else { fail++; console.log('  FAIL ' + what); } }
async function settle() { for (let i = 0; i < 12; i++) await new Promise((r) => setImmediate(r)); }

// one page load; mode 'debug' = the buttons keep title, 'release' = theme.js already moved it to data-htitle
function load(mode) {
  const els = {};
  function mk(id, attrs) {
    const e = { listeners: {}, style: {}, attrs: Object.assign({}, attrs || {}), textContent: '', title: '' };
    let id0 = id || '';
    Object.defineProperty(e, 'id', { get() { return id0; }, set(v) { id0 = String(v); if (id0) els[id0] = e; } });
    if (id0) els[id0] = e;
    e.getAttribute = (n) => (n in e.attrs ? e.attrs[n] : null);
    e.setAttribute = (n, v) => { e.attrs[n] = String(v); };
    e.addEventListener = (t, f) => { (e.listeners[t] = e.listeners[t] || []).push(f); };
    e.appendChild = (c) => c;
    e.contains = () => false;
    return e;
  }
  ['palMainStatus', 'SitePanel', 'spbFanCap', 'spbLight'].forEach((id) => mk(id));
  const attr = mode === 'release' ? 'data-htitle' : 'title';
  const bigbtns = [mk('', { class: 'bigbtn', [attr]: 'spbLight' }), mk('', { class: 'bigbtn', [attr]: 'spbFan' })];
  function matches(e, sel) {                                       // the only selector shapes the script uses on .bigbtn
    const m = /^\.bigbtn\[(title|data-htitle)="([^"]+)"\]$/.exec(sel.trim());
    return !!m && e.attrs.class === 'bigbtn' && e.attrs[m[1]] === m[2];
  }
  const cmds = [];
  const R = {
    status() { return { connected: true, holdsToken: true }; },
    keepAlive() { return Promise.resolve(); },
    release() { return Promise.resolve(); },
    rawCmd(name, extra) { cmds.push({ name, value: extra && extra.value }); return Promise.resolve({ type: 'ack', ok: true, executed: true, caption: 'X' }); },
  };
  const sb = {
    console: { info() {}, warn() {}, error() {}, log() {} },
    setTimeout, clearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    HT9045Recipe: R, HT9045Tags: { on() { return () => {}; } }, HT9045Wire: { say() {} },
    getComputedStyle() { return { display: '' }; },
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
  };
  sb.window = sb;
  sb.document = {
    readyState: 'complete',
    getElementById: (id) => els[id] || null,
    querySelector: (sel) => { for (const part of String(sel).split(',')) { const hit = bigbtns.find((e) => matches(e, part)); if (hit) return hit; } return null; },
    querySelectorAll: () => [],
    addEventListener() {},
    createElement: () => mk(''),
    body: { appendChild() {} },
  };
  vm.createContext(sb);
  vm.runInContext(busyCode, sb, { filename: 'ht9045_busy_util.js' });
  vm.runInContext(code, sb, { filename: path.basename(SRC) });
  return { light: bigbtns[0], fan: bigbtns[1], cmds };
}

(async () => {
  for (const mode of ['debug', 'release']) {
    console.log('-- ' + mode + ' (the buttons carry ' + (mode === 'release' ? 'data-htitle' : 'title') + ')');
    const p = load(mode);
    check((p.light.listeners.click || []).length === 1, mode + ': Light has its click handler');
    check((p.fan.listeners.click || []).length === 1, mode + ': FAN has its click handler');
    if ((p.light.listeners.click || []).length) { p.light.listeners.click[0](); await settle(); }
    if ((p.fan.listeners.click || []).length) { p.fan.listeners.click[0](); await settle(); }
    const names = p.cmds.map((c) => c.name);
    check(names.indexOf('act.main.light') >= 0, mode + ': a Light click sends act.main.light (golden spbLightClick main.cpp:26780)');
    check(names.indexOf('act.main.fan') >= 0, mode + ': a FAN click sends act.main.fan (golden spbFanClick main.cpp:26763)');
  }
  console.log('-- page pins');
  check(/<div class="bigbtn" title="spbLight">/.test(mainHtml) && /<div class="bigbtn" title="spbFan">/.test(mainHtml),
        'main.html has the two .bigbtn with title spbLight / spbFan (what theme.js turns into data-htitle in release)');
  check(/<script src="theme\.js"><\/script>/.test(mainHtml) && mainHtml.indexOf('<script src="theme.js"></script>') < mainHtml.indexOf('<script src="ht9045_main_st01_ev.js"></script>'),
        'main.html loads theme.js before ht9045_main_st01_ev.js (so release strips the titles first)');
  console.log('St02_W121LightFanPage: ' + pass + ' ok, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})();
