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
 *        ack.todo 會說明（B8／Jimmy）。⛔ 20261001 更正 AI(W906-B8-OS1B) St01：B8 OS-1b 已照 golden 接上——勾了就在伺服器回覆之後送 START（ack.afterAck 有一行說明，下面兩處照印）。
 *  送法（一次點擊送兩則，都等 ack）：
 *    ① {"form":"TfOffSet","control":"<目前部位鈕 id，例 sbLoader>","event":"click"}  → golden SpBotSelClick：伺服器換到頁面正在看的
 *       那一組（開頁時伺服器把每一組都跑過一次，停在最後一組；不先換就會存錯部位）。回覆的 changed 不套（頁面照自己的整包顯示）。
 *    ② {"form":"TfOffSet","control":"sb_AutoOffsetUp","event":"click","state":{這一組畫面上可改的值＋cb_AutoOffsetPositionCheck}}
 *       → 伺服器先套 state（＝golden 畫面上的值），再跑 golden 處理器（±0.1 後整組存）。回覆的 changed 只套這一組的欄位
 *       （edArmX／edArmY 的新值）與 labWarningForStop。
 *  跟 golden 不同（C 路頁面本來就是這樣，見 ht9045_offset_wire.js 檔頭）：golden 換部位時會先存上一組；這裡微調鈕只存目前這一組，
 *    別組沒按 Save 的修改還留在頁面上，照舊按 Save 才寫。沒有選部位時不送（golden 開窗後還沒選部位 iNowOffsetSel=-1，存不到東西）。
 *  ⛔ 更正 AI(W906-D013) 20260929 [W906] St01（todo D-013，decisions-decided R127／R128「20260929 結果」：照 BCB）：
 *    R127：golden 每換一次部位就先存上一組（SpBotSelClick :2487，存畫面上的值）。按微調時，① 之前先把「改過還沒存」的 stander 組
 *      （ht9045_offset_wire.js collect() 說 changed 的，目前這一組除外）照 golden 的順序點一遍：依序送那些組的部位鈕 click，
 *      點下一組時 state 帶上一組的值（伺服器先套上去，golden SpBotSelClick 就存那一組），最後點目前這一組（帶最後一個改過的組的值）
 *      ＝golden 操作員一路切過來、每切一次存上一次。存過的組把頁面的「原值」改成剛存的值（之後按 Save 不會再當成改過；改回原值也會照送）。
 *      special 組（Index／Tray 那一頁）不在這裡存：那些鈕在 golden 藏起來的 tsIndexOffset 頁上（R129），照舊按 Save 才寫。
 *    R128：沒有選部位也照送 ②（value 帶 "noPart":true、不送 ①）：golden iNowOffsetSel=-1 ⇒ SaveFile(-1) 什麼都不存，
 *      spbSaveClick 尾端照跑（重讀參數、Pause 計數、SetWorkParameter）。
 *  防連點：上一下還沒回覆，這一下不送（每一下都會存檔）；busy: 等 450 ms 重送（最多 3 次）；not-operator 續權杖再送一次；
 *    "reload page" → HT9045Page.load()；events.<id>.operable＝false 不送；存檔時還有微調在等回覆 → 這次不存。
 *  伺服器沒有 eventTag（C++ 還沒進來）＝一個都不送，按了只提示。
 *  另：pan_AutoOffsetMove 在 HTML 是 display:none（DFM Visible=False），引擎套 proxies 的 visible 只改 visibility、打不開 display ⇒
 *    伺服器說看得見（golden FormShow :572）時這裡打開。
 *
 *  AI(W906-B8-OS5) 20260930 [W906] St01：B8 OS-5（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「OS-5」；
 *    Steven 20260929「請按照bcb的邏輯處理」）—— Setup Teaching 分頁的 12 顆排序鈕 btnSortAuto1～6／btnSortFix1～6。
 *    golden V912 cOffSet.cpp:3211-3221 btnSortAuto1Click（12 顆共用）：A30 Setup Teach 開、離線、需要 Setup Teach ⇒ iSortUnloadT6＝鈕的 Tag
 *    （運轉中 Out Arm 放料時下一顆放到那一盤；按下當下機台不動；條件不成立 golden 什麼都不做，也不跳訊息）。
 *    送 {"form":"TfOffSet","control":"btnSortAuto3","event":"click"}（tag＝eventTag "Setup.OffSet"，跟微調鈕同一個別名頁）；不帶 state
 *    （處理器只讀全域和自己的 Tag）、不先點部位鈕（golden 不看部位）。C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.cpp 檔尾
 *    OS_EvSort、產生檔 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\Offset_File.gen.inc 的 OS_btnSortAuto1Click。
 *    grpSetupTeach 在 HTML 是 display:none（DFM Visible=False）⇒ 伺服器說看得見（golden FormShow :712-721，A30＋離線）時這裡打開，同上面 pan_AutoOffsetMove。
 *    點不點得到照伺服器（events.<id>.operable，ELOperable 沿 DFM 父層查）。防連點：上一下還沒回覆不送；busy: 重送；not-operator 續權杖。
 *    ⚠ 運轉中伺服器不收設定頁事件（回 running）；golden 運轉中按得到（Offset 非模態，A30 時 golden START 還會自己打開 Offset）——差異寫在 C++ 檔尾。⛔ 20260930 更正 AI(W906-FE-RUNEXC)：C++ 已開例外（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp 檔尾），這 12 顆運轉中照 golden 收（要頁面表說 Offset 開著）；本檔運轉中沒有停用它們，不用改；running 那一支留著給其他鈕與頁面表沒看到視窗的時候。
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
  var SORT = ['btnSortAuto1', 'btnSortAuto2', 'btnSortAuto3', 'btnSortAuto4', 'btnSortAuto5', 'btnSortAuto6',   // AI(W906-B8-OS5) 20260930 [W906]：OS-5（＝kOS_Events 的排序鈕列）
              'btnSortFix1', 'btnSortFix2', 'btnSortFix3', 'btnSortFix4', 'btnSortFix5', 'btnSortFix6'];
  var SORT_INFLIGHT = null;                             // AI(W906-B8-OS5) 20260930 [W906]：排序鈕送出中（跟微調鈕的 INFLIGHT 分開：排序鈕不存檔，不擋 Save）

  var LAST = null, GEN = 0, INFLIGHT = null;

  function $(id) { return document.getElementById(id); }
  // AI(W906-ES02-W161) 20261008（W-161，EastSun 1007「不要蓋住畫面」）：「golden 還有沒做到的步驟」是開發用說明 —— 只在 debug 進訊息框，release 寫 console
  function isDev() { try { return document.documentElement.getAttribute('data-mode') === 'debug'; } catch (e) { return false; } }
  function devTodo(lines, text) { if (isDev()) lines.push(text); else if (window.console) console.info(LOG + text); }
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
  function groupVals(key) {                              // AI(W906-D013) 20260929 [W906]：R127 一組可改欄位的工作副本值（不含勾選框）
    var H = window.HT9045Offset, g = LAST && LAST.offsets && LAST.offsets[key], out = {};
    if (!H || !g) return out;
    var w = H.work(key) || {};
    Object.keys(g.widgets || {}).forEach(function (id) {
      if (g.widgets[id] && g.widgets[id].editable && w[id]) out[id] = w[id];
    });
    return out;
  }
  function checkState() {                                // cb_AutoOffsetPositionCheck（golden 微調存完才看它，OS-1b）
    var ck = $(CHECK), c = ck ? (ck.tagName === 'INPUT' ? ck : ck.querySelector('input[type="checkbox"]')) : null, out = {};
    if (c) out[CHECK] = { checked: c.checked };
    return out;
  }
  function groupState(key) {
    var H = window.HT9045Offset, g = LAST && LAST.offsets && LAST.offsets[key];
    if (!H || !g) return {};
    H.collect();
    var out = groupVals(key), ck = checkState();
    Object.keys(ck).forEach(function (id) { out[id] = ck[id]; });
    return out;
  }
  // AI(W906-D013) 20260929 [W906]：R127 —— 存過的組：頁面的「原值」（editlist.get 回的 widgets，ht9045_offset_wire.js 拿它判斷改過沒）改成剛存的值
  function markSaved(key, vals) {
    var g = LAST && LAST.offsets && LAST.offsets[key];
    if (!g || !g.widgets) return;
    Object.keys(vals || {}).forEach(function (id) {
      var w = g.widgets[id], v = vals[id];
      if (!w || !v) return;
      if (v.checked !== undefined && w.checked !== undefined) w.checked = v.checked;
      else if (v.text !== undefined && w.text !== undefined) w.text = v.text;
    });
  }
  // R127：改過還沒存、目前這一組以外的 stander 組（照 collect() 的順序＝存檔送的順序），部位鈕伺服器點得到的
  function dirtyStander(key, skipped) {
    var H = window.HT9045Offset, c = H && H.collect ? H.collect() : null, out = [];
    if (!c || !c.widgets || !c.widgets.offsets) return out;
    Object.keys(c.widgets.offsets).forEach(function (k) {
      if (k === key || /:2$/.test(k) || !/^changed/.test((c.why && c.why[k]) || '')) return;
      var gg = LAST && LAST.offsets && LAST.offsets[k];
      if (gg && canSend(gg.button)) out.push(k); else skipped.push(k);
    });
    return out;
  }
  function partOf(k) { var gg = LAST && LAST.offsets && LAST.offsets[k]; return gg ? gg.part + '（' + gg.button + '）' : k; }
  function needsReload(a) { return ((a && a.todo) || []).some(function (t) { return /reload page/.test(t); }); }
  function applyAfter(key, ch) {                         // 只套這一組的欄位（值）與 labWarningForStop（golden spbSaveClick :2881）
    var g = LAST && LAST.offsets && LAST.offsets[key];
    Object.keys(ch || {}).forEach(function (id) {
      var v = ch[id] || {}, el = $(id);
      if (!el) return;
      if (g && g.widgets && g.widgets[id] && v.text !== undefined && g.widgets[id].text !== undefined) g.widgets[id].text = String(v.text);   // AI(W906-D013) 20260929 [W906]：R127 存過＝新原值
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
      var gen = GEN;
      if (!g && key !== null && key !== undefined) { say2('Offset 微調：頁面上的部位 ' + key + ' 在伺服器的資料裡找不到 —— 沒有送；請重讀', '#f88'); return; }
      if (!g) {                                          // AI(W906-D013) 20260929 [W906]：R128 照 BCB：沒選部位也照送（見檔頭 ⛔）
        INFLIGHT = { control: id, key: null };
        send({ form: FORM, control: id, event: 'click', noPart: true, state: checkState() }, 0).then(function (a0) {
          if (!a0 || gen !== GEN) return;
          applyAfter(null, a0.changed);
          var lines0 = ['✔ 還沒選部位：golden ' + id + 'Click → spbSaveClick 照跑，SaveFile(-1) 沒有存任何部位；存檔尾端照 golden 跑完（重讀參數、Pause 計數、SetWorkParameter）'];
          (a0.messages || []).forEach(function (m) { lines0.push('訊息：' + (m.zh || m.en)); });  (a0.afterAck || []).forEach(function (s) { lines0.push('▶ 勾了 Auto Offset Position Check：照 golden 存完就送 START（' + s + '）'); });   // AI(W906-B8-OS1B) 20261001 St01：同一行附加
          if ((a0.todo || []).length) devTodo(lines0, '⚠ golden 還有沒做到的步驟：' + a0.todo.join('；'));   // AI(W906-ES02-W161) 20261008: debug only
          say2(lines0.join('\n'), (a0.todo || []).length && isDev() ? '#ffcc66' : '#9f9');
        }).catch(function (e) {
          var msg0 = (e && e.message) || String(e);
          if (/reload page/.test(msg0)) {
            say2('Offset：伺服器要求重新開頁（' + msg0 + '）—— 重讀中（這一下沒有送到）');
            if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
            return;
          }
          say2('Offset 微調 ' + id + '（沒選部位）沒有完成（form.event）：' + msg0, '#f88');
        }).then(function () { INFLIGHT = null; });
        return;
      }
      if (!canSend(g.button)) { say2('Offset 微調：部位鈕 ' + g.button + ' 伺服器說點不到，沒有送', '#f88'); return; }
      INFLIGHT = { control: id, key: key };
      var skipped = [], chain = dirtyStander(key, skipped);   // AI(W906-D013) 20260929 [W906]：R127 照 BCB：先照 golden 一路切過來存上一組（見檔頭 ⛔）
      var seq = chain.concat([key]), saved = [], vals = {};
      seq.forEach(function (k) { vals[k] = groupVals(k); });
      var state = groupState(key);
      var step = function (i) {                          // ① 部位鈕：seq[i]；state＝上一組 seq[i-1] 的畫面值（golden :2487 存的就是它）
        var gg = LAST.offsets[seq[i]], v = { form: FORM, control: gg.button, event: 'click' };
        if (i > 0) v.state = vals[seq[i - 1]];
        return send(v, 0).then(function (a1) {
          if (gen !== GEN) return null;
          if (a1 && a1.messages && a1.messages.length) say2(a1.messages.map(function (x) { return x.zh || x.en; }).join('\n'));
          if (needsReload(a1)) throw new Error('reload page: ' + a1.todo.join('；'));
          var bc = a1 && Array.isArray(a1.barcode) ? a1.barcode.filter(function (x) { return x && x.accepted === false; })[0] : null;   // AI(W906-D013) 20260929 [W906]：KYEC：golden SpBotSelClick :2479-2484 先問條碼、沒刷就 return（上一組沒存、也沒換部位）
          if (bc) throw new Error('golden 換部位前要先刷條碼（' + (bc.caption || 'Input Operator ID:') + '，KYEC 刷條碼機台）—— Offset 頁的網頁刷條碼框還沒接，這一下沒有存、沒有微調');
          if (i > 0) { markSaved(seq[i - 1], vals[seq[i - 1]]); saved.push(seq[i - 1]); }
          return i + 1 < seq.length ? step(i + 1) : a1;
        });
      };
      step(0).then(function (a1) {
        if (!a1 || gen !== GEN) return null;
        return send({ form: FORM, control: id, event: 'click', state: state }, 0);             // ②
      }).then(function (a2) {
        if (!a2 || gen !== GEN) return;
        markSaved(key, groupVals(key));                   // AI(W906-D013) 20260929 [W906]：R127 這一組也存了（spbSaveClick SaveFile）；±0.1 的新值在 applyAfter
        applyAfter(key, a2.changed);
        var lines = ['✔ ' + g.part + '（' + g.button + '）已微調並存檔（golden ' + id + 'Click → spbSaveClick）'];
        if (saved.length) lines.push('   照 golden 換部位時先存上一組：' + saved.map(partOf).join('、') + ' 也已寫入');
        if (skipped.length) lines.push('   ⓘ 改過但伺服器點不到部位鈕、沒有先存（按 Save 才寫）：' + skipped.map(partOf).join('、'));
        (a2.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });  (a2.afterAck || []).forEach(function (s) { lines.push('▶ 勾了 Auto Offset Position Check：照 golden 存完就送 START（' + s + '）'); });   // AI(W906-B8-OS1B) 20261001 St01：同一行附加
        if ((a2.todo || []).length) devTodo(lines, '⚠ golden 還有沒做到的步驟：' + a2.todo.join('；'));   // AI(W906-ES02-W161) 20261008: debug only
        say2(lines.join('\n'), (a2.todo || []).length && isDev() ? '#ffcc66' : '#9f9');
      }).catch(function (e) {
        var msg = (e && e.message) || String(e);
        var part = saved.length ? '（已照 golden 先存：' + saved.map(partOf).join('、') + '）' : '';
        if (/reload page/.test(msg)) {
          say2('Offset：伺服器要求重新開頁（' + msg + '）' + part + ' —— 重讀中（這一下沒有微調）');
          if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
          return;
        }
        say2('Offset 微調 ' + id + ' 沒有完成（form.event）：' + msg + part, '#f88');
      }).then(function () { INFLIGHT = null; });
    };
  }
  // AI(W906-B8-OS5) 20260930 [W906]：OS-5 排序鈕 → golden btnSortAuto1Click（cOffSet.cpp:3211）
  function onSort(id) {
    return function (ev) {
      if (ev && ev.isTrusted === false) return;
      var b = $(id), name = id.replace(/^btnSort/, '');
      if (!eventTag()) { say2('Setup Teach 排序鈕：伺服器還沒有這個事件（editlist.get 沒有 eventTag）—— 沒有送', '#f88'); return; }
      if (SORT_INFLIGHT) { say2('Setup Teach 排序鈕：上一下（' + SORT_INFLIGHT + '）還在等伺服器回覆，這一下沒有送'); return; }
      if (!usable(b) || !canSend(id)) { say2('Setup Teach 排序鈕 ' + id + '：伺服器說現在點不到（golden 只在 A30 Setup Teach＋離線時顯示這一組，events.operable=false）', '#f88'); return; }
      var gen = GEN;
      SORT_INFLIGHT = id;
      send({ form: FORM, control: id, event: 'click' }, 0).then(function (a) {
        if (!a || gen !== GEN) return;
        var lines = ['✔ ' + name + '：已送出 golden btnSortAuto1Click —— A30 Setup Teach 開、離線、而且需要 Setup Teach 時，運轉中 Out Arm 會把下一顆放到 ' +
                     name + '；條件不成立 golden 什麼都不做。按下當下機台不動'];
        (a.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });
        if ((a.todo || []).length) lines.push('⚠ ' + a.todo.join('；'));
        say2(lines.join('\n'), (a.todo || []).length ? '#ffcc66' : '#9f9');
      }).catch(function (e) {
        var msg = (e && e.message) || String(e);
        if (/reload page/.test(msg)) {
          say2('Offset：伺服器要求重新開頁（' + msg + '）—— 重讀中（這一下沒有送到）');
          if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
          return;
        }
        if (/^running/.test(msg)) { say2('Setup Teach 排序鈕 ' + name + '：機台運轉中，伺服器不收設定頁事件（' + msg + '）—— 這一下沒有送到', '#f88'); return; }
        say2('Setup Teach 排序鈕 ' + id + ' 沒有完成（form.event）：' + msg, '#f88');
      }).then(function () { SORT_INFLIGHT = null; });
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
    SORT.forEach(function (id) {                          // AI(W906-B8-OS5) 20260930 [W906]：OS-5 排序鈕（見檔頭）
      var b = $(id);
      if (!b || b.__b8os5) return;
      b.__b8os5 = true;
      b.addEventListener('click', function (ev) { ev.preventDefault(); });
      b.addEventListener('click', onSort(id));
    });
    var pg = LAST && LAST.proxies && LAST.proxies.grpSetupTeach, grp = $('grpSetupTeach');   // AI(W906-B8-OS5) 20260930 [W906]：DFM Visible=False → HTML display:none（見檔頭）
    if (pg && grp && pg.visible === true && grp.style.display === 'none') grp.style.display = '';
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
