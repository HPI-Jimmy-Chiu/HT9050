// w155_pad_page_selftest.cjs -- ctest St02_W155PadPageJs (node, offline): web/page/ht9045_padinterface_c.js, the page half of the
// web Pad window (HW.PadInterface.html = golden 0618 TfPadInterface), against a fake wb_serve (pad.* acks shaped like
// W906_PadWire's, PadInterface_St02.cpp EOF).  AI(W906-W155) 20261007 (St02-E), W-155 MR B.
//   1. load: one pad.get (no token, nothing written)
//   2. the window shows (HT_WIN open) -> control.takeover, then pad.open (golden FormShow); the screen = the snapshot
//   3. a pad button sends its COMPONENT name (sb_PadInterface_RearSafeLock, whose dfm Alias is wrong) and does not toggle itself
//   4. Send: a non-lamp text is refused on the page (DEVIATION W155-D2), a lamp frame goes as pad.send
//   5. the blink checkbox -> pad.bling, the box follows the snapshot, not the click
//   6. Reset Com sends nothing (no OnClick in golden)
//   7. the poll is the heartbeat; a snapshot with show=false (watchdog) is reported and locks the window
//   8. the window hides -> polling stops and pad.close goes out (DEVIATION W155-D3); Exit -> pad.exit
//   9. ControlPanelMode 0: pad.open refused -> every button locked, a click sends nothing
// Usage: node w155_pad_page_selftest.cjs <web/page dir>.  Control: W906_PAD_JS=<a copy that sends the Alias> must be red.
// Fake DOM rule (St02 workflow skill section 5 item 20): every property the script reads is set when the DOM is built.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_PAD_JS || path.join(PAGE, 'ht9045_padinterface_c.js');
const code = fs.readFileSync(SRC, 'utf8');

let pass = 0, fail = 0;
function check(cond, what, got) { if (cond) { pass++; console.log('  ok   ' + what); } else { fail++; console.log('  FAIL ' + what + '   [got ' + got + ']'); } }
async function settle() { for (let i = 0; i < 30; i++) await new Promise((r) => setImmediate(r)); }

// ---- the 31 PadItems (golden InitialVariable uPadInterface.cpp:211-241) ----
const NAMES = ['FrontPowerOff', 'FrontPowerOn', 'Front', 'FrontReset', 'FrontPause', 'FrontHome', 'FrontStart', 'FrontOneCycle', 'FrontRetry',
  'FrontSkip', 'FrontCleanOut', 'FrontTrayFeed', 'FrontTrayEnd', 'FrontAlarmReset', 'RearPowerOff', 'RearPowerOn', 'RearReset', 'RearPause',
  'RearHome', 'RearStart', 'RearOneCycle', 'RearRetry', 'RearSkip', 'RearCleanOut', 'RearTrayFeed', 'RearTrayEnd', 'RearAlarmReset',
  'RearSafeLock', 'RearStep', 'RearTStart', 'Rear'];

// ---- fake DOM ----
function classList() {
  const s = new Set();
  return { add(c) { s.add(c); }, remove(c) { s.delete(c); }, contains(c) { return s.has(c); },
           toggle(c, on) { if (on === undefined) on = !s.has(c); if (on) s.add(c); else s.delete(c); return on; } };
}
function mkEl(id, tagName, cls) {
  const attrs = { title: id + ' : x' };
  const e = { id, tagName, value: '', checked: false, disabled: false, style: {}, classList: classList(), scrollTop: 0, scrollHeight: 100,
    setAttribute(k, v) { attrs[k] = String(v); }, getAttribute(k) { return k in attrs ? attrs[k] : null; },
    hasAttribute(k) { return k in attrs; }, removeAttribute(k) { delete attrs[k]; },
    closest(sel) { if (sel === '.btnpanel') return e.classList.contains('btnpanel') ? e : null;
                   if (sel === 'button') return e.tagName === 'BUTTON' ? e : (e.parentButton || null); return null; },
    querySelector() { return null; }, contains(x) { return x === e; }, appendChild(c) { return c; } };
  (cls || []).forEach((c) => e.classList.add(c));
  return e;
}
const els = {};
NAMES.forEach((n) => { els['sb_PadInterface_' + n] = mkEl('sb_PadInterface_' + n, 'DIV', ['btnpanel']); els['ml_PadInterface_' + n] = mkEl('ml_PadInterface_' + n, 'SPAN', ['aled']); });
els.sb_PadInterface_ManualSend = mkEl('sb_PadInterface_ManualSend', 'BUTTON');
els.ed_PadInterface_ManualSend = mkEl('ed_PadInterface_ManualSend', 'INPUT');
els.btnResetCom = mkEl('btnResetCom', 'BUTTON');
els.sb_PadInterface_Exit = mkEl('sb_PadInterface_Exit', 'BUTTON', ['exitbtn']);
els.Memo_PadInterface = mkEl('Memo_PadInterface', 'TEXTAREA');
const cbInput = mkEl('', 'INPUT');
const cbLabel = mkEl('cb_PadInterface_PadLedBling', 'LABEL');
cbLabel.querySelector = (q) => (q === 'input' ? cbInput : null);
cbLabel.contains = (x) => x === cbInput || x === cbLabel;
els.cb_PadInterface_PadLedBling = cbLabel;

