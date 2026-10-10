# dialog host完整原文（1／1）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `06c430c157a3e39c14d33a5b29219925b2879854`，來源 `web/page/ht9045_dialog_host.js`。
整檔原文380行含所有metadata、註解與IIFE／anonymous callbacks；本頁payload 180行，context credit0。

<a id="log"></a> `function log(kind, text)`。
<a id="isacknowledge"></a> `function isAcknowledge(response)`。
<a id="optionof"></a> `function optionOf(response)`。
<a id="pickoption"></a> `function pickOption(want, offered)`。
<a id="submitresponse"></a> `function submitResponse(file, response)`。
<a id="watch"></a> `function watch()`。
<a id="verifyauth"></a> `function verifyAuth(verify)`。
<a id="sendauthcancel"></a> `function sendAuthCancel()`。

```javascript
<!-- preserved-content:start -->
/* ht9045_dialog_host.js -- dialog-bridge.js 的應答掛鉤（window.HTDialogHost）
 * ---------------------------------------------------------------------------
 * //Steven 20260922
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260922_Steven.md §20
 * 相關技能：ht9045-alarm-dismissal
 * ---------------------------------------------------------------------------
 * 載入順序：**必須在 dialog-bridge.js 之前**（background.html:304-305）。
 *   <script src="page/ht9045_dialog_host.js"></script>     <- 本檔
 *   <script src="page/dialog-bridge.js"></script>
 *
 * 對 ht9045_recipe_client.js 則**沒有**順序要求：
 *   * watch() 延到 DOMContentLoaded 才訂閱，那時同步 <script> 都載完了
 *   * submitResponse() 在**呼叫當下**才去看 global.HT9045Recipe
 *   實際的 background.html 就是 recipe_client 排在本檔後面（:311），能動。
 *
 * ---------------------------------------------------------------------------
 * 這個檔存在的理由
 * ---------------------------------------------------------------------------
 * `dialog-bridge.js` 的 `submit()` 有四段 fallback：
 *
 *   1. window.HTDialogHost.submitResponse      <- 沒有人設它
 *   2. window.HTJsonWriter                     <- 只有 debug，且每次開機要操作員授權資料夾
 *   3. chrome.webview.postMessage              <- 只有 WebView2 宿主
 *   4. postMessage(...,'*') 然後 Promise.reject <- 實際會落到這裡
 *
 * 落到第 4 段的現場行為是：**框跳得出來、按得下去，然後 submit() reject
 * —— 框不關、沒有東西被寫、C++ 永遠等。** 操作員看到的是一個按不掉的警報。
 *
 * 本檔補的就是第 1 段。（Jimmy 那邊 20260922 也各自做了一份，形狀一樣；
 * web 端是我們的責任範圍，所以這一支由我們維護，不是跟他要檔案。）
 *
 * ---------------------------------------------------------------------------
 * ⚠ 一個會讓對話框永久鎖死的坑（Jimmy 20260922 踩過，這裡照他的說明避開）
 * ---------------------------------------------------------------------------
 * `dialog-bridge.js` 寫的是 `Promise.resolve(f())` —— 它**先呼叫 f()**。
 * 若 `f` 同步丟例外，例外會直接炸穿 `submit()`，連 `.catch` 都進不去，
 * 結果 `active.submitting` 卡在 `true` ⇒ **對話框永遠不能再按**。
 *
 * ⇒ 本檔每一條路都自己 try/catch，**保證回一個 Promise，絕不同步 throw**。
 *
 * ---------------------------------------------------------------------------
 * 傳輸：wb_serve 的 WebSocket，不是檔案
 * ---------------------------------------------------------------------------
 * 請求方向（C++ -> 畫面）可以走檔案信箱，但**回應方向走 WebSocket**：
 * 量產 kiosk 若是 `file:` 協定，瀏覽器根本沒有寫本地檔的能力，
 * 上面那四段 fallback 就是為了這件事而全部踩空的。
 *
 * 指令形狀是 20260922 對活的 wb_serve 探測出來的，不是猜的：
 *
 *   query 訊框（伺服器 -> 畫面）
 *     {"type":"query","qid":1,"code":"WAR0000","kcode":3,
 *      "options":["RETRY","SKIP"],"at":"2026-09-22T14:11:46"}
 *
 *   答覆（畫面 -> 伺服器）
 *     cmd('modal.answer', {tag: "1", value: "RETRY"})
 *      ^ qid 在訊框裡是**數字**，在 tag 裡要**字串**
 *      ^ value 必須逐字出現在該訊框的 options[]，否則回 "not an offered option"
 *
 * ⚠ **modal 掛著時，其他每一個指令都回 `modal-pending`** —— 實測連 `sys.ping`
 *   都不行。佇列真的被擋住，只有 `modal.answer` 進得去。所以本檔在答覆之前
 *   不做任何別的往返（例如不要先去 acquire 權杖 —— 那會直接被 modal-pending 擋掉）。
 */
(function (global) {
  'use strict';

  /* 最近一個還沒答的 query 訊框。dialog-bridge 的 response 物件裡沒有 qid
   * （它的世界是檔案信箱），所以對應關係只能從這裡取。
   * 伺服器一次只會有一個 pending query（modal-pending 就是它在擋），
   * 所以單一變數是夠的，不需要表。 */
  var pendingQuery = null;
  var answered = {};          // qid -> true，避免重複答（第二次會回 "no query pending"）

  function log(kind, text) {
    try { console[kind === 'err' ? 'error' : 'warn']('[DialogHost] ' + text); } catch (e) {}
  }

  /* dialog-bridge 送來的 response 形狀（見 responseFor()）：
   *   {schemaVersion, channel, seq, requestId, requestSeq, state, accepted,
   *    closedBy, completedAt, durationMs, error, selectedAction, ...}
   * 我們要的是 selectedAction —— 它就是 KeyComp 的名字（RETRY / SKIP / ...）。 */
  /* AI(W906-DLGOBJ) 20260923: `selectedAction` 有**兩種形狀**，原本只處理了字串那種。
   *
   * 使用者 20260923 17:58 實測：MES0920 告警框點 RETRY -> PAUSE，畫面回
   *   選項 "[OBJECT OBJECT]" 不在伺服器提供的 options ["RETRY","CLEAN_OUT"] 裡
   * 伺服器那側是對的（kcode=5 = K_RETRY 0x1 | K_CLEAN_OUT 0x4，cmydef.cpp:337/339）。
   * 壞的是這裡：`String(物件)` === "[object Object]"，再 .toUpperCase() 就成了截圖那個字。
   *
   * 兩種形狀是 dialog-bridge.js 依通道產生的，而且**契約就是這樣定的**：
   *   :548  告警 show-error-message   selectedAction = { name, code }   <- 物件
   *   :552  訊息 show-my-message      selectedAction = definition.name  <- 字串
   * 佐證：JSON/Alarm-dialog-response.json 是 {"name":"NONE","code":0}，
   *       JSON/Message-dialog-response.json 是 "NONE"；
   *       JSON/Dialog-bridge-contract.json 的 returnMapping 明寫
   *       "response.selectedAction.code is returned unchanged as fNote->ReturnCode"。
   * ⇒ **不要去改 dialog-bridge.js 讓兩邊一致** —— 那是同事的檔，物件形狀是契約 v1.3.0
   *   明訂的（code 要原封不動回給 fNote->ReturnCode）。要適應的是我們這側。
   *
   * ⓘ "NONE" 當成「沒有選擇」而不是一個選項名：它是兩個樣板檔的閒置值，
   *   送出去只會被伺服器以 "not an offered option" 拒絕，而且錯誤訊息會指向
   *   kCode 遮罩（真正的原因卻是「根本沒選」）。在這裡擋掉才說得出實話。
   */
  /* AI(W906-J5-ACK) 20260930：「確認」＝ dialog-page.js alarmAction 在 kCode==0 送的 {name:'ACKNOWLEDGE', code:0}（或字串 ACKNOWLEDGE） */
  function isAcknowledge(response) {
    var sa = response && response.selectedAction;
    if (sa && typeof sa === 'object') return String(sa.name || '').toUpperCase() === 'ACKNOWLEDGE' || Number(sa.code) === 0;
    return String(sa || '').toUpperCase() === 'ACKNOWLEDGE';
  }

  function optionOf(response) {
    if (!response) return null;
    var sa = response.selectedAction;
    var name = null;
    if (sa && typeof sa === 'object') name = sa.name;      /* 告警通道 {name, code} */
    else if (sa)                      name = sa;           /* 訊息通道 "RETRY"     */
    if (!name) name = (response.action && response.action.name) || null;
    if (!name) return null;
    var up = String(name).toUpperCase();
    if (up === 'NONE') return null;
    return up;
  }

  /* 送出前先確認這個選項真的在伺服器給的 options[] 裡。
   * 不在的話送過去只會拿到 "not an offered option" —— 同樣關不掉框，
   * 但至少 console 會說出為什麼，而不是靜悄悄。 */
  function pickOption(want, offered) {
    if (!offered || !offered.length) return want;
    for (var i = 0; i < offered.length; i++) {
      if (String(offered[i]).toUpperCase() === want) return offered[i];
    }
    return null;
  }

  function submitResponse(file, response) {
    /* 這個函式**只能回 Promise**。上面檔頭那個坑就是它同步 throw 造成的。 */
    try {
      var R = global.HT9045Recipe;
      if (!R || typeof R.modalAnswer !== 'function') {
        return Promise.reject(new Error(
          'HT9045Recipe.modalAnswer 不存在 —— ht9045_recipe_client.js 沒載到，' +
          '或版本太舊（20260922 才加）。載入順序必須是 recipe_client -> dialog_host -> dialog-bridge。'));
      }

      /* 關閉通道的回應（Dialog-close-response）是**畫面回報關閉結果**用的，
       * 不是一個可以送給 modal.answer 的答案 —— 那條路的答案是 C++ 自己
       * 在 close-request 裡已經決定好的 resolvedAction。這裡吞掉即可，
       * 回 resolve 讓 bridge 的 submitCloseResponse() 正常收尾。 */
      if (String(file || '').indexOf('Dialog-close-response') === 0) {
        return Promise.resolve('close-response-noop');
      }

      /* AI(W906-SMM-IO) 20260925：C++ 由實體 IO 解除的框（RULINGS_20260925 第 42 條）。
       * C++ 已經自己結案、寫了 Dialog-close-request；dialog-bridge.js inspectClose() → complete(…, closeRequest)
       * 產生的這份回應 closedBy='external-io'。契約 dialogClose：「C++ detects and resolves IO; HTML only closes」
       * ⇒ 不再送回 C++（送了只會拿到 no query pending，框反而關不掉）。 */
      if (response && response.closedBy === 'external-io') {
        return Promise.resolve('closed-by-cpp-io');
      }

      /* AI(W906-SMM) 20260925：訊息通道（show-my-message）的回答。
       * golden ShowMyMessage／ShowMyMessageBox_YES_NO／ShowUnloaderTrayMessage 在 C++ 端寫
       * Message-dialog-request（tools/wb_serve.cpp 檔尾 W906MbShowMyMessage 等），**不發 WS query 訊框**，
       * 所以不能走下面 pendingQuery 那條。回答用 dialog.response，tag = requestId（"msg-<N>"），
       * value = 動作名：OK／PAUSE（pnlPause）、YES／NO（pnlYes／pnlNo）、ACKNOWLEDGE（不停機頁）。
       * 「no query pending」＝ C++ 那一則已經收尾（別的畫面先答了、或被新的一則取代）—— 當成已關，
       * 讓 dialog-bridge 把框收掉，否則會留一個永遠關不掉的框。 */
      if (response && response.channel === 'show-my-message') {
        if (typeof R.dialogResponse !== 'function') {
          return Promise.reject(new Error('HT9045Recipe.dialogResponse 不存在 —— ht9045_recipe_client.js 太舊'));
        }
        var rid = response.requestId;
        var sa = response.selectedAction;
        var act = String((sa && typeof sa === 'object') ? (sa.name || '') : (sa || '')).toUpperCase();
        if (!rid) return Promise.reject(new Error('訊息回應沒有 requestId，無法對應 C++ 那一則。file=' + file));
        if (!act || act === 'NONE') return Promise.reject(new Error('訊息回應沒有 selectedAction。file=' + file));
        return R.dialogResponse(rid, act).then(function () { return 'message-answered'; }, function (e) {
          var msg = (e && e.message) || '';
          if (/no query pending|bridge is read-only/i.test(msg)) return 'message-already-closed';
          throw e;
        });
      }

<!-- preserved-content:end -->
```
