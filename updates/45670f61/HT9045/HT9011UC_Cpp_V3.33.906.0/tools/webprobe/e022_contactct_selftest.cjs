// AI(W906-E022-CK1) 20261002 [W906] (St01): todo E-022 CK-1 / CK-2 / CK-3 / SC-2, the page half (node, offline).
//   golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] (cp950, not in git; AI(W906-E030-CITE) 20261003: ":N" = 906 line, V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in brackets when it differs):
//     cContactCT.cpp btClearCountClick :944-1069 (MessageBox YES/NO :1001), sgYieldDblClick :740-886 (:853), btYieldChartClick :1071-1075;
//     cObserver.cpp FormShow :390-404 (iShowYieldChart==1 -> pgcObserv->ActivePageIndex=4);
//     cStartCondition.cpp sb_Maintenance_SmartDiagnosticFunctionClick :1080-1084 [V912 :1254-1258] (fSmartDiagnostic->ShowModal()).
//   The C++ half is ctest E022_ContactCT (tests/test_e022_contactct.cpp).
//   [A] D:\HT9045\web\page\ht9045_contactct_ev.js in a node vm with a fake DOM / HT9045Recipe / HT9045ContactCT (+ the real ht9045_busy_util.js):
//       compiles, one EOL style, no BOM; Count Clear = control.acquire, act.contactCT.clearCount {answer:null}, needConfirm -> confirm(golden's
//       two strings) -> {answer:"yes"} / {answer:"no"} (NO is sent: golden's tail); refusal shown, no box; busy: is not an error; repaint after;
//       grid double-click sends the td's col / row + the shown yieldType; Yield Chart writes ht9045.observer.yieldChart and posts {open:'observer'};
//       a disabled button sends nothing
//   [B] ht9045_contactct_wire.js as is: binds "not wired" to the two buttons only when ht9045_contactct_ev.js is absent
//   [C] Data.ContactCT.html loads ht9045_busy_util.js and ht9045_contactct_ev.js after ht9045_contactct_wire.js (outside comments)
//   [D] ht9045_observer_wire.js (cut: openPage, the tab listener line, main's Yield Chart block var winOpen .. storage listener, start()'s
//       openPage line): an open answer with activePage 4 shows the Yield tab without sending act tab; a user click still sends it; the
//       storage key re-runs open with arg 1 (main's takeYc listener, the only one); a stale key is dropped at load.
//       AI(W906-R6MERGE) 20261002 (St01): merge of main 283f8382 -- main's AI(W906-OBS-YCHART) storage listener replaced St01's (two listeners ran golden
//       FormShow twice); openPage takes main's yieldChart parameter.
//   [E] ht9045_startcondition_c.js (cut: usable() + decorate()'s button loop): Smart Diagnostic posts {open:'smartdiag'}, not when unusable;
//       btnSetOffsetLimit stays disabled
//   Control: W906_E022_PAGE_DIR -> the pre-change page directory must be red.
//   No wb_serve, no machine file (reads .js / .html under web\page). Use: only through ctest (E022_ContactCTPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_E022_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async () => { for (let i = 0; i < 10; i++) await new Promise(r => setImmediate(r)); };
function read(f) { return fs.readFileSync(path.join(PAGE_DIR, f)); }

function makeEl(id, tag) {
  const attrs = {}, listeners = {};
  const el = {
    id, tagName: tag || 'DIV', nodeType: 1, disabled: false, title: '', style: {}, textContent: '', innerHTML: '', children: [],
    classList: (() => { const s = new Set(); return { contains: c => s.has(c), add: c => s.add(c), remove: c => s.delete(c) }; })(),
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    hasAttribute: k => k in attrs,
    removeAttribute: k => { delete attrs[k]; },
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    listeners: t => (listeners[t] || []).length,
    fire: (t, ev) => { (listeners[t] || []).forEach(fn => fn(Object.assign({ type: t, preventDefault() {} }, ev || {}))); },
    click: () => el.fire('click'),
    appendChild: c => { el.children.push(c); return c; },
    querySelector: () => null,
    querySelectorAll: () => [],
    parentNode: null,
  };
  return el;
}

