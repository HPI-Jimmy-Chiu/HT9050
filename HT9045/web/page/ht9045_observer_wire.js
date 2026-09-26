/* ht9045_observer_wire.js -- Data.Observer.html（golden V912 TfObserver，cObserver.cpp）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：wb_serve WS 指令 observer.get → cObserver.cpp 檔尾 W906_ObserverJson。
 *   value = {"act":..., "arg":n, "text":"..."}，每個 act 重播 golden 的一個操作：
 *     open    開頁＝golden 開窗 FormShow（:347-652）
 *     timer   每 1000 ms 一次＝golden Timer1（dfm 沒設 Interval → VCL 預設 1000；FormShow :435 啟動）
 *             → Timer1Timer → GetMachineData → ProcessRunInfo，只回 captions
 *     tab     點「Tester Category」(1)／「System Message」(3) 頁籤＝pgcObservChange（:2336）
 *     rowNo   點 rgRowNo 選項＝rgRowNoClick（:1665）
 *     form    點 Display Form 四顆 radio（text＝元件名）＝dfm OnClick=rgRowNoClick
 *     year    改 cbbEventLogYear（golden 沒有 OnChange，只改 Text）
 *     month   改 cbbMonth＝cbbMonthChange（:3764）
 *     file    點 lstEventLog＝lstEventLogClick（:3758）
 *     filter  改 cbbFilter（golden 沒有 OnChange，按 Query 才套用）
 *     query   按 Query＝btnQueryEventLogTxtClick（:4479）
 *   回應：captions{元件:Caption}、visible、noSource{元件:原因}、sources（golden 讀的輸入）、
 *         lastdata（那些欄位在 system\lastdata.dat 的位移）、category（16 個 TTMyTray＋radio）、
 *         eventLog（strngrdEventLog 全格＋年／月／檔案／Filter 狀態）。timer 只有前四項（full:false）。
 *
 * 這一支只接 golden 在 wb_serve 沒有其他 producer 的 14 格（Operating Information 8 格、
 * labVersion／labFactory、四個 *Version）＋ APHeadLabel18、Tester Category 整張、System Message 整張。
 * labModel／labSerialNo／labMachineID／labDeviceName／labReleaseDate 維持 ht9045_wire_dataobserver.js
 * （sysText／tag）接線，不在這裡重複寫，免得兩個寫入者搶同一格。
 *
 * 權杖：observer.get 不在 WebBridgeServer 的豁免清單，要 control 權杖。每次指令都是
 *   acquire → observer.get → release（持有只有幾毫秒）。Timer 每秒一次，若長期持有，
 *   其他頁面（Setup／Contact…）就永遠拿不到權杖存檔。別人正持有時，timer 這一拍略過。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var CAPS = ['labPowerOnTime', 'labRunningTime', 'labProductTime', 'labLoadingCount',
              'labMUBA', 'labMTBA', 'labMTBF', 'pnlDayJamRate',
              'labVersion', 'labFactory', 'pnlGPIBVersion', 'pnlESDVersion', 'pnlATCVersion', 'pnlTTLRS232Version',
              'APHeadLabel18'];
  var SRC = {   // 畫面 title 用：golden 寫這一格的那一行
    labPowerOnTime: 'golden cObserver.cpp:757 ConvertMSecToTime(LastSet.SystemAccSecond[0][stPowerOn])',
    labRunningTime: 'golden cObserver.cpp:758 ConvertMSecToTime(LastSet.SystemAccSecond[0][stStartTime])',
    labProductTime: 'golden cObserver.cpp:759 ConvertMSecToTime(LastSet.SystemAccSecond[0][stProductTime])',
    labLoadingCount: 'golden cObserver.cpp:761-764 LastSet.SendCT[1]（CC_AMKOR_Korea 用 SendCT[0]）',
    labMUBA: 'golden cObserver.cpp:2283-2307 ProcessRunInfo（LastSet.iJamCount[1]／SendCT[1]）',
    labMTBA: 'golden cObserver.cpp:2261-2281 ProcessRunInfo（(Pause+Product+Jam)/1000/iJamCount[1]）',
    labMTBF: 'golden cObserver.cpp:2309 ConvertSecondToSPC(LastSet.SystemAccSecond[0][stPowerOn]/1000)',
    pnlDayJamRate: 'golden cObserver.cpp:2311-2330 ProcessRunInfo（IniConfig.bVTESTFunction 才顯示）',
    labVersion: 'golden cObserver.cpp:3629 labVersion=Memo1 第 0 行（ShowVer :3554 asVer）',
    labFactory: 'golden main.cpp:11057 labFactory=RunInfo.Factory',
    APHeadLabel18: 'golden cObserver.cpp:375 MyDBQClearDT()'
  };

  var D = null;             // 最近一次完整（full）回應
  var busy = false, opened = false, tick = null;
  var st = window.__observer = { loads: 0, ticks: 0, skipped: 0, data: null, last: null, error: null, acts: [] };

  function $(id) { return document.getElementById(id); }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }
  function say(msg, colour) {
    var el = $('obsStatus');
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

  // TPanel 匯出的 <div class="pnl">：值寫進 .pnlCap（同 ht9045_wire_engine.js showText）
  function showText(el, text) {
    if (!el) return;
    if (el.classList && el.classList.contains('pnl')) {
      var cap = el.querySelector(':scope > .pnlCap');
      if (!cap) { cap = document.createElement('span'); cap.className = 'pnlCap'; el.appendChild(cap); }
      cap.classList.add('runtimePanelValue');
      cap.textContent = text;
      return;
    }
    el.textContent = text;
  }

  // acquire → observer.get → release。quiet=true（timer）時，權杖在別人手上就略過這一拍。
  // 操作員的點擊不可以因為剛好撞上 timer 那一拍而被吃掉：排隊，等前一個做完再送；timer 撞上就略過。
  var queue = [];
  function send(v, quiet) {
    if (busy) {
      if (quiet) { st.skipped++; return Promise.resolve(null); }
      return new Promise(function (resolve) { queue.push({ v: v, resolve: resolve }); });
    }
    return sendNow(v, quiet);
  }
  function sendNow(v, quiet) {
    busy = true;
    var held = false;
    return raw('control.acquire').then(function () { held = true; }, function (e) {
      if (quiet) throw new Error('control-held');
      throw new Error('拿不到控制權杖（' + e.message + '）—— 別的頁面正持有，observer.get 需要權杖');
    }).then(function () {
      return raw('observer.get', { value: JSON.stringify(v) }).then(unwrap);
    }).then(function (d) {
      st.last = d; st.error = null;
      if (v.act === 'timer') st.ticks++; else { st.loads++; st.acts.push(v.act); }
      if (d.full) { D = d; st.data = d; }
      render(d);
      return d;
    }).catch(function (e) {
      if (quiet && e.message === 'control-held') { st.skipped++; return null; }
      st.error = e.message;
      say('❌ observer.get（' + v.act + '）失敗：' + e.message, '#c00');
      return null;
    }).then(function (d) {
      var rel = held ? raw('control.release').catch(function () {}) : Promise.resolve();
      return rel.then(function () {
        busy = false;
        var next = queue.shift();
        if (next) sendNow(next.v, false).then(next.resolve);
        return d;
      });
    });
  }

  function render(d) {
    renderCaptions(d);
    if (d.full) {
      renderCategory(d.category);
      renderEventLog(d.eventLog);
    }
    var ns = Object.keys(d.noSource || {});
    say('observer.get ' + d.act + ' ✓ ' + new Date().toLocaleTimeString() +
        '（沒有來源、顯示 "---" 的格子：' + (ns.length ? ns.join('、') : '無') + '）', '#282');
  }

  function renderCaptions(d) {
    var caps = d.captions || {}, ns = d.noSource || {}, vis = d.visible || {};
    CAPS.forEach(function (id) {
      var el = $(id);
      if (!el) return;
      if (Object.prototype.hasOwnProperty.call(ns, id)) {
        showText(el, '---');
        el.title = id + '：沒有來源，顯示 "---"（不可知，不是 0）。\n' + ns[id];
        el.setAttribute('data-obs', 'nosource');
      } else if (Object.prototype.hasOwnProperty.call(caps, id)) {
        showText(el, caps[id]);
        el.title = id + '：C++ observer.get（' + (SRC[id] || 'golden TfObserver') + '）';
        el.setAttribute('data-obs', 'value');
      }
    });
    ['pnlDayJamRate', 'labDayJamRate'].forEach(function (id) {
      var el = $(id);
      if (el && Object.prototype.hasOwnProperty.call(vis, id)) el.style.display = vis[id] ? '' : 'none';
    });
  }

  // ---- Tester Category：golden ScrollBox1 的 16 個 TTMyTray ＋ rgRowNo ＋ GroupBox8，座標照 dfm ----
  function renderCategory(c) {
    var host = $('tcTrays');
    if (!host || !c) return;
    var sb = $('ScrollBox1');
    if (sb && c.scrollBoxColor) sb.style.background = c.scrollBoxColor;
    var h = '', maxH = 0;
    var rg = c.rgRowNo;
    h += '<fieldset class="tcRg" id="rgRowNo" title="rgRowNo : TRadioGroup（golden :1665 rgRowNoClick）" style="left:' + rg.left + 'px;top:' + rg.top +
         'px;width:' + rg.width + 'px;height:' + rg.height + 'px;"><legend>' + esc(rg.caption) + '</legend><div class="tcRgItems" style="grid-template-columns:repeat(' +
         Math.max(1, rg.columns) + ',1fr);">';
    rg.items.forEach(function (t, i) {
      h += '<label><input type="radio" name="rgRowNo" value="' + i + '"' + (i === rg.itemIndex ? ' checked' : '') + '>' + esc(t) + '</label>';
    });
    h += '</div></fieldset>';
    var gb = c.groupBox8;
    h += '<fieldset class="tcRg" id="GroupBox8" title="GroupBox8 : TGroupBox" style="left:' + gb.left + 'px;top:' + gb.top +
         'px;width:' + gb.width + 'px;height:' + gb.height + 'px;"><legend>' + esc(gb.caption) + '</legend>';
    (c.labels || []).forEach(function (l) {
      h += '<span class="tcLab" style="left:' + l.left + 'px;top:' + l.top + 'px;">' + esc(l.caption) + '</span>';
    });
    Object.keys(c.radios).forEach(function (id) {
      var r = c.radios[id];
      h += '<label class="tcRb" title="' + id + ' : TRadioButton" style="left:' + r.left + 'px;top:' + r.top + 'px;width:' + r.width + 'px;' +
           (r.enabled ? '' : 'color:#999;') + '"><input type="radio" name="tcForm" id="' + id + '" value="' + id + '"' +
           (r.checked ? ' checked' : '') + (r.enabled ? '' : ' disabled') + '>' + esc(r.caption) + '</label>';
    });
    h += '</fieldset>';
    c.trays.forEach(function (t) {
      maxH = Math.max(maxH, t.top + t.height);
      h += '<div class="tcTray" id="' + t.name + '" data-x="' + t.x + '" data-y="' + t.y + '" title="' + t.name +
           ' : TTMyTray（' + t.x + '×' + t.y + '）" style="left:' + t.left + 'px;top:' + t.top + 'px;width:' + t.width + 'px;height:' +
           t.height + 'px;background:' + t.color + ';grid-template-columns:repeat(' + Math.max(1, t.x) + ',1fr);grid-template-rows:repeat(' +
           Math.max(1, t.y) + ',1fr);">';
      for (var y = 0; y < t.y; y++) {
        for (var x = 0; x < t.x; x++) {
          var ci = (t.colorIndex[y] && t.colorIndex[y][x]) || 0;
          h += '<div class="tcCell" data-cx="' + x + '" data-cy="' + y + '" style="background:' + (t.colorMap[ci] || t.colorMap[0]) + ';">' +
               esc(t.cells[y][x]) + '</div>';
        }
      }
      h += '</div>';
    });
    host.innerHTML = h;
    host.style.height = (maxH + 12) + 'px';
    host.querySelectorAll('input[name=rgRowNo]').forEach(function (r) {
      r.addEventListener('change', function () { if (r.checked) send({ act: 'rowNo', arg: +r.value }); });
    });
    host.querySelectorAll('input[name=tcForm]').forEach(function (r) {
      r.addEventListener('change', function () { if (r.checked) send({ act: 'form', text: r.value }); });
    });
  }

  // ---- System Message／Text：strngrdEventLog 全格，年／月／檔案清單／Filter 照 C++ 狀態 ----
  function fillSelect(sel, items, idx, text) {
    if (!sel) return;
    var h = '';
    items.forEach(function (t, i) { h += '<option value="' + i + '">' + esc(t) + '</option>'; });
    sel.innerHTML = h;
    if (idx >= 0 && idx < items.length) sel.selectedIndex = idx;
    else if (text !== undefined && text !== '') {       // csDropDown：Text 可以不在 Items 裡
      var op = document.createElement('option'); op.value = 'text'; op.text = text; op.setAttribute('data-text', '1');
      sel.appendChild(op); sel.selectedIndex = sel.options.length - 1;
    } else sel.selectedIndex = -1;
    for (var i = 0; i < sel.options.length; i++) {
      if (i === sel.selectedIndex) sel.options[i].setAttribute('selected', 'selected'); else sel.options[i].removeAttribute('selected');
    }
  }

  function renderEventLog(e) {
    if (!e) return;
    var yi = e.years.indexOf(e.yearText);
    fillSelect($('cbbEventLogYear'), e.years, yi, e.yearText);
    fillSelect($('cbbMonth'), e.months, e.monthIndex, e.monthText);
    fillSelect($('cbbFilter'), e.filters, e.filterIndex, e.filterText);
    var lst = $('lstEventLog');
    if (lst) {
      var h = '';
      e.files.forEach(function (f, i) {
        h += '<div class="obsLi' + (i === e.fileIndex ? ' sel' : '') + '" data-i="' + i + '">' + esc(f) + '</div>';
      });
      lst.innerHTML = h;
      lst.querySelectorAll('.obsLi').forEach(function (d) {
        d.addEventListener('click', function () { send({ act: 'file', arg: +d.getAttribute('data-i') }); });
      });
    }
    var t = $('evGrid');
    if (!t) return;
    var cw = e.colWidths || [], tw = 0;
    var h2 = '<colgroup>';
    for (var c = 0; c < e.cols; c++) { h2 += '<col style="width:' + (cw[c] || 64) + 'px">'; tw += (cw[c] || 64); }
    h2 += '</colgroup><thead>';
    for (var r = 0; r < e.rows; r++) {
      if (r === e.fixedRows) h2 += '</thead><tbody>';
      var fixed = r < e.fixedRows;
      h2 += '<tr data-r="' + r + '">';
      for (c = 0; c < e.cols; c++) {
        var v = (e.cells[r] && e.cells[r][c] !== undefined) ? e.cells[r][c] : '';
        h2 += (fixed ? '<th' : '<td') + ' style="background:' + (fixed ? e.fixedColor : e.color) + ';">' + esc(v) + (fixed ? '</th>' : '</td>');
      }
      h2 += '</tr>';
    }
    if (e.rows <= e.fixedRows) h2 += '</thead><tbody>';
    h2 += '</tbody>';
    t.style.tableLayout = 'fixed';
    t.style.width = tw + 'px';
    t.innerHTML = h2;
    t.title = 'strngrdEventLog（golden GetEventLogText :3857）：' + (e.file || '（沒有檔案）') + '，' + e.rows + ' 列 × ' + e.cols + ' 欄';
  }

  function wire() {
    var y = $('cbbEventLogYear'), m = $('cbbMonth'), f = $('cbbFilter'), q = $('btnQueryEventLogTxt'), b = $('btnBackupLogYear');
    if (y) y.addEventListener('change', function () {
      send({ act: 'year', text: y.options[y.selectedIndex] ? y.options[y.selectedIndex].text : '' });
    });
    if (m) m.addEventListener('change', function () { send({ act: 'month', arg: m.selectedIndex }); });
    if (f) f.addEventListener('change', function () { send({ act: 'filter', arg: f.selectedIndex }); });
    if (q) q.addEventListener('click', function () { send({ act: 'query' }); });
    if (b) b.addEventListener('click', function () {
      say('尚未接：btnBackupLogYear（golden btnBackupLogYearClick cObserver.cpp:5427 會複製整年記錄）—— 這一版只顯示資料，不寫入', '#c60');
    });
    document.querySelectorAll('#pgcObserv > .pcTabs > .tab').forEach(function (tb) {
      var n = +tb.getAttribute('data-t');
      if (n === 1 || n === 3) tb.addEventListener('click', function () { if (opened) send({ act: 'tab', arg: n }); });
    });
  }

  function start() {
    // 開頁前先把 dfm 匯出的展示值拿掉（lstEventLog 的 3333/1/1…、evGrid 的說明列），等 C++ 回應
    var lst = $('lstEventLog'); if (lst) lst.innerHTML = '';
    var t = $('evGrid'); if (t) t.innerHTML = '<tbody><tr><td style="text-align:center;color:#666;padding:8px;">讀取中…（observer.get）</td></tr></tbody>';
    wire();
    say('讀取中…（observer.get open）');
    send({ act: 'open' }).then(function (d) {
      if (!d) return;
      opened = true;
      tick = setInterval(function () {
        if (document.hidden) return;             // 沒人在看的分頁不拍（golden Timer1 只在視窗開著時跑）
        send({ act: 'timer' }, true);
      }, 1000);
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045Observer = { send: send, data: function () { return D; } };
})();
