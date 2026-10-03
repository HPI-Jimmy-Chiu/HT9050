/* ht9045_observer_sgjam.js -- Data.Observer.html：System Message → SG_JamCount 子分頁（golden TfObserver，906_0625 cObserver.cpp）
 * ---------------------------------------------------------------------------
 * AI(W906-ST02-OB7) 20261002 (St02-E helper)：E-019 OB-7。計畫 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\C7_FTPSAVE_SGJAM_PLAN_20261002.md 第二節。
 * 手寫補件（檔名刻意不叫 ht9045_wire_<slug>.js）；開頁、每秒 timer、其他分頁仍在 ht9045_observer_wire.js／ht9045_observer_ev.js。
 * golden（D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven，cp950）：
 *   btnSG_QueryNow（Query Now，dfm cObserver.dfm:1879-1887）       → btnSG_QueryNowClick :5361-5364       → StatisticalJamCount(false)
 *   btnSG_QueryYesterday（Query Yesterday，dfm :1888-1896）         → btnSG_QueryYesterdayClick :5366-5369 → StatisticalJamCount(true)
 *   StatisticalJamCount :5060-5276：讀今天／昨天的事件記錄 CSV，數 JAM01～JAM19 開頭的代碼，填 strngrdJamLog，寫 RawData.csv，
 *   Loader Count（labLoaderCount）＝iOneDayLoaderCount；Query Yesterday 之後 iOneDayLoaderCount 歸零（:5262-5263，照 golden）。
 *   golden 這一頁**沒有任何訊息框**；CSV 不存在時只寫程序記錄就返回、表格不變（:5106-5111）⇒ 頁面也不清表格，只在狀態列寫一行。
 * C++：act.observerSG.queryNow／queryYesterday（golden 兩支鈕）、act.observerSG.state（唯讀：golden 的 VCL 表格自己留著內容，
 *   網頁版切到 SG_JamCount 子分頁時把 C++ 表單上的表格讀回來）—— JsonBridge\actions\ObserverSGJam.cpp，經 JsonBridge\ChanAction.cpp:347。
 * 權杖：每次 control.acquire → act → control.release（同 ht9045_observer_ev.js）。
 * 防連點：伺服器 WebCmdGuard（busy: 不是失敗，HT9045Busy）＋本頁 busy 旗標與冷卻（HT9045Busy.coolMs，預設 400 ms）。
 * 子分頁：ht9045_observer_wire.js 在頁籤 data-t=3 上送 observer.get msgTab（golden pgcMessageChange）；本檔在同一下點擊之後
 *   STATE_DELAY_MS 再送 state（兩邊共用同一條連線的權杖，錯開送，免得對方的 release 落在本檔的 act 之前）。
 * 測試：ctest St02_ObserverSGJamPage（tools\webprobe\st02_observer_sgjam_selftest.cjs，node、離線）。
 * window.HT9045ObserverSG：act(op)／render(reply)／state()。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var CMD = 'act.observerSG.';
  var BUTTONS = { btnSG_QueryNow: 'queryNow', btnSG_QueryYesterday: 'queryYesterday' };   // golden dfm OnClick
  var LABEL = { queryNow: 'Query Now', queryYesterday: 'Query Yesterday', state: 'SG_JamCount' };
  // golden 建構子 TfObserver::TfObserver :305-317：ColCount=6、ColWidths、第 0 列表頭；dfm DefaultColWidth=120、RowCount 預設 5
  var HEADER = ['No', 'UnitName', 'AlarmCode', 'Message', 'Count', 'Rate (%)'];
  var WIDTHS = [50, 100, 100, 400, 100, 100];
  var STATE_DELAY_MS = 300;

  var busy = false, coolUntil = 0, inFlight = '', pending = null;
  var st = { sent: [], renders: 0, lastError: '', last: null, states: 0 };

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function say(msg, color) {
    var el = $('obsStatus');
    if (el) { el.textContent = msg ? 'SG_JamCount：' + msg : ''; el.style.color = color || ''; }
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function isBusy(x) { return !!(window.HT9045Busy && HT9045Busy.is(x)) || /^busy:/.test((x && (x.detail || x.message)) || ''); }
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function busyNote() { return window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'; }
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }

  // ---- C++ -> 頁面：strngrdJamLog／labLoaderCount -----------------------------------------------------------------------------
  function drawGrid(g) {
    var host = $('strngrdJamLog');
    if (!host || !g) return;
    var rows = Math.max(0, +g.rows || 0), cols = Math.max(0, +g.cols || 0), fr = +g.fixedRows || 0, fc = +g.fixedCols || 0;
    var cells = g.cells || [];
    var t = '<table><colgroup>';
    for (var c = 0; c < cols; c++) t += '<col style="width:' + (c < WIDTHS.length ? WIDTHS[c] : 120) + 'px">';
    t += '</colgroup>';
    for (var r = 0; r < Math.min(rows, cells.length); r++) {
      t += '<tr>';
      for (c = 0; c < cols; c++) {
        var v = (cells[r] && cells[r][c] !== undefined) ? cells[r][c] : '';
        t += (r < fr || c < fc) ? '<th>' + esc(v) + '</th>' : '<td>' + esc(v) + '</td>';
      }
      t += '</tr>';
    }
    host.innerHTML = t + '</table>';
    host.setAttribute('data-rows', String(rows));
    host.title = 'strngrdJamLog : TStringGrid —— C++ act.observerSG（golden StatisticalJamCount 906_0625 cObserver.cpp:5060-5276）' +
                 (g.truncated ? '；只顯示前 ' + cells.length + ' 列' : '');
  }
  function render(r) {
    if (!r || typeof r !== 'object') return;
    if (r.grid) { drawGrid(r.grid); st.renders++; }
    var lab = $('labLoaderCount');
    // C++ 的 Caption 是空字串＝golden 還沒寫過（StatisticalJamCount :5268 才寫）⇒ 保留 dfm 的設計時字 'labLoaderCount'（dfm :1857）
    if (lab && typeof r.labLoaderCount === 'string' && r.labLoaderCount !== '') lab.textContent = r.labLoaderCount;
  }
  function defaultGrid() {   // golden 建構子之後、還沒有人算過的樣子（表頭＋4 列空白）；C++ 回來的表格會蓋過它
    var cells = [HEADER.slice()];
    for (var i = 1; i < 5; i++) cells.push(['', '', '', '', '', '']);
    return { rows: 5, cols: 6, fixedRows: 1, fixedCols: 0, cells: cells };
  }

  // ---- act.observerSG.<op> ---------------------------------------------------------------------------------------------------
  function describe(r) {
    if (!r) return '沒有回應';
    if (/unknown cmd|unknown-action/.test((r.guard || '') + ' ' + (r.detail || '')) && !/act\.observerSG\.\* has/.test(r.detail || ''))
      return '這一版 wb_serve 還沒有 act.observerSG.*（JsonBridge\\ChanAction.cpp:347）：沒有執行';
    if (r.guard === 'not-open') return '沒有執行：Observer 視窗還沒開好（observer.get open）';
    if (r.guard === 'log-objects-not-created') return '沒有執行：事件記錄物件 slEventLog 還沒建立（[W906] 移植版的守衛；golden 不會發生）';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '');
  }
  function send(op) {
    var v = {};
    st.sent.push({ cmd: CMD + op, value: v });
    if (st.sent.length > 60) st.sent.shift();
    var held = false;
    function release() { return held ? raw('control.release').catch(function () {}) : Promise.resolve(); }
    return raw('control.acquire').then(function () { held = true; }, function () { /* 他人持有：後面的指令自己會回 not-operator */ })
      .then(function () { return raw(CMD + op, { value: JSON.stringify(v) }).then(unwrap, function (e) { return parseErr(e); }); })
      .then(function (r) { return release().then(function () { return r; }); });
  }
  function done(op, r, quiet) {
    st.last = r;
    render(r);
    if (r && r.executed) {
      st.lastError = '';
      if (op === 'state') { st.states++; return r; }
      if (r.early === 'eventlog-missing')
        say(LABEL[op] + '：沒有事件記錄檔 ' + ((r.eventLogCsv && r.eventLogCsv.path) || '') + '（golden 只寫程序記錄就返回，表格不變）', '#c60');
      else if (r.early === 'initial-not-ok')
        say(LABEL[op] + '：機台還沒初始化完成（golden InitialOK==false 直接返回，表格不變）', '#c60');
      else
        say(LABEL[op] + ' ✓ ' + Math.max(0, ((r.grid && +r.grid.rows) || 0) - 2) + ' 種 JAM 代碼；Loader Count ' +
            (r.labLoaderCount !== undefined ? r.labLoaderCount : '') + (r.skipped && r.skipped.length ? '（沒做：' + r.skipped.join('；') + '）' : ''));
    } else if (isBusy(r)) {
      st.lastError = '';
      if (!quiet) say(busyNote());
    } else {
      st.lastError = describe(r);
      if (!quiet) say(st.lastError, '#c00');
    }
    return r;
  }
  function act(op) {
    var quiet = (op === 'state');
    if (busy) {
      // 只是背景的 state 在跑：操作員按的這一下排在它後面送（不丟）；按鈕自己的請求還在跑：丟掉（防連點）
      if (!quiet && inFlight === 'state') { pending = op; return Promise.resolve({ executed: false, guard: 'queued' }); }
      if (!quiet) say(busyNote());
      return Promise.resolve({ executed: false, guard: 'busy' });
    }
    if (!quiet && Date.now() < coolUntil) { say(busyNote()); return Promise.resolve({ executed: false, guard: 'busy' }); }
    busy = true; inFlight = op;
    if (!quiet) say(LABEL[op] + ' 送出中…');
    return send(op).then(function (r) {
      busy = false; inFlight = '';
      if (!quiet) coolUntil = Date.now() + coolMs();
      done(op, r, quiet);
      if (pending) { var p = pending; pending = null; act(p); }
      return r;
    });
  }

  function start() {
    drawGrid(defaultGrid());
    Object.keys(BUTTONS).forEach(function (id) {
      var b = $(id); if (!b) return;
      b.addEventListener('click', function () { if (!b.disabled) act(BUTTONS[id]); });
    });
    var tab = document.querySelector('#pgcMessage > .pcTabs > .tab[data-t="3"]');   // tsSGJamCount
    if (tab) tab.addEventListener('click', function () { setTimeout(function () { act('state'); }, STATE_DELAY_MS); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ObserverSG = { act: act, render: render, state: function () { return st; } };
})();
