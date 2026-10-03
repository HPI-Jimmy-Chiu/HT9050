/* ht9045_teach_zdown_c.js -- HW.teach.html: golden Set InArm Z / Set OutArm Z / Out Z All Down (INBOX 147).
 * ---------------------------------------------------------------------------
 * AI(W906-TEACH-ZDOWN) 20261003.  Hand-written, not a gen_wire.py product; not named ht9045_wire_<slug>.js for the same reason as
 *   ht9045_teach_zallup_c.js / ht9045_teach_trayz_c.js (the generator would overwrite it).
 *
 *   Hand Pitch > InArm      btnSetAllInArmZ   "Set InArm Z"   golden uteach.cpp:4300 btnSetAllInArmZClick
 *   Hand Pitch > OutArm     btnSetAllOutArmZ  "Set OutArm Z"  golden uteach.cpp:4367 btnSetAllOutArmZClick
 *   Output Arm > Pick Up    btnOutZAllDown    "All Down"      golden uteach.cpp:4542 btnOutZAllDownClick
 *
 * The page sends which button was pressed (HTMotorAccess.send(<button>), catalog JSON/motor-access.json, source uteach) plus the
 *   screen values golden reads with atoi, as params.fields -- only values teachFieldValue() accepts (loaded from C++ or typed,
 *   an integer); a blank / static box is not sent and C++ refuses rather than taking it as 0 (the W5B-4 rule):
 *     Set All    the arm's sixteen Z edits (setEditZ1A..P / setEditZ2A..P) -- C++ uses only the base nozzle's, and only when the
 *                base lies off the nozzle grid (ep1Picker: golden reads setEditZ1E / setEditZ2E's own text, then writes 0 into it);
 *     All Down   SetEditPickOutSht + setEditZ2A..P (golden Pos[i][j] = atoi(SetEditPickOutSht) + atoi(OutZEditPtr[i][j])).
 *   Everything else -- which nozzles, which motors, the 10 % speed, the moves, the refusals (a run, a hand teach, a busy axis:
 *   Set All looks at that arm, All Down at every motor of the machine; a value missing) -- is C++'s, the golden way
 *   (WebMotorAccess.cpp EOF, AI(W906-TEACH-ZDOWN)); the reason of a refusal shows on the status line (motor-access.js).
 * Set All moves nothing: the ack's "edits" are written into the boxes (data-src cpp-pos, as btnSetTo does) and the operator then
 *   presses Save (golden only fills the TEdits; the page's Save writes them).  The ack reaches this file through teachOnAck
 *   (HW.teach.html, one same-line hook next to the Arm Cell's): HT9045TeachZDown.onAck(req, ack, err).
 * No confirmation box (EastSun: a golden button that acts on press acts on press here too).  STOP / closing the window stop the
 *   Zs in C++ (DoStop / the Teach close edge = golden btnStop->Click()).
 * Catalog without these rows (an older motor-access.json) -> greyed with the reason (teachMarkUnwiredEl), never a silent button.
 * ---------------------------------------------------------------------------
 */
