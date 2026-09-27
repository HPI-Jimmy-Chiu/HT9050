/* ht9045_groundman_c.js -- Status.GroundMan.html（golden TfGroundMan，GroundMan\GroundMan.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-CRT-GroundMan) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，
 *   免得產生器覆蓋）。
 *
 * 後端：FileRW/GroundMan.cpp（WS editlist.get／editlist.save tag=GroundMan）。
 *   開頁＝golden FormShow（:139）：依 Gerneral.ini [Ground_Man] Ground_Man_ScanPoint（8／22／28 點）顯示板子、通道名稱。
 *   存檔＝golden spbSaveClick（:1418）：D:\HT9045\system\GroundMan.ini [System] Alarm_Continuous_Time／Alarm_Occurrences
 *        → ReadGroundOffset（:1573）重讀。golden 沒有 YES/NO 確認框 → 不登錄 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 取代 B 路接線檔 ht9045_wire_statusgroundman.js（那份只有 2 個 pending、沒有欄位 —— 抽取器找不到 GroundMan.ini，因為它不是配方檔）。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Status.GroundMan.html' → 'GroundMan'）照通用規則讀寫兩個輸入框；本檔補：
 *
 *  (1) 存檔鈕 spbSave → 引擎 HT9045Page.save()（沒有 B 路欄位，引擎不會自己攔存檔鈕）。
 *      sbtExit 照頁面內建 .exitbtn 關視窗 —— golden sbtExitClick（:1472）→ Close → FormClose（:228）會 ReStart()＝RS232 重開，網頁不做。
 *  (2) 引擎沒有套的替身狀態：TLabel 的 Caption（labCH_*、labUseOffset、labResetByStart）、DFM Visible=False 產生的
 *      display:none（labBoardOhrm*／labBoardVersion*：golden FormShow 依點數打開）；labStatus（golden Timer1Timer :572，開頁當下值）。
 *  (3) 硬體／執行期鈕停用（網頁 → 機台的指令通道還沒設計；RS232 指令是 S48 待辦）：
 *      spbStartCom（:247 comGM->StopComm＋Init_GM_RS232 開 COM）、spbStopCom（:253 comGM->StopComm）、
 *      btnMaintenanceMode（DFM 無 OnClick；按下狀態讓 DoGroundMasterMonitor :1289／:1389 壓掉 PeiXing 的接地警報）。
 *  (4) 執行期顯示沒有接：labValue_*（阻值）、led_*、labCount_*（警報計數）、labBoardOhrm*／labBoardVersion*（板子內阻／版本）、
 *      Log 分頁 —— 寫它們的是 RS232 監測（comGMReceiveData :303、DoGroundMasterMonitor :577、ShowGroundManLog :1480，S48）。
 *      畫面保留 DFM 設計期字樣（golden 沒連上時也是這樣），滑鼠提示說明。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'GroundMan';
  var PAGE = 'Status.GroundMan.html';

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[GroundMan/C] ' + msg);
  }
  function addTitle(el, text) {
    if (!el || el.__gmT === text) return;
    el.__gmT = text;
    el.title = (el.title ? el.title + '\n' : '') + text;
  }

  /* ---- (3) 硬體／執行期鈕停用 ------------------------------------------------------------------- */
  var HW_NOTE = '網頁停用：RS232 通訊指令（S48 待辦；網頁 → 機台的指令通道還沒設計，互鎖在 C++ 端）。這一頁只做 GroundMan.ini 的讀寫。';
  var ACTIONS = {
    spbStartCom: 'golden spbStartComClick（GroundMan.cpp:247）：comGM->StopComm() → Init_GM_RS232()（:259，重讀 GroundMan.ini、開 COM 9600-8-N-1）',
    spbStopCom: 'golden spbStopComClick（GroundMan.cpp:253）：comGM->StopComm()、bRs232Ok=false',
    btnMaintenanceMode: 'golden btnMaintenanceMode（DFM 無 OnClick）：按下時 DoGroundMasterMonitor（:1289／:1389）不發 WAR1609 接地警報（CC_SIGURD_PeiXing）'
  };
  function lockActions() {
    Object.keys(ACTIONS).forEach(function (id) {
      var el = $(id);
      if (!el) return;
      el.disabled = true;                      // 每次開頁都重設：引擎 gbSetEnabled 會打開它自己標記（data-gb-dis）關過的元件
      el.removeAttribute('data-gb-dis');
      addTitle(el, ACTIONS[id] + '。' + HW_NOTE);
    });
    addTitle($('sbtExit'), 'golden sbtExitClick（:1472）→ Close → FormClose（:228）會 ReStart()（RS232 重開，CC_SIGURD_HUKOU／PeiXing 除外）；網頁只關視窗。');
  }

  /* ---- (4) 執行期顯示：說明沒有接 --------------------------------------------------------------- */
  var RT_NOTE = '執行期值未接：golden 由 RS232 監測（comGMReceiveData :303／DoGroundMasterMonitor :577，S48）寫入；目前是 DFM 設計期字樣。';
  function markRuntime() {
    for (var b = 0; b < 4; b++) {
      for (var c = 0; c < 8; c++) {
        ['labValue_', 'led_', 'labCount_'].forEach(function (p) { addTitle($(p + b + '_' + c), RT_NOTE); });
      }
    }
    ['labBoardOhrm', 'labBoardOhrm2', 'labBoardOhrm3', 'labBoardOhrm4',
     'labBoardVersion', 'labBoardVersion2', 'labBoardVersion3', 'labBoardVersion4', 'mmGroundManLog'].forEach(function (id) {
      addTitle($(id), RT_NOTE);
    });
  }

  /* ---- (2) 引擎沒有套的替身狀態 ---------------------------------------------------------------- */
  function applyExtra(d) {
    if (!d) return;
    var px = d.proxies || {};
    Object.keys(px).forEach(function (id) {
      var el = $(id), v = px[id];
      if (!el) return;
      // TLabel Caption：只套在純文字元件上（<span class="lb">，沒有子元件、不是輸入框）
      if (v.caption !== undefined && !('value' in el) && !el.children.length) el.textContent = v.caption;
      // DFM Visible=False 的元件頁面產生器畫成 display:none；引擎只切 visibility → golden FormShow 打開時這裡要把 display 還原
      if (v.visible === true && el.style.display === 'none') el.style.display = '';
    });
    var x = d.extra || {};
    var st = $('labStatus');
    if (st && x.labStatus !== undefined) {
      st.textContent = x.labStatus;
      addTitle(st, 'golden Timer1Timer（:572）labStatus＝iGroundMasterTask（RS232 監測狀態）。這裡是開頁當下的值，不會跟著更新（S48）。');
    }
    var lines = [];
    if (x.useGroundMan === 0) {
      lines.push('ⓘ 這台 Gerneral.ini [Ground_Man] USE_GROUND_MAN=0：golden 主畫面不顯示 GroundMan 鈕（main.cpp:24327），也不開 RS232；' +
                 '存檔照 golden 仍會寫 system\\GroundMan.ini。');
    }
    if (x.scanPoint !== undefined) {
      lines.push('ⓘ Ground_Man_ScanPoint=' + x.scanPoint + '（' + (['8 點', '22 點', '28 點'][x.scanPoint] || '?') + '，' +
                 x.useGndBoard + ' 塊板）；COM=' + (x.comPort || '') + '；阻值上限 ' + x.alarmOhm + ' Ohm。');
    }
    if (x.runtime && x.runtime.wired === false) lines.push('⚠ 阻值／燈號／計數／Log 是執行期顯示，尚未接（RS232 監測，S48）。');
    if (lines.length) say2(lines.join('\n'));
    lockActions();
    markRuntime();
  }

  var R = window.HT9045Recipe;
  if (R && R.editlistGet && !R.__groundManCWrapped) {
    R.__groundManCWrapped = true;
    var get0 = R.editlistGet;
    R.editlistGet = function (st) {
      if (st !== STRUCT) return get0.apply(this, arguments);
      return get0.apply(this, arguments).then(function (d) {
        setTimeout(function () { applyExtra(d); }, 0);   // 引擎在這個 promise 的 then 裡套值；之後再補
        return d;
      });
    };
  }

  /* ---- (1) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('spbSave');
    if (!b || b.__gmC) return;
    b.__gmC = true;
    b.title = 'spbSave : golden spbSaveClick（GroundMan.cpp:1418）→ system\\GroundMan.ini [System] Alarm_Continuous_Time／Alarm_Occurrences' +
              ' → ReadGroundOffset 重讀（ContinuousTime/Occurrences 必須大於 5，否則不寫）';
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

  function onReady() { lockActions(); markRuntime(); bindSave(); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();

  /* ---- 引擎註冊（取代 B 路接線檔的 register；只留小鍵盤） -------------------------------------------- */
  // golden edContinuous_TimeMouseDown（:1630）ShowQwertyKey(Sender, N_INTEGER, 5, true, 5, 50)、
  //        edOccurrencesMouseDown（:1624）ShowQwertyKey(Sender, N_INTEGER, 1, true, 1, 10)（沿用 ht9045_wire_statusgroundman.js 的 kb 表）
  var kb = {
    edContinuous_Time:           ['INTEGER', 5, true, 5, 50],
    edOccurrences:               ['INTEGER', 1, true, 1, 10]
  };
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'groundman_c', fields: {}, optional: {}, kb: kb });
  }
})();
