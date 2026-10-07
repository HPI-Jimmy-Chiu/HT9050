/* dialog-page.js —— Alert.Note.html（fNote）／Alert.MyMessageBox.html（MyMessageBox）的 request 綁定層。
   由 background 的 dialog-bridge.js 以 iframe overlay 開啟，流程：
     頁面 ready → postMessage {type:'HT_DIALOG_READY'} → 收 {type:'HT_DIALOG_REQUEST',kind,request}
     → 依 BCB6 FormShow()/ShowErrorMessage()/ShowMyMessage() 填值與顯示按鍵
     → 操作員按鍵 → postMessage {type:'HT_DIALOG_ACTION',...}（關閉由 bridge 執行並寫 response）
   HTML 不讀 IO；實體鍵關閉由 C++ 寫 Dialog-close-request.json，bridge 直接關 overlay。 */
(function (global) {
  'use strict';
  var kind = document.body.getAttribute('data-dialog');   // 'alarm' | 'message'
  var current = null;
  var isTop = global.parent === global;

  function $(id) { return document.getElementById(id); }
  function show(el, on) { if (el) el.style.display = on ? '' : 'none'; }
  function setText(el, text) {
    if (!el) return;
    if (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA') el.value = text || '';
    else if (el.classList.contains('pnl')) { var cap = el.querySelector(':scope > .pnlCap'); if (cap) cap.textContent = text || ''; }
    else el.textContent = text || '';
  }
  function post(msg) { if (!isTop) global.parent.postMessage(msg, '*'); }

  // ---- fNote（ShowErrorMessage）：KeyComp[] ↔ BtnPanel（note.cpp FormShow/UpdateButtonStatus/Start/BtnPauseClick） ----
  var ALARM_BTNS = [
    ['BtnSkip', 0x0002, 'SKIP'], ['BtnRetry', 0x0001, 'RETRY'], ['BtnTrayFeed', 0x0008, 'TRAY_FEED'],
    ['BtnTrayEnd', 0x0010, 'TRAY_END'], ['BtnCleanOut', 0x0004, 'CLEAN_OUT'], ['BtnReset', 0x0020, 'RESET'],
    ['BtnHome', 0x0040, 'HOME'], ['BtnTrain', 0x0080, 'TRAIN'], ['BtnOneCycle', 0x0200, 'ONECYCLE']
  ];
  var selected = null;   // 目前 Down 的選擇鍵（Select[i] 互斥）

  /* //Steven 20260923：kCode 守門。契約 Dialog-bridge-contract.json 的 kCodeGuard。
   * ALARM_BTNS 這 9 個位元就是 golden note.cpp:1245 KeyComp[] 的全部。
   * 12 個 K_* 常數裡另外三個（FIX 0x0100 / PAUSE 0x0400 / START 0x0800）
   * 在 V912 與 V899 的 ShowErrorMessage() 呼叫端**各 0 筆**（20260923 逐一剖析
   * 2000 / 1763 個呼叫點量出來的），所以 golden 本身送不出這種 kCode。
   *
   * ⚠ 但若上游（C++ 合成 request）送來 kCode!=0 卻不含任何可選位元：
   *   note.cpp:1537-1549 一顆動作鍵都不畫，BtnStart 卻因 kCode!=0 而顯示；
   *   TfNote::Start() 走完 9 格 Select[] 迴圈沒中，又跳過 note.cpp:3716 的
   *   KeyCode==0 分支 -> Start 與 Pause 都變 no-op
   *   => 框關不掉、機台停在那裡、log 什麼都沒有。
   * 所以這裡降級成 kCode==0 的確認框，讓操作員一定關得掉，並大聲留話。 */
  var SELECTABLE_MASK = 0x02FF;   // 9 個可選位元的聯集（0x03FF 去掉 K_FIX 0x0100）

  function guardKCode(kCode, code) {
    if (kCode === 0 || (kCode & SELECTABLE_MASK) !== 0) return kCode;
    console.error('[dialog-page] kCode=0x' + kCode.toString(16) +
      ' 不含任何可選鍵（code=' + code + '）。這是上游缺陷：golden 送不出這種值。' +
      ' 依契約 kCodeGuard.consumerRule 降級為確認框。');
    return 0;
  }

  function renderAlarm(req) {
    var a = req.arguments || {}, d = req.display || {};
    var code = String(a.code || '');
    var kCode = guardKCode(Number(a.kCode) || 0, code);
    var msg = d.message || '';
    if (a.errorPart && a.errorPart !== ' ') msg += ' : ' + a.errorPart;
    if (a.duplicateError) msg += ' (Again!!)';
    setText($('edErrorCode'), code);
    setText($('Edit3'), code.substr(3, 2));            // Code.SubString(4,2)
    setText($('edUnitName'), d.unitName || '');
    setText($('ShowMessageEdit1'), msg);
    setText($('reDescription'), d.description || '');
    setText($('BtnHome'), code === 'WAR07352' ? 'HOME' : 'HOME & RETRY');
    selected = null;
    ALARM_BTNS.forEach(function (b) {
      var el = $(b[0]); if (!el) return;
      el.classList.remove('down');
      show(el, (kCode & b[1]) !== 0);
    });
    show($('BtnStart'), kCode !== 0);
    show($('BtnPause'), true);
    document.querySelectorAll('.dbFlush').forEach(function (el) { el.classList.remove('dbFlush'); });
    errorUnitVisible(Number(a.position));   // AI(W906-MOTB-NOTEUNIT) 20261002: golden Reset + ShowErrorUnit's Visible / Left half (note.cpp:312-331, :4513-4739); the red flash stays in ht9045_alarm_motionview.js
    if (d.flushPanel && $(d.flushPanel)) $(d.flushPanel).classList.add('dbFlush');   // ShowErrorUnit(Pos)
  }
  /* AI(W906-MOTB-NOTEUNIT) 20261002: the Visible / Left half of golden TfNote::Reset (note.cpp:312-331, run by FormClose :2558 before the
   *   next note) and of ShowErrorUnit(Pos) (note.cpp:4511-4739). ShowErrorUnit has no C++ port and the mailbox sends flushPanel null
   *   (tools/wb_dialog_mailbox.h:598); the red flash is drawn by ht9045_alarm_motionview.js from JSON/Alarm-unit-map.json, but the
   *   panels golden first makes visible (palInSh / palOutSh / palTemp / palScan / palCCD / palOCR / the ion fans, all Visible=False in the
   *   dfm) stayed hidden, so their flash could not be seen. Numbers = cmydef.cpp:2561 / :2698-2742.
   *   Golden quirks kept: Reset brings palInSh1 / palInSh2 back but never palOutSh1 / palOutSh2 (:316-317), so after one MMOutShuttle
   *   alarm they stay hidden; nothing hides palOCR again. Not done: the CCD / OCR full-view images (:4645-4733; REAL_TIME_CCD=0 here). */
  function errorUnitVisible(pos) {
    ['palOutSh', 'palInSh', 'palTemp', 'palScan'].forEach(function (id) { show($(id), false); });       // Reset :312-315
    show($('palInSh1'), true); show($('palInSh2'), true);                                                  // :316-317
    var h = $('palHead'); if (h) h.style.left = '229px';                                                   // :318
    for (var f = 1; f <= 12; f++) show($('palIonFan' + (f < 10 ? '0' : '') + f), false);                  // :320-331
    show($('palCCD'), false);                                                                              // ShowErrorUnit :4513
    if (pos === 501) { show($('palInSh'), true); show($('palInSh1'), false); show($('palInSh2'), false); if (h) h.style.left = '301px'; }    // MMInShuttle :4603-4610
    else if (pos === 502) { show($('palOutSh'), true); show($('palOutSh1'), false); show($('palOutSh2'), false); if (h) h.style.left = '145px'; }   // MMOutShuttle :4611-4618
    else if (pos === 504) show($('palTemp'), true);                                                        // MMTemperature :4625-4629
    else if (pos === 506) show($('palScan'), true);                                                        // MMScanner :4634-4638
    else if (pos === 507) show($('palCCD'), true);                                                         // MMCCD :4639-4642
    else if (pos === 186) { show($('palOCR'), true); show($('Label8'), false); }                           // MMOCR :4701-4705
    else if (pos >= 540 && pos <= 551) show($('palIonFan' + (pos < 549 ? '0' : '') + (pos - 539)), true);   // MMIonFan01..12 :4582-4593, :4739
  }

  function alarmAction(pressed) {
    if (!current) return;
    // //Steven 20260923：與 renderAlarm 用同一道守門，否則畫面降級了、答案這條路還是卡著
    var kCode = guardKCode(Number(current.arguments && current.arguments.kCode) || 0,
                           String((current.arguments && current.arguments.code) || ''));
    var action = { name: 'ACKNOWLEDGE', code: 0 };
    if (kCode !== 0) {
      if (!selected) return;                           // cpp：KeyCode!=0 未選擇鍵 → Start/Pause 無動作
      action = { name: selected[2], code: selected[1] };
    }
    post({ type: 'HT_DIALOG_ACTION', kind: 'alarm', requestId: current.requestId, action: action, pressedButton: pressed });
  }

  function bindAlarm() {
    ALARM_BTNS.forEach(function (b) {
      var el = $(b[0]); if (!el) return;
      el.addEventListener('click', function () {        // UpdateButtonStatus：互斥 Down（TrueColor 紅）
        ALARM_BTNS.forEach(function (o) { var x = $(o[0]); if (x && x !== el) x.classList.remove('down'); });
        el.classList.add('down'); selected = b;
      });
    });
    if ($('BtnStart')) $('BtnStart').addEventListener('click', function () { $('BtnStart').classList.remove('down'); alarmAction('BtnStart'); });
    if ($('BtnPause')) $('BtnPause').addEventListener('click', function () { $('BtnPause').classList.remove('down'); alarmAction('BtnPause'); });
    if ($('BtnAlarmReset')) $('BtnAlarmReset').addEventListener('click', function () {
      post({ type: 'HT_DIALOG_EVENT', kind: 'alarm', requestId: current && current.requestId, event: 'ALARM_RESET' });
    });
    /* AI(W906-MOTB-NOTEFLASH) 20261002: golden TfNote::FlushLabel (note.cpp:3084-3139), run by Timer1Timer (:3377) on every edge of
     *   FlushFlag, which flips every 250 ms (main.cpp:3181-3184). A key that is not the chosen one blinks clBlue / 0x00804000
     *   (#004080) (:3103-3112); the chosen one is red (UpdateButtonStatus :2815 -- here the .down class = dfm TrueColor #ff0000);
     *   BtnStart / BtnPause blink blue once a key is chosen, or for a KeyCode==0 note, else stay #004080 (:3116-3132).
     *   Before this the page drew the keys still and BtnStart in its dfm clBlue. */
    var flushOn = false;
    setInterval(function () {
      if (!current) return;
      flushOn = !flushOn;
      var kc = guardKCode(Number(current.arguments && current.arguments.kCode) || 0, String((current.arguments && current.arguments.code) || ''));
      ALARM_BTNS.forEach(function (b) {
        var el = $(b[0]); if (el && selected !== b) el.style.setProperty('--bp-false', flushOn ? '#0000ff' : '#004080');
      });
      var go = flushOn && (selected !== null || kc === 0);
      ['BtnStart', 'BtnPause'].forEach(function (id) { var el = $(id); if (el) el.style.setProperty('--bp-false', go ? '#0000ff' : '#004080'); });
    }, 250);
  }

  // ---- MyMessageBox（ShowMyMessage）：OK/Pause 關閉；AlarmReset 只消音不關閉（pnlAlarmResetClick） ----
  /* AI(W906-YESNO) 20260925：同一頁也畫 golden ShowMyMessageBox_YES_NO（mymessbox.cpp:1009-1062）。
   * 使用者 20260925 裁決（RULINGS_20260925.md 第 10 條）：YES/NO 不再被 C++ 替身自動回答，
   * 照 golden 跳框等操作員按是／否。C++（tools/wb_serve.cpp ForwardShowMyMessageBoxYesNo）
   * 在同一個 show-my-message 信箱寫 request，多帶三格：
   *   display.yesNo=true   -> 畫 pnlYes/pnlNo、藏 pnlPause/pnlAlarmReset（golden :1024-1027）
   *   display.secondaryText／display.subText -> S2 以第一個 ';' 切兩段（golden :1032-1047）
   * 按鍵送 HT_DIALOG_ACTION {action:{name:'YES'|'NO', code:1|2}, pressedButton:'pnlYes'|'pnlNo'}，
   * 之後的路（dialog-bridge.js complete() -> HTDialogHost.submitResponse -> modal.answer）一行沒改；
   * message 通道的 responseFor() 不帶 pressedButton，所以送到 C++ 的是純 "YES"／"NO"
   * （20260925 無頭 Edge 自測實測：modalAnswer(qid, "YES")）。
   * pnlYes/pnlNo 兩個 TPanel 本來就在這一頁（mymessbox.dfm 產生），只是以前一律藏起來。 */
  function rememberTop(el) { if (el && el.getAttribute('data-top0') === null) el.setAttribute('data-top0', el.style.top || ''); }
  function restoreTop(el) { if (el && el.getAttribute('data-top0') !== null) el.style.top = el.getAttribute('data-top0'); }

  function renderMessage(req) {
    var a = req.arguments || {}, d = req.display || {}, r = req.runtime || {};
    var secs = !!(r.secsGemAlarm || r.haltHandler);
    var yn = !!d.yesNo;                                  // AI(W906-YESNO) 20260925
    var label = d.buttonLabel || ((a.ok || secs) ? 'OK' : 'Pause');
    show($('moSecsGem'), secs);
    setText($('moSecsGem'), secs ? (a.s1 || '') : '');
    setText($('lblMainMsg'), secs ? '' : (d.primaryText || a.s1 || ''));
    setText($('lblChineseMsg'), yn ? (d.secondaryText || '') : (d.secondaryText || a.s2 || ''));
    setText($('pnlPause'), label);
    show($('pnlPause'), !yn && d.buttonEnabled !== false && !r.employeeIdCheck);   // bEnableEmployeeIDCheck 等 EAP 時隱藏；YES/NO 時 golden :1025 藏起來
    show($('pnlAlarmReset'), !yn && !!d.showAlarmReset);                           // golden :1024
    show($('pnlYes'), yn); show($('pnlNo'), yn);                                   // golden :1026-1027
    // golden :1034-1047 + FormShow :312-319（bChangeForm）：S2 有 ';' 才顯示 lblSubMsg 並把兩行往上挪
    var main = $('lblMainMsg'), chin = $('lblChineseMsg'), sub = $('lblSubMsg');
    [main, chin, sub].forEach(rememberTop);
    var semi = yn && String(a.s2 || '').indexOf(';') >= 0;
    setText(sub, semi ? (d.subText || '') : '');
    show(sub, semi);
    if (semi) {
      if (main) main.style.top = '8px';                                            // FormShow :316
      if (chin) chin.style.top = '64px';                                           // :1038 / :317
      if (sub && chin) sub.style.top = (64 + (chin.offsetHeight || 0) + 15) + 'px'; // :1040 / :318
    } else {
      [main, chin, sub].forEach(restoreTop);
    }
    if (d.panels) applyPanels(d);   // AI(W906-SMM) 20260925：ShowMyMessage／ShowUnloaderTrayMessage 帶 display.panels；Jimmy 的 YES/NO（display.yesNo）不帶，由上面畫
  }

  /* AI(W906-SMM) 20260925：golden 各函式把同一張 MyMessageBox 的按鈕排成不同樣子
   * （mymessbox.cpp：ShowMyMessage 只留 pnlPause；ShowMyMessageBox_YES_NO :1044-1047 藏 pnlPause、
   * 露 pnlYes/pnlNo；YES_SKIP :1326 pnlNo 改 "Skip"；ShowLotEndMessage :972-992 三顆都露並改位置）。
   * C++ 在 display.panels 逐顆給 {visible, caption, left}（tools/wb_serve.cpp 檔尾 MbPost）；
   * 沒有 panels 的舊 request 照上面原本的規則畫，行為不變。
   * S2 含 ';' 時 golden 拆出 lblSubMsg 並改排版（bChangeForm :1052-1062／FormShow :313-320）。 */
  var PANEL_IDS = ['pnlPause', 'pnlYes', 'pnlNo', 'pnlAlarmReset'];
  var homeTop = null, homeLeft = null;
  function applyPanels(d) {
    var P = d && d.panels;
    if (homeTop === null) {                               // 第一次記下 dfm／FormShow 的原始位置，之後每則先還原
      homeTop = {}; homeLeft = {};
      ['lblMainMsg', 'lblChineseMsg', 'lblSubMsg'].forEach(function (id) { var el = $(id); if (el) homeTop[id] = el.style.top; });
      PANEL_IDS.forEach(function (id) { var el = $(id); if (el) homeLeft[id] = el.style.left; });
    }
    ['lblMainMsg', 'lblChineseMsg', 'lblSubMsg'].forEach(function (id) { var el = $(id); if (el) el.style.top = homeTop[id]; });
    PANEL_IDS.forEach(function (id) { var el = $(id); if (el) el.style.left = homeLeft[id]; });
    if (!P) return;
    PANEL_IDS.forEach(function (id) {
      var el = $(id), p = P[id];
      if (!el || !p) return;
      show(el, !!p.visible);
      if (p.caption !== undefined && p.caption !== null && p.caption !== '') setText(el, p.caption);
      if (typeof p.left === 'number') el.style.left = p.left + 'px';
    });
    if (d.subText) { show($('lblSubMsg'), true); setText($('lblSubMsg'), d.subText); }
    if (d.changeForm) {                                   // golden FormShow :316-319
      if ($('lblMainMsg')) $('lblMainMsg').style.top = '8px';
      if ($('lblChineseMsg')) $('lblChineseMsg').style.top = '64px';
      if ($('lblSubMsg')) $('lblSubMsg').style.top = (64 + 62 + 15) + 'px';   // lblChineseMsg->Top+Height+15（dfm Height=62）
    }
  }

  function bindMessage() {
    ['pnlPause', 'pnlAlarmReset'].forEach(function (id) { var el = $(id); if (el) el.style.cursor = 'pointer'; });
    // AI(W906-YESNO) 20260925：golden pnlYesClick（mymessbox.cpp:1132-1142，pnlYes/pnlNo 共用 OnClick，iValue = Tag）
    //   Tag：pnlYes=1、pnlNo=2（mymessbox.dfm:160／:141）。只在 display.yesNo 的 request 上有效 ——
    //   一般 ShowMyMessage 時這兩顆是藏起來的，就算被點到也不送（C++ 那邊不認得 YES/NO）。
    [['pnlYes', 'YES', 1], ['pnlNo', 'NO', 2]].forEach(function (b) {
      var el = $(b[0]); if (!el) return;
      el.style.cursor = 'pointer';
      el.addEventListener('click', function () {
        if (!current || !(current.display && current.display.yesNo)) return;
        post({ type: 'HT_DIALOG_ACTION', kind: 'message', requestId: current.requestId,
               action: { name: b[1], code: b[2] }, pressedButton: b[0] });
      });
    });
    if ($('pnlPause')) $('pnlPause').addEventListener('click', function () {
      if (!current) return;
      var cap = $('pnlPause').querySelector('.pnlCap');
      /* AI(W906-SMM) 20260925：pnlPause 不管字樣是 Pause／PAUSE／OK／Skip／不供給，golden 都是同一支
       * pnlPauseClick（mymessbox.cpp:448，iValue=0 關框）⇒ 動作名只分 OK 與 PAUSE，C++ 才認得。 */
      var up = (cap ? cap.textContent : 'PAUSE').trim().toUpperCase();
      post({ type: 'HT_DIALOG_ACTION', kind: 'message', requestId: current.requestId,
             action: { name: up === 'OK' ? 'OK' : 'PAUSE' }, pressedButton: 'pnlPause' });
    });
    /* AI(W906-SMM) 20260925：pnlYes/pnlNo 的點擊改由上面 Jimmy 的 AI(W906-YESNO) 那一組處理（合併 main，避免一次點擊送兩次）。 */
    if ($('pnlAlarmReset')) $('pnlAlarmReset').addEventListener('click', function () {
      post({ type: 'HT_DIALOG_EVENT', kind: 'message', requestId: current && current.requestId, event: 'ALARM_RESET' });
    });
  }

  var style = document.createElement('style');
  style.textContent = '@keyframes dbFlush{0%,49%{background:#ff0000 !important}50%,100%{background:#517b91 !important}}' +
    '.dbFlush{animation:dbFlush 1s step-end infinite;}';
  document.head.appendChild(style);

  global.addEventListener('message', function (e) {
    var m = e.data; if (!m || m.type !== 'HT_DIALOG_REQUEST' || m.kind !== kind) return;
    current = m.request || null;
    if (!current) return;
    if (kind === 'alarm') renderAlarm(current); else renderMessage(current);
  });

  if (kind === 'alarm') bindAlarm(); else bindMessage();
  global.HTDialogPage = { kind: kind, current: function () { return current; }, render: kind === 'alarm' ? renderAlarm : renderMessage };
  post({ type: 'HT_DIALOG_READY', kind: kind });
})(window);
