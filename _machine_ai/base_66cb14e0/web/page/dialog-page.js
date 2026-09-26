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
    if (d.flushPanel && $(d.flushPanel)) $(d.flushPanel).classList.add('dbFlush');   // ShowErrorUnit(Pos)
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
  }

  // ---- MyMessageBox（ShowMyMessage）：OK/Pause 關閉；AlarmReset 只消音不關閉（pnlAlarmResetClick） ----
  function renderMessage(req) {
    var a = req.arguments || {}, d = req.display || {}, r = req.runtime || {};
    var secs = !!(r.secsGemAlarm || r.haltHandler);
    var label = d.buttonLabel || ((a.ok || secs) ? 'OK' : 'Pause');
    show($('moSecsGem'), secs);
    setText($('moSecsGem'), secs ? (a.s1 || '') : '');
    setText($('lblMainMsg'), secs ? '' : (d.primaryText || a.s1 || ''));
    setText($('lblChineseMsg'), d.secondaryText || a.s2 || '');
    setText($('pnlPause'), label);
    show($('pnlPause'), d.buttonEnabled !== false && !r.employeeIdCheck);   // bEnableEmployeeIDCheck 等 EAP 時隱藏
    show($('pnlAlarmReset'), !!d.showAlarmReset);
    show($('pnlYes'), false); show($('pnlNo'), false); show($('lblSubMsg'), false);
  }

  function bindMessage() {
    ['pnlPause', 'pnlAlarmReset'].forEach(function (id) { var el = $(id); if (el) el.style.cursor = 'pointer'; });
    if ($('pnlPause')) $('pnlPause').addEventListener('click', function () {
      if (!current) return;
      var cap = $('pnlPause').querySelector('.pnlCap');
      post({ type: 'HT_DIALOG_ACTION', kind: 'message', requestId: current.requestId,
             action: { name: (cap ? cap.textContent : 'PAUSE').trim().toUpperCase() }, pressedButton: 'pnlPause' });
    });
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
