/* ht9045_contactct_wire.js -- Data.ContactCT.html（golden TfContactCT，cContactCT.cpp V912）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：wb_serve WS 指令 contactct.get → cContactCT.cpp 檔尾 W906_ContactCTJson。
 *   開頁：contactct.get（不帶 value）＝ golden 開窗 FormShow（ItemIndex=3）→ 整格重畫
 *   切選項：contactct.get value={"yieldType":n} ＝ golden 點 rgYieldType 第 n 項（rgYieldTypeClick）→ 整格重畫
 *   回應：{rgYieldType:{items,itemIndex,columns}, rows, cols, colWidths, rowHeights,
 *          cells[row][col]:{text,bg,fg,drawn}, palette:{clXxx:"#RRGGBB"}, font:{bold,size}, initialOK, errors[],
 *          enabled:{btClearCount,btYieldChart}（AI(W906-FRW-S115) 20260927 起）}
 *   格子文字與顏色全部照回應畫（bg／fg 是 golden 的 clXxx 色名，RGB 由回應的 palette 對照）。
 * Count Clear（golden btClearCountClick :944-1069）與 Yield Chart（btYieldChartClick :1071-1075）這次不接。
 *
 * AI(W906-FRW-S115) 20260927 即時更新（RULINGS_20260926 S115；S124＝B：唯讀 *.get 免權杖）
 *   golden 這張表沒有自己的 Timer，是別的程式叫 fContactCT->sgYield->Refresh() 重畫：
 *     每測完一次（ProcessCount 結尾 atester_ProcessCount.cpp:1999，大約一個 Index 週期一次）、
 *     各種清除之後（cSortCT.cpp:694／:866、Alarm4Yield :1336／:1363／:1403、main.cpp:15516、Timer10Timer :35349、
 *     uYieldMonitoring.cpp:3983／:5648、SECS uHGemHT9045.cpp:1924／:1952）。
 *   另外 golden TfMain::Timer1Timer（main.cpp:3222-3233，Interval 30 ms）一直寫 btClearCount->Enabled = !SystemStart。
 *   網頁照 Steven S124 的話（「可能一秒鐘就更新一次」）：
 *     tick   每 1000 ms 送一次 contactct.get value={"yieldType":<頁面正在看的那一項>}。它等於 C++ 的 ItemIndex
 *            ⇒ C++ 不觸發 OnClick、只重畫＝golden sgYield->Refresh()。回應跟上一次一樣就不動 DOM。
 *            btClearCount 的 disabled 跟著回應的 enabled 走（舊 C++ 沒帶 enabled 就不動）。
 *     權杖   contactct.get 在 WebBridgeServer.cpp:1448 免權杖（St02 9d790ff2），這一頁不再拿權杖。
 *            （原本開頁 control.acquire 之後沒有 release，權杖會被這一頁佔到伺服器 10 分鐘閒置才收回。）
 *            舊的 wb_serve（沒有那一行）回 not-operator：tick 這一拍略過；開頁／點選項改走 acquire → get → release
 *            （持有只有幾毫秒，同 Data.Observer）。
 *     防連點 contactct.get 在 WebCmdGuard 名稱級白名單（WebCmdGuard.cpp:89），伺服器不會回 busy:。頁面一次只送一條：
 *            tick 撞上送出中就略過；使用者點的選項不能被吃掉 ⇒ 排隊（只留最後一個），送出中那一條的回應已過時、不畫。
 *     沒人看 分頁在背景（document.hidden）或外框被縮小／關掉（background.html 把外框設 display:none，iframe 還在）
 *            就不拍；golden 視窗沒顯示時 Refresh 也畫不出東西，再顯示時畫的就是當下的值。
 *   window.__contactct：data（畫面上正在顯示的那一份回應）、loads（開頁＋點選項送出的次數）、ticks、same（tick 回應跟上一次
 *     一樣）、renders、skipped（略過的拍）、tokenFallback、error（開頁／點選）、tickError —— 探針
 *     tools/webprobe/data_contactct_probe.py 與 data_contactct_live_probe.py 讀這些。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var TICK_MS = 1000;
  var D = null;            // 畫面上正在顯示的那一份回應（探針也讀這個）
  var sel = null;          // 頁面正在看的 rgYieldType 項次（開頁回應之後才有）
  var inflight = false;    // 一次只送一條 contactct.get
  var queued = null;       // 送出中時使用者點的選項：{yt, kind}，只留最後一個
  var sigRadio = '', sigGrid = '', lastWarn = null, errShown = false, oldServerNoted = false;
  var st = window.__contactct = { data: null, loads: 0, ticks: 0, same: 0, renders: 0, skipped: 0,
                                  tokenFallback: 0, error: null, tickError: null };

  function $(id) { return document.getElementById(id); }
  function say(msg, colour) {
    var el = $('ctStatus');
    if (el) { el.textContent = msg || ''; el.style.color = colour || ''; }
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function colour(name) {
    if (D && D.palette && D.palette[name]) return D.palette[name];
    return '';
  }

  // contactct.get。quiet（tick）＝舊伺服器要權杖時不去拿，把 not-operator 丟回去讓這一拍略過。
  function fetchCT(yt, quiet) {
    var extra = {};
    if (yt !== undefined && yt !== null) extra.value = JSON.stringify({ yieldType: yt });
    return raw('contactct.get', extra).then(unwrap, function (e) {
      if (quiet || !e || e.message !== 'not-operator') throw e;
      // 舊的 wb_serve（沒有 S124 免權杖那一行）：acquire → get → release，持有只有幾毫秒
      st.tokenFallback++;
      var held = false;
      function release() { return held ? raw('control.release').catch(function () {}) : Promise.resolve(); }
      return raw('control.acquire').then(function () {
        held = true;
        return raw('contactct.get', extra).then(unwrap);
      }, function (e2) {
        throw new Error('這一版 wb_serve 的 contactct.get 還要權杖，而權杖在別的頁面（' + e2.message + '）');
      }).then(function (d) { return release().then(function () { return d; }); },
              function (e3) { return release().then(function () { throw e3; }); });
    });
  }

  function renderRadio() {
    var box = $('rgYieldType');
    var rg = D.rgYieldType || { items: [], itemIndex: -1, columns: 1 };
    box.style.gridTemplateColumns = 'repeat(' + Math.max(1, rg.columns || 1) + ', 1fr)';
    box.innerHTML = '';
    // VCL TRadioGroup：Columns>1 時選項是「先直後橫」排（每欄 ceil(n/columns) 個）
    var n = rg.items.length, cols = Math.max(1, rg.columns || 1), per = Math.ceil(n / cols) || 1;
    box.style.gridAutoFlow = 'column';
    box.style.gridTemplateRows = 'repeat(' + per + ', auto)';
    rg.items.forEach(function (text, i) {
      var lab = document.createElement('label');
      var r = document.createElement('input');
      r.type = 'radio'; r.name = 'rgYieldType'; r.value = String(i);
      r.checked = (i === rg.itemIndex);
      r.addEventListener('change', function () { if (r.checked) { sel = i; send(i, 'click'); } });
      lab.appendChild(r);
      lab.appendChild(document.createTextNode(' ' + text));
      box.appendChild(lab);
    });
  }

  function renderGrid() {
    var t = $('sgYield');
    var html = '';
    var cw = D.colWidths || [];
    html += '<colgroup>';
    for (var c = 0; c < D.cols; c++) html += '<col style="width:' + (cw[c] || 64) + 'px">';
    html += '</colgroup>';
    for (var r = 0; r < D.cells.length; r++) {
      var h = (D.rowHeights && D.rowHeights[r]) ? D.rowHeights[r] : 16;
      html += '<tr style="height:' + h + 'px">';
      var row = D.cells[r];
      for (var k = 0; k < row.length; k++) {
        var cell = row[k];
        var sty = 'background:' + colour(cell.bg) + ';color:' + colour(cell.fg) + ';';
        if (cell.drawn && D.font) {
          if (D.font.bold) sty += 'font-weight:bold;';
          if (D.font.size) sty += 'font-size:' + Math.round(D.font.size * 96 / 72) + 'px;';
        }
        html += '<td class="' + (cell.drawn ? 'drawn' : '') + '" data-r="' + r + '" data-c="' + k +
                '" data-bg="' + cell.bg + '" data-fg="' + cell.fg + '" style="' + sty + '">' +
                String(cell.text).replace(/&/g, '&amp;').replace(/</g, '&lt;') + '</td>';
      }
      html += '</tr>';
    }
    t.innerHTML = html;
  }

  // golden TfMain::Timer1Timer main.cpp:3222-3233：運轉中（SystemStart）btClearCount->Enabled=false，停下來 true
  function renderButtons(d) {
    var en = d.enabled || {};
    ['btClearCount', 'btYieldChart'].forEach(function (id) {
      var b = $(id);
      if (b && typeof en[id] === 'boolean') b.disabled = !en[id];
    });
  }

  function warnText(d) {
    var msg = '';
    if (!d.initialOK) msg = '⚠ C++ InitialOK=false：golden sgYieldDrawCell 開頭就 return，格子不會畫（顯示的是 VCL 預設格）';
    if (d.errors && d.errors.length) msg += (msg ? '；' : '') + '⚠ ' + d.errors.length + ' 格繪製例外：' + d.errors[0].error;
    return msg;
  }

  // 開頁／點選項：整張重畫（同原本）。tick：只重畫有變的那一塊，全部一樣就不動 DOM
  // （每秒整張 innerHTML 會把正在按下去的 radio 換掉，那一下點選就丟了）。
  function render(d, kind) {
    var sr = JSON.stringify(d.rgYieldType || null);
    var sg = JSON.stringify([d.cols, d.colWidths, d.rowHeights, d.font, d.palette, d.cells]);
    var changed = false;
    if (kind !== 'tick' || sr !== sigRadio) { renderRadio(); changed = true; }
    if (kind !== 'tick' || sg !== sigGrid) { renderGrid(); changed = true; }
    sigRadio = sr; sigGrid = sg;
    renderButtons(d);
    if (changed) st.renders++; else st.same++;
    var w = warnText(d);
    // tick 不蓋掉「尚未接」之類的提示：只有警告內容變了、或剛從錯誤恢復才改狀態列
    if (kind !== 'tick' || w !== lastWarn || errShown) { say(w, w ? '#c60' : ''); errShown = false; }
    lastWarn = w;
  }

  function fail(e, kind) {
    var m = (e && e.message) || String(e);
    if (kind === 'tick') {
      if (m === 'not-operator') {                // 舊 wb_serve：這一拍略過（同 Data.Observer 拿不到權杖時）
        st.skipped++;
        if (!oldServerNoted) {
          oldServerNoted = true; errShown = true;
          say('⚠ 這一版 wb_serve 的 contactct.get 還要權杖（沒有 S124 免權杖那一行）：每秒更新暫停，點選項仍會重取', '#c60');
        }
        return;
      }
      st.tickError = m; errShown = true;
      say('❌ contactct.get（每秒更新）失敗：' + m, '#c00');
      return;
    }
    st.error = m; errShown = true;
    say('❌ contactct.get 失敗：' + m, '#c00');
    if (D) {                                      // 還原成伺服器那邊的選取
      if (D.rgYieldType && typeof D.rgYieldType.itemIndex === 'number') sel = D.rgYieldType.itemIndex;
      renderRadio();
    }
  }

  // kind：'open'（開頁＝FormShow）、'click'（點 rgYieldType）、'tick'（每秒重畫）
  function send(yt, kind) {
    if (inflight) {
      if (kind === 'tick') { st.skipped++; return Promise.resolve(null); }
      queued = { yt: yt, kind: kind };
      return Promise.resolve(null);
    }
    inflight = true;
    if (kind !== 'tick') say('讀取中…');
    return fetchCT(yt, kind === 'tick').then(function (d) {
      if (kind === 'tick') { st.ticks++; st.tickError = null; } else { st.loads++; st.error = null; }
      if (queued) return d;                       // 送出中使用者又點了：這一條已過時，不畫、不收（D／data＝畫面上那一份），下一條馬上送
      D = d; st.data = d;
      if (d.rgYieldType && typeof d.rgYieldType.itemIndex === 'number') sel = d.rgYieldType.itemIndex;
      render(d, kind);
      return d;
    }, function (e) {
      if (kind === 'tick') st.ticks++; else st.loads++;
      fail(e, kind);
      return null;
    }).then(function (d) {
      inflight = false;
      var q = queued; queued = null;
      if (q) return send(q.yt, q.kind);
      return d;
    });
  }

  function visible() {
    if (document.hidden) return false;
    try {
      var fe = window.frameElement;               // background.html 的 iframe；直接開這一頁時是 null
      if (fe && fe.getClientRects().length === 0) return false;   // 外框 display:none（縮小／關掉）
    } catch (e) { /* 拿不到 frameElement：當作看得到 */ }
    return true;
  }

  function onTick() {
    if (!visible()) return;
    if (D === null) {                             // 開頁那一下沒成功（例如 wb_serve 還沒起來）：每拍重試開窗
      if (!inflight) send(undefined, 'open');
      return;
    }
    send(sel, 'tick');
  }

  function notWired(ev) {
    say('尚未接：' + ev.currentTarget.id + '（golden ' +
        (ev.currentTarget.id === 'btClearCount' ? 'btClearCountClick cContactCT.cpp:944-1069' : 'btYieldChartClick cContactCT.cpp:1071-1075') +
        '）這一版只顯示資料，不寫入', '#c60');
  }

  function start() {
    $('btClearCount').addEventListener('click', notWired);
    $('btYieldChart').addEventListener('click', notWired);
    send(undefined, 'open');
    setInterval(onTick, TICK_MS);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ContactCT = {
    reload: function (yieldType) {
      if (yieldType === undefined || yieldType === null) return send(undefined, 'open');
      sel = yieldType;
      return send(yieldType, 'click');
    },
    data: function () { return D; },
    tick: onTick
  };
})();
