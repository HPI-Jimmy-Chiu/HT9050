/* ht9045_kb_generic.js -- keyboard ranges that golden chooses by a MACHINE setting, chosen when the keyboard opens.
 * ---------------------------------------------------------------------------
 * AI(W906-C2) 20261002 (St02-E) new file (hand-written, not a gen_wire.py product).  Card C-2 "machine-setting fields"
 * (St02-M 20261002 05:1x), docs/QWERTY_FIRST_BRANCH_AUDIT_20261002.md fix 2: the generated ht9045_wire_*.js took the
 * FIRST ShowQwertyKey call of each golden handler; where the branch is a machine setting (not a customer code -- those
 * take golden's else, S25, and the laptop regenerates them, INBOX 140) both golden branches are kept and the right one is
 * chosen when the keyboard opens.
 *
 *   Main.CommView.html:177 (same-line claim, St01 OK 20261002)  TfMain::edtSetZ1Click -- the OnClick of edtSetZ1 AND
 *   edtSetZ2 (main.dfm:15232 / :15249) -- golden 906_0625_Steven main.cpp:26175-26185:
 *       if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER) ShowQwertyKey(..., N_INTEGER, 0, true, 0, 100);   // :26179 (generated)
 *       else                                     ShowQwertyKey(..., N_INTEGER, 0, true, 0, 300);   // :26183 (Panasonic)
 *     INDEX_DRIVER_TYPE = Gerneral.ini [IndexDriver] INDEX_DRIVER_TYPE (Mitsubishi_DRIVER = 1, cmydef.h:94), read once
 *     through HT9045System.read('gerneral') when this file loads.  A capture-phase mousedown opens the keyboard itself:
 *     0..300 when the value is known and not Mitsubishi; otherwise -- Mitsubishi, or the read failed / has not answered
 *     yet -- 0..100, the narrower range (the safe side).  It does not rely on the generated kb entry, so a later
 *     regeneration of the wire data cannot change this.  Same opening as ht9045_wire_engine.js attachKeyboards
 *     (preventDefault; input + change after OK); the engine's own listener on the field is stopped.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var P = window.HT9045Page, page = P && P.cfg && P.cfg.page;
  var DRIVER = { known: false, type: null, error: null };
  var Z_IDS = ['edtSetZ1', 'edtSetZ2'];
  var Z_ELSE = ['INTEGER', 0, true, 0, 300];          // golden main.cpp:26183 (not Mitsubishi_DRIVER)
  var Z_MITSU = ['INTEGER', 0, true, 0, 100];         // golden main.cpp:26179 (Mitsubishi_DRIVER) -- also when unknown
  var MITSUBISHI_DRIVER = 1;                          // cmydef.h:94
  function cellNum(c) {                               // HT9045System.read ini cell -> number (or NaN)
    if (c && typeof c === 'object') c = (c.value !== undefined && c.value !== null) ? c.value : c.raw;
    return parseInt(String(c == null ? '' : c), 10);
  }
  function readDriver() {
    var S = window.HT9045System;
    if (!S || typeof S.read !== 'function') { DRIVER.error = 'no HT9045System'; return Promise.resolve(); }
    return Promise.resolve().then(function () { return S.read('gerneral'); }).then(function (d) {
      var sec = d && d.sections && d.sections.IndexDriver, v = cellNum(sec && sec.INDEX_DRIVER_TYPE);
      if (isNaN(v)) { DRIVER.error = 'INDEX_DRIVER_TYPE not in Gerneral.ini [IndexDriver]'; return; }
      DRIVER.known = true; DRIVER.type = v; DRIVER.error = null;
    }, function (e) { DRIVER.error = String((e && e.message) || e); });
  }
  var zPromise = null;
  if (page === 'Main.CommView.html') {
    zPromise = readDriver();
    document.addEventListener('mousedown', function (ev) {
      var el = ev && ev.target;
      if (!el || !el.id || Z_IDS.indexOf(el.id) < 0 || el.disabled) return;
      var Q = window.HTQwerty;
      if (!Q || !Q.show) return;
      var kb = (DRIVER.known && DRIVER.type !== MITSUBISHI_DRIVER) ? Z_ELSE : Z_MITSU;   // unknown -> the safe side
      ev.preventDefault();
      ev.stopPropagation();                           // the engine's listener on the field does not open a second keyboard
      Q.show(el, (Q.N && Q.N[kb[0]]) || 0, {
        dp: kb[1], checkRange: kb[2], min: kb[3], max: kb[4],
        onCommit: function () {
          ['input', 'change'].forEach(function (evn) {
            var e2;
            try { e2 = new Event(evn, { bubbles: true }); } catch (x) { e2 = document.createEvent('Event'); e2.initEvent(evn, true, true); }
            el.dispatchEvent(e2);
          });
        }
      });
    }, true);
  }
  window.HT9045KbGeneric = { page: page || null,                                           // probe / ctest KB_MachineSetting
                             driver: function () { return { known: DRIVER.known, type: DRIVER.type, error: DRIVER.error }; },
                             driverRead: function () { return zPromise; } };
})();
