/* ht9045_modal.js -- C++ 的 ShowMyMessage／MyMessageBox 家族在「任何頁面」照 golden 顯示並回答
 * ---------------------------------------------------------------------------
 * AI(W906-SMM) 20260925  Steven 20260925 指示（優先處理）：C++ 的 ShowMyMessage 在網頁上看不到，
 * 操作員會漏看 golden 的警告。golden：HT9011UC_Code_V3.33.912.0_20260908_Jimmy\mymessbox.cpp／.dfm。
 *
 * 載入方式：theme.js 檔尾注入（幾乎每一頁都載 theme.js），所以不必逐頁加 <script>。
 * 重複載入無害（__HT9045_MODAL__ 旗標）。
 *
 * ---------------------------------------------------------------------------
 * 誰負責畫（一個瀏覽器分頁裡只能有一個人畫，golden 是 modal、一次一個）
 * ---------------------------------------------------------------------------
 *   在 iframe 裡            → 不畫。上層（background.html 或獨立開的頁）負責。
 *   file: 協定               → 不畫。沒有 wb_serve；background 的 dialog-bridge 走 JSON 墊片。
 *   dialog 頁本身            → 不畫（Alert.*.html，body 有 data-dialog）。
 *   頂層且有 HTDialogBridge  → 'bridge'：background.html。dialog-bridge.js 讀同一個信箱、畫
 *                              #dialogBridge，回答經 ht9045_dialog_host.js（本次補了訊息通道）。
 *   其他頂層頁               → 'own'：本檔自己畫。main.html、Setup.*.html… 被直接打開的時候。
 *
 * ---------------------------------------------------------------------------
 * 通道（tools/wb_serve.cpp 檔尾 AI(W906-SMM) 區塊）
 * ---------------------------------------------------------------------------
 *   C++ → 網頁  信箱 /JSON/Message-dialog-request.json（伺服器先對到 JSON/runtime/，
 *               HttpStatic.cpp AI(W906-MAILBOX-RT)）。每 300 ms 讀一次；WS 的
 *               {"type":"modal"} 訊框只當「馬上去讀」的觸發。
 *               ⚠ 路徑用**絕對**的 /JSON/…：本檔跑在 page/ 底下的頁面，相對路徑會變成 /page/JSON/。
 *   網頁 → C++  WS dialog.response，tag = requestId（"msg-<N>"），value = 動作名
 *               OK／PAUSE（pnlPause）、YES／NO（pnlYes／pnlNo）、ACKNOWLEDGE（不停機頁）。
 *               dialog.response 免單一操作員權杖（WebBridgeServer.cpp:1446）—— 框一定要按得掉。
 *
 * 畫面本體沿用 Alert.MyMessageBox.html／Alert.MyMessageBox.NonStop.html（dfm 產生與 Steven 0922
 * 的不停機小窗），協定同 dialog-bridge：READY → REQUEST → ACTION。
 *
 * ---------------------------------------------------------------------------
 * 排隊（照 golden：modal，一次一個）
 * ---------------------------------------------------------------------------
 *   會停機的（阻塞型、或 stopAllMotor=true 的非阻塞型）：中央遮罩，FIFO，一次一個。
 *   不停機的（requestedSideEffects.stopAllMotor=false）：右下角小窗，不擋畫面，FIFO，上限 8
 *   （與 dialog-bridge.js 的 queueNS 相同，Steven 20260922「排隊而不是覆蓋」）。
 *   C++ 那一則已經收尾（別的畫面先答了、被新的一則取代）→ 信箱不再是它 → 這裡的框自動收掉，
 *   不留一個按了也沒人收的框。
 */
