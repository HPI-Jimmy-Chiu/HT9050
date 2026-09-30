// AI(W906-STREAM-S2E) 20260930: 第 2 階段 E 的離線自我測試 —— IO 頁與 Teach 頁「視窗關著不拿 runtime JSON」。
//   golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\iosetview.cpp:162（Tfiosetview::Timer1Timer）與
//   uteach.cpp:1366（TfTeach::Timer1Timer）第一件事都是 `if(fShow==false) return;`；網頁那一側看外框送的 HT_WIN
//   （D:\HT9045\web\background.html:682，縮小送 open:true），寫法照 HW.MotorTest.html:848-851、Main.MotorView.html:253-256。
//   設計：D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\stream-by-open-page.md §4.5。
//
//   做法：從兩個頁面「切出真的那一段」（標記找不到就紅），放進 node vm，假的 window／HT_WIN／setInterval／fetch：
//     IO 頁：`var ioLive=` 到 `function ioLoadAndBind(){` 之前（ioStartRuntimePoll 與新的 HT_WIN 聽者）。
//     Teach 頁：`var teachMotorConfig=null` 到第一個頂層 `teachLoadMotors();`（含 teachSyncHome、teachLoadMotors 的 bind）。
//   驗：關著不拿／開窗立刻拿一次再照拍拿／縮小照拿／從沒收過 HT_WIN 照拿／不是外框送的 HT_WIN 不算／
//       Teach 的 HOME 彈起（teachSyncHome）跟著同一道閘；Teach 關窗那一下照 golden FormClose → AllBtnUp（uteach.cpp:2092、
//       :5346-5358）當場把 #btnHome 彈起、holdToken()（從頁面切出來的那一支）不再成立，縮小不彈、單獨開頁不受影響；
//       另外每一段頂層 <script> 都要編得過、兩頁不混 EOL、無 BOM。
//   對照組：W906_S2E_PAGE_DIR 指向修改前的兩頁（git show HEAD:web/page/...）必須紅。
//   不連 wb_serve、不讀寫機台檔（只讀 web\page 兩個 .html）。用法：只經 ctest（Stream2E_PagePolls）。
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_S2E_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const IO_PAGE = path.join(PAGE_DIR, 'HW.IoSetView.html');
const TEACH_PAGE = path.join(PAGE_DIR, 'HW.teach.html');

let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function eq(a, b, what) { if (a !== b) throw new Error(what + ': expected ' + JSON.stringify(b) + ', got ' + JSON.stringify(a)); }
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async () => { for (let i = 0; i < 4; i++) await new Promise(r => setImmediate(r)); };

function readPage(p) {
  const b = fs.readFileSync(p);
  return { bytes: b, text: b.toString('utf8') };
}
// 標記之間的原文（CR 先拿掉，標記裡不含換行）
function region(text, startMarker, endMarker, includeEnd) {
  const t = text.replace(/\r/g, '');
  const s = t.indexOf(startMarker);
  if (s < 0) throw new Error('start marker not found: ' + startMarker);
  const e = t.indexOf(endMarker, s);
  if (e < 0) throw new Error('end marker not found: ' + endMarker);
  return t.slice(s, includeEnd ? e + endMarker.length : e);
}

