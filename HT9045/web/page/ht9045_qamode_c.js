/* ht9045_qamode_c.js -- Setup.QAMode.html（golden TfQAMode，QAMode.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_QAMode.cpp（WS editlist.get／editlist.save tag=TestIF_File_QAMode）。
 * 取代 B 路兩份接線檔（ht9045_wire_qamode.js／ht9045_wire_setupqamode.js：只接 edtQASampleCnt、cbbQASampleTray 兩鍵，
 * 直接讀寫 Tester.Data）——同一個檔只能有一個寫者，[QA Mode]／[QA Sampling] 12 鍵現在由 golden btnApplyClick（:73）寫。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Setup.QAMode.html' → 'TestIF_File_QAMode'，整合者登錄）照通用規則讀寫元件；
 * 本檔只補三件事：
 *
 *  (1) 下拉選項＝golden FormShow（:115-124）執行期重建的那份（editlist.get 回應 extra.items）：
 *        cbQAModeBin     0..iTestBinCount-1（DFM 的 0..16 不是執行期選項）
 *        cbbQASampleTray Prod.iTrayType[i]==tTrayAuto 的 s6TrayName[i]（i<iFixRight）
 *      在引擎套值「之前」重建，引擎再照後端 itemIndex／text 選（清單外的 Tray Name 由引擎補成 cpp-text 選項，VCL csDropDown）。
 *  (2) 存檔鈕 btnApply → 引擎 HT9045Page.save()（golden btnApplyClick 沒有 YES/NO 確認框 → 不登錄 GB_SAVE_Q，頁面仍確認一次，引擎慣例）。
 *      B 路接線檔拿掉之後引擎不會自己攔存檔鈕（它只在有 B 路欄位時裝攔截），所以由這裡綁。
 *      btnOk（Exit）照頁面內建 .exitbtn 關視窗 —— golden btnOkClick → Close() → FormClose（fShow=false＋DoIniDataToForm），不寫檔。
 *  (3) 小鍵盤：golden edQAModeMouseDown（:35）N_INTEGER (10000, 5)、edtQASampleCntMouseDown（:283）N_INTEGER (10000, 0)，原樣
 *      （golden 給的 min>max，qwerty.js 取 min/max 較小／較大那個）。
 *
 * 客戶專屬（Steven 20260925 決定先跳過，只註記）：gbLoaderDirection／Image1Click（CosFunction.bQAmodeSupplyTrayDir：AMKOR_China／
 * RF360／QUALCOMM）——後端 golden ShowTrayDirectIMG 對其他客戶把 gbLoaderDirection 設看不見；Image1 沒有替身，頁面點了不會改
 * iQATrayDirect。tsQASampling 只有 CC_AMKOR_Korea＋bSCKART_EnableART 才顯示（後端 golden FormShow :165 決定）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_QAMode';
  var PAGE = 'Setup.QAMode.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__qaModeCWrapped) return;
  R.__qaModeCWrapped = true;

  var get0 = R.editlistGet;

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[QAMode/C] ' + msg);
  }

  /* ---- (1) golden FormShow 重建的下拉選項 ------------------------------------------------ */
  function rebuildItems(d) {
    var items = d && d.extra && d.extra.items;
    if (!items) { setTimeout(function () { say2('⚠ 後端回應沒有 extra.items（golden FormShow 的下拉選項），沿用網頁自己的選項'); }, 0); return; }
    Object.keys(items).forEach(function (id) {
      var el = $(id);
      if (!el || el.tagName !== 'SELECT') return;
      el.innerHTML = '';
      items[id].forEach(function (t, k) {
        var o = document.createElement('option');
        o.value = String(k);
        o.textContent = t;
        o.setAttribute('data-src', 'cpp-items');
        el.appendChild(o);
      });
      el.selectedIndex = -1;                 // 值等一下由引擎照後端 itemIndex／text 套
    });
  }

  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      rebuildItems(d);                       // 引擎在這個 promise 的 then 裡才套值
      return d;
    });
  };

  /* ---- (2) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('btnApply');
    if (!b || b.__qaC) return;
    b.__qaC = true;
    b.title = 'btnApply : golden btnApplyClick（QAMode.cpp:73）→ DoFormToData → Tester.Data [QA Mode]／[QA Sampling] 12 鍵 → ReadFile → fBinSel->ReadFile';
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

  /* ---- (3) 引擎註冊（取代 B 路兩份接線檔的 register；只留小鍵盤） --------------------------- */
  var kb = {
    edQAMode:       ['INTEGER', 0, true, 10000, 5],     // golden QAMode.cpp:35
    edtQASampleCnt: ['INTEGER', 0, true, 10000, 0]      // golden QAMode.cpp:283
  };
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'qamode_c', fields: {}, optional: {}, kb: kb });
  }
})();
