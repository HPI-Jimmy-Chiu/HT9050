/* =============================================================================
   ht9045_teach_st02.js -- HW.teach.html: St02 card ST02-C9, golden uteach.cpp buttons the laptop has not wired.
   AI(W906-ST02-C9-G2) 20261002 (St02-E helper).  golden = HT9011UC_Code_V3.33.906.0_20260625_Steven.

   G2, each a click -> HTMotorAccess.send('St02Teach*', {btn: <golden button>, ...the fields golden reads with atoi})
   -> C++ motor.access action "teachSt02" (WebMotorAccess.cpp EOF block, WebTeachSt02.h):
     SpeedButtonInRotatePos90 / SpeedButtonInRotateNeg90   golden :4605-4707   + backlashIn / backlashOut (edtEditRotateIn/OutBacklash)
       and the 34 other buttons golden gives the same two OnClick (uteach.dfm; ROT below): SpeedButtonOutRotatepPos90 / Neg90
       (grpRotate_Kit) and btnPos90 / btnNeg90 In / Out R A..H (tab tsRotate) -- golden's handler ignores the button: the motor
       is the selected one, the backlash field is chosen by that motor (C++), so every one of them sends both fields.
     btnSht1GoLatch / btnSht2GoLatch                       golden :4721-4751   + shtSpeed (EditSh1Speed / EditSh2Speed)
     btnSetAllInArmZ_Move / btnSetAllOutArmZ_Move          golden :5948-5996   (no field)
   AI(W906-ST02-C9) 20261002 (St02-E helper): St02's G3 (BtnPanelLane1-3, btnArm1Y / Arm2Y / Z1 / Z2Servo, spTTLReset /
     cbEnableTTLButtonUse) was dropped in the C9 rebase onto main 2dd90ef3: the machine wired them first (ht9045_io_do.js
     IOWIDGET, ht9045_teach_zallup_c.js TEACH-ZALLUP; the TTL box greyed by TEACH-FORMSHOW) -- this file does not touch them.
   The motor of the rotate buttons is the page's selected motor (golden ActiveMotorIndex = HTMotorAccess getMotors); C++ reads
   the position itself (golden P1=atoi(edtNowPosition), the same source).  The ack's message is shown on the status line by
   motor-access.js; a press that refused some axes comes back as an error (C++ sets state "error").

   Golden visibility (FormShow :1457-1458 / :1652 / :1668 / :1690-1703, uteach.dfm:5850; the tsRotate TAB is hidden as golden's
   TabVisible, its 16 grbIn/OutR* groups by their own Visible) comes from C++ (the same rules C++ refuses with):
   {btn:'St02State', query:true} sent straight to HT9045Recipe.motorAccess (a read: automatic acquire, no take-over, no lock),
   at window load and when the window is shown again.  AI(W906-ST02-C9) 20261002: main's TEACH-FORMSHOW
   (ht9045_teach_formshow_c.js, FileRW/TeachFormShow_File.cpp) applies the same golden Visible / TabVisible lines at load and
   at every window open; both read golden's globals, so they agree.
   shown[id] true / false -> the group shown / hidden; no answer (offline, refused while running, no C++) -> the dfm default
   is kept and the group is outlined (dashed) with the reason, retried every 5 s.

   Load order (HW.teach.html, ONE tag next to motor-access.js, :81): this file runs BEFORE the page's inline scripts, so the
   click listener below is the first capture listener on each button.  The page's TEACH_UNWIRED grey (teachMarkUnwiredEl,
   applied once at DOMContentLoaded; on main 2dd90ef3 TEACH_UNWIRED (TEACH-UNWIRED-2) lists the 36 rotate buttons and the two
   Set All Z _Move, and TEACH_UNWIRED_B38 the 36 rotate buttons and the Go Latch / Set All Z four) adds its own capture listener
   after it; when a button was greyed, this file removes the grey right after DOMContentLoaded (class teach-unwired, attribute
   data-unwired) and its listener stops the grey's listener (stopImmediatePropagation) -- only on a button that was greyed; a
   no-op otherwise.
   Pinned in tools/webprobe/teach_st02_selftest.cjs.
   ============================================================================= */
