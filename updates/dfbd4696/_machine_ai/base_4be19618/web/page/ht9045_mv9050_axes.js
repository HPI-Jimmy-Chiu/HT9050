/* ---------------------------------------------------------------------------
 * ht9045_mv9050_axes.js -- Main.MotionView9050.html: fill the "軸讀數 pulse / mm" table with the Handler's live positions.
 *
 * AI(W906-MV9050-WIRE) 20261002: Jimmy 1002 20:2x「現在已經有HT9050專用的motion ， Main.MotionView9050.html ，需要接上去」.
 *   The page (Steven's HTML-only design, runtimeSupported:false) builds the axis table from MotionView9050-layout.json
 *   axes.bindings (renderAxes / axRow: id | type | reading | label), and its reading column is always "–": the page has no
 *   producer for its state.motionView contract (machine:'HT9050', stations, hold / vacuum ...) and draws no LIVE positions.
 *   This file fills only that column, from the same source the HT9045 page uses for its arms (ht9045_mv_motor.js):
 *   /api/struct/motor/runtime every 500 ms, cmdPos (golden moves the Motion View panels from TMyMotor::Position, the
 *   commanded position; mymotor.cpp:935-951), falling back to encPos; neither = "—". Pulses, as the API gives them --
 *   no conversion to mm and no placing of the drawing (the page's own rule: LIVE shows what the Runtime publishes and
 *   infers nothing; its drawing waits for a state.motionView producer, NB2 R161 / NIGHT_REPORT).
 *   Rows: the page's own rows; a row is matched by the last word of its first cell (motorId, e.g. "M00 MInArmX" ->
 *   MInArmX). axis-disabled rows (axes.absent: HT9045 axes HT9050 does not have) and the cylinder row (ioId, not a motor)
 *   keep "–". renderAxes() rebuilds the table (layout reload) -> the next poll fills it again.
 *   Polls only while the page is not gated (body.gated = not HT9050), the window is open (background.html HT_WIN /
 *   WIN_STATE, as ht9045_mv_motor.js; RULINGS_20260930 #12 "有開的網頁才能更新資料") and the document is visible.
 *   file:// = no server, nothing happens.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  if (location.protocol !== 'http:' && location.protocol !== 'https:') return;
  var URL_RT = '/api/struct/motor/runtime';
  var POLL_MS = 500;
  var busy = false, winHosted = false, winShown = false, fails = 0;
  (function () {   // the frame's state at start (this page may be loaded after the window's initial HT_WIN went by)
    try {
      var fe = window.frameElement, w = fe && fe.closest && fe.closest('.win'), W = w && window.parent && window.parent.WIN_STATE;
      if (W) { var st = W[String(w.id).replace(/^win-/, '')]; winHosted = true; winShown = (st === 'open' || st === 'minimized'); }
    } catch (e) { /* not same-origin / no frame: as a top-level page */ }
  })();

  var liveBar, lastSeq = null, lastSeqAt = 0, lastPositions = null;
  function status(text, ok) {
    if (!liveBar) {
      liveBar = document.createElement('div');
      liveBar.setAttribute('role', 'status');
      liveBar.style.cssText = 'position:sticky;top:0;z-index:50;padding:8px 12px;background:#fff;border-bottom:1px solid #b9c2c8;font:14px sans-serif;';
      document.body.insertBefore(liveBar, document.body.firstChild);
    }
    liveBar.style.color = ok ? '#17683d' : '#a13c13';
    liveBar.textContent = text;
  }
  function updateStatus(rt, pos) {
    var r = rt && rt.runtime, now = Date.now(), moved = false;
    if (!r || r.connected !== true || r.seq == null) {
      status('尚未收到即時資料，請確認控制程式已啟動。', false); return;
    }
    if (r.seq !== lastSeq) { lastSeq = r.seq; lastSeqAt = now; }
    Object.keys(pos).forEach(function (id) {
      if (lastPositions && lastPositions[id] && lastPositions[id].v !== pos[id].v) moved = true;
    });
    lastPositions = pos;
    if (now - lastSeqAt > 3000) {
      status('資料已超過 3 秒未更新，請檢查程式連線。', false); return;
    }
    status('即時資料更新中｜' + (moved ? '馬達位置有變化' : '馬達位置未變（可能停止或等候）') +
      '｜更新第 ' + r.seq + ' 次。請看下方軸讀數；示意圖尚未顯示即時動作。', true);
  }

  function gated() { return !!(document.body && document.body.classList.contains('gated')); }
  function pollAllowed() { return !document.hidden && (!winHosted || winShown) && !gated(); }

  function positions(rt) {
    var out = {}, arr = (rt && rt.motors) || [], i, r, p;
    for (i = 0; i < arr.length; i++) {
      r = arr[i]; p = (r && r.position) || {};
      if (!r || !r.motorId) continue;
      if (p.cmdPos != null) out[r.motorId] = { v: p.cmdPos, src: 'cmd' };
      else if (p.encPos != null) out[r.motorId] = { v: p.encPos, src: 'enc' };
    }
    return out;
  }

  function fill(pos) {
    var rows = document.querySelectorAll('#axTable tbody tr'), i, tr, td, id, hit, txt, tip;
    for (i = 0; i < rows.length; i++) {
      tr = rows[i];
      if (tr.classList.contains('axis-disabled') || tr.cells.length < 3) continue;
      id = String(tr.cells[0].textContent || '').trim().split(/\s+/).pop();
      td = tr.cells[2];
      hit = pos && Object.prototype.hasOwnProperty.call(pos, id) ? pos[id] : null;
      if (!hit && !(pos && td.getAttribute('data-live'))) continue;   // not a motor of the API (cylinder) and never filled
      txt = hit ? String(Math.round(Number(hit.v))) + (hit.src === 'enc' ? '（enc）' : '') : '—';
      tip = hit ? (hit.src === 'cmd' ? 'cmdPos（/api/struct/motor/runtime）' : 'encPos（沒有 cmdPos）') : '這一軸沒有讀值';
      if (td.textContent !== txt) td.textContent = txt;
      if (td.title !== tip) td.title = tip;
      td.setAttribute('data-live', '1');
    }
  }

  function poll() {
    if (busy || !pollAllowed()) return;
    busy = true;
    fetch(URL_RT + '?_=' + Date.now(), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    }).then(function (rt) {
      busy = false; fails = 0;
      var pos = positions(rt);
      fill(pos); updateStatus(rt, pos);
    }).catch(function () {
      busy = false;
      if (++fails >= 3) { fill({}); status('無法取得即時資料，請確認控制程式與連線。', false); }   // server gone: show "—" instead of the last positions
    });
  }

  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    winHosted = true; winShown = !!m.open;
    if (winShown) poll();
  });
  window.HT9045Mv9050Axes = { poll: poll, positions: positions, fill: fill };
  poll();
  setInterval(poll, POLL_MS);
})();
