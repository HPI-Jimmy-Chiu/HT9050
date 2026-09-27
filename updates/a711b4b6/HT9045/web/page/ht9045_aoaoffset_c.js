/* ht9045_aoaoffset_c.js -- Main.AOAInfo.html（golden TfMain，main.cpp V912）AOA offset 那一塊的 C 路頁面補件。
 * ---------------------------------------------------------------------------
 * //AI(W906-FRW-AOA) 20260926: 新檔（Steven 團隊；手寫，不是 gen_wire.py 產物 —— 檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）。
 *
 * 後端：FileRW/AOAOffset.cpp（WS editlist.get／editlist.save tag=AOAOffset；設定 tools/editlist/AOAOffset.py）。
 *   開機＝golden TfMain::FormShow main.cpp:11679-11724：MACHINE_HAS_AUTO_ALIGNMENT_CCD && 配方 Enable Auto Alignment 時
 *        iAOA_* → 38 個 Ed_*Offset_X/Y；:11736 pnlAOAForHT9011（Auto4-6／Fix4-6）只在 AUTO_EMPTY_COLOR>=3 顯示。
 *   開頁＝golden 切到 AOA Info 頁籤：不跑程式（元件值是開機填的；換配方也不重填）。
 *   存檔＝golden OffsetSaveClick（main.cpp:34956）：38 個 Ed_* → iAOA_* → D:\HT9045\system\Gerneral.ini [System] AOA_* 26 鍵
 *        （AUTO_EMPTY_COLOR>=3 時 38 鍵）。golden 沒有權限檢查、沒有確認框 → 不登錄 GB_SAVE_Q（頁面仍確認一次，引擎慣例）。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Main.AOAInfo.html' → 'AOAOffset'，整合者加）照通用規則讀寫；本檔補：
 *
 *  (1) 存檔鈕 OffsetSave → 引擎 HT9045Page.save()（沒有 B 路欄位，引擎不會自己攔存檔鈕）。
 *  (2) golden main.dfm 38 個 Ed_*Offset_X/Y 全部 Visible=False，golden 全樹沒有打開它們 ⇒ 操作員看不到也改不到（替身 editable=false，
 *      頁面送的值伺服器一律 ignored，存的是開機填的值）。畫面照畫成 display:none；OffsetSave 的滑鼠提示列出「存檔會寫的值」。
 *  (3) golden 的地雷照 golden 不擋，但在確認前講出來（extra.wouldChange／gate.bootFilled）：開機時 AOA 沒開 ⇒ 元件是 DFM 的 "0"
 *      ⇒ 按 Save 會把 AOA_* 寫成 0；開機後有人在檔案外改過 AOA_* ⇒ 按 Save 會寫回開機值。
 *  (4) 執行期顯示沒有接：labXxx_X/Y、Memo3_AOA_IN／OUT（golden AutoAlignment CCD 對位流程寫，:3425／:4880／:5325；移植樹沒有，Jimmy）。
 *  小鍵盤：golden Ed_LoaderOffset_XClick（main.cpp:35051）ShowQwertyKey(N_INTEGER, 0, true, InputLimit.iOffsetXYHigh*100, …Low*100)，
 *      但元件看不見 → 不掛（kb 空表，沒有載入 qwerty.js）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'AOAOffset';
  var PAGE = 'Main.AOAInfo.html';
  var LAST = null;                                   // 最近一次 editlist.get 的 extra

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[AOAOffset/C] ' + msg);
  }
  function setTitle(el, base, text) {
    if (!el) return;
    el.title = base + (text ? '\n' + text : '');
  }

  var SAVE_BASE = 'OffsetSave : golden TfMain::OffsetSaveClick（main.cpp:34956）→ Ed_*Offset_X/Y → iAOA_* → ' +
                  'D:\\HT9045\\system\\Gerneral.ini [System] AOA_*（沒有權限檢查、沒有確認框、寫完不重讀）';
  var RT_NOTE = '執行期值未接：golden 由 AutoAlignment CCD 對位流程寫入（AutoAlignment.cpp:3425／:4880／:5325），移植樹沒有（Jimmy）。';

  function markRuntime() {
    ['Memo3_AOA_IN', 'Memo3_AOA_OUT'].forEach(function (id) { var el = $(id); if (el && el.title.indexOf(RT_NOTE) < 0) el.title += '\n' + RT_NOTE; });
    var t = $('aoaPosTbl');
    if (t && t.title.indexOf(RT_NOTE) < 0) t.title = (t.title ? t.title + '\n' : '') + RT_NOTE;
  }

  function keyLine(k) {
    return k.key + '=' + k.editValue + (k.file === null ? '（檔案沒有這鍵）' : (k.file !== k.editValue ? '（檔案現在 ' + k.file + '）' : ''));
  }

  /* ---- (2)(3) 替身狀態與警告 ------------------------------------------------------------------ */
  function applyExtra(d) {
    if (!d) return;
    var px = d.proxies || {};
    Object.keys(px).forEach(function (id) {
      var el = $(id), v = px[id];
      if (!el) return;
      // DFM Visible=False 的元件頁面畫成 display:none；引擎只切 visibility → C++ 說看得見時把 display 還原（golden 不會發生，照通用規則）
      if (v.visible === true && el.style.display === 'none') el.style.display = '';
    });
    var x = LAST = d.extra || {};
    var g = x.gate || {}, keys = x.keys || [];
    var writes = keys.filter(function (k) { return k.writes; });
    setTitle($('OffsetSave'), SAVE_BASE, '這次會寫 ' + writes.length + ' 鍵：\n  ' + writes.map(keyLine).join('\n  '));
    var lines = [];
    if (g.bHandlerModel === false) lines.push('❌ D:\\GPIB9045\\system\\general.ini 的 Model 讀取失敗：golden 會結束程式，這一頁不能存檔。');
    if (g.bootFilled === false) {
      lines.push('ⓘ 開機時 golden FormShow（main.cpp:11679）沒有把 iAOA_* 填進元件（MACHINE_HAS_AUTO_ALIGNMENT_CCD=' +
                 (g.MACHINE_HAS_AUTO_ALIGNMENT_CCD ? 1 : 0) + '、配方 Enable Auto Alignment=' + (g.bEnableAutoAlignment ? 1 : 0) +
                 '）→ 元件是 DFM 預設 "0"；按 Save 照 golden 會寫 0。');
    }
    if ((x.wouldChange || []).length) {
      lines.push('⚠ 按 Save 會改掉 Gerneral.ini 的 ' + x.wouldChange.length + ' 鍵：' +
                 keys.filter(function (k) { return x.wouldChange.indexOf(k.key) >= 0; }).map(keyLine).join('、'));
    }
    lines.push('ⓘ golden 的 38 個 AOA offset 輸入框是隱藏的（main.dfm Visible=False），畫面上改不到；Save 寫的是開機時的值（' +
               writes.length + ' 鍵，AUTO_EMPTY_COLOR=' + g.AUTO_EMPTY_COLOR + '）。');
    if (x.runtime && x.runtime.wired === false) lines.push('⚠ labXxx_X/Y 與 AOA IN/OUT 記錄是執行期顯示，尚未接（AutoAlignment CCD 流程）。');
    say2(lines.join('\n'), (g.bHandlerModel === false || (x.wouldChange || []).length) ? '#ffcc66' : undefined);
    markRuntime();
  }

  var R = window.HT9045Recipe;
  if (R && R.editlistGet && !R.__aoaOffsetCWrapped) {
    R.__aoaOffsetCWrapped = true;
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
    var b = $('OffsetSave');
    if (!b || b.__aoaC) return;
    b.__aoaC = true;
    setTitle(b, SAVE_BASE, '');
    b.addEventListener('click', function (ev) {
      ev.preventDefault(); ev.stopPropagation();
      if (!usable(b)) return;
      if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔', '#f88'); return; }
      if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
        say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88');
        return;
      }
      var x = LAST || {}, ch = x.wouldChange || [];
      if (ch.length) {
        var keys = (x.keys || []).filter(function (k) { return ch.indexOf(k.key) >= 0; });
        // golden 沒有這個確認框：網頁誤觸保護（寫的是量產共用檔），答「取消」就不送 editlist.save
        if (!window.confirm('⚠ golden OffsetSaveClick 會把 Gerneral.ini 這 ' + ch.length + ' 鍵改掉：\n  ' + keys.map(keyLine).join('\n  ') +
                            '\n\n（輸入框在 golden 畫面上是隱藏的，寫的是開機時填進去的值。）\n確定要繼續？')) {
          say2('已取消，未寫入。', '#ffcc66');
          return;
        }
      }
      HT9045Page.save();
    });
  }

  function onReady() { bindSave(); markRuntime(); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();

  /* ---- 引擎註冊（這一頁原本沒有接線檔）-------------------------------------------------------- */
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'aoaoffset_c', fields: {}, optional: {}, kb: {} });
  }
})();