(function (global) {
  'use strict';
  var doc = global.document;
  if (!doc) return;
  var CAT_BTN = 'St02Teach*';
  // AI(W906-ST02-C9-G2) 20261002: the 36 buttons whose golden OnClick is SpeedButtonInRotatePos90Click / ...Neg90Click (uteach.dfm)
  var ROT_GRB = ['InRA', 'InRB', 'InRC', 'InRD', 'InRE', 'InRF', 'InRG', 'InRH', 'OutRA', 'OutRB', 'OutRC', 'OutRD', 'OutRE', 'OutRF', 'OutRG', 'OutRH'];
  var ROT = ['SpeedButtonInRotatePos90', 'SpeedButtonInRotateNeg90', 'SpeedButtonOutRotatepPos90', 'SpeedButtonOutRotatepNeg90'];
  ROT_GRB.forEach(function (x) { ROT.push('btnPos90' + x); ROT.push('btnNeg90' + x); });
  var G2 = ROT.concat(['btnSht1GoLatch', 'btnSht2GoLatch', 'btnSetAllInArmZ_Move', 'btnSetAllOutArmZ_Move']);
  var ALL = G2;
  var GROUPS = ['grpRotate_Kit', 'pnlInArmZ', 'pnlOutArmZ', 'grpShtSensor', 'tsRotate']   // golden FormShow's containers of the G2 buttons
    .concat(ROT_GRB.map(function (x) { return 'grb' + x; }));                     // tsRotate = the tab (no id), the rest by id
  var TAB_SEL = { tsRotate: '.tab[title^="tsRotate "],.tab[data-htitle^="tsRotate "]' };   // release theme.js moves title to data-htitle
  var RETRY_MS = 5000, RETRY_MAX = 24;
  var st = { bound: [], ungreyed: [], last: null, lastErr: '', tries: 0, timer: 0, qseq: 0 };

  function byId(id) { return doc.getElementById(id); }
  function groupEl(name) {                          // a group by id; a TTabSheet by its tab header
    if (TAB_SEL[name]) return typeof doc.querySelector === 'function' ? doc.querySelector(TAB_SEL[name]) : null;
    return byId(name);
  }
  // golden TabVisible=false: the header is hidden; when it was the selected tab, VCL selects another visible one
  function setTabShown(tab, on) {
    tab.style.display = on ? '' : 'none';
    if (on || !tab.classList || !tab.classList.contains('act') || !tab.parentElement || !tab.parentElement.querySelector) return;
    var next = tab.parentElement.querySelector(':scope > .tab:not([style*="display: none"])');
    if (next && next !== tab && typeof next.click === 'function') next.click();
  }
  function info(msg, cls) { if (typeof global.teachSetInfo === 'function') global.teachSetInfo(msg, cls); }
  function query(name) {
    var q = String((global.location && global.location.search) || '').replace(/^\?/, '').split('&');
    for (var i = 0; i < q.length; i++) {
      var kv = q[i].split('=');
      if (decodeURIComponent(kv[0] || '') === name) return decodeURIComponent(kv[1] || '').toLowerCase();
    }
    return '';
  }
  function offline() {
    var a = query('offline'), b = query('cppoffline');
    return a === '1' || a === 'true' || a === 'on' || b === '1' || b === 'true' || b === 'on';
  }
  // golden atoi(Edit->Text): the leading integer, else 0 (an empty field = 0)
  function gAtoi(id) {
    var e = byId(id), m = /^\s*([+-]?\d+)/.exec(e ? String(e.value == null ? '' : e.value) : '');
    return m ? parseInt(m[1], 10) : 0;
  }
  function paramsOf(id) {
    if (ROT.indexOf(id) >= 0)                       // all 36: golden reads the field of the selected motor's side (:4643-4650)
      return { backlashIn: gAtoi('edtEditRotateInBacklash'), backlashOut: gAtoi('edtEditRotateOutBacklash') };   // golden :4645 / :4649
    if (id === 'btnSht1GoLatch') return { shtSpeed: gAtoi('EditSh1Speed') };   // golden :4732
    if (id === 'btnSht2GoLatch') return { shtSpeed: gAtoi('EditSh2Speed') };   // golden :4748
    return {};
  }
  function press(id, extra) {
    var A = global.HTMotorAccess;
    if (!A || typeof A.send !== 'function') { info(id + '：motor-access.js 還沒載入，沒有送出', 'err'); return null; }
    var p = paramsOf(id);
    if (extra) for (var k in extra) if (Object.prototype.hasOwnProperty.call(extra, k)) p[k] = extra[k];
    p.btn = id;
    return A.send(CAT_BTN, p);
  }

  function isGrey(el) {
    return !!(el && ((el.classList && el.classList.contains && el.classList.contains('teach-unwired')) || el.getAttribute('data-unwired')));
  }
  function bindOne(id) {
    var b = byId(id);
    if (!b || b.getAttribute('data-st02')) return;
    b.setAttribute('data-st02', 'G2');
    b.setAttribute('data-acc', '1');                 // HW.teach.html teachOn / teachBindTechButtons leave a data-acc button alone
    b.addEventListener('click', function (ev) {
      if (b.__st02WasGrey || isGrey(b)) ev.stopImmediatePropagation();   // the grey's capture listener (registered after this one) must not answer
      press(id);
    }, true);
    st.bound.push(id);
  }
  // the page's TEACH_UNWIRED grey on one of these buttons, undone (no-op on a button that is not greyed)
  function ungrey() {
    for (var i = 0; i < ALL.length; i++) {
      var el = byId(ALL[i]);
      if (!isGrey(el)) continue;
      if (el.classList && el.classList.remove) el.classList.remove('teach-unwired');
      el.removeAttribute('data-unwired');
      el.__st02WasGrey = true;
      if (st.ungreyed.indexOf(ALL[i]) < 0) st.ungreyed.push(ALL[i]);
    }
  }
  function markUnknown(why) {
    for (var i = 0; i < GROUPS.length; i++) {
      var el = groupEl(GROUPS[i]);
      if (!el) continue;
      el.style.outline = '2px dashed #c60';
      el.setAttribute('data-st02-rule', 'unknown: ' + why);
    }
  }
  function applyShown(shown) {
    var known = 0;
    for (var i = 0; i < GROUPS.length; i++) {
      var el = groupEl(GROUPS[i]);
      if (!el) continue;
      var v = shown ? shown[GROUPS[i]] : undefined;
      if (typeof v !== 'boolean') { el.style.outline = '2px dashed #c60'; el.setAttribute('data-st02-rule', 'unknown: no answer for ' + GROUPS[i]); continue; }
      if (TAB_SEL[GROUPS[i]]) setTabShown(el, v);   // golden FormShow's TabVisible (:1668)
      else el.style.display = v ? '' : 'none';     // golden FormShow's Visible
      el.style.outline = '';
      el.setAttribute('data-st02-rule', v ? 'shown' : 'hidden');
      known++;
    }
    return known;
  }
  function retryLater() {
    if (st.timer || st.tries >= RETRY_MAX) return;
    st.timer = global.setTimeout(function () { st.timer = 0; refresh(); }, RETRY_MS);
  }
  function refresh() {
    if (offline()) { markUnknown('offline'); return Promise.resolve(null); }
    var R = global.HT9045Recipe;
    if (!R || typeof R.motorAccess !== 'function') { st.lastErr = 'ht9045_recipe_client.js not loaded'; markUnknown(st.lastErr); retryLater(); return Promise.resolve(null); }
    st.tries++;
    st.qseq++;
    var req = { seq: 900000 + st.qseq, id: 'st02-' + st.qseq, source: 'uteach', button: 'St02Query', action: 'teachSt02',
                kind: 'control', motors: [], params: { btn: 'St02State', query: true }, issuedAt: new Date().toISOString(), state: 'requested' };
    return R.motorAccess(req).then(function (ack) {
      st.last = ack || null;
      st.lastErr = '';
      if (!ack || !ack.shown) { markUnknown('no shown[] in the answer'); retryLater(); return null; }
      applyShown(ack.shown);
      ungrey();
      return ack;
    }, function (e) {
      st.lastErr = (e && e.message) || String(e);
      markUnknown(st.lastErr);
      retryLater();
      return null;
    });
  }

  for (var i = 0; i < ALL.length; i++) bindOne(ALL[i]);
  function afterLoadDom() { global.setTimeout(ungrey, 0); }   // after the page's DOMContentLoaded grey (registered later, runs later)
  if (doc.readyState === 'loading') doc.addEventListener('DOMContentLoaded', afterLoadDom); else afterLoadDom();
  if (global.addEventListener) {
    global.addEventListener('load', function () { ungrey(); refresh(); });
    global.addEventListener('message', function (ev) {   // the frame's HT_WIN (background.html): the window shown again -> ask again
      var d = ev && ev.data;
      if (d && d.type === 'HT_WIN' && d.open) { st.tries = 0; refresh(); }
    });
  }

  global.HT9045TeachSt02 = {
    G2: G2.slice(), ROT: ROT.slice(), GROUPS: GROUPS.slice(), CAT_BTN: CAT_BTN,
    refresh: refresh, ungrey: ungrey, applyShown: applyShown, paramsOf: paramsOf, gAtoi: gAtoi,
    state: function () { return { bound: st.bound.slice(), ungreyed: st.ungreyed.slice(), last: st.last, lastErr: st.lastErr, tries: st.tries,
                                  retrying: !!st.timer }; }
  };
})(window);
