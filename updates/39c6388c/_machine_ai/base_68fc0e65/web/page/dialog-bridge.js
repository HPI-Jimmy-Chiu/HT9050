/* ShowErrorMessage / ShowMyMessage bidirectional modal bridge. */
/* Page side of contract 1.3.1 -- 1.3.1 (20261003 S-17): inspectClose handles Dialog-close-request.recent[] and closes queued (not yet shown) boxes too.  AI(W906-S17D) 20261003 (St01) */
(function (global) {
  'use strict';

  var POLL_MS = 100;
  var channels = {
    alarm: { request: 'Alarm-dialog-request', response: 'Alarm-dialog-response', lastSeq: 0, loading: false },
    message: { request: 'Message-dialog-request', response: 'Message-dialog-response', lastSeq: 0, loading: false }
  };
  var closeChannel = { request: 'Dialog-close-request', response: 'Dialog-close-response', lastSeq: 0, loading: false };
  var authChannel = { verify: 'Dialog-auth-verify', result: 'Dialog-auth-result' };   // HTML 只收集輸入，驗證一律由 C++ 執行
  var active = null;
  /* Steven 20260922：不停機的告警自己一格，**不共用 active**。
   *
   * 解除路徑（使用者 20260922 說明，第二次更正後的正確版本）：
   *   **四種都可以用實體 IO 按鈕解除** —— 不是只有停機的那兩種。
   *     停機 note     兩段式：K_RETRY / K_SKIP 等功能鍵先「選取」，
   *                   再按 K_PAUSE（或 K_START）確認才關窗
   *                   golden: note.cpp:2962-2964 只 UpdateButtonStatus()；
   *                           note.cpp:3084 -> Start()、:3087 -> BtnPauseClick()
   *     停機 message  一段式：K_PAUSE / K_RETRY / K_SKIP 任一鍵直接關
   *                   golden: mymessbox.cpp:564
   *     （面板上沒有 Stop 鍵 —— SnFKStop / K_STOP 不存在，見 cmydef.h:694-726）
   *     完整規則見 skill `ht9045-alarm-dismissal`。
   *   **不停機的「額外」允許滑鼠點畫面上的按鈕解除**，因為它比較不重要。
   *
   * ⇒ 由此推出兩個設計要求：
   *   (a) 不停機的窗必須在最上層且不被遮住 —— 滑鼠點得到才有意義
   *   (b) 實體鍵那條路（Dialog-close-request）必須**也能關到它**，
   *       不能只認 active（見 inspectClose）
   *
   * 不共用 active 的理由是另一件事：機台停著、停機告警開著的時候，
   * 權限不足的提示不該被丟掉 —— 那正是它最該出現的時候。 */
  var activeNS = null;
  var auth = null;       // 進行中的權限驗證 {authId, definition, timer}
  var pendingCloseResponse = null;
  var closeResponseSubmitting = false;
  var closeResponseBacklog = [];   // AI(W906-S17D) 20261003 (St01)：一輪可能關好幾個框（recent[]），送出中又來的關閉回報排在這裡，輪詢補送

  function jsonBase() {
    var path = location.pathname.replace(/\\/g, '/');
    return path.substring(0, path.lastIndexOf('/') + 1) + 'JSON/';
  }

  function loadFresh(name) {
    var url = jsonBase() + name + '.json';
    if (location.protocol !== 'file:') {
      return fetch(url + '?_=' + Date.now(), { cache: 'no-store' }).then(function (response) {
        if (!response.ok) throw new Error('HTTP ' + response.status);
        return response.json();
      });
    }
    return new Promise(function (resolve, reject) {
      var store = global.__HT9045_DATA__ || (global.__HT9045_DATA__ = {});
      var script = document.createElement('script');
      delete store[name];
      script.src = jsonBase() + 'js/' + name + '.js?_=' + Date.now();
      script.onload = function () {
        var value = store[name];
        script.remove();
        value ? resolve(value) : reject(new Error('empty shim ' + name));
      };
      script.onerror = function () { script.remove(); reject(new Error('shim load failed ' + name)); };
      document.head.appendChild(script);
    });
  }

  /* //Steven 20260922：多兩個不停機的顯示 kind。
   * NonStop 的定義是「C++ 那邊不呼叫 StopAllMotor()」（使用者 20260922 裁定）。
   * 這裡只決定**畫哪一頁**；停不停機在 request 送到瀏覽器之前就已成定局，
   * 見 ht9045_nonstop_alarm.js 檔頭。 */
  var PAGES = { alarm: 'page/Alert.Note.html', message: 'page/Alert.MyMessageBox.html', auth: 'page/Alert.Password.html',
                alarmNonStop: 'page/Alert.Note.NonStop.html', messageNonStop: 'page/Alert.MyMessageBox.NonStop.html' };
  var NONSTOP_OF = { alarm: 'alarmNonStop', message: 'messageNonStop' };
  var frames = {};       // kind → {el, ready}

  function pageBase() {
    var path = location.pathname.replace(/\\/g, '/');
    return path.substring(0, path.lastIndexOf('/') + 1);
  }
  function withMode(src) {
    var mode = (/[?&]mode=(release|debug)/.exec(location.search) || [])[1] || 'release';
    return src + (src.indexOf('?') < 0 ? '?' : '&') + 'mode=' + mode;
  }

  function installView() {
    var style = document.createElement('style');
    style.textContent =
      '#dialogBridge{position:fixed;inset:0;z-index:20000;display:none;align-items:center;justify-content:center;background:rgba(0,0,0,.58)}' +
      '#dialogBridge.open{display:flex}' +
      '#dialogBridge .dbWindow{display:none;flex-direction:column;background:var(--panel,#ece9d8);border:1px solid var(--win-border,#456);border-radius:4px;overflow:hidden;box-shadow:4px 4px 16px rgba(0,0,0,.6)}' +
      '#dialogBridge .dbWindow.show{display:flex}' +
      '#dialogBridge .dbTitle{background:linear-gradient(90deg,var(--tbar-focus1,#0a246a),var(--tbar-focus2,#a6caf0));color:var(--tbar-text,#fff);font-weight:bold;padding:3px 8px;font-size:12px;user-select:none}' +
      '#dialogBridge iframe{border:none;display:block;background:var(--panel,#ece9d8)}' +
      '#dialogBridge .dbStatus{position:absolute;left:0;right:0;bottom:0;padding:2px 8px;font:12px Consolas,monospace;color:#ffd;background:#7b2020;display:none}' +
      '#dialogBridge .dbStatus.show{display:block}' +
      // 置頂層級：桌面視窗 zTop（幾百）＜ layoutBar/compPalette 9999 ＜ #dialogBridge 20000 ＜ #dialogAuth 21000 ＜ HTQwerty .qkOv 99999
      'body.dialogOpen #desktopWrap,body.dialogOpen #taskbar,body.dialogOpen #layoutBar,body.dialogOpen #compPalette{pointer-events:none}' +
      '#dialogAuth{position:fixed;inset:0;z-index:21000;display:none;align-items:center;justify-content:center;background:rgba(0,0,0,.25)}' +
      '#dialogAuth.open{display:flex}' +
      '#dialogAuth .daWin{display:flex;flex-direction:column;background:var(--panel,#ece9d8);border:1px solid var(--win-border,#456);border-radius:4px;overflow:hidden;box-shadow:4px 4px 16px rgba(0,0,0,.6)}' +
      '#dialogAuth .daBar{background:linear-gradient(90deg,var(--tbar-focus1,#0a246a),var(--tbar-focus2,#a6caf0));color:#fff;font-weight:bold;padding:3px 8px;font-size:12px;user-select:none}' +
      '#dialogAuth iframe{border:none;display:block;background:var(--panel,#ece9d8)}' +
      '#dialogAuth .daMsg{padding:2px 8px;font:12px Consolas,monospace;color:#ffd;background:#7b2020;display:none}' +
      '#dialogAuth .daMsg.show{display:block}' +
      // Steven 20260922（使用者指定）：不停機的告警**必須在最上層**、且**可以單獨按掉**。
      // 所以它是獨立的一層，不是 #dialogBridge 裡的另一個 .dbWindow：
      //   * z-index 21500 -> 壓在 #dialogBridge(20000) 與 #dialogAuth(21000) 之上。
      //     唯一在它之上的是 HTQwerty 的 .qkOv(99999) —— 小鍵盤必須壓過所有東西，
      //     否則操作員看得到輸入框卻打不了字。
      //   * **沒有整片遮罩**，container 是 pointer-events:none ——
      //     底下那個會停機的告警仍然按得到。不停機的本來就不該擋住別人。
      //   * 固定在右下角，和置中的停機告警明顯分開，一眼看得出是兩件事。
      '#dialogNonStop{position:fixed;inset:auto 16px 16px auto;z-index:21500;' +
      'display:none;flex-direction:column;gap:8px;pointer-events:none}' +
      '#dialogNonStop.open{display:flex}' +
      '#dialogNonStop .dnWindow{display:none;flex-direction:column;pointer-events:auto;' +
      'background:var(--panel,#ece9d8);border:1px solid var(--win-border,#456);' +
      'border-radius:4px;overflow:hidden;box-shadow:4px 4px 16px rgba(0,0,0,.55)}' +
      '#dialogNonStop .dnWindow.show{display:flex}' +
      '#dialogNonStop .dnTitle{background:linear-gradient(90deg,var(--tbar-focus1,#0a246a),' +
      'var(--tbar-focus2,#a6caf0));color:var(--tbar-text,#fff);font-weight:bold;' +
      'padding:3px 8px;font-size:12px;user-select:none}' +
      '#dialogNonStop iframe{border:none;display:block;background:var(--panel,#ece9d8)}';
    document.head.appendChild(style);

    var view = document.createElement('div');
    view.id = 'dialogBridge';
    view.setAttribute('role', 'dialog');
    view.setAttribute('aria-modal', 'true');
    // fNote 1184×761 = note.dfm 的 ClientWidth/ClientHeight（Steven 20260924 修正）。
    //   原本寫 972（golden FormShow AUTO_EMPTY_COLOR<3 的 Width=980 扣邊框），
    //   但 HTML 頁把所有子元件照 dfm 絕對座標排，**沒有實作 golden 在 980 時的重排**
    //   （palAuto4-6／palFix4-6 隱藏、palOutArm 縮窄）。結果 Panel5 只剩 737 寬，
    //   機構圖右側 210px 連同 pnlSafeDoorRight(Left=973) 被 .pcPane 的 overflow:hidden 切掉。
    //   實測：Panel5 = 737×549（dfm 是 947×531）。
    // MyMessageBox 472×219（FormShow 預設 Width=480/Height=250）；標題列 +24
    view.innerHTML =
      '<section class="dbWindow" data-kind="alarm"><header class="dbTitle">Note</header>' +
      '<iframe data-kind="alarm" width="1184" height="761" title="fNote"></iframe><div class="dbStatus"></div></section>' +
      '<section class="dbWindow" data-kind="message"><header class="dbTitle">Message</header>' +
      '<iframe data-kind="message" width="472" height="219" title="MyMessageBox"></iframe><div class="dbStatus"></div></section>';
    document.body.appendChild(view);
    var authView = document.createElement('div');
    authView.id = 'dialogAuth';
    // fPassword 609×305（Password.dfm Width=617/Height=336 扣邊框）；登入頁本體由 dfm 生成（Alert.Password.html），帳密只經 bridge 轉交 C++
    authView.innerHTML = '<div class="daWin"><div class="daBar">Password</div>' +
      '<iframe data-kind="auth" width="609" height="305" title="fPassword"></iframe><div class="daMsg"></div></div>';
    document.body.appendChild(authView);
    // Steven 20260922：不停機的兩個窗自成一層（見上面的 CSS 註解）。
    // 刻意做小（420x230 / 420x200）且不帶動作鍵 —— 版面差異本身就是訊息。
    var nsView = document.createElement('div');
    nsView.id = 'dialogNonStop';
    nsView.innerHTML =
      '<section class="dnWindow" data-kind="alarmNonStop"><header class="dnTitle">Note（不停機）</header>' +
      '<iframe data-kind="alarmNonStop" width="420" height="230" title="fNote NonStop"></iframe></section>' +
      '<section class="dnWindow" data-kind="messageNonStop"><header class="dnTitle">Message（不停機）</header>' +
      '<iframe data-kind="messageNonStop" width="420" height="200" title="MyMessageBox NonStop"></iframe></section>';
    document.body.appendChild(nsView);
    ['alarmNonStop', 'messageNonStop'].forEach(function (kind) {
      var el = nsView.querySelector('iframe[data-kind="' + kind + '"]');
      frames[kind] = { el: el, ready: false };
      el.src = withMode(pageBase() + PAGES[kind]);
    });
    ['alarm', 'message'].forEach(function (kind) {
      var el = view.querySelector('iframe[data-kind="' + kind + '"]');
      frames[kind] = { el: el, ready: false };
      el.src = withMode(pageBase() + PAGES[kind]);
    });
    frames.auth = { el: authView.querySelector('iframe[data-kind="auth"]'), ready: false };
    frames.auth.el.src = withMode(pageBase() + PAGES.auth);
    document.addEventListener('keydown', function (event) {
      if (active && event.key === 'Escape') { event.preventDefault(); event.stopPropagation(); }
    }, true);
    global.addEventListener('message', onPageMessage);
  }

  function frameOf(kind) { return frames[kind] && frames[kind].el; }
  function status(kind, text) {
    var el = document.querySelector('#dialogBridge .dbWindow[data-kind="' + kind + '"] .dbStatus');
    if (!el) return;
    el.textContent = text || '';
    el.className = 'dbStatus' + (text ? ' show' : '');
  }

  function sendRequest(kind) {
    var f = frames[kind];
    if (!f || !f.ready) return;                        // 等 HT_DIALOG_READY 再送
    /* Steven 20260922：比對的是 displayKind 不是 channel kind ——
     * 不停機的那兩頁是同一個 channel 的另一種畫法，channel kind 必須保持
     * 'alarm'/'message'，否則 responseFor() 與 channels[] 都會查不到。
     *
     * ⚠ 20260922 實機驗證抓到的缺陷：原本這裡**只看 active**。
     *   不停機的那一層用的是 activeNS（刻意不共用，見檔頭），
     *   所以 renderNonStop() 末尾那次 sendRequest() 一定在這行被擋掉，
     *   造成兩個症狀：
     *     (1) 不停機的視窗是**空白的** —— 頁面沒收到 HT_DIALOG_REQUEST
     *     (2) 滑鼠**關不掉**它 —— 頁內 current 永遠是 null，
     *         acknowledge() 開頭就 return，HT_DIALOG_ACTION 根本沒送出
     *   而「不停機允許用滑鼠點畫面按鈕解除」正是它和停機版唯一的差別。
     *   兩組 displayKind 互斥（alarm/message vs alarmNonStop/messageNonStop），
     *   所以這裡先比 activeNS 再比 active 不會有歧義。 */
    var st = (activeNS && activeNS.displayKind === kind) ? activeNS
           : (active && active.displayKind === kind) ? active
           : null;
    if (!st) return;
    try { f.el.contentWindow.postMessage({ type: 'HT_DIALOG_REQUEST', kind: kind, request: st.request }, '*'); } catch (e) {}
  }

  function onPageMessage(e) {
    var m = e.data; if (!m || !m.type) return;
    var kind = m.kind;
    // Steven 20260922：web 端自行發起的不停機告警（HT9045NonStop.raise）。
    // 來源是桌面上任何一個 iframe，不是 dialog frame，所以要在下面那道
    // 來源過濾之前處理。
    if (m.type === 'HT_NONSTOP_RAISE') { raiseNonStop(m); return; }
    if (!frames[kind] || e.source !== frames[kind].el.contentWindow) return;
    if (kind === 'auth') return onAuthPageMessage(m);
    if (m.type === 'HT_DIALOG_READY') { frames[kind].ready = true; sendRequest(kind); return; }
    /* 不停機的窗：自己的狀態、自己的關閉路徑，不經過 active / complete()。
     * 這是**滑鼠**那條路；實體 IO 按鈕那條在 inspectClose()。兩條都要能關掉它。
     * ⚠ **刻意不做 needsAuth()**。停機告警的解除可以要密碼
     *   （golden note.cpp:5261 TfNote::DoPassword、mymessbox.cpp:458 DoPassword_MBox，
     *    契約的 request.auth.required 就是它），但不停機的不該要 ——
     *   其中一種就是「權限不足」本身，再要一次權限等於把人鎖在門外。
     *   若日後真有需要密碼才能關的不停機告警，那它就不該是不停機的。 */
    if (isNonStop(kind)) {
      if (!activeNS || activeNS.displayKind !== kind ||
          m.requestId !== (activeNS.request || {}).requestId) return;
      if (m.type === 'HT_DIALOG_ACTION') completeNonStop();
      return;
    }
    if (!active || active.displayKind !== kind || m.requestId !== active.request.requestId) return;
    if (m.type === 'HT_DIALOG_ACTION') {
      var a = m.action || {};
      var definition = { name: a.name, label: a.name, code: a.code, pressedButton: m.pressedButton || null,
                         closedBy: a.name === 'ALARM_RESET' ? 'alarm-reset' : undefined };
      if (needsAuth()) openAuth(definition); else complete(definition);
    } else if (m.type === 'HT_DIALOG_EVENT') {
      global.dispatchEvent(new CustomEvent('ht-dialog-event', { detail: { kind: kind, requestId: m.requestId, event: m.event } }));
    }
  }

  function render(kind, request, displayKind) {
    displayKind = displayKind || kind;
    active = { kind: kind, displayKind: displayKind, request: request,
               openedAt: Date.now(), submitting: false, authVerified: null };
    var view = document.getElementById('dialogBridge');
    view.className = 'open ' + displayKind;
    document.body.classList.add('dialogOpen');
    view.querySelectorAll('.dbWindow').forEach(function (w) { w.classList.toggle('show', w.dataset.kind === displayKind); });
    status(displayKind, '');
    sendRequest(displayKind);
  }

  /* Steven 20260922：查 AlarmNonStop.json 決定畫哪一頁，再 render。
   * 查表失敗一律退回原本會停機的頁 —— 保守側。把該停的畫成不該停的，
   * 比沒有這個功能更糟。 */
  function resolveDisplay(kind, request) {
    if (!global.HT9045NonStop || !global.HT9045NonStop.route) {
      return Promise.resolve({ kind: kind, info: null, why: 'no-router' });
    }
    return global.HT9045NonStop.route(kind, request).then(function (r) {
      var dk = (r && r.kind) || kind;
      if (!frames[dk]) dk = kind;                       // 頁沒部署 -> 退回會停機的那一頁
      return { kind: dk, info: r && r.info, why: r && r.why };
    }, function () { return { kind: kind, info: null, why: 'router-failed' }; });
  }

  function isNonStop(dk) { return dk === 'alarmNonStop' || dk === 'messageNonStop'; }

  /* 不停機的顯示／關閉 —— 完全不碰 active、不碰 #dialogBridge。
   * 停機那條路是機台真的在用的，這裡一個字都不動它。 */
  function renderNonStop(kind, request, dk) {
    activeNS = { kind: kind, displayKind: dk, request: request, openedAt: Date.now() };
    var v = document.getElementById('dialogNonStop');
    v.classList.add('open');
    v.querySelectorAll('.dnWindow').forEach(function (w) {
      w.classList.toggle('show', w.dataset.kind === dk);
    });
    sendRequest(dk);
  }

  function closeNonStop() {
    activeNS = null;
    var v = document.getElementById('dialogNonStop');
    v.classList.remove('open');
    v.querySelectorAll('.dnWindow').forEach(function (w) { w.classList.remove('show'); });
  }

  /* 按下螢幕上那顆確認鍵。
   * 本地發起的（channel 'nonstop-local'）沒有對應的 C++ request，只關窗；
   * C++ 發來的依契約 blocking:false，C++ 不等回應，response 只是畫面關閉的紀錄。 */
  function completeNonStop() {
    if (!activeNS) return;
    var req = activeNS.request || {};
    var ch = channels[activeNS.kind];
    var local = (req.channel === 'nonstop-local');
    if (!local && ch) {
      submit({
        schemaVersion: '1.0.0', channel: req.channel, seq: Date.now(),
        requestId: req.requestId, requestSeq: req.seq,
        state: 'completed', accepted: true, closedBy: 'action-button',
        completedAt: new Date().toISOString(),
        durationMs: Date.now() - activeNS.openedAt, error: null,
        selectedAction: 'ACKNOWLEDGE', nonStop: true
      }, ch.response).catch(function (e) {
        // 不停機的回應送不出去不該卡住畫面 —— C++ 本來就沒在等
        console.warn('[NonStop] 回應沒送出（C++ 不等待，畫面照關）：' + e.message);
      });
    }
    closeNonStop();
  }

  /* =========================================================================
   *  兩條獨立佇列（Steven 20260922 裁定）
   * =========================================================================
   *  使用者的裁定：**停機告警的「顯示」本身就是安全要件** ——
   *  它關係到人身安全與機台重大風險（ESD、安全門、溫度異常、EMG）。
   *  所以會停機的那條佇列**永不丟棄**，這是**刻意偏離 golden** 的地方。
   *
   *  golden 的做法是丟掉（note.cpp:824-828）：
   *      if(fNote->fShow)
   *      {
   *          MyDBIProcess("Exception", "Alarm at same time: "+Code, errPart);
   *          return 0;                         // <- 第二則直接消失，不排隊
   *      }
   *  在 golden 那是可接受的，因為那個 return 發生在 StopAllMotor()（:805-808）
   *  **之後** —— 機台已經停了，沒顯示只是少一筆資訊。
   *  但在 web 版，C++ 停機與瀏覽器顯示是兩個行程、兩條通道，
   *  「沒顯示」可能是操作員唯一會察覺的訊號，所以不能丟。
   *
   *  多個 alarm 會不會連續出現？**會**，golden 為此專門寫了上面那道保護。
   *  另外 TfNote::ScanKey() 在 fNote **顯示中**（Timer1Timer 開頭 if(!fShow) return）
   *  還會呼叫 ShowErrorMessage（note.cpp:3069 / :3077）—— 真正的巢狀。
   *  全樹 ShowErrorMessage 呼叫點 1950 個。
   *
   *  ⚠ 舊做法為什麼不夠：
   *    原本是「`if (active) return;` 且**不推進 lastSeq**，下一輪再讀檔案」。
   *    那不是佇列，是重讀 —— 它依賴 C++ 把 request 檔維持在 pending。
   *    C++ 只要在這期間把同一個單槽檔覆寫成新的一則，
   *    **前一則就永久消失，而且沒有任何痕跡**。
   *  ⇒ 現在改成：讀到就**立刻推進 lastSeq 並入列**，之後只從佇列出貨。
   * ========================================================================= */
  var queueStop = [];          // 會停機：FIFO，**永不丟棄**
  var queueNS = [];            // 不停機：FIFO，滿了丟**最舊**的（保留最新，符合 golden 的覆蓋語意）
  var NS_QUEUE_MAX = 8;
  var STOP_DEPTH_WARN = 3;     // 超過就在 console 出聲 —— 正常情況不該堆起來
  var seenIds = [];            // 去重用，只留最近 64 筆
  var SEEN_MAX = 64;

  function alreadySeen(id) {
    if (!id) return false;                       // 沒有 requestId 就不去重，寧可重複也不要漏
    if (seenIds.indexOf(id) >= 0) return true;
    seenIds.push(id);
    if (seenIds.length > SEEN_MAX) seenIds.shift();
    return false;
  }

  function enqueue(kind, request, displayKind) {
    if (alreadySeen(request && request.requestId)) return false;
    var item = { kind: kind, displayKind: displayKind, request: request, at: Date.now() };
    if (isNonStop(displayKind)) {
      queueNS.push(item);
      while (queueNS.length > NS_QUEUE_MAX) {
        var dropped = queueNS.shift();
        console.warn('[Dialog] 不停機佇列滿了（上限 ' + NS_QUEUE_MAX + '），丟棄最舊的一則：'
                     + ((dropped.request.arguments || {}).code || dropped.request.requestId || '?'));
      }
    } else {
      queueStop.push(item);
      /* 永不丟棄，但要出聲 —— 堆起來代表有人沒把前一則關掉，
       * 或 C++ 在阻塞等回應的同時又送了新的，兩者都該被看見。 */
      if (queueStop.length >= STOP_DEPTH_WARN) {
        console.error('[Dialog] **停機告警佇列深度 ' + queueStop.length + '** —— '
                      + '這些都不會被丟棄，但堆積本身是異常。目前排隊中：'
                      + queueStop.map(function (q) {
                          return (q.request.arguments || {}).code || q.request.requestId || '?';
                        }).join(', '));
      }
    }
    return true;
  }

  /* 出貨。放在輪詢迴圈裡跑，不掛在每一個關閉點上 ——
   * 關閉點有五處（complete 兩處、inspectClose、completeNonStop、raiseNonStop），
   * 漏掉任何一處都會讓佇列靜默卡住。輪詢延遲 100ms，換到的是漏不掉。 */
  function drain() {
    if (!active && queueStop.length) {
      var a = queueStop.shift();
      render(a.kind, a.request, a.displayKind);
    }
    if (!activeNS && queueNS.length) {
      var b = queueNS.shift();
      renderNonStop(b.kind, b.request, b.displayKind);
    }
  }

  function routeAndRender(kind, request) {
    resolveDisplay(kind, request).then(function (r) {
      if (r.info) request.nonStopInfo = Object.assign({}, r.info, request.nonStopInfo || {});
      if (isNonStop(r.kind)) renderNonStop(kind, request, r.kind);
      else render(kind, request, r.kind);
    });
  }

  /* web 端自行發起（不經過 C++）。StopAllMotor() 連被呼叫的機會都沒有，
   * 所以這一條是唯一真的能保證「機台沒停」的路徑。 */
  function raiseNonStop(msg) {
    /* ⚠ 這裡**刻意沒有** `if (active) return` ——
     * 初版有，而那是個退步：機台停著、停機告警開著的時候，權限不足的提示會被
     * 直接丟掉（只留一行 console.warn），而那正是它最該出現的時候。 */
    var kind = (msg && msg.kind === 'message') ? 'message' : 'alarm';
    var dk = NONSTOP_OF[kind];
    if (!frames[dk]) {
      console.warn('[NonStop] ' + dk + ' 的頁面沒有部署，raise 無法顯示：', msg && msg.request);
      return;
    }
    /* 同時只顯示一個不停機的窗，但**排隊**而不是覆蓋。
     * golden 的 ShowUnloaderTrayMessage 是先 MyMessageBox->Close() 再 Show()
     * （等於覆蓋），這裡刻意做得寬一點：佇列上限 8，滿了才丟最舊的，
     * 所以「最新的一定看得到」這個 golden 語意保留，中間的也不會全丟。 */
    enqueue(kind, msg.request || {}, dk);
    drain();
  }

  function closeView() {
    cancelAuth();
    var view = document.getElementById('dialogBridge');
    view.className = '';
    document.body.classList.remove('dialogOpen');
    view.querySelectorAll('.dbWindow').forEach(function (w) { w.classList.remove('show'); });
  }

  // ---- 權限／密碼：BCB6 fNote::DoPassword()/DoUnlockPassword()/PanSpecialNoteClick、MyMessageBox::DoPassword_MBox()
  //      HTML 只收集 User ID/Password 並寫 Dialog-auth-verify；C++ 比對 LevelSet/密碼檔後寫 Dialog-auth-result；通過才送 normal response 與關畫面 ----
  function needsAuth() {
    var a = active && active.request.auth;
    return !!(a && a.required && !active.authVerified);
  }
  function authMsg(text) {
    var el = document.querySelector('#dialogAuth .daMsg');
    el.textContent = text || ''; el.className = 'daMsg' + (text ? ' show' : '');
  }
  function postAuthPage(msg) {
    var f = frames.auth;
    if (f && f.ready) { try { f.el.contentWindow.postMessage(msg, '*'); } catch (e) {} }
  }
  function sendAuthRequest(message) {
    if (!auth || !active) return;
    postAuthPage({ type: 'HT_DIALOG_AUTH_REQUEST', auth: active.request.auth || {}, message: message || null });
  }

  function onAuthPageMessage(m) {
    if (m.type === 'HT_DIALOG_READY') { frames.auth.ready = true; if (auth) sendAuthRequest(); return; }
    if (!auth) return;
    if (m.type === 'HT_DIALOG_INPUT') {                 // 鍵盤必須在頂層彈出（609×305 的登入頁容不下 HTQwerty）
      if (!global.HTQwerty) return;
      var proxy = document.createElement('input'); proxy.value = m.current || '';
      global.HTQwerty.show(proxy, m.flags || 0, {
        onCommit: function (v) { postAuthPage({ type: 'HT_DIALOG_INPUT_RESULT', field: m.field, value: v, commit: true }); }
      });
    } else if (m.type === 'HT_DIALOG_AUTH_SUBMIT') {
      submitAuth(m.userId, m.password);
    } else if (m.type === 'HT_DIALOG_AUTH_CANCEL') {
      cancelAuth();                                       // spbCancel：只關登入層，Alert 保持開啟，不送 JSON
    }
  }

  function openAuth(definition) {
    var a = active.request.auth || {};
    auth = { authId: 'auth-' + Date.now() + '-' + Math.random().toString(16).slice(2), definition: definition, timer: null, submitting: false };
    document.querySelector('#dialogAuth .daBar').textContent = a.title || 'Password';
    authMsg('');
    document.getElementById('dialogAuth').className = 'open';
    sendAuthRequest();
    global.dispatchEvent(new CustomEvent('ht-dialog-auth', { detail: { state: 'prompt', authId: auth.authId } }));
  }

  function cancelAuth() {
    if (!auth) return;
    if (auth.timer) clearTimeout(auth.timer);
    auth = null;
    document.getElementById('dialogAuth').className = '';
  }

  function submitAuth(userId, password) {
    if (!auth || !active || auth.submitting) return;
    var a = active.request.auth || {};
    var verify = {
      schemaVersion: '1.0.0', channel: 'dialog-auth', seq: Date.now(), authId: auth.authId, state: 'pending',
      requestedAt: new Date().toISOString(),
      target: { channel: active.request.channel, requestId: active.request.requestId, requestSeq: active.request.seq },
      kind: a.kind || 'access-level', level: a.level == null ? null : a.level,
      pendingAction: { name: auth.definition.name, code: auth.definition.code == null ? null : Number(auth.definition.code) || 0,
                       pressedButton: auth.definition.pressedButton || null },
      credentials: { userId: userId || null, password: password || '' }
    };
    auth.submitting = true;
    authMsg('Verifying...');
    var p = (global.HTDialogHost && typeof global.HTDialogHost.verifyAuth === 'function')
      ? Promise.resolve(global.HTDialogHost.verifyAuth(verify))
      : submit(verify, authChannel.verify).then(function () { return waitAuthResult(verify.authId); });
    p.then(function (result) {
      if (!auth || auth.authId !== verify.authId) return;
      if (result && result.accepted) {
        active.authVerified = { authId: verify.authId, accessLevel: result.accessLevel == null ? null : result.accessLevel, userId: result.userId || verify.credentials.userId };
        var def = auth.definition;
        cancelAuth();
        complete(def);
      } else {
        // cpp：密碼錯誤/權限不足 → fPassword Label3/Label4，Alert 保持開啟可重試
        auth.submitting = false;
        authMsg('');
        postAuthPage({ type: 'HT_DIALOG_AUTH_RESULT', accepted: false, message: (result && result.message) || null });
      }
    }).catch(function (err) {
      if (!auth) return;
      auth.submitting = false;
      authMsg(err.message);
    });
  }

  function waitAuthResult(authId, timeoutMs) {
    var deadline = Date.now() + (timeoutMs || 15000);
    return new Promise(function (resolve, reject) {
      (function poll() {
        if (!auth || auth.authId !== authId) return reject(new Error('auth cancelled'));
        loadFresh(authChannel.result).then(function (r) {
          if (r && r.authId === authId && r.state !== 'pending' && r.state !== 'idle') return resolve(r);
          if (Date.now() > deadline) return reject(new Error('C++ auth result timeout'));
          auth.timer = setTimeout(poll, 200);
        }).catch(function () {
          if (Date.now() > deadline) return reject(new Error('C++ auth result timeout'));
          auth.timer = setTimeout(poll, 200);
        });
      })();
    });
  }

  function responseFor(definition, closeRequest) {
    var request = active.request;
    var common = {
      schemaVersion: '1.0.0', channel: request.channel, seq: Date.now(), requestId: request.requestId,
      requestSeq: request.seq, state: 'completed', accepted: true,
      closedBy: closeRequest ? (closeRequest.closeReason || 'external-io') : (definition.closedBy || 'action-button'),
      completedAt: new Date().toISOString(), durationMs: Date.now() - active.openedAt, error: null
    };
    var reqAuth = active.request.auth || {};
    common.auth = { required: !!reqAuth.required, verified: !!active.authVerified,
                    authId: active.authVerified ? active.authVerified.authId : null,
                    accessLevel: active.authVerified ? active.authVerified.accessLevel : null,
                    bypassedByCloseRequest: !!(closeRequest && reqAuth.required && !active.authVerified) };
    if (active.kind === 'alarm') {
      common.selectedAction = { name: definition.name, code: Number(definition.code) || 0 };
      // fNote::Start()（BtnStart）與 BtnPauseClick 回同一 ReturnCode，但 C++ 需分 Start/Pause 後續動作
      common.pressedButton = closeRequest ? null : (definition.pressedButton || null);
    } else {
      common.selectedAction = definition.name;
      common.sideEffects = {
        handlerPaused: null, motorsStopped: null, servoOffInArmXY: null,
        secsGemEventReported: null, formCloseCleanupCompleted: null
      };
    }
    return common;
  }

  function closeResponseFor(response, closeRequest, wasOpen, error) {
    var target = closeRequest && closeRequest.target || {};
    return {
      schemaVersion: '1.0.0', channel: 'dialog-close', seq: Date.now(),
      closeEventId: 'close-' + Date.now() + '-' + Math.random().toString(16).slice(2),
      closeRequestId: closeRequest ? closeRequest.closeRequestId : null,
      closeRequestSeq: closeRequest ? closeRequest.seq : null,
      state: error ? 'error' : 'completed', accepted: !error,
      target: {
        channel: response ? response.channel : (target.channel || ''),
        requestId: response ? response.requestId : (target.requestId || ''),
        requestSeq: response ? response.requestSeq : (Number(target.requestSeq) || 0)
      },
      trigger: {
        source: closeRequest && closeRequest.trigger ? closeRequest.trigger.source : 'html-action',
        inputName: closeRequest && closeRequest.trigger ? (closeRequest.trigger.inputName || null) : null
      },
      selectedAction: closeRequest && closeRequest.resolvedAction
        ? closeRequest.resolvedAction
        : (response ? response.selectedAction : { name: 'NONE', code: null }),
      closedBy: response ? response.closedBy : (closeRequest && closeRequest.closeReason || null),
      dialogWasOpen: !!wasOpen, closedAt: error ? null : new Date().toISOString(),
      normalResponse: { file: response ? channels[active && active.kind || 'alarm'].response + '.json' : '', seq: response ? response.seq : 0 },
      error: error || null
    };
  }

  function submit(response, responseName) {
    /* //Steven 20260924：第 1 段（HTDialogHost）**不通時要往下掉**，不能直接 return。
     * -----------------------------------------------------------------------
     * ht9045_dialog_host.js:143 在「沒有待答的 query」時回 Promise.reject
     * （'這個畫面的來源不是 wb_serve 的 modal ...'）。檔案信箱送來的告警
     * 每一則都會踩到這條 —— 原本寫成 `return Promise.resolve(f(...))`，
     * 等於把那個 reject 直接當成 submit() 的結果，後面三段傳輸與 debug 本機關閉
     * **一次都沒機會跑**。現場症狀就是：選了 K_SKIP、按了 K_PAUSE，
     * 跳出「沒有待答的 query」然後框還在。
     *
     * ⚠ 同步 throw 也要接（見 ht9045_dialog_host.js 檔頭那個坑：例外炸穿
     *   submit() 會讓 active.submitting 卡在 true，對話框永久鎖死）。 */
    if (global.HTDialogHost && typeof global.HTDialogHost.submitResponse === 'function') {
      var first;
      try {
        first = Promise.resolve(global.HTDialogHost.submitResponse(responseName + '.json', response));
      } catch (e) {
        first = Promise.reject(e);
      }
      return first.catch(function (err) {
        if (global.console) console.warn(
          '[dialog-bridge] HTDialogHost 這條不通，改走下一段傳輸：%s',
          (err && err.message) || err);
        return submitRest(response, responseName);
      });
    }
    return submitRest(response, responseName);
  }

  function submitRest(response, responseName) {
    if (global.HTJsonWriter && global.HTJsonWriter.ready()) return global.HTJsonWriter.write(responseName, response);
    if (global.chrome && global.chrome.webview && typeof global.chrome.webview.postMessage === 'function') {
      global.chrome.webview.postMessage({ type: 'HT_DIALOG_RESPONSE', file: responseName + '.json', response: response });
      return Promise.resolve('webview-posted');
    }
    global.postMessage({ type: 'HT_DIALOG_RESPONSE', file: responseName + '.json', response: response }, '*');
    /* //Steven 20260924（使用者指定）：debug 模式的**本機**解除。
     * -----------------------------------------------------------------------
     * file: 底下沒有 wb_serve、沒有 WebView2、沒有 HTJsonWriter，上面四段傳輸
     * 全部踩空 -> 框永遠關不掉，連 golden 的主路徑
     * 「K_SKIP / K_RETRY 選鍵 + START / PAUSE 確認」都沒辦法驗。
     *
     * ⚠ 兩道閘同時成立才放行，缺一不可：
     *     mode=debug                 —— 明示是除錯
     *     location.protocol==='file:' —— 實機是用 http 連 wb_serve，永遠不是 file:
     *   只靠 mode=debug 不夠：有人在實機上帶了 ?mode=debug，框會「看起來解除了」
     *   而 C++ 完全沒收到回應 —— 機台還停著，操作員以為好了。那是會出事的。
     * 回應不丟掉，留在 __HT_DEBUG_RESPONSES__ 供檢查。 */
    if (isFileDebug()) {
      (global.__HT_DEBUG_RESPONSES__ = global.__HT_DEBUG_RESPONSES__ || [])
        .push({ at: new Date().toISOString(), file: responseName + '.json', response: response });
      if (global.console) console.warn(
        '[dialog-bridge] DEBUG 本機關閉：%s 沒有送到 C++（file: + mode=debug）。' +
        '回應留在 window.__HT_DEBUG_RESPONSES__', responseName);
      return Promise.resolve('debug-local');
    }
    return Promise.reject(new Error('C++ dialog response transport is not connected'));
  }

  function isFileDebug() {
    return location.protocol === 'file:' &&
           /[?&]mode=debug/.test(location.search);
  }

  function submitCloseResponse(value) {
    /* AI(W906-S17D) 20261003 (St01)：以前這裡直接 reject（回報就丟了）。recent[] 一輪關兩個以上時，第二份一定撞上第一份 ⇒ 改成排隊。 */
    if (closeResponseSubmitting) {
      if (closeResponseBacklog.length < 16) closeResponseBacklog.push(value);
      return Promise.resolve('close-response-queued');
    }
    pendingCloseResponse = value;
    closeResponseSubmitting = true;
    return submit(value, closeChannel.response).then(function () {
      pendingCloseResponse = null;
      closeResponseSubmitting = false;
    }).catch(function (error) {
      closeResponseSubmitting = false;
      throw error;
    });
  }

  function complete(definition, closeRequest) {
    if (!active || active.submitting) return;
    var mask = Number(active.request.arguments && active.request.arguments.kCode) || 0;
    var selectedCode = Number(definition.code) || 0;
    if (!closeRequest && active.kind === 'alarm' && ((mask === 0 && selectedCode !== 0) ||
      (mask !== 0 && (selectedCode === 0 || (mask & selectedCode) === 0)))) return;
    active.submitting = true;
    var kind = active.kind;
    var dk = active.displayKind || kind;          // 狀態列要貼在實際顯示的那個視窗上

    /* Steven 20260922：web 端自行發起的不停機告警（channel 'nonstop-local'）
     * 沒有對應的 C++ request，**不可以**寫 response 檔 ——
     * 那會讓 C++ 收到一份它從來沒有發問過的回覆。按確認就只是關掉。 */
    if (active.request && active.request.channel === 'nonstop-local') {
      closeView();
      active = null;
      return;
    }

    status(dk, 'Submitting response...');
    var response = responseFor(definition, closeRequest);
    submit(response, channels[kind].response).then(function () {
      global.dispatchEvent(new CustomEvent('ht-dialog-response', { detail: response }));
      closeView();
      var closeResponse = closeResponseFor(response, closeRequest, true, null);
      closeResponse.normalResponse.file = channels[kind].response + '.json';
      active = null;
      return submitCloseResponse(closeResponse);
    }).catch(function (error) {
      if (!active) return;
      active.submitting = false;
      status(dk, error.message);
    });
  }

  function rejectClose(closeRequest, message) {
    return submitCloseResponse(closeResponseFor(null, closeRequest, !!active, {
      code: 'CLOSE_REQUEST_REJECTED', message: message
    })).catch(function () {});
  }

  /* AI(W906-S17D) 20261003 (St01)：Dialog-close-request 是單槽檔，100 ms 內 C++ 連關兩個框時舊版只看得到最後一個；
   *   而且只認 active／activeNS，還在佇列裡（沒顯示）的框收到關閉會被拒，之後照樣跳出來、C++ 早就不等了 ⇒ 關不掉。
   *   契約 1.3.1：檔頂層照舊是最新一筆，另加 recent[]（最多 8 筆、舊到新、含最新）；沒有 recent 的舊檔當成 [request]。 */
  function sameTarget(target, req) {
    return !!(target && req) && target.channel === req.channel && target.requestId === req.requestId &&
           Number(target.requestSeq) === Number(req.seq);
  }
  function dropQueued(target) {        // 排隊中（還沒顯示）的框：直接拿掉，永遠不畫
    var lists = [queueStop, queueNS];
    for (var l = 0; l < lists.length; l++) {
      for (var i = 0; i < lists[l].length; i++) {
        if (sameTarget(target, lists[l][i].request)) return lists[l].splice(i, 1)[0];
      }
    }
    return null;
  }

  function closeOne(request) {
      var target = request.target || {};
      /* Steven 20260922：實體 IO 按鈕**四種都能解除**（使用者說明），
       * 所以這條路也要認得不停機那一層，不能只看 active。
       * 先比對不停機的那一個；沒中再走原本的停機路徑。 */
      if (activeNS && activeNS.request && sameTarget(target, activeNS.request)) {
        closeNonStop();
        return submitCloseResponse(closeResponseFor(null, request, true, null)).catch(function () {});
      }
      if (active && sameTarget(target, active.request)) {
        var action = request.resolvedAction || {};
        if (!action.name || action.name === 'NONE') return rejectClose(request, 'C++ did not provide a resolved action');
        complete({ name: action.name, label: action.name, code: action.code }, request);
        return;
      }
      // AI(W906-S17D)：佇列裡的拿掉是同步的，不等 complete()（它是非同步的）
      if (dropQueued(target)) return submitCloseResponse(closeResponseFor(null, request, false, null)).catch(function () {});   // never shown: dialogWasOpen false
      if (!active) return rejectClose(request, 'No dialog is open');
      return rejectClose(request, 'Target does not match the active dialog');
  }

  function inspectClose() {
    if (closeChannel.loading) return;
    closeChannel.loading = true;
    loadFresh(closeChannel.request).then(function (request) {
      closeChannel.loading = false;
      var seq = Number(request && request.seq) || 0;
      if (!request || request.state !== 'pending' || !request.closeRequestId || seq <= closeChannel.lastSeq) return;
      // AI(W906-S17D)：recent[] 裡 seq > lastSeq 的每一筆依序處理；lastSeq 逐筆推進（中途出錯也不會重做已處理的）
      var recent = (request.recent && request.recent.length) ? request.recent.slice() : [request];
      if (!recent.some(function (e) { return e && Number(e.seq) === seq; })) recent.push(request);
      recent.sort(function (a, b) { return (Number(a && a.seq) || 0) - (Number(b && b.seq) || 0); });
      recent.forEach(function (entry) {
        var s = Number(entry && entry.seq) || 0;
        if (!entry || !entry.closeRequestId || s <= closeChannel.lastSeq) return;
        closeChannel.lastSeq = s;
        try { closeOne(entry); } catch (e) { console.warn('[dialog-bridge] close ' + entry.closeRequestId + ' 失敗：' + ((e && e.message) || e)); }
      });
    }).catch(function () { closeChannel.loading = false; });
  }

  function inspect(kind) {
    var channel = channels[kind];
    if (channel.loading) return;
    channel.loading = true;
    loadFresh(channel.request).then(function (request) {
      channel.loading = false;
      var seq = Number(request && request.seq) || 0;
      if (request && request.state === 'pending' && request.requestId && seq > channel.lastSeq) {
        /* Steven 20260922：先解析要開哪一頁，再決定擋不擋。
         * 舊版是 `if (!active)` 一律擋 —— 那會讓不停機的訊息排在停機告警後面，
         * 而不停機的本意就是「不要等」。停機類維持一次一個（golden 的 modal 語意）。 */
        if (channel.routing) return;
        channel.routing = true;
        resolveDisplay(kind, request).then(function (r) {
          channel.routing = false;
          if (r.info) request.nonStopInfo = Object.assign({}, r.info, request.nonStopInfo || {});
          /* ⚠ lastSeq 一定要在這裡推進，**不可以**因為畫面忙就跳過。
           * 舊版跳過它，等下一輪再讀同一個檔 —— 但那個檔只有一格，
           * C++ 覆寫它的瞬間舊的那一則就永久消失了。
           * 入列之後由 drain() 負責出貨，畫面忙不忙與收不收件無關。 */
          channel.lastSeq = seq;
          enqueue(kind, request, r.kind);
          drain();
        }, function () { channel.routing = false; });
      }
    }).catch(function () { channel.loading = false; });
  }

  installView();
  setInterval(function () {
    if (pendingCloseResponse && !closeResponseSubmitting) submitCloseResponse(pendingCloseResponse).catch(function () {});
    else if (!pendingCloseResponse && !closeResponseSubmitting && closeResponseBacklog.length) submitCloseResponse(closeResponseBacklog.shift()).catch(function () {});   // AI(W906-S17D) 20261003 (St01)
    inspectClose();
    drain();                     // 前一則關掉之後把佇列裡的下一則叫出來
    inspect('alarm');
    inspect('message');
  }, POLL_MS);
  global.HTDialogBridge = {
    /* 版本標記：`file:` 下 Edge 會快取 .js，改了沒生效時第一個要確認的就是它。
     * console 打 HTDialogBridge.build 對不上就是吃到舊快取 -> Ctrl+F5。 */
    build: '20261003-s17-close-recent',   // AI(W906-S17D) 20261003 (St01)：was '20260924b-dialoghost-falls-through'
    inspect: inspect, inspectClose: inspectClose, render: render,
    routeAndRender: routeAndRender, raiseNonStop: raiseNonStop,
    active: function () { return active; }, auth: function () { return auth; },
    activeNS: function () { return activeNS; },
    /* 佇列可觀測性。現場若懷疑「有 alarm 沒跳出來」，先看這裡的深度。 */
    queues: function () {
      function brief(q) {
        return q.map(function (i) {
          return { code: (i.request.arguments || {}).code || null,
                   requestId: i.request.requestId || null,
                   displayKind: i.displayKind, waitedMs: Date.now() - i.at };
        });
      }
      return { stop: brief(queueStop), nonStop: brief(queueNS),
               stopDepth: queueStop.length, nonStopDepth: queueNS.length };
    },
    enqueue: enqueue, drain: drain
  };
})(window);