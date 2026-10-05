// AI(W906-WSLINK) 20260929 [W906] ST01-E3: web/page/ht9045_link.js（一個瀏覽器分頁只開一條 WebSocket）的離線自我測試。
//   Jimmy 20260929：機台 9/26 實測每個 iframe 各開一條連線，23 條撞 16 條上限，Motor Test 整頁沒反應。
//   這支用 node 的 vm 開一個「外框」（background.html，跑 hub）和幾個「iframe」（各自載入 ht9045_link.js），
//   iframe 經 MessageChannel 找外框的 hub；假伺服器照 WebBridgeServer.cpp 的權杖規則（control.acquire／release
//   只看連線；免權杖指令；其他指令要是持有者，否則 not-operator），另外照 WebCmdGuardGlobal 的 400 ms 防連點
//   （cmd+tag+value 相同就 busy，WebCmdGuard.cpp:412）記錄有沒有 hub 重送造成的 busy。
//   [11] 把真的 web/page/ht9045_recipe_client.js 載進兩個 iframe，只把它開連線的那一行換成 HT9045Link.open
//   （＝等 Jimmy 那包合進來之後要改的那一行），驗 Motor Test 還拿著權杖時 IO 頁閒置 release 不會把權杖交還伺服器。
//   不連真的 wb_serve、不讀寫任何檔。用法：node tools/webprobe/ws_link_selftest.cjs
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');
const { MessageChannel } = require('worker_threads');

const WEB = path.join(__dirname, '..', '..', '..', 'web', 'page');
const LINK = process.env.W906_WS_LINK || path.join(WEB, 'ht9045_link.js');
const RECIPE = process.env.W906_RECIPE_CLIENT || path.join(WEB, 'ht9045_recipe_client.js');   // AI(W906-SCREEN-TOKEN) 20261001: control runs point it at the old file
const linkCode = fs.readFileSync(LINK, 'utf8');
const recipeCode = fs.readFileSync(RECIPE, 'utf8');
const URL = 'ws://127.0.0.1:8055/ht9045';

// ---- 假伺服器 --------------------------------------------------------------------------------
const server = { conns: [], owner: 0, nextConn: 1, log: [], busy: 0, guard: {}, seq: 1, tags: { 'machine.state': 'Stop', 'clock.text': '13:00:00' } };
const EXEMPT = new Set(['motor.stop', 'modal.answer', 'dialog.response', 'ui.windows.put', 'cfg.resync', 'log.event', 'sys.ping', 'stream.resync']);
const GUARD_FREE = new Set(['sys.ping', 'cfg.resync', 'log.event', 'ui.windows.put', 'stream.resync', 'motor.stop',
  'control.acquire', 'control.takeover', 'control.release']);   // control.* 在 WebBridgeServer.cpp:1388 的 socket 執行緒就回了，不進 WebCmdGuard
function serverHandle(conn, msg) {
  const reply = (ok, error, extra) => conn._deliver(Object.assign({ type: 'ack', id: msg.id, ok, error: error || '' }, extra || {}));
  if (msg.type !== 'cmd') return reply(true);
  const name = msg.cmd;
  server.log.push({ cid: conn.cid, cmd: name, tag: msg.tag, value: msg.value });
  if (!GUARD_FREE.has(name)) {                       // WebCmdGuardGlobal：同一個 cmd+tag+value 400 ms 內第二次＝busy
    const key = name + '|' + (msg.tag || '') + '|' + (msg.value || '');
    const t = Date.now();
    if (server.guard[key] && t - server.guard[key] < 400) { server.busy++; return reply(false, 'busy: ' + name); }
    server.guard[key] = t;
  }
  if (name === 'motor.stop' && server.refuseStop && (JSON.parse(msg.value || '{}').button === server.refuseStop)) return reply(false, 'refused (test)');   // [19] m11
  if (name === 'control.acquire' || name === 'control.takeover') {
    if (server.owner === 0 || server.owner === conn.cid || name === 'control.takeover') { server.owner = conn.cid; return reply(true); }
    return reply(false, 'control-held');
  }
  if (name === 'control.release') {
    if (server.owner === conn.cid) { server.owner = 0; return reply(true); }
    return reply(false, 'not-operator');
  }
  if (!name.startsWith('auth.') && !EXEMPT.has(name) && server.owner !== conn.cid) return reply(false, 'not-operator');
  return reply(true, '', { state: 'done', result: 'ok', echo: name });
}
class FakeWS {
  constructor(url) {
    this.url = url; this.cid = server.nextConn++; this.readyState = 0;
    server.conns.push(this);
    if (server.down) {   // AI(W906-SCREEN-TOKEN) 20261001: [S10] wb_serve still starting: the attempt errors and closes, never opens
      setTimeout(() => { this.readyState = 3; if (this.onerror) this.onerror({}); if (this.onclose) this.onclose({ code: 1006, reason: '', wasClean: false }); }, 1);
      return;
    }
    setTimeout(() => {
      this.readyState = 1; if (this.onopen) this.onopen({});
      this._deliver({ type: 'snapshot', seq: server.seq, data: Object.assign({}, server.tags) });
    }, 1);
  }
  send(s) { const m = JSON.parse(s); setTimeout(() => { if (this.readyState === 1 && !server.swallow) serverHandle(this, m); }, 2 + (server.slow || 0)); }   // slow: [19] m11; a closed connection's frames are not handled (as WebBridgeServer); swallow: [S11]
  _deliver(o) { if (this.readyState === 1 && this.onmessage) this.onmessage({ data: JSON.stringify(o) }); }
  close() { if (this.readyState === 3) return; this.readyState = 3; if (server.owner === this.cid) server.owner = 0; if (this.onclose) this.onclose({ code: 1000, reason: '', wasClean: true }); }
}
function openConns() { return server.conns.filter((c) => c.readyState === 1).length; }
function pushPatch(data) {
  server.seq++; Object.assign(server.tags, data);
  server.conns.forEach((c) => c._deliver({ type: 'patch', seq: server.seq, data }));
}

// ---- 假視窗 ----------------------------------------------------------------------------------
// AI(W906-SCREEN-TOKEN) 20261001: [1]-[17] / F / F2 test the per-page token rules (screenTakeover:false); [S1]-[S8] test the
//   default (the screen takes the token on connect and keeps it) with CFG_S and windows that can read parent.HT9045Link.
const CFG = { helloWaitMs: 150, pingMs: 40, pingLoss: 3, idleCloseMs: 100, dupReleaseMs: 300, screenTakeover: false };
const CFG_S = { helloWaitMs: 150, pingMs: 5000, pingLoss: 3, idleCloseMs: 100, dupReleaseMs: 300, screenKeepMs: 200, screenRegainMs: 400 };   // pingMs as shipped: a ghost client is NOT hidden by ping loss inside the test (review R2)   // screenTakeover left out = the shipped default
function makeWindow(name, parentWin, opts) {
  const listeners = {};
  const w = {
    console, setTimeout, clearTimeout, setInterval, clearInterval, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
    MessageChannel, WebSocket: FakeWS, HT9045LinkConfig: (opts && opts.cfg) || CFG,
    location: { protocol: 'http:', host: '127.0.0.1:8055', pathname: '/page/' + name + '.html', search: '' },
    document: { addEventListener() {}, removeEventListener() {}, hidden: false },
    XMLHttpRequest: function () { this.open = () => {}; this.send = () => {}; },
    localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
    sessionStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
    addEventListener(t, f) { (listeners[t] || (listeners[t] = [])).push(f); },
    removeEventListener(t, f) { const a = listeners[t] || []; const i = a.indexOf(f); if (i >= 0) a.splice(i, 1); },
    _fire(t, ev) { (listeners[t] || []).slice().forEach((f) => f(ev)); },
    // 別的視窗 postMessage 給這個視窗；source＝一個「這個視窗的子框架」（hub 只收自己 iframe 的 hello）
    postMessage(data, origin, transfer) { setTimeout(() => w._fire('message', { data, ports: transfer || [], source: { parent: w._inner } }), 0); },
  };
  w.window = w; w.self = w; w.globalThis = w;
  vm.createContext(w);
  // node vm：context 裡看到的 global 不是外面這個 sandbox 物件本身（瀏覽器裡 iframe.parent 就是父視窗），
  // 所以 parent／source 都用 context 裡的那個 global，hub 的「只收自己 iframe 的 hello」與 isTop() 才量得到。
  w._inner = vm.runInContext('this', w);
  // 子視窗各拿一個自己的「父視窗代理」：它 postMessage 過去時 ev.source＝w._src（瀏覽器裡就是這個 iframe 的 WindowProxy），
  // hub 才分得出是哪一個 iframe（[17] windowHidden(iframe.contentWindow)）
  if (parentWin) {
    const src = { parent: parentWin._inner };
    w._src = src;
    w.parent = { postMessage(data, origin, transfer) { setTimeout(() => parentWin._fire('message', { data, ports: transfer || [], source: src }), 0); } };
    // [S*]: a same-origin iframe reads parent.HT9045Link (ht9045_link.js parentHasHub, ht9045_recipe_client.js linkWait)
    if (opts && opts.exposeParent) Object.defineProperty(w.parent, 'HT9045Link', { get: () => parentWin.HT9045Link });
  } else w.parent = w._inner;
  w.name = name;
  return w;
}
function load(w, code, file) { vm.runInContext(code, w, { filename: w.name + ':' + file }); }
// Jimmy 那包（55007a32／6f7f9624）合進來之後 ht9045_recipe_client.js 自己就走 HT9045Link.open（:146），這裡原檔照載、不改寫；
// 連線那行被改回去就紅
function recipeViaLink(code) {
  const a = 'try { s = global.HT9045Link ? global.HT9045Link.open(wsUrl()) : new WebSocket(wsUrl()); }';
  if (code.indexOf(a) < 0) throw new Error('recipe client connect line does not go through HT9045Link.open; see ht9045_recipe_client.js:146');
  return code;
}

let fail = 0, pass = 0;
function check(cond, what) { if (cond) { pass++; console.log('  ok   ' + what); } else { fail++; console.log('  FAIL ' + what); } }
// one source line without its // and /* */ comments (quote-aware: 'wss://' stays) -- [17] / [18] check code, not comments
function codeOfLine(line) {
  let out = '', q = '';
  for (let k = 0; k < line.length; k++) {
    const ch = line[k], nx = line[k + 1];
    if (q) { out += ch; if (ch === '\\') { out += nx || ''; k++; } else if (ch === q) q = ''; continue; }
    if (ch === '/' && nx === '/') break;
    if (ch === '/' && nx === '*') { const e = line.indexOf('*/', k + 2); if (e < 0) break; k = e + 1; out += ' '; continue; }
    if (ch === '\'' || ch === '"' || ch === '`') q = ch;
    out += ch;
  }
  return out;
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));  const waitFor = async (cond, ms) => { const t0 = Date.now(); while (!cond() && Date.now() - t0 < (ms || 2000)) await sleep(5); return !!cond(); };   // 機台忙（CPU 100%）時固定 sleep 不夠
function collector(sock) {
  const got = [];
  sock.onmessage = (ev) => got.push(JSON.parse(ev.data));
  return got;
}
function waitOpen(sock, ms) { return new Promise((res) => { if (sock.readyState === 1) return res(true); const t = setTimeout(() => res(false), ms || 1000); sock.addEventListener('open', () => { clearTimeout(t); res(true); }); }); }
const outcome = (p) => p.then(() => 'ok', (e) => 'err:' + (e && e.message));
const cmds = (name) => server.log.filter((x) => x.cmd === name);