// ---- 假的 window（iframe 裡的頁面）----------------------------------------------------
function makeEnv(hosted) {
  const timers = [];
  const listeners = {};
  const els = {};
  function el(id) {
    if (!els[id]) {
      const cls = new Set();
      els[id] = {
        id, value: '', textContent: '', style: {},
        classList: { contains: c => cls.has(c), add: c => cls.add(c), remove: c => cls.delete(c), toggle: (c, f) => { const on = (f === undefined) ? !cls.has(c) : !!f; if (on) cls.add(c); else cls.delete(c); return on; } },   // AI(W906-STREAM-2E-MERGE) 20260930: + toggle -- machine web 0019 TEACH-LED (batch 6) made teachSelectMotor call classList.toggle
        setAttribute() {}, getAttribute() { return ''; }, addEventListener() {},
      };
    }
    return els[id];
  }
  const ctx = {
    console, Date, Math, Promise, JSON, Error, String, Number, Object, Array, RegExp, parseInt, parseFloat, isNaN,
    location: { protocol: 'http:' },
    document: {
      hidden: false,
      getElementById: el,
      querySelector: sel => {   // 只認 '#id' 與 '#id.cls'（頁面的 holdToken 用 '#btnHome.down'）
        const m = /^#([A-Za-z0-9_]+)(?:\.([A-Za-z0-9_-]+))?$/.exec(sel);
        if (!m || !els[m[1]]) return null;
        return (!m[2] || els[m[1]].classList.contains(m[2])) ? els[m[1]] : null;
      },
      querySelectorAll: () => [],
      body: { setAttribute() {} },
    },
    setInterval(fn, ms) { timers.push({ fn, ms, cleared: false }); return timers.length; },
    clearInterval(id) { if (timers[id - 1]) timers[id - 1].cleared = true; },
    setTimeout() { return 0; },
    addEventListener(type, fn) { (listeners[type] = listeners[type] || []).push(fn); },
  };
  ctx.window = ctx;
  ctx.parent = hosted ? { frame: 'background.html' } : ctx;   // 單獨開頁：window.parent===window
  vm.createContext(ctx);
  return {
    ctx, el,
    run(code, file) { return vm.runInContext(code, ctx, { filename: file }); },
    live() { return timers.filter(t => !t.cleared); },
    tick() { const l = timers.filter(t => !t.cleared); if (l.length !== 1) throw new Error('expected 1 live timer, got ' + l.length); l[0].fn(); },
    htWin(open, state, source) {
      const ev = { data: { type: 'HT_WIN', id: 'x', open: !!open, state: state || (open ? 'open' : 'closed'), initial: false },
                   source: source === undefined ? ctx.parent : source };
      (listeners.message || []).forEach(fn => fn(ev));
    },
  };
}

// ---- IO 頁 ------------------------------------------------------------------------
const IO_STUBS = [
  'var __ioFetches=[];',
  'function ioLoadJsonNet(url){ __ioFetches.push(url); return Promise.resolve({runtime:{}, points:[]}); }',
  'function ioMergeConfigRuntime(cfg, rt){ return {cfg:cfg, rt:rt}; }',
  'function ioBindStatus(d){}',
  'function ioSetInfo(){}',
].join('\n');

function ioEnv(pageText, hosted) {
  const env = makeEnv(hosted);
  env.run(IO_STUBS, 'io-stubs.js');
  env.run(region(pageText, 'var ioLive=', 'function ioLoadAndBind(){', false), 'HW.IoSetView.html#ioLive..ioStartRuntimePoll');
  env.fetches = () => env.run('__ioFetches.filter(function(u){ return u.indexOf("runtime")>=0; }).length');
  env.start = () => env.run('ioStartRuntimePoll({points:[]}, 200)');
  return env;
}

// ---- Teach 頁 ---------------------------------------------------------------------
const TEACH_STUBS = [
  'var __teachFetches=[]; var __homeJob=true;',
  'function __rt(){ return {motors:[{motorId:"MInArmX", position:{cmdPos:11, encPos:12}, motion:{homeJob:__homeJob}}]}; }',
  'function teachSetInfo(){}',
  'function teachOfflineEnabled(){ return false; }',
  'function teachInitAccess(){}',
  'function teachLoadJsonNet(url, n){ __teachFetches.push(url); return Promise.resolve(url.indexOf("config")>=0 ? {motors:[{motorId:"MInArmX", motorIndex:0, alias:"InArmX"}]} : __rt()); }',
  'function teachLoadJson(url){ return Promise.reject(new Error("no static file in the test")); }',
].join('\n');

