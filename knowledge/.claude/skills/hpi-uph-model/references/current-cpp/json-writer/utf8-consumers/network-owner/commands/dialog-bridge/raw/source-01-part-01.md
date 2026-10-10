# Dialog bridge完整原文（1／5）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`，來源 `web/page/dialog-bridge.js`。
全文835行保留metadata、原裁決註解、API object及callbacks；本頁payload 180行，context credit0。

<a id="sendrequest"></a> `function sendRequest(kind)`。
<a id="onpagemessage"></a> `function onPageMessage(e)`。
<a id="submit"></a> `function submit(response, responseName)`。
<a id="submitrest"></a> `function submitRest(response, responseName)`。
<a id="submitcloseresponse"></a> `function submitCloseResponse(value)`。
<a id="complete"></a> `function complete(definition, closeRequest)`。
<a id="rejectclose"></a> `function rejectClose(closeRequest, message)`。

```javascript
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
