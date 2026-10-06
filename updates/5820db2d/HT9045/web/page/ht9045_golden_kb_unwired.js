/* ht9045_golden_kb_unwired.js -- 小鍵盤照 golden（這台 CC_PTI 走到的那一支 ShowQwertyKey）＋「golden 有功能、網頁還沒接」的元件灰掉說明。
 * ---------------------------------------------------------------------------
 * AI(W906-KBGOLD) 20261002 新檔（手寫）。EastSun 1001「請檢查每個頁面元件…不是只有檢查按鈕喔 我說的是所有元件」「用枚舉 每個東西都檢查」：
 *   setupA 枚舉（Setup/Config 12 頁）逐格對 golden 的 OnClick／OnMouseDown → ShowQwertyKey(旗標, 小數位, 卡範圍, 下限, 上限)，
 *   找到接線檔抄錯分支（例：TrayForm 起點／厚度 0～1000，golden :495 是 0.001～1000；TesterIF 最大測試時間那 9 格 St02 package 117 已修）、
 *   漏抄（Configuration edA32_1 N_NO_SYMBOL|N_NO_SPACE）、或範圍跟著別的欄位動（TesterIF edPowerSwitchDelay 上限＝edMaxTestTime）。
 *   做法同 St02 ht9045_trayform_q41.js 的 pitchIntercept：document 捕獲階段先攔 mousedown、開 golden 的小鍵盤、不讓引擎的
 *   attachKeyboards 再開一次；提交時照引擎補發 input／change（其他補件聽 change）。別的補件已經處理掉的（defaultPrevented）不碰。
 *   欄位停用（disabled／aria-disabled）時不開（VCL 停用的 TEdit 收不到 OnMouseDown）。
 *   AI(W906-SETUPA-KB) 20261002：上下限是執行期變數的格子（Contact edForcePerPin*／edContactOffsetArm*／edShtPickOffset*／edDoubleForce）
 *   讀 C++ 開頁送的 editlist.get extra.kb（FileRW/DeviceForm_KbExtra.cpp，照 golden 式子算）；golden 處理器在小鍵盤之後還做的事
 *   （POST：Contact edDropWaitTimeMouseDown :2103-2238 那一整段修正、edDoubleForce :18368 CountDieForceKg(false)）在提交之後照做，
 *   伺服器端的值讀 extra.clamp；舊伺服器沒送 extra ⇒ 只有 golden 的旗標／小數位、不做修正（不猜）。
 *
 *   UNWIRED：golden 在這台看得見、按下去有動作，但移植樹沒有接（或是機台動作還沒移植）的元件 —— 照 HW.teach.html TEACH-UNWIRED
 *   的規則灰掉、點了在狀態列說原因，不送任何命令（不藏：藏掉的東西沒有人會發現它本來該在）。
 *   ⚠ 只是畫面標記；golden 自己藏起來的元件 C++ 開頁照樣藏（引擎 gbApply），這裡不管。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  function $(id) { return document.getElementById(id); }
  function num(id) { var el = $(id); var v = el ? parseFloat(el.value) : NaN; return isFinite(v) ? v : 0; }
  function ival(id) { var el = $(id); var v = el ? parseInt(el.value, 10) : NaN; return isFinite(v) ? v : 0; }
  var page = (location.pathname.split('/').pop() || '').replace(/\?.*$/, '');

  /* ---- 每頁的 golden 小鍵盤：id -> function () { return [旗標, 小數位, 卡範圍, 下限, 上限] } ------------------------- */
  var K = {}, POST = {};                                  // POST：golden 小鍵盤關掉之後同一支處理器接著做的事（ShowQwertyKey 是 modal）
  function set(ids, fn) { ids.forEach(function (id) { K[id] = fn; }); }
  function extra() { var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null; return (g && g.page && g.page.extra) || null; }
  function extraKb(id) { var e = extra(); var k = e && e.kb && e.kb[id]; return (k && k.length === 5) ? k : null; }
  function atof(s) { var v = parseFloat(String(s == null ? '' : s).replace(/^\s+/, '')); return isFinite(v) ? v : 0; }   // C atof：讀不到＝0
  function atoi(s) { var v = parseInt(String(s == null ? '' : s).replace(/^\s+/, ''), 10); return isFinite(v) ? v : 0; }
  function vcl(x) { return String(Number(Number(x).toPrecision(15))); }   // AnsiString(double)＝FloatToStr（15 位有效數字）
  function setTxt(id, s, changed) { var el = $(id); if (!el || el.value === s) return; el.value = s; changed.push(el); }   // VCL：Text 沒變就不觸發 OnChange
  function val(id) { var el = $(id); return el ? el.value : ''; }
  // golden cContact.cpp:2089 edDropWaitTimeMouseDown 的 28 個 OnMouseDown（cContact.dfm）
  var DROPWAIT = ['edYDimension', 'edXDimension', 'edReleaseHeight1', 'edPickUp1', 'edDropOffset1', 'edContactHeight1', 'edtTorqueMax', 'edtTorqueCmp',
                  'edReleaseHeight2', 'edPickUp2', 'edDropOffset2', 'edContactHeight2', 'edOrgPick1', 'edOrgPick2', 'edLoadCellHeight1', 'edLoadCellHeight2',
                  'edUpOffset2', 'edUpOffset1', 'edContactRelativeZ1', 'edContactRelativeZ2', 'edDropWaitTime', 'edDropSpeed', 'edtPurgeBeforePickShuttleTime',
                  'edtPurgeBeforePickShuttleInterval', 'edtPurgeBdforePickShuttleOffSet', 'edUpWaitTime', 'edUpSpeed', 'edSidePushWaitTime'];
  // golden :2103-2238：小鍵盤之後的修正（照原文順序；伺服器端的值在 extra.clamp，FileRW/DeviceForm_KbExtra.cpp）
  function dropWaitAfter() {
    var e = extra(), C = e && e.clamp;
    if (!C) return [];                                     // 舊伺服器沒送 ⇒ 不猜（Tray pitch 0 會把 IC 大小改成 0）
    var ch = [];
    if (C.hanaMicron) {                                                                       // :2103-2109
      if (atof(val('edReleaseHeight1')) < atof(val('edPickUp1'))) setTxt('edReleaseHeight1', vcl(atof(val('edPickUp1'))), ch);
      if (atof(val('edReleaseHeight2')) < atof(val('edPickUp2'))) setTxt('edReleaseHeight2', vcl(atof(val('edPickUp2'))), ch);
    } else {                                                                                  // :2110-2116 Release 至少比 Pick 高 1mm
      if (atof(val('edReleaseHeight1')) < (atof(val('edPickUp1')) + 1.00)) setTxt('edReleaseHeight1', vcl(atof(val('edPickUp1')) + 1.00), ch);
      if (atof(val('edReleaseHeight2')) < (atof(val('edPickUp2')) + 1.00)) setTxt('edReleaseHeight2', vcl(atof(val('edPickUp2')) + 1.00), ch);
    }
    if (atof(val('edContactHeight1')) >= 0.0) setTxt('edContactHeight1', '1', ch);            // :2118-2121
    if (atof(val('edContactHeight2')) >= 0.0) setTxt('edContactHeight2', '1', ch);
    if (atof(val('edContactHeight1')) <= C.indexDownPos) setTxt('edContactHeight1', vcl(C.indexDownPos), ch);   // :2122-2125
    if (atof(val('edContactHeight2')) <= C.indexDownPos) setTxt('edContactHeight2', vcl(C.indexDownPos), ch);
    var cm = $('cbContactMode'), slow = (cm && cm.selectedIndex === C.directContactModeDiffentSpeed) || C.tmoveSlowContact;   // :2127-2128
    ['edDropOffset1', 'edDropOffset2'].forEach(function (id) {                               // :2127-2155
      if (slow) { if (atof(val(id)) < 2.0) setTxt(id, '2', ch); }
      else if (atof(val(id)) <= 0.0) setTxt(id, '0', ch);
      if (atof(val(id)) >= 10.0) setTxt(id, '10', ch);
    });
    if ($('edYDimension')) {                                                                  // :2157-2162
      if (atof(val('edYDimension')) <= 2.0) setTxt('edYDimension', '2.0', ch);
      else if (atof(val('edYDimension')) >= C.loaderYPitch) setTxt('edYDimension', vcl(C.loaderYPitch), ch);
      setTxt('edYDimension', atof(val('edYDimension')).toFixed(2), ch);                       // FormatFloat("0.00", …)
    }
    if ($('edXDimension')) {                                                                  // :2164-2179
      if (atof(val('edXDimension')) <= 2.0) setTxt('edXDimension', '2.0', ch);
      else if (C.loadFormXDivision === 1 || C.loaderXPitch <= 0) { if (atof(val('edXDimension')) >= 130.0) setTxt('edXDimension', '130.0', ch); }
      else if (atof(val('edXDimension')) >= C.loaderXPitch) setTxt('edXDimension', vcl(C.loaderXPitch), ch);
      setTxt('edXDimension', atof(val('edXDimension')).toFixed(2), ch);
    }
    if ($('edDropSpeed')) {                                                                   // :2181-2192
      if (atoi(val('edDropSpeed')) > 100) setTxt('edDropSpeed', '100', ch);
      if (atoi(val('edDropSpeed')) < 1) setTxt('edDropSpeed', '1', ch);
      if (C.fixedDropSpeed && atof(val('edDropSpeed')) > C.iFixedDropSpeed) setTxt('edDropSpeed', String(C.iFixedDropSpeed), ch);
    }
    if ($('edDropWaitTime')) {                                                                // :2194-2197
      if (atof(val('edDropWaitTime')) > 10) setTxt('edDropWaitTime', '10.0', ch);
      if (atof(val('edDropWaitTime')) < 0.01) setTxt('edDropWaitTime', '0.01', ch);
    }
    if ($('edUpWaitTime')) {                                                                  // :2199-2202
      if (atof(val('edUpWaitTime')) > 10) setTxt('edUpWaitTime', '10.0', ch);
      if (atof(val('edUpWaitTime')) < 0.1) setTxt('edUpWaitTime', '0.1', ch);
    }
    if ($('edUpSpeed')) {                                                                     // :2204-2207
      if (atoi(val('edUpSpeed')) > 100) setTxt('edUpSpeed', '100', ch);
      if (atoi(val('edUpSpeed')) < 1) setTxt('edUpSpeed', '1', ch);
    }
    if ($('edtTorqueMax')) {                                                                  // :2209-2212
      if (atoi(val('edtTorqueMax')) > 150) setTxt('edtTorqueMax', '150.0', ch);
      if (atoi(val('edtTorqueMax')) < 1) setTxt('edtTorqueMax', '1.0', ch);
    }
    if ($('edtTorqueCmp')) {                                                                  // :2214-2217
      if (atoi(val('edtTorqueCmp')) > 30) setTxt('edtTorqueCmp', '30.0', ch);
      if (atoi(val('edtTorqueCmp')) < 10) setTxt('edtTorqueCmp', '10.0', ch);
    }
    if (C.koreaFunction) {                                                                    // :2219-2238
      ['edtPurgeBeforePickShuttleTime', 'edtPurgeBeforePickShuttleInterval', 'edtPurgeBdforePickShuttleOffSet'].forEach(function (id) {
        if (!$(id)) return;
        setTxt(id, String(atoi(val(id))), ch);
        if (atoi(val(id)) > 10) setTxt(id, '10', ch);
        if (atoi(val(id)) < 1) setTxt(id, '1', ch);
      });
    }
    return ch;
  }
  // golden :18371 CountDieForceKg(false)（edDoubleForce 輸入之後）：edtPinOfDie＝int(edDoubleForce*1000／edForcePerPinG)；edtPinOfDie 沒有 OnChange
  function countDieForceKgByForce() {
    var ch = [], dForce = atof(val('edDoubleForce')) * 1000, dPinForce = atof(val('edForcePerPinG'));
    var iPinCount = (dPinForce !== 0) ? (dForce / dPinForce) : (dForce / 1.0);
    setTxt('edtPinOfDie', String(Math.trunc(iPinCount)), ch);
    return [];                                             // 不補發事件（golden 沒有 OnChange）
  }
  if (page === 'Setup.TesterIF.html') {
    // golden cTesterIF.cpp:1380-1403 edInitialMaxTestClick（:1401 N_DOUBLE 2 0～15000）與 :1410-1416 edtInitStartDelayClick（:1415 不卡範圍）
    //   那 9 格由 St02 ht9045_testerif_c_wire.js (8)（AI(W906-TIF-KB) 20261002，laptop package 117）改引擎的 CFG.kb —— 值相同，這裡不重複攔。
    // golden :1535-1538 edPowerSwitchDelayClick：上限＝atof(edMaxTestTime->Text)（按下去當下的值）
    set(['edPowerSwitchDelay', 'edTestOKWaitTime', 'edSendNEXTDelay'], function () { return ['DOUBLE', 2, true, 0, num('edMaxTestTime')]; });
    // golden :1540-1543 edMaxBIOSWaitTimeClick：上限＝(atoi(edMaxTestTime)-2>0)?atof(edMaxTestTime)-2:0
    set(['edMaxBIOSWaitTime', 'edSLTMaxTestTime'], function () { return ['DOUBLE', 2, true, 0, (ival('edMaxTestTime') - 2 > 0) ? num('edMaxTestTime') - 2 : 0]; });
    // golden :1545-1548 edMinTestTimeClick：上限＝atof(edSLTMaxTestTime)-1
    set(['edMinTestTime'], function () { return ['DOUBLE', 2, true, 0, num('edSLTMaxTestTime') - 1]; });
  } else if (page === 'Setup.TrayForm.html') {
    // golden cTrayForm.cpp:481-497 YST1MouseDown：不是 CC_ASE_SG ⇒ :495 N_DOUBLE 3 0.001～1000（接線檔抄成 0～1000）
    set(['XST1', 'XST2', 'XST3', 'YST1', 'YST2', 'YST3', 'Tp1Thick', 'Tp2Thick', 'Tp3Thick', 'YPitch2', 'YPitch3',
         'XBP1', 'XBP2', 'XBP3', 'YBP1', 'YBP2', 'YBP3', 'YBItem1', 'YBItem2', 'YBItem3', 'YBTypeSize1', 'YBTypeSize2', 'YBTypeSize3'],
        function () { return ['DOUBLE', 3, true, 0.001, 1000.00]; });
    // golden :1021-1072 XPitch1Click／:1079-1130 YPitch1Click：數量不是 "1" 時 :1040／:1098 N_DOUBLE 3 0.001～1000（"1" 的那支由 ht9045_trayform_q41.js 先處理）
    set(['XPitch1', 'YPitch1'], function () { return ['DOUBLE', 3, true, 0.001, 1000.00]; });
    // golden :789-801 Tp1TickUpMouseDown：IniConfig.bC03UseCatchTray ? 60～73 : 30～110 —— 同一個旗標 FormShow :256-267 寫 Lab1XPickup 的字（C++ 開頁帶回）
    set(['Tp1TickUp', 'Tp2TickUp', 'Tp3TickUp'], function () {
      var catchTray = /Catch/i.test((($('Lab1XPickup') || {}).textContent) || '');
      return catchTray ? ['DOUBLE', 3, true, 60, 73.00] : ['DOUBLE', 3, true, 30, 110.00];
    });
  } else if (page === 'Config.Configuration.html') {
    // golden cConfiguration.cpp:7611-7614 edA32_1Click（edA32_1、edHeadCondition1-3 共用）：N_NO_SYMBOL|N_NO_SPACE（接線檔漏抄 ⇒ 原本是通用 QWERTY 可打符號）
    set(['edA32_1', 'edHeadCondition1', 'edHeadCondition2', 'edHeadCondition3'], function () { return ['NO_SYMBOL|NO_SPACE', 0, false, 0, 0]; });
    // golden :6603-6607 edA22_3Click：下限＝atof(edA22_2->Text)+0.01、上限 10（接線檔抄成不卡範圍）
    set(['edA22_3'], function () { return ['DOUBLE', 2, true, num('edA22_2') + 0.01, 10.0]; });
    // golden :6440-6443 edD25_60mmClick／:6580-6583 edD60_56mmClick：N_DOUBLE 2 卡 ±0.5（接線檔抄成不卡）
    set(['edD25_60mm', 'edD25_40mm', 'edD25_30mm', 'edD60_56mm'], function () { return ['DOUBLE', 2, true, 0.5, -0.5]; });
  } else if (page === 'Setup.Contact.html') {
    // golden cContact.cpp:17189-17193 edDropByPassDetectMouseDown：N_DOUBLE 2 卡 0～20（接線檔抄成不卡）
    set(['edDropByPassDetect'], function () { return ['DOUBLE', 2, true, 20.0, 0.00]; });
    // golden :15321-15328 edD41MouseDown：不是 CC_KYEC_LEE ⇒ :15327 N_DOUBLE 2 卡 0～20
    set(['edD41'], function () { return ['DOUBLE', 2, true, 20.0, 0.00]; });
    // golden :15035-15041 edSpeedZMouseDown（edSpeed、edSpeedZ 共用）：MyInputBox 之後 edSpeed=CheckRange(…,90,10)、edSpeedZ=CheckRange(…,150,30)
    set(['edSpeed'], function () { return ['INTEGER', 0, true, 10, 90]; });
    set(['edSpeedZ'], function () { return ['INTEGER', 0, true, 30, 150]; });
    // AI(W906-SETUPA-KB) 20261002：上下限是執行期變數的那幾格 —— C++ 開頁照 golden 式子算好放在 editlist.get extra.kb
    //   （FileRW/DeviceForm_KbExtra.cpp）：edForcePerPinG/N＋edDieForcePerPinG/N :19029／:18542（InputLimit.dForcePerpin*，非 JCET／AMKOR_China +10）、
    //   edContactOffsetArm1/2 :15164（InputLimit.dContact*）、edShtPickOffset1/2 :16867（bChangeKitNoHardStop ? dShuttle* : iOffsetZ*）、
    //   edDoubleForce :18357（Die Force kit 直徑換算 40～500 kPa）。舊伺服器沒送 extra.kb ⇒ 只有 golden 的旗標／小數位（同以前）。
    set(['edForcePerPinG', 'edDieForcePerPinG', 'edForcePerPinN', 'edDieForcePerPinN'], function (id) { return extraKb(id) || ['DOUBLE', 4, false, 0, 0]; });
    set(['edContactOffsetArm1', 'edContactOffsetArm2', 'edShtPickOffset1', 'edShtPickOffset2', 'edDoubleForce'], function (id) { return extraKb(id) || ['DOUBLE', 2, false, 0, 0]; });
    // golden :2089-2239 edDropWaitTimeMouseDown（這 28 格共用）：N_DOUBLE 3 不卡範圍，輸入後跑一整段修正（POST 下面）
    set(DROPWAIT, function () { return ['DOUBLE', 3, false, 0, 0]; });
    DROPWAIT.forEach(function (id) { POST[id] = dropWaitAfter; });
    POST.edDoubleForce = countDieForceKgByForce;          // golden :18368 CountDieForceKg(false)
  } else if (page === 'Config.DIOInterFaceCFG.html' || page === 'Setup.DIOInterFaceCFG.html') {
    // golden DIOInterFaceCFG.cpp:178-189 edPulseWidthMouseDown：CosFunction.bTTLUseUSec（CC_PTI 沒開）? 1～500000 : 10～500 ⇒ :187
    set(['edPulseWidth', 'edSignalBeforeOn', 'edSignalAfterOff'], function () { return ['INTEGER', 0, true, 10, 500]; });
  }

  function fire(el) {                                     // 同引擎小鍵盤 onCommit（ht9045_wire_engine.js attachKeyboards）
    ['input', 'change'].forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function flagsOf(s) { var f = 0; String(s).split('|').forEach(function (n) { f |= (window.HTQwerty && HTQwerty.N[n]) || 0; }); return f; }
  document.addEventListener('mousedown', function (ev) {
    var t = ev.target;
    if (!t || !t.id || !K[t.id] || ev.defaultPrevented || ev.button > 0) return;
    if (!window.HTQwerty || t.disabled || t.getAttribute('aria-disabled') === 'true') return;
    for (var p = t.parentElement; p; p = p.parentElement) if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return;
    ev.preventDefault(); ev.stopPropagation();            // 引擎掛在欄位上的 mousedown（通用／抄錯的小鍵盤）不跑
    var k = K[t.id](t.id);
    HTQwerty.show(t, flagsOf(k[0]), { dp: k[1], checkRange: !!k[2], min: k[3], max: k[4], onCommit: function () {
      fire(t);
      if (POST[t.id]) POST[t.id]().forEach(function (el) { fire(el); });   // golden 同一支處理器在小鍵盤之後改的格子（含自己；VCL：值有變才 OnChange）
    } });
  }, true);

  /* ---- UNWIRED：golden 在這台會動作、網頁還沒接的元件 ----------------------------------------------------- */
  //AI(W906-CTREASON) 20261006 (Jerry, JCH-2 第三步)：原本 11 顆模式單選共用一個 NOT_PORTED_CT，理由是
  //  「還沒移植到 C++（forms/fContact.cpp GATE）」。那句**已經過期**：本檔最後更動 2026-10-02（40384d48），
  //  而 St01 的 E-042 在 2026-10-05 把 SetContactMode／DoTestContactFunction 翻進 forms/fContact_ContactSM.cpp
  //  （936b9c99）。照 C++ 的**實際**狀態拆成兩個理由，灰掉狀態一顆都沒有解除。
  //  盤點與出處：docs/handoff/JCH2_CONTACT_HEIGHT_INVENTORY_20261005.md §2（模式表）、§8（三層現況）。
  //  ⚠ 兩個理由都保留「這一版不接（會讓機台自己動）」那半句 —— 那是**安全決定**，不是事實陳述，
  //    更正事實不等於可以解灰。要解灰得有 Jimmy／EastSun 的裁決（盤點 §8.4 第 3 件）。
  //  模式 0／1／2／3／8：C++ 翻了，但全樹唯一的呼叫點還閘著。
  var CT_WAIT_CALLER = 'Contact 模式（Normal／Auto Height／Manual Height／Contact Test／LoadCell Auto High）：C++ 已經翻好了（E-042，forms/fContact_ContactSM.cpp SetContactMode:109、DoTestContactFunction:209），但**全樹唯一的呼叫點還閘著** —— csystem.cpp:31453 GATE(W906-HOME-W1-CONTACTFN)：全域 fContact 指向 TfContactShim，沒有這個成員 ⇒ 按了不會動。這一版也刻意不接（接上去機台會自己動，要先有安全裁決）';
  //  模式 4／5／9／10／11／12：葉子流程真的沒翻，IndexZTorqueCore.h:494 ModeUntranslated() 會在 case 1 當場拒絕。
  var CT_NOT_PORTED = 'Contact 模式（Auto Contact Test／Step Contact Test／Device Map Check／Device Loop Test／K Temp Index Move／Visual Detection）：葉子流程還沒移植到 C++（forms/fContact_ContactSM.cpp 的 E042Leaf 樁；IndexZTorqueCore.h:494 ModeUntranslated() 會在 case 1 當場拒絕，不會跑到一半停住）⇒ 按了不會動。Visual Detection 在 golden 0618 根本沒有 arm（TODO E-052）。這一版不接（會讓機台自己動）';
  var U = {
    'Setup.Contact.html': [
      ['rbModeNormal', CT_WAIT_CALLER], ['rbAutoHeight', CT_WAIT_CALLER], ['rbManualHeight', CT_WAIT_CALLER], ['rbContactTest', CT_WAIT_CALLER],
      ['rbAutoContactTest', CT_NOT_PORTED], ['rbStepContactTest', CT_NOT_PORTED], ['rbDeviceMapping', CT_NOT_PORTED], ['rbLoadCellAutoHigh', CT_WAIT_CALLER],
      ['rbKTempIndexMove', CT_NOT_PORTED], ['rbDeviceLoopTest', CT_NOT_PORTED], ['rbVisualDetectionTest', CT_NOT_PORTED],
      ['btnStart', 'Contact Start（golden btnStartClick cContact.cpp:13965 → fMain->BtnStartClick）：接觸測試模式還沒移植，這裡不接（按了會開始生產流程）'],
      ['btnPause', 'Contact Pause（golden btnPauseClick :13972 → fMain->BtnPauseClick）：接觸測試模式還沒移植，這裡不接'],
      ['btnTStart', 'Step Contact Test Start（golden btnTStartClick :2251 bSetupStart=true）：步進接觸測試還沒移植'],
      ['btnTStep', 'Step Contact Test Step（golden btnTStepClick :2256 bSetupStep=true）：步進接觸測試還沒移植'],
      ['spbOneCycle', 'One Cycle（golden spbOneCycleClick :16861 OneCycleProcess）：還沒移植（forms/fContact.cpp GATE X-32）'],
      ['AutoZTeachButton', 'Auto Z Teach（golden AutoZTeachButtonClick :18430）：自動教導 Z 會讓機台自己動，這一版不接'],
      ['btnIndexArmJogMove_Up', 'Index Arm 上移（golden btnIndexArmJogMove_UpClick :16950，接觸測試中寸動）：還沒移植，不接（會動 Z 軸）'],
      ['btnIndexArmJogMove_Down', 'Index Arm 下移（golden btnIndexArmJogMove_DownClick :17002）：還沒移植，不接（會動 Z 軸）'],
      ['btEditTray', 'Edit Tray（golden btEditTrayClick :16853 EditTray(MMTrayY)）：還沒移植'],
      ['cbRTCAutoTuning', 'RTC Auto Tuning（golden cbRTCAutoTuningClick :20365）：RTC 自動調整流程還沒移植（forms/fContact.cpp GATE S-36）'],
      ['labMPa', 'golden labMPaDblClick :16943 顯示 labEPValue（即時 EP 值）：這台沒有 labEPValue 的資料來源'],
      ['btnTrayMap', 'Tray Map（golden btnTrayMapClick :18402 RecordProcess＋fTrayMapping->Show）：Tray Mapping 視窗沒有網頁（background.html 沒有這個視窗）']
    ],
    'Config.Configuration.html': [
      ['btnA71Manually', '[A71] Manually（golden btnA71ManuallyClick cConfiguration.cpp:7771 → fMain->RunBatchCopyRecipe main.cpp:34444：xcopy 目前配方到 D:\\Run）：C++ 還沒移植'],
      ['btnUploadAll', '[N05] Upload All（golden btnUploadAllClick :7757 → 每個配方 fLotInfo->DoUpload，uLotInfo.cpp:15462）：C++ 還沒移植'],
      ['btnN35_Test', '[N35] Test（golden btnN35_TestClick :7752 → fLotInfo->SaveGroundESDData_Upolad，FTP 上傳）：C++ 還沒移植']
    ],
    'Config.DIOInterFaceCFG.html': [
      ['spbLoad', 'Load（golden spbLoadClick DIOInterFaceCFG.cpp:237：開檔對話框選另一個 DIO 檔載入）：C++ 只會載入本配方目前的 DIO 檔（開頁時，TTLCfg.cpp:259），選別的檔還沒做']
    ]
  };
  var CSS = '.ht-unwired{opacity:.4 !important;filter:grayscale(1);cursor:not-allowed !important;}';
  function mark(el, why) {
    if (!el || el.getAttribute('data-unwired')) return;
    if (!$('htUnwiredCss')) { var st = document.createElement('style'); st.id = 'htUnwiredCss'; st.textContent = CSS; document.head.appendChild(st); }
    el.classList.add('ht-unwired');
    el.setAttribute('data-unwired', why);
    el.title = (el.title ? el.title + ' | ' : '') + why;
    el.addEventListener('click', function (ev) {
      ev.stopImmediatePropagation();
      var box = (el.tagName === 'INPUT' ? el : null) || (el.querySelector ? el.querySelector('input[type="checkbox"],input[type="radio"]') : null);
      if (box && (box.type === 'checkbox' || box.type === 'radio')) ev.preventDefault();   // 勾選框／選項灰掉也不能被選（同 TEACH-UNWIRED；label 的點擊也撤銷）
      if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(why, '#ffcc66', 'transient');
    }, true);
    el.addEventListener('dblclick', function (ev) { ev.stopImmediatePropagation(); if (window.HT9045Wire && HT9045Wire.say) HT9045Wire.say(why, '#ffcc66', 'transient'); }, true);
  }
  function markAll() { (U[page] || []).forEach(function (p) { mark($(p[0]), p[1]); }); }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', markAll); else markAll();

  /* ---- AI(W906-MOTB-GKB) 20261002 (motorB): the same golden-keypad rule for 7 more pages, one capture handler (the one above) --------
   *   The generated wire files (gen_wire.py) switch the range OFF whenever the golden min is <= 0 (citing myQwertyKeyBoard.cpp:252),
   *   but golden swaps the range only for N_PORT (:248-255); edits whose golden range is an expression got no keypad entry at all.
   *   Rows = golden ShowQwertyKey(Sender, flags, dp, rangeCheck, a, b) of the dfm OnClick / OnMouseDown handler (cited per row; built
   *   by scratchpad/compK_motorB/genkb.js from golden). a / b = a number, or {edit:id, f, add} = golden atoi/atof(id->Text)+add read at
   *   the click. Run-time ranges (InputLimit.*, MaxTempSetting() ...) are not here. A page's own K rows (above) win. */
  var MOTB = {
    "Alert.Note.html": {
      "edEQCQty": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyAuto1": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyAuto2": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyAuto3": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix1": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix10": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix11": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix12": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix2": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix3": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix4": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix5": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix6": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix7": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix8": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyFix9": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edQtyLoader": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
      "edtLotID": ["NO_SPACE|NO_SYMBOL",0,false],   // edtLotIDClick :5769 (was none)
    },
    "Setup.Cleaning.html": {
      "edACInitalContactCount": ["INTEGER",0,true,100,0],   // edACInitalContactCountClick :2874 (was 'INTEGER', 0, false, 0, 0)
      "edAutoCleanAirForce_Kg": ["DOUBLE",2,true,500,0],   // edAutoCleanAirForce_KgClick :2631 (was 'DOUBLE', 2, false, 0, 0)
      "edAutoCleanAirForce_N": ["DOUBLE",2,true,5000,0],   // edAutoCleanAirForce_NClick :2636 (was 'DOUBLE', 2, false, 0, 0)
      "edCleanPadDeviation": ["DOUBLE",1,true,10,0],   // edDropOffset1Click :2621 (was 'DOUBLE', 1, false, 0, 0)
      "edCleaningCount": ["INTEGER",0,true,{"edit":"edAlarmCount","f":0,"add":-1},0],   // edCleanCountClick :2080; (atoi(edAlarmCount->Text.c_str()))-1 (was none)
      "edContactTime": ["DOUBLE",1,true,10,0],   // edDropOffset1Click :2621 (was 'DOUBLE', 1, false, 0, 0)
      "edDropOffset1": ["DOUBLE",1,true,10,0],   // edDropOffset1Click :2621 (was 'DOUBLE', 1, false, 0, 0)
      "edIndexArmAutoCleanCnt": ["INTEGER",0,true,{"edit":"edAlarmCount","f":0,"add":-1},0],   // edCleanCountClick :2080; (atoi(edAlarmCount->Text.c_str()))-1 (was none)
      "edtAutoCleanDieForce": ["DOUBLE",2,true,500,0],   // edAutoCleanAirForce_KgClick :2631 (was 'DOUBLE', 2, false, 0, 0)
    },
    "Setup.OffSet.html": {
      "IndexArmOffSet4": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edArmX": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edArmY": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPitchX1": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPitchX2": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPitchX3": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPitchX4": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPitchY": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edPreciserClose": ["DOUBLE",0,true,0.5,0],   // edPreciserOpenMouseDown :3088 (was 'DOUBLE', 0.0, false, 0, 0)
      "edPreciserOpen": ["DOUBLE",0,true,0.5,0],   // edPreciserOpenMouseDown :3088 (was 'DOUBLE', 0.0, false, 0, 0)
      "edlLoadZ": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edtARTPlace": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
      "edtTrayArmX": ["DOUBLE",2,true,100,-100],   // edArmXMouseDown :2601 (was 'DOUBLE', 2, false, 0, 0)
    },
    "Setup.SetUp.html": {
      "XPitch": ["DOUBLE",3,true,0,1000],   // XPitchMouseDown :3409; IniConfig.bSPILFunction=false here -> golden cSetUp.cpp:3418 N_DOUBLE dp 3 (the wire kb took the SPIL branch, dp 2) (was 'DOUBLE', 2, true, 0.0, 1000.0)
      "XShiftPitch": ["DOUBLE",3,true,-4000,4000],   // XShiftPitchMouseDown :4738; IniConfig.bSPILFunction=false here (not set in D:\HT9045\config\config.ini) -> golden cSetUp.cpp:4748 N_DOUBLE dp 3 (was 'DOUBLE', 2, false, 0, 0)
      "YPitch": ["DOUBLE",3,true,0,1000],   // XPitchMouseDown :3409; IniConfig.bSPILFunction=false here -> golden cSetUp.cpp:3418 N_DOUBLE dp 3 (the wire kb took the SPIL branch, dp 2) (was 'DOUBLE', 2, true, 0.0, 1000.0)
      "edOcrText": ["NO_SPACE|NO_SYMBOL",0,false],   // edOcrTextMouseDown :4631 (was none)
      "edOverRange": ["DOUBLE",0,true,100,0],   // edOverRangeClick :4839 (was 'DOUBLE', 0, false, 0, 0)
      "edPreciserXPitch": ["DOUBLE",3,true,0,1000],   // XPitchMouseDown :3409; IniConfig.bSPILFunction=false here -> golden cSetUp.cpp:3418 N_DOUBLE dp 3 (the wire kb took the SPIL branch, dp 2) (was 'DOUBLE', 2, true, 0.0, 1000.0)
      "edPreciserYPitch": ["DOUBLE",3,true,0,1000],   // XPitchMouseDown :3409; IniConfig.bSPILFunction=false here -> golden cSetUp.cpp:3418 N_DOUBLE dp 3 (the wire kb took the SPIL branch, dp 2) (was 'DOUBLE', 2, true, 0.0, 1000.0)
      "edYOffset": ["DOUBLE",3,true,-4000,4000],   // XShiftPitchMouseDown :4738; IniConfig.bSPILFunction=false here (not set in D:\HT9045\config\config.ini) -> golden cSetUp.cpp:4748 N_DOUBLE dp 3 (was 'DOUBLE', 2, false, 0, 0)
      "edtXCenterPitch": ["DOUBLE",3,true,0,1000],   // XPitchMouseDown :3409; IniConfig.bSPILFunction=false here -> golden cSetUp.cpp:3418 N_DOUBLE dp 3 (the wire kb took the SPIL branch, dp 2) (was 'DOUBLE', 2, true, 0.0, 1000.0)
    },
    "Setup.Speed.html": {
      "edInArmDestroyAgainCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edInArmRetryCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edInArmRetryMM": ["DOUBLE",2,true,-0.01,-0.3],   // edIndexArmRetryMMMouseDown :1254 (was 'DOUBLE', 2, false, 0, 0)
      "edInPitchSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edInRotAcc": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edInRotSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edInXYSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edInZSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edIndexArmRetryMM": ["DOUBLE",2,true,-0.01,-0.3],   // edIndexArmRetryMMMouseDown :1254 (was 'DOUBLE', 2, false, 0, 0)
      "edIndexDestroyAgainCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edIndexRetryCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edIndexSpeed": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edMagCatchYSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edMagZSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edOutArmDestroyAgainCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edOutArmRetryCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edOutArmRetryMM": ["DOUBLE",2,true,-0.01,-0.3],   // edIndexArmRetryMMMouseDown :1254 (was 'DOUBLE', 2, false, 0, 0)
      "edOutArmShtWaitTime": ["DOUBLE",2,true,50,0],   // edOutArmShtWaitTimeMouseDown :2469 (was 'DOUBLE', 2, false, 0, 0)
      "edOutPitchSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edOutRotSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edOutXSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edOutZSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edPrecisorCloseSp": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edPrecisorOpenSp": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edSecondADCCatchY": ["INTEGER",0,true,1,{"edit":"edInZAcc","f":0,"add":0}],   // edSecondADCInMouseDown :1959; atoi(edInZAcc->Text.c_str()) (was none)
      "edSecondADCIn": ["INTEGER",0,true,1,{"edit":"edInZAcc","f":0,"add":0}],   // edSecondADCInMouseDown :1959; atoi(edInZAcc->Text.c_str()) (was none)
      "edSecondADCOut": ["INTEGER",0,true,1,{"edit":"edOutZAcc","f":0,"add":0}],   // edSecondADCOutMouseDown :1971; atoi(edOutZAcc->Text.c_str()) (was none)
      "edSecondSpeedCatchY": ["INTEGER",0,true,1,{"edit":"edInZSpd","f":0,"add":0}],   // edSecondSpeedInMouseDown :1953; atoi(edInZSpd->Text.c_str()) (was none)
      "edSecondSpeedIn": ["INTEGER",0,true,1,{"edit":"edInZSpd","f":0,"add":0}],   // edSecondSpeedInMouseDown :1953; atoi(edInZSpd->Text.c_str()) (was none)
      "edSecondSpeedOut": ["INTEGER",0,true,1,{"edit":"edOutZSpd","f":0,"add":0}],   // edSecondSpeedOutMouseDown :1965; atoi(edOutZSpd->Text.c_str()) (was none)
      "edShakeDelay": ["DOUBLE",0,true,2,0],   // edShakeDelayMouseDown :2499 (was 'DOUBLE', 0.0, false, 0, 0)
      "edSht1Spd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edSht2Spd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edTrayArmRetryCount": ["INTEGER",2,true,3,0],   // edInArmRetryCountMouseDown :1235 (was 'INTEGER', 2, false, 0, 0)
      "edTrayXSpd": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto1Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto1SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto2Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto2SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto3Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto3SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto4Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto4SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto5Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto5SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto6Speed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtAuto6SpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtColorSpeed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtColorSpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtEmptySpeed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtEmptySpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtLoaderSpeed1": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtLoaderSpeedZ": ["INTEGER",1,true,100,1],   // edtLoaderSpeed1MouseDown :2448; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
      "edtTrySpeed": ["INTEGER",0,true,100,1],   // edIndexSpeedMouseDown :1260; (CosFunction.bLimitMaxSpeed)?CosFunction.iLimitMaxSpeed:100=100 (CosFunction.bLimitMaxSpeed=false for CC_PTI (CosFunction.cpp:4236; only ASE K12 sets it, :733)) (was none)
    },
    "Setup.Temp_Set.html": {
      "edATCContFailOffset1": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCContFailOffset2": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCContFailOffset3": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCContFailOffset4": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCInitialOffset1": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCInitialOffset2": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCInitialOffset3": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCInitialOffset4": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCQAModeOffset1": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCQAModeOffset2": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCQAModeOffset3": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCQAModeOffset4": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCTestTimeOffset1": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCTestTimeOffset2": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCTestTimeOffset3": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATCTestTimeOffset4": ["DOUBLE",1,true,40,0],   // edATCInitialOffset1MouseDown :5474 (was none)
      "edATC_Arm1_S1_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S1_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S1_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S1_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S2_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S2_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S2_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S2_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S3_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S3_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S3_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S3_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S4_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S4_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S4_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm1_S4_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S1_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S1_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S1_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S1_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S2_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S2_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S2_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S2_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S3_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S3_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S3_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S3_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S4_TC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S4_TC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S4_TC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edATC_Arm2_S4_TC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edAbitColdTime": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edAbitInitWaitTime": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edArm1NoFullsiteOffset_1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm1NoFullsiteOffset_2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm1NoFullsiteOffset_3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm1NoFullsiteOffset_4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm1NoFullsiteOffset_5": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm1Offset": ["DOUBLE",2,true,-2,2],   // edArm1OffsetMouseDown :5430 (was 'DOUBLE', 2, false, 0, 0)
      "edArm2NoFullsiteOffset_1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm2NoFullsiteOffset_2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm2NoFullsiteOffset_3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm2NoFullsiteOffset_4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm2NoFullsiteOffset_5": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edArm2Offset": ["DOUBLE",2,true,-2,2],   // edArm1OffsetMouseDown :5430 (was 'DOUBLE', 2, false, 0, 0)
      "edChillerTemp": ["INTEGER",0,true,-20,30],   // edATCChillerTempMouseDown :5451 (was 'INTEGER', 0, true, 5, 40)
      "edContinuousSec": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_OffsetClick :6281 (was none)
      "edFFC_Arm1Offset_01": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_02": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_03": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_04": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_05": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_06": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_07": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_08": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_09": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1Offset_10": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm1TimeOff_01": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_02": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_03": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_04": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_05": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_06": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_07": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_08": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_09": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOff_10": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_01": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_02": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_03": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_04": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_05": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_06": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_07": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_08": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_09": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm1TimeOn_10": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2Offset_01": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_02": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_03": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_04": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_05": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_06": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_07": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_08": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_09": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2Offset_10": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edFFC_Arm2TimeOff_01": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_02": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_03": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_04": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_05": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_06": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_07": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_08": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_09": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOff_10": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_01": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_02": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_03": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_04": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_05": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_06": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_07": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_08": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_09": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edFFC_Arm2TimeOn_10": ["INTEGER",0,true,10000,0],   // edFFC_Arm1TimeOn_01Click :6261 (was 'INTEGER', 0, false, 0, 0)
      "edInitialStart1Time": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edInitialStart2Time": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edInitialWaitTime": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edJamSoakTime": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edSoakTime": ["INTEGER",0,true,10000,0],   // edSoakTimeClick :4109 (was 'INTEGER', 0, false, 0, 0)
      "edSocketAirCoolingOff": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edSocketAirCoolingOn": ["INTEGER",0,true,10000,0],   // edJamSoakTimeClick :4123 (was 'INTEGER', 0, false, 0, 0)
      "edTSDTimeOut": ["DOUBLE",1,true,100,0],   // edTSDTimeOutMouseDown :5497 (was none)
      "edt3SigmaTempMonitior_Set3xSigmaValue": ["DOUBLE",1,true,100,0],   // edTSDTimeOutMouseDown :5497 (was none)
      "edt3SigmaTempMonitior_SetCount": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCInPC1": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCInPC2": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCInPC3": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCInPC4": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_01": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_02": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_03": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_04": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_05": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_06": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_07": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_08": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_09": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_10": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_11": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_12": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_13": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_14": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_15": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_16": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_17": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_18": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_19": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_20": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_21": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_22": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_23": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_24": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_25": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_26": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_27": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_28": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_29": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_30": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_31": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCOffset_32": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATCPIDOffset_MaxD": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCPIDOffset_MaxI": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCPIDOffset_MaxP": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCPIDOffset_MinD": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCPIDOffset_MinI": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATCPIDOffset_MinP": ["INTEGER",0,true,99999,0],   // edtATCPIDOffset_MinPClick :6256 (was 'INTEGER', 0, false, 0, 0)
      "edtATC_HotGunTime": ["DOUBLE",2,true,30,0],   // edtATC_HotGunTimeClick :6266 (was 'DOUBLE', 2, false, 0, 0)
      "edtATC_PackageOffset_01": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATC_PackageOffset_02": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtATC_PackageOffset_03": ["DOUBLE",1,true,20,-20],   // edtATCInPC1MouseDown :5379; CUSTOMER_CODE=CC_PTI (not CC_ASE_KaohSiung) -> golden uTemp_Set.cpp:5393 N_DOUBLE 1, 20.0..-20.0 (was none)
      "edtBoostOffset": ["DOUBLE",2,true,-5,5],   // edtBoostOffsetClick :5754 (was 'DOUBLE', 2, false, 0, 0)
      "edtBoostOffset_LB": ["DOUBLE",2,true,-30,30],   // edtBoostOffset_LongClick :5749 (was 'DOUBLE', 2, false, 0, 0)
      "edtBoostOffset_Long": ["DOUBLE",2,true,-30,30],   // edtBoostOffset_LongClick :5749 (was 'DOUBLE', 2, false, 0, 0)
      "edtBoostOffset_Mid": ["DOUBLE",2,true,-30,30],   // edtBoostOffset_LongClick :5749 (was 'DOUBLE', 2, false, 0, 0)
      "edtBoostOffset_Short": ["DOUBLE",2,true,-30,30],   // edtBoostOffset_LongClick :5749 (was 'DOUBLE', 2, false, 0, 0)
      "edtDelayAfterSOT": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_OffsetClick :6281 (was none)
      "edtInputVHigh": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_OffsetClick :6281 (was none)
      "edtInputVLow": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_OffsetClick :6281 (was none)
      "edtLBTempOffset": ["DOUBLE",2,true,-30,30],   // edtBoostOffset_LongClick :5749 (was 'DOUBLE', 2, false, 0, 0)
      "edtSetTJ_Offset": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_OffsetClick :6281 (was none)
      "edtSetTJ_Slope": ["DOUBLE",2,true,99999,-99999],   // edtSetTJ_SlopeChange :6286 (was none)
    },
    "Setup.YieldMonitoring.html": {
      "edAdaptiveYieldMax_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAdaptiveYieldMax_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAdaptiveYieldMin_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAdaptiveYieldMin_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm4ContinueCount": ["INTEGER",0,true,0,100],   // edAlarm4ContinueCountMouseDown :3628; iMinYield=0 (uYieldMonitoring.cpp:292); iMaxYield=100 (:293) (was 'INTEGER', 0, false, 0, 0)
      "edAlarm4IntervalCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edAlarm5_BySiteAlarmYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySiteAlarmYieldRej": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySiteCmpYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySiteCmpYieldRej": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySiteIntervalContactCnt": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edAlarm5_BySiteLowYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySiteLowYieldRej": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySitePreCmpYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAlarm5_BySitePreCmpYieldRej": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edAllSiteFailCount": ["INTEGER",0,true,1,100000],   // edContsFailIgnore_FTMouseDown :3573; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edAllSiteFailCountRT": ["INTEGER",0,true,1,100000],   // edContsFailIgnore_FTMouseDown :3573; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edAutoCleanLowYieldCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edAutoLeastRetestLimitFile": ["INTEGER",0,true,{"edit":"edAutoRetestLimitFile","f":0,"add":0},1],   // edAutoLeastRetestLimitFileMouseDown :3583; atoi(edAutoRetestLimitFile->Text.c_str()) (was none)
      "edByArmSiteGapCatCT_FT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edByArmSiteGapCatCT_RT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edByBinFailureCatCT_FT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edByBinFailureCatCT_RT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edByBinFailureCatXX_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was none)
      "edByBinSiteGapCatCT_FT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edByBinSiteGapCatCT_RT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContactCountFT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContactCountRT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuPassSkt_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuPassSkt_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuousLoad_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuousLoad_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuousPass_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContinuousPass_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContsFailHeadAlarmCT_FT": ["INTEGER",0,true,1,100000],   // edContsFailHeadAlarmCT_FTMouseDown :3256; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContsFailHeadAlarmCT_RT": ["INTEGER",0,true,1,100000],   // edContsFailHeadAlarmCT_RTMouseDown :3266; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContsFailIgnore_FT": ["INTEGER",0,true,1,100000],   // edContsFailIgnore_FTMouseDown :3573; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContsFailIgnore_RT": ["INTEGER",0,true,1,100000],   // edContsFailIgnore_FTMouseDown :3573; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edContsFailSocketAlarmCT_RT": ["INTEGER",0,true,1,100000],   // edContsFailSocketAlarmCT_RTMouseDown :3246; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edFailYieldMaxRate_ARTFTFile": ["DOUBLE",2,true,100,{"edit":"edFailYieldMinRate_ARTFTFile","f":1,"add":0}],   // edFailYieldMaxRate_ARTFTFileMouseDown :3594; atof(edFailYieldMinRate_ARTFTFile->Text.c_str()) (was none)
      "edFailYieldMaxRate_ARTRTFile": ["DOUBLE",2,true,100,{"edit":"edFailYieldMinRate_ARTRTFile","f":1,"add":0}],   // edFailYieldMaxRate_ARTRTFileMouseDown :3605; atof(edFailYieldMinRate_ARTRTFile->Text.c_str()) (was none)
      "edFailYieldMinRate_ARTFTFile": ["DOUBLE",2,true,0,100],   // edFailYieldRate_ARTFTFileMouseDown :3561; dMinYield=0 (:294); dMaxYield=100 (:295) (was 'DOUBLE', 2, false, 0, 0)
      "edFailYieldMinRate_ARTRTFile": ["DOUBLE",2,true,0,100],   // edFailYieldRate_ARTFTFileMouseDown :3561; dMinYield=0 (:294); dMaxYield=100 (:295) (was 'DOUBLE', 2, false, 0, 0)
      "edFailYieldRate_ARTFTFile": ["DOUBLE",2,true,0,100],   // edFailYieldRate_ARTFTFileMouseDown :3561; dMinYield=0 (:294); dMaxYield=100 (:295) (was 'DOUBLE', 2, false, 0, 0)
      "edFailYieldRate_ARTFile": ["DOUBLE",2,true,0,100],   // edFailYieldRate_ARTFTFileMouseDown :3561; dMinYield=0 (:294); dMaxYield=100 (:295) (was 'DOUBLE', 2, false, 0, 0)
      "edFailYieldRate_ARTRTFile": ["DOUBLE",2,true,0,100],   // edFailYieldRate_ARTFTFileMouseDown :3561; dMinYield=0 (:294); dMaxYield=100 (:295) (was 'DOUBLE', 2, false, 0, 0)
      "edIntervalLowYieldBySite_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edIntervalLowYieldBySite_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edIntervalLowYieldByTotal_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edIntervalLowYieldByTotal_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edLoadCellMeasure": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldByTotalIg_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldByTotalIg_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldByTotal_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edLowYieldByTotal_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edLowYieldIg_FT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldIg_RT": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldIg_Special1": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYieldIg_Special2": ["INTEGER",0,true,1,100000],   // edLowYieldIg_FTMouseDown :3532; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edLowYield_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edLowYield_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edLowYield_Special": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edSiteYieldCmpIg_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edSiteYieldCmpIg_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edSiteYieldCmp_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edSiteYieldCmp_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edSiteYieldDifferentIg_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edSiteYieldDifferentIg_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edSiteYieldDifferent_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edSiteYieldDifferent_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "ed_Alarm4IntervalYieldContinueCount": ["INTEGER",0,true,0,100],   // edAlarm4ContinueCountMouseDown :3628; iMinYield=0 (uYieldMonitoring.cpp:292); iMaxYield=100 (:293) (was 'INTEGER', 0, false, 0, 0)
      "ed_Alarm4IntervalYieldIntervalCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "ed_Alarm4IntervalYieldYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "ed_HeadToHeadYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "ed_HeadToHeadYieldCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "ed_SiteToSiteYield": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "ed_SiteToSiteYieldCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "ed_SiteYieldOverAlert": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "ed_SiteYieldOverAlertCount": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edlContsLowerAlarmMin_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edlContsLowerAlarmMin_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edlContsLowerAlarmNor_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edlContsLowerAlarmNor_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edtLowYieldByPickerIg_FT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edtLowYieldByPickerIg_RT": ["INTEGER",0,true,1,100000],   // edContactCountFTMouseDown :3152; iMinCount=1 (:296); iMaxCount=100000 (:297) (was 'INTEGER', 0, false, 0, 0)
      "edtLowYieldByPicker_FT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
      "edtLowYieldByPicker_RT": ["INTEGER",0,true,0,100],   // edLowYield_RTMouseDown :3162; CosFunction.bYieldAlarmUseDouble=false for CC_PTI (CosFunction.cpp:4351) -> golden :3171 N_INTEGER iMinYield..iMaxYield (0..100, :292-293) (was 'DOUBLE', 2, false, 0, 0)
    }
  };
  function motbBound(v) {                                // number, or golden atoi/atof(edX->Text)+add read now (C: no digits -> 0)
    if (typeof v === 'number') return v;
    if (v && v.edit) { var e = $(v.edit), s = e ? String(e.value) : '', n = v.f ? parseFloat(s) : parseInt(s, 10); return (isNaN(n) ? 0 : n) + (v.add || 0); }
    return 0;
  }
  Object.keys(MOTB[page] || {}).forEach(function (id) {
    if (K[id]) return;
    var k = MOTB[page][id];
    K[id] = function () { return [k[0], k[1] || 0, !!k[2], k[2] ? motbBound(k[3]) : 0, k[2] ? motbBound(k[4]) : 0]; };
  });

  window.HT9045GoldenKb = { page: page, ids: Object.keys(K), unwired: (U[page] || []).map(function (p) { return p[0]; }), kb: function (id) { return K[id] ? K[id](id) : null; } };   // 探針／除錯用
})();
