// AI(W906-D026) 20261001 [W906] St01: offline selftest for the page half of D-026 (ctest D026_NoteAuthPage).
//   The alarm note's password layer: dialog-bridge.js (not changed) opens its login layer when request.auth.required and, on OK,
//   calls window.HTDialogHost.verifyAuth(verify); web/page/ht9045_dialog_host.js hands the Dialog-auth-verify object to C++ as WS
//   dialog.auth (tag = authId, value = the JSON string) and maps the reply to what the bridge reads ({accepted, accessLevel, message}).
//   The login box's Cancel (golden: blank = wrong, Q45-4 = A) becomes one dialog.auth {"cancelled":true} with the pressed key.
//   C++ half: tests/test_note_auth.cpp (D026_NoteAuth).  Golden: V912 note.cpp:5277 TfNote::DoPassword, :5431 DoUnlockPassword.
//   Loads the REAL ht9045_dialog_host.js in a node vm with a fake window / document / HT9045Tags / HT9045Recipe.  No socket, no
//   wb_serve, no file written.  Usage: node tools/webprobe/d026_note_auth_selftest.cjs [<web/page dir>]
//   Control run: W906_DIALOG_HOST_JS at the pre-change file -> must go red.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_DIALOG_HOST_JS || path.join(PAGE, 'ht9045_dialog_host.js');
const code = fs.readFileSync(SRC, 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }
async function settle() { for (let i = 0; i < 8; i++) await new Promise((r) => setImmediate(r)); }

function rnd(prefix) { return prefix + Math.random().toString(36).slice(2, 10) + Date.now().toString(36).slice(-3); }
const SECRET = rnd('pw');                       // made up per run; must never show in a log or a cancel frame
const ALL_LOGS = [];

function makeHost() {
  const listeners = {};
  const logs = ALL_LOGS;                          // every host of the run logs into one list (step 5)
  const calls = [];
  let next = null;                              // scripted reply of the next dialogAuth: {ok:true,value:{...}} | {ok:false,error}
  const win = {
    addEventListener(t, f) { (listeners[t] = listeners[t] || []).push(f); },
    dispatch(t, ev) { (listeners[t] || []).forEach((f) => f(ev)); },
  };
  const fakeConsole = {
    log: (...a) => logs.push(a.join(' ')), warn: (...a) => logs.push(a.join(' ')),
    error: (...a) => logs.push(a.join(' ')), info: (...a) => logs.push(a.join(' ')),
  };
  win.HT9045Tags = { onEvent() {}, connect() { return Promise.resolve(); } };
  win.HT9045Recipe = {
    dialogAuth(authId, payload) {
      calls.push({ authId, payload });
      const r = next || { ok: true, value: { accepted: true, accessLevel: 2, message: null, stage: 'done' } };
      next = null;
      if (!r.ok) return Promise.reject(new Error(r.error));
      return Promise.resolve(Object.assign({ type: 'ack', id: calls.length, ok: true }, r.value));
    },
    modalAnswer() { return Promise.resolve({ ok: true }); },
    rawCmd() { return Promise.resolve({ ok: true }); },
  };
  const ctx = { window: win, document: { readyState: 'complete', addEventListener() {} }, console: fakeConsole,
                setTimeout, clearTimeout, Promise, JSON, Object, String, Number, Date, Error, Math };
  win.window = win;
  vm.createContext(ctx);
  // the IIFE takes `window`; run it with the fake window as the vm's `window`
  vm.runInContext(code, ctx, { filename: SRC });
  return { win, logs, calls, script(r) { next = r; }, H: win.HTDialogHost };
}
function verifyObj(authId, rid, pw) {
  return { schemaVersion: '1.0.0', channel: 'dialog-auth', seq: 1, authId, state: 'pending', requestedAt: 'x',
           target: { channel: 'show-error-message', requestId: rid, requestSeq: 3 }, kind: 'access-level', level: 2,
           pendingAction: { name: 'RETRY', code: 1, pressedButton: 'BtnStart' }, credentials: { userId: 'u1', password: pw } };
}

