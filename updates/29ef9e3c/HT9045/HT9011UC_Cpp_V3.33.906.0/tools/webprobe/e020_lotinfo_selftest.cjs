// AI(W906-E020-LI11) 20261002 [W906] (St01): todo E-020 LI-11 / LI-13, the page half (node, offline).
//   golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618\uLotInfo.cpp [AI(W906-E032) 20261003] (AI(W906-E030-CITE) 20261003; V912 in [ ]): btChangeFileClick :10213-10237 [10394-10418],
//   palSecsGemMouseDown :10351-10425 [10532-10606]. The C++ half is ctest E020_LotInfoActs (tests/test_e020_lotinfo_e020.cpp).
//   [A] D:\HT9045\web\page\ht9045_lotinfo_e020.js in a node vm with a fake DOM / HT9045Recipe (+ the real ht9045_busy_util.js):
//       compiles, one EOL style, no BOM; Change File = control.acquire -> act.lotInfo.changeFile {} -> control.release; executed /
//       gated (whyNot "GATE (W906-E020-LI11-n)") / busy / old wb_serve on #barcodeStatus; a second click while busy or cooling sends
//       nothing; a disabled button sends nothing
//   [B] palSecsGem: every mouse-down is sent, in order, never dropped (six fast presses = six commands, seq 1..6, buttons
//       left/left/right/right/left/left); middle is sent; a press on a child control (the Lot End button) is not; the context menu
//       is suppressed on the panel only
//   [C] Data.LotInfo.html: loads ht9045_lotinfo_e020.js after ht9045_recipe_client.js / ht9045_busy_util.js / ht9045_lotinfo_wire.js
//       (outside comments); btChangeFile is no longer disabled; div#palSecsGem carries data-e020="palSecsGemMouseDown"
//   Control: W906_E020_PAGE_DIR -> the pre-change page directory must be red.
//   No wb_serve, no machine file (reads .js / .html under web\page).  Use: only through ctest (E020_LotInfoPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_E020_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async (ms) => { if (ms) await new Promise(r => setTimeout(r, ms)); for (let i = 0; i < 40; i++) await new Promise(r => setImmediate(r)); };
function read(f) { return fs.readFileSync(path.join(PAGE_DIR, f)); }
// wait until cond() holds (ctest may run this beside a full build: timers are slow then), at most ms
async function waitFor(cond, ms) { const end = Date.now() + (ms || 5000); while (!cond() && Date.now() < end) await flush(10); await flush(); }

function makeEl(id, tag) {
  const listeners = {};
  const el = {
    id, tagName: tag || 'DIV', disabled: false, style: {}, textContent: '',
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    listeners: t => (listeners[t] || []).length,
    fire: (t, ev) => { let prevented = false; (listeners[t] || []).forEach(fn => fn(Object.assign({ type: t, preventDefault() { prevented = true; } }, ev || {}))); return prevented; },
    click: () => el.fire('click'),
  };
  return el;
}

