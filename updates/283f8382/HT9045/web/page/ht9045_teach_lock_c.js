/* ht9045_teach_lock_c.js -- Teach: golden VerifyMotorAction's lock display (labLock / pnlStop / LockAllButton). Hand-written.
 *
 * AI(W906-TEACH-LOCK) 20261002: the every-component check (EastSun 20261001「不是只有檢查按鈕喔 我說的是所有元件」) found
 *   labLock and pnlStop never written on this page.  Golden (uteach.cpp:5346-5405 VerifyMotorAction, every tick while Teach or
 *   Motor Test is shown): any motor moving / homing -> fTeach->LockAllButton(true) (:5321-5329: PageControl2, btnMoveN,
 *   btnMoveP, btnHome, btnMoveTo Enabled=false; pnlStop clYellow) and labLock Visible with "*Lock by Homeing" /
 *   "*Lock by M03 moveing"; nothing moving -> labLock hidden (pnlStop back to (TColor)0x00DFD9CC after the 2 s MoveDelay).
 *   C++ computes it (WebMotorAccessLive.cpp:1271 -> runtime "lock", JsonBridge/ChanMotorPoints.cpp:437-446), the same block
 *   Motor Test draws (HW.MotorTest.html applyLock).
 *   ⚠ Deviation, same as Motor Test (EastSun 20260929 AI(W906-MT-AXISLOCK)「每個軸都是獨立可控的」): the lock follows the
 *   SELECTED axis only (runtime lock.motors) -- another axis moving does not lock this one; PageControl2 (every tab) is never
 *   disabled.  An old C++ without lock.perAxis falls back to golden's whole-page lock (buttons only).
 *   While a motor-access motion command holds its own lock (HTMotorAccess lockAll) the buttons are left to it.
 */
(function () {
  'use strict';
  var BTN = ['btnMoveN', 'btnMoveP', 'btnHome', 'btnMoveTo'];        // golden LockAllButton :5324-5327
  var on = false;
  function $(id) { return document.getElementById(id); }
  function inAccessLock() { var c = window.HTMotorAccess && HTMotorAccess.current && HTMotorAccess.current(); return !!(c && c.kind === 'motion'); }
  function apply(rt, sel) {
    var lk = rt && ((rt.lock && typeof rt.lock === 'object') ? rt.lock : (rt.runtime && rt.runtime.lock));
    if (!lk || typeof lk.locked !== 'boolean') return;                   // old C++: leave FormShow's colour, do not guess
    var text = '';
    if (lk.perAxis === true && Array.isArray(lk.motors)) {
      var hit = null;
      for (var i = 0; i < lk.motors.length; i++) if (lk.motors[i] && lk.motors[i].motorId === sel) { hit = lk.motors[i]; break; }
      on = !!hit;
      if (hit) text = (lk.text && lk.text.indexOf(sel) >= 0) ? lk.text : ('*Lock by ' + sel + ' ' + (hit.why || 'moveing'));
    } else {
      on = lk.locked; text = lk.text || '';
    }
    var p = $('pnlStop');
    if (p) { p.style.backgroundColor = on ? '#ffff00' : '#ccd9df'; p.setAttribute('data-lock', on ? '1' : '0'); }   // :5328
    var l = $('labLock');
    if (l) { l.textContent = text; l.style.display = (on && text) ? '' : 'none'; }                                  // :5399-5404
    if (inAccessLock()) return;
    BTN.forEach(function (id) {
      var b = $(id); if (!b || b.getAttribute('data-unwired')) return;
      if (on) { b.disabled = true; b.setAttribute('data-teach-lock', '1'); b.style.opacity = '0.55'; }
      else if (b.getAttribute('data-teach-lock')) { b.disabled = false; b.removeAttribute('data-teach-lock'); b.style.opacity = ''; }
    });
  }
  window.HT9045TeachLock = { apply: apply, locked: function () { return on; } };
})();
