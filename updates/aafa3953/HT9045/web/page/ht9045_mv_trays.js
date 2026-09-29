/* ---------------------------------------------------------------------------
 * ht9045_mv_trays.js -- S118: feed Main.MotionView's LIVE tray state from the Handler (St02-E).
 *
 * AI(W906-S118) 20260928 (St02-E): the C++ producer JsonBridge/ChanMvTrays.* publishes one string tag per tray,
 *   motionView.trays.<name> = {"name","xItem","yItem","ver","cells":[[row0...],...]}, plus motionView.trays.ver (+1 on any
 *   change) and motionView.trays.keys (a JSON array of the 34 names). This file reads them through HT9045Tags
 *   (ht9045_recipe_client.js), builds {state:{motionView:{trays:{<name>: tray}}}} and hands it to the page's own
 *   applyRuntimeState() + rebuild() (Main.MotionView.html :2904), which already draws LIVE trays (liveSnap / livePanel).
 *   Loaded from Main.MotionView.html :2970 (the laptop-approved same-line hook).
 * No tags yet (an older wb_serve, or the producer not staged): nothing happens -- the page keeps its static layout.
 * The page decides which trays to draw per machine profile (W54 = A, Steven 0928); this file passes all it gets.
 * Cell values are raw Tray.Data; the colour rule (>= 1000 -> -1000, TTMyTray palette) is documented in
 * docs/handoff/ST02_S118_TRAY_PAYLOAD.md (v2).
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var T = window.HT9045Tags;
  if (!T || typeof T.subscribe !== 'function' || typeof T.get !== 'function') return;
  var lastVer = null;

  function parse(v) {
    if (typeof v !== 'string' || v === '') return null;
    try { return JSON.parse(v); } catch (e) { return null; }
  }

  function push() {
    var ver = T.get('motionView.trays.ver');
    if (ver === null || ver === undefined || ver === lastVer) return;
    var keys = parse(T.get('motionView.trays.keys'));
    if (!keys || !keys.length) return;
    var trays = {}, any = false, i, t;
    for (i = 0; i < keys.length; i++) {
      t = parse(T.get('motionView.trays.' + keys[i]));
      if (t && typeof t === 'object') { trays[keys[i]] = t; any = true; }
    }
    if (!any) return;
    if (typeof window.applyRuntimeState !== 'function') return;
    lastVer = ver;
    window.applyRuntimeState({ state: { motionView: { trays: trays } } });
    if (typeof window.rebuild === 'function') window.rebuild();
  }

  window.HT9045MvTrays = { push: push, lastVer: function () { return lastVer; } };
  T.subscribe(function () { push(); });
  if (typeof T.connect === 'function') {
    T.connect().then(push, function () { /* no socket: the page shows its static layout */ });
  } else {
    push();
  }
})();
