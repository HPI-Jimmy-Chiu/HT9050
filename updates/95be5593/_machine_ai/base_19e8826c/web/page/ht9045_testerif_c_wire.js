/* ht9045_testerif_c_wire.js -- Setup.TesterIF.html（golden TFTestIF，cTesterIF.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_TesterIF.cpp（WS editlist.get／editlist.save tag=TestIF_File_TesterIF）。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE）照通用規則讀寫；本檔只補通用規則做不到的五件事：
 *
 *  (0) 這一頁不走 B 路寫 Tester.Data：HT9045Recipe.write('tester', …) 在這一頁直接拒絕
 *      （Tester.Data 由 golden TFTestIF::spbSaveClick 寫；同一個檔只能有一個寫者）。
 *  (1) 下拉選項＝後端 golden 執行期的 Items（editlist.get 回應 extra.items）：
 *        cbDIOType：golden InitcbDIOType（:70）列 DIOCFGPath\*.ini —— DFM 沒有 Items，頁面原本只有佔位選項；
 *        cbGPIBType：CC_ASE_KaohSiung 時 golden 建構子（:47-60）改名；其餘三個照 DFM（一起重建，值才對得上）。
 *      在引擎套值「之前」重建，引擎照後端 itemIndex 選。
 *  (2) lstTTL：golden ShowTTLState（:1238）填的 TTL 設定摘要（extra.lstTTL；通用 proxies 不帶 TListBox 的內容）。
 *  (3) golden 兩個畫面事件搬到瀏覽器（存檔時後端 BeforeApply 照 golden 重播、重新判斷可改）：
 *        rgInterfaceTypeClick（:1157）→ ShowPageControl2（:1192）：四個分頁只顯示選到的那一頁
 *        cbRs232TypeChange（:1312）：cbRs232Type 不是第 0 項才顯示 gbRs232BinCount
 *      切換後哪些存檔欄位可改＝後端照同一個 golden 狀態算好的表（extra.editable["<type>:<rs232>"]），
 *      這裡只照表開關（只動引擎關的 data-gb-dis，不碰本來就停用的）。golden rgInterfaceTypeClick 的 Barcode／bflag
 *      「改回原值」只在後端做（存檔後重讀會看到結果）。
 *  (4) 產生器把 DFM Visible=False 的元件寫成 display:none，引擎只改 visibility → 後端說 visible 的元件補回 display。
 *      頁籤的 TabVisible 也在這裡套：theme.js release 模式把 title 搬到 data-htitle，引擎 gbApply 用 [title^=] 找不到頁籤
 *      （實測 20260925：DIO 頁籤在 GPIB 配方下仍顯示）→ 兩種屬性都找；作用中的頁籤被藏掉就換到第一個看得見的。
 *
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.7 的 TI-1～TI-5 與 W6（Steven 20260927 W6＝A）：
 *  (5) TI-1 cbDIOTypeChange（golden 906_0625_Steven cTesterIF.cpp:1281-1287；912 :1299-1305）：換 DIO 檔就照後端開頁時先算好的
 *      extra.dioPreview[第幾項] 換 lstTTL 摘要（golden ShowTTLState 清單段的原文複製；後端 TIF_Q41_DioPreview）。執行期 LoadData 仍在
 *      存檔前重播（C 路只有開頁／存檔兩個入口，C-4 form.event 還沒有）。檔案不在＝golden LoadData 直接 return，清單照舊＋TTL 模式提示。
 *  (6) TI-3 btTesterTCPShowClick（906_0625_Steven cTesterIF.cpp:1516-1519；912 :1535-1538）fTesterTCP->Show()：開 background 的
 *      'testercomm' 視窗（testercomm.html 的 TCP/IP 分頁＝golden TfTesterTCP 的 32 站畫面）；分頁用 localStorage 'ht9045.testercomm.tab' 傳。
 *  (7) W6：GPIB 模式也露出 RS232 分頁，只有 pnlRS232 四個（cbbBaudRate／rgBitLength／rgStopBit／rgParity）可改，其他照舊停用；
 *      後端 extra.editable 的 GPIB 列多這四個、存檔 BeforeApply 自己套（刻意偏離 golden ShowPageControl2，見 TestIF_File_TesterIF.cpp 檔尾）。
 *      新的 RS232 選項（5／6 Bits、1.5 Bits、Mark／Space）在 Setup.TesterIF.html:56 的三組 radio 後面（index 同 C++ FileRW_TesterIF_Boot）。
 *  TI-2 AntiSignalCBoxClick：只改執行期 TestIF.bAntiSignal；存檔前照舊重播，關頁尾段的 DoStructUnitConvert 又把它設回檔案值 → 網頁不另做。
 *  TI-4／TI-5（FormClose＋主畫面 sbTesterClick 尾段）：後端存檔成功後跑（S88 目前做法），頁面不用改。
 *
 * golden spbSaveClick 沒有 YES/NO 確認框 → 不登錄引擎 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 客戶專屬條件：Steven 20260925 決定先跳過 —— 這裡不做任何客戶碼分支，可見／可改全由後端 golden FormShow 決定。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_TesterIF';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__testerIfCWrapped) return;
  R.__testerIfCWrapped = true;

  var PC2 = ['tsDio', 'tsGpib', 'tsRs232', 'tsTCPIP'];   // golden ShowPageControl2 :1194 tsTemp[]
  var get0 = R.editlistGet, save0 = R.editlistSave, write0 = R.write;
  var LAST = null;                                       // 最近一次 editlist.get 的回應
  var notes = [];

  function $(id) { return document.getElementById(id); }

  /* ---- (0) B 路寫入擋掉 ------------------------------------------------ */
  if (typeof write0 === 'function') {
    R.write = function (doc) {
      if (String(doc).toLowerCase() === 'tester') {
        return Promise.reject(new Error('Tester.Data 在這一頁由 C 路（golden TFTestIF 存檔流程，editlist.save ' + STRUCT + '）寫，不走 recipe.doc.put'));
      }
      return write0.apply(this, arguments);
    };
  }

  /* ---- (1) 下拉選項 ----------------------------------------------------- */
  function rebuildItems(d) {
    var items = d && d.extra && d.extra.items;
    if (!items) { notes.push('⚠ 後端回應沒有 extra.items（golden 執行期的下拉選項），cbDIOType 沿用網頁的佔位選項'); return; }
    Object.keys(items).forEach(function (id) {
      var el = $(id);
      if (!el || el.tagName !== 'SELECT') return;
      el.innerHTML = '';
      (items[id] || []).forEach(function (t, k) {
        var o = document.createElement('option');
        o.value = String(k);
        o.textContent = t;
        o.setAttribute('data-src', 'cpp-items');
        el.appendChild(o);
      });
      el.selectedIndex = -1;                 // 值等一下由引擎照後端 itemIndex 套
    });
    if (items.cbDIOType && !items.cbDIOType.length) notes.push('❌ DIOCFGPath 沒有任何 DIO 設定檔（golden InitcbDIOType 會結束程式）——這一頁存檔會被後端拒絕');
  }

  /* ---- (2) lstTTL ------------------------------------------------------- */
  function renderTTL(d) { renderLines((d && d.extra && d.extra.lstTTL) || []); }
  function renderLines(arr) {                      // AI(W906-Q41) 20260927: TI-1 共用（開頁的 lstTTL／換 DIO 檔的預覽）
    var el = $('lstTTL');
    if (!el) return;
    el.innerHTML = '';
    arr.forEach(function (t) {
      var row = document.createElement('div');
      row.style.whiteSpace = 'pre';
      row.textContent = t === '' ? ' ' : t;
      el.appendChild(row);
    });
  }

  /* ---- (3) ShowPageControl2／cbRs232TypeChange -------------------------- */
  // theme.js 在 release 模式把 title 搬到 data-htitle（去掉 hover 露出的 dfm 名）→ 兩個都找
  function tabOf(id) {
    return document.querySelector('.tab[title^="' + id + ' :"]') || document.querySelector('.tab[data-htitle^="' + id + ' :"]');
  }
  // 後端的 TabVisible（golden ShowPageControl2／FormShow 設的）。引擎 gbApply 只用 [title^=] 找頁籤，release 模式找不到 → 這裡補
  function applyTabs(d) {
    var px = (d && d.proxies) || {};
    Object.keys(px).forEach(function (id) {
      if (px[id].tabVisible === undefined) return;
      var t = tabOf(id);
      if (t) t.style.display = px[id].tabVisible ? '' : 'none';
    });
    // 每個 PageControl：作用中的頁籤被藏起來了 → 換到第一個看得見的（VCL 藏掉 ActivePage 時同樣會換頁）
    Array.prototype.forEach.call(document.querySelectorAll('.pcTabs'), function (bar) {
      var act = bar.querySelector(':scope > .tab.act');
      if (act && act.style.display !== 'none') return;
      var vis = Array.prototype.filter.call(bar.querySelectorAll(':scope > .tab'), function (x) { return x.style.display !== 'none'; });
      if (vis.length) vis[0].click();
    });
  }
  function rgIndex(id) {
    var el = $(id), rs = el ? el.querySelectorAll('input[type="radio"]') : [];
    for (var i = 0; i < rs.length; i++) if (rs[i].checked) return i;
    return -1;
  }
  function setEnabled(el, on) {                    // 同引擎 gbSetEnabled：只動自己關的
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if (!('disabled' in x)) return;
      if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
      else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
    });
    if (!on) el.setAttribute('aria-disabled', 'true'); else el.removeAttribute('aria-disabled');
  }
  function applyEditable() {
    var ed = LAST && LAST.extra && LAST.extra.editable;
    if (!ed) return;
    var cb = $('cbRs232Type');
    var key = rgIndex('rgInterfaceType') + ':' + (cb && cb.selectedIndex > 0 ? 1 : 0);
    var list = ed[key];
    if (!list) return;
    var on = {};
    list.forEach(function (n) { on[n] = true; });
    (LAST.mustSend || []).forEach(function (id) { var el = $(id); if (el) setEnabled(el, !!on[id]); });
  }
  function rs232TypeChange() {                      // golden cbRs232TypeChange :1312
    var cb = $('cbRs232Type'), g = $('gbRs232BinCount');
    if (g && cb) { g.style.visibility = cb.selectedIndex === 0 ? 'hidden' : ''; if (cb.selectedIndex !== 0 && g.style.display === 'none') g.style.display = ''; }
  }
  function showPageControl2(idx) {                  // golden ShowPageControl2 :1192
    if (idx < 0 || idx > 3) return;
    PC2.forEach(function (id, i) { var t = tabOf(id); if (t) t.style.display = (i === idx) ? '' : 'none'; });
    var t = tabOf(PC2[idx]);
    if (t && !t.classList.contains('act')) t.click();   // 頁面自己的分頁切換（PageControl2->ActivePage）
    rs232TypeChange();
    w6Rs232InGpib();                                // AI(W906-Q41) 20260927: W6（見 (7)）
  }

  /* ---- (5) AI(W906-Q41) 20260927 (St02-E): TI-1 golden cbDIOTypeChange 的摘要 ----------------------------- */
  function dioTypeChange() {                        // golden cTesterIF.cpp:1281（912 :1299）→ ShowTTLState 清單段（預覽）
    var pv = LAST && LAST.extra && LAST.extra.dioPreview, cb = $('cbDIOType');
    if (!pv || !cb) return;
    var p = pv[cb.selectedIndex];
    if (!p) return;
    if (p.missing) {                                // golden DIOInterFaceCFG.cpp:76-84：直接 return，TTLCfg 不變 → 清單照舊
      if (p.lostMsg) say2('訊息：DIO資料遺失，請檢查！！（Current DIO data has been lossed, please check!!）\n   golden 同時停機（SystemStart=false）；網頁版在存檔時照 golden 重播', '#f88');
      return;
    }
    renderLines(p.lines || []);
    if (p.ttlWarn && rgIndex('rgInterfaceType') === 0)   // golden cTesterIF.cpp:1261／:1269-1276（912 :1279／:1287-1294）
      say2('訊息：TTL 2 Site 以上不支援 8bit & 10bit 模式（TTL 8bit & 10bit only support less than 2 site!）');
  }

  /* ---- (6) AI(W906-Q41) 20260927 (St02-E): TI-3 golden btTesterTCPShowClick → fTesterTCP->Show() --------- */
  var TC_KEY = 'ht9045.testercomm.tab';             // testercomm.html 讀（同 origin 的 localStorage；它已開著時靠 storage 事件）
  function testerTcpShow() {
    var b = $('btTesterTCPShow');
    if (b && (b.disabled || b.getAttribute('aria-disabled') === 'true')) return;
    try { localStorage.setItem(TC_KEY, 'tcpip'); } catch (e) { /* 私密視窗等：開視窗但停在預設分頁 */ }
    if (window.parent && window.parent !== window) window.parent.postMessage({ open: 'testercomm' }, '*');   // background.html WINDOWS 表 id 'testercomm'
    else window.open('testercomm.html#tcpip', '_blank');                                                       // 單獨開這一頁時
  }

  /* ---- (7) AI(W906-Q41) 20260927 (St02-E): W6（Steven 20260927 W6＝A）GPIB 模式也能改 RS232 那組 ----------- */
  var W6 = ['cbbBaudRate', 'rgBitLength', 'rgStopBit', 'rgParity'];   // golden pnlRS232／gbBaudRate；同 C++ kQ41W6Rs232
  function isW6(x) { return W6.some(function (id) { var w = $(id); return w && (w === x || w.contains(x)); }); }
  function w6Rs232InGpib() {
    var gpib = rgIndex('rgInterfaceType') === 1;    // GPIB_MODE（cmydef.h:62）
    var t = tabOf('tsRs232');
    if (gpib && t) t.style.display = '';            // golden ShowPageControl2 只留 GPIB 分頁；W6 另外露出 RS232 分頁
    var pc = $('pgcRS232'), pane = pc && pc.closest ? pc.closest('.pcPane') : null;
    if (pane) Array.prototype.forEach.call(pane.querySelectorAll('input,select,button,textarea'), function (x) {
      if (isW6(x)) return;                          // 四個的可改由後端 extra.editable 決定（applyEditable／w6ApplyEditable）
      if (gpib) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-q41-dis', '1'); } }
      else if (x.getAttribute('data-q41-dis') === '1') { x.disabled = false; x.removeAttribute('data-q41-dis'); }
    });
    var host = $('pnlRS232'), note = $('q41W6Note');
    if (host && !note) {
      note = document.createElement('div');
      note.id = 'q41W6Note';
      note.style.cssText = 'position:absolute;left:11px;top:263px;right:4px;font-size:10px;color:#804000;white-space:nowrap;overflow:hidden;';
      note.textContent = 'GPIB 模式：這四項給 GPIB 程式的額外 RS232 port（裁決 W6）';
      host.appendChild(note);
    }
    if (note) note.style.display = gpib ? '' : 'none';
  }
  // 開頁：引擎照伺服器當下的 editable 套過（GPIB 模式 tsRs232 看不見 → 四個都關）；GPIB 列照矩陣把四個打開（只開不關）
  function w6ApplyEditable() {
    var ed = LAST && LAST.extra && LAST.extra.editable;
    if (!ed || rgIndex('rgInterfaceType') !== 1) return;
    var cb = $('cbRs232Type');
    var list = ed['1:' + (cb && cb.selectedIndex > 0 ? 1 : 0)] || [];
    W6.forEach(function (id) { var el = $(id); if (el && list.indexOf(id) >= 0) setEnabled(el, true); });
  }
  function hookEvents() {
    var rg = $('rgInterfaceType');
    if (rg && !rg.__tifC) {
      rg.__tifC = true;
      rg.addEventListener('change', function () { showPageControl2(rgIndex('rgInterfaceType')); applyEditable(); });
    }
    var cb = $('cbRs232Type');
    if (cb && !cb.__tifC) {
      cb.__tifC = true;
      cb.addEventListener('change', function () { rs232TypeChange(); applyEditable(); });
    }
    var dio = $('cbDIOType');                       // AI(W906-Q41) 20260927: TI-1
    if (dio && !dio.__tifC) { dio.__tifC = true; dio.addEventListener('change', dioTypeChange); }
    var tcp = $('btTesterTCPShow');                 // AI(W906-Q41) 20260927: TI-3
    if (tcp && !tcp.__tifC) { tcp.__tifC = true; tcp.addEventListener('click', testerTcpShow); }
  }

  /* ---- (4) 引擎套完之後 -------------------------------------------------- */
  function afterApply(d) {
    var px = d.proxies || {};
    Object.keys(px).forEach(function (id) {
      var el = $(id);
      if (el && px[id].visible === true && el.style.display === 'none') el.style.display = '';
    });
    applyTabs(d);
    hookEvents();
    w6Rs232InGpib(); w6ApplyEditable();             // AI(W906-Q41) 20260927: W6（applyTabs 照後端把 tsRs232 藏了之後再露出）
    if (d.extra && d.extra.dioListLost) notes.push('❌ golden InitcbDIOType 找不到 DIO 檔（golden 會結束程式）：這一頁存檔會被拒絕');
    if (notes.length) say2(notes.join('\n'));
    notes = [];
  }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[TesterIF/C] ' + msg);
  }

  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d;
      rebuildItems(d);                     // 引擎套值之前：選項要先在
      renderTTL(d);
      setTimeout(function () { afterApply(d); }, 0);   // 引擎在這個 promise 的 then 裡同步套完
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    return save0.apply(this, arguments).then(function (a) {
      var ev = (a && a.events) || [];               // AI(W906-Q41) 20260927: W6 套的四個不是 golden 事件，分開說
      var w6 = ev.filter(function (n) { return W6.indexOf(n) >= 0; }), gev = ev.filter(function (n) { return W6.indexOf(n) < 0; });
      if (gev.length) notes.push('ⓘ 存檔前照 golden 重播的事件：' + gev.join(', '));
      if (w6.length) notes.push('ⓘ W6（GPIB 模式也能改 RS232 那組，Steven 20260927）照頁面值寫入：' + w6.join(', '));
      return a;
    });
  };

  // 探針／除錯用
  window.HT9045TesterIfC = {
    state: function () {
      return { struct: STRUCT, loaded: !!LAST, type: rgIndex('rgInterfaceType'),
               rs232: ($('cbRs232Type') || {}).selectedIndex, lstTTL: LAST && LAST.extra ? (LAST.extra.lstTTL || []).length : 0,
               dioPreview: LAST && LAST.extra ? (LAST.extra.dioPreview || []).length : 0 };   // AI(W906-Q41) 20260927
    },
    showPageControl2: showPageControl2, applyEditable: applyEditable,
    dioTypeChange: dioTypeChange, testerTcpShow: testerTcpShow, w6Rs232InGpib: w6Rs232InGpib   // AI(W906-Q41) 20260927
  };
})();
