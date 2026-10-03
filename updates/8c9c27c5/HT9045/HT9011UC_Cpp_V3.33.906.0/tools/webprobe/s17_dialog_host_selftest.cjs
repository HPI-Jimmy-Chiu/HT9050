// AI(W906-S17C) 20261003 [W906] St01: offline selftest for fix C of card S-17 (web/page/ht9045_dialog_host.js submitResponse).
//   ht9045_dialog_host.js keeps ONE pendingQuery (the last WS 'query' frame).  For the alarm channel the box's requestId IS the
//   query's qid (tools/wb_serve.cpp qidStr), so a pendingQuery whose qid differs from the box belongs to ANOTHER alarm:
//     (a) ACKNOWLEDGE on a kCode==0 notice while another alarm's query is pending -> dialog.notifyAck (not modal.answer to the
//         other qid, which answered the wrong box or was refused and left the notice up);
//     (b) a blocking answer whose requestId differs from pendingQuery (or with no pendingQuery) -> modal.answer tag = requestId,
//         payload = the wanted option (C++'s wait loop matches its own qid and validates the option itself);
//     (c) "no query pending" (incl. "no query pending:superseded-by=<id>") from modal.answer -> resolved as closed so
//         dialog-bridge drops the box; pendingQuery cleared when it was that qid.
//   Loads the REAL file in a node vm with a fake window / document / HT9045Tags / HT9045Recipe.  No socket, no wb_serve, no file
//   written.  Usage: node tools/webprobe/s17_dialog_host_selftest.cjs [<web/page dir>]
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

function makeHost() {
  const listeners = {};
  const calls = [];                                   // every recipe call: {fn, a, b}
  const scripted = { modalAnswer: [], rawCmd: [], dialogResponse: [] };   // FIFO of scripted replies per fn
  let onEventCb = null;
  function reply(fn) {
    const r = scripted[fn].shift() || { ok: true, value: {} };
    if (r.reject) return Promise.reject(new Error(r.reject));
    return Promise.resolve(Object.assign({ type: 'ack', id: calls.length, ok: r.ok !== false }, r.ok === false ? { error: r.error } : {}, r.value || {}));
  }
  const win = {
    addEventListener(t, f) { (listeners[t] = listeners[t] || []).push(f); },
    dispatch(t, ev) { (listeners[t] || []).forEach((f) => f(ev)); },
  };
  const quiet = { log() {}, warn() {}, error() {}, info() {} };
  win.HT9045Tags = { onEvent(cb) { onEventCb = cb; }, connect() { return Promise.resolve(); } };
  win.HT9045Recipe = {
    modalAnswer(qid, option) { calls.push({ fn: 'modalAnswer', a: qid, b: option }); return reply('modalAnswer'); },
    rawCmd(name, extra) { calls.push({ fn: 'rawCmd', a: name, b: extra }); return reply('rawCmd'); },
    dialogResponse(rid, act) { calls.push({ fn: 'dialogResponse', a: rid, b: act }); return reply('dialogResponse'); },
    keepAlive() { calls.push({ fn: 'keepAlive' }); return Promise.resolve(); },
    dialogAuth() { return Promise.resolve({ ok: true }); },
  };
  const ctx = { window: win, document: { readyState: 'complete', addEventListener() {} }, console: quiet,
                setTimeout, clearTimeout, Promise, JSON, Object, String, Number, Date, Error, Math };
  win.window = win;
  vm.createContext(ctx);
  vm.runInContext(code, ctx, { filename: SRC });
  return {
    win, calls, H: win.HTDialogHost,
    script(fn, r) { scripted[fn].push(r); },
    query(qid, options, kcode) { if (onEventCb) onEventCb({ type: 'query', qid, code: 'WAR0000', kcode: kcode == null ? 3 : kcode, options, at: 'x' }); },
    of(fn) { return calls.filter((c) => c.fn === fn); },
  };
}
function alarmResp(rid, name, code, pressed) {
  return { schemaVersion: '1.0.0', channel: 'show-error-message', seq: 1, requestId: rid, requestSeq: 5, state: 'completed',
           accepted: true, closedBy: 'action-button', selectedAction: { name, code }, pressedButton: pressed || null };
}
async function settle(p) { try { return { ok: true, v: await p }; } catch (e) { return { ok: false, e }; } }
function noSyncThrow(fn) { try { const p = fn(); return { threw: false, p }; } catch (e) { return { threw: true, p: Promise.reject(e) }; } }

