// AI(W906-B8-CT3B) 20261001 [W906] (St01): B8 CT-3b', the page half -- D:\HT9045\web\page\ht9045_contact_ev.js on Setup.Contact
//   (START / PAUSE). Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3"; the C++ half is ctest
//   B8_Ct3b_ContactStartPause (tests/test_b8_ct3b_contactstartpause.cpp).
//   golden V912 cContact.cpp: btnStart / btnPause are DFM Visible=False (cContact.dfm:236-255), FormShow :1628-1629 shows them with [D16]
//   (SOFT_SIMULTE :1662-1663); btnStartClick :14072-14077 / btnPauseClick :14079-14082 run on the server after the reply (ack "afterAck").
//   How: the whole ht9045_contact_ev.js in a node vm with a fake window / document / HT9045Recipe (editlistGet / editlistSave / rawCmd):
//     [1] compiles, one EOL style, no BOM
//     [2] editlist.get: proxies.btnStart / btnPause visible:true => the display:none buttons are shown (ct3aLoad); visible:false => stay hidden
//     [3] START click => one form.event {TfContact, btnStart, click} on tag DeviceForm_File; the ack's afterAck is shown (green, "START")
//     [4] PAUSE click => {TfContact, btnPause, click}; the ack's afterAck is shown ("PAUSE")
//     [5] an ack without afterAck => no START / PAUSE line
//     [6] hidden / operable=false => a click sends nothing
//   Control: W906_CT3B_PAGE_DIR -> the page before CT-3b' (git show HEAD:web/page/ht9045_contact_ev.js) must be red ([2] / [3] / [4]).
//   No wb_serve, no machine file (reads one .js under web\page). Use: only through ctest (B8_Ct3b_ContactPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_CT3B_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
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
function makeEl(id, tag) {
  const attrs = {};
  const listeners = {};
  return {
    id, tagName: tag, nodeType: 1, disabled: false, title: '', style: {}, parentElement: null, children: [],
    classList: makeClassList(),
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    removeAttribute: k => { delete attrs[k]; },
    querySelector: () => null,
    querySelectorAll: () => [],
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    fire: t => { (listeners[t] || []).forEach(fn => fn({ isTrusted: true, type: t })); },
  };
}

function makeEnv(events, proxies) {
  const els = { btnStart: makeEl('btnStart', 'BUTTON'), btnPause: makeEl('btnPause', 'BUTTON') };
  els.btnStart.style.display = 'none';                           // Setup.Contact.html: display:none (DFM Visible=False)
  els.btnPause.style.display = 'none';
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
  btnStart: { event: 'click', operable: true, golden: 'cContact.cpp:14072 TfContact::btnStartClick' },
  btnPause: { event: 'click', operable: true, golden: 'cContact.cpp:14079 TfContact::btnPauseClick' },
};
const START_ITEM = 'golden cContact.cpp:14074-14076 TfContact::btnStartClick: TfMain::BtnStartClick (main.cpp:6529-6593) then edTorue0 / edTorue1 = "10", right after this reply';
const PAUSE_ITEM = 'golden cContact.cpp:14081 TfContact::btnPauseClick: TfMain::BtnPauseClick -> Pause("BtnPauseClick") (main.cpp:7387-7393), right after this reply';

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
  if (!script) { console.log('B8_Ct3b_ContactPage: ' + passed + ' passed, ' + (failed || 1) + ' failed'); process.exit(1); }

  {
    const env = makeEnv(EV, { btnStart: { visible: true, enabled: true }, btnPause: { visible: true, enabled: true } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    await check('[2] proxies visible:true => START / PAUSE shown (display:none cleared, golden FormShow :1628-1629 / SOFT_SIMULTE :1662-1663)', () => {
      ok(env.els.btnStart.style.display === '' && env.els.btnPause.style.display === '', 'display ' + env.els.btnStart.style.display + ' / ' + env.els.btnPause.style.display);
    });
    env.els.btnStart.fire('click');
    await flush();
    await check('[3] START click => one form.event {TfContact, btnStart, click} on tag DeviceForm_File', () => {
      ok(env.sent.length === 1, 'sent ' + env.sent.length);
      const s = env.sent[0], v = JSON.parse(s.value);
      ok(s.cmd === 'form.event' && s.tag === 'DeviceForm_File', 'cmd/tag ' + s.cmd + ' ' + s.tag);
      ok(v.form === 'TfContact' && v.control === 'btnStart' && v.event === 'click', 'value ' + s.value);
    });
    const n0 = env.said.length;
    env.answer({ changed: {}, messages: [], todo: [], afterAck: [START_ITEM] });
    await flush();
    await check('[3] the ack\'s afterAck is shown: one green line naming START and the server\'s item', () => {
      const lines = env.said.slice(n0).filter(x => /START/.test(x.msg) && x.msg.indexOf(START_ITEM) >= 0);
      ok(lines.length === 1 && lines[0].colour === '#9f9', 'said ' + JSON.stringify(env.said.slice(n0)));
    });
    env.els.btnPause.fire('click');
    await flush();
    await check('[4] PAUSE click => one form.event {TfContact, btnPause, click}', () => {
      ok(env.sent.length === 2, 'sent ' + env.sent.length);
      const v = JSON.parse(env.sent[1].value);
      ok(v.form === 'TfContact' && v.control === 'btnPause' && v.event === 'click', 'value ' + env.sent[1].value);
    });
    const n1 = env.said.length;
    env.answer({ changed: {}, messages: [], todo: [], afterAck: [PAUSE_ITEM] });
    await flush();
    await check('[4] the ack\'s afterAck is shown: one line naming PAUSE', () => {
      const lines = env.said.slice(n1).filter(x => /PAUSE/.test(x.msg) && x.msg.indexOf(PAUSE_ITEM) >= 0);
      ok(lines.length === 1, 'said ' + JSON.stringify(env.said.slice(n1)));
    });
    env.els.btnStart.fire('click');
    await flush();
    const n2 = env.said.length;
    env.answer({ changed: {}, messages: [], todo: [] });
    await flush();
    await check('[5] an ack without afterAck => no START / PAUSE line', () => {
      ok(env.sent.length === 3, 'sent ' + env.sent.length);
      ok(env.said.slice(n2).filter(x => /START|PAUSE/.test(x.msg)).length === 0, 'said ' + JSON.stringify(env.said.slice(n2)));
    });
  }
  {
    const ev = JSON.parse(JSON.stringify(EV));
    ev.btnStart.operable = false; ev.btnPause.operable = false;
    const env = makeEnv(ev, { btnStart: { visible: false, enabled: true }, btnPause: { visible: false, enabled: true } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    await check('[2] proxies visible:false ([D16] off, ship build) => START / PAUSE stay hidden', () => {
      ok(env.els.btnStart.style.display === 'none' && env.els.btnPause.style.display === 'none', 'display ' + env.els.btnStart.style.display);
    });
    env.els.btnStart.fire('click');
    env.els.btnPause.fire('click');
    await flush();
    await check('[6] hidden / operable=false => a click sends nothing', () => { ok(env.sent.length === 0, 'sent ' + env.sent.length); });
  }
  console.log('B8_Ct3b_ContactPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
