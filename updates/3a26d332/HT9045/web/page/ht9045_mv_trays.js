/* ---------------------------------------------------------------------------
 * ht9045_mv_trays.js -- S118: feed Main.MotionView's LIVE tray state from the Handler (St02-E).
 *
 * AI(W906-S118) 20260928 (St02-E): the C++ producer JsonBridge/ChanMvTrays.* publishes one string tag per tray,
 *   motionView.trays.<name> = {"name","xItem","yItem","ver","cells":[[row0...],...]}, plus motionView.trays.ver (+1 on any
 *   change) and motionView.trays.keys (a JSON array of the 34 names). This file reads them through HT9045Tags
 *   (ht9045_recipe_client.js), builds {state:{motionView:{trays:{<name>: tray}}}} and hands it to the page's own
 *   applyRuntimeState() + rebuild() / render() (Main.MotionView.html :2904 / :2816 / :2453), which already draws LIVE
 *   trays (liveSnap / livePanel / applyLiveCell).
 *   Loaded from Main.MotionView.html :2970 (the laptop-approved same-line hook).
 * No tags yet (an older wb_serve, or the producer not staged): nothing happens -- the page keeps its static layout.
 * The page decides which trays to draw per machine profile (W54 = A, Steven 0928); this file passes all it gets.
 *
 * AI(W906-S118-A13) 20260928 (St02-E2 draft, reviewed and committed by St02-E; choices: disconnected = not live, 1005 and
 *   HAS_NULL_CLEAN_IC 8 count as a device as golden HasRealIC does, reconnect back-off 2 -> 15 s; review A13): each cell goes out as {item, raw, colorCss} (the page's
 *   liveCell() takes objects; livePanel() counts a device when liveHas(item), applyLiveCell() paints colorCss).
 *   - colour = golden TTMyTray: idx = raw >= 1000 ? raw - 1000 : raw (golden 906_0625_Steven Motor/mymotor.cpp:1115-1116,
 *     :1128-1129), palette = TTMyTray's default ColorMap (elec\myvcl\HTray.cpp:23-27, BGR TColor -> CSS RGB; slots 11-31
 *     are 0 = clBlack), an index < 0 or >= 32 paints nothing (SetCellColorIndex :384-385); the autoClean tray paints
 *     index 7 (HAS_CLEAN_IC) clInactiveCaption (AutoClean/uCleaning.cpp:1925; classic-theme #BFCDDB, 待上機確認).
 *   - occupancy = golden TMyTray::HasRealIC (mytray.cpp:192-199): a device only when raw != 0 and raw != HAS_NULL_IC (5);
 *     65535 stays empty as the page always had it. So item = 0 for those three, else the raw value. golden's output trays
 *     store a status too (HAS_IC / HAS_BARCODEERROR_IC / NULL_IC, aoutarm9045.cpp:2683-2714; the bin is the cell TEXT,
 *     SetCellNumber), so 5 is never a bin number.
 *   - raw = the untouched Tray.Data value (for anyone who needs it; the page ignores unknown fields).
 * AI(W906-S118-A15) 20260928 (St02-E2 draft, review A15): wb_serve restarts / socket drops.
 *   - HT9045Tags does not reconnect by itself (ht9045_recipe_client.js:141-160): a 2 s watchdog re-calls T.connect()
 *     (back-off 2 -> 4 -> 8 -> 15 s) while status().connected is false.
 *   - lastVer is reset on the reconnect edge, on a generation change, and when ver goes backwards (a restarted
 *     wb_serve publishes ver = 34 again, which could equal the old lastVer and hide the new trays).
 *   - while disconnected, or when the tray tags vanish from a snapshot, the page is put back into its not-live state
 *     (applyRuntimeState(null) + rebuild(): "等待 C++ Runtime 發布" + the static layout) instead of showing the last trays.
 * AI(W906-S118-A16) 20260928 (St02-E2 draft, review A16): rebuild() (build() + grids + axes + timeline, P=0) only on the
 *   first push, after a reset, or when a tray's size changed; every other change is render(). A tray tag string that
 *   did not change is not parsed again.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var T = window.HT9045Tags;
  if (!T || typeof T.subscribe !== 'function' || typeof T.get !== 'function') return;

  // TTMyTray default ColorMap (HTray.cpp:23-27) as CSS RGB; index 11-31 = clBlack (the rest of the 32-slot array is 0)
  var PALETTE = ['#FFFFFF', '#008000', '#FFFF00', '#00FF00', '#FF0000', '#8080FF', '#BE34DC', '#0000FF', '#FF80FF',
                 '#8000FF', '#80FF00'];
  var MAX_COLOR_INDEX = 32;                 // HTray.cpp SetCellColorIndex: Index >= MAX_COLOR_INDEX -> ignored
  var HAS_NULL_IC = 5;                      // V906 cmydef.cpp:159 (golden 906_0625_Steven cmydef.cpp:155)
  var HAS_CLEAN_IC = 7;                     // V906 cmydef.cpp:161 (golden 906_0625_Steven cmydef.cpp:157)
  var AUTOCLEAN_CLEAN_IC = '#BFCDDB';       // clInactiveCaption, classic theme (uCleaning.cpp:1925)

  var lastVer = null, lastGen = null, lastSizes = '', live = false, wasConnected = false;
  var cache = {};                           // name -> {s: tag string, t: converted tray}
  var retryMs = 2000, nextTry = 0, connecting = false;

  function parse(v) {
    if (typeof v !== 'string' || v === '') return null;
    try { return JSON.parse(v); } catch (e) { return null; }
  }

  function colorOf(name, raw) {
    if (typeof raw !== 'number' || raw !== Math.floor(raw)) return null;
    var idx = raw >= 1000 ? raw - 1000 : raw;
    if (idx < 0 || idx >= MAX_COLOR_INDEX) return null;
    if (name === 'autoClean' && idx === HAS_CLEAN_IC) return AUTOCLEAN_CLEAN_IC;
    return idx < PALETTE.length ? PALETTE[idx] : '#000000';
  }

  function cellOf(name, raw) {
    if (typeof raw !== 'number') return raw;          // null / missing: the page's liveCell() skips it
    var empty = (raw === 0 || raw === 65535 || raw === HAS_NULL_IC);
    var c = { item: empty ? 0 : raw, raw: raw };
    var css = colorOf(name, raw);
    if (css) c.colorCss = css;
    return c;
  }

  function convert(name, t) {
    var out = {}, k, r, c, row, rows = [];
    for (k in t) if (Object.prototype.hasOwnProperty.call(t, k) && k !== 'cells') out[k] = t[k];
    if (t.cells && t.cells.length) {
      for (r = 0; r < t.cells.length; r++) {
        row = t.cells[r] || [];
        var o = [];
        for (c = 0; c < row.length; c++) o.push(cellOf(name, row[c]));
        rows.push(o);
      }
    }
    out.cells = rows;
    return out;
  }

  function trayOf(name) {
    var s = T.get('motionView.trays.' + name);
    var hit = cache[name];
    if (hit && hit.s === s) return hit.t;
    var t = parse(s);
    var conv = (t && typeof t === 'object') ? convert(name, t) : null;
    cache[name] = { s: s, t: conv };
    return conv;
  }

  function resetVer() { lastVer = null; lastSizes = ''; }

  // AI(W906-INBOX117) 20260929 (St02-E): INBOX 117 -- golden SetScreenScale pairs (motionView.screenScale) for liveMech in Main.MotionView.html.
  //   {alias: [refStart, refEnd, factStart, factEnd]}; null when the tag is not in the snapshot (an older wb_serve).
  var lastScaleVer = null;
  function pushScale() {
    var v = T.get('motionView.screenScale.ver');
    if (v === null || v === undefined) { window.MV_SCREEN_SCALE = null; lastScaleVer = null; return; }
    if (v === lastScaleVer) return;
    var sc = parse(T.get('motionView.screenScale'));
    if (!sc) return;
    lastScaleVer = v;
    window.MV_SCREEN_SCALE = sc;
  }

  // back to the page's not-live state (the static layout + "等待 C++ Runtime 發布")
  function dropLive() {
    if (!live) return;
    live = false;
    resetVer();
    cache = {};
    if (typeof window.applyRuntimeState === 'function') window.applyRuntimeState(null);
    if (typeof window.rebuild === 'function') window.rebuild();
  }

  function push() {
    pushScale();
    var st = typeof T.status === 'function' ? T.status() : null;
    if (st) {
      if (st.connected && !wasConnected) resetVer();              // reconnect edge: the snapshot is a new baseline
      wasConnected = !!st.connected;
      if (typeof st.generation === 'number' && st.generation !== lastGen) { lastGen = st.generation; resetVer(); }
    }
    var ver = T.get('motionView.trays.ver');
    if (ver === null || ver === undefined) { dropLive(); return; }  // the tray tags left the snapshot
    if (typeof ver === 'number' && typeof lastVer === 'number' && ver < lastVer) resetVer();   // wb_serve restarted
    if (ver === lastVer) return;
    var keys = parse(T.get('motionView.trays.keys'));
    if (!keys || !keys.length) return;
    var trays = {}, sizes = [], any = false, i, t;
    for (i = 0; i < keys.length; i++) {
      t = trayOf(keys[i]);
      if (t) { trays[keys[i]] = t; any = true; sizes.push(keys[i] + ':' + t.xItem + 'x' + t.yItem); }
    }
    if (!any) return;
    if (typeof window.applyRuntimeState !== 'function') return;
    var sizeSig = sizes.join(',');
    var full = !live || lastVer === null || sizeSig !== lastSizes;
    lastVer = ver;
    lastSizes = sizeSig;
    live = true;
    window.applyRuntimeState({ state: { motionView: { trays: trays } } });
    if (full || typeof window.render !== 'function') {
      if (typeof window.rebuild === 'function') window.rebuild();
    } else {
      window.render();
    }
  }

  function tryConnect() {
    if (typeof T.connect !== 'function' || connecting) return;
    connecting = true;
    T.connect().then(function () {
      connecting = false;
      retryMs = 2000;
      push();
    }, function () {
      connecting = false;                                             // no socket: the page shows its static layout
      nextTry = Date.now() + retryMs;
      retryMs = Math.min(retryMs * 2, 15000);
    });
  }

  function watchdog() {
    var st = typeof T.status === 'function' ? T.status() : null;
    if (!st || st.connected) return;
    if (wasConnected) { wasConnected = false; dropLive(); }
    if (Date.now() >= nextTry) tryConnect();
  }

  window.HT9045MvTrays = {
    push: push,
    lastVer: function () { return lastVer; },
    cellOf: cellOf,                                                   // for a page-side check / a future test
    palette: PALETTE.slice(0)
  };
  T.subscribe(function () { push(); });
  if (typeof T.connect === 'function') {
    tryConnect();
    if (typeof T.status === 'function') window.setInterval(watchdog, 2000);
  } else {
    push();
  }
})();
