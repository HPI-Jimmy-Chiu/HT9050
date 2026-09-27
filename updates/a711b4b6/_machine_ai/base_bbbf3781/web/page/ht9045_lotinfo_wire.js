/* ht9045_lotinfo_wire.js -- Data.LotInfo.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * Steven 20260925 (Data.LotInfo)
 * 手寫接線檔（不是 gen_wire.py 產生的）。golden = V912 uLotInfo.cpp／.h／.dfm（HT9011UC_Code_V3.33.912.0_20260908_Jimmy）。
 * 探針：HT9011UC_Cpp_V3.33.906.0\tools\webprobe\data_lotinfo_probe.py
 *
 * ── Lot 分頁（20260925 上午那一波，未改）───────────────────────────────────────
 * 對照表（golden V912 uLotInfo.dfm -> 移植樹 fLotInfo 元件 -> tag）
 *   Lot No      edtSysLotID       dfm:460   「Lot ID :」       -> lot.id
 *   Device      lbledtDeviceName  dfm:1071  「Device Name」    -> lot.device
 *                 （Lot 分頁的子分頁 tsChipAdv:973，一般客戶改名「Lot Info」顯示，uLotInfo.cpp:1281-1296。
 *                   不是另一個分頁 tsDeviceInfo 的 edDeviceName —— 那個是 FTP／RMS 下載配方用的裝置名，
 *                   只在 IniConfig.bEnableRms 時顯示，uLotInfo.cpp:374）
 *   Run Mode    cbRunMode         dfm:529   「Run Mode :」     -> lot.runMode
 *                 （golden Lot 分頁有這一格；舊註解說「golden 無對應欄位」是錯的）
 *   Operator    edtSysOperatorID  dfm:517   「Operator ID :」  -> lot.operator
 *   Start Time  lbledtStarTime    dfm:976   「Start Time」     -> lot.startTime
 *                 （config.ini [Lot Info] Start Time，格式 yyyymmdd_hhnnss，uLotInfo.cpp:1630-1634；
 *                   不是 LotStartTime 那個鍵 —— 那是 RunInfo.LotStartTime，golden Lot 分頁不顯示它）
 *   Loading     fSortCT pnlLoader=LastSet.SendCT[0]（V912 cSortCT.cpp:213）-> sort.loading（既有 tag）
 *                 （golden uLotInfo 的 Lot 分頁沒有投入數；uLotInfo 自己的 pnlLoader 是 RFID Loader ID，
 *                   只有 USE_RFID_READER 客戶才有 —— 客戶專屬，跳過）
 *
 * ⚠ 開機到第一次 lot.start 之前，lot.* 是 null（畫面 "---"）。
 *   golden 開機會 TfMain::DoShowUserDefFrom -> fLotInfo->Show()（main.cpp:9238）-> FormShow ->
 *   SetLotStart(..., true)（uLotInfo.cpp:1199）把 config.ini 讀進表單；移植樹那一步是 GATE WC-19
 *   （它同時會把 RunInfo.bLotStart 打開），所以 C++ 在那之前不知道表單上「應該」是什麼，送 null。
 *   liveness 的完整理由在 WebBridgeTags.cpp 檔尾 W906_StageLotInfoTags 的註解。
 *
 * ── 其餘分頁（Steven 20260925 下午，Data.LotInfo 其餘分頁）─────────────────────────
 *   C++ 本體：forms/fLotInfo.cpp 檔尾（golden 敘述逐行翻）；tag：WebBridgeTags.cpp 檔尾 W906_StageLotInfoTabTags；
 *   WS：WebLotInfo.cpp W906_LotInfoOp（指令 lotinfo.op）。網頁不做任何判斷、不算任何數字，只貼 C++ 送來的東西。
 *   lot.tab.<dfm 名>            27＋8＋2 個 TTabSheet 的 TabVisible —— false 才藏頁籤；null（不可知）照樣顯示（不把「不知道」當「關」）
 *   lot.tab.active              golden FormShow 開窗時選中的分頁 —— 第一次收到時切過去一次（之後使用者自己切）
 *   lot.tab.tsChipAdv.caption   子分頁標題（一般客戶 "Lot Info"）
 *   lot.endTime … lot.step      Lot Info 子分頁另外 12 格（同 lot.* 的 liveness）
 *   lot.atc.*                   ATC 分頁（標題、Working Temperature、ATC On/Off Line＋底色、ATC Power 燈、各元件／CH 可見度）
 *   lot.barcode.r<R>.c<C>       sgBarcode 格子（golden DoBarcodeCount 的字）；Clear Count = lotinfo.op barcode.clearCount（兩段式）
 *   lot.testerLog.*             labTCPIPStatus（字＋底色）、mmTesterLog 行數與最後 50 行；「全部」= lotinfo.op testerLog.get
 *   Selection 分頁              切過去 = lotinfo.op selection.get（golden pgLotinfoChange 讀 config\Security_new.def）；
 *                               Save = lotinfo.op selection.save（golden btnSaveClick 寫回）—— 看不到的分頁 C++ 回 guard tab-hidden
 *   ⚠ lotinfo.op 的伺服器分派由整合者接（tools/wb_serve.cpp；範例在 WebLotInfo.h）。沒接之前按鈕會顯示「分派未接」，不會假裝做了。
 */
