// AI(W906-B8-CT3A) 20261001 [W906] (St01): B8 CT-3a, the page half -- D:\HT9045\web\page\ht9045_contact_ev.js on Setup.Contact
//   (T.Start / T.Step / One Cycle and the 11 mode radios). Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3";
//   the C++ half is ctest B8_Ct3a_ContactFlags (tests/test_b8_ct3a_contactflags.cpp).
//   golden V912 cContact.cpp: btnTStart / btnTStep are DFM Visible=False (cContact.dfm:16136 / :16146), FormShow :1630-1631 shows them with [D16]
//   (SOFT_SIMULTE :1664-1665); ledOneCycle->Value=bContinueContact (:17155) -- C++ carries that value on the proxy Tag.
//   How: the whole ht9045_contact_ev.js in a node vm with a fake window / document / HT9045Recipe (editlistGet / editlistSave / rawCmd):
//     [1] compiles, one EOL style, no BOM
//     [2] editlist.get: proxies.btnTStart / btnTStep visible:true => the display:none buttons are shown (ct3aLoad); visible:false => stay hidden;
//         proxies.ledOneCycle.tag 1 => the LED gets class "on"
//     [3] a click on T.Start sends one form.event {TfContact, btnTStart, click} on tag DeviceForm_File
//     [4] the ack's changed.ledOneCycle.tag 0 / 1 turns the LED off / on (applyChanged)
//     [5] a mode radio (label wrapping one <input type=radio>) sends {control: rbAutoHeight, event: click}; the ack's changed turns the other
//         radio off (C++ does VCL TurnSiblingsOff; the page's radios have no name attribute)
//     [6] operable=false => a click sends nothing
//   Control: W906_CT3A_PAGE_DIR -> the page before CT-3a (git show HEAD:web/page/ht9045_contact_ev.js) must be red ([2] / [4]).
//   No wb_serve, no machine file (reads one .js under web\page). Use: only through ctest (B8_Ct3a_ContactPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_CT3A_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const FILE = path.join(PAGE_DIR, 'ht9045_contact_ev.js');

let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async () => { for (let i = 0; i < 6; i++) await new Promise(r => setImmediate(r)); };

function makeClassList() {
  const s = new Set();
  return {
    contains: c => s.has(c),
    add: c => { s.add(c); },
    remove: c => { s.delete(c); },
    toggle: (c, on) => { const v = on === undefined ? !s.has(c) : !!on; if (v) s.add(c); else s.delete(c); return v; },
    has: c => s.has(c),
  };
}
function makeEl(id, tag, opts) {
  const attrs = {};
  const listeners = {};
  const el = {
    id, tagName: tag, nodeType: 1, disabled: false, title: '', style: {}, parentElement: null, children: [],
    classList: makeClassList(),
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    removeAttribute: k => { delete attrs[k]; },
    querySelector: () => null,
    querySelectorAll: () => [],
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    fire: t => { (listeners[t] || []).forEach(fn => fn({ isTrusted: true, type: t })); },
    listenerCount: t => (listeners[t] || []).length,
  };
  if (opts && opts.radio) {                                     // <label class="ckb" id=…><input type="radio">…</label>
    const input = makeEl('', 'INPUT');
    input.type = 'radio'; input.checked = !!opts.checked; input.parentElement = el;
    el.input = input;
    el.children = [input];
    el.querySelector = sel => (/input/.test(sel) && /radio/.test(sel) ? input : null);
    el.querySelectorAll = sel => (/input/.test(sel) && /radio/.test(sel) ? [input] : []);
  }
  return el;
}

