/* ht9045_teach_homeall_c.js -- HW.teach.html「Home All」：全機回原點的測試鈕。
 * ---------------------------------------------------------------------------
 * AI(W906-TEACH-HOMEALL) 20261003: EastSun「請你先幫我在 teach 做一個全機回 home 的按鈕 我需要做測試 因為現在實體按鈕沒作用」。
 *   機台 web 0096（cpp 0153 同一包）；ST02-P2 20261006 (St02-E) 搬進 main，內容不變。
 *   golden 沒有這顆鈕：真機是用面板 HOME 鍵回原點（TfMain::ScanKey main.cpp:2545-2563），主畫面的 HOME 在真機上不動作。
 *   這顆鈕送 HTMotorAccess.send('btnHomeAll')（指令表 uteach btnHomeAll -> teachHomeAll），C++ 跑的就是面板 HOME 鍵那一段
 *   （WebMainScanKey.cpp W906_PanelHomeKeyArm），回原點的順序是 golden 的（Z 先回完再回 XY，最後 TrayX／CCDY）。
 *   C++ 接受後會關掉教導頁：golden 教導頁開著時 MainProc 每拍都 return，回原點要等教導頁關了才會開始；進度看 Home Monitor。
 *   運轉中、手動教導中、還有單軸回原點／JOG 在跑時 C++ 拒絕，理由顯示在狀態列。不加確認視窗（EastSun：按下就送）。
 * 手寫，不是 gen_wire.py 產物（檔名同 ht9045_teach_zallup_c.js 的理由）。
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';
  var BTN = 'btnHomeAll', ACTION = 'teachHomeAll', WAIT_MS = 30000, POLL_MS = 300;

  function $(id) { return document.getElementById(id); }
  function info(msg, cls) {
    if (typeof global.teachSetInfo === 'function') { try { global.teachSetInfo(msg, cls); return; } catch (e) {} }
    if (global.console) console.warn('[homeall] ' + msg);
  }
  function unwired(why) {
    var el = $(BTN);
    if (!el) return;
    if (typeof global.teachMarkUnwiredEl === 'function') global.teachMarkUnwiredEl(el, why);
    else el.setAttribute('data-unwired', why);
  }

  function onClick() {
    var ma = global.HTMotorAccess;
    if (!ma) { info('Home All：HTMotorAccess 不存在（motor-access.js 沒載入）', 'err'); return; }
    ma.send(BTN);                                        // 結果（接受／拒絕理由）由 HTMotorAccess 顯示在狀態列
  }

  function bind(cat) {
    var cmds = (cat && cat.commands) || [], have = false;
    for (var i = 0; i < cmds.length; i++) {
      var c = cmds[i];
      if (c && c.source === 'uteach' && c.button === BTN && c.action === ACTION) { have = true; break; }
    }
    var el = $(BTN);
    if (!el) { info('Home All：頁面上找不到 ' + BTN + '（頁面改版？）', 'warn'); return; }
    if (!have) { unwired('Home All：指令表 motor-access.json 還沒有 uteach ' + BTN + ' → ' + ACTION + ' 這一列，按了不會動作'); return; }
    if (el.getAttribute('data-homeall')) return;
    el.setAttribute('data-homeall', '1');
    el.setAttribute('data-acc', '1');
    el.addEventListener('click', onClick);
    document.documentElement.setAttribute('data-teach-homeall', 'ok');
  }

  function boot() {
    var t0 = Date.now();
    (function poll() {
      var ma = global.HTMotorAccess, d = ma && ma.debug ? ma.debug() : null;
      if (d && d.hasCatalog && d.source === 'uteach') {
        ma.loadJson('../JSON/motor-access.json').then(bind, function (e) {
          unwired('Home All：讀不到指令表 motor-access.json（' + ((e && e.message) || String(e)) + '），這顆沒有接上');
        });
        return;
      }
      if (Date.now() - t0 > WAIT_MS) {
        document.documentElement.setAttribute('data-teach-homeall', 'fail');
        unwired('Home All：等了 ' + (WAIT_MS / 1000) + ' 秒，教導頁的馬達指令通道（HTMotorAccess）還沒初始化（重新整理頁面再試）');
        return;
      }
      setTimeout(poll, POLL_MS);
    })();
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true });
  else boot();
  global.HT9045TeachHomeAll = { BTN: BTN, ACTION: ACTION };
})(window);