(async function main() {
  console.log('D026_NoteAuthPage  source: ' + SRC);
  // ---- 1. the hook exists ----------------------------------------------------------------------------------------------
  let h = makeHost();
  check(h.H && typeof h.H.verifyAuth === 'function', 'HTDialogHost.verifyAuth is there (dialog-bridge.js submitAuth calls it)');
  check(h.H && typeof h.H.submitResponse === 'function', 'HTDialogHost.submitResponse still there');
  if (!h.H || typeof h.H.verifyAuth !== 'function') { console.log('FAIL: ' + pass + ' passed, ' + (fail) + ' failed'); process.exit(1); }

  // ---- 2. accepted ---------------------------------------------------------------------------------------------------
  let v = verifyObj('auth-1', '42', SECRET);
  let p = h.H.verifyAuth(v);
  check(p && typeof p.then === 'function', 'verifyAuth returns a Promise');
  check(v.credentials.password === '', 'the password in the bridge object is cleared after the send');
  let res = await p;
  check(h.calls.length === 1 && h.calls[0].authId === 'auth-1', 'one dialog.auth, tag = authId');
  let sent = {};
  try { sent = JSON.parse(h.calls[0].payload); } catch (e) {}
  check(sent.target && sent.target.requestId === '42' && sent.pendingAction && sent.pendingAction.name === 'RETRY' &&
        sent.pendingAction.pressedButton === 'BtnStart', 'value = the Dialog-auth-verify JSON (target, pendingAction)');
  check(sent.credentials && sent.credentials.password === SECRET && sent.credentials.userId === 'u1', 'the typed credentials go to C++ as typed');
  check(res && res.accepted === true && res.accessLevel === 2 && res.userId === null && res.authId === 'auth-1' && res.state === 'completed',
        'reply mapped to {accepted, accessLevel, userId:null, authId, state}');

  // ---- 3. refused / not accepted / errors -----------------------------------------------------------------------------
  h.script({ ok: true, value: { accepted: false, accessLevel: 0, message: 'Wrong ID or password or Insufficient privileges!!', stage: 'done' } });
  res = await h.H.verifyAuth(verifyObj('auth-2', '42', SECRET));
  check(res.accepted === false && /Wrong ID/.test(res.message), 'accepted:false passes C++ message through (login box Label3)');
  h.script({ ok: true, value: { accepted: false, message: 'Unlock password OK -- now log in', stage: 'login' } });
  res = await h.H.verifyAuth(verifyObj('auth-3', '42', SECRET));
  check(res.accepted === false && res.stage === 'login', 'the two-step unlock reply keeps its stage');
  h.script({ ok: false, error: 'not-the-current-dialog: current=43' });
  let err = null;
  try { await h.H.verifyAuth(verifyObj('auth-4', '42', SECRET)); } catch (e) { err = e; }
  check(err && /dialog\.auth/.test(err.message) && /not-the-current-dialog/.test(err.message) && err.message.indexOf(SECRET) < 0,
        'a refusal rejects with the server reason, never the password');
  let threw = false, pr = null;
  try { pr = h.H.verifyAuth(null); } catch (e) { threw = true; }
  err = null; try { await pr; } catch (e) { err = e; }
  check(!threw && err, 'a bad object: rejected Promise, no synchronous throw');
  const R = h.win.HT9045Recipe; h.win.HT9045Recipe = null;
  threw = false; pr = null;
  try { pr = h.H.verifyAuth(verifyObj('auth-5', '42', SECRET)); } catch (e) { threw = true; }
  err = null; try { await pr; } catch (e) { err = e; }
  check(!threw && err && /dialogAuth/.test(err.message), 'no recipe client: rejected Promise, no synchronous throw');
  h.win.HT9045Recipe = R;

  // ---- 4. Cancel = one dialog.auth {cancelled:true} ------------------------------------------------------------------
  h = makeHost();
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_ACTION', kind: 'alarm', requestId: '77', action: { name: 'SKIP', code: 2 }, pressedButton: 'BtnPause' } });
  h.win.dispatch('ht-dialog-auth', { detail: { state: 'prompt', authId: 'auth-c1' } });
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_AUTH_CANCEL', kind: 'auth' } });
  await settle();
  check(h.calls.length === 1 && h.calls[0].authId === 'auth-c1', 'Cancel sends one dialog.auth');
  sent = {}; try { sent = JSON.parse(h.calls[0] ? h.calls[0].payload : '{}'); } catch (e) {}
  check(sent.cancelled === true && sent.target && sent.target.requestId === '77' && sent.pendingAction.name === 'SKIP' &&
        sent.pendingAction.pressedButton === 'BtnPause', 'the cancel carries the note and the pressed key');
  check(h.calls[0] && !/passw/i.test(h.calls[0].payload), 'the cancel frame has no password field');
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_AUTH_CANCEL', kind: 'auth' } });
  await settle();
  check(h.calls.length === 1, 'a second Cancel for the same prompt sends nothing');

  h = makeHost();                                                   // accepted, then the bridge closes the layer (no cancel message)
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_ACTION', kind: 'alarm', requestId: '78', action: { name: 'RETRY', code: 1 }, pressedButton: 'BtnStart' } });
  h.win.dispatch('ht-dialog-auth', { detail: { state: 'prompt', authId: 'auth-c2' } });
  await h.H.verifyAuth(verifyObj('auth-c2', '78', SECRET));
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_AUTH_CANCEL', kind: 'auth' } });
  await settle();
  check(h.calls.length === 1, 'after an accepted verify a stray Cancel sends nothing');

  h = makeHost();                                                   // a message-box action in between: no stale alarm action
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_ACTION', kind: 'alarm', requestId: '79', action: { name: 'RETRY', code: 1 } } });
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_ACTION', kind: 'message', requestId: 'msg-3', action: { name: 'OK' } } });
  h.win.dispatch('ht-dialog-auth', { detail: { state: 'prompt', authId: 'auth-c3' } });
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_AUTH_CANCEL', kind: 'auth' } });
  await settle();
  check(h.calls.length === 0, 'a prompt that is not an alarm note: Cancel sends nothing');

  h = makeHost();                                                   // cancel from something that is not the login page
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_ACTION', kind: 'alarm', requestId: '80', action: { name: 'RETRY', code: 1 } } });
  h.win.dispatch('ht-dialog-auth', { detail: { state: 'prompt', authId: 'auth-c4' } });
  h.win.dispatch('message', { data: { type: 'HT_DIALOG_AUTH_CANCEL', kind: 'alarm' } });
  await settle();
  check(h.calls.length === 0, 'HT_DIALOG_AUTH_CANCEL counts only from the login page (kind auth)');

  // ---- 5. nothing logged with the password ---------------------------------------------------------------------------
  const allLogs = h.logs.join('\n');
  check(allLogs.indexOf(SECRET) < 0, 'no console line carries the password');
  check(!/console\.(log|warn|error|info)\([^)]*password/i.test(code), 'the host never logs a password field');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL: exception ' + (e && e.stack)); process.exit(1); });