function makeEnv(script, busyScript, opts) {
  const els = {};
  ['btChangeFile', 'barcodeStatus', 'palSecsGem'].forEach(id => { els[id] = makeEl(id, id === 'btChangeFile' ? 'BUTTON' : 'DIV'); });
  const sent = [], replies = [];
  const delayMs = (opts && opts.delayMs) || 0;
  const R = {
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, value: extra && extra.value ? JSON.parse(extra.value) : undefined });
      if (cmd === 'control.acquire' || cmd === 'control.release') return Promise.resolve({});
      const r = replies.shift() || { ok: true, v: { executed: true, op: 'palSecsGemMouseDown', filled: false } };
      const out = () => {
        if (r.busy) return Promise.reject(new Error('busy: same command in progress or just done (' + cmd + ', 10 ms ago)'));
        return r.ok ? Promise.resolve(r.v) : Promise.reject(new Error(JSON.stringify(r.v)));
      };
      return delayMs ? new Promise(res => setTimeout(res, delayMs)).then(out) : out();
    },
  };
  const doc = { readyState: 'complete', getElementById: id => els[id] || null, addEventListener() {} };
  const win = vm.createContext({
    HT9045Recipe: R, document: doc, console: { info() {}, log() {}, error() {} },
    setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', win);
  if (busyScript) busyScript.runInContext(win);
  script.runInContext(win);
  return { win, els, sent, replies };
}
const acts = env => env.sent.filter(s => s.cmd !== 'control.acquire' && s.cmd !== 'control.release');
const target = (isControl) => ({ closest: s => (isControl && /button/.test(s) ? { tagName: 'BUTTON' } : null) });

(async () => {
  let text = '', bytes = null, script = null, busyScript = null;
  await check('[A] read ht9045_lotinfo_e020.js', () => { bytes = read('ht9045_lotinfo_e020.js'); text = bytes.toString('utf8'); });
  await check('[A] no BOM, one EOL style', () => {
    ok(bytes, 'not read');
    ok(!(bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  await check('[A] compiles', () => { ok(bytes, 'not read'); script = new vm.Script(text, { filename: 'ht9045_lotinfo_e020.js' }); });
  await check('[A] ht9045_busy_util.js compiles', () => { busyScript = new vm.Script(read('ht9045_busy_util.js').toString('utf8'), { filename: 'ht9045_busy_util.js' }); });

  if (script) {
    {
      const env = makeEnv(script, busyScript);
      env.replies.push({ ok: true, v: { executed: true, op: 'changeFile', branch: 0, result: 'TestIF_File.bEnableBarCode is off (:10215, V912 :10396): golden does nothing' } });
      env.els.btChangeFile.click();
      await flush();
      await check('[A1] Change File: control.acquire -> act.lotInfo.changeFile {} -> control.release', () => {
        ok(env.sent.length === 3 && env.sent[0].cmd === 'control.acquire' && env.sent[1].cmd === 'act.lotInfo.changeFile' && env.sent[2].cmd === 'control.release', JSON.stringify(env.sent));
        ok(JSON.stringify(env.sent[1].value) === '{}', JSON.stringify(env.sent[1].value));
      });
      await check('[A1] executed -> the result on #barcodeStatus (normal colour)', () => {
        ok(/golden does nothing/.test(env.els.barcodeStatus.textContent) && env.els.barcodeStatus.style.color === '', env.els.barcodeStatus.textContent);
      });
      env.els.btChangeFile.click();
      await flush();
      await check('[A1] a click inside the 400 ms cooling sends nothing', () => { ok(acts(env).length === 1, JSON.stringify(acts(env))); });
      await flush(450);
      env.replies.push({ ok: true, v: { executed: false, op: 'changeFile', guard: 'gated', branch: 2, gate: 'W906-E020-LI11-2', whyNot: 'GATE (W906-E020-LI11-2): the Cognex EtherNet change-file chain is not ported; nothing done' } });
      env.els.btChangeFile.click();
      await flush();
      await check('[A2] gated -> whyNot "GATE (W906-E020-LI11-2)" in the error colour', () => {
        ok(/GATE \(W906-E020-LI11-2\)/.test(env.els.barcodeStatus.textContent) && env.els.barcodeStatus.style.color === '#b00', env.els.barcodeStatus.textContent + ' / ' + env.els.barcodeStatus.style.color);
        ok(env.win.HT9045LotInfoE020.state().lastError !== '', 'lastError empty');
      });
      await flush(450);
      env.replies.push({ busy: true });
      env.els.btChangeFile.click();
      await flush();
      await check('[A3] busy: -> the shared note, not an error', () => {
        ok(env.els.barcodeStatus.textContent === env.win.HT9045Busy.NOTE && env.els.barcodeStatus.style.color === '', env.els.barcodeStatus.textContent);
      });
      await flush(450);
      env.replies.push({ ok: false, v: { executed: false, guard: 'unknown-action', detail: 'unknown cmd' } });
      env.els.btChangeFile.click();
      await flush();
      await check('[A4] an old wb_serve without act.lotInfo.* -> says so, nothing executed', () => {
        ok(/還沒有 act\.lotInfo\.\*/.test(env.els.barcodeStatus.textContent), env.els.barcodeStatus.textContent);
      });
      await flush(450);
      const n = acts(env).length;
      env.els.btChangeFile.disabled = true;
      env.els.btChangeFile.click();
      await flush();
      await check('[A5] a disabled (hidden by tag) button sends nothing', () => { ok(acts(env).length === n, JSON.stringify(acts(env))); });
    }
    {
      const env = makeEnv(script, busyScript, { delayMs: 5 });
      const seq = [0, 0, 2, 2, 0, 0];                                           // L L R R L L (MouseEvent.button)
      seq.forEach(b => env.els.palSecsGem.fire('mousedown', { button: b, target: target(false) }));
      await waitFor(() => env.sent.length >= 18);
      const a = acts(env);
      await check('[B1] six fast presses -> six act.lotInfo.palSecsGemMouseDown, in order, seq 1..6', () => {
        ok(a.length === 6, a.length + ' sent: ' + JSON.stringify(a));
        const want = ['left', 'left', 'right', 'right', 'left', 'left'];
        a.forEach((s, i) => {
          ok(s.cmd === 'act.lotInfo.palSecsGemMouseDown', s.cmd);
          ok(s.value.button === want[i] && s.value.seq === i + 1, i + ': ' + JSON.stringify(s.value));
        });
      });
      await check('[B1] one at a time: acquire / act / release per press, never interleaved', () => {
        const cmds = env.sent.map(s => s.cmd);
        for (let i = 0; i < 6; i++) ok(cmds[i * 3] === 'control.acquire' && cmds[i * 3 + 1] === 'act.lotInfo.palSecsGemMouseDown' && cmds[i * 3 + 2] === 'control.release', JSON.stringify(cmds));
      });
      env.els.palSecsGem.fire('mousedown', { button: 1, target: target(false) });
      await waitFor(() => env.sent.length >= 21);
      await check('[B2] middle button is sent (golden: a non-matching button resets the sequence)', () => {
        const last = acts(env).slice(-1)[0];
        ok(acts(env).length === 7 && last.value.button === 'middle' && last.value.seq === 7, JSON.stringify(last));
      });
      env.els.palSecsGem.fire('mousedown', { button: 0, target: target(true) });
      await flush(60);
      await check('[B3] a press on a child control (Lot End button) is not the panel\'s mouse-down', () => { ok(acts(env).length === 7, JSON.stringify(acts(env).slice(-1))); });
      await check('[B4] the context menu is suppressed on the panel, not on a child control', () => {
        ok(env.els.palSecsGem.fire('contextmenu', { target: target(false) }) === true, 'panel: not prevented');
        ok(env.els.palSecsGem.fire('contextmenu', { target: target(true) }) === false, 'child control: prevented');
      });
      await check('[B5] the panel has one mousedown and one contextmenu listener', () => {
        ok(env.els.palSecsGem.listeners('mousedown') === 1 && env.els.palSecsGem.listeners('contextmenu') === 1, 'listeners');
      });
    }
  }

  await check('[C] Data.LotInfo.html loads ht9045_lotinfo_e020.js after the client, the busy util and ht9045_lotinfo_wire.js (outside comments)', () => {
    const html = read('Data.LotInfo.html').toString('utf8').replace(/<!--[\s\S]*?-->/g, '');
    const c = html.indexOf('<script src="ht9045_recipe_client.js"></script>'), b = html.indexOf('<script src="ht9045_busy_util.js"></script>');
    const w = html.indexOf('<script src="ht9045_lotinfo_wire.js"></script>'), e = html.indexOf('<script src="ht9045_lotinfo_e020.js"></script>');
    ok(c >= 0 && b >= 0 && w >= 0 && e > w && e > b && e > c, 'client ' + c + ', busy ' + b + ', wire ' + w + ', e020 ' + e);
  });
  await check('[C] btChangeFile is not disabled; div#palSecsGem carries data-e020="palSecsGemMouseDown"', () => {
    const html = read('Data.LotInfo.html').toString('utf8');
    const m = html.match(/<button[^>]*id="btChangeFile"[^>]*>/);
    ok(m && !/\sdisabled[\s>]/.test(m[0]), m ? m[0] : 'no btChangeFile');
    ok(/<div id="palSecsGem"[^>]*data-e020="palSecsGemMouseDown"/.test(html), 'palSecsGem tag');
  });

  console.log('\nE020_LotInfoPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
