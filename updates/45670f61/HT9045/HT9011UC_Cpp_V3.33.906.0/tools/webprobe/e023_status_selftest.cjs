// AI(W906-E023-SB1) 20261002 [W906] (St01): todo E-023 SB-1 / SB-3 / SB-4 / TP-2, the page half (node, offline).
//   golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] (cp950, not in git; AI(W906-E030-CITE) 20261003: a bare ":N" is 906,
//   （V912 :N） is V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy):
//     cShowBinSelect.cpp btnAutoCleanClick :2101-2166（V912 :2248-2315） (Visible FormShow :803（V912 :864）), UPH_StringGridDblClick :2054-2099（V912 :2201-2246） (MessageBoxA MB_OKCANCEL :2063（V912 :2210）),
//     sbCopyRecipeClick :2845-2850（V912 :2994-2999）; cTemperFrom.cpp Panel73/72/71MouseDown :1692-1769（V912 :1702-1779） (MB_YESNO :1752（V912 :1762）, QWERTY password :1760-1761（V912 :1770-1771）).
//   The C++ half is ctest E023_StatusEvents (tests/test_e023_statusev.cpp).
//   [A] D:\HT9045\web\page\ht9045_showbinselect_ev.js in a node vm with a fake DOM / HT9045Recipe / HT9045Tags (+ the real ht9045_busy_util.js):
//       compiles, one EOL style, no BOM; the load-time state hides / shows btnAutoClean (unknown -> shown); Auto Clean and Copy Recipe send one
//       act each (control.acquire first, control.release after) and explain the reply (a refusal in red, golden's message text); the UPH
//       double-click sends row / pageIndex 2 / the row's cells from binsel.uph.grid, asks golden's OK / Cancel text, sends "ok" / "cancel";
//       busy: is not an error; a disabled button and a click outside a row send nothing
//   [B] ht9045_temperfrom_ev.js: mousedown on Panel73 / 72 / 71 sends act.temperFrom.mouseDown in click order with golden's button names;
//       needConfirm -> confirm(golden's text); needPassword -> the QWERTY keyboard with N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD, the typed text goes to
//       C++ only (state() keeps "***"); NO is sent; opened -> postMessage {open:'handlersys'}; not opened -> nothing; right click has no menu
//   [C] the pages: Status.ShowBinSelect.html loads ht9045_showbinselect_ev.js after the wire file and has btnAutoClean / sbCopyRecipe /
//       sbsUphStatus; the wire file marks UPH rows data-r; Status.TemperFrom.html has the three LEDs, loads ht9045_temperfrom_ev.js, has no
//       prompt() and does NOT contain golden's password literal (read from golden V912 cTemperFrom.cpp:1771 (= 906 :1761; the V912 tree is the one in git), or the port's cTemperFrom_E023.cpp;
//       never printed)
//   Control: W906_E023_PAGE_DIR -> the pre-change page directory must be red.
//   No wb_serve, no machine file (reads .js / .html under web\page and the two source files). Use: only through ctest (E023_StatusPages).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_E023_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const PORT_DIR = path.join(__dirname, '..', '..');
let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async () => { for (let i = 0; i < 20; i++) await new Promise(r => setImmediate(r)); };
function read(f) { return fs.readFileSync(path.join(PAGE_DIR, f)); }

function makeEl(id, tag) {
  const attrs = {}, listeners = {};
  const el = {
    id, tagName: tag || 'DIV', nodeType: 1, disabled: false, title: '', style: {}, textContent: '', innerHTML: '',
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    hasAttribute: k => k in attrs,
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    listeners: t => (listeners[t] || []).length,
    fire: (t, ev) => { const e = Object.assign({ type: t, prevented: 0, preventDefault() { e.prevented++; } }, ev || {}); (listeners[t] || []).forEach(fn => fn(e)); return e; },
  };
  return el;
}
function noBomOneEol(bytes, text) {
  ok(!(bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF), 'has a UTF-8 BOM');
  const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
  ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
}

