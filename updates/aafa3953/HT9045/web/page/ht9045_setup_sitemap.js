/* ht9045_setup_sitemap.js -- Setup.SetUp.html 的 Site Mode 行為（手寫，非產生）
 * ---------------------------------------------------------------------------
 * //Steven 20260921
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260921_Steven.md
 * ---------------------------------------------------------------------------
 * 這支把 golden 的四支 handler 搬到瀏覽器：
 *
 *   cSetUp.cpp  TfSetup::ScrollBar1Change()        (232..1196)  主流程
 *   cSetUp.cpp  TfSetup::CompChange(int iMode)     (1197..1264) Site Map 格子
 *   cSetUp.cpp  TfSetup::chkOffCenterkitClick()    (4499..4546) Image1 換圖
 *   cSetUp.cpp  TfSetup::btnLUpToRDownNClick()     (4176..4299) 六顆排序鈕
 *
 * 外加 dfm 的 TScrollBar（Kind=sbVertical, Min=0, Max=8, PageSize=0）
 * —— 產生器只畫了一條靜態的槽和假滑塊，這裡把它升級成真的能拖、能點、
 * 能滾輪、能鍵盤的捲軸，OnChange 接到 scrollBar1Change()。
 *
 * ---------------------------------------------------------------------------
 * 三個資料來源，以及為什麼是這三個
 * ---------------------------------------------------------------------------
 * golden 的每一條分支都在問三種東西，這裡逐一對應：
 *
 *   1. 機台硬體設定  -> system\Gerneral.ini      HT9045System.read('gerneral')
 *      USE_PICKER_COUNT / USE_16_HEATER / INSTALL_DOUBLE_EP /
 *      USE_IN_OUT_ARM_Y_PITCH / ATC 的 USE_ATC_MODE 與 ATC_SYSTEM_USEHEAT /
 *      ROTATE_KIT 的 USE_ROTATE_KIT / bHT9045S_USE2x4 / [Version] SubModel /
 *      [System] CUSTOMER_CODE
 *
 *   2. 功能開關      -> config\config.ini        HT9045System.read('config')
 *      bD30EnableSiteModeSelect / bUseArm1PickPlaceArm2Test（=D58）/
 *      bNeedPasswordWhenEditSiteMap（=G09）/ bL30Use1CableLayoutKitByConfig /
 *      bA32Enable1x4BiasYOffset（=A50，鍵名是 A32，golden 就是這樣）
 *
 *   3. 客戶碼旗標    -> ht9045_setup_cosflags.js（產生檔）
 *      CosFunction.* 與少數 IniConfig.*（bSPILFunction / bVTESTFunction /
 *      bKoreaFunction / bDisableSelectSearchLast / bDualSiteSupply4CH /
 *      bIndexArm2SupplyLight）**完全不在任何 ini 裡**，是 CosFunction.cpp
 *      依 CUSTOMER_CODE 在開機時寫死的。那張表由 scratchpad\gen_setup_cosflags.py
 *      從 golden 抽出來。
 *
 *   4. 32-Site 的氣壓 Sensor -> system\IO_Table.csv  HT9045System.read('ioTable')
 *      golden: bShow32Site = Sen[SnNegativePressureAir2].Enable
 *      （「需安裝兩顆才可以跑 32Site」）。CSV 以 Alias 當 rowKey，
 *      取 Alias=SnNegativePressureAir2 那列的 Enable 欄。
 *
 * ---------------------------------------------------------------------------
 * 已知的界線 —— 這些 golden 有、這裡沒有，是刻意的
 * ---------------------------------------------------------------------------
 * a. MachineTypeChoice。golden 讀的是 D:\GPIB9045\system\general.ini
 *    [Version] Model（GPIB 橋接程式的檔，不是 Handler 的），那個檔不在
 *    wb_serve 的 /api/system 清單裡。這裡改用 Handler 自己的
 *    system\Gerneral.ini [Version] Model 推機種，見 machineFromModel()。
 *    要指定就設 window.HT9045SetupCaps = {machineType: 300} 之類。
 *
 *    HT-9050 golden 根本沒有對應的 eMachineType（V910 / V912 的
 *    database.cpp 都沒有 9050 這一支），所以它的開放範圍是 web 這一層
 *    自己訂的政策，不是翻譯 golden —— 見 MODE_CAP。
 *
 * b. 互鎖與權限不在這裡。golden 的 Barcode_Reader(bcSetup)、
 *    IniConfig.bG09NeedPasswordWhenEditSiteMap 密碼框、fSecurity->Insufficient()
 *    都沒有搬過來 —— 照 ht9045_contact_wire.js 檔頭那條規矩：
 *    「該擋的由 C++ 端擋；兩層互鎖會互相遮蔽」。
 *
 * c. DeviceForm_File.iHeadDeviceCT（CC_ATEC 的 cb2CableLayoutKit 條件）與
 *    TestIF_File.bQualSite2X2Shift 來自配方的 DeviceForm / TestIF，這一頁
 *    沒接那兩份文件。前者一律當 0（條件不成立），後者改看畫面上
 *    cbQualSite2X2Shift 自己的勾選狀態。
 *
 * d. 存檔沒有接。golden 的 sbUpdateClick 會把 [Configuration] Test Mode 與
 *    Site Aa..Dh 寫回 HandlerCondition.Data；這裡只**讀**回來顯示。
 *    存檔鈕仍是 ht9045_wire_engine.js 管的那一顆，它只存它自己對照表裡的欄位。
 *
 * e. 換模式後 Site Map 的格子會變空白 —— 這不是 bug，是 golden 的行為：
 *    CompChange() 對每個 TComboBox 先 Clear() 再重建 Items，VCL 的
 *    ItemIndex 因此回到 -1。六顆排序鈕存在的理由就是讓操作員重填。
 *
 * f. fSetup->Width 818/1090 那段有搬，但只改 .form 的寬度。background.html
 *    沒有「請把我的視窗改寬」這種訊息，桌面視窗框不會跟著變。
 *
 * ---------------------------------------------------------------------------
 * 載入順序（Setup.SetUp.html 的 </body> 之前）
 * ---------------------------------------------------------------------------
 *   ht9045_recipe_client.js   （HT9045Recipe / HT9045System）
 *   ht9045_setup_cosflags.js  （HT9045SetupCos，產生檔）
 *   ht9045_setup_sitemap.js   （本檔）
 *
 * 伺服器連不上時不會整頁壞掉：CAPS 退回 DEFAULT_CAPS（＝golden 的全域初值
 * 加上客戶碼 0），捲軸與六顆鈕照常可用，只是模式的開放範圍用預設值。
 * 這件事會寫到 console，不靜默。
 */
