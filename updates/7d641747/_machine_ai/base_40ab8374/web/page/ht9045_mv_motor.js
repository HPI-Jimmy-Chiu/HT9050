/* ---------------------------------------------------------------------------
 * ht9045_mv_motor.js -- INBOX 117: feed Main.MotionView's LIVE motor positions from the Handler (laptop).
 *
 * AI(W906-MV-LIVE) 20260929 [W906]: the user asked (0929 19:1x) "確認 motion view 畫面是否能因為 motor 位置不同而更新內容".
 *   Measured on the SIM wb_serve (S1 flow, 20:53-21:01): /api/struct/motor/runtime moves (MInArmX 0 -> 30732 -> 4203 ->
 *   35399 ..., MTrayX 0 -> 39662 -> 61662 -> 96912), but the Motion View arms never moved: the page's liveMech()
 *   (Main.MotionView.html:1058-1080) places the in / out arms, shuttles and index from LIVE.motor[<alias>] (pulses), and
 *   nothing ever filled LIVE.motor -- ht9045_mv_trays.js (S118) sends trays only.
 *
 *   This file fills it: every 500 ms (same as Main.MotorView.html / HW.MotorTest.html:1286) it reads
 *   /api/struct/motor/runtime and puts {<motorId>: cmdPos} into LIVE.motor, then calls the page's render().
 *   cmdPos, not encPos: golden moves the Motion View panels from TMyMotor::Position (MotorMove, golden
 *   Motor/mymotor.cpp:935-951, ScreenPos = Scale*(Position-FactStart)+RefStart); an axis whose cmdPos is null falls
 *   back to encPos, and an axis with neither is left out (liveMech() then keeps its static default -- no guessing).
 *
 *   It only ADDS to a LIVE the page already has (the tray feed makes the page LIVE); with no LIVE (no tray tags,
 *   disconnected) it does nothing, so the page's "等待 C++ Runtime 發布" state is unchanged. applyRuntimeState() is
 *   wrapped so a new LIVE from the tray feed keeps the last positions (otherwise every tray push would snap the arms
 *   back to their defaults until the next poll).
 *
 *   Not golden: golden moves the panels only while CheckBox2 "Simulte Enable" is ticked on the Motion View tab
 *   (golden main.cpp:8676-8686 CheckBox2Click sets MOT[i].bShowMotorMove=true; main.dfm:12275). This page is Steven's
 *   HTML design and already draws LIVE positions whenever it is LIVE; this file follows the page, not the checkbox
 *   (NIGHT_REPORT 0929 section 0 asks whether to add the checkbox).
 *   No poll while the window is closed (HT_WIN from background.html) or the document is hidden; file:// = no server,
 *   nothing happens.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  if (location.protocol !== 'http:' && location.protocol !== 'https:') return;
  var URL_RT = '/api/struct/motor/runtime';
  var POLL_MS = 500;
  var MOTOR = null, busy = false, winHosted = false, winShown = false, fails = 0;

  function pollAllowed() { return !document.hidden && (!winHosted || winShown); }

  function positions(rt) {
    var out = {}, arr = (rt && rt.motors) || [], i, r, p, v;
    for (i = 0; i < arr.length; i++) {
      r = arr[i]; p = (r && r.position) || {};
      v = (p.cmdPos != null) ? p.cmdPos : p.encPos;
      if (r && r.motorId && v != null) out[r.motorId] = v;
    }
    return out;
  }

  function same(a, b) {
    if (!a || !b) return false;
    var k;
    for (k in a) if (a[k] !== b[k]) return false;
    for (k in b) if (!(k in a)) return false;
    return true;
  }

  function apply() {
    var L = window.LIVE;
    if (!MOTOR || !L || typeof L !== 'object') return;
    if (same(L.motor, MOTOR)) return;
    L.motor = MOTOR;
    if (typeof window.render === 'function') window.render();
  }

  function wrapApply() {
    var orig = window.applyRuntimeState;
    if (typeof orig !== 'function' || orig.__w906MvMotor) return typeof orig === 'function';
    var w = function (runtime) {
      var mv = runtime && runtime.state && runtime.state.motionView;
      if (mv && MOTOR && mv.motor == null) mv.motor = MOTOR;
      return orig.apply(this, arguments);
    };
    w.__w906MvMotor = true;
    window.applyRuntimeState = w;
    return true;
  }

  function poll() {
    if (busy || !pollAllowed()) return;
    busy = true;
    fetch(URL_RT + '?_=' + Date.now(), { cache: 'no-store' }).then(function (r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    }).then(function (rt) {
      busy = false; fails = 0;
      MOTOR = positions(rt);
      apply();
    }).catch(function () {
      busy = false; fails++;
    });
  }

  window.addEventListener('message', function (ev) {
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    winHosted = true; winShown = !!m.open;
    if (winShown) poll();
  });

  var tries = 0;
  (function start() {
    if (!wrapApply() && ++tries < 40) { setTimeout(start, 250); return; }
    poll();
    setInterval(poll, POLL_MS);
  })();
})();