(async function main() {
  console.log('S17_DialogHostPage  source: ' + SRC);
  let h, r, t;

  // ---- 1. (a) ACKNOWLEDGE on notice "26" while alarm "27" has the pending query -> dialog.notifyAck, no modal.answer ----------
  h = makeHost();
  h.query(27, ['RETRY', 'SKIP']);
  t = noSyncThrow(() => h.H.submitResponse('Alarm-dialog-response.json', alarmResp('26', 'ACKNOWLEDGE', 0, 'BtnPause')));
  r = await settle(t.p);
  check(!t.threw, '1: no synchronous throw');
  const ack = h.of('rawCmd');
  check(ack.length === 1 && ack[0].a === 'dialog.notifyAck' && ack[0].b && ack[0].b.tag === '26',
        "1: notice '26' with query 27 pending -> rawCmd('dialog.notifyAck', {tag:'26'})");
  check(h.of('modalAnswer').length === 0, '1: modal.answer NOT sent (it would answer alarm 27 = the wrong box)');
  check(r.ok && r.v === 'notice-retired', '1: resolves notice-retired (bridge closes the notice box)');
  check(h.H.pending() && h.H.pending().qid === 27, "1: alarm 27's pendingQuery is left alone");

  // ---- 2. (b) answer for requestId "30" with no pendingQuery -> modal.answer('30', 'RETRY:BtnStart') --------------------------
  h = makeHost();
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('30', 'RETRY', 1, 'BtnStart')));
  let ma = h.of('modalAnswer');
  check(ma.length === 1 && String(ma[0].a) === '30', "2: no pendingQuery, box '30' -> modal.answer tag '30'");
  check(ma.length === 1 && /^RETRY(:|$)/.test(String(ma[0].b)) && ma[0].b === 'RETRY:BtnStart', "2: payload 'RETRY:BtnStart' (option + ':' + pressedButton)");
  check(r.ok && r.v === 'modal-answered', '2: resolves modal-answered');

  // ---- 3. (b) stale pendingQuery "12", box "15" -> modal.answer uses '15', not 12 and not 12's options ------------------------
  h = makeHost();
  h.query(12, ['RETRY', 'SKIP']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('15', 'RETRY', 1, null)));
  ma = h.of('modalAnswer');
  check(ma.length === 1 && String(ma[0].a) === '15', "3: stale pendingQuery 12, box '15' -> modal.answer tag '15' (not 12)");
  check(ma.length === 1 && ma[0].b === 'RETRY', "3: payload 'RETRY' (no pressedButton)");
  check(r.ok, '3: resolves');
  h = makeHost();                                                    // 12's options must not veto 15's answer
  h.query(12, ['SKIP']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('15', 'CLEAN_OUT', 4, 'BtnPause')));
  ma = h.of('modalAnswer');
  check(ma.length === 1 && String(ma[0].a) === '15' && ma[0].b === 'CLEAN_OUT:BtnPause',
        "3: another query's options are not used to refuse box 15's option (C++ validates it)");

  // ---- 4. (c) 'no query pending:superseded-by=31' -> resolved as closed; pendingQuery cleared when it was that qid ------------
  h = makeHost();
  h.query(29, ['RETRY', 'SKIP']);
  h.script('modalAnswer', { reject: 'no query pending:superseded-by=31' });
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('29', 'SKIP', 2, 'BtnPause')));
  check(h.of('modalAnswer').length === 1 && String(h.of('modalAnswer')[0].a) === '29', '4: own query 29 answered with tag 29');
  check(r.ok && r.v === 'alarm-already-closed', "4: rejected 'no query pending:superseded-by=31' -> resolves alarm-already-closed");
  check(h.H.pending() === null, '4: pendingQuery 29 cleared (it was that qid)');
  h = makeHost();
  h.query(40, ['RETRY']);
  h.script('modalAnswer', { ok: false, error: 'no query pending' });
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('33', 'RETRY', 1, null)));
  check(r.ok && r.v === 'alarm-already-closed', "4: ack ok:false 'no query pending' for box 33 -> resolves alarm-already-closed");
  check(h.H.pending() && h.H.pending().qid === 40, "4: pendingQuery 40 (another alarm) is NOT cleared by 33's answer");
  h = makeHost();
  h.query(34, ['RETRY']);
  h.script('modalAnswer', { reject: 'not an offered option' });
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('34', 'RETRY', 1, null)));
  check(!r.ok && /not an offered option/.test(r.e.message), '4: any other refusal still rejects (box stays, operator may press again)');
  h = makeHost();                                                    // after a transient refusal the same box may answer again
  h.query(35, ['RETRY']);
  h.script('modalAnswer', { reject: 'modal-pending' });
  await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('35', 'RETRY', 1, null)));
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('35', 'RETRY', 1, null)));
  check(r.ok && h.of('modalAnswer').length === 2, '4: a refused answer can be sent again');

  // ---- 5. unchanged behaviour -----------------------------------------------------------------------------------------------
  h = makeHost();
  h.query(41, ['RETRY', 'SKIP']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('41', 'skip', 2, 'BtnPause')));
  ma = h.of('modalAnswer');
  check(ma.length === 1 && ma[0].a === 41 && ma[0].b === 'SKIP:BtnPause', "5: matching pendingQuery 41 -> modal.answer(41, 'SKIP:BtnPause') as before");
  check(r.ok && r.v === 'modal-answered' && h.H.pending() === null, '5: resolves modal-answered and clears pendingQuery');
  h = makeHost();
  h.query(42, ['RETRY']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('42', 'SKIP', 2, null)));
  check(!r.ok && /RETRY/.test(r.e.message) && h.of('modalAnswer').length === 0, "5: own query's options still checked (SKIP not offered -> reject, nothing sent)");
  h = makeHost();
  h.query(43, ['RETRY']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', Object.assign(alarmResp('43', 'RETRY', 1, null), { requestId: undefined })));
  check(h.of('modalAnswer').length === 1 && h.of('modalAnswer')[0].a === 43, '5: a response without requestId still answers the pendingQuery');
  h = makeHost();
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', Object.assign(alarmResp('', 'RETRY', 1, null))));
  check(!r.ok && h.calls.length === 0, '5: no pendingQuery and no requestId -> reject, nothing sent');
  h = makeHost();
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('44', 'ACKNOWLEDGE', 0, null)));
  check(h.of('rawCmd').length === 1 && h.of('rawCmd')[0].b.tag === '44' && r.ok, '5: notice with no pendingQuery -> dialog.notifyAck as before');
  h = makeHost();
  h.script('rawCmd', { reject: 'no-pending-notice:superseded-by=45' });
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', alarmResp('44', 'ACKNOWLEDGE', 0, null)));
  check(r.ok && r.v === 'notice-already-closed', "5: notifyAck 'no-pending-notice:superseded-by' -> closed as before");
  h = makeHost();
  h.script('dialogResponse', { reject: 'no query pending:superseded-by=msg-9' });
  r = await settle(h.H.submitResponse('Message-dialog-response.json',
    { channel: 'show-my-message', requestId: 'msg-8', selectedAction: 'OK', closedBy: 'action-button' }));
  check(h.of('dialogResponse').length === 1 && h.of('dialogResponse')[0].a === 'msg-8' && h.of('dialogResponse')[0].b === 'OK',
        "5: show-my-message -> dialog.response('msg-8', 'OK')");
  check(r.ok && r.v === 'message-already-closed', "5: show-my-message 'no query pending:superseded-by=msg-9' -> resolves as closed");
  h = makeHost();
  r = await settle(h.H.submitResponse('Dialog-close-response.json', { channel: 'dialog-close' }));
  check(r.ok && r.v === 'close-response-noop' && h.calls.length === 0, '5: close response -> noop');
  h = makeHost();
  h.query(46, ['RETRY']);
  r = await settle(h.H.submitResponse('Alarm-dialog-response.json', Object.assign(alarmResp('46', 'RETRY', 1, null), { closedBy: 'external-io' })));
  check(r.ok && r.v === 'closed-by-cpp-io' && h.calls.length === 0, '5: external-io close -> not sent back to C++');
  h = makeHost();
  const R = h.win.HT9045Recipe; h.win.HT9045Recipe = null;
  t = noSyncThrow(() => h.H.submitResponse('Alarm-dialog-response.json', alarmResp('47', 'RETRY', 1, null)));
  r = await settle(t.p);
  check(!t.threw && !r.ok, '5: no recipe client -> rejected Promise, no synchronous throw');
  h.win.HT9045Recipe = R;
  t = noSyncThrow(() => h.H.submitResponse('Alarm-dialog-response.json', null));
  r = await settle(t.p);
  check(!t.threw && !r.ok, '5: null response -> rejected Promise, no synchronous throw');

  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL: exception ' + (e && e.stack)); process.exit(1); });