(function (global) {
  'use strict';

  var ACTION = { btnSetAllInArmZ: 'teachSetAllArmZ', btnSetAllOutArmZ: 'teachSetAllArmZ', btnOutZAllDown: 'teachOutZAllDown' };
  var LETTERS = 'ACEGIKMOBDFHJLNP';                        // golden teInArm / teOutArm (uteach.cpp:286-290): [0][0..7] then [1][0..7]
  function zEdits(arm) { var a = []; for (var i = 0; i < LETTERS.length; i++) a.push('setEditZ' + arm + LETTERS.charAt(i)); return a; }
  var FIELDS = {
    btnSetAllInArmZ:  zEdits(1),
    btnSetAllOutArmZ: zEdits(2),
    btnOutZAllDown:   ['SetEditPickOutSht'].concat(zEdits(2))
  };
  var WAIT_MS = 30000, POLL_MS = 300;

  function $(id) { return document.getElementById(id); }
  function info(msg, cls) {
    if (typeof global.teachSetInfo === 'function') { try { global.teachSetInfo(msg, cls); return; } catch (e) {} }
    if (global.console) console.warn('[zdown] ' + msg);
  }
  function unwired(id, why) {
    var el = $(id);
    if (!el) return;
    if (typeof global.teachMarkUnwiredEl === 'function') global.teachMarkUnwiredEl(el, why);
    else el.setAttribute('data-unwired', why);
  }

  // the screen values golden reads with atoi -- only the ones the page trusts (HW.teach.html teachFieldValue)
  function fieldsOf(b) {
    var out = {}, f = global.teachFieldValue;
    if (typeof f !== 'function') return out;
    FIELDS[b].forEach(function (id) { var v = f(id); if (v !== null && v !== undefined) out[id] = v; });
    return out;
  }

  var ST = { bound: false, reason: '', missing: [], lastEdits: null };

  function onClick(b) {
    var ma = global.HTMotorAccess;
    if (!ma) { info(b + '：HTMotorAccess 不存在（motor-access.js 沒載入）', 'err'); return; }
    ma.send(b, { fields: fieldsOf(b) });                   // Busy / no catalog row: HTMotorAccess says so on the status line
  }

  // golden: InZEditPtr[i][j]->Text = ... (Set All only fills the boxes)
  function onAck(req, ack, err) {
    if (!req || !ACTION[req.button] || req.action !== 'teachSetAllArmZ' || err || !ack || !ack.edits) return;
    var w = [];
    Object.keys(ack.edits).forEach(function (id) {
      var e = $(id);
      if (!e) return;
      e.value = String(ack.edits[id]);
      e.setAttribute('data-src', 'cpp-pos');                // C++ positions (a later All Down / Go may send them, like btnSetTo's)
      w.push(id + '=' + ack.edits[id]);
    });
    ST.lastEdits = ack.edits;
    info('Set ' + (ack.arm === 'in' ? 'InArm' : 'OutArm') + ' Z：已填 ' + w.length + ' 格（' + w.join(', ') + '）—— 只改畫面，記得按存檔', 'ok');
  }

  function bind(cat) {
    var have = {}, cmds = (cat && cat.commands) || [];
    for (var i = 0; i < cmds.length; i++) {
      var c = cmds[i];
      if (c && c.source === 'uteach' && ACTION[c.button] && c.action === ACTION[c.button]) have[c.button] = true;
    }
    ST.missing = [];
    Object.keys(ACTION).forEach(function (b) {
      var el = $(b);
      if (!el) { ST.missing.push(b); return; }
      if (!have[b]) {
        unwired(b, b + '：指令表 motor-access.json 還沒有教導頁的這一列（uteach ' + b + ' → ' + ACTION[b] + '），這顆還沒接上、按了不會動作');
        return;
      }
      if (el.getAttribute('data-zdown')) return;
      el.setAttribute('data-zdown', '1');
      el.setAttribute('data-acc', '1');                     // the HW.teach.html teachOn mark: wired
      el.addEventListener('click', function () { onClick(b); });
    });
    ST.bound = true;
    document.documentElement.setAttribute('data-teach-zdown', 'ok');
    if (ST.missing.length) info('Set All Z／All Down：頁面上找不到 ' + ST.missing.join(', ') + '（頁面改版？）', 'warn');
  }

  // wait for the page's teachInitAccess (HTMotorAccess initialised with source=uteach and its catalog), then read the same catalog
  function boot() {
    var t0 = Date.now();
    (function poll() {
      var ma = global.HTMotorAccess, d = ma && ma.debug ? ma.debug() : null;
      if (d && d.hasCatalog && d.source === 'uteach') {
        ma.loadJson('../JSON/motor-access.json').then(bind, function (e) {
          ST.reason = (e && e.message) || String(e);
          Object.keys(ACTION).forEach(function (b) { unwired(b, b + '：讀不到指令表 motor-access.json（' + ST.reason + '），這顆沒有接上'); });
        });
        return;
      }
      if (Date.now() - t0 > WAIT_MS) {
        ST.reason = '等了 ' + (WAIT_MS / 1000) + ' 秒，教導頁的馬達指令通道（HTMotorAccess）還沒初始化';
        document.documentElement.setAttribute('data-teach-zdown', 'fail');
        Object.keys(ACTION).forEach(function (b) { unwired(b, b + '：' + ST.reason + '，這顆沒有接上（重新整理頁面再試）'); });
        return;
      }
      setTimeout(poll, POLL_MS);
    })();
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot, { once: true });
  else boot();

  global.HT9045TeachZDown = { ACTION: ACTION, FIELDS: FIELDS, fieldsOf: fieldsOf, onAck: onAck, state: ST };
})(window);
