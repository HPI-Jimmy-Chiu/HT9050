/* ht9045_offset_ev.js -- Setup.OffSet.html（golden TfOffSet，V912 cOffSet.cpp）OS-1 的 form.event 送出點。
 * ---------------------------------------------------------------------------
 * AI(W906-EVB3) 20260928 [W906] St01 新檔（手寫）。批次 B3 的 OS-1（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md
 *   第三節 B3、第四節 OS-1 列）。Steven 20260928：「任何畫面的事件, 都是我們做」「如果沒有移植的, 我們直接實作」。
 *   C++ 那一半：golden 處理器 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.gen.inc 檔尾（設定 tools\editlist\Offset_File.py），
 *   別名頁 tag "Setup.OffSet"、53 顆部位鈕的事件列、選取檢查在 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp 檔尾。
 *   寫法照 St02 的 ht9045_config_q41.js :117-163（St02 分支 v906/steven-gpib-widget）。同頁的 ht9045_offset_wire.js（選取式編輯器）、
 *   引擎 ht9045_wire_engine.js 不改；本檔只用 ht9045_offset_wire.js 匯出的 window.HT9045Offset（state／work／collect）。
 *   頁面 Setup.OffSet.html:129（St01 e086de38 那一行，ht9045_offset_wire.js 之後）同一行載入。
 *
 *  OS-1  微調 offset 的上／下／右／左（sb_AutoOffsetUp／Down／Right／Left，在 pan_AutoOffsetMove 裡；那個面板只有 Configuration
 *        開了 bUseAutoOffsetFunction 才顯示，golden FormShow :572）：golden :2892-2943 ＝ edArmY／edArmX ±0.1 → 立刻存目前選的部位
 *        （spbSaveClick）。勾了「Auto Offset Position Check」時 golden 還會讓機台跑一次（fMain->Start，OS-1b）—— 那一段伺服器不做，
 *        ack.todo 會說明（B8／Jimmy）。
 *  送法（一次點擊送兩則，都等 ack）：
 *    ① {"form":"TfOffSet","control":"<目前部位鈕 id，例 sbLoader>","event":"click"}  → golden SpBotSelClick：伺服器換到頁面正在看的
 *       那一組（開頁時伺服器把每一組都跑過一次，停在最後一組；不先換就會存錯部位）。回覆的 changed 不套（頁面照自己的整包顯示）。
 *    ② {"form":"TfOffSet","control":"sb_AutoOffsetUp","event":"click","state":{這一組畫面上可改的值＋cb_AutoOffsetPositionCheck}}
 *       → 伺服器先套 state（＝golden 畫面上的值），再跑 golden 處理器（±0.1 後整組存）。回覆的 changed 只套這一組的欄位
 *       （edArmX／edArmY 的新值）與 labWarningForStop。
 *  跟 golden 不同（C 路頁面本來就是這樣，見 ht9045_offset_wire.js 檔頭）：golden 換部位時會先存上一組；這裡微調鈕只存目前這一組，
 *    別組沒按 Save 的修改還留在頁面上，照舊按 Save 才寫。沒有選部位時不送（golden 開窗後還沒選部位 iNowOffsetSel=-1，存不到東西）。
 *  防連點：上一下還沒回覆，這一下不送（每一下都會存檔）；busy: 等 450 ms 重送（最多 3 次）；not-operator 續權杖再送一次；
 *    "reload page" → HT9045Page.load()；events.<id>.operable＝false 不送；存檔時還有微調在等回覆 → 這次不存。
 *  伺服器沒有 eventTag（C++ 還沒進來）＝一個都不送，按了只提示。
 *  另：pan_AutoOffsetMove 在 HTML 是 display:none（DFM Visible=False），引擎套 proxies 的 visible 只改 visibility、打不開 display ⇒
 *    伺服器說看得見（golden FormShow :572）時這裡打開。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__evb3Offset) return;
  R.__evb3Offset = true;

  var STRUCT = 'Offset_File', FORM = 'TfOffSet', LOG = '[Offset/EVB3] ';
  var BTNS = ['sb_AutoOffsetUp', 'sb_AutoOffsetDown', 'sb_AutoOffsetRight', 'sb_AutoOffsetLeft'];   // ＝kOS_Events
  var CHECK = 'cb_AutoOffsetPositionCheck';

  var LAST = null, GEN = 0, INFLIGHT = null;

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
  function hiddenUp(el) {
    for (var p = el; p; p = p.parentElement) if (p.style && (p.style.display === 'none' || p.style.visibility === 'hidden')) return true;
    return false;
  }
  function usable(el) { return !!el && !el.disabled && el.getAttribute('aria-disabled') !== 'true' && !ancestorDisabled(el) && !hiddenUp(el); }
  function eventTag() { return LAST && typeof LAST.eventTag === 'string' && LAST.eventTag ? LAST.eventTag : null; }
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function canSend(id) { var e = evInfo(id); return !!e && e.event === 'click' && e.operable !== false; }

  function send(v, tries) {
    var extra = { tag: eventTag(), value: JSON.stringify(v) };
    var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
    return pre.then(function () { return R.rawCmd('form.event', extra); }).then(function (m) { return unwrap(m) || {}; }, function (e) {
      var msg = (e && e.message) || String(e);
      if (/^busy/.test(msg) && tries < 3) {
        return new Promise(function (res) { setTimeout(res, 450); }).then(function () { return send(v, tries + 1); });
      }
      if (msg === 'not-operator' && tries < 1 && R.keepAlive) return R.keepAlive().then(function () { return send(v, tries + 1); });
      throw new Error(msg);
    });
  }

  // ② 的 state：這一組 ht9045_offset_wire.js 工作副本裡可改的欄位（collect() 先把畫面值收進工作副本）＋勾選框
  function groupState(key) {
    var H = window.HT9045Offset, g = LAST && LAST.offsets && LAST.offsets[key], out = {};
    if (!H || !g) return out;
    H.collect();
    var w = H.work(key) || {};
    Object.keys(g.widgets || {}).forEach(function (id) {
      if (g.widgets[id] && g.widgets[id].editable && w[id]) out[id] = w[id];
    });
    var ck = $(CHECK), c = ck ? (ck.tagName === 'INPUT' ? ck : ck.querySelector('input[type="checkbox"]')) : null;
    if (c) out[CHECK] = { checked: c.checked };
    return out;
  }
  function applyAfter(key, ch) {                         // 只套這一組的欄位（值）與 labWarningForStop（golden spbSaveClick :2881）
    var g = LAST && LAST.offsets && LAST.offsets[key];
    Object.keys(ch || {}).forEach(function (id) {
      var v = ch[id] || {}, el = $(id);
      if (!el) return;
      if (g && g.widgets && g.widgets[id] && v.text !== undefined && 'value' in el) el.value = String(v.text);
      if (id === 'labWarningForStop' && v.visible !== undefined) {
        el.style.visibility = v.visible ? '' : 'hidden';
        if (v.visible && el.style.display === 'none') el.style.display = '';
      }
    });
  }

  function onClick(id) {
    return function (ev) {
      if (ev && ev.isTrusted === false) return;
      var b = $(id);
      if (!eventTag()) { say2('Offset 微調鈕：伺服器還沒有這個事件（editlist.get 沒有 eventTag）—— 沒有送、沒有存', '#f88'); return; }
      if (INFLIGHT) { say2('Offset 微調：上一下還在等伺服器回覆（每一下都會存檔），這一下沒有送'); return; }
      if (!usable(b) || !canSend(id)) { say2('Offset 微調鈕 ' + id + '：伺服器說現在點不到（golden 點不到，events.operable=false）', '#f88'); return; }
      var H = window.HT9045Offset, st = H && H.state ? H.state() : null, key = st && st.cur ? st.cur.stander : null;
      var g = key !== null && key !== undefined && LAST.offsets ? LAST.offsets[key] : null;
      if (!g) { say2('Offset 微調：請先點一個部位（Loader、Auto1…）再微調 —— golden 還沒選部位時存不到東西', '#ffcc66'); return; }
      if (!canSend(g.button)) { say2('Offset 微調：部位鈕 ' + g.button + ' 伺服器說點不到，沒有送', '#f88'); return; }
      var gen = GEN;
      INFLIGHT = { control: id, key: key };
      var state = groupState(key);
      send({ form: FORM, control: g.button, event: 'click' }, 0).then(function (a1) {           // ①
        if (gen !== GEN) return null;
        if (a1 && a1.messages && a1.messages.length) say2(a1.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
        return send({ form: FORM, control: id, event: 'click', state: state }, 0);             // ②
      }).then(function (a2) {
        if (!a2 || gen !== GEN) return;
        applyAfter(key, a2.changed);
        var lines = ['✔ ' + g.part + '（' + g.button + '）已微調並存檔（golden ' + id + 'Click → spbSaveClick）'];
        (a2.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });
        if ((a2.todo || []).length) lines.push('⚠ golden 還有沒做到的步驟：' + a2.todo.join('；'));
        say2(lines.join('\n'), (a2.todo || []).length ? '#ffcc66' : '#9f9');
      }).catch(function (e) {
        var msg = (e && e.message) || String(e);
        if (/reload page/.test(msg)) {
          say2('Offset：伺服器要求重新開頁（' + msg + '）—— 重讀中（這一下沒有存）');
          if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
          return;
        }
        say2('Offset 微調 ' + id + ' 沒有完成（form.event）：' + msg, '#f88');
      }).then(function () { INFLIGHT = null; });
    };
  }
  function hook() {
    BTNS.forEach(function (id) {
      var b = $(id);
      if (!b || b.__evb3) return;
      b.__evb3 = true;
      b.addEventListener('click', function (ev) { ev.preventDefault(); });
      b.addEventListener('click', onClick(id));
    });
    var px = LAST && LAST.proxies && LAST.proxies.pan_AutoOffsetMove, pan = $('pan_AutoOffsetMove');   // 見檔頭「另」
    if (px && pan && px.visible === true && pan.style.display === 'none') pan.style.display = '';
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++;
      setTimeout(function () {
        try { hook(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); }
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (INFLIGHT) return Promise.reject(new Error('Offset 微調還在等伺服器回覆（form.event）—— 這次沒有存檔；等微調完成再按一次存檔'));
    return save0.apply(this, arguments);
  };

  window.HT9045EvB3Offset = {                             // 探針／除錯用
    eventTag: eventTag, inflight: function () { return INFLIGHT; }, groupState: groupState
  };
})();