const listeners = { click: [], change: [], message: [], pagehide: [] };
const timers = [];
const sent = [];
let server = { mode: 1, show: false, bling: false, down: {}, refuseOpen: false };

function snapshot(op, extra) {
  return Object.assign({ op, mode: server.mode, rs232Ok: true, com: 'COM18', show: server.show, bling: server.bling, requestVer: false,
    sendLampOnly: true, showTimeoutMs: 10000, watchdogCloses: server.wd || 0, droppedT07T08: 0,
    items: NAMES.map((n, i) => ({ i, sb: 'sb_PadInterface_' + n, ml: 'ml_PadInterface_' + n, pad: 'Sw?', key: 'Sn?', tag: i >= 14 && i !== 2 ? 1 : 0,
                                  down: !!server.down[n], led: n === 'FrontStart', lamp: false })),
    memo: ['line A', 'line B'], memoCount: 2 }, extra || {});
}
function rawCmd(name, extra) {
  const v = extra && typeof extra.value === 'string' ? JSON.parse(extra.value) : null;
  sent.push({ name, v });
  if (name.indexOf('control.') === 0) return Promise.resolve({ ok: true });
  if (name === 'pad.get') return Promise.resolve({ value: JSON.stringify(snapshot('get')) });
  if (name === 'pad.close') { server.show = false; return Promise.resolve({ value: JSON.stringify(snapshot('close')) }); }
  if (server.mode !== 1) return Promise.reject(new Error('ControlPanelMode 不是 1'));
  if (name === 'pad.open') { server.show = true; server.down = {}; return Promise.resolve({ value: JSON.stringify(snapshot('open', { frames: ['f1', 'f2', 'f3', 'f4', 'f5', 'f6'] })) }); }
  if (name === 'pad.button') { const n = v.name.replace('sb_PadInterface_', ''); server.down[n] = !server.down[n]; return Promise.resolve({ value: JSON.stringify(snapshot('button', { frames: ['[Send] t05x'] })) }); }
  if (name === 'pad.send') return Promise.resolve({ value: JSON.stringify(snapshot('send', { frames: ['[Send] ' + v.text] })) });
  if (name === 'pad.bling') { server.bling = !!v.on; return Promise.resolve({ value: JSON.stringify(snapshot('bling', { frames: [] })) }); }
  if (name === 'pad.exit') { server.show = false; return Promise.resolve({ value: JSON.stringify(snapshot('exit', { frames: [] })) }); }
  return Promise.reject(new Error('unknown ' + name));
}
const parent = { postMessage() {} };
const sb = {
  console: { info() {}, warn() {}, error() {}, log() {} },
  setTimeout(f, ms) { timers.push({ f, ms, once: true }); return timers.length; }, clearTimeout() {},
  setInterval(f, ms) { const t = { f, ms, live: true }; timers.push(t); return t; }, clearInterval(t) { if (t) t.live = false; },
  Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp, Set,
  HT9045Recipe: { rawCmd, status() { return { holdsToken: false }; } },
  document: {
    readyState: 'complete',
    getElementById(id) { return els[id] || null; },
    addEventListener(t, f) { if (listeners[t]) listeners[t].push(f); },
    createElement() { return mkEl('', 'DIV'); },
    body: { appendChild(c) { return c; } },
    documentElement: { appendChild(c) { return c; } },
  },
  addEventListener(t, f) { if (listeners[t]) listeners[t].push(f); },
  parent,
};
sb.window = sb;
vm.createContext(sb);
vm.runInContext(code, sb, { filename: SRC });

function click(el) {
  const ev = { target: el, stopped: false, prevented: false, stopPropagation() { this.stopped = true; }, preventDefault() { this.prevented = true; } };
  listeners.click.forEach((f) => f(ev));
  return ev;
}
function hw(open) { listeners.message.forEach((f) => f({ data: { type: 'HT_WIN', open }, source: parent })); }
function names(from) { return sent.slice(from).map((s) => s.name); }
const liveInterval = () => timers.filter((t) => t.live).length;

