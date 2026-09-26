/* ht9045_shuttlemove_c.js -- HW.ShuttleMove.html（golden TfShuttleMove，ShuttleMove.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260926（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/ShuttleMove.cpp（WS editlist.get／editlist.save tag=ShuttleMove）。
 *   開頁＝golden FormShow（:67）：十個 Shuttle 教導點（Tech.*）→ 輸入框；latch 機台另外讀 HandlerCondition.Data [Shuttle]。
 *   存檔＝golden sbUpdateClick（:1965）：輸入框 → Tech.* → system\teach.ini（fTeach->SaveFile(true)）；latch 機台另寫
 *   HandlerCondition.Data [Shuttle] InSH1SenICAddPos／InSH2SenICAddPos。Steven 裁決 S47：沒有 latch 的機台不讀寫 [Shuttle]。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'HW.ShuttleMove.html' → 'ShuttleMove'）照通用規則讀寫輸入框；本檔補三件事：
 *
 *  (1) 存檔鈕 sbUpdate → 引擎 HT9045Page.save()（這一頁沒有 B 路欄位，引擎不會自己攔存檔鈕）。golden sbUpdateClick 沒有
 *      YES/NO → 不登錄 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。sbtExit 照頁面內建 .exitbtn 關視窗；golden sbtExitClick
 *      （:1956 fAllMotorHome=false＋Close → FormClose :2132 SystemStart=false）是機台狀態動作，網頁不做。
 *  (2) golden ShowShuttleSensorPosition（:1998）的四個格子（TTMyTray）：數字在回應的 extra.trays，照 XItem 重畫；
 *      palSh1Encoder／palSh2Encoder（golden FormShow :137-138 開頁讀編碼器）、gbScanOutShuttle 標題在 extra.captions。
 *      mtInSHBarCodePos 的格子 golden 由 fBarCode->GetMovePos 算，移植樹沒有 → 格子留空、標出缺口（portGap）。
 *  (3) 機台動作鈕停用（網頁 → 機台的指令通道還沒設計；互鎖與安全判斷在 C++ 端，瀏覽器不可直接驅動硬體）：
 *      移動／掃描／latch 自動教導（ShuttleMoveClick :1410）、T.Step（btnTStepClick :2172 fMain->Start）、Start（btStartClick :2247）、
 *      Retry（btRetryClick :1405）、Sensor Adj.（sbShuttleSensorClick :1769）、Sen. Latch（sbSensorLatchClick :2144 開 fLtcSensor）、
 *      Bar Code（sbBarCode，golden DFM 沒有 OnClick）、mtOutSHDetectPos 雙擊移動（mtOutSHDetectPosMouseDown :2178）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'ShuttleMove';
  var PAGE = 'HW.ShuttleMove.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || R.__shuttleMoveCWrapped) return;
  R.__shuttleMoveCWrapped = true;

  var HW_NOTE = '網頁停用：golden 這顆鈕會驅動機台（網頁 → 機台的指令通道還沒設計；互鎖在 C++ 端）。這一頁只做教導點與設定檔的讀寫。';
  var ACTIONS = {
    btShu1Left: 'ShuttleMoveClick :1410（Shuttle1 移到左側）', btShu1Right: 'ShuttleMoveClick :1410（Shuttle1 移到右側）',
    btShu2Left: 'ShuttleMoveClick :1410（Shuttle2 移到左側）', btShu2Right: 'ShuttleMoveClick :1410（Shuttle2 移到右側）',
    btScanOutShu1: 'ShuttleMoveClick :1410（掃描 Out Shuttle1）', btScanOutShu2: 'ShuttleMoveClick :1410（掃描 Out Shuttle2）',
    btnInSH1Sen7DetectPos: 'ShuttleMoveClick :1410（In Shuttle1 移到感測器）', btnInSH2Sen7DetectPos: 'ShuttleMoveClick :1410（In Shuttle2 移到感測器）',
    btInSH1BarCodePos: 'ShuttleMoveClick :1410（移到 Bar Code 位置）', btInSH2BarCodePos: 'ShuttleMoveClick :1410（移到 Bar Code 位置）',
    btOutSH1ZDetectPos: 'ShuttleMoveClick :1410（Out Shuttle1 感測）', btOutSH2ZDetectPos: 'ShuttleMoveClick :1410（Out Shuttle2 感測）',
    btOutSH1OneRowDetectPos: 'ShuttleMoveClick :1410（Out Shuttle1 One Row 感測）', btOutSH2OneRowDetectPos: 'ShuttleMoveClick :1410（Out Shuttle2 One Row 感測）',
    btnInSH1SenICDetectPos: 'ShuttleMoveClick :1410（latch 感測）', btnInSH2SenICDetectPos: 'ShuttleMoveClick :1410（latch 感測）',
    btnInSH1SenICDetectAction: 'ShuttleMoveClick :1410 → DoInShuttleChkStackAction :2387（latch 自動教導）',
    btnInSH2SenICDetectAction: 'ShuttleMoveClick :1410 → DoInShuttleChkStackAction :2387（latch 自動教導）',
    btnTStep: 'btnTStepClick :2172（fMain->Start）', btStart: 'btStartClick :2247（fMain->BtnStartClick）',
    btRetry: 'btRetryClick :1405（bShuttleRetry）', sbShuttleSensor: 'sbShuttleSensorClick :1769（Shuttle 感測器調整）',
    sbSensorLatch: 'sbSensorLatchClick :2144（開 fLtcSensor）', sbBarCode: 'sbBarCode（golden DFM 沒有 OnClick）'
  };

  function $(id) { return document.getElementById(id); }
  function say2(msg) {
    if (window.HT9045Wire && HT9045Wire.say) {
      var b = $('ht9045WireBar');
      var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
      HT9045Wire.say((prev ? prev + '\n' : '') + msg, (b && b.style.color) || '#ffcc66');
    }
    console.info('[ShuttleMove/C] ' + msg);
  }

  /* ---- (3) 機台動作鈕停用 ------------------------------------------------------------------ */
  function lockActions() {
    Object.keys(ACTIONS).forEach(function (id) {
      var el = $(id);
      if (!el) return;
      el.disabled = true;                      // 每次開頁都重設：引擎 gbSetEnabled 會打開它自己標記（data-gb-dis）關過的元件
      el.removeAttribute('data-gb-dis');
      if (el.__smLocked) return;
      el.__smLocked = true;
      el.title = (el.title ? el.title + '\n' : '') + 'golden ' + ACTIONS[id] + '。' + HW_NOTE;
    });
    var t = $('mtOutSHDetectPos');
    if (t) t.title = (t.title ? t.title + '\n' : '') + 'golden mtOutSHDetectPosMouseDown :2178 雙擊格子會移動 Shuttle。' + HW_NOTE;
  }

  /* ---- (2) 格子與標題 ------------------------------------------------------------------------ */
  function drawTray(name, t) {
    var ph = $(name), W = window.HTWidgets;
    if (!ph || !W || !W.makeMyTray || !t) return;
    var spec = {};
    try { spec = JSON.parse(ph.dataset.tray || '{}'); } catch (e) { spec = {}; }
    var xi = t.xItem > 0 ? t.xItem : (spec.xitem || 1), yi = t.yItem || spec.yitem || 2;
    var w = spec.w || ph.clientWidth || 600, h = spec.h || ph.clientHeight || 80;
    var cells = (t.cells || []).map(function (c) { return { x: c[0], y: c[1], text: String(c[2]) }; });
    var tray = W.makeMyTray({ name: name + '_t', xitem: xi, yitem: yi, cellW: Math.max(4, Math.floor((w - 10) / xi)),
                              cellH: Math.max(4, Math.floor((h - 10) / yi)), trayColor: spec.trayColor, trayDirect: spec.trayDirect,
                              cells: cells });
    while (ph.firstChild) ph.removeChild(ph.firstChild);
    ph.appendChild(tray);
    if (t.portGap) {
      tray.title += '\n⚠ golden 由 fBarCode->GetMovePos 算格子（BarCode.cpp），移植樹沒有這一支 → 格子留空';
      tray.style.outline = '2px dashed #cc8800';
    }
  }
  function setCaption(id, text) {
    var el = $(id);
    if (!el || text === undefined) return;
    var cap = el.querySelector('.pnlCap') || el.querySelector('legend');
    if (cap) cap.textContent = text;
  }
  function applyExtra(d) {
    var x = d && d.extra;
    if (!x) return;
    Object.keys(x.trays || {}).forEach(function (n) { drawTray(n, x.trays[n]); });
    Object.keys(x.captions || {}).forEach(function (n) { setCaption(n, x.captions[n]); });
    lockActions();
  }

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      setTimeout(function () { applyExtra(d); }, 0);     // 引擎在這個 promise 的 then 裡套值（含 applyNoSource 的 '---'）；之後再畫
      return d;
    });
  };

  /* ---- (1) 存檔鈕 ----------------------------------------------------------------------- */
  function bindSave() {
    var b = $('sbUpdate');
    if (!b || b.__smC) return;
    b.__smC = true;
    b.title = 'sbUpdate : golden sbUpdateClick（ShuttleMove.cpp:1965）→ Tech.* → fTeach->SaveFile(true)（system\\teach.ini）；' +
              'latch 機台另寫 HandlerCondition.Data [Shuttle]';
    b.addEventListener('click', function (ev) {
      ev.preventDefault(); ev.stopPropagation();
      if (b.disabled) return;
      if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔'); return; }
      if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
        say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔');
        return;
      }
      HT9045Page.save();
    });
  }

  function onReady() { lockActions(); bindSave(); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();
})();
