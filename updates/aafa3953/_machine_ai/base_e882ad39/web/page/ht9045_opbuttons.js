/* ht9045_opbuttons.js -- 把主控制面板的 START / PAUSE 接到底層。
 * AI(W906-OPBTN) 20260922 20:0x
 *
 * ---------------------------------------------------------------------------
 *  為什麼需要這個檔
 * ---------------------------------------------------------------------------
 *  20260922 實測：`Main.gbControlBtn.html:93-95` 是那 8 顆操作鈕的**唯一** handler，
 *  而它只做一件事：
 *      b.addEventListener('click', function(){ b.classList.toggle('on'); });
 *  也就是純視覺的 class 切換，模擬 BCB6 的 TBtnPanel SetPanelStatus。
 *
 *  結果：全樹 722 個檔對 `HT9045Recipe.start` 的呼叫點是 **0**
 *  （`.start(` 這個形狀在整棵 web 樹出現 0 次，別名與計算式存取也各自掃過都是 0）。
 *  ⇒ 按 START **從來沒有送出任何指令**，C++ 那端一個 frame 都沒收到，
 *    所以 `StartFromWeb` 的中斷點永遠不會觸發。那不是 gdb 的問題，也不是 -g 的問題。
 *
 *  `HT9045Recipe.start()` / `.pause()` 本身是完整可用的
 *  （`ht9045_recipe_client.js:435` / `:447`），只是從來沒有人呼叫它們。本檔就是呼叫者。
 *
 * ---------------------------------------------------------------------------
 *  為什麼是獨立檔，而不是改 Main.gbControlBtn.html 的那三行
 * ---------------------------------------------------------------------------
 *  `Main.gbControlBtn.html` 是 Steven 的檔（由 .dfm 轉出）。他每次交付都會覆蓋它。
 *  邏輯放在這裡，他的交付最多只會弄掉一行 `<script src=...>`，而那一行的遺失
 *  是**看得見**的（本檔會在狀態列說自己沒載入）。把邏輯寫進他的檔則會靜默消失 ——
 *  20260922 09:44 那次重佈署就這樣弄丟了 `ht9045_recipe_client.js` 的 script 標籤，
 *  而 reapply_overlay.py 的探針只認 "ht9045_lotstart.js"，命中就短路成 ok，
 *  所以 --check 三項全綠、exit 0，完全看不到缺口。
 *  同樣的理由見 `ht9045_lotstart.js` 的檔頭。
 *
 * ---------------------------------------------------------------------------
 *  範圍：只有 START 與 PAUSE
 * ---------------------------------------------------------------------------
 *  `HT9045Recipe` 只匯出 10 個方法（configure/list/read/write/preview/release/
 *  status/start/pause/lotStart），**沒有通用的 cmd**，所以外面送不了任意指令。
 *  HOME / RESET / ONE CYCLE / CLEAN OUT / TRAY FEED / ALARM RESET 這 6 顆
 *  目前沒有對應的 API —— 本檔**不假裝**它們能動，按下去會明講「尚未接線」。
 *  這是刻意的：一顆看起來會動、其實只換顏色的鈕，比一顆誠實說自己沒接線的鈕危險。
 *
 *  ⚠ 不移除原本那個 class toggle。它是視覺回饋，保留；本檔的監聽器加在它之上。
 */