async function teachEnv(pageText, hosted, beforeLoad) {
  const env = makeEnv(hosted);
  env.run(TEACH_STUBS, 'teach-stubs.js');
  env.fetches = () => env.run('__teachFetches.filter(function(u){ return u.indexOf("runtime")>=0; }).length');
  // 頂層的 teachLoadMotors(); 一起跑（= 頁面載入）；beforeLoad：HT_WIN initial 比載入完成早到
  env.run(region(pageText, 'var teachMotorConfig=null', '\nteachLoadMotors();', true), 'HW.teach.html#teachMotorConfig..teachLoadMotors()');
  if (beforeLoad) beforeLoad(env);
  await flush();
  if (env.live().length !== 1) throw new Error('page load did not start the 1 s motor timer');
  eq(env.live()[0].ms, 1000, 'Teach poll interval');
  env.homeDown = () => env.el('btnHome').classList.contains('down');
  env.pressHome = () => { env.el('btnHome').classList.add('down'); env.run('teachHomeMotor="MInArmX"; teachHomeGone=0;'); };
  // 頁面自己的 holdToken（teachInitAccess 裡 HTMotorAccess.init 的那一支，照原文切出來）
  const hm = /holdToken:\s*(function\s*\(\)\s*\{[^{}]*\})/.exec(pageText.replace(/\r/g, ''));
  if (!hm) throw new Error('holdToken:function(){...} not found in HW.teach.html');
  env.run('var __holdToken=' + hm[1] + ';', 'HW.teach.html#holdToken');
  env.holdToken = () => env.run('__holdToken()');
  return env;
}

