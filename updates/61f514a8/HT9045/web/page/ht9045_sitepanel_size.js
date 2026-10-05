/* AI(W906-SITEPANEL-SIZE) 20261005 -- 主畫面 Test Site 格子（#SitePanel，golden mtDutOnOff : TTMyTray）照配方的 Site 數變形。
 *  EastSun 1005：「這個紅框處應該是要照著Tools->Setup裡面所設定的Site數改變，現在的設定為Single Site，所以應該Show出單框，
 *  單框內容是1，然後亮綠色」。
 *  golden TfMain::DrawTestSitePanel（906 main.cpp:22319-22342）：mtDutOnOff->XItem = TestSocket.iShtCol、->YItem = TestSocket.iShtRow
 *  （IniConfig.bA09_ByArmCloseSite && !bUseTwoArm32Site 時 YItem×2＝兩臂上下排）。C++ 發布 tag site.cols / site.rows / site.byArm
 *  （WebBridgeTags.cpp AI(W906-SITEPANEL-SIZE)）。
 *  頁面的格子固定 8×4（main.html：x＝col，y＝(arm-1)*2+row，ht9045_wire_engine.js siteAddr），這裡只把範圍外的格子藏起來：
 *    非兩臂：只顯示 Arm1（y 0..1）的前 rows 列、前 cols 欄；兩臂：兩臂各前 rows 列。
 *  三個 tag 任一個不可知（null／還沒收到）＝全部 32 格照舊顯示（不猜）。顏色、點格事件都不動（照舊由 site.arm*.s* 與 ht9045_main_st01_ev.js）。
 */
(function () {
  'use strict';
  var st = { cols: null, rows: null, byArm: null };

  function apply() {
    var host = document.getElementById('SitePanel');
    if (!host) return;
    var known = (typeof st.cols === 'number' && typeof st.rows === 'number' && typeof st.byArm === 'number' && st.cols > 0 && st.rows > 0);
    var rowsUsed = {};
    Array.prototype.forEach.call(host.querySelectorAll('.cell'), function (c) {
      var x = parseInt(c.getAttribute('data-x'), 10), y = parseInt(c.getAttribute('data-y'), 10);
      var arm = Math.floor(y / 2), r = y % 2;
      var show = !known || (x < st.cols && r < st.rows && (st.byArm ? true : arm === 0));
      c.style.display = show ? '' : 'none';
      if (show) rowsUsed[y] = true;
    });
    Array.prototype.forEach.call(host.querySelectorAll('.trow'), function (row) {
      var c = row.querySelector('.cell');
      var y = c ? parseInt(c.getAttribute('data-y'), 10) : -1;
      row.style.display = (!known || rowsUsed[y]) ? '' : 'none';
    });
    var tray = host.querySelector('.mytray');
    if (tray) tray.title = 'mtDutOnOff : TTMyTray｜' + (known
      ? ('XItem=' + st.cols + ' YItem=' + (st.byArm ? st.rows * 2 : st.rows) + '（TestSocket.iShtCol／iShtRow' + (st.byArm ? '，兩臂' : '') + '）')
      : 'Site 數不可知（tag site.cols／site.rows／site.byArm 還沒有值），先顯示全部 32 格');
  }

  function hook() {
    if (!window.HT9045Tags) { setTimeout(hook, 300); return; }
    ['cols', 'rows', 'byArm'].forEach(function (k) {
      HT9045Tags.on('site.' + k, function (v) { st[k] = (typeof v === 'number') ? v : null; apply(); });
    });
    apply();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', hook); else hook();
})();
