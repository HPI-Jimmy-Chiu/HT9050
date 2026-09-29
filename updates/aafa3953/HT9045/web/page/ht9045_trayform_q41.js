/* ht9045_trayform_q41.js -- Setup.TrayForm.html（golden TfTrayForm，cTrayForm.cpp）Q41 的頁面事件補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.2 的 TF-2／TF-3。St02 的新檔（手寫，不是 gen_wire.py 產物）；
 * 頁面 Setup.TrayForm.html:121 同一行載入，在接線資料檔之後。引擎 ht9045_wire_engine.js 不改。
 * golden 行號：906_0625_Steven 與 912 的 cTrayForm.cpp 這幾支行號相同（912 spbCopyClick 多 1 行，見下）。
 *
 *  TF-2  XCT1MouseDown（:499-527）／YCT1MouseDown（:529-557）：數量欄（XCT1..3／YCT1..3，Tag 0..2）小鍵盤提交後，數量是 "1" 就把
 *        同一組的 XPitchN／YPitchN 設 "0"，並更新那一組的 TMyTray 格數（XItem／YItem）。DFM 的 XBStart／YBStart／XBItem 也綁
 *        XCT1MouseDown，但 Tag=4，golden 三個分支都不進，所以不接。golden 在小鍵盤關掉後才看 Text；網頁小鍵盤提交時引擎補發 change，
 *        這裡聽 change（取消小鍵盤不會有事件 —— golden 取消時仍會用舊值檢查一次，數量本來就是 "1" 時 Pitch 會被設 0，網頁不做這一次）。
 *        XPitch1Click（:1022-1072）／YPitch1Click（:1080-1130）：只有 XPitch1、YPitch1 在 DFM 綁了（:469、:499；XPitch2／3 沒綁，照翻）。
 *        XCT1（YCT1）是 "1" 時不開小鍵盤、直接設 "0"。golden 三個 Tag 分支都拿 XCT1／YCT1 判斷（看起來應該是 XCT2／XCT3），
 *        但只有 Tag 0 的元件有綁，所以結果一樣。CC_SCC 的例外（可以改成 0.01）是客戶專屬，Steven 20260925 決定先跳過 → 不做。
 *        開頁：golden DoIniDataToForm（:330-335）把三組 TMyTray 的 XItem／YItem 設成檔案的 X／Y Division ＝ 這裡用開頁的 XCT／YCT 值畫。
 *  TF-3  PageControl1Change（:455）→ UpDateType（:460-467）：cbCopyFrom 列出「目前這一頁以外」的兩個 Type（依 ActivePage->Tag；
 *        TabSheet1..3 的 Tag＝0..2，其他頁 Tag＝0，照翻）。FormShow（:149-150）開在 TabSheet1。
 *        spbCopyClick（:692-714；912 :693-714）：把 cbCopyFrom 選的那個 Type 的前 9 個欄位（TrayEdit[i][0..8]：TrayName、XST、YST、
 *        XPitch、YPitch、TpThick、XCT、YCT、TpTickUp，建構子 :35-40）抄到 ActivePageIndex 那一組。golden 在 Bin Box 等頁按 Copy 時
 *        ActivePageIndex＝3..5 會超出 TrayEdit[3][17]（看起來是 golden 的越界）；網頁不抄，寫在狀態列。
 *        Barcode_Reader（KYEC 刷條碼）客戶專屬，跳過。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'UserDefForm_File';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || R.__trayFormQ41Wrapped) return;
  R.__trayFormQ41Wrapped = true;

  var TYPES = ['Type1', 'Type2', 'Type3'];                              // golden :463 S[3]
  var TRAY_EDIT = ['TrayName', 'XST', 'YST', 'XPitch', 'YPitch', 'Tp#Thick', 'XCT', 'YCT', 'Tp#TickUp'];   // golden :37-39 前 9 欄
  var TAB_TAG = { TabSheet1: 0, TabSheet2: 1, TabSheet3: 2 };          // DFM :242／:4022／:7748（其他頁 Tag 0）

  function $(id) { return document.getElementById(id); }
  function editId(col, n) { return col.indexOf('#') >= 0 ? col.replace('#', String(n)) : col + n; }   // n＝1..3
  function fire(el) {                                                   // 同引擎小鍵盤 onCommit：補發 input／change
    ['input', 'change'].forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function say2(msg) { if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, '#ffcc66', 'transient'); console.info('[TrayForm/Q41] ' + msg); }

  /* ---- TMyTray 格數（golden TMyTrayN->XItem／YItem） --------------------- */
  function trayItems(n, xitem, yitem) {
    var ph = $('TMyTray' + n);
    if (!ph || !window.HTWidgets || !HTWidgets.makeMyTray) return;
    var s; try { s = JSON.parse(ph.getAttribute('data-tray') || '{}'); } catch (e) { return; }
    if (xitem !== null) s.xitem = xitem;
    if (yitem !== null) s.yitem = yitem;
    ph.setAttribute('data-tray', JSON.stringify(s));
    if (!(s.xitem >= 1) || !(s.yitem >= 1)) return;                    // atoi 出 0：golden 畫不出格子，這裡保留舊圖
    var cw = Math.max(4, Math.floor((s.w - 10) / s.xitem)), chh = Math.max(4, Math.floor((s.h - 10) / s.yitem));
    var t = HTWidgets.makeMyTray({ name: s.name + '_t', xitem: s.xitem, yitem: s.yitem, xblockItem: s.xblockItem, yblockItem: s.yblockItem,
                                   cellW: cw, cellH: chh, trayColor: s.trayColor, trayDirect: s.trayDirect });   // 同頁面 inline script 的建法
    while (ph.firstChild) ph.removeChild(ph.firstChild);
    ph.appendChild(t);
  }
  function atoi(v) { var n = parseInt(String(v || ''), 10); return isNaN(n) ? 0 : n; }

  /* ---- TF-2 ------------------------------------------------------------- */
  function ctChange(axis, n) {                                           // golden XCT1MouseDown／YCT1MouseDown 的 Tag 0..2 分支
    var ed = $(axis + 'CT' + n);
    if (!ed) return;
    if (ed.value === '1') {
      var p = $(axis + 'Pitch' + n);
      if (p && p.value !== '0') { p.value = '0'; fire(p); }
    }
    if (axis === 'X') trayItems(n, atoi(ed.value), null); else trayItems(n, null, atoi(ed.value));
  }
  function pitchIntercept(ev) {                                          // golden XPitch1Click／YPitch1Click（只有 Tag 0 那顆有綁）
    var t = ev.target, axis = t && t.id === 'XPitch1' ? 'X' : (t && t.id === 'YPitch1' ? 'Y' : null);
    if (!axis || t.disabled) return;
    var ct = $(axis + 'CT1');
    if (!ct || ct.value !== '1') return;                                 // 不是 1 → 照常開小鍵盤（引擎）
    ev.preventDefault();
    ev.stopPropagation();                                                // 不讓引擎的 mousedown 開小鍵盤（golden 不開）
    if (t.value !== '0') { t.value = '0'; fire(t); }
  }

  /* ---- TF-3 ------------------------------------------------------------- */
  function activeTab() {
    var pc = $('PageControl1'), bar = pc && pc.querySelector(':scope > .pcTabs');
    var act = bar && bar.querySelector(':scope > .tab.act');
    if (!act) return { index: 0, tag: 0 };
    var title = act.getAttribute('title') || act.getAttribute('data-htitle') || '';
    var name = title.split(' :')[0];
    return { index: parseInt(act.getAttribute('data-t'), 10) || 0, tag: TAB_TAG[name] || 0 };
  }
  function upDateType() {                                                // golden :460-467（cbCopyFrom->Clear() 之後 Text 是空的）
    var cb = $('cbCopyFrom');
    if (!cb) return;
    var tag = activeTab().tag;
    cb.innerHTML = '';
    var o0 = document.createElement('option'); o0.value = ''; o0.textContent = ''; o0.setAttribute('data-src', 'q41-text'); cb.appendChild(o0);
    for (var i = 0; i < 3; i++) {
      if (i === tag) continue;
      var o = document.createElement('option'); o.value = TYPES[i]; o.textContent = TYPES[i]; o.setAttribute('data-src', 'q41-items'); cb.appendChild(o);
    }
    cb.selectedIndex = 0;
  }
  function spbCopyClick() {                                              // golden :693-714
    var cb = $('cbCopyFrom'), b = $('spbCopy');
    if (b && (b.disabled || b.getAttribute('aria-disabled') === 'true')) return;
    var txt = cb && cb.selectedIndex >= 0 ? cb.options[cb.selectedIndex].textContent : '';
    if (txt === '') return;
    var copy = 0, now = activeTab().index;
    for (var i = 0; i < 3; i++) if (TYPES[i] === txt) { copy = i; break; }
    if (now < 0 || now > 2) { say2('golden spbCopyClick 在這一頁（ActivePageIndex=' + now + '）會超出 TrayEdit[3][]，網頁不抄'); return; }
    TRAY_EDIT.forEach(function (col) {
      var dst = $(editId(col, now + 1)), src = $(editId(col, copy + 1));
      if (dst && src && !dst.disabled && dst.value !== src.value) { dst.value = src.value; fire(dst); }
    });
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function hook() {
    [1, 2, 3].forEach(function (n) {
      ['X', 'Y'].forEach(function (axis) {
        var ed = $(axis + 'CT' + n);
        if (ed && !ed.__q41) { ed.__q41 = true; ed.addEventListener('change', function () { ctChange(axis, n); }); }
      });
    });
    var pc = $('PageControl1');
    if (pc && !pc.__q41) {
      pc.__q41 = true;
      Array.prototype.forEach.call(pc.querySelectorAll(':scope > .pcTabs > .tab'), function (tb) {
        tb.addEventListener('click', function () { setTimeout(upDateType, 0); });   // 頁面自己的切頁先跑，再 PageControl1Change
      });
    }
    var b = $('spbCopy');
    if (b && !b.__q41) { b.__q41 = true; b.addEventListener('click', spbCopyClick); }
  }
  document.addEventListener('mousedown', pitchIntercept, true);         // capture：早於引擎掛在欄位上的 mousedown

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      setTimeout(function () {                                           // 引擎在這個 promise 的 then 裡同步套完值
        hook();
        [1, 2, 3].forEach(function (n) { trayItems(n, atoi(($('XCT' + n) || {}).value), atoi(($('YCT' + n) || {}).value)); });   // golden :330-335
        upDateType();                                                    // golden FormShow :150（開頁在 TabSheet1；重讀時照目前這一頁）
      }, 0);
      return d;
    });
  };

  window.HT9045TrayFormQ41 = { ctChange: ctChange, upDateType: upDateType, spbCopyClick: spbCopyClick, trayItems: trayItems };   // 探針／除錯用
})();
