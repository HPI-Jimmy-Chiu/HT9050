/* ht9045_yieldmon_q41.js -- Setup.YieldMonitoring.html（golden TfYieldMonitoring，uYieldMonitoring.cpp）Q41 的頁面事件補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.8 的 YM-4。St02 的新檔（手寫，不是 gen_wire.py 產物）；
 * 頁面 Setup.YieldMonitoring.html:127 同一行載入，在 St01 的 ht9045_yieldmonitoring_c.js 之後（St02-M 認領 option A）。
 * 引擎 ht9045_wire_engine.js、St01 的 ht9045_yieldmonitoring_c.js 都不改。
 *
 *  YM-4  Timer1Timer（golden 912 uYieldMonitoring.cpp:3666-3676；906_0625_Steven :3616-3626；DFM 912 :6035-6037 Interval 100）：
 *        頁面這一段只有 :3671  rgFT_ART->Visible = !(ckUseLeastRetestTimes->Checked)。
 *        golden 是 100 ms 的計時器（InitialOK 之後一直跑）；這裡在引擎開頁套完值之後、以及 ckUseLeastRetestTimes 每次改值時跑，
 *        畫面結果相同。:3672-3675 是 fLotInfo 的元件（Lot Info 頁）而且只在 CC_KYEC_LEE（客戶專屬，Steven 20260925 決定先跳過）→ 不做。
 *        伺服器端 rgFT_ART 的可見是開頁時的狀態（C 路沒有跑 Timer1），存檔照規則收值；頁面藏起來時使用者改不到，跟 golden 一樣。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_YieldMonitoring';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || R.__yieldQ41Wrapped) return;
  R.__yieldQ41Wrapped = true;

  function $(id) { return document.getElementById(id); }
  function checked(id) {                                                 // TCheckBox->Checked（label.ckb > input）
    var el = $(id), c = el ? (el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]')) : null;
    return !!(c && c.checked);
  }
  function timer1Display() {                                             // golden 912 :3671
    var rg = $('rgFT_ART');
    if (!rg) return;
    var on = !checked('ckUseLeastRetestTimes');
    rg.style.visibility = on ? '' : 'hidden';                            // 引擎的 Visible 也是用 visibility
    if (on && rg.style.display === 'none') rg.style.display = '';        // 產生器把 DFM Visible=False 寫成 display:none
  }
  function hook() {
    var ck = $('ckUseLeastRetestTimes');
    if (ck && !ck.__q41) { ck.__q41 = true; ck.addEventListener('change', timer1Display); }
  }

  var get0 = R.editlistGet;
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      setTimeout(function () { hook(); timer1Display(); }, 0);          // 引擎在這個 promise 的 then 裡同步套完值
      return d;
    });
  };

  window.HT9045YieldQ41 = { timer1Display: timer1Display };             // 探針／除錯用
})();
