/* ht9045_cleaning_ev.js -- Setup.Cleaning.html（golden TfCleaning，V912 AutoClean\uCleaning.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB3) 20260928 [W906] St01 新檔（手寫）。批次 B3 的 CL-1／CL-2／CL-3／CL-5／CL-7
 *   （D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B3、第四節 CL 列）。
 *   Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。
 *   C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_Cleaning.cpp 檔尾「批次 B3」段（事件表 kCL_Events 83 列、跳板 EvB3Run）；
 *   設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_Cleaning.py 的 events。寫法照 St02 的 ht9045_config_q41.js :117-163
 *   （St02 分支 v906/steven-gpib-widget）與 B2 的 ht9045_trayassign_ev.js。引擎 ht9045_wire_engine.js、ht9045_cleaning_c.js 不改。
 *   頁面 Setup.Cleaning.html:126 同一行載入（在 ht9045_cleaning_c.js 之後），行數不變。
 *
 * 送什麼（WS cmd "form.event"，tag "TestIF_File_Cleaning"）：
 *   CL-1 btInclude            {"form":"TfCleaning","control":"btInclude","event":"click","state":{…}}
 *        golden btIncludeClick :2269 —— 用伺服器當下的 Loader 盤格式填 Clean Tray 欄位（ack.changed 帶 XST2／XPitch2／XCT2／YST2／YPitch2／YCT2…）
 *   CL-2 btnResetCleanCount   {"control":"btnResetCleanCount","event":"click",…}  golden :2116（計數歸零寫檔、解警報、SECS）
 *   CL-3 btnResetInterval     {"control":"btnResetInterval","event":"click",…}    golden :2842（Smart AC 間隔回初始、直接寫配方）
 *   CL-5 sbTrayAssign         {"control":"sbTrayAssign","event":"click",…}        golden :2311（只動主畫面；主畫面那一半是 B6，見 ack.todo "main:…"）
 *   CL-7 rgCleanKitType       {"control":"rgCleanKitType","event":"click","itemIndex":1,…}    golden :2214
 *        小鍵盤元件（XCT1、edPinSingleGf、edDevicePices…，伺服器 events 裡 event＝"click" 的輸入框）
 *                             {"control":"XCT1","event":"click","text":"7",…}    ＝操作員在小鍵盤按確定的值；伺服器照 golden 夾限、觸發 OnChange、重算
 *   state＝引擎存檔會送的那一包（HT9045Page.golden().kinds，讀法同引擎 gbValue），但事件表裡的元件一律不放
 *   （它們只能經由自己的事件同步：state 靜默改掉小鍵盤格子，之後存檔重播就不會觸發 OnChange —— 見 C++ 檔尾 ⚠）；
 *   另帶 pgCleanType 的 {"activePageIndex":n}（golden XCT1Change :2145 看使用者在 Kit 還是 Tray 頁）。存檔也補同一個分頁值。
 * 規則（同 St02 範本與 B2）：一次一個、等 ack 再送下一個；同一個元件同一個值還沒回覆又來一次＝連點，丟掉；busy:（WebCmdGuard 400 ms）
 *   等 450 ms 重送、最多 3 次；not-operator 先續權杖；錯誤含 "reload page" → 重開頁；events.<id>.operable＝false 不送；
 *   其他失敗（running、點不到、handler-failed）→ 值類元件改回點之前的值、狀態列說原因（存檔時伺服器照 golden 補重播沒送到的）。
 *   套 ack.changed 時不補發 change／input（VCL 程式設值不是使用者點的）。存檔時還有事件在等回覆 → 這次不存。
 *   ht9045_cleaning_c.js 標成「未移植」停用的四顆鈕（btInclude／btnResetCleanCount／btnResetInterval／sbTrayAssign）在伺服器說
 *   operable 時重新打開；btnStartAutoClean（CL-4，機台會動，B8）、cbbSelectTray（golden DFM 停用，R77）照舊。   ⛔ 更正 AI(W906-B8-CL4) 20261001 [W906] (St01)：btnStartAutoClean 已接（CL-4，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「CL-4」）—— 送 form.event click、伺服器說 operable 時打開（PORTED）、ack 後說明 golden 做了什麼（afterAck）。
 * AI(W906-D013) 20260929 [W906] St01：R126（todo D-013，decisions-decided R126「20260929 結果」：照 BCB 做網頁的刷條碼框）——
 *   KYEC 刷條碼機台（CC_KYEC_LEE／CC_KYEC_XILINX＋USE_BARCODE_AS_KEYBOARD＋bBarcodeReader）按 Reset（btnResetCleanCount）時，golden
 *   先跳「Barcode Reader」框（V912 AutoClean\uCleaning.cpp:2118 Barcode_Reader(bcAutoClean)，BarcodeReader.cpp／.dfm），刷到操作員 ID
 *   才歸零。伺服器照 golden 開框時，form.event 的 ack 多一個 barcode 陣列（{caption,inputType,scanned,accepted}，
 *   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp 檔尾）；有一個 accepted=false ⇒ 這一次 golden 在框那一行就 return 了（沒歸零），
 *   這裡跳同樣的框（標題 Barcode Reader、標籤＝caption、一格輸入、Enter 鈕、右上角關閉），操作員刷完按 Enter ⇒ 再送同一個事件並帶
 *   "barcode":"<刷到的字>"，伺服器照 golden 的框程式（TimerKeyIn 檢查＋btnEnterClick＋FormClose）跑那段字。刷到的字被 golden 擋掉
 *   （scanned && !accepted）⇒ 框再開一次、說明原因（golden 是框不關、字被清掉）。按關閉＝golden「沒刷就關」＝不歸零、什麼都不送。
 *   照 golden：Enter 鍵不送（DFM 的 btnEnter 不是 Default，條碼槍送的 CR 按不到它）、少於 4 個字按 Enter 不關（btnEnterClick :32-44）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb3Cleaning) return;
  R.__evb3Cleaning = true;

  var STRUCT = 'TestIF_File_Cleaning';      // editlist.get／editlist.save／form.event 的 tag（FileRW/TestIF_File_Cleaning.cpp g_page）
  var FORM = 'TfCleaning';                  // golden 表單類別（form.event 的 form；不同 ⇒ bad-payload）
  var NAME = 'Cleaning';                    // 狀態列訊息用的頁名
  var LOG = '[Cleaning/EVB3] ';
  var PC = 'pgCleanType';                   // golden TPageControl（Kit／Tray）
  var MAX_WS = 60000;                       // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage），留一點給外框
  var PORTED = {                            // ht9045_cleaning_c.js NOT_PORTED 裡、這一批接上的
    btInclude: 'CL-1 golden btIncludeClick（:2269）：用 Loader 盤格式填 Clean Tray 欄位（伺服器照 golden 算）',
    btnResetCleanCount: 'CL-2 golden btnResetCleanCountClick（:2116）：清潔計數歸零並存檔、解除計數警報、送 SECS AutoCleanClearCount',
    btnResetInterval: 'CL-3 golden btnResetIntervalClick（:2842）：Smart Auto Clean 間隔回初始並直接寫配方',
    sbTrayAssign: 'CL-5 golden sbTrayAssignClick（:2311）：切主畫面 Motion View 並顯示 Auto Clean 計數格（主畫面那一半還沒接，B6）', btnStartAutoClean: 'CL-4 golden btnStartAutoCleanClick（:2901）→ TfShowBinSelect::btnAutoCleanClick（cShowBinSelect.cpp:2248）：登記一次 Auto Clean，按 START 之後機台才清潔（沒料時要先歸零、Tray arm／Index 在安全位）'   // AI(W906-B8-CL4) 20261001 [W906] (St01)：同一行附加
  };

  var LAST = null;                          // 最近一次 editlist.get 的回應
  var GEN = 0;                              // 每次開頁 +1：重開頁之前送出去的事件，回覆不再套
  var QUEUE = [], INFLIGHT = null, APPLYING = false, KNOWN = {}, OFF = '';

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info(LOG + msg);
  }
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { return m; } }
    return m;
  }
  function utf8Len(s) {
    var n = 0;
    for (var i = 0; i < s.length; i++) {
      var c = s.charCodeAt(i);
      if (c < 0x80) n += 1; else if (c < 0x800) n += 2; else if (c >= 0xD800 && c <= 0xDBFF) { n += 4; i++; } else n += 3;
    }
    return n;
  }
  function ancestorDisabled(el) {
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function hidden(el) {
    for (var p = el; p && p.nodeType === 1; p = p.parentElement) {
      var cs = window.getComputedStyle(p);
      if (cs.display === 'none' || cs.visibility === 'hidden') return true;
    }
    return false;
  }
  function usable(el) {                                  // 引擎／權限停用、藏起來的元件（自己或上層）一樣按不到
    if (!el || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el) || hidden(el)) return false;
    var x = (el.tagName === 'INPUT' || el.tagName === 'SELECT' || el.tagName === 'BUTTON') ? el : el.querySelector('input,select');
    return !x || !x.disabled;
  }
  function radiosOf(el) { return el ? el.querySelectorAll('input[type="radio"]') : []; }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function evIds() { var e = LAST && LAST.events; return (e && typeof e === 'object') ? Object.keys(e) : []; }
  function canSend(id, evn) { var e = evInfo(id); return !!e && e.event === evn && e.operable !== false; }

  /* ---- 元件種類與值 ------------------------------------------------------- */
  function kindOf(el) {
    if (!el) return '';
    if (el.tagName === 'SELECT') return 'select';
    if (el.tagName === 'TEXTAREA') return 'text';
    if (el.tagName === 'INPUT') return (el.type === 'checkbox' || el.type === 'radio') ? 'check' : 'text';
    if (radiosOf(el).length) return 'radio';
    if (el.querySelector && el.querySelector('input[type="checkbox"]')) return 'check';
    return 'button';
  }
  function goldenIndex(id, sel) {                        // <select> → golden ItemIndex／Text（對 events.<id>.items）
    var o = sel.options[sel.selectedIndex];
    if (!o) return { itemIndex: -1, text: '' };
    var t = o.textContent, src = o.getAttribute('data-src');
    if (src === 'cpp-text') return { itemIndex: -1, text: t };
    var e = evInfo(id), items = e && Array.isArray(e.items) ? e.items : null;
    if (items) return { itemIndex: items[sel.selectedIndex] === t ? sel.selectedIndex : items.indexOf(t), text: t };
    return { itemIndex: sel.selectedIndex, text: t };
  }
  function readCtl(id) {
    var el = $(id), k = kindOf(el);
    if (k === 'text') return { text: String(el.value) };
    if (k === 'select') { var g = goldenIndex(id, el); g.dom = el.selectedIndex; return g; }
    if (k === 'radio') { var rs = radiosOf(el); for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i }; return { itemIndex: -1 }; }
    if (k === 'check') { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]'); return { checked: !!(c && c.checked) }; }
    return k === 'button' ? {} : null;
  }
  function writeCtl(id, v) {                             // 改回點之前的值（不發事件）
    var el = $(id), k = kindOf(el);
    if (!el || !v) return;
    APPLYING = true;
    try {
      if (k === 'text' && v.text !== undefined) el.value = v.text;
      else if (k === 'select' && v.dom !== undefined) el.selectedIndex = v.dom;
      else if (k === 'radio' && v.itemIndex !== undefined) { var rs = radiosOf(el); for (var i = 0; i < rs.length; i++) rs[i].checked = (i === v.itemIndex); }
      else if (k === 'check' && v.checked !== undefined) { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]'); if (c) c.checked = !!v.checked; }
    } finally { APPLYING = false; }
  }
  function payloadOf(v) {
    var p = {};
    if (!v) return p;
    ['itemIndex', 'text', 'checked'].forEach(function (k) { if (v[k] !== undefined) p[k] = v[k]; });
    return p;
  }
  function gbValueOf(id, k) {                            // 同引擎 gbValue（ht9045_wire_engine.js，Jimmy 的檔，沒有匯出）
    var el = $(id);
    if (!el || !k) return null;
    if (k === 'checked') { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); return c ? { checked: c.checked } : null; }
    if (k === 'itemIndex') {
      if (el.tagName === 'SELECT') {
        var o = el.options[el.selectedIndex];
        if (o && o.getAttribute('data-src') === 'cpp-text') return { itemIndex: -1, text: o.textContent };
        return { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
      }
      var rs = radiosOf(el);
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
      return { itemIndex: -1 };
    }
    if (k === 'text') return { text: String(el.value) };
    if (k === 'position') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }
    return null;                                         // dateTime／cells／tag：這一頁的處理器不讀，不放進 state
  }
  function pcSupported() { var p = LAST && LAST.proxies && LAST.proxies[PC]; return !!p && typeof p.activePageIndex === 'number'; }
  function pageTab() {                                   // 頁面上 pgCleanType 目前的頁籤（data-t＝golden PageIndex）
    var pc = $(PC), act = pc && pc.querySelector(':scope > .pcTabs > .tab.act');
    var n = act ? parseInt(act.getAttribute('data-t'), 10) : NaN;
    return isNaN(n) ? null : n;
  }
  function stateNow(self) {                              // 見檔頭 state
    var g = (window.HT9045Page && HT9045Page.golden) ? HT9045Page.golden() : null, kinds = (g && g.kinds) || {};
    var ev = (LAST && LAST.events) || {}, out = {};
    Object.keys(kinds).forEach(function (id) {
      if (id === self || ev[id]) return;
      var v = gbValueOf(id, kinds[id]);
      if (v) out[id] = v;
    });
    var n = pageTab();
    if (n !== null && pcSupported()) out[PC] = { activePageIndex: n };
    return out;
  }

  /* ---- 套 ack.changed ---------------------------------------------------- */
  function setVis(el, on) {
    el.style.visibility = on ? '' : 'hidden';
    if (on && el.style.display === 'none' && !el.classList.contains('pcPane')) el.style.display = '';
  }
  function setEditable(el, on) {                         // 同引擎 gbSetEnabled：只動自己關的；打開時看上層
    if (on && ancestorDisabled(el)) return;
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if (!('disabled' in x)) return;
      if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
      else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
    });
    if (!on) el.setAttribute('aria-disabled', 'true'); else el.removeAttribute('aria-disabled');
  }
  function tabOf(id) {
    var el = $(id);
    if (el && el.classList && el.classList.contains('pcPane')) {
      var wrap = el.parentElement && el.parentElement.parentElement;
      return wrap ? wrap.querySelector(':scope > .pcTabs > .tab[data-t="' + el.getAttribute('data-p') + '"]') : null;
    }
    return document.querySelector('.tab[data-htitle^="' + id + ' :"]') || document.querySelector('.tab[title^="' + id + ' :"]');
  }
  function setTabVisible(id, on) {
    var tab = tabOf(id);
    if (!tab) return;
    tab.style.display = on ? '' : 'none';
    if (!on && tab.classList.contains('act')) {
      var first = tab.parentElement.querySelector(':scope > .tab:not([style*="display: none"])');
      if (first && first !== tab) first.click();
    }
  }
  function clickTab(pcId, n) {
    var pc = $(pcId), tab = pc && pc.querySelector(':scope > .pcTabs > .tab[data-t="' + n + '"]');
    if (tab && !tab.classList.contains('act')) tab.click();
  }
  function setItems(el, items) {
    var keep = el.selectedIndex >= 0 && el.options[el.selectedIndex] ? el.options[el.selectedIndex].textContent : null;
    while (el.options.length) el.remove(0);
    items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; o.setAttribute('data-src', 'ev-items'); el.appendChild(o); });
    el.selectedIndex = keep === null ? -1 : items.indexOf(keep);
  }
  function setIndex(el, i, text) {
    if (el.tagName === 'SELECT') {
      var olds = el.querySelectorAll('option[data-src="cpp-text"]');
      for (var k = 0; k < olds.length; k++) olds[k].parentNode.removeChild(olds[k]);
      if (i >= 0 && i < el.options.length && (text === undefined || el.options[i].textContent === text)) { el.selectedIndex = i; return; }
      if (text === undefined) return;
      for (var j = 0; j < el.options.length; j++) if (el.options[j].textContent === text) { el.selectedIndex = j; return; }
      var o = document.createElement('option'); o.textContent = text; o.setAttribute('data-src', 'cpp-text');
      el.appendChild(o); el.selectedIndex = el.options.length - 1;
      return;
    }
    var rs = radiosOf(el);
    for (var r = 0; r < rs.length; r++) rs[r].checked = (r === i);
  }
  function applyChanged(ch) {                            // {元件:{text,itemIndex,checked,items,position,caption,visible,enabled,editable,tabVisible,activePageIndex}}
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id] || {}, el = $(id);
        if (v.tabVisible !== undefined) setTabVisible(id, !!v.tabVisible);
        if (!el) return;
        if (v.items !== undefined && el.tagName === 'SELECT' && Array.isArray(v.items)) setItems(el, v.items);
        if (v.itemIndex !== undefined) setIndex(el, v.itemIndex, v.text);
        else if (v.text !== undefined && el.tagName === 'SELECT') setIndex(el, -1, v.text);
        else if (v.text !== undefined && 'value' in el && !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) el.value = String(v.text);
        if (v.checked !== undefined) { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); if (c) c.checked = !!v.checked; }
        if (v.position !== undefined) el.value = v.position;
        if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;
        if (v.activePageIndex !== undefined) clickTab(id, v.activePageIndex);
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
        var e = evInfo(id);                              // 點得到嗎跟著伺服器改（小鍵盤格有 ReadOnly：editable＝false 仍點得到，只看 enabled／visible）
        if (e) {
          if (v.enabled === false || v.visible === false) e.operable = false;
          else if (v.editable !== undefined && kindOf(el) !== 'text') e.operable = !!v.editable;
        }
      });
      dis.forEach(function (el) { setEditable(el, false); });
    } finally { APPLYING = false; }
  }

  /* ---- R126 網頁刷條碼框（見檔頭 AI(W906-D013)） ------------------------------- */
  var BOX = null;                                        // 開著的框（一次一個，golden 是 modal）
  function barcodeAsk(a) {                               // ack.barcode 裡第一個沒過的框（golden 在那一行 return）
    var bs = a && Array.isArray(a.barcode) ? a.barcode : [];
    for (var i = 0; i < bs.length; i++) if (bs[i] && bs[i].accepted === false) return bs[i];
    return null;
  }
  function closeBox() { if (BOX && BOX.parentNode) BOX.parentNode.removeChild(BOX); BOX = null; }
  function openBarcodeBox(item, ask) {                   // golden BarcodeReader.dfm：Color 6682876、lblInputType 字色 11359005
    closeBox();
    var ov = document.createElement('div');
    ov.setAttribute('data-ht-barcode', '1');
    ov.style.cssText = 'position:fixed;left:0;top:0;right:0;bottom:0;z-index:2147483000;background:rgba(0,0,0,.35);' +
                       'display:flex;align-items:center;justify-content:center;font-family:"MS Sans Serif",Tahoma,sans-serif';
    var win = document.createElement('div');
    win.setAttribute('role', 'dialog');
    win.setAttribute('aria-modal', 'true');
    win.style.cssText = 'width:368px;max-width:calc(100vw - 32px);background:#d4d0c8;border:1px solid #404040;box-shadow:2px 2px 8px rgba(0,0,0,.5)';
    var bar = document.createElement('div');
    bar.style.cssText = 'display:flex;align-items:center;justify-content:space-between;background:#0a246a;color:#fff;font-size:12px;padding:2px 4px';
    var ttl = document.createElement('span'); ttl.textContent = 'Barcode Reader';
    var x = document.createElement('button'); x.type = 'button'; x.textContent = '✕'; x.title = '關閉（golden：沒刷就關 ⇒ 不歸零）';
    x.style.cssText = 'font-size:11px;line-height:12px;padding:0 4px;cursor:pointer';
    bar.appendChild(ttl); bar.appendChild(x);
    var pnl = document.createElement('div');
    pnl.style.cssText = 'background:#fcf865;border:2px inset #fff;padding:12px 24px 10px;font-size:19px';
    var lbl = document.createElement('div'); lbl.textContent = ask.caption || 'Input Operator ID:'; lbl.style.cssText = 'color:#1d53ad;margin-bottom:6px';
    var inp = document.createElement('input');
    inp.type = ask.inputType === 'Password' ? 'password' : 'text';   // golden InputBarcodeNumber :128-131 PasswordChar='*'
    inp.autocomplete = 'off';
    inp.style.cssText = 'width:100%;box-sizing:border-box;font-size:19px;padding:2px 4px';
    var note = document.createElement('div'); note.style.cssText = 'font-size:12px;color:#a00000;min-height:15px;margin-top:4px';
    if (ask.scanned) note.textContent = '上一次刷到的條碼沒有通過 golden 的檢查（KYEC 條碼：6 或 7 碼、前 2／3 碼要在範圍內），請重刷';
    var ent = document.createElement('button'); ent.type = 'button'; ent.textContent = 'Enter';
    ent.style.cssText = 'display:block;margin:6px auto 0;width:193px;height:36px;font-size:19px;cursor:pointer';
    pnl.appendChild(lbl); pnl.appendChild(inp); pnl.appendChild(note); pnl.appendChild(ent);
    win.appendChild(bar); win.appendChild(pnl); ov.appendChild(win);
    document.body.appendChild(ov);
    BOX = ov;
    setTimeout(function () { try { inp.focus(); } catch (e) {} }, 0);   // golden FormShow :52 edtBarcodeNumber->SetFocus()
    x.addEventListener('click', function () {
      closeBox();
      say2(NAME + ' ' + item.control + '：沒有刷條碼就關掉了（golden：刷條碼框沒輸入 ⇒ Barcode_Reader 回 0 ⇒ 這一下不做）', '#ffcc66');
    });
    ent.addEventListener('click', function (ev) {
      if (ev && ev.isTrusted === false) return;
      var t = String(inp.value);
      if (ask.inputType !== 'RunMode' && t.length < 4) {  // golden btnEnterClick :32-44：少於 4 個字 return（框不關）
        note.textContent = '至少要 4 個字（golden btnEnterClick）';
        return;
      }
      closeBox();
      if (item.gen !== GEN) { say2(NAME + '：頁面已重新開過，這一次刷的條碼沒有送', '#ffcc66'); return; }
      say2(NAME + ' ' + item.control + '：已刷條碼，照 golden 再送一次（伺服器照 golden 的框檢查這段字）');
      enqueue({ control: item.control, event: item.event, val: item.val, prev: item.prev, gen: item.gen, barcode: t });
    });
  }

  /* ---- 佇列 -------------------------------------------------------------- */
  function latestFor(id) {
    for (var i = QUEUE.length - 1; i >= 0; i--) if (QUEUE[i].control === id) return QUEUE[i];
    return INFLIGHT && INFLIGHT.control === id ? INFLIGHT : null;
  }
  function pendingFor(id) { return QUEUE.some(function (q) { return q.control === id; }); }
  function refreshKnown() { evIds().forEach(function (id) { if (!pendingFor(id) && $(id)) KNOWN[id] = readCtl(id); }); }
  function idle() { return !INFLIGHT && !QUEUE.length; }
  function fail(item, msg) {
    QUEUE = QUEUE.filter(function (q) { return q.control !== item.control; });
    if (item.gen === GEN && item.prev) { writeCtl(item.control, item.prev); KNOWN[item.control] = item.prev; }
    say2(NAME + ' ' + item.control + ' 沒有送到伺服器（form.event）：' + msg +
         (item.prev && Object.keys(payloadOf(item.prev)).length ? ' —— 已改回點之前的值' : ''), '#f88');
  }
  function afterAck(item, a) {                           // 各鈕 ack 之後的頁面那一半
    var todo = (a && a.todo) || [];
    if (item.control === 'sbTrayAssign' && todo.some(function (t) { return /^main:AutoCleanStringGrid\b/.test(t); })) {
      say2('Tray Assign：伺服器已照 golden 跑（主畫面 AutoCleanStringGrid 設成顯示）；切到 Motion View、顯示 Auto Clean 計數格（golden MainFormSizeToEpson(false)，uCleaning.cpp:2313）'); try { localStorage.setItem('ht9045-mv-autocleangrid', String(Date.now())); } catch (e) {} try { new BroadcastChannel('ht9045-main-ev').postMessage({ type: 'autoCleanGrid', visible: true }); } catch (e) {} if (window.parent !== window) window.parent.postMessage({ open: 'motionview' }, '*');   // AI(W906-EVB6) 20260928 [W906]: CL-5 主畫面那一半（Motion View 頁的 ht9045_main_st01_ev.js 收）；同一行
    }
    if (item.control === 'btnResetCleanCount' || item.control === 'btnResetInterval') {
      say2(NAME + ' ' + item.control + '：伺服器已照 golden 跑完（' + (item.control === 'btnResetCleanCount' ? '清潔計數歸零並存檔' : 'Smart Auto Clean 間隔回初始') + '）', '#9f9');
    }  if (item.control === 'btnStartAutoClean') { var ms = (a && a.messages) || []; say2(NAME + ' Start Auto Clean：' + (ms.length ? 'golden 擋下、沒有排 Auto Clean —— ' + ms.map(function (x) { return x.zh || x.en; }).join('；') : '伺服器已照 golden btnAutoCleanClick（cShowBinSelect.cpp:2248）跑完。機台沒料：已排一次 Auto Clean，按主畫面 START 之後 Index／Shuttle／清潔 Kit 才會動；機台有料：先 One Cycle 清機、清完再清潔。golden 在已經在清潔、One Cycle 中、配方沒開 Auto Clean 或模式沒勾手動時不出聲、什麼都不做 —— 結果看 wb_serve 主控台 [B8-CL4] 那一行'), ms.length ? '#f88' : '#9f9'); }   // AI(W906-B8-CL4) 20261001 [W906] (St01)：CL-4 ack 之後的說明（golden 的擋下訊息在 ack.messages）；同一行
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    if (typeof item.barcode === 'string') v.barcode = item.barcode;   // AI(W906-D013) 20260929 [W906]：R126 操作員在網頁框刷到的字
    v.state = stateNow(item.control);                    // 送出當下取（前一個 ack 已套上）
    var extra = { tag: STRUCT, value: JSON.stringify(v) };
    if (utf8Len(JSON.stringify({ type: 'cmd', id: 999999, cmd: 'form.event', tag: extra.tag, value: extra.value })) > MAX_WS) {
      fail(item, '這次要送的畫面值超過伺服器單則上限 64 KB');
      return Promise.resolve(null);
    }
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a.todo && a.todo.length && window.console) console.info(LOG + item.control + ' todo: ' + a.todo.join(' | '));
      var ask = barcodeAsk(a);                           // AI(W906-D013) 20260929 [W906]：R126 golden 在刷條碼框那一行 return ⇒ 跳框（見檔頭）
      if (ask) {
        say2(NAME + ' ' + item.control + '：golden 要先刷條碼（' + (ask.caption || 'Input Operator ID:') + '，KYEC 刷條碼機台）' +
             (ask.scanned ? ' —— 上一次刷的沒有通過，請重刷' : '') + '；刷完按 Enter 才會做', '#ffcc66');
        openBarcodeBox(item, ask);
      } else {
        afterAck(item, a);
      }
      refreshKnown();
      return a;
    }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(item, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(item, tries + 1); }, function () { fail(item, msg); return null; });
      }
      if (/reload page/.test(msg)) {
        QUEUE = [];
        say2(NAME + '：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      if (/unknown cmd/i.test(msg)) OFF = msg;
      fail(item, msg);
      return null;
    });
  }
  function pump() {
    if (INFLIGHT || !QUEUE.length) return;
    INFLIGHT = QUEUE.shift();
    var done = function () { INFLIGHT = null; pump(); };
    try { send(INFLIGHT, 0).then(done, done); } catch (x) { if (window.console) console.error(LOG + 'send', x); done(); }
  }
  function enqueue(item) {
    var last = latestFor(item.control);
    if (last && last.event === item.event && JSON.stringify(last.val) === JSON.stringify(item.val) && last.barcode === item.barcode) return false;   // 連點：同一個值還沒回覆（AI(W906-D013) 20260929：刷過條碼的那一次不算連點）
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function onUser(id) {
    return function (ev) {
      if (APPLYING) return;
      var el = $(id), k = kindOf(el), e = evInfo(id);
      if (!el || !e) return;
      if (k !== 'text' && k !== 'button' && ev && ev.isTrusted === false) return;   // 別的程式 dispatch 的 change（小鍵盤的補發只在輸入框）
      var cur = readCtl(id), prev = KNOWN[id];
      if (OFF || !canSend(id, e.event) || (k === 'button' && !usable(el))) {
        if (k !== 'button') {
          writeCtl(id, prev);
          say2(NAME + ' ' + id + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）' : '伺服器說這個元件現在點不到（events.operable=false）') +
               ' —— 已改回（存檔時伺服器照 golden 處理）', '#f88');
        }
        return;
      }
      if (k !== 'button' && prev && cur && JSON.stringify(payloadOf(prev)) === JSON.stringify(payloadOf(cur))) return;   // 值沒變（golden 不會 OnChange／OnClick）
      if (enqueue({ control: id, event: e.event, val: payloadOf(cur), prev: prev, gen: GEN })) KNOWN[id] = cur;
    };
  }
  function hook() {
    evIds().forEach(function (id) {
      var el = $(id);
      if (!el || el.__evb3) return;
      el.__evb3 = true;
      el.addEventListener(kindOf(el) === 'button' ? 'click' : 'change', onUser(id));
    });
  }
  function reopenPorted() {                              // ht9045_cleaning_c.js markNotPorted 停用的四顆：伺服器說點得到就打開
    Object.keys(PORTED).forEach(function (id) {
      var el = $(id), e = evInfo(id);
      if (!el) return;
      if (e && e.operable !== false && !OFF) {
        el.disabled = false;
        el.removeAttribute('data-gb-dis');
        el.style.opacity = '';
        el.title = id + '：' + PORTED[id];
      }
    });
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = ''; closeBox();   // AI(W906-D013) 20260929 [W906]：R126 重開頁＝golden 表單重開，刷條碼框收掉
      setTimeout(function () {                           // ht9045_cleaning_c.js 的 setTimeout（停用未移植鈕）先跑，這裡排在它後面
        try { hook(); reopenPorted(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (BOX) return Promise.reject(new Error(NAME + ' 的刷條碼框還開著（golden 是 modal）—— 這次沒有存檔；先刷完或關掉那個框'));   // AI(W906-D013) 20260929 [W906]：R126
    if (!idle()) return Promise.reject(new Error(NAME + ' 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個事件在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    if (widgets && typeof widgets === 'object' && pcSupported() && !widgets[PC]) {   // 存檔重播（XCT1Change）看使用者在哪一頁
      var n = pageTab();
      if (n !== null) {
        var w2 = {};
        Object.keys(widgets).forEach(function (k) { w2[k] = widgets[k]; });
        w2[PC] = { activePageIndex: n };
        widgets = w2;
      }
    }
    return save0.call(this, st, widgets, answers, extra);
  };

  window.HT9045EvB3Cleaning = {                          // 探針／除錯用
    state: function () { return stateNow(''); }, queue: function () { return QUEUE.slice(); },
    inflight: function () { return INFLIGHT; }, known: function () { return KNOWN; }, applyChanged: applyChanged, pageTab: pageTab
  };
})();
