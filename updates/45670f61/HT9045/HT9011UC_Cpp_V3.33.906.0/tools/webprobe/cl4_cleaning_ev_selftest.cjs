// AI(W906-B8-CL4) 20261001 [W906] (St01): B8 CL-4 的頁面那一半 —— D:\HT9045\web\page\ht9045_cleaning_ev.js 的「Clean」鈕（btnStartAutoClean）。
//   派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CL-4」；C++ 那一半 ctest B8_Cl4_StartAutoClean（tests/test_b8_cl4_autoclean.cpp）。
//   golden 906 AutoClean\uCleaning.cpp:2869-2872（V912 :2901-2904） btnStartAutoCleanClick → cShowBinSelect.cpp:2101-2166（V912 :2248-2315） btnAutoCleanClick（AI(W906-E030-CITE) 20261003；下面 golden 字串 AutoClean/uCleaning.cpp:2901 是產生檔 kCL_Events 的 V912 標籤，照它不改）。
//
//   做法：把整支 ht9045_cleaning_ev.js 原樣放進 node vm，給假的 window／document／HT9045Recipe（editlistGet／editlistSave／rawCmd）／HT9045Wire.say：
//     [1] 編得過、不混 EOL、無 BOM
//     [2] editlist.get 說 events.btnStartAutoClean operable ⇒ ht9045_cleaning_c.js 停用的鈕被打開（PORTED），title 寫 CL-4
//     [3] 點一下 ⇒ 送一則 form.event：tag TestIF_File_Cleaning、value {form:TfCleaning, control:btnStartAutoClean, event:click}
//     [4] ack 沒有 messages ⇒ 狀態列說明「按 START 之後才會動」（綠）；[5] ack 有 golden 擋下訊息 ⇒ 狀態列帶那一句（紅）
//     [6] 回覆前再點一下＝連點，不送第二次
//     [7] operable=false ⇒ 不打開、點了不送
//   對照組：W906_CL4_PAGE_DIR 指向修改前的頁面（git show HEAD:web/page/ht9045_cleaning_ev.js）必須紅。
//   不連 wb_serve、不讀寫機台檔（只讀 web\page 一個 .js）。用法：只經 ctest（B8_Cl4_CleaningPage）。
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_CL4_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const FILE = path.join(PAGE_DIR, 'ht9045_cleaning_ev.js');

let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async () => { for (let i = 0; i < 6; i++) await new Promise(r => setImmediate(r)); };

// ---- 假的 DOM（只做這支 js 在按鈕這條路上用到的）
function makeEl(id, tag) {
  const attrs = {};
  const listeners = {};
  return {
    id, tagName: tag, nodeType: 1, disabled: false, title: '', style: {}, parentElement: null, children: [],
    classList: { contains: () => false },
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    removeAttribute: k => { delete attrs[k]; },
    querySelector: () => null,
    querySelectorAll: () => [],
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    fire: t => { (listeners[t] || []).forEach(fn => fn({ isTrusted: true, type: t })); },
    listenerCount: t => (listeners[t] || []).length,
  };
}

function makeEnv(events) {
  const els = { btnStartAutoClean: makeEl('btnStartAutoClean', 'BUTTON') };
  els.btnStartAutoClean.disabled = true;                       // ht9045_cleaning_c.js markNotPorted 停用的樣子
  els.btnStartAutoClean.setAttribute('data-gb-dis', '1');
  const said = [], sent = [];
  let reply = null;                                            // 下一則 rawCmd 的回覆（Promise 由測試決定何時 resolve）
  const R = {
    editlistGet: st => Promise.resolve({ struct: st, events }),
    editlistSave: () => Promise.resolve({ ok: true }),
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, tag: extra && extra.tag, value: extra && extra.value });
      return new Promise(res => { reply = res; });
    },
  };
  const win = {
    HT9045Recipe: R,
    HT9045Wire: { say: (msg, colour) => { said.push({ msg, colour }); } },
    getComputedStyle: () => ({ display: '', visibility: '' }),
    console: { info() {}, error() {}, log() {} },
    localStorage: { setItem() {} },
    parent: null,
  };
  win.window = win;
  win.parent = win;
  const doc = {
    getElementById: id => els[id] || null,
    querySelector: () => null,
    querySelectorAll: () => [],
    createElement: t => makeEl('', String(t).toUpperCase()),
    body: { appendChild() {} },
  };
  win.document = doc;
  const ctx = vm.createContext({ window: win, document: doc, console: win.console, setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp,
                                 localStorage: win.localStorage, getComputedStyle: win.getComputedStyle, HT9045Wire: win.HT9045Wire });
  return { ctx, win, els, said, sent, R, answer: v => { const r = reply; reply = null; if (r) r({ value: JSON.stringify(v) }); } };
}

