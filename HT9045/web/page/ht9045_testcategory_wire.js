/* ht9045_testcategory_wire.js -- Data.TestCategory.html <-> golden V912 TfTestCategory（cTestCategory.cpp／cTestCategory.dfm）
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925 (Data.TestCategory)
 * 手寫接線檔（檔名刻意不叫 ht9045_wire_<slug>.js，免得 gen_wire.py 重跑時覆蓋）。
 * 探針：HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_testcategory_probe.py
 *
 * 資料：全部是執行期 tag（WebBridgeTags.cpp 檔尾 W906_StageTestCategoryTags），不需要控制權杖。
 *   tcat.<g>.r<R>.c<C>        格子文字 —— golden sgArm1DrawCell（:145-359）畫出來的字，C++ 算好（-1→空白、關 site→"X"、
 *                             >=iTestBinCount→I20 Error Bin 字樣、NN 列對應…都在 C++）。網頁只貼上去，不判斷任何東西。
 *   tcat.<g>.r<R>.c<C>.bg     底色（golden TColor 整數：0x00BBGGRR；負數＝系統色 0x80000000|n）
 *   tcat.<g>.r<R>.c<C>.bold   粗體（golden pCanvas->Font->Style 含 fsBold）
 *   tcat.<g>.rowCount／colCount／colWidths／rowHeights   golden AdjFormData（:42-143）的版面
 *   tcat.<g>.visible          sgArm1 恆 true；sgArm2＝By Arm（golden SetShowCateMode 的表單高度 192／105）
 *   tcat.show／cateByArm／width／height                   表單狀態（說明列用）
 *   <g> = sgArm2（dfm 上方）／sgArm1（dfm 下方），畫面順序照 dfm。
 *
 * 不可知的規則（同 Data.Observer／Data.SortCT）：tag 是 null、還沒收到、或串流斷線 → 該格顯示 "---"、不上底色。
 *   底色也是狀態：不知道的時候不可以畫成白底或綠底。
 *   列欄數不可知時用 dfm 的 3x3（cTestCategory.dfm:29-33／:52-56）排出 "---" 格子。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var GRIDS = ['sgArm2', 'sgArm1'];          // DOM 順序＝dfm 由上而下（sgArm2 alClient 在上、sgArm1 alBottom 在下）
  var MAX_R = 3, MAX_C = 9;                  // C++ 固定送 r0..2 × c0..8（golden AdjFormData 的上限）
  var DFM_R = 3, DFM_C = 3, DFM_W = 80, DFM_H = 24;   // golden dfm RowCount/ColCount/DefaultColWidth、VCL 預設列高

  // golden 系統色（Graphics.hpp clXxx = 0x80000000 | COLOR_xxx）→ 佈景變數
  var SYS = {
    0x0F: 'var(--grid-head, #d4d0c8)',       // clBtnFace（列／欄標題）
    0x05: 'var(--input-bg, #ffffff)'         // clWindow（TStringGrid 預設底色，golden 沒畫的內容格）
  };

  var st = window.__tcat = { renders: 0, frames: 0, lastRender: null, connected: false, error: null };

  function $(id) { return document.getElementById(id); }
  function T(tag) {
    if (!window.HT9045Tags || !st.connected || !HT9045Tags.has(tag)) return null;
    var v = HT9045Tags.get(tag);
    return (v === undefined) ? null : v;
  }
  function say(msg, colour) {
    var el = $('tcatStatus');
    if (el) { el.textContent = msg || ''; el.style.color = colour || ''; }
  }

  // golden TColor → CSS。null＝不可知 → ''（不上色）。系統色 → 佈景變數；其餘 0x00BBGGRR。
  function css(v) {
    if (v === null || typeof v !== 'number') return '';
    var n = v | 0;
    if (n < 0) return SYS[n & 0xFF] || '';
    return 'rgb(' + (n & 0xFF) + ',' + ((n >> 8) & 0xFF) + ',' + ((n >> 16) & 0xFF) + ')';
  }
  function isSys(v) { return typeof v === 'number' && (v | 0) < 0; }
  function nums(s, n, dflt) {
    var a = (typeof s === 'string' && s !== '') ? s.split(',').map(function (x) { return parseInt(x, 10); }) : [];
    var out = [];
    for (var i = 0; i < n; i++) out.push(isFinite(a[i]) ? a[i] : dflt);
    return out;
  }

  function renderGrid(g) {
    var tbl = $(g);
    if (!tbl) return null;
    var p = 'tcat.' + g + '.';
    var rowsT = T(p + 'rowCount'), colsT = T(p + 'colCount');
    var rows = (typeof rowsT === 'number') ? Math.min(rowsT, MAX_R) : DFM_R;
    var cols = (typeof colsT === 'number') ? Math.min(colsT, MAX_C) : DFM_C;
    var cw = nums(T(p + 'colWidths'), cols, DFM_W);
    var rh = nums(T(p + 'rowHeights'), rows, DFM_H);
    var vis = T(p + 'visible');
    // false 才藏；null（不可知）照樣顯示（內容是 "---"）—— 不把「不知道」當成「關」。
    tbl.style.display = (vis === false) ? 'none' : '';

    var snap = { rows: rows, cols: cols, visible: vis, cells: [] };
    var html = [];
    for (var r = 0; r < rows; r++) {
      var rowSnap = [];
      html.push('<tr>');
      for (var c = 0; c < cols; c++) {
        var q = p + 'r' + r + '.c' + c;
        var text = T(q), bg = T(q + '.bg'), bold = T(q + '.bold');
        var unk = (text === null);
        var shown = unk ? '---' : String(text);
        var style = 'width:' + Math.max(cw[c] - 2, 8) + 'px;height:' + Math.max(rh[r] - 2, 8) + 'px;';
        style += 'font-weight:' + (bold === true ? 'bold' : 'normal') + ';';
        if (!unk && css(bg)) style += 'background:' + css(bg) + ';';
        if (!unk && typeof bg === 'number' && !isSys(bg)) style += 'color:#000;';   // golden pCanvas 字色＝TFont 預設 clWindowText（黑）；明確底色時不跟佈景
        var tagName = (r === 0 || c === 0) ? 'th' : 'td';          // VCL FixedRows=1／FixedCols=1
        html.push('<' + tagName + ' data-r="' + r + '" data-c="' + c + '"' + (unk ? ' class="tcatUnknown"' : '') +
                  ' title="' + q + '" style="' + style + '">' +
                  (shown === '' ? '&nbsp;' : shown.replace(/&/g, '&amp;').replace(/</g, '&lt;')) +
                  '</' + tagName + '>');
        rowSnap.push({ text: text, shown: shown, bg: bg, bold: bold });
      }
      html.push('</tr>');
      snap.cells.push(rowSnap);
    }
    tbl.innerHTML = html.join('');
    return snap;
  }

  function render() {
    // 連線狀態每次重畫都現問（status().connected 在收到第一個 tag 訊框時才變 true、斷線時變 false）
    st.connected = !!(window.HT9045Tags && HT9045Tags.status().connected);
    var out = {};
    GRIDS.forEach(function (g) { out[g] = renderGrid(g); });
    st.renders++;
    st.lastRender = out;
    if (!st.connected) {
      say('執行期資料未連線 —— 格子顯示 "---"（不可知）。', '#b60');
      return;
    }
    var show = T('tcat.show'), byArm = T('tcat.cateByArm'), w = T('tcat.width'), h = T('tcat.height');
    if (show === null) {
      say('C++ 還沒送 tcat.*（wb_serve 開機序列沒有跑 golden InitCateCell／DoShowUserDefFrom，或 config 未載入）—— 顯示 "---"。', '#b60');
      return;
    }
    var msg = 'golden fTestCategory：' + (byArm ? 'By Arm（Arm1／Arm2 兩張）' : 'By Socket（只看得到 sgArm1）') +
              (typeof w === 'number' ? '，Width=' + w : '') + (typeof h === 'number' ? '，Height=' + h : '');
    if (show === false) msg += '。⚠ config.ini [Visible] bShowTestCate=0：golden 這個視窗是關著的（值仍是機台上的真值）';
    say(msg, show === false ? '#b60' : '');
  }

  var pending = false;
  function schedule() {
    if (pending) return;
    pending = true;
    (window.requestAnimationFrame || function (f) { return setTimeout(f, 16); })(function () {
      pending = false;
      try { render(); } catch (e) { st.error = String(e && e.message || e); say('❌ 畫面更新失敗：' + st.error, '#c00'); }
    });
  }

  function start() {
    render();                                // 先畫出 dfm 3x3 的 "---"
    if (typeof HT9045Tags === 'undefined') {
      say('❌ 沒有載入 ht9045_recipe_client.js（HT9045Tags 不存在），無法接執行期資料。', '#c00');
      return;
    }
    HT9045Tags.subscribe(function (changed) {
      st.frames++;
      for (var k in changed) {
        if (k.lastIndexOf('tcat.', 0) === 0) { schedule(); return; }
      }
    });
    // 連線狀態：斷線時資料立刻當成不可知（recipe_client 斷線不會清 tag 表，殘值看起來會像活的）
    function poll() {
      var c = !!HT9045Tags.status().connected;
      if (c !== st.connected) schedule();
      if (!c) HT9045Tags.connect().then(schedule, function (e) { st.error = e.message; });
    }
    HT9045Tags.connect().then(schedule,
                              function (e) { st.error = e.message; say('⚠ 執行期資料連不上（' + e.message + '）—— 格子停在 "---"。', '#b60'); });
    setInterval(poll, 3000);
  }

  window.HT9045TestCategory = {
    grids: function () { return GRIDS.slice(); },
    last: function () { return st.lastRender; },
    render: render
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