// replies: { '<cmd>': [ {ok, v} | {busy:true} | {delay, ok, v} ] }
function makeRecipe(replies, sent) {
  return {
    status: () => ({ holdsToken: false }),
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, value: extra && extra.value ? JSON.parse(extra.value) : undefined });
      if (cmd === 'control.acquire' || cmd === 'control.release') return Promise.resolve({});
      const q = replies[cmd] || [];
      const r = q.shift() || { ok: false, v: { executed: false, guard: 'test-no-reply' } };
      const p = r.busy ? Promise.reject(new Error('busy: same command in progress or just done (' + cmd + ', 10 ms ago)'))
              : r.ok ? Promise.resolve(r.v) : Promise.reject(new Error(JSON.stringify(r.v)));
      return r.delay ? new Promise((res, rej) => setTimeout(() => p.then(res, rej), r.delay)) : p;
    },
  };
}

// ---- [A] ht9045_showbinselect_ev.js -------------------------------------------------------------------------------------------
function makeSbsEnv(script, busyScript, opts) {
  opts = opts || {};
  const els = { btnAutoClean: makeEl('btnAutoClean', 'BUTTON'), sbCopyRecipe: makeEl('sbCopyRecipe', 'BUTTON'),
                UPH_StringGrid: makeEl('UPH_StringGrid', 'TABLE'), sbsStatus: makeEl('sbsStatus'), sbsUphStatus: makeEl('sbsUphStatus') };
  const tabAct = makeEl('', 'SPAN'); tabAct.setAttribute('data-tab', opts.tab || 'uph');
  const tabIndex = makeEl('', 'SPAN'); tabIndex.setAttribute('data-tab', 'index');
  const sent = [], asked = [];
  const replies = opts.replies || {};
  if (!replies['act.showBinSelect.state']) replies['act.showBinSelect.state'] = [{ ok: true, v: { executed: true, autoCleanVisible: true, running: false } }];
  const tags = { 'binsel.uph.grid': opts.grid === undefined ? 'Time\tLot\tCount\tUPH\nT01\tL01\t10\t100\nT02\tL02\t20\t200\nT03\tL03\t30\t300' : opts.grid };
  const doc = {
    readyState: 'complete', getElementById: id => els[id] || null, addEventListener() {},
    querySelector: s => (s === '#PageControl1 > .tab.act' ? tabAct : s === '#PageControl1 > .tab[data-tab="index"]' ? tabIndex : null),
  };
  const ctx = vm.createContext({
    HT9045Recipe: makeRecipe(replies, sent),
    HT9045Tags: { has: k => k in tags && tags[k] !== null, get: k => tags[k] },
    confirm: t => { asked.push(t); return opts.confirm !== undefined ? opts.confirm : true; },
    console: { info() {}, log() {}, error() {} },
    document: doc, setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', ctx);
  if (busyScript) busyScript.runInContext(ctx);
  script.runInContext(ctx);
  return { ctx, els, sent, asked, replies, tabIndex };
}
const acts = env => env.sent.filter(s => !/^control\./.test(s.cmd));

// ---- [B] ht9045_temperfrom_ev.js ----------------------------------------------------------------------------------------------
function makeTpEnv(script, opts) {
  opts = opts || {};
  const els = { Panel71: makeEl('Panel71', 'SPAN'), Panel72: makeEl('Panel72', 'SPAN'), Panel73: makeEl('Panel73', 'SPAN') };
  const sent = [], asked = [], posted = [], kb = [];
  const replies = opts.replies || {};
  const ctx = vm.createContext({
    HT9045Recipe: makeRecipe(replies, sent),
    HTQwerty: { N: { NO_SYMBOL: 0x0004, PASSWORD: 0x0008, NO_SPACE: 0x0010 },
                show: (t, flags, o) => { kb.push(flags); setTimeout(() => { if (opts.kbAbort) o.onAbort(); else o.onCommit(opts.typed || ''); }, 1); } },
    confirm: t => { asked.push(t); return opts.confirm !== undefined ? opts.confirm : true; },
    prompt: () => { throw new Error('prompt() must not be used when HTQwerty is there'); },
    console: { info() {}, log() {}, error() {} },
    parent: { postMessage: (m, o) => posted.push({ m, o }) },
    document: { readyState: 'complete', getElementById: id => els[id] || null, addEventListener() {} },
    setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', ctx);
  script.runInContext(ctx);
  return { ctx, els, sent, asked, posted, kb, replies };
}

function goldenLiteral() {
  // golden V912 cTemperFrom.cpp:1771 (= 906 :1761, same line; cp950 file, the literal is ASCII); else the port's translation.  The value is never printed.
  const tries = [['D:\\HT9045\\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\\cTemperFrom.cpp', /edPassword->Text!="([^"]+)"/],
                 [path.join(PORT_DIR, 'cTemperFrom_E023.cpp'), /edPasswordText!="([^"@]+)"/]];
  for (const [f, re] of tries) {
    try { const m = fs.readFileSync(f).toString('latin1').match(re); if (m) return m[1]; } catch (e) { /* next */ }
  }
  return null;
}

(async () => {
  let sbsText = '', sbsBytes = null, sbsScript = null, busyScript = null, tpText = '', tpBytes = null, tpScript = null;
  await check('[A] read ht9045_showbinselect_ev.js', () => { sbsBytes = read('ht9045_showbinselect_ev.js'); sbsText = sbsBytes.toString('utf8'); });
  await check('[A] no BOM, one EOL style', () => noBomOneEol(sbsBytes, sbsText));
  await check('[A] compiles', () => { ok(sbsBytes, 'the file was not read'); sbsScript = new vm.Script(sbsText, { filename: 'ht9045_showbinselect_ev.js' }); });
  await check('[A] ht9045_busy_util.js compiles', () => { busyScript = new vm.Script(read('ht9045_busy_util.js').toString('utf8'), { filename: 'ht9045_busy_util.js' }); });

  if (sbsScript) {
    // A1 load-time state
    {
      const env = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.state': [{ ok: true, v: { executed: true, autoCleanVisible: false } }] } });
      await flush();
      await check('[A1] load: control.acquire, act.showBinSelect.state {}, control.release; autoCleanVisible false -> btnAutoClean hidden (golden FormShow :803 (V912 :864))', () => {
        ok(env.sent[0] && env.sent[0].cmd === 'control.acquire', JSON.stringify(env.sent[0]));
        const a = acts(env);
        ok(a.length === 1 && a[0].cmd === 'act.showBinSelect.state', JSON.stringify(a));
        ok(env.sent.some(s => s.cmd === 'control.release'), 'no release');
        ok(env.els.btnAutoClean.style.display === 'none', 'display ' + env.els.btnAutoClean.style.display);
      });
      env.replies['act.showBinSelect.state'] = [{ ok: true, v: { executed: true, autoCleanVisible: true, running: true } }];
      env.tabIndex.fire('click');
      await flush();
      await check('[A1] clicking the Index tab asks again; visible -> shown, the running note in the title (allowed while running, Steven Q67 = B)', () => {
        ok(acts(env).filter(s => s.cmd === 'act.showBinSelect.state').length === 2, JSON.stringify(acts(env)));
        ok(env.els.btnAutoClean.style.display === '' && /運轉中/.test(env.els.btnAutoClean.title) && /Q67/.test(env.els.btnAutoClean.title) &&
           !/先不收/.test(env.els.btnAutoClean.title), env.els.btnAutoClean.title);
      });
      const env2 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.state': [{ ok: false, v: { executed: false, guard: 'transport' } }] } });
      await flush();
      await check('[A1] state unknown -> the button is shown (C++ checks again)', () => { ok(env2.els.btnAutoClean.style.display === '', env2.els.btnAutoClean.style.display); });
    }
    // A2 Auto Clean
    {
      const env = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.autoClean': [{ ok: true, v: { executed: true, result: 'armed', messages: [] } }] } });
      await flush();
      env.els.btnAutoClean.fire('click');
      await flush();
      await check('[A2] Auto Clean -> one act.showBinSelect.autoClean {} (golden has no confirm box), armed explained', () => {
        const a = acts(env).filter(s => s.cmd !== 'act.showBinSelect.state');
        ok(a.length === 1 && a[0].cmd === 'act.showBinSelect.autoClean' && JSON.stringify(a[0].value) === '{}', JSON.stringify(a));
        ok(env.asked.length === 0, 'asked ' + env.asked.length);
        ok(/Auto Clean 已登記/.test(env.els.sbsStatus.textContent) && /START/.test(env.els.sbsStatus.textContent), env.els.sbsStatus.textContent);
      });
      // AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B -- C++ no longer refuses while running; a refusal it still sends is not-open
      const env2 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.autoClean': [{ ok: false, v: { executed: false, guard: 'not-open', zh: 'BinSelect 視窗沒有開著' } }] } });
      await flush();
      env2.els.btnAutoClean.fire('click');
      await flush();
      await check('[A2] refused (window not open) -> the C++ Chinese text in red', () => {
        ok(/BinSelect 視窗沒有開著/.test(env2.els.sbsStatus.textContent) && env2.els.sbsStatus.style.color === '#b00', env2.els.sbsStatus.textContent);
      });
      const env3 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.autoClean': [{ ok: true, v: { executed: true, result: 'nothing', messages: [{ en: 'Tray arm not Safe pos', zh: '' }] } }] } });
      await flush();
      env3.els.btnAutoClean.fire('click');
      await flush();
      await check('[A2] golden returned with a message -> the message text on the status line (the box itself came from wb_serve)', () => {
        ok(/Tray arm not Safe pos/.test(env3.els.sbsStatus.textContent), env3.els.sbsStatus.textContent);
      });
      const env4 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.autoClean': [{ ok: true, v: { executed: true, result: 'oneCycle', messages: [] } }] } });
      await flush();
      env4.els.btnAutoClean.fire('click');
      await flush();
      await check('[A2] part in the machine -> "One Cycle first" explained', () => { ok(/One Cycle/.test(env4.els.sbsStatus.textContent), env4.els.sbsStatus.textContent); });
      const env5 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.autoClean': [{ ok: true, v: { executed: true, result: 'armed', running: true, messages: [] } }] } });
      await flush();
      env5.els.btnAutoClean.fire('click');
      await flush();
      await check('[A2] armed while running (Steven Q67 = B) -> the run takes it over, no "press START" text', () => {
        ok(/Auto Clean 已登記/.test(env5.els.sbsStatus.textContent) && /運轉中/.test(env5.els.sbsStatus.textContent) &&
           !/START/.test(env5.els.sbsStatus.textContent) && env5.els.sbsStatus.style.color !== '#b00', env5.els.sbsStatus.textContent);
      });
    }
    // A3 Copy Recipe
    {
      const env = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.copyRecipe': [{ ok: true, v: { executed: true, recipe: 'R1', target: 'D:\\Run', rc: [0, 0] } }] } });
      await flush();
      env.els.sbCopyRecipe.fire('click');
      await flush();
      await check('[A3] Copy Recipe -> one act.showBinSelect.copyRecipe {}, "copied to D:\\Run" explained', () => {
        const a = acts(env).filter(s => s.cmd !== 'act.showBinSelect.state');
        ok(a.length === 1 && a[0].cmd === 'act.showBinSelect.copyRecipe', JSON.stringify(a));
        ok(/已把配方「R1」複製到 D:\\Run/.test(env.els.sbsStatus.textContent), env.els.sbsStatus.textContent);
      });
      const env2 = makeSbsEnv(sbsScript, busyScript, { replies: { 'act.showBinSelect.copyRecipe': [{ busy: true }] } });
      await flush();
      env2.els.sbCopyRecipe.fire('click');
      await flush();
      await check('[A3] busy: from WebCmdGuard is not an error (normal colour)', () => { ok(env2.els.sbsStatus.style.color === '', env2.els.sbsStatus.textContent); });
      const env3 = makeSbsEnv(sbsScript, busyScript);
      await flush();
      env3.els.sbCopyRecipe.disabled = true; env3.els.btnAutoClean.disabled = true;
      env3.els.sbCopyRecipe.fire('click'); env3.els.btnAutoClean.fire('click');
      await flush();
      await check('[A3] disabled buttons send nothing', () => { ok(acts(env3).filter(s => s.cmd !== 'act.showBinSelect.state').length === 0, JSON.stringify(acts(env3))); });
    }
    // A4 UPH double-click
    {
      const tr = makeEl('', 'TR'); tr.setAttribute('data-r', '2');
      const env = makeSbsEnv(sbsScript, busyScript, { confirm: true, replies: { 'act.showBinSelect.uphDblClick': [
        { ok: false, v: { executed: false, needConfirm: true, prompt: ['Do you want to delete this record?', 'Confirm'] } },
        { ok: true, v: { executed: true, deleted: true, avgUPH: '200' } }] } });
      await flush();
      env.els.UPH_StringGrid.fire('dblclick', { target: { closest: s => (s === 'tr' ? tr : null) } });
      await flush();
      await check('[A4] dblclick on row 2 -> {row:2, pageIndex:2 (Tab_UPH), cells: the row from binsel.uph.grid, answer:null}', () => {
        const a = acts(env).filter(s => s.cmd === 'act.showBinSelect.uphDblClick');
        ok(a.length === 2, JSON.stringify(a));
        ok(a[0].value.row === 2 && a[0].value.pageIndex === 2 && a[0].value.answer === null &&
           JSON.stringify(a[0].value.cells) === JSON.stringify(['T02', 'L02', '20', '200']), JSON.stringify(a[0]));
      });
      await check('[A4] needConfirm -> confirm(golden text + caption, :2063 (V912 :2210)), OK -> {answer:"ok"}; deleted explained on the UPH status line', () => {
        ok(env.asked.length === 1 && env.asked[0] === 'Do you want to delete this record?\nConfirm', JSON.stringify(env.asked));
        const a = acts(env).filter(s => s.cmd === 'act.showBinSelect.uphDblClick');
        ok(a[1].value.answer === 'ok' && a[1].value.row === 2, JSON.stringify(a[1]));
        ok(/已刪除/.test(env.els.sbsUphStatus.textContent), env.els.sbsUphStatus.textContent);
      });
      const env2 = makeSbsEnv(sbsScript, busyScript, { confirm: false, tab: 'index', replies: { 'act.showBinSelect.uphDblClick': [
        { ok: false, v: { executed: false, needConfirm: true } }, { ok: true, v: { executed: true, deleted: false } }] } });
      await flush();
      await env2.ctx.HT9045ShowBinSelectEv.uphDblClick(1);
      await check('[A4] Cancel -> {answer:"cancel"}; another active tab is sent as its golden index (C++ refuses, golden :2058 (V912 :2205))', () => {
        const a = acts(env2).filter(s => s.cmd === 'act.showBinSelect.uphDblClick');
        ok(a.length === 2 && a[1].value.answer === 'cancel' && a[0].value.pageIndex === 4, JSON.stringify(a));
      });
      const env3 = makeSbsEnv(sbsScript, busyScript);
      await flush();
      env3.els.UPH_StringGrid.fire('dblclick', { target: { closest: () => null } });
      await flush();
      const env4 = makeSbsEnv(sbsScript, busyScript, { grid: null });
      await flush();
      await env4.ctx.HT9045ShowBinSelectEv.uphDblClick(1);
      await check('[A4] dblclick outside a row, or no binsel.uph.grid yet -> nothing sent', () => {
        ok(acts(env3).filter(s => s.cmd === 'act.showBinSelect.uphDblClick').length === 0, JSON.stringify(acts(env3)));
        ok(acts(env4).filter(s => s.cmd === 'act.showBinSelect.uphDblClick').length === 0 && /還沒讀到/.test(env4.els.sbsUphStatus.textContent), env4.els.sbsUphStatus.textContent);
      });
    }
  }

  await check('[B] read ht9045_temperfrom_ev.js', () => { tpBytes = read('ht9045_temperfrom_ev.js'); tpText = tpBytes.toString('utf8'); });
  await check('[B] no BOM, one EOL style', () => noBomOneEol(tpBytes, tpText));
  await check('[B] compiles', () => { ok(tpBytes, 'the file was not read'); tpScript = new vm.Script(tpText, { filename: 'ht9045_temperfrom_ev.js' }); });
  const lit = goldenLiteral();
  if (tpScript) {
    const mdAct = env => env.sent.filter(s => s.cmd === 'act.temperFrom.mouseDown').map(s => s.value);
    // B1 order + names, no password
    {
      const env = makeTpEnv(tpScript, { confirm: true, replies: { 'act.temperFrom.mouseDown': [
        { delay: 15, ok: true, v: { executed: true, panel: 73 } }, { ok: true, v: { executed: true, panel: 72 } },
        { ok: false, v: { executed: false, needConfirm: true, prompt: ['Reset the hardware apparatus information?'], needPassword: false } },
        { ok: true, v: { executed: true, opened: true, open: 'handlersys' } }] } });
      env.els.Panel73.fire('mousedown', { button: 0 });
      env.els.Panel72.fire('mousedown', { button: 0 });
      const cm = env.els.Panel71.fire('contextmenu', {});
      env.els.Panel71.fire('mousedown', { button: 2 });
      await new Promise(r => setTimeout(r, 40)); await flush();
      await check('[B1] 73 left, 72 left, 71 right -> three requests in click order (the first reply was slow), golden button names', () => {
        const v = mdAct(env);
        ok(v.length === 4, JSON.stringify(v));
        ok(v[0].panel === 73 && v[0].button === 'left' && v[1].panel === 72 && v[1].button === 'left' && v[2].panel === 71 && v[2].button === 'right', JSON.stringify(v));
        ok(cm.prevented === 1, 'contextmenu not prevented');
      });
      await check('[B1] needConfirm -> confirm(golden :1752 (V912 :1762) text); needPassword false -> {answer:"yes"} without a password, no keyboard', () => {
        ok(env.asked.length === 1 && env.asked[0] === 'Reset the hardware apparatus information?', JSON.stringify(env.asked));
        const v = mdAct(env);
        ok(v[3].answer === 'yes' && !('password' in v[3]) && env.kb.length === 0, JSON.stringify(v[3]));
      });
      await check('[B1] opened -> postMessage {open:"handlersys"} (golden HandlerSystem->ShowModal :1768 (V912 :1778))', () => {
        ok(env.posted.length === 1 && env.posted[0].m.open === 'handlersys', JSON.stringify(env.posted));
      });
    }
    // B2 password through the keyboard
    {
      const env = makeTpEnv(tpScript, { confirm: true, typed: 'pw-test-1', replies: { 'act.temperFrom.mouseDown': [
        { ok: false, v: { executed: false, needConfirm: true, needPassword: true } }, { ok: true, v: { executed: true, opened: false, returned: '906 cTemperFrom.cpp:1761-1764 (V912 :1771-1774) (password)' } }] } });
      env.els.Panel71.fire('mousedown', { button: 2 });
      await new Promise(r => setTimeout(r, 10)); await flush();
      await check('[B2] needPassword -> the QWERTY keyboard with N_NO_SYMBOL|N_NO_SPACE|N_PASSWORD (golden :1760 (V912 :1770)); the typed text goes to C++', () => {
        ok(env.kb.length === 1 && env.kb[0] === (0x0004 | 0x0010 | 0x0008), JSON.stringify(env.kb));
        const v = mdAct(env);
        ok(v.length === 2 && v[1].answer === 'yes' && v[1].password === 'pw-test-1', JSON.stringify(v.map(x => Object.assign({}, x, { password: x.password ? '(set)' : x.password }))));
      });
      await check('[B2] state() keeps "***" for the password; not opened -> no postMessage', () => {
        const s = env.ctx.HT9045TemperFromEv.state();
        ok(s.sent.length === 2 && s.sent[1].password === '***', JSON.stringify(s.sent));
        ok(env.posted.length === 0, JSON.stringify(env.posted));
      });
      const env2 = makeTpEnv(tpScript, { confirm: true, kbAbort: true, replies: { 'act.temperFrom.mouseDown': [
        { ok: false, v: { executed: false, needConfirm: true, needPassword: true } }, { ok: true, v: { executed: true, opened: false } }] } });
      env2.els.Panel71.fire('mousedown', { button: 1 });
      await new Promise(r => setTimeout(r, 10)); await flush();
      await check('[B2] keyboard Abort -> an empty password is sent (golden :1759 (V912 :1769) cleared the edit), middle button named "middle"', () => {
        const v = mdAct(env2);
        ok(v.length === 2 && v[0].button === 'middle' && v[1].password === '', JSON.stringify(v));
      });
    }
    // B3 NO
    {
      const env = makeTpEnv(tpScript, { confirm: false, replies: { 'act.temperFrom.mouseDown': [
        { ok: false, v: { executed: false, needConfirm: true, needPassword: true } }, { ok: true, v: { executed: true, opened: false } }] } });
      env.els.Panel71.fire('mousedown', { button: 2 });
      await flush();
      await check('[B3] NO -> {answer:"no"} is sent, no keyboard, no postMessage', () => {
        const v = mdAct(env);
        ok(v.length === 2 && v[1].answer === 'no' && env.kb.length === 0 && env.posted.length === 0, JSON.stringify(v));
      });
    }
  }

  // ---- [C] the pages -------------------------------------------------------------------------------------------------------------
  await check('[C] Status.ShowBinSelect.html loads ht9045_showbinselect_ev.js after the wire file; btnAutoClean / sbCopyRecipe / sbsUphStatus are there', () => {
    const h = read('Status.ShowBinSelect.html').toString('utf8').replace(/<!--[\s\S]*?-->/g, '');
    const w = h.indexOf('<script src="ht9045_showbinselect_wire.js"></script>'), e = h.indexOf('<script src="ht9045_showbinselect_ev.js"></script>');
    ok(w >= 0 && e > w, 'wire ' + w + ' ev ' + e);
    ok(/id="btnAutoClean"/.test(h) && /id="sbCopyRecipe"/.test(h) && /id="sbsUphStatus"/.test(h), 'an element is missing');
  });
  await check('[C] ht9045_showbinselect_wire.js marks each UPH row with data-r', () => {
    const t = read('ht9045_showbinselect_wire.js').toString('utf8');
    ok(t.indexOf("h += '<tr data-r=\"' + r + '\">';") >= 0, 'data-r line missing');
  });
  await check('[C] Status.TemperFrom.html: three LEDs, loads ht9045_temperfrom_ev.js after the recipe client, no prompt(), no old link', () => {
    const raw = read('Status.TemperFrom.html').toString('utf8');
    const h = raw.replace(/<!--[\s\S]*?-->/g, '');
    ok(/id="Panel71"/.test(h) && /id="Panel72"/.test(h) && /id="Panel73"/.test(h), 'a panel is missing');
    const c = h.indexOf('<script src="ht9045_recipe_client.js"></script>'), e = h.indexOf('<script src="ht9045_temperfrom_ev.js"></script>');
    ok(c >= 0 && e > c, 'recipe client ' + c + ' ev ' + e);
    ok(h.indexOf('prompt(') < 0 && h.indexOf('Handler System 密碼') < 0 && h.indexOf('⚙ Handler System') < 0, 'the old prompt / link is still there');
  });
  await check('[C] golden\'s password literal is not in Status.TemperFrom.html or ht9045_temperfrom_ev.js (comments included; value not printed)', () => {
    ok(lit && lit.length > 0, 'golden literal not found (golden V912 cTemperFrom.cpp:1771 = 906 :1761 / port cTemperFrom_E023.cpp)');
    ok(read('Status.TemperFrom.html').toString('latin1').indexOf(lit) < 0, 'Status.TemperFrom.html still holds it');
    ok(read('ht9045_temperfrom_ev.js').toString('latin1').indexOf(lit) < 0, 'ht9045_temperfrom_ev.js holds it');
  });

  console.log(passed + '/' + (passed + failed) + ' checks passed (E023_StatusPages, page dir ' + PAGE_DIR + ')');
  process.exit(failed ? 1 : 0);
})();
