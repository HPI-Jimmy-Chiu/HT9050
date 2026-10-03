/* ht9045_mt_gearratio.js -- Motor Test: the "Gear Ratio" tab -- measure one axis and save the fitted GearRatio. Hand-written.
 *
 * AI(W906-GEARRATIO) 20261002 [W906]: RULINGS_20261002 #22 (NB2 spec RD5軟體_NB2規格_MotorTest頁GearRatio校正分頁_20261002_213826.md §4,
 *   on origin/v906/nb2-assist). golden has no such tab: a new design the user ruled on (four questions, all "A").
 *   The C++ side (WebMotorAccess.cpp EOF; motor.access gearCalMove / gearRatioPreview / gearRatioSave) does every check, the fit,
 *   the plan, the files and the memory -- this page only chooses, shows and asks.
 * Flow: pick an axis (the page's own selectMotor, so C++'s selectedMotor, the per-axis locks and getMotors agree) -> Start = the
 *   backlash take-up in the measurement direction (gearCalMove begin=true; its target becomes the gauge zero) -> zero the gauge ->
 *   Next point x N forward, then back to 0 (each arrival read from the RUNTIME, never from the ack) -> type each reading (DOUBLE
 *   keypad) -> Preview (old -> new of every value that changes, nothing written) -> Save (C++ re-checks the preview key;
 *   2..10 % asks twice).
 * Readings: "how far from the gauge zero, in the measurement direction" (a positive number); this page signs them with the
 *   direction before sending (measA = dir x reading, measC = dir x distance: the C++ session's own signed cNom).
 * Arrival (spec §4 + the C++ session): runtime motors[m].position.cmdPos === the ack's arriveCmdPos, motion.busy === false,
 *   state.inPos === true, and gearCal.session (same id) active + arrived (C++ GearIntactWhy: READY, fresh sample, command
 *   position on the target, no foreign command), on two consecutive NEW runtime reads after the ack. 60 s without progress
 *   (the runtime cmdPos not changing) = timeout; the axis is not stopped by the page (STOP is right there).
 * Data: HTMtPageApi (HW.MotorTest.html, same-line hook before loadAndBind) -- the page's runtime copy, selectMotor, curMotorId,
 *   setInfo, dbReload, the data source; runtime.gearCal (axes + session + limits, C++). Hooks the page calls: HTMtGear.onAck
 *   (first line of its onAck), HTMtGear.onFinish (its onFinish). Never HTMotorAccess.init (it would replace the page's whole
 *   configuration and ack callback).
 * Lock: while a move of this tab is under way its buttons do nothing (state, not only disabled). STOP is a div (motor-access.js
 *   lockAll disables button / input / select) and always works: it sends the page's btnStop row (motor.stop, no token needed).
 * Greying: class teach-unwired + data-unwired + a capture click that tells why (the Teach page's style); reversible here.
 * Built after DOMContentLoaded + one turn (after ht9045_wire_engine.js attached its keypads and after theme.js moved the titles
 *   in release mode): the inputs here are this file's own divs with their own HTQwerty.show(DOUBLE).
 * After a save: dbReload + HT9045Page.load (spec §4: the Motor Database tab shows the new GearRatio) -- never btnReloadMotorData
 *   (it zeroes every open 1203 axis and clears every HomeFlag). HT9045Page.load is skipped while that grid has unsaved cells.
 */
