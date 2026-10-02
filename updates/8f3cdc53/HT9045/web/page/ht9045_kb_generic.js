/* ht9045_kb_generic.js -- keyboard ranges that golden chooses by a MACHINE setting, chosen when the keyboard opens.
 * ---------------------------------------------------------------------------
 * AI(W906-C2) 20261002 (St02-E) new file (hand-written, not a gen_wire.py product); AI(W906-C8) 20261002: table-driven,
 * + Setup.TrayForm.  docs/QWERTY_FIRST_BRANCH_AUDIT_20261002.md fix 2: the generated ht9045_wire_*.js took the FIRST
 * ShowQwertyKey call of each golden handler; where the branch is a machine setting (not a customer code -- those take
 * golden's else, S25, and the laptop regenerates them, INBOX 140) both golden branches are kept and the right one is
 * chosen when the keyboard opens.  A capture-phase mousedown on the page's fields always opens the keyboard itself (the
 * generated kb entry is not used, so a later regeneration cannot widen it).  Same opening as ht9045_wire_engine.js
 * attachKeyboards (preventDefault; input + change after OK); the engine's own listener on the field is stopped.
 * AI(W906-C8) 20261002 (St02-E helper): each setting now follows golden's own value (St02-M review of !101).
 * [W906] unknown (read failed / not answered yet) -> the narrower range, the safe side; golden has no unknown state.
 *
 *   Main.CommView.html:177 (card C-2, claim St01 OK)  TfMain::edtSetZ1Click, the OnClick of edtSetZ1 AND edtSetZ2
 *     (main.dfm:15232 / :15249), golden 906_0625_Steven main.cpp:26175-26185:
 *       if(INDEX_DRIVER_TYPE==Mitsubishi_DRIVER) N_INTEGER, 0, true, 0, 100   (:26179)
 *       else (Panasonic)                         N_INTEGER, 0, true, 0, 300   (:26183)
 *     setting: golden rs232.cpp:103 INDEX_DRIVER_TYPE=CheckAndReadIniDataGeneral("IndexDriver", "INDEX_DRIVER_TYPE",
 *     Panasonic_DRIVER), read here from Gerneral.ini ('gerneral') with golden's rules (common.cpp:1444-1455): the key
 *     missing -> Panasonic_DRIVER (0, golden cmydef.h:80); present -> TIniFile ReadInteger, a text that is not an
 *     integer -> the same default.  rs232.cpp:107-111 turns Panasonic_DRIVER_A5 (2) into Panasonic_DRIVER (0): the choice
 *     does not change (only Mitsubishi_DRIVER = 1, golden cmydef.h:81, takes :26179).  The port reads it the same way at
 *     boot (rs232.cpp:219, tools/wb_serve.cpp:4052 W906_COM2_CreateFormBoot) but sends it to no page, so the file is read.
 *   Setup.TrayForm.html:133 (card C8)  TfTrayForm::Tp1TickUpMouseDown, the OnMouseDown of Tp1TickUp / Tp2TickUp /
 *     Tp3TickUp (cTrayForm.dfm:577 / :4355 / :8081), golden cTrayForm.cpp:789-801:
 *       if(IniConfig.bC03UseCatchTray) N_DOUBLE, 3, true, 60, 73    (:797-798, the catch-tray anti-collision width)
 *       else                           N_DOUBLE, 3, true, 30, 110   (:800)
 *     setting: C++'s own IniConfig.bC03UseCatchTray, NOT the LastSet.ini file.  golden cConfiguration.cpp:1054-1066:
 *     USE_AUTO_RETEST==eartInstall -> fixed (0 when SubMachineType==Type_HT9046AU, else 1), the file is not read;
 *     otherwise LastSet.ini [Tray] bC03UseCatchTray, default LastSet.bC03UseCatchTray (the binary lastdata.dat, cprod.cpp
 *     ReadLastDataFile; wb_serve does not serve it).  The port runs that edit list at boot (tools/wb_serve.cpp:4052
 *     FileRW_IniConfig_Boot -> FileRW/IniConfig.gen.inc:3979-3991, then ReadLastSetIni).  This page's own C route
 *     (editlist.get UserDefForm_File = golden TfTrayForm::FormShow, which the engine already runs when the page opens)
 *     sets Lab1/2/3XPickup->Caption from that same variable (golden cTrayForm.cpp:256-267; FileRW/UserDefForm_File.gen.inc
 *     :545-555) and sends the captions (proxies.<id>.caption, FileRW/_EditList.cpp:58); read here through
 *     HT9045Page.golden() (ht9045_wire_engine.js:2175) -- no extra request.  All three = golden's :258-260 text -> on;
 *     all three = :264-266 -> off; anything else -> unknown.
 *     [W906] golden reads IniConfig at the click (:797); here it is the value of this page's last FormShow (opening, or
 *     the engine's re-read after a save).  golden's TfTrayForm is modal (main.cpp:27497 ShowModal), so its operator
 *     cannot change C03 in between; a second browser screen saving Configuration can, until this page reads again.
 *     The Barcode_Reader(bcTrayForm) return before it (:792-795) is KYEC only (S25).
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';
  var P = window.HT9045Page, page = P && P.cfg && P.cfg.page;
  var CAP_ON = 'Catch Tray Range:60-73', CAP_OFF = 'Tray Range:30-110';   // golden cTrayForm.cpp:258-260 / :264-266
  var SETTINGS = {                                    // page -> the fields, where the setting comes from, golden's two branches
    'Main.CommView.html': {
      ids: ['edtSetZ1', 'edtSetZ2'], from: 'ini', file: 'gerneral', sec: 'IndexDriver', key: 'INDEX_DRIVER_TYPE',
      def: 0,                                         // Panasonic_DRIVER, golden rs232.cpp:103's default
      pick: function (s) {                            // main.cpp:26183 when known and not Mitsubishi_DRIVER (1); else :26179
        return (s.known && s.value !== 1) ? ['INTEGER', 0, true, 0, 300] : ['INTEGER', 0, true, 0, 100];
      }
    },
    'Setup.TrayForm.html': {
      ids: ['Tp1TickUp', 'Tp2TickUp', 'Tp3TickUp'], from: 'golden', struct: 'UserDefForm_File',
      labels: ['Lab1XPickup', 'Lab2XPickup', 'Lab3XPickup'],
      pick: function (s) {                            // cTrayForm.cpp:800 when known and off; else :798 (on, or unknown)
        return (s.known && s.value === false) ? ['DOUBLE', 3, true, 30, 110] : ['DOUBLE', 3, true, 60, 73];
      }
    }
  };
  var CFG = SETTINGS[page] || null;
  var STATE = { known: false, value: null, error: null };
  function findCI(obj, name) {                        // GetPrivateProfileString: names are case-insensitive, the first one wins
    if (!obj || typeof obj !== 'object') return undefined;
    var low = name.toLowerCase(), ks = Object.keys(obj);
    for (var i = 0; i < ks.length; i++) if (ks[i].toLowerCase() === low) return obj[ks[i]];
    return undefined;
  }
  function cellText(c) {                              // HT9045System.read ini cell -> the text BCB6 reads (bcb = trimmed)
    if (c && typeof c === 'object') c = (typeof c.bcb === 'string') ? c.bcb : (c.raw !== undefined && c.raw !== null) ? c.raw : c.value;
    return c == null ? '' : String(c).trim();
  }
  function iniInteger(t, def) {                       // BCB6 TCustomIniFile::ReadInteger: '0x..' -> '$..', then StrToIntDef
    if (t.length > 2 && /^0x/i.test(t)) t = '$' + t.slice(2);
    var m = /^([+-]?)(?:(\d+)|\$([0-9a-f]+))$/i.exec(t);
    if (!m) return def;
    var n = (m[2] !== undefined) ? parseInt(m[2], 10) : parseInt(m[3], 16);
    if (m[1] === '-') n = -n;
    return (n >= -2147483648 && n <= 2147483647) ? n : def;   // [W906] out of a Longint -> the default (Val reports an error)
  }
  function readIni() {
    var S = window.HT9045System;
    if (!S || typeof S.read !== 'function') { STATE.error = 'no HT9045System'; return Promise.resolve(); }
    return Promise.resolve().then(function () { return S.read(CFG.file); }).then(function (d) {
      if (!d || typeof d !== 'object' || d.available === false || !d.sections || typeof d.sections !== 'object') {
        STATE.error = CFG.file + ' could not be read'; return;                                    // the safe side
      }
      var c = findCI(findCI(d.sections, CFG.sec), CFG.key);
      STATE.value = (c === undefined) ? CFG.def : iniInteger(cellText(c), CFG.def);              // common.cpp:1446-1452
      STATE.known = true; STATE.error = null;
      STATE.from = (c === undefined) ? 'golden default (no ' + CFG.key + ' in [' + CFG.sec + '])' : CFG.file + ' [' + CFG.sec + ']';
    }, function (e) { STATE.error = String((e && e.message) || e); });
  }
  function goldenSetting() {                          // C++'s IniConfig.bC03UseCatchTray, as this page's FormShow showed it
    var H = window.HT9045Page, g = null;
    try { g = (H && typeof H.golden === 'function') ? H.golden() : null; } catch (e) {
      return { known: false, value: null, error: 'HT9045Page.golden(): ' + String((e && e.message) || e) };
    }
    if (!g || !g.page) return { known: false, value: null, error: 'the C route has not answered yet (editlist.get ' + CFG.struct + ')' };
    if (g.struct !== CFG.struct) return { known: false, value: null, error: 'C route ' + g.struct + ', not ' + CFG.struct };
    var px = (g.page.proxies && typeof g.page.proxies === 'object') ? g.page.proxies : {};
    var caps = CFG.labels.map(function (id) { return px[id] ? px[id].caption : undefined; });
    function all(t) { return caps.every(function (c) { return c === t; }); }
    if (all(CAP_ON)) return { known: true, value: true, error: null };
    if (all(CAP_OFF)) return { known: true, value: false, error: null };
    return { known: false, value: null, error: CFG.labels.join('/') + ' captions are not golden FormShow\'s: ' + JSON.stringify(caps) };
  }
  function snap() { return { known: STATE.known, value: STATE.value, error: STATE.error, from: STATE.from || null }; }
  function current() { return (CFG && CFG.from === 'golden') ? goldenSetting() : snap(); }
  var readPromise = null;
  if (CFG) {
    readPromise = (CFG.from === 'ini') ? readIni() : Promise.resolve();   // the C route value is taken at the click
    document.addEventListener('mousedown', function (ev) {
      var el = ev && ev.target;
      if (!el || !el.id || CFG.ids.indexOf(el.id) < 0 || el.disabled) return;
      var Q = window.HTQwerty;
      if (!Q || !Q.show) return;
      var kb = CFG.pick(current());                   // unknown -> the narrower range (the safe side)
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
                             setting: current, settingRead: function () { return readPromise; },
                             driver: function () { var s = current(); return { known: s.known, type: s.value, error: s.error }; },   // C-2 name
                             driverRead: function () { return readPromise; } };
})();
