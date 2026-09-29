/* ht9045_lduld_q41.js -- Setup.Ld_ULd.html（golden TfLd_ULd，cLd_ULd.cpp）Q41 的頁面事件補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.1 的 LU-1。St02 的新檔（手寫，不是 gen_wire.py 產物）；
 * 頁面 Setup.Ld_ULd.html:121 同一行載入，在接線資料檔之後。引擎 ht9045_wire_engine.js 不改。
 *
 *  LU-1  btnDefaultValueClick（golden 906_0625_Steven cLd_ULd.cpp:224-237；912 :225-238；DFM 912 :246）：8 個上下料延遲欄填回
 *        出廠值。golden 是 TEdit->Text = 0.2 這類數字指定（AnsiString 轉成 "0.2"、"2"…），這裡填同樣的字串，再補發 input／change
 *        （同引擎小鍵盤提交的做法）。按鈕能不能按由 C 路開頁帶到（等級 115，FileRW/Ld_UldDelayTime.gen.inc:220），停用時不動。
 *        golden 不檢查各欄是不是可改，照填；伺服器存檔時照規則丟掉不可改欄位的值（ack 會列出）。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var DEF = [                                                            // golden :228-237（順序照 golden）
    ['edtLD_TrayArrivalDely',   '0.2'],                                  // tray Arrival Lock (Sec)
    ['edtLD_FixTrayDely',       '0.5'],                                  // Lock-Preparation is completed(Sec)
    ['edtLD_MiddLockDelay',     '2'],                                    // Lifter down to the middle-separator closed(Sec)
    ['edtLD_LiftDownDelay',     '1'],                                    // Lifter down-Tray exist/non-exit check(Sec)
    ['edtULD_TrayArrivalDelay', '0.2'],
    ['edtULD_FixTrayDely',      '0.5'],
    ['edtULD_TrayBackDelay',    '1'],
    ['edtULD_LiftDownDelay',    '0.2']
  ];

  function $(id) { return document.getElementById(id); }
  function fire(el) {
    ['input', 'change'].forEach(function (evn) {
      var e2; try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
      el.dispatchEvent(e2);
    });
  }
  function btnDefaultValueClick() {
    var b = $('btnDefaultValue');
    if (!b || b.disabled || b.getAttribute('aria-disabled') === 'true') return;
    DEF.forEach(function (p) {
      var el = $(p[0]);
      if (el && el.value !== p[1]) { el.value = p[1]; fire(el); }
    });
  }
  function hook() {
    var b = $('btnDefaultValue');
    if (b && !b.__q41) { b.__q41 = true; b.style.cursor = 'pointer'; b.addEventListener('click', btnDefaultValueClick); }
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045LdUldQ41 = { btnDefaultValueClick: btnDefaultValueClick };   // 探針／除錯用
})();
