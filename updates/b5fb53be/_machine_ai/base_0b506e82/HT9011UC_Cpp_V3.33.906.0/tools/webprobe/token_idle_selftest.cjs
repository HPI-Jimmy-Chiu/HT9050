// AI(W906-TOKEN-IDLE) 20260926: web/page/ht9045_recipe_client.js 的「閒置自動還權杖」離線自我測試。
//   機台 0926 實測：Motor Test 拿到單一操作員權杖後從來不 release，之後 10 分鐘 IO 頁按 Output 都被擋。
//   這支用 node 的 vm 開兩個「頁面」（各自載入一份 recipe client），接到同一個假的 WebBridge 伺服器；
//   假伺服器照 WebBridgeServer.cpp 的權杖規則（control.acquire／release 只看連線；motor.stop、modal.answer
//   免權杖；其他指令要是持有者，否則回 not-operator；別人拿著時 acquire 回 control-held）。
//   不連真的 wb_serve、不讀寫任何檔。用法：node tools/webprobe/token_idle_selftest.cjs
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const SRC = process.env.W906_RECIPE_CLIENT || path.join(__dirname, '..', '..', '..', 'web', 'page', 'ht9045_recipe_client.js');   // 對照組：指向舊版必須紅
const code = fs.readFileSync(SRC, 'utf8');

// ---- 假伺服器（WebBridgeServer.cpp:1386-1440 的權杖規則） ------------------------------
const server = { owner: 0, nextConn: 1, log: [] };
const EXEMPT = new Set(['motor.stop', 'modal.answer', 'dialog.response', 'ui.windows.put', 'cfg.resync', 'log.event']);
function serverHandle(conn, msg) {
  const reply = (ok, error, extra) => conn._deliver(Object.assign({ type: 'ack', id: msg.id, ok, error: error || '' }, extra || {}));
  const name = msg.cmd;
  server.log.push(conn.cid + ':' + name);
  if (name === 'control.acquire') {
    if (server.owner === 0 || server.owner === conn.cid) { server.owner = conn.cid; return reply(true); }
    return reply(false, 'control-held');
  }
  if (name === 'control.release') {
    if (server.owner === conn.cid) { server.owner = 0; return reply(true); }
    return reply(false, 'not-operator');
  }
  if (!name.startsWith('auth.') && !EXEMPT.has(name) && server.owner !== conn.cid) return reply(false, 'not-operator');
  return reply(true, '', { state: 'done', result: 'ok' });
}
class FakeWS {
  constructor() {
    this.cid = server.nextConn++;
    this.readyState = 0;
    setTimeout(() => { this.readyState = 1; if (this.onopen) this.onopen(); }, 1);
  }
  send(s) { const m = JSON.parse(s); setTimeout(() => serverHandle(this, m), 2); }
  _deliver(o) { if (this.onmessage) this.onmessage({ data: JSON.stringify(o) }); }
  close() { this.readyState = 3; if (server.owner === this.cid) server.owner = 0; if (this.onclose) this.onclose(); }
}

function makePage(name) {
  const sb = {
    console, setTimeout, clearTimeout, setInterval, clearInterval, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    WebSocket: FakeWS,
    location: { protocol: 'http:', host: '127.0.0.1:8055', search: '' },
    document: { addEventListener() {}, removeEventListener() {}, hidden: false },
    XMLHttpRequest: function () { this.open = () => {}; this.send = () => {}; },
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
    addEventListener() {}, removeEventListener() {},
  };
  sb.window = sb; sb.self = sb; sb.globalThis = sb;
  vm.createContext(sb);
  vm.runInContext(code, sb, { filename: name + ':ht9045_recipe_client.js' });
  if (!sb.HT9045Recipe) throw new Error('HT9045Recipe not exported');
  return sb.HT9045Recipe;
}

let fail = 0, pass = 0;
function check(cond, what) { if (cond) { pass++; console.log('  ok   ' + what); } else { fail++; console.log('  FAIL ' + what); } }
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const outcome = (p) => p.then(() => 'ok', (e) => 'err:' + (e && e.message));
const req = { motors: ['MInArmX'], action: 'move', button: 'btnGo' };