(async () => {
  const frame = makeWindow('background', null);
  load(frame, linkCode, 'ht9045_link.js');
  const hub = frame.HT9045Link.startHub(URL);
  const A = makeWindow('HW.MotorTest', frame), B = makeWindow('HW.IoSetView', frame);
  load(A, linkCode, 'ht9045_link.js'); load(B, linkCode, 'ht9045_link.js');

  console.log('[1] 外框＋兩個 iframe 各開一條 → 伺服器只有 1 條連線');
  const sF = frame.HT9045Link.open(URL, 'frame'), sA = A.HT9045Link.open(URL, 'A'), sB = B.HT9045Link.open(URL, 'B');
  const gF = collector(sF), gA = collector(sA), gB = collector(sB);
  check(await waitOpen(sF) && await waitOpen(sA) && await waitOpen(sB), '三個虛擬 socket 都 open');
  check(server.conns.length === 1 && openConns() === 1, '伺服器連線數 = 1（原本 3）');
  check(sF.via === 'hub' && sA.via === 'relay' && sB.via === 'relay', '外框走 hub、iframe 走 relay（' + [sF.via, sA.via, sB.via].join('/') + '）');
  await waitFor(() => [gF, gA, gB].every((g) => g.some((m) => m.type === 'snapshot')));
  check([gF, gA, gB].every((g) => g.some((m) => m.type === 'snapshot' && m.data['machine.state'] === 'Stop')), '三個都收到 snapshot');

  console.log('[2] 兩個 iframe 都用 id 1 → hub 換成不同的 id，ack 只回給發的那一個、id 還原');
  gA.length = 0; gB.length = 0;
  sA.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'sys.ping', tag: 'A' }));
  sB.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'sys.ping', tag: 'B' }));
  await waitFor(() => gA.some((m) => m.type === 'ack') && gB.some((m) => m.type === 'ack'));
  const aA = gA.filter((m) => m.type === 'ack'), aB = gB.filter((m) => m.type === 'ack');
  check(aA.length === 1 && aA[0].id === 1 && aB.length === 1 && aB[0].id === 1, '各自收到一個 ack，id 都是 1');
  const pings = cmds('sys.ping');
  check(pings.length === 2 && pings[0].tag === 'A' && pings[1].tag === 'B', '伺服器收到兩個 ping，各一次');

  console.log('[3] patch 扇出給所有頁面；hub 記下快取');
  gF.length = 0; gA.length = 0; gB.length = 0;
  pushPatch({ 'clock.text': '13:00:01', 'io.x': null });
  await waitFor(() => [gF, gA, gB].every((g) => g.some((m) => m.type === 'patch')));
  check([gF, gA, gB].every((g) => g.some((m) => m.type === 'patch' && m.data['clock.text'] === '13:00:01')), '三個都收到 patch');
  check(hub.cache['clock.text'] === '13:00:01' && 'io.x' in hub.cache && hub.cache['io.x'] === null, '快取有新值，null 照存（不是刪掉）');

  console.log('[4] 晚開的 iframe 收到合成的 snapshot（快取內容＋最後的 seq），不另開連線');
  const C = makeWindow('HW.teach', frame); load(C, linkCode, 'ht9045_link.js');
  const sC = C.HT9045Link.open(URL, 'C'); const gC = collector(sC);
  check(await waitOpen(sC), 'C open');
  await waitFor(() => gC.some((m) => m.type === 'snapshot'));
  const snapC = gC.find((m) => m.type === 'snapshot');
  check(!!snapC && snapC.synthetic === true && snapC.data['clock.text'] === '13:00:01' && snapC.seq === server.seq, '合成 snapshot：快取值＋seq=' + server.seq);
  check(server.conns.length === 1, '連線數仍是 1');

  console.log('[5] 權杖由 hub 管：A 拿（伺服器 1 次）、B 拿（本地回 ok）、A 放（本地）、B 放（這時才送伺服器）');
  gA.length = 0; gB.length = 0;
  const acq0 = cmds('control.acquire').length;
  sA.send(JSON.stringify({ type: 'cmd', id: 11, cmd: 'control.acquire' })); await waitFor(() => hub.token.held && gA.some((m) => m.id === 11));
  sB.send(JSON.stringify({ type: 'cmd', id: 12, cmd: 'control.acquire' })); await waitFor(() => gB.some((m) => m.id === 12));
  check(cmds('control.acquire').length - acq0 === 1, '伺服器只收到 1 個 acquire');
  check(gB.some((m) => m.type === 'ack' && m.id === 12 && m.ok && m.link === 'local'), 'B 的 acquire 由 hub 本地回 ok');
  sA.send(JSON.stringify({ type: 'cmd', id: 13, cmd: 'control.release' })); await waitFor(() => gA.some((m) => m.id === 13));
  check(server.owner === server.conns[0].cid && cmds('control.release').length === 0, 'A 放掉：B 還拿著 → 沒送伺服器，伺服器仍是持有者');
  sB.send(JSON.stringify({ type: 'cmd', id: 14, cmd: 'control.release' })); await waitFor(() => gB.some((m) => m.id === 14) && server.owner === 0);
  check(server.owner === 0 && cmds('control.release').length === 1, 'B 放掉（最後一個）→ 才送 release，伺服器 owner=0');

  console.log('[6] A、B 同時 acquire（沒人拿著）→ 只送一個，兩個都拿到 ok（不會有第二個被 busy）');
  await sleep(450);                                                                       // 跟 [5] 隔開（control.* 真的伺服器不經 WebCmdGuard，假伺服器也一樣；留著不影響）
  gA.length = 0; gB.length = 0;
  const acq1 = cmds('control.acquire').length, busy0 = server.busy;
  sA.send(JSON.stringify({ type: 'cmd', id: 21, cmd: 'control.acquire' }));
  sB.send(JSON.stringify({ type: 'cmd', id: 22, cmd: 'control.acquire' }));
  await waitFor(() => gA.some((m) => m.id === 21) && gB.some((m) => m.id === 22));
  check(cmds('control.acquire').length - acq1 === 1 && server.busy === busy0, '伺服器 1 個 acquire、0 個 busy');
  check(gA.some((m) => m.id === 21 && m.ok) && gB.some((m) => m.id === 22 && m.ok), 'A、B 都收到 ok');

  console.log('[7] 伺服器收回權杖（閒置 10 分鐘）→ 下一個指令 not-operator → 所有頁面收到 link.token owner:false');
  gF.length = 0; gA.length = 0; gB.length = 0; gC.length = 0;
  server.owner = 0;
  sA.send(JSON.stringify({ type: 'cmd', id: 31, cmd: 'recipe.doc.put', tag: 'x', value: '{}' }));
  await waitFor(() => gA.some((m) => m.id === 31) && [gF, gA, gB, gC].every((g) => g.some((m) => m.type === 'link.token')));
  check(gA.some((m) => m.type === 'ack' && m.id === 31 && m.error === 'not-operator'), 'A 收到 not-operator');
  check([gF, gA, gB, gC].every((g) => g.some((m) => m.type === 'link.token' && m.owner === false)), '四個頁面都收到 link.token owner:false');
  check(hub.token.held === false, 'hub 記成沒有權杖');

  // AI(W906-WSLINK) 20260930 ST01-E3: M1 (St02-E2 review B) -- page gone never sends a STOP; only held jogs are released
  console.log('[8] 頁面不見了 → 只放開它還按著的 jog（自己的按鈕、只那一軸）；從來不送 STOP（M1：原本 link.pageGone＝整台 StopAllMotor）');
  const ma8 = (id, v) => JSON.stringify({ type: 'cmd', id, cmd: 'motor.access', tag: (v.motors || [])[0] || '', value: JSON.stringify(v) });
  sA.send(JSON.stringify({ type: 'cmd', id: 41, cmd: 'control.acquire' })); await waitFor(() => gA.some((m) => m.id === 41));
  sA.send(ma8(42, { source: 'uMotorTest', action: 'formShow', button: 'FormShow', kind: 'control', motors: [] }));
  sA.send(ma8(43, { source: 'uteach', action: 'teachSet', button: 'SetButton030', kind: 'motion', motors: ['MInArmX'], params: { query: true } }));
  sA.send(ma8(44, { source: 'uMotorTest', action: 'moveAbsolute', button: 'btnMoveAbs', kind: 'motion', motors: ['MInArmX'] }));
  await waitFor(() => gA.some((m) => m.id === 44));
  const stop0 = cmds('motor.stop').length;
  sA.close(); await sleep(80);
  check(cmds('motor.stop').length === stop0,
        'M1：只送過 formShow／Teach 載入查詢／一般移動的頁面不見了（例：生產中重新整理 Motor Test）→ 什麼都不送（原本是整台 STOP）');
  const A8 = makeWindow('HW.MotorTest8', frame); load(A8, linkCode, 'ht9045_link.js');
  const s8 = A8.HT9045Link.open(URL, 'A8'); const g8 = collector(s8);
  check(await waitOpen(s8), 'setup: 另一個 Motor Test iframe');
  s8.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.acquire' })); await waitFor(() => g8.some((m) => m.id === 1));
  s8.send(ma8(2, { source: 'uMotorTest', action: 'jogP', button: 'sbMotorTest_JogP', kind: 'motion', motors: ['MInArmX'] }));
  s8.send(ma8(3, { source: 'uMotorTest', action: 'jogN', button: 'sbMotorTest_JogN', kind: 'motion', motors: ['MOutArmY'] }));
  s8.send(ma8(4, { source: 'uMotorTest', action: 'formClose', button: 'FormClose', kind: 'control', motors: [] }));
  await waitFor(() => g8.some((m) => m.id === 4));
  const st8 = cmds('motor.stop').length;
  s8.close(); await waitFor(() => cmds('motor.stop').length >= st8 + 2);
  await sleep(60);
  const r8 = cmds('motor.stop').slice(st8).map((x) => JSON.parse(x.value));
  const b8 = r8.map((v) => v.button + '=' + JSON.stringify(v.motors)).sort().join(' ');
  check(r8.length === 2 && b8 === 'sbMotorTest_JogN=["MOutArmY"] sbMotorTest_JogP=["MInArmX"]' &&
        r8.every((v) => v.action === 'stop' && v.source === 'uMotorTest' && /^link[.]pageGone:/.test(v.reason)),
        '按著兩個 jog 的頁面不見了 → 每個 jog 剛好一個放開，按鈕是它自己的（DoJogRelease：只那一軸）（' + b8 + '）');
  check(!r8.some((v) => v.button === 'link.pageGone' || !/jog/i.test(v.button)), '沒有 link.pageGone、沒有任何非 jog 按鈕的停止（不會走伺服器的 DoStop）');
  check(server.conns.length === 1 && openConns() === 1, '連線還在（其他頁面仍開著）');

  console.log('[9] iframe 不回 ping（當掉）→ 3 次後 hub 把它移掉；沒動過馬達的不送 stop');
  const deadCh = new MessageChannel();
  let welcomed = false;
  deadCh.port1.onmessage = (e) => { if (e.data && e.data.k === 'welcome') welcomed = true; };   // 故意不回 pong
  frame.postMessage({ ht9045link: 'hello', v: 1, url: URL, name: 'dead' }, '*', [deadCh.port2]);
  await waitFor(() => welcomed && hub.status().clients.some((c) => c.name === 'dead'));
  const n0 = hub.status().clients.length;
  check(welcomed && hub.status().clients.some((c) => c.name === 'dead'), '死掉的頁面先被收進來（welcomed=' + welcomed + ' clients=' + hub.status().clients.map((c) => c.name).join(',') + ' state=' + hub.state + '）');
  const s1 = cmds('motor.stop').length;
  await sleep(CFG.pingMs * (CFG.pingLoss + 3));
  check(!hub.status().clients.some((c) => c.name === 'dead') && hub.status().clients.length === n0 - 1, 'ping 遺失後被移掉');
  check(cmds('motor.stop').length === s1, '沒動過馬達 → 不送 stop');
  deadCh.port1.close();

  console.log('[10] 退路：父視窗沒有 hub → 直接開真的連線（今天的行為）；別的 URL 也直接開');
  const lone = makeWindow('lonely-frame', null); load(lone, linkCode, 'ht9045_link.js');   // 有 link、沒 startHub
  const D = makeWindow('Standalone', lone); load(D, linkCode, 'ht9045_link.js');
  const c0 = server.conns.length;
  const sD = D.HT9045Link.open(URL, 'D');
  check(await waitOpen(sD, 1000) && sD.via === 'direct' && server.conns.length === c0 + 1, '回 nohub → 直連（多 1 條）');
  const bare = makeWindow('no-link-parent', null);                                         // 父視窗根本沒有 link
  const E = makeWindow('OldFrameChild', bare); load(E, linkCode, 'ht9045_link.js');
  const t0 = Date.now(); const sE = E.HT9045Link.open(URL, 'E');
  check(await waitOpen(sE, 1000) && sE.via === 'direct' && Date.now() - t0 >= CFG.helloWaitMs, '等 hello 逾時 → 直連');
  const top = makeWindow('top-no-hub', null); load(top, linkCode, 'ht9045_link.js');
  const sT = top.HT9045Link.open(URL, 'T');
  check(await waitOpen(sT, 500) && sT.via === 'direct', '最上層沒有 hub → 立刻直連（不等）');
  const sSim = A.HT9045Link.open('ws://127.0.0.1:9045/ht9045-json/', 'sim');
  check(await waitOpen(sSim, 1000) && sSim.via !== 'relay', '模擬器的 URL 不走 hub（外框的 hub 只收 bridge 那條）');
  [sD, sE, sT, sSim].forEach((s) => s.close());
  await sleep(20);

  console.log('[11] 真的 recipe client（Motor Test、IO 兩頁）經 hub：同一瀏覽器＝同一操作員；IO 閒置 release 不會放掉 Motor Test 的權杖');
  const MT = makeWindow('HW.MotorTest2', frame), IO = makeWindow('HW.IoSetView2', frame);
  [MT, IO].forEach((w) => { load(w, linkCode, 'ht9045_link.js'); load(w, recipeViaLink(recipeCode), 'ht9045_recipe_client.js'); });
  const RM = MT.HT9045Recipe, RI = IO.HT9045Recipe;
  const req = { motors: ['MInArmY'], action: 'move', button: 'btnGo', source: 'motortest' };
  if (RM.tokenIdleMs) { RM.tokenIdleMs(150); RI.tokenIdleMs(150); }
  let homing = true;
  if (RM.setTokenHold) RM.setTokenHold(() => homing);
  const connsBefore = server.conns.length, busy1 = server.busy;
  const o11 = await outcome(RM.motorAccess(req));
  check(o11 === 'ok', 'Motor Test motor.access ok（經 hub 拿權杖）（' + o11 + '）');
  check(await outcome(RI.write('Doc.Data', { S: { K: '1' } })) === 'ok', 'IO 頁寫入 ok（同一瀏覽器同一條連線：takeover 到伺服器 owner 不變；原本兩條連線會 control-held）');
  await sleep(400);                                                                       // IO 閒置 → release
  check(server.owner !== 0, 'IO 閒置放掉後，伺服器上權杖仍在（Motor Test hold 中）');
  const no0 = server.log.length;
  check(await outcome(RM.motorAccess(req)) === 'ok', 'Motor Test 下一個 motor.access 仍 ok');
  check(!server.log.slice(no0).some((x) => x.cmd === 'control.acquire'), '而且不必重拿（沒有被 not-operator）');
  homing = false;
  await sleep(400);
  check(server.owner === 0, 'hold 結束、兩頁都閒置 → 最後一個放掉時才還伺服器');
  check(server.conns.length === connsBefore, '兩個 recipe client 沒有多開連線（' + connsBefore + ' 條不變）');
  check(server.busy === busy1, '整段 0 個 busy（hub 沒有重送）');

  console.log('[12] 外框 hub 的真連線斷掉 → 所有虛擬 socket 收到 close；下一次 open 重連，仍只有 1 條');
  const hubConn = server.conns.find((c) => c.readyState === 1 && c.url === URL && hub.sock === c);
  let closedB = false; sB.onclose = () => { closedB = true; };
  hubConn.close();
  await waitFor(() => closedB);
  check(closedB && sB.readyState === 3, 'B 的虛擬 socket 收到 close');
  const before12 = server.conns.length;
  const sB2 = B.HT9045Link.open(URL, 'B2'), sF2 = frame.HT9045Link.open(URL, 'F2');
  check(await waitOpen(sB2) && await waitOpen(sF2) && server.conns.length === before12 + 1, '重連：兩個頁面共用 1 條新連線');

  // AI(W906-WSLINK) 20260929 ST01-E3: [13]-[16] after Jimmy's TAKEOVER pack (55007a32 / 6f7f9624 / 8c6ac9a0) and his conditions ① / ②
  //   (main TO_STEVEN.md section 4, 14:2x): cancel on bye / heartbeat loss; an in-browser hand-over must not cancel.
  console.log('[13] 同一瀏覽器換頁接手（takeover）→ 送伺服器、owner 不變、不送 motor.stop（Jimmy ②）；別的瀏覽器拿走後，這邊按一下就拿回來');
  const M3 = makeWindow('HW.MotorTest3', frame); load(M3, linkCode, 'ht9045_link.js');
  const sM3 = M3.HT9045Link.open(URL, 'M3'); const gM3 = collector(sM3), gB2 = collector(sB2);
  check(await waitOpen(sM3), 'M3 open');
  sM3.send(JSON.stringify({ type: 'cmd', id: 51, cmd: 'control.acquire' })); await waitFor(() => gM3.some((m) => m.id === 51));
  sM3.send(JSON.stringify({ type: 'cmd', id: 52, cmd: 'motor.access', tag: 'MInArmX',
    value: JSON.stringify({ source: 'uMotorTest', action: 'jogP', motors: ['MInArmX'], button: 'sbMotorTest_JogP' }) }));
  await waitFor(() => gM3.some((m) => m.id === 52));
  const hubCid = hub.sock.cid, tk0 = cmds('control.takeover').length, st13 = cmds('motor.stop').length;
  check(server.owner === hubCid && gM3.some((m) => m.id === 52 && m.ok), 'setup: Motor Test 拿著權杖、正在 jog');
  sB2.send(JSON.stringify({ type: 'cmd', id: 53, cmd: 'control.takeover' })); await waitFor(() => gB2.some((m) => m.id === 53));
  check(cmds('control.takeover').length - tk0 === 1 && gB2.some((m) => m.type === 'ack' && m.id === 53 && m.ok), 'IO 頁 takeover → 送伺服器一次、ok');
  check(server.owner === hubCid && cmds('motor.stop').length === st13,
        'owner 還是同一條連線（wb_serve W906_OwnerHeldSince 看不到換手）、沒有 motor.stop → Motor Test 的 jog 不被取消');
  const other = new FakeWS(URL); await waitFor(() => other.readyState === 1);                                         // 另一個瀏覽器（直連）
  other.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.takeover' })); await waitFor(() => server.owner === other.cid);
  check(server.owner === other.cid, 'setup: 另一個瀏覽器 takeover（伺服器 owner＝它）');
  const tk1 = cmds('control.takeover').length;
  sM3.send(JSON.stringify({ type: 'cmd', id: 54, cmd: 'control.takeover' })); await waitFor(() => gM3.some((m) => m.id === 54));
  check(cmds('control.takeover').length - tk1 === 1 && server.owner === hubCid && gM3.some((m) => m.id === 54 && m.ok),
        'Motor Test 按一下（takeover）：hub 以為自己還拿著也照送伺服器 → 權杖拿回來（原本就地回 ok、下一個指令才 not-operator）');
  const tk2 = cmds('control.takeover').length;
  sM3.send(JSON.stringify({ type: 'cmd', id: 55, cmd: 'control.takeover' }));
  sB2.send(JSON.stringify({ type: 'cmd', id: 56, cmd: 'control.takeover' }));
  await waitFor(() => gM3.some((m) => m.id === 55) && gB2.some((m) => m.id === 56));
  check(cmds('control.takeover').length - tk2 === 1 && gM3.some((m) => m.id === 55 && m.ok) && gB2.some((m) => m.id === 56 && m.ok),
        '兩頁同時 takeover → 伺服器只收到 1 個、兩頁都 ok');

  console.log('[14] 動過馬達的頁面：pagehide（F5／關分頁的 bye）、當掉（ping 漏 3 次）都算不見 → hub 代送 motor.stop（Jimmy ①）');
  const st14 = cmds('motor.stop').length;
  M3._fire('pagehide', {}); await waitFor(() => cmds('motor.stop').length > st14);
  let s14 = cmds('motor.stop').slice(st14).map((x) => JSON.parse(x.value));
  check(s14.length === 1 && JSON.stringify(s14[0].motors) === '["MInArmX"]' && s14[0].button === 'sbMotorTest_JogP' &&
        s14[0].reason === 'link.pageGone:bye' && s14[0].source === 'uMotorTest',
        'pagehide → bye → 放開它按著的 sbMotorTest_JogP（MInArmX，reason=link.pageGone:bye，不是 STOP）');
  const hung = new MessageChannel();
  let hungWelcomed = false;
  hung.port1.onmessage = (e) => {                                                         // 送一個 jog 之後就當掉：不回 pong
    if (!e.data || e.data.k !== 'welcome') return;
    hungWelcomed = true;
    hung.port1.postMessage({ k: 'send', data: JSON.stringify({ type: 'cmd', id: 1, cmd: 'motor.access', tag: 'MOutArmY',
      value: JSON.stringify({ source: 'uteach', action: 'jogP', motors: ['MOutArmY'], button: 'btnJogP' }) }) });
  };
  const st14b = cmds('motor.stop').length;
  frame.postMessage({ ht9045link: 'hello', v: 1, url: URL, name: 'hung-teach' }, '*', [hung.port2]);
  await waitFor(() => cmds('motor.stop').length > st14b, 3000);
  s14 = cmds('motor.stop').slice(st14b).map((x) => JSON.parse(x.value));
  check(hungWelcomed && s14.length === 1 && JSON.stringify(s14[0].motors) === '["MOutArmY"]' && s14[0].button === 'btnJogP' &&
        s14[0].reason === 'link.pageGone:ping lost' && s14[0].source === 'uteach',
        '當掉的 Teach 頁（按著 btnJogP）→ 放開那一個 jog（MOutArmY，reason=link.pageGone:ping lost，不是 STOP）');
  hung.port1.close();

  console.log('[15] 真的 recipe client、頁面沒載 ht9045_link.js：自己載同資料夾那支、等它、走 hub；TK-2 的載入查詢照舊 acquire；載不到 → 直連');
  function withLoader(w, loadIt) {
    let appended = null;
    w.document = { addEventListener() {}, removeEventListener() {}, hidden: false,
      currentScript: { src: 'http://127.0.0.1:8055/page/ht9045_recipe_client.js?v=7' },
      createElement: (t) => ({ tagName: t }),
      head: { appendChild: (el) => { appended = el; setTimeout(() => { if (loadIt) { load(w, linkCode, 'ht9045_link.js'); el.onload(); } else el.onerror(); }, 30); } } };
    return () => appended;
  }
  const T15 = makeWindow('HW.teach', frame);
  const app15 = withLoader(T15, true);
  load(T15, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
  check(!!app15() && app15().src === 'http://127.0.0.1:8055/page/ht9045_link.js', '載入同資料夾的 ht9045_link.js（' + (app15() && app15().src) + '）');
  const RT = T15.HT9045Recipe, conns15 = server.conns.length, acq15 = cmds('control.acquire').length, tk15 = cmds('control.takeover').length;
  const q15 = { motors: ['MInArmX'], action: 'teachSet', button: 'SetButton030', source: 'uteach', params: { query: true } };
  const o15 = await outcome(RT.motorAccess(q15));
  check(o15 === 'ok' && server.conns.length === conns15 && T15.HT9045Link.status().relayedSockets === 1,
        'Teach 頁的 recipe client 走 relay、伺服器連線數不變（' + o15 + '）');
  check(cmds('control.takeover').length === tk15 && cmds('control.acquire').length === acq15,
        'TK-2：載入查詢（params.query）＝自動 → acquire（hub 拿著 → 本地回），不 takeover');
  const F15 = makeWindow('HW.OldPage', frame);
  withLoader(F15, false);
  load(F15, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
  const conns15b = server.conns.length;
  await outcome(F15.HT9045Recipe.motorAccess({ motors: [], action: 'formShow', source: 'uMotorTest' }));
  check(server.conns.length === conns15b + 1 && !F15.HT9045Link, '載不到 ht9045_link.js → 照舊直接開 WebSocket（多 1 條）');

  console.log('[16] link.token owner:false → recipe client 忘掉權杖：下一個自動動作重新 acquire（不帶著過期的 haveToken 送出去）');
  other.send(JSON.stringify({ type: 'cmd', id: 2, cmd: 'control.takeover' })); await waitFor(() => server.owner === other.cid);  // 另一個瀏覽器又拿走
  sB2.send(JSON.stringify({ type: 'cmd', id: 61, cmd: 'recipe.doc.put', tag: 'x', value: '{}' })); await waitFor(() => gB2.some((m) => m.id === 61) && !hub.token.held);
  check(gB2.some((m) => m.id === 61 && m.error === 'not-operator') && hub.token.held === false, 'setup: IO 頁 not-operator → hub 通知每一頁 link.token');
  const acq16 = cmds('control.acquire').length;
  const o16 = await outcome(RT.motorAccess(Object.assign({}, q15, { button: 'SetButton031' })));
  check(cmds('control.acquire').length === acq16 + 1 && o16 === 'err:control-held',
        'Teach 頁下一個自動查詢 → 先送 control.acquire（別的瀏覽器拿著 → control-held，照原樣報）（' + o16 + '）');
  other.close();

  // AI(W906-WSLINK) 20260929 ST01-E3: [17] Jimmy 20260929 -- the frame's closeWin / minimizeWin only set display:none
  //   (the iframe stays loaded, no bye, pings go on), so [8] / [14] never fire on a normal close; HW.teach.html has
  //   no HT_WIN release. background.html setWinState (st != 'open') -> HT9045Link.windowHidden(iframe.contentWindow).
  console.log('[17] 關窗／縮小（display:none，頁面沒卸載）→ hub 放開那一頁還按著的 jog（只那一軸，不是整台 STOP）');
  const T17 = makeWindow('HW.teach17', frame), M17 = makeWindow('HW.MotorTest17', frame);
  load(T17, linkCode, 'ht9045_link.js'); load(M17, linkCode, 'ht9045_link.js');
  const sT17 = T17.HT9045Link.open(URL, 'T17'), sM17 = M17.HT9045Link.open(URL, 'M17');
  const gT17 = collector(sT17), gM17 = collector(sM17);
  check(await waitOpen(sT17) && await waitOpen(sM17), 'setup: Teach 與 Motor Test 兩個 iframe 經 hub 開好');
  sT17.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.acquire' }));
  await waitFor(() => gT17.some((m) => m.id === 1));
  const jogReq = (id, source, button, action, motor) => JSON.stringify({ type: 'cmd', id, cmd: 'motor.access', tag: motor,
    value: JSON.stringify({ source, button, action, kind: 'motion', motors: [motor] }) });
  const relReq = (id, source, button, motor) => JSON.stringify({ type: 'cmd', id, cmd: 'motor.stop', tag: motor,
    value: JSON.stringify({ source, button, action: 'stop', kind: 'control', motors: motor ? [motor] : [] }) });
  const ma0 = cmds('motor.access').length;
  sT17.send(jogReq(2, 'uteach', 'btnJogP', 'jogP', 'MInArmX'));
  sM17.send(jogReq(3, 'uMotorTest', 'sbMotorTest_JogP', 'jogP', 'MOutArmY'));
  await waitFor(() => cmds('motor.access').length >= ma0 + 2);
  let st17 = cmds('motor.stop').length;
  let n17 = frame.HT9045Link.windowHidden(T17._src, 'closed');
  await waitFor(() => cmds('motor.stop').length > st17);
  let s17 = cmds('motor.stop').slice(st17).map((x) => JSON.parse(x.value));
  check(n17 === 1 && s17.length === 1 && s17[0].button === 'btnJogP' && JSON.stringify(s17[0].motors) === '["MInArmX"]' &&
        s17[0].action === 'stop' && s17[0].source === 'uteach' && /windowHidden:closed/.test(s17[0].reason),
        '關 Teach（按著 btnJogP）→ 一個 motor.stop，button＝btnJogP（伺服器 DoJogRelease：只停 MInArmX），source＝uteach');
  check(!s17.some((v) => v.button === 'link.pageGone' || v.button === 'btnStop'), '不是整台 STOP（別頁的 HOME／Loop 不受影響）');
  await sleep(60);
  check(cmds('motor.stop').length === st17 + 1, 'Motor Test 還按著的 jog 沒有被放開（不是它的視窗）');
  st17 = cmds('motor.stop').length;
  n17 = frame.HT9045Link.windowHidden(T17._src, 'closed'); await sleep(60);
  check(n17 === 0 && cmds('motor.stop').length === st17, '同一個視窗再關一次：已經放開了，什麼都不送');
  sM17.send(relReq(4, 'uMotorTest', 'sbMotorTest_JogP', 'MOutArmY'));
  await waitFor(() => cmds('motor.stop').length > st17);
  st17 = cmds('motor.stop').length;
  n17 = frame.HT9045Link.windowHidden(M17._src, 'minimized'); await sleep(60);
  check(n17 === 0 && cmds('motor.stop').length === st17, 'Motor Test 自己放開之後再縮小：沒有按著的 jog，什麼都不送');
  sM17.send(jogReq(5, 'uMotorTest', 'sbMotorTest_JogN', 'jogN', 'MOutArmY'));
  await waitFor(() => cmds('motor.access').length >= ma0 + 3);
  n17 = frame.HT9045Link.windowHidden(M17._src, 'minimized');
  await waitFor(() => cmds('motor.stop').length > st17);
  s17 = cmds('motor.stop').slice(st17).map((x) => JSON.parse(x.value));
  check(n17 === 1 && s17.length === 1 && s17[0].button === 'sbMotorTest_JogN' && /windowHidden:minimized/.test(s17[0].reason),
        '縮小 Motor Test（按著 jogN）→ 放開 sbMotorTest_JogN（縮小時手指也放不開）');
  st17 = cmds('motor.stop').length;
  sT17.send(jogReq(6, 'uteach', 'btnJogP', 'jogP', 'MInArmX'));
  sT17.send(relReq(7, 'uteach', 'btnJogN', 'MInArmX'));                                   // 放開的是另一顆（沒按著）
  await waitFor(() => cmds('motor.stop').length > st17);
  st17 = cmds('motor.stop').length;
  n17 = frame.HT9045Link.windowHidden(T17._src, 'closed');
  await waitFor(() => cmds('motor.stop').length > st17);
  check(n17 === 1 && JSON.parse(cmds('motor.stop').slice(-1)[0].value).button === 'btnJogP',
        '放開另一顆 jog 鈕不會清掉真正按著的 btnJogP → 關窗時照樣放開它');
  st17 = cmds('motor.stop').length;
  sT17.send(jogReq(8, 'uteach', 'btnJogP', 'jogP', 'MInArmX'));
  sT17.send(relReq(9, 'uteach', 'btnStop', ''));                                          // STOP：golden StopAllMotor
  await waitFor(() => cmds('motor.stop').length > st17);
  st17 = cmds('motor.stop').length;
  n17 = frame.HT9045Link.windowHidden(T17._src, 'closed'); await sleep(60);
  check(n17 === 0 && cmds('motor.stop').length === st17, '按過 STOP（btnStop）之後關窗：STOP 已經停了全部，不再多送');
  n17 = frame.HT9045Link.windowHidden({ parent: null }, 'closed') + frame.HT9045Link.windowHidden(null, 'closed');
  check(n17 === 0, '不認得的視窗／null：什麼都不做');
  check(hub.status().clients.some((c) => c.name === 'T17') && hub.status().clients.some((c) => c.name === 'M17'),
        '藏起來的頁面仍是 hub 的 client（沒卸載、連線還在；bye／ping 遺失那條路照舊）');
  {
    // AI(W906-WSLINK) 20260929 ST01-E3: compare the CODE only -- every line's // and /* */ comments are cut first (quotes are
    //   respected, so 'wss://' stays). 3f95f0c8 appended the call AFTER :668's // comment, which made it dead code
    //   (ST01-E fixed it in fd4ecb82); the old text-only match here passed anyway. W906_WS_BG = another background.html (control).
    const codeOf = codeOfLine;
    const bg =fs.readFileSync(process.env.W906_WS_BG || path.join(WEB, '..', 'background.html'), 'utf8');
    const i = bg.indexOf('function setWinState(');
    const code = (i >= 0 ? bg.slice(i, bg.indexOf('\n}', i)) : '').split('\n').map(codeOf).join('\n');
    check(/st!=='open'/.test(code) && /HT9045Link\.windowHidden\(hf\.contentWindow,st\)/.test(code) && /querySelector\('iframe'\)/.test(code),
          'background.html setWinState：狀態不是 open（closed／minimized）就呼叫 HT9045Link.windowHidden(iframe.contentWindow)——在程式裡，不在 // 註解後面');
  }

  // AI(W906-WSLINK) 20260929 ST01-E3: [18] Jimmy 「可以，請 ST01-E3 做」 -- HW.teach.html's own release (the HW.MotorTest.html
  //   pattern). The code part of the teach page's line (comments cut) runs in a sandbox with a fake window / HTMotorAccess.
  console.log('[18] HW.teach.html 頁面這一側的放開：HT_WIN 關／縮小、pagehide → HTMotorAccess.releaseHeld()（同 Motor Test）');
  {
    const tp = process.env.W906_WS_TEACH || path.join(WEB, 'HW.teach.html');
    const line = fs.readFileSync(tp, 'utf8').split('\n').find((l) => /teachOn\('btnJogN','mouseup'/.test(l)) || '';
    const code = codeOfLine(line.replace(/\r$/, ''));
    const ls = {}, parent = { name: 'frame' }, other = { name: 'someone' };
    let rel = 0;
    const win = { addEventListener(t, f) { (ls[t] || (ls[t] = [])).push(f); }, parent };
    const ctx = { window: win, HTMotorAccess: { releaseHeld() { rel++; }, release() {} }, teachOn() {} };
    let ran = true;
    try { vm.runInNewContext(code, ctx); } catch (e) { ran = false; console.log('    (' + e.message + ')'); }
    const fire = (t, ev) => (ls[t] || []).forEach((f) => f(ev || {}));
    const msg = (d, src) => fire('message', { data: d, source: src || parent });
    check(ran && (ls.message || []).length === 1 && (ls.pagehide || []).length === 1, '掛了一個 message、一個 pagehide 監聽（程式在 // 註解前面，真的會跑）');
    msg({ type: 'HT_WIN', id: 'teach', open: true, state: 'open', initial: true });
    check(rel === 0, 'HT_WIN open（含 initial）→ 不放開');
    msg({ type: 'HT_WIN', id: 'teach', open: false, state: 'closed' });
    check(rel === 1, 'HT_WIN closed → releaseHeld 一次');
    msg({ type: 'HT_WIN', id: 'teach', open: true, state: 'minimized' });
    check(rel === 2, 'HT_WIN minimized（open 仍是 true）→ 也放開（藏起來的 iframe 收不到 pointerup）');
    msg({ type: 'HT_WIN', id: 'teach', open: false });
    check(rel === 3, '舊外框沒有 state：看 open=false → 放開');
    msg({ type: 'HT_WIN', id: 'teach', open: false, state: 'closed' }, other);
    msg({ type: 'HT_LANG', lang: 'en' });
    check(rel === 3, '不是父視窗送的 HT_WIN、別的訊息 → 不理');
    fire('pagehide');
    check(rel === 4, 'pagehide（F5／重載／關分頁）→ releaseHeld');
  }

  // AI(W906-WSLINK) 20260930 ST01-E3: [19] E-017 m11 (St02-E2 review B) -- a window close sent two identical jog releases
  //   (the hub's windowHidden, then the page's own HT_WIN -> releaseHeld). The page's copy now rides on the hub's release.
  console.log('[19] 關窗時同一個 jog 只放開一次：頁面自己的放開等 hub 那次的結果（成功＝回 ok；被拒＝照送當重試）；STOP 一律照送（m11）');
  {
    const P = makeWindow('HW.teach19', frame); load(P, linkCode, 'ht9045_link.js');
    const sP = P.HT9045Link.open(URL, 'P19'), gP = collector(sP);
    check(await waitOpen(sP), 'setup: Teach iframe P19');
    sP.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.acquire' })); await waitFor(() => gP.some((m) => m.id === 1));
    const press = (id, button, action) => JSON.stringify({ type: 'cmd', id, cmd: 'motor.access', tag: 'MInArmX',
      value: JSON.stringify({ source: 'uteach', button, action, kind: 'motion', motors: ['MInArmX'] }) });
    const rel = (id, button) => JSON.stringify({ type: 'cmd', id, cmd: 'motor.stop', tag: 'MInArmX',
      value: JSON.stringify({ source: 'uteach', button, action: 'stop', kind: 'control', motors: ['MInArmX'], reason: 'release' }) });
    const ackOf = (id) => gP.find((m) => m.type === 'ack' && m.id === id);
    let ma = cmds('motor.access').length, st = cmds('motor.stop').length;
    sP.send(press(2, 'btnJogP', 'jogP')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'closed');
    sP.send(rel(3, 'btnJogP'));                                                            // the page's own HT_WIN release
    await waitFor(() => !!ackOf(3)); await sleep(40);
    check(cmds('motor.stop').length === st + 1 && ackOf(3).ok && ackOf(3).linkDup === true,
          '關窗：hub 放開 btnJogP 一次，頁面自己的放開不再送（回 ok、linkDup）→ 伺服器只收到 1 個停止');
    server.slow = 60;                                                                       // the page's release arrives before the hub's ack
    ma = cmds('motor.access').length; st = cmds('motor.stop').length;
    sP.send(press(4, 'btnJogN', 'jogN')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'minimized');
    sP.send(rel(5, 'btnJogN'));
    await waitFor(() => !!ackOf(5)); await sleep(100);
    check(cmds('motor.stop').length === st + 1 && ackOf(5).ok && ackOf(5).linkDup === true,
          '頁面的放開比 hub 的 ack 先到：等 hub 那次的結果再回 ok，伺服器仍只收到 1 個停止');
    server.refuseStop = 'btnJogP';
    ma = cmds('motor.access').length; st = cmds('motor.stop').length;
    sP.send(press(6, 'btnJogP', 'jogP')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'closed');
    sP.send(rel(7, 'btnJogP'));
    await waitFor(() => !!ackOf(7), 3000); await sleep(100);
    check(cmds('motor.stop').length === st + 2 && ackOf(7) && ackOf(7).ok === false && !ackOf(7).linkDup,
          'hub 的放開被伺服器拒絕 → 頁面自己的放開照送（重試），頁面拿到伺服器的真結果');
    server.refuseStop = null; server.slow = 0;
    ma = cmds('motor.access').length; st = cmds('motor.stop').length;
    sP.send(press(8, 'btnJogP', 'jogP')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'closed');
    await sleep(CFG.dupReleaseMs + 60);
    sP.send(rel(9, 'btnJogP')); await waitFor(() => !!ackOf(9));
    check(cmds('motor.stop').length === st + 2 && !ackOf(9).linkDup, '超過重複判定時間（' + CFG.dupReleaseMs + ' ms）才來的放開 → 照送');
    ma = cmds('motor.access').length; st = cmds('motor.stop').length;
    sP.send(press(10, 'btnJogP', 'jogP')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'closed');
    sP.send(rel(11, 'btnStop')); await waitFor(() => !!ackOf(11));
    check(cmds('motor.stop').length === st + 2 && !ackOf(11).linkDup, '頁面的 STOP（btnStop）絕不攔：照送');
    ma = cmds('motor.access').length; st = cmds('motor.stop').length;
    sP.send(press(12, 'btnJogP', 'jogP')); await waitFor(() => cmds('motor.access').length > ma);
    frame.HT9045Link.windowHidden(P._src, 'closed');
    sP.send(press(13, 'btnJogP', 'jogP')); sP.send(rel(14, 'btnJogP'));                  // pressed again, then released
    await waitFor(() => !!ackOf(14));
    check(cmds('motor.stop').length === st + 2 && !ackOf(14).linkDup, '藏起來之後又按了同一顆 → 那一次的放開是新的，照送');
    sP.close();
  }

  // AI(W906-WSLINK) 20260930 ST01-E3: [20] E-017 m13 (St02-E2 review B) -- relayOk was one global, sticky flag
  console.log('[20] relay 探測依網址記：hello 逾時只讓這一條直連（警告、下次再問）；別的網址回 nohub 不影響 bridge 那條（m13）');
  {
    const slowFrame = makeWindow('m13-frame', null);                                        // no link yet: the hello goes unanswered
    const X = makeWindow('m13-child', slowFrame); load(X, linkCode, 'ht9045_link.js');
    const warns = []; X.console = { warn: (t) => warns.push(String(t)), log() {}, error() {}, info() {} };
    const cBefore = server.conns.length;
    const sX1 = X.HT9045Link.open(URL, 'x1');
    check(await waitOpen(sX1, 1500) && sX1.via === 'direct' && server.conns.length === cBefore + 1, 'hello 逾時 → 這一條直連');
    check(warns.length === 1 && /no hub answered/.test(warns[0]) && X.HT9045Link.status().relayMisses === 1,
          '逾時會在 console 警告（不再靜默），status().relayMisses=1');
    load(slowFrame, linkCode, 'ht9045_link.js'); slowFrame.HT9045Link.startHub(URL);         // the frame's hub is up now
    const sX2 = X.HT9045Link.open(URL, 'x2');
    check(await waitOpen(sX2, 1500) && sX2.via === 'relay', '下一次 open() 再問一次 → 這次走 hub（原本：一次逾時之後永遠直連）');
    const Y = makeWindow('m13-y', frame); load(Y, linkCode, 'ht9045_link.js');
    const sY1 = Y.HT9045Link.open('ws://127.0.0.1:9045/ht9045-json/', 'sim');
    check(await waitOpen(sY1, 1500) && sY1.via === 'direct', 'setup: 模擬器網址 → 外框回 nohub → 直連');
    const sY2 = Y.HT9045Link.open(URL, 'bridge');
    check(await waitOpen(sY2, 1500) && sY2.via === 'relay' && Y.HT9045Link.status().relay[URL] === true &&
          Y.HT9045Link.status().relay['ws://127.0.0.1:9045/ht9045-json'] === 'nohub',
          '同一個 iframe 之後開 bridge 網址 → 照樣走 hub（nohub 只記在那個網址）');
    [sX1, sX2, sY1, sY2].forEach((x) => x.close());
    await sleep(20);
  }

  // AI(W906-STREAM-F) 20260930: RULINGS_20260930 #12 (only open pages update data) -- the hub gives tag frames (snapshot /
  //   patch) only to pages whose window the frame reports open (background.html postWinState: open or minimized); a closed
  //   window's page gets ONE small snapshot (BOOT_TAGS: auth.level, machine.gpibModel, site.arm{1,2}.s{n}) when its socket
  //   opens; a page that opens gets a synthetic snapshot of the cache at once; alarm / modal / query / link.* / acks unchanged.
  {
    console.log('[F] closed windows: boot tags only at hello, no patches; an alarm still reaches them; opening resyncs from the cache');
    pushPatch({ 'auth.level': 2, 'machine.gpibModel': 'HT9050', 'site.arm1.s1': 1, 'site.arm2.s16': 0, 'site.arm1.count': 7 });
    await sleep(20);
    const BOOT = 'auth.level,machine.gpibModel,site.arm1.s1,site.arm2.s16';
    const keysOf = (m) => Object.keys((m && m.data) || {}).sort().join(',');
    const FA = makeWindow('F-open', frame); load(FA, linkCode, 'ht9045_link.js');
    const FB = makeWindow('F-closed', frame); load(FB, linkCode, 'ht9045_link.js');
    const FC = makeWindow('F-early', frame); load(FC, linkCode, 'ht9045_link.js');
    const made0 = server.conns.length;                             // connections ever created (openConns() moves with earlier sections' idle closes)
    frame.HT9045Link.windowState(FC._src, false);                  // reported closed before its page even said hello
    const sFA = FA.HT9045Link.open(URL, 'fa'), gFA = collector(sFA);
    const sFB = FB.HT9045Link.open(URL, 'fb'), gFB = collector(sFB);
    const sFC = FC.HT9045Link.open(URL, 'fc'), gFC = collector(sFC);
    check(await waitOpen(sFA) && await waitOpen(sFB) && await waitOpen(sFC), 'F: three pages open through the hub');
    await waitFor(() => [gFA, gFB, gFC].every((g) => g.some((m) => m.type === 'snapshot')));
    check([gFA, gFB].every((g) => g.some((m) => m.type === 'snapshot' && m.data['clock.text'] !== undefined && m.data['site.arm1.count'] === 7)),
          'F: windows the frame never reported get the full late-join snapshot, as today');
    const bootFC = gFC.find((m) => m.type === 'snapshot');
    check(!!bootFC && bootFC.synthetic === true && bootFC.seq === server.seq && keysOf(bootFC) === BOOT,
          'F: a window reported closed before hello gets a boot-tags-only snapshot (' + keysOf(bootFC) + ')');
    frame.HT9045Link.windowState(FB._src, false);
    gFA.length = 0; gFB.length = 0; gFC.length = 0;
    const skipped0 = hub.stats.tagSkipped;
    pushPatch({ 'clock.text': '21:40:00', 'pci1203.x': 1, 'auth.level': 3 });
    await waitFor(() => gFA.some((m) => m.type === 'patch'));
    await sleep(30);
    check(gFA.some((m) => m.type === 'patch' && m.data['clock.text'] === '21:40:00'), 'F: the open page gets the patch');
    check(!gFB.some((m) => m.type === 'patch') && !gFC.some((m) => m.type === 'patch'), 'F: the two closed pages get no patch (not even auth.level)');
    check(hub.stats.tagSkipped - skipped0 === 2, 'F: stats.tagSkipped counts the two skipped deliveries');
    check(frame.HT9045Link.hub(URL).status().tagsOff === 2, 'F: status().tagsOff = 2');
    server.conns.forEach((c) => c._deliver({ type: 'alarm', data: { code: 'F1' } }));
    await waitFor(() => gFB.some((m) => m.type === 'alarm') && gFC.some((m) => m.type === 'alarm'));
    check([gFA, gFB, gFC].every((g) => g.some((m) => m.type === 'alarm')), 'F: an alarm still reaches every page, closed or not');
    frame.HT9045Link.windowState(FB._src, true);                   // opened (the frame reports open=true for open and minimized)
    await waitFor(() => gFB.some((m) => m.type === 'snapshot'));
    const snapFB = gFB.find((m) => m.type === 'snapshot');
    check(!!snapFB && snapFB.synthetic === true && snapFB.data['clock.text'] === '21:40:00' && snapFB.data['pci1203.x'] === 1 &&
          snapFB.data['auth.level'] === 3 && snapFB.seq === server.seq,
          'F: opening -> one synthetic snapshot carrying what it missed (seq=' + server.seq + ')');
    const nSnap = gFB.filter((m) => m.type === 'snapshot').length;
    frame.HT9045Link.windowState(FB._src, true); await sleep(20);
    check(gFB.filter((m) => m.type === 'snapshot').length === nSnap, 'F: open -> open again sends no second snapshot');
    gFB.length = 0; pushPatch({ 'clock.text': '21:40:01' });
    await waitFor(() => gFB.some((m) => m.type === 'patch'));
    check(gFB.some((m) => m.type === 'patch' && m.data['clock.text'] === '21:40:01' && m.seq === snapFB.seq + 1),
          'F: once open the page gets patches again, seq right after the snapshot');
    check(!gFC.some((m) => m.type === 'patch'), 'F: the early-closed page still gets none');
    frame.HT9045Link.windowState(FC._src, true);
    await waitFor(() => gFC.some((m) => m.type === 'snapshot'));
    check(gFC.some((m) => m.type === 'snapshot' && m.data['clock.text'] === '21:40:01' && m.data['pci1203.x'] === 1),
          'F: the early-closed page resyncs (whole cache) when it opens');
    check(frame.HT9045Link.hub(URL).status().tagsOff === 0, 'F: status().tagsOff back to 0');
    frame.HT9045Link.windowState(null, false);                    // no window -> ignored, nothing throws
    check(server.conns.length === made0, 'F: the three pages created no server connection (' + made0 + ' ever created, as before)');
    [sFA, sFB, sFC].forEach((x) => x.close()); await sleep(20);
  }
  // AI(W906-STREAM-F) 20260930 boot race: the pages say hello before the hub's WebSocket has sent its first snapshot. A page
  //   whose window is already reported closed then gets that first server snapshot filtered to BOOT_TAGS, and nothing more.
  {
    console.log('[F2] boot race: closed window, hub not connected yet -> the first server snapshot, boot tags only');
    const URL2 = 'ws://127.0.0.1:8056/ht9045';
    frame.HT9045Link.startHub(URL2);                               // not connected until its first client
    const FD = makeWindow('F-boot', frame); load(FD, linkCode, 'ht9045_link.js');
    frame.HT9045Link.windowState(FD._src, false);
    const sFD = FD.HT9045Link.open(URL2, 'fd'), gFD = collector(sFD);
    check(await waitOpen(sFD), 'F2: the page opens through the second hub');
    await waitFor(() => gFD.some((m) => m.type === 'snapshot'));
    const bootFD = gFD.find((m) => m.type === 'snapshot');
    check(!!bootFD && bootFD.synthetic !== true && keysOf2(bootFD) === 'auth.level,machine.gpibModel,site.arm1.s1,site.arm2.s16',
          'F2: the server\'s first snapshot, boot tags only (' + keysOf2(bootFD) + ')');
    pushPatch({ 'clock.text': '21:40:02' }); await sleep(30);
    check(!gFD.some((m) => m.type === 'patch'), 'F2: no patch while closed');
    frame.HT9045Link.windowState(FD._src, true);
    await waitFor(() => gFD.filter((m) => m.type === 'snapshot').length === 2);
    const openFD = gFD.filter((m) => m.type === 'snapshot')[1];
    check(!!openFD && openFD.synthetic === true && openFD.data['clock.text'] === '21:40:02', 'F2: opening resyncs from the second hub\'s cache');
    sFD.close(); await sleep(20);
  }
  function keysOf2(m) { return Object.keys((m && m.data) || {}).sort().join(','); }

  // ==============================================================================================================
  // AI(W906-SCREEN-TOKEN) 20261001: RULINGS_20261001 (Jimmy 1001 12:3x "1->A"): the newest screen wins. The 1001 screenshot
  //   ("C 路讀取失敗 (editlist.get TestIF_File_YieldMonitoring): control-held") = a second live connection held the token
  //   (an old window still connected next to the reopened one). Screen mode = the shipped default (CFG_S has no
  //   screenTakeover key).
  // ==============================================================================================================
  {
    const URL3 = URL;                                    // the recipe client builds its URL from location (127.0.0.1:8055); each frame window has its own hub
    const S = { cfg: CFG_S, exposeParent: true };
    const onConn = (cid) => server.log.filter((x) => x.cid === cid).map((x) => x.cmd);
    server.owner = 0;

    console.log('[S1] 舊畫面連著、新畫面開起來 → 新畫面連上第一件事就 takeover；舊畫面變唯讀（自動 acquire 得 control-held）');
    const oldF = makeWindow('background-old', null, S); load(oldF, linkCode, 'ht9045_link.js');
    const oldHub = oldF.HT9045Link.startHub(URL3);
    const oldP = makeWindow('Setup.YieldMonitoring-old', oldF, S); load(oldP, linkCode, 'ht9045_link.js');
    const sOldF = oldF.HT9045Link.open(URL3, 'old-frame'); collector(sOldF);             // the frame's own socket (modal / dialog bridge): keeps the hub's connection up
    const sOld = oldP.HT9045Link.open(URL3, 'old'); const gOld = collector(sOld);
    check(await waitOpen(sOld) && sOld.via === 'relay', 'setup: 舊畫面的頁面經舊畫面的 hub 開好');
    await waitFor(() => oldHub.status().screenHeld);
    check(oldHub.status().screen === true && oldHub.status().screenHeld && server.owner === oldHub.sock.cid,
          '預設就是畫面模式：舊畫面連上就拿到權杖（screen=' + oldHub.status().screen + '）');
    check(onConn(oldHub.sock.cid)[0] === 'control.takeover', '舊畫面那條連線送出的第一個指令是 control.takeover（' + onConn(oldHub.sock.cid).join(',') + '）');
    const newF = makeWindow('background-new', null, S); load(newF, linkCode, 'ht9045_link.js');
    const newHub = newF.HT9045Link.startHub(URL3);
    const newP = makeWindow('Setup.YieldMonitoring-new', newF, S); load(newP, linkCode, 'ht9045_link.js');
    const sNewF = newF.HT9045Link.open(URL3, 'new-frame'); collector(sNewF);
    const sNew = newP.HT9045Link.open(URL3, 'new'); const gNew = collector(sNew);
    check(await waitOpen(sNew) && sNew.via === 'relay', '新畫面的頁面經新畫面的 hub 開好');
    await waitFor(() => server.owner === (newHub.sock && newHub.sock.cid) && newHub.status().screenHeld);
    check(server.owner === newHub.sock.cid && newHub.status().screenHeld && onConn(newHub.sock.cid)[0] === 'control.takeover',
          '新畫面連上第一個指令就是 takeover，伺服器 owner＝新畫面（' + onConn(newHub.sock.cid).join(',') + '）');
    const acqS1 = cmds('control.acquire').length;
    sNew.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.acquire' })); await waitFor(() => gNew.some((m) => m.id === 1));
    check(gNew.some((m) => m.id === 1 && m.ok && m.link === 'local') && cmds('control.acquire').length === acqS1,
          '新畫面頁面的 acquire（開窗讀資料那一個）由 hub 本地回 ok，不必到伺服器');
    sNew.send(JSON.stringify({ type: 'cmd', id: 2, cmd: 'editlist.get', tag: 'TestIF_File_YieldMonitoring' })); await waitFor(() => gNew.some((m) => m.id === 2));
    check(gNew.some((m) => m.id === 2 && m.ok), '新畫面的 editlist.get（截圖那一個讀取）ok');
    sOld.send(JSON.stringify({ type: 'cmd', id: 3, cmd: 'editlist.get', tag: 'TestIF_File_YieldMonitoring', value: 'old' }));   // value: not the same cmd+tag+value as id 2 (the 400 ms busy guard)
    await waitFor(() => gOld.some((m) => m.id === 3) && gOld.some((m) => m.type === 'link.token'));
    check(gOld.some((m) => m.id === 3 && m.error === 'not-operator') && gOld.some((m) => m.type === 'link.token' && m.owner === false),
          '舊畫面的下一個指令 not-operator → 舊畫面的頁面收到 link.token owner:false');
    sOld.send(JSON.stringify({ type: 'cmd', id: 4, cmd: 'control.acquire' })); await waitFor(() => gOld.some((m) => m.id === 4));
    check(gOld.some((m) => m.id === 4 && m.error === 'control-held') && server.owner === newHub.sock.cid,
          '舊畫面的自動 acquire → control-held，權杖留在新畫面（舊畫面＝只能看）');

    console.log('[S2] 畫面握著就一直握著：頁面 release、頁面關掉，都不會把伺服器的權杖還掉');
    const relS2 = cmds('control.release').length;
    sNew.send(JSON.stringify({ type: 'cmd', id: 5, cmd: 'control.release' })); await waitFor(() => gNew.some((m) => m.id === 5));
    check(gNew.some((m) => m.id === 5 && m.ok && m.link === 'local') && cmds('control.release').length === relS2 && server.owner === newHub.sock.cid,
          '頁面 release → 本地回 ok、沒送伺服器、owner 不變');
    sNew.send(JSON.stringify({ type: 'cmd', id: 6, cmd: 'control.acquire' })); await waitFor(() => gNew.some((m) => m.id === 6));
    check(newHub.status().clients.some((c) => c.name === 'new' && c.holder), 'setup: 頁面又 acquire（變成持有者）之後才關');
    sNew.close(); await sleep(60);
    check(cmds('control.release').length === relS2 && server.owner === newHub.sock.cid, '頁面關掉（bye）→ 一樣不還');

    console.log('[S3] 真的 recipe client 在新畫面：寫入 ok、閒置自動還（30 秒那條）只在本地，伺服器上仍是畫面持有');
    const RW = makeWindow('Setup.YieldMonitoring-rc', newF, S);
    load(RW, linkCode, 'ht9045_link.js'); load(RW, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
    if (RW.HT9045Recipe.tokenIdleMs) RW.HT9045Recipe.tokenIdleMs(120);
    const connS3 = server.conns.length;
    check(await outcome(RW.HT9045Recipe.write('Doc.Data', { S: { K: '1' } })) === 'ok', '新畫面存檔 ok');
    await sleep(350);
    check(server.owner === newHub.sock.cid && cmds('control.release').length === relS2, '閒置放掉之後伺服器上權杖仍是新畫面（release 沒送出去）');
    check(server.conns.length === connS3, 'recipe client 沒多開連線');

    console.log('[S4] 保活：每 screenKeepMs 送一次 acquire；伺服器閒置收回會被補回；別的連線 takeover → 畫面變唯讀、頁面收到通知；按一下拿回；對方走了自動拿回');
    sOld.close(); sOldF.close();                                                          // the old screen is closed now (its keep-alive would otherwise win an owner=0 race -- by design: acquire succeeds whenever nobody holds)
    check(await waitFor(() => oldHub.state === 'idle' && !oldHub.keepTimer, 1000), 'setup: 舊畫面關掉（hub 連線收掉、保活停止）');
    const keep0 = onConn(newHub.sock.cid).filter((c) => c === 'control.acquire').length;
    await sleep(CFG_S.screenKeepMs * 2 + 80);
    check(onConn(newHub.sock.cid).filter((c) => c === 'control.acquire').length - keep0 >= 2, '兩個週期內至少送了 2 個保活 acquire');
    server.owner = 0;                                                                     // 伺服器 10 分鐘閒置收回
    await waitFor(() => server.owner === newHub.sock.cid, CFG_S.screenKeepMs * 3);
    check(server.owner === newHub.sock.cid, '伺服器收回（owner=0）→ 下一次保活就補回（acquire 只在沒人拿時成功）');
    const intruder = new FakeWS(URL3); await waitFor(() => intruder.readyState === 1);
    intruder.send(JSON.stringify({ type: 'cmd', id: 1, cmd: 'control.takeover' })); await waitFor(() => server.owner === intruder.cid);
    const lost0 = newHub.stats.screenLost;
    await waitFor(() => newHub.stats.screenLost > lost0 && !newHub.token.held, CFG_S.screenKeepMs * 3);
    check(newHub.stats.screenLost === lost0 + 1 && newHub.token.held === false && server.owner === intruder.cid,
          '別的連線 takeover → 保活 acquire 得 control-held → hub 記成沒有、不搶回（owner 仍是對方）');
    await sleep(CFG_S.screenKeepMs * 2);
    check(server.owner === intruder.cid && newHub.stats.screenLost === lost0 + 1, '之後的保活也不會搶（control-held 不重複通知）');
    const o4 = await outcome(RW.HT9045Recipe.write('Doc.Data', { S: { K: '2' } }));       // write = operator press = takeover
    check(o4 === 'ok' && server.owner === newHub.sock.cid && newHub.status().screenHeld, '操作員在這個畫面按存檔（takeover）→ 拿回來，畫面又一直握著（' + o4 + '）');
    intruder.send(JSON.stringify({ type: 'cmd', id: 2, cmd: 'control.takeover' })); await waitFor(() => server.owner === intruder.cid);
    await waitFor(() => !newHub.token.held, CFG_S.screenKeepMs * 3);
    const reg0 = newHub.stats.screenRegained;
    intruder.close();                                                                     // 對方關掉（伺服器 CloseConn 清 owner）
    await waitFor(() => server.owner === newHub.sock.cid, CFG_S.screenKeepMs * 3);
    check(server.owner === newHub.sock.cid && newHub.stats.screenRegained === reg0 + 1 && newHub.status().screenHeld,
          '對方走了 → 下一次保活自動拿回（screenRegained+1）');

    console.log('[S5] 伺服器重啟（hub 的連線斷）→ 重連的第一個指令又是 takeover');
    const hubConn3 = newHub.sock, takes5 = newHub.stats.screenTakes; hubConn3.close();
    await waitFor(() => newHub.state === 'idle');
    const sNewF1 = newF.HT9045Link.open(URL3, 'new-frame-1'); collector(sNewF1);
    check(await waitOpen(sNewF1) && await waitFor(() => server.owner === newHub.sock.cid), '外框自己的 socket 重連（頁面沒送任何 takeover）→ owner 回到這個畫面');
    const conn5 = newHub.sock;
    check(newHub.stats.screenTakes === takes5 + 1 && onConn(newHub.sock.cid)[0] === 'control.takeover',
          '斷線前握著權杖 → 重連的第一個指令是 hub 自己的 takeover（screenTakes+1）');
    const RW2 = makeWindow('Setup.Speed-rc', newF, S);
    load(RW2, linkCode, 'ht9045_link.js'); load(RW2, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
    check(await outcome(RW2.HT9045Recipe.write('Doc.Data', { S: { K: '3' } })) === 'ok', '重連後存檔 ok');
    const sNewF2 = newF.HT9045Link.open(URL3, 'new-frame-2'); collector(sNewF2); await waitOpen(sNewF2);
    check(newHub.sock === conn5 && server.owner === conn5.cid && newHub.stats.screenTakes === takes5 + 1,
          '重連後存檔與外框走同一條連線、權杖仍在這個畫面、沒有第二次 screen takeover');

    console.log('[S6] 外框忙、hello 晚回 → 不開直連（一個畫面一條連線）；再問一次，先到的 welcome 綁定，晚到的那個立刻 bye（hub 不留幽靈）');
    const L6 = makeWindow('Setup.Late', newF, S); load(L6, linkCode, 'ht9045_link.js');
    const realPM = L6.parent.postMessage; let firstHello = true;
    L6.parent.postMessage = function (d, o, t) { if (firstHello) { firstHello = false; setTimeout(() => realPM.call(this, d, o, t), 450); } else realPM.call(this, d, o, t); };
    const conn6 = server.conns.length, miss6 = L6.HT9045Link.status().relayMisses;
    const s6 = L6.HT9045Link.open(URL3, 'late'); collector(s6);
    check(await waitOpen(s6, 2000) && s6.via === 'relay' && server.conns.length === conn6, 'hello 晚回 → 走 relay、伺服器連線數不變（原本 1.5 秒就直連＝多 1 條）');
    check(L6.HT9045Link.status().relayMisses === miss6 + 1, 'relayMisses 記到 1 次晚回');
    await sleep(600);
    check(newHub.status().clients.filter((c) => c.name === 'late').length === 1, '第一個 hello 晚到被收進來之後立刻 bye → hub 裡這一頁只有 1 個 client');
    const B6 = makeWindow('Setup.LateClosed', newF, S); load(B6, linkCode, 'ht9045_link.js');
    const realPM6 = B6.parent.postMessage; B6.parent.postMessage = function (d, o, t) { setTimeout(() => realPM6.call(this, d, o, t), 450); };
    const conn6b = server.conns.length;
    const s6b = B6.HT9045Link.open(URL3, 'given-up'); s6b.onerror = () => {}; s6b.close();   // 呼叫端放棄（recipe client 的 open 逾時）
    await sleep(900);
    check(server.conns.length === conn6b && newHub.status().clients.filter((c) => c.name === 'given-up').length === 0,
          '已經放棄的 socket：不開直連、晚到的 welcome 也立刻 bye（原本逾時照樣開一條沒人管的直連）');
    s6.close();

    console.log('[S7] recipe client 在有 hub 的外框裡：ht9045_link.js 載得慢（超過舊的 2 秒）也等它、走 hub；載入失敗會再試一次，兩次都失敗才直連');
    function withLoader2(w, plan) {           // plan: array of 'load' / 'error' per attempt, each after `delay` ms
      const att = [];
      w.document = { addEventListener() {}, removeEventListener() {}, hidden: false,
        currentScript: { src: 'http://127.0.0.1:8057/page/ht9045_recipe_client.js?v=8' },
        createElement: (t) => ({ tagName: t }),
        head: { appendChild: (el) => { const i = att.length; att.push(el); const p = plan[Math.min(i, plan.length - 1)];
          setTimeout(() => { if (p.what === 'load') { load(w, linkCode, 'ht9045_link.js'); el.onload(); } else el.onerror(); }, p.delay); } } };
      return att;
    }
    const W7 = makeWindow('Setup.SlowLink', newF, S);
    const att7 = withLoader2(W7, [{ what: 'load', delay: 2600 }]);
    load(W7, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
    const conn7 = server.conns.length, t7 = Date.now();
    const o7 = await outcome(W7.HT9045Recipe.write('Doc.Data', { S: { K: '7' } }));
    check(o7 === 'ok' && Date.now() - t7 >= 2400 && server.conns.length === conn7 && W7.HT9045Link && W7.HT9045Link.status().relayedSockets === 1,
          '載了 2.6 秒才好 → 還是走 hub、沒多開連線（原本 2 秒一到就直連）（' + o7 + '）');
    check(att7.length === 1, '只載一次');
    const W7b = makeWindow('Setup.NoLink', newF, S);
    const att7b = withLoader2(W7b, [{ what: 'error', delay: 20 }]);
    load(W7b, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
    const conn7b = server.conns.length;
    await outcome(W7b.HT9045Recipe.motorAccess({ motors: [], action: 'formShow', source: 'uMotorTest' }));
    check(att7b.length === 2 && server.conns.length === conn7b + 1 && !W7b.HT9045Link, '載入失敗 → 再試一次 → 還是失敗才直連（多 1 條，跟以前一樣不會整頁沒連線）');
    const W7c = makeWindow('Setup.NoHubParent', frame);                                   // 父視窗讀不到 HT9045Link（跨來源／舊外框）：跟以前一樣 2 秒就放棄
    const att7c = withLoader2(W7c, [{ what: 'load', delay: 2600 }]);
    load(W7c, recipeViaLink(recipeCode), 'ht9045_recipe_client.js');
    const conn7c = server.conns.length, t7c = Date.now();
    await outcome(W7c.HT9045Recipe.motorAccess({ motors: [], action: 'formShow', source: 'uMotorTest' }));
    check(server.conns.length === conn7c + 1 && Date.now() - t7c < 2500 && att7c.length === 1, '對照：讀不到父視窗的 hub → 照舊 2 秒後直連（行為不變）');

    console.log('[S9] 伺服器重啟、兩個畫面都斷：不管誰先重連，權杖都回到最新的畫面（舊分頁晚重連不會搶走，review R1）');
    const F9a = makeWindow('background-stale9', null, S); load(F9a, linkCode, 'ht9045_link.js'); const H9a = F9a.HT9045Link.startHub(URL3);
    const s9a = F9a.HT9045Link.open(URL3, 'stale9'); collector(s9a); await waitOpen(s9a);
    const F9b = makeWindow('background-newest9', null, S); load(F9b, linkCode, 'ht9045_link.js'); const H9b = F9b.HT9045Link.startHub(URL3);
    const s9b = F9b.HT9045Link.open(URL3, 'newest9'); collector(s9b); await waitOpen(s9b);
    check(await waitFor(() => H9b.sock && server.owner === H9b.sock.cid && H9b.status().screenHeld), 'setup: 後開的畫面握著權杖');
    const restart = async () => { [H9a.sock, H9b.sock].forEach((c) => c && c.close()); await waitFor(() => H9a.state === 'idle' && H9b.state === 'idle'); };
    check(await waitFor(() => !H9a.token.held, CFG_S.screenKeepMs * 3), 'setup: 舊畫面一個保活週期內就知道自己被搶（control-held → 記成沒有）');
    await restart();
    const r9b = F9b.HT9045Link.open(URL3, 'newest9-r1'); collector(r9b); await waitOpen(r9b); await waitFor(() => server.owner === H9b.sock.cid);
    const rej9 = H9a.stats.screenRejoins;
    const r9a = F9a.HT9045Link.open(URL3, 'stale9-r1'); collector(r9a); await waitOpen(r9a); await sleep(80);
    check(server.owner === H9b.sock.cid && H9a.stats.screenRejoins === rej9 + 1 && !H9a.token.held && onConn(H9a.sock.cid)[0] === 'control.acquire',
          '最新的先重連、舊的後重連：舊的只送 acquire（control-held）→ 權杖留在最新的畫面（' + onConn(H9a.sock.cid).join(',') + '）');
    await sleep(CFG_S.screenKeepMs * 2);
    check(server.owner === H9b.sock.cid, '之後舊畫面的保活也搶不走');
    await restart();
    const r9a2 = F9a.HT9045Link.open(URL3, 'stale9-r2'); collector(r9a2); await waitOpen(r9a2); await sleep(60);
    const r9b2 = F9b.HT9045Link.open(URL3, 'newest9-r2'); collector(r9b2); await waitOpen(r9b2);
    check(await waitFor(() => server.owner === H9b.sock.cid) && onConn(H9b.sock.cid)[0] === 'control.takeover',
          '舊的先重連（沒人拿 → 暫時拿到）、最新的後重連（斷線前握著 → takeover）→ 權杖回到最新的畫面');
    check(await waitFor(() => !H9a.token.held, CFG_S.screenKeepMs * 3), '舊畫面下一次保活得 control-held → 記成沒有權杖（只能看）');
    [s9a, s9b, r9b, r9a, r9a2, r9b2].forEach((x) => { try { x.close(); } catch (e) {} });

    console.log('[S10] 重啟期間重連先失敗幾次（wb_serve 還沒起來）、背景舊分頁的計時器被節流：權杖仍回到最新的畫面（review R3）');
    const tryOpen = (F, n) => { const x = F.HT9045Link.open(URL3, n); x.onerror = () => {}; return x; };
    const F10a = makeWindow('background-stale10', null, S); load(F10a, linkCode, 'ht9045_link.js'); const H10a = F10a.HT9045Link.startHub(URL3);
    const a10 = F10a.HT9045Link.open(URL3, 'stale10'); collector(a10); await waitOpen(a10); await waitFor(() => H10a.status().screenHeld);
    H10a._screenStop();                                                                   // hidden tab: Chromium throttles its timers, the keep-alive does not run
    const F10b = makeWindow('background-newest10', null, S); load(F10b, linkCode, 'ht9045_link.js'); const H10b = F10b.HT9045Link.startHub(URL3);
    const b10 = F10b.HT9045Link.open(URL3, 'newest10'); collector(b10); await waitOpen(b10);
    check(await waitFor(() => server.owner === H10b.sock.cid) && H10a.token.held === true, 'setup: 最新的畫面 takeover；被節流的舊分頁還以為自己握著');
    await sleep(CFG_S.screenKeepMs * 2);                                                 // the stale belief is now older than 1.5 keep periods
    const restart10 = async () => {
      server.down = true;
      [H10a.sock, H10b.sock].forEach((c) => c && c.close());
      await waitFor(() => H10a.state === 'idle' && H10b.state === 'idle');
      for (let i = 0; i < 2; i++) { tryOpen(F10a, 'try-a'); tryOpen(F10b, 'try-b'); await sleep(30); }   // reconnect attempts while wb_serve is down
      check(H10a.state === 'idle' && H10b.state === 'idle', 'setup: 兩邊各失敗兩次重連');
      server.down = false;
    };
    await restart10();
    const n10 = F10b.HT9045Link.open(URL3, 'newest10-r1'); collector(n10); await waitOpen(n10); await waitFor(() => server.owner === H10b.sock.cid);
    const s10 = F10a.HT9045Link.open(URL3, 'stale10-r1'); collector(s10); await waitOpen(s10); await sleep(80);
    check(server.owner === H10b.sock.cid && onConn(H10a.sock.cid)[0] === 'control.acquire',
          '最新的先、舊的後：舊分頁「握著」的證明過期 → 只送 acquire，搶不走（' + onConn(H10a.sock.cid).join(',') + '）');
    await restart10();
    const s10b = F10a.HT9045Link.open(URL3, 'stale10-r2'); collector(s10b); await waitOpen(s10b); await sleep(60);
    const n10b = F10b.HT9045Link.open(URL3, 'newest10-r2'); collector(n10b); await waitOpen(n10b);
    check(await waitFor(() => server.owner === H10b.sock.cid) && onConn(H10b.sock.cid)[0] === 'control.takeover',
          '舊的先、最新的後：最新的畫面重連失敗兩次之後仍記得要 takeover → 權杖回到最新的畫面');
    [a10, b10, n10, s10, s10b, n10b].forEach((x) => { try { x.close(); } catch (e) {} });

    console.log('[S11] 新畫面第一次的 takeover 沒有回覆連線就斷了 → 下次連上仍送 takeover（review R3）');
    const F11a = makeWindow('background-stale11', null, S); load(F11a, linkCode, 'ht9045_link.js'); const H11a = F11a.HT9045Link.startHub(URL3);
    const a11 = F11a.HT9045Link.open(URL3, 'stale11'); collector(a11); await waitOpen(a11); await waitFor(() => server.owner === H11a.sock.cid);
    const F11b = makeWindow('background-new11', null, S); load(F11b, linkCode, 'ht9045_link.js'); const H11b = F11b.HT9045Link.startHub(URL3);
    server.swallow = true;
    const b11 = F11b.HT9045Link.open(URL3, 'new11'); collector(b11); await waitOpen(b11); await sleep(40);
    check(H11b.status().takeNext === true && server.owner === H11a.sock.cid, 'setup: 新畫面的 takeover 沒有被處理（沒有回覆），權杖還在舊的');
    H11b.sock.close(); await waitFor(() => H11b.state === 'idle');
    server.swallow = false;
    const b11r = F11b.HT9045Link.open(URL3, 'new11-r'); collector(b11r); await waitOpen(b11r);
    check(await waitFor(() => server.owner === H11b.sock.cid) && onConn(H11b.sock.cid)[0] === 'control.takeover' && H11b.status().takeNext === false,
          '重連的第一個指令仍是 takeover → 權杖到新畫面；回覆 ok 之後才算用掉（takeNext=false）');
    [a11, b11, b11r].forEach((x) => { try { x.close(); } catch (e) {} });

    console.log('[S12] 外框有 HT9045Link 但沒有這個 URL 的 hub、而且很忙：socket 已經放棄後才回 nohub → 不開直連（review R3）');
    const F12 = makeWindow('background-otherhub12', null, S); load(F12, linkCode, 'ht9045_link.js'); F12.HT9045Link.startHub('ws://127.0.0.1:9045/ht9045-json/');
    const I12 = makeWindow('Setup.GivenUp12', F12, S); load(I12, linkCode, 'ht9045_link.js');
    const realPM12 = I12.parent.postMessage; I12.parent.postMessage = function (d, o, t) { setTimeout(() => realPM12.call(this, d, o, t), 450); };
    const conn12 = server.conns.length;
    const s12 = I12.HT9045Link.open(URL3, 'given-up12'); s12.onerror = () => {}; s12.close();
    await sleep(700);
    check(server.conns.length === conn12 && s12.via !== 'direct', '晚到的 nohub 不會替已經關掉的 socket 開直連（via=' + s12.via + '）');

    console.log('[S8] screenTakeover:false 的外框（[1]-[17] 用的那個）不送 takeover');
    check(hub.status().screen === false && hub.status().screenHeld === false, '舊規則的 hub：screen=false');
  }

  // AI(W906-WSLINK) 20260930 M1: over the whole run, every stop the HUB itself sent (reason link.*) is a jog release
  {
    const hubStops = cmds('motor.stop').map((x) => JSON.parse(x.value)).filter((v) => /^link[.]/.test(v.reason || '') || v.button === 'link.pageGone');
    check(hubStops.length > 0 && hubStops.every((v) => /jog/i.test(v.button) && v.button !== 'link.pageGone'),
          '整段測試裡 hub 自己送的 ' + hubStops.length + ' 個停止，全部是 jog 放開（沒有一個是 STOP／link.pageGone）');
  }

  console.log('\n' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('FAIL exception: ' + (e && e.stack || e)); process.exit(1); });