(function (g) {
  'use strict';
  if (g.__HT9045_MODAL__) return;
  g.__HT9045_MODAL__ = true;

  var BUILD = '20260925-smm';
  var POLL_MS = 300;
  var MBOX_URL = '/JSON/Message-dialog-request.json';
  var NS_MAX = 8, SEEN_MAX = 64;

  var SELF_DIR = (function () {
    var s = document.currentScript;
    if (!s) {
      var all = document.getElementsByTagName('script');
      for (var i = all.length - 1; i >= 0; i--) {
        if (/ht9045_modal\.js/.test(all[i].src || '')) { s = all[i]; break; }
      }
    }
    var src = (s && s.src) ? String(s.src) : '';
    return src ? src.substring(0, src.lastIndexOf('/') + 1) : '';
  })();

  var mode = 'pending', why = '';
  var lastSeq = 0, seen = [];
  var qStop = [], qNS = [];
  var active = null, activeNS = null;
  var frames = {};                 // 'message' | 'messageNonStop' -> {el, ready}
  var polling = false;

  function isTop() { try { return g.top === g; } catch (e) { return false; } }
  function warn(t) { try { console.warn('[HT9045Modal] ' + t); } catch (e) {} }
  function off(reason) { mode = 'off'; why = reason; }
  function pageMode() { return (/[?&]mode=(release|debug)/.exec(location.search) || [])[1] || 'release'; }

  // ---- 決定誰畫 --------------------------------------------------------------
  function decide() {
    if (!isTop()) return off('iframe');
    if (location.protocol === 'file:') return off('file');
    if (document.body && document.body.hasAttribute('data-dialog')) return off('dialog-page');
    if (g.HTDialogBridge) { mode = 'bridge'; why = 'background dialog-bridge.js'; return; }
    mode = 'own'; why = 'standalone page';
    install();
    hookWs();
    poll();
    setInterval(function () { poll(); drain(); }, POLL_MS);
  }

  // ---- 信箱 ------------------------------------------------------------------
  function poll() {
    if (polling || mode !== 'own') return;
    polling = true;
    fetch(MBOX_URL + '?_=' + Date.now(), { cache: 'no-store' }).then(function (r) {
      return r.ok ? r.json() : null;
    }).then(function (req) { polling = false; onMailbox(req); },
            function () { polling = false; });
  }

  function alreadySeen(id) {
    if (seen.indexOf(id) >= 0) return true;
    seen.push(id);
    if (seen.length > SEEN_MAX) seen.shift();
    return false;
  }

  function isNonStop(req) {
    var se = req && req.requestedSideEffects;
    return !!((se && se.stopAllMotor === false) || (req && req.nonStop === true));
  }

  function onMailbox(req) {
    if (!req) return;
    var pending = req.state === 'pending' && !!req.requestId;
    // C++ 那一則已經收尾 ⇒ 收掉這裡的框（只看會停機的那一格；不停機的照 dialog-bridge 等操作員確認）
    if (active && !active.submitting && !(pending && req.requestId === active.req.requestId)) {
      closeActive('C++ 已收尾（' + (pending ? '換成 ' + req.requestId : req.state) + '）');
    }
    if (!pending) return;
    var seq = Number(req.seq) || 0;
    if (seq <= lastSeq) return;
    lastSeq = seq;
    if (alreadySeen(req.requestId)) return;
    if (isNonStop(req)) {
      qNS.push(req);
      while (qNS.length > NS_MAX) warn('不停機佇列滿了（上限 ' + NS_MAX + '），丟棄最舊的一則：' + qNS.shift().requestId);
    } else {
      qStop.push(req);
    }
    drain();
  }

  function drain() {
    if (!active && qStop.length) { active = { req: qStop.shift(), submitting: false, at: Date.now() }; showStop(); }
    if (!activeNS && qNS.length) { activeNS = { req: qNS.shift(), submitting: false, at: Date.now() }; showNS(); }
  }

  // ---- 畫面 ------------------------------------------------------------------
  function install() {
    var st = document.createElement('style');
    st.textContent =
      '#htMsgHost{position:fixed;inset:0;z-index:2147480000;display:none;align-items:center;justify-content:center;background:rgba(0,0,0,.55)}' +
      '#htMsgHost.open{display:flex}' +
      '#htMsgHost .htmWin,#htMsgNS .htmWin{display:flex;flex-direction:column;background:var(--panel,#ece9d8);' +
      'border:1px solid var(--win-border,#456);border-radius:4px;overflow:hidden;box-shadow:4px 4px 16px rgba(0,0,0,.6)}' +
      '#htMsgHost .htmTitle,#htMsgNS .htmTitle{background:linear-gradient(90deg,var(--tbar-focus1,#0a246a),var(--tbar-focus2,#a6caf0));' +
      'color:var(--tbar-text,#fff);font:bold 12px sans-serif;padding:3px 8px;user-select:none}' +
      '#htMsgHost iframe,#htMsgNS iframe{border:none;display:block;background:var(--panel,#ece9d8)}' +
      '#htMsgHost .htmStatus,#htMsgNS .htmStatus{padding:2px 8px;font:12px Consolas,monospace;color:#ffd;background:#7b2020;display:none}' +
      '#htMsgHost .htmStatus.show,#htMsgNS .htmStatus.show{display:block}' +
      // 不停機：右下角、沒有遮罩、不擋底下的畫面（同 dialog-bridge.js #dialogNonStop 的理由）
      '#htMsgNS{position:fixed;right:16px;bottom:16px;z-index:2147480001;display:none;flex-direction:column;pointer-events:none}' +
      '#htMsgNS.open{display:flex}#htMsgNS .htmWin{pointer-events:auto}';
    document.head.appendChild(st);

    var host = document.createElement('div');
    host.id = 'htMsgHost';
    host.setAttribute('role', 'dialog');
    host.setAttribute('aria-modal', 'true');
    // golden mymessbox.dfm Caption='Message'；ClientWidth/Height 由 FormShow 設成 480x250（:283-284），頁面 472x219
    host.innerHTML = '<section class="htmWin"><header class="htmTitle">Message</header>' +
      '<iframe data-kind="message" width="472" height="219" title="MyMessageBox"></iframe>' +
      '<div class="htmStatus"></div></section>';
    document.body.appendChild(host);

    var ns = document.createElement('div');
    ns.id = 'htMsgNS';
    ns.innerHTML = '<section class="htmWin"><header class="htmTitle">Message（不停機）</header>' +
      '<iframe data-kind="messageNonStop" width="420" height="200" title="MyMessageBox NonStop"></iframe>' +
      '<div class="htmStatus"></div></section>';
    document.body.appendChild(ns);

    frames.message = { el: host.querySelector('iframe'), ready: false };
    frames.messageNonStop = { el: ns.querySelector('iframe'), ready: false };
    frames.message.el.src = SELF_DIR + 'Alert.MyMessageBox.html?mode=' + pageMode();
    frames.messageNonStop.el.src = SELF_DIR + 'Alert.MyMessageBox.NonStop.html?mode=' + pageMode();

    g.addEventListener('message', onFrameMessage);
    // golden modal：框後面的東西都不能動。遮罩擋住滑鼠；鍵盤在這裡擋（Esc 不關框 —— 警告要被看見）。
    document.addEventListener('keydown', function (ev) {
      if (!active) return;
      if (ev.key === 'Escape' || !host.contains(ev.target)) { ev.preventDefault(); ev.stopPropagation(); }
    }, true);
  }

  function status(which, text) {
    var el = document.querySelector((which === 'ns' ? '#htMsgNS' : '#htMsgHost') + ' .htmStatus');
    if (!el) return;
    el.textContent = text || '';
    el.className = 'htmStatus' + (text ? ' show' : '');
  }

  function sendReq(kind) {
    var f = frames[kind];
    var st = kind === 'message' ? active : activeNS;
    if (!f || !f.ready || !st) return;
    try { f.el.contentWindow.postMessage({ type: 'HT_DIALOG_REQUEST', kind: kind, request: st.req }, '*'); } catch (e) {}
  }

  function showStop() {
    status('stop', '');
    document.getElementById('htMsgHost').className = 'open';
    sendReq('message');
  }
  function showNS() {
    status('ns', '');
    document.getElementById('htMsgNS').className = 'open';
    sendReq('messageNonStop');
  }
  function closeActive(reason) {
    if (!active) return;
    if (reason) warn('關閉 ' + active.req.requestId + '：' + reason);
    active = null;
    document.getElementById('htMsgHost').className = '';
    drain();
  }
  function closeNS() {
    activeNS = null;
    document.getElementById('htMsgNS').className = '';
    drain();
  }

  function onFrameMessage(e) {
    var m = e.data;
    if (!m || !m.type) return;
    var kind = null;
    Object.keys(frames).forEach(function (k) { if (frames[k].el && e.source === frames[k].el.contentWindow) kind = k; });
    if (!kind) return;
    if (m.type === 'HT_DIALOG_READY') { frames[kind].ready = true; sendReq(kind); return; }
    if (m.type !== 'HT_DIALOG_ACTION') return;           // HT_DIALOG_EVENT（ALARM_RESET）：golden 只消音不關框
    var a = m.action || {};
    if (kind === 'message' && active && m.requestId === active.req.requestId) answer(active, String(a.name || ''), 'stop');
    if (kind === 'messageNonStop' && activeNS && m.requestId === activeNS.req.requestId) answer(activeNS, 'ACKNOWLEDGE', 'ns');
  }

  function answer(st, name, which) {
    if (st.submitting) return;
    name = name.toUpperCase();
    if (!name || name === 'NONE') return;
    st.submitting = true;
    status(which, '送出中…');
    send(st.req.requestId, name).then(function () {
      if (which === 'ns') closeNS(); else closeActive();
    }, function (err) {
      var msg = (err && err.message) || String(err);
      // no query pending：C++ 那一則已經收尾；read-only：答不回去 —— 兩種都關掉，不留一個按了沒反應的框。
      // 不停機的本來就不等（C++ 不阻塞），送不到也關（同 dialog-bridge.js completeNonStop）。
      if (which === 'ns' || /no query pending|bridge is read-only/i.test(msg)) {
        warn(st.req.requestId + ' 的回答：' + msg + '（框照關）');
        if (which === 'ns') closeNS(); else closeActive();
        return;
      }
      st.submitting = false;
      status(which, '回答沒有送到 C++：' + msg + '（可再按一次）');
    });
  }

  // ---- 回答的傳輸：有 ht9045_recipe_client.js 就用它的連線，沒有就自己開一條 ----------
  function send(requestId, value) {
    var R = g.HT9045Recipe;
    if (R && typeof R.dialogResponse === 'function') return R.dialogResponse(requestId, value);
    return ownCmd('dialog.response', { tag: String(requestId), value: String(value) });
  }

  var sock = null, opening = null, nextId = 1, pend = {};
  function wsUrl() { return (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/ht9045'; }
  function connect() {
    if (sock && sock.readyState === 1) return Promise.resolve(sock);
    if (opening) return opening;
    opening = new Promise(function (resolve, reject) {
      var s;
      try { s = g.HT9045Link ? g.HT9045Link.open(wsUrl(), 'modal') : new WebSocket(wsUrl()); } catch (e) { opening = null; reject(e); return; }   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：在外框裡經 hub（ht9045_link.js）
      s.onopen = function () { sock = s; opening = null; resolve(s); };
      s.onerror = function () { opening = null; reject(new Error('cannot open ' + wsUrl())); };
      s.onclose = function () {
        sock = null;
        Object.keys(pend).forEach(function (k) { clearTimeout(pend[k].t); pend[k].reject(new Error('socket closed before ack')); delete pend[k]; });
      };
      s.onmessage = function (ev) {
        var m; try { m = JSON.parse(ev.data); } catch (e) { return; }
        if (!m) return;
        if (m.type === 'ack' && pend[m.id]) {
          var p = pend[m.id]; delete pend[m.id]; clearTimeout(p.t);
          if (m.ok) p.resolve(m); else p.reject(new Error(m.error || 'command refused'));
        } else if (m.type === 'modal') {
          poll();
        }
      };
    });
    return opening;
  }
  function ownCmd(name, extra) {
    return connect().then(function (s) {
      return new Promise(function (resolve, reject) {
        var id = nextId++, msg = { type: 'cmd', id: id, cmd: name };
        Object.keys(extra || {}).forEach(function (k) { msg[k] = extra[k]; });
        var t = setTimeout(function () {
          if (pend[id]) { delete pend[id]; reject(new Error(name + ': no ack within 15000ms')); }
        }, 15000);
        pend[id] = { resolve: resolve, reject: reject, t: t };
        s.send(JSON.stringify(msg));
      });
    });
  }

  // WS 的 modal 訊框 = 「信箱剛寫了一則」，馬上去讀，不用等下一次輪詢。
  function hookWs() {
    var T = g.HT9045Tags;
    if (!T || typeof T.onEvent !== 'function') return;          // 沒有 tag 串流就靠輪詢（300 ms）
    T.onEvent(function (m) { if (m && m.type === 'modal') poll(); });
    if (typeof T.connect === 'function') T.connect().catch(function () {});
  }

  g.HT9045Modal = {
    build: BUILD,
    mode: function () { return mode; },
    why: function () { return why; },
    active: function () { return active ? active.req : null; },
    activeNS: function () { return activeNS ? activeNS.req : null; },
    queues: function () { return { stop: qStop.length, nonStop: qNS.length }; },
    poll: poll
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', decide);
  else decide();
})(window);
