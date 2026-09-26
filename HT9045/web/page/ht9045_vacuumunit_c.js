/* ht9045_vacuumunit_c.js -- HW.VacuumUnit.html（golden TfVacuumUnit，VacuumUnit\VacuumUnit.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/TestIF_File_VacuumUnit.cpp（WS editlist.get／editlist.save tag=TestIF_File_VacuumUnit）。
 * golden elVacuumUnit（HTEditList）→ <recipe>\HandlerCondition.Data [Vacuum Threshold] IndexArm1_<row>_<col>…／[Tray] CheckBox1、Edit1。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'HW.VacuumUnit.html' → 'TestIF_File_VacuumUnit'，整合者登錄）照通用規則讀寫：
 *   清單筆 id＝面板 edSV（hwidgets.js makeVacuumPanel：myPal<Arm>_<iCol>_<iRow>_edSV）、CheckBox1、Edit1；
 *   面板 myPal<Arm>_<iCol>_<iRow> 的 visible＝golden SetPanelPos／ShowSuckMode（Index 面板只顯示 FTestSuck.iShtRow×iShtCol 那幾顆），
 *   看不見的面板值不收（後端丟掉，沿用檔案值）。
 * 本檔補四件事：
 *
 *  (1) 面板數＝golden Initial（:38-49）：HT9045 Index 4 欄；HT9046／HT9046LS／HT1032／USE_46_SUCKER_DB=1 是 8 欄。頁面是照 4 欄產生的
 *      → editlist.get 回應裡有、頁面沒有的 myPal*_<c>_<r>_edSV，在引擎套值之前照 makeVacuumPanel 補建（caption 照 golden
 *      MyVacuumPanel.cpp:20-24 sIndexName_16／sInOutName_8），小鍵盤照 golden edSVClick。
 *  (2) 存檔鈕 spbSave → 引擎 HT9045Page.save()（golden spbSaveClick 沒有 YES/NO → 不登錄 GB_SAVE_Q，頁面仍確認一次，引擎慣例）。
 *      這一頁沒有 B 路欄位，引擎不會自己攔存檔鈕 → 由這裡綁。sbtExit 照頁面內建 .exitbtn 關視窗（golden sbtExitClick → Close →
 *      FormClose：ReadFile＋DoIniDataToForm，不寫檔）。
 *  (3) 硬體指令鈕停用（網頁 → 機台的指令通道還沒設計；互鎖在 C++ 端，瀏覽器不可直接驅動硬體）：
 *        每顆面板的 Set（btnSV → MyVacuumPanel.cpp:385 WriteVaccumThreshold → MyLaneIO.SetIOValueThread 寫 ECAT-VC8 閥值）
 *        ^／v（bplOn／bplOff → MyVacuumPanel.cpp:391 Acm_DaqDoSetBitEx 吸／破真空 DO）
 *        Set All Value 三顆（btnSetInArmClick VacuumUnit.cpp:551：改全部 edSV＋逐顆寫 VC8 閥值）與它的三個輸入框
 *        Reset（sbResetClick :409：清面板事件、下一個 tick 重寫 VC8 閥值模式 SDO）
 *      ⚠ 這代表：頁面 Save 只把 edSV 存進 HandlerCondition.Data（golden 也是只存檔）；VC8 硬體閥值要在 golden BCB 按 Set 才會生效。
 *  (4) 小鍵盤：golden edSVClick（MyVacuumPanel.cpp:382）N_DOUBLE (-116.0, 148.0)；Edit1 golden 沒有小鍵盤事件 → 引擎的通用鍵盤
 *      （範圍取清單筆 0~300）。
 *
 * 現值／閥值／Event／myld1（tmr1Timer :255 讀 VC8）是即時硬體顯示，不在讀寫檔範圍：頁面保留 DFM 的 0.0／Event。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'TestIF_File_VacuumUnit';
  var PAGE = 'HW.VacuumUnit.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__vacuumCWrapped) return;
  R.__vacuumCWrapped = true;

  var get0 = R.editlistGet;
  var ARMS = { myPalArm1: 'scrlbxIndexArm1', myPalArm2: 'scrlbxIndexArm2', myPalInArm: 'scrlbxInArm', myPalOutArm: 'scrlbxOutArm' };
  var KB_SV = ['DOUBLE', 0, true, -116.0, 148.0];          // golden MyVacuumPanel.cpp:382 edSVClick
  var HW_NOTE = '網頁停用：golden 這顆鈕直接寫 ECAT-VC8 硬體（網頁 → 機台的指令通道還沒設計）。存檔只寫 HandlerCondition.Data。';

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[VacuumUnit/C] ' + msg);
  }

  /* ---- (3) 硬體指令鈕停用 ------------------------------------------------------------------ */
  function lockButton(el, why) {
    if (!el || el.__vuLocked) return el;
    if (el.tagName !== 'BUTTON' && el.tagName !== 'INPUT') {   // TBtnPanelLane（div）：換成不帶監聽器的複本
      var n = el.cloneNode(true);
      el.parentNode.replaceChild(n, el);
      el = n;
      el.style.cursor = 'not-allowed';
      el.style.opacity = '0.55';
      el.setAttribute('aria-disabled', 'true');
    } else {
      el.disabled = true;
    }
    el.__vuLocked = true;
    el.title = (el.title ? el.title + '\n' : '') + why;
    return el;
  }
  function lockPanel(gb) {
    if (!gb || gb.__vuPanel) return;
    gb.__vuPanel = true;
    Array.prototype.forEach.call(gb.querySelectorAll('button'), function (b) {
      lockButton(b, 'btnSV Set → MyVacuumPanel.cpp:385 WriteVaccumThreshold（VC8 閥值）。' + HW_NOTE);
    });
    ['_bplOn', '_bplOff'].forEach(function (s) {
      lockButton($(gb.id + s), (s === '_bplOn' ? 'bplOn 吸真空' : 'bplOff 破真空') + ' → MyVacuumPanel.cpp:391 Acm_DaqDoSetBitEx（DO）。' + HW_NOTE);
    });
  }
  function lockStatic() {
    ['btnSetInArm', 'btnSetIndexArm', 'btnSetOutArm'].forEach(function (id) {
      lockButton($(id), 'btnSetInArmClick（VacuumUnit.cpp:551）：改全部 edSV＋逐顆寫 VC8 閥值。' + HW_NOTE);
    });
    ['edSetInArm', 'edSetIndexArm', 'edSetOutArm'].forEach(function (id) {
      lockButton($(id), 'Set All Value 的輸入（只給上面的 Set 用）。' + HW_NOTE);
    });
    lockButton($('sbReset'), 'sbResetClick（VacuumUnit.cpp:409）：清面板事件，下一個 tick 重寫 VC8 閥值模式（SDO）。' + HW_NOTE);
    Array.prototype.forEach.call(document.querySelectorAll('.vacpanel'), lockPanel);
  }

  /* ---- (1) golden Initial 的面板數 --------------------------------------------------------- */
  function caption(arr, col, row) {                  // golden MyVacuumPanel.cpp:20-24
    if (arr === 'myPalArm1' || arr === 'myPalArm2') return '    ' + 'AB'.charAt(row) + 'abcdefgh'.charAt(col) + '    ';
    return '     ' + 'ACEGBDFH'.charAt(row * 4 + col) + '    ';
  }
  function kbAttach(el) {
    if (!window.HTQwerty || el.__vuKb) return;
    el.__vuKb = true;
    el.setAttribute('readonly', 'readonly');
    el.style.cursor = 'pointer';
    el.addEventListener('mousedown', function (ev) {
      ev.preventDefault();
      if (el.disabled) return;
      HTQwerty.show(el, HTQwerty.N[KB_SV[0]] || 0, { dp: KB_SV[1], checkRange: KB_SV[2], min: KB_SV[3], max: KB_SV[4] });
    });
  }
  function buildMissing(d) {
    var made = [];
    var W = window.HTWidgets;
    var ents = ((d && d.lists && d.lists.elVacuumUnit) || {}).entries || [];
    ents.forEach(function (e) {
      var m = /^(myPal(?:Arm1|Arm2|InArm|OutArm))_(\d+)_(\d+)_edSV$/.exec(e.id || '');
      if (!m || $(e.id)) return;
      var arr = m[1], col = +m[2], row = +m[3];
      var box = $(ARMS[arr]), host = box && box.querySelector('.htWidgetHost');
      if (!host || !W || !W.makeVacuumPanel) return;
      var gb = W.makeVacuumPanel({ name: arr + '_' + col + '_' + row, caption: caption(arr, col, row), cur: '0.0',
                                   event: 'Event', threshold: '0.0', sv: '0', left: col * 81, top: 10 + row * 177 });
      host.appendChild(gb);
      host.style.width = Math.max(parseInt(host.style.width, 10) || 0, (col + 1) * 81) + 'px';
      lockPanel(gb);
      if ($(e.id)) kbAttach($(e.id));
      made.push(arr + '_' + col + '_' + row);
    });
    if (made.length) setTimeout(function () {
      say2('ⓘ 後端 golden Initial 建了 ' + made.length + ' 個頁面沒有的面板（Index 8 欄機種），已補上：' + made.join(', '));
    }, 0);
  }

  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      buildMissing(d);                       // 引擎在這個 promise 的 then 裡才套值：元件要先在
      return d;
    });
  };

  /* ---- (2) 存檔鈕 ----------------------------------------------------------------------- */
  function usable(b) { return b && !b.disabled && b.getAttribute('aria-disabled') !== 'true'; }
  function bindSave() {
    var b = $('spbSave');
    if (!b || b.__vuC) return;
    b.__vuC = true;
    b.title = 'spbSave : golden spbSaveClick（VacuumUnit.cpp:378）→ SaveSetupFile → elVacuumUnit->SaveEditTextToFile（HandlerCondition.Data）→ ReadFile';
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

  /* ---- 面板由 page-widgets.js 在 DOMContentLoaded 建立（它的監聽器先註冊）；本監聽器排在它之後、引擎 attach 之前 ---- */
  function onReady() { lockStatic(); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', onReady);
  else onReady();

  /* ---- (4) 引擎註冊（這一頁沒有 B 路接線檔；只給小鍵盤） -------------------------------------- */
  var kb = {};
  Object.keys(ARMS).forEach(function (arr) {
    for (var c = 0; c < 8; c++) for (var r = 0; r < 2; r++) kb[arr + '_' + c + '_' + r + '_edSV'] = KB_SV;
  });
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'vacuumunit_c', fields: {}, optional: {}, kb: kb });
  }
})();
