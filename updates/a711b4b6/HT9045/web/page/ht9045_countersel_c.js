/* ht9045_countersel_c.js -- Status.CounterSel.html（golden TfCounterSel，cCounterSel.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * AI(W906-CRT-CounterSel) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/IniConfig_CounterSel.cpp（WS editlist.get／editlist.save tag=IniConfig_CounterSel）。
 *   開頁＝golden FormShow（:29）：ProcessLastSetIni_Visible(bReadFile) 讀 config.ini [Visible] 11 鍵 → 勾選框／單選鈕／rgTestCategory。
 *   存檔＝golden FormClose（:56）：畫面 → IniConfig.bShow* → ProcessLastSetIni_Visible(bWriteFile) 寫回 config.ini [Visible]。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Status.CounterSel.html' → 'IniConfig_CounterSel'）照通用規則讀寫元件；本檔補三件事：
 *
 *  (1) Exit 鈕 spbExit ＝ 存檔。golden 沒有存檔鈕：spbExitClick（:145）→ Close() → OnClose=FormClose 才寫檔（golden BorderIcons=[]，
 *      只能從 Exit 關）。這裡攔下頁面內建的 .exitbtn（直接關視窗、不存檔），改成走引擎 HT9045Page.save()（golden FormClose 沒有
 *      YES/NO → 不登錄 GB_SAVE_Q；頁面照引擎慣例確認一次），**寫入成功才關視窗**；取消或失敗就留在頁面上看訊息。
 *      還沒接上 C 路（引擎 GOLDEN_BRIDGE 沒有這一頁，或開頁讀取失敗）時照舊直接關，不存檔。
 *      ⚠ 偏離：網頁視窗框的 ✕ 不經過這裡 —— 關了不存（golden 沒有 ✕）。
 *  (2) 單選鈕分組：VCL TRadioButton 以 Parent 互斥（gbLoadingCount 等六個 GroupBox 各一對 On／Off）；頁面的 <input type=radio>
 *      沒有 name，點 ON 不會把 OFF 清掉。這裡照 DFM 的 Parent 補 name（同 ht9045_startcondition_c.js gbStartMode 的做法）。
 *  (3) cbDefaultValue（Default Position Value）：golden FormClose :87-104 勾選時把 config\FormPos.def 的視窗位置還原成預設值，
 *      再把勾選清掉。網頁視窗位置不用 FormPos.def（background.html 自己的版面表）→ 後端擋掉寫檔那一段、照 golden 清掉勾選，
 *      ack 的 todo 會列出這一段。頁面上保持可勾（後端 editable=true，不另外停用），只在 title 說明。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'IniConfig_CounterSel';
  var PAGE = 'Status.CounterSel.html';
  // golden cCounterSel.dfm：TRadioButton 的 Parent（VCL 同一個 Parent 的單選鈕互斥）
  var RADIO_GROUPS = {
    gbLoadingCount: ['rbLoadingCount_On', 'rbLoadingCount_Off'],
    gbContactCount: ['rbContactCount_On', 'rbContactCount_Off'],
    gbTestCategory: ['rbTestCategory_On', 'rbTestCategory_Off'],
    gbScanner:      ['rbScanner_On', 'rbScanner_Off'],
    gbTemperature:  ['rbTemperature_On', 'rbTemperature_Off'],
    gbBinAssign:    ['rbBinAssign_On', 'rbBinAssign_Off']
  };

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[CounterSel/C] ' + msg);
  }
  function closeWin() {
    if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*');   // 同頁面內建 .exitbtn
  }
  function cRoute() {
    return !!(window.HT9045Page && HT9045Page.save && HT9045Page.golden && HT9045Page.golden().struct === STRUCT);
  }

  /* ---- (2) 單選鈕分組 ---------------------------------------------------------------------- */
  function groupRadios() {
    Object.keys(RADIO_GROUPS).forEach(function (g) {
      RADIO_GROUPS[g].forEach(function (id) {
        var el = $(id), rb = el && el.querySelector('input[type="radio"]');
        if (rb) rb.name = 'cs_' + g;
      });
    });
  }

  /* ---- (1) Exit ＝ golden FormClose ------------------------------------------------------ */
  var busy = false;
  function exitClick() {
    if (busy) return;
    if (!cRoute()) {
      console.info('[CounterSel/C] 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '）：Exit 照舊直接關，不存檔');
      closeWin();
      return;
    }
    if (!HT9045Page.golden().page) {                 // 開頁讀取沒成功：沒有 golden FormShow 的值可存，照舊關
      console.info('[CounterSel/C] 還沒讀到 golden FormShow 的值：Exit 直接關，不存檔');
      closeWin();
      return;
    }
    var before = HT9045Page.golden().lastSave;
    busy = true;
    Promise.resolve(HT9045Page.save()).then(function () {
      busy = false;
      var a = HT9045Page.golden().lastSave;
      if (a && a !== before && a.saved) closeWin(); // golden：FormClose 寫完就關
      else if (a && a !== before && !a.saved) say2('視窗不關：這次沒有寫入（見上方訊息）', '#f88');
    }, function (e) {
      busy = false;
      say2('❌ 存檔失敗，視窗不關：' + (e && e.message ? e.message : e), '#f88');
    });
  }
  // 捕獲階段掛在 window：比頁面內建 .exitbtn 的 click（掛在按鈕上、直接 postMessage closeMe）先跑，並擋住它
  window.addEventListener('click', function (ev) {
    var t = ev.target && ev.target.closest ? ev.target.closest('#spbExit') : null;
    if (!t) return;
    ev.stopPropagation();
    ev.preventDefault();
    exitClick();
  }, true);

  function decorate() {
    groupRadios();
    var b = $('spbExit');
    if (b && !b.__csC) {
      b.__csC = true;
      b.title = 'spbExit : golden spbExitClick（cCounterSel.cpp:145）→ Close() → FormClose（:56）：' +
                '寫 config.ini [Visible]（ProcessLastSetIni_Visible）；寫入成功才關視窗';
    }
    var d = $('cbDefaultValue');
    if (d && !d.__csC) {
      d.__csC = true;
      d.title = 'cbDefaultValue : golden FormClose（:87-104）勾選時把 config\\FormPos.def 的視窗位置還原成預設值。' +
                '網頁視窗位置不用 FormPos.def → 這一段不寫檔（勾選照 golden 在存檔時清掉，存檔結果會列出這一段）';
    }
  }

  /* ---- 引擎註冊（沒有 B 路欄位、沒有文字輸入框） ---------------------------------------------- */
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'countersel_c', fields: {}, optional: {}, kb: {} });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', decorate);
  else decorate();
})();