(async () => {
  let text = '', bytes = null;
  await check('[1] read ' + FILE, () => { bytes = fs.readFileSync(FILE); text = bytes.toString('utf8'); });
  await check('[1] no BOM, one EOL style', () => {
    ok(!(bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  let script = null;
  await check('[1] compiles', () => { script = new vm.Script(text, { filename: 'ht9045_cleaning_ev.js' }); });
  if (!script) { console.log('B8_Cl4_CleaningPage: ' + passed + ' passed, ' + (failed || 1) + ' failed'); process.exit(1); }

  // ---- 打得開、送得出
  {
    const env = makeEnv({ btnStartAutoClean: { event: 'click', operable: true, golden: 'AutoClean/uCleaning.cpp:2901 TfCleaning::btnStartAutoCleanClick' } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('TestIF_File_Cleaning');
    await new Promise(r => setTimeout(r, 5));
    const b = env.els.btnStartAutoClean;
    await check('[2] operable => the button is re-enabled (PORTED), title cites CL-4', () => {
      ok(b.disabled === false, 'still disabled');
      ok(/CL-4/.test(b.title) && /btnAutoCleanClick/.test(b.title), 'title: ' + b.title);
      ok(b.listenerCount('click') === 1, 'click listeners: ' + b.listenerCount('click'));
    });
    b.fire('click');
    await flush();
    await check('[3] one click => one form.event {TfCleaning, btnStartAutoClean, click} on tag TestIF_File_Cleaning', () => {
      ok(env.sent.length === 1, 'sent ' + env.sent.length);
      const s = env.sent[0], v = JSON.parse(s.value);
      ok(s.cmd === 'form.event' && s.tag === 'TestIF_File_Cleaning', 'cmd/tag ' + s.cmd + ' ' + s.tag);
      ok(v.form === 'TfCleaning' && v.control === 'btnStartAutoClean' && v.event === 'click', 'value ' + s.value);
    });
    b.fire('click');
    await flush();
    await check('[6] a second click before the reply is a double click: not sent', () => { ok(env.sent.length === 1, 'sent ' + env.sent.length); });
    env.answer({ changed: {}, messages: [], todo: [] });
    await flush();
    await check('[4] ack without messages => status line: golden ran, the machine moves only after START (green)', () => {
      const last = env.said[env.said.length - 1] || {};
      ok(/Start Auto Clean/.test(last.msg || '') && /START/.test(last.msg) && /btnAutoCleanClick/.test(last.msg), 'said: ' + last.msg);
      ok(last.colour === '#9f9', 'colour ' + last.colour);
    });
    b.fire('click');
    await flush();
    env.answer({ changed: {}, messages: [{ en: 'Auto Clean Need Home', zh: 'Auto Clean 需要歸零' }], todo: [] });
    await flush();
    await check('[5] ack with golden\'s refusal => status line carries it (red)', () => {
      ok(env.sent.length === 2, 'sent ' + env.sent.length);
      const last = env.said[env.said.length - 1] || {};
      ok(/golden 擋下/.test(last.msg || '') && /需要歸零/.test(last.msg), 'said: ' + last.msg);
      ok(last.colour === '#f88', 'colour ' + last.colour);
    });
  }
  // ---- 點不到
  {
    const env = makeEnv({ btnStartAutoClean: { event: 'click', operable: false } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('TestIF_File_Cleaning');
    await new Promise(r => setTimeout(r, 5));
    const b = env.els.btnStartAutoClean;
    b.fire('click');
    await flush();
    await check('[7] operable=false => stays disabled, a click sends nothing', () => {
      ok(b.disabled === true, 'enabled');
      ok(env.sent.length === 0, 'sent ' + env.sent.length);
    });
  }
  console.log('B8_Cl4_CleaningPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
