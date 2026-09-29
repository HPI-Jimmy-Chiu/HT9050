/* ht9045_trayform_ev.js -- Setup.TrayForm.html（golden TfTrayForm，V912 cTrayForm.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB2) 20260928 [W906] St01 新檔（手寫）。批次 B2 的 Q40-TF（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B2、第四節 Q40-TF 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」。
 *   C++ 那一半已做（76058840）：事件表 kTF_Events（FileRW/UserDefForm_File.gen.inc 檔尾，3 列）、註冊 FileRW/UserDefForm_File.cpp g_evreg、
 *   本體 FileRW/_EditPage.cpp RunPageEvent。這個檔只補頁面那一半。寫法照 St02 的 ht9045_config_q41.js :117-163
 *   （St02 分支 v906/steven-gpib-widget）。引擎 ht9045_wire_engine.js 不改；St02 的 ht9045_trayform_q41.js（TF-2／TF-3）不動。
 *   頁面 Setup.TrayForm.html:126（St01 9a43a8cf 的註解行）同一行載入；:121 是 St02 那一行，不碰、也不碰它的上下一行（git 合併
 *   相鄰行會衝突）。
 *
 * Q40-TF：「從資料庫選」下拉 cbTrayType1／2／3（DFM Tag 0／1／2；OnChange 都是 golden cbTrayType1Change :570-577 →
 *   ShowTypePage(Tag, ItemIndex) :579-602：D:\HT9045\system\TrayForm.csv 第 ItemIndex 列的 10 個欄位填進 TrayEdit[Tag][0..9]，
 *   ItemIndex<1（第 0 列是表頭）什麼都不做）。
 *   送 {"form":"TfTrayForm","control":"cbTrayType2","event":"change","itemIndex":5,"text":"<選項文字>","state":{…}}
 *   itemIndex＝golden 清單裡的位置、text＝選項文字（伺服器拿它做「清單過期」檢查，R78）。
 *   ⚠ 選項：C 路 proxies 不帶 Items（St01 FROM_STEVEN 20260927 15:45 ⑤），頁面 HTML 只有 DFM 的一行 "Select from Database..."。
 *     開頁時（引擎套值之前）用 editlist.get events.<id>.items（伺服器 golden FormShow :155-174 填的 Items）重建選項；
 *     proxies 的 itemIndex 對不上文字時改成 -1（VCL：Items->Clear() 之後 Text 不是清單的一項 ⇒ ItemIndex=-1；vclcompat 可能停在 0），
 *     引擎就照 golden Text 補一個 "Select from Database..." 選項。同 Setup.Temp_Set.html inline fixCombos 的作法。
 *   ack.changed：TrayName／XST／YST／XPitch／YPitch／Tp<n>Thick／XCT／YCT／Tp<n>TickUp／Memo 的 text。套值時不補發 change
 *     —— St02 的 TF-2（XCT 是 "1" 就把 Pitch 設 0，golden XCT1MouseDown 只在小鍵盤之後）聽 change，golden 用程式填值不會跑它。
 *   盤面示意圖：golden :599-600 TMyTray[Tag]->XItem／YItem ＝ CSV 的 Columns／Rows（＝剛填的 XCT／YCT），伺服器沒有這個元件
 *     （純畫面）⇒ 頁面在 ack 之後重畫 TMyTray<n>（n＝Tag+1）；有 St02 的 HT9045TrayFormQ41.trayItems 就用它，沒有就用同樣的畫法。
 * TF-4（AI(W906-EVB2) 20260928 [W906] 追加；C++ 那一半是 B3 lane 2 b08ae6ad：kTF_Events 第 4 列、FileRW/UserDefForm_File.cpp
 *   TF_EvBootProxies 補按鈕替身與父層 pnlBixBox）：Bin Box 分頁的 Reset 鈕 btnBinBoxReset click（golden btnBinBoxResetClick
 *   cTrayForm.cpp:729-734：edtBinBoxNow->Text=0、LastSet.iBinBoxCount=0、MOT[MManualTray3].ClearTray —— Fix3 盤的格子資料清空，
 *   只改記憶體、不動馬達、不寫檔；golden 沒有確認框，這裡也不問）。
 *   送 {"form":"TfTrayForm","control":"btnBinBoxReset","event":"click","state":{…}}（按鈕沒有值）；ack.changed 帶 edtBinBoxNow.text="0"。
 *   只收真的點擊；還沒回覆又按＝連點丟掉（結果一樣，golden 按兩下也只是清兩次）；失敗時沒有東西要改回，狀態列說原因。
 *
 * state ＝引擎存檔會送的那一包（HT9045Page.golden().kinds，值的讀法同引擎 gbValue），事件表裡的控制項不放（它們各自由自己的
 *   事件同步；放進 state 伺服器只設值、不跑 golden 處理器）。這一頁的處理器不看分頁，所以 PageControl1 不放。
 * 其他規則同 ht9045_trayassign_ev.js：一次一個、等 ack；同一個值還沒回覆＝連點丟掉；busy: 等 450 ms 重送（最多 3 次）；
 *   not-operator 續權杖再送一次；"reload page" → HT9045Page.load()（editlist.get）；events.<id>.operable＝false 不送；
 *   其他失敗改回點之前的選項並說原因；存檔時還有事件在等回覆 → 這次不存。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb2TrayForm) return;
  R.__evb2TrayForm = true;

  var STRUCT = 'UserDefForm_File';          // editlist.get／editlist.save 的 tag（FileRW/UserDefForm_File.cpp kPage；golden TfTrayForm）
  var FORM = 'TfTrayForm';
  var EV_TAG = 'Setup.TrayForm';            // form.event 的 tag（頁名；⚠ 結構名 "TrayForm" 是 Tray Assignment 頁）
  var LOG = '[TrayForm/EVB2] ';
  var MAX_WS = 60000;                       // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage）
  var CTLS = [['cbTrayType1', 'change'], ['cbTrayType2', 'change'], ['cbTrayType3', 'change']];   // ＝kTF_Events 的 3 個下拉
  var BTN = 'btnBinBoxReset';               // AI(W906-EVB2) 20260928 [W906] TF-4：kTF_Events 第 4 列（b08ae6ad），按鈕沒有值、不進 CTLS

  var LAST = null, GEN = 0, QUEUE = [], INFLIGHT = null, APPLYING = false, KNOWN = {}, OFF = '';

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
  function usable(el) {
    if (!el || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) return false;
    var x = (el.tagName === 'INPUT' || el.tagName === 'SELECT') ? el : el.querySelector('input,select');
    return !x || !x.disabled;
  }
  function radiosOf(el) { return el ? el.querySelectorAll('input[type="radio"]') : []; }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function gateOn() {
    for (var i = 0; i < CTLS.length; i++) if (evInfo(CTLS[i][0])) return true;
    return false;
  }
  function canSend(id, evn) { var e = evInfo(id); return !!e && e.event === evn && e.operable !== false; }
  function atoi(v) { var n = parseInt(String(v === undefined || v === null ? '' : v), 10); return isNaN(n) ? 0 : n; }

  /* ---- 開頁：下拉選項（見檔頭 ⚠ 選項） ---------------------------------- */
  function fillCombos(d) {
    CTLS.forEach(function (c) {
      var id = c[0], el = $(id), e = d && d.events && d.events[id];
      if (!el || el.tagName !== 'SELECT' || !e || !Array.isArray(e.items)) return;
      while (el.options.length) el.remove(0);
      e.items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; o.setAttribute('data-src', 'ev-items'); el.appendChild(o); });
      el.selectedIndex = -1;
      var px = d.proxies && d.proxies[id];
      if (px && px.itemIndex !== undefined && px.itemIndex >= 0 && px.text !== undefined && e.items[px.itemIndex] !== px.text) px.itemIndex = -1;
    });
  }

  /* ---- 值 --------------------------------------------------------------- */
  function goldenIndex(id, sel) {
    var o = sel.options[sel.selectedIndex];
    if (!o) return { itemIndex: -1, text: '' };
    var t = o.textContent, src = o.getAttribute('data-src');
    if (src === 'cpp-text') return { itemIndex: -1, text: t };
    var e = evInfo(id), items = e && Array.isArray(e.items) ? e.items : null;
    if (items) {
      if (items[sel.selectedIndex] === t) return { itemIndex: sel.selectedIndex, text: t };
      return { itemIndex: items.indexOf(t), text: t };
    }
    if (src === 'cpp-item') return { itemIndex: -1, text: t };
    return { itemIndex: sel.selectedIndex, text: t };
  }
  function readCtl(id) {
    var el = $(id);
    if (!el || el.tagName !== 'SELECT') return null;
    var g = goldenIndex(id, el);
    g.dom = el.selectedIndex;
    return g;
  }
  function writeCtl(id, v) {
    var el = $(id);
    if (!el || !v || v.dom === undefined) return;
    APPLYING = true;
    try { el.selectedIndex = v.dom; } finally { APPLYING = false; }
  }
  function payloadOf(v) { return v ? { itemIndex: v.itemIndex, text: v.text } : {}; }
  function oleOf(s) {
    var m = /^\s*(\d{4})\/(\d{1,2})\/(\d{1,2})(?:\s+(\d{1,2}):(\d{2})(?::(\d{2}))?)?\s*$/.exec(s || '');
    if (!m) return null;
    return Date.UTC(+m[1], +m[2] - 1, +m[3], +(m[4] || 0), +(m[5] || 0), +(m[6] || 0)) / 86400000 + 25569;
  }
  function gridOf(el) {
    var out = [], trs = el.querySelectorAll('tr');
    for (var r = 0; r < trs.length; r++) {
      var cs = trs[r].children;
      for (var c = 0; c < cs.length; c++) { (out[c] = out[c] || [])[r] = cs[c].textContent; }
    }
    return out;
  }
  function gbValueOf(id, k) {                            // 同引擎 gbValue（沒有匯出）
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
    if (k === 'dateTime') { var d = oleOf(el.value); return d === null ? null : { dateTime: d }; }
    if (k === 'cells') return { cells: gridOf(el) };
    if (k === 'tag') return { tag: parseInt(el.getAttribute('data-tag'), 10) || 0 };
    return null;
  }
  function stateNow(self) {
    var g = (window.HT9045Page && HT9045Page.golden) ? HT9045Page.golden() : null, kinds = (g && g.kinds) || {};
    var ev = (LAST && LAST.events) || {}, out = {};
    Object.keys(kinds).forEach(function (id) {
      if (id === self || ev[id]) return;
      var v = gbValueOf(id, kinds[id]);
      if (v) out[id] = v;
    });
    return out;
  }

  /* ---- 套 ack.changed ---------------------------------------------------- */
  function setVis(el, on) {
    el.style.visibility = on ? '' : 'hidden';
    if (on && el.style.display === 'none' && !el.classList.contains('pcPane')) el.style.display = '';
  }
  function setEditable(el, on) {
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
  function applyChanged(ch) {
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
        if (v.tag !== undefined && el.tagName === 'IMG') { el.setAttribute('data-tag', v.tag); el.src = 'img/dfm_type' + v.tag + '.png'; }
        if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;
        if (v.activePageIndex !== undefined) clickTab(id, v.activePageIndex);
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
        var e = evInfo(id);
        if (e && v.editable !== undefined) e.operable = !!v.editable;
      });
      dis.forEach(function (el) { setEditable(el, false); });
    } finally { APPLYING = false; }
  }
  // golden ShowTypePage :599-600：TMyTray[Tag]->XItem／YItem（同 St02 ht9045_trayform_q41.js trayItems 的畫法）
  function trayRedraw(n) {
    var xitem = atoi(($('XCT' + n) || {}).value), yitem = atoi(($('YCT' + n) || {}).value);
    if (window.HT9045TrayFormQ41 && typeof HT9045TrayFormQ41.trayItems === 'function') { HT9045TrayFormQ41.trayItems(n, xitem, yitem); return; }
    var ph = $('TMyTray' + n);
    if (!ph || !window.HTWidgets || !HTWidgets.makeMyTray) return;
    var s; try { s = JSON.parse(ph.getAttribute('data-tray') || '{}'); } catch (e) { return; }
    s.xitem = xitem; s.yitem = yitem;
    ph.setAttribute('data-tray', JSON.stringify(s));
    if (!(s.xitem >= 1) || !(s.yitem >= 1)) return;                    // atoi 出 0：golden 畫不出格子，保留舊圖
    var cw = Math.max(4, Math.floor((s.w - 10) / s.xitem)), chh = Math.max(4, Math.floor((s.h - 10) / s.yitem));
    var t = HTWidgets.makeMyTray({ name: s.name + '_t', xitem: s.xitem, yitem: s.yitem, xblockItem: s.xblockItem, yblockItem: s.yblockItem,
                                   cellW: cw, cellH: chh, trayColor: s.trayColor, trayDirect: s.trayDirect });
    while (ph.firstChild) ph.removeChild(ph.firstChild);
    ph.appendChild(t);
  }

  /* ---- 佇列 -------------------------------------------------------------- */
  function latestFor(id) {
    for (var i = QUEUE.length - 1; i >= 0; i--) if (QUEUE[i].control === id) return QUEUE[i];
    return INFLIGHT && INFLIGHT.control === id ? INFLIGHT : null;
  }
  function pendingFor(id) { return QUEUE.some(function (q) { return q.control === id; }); }
  function refreshKnown() { CTLS.forEach(function (c) { if (!pendingFor(c[0])) KNOWN[c[0]] = readCtl(c[0]); }); }
  function idle() { return !INFLIGHT && !QUEUE.length; }
  function fail(item, msg) {
    QUEUE = QUEUE.filter(function (q) { return q.control !== item.control; });
    if (item.control === BTN) {                          // TF-4：按鈕沒有值，沒有東西要改回（Bin Box 數量／Fix3 盤沒有清）
      say2('Tray Form Bin Box Reset 沒有送到伺服器（form.event）：' + msg + ' —— Bin Box 數量沒有歸零、Fix3 盤沒有清', '#f88');
      return;
    }
    if (item.gen === GEN) { writeCtl(item.control, item.prev); KNOWN[item.control] = item.prev; }
    say2('Tray Form ' + item.control + ' 的選擇沒有送到伺服器（form.event）：' + msg + ' —— 已改回原來的選項，欄位沒有填', '#f88');
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    v.state = stateNow(item.control);
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    if (utf8Len(JSON.stringify({ type: 'cmd', id: 999999, cmd: 'form.event', tag: extra.tag, value: extra.value })) > MAX_WS) {
      fail(item, '這次要送的畫面值超過伺服器單則上限 64 KB');
      return Promise.resolve(null);
    }
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (p.itemIndex >= 1) trayRedraw(atoi(item.control.slice(-1)));   // golden ShowTypePage：index<1 直接 return，不畫
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a.todo && a.todo.length && window.console) console.info(LOG + item.control + ' todo: ' + a.todo.join(' | '));
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
        say2('Tray Form：伺服器要求重新開頁（' + msg + '）—— 重讀中');
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
    if (last && last.event === item.event && JSON.stringify(last.val) === JSON.stringify(item.val)) return false;
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function onUser(id, evn) {
    return function (ev) {
      if (APPLYING) return;
      if (ev && ev.isTrusted === false) return;          // 別的程式 dispatch 的 change 不是使用者選的
      var el = $(id);
      if (!el || !gateOn()) return;                      // 伺服器沒有事件表：照舊（選了不會填）
      var cur = readCtl(id), prev = KNOWN[id];
      if (OFF || !canSend(id, evn) || !usable(el)) {
        var e = evInfo(id);
        writeCtl(id, prev);
        say2('Tray Form ' + id + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
              : !e ? '伺服器的事件表沒有這個元件' : e.event !== evn ? '伺服器的事件是 ' + e.event + '，不是 ' + evn
              : '伺服器說這個元件現在點不到（golden 點不到，events.operable=false）') + ' —— 已改回', '#f88');
        return;
      }
      if (prev && cur && JSON.stringify(payloadOf(prev)) === JSON.stringify(payloadOf(cur))) return;
      if (enqueue({ control: id, event: evn, val: payloadOf(cur), prev: prev, gen: GEN })) KNOWN[id] = cur;
    };
  }
  function hook() {
    CTLS.forEach(function (c) {
      var el = $(c[0]);
      if (!el || el.__evb2) return;
      el.__evb2 = true;
      el.addEventListener('change', onUser(c[0], c[1]));
    });
    hookBinBox();
  }
  // AI(W906-EVB2) 20260928 [W906] TF-4：btnBinBoxReset（見檔頭）
  function hookBinBox() {
    var b = $(BTN);
    if (!b || b.__evb2) return;
    b.__evb2 = true;
    b.addEventListener('click', function (ev) {
      if (ev && ev.isTrusted === false) return;          // 只收真的點擊
      if (!gateOn()) return;                             // 伺服器沒有這一頁的事件表：照舊（按了沒反應）
      if (b.disabled || !usable(b)) return;              // 引擎／權限停用：VCL 一樣按不到
      if (OFF || !canSend(BTN, 'click')) {
        var e = evInfo(BTN);
        say2('Tray Form Bin Box Reset：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
              : !e ? '伺服器的事件表沒有這個按鈕（要 b08ae6ad 之後的 wb_serve）' : e.event !== 'click' ? '伺服器的事件是 ' + e.event + '，不是 click'
              : '伺服器說這個按鈕現在按不到（golden 按不到，events.operable=false）') + ' —— 沒有送', '#f88');
        return;
      }
      if (latestFor(BTN)) return;                        // 連點：上一下還沒回覆
      enqueue({ control: BTN, event: 'click', val: {}, prev: null, gen: GEN });
    });
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      try { fillCombos(d); } catch (e) { if (window.console) console.error(LOG + 'fillCombos', e); }   // 引擎套值之前（這個 then 先跑）
      setTimeout(function () {
        try { hook(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Tray Form 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個事件（從資料庫選／Bin Box Reset）在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    return save0.apply(this, arguments);
  };

  window.HT9045EvB2TrayForm = {                           // 探針／除錯用
    gate: gateOn, state: function () { return stateNow(''); }, queue: function () { return QUEUE.slice(); },
    inflight: function () { return INFLIGHT; }, known: function () { return KNOWN; }, applyChanged: applyChanged, trayRedraw: trayRedraw
  };
})();