(function () {
  'use strict';
  var TAG = 'AI(W906-GEARRATIO) 20261002';
  var TAB_TITLE = 'tsGearRatio : TTabSheet';
  var MOVE_TIMEOUT_MS = 60000, READ_MAX_MM = 2000;
  var S = {
    motor: '', mi: -1, dir: 1, placeholder: false, pts: [], next: 0,
    moving: false, target: null, sentAt: 0, progressAt: 0, lastPos: null, good: 0, lastRt: null, reads: 0, readsAtAck: 0,
    sessionId: 0, preview: null, previewKey: '', needConfirm: false, canSave: false,
    winClosed: false, holdSet: false, lastAxesText: '', f2Auto: false,   // AI(W906-GEARRATIO2) 20261003: f2Auto = pickAxis lowered point 2 for a placeholder axis (R171 L1 (a))
    mode: 'move', handRuler: null, servoReq: null, moveEls: []          // AI(W906-GEARHAND) 20261003: the hand-push mode (RULINGS_20261003 #8): 'move' | 'hand', the ruler (mm, signed), this tab's servo request
  };
  var pane = null, tab = null;

  function $(id) { return document.getElementById(id); }
  function api() { return window.HTMtPageApi || null; }
  function isRelease() { return document.documentElement.getAttribute('data-mode') === 'release'; }
  function mk(tag, attrs, text) {
    var e = document.createElement(tag);
    for (var k in attrs) if (attrs.hasOwnProperty(k)) e.setAttribute(k === 'title' && isRelease() ? 'data-htitle' : k, attrs[k]);
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function pos(l, t, w, h) { return 'position:absolute;left:' + l + 'px;top:' + t + 'px;width:' + w + 'px;height:' + h + 'px;'; }
  function grey(el, why) { if (!el) return; el.classList.add('teach-unwired'); el.setAttribute('data-unwired', why); }
  function ungrey(el) { if (!el) return; el.classList.remove('teach-unwired'); el.removeAttribute('data-unwired'); }
  function greyWhy(el) { return el ? el.getAttribute('data-unwired') : null; }
  function status(msg, cls) {
    var e = $('mtGearStatus'); if (!e) return;
    e.textContent = msg;
    e.style.background = cls === 'ok' ? '#0a5' : cls === 'err' ? '#b33' : cls === 'warn' ? '#d9c27a' : 'transparent';
    e.style.color = (cls === 'ok' || cls === 'err') ? '#fff' : 'var(--text,#222)';
    e.setAttribute('data-cls', cls || '');
  }
  function num(id) { var e = $(id); var v = e ? parseFloat(e.textContent) : NaN; return isFinite(v) ? v : NaN; }
  function fmt(v, d) { return (typeof v === 'number' && isFinite(v)) ? v.toFixed(d) : '—'; }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;'); }

  // ---- the tab + pane (the page's binder, HW.MotorTest.html :89-97, only bound the tabs it saw) ----
  function build() {
    var pc = $('PageControl1');
    if (!pc || pane) return;
    var tabs = pc.querySelector(':scope > .pcTabs'), body = pc.querySelector(':scope > .pcBody');
    if (!tabs || !body) return;
    var max = -1;
    Array.prototype.forEach.call(tabs.querySelectorAll(':scope > .tab'), function (t) { var n = parseInt(t.getAttribute('data-t'), 10); if (n > max) max = n; });
    var n = String(max + 1);                                                     // 5 today (spec §4: the 6th tab)
    tab = mk('div', { 'class': 'tab', 'data-t': n, title: TAB_TITLE, id: 'mtGearTab' }, 'Gear Ratio');
    tabs.appendChild(tab);
    pane = mk('div', { 'class': 'pcPane', 'data-p': n, title: 'tsGearRatio', id: 'mtGearPane', style: 'display:none;' });
    body.appendChild(pane);
    tab.addEventListener('click', function () {
      tabs.querySelectorAll(':scope > .tab').forEach(function (x) { x.classList.remove('act'); });
      tab.classList.add('act');
      body.querySelectorAll(':scope > .pcPane').forEach(function (p) { p.style.display = (p.dataset.p === tab.dataset.t) ? 'block' : 'none'; });
      render();
    });
    var css = mk('style', { id: 'mtGearCss' });
    css.textContent = '#mtGearPane .teach-unwired{opacity:.4 !important;filter:grayscale(1);cursor:not-allowed !important;}' +
      '#mtGearPane .mtGearSel{outline:2px solid #06c;outline-offset:-2px;font-weight:bold;}' +
      '#mtGearPane .mtGearAxis{width:118px;height:28px;margin:2px;font-size:11px;position:static;}' +
      '#mtGearPane .mtGearNote{position:absolute;font-size:11px;color:var(--text,#222);white-space:pre-wrap;overflow:auto;}' +
      '#mtGearPane .mtGearNum{position:absolute;font-size:12px;border:1px inset var(--input-border,#aaa);background:var(--input-bg,#fff);' +
      'color:var(--text,#222);padding:2px 4px;box-sizing:border-box;cursor:pointer;text-align:right;}' +
      '#mtGearPane table{border-collapse:collapse;font-size:12px;}#mtGearPane td,#mtGearPane th{border:1px solid #9aa;padding:2px 6px;}' +
      '#mtGearPane .mtGearRead{min-width:110px;cursor:pointer;background:var(--input-bg,#fff);border:1px inset #aaa;padding:1px 4px;text-align:right;}' +
      '#mtGearPane .mtGearStop{position:absolute;display:flex;align-items:center;justify-content:center;background:#c00;color:#fff;' +
      'font-weight:bold;font-size:15px;border:2px outset #e66;cursor:pointer;user-select:none;-webkit-user-select:none;touch-action:manipulation;}';
    document.head.appendChild(css);
    function group(id, legend, l, t, w, h) {
      var g = mk('fieldset', { 'class': 'gbx', id: id, title: id + ' : TGroupBox（' + TAG + '）', style: pos(l, t, w, h) });
      g.appendChild(mk('legend', {}, legend));
      var c = mk('div', { style: 'position:absolute;inset:0;overflow:hidden;' });
      g.appendChild(c); pane.appendChild(g);
      return c;
    }
    function button(c, id, cap, l, t, w, h, fs) {
      var b = mk('button', { 'class': 'btn3d', id: id, title: id + ' : Gear Ratio（' + TAG + '）', style: pos(l, t, w, h) + 'font-size:' + (fs || 12) + 'px;' }, cap);
      c.appendChild(b);
      return b;
    }
    function field(c, id, label, l, t, w, val) {
      c.appendChild(mk('span', { 'class': 'lb', style: pos(l, t + 4, 130, 16) }, label));
      var f = mk('div', { 'class': 'mtGearNum', id: id, title: id + ' : Gear Ratio（' + TAG + '）', style: pos(l + 132, t, w, 24) }, val);
      c.appendChild(f);
      return f;
    }
    var c1 = group('mtGearGrpAxis', 'Axis', 0, 0, 1068, 132);
    c1.appendChild(mk('div', { id: 'mtGearAxes', style: 'position:absolute;left:6px;top:16px;right:6px;height:68px;display:flex;flex-wrap:wrap;align-content:flex-start;overflow:hidden;' }));
    c1.appendChild(mk('div', { 'class': 'mtGearNote', id: 'mtGearAxisInfo', style: pos(8, 86, 800, 42) }));   // AI(W906-GEARHAND) 20261003: 1050 -> 800, the mode switch sits on its right
    button(c1, 'mtGearModeMove', '移動量測', 816, 90, 116, 30);                    // AI(W906-GEARHAND) 20261003: the mode switch (RULINGS_20261003 #8)
    button(c1, 'mtGearModeHand', '手推量測', 938, 90, 116, 30);
    var c2 = group('mtGearGrpMeasure', 'Measure', 0, 136, 650, 470);
    c2.appendChild(mk('span', { 'class': 'lb', style: pos(10, 22, 120, 16) }, '量測方向'));
    button(c2, 'mtGearDirP', '+', 142, 16, 46, 26, 14);
    button(c2, 'mtGearDirN', '−', 192, 16, 46, 26, 14);
    field(c2, 'mtGearSpeed', '速度 %（1..20）', 260, 18, 60, '10');
    field(c2, 'mtGearTakeup', '消背隙 mm（≤ 5）', 10, 52, 80, '2');
    field(c2, 'mtGearF1', '往前點 1 mm', 10, 82, 80, '50');
    field(c2, 'mtGearF2', '往前點 2 mm', 10, 112, 80, '100');
    field(c2, 'mtGearF3', '往前點 3 mm（可空）', 10, 142, 80, '200');
    c2.appendChild(mk('div', { 'class': 'mtGearNote', id: 'mtGearPlanNote', style: pos(240, 50, 400, 120) }));
    button(c2, 'mtGearStart', 'Start（消背隙）', 10, 178, 150, 34);
    button(c2, 'mtGearNext', 'Next point', 168, 178, 130, 34);
    c2.appendChild(mk('div', { 'class': 'mtGearStop', id: 'mtGearStop', title: 'mtGearStop : Gear Ratio STOP（' + TAG + '）', style: pos(310, 178, 120, 34) }, 'STOP'));
    c2.appendChild(mk('div', { id: 'mtGearPoints', style: 'position:absolute;left:10px;top:220px;right:10px;bottom:8px;overflow:auto;' }));
    // AI(W906-GEARHAND) 20261003: the hand-push mode's controls (RULINGS_20261003 #8) -- shown instead of the move mode's above
    S.moveEls = Array.prototype.slice.call(c2.children);
    var hb = mk('div', { id: 'mtGearHandBox', style: 'position:absolute;inset:0;display:none;' });
    c2.appendChild(hb);
    hb.appendChild(mk('div', { 'class': 'mtGearNote', id: 'mtGearHandSteps', style: pos(10, 12, 630, 150) }));
    button(hb, 'mtGearHandStart', '開始（記錄 E0）', 10, 168, 150, 34);
    button(hb, 'mtGearHandSvOff', '伺服 OFF', 168, 168, 110, 34);
    button(hb, 'mtGearHandSvOn', '伺服 ON', 286, 168, 110, 34);
    hb.appendChild(mk('div', { 'class': 'mtGearStop', id: 'mtGearHandStop', title: 'mtGearHandStop : Gear Ratio STOP（' + TAG + '）', style: pos(404, 168, 120, 34) }, 'STOP'));
    field(hb, 'mtGearHandRuler', '尺上量到的距離 mm', 10, 214, 100, '');
    hb.appendChild(mk('div', { 'class': 'mtGearNote', id: 'mtGearHandVals', style: pos(10, 248, 630, 210) }));
    var c3 = group('mtGearGrpResult', 'Result', 656, 136, 412, 470);
    button(c3, 'mtGearPreview', 'Preview', 10, 16, 120, 34);
    button(c3, 'mtGearSave', 'Save', 140, 16, 120, 34);
    c3.appendChild(mk('div', { 'class': 'mtGearNote', id: 'mtGearResult', style: pos(10, 56, 390, 406) }));
    pane.appendChild(mk('div', { id: 'mtGearStatus', title: 'mtGearStatus : Gear Ratio status（' + TAG + '）',
      style: pos(0, 612, 1068, 26) + 'font-size:12px;font-weight:bold;padding:4px 8px;box-sizing:border-box;border-radius:3px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis;' }));
    wire();
    planPoints();
    status('Gear Ratio：先選一軸（灰色的點一下會說原因）。上機時 ES02 或 EastSun 要在機台旁', '');
  }

  // ---- the plan (cumulative from the gauge zero, in the measurement direction): forward f1 < f2 < f3, back through the lower ones to 0 ----
  function fwdList() {
    var f = [num('mtGearF1'), num('mtGearF2'), num('mtGearF3')].filter(function (v) { return isFinite(v) && v > 0; });
    f.sort(function (a, b) { return a - b; });
    var u = []; f.forEach(function (v) { if (!u.length || Math.abs(u[u.length - 1] - v) > 1e-9) u.push(v); });
    return u;
  }
  function planPoints() {
    var u = fwdList(), pts = [];
    u.forEach(function (v) { pts.push({ mag: v, c: S.dir * v, dir: 1, read: null, state: 'todo' }); });
    for (var i = u.length - 2; i >= 0; --i) pts.push({ mag: u[i], c: S.dir * u[i], dir: -1, read: null, state: 'todo' });
    if (u.length) pts.push({ mag: 0, c: 0, dir: -1, read: null, state: 'todo' });
    S.pts = pts; S.next = 0;
    var g = gearBlock(rt()), lim = (g && g.limits) || {};
    var n = $('mtGearPlanNote');
    if (n) n.textContent = '量測點（從量具零點算、往量測方向的累計距離 mm）：' + pts.map(function (p) { return (p.dir > 0 ? '→' : '←') + fmt(p.mag, 2); }).join('　') +
      '\n往前至少 2 點、最長 ≥ 50 mm；各點比值相差 ≤ 0.1 %；往回各點只估背隙，不進齒輪比。' +
      '\n軟體極限還是 ±999999 佔位值時（它們不限制任何東西）：每段 ≤ ' + (lim.stepMaxMm || 50) + ' mm、離開起點（按 Start 時的位置，消背隙也算）≤ ' + (lim.spanMaxMm || 100) + ' mm（C++ 擋）。' +   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a) -- the 100 mm is counted from the start point now (was: from the gauge zero)
      '\n讀數一律填「從量具零點往量測方向走了多少 mm」（正數；量具顯示負號就去掉負號）。';
    renderPoints();
  }
  // "" = the plan fits the rules the C++ will check (so a session is not started only to be refused half-way)
  function planWhy() {
    var u = fwdList(), g = gearBlock(rt()), lim = (g && g.limits) || {};
    var tk = num('mtGearTakeup'), sp = num('mtGearSpeed');
    if (!(isFinite(tk) && tk > 0 && tk <= (lim.takeupMaxMm || 5))) return '消背隙要 0.01 ～ ' + (lim.takeupMaxMm || 5) + ' mm';
    if (!(isFinite(sp) && sp >= 1 && sp <= 20 && Math.floor(sp) === sp)) return '速度要是 1 ～ 20 的整數 %';
    if (u.length < 2) return '往前至少要 2 個點';
    if (u[u.length - 1] < 50) return '最長一段要 ≥ 50 mm（現在 ' + fmt(u[u.length - 1], 2) + ' mm）';
    var bad = u.concat([tk]).filter(function (v) { return Math.abs(Math.round(v * 100) - v * 100) > 1e-6; });
    if (bad.length) return '距離要是 0.01 mm 的整數倍（' + bad.join('、') + '）';
    if (S.placeholder) {
      var step = lim.stepMaxMm || 50, span = lim.spanMaxMm || 100, prev = 0;
      if (Math.round((tk + u[u.length - 1]) * 100) > Math.round(span * 100)) return '這一軸軟體極限是佔位值：離開起點（按 Start 時的位置）最多 ' + span + ' mm —— 消背隙 ' + tk + ' mm＋最遠點 ' + u[u.length - 1] + ' mm＝' + fmt(tk + u[u.length - 1], 2) + ' mm';   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a) -- as C++ counts it: the take-up is part of the travel from the start
      for (var i = 0; i < u.length; i++) { if (u[i] - prev > step) return '這一軸軟體極限是佔位值：每段最多 ' + step + ' mm（' + prev + ' → ' + u[i] + ' mm）'; prev = u[i]; }
    }
    return '';
  }
  function clear(el) { while (el.firstChild) el.removeChild(el.firstChild); }
  function renderPoints() {
    var box = $('mtGearPoints'); if (!box) return;
    clear(box);
    var tb = mk('table', {}), hr = mk('tr', {});
    ['#', '方向', '離量具零點 mm', '狀態', '量具讀數 mm（點一下輸入）', '比值'].forEach(function (h) { hr.appendChild(mk('th', {}, h)); });
    tb.appendChild(hr);
    S.pts.forEach(function (p, i) {
      var st = p.state === 'todo' ? '—' : p.state === 'moving' ? '移動中' : p.state === 'arrived' ? '到位' : p.state;
      var k = (p.read !== null && p.dir > 0 && p.mag > 0) ? (p.read / p.mag).toFixed(6) : '';
      var tr = mk('tr', {});
      [String(i + 1), p.dir > 0 ? '往前' : '往回', fmt(p.mag, 2), st].forEach(function (s) { tr.appendChild(mk('td', {}, s)); });
      var td = mk('td', {});
      var rd = mk('div', { 'class': 'mtGearRead', id: 'mtGearRead' + i, title: 'mtGearRead' + i + ' : Gear Ratio reading（' + TAG + '）' },
                  p.read === null ? (p.state === 'arrived' ? '（輸入）' : '') : fmt(p.read, 3));
      rd.addEventListener('click', function () { readingClick(i, rd); });
      td.appendChild(rd); tr.appendChild(td);
      tr.appendChild(mk('td', {}, k));
      tb.appendChild(tr);
    });
    box.appendChild(tb);
  }

  // ---- runtime ----
  function rt() { var a = api(); return a && a.runtime ? a.runtime() : null; }
  function gearBlock(r) { return (r && r.gearCal && typeof r.gearCal === 'object') ? r.gearCal : null; }
  function motorRt(r, id) {
    var arr = (r && r.motors) || [];
    for (var i = 0; i < arr.length; i++) if (arr[i].motorId === id) return arr[i];
    return null;
  }
  function axisOf(g, id) { var v = (g && g.axes) || []; for (var i = 0; i < v.length; i++) if (v[i].motor === id) return v[i]; return null; }
  function paneWhy(r, g) {
    var a = api();
    if (location.protocol === 'file:') return 'Gear Ratio 需要 wb_serve（C++）：file:// 開的頁面不能量測、也不能存檔';
    if (!a) return 'HW.MotorTest.html 沒有提供 HTMtPageApi（頁面版本不對）';
    if (a.source && a.source() !== 'C++') return '這頁的馬達資料來源是「' + a.source() + '」不是 C++ —— Gear Ratio 只接 C++，不退回靜態快照';
    if (!r) return '還沒讀到 C++ 的 runtime（/api/struct/motor/runtime）';
    if (!g) return '這版 C++ 的 runtime 沒有 gearCal 區塊（舊版 wb_serve）—— Gear Ratio 不能用';
    return '';
  }
  var ALL_IDS = ['mtGearDirP', 'mtGearDirN', 'mtGearStart', 'mtGearNext', 'mtGearPreview', 'mtGearSave', 'mtGearSpeed', 'mtGearTakeup', 'mtGearF1', 'mtGearF2', 'mtGearF3',
                 'mtGearModeMove', 'mtGearModeHand', 'mtGearHandStart', 'mtGearHandSvOff', 'mtGearHandSvOn', 'mtGearHandRuler'];   // AI(W906-GEARHAND) 20261003: + the mode switch and the hand controls
  function render() {
    if (!pane) return;
    var r = rt(), g = gearBlock(r), why = paneWhy(r, g);
    if (why) {
      ALL_IDS.forEach(function (id) { grey($(id), why); });
      var ai = $('mtGearAxisInfo'); if (ai) ai.textContent = why;
      pane.setAttribute('data-grey', why);
      return;
    }
    pane.removeAttribute('data-grey');
    renderAxes(g);
    renderButtons(g);
  }
  // AI(W906-GEARHAND) 20261003: an axis' eligibility in the current mode (the hand mode: C++ handEligible -- no Z, the encoder base is fine)
  function axisOk(x) {
    if (S.mode !== 'hand') return { ok: !!x.eligible, why: x.why || (x.motor + '：不能量測') };
    if (x.handEligible === undefined) return { ok: false, why: '這版 C++ 沒有手推模式（舊版 wb_serve）' };
    return { ok: !!x.handEligible, why: x.handWhy || (x.motor + '：不能用手推模式') };
  }
  function renderAxes(g) {
    var box = $('mtGearAxes'); if (!box) return;
    var txt = JSON.stringify((g.axes || []).map(function (x) { var e = axisOk(x); return [x.motor, e.ok, e.why]; })) + '|' + S.motor + '|' + S.mode;   // AI(W906-GEARHAND) 20261003: + the mode
    if (txt !== S.lastAxesText) {
      S.lastAxesText = txt;
      clear(box);
      (g.axes || []).forEach(function (x) {
        var b = mk('button', { 'class': 'btn3d mtGearAxis', id: 'mtGearAxis_' + x.motor, title: 'mtGearAxis_' + x.motor + ' : Gear Ratio axis（' + TAG + '）' }, x.motor);
        var ok = axisOk(x);                                                      // AI(W906-GEARHAND) 20261003
        if (!ok.ok) grey(b, ok.why);
        if (x.motor === S.motor) b.classList.add('mtGearSel');
        b.addEventListener('click', function (ev) {
          var w = greyWhy(b);
          if (w) { ev.preventDefault(); status(w, 'warn'); return; }
          if (S.moving) { status('移動中不能換軸 —— 先等到位或按 STOP', 'warn'); return; }
          pickAxis(x);
        });
        box.appendChild(b);
      });
    }
    var info = $('mtGearAxisInfo'); if (!info) return;
    var ax = axisOf(g, S.motor), m = S.motor ? motorRt(rt(), S.motor) : null, cur = m && m.cur;
    if (!ax) { info.textContent = '先選一軸。灰色的軸點一下會說原因（旋轉軸、Z 軸還是佔位軟極限、不是 PCI1203、手動教導過……）'; return; }
    var lim = g.limits || {};
    if (S.mode === 'hand') {                                                     // AI(W906-GEARHAND) 20261003
      info.textContent = ax.motor + '（MOT ' + ax.mi + '）　目前 GearRatio ' + (cur && cur.gearRatio != null ? cur.gearRatio : ax.gearRatio) +
        '　編碼器 ' + (m && m.position && m.position.encPos != null ? m.position.encPos : '—') +
        '　伺服 ' + (m && m.state && m.state.servoOn === true ? 'ON' : m && m.state && m.state.servoOn === false ? 'OFF' : '—') +
        '\n手推模式：不下任何移動命令（C++ 只讀編碼器）；Z 軸不能用（伺服一關會往下掉）；HomeFlag 不用是 1，但存檔之後要先重開 wb_serve 再回原點。';
      return;
    }
    info.textContent = ax.motor + '（MOT ' + ax.mi + '）　目前 GearRatio ' + (cur && cur.gearRatio != null ? cur.gearRatio : ax.gearRatio) +
      '　位置 ' + (m && m.position && m.position.cmdPos != null ? m.position.cmdPos : '—') +
      '　HomeFlag ' + (cur && cur.homeFlag != null ? cur.homeFlag : ax.homeFlag) +
      '　軟體極限 ' + (ax.placeholder ? '±999999 佔位值（不限制任何東西）：每段 ≤ ' + (lim.stepMaxMm || 50) + ' mm、離開起點 ≤ ' + (lim.spanMaxMm || 100) + ' mm（消背隙也算）'   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a)
                                       : ax.softN + ' ～ ' + ax.softP) +
      '\n手臂 X／Y 量測前，這支手臂的 Z 要在原點；HomeFlag 要是 1；伺服開、沒有警報、沒有急停（C++ 擋，被拒會寫原因）。';
  }
  function selWhy() {
    var a = api(), cur = a && a.curMotorId ? a.curMotorId() : null;
    if (S.motor && cur !== S.motor) return '主分頁選的軸是 ' + (cur || '（沒有）') + '，不是 ' + S.motor + ' —— 在這裡重新點一次軸';
    return '';
  }
  function set(id, why) { var e = $(id); if (!e) return; if (why) grey(e, why); else ungrey(e); }   // AI(W906-GEARHAND) 20261003: was renderButtons' own (renderHand uses it too)
  function renderButtons(g) {
    var sess0 = g && g.session, modeWhy = (S.moving || (sess0 && sess0.active === true)) ? '量測進行中不能換模式（按 STOP 結束這次量測）' : '';   // AI(W906-GEARHAND) 20261003: the mode switch
    set('mtGearModeMove', modeWhy); set('mtGearModeHand', modeWhy);
    var mm = $('mtGearModeMove'), mh = $('mtGearModeHand');
    if (mm) mm.classList.toggle('mtGearSel', S.mode !== 'hand');
    if (mh) mh.classList.toggle('mtGearSel', S.mode === 'hand');
    showMode();
    if (S.mode === 'hand') { renderHand(g); return; }
    var ax = axisOf(g, S.motor);
    var noAxis = !S.motor ? '先選一軸' : !ax ? S.motor + ' 不在 C++ 的軸清單裡' : (!ax.eligible ? ax.why : '') || selWhy();
    var sess = g && g.session;
    var mine = !!(sess && sess.active && S.sessionId && sess.id === S.sessionId);
    var allRead = S.pts.length > 0 && S.pts.every(function (p) { return p.read !== null; });
    var inSession = mine ? '量測進行中不能改（按 STOP 結束後重新開始）' : '';
    set('mtGearDirP', noAxis || inSession);
    set('mtGearDirN', noAxis || inSession);
    ['mtGearTakeup', 'mtGearF1', 'mtGearF2', 'mtGearF3'].forEach(function (id) { set(id, noAxis || inSession); });
    set('mtGearSpeed', noAxis || (S.moving ? '移動中' : ''));
    set('mtGearStart', noAxis || (S.moving ? '移動中' : '') || ((sess && sess.active === true) ? '量測進行中（' + (sess.motor || '?') + '）—— 先按 STOP 結束這次量測，才能重新按 Start' : '') || planWhy());   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (b) -- Start is closed while a C++ session is active (any session: C++ refuses a begin then), not only while moving
    set('mtGearNext', noAxis || (S.moving ? '移動中' : (!mine ? '先按 Start（消背隙）' : (S.next >= S.pts.length ? '量測點都走完了 —— 按 Preview' :
      (S.next > 0 && S.pts[S.next - 1].read === null ? '先輸入第 ' + S.next + ' 點的量具讀數' : '')))));
    set('mtGearPreview', noAxis || (S.moving ? '移動中' : (!allRead ? '每一點都要有量具讀數' : '')));
    set('mtGearSave', noAxis || (S.moving ? '移動中' : (!S.preview ? '先按 Preview 看過「舊 → 新」' : (!S.canSave ? (S.preview.saveWhy || '預覽說不能存') : ''))));
    var dp = $('mtGearDirP'), dn = $('mtGearDirN');
    if (dp) dp.classList.toggle('mtGearSel', S.dir > 0);
    if (dn) dn.classList.toggle('mtGearSel', S.dir < 0);
  }
  function pickAxis(x) {
    var a = api(); if (!a || !a.selectMotor) { status('頁面沒有 selectMotor', 'err'); return; }
    if (!a.selectMotor(x.mi)) { status(x.motor + '：頁面選不到這一軸（Enable=0？）', 'warn'); return; }
    S.motor = x.motor; S.mi = x.mi; S.placeholder = !!x.placeholder; S.sessionId = 0;
    S.preview = null; S.previewKey = ''; S.canSave = false;
    var f3 = $('mtGearF3'), f2 = $('mtGearF2'), lim0 = (gearBlock(rt()) || {}).limits || {}, cap = (lim0.spanMaxMm || 100) - (lim0.takeupMaxMm || 5);
    if (f3) f3.textContent = S.placeholder ? '' : (f3.textContent || '200');                 // placeholder limits: 50 / 95 (<= 100 mm from the start, the take-up included)
    if (f2 && S.placeholder && !(num('mtGearF2') <= cap)) { f2.textContent = String(cap); S.f2Auto = true; }   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (a) -- 100 mm from the START: the farthest point is 100 - the take-up (<= 5) -> 95 by default
    else if (f2 && !S.placeholder && S.f2Auto && f2.textContent === String(cap)) { f2.textContent = '100'; S.f2Auto = false; }
    planPoints();
    showResult('');
    S.handRuler = null; var hr = $('mtGearHandRuler'); if (hr) hr.textContent = '';   // AI(W906-GEARHAND) 20261003
    status(S.mode === 'hand' ? '選了 ' + x.motor + '（手推）：伺服 ON、把軸停在尺的起點記號，然後按「開始（記錄 E0）」'
                             : '選了 ' + x.motor + '：設定方向、消背隙與往前點，把量具架好，然後按 Start', '');
    render();
  }

  // ---- arrival: the runtime, two fresh reads after the ack ----
  function tick() {
    if (!pane || S.winClosed) return;
    var r = rt();
    if (r && r !== S.lastRt) { S.lastRt = r; S.reads++; }
    else { render(); if (S.moving) timeoutCheck(null); return; }
    render();
    var g = gearBlock(r), sess = g && g.session, since = S.reads - S.readsAtAck;
    if (!S.moving) {
      // AI(W906-GEARHAND) 20261003: + since >= 2 -- a hand session is idle right after its ack (a move session only after its arrival,
      //   which already needs two reads), so a runtime read fetched before the session existed must not end it
      if (S.sessionId && since >= 2 && (!sess || sess.id !== S.sessionId || sess.active !== true)) {           // idle with a session = arrived = >= 2 reads after its ack
        var w = (sess && sess.id === S.sessionId) ? (sess.why || '') : '有別的量測開始了';
        S.sessionId = 0;
        if (S.mode === 'hand') {                                                 // AI(W906-GEARHAND) 20261003: spec (10) -- C++'s why says when the servo is still OFF
          S.handRuler = null; S.preview = null; S.previewKey = ''; S.canSave = false;
          status('手推量測已結束：' + w + ' —— 要量請重新按「開始（記錄 E0）」', /伺服還是 OFF/.test(w) ? 'err' : 'warn');
        } else status('量測已結束：' + w + ' —— 要量請重新按 Start', 'warn');
        render();
      }
      return;
    }
    if (S.target === null) { timeoutCheck(null); return; }                      // the ack has not come back yet
    var m = motorRt(r, S.motor), p = m && m.position, mo = m && m.motion, st = m && m.state;
    if (p && p.cmdPos !== S.lastPos) { S.lastPos = p.cmdPos; S.progressAt = Date.now(); }
    if (since >= 2 && S.sessionId && (!sess || sess.id !== S.sessionId || sess.active !== true)) {
      aborted('量測被中止：' + ((sess && sess.id === S.sessionId && sess.why) || '有別的量測開始了'));
      return;
    }
    var ok = !!(p && mo && st && sess) && p.cmdPos === S.target && mo.busy === false && st.inPos === true &&
             sess.id === S.sessionId && sess.active === true && sess.arrived === true;
    if (ok) { if (++S.good >= 2) arrived(); }
    else { S.good = 0; timeoutCheck(m); }
  }
  function timeoutCheck(m) {
    if (Date.now() - Math.max(S.sentAt, S.progressAt) <= MOVE_TIMEOUT_MS) return;
    var p = m && m.position, mo = m && m.motion, st = m && m.state, g = gearBlock(rt()), sess = g && g.session;
    aborted('到位逾時（60 秒沒有進展）：cmdPos=' + (p ? p.cmdPos : '?') + '（要 ' + S.target + '）、busy=' + (mo ? mo.busy : '?') + '、inPos=' + (st ? st.inPos : '?') +
            (sess && sess.intactWhy ? '、C++：' + sess.intactWhy : '') + ' —— 按 STOP 後檢查；INP 一直不亮請 EastSun 看驅動器的到位設定');
  }
  function arrived() {
    S.moving = false; S.good = 0;
    var i = S.next - 1;
    if (i < 0) status('消背隙到位：請把量具歸零，然後按 Next point', 'ok');
    else { S.pts[i].state = 'arrived'; status('第 ' + (i + 1) + ' 點到位（離零點 ' + fmt(S.pts[i].mag, 2) + ' mm）：點表格輸入量具讀數', 'ok'); }
    renderPoints(); render();
  }
  function aborted(why) {
    S.moving = false; S.good = 0;
    if (S.next > 0 && S.pts[S.next - 1] && S.pts[S.next - 1].state === 'moving') S.pts[S.next - 1].state = '中止';
    status(why, 'err'); renderPoints(); render();
  }

  // ---- AI(W906-GEARHAND) 20261003: the hand-push mode (RULINGS_20261003 #8 = NIGHT_REPORT s0 #81 A, INBOX 154) ----
  //   Jimmy: "we put a ruler under the axis, servo off and push it by hand to a mark, servo on and read the encoder". The tab never
  //   asks C++ to move anything in this mode: gearHandBegin (C++ takes E0 from the encoder), then the page's own servo row
  //   btnServoOff with servoOn false / true (the existing path: C++ holds a brake 100 ms before a servo off), C++ takes E1 on a
  //   settled sample after the servo is back on, the operator types the ruler distance, and Preview / Save send {hand:true, rulerMm}
  //   to the same gearRatioPreview / gearRatioSave. C++ (WebMotorAccess.cpp HAND-PUSH block) does every check; this page shows the
  //   steps and the C++ session (runtime gearCal.session: mode hand, phase 0..3, e0Card / e1Card, servoOn).
  var HAND_STEPS = [
    '① 選一個直線軸（Z 軸不能用：伺服一關會往下掉）；伺服 ON，把軸停在尺的起點記號（可以用主分頁的 JOG 對準）',
    '② 按「開始（記錄 E0）」：C++ 記下編碼器 E0（這個模式不下任何移動命令）',
    '③ 按「伺服 OFF」，用手把軸推到尺上的終點記號（推之前確認這支手臂的 Z 在上面、路上沒有東西）',
    '④ 推好了：手離開軸、站開，再按「伺服 ON」：C++ 等軸停穩（連續兩次讀到一樣的位置）才記下 E1',   // AI(W906-GEARHAND) 20261003: NB2-1 (g) L1 (St01 ST01-E2): the hands-off line before the servo comes back on
    '⑤ 輸入尺上量到的距離（mm，到 0.01；往負方向推就輸入負數）',
    '⑥ Preview 看「舊 → 新」，再 Save；存檔之後要先重開 wb_serve，才能移動這一軸'
  ];
  function setMode(m) {
    if (m === S.mode) return;
    var sess = (gearBlock(rt()) || {}).session;
    if (S.moving || (sess && sess.active === true)) { status('量測進行中不能換模式 —— 先按 STOP 結束這次量測', 'warn'); return; }
    S.mode = m; S.sessionId = 0; S.preview = null; S.previewKey = ''; S.canSave = false; S.handRuler = null; S.lastAxesText = '';
    var hr = $('mtGearHandRuler'); if (hr) hr.textContent = '';
    showResult('');
    status(m === 'hand' ? '手推量測：選直線軸 → 開始（記錄 E0）→ 伺服 OFF → 用手推到尺的記號 → 伺服 ON → 輸入尺的距離 → Preview → Save（不下任何移動命令）'
                        : '移動量測：選軸 → 設定方向、消背隙與往前點 → Start', '');
    render();
  }
  function showMode() {
    var hand = S.mode === 'hand', hb = $('mtGearHandBox');
    if (hb) hb.style.display = hand ? 'block' : 'none';
    S.moveEls.forEach(function (e) { e.style.display = hand ? 'none' : ''; });
  }
  function handSession(g) {
    var sess = g && g.session;
    return (sess && sess.active === true && sess.mode === 'hand' && S.sessionId && sess.id === S.sessionId) ? sess : null;
  }
  function renderHand(g) {
    var ax = axisOf(g, S.motor), sess = g && g.session, mine = handSession(g);
    var noAxis = !S.motor ? '先選一軸' : !ax ? S.motor + ' 不在 C++ 的軸清單裡' : (!axisOk(ax).ok ? axisOk(ax).why : '') || selWhy();
    var m = S.motor ? motorRt(rt(), S.motor) : null, sv = m && m.state ? m.state.servoOn : null, busy = m && m.motion ? m.motion.busy : null;
    var svNow = mine && typeof mine.servoOn === 'boolean' ? mine.servoOn : sv;
    var notMine = '先按「開始（記錄 E0）」';
    set('mtGearHandStart', noAxis || ((sess && sess.active === true) ? '量測進行中（' + (sess.motor || '?') + '）—— 先按 STOP 結束這次量測，才能重新開始' : '') ||
                           (sv !== true ? '伺服沒開：先開伺服、把軸停在尺的起點記號（可以用主分頁的 JOG 對準）' : '') || (busy === true ? '軸在動 —— 等它停好' : ''));
    set('mtGearHandSvOff', noAxis || (!mine ? notMine : (svNow === false ? '伺服已經是 OFF：推好了，手離開軸、站開，再按「伺服 ON」' : '')));
    set('mtGearHandSvOn', noAxis || (!mine ? notMine : (svNow !== false ? '伺服是 ON（要推，先按「伺服 OFF」）' : '')));
    set('mtGearHandRuler', noAxis || (!mine ? notMine : (mine.phase !== 3 ? '等 E1：伺服 OFF → 推 → 伺服 ON，C++ 讀到停穩的位置才記' : '')));
    set('mtGearPreview', noAxis || (!mine ? notMine : (mine.phase !== 3 ? '還沒有 E1（伺服 OFF → 推 → 伺服 ON）' : (S.handRuler === null ? '先輸入尺上量到的距離' : ''))));
    set('mtGearSave', noAxis || (!S.preview ? '先按 Preview 看過「舊 → 新」' : (!S.canSave ? (S.preview.saveWhy || '預覽說不能存') : '')));
    var cur = !S.motor ? 0 : !mine ? 1 : mine.phase === 0 ? 2 : (mine.phase === 1 || mine.phase === 2) ? 3 : (S.handRuler === null ? 4 : 5);
    var st = $('mtGearHandSteps');
    if (st) st.textContent = '手推量測（尺＋編碼器；這個模式不下任何移動命令）：\n' + HAND_STEPS.map(function (s, i) { return (i === cur ? '▶ ' : '　 ') + s; }).join('\n') +
                             '\n（STOP、換軸、關頁、斷線、警報都會結束這次量測；伺服維持你設的狀態）';
    var vals = $('mtGearHandVals');
    if (!vals) return;
    if (!mine) { vals.textContent = 'E0 —　E1 —' + (sess && sess.active === true && sess.mode !== 'hand' ? '\n（進行中的是移動量測：先按 STOP 結束）' : ''); return; }
    var ph = ['已記 E0：按「伺服 OFF」', '伺服 OFF：推到尺上的終點記號，手離開軸、站開，再按「伺服 ON」', '伺服 ON：等 C++ 讀到停穩的位置', '已記 E1：輸入尺的距離，按 Preview'][mine.phase] || String(mine.phase);
    var L = [];
    L.push('E0（開始時的編碼器）：' + fmt(mine.e0Card, 0) + ' 脈波');
    L.push('E1（伺服 ON 之後）：' + (typeof mine.e1Card === 'number' ? fmt(mine.e1Card, 0) + ' 脈波' : '—'));
    L.push('E1 − E0：' + (typeof mine.deltaCard === 'number' ? fmt(mine.deltaCard, 0) + ' 脈波 ＝ ' + fmt(mine.deltaMm, 3) + ' mm（照目前的齒輪比）' : '—'));
    L.push('伺服：' + (svNow === true ? 'ON' : svNow === false ? 'OFF（可以推）' : '讀不到'));
    L.push('狀態：' + ph + (mine.intactWhy ? '\nC++：' + mine.intactWhy : ''));
    if (S.handRuler !== null) L.push('尺：' + fmt(S.handRuler, 2) + ' mm' + (typeof mine.deltaMm === 'number' && mine.deltaMm ? '（尺 ÷ 編碼器 = ' + (S.handRuler / mine.deltaMm).toFixed(6) + '）' : ''));
    vals.textContent = L.join('\n');
  }
  function handStart() {
    var sess = (gearBlock(rt()) || {}).session;
    if (sess && sess.active === true) { status('量測進行中 —— 先按 STOP 結束這次量測，才能重新開始', 'warn'); return; }
    holdToken();
    S.sessionId = 0; S.preview = null; S.previewKey = ''; S.canSave = false; S.handRuler = null; showResult('');
    var hr = $('mtGearHandRuler'); if (hr) hr.textContent = '';
    if (send('mtGearHandStart', {})) status('手推量測：送出「開始」—— 等 C++ 記下 E0', 'warn');
    render();
  }
  function handServo(on) {
    if (!window.HTMotorAccess) { status('motor-access.js 沒有載入', 'err'); return; }
    var req = HTMotorAccess.send('btnServoOff', { servoOn: !!on }, [S.motor]);     // the page's own servo row (servoToggle): the brake rule is C++'s
    if (!req) { status('忙碌中：上一個命令還沒回覆 —— 稍候再按', 'warn'); return; }
    S.servoReq = req;
    status(on ? '伺服 ON 已送出：C++ 等軸停穩後記下 E1' : '伺服 OFF 已送出：用手把軸推到尺上的終點記號，推好了手離開軸、站開，再按「伺服 ON」', 'warn');
    render();
  }

  // ---- buttons ----
  function wire() {
    function on(id, fn) {
      var e = $(id); if (!e) return;
      e.addEventListener('click', function (ev) {
        var w = greyWhy(e);
        if (w) { ev.preventDefault(); ev.stopImmediatePropagation(); status(w, 'warn'); return; }
        fn(ev);
      }, true);
    }
    on('mtGearDirP', function () { S.dir = 1; planPoints(); render(); });
    on('mtGearDirN', function () { S.dir = -1; planPoints(); render(); });
    ['mtGearSpeed', 'mtGearTakeup', 'mtGearF1', 'mtGearF2', 'mtGearF3'].forEach(function (id) {
      on(id, function () {
        var el = $(id), before = el.textContent;
        if (!window.HTQwerty) { status('沒有小鍵盤（qwerty.js）', 'err'); return; }
        var spd = id === 'mtGearSpeed';
        HTQwerty.show(el, spd ? HTQwerty.N.INTEGER : HTQwerty.N.DOUBLE, { dp: spd ? 0 : 2, checkRange: false, onCommit: function (v) {
          var x = parseFloat(v);
          if (String(v).trim() === '' && id === 'mtGearF3') el.textContent = '';                   // point 3 may be left empty (2 forward points)
          else if (!isFinite(x)) { el.textContent = before; status('不是數字：' + v, 'warn'); }
          else el.textContent = String(x);
          if (!spd) planPoints();
          render();
        } });
      });
    });
    on('mtGearStart', start);
    on('mtGearNext', nextPoint);
    on('mtGearPreview', preview);
    on('mtGearSave', save);
    function stopNow() {
      if (!window.HTMotorAccess) { status('STOP：motor-access.js 沒有載入 —— 請按主分頁的 STOP', 'err'); return; }
      HTMotorAccess.send('btnStop', {}, S.motor ? [S.motor] : null);         // motor.stop -> C++ stops the axes; the session ends (CancelAllJobs)
      S.moving = false; S.good = 0; S.target = null; S.sessionId = 0;
      S.pts.forEach(function (p) { if (p.state === 'moving') p.state = '中止'; });
      if (S.mode === 'hand') {                                                   // AI(W906-GEARHAND) 20261003: spec (10) -- the servo stays as it is
        var m = S.motor ? motorRt(rt(), S.motor) : null, off = !!(m && m.state && m.state.servoOn === false);
        S.handRuler = null; S.preview = null; S.previewKey = ''; S.canSave = false;
        status('STOP 已送出：手推量測結束（C++ 不動伺服）' + (off ? '；⚠ 這一軸伺服還是 OFF —— 要用這一軸先按「伺服 ON」' : '') + '；要重新量請按「開始（記錄 E0）」', off ? 'err' : 'warn');
      } else status('STOP 已送出：量測中止（C++ 結束量測）；要重新量請按 Start', 'warn');
      renderPoints(); render();
    }
    $('mtGearStop').addEventListener('click', stopNow);
    $('mtGearHandStop').addEventListener('click', stopNow);                     // AI(W906-GEARHAND) 20261003: the hand box's own STOP (a div too: lockAll cannot disable it)
    // AI(W906-GEARHAND) 20261003: the mode switch and the hand-push controls (RULINGS_20261003 #8)
    on('mtGearModeMove', function () { setMode('move'); });
    on('mtGearModeHand', function () { setMode('hand'); });
    on('mtGearHandStart', handStart);
    on('mtGearHandSvOff', function () { handServo(false); });
    on('mtGearHandSvOn', function () { handServo(true); });
    on('mtGearHandRuler', function () {
      var el = $('mtGearHandRuler'), before = el.textContent;
      if (!window.HTQwerty) { status('沒有小鍵盤（qwerty.js）', 'err'); return; }
      var lim = (gearBlock(rt()) || {}).limits || {}, maxMm = lim.handRulerMaxMm || 2000;
      HTQwerty.show(el, HTQwerty.N.DOUBLE, { dp: 2, checkRange: false, onCommit: function (v) {
        var s = String(v).trim(), x = parseFloat(s);
        if (s === '' || !isFinite(x)) { el.textContent = ''; S.handRuler = null; }   // empty = no ruler distance (never 0)
        else if (x === 0 || Math.abs(x) > maxMm) { el.textContent = before; status('尺的距離 ' + s + ' mm 不合理（0 或 > ' + maxMm + ' mm）—— 沒有採用', 'warn'); }
        else if (Math.abs(Math.round(x * 100) - x * 100) > 1e-6) { el.textContent = before; status('尺的距離要到 0.01 mm（' + s + '）—— 沒有採用', 'warn'); }
        else { el.textContent = String(x); S.handRuler = x; }
        S.preview = null; S.previewKey = ''; S.canSave = false;
        render();
      } });
    });
    window.addEventListener('message', function (ev) {                          // background.html opens / closes the window (HT_WIN)
      var d = ev.data;
      if (!d || d.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
      S.winClosed = !d.open;
      if (!d.open) { S.moving = false; S.good = 0; S.target = null; S.sessionId = 0; }   // the page's formClose ends the C++ session
    });
  }
  function holdToken() {
    if (S.holdSet || !window.HT9045Recipe || !HT9045Recipe.setTokenHold) return;
    S.holdSet = true;
    HT9045Recipe.setTokenHold(function () { return !S.winClosed && (S.moving || !!S.sessionId); });   // an idle token release would end the session
    setInterval(function () {
      if (!S.winClosed && (S.moving || S.sessionId) && HT9045Recipe.keepAlive) HT9045Recipe.keepAlive().catch(function () {});
    }, 60000);
  }
  function send(button, params) {
    if (!window.HTMotorAccess) { status('motor-access.js 沒有載入', 'err'); return null; }
    var req = HTMotorAccess.send(button, params, [S.motor]);
    if (!req) status('忙碌中：上一個命令還沒回覆（或指令表還沒載入）—— 稍候再按', 'warn');
    return req;
  }
  function move(d, begin) {
    S.moving = true; S.good = 0; S.target = null; S.sentAt = Date.now(); S.progressAt = 0; S.lastPos = null;
    var req = send('mtGearMove', { distanceMm: Math.round(d * 100) / 100, speedPct: Math.round(num('mtGearSpeed')), begin: !!begin });
    if (!req) { S.moving = false; return false; }
    return true;
  }
  function start() {
    var w = planWhy(); if (w) { status(w, 'warn'); return; }
    var s0 = (gearBlock(rt()) || {}).session; if (S.moving || (s0 && s0.active === true)) { status('量測進行中 —— 先按 STOP 結束這次量測，才能重新按 Start', 'warn'); return; }   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L1 (b) -- also refused by C++
    planPoints();
    holdToken();
    S.sessionId = 0; S.preview = null; S.previewKey = ''; S.canSave = false; showResult('');
    var tk = S.dir * num('mtGearTakeup');
    if (move(tk, true)) status('送出消背隙 ' + fmt(tk, 2) + ' mm —— 等 C++ 受理', 'warn');
    render();
  }
  function nextPoint() {
    if (S.next >= S.pts.length) return;
    var prev = S.next === 0 ? 0 : S.pts[S.next - 1].c, p = S.pts[S.next], d = p.c - prev;
    S.next++;
    p.state = 'moving';
    renderPoints();
    if (!move(d, false)) { S.next--; p.state = 'todo'; renderPoints(); render(); return; }
    status('第 ' + S.next + ' 點：送出 ' + fmt(d, 2) + ' mm（到離零點 ' + fmt(p.mag, 2) + ' mm）', 'warn');
    render();
  }
  function readingClick(i, el) {
    var p = S.pts[i];
    if (!p || p.state !== 'arrived' || S.moving) { status('這一點還沒到位', 'warn'); return; }
    if (!window.HTQwerty) { status('沒有小鍵盤（qwerty.js）', 'err'); return; }
    el.textContent = p.read === null ? '' : String(p.read);
    HTQwerty.show(el, HTQwerty.N.DOUBLE, { dp: 3, checkRange: false, onCommit: function (v) {
      var s = String(v).trim(), x = parseFloat(s);
      if (s === '' || !isFinite(x)) p.read = null;                                // an empty commit is "no reading", never 0
      else if (Math.abs(x) > READ_MAX_MM) status('讀數 ' + s + ' mm 不合理（> ' + READ_MAX_MM + ' mm）—— 沒有採用', 'warn');
      else p.read = x;
      S.preview = null; S.previewKey = ''; S.canSave = false;
      renderPoints(); render();
    }, onAbort: function () { renderPoints(); } });
  }
  function measParams() {
    var r = rt(), m = motorRt(r, S.motor), ax = axisOf(gearBlock(r), S.motor);
    var old = (m && m.cur && typeof m.cur.gearRatio === 'number') ? m.cur.gearRatio : (ax ? ax.gearRatio : null);
    if (S.mode === 'hand') return { oldRatio: old, hand: true, rulerMm: S.handRuler };   // AI(W906-GEARHAND) 20261003: C++ takes E0 / E1 from its own session
    return { oldRatio: old, measC: S.pts.map(function (p) { return p.c; }), measA: S.pts.map(function (p) { return S.dir * p.read; }),
             measDir: S.pts.map(function (p) { return p.dir; }) };
  }
  function preview() {
    var q = measParams();
    if (typeof q.oldRatio !== 'number') { status('讀不到目前的齒輪比（runtime cur.gearRatio）', 'err'); return; }
    if (send('mtGearPreview', q)) status('預覽中（C++ 重算，不寫檔）……', 'warn');
  }
  function save() {
    if (!S.preview || !S.previewKey) { status('先按 Preview', 'warn'); return; }
    var p = S.preview, f = p.fit || {}, pl = p.plan || {};
    var msg = S.motor + '：齒輪比 ' + f.oldRatio + ' → ' + f.newRatioText + '（差 ' + fmt(f.deviationPct, 3) + ' %）\n' +
              '教導點 ' + (pl.changedCount || 0) + ' 個等比例換算；' + softText(pl) + '；Mot_Table.csv 與 teach.ini 先備份再寫。\n' +   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L5 -- it said "軟體極限一起等比例換算" also when both are the +-999999 placeholders (not rescaled)
              (S.mode === 'hand' ? '這是手推量測（伺服關過）：存檔之後，要移動這一軸之前先重開 wb_serve，再回原點（HOME）。\n\n確定要存檔？'   // AI(W906-GEARHAND) 20261003: spec (9)
                                 : '存檔之後這一軸 HomeFlag=0，要重新回原點。\n\n確定要存檔？');
    if (!window.confirm(msg)) return;
    var q = measParams();
    q.previewKey = S.previewKey;
    if (S.needConfirm) {
      if (!window.confirm('再確認一次：' + (f.why || '') + '\n\n真的要存？')) return;
      q.confirmLarge = true;
    }
    if (send('mtGearSave', q)) status('存檔中……', 'warn');
  }
  // AI(W906-GEARRATIO2) 20261003: NB2 R171 L5 -- what the save does to the soft limits, from the preview's plan.softP / softN
  //   (C++ GearPlanBuild: a +-999999 placeholder is never rescaled)
  function softText(pl) {
    var p = pl && pl.softP, n = pl && pl.softN;
    if (!p || !n) return '軟體極限：預覽沒有帶回（不知道會不會換算）';
    var re = [], ph = [];
    (p.placeholder ? ph : re).push('P ' + p.old + (p.placeholder ? '' : ' → ' + p['new']));
    (n.placeholder ? ph : re).push('N ' + n.old + (n.placeholder ? '' : ' → ' + n['new']));
    if (!re.length) return '軟體極限兩個都是 ±999999 佔位值：不換算（維持 ' + p.old + '／' + n.old + '）';
    if (!ph.length) return '軟體極限一起等比例換算（' + re.join('、') + '）';
    return '軟體極限只換算 ' + re.join('、') + '；' + ph.join('、') + ' 是 ±999999 佔位值，不換算';
  }
  function showResult(t) { var e = $('mtGearResult'); if (e) e.textContent = t; }
  function resultText(ack) {
    var f = ack.fit || {}, p = ack.plan || {}, L = [];
    if (ack.mode === 'hand' && ack.hand) L.push('手推：E0 ' + fmt(ack.hand.e0Card, 0) + '、E1 ' + fmt(ack.hand.e1Card, 0) + ' 脈波（差 ' + fmt(ack.hand.deltaCard, 0) + ' ＝ ' +
                                                fmt(ack.hand.encMm, 3) + ' mm 照目前齒輪比）；尺 ' + fmt(ack.hand.rulerMm, 2) + ' mm');   // AI(W906-GEARHAND) 20261003
    L.push('k = ' + (typeof f.k === 'number' ? f.k.toFixed(7) : '—') + '　新齒輪比 ' + (f.newRatioText || '—') + '（舊 ' + f.oldRatio + '）　差 ' + fmt(f.deviationPct, 3) + ' %');
    L.push('一致性 ' + fmt(f.consistencyPct, 3) + ' %　往前 ' + f.nForward + ' 點、往回 ' + f.nBack + ' 點、最長 ' + fmt(f.maxSpanMm, 2) + ' mm　背隙 ' +
           (typeof f.backlashMm === 'number' ? fmt(f.backlashMm, 3) + ' mm' : '—'));
    if (f.why) L.push((f.level === 'confirm' ? '⚠ ' : '✗ ') + f.why);
    if (p.softP) L.push('軟體極限 P ' + p.softP.old + ' → ' + p.softP['new'] + (p.softP.placeholder ? '（佔位，不換算）' : '') +
                        '；N ' + p.softN.old + ' → ' + p.softN['new'] + (p.softN.placeholder ? '（佔位，不換算）' : ''));
    (p.teach || []).forEach(function (c) { L.push('  ' + c.where + '：' + c.old + ' → ' + c['new'] + (c.slots && c.slots.length > 1 ? '（同一個變數 ' + c.slots.length + ' 處）' : '')); });
    if (p.manual && p.manual.length) { L.push('不自動換算，請人工確認：'); p.manual.forEach(function (m) { L.push('  ' + m.where + (m.value != null ? ' = ' + m.value : '') + '：' + m.why); }); }
    if (p.zeroSkipped) L.push('（另有 ' + p.zeroSkipped + ' 個值是 0 的教導值沒列出）');
    (ack.notes || []).forEach(function (n) { L.push('• ' + n); });
    if (p.why) L.push('✗ ' + p.why);
    if (ack.sessionWhy) L.push('量測：' + ack.sessionWhy);
    if (ack.saveWhy) L.push('不能存：' + ack.saveWhy);
    return L.join('\n');
  }
  // HW.MotorTest.html's onAck hands every reply here first (same-line hook); true = this file's (the page does nothing more)
  function onAck(req, ack, err) {
    // AI(W906-GEARHAND) 20261003: + the hand session's begin, and the servo requests this tab sent (S.servoReq; the page's own Servo button stays the page's)
    if (req && req === S.servoReq && req.action === 'servoToggle') {
      S.servoReq = null;
      var on = !!(req.params && req.params.servoOn);
      if (!ack) status('伺服' + (on ? ' ON' : ' OFF') + ' 被拒（C++）：' + ((err && err.message) || '?'), 'err');
      else status(on ? '伺服 ON：C++ 等軸停穩後記下 E1' : '伺服 OFF：用手把軸推到尺上的終點記號，推好了手離開軸、站開，再按「伺服 ON」', 'ok');
      render();
      return true;
    }
    if (req && req.action === 'gearHandBegin') {
      if (!ack) { S.sessionId = 0; status('C++ 拒絕開始手推量測（什麼都沒動）：' + ((err && err.message) || '?'), 'err'); render(); return true; }
      if (ack.session && ack.session.id) S.sessionId = ack.session.id;
      S.readsAtAck = S.reads;
      status('已記下 E0 = ' + fmt(ack.e0Card, 0) + ' 脈波 —— 下一步：按「伺服 OFF」，用手把軸推到尺上的終點記號，再按「伺服 ON」', 'ok');
      render();
      return true;
    }
    if (!req || (req.action !== 'gearCalMove' && req.action !== 'gearRatioPreview' && req.action !== 'gearRatioSave')) return false;
    if (req.action === 'gearCalMove') {
      if (!ack) {
        S.moving = false; S.target = null;
        if (S.next > 0 && S.pts[S.next - 1] && S.pts[S.next - 1].state === 'moving') { S.pts[S.next - 1].state = 'todo'; S.next--; }
        status('C++ 拒絕移動（什麼都沒動）：' + ((err && err.message) || '?'), 'err'); renderPoints(); render();
        return true;
      }
      S.target = ack.arriveCmdPos; S.readsAtAck = S.reads; S.progressAt = Date.now();
      if (ack.session && ack.session.id) S.sessionId = ack.session.id;
      status('受理（還沒到位，看 runtime）：目標 ' + ack.target + '，到位時 runtime cmdPos = ' + ack.arriveCmdPos, 'warn');
      return true;
    }
    if (req.action === 'gearRatioPreview') {
      if (!ack) { S.preview = null; S.previewKey = ''; S.canSave = false; status('預覽被拒：' + ((err && err.message) || '?'), 'err'); showResult(''); render(); return true; }
      S.preview = ack; S.previewKey = (ack.plan && ack.plan.previewKey) || ''; S.canSave = !!ack.canSave; S.needConfirm = !!ack.needConfirm;
      showResult(resultText(ack));
      status(ack.canSave ? '預覽好了（沒有寫檔）：看過「舊 → 新」再按 Save' : '預覽好了，但不能存：' + (ack.saveWhy || ''), ack.canSave ? 'ok' : 'warn');
      render();
      return true;
    }
    if (!ack) {
      var em = (err && err.message) || '?';
      // AI(W906-GEARRATIO2) 20261003: NB2 R171 M2 follow-up -- a teach.ini this version's Teach page never saved: C++ refused and
      //   restored everything (kGearTeachNotNormalized); say what to do, the full detail list goes to the result pane
      if (/還沒有用目前版本的教導頁存過檔/.test(em)) {
        status('沒有存檔（已全部還原，什麼都沒改）：teach.ini 還沒有用目前版本的教導頁存過檔 —— 請先在教導頁按一次「存檔」、關掉教導頁，再回來重新 Preview、Save', 'warn');
        showResult(em);
      } else status('存檔被拒（C++ 沒寫，或寫了又還原）：' + em, 'err');
      render(); return true;
    }
    S.preview = null; S.previewKey = ''; S.canSave = false; S.sessionId = 0;
    showResult((ack.message || '') + '\n\n' + resultText(ack) + '\n\n' + (ack.next || ''));
    var a = api(), extra = '';
    if (a && a.dbReload) a.dbReload();                                           // the page's own reread (never btnReloadMotorData)
    if (window.HT9045Page && HT9045Page.load) {
      if (document.querySelector('.wbGrid td.dirty')) extra = '（Motor Database 分頁有還沒存的格子，沒有自動重讀 —— 存好或放棄後按 Reload）';
      else {
        try { var pr = HT9045Page.load(); if (pr && pr.catch) pr.catch(function (e) { status('Motor Database 重讀失敗：' + ((e && e.message) || e), 'warn'); }); }
        catch (e) { extra = '（Motor Database 重讀失敗：' + e.message + '）'; }
      }
    }
    var lim1 = (gearBlock(rt()) || {}).limits || {};   // AI(W906-GEARRATIO2) 20261003: NB2 R171 L5 -- a +-999999 axis cannot go 200 mm in this tab (<= 100 mm from the start)
    if (ack.restartWbServe || ack.mode === 'hand' || S.mode === 'hand') {         // AI(W906-GEARHAND) 20261003: spec (9)
      S.handRuler = null; var hr = $('mtGearHandRuler'); if (hr) hr.textContent = '';
      status('已存檔：' + S.motor + ' 齒輪比 → ' + ((ack.gearRatio || {}).newText || '') + ' —— 這一軸手推過（伺服關過）：移動它之前先重開 wb_serve，再回原點（HOME）；重開之後再用手推模式量一次驗證' + extra, 'ok');
    } else
    status('已存檔：' + S.motor + ' 齒輪比 → ' + ((ack.gearRatio || {}).newText || '') + ' —— 這一軸要重新回原點（HOME），' +
           (S.placeholder ? '再用這個分頁量一次驗證（佔位軟體極限：離開起點最多 ' + (lim1.spanMaxMm || 100) + ' mm）' : '再走 200 mm 驗證') + extra, 'ok');
    render();
    return true;
  }
  function onFinish(was) {
    if (was && was.action === 'gearCalMove' && S.moving && S.target === null && was.state === 'aborted') S.moving = false;   // stopped before the ack
    render();
  }

  window.HTMtGear = { onAck: onAck, onFinish: onFinish, tick: tick, state: function () { return S; }, build: build };
  function boot() { setTimeout(function () { build(); render(); setInterval(tick, 250); }, 0); }   // after the engine's DOMContentLoaded keypad attach
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', boot);
  else boot();
})();
