/* ht9045_config_st01_ev.js -- Config.Configuration.html（golden TfConfiguration，V912 cConfiguration.cpp）St01 的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB4) 20260928 [W906] St01 新檔（手寫）。批次 B4 的 CC-E1／CC-E3／CC-E4／CC-E6／CC-E8＋CC-L2 的頁面那一半
 *   （D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B4、第四節各列）。
 *   Steven 20260928：「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上, 如果沒有移植的, 我們直接實作」。
 *   C++ 那一半：事件表 kIC_Events（FileRW/IniConfig.gen.inc 檔尾，設定 tools/editlist/IniConfig.py 'events'）、註冊與開頁／存檔補重播
 *   FileRW/IniConfig.cpp 檔尾 (1)～(6)。寫法照 B2 的 ht9045_trayassign_ev.js（1b213d11）。引擎 ht9045_wire_engine.js 不改。
 *   頁面 Config.Configuration.html:126 同一行載入（接線資料檔之後），行數不變。
 *
 * 送什麼（WS cmd "form.event"，tag＝editlist.get IniConfig 回的 eventTag，也就是 "Config.Configuration"；送 "IniConfig" 會回 unknown-page）：
 *   CC-E1 btD47 click（golden btD47Click :6062：Socket 已測次數歸零）                {"form":"TfConfiguration","control":"btD47","event":"click"}
 *   CC-E3 btnRecordJamRateByTimeClear click（:6691：Jam 率計數歸零；DFM 字是 'Set All'）  {"form":"TfConfiguration","control":"btnRecordJamRateByTimeClear","event":"click"}
 *   CC-E4 btResume click（:6257：手動解除伺服器鎖機，只有 CC_MTI 看得到）              {"form":"TfConfiguration","control":"btResume","event":"click"}
 *   CC-E6 cbA09／chkA09_1／cbA14 click（:6453 cbA09Click，golden 三格綁同一支）         {"form":"TfConfiguration","control":"cbA09","event":"click","checked":true}
 *         機台內還有 IC ⇒ 伺服器照 golden 把 cbA09 改回（ack.changed 帶回），並跳 MES1645／MES1646（告警框）。
 *   CC-E8 8 條 EP 滑桿 change（:6271-:6684 tbD25_*／tbD60_* Change；7e1785dc 起 form.event 收 position）
 *         {"form":"TfConfiguration","control":"tbD25_Index60mm","event":"change","position":97}
 *         瀏覽器的 range 在放開時才發 change ⇒ 一次拖動送一次（golden TTrackBar 拖動中每一格都 OnChange；R 題）。
 *   沒有 state：這幾支 golden 處理器除了自己之外只讀記憶體（IniConfig／LastSet／機台有沒有料）和事件表上的勾選框（Timer1 一拍讀
 *   cbD21／cbD47／cbF05，它們由 St02 的 ht9045_config_q41.js 各自送事件同步）。Configuration 頁約 1400 個替身，整包 state 會逼近
 *   伺服器單則 64 KB 上限；St02 那支也不帶。B2 規則「state 不放事件表上的控制項」在這裡等於空集合。
 *
 * 控制項歸屬（同一頁兩支送出點，不重複掛）：
 *   St02 ht9045_config_q41.js（St02 分支 v906/steven-gpib-widget）：udD46（btNext／btPrev）、cbE30／cbE31／cbE31_1／cbE31_2／cbE32／
 *     cbE32_1／cbE32_2／cbE33、cbE39、cbD36／cbD37／cbD38／cbD36_1／cbD36_2、cbD21／cbD47／cbF05（CC-E2／CC-E7），btnAutoSaveSetAll（CC-E9，
 *     頁面本地）、edtSearchFunction（CC-E14，頁面本地）。
 *   本檔：btD47、btnRecordJamRateByTimeClear、btResume、cbA09、chkA09_1、cbA14、tbD25_Index30mm／40mm／60mm、tbD25_Index30mm_NS／
 *     40mm_NS／60mm_NS、tbD60_Index56mm、tbD60_Index56mm_NS（CC-E1／E3／E4／E6／E8），以及 CC-L2 的標示（下面）。
 *
 * CC-L2（Q45 #1～#3、#5＝C：Steven 20260928「按照你的建議執行」＝維持改不了、網頁說明原因；不送事件、不問密碼）：
 *   cbC12 群組（cbC12／cbC13／chkC14／cbC17／cbC24，golden :6775 cbC12Click）、cbA27（:6824）、cbN07_EnableEmployeeCheak（:6860）、
 *   Image1 連點兩下（:7274 Image1DblClick，SG_PW.ini）。開頁把這幾格標成「要廠商密碼，網頁版不提供」；使用者點了會落到 C++ 拒存
 *   （FileRW/IniConfig.cpp IC_PasswordGuard）的狀態 ⇒ 當場改回並說明（跟 IC_PasswordGuard 同一條規則，比的是開頁值；C++ 照舊拒存，
 *   頁面只是第二道）。規則：cbA27／N07-5 跟開頁值不同就改回；C12 群組點完之後 C12 勾著、而且（C12 開頁沒勾，或另外四格有一格跟開頁值
 *   不同）就改回 —— 先取消 C12 再改另外四格，golden 不用密碼也做得到，所以放行。
 *
 * 其他規則（同 B2）：只收使用者真的點的（isTrusted）；一次一個、等 ack 再送下一個；同一個控制項同一個值還沒回覆又來一次＝連點，丟掉
 *   （按鈕：還有一個在等就丟掉）；busy:（WebCmdGuard 400 ms）等 450 ms 重送、最多 3 次；not-operator 先續權杖再送一次；錯誤含
 *   "reload page" → 重開頁（HT9045Page.load()＝editlist.get）；其他失敗（running、點不到、handler-failed）→ 把那個控制項改回點之前的值、
 *   狀態列說原因。套 ack.changed 時不補發 change／input。存檔時還有事件在等回覆 → 這次不存。
 *   ⛔ AI(W906-INBOX108) 20260930：開頁時補 8 條滑桿標籤（proxies.caption）那一段拿掉了 —— 引擎 gbApply 現在自己套 TLabel 的 caption（Jimmy INBOX 108，
 *   ht9045_wire_engine.js gbApply；golden FormShow 設 Position 觸發 tb*Change 改的標籤照樣會出現）。form.event 回應的 caption 仍由下面 applyChanged 套。
 *
 * AI(W906-EVB10B) 20260929 [W906] 事件批次 B10 part b CC-L1（Q46：Steven 20260929「依權限檔鎖住是要執行的」＝照 BCB 翻切頁程式）＋X-5：
 *   操作員「點」外層 PageControl1（Soft／Comm／Config／Tray／Hot Plate）或 A～M 大分頁 pcConfig 的頁籤 ⇒ 送
 *   {"form":"TfConfiguration","control":"pcConfig","event":"change","activePageIndex":<data-t>}（golden :6101 PageControl1Change／
 *   :6116 pcConfigChange：依 D:\HT9045\config\Security_new.def 把切到的那一頁整頁鎖住；ASE 高雄切到 Config 頁要密碼 → 網頁沒有密碼框，
 *   伺服器當「密碼錯」切回 Soft 頁）。ack.changed 帶鎖住的頁與底下每一格的 editable:false（照套），分頁被改回時帶 activePageIndex
 *   （照點回去、不再送事件）。程式切頁（引擎／本檔自己點頁籤）不送：VCL 程式設 ActivePage 不觸發 OnChange。沒有 state（處理器只讀分頁、
 *   AccessLevel、權限檔）。點到目前那一頁不送（VCL 不 OnChange）。伺服器拒絕（running、分頁控制點不到…）⇒ 點回原來那一頁、狀態列說原因。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb4Config) return;
  R.__evb4Config = true;

  var STRUCT = 'IniConfig';                 // editlist.get／editlist.save 的 tag（FileRW/IniConfig.cpp）
  var FORM = 'TfConfiguration';             // golden 表單類別（form.event 的 form；不同 ⇒ bad-payload）
  var LOG = '[Config/EVB4] ';

  // 送出點：[元件, 事件, 種類]（＝kIC_Events 的 B4 列；伺服器沒列的不送）
  var CTLS = [
    ['btD47', 'click', 'btn'], ['btnRecordJamRateByTimeClear', 'click', 'btn'], ['btResume', 'click', 'btn'],   // CC-E1／E3／E4
    ['cbA09', 'click', 'chk'], ['chkA09_1', 'click', 'chk'], ['cbA14', 'click', 'chk'],                          // CC-E6
    ['tbD25_Index60mm', 'change', 'trk'], ['tbD25_Index30mm', 'change', 'trk'], ['tbD25_Index40mm', 'change', 'trk'],   // CC-E8
    ['tbD25_Index60mm_NS', 'change', 'trk'], ['tbD25_Index40mm_NS', 'change', 'trk'], ['tbD25_Index30mm_NS', 'change', 'trk'],
    ['tbD60_Index56mm', 'change', 'trk'], ['tbD60_Index56mm_NS', 'change', 'trk'],
    ['PageControl1', 'change', 'tab'], ['pcConfig', 'change', 'tab']                                              // AI(W906-EVB10B)：CC-L1 切頁
  ];
  var KIND = {};
  CTLS.forEach(function (c) { KIND[c[0]] = c[2]; });
  var CAPTION = { btD47: 'D47 Clear', btnRecordJamRateByTimeClear: 'O11 Jam 率計數清除', btResume: 'Resume' };

  var LAST = null;                          // 最近一次 editlist.get IniConfig 的回應
  var GEN = 0;                              // 每次開頁 +1：重開頁之前送出去的事件，回覆不再套
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
  function ancestorDisabled(el) {
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function inputOf(el) { return !el ? null : (el.tagName === 'INPUT' || el.tagName === 'BUTTON') ? el : el.querySelector('input'); }
  function usable(el) {                                  // 引擎／權限停用的元件（自己或上層）一樣按不到
    if (!el || el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) return false;
    var x = inputOf(el);
    return !x || !x.disabled;
  }
  function eventTag() { return LAST && typeof LAST.eventTag === 'string' && LAST.eventTag ? LAST.eventTag : null; }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function gateOn() {                                    // 伺服器有 eventTag、而且事件表有 B4 的列（舊伺服器 → 一個都不送，照舊）
    if (!eventTag()) return false;
    for (var i = 0; i < CTLS.length; i++) if (evInfo(CTLS[i][0])) return true;
    return false;
  }
  // events.<id>.operable 是開頁當下算的；按鈕可不可見會隨 St02 那支送的勾選事件改（例 btD47 看 cbD47，Timer1 一拍）⇒ 不拿它擋，
  // 只看畫面上點不點得到，伺服器 RunPageEvent 第 2 步再照 golden 查一次（點不到回 bad-payload，頁面改回）
  function canSend(id, evn) { var e = evInfo(id); return !!e && e.event === evn; }

  /* ---- 值 --------------------------------------------------------------- */
  function tabsOf(el) { return el ? el.querySelectorAll(':scope > .pcTabs > .tab') : []; }                       // AI(W906-EVB10B)：TPageControl 的頁籤
  function actTab(el) {                                  // 目前頁籤的 data-t（＝golden PageIndex）
    var a = el && el.querySelector(':scope > .pcTabs > .tab.act'), n = a ? parseInt(a.getAttribute('data-t'), 10) : NaN;
    return isNaN(n) ? null : n;
  }
  function clickTab(el, n) {                             // 程式切頁（APPLYING：本檔的頁籤監聽不送事件；頁面自己的切換照跑）
    var t = el && el.querySelector(':scope > .pcTabs > .tab[data-t="' + n + '"]');
    if (!t || t.classList.contains('act')) return;
    var was = APPLYING;
    APPLYING = true;
    try { t.click(); } finally { APPLYING = was; }
  }
  function paneOf(id) {                                  // TTabSheet（Configuration 的 .pcPane 沒有 id，只有 title／data-htitle）
    return document.querySelector('.pcPane[title="' + id + '"]') || document.querySelector('.pcPane[data-htitle="' + id + '"]');
  }
  function readCtl(id) {                                 // 送出點控制項目前的值（payload 與「改回」用）
    var el = $(id), k = KIND[id];
    if (!el) return null;
    if (k === 'btn') return {};
    if (k === 'tab') { var n0 = actTab(el); return n0 === null ? null : { activePageIndex: n0 }; }                  // AI(W906-EVB10B)
    if (k === 'chk') { var c = inputOf(el); return c ? { checked: !!c.checked } : null; }
    if (k === 'trk') { var n = parseInt(el.value, 10); return isNaN(n) ? null : { position: n }; }
    return null;
  }
  function writeCtl(id, v) {                             // 改回點之前的值（不發事件）
    var el = $(id);
    if (!el || !v) return;
    APPLYING = true;
    try {
      if (v.checked !== undefined) { var c = inputOf(el); if (c) c.checked = !!v.checked; }
      if (v.position !== undefined) el.value = v.position;
      if (v.activePageIndex !== undefined) clickTab(el, v.activePageIndex);                                        // AI(W906-EVB10B)
    } finally { APPLYING = false; }
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
  function applyChanged(ch) {                            // {元件:{checked,text,position,caption,visible,enabled,editable}}，只有變的鍵
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id] || {}, el = $(id) || paneOf(id);                                                             // AI(W906-EVB10B)：TTabSheet 沒有 id
        if (!el) return;
        if (v.activePageIndex !== undefined) clickTab(el, v.activePageIndex);                                        // AI(W906-EVB10B)：golden 處理器改了分頁（ASE 切回 Soft）
        if (v.checked !== undefined) { var c = inputOf(el); if (c && c.type !== 'button') c.checked = !!v.checked; }
        if (v.text !== undefined && 'value' in el && el.tagName !== 'BUTTON' && !(el.tagName === 'INPUT' && (el.type === 'checkbox' || el.type === 'radio'))) el.value = String(v.text);
        if (v.position !== undefined) el.value = v.position;                                  // 同引擎 gbApply
        if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;   // TLabel（引擎 gbApply 不套 caption）
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
      });
      dis.forEach(function (el) { setEditable(el, false); });                                 // 先開後關（同引擎 gbLoad）
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
  function label(id) { return CAPTION[id] || id; }
  function fail(item, msg) {                             // 伺服器沒跑 golden 處理器 → 改回點之前的值；同一個控制項後面排著的一起丟
    QUEUE = QUEUE.filter(function (q) { return q.control !== item.control; });
    if (item.gen === GEN && KIND[item.control] !== 'btn') { writeCtl(item.control, item.prev); KNOWN[item.control] = item.prev; }
    say2('Configuration ' + label(item.control) + ' 沒有送到伺服器（form.event）：' + msg +
         (KIND[item.control] === 'btn' ? ' —— 伺服器沒有執行' : ' —— 已改回點之前的值'), '#f88');
  }
  function reverted(item, a) {                           // CC-E6：伺服器照 golden 把 cbA09 改回（機台內還有 IC）
    var ch = (a && a.changed) || {}, c = ch.cbA09;
    if (!c || c.checked === undefined) return;
    if (item.control === 'cbA09' && item.val.checked !== undefined && c.checked === item.val.checked) return;
    say2('Configuration [A09]：機台內還有 IC，伺服器照 golden cbA09Click（cConfiguration.cpp:6453）把 A09 改回 ' +
         (c.checked ? '勾選' : '不勾') + '（常溫模式要先 One Cycle，加熱模式要先 Clean out；告警框 MES1645／MES1646）', '#f88');
  }
  function send(item, tries) {
    var tag = eventTag();
    if (!tag) { fail(item, '伺服器沒有給 eventTag'); return Promise.resolve(null); }
    var v = { form: FORM, control: item.control, event: item.event }, p = item.val;
    Object.keys(p).forEach(function (k) { v[k] = p[k]; });
    var extra = { tag: tag, value: JSON.stringify(v) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) {
      if (item.gen !== GEN) return null;                 // 期間重開過頁：editlist.get 已經給了新狀態
      var a = unwrap(m) || {};
      applyChanged(a.changed);
      if (KIND[item.control] === 'chk') reverted(item, a);
      if (a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      else if (KIND[item.control] === 'btn') say2('Configuration ' + label(item.control) + '：伺服器已照 golden ' + ((evInfo(item.control) || {}).golden || '') + ' 執行');
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
        say2('Configuration：伺服器要求重新開頁（' + msg + '）—— 重讀中');
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
    if (last && last.event === item.event && (KIND[item.control] === 'btn' || JSON.stringify(last.val) === JSON.stringify(item.val))) return false;   // 連點
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- 接線：B4 送出點 ---------------------------------------------------- */
  function onUser(id, evn) {
    return function (ev) {
      if (APPLYING) return;
      if (ev && ev.isTrusted === false) return;          // 只收使用者真的點的（別的程式 dispatch 的不算）
      var el = $(id), k = KIND[id];
      if (!el || !gateOn()) return;                      // 伺服器沒有事件表：照舊（存檔時伺服器補重播，FileRW/IniConfig.cpp (4)(6)）
      var cur = readCtl(id), prev = KNOWN[id];
      var can = k === 'tab' ? !(el.getAttribute('aria-disabled') === 'true' || ancestorDisabled(el)) : usable(el);   // AI(W906-EVB10B)：分頁控制只看自己／上層有沒有停用（usable 會看到頁裡的第一個 input）
      if (k === 'tab' && (OFF || !canSend(id, evn))) return;   // AI(W906-EVB10B)：伺服器沒有切頁事件（舊伺服器）→ 頁籤照舊只切畫面，不改回
      if (OFF || !canSend(id, evn) || !can) {
        if (k !== 'btn') writeCtl(id, prev);
        var e = evInfo(id);
        say2('Configuration ' + label(id) + '：' + (OFF ? '伺服器不支援 form.event（' + OFF + '）'
              : !e ? '伺服器的事件表沒有這個元件' : e.event !== evn ? '伺服器的事件是 ' + e.event + '，不是 ' + evn
              : '現在點不到') + (k !== 'btn' ? ' —— 已改回' : ''), '#f88');
        return;
      }
      if (k !== 'btn' && prev && cur && JSON.stringify(prev) === JSON.stringify(cur)) return;   // 值沒變（golden 不會 OnClick／OnChange）
      if (enqueue({ control: id, event: evn, val: cur || {}, prev: prev, gen: GEN }) && k !== 'btn') KNOWN[id] = cur;
    };
  }

  /* ---- CC-L2：要廠商密碼的格子（Q45 #1～#3、#5＝C，不送事件） ----------- */
  var C12G = ['cbC12', 'cbC13', 'chkC14', 'cbC17', 'cbC24'];                  // golden :6775 cbC12Click（dfm:3178／:3161／:3238／:3533／:3903）
  var VEND = ['cbA27', 'cbN07_EnableEmployeeCheak'];                          // golden :6824、:6860
  var VEND_NOTE = '這一項要廠商密碼，網頁版不提供，請到 BCB 版機台改';
  function openChecked(id) {                             // 開頁值（editlist.get 的 proxies；IC_PasswordGuard 比的也是它）
    var p = LAST && LAST.proxies && LAST.proxies[id];
    if (p && typeof p.checked === 'boolean') return p.checked;
    return KNOWN['__open_' + id];
  }
  function checkedNow(id) { var c = inputOf($(id)); return !!(c && c.checked); }
  function markVendor(id, text, why) {                   // 開頁標示：小字＋滑鼠提示（先把 dfm 說明搬到 data-htitle，同 theme.js，引擎判斷型別照讀得到）
    var el = $(id);
    if (!el) return;
    if (!el.hasAttribute('data-htitle') && el.getAttribute('title')) el.setAttribute('data-htitle', el.getAttribute('title'));
    el.title = why;
    el.setAttribute('data-st01-vendor', text);
    el.classList.add('st01VendorPw');
  }
  function c12Refused() {                                // 同 FileRW/IniConfig.cpp IC_PasswordGuard 的 CC-L2 那一段
    if (!checkedNow('cbC12')) return false;
    if (!openChecked('cbC12')) return true;
    for (var i = 1; i < C12G.length; i++) if (checkedNow(C12G[i]) !== !!openChecked(C12G[i])) return true;
    return false;
  }
  function onVendor(id) {
    return function (ev) {
      if (APPLYING || (ev && ev.isTrusted === false)) return;
      var refused = C12G.indexOf(id) >= 0 ? c12Refused() : checkedNow(id) !== !!openChecked(id);
      if (!refused) return;
      var c = inputOf($(id));
      APPLYING = true;
      try { if (c) c.checked = !c.checked; } finally { APPLYING = false; }   // 點之前的值（一次點一格，change 之前就是相反值）
      say2('Configuration ' + id + '：' + VEND_NOTE + (C12G.indexOf(id) >= 0
            ? '（golden cConfiguration.cpp:6775 cbC12Click：[C12] PE 模式勾著的時候，勾 C12 或改 C13／C14／C17／C24 都要廠商密碼；先取消 C12 再改其他格不用）'
            : id === 'cbA27' ? '（golden :6824 cbA27Click：勾、取消都要）' : '（golden :6860 cbN07_EnableEmployeeCheakClick：勾、取消都要）') +
           ' —— 已改回（Q45 #1～#3＝C：維持改不了；存檔時伺服器也照這條規則拒存）', '#f88');
    };
  }
  function hookVendor() {
    if (!document.getElementById('st01VendorPwCss')) {
      var st = document.createElement('style');
      st.id = 'st01VendorPwCss';
      st.textContent = '.st01VendorPw::after{content:attr(data-st01-vendor);margin-left:6px;font-size:10px;color:#c60;}';
      (document.head || document.documentElement).appendChild(st);
    }
    C12G.forEach(function (id, i) {
      markVendor(id, i === 0 ? '（要廠商密碼）' : '（C12 勾著時要廠商密碼）',
                 VEND_NOTE + '（golden cConfiguration.cpp:6775 cbC12Click' + (i === 0 ? '' : '：綁在這一格、只看 C12') + '；Q45 #1＝C）');
    });
    markVendor('cbA27', '（要廠商密碼）', VEND_NOTE + '（golden :6824 cbA27Click；Q45 #2＝C）');
    markVendor('cbN07_EnableEmployeeCheak', '（要廠商密碼）', VEND_NOTE + '（golden :6860；Q45 #3＝C）');
    var img = $('Image1');
    if (img) {
      if (!img.hasAttribute('data-htitle') && img.getAttribute('title')) img.setAttribute('data-htitle', img.getAttribute('title'));
      img.title = 'Check List Enable 區塊（[A32_1]）要 SG_PW.ini 的密碼，網頁版不提供（golden :7274 Image1DblClick；Q45 #5＝C）';
      if (!img.__evb4) {
        img.__evb4 = true;
        img.addEventListener('dblclick', function (ev) {
          if (ev && ev.isTrusted === false) return;
          say2('Configuration [A32_1] Check List Enable：要 SG_PW.ini 的密碼，網頁版不提供，請到 BCB 版機台改（golden :7274 Image1DblClick；Q45 #5＝C）', '#f88');
        });
      }
    }
    C12G.concat(VEND).forEach(function (id) {
      var el = $(id);
      if (!el || el.__evb4v) return;
      el.__evb4v = true;
      el.addEventListener('change', onVendor(id));
    });
  }

  function hook() {
    CTLS.forEach(function (c) {
      var el = $(c[0]);
      if (!el || el.__evb4) return;
      el.__evb4 = true;
      if (c[2] === 'tab') {                              // AI(W906-EVB10B)：頁籤的 click（頁面自己的切換先跑，這裡讀到的是新頁）
        var h = onUser(c[0], c[1]);
        Array.prototype.forEach.call(tabsOf(el), function (tb) { tb.addEventListener('click', h); });
        return;
      }
      el.addEventListener(c[2] === 'btn' ? 'click' : 'change', onUser(c[0], c[1]));
    });
    hookVendor();
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++; QUEUE = []; OFF = '';
      setTimeout(function () {                           // 引擎在這個 promise 的 then 裡同步套完值
        try {
          hook(); refreshKnown();   // AI(W906-INBOX108) 20260930: applyOpenCaptions 拿掉（引擎 gbApply 套 caption）
          C12G.concat(VEND).forEach(function (id) { KNOWN['__open_' + id] = checkedNow(id); });   // 沒有 proxies 時的開頁值備用
        } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (!idle()) return Promise.reject(new Error('Configuration 還有 ' + (QUEUE.length + (INFLIGHT ? 1 : 0)) +
                                                 ' 個點擊事件在等伺服器回覆（form.event）—— 這次沒有存檔；畫面更新完再按一次存檔'));
    return save0.apply(this, arguments);
  };

  window.HT9045EvB4Config = {                             // 探針／除錯用
    gate: gateOn, queue: function () { return QUEUE.slice(); }, inflight: function () { return INFLIGHT; },
    known: function () { return KNOWN; }, applyChanged: applyChanged, c12Refused: c12Refused, controls: function () { return CTLS.slice(); }
  };
})();
