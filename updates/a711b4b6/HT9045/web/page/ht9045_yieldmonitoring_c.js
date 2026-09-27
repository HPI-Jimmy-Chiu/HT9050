/* ht9045_yieldmonitoring_c.js -- Setup.YieldMonitoring.html（golden TfYieldMonitoring，V912 uYieldMonitoring.cpp）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-S158) 20260927 [W906] Steven 團隊 St01（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 * 依據：Steven 20260927 Q41（RULINGS_20260926 S158）；盤點 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 3.8 節 YM-1。
 *
 * 問題（看程式推的，這台不能開 wb_serve，沒有用瀏覽器實測）：
 *   golden（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uYieldMonitoring.cpp）FormShow :2640 ChangeData(fYieldMonitoring)
 *   （:2552-2598）把全頁 TCheckBox／TEdit／TLabeledEdit／TComboBox／TRadioButton 的 OnClick／OnChange 都接到 edContactCountFTChange
 *   （:3226，最後一行 btnApply->Enabled=true）；9 個 TRadioGroup 的 DFM OnClick 也全部接到會打開 Apply 的處理器（rgPiggyBack_FTClick
 *   :3197 或 edContactCountFTChange）；接著 :2642 btnApply->Enabled=false。所以 golden 是「開頁先停用，改了任何一格才打開」，
 *   存完 btnApplyClick :3173 再停用。
 *   移植樹 C 路 D:\HT9045\HT9011UC_Cpp_V3.33.906.0\FileRW\TestIF_File_YieldMonitoring.gen.inc:1283-1286 把 ChangeData 閘掉（註解寫
 *   「頁面自己做」）、:1288 照翻停用 → editlist.get 回應 proxies.btnApply.editable=false → 引擎 gbLoad 用 gbSetEnabled 把按鈕
 *   disabled（D:\HT9045\web\page\ht9045_wire_engine.js gbSetEnabled／gbLoad，Jimmy 的檔，不改）。頁面上沒有任何程式再打開它，
 *   停用的 <button> 瀏覽器不送 click → 引擎在 document 捕獲階段的存檔攔截也收不到 → Save 永遠按不下去、整頁存不了。
 *   （C++ 存檔本身不看 btnApply 的 Enabled：FileRW/_EditPage.cpp PageSave、YM_btnApplyClick 都沒有檢查。）
 *
 * 做法（照 golden 語意補「頁面自己做」的那一半，不改引擎、不改 C++）：
 *   document 捕獲階段聽 change／input。事件來源是 golden ChangeData 會接的 5 種元件或 TRadioGroup（看元素自己或最近祖先的
 *   title／data-htitle "名稱 : 型別"；release 模式 theme.js 會把 title 搬到 data-htitle）→ 打開 btnApply。
 *   golden DFM 核對（scratchpad 腳本重播 ChangeData 的遞迴規則）：235 個值元件全部在會遞迴的容器底下、9 個 TRadioGroup 全部
 *   接到打開 Apply 的處理器 ⇒ 等於「頁面上任何一個值元件被使用者改了」。
 *   - 小鍵盤：引擎 attachKeyboards 的 onCommit 會補發 input／change，所以小鍵盤改值也算（同 golden 的 OnChange）。
 *   - 引擎套值（開頁／存完重讀）是程式直接改 value／checked，不發事件 → 不會誤開（golden 也是 ChangeData 之後最後一行才停用）。
 *   - 只打開「引擎照 C++ 停用」的那一種（data-gb-dis="1"）；上層容器 C++ 說不可改／看不見（例權限）就不開（golden 設
 *     btnApply->Enabled=true 但父層停用時一樣按不到）。
 *   - 存完引擎一定重讀（gbLoad → golden FormShow → :2642 又停用）＝golden btnApplyClick :3173 存完停用。
 *   - 按鈕樣式：dfm2web 把 DFM 設計期 Enabled=False 畫成固定 opacity:.45；這裡跟著 disabled 切換，免得打開了看起來還是灰的。
 * 不做（照 golden 註記）：
 *   - edContactCountFTChange 裡 SIGURD 的兩欄同步（:3228-3241，IniConfig.bSIGURDFunction）：客戶專屬，Steven 20260925 決定先跳過。
 *   - MyYieldPanel 的 TTMyTray 格子（mtBinSelectYieldMouseDown :3425、mtTrayNameMouseDown :3448 也會打開 Apply）：C 路沒有移植
 *     那些格子（TestIF_File_YieldMonitoring.gen.inc:1274-1278 GATE「格子沒有移植」），頁面點了也不會改存檔值。
 * 載入位置：Setup.YieldMonitoring.html 兩份接線檔之後（HT9045Page 已由 register 建好）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_YieldMonitoring';
  var BTN = 'btnApply';
  // golden ChangeData（:2570-2598）接的 5 種 ＋ TRadioGroup（DFM OnClick 全部打開 Apply）
  var TYPES = { TCheckBox: 1, TEdit: 1, TLabeledEdit: 1, TComboBox: 1, TRadioButton: 1, TRadioGroup: 1 };

  function $(id) { return document.getElementById(id); }

  // 元素自己或最近祖先的 VCL 型別（dfm2web 的 title="名稱 : 型別"；release 模式在 data-htitle）
  function vclType(el) {
    for (var n = el; n && n.nodeType === 1; n = n.parentNode) {
      var t = n.getAttribute('data-htitle') || n.getAttribute('title') || '';
      var m = /^\s*\w+\s*:\s*(T\w+)/.exec(t);
      if (m) return m[1];
    }
    return '';
  }

  function goldenPage() {
    var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null;
    return g && g.struct === STRUCT ? g.page : null;          // 還沒讀取（C 路 editlist.get）→ null
  }

  // C++ 說某一層上層容器不可改／看不見 → golden 就算 Enabled=true 也按不到
  function parentBlocked(b, page) {
    var px = (page && page.proxies) || {};
    for (var n = b.parentNode; n && n.nodeType === 1; n = n.parentNode) {
      var p = n.id ? px[n.id] : null;
      if (p && (p.editable === false || p.visible === false)) return true;
    }
    return false;
  }

  function openApply() {                                      // golden edContactCountFTChange :3243 btnApply->Enabled=true
    var b = $(BTN), page = goldenPage();
    if (!b || !b.disabled || !page) return;
    if (b.getAttribute('data-gb-dis') !== '1') return;        // 不是引擎照 C++ 停用的，不碰
    if (parentBlocked(b, page)) return;
    b.disabled = false;
    b.removeAttribute('data-gb-dis');                         // 下次 gbLoad 引擎會照 C++ 重新停用並標記
    b.removeAttribute('aria-disabled');
  }

  function onEdit(ev) {
    var el = ev.target;
    if (!el || !el.tagName) return;
    var tn = el.tagName;
    if (tn !== 'INPUT' && tn !== 'SELECT' && tn !== 'TEXTAREA') return;
    if (el.disabled) return;
    if (!TYPES[vclType(el)]) return;                          // 小鍵盤本身、狀態列等沒有 VCL 型別 → 不算
    openApply();
  }
  document.addEventListener('change', onEdit, true);
  document.addEventListener('input', onEdit, true);

  // 按鈕樣式跟著 disabled（dfm2web 的固定 opacity:.45 是 DFM 設計期 Enabled=False 畫的）
  function syncLook() {
    var b = $(BTN);
    if (b) b.style.opacity = b.disabled ? '.45' : '';
  }
  function watch() {
    var b = $(BTN);
    if (!b || b.__ymC) return;
    b.__ymC = true;
    if (window.MutationObserver) new MutationObserver(syncLook).observe(b, { attributes: true, attributeFilter: ['disabled'] });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', watch);
  else watch();
})();