(async function main() {
  console.log('pages: ' + PAGE_DIR);
  const io = readPage(IO_PAGE);
  const te = readPage(TEACH_PAGE);

  // ---- 0. 兩頁本身 ----
  for (const [name, pg] of [['HW.IoSetView.html', io], ['HW.teach.html', te]]) {
    await check(name + ': every inline <script> compiles', () => {
      const re = /<script(?![^>]*\bsrc=)([^>]*)>([\s\S]*?)<\/script>/gi;
      let m, n = 0;
      while ((m = re.exec(pg.text))) {
        if (/type\s*=\s*["']?(?!text\/javascript|module)[a-z]/i.test(m[1])) continue;   // JSON 資料塊不算
        new vm.Script(m[2], { filename: name + '#script' + (++n) });
      }
      ok(n > 0, 'no inline script found');
    });
    await check(name + ': one EOL style (no mixed CRLF/LF), no BOM', () => {   // index is LF, a core.autocrlf=true checkout is CRLF: both fine
      const b = pg.bytes;
      ok(!(b[0] === 0xef && b[1] === 0xbb && b[2] === 0xbf), 'file starts with a UTF-8 BOM');
      const lf = pg.text.split('\n').length - 1, crlf = pg.text.split('\r\n').length - 1;
      ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' lines');
    });
  }

  // ---- 1. IO 頁 ----
  await check('IO: hosted, window closed -> no runtime fetch', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(false, 'closed');
    for (let i = 0; i < 5; i++) { e.tick(); await flush(); }
    eq(e.fetches(), 0, 'fetches while closed');
  });
  await check('IO: open -> one immediate fetch, then every tick', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(false, 'closed');
    e.tick(); await flush();
    eq(e.fetches(), 0, 'fetches while closed');
    e.htWin(true, 'open');
    eq(e.fetches(), 1, 'immediate fetch when the window opens (no tick yet)');
    await flush();
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), 4, 'fetches after 3 ticks while open');
    eq(e.live()[0].ms, 200, 'IO poll interval (C++ reports 200)');
  });
  await check('IO: minimized counts as open (keeps polling, no extra fetch on open->minimized)', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(true, 'open'); await flush();
    const n0 = e.fetches();
    e.htWin(true, 'minimized');
    eq(e.fetches(), n0, 'open -> minimized is not an opening edge');
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0 + 3, 'fetches while minimized');
  });
  await check('IO: open -> closed stops polling again', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(true, 'open'); await flush();
    e.tick(); await flush();
    const n0 = e.fetches();
    e.htWin(false, 'closed');
    for (let i = 0; i < 4; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0, 'fetches after close');
  });
  await check('IO: never received HT_WIN (standalone page) -> polls as today', async () => {
    const e = ioEnv(io.text, false);
    e.start();
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), 3, 'fetches without HT_WIN');
  });
  await check('IO: hosted but no HT_WIN yet (old frame) -> polls as today', async () => {
    const e = ioEnv(io.text, true);
    e.start();
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), 3, 'fetches without HT_WIN');
  });
  await check('IO: an HT_WIN that is not from the frame is ignored', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(false, 'closed', { someone: 'else' });
    for (let i = 0; i < 2; i++) { e.tick(); await flush(); }
    eq(e.fetches(), 2, 'fetches after a foreign HT_WIN');
  });
  await check('IO: window opens before the poll exists (page still loading) -> no stray fetch, then polls', async () => {
    const e = ioEnv(io.text, true);
    e.htWin(true, 'open');
    eq(e.fetches(), 0, 'no fetch before ioStartRuntimePoll');
    e.start(); e.tick(); await flush();
    eq(e.fetches(), 1, 'first tick fetches');
  });
  await check('IO: document.hidden still pauses (existing rule kept)', async () => {
    const e = ioEnv(io.text, true);
    e.start(); e.htWin(true, 'open'); await flush();
    const n0 = e.fetches();
    e.ctx.document.hidden = true;
    for (let i = 0; i < 2; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0, 'fetches while the browser tab is hidden');
  });

  // ---- 2. Teach 頁 ----
  await check('Teach: hosted, window closed -> no runtime fetch', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(false, 'closed'));
    const n0 = e.fetches();   // 開站那一次（golden 建構／FormShow 之前的載入）
    for (let i = 0; i < 4; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0, 'fetches while closed');
  });
  await check('Teach: open -> one immediate fetch, then every tick', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(false, 'closed'));
    const n0 = e.fetches();
    e.htWin(true, 'open');
    eq(e.fetches(), n0 + 1, 'immediate fetch when the window opens');
    await flush();
    for (let i = 0; i < 2; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0 + 3, 'fetches after 2 ticks while open');
  });
  await check('Teach: minimized counts as open', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(true, 'open'));
    const n0 = e.fetches();
    e.htWin(true, 'minimized');
    eq(e.fetches(), n0, 'open -> minimized is not an opening edge');
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0 + 3, 'fetches while minimized');
  });
  await check('Teach: never received HT_WIN (standalone page) -> polls as today', async () => {
    const e = await teachEnv(te.text, false);
    const n0 = e.fetches();
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0 + 3, 'fetches without HT_WIN');
  });
  await check('Teach: an HT_WIN that is not from the frame is ignored', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(false, 'closed', { someone: 'else' }));
    const n0 = e.fetches();
    for (let i = 0; i < 2; i++) { e.tick(); await flush(); }
    eq(e.fetches(), n0 + 2, 'fetches after a foreign HT_WIN');
  });
  await check('Teach: HOME pop-up (teachSyncHome) follows the same gate -- closed: stays down; open: pops after 2 reads', async () => {
    // 關著的時候按鈕是下去的（例：關窗之後才回來的 home ack 把它設回 down），只有計時器那道閘在管
    const e = await teachEnv(te.text, true, env => env.htWin(false, 'closed'));
    e.pressHome(); e.run('__homeJob=false;');
    for (let i = 0; i < 4; i++) { e.tick(); await flush(); }
    ok(e.homeDown(), 'HOME popped up while the window was closed (golden Timer1Timer returns first)');
    e.htWin(true, 'open'); await flush();          // 開窗立刻拿一次 = 第 1 次讀到 homeJob false
    ok(e.homeDown(), 'HOME popped after only one read');
    e.tick(); await flush();                        // 第 2 次
    ok(!e.homeDown(), 'HOME did not pop after the window opened and two reads saw homeJob=false');
  });
  await check('Teach: HOME pop-up still works while open (control)', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(true, 'open'));
    e.pressHome(); e.run('__homeJob=false;');
    e.tick(); await flush();
    ok(e.homeDown(), 'popped after one read');
    e.tick(); await flush();
    ok(!e.homeDown(), 'did not pop after two reads');
  });
  await check('Teach: HOME pop-up on a standalone page (no HT_WIN) works as today', async () => {
    const e = await teachEnv(te.text, false);
    e.pressHome(); e.run('__homeJob=false;');
    e.tick(); await flush(); e.tick(); await flush();
    ok(!e.homeDown(), 'did not pop after two reads');
  });
  await check('Teach: close pops HOME at once, no fetch needed (golden FormClose :2092 -> AllBtnUp :5346-5358)', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(true, 'open'));
    e.pressHome(); e.run('__homeJob=true;');       // HOME 還在跑：teachSyncHome 自己不會彈
    ok(e.homeDown(), 'precondition: HOME is down');
    eq(e.holdToken(), true, 'precondition: holdToken() while HOME is down');
    const n0 = e.fetches();
    e.htWin(false, 'closed');
    ok(!e.homeDown(), 'HOME still down right after the close edge');
    eq(e.holdToken(), false, 'holdToken() after the close edge');
    eq(e.fetches(), n0, 'the close edge must not fetch');
    eq(e.run('teachHomeGone'), 0, 'teachHomeGone reset');
  });
  await check('Teach: minimized (open:true) does NOT pop HOME', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(true, 'open'));
    e.pressHome(); e.run('__homeJob=true;');
    e.htWin(true, 'minimized');
    ok(e.homeDown(), 'HOME popped on minimize');
    eq(e.holdToken(), true, 'holdToken() while minimized with HOME down');
    e.tick(); await flush();
    ok(e.homeDown(), 'HOME popped while minimized and homeJob still true');
  });
  await check('Teach: only the open->closed edge pops (a closed->closed repeat or the first closed message does not)', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(false, 'closed'));
    e.pressHome(); e.run('__homeJob=true;');
    e.htWin(false, 'closed');
    ok(e.homeDown(), 'HOME popped without an open->closed edge');
  });
  await check('Teach: standalone page (no HT_WIN) -- nothing pops HOME but homeJob', async () => {
    const e = await teachEnv(te.text, false);
    e.pressHome(); e.run('__homeJob=true;');
    e.htWin(false, 'closed', e.ctx);                // window.parent===window：頁面照規則不理
    for (let i = 0; i < 3; i++) { e.tick(); await flush(); }
    ok(e.homeDown(), 'HOME popped on a standalone page while homeJob is still true');
    eq(e.holdToken(), true, 'holdToken() on a standalone page with HOME down');
  });
  await check('Teach: an HT_WIN close that is not from the frame does not pop HOME', async () => {
    const e = await teachEnv(te.text, true, env => env.htWin(true, 'open'));
    e.pressHome(); e.run('__homeJob=true;');
    e.htWin(false, 'closed', { someone: 'else' });
    ok(e.homeDown(), 'HOME popped by a foreign HT_WIN');
  });
  await check('Teach: window opens before load finished -> no stray fetch from the listener', async () => {
    let before = -1, after = -1;
    await teachEnv(te.text, true, env => { before = env.fetches(); env.htWin(true, 'open'); after = env.fetches(); });
    eq(after, before, 'HT_WIN open before the timer exists must not fetch');
  });

  console.log(passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})().catch(e => { console.log('FAIL (harness) ' + (e && e.stack || e)); process.exit(1); });
