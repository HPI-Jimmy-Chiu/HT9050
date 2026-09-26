/* ht9045_barcode_c.js -- Setup.BarCode.html（golden TfBarCode，BarCode\BarCode.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_BarCode.cpp（WS editlist.get／editlist.save tag=TestIF_File_BarCode）。
 * 取代 B 路兩份接線檔（ht9045_wire_barcode.js／ht9045_wire_setupbarcode.js：只接 [Lot Verification] 6 鍵（optional），
 * 直接讀寫 HandlerCondition.Data）——同一個檔只能有一個寫者，[Configuration]／[Lot Verification] 現在由 golden spbSaveClick（:1268）寫。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Setup.BarCode.html' → 'TestIF_File_BarCode'）照通用規則讀寫元件；本檔只補兩件事：
 *
 *  (1) 存檔鈕 spbSave → 引擎 HT9045Page.save()（golden spbSaveClick 沒有 YES/NO 確認框 → 不登錄 GB_SAVE_Q，頁面仍確認一次，引擎慣例）。
 *      B 路接線檔拿掉之後引擎不會自己攔存檔鈕（它只在有 B 路欄位時裝攔截），所以由這裡綁。
 *      sbtExit 照頁面內建 .exitbtn 關視窗 —— golden sbtExitClick → Close() → FormClose（DoIniDataToForm），不寫檔。
 *  (2) 小鍵盤：沿用 ht9045_wire_setupbarcode.js 的 kb 表（golden ShowQwertyKey 參數，gen_wire.py 從 golden 抽出；
 *      golden 範圍 max<=0 的 7 個欄位照那份的決斷關掉夾限）。
 *
 * golden 事件（chkMulti2DIDClick／rgMulti2DTypeClick）在伺服器端存檔前重播（FileRW/TestIF_File_BarCode.cpp BeforeApply）；
 * 頁面上切換不會即時重建 cbAa..cbBb 的選項（下次重讀時由後端 golden SetMulti2DMap 的結果套上）。
 * 客戶專屬（Steven 20260925 決定先跳過，只註記）：tsCCD_Unloader（UnloaderClip，bReadClipCodeFromUnloader：HONPREC_QC／JSSI）
 * 的 12 個 IP／Port 欄位後端沒有替身（elUnloaderClip 未移植）→ 頁面上改了不會存。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_BarCode';
  var PAGE = 'Setup.BarCode.html';

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[BarCode/C] ' + msg);
  }

  /* ---- (1) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('spbSave');
    if (!b || b.__bcC) return;
    b.__bcC = true;
    b.title = 'spbSave : golden spbSaveClick（BarCode.cpp:1268）→ HandlerCondition.Data [Configuration]／[Lot Verification] → ReadFile → BackupSetupFile';
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
  bindSave();

  /* ---- (2) 引擎註冊（取代 B 路兩份接線檔的 register；只留小鍵盤） --------------------------- */
  var kb = {
    IPPort1:                     ['PORT', 0, false, 0, 0],
    edAutoAdjustLightTimeOut:    ['INTEGER', 2, true, 0, 100000],
    edBarcodePosDelayTime:       ['INTEGER', 2, true, 0, 5000],
    edBarcodeRetryCount:         ['INTEGER', 0, true, 0, 100],
    edBarcodeScanDelayTime:      ['INTEGER', 2, true, 0, 100000],
    edConsecutiveFailure:        ['INTEGER', 0, true, 0, 100],
    edFirstDelay:                ['INTEGER', 2, true, 0, 5000],
    edHandShakeTimeOut:          ['INTEGER', 2, true, 0, 100000],
    edRetryOffsetMove:           ['INTEGER', 0, false, 0, 0],
    edRetryShiftOffsetMove:      ['INTEGER', 0, false, 0, 0],
    edSFCAutoRetry:              ['INTEGER', 0, true, 0, 100],
    edSFCExposureTimeOut:        ['INTEGER', 2, true, 0, 100000],
    edSFCGetResultTimeOut:       ['INTEGER', 2, true, 0, 100000],
    edSFCStartDelay:             ['INTEGER', 2, true, 0, 5000],
    edSFCUse2PhotoOffset:        ['INTEGER', 0, true, 0, 100],
    edShtDuplicateRetryCnt:      ['INTEGER', 0, true, 0, 100],
    edShuttle_1A_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_1B_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_2A_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_2B_Port:           ['PORT', 0, false, 0, 0],
    edTriggerTime:               ['INTEGER', 2, true, 0, 100000],
    ed_2D_YieldIgnoreCnt:        ['INTEGER', 2, true, 1, 100],
    edt1stLineLength_2DBarCode:  ['INTEGER', 0, false, 0, 0],
    edt2DEnd:                    ['INTEGER', 0, true, 0, 100],
    edt2DIDYield:                ['DOUBLE', 3, true, 0.001, 100.0],
    edt2DStart:                  ['INTEGER', 0, true, 0, 100],
    edt2ndLineLength_2DBarCode:  ['INTEGER', 0, false, 0, 0],
    edtCheckSumLength:           ['INTEGER', 0, true, 0, 100],
    edtLotIDEnd:                 ['INTEGER', 0, true, 0, 100],
    edtLotIDStart:               ['INTEGER', 0, true, 0, 100],
    edtOffsetX:                  ['DOUBLE', 2, false, 0, 0],
    edtOffsetY:                  ['DOUBLE', 2, false, 0, 0],
    edtXPitch:                   ['DOUBLE', 2, false, 0, 0]
  };
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'barcode_c', fields: {}, optional: {}, kb: kb });
  }
})();
