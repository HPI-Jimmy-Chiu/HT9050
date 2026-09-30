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
 *                               Clear List = lotinfo.op barcode.clearList（兩段式；golden btClearBarcodeListClick :10186-10195：
 *                               清 2D 重複碼清單、LotData.txt 寫成空檔、再 Clear Count；:10191 CCD log 那一行 C++ 閘住）
 *                               AI(W906-FRW-S94) 20260926。按鈕可見度 golden 只在 CC_KYEC_XILINX 改（C++ 回 guard button-hidden）。
 *                               OCRBarCode 分頁的 Clean List（spOCRCleanList）頁面沒畫（分頁跳過）；C++ 有 ocr.clearList，這裡只匯出 ocrClearList()。
 *   lot.testerLog.*             labTCPIPStatus（字＋底色）、mmTesterLog 行數與最後 50 行；「全部」= lotinfo.op testerLog.get
 *   Selection 分頁              切過去 = lotinfo.op selection.get（golden pgLotinfoChange 讀 config\Security_new.def）；
 *                               Save = lotinfo.op selection.save（golden btnSaveClick 寫回）—— 看不到的分頁 C++ 回 guard tab-hidden
 *   Lot 分頁 Lot End          AI(W906-PROD-S117) 20260926：按鈕 sbSECSLotEnd ＝ lotinfo.op lotEnd（兩段式；golden sbSECSLotEndClick
 *                               V912 uLotInfo.cpp:1387-1444 → SetLotEnd :2014-2379，C++ 本體 forms/fLotInfo.cpp 檔尾）。
 *                               顯示／反灰／開批狀態＝lotinfo.op lotEnd.state（切到 Lot 分頁時問一次、按完再問一次）；
 *                               golden 本體拒絕（機台運轉中、MES1646 要先 Clean out、OEE 沒開批）時 C++ 回 guard "refused"。
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
  // AI(W906-D020) 20261001（todo D-020，St01）：藏掉「選中的頁」時只換畫面、不送那一頁的指令。
  //   以前是 f.click()：tabs.js 換畫面之外，也會觸發本檔 hook() 掛在頁籤上的 click —— tab_tsSelection → selectionGet
  //   （lotinfo.op selection.get；缺鍵時 C++ 照 golden CheckAndReadIniData 回寫 Security_new.def）、tab_tsLotID → lotEndState
  //   （lotinfo.op lotEnd.state），兩條都先 control.acquire；而且 tag 在視窗關著時也照來（review6 沒有 stage F）。
  //   golden V912 uLotInfo.cpp（D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy）：pgLotinfoChange（:7340-7385；只有
  //   ActivePage==tsSelection 才讀 Security_new.def :7343-7382，其餘只調尺寸 :7384）只掛在 uLotInfo.dfm:98 OnChange；
  //   全樹沒有任何地方直接呼叫它（grep 20261001 00:5x：只有 :7340 定義與 uLotInfo.h:1145 宣告）。
  //   VCL TPageControl 的 OnChange 只在使用者點頁籤（TCN_SELCHANGE）時發；程式設 ActivePage／TabIndex（TCM_SETCURSEL
  //   不送 TCN_SELCHANGE）、或 TabVisible=false 拿掉選中的頁，都不發。golden 自己就是這樣寫的：FormShow 用程式設
  //   ActivePage（:348、:378、:895、:1076-1098），要調尺寸就在 :1270 直接叫 AdjtsYieldMonitiorSize()，不叫 pgLotinfoChange；
  //   執行中改可見度的 Timer2Timer（:7051；TabVisible :7241-7247、ActivePage :7253／:7261）也不叫 —— lot.tab.* 就是這一類。
  //   畫面切換照 tabs.js 的同一套（.tab.act ＋ 同層 .tabPane[data-pane]）。使用者自己點頁籤（真的 click）照舊送指令。
  //   ⚠ 選哪一頁沒改（firstVisibleTab）：VCL 拿掉選中的頁後選的是「同一個位置的下一頁」（VCL 原始碼不在這個 repo，未實測），
  //     兩者不同時畫面停的頁會跟 golden 不一樣 —— 只影響畫面，另案。
  function showTabOnly(tab) {
    var tabs = tab.parentElement;
    if (!tabs) return;
    tabs.querySelectorAll('.tab').forEach(function (t) { t.classList.remove('act'); });
    tab.classList.add('act');
    var scope = tabs.parentElement;
    if (!scope) return;
    scope.querySelectorAll('.tabPane').forEach(function (p) {
      if (p.parentElement !== scope) return;
      p.style.display = (p.dataset.pane === tab.dataset.tab) ? '' : 'none';
    });
  }
  // VCL：選中的頁被 TabVisible=false 時，TPageControl 改選下一個看得到的頁
  function tabVis(v, el) {
    var hide = (v === false);
    el.style.display = hide ? 'none' : '';
    el.title = el.title.replace(/　TabVisible=.*$/, '') + '　TabVisible=' + (v === null || v === undefined ? '不可知' : String(v));
    if (hide && el.classList.contains('act')) {
      var f = firstVisibleTab();
      if (f) showTabOnly(f);                     // AI(W906-D020) 20261001：只換畫面（見 showTabOnly）；以前 f.click() 會送那一頁的指令
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
        else if (window.HT9045Busy && HT9045Busy.is(r)) say(HT9045Busy.NOTE);   // AI(W906-CMDGUARD-UI) 20260926：伺服器 busy: 不是失敗
        else say('沒有清除：' + ((r && (r.guard || r.detail)) || '?'), true);
        return r;
      })
      .then(function (r) { busy = false; return r; }, function (e) { busy = false; var x = parseErr(e); say('失敗：' + x.detail, true); return x; });
  }

  // BarCode Clear List（golden btClearBarcodeListClick :10186-10195）。AI(W906-FRW-S94) 20260926。兩段式，同 Clear Count
  //   （最後一步 golden 會接著清計數，而 .xls 在移植樹不會存 —— C++ 回 needConfirm 時用它的兩行字問）。opts.answer：測試用。
  //   S107 防連點（AI(W906-CMDGUARD-UI) 20260926 的前端第二道）：busy 擋到 ack 回來；ack 之後再冷卻 coolMs()（400 ms），
  //   冷卻中的點擊直接忽略（不送、不顯示）。主防線是伺服器 WebCmdGuard（barcode.clearList 不在它的豁免清單）。
  //   跟 Clear Count 共用 busy（同一個狀態列，不要兩個一起跑）。
  var listCoolUntil = 0;
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function isBusyReply(x) { return !!(window.HT9045Busy && HT9045Busy.is(x)); }   // ht9045_busy_util.js
  function clearList(opts) {
    if (busy || Date.now() < listCoolUntil) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    say('Clear List…');
    var asked = false;
    function done(r) { busy = false; listCoolUntil = Date.now() + coolMs(); return r; }
    return op({ op: 'barcode.clearList', confirmed: false })
      .then(function (r) {
        if (r && r.needConfirm) {
          asked = true;
          var text = (r.prompt || ['Clear barcode list?']).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          if (!yes) return { executed: false, guard: 'confirm-no', detail: '使用者取消', cancelled: true };
          return op({ op: 'barcode.clearList', confirmed: true });
        }
        return r;
      })
      .then(function (r) {
        last = r; if (r) r.asked = asked;
        if (r && r.executed) say('2D 重複碼清單已清除（golden btClearBarcodeListClick；LotData.txt 已清空、Barcode 計數也清了；.xls 沒有存）');
        else if (notWired(r)) say('lotinfo.op 的伺服器分派還沒接（WebLotInfo.cpp W906_LotInfoOp），沒有清除任何東西', true);
        else if (r && r.cancelled) say('已取消');
        else if (isBusyReply(r)) say(HT9045Busy.NOTE);                          // 伺服器 busy: 不是失敗
        else say('沒有清除：' + ((r && (r.guard || r.detail)) || '?') + (r && r.guard && r.detail ? '（' + r.detail + '）' : ''), true);
        return r;
      })
      .then(done, function (e) {
        var x = parseErr(e); done(x);
        if (isBusyReply(x)) say(HT9045Busy.NOTE); else say('失敗：' + x.detail, true);
        return x;
      });
  }
  // OCRBarCode 分頁 Clean List（golden spOCRCleanListClick :14461-14466）。頁面沒有這顆鈕（分頁跳過）—— 給探針／之後畫分頁時用。
  function ocrClearList() {
    if (busy || Date.now() < listCoolUntil) return Promise.resolve({ executed: false, guard: 'busy' });
    busy = true;
    return op({ op: 'ocr.clearList' }).then(function (r) { last = r; busy = false; listCoolUntil = Date.now() + coolMs(); return r; },
      function (e) { var x = parseErr(e); busy = false; listCoolUntil = Date.now() + coolMs(); last = x; return x; });
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
        else if (window.HT9045Busy && HT9045Busy.is(r)) selSay(HT9045Busy.NOTE);   // AI(W906-CMDGUARD-UI) 20260926：伺服器 busy: 不是失敗
        else selSay('沒有存檔：' + ((r && (r.guard || r.detail)) || '?'), true);
        return r;
      }, function (e) { selBusy = false; var x = parseErr(e); selSay('失敗：' + x.detail, true); return x; });
  }

  // --- Lot 分頁 Lot End（AI(W906-PROD-S117) 20260926，Steven 團隊）----------------------------------------------------
  // golden sbSECSLotEndClick（V912 uLotInfo.cpp:1387-1444）→ SetLotEnd（:2014-2379）；C++：WebLotInfo.cpp lotinfo.op lotEnd.state／lotEnd。
  // 網頁不判斷任何條件：可見度（palSecsGem／sbSECSLotEnd）、反灰、開批狀態（RunInfo.bLotStart）、golden 會不會拒絕，全由 C++ 照 golden 算。
  // S107 防連點（前端第二道，比照 clearList）：lotBusy 擋到 ack 回來；ack 之後冷卻 coolMs()（400 ms），冷卻中的點擊直接忽略（不送、不顯示）。
  //   主防線是伺服器 WebCmdGuard（lotinfo.op 的 lotEnd／lotEnd.state 都不在它的豁免清單）。
  var lotBusy = false, lotCoolUntil = 0, lastLot = null;
  function lotSay(msg, bad) {
    var s = $('lotEndStatus');
    if (!s) return;
    s.textContent = msg;
    s.style.color = bad ? '#b00' : '';
  }
  function lotPaint(r) {
    if (!r || typeof r !== 'object') return;
    var pal = $('palSecsGem'), btn = $('sbSECSLotEnd'), st = $('lotEndLotState');
    if (pal && (r.panelVisible === false || r.tabVisible === false)) pal.style.display = 'none';        // false 才藏
    else if (pal && r.panelVisible === true) pal.style.display = '';
    if (btn) {
      if (r.buttonVisible === false) btn.style.display = 'none'; else if (r.buttonVisible === true) btn.style.display = '';
      if (typeof r.buttonEnabled === 'boolean') btn.disabled = !r.buttonEnabled;
      if (typeof r.caption === 'string' && r.caption) btn.textContent = r.caption;           // OEE 機台 golden 改成 "End Lot"（FormShow :722）
    }
    var lot = r.lot || r.after;
    if (st && lot && typeof lot.lotStart === 'boolean')
      st.textContent = (lot.lotStart ? '開批中' : '未開批') + '（Lot ID：' + (lot.lotId || '—') + '）';
  }
  function lotEndState() {
    return op({ op: 'lotEnd.state' }).then(function (r) {
      if (r && r.executed) {
        lastLot = r; lotPaint(r);
        if (r.precheck) lotSay('預覽：現在按下去 golden 會拒絕 —— ' + (r.precheckText || r.precheck));
      } else if (notWired(r)) lotSay('lotinfo.op 的伺服器分派還沒接（WebLotInfo.cpp W906_LotInfoOp）', true);
      else if (!isBusyReply(r)) lotSay('Lot End 狀態讀不到（' + ((r && (r.guard || r.detail)) || '?') + '）—— 按鈕照樣顯示，按下去由 C++ 判斷');   // busy: 安靜略過；讀不到不當成錯誤（例：別人持有操作權杖）
      return r;
    });
  }
  // opts.answer：測試用，直接回答確認框。
  function lotEnd(opts) {
    if (lotBusy || Date.now() < lotCoolUntil) return Promise.resolve({ executed: false, guard: 'busy' });
    lotBusy = true;
    lotSay('Lot End…');
    var asked = false;
    function done(r) { lotBusy = false; lotCoolUntil = Date.now() + coolMs(); return r; }
    return op({ op: 'lotEnd', confirmed: false })
      .then(function (r) {
        if (r && r.needConfirm) {
          asked = true;
          var text = (r.prompt || ['Lot End?']).join('\n');
          var yes = (opts && typeof opts.answer === 'boolean') ? opts.answer : window.confirm(text);
          if (!yes) return { executed: false, guard: 'confirm-no', detail: '使用者取消', cancelled: true };
          return op({ op: 'lotEnd', confirmed: true });
        }
        return r;
      })
      .then(function (r) {
        lastLot = r; last = r; if (r) r.asked = asked;
        if (r && r.executed) lotSay('已結批（golden SetLotEnd）：config.ini 的 Lot 資訊已清空、Lot Info log 已寫一行' +
                                    (r.gated && r.gated.length ? '；有 ' + r.gated.length + ' 項 golden 敘述在移植樹閘住（C++ 回應 gated）' : ''));
        else if (notWired(r)) lotSay('lotinfo.op 的伺服器分派還沒接（WebLotInfo.cpp W906_LotInfoOp），沒有結批', true);
        else if (r && r.cancelled) lotSay('已取消');
        else if (isBusyReply(r)) lotSay(HT9045Busy.NOTE);                        // 伺服器 busy: 不是失敗
        else if (r && r.guard === 'refused') lotSay('golden 拒絕結批：' + (r.detail || r.refused), true);
        else lotSay('沒有結批：' + ((r && (r.guard || r.detail)) || '?') + (r && r.guard && r.detail ? '（' + r.detail + '）' : ''), true);
        lotPaint(r);
        return r;
      })
      .then(done, function (e) {
        var x = parseErr(e); done(x);
        if (isBusyReply(x)) lotSay(HT9045Busy.NOTE); else lotSay('失敗：' + x.detail, true);
        return x;
      })
      .then(function (r) { if (r && r.confirmed) lotEndState(); return r; });     // 按完再問一次可見度／開批狀態
  }

  function hook() {
    var ts = $('tab_tsSelection');
    if (ts) ts.addEventListener('click', function () { selectionGet(); });
    var sv = $('btnSave');
    if (sv) sv.addEventListener('click', function () { selectionSave(); });
    var b = $('btClearBarcodeCount');
    if (b) b.addEventListener('click', function () { clearBarcode(); });
    var cl = $('btClearBarcodeList');                                            // AI(W906-FRW-S94) 20260926
    if (cl) cl.addEventListener('click', function () { clearList(); });
    var le = $('sbSECSLotEnd');                                                  // AI(W906-PROD-S117) 20260926：Lot End
    if (le) le.addEventListener('click', function () { lotEnd(); });
    var tl = $('tab_tsLotID');                                                   // 切到 Lot 分頁（含 lot.tab.active 開窗時自動切）＝問一次 Lot End 狀態
    if (tl) tl.addEventListener('click', function () { lotEndState(); });
    var a = $('btTesterLogAll');
    if (a) a.addEventListener('click', function () { if (FULL) { FULL = false; var t = window.HT9045Tags && HT9045Tags.get('lot.testerLog.tail'); txt(t === undefined ? null : t, $('mmTesterLog')); } else fetchFullLog(); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045LotInfo = { clearBarcode: clearBarcode, testerLogAll: fetchFullLog,
                           clearList: clearList, ocrClearList: ocrClearList,                                   // AI(W906-FRW-S94) 20260926
                           lotEnd: lotEnd, lotEndState: lotEndState, lotEndLast: function () { return lastLot; },         // AI(W906-PROD-S117) 20260926
                           lotEndBusy: function () { return lotBusy || Date.now() < lotCoolUntil; },
                           busy: function () { return busy || Date.now() < listCoolUntil; },                  // 含 Clear List 冷卻（探針等 !busy()）
                           selectionGet: selectionGet, selectionSave: selectionSave, selection: function () { return lastSel; },
                           last: function () { return last; }, tabs: TABS.slice(), skippedSubTabs: SUBTABS_SKIPPED.slice() };
})();
