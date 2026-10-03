// AI(W906-ST02-OB7) 20261002 (St02-E helper): E-019 OB-7, the page half (node, offline) -- ctest St02_ObserverSGJamPage.
//   golden 906_0625 D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven\cObserver.cpp: btnSG_QueryNowClick :5361-5364,
//   btnSG_QueryYesterdayClick :5366-5369 -> StatisticalJamCount :5060-5276; constructor grid header :305-317; dfm
//   cObserver.dfm:1839-1913 (tsSGJamCount, labLoaderCount Caption 'labLoaderCount' :1857).  The C++ half is ctest St02_ObserverSGJam.
//   [A] D:\HT9045\web\page\ht9045_observer_sgjam.js in a node vm with a fake DOM / HT9045Recipe (+ the real ht9045_busy_util.js):
//       compiles, one EOL style, no BOM; at load the grid has golden's constructor shape; Query Now / Query Yesterday send
//       control.acquire -> act.observerSG.<op> -> control.release; the reply's grid and Loader Count are drawn (an empty Caption keeps
//       the dfm text); eventlog-missing / initial-not-ok / not-open / an old wb_serve / busy: on the status line; a second click
//       inside the cool-down is not sent; the SG_JamCount tab sends a quiet state after STATE_DELAY_MS; a click while that state is
//       in flight is sent after it
//   [C] Data.Observer.html loads ht9045_observer_sgjam.js after ht9045_observer_ev.js (outside comments)
//   Control: W906_ST02_OB7_PAGE_DIR -> a page directory without the new file must be red.
//   No wb_serve, no machine file (reads .js / .html under web\page).  Use: only through ctest (St02_ObserverSGJamPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_ST02_OB7_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async (ms) => { if (ms) await new Promise(r => setTimeout(r, ms)); for (let i = 0; i < 12; i++) await new Promise(r => setImmediate(r)); };
function read(f) { return fs.readFileSync(path.join(PAGE_DIR, f)); }

function makeEl(id, tag) {
  const attrs = {}, listeners = {};
  const el = {
    id, tagName: tag || 'DIV', disabled: false, title: '', style: {}, textContent: '', innerHTML: '',
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    listeners: t => (listeners[t] || []).length,
    fire: (t, ev) => { (listeners[t] || []).forEach(fn => fn(Object.assign({ type: t, preventDefault() {} }, ev || {}))); },
    click: () => el.fire('click'),
  };
  return el;
}

function grid(rows) {   // rows: array of 6-cell arrays after the header
  const cells = [['No', 'UnitName', 'AlarmCode', 'Message', 'Count', 'Rate (%)']].concat(rows);
  return { rows: cells.length, cols: 6, fixedRows: 1, fixedCols: 0, truncated: false, cells };
}
const G_NOW = grid([['1', 'InArm', 'JAM0301', 'Loader, jam near site A', '2', '2.00'], ['2', 'Shuttle', 'JAM0302', 'Shuttle <jam>', '1', '1.00'], ['', '', '', '', '', '']]);
const G_YDAY = grid([['1', 'OutArm', 'JAM1901', 'Out arm jam', '1', '1.00'], ['', '', '', '', '', '']]);

