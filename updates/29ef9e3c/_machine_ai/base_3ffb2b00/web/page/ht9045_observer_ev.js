/* ht9045_observer_ev.js -- Data.Observer.html：Record 分頁、Exit、Time Data、Backup Log、Clear Time Data（golden TfObserver，V912 cObserver.cpp）
 * ---------------------------------------------------------------------------
 * //AI(W906-E021-OB1) 20261002 [W906] (St01) todo E-021（D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-021；
 *   St02 盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.2；Jimmy RULINGS_20261001 第 0 條）。
 * 手寫補件（檔名刻意不叫 ht9045_wire_<slug>.js）；開頁、每秒 timer、其他分頁仍在 ht9045_observer_wire.js：
 *   它 render() 看到回應有 dataRecord 就交給本檔 render（golden FormShow :571-593 的 Record 分頁），
 *   它 wire() 看到本檔就不綁 Backup Log 的「尚未接」。
 * 守衛與 golden 本體全部在 C++（cObserver.cpp 檔尾 ht9045::sjson::W906_ObserverAct，經 JsonBridge/ChanAction.cpp:344）：
 *   act.observer.<op>  value {"widgets":{...},"arg":n,"year":"..."}
 *   OB-1 exit（BtnExitClick :697-706）；關窗後 golden FormClose（:654-674，存 PrecautionParameter.ini）由頁面表的關窗邊緣跑（不是本檔送）
 *   OB-2 prNoteSet／prRecordSet／prRecordClear／prFormShow／prSave／prStartDate／prFinishDate（:4683-4764）
 *   OB-3 mmDate／mmStart／mmEnd／mmPhenAdd／mmCmAdd／mmPhenClear／mmCmClear／mmSave／mmSearch（:4766-4909）
 *   OB-4 prLogSearch（:4911-4984）
 *   OB-6 msgTab（pgcMessage 換分頁＝pgcMessageChange :4997-5024）／timeFile（lstTimeDataClick :5072-5075）；
 *        btnTimeData（Time Data 分頁的 Query）＝golden dfm :1813-1822 OnClick=btnQueryEventLogTxtClick ⇒ observer.get query（golden 怪處照接）
 *   OB-8 backupLogYear（:5602-5622；year＝cbbEventLogYear 的字，C++ 只用 atoi 的整數）
 *   OB-9 clearTime（:5624-5630）
 *   sync：操作員在 Record 分頁打字／選下拉（golden 的 TEdit／TComboBox／TMemo 自己留著；網頁版在瀏覽器）⇒ 改了就送，
 *        關窗時 golden FormClose 存的就是操作員打的字。
 * 每一個 act 都帶整個 Record 分頁的輸入（widgets），C++ 先套上再跑 golden，回來的 dataRecord 整塊重畫（焦點所在的那一格不動）。
 * golden 的提示框（ShowMyMessage：Not Enter Complete…）在 C++ 那邊等操作員按 OK（wb_serve MbWait），回應晚到是正常的。
 * //AI(W906-E021-B5) 20261002：紀錄檔照 golden 是 Big5；有字元沒有 Big5 字形時 C++ 回 guard not-big5、不帶 dataRecord（畫面上的字不被蓋掉，操作員改了再送）。
 * Exit：攔在 btExit 自己的 .exitbtn 之前（capture）；C++ 回 close:true 才關窗（golden Close()），否則不關（golden 的提示已經跳）；
 *   舊的 wb_serve（沒有 act.observer.*）或傳輸失敗 ⇒ 照舊直接關（E-021 之前的行為）。
 * 權杖：每次 control.acquire → act → control.release（同 ht9045_observer_wire.js getWithToken）。
 * 防連點：伺服器 WebCmdGuard（busy: 不是失敗，HT9045Busy）＋本頁 busy 旗標與 400 ms 冷卻（sync 排在後面，不丟）。
 * 測試：ctest E021_ObserverPage（tools/webprobe/e021_observer_selftest.cjs，node、離線）。
 * window.HT9045ObserverEv：render(dataRecord)／act(op, extra)／sync()／state()。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var EDITS = ['edPrecautionRecordDocumentNo', 'edNoteContents', 'edApprovedManager', 'edWatchmakers', 'edFinishName', 'edPromptDay',
               'edMajorMaintenanceCheckNo', 'edMajorMaintenancePersonnel', 'edMajorMaintenanceCheckPersonnel'];
  var COMBOS = ['cobPRFinishType', 'cobNoteContents', 'cobHandlerPrecautionRecord', 'cobMajorMaintenanceClassType',
                'cobUndesirablePhenomenon', 'cobCountermeasure', 'cobMajorMaintenanceSearch', 'cobSearchPrecautionLog'];
  var MEMOS_IN = ['MemoHandlerPrecautionRecord', 'MemoUndesirablePhenomenon', 'MemoCountermeasure'];
  var MEMOS_OUT = ['MemoNoteLog'];
  var BUTTONS = {   // 元件 id -> op（golden dfm OnClick）
    cobNoteContentsSet: 'prNoteSet', sbHandlerPrecautionRecordSet: 'prRecordSet', sbHandlerPrecautionRecordClear: 'prRecordClear',
    sbHandlerPrecautionFormShow: 'prFormShow', sbPrecautionSave: 'prSave', sbPRStartDate: 'prStartDate', sbPRFinishDate: 'prFinishDate',
    sbMajorMaintenanceDate: 'mmDate', sbMajorMaintenanceStartTime: 'mmStart', sbMajorMaintenanceEndTime: 'mmEnd',
    sbUndesirablePhenomenon: 'mmPhenAdd', sbCountermeasure: 'mmCmAdd', sbUndesirablePhenomenonClear: 'mmPhenClear',
    sbCountermeasureClear: 'mmCmClear', sbMajorMaintenanceSave: 'mmSave', sbMajorMaintenanceSearch: 'mmSearch',
    sbSearchPrecautionLog: 'prLogSearch', btnClearTime: 'clearTime', btnBackupLogYear: 'backupLogYear'
  };
  var PR_TABS = { 0: 'tsPrecautionsRecord', 1: 'tsHanderMajorMaintenance', 2: 'tsPrecautionLog' };

  var ready = false;              // 第一次拿到 dataRecord（C++ 的下拉清單）之前不送 widgets：頁面上還是 dfm 匯出的假選項
  var busy = false, coolUntil = 0, syncWanted = false, syncTimer = null;
  var st = { sent: [], renders: 0, lastError: '', last: null, syncs: 0, exitClosed: 0, exitFallback: 0, timeData: null };

  function $(id) { return document.getElementById(id); }
  function say(msg, bad) {
    var el = $('obsStatus');
    if (el) { el.textContent = msg ? 'Record：' + msg : ''; el.style.color = bad ? '#c00' : ''; }
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
  function raw(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra);
  }
  function setArea(el, text) { if (!el) return; el.textContent = text; el.value = text; }   // 同 Data.Observer.html 的 setArea（textContent＋value）
  function showText(el, text) {                                                         // 同 ht9045_observer_wire.js showText
    if (!el) return;
    if (el.classList && el.classList.contains('pnl')) {
      var cap = el.querySelector(':scope > .pnlCap');
      if (!cap) { cap = document.createElement('span'); cap.className = 'pnlCap'; el.appendChild(cap); }
      cap.textContent = text;
      return;
    }
    el.textContent = text;
  }
  function memoLines(el) {
    var v = String(el.value === undefined ? '' : el.value).replace(/\r/g, '');
    if (v === '') return [];
    if (v.charAt(v.length - 1) === '\n') v = v.slice(0, -1);   // VCL Lines.Text "a\r\n" = 1 行
    return v.split('\n');
  }

  // ---- 頁面 -> C++：Record 分頁的輸入 ----------------------------------------------------------------------------------------
  function collect() {
    if (!ready) return null;
    var w = {};
    EDITS.forEach(function (id) { var el = $(id); if (el) w[id] = String(el.value === undefined ? '' : el.value); });
    COMBOS.forEach(function (id) {
      var el = $(id); if (!el) return;
      var n = +el.getAttribute('data-items') || 0, i = el.selectedIndex;
      w[id] = { itemIndex: (typeof i === 'number' && i >= 0 && i < n) ? i : -1 };   // 多出來那一格（C++ 的 Text 不在清單裡）＝-1
    });
    MEMOS_IN.forEach(function (id) { var el = $(id); if (el) w[id] = memoLines(el); });
    return w;
  }

  // ---- C++ -> 頁面：dataRecord（W906_E021_DataRecordJson）------------------------------------------------------------------------
  function render(dr) {
    if (!dr || typeof dr !== 'object') return;
    st.renders++;
    var focused = document.activeElement;
    var tv = dr.tabVisible || {};
    var main = document.querySelector('#pgcObserv > .pcTabs > .tab[data-t="7"]');
    if (main) main.style.display = tv.tsDataRecord ? '' : 'none';                      // golden FormShow :571-593
    var first = null, activeHidden = false;
    Object.keys(PR_TABS).forEach(function (k) {
      var tb = document.querySelector('#pgcPrecautions > .pcTabs > .tab[data-t="' + k + '"]');
      if (!tb) return;
      var vis = !!tv[PR_TABS[k]];
      tb.style.display = vis ? '' : 'none';
      if (vis && first === null) first = tb;
      if (!vis && tb.classList.contains('act')) activeHidden = true;
    });
    if (activeHidden && first) first.click();                                           // VCL：藏起目前那一頁 ⇒ 換到看得到的一頁
    var ed = dr.edits || {};
    EDITS.forEach(function (id) { var el = $(id); if (el && el !== focused && typeof ed[id] === 'string') el.value = ed[id]; });
    var cb = dr.combos || {};
    COMBOS.forEach(function (id) {
      var el = $(id), c = cb[id]; if (!el || !c || el === focused) return;
      var items = c.items || [], h = '';
      items.forEach(function (t) { h += '<option>' + esc(t) + '</option>'; });
      var extra = !(c.itemIndex >= 0 && c.itemIndex < items.length);
      if (extra) h += '<option data-extra="1">' + esc(c.text || '') + '</option>';     // golden Text 不在清單裡（或沒有選）
      el.innerHTML = h;
      el.setAttribute('data-items', String(items.length));
      el.selectedIndex = extra ? items.length : c.itemIndex;
    });
    var me = dr.memos || {};
    MEMOS_IN.concat(MEMOS_OUT).forEach(function (id) { var el = $(id); if (el && el !== focused && me[id]) setArea(el, me[id].join('\n')); });
    var pn = dr.panels || {};
    Object.keys(pn).forEach(function (id) { showText($(id), pn[id]); });
    ready = true;
  }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }

  function renderTimeData(td) {
    if (!td) return;
    st.timeData = td;
    var lst = $('lstTimeData');
    if (lst) {
      var h = '';
      (td.files || []).forEach(function (f, i) {
        h += '<div data-i="' + i + '"' + (i === td.itemIndex ? ' class="sel" style="background:#316ac5;color:#fff;"' : '') + '>' + esc(f) + '</div>';
      });
      lst.innerHTML = h;
    }
    var g = td.grid, host = $('strngrdTimeData');
    if (host && g) {
      var t = '<table>';
      for (var r = 0; r < g.rows; r++) {
        t += '<tr>';
        for (var c = 0; c < g.cols; c++) {
          var v = (g.cells[r] && g.cells[r][c] !== undefined) ? g.cells[r][c] : '';
          t += (r < g.fixedRows || c < g.fixedCols) ? '<th>' + esc(v) + '</th>' : '<td>' + esc(v) + '</td>';
        }
        t += '</tr>';
      }
      host.innerHTML = t + '</table>';
      host.title = 'strngrdTimeData : TStringGrid —— C++ act.observer：golden GetTimeDataText（:5026-5070）讀 ' + (td.root || '') + '\\<年>\\*.CSV';
    }
  }

  // ---- act.observer.<op> -----------------------------------------------------------------------------------------------------
  function describe(r) {
    if (!r) return '沒有回應';
    if (r.guard === 'unknown-action' || /unknown cmd|unknown-action/.test(r.detail || '')) return '這一版 wb_serve 還沒有 act.observer.*（JsonBridge/ChanAction.cpp:344）：沒有執行';
    if (r.guard === 'record-name') return '沒有存檔：' + (r.detail || '') + '（[W906] 移植版比 golden 嚴：檔名裡不收路徑字元）';
    // //AI(W906-E021-B5) 20261002 [W906] (St01)：紀錄檔照 golden 是 Big5（cp950）；沒有 Big5 字形的字（簡體字、emoji）不收，畫面上的字留著讓操作員改
    if (r.guard === 'not-big5') return '沒有接受：有字元不是 Big5（繁體）字 —— ' + (r.detail || '') + '（紀錄檔照 golden 用 Big5 存；改掉那個字再送）';
    return '沒有執行：' + (r.guard || '?') + (r.detail ? '（' + r.detail + '）' : '') + (r.goldenLine ? ' ' + r.goldenLine : '');
  }
  function send(op, extra) {
    var v = {};
    var w = collect();
    if (w) v.widgets = w;
    Object.keys(extra || {}).forEach(function (k) { v[k] = extra[k]; });
    st.sent.push({ cmd: 'act.observer.' + op, value: v });
    if (st.sent.length > 60) st.sent.shift();
    var held = false;
    function release() { return held ? raw('control.release').catch(function () {}) : Promise.resolve(); }
    return raw('control.acquire').then(function () { held = true; }, function () { /* 他人持有：後面的指令自己會回 not-operator */ })
      .then(function () { return raw('act.observer.' + op, { value: JSON.stringify(v) }).then(unwrap, function (e) { return parseErr(e); }); })
      .then(function (r) { return release().then(function () { return r; }); });
  }
  function done(op, r) {
    st.last = r;
    if (r && r.dataRecord) render(r.dataRecord);
    if (r && r.timeData) renderTimeData(r.timeData);
    if (r && r.executed) {
      st.lastError = '';
      if (op !== 'sync') say(op + ' ✓ ' + (r.result || '') + (r.skipped && r.skipped.length ? '（沒做：' + r.skipped.join('；') + '）' : ''));
    } else if (isBusy(r)) { st.lastError = ''; say(window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'); }
    else { st.lastError = describe(r); say(st.lastError, true); }
    return r;
  }
  function act(op, extra) {
    if (busy || Date.now() < coolUntil) {
      if (op === 'sync') { syncWanted = true; return Promise.resolve({ executed: false, guard: 'queued' }); }
      say(window.HT9045Busy ? HT9045Busy.NOTE : '上一個操作還在跑，這一下略過');
      return Promise.resolve({ executed: false, guard: 'busy' });
    }
    busy = true;
    if (op !== 'sync') say(op + ' 送出中…');
    return send(op, extra).then(function (r) {
      busy = false; coolUntil = Date.now() + (op === 'sync' ? 0 : coolMs());
      done(op, r);
      if (op === 'sync') st.syncs++;
      if (syncWanted && op !== 'sync') syncWanted = false;   // 這一下已經帶了全部 widgets
      if (syncWanted) { syncWanted = false; setTimeout(function () { act('sync'); }, coolMs()); }
      return r;
    });
  }
  function sync() {
    if (!ready) return Promise.resolve(null);
    if (syncTimer) clearTimeout(syncTimer);
    return new Promise(function (resolve) { syncTimer = setTimeout(function () { syncTimer = null; act('sync').then(resolve); }, 150); });
  }

  // OB-1 Exit：攔在 .exitbtn（Data.Observer.html 的通用關窗）之前
  function exitClick() {
    if (busy) { say(window.HT9045Busy ? HT9045Busy.NOTE : '上一個操作還在跑，這一下略過'); return Promise.resolve(null); }
    busy = true;
    say('Exit 送出中…（golden BtnExitClick）');
    return send('exit', {}).then(function (r) {
      busy = false; coolUntil = Date.now() + coolMs();
      st.last = r;
      if (r && r.dataRecord) render(r.dataRecord);
      if (r && r.executed && r.close) { st.exitClosed++; closeWin(); }
      else if (r && r.executed) { say('沒有關窗：' + (r.result || 'golden BtnExitClick :699-704'), true); }   // golden 的提示框已經在 C++ 那邊跳過
      else if (isBusy(r)) { say(window.HT9045Busy ? HT9045Busy.NOTE : '同一個指令剛送過，這一下略過'); }
      else { st.exitFallback++; say(describe(r) + ' —— 照舊直接關窗', true); closeWin(); }
      return r;
    });
  }
  function closeWin() { try { if (window.parent && window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); } catch (e) {} }
  function onCaptureClick(ev) {
    var t = ev && ev.target, b = t && t.closest ? t.closest('#btExit') : null;
    if (!b) return;
    if (ev.stopImmediatePropagation) ev.stopImmediatePropagation();
    if (ev.preventDefault) ev.preventDefault();
    exitClick();
  }

  function yearText() {
    var y = $('cbbEventLogYear');
    if (!y) return '';
    if (y.options && y.selectedIndex >= 0 && y.options[y.selectedIndex]) return String(y.options[y.selectedIndex].text);
    return String(y.value || '');
  }

  function start() {
    Object.keys(BUTTONS).forEach(function (id) {
      var b = $(id); if (!b) return;
      b.addEventListener('click', function () {
        if (b.disabled) return;
        var op = BUTTONS[id];
        if (op === 'backupLogYear') return act(op, { year: yearText() });
        if (op === 'clearTime') return act(op).then(function (r) { if (r && r.executed && window.HT9045Observer && HT9045Observer.tick) HT9045Observer.tick(); });
        return act(op);
      });
    });
    EDITS.concat(COMBOS).concat(MEMOS_IN).forEach(function (id) { var el = $(id); if (el) el.addEventListener('change', function () { sync(); }); });
    if (!window.HT9045ObserverTimeDataWired) document.querySelectorAll('#pgcMessage > .pcTabs > .tab').forEach(function (tb) {   // golden pgcMessage OnChange（dfm :1326）；AI(W906-B43) 20261002：ht9045_observer_wire.js（機台 OBS-TIMEDATA）在就讓它接，不送第二次
      var n = +tb.getAttribute('data-t');
      tb.addEventListener('click', function () { act('msgTab', { arg: n }); });
    });
    var lst = window.HT9045ObserverTimeDataWired ? null : $('lstTimeData');   // AI(W906-B43) 20261002: the machine's ht9045_observer_wire.js already sends timeFile
    if (lst) lst.addEventListener('click', function (ev) {
      var d = ev.target && ev.target.closest ? ev.target.closest('[data-i]') : null;
      if (!d) return;                                                                   // dfm 的假項目（3333、1…）沒有 data-i：還沒列過檔
      act('timeFile', { arg: +d.getAttribute('data-i') });
    });
    var q = window.HT9045ObserverTimeDataWired ? null : $('btnTimeData');   // AI(W906-B43) 20261002: the machine's ht9045_observer_wire.js already sends query
    if (q) q.addEventListener('click', function () {                                   // golden dfm :1813-1822 OnClick=btnQueryEventLogTxtClick
      if (window.HT9045Observer && HT9045Observer.send) HT9045Observer.send({ act: 'query' });
    });
    document.addEventListener('click', onCaptureClick, true);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ObserverEv = { render: render, act: act, sync: sync, state: function () { return st; }, renderTimeData: renderTimeData };
})();
