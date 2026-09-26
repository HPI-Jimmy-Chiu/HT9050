/* ht9045_contactct_wire.js -- Data.ContactCT.html（golden TfContactCT，cContactCT.cpp V912）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：wb_serve WS 指令 contactct.get → cContactCT.cpp 檔尾 W906_ContactCTJson。
 *   開頁：contactct.get（不帶 value）＝ golden 開窗 FormShow（ItemIndex=3）→ 整格重畫
 *   切選項：contactct.get value={"yieldType":n} ＝ golden 點 rgYieldType 第 n 項（rgYieldTypeClick）→ 整格重畫
 *   回應：{rgYieldType:{items,itemIndex,columns}, rows, cols, colWidths, rowHeights,
 *          cells[row][col]:{text,bg,fg,drawn}, palette:{clXxx:"#RRGGBB"}, font:{bold,size}, initialOK, errors[]}
 *   格子文字與顏色全部照回應畫（bg／fg 是 golden 的 clXxx 色名，RGB 由回應的 palette 對照）。
 * Count Clear（golden btClearCountClick :944-1069）與 Yield Chart（btYieldChartClick :1071-1075）這次不接。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var D = null;          // 最近一次回應（探針也讀這個）
  var busy = false;

  function $(id) { return document.getElementById(id); }
  function say(msg, colour) {
    var el = $('ctStatus');
    if (el) { el.textContent = msg || ''; el.style.color = colour || ''; }
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function acquire() {
    return cmd('control.acquire').catch(function () { /* 已持有或他人持有：後面的指令自己會回錯 */ });
  }
  function colour(name) {
    if (D && D.palette && D.palette[name]) return D.palette[name];
    return '';
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
      r.addEventListener('change', function () { if (r.checked) load(i); });
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
        var st = 'background:' + colour(cell.bg) + ';color:' + colour(cell.fg) + ';';
        if (cell.drawn && D.font) {
          if (D.font.bold) st += 'font-weight:bold;';
          if (D.font.size) st += 'font-size:' + Math.round(D.font.size * 96 / 72) + 'px;';
        }
        html += '<td class="' + (cell.drawn ? 'drawn' : '') + '" data-r="' + r + '" data-c="' + k +
                '" data-bg="' + cell.bg + '" data-fg="' + cell.fg + '" style="' + st + '">' +
                String(cell.text).replace(/&/g, '&amp;').replace(/</g, '&lt;') + '</td>';
      }
      html += '</tr>';
    }
    t.innerHTML = html;
  }

  function load(yieldType) {
    if (busy) return Promise.resolve();
    busy = true;
    say('讀取中…');
    var extra = {};
    if (yieldType !== undefined && yieldType !== null) extra.value = JSON.stringify({ yieldType: yieldType });
    return acquire().then(function () { return cmd('contactct.get', extra); }).then(function (d) {
      D = d;
      window.__contactct = { data: d, loads: (window.__contactct ? window.__contactct.loads : 0) + 1 };
      renderRadio();
      renderGrid();
      var msg = '';
      if (!d.initialOK) msg = '⚠ C++ InitialOK=false：golden sgYieldDrawCell 開頭就 return，格子不會畫（顯示的是 VCL 預設格）';
      if (d.errors && d.errors.length) msg += (msg ? '；' : '') + '⚠ ' + d.errors.length + ' 格繪製例外：' + d.errors[0].error;
      say(msg, msg ? '#c60' : '');
    }).catch(function (e) {
      say('❌ contactct.get 失敗：' + e.message, '#c00');
      window.__contactct = { error: e.message, loads: (window.__contactct ? window.__contactct.loads : 0) + 1 };
      if (D) renderRadio();   // 還原成伺服器那邊的選取
    }).then(function () { busy = false; });
  }

  function notWired(ev) {
    say('尚未接：' + ev.currentTarget.id + '（golden ' +
        (ev.currentTarget.id === 'btClearCount' ? 'btClearCountClick cContactCT.cpp:944-1069' : 'btYieldChartClick cContactCT.cpp:1071-1075') +
        '）這一版只顯示資料，不寫入', '#c60');
  }

  function start() {
    $('btClearCount').addEventListener('click', notWired);
    $('btYieldChart').addEventListener('click', notWired);
    load();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045ContactCT = { reload: load, data: function () { return D; } };
})();
