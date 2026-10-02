/* ht9045_config_exit_c.js -- Config.Configuration.html（golden TfConfiguration）的 Exit＝golden 關頁存檔。
 * ---------------------------------------------------------------------------
 * AI(W906-CFG-EXITSAVE) 20261002 新檔（手寫）。EastSun 1001「請檢查每個頁面元件」—— 這台（CC_PTI，沒有 CosFunction.bUseConfigSaveButton）
 *   golden 的 Save Config. 鈕是藏起來的（cConfiguration.cpp:5354-5361 btnSave->Visible=false，C++ 開頁照跑、網頁照藏），
 *   golden 唯一的存檔路是 Exit：sbExitClick（:6277）→ Close() → FormClose（:5710）→ ShowMyMessageBox_YES_NO("Config data save to define?",
 *   "確定要寫入資料？")，YES 才存（:5736 → CheckConfigurationBeforeSave/SaveConfiguration/LoadConfiguration :5765-5767），
 *   NO（ret=2）＝InitialDataToEdit 還原、不存（:5737-5747）；兩種答案表單都會關。
 *   以前網頁的 Exit 只關窗（.exitbtn），btnSave 又被藏 ⇒ 這台的 Configuration 頁「改了存不進去」。
 *   這裡照 golden：
 *     * btnSave 看得見（bUseConfigSaveButton 的機台）⇒ golden FormClose ret=2（:5731-5734）：Exit 不問、不存，照舊直接關。
 *     * 否則 ⇒ Exit 走引擎 HT9045Page.save()：引擎問的就是 golden 這一題（GB_SAVE_Q.IniConfig＝"Config data save to define?"），
 *       YES ⇒ editlist.save IniConfig（C++ FileRW_IniConfig_Save：sbExitClick 前三句＋golden FormClose），存好了才關窗；
 *       NO ⇒ 不存、直接關（golden ret!=1 也關；伺服器端的替身下次開頁 FormShow 會重讀）。
 *       存檔被拒（欄位值讀不出來、golden A02 Operator 權限訊息…）⇒ 不關，訊息留在狀態列讓操作員看到（golden 是先跳訊息再關）。
 *   這個題目是 golden 自己的確認框（不是網頁加的），所以照問。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'IniConfig';
  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(msg, colour || '#ffcc66');
    if (window.console) console.info('[Config/Exit] ' + msg);
  }
  function closeWin() { if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); }
  function btnSaveShown(g) {                             // golden btnSave->Visible（C++ FormShow :5354-5361，editlist.get proxies）
    var p = g && g.page && g.page.proxies && g.page.proxies.btnSave;
    return !!(p && p.visible === true);
  }
  var BUSY = false, ARMED = false;
  function onExit(ev) {
    var b = $('sbExit');
    if (!b || !ev.target.closest || ev.target.closest('#sbExit') !== b) return;
    var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null;
    if (!g || g.struct !== STRUCT || !g.page) return;    // 還沒開頁（editlist.get）＝照舊關窗（頁面內建 .exitbtn）
    if (btnSaveShown(g)) return;                         // golden :5731 bUseConfigSaveButton && bSave==false ⇒ ret=2，不存直接關
    ev.stopImmediatePropagation(); ev.preventDefault();
    if (BUSY) return;
    if (ARMED) { ARMED = false; closeWin(); return; }    // 上一次頁面在問題之前就拒存（golden 沒有這一步）：再按一次＝不存直接關（golden 表單一定關得掉）
    BUSY = true;
    var asked = null, conf0 = window.confirm;
    window.confirm = function (m) { var r = conf0.call(window, m); asked = r; return r; };   // 引擎 gbSave 問 golden 那一題，記下答案
    var before = g.lastSave;
    Promise.resolve(HT9045Page.save()).then(function () {
      window.confirm = conf0; BUSY = false;
      var after = HT9045Page.golden().lastSave;
      if (asked === false) { closeWin(); return; }                                  // golden NO：不存，表單照關
      if (asked === true && after && after !== before && after.saved) { closeWin(); return; }   // golden YES：存好了才關
      if (asked === true) say2('Exit：golden 存檔流程沒有寫入（原因見上面的訊息）—— 視窗先不關，讓你看到原因；要放棄修改請再按一次 Exit 並選「取消」', '#f88');
      else if (asked === null) { ARMED = true; say2('Exit：頁面在 golden 那一題之前就拒存（原因見上面的訊息）—— 再按一次 Exit＝不存直接關', '#f88'); }
    }, function () { window.confirm = conf0; BUSY = false; });
  }
  document.addEventListener('click', onExit, true);      // capture：早於頁面內建 .exitbtn（直接關窗）與引擎的存檔攔截
})();
