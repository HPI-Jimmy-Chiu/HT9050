/* ht9045_startcondition_c.js -- Data.StartCondition.html（golden TfStartCondition，cStartCondition.cpp V912）C 路的頁面補件。
 * ---------------------------------------------------------------------------
 * Steven 團隊 20260925（手寫，不是 gen_wire.py 產物；檔名刻意不叫 ht9045_wire_<slug>.js，免得產生器覆蓋）
 *
 * 後端：FileRW/StartCondition.cpp（WS editlist.get／editlist.save tag=StartCondition）。
 * 取代 B 路那份 ht9045_wire_datastartcondition.js（只接 5 個欄位直接讀寫 HandlerCondition.Data）——同一個檔只能有一個寫者，
 * 那 5 個欄位（Kit No 1-3、HeadContactSet[4]、O_20InOutArmLifeCntSet）現在由 golden 存檔鈕寫。
 * 引擎（ht9045_wire_engine.js GOLDEN_BRIDGE 'Data.StartCondition.html' → 'StartCondition'，整合者登錄）照通用規則讀寫元件；
 * 本檔補通用規則做不到的：
 *
 *  (1) 三顆 golden 存檔鈕各跑各的處理器（引擎只有一顆存檔鈕）：
 *        sbHeadCondition1Save（Life Time 的 Save，:1091）／sbSave（Contact count 的 Save，:708）／spbExit（Exit，:751 → FormClose :619）
 *      按下去 → 記下是哪一顆 → HT9045Page.save()（引擎 gbSave：確認框、送值、存完重讀）；editlist.save 時注入
 *      widgets.W906_scButton。Exit 存成功後關視窗（golden Exit 關表單）。
 *  (2) golden pgLifeTime->ActivePage：存檔 ReadWriteStartCondition 依它決定寫 HeadContactSet[n]、套哪一頁 grid。
 *      開頁時照後端（extra.activePage）切到那一頁；使用者換頁時照 golden pgLifeTimeChange（:1199）把該頁的值放進
 *      editContactCountAlarm（extra.tabAlarm＝後端跑 golden pgLifeTimeChange 的結果）；存檔注入 widgets.W906_scActivePage。
 *  (3) 清除類按鈕（golden 按下去就改元件，有的也直接改 LastSet／IniConfig）：畫面照 golden 處理器做一次，並依序記下；
 *      存檔注入 widgets.W906_scActions，後端在套頁面值之前依序重放 golden 處理器（見 FileRW/StartCondition.cpp 檔頭）。
 *        sbSameAsHead1（:671）sbClearCount（:688）sbHeadCondition1Clear（:772，依當時分頁）btnClearAa..Dh（:1390）
 *        btnInArmA..H（:1306）btnOutArmA..H（:1313）btnArm1Aa..Bh（:1327）btnArm2Aa..Bh（:1320）
 *      picker 計數框（golden DFM Enabled=False）只能被清除鈕歸零 —— 後端丟掉頁面值，只收重放結果。
 *  (4) TStringGrid 格子編輯照 golden OnSelectCell（引擎的 grid 點擊是 Configuration 頁的 "On" 切換，不適用這頁）：
 *        sgContactCount（:741）：第 1 欄起、第 1 列起可改，小鍵盤 0~2000000
 *        sgHeadCondition1..3／sgSocketCount（:1143）：只有第 2 欄（Current Status）、且該列有名稱；AMD 先問是否歸零；0~999999
 *  (5) 畫面：rbStartMode1..3 同一組（VCL 同 Parent 互斥；HTML 沒 name）、tsCondition01..03 的 Caption（extra.tabCaption）、
 *      palVibrator*（golden TPanel Caption，後端以 text 送來）。
 *  (6) 未移植（停用並註明）：Cylinder 頁（strngrdCylinderView／彈出選單 → MachineLife.ini）、btnSetOffsetLimit（Security_new.def，
 *      只有 CosFunction.bSetOffsetLimitToAll 顯示）；sb_Maintenance_SmartDiagnosticFunction（開 fSmartDiagnostic）AI(W906-SC-SDIAG) 20261001 起開 smartdiag 視窗；
 *      cbStartModeOnlyFT（CC_SIGURD_HUKOU 才顯示）的 golden OnClick 不做。客戶專屬條件：Steven 20260925 決定先跳過。
 *
 * golden 存檔流程沒有 YES/NO 確認框 → 不登錄引擎 GB_SAVE_Q；AI(W906-NOASK) 20261001 起登錄 GB_NO_ASK：按下就存，頁面不再自己問。
 * 視窗標題列 ✕ 關閉：golden 會跑 FormClose（sbSaveClick＋iStartMode），網頁端沒有這個事件 → 不存（用 Exit 存）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var STRUCT = 'StartCondition';
  var PAGE = 'Data.StartCondition.html';
  var R = window.HT9045Recipe;
  if (!R || !R.editlistGet || !R.editlistSave || R.__startCondCWrapped) return;
  R.__startCondCWrapped = true;

  var ROWS = 'ABCD', COLS = 'abcdefgh', ARM8 = 'ABCDEFGH';
  var LIFE_TABS = ['tsCondition01', 'tsCondition02', 'tsCondition03', 'TabsSocketID', 'tsVibration',
                   'tsSmartDiagnostic', 'tsSocketCount', 'tsPickerLifeTime', 'tsCylinderView'];
  var get0 = R.editlistGet, save0 = R.editlistSave;
  var META = null;                 // editlist.get 回應的 extra（activePage／tabAlarm／tabCaption／amd）
  var CUR = '';                    // pgLifeTime 目前分頁（golden ActivePage）
  var ACTS = [];                   // 開頁後點過的清除類按鈕（依序）
  var BUTTON = '';                 // 這次存檔是哪一顆 golden 按鈕
  var silent = false;              // 程式切頁（golden FormShow 設 ActivePage 不觸發 OnChange）

  function $(id) { return document.getElementById(id); }
  function say2(msg, colour) {                       // 接在引擎的狀態列後面（不蓋掉讀取結果）
    var b = $('ht9045WireBar');
    var prev = b && b.firstChild && b.firstChild.nodeType === 3 ? b.firstChild.nodeValue : '';
    if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say((prev ? prev + '\n' : '') + msg, colour || (b && b.style.color) || '#ffcc66');
    console.info('[StartCondition/C] ' + msg);
  }
  function usable(el) {                              // golden 使用者按得到／改得到：沒停用、沒藏起來
    if (!el || el.disabled) return false;
    for (var p = el; p && p.nodeType === 1; p = p.parentNode) {
      if (p.getAttribute('aria-disabled') === 'true') return false;
      var cs = window.getComputedStyle(p);
      if (cs.visibility === 'hidden' || cs.display === 'none') return false;
    }
    return true;
  }
  function setVal(id, v) { var e = $(id); if (e && 'value' in e) e.value = v; }

  /* ---- grid 小工具（引擎 gbGrid 畫的 <table>，td/th 帶 data-c／data-r） ------------------------------ */
  function cell(grid, c, r) { var g = $(grid); return g ? g.querySelector('[data-c="' + c + '"][data-r="' + r + '"]') : null; }
  function cellText(grid, c, r) { var x = cell(grid, c, r); return x ? x.textContent : ''; }
  function setCell(grid, c, r, v) { var x = cell(grid, c, r); if (x) x.textContent = v; }
  function colCount(grid) { var g = $(grid), tr = g && g.querySelector('tr'); return tr ? tr.children.length : 0; }
  function rowCount(grid) { var g = $(grid); return g ? g.querySelectorAll('tr').length : 0; }
  function clearNamedCol2(grid) {                    // golden :799-803／:834-838：有名稱的列，第 2 欄歸零
    for (var r = 1; r < rowCount(grid); r++) if (cellText(grid, 0, r).trim() !== '') setCell(grid, 2, r, '0');
  }

  /* ---- (2) pgLifeTime 分頁 ------------------------------------------------------------------------- */
  function lifeTab(name) {
    var pc = $('pgLifeTime');
    return pc ? pc.querySelector(':scope > .pcTabs > .tab[title^="' + name + ' :"]') : null;
  }
  function tabNameOf(tab) { var t = (tab.getAttribute('title') || '').split(' :')[0]; return LIFE_TABS.indexOf(t) >= 0 ? t : ''; }
  function activate(name) {                          // golden FormShow 設 ActivePage（不觸發 OnChange）
    var tab = lifeTab(name);
    if (!tab || tab.style.display === 'none') return false;
    silent = true;
    try { tab.click(); } finally { silent = false; }
    CUR = name;
    return true;
  }
  function onLifeTabClick(ev) {                      // 使用者換頁 → golden pgLifeTimeChange（:1199）
    if (silent) return;
    var name = tabNameOf(ev.currentTarget);
    if (!name) return;
    CUR = name;
    var v = META && META.tabAlarm ? META.tabAlarm[name] : undefined;
    if (v !== undefined) setVal('editContactCountAlarm', v);
  }

  /* ---- (3) 清除類按鈕：畫面效果（golden 處理器的元件那一半）＋記下點擊 ------------------------------------ */
  function sameAsHead1() {                           // golden :671-686
    if (colCount('sgContactCount') >= 4) {           // bContactAlmNeedOneCycle：ColCount=4
      setCell('sgContactCount', 1, 2, cellText('sgContactCount', 1, 1));
      setCell('sgContactCount', 2, 2, cellText('sgContactCount', 2, 1));
    } else {
      setCell('sgContactCount', 1, 2, cellText('sgContactCount', 1, 1));
    }
  }
  function clearCount() {                            // golden :688-706
    var c = colCount('sgContactCount') >= 4 ? 3 : 2;
    for (var i = 0; i < 2; i++) setCell('sgContactCount', c, 1 + i, '0');
  }
  function clearPickers() {                          // golden ClearPickerCount(-1) :1290-1303
    for (var i = 0; i < 8; i++) { setVal('edtInArm' + ARM8[i], '0'); setVal('edtOutArm' + ARM8[i], '0'); }
    for (var r = 0; r < 2; r++) for (var c = 0; c < 8; c++) {
      setVal('edtArm1' + ROWS[r] + COLS[c], '0'); setVal('edtArm2' + ROWS[r] + COLS[c], '0');
    }
  }
  function headClear() {                             // golden :772-846（依 pgLifeTime->ActivePage）
    var t = CUR;
    if (t === 'tsCondition01' || t === 'tsCondition02' || t === 'tsCondition03') {
      clearNamedCol2('sgHeadCondition' + t.slice(-1));
    } else if (t === 'TabsSocketID') {
      for (var r = 0; r < 4; r++) for (var c = 0; c < 8; c++) {
        setVal('editSocket' + ROWS[r] + COLS[c], ''); setVal('Panel' + ROWS[r] + COLS[c], '0');
      }
    } else if (t === 'tsVibration') {
      ['palVibratorHP1', 'palVibratorSht1', 'palVibratorSht2', 'palVibratorUnloader'].forEach(function (id) { panelCap(id, '0'); });
    } else if (t === 'tsSocketCount') {
      clearNamedCol2('sgSocketCount');
    } else if (t === 'tsPickerLifeTime') {
      clearPickers();
    } else {
      // golden :776：其他頁（Smart Diagnostic／Cylinder）iTag 沿用按鈕的 Tag＝0 → 清 HeadCondition1（golden 行為照做）
      clearNamedCol2('sgHeadCondition1');
    }
    return 'sbHeadCondition1Clear@' + t;
  }
  function bindActions() {
    function bind(id, fn) {
      var b = $(id);
      if (!b || b.__scC) return;
      b.__scC = true;
      b.addEventListener('click', function (ev) {
        ev.preventDefault();
        if (!usable(b)) return;
        var a = fn();
        ACTS.push(a || id);
      });
    }
    bind('sbSameAsHead1', function () { sameAsHead1(); });
    bind('sbClearCount', function () { clearCount(); });
    bind('sbHeadCondition1Clear', headClear);
    for (var r = 0; r < 4; r++) for (var c = 0; c < 8; c++) (function (k) {
      bind('btnClear' + k, function () { setVal('Panel' + k, '0'); });          // golden :1390-1406（Tag 對到同位置）
    })(ROWS[r] + COLS[c]);
    for (var i = 0; i < 8; i++) (function (k) {
      bind('btnInArm' + k, function () { setVal('edtInArm' + k, '0'); });       // golden :1306 ClearPickerCount(0, Tag)
      bind('btnOutArm' + k, function () { setVal('edtOutArm' + k, '0'); });     // golden :1313 ClearPickerCount(1, Tag)
    })(ARM8[i]);
    for (var r2 = 0; r2 < 2; r2++) for (var c2 = 0; c2 < 8; c2++) (function (k) {
      bind('btnArm1' + k, function () { setVal('edtArm1' + k, '0'); });         // golden :1327 ClearPickerCount(2, Tag)
      bind('btnArm2' + k, function () { setVal('edtArm2' + k, '0'); });         // golden :1320 ClearPickerCount(3, Tag)
    })(ROWS[r2] + COLS[c2]);
  }

  /* ---- (1) 三顆 golden 存檔鈕 ------------------------------------------------------------------------ */
  function bindSaveButtons() {
    ['sbHeadCondition1Save', 'sbSave', 'spbExit'].forEach(function (id) {
      var b = $(id);
      if (!b || b.__scC) return;
      if (id === 'spbExit') {                        // 頁面內建的 .exitbtn 處理器會直接關視窗 → 換掉（cloneNode 不帶監聽器）
        var n = b.cloneNode(true);
        b.parentNode.replaceChild(n, b);
        b = n;
        b.title = 'spbExit : golden spbExitClick（:751）→ FormClose（:619）：存 Counter Clear／Start Mode／Contact count，存成功後關視窗';
      }
      b.__scC = true;
      b.addEventListener('click', function (ev) {
        ev.preventDefault(); ev.stopPropagation();
        if (!usable(b)) return;
        if (!window.HT9045Page || !HT9045Page.save) { say2('❌ 引擎還沒載入，不能存檔', '#f88'); return; }
        if (!(HT9045Page.golden && HT9045Page.golden().struct === STRUCT)) {
          say2('❌ 這一頁還沒走 C 路（引擎 GOLDEN_BRIDGE 沒有 ' + PAGE + '），不能存檔', '#f88');
          return;
        }
        BUTTON = id;
        HT9045Page.save();
      });
    });
  }

  /* ---- (4) grid 格子編輯 ----------------------------------------------------------------------------- */
  function kbEdit(td, max) {
    if (!window.HTQwerty) {
      var v = window.prompt('輸入整數（0~' + max + '）', td.textContent.trim());
      if (v === null) return;
      var n = parseInt(v, 10);
      if (isNaN(n)) return;
      td.textContent = String(Math.max(0, Math.min(max, n)));
      return;
    }
    HTQwerty.show(td, HTQwerty.N.INTEGER, { dp: 0, checkRange: true, min: 0, max: max });
  }
  // AI(W906-SC-ALMRANGE) 20261001: golden editContactCountAlarmMouseDown (906 cStartCondition.cpp:1049-1054)
  //   ShowQwertyKey(N_INTEGER, 0, true, InputLimit.iContactCntAlm, 0) -> range 0..InputLimit.iContactCntAlm (golden swaps min/max).
  //   The engine's keypad for this box had no range (kb below: the limit is a runtime value); C++ now sends it (extra.contactCntAlmMax,
  //   FileRW/StartCondition.cpp ExtraJson). Capture phase on the document, so it runs before the engine's own mousedown on the box.
  var almBound = false;
  function bindAlarmKeypad() {
    if (almBound) return;
    almBound = true;
    document.addEventListener('mousedown', function (ev) {
      var el = $('editContactCountAlarm');
      var max = META && typeof META.contactCntAlmMax === 'number' ? META.contactCntAlmMax : null;
      if (!el || ev.target !== el || max === null || !window.HTQwerty || !usable(el)) return;
      ev.preventDefault(); ev.stopPropagation();
      HTQwerty.show(el, HTQwerty.N.INTEGER, { dp: 0, checkRange: true, min: 0, max: max, onCommit: function () {
        ['input', 'change'].forEach(function (evn) { var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); } el.dispatchEvent(e2); });
      } });
    }, true);
  }
  function bindGrids() {
    bindAlarmKeypad();   // AI(W906-SC-ALMRANGE) 20261001
    var g1 = $('sgContactCount');
    if (g1) g1.onclick = function (ev) {             // golden sgContactCountSelectCell :741-749
      var td = ev.target.closest && ev.target.closest('td');
      if (!td || g1.getAttribute('aria-disabled') === 'true') return;
      var c = +td.getAttribute('data-c'), r = +td.getAttribute('data-r');
      if (c === 0 || r === 0) return;
      kbEdit(td, 2000000);
    };
    ['sgHeadCondition1', 'sgHeadCondition2', 'sgHeadCondition3', 'sgSocketCount'].forEach(function (id) {
      var g = $(id);
      if (!g) return;
      g.onclick = function (ev) {                    // golden sgHeadCondition1SelectCell :1143-1177
        var td = ev.target.closest && ev.target.closest('td');
        if (!td || g.getAttribute('aria-disabled') === 'true') return;
        var c = +td.getAttribute('data-c'), r = +td.getAttribute('data-r');
        if (r >= 1 && cellText(id, 0, r).trim() === '') return;       // :1151 沒有名稱的列＝site 未開啟
        if (c <= 1 || r === 0 || c > 2) return;                       // :1154 只有第 2 欄
        if (META && META.amd && c === 2 && r >= 1) {                  // :1157 AMD：先問是否歸零
          if (window.confirm('Config data reset to zero?\n確定資料要重置為零？')) { td.textContent = '0'; return; }
        }
        kbEdit(td, 999999);                                           // :1175
      };
    });
  }

  /* ---- (5)(6) 畫面 --------------------------------------------------------------------------------- */
  function panelCap(id, v) {
    var p = $(id);
    if (!p) return;
    var cap = p.querySelector('.pnlCap') || p;
    cap.textContent = v;
  }
  function decorate(d) {
    ['rbStartMode1', 'rbStartMode2', 'rbStartMode3'].forEach(function (id) {   // VCL：同 Parent（gbStartMode）的 radio 互斥
      var el = $(id), rb = el && el.querySelector('input[type="radio"]');
      if (rb) rb.name = 'gbStartMode';
    });
    var cap = (META && META.tabCaption) || {};
    Object.keys(cap).forEach(function (n) {                                       // golden FormShow :474-476
      var t = lifeTab(n);
      if (t && cap[n] !== '') t.textContent = cap[n];
    });
    var px = (d && d.proxies) || {};
    ['palVibratorHP1', 'palVibratorSht1', 'palVibratorSht2', 'palVibratorUnloader'].forEach(function (id) {   // golden FormShow :514-517
      if (px[id] && px[id].text !== undefined) panelCap(id, px[id].text);
    });
    var cyl = $('strngrdCylinderView');
    if (cyl) {
      cyl.onclick = null;
      cyl.innerHTML = '<div style="padding:8px;color:#a33;font-size:12px;">Cylinder 頁未移植（golden UpdateCylinderScreen 由主計時器刷新、'
                    + '彈出選單改值即寫 MachineLife.ini；見 FileRW/StartCondition.cpp 檔頭）</div>';
    }
    [['btnSetOffsetLimit', 'golden btnSetOffsetLimitClick 寫 Security_new.def（CosFunction.bSetOffsetLimitToAll 客戶專屬）：未移植']
    ].forEach(function (x) {   // AI(W906-SC-SDIAG) 20261001: sb_Maintenance_SmartDiagnosticFunction is no longer disabled here -- see the line after this block
      var b = $(x[0]);
      if (b) { b.disabled = true; b.title = x[0] + '：' + x[1]; }
    });  var sd = $('sb_Maintenance_SmartDiagnosticFunction'); if (sd && !sd.__scSD) { sd.__scSD = true; sd.disabled = false; sd.title = 'sb_Maintenance_SmartDiagnosticFunction : golden :1080-1084 fSmartDiagnostic->ShowModal() → 開 Smart Diagnostic 視窗'; sd.addEventListener('click', function (ev) { ev.preventDefault(); if (usable(sd) && window.parent !== window) window.parent.postMessage({ open: 'smartdiag' }, '*'); }); }   // AI(W906-SC-SDIAG) 20261001: golden cStartCondition.cpp:1080-1084 opens fSmartDiagnostic (the tab it sits on, tsSmartDiagnostic, is TabVisible=CosFunction.bCylinderOnOffTimeLog :499-506, sent by C++); was disabled "請用 Data.SmartDiagnostic 頁"
    var ft = $('cbStartModeOnlyFT');
    if (ft) ft.title = 'cbStartModeOnlyFT：golden OnClick（cbStartModeOnlyFTClick :1235，CC_SIGURD_HUKOU）未移植';
  }
  function bindTabs() {
    LIFE_TABS.forEach(function (n) {
      var t = lifeTab(n);
      if (t && !t.__scC) { t.__scC = true; t.addEventListener('click', onLifeTabClick); }
    });
  }

  /* ---- editlist.get／save 包裝 ---------------------------------------------------------------------- */
  R.editlistGet = function (st) {
    if (st !== STRUCT) return get0.apply(this, arguments);
    return get0.apply(this, arguments).then(function (d) {
      META = (d && d.extra) || null;
      ACTS = [];
      BUTTON = '';
      // 引擎在這個 then 之後才套元件（gbApply 會重畫 grid、設 visibility）→ 下一輪再補
      setTimeout(function () {
        bindTabs();
        bindActions();
        bindSaveButtons();
        bindGrids();
        decorate(d);
        var want = META && META.activePage;
        if (want && !activate(want)) say2('⚠ 後端的 Life Time 分頁 ' + want + ' 在頁面上看不到');
        if (!want) CUR = '';
      }, 0);
      return d;
    });
  };
  R.editlistSave = function (st, widgets, answers, extra) {
    if (st !== STRUCT) return save0.apply(this, arguments);
    widgets = widgets || {};
    widgets.W906_scButton = { text: BUTTON };                     // 見 FileRW/StartCondition.cpp BeforeApply
    widgets.W906_scActivePage = { text: CUR };
    widgets.W906_scActions = { text: JSON.stringify(ACTS) };
    var btn = BUTTON;
    BUTTON = '';
    return save0.call(this, st, widgets, answers, extra).then(function (a) {
      ACTS = [];
      if (btn === 'spbExit' && a && a.saved) {                   // golden Exit 關表單
        setTimeout(function () { if (window.parent !== window) window.parent.postMessage({ closeMe: 1 }, '*'); }, 600);
      }
      return a;
    });
  };

  /* ---- 引擎註冊（取代 B 路 ht9045_wire_datastartcondition.js 的 register；只留小鍵盤） ---------------------- */
  var kb = {
    editContactCountAlarm:   ['INTEGER', 0, false, 0, 0],          // golden :1227 上限 InputLimit.iContactCntAlm —— AI(W906-SC-ALMRANGE) 20261001：範圍改由 bindAlarmKeypad 照 C++ 送來的上限套（這一格只在拿不到上限時用）
    edtInOutArmPickerAlmCnt: ['INTEGER', 0, true, 0, 100000],      // golden :1382
    edtSetXYOffsetLimit:     ['INTEGER', 0, true, 0, 10],          // golden :1427
    edtSetZOffsetLimit:      ['INTEGER', 0, true, 0, 10]           // golden :1432
  };
  for (var r = 0; r < 4; r++) for (var c = 0; c < 8; c++) kb['Panel' + ROWS[r] + COLS[c]] = ['INTEGER', 0, true, 0, 2000000];   // golden :1232
  if (window.HT9045Wire && HT9045Wire.register) {
    HT9045Wire.register({ page: PAGE, slug: 'startcondition_c', fields: {}, optional: {}, kb: kb });
  }
})();
