// AI(W906-TOKEN-IDLE) 20260926: web/page/ht9045_recipe_client.js 的「閒置自動還權杖」離線自我測試。
//   機台 0926 實測：Motor Test 拿到單一操作員權杖後從來不 release，之後 10 分鐘 IO 頁按 Output 都被擋。
//   這支用 node 的 vm 開兩個「頁面」（各自載入一份 recipe client），接到同一個假的 WebBridge 伺服器；
//   假伺服器照 WebBridgeServer.cpp 的權杖規則（control.acquire／release 只看連線；motor.stop、modal.answer
//   免權杖；其他指令要是持有者，否則回 not-operator；別人拿著時 acquire 回 control-held）。
//   不連真的 wb_serve、不讀寫任何檔。用法：node tools/webprobe/token_idle_selftest.cjs
// AI(W906-MERGE-0929) 20260929: 機台 0016＋web 0008（TAKEOVER，EastSun 20260926「我在哪一頁按按鈕，那一頁就把控制權拿回來」）合進來後：
//   * 假伺服器補 control.takeover —— 照 WebBridgeServer.cpp:1388-1396：不管誰拿著，都交給呼叫的連線。
//   * 操作員按的（motor.access 的按鈕動作、真的寫入）每次先送 control.takeover；自動送的（keepAlive、formShow／formClose、
//     dryRun 預覽）照舊 acquire。所以 [1][3][4][5][7][9] 的期望改成新規則；舊的「acquire 被拒 → 重拿重送一次、不無限重試」
//     改用 dryRun 預覽測（[4a][5]）。[3] 把機台 commit 自己記的已知邊角寫明：HOME／Loop 進行中，別頁按寫入一樣會接管。
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
  if (name === 'control.acquire' || name === 'control.takeover') {   // WebBridgeServer.cpp:1388-1396
    if (server.owner === 0 || server.owner === conn.cid || name === 'control.takeover') { server.owner = conn.cid; return reply(true); }
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
const since = (n) => server.log.slice(n).map((s) => s.split(':')[1]).join(',');
const PREVIEW = { dryRun: true };

(async () => {
  const A = makePage('MotorTest'), B = makePage('IoSetView');
  const aid = () => server.log.length;   // (bookmark helper)
  check(typeof A.setTokenHold === 'function' && typeof A.tokenIdleMs === 'function', 'API: setTokenHold／tokenIdleMs 存在');
  check(!!A.tokenIdleMs && A.tokenIdleMs() === 30000, '預設閒置 30 秒');
  if (A.tokenIdleMs) { A.tokenIdleMs(200); B.tokenIdleMs(200); }   // 對照組（舊版沒有這個 API）照樣往下跑，要在 [2] 紅

  console.log('[1] Motor Test 按一顆鈕 → 拿到權杖；IO 頁操作員按寫入 → 接管成功（TAKEOVER；舊行為是 control-held）');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A motor.access ok');
  check(A.status().holdsToken === true && server.owner !== 0, 'A 持有權杖');
  const b1 = aid();
  check(await outcome(B.write('Doc.Data', { S: { K: '1' } })) === 'ok', 'B 寫入成功（接管）');
  check(since(b1) === 'control.takeover,recipe.doc.put', 'B 送的順序＝takeover、寫入（' + since(b1) + '）');
  check(server.owner !== 0 && B.status().holdsToken === true, 'B 持有權杖');

  console.log('[2] 閒置超過 tokenIdleMs → 自動 release；B 再寫就過');
  await sleep(350);
  check(A.status().holdsToken === false && server.owner === 0, 'A、B 都已自動還權杖（伺服器 owner=0）');
  check(await outcome(B.write('Doc.Data', { S: { K: '1' } })) === 'ok', 'B 寫入成功');
  await sleep(350);
  check(server.owner === 0, 'B 用完也自動還');

  console.log('[3] hold：HOME／Loop 進行中不還；但別頁操作員按的一樣會接管（機台 0016 已知邊角），A 下一次按鍵再拿回來');
  let running = true;
  if (A.setTokenHold) A.setTokenHold(() => running);
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 開始 HOME（motor.access ok）');
  await sleep(500);
  check(server.owner !== 0 && A.status().holdsToken === true, 'hold 期間 A 仍持有（過了 2.5 倍閒置時間）');
  check(await outcome(B.write('Doc.Data', { S: { K: '2' } })) === 'ok', 'hold 期間 B 按寫入 → 接管成功');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 送 start:false → 再接管、成功');
  running = false;
  await sleep(500);
  check(server.owner === 0, 'hold 結束後自動還');

  console.log('[4a] 伺服器 10 分鐘收回（本頁以為還拿著）→ dryRun 預覽（acquire 路徑）not-operator → 自動重拿重送一次');
  check(await outcome(A.write('Doc.Data', { S: { K: '4' } }, PREVIEW)) === 'ok', 'A 預覽拿到（acquire）');
  server.owner = 0;                                     // PumpLiveness 的 controlIdleTimeoutMs 收回
  const b4 = aid();
  check(await outcome(A.write('Doc.Data', { S: { K: '4' } }, PREVIEW)) === 'ok', 'A 下一個預覽仍成功（重拿後重送）');
  check(since(b4) === 'recipe.doc.put,control.acquire,recipe.doc.put', '伺服器看到的順序＝被拒、重拿、重送（' + since(b4) + '）');
  console.log('[4b] 同樣被收回，但這次是真的寫入（操作員按的）→ 先 takeover 再寫，不會先被拒');
  server.owner = 0;
  const b4b = aid();
  check(await outcome(A.write('Doc.Data', { S: { K: '4' } })) === 'ok', 'A 寫入成功');
  check(since(b4b) === 'control.takeover,recipe.doc.put', '順序＝takeover、寫入（' + since(b4b) + '）');

  console.log('[5] 伺服器收回後被別人拿走 → dryRun 預覽重拿回 control-held，照原樣報錯、不無限重試');
  server.owner = 999;
  const b5 = aid();
  check(await outcome(A.write('Doc.Data', { S: { K: '5' } }, PREVIEW)) === 'err:control-held', 'A 預覽回 control-held');
  check(server.log.length - b5 === 2, '只多送 2 個指令（recipe.doc.put 被拒＋control.acquire 被拒），沒有迴圈（' + since(b5) + '）');
  server.owner = 0;

  console.log('[6] 免權杖指令（motor.stop）不拿權杖、不影響別人');
  await sleep(350);
  check(await outcome(B.write('Doc.Data', { S: { K: '3' } })) === 'ok', 'B 拿著');
  check(await outcome(A.motorStop(req)) === 'ok', 'A motor.stop 在 B 持有時照樣成功');
  check(A.status().holdsToken === false, 'A 沒有因此拿到權杖');

  console.log('[7] 明確 release 之後下一個按鍵會先 takeover（同一條連線依序處理）');
  await sleep(350);
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 拿到');
  const b7 = aid();
  const r = A.release();                                // 同步把 haveToken 放掉
  const m = A.motorAccess(req);
  check(await outcome(r) === 'ok' && await outcome(m) === 'ok', 'release 與緊接的指令都成功');
  check(since(b7) === 'control.release,control.takeover,motor.access', '順序＝release、takeover、指令（' + since(b7) + '）');

  console.log('[8] 機台版 keepAlive（MotorTest 每 60 秒續權杖）重拿之後，閒置照樣會還（20260926 合併機台版時補）');
  await sleep(350);
  check(server.owner === 0, '起點：沒人持有');
  check(await outcome(A.keepAlive ? A.keepAlive() : Promise.reject(new Error('no keepAlive'))) === 'ok' && server.owner !== 0 && A.status().holdsToken === true, 'keepAlive 拿到權杖');
  await sleep(350);
  check(server.owner === 0 && A.status().holdsToken === false, 'keepAlive 拿到的權杖閒置後自動還（舊版不掛計時，永遠不還）');

  console.log('[9] 運動指令 motor.access：每次按鍵先 takeover 再送一次，被收回也不會重送（NB2 R68／R69 B2：重送的 jog 會落在免權杖的 stop 之後）');
  check(await outcome(A.motorAccess(req)) === 'ok', 'A 拿到（jog 開始）');
  server.owner = 0;                                     // 伺服器收回
  const b9 = aid();
  check(await outcome(A.motorAccess(req)) === 'ok', '下一次按鍵成功');
  check(since(b9) === 'control.takeover,motor.access', '伺服器只看到 takeover＋那一個 motor.access（沒有重送；' + since(b9) + '）');

  console.log((fail ? 'FAILED' : 'ALL PASSED') + ' (' + pass + ' passed, ' + fail + ' failed)');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('CRASH ' + (e && e.stack)); process.exit(2); });