(async () => {
  const A = makePage('MotorTest'), B = makePage('IoSetView');
  check(typeof A.setTokenHold === 'function' && typeof A.tokenIdleMs === 'function', 'API: setTokenHold／tokenIdleMs 存在');
  check(!!A.tokenIdleMs && A.tokenIdleMs() === 30000, '預設閒置 30 秒');
  if (A.tokenIdleMs) { A.tokenIdleMs(200); B.tokenIdleMs(200); }   // 對照組（舊版沒有這個 API）照樣往下跑，要在 [2] 紅

  console.log('[1] Motor Test 按一顆鈕 → 拿到權杖；IO 頁馬上寫 → control-held（舊行為的症狀）');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A motor.access ok');
  check(A.status().holdsToken === true, 'A 持有權杖');
  check(await outcome(B.write('Doc.Data', { S: { K: '1' } })) === 'err:control-held', 'B 寫入被擋 control-held');

  console.log('[2] A 閒置超過 tokenIdleMs → 自動 release；B 再寫就過');
  await sleep(350);
  check(A.status().holdsToken === false && server.owner === 0, 'A 已自動還權杖（伺服器 owner=0）');
  check(await outcome(B.write('Doc.Data', { S: { K: '1' } })) === 'ok', 'B 寫入成功');
  await sleep(350);
  check(server.owner === 0, 'B 用完也自動還');

  console.log('[3] hold：HOME／Loop 進行中不還');
  let running = true;
  if (A.setTokenHold) A.setTokenHold(() => running);
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 開始 HOME（motor.access ok）');
  await sleep(500);
  check(server.owner !== 0 && A.status().holdsToken === true, 'hold 期間 A 仍持有（過了 2.5 倍閒置時間）');
  check(await outcome(B.write('Doc.Data', { S: { K: '2' } })) === 'err:control-held', 'hold 期間 B 被擋');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 送 start:false 仍然成功');
  running = false;
  await sleep(500);
  check(server.owner === 0, 'hold 結束後自動還');

  console.log('[4] 伺服器 10 分鐘收回（本頁以為還拿著）→ not-operator → 自動重拿重送一次');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 拿到');
  server.owner = 0;                                     // PumpLiveness 的 controlIdleTimeoutMs 收回
  const before = server.log.length;
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 下一個指令仍成功（重拿後重送）');
  const seq = server.log.slice(before).map((s) => s.split(':')[1]).join(',');
  check(seq === 'motor.access,control.acquire,motor.access', '伺服器看到的順序＝被拒、重拿、重送（' + seq + '）');

  console.log('[5] 伺服器收回後被別人拿走 → 重拿回 control-held，照原樣報錯、不無限重試');
  server.owner = 999;
  const b2 = server.log.length;
  check(await outcome(A.motorAccess(req)) === 'err:control-held', 'A 回 control-held');
  check(server.log.length - b2 === 2, '只多送 2 個指令（motor.access 被拒＋control.acquire 被拒），沒有迴圈');
  server.owner = 0;

  console.log('[6] 免權杖指令（motor.stop）不拿權杖、不影響別人');
  await sleep(350);
  check(await outcome(B.write('Doc.Data', { S: { K: '3' } })) === 'ok', 'B 拿著');
  check(await outcome(A.motorStop(req)) === 'ok', 'A motor.stop 在 B 持有時照樣成功');
  check(A.status().holdsToken === false, 'A 沒有因此拿到權杖');

  console.log('[7] 明確 release 之後下一個指令會先重拿（同一條連線依序處理）');
  await sleep(350);
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 拿到');
  const b3 = server.log.length;
  const r = A.release();                                // 同步把 haveToken 放掉
  const m = A.motorAccess(req);
  check(await outcome(r) === 'ok' && await outcome(m) === 'ok', 'release 與緊接的指令都成功');
  check(server.log.slice(b3).map((s) => s.split(':')[1]).join(',') === 'control.release,control.acquire,motor.access', '順序＝release、acquire、指令');

  console.log((fail ? 'FAILED' : 'ALL PASSED') + ' (' + pass + ' passed, ' + fail + ' failed)');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('CRASH ' + (e && e.stack)); process.exit(2); });
