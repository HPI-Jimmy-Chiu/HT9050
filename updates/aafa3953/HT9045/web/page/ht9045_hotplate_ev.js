/* ht9045_hotplate_ev.js -- Setup.HotPlate.html（golden TfHotPlate，V912 cHotPlate.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB2) 20260928 [W906] St01 新檔（手寫）。批次 B2 的 Q40-HP（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B2、第四節 Q40-HP 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」。
 *   C++ 那一半已做（76058840）：A 形狀事件表 kEvents（FileRW/HotPlateForm_File.cpp；設定 tools/formbridge/TfHotPlate.py 'events'）、
 *   本體 JsonBridge/FormBridge.cpp RunEvent。這個檔只補頁面那一半。寫法照 St02 的 ht9045_config_q41.js :117-163
 *   （St02 分支 v906/steven-gpib-widget）。引擎 ht9045_wire_engine.js、舊接線檔 ht9045_hotplate_wire.js（St02 的 HP-2 在裡面）不改。
 *   頁面 Setup.HotPlate.html:126 同一行載入（在接線資料檔之後），行數不變。
 *
 * Q40-HP：「從資料庫選」下拉 cbSelectHPFromDB 選一筆（golden cbSelectHPFromDBChange :412-438：fShow 時、PlateForm.csv 第 ItemIndex 列
 *   的 7 個欄位填進 HotPlateName／XST1／YST1／XPitch1／YPitch1／XCT1／YCT1；ItemIndex<1（第 0 列是表頭）什麼都不做）。
 *   送 {"form":"TfHotPlate","control":"cbSelectHPFromDB","event":"change","itemIndex":4,"text":"<選項文字>","state":{…}}
 *   （tag "Setup.HotPlate"；itemIndex＝golden 清單裡的位置〔對 /api/form widgets.cbSelectHPFromDB.items〕、引擎補的清單外文字送 -1；
 *   text＝選項文字，伺服器拿它做「清單過期」檢查，R78）。
 *   A 形狀：伺服器每次事件先跑一次 golden FormShow（R79：重讀配方 HotPlate.Data、缺鍵補寫、重算 Y pitch），再套 state 與控制項值、
 *   跑處理器，回處理器賦值的屬性（ack.changed 只有那 7 個欄位的 text）。這一頁沒有 editlist.get 的 events／operable，
 *   點不點得到由伺服器照 golden DFM＋FormShow 判斷（EventGuard），頁面只看元件自己有沒有被停用。
 * state ＝引擎 form.save 會送的那一包（/api/form 的 saveReads 裡有可信來源的，值的讀法同引擎 widgetValue），控制項自己不放。
 * 開頁：/api/form 的 cbSelectHPFromDB 若是 itemIndex 對不上 text（golden FormShow :43-45 ItemIndex=0 → Items->Clear() → Text=
 *   "Select from Database..."；VCL 清掉清單後 ItemIndex＝-1），在引擎套值之前改成 -1，畫面才會停在 "Select from Database..."
 *   ——不然選中的是第 0 列（表頭），選同一列不會觸發 change。同 Setup.Temp_Set.html inline fixCombos 的作法。
 * 套 ack.changed 時不補發 change／input：St02 的 HP-2（ht9045_hotplate_wire.js，XCT1 是 "1" 就把 XPitch1 設 0；golden XCT1MouseDown
 *   只在小鍵盤之後）聽 change，golden 用程式填值不會跑它。
 * 其他規則同 ht9045_trayassign_ev.js：一次一個、等 ack；同一個值還沒回覆＝連點丟掉；busy: 等 450 ms 重送（最多 3 次）；
 *   not-operator 續權杖再送一次；錯誤含 "reload page" → HT9045Page.load()（重讀檔＋/api/form）；其他失敗改回原來的選項並說原因；
 *   存檔（form.save）時還有事件在等回覆 → 這次不存。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.form || !R.formSave || !R.rawCmd || R.__evb2HotPlate) return;
  R.__evb2HotPlate = true;

  var PAGE = 'Setup.HotPlate.html';         // /api/form／form.save 的頁名（引擎 CFG.page）
  var EV_TAG = 'Setup.HotPlate';            // form.event 的 tag（FileRW/_FormEvent.cpp：FindBridge(tag)／(tag + ".html")）
  var FORM = 'TfHotPlate';
  var CTL = 'cbSelectHPFromDB', EVN = 'change';
  var LOG = '[HotPlate/EVB2] ';
  var MAX_WS = 60000;                       // 伺服器單則 WS 上限 64 KB（WebBridgeServer.cpp kMaxWsMessage）

  var FORMD = null, GEN = 0, QUEUE = [], INFLIGHT = null, APPLYING = false, KNOWN = null, OFF = '';

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
  function usable(el) { return !!el && !el.disabled && el.getAttribute('aria-disabled') !== 'true' && !ancestorDisabled(el); }
  function gateOn() {                                    // /api/form 有這一頁的第二型 bridge、讀檔端沒有缺口
    return !!FORMD && FORMD.kind === 'golden-bridge' && FORMD.available !== false && !FORMD.sourceGap;
  }
  function items() { var w = FORMD && FORMD.widgets && FORMD.widgets[CTL]; return w && Array.isArray(w.items) ? w.items : null; }

  /* ---- 值 --------------------------------------------------------------- */
  function readCtl() {                                   // golden ItemIndex／Text；dom＝改回用
    var sel = $(CTL);
    if (!sel || sel.tagName !== 'SELECT') return null;
    var o = sel.options[sel.selectedIndex], dom = sel.selectedIndex;
    if (!o) return { itemIndex: -1, text: '', dom: dom };
    var t = o.textContent, it = items();
    if (o.getAttribute('data-src') === 'cpp-text') return { itemIndex: -1, text: t, dom: dom };
    if (it) return { itemIndex: it[dom] === t ? dom : it.indexOf(t), text: t, dom: dom };
    return { itemIndex: dom, text: t, dom: dom };
  }
  function writeCtl(v) {
    var sel = $(CTL);
    if (!sel || !v || v.dom === undefined) return;
    APPLYING = true;
    try { sel.selectedIndex = v.dom; } finally { APPLYING = false; }
  }
  function payloadOf(v) { return v ? { itemIndex: v.itemIndex, text: v.text } : {}; }
  function widgetValue(el) {                             // 同引擎 widgetValue（A 形狀 form.save 的值；沒有匯出）
    if (el.tagName === 'SELECT') {
      var o = el.options[el.selectedIndex];
      return { itemIndex: el.selectedIndex, text: o ? o.textContent : '' };
    }
    var cb = el.tagName === 'INPUT' && el.type === 'checkbox' ? el : el.querySelector('input[type="checkbox"]');
    if (cb && !('value' in el && el.tagName === 'INPUT' && el.type !== 'checkbox')) return { checked: cb.checked };
    var rs = el.querySelectorAll ? el.querySelectorAll('input[type="radio"]') : [];
    if (rs.length) {
      for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i };
      return { itemIndex: -1 };
    }
    if ('value' in el) return { text: String(el.value) };
    return null;
  }
  function stateNow() {                                  // 同引擎 bridgeSave 的挑法：saveReads 裡有 C++ 值或檔案接線來源的
    var ids = (FORMD && FORMD.saveReads) || [], out = {};
    var fields = (window.HT9045Page && HT9045Page.cfg && HT9045Page.cfg.fields) || {};
    ids.forEach(function (id) {
      if (id === CTL) return;
      var el = $(id), fw = FORMD.widgets && FORMD.widgets[id];
      var cppVal = fw && (fw.text !== undefined || fw.checked !== undefined || fw.itemIndex !== undefined);
      if (!el || !(cppVal || fields[id])) return;
      var v = widgetValue(el);
      if (v) out[id] = v;
    });
    return out;
  }

  /* ---- 套 ack.changed（A 形狀：處理器有賦值的屬性） ------------------------ */
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
  function applyChanged(ch) {                            // {元件:{text,itemIndex,checked,caption,visible,enabled,items,…}}
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id] || {}, el = $(id);
        if (!el) return;
        if (v.items !== undefined && el.tagName === 'SELECT' && Array.isArray(v.items)) {
          var keep = el.options[el.selectedIndex] ? el.options[el.selectedIndex].textContent : null;
          el.innerHTML = '';
          v.items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; el.appendChild(o); });
          el.selectedIndex = keep === null ? -1 : v.items.indexOf(keep);
        }
        if (el.tagName === 'SELECT' && (v.itemIndex !== undefined || v.text !== undefined)) {
          if (v.itemIndex !== undefined && v.itemIndex >= 0 && v.itemIndex < el.options.length) el.selectedIndex = v.itemIndex;
          else if (v.text !== undefined) {
            var found = false;
            for (var j = 0; j < el.options.length; j++) if (el.options[j].textContent === v.text) { el.selectedIndex = j; found = true; break; }
            if (!found) { var o2 = document.createElement('option'); o2.textContent = v.text; o2.setAttribute('data-src', 'cpp-text'); el.appendChild(o2); el.selectedIndex = el.options.length - 1; }
          }
        } else if (v.text !== undefined && 'value' in el && !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) {
          el.value = String(v.text);                     // 不補發 change（見檔頭）
        }
        if (v.checked !== undefined) { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); if (c) c.checked = !!v.checked; }
        if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;
        if (v.visible !== undefined) el.style.visibility = v.visible ? '' : 'hidden';
        if (v.enabled !== undefined) { if (v.enabled) setEditable(el, true); else dis.push(el); }
      });
      dis.forEach(function (el) { setEditable(el, false); });
    } finally { APPLYING = false; }
  }

  /* ---- 佇列 -------------------------------------------------------------- */
  function latest() { return QUEUE.length ? QUEUE[QUEUE.length - 1] : INFLIGHT; }
  function idle() { return !INFLIGHT && !QUEUE.length; }
  function refreshKnown() { if (!QUEUE.length) KNOWN = readCtl(); }
  function fail(item, msg) {
    QUEUE = [];
    if (item.gen === GEN) { writeCtl(item.prev); KNOWN = item.prev; }
    say2('Hot Plate 從資料庫選的那一筆沒有送到伺服器（form.event）：' + msg + ' —— 已改回原來的選項，欄位沒有填', '#f88');
  }
  function send(item, tries) {
    var v = { form: FORM, control: CTL, event: EVN, itemIndex: item.val.itemIndex, text: item.val.text, state: stateNow() };
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    if (utf8Len(JSON.stringify({ type: 'cmd', id: 999999, cmd: 'form.event', tag: extra.tag, value: extra.value })) > MAX_WS) {
      fail(item, '這次要送的畫面值超過伺服器單則上限 64 KB');
      return Promise.resolve(null);
    }
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;                 // 期間重開過頁：/api/form 已經給了新狀態
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      if (a.todo && a.todo.length && window.console) console.info(LOG + 'todo: ' + a.todo.join(' | '));
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
        say2('Hot Plate：伺服器要求重新開頁（' + msg + '）—— 重讀中');
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

  /* ---- 接線 --------------------------------------------------------------- */
  function onChange(ev) {
    if (APPLYING) return;
    if (ev && ev.isTrusted === false) return;            // 別的程式 dispatch 的 change 不是使用者選的
    var el = $(CTL);
    if (!el || !gateOn()) return;                        // 沒有 golden bridge：照舊（選了不會填）
    var cur = readCtl(), prev = KNOWN;
    if (OFF || !usable(el)) {
      writeCtl(prev);
      say2('Hot Plate 從資料庫選：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）' : '這個下拉現在停用') + ' —— 已改回', '#f88');
      return;
    }
    if (!cur || (prev && JSON.stringify(payloadOf(prev)) === JSON.stringify(payloadOf(cur)))) return;
    var last = latest();
    if (last && JSON.stringify(last.val) === JSON.stringify(payloadOf(cur))) return;   // 連點：同一筆還沒回覆
    QUEUE.push({ val: payloadOf(cur), prev: prev, gen: GEN });
    KNOWN = cur;
    pump();
  }
  function hook() {
    var el = $(CTL);
    if (el && !el.__evb2) { el.__evb2 = true; el.addEventListener('change', onChange); }
  }
  function fixCombo(d) {                                 // 見檔頭「開頁」
    var w = d && d.widgets && d.widgets[CTL];
    if (w && Array.isArray(w.items) && w.itemIndex !== undefined && w.itemIndex >= 0 && w.text !== undefined && w.items[w.itemIndex] !== w.text) w.itemIndex = -1;
  }

  var form0 = R.form, fsave0 = R.formSave;
  R.form = function (page) {
    var pr = form0.apply(this, arguments);
    if (page !== PAGE) return pr;
    return pr.then(function (d) {
      FORMD = d; GEN++; QUEUE = []; OFF = '';
      try { fixCombo(d); } catch (e) { if (window.console) console.error(LOG + 'fixCombo', e); }   // 引擎 formOverlay 套值之前（這個 then 先跑）
      setTimeout(function () { try { hook(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); } }, 0);
      return d;
    });
  };
  R.formSave = function (page) {
    if (page !== PAGE) return fsave0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Hot Plate 還有「從資料庫選」事件在等伺服器回覆（form.event）—— 這次沒有存檔；欄位填好之後再按一次存檔'));
    return fsave0.apply(this, arguments);
  };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045EvB2HotPlate = {                           // 探針／除錯用
    gate: gateOn, state: stateNow, queue: function () { return QUEUE.slice(); }, inflight: function () { return INFLIGHT; },
    known: function () { return KNOWN; }, applyChanged: applyChanged
  };
})();
