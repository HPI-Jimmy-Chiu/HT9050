/* ht9045_contact_q41.js -- Setup.Contact.html（golden TfContact，cContact.cpp）Q41 的「開子頁鈕」補件。
 * ---------------------------------------------------------------------------
 * AI(W906-Q41) 20260927 (St02-E)：Q41（Steven 20260927 S158）盤點 §3.11 的 CT-2。St02 的新檔（手寫，不是 gen_wire.py 產物）；
 * 頁面 Setup.Contact.html:134 同一行載入（在 ht9045_contact_slk.js 之後）。引擎 ht9045_wire_engine.js、background.html 不改。
 * golden 行號：912 cContact.cpp（906_0625_Steven 在括號裡）。golden 都是 fXxx->Show()（非 modal）；網頁叫 background.html 開同一個
 * WINDOWS 表視窗（postMessage {open:id}，同 main.html 選單的做法；開窗守衛、權限表照 background.html 自己的規則）。
 *
 *   btContactForce  :15237（:15012）fContactForce->Show()          → 'contactforce'（Setup.ContactForce.html）
 *   btOffset        :17167（:16876）fOffSet->Show()                → 'offset'
 *   btBarcode       :17172（:16881）fBarCode->Show()               → 'barcode'
 *   btnTempSet      :17475（:17184）fTemp_Set->Show()              → 'tempset'
 *   btTempOffset    :15254（:15029）fTemp_Set->Show()+BringToFront → 'tempset'
 *   btAt            :15248（:15023）fOmron->Show()+BringToFront    → 'omron'（HW.OmronEJ1N.html）
 *   btTCPIP         :19315（:19024）fTesterTCP->Show()             → 'testercomm' 的 TCP/IP 分頁（同 Setup.TesterIF 的 TI-3：
 *                   localStorage 'ht9045.testercomm.tab'＝'tcpip'，testercomm.html 讀）
 * 不做（寫在 Q41 回報）：
 *   btShowDynaTemp  :15242（:15017）fDynamicTemp —— 沒有網頁
 *   btnTrayMap      :18693（:18402）RecordProcess("Enter Tray Map Form")＋fTrayMapping —— 沒有網頁（RecordProcess 在伺服器端）
 *   pnlSensorAdj    :14171（:14064）golden 先 fSecurity->Insufficient(109)（等級 109）才開 fCCLink —— CT-L1 由 St01 ht9045_contact_ev.js
 *                   （review6 6ba451d5 :267-276）送 form.event、ack.todo 有 "open:cclink" 才開窗；本檔不接（頁面不能自己放行）。AI(W906-Q41) 20260928 (St02-E)
 * 按鈕停用／看不見（C 路開頁套的 Enabled／Visible）時不開。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var OPEN = [                                                          // [按鈕 id, background 視窗 id, testercomm 分頁]
    ['btContactForce', 'contactforce'],
    ['btOffset', 'offset'],
    ['btBarcode', 'barcode'],
    ['btnTempSet', 'tempset'],
    ['btTempOffset', 'tempset'],
    ['btAt', 'omron'],
    ['btTCPIP', 'testercomm', 'tcpip']
  ];
  var TC_KEY = 'ht9045.testercomm.tab';

  function $(id) { return document.getElementById(id); }
  function usable(b) {
    if (!b || b.disabled || b.getAttribute('aria-disabled') === 'true') return false;
    for (var p = b; p && p.nodeType === 1; p = p.parentElement) {
      if (p.style && (p.style.display === 'none' || p.style.visibility === 'hidden')) return false;
      if (p !== b && p.getAttribute && p.getAttribute('aria-disabled') === 'true') return false;
    }
    return true;
  }
  function open(win, tab) {
    if (tab) { try { localStorage.setItem(TC_KEY, tab); } catch (e) { /* 私密視窗等：開視窗但停在預設分頁 */ } }
    if (window.parent && window.parent !== window) window.parent.postMessage({ open: win }, '*');
    else console.info('[Contact/Q41] 單獨開頁，沒有 background.html 可以開視窗：' + win);
  }
  function hook() {
    OPEN.forEach(function (q) {
      var b = $(q[0]);
      if (!b || b.__q41) return;
      b.__q41 = true;
      b.style.cursor = 'pointer';
      b.addEventListener('click', function () { if (usable(b)) open(q[1], q[2]); });
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045ContactQ41 = { open: open, table: OPEN };                // 探針／除錯用
})();
