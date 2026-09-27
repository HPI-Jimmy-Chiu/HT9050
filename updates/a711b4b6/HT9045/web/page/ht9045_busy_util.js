/* ht9045_busy_util.js -- 防連點：伺服器 busy: 回覆的共用判斷（St01 頁面共用）
 * ---------------------------------------------------------------------------
 * AI(W906-CMDGUARD-UI) 20260926（Steven 團隊，S107 防連點的前端第二道）
 * Steven 20260926 規則：「全部的按鈕事件要小心使用者短時間連點，類似滑鼠 double click，可能要進行阻斷。」
 *
 * 主防線在伺服器：wb_serve 的指令 guard（WebCmdGuard.cpp，env W906_CMDGUARD_MS，預設 400 ms）——
 *   同一個「指令＋tag＋value」在上一條完成後 400 ms 內又到（或上一條還在跑），就不執行，
 *   ack 回 ok:false、error 以 "busy:" 開頭（例 "busy: same command in progress or just done (towerlight.op, 120 ms ago)"）。
 *   意思是「第一下還在跑或剛做完」，**不是失敗**：頁面不要在狀態列顯示成失敗、不要跳錯誤框。
 *
 * 用法（各頁在「呼叫當下」才看 window.HT9045Busy；這支沒載入時各頁照舊當成一般錯誤，不會壞）：
 *   HT9045Busy.is(x)     x 可以是 Error（recipe_client 把 ack.error 包成 Error 的 message）、字串、
 *                        原始 ack 物件（{ok:false, error}）、或頁面 parseErr 自己包的 {guard:'transport', detail}
 *   HT9045Busy.coolMs()  頁面自己的冷卻時間（完成後幾毫秒內的點擊直接忽略；跟伺服器預設同為 400 ms）
 *   HT9045Busy.NOTE      頁面原本已經顯示「…中」的，換成這句（一般顏色，不是錯誤色），免得「…中」一直掛著
 * 載入：<script src="ht9045_busy_util.js"></script>，放在頁面接線檔之前或之後都可以（呼叫當下才取）。
 * ⚠ 不要搬進 ht9045_recipe_client.js／ht9045_wire_engine.js（Jimmy 登記的引擎檔）。
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';

  var RE = /^busy:/;                  // 伺服器字首固定（guard 設計：busy 字首要固定，前端靠它安靜處理）

  function text(x) {
    if (x === null || x === undefined) return '';
    if (typeof x === 'string') return x;
    if (typeof x !== 'object') return String(x);
    if (typeof x.message === 'string' && x.message) return x.message;   // Error
    if (typeof x.error === 'string' && x.error) return x.error;         // 原始 ack
    if (typeof x.detail === 'string') return x.detail;                  // {guard:'transport', detail}
    return '';
  }
  function is(x) { return RE.test(text(x)); }

  global.HT9045Busy = {
    is: is,
    text: text,
    coolMs: function () { return 400; },
    NOTE: '同一個指令剛送過（上一下還在跑或剛做完），這一下略過'
  };
})(window);