(function (g) {
  'use strict';

  /* ==========================================================================
   * 1. golden 的常數表
   * ========================================================================== */

  /* MachineType.h enum eTestMode */
  var SingleSite = 0, DualSite = 1, TriSite1X3 = 2, QualSite1X4 = 3,
      DualSite2x1 = 4, QualSite2X2 = 5, QualSite2X2N = 6, _6Site2X3 = 7,
      _6Site2X3N = 8, _8Site2X4 = 9, _8Site2X4N = 10, _10Site2X5 = 11,
      _12Site2X6 = 12, _16Site2X8 = 13, _16Site4X4 = 14, _32Site4X8N = 15,
      _32Site4X8M = 16, _8Site1X4 = 17;

  /* cmydef.cpp TestSiteFileName[0][] / [1][] ＋ cmydef.cpp SiteData[].SetData(X,Y)
   * name -> Panel1 的 Caption；png -> Image1（gen_setup_siteimgs.py 轉出來的）
   * x/y  -> SiteData[].XItem / YItem；cnt = x*y */
  var MODES = [
    { name: 'Single Site',                png: '1site',          x: 1, y: 1 },
    { name: '2-Site',                     png: '2site',          x: 2, y: 1 },
    { name: '3-Site (1x3)',               png: '3site',          x: 3, y: 1 },
    { name: 'In-Line 4-Site(1X4)',        png: '4site',          x: 4, y: 1 },
    { name: '2-Site (2x1)',               png: '2site2x1',       x: 1, y: 2 },
    { name: 'Square 4-Site(2X2)',         png: '4siteRow',       x: 2, y: 2 },
    { name: 'Square 4-Site(2X2) NN Mode', png: '2x2siteRow_NN',  x: 2, y: 2 },
    { name: '6-Site',                     png: '6Site',          x: 3, y: 2 },
    { name: '6-Site (2x3) NN Mode',       png: '2x3site_NN',     x: 3, y: 2 },
    { name: '8-Site',                     png: '8Site',          x: 4, y: 2 },
    { name: '8-Site (2x4) NN Mode',       png: '2x4site_NN',     x: 4, y: 2 },
    { name: '10-Site',                    png: '10Site',         x: 5, y: 2 },
    { name: '12-Site',                    png: '12Site',         x: 6, y: 2 },
    { name: '16-Site',                    png: '16Site',         x: 8, y: 2 },
    { name: '16-Site (4X4)',              png: '16Site4X4',      x: 4, y: 4 },
    { name: '32-Site N Mode',             png: '32SiteN',        x: 8, y: 4 },
    { name: '32-Site M Mode',             png: '32SiteM',        x: 8, y: 4 },
    // golden 的表是 '8-Site Pop.bmp'，機台的 D:\HT9045\IMG\BMP 底下沒有這個檔
    // （gen_setup_siteimgs.py 會把它列在 NOT FOUND）。沿用上一張圖，不整頁壞掉。
    { name: '8-Site Pop',                 png: '8-Site_Pop',     x: 4, y: 1 }
  ];
  MODES.forEach(function (m) { m.cnt = m.x * m.y; });

  var MAX_SOCKET_ROW = 4, MAX_SOCKET_COL = 8;

  /* MachineType.h enum eMachineType */
  var Type_HT9045 = 100, Type_HT9046 = 200, Type_HT9046_LS = 300,
      Type_HT9045_12Site = 400, Type_HT502 = 500, Type_HT1032 = 600,
      Type_HT7080 = 700;

  /* ⚠ Type_HT9050 不是 golden 的 eMachineType。
   *   golden 的值域是 100..700（MachineType.h:424），所以 9050 不會撞號；
   *   web 這一層用它掛「HT-9050 這台目前開放到哪裡」這條政策（見 MODE_CAP）。
   *   哪天 Jimmy 在 C++ 那邊決定 HT-9050 對到哪一個 eMachineType，
   *   machineFromModel() 改成回那一支、MODE_CAP 拿掉 9050 這一列就好。 */
  var Type_HT9050 = 9050;

  /* 機種 -> 這一版 web 最多開放到哪個 Site Mode。
   * golden 的 iMax 是機種 ＋ 氣壓 Sensor ＋ USE_PICKER_COUNT 算出來的（見 maxMode()）；
   * 這張表是**額外**的上限，只會往下壓、不會往上放。
   *
   * Steven 20260921（使用者裁決）：HT-9050 預設先只支援 1x1 與 1x2。
   *   理由是 golden 沒有 HT-9050 的規則可以翻譯，與其拿別的機種的 iMax 硬套，
   *   不如先只開已經確定會跑的兩個模式；其餘的等機構與 C++ 那邊確認再逐一放。
   *   要臨時放寬：window.HT9045SetupCaps = { modeCap: 13 }（13 = _16Site2X8），
   *   或直接指定機種 window.HT9045SetupCaps = { machineType: 300 }。 */
  var MODE_CAP = {};
  MODE_CAP[Type_HT9050] = DualSite;                   // 1x1 / 1x2
  /* enum eSubMachineType */
  var Type_HT9046LA = 1, Type_HT9016C = 2;
  /* enum eHeaterType */
  var eht4Heater = 0, eht16Heater = 1, eht16HeaterEJ1N = 2, eht32HeaterEJ1N = 3,
      eht32HeaterKT4H = 4, eht16HeaterDTME08 = 5, eht32HeaterDTME08 = 6;
  /* enum eATCType */
  var eATC30 = 2, eNewATCSystem = 6;
  /* cmydef.cpp 的 XY 變距模式 */
  var iXPitchManual360 = 3, iXYPitch16Picker = 5, iXYPitch16Bd_Be = 7;
  /* cmydef.h #define */
  var DOUBLE_EP_INDIVIAL = 2, DOUBLE_EP_MULTI = 3;
  /* MachineType.h #define CC_* —— 只列 cSetUp.cpp 真的比對到的幾個 */
  var CC_HONPREC_QC = 0, CC_TSMC_TAINAN = 820, CC_HANA_MICRON = 865,
      CC_ATEC = 891, CC_ASE_KaohSiung = 936, CC_SCC = 943, CC_SCS = 944,
      CC_SIGURD_PeiXing = 946, CC_SCK = 947, CC_AMKOR_China = 972,
      CC_QUALCOMM = 999;

  /* ==========================================================================
   * 2. DOM 小工具
   * ========================================================================== */

  function $(id) { return document.getElementById(id); }

  /* VCL Visible。用 display 而不是 visibility —— VCL 的 Visible=false 不佔位。 */
  function vis(id, on) {
    var el = (typeof id === 'string') ? $(id) : id;
    if (el) el.style.display = on ? '' : 'none';
    return el;
  }
  function isVis(id) {
    var el = (typeof id === 'string') ? $(id) : id;
    return !!el && el.style.display !== 'none';
  }
  /* VCL Enabled。select/input/button 用原生 disabled；容器用 class 讓 CSS 變灰
   * 並吃掉點擊（沒有 class 對應也不會壞，只是看不出灰）。 */
  function ena(id, on) {
    var el = (typeof id === 'string') ? $(id) : id;
    if (!el) return null;
    if ('disabled' in el) { el.disabled = !on; return el; }
    el.classList.toggle('disabled', !on);
    el.style.pointerEvents = on ? '' : 'none';
    el.style.opacity = on ? '' : '0.55';
    el.querySelectorAll('input,select,button,textarea').forEach(function (c) {
      c.disabled = !on;
    });
    return el;
  }
  /* TPanel 的 Caption。產生器把它放在 .pnlCap 這個 span 裡。 */
  function caption(id, text) {
    var el = $(id);
    if (!el) return;
    var cap = el.querySelector(':scope > .pnlCap');
    if (cap) cap.textContent = text; else el.textContent = text;
  }
  /* TCheckBox 的 Checked。產生器把 TCheckBox 畫成 label.ckb > input[type=checkbox]。 */
  function chk(id, v) {
    var el = $(id), inp = el && el.querySelector('input');
    if (!inp) return false;
    if (v !== undefined) inp.checked = !!v;
    return inp.checked;
  }
  /* TCheckBox 的 Caption：label 裡最後一個文字節點。 */
  function chkCaption(id, text) {
    var el = $(id);
    if (!el) return;
    for (var i = el.childNodes.length - 1; i >= 0; i--) {
      if (el.childNodes[i].nodeType === 3) { el.childNodes[i].nodeValue = text; return; }
    }
    el.appendChild(document.createTextNode(text));
  }
  /* TRadioGroup 的 ItemIndex / Items->Strings[n]。
   * 產生器畫成 fieldset.rg > .cli > label.rgi > input[type=radio]。 */
  function rgItems(id) {
    var el = $(id);
    return el ? el.querySelectorAll('label.rgi') : [];
  }
  function rgIndex(id, v) {
    var its = rgItems(id), i;
    if (v !== undefined) {
      for (i = 0; i < its.length; i++) its[i].querySelector('input').checked = (i === v);
      return v;
    }
    for (i = 0; i < its.length; i++) {
      if (its[i].querySelector('input').checked) return i;
    }
    return -1;
  }
  function rgText(id, n, text) {
    var its = rgItems(id);
    if (!its[n]) return;
    for (var i = its[n].childNodes.length - 1; i >= 0; i--) {
      if (its[n].childNodes[i].nodeType === 3) { its[n].childNodes[i].nodeValue = text; return; }
    }
  }
  /* TEdit 的 Text */
  function text(id, v) {
    var el = $(id);
    if (!el) return '';
    if (v !== undefined) el.value = String(v);
    return el.value;
  }

  /* TestSiteCH[row][col] -> cbAa..cbDh；TestLabRow/TestLabCol -> labRowA..D / labColA..H
   * （golden cSetUp.cpp 建構式的 tempTestSiteCBox / tempTestLabRow / tempTestLabCol） */
  var ROWCH = 'ABCD', COLCH = 'abcdefgh';
  var COLLAB = 'ABCDEFGH';          // ⚠ 欄標籤用大寫（labColA..labColH），
                                    //   格子 id 的欄卻是小寫（cbAa..cbAh）。
  function siteCH(r, c) { return $('cb' + ROWCH[r] + COLCH[c]); }
  function labRow(r) { return $('labRow' + ROWCH[r]); }
  function labCol(c) { return $('labCol' + COLLAB[c]); }

  /* ==========================================================================
   * 3. 機台能力（CAPS）
   * ========================================================================== */

  /* 伺服器連不上時用的保底值：golden 的全域初值 ＋ 客戶碼 0（鴻勁自用）。
   * 刻意不「全開」—— 全開會讓操作員在畫面上看到機台其實跑不了的模式。 */
  var DEFAULT_CAPS = {
    machineType: Type_HT9045,
    subMachineType: 0,
    customerCode: CC_HONPREC_QC,
    usePickerCount: 1,          // Gerneral [System] USE_PICKER_COUNT, 預設 1
    ht9045SUse2x4: false,
    use16Heater: eht4Heater,    // Gerneral [System] USE_16_HEATER, 預設 eht4Heater
    atcSystem: 0,               // Gerneral [ATC] USE_ATC_MODE, 預設 eATCUninstall
    atcUseHeatCount: 0,         // Gerneral [ATC] ATC_SYSTEM_USEHEAT
    installDoubleEp: 0,
    useRotateKit: 0,
    useInOutArmYPitch: 0,       // iXPitch60
    show32Site: false,
    headDeviceCT: 0,            // 見檔頭界線 c
    modeCap: null,              // web 這一層的額外上限；null = 用 MODE_CAP[機種]
    cos: null,                  // 由 HT9045SetupCos.forCode() 填
    ini: {
      bD30EnableSiteModeSelect: false,
      bD58UseArm1PickPlaceArm2Test: false,
      bG09NeedPasswordWhenEditSiteMap: false,
      bL30Use1CableLayoutKitByConfig: false,
      bA50Enable1x4BiasYOffset: false
    }
  };

  var CAPS = null;

  /* golden 讀的是 GPIB 橋接程式那支 general.ini 的 Model（9045GPIB /
   * 9046_32GPIB / 1032GPIB ...），web 這側拿不到，改用 Handler 自己的
   * system\Gerneral.ini [Version] Model（HT-9045 / HT-9050 / HT-9046LS ...）。
   *
   * ⚠ HT-9050 在 golden 裡沒有對應：
   *   這台開發機的 Gerneral.ini 是 Model=HT-9050，而 D:\GPIB9045 那支的
   *   Model=9050GPIB —— V910 與 V912 的 database.cpp 都沒有 9050 這一支，
   *   所以 golden 走的是 bHandlerModel=false 那條「型號讀取失敗要 Alarm」，
   *   MachineTypeChoice 留在全域初值 Type_HT9045、iMax 變成 _8Site2X4。
   *   web 這邊不套用那個意外的結果，改成回 Type_HT9050 並由 MODE_CAP 決定
   *   開放範圍（目前 1x1 / 1x2）。
   *
   * ⚠ 其餘認不出來的型號回 Type_HT9045 —— 就是 golden 在
   *   bHandlerModel==false 時留下的全域初值。夾限時會 console.warn，
   *   不會靜默改掉操作員的模式。
   */
  /* 只給 console 訊息用 —— 「機種=9050」這種數字自己看不出是什麼。 */
  function machineName(mt) {
    var n = { 100: 'HT9045', 200: 'HT9046', 300: 'HT9046_LS', 400: 'HT9045_12Site',
              500: 'HT502', 600: 'HT1032', 700: 'HT7080', 9050: 'HT9050(web)' }[mt];
    return (n ? n + '(' + mt + ')' : String(mt));
  }

  function machineFromModel(model) {
    var s = String(model || '').toUpperCase().replace(/[\s_-]/g, '');
    if (/12SITE/.test(s)) return Type_HT9045_12Site;
    if (/9050/.test(s)) return Type_HT9050;
    if (/1032/.test(s)) return Type_HT1032;
    if (/7080/.test(s)) return Type_HT7080;
    if (/502/.test(s)) return Type_HT502;
    if (/9046/.test(s)) return (/LS|32/.test(s)) ? Type_HT9046_LS : Type_HT9046;
    return Type_HT9045;                       // 9045 與其他未列入的型號
  }

  /* ini 取值：sections[sec][key] 可能是字串，也可能是 {raw:..} / {value:..} */
  function iniRaw(doc, sec, key) {
    if (!doc || !doc.sections) return null;
    var s = doc.sections[sec];
    if (!s || !(key in s)) return null;
    var v = s[key];
    if (v === null || v === undefined) return null;
    if (typeof v === 'object') return (v.raw !== undefined && v.raw !== null) ? String(v.raw) : String(v.value);
    return String(v);
  }
  function iniInt(doc, sec, key, dflt) {
    var r = iniRaw(doc, sec, key);
    if (r === null || r === '') return dflt;
    var n = parseInt(String(r).trim(), 10);
    return isNaN(n) ? dflt : n;
  }
  function iniBool(doc, sec, key, dflt) {
    var r = iniRaw(doc, sec, key);
    if (r === null || r === '') return dflt;
    r = String(r).trim().toLowerCase();
    return (r === '1' || r === 'true' || r === 'yes');
  }

  /* IO_Table.csv：以 Alias 當 rowKey（wb_serve CsvKeyColumn）。
   * 一列可能是 {Alias:'..', Enable:'1'} 也可能是 {key:'..', cells:{..}}，
   * 兩種形狀都認，因為投影格式改過一次。 */
  function csvCell(row, col) {
    if (!row) return null;
    var v = (row[col] !== undefined) ? row[col]
          : (row.cells && row.cells[col] !== undefined) ? row.cells[col] : null;
    if (v === null || v === undefined) return null;
    if (typeof v === 'object') return (v.raw !== undefined && v.raw !== null) ? String(v.raw) : String(v.value);
    return String(v);
  }
  function sensorEnabled(csv, alias) {
    if (!csv || !csv.rows) return false;
    for (var i = 0; i < csv.rows.length; i++) {
      var r = csv.rows[i];
      var a = csvCell(r, 'Alias');
      if (a === null && r.key !== undefined) a = String(r.key);
      if (a === alias) return String(csvCell(r, 'Enable') || '').trim() === '1';
    }
    return false;
  }

  function cosFor(code) {
    if (g.HT9045SetupCos) return g.HT9045SetupCos.forCode(code);
    console.warn('[Setup/SiteMap] 沒有載入 ht9045_setup_cosflags.js —— '
               + '客戶碼旗標全部當 false，部分 Site Mode 會被擋住。');
    return {};
  }

  function loadCaps() {
    var caps = JSON.parse(JSON.stringify(DEFAULT_CAPS));
    if (typeof g.HT9045System === 'undefined') {
      caps.cos = cosFor(caps.customerCode);
      console.warn('[Setup/SiteMap] 沒有 HT9045System（ht9045_recipe_client.js 未載入），'
                 + '機台設定改用保底值。');
      applyOverride(caps);
      return Promise.resolve(caps);
    }
    var sys = g.HT9045System;
    var soft = function (p) { return p.then(function (r) { return r; },
                                            function (e) { return { __err: e }; }); };
    return Promise.all([soft(sys.read('gerneral')), soft(sys.read('config')),
                        soft(sys.read('ioTable'))])
      .then(function (r) {
        var gen = r[0], cfg = r[1], io = r[2], bad = [];

        if (gen && !gen.__err) {
          caps.machineType       = machineFromModel(iniRaw(gen, 'Version', 'Model'));
          caps.subMachineType    = iniInt(gen, 'Version', 'SubModel', 0);
          caps.customerCode      = iniInt(gen, 'System', 'CUSTOMER_CODE', CC_HONPREC_QC);
          caps.usePickerCount    = iniInt(gen, 'System', 'USE_PICKER_COUNT', 1);
          caps.ht9045SUse2x4     = iniBool(gen, 'System', 'bHT9045S_USE2x4', false);
          caps.use16Heater       = iniInt(gen, 'System', 'USE_16_HEATER', eht4Heater);
          caps.installDoubleEp   = iniInt(gen, 'System', 'INSTALL_DOUBLE_EP', 0);
          caps.useInOutArmYPitch = iniInt(gen, 'System', 'USE_IN_OUT_ARM_Y_PITCH', 0);
          caps.atcSystem         = iniInt(gen, 'ATC', 'USE_ATC_MODE', 0);
          // golden 的 iATC_Use_Heat_Count 是 ATC 連線後才填的執行期值，
          // 來源就是 HSys.asATCSYSTEMUSEHEAT ＝ 這個鍵。沒連線時用設定值最接近。
          caps.atcUseHeatCount   = iniInt(gen, 'ATC', 'ATC_SYSTEM_USEHEAT', 0);
          caps.useRotateKit      = iniInt(gen, 'ROTATE_KIT', 'USE_ROTATE_KIT', 0);
        } else { bad.push('Gerneral.ini'); }

        if (cfg && !cfg.__err) {
          caps.ini.bD30EnableSiteModeSelect =
            iniBool(cfg, 'Index', 'bD30EnableSiteModeSelect', false);
          caps.ini.bD58UseArm1PickPlaceArm2Test =
            iniBool(cfg, 'Index', 'bUseArm1PickPlaceArm2Test', false);
          caps.ini.bG09NeedPasswordWhenEditSiteMap =
            iniBool(cfg, 'Function', 'bNeedPasswordWhenEditSiteMap', false);
          caps.ini.bL30Use1CableLayoutKitByConfig =
            iniBool(cfg, 'Tempture', 'bL30Use1CableLayoutKitByConfig', false);
          // golden 的成員叫 bA50...，config.ini 的鍵卻是 bA32...（cConfiguration.cpp
          // 那一行就是這樣寫的）。照鍵名讀，不要「順手改成 A50」。
          caps.ini.bA50Enable1x4BiasYOffset =
            iniBool(cfg, 'Function', 'bA32Enable1x4BiasYOffset', false);
        } else { bad.push('config.ini'); }

        if (io && !io.__err) {
          caps.show32Site = sensorEnabled(io, 'SnNegativePressureAir2');
        } else { bad.push('IO_Table.csv'); }

        caps.cos = cosFor(caps.customerCode);
        if (g.HT9045SetupCos && !g.HT9045SetupCos.known(caps.customerCode)) {
          console.warn('[Setup/SiteMap] 客戶碼 ' + caps.customerCode
                     + ' 不在 CosFunction.cpp 的 switch 裡，旗標用 DEFAULTS。');
        }
        if (bad.length) {
          console.warn('[Setup/SiteMap] 讀不到 ' + bad.join(' / ')
                     + '，這幾項用保底值。Site Mode 的開放範圍可能與機台不同。');
        }
        applyOverride(caps);
        return caps;
      });
  }

  /* 人工覆寫：window.HT9045SetupCaps 的鍵一律蓋掉自動判讀的結果。
   * 最常用的就是 machineType（見 machineFromModel 的說明）。
   * 覆寫了什麼一定寫到 console —— 覆寫本身很容易變成下一個查不出來的怪現象。 */
  function applyOverride(caps) {
    var ov = g.HT9045SetupCaps;
    if (!ov) return;
    Object.keys(ov).forEach(function (k) {
      if (k === 'ini' && ov.ini) {
        Object.keys(ov.ini).forEach(function (j) { caps.ini[j] = ov.ini[j]; });
      } else if (k === 'cos' && ov.cos) {
        Object.keys(ov.cos).forEach(function (j) { caps.cos[j] = ov.cos[j]; });
      } else {
        caps[k] = ov[k];
      }
    });
    console.info('[Setup/SiteMap] 套用 window.HT9045SetupCaps 覆寫：'
               + Object.keys(ov).join(', '));
  }

  /* ==========================================================================
   * 4. golden: CompChange(int iMode)      cSetUp.cpp 1197..1264
   * ========================================================================== */
  function compChange(iMode) {
    var m = MODES[iMode], cos = CAPS.cos || {};
    var iTestCHCT = m.cnt;

    if (cos.bDualSiteSupply4CH === true && iMode === DualSite) {
      iTestCHCT += 2;                                   // jou 2012-11-20
    }

    if (cos.bSPILFunction === true || CAPS.customerCode === CC_ASE_KaohSiung) {
      if (iMode === _8Site2X4 && cos.bEnableOctal_12Kit === true && chk('cbOctal12Site')) {
        iTestCHCT += 4;
      } else if (iMode === _8Site2X4 && cos.bEnableOctal_12Kit === true && !chk('cbOctal12Site')) {
        iTestCHCT = m.cnt;
      }
    }

    if (cos.bUse32ChanelSiteMap) {                      // Steven 20170530
      iTestCHCT = 32;
      if (cos.bVTESTFunction) iTestCHCT = 16;           // RogerYang 20260529
    }

    var i, j, k, el;
    for (i = 0; i < MAX_SOCKET_ROW; i++) {
      vis(labRow(i), false);
      for (j = 0; j < MAX_SOCKET_COL; j++) {
        el = siteCH(i, j);
        if (el) { el.innerHTML = ''; vis(el, false); }  // TComboBox::Clear() -> ItemIndex 回 -1
        vis(labCol(j), false);
      }
    }

    for (i = 0; i < m.x; i++) {                         // SiteData[iMode].XItem
      vis(labCol(i), true);
      for (j = 0; j < m.y; j++) {                       // SiteData[iMode].YItem
        el = siteCH(j, i);
        if (!el) continue;
        vis(el, true);
        ena(el, true);
        for (k = 0; k <= iTestCHCT; k++) {
          var o = document.createElement('option');
          o.value = String(k);
          o.textContent = (k === 0) ? '- - -' : ('CH ' + k);
          el.appendChild(o);
        }
        el.selectedIndex = -1;                          // VCL Clear() 之後就是 -1
        vis(labRow(j), true);
      }
    }
    return iTestCHCT;
  }

  /* ==========================================================================
   * 5. golden: chkOffCenterkitClick()     cSetUp.cpp 4499..4546
   *    （Image1 換圖；ScrollBar1Change 一開頭就呼叫它）
   * ========================================================================== */
  function setImage(png) {
    var el = $('Image1');
    if (el) el.setAttribute('src', 'img/dfm_' + png + '.png');
  }
  function modePng(pos) { return (MODES[pos] || MODES[0]).png; }
  /* AI(W906-Q41) 20260927 (St02-E): Q41 SU-1..SU-6 的頁面半邊（golden 912 cSetUp.cpp；存檔時 FileRW/TestIF_File_SetUp.cpp BeforeApply (4) 照 golden 重播，伺服器才是準）。q41Ena：golden 直接設 Enabled；上層容器被引擎停用（aria-disabled）時 VCL 一樣按不到 → 不打開 */ function q41Ena(id, on) { var el = $(id); if (!el) return; if (on) { for (var p = el.parentElement; p; p = p.parentElement) { if (p.getAttribute && p.getAttribute('aria-disabled') === 'true') return; } } ena(el, on); } function rgShtModeNormalClick() { /* SU-1 golden :1272-1290（rgShtModeNormal／rgShtModeOneSide 兩顆 TRadioButton 同一支） */ if (chk('rgShtModeOneSide') === false) { chk('rgUseSht1', false); chk('rgUseSht2', false); q41Ena('rgUseSht1', false); q41Ena('rgUseSht2', false); q41Ena('rgUseSuckMode', true); } else { chk('rgUseSht1', true); q41Ena('rgUseSht1', true); q41Ena('rgUseSht2', true); rgIndex('rgUseSuckMode', 0); q41Ena('rgUseSuckMode', false); } }
  function chkOffCenterkitClick() {
    var iCheckPos = position(), cos = CAPS.cos || {}, ini = CAPS.ini;

    if (cos.b2x4SupportCenterPitch === true && iCheckPos === _8Site2X4) {
      // golden 這裡還有 FileExists()；web 端換成「圖存在與否交給 <img> 自己」，
      // 轉檔腳本已經把 8siteCenterX 轉出來了。
      setImage(chk('chkUseXCenterPitch') ? '8siteCenterX' : modePng(iCheckPos));
    } else if (chk('chkOffCenterkit') === false) {
      setImage(modePng(iCheckPos));
      rgIndex('rgYOffset', 0);
      vis('rgYOffset', false);
    } else {
      rgIndex('rgYOffset', 0);
      vis('rgYOffset', false);
      switch (iCheckPos) {
        case SingleSite:  setImage('1siteOffCentre'); break;
        case DualSite:    setImage('2siteOffCentre'); break;
        case QualSite1X4:
          if (cos.bNonCenterModeCanUseShtOffset && ini.bA50Enable1x4BiasYOffset
              && chk('chkOffCenterkit')) {
            vis('rgYOffset', true);
          }
          setImage('4siteOffCentre');
          break;
        default:          setImage(modePng(iCheckPos));
      }
    }
  }
  /* AI(W906-Q41) 20260927 (St02-E) */ function rgUseSuckModeClick() { /* SU-2 golden :3447-3474 */ var p = position(); if (p === DualSite || p === QualSite1X4 || p === _8Site1X4) { if (rgIndex('rgUseSuckMode') === 1) searchLastByConfig(CAPS.cos || {}); else vis('rgSelectSearchLast', false); } } function arm1PickArm2TestClick() { /* SU-3 golden :4497-4507（三顆勾選框都只看 Arm1PickArm2Test，照翻） */ if (chk('Arm1PickArm2Test')) vis('gbShuttleMode', false); else vis('gbShuttleMode', !!(CAPS.ini && CAPS.ini.bD30EnableSiteModeSelect)); } function cbUseSLKClampClick() { /* SU-4 golden :4670-4676（cbUseTesterDry 也綁這支，只看 cbUseSLKClamp，照翻） */ q41Ena('rgseparabilityTest', chk('cbUseSLKClamp') === true); } function q41FileYOfs() { /* golden TestIF_File.dSiteYOffset：開頁模式不是 NN 時＝開頁的 edYOffset；NN 開頁時頁面不知道 → null（存檔時伺服器照 golden 補） */ var g = window.HT9045Page && HT9045Page.golden ? HT9045Page.golden() : null, px = g && g.page && g.page.proxies; if (!px || !px.edYOffset || !px.rgYPitchOffsetMode || px.rgYPitchOffsetMode.itemIndex === 1) return null; return px.edYOffset.text; } function rgYPitchOffsetModeClick() { /* SU-6 golden :4812-4835（labYOffset->Width 不用） */ var i = rgIndex('rgYPitchOffsetMode'), lab = $('labYOffset'); if (i === 1) { if (lab) lab.textContent = 'Y Offset (mm)'; q41Ena('edYOffset', false); text('edYOffset', 10); } else { if (lab) lab.textContent = (i === 0) ? 'Y Offset (mm)' : 'Y Center Pitch (mm)'; var f = q41FileYOfs(); if (f !== null && f !== undefined) text('edYOffset', f); q41Ena('edYOffset', true); } }
  /* golden: cbQualSite2X2ShiftClick()     cSetUp.cpp 4548..4584 */
  function cbQualSite2X2ShiftClick() {
    var iCheckPos = position();
    if (!chk('cbQualSite2X2Shift')) {
      setImage(chk('chkOffCenterkit') ? '2siteOffCentre' : modePng(iCheckPos));
      vis('paQualSite2X2Shift', false);
      return;
    }
    if (iCheckPos === QualSite2X2) {
      setImage('4siteRowOffCentre');
      vis('paQualSite2X2Shift', true);
      text('XShiftPitch', '40.0');
    } else if (iCheckPos === DualSite) {
      setImage(chk('chkOffCenterkit') ? '2siteOffCentre' : modePng(iCheckPos));
      vis('paQualSite2X2Shift', true);
      text('XShiftPitch', '-2000');
    } else {
      setImage(modePng(iCheckPos));
      vis('paQualSite2X2Shift', false);
    }
  }

  /* ==========================================================================
   * 6. golden: ScrollBar1Change()         cSetUp.cpp 232..1196
   * ========================================================================== */

  var OrgTestMode = 0;            // golden 的同名全域（cSetUp.cpp:120）
  var bFirstRead = true;          // golden 的 static
  var asHandlingMode = '';        // 程式開啟時的 Handler Mode（Ifor 20161117）
  var iSiteTotal = 0;             // kevin 20160125
  var bUseTwoArm32Site = false;   // kevin 20190322
  var reentrant = false;          // 取代 golden 靠 return 擋住的二次觸發
  var FORM_W = null;              // 產生器給的 .form 原始寬（= golden 的 1090 客戶區）

  /* golden 的 iMax 計算（含 HT9045S 鎖 2x2），最後再壓上 web 這一層的 MODE_CAP。 */
  function maxMode() {
    var iMax;
    if (CAPS.machineType === Type_HT9045) {
      iMax = _8Site2X4;
    } else if ((CAPS.machineType === Type_HT9046_LS || CAPS.machineType === Type_HT1032)
               && CAPS.show32Site === true) {
      iMax = _32Site4X8N;                               // 需安裝兩顆氣壓 Sensor
    } else if (CAPS.machineType === Type_HT9045_12Site) {
      iMax = _12Site2X6;
    } else {
      iMax = _16Site2X8;
    }
    if (CAPS.usePickerCount === 0 && CAPS.ht9045SUse2x4 === false) {
      iMax = QualSite2X2;                               // 鎖住 HT9045S 僅可跑 2*2
    }

    /* 到這裡為止都是 golden。以下是 web 這一層的額外上限（只往下壓）。
     * 兩個來源：window.HT9045SetupCaps.modeCap（人工覆寫）優先於 MODE_CAP[機種]。 */
    var cap = (CAPS.modeCap !== undefined && CAPS.modeCap !== null)
            ? CAPS.modeCap : MODE_CAP[CAPS.machineType];
    if (cap !== undefined && cap !== null && cap < iMax) iMax = cap;

    if (iMax < 0) iMax = SingleSite;                    // 再怎麼壓也要留 1x1
    return iMax;
  }

  /* golden 那一大串 if 的正面說法：這個模式現在能不能選。
   * 回 false 代表要跳過（golden 用 iPos++/iPos-- 往同方向再推一格）。 */
  function modeAllowed(iPos) {
    var cos = CAPS.cos || {}, mt = CAPS.machineType, sub = CAPS.subMachineType;

    var nnBlocked =
      (!(mt === Type_HT9046_LS || mt === Type_HT1032) ||
       (mt === Type_HT9046_LS && sub === Type_HT9016C) ||   // HT9016C 不可以跑 NN Mode
       (mt === Type_HT9046_LS && sub === Type_HT9046LA)) && // 9046LA 不可以跑 NN Mode
      (iPos === _32Site4X8N || iPos === _32Site4X8M || iPos === _16Site4X4 ||
       iPos === QualSite2X2N || iPos === _6Site2X3N || iPos === _8Site2X4N);

    if (nnBlocked) return false;
    if (mt === Type_HT9045 && iPos >= _10Site2X5) return false;
    if (mt === Type_HT9045_12Site && iPos >= _16Site2X8) return false;
    if (iPos === DualSite2x1 && cos.bEnable2x1Site === false) return false;
    if (iPos === _12Site2X6 && cos.bEnable12Site === false) return false;
    if (iPos === _6Site2X3 && cos.bEnable6Site === false) return false;
    if (iPos === _6Site2X3 && CAPS.usePickerCount === 0) return false;    // HT-9045S
    if (iPos === TriSite1X3 && CAPS.usePickerCount === 0) return false;   // HT-9045S
    if (iPos === QualSite2X2N && cos.bCanUse2x2NNMode === false) return false;
    if (iPos === _6Site2X3N && cos.bCanUse2x3NNMode === false) return false;
    if (iPos === _8Site2X4N && cos.bCanUse2x4NNMode === false) return false;
    return true;
  }

  /* golden 的 cb16DirectHeater 條件，12 個 case 各抄一次 —— 這裡收成一支。 */
  function direct16Visible(withDTME08) {
    var h = CAPS.use16Heater;
    return (h === eht32HeaterEJ1N || h === eht32HeaterKT4H ||
            (CAPS.atcSystem === eNewATCSystem && CAPS.atcUseHeatCount >= 16) ||
            (withDTME08 !== false && h === eht32HeaterDTME08));
  }
  /* golden 的「16 溫控器家族」條件 */
  function heater16Family() {
    var h = CAPS.use16Heater;
    return (h === eht16Heater || h === eht16HeaterEJ1N || h === eht32HeaterEJ1N ||
            h === eht32HeaterKT4H || h === eht16HeaterDTME08 || h === eht32HeaterDTME08);
  }
  /* golden 的 rgSelectSearchLast 三行樣板 */
  function searchLastByConfig(cos) {
    if (cos.bDisableSelectSearchLast === true) {
      vis('rgSelectSearchLast', false);
      rgIndex('rgSelectSearchLast', 0);
    } else {
      vis('rgSelectSearchLast', true);
    }
  }

  function scrollBar1Change() {
    if (reentrant) return;
    var cos = CAPS.cos || {}, ini = CAPS.ini;

    vis('gbShuttleMode', true);                         // kevin 20210813
    // golden: CosFunction.bDisableOpenAllSiteWhenChangeShtMod -> shtMode.Clear()
    // shtMode 是執行期的 Shuttle 模式暫存，這一頁沒有它，不搬。

    // golden 這裡還有一個 TestIF_File.bForEgisTecTest（Arm2 當指紋測試）。
    // 那是配方 TestIF 的值，這一頁沒接那份文件 -> 一律當 false。
    // 影響面：開了指紋測試的機台，這裡的 gbShuttleMode 會多顯示出來。
    if (cos.bIndexArm2SupplyLight === true ||
        (chk('Arm1PickArm2Test') && ini.bD58UseArm1PickPlaceArm2Test === true)) {
      vis('gbShuttleMode', false);                      // only use shuttle 1
    } else {
      vis('gbShuttleMode', ini.bD30EnableSiteModeSelect);
    }

    var iPos = position();
    var iMax = maxMode();

    if (iPos >= iMax) {
      setPosition(iMax, true);
      OrgTestMode = iPos;
      iPos = iMax;
    }

    /* golden 是「改了 Position 就 return，靠 OnChange 再被觸發一次」。
     * 這裡不遞迴，直接往同方向推到下一個能用的模式（結果等價，但不會
     * 在客戶碼把整段都關掉時無限來回）。 */
    if (!modeAllowed(iPos)) {
      var dir = (OrgTestMode < iPos) ? 1 : -1;
      var p = iPos, guard = 0;
      do {
        p += dir;
        if (p < 0) p = 1;                               // Steven 20120816：修正小於 1 的 Error
        if (p > iMax) { dir = -1; p = iMax; }
      } while (!modeAllowed(p) && ++guard < MODES.length * 2);
      if (!modeAllowed(p)) {
        console.warn('[Setup/SiteMap] 依目前機種／客戶碼，沒有任何 Site Mode 可選（客戶碼 '
                   + CAPS.customerCode + '、機種 ' + machineName(CAPS.machineType) + '）。');
        return;
      }
      OrgTestMode = p;
      reentrant = true;
      try { setPosition(p, true); } finally { reentrant = false; }
      iPos = p;
    }

    var iCheckPos = position();
    OrgTestMode = iCheckPos;

    chkOffCenterkitClick();                             // golden 就是在這個位置呼叫
    caption('Panel1', MODES[iCheckPos].name);
    compChange(iCheckPos);

    if (bFirstRead === true) {                          // Ifor 20161117
      asHandlingMode = MODES[iCheckPos].name;
      bFirstRead = false;
    }

    /* ---- golden 的「先全部關掉」區塊（cSetUp.cpp 365..413） ---------------- */
    ena('cbAa', true);
    vis('XPitch', true); vis('lblXPitch', true); vis('lblXPMM', true);
    vis('cbNS8000H', false);
    vis('chkOctal80', false);
    vis('cb2CableLayoutKit', false);
    vis('cb1CableLayoutKit', false);
    vis('cb6CableLayoutKit', false);
    vis('cbOctal16Site', false);
    vis('chk12SiteUse2x8SLK', false);
    vis('cbSquareOctalLayout', false);
    vis('chk2x2Use16siteSLK', false);
    vis('cb1x2Use1x4siteSLK', false);
    vis('cbUse1x3siteSLK', false);
    vis('cbOctal12Site', false);
    vis('labYOffset', false);
    vis('edYOffset', false);
    vis('rgYOffset', false);
    vis('cb16DirectHeater', false);
    vis('cb12Site10DirectHeater', false);
    vis('cbHotechLayoutKit2x2', false);
    vis('cbQualSite2X2Shift', false);
    vis('paQualSite2X2Shift', false);
    vis('cbSingleSiteSingleHeater', false);
    vis('gbInUseBackRow', false);
    vis('gbOutUseBackRow', false);
    chk('cbQualSite2X2Shift', false);                   // kevin 20170513
    vis('grpUseXCenterPitch', cos.b2x4SupportCenterPitch === true && iCheckPos === _8Site2X4);
    bUseTwoArm32Site = false;
    vis('cb16change12DirectHeater', false); chk('cb16change12DirectHeater', false);
    vis('cb16change8DirectHeater', false);  chk('cb16change8DirectHeater', false);
    vis('rgYPitchOffsetMode', false);
    vis('cbIndSLK', false);
    vis('cbSingleUseOtherSuck', false);
    chk('chkOffCenterkit', false); vis('chkOffCenterkit', false);
    vis('cbUseRotateForHT7000HPKit', false);
    vis('palVisibleIndex', false);
    vis('cbSingleInArmUseOtherSuck', false);
    vis('cbPreventDropfunction', false);

    var yp = CAPS.useInOutArmYPitch;
    var pick16 = (yp === iXYPitch16Picker || yp === iXYPitch16Bd_Be);

    /* ---- golden 的 switch(iCheckPos)（cSetUp.cpp 414..1145）---------------- */
    switch (iCheckPos) {

      case SingleSite:                                  // 1x1
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        ena('cbAa', false);
        if ($('cbAa')) $('cbAa').selectedIndex = 1;
        vis('rgSelectSearchLast', false);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        vis('chkOffCenterkit', cos.bCanUseBias);
        if (heater16Family()) {
          if (CAPS.customerCode !== CC_SCS) vis('cb2CableLayoutKit', true);
        } else if (CAPS.use16Heater === eht4Heater) {
          vis('cbSingleSiteSingleHeater', true);
        }
        vis('lblXPitch', false); vis('lblXPMM', false);
        vis('lblYPitch', false); vis('YPitch', false); vis('XPitch', false);
        text('XPitch', '1'); text('YPitch', '0');
        iSiteTotal = 1;
        vis('cb16DirectHeater', direct16Visible());
        vis('cbSingleUseOtherSuck', true);
        vis('cbSingleInArmUseOtherSuck', true);
        vis('cbPreventDropfunction', true);
        break;

      case DualSite:                                    // 1x2
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        vis('rgSelectSearchLast', false);
        vis('rgUseSuckMode', true);
        rgText('rgUseSuckMode', 0, 'Use 2 pick unit');
        rgText('rgUseSuckMode', 1, 'Use 4 pick unit');
        if (rgIndex('rgUseSuckMode') === 1) searchLastByConfig(cos);
        if (heater16Family()) {
          vis('cb2CableLayoutKit', true);
          if (ini.bL30Use1CableLayoutKitByConfig === false) vis('cb1CableLayoutKit', true);
        }
        vis('chkNS7000CS', false);
        vis('chkOffCenterkit', cos.bCanUseBias);
        if (cos.bEnable_1x3Kit) {                       // KevinCheng 20260109
          vis('cbUse1x3siteSLK', true);
          chkCaption('cbUse1x3siteSLK', 'Dual Site use 1x3 SLK');
          vis('cbUse1x3siteSLK', cos.bEnable_1x3Kit);
        }
        if (CAPS.customerCode === CC_ASE_KaohSiung) {   // kevin 20170415
          vis('cbQualSite2X2Shift', true);
          vis('paQualSite2X2Shift', chk('cbQualSite2X2Shift'));
        }
        if (CAPS.installDoubleEp === DOUBLE_EP_INDIVIAL ||
            CAPS.installDoubleEp === DOUBLE_EP_MULTI) {
          vis('cbIndSLK', true);
        }
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '80'); text('YPitch', '0');
        iSiteTotal = 2;
        vis('cb16DirectHeater', direct16Visible());
        vis('cb1x2Use1x4siteSLK', cos.bEnableDual_1x4Kit);
        vis('cbUseRotateForHT7000HPKit',
            cos.bRotateUseHT7000HPKit && !!CAPS.useRotateKit);
        break;

      case DualSite2x1:                                 // 2x1
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        vis('rgSelectSearchLast', false);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', true);                       // Steven 20140312 : For HT9045WA
        vis('lblXPitch', false); vis('lblXPMM', false);
        vis('lblYPitch', true); vis('YPitch', true); vis('XPitch', false);
        text('XPitch', '0'); text('YPitch', '');
        iSiteTotal = 2;
        vis('cb16DirectHeater', direct16Visible());
        break;

      case TriSite1X3:                                  // 1x3
        chk('Arm1PickArm2Test', false);
        vis('gbArm1PickArm2Test', false);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', CAPS.atcSystem > eATC30);
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '40'); text('YPitch', '0');
        iSiteTotal = 3;
        vis('cb16DirectHeater', direct16Visible());
        break;

      case _8Site1X4:                                   // 海思 8-Site Pop
        chk('Arm1PickArm2Test', false);
        vis('gbArm1PickArm2Test', false);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', CAPS.atcSystem > eATC30);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '40'); text('YPitch', '60');
        iSiteTotal = 4;
        vis('cb16DirectHeater', direct16Visible());
        break;

      case QualSite1X4:                                 // 1x4
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        vis('rgSelectSearchLast', false);
        if (CAPS.usePickerCount !== 0) {
          vis('rgUseSuckMode', true);
          rgText('rgUseSuckMode', 0, 'Use 4 pick unit');
          rgText('rgUseSuckMode', 1, 'Use 8 pick unit');
          if (rgIndex('rgUseSuckMode') === 1) {
            if (cos.bDisableSelectSearchLast === true) rgIndex('rgSelectSearchLast', 0);
            else vis('rgSelectSearchLast', true);
          }
        }
        if (CAPS.customerCode !== CC_SCS && heater16Family()) {
          vis('cb2CableLayoutKit', true);
          if (ini.bL30Use1CableLayoutKitByConfig === false) vis('cb1CableLayoutKit', true);
        }
        if (cos.bInOutArmUseBackRowSuck === true) {
          vis('gbInUseBackRow', true);
          vis('gbOutUseBackRow', true);
        }
        vis('chkNS7000CS', true);                       // kevin 20140829 使用偏心氣孔
        if (CAPS.customerCode === CC_ASE_KaohSiung) {
          vis('chkOffCenterkit', true);
          chk('chkOffCenterkit', true);                 // kevin 20200924
        } else {
          vis('chkOffCenterkit', cos.bCanUseBias);
        }
        if (cos.bNonCenterModeCanUseShtOffset && ini.bA50Enable1x4BiasYOffset
            && chk('chkOffCenterkit')) {
          vis('rgYOffset', true);
          rgIndex('rgYOffset', 0);
        }
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '40'); text('YPitch', '0');
        iSiteTotal = 4;
        vis('cb16DirectHeater', direct16Visible());
        if (CAPS.installDoubleEp === DOUBLE_EP_MULTI) vis('cbIndSLK', true);
        break;

      case QualSite2X2N:                                // 2x2 NN
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        if (CAPS.customerCode === CC_SCS) {
          vis('cbHotechLayoutKit2x2', true);
        } else {
          vis('chkOffCenterkit', cos.bCanUseBias);
        }
        if (cos.bEnable_1x3Kit) {
          vis('cbUse1x3siteSLK', true);
          chkCaption('cbUse1x3siteSLK', '2x2 NN Mode use 1x3 SLK');
          vis('cbUse1x3siteSLK', cos.bEnable_1x3Kit);
        }
        rgIndex('rgYPitchOffsetMode', 0);
        vis('labYOffset', true); vis('edYOffset', true);
        text('edYOffset', '80');
        vis('gbShuttleMode', false);
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '80'); text('YPitch', '0');
        iSiteTotal = 4;
        if (CAPS.installDoubleEp === DOUBLE_EP_MULTI) vis('cbIndSLK', true);
        break;

      case QualSite2X2:                                 // 2x2
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        if (CAPS.customerCode !== CC_SCS) {
          vis('cbSquareOctalLayout', true);
          if (CAPS.customerCode === CC_HANA_MICRON) vis('chk2x2Use16siteSLK', true);
        }
        if (CAPS.usePickerCount !== 0) {
          vis('rgUseSuckMode', true);
          rgText('rgUseSuckMode', 0, 'Use 4 pick unit');
          rgText('rgUseSuckMode', 1, 'Use 8 pick unit');
        }
        if (ini.bL30Use1CableLayoutKitByConfig === false) vis('cb1CableLayoutKit', true);
        vis('chkNS7000CS', true);
        if (CAPS.customerCode === CC_TSMC_TAINAN || CAPS.customerCode === CC_ASE_KaohSiung) {
          vis('cbQualSite2X2Shift', true);
          // golden 看的是 TestIF_File.bQualSite2X2Shift（配方裡的值）；
          // 這一頁沒接 TestIF，改看畫面上這顆 checkbox 自己的狀態（見檔頭界線 c）。
          vis('paQualSite2X2Shift', chk('cbQualSite2X2Shift'));
        }
        vis('chkOffCenterkit', !!cos.bCanUse2x2Bias);
        if (CAPS.customerCode === CC_SCS) vis('cbHotechLayoutKit2x2', true);
        if (CAPS.customerCode === CC_ATEC && CAPS.headDeviceCT === 3) {
          vis('cb2CableLayoutKit', true);
        }
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '80'); text('YPitch', '60');
        iSiteTotal = 4;
        vis('cb16DirectHeater', direct16Visible());
        if (CAPS.installDoubleEp === DOUBLE_EP_MULTI) vis('cbIndSLK', true);
        break;

      case _6Site2X3:                                   // 2x3
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '40'); text('YPitch', '60');
        iSiteTotal = 6;
        vis('cb16DirectHeater', direct16Visible());
        break;

      case _6Site2X3N:                                  // 2x3 NN
        vis('gbArm1PickArm2Test', false);
        vis('rgSelectSearchLast', false);
        rgIndex('rgSelectSearchLast', 0);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        vis('cbHotechLayoutKit2x2', false);
        rgIndex('rgYPitchOffsetMode', 0);
        vis('labYOffset', true); vis('edYOffset', true);
        text('edYOffset', '80');
        vis('gbShuttleMode', false);
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '120'); text('YPitch', '0');
        iSiteTotal = 6;
        break;

      case _8Site2X4N:                                  // 2x4 NN
        vis('gbArm1PickArm2Test', false);
        vis('rgSelectSearchLast', false);
        if (CAPS.usePickerCount !== 0) {
          vis('rgUseSuckMode', true);
          rgText('rgUseSuckMode', 0, 'Use 4 pick unit');
          rgText('rgUseSuckMode', 1, 'Use 8 pick unit');
          if (rgIndex('rgUseSuckMode') === 1) {
            if (cos.bDisableSelectSearchLast === true) rgIndex('rgSelectSearchLast', 0);
            else vis('rgSelectSearchLast', true);
          }
        }
        vis('chkNS7000CS', false);
        vis('cbHotechLayoutKit2x2', false);
        rgIndex('rgYPitchOffsetMode', 0);
        vis('labYOffset', false); vis('edYOffset', false);
        text('edYOffset', '80');
        vis('gbShuttleMode', false);
        vis('lblYPitch', false); vis('YPitch', false);
        text('XPitch', '80'); text('YPitch', '0');
        iSiteTotal = 8;
        break;

      case _8Site2X4:                                   // 2x4
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        if (pick16) {                                   // Steven 20231207 : 2x4 16 picker
          rgText('rgUseSuckMode', 0, 'Use 8 pick unit');
          rgText('rgUseSuckMode', 1, 'Use 16 pick unit');
          vis('rgUseSuckMode', true);
        } else {
          vis('rgUseSuckMode', false);
        }
        searchLastByConfig(cos);
        // ⚠ golden 在這裡又無條件關掉一次，把上面 16-picker 那支的 Visible=true
        //   蓋掉。照抄，不要「修好」—— 這是機台現在的行為。
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        if (CAPS.customerCode === CC_ATEC && CAPS.headDeviceCT === 4) {
          vis('cb2CableLayoutKit', true);
        }
        if (ini.bL30Use1CableLayoutKitByConfig === false) vis('cb1CableLayoutKit', true);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '40'); text('YPitch', '60');
        iSiteTotal = 8;
        if (CAPS.customerCode !== CC_AMKOR_China && CAPS.customerCode !== CC_QUALCOMM &&
            cos.bKoreaFunction === false && heater16Family()) {
          vis('cbNS8000H', true);
        }
        if (CAPS.machineType === Type_HT9046 || CAPS.machineType === Type_HT9046_LS) {
          if (CAPS.customerCode === CC_SCK) vis('chkOctal80', true);
          vis('cbOctal12Site', cos.bEnableOctal_12Kit);
          chkCaption('cbOctal16Site', 'Octal site use 16 Site Layout Kit');
          vis('cbOctal16Site', cos.bEnableOctal_16Kit);
        }
        vis('cb16DirectHeater', direct16Visible());
        break;

      case _16Site2X8:                                  // 2x8
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        vis('cb16change12DirectHeater', true);
        vis('cb16change8DirectHeater', true);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '30'); text('YPitch', '60');
        iSiteTotal = 16;
        vis('cb6CableLayoutKit', CAPS.machineType === Type_HT1032);
        vis('cb16DirectHeater', direct16Visible());
        break;

      case _12Site2X6:                                  // 2x6
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', CAPS.customerCode === CC_SIGURD_PeiXing);
        vis('chk12SiteUse2x8SLK', !!cos.bEnable12SiteUse16SLK);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '40'); text('YPitch', '63.5');
        vis('cb16DirectHeater', direct16Visible());
        if (CAPS.customerCode === CC_ASE_KaohSiung) {
          vis('cb12Site10DirectHeater', false);         // kevin 20161102
        } else {
          vis('cb12Site10DirectHeater',
              CAPS.use16Heater === eht32HeaterEJ1N || CAPS.use16Heater === eht32HeaterKT4H ||
              CAPS.use16Heater === eht32HeaterDTME08);
        }
        iSiteTotal = 12;
        break;

      case _10Site2X5:                                  // 2x5
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        vis('rgUseSuckMode', false);
        vis('chkNS7000CS', CAPS.customerCode === CC_SIGURD_PeiXing);
        vis('cb6CableLayoutKit', CAPS.machineType === Type_HT1032);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '60'); text('YPitch', '60');
        // ⚠ 10-Site 這一支 golden 少了 DTME08（其他 11 支都有）。照抄。
        vis('cb16DirectHeater', direct16Visible(false));
        if (CAPS.customerCode === CC_ASE_KaohSiung) {
          vis('cb12Site10DirectHeater', false);
        } else {
          vis('cb12Site10DirectHeater',
              CAPS.use16Heater === eht32HeaterEJ1N || CAPS.use16Heater === eht32HeaterKT4H);
        }
        iSiteTotal = 10;
        break;

      case _16Site4X4:                                  // 4x4 —— golden 這裡沒有 break，
      case _32Site4X8N:                                 //        直接掉進 4x8。照抄。
        if (iCheckPos === _16Site4X4) {
          if (pick16) {
            rgText('rgUseSuckMode', 0, 'Use 8 pick unit');
            rgText('rgUseSuckMode', 1, 'Use 16 pick unit');
            vis('rgUseSuckMode', true);
          } else {
            vis('rgUseSuckMode', false);
          }
          chkCaption('cbOctal16Site', '4x4 site use 4x8 site layout kit');
          vis('cbOctal16Site', cos.bEnableOctal_16Kit);
        }
        vis('gbArm1PickArm2Test', ini.bD58UseArm1PickPlaceArm2Test);
        searchLastByConfig(cos);
        if (iCheckPos === _32Site4X8N) vis('rgUseSuckMode', false);
        vis('chkNS7000CS', false);
        vis('cb6CableLayoutKit', CAPS.machineType === Type_HT1032);
        vis('lblYPitch', true); vis('YPitch', true);
        text('XPitch', '40'); text('YPitch', '60');
        vis('rgYPitchOffsetMode', true);
        vis('labYOffset', true); vis('edYOffset', true);
        text('edYOffset', '80');
        vis('gbShuttleMode', false);
        vis('cb16DirectHeater', direct16Visible());
        if (iCheckPos === _16Site4X4) {
          iSiteTotal = 16;
        } else {
          iSiteTotal = 32;
          if (cos.b32SiteYOffsetMode) vis('palVisibleIndex', true);
        }
        break;

      // golden 的 switch 沒有 _32Site4X8M 這一支 —— 選到它時所有可選元件
      // 維持上面那段「先全部關掉」的狀態。照抄，不要補。
    }

    bUseTwoArm32Site = (iCheckPos === QualSite2X2N || iCheckPos === _6Site2X3N ||
                        iCheckPos === _8Site2X4N || iCheckPos === _16Site4X4 ||
                        iCheckPos === _32Site4X8N);

    /* golden 結尾：右半邊四個區塊全隱藏就把表單縮成 818，否則 1090。
     * dfm 的 fSetup.Width=1090 是**含邊框**的視窗寬，產生器給 .form 的 1082
     * 是客戶區寬（1090 - 2×4 邊框）。所以這裡不能直接寫 818/1090，
     * 要用「產生器給的寬度」減掉 golden 的差 272，頁面在常見情況下才
     * 一個 pixel 都不會動。 */
    var wide = isVis('gbArm1PickArm2Test') || isVis('gbOcr') ||
               isVis('gbSocketClamp') || isVis('grpSocketSensor');
    var form = document.querySelector('.form');
    if (form) {
      if (FORM_W === null) FORM_W = form.offsetWidth || 1082;
      form.style.width = (wide ? FORM_W : Math.max(0, FORM_W - (1090 - 818))) + 'px';
    }
    vis('grpFunction', wide);
  }

  /* ==========================================================================
   * 7. golden: btnLUpToRDownNClick()      cSetUp.cpp 4176..4299
   *    六顆鈕共用這一支，方向來自 dfm 的 Tag。
   * ========================================================================== */

  /* dfm cSetUp.dfm 的 Tag（沒寫 Tag 的就是 0） */
  var SORT_BTN = {
    btnLUpToRDownZ: 0,      // Left_Top    To Right      -> iSiteMapDirection 5
    btnLDownToRUpZ: 1,      // Left_Bottom To Right      -> 10
    btnRUpToLDownZ: 2,      // Right_Top   To Left       -> 9
    btnRDownToLUpZ: 3,      // Right_Bottom To Left      -> 6
    btnLUpToRDownN: 4,      // Left_Top    To Bottom     -> 7
    btnRUpToLDownN: 5       // Right_Top   To Bottom     -> 11
  };
  var DIR2CODE = { 0: 5, 1: 10, 2: 9, 3: 6, 4: 7, 5: 11 };

  var iSiteMapDirection = 0;    // golden 的 IniConfig.iSiteMapDirection

  function chVisible(r, c) {
    var el = siteCH(r, c);
    return !!el && el.style.display !== 'none';
  }
  function chEnabled(r, c) {
    var el = siteCH(r, c);
    return !!el && !el.disabled;
  }
  function setCH(r, c, v) {
    var el = siteCH(r, c);
    if (el) el.selectedIndex = (v >= 0 && v < el.options.length) ? v : -1;
  }

  function sortSiteMap(iDirection) {
    var iSX, iSY, iCT = 1, i, j;

    if (iDirection >= 0 && iDirection <= 3) {
      iSX = MAX_SOCKET_ROW; iSY = MAX_SOCKET_COL;
    } else {
      iSX = MAX_SOCKET_COL; iSY = MAX_SOCKET_ROW;
    }

    for (i = 0; i < iSX; i++) {
      for (j = 0; j < iSY; j++) {
        // ⚠ golden 的 [0..3] 那四支，Visible 看目標格、Enabled 卻看 [i][j]
        //   （只有 direction 0 兩者剛好同一格）。這是 golden 的原樣，照抄；
        //   改成「都看目標格」會讓 12-Site 的行為和機台不一樣。
        if (iDirection === 0) {                         // Left_Top To Right
          if (chVisible(i, j) && chEnabled(i, j)) { setCH(i, j, iCT); iCT++; }
        } else if (iDirection === 1) {                  // Left_Bottom To Right
          if (chVisible(3 - i, j) && chEnabled(i, j)) { setCH(3 - i, j, iCT); iCT++; }
        } else if (iDirection === 2) {                  // Right_Top To Left
          if (chVisible(i, 7 - j) && chEnabled(i, j)) { setCH(i, 7 - j, iCT); iCT++; }
        } else if (iDirection === 3) {                  // Right_Bottom To Left
          if (chVisible(3 - i, 7 - j) && chEnabled(i, j)) { setCH(3 - i, 7 - j, iCT); iCT++; }
        } else if (iDirection === 4) {                  // Left_Top To Bottom
          if (chVisible(j, i) && chEnabled(j, i)) { setCH(j, i, iCT); iCT++; }
        } else {                                        // Right_Top To Bottom
          if (chVisible(j, 7 - i) && chEnabled(j, i)) { setCH(j, 7 - i, iCT); iCT++; }
        }
      }
    }

    iSiteMapDirection = DIR2CODE[iDirection] || 0;      // Jimmychiu 20230807
  }

  /* ==========================================================================
   * 8. TScrollBar（Kind=sbVertical, Min=0, Max=8→執行期改, PageSize=0）
   * ========================================================================== */

  var SB = { min: 0, max: _8Site1X4, pos: 0, el: null, thumb: null, track: null };

  function position() { return SB.pos; }

  /* silent=true 只改值與外觀，不觸發 OnChange —— 給 ScrollBar1Change 自己
   * 夾限位置時用（golden 是靠「改 Position 會再進一次事件」，這裡不要遞迴）。 */
  function setPosition(v, silent) {
    v = Math.max(SB.min, Math.min(SB.max, Math.round(v)));
    var changed = (v !== SB.pos);
    SB.pos = v;
    layoutThumb();
    if (SB.el) {
      SB.el.setAttribute('aria-valuenow', String(v));
      SB.el.setAttribute('aria-valuetext', MODES[v] ? MODES[v].name : String(v));
    }
    if (changed && !silent) scrollBar1Change();
    return SB.pos;
  }

  var ARROW = 14;   // 上下箭頭高度（VCL 是方形按鈕，寬度＝捲軸寬 17）

  function trackGeom() {
    var h = SB.el ? SB.el.clientHeight : 0;
    var top = ARROW, bot = h - ARROW;
    return { top: top, height: Math.max(0, bot - top) };
  }
  function layoutThumb() {
    if (!SB.thumb) return;
    var gm = trackGeom(), span = SB.max - SB.min;
    var th = Math.max(12, Math.floor(gm.height / (span + 1)));
    var free = Math.max(0, gm.height - th);
    var y = gm.top + (span > 0 ? Math.round(free * (SB.pos - SB.min) / span) : 0);
    SB.thumb.style.height = th + 'px';
    SB.thumb.style.top = y + 'px';
  }

  function buildScrollBar() {
    var el = $('ScrollBar1');
    if (!el) { console.warn('[Setup/SiteMap] 找不到 ScrollBar1，捲軸沒有接。'); return; }
    SB.el = el;
    el.innerHTML = '';
    el.style.overflow = 'hidden';
    el.style.cursor = 'default';
    el.setAttribute('tabindex', '0');
    el.setAttribute('role', 'slider');
    el.setAttribute('aria-orientation', 'vertical');

    var btnCss = 'position:absolute;left:0;right:0;height:' + ARROW + 'px;'
               + 'background:var(--tab-bg,#d4d0c8);border:1px outset #ddd;'
               + 'box-sizing:border-box;display:flex;align-items:center;'
               + 'justify-content:center;font-size:8px;line-height:1;'
               + 'color:var(--fg,#222);user-select:none;';
    var up = document.createElement('div');
    up.setAttribute('style', btnCss + 'top:0;');
    up.textContent = '\u25B2';
    var down = document.createElement('div');
    down.setAttribute('style', btnCss + 'bottom:0;');
    down.textContent = '\u25BC';

    var thumb = document.createElement('div');
    thumb.setAttribute('style',
      'position:absolute;left:1px;right:1px;background:var(--form-bg,#ece9d8);'
      + 'border:1px outset #ddd;box-sizing:border-box;');
    SB.thumb = thumb;

    el.appendChild(up); el.appendChild(down); el.appendChild(thumb);

    function step(d) { setPosition(SB.pos + d); }
    up.addEventListener('mousedown', function (e) { e.preventDefault(); step(-1); });
    down.addEventListener('mousedown', function (e) { e.preventDefault(); step(1); });

    /* 槽上點擊：VCL 的 LargeChange 預設 1，所以一次一格，和箭頭同樣幅度。 */
    el.addEventListener('mousedown', function (e) {
      if (e.target === up || e.target === down || e.target === thumb) return;
      var r = el.getBoundingClientRect(), y = e.clientY - r.top;
      if (y < ARROW || y > r.height - ARROW) return;
      var tr = thumb.getBoundingClientRect();
      step(e.clientY < tr.top ? -1 : 1);
      e.preventDefault();
    });

    /* 拖曳滑塊 */
    thumb.addEventListener('mousedown', function (e) {
      e.preventDefault();
      el.focus();
      var gm = trackGeom(), span = SB.max - SB.min;
      var th = thumb.offsetHeight, free = Math.max(1, gm.height - th);
      var r = el.getBoundingClientRect();
      var grab = e.clientY - thumb.getBoundingClientRect().top;
      function move(ev) {
        var y = ev.clientY - r.top - grab - gm.top;
        setPosition(SB.min + Math.round(span * Math.max(0, Math.min(free, y)) / free));
      }
      function up2() {
        document.removeEventListener('mousemove', move);
        document.removeEventListener('mouseup', up2);
      }
      document.addEventListener('mousemove', move);
      document.addEventListener('mouseup', up2);
    });

    el.addEventListener('wheel', function (e) {
      e.preventDefault();
      step(e.deltaY > 0 ? 1 : -1);
    }, { passive: false });

    el.addEventListener('keydown', function (e) {
      var k = e.key;
      if (k === 'ArrowDown' || k === 'ArrowRight') { step(1); }
      else if (k === 'ArrowUp' || k === 'ArrowLeft') { step(-1); }
      else if (k === 'PageDown') { step(1); }
      else if (k === 'PageUp') { step(-1); }
      else if (k === 'Home') { setPosition(SB.min); }
      else if (k === 'End') { setPosition(SB.max); }
      else return;
      e.preventDefault();
    });

    if (window.ResizeObserver) new ResizeObserver(layoutThumb).observe(el);
  }

  /* ==========================================================================
   * 9. 起始：讀 CAPS -> 接捲軸與六顆鈕 -> 依配方還原 Test Mode 與 Site Map
   * ========================================================================== */

  /* golden cSetUp.cpp:2185 GetTestMode()：存檔存的是模式的文字，不是編號。 */
  function modeFromText(s) {
    if (!s) return -1;
    s = String(s).trim();
    for (var i = 0; i < MODES.length; i++) if (MODES[i].name === s) return i;
    var n = parseInt(s, 10);                            // 舊配方可能還是數字
    return (!isNaN(n) && n >= 0 && n < MODES.length) ? n : -1;
  }

  /* golden FormShow 的順序（cSetUp.cpp 1850..1865）：
   *     DoIniDataToForm()                 -> ScrollBar1->Position = 配方的 Test Mode
   *     ScrollBar1Change(this)            -> 「要放在 DoIniDataToForm(); 後面」
   *     TestSiteCH[i][j]->ItemIndex = ... -> 再把 Site Map 填回格子
   * 順序不能顛倒：CompChange() 會把格子的 Items 清空，先填的值會被吃掉；
   * 而且 asHandlingMode（開機時的 Handler Mode）是在 ScrollBar1Change 裡
   * 第一次執行時記下來的 —— 先跑事件再讀配方的話，它會記成 Single Site。 */
  function readRecipe() {
    if (typeof g.HT9045Recipe === 'undefined') return Promise.resolve(null);
    return g.HT9045Recipe.read('handlerCondition').then(function (doc) { return doc; },
      function (e) {
        console.warn('[Setup/SiteMap] 讀不到 HandlerCondition.Data，'
                   + 'Test Mode 與 Site Map 維持預設：' + e);
        return null;
      });
  }
  function applyRecipeSiteMap(doc) {
    if (!doc) return;
    for (var r = 0; r < MAX_SOCKET_ROW; r++) {
      for (var c = 0; c < MAX_SOCKET_COL; c++) {
        var raw = iniRaw(doc, 'Configuration', 'Site ' + ROWCH[r] + COLCH[c]);
        if (raw === null) continue;
        var v = parseInt(String(raw).trim(), 10);
        if (!isNaN(v)) setCH(r, c, v);
      }
    }
    var d = parseInt(String(iniRaw(doc, 'Configuration', 'SiteMapDirection') || '0').trim(), 10);
    if (!isNaN(d)) iSiteMapDirection = d;
  }

  function attach() {
    buildScrollBar();
    SB.max = _8Site1X4;                                 // golden 的 iMax 在事件裡才夾

    Object.keys(SORT_BTN).forEach(function (id) {
      var b = $(id);
      if (!b) { console.warn('[Setup/SiteMap] 找不到排序鈕 ' + id); return; }
      b.style.cursor = 'pointer';
      b.addEventListener('click', function () { sortSiteMap(SORT_BTN[id]); });
    });

    /* golden 裡這三顆的 OnClick 會回頭影響 Image1 / CH 數量 */
    var off = $('chkOffCenterkit');
    if (off) off.addEventListener('change', chkOffCenterkitClick);
    var shift = $('cbQualSite2X2Shift');
    if (shift) shift.addEventListener('change', cbQualSite2X2ShiftClick);
    var xcp = $('chkUseXCenterPitch');
    if (xcp) xcp.addEventListener('change', chkOffCenterkitClick);   /* AI(W906-Q41) 20260927 (St02-E): SU-1..SU-6 的 DFM OnClick（912 cSetUp.dfm :70/:80/:562/:1446/:1471/:1535/:1591/:925/:874/:901/:770） */ [['cbEnablePreciser', chkOffCenterkitClick], ['cbEnabledPreciserRT', chkOffCenterkitClick], ['rgShtModeNormal', rgShtModeNormalClick], ['rgShtModeOneSide', rgShtModeNormalClick], ['rgUseSuckMode', rgUseSuckModeClick], ['Arm1PickArm2Test', arm1PickArm2TestClick], ['cbArm1UseHeat', arm1PickArm2TestClick], ['cbArm1OnlyPlaceArm2TestAndSuck', arm1PickArm2TestClick], ['cbUseSLKClamp', cbUseSLKClampClick], ['cbUseTesterDry', cbUseSLKClampClick], ['rgYPitchOffsetMode', rgYPitchOffsetModeClick]].forEach(function (q) { var e = $(q[0]); if (e) e.addEventListener('change', q[1]); });   /* 用 id 找（R76 會替沒 name 的 radio 補 name） */
    var oct12 = $('cbOctal12Site');                     // golden cbOctal12SiteClick
    if (oct12) oct12.addEventListener('change', function () {
      var cos = CAPS.cos || {};
      if ((cos.bSPILFunction === true || CAPS.customerCode === CC_ASE_KaohSiung) &&
          (position() === _8Site2X4 || position() === _16Site4X4) &&
          cos.bEnableOctal_12Kit === true) {
        scrollBar1Change();
      }
    });

    loadCaps().then(function (caps) {
      CAPS = caps;
      SB.max = maxMode();
      return readRecipe();
    }).then(function (doc) {
      var want = doc ? iniRaw(doc, 'Configuration', 'Test Mode') : null;
      var mode = modeFromText(want);
      if (mode >= 0) SB.pos = mode;                     // = DoIniDataToForm()
      scrollBar1Change();                               // 夾限、換圖、重建格子
      applyRecipeSiteMap(doc);                          // 最後才填 Site Map
      layoutThumb();
      /* golden 夾限是靜默的（iPos>=iMax -> Position=iMax）。畫面上只會看到
       * 「模式自己變了」，查起來很費工 —— 所以這裡一定講出來。 */
      if (mode >= 0 && position() !== mode) {
        var capNote = (MODE_CAP[CAPS.machineType] !== undefined || CAPS.modeCap !== null)
          ? '（含 web 這一層的 MODE_CAP 上限「' + MODES[maxMode()].name + '」）' : '';
        console.warn('[Setup/SiteMap] 配方的 Test Mode 是「' + want + '」，'
                   + '但依目前機種／客戶碼只能到「' + MODES[position()].name
                   + '」' + capNote + '，已夾限。機種=' + machineName(CAPS.machineType)
                   + '（Gerneral.ini [Version] Model 推的）、客戶碼=' + CAPS.customerCode
                   + '。要放寬請設 window.HT9045SetupCaps = {modeCap: N} 或 {machineType: N}。');
      } else if (mode < 0 && want) {
        console.warn('[Setup/SiteMap] 配方的 Test Mode「' + want + '」不在 golden 的'
                   + ' TestSiteFileName[0][] 裡，畫面維持 ' + MODES[position()].name + '。');
      }
    }, function (e) {
      console.error('[Setup/SiteMap] 起始失敗：' + (e && e.stack ? e.stack : e));
    });
  }

  /* 給探針／除錯用（和 ht9045_wire_engine.js 的 HT9045Page 同一個用意）。 */
  g.HT9045SetupSiteMap = {
    scrollBar1Change: scrollBar1Change,
    compChange: compChange,
    sortSiteMap: sortSiteMap,
    position: position,
    setPosition: setPosition,
    modes: MODES,
    caps: function () { return CAPS; },
    state: function () {
      return { pos: SB.pos, max: SB.max, orgTestMode: OrgTestMode,
               iSiteTotal: iSiteTotal, bUseTwoArm32Site: bUseTwoArm32Site,
               iSiteMapDirection: iSiteMapDirection, asHandlingMode: asHandlingMode };
    }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', attach);
  } else {
    attach();
  }
}(this));