(async () => {
  console.log('St02_W155PadPageJs (' + path.basename(SRC) + ')');
  await settle();
  check(sent.length === 1 && sent[0].name === 'pad.get' && sent[0].v.beat === false, '1. load: one pad.get (beat:false), no token, nothing written', names(0).join(',') + ' ' + JSON.stringify(sent[0] && sent[0].v));

  let n0 = sent.length;
  hw(true); await settle();
  check(names(n0).slice(0, 2).join(',') === 'control.takeover,pad.open', '2a. window shows -> control.takeover then pad.open (golden FormShow)', names(n0).join(','));
  check(liveInterval() === 1, '2b. the 1 s poll runs (the heartbeat)', liveInterval());
  check(els.ml_PadInterface_FrontStart.classList.contains('on') && !els.ml_PadInterface_FrontHome.classList.contains('on'), '2c. LEDs = the snapshot (mlEvent)', '');
  check(els.Memo_PadInterface.value === 'line A\nline B', '2d. the memo = the snapshot', JSON.stringify(els.Memo_PadInterface.value));

  n0 = sent.length;
  const ev = click(els.sb_PadInterface_RearSafeLock);
  check(ev.stopped && ev.prevented, '3a. the click is taken in capture (the generic .btnpanel toggle never runs)', ev.stopped + '/' + ev.prevented);
  check(!els.sb_PadInterface_RearSafeLock.classList.contains('down'), '3b. no local toggle before the ack', '');
  await settle();
  const pb = sent.slice(n0).filter((s) => s.name === 'pad.button')[0];
  check(pb && pb.v.name === 'sb_PadInterface_RearSafeLock', '3c. pad.button carries the COMPONENT (not Alias SwRKManualStep)', pb && JSON.stringify(pb.v));
  check(names(n0)[0] === 'control.takeover', '3d. the token is taken first', names(n0).join(','));
  check(els.sb_PadInterface_RearSafeLock.classList.contains('down'), '3e. Down painted from the ack snapshot', '');

  n0 = sent.length;
  els.ed_PadInterface_ManualSend.value = 't051120';
  click(els.sb_PadInterface_ManualSend); await settle();
  check(names(n0).length === 0, '4a. Send "t051120" (version poll) is refused on the page: nothing sent, no token taken', names(n0).join(','));
  els.ed_PadInterface_ManualSend.value = ' t051491004000 ';
  click(els.sb_PadInterface_ManualSend); await settle();
  const ps = sent.slice(n0).filter((s) => s.name === 'pad.send')[0];
  check(ps && ps.v.text === 't051491004000', '4b. a lamp frame goes out as pad.send (trimmed)', ps && JSON.stringify(ps.v));

  n0 = sent.length;
  cbInput.checked = true;
  listeners.change.forEach((f) => f({ target: cbInput }));
  check(cbInput.checked === false, '5a. the box waits for C++ (reverted until the ack)', cbInput.checked);
  await settle();
  const pl = sent.slice(n0).filter((s) => s.name === 'pad.bling')[0];
  check(pl && pl.v.on === true && cbInput.checked === true, '5b. pad.bling {on:true}, then the box = the snapshot', pl && JSON.stringify(pl.v) + ' / ' + cbInput.checked);

  n0 = sent.length;
  click(els.btnResetCom); await settle();
  check(names(n0).length === 0 && els.btnResetCom.disabled === true, '6. Reset Com: locked, sends nothing (no OnClick in golden)', names(n0).join(','));

  n0 = sent.length;
  timers.filter((t) => t.live)[0].f(); await settle();
  check(names(n0).join(',') === 'pad.get' && sent[n0].v.beat === true, '7a. a poll tick of the window that opened = one pad.get {beat:true} (the heartbeat)', names(n0).join(',') + ' ' + JSON.stringify(sent[n0] && sent[n0].v));
  server.show = false; server.wd = 1;
  timers.filter((t) => t.live)[0].f(); await settle();
  n0 = sent.length;
  click(els.sb_PadInterface_FrontStart); await settle();
  check(names(n0).length === 0 && els.sb_PadInterface_FrontStart.__padLocked === true, '7b. show=false from C++ (watchdog) -> the window is locked, a click sends nothing', names(n0).join(','));
  n0 = sent.length;
  server.show = true;                                // someone else reopened it: this locked window must not keep it alive
  timers.filter((t) => t.live)[0].f(); await settle();
  check(names(n0).join(',') === 'pad.get' && sent[n0].v.beat === false, '7c. a locked window keeps polling for display but sends beat:false (cannot keep another window alive)', JSON.stringify(sent[n0] && sent[n0].v));

  // reopen, then hide
  hw(false); await settle();
  hw(true); await settle();
  n0 = sent.length;
  hw(false); await settle();
  check(liveInterval() === 0, '8a. hidden -> the poll stops', liveInterval());
  check(names(n0).join(',') === 'pad.close', '8b. hidden -> pad.close (golden FormClose, DEVIATION W155-D3)', names(n0).join(','));
  hw(true); await settle();
  n0 = sent.length;
  click(els.sb_PadInterface_Exit); await settle();
  check(names(n0).indexOf('pad.exit') >= 0, '8c. Exit -> pad.exit (golden ExitClick)', names(n0).join(','));
  hw(false); await settle();

  server.mode = 0;
  hw(true); await settle();
  n0 = sent.length;
  click(els.sb_PadInterface_FrontHome); await settle();
  check(names(n0).length === 0 && els.sb_PadInterface_FrontHome.__padLocked === true && els.sb_PadInterface_ManualSend.disabled === true,
        '9. ControlPanelMode 0: pad.open refused -> everything locked, a click sends nothing', names(n0).join(','));

  console.log('St02_W155PadPageJs: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})();
