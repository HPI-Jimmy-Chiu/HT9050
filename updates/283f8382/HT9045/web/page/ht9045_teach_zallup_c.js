/* ht9045_teach_zallup_c.js -- HW.teach.html：golden 有處理函式、網頁上按了一直沒反應的 6 顆鈕。
 * ---------------------------------------------------------------------------
 * AI(W906-TEACH-ZALLUP) 20261001: 10-01 Teach 按鈕稽核。手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js
 *   （同 ht9045_teach_trayz_c.js 的理由），免得產生器覆蓋。
 *
 *   Axle Control 頁  btnInZAllUp   "In Z All Up"   golden uteach.cpp:4466 btnInZAllUpClick
 *                    btnOutZAllUp  "Out Z All Up"  golden uteach.cpp:4480 btnOutZAllUpClick
 *   Index 頁         btnZ1Servo    "Server ON"     golden uteach.cpp:4253 btnZ1ServoClick（MTestZ1）
 *                    btnZ2Servo    "Server ON"     golden uteach.cpp:4270 btnZ2ServoClick（MTestZ2）
 *                    btnArm1YServo "Server ON"     golden uteach.cpp:2919 btnArm1YServoClick（MTestY1；4 軸 Index 另加 MTestY2）
 *                    btnArm2YServo "Server ON"     uteach.dfm 的 OnClick 也是 btnArm1YServoClick（同一個處理函式）
 *
 * 頁面只送「按了哪一顆」：HTMotorAccess.send(<按鈕名>)，指令表 JSON/motor-access.json 的 uteach 列
 *   btnIn/OutZAllUp -> teachZAllUp（kind motion：C++ 回覆前畫面照例鎖住）、四顆 Servo -> teachIndexServo（kind control）。
 *   哪幾軸、開還是關、互鎖全部由 C++ 照 golden 決定（移植樹 WebMotorAccess.cpp 檔尾 AI(W906-TEACH-ZALLUP)）：
 *     Z All Up = 那一臂每一個 Z 照 HOME 鈕的單軸歸零（1203 軸 DS402 歸零→ZSafePos），Enable=0 的照 golden 設 HomeFlag=1 不動；
 *     Servo    = golden 固定的那一軸（不是 Active Motor 選的那一軸），依實際伺服狀態取反（讀不到才照 golden 的 static 輪流）。
 *   運轉中（SystemStart）、手動教導對話框開著時，C++ 一律拒絕，理由顯示在右下角（HTMotorAccess 的狀態列）。
 *   不加確認視窗（EastSun：原版按下就動作的鈕，網頁也按下就送）。
 *
 * 按鈕上的字（golden Caption）：
 *   Z All Up：golden DoZHome（uteach.cpp:4526-4534）有 Z 還在歸零就是 "Homeing..."，做完是 "In Z All Up"／"Out Z All Up"。
 *     這裡：送出就顯示 "Homeing..."；C++ 拒絕（req.state==='error'）立刻改回；C++ 接受後看 C++ runtime 的 motion.homeJob
 *     （那一臂的 Z 軸：golden InArmZIndex／OutArmZIndex 的別名），連續兩次新讀到的 runtime 都沒有 Z 在歸零就改回。
 *     runtime 讀不到時不猜，最多 5 分鐘後改回並說明（C++ 的歸零工作照它自己的逾時處理）。
 *   Servo：golden 按一次換一次字（"Server OFF"＝剛關、"Server ON"＝剛開）。這裡照 C++ runtime 那一軸的 state.servoOn 顯示
 *     （MTestZ1／MTestZ2／MTestY1）；讀不到就維持原字（.dfm 的 "Server ON"）。
 *   Z 在歸零期間不還權杖（HT9045Recipe.setTokenHold，同 btnHome；權杖還掉＝操作員連線消失，C++ 會取消歸零工作）。
 *   關窗（HT_WIN open=false）＝golden FormClose → btnStop：C++ 那一半由 C++ 自己做（MotorAccessPageClosed），這裡只把字改回。
 * 指令表沒有這幾列（motor-access.json 是舊版）⇒ 照 TEACH-UNWIRED 灰掉並說明（teachMarkUnwiredEl），不讓操作員按了沒反應。
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';

  // AI(W906-TEACH-ZALLUP) 20261001: golden InArmZIndex／OutArmZIndex（cmydef.cpp:3333／:3335）的別名：Z A..H、Z Ae..Ah、Z Be..Bh
  var ZBTN = {
    btnInZAllUp:  { cap: 'In Z All Up',  re: /^MInArmZ([A-H]|[AB][e-h])$/ },
    btnOutZAllUp: { cap: 'Out Z All Up', re: /^MOutArmZ([A-H]|[AB][e-h])$/ }
  };
  // AI(W906-TEACH-ZALLUP) 20261001: Servo 鈕 → golden 的第一軸（字照它的 state.servoOn）與 golden 會改字的按鈕
  var SBTN = {
    btnZ1Servo:    { motor: 'MTestZ1', caps: ['btnZ1Servo'] },
    btnZ2Servo:    { motor: 'MTestZ2', caps: ['btnZ2Servo'] },
    btnArm1YServo: { motor: 'MTestY1', caps: ['btnArm1YServo', 'btnArm2YServo'] },
    btnArm2YServo: { motor: 'MTestY1', caps: ['btnArm1YServo', 'btnArm2YServo'] }
  };
  var ACTION = { btnInZAllUp: 'teachZAllUp', btnOutZAllUp: 'teachZAllUp',
                 btnZ1Servo: 'teachIndexServo', btnZ2Servo: 'teachIndexServo', btnArm1YServo: 'teachIndexServo', btnArm2YServo: 'teachIndexServo' };
  var WAIT_MS = 30000, POLL_MS = 300, TICK_MS = 500, MAX_HOMING_MS = 300000;

  function $(id) { return document.getElementById(id); }
  function info(msg, cls) {
    if (typeof global.teachSetInfo === 'function') { try { global.teachSetInfo(msg, cls); return; } catch (e) {} }
    if (global.console) console.warn('[zallup] ' + msg);
  }
  function setText(id, t) { var el = $(id); if (el && el.textContent !== t) el.textContent = t; }
  function unwired(id, why) {
    var el = $(id);
    if (!el) return;
    if (typeof global.teachMarkUnwiredEl === 'function') global.teachMarkUnwiredEl(el, why);
    else el.setAttribute('data-unwired', why);
  }
  function runtimeMotors() {
    var rt = global.teachMotorRuntime;
    return (rt && rt.motors && typeof rt.motors.length === 'number') ? rt : null;
  }
  function runtimeOf(rt, id) {
    for (var i = 0; i < rt.motors.length; i++) if (rt.motors[i] && rt.motors[i].motorId === id) return rt.motors[i];
    return null;
  }

  var ST = { bound: false, reason: '', missing: [], track: {} };   // track: 按鈕 → { req, t0, gone, lastRt }

  // ---- Z All Up 的字：golden DoZHome（:4526-4534）----
  function endTrack(b, why) {
    ST.track[b] = null;
    setText(b, ZBTN[b].cap);
    if (why) info(why, 'warn');
  }
  function zBusy(rt, b) {
    for (var i = 0; i < rt.motors.length; i++) {
      var m = rt.motors[i];
      if (m && ZBTN[b].re.test(String(m.motorId)) && m.motion && m.motion.homeJob === true) return true;
    }
    return false;
  }
  function tickZ() {
    Object.keys(ZBTN).forEach(function (b) {
      var s = ST.track[b];
      if (!s) return;
      var st = s.req && s.req.state;
      if (st === 'error' || st === 'aborted') { endTrack(b, ''); return; }        // C++ 拒絕／被 STOP 結束：理由已在狀態列
      if (Date.now() - s.t0 > MAX_HOMING_MS) {
        endTrack(b, ZBTN[b].cap + '：5 分鐘還看不到 Z 歸零結束（runtime 讀不到？）—— 按鈕字改回；C++ 的歸零工作照它自己的逾時處理');
        return;
      }
      if (st !== 'done') return;                                                  // C++ 還沒回覆
      var rt = runtimeMotors();
      if (!rt || rt === s.lastRt) return;                                         // 沒有新的 runtime：不判
      s.lastRt = rt;
      if (zBusy(rt, b)) { s.gone = 0; return; }
      if (++s.gone >= 2) endTrack(b, '');                                         // 連續兩次沒有 Z 在歸零（一次可能是還沒排進去）
    });
  }
  // ---- Servo 的字：C++ runtime 那一軸的 state.servoOn ----
  function tickServo() {
    var rt = runtimeMotors();
    if (!rt) return;
    Object.keys(SBTN).forEach(function (b) {
      var m = runtimeOf(rt, SBTN[b].motor), on = m && m.state ? m.state.servoOn : null;
      if (typeof on !== 'boolean') return;                                        // 讀不到：維持原字
      SBTN[b].caps.forEach(function (c) { setText(c, on ? 'Server ON' : 'Server OFF'); });
    });
  }
  function tick() {
    if (!ST.bound) return;
    try { tickZ(); tickServo(); } catch (e) { if (global.console) console.warn('[zallup] tick: ' + (e && e.message)); }
  }
  function anyTracking() {
    for (var b in ST.track) if (ST.track.hasOwnProperty(b) && ST.track[b]) return true;
    return false;
  }

  function onClick(b) {
    var ma = global.HTMotorAccess;
    if (!ma) { info(b + '：HTMotorAccess 不存在（motor-access.js 沒載入）', 'err'); return; }
    var req = ma.send(b);                                                        // 參數照頁面的 getParams（speedEvent 等），軸由 C++ 照 golden 決定
    if (!req) return;                                                             // Busy／指令表沒有：HTMotorAccess 已經在狀態列說了
    if (ZBTN[b]) {
      var s = ST.track[b];
      // 已經在追一次被 C++ 接受的 Z All Up：這一下 C++ 會回「已在歸零中」（或被 400 ms 防連點擋下）—— 不換掉原本那一次，只重新數
      if (s && s.req && s.req.state === 'done') { s.gone = 0; return; }
      ST.track[b] = { req: req, t0: Date.now(), gone: 0, lastRt: runtimeMotors() };
      setText(b, 'Homeing...');                                                   // golden DoZHome :4527／:4532
    }
  }

  function bind(cat) {
    var have = {}, cmds = (cat && cat.commands) || [];
    for (var i = 0; i < cmds.length; i++) {
      var c = cmds[i];
      if (c && c.source === 'uteach' && ACTION[c.button] && c.action === ACTION[c.button]) have[c.button] = true;
    }
    ST.missing = [];
    Object.keys(ACTION).forEach(function (b) {
      var el = $(b);
      if (!el) { ST.missing.push(b); return; }
      if (!have[b]) {
        unwired(b, b + '：指令表 motor-access.json 還沒有教導頁的這一列（uteach ' + b + ' → ' + ACTION[b] + '），這顆還沒接上、按了不會動作');
        return;
      }
      if (el.getAttribute('data-zallup')) return;
      el.setAttribute('data-zallup', '1');
      el.setAttribute('data-acc', '1');                                           // 同 HW.teach.html teachOn 的標記：已接上
      el.addEventListener('click', function () { onClick(b); });
    });
    ST.bound = true;
    document.documentElement.setAttribute('data-teach-zallup', 'ok');
    if (ST.missing.length) info('Z All Up／Servo：頁面上找不到 ' + ST.missing.join(', ') + '（頁面改版？）', 'warn');
  }

  // 等頁面的 teachInitAccess 把 HTMotorAccess 初始化好（指令表載入、source=uteach），再讀一次同一份指令表確認有這 6 列
  function boot() {
    var t0 = Date.now();
    (function poll() {
      var ma = global.HTMotorAccess, d = ma && ma.debug ? ma.debug() : null;
      if (d && d.hasCatalog && d.source === 'uteach') {
        ma.loadJson('../JSON/motor-access.json').then(bind, function (e) {
          ST.reason = (e && e.message) || String(e);
          Object.keys(ACTION).forEach(function (b) { unwired(b, b + '：讀不到指令表 motor-access.json（' + ST.reason + '），這顆沒有接上'); });
        });
        return;
      }
      if (Date.now() - t0 > WAIT_MS) {
        ST.reason = '等了 ' + (WAIT_MS / 1000) + ' 秒，教導頁的馬達指令通道（HTMotorAccess）還沒初始化';
        document.documentElement.setAttribute('data-teach-zallup', 'fail');
        Object.keys(ACTION).forEach(function (b) { unwired(b, b + '：' + ST.reason + '，這顆沒有接上（重新整理頁面再試）'); });
        return;
      }
      setTimeout(poll, POLL_MS);
    })();
  }

  // Z 在歸零期間不還權杖（同 btnHome 的 holdToken；HT9045Recipe 允許登記多個 hold）
  (function holdToken() {
    var t0 = Date.now();
    (function poll() {
      if (global.HT9045Recipe && typeof global.HT9045Recipe.setTokenHold === 'function') { global.HT9045Recipe.setTokenHold(anyTracking); return; }
      if (Date.now() - t0 < WAIT_MS) setTimeout(poll, POLL_MS);
    })();
  })();

  // 關窗：golden FormClose → btnStop（C++ 那一半在 C++，MotorAccessPageClosed）；這裡只把 Z All Up 的字改回
  global.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN' || global.parent === global || ev.source !== global.parent) return;
    if ((m.state ? m.state !== 'open' && m.state !== 'minimized' : !m.open)) Object.keys(ZBTN).forEach(function (b) { if (ST.track[b]) endTrack(b, ''); });
  });

  setInterval(tick, TICK_MS);
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true });
  else boot();

  global.HT9045TeachZAllUp = { ZBTN: ZBTN, SBTN: SBTN, ACTION: ACTION, state: ST, tick: tick };
})(window);
