/* ht9045_agv_c.js -- Setup.AGV.html（golden TfAGV，V912 Automation\AGV.cpp）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * AI(W906-B8-AG1) 20260930 [W906] St01 新檔（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *   派工 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」（Steven 20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
 *
 * 後端：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_AGV.cpp（WS editlist.get／editlist.save tag=TestIF_File_AGV）。
 *   開頁＝golden FormShow（:1304）：ReadFile（:1227，D:\HT9045\config\AGV.ini [Configuration] 26 鍵）→ DoIniDataToForm（:1264）。
 *   存檔＝golden spbSaveClick（:1184）：26 個 WriteIniData → ReadFile。golden 沒有 YES/NO 確認框 → 不登錄 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Setup.AGV.html' → 'TestIF_File_AGV'）照通用規則讀寫 25 格與 cbEnableAGVFunction。
 *   ⚠ 那一行在筆電的引擎檔，要筆電同意後由整合者加（St01 交件附了那一行）；還沒加之前，這一頁開頁不讀、存檔鈕只提示、不存。
 * 本檔補兩件事：
 *  (1) 存檔鈕 spbSave → 引擎 HT9045Page.save()（這一頁沒有 B 路欄位，引擎不會自己攔存檔鈕）。
 *      sbtExit 照頁面內建 .exitbtn 關視窗 —— golden sbtExitClick（:1297）→ Close → FormClose（:1311）只有 fShow=false，不寫檔。
 *  (2) 小鍵盤：golden edE84_1_TP1MouseDown（:1445）N_INTEGER (0, 300)（22 格逾時共用）、edAuto1CountMouseDown（:1439）N_INTEGER (0, 20)。
 *
 * ⚠ E84 會動（golden 行為，照做）：開頁／存檔都跑 golden ReadFile，把 AGV.ini "E84 Enable" 讀進 TestIF_File.bEnableE84；
 *   Gerneral.ini [System] AGVModal=1（USE_E84_Sensor）的機台，E84 交握從那一刻起每一拍都跑（移植樹 csystem.cpp:30394），
 *   會開關 E84 交握輸出（SW[SwE84_1_*]／SW[SwE84_2_*]）。AGVModal=0 的機台伺服器照 golden 不讓這一頁開（spbAGV 看不見）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_AGV';
  var PAGE = 'Setup.AGV.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__agvC) return;
  R.__agvC = true;

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    if (window.console) console.info('[AGV/C] ' + msg);
  }

  /* ---- (1) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('spbSave');
    if (!b || b.__agvC) return;
    b.__agvC = true;
    b.title = 'spbSave : golden spbSaveClick（Automation/AGV.cpp:1184）→ D:\\HT9045\\config\\AGV.ini [Configuration] 26 鍵 → ReadFile' +
              '（Enable AMR function 勾著、而且 AGVModal=1 的機台，E84 交握會開始跑）';
    b.addEventListener('click', function (ev) {
      ev.preventDefault(); ev.stopPropagation();
      if (!usable(b)) return;
      if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔', '#f88'); return; }
      if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
        say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88');
        return;
      }
      HT9045Page.save();
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bindSave); else bindSave();

  /* ---- (2) 引擎註冊（這一頁沒有 B 路欄位；只給小鍵盤） ------------------------------------ */
  var kb = {};
  ['1', '2'].forEach(function (s) {
    ['TP1', 'TP2', 'TP3', 'TP4', 'TP5', 'TP6', 'TA1', 'TA2', 'TA3', 'TD0', 'TD1'].forEach(function (t) {
      kb['edE84_' + s + '_' + t] = ['INTEGER', 0, true, 0, 300];     // golden AGV.cpp:1448 edE84_1_TP1MouseDown
    });
  });
  ['edAuto1Count', 'edAuto2Count', 'edAuto3Count'].forEach(function (id) {
    kb[id] = ['INTEGER', 0, true, 0, 20];                              // golden AGV.cpp:1442 edAuto1CountMouseDown
  });
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'agv_c', fields: {}, optional: {}, kb: kb });
  }
})();

/* ---------------------------------------------------------------------------
 * AI(W906-B8-AG1) 20260930 [W906] St01 patch B（Steven 請看：按下去就關真機的 E84 交握輸出）：
 *   Initial Load／Initial Unload 兩顆鈕 → WS form.event {"form":"TfAGV","control":"btInitalLoad"|"btInitalUnLoad","event":"click"}
 *   （tag "Setup.AGV"；不帶 state：處理器不讀畫面上的值）。C++：D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_AGV.gen.inc 的
 *   AG_btInitalLoadClick／AG_btInitalUnLoadClick＝golden V912 Automation\AGV.cpp:1316-1322／:1324-1330：
 *     Load   → iE84LoadTask=1、SW[SwE84_1_LREQ／UREQ／VA／READY／VS0／VS1].Off()、bE84LoaderActionflag[0..2]=false
 *     Unload → iE84UnloadTask=1、SW[SwE84_2_* 同 6 顆].Off()、bE84UnloaderActionflag[0..2]=false
 *   golden 不問、不查（照做，這裡也不另外問）；點不點得到照伺服器（editlist.get 的 events.<id>.operable）。
 *   ⚠ 運轉中伺服器不收設定頁事件（回 running）；golden 非模態，運轉中按得到 —— 差異寫在 C++ 檔尾。⛔ 20260930 更正 AI(W906-FE-RUNEXC)：C++ 已開例外（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\_FormEvent.cpp 檔尾），這兩顆運轉中照 golden 收（要頁面表說 AGV 開著）；本檔運轉中沒有停用它們，不用改；running 那一支留著給頁面表沒看到視窗的時候。
 *   防連點：上一下還沒回覆不送；busy: 等 450 ms 重送（最多 3 次）；not-operator 續權杖再送一次；"reload page" → HT9045Page.load()。
 * --------------------------------------------------------------------------- */
