/* ht9045_main_softkeys.js -- main.html「面板按鍵（軟體）」：實體前面板按鍵的螢幕版。
 * ---------------------------------------------------------------------------
 * AI(W906-SOFTKEY) 20261003: EastSun「你可以幫我圖片上 紅框內做出按鈕嗎 因為我現在實體按鈕沒辦法通訊 需要在軟體上 按按鈕做動」。
 *   機台 web 0096 / 0099 SOFTKEY-NOTOKEN / 0100 SOFTKEY-DELEGATE；ST02-P2 20261006 (St02-E) 搬進 main，**沒有緊急停止鈕**
 *   （機台 web 0097 / 0100 的 E-STOP 部分不收：RULINGS_20261005 第 17 條，SOFT E-STOP 只留在機台）。
 *   每顆送 WS panel.key（value＝按鍵名），C++ 把它當成「按了一下實體面板鍵」：WebMainScanKey.cpp W906_SoftPanelKeyPush 設 golden
 *   自己的軟體按鍵旗標 bAse*，ScanPannelKey 在每顆鍵的判斷裡讀它（跟實體鍵同一段，副作用照跑），交給第一個讀按鍵的地方
 *   （golden TfMain::ScanKey、回原點中的 PAUSE、告警／訊息框等待中的 RETRY／SKIP／PAUSE／ALARM RESET），要不要動作全照原版判斷。
 *   3 秒內沒被讀走（例如有對話框擋住主流程的按鍵掃描）C++ 會丟掉，不會晚點才突然動作。
 *   不加確認視窗（EastSun：按下就送）。結果只寫在面板自己的那一行，不蓋畫面。
 * 權杖：panel.key 在伺服器端免權杖（WebBridgeServer.cpp，同實體鍵），所以馬上送、不先 keepAlive（0099：那一步卡住時什麼都沒送出）。
 * 點擊：一個 document 層的 capture 監聽（0100：那一區被重畫後，載入時綁在按鈕上的監聽就失效，一下都沒送到 wb_serve）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var R = window.HT9045Recipe;
  if (window.__htMainSoftKeys) return;
  window.__htMainSoftKeys = true;

  var COOL_MS = 400, BUSY = {}, COOL = {};
  function $(id) { return document.getElementById(id); }
  function msg(t, kind) {
    var el = $('softKeysMsg');
    if (!el) return;
    el.textContent = t;
    el.style.color = kind === 'err' ? '#c00' : (kind === 'warn' ? '#b60' : '#060');
  }
  function detailOf(e) {
    var s = String((e && e.message) || e || '');
    try { var j = JSON.parse(s); return j.detail || j.guard || s; } catch (x) { return s; }
  }

  function press(key, label) {
    if (!R || !R.rawCmd) { msg(label + '：連線元件（HT9045Recipe）沒載入，沒有送出', 'err'); return; }
    var now = Date.now();
    if (BUSY[key]) { msg(label + '：上一下還在處理，這一下略過', 'warn'); return; }
    if (COOL[key] && now < COOL[key]) return;                       // 400 ms 內的第二下（雙擊）
    BUSY[key] = true;
    // AI(W906-SOFTKEY-NOTOKEN) 20261003 (machine web 0099): a panel key is a press -- sent at once (the server is token-exempt for it),
    //   with immediate feedback and a 3 s no-reply note.
    msg(label + '：送出中…', 'warn');
    var answered = false;
    var to = setTimeout(function () { if (!answered) { BUSY[key] = false; msg(label + '：3 秒沒有回覆（wb_serve 有在跑嗎？）', 'err'); } }, 3000);
    R.rawCmd('panel.key', { value: key })
      .then(function (m) {
        var d = (m && (m.detail || m.message)) || '';
        msg(label + '：已送出' + (d ? '（' + d + '）' : ''), 'ok');
      }, function (e) {
        msg(label + '：沒有送出 —— ' + detailOf(e), 'err');
      })
      .then(function () {
        answered = true; clearTimeout(to);
        BUSY[key] = false;
        COOL[key] = Date.now() + COOL_MS;
      });
  }

  function bind() {
    var box = $('softKeys');
    if (!box) return;
    var btns = box.querySelectorAll('[data-key]');
    for (var i = 0; i < btns.length; i++) {
      (function (b) {
        if (b.__softkey) return;
        b.__softkey = true;
        b.setAttribute('data-st01ev', '1');                          // ht9045_opbuttons.js：已接線
        b.addEventListener('click', function () { press(b.getAttribute('data-key'), b.textContent.trim()); });
      })(btns[i]);
    }
    if (!R || !R.rawCmd) msg('連線元件（HT9045Recipe）沒載入，按鍵不會送出', 'err');
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bind, { once: true });
  else bind();
  // AI(W906-SOFTKEY-DELEGATE) 20261003 (machine web 0100, without its E-STOP selector): one document-level capture listener answers
  //   every soft key however often that area is rebuilt; it stops the event, so a button that still has its own listener is not sent twice.
  document.addEventListener('click', function (ev) {
    var t = ev.target && ev.target.closest ? ev.target.closest('#softKeys [data-key]') : null;
    if (!t) return;
    ev.stopPropagation();
    press(t.getAttribute('data-key'), t.textContent.trim());
  }, true);
  window.HT9045MainSoftKeys = { press: press };
})();
