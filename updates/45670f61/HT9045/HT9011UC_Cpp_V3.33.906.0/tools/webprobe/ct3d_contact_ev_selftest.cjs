// AI(W906-B8-CT3D) 20261001 [W906] (St01): B8 CT-3d, the page half -- D:\HT9045\web\page\ht9045_contact_ev.js on Setup.Contact
//   (OTD panels palOTD_4 / palOTD_6 and the ledOTD lamp). Dispatch D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md "CT-3";
//   the C++ half is ctest B8_Ct3d_OtdDock (tests/test_b8_ct3d_otddock.cpp).
//   golden 906 cContact.cpp (（V912 :N）= V912; AI(W906-E030-CITE) 20261003; mock golden labels = generated V912 ones): palOTD_4Click :15170-15193（V912 :15395-15418） / palOTD_6Click :15195-15218（V912 :15420-15443） set BevelOuter (bvLowered / bvRaised); OTDTimerTimer
//   :15220-15282（V912 :15445-15507） sets ledOTD TrueColor (clRed / clLime) + Value. C++ puts both on the proxy Tag: the panel Tag = VCL TBevelCut (1 bvLowered,
//   2 bvRaised; 0 / absent = never clicked = DFM default bvRaised -- a TPanel tag is only sent when non-zero), the lamp Tag 0 off / 1 lime / 2 red.
//   How: the whole ht9045_contact_ev.js in a node vm with a fake window / document / HT9045Recipe (editlistGet / editlistSave / rawCmd):
//     [1] compiles, one EOL style, no BOM
//     [2] editlist.get: ledOTD tag 2 -> lamp on, red; palOTD_4 tag 1 -> inset; palOTD_6 without a tag -> outset (ct3dLoad)
//     [3] a palOTD_4 click -> one form.event {TfContact, palOTD_4, click}; the ack's changed tags redraw both panels and the lamp (lime)
//     [4] an ack with ledOTD tag 0 -> lamp off; palOTD_6 click -> {TfContact, palOTD_6, click}
//     [5] hidden / operable=false -> a click sends nothing
//     [6] a click on the lamp does not reach palOTD_4 (golden TALed is a TGraphicControl, elec\Component\aled.pas:24, no OnClick in the DFM);
//         a click on the panel itself still sends
//   Control: W906_CT3D_PAGE_DIR -> the page before CT-3d must be red ([2] / [3] / [4]).
//   No wb_serve, no machine file (reads one .js under web\page). Use: only through ctest (B8_Ct3d_ContactPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_CT3D_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
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
function makeStyle(init) {
  const vars = {};
  const st = Object.assign({}, init || {});
  st.setProperty = (k, v) => { vars[k] = String(v); };
  st.getPropertyValue = k => (k in vars ? vars[k] : '');
  return st;
}
function makeEl(id, tag, cls, style) {
  const attrs = {};
  const listeners = {};
  const el = {
    id, tagName: tag, nodeType: 1, disabled: false, title: '', style: makeStyle(style), parentElement: null, children: [],
    classList: makeClassList(),
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    removeAttribute: k => { delete attrs[k]; },
    querySelector: () => null,
    querySelectorAll: () => [],
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    fire: t => { (listeners[t] || []).forEach(fn => fn({ isTrusted: true, type: t })); },
    // DOM bubbling: this element first, then each parent, until a listener calls stopPropagation()
    bubble: t => {
      const ev = { isTrusted: true, type: t, stopped: false, stopPropagation() { this.stopped = true; } };
      for (let e = el; e && !ev.stopped; e = e.parentElement) (e.__listeners[t] || []).forEach(fn => fn(ev));
    },
    __listeners: listeners,
  };
  (cls || []).forEach(c => el.classList.add(c));
  return el;
}

