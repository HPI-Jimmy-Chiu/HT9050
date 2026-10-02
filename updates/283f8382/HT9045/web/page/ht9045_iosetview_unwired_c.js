/* ht9045_iosetview_unwired_c.js -- HW.IoSetView: the buttons / track bars golden has a handler for that do nothing on the web.
 *
 * AI(W906-IOSV-UNWIRED) 20261002: EastSun 20261001「不要像 mot teach 等頁面元件都沒功能」—— the every-component check (compK ioS)
 *   found these with a golden OnClick / OnChange and no web handler: pressing them did nothing and said nothing. Each is greyed
 *   with golden's handler and the reason it cannot run here (title + data-unwired, the same marker the Teach page uses).
 *   Nothing is sent. When one gets wired, delete its row here.
 *   (The IO panel buttons, the IO Table grid, Load / Add / Delete / Modify / Save and the filters are wired elsewhere:
 *    ht9045_io_do.js, the engine's sysGrid. The panels that golden hides on this machine are hidden by
 *    ht9045_iosetview_formshow_c.js.)
 */
(function () {
  'use strict';
  var NO_ADAM = '這個移植樹的 ADAM-6024 EP 介面是空的替身（atester_shims.h 的 ADAM_DirectWriteData；ADAM_ReadPA 沒有移植）—— 沒有送到 EP 的路徑';
  var UNWIRED = {
    btn500KPA:     'golden btn500KPAClick（iosetview.cpp:4045）：ADAM_DirectWriteData(EP_MAXKPA<=500?4095:2275, 0) 把 Index EP 打到 0.5MPa。' + NO_ADAM,
    tbarIndexEP:   'golden tbarIndexEPChange（iosetview.cpp:1800）：ADAM_DirectWriteData(Position, 0) 調 Index EP。' + NO_ADAM,
    tbarIndexEP2:  'golden tbarIndexEP2Change（iosetview.cpp:3965）：ADAM_DirectWriteData(Position, 0, 11) 調 Index 2 EP。' + NO_ADAM,
    tbarDieForce:  'golden tbarDieForceChange（iosetview.cpp:1826）：ADAM_DirectWriteData(Position, 0, 0)。' + NO_ADAM,
    tbarLoaderEP:  'golden tbarLoaderEPChange（iosetview.cpp:1808）：ADAM_DirectWriteData(Position, 1) 調 Loader CKD 吹氣。' + NO_ADAM,
    btnAllLock:    'golden btnAllLockClick（iosetview.cpp:4050）：8 個 SwFixedSeat* 開關跟目標不同的就按一下（BtnPanelClick）。' +
                   '這台的 IO 表（D:\\HT9045\\system\\IO_Table.csv）8 個 SwFixedSeat* 全是 Enable=0、不在 1203 上 —— 沒有東西可以切',
    btnAllUnLock:  'golden btnAllUnLockClick（iosetview.cpp:4072）：同上，往 Unlock 方向。這台 8 個 SwFixedSeat* 全是 Enable=0、不在 1203 上',
    btnTTLTest:    'golden btnTTLTestClick（iosetview.cpp:1268）＋Timer2 DoTTL_Spin（:1294）：照 Send Count／Pulse Delay 對 SW[SwStart0]／SW[SwStart1] 打脈衝。' +
                   '這台的 IO 表沒有 SwStart0／SwStart1 —— 沒有東西可以打',
    btnTool:       'golden btnToolClick（iosetview.cpp:1025）：FTool->ShowModal()（tools.cpp：逐點選 Lane／IP／Port 切輸出、Timer 先查安全門）。網頁沒有這個表單',
    btnReload:     'golden btnReloadClick（iosetview.cpp:1252）：HSys.LoadIoData＋LoadIoTable＋InitialSwitch／InitialSensor／InitSucker／InitCylinder／ChangeSite' +
                   '（把 IO 表重新載進引擎，不必重開）。C++ 只在開機載入 IO_Table.csv，要不要做「不重開就生效」等 EastSun 決定；IO Table 頁的 Load 只重讀表格',
    TrackBarLight: 'golden TrackBarLightChange（iosetview.cpp:4094）→ SetLightZI0：SW[SwPRGSEL0..6] 送燈高（只在 Top&Bottom AOI，這台照 golden 隱藏）；沒有接'
  };
  function mark() {
    Object.keys(UNWIRED).forEach(function (id) {
      var el = document.getElementById(id); if (!el) return;
      if ('disabled' in el) el.disabled = true;
      el.setAttribute('aria-disabled', 'true');
      el.setAttribute('data-unwired', UNWIRED[id]);
      el.style.cursor = 'not-allowed';
      var a = el.hasAttribute('title') ? 'title' : (el.hasAttribute('data-htitle') ? 'data-htitle' : 'title');
      var t = el.getAttribute(a) || '';
      if (t.indexOf('（網頁停用）') < 0) el.setAttribute(a, t + '\n（網頁停用）' + UNWIRED[id]);
    });
  }
  window.HT9045IoSetViewUnwired = { list: UNWIRED, mark: mark };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', mark); else mark();
})();