function makeEnv(events, proxies) {
  const els = {
    btnTStart: makeEl('btnTStart', 'BUTTON'), btnTStep: makeEl('btnTStep', 'BUTTON'), spbOneCycle: makeEl('spbOneCycle', 'BUTTON'),
    ledOneCycle: makeEl('ledOneCycle', 'SPAN'),
    rbModeNormal: makeEl('rbModeNormal', 'LABEL', { radio: true, checked: true }),
    rbAutoHeight: makeEl('rbAutoHeight', 'LABEL', { radio: true, checked: false }),
  };
  els.btnTStart.style.display = 'none';                          // Setup.Contact.html: display:none (DFM Visible=False)
  els.btnTStep.style.display = 'none';
  els.ledOneCycle.classList.add('aled');
  const said = [], sent = [];
  let reply = null;
  const R = {
    editlistGet: st => Promise.resolve({ struct: st, events, proxies }),
    editlistSave: () => Promise.resolve({ ok: true }),
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, tag: extra && extra.tag, value: extra && extra.value });
      return new Promise(res => { reply = res; });
    },
  };
  const win = {
    HT9045Recipe: R,
    HT9045Wire: { say: (msg, colour) => { said.push({ msg, colour }); } },
    getComputedStyle: e => ({ display: (e && e.style && e.style.display) || '', visibility: (e && e.style && e.style.visibility) || '' }),
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

const EV = {
  btnTStart: { event: 'click', operable: true, golden: 'cContact.cpp:2280 TfContact::btnTStartClick' },
  btnTStep: { event: 'click', operable: true, golden: 'cContact.cpp:2285 TfContact::btnTStepClick' },
  spbOneCycle: { event: 'click', operable: true, golden: 'cContact.cpp:17152 TfContact::spbOneCycleClick' },
  rbModeNormal: { event: 'click', operable: true, golden: 'cContact.cpp:15555 TfContact::rbModeNormalClick' },
  rbAutoHeight: { event: 'click', operable: true, golden: 'cContact.cpp:15555 TfContact::rbModeNormalClick' },
};

(async () => {
  let text = '', bytes = null;
  await check('[1] read ' + FILE, () => { bytes = fs.readFileSync(FILE); text = bytes.toString('utf8'); });
  await check('[1] no BOM, one EOL style', () => {
    ok(!(bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (text.match(/\r\n/g) || []).length, lf = (text.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  let script = null;
  await check('[1] compiles', () => { script = new vm.Script(text, { filename: 'ht9045_contact_ev.js' }); });
  if (!script) { console.log('B8_Ct3a_ContactPage: ' + passed + ' passed, ' + (failed || 1) + ' failed'); process.exit(1); }

  {
    const env = makeEnv(EV, { btnTStart: { visible: true, enabled: true }, btnTStep: { visible: true, enabled: true },
                              spbOneCycle: { visible: true, enabled: true }, ledOneCycle: { visible: true, enabled: true, tag: 1 } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    await check('[2] proxies visible:true => T.Start / T.Step shown (display:none cleared, golden FormShow :1630-1631 / SOFT_SIMULTE :1664-1665)', () => {
      ok(env.els.btnTStart.style.display === '' && env.els.btnTStep.style.display === '', 'display ' + env.els.btnTStart.style.display + ' / ' + env.els.btnTStep.style.display);
    });
    await check('[2] proxies.ledOneCycle.tag 1 => LED on', () => { ok(env.els.ledOneCycle.classList.has('on'), 'LED off'); });
    env.els.btnTStart.fire('click');
    await flush();
    await check('[3] T.Start click => one form.event {TfContact, btnTStart, click} on tag DeviceForm_File', () => {
      ok(env.sent.length === 1, 'sent ' + env.sent.length);
      const s = env.sent[0], v = JSON.parse(s.value);
      ok(s.cmd === 'form.event' && s.tag === 'DeviceForm_File', 'cmd/tag ' + s.cmd + ' ' + s.tag);
      ok(v.form === 'TfContact' && v.control === 'btnTStart' && v.event === 'click', 'value ' + s.value);
    });
    env.answer({ changed: {}, messages: [], todo: [] });
    await flush();
    env.els.spbOneCycle.fire('click');
    await flush();
    env.answer({ changed: { ledOneCycle: { tag: 0 } }, messages: [], todo: [] });
    await flush();
    await check('[4] ack changed.ledOneCycle.tag 0 => LED off', () => {
      ok(env.sent.length === 2 && JSON.parse(env.sent[1].value).control === 'spbOneCycle', 'sent ' + env.sent.length);
      ok(!env.els.ledOneCycle.classList.has('on'), 'LED still on');
    });
    env.els.spbOneCycle.fire('click');
    await flush();
    env.answer({ changed: { ledOneCycle: { tag: 1 } }, messages: [], todo: [] });
    await flush();
    await check('[4] ack changed.ledOneCycle.tag 1 => LED on', () => { ok(env.els.ledOneCycle.classList.has('on'), 'LED off'); });
    env.els.rbAutoHeight.input.checked = true;                   // the user ticks it (the browser does not untick rbModeNormal: no name attribute)
    env.els.rbAutoHeight.fire('change');
    await flush();
    await check('[5] a mode radio => one form.event {TfContact, rbAutoHeight, click}', () => {
      ok(env.sent.length === 4, 'sent ' + env.sent.length);
      const v = JSON.parse(env.sent[3].value);
      ok(v.form === 'TfContact' && v.control === 'rbAutoHeight' && v.event === 'click', 'value ' + env.sent[3].value);
    });
    env.answer({ changed: { rbAutoHeight: { checked: true }, rbModeNormal: { checked: false } }, messages: [], todo: [] });
    await flush();
    await check('[5] the ack turns the previous radio off (C++ TurnSiblingsOff)', () => {
      ok(env.els.rbAutoHeight.input.checked === true && env.els.rbModeNormal.input.checked === false,
         'rbAutoHeight ' + env.els.rbAutoHeight.input.checked + ' rbModeNormal ' + env.els.rbModeNormal.input.checked);
    });
  }
  {
    const ev = JSON.parse(JSON.stringify(EV));
    ev.btnTStart.operable = false; ev.btnTStep.operable = false;
    const env = makeEnv(ev, { btnTStart: { visible: false, enabled: true }, btnTStep: { visible: false, enabled: true }, ledOneCycle: { visible: true, enabled: true, tag: 0 } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    await check('[2] proxies visible:false ([D16] off, ship build) => T.Start / T.Step stay hidden; LED off', () => {
      ok(env.els.btnTStart.style.display === 'none' && env.els.btnTStep.style.display === 'none', 'display ' + env.els.btnTStart.style.display);
      ok(!env.els.ledOneCycle.classList.has('on'), 'LED on');
    });
    env.els.btnTStart.fire('click');
    await flush();
    await check('[6] operable=false => a click sends nothing', () => { ok(env.sent.length === 0, 'sent ' + env.sent.length); });
  }
  console.log('B8_Ct3a_ContactPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