function makeEnv(events, proxies) {
  // Setup.Contact.html: palOTD_4 / palOTD_6 are <div class="pnl" style="... border:1px outset #ddd;">, ledOTD is <span class="aled LEDSqLarge"
  //   style="... --led-on:#00ff00; --led-off:#c0c0c0;"> inside palOTD_4
  const els = {
    palOTD_4: makeEl('palOTD_4', 'DIV', ['pnl'], { borderStyle: 'outset' }),
    palOTD_6: makeEl('palOTD_6', 'DIV', ['pnl'], { borderStyle: 'outset' }),
    ledOTD: makeEl('ledOTD', 'SPAN', ['aled', 'LEDSqLarge']),
  };
  els.ledOTD.parentElement = els.palOTD_4;
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
  palOTD_4: { event: 'click', operable: true, golden: 'cContact.cpp:15395 TfContact::palOTD_4Click' },
  palOTD_6: { event: 'click', operable: true, golden: 'cContact.cpp:15420 TfContact::palOTD_6Click' },
};
const lampOn = el => el.classList.contains('on');
const lampColour = el => el.style.getPropertyValue('--led-on');

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
  if (!script) { console.log('B8_Ct3d_ContactPage: ' + passed + ' passed, ' + (failed || 1) + ' failed'); process.exit(1); }

  {
    const env = makeEnv(EV, { palOTD_4: { visible: true, enabled: true, tag: 1 }, palOTD_6: { visible: true, enabled: true }, ledOTD: { visible: true, enabled: true, tag: 2 } });
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    await check('[2] editlist.get: ledOTD tag 2 -> lamp on, red (golden :15242-15243 (V912 :15467-15468) / :15273-15274 (V912 :15498-15499) clRed)', () => {
      ok(lampOn(env.els.ledOTD) && lampColour(env.els.ledOTD) === '#ff0000', 'on ' + lampOn(env.els.ledOTD) + ' colour ' + lampColour(env.els.ledOTD));
    });
    await check('[2] palOTD_4 tag 1 (bvLowered) -> inset; palOTD_6 without a tag (DFM default bvRaised) -> outset', () => {
      ok(env.els.palOTD_4.style.borderStyle === 'inset' && env.els.palOTD_6.style.borderStyle === 'outset',
         'border ' + env.els.palOTD_4.style.borderStyle + ' / ' + env.els.palOTD_6.style.borderStyle);
    });
    env.els.palOTD_4.fire('click');
    await flush();
    await check('[3] palOTD_4 click => one form.event {TfContact, palOTD_4, click} on tag DeviceForm_File', () => {
      ok(env.sent.length === 1, 'sent ' + env.sent.length);
      const s = env.sent[0], v = JSON.parse(s.value);
      ok(s.cmd === 'form.event' && s.tag === 'DeviceForm_File', 'cmd/tag ' + s.cmd + ' ' + s.tag);
      ok(v.form === 'TfContact' && v.control === 'palOTD_4' && v.event === 'click', 'value ' + s.value);
    });
    env.answer({ changed: { palOTD_4: { tag: 2 }, palOTD_6: { tag: 1 }, ledOTD: { tag: 1 } }, messages: [], todo: [] });
    await flush();
    await check('[3] ack.changed tags: palOTD_4 bvRaised -> outset, palOTD_6 bvLowered -> inset, ledOTD 1 -> on, lime (golden :15249-15250 (V912 :15474-15475) clLime)', () => {
      ok(env.els.palOTD_4.style.borderStyle === 'outset' && env.els.palOTD_6.style.borderStyle === 'inset',
         'border ' + env.els.palOTD_4.style.borderStyle + ' / ' + env.els.palOTD_6.style.borderStyle);
      ok(lampOn(env.els.ledOTD) && lampColour(env.els.ledOTD) === '#00ff00', 'on ' + lampOn(env.els.ledOTD) + ' colour ' + lampColour(env.els.ledOTD));
    });
    env.els.palOTD_6.fire('click');
    await flush();
    await check('[4] palOTD_6 click => {TfContact, palOTD_6, click}', () => {
      ok(env.sent.length === 2, 'sent ' + env.sent.length);
      const v = JSON.parse(env.sent[1].value);
      ok(v.form === 'TfContact' && v.control === 'palOTD_6' && v.event === 'click', 'value ' + env.sent[1].value);
    });
    env.answer({ changed: { ledOTD: { tag: 0 } }, messages: [], todo: [] });
    await flush();
    await check('[4] ack ledOTD tag 0 (golden :15268 (V912 :15493) Value=false, all open) -> lamp off', () => { ok(!lampOn(env.els.ledOTD), 'still on'); });
    env.els.ledOTD.bubble('click');
    await flush();
    await check('[6] a click on the lamp (inside palOTD_4) sends nothing: golden TALed is a TGraphicControl (aled.pas:24) without OnClick, '
                + 'VCL gives it the click, palOTD_4Click does not run', () => { ok(env.sent.length === 2, 'sent ' + env.sent.length); });
    env.els.palOTD_4.bubble('click');
    await flush();
    await check('[6] a click on the panel itself still sends palOTD_4', () => {
      ok(env.sent.length === 3 && JSON.parse(env.sent[2].value).control === 'palOTD_4', 'sent ' + env.sent.length);
    });
    env.answer({ changed: {}, messages: [], todo: [] });
    await flush();
  }
  {
    const ev = JSON.parse(JSON.stringify(EV));
    ev.palOTD_4.operable = false; ev.palOTD_6.operable = false;
    const env = makeEnv(ev, { palOTD_4: { visible: false, enabled: true }, palOTD_6: { visible: false, enabled: true }, ledOTD: { visible: true, enabled: true, tag: 0 } });
    env.els.palOTD_4.style.visibility = 'hidden';                   // the engine applies visible:false (USE_OTD != 1, golden FormShow :1310-1311（V912 :1329-1330）)
    env.els.palOTD_6.style.visibility = 'hidden';
    script.runInContext(env.ctx);
    await env.win.HT9045Recipe.editlistGet('DeviceForm_File');
    await new Promise(r => setTimeout(r, 5));
    env.els.palOTD_4.fire('click');
    env.els.palOTD_6.fire('click');
    await flush();
    await check('[5] hidden / operable=false (USE_OTD != 1) => a click sends nothing', () => { ok(env.sent.length === 0, 'sent ' + env.sent.length); });
  }
  console.log('B8_Ct3d_ContactPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
