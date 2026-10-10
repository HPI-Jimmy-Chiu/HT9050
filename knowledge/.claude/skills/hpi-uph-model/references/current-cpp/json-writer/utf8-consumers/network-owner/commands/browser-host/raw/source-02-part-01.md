# dialog host完整原文（2／1）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `06c430c157a3e39c14d33a5b29219925b2879854`，來源 `HT9011UC_Cpp_V3.33.906.0/web-overlay/page/ht9045_dialog_host.js`。
整檔原文127行含所有metadata、註解與IIFE／anonymous callbacks；本頁payload 127行，context credit0。

<a id="api"></a> `function api()`。
<a id="channelof"></a> `function channelOf(fileName)`。
<a id="encode"></a> `function encode(resp)`。
<a id="submitresponse"></a> `submitResponse: function (fileName, response)`。
<a id="verifyauth"></a> `verifyAuth: function (payload)`。

```javascript
<!-- preserved-content:start -->
/* ===========================================================================
 *  ht9045_dialog_host.js -- 把 Steven 的警報對話框接回 C++
 *
 *  AI(W906-Q30-8) 20260922。使用者裁決「依據建議甲，先優先處理」。
 *
 *  ## 這支存在的唯一理由
 *
 *  `dialog-bridge.js:319-330` 的 submit() 有四段 fallback：
 *      1. window.HTDialogHost.submitResponse   <- 這支就是在補它
 *      2. window.HTJsonWriter                  -- 只有 debug 模式，
 *                                                 且每次開機要操作員授權資料夾
 *      3. chrome.webview.postMessage           -- 只有 WebView2 宿主有
 *      4. postMessage(...,'*') 然後 reject
 *
 *  在量產的 release kiosk（file: 協定）下，1~3 全部不存在 ⇒ 落到 4 ⇒
 *  **框會跳出來、按得下去，然後 submit() reject：框不關、沒有檔被寫、
 *  C++ 永遠等。** 那正是「沒辦法解除導致 hang up」。
 *
 *  ⇒ 我們設第 1 段，把答案走**既有的 WebSocket** 送回 C++。
 *    不碰他的任何邏輯，只補他留的那個掛鉤。
 *
 *  ## ⚠⚠ submitResponse 必須自己 try/catch
 *
 *  他那邊是 `Promise.resolve(global.HTDialogHost.submitResponse(...))`。
 *  `Promise.resolve(f())` 會**先呼叫 f()** —— f 同步丟例外的話會直接炸穿
 *  submit()，連 `.catch` 都進不去（那一整串 .then().catch() 根本沒建立），
 *  結果是 `active.submitting` 卡在 true ⇒ **對話框永遠不能再按**。
 *  ⇒ 這支的每一條路都必須回 Promise，絕不同步 throw。
 *
 *  ## 回傳值要帶什麼
 *
 *  C++ 那側收 `dialog.response`，`tag` = requestId，`value` = 動作名。
 *  ⚠ value 允許 `"<ACTION>:<pressedButton>"` —— 因為
 *  `selectedAction.code` **分不出 Start 還是 Pause**（code 由先點的選擇鍵
 *  決定，Start/Pause 只是「送出」）。golden 的 fNote 靠 pressedButton 決定
 *  後續 SoftStart / SoftStop，所以那個資訊不能丟。
 *
 *  ## 部署
 *
 *  `background.html` 要在載 `page/dialog-bridge.js`（:222）**之前**多一行：
 *      <script src="page/ht9045_dialog_host.js"></script>
 *  ⚠ `D:\HT9045\web` 沒有版控，Steven 下一包會覆蓋 background.html ⇒
 *    這支與那一行都鏡在 `HT9011UC_Cpp_V3.33.906.0/web-overlay/`，
 *    而且本檔已加進 `tools/websync/sync_web.py` 的 OURS 保護名單。
 * =========================================================================== */
(function (global) {
  'use strict';

  // ⚠ 用 `HT9045Recipe`（指令通道）而**不是** `HT9045Tags`（唯讀的 tag 串流）。
  //   第一版我寫成 `HT9045Tags.send(...)` —— 那個方法**不存在**：
  //   `ht9045_recipe_client.js` 的 tagsApi 只匯出
  //   connect/get/has/all/subscribe/on/onEvent/status。
  //   送指令的 `cmd()` 是模組私有的（:178），所以我們照 Q27 `lotStart` 的前例
  //   在那支檔案加了兩個**窄方法** dialogResponse / dialogAuth。
  //   ⛔ 刻意不加通用的 `cmd(name, extra)` —— 那等於讓任何頁面送任意指令給機台。
  function api() {
    return global.HT9045Recipe || null;
  }

  // dialog-bridge 傳進來的檔名 -> 我們的通道名
  function channelOf(fileName) {
    if (/^Alarm-dialog-response/.test(fileName))   return 'alarm';
    if (/^Message-dialog-response/.test(fileName)) return 'message';
    if (/^Dialog-close-response/.test(fileName))   return 'close';
    if (/^Dialog-auth-verify/.test(fileName))      return 'auth';
    return '';
  }

  // response 物件 -> C++ 要的 "<ACTION>:<pressedButton>"
  function encode(resp) {
    var action = 'NONE';
    if (resp && resp.selectedAction) {
      // ⚠ alarm 的 selectedAction 是**物件** {name, code}；
      //   message 的是**字串**。同名不同型，共用一個 parser 會爆。
      action = (typeof resp.selectedAction === 'string')
             ? resp.selectedAction
             : (resp.selectedAction.name || 'NONE');
    }
    var pressed = (resp && resp.pressedButton) ? resp.pressedButton : '';
    return pressed ? (action + ':' + pressed) : action;
  }

  global.HTDialogHost = {
    // dialog-bridge.js:320 會這樣叫：submitResponse(name + '.json', response)
    submitResponse: function (fileName, response) {
      try {
        var a  = api();
        var ch = channelOf(String(fileName || ''));
        if (!a || typeof a.dialogResponse !== 'function') {
          // ⚠ 回 rejected promise，**不要 throw** —— 見檔頭。
          return Promise.reject(new Error(
            'HT9045: HT9045Recipe 還沒就緒（或版本太舊，沒有 dialogResponse）'));
        }
        if (!ch) {
          return Promise.reject(new Error('HT9045: 不認得的通道 ' + fileName));
        }
        if (ch === 'auth') {
          // 密碼那條：契約 dialogAuth.verifier 明文「C++ only，HTML 永不比對」。
          // 這裡只負責把使用者輸入原封轉送，不做任何判斷。
          return a.dialogAuth(String(response && response.authId || ''),
                              JSON.stringify(response || {}));
        }
        return a.dialogResponse(String(response && response.requestId || ''),
                                encode(response));
      } catch (e) {
        // 同步例外一律轉成 rejected promise。
        return Promise.reject(e);
      }
    },

    // 契約 dialogAuth：HTML 只收集輸入，驗證一律由 C++ 執行。
    verifyAuth: function (payload) {
      try {
        var a = api();
        if (!a || typeof a.dialogAuth !== 'function') {
          return Promise.reject(new Error('HT9045: HT9045Recipe 還沒就緒'));
        }
        // ⚠ authId 是**瀏覽器產生**的，C++ 必須原字串回填；
        //   填錯或留空 -> 15 秒逾時、登入層顯示 'C++ auth result timeout'。
        return a.dialogAuth(String(payload && payload.authId || ''),
                            JSON.stringify(payload || {}));
      } catch (e) {
        return Promise.reject(e);
      }
    }
  };
}(window));

<!-- preserved-content:end -->
```
