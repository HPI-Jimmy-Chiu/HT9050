/* ht9045_contact_slk.js -- Setup.Contact.html 的 scrbSLK（Compliance Unit 捲軸）
 * ---------------------------------------------------------------------------
 * //Steven 20260921
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260921_Steven.md
 * ---------------------------------------------------------------------------
 * 把 golden 的這一支 handler 搬到瀏覽器：
 *
 *   cContact.cpp  TfContact::scrbSLKChange(TObject*)        2085..2092
 *
 * 它本身只有四行，但那四行把三支函式整個拉進來，所以這個檔一起翻譯了：
 *
 *   cContact.cpp  TfContact::DutCount()                     1970..2083
 *   cContact.cpp  TfContact::ShowArmAndDeviceForce()        1907..1930
 *   cContact.cpp  TfContact::CalculateTotalAirForce()      18765..18981
 *   cContact.cpp  TfContact::GetMaxIndexForceLimit()       18983..19045
 *   cContact.cpp  TfContact::GetMinForce()                 19047..19112
 *   cContact.cpp  TfContact::CalcDeviceForce()             22852..22858
 *   cContact.cpp  TfContact::edAirForceChange()             2279..2284
 *
 * //Steven 20260921 第二批：同一條計算鏈上的另外三支 OnChange，以及存檔。
 *
 *   cContact.cpp  TfContact::edPinCountChange()            1932..1938
 *   cContact.cpp  TfContact::edForcePerPinNChange()        1939..1954
 *   cContact.cpp  TfContact::edDieForcePerPinGChange()     1955..1974
 *   cContact.cpp  TfContact::CountDieForceKg(bool)        18461..18484
 *   cContact.cpp  TfContact::SaveSetupFile() 的三行        14381 / 14389..14415 / 14418
 *
 * 外加 cContact.dfm 的 TScrollBar（Kind=sbVertical, Min=2, Max=5, Position=2）
 * —— 產生器只畫了一個空的 <div>，這裡把它升級成真的能拖、能點、能滾輪、
 * 能鍵盤的捲軸，OnChange 接到 scrbSLKChange()。做法與
 * ht9045_setup_sitemap.js 的 ScrollBar1 同一套，兩邊改動請一起看。
 *
 * ---------------------------------------------------------------------------
 * scrbSLK 是什麼
 * ---------------------------------------------------------------------------
 * 「幾顆 Device 共用一個浮動頭（Compliance Unit）」。Position 就是
 * DeviceForm_File.iHeadDeviceCT，值域 2..5（HT9046 / HT9045_12Site 是 2..6）：
 *
 *   2  1 Device with 1 Compliance Unit   fComplianceUnit = 1.0
 *   3  2 Device with 1 Compliance Unit   fComplianceUnit = 0.5
 *   4  4 Device with 1 Compliance Unit   fComplianceUnit = 0.25
 *   5  2 Device with 4 Compliance Unit   fComplianceUnit = 2.0
 *   6  8 Device with 1 Compliance Unit   fComplianceUnit = 0.125
 *
 * 換 Position 會換 imgSLK 的圖（IMG\BMP\Contact<N>.bmp），並且重算整組
 * Contact Force —— 每顆 IC 的 kgf/N、總空壓 kgf/N、最小/最大力量保護。
 *
 * ---------------------------------------------------------------------------
 * 五個資料來源，以及為什麼是這五個
 * ---------------------------------------------------------------------------
 *   1. system\Gerneral.ini      HT9045System.read('gerneral')
 *      [Version] Model          -> MachineTypeChoice（決定 scrbSLK->Max）
 *      [System]  CUSTOMER_CODE  -> CC_ASE_KaohSiung 的最小力量特例
 *      [System]  INDEX_PRESS_TYPE -> GetMaxIndexForceLimit() 的上限
 *      [System]  EP_MAXKPA      -> [D28] 缸徑上限的係數 5.0 / 6.0
 *
 *   2. config\config.ini        HT9045System.read('config')
 *      [Index] UseSingleSite85kg          = IniConfig.bD27UseSingleSite85kg
 *      [Index] bD28MaxForceLimitByDiameter
 *      [Contact Force] bD04MinForceByFile 與 dD04MinForceByFile*
 *
 *   3. system\ContactInfo.ini   HT9045System.read('contactInfo')
 *      [SLK Type] Type / Visible          -> rgKitDiameter 的選項與 SLKClass
 *      [Diameter_<d>.000mm] ContactOffset / ContactOffset_NS -> GetMinForce 的
 *      「不是 20/28/30/40/58/60/80」那條 fallback
 *
 *   4. 配方 handlerCondition    HT9045Recipe.read('handlerCondition')
 *      [Configuration] Test Mode          -> TestIF_File.iTestMode（存的是文字）
 *      [Configuration] Site Aa..Bh        -> TestIF_File.iSiteMap（16 site 關 site）
 *      [Configuration] Octal 12Kit / QualSite2X2 Shift /
 *                      NS7000 bias kit / bNSKitPress
 *
 *   5. 配方 contact             HT9045Recipe.read('contact')
 *      [Mode] Head Device Mode            -> scrbSLK->Position 的起始值
 *      [Mode] Kit Diameter                -> rgKitDiameter->ItemIndex（存的是
 *                                            公分，3.0 = 30mm；"40x2" 存 40.2）
 *
 * 另外 LastSet.bUseTestSocket 走執行期 tag（見下面「已知的界線」d.）。
 *
 * ---------------------------------------------------------------------------
 * 已知的界線 —— 這些 golden 有、這裡沒有，是刻意的
 * ---------------------------------------------------------------------------
 * a. ADAM_WriteVoltage() 沒有搬。edAirForceChange 在 golden 的第三行是把
 *    算出來的公斤數寫成 EP 比例閥的電壓 —— 那是真的會讓機台出力的動作。
 *    照 ht9045_contact_wire.js 檔頭那條規矩，硬體與互鎖留在 C++ 端；
 *    這裡只更新 DeviceForm.dPress 的畫面模型。
 *
 * b. iCloseSiteModeFor2x8 一律當 e2x8Standard(0)。它是
 *    ainarm9045_2x8_8.cpp 跑料途中才會改的執行期全域，不在任何 ini 裡，
 *    也沒有對應的 tag。影響只有 16-Site 的 dDutCount 會算成 16 而不是 8，
 *    而那個分支本來就要機台真的在跑 2x8 關 site 才成立。
 *
 * c. IniConfig.iEP_Min_KG 沒有回寫。golden 在 CalculateTotalAirForce 裡
 *    順手把算出來的 dMinKgPerHead 塞回 IniConfig，那是給 adam6024 用的
 *    執行期副作用，不是畫面的事。
 *
 * d. LastSet.bUseTestSocket 走 tag site.arm{1,2}.s{1..16}
 *    （producer WebBridgeTags.cpp:789，對應
 *      bUseTestSocket[arm-1][ n>8 ? 1 : 0 ][ (n-1)%8 ]）。
 *    ⚠ tag 沒接上時一律當 true（site 都在用），並且寫 console。
 *      這只會影響 1x2 / 2x1 / 2x2NN 在 [D27] 開啟時「關一邊 site 可以壓到
 *      85kg」那條分支；當 true 就是走一般的 dDutCount=2，比猜 false 安全
 *      —— 猜 false 會讓畫面顯示一個比實際允許值大一倍的空壓。
 *
 * e. EP_Install==5（雙臂各自一組 SLK）與 CC_ASE_SG 的 "80_Hi" 沒有搬進
 *    SLKClass。兩者都會讓 SLKClass 的長度與 rgKitDiameter 的選項對不齊，
 *    要搬就得連 ContactForce.cpp 的分頁一起搬，不是這一支 handler 的範圍。
 *
 * f. scrbSLK->Enabled 的開關沒有搬（cContact.cpp:1091 / :1706 各一處，
 *    條件在 SetContactMode / Contact 測試進行中）。這裡捲軸一律可動。
 *
 * g. （20260921 第二批已補）存檔接上了，走 ht9045_contact_wire.js 的
 *    addCollector() 擴充點，寫回 [Mode] Head Device Mode / Kit Diameter /
 *    KitDiameterMode 三個鍵。細節見 §17。
 *
 * h. （20260921 第二批已補）另外三支 OnChange 也接上了，見 §12a。
 *
 * i. golden 還有兩支同一區的 handler 沒接，因為它們是「點下去叫小鍵盤」而不是
 *    OnChange，與這條計算鏈的耦合方式不同：
 *      edForcePerDeviceKGClick(18497)  改每顆公斤 -> 反算 edForcePerPinG
 *      edForcePerPinGMouseDown(19119)  帶 InputLimit 的上下限（執行期變數）
 *    後者的夾限值 web 拿不到，contact_wire 的 KB 表已經把那兩格關掉夾限。
 *
 * ---------------------------------------------------------------------------
 * 一個刻意的加碼：rgKitDiameter 的選項
 * ---------------------------------------------------------------------------
 * 嚴格說這是 FormShow 的事（cContact.cpp:143..193），不是 scrbSLKChange。
 * 但 CalculateTotalAirForce 要拿 rgKitDiameter->Items 的字串去比 SLKClass，
 * 而 V910 的 CosFunction.bUseDynamicKitDiameter 是 InitialCosFunction() 給的
 * 預設 true 且全樹沒有任何一處設回 false —— 也就是說 golden **一定**會用
 * ContactInfo.ini 的 [SLK Type] Type 重建選項，dfm 裡那三顆 30/40/60 是
 * 永遠看不到的展示值。這台機器的 Type 是 30,40,60,56,80（五顆）。
 * 不重建就會有一個查不出來的錯：配方選 80mm 時 DOM 上根本沒有那一格，
 * ItemIndex 落空 -> iTag=-1 -> 最小力量用 30mm 的 1.5kg 而不是 80mm 的 15kg。
 * 所以這裡照 golden 的非 KYEC / 非 ASE-KH 那條分支重建選項。
 *
 * ---------------------------------------------------------------------------
 * 載入順序（Setup.Contact.html 的 </body> 之前，放在最後）
 * ---------------------------------------------------------------------------
 *   qwerty.js
 *   ht9045_recipe_client.js      （HT9045Recipe / HT9045System / HT9045Tags）
 *   ht9045_contact_wire.js       （HT9045Contact）
 *   ht9045_wire_engine.js        （HT9045Page）
 *   ht9045_wire_setupcontact.js
 *   ht9045_contact_slk.js        （本檔）
 *
 * ⚠ 本檔一定要最後跑，而且要等前面兩支接線把欄位填完才能算 —— 它們都會寫
 *   edAirForce / edPinCount / edForcePerPin*，我們算完又會蓋回去。golden 的
 *   順序就是這樣（cContact.cpp:1166 ReadFile -> :1175 fShow=true ->
 *   :1177 Position -> :1178 scrbSLKChange），所以這裡明確再呼叫一次它們的
 *   load() 並等兩個都 resolve，而不是賭誰先回來。多一次 GET，換一個確定的
 *   順序，值得。
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

  /* cmydef.cpp TestSiteFileName[] —— 配方 [Configuration] Test Mode 存的是
   * 這些字串，不是編號。與 ht9045_setup_sitemap.js 的 MODES 同一張表。 */
  var MODE_NAMES = [
    'Single Site', '2-Site', '3-Site (1x3)', 'In-Line 4-Site(1X4)',
    '2-Site (2x1)', 'Square 4-Site(2X2)', 'Square 4-Site(2X2) NN Mode',
    '6-Site', '6-Site (2x3) NN Mode', '8-Site', '8-Site (2x4) NN Mode',
    '10-Site', '12-Site', '16-Site', '16-Site (4X4)',
    '32-Site N Mode', '32-Site M Mode', '8-Site Pop'
  ];

  /* MachineType.h enum eMachineType（＋ web 自己的 HT-9050，見 sitemap 檔頭） */
  var Type_HT9046 = 200, Type_HT9045_12Site = 400;

  /* MachineType.h enum eIndexPressType -> 最大 kgf */
  var INDEX_PRESS_KG = { 1: 240, 2: 120, 3: 500, 4: 260, 5: 400,
                         6: 360, 7: 160, 8: 640, 9: 800 };

  /* MachineType.h #define CC_* —— 只列這幾支函式真的比對到的 */
  var CC_HONPREC_QC = 0, CC_SJ_Semiconductor = 791, CC_HANA_MICRON = 865,
      CC_VTEST_Shanghai = 919, CC_ASE_KaohSiung = 936, CC_JCET = 959;

  /* CosFunction.bForecePerPinKGf —— 「每個 pin 的力量以 kgf 為主」。
   * ht9045_setup_cosflags.js 那張產生表只涵蓋 Set Up 畫面的 26 個旗標，
   * 沒有這一個，所以在這裡自己列。來源 CosFunction.cpp：
   *   InitialCosFunction():4297        = false   <- 預設
   *   FUNC_CC_SJ_Semiconductor():1582  = true
   *   FUNC_CC_JCET():1701              = true
   *   FUNC_CC_VTEST_Shanghai():2305    = true
   *   FUNC_CC_HANA_MICRON():3109       = true
   * 全樹沒有第六處，FUNC_CC_Common() 也沒有覆寫。
   * true 的意義：edForcePerPinN 變唯讀，操作員只輸入 gf；
   *              edForcePerPinNChange 整支不做事（它是 ==false 的分支）。 */
  var FORCE_PER_PIN_KGF = {};
  FORCE_PER_PIN_KGF[CC_SJ_Semiconductor] = true;
  FORCE_PER_PIN_KGF[CC_JCET]             = true;
  FORCE_PER_PIN_KGF[CC_VTEST_Shanghai]   = true;
  FORCE_PER_PIN_KGF[CC_HANA_MICRON]      = true;

  /* ainarm9045_2x8_8.h enum：0 = 標準 2x8，其餘都是跑料途中的關 site 模式 */
  var e2x8Standard = 0;

  var MAX_SOCKET_ROW = 4, MAX_SOCKET_COL = 8;

  /* scrbSLK Position -> fComplianceUnit（cContact.cpp:18783..18797） */
  var COMPLIANCE_UNIT = { 2: 1.0, 3: 0.5, 4: 0.25, 5: 2.0, 6: 0.125 };

  /* ==========================================================================
   * 2. DOM 小工具
   * ========================================================================== */

  function $(id) { return document.getElementById(id); }

  function text(id, v) { var el = $(id); if (el) el.textContent = v; return el; }

  /* VCL Visible。用 display 而不是 visibility —— VCL 的 Visible=false 不佔位。 */
  function vis(id, on) {
    var el = $(id);
    if (el) el.style.display = on ? '' : 'none';
    return el;
  }

  /* VCL Color。clWhite 在這裡是「回到佈景的預設底色」而不是硬寫 #fff ——
   * 這一頁有深色佈景，硬寫白底會把深色模式的欄位變成一塊白。 */
  var CL_WHITE = '', CL_YELLOW = '#ffff80', CL_RED = '#ff8080';
  function colour(id, c) {
    var el = $(id);
    if (el) el.style.background = c;
    return el;
  }

  function fieldValue(id) {
    var el = $(id);
    if (!el) return '';
    return ('value' in el) ? el.value : el.textContent;
  }
  function setField(id, v) {
    var el = $(id);
    if (!el) return null;
    if ('value' in el) el.value = v; else el.textContent = v;
    return el;
  }

  /* C 的 atof：吃前綴數字、認不出來回 0。parseFloat 的語意剛好一樣，
   * 只差空字串會回 NaN。 */
  function atof(s) {
    var n = parseFloat(String(s === null || s === undefined ? '' : s).trim());
    return isNaN(n) ? 0.0 : n;
  }
  /* VCL FormatFloat("0.0000") / sprintf("%0.2f") */
  function fmt4(v) { return (isFinite(v) ? v : 0).toFixed(4); }
  function fmt2(v) { return (isFinite(v) ? v : 0).toFixed(2); }

  /* MachineType.h ChangeToFloatNonPcnt：分母 0 時回 0，不是 Infinity/NaN */
  function divNonPcnt(num, den) { return (den !== 0) ? (num / den) : 0.0; }

  /* ContactForce.cpp 的 CheckRange(value, hi, lo) */
  function checkRange(v, hi, lo) { return Math.min(hi, Math.max(lo, v)); }

  /* ==========================================================================
   * 3. ini / 配方 取值（與 ht9045_setup_sitemap.js 同一套）
   * ========================================================================== */

  function iniRaw(doc, sec, key) {
    if (!doc || !doc.sections) return null;
    var s = doc.sections[sec];
    if (!s || !(key in s)) return null;
    var v = s[key];
    if (v === null || v === undefined) return null;
    if (typeof v === 'object') {
      return (v.raw !== undefined && v.raw !== null) ? String(v.raw) : String(v.value);
    }
    return String(v);
  }
  function iniInt(doc, sec, key, dflt) {
    var r = iniRaw(doc, sec, key);
    if (r === null || r === '') return dflt;
    var n = parseInt(String(r).trim(), 10);
    return isNaN(n) ? dflt : n;
  }
  function iniNum(doc, sec, key, dflt) {
    var r = iniRaw(doc, sec, key);
    if (r === null || r === '') return dflt;
    var n = parseFloat(String(r).trim());
    return isNaN(n) ? dflt : n;
  }
  function iniBool(doc, sec, key, dflt) {
    var r = iniRaw(doc, sec, key);
    if (r === null || r === '') return dflt;
    r = String(r).trim().toLowerCase();
    return (r === '1' || r === 'true' || r === 'yes');
  }

  /* 見 sitemap 檔頭：golden 讀的是 GPIB 那支 general.ini，web 拿不到，
   * 改用 Handler 自己的 [Version] Model。這裡只需要分出
   * HT9046 / HT9045_12Site（Max=6）與其餘（Max=5）。 */
  function machineFromModel(model) {
    var s = String(model || '').toUpperCase().replace(/[\s_-]/g, '');
    if (/12SITE/.test(s)) return Type_HT9045_12Site;
    if (/9050|1032|7080|502/.test(s)) return 0;
    if (/9046/.test(s)) return Type_HT9046;
    return 0;                                   // 9045 與其他未列入的型號
  }

  function modeFromText(s) {
    if (s === null || s === undefined) return -1;
    s = String(s).trim();
    for (var i = 0; i < MODE_NAMES.length; i++) if (MODE_NAMES[i] === s) return i;
    var n = parseInt(s, 10);                    // 舊配方可能還是數字
    return isNaN(n) ? -1 : n;
  }

  /* ==========================================================================
   * 4. 機台狀態（golden 的那一堆全域）
   * ========================================================================== */

  var CAPS = {
    machineType:  0,
    customerCode: CC_HONPREC_QC,
    indexPressType: 0,
    epMaxKpa: 0,
    /* database.cpp:1104  INSTALL_DOUBLE_EP = Gerneral.ini [System] 同名鍵。
     * edDieForcePerPinGChange 尾端 !=0 才呼叫 CountDieForceKg()。 */
    installDoubleEp: 0,
    /* CosFunction —— 這支 handler 只用得到一個旗標，見 FORCE_PER_PIN_KGF */
    cos: { bForecePerPinKGf: false },
    ini: {
      bD27UseSingleSite85kg:      false,
      bD28MaxForceLimitByDiameter: false,
      bD04MinForceByFile:         false,
      dD04MinForceByFile:         1.0,
      dD04MinForceByFile_20mm:    1.0,
      dD04MinForceByFile_30mm:    1.5,
      dD04MinForceByFile_40mm:    4.0,
      dD04MinForceByFile_60mm:    8.0,
      dD04MinForceByFile_80mm:   15.0
    },
    /* TestIF_File —— golden 有 TestIF（執行期）與 TestIF_File（配方）兩份，
     * CalculateTotalAirForce 讀前者、DutCount 讀後者。web 只有配方那一份，
     * 兩處都用它。沒開過 Set Up 改過模式的機台上兩份是一樣的。 */
    iTestMode: SingleSite,
    iSiteMap: null,                             // [MAX_SOCKET_ROW][MAX_SOCKET_COL]
    bOctal_12Kit: false,
    bQualSite2X2Shift: false,
    bNS7000kit: false,
    bNSKitPress: false,
    /* ainarm9045_2x8_8.cpp 的執行期全域，web 拿不到 —— 見檔頭界線 b. */
    iCloseSiteModeFor2x8: e2x8Standard,
    /* ContactForce.cpp 的 SLKClass：[{sDiameter, dDiameter, iTag, bShow,
     *                                 dContactOffset, dContactOffset_NS}] */
    SLKClass: [],
    /* 配方 contact [Mode] */
    iHeadDeviceCT: 0,
    dKitDiameter: 3.0
  };

  /* DeviceForm / DeviceForm_File 裡這支 handler 真的會動到的欄位 */
  var DeviceForm = { dPress: 0.0, iHeadDeviceCT: 0 };

  /* golden 的成員變數 */
  var dDutCount = 2;        // TfContact::dDutCount
  var dMinForce = 0.0;      // TfContact::dMinForce（GetMinForce 會寫它）
  var iTotalGf  = 0.0;      // TfContact::iTotalGf
  var fShow     = false;    // TfContact::fShow

  function emptySiteMap() {
    var m = [], i, j;
    for (i = 0; i < MAX_SOCKET_ROW; i++) {
      m.push([]);
      for (j = 0; j < MAX_SOCKET_COL; j++) m[i].push(0);
    }
    return m;
  }
  CAPS.iSiteMap = emptySiteMap();

  /* LastSet.bUseTestSocket[arm][row][col]。
   * 來源是執行期 tag site.arm{1,2}.s{1..16}（WebBridgeTags.cpp:789）：
   *   sites 1..8 = row 0，9..16 = row 1，col = (n-1)%8
   * row 2/3 沒有 tag —— 這幾支函式也只問 row 0/1。
   * 接不上時回 true，理由見檔頭界線 d. */
  var bTagsWarned = false;
  function bUseTestSocket(arm, row, col) {
    if (row > 1 || col > 7 || arm > 1) return true;
    var tag = 'site.arm' + (arm + 1) + '.s' + (row * 8 + col + 1);
    if (typeof g.HT9045Tags === 'undefined' || !g.HT9045Tags.has(tag)) {
      if (!bTagsWarned) {
        bTagsWarned = true;
        console.warn('[Contact/SLK] 讀不到 tag ' + tag + ' 等 site 狀態'
                   + '（Handler 沒在跑，或 tag 串流沒接上）—— '
                   + 'LastSet.bUseTestSocket 一律當 true。'
                   + '只影響 [D27] 開啟時 1x2 / 2x1 / 2x2NN 的「關一邊 site 可壓 85kg」分支。');
      }
      return true;
    }
    var v = g.HT9045Tags.get(tag);
    if (v === true || v === 1 || v === '1') return true;
    if (v === false || v === 0 || v === '0') return false;
    return true;
  }

  /* ==========================================================================
   * 5. rgKitDiameter
   * ========================================================================== */

  /* 頁面上的 TRadioGroup 是 <fieldset><label class="rgi"><input type=radio>文字 */
  function rgInputs(id) {
    var el = $(id);
    if (!el) return [];
    return Array.prototype.slice.call(el.querySelectorAll('input[type="radio"]'));
  }
  function rgItems(id) {
    var el = $(id);
    if (!el) return [];
    return Array.prototype.map.call(el.querySelectorAll('label.rgi'),
      function (l) { return l.textContent.trim(); });
  }
  function rgIndex(id) {
    var ins = rgInputs(id);
    for (var i = 0; i < ins.length; i++) if (ins[i].checked) return i;
    return -1;
  }
  function rgSetIndex(id, n) {
    var ins = rgInputs(id);
    for (var i = 0; i < ins.length; i++) ins[i].checked = (i === n);
  }

  /* golden cContact.cpp:143..193 —— CosFunction.bUseDynamicKitDiameter 那條
   * 分支的非 KYEC / 非 ASE-KH 版本。理由見檔頭「一個刻意的加碼」。 */
  function rebuildKitDiameter(types) {
    var el = $('rgKitDiameter');
    if (!el || !types.length) return;
    var body = el.querySelector('.cli');
    if (!body) return;
    var old = rgItems('rgKitDiameter');
    if (old.join(',') === types.join(',')) return;       // 一樣就不動 DOM
    body.innerHTML = '';
    types.forEach(function (t) {
      var lab = document.createElement('label');
      lab.className = 'rgi';
      var inp = document.createElement('input');
      inp.type = 'radio';
      inp.name = 'rg_rgKitDiameter';
      lab.appendChild(inp);
      lab.appendChild(document.createTextNode(t));
      body.appendChild(lab);
    });
    /* TRadioGroup 的 Columns 沒變，但項目變多時一列會擠 —— 讓它自己換行。 */
    body.style.gridTemplateRows = 'repeat(1,1fr)';
    console.info('[Contact/SLK] rgKitDiameter 依 ContactInfo.ini [SLK Type] Type 重建：'
               + old.join(',') + '  ->  ' + types.join(','));
  }

  /* golden cContact.cpp:855..900 的一般分支：
   * atof(item)/10 == DeviceForm_File.dKitDiameter 的那一格就是 ItemIndex。 */
  function selectKitDiameter(dKitDiameter) {
    var items = rgItems('rgKitDiameter');
    for (var i = 0; i < items.length; i++) {
      var d = (items[i] === '40x2') ? 40.2 : atof(items[i]) / 10.0;
      if (d === dKitDiameter) { rgSetIndex('rgKitDiameter', i); return i; }
    }
    console.warn('[Contact/SLK] 配方的 [Mode] Kit Diameter = ' + dKitDiameter
               + '（＝' + (dKitDiameter * 10) + 'mm）在 rgKitDiameter 的選項 '
               + items.join(',') + ' 裡找不到 —— ItemIndex 維持 '
               + rgIndex('rgKitDiameter') + '，最小力量會用預設的 30mm。');
    return rgIndex('rgKitDiameter');
  }

  /* ==========================================================================
   * 6. golden: CalcDeviceForce()          cContact.cpp 22852..22858
   * ========================================================================== */
  function calcDeviceForce(dPinCount, dForcePerPinN, dForcePerPinGf, bReturnKgf) {
    if (bReturnKgf) return dPinCount * dForcePerPinGf * 0.001;   // gf -> Kgf
    return dPinCount * dForcePerPinN;                            // N
  }

  /* ==========================================================================
   * 7. golden: GetMinForce()              cContact.cpp 19047..19112
   * ========================================================================== */
  function getMinForce(dKitDiameter, iTag) {
    var C = CAPS.ini;
    if (dKitDiameter === 20) {
      dMinForce = 0.5;
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile_20mm)
        dMinForce = C.dD04MinForceByFile_20mm;
    } else if (dKitDiameter === 30 || dKitDiameter === 28) {
      dMinForce = 1.5;
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile_30mm)
        dMinForce = C.dD04MinForceByFile_30mm;
    } else if (dKitDiameter === 40) {
      dMinForce = 4.0;
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile_40mm)
        dMinForce = C.dD04MinForceByFile_40mm;
    } else if (dKitDiameter === 60 || dKitDiameter === 58) {
      dMinForce = 8.0;
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile_60mm)
        dMinForce = C.dD04MinForceByFile_60mm;
    } else if (dKitDiameter === 80) {
      dMinForce = 15.0;
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile_80mm)
        dMinForce = C.dD04MinForceByFile_80mm;
    } else {
      /* 不是表列口徑 -> 用 ContactInfo.ini 該口徑的 ContactOffset。
       * SLKClass 空的時候 golden 會直接取值（未定義行為），這裡回 0 並警告。 */
      var k = CAPS.SLKClass[iTag];
      if (!k) {
        console.warn('[Contact/SLK] GetMinForce：SLKClass[' + iTag + '] 不存在'
                   + '（ContactInfo.ini 讀不到或 [SLK Type] Type 是空的），最小力量當 0。');
        dMinForce = 0.0;
      } else {
        dMinForce = CAPS.bNSKitPress ? k.dContactOffset_NS : k.dContactOffset;
      }
      if (C.bD04MinForceByFile && dMinForce < C.dD04MinForceByFile)
        dMinForce = C.dD04MinForceByFile;
    }
    return dMinForce;
  }

  /* ==========================================================================
   * 8. golden: GetMaxIndexForceLimit()    cContact.cpp 18983..19045
   * ========================================================================== */
  function getMaxIndexForceLimit() {
    var kg = INDEX_PRESS_KG[CAPS.indexPressType];
    if (kg !== undefined) return kg;

    /* INDEX_PRESS_TYPE 不在 1..9（含 0 ＝ 沒設定）時才看模式與口徑。
     * ⚠ 這裡 golden 有一條會回 0 的路：dDutCount==1 而且 ItemIndex 既不是
     *   1 也不是 2 時 dMaxLimit 留在 0.0，代表「不夾上限」。照搬。 */
    var dMaxLimit = 0.0;
    if (dDutCount === 1 &&
        (CAPS.iTestMode === DualSite || CAPS.iTestMode === SingleSite ||
         CAPS.iTestMode === QualSite2X2N) &&
        CAPS.ini.bD27UseSingleSite85kg === true) {
      var idx = rgIndex('rgKitDiameter');
      if (idx === 1) dMaxLimit = 55;            // 40mm 浮動頭最多 55kgf
      else if (idx === 2) dMaxLimit = 85;       // 60mm 浮動頭
    } else {
      dMaxLimit = 85;                           // 9045/9046 都是 85kgf
    }
    return dMaxLimit;
  }

  /* ==========================================================================
   * 9. golden: CalculateTotalAirForce()   cContact.cpp 18765..18981
   * ========================================================================== */
  function calculateTotalAirForce(dBallCount, dSingleGf) {
    var Str = '';
    var dTotalForce = 0.0;
    var fComplianceUnit = 1.0, dKitDiameter = 30.0;
    var dMinKgPerHead = 1.0;
    var dNowKgPerHead = 0.0, dHeadMaxForce = 0.0;
    var iTag = -1;
    var dDeviceGf = dBallCount * dSingleGf * 0.001;      // device 要壓的公斤

    if (CAPS.iTestMode === DualSite && CAPS.bQualSite2X2Shift && CAPS.bNS7000kit)
      dDutCount = 4;                                     // kevin 20170513 (wei)

    var cu = COMPLIANCE_UNIT[position()];
    if (cu !== undefined) fComplianceUnit = cu;

    var idx = rgIndex('rgKitDiameter');
    var items = rgItems('rgKitDiameter');
    if (idx >= 0 && idx < items.length) {
      for (var i = 0; i < CAPS.SLKClass.length; i++) {
        var d1 = CAPS.SLKClass[i].dDiameter;
        var d2 = atof(items[idx]);
        if (items[idx] === '40x2') d2 = 402;             // Ifor 20230830：40 倍立缸
        if (d1 === d2) {
          iTag = i;
          dKitDiameter = d2;
          if (dKitDiameter === 402) dKitDiameter = 40;
        }
      }
    }

    colour('edAirForce',         CL_WHITE);              // Steven 20230901：要先重置顏色
    colour('edAirKPA',           CL_WHITE);
    colour('edSetKg',            CL_WHITE);
    colour('edForcePerDeviceKG', CL_WHITE);
    colour('edForcePerDeviceN',  CL_WHITE);

    if (CAPS.ini.bD28MaxForceLimitByDiameter) {          // Steven 20200813：用缸徑計算最大壓力
      vis('lblMaxForcePerIC', true);
      var coefficient = (CAPS.epMaxKpa <= 500) ? 5.0 : 6.0;
      dHeadMaxForce = (((dKitDiameter * dKitDiameter * 3.14) / 4.0)
                       * coefficient * 0.0101972) * fComplianceUnit;
      text('lblMaxForcePerIC',
           'Max force per compliance: ' + fmt2(divNonPcnt(dHeadMaxForce, fComplianceUnit)) + 'kg');
      if (dDeviceGf > dHeadMaxForce) {
        dDeviceGf = dHeadMaxForce;
        colour('edAirKPA',           CL_RED);
        colour('edSetKg',            CL_RED);
        colour('edForcePerDeviceKG', CL_RED);
        colour('edForcePerDeviceN',  CL_RED);
        setField('edForcePerDeviceKG', fmt4(dDeviceGf));
        /* ⚠ golden 這裡是 dDeviceGf*9.8（kgf 換 N），和 ShowArmAndDeviceForce
         *   的 dBallCount*dSingleN 不是同一條公式。照搬，不要「順手統一」。 */
        setField('edForcePerDeviceN', fmt4(dDeviceGf * 9.8));
      }
    } else {
      vis('lblMaxForcePerIC', false);
    }

    dTotalForce = dDeviceGf * dDutCount;

    if (iTag === -1 || iTag > CAPS.SLKClass.length) iTag = 0;
    dMinForce = getMinForce(dKitDiameter, iTag);

    /* 一個浮動頭要分攤幾顆 IC。分母就是該模式的 site 數 × fComplianceUnit。 */
    var n = 0;
    switch (CAPS.iTestMode) {
      case SingleSite:
        n = 1.0; break;                                  // Steven 20231110
      case DualSite:
      case QualSite2X2N:
        n = (((bUseTestSocket(0, 0, 1) === false && bUseTestSocket(1, 0, 1) === false) ||
              (bUseTestSocket(0, 0, 0) === false && bUseTestSocket(1, 0, 0) === false)) &&
             CAPS.ini.bD27UseSingleSite85kg) ? 1.0 : 2.0;
        break;
      case DualSite2x1:
        n = (((bUseTestSocket(0, 1, 0) === false && bUseTestSocket(1, 1, 0) === false) ||
              (bUseTestSocket(0, 0, 0) === false && bUseTestSocket(1, 0, 0) === false)) &&
             CAPS.ini.bD27UseSingleSite85kg) ? 1.0 : 2.0;
        break;
      case _6Site2X3N:
      case TriSite1X3:
        n = 3.0; break;
      case QualSite1X4:
      case QualSite2X2:
      case _8Site1X4:
      case _8Site2X4N:
        n = 4.0; break;
      case _6Site2X3:
        n = 6.0; break;
      case _16Site4X4:
      case _8Site2X4:
        n = CAPS.bOctal_12Kit ? 12.0 : 8.0; break;
      case _10Site2X5:
        n = 10.0; break;
      case _12Site2X6:
        n = 12.0; break;
      case _16Site2X8:
        n = (CAPS.iCloseSiteModeFor2x8 > e2x8Standard) ? 8.0 : 16.0; break;
      case _32Site4X8N:
      case _32Site4X8M:
        n = 16.0; break;                                 // KenHsieh 20230313：NN Mode 32 -> 16
      default:
        /* golden 的 switch 沒有 default —— 沒中的模式 dNowKgPerHead 與
         * dMinKgPerHead 都留在初值（0.0 / 1.0）。照搬。 */
        n = 0.0; break;
    }
    if (n > 0.0) {
      dNowKgPerHead = dTotalForce / (n * fComplianceUnit);
      dMinKgPerHead = dMinForce * (n * fComplianceUnit);
    }

    /* golden 這裡還有 IniConfig.iEP_Min_KG=dMinKgPerHead —— 見檔頭界線 c. */

    Str = 'Min force per compliance: ' + fmt2(dMinForce) + 'kg';

    if (dNowKgPerHead <= dMinForce) {                    // 1 個 arm 要壓的公斤
      dTotalForce = dMinKgPerHead;                       // Steven 20170705 (wei)
      if (CAPS.customerCode === CC_ASE_KaohSiung && CAPS.iTestMode === QualSite2X2) {
        dTotalForce = dDeviceGf * dDutCount;             // kevin 20191204 ASE KH Telix 2x2
        if (dKitDiameter === 30) dMinForce = 1;
        Str = 'Min force per ic: ' + fmt2(dMinForce) + 'kg';
      } else {
        colour('edAirKPA',           CL_YELLOW);
        colour('edSetKg',            CL_YELLOW);
        colour('edForcePerDeviceKG', CL_YELLOW);
        colour('edForcePerDeviceN',  CL_YELLOW);
        Str = 'Min force per compliance: ' + fmt2(dMinForce) + 'kg';
      }
    }
    text('lblMinForce', Str);

    var dMaxLimit = getMaxIndexForceLimit();
    if (dMaxLimit > 0 && dTotalForce > dMaxLimit) {
      colour('edAirForce', CL_RED);
      dTotalForce = dMaxLimit;
    }
    return dTotalForce;
  }

  /* ==========================================================================
   * 10. golden: ShowArmAndDeviceForce()   cContact.cpp 1907..1930
   * ========================================================================== */
  function showArmAndDeviceForce() {
    colour('edForcePerDeviceKG', CL_WHITE);
    colour('edForcePerDeviceN',  CL_WHITE);
    colour('edAirForce',         CL_WHITE);

    var dBallCount = atof(fieldValue('edPinCount'));       // 腳數
    var dSingleN   = atof(fieldValue('edForcePerPinN'));   // 牛頓
    var dSingleGf  = atof(fieldValue('edForcePerPinG'));   // gf

    iTotalGf = calculateTotalAirForce(dBallCount, dSingleGf);
    DeviceForm.dPress = iTotalGf;

    var dDeviceN  = calcDeviceForce(dBallCount, dSingleN, dSingleGf, false);
    var dDeviceGf = calcDeviceForce(dBallCount, dSingleN, dSingleGf, true);
    setField('edForcePerDeviceKG', fmt4(dDeviceGf));       // 一個 ic 要壓的公斤
    setField('edForcePerDeviceN',  fmt4(dDeviceN));        // 一個 ic 要壓的牛頓

    setField('edAirForce',  fmt4(iTotalGf));
    /* JerryYang (Steven) 20161215：由 KG 換算成牛頓，避免下限時 kgf 與 N 不一致 */
    setField('edAirForceN', fmt4(iTotalGf * 9.8));
  }

  /* ==========================================================================
   * 11. golden: DutCount()                cContact.cpp 1970..2083
   * ========================================================================== */
  function dutCount() {
    var S = CAPS.iSiteMap;
    switch (CAPS.iTestMode) {
      case SingleSite:
        dDutCount = 1;
        break;
      case DualSite:                                       // 1x2
      case QualSite2X2N:                                   // Frank 20200520 2X2NN Mode
        if (CAPS.ini.bD27UseSingleSite85kg === true) {
          dDutCount = ((bUseTestSocket(0, 0, 1) === false && bUseTestSocket(1, 0, 1) === false) ||
                       (bUseTestSocket(0, 0, 0) === false && bUseTestSocket(1, 0, 0) === false))
                      ? 1 : 2;
        } else {
          dDutCount = 2;
        }
        break;
      case DualSite2x1:
        if (CAPS.ini.bD27UseSingleSite85kg === true) {
          dDutCount = ((bUseTestSocket(0, 1, 0) === false && bUseTestSocket(1, 1, 0) === false) ||
                       (bUseTestSocket(0, 0, 0) === false && bUseTestSocket(1, 0, 0) === false))
                      ? 1 : 2;
        } else {
          dDutCount = 2;
        }
        break;
      case _6Site2X3N:                                     // Steven 20220425 2X3NN Mode
      case TriSite1X3:
        dDutCount = 3;
        break;
      case QualSite1X4:                                    // 1x4
      case QualSite2X2:                                    // 2x2
      case _8Site1X4:                                      // ChungHung 20150528 海思 8Site1x4
      case _8Site2X4N:                                     // Wei 20231211 2X4NN Mode
        dDutCount = 4;
        break;
      case _6Site2X3:                                      // ChungHung 20140115 2x3_6
        dDutCount = 6;
        break;
      case _8Site2X4:                                      // 2x4
        dDutCount = CAPS.bOctal_12Kit ? 12 : 8;            // ChungHung 20140508 SCK
        break;
      case _10Site2X5:                                     // wei 20190614 10 site
        dDutCount = 10;
        break;
      case _12Site2X6:
        dDutCount = 12;
        break;
      case _16Site2X8:                                     // 2x8
        /* Steven 20221122：16 site 關 site 的壓力顯示修正。
         * 四種「其實只有 8 顆在用」的盤法，任一種成立就當 8。 */
        if (S[0][0] <= 0 && S[0][2] <= 0 && S[0][4] <= 0 && S[0][6] <= 0 &&
            S[1][0] <= 0 && S[1][2] <= 0 && S[1][4] <= 0 && S[1][6] <= 0) {
          dDutCount = 8;
        } else if (S[0][1] <= 0 && S[0][3] <= 0 && S[0][5] <= 0 && S[0][7] <= 0 &&
                   S[1][1] <= 0 && S[1][3] <= 0 && S[1][5] <= 0 && S[1][7] <= 0) {
          dDutCount = 8;
        } else if (S[0][0] <= 0 && S[0][2] <= 0 && S[0][4] <= 0 && S[0][6] <= 0 &&
                   S[1][1] <= 0 && S[1][3] <= 0 && S[1][5] <= 0 && S[1][7] <= 0) {
          dDutCount = 8;
        } else if (S[0][1] <= 0 && S[0][3] <= 0 && S[0][5] <= 0 && S[0][7] <= 0 &&
                   S[1][0] <= 0 && S[1][2] <= 0 && S[1][4] <= 0 && S[1][6] <= 0) {
          dDutCount = 8;
        } else if (CAPS.iCloseSiteModeFor2x8 > e2x8Standard) {   // Steven 20260420 : != --> >
          dDutCount = 8;
        } else {
          dDutCount = 16;
        }
        break;
      case _16Site4X4:                                     // Sam 20190226 16Site4X4
        dDutCount = 8;                                     // KenHsieh 20230313 NN Mode
        break;
      case _32Site4X8N:
        dDutCount = 16;                                    // KenHsieh 20230313 32 -> 16
        break;
      /* ⚠ _32Site4X8M 在 golden 的 switch 裡沒有 case —— dDutCount 留原值。照搬。 */
    }

    /* JerryYang 20171215 (Steven)：Single site 只支援浮動頭 1 對 1 或 2 對 1 */
    if (CAPS.iTestMode === SingleSite) {
      var p = position();
      if (p === 3 || p === 6) setPosition(5, true);        // silent：不要遞迴回 OnChange
      else if (p === 4) setPosition(2, true);
    }

    /* jou 2014-04-24：修正選擇 head 數時 force 值不會馬上改變 */
    DeviceForm.iHeadDeviceCT = position();
    CAPS.iHeadDeviceCT = position();

    /* golden: asStr.sprintf("%sContact%d.bmp", BmpPath, scrbSLK->Position)
     * web：IMG\BMP\Contact<N>.bmp 由 scratchpad/gen_contact_slk_imgs.py 轉成
     *      page/img/dfm_Contact<N>.png（與 _gen_dfm_abs.py 同一條去背規則）。 */
    var img = $('imgSLK');
    if (img) {
      var src = 'img/dfm_Contact' + position() + '.png';
      if (img.getAttribute('src') !== src) img.setAttribute('src', src);
    }
  }

  /* ==========================================================================
   * 12. golden: edAirForceChange()        cContact.cpp 2279..2284
   * ========================================================================== */
  function edAirForceChange() {
    DeviceForm.dPress = atof(fieldValue('edAirForce'));
    /* golden 第三行是 ADAM_WriteVoltage(DeviceForm.dPress) —— 見檔頭界線 a.，
     * 那是真的會讓 EP 出力的動作，留在 C++ 端。 */
  }

  /* ==========================================================================
   * 12a. 同一條計算鏈上的另外三支 OnChange   //Steven 20260921
   * --------------------------------------------------------------------------
   * cContact.dfm 的 OnChange 對照（重點是第三條：edForcePerPinG 與
   * edDieForcePerPinG **共用**同一支 handler，不是筆誤）：
   *
   *   :16197  edPinCount        -> edPinCountChange
   *   :16218  edForcePerPinN    -> edForcePerPinNChange
   *   :16036  edForcePerPinG    -> edDieForcePerPinGChange   <-- 共用
   *   :16579  edDieForcePerPinG -> edDieForcePerPinGChange
   *   :16051  edAirForce        -> edAirForceChange
   * ========================================================================== */

  /* golden: edPinCountChange()            cContact.cpp 1932..1938 */
  function edPinCountChange() {
    if (fShow === false) return;                           // JimmyChiu 20220121
    showArmAndDeviceForce();
  }

  /* golden: edForcePerPinNChange()        cContact.cpp 1939..1954
   * JerryYang 20180515：輸入每個 pin 的力量以 kgf 為主，避免換成牛頓再換回 kgf
   * 會有小數點兩位的差異 —— 所以 bForecePerPinKGf 為 true 時這一支整個不做事。 */
  function edForcePerPinNChange() {
    if (fShow === false) return;
    if (CAPS.cos.bForecePerPinKGf === false) {
      var f = atof(fieldValue('edForcePerPinN'));
      f = f * 1000.0 / 9.8;
      setField('edForcePerPinG', fmt4(f));
      showArmAndDeviceForce();
    }
  }

  /* golden: edDieForcePerPinGChange()     cContact.cpp 1955..1974
   * ⚠ 這一支同時是 edForcePerPinG 的 OnChange。它會做三件事：
   *   1. edDieForcePerPinG(gf) -> edDieForcePerPinN(N)
   *   2. edForcePerPinG(gf)    -> edForcePerPinN(N)      <- 不管是誰觸發的都做
   *   3. ShowArmAndDeviceForce()，再視 INSTALL_DOUBLE_EP 連動 Die Force 公斤數
   * 第 2 步就是 edForcePerPinNChange 的反向換算，兩支互相寫對方的欄位 ——
   * 能收斂是因為 VCL 的 TControl::SetText 只在**文字不同**時才觸發 OnChange
   * （見 §12b 的 watchValue，web 這邊照做了同一道守衛）。 */
  function edDieForcePerPinGChange() {
    if (fShow === false) return;
    var f;
    f = atof(fieldValue('edDieForcePerPinG'));
    f = f * 9.8 / 1000.0;
    setField('edDieForcePerPinN', fmt4(f));

    f = atof(fieldValue('edForcePerPinG'));
    f = f * 9.8 / 1000.0;
    setField('edForcePerPinN', fmt4(f));

    showArmAndDeviceForce();
    if (CAPS.installDoubleEp !== 0)
      countDieForceKg(true);                               // Steven 20210202
  }

  /* golden: CountDieForceKg(bool)         cContact.cpp 18461..18484
   * 這裡只會用到 bByPinCount==true 那一半（另一半是 edDoubleForce 的
   * OnChange 在用的，那支沒接）。 */
  function countDieForceKg(bByPinCount) {
    var dForce, dPinCount, dPinForce, iPinCount;
    if (bByPinCount) {
      dPinCount = atof(fieldValue('edtPinOfDie'));
      dPinForce = atof(fieldValue('edDieForcePerPinG')) / 1000.0;
      dForce = dPinCount * dPinForce;
      setField('edDoubleForce', fmt2(dForce));             // golden: sprintf("%0.2f")
    } else {
      dForce = atof(fieldValue('edDoubleForce')) * 1000;
      dPinForce = atof(fieldValue('edForcePerPinG'));
      iPinCount = (dPinForce !== 0) ? (dForce / dPinForce) : (dForce / 1.0);
      /* golden 是 AnsiString(int)，C 的 double->int 是往零截斷，不是四捨五入 */
      setField('edtPinOfDie', String(Math.trunc(iPinCount)));
    }
  }

  /* ==========================================================================
   * 12b. VCL 的 OnChange 在 web 沒有對應 —— 自己做一個
   * --------------------------------------------------------------------------
   * VCL：TEdit->Text = x 會觸發 OnChange，但 TControl::SetText 是
   *      `if GetText <> Value then ...`，所以**指派相同的字串不會觸發**。
   *      整條換算鏈能收斂，靠的就是這一道守衛。
   *
   * web：欄位是 readonly，只有 qwerty.js 的 commit() 會寫值，而它就是
   *      `tgt.value = val`，不發任何事件（qwerty.js:74）。DOM 也不會因為
   *      程式指派 .value 就送 input/change。所以這裡把該欄位的 value
   *      改成自己的 accessor，指派時比對舊值、不同才呼叫 handler ——
   *      語意與 VCL 完全一樣，而且 qwerty、兩支接線的 load()、Console 手動
   *      指派，三種來源都涵蓋。
   *
   * ⚠ 不要改 qwerty.js 去發事件。那支是全站共用的，改它等於幫所有頁面的
   *   每一個輸入框都加上事件，影響面遠大於這一頁。
   *
   * ⚠ 遞迴深度守衛是 golden 沒有的。golden 靠「相同文字不觸發」＋ gf/N 的
   *   4 位小數來回換算剛好是不動點來收斂；浮點來回極少數情況會差一個 ulp，
   *   在瀏覽器裡那就是整頁卡死。所以加一道上限，而且踩到一定寫 console.error
   *   —— 這種事不可以靜默。
   */
  var CHAIN_DEPTH = 0, CHAIN_MAX = 16;

  function fireChange(name, fn) {
    if (CHAIN_DEPTH >= CHAIN_MAX) {
      console.error('[Contact/SLK] OnChange 連鎖超過 ' + CHAIN_MAX + ' 層，於 '
                   + name + ' 中止。這代表兩個欄位的換算沒有收斂 —— '
                   + '請把當下的 edForcePerPinN / edForcePerPinG 值回報。');
      return;
    }
    CHAIN_DEPTH++;
    try { fn(); } finally { CHAIN_DEPTH--; }
  }

  /* 把 el.value 換成會通知的 accessor。回傳是否掛上。 */
  function watchValue(id, name, fn) {
    var el = $(id);
    if (!el) { console.warn('[Contact/SLK] 頁面上沒有 ' + id + '，OnChange 沒接。'); return false; }
    var proto = Object.getPrototypeOf(el);
    var D = (proto && Object.getOwnPropertyDescriptor(proto, 'value'))
         || (g.HTMLInputElement && Object.getOwnPropertyDescriptor(g.HTMLInputElement.prototype, 'value'))
         || null;
    var getv, setv;
    if (D && D.get && D.set) {
      getv = function () { return D.get.call(el); };
      setv = function (v) { D.set.call(el, v); };
    } else {
      /* 沒有原生 accessor（例如測試用的假 DOM）：自己留一個槽 */
      var slot = el.value;
      getv = function () { return slot; };
      setv = function (v) { slot = v; };
    }
    Object.defineProperty(el, 'value', {
      configurable: true, enumerable: true,
      get: getv,
      set: function (v) {
        var old = getv();
        setv(v);
        if (String(v) !== String(old)) fireChange(name, fn);   // <- VCL 的那道守衛
      }
    });
    /* 真人直接打字（欄位現在是 readonly，但別的接線哪天拿掉 readonly 就會用到） */
    el.addEventListener('input',  function () { fireChange(name, fn); });
    el.addEventListener('change', function () { fireChange(name, fn); });
    return true;
  }

  function attachChangeHandlers() {
    var n = 0;
    if (watchValue('edPinCount',        'edPinCountChange',        edPinCountChange))        n++;
    if (watchValue('edForcePerPinN',    'edForcePerPinNChange',    edForcePerPinNChange))    n++;
    if (watchValue('edForcePerPinG',    'edDieForcePerPinGChange', edDieForcePerPinGChange)) n++;
    if (watchValue('edDieForcePerPinG', 'edDieForcePerPinGChange', edDieForcePerPinGChange)) n++;
    if (watchValue('edAirForce',        'edAirForceChange',        edAirForceChange))        n++;

    /* golden cContact.cpp:946..950（DoIniDataToForm）：以 kgf 為主的客戶，
     * edForcePerPinN 變成唯讀。contact_wire 的 attachKeyboards() 已經把所有
     * 文字框設成 readonly 並掛上小鍵盤，所以這裡改的是「按了不開小鍵盤」。 */
    if (CAPS.cos.bForecePerPinKGf) {
      var el = $('edForcePerPinN');
      if (el) {
        el.disabled = true;
        el.style.cursor = 'default';
        el.title = (el.title || '') + '（CosFunction.bForecePerPinKGf：'
                 + '這個客戶碼只輸入 gf，N 由 gf 換算，不可直接改）';
      }
    }
    return n;
  }

  /* ==========================================================================
   * 13. golden: scrbSLKChange()           cContact.cpp 2085..2092
   * ========================================================================== */
  function scrbSLKChange() {
    if (fShow === false) return;                           // JimmyChiu 20220121
    dutCount();
    showArmAndDeviceForce();                               // JimmyChiu 20220121
    edAirForceChange();                                    // jou 2014-04-24
  }

  /* ==========================================================================
   * 14. TScrollBar（Kind=sbVertical, Min=2, Max=5/6, PageSize=0）
   * ========================================================================== */

  var SB = { min: 2, max: 5, pos: 2, el: null, thumb: null };

  function position() { return SB.pos; }

  /* silent=true 只改值與外觀，不觸發 OnChange —— 給 DutCount 自己夾限
   * Position 時用（golden 靠「改 Position 會再進一次事件」，這裡不要遞迴）。 */
  function setPosition(v, silent) {
    v = Math.max(SB.min, Math.min(SB.max, Math.round(v)));
    var changed = (v !== SB.pos);
    SB.pos = v;
    layoutThumb();
    if (SB.el) {
      SB.el.setAttribute('aria-valuenow', String(v));
      SB.el.setAttribute('aria-valuetext', SLK_HINT[v] || String(v));
      SB.el.title = 'scrbSLK : TScrollBar  —  ' + (SLK_HINT[v] || '');
    }
    if (changed && !silent) scrbSLKChange();
    return SB.pos;
  }

  var SLK_HINT = {
    2: '1 Device with 1 Compliance Unit',
    3: '2 Device with 1 Compliance Unit',
    4: '4 Device with 1 Compliance Unit',
    5: '2 Device with 4 Compliance Unit',
    6: '8 Device with 1 Compliance Unit'
  };

  var ARROW = 14;      // 上下箭頭高度（VCL 是方形按鈕，寬度＝捲軸寬 17）

  function trackGeom() {
    var h = SB.el ? SB.el.clientHeight : 0;
    return { top: ARROW, height: Math.max(0, h - ARROW * 2) };
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
    var el = $('scrbSLK');
    if (!el) { console.warn('[Contact/SLK] 找不到 scrbSLK，捲軸沒有接。'); return false; }
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

    /* 槽上點擊：VCL 的 LargeChange 預設 1，一次一格，和箭頭同幅度。 */
    el.addEventListener('mousedown', function (e) {
      if (e.target === up || e.target === down || e.target === thumb) return;
      var r = el.getBoundingClientRect(), y = e.clientY - r.top;
      if (y < ARROW || y > r.height - ARROW) return;
      step(e.clientY < thumb.getBoundingClientRect().top ? -1 : 1);
      e.preventDefault();
    });

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
      if (k === 'ArrowDown' || k === 'ArrowRight' || k === 'PageDown') step(1);
      else if (k === 'ArrowUp' || k === 'ArrowLeft' || k === 'PageUp') step(-1);
      else if (k === 'Home') setPosition(SB.min);
      else if (k === 'End') setPosition(SB.max);
      else return;
      e.preventDefault();
    });

    if (g.ResizeObserver) new g.ResizeObserver(layoutThumb).observe(el);
    return true;
  }

  /* ==========================================================================
   * 15. 讀資料
   * ========================================================================== */

  /* ContactForce.cpp 455..492 的 SLKClass 建法（非 EP_Install==5 / 非 CC_ASE_SG），
   * 加上 1038..1065 的 ContactOffset 讀取。 */
  function buildSLKClass(ci) {
    var types   = String(iniRaw(ci, 'SLK Type', 'Type')    || '').split(',');
    var visible = String(iniRaw(ci, 'SLK Type', 'Visible') || '').split(',');
    var out = [];
    for (var i = 0; i < types.length; i++) {
      var s = String(types[i]).trim();
      if (s === '' || atof(s) <= 15.0) continue;           // golden 的門檻
      var sDia = (s === '402') ? '40x2' : s;               // Ifor 20230830
      var dDia = (s === '402') ? 402 : atof(s);
      var sec  = 'Diameter_' + dDia.toFixed(3) + 'mm';
      out.push({
        sDiameter: sDia,
        dDiameter: dDia,
        iTag: i,
        bShow: String(visible[i] || '').trim() === '1',
        dContactOffset:    checkRange(iniNum(ci, sec, 'ContactOffset',    0.0), 10.0, -10.0),
        dContactOffset_NS: checkRange(iniNum(ci, sec, 'ContactOffset_NS', 0.0), 10.0, -10.0)
      });
    }
    return out;
  }

  function loadCaps() {
    var soft = function (p) {
      return p.then(function (r) { return r; }, function (e) { return { __err: e }; });
    };
    var bad = [];

    if (typeof g.HT9045System === 'undefined' || typeof g.HT9045Recipe === 'undefined') {
      console.warn('[Contact/SLK] 沒有 HT9045System / HT9045Recipe'
                 + '（ht9045_recipe_client.js 未載入），全部用保底值。');
      return Promise.resolve(false);
    }

    return Promise.all([
      soft(g.HT9045System.read('gerneral')),
      soft(g.HT9045System.read('config')),
      soft(g.HT9045System.read('contactInfo')),
      soft(g.HT9045Recipe.read('handlerCondition')),
      soft(g.HT9045Recipe.read('contact'))
    ]).then(function (r) {
      var gen = r[0], cfg = r[1], ci = r[2], hc = r[3], ct = r[4];

      if (gen && !gen.__err) {
        CAPS.machineType    = machineFromModel(iniRaw(gen, 'Version', 'Model'));
        CAPS.customerCode   = iniInt(gen, 'System', 'CUSTOMER_CODE', CC_HONPREC_QC);
        CAPS.indexPressType = iniInt(gen, 'System', 'INDEX_PRESS_TYPE', 0);
        CAPS.epMaxKpa       = iniInt(gen, 'System', 'EP_MAXKPA', 0);
        CAPS.installDoubleEp = iniInt(gen, 'System', 'INSTALL_DOUBLE_EP', 0);   // database.cpp:1104
        CAPS.cos.bForecePerPinKGf = !!FORCE_PER_PIN_KGF[CAPS.customerCode];
      } else { bad.push('Gerneral.ini'); }

      if (cfg && !cfg.__err) {
        var C = CAPS.ini;
        C.bD27UseSingleSite85kg =
          iniBool(cfg, 'Index', 'UseSingleSite85kg', false);
        C.bD28MaxForceLimitByDiameter =
          iniBool(cfg, 'Index', 'bD28MaxForceLimitByDiameter', false);
        C.bD04MinForceByFile =
          iniBool(cfg, 'Contact Force', 'bD04MinForceByFile', false);
        C.dD04MinForceByFile      = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile',       1.0);
        C.dD04MinForceByFile_20mm = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile_20mm',  1.0);
        C.dD04MinForceByFile_30mm = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile_30mm',  1.5);
        C.dD04MinForceByFile_40mm = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile_40mm',  4.0);
        C.dD04MinForceByFile_60mm = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile_60mm',  8.0);
        C.dD04MinForceByFile_80mm = iniNum(cfg, 'Contact Force', 'dD04MinForceByFile_80mm', 15.0);
      } else { bad.push('config.ini'); }

      if (ci && !ci.__err) {
        CAPS.SLKClass = buildSLKClass(ci);
        rebuildKitDiameter(CAPS.SLKClass.map(function (k) { return k.sDiameter; }));
      } else { bad.push('ContactInfo.ini'); }

      if (hc && !hc.__err) {
        var m = modeFromText(iniRaw(hc, 'Configuration', 'Test Mode'));
        if (m < 0) {
          bad.push('HandlerCondition.Data 的 [Configuration] Test Mode');
        } else {
          CAPS.iTestMode = m;
        }
        /* cSetUp.cpp:2361  str.sprintf("Site %c%c", i+'A', j+'a')
         * ⚠ 機台上的檔案 C/D 兩列是 "Site CA".."Site DH"（大寫），和 golden 的
         *   鍵名對不上，所以那兩列 golden 自己也讀成 0。這裡照 golden 的鍵名讀，
         *   不要「順手修正大小寫」—— 這幾支函式只問第 0/1 列。 */
        var A = 'A'.charCodeAt(0), a = 'a'.charCodeAt(0);
        for (var i = 0; i < MAX_SOCKET_ROW; i++) {
          for (var j = 0; j < MAX_SOCKET_COL; j++) {
            var v = iniInt(hc, 'Configuration',
                           'Site ' + String.fromCharCode(A + i) + String.fromCharCode(a + j), 0);
            CAPS.iSiteMap[i][j] = (v <= 0) ? 0 : v;        // kevin 20150427
          }
        }
        CAPS.bOctal_12Kit      = iniBool(hc, 'Configuration', 'Octal 12Kit',        false);
        CAPS.bQualSite2X2Shift = iniBool(hc, 'Configuration', 'QualSite2X2 Shift',  false);
        CAPS.bNS7000kit        = iniBool(hc, 'Configuration', 'NS7000 bias kit',    false);
        CAPS.bNSKitPress       = iniBool(hc, 'Configuration', 'bNSKitPress',        false);
      } else { bad.push('HandlerCondition.Data'); }

      if (ct && !ct.__err) {
        CAPS.iHeadDeviceCT = iniInt(ct, 'Mode', 'Head Device Mode', 0);
        CAPS.dKitDiameter  = iniNum(ct, 'Mode', 'Kit Diameter',     3.0);
      } else { bad.push('Contact.Data'); }

      if (bad.length) {
        console.warn('[Contact/SLK] 讀不到 ' + bad.join(' / ')
                   + '，這幾項用保底值。畫面上的 Contact Force 可能與機台不同。');
      }
      applyOverride();
      return bad.length === 0;
    });
  }

  /* 人工覆寫：window.HT9045ContactCaps 的鍵一律蓋掉自動判讀的結果。
   * 最常用的是 iTestMode 與 iCloseSiteModeFor2x8（後者 web 本來就拿不到）。
   * 覆寫了什麼一定寫到 console —— 覆寫本身很容易變成下一個查不出來的怪現象。 */
  function applyOverride() {
    var ov = g.HT9045ContactCaps;
    if (!ov) return;
    Object.keys(ov).forEach(function (k) {
      if (k === 'ini' && ov.ini) {
        Object.keys(ov.ini).forEach(function (j) { CAPS.ini[j] = ov.ini[j]; });
      } else {
        CAPS[k] = ov[k];
      }
    });
    console.info('[Contact/SLK] 套用 window.HT9045ContactCaps 覆寫：'
               + Object.keys(ov).join(', '));
  }

  /* ==========================================================================
   * 16. 存檔：[Mode] Head Device Mode / Kit Diameter / KitDiameterMode
   * --------------------------------------------------------------------------
   * golden TfContact::SaveSetupFile()：
   *   :14381  WriteIniData(szDir,"Mode","Head Device Mode", scrbSLK->Position);
   *   :14389  if     (ItemIndex==2) Kit Diameter = 6.0
   *   :14393  else if(ItemIndex==1) Kit Diameter = 4.0
   *   :14401  else if(ItemIndex==0) Kit Diameter = 3.0
   *   :14405  else  items[ItemIndex]=="40x2" ? 40.2 : atof(items[ItemIndex])/10.0
   *   :14418  WriteIniData(szDir,"Mode","KitDiameterMode", ItemIndex);
   *
   * ⚠ 前三個 ItemIndex 是寫死的 0/1/2 -> 3.0/4.0/6.0，也就是 golden 假設頭三顆
   *   一定是 30/40/60。這台機的選項是 30,40,60,56,80，所以第 3 顆是 5.6、
   *   第 4 顆是 8.0（走 else 那條 /10.0）。照搬，不要「順手改成一律 /10」——
   *   改了會讓非 30/40/60 開頭的機台存出跟 golden 不一樣的值。
   *
   * ⚠ 格式要對齊 golden 的 WriteIniData：common.cpp:887 double 是 "%0.4f"，
   *   common.cpp:691 int 就是整數。格式不同會讓 preview 每次都報 changed。
   *
   * 掛法是 ht9045_contact_wire.js 的 addCollector() —— 走它原本的
   * preview -> 人確認 -> 寫入 -> 重讀，不另外開一條側門。
   * ========================================================================== */

  function kitDiameterForSave(idx, items) {
    if (idx === 2) return 6.0;
    if (idx === 1) return 4.0;
    if (idx === 0) return 3.0;
    if (items[idx] === '40x2') return 40.2;                // Ifor 20230830：40 倍立缸
    return atof(items[idx]) / 10.0;
  }

  function collectSaveEdits() {
    var out = { Mode: {} };
    out.Mode['Head Device Mode'] = String(position());     // cContact.cpp:14381

    var idx = rgIndex('rgKitDiameter'), items = rgItems('rgKitDiameter');
    if (idx < 0 || idx >= items.length) {
      /* golden 在這裡會拿 Items->Strings[-1]，是未定義行為。web 不猜一個值
       * 寫進機台配方 —— 少寫兩個鍵、把原因講出來，比寫錯安全。 */
      console.warn('[Contact/SLK] rgKitDiameter 沒有選中任何一顆，'
                 + '這次存檔略過 [Mode] Kit Diameter 與 KitDiameterMode。');
      return out;
    }
    out.Mode['Kit Diameter']    = kitDiameterForSave(idx, items).toFixed(4);
    out.Mode['KitDiameterMode'] = String(idx);             // cContact.cpp:14418
    return out;
  }

  function attachSave() {
    if (!(g.HT9045Contact && g.HT9045Contact.addCollector)) {
      console.warn('[Contact/SLK] ht9045_contact_wire.js 沒有 addCollector()'
                 + '（版本太舊？）—— Head Device Mode / Kit Diameter 按 Save '
                 + '不會寫回配方。');
      return false;
    }
    g.HT9045Contact.addCollector('contact-slk', collectSaveEdits);
    return true;
  }

  /* ==========================================================================
   * 17. 起始
   * ========================================================================== */

  /* 等這一頁其他兩支接線把欄位填完 —— 理由見檔頭「載入順序」。
   * 兩支都沒有就直接往下走（欄位會是 dfm 的展示值，算出來沒有意義，會警告）。 */
  function waitForOtherWires() {
    var ps = [], names = [];
    if (g.HT9045Contact && g.HT9045Contact.load) {
      ps.push(g.HT9045Contact.load().catch(function () {})); names.push('HT9045Contact');
    }
    if (g.HT9045Page && g.HT9045Page.load) {
      ps.push(g.HT9045Page.load().catch(function () {})); names.push('HT9045Page');
    }
    if (!ps.length) {
      console.warn('[Contact/SLK] 這一頁沒有 HT9045Contact / HT9045Page —— '
                 + '欄位不會被配方填過，算出來的 Contact Force 只是 dfm 的展示值。');
      return Promise.resolve();
    }
    return Promise.all(ps).then(function () { return names; });
  }

  function attach() {
    if (!buildScrollBar()) return;

    Promise.all([loadCaps(), waitForOtherWires()]).then(function () {
      /* golden cContact.cpp 1174..1178 的順序，一步都不要換：
       *   ShowArmAndDeviceForce();   <- :1173（fShow 還是 false，所以其實沒動作）
       *   fShow=true;                <- :1175 「要在 ScrollBar1 被修改前」
       *   scrbSLK->Visible=true;     <- :1176
       *   scrbSLK->Position=DeviceForm_File.iHeadDeviceCT;   <- :1177
       *   scrbSLKChange(this);       <- :1178 明確再叫一次 */
      SB.max = (CAPS.machineType === Type_HT9046 ||
                CAPS.machineType === Type_HT9045_12Site) ? 6 : 5;   // cContact.cpp:251..256

      selectKitDiameter(CAPS.dKitDiameter);

      /* OnChange 要在 fShow=true **之前**掛好，理由和 golden 把 fShow=true 放在
       * :1175（「要在 ScrollBar1 被修改前」）一樣：掛的動作本身不會觸發，
       * 而 fShow 還是 false 時就算誤觸也會被每一支 handler 的第一行擋掉。 */
      var chn = attachChangeHandlers();
      var saveOk = attachSave();

      fShow = true;
      vis('scrbSLK', true);
      setPosition(CAPS.iHeadDeviceCT, true);   // silent：下一行才是 golden 的那一次呼叫
      scrbSLKChange();

      console.info('[Contact/SLK] OnChange 掛上 ' + chn + ' 個欄位'
                 + '（bForecePerPinKGf=' + CAPS.cos.bForecePerPinKGf
                 + '  INSTALL_DOUBLE_EP=' + CAPS.installDoubleEp + '）；'
                 + (saveOk ? '存檔已接上 contact_wire 的 addCollector。'
                           : '⚠ 存檔沒接上。'));
      console.info('[Contact/SLK] 接好了：Position=' + position()
                 + '（' + (SLK_HINT[position()] || '?') + '）'
                 + '  Min=' + SB.min + ' Max=' + SB.max
                 + '  TestMode=' + (MODE_NAMES[CAPS.iTestMode] || CAPS.iTestMode)
                 + '  dDutCount=' + dDutCount
                 + '  KitDiameter=' + (rgItems('rgKitDiameter')[rgIndex('rgKitDiameter')] || '?') + 'mm'
                 + '  MinForce=' + fmt2(dMinForce) + 'kg'
                 + '  AirForce=' + fmt4(iTotalGf) + 'kg');
    }).catch(function (e) {
      console.error('[Contact/SLK] 起始失敗：', e);
    });
  }

  /* ==========================================================================
   * 18. 開窗重算 —— golden FormShow 每次開窗都跑一次 scrbSLKChange
   *     AI(W906-D019) 20260930（todo D-019，St01）
   * --------------------------------------------------------------------------
   * golden V912 cContact.cpp（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy）：
   *   :1094  TfContact::FormShow —— 每一次 Show() 都跑（關窗 FormClose :1842 設 fShow=false :1847）
   *   :1181  ShowArmAndDeviceForce();
   *   :1183  fShow=true;   :1185  scrbSLK->Position=DeviceForm_File.iHeadDeviceCT;   :1186  scrbSLKChange(this);
   *   :2103  scrbSLKChange = DutCount()（:1988；TestIF_File.iTestMode／iSiteMap、LastSet.bUseTestSocket :2001-2002／:2015-2016）
   *          -> ShowArmAndDeviceForce()（:1925 -> CalculateTotalAirForce，再讀一次 LastSet.bUseTestSocket :19064-19065／:19078-19079）
   *          -> edAirForceChange()
   *   cContact.cpp 裡會重算的只有 FormShow（:1181／:1186）、三支欄位 OnChange（:1953／:1966／:1983）、兩支 OnClick
   *   （rgKitDiameterClick :15542、rgOutKitDiameterClick :17427）；別的檔只在自己的動作裡叫（main.cpp:9386-9387
   *   DoReadLastData、cSetUp.cpp:3633 sbUpdateClick、cBuilder.cpp:516、ContactForce.cpp:921 FormClose、
   *   AutoClean\uCleaning.cpp:1746）—— 沒有 Timer；畫面開著時 LastSet.bUseTestSocket 變了，golden 也不重算。
   * web：background.html 關「非 lazy」視窗只是把 iframe 藏起來（頁面不重載），§17 那一次只在載入時跑；之後每次開窗，
   *   site 開關（tag site.arm{1,2}.s{n}，§4 bUseTestSocket）可能已經變了，畫面卻還是上一次的。現在：
   *   a. HT_WIN（background.html postWinState {type:'HT_WIN', id, open, state, initial}；open＝'open' 或 'minimized'）
   *      「關 -> 開」的邊緣：立刻用目前的值照 :1186 重算一次（＝golden FormShow）。最小化不算關（golden 最小化不跑
   *      FormClose／FormShow），開 <-> 最小化 不是邊緣。只看 open，跟引擎 H4（ht9045_wire_engine.js 開窗重讀）同一個判斷。
   *   b. 邊緣之後 REOPEN_FOLLOW_MS（2 秒）內，任何一個 site.arm* 變了就「再」重算一次，然後收手。用 subscribe，不在
   *      HT_WIN 裡 get()：stage F（筆電，main e977284a，ht9045_link.js Hub.windowState）關著的視窗收不到 tag frame，
   *      開窗那一刻才補一份整張的合成快照；外框先送 HT_WIN（background.html:697）、快照走 MessagePort，沒有順序保證 ——
   *      a. 拿到的可能是關窗前的舊值，b. 等的就是那份快照。只補一次、只等 2 秒：快照是一幀；之後的變動 golden 本來就不重算。
   *      今天的 review6（沒有 stage F）tag 關著也照送，a. 已經是新值；b. 只在開窗 2 秒內剛好有人改 site 時多算一次，結果一樣。
   *   c. 從沒收過 HT_WIN（單獨開頁、被別的頁嵌著）＝照舊：不訂閱、不重算，§17 那一次就是全部。
   *   d. 「沒收過 -> 開」（lazy 視窗第一次開、或開著時重新整理：外框在 iframe load 補送 initial:true）不算邊緣 ——
   *      §17 載入那一次就是這一次 FormShow。
   *   e. 邊緣比 §17 早到（fShow 還是 false）：a. 跳過（§17 會用它當下的值算），b. 照樣掛著。
   *   沒做（不是這一段的範圍）：golden FormClose :1848-1849 的 ReadFile＋DoIniDataToForm（丟掉沒存的改動）與 FormShow :1185
   *   把 Position 放回檔案值 —— 網頁的欄位由引擎開窗時的 editlist.get（C 路）重填，本檔的 Position（SB.pos）不跟著回檔案值。
   * ========================================================================== */
  var REOPEN_FOLLOW_MS = 2000;
  var SITE_TAG = /^site\.arm[12]\.s\d+$/;
  var WIN = { open: null, follow: false, timer: null, subscribed: false, shows: 0, follows: 0 };   // open：null＝沒收過 HT_WIN

  function reopenRecalc(which) {
    if (fShow === false) return false;                     // e.：§17 還沒算完
    if (which === 'follow') WIN.follows++; else WIN.shows++;
    scrbSLKChange();                                       // golden :1186
    return true;
  }
  function followStop() {
    WIN.follow = false;
    if (WIN.timer !== null) { clearTimeout(WIN.timer); WIN.timer = null; }
  }
  function followStart() {
    followStop();
    WIN.follow = true;
    WIN.timer = setTimeout(function () { WIN.timer = null; WIN.follow = false; }, REOPEN_FOLLOW_MS);
  }
  /* HT9045Tags.subscribe 的 fn(changed, all)：changed 只有這一幀真的變了的 tag（ht9045_recipe_client.js tagNotify） */
  function onSiteTags(changed) {
    if (!WIN.follow || !changed) return;                   // 訂閱當下的補送、2 秒之後、關著：都不算
    var hit = Object.keys(changed).some(function (k) { return SITE_TAG.test(k); });
    if (hit && reopenRecalc('follow')) followStop();       // b.：只補一次
  }
  function subscribeSites() {
    if (WIN.subscribed || !g.HT9045Tags || typeof g.HT9045Tags.subscribe !== 'function') return;
    WIN.subscribed = true;
    g.HT9045Tags.subscribe(onSiteTags);
  }
  function onHtWin(m) {
    var was = WIN.open, now = !!m.open;
    WIN.open = now;
    subscribeSites();                                      // 先訂閱（訂閱當下的補送 follow 還是 false）
    if (was === false && now) {
      followStart();
      reopenRecalc('open');                                // a.
    } else if (!now) {
      followStop();                                        // 2 秒內又關了：不補
    }
  }
  /* 監聽器在載入時就掛：外框在 iframe load 補送的 HT_WIN（initial:true）可能比 attach() 早到（同引擎 H4）。
   * 只收外框送的（同 HW.MotorTest.html:1962）。 */
  if (g.addEventListener) {
    g.addEventListener('message', function (ev) {
      var m = ev && ev.data;
      if (!m || m.type !== 'HT_WIN') return;
      if (!g.parent || g.parent === g || ev.source !== g.parent) return;
      onHtWin(m);
    });
  }

  /* 對外，方便在 Console 手動操作／覆寫後重算 */
  g.HT9045ContactSLK = {
    position: position,
    setPosition: function (v) { return setPosition(v); },
    recalc: scrbSLKChange,
    /* 存檔會送出哪三個鍵 —— 按 Save 之前想先看一眼時用 */
    saveEdits: collectSaveEdits,
    reload: function () { return loadCaps().then(function () { scrbSLKChange(); return CAPS; }); },
    caps: function () { return CAPS; },
    /* AI(W906-D019) 20260930：§18 開窗重算的狀態（winOpen：null＝沒收過 HT_WIN；shows＝開窗邊緣重算幾次；follows＝邊緣後 2 秒內 site.arm* 變了補算幾次） */
    reopen: function () { return { winOpen: WIN.open, follow: WIN.follow, shows: WIN.shows, follows: WIN.follows }; },
    state: function () {
      return { fShow: fShow, dDutCount: dDutCount, dMinForce: dMinForce,
               iTotalGf: iTotalGf, dPress: DeviceForm.dPress,
               iHeadDeviceCT: DeviceForm.iHeadDeviceCT,
               fComplianceUnit: COMPLIANCE_UNIT[position()] };
    }
  };

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', attach);
  } else {
    attach();
  }
})(window);