// ---- [A] ht9045_contactct_ev.js ----------------------------------------------------------------------------------------------
function makeEvEnv(script, busyScript, opts) {
  opts = opts || {};
  const els = { btClearCount: makeEl('btClearCount', 'BUTTON'), btYieldChart: makeEl('btYieldChart', 'BUTTON'),
                sgYield: makeEl('sgYield', 'TABLE'), ctStatus: makeEl('ctStatus') };
  const sent = [], posted = [], stored = {}, asked = [], reloads = [];
  const replies = [];          // queue: {ok:bool, v:object}
  const R = {
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, value: extra && extra.value ? JSON.parse(extra.value) : undefined });
      if (cmd === 'control.acquire') return Promise.resolve({});
      const r = replies.shift() || { ok: false, v: { executed: false, guard: 'test-no-reply' } };
      if (r.busy) return Promise.reject(new Error('busy: same command in progress or just done (' + cmd + ', 10 ms ago)'));
      return r.ok ? Promise.resolve(r.v) : Promise.reject(new Error(JSON.stringify(r.v)));
    },
  };
  // the vm context IS the window (a browser resolves bare HT9045Recipe / HT9045Busy on window)
  const doc = { readyState: 'complete', getElementById: id => els[id] || null, addEventListener() {} };
  const win = vm.createContext({
    HT9045Recipe: R,
    HT9045ContactCT: { data: () => ({ rgYieldType: { itemIndex: opts.itemIndex === undefined ? 3 : opts.itemIndex } }),
                       reload: yt => { reloads.push(yt); return Promise.resolve(null); } },
    confirm: t => { asked.push(t); return opts.confirm !== undefined ? opts.confirm : true; },
    localStorage: { setItem: (k, v) => { stored[k] = v; }, removeItem: k => { delete stored[k]; } },
    console: { info() {}, log() {}, error() {} },
    parent: { postMessage: (m, o) => posted.push({ m, o }) },
    document: doc, setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', win);
  const ctx = win;
  if (busyScript) busyScript.runInContext(ctx);
  script.runInContext(ctx);
  return { ctx, win, els, sent, posted, stored, asked, reloads, replies };
}
const acts = env => env.sent.filter(s => s.cmd !== 'control.acquire');

(async () => {
  let evText = '', evBytes = null, evScript = null, busyScript = null;
  await check('[A] read ht9045_contactct_ev.js', () => { evBytes = read('ht9045_contactct_ev.js'); evText = evBytes.toString('utf8'); });
  await check('[A] no BOM, one EOL style', () => {
    ok(!(evBytes[0] === 0xEF && evBytes[1] === 0xBB && evBytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (evText.match(/\r\n/g) || []).length, lf = (evText.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  await check('[A] compiles', () => { evScript = new vm.Script(evText, { filename: 'ht9045_contactct_ev.js' }); });
  await check('[A] ht9045_busy_util.js compiles', () => { busyScript = new vm.Script(read('ht9045_busy_util.js').toString('utf8'), { filename: 'ht9045_busy_util.js' }); });

  if (evScript) {
    // A1 Count Clear, YES
    {
      const env = makeEvEnv(evScript, busyScript, { confirm: true });
      env.replies.push({ ok: false, v: { executed: false, needConfirm: true, prompt: ['Do you want to clear the data?', 'Warning!!'], goldenLine: '906 cContactCT.cpp:1001 (V912 :1001)' } });
      env.replies.push({ ok: true, v: { executed: true, answer: 'yes', cleared: true } });
      env.els.btClearCount.fire('click');
      await flush();
      await check('[A1] Count Clear: control.acquire, then act.contactCT.clearCount {answer:null}', () => {
        ok(env.sent[0] && env.sent[0].cmd === 'control.acquire', 'first ' + JSON.stringify(env.sent[0]));
        const a = acts(env);
        ok(a.length === 2 && a[0].cmd === 'act.contactCT.clearCount' && a[0].value.answer === null, JSON.stringify(a));
      });
      await check('[A1] needConfirm -> confirm() with golden\'s text and caption (:1001), YES -> {answer:"yes"}', () => {
        ok(env.asked.length === 1 && env.asked[0] === 'Do you want to clear the data?\nWarning!!', JSON.stringify(env.asked));
        ok(acts(env)[1].value.answer === 'yes', JSON.stringify(acts(env)[1]));
      });
      await check('[A1] executed -> status "完成", repaint = HT9045ContactCT.reload(shown item 3)', () => {
        ok(/Count Clear 完成/.test(env.els.ctStatus.textContent), env.els.ctStatus.textContent);
        ok(env.reloads.length === 1 && env.reloads[0] === 3, JSON.stringify(env.reloads));
      });
    }
    // A2 Count Clear, NO is sent
    {
      const env = makeEvEnv(evScript, busyScript, { confirm: false });
      env.replies.push({ ok: false, v: { executed: false, needConfirm: true, prompt: ['Do you want to clear the data?', 'Warning!!'] } });
      env.replies.push({ ok: true, v: { executed: true, answer: 'no', cleared: false } });
      env.els.btClearCount.fire('click');
      await flush();
      await check('[A2] confirm() = false -> {answer:"no"} IS sent (golden NO still runs the tail :1050-1068)', () => {
        const a = acts(env);
        ok(a.length === 2 && a[1].cmd === 'act.contactCT.clearCount' && a[1].value.answer === 'no', JSON.stringify(a));
        ok(/收尾/.test(env.els.ctStatus.textContent), env.els.ctStatus.textContent);
      });
    }
    // A3 refusal, no box
    {
      const env = makeEvEnv(evScript, busyScript);
      env.replies.push({ ok: false, v: { executed: false, guard: 'running', goldenLine: '906 cContactCT.cpp:947 (V912 :947)', detail: 'SystemStart' } });
      env.els.btClearCount.fire('click');
      await flush();
      await check('[A3] refused (running) -> no confirm(), one request, the guard on the status line (red)', () => {
        ok(env.asked.length === 0 && acts(env).length === 1, 'asked ' + env.asked.length + ' sent ' + acts(env).length);
        ok(/running/.test(env.els.ctStatus.textContent) && env.els.ctStatus.style.color === '#c00', env.els.ctStatus.textContent);
        ok(env.reloads.length === 0, 'reloaded after a refusal');
      });
    }
    // A4 busy:
    {
      const env = makeEvEnv(evScript, busyScript);
      env.replies.push({ busy: true });
      env.els.btClearCount.fire('click');
      await flush();
      await check('[A4] busy: from WebCmdGuard is not an error (HT9045Busy.NOTE, normal colour)', () => {
        ok(env.els.ctStatus.style.color === '' && /同一個指令剛送過/.test(env.els.ctStatus.textContent), env.els.ctStatus.textContent);
      });
    }
    // A5 grid double-click
    {
      const env = makeEvEnv(evScript, busyScript, { confirm: true, itemIndex: 2 });
      env.replies.push({ ok: false, v: { executed: false, needConfirm: true, prompt: ['Do you want to clear the data?', 'Warning!!'] } });
      env.replies.push({ ok: true, v: { executed: true, asked: true, answer: 'yes', cleared: true } });
      const td = makeEl('', 'TD'); td.setAttribute('data-r', '3'); td.setAttribute('data-c', '1');
      env.els.sgYield.fire('dblclick', { target: { closest: s => (s === 'td' ? td : null) } });
      await flush();
      await check('[A5] dblclick on a td -> act.contactCT.dblClick {col:1,row:3,yieldType:<shown item>,answer:null} then "yes"', () => {
        const a = acts(env);
        ok(a.length === 2 && a[0].cmd === 'act.contactCT.dblClick', JSON.stringify(a));
        ok(a[0].value.col === 1 && a[0].value.row === 3 && a[0].value.yieldType === 2 && a[0].value.answer === null, JSON.stringify(a[0]));
        ok(a[1].value.answer === 'yes' && a[1].value.row === 3, JSON.stringify(a[1]));
      });
      const env2 = makeEvEnv(evScript, busyScript);
      env2.els.sgYield.fire('dblclick', { target: { closest: () => null } });
      await flush();
      await check('[A5] dblclick outside a cell sends nothing', () => { ok(env2.sent.length === 0, JSON.stringify(env2.sent)); });
    }
    // A6 Yield Chart
    {
      const env = makeEvEnv(evScript, busyScript);
      env.replies.push({ ok: true, v: { executed: true, iShowYieldChart: 1, observerShown: true, open: 'observer' } });
      env.els.btYieldChart.fire('click');
      await flush();
      await check('[A6] Yield Chart -> act.contactCT.yieldChart {}, then localStorage ht9045.observer.yieldChart + postMessage({open:"observer"})', () => {
        const a = acts(env);
        ok(a.length === 1 && a[0].cmd === 'act.contactCT.yieldChart', JSON.stringify(a));
        ok(typeof env.stored['ht9045.observer.yieldChart'] === 'string', JSON.stringify(env.stored));
        ok(env.posted.length === 1 && env.posted[0].m.open === 'observer', JSON.stringify(env.posted));
      });
      const env2 = makeEvEnv(evScript, busyScript);
      env2.replies.push({ ok: false, v: { executed: false, guard: 'not-open' } });
      env2.els.btYieldChart.fire('click');
      await flush();
      await check('[A6] refused -> no localStorage key, no postMessage', () => {
        ok(env2.posted.length === 0 && !('ht9045.observer.yieldChart' in env2.stored), JSON.stringify(env2.posted));
      });
    }
    // A7 disabled buttons
    {
      const env = makeEvEnv(evScript, busyScript);
      env.els.btClearCount.disabled = true; env.els.btYieldChart.disabled = true;
      env.els.btClearCount.fire('click'); env.els.btYieldChart.fire('click');
      await flush();
      await check('[A7] disabled buttons (golden Timer1Timer greys btClearCount while running) send nothing', () => { ok(env.sent.length === 0, JSON.stringify(env.sent)); });
    }
  }

  // ---- [B] ht9045_contactct_wire.js: "not wired" only without the ev file ------------------------------------------------------
  {
    let wireScript = null;
    await check('[B] ht9045_contactct_wire.js compiles', () => { wireScript = new vm.Script(read('ht9045_contactct_wire.js').toString('utf8'), { filename: 'ht9045_contactct_wire.js' }); });
    function runWire(withEv) {
      const els = { btClearCount: makeEl('btClearCount', 'BUTTON'), btYieldChart: makeEl('btYieldChart', 'BUTTON'), ctStatus: makeEl('ctStatus'),
                    sgYield: makeEl('sgYield', 'TABLE'), rgYieldType: makeEl('rgYieldType') };
      const doc = { readyState: 'complete', hidden: false, getElementById: id => els[id] || null, addEventListener() {} };
      const ctx = vm.createContext({ HT9045Recipe: { rawCmd: () => new Promise(() => {}) }, console: { info() {}, log() {} },
                                     document: doc, setTimeout, clearTimeout, setInterval: () => 0,
                                     Promise, JSON, Object, Array, String, Date, Error, RegExp, Math });
      if (withEv) ctx.HT9045ContactCTEv = {};
      vm.runInContext('var window = this;', ctx);
      wireScript.runInContext(ctx);
      return els;
    }
    if (wireScript) {
      await check('[B] with ht9045_contactct_ev.js: the wire file binds no click on btClearCount / btYieldChart', () => {
        const els = runWire(true);
        ok(els.btClearCount.listeners('click') === 0 && els.btYieldChart.listeners('click') === 0,
           els.btClearCount.listeners('click') + ' / ' + els.btYieldChart.listeners('click'));
      });
      await check('[B] without it: "not wired" stays bound (old behaviour)', () => {
        const els = runWire(false);
        ok(els.btClearCount.listeners('click') === 1 && els.btYieldChart.listeners('click') === 1,
           els.btClearCount.listeners('click') + ' / ' + els.btYieldChart.listeners('click'));
      });
    }
  }

  // ---- [C] Data.ContactCT.html script order --------------------------------------------------------------------------------------
  await check('[C] Data.ContactCT.html loads ht9045_busy_util.js and ht9045_contactct_ev.js after ht9045_contactct_wire.js (outside comments)', () => {
    const h = read('Data.ContactCT.html').toString('utf8').replace(/<!--[\s\S]*?-->/g, '');
    const w = h.indexOf('<script src="ht9045_contactct_wire.js"></script>'), b = h.indexOf('<script src="ht9045_busy_util.js"></script>'),
          e = h.indexOf('<script src="ht9045_contactct_ev.js"></script>');
    ok(w >= 0 && b > w && e > b, 'wire ' + w + ' busy ' + b + ' ev ' + e);
  });

  // ---- [D] ht9045_observer_wire.js -----------------------------------------------------------------------------------------------
  {
    const t = read('ht9045_observer_wire.js').toString('utf8').replace(/\r/g, '');
    const lines = t.split('\n');
    let code = null;
    await check('[D] cut openPage / the tab listener / the Yield Chart storage listener / start()\'s openPage line from ht9045_observer_wire.js', () => {
      const i = lines.findIndex(l => /function openPage\((?:yieldChart)?\) \{/.test(l));   // AI(W906-R6MERGE) 20261002 (St01): + main's parameter
      ok(i >= 0, 'openPage not found');
      let j = i; while (j < lines.length && lines[j] !== '  }') j++;
      ok(j < lines.length, 'end of openPage not found');
      const tabLine = lines.find(l => /n === 4 \|\| n === 6\) tb\.addEventListener\('click'/.test(l));
      ok(tabLine, 'tab listener line not found');
      const startLine = lines.find(l => /^    openPage\(\);/.test(l));
      ok(startLine, "start()'s openPage line not found");
      // AI(W906-R6MERGE) 20261002 (St01): main's AI(W906-OBS-YCHART) block, from 'var winOpen = null, lastYc = 0;' to the end of its storage listener
      const w0 = lines.findIndex(l => /^  var winOpen = null, lastYc = 0;/.test(l));
      ok(w0 >= 0, "main's Yield Chart block (var winOpen / lastYc) not found");
      const s0 = lines.findIndex((l, k) => k > w0 && /^  window\.addEventListener\('storage', function \(ev\) \{/.test(l));
      ok(s0 > w0, "main's storage listener not found");
      let s1 = s0; while (s1 < lines.length && lines[s1] !== '  });') s1++;
      ok(s1 < lines.length, 'end of the storage listener not found');
      code = lines.slice(i, j + 1).join('\n') + '\nvar n = 4, tb = TAB4;\n' + tabLine + '\n' + lines.slice(w0, s1 + 1).join('\n') + '\n' + startLine + '\n';
    });
    function runObs(reply, preKey) {
      const sent = [], storageFns = [], stored = {};
      if (preKey) stored['ht9045.observer.yieldChart'] = '1';
      const tab4 = makeEl('', 'DIV');
      const ctx = vm.createContext({
        TAB4: tab4, st: {}, opened: false, Promise, JSON, Object,
        send: v => { sent.push(v); return Promise.resolve(reply()); },
        document: { querySelector: s => (s === '#pgcObserv > .pcTabs > .tab[data-t="4"]' ? tab4 : null) },
      });
      ctx.window = { localStorage: { removeItem: k => { delete stored[k]; }, setItem: (k, v) => { stored[k] = v; },
                                     getItem: k => (k in stored ? stored[k] : null) },   // AI(W906-R6MERGE) 20261002 (St01): main's takeYc reads it
                     addEventListener: (ty, fn) => { if (ty === 'storage') storageFns.push(fn); } };
      ctx.localStorage = ctx.window.localStorage;   // AI(W906-R6MERGE) 20261002 (St01): main's takeYc uses the bare global
      vm.runInContext(code, ctx);
      return { ctx, sent, storageFns, stored, tab4 };
    }
    if (code) {
      const o = runObs(() => ({ full: true, activePage: 4 }));
      await flush();
      await check('[D] open answers activePage 4 (golden FormShow iShowYieldChart==1 :390-404) -> the Yield tab is shown, no act tab sent', () => {
        ok(o.sent.length === 1 && o.sent[0].act === 'open', JSON.stringify(o.sent));
        ok(o.ctx.st.e022YieldTab === 1, 'e022YieldTab ' + o.ctx.st.e022YieldTab);
      });
      o.tab4.click();
      await flush();
      await check('[D] a user click on the Yield tab still sends act tab 4 (pgcObservChange)', () => {
        ok(o.sent.length === 2 && o.sent[1].act === 'tab' && o.sent[1].arg === 4, JSON.stringify(o.sent));
      });
      await check('[D] the storage key ht9045.observer.yieldChart re-runs open with arg 1 (golden ShowModal -> FormShow again; one listener); other keys do nothing', () => {
        ok(o.storageFns.length === 1, 'storage listeners ' + o.storageFns.length);
        o.storageFns[0]({ key: 'something.else', newValue: '1' });
        o.stored['ht9045.observer.yieldChart'] = '2';
        o.storageFns[0]({ key: 'ht9045.observer.yieldChart', newValue: '2' });
        ok(o.sent.length === 3 && o.sent[2].act === 'open' && o.sent[2].arg === 1, JSON.stringify(o.sent));   // AI(W906-R6MERGE) 20261002 (St01): arg 1 = main's open
        ok(!('ht9045.observer.yieldChart' in o.stored), 'key not removed');
      });
      const o2 = runObs(() => ({ full: true, activePage: 0 }), true);
      await flush();
      await check('[D] activePage 0 -> no tab switch; a key left from before this load is dropped', () => {
        ok(o2.ctx.st.e022YieldTab === undefined && o2.sent.length === 1, JSON.stringify(o2.sent));
        ok(!('ht9045.observer.yieldChart' in o2.stored), 'stale key kept');
      });
    }
  }

  // ---- [E] ht9045_startcondition_c.js SC-2 -------------------------------------------------------------------------------------------
  {
    const t = read('ht9045_startcondition_c.js').toString('utf8').replace(/\r/g, '');
    const lines = t.split('\n');
    let code = null;
    await check('[E] cut usable() and the decorate() button loop from ht9045_startcondition_c.js', () => {
      const u = lines.findIndex(l => /^  function usable\(el\) \{/.test(l));
      ok(u >= 0, 'usable() not found');
      let ue = u; while (ue < lines.length && lines[ue] !== '  }') ue++;
      const s = lines.findIndex(l => /^    \[\['btnSetOffsetLimit'/.test(l));
      ok(s >= 0, 'button loop not found');
      let e = s; while (e < lines.length && lines[e] !== '    });') e++;
      ok(e < lines.length, 'end of the loop not found');
      code = lines.slice(u, ue + 1).join('\n') + '\n' + lines.slice(s, e + 1).join('\n') + '\n';
    });
    if (code) {
      function runSc(disabled) {
        const els = { btnSetOffsetLimit: makeEl('btnSetOffsetLimit', 'BUTTON'), sb_Maintenance_SmartDiagnosticFunction: makeEl('sb_Maintenance_SmartDiagnosticFunction', 'BUTTON') };
        els.sb_Maintenance_SmartDiagnosticFunction.disabled = !!disabled;
        const posted = [];
        const win = { getComputedStyle: () => ({ visibility: '', display: '' }) };
        win.parent = { postMessage: m => posted.push(m) };
        const ctx = vm.createContext({ window: win, $: id => els[id] || null, Object, String });
        vm.runInContext(code, ctx);
        return { els, posted };
      }
      const r = runSc(false);
      r.els.sb_Maintenance_SmartDiagnosticFunction.click();
      await check('[E] Smart Diagnostic (golden 906 :1080-1084 / V912 :1254-1258 fSmartDiagnostic->ShowModal) -> postMessage({open:"smartdiag"}); not disabled', () => {
        ok(!r.els.sb_Maintenance_SmartDiagnosticFunction.disabled, 'disabled');
        ok(r.posted.length === 1 && r.posted[0].open === 'smartdiag', JSON.stringify(r.posted));
      });
      await check('[E] btnSetOffsetLimit stays disabled (customer-only, not ported)', () => { ok(r.els.btnSetOffsetLimit.disabled === true, 'enabled'); });
      const r2 = runSc(true);
      r2.els.sb_Maintenance_SmartDiagnosticFunction.click();
      await check('[E] an unusable Smart Diagnostic button posts nothing', () => { ok(r2.posted.length === 0, JSON.stringify(r2.posted)); });
    }
  }

  console.log('\nE022_ContactCTPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
