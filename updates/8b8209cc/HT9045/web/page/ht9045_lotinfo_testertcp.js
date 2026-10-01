/* ht9045_lotinfo_testertcp.js -- Lot Info「Tester Log」分頁的 Tester TCP Show 鈕（golden TfLotInfo::btTesterTCPShowClick）。
 * ---------------------------------------------------------------------------
 * AI(W906-LI12) 20261002 (St02-E) 新檔（手寫）。E-019 LI-12（HT9011UC_Cpp_V3.33.906.0/docs/E019_DATA_STATUS_EVENTS_20261001.md）。
 *
 * golden（906_0625_Steven uLotInfo.cpp:13841-13844；V912 :14116）：
 *     void __fastcall TfLotInfo::btTesterTCPShowClick(TObject *Sender) { fTesterTCP->Show(); }
 *   非 modal 開 TfTesterTCP。這顆鈕在 tsTesterLog 分頁上，而 tsTesterLog 只在 TestIF_File.iTestType==TCP_IP_MODE 時看得到
 *   （uLotInfo.cpp:7124；網頁的頁籤由 tag lot.tab.tsTesterLog 套，ht9045_lotinfo_wire.js）。
 *
 * 網頁：TfTesterTCP＝testercomm.html 的 TCP/IP 分頁（background.html WINDOWS 表 id 'testercomm'）。做法同 Q41 TI-3
 *   （ht9045_testerif_c_wire.js (6)、ht9045_contact_q41.js）：localStorage 'ht9045.testercomm.tab'＝'tcpip'，再
 *   postMessage {open:'testercomm'}（background.html 的開窗守衛照舊）；單獨開這一頁時開新分頁 testercomm.html#tcpip。
 *   testercomm.html 已經開著時靠 storage 事件換分頁。
 *   出貨版 testercomm 只留使用中的介面（testercomm.html applyBuildTabs）：不是 TCP/IP 時沒有 TCP/IP 分頁，會停在 Test Result
 *   ——golden 這時候看不到這顆鈕（tsTesterLog 藏起來），網頁只有在頁籤可見度還不知道時才按得到。
 * 沒有 C++、不送 WS 指令、不寫檔。
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  if (window.HT9045LotInfoTesterTcp) return;

  var TC_KEY = 'ht9045.testercomm.tab';             // testercomm.html 讀（同 origin 的 localStorage）

  function show(ev) {                                // golden fTesterTCP->Show()
    var b = ev && ev.currentTarget;
    if (b && (b.disabled || b.getAttribute('aria-disabled') === 'true')) return;
    try { localStorage.setItem(TC_KEY, 'tcpip'); } catch (e) { /* 私密視窗等：開視窗但停在預設分頁 */ }
    if (window.parent && window.parent !== window) window.parent.postMessage({ open: 'testercomm' }, '*');   // background.html id 'testercomm'
    else window.open('testercomm.html#tcpip', '_blank');                                                       // 單獨開這一頁時
  }
  function hook() {
    var b = document.getElementById('btTesterTCPShow');
    if (!b || b.__w906Li12) return;
    b.__w906Li12 = true;
    b.addEventListener('click', show);
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();

  window.HT9045LotInfoTesterTcp = { show: function () { show(null); } };   // 探針／除錯用
})();
