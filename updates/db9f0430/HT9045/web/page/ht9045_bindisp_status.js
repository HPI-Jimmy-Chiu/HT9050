/* ht9045_bindisp_status.js -- Status.ShowBinSelect.html 的「Bin Display Status」分頁（golden 906 TfShowBinSelect tsUnloadMap）
 * ---------------------------------------------------------------------------
 * AI(W906-ST02-C14) 20261002 (St02-E helper) 新檔（手寫）。卡 ST02-C14 第 3 部分（Steven 1002 14:4x）。
 * golden 906 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven（cp950；RULINGS_20261002 #20：這張卡只照 906）：
 *   版面  cShowBinSelect.dfm:1361-2790 tsUnloadMap：pnlLoad（Top 0）、pnlFix123（58）、pnlFix789（174）、pnlAuto123（290）、
 *         pnlAuto456（348）、pnlMag123（408）由上而下 Align=alTop，每列三格（Left 4／88／172），gbBinBox 在 pnlFix123 第一列第四格；
 *         每格 gbX（80×58）裡 pnlX（65×30，粗體 16px，BevelInner bvLowered）＋ lblX（clNavy 粗體 13px）；pnlLoad 三格沒有 lblX；
 *         sbRunStatus 在最下面。golden PageControl1Change（cShowBinSelect.cpp:1594-1612）把視窗放寬到 280（有 BulkBox 364）——
 *         網頁視窗寬度歸 background.html（245），這裡不改，分頁內捲動（同本頁其他分頁）。
 *   畫法  ChangeBinDispStatus（:208-386）由 C++ 畫在門面上（每秒一次，MainTimer3.cpp），這一頁只貼結果：
 *         底色＝pnlX->Color（golden ColorMap :218 五色：clGray、clRed、clGreen、0x000080FF 橘、clBlack），字＝pnlX->Caption
 *         （:304-323 L／E／C／X／數字；沒裝的格子灰底 X :333-337），錯的格子黑紅每秒交替（:325-331，C++ 的 bChangeColor），
 *         狀態列＝sbRunStatus（:376-385，「Bin display got error!!」紅底，或 GetRunStatus() 灰底）。字色照 dfm（clWindowText 黑）。
 *   字列  ShowBinSel（:388-756）寫的 lblX（bin 清單）與它的字色。
 *   可見  FormShow :760-764：NUMBER_PANEL_TYPE 不是 3／4 就隱藏這個分頁（binsel.disp.tabVisible）；
 *         SetAutoVisible :1436-1477：pnlMag123／pnlFix789／pnlAuto456（binsel.disp.groups），每一站的 gbX＝grpBinDisp[i]
 *         （golden 建構子 :110-130 的陣列就是 gbAuto1..gbMag14）＝bin.visible。bin.visible 還不可知時全部列出（同 Test Bin 分頁）。
 *   跳頁  golden :269-273 有錯就 PageControl1->ActivePageIndex=3（每次呼叫，也就是每秒一次）：C++ 數這個跳頁
 *         （binsel.disp.jumpSeq，BinDisplay/BinDispBringUp_St02.h W906_BinDispShownScope_St02），每加一這一頁就切到本分頁。
 *         所以有錯的期間操作員停不住別的分頁 —— golden 906 本來就這樣（HUMAN REVIEW B）。
 * 資料：wb_serve 的 tag binsel.disp.*（HT9011UC_Cpp_V3.33.906.0/WebBinDispStatus_St02.cpp 檔頭有每一個的定義），只讀、推送式（HT9045Tags），
 *   不輪詢：golden 的 1 秒節拍在 C++（MainTimer3.cpp 每秒一次 ChangeBinDispStatus），C++ 只在 BinSelect 視窗開著或縮小時送這組 tag
 *   （RULINGS_20260930 #12），這一頁收到變動才重畫 —— 視窗關著時沒有任何請求。
 * 沒有連上／沒有這組 tag（離線預覽）：格子照 golden 版面列出，字「---」、不上底色；狀態列「---」。null＝不可知，不是 0。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  // eBinDispName 順序 ＝ golden ChangeBinDispStatus UnLoadPanel[]（:210-216）＝ binsel.disp.caption／color／inst／err／colorNow／binNow
  var DISP = ['Loader', 'Empty', 'Color', 'Auto1', 'Auto2', 'Auto3',
              'Fix1', 'Fix2', 'Fix3', 'Fix4', 'Fix5', 'Fix6', 'BinBox',
              'Mag1', 'Mag2', 'Mag3', 'Mag4', 'Mag5', 'Mag6', 'Mag7', 'Mag8', 'Mag9', 'Mag10', 'Mag11', 'Mag12', 'Mag13', 'Mag14',
              'Auto4', 'Auto5', 'Auto6', 'Fix7', 'Fix8', 'Fix9', 'Fix10', 'Fix11', 'Fix12'];
  // e6TrayName 順序 ＝ UnLoadLabel[]／grpBinDisp[]（golden :110-130）＝ binsel.disp.lbl／lblColor、bin.visible
  var TRAY = ['Auto1', 'Auto2', 'Auto3', 'Auto4', 'Auto5', 'Auto6',
              'Fix1', 'Fix2', 'Fix3', 'Fix4', 'Fix5', 'Fix6', 'Fix7', 'Fix8', 'Fix9', 'Fix10', 'Fix11', 'Fix12',
              'BinBox',
              'Mag1', 'Mag2', 'Mag3', 'Mag4', 'Mag5', 'Mag6', 'Mag7', 'Mag8', 'Mag9', 'Mag10', 'Mag11', 'Mag12', 'Mag13', 'Mag14'];
  // golden dfm 版面：[容器名, 列[]]，格＝[元件後綴, gbX 的 Caption]（dfm 原字，含 'Bulkbox'、'Mag.1'）
  function row(names, cap) { return names.map(function (n) { return [n, cap ? cap(n) : n]; }); }
  function mag(n) { return 'Mag.' + n.slice(3); }
  var LAYOUT = [
    ['pnlLoad',    [row(['Loader', 'Empty', 'Color'])]],
    ['pnlFix123',  [row(['Fix1', 'Fix2', 'Fix3']).concat([['BinBox', 'Bulkbox']]), row(['Fix4', 'Fix5', 'Fix6'])]],
    ['pnlFix789',  [row(['Fix7', 'Fix8', 'Fix9']), row(['Fix10', 'Fix11', 'Fix12'])]],
    ['pnlAuto123', [row(['Auto1', 'Auto2', 'Auto3'])]],
    ['pnlAuto456', [row(['Auto4', 'Auto5', 'Auto6'])]],
    ['pnlMag123',  [row(['Mag1', 'Mag2', 'Mag3'], mag), row(['Mag4', 'Mag5', 'Mag6'], mag), row(['Mag7', 'Mag8', 'Mag9'], mag),
                    row(['Mag10', 'Mag11', 'Mag12'], mag), row(['Mag13', 'Mag14'], mag)]]
  ];
  var GROUP_AT = { pnlMag123: 0, pnlFix789: 1, pnlAuto456: 2 };   // binsel.disp.groups 的字元位置；pnlLoad／pnlFix123／pnlAuto123 golden 沒有開關
  var NO_LABEL = { Loader: 1, Empty: 1, Color: 1 };                // dfm gbLoader／gbEmpty／gbColor 只有 pnlX（高 47）
  var COLORMAP = ['clGray', 'clRed', 'clGreen', '0x000080FF（橘）', 'clBlack'];   // golden :218（906 五色），只用在說明

  var st = window.__binDispStatus = { renders: 0, jumps: 0, lastJumpSeq: null, built: false };

  function $(id) { return document.getElementById(id); }
  function tag(name) { return (window.HT9045Tags && HT9045Tags.has(name)) ? HT9045Tags.get(name) : null; }

  // golden TColor（$00BBGGRR；$80000000|n 是系統色）→ CSS（同 ht9045_showbinselect_wire.js 的換法）
  var SYSCOLOR = { 0x05: '#ffffff', 0x08: '#000000', 0x0F: '#f0f0f0', 0x10: '#a0a0a0', 0x12: '#000000' };
  function tcolor(v) {
    var n = Number(v);
    if (v === null || v === undefined || v === '' || !isFinite(n)) return '';
    n = n >>> 0;
    if ((n & 0xFF000000) >>> 0 === 0x80000000) return SYSCOLOR[n & 0xFF] || '';
    var r = n & 0xFF, g = (n >>> 8) & 0xFF, b = (n >>> 16) & 0xFF;
    function h(x) { return (x < 16 ? '0' : '') + x.toString(16).toUpperCase(); }
    return '#' + h(r) + h(g) + h(b);
  }
  function list(v, sep, n) {                                       // 長度不對＝不可知
    if (typeof v !== 'string') return null;
    var a = v.split(sep);
    return a.length === n ? a : null;
  }
  function flags(v, n) { return (typeof v === 'string' && v.length === n) ? v : null; }

  // ---- 純計算：tag 值 → 每個元件要顯示的東西（node 測試直接呼叫這一支） --------------------------------------------
  function model(get) {
    var cap = list(get('binsel.disp.caption'), '\t', DISP.length), col = list(get('binsel.disp.color'), ',', DISP.length);
    var inst = flags(get('binsel.disp.inst'), DISP.length), err = flags(get('binsel.disp.err'), DISP.length);
    var cnow = list(get('binsel.disp.colorNow'), ',', DISP.length), bnow = list(get('binsel.disp.binNow'), ',', DISP.length);
    var lbl = list(get('binsel.disp.lbl'), '\t', TRAY.length), lcol = list(get('binsel.disp.lblColor'), ',', TRAY.length);
    var vis = flags(get('bin.visible'), TRAY.length), groups = flags(get('binsel.disp.groups'), 3);
    var m = { pnl: {}, lbl: {}, gb: {}, grp: {}, status: null, tab: null, jumpSeq: null };
    DISP.forEach(function (n, i) {
      var known = !!(cap && col);
      var t = 'pnl' + n + '（golden ChangeBinDispStatus UnLoadPanel[' + i + ']）';
      if (inst) t += '：UnitHasInstall=' + (inst.charAt(i) === '1' ? '是' : '否');
      if (err) t += '、GerErrNow=' + (err.charAt(i) === '1' ? '是（黑紅交替）' : '否');
      if (cnow) { var ci = Number(cnow[i]); t += '、GetColorNow=' + cnow[i] + (COLORMAP[ci] ? '（' + COLORMAP[ci] + '）' : ''); }
      if (bnow) t += '、GetBinNow=' + bnow[i];
      if (!known) t += '：沒有來源 → 不可知';
      m.pnl[n] = { text: known ? cap[i] : '---', bg: known ? tcolor(col[i]) : '', title: t };
    });
    TRAY.forEach(function (n, i) {
      var known = !!lbl;
      m.lbl[n] = { text: known ? lbl[i] : '---', color: (known && lcol) ? tcolor(lcol[i]) : '',
                   title: 'lbl' + n + '（golden ShowBinSel UnLoadLabel[' + i + ']）' + (known ? '' : '：沒有來源 → 不可知') };
      m.gb[n] = vis ? (vis.charAt(i) === '1') : true;               // grpBinDisp[i]->Visible；不可知＝列出
    });
    Object.keys(GROUP_AT).forEach(function (g) {
      m.grp[g] = (vis && groups) ? (groups.charAt(GROUP_AT[g]) === '1') : true;
    });
    var s = get('binsel.disp.status'), sc = get('binsel.disp.statusColor');
    m.status = { text: (typeof s === 'string') ? s : '---', bg: (typeof s === 'string') ? tcolor(sc) : '',
                 title: 'sbRunStatus（golden :376-385）' + ((typeof s === 'string') ? '' : '：沒有來源 → 不可知') };
    var tv = get('binsel.disp.tabVisible');
    m.tab = (tv === true || tv === false) ? tv : null;
    var j = get('binsel.disp.jumpSeq');
    m.jumpSeq = (typeof j === 'number' && isFinite(j)) ? j : null;
    return m;
  }

  // ---- DOM ------------------------------------------------------------------------------------------------------------
  var CSS = '#bdMap .bdGrp{margin:0 0 2px 0;}' +
            '#bdMap .bdRow{display:flex;gap:3px;margin:0 0 2px 0;}' +
            '#bdMap fieldset.bdGb{flex:none;width:72px;box-sizing:border-box;margin:0;padding:0 3px 2px 3px;border:1px solid #999;}' +
            '#bdMap fieldset.bdGb>legend{font-size:11px;padding:0 2px;line-height:13px;}' +
            '#bdMap .bdPnl{height:26px;line-height:26px;text-align:center;font-weight:bold;font-size:16px;color:#000;' +
            'border:2px inset #c0c0c0;box-sizing:border-box;overflow:hidden;white-space:nowrap;}' +
            '#bdMap .bdLbl{height:15px;line-height:15px;font-weight:bold;font-size:12px;color:#000080;overflow:hidden;white-space:nowrap;}' +
            '#sbRunStatus.bdSb{margin-top:4px;padding:1px 4px;border:1px inset #c0c0c0;font-size:11px;min-height:15px;}';
  function el(tagName, cls, id, text) {
    var e = document.createElement(tagName);
    if (cls) e.className = cls;
    if (id) e.id = id;
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function build() {
    var map = $('bdMap');
    if (!map || st.built) return !!map;
    if (!$('bdStyle') && document.head) { var s = el('style', '', 'bdStyle'); s.textContent = CSS; document.head.appendChild(s); }
    map.textContent = '';
    LAYOUT.forEach(function (p) {
      var grp = el('div', 'bdGrp', p[0]);
      grp.title = p[0] + '（golden cShowBinSelect.dfm tsUnloadMap，Align=alTop）';
      p[1].forEach(function (cells) {
        var r = el('div', 'bdRow');
        cells.forEach(function (c) {
          var gb = el('fieldset', 'gbx bdGb', 'gb' + c[0]);
          gb.appendChild(el('legend', '', '', c[1]));
          gb.appendChild(el('div', 'bdPnl', 'pnl' + c[0], '---'));
          if (!NO_LABEL[c[0]]) gb.appendChild(el('div', 'bdLbl', 'lbl' + c[0], '---'));
          r.appendChild(gb);
        });
        grp.appendChild(r);
      });
      map.appendChild(grp);
    });
    st.built = true;
    return true;
  }
  function tabEl(name) { return document.querySelector('#PageControl1 > .tab[data-tab="' + name + '"]'); }
  function show(e, on) { if (e) e.style.display = on ? '' : 'none'; }

  function render() {
    if (!build()) return;
    var m = model(tag);
    st.renders++;
    st.last = m;
    Object.keys(m.pnl).forEach(function (n) {
      var e = $('pnl' + n), v = m.pnl[n];
      if (!e) return;
      e.textContent = v.text === '' ? '\u00a0' : v.text;
      e.style.background = v.bg;
      e.title = v.title;
    });
    Object.keys(m.lbl).forEach(function (n) {
      var e = $('lbl' + n), v = m.lbl[n];
      if (e) { e.textContent = v.text === '' ? '\u00a0' : v.text; e.style.color = v.color; e.title = v.title; }
      show($('gb' + n), m.gb[n]);
    });
    Object.keys(m.grp).forEach(function (g) { show($(g), m.grp[g]); });
    var sb = $('sbRunStatus');
    if (sb) { sb.textContent = m.status.text === '' ? '\u00a0' : m.status.text; sb.style.background = m.status.bg; sb.title = m.status.title; }

    var t = tabEl('bindisp');
    if (t) {
      show(t, m.tab !== false);                                     // golden FormShow :760-764；不可知時照樣顯示
      t.title = 'tsUnloadMap（golden FormShow :760-764：NUMBER_PANEL_TYPE 3／4 才有）' + (m.tab === null ? '：可見度不可知' : '');
      if (m.tab === false && t.classList.contains('act')) { var first = tabEl('testbin'); if (first) first.click(); }
    }
    if (m.jumpSeq !== null) {                                       // golden :272 PageControl1->ActivePageIndex=3
      if (st.lastJumpSeq !== null && m.jumpSeq > st.lastJumpSeq && m.tab !== false && t && !t.classList.contains('act')) {
        st.jumps++;
        t.click();
      }
      st.lastJumpSeq = m.jumpSeq;
    }
  }

  function interesting(changed) {
    if (!changed) return true;
    for (var k in changed) if (k === 'bin.visible' || k.indexOf('binsel.disp.') === 0) return true;
    return false;
  }
  function start() {
    render();
    if (window.HT9045Tags && typeof HT9045Tags.subscribe === 'function')
      HT9045Tags.subscribe(function (changed) { if (interesting(changed)) render(); });
    if (window.HT9045Tags && typeof HT9045Tags.connect === 'function') HT9045Tags.connect().catch(function () {});
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();

  window.HT9045BinDispStatus = { model: model, render: render, tcolor: tcolor, DISP: DISP, TRAY: TRAY, LAYOUT: LAYOUT, state: function () { return st; } };
})();
