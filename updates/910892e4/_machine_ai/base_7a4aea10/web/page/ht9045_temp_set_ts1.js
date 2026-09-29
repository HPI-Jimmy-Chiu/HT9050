/* ht9045_temp_set_ts1.js -- Setup.Temp_Set.html（golden TfTemp_Set，V912 uTemp_Set.cpp）TS-1＋TS-2 的 form.event 送出點（檔名沿用 ts1）。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB2) 20260928 [W906] St01 新檔（手寫）。批次 B2 的 TS-1（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B2、第四節 TS-1 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」。
 *   C++ 那一半已做（4e74e8b4）：kTS_Events（FileRW/Temperature.gen.inc 檔尾）的 rgIndexHeatMode／chkTempCalByRecipe 兩列、
 *   註冊 FileRW/Temperature.cpp g_evreg、本體 FileRW/_EditPage.cpp RunPageEvent。這個檔只補頁面那一半。
 *   寫法照 St02 的 ht9045_config_q41.js :117-163 與 ht9045_temp_set_c.js（TS-7，St02 分支 v906/steven-gpib-widget）。
 *   St02 的 ht9045_temp_set_c.js 只送 TS-7 那 6 個（rb1/2/3/5/6Point、rgBasePoint），刻意不送這兩個；兩個檔互不相干，誰先載入都可以。
 *   頁面 Setup.Temp_Set.html:126（St01 bc935659 的 "<script>"）同一行、inline script 之前載入；:348 是 St02 核准的那一行，不碰，
 *   也不碰它的上下一行（git 合併相鄰行會衝突）；350 行不變。
 *
 * TS-1（golden V912 uTemp_Set.cpp:4232-4236 rgIndexHeatModeClick：fShow 時 ReadTempFile(false)；
 *       :6333-6341 chkTempCalByRecipeClick：CosFunction.bTempCalByRecipe && fShow 時 Temperature.bTempCalByRecipe=Checked、
 *       ReadTempFile(false)、DoIniDataToForm(false)）：換加熱模式／校正來源 ＝ 換讀另一份溫度補償表，畫面數字當場跟著變。
 *   送 {"form":"TfTemp_Set","control":"rgIndexHeatMode","event":"click","itemIndex":2,"state":{…}}
 *      {"form":"TfTemp_Set","control":"chkTempCalByRecipe","event":"click","checked":true,"state":{…}}
 *   沒送就存檔 → 伺服器整頁拒存（R97，FileRW/Temperature.cpp SaveFlow (1)）；送過之後伺服器端的模式／勾選＝頁面的，存檔照常。
 *   ⚠ 照 golden（R98／R99，待 Steven／Jimmy）：點下去的當下伺服器就會照 golden 讀檔補寫缺鍵／變體檔，而且機台記憶體的補償表
 *     （Temperature.fTempOffSet[][]，加熱設定值 ConvertTempOffset 讀它）就換成新那份，沒存檔也一樣；運轉中伺服器回 running。
 * TS-2（AI(W906-EVB2) 20260928 [W906] 追加；C++ 那一半是 B3 lane 2 b08ae6ad：kTS_Events 再 6 列，FileRW/Temperature.cpp 檔尾包一層、
 *   事件後把 g_open.atcActive／atc70／referSensor 同步成伺服器現值，存檔時 SaveFlow (2)(3) 就不再重播）：6 個都是 TCheckBox
 *   （golden uTemp_Set.h:483／:112；名字 rb… 但不是單選鈕），點一下送 {"form":"TfTemp_Set","control":"rbATCActiveOn","event":"click",
 *   "checked":<點完的值>,"state":{…}}：
 *     rbATCActiveOn → golden rbATCActiveOnClick（uTemp_Set.cpp:5430-5451；dfm:6807）：勾了就取消並停用 rbATC70ActiveOn、停用 TSD；
 *     rbATC70ActiveOn → rbATC70ActiveOnClick（:5407-5428；dfm:7143）：勾了就取消並停用 rbATCActiveOn、停用 Chiller／ATCInPC1..4；
 *     cbEnableATCConsFailOffset／cbEnableATCQAModeOffset／cbATCTestTimeOffset → golden DFM 的 OnClick 也是 rbATC70ActiveOnClick
 *       （dfm:7273／:7408／:7538；golden 怪處照留：處理器只看 rbATC70ActiveOn，由伺服器的事件表決定）；
 *     cbATCReferTempSensor → cbATCReferTempSensorClick（:7073-7084；dfm:6822）：取消勾選就藏起並取消 cbUseTC2Offset。
 *   以前只在存檔時重播（兩個 ATC 都改過就拒存）；現在點的當下就跑，ack.changed 回對方的 checked／enabled 與 cbUseTC2Offset 的 visible。
 *
 * state（同 St02 ht9045_temp_set_c.js 的規則）＝引擎存檔會送的那一包（HT9045Page.golden().kinds，值的讀法同引擎 gbValue），但：
 *   (a) 事件表裡的控制項（editlist.get "events" 的鍵：TS-1 兩個＋TS-7 的 6 個＋TS-2 的 6 個）不放 —— 例 rgIndexHeatMode 夾在別的
 *       事件的 state 裡會「只設 ItemIndex、不重讀補償表」，存檔時 SaveFlow 比的是套值前伺服器的值，就不再拒存（舊表寫進新模式的檔）；
 *       TS-2 的 g_open 同步也是假設這 6 個只由自己的事件改。伺服器 29a13bdb 起也會把 state 裡有事件列的控制項丟掉（兩道都在）；
 *   (b) 通道面板欄位（myTempPal<i>_ed*）跟伺服器手上的值一樣的不放（1420 個全送會超過 WS 單則 64 KB；同 inline 存檔包裝的合約）。
 * 寫回 LAST.proxies（＝HT9045Page.golden().page，同一個物件）：inline 的存檔包裝只送「跟 proxies 不同」的面板欄位。事件之後把
 *   伺服器現在的面板值（送過去的 state、ack.changed 回來的新補償表）寫回 proxies；不寫回的話，換表後沒動過的欄位存檔時全被當成
 *   「改過」送出（可能超過 64 KB），被 state 推過去又改回開頁值的欄位則不會送（伺服器留著推過去的值）。St02 03:34 的提醒。
 * ack.changed 照 inline after() 的畫法套：myTempPal<i>_palTemp 的 visible 用 display（alTop 不佔位）、labName 的 caption、
 *   TTabSheet（.pcPane）的 tabVisible；套值時不補發 change（St02 的 TS-7 檔聽單選鈕的 change，那不是使用者點的）。
 * 其他規則同 ht9045_trayassign_ev.js：一次一個、等 ack；同一個值還沒回覆＝連點丟掉；busy: 等 450 ms 重送（最多 3 次）；
 *   not-operator 續權杖再送一次；"reload page" → HT9045Page.load()（editlist.get）；events.<id>.operable＝false 不送；
 *   其他失敗改回點之前的值並說原因（沒改回的話存檔一定被 R97 拒）；存檔時還有事件在等回覆 → 這次不存。
 * 已知限制：St02 的 TS-7 檔有自己的佇列。兩個檔的事件在同一個 WS 上照送出順序處理，但 St02 那一則的 state 是在它送出時取的；
 *   如果在 TS-1 的回覆回來之前就點了基準點數，St02 的 state 可能帶著換表前的面板值。等 St02 的檔合進來後，兩邊合用一個佇列（交件列給 ST01-E）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb2TempSetTs1) return;
  R.__evb2TempSetTs1 = true;

  var STRUCT = 'Temperature', FORM = 'TfTemp_Set', EV_TAG = 'Setup.Temp_Set', LOG = '[Temp_Set/EVB2] ';
  var MAX_WS = 60000;                                    // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage）
  var CTLS = [['rgIndexHeatMode', 'click'], ['chkTempCalByRecipe', 'click'],    // kTS_Events 的 TS-1 兩列
              ['rbATCActiveOn', 'click'], ['rbATC70ActiveOn', 'click'],           // AI(W906-EVB2) 20260928 [W906] TS-2（b08ae6ad）：6 個 TCheckBox，送 checked
              ['cbEnableATCConsFailOffset', 'click'], ['cbEnableATCQAModeOffset', 'click'], ['cbATCTestTimeOffset', 'click'],
              ['cbATCReferTempSensor', 'click']];
  var PANEL_RE = /^myTempPal\d+_ed[A-Za-z_]+$/;          // 同 inline script PANEL_RE 的成員
  var PAL_RE = /^myTempPal\d+_palTemp$/;
  var LAB_RE = /^myTempPal\d+_labName$/;
  var VKEYS = ['text', 'checked', 'itemIndex', 'position'];

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
  function checkboxOf(el) { return !el ? null : (el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]')); }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function gateOn() {
    for (var i = 0; i < CTLS.length; i++) if (evInfo(CTLS[i][0])) return true;
    return false;
  }
  function canSend(id, evn) { var e = evInfo(id); return !!e && e.event === evn && e.operable !== false; }

  /* ---- 值 --------------------------------------------------------------- */
  function readCtl(id) {                                 // rgIndexHeatMode（TRadioGroup）／chkTempCalByRecipe（TCheckBox）
    var el = $(id);
    if (!el) return null;
    var rs = radiosOf(el);
    if (rs.length) { for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i }; return { itemIndex: -1 }; }
    var c = checkboxOf(el);
    return c ? { checked: c.checked } : null;
  }
  function writeCtl(id, v) {
    var el = $(id);
    if (!el || !v) return;
    APPLYING = true;
    try {
      if (v.itemIndex !== undefined) { var rs = radiosOf(el); for (var i = 0; i < rs.length; i++) rs[i].checked = (i === v.itemIndex); }
      else if (v.checked !== undefined) { var c = checkboxOf(el); if (c) c.checked = !!v.checked; }
    } finally { APPLYING = false; }
  }
  function payloadOf(v) { var p = {}; if (v && v.itemIndex !== undefined) p.itemIndex = v.itemIndex; if (v && v.checked !== undefined) p.checked = v.checked; return p; }
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
  function known(id) { return (LAST && LAST.proxies && LAST.proxies[id]) || null; }
  function remember(id, v) {                             // 面板欄位：LAST.proxies[id] ＝ 伺服器現在的值（見檔頭「寫回」）
    if (!PANEL_RE.test(id) || !LAST || !LAST.proxies || !v) return;
    var p = LAST.proxies[id] || (LAST.proxies[id] = {});
    VKEYS.forEach(function (k) { if (v[k] !== undefined) p[k] = v[k]; });
  }
  function stateNow(self) {                              // 見檔頭 (a)(b)
    var g = (window.HT9045Page && HT9045Page.golden) ? HT9045Page.golden() : null, kinds = (g && g.kinds) || {};
    var ev = (LAST && LAST.events) || {}, out = {};
    Object.keys(kinds).forEach(function (id) {
      if (id === self || ev[id]) return;
      var v = gbValueOf(id, kinds[id]);
      if (!v) return;
      if (PANEL_RE.test(id)) {
        var k = known(id);
        if (k && k.text !== undefined && v.text !== undefined && String(k.text) === String(v.text)) return;
      }
      out[id] = v;
    });
    return out;
  }

  /* ---- 套 ack.changed ---------------------------------------------------- */
  function setVis(el, on) {                              // VCL Visible（.pcPane 的 display 是頁籤切換用的，不碰）
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
  function tabOf(id) {                                   // 這一頁的 TTabSheet 是 .pcPane（id＝golden 名），頁籤是同序號的 .tab（同 inline tabOf）
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
    if (!on && tab.classList.contains('act')) {          // VCL：藏起作用中的頁 → 換到第一個看得到的頁（同 inline after()）
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
        remember(id, v);
        if (v.tabVisible !== undefined) setTabVisible(id, !!v.tabVisible);
        if (!el) return;
        if (LAB_RE.test(id) && (v.caption !== undefined || v.text !== undefined)) el.textContent = v.caption !== undefined ? v.caption : v.text;
        if (v.items !== undefined && el.tagName === 'SELECT' && Array.isArray(v.items)) setItems(el, v.items);
        if (v.itemIndex !== undefined) setIndex(el, v.itemIndex, v.text);
        else if (v.text !== undefined && el.tagName === 'SELECT') setIndex(el, -1, v.text);
        else if (v.text !== undefined && 'value' in el && !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) el.value = String(v.text);
        if (v.checked !== undefined) { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); if (c) c.checked = !!v.checked; }
        if (v.position !== undefined) el.value = v.position;
        if (v.caption !== undefined && !LAB_RE.test(id) && !('value' in el) && !el.children.length) el.textContent = v.caption;
        if (v.activePageIndex !== undefined) clickTab(id, v.activePageIndex);
        if (v.visible !== undefined) {
          if (PAL_RE.test(id)) { el.style.display = v.visible ? '' : 'none'; el.style.visibility = v.visible ? '' : 'hidden'; }   // 同 inline after()：看不見的 alTop 面板不佔位
          else setVis(el, !!v.visible);
        }
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
        var e = evInfo(id);
        if (e && v.editable !== undefined) e.operable = !!v.editable;
      });
      dis.forEach(function (el) { setEditable(el, false); });
    } finally { APPLYING = false; }
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
    if (item.gen === GEN) { writeCtl(item.control, item.prev); KNOWN[item.control] = item.prev; }
    say2('Temp_Set ' + item.control + ' 的點擊沒有送到伺服器（form.event）：' + msg + ' —— 已改回點之前的值' +
         (/^(rgIndexHeatMode|chkTempCalByRecipe)$/.test(item.control) ? '，補償表沒有換' : '，連動的元件沒有變'), '#f88');
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    var st = stateNow(item.control);                     // 送出當下取（前一個 ack 已套上）
    v.state = st;
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    if (utf8Len(JSON.stringify({ type: 'cmd', id: 999999, cmd: 'form.event', tag: extra.tag, value: extra.value })) > MAX_WS) {
      fail(item, '改過的面板欄位太多，這次要送的畫面值超過伺服器單則上限 64 KB（先存檔再換）');
      return Promise.resolve(null);
    }
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;
      var a = unwrap(m) || {};
      Object.keys(st).forEach(function (id) { remember(id, st[id]); });   // RunPageEvent 第 4 步已把 state 套進伺服器（不可改的它會丟掉，那種欄位存檔時一樣被丟，結果相同）
      applyChanged(a.changed);
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
        say2('Temp_Set：伺服器要求重新開頁（' + msg + '）—— 重讀中');
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
      if (ev && ev.isTrusted === false) return;          // 別的程式 dispatch 的 change（例 St02 TS-7 套值時補發）不是使用者點的
      var el = $(id);
      if (!el || !gateOn()) return;                      // 伺服器沒有事件表：照舊（存檔時伺服器照 R97 拒存）
      var cur = readCtl(id), prev = KNOWN[id];
      if (OFF || !canSend(id, evn) || !usable(el)) {
        var e = evInfo(id);
        writeCtl(id, prev);
        say2('Temp_Set ' + id + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
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
      el.addEventListener('change', onUser(c[0], c[1]));  // 單選鈕／勾選框的 change 冒泡到 fieldset／label
    });
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      setTimeout(function () {                           // 引擎（與 inline 的 build／fixCombos）在這個 promise 的 then 裡同步套完值
        try { hook(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Temp_Set 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個點擊（換補償表／ATC 選項）在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    return save0.apply(this, arguments);
  };

  window.HT9045EvB2TempSetTs1 = {                         // 探針／除錯用
    gate: gateOn, state: function () { return stateNow(''); }, queue: function () { return QUEUE.slice(); },
    inflight: function () { return INFLIGHT; }, known: function () { return KNOWN; }, applyChanged: applyChanged
  };
})();