(function () {
  'use strict';

  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || !R.rawCmd || R.__agvEvB8) return;
  R.__agvEvB8 = true;

  var STRUCT = 'TestIF_File_AGV', EV_TAG = 'Setup.AGV', FORM = 'TfAGV', LOG = '[AGV/B8] ';
  var BTNS = {
    btInitalLoad:   'Initial Load：E84_1 交握輸出 L_REQ／U_REQ／VA／READY／VS_0／VS_1 關掉、Loader 交握回到第 1 步、放掉 Loader 軌道互鎖',
    btInitalUnLoad: 'Initial Unload：E84_2 交握輸出 L_REQ／U_REQ／VA／READY／VS_0／VS_1 關掉、Unloader 交握回到第 1 步、放掉 Unloader 軌道互鎖'
  };
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
  function evInfo(id) { var e = LAST && LAST.events; return (e && typeof e === 'object' && e[id]) || null; }
  function canSend(id) { var e = evInfo(id); return !!e && e.event === 'click' && e.operable !== false; }

  function send(v, tries) {
    var extra = { tag: EV_TAG, value: JSON.stringify(v) };
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

  function onClick(id) {
    return function (ev) {
      if (ev && ev.isTrusted === false) return;             // 別的程式 dispatch 的 click 不是操作員按的
      var b = $(id);
      if (!LAST) { say2('AMR ' + id + '：這一頁還沒從伺服器讀到（editlist.get）—— 沒有送', '#f88'); return; }
      if (INFLIGHT) { say2('AMR：上一下（' + INFLIGHT + '）還在等伺服器回覆，這一下沒有送'); return; }
      if (!usable(b) || !canSend(id)) { say2('AMR ' + id + '：伺服器說現在點不到（events.operable=false）—— 沒有送', '#f88'); return; }
      var gen = GEN;
      INFLIGHT = id;
      send({ form: FORM, control: id, event: 'click' }, 0).then(function (a) {
        if (!a || gen !== GEN) return;
        var lines = ['✔ ' + BTNS[id] + '（golden ' + id + 'Click）'];
        (a.messages || []).forEach(function (m) { lines.push('訊息：' + (m.zh || m.en)); });
        if ((a.todo || []).length) lines.push('⚠ ' + a.todo.join('；'));
        say2(lines.join('\n'), (a.todo || []).length ? '#ffcc66' : '#9f9');
      }).catch(function (e) {
        var msg = (e && e.message) || String(e);
        if (/reload page/.test(msg)) {
          say2('AMR：伺服器要求重新開頁（' + msg + '）—— 重讀中（這一下沒有送到）');
          if (window.HT9045Page && HT9045Page.load) HT9045Page.load();
          return;
        }
        if (/^running/.test(msg)) { say2('AMR ' + id + '：機台運轉中，伺服器不收設定頁事件（' + msg + '）—— 這一下沒有送到', '#f88'); return; }
        say2('AMR ' + id + ' 沒有完成（form.event）：' + msg, '#f88');
      }).then(function () { INFLIGHT = null; });
    };
  }
  function hook() {
    Object.keys(BTNS).forEach(function (id) {
      var b = $(id);
      if (!b || b.__agvB8) return;
      b.__agvB8 = true;
      b.title = id + ' : golden ' + id + 'Click（Automation/AGV.cpp:' + (id === 'btInitalLoad' ? '1316' : '1324') + '）—— ' + BTNS[id];
      b.addEventListener('click', function (ev) { ev.preventDefault(); });
      b.addEventListener('click', onClick(id));
    });
  }

  var get0 = R.editlistGet, save0 = R.editlistSave;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      LAST = d; GEN++;
      setTimeout(function () { try { hook(); } catch (e) { if (window.console) console.error(LOG + 'after load', e); } }, 0);
      return d;
    });
  };
  R.editlistSave = function (st) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    if (INFLIGHT) return Promise.reject(new Error('AMR：' + INFLIGHT + ' 還在等伺服器回覆（form.event）—— 這次沒有存檔；等它完成再按一次存檔'));
    return save0.apply(this, arguments);
  };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045AgvB8 = { inflight: function () { return INFLIGHT; }, last: function () { return LAST; } };   // 探針／除錯用
})();