(function () {
  'use strict';

  var STATUS_ID = 'ht9045OpMsg';

  /* 有 API 的：id -> {label, call} */
  var WIRED = {
    BtnStart: {
      label: 'START',
      call: function () { return window.HT9045Recipe.start('web'); }
    },
    BtnPause: {
      label: 'PAUSE',
      call: function () { return window.HT9045Recipe.pause('web'); }
    }
  };

  /* 沒有 API 的，按下去要誠實講 */
  var UNWIRED = ['BtnHome', 'BtnReset', 'BtnOneCycle', 'BtnCleanOut',
                 'BtnTrayEnd', 'BtnAlarmReset'];

  function $(id) { return document.getElementById(id); }

  /* AI(W906-OPBTN-ACK) 20260922: 'warn' 本來沒有自己的顏色，掉進 '#345'，
   * 跟 info 長得一模一樣。而修好 catch 之後 'warn' 正是最常出現的那一類
   * （底層**收到並回答了**，只是答案是「不行」），它必須跟
   * 'err'（根本沒送到 / 沒回答）在視覺上分得開。 */
  function say(msg, kind) {
    var el = $(STATUS_ID);
    if (!el) return;
    el.textContent = msg;
    el.style.color = (kind === 'err') ? '#a00'
                   : (kind === 'ok') ? '#070'
                   : (kind === 'warn') ? '#b26b00'
                   : '#345';
  }

  function buildStatusLine() {
    if ($(STATUS_ID)) return;
    var host = $('gbControlBtn') || document.querySelector('.cbCol');
    if (!host) return;
    var el = document.createElement('div');
    el.id = STATUS_ID;
    el.style.cssText = "font:11px 'Microsoft JhengHei',sans-serif;color:#345;" +
                       'min-height:14px;word-break:break-all;padding:2px 0;';
    if (host.parentNode) host.parentNode.insertBefore(el, host.nextSibling);
    else host.appendChild(el);
  }

  function apiReady() {
    return !!(window.HT9045Recipe &&
              typeof window.HT9045Recipe.start === 'function' &&
              typeof window.HT9045Recipe.pause === 'function');
  }

  /* ack 攤平：CompleteCommand(wb_serve.cpp:3149-3151) 把物件併進 ack 頂層，
   * 但舊形狀是放在 detail 底下。兩邊都看，頂層優先 —— 與 ht9045_lotstart.js
   * 同樣的處置，理由見該檔（探針第一版只看 detail，誤報了 2 個 FAIL）。 */
  function flatten(r) {
    var d = r || {};
    if (d.detail && typeof d.detail === 'object') {
      for (var k in d.detail) { if (!(k in d)) { d[k] = d.detail[k]; } }
    }
    return d;
  }

  function describe(d) {
    var bits = [];
    if ('accepted' in d)    { bits.push('accepted=' + d.accepted); }
    /* AI(W906-OPBTN-ACK) 20260922: softStart 本來漏了。
     * start.run 的 ack 送三個欄位（wb_serve.cpp:3312-3314 accepted/softStart/
     * systemStart），而這裡只認 softStop —— 也就是 START 的**關鍵欄位**永遠不會
     * 被格式化出來，只有 PAUSE 的那一個會。 */
    if ('softStart' in d)   { bits.push('softStart=' + d.softStart); }
    if ('softStop' in d)    { bits.push('softStop=' + d.softStop); }
    if ('systemStart' in d) { bits.push('systemStart=' + d.systemStart); }
    if ('reason' in d && d.reason) { bits.push('reason=' + d.reason); }
    return bits.length ? bits.join('  ') : JSON.stringify(d).slice(0, 200);
  }

  /* AI(W906-OPBTN-ACK) 20260922: 從一個 reject 裡把「底層的答案」挖出來。
   *
   *  為什麼需要它 —— ack 的兩種形狀是**不對稱**的
   *  （WebBridgeServer.cpp:1262 AckJson）：
   *
   *    ok=true  → 明細物件**去掉大括號併進 ack 頂層**（:1297-1298），
   *               所以 then 分支拿到的 r 直接就有 accepted/softStart/...
   *    ok=false → 明細物件被 **QuoteString 成一個字串**塞進 "error"（:1271），
   *               而 ht9045_recipe_client.js:169 對 ok=false 一律
   *               `p.reject(new Error(m.error))`。
   *
   *  ⇒ 失敗時的明細活在 `e.message` 裡，是一串 **JSON 文字**，不是物件。
   *    原本的 catch 只做字串相加，於是整包 JSON 被原樣印成
   *    「START 被拒絕或逾時：Error: {"accepted":false,...}」，
   *    而 then 分支裡那段友善說明是**死碼**（ok=false 永遠走不到 then）。
   *    20260922 使用者就是撞到這個：真正的原因 systemStart=true 藏在噪音裡。
   *
   *  ⓘ 只解析、不改變流程：解不出來就回 null，照舊當純文字錯誤顯示。 */
  function detailOf(e) {
    var s = (e && typeof e.message === 'string') ? e.message : String(e || '');
    s = s.replace(/^\s*Error:\s*/, '');            // new Error(...) 的 toString 前綴
    if (s.charAt(0) !== '{') return null;
    var d;
    try { d = JSON.parse(s); } catch (ignored) { return null; }
    return (d && typeof d === 'object') ? d : null;
  }

  /* AI(W906-OPBTN-ACK) 20260922: 把 accepted=false 翻成「下一步該做什麼」。
   *
   *  accepted=false 至少有三種完全不同的意思，而原本的文案把它們全押在
   *  「機台尚未歸零」一種上。最常見的那一種其實是機台**已經在跑**：
   *
   *    WebStart.cpp:3128（golden main.cpp:5871）
   *        if (SystemStart) { iStartIn = 0; return false; }
   *
   *    而 {softStart:false, systemStart:true} 正是「已啟動且運轉中」的穩定組合 ——
   *    ckernel.cpp:816 收到 SoftStart，:837 把它清掉，:1015 設 SystemStart=true。
   *    要再啟動必須先 PAUSE：ckernel.cpp:1046 的暫停檢查才會把 SystemStart 清回 false。 */
  function hint(label, d) {
    if (d.accepted === true) { return ''; }
    if (label === 'START') {
      if (d.systemStart === true) {
        return '　→ 機台已在運轉中（SystemStart=1），golden 不允許重複啟動' +
               '（WebStart.cpp:3128）。要重新啟動請先按 PAUSE。';
      }
      return '　→ accepted=false 常見原因是機台尚未歸零：它正在去歸零' +
             '（Home by Start），歸完再按一次 START。';
    }
    if (label === 'PAUSE') {
      if (d.systemStart !== true && d.softStop !== true) {
        return '　→ 機台本來就不在運轉狀態，沒有東西可以暫停。';
      }
      return '　→ 底層拒絕了暫停；請看 wb_serve 主控台的 pause.run 那一行。';
    }
    return '';
  }

  function wireOne(id, spec) {
    var b = $(id);
    if (!b) return false;
    b.addEventListener('click', function () {
      if (!apiReady()) {
        say(spec.label + ' 未接線：HT9045Recipe.start/pause 不存在。' +
            '多半是本頁沒載入 ht9045_recipe_client.js（見 INBOX N4）', 'err');
        return;
      }
      say(spec.label + ' 送出中 …');
      var p;
      try { p = spec.call(); }
      catch (e) { say(spec.label + ' 送出失敗：' + e, 'err'); return; }

      /* ⚠ accepted=false **不代表失敗**。golden 的 TfMain::Start 在機台還沒
       * 歸零時會去觸發 Home("Home by Start") 然後回 false
       * （WebStart.cpp:3594 ← golden main.cpp:6166）；已經在跑時則直接回 false
       * （WebStart.cpp:3128）。兩者都是正常行為，不是錯誤。
       *
       * AI(W906-OPBTN-ACK) 20260922: then 與 catch 走**同一條**呈現路徑。
       *   在 client 對 ok=false 一律 reject 的前提下（ht9045_recipe_client.js:169）
       *   實際命中的是 catch；then 留著是因為 ok=true 仍然走它，
       *   而且 client 若哪天改成 resolve，這裡不必再改一次。 */
      function report(d) {
        var ok = (d.accepted === true);
        say(spec.label + ' ack: ' + describe(d) + hint(spec.label, d),
            ok ? 'ok' : 'warn');
      }

      p.then(function (r) {
        report(flatten(r));
      }).catch(function (e) {
        var d = detailOf(e);
        if (d) { report(d); return; }
        /* 解不出明細 = 真的沒收到答案（socket 斷、逾時、非 JSON 的拒絕字串）。
         * 這一類才配紅色。 */
        say(spec.label + ' 被拒絕或逾時：' + e, 'err');
      });
    });
    return true;
  }

  function wireUnwired() {
    UNWIRED.forEach(function (id) {
      var b = $(id);
      if (!b) return;
      b.addEventListener('click', function () {
        say(b.textContent.trim() + '：尚未接線（HT9045Recipe 沒有對應的方法）。' +
            '目前只有 START 與 PAUSE 會真的送到底層。', 'warn');
      });
    });
  }

  function init() {
    buildStatusLine();
    var n = 0;
    for (var id in WIRED) { if (wireOne(id, WIRED[id])) { n++; } }
    wireUnwired();
    if (n === 0) {
      say('ht9045_opbuttons.js 已載入，但找不到 BtnStart / BtnPause', 'err');
    } else {
      say('START / PAUSE 已接到底層' +
          (apiReady() ? '' : '（⚠ 但 HT9045Recipe 尚未就緒，請確認 ht9045_recipe_client.js 有載入）'),
          apiReady() ? 'ok' : 'err');
    }
  }

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    init();
  }
})();
