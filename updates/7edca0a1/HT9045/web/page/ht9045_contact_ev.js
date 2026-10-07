/* ht9045_contact_ev.js -- Setup.Contact.html（golden TfContact，V912 cContact.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB3) 20260928 [W906] St01 新檔（手寫）。批次 B3 的 CT-1／CT-L1
 *   （D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B3、第四節 CT 列）。
 *   Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。
 *   C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\DeviceForm_File.cpp 檔尾「批次 B3」段（事件表 kDF_Events 8 列、跳板 EvB3Run、機台記憶體 session）；
 *   設定 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\DeviceForm_File.py 的 events。佇列／state／套 ack 的寫法同 ht9045_cleaning_ev.js
 *   （St02 ht9045_config_q41.js :117-163 與 B2 ht9045_trayassign_ev.js 的做法）。引擎、ht9045_contact_wire.js、ht9045_contact_slk.js、
 *   St02 的 ht9045_contact_q41.js 不改。頁面 Setup.Contact.html:126 同一行載入（在接線資料檔之後；:134 是 St02 那一行，不碰、也不碰它的鄰行），行數不變。
 *
 * 送什麼（WS cmd "form.event"，tag "DeviceForm_File"）：
 *   CT-1 cbContactMode change    {"form":"TfContact","control":"cbContactMode","event":"change","itemIndex":3,"text":"…","state":{…}}
 *                                golden cbContactModeChange :14084（Drop Offset 兩格的值與可改、Side Push 面板）
 *        rgKitDiameter／rgOutKitDiameter／rgDieForceKitDiameter click {"control":"rgKitDiameter","event":"click","itemIndex":2,…}
 *                                golden :15509／:17393／:19796（力量當場重算：edAirForce／edAirForceN／edForcePerDeviceKG…）
 *        chkUseAddWeight／cbEnableUK click {"control":"cbEnableUK","event":"click","checked":true,…}  golden :17387／:17430
 *        coD41 change            {"control":"coD41","event":"change","itemIndex":1,"text":"…",…}   golden :15274（edD41 顯示）
 *   CT-L1 pnlSensorAdj click     {"control":"pnlSensorAdj","event":"click","state":{…}}  golden pnlSensorAdjClick :14171：等級 109 在伺服器查
 *                                （不足 golden 跳 WAR1676）；過了 ack.todo 會有 "open:cclink …" → 這支叫 background.html 開 'cclink' 視窗
 *                                （HW.MyCCLinkSensor.html）。沒有這一筆就不開（頁面不能自己放行）。
 *   state＝引擎存檔會送的那一包（HT9045Page.golden().kinds），事件表裡的元件不放；打字欄位（edPinCount、N、G…）與 scrbSLK 的 position
 *   都在裡面 —— golden 處理器（ShowArmAndDeviceForce）讀的是畫面上目前的腳數／力量。
 *   ⚠ 伺服器端：這幾支 golden 處理器會改機台正在用的記憶體（DeviceForm.dPress 等）；網頁沒有關窗事件，所以伺服器只把結果記在這一頁的
 *     session、畫面照 golden 變，機台記憶體等存檔才改（R 題，見 C++ 檔尾 (3)）。
 * 規則同 ht9045_cleaning_ev.js：一次一個、連點丟掉、busy: 重送、not-operator 續權杖、"reload page" 重開頁、operable＝false 不送、
 *   失敗改回、套 ack 不補發事件（ht9045_contact_slk.js 攔的是 value setter，照 VCL「程式設 Text 也觸發 OnChange」自己會算）、事件沒回完不存檔。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb3Contact) return;
  R.__evb3Contact = true;

  var STRUCT = 'DeviceForm_File';           // editlist.get／editlist.save／form.event 的 tag（FileRW/DeviceForm_File.cpp g_page）
  var FORM = 'TfContact';                   // golden 表單類別（form.event 的 form；不同 ⇒ bad-payload）
  var NAME = 'Contact';                     // 狀態列訊息用的頁名
  var LOG = '[Contact/EVB3] ';
  var PC = '';                              // 這一頁的處理器不看分頁
  var MAX_WS = 60000;                       // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage），留一點給外框
  var PORTED = {                            // 沒有被別的補件停用的鈕
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
  function pageTab() {                                   // 頁面上 PC 目前的頁籤（這一頁 PC 是空的 ⇒ 永遠 null）
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
        if (v.visible !== undefined) setVis(el, !!v.visible);  if (v.tag !== undefined && el.classList && el.classList.contains('aled')) el.classList.toggle('on', !!v.tag);  if (v.tag !== undefined) ct3dLook(id, el, v.tag);   // AI(W906-B8-CT3A) 20261001 [W906] (St01): ledOneCycle (golden TMyLed ->Value) -- C++ puts the value on the proxy Tag (golden cContact.cpp:17155 / :1633); same line  //AI(W906-B8-CT3D) 20261001 [W906] (St01): + ledOTD colour / palOTD_4 palOTD_6 bevel from the proxy Tag (golden OTDTimerTimer :15467-15499 / palOTD_*Click BevelOuter, see ct3dLook); same line
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
    var todo = (a && a.todo) || [];  if ((item.control === 'btnStart' || item.control === 'btnPause') && a && Array.isArray(a.afterAck) && a.afterAck.length) say2(NAME + ' ' + (item.control === 'btnStart' ? 'START' : 'PAUSE') + '：' + a.afterAck.join(' | '), '#9f9');   // AI(W906-B8-CT3B) 20261001 [W906] (St01): golden btnStartClick / btnPauseClick run right after this reply (form.event after-ack, FileRW/DeviceForm_File.cpp tail); show what the server queued; same line
    if (item.control !== 'pnlSensorAdj') return;
    if (todo.some(function (t) { return /^open:cclink\b/.test(t); })) {   // CT-L1：伺服器照 golden 查過等級 109
      if (window.parent && window.parent !== window) window.parent.postMessage({ open: 'cclink' }, '*');
      else say2('Sensor Adjustment：單獨開頁，沒有 background.html 可以開 Shuttle Sensor Utility（cclink）');
    } else {
      say2('Sensor Adjustment：伺服器沒有放行（golden 等級 109 不足時跳 WAR1676）', '#f88');
    }
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
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
      afterAck(item, a);
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
    if (last && last.event === item.event && JSON.stringify(last.val) === JSON.stringify(item.val)) return false;   // 連點：同一個值還沒回覆
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
  function reopenPorted() {                              // 這一頁沒有被別的補件停用、要重新打開的鈕（PORTED 是空的；形狀同 ht9045_cleaning_ev.js）
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
  }  function ct3aLoad() { var px = (LAST && LAST.proxies) || {}; ['btnTStart', 'btnTStep', 'spbOneCycle', 'ledOneCycle', 'btnStart', 'btnPause'].forEach(function (id) { var el = $(id), p = px[id]; if (!el || !p) return; if (p.visible === true) setVis(el, true); if (p.tag !== undefined && el.classList && el.classList.contains('aled')) el.classList.toggle('on', !!p.tag); });  ct3dLoad(); }  function ct3dLook(id, el, tag) { var t = typeof tag === 'number' ? tag : 0; if (id === 'ledOTD') { el.classList.toggle('on', t !== 0); if (el.style && el.style.setProperty) el.style.setProperty('--led-on', t === 2 ? '#ff0000' : '#00ff00'); } else if ((id === 'palOTD_4' || id === 'palOTD_6') && el.style) el.style.borderStyle = t === 1 ? 'inset' : 'outset'; }  function ct3dLoad() { var px = (LAST && LAST.proxies) || {}; ['palOTD_4', 'palOTD_6', 'ledOTD'].forEach(function (id) { var el = $(id), p = px[id]; if (el && p) ct3dLook(id, el, p.tag); }); var lo = $('ledOTD'); if (lo && !lo.__ct3d) { lo.__ct3d = true; lo.addEventListener('click', function (ev) { if (ev && ev.stopPropagation) ev.stopPropagation(); }); } }   // AI(W906-B8-CT3A) 20261001 [W906] (St01): B8 CT-3a -- T.Start / T.Step are display:none in the generated page (DFM Visible=False); the engine only sets visibility, so show them when golden FormShow made them visible ([D16] :1630-1631, SOFT_SIMULTE :1664-1665); One Cycle LED from the proxy Tag; same line  //AI(W906-B8-CT3B) 20261001 [W906] (St01): + btnStart / btnPause (golden FormShow [D16] :1628-1629, SOFT_SIMULTE :1662-1663; display:none in the generated page, DFM Visible=False); same line  //AI(W906-B8-CT3D) 20261001 [W906] (St01): + ct3dLook / ct3dLoad (ct3dLoad runs at the end of ct3aLoad, i.e. after every load) -- golden TMyLed ledOTD (Value + TrueColor, OTDTimerTimer :15445-15507) is the proxy Tag 0 off / 1 clLime / 2 clRed; palOTD_4 / palOTD_6 BevelOuter (palOTD_4Click :15395 / palOTD_6Click :15420) is the panel Tag = VCL TBevelCut 1 bvLowered (inset) / 2 bvRaised; 0 = never clicked = DFM default bvRaised (a TPanel tag is only sent when non-zero); a click on the lamp stops there (golden TALed is a TGraphicControl, elec\Component\aled.pas:24, enabled, no OnClick in cContact.dfm:16347-16353: VCL gives the click to the lamp, palOTD_4Click does not run); same line

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      setTimeout(function () {                           // 引擎在這個 promise 的 then 裡同步套完值
        try { hook(); reopenPorted(); ct3aLoad(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }   // AI(W906-B8-CT3A) 20261001 [W906] (St01): + ct3aLoad (above); same line
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error(NAME + ' 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個事件在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    if (widgets && typeof widgets === 'object' && pcSupported() && !widgets[PC]) {   // 這一頁 PC 是空的 ⇒ 不會進來
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

  window.HT9045EvB3Contact = {                          // 探針／除錯用
    state: function () { return stateNow(''); }, queue: function () { return QUEUE.slice(); },
    inflight: function () { return INFLIGHT; }, known: function () { return KNOWN; }, applyChanged: applyChanged, pageTab: pageTab
  };
})();
// =============================================================================================
// AI(W906-W152) 20261007 (St02-E; stand-in claim for St01, FROM_STEVEN s1 W-152): W-152 (B) -- re-read this page after a contact /
//   auto-height run.  A UI-only port mechanism with a golden-equivalent result: golden has ONE TfContact form, so the run's values
//   are on the operator's screen at once; here the run writes the facade, case 1800 copies the result to the C-route stand-ins
//   (FileRW/DeviceForm_File.cpp EOF) and bumps tag contact.runResultSeq (tools/wb_serve.cpp PublishExtraTags, only while this
//   page is open).  On a change after the first value: wait until this page has no form.event queued or in flight, then the
//   engine's own full read (HT9045Wire.reload = load()); the server answers with golden FormShow + the kept run result.
//   Nothing is saved (golden saves only on spbSaveClick :14072).  Values typed here but not yet sent are replaced by the re-read.
// =============================================================================================
(function () {
  'use strict';
  var T = window.HT9045Tags, LOGP = '[W152 contact re-read] ';
  if (!T || typeof T.on !== 'function') return;
  var last = null, pending = false;
  function busy() {
    var P = window.HT9045EvB3Contact;
    return !!(P && ((P.queue && P.queue().length) || (P.inflight && P.inflight())));
  }
  function reread() {
    var W = window.HT9045Wire;
    if (!W || typeof W.reload !== 'function') { pending = false; return; }
    if (busy()) { setTimeout(reread, 300); return; }
    pending = false;
    Promise.resolve(W.reload()).then(function () {
      if (W.say) W.say('自動測高／接觸測試結果已更新到畫面（尚未存檔：要保留請按存檔）', '#9f9', 'sticky');
    }, function (e) { if (window.console) console.warn(LOGP + 'reload failed', e); });
  }
  T.on('contact.runResultSeq', function (v) {
    if (typeof v !== 'number') return;
    if (last === null) { last = v; return; }          // the first value (the snapshot when the page opens) is not a new result
    if (v === last) return;
    last = v;
    if (!pending) { pending = true; reread(); }
  });
})();
