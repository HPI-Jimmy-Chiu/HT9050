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
  var pending = null;             // 目前指令
  var pollTimer = 0;
  var lockedEls = [];

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
  function lockAll() {
    unlockAll();
    var els = document.querySelectorAll('button,input,select');
    for (var i = 0; i < els.length; i++) {
      var el = els[i];
      var id = el.id || '';
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

    if (c.kind === 'motion') lockAll();

    // jog 類：按鈕會被 lockAll 停用而收不到 mouseup → 改在 document 監聽放開
    if (c.release === 'stop') {
      var up = function () {
        document.removeEventListener('mouseup', up);
        window.removeEventListener('blur', up);
        release(button);
      };
      document.addEventListener('mouseup', up);
      window.addEventListener('blur', up);
    }

    if (cfg.offline) {                             // 畫面開發用的離線模擬：維持原行為，但訊息講明是 offline
      if (c.kind === 'motion') watchAck();
      else finish('done', 'offline: no C++');
      return req;
    }
    toCpp(req).then(function (ack) {
      if (cfg.onAck) cfg.onAck(req, ack, null);    // AI(W906-W4D) 20260925：頁面依伺服端的回覆（loopActive／homeActive）設按鈕狀態（NB2 R23 W4C-5）
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

  // jog 放開 → 停止。golden 的 MouseUp 呼叫 PCIL132_StopMotor（只停那一軸，uMotorTest.cpp:900）。
  // AI(W906-W4-MOTOR) 20260925：**不看 pending 一律送**—— C++ 的 ack 可能在放開之前就回來把 pending 清掉，
  //   那時軸還在跑；原本這裡 `if (!pending) return;` 然後只 finish('aborted')，什麼都沒送。
  function release(button) {
    var was = pending;
    var motors = (was && was.button === button) ? was.motors : (cfg.getMotors ? cfg.getMotors() : []);
    if (was && was.button === button) finish('aborted', 'released ' + button);
    var c = catalogOf(button);
    if (c && c.release === 'stop') sendStop(stopReq(button, motors, 'release'));
  }

  function init(options) {
    cfg = options || {};
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
    init: init, send: send, release: release, loadJson: loadJson,
    isBusy: function () { return !!pending; },
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
