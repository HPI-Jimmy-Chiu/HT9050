/* ht9045_barcode_ev.js -- Setup.BarCode.html（golden TfBarCode，V912 BarCode\BarCode.cpp）的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB3) 20260928 [W906] St01 新檔（手寫）。批次 B3 的 BC-1／BC-3（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B3、第四節 BC-1／BC-3 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。
 *   C++ 那一半：事件表 kBC_Events（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.gen.inc 檔尾，設定
 *   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\tools\editlist\TestIF_File_BarCode.py 'events'）、註冊與按鈕替身
 *   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.cpp 檔尾、本體 FileRW\_EditPage.cpp RunPageEvent。
 *   寫法照 St02 的 ht9045_config_q41.js :117-163（St02 分支 v906/steven-gpib-widget）與 B2 的 ht9045_trayform_ev.js。
 *   引擎 ht9045_wire_engine.js、同頁的 ht9045_barcode_c.js 不改。頁面 Setup.BarCode.html:126（St01 0609a14f 那一行）同一行載入。
 *
 *  BC-1  chkMulti2DID（勾「多 2DID」）click → golden chkMulti2DIDClick（:8825）：tsMulti2DID 頁籤看不看得見。
 *        rgMulti2DType（2DID 類型）click → golden rgMulti2DTypeClick（:8861）→ SetMulti2DMap：cbAa／cbAb／cbBa／cbBb 的選項重建、
 *        選第 0 項、依類型藏起用不到的那幾個。
 *        送 {"form":"TfBarCode","control":"chkMulti2DID","event":"click","checked":true}
 *           {"form":"TfBarCode","control":"rgMulti2DType","event":"click","itemIndex":3}
 *        以前只在存檔前由伺服器重播（BeforeApply），畫面不會當場變；送過之後伺服器端＝頁面值，存檔時不再重播。
 *  BC-3  btResetBarCodeCount（「Reset Count」，在 gbManualTest 裡：只有軟體模擬版、等級夠才看得見）click
 *        → golden btResetBarCodeCountClick（:2424）：讀碼計數 iBarCodeNo[4][8] 歸零。按下即生效、不寫檔（golden 同）。
 *        送 {"form":"TfBarCode","control":"btResetBarCodeCount","event":"click"}
 *  Exit  AI(W906-EVB10C-BC) 20260930：sbtExit（Exit）click → golden sbtExitClick（:2408-2417）：Close()＋2DID 檢查工作重設＋兩顆檢查鈕重新可按；
 *        捕獲階段攔 .exitbtn，ack.closed 才關視窗（檔尾「Exit 鈕」段）。
 *  不帶 state：這三支 golden 處理器只看自己（勾選／單選／沒有值），不讀別的元件；存檔時頁面照舊送整包。
 *  ack.changed 照引擎 gbApply 的鍵套回（items／itemIndex／text／checked／visible／editable／tabVisible），套值時不補發 change。
 *  其他規則同 ht9045_trayform_ev.js：一次一個、等 ack；同一個值還沒回覆＝連點丟掉；busy: 等 450 ms 重送（最多 3 次）；
 *    not-operator 續權杖再送一次；"reload page" → HT9045Page.load()（editlist.get）；events.<id>.operable＝false 不送；
 *    其他失敗改回點之前的值並說原因；存檔時還有事件在等回覆 → 這次不存。
 *  伺服器沒有事件表（C++ 還沒進來）＝一個都不送，照舊（存檔時伺服器重播 BC-1）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb3BarCode) return;
  R.__evb3BarCode = true;

  var STRUCT = 'TestIF_File_BarCode', FORM = 'TfBarCode', EV_TAG = 'Setup.BarCode', LOG = '[BarCode/EVB3] ';
  var CTLS = [['chkMulti2DID', 'click'], ['rgMulti2DType', 'click'], ['btResetBarCodeCount', 'click']];   // ＝kBC_Events

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
  function ancestorDisabled(el) {
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function usable(el) {
    if (!el || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) return false;
    var x = (el.tagName === 'INPUT' || el.tagName === 'SELECT' || el.tagName === 'BUTTON') ? el : el.querySelector('input,select,button');
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
  function readCtl(id) {                                 // TRadioGroup → itemIndex、TCheckBox → checked、TButton → {}
    var el = $(id);
    if (!el) return null;
    var rs = radiosOf(el);
    if (rs.length) { for (var i = 0; i < rs.length; i++) if (rs[i].checked) return { itemIndex: i }; return { itemIndex: -1 }; }
    var c = checkboxOf(el);
    if (c) return { checked: c.checked };
    return {};
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

  /* ---- ack.changed → 畫面（同引擎 gbApply 的鍵） ------------------------ */
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
    return document.querySelector('.tab[data-htitle^="' + id + ' :"]') || document.querySelector('.tab[title^="' + id + ' :"]');
  }
  function setTabVisible(id, on) {                       // golden TTabSheet->TabVisible（例 chkMulti2DIDClick 的 tsMulti2DID）
    var tab = tabOf(id);
    if (!tab) return;
    tab.style.display = on ? '' : 'none';
    if (!on && tab.classList.contains('act')) {
      var first = tab.parentElement.querySelector(':scope > .tab:not([style*="display: none"])');
      if (first && first !== tab) first.click();
    }
  }
  function setItems(el, items) {                         // golden SetMulti2DMap：Clear()＋Items->Add（選取由 itemIndex 另外套）
    while (el.options.length) el.remove(0);
    items.forEach(function (t) { var o = document.createElement('option'); o.textContent = t; o.setAttribute('data-src', 'ev-items'); el.appendChild(o); });
    el.selectedIndex = -1;
  }
  function setIndex(el, i, text) {
    if (el.tagName === 'SELECT') {
      if (i >= 0 && i < el.options.length && (text === undefined || el.options[i].textContent === text)) { el.selectedIndex = i; return; }
      el.selectedIndex = -1;
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
        else if (v.text !== undefined && 'value' in el && !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) el.value = String(v.text);
        if (v.checked !== undefined) { var c = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"],input[type="radio"]'); if (c) c.checked = !!v.checked; }
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
        var e = evInfo(id);
        if (e && v.editable !== undefined) e.operable = !!v.editable;
      });
      dis.forEach(function (el) { setEditable(el, false); });   // 先開後關（同引擎 gbLoad）
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
    say2('Bar Code ' + item.control + ' 沒有送到伺服器（form.event）：' + msg + ' —— 已改回原來的值', '#f88');
  }
  function send(item, tries) {
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      else if (item.control === 'btResetBarCodeCount') say2('讀碼計數（4×8）已歸零（golden btResetBarCodeCountClick；不寫檔）', '#9f9');
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
        say2('Bar Code：伺服器要求重新開頁（' + msg + '）—— 重讀中');
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
    if (last && last.event === item.event && JSON.stringify(last.val) === JSON.stringify(item.val)) return false;   // 連點丟掉
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function onUser(id, evn) {
    return function (ev) {
      if (APPLYING) return;
      if (ev && ev.isTrusted === false) return;          // 別的程式 dispatch 的事件不是使用者點的
      var el = $(id);
      if (!el || !gateOn()) return;                      // 伺服器沒有事件表：照舊（存檔時伺服器重播 BC-1）
      var cur = readCtl(id), prev = KNOWN[id];
      if (OFF || !canSend(id, evn) || !usable(el)) {
        var e = evInfo(id);
        writeCtl(id, prev);
        say2('Bar Code ' + id + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
              : !e ? '伺服器的事件表沒有這個元件' : e.event !== evn ? '伺服器的事件是 ' + e.event + '，不是 ' + evn
              : '伺服器說這個元件現在點不到（golden 點不到，events.operable=false）') + (prev ? ' —— 已改回' : ''), '#f88');
        return;
      }
      var isBtn = !(cur && (cur.itemIndex !== undefined || cur.checked !== undefined));
      if (!isBtn && prev && cur && JSON.stringify(payloadOf(prev)) === JSON.stringify(payloadOf(cur))) return;   // VCL：值沒變不觸發
      if (enqueue({ control: id, event: evn, val: payloadOf(cur), prev: isBtn ? null : prev, gen: GEN }) && !isBtn) KNOWN[id] = cur;
    };
  }
  function hook() {
    CTLS.forEach(function (c) {
      var el = $(c[0]);
      if (!el || el.__evb3) return;
      el.__evb3 = true;
      var isBtn = el.tagName === 'BUTTON' || !(radiosOf(el).length || checkboxOf(el));
      el.addEventListener(isBtn ? 'click' : 'change', onUser(c[0], c[1]));   // 單選群組：input 的 change 冒泡到容器
    });
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      setTimeout(function () {                           // 引擎在這個 promise 的 then 裡同步套完值
        try { hook(); refreshKnown(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Bar Code 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個事件在等伺服器回覆（form.event）—— 這次沒有存檔；等畫面更新好再按一次存檔'));
    return save0.apply(this, arguments);
  };

  /* ---- AI(W906-EVB10C-BC) 20260930 [W906] St01：Exit 鈕 sbtExit ＝ golden TfBarCode::sbtExitClick（V912 BarCode\BarCode.cpp:2408-2417）------------
   *   B10c 後續（D:\docs\ChangeLog\CHANGES_20260929_Steven.md §11.58g「後續」）；Steven 20260928「任何畫面的事件, 都是我們做」、
   *   20260929「請按照bcb的邏輯處理」。寫法照 SetUp 的 SU-9（D:\HT9045\web\page\ht9045_setup_c_wire.js 第 (6) 段）。
   *   頁面內建的 .exitbtn（Setup.BarCode.html 產生器那一段）會直接叫外框關視窗（C++ 什麼都沒跑）→ 捕獲階段攔下，改送 WS form.event
   *   {"form":"TfBarCode","control":"sbtExit","event":"click"}。golden：Close()（→ FormClose：bShow=false、tmr1 關、DoIniDataToForm）、
   *   bShow=false、i2DIDCheckSH1Task／SH2Task=1、兩顆「Check 2DID SH1／SH2」鈕重新可按。golden 這支沒有擋關的條件 ⇒ 一定關窗：
   *     ack.closed＝true → 關；伺服器沒有這個事件（舊版）、回「reload page」（伺服器端這一頁沒開著）或 running（運轉中不收 form.event）
   *     → 照舊直接關（關窗那一下伺服器由頁面表的關窗邊緣照 golden 跑 FormClose，✕ 也走那一條）；其他失敗 → 說原因、1.2 秒後照樣關。
   *   C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_BarCode.cpp 檔尾、產生檔 BC_sbtExitClick。
   *   防連點：送出到回覆之間再按不送（WebCmdGuard 另外擋同一個指令）；busy: 等 450 ms 重送（最多 3 次）；not-operator 續權杖再送一次。 */
  var exitBusy = false;
  function exitCloseWin() { if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); }
  function exitSend(v, tries) {
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) { return unwrap(m) || {}; }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return exitSend(v, tries + 1); });
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) return R.keepAlive().then(function () { return exitSend(v, tries + 1); });
      throw new Error(msg);
    });
  }
  function exitClick() {
    if (exitBusy) return;
    if (OFF || !canSend('sbtExit', 'click')) { exitCloseWin(); return; }   // 伺服器沒有這個事件（或說點不到）：照舊（關窗邊緣跑 FormClose）
    exitBusy = true;
    exitSend({ form: FORM, control: 'sbtExit', event: 'click' }, 0).then(function (a) {
      exitBusy = false;
      var ms = (a.messages || []).map(function (x) { return x.zh || x.en; }).filter(Boolean);
      if (ms.length) say2(ms.join('\n'));
      if (a.todo && a.todo.length && window.console) console.info(LOG + 'sbtExit todo: ' + a.todo.join(' | '));
      if (!a.closed && window.console) console.warn(LOG + 'sbtExit: golden sbtExitClick did not report Close() -- closing anyway (golden always closes)');
      exitCloseWin();
    }, function (e) {
      exitBusy = false;
      var msg = (e && e.message) || String(e);
      if (/reload page|^running/.test(msg)) { exitCloseWin(); return; }   // 伺服器端這一頁沒開著／運轉中：關窗邊緣照 golden 跑 FormClose
      say2('⚠ Exit：伺服器端 sbtExitClick 沒有跑（' + msg + '）；視窗照樣關閉（關窗時伺服器照 golden 跑 FormClose）', '#ffcc66');
      setTimeout(exitCloseWin, 1200);
    });
  }
  // 捕獲階段掛在 window：比頁面內建 .exitbtn 的 click（掛在按鈕上、直接 postMessage closeMe）先跑，並擋住它（同 ht9045_setup_c_wire.js 第 (6) 段）
  window.addEventListener('click', function (ev) {
    var t = ev.target && ev.target.closest ? ev.target.closest('#sbtExit') : null;
    if (!t) return;
    ev.stopPropagation();
    ev.preventDefault();
    exitClick();
  }, true);

  window.HT9045EvB3BarCode = {                            // 探針／除錯用
    gate: gateOn, queue: function () { return QUEUE.slice(); }, inflight: function () { return INFLIGHT; },
    known: function () { return KNOWN; }, applyChanged: applyChanged, exitClick: exitClick
  };
})();
