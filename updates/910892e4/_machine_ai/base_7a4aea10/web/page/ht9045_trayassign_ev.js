/* ht9045_trayassign_ev.js -- Setup.TrayAssignment.html（golden TfTrayAssignment，V912 cTrayAssignment.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB2) 20260928 [W906] St01 新檔（手寫）。批次 B2 的 TA-1～TA-4（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B2、第四節 TA-1～TA-4 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」。
 *   C++ 那一半已做（4e74e8b4）：事件表 kTA_Events（FileRW/TrayForm.gen.inc 檔尾，32 列）、註冊 FileRW/TrayForm.cpp g_evreg
 *   （pgRunMode 分頁上的 14 個單選群組換成跳板 TA_EvOnTab）、本體 FileRW/_EditPage.cpp RunPageEvent。這個檔只補頁面那一半。
 *   寫法照 St02 的 ht9045_config_q41.js :117-163（St02 分支 v906/steven-gpib-widget）。引擎 ht9045_wire_engine.js 不改。
 *   頁面 Setup.TrayAssignment.html:125 同一行載入（在接線資料檔之後），行數不變。
 *
 * 送什麼（WS cmd "form.event"，tag 用頁名 "Setup.TrayAssignment"。⚠ 結構名 "TrayForm" 其實是這一頁（Tray Assignment），
 *   Tray Form 頁的結構是 "UserDefForm_File" —— 用頁名不會送錯頁）：
 *   TA-1 13 張方向圖 imgLoader／imgAuto1..6／imgFix1..6 click（golden imgLoaderClick :971-991）
 *        {"form":"TfTrayAssignment","control":"imgAuto2","event":"click","state":{…}}
 *        （沒有值：伺服器照 golden 把 iTrayDirect[] 加一、8 以上回 0，ack.changed 帶新的 tag；引擎自己在頁面上 +1 的那一下由 ack 蓋掉）
 *   TA-2 rgLoaderType／RGLoader／rgLoad_RT click（:1153／:1233／:1275）、cbLoader change（:1317）
 *        {"form":"TfTrayAssignment","control":"RGLoader","event":"click","itemIndex":1,"state":{…}}
 *        {"form":"TfTrayAssignment","control":"cbLoader","event":"change","itemIndex":2,"text":"Type3","state":{…}}
 *   TA-3 rgFixTrayMode click（:1166-1195；Fix 盤有 IC 時伺服器照 golden 改回，ack.changed 帶回原值）
 *   TA-4 cbEmpty／cbColor change、RGAuto1..6／rgAuto1_RT..6_RT click（:1197 cbEmptyChange、:1772 RGAuto2Click；golden 怪處照留：
 *        RGAuto5 的 OnClick 是 RGAuto2Click，由伺服器的事件表決定，頁面不管）
 *   itemIndex＝golden 清單裡的位置（下拉用 editlist.get events.<id>.items 對文字；引擎補的 cpp-text 選項送 -1）；下拉另帶 text
 *   讓伺服器做「清單過期」檢查（R78）。
 *
 * state（RunPageEvent 第 4 步先套再跑處理器）＝引擎存檔會送的那一包（HT9045Page.golden().kinds，值的讀法同引擎 gbValue），但：
 *   (a) 事件表裡的控制項（editlist.get "events" 的鍵）一律不放：它們各自由自己的事件同步。放進 state 的話伺服器會「只設值、不跑
 *       golden 處理器」—— 例：rgFixTrayMode 夾在別的事件的 state 裡送過去，Fix 盤有 IC 的互鎖（rgFixTrayModeClick）不會跑，之後存檔
 *       BeforeApply 看到頁面值＝伺服器值也不再重查（R95 那道防線被繞過）。（伺服器端也該擋，交件列給 ST01-E。）
 *   (b) 多放 pgRunMode 的分頁 {"activePageIndex":n}（c9d3c932 起伺服器收；St01 20260927 19:50 規格「C 路頁面 state 要帶每個
 *       TPageControl」）：golden cbLoaderChange／ShowCompnet 看「使用者當下在哪一頁」（:1235-1236、:1277-1278、:1033-1034），
 *       伺服器自己不知道。
 * 分頁三件事（只在伺服器有事件表、而且 proxies.pgRunMode 帶 activePageIndex 時做）：
 *   ① 開頁照伺服器的分頁點頁籤（golden 表單建一次不銷毀，ActivePage 沿用上次；開機是 DFM 的 tsReTestGroup＝1）；
 *   ② 每個事件的 state 帶目前頁籤；
 *   ③ editlist.save 的 widgets 補 pgRunMode（golden 存檔鈕 :1353-1372：IniConfig.bFTBin2RTBin 時「在 FT 頁存」才把 FT 設定抄到 RT）。
 *   ①③ 不做的話，送過事件（state 把伺服器分頁改成頁面的）與沒送事件存出來的結果會不一樣；三件一起做＝照 golden「看哪一頁存就照哪一頁」。
 *
 * 其他規則（同 St02 範本）：一次一個、等 ack 再送下一個；同一個控制項、同一個值還沒回覆又來一次＝連點，丟掉；busy:（WebCmdGuard 400 ms）
 *   等 450 ms 重送、最多 3 次；not-operator 先續權杖再送一次；錯誤含 "reload page" → 重開頁（HT9045Page.load() ＝ editlist.get）；
 *   events.<id>.operable＝false 不送。其他失敗（running、清單過期、點不到、handler-failed）→ 把那個控制項改回點之前的值，狀態列說原因
 *   （golden 的點擊一定會跑處理器；伺服器沒跑，頁面就不能停在新值）。
 *   套 ack.changed 時不補發 change／input（別的補件聽 change：例 St02 的 TF-2／HP-2 規則；VCL 程式設值也不是使用者點的）。
 *   存檔時還有事件在等回覆 → 這次不存（引擎存檔的 widgets 是按下當時的畫面，事件回來之前的值），請等畫面更新完再按。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb2TrayAssign) return;
  R.__evb2TrayAssign = true;

  var STRUCT = 'TrayForm';                  // editlist.get／editlist.save 的 tag（FileRW/TrayForm.cpp kPage；golden TfTrayAssignment）
  var FORM = 'TfTrayAssignment';            // golden 表單類別（form.event 的 form；不同 ⇒ bad-payload）
  var EV_TAG = 'Setup.TrayAssignment';      // form.event 的 tag（FileRW/_EditPage.cpp FindPageForEvent：頁名）
  var LOG = '[TrayAssign/EVB2] ';
  var PC = 'pgRunMode';                     // golden TPageControl（DFM ActivePage = tsReTestGroup）
  var MAX_WS = 60000;                       // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage），留一點給外框

  // 送出點：[元件, 事件]（＝kTA_Events 32 列；伺服器沒列的不送）
  var CTLS = [];
  ['imgLoader', 'imgAuto1', 'imgAuto2', 'imgAuto3', 'imgAuto4', 'imgAuto5', 'imgAuto6',
   'imgFix1', 'imgFix2', 'imgFix3', 'imgFix4', 'imgFix5', 'imgFix6'].forEach(function (id) { CTLS.push([id, 'click']); });      // TA-1
  ['rgLoaderType', 'RGLoader', 'rgLoad_RT'].forEach(function (id) { CTLS.push([id, 'click']); });                                 // TA-2
  CTLS.push(['cbLoader', 'change']);                                                                                              // TA-2
  CTLS.push(['rgFixTrayMode', 'click']);                                                                                          // TA-3
  ['cbEmpty', 'cbColor'].forEach(function (id) { CTLS.push([id, 'change']); });                                                   // TA-4
  ['RGAuto1', 'RGAuto2', 'RGAuto3', 'RGAuto4', 'RGAuto5', 'RGAuto6',
   'rgAuto1_RT', 'rgAuto2_RT', 'rgAuto3_RT', 'rgAuto4_RT', 'rgAuto5_RT', 'rgAuto6_RT'].forEach(function (id) { CTLS.push([id, 'click']); });   // TA-4

  var LAST = null;                          // 最近一次 editlist.get TrayForm 的回應（＝HT9045Page.golden().page，同一個物件）
  var GEN = 0;                              // 每次開頁 +1：重開頁之前送出去的事件，回覆不再套（editlist.get 已經給了新狀態）
  var QUEUE = [], INFLIGHT = null, APPLYING = false, KNOWN = {}, OFF = '';

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    if (window.console) console.info(LOG + msg);
  }
  function unwrap(m) {                                   // 伺服器把回傳欄位併進 ack；舊寫法放在 value 字串裡 —— 兩種都收
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
  function usable(el) {                                  // 引擎／權限停用的元件（自己或上層）一樣按不到
    if (!el || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) return false;
    var x = (el.tagName === 'INPUT' || el.tagName === 'SELECT') ? el : el.querySelector('input,select');
    return !x || !x.disabled;
  }
  function radiosOf(el) { return el ? el.querySelectorAll('input[type="radio"]') : []; }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function gateOn() {                                    // 伺服器有這一頁的事件表（舊伺服器沒有 events → 一個都不送，照舊）
    for (var i = 0; i < CTLS.length; i++) if (evInfo(CTLS[i][0])) return true;
    return false;
  }
  function canSend(id, evn) { var e = evInfo(id); return !!e && e.event === evn && e.operable !== false; }

  /* ---- 值 --------------------------------------------------------------- */
  function goldenIndex(id, sel) {                        // <select> → golden ItemIndex／Text（對 events.<id>.items）
    var o = sel.options[sel.selectedIndex];
    if (!o) return { itemIndex: -1, text: '' };
    var t = o.textContent, src = o.getAttribute('data-src');
    if (src === 'cpp-text') return { itemIndex: -1, text: t };          // VCL csDropDown：清單外的文字
    var e = evInfo(id), items = e && Array.isArray(e.items) ? e.items : null;
    if (items) {
      if (items[sel.selectedIndex] === t) return { itemIndex: sel.selectedIndex, text: t };
      return { itemIndex: items.indexOf(t), text: t };
    }
    if (src === 'cpp-item') return { itemIndex: -1, text: t };
    return { itemIndex: sel.selectedIndex, text: t };
  }
  function readCtl(id) {                                 // 送出點控制項目前的值（payload 與「改回」用）
    var el = $(id);
    if (!el) return null;
    if (el.tagName === 'IMG') return { tag: parseInt(el.getAttribute('data-tag'), 10) || 0 };
    if (el.tagName === 'SELECT') { var g = goldenIndex(id, el); g.dom = el.selectedIndex; return g; }
    var rs = radiosOf(el);
    for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
    return rs.length ? { itemIndex: -1 } : null;
  }
  function writeCtl(id, v) {                             // 改回點之前的值（不發事件）
    var el = $(id);
    if (!el || !v) return;
    APPLYING = true;
    try {
      if (el.tagName === 'IMG' && v.tag !== undefined) { el.setAttribute('data-tag', v.tag); el.src = 'img/dfm_type' + v.tag + '.png'; }
      else if (el.tagName === 'SELECT' && v.dom !== undefined) el.selectedIndex = v.dom;
      else if (v.itemIndex !== undefined) { var rs = radiosOf(el); for (var i = 0; i < rs.length; i++) rs[i].checked = (i === v.itemIndex); }
    } finally { APPLYING = false; }
  }
  function payloadOf(v) {                                // TImage 沒有值；單選群組 itemIndex；下拉 itemIndex＋text
    var p = {};
    if (!v || v.tag !== undefined) return p;
    if (v.itemIndex !== undefined) p.itemIndex = v.itemIndex;
    if (v.text !== undefined) p.text = v.text;
    return p;
  }
  // 同引擎 gbValue（ht9045_wire_engine.js，Jimmy 的檔，沒有匯出）：state 跟存檔送同一種值
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
  function gbValueOf(id, k) {
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
  function pcSupported() { var p = LAST && LAST.proxies && LAST.proxies[PC]; return !!p && typeof p.activePageIndex === 'number'; }
  function pageTab() {                                   // 頁面上 pgRunMode 目前的頁籤（data-t＝golden PageIndex）
    var pc = $(PC), act = pc && pc.querySelector(':scope > .pcTabs > .tab.act');
    var n = act ? parseInt(act.getAttribute('data-t'), 10) : NaN;
    return isNaN(n) ? null : n;
  }
  function stateNow(self) {                              // 見檔頭 (a)(b)
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
  function setVis(el, on) {                              // VCL Visible（引擎用 visibility；產生器的 DFM Visible=False 是 display:none）
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
  function tabOf(id) {                                   // TTabSheet 的頁籤（頁籤沒有 id：title／data-htitle "<id> : TTabSheet"）
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
    if (!on && tab.classList.contains('act')) {          // VCL：藏起作用中的頁 → 換到第一個看得到的頁（同 Setup.Temp_Set.html inline after()）
      var first = tab.parentElement.querySelector(':scope > .tab:not([style*="display: none"])');
      if (first && first !== tab) first.click();
    }
  }
  function clickTab(pcId, n) {
    var pc = $(pcId), tab = pc && pc.querySelector(':scope > .pcTabs > .tab[data-t="' + n + '"]');
    if (tab && !tab.classList.contains('act')) tab.click();
  }
  function setItems(el, items) {                         // TComboBox->Items 換了：重建選項（保留目前文字）
    var keep = el.selectedIndex >= 0 && el.options[el.selectedIndex] ? el.options[el.selectedIndex].textContent : null;
    while (el.options.length) el.remove(0);
    items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; o.setAttribute('data-src', 'ev-items'); el.appendChild(o); });
    el.selectedIndex = keep === null ? -1 : items.indexOf(keep);
  }
  function setIndex(el, i, text) {                       // TComboBox／TRadioGroup ItemIndex（下拉 -1＝清單外文字）
    if (el.tagName === 'SELECT') {
      var olds = el.querySelectorAll('option[data-src="cpp-text"]');
      for (var k = 0; k < olds.length; k++) olds[k].parentNode.removeChild(olds[k]);
      if (i >= 0 && i < el.options.length && (text === undefined || el.options[i].textContent === text)) { el.selectedIndex = i; return; }
      if (text === undefined) return;                    // 選項數不夠又沒有文字：不猜
      for (var j = 0; j < el.options.length; j++) if (el.options[j].textContent === text) { el.selectedIndex = j; return; }
      var o = document.createElement('option'); o.textContent = text; o.setAttribute('data-src', 'cpp-text');
      el.appendChild(o); el.selectedIndex = el.options.length - 1;
      return;
    }
    var rs = radiosOf(el);
    for (var r = 0; r < rs.length; r++) rs[r].checked = (r === i);
  }
  function applyChanged(ch) {                            // {元件:{text,itemIndex,checked,items,position,tag,caption,visible,enabled,editable,tabVisible,activePageIndex}}，只有變的鍵
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
        if (v.position !== undefined) el.value = v.position;                                  // 同引擎 gbApply（TScrollBar 是 div，值掛在 el.value）
        if (v.tag !== undefined && el.tagName === 'IMG') { el.setAttribute('data-tag', v.tag); el.src = 'img/dfm_type' + v.tag + '.png'; }
        if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;
        if (v.activePageIndex !== undefined) clickTab(id, v.activePageIndex);
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
        var e = evInfo(id);
        if (e && v.editable !== undefined) e.operable = !!v.editable;                        // 這幾種元件沒有 ReadOnly：可改＝點得到
      });
      dis.forEach(function (el) { setEditable(el, false); });                                 // 先開後關（同引擎 gbLoad）：停用的容器蓋過子元件
    } finally { APPLYING = false; }
  }

  /* ---- 佇列 -------------------------------------------------------------- */
  function latestFor(id) {
    for (var i = QUEUE.length - 1; i >= 0; i--) if (QUEUE[i].control === id) return QUEUE[i];
    return INFLIGHT && INFLIGHT.control === id ? INFLIGHT : null;
  }
  function pendingFor(id) { return QUEUE.some(function (q) { return q.control === id; }); }
  function refreshKnown() { CTLS.forEach(function (c) { if (!pendingFor(c[0])) KNOWN[c[0]] = readCtl(c[0]); }); }   // 排著還沒送的控制項不動（改回時用它自己的 prev）
  function idle() { return !INFLIGHT && !QUEUE.length; }
  function fail(item, msg) {                             // 伺服器沒跑 golden 處理器 → 改回點之前的值；同一個控制項後面排著的一起丟
    QUEUE = QUEUE.filter(function (q) { return q.control !== item.control; });
    if (item.gen === GEN) { writeCtl(item.control, item.prev); KNOWN[item.control] = item.prev; }
    say2('Tray Assignment ' + item.control + ' 的點擊沒有送到伺服器（form.event）：' + msg + ' —— 已改回點之前的值', '#f88');
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    v.state = stateNow(item.control);                    // 送出當下取（前一個 ack 已套上），不是點的當下
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    if (utf8Len(JSON.stringify({ type: 'cmd', id: 999999, cmd: 'form.event', tag: extra.tag, value: extra.value })) > MAX_WS) {
      fail(item, '這次要送的畫面值超過伺服器單則上限 64 KB');
      return Promise.resolve(null);
    }
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;                 // 期間重開過頁：editlist.get 已經給了新狀態
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a.todo && a.todo.length && window.console) console.info(LOG + item.control + ' todo: ' + a.todo.join(' | '));
      refreshKnown();
      return a;
    }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {             // 同一指令 400 ms 內重送被擋（WebCmdGuard）→ 等一下再送
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(item, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(item, tries + 1); }, function () { fail(item, msg); return null; });
      }
      if (/reload page/.test(msg)) {
        QUEUE = [];
        say2('Tray Assignment：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      if (/unknown cmd/i.test(msg)) OFF = msg;           // 舊伺服器沒有 form.event：之後不再送
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
    if (last && last.event === item.event && JSON.stringify(last.val) === JSON.stringify(item.val)) return false;   // 連點：同一個值還沒回覆
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function onUser(id, evn) {
    return function (ev) {
      if (APPLYING) return;
      if (ev && ev.type === 'change' && ev.isTrusted === false) return;   // 別的程式 dispatch 的 change 不是使用者點的
      var el = $(id);
      if (!el || !gateOn()) return;                      // 伺服器沒有事件表：照舊（存檔時伺服器補 R94／R95）
      var cur = readCtl(id), prev = KNOWN[id];
      if (OFF || !canSend(id, evn) || !usable(el)) {
        if (el.tagName !== 'IMG' || usable(el)) {        // 圖被停用時引擎本來就不換圖，沒有東西要改回
          var e = evInfo(id);
          writeCtl(id, prev);
          say2('Tray Assignment ' + id + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
                : !e ? '伺服器的事件表沒有這個元件' : e.event !== evn ? '伺服器的事件是 ' + e.event + '，不是 ' + evn
                : '伺服器說這個元件現在點不到（golden 點不到，events.operable=false）') + ' —— 已改回', '#f88');
        }
        return;
      }
      if (el.tagName !== 'IMG' && prev && cur && JSON.stringify(payloadOf(prev)) === JSON.stringify(payloadOf(cur))) return;   // 值沒變（golden 不會 OnClick）
      if (enqueue({ control: id, event: evn, val: payloadOf(cur), prev: prev, gen: GEN })) KNOWN[id] = cur;
    };
  }
  function hook() {
    CTLS.forEach(function (c) {
      var el = $(c[0]);
      if (!el || el.__evb2) return;
      el.__evb2 = true;
      el.addEventListener(el.tagName === 'IMG' ? 'click' : 'change', onUser(c[0], c[1]));
    });
  }
  function applyOpenTab() {                              // 分頁 ①：開頁照伺服器的分頁（golden 沿用上次的 ActivePage）
    if (!gateOn() || !pcSupported()) return;
    var n = LAST.proxies[PC].activePageIndex, pc = $(PC);
    var tab = pc && pc.querySelector(':scope > .pcTabs > .tab[data-t="' + n + '"]');
    if (tab && tab.style.display !== 'none' && !tab.classList.contains('act')) tab.click();
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      setTimeout(function () {                           // 引擎在這個 promise 的 then 裡同步套完值
        try { hook(); applyOpenTab(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Tray Assignment 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個點擊事件在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    if (widgets && typeof widgets === 'object' && gateOn() && pcSupported() && !widgets[PC]) {   // 分頁 ③
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

  window.HT9045EvB2TrayAssign = {                         // 探針／除錯用
    gate: gateOn, state: function () { return stateNow(''); }, queue: function () { return QUEUE.slice(); },
    inflight: function () { return INFLIGHT; }, known: function () { return KNOWN; }, applyChanged: applyChanged, pageTab: pageTab
  };
})();
