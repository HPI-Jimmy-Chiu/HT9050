/* ht9045_config_q41.js -- Config.Configuration.html（golden TfConfiguration，cConfiguration.cpp）Q41 的頁面事件補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.19 標 Steven02 的頁面事件。St02 的新檔（手寫，
 * 不是 gen_wire.py 產物）；頁面 Config.Configuration.html:128 同一行載入，在接線資料檔與 St01 的 P26 補件之後。引擎 ht9045_wire_engine.js 不改。
 * golden 行號：912 cConfiguration.cpp（906_0625_Steven 在括號裡；本體相同）。
 *
 *  CC-E2／CC-E7 兩條路（看 editlist.get IniConfig 回應有沒有 eventTag）：
 *   (A) 有 eventTag（St01 64ade3b7 起，C 路 form.event，tag＝eventTag＝"Config.Configuration"；送 "IniConfig" 會回 unknown-page）：
 *       事件送伺服器、照 ack.changed 局部套值（伺服器跑 golden 處理器＋Timer1 一拍，是準）。
 *       - udD46（TUpDown，DFM 912 :4954-4964 Min 5 Max 15 Wrap=False；頁面沒有這個元件）：在 DFM 位置（Left 322、Top 296、19x24，
 *         跟 edD46 同一個容器）補兩顆鈕，上＝{"event":"btNext"}、下＝{"event":"btPrev"}（golden udD46Click :6094（:5991））。
 *         按鈕 id 用 q41udD46（不用 udD46：引擎會把替身值套到同 id 的元素上）。
 *       - 勾選框：cbE30Click :6125（:6022）8 顆、cbE39Click :6369（:6266）、cbD36Click :6551（:6448）5 顆、cbD21／cbD47／cbF05（golden 靠
 *         Timer1Timer :6025（:5922）每秒一拍，:6032-6044（:5929-5941））→ {"event":"click","checked":<點完的值>}。
 *       - 一次一個、等 ack 再送下一個；同一個 value（例 udD46 連按）距上一次回覆至少 500 ms 才送（R117：WebCmdGuard 同 key 完成後 400 ms 內回 busy:，還是遇到就照同一規則重送）；events.<id>.operable＝false 的不送；
 *         錯誤含 "reload page" → 重新開頁（editlist.get）；not-operator → 續權杖後重送一次。
 *       - edD46：照 proxies.edD46.editable（golden 開頁 InitialDataToEdit 會打開它，不是照 DFM 的 Enabled=False）。
 *   (B) 沒有 eventTag（64ade3b7 還沒進來）：**一個事件都不送**，照舊在頁面上照 golden 算顯示（下面 CC-E7 本地版），udD46 鈕不建。
 *  CC-E9  btnAutoSaveSetAllClick :6437（:6334）→ strngrdAutoSaveLog 第 1 列 7 格＝"On"（格子點一下切換在引擎 gbGrid，Jimmy 的檔）。
 *  CC-E14 edtSearchFunctionChange :7219（:7116）→ 打字搜尋：0 字全部放回原位；2 字以上把 elConfig 裡 Caption 含這串的 TCheckBox 搬到
 *         scrlbxSearch（alTop），其他放回；1 個字什麼都不做（golden 兩個分支都不進）。元件搬家後 id 不變，引擎照 id 收值不受影響。
 * 存檔照舊 editlist.save tag=IniConfig；伺服器存檔前也會重播沒送過事件的勾選（St01 IC_EvBeforeApply）。
 * ack.ignored 出現 edD46 時 console.warn 整個 ack（給 ST01-E 看）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'IniConfig';
  var FORM = 'TfConfiguration';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || R.__configQ41Wrapped) return;
  R.__configQ41Wrapped = true;

  var LAST = null;                                       // 最近一次 editlist.get IniConfig 的回應
  function $(id) { return document.getElementById(id); }
  function cbOf(id) { var el = $(id); return el ? (el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]')) : null; }
  function chk(id, v) {                                  // TCheckBox->Checked
    var c = cbOf(id);
    if (!c) return false;
    if (v !== undefined) c.checked = !!v;
    return c.checked;
  }
  function fire(el, names) {                             // 同引擎小鍵盤 onCommit：補發 input／change
    (names || ['input', 'change']).forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function setVis(id, on) {                              // VCL Visible（引擎用 visibility；產生器的 DFM Visible=False 是 display:none）
    var el = typeof id === 'string' ? $(id) : id;
    if (!el) return;
    el.style.visibility = on ? '' : 'hidden';
    if (on && el.style.display === 'none') el.style.display = '';
  }
  function ancestorDisabled(el) {                        // 上層容器被引擎停用（權限）→ VCL 一樣按不到
    for (var p = el.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return true;
    return false;
  }
  function setEditable(el, on) {                         // 同引擎 gbSetEnabled：只動自己關的；打開時看上層
    if (!el) return;
    if (on && ancestorDisabled(el)) return;
    var list = [el].concat(Array.prototype.slice.call(el.querySelectorAll('input,select,button,textarea')));
    list.forEach(function (x) {
      if (!('disabled' in x)) return;
      if (!on) { if (!x.disabled) { x.disabled = true; x.setAttribute('data-gb-dis', '1'); } }
      else if (x.getAttribute('data-gb-dis') === '1') { x.disabled = false; x.removeAttribute('data-gb-dis'); }
    });
    if (!on) el.setAttribute('aria-disabled', 'true'); else el.removeAttribute('aria-disabled');
  }
  function usable(el) { return el && !el.disabled && el.getAttribute('aria-disabled') !== 'true' && !ancestorDisabled(el); }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66', 'transient');
    console.info('[Config/Q41] ' + msg);
  }

  var CK = ['cbE30', 'cbE31', 'cbE31_1', 'cbE31_2', 'cbE32', 'cbE32_1', 'cbE32_2', 'cbE33',   // DFM 912 :5880 等（cbE30Click）
            'cbE39',                                                                              // :9189（cbE39Click）
            'cbD36', 'cbD37', 'cbD38', 'cbD36_1', 'cbD36_2',                                      // :4824 起（cbD36Click）
            'cbD21', 'cbD47', 'cbF05'];                                                           // Timer1Timer 的顯示段
  function eventTag() { return LAST && typeof LAST.eventTag === 'string' && LAST.eventTag ? LAST.eventTag : null; }
  function operable(id) {
    var ev = LAST && LAST.events && LAST.events[id];
    return !!ev && ev.operable !== false;
  }

  /* ---- (B) CC-E7 本地版（沒有 eventTag 時） ------------------------------ */
  function cbE30Click() {                                // golden :6125-6130
    setVis('palE30', chk('cbE30'));
    setVis('palE31', chk('cbE31'));
    setVis('palE32', chk('cbE32'));
  }
  function e39_1Value() {                                // golden IniConfig.bE39_1PutTheDevicesToErrorBin（沒存檔前＝開頁值）
    var px = LAST && LAST.proxies && LAST.proxies.cbE39_1;
    if (px && px.checked !== undefined) return !!px.checked;
    var ents = (LAST && LAST.lists && LAST.lists.elConfig && LAST.lists.elConfig.entries) || [];
    for (var i = 0; i < ents.length; i++) if (ents[i].id === 'cbE39_1' && ents[i].checked !== undefined) return !!ents[i].checked;
    return false;
  }
  function cbE39Click() {                                // golden :6369-6381
    if (chk('cbE39')) { setVis('cbE39_1', true); chk('cbE39_1', e39_1Value()); }
    else { setVis('cbE39_1', false); chk('cbE39_1', false); }
  }
  function cbD36Click() {                                // golden :6551-6558（cbD33／cbD35 沒有 OnClick，不會連鎖）
    if (chk('cbD36') === true) { chk('cbD33', false); chk('cbD35', true); }
  }
  function timer1Display() {                             // golden :6032-6044（計時器的顯示段）
    var d21 = chk('cbD21'), d47 = chk('cbD47'), f05 = chk('cbF05');
    ['edD21_mm', 'edD21_Sec', 'labD21_1', 'labD21_2'].forEach(function (id) { setVis(id, d21); });
    ['labD47_1', 'labD47_2', 'labD47_3', 'edD47_3', 'btD47', 'edD47_Count', 'edD47_Time'].forEach(function (id) { setVis(id, d47); });
    ['edF05', 'labF05'].forEach(function (id) { setVis(id, f05); });
  }
  var LOCAL = { cbE30: cbE30Click, cbE31: cbE30Click, cbE31_1: cbE30Click, cbE31_2: cbE30Click, cbE32: cbE30Click, cbE32_1: cbE30Click,
                cbE32_2: cbE30Click, cbE33: cbE30Click, cbE39: cbE39Click, cbD36: cbD36Click, cbD37: cbD36Click, cbD38: cbD36Click,
                cbD36_1: cbD36Click, cbD36_2: cbD36Click, cbD21: timer1Display, cbD47: timer1Display, cbF05: timer1Display };

  /* ---- (A) form.event（有 eventTag 時） ---------------------------------- */
  var QUEUE = [], sending = false, APPLYING = false;   // APPLYING：套 ack.changed 時補發的 change 不再送事件
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) { /* 照原樣 */ } }
    return m;
  }
  function applyChanged(ch) {                            // ack.changed：{元件:{text,itemIndex,checked,position,visible,enabled,editable,…}}，只有變的鍵
    var dis = [];
    APPLYING = true;
    try {
      Object.keys(ch || {}).forEach(function (id) {
        var v = ch[id], el = $(id);
        if (!el) return;                                   // 例 udD46：頁面沒有這個元件（值看 edD46）
        if (v.checked !== undefined) { var c = cbOf(id) || (el.querySelector && el.querySelector('input[type="radio"]')); if (c && c.checked !== !!v.checked) { c.checked = !!v.checked; fire(c, ['change']); } }
        if (v.text !== undefined && 'value' in el && el.type !== 'checkbox' && el.type !== 'radio' && el.value !== String(v.text)) { el.value = String(v.text); fire(el); }
        if (v.visible !== undefined) setVis(el, !!v.visible);
        var ena = v.editable !== undefined ? v.editable : v.enabled;
        if (ena !== undefined) { if (ena) setEditable(el, true); else dis.push(el); }
      });
      dis.forEach(function (el) { setEditable(el, false); });   // 先開後關（同引擎 gbLoad）：停用的容器蓋過子元件
    } finally { APPLYING = false; }
  }
  var LASTV = null, LASTAT = 0, SAME_GAP_MS = 500;  function send(item, tries) {   // AI(W906-R117) 20260929 (St02-E): LASTV / LASTAT = the previous form.event value and when its reply came
    var tag = eventTag();
    if (!tag) return Promise.resolve(null);              // 安全閘：沒有 eventTag 一個都不送
    var v = { form: FORM, control: item.control, event: item.event };
    if (item.checked !== undefined) v.checked = item.checked;  var vs = JSON.stringify(v), wait = vs === LASTV ? SAME_GAP_MS - (Date.now() - LASTAT) : 0;  if (wait > 0) return new Promise(function (res) { setTimeout(res, wait); }).then(function () { return send(item, tries); });   // AI(W906-R117) 20260929: WebCmdGuard keys cmd+tag+value, busy within 400 ms of the same key finishing -- wait out 500 ms first
    return R.rawCmd('form.event', { tag: tag, value: vs }).then(function (m) {  LASTV = vs; LASTAT = Date.now();
      var a = unwrap(m);
      applyChanged(a && a.changed);
      if (a && a.messages && a.messages.length) say2(a.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
      return a;
    }, function (e) {  LASTV = vs; LASTAT = Date.now();
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {             // 同一指令 400 ms 內重送被擋 → 照上面的 500 ms 規則再送（AI(W906-R117) 20260929：原本固定 450 ms）
        return send(item, tries + 1);
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) {
        return R.keepAlive().then(function () { return send(item, tries + 1); });
      }
      if (/reload page/.test(msg)) {
        say2('Configuration：伺服器要求重新開頁（' + msg + '）—— 重讀中');
        QUEUE = [];
        if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
        return null;
      }
      say2('Configuration form.event ' + item.control + ' ' + item.event + ' 沒有跑：' + msg + '（存檔時伺服器會照 golden 補重播）', '#f88');
      return null;
    });
  }
  function pump() {
    if (sending || !QUEUE.length) return;  var b4 = window.HT9045EvB4Config; if (b4 && ((typeof b4.inflight === 'function' && b4.inflight()) || (typeof b4.queue === 'function' && b4.queue().length))) { setTimeout(pump, 60); return; }   // AI(W906-Q41-CFG) 20260928 (St02-E): TEMPORARY until St01's shared Configuration queue -- let St01 ht9045_config_st01_ev.js (review6 875d3499, probe :357-360) acks land before ours goes out (same as Temp_Set TS-7)
    sending = true;
    var item = QUEUE.shift();
    send(item, 0).then(function () { sending = false; pump(); }, function () { sending = false; pump(); });
  }
  function enqueue(item) {
    if (!eventTag() || !operable(item.control)) return false;
    QUEUE.push(item);
    pump();
    return true;
  }

  /* ---- CC-E2 udD46 上下鍵（只有 eventTag 時才建） ------------------------- */
  function udD46() {
    var ed = $('edD46'), host = ed && ed.parentElement, cli = host && host.parentElement, box = $('q41udD46');
    if (!cli) return;
    if (!eventTag()) { if (box) box.style.display = 'none'; return; }
    if (!box) {
      box = document.createElement('span');
      box.id = 'q41udD46';
      box.setAttribute('data-htitle', 'udD46 : TUpDown（Q41 頁面補件）');
      box.style.cssText = 'position:absolute;left:322px;top:296px;width:19px;height:24px;display:flex;flex-direction:column;';
      [['▲', 'btNext'], ['▼', 'btPrev']].forEach(function (q) {
        var b = document.createElement('button');
        b.type = 'button';
        b.textContent = q[0];
        b.style.cssText = 'flex:1 1 0;min-height:0;padding:0;font-size:7px;line-height:1;cursor:pointer;';
        b.addEventListener('click', function (ev) {
          ev.preventDefault();
          if (b.disabled || box.getAttribute('aria-disabled') === 'true') return;
          enqueue({ control: 'udD46', event: q[1] });
        });
        box.appendChild(b);
      });
      cli.appendChild(box);
    }
    var px = LAST && LAST.proxies && LAST.proxies.udD46;
    var on = operable('udD46') && !(px && (px.editable === false || px.enabled === false)) && !ancestorDisabled(box);
    box.style.display = (px && px.visible === false) ? 'none' : '';
    Array.prototype.forEach.call(box.querySelectorAll('button'), function (b) { b.disabled = !on; });
    if (on) box.removeAttribute('aria-disabled'); else box.setAttribute('aria-disabled', 'true');
    // edD46：照 proxies.edD46.editable（golden 開頁 InitialDataToEdit 打開它；dfm2web 照 DFM Enabled=False 把外框畫成 opacity:.45）
    var pe = LAST && LAST.proxies && LAST.proxies.edD46;
    if (pe && pe.editable !== undefined && host) {
      if (pe.editable && !ancestorDisabled(ed)) { setEditable(ed, true); host.style.opacity = ''; }
      else host.style.opacity = '.45';
    }
  }

  /* ---- CC-E9 ------------------------------------------------------------ */
  function btnAutoSaveSetAllClick() {                    // golden :6437-6443
    var g = $('strngrdAutoSaveLog');
    if (!g || g.getAttribute('aria-disabled') === 'true') return;
    for (var i = 0; i < 7; i++) {
      var td = g.querySelector('td[data-c="' + i + '"][data-r="1"]');
      if (td) td.textContent = 'On';
    }
  }

  /* ---- CC-E14 ----------------------------------------------------------- */
  var HOME = {};                                         // id -> {parent, next, css}（golden THTEdit::SetToDefaultPosition 的原位）
  function toDefault(el) {
    var h = HOME[el.id];
    if (!h) return;
    if (h.next && h.next.parentNode === h.parent) h.parent.insertBefore(el, h.next); else h.parent.appendChild(el);
    el.style.cssText = h.css;
    delete HOME[el.id];
  }
  function toSearch(el, box) {                           // golden Parent=scrlbxSearch、Align=alTop
    if (!HOME[el.id]) HOME[el.id] = { parent: el.parentNode, next: el.nextSibling, css: el.style.cssText };
    box.appendChild(el);
    el.style.position = 'relative';
    el.style.left = '0px';
    el.style.top = '0px';
    el.style.width = '100%';
  }
  function configCheckBoxes() {                          // elConfig 裡 SourceControl 是 TCheckBox 的筆
    var ents = (LAST && LAST.lists && LAST.lists.elConfig && LAST.lists.elConfig.entries) || [], out = [];
    ents.forEach(function (e) {
      var el = e.id ? $(e.id) : null;
      if (el && el.tagName === 'LABEL' && el.querySelector(':scope > input[type="checkbox"]')) out.push(el);
    });
    return out;
  }
  function caption(el) { return (el.textContent || '').replace(/\s+$/, ''); }
  function edtSearchFunctionChange() {                   // golden :7219-7252
    var ed = $('edtSearchFunction'), box = $('scrlbxSearch');
    if (!ed || !box) return;
    var t = String(ed.value || '');
    if (t.length === 0) {
      configCheckBoxes().forEach(toDefault);
    } else if (t.length >= 2) {
      var up = t.toUpperCase();
      configCheckBoxes().forEach(function (el) {
        if (caption(el).toUpperCase().indexOf(up) >= 0) toSearch(el, box); else toDefault(el);
      });
    }
  }

  /* ---- 接線 --------------------------------------------------------------- */
  function onCk(id) {
    return function () {
      if (APPLYING) return;                                                                         // 伺服器套回來的值，不是使用者點的
      if (eventTag()) { enqueue({ control: id, event: 'click', checked: chk(id) }); return; }   // (A)：點完的值送伺服器
      LOCAL[id]();                                                                                  // (B)：本地照 golden 算
    };
  }
  function hook() {
    CK.forEach(function (id) {
      var el = $(id);
      if (el && !el['__q41' + id]) { el['__q41' + id] = true; el.addEventListener('change', onCk(id)); }
    });
    var b = $('btnAutoSaveSetAll');
    if (b && !b.__q41) { b.__q41 = true; b.addEventListener('click', function () { if (usable(b)) btnAutoSaveSetAllClick(); }); }
    var ed = $('edtSearchFunction');
    if (ed && !ed.__q41) { ed.__q41 = true; ed.addEventListener('input', edtSearchFunctionChange); ed.addEventListener('change', edtSearchFunctionChange); }
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d;
      QUEUE = [];
      setTimeout(function () {                           // 引擎在這個 promise 的 then 裡同步套完值
        hook();
        if (!eventTag()) timer1Display();                // (A)：伺服器開頁已補 Timer1 第一拍，proxies 就是結果
        udD46();
        var ed = $('edtSearchFunction');
        if (ed && ed.value) edtSearchFunctionChange();   // 重讀後搜尋框還有字 → 照 golden 再排一次
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    return save0.apply(this, arguments).then(function (a) {
      if (a && a.ignored && a.ignored.indexOf('edD46') >= 0) console.warn('[Config/Q41] editlist.save ignored edD46 (for ST01-E):', JSON.stringify(a));
      return a;
    });
  };

  window.HT9045ConfigQ41 = {                              // 探針／除錯用
    cbE30Click: cbE30Click, cbE39Click: cbE39Click, cbD36Click: cbD36Click, timer1Display: timer1Display,
    btnAutoSaveSetAllClick: btnAutoSaveSetAllClick, edtSearchFunctionChange: edtSearchFunctionChange,
    eventTag: eventTag, queue: function () { return QUEUE.slice(); }, applyChanged: applyChanged,
    moved: function () { return Object.keys(HOME); }
  };
})();
