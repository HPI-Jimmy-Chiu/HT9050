/* HT9045 馬達動作指令通道（HTML → C++ 請求 / C++ → HTML 完成回報）
   - 互斥：一次只允許一筆 pending 指令
   - kind=motion 執行中鎖定所有按鈕，只留 allowedWhileBusy（btnStop）
   - 收到 ack（done/error/aborted）或離線自動回報後解鎖
   協定檔：JSON/motor-access.json（catalog+request）、JSON/motor-access-ack.json（ack，只剩離線模式在用）
   AI(W906-W4-MOTOR) 20260925：線上模式改走 WS（HT9045Recipe.motorAccess／motorStop → C++ WebMotorAccess），
   頁面要先載 ht9045_recipe_client.js（HW.MotorTest.html／HW.teach.html 檔尾已載）。 */
(function (global) {
  if (!global.HTJsonWriter) { var _s = document.createElement('script'); _s.src = 'json-writer.js'; document.head.appendChild(_s); }
  var CAT = null;                 // motor-access.json
  var cfg = null;                 // init 設定
  var seq = 0;
  var pending = null;             var tokenHoldSet = false;   // 目前指令   // AI(W906-TOKEN-IDLE) 20260926: tokenHoldSet＝只登記一次 hold（init 可能被叫不只一次）
  var pollTimer = 0;
  var lockedEls = [];
  var later = [];                 // AI(W906-MT-E2) 20260925：pending 中又要送的請求（sendLater），pending 結束後依序送出
  var held = {};                  // AI(W906-MT-E2) 20260925：button -> true＝這一次按下已送出 jog、還沒送過停止
  var lastStopAt = {};            // AI(W906-MT-E2) 20260925：button -> ms，同一次放開會從好幾條路進來，只送一次停止
  var heldMotors = {};            // AI(W906-MT-FIX1) 20260925：button -> 按下那一刻 jog 的馬達。放開要停「當時在 jog 的那一軸」，
                                  //   不是放開時畫面上選的那一軸（按住時第二根手指點了別的馬達、或 FormShow 把選取清掉）

  function nowIso() { return new Date().toISOString(); }

  /* ---- JSON 載入（file:// 下 Edge 封鎖 XHR → 先用 <script> 墊片） ---- */
  function shimKey(url) {
    var u = String(url).split('?')[0].split('#')[0];
    return u.substring(u.lastIndexOf('/') + 1).replace(/\.json$/i, '');
  }
  function viaScript(url) {
    return new Promise(function (res, rej) {
      var key = shimKey(url), store = global.__HT9045_DATA__ || {};
      if (store[key]) return res(store[key]);
      var s = document.createElement('script');
      s.src = '../JSON/js/' + key + '.js';
      s.onload = function () {
        var d = (global.__HT9045_DATA__ || {})[key];
        d ? res(d) : rej(new Error('shim empty: ' + key));
      };
      s.onerror = function () { rej(new Error('shim fail: ' + key)); };
      document.head.appendChild(s);
    });
  }
  function viaNet(url) {
    return new Promise(function (res, rej) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        var t = x.responseText || '';
        if ((x.status === 200 || x.status === 0) && t.length > 1) {
          try { res(JSON.parse(t)); } catch (e) { rej(e); }
        } else rej(new Error('http ' + x.status));
      };
      x.onerror = function () { rej(new Error('xhr error')); };
      x.send();
    });
  }
  function loadJson(url, freshFirst) {
    // ack 需要最新內容 → 先走網路；catalog 可用墊片
    if (freshFirst) return viaNet(url).catch(function () { return viaScript(url); });
    if (location.protocol === 'file:') return viaScript(url).catch(function () { return viaNet(url); });
    return viaNet(url).catch(function () { return viaScript(url); });
  }

  function say(msg, cls) { if (cfg && cfg.status) cfg.status(msg, cls); }

  function catalogOf(button) {
    if (!CAT || !CAT.commands) return null;
    for (var i = 0; i < CAT.commands.length; i++) {
      var c = CAT.commands[i];
      if (c.source === cfg.source && c.button === button) return c;
    }
    return null;
  }
  function allowedWhileBusy(button) {
    var c = catalogOf(button);
    return !!(c && c.allowedWhileBusy);
  }

  /* ---- 鎖定/解鎖：motion 執行中只留 btnStop ---- */
  // AI(W906-MT-E2) 20260925：keepId＝正在按住的 jog 鈕，不鎖。golden LockAllButton（uMotorTest.cpp:2356-2364）把
  //   JogN／JogP 那兩行註解掉：「不能鎖定Jog, 會導致功能失效」。網頁上一樣 —— 鎖了 disabled 的按鈕收不到放開
  //   （mouseup 不送給 disabled 的表單元件），setPointerCapture 也可能被解除而變成「一按就停」。
  function lockAll(keepId) {
    unlockAll();
    var els = document.querySelectorAll('button,input,select');
    for (var i = 0; i < els.length; i++) {
      var el = els[i];
      var id = el.id || '';
      if (id && keepId && id === keepId) continue;
      if (id && allowedWhileBusy(id)) continue;
      if (cfg.alwaysEnabled && cfg.alwaysEnabled.indexOf(id) >= 0) continue;
      lockedEls.push([el, el.disabled]);
      el.disabled = true;
      el.style.opacity = '0.55';
    }
  }
  function unlockAll() {
    for (var i = 0; i < lockedEls.length; i++) {
      lockedEls[i][0].disabled = lockedEls[i][1];
      lockedEls[i][0].style.opacity = '';
    }
    lockedEls = [];
  }

  function publish(req) {
    global.__MOTOR_ACCESS_REQUEST__ = req;
    try { localStorage.setItem('ht9045-motor-access-request', JSON.stringify(req)); } catch (e) { }
    // C++ 離線模式：真的寫回 JSON（json-writer.js，需 background 已授權資料夾）供除錯比對
    if (cfg && cfg.offline && global.HTJsonWriter && HTJsonWriter.ready()) {
      var out = JSON.parse(JSON.stringify(CAT || {})); out.request = req;
      HTJsonWriter.write('motor-access', out).catch(function () { });
      if (req.state === 'done' || req.state === 'error' || req.state === 'aborted') {
        HTJsonWriter.write('motor-access-ack', {
          schemaVersion: '1.0.0', generatedAt: nowIso(), source: { toolchain: 'HTML-offline' },
          ack: { seq: req.seq, id: req.id, state: req.state, result: '', message: 'offline auto-ack',
                 motorId: (req.motors || [])[0] || null, position: null, completedAt: nowIso() }
        }).catch(function () { });
      }
    }
  }

  function finish(state, msg) {
    if (!pending) return;
    pending.state = state;
    publish(pending);
    if (pollTimer) { clearTimeout(pollTimer); pollTimer = 0; }
    unlockAll();
    var was = pending;
    pending = null;
    say('[' + state + '] ' + was.button + ' ' + was.action + (msg ? ' - ' + msg : ''),
      state === 'done' ? 'ok' : (state === 'error' ? 'err' : 'warn'));
    if (cfg.onFinish) cfg.onFinish(was, state);
    if (later.length) setTimeout(flushLater, 0);    // AI(W906-MT-E2) 20260925
  }

  /* ---- AI(W906-MT-E2) 20260925：排隊送（互斥還在，只是不丟掉） ----
     golden 一次點擊常連做好幾件事（lM00Click：抬起 LoopMove／HOME → 選軸 → SetSpeed(1)），網頁上每件是一筆 request，
     第二筆會撞到互斥被「Busy」丟掉。sendLater：沒有 pending 就照常 send；有就排隊，pending 結束（done／error／aborted）
     後依序送出。key 相同的只留最新一筆（拖捲軸時的 setSpeed 只要最後的值）。
     ⚠ 只給「不會自己開始動」的請求用（選軸、設速度、參數格、Copy From、HOME／LoopMove 的 start:false）——
       排隊的運動命令會在操作員已經不預期的時候才出發。 */
  function sendLater(button, extraParams, motorsOverride, key) {
    if (!pending) return send(button, extraParams, motorsOverride);
    var motors = motorsOverride || (cfg.getMotors ? cfg.getMotors() : []);
    for (var i = 0; i < later.length; i++) {
      if (key && later[i].key === key) { later.splice(i, 1); break; }
    }
    later.push({ button: button, params: extraParams, motors: motors, key: key || null });
    return 'queued';
  }
  function flushLater() {
    while (later.length && !pending) {
      var q = later.shift();
      send(q.button, q.params, q.motors);
    }
  }

  /* ---- ack 輪詢；離線模式自動回報 ---- */
  function watchAck() {
    var p = (CAT && CAT.protocol) || {};
    if (cfg.offline) {
      pollTimer = setTimeout(function () { finish('done', 'offline auto-ack'); },
        p.offlineAutoAckMs || 700);
      return;
    }
    var t0 = Date.now();
    (function poll() {
      pollTimer = setTimeout(function () {
        loadJson('../JSON/motor-access-ack.json', true).then(function (d) {
          var a = (d && d.ack) || {};
          if (pending && a.seq === pending.seq &&
            (a.state === 'done' || a.state === 'error' || a.state === 'aborted')) {
            finish(a.state, a.message || '');
            return;
          }
          if (Date.now() - t0 > (cfg.timeoutMs || 30000)) { finish('error', 'ack timeout'); return; }
          poll();
        }).catch(function () {
          if (Date.now() - t0 > (cfg.timeoutMs || 30000)) { finish('error', 'ack unreachable'); return; }
          poll();
        });
      }, p.ackPollMs || 200);
    })();
  }

  /* ---- AI(W906-W4-MOTOR) 20260925：線上模式把 request 送進 C++（WS motor.access／motor.stop，WebMotorAccess.h） ----
     原本 publish() 只寫 localStorage、ack 去輪詢一個沒有人寫的靜態檔，非 motion 命令還立刻顯示假成功 'no motion'。
     現在：C++ 回 ok → 依 ack.state 結束；C++ 拒絕（含「尚未接上」）→ 顯示 error 與理由。
     離線模式（?offline=1）維持原本的自動回報，那是畫面開發用的模擬，訊息會寫 offline。 */
  function cppChannel() { return global.HT9045Recipe && HT9045Recipe.motorAccess ? HT9045Recipe : null; }
  function toCpp(req) {
    var ch = cppChannel();
    if (!ch) return Promise.reject(new Error('C++ channel missing (ht9045_recipe_client.js not loaded)'));
    return req.action === 'stop' ? ch.motorStop(req) : ch.motorAccess(req);
  }
  function stopReq(button, motors, why) {
    return { seq: ++seq, id: 'cmd-' + seq, source: cfg.source, button: button, action: 'stop',
      kind: 'control', motors: motors || [], params: {}, issuedAt: nowIso(), state: 'requested', reason: why || '' };
  }
  function sendStop(req) {                         // 停止：不進互斥（pending），結果只顯示在狀態列
    publish(req);
    if (cfg.offline) { say('[offline] stop ' + req.button + ' (no C++)', 'warn'); return; }
    toCpp(req).then(function (ack) {
      var bad = !!(ack && ack.partial);            // AI(W906-W4-MOTOR) 20260925 NB2 R19 RW4-2：有軸拒絕停止就不能顯示綠色
      say((bad ? '[stop PARTIAL] ' : '[stop] ') + req.button + ' - ' + (ack.message || ack.result || 'ok'), bad ? 'err' : 'ok');
    }, function (e) {
      say('[stop FAILED] ' + req.button + ' - ' + e.message, 'err');
    });
  }

  /* ---- 送出指令（互斥） ---- */
  function send(button, extraParams, motorsOverride) {
    var c = catalogOf(button);
    if (!c) { say('No command catalog for ' + button, 'err'); return null; }

    if (pending && !c.allowedWhileBusy) {
      say('Busy: ' + pending.button + ' (' + pending.action + ') running', 'warn');
      return null;
    }

    var motors = motorsOverride || (cfg.getMotors ? cfg.getMotors() : []);

    if (c.action === 'stop') {                     // STOP：先結束畫面上的 pending，再真的送停止給 C++
      if (pending) { motors = motors.length ? motors : pending.motors; finish('aborted', 'stopped by ' + button); }
      sendStop(stopReq(button, motors, 'button'));
      return null;
    }

    var params = {};
    if (cfg.getParams) params = cfg.getParams() || {};
    if (extraParams) for (var k in extraParams) params[k] = extraParams[k];

    var req = {
      seq: ++seq, id: 'cmd-' + seq, source: cfg.source, button: button,
      action: c.action, kind: c.kind, motors: motors, params: params,
      issuedAt: nowIso(), state: 'requested'
    };
    pending = req;
    publish(req);
    say('→ ' + c.action + ' ' + (motors.join(',') || '-') +
      ' ' + JSON.stringify(params), 'warn');

    if (c.kind === 'motion') lockAll(c.release === 'stop' ? button : null);

    // jog 類：放開可能不落在按鈕上（手指滑出去、視窗失焦、頁面被藏起來）→ 也在 document／window 監聽
    // AI(W906-MT-E2) 20260925：觸控螢幕要聽 pointer 事件 —— Edge 對手指常常等放開才一起補送 mousedown／mouseup，
    //   只聽 mouseup 時放開前根本沒開始、或放開後才開始。pointercancel（系統把手勢搶走）、blur、
    //   visibilitychange(hidden) 都當放開；每一條都走 release → 送停止。C++ 的斷線停止照舊是最後一道。
    if (c.release === 'stop') {
      held[button] = true;
      heldMotors[button] = motors.slice();          // AI(W906-MT-FIX1) 20260925
      var evs = [[document, 'mouseup'], [document, 'pointerup'], [document, 'pointercancel'],
                 [window, 'blur'], [document, 'visibilitychange']];
      var up = function (ev) {
        if (ev && ev.type === 'visibilitychange' && document.visibilityState !== 'hidden') return;
        // AI(W906-JOGBLUR) 20260930：blur 是用捕捉階段（第三參數 true）掛在 window 上的 —— 捕捉階段會收到「任何元件」的 blur，
        //   不只是視窗失焦。按下 JOG 的那一刻，原本有焦點的輸入框（例：速度欄）失焦 → 被當成放開 → 寸動一開始就停。
        //   只有視窗本身失焦才算放開（ev.target===window）；放開滑鼠／手指、pointercancel、頁面被藏起來照舊都送停止。
        if (ev && ev.type === 'blur' && ev.target !== window) return;
        for (var i = 0; i < evs.length; i++) evs[i][0].removeEventListener(evs[i][1], up, true);
        release(button);
      };
      for (var j = 0; j < evs.length; j++) evs[j][0].addEventListener(evs[j][1], up, true);
    }

    if (cfg.offline) {                             // 畫面開發用的離線模擬：維持原行為，但訊息講明是 offline
      if (c.kind === 'motion') watchAck();
      else finish('done', 'offline: no C++');
      return req;
    }
    toCpp(req).then(function (ack) {
      if (cfg.onAck) cfg.onAck(req, ack, null);    // AI(W906-W4D) 20260925：頁面依伺服端的回覆（loopActive／homeActive）設按鈕狀態（NB2 R23 W4C-5）
      // AI(W906-MT-FIX1) 20260925：保險絲（審查 high）。jog 被 C++ 接受了，但按鈕在 ack 回來之前就已經放開 ——
      //   那次放開送出的停止有可能比 jog 先到 C++（線路重連、主執行緒卡頓）。再補送一次停止給「這個 jog 的馬達」；
      //   軸若已經停了，多一次 StopDec 無害。寧可多送不可漏送（同 release() 的原則）。
      if (c.release === 'stop' && !held[button]) sendStop(stopReq(button, req.motors, 'late-ack'));
      if (pending !== req) return;                 // 已被 STOP／放開結束
      var st = ack && ack.state;
      finish(st === 'error' || st === 'aborted' ? st : 'done', (ack && (ack.message || ack.result)) || '');
    }, function (e) {
      if (cfg.onAck) cfg.onAck(req, null, e);
      if (pending !== req) return;
      finish('error', e.message);
    });
    return req;
  }

  // AI(W906-SIDECMD) 20261001：EastSun「每次我使用不是被鎖住 就是沒有功能」—— 幫手查證（Teach／Motor Test oplog）：每次 wb_serve 剛開的
  //   約 30 秒 C++ 不回應，這時頁面開窗自己送的背景命令（Teach 的手動教導查詢 teachSet {query}、Motor Test 的 formShow／formClose）
  //   佔著上面 send() 的互斥（pending），操作員按的每一顆都被 `Busy: … running` 丟掉，只在角落一行小字。
  //   背景命令改走這一條：不設 pending、不鎖畫面、不擋按鈕；回覆照樣交給 cfg.onAck。只准非運動的命令（motion 一律拒絕，照舊走 send）。
  function sendSide(button, extraParams, motorsOverride) {
    var c = catalogOf(button);
    if (!c) { say('No command catalog for ' + button, 'err'); return null; }
    if (c.kind === 'motion' || c.action === 'stop') { say('sendSide: ' + button + ' 是運動／停止命令，不走背景通道', 'err'); return null; }
    var params = {};
    if (cfg.getParams) params = cfg.getParams() || {};
    if (extraParams) for (var k in extraParams) params[k] = extraParams[k];
    var req = {
      seq: ++seq, id: 'cmd-' + seq, source: cfg.source, button: button,
      action: c.action, kind: c.kind, motors: motorsOverride || (cfg.getMotors ? cfg.getMotors() : []), params: params,
      issuedAt: nowIso(), state: 'requested'
    };
    publish(req);
    if (cfg.offline) return req;
    toCpp(req).then(function (ack) { if (cfg.onAck) cfg.onAck(req, ack, null); },
                    function (e) { if (cfg.onAck) cfg.onAck(req, null, e); });
    return req;
  }

  // jog 放開 → 停止。golden 的 MouseUp 呼叫 PCIL132_StopMotor（只停那一軸，uMotorTest.cpp:900）。
  // AI(W906-W4-MOTOR) 20260925：**不看 pending 一律送**—— C++ 的 ack 可能在放開之前就回來把 pending 清掉，
  //   那時軸還在跑；原本這裡 `if (!pending) return;` 然後只 finish('aborted')，什麼都沒送。
  // AI(W906-MT-E2) 20260925：同一次放開會從好幾條路進來（按鈕的 pointerup、document 的 pointerup、lostpointercapture），
  //   第一條送停止，500 ms 內沒有按住的其餘幾條不重送；有按住（held）就一定送 —— 寧可多送不可漏送。
  function release(button) {
    var now = Date.now();
    if (!held[button] && lastStopAt[button] && now - lastStopAt[button] < 500) return;
    held[button] = false;
    lastStopAt[button] = now;
    var was = pending;
    var motors = heldMotors[button] ||             // AI(W906-MT-FIX1) 20260925：按下時記住的馬達優先
      ((was && was.button === button) ? was.motors : (cfg.getMotors ? cfg.getMotors() : []));
    if (was && was.button === button) finish('aborted', 'released ' + button);
    var c = catalogOf(button);
    if (c && c.release === 'stop') sendStop(stopReq(button, motors, 'release'));
  }

  // AI(W906-MT-FIX1) 20260925：把每一個還按著的 jog 都當成放開（送停止）。background.html 關視窗是 display:none，
  //   iframe 的 visibilityState 跟著最上層頁面走、不會變 hidden，pointerup 也不保證送得到被藏起來的 frame ——
  //   所以頁面在 FormClose／HT_WIN 關閉／pagehide 時要自己呼叫這一支。
  function releaseHeld() {
    for (var b in held) if (held.hasOwnProperty(b) && held[b]) release(b);
  }

  function init(options) {
    cfg = options || {};   if (global.HT9045Recipe && HT9045Recipe.setTokenHold && !tokenHoldSet) { tokenHoldSet = true; HT9045Recipe.setTokenHold(function () { if (pending) return true; for (var hb in held) if (held.hasOwnProperty(hb) && held[hb]) return true; return !!(cfg.holdToken && cfg.holdToken()); }); }   // AI(W906-TOKEN-IDLE) 20260926: 動作還沒回報完成（pending）或頁面說還在跑（cfg.holdToken）時，不讓 recipe client 閒置還權杖  ／  AI(W906-TOKEN-IDLE) 20260926: NB2 R69 B1 —— 按住中的 jog（held）也不還：golden 一直 jog 到放開為止，原本 30 秒後權杖被還、deadman 會停下 jog
    return loadJson('../JSON/motor-access.json').then(function (d) {
      CAT = d;
      say('Motor access ready (' + (CAT.commands || []).length + ' commands)', 'ok');
      return CAT;
    }).catch(function (e) {
      say('motor-access.json load failed: ' + e.message, 'err');
      throw e;
    });
  }

  global.HTMotorAccess = {
    init: init, send: send, sendLater: sendLater, sendSide: sendSide, release: release, releaseHeld: releaseHeld, loadJson: loadJson,   // AI(W906-SIDECMD) 20261001: + sendSide
    isBusy: function () { return !!pending; },
    queued: function () { return later.length; },   // AI(W906-MT-E2) 20260925
    current: function () { return pending; },
    debug: function () {
      return {
        offline: !!(cfg && cfg.offline),
        source: cfg && cfg.source,
        hasCatalog: !!CAT,
        protocol: CAT && CAT.protocol,
        pollTimer: pollTimer,
        pending: pending && pending.button
      };
    },
    lockAll: lockAll, unlockAll: unlockAll,
    exportRequest: function () {
      var r = global.__MOTOR_ACCESS_REQUEST__;
      if (!r) return;
      var out = JSON.parse(JSON.stringify(CAT || {}));
      out.request = r;
      var a = document.createElement('a');
      a.href = 'data:application/json;charset=utf-8,' + encodeURIComponent(JSON.stringify(out, null, 1));
      a.download = 'motor-access.json';
      a.click();
    }
  };
})(window);