(function () {
  'use strict';

  function $(id) { return document.getElementById(id); }

  // golden TColor（0x00BBGGRR）→ CSS；系統色（0x80000000|n）與 null 回 ''（保留樣式表的顏色）
  function tcolor(c) {
    if (typeof c !== 'number' || c < 0 || c > 0xFFFFFF) return '';
    var r = c & 0xFF, g = (c >> 8) & 0xFF, b = (c >> 16) & 0xFF;
    return 'rgb(' + r + ',' + g + ',' + b + ')';
  }

  // --- 分頁 ------------------------------------------------------------------
  // 27 個頁籤的 dfm 名（＝ lot.tab.<名>，頁面 id tab_<名>）
  var TABS = ['tsDeviceInfo', 'tsLotID', 'tsFTP', 'tsRTCFullViewImg', 'tsATC', 'ts_OCRInterface', 'ts_SocketInterface',
              'tsSelection', 'tsBarCode', 'ts_AutoCleanMonitor', 'ts_AutoRetestMonitor', 'tsOCRBarCode', 'tsESDMonitor',
              'tsASEMARMS', 'tsASECLEventLog', 'tsChamberBoost', 'ATC_WinWay', 'tsRFMD', 'ts_FTPAutomation', 'tsYieldMonitior',
              'tsTesterLog', 'ts_ATC6_1', 'tsBundle', 'tsSetupFileCheck', 'tsAMR', 'tsKYEC_AMR', 'tsOtherTool'];
  // 沒有畫出來的子分頁（客戶專屬，本波跳過）：只收 tag，不綁元素 —— 見 Data.LotInfo.html pgcLotInfo 的註解
  var SUBTABS_SKIPPED = ['tsMurata', 'tsSigurd_CX', 'tsSPIL_SZ', 'tsOEE', 'ts2DSort', 'tsVTest', 'tsPATSetUp', 'tsSigurd', 'tsTPW'];

  function firstVisibleTab() {
    for (var i = 0; i < TABS.length; i++) {
      var t = $('tab_' + TABS[i]);
      if (t && t.style.display !== 'none') return t;
    }
    return null;
  }
  // VCL：選中的頁被 TabVisible=false 時，TPageControl 改選下一個看得到的頁
  function tabVis(v, el) {
    var hide = (v === false);
    el.style.display = hide ? 'none' : '';
    el.title = el.title.replace(/　TabVisible=.*$/, '') + '　TabVisible=' + (v === null || v === undefined ? '不可知' : String(v));
    if (hide && el.classList.contains('act')) {
      var f = firstVisibleTab();
      if (f) f.click();
    }
  }
  var activeApplied = false;
  function activePage(v) {
    if (activeApplied || typeof v !== 'string' || !v) return;
    var t = $('tab_' + v);
    if (!t || t.style.display === 'none') return;
    activeApplied = true;                       // golden FormShow 一次；之後使用者自己切
    t.click();
  }
  function chipAdvVis(v, el) {
    var hide = (v === false);
    el.style.display = hide ? 'none' : '';
    var p = document.querySelector('[data-pane="chipadv"]');
    if (p && hide) p.style.display = 'none';
  }
  function chipAdvCaption(v, el) { if (typeof v === 'string' && v !== '') el.textContent = v; }   // null 保留 dfm "ChipAdv"

  // --- 一般 ------------------------------------------------------------------
  function show(v, el) { el.style.display = (v === false) ? 'none' : ''; }          // false 才藏
  function keep(v, el) { el.style.visibility = (v === false) ? 'hidden' : ''; }     // 表格格子：藏字不藏格
  function txt(v, el) { el.textContent = (v === null || v === undefined) ? '---' : String(v); }

  var tags = {
    // Lot 分頁（上午那一波）
    'lot.id':        ['lotNo',        'text'],      // fLotInfo->edtSysLotID->Text
    'lot.device':    ['deviceName',   'text'],      // fLotInfo->lbledtDeviceName->Text
    'lot.runMode':   ['runMode',      'text'],      // fLotInfo->cbRunMode->Text
    'lot.operator':  ['operator',     'text'],      // fLotInfo->edtSysOperatorID->Text
    'lot.startTime': ['startTime',    'text'],      // fLotInfo->lbledtStarTime->Text
    'sort.loading':  ['loadingCount', 'text', 0],   // LastSet.SendCT[0]（WebBridgeTags.cpp stageInt "sort.loading"）

    // 分頁可見度／開窗頁／子分頁
    'lot.tab.active':            ['pgLotinfo', activePage],
    'lot.tab.tsChipAdv':         ['tab_tsChipAdv', chipAdvVis],
    'lot.tab.tsChipAdv.caption': ['tab_tsChipAdv', chipAdvCaption],

    // Lot Info 子分頁（tsChipAdv）
    'lot.endTime':     ['lbledtEndTime',     'text'],
    'lot.testerOsVer': ['lbledtTesterOsVer', 'text'],
    'lot.customer':    ['lbledtCustomer',    'text'],
    'lot.testProg':    ['lbledtTestProg',    'text'],
    'lot.testerId':    ['lbledtTesterID',    'text'],
    'lot.subLotNo':    ['lbledtSubLotNo',    'text'],
    'lot.testCode':    ['lbledtTestCode',    'text'],
    'lot.machineId':   ['lbledtMachineID',   'text'],
    'lot.testBinNo':   ['lbledtTestBinNo',   'text'],
    'lot.modeCode':    ['lbledtModeCode',    'text'],
    'lot.stage':       ['edtStage',          'text'],
    'lot.step':        ['edtStep',           'text'],

    // ATC 分頁
    'lot.atc.caption':      ['palATC', 'text'],
    'lot.atc.workTemp':     ['palATCWorkingTemp', 'text'],
    'lot.atc.online':       ['pl_ATC_Online', txt],
    'lot.atc.online.color': ['pl_ATC_Online', function (v, el) { el.style.background = tcolor(v); }],
    'lot.atc.power':        ['aldATCPower', 'led', 0, 'ATC Power'],
    'lot.atc.chiller.visible':          ['aldATCChillerStatus', show],
    'lot.atc.chillerLabel.visible':     ['lblChiller', show],
    'lot.atc.atc70.visible':            ['aldATC7Status', show],
    'lot.atc.atc70Label.visible':       ['lblATC70', show],
    'lot.atc.chillerSV.visible':        ['pan_ATCChillerSV', show],
    'lot.atc.chillerSVValue.visible':   ['pl_ATCChillerSV', show],
    'lot.atc.recipeFile.visible':       ['lblATC_Now_RecipeFile', show],
    'lot.atc.dewPoint.visible':         ['grpDewPoint', show],
    'lot.atc.use4.visible':             ['Pan_ATC_Use_4Head', show],
    'lot.atc.use8.visible':             ['Pan_ATC_Use_8Head', show],
    'lot.atc.use32.visible':            ['Pan_ATC_Use_32Head', show],

    // BarCode 分頁
    'lot.barcode.checkByLot':         ['lbCheckCodeByLot', function (v, el) { if (typeof v === 'string' && v !== '') el.textContent = v; }],   // null 保留 dfm 字樣
    'lot.barcode.checkByLot.color':   ['lbCheckCodeByLot', function (v, el) { el.style.background = tcolor(v); }],
    'lot.barcode.changeFile.visible': ['btChangeFile', show],
    'lot.barcode.display.visible':    ['grpBarcodeDisplayLotInfo', show],

    // Tester Log 分頁
    'lot.testerLog.status':       ['labTCPIPStatus', txt],
    'lot.testerLog.status.color': ['labTCPIPStatus', function (v, el) { el.style.background = tcolor(v); }],
    'lot.testerLog.count':        ['mmTesterLogCount', 'text', 0],
    'lot.testerLog.tail':         ['mmTesterLog', function (v, el) { if (!FULL) txt(v, el); else if (v !== null && v !== undefined) fetchFullLog(); }]
  };
  TABS.forEach(function (n) { tags['lot.tab.' + n] = ['tab_' + n, tabVis]; });
  for (var i = 1; i <= 32; i++) {
    var nn = (i < 10 ? '0' : '') + i;
    tags['lot.atc.ch' + i + '.visible']     = ['row_ATC' + nn, show];
    tags['lot.atc.ch' + i + '.ref.visible'] = ['pl_ATCRefHead' + nn, keep];
  }
  for (var r = 1; r <= 6; r++)
    for (var c = 1; c <= 5; c++)
      tags['lot.barcode.r' + r + '.c' + c] = ['sgBarcode_r' + r + '_c' + c, 'text'];

  HT9045Wire.register({
    page: 'Data.LotInfo.html',
    slug: 'datalotinfo',
    tags: tags
  });

  // --- lotinfo.op ----------------------------------------------------------------
  var busy = false, last = null, FULL = false;

  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  // WebBridgeServer.cpp:1380 起：除了 control.* 與少數豁免，每個指令都要先持有操作權杖 —— lotinfo.op 也一樣（連 get 都是：
  // selection.get 在缺鍵時會回寫 Security_new.def）。已持有或他人持有時 acquire 回錯不要緊，後面的指令自己會回 not-operator。
  function op(payload) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.resolve({ executed: false, guard: 'no-client', detail: 'ht9045_recipe_client.js 沒有載入' });
    return HT9045Recipe.rawCmd('control.acquire').catch(function () {})
      .then(function () { return HT9045Recipe.rawCmd('lotinfo.op', { value: JSON.stringify(payload) }); })
      .then(unwrap, parseErr);
  }
  function notWired(r) { return r && (r.guard === 'unknown-action' || /unknown cmd|unknown command/i.test(r.detail || '')); }
  function say(msg, bad) {
    var s = $('barcodeStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
  }

  // BarCode Clear Count（golden btClearBarcodeCountClick :10168-10184）。opts.answer：測試用，直接回答確認框。
  function clearBarcode(opts) {
    if (busy) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say('Clear Count…');
    var asked = false;
    return HT9045Recipe.rawCmd('control.acquire').catch(function () { /* 已持有或他人持有：後續指令自己會回錯 */ })
      .then(function () { return op({ op: 'barcode.clearCount', confirmed: false }); })
      .then(function (r) {
        if (r && r.needConfirm) {
          asked = true;
          var text = (r.prompt || ['Clear barcode count?']).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          if (!yes) return { executed: false, guard: 'confirm-no', detail: '使用者取消', cancelled: true };
          return op({ op: 'barcode.clearCount', confirmed: true });
        }
        return r;
      })
      .then(function (r) {
        last = r; if (r) r.asked = asked;
        if (r && r.executed) say('Barcode 計數已清除（golden btClearBarcodeCountClick）；.xls 沒有存（SGDToXLS 未移植）');
        else if (notWired(r)) say('lotinfo.op 的伺服器分派還沒接（WebLotInfo.cpp W906_LotInfoOp），沒有清除任何東西', true);
        else if (r && r.cancelled) say('已取消');
        else say('沒有清除：' + ((r && (r.guard || r.detail)) || '?'), true);
        return r;
      })
      .then(function (r) { busy = false; return r; }, function (e) { busy = false; var x = parseErr(e); say('失敗：' + x.detail, true); return x; });
  }

  // Tester Log「全部」（唯讀）
  function fetchFullLog() {
    return op({ op: 'testerLog.get' }).then(function (r) {
      var el = $('mmTesterLog');
      if (r && r.executed && el) { FULL = true; el.textContent = r.text || ''; el.scrollTop = el.scrollHeight; }
      else if (el && notWired(r)) { el.title = 'lotinfo.op 分派未接 —— 只顯示最後 50 行（tag lot.testerLog.tail）'; }
      last = r;
      return r;
    });
  }

  // Selection 分頁：切過去＝golden pgLotinfoChange（:7340-7385）讀檔；Save＝golden btnSaveClick（:7387-7415）。
  // 可見度／反灰／勾選值全部照 C++ 回的元件狀態畫；網頁不判斷權限（A75 等由 C++ 的 golden 狀態決定）。
  var SEL_BOXES = ['chkTempOffset', 'chkContactHigh', 'chkContactForce', 'chkContactMode', 'chkHotPlate', 'chkLoadUnload',
                   'chkSpeedSetting', 'chkShuttleMode', 'chkTestMode', 'chkBinasgn', 'chkBinasgnOff', 'checkbAutoClean',
                   'chkIndexHeatingMode', 'chkART', 'chkART_RTCount', 'cbBottom2DOffset', 'chkCleanCount', 'chkAutoCleanContactHeight',
                   'chkStopYield', 'chkConsecutiveFailure'];
  var selBusy = false, lastSel = null;
  function selSay(msg, bad) {
    var s = $('selectionStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
  }
  function selPaint(r) {
    if (!r || !r.boxes) return;
    var g = $('groupbDownloadItem'); if (g) g.style.display = r.groupVisible === false ? 'none' : '';
    var m = $('grpMesCheck'); if (m) m.style.display = r.mesCheckVisible ? '' : 'none';
    var a = $('lblDownloadAccessWarning'); if (a) a.style.display = r.accessWarningVisible ? '' : 'none';
    var s = $('btnSave'); if (s) s.disabled = !r.saveEnabled;
    SEL_BOXES.forEach(function (n) {
      var b = r.boxes[n], el = $(n), lab = $('lab_' + n);
      if (!b || !el) return;
      el.checked = !!b.checked;
      el.disabled = !b.enabled;
      if (lab) lab.style.display = b.visible ? '' : 'none';
    });
  }
  function selectionGet() {
    if (selBusy) return Promise.resolve({ executed: false, guard: 'busy' });
    selBusy = true;
    selSay('讀取 Security_new.def…');
    return op({ op: 'selection.get' }).then(function (r) {
      selBusy = false; lastSel = r; last = r;
      if (r && r.executed) { selPaint(r); selSay('已讀取（golden pgLotinfoChange）'); }
      else if (notWired(r)) selSay('lotinfo.op 的伺服器分派還沒接，勾選值沒有讀到', true);
      else selSay('沒有讀取：' + ((r && (r.guard || r.detail)) || '?'), true);
      return r;
    }, function (e) { selBusy = false; var x = parseErr(e); selSay('失敗：' + x.detail, true); return x; });
  }
  // values 沒給：送畫面上所有勾選框的值（C++ 只套用看得見且沒反灰的那些）
  function selectionSave(values) {
    if (selBusy) return Promise.resolve({ executed: false, guard: 'busy' });
    if (!values) {
      values = {};
      SEL_BOXES.forEach(function (n) { var el = $(n); if (el) values[n] = !!el.checked; });
    }
    selBusy = true;
    selSay('Save…');
    return HT9045Recipe.rawCmd('control.acquire').catch(function () {})
      .then(function () { return op({ op: 'selection.save', values: values }); })
      .then(function (r) {
        selBusy = false; lastSel = r; last = r;
        if (r && r.executed) { selPaint(r); selSay('已存檔（golden btnSaveClick → ' + (r.file || 'Security_new.def') + '）' + (r.ignored ? '；忽略：' + r.ignored : '')); }
        else if (notWired(r)) selSay('lotinfo.op 的伺服器分派還沒接，沒有存檔', true);
        else selSay('沒有存檔：' + ((r && (r.guard || r.detail)) || '?'), true);
        return r;
      }, function (e) { selBusy = false; var x = parseErr(e); selSay('失敗：' + x.detail, true); return x; });
  }

  function hook() {
    var ts = $('tab_tsSelection');
    if (ts) ts.addEventListener('click', function () { selectionGet(); });
    var sv = $('btnSave');
    if (sv) sv.addEventListener('click', function () { selectionSave(); });
    var b = $('btClearBarcodeCount');
    if (b) b.addEventListener('click', function () { clearBarcode(); });
    var a = $('btTesterLogAll');
    if (a) a.addEventListener('click', function () { if (FULL) { FULL = false; var t = window.HT9045Tags && HT9045Tags.get('lot.testerLog.tail'); txt(t === undefined ? null : t, $('mmTesterLog')); } else fetchFullLog(); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045LotInfo = { clearBarcode: clearBarcode, testerLogAll: fetchFullLog,
                           selectionGet: selectionGet, selectionSave: selectionSave, selection: function () { return lastSel; },
                           last: function () { return last; }, tabs: TABS.slice(), skippedSubTabs: SUBTABS_SKIPPED.slice() };
})();