function makeEnv(script, busyScript) {
  const els = {};
  const add = (id, tag) => { els[id] = makeEl(id, tag); return els[id]; };
  add('btnSG_QueryNow', 'BUTTON'); add('btnSG_QueryYesterday', 'BUTTON'); add('obsStatus'); add('strngrdJamLog', 'DIV');
  add('labLoaderCount', 'SPAN').textContent = 'labLoaderCount';
  const tab3 = makeEl('', 'DIV'); tab3.setAttribute('data-t', '3');
  const sent = [], replies = [];
  const R = {
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, value: extra && extra.value ? JSON.parse(extra.value) : undefined });
      if (cmd === 'control.acquire' || cmd === 'control.release') return Promise.resolve({});
      const r = replies.shift() || { ok: false, v: { executed: false, guard: 'test-no-reply' } };
      if (r.defer) return new Promise(res => { r.release = () => res(r.v); });
      if (r.busy) return Promise.reject(new Error('busy: same command in progress or just done (' + cmd + ', 10 ms ago)'));
      return r.ok ? Promise.resolve(r.v) : Promise.reject(new Error(JSON.stringify(r.v)));
    },
  };
  const doc = {
    readyState: 'complete',
    getElementById: id => els[id] || null,
    addEventListener: () => {},
    querySelector: sel => (sel === '#pgcMessage > .pcTabs > .tab[data-t="3"]' ? tab3 : null),
    querySelectorAll: () => [],
  };
  const win = vm.createContext({
    HT9045Recipe: R, console: { info() {}, log() {}, error() {} },
    document: doc, setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', win);
  if (busyScript) busyScript.runInContext(win);
  script.runInContext(win);
  return { win, els, sent, replies, tab3 };
}
const acts = env => env.sent.filter(s => s.cmd !== 'control.acquire' && s.cmd !== 'control.release');
const trs = html => (html.match(/<tr>/g) || []).length;

(async () => {
  let text = '', bytes = null, script = null, busyScript = null;
  await check('[A] read ht9045_observer_sgjam.js', () => { bytes = read('ht9045_observer_sgjam.js'); text = bytes.toString('utf8'); });
  await check('[A] no BOM, one EOL style', () => {
    ok(bytes, 'not read');
    ok(!(bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  await check('[A] compiles', () => { ok(bytes, 'not read'); script = new vm.Script(text, { filename: 'ht9045_observer_sgjam.js' }); });
  await check('[A] ht9045_busy_util.js compiles', () => { busyScript = new vm.Script(read('ht9045_busy_util.js').toString('utf8'), { filename: 'ht9045_busy_util.js' }); });

  if (script) {
    // A1 load + Query Now
    {
      const env = makeEnv(script, busyScript);
      await check('[A1] at load: golden constructor grid (header No..Rate (%), 6 columns, 5 rows)', () => {
        const h = env.els.strngrdJamLog.innerHTML;
        ok(trs(h) === 5 && (h.match(/<col /g) || []).length === 6, 'rows/cols ' + h);
        ok(/<th>No<\/th><th>UnitName<\/th><th>AlarmCode<\/th><th>Message<\/th><th>Count<\/th><th>Rate \(%\)<\/th>/.test(h), 'header ' + h);
        ok(/width:400px/.test(h), 'golden ColWidths[3]=400 (:309)');
      });
      await check('[A1] one click listener per button, one on the SG_JamCount tab', () => {
        ok(env.els.btnSG_QueryNow.listeners('click') === 1 && env.els.btnSG_QueryYesterday.listeners('click') === 1 && env.tab3.listeners('click') === 1, 'listeners');
      });
      env.replies.push({ ok: true, v: { executed: true, op: 'queryNow', early: '', grid: G_NOW, labLoaderCount: '100', iOneDayLoaderCount: 100, skipped: [] } });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A1] Query Now: control.acquire -> act.observerSG.queryNow {} -> control.release', () => {
        ok(env.sent.length === 3 && env.sent[0].cmd === 'control.acquire' && env.sent[1].cmd === 'act.observerSG.queryNow' &&
           env.sent[2].cmd === 'control.release' && JSON.stringify(env.sent[1].value) === '{}', JSON.stringify(env.sent));
      });
      await check('[A1] the reply is drawn: grid (escaped), Loader Count, status line', () => {
        const h = env.els.strngrdJamLog.innerHTML;
        ok(trs(h) === 4 && /<td>JAM0301<\/td>/.test(h) && /<td>Shuttle &lt;jam&gt;<\/td>/.test(h) && /<td>2\.00<\/td>/.test(h), h);
        ok(env.els.labLoaderCount.textContent === '100', env.els.labLoaderCount.textContent);
        ok(/^SG_JamCount：Query Now ✓ 2 種/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
      });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A1] a second click inside the cool-down is not sent', () => {
        ok(acts(env).length === 1, JSON.stringify(acts(env)));
        ok(/略過/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
      });
    }
    // A2 Query Yesterday, eventlog-missing
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: true, v: { executed: true, op: 'queryYesterday', early: 'eventlog-missing', eventLogCsv: { path: 'X:\\EventLogTxt_20261001.csv', exists: false }, grid: G_YDAY, labLoaderCount: '7', skipped: [] } });
      env.els.btnSG_QueryYesterday.click();
      await flush();
      await check('[A2] Query Yesterday -> act.observerSG.queryYesterday; eventlog-missing on the status line (orange, not an error box)', () => {
        ok(acts(env).length === 1 && acts(env)[0].cmd === 'act.observerSG.queryYesterday', JSON.stringify(acts(env)));
        ok(/沒有事件記錄檔 X:\\EventLogTxt_20261001\.csv/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '#c60', env.els.obsStatus.textContent);
        ok(/<td>JAM1901<\/td>/.test(env.els.strngrdJamLog.innerHTML), 'grid from the reply');
      });
    }
    // A3 initial-not-ok, empty Caption keeps the dfm text
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: true, v: { executed: true, op: 'queryNow', early: 'initial-not-ok', grid: grid([['', '', '', '', '', ''], ['', '', '', '', '', ''], ['', '', '', '', '', '']]), labLoaderCount: '' } });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A3] initial-not-ok on the status line; an empty Caption keeps the dfm text labLoaderCount', () => {
        ok(/InitialOK==false/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
        ok(env.els.labLoaderCount.textContent === 'labLoaderCount', env.els.labLoaderCount.textContent);
      });
    }
    // A4 refusals: not-open, old wb_serve, busy
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: false, v: { executed: false, guard: 'not-open', detail: 'the Observer is not open' } });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A4] not-open -> red status, grid untouched', () => {
        ok(/Observer 視窗還沒開好/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '#c00', env.els.obsStatus.textContent);
        ok(trs(env.els.strngrdJamLog.innerHTML) === 5, 'grid redrawn on a refusal');
      });
    }
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: false, v: { executed: false, guard: 'unknown-action', detail: '目前只有 act.main.clarnData、act.main.stateRecord…' } });
      env.els.btnSG_QueryYesterday.click();
      await flush();
      await check('[A4] an old wb_serve without act.observerSG.* -> says so', () => {
        ok(/還沒有 act\.observerSG\.\*/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
      });
    }
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ busy: true });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A4] busy: is not a failure (HT9045Busy.NOTE, normal colour)', () => {
        ok(/略過/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '', env.els.obsStatus.textContent + ' / ' + env.els.obsStatus.style.color);
      });
    }
    // A5 the SG_JamCount tab -> quiet state after the delay; a click while it runs goes after it
    {
      const env = makeEnv(script, busyScript);
      const st = { defer: true, v: { executed: true, op: 'state', guard: '', grid: G_NOW, labLoaderCount: '100' } };
      env.replies.push(st);
      env.replies.push({ ok: true, v: { executed: true, op: 'queryNow', early: '', grid: G_YDAY, labLoaderCount: '0', skipped: [] } });
      env.tab3.click();
      await flush();
      await check('[A5] the tab click sends nothing at once (the machine\'s msgTab goes first)', () => { ok(env.sent.length === 0, JSON.stringify(env.sent)); });
      await flush(380);
      await check('[A5] after STATE_DELAY_MS: act.observerSG.state, quiet (status line untouched)', () => {
        ok(acts(env).length === 1 && acts(env)[0].cmd === 'act.observerSG.state', JSON.stringify(acts(env)));
        ok(env.els.obsStatus.textContent === '', env.els.obsStatus.textContent);
      });
      env.els.btnSG_QueryNow.click();
      await flush();
      await check('[A5] Query Now while the state read is in flight is held, not dropped', () => { ok(acts(env).length === 1, JSON.stringify(acts(env))); });
      st.release();
      await flush();
      await check('[A5] ... and sent right after it; both replies drawn in order', () => {
        ok(acts(env).length === 2 && acts(env)[1].cmd === 'act.observerSG.queryNow', JSON.stringify(acts(env)));
        ok(/<td>JAM1901<\/td>/.test(env.els.strngrdJamLog.innerHTML) && env.els.labLoaderCount.textContent === '0', env.els.strngrdJamLog.innerHTML);
        ok(env.win.HT9045ObserverSG.state().states === 1, 'state count ' + env.win.HT9045ObserverSG.state().states);
      });
    }
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: false, v: { executed: false, guard: 'not-open', detail: 'the Observer is not open' } });
      env.tab3.click();
      await flush(380);
      await check('[A5] a refused state read stays quiet', () => {
        ok(acts(env).length === 1 && env.els.obsStatus.textContent === '', env.els.obsStatus.textContent);
        ok(/not-open/.test(env.win.HT9045ObserverSG.state().lastError) || /Observer/.test(env.win.HT9045ObserverSG.state().lastError), env.win.HT9045ObserverSG.state().lastError);
      });
    }
  }

  // ---- [C] Data.Observer.html -------------------------------------------------------------------------------------------------
  await check('[C] Data.Observer.html loads ht9045_observer_sgjam.js after ht9045_observer_ev.js (outside comments)', () => {
    const html = read('Data.Observer.html').toString('utf8').replace(/<!--[\s\S]*?-->/g, '');
    const a = html.indexOf('<script src="ht9045_observer_ev.js"></script>'), b = html.indexOf('<script src="ht9045_observer_sgjam.js"></script>');
    ok(a >= 0 && b > a, 'ev ' + a + ' sgjam ' + b);
  });

  console.log('RESULT: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
