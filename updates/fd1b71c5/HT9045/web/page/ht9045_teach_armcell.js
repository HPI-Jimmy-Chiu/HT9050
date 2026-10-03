/* ht9045_teach_armcell.js -- Teach: the "Arm Cell" tab -- move a chosen In / Out Arm nozzle over a chosen tray cell. Hand-written.
 *
 * AI(W906-ARMCELL) 20261002 [W906]: RULINGS_20261002 #18 (NB2 spec RD5軟體_NB2規格_Teach頁ArmCell分頁_20261002_101914.md §3.1).
 *   golden has no such tab -- the HT160 equivalent is Teach -> Advanced -> Sort Arm "Pick / Place Test". The C++ side
 *   (WebMotorAccess.cpp EOF, motor.access moveToTrayCell) does all of it: the S0 checks, every Enable Z to ZSafePos, golden's
 *   Z-at-home check, X / Y together, the optional Z down, every cancel (STOP, alarm, operator gone, page close ...). This page
 *   only chooses and shows: the arm, the nozzle (the Enable Z the C++ lists), the area (every golden area; the ones this
 *   machine or the first version cannot do are greyed, a click says why), the cell (tray picker + Col / Row: 1-based on the
 *   screen, 0-based on the wire), Z Down.
 * Data: teachMotorRuntime.armCell -- the page's existing 1 s /api/struct/motor/runtime read (HW.teach.html teachLoadMotors);
 *   no request of its own. {catalogRev, catalog:{ok, why, in, out, areas, zDownDefault, zDownWhy, zSafePos}, job:{...}}.
 *   No catalog (file://, C++ unreachable, an older wb_serve) -> the whole pane greyed with the reason, never a static
 *   snapshot (W5B-7).
 * "到位" (green) only from job.result === 'arrived' of the job this page started (job.jobId === the ack's cellSeq); the ack
 *   only says "accepted".
 * Loaded on HW.teach.html's PageControl2 line (the same line as its closing </div>), BEFORE the page's tab binder: the tab
 *   and pane inserted here get the same click handling as the golden sheets. At load time only the DOM is touched;
 *   HTMotorAccess, HTWidgets, HT9045Recipe and the page's teach* functions are used from events / timers (they load later).
 * Lock: motor-access.js unlocks right after the ack (finish), so the job's lock is taken one turn later
 *   (setTimeout 0, HTMotorAccess.lockAll keeping armCellStop) and dropped when the job reads inactive twice, on STOP, or when
 *   the window closes (HT_WIN). The C++ refuses every other Teach action while the job runs anyway (the real gate).
 * Token: HT9045Recipe.setTokenHold(busy) keeps the operator token while a job of this page runs (an idle release would
 *   cancel it half way, D3); keepAlive every 60 s like the HOME one.
 * Greying is reversible here (class teach-unwired + data-unwired, judged at click time), NOT teachMarkUnwiredEl (one way)
 *   and NOT TEACH_UNWIRED_B38 (its probe wants exactly 106).
 * AI(W906-ARMCELL2) 20261002 [W906]: NB2 R165 (the user 1002 22:1x 「照建議」):
 *   - "All Z Up (to Safe Z)" (HT160 uteach.dfm btnSaAllZUp on the same sheet): presses Teach's OWN In Z All Up / Out Z All Up
 *     for the chosen arm (btnInZAllUp / btnOutZAllUp, ht9045_teach_zallup_c.js AI(W906-TEACH-ZALLUP), C++ teachZAllUp) -- the
 *     same command, the same C++ gates, the same token hold and "Homeing..." caption; no second Z-up here. That button greyed
 *     (its catalog row missing, motor-access.js not ready) -> this one says the same reason and presses nothing.
 *   - a cancelled job shows "已中止" (the C++ ends it "cancelled" for STOP, a box on screen, EMG, an alarm lamp, ...; why says which).
 */
(function () {
  'use strict';
  var TAB_TITLE = 'tsArmCell : TTabSheet';
  var TAG = 'AI(W906-ARMCELL) 20261002';
  var TAG2 = 'AI(W906-ARMCELL2) 20261002', ZUP_CAP = 'All Z Up (to Safe Z)';
  var S = {
    arm: 'in', nozzle: '', area: '', col: 0, row: 0, zDownTouched: false,
    cat: null, catRev: -1, catText: '', paneWhy: '查無 C++ 的 Arm Cell 目錄（頁面剛載入）',
    pendingSeq: 0, stopAfterSeq: 0, lastSeq: 0, stopped: false, winClosed: false,
    myJob: 0, locked: false, inactive: 0, lastRt: null, lastActive: '', lastResultKey: '', lastActiveMotor: '',
    holdSet: false, keepTimer: 0
  };
  var pane = null, tab = null;

  function $(id) { return document.getElementById(id); }
  function info(msg, cls) { if (window.teachSetInfo) window.teachSetInfo(msg, cls); }
  function mk(tag, attrs, text) {
    var e = document.createElement(tag);
    for (var k in attrs) if (attrs.hasOwnProperty(k)) e.setAttribute(k, attrs[k]);
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function pos(l, t, w, h) { return 'position:absolute;left:' + l + 'px;top:' + t + 'px;width:' + w + 'px;height:' + h + 'px;'; }
  function grey(el, why) {
    if (!el) return;
    el.classList.add('teach-unwired');
    el.setAttribute('data-unwired', why);
    if (el.tagName === 'SELECT' || (el.tagName === 'INPUT' && el.type === 'checkbox')) { if (!S.locked) el.disabled = true; }
  }
  function ungrey(el) {
    if (!el || !el.getAttribute('data-unwired')) return;
    el.classList.remove('teach-unwired');
    el.removeAttribute('data-unwired');
    if (el.tagName === 'SELECT' || (el.tagName === 'INPUT' && el.type === 'checkbox')) { if (!S.locked) el.disabled = false; }
  }
  function greyWhy(el) { return el ? el.getAttribute('data-unwired') : null; }
  function status(msg, cls) {
    var e = $('armCellStatus'); if (!e) return;
    e.textContent = msg;
    e.style.background = cls === 'ok' ? '#0a5' : cls === 'err' ? '#b33' : cls === 'warn' ? '#d9c27a' : 'transparent';
    e.style.color = (cls === 'ok' || cls === 'err') ? '#fff' : 'var(--text,#222)';
  }

  // ---- the tab + pane (synchronous, before HW.teach.html's PageControl2 binder) ----
  function build() {
    var pc = $('PageControl2');
    if (!pc) return;
    var tabs = pc.querySelector(':scope > .pcTabs'), body = pc.querySelector(':scope > .pcBody');
    if (!tabs || !body || pc.querySelector(':scope > .pcTabs > .tab[title="' + TAB_TITLE + '"]')) return;
    var max = -1;
    Array.prototype.forEach.call(tabs.querySelectorAll(':scope > .tab'), function (t) { var n = parseInt(t.getAttribute('data-t'), 10); if (n > max) max = n; });
    var n = String(max + 1);                                                     // 17 today (spec §3.1: max + 1)
    tab = mk('div', { 'class': 'tab', 'data-t': n, title: TAB_TITLE }, 'Arm Cell');
    tabs.appendChild(tab);
    pane = mk('div', { 'class': 'pcPane', 'data-p': n, title: 'tsArmCell', id: 'armCellPane', style: 'display:none;' });
    body.appendChild(pane);
    var css = mk('style', { id: 'armCellCss' });
    css.textContent = '.teach-unwired{opacity:.4 !important;filter:grayscale(1);cursor:not-allowed !important;}' +
      '#armCellPane .armCellSel{outline:2px solid #06c;outline-offset:-2px;font-weight:bold;}' +
      '#armCellPane .armCellArea{width:134px;height:30px;margin:2px;font-size:11px;}' +
      '#armCellPane .armCellNote{position:absolute;font-size:11px;color:var(--text,#222);white-space:pre-wrap;overflow:hidden;}';
    document.head.appendChild(css);
    function group(id, legend, l, t, w, h) {
      var g = mk('fieldset', { 'class': 'gbx', id: id, title: id + ' : TGroupBox（' + TAG + '）', style: pos(l, t, w, h) });
      g.appendChild(mk('legend', { style: 'background:var(--panel,#ece9d8);' }, legend));
      var c = mk('div', { 'class': 'cli', style: 'position:absolute;inset:0;overflow:hidden;' });
      g.appendChild(c); pane.appendChild(g);
      return c;
    }
    function button(c, id, cap, l, t, w, h, fs) {
      var b = mk('button', { 'class': 'btn3d', id: id, title: id + ' : Arm Cell（' + TAG + '）', style: pos(l, t, w, h) + 'font-size:' + (fs || 11) + 'px;' }, cap);
      c.appendChild(b);
      return b;
    }
    var c1 = group('armCellGrpArm', 'Arm / Nozzle', 0, 0, 862, 66);
    button(c1, 'armCellArmIn', 'In Arm', 10, 22, 90, 30, 12);
    button(c1, 'armCellArmOut', 'Out Arm', 106, 22, 90, 30, 12);
    c1.appendChild(mk('span', { 'class': 'lb', style: pos(214, 29, 50, 16) }, 'Nozzle'));
    c1.appendChild(mk('select', { 'class': 'ed', id: 'armCellNozzle', title: 'armCellNozzle : Arm Cell nozzle（' + TAG + '）', style: pos(266, 23, 120, 28) }));
    c1.appendChild(mk('div', { 'class': 'armCellNote', id: 'armCellArmNote', style: pos(398, 16, 456, 46) }));
    var c2 = group('armCellGrpArea', 'Area', 0, 68, 862, 148);
    c2.appendChild(mk('div', { id: 'armCellAreas', style: 'position:absolute;left:6px;top:16px;right:6px;bottom:4px;display:flex;flex-wrap:wrap;align-content:flex-start;' }));
    var c3 = group('armCellGrpCell', 'Cell', 0, 218, 862, 420);
    c3.appendChild(mk('div', { id: 'armCellTrayBox', style: pos(8, 18, 560, 394) + 'overflow:hidden;' }));
    c3.appendChild(mk('span', { 'class': 'lb', style: pos(584, 24, 40, 16) }, 'Col'));
    c3.appendChild(mk('select', { 'class': 'ed', id: 'armCellCol', title: 'armCellCol : Arm Cell column（' + TAG + '）', style: pos(626, 18, 80, 28) }));
    c3.appendChild(mk('span', { 'class': 'lb', style: pos(716, 24, 40, 16) }, 'Row'));
    c3.appendChild(mk('select', { 'class': 'ed', id: 'armCellRow', title: 'armCellRow : Arm Cell row（' + TAG + '）', style: pos(758, 18, 80, 28) }));
    c3.appendChild(mk('div', { 'class': 'armCellNote', id: 'armCellCellNote', style: pos(584, 56, 270, 356) }));
    var c4 = group('armCellGrpGo', 'Move', 0, 640, 862, 186);
    var lab = mk('label', { 'class': 'ckb', id: 'armCellZDownLabel', title: 'armCellZDown : Arm Cell Z down（' + TAG + '）', style: 'position:absolute;left:10px;top:20px;height:22px;font-size:13px;white-space:nowrap;cursor:pointer;' });
    lab.appendChild(mk('input', { type: 'checkbox', id: 'armCellZDown' }));
    lab.appendChild(document.createTextNode('Z Down'));
    c4.appendChild(lab);
    c4.appendChild(mk('div', { 'class': 'armCellNote', id: 'armCellZNote', style: pos(10, 44, 560, 30) }));
    button(c4, 'armCellGo', 'GO', 600, 16, 120, 40, 16);
    button(c4, 'armCellStop', 'STOP', 730, 16, 120, 40, 16);
    button(c4, 'armCellZAllUp', ZUP_CAP, 436, 14, 156, 28, 11).setAttribute('title', 'armCellZAllUp : Arm Cell All Z Up（' + TAG2 + '）');   // HT160 btnSaAllZUp (NB2 R165 item 3)
    c4.appendChild(mk('div', { id: 'armCellStatus', title: 'armCellStatus : Arm Cell status（' + TAG + '）', style: pos(10, 78, 840, 22) + 'font-size:12px;font-weight:bold;padding:2px 6px;box-sizing:border-box;border-radius:3px;overflow:hidden;white-space:nowrap;' }));
    c4.appendChild(mk('div', { 'class': 'armCellNote', id: 'armCellPlan', title: 'armCellPlan : Arm Cell plan（' + TAG + '）', style: pos(10, 104, 840, 76) + 'font-size:10px;' }));
    wire();
    paneGrey(S.paneWhy);
    status('Arm Cell：等 C++ 的目錄（/api/struct/motor/runtime armCell）', '');
    planText('按 GO 之後由 C++ 照 golden 量產公式算目標（RULINGS_20261002 第 18 條）；這裡不算座標。');
  }

  // ---- greying of the whole pane (no catalog) ----
  function paneGrey(why) {
    S.paneWhy = why;
    ['armCellArmIn', 'armCellArmOut', 'armCellNozzle', 'armCellCol', 'armCellRow', 'armCellZDown', 'armCellGo', 'armCellZAllUp'].forEach(function (id) { grey($(id), why); });
    var n = $('armCellArmNote'); if (n) n.textContent = why;
  }
  function paneUngrey() {
    S.paneWhy = '';
    ['armCellArmIn', 'armCellArmOut', 'armCellNozzle', 'armCellCol', 'armCellRow', 'armCellZAllUp'].forEach(function (id) { ungrey($(id)); });
  }

  // ---- render from the catalog ----
  function armCat() { return S.cat ? S.cat[S.arm] : null; }
  function areasOfArm() { return S.cat && S.cat.areas ? S.cat.areas.filter(function (a) { return a.arm === S.arm; }) : []; }
  function areaOf(id) { var v = areasOfArm(); for (var i = 0; i < v.length; i++) if (v[i].id === id) return v[i]; return null; }
  function render() {
    if (!S.cat) return;
    var ac = armCat() || {};
    $('armCellArmIn').classList.toggle('armCellSel', S.arm === 'in');
    $('armCellArmOut').classList.toggle('armCellSel', S.arm === 'out');
    var sel = $('armCellNozzle'), nz = ac.nozzles || [];
    sel.innerHTML = '';
    nz.forEach(function (n) { sel.appendChild(mk('option', { value: n.alias }, n.alias + '  [' + n.i + '][' + n.j + ']' + (n.pci1203 ? '' : '（非 1203）'))); });
    if (!nz.some(function (n) { return n.alias === S.nozzle; })) S.nozzle = nz.length ? nz[0].alias : '';
    sel.value = S.nozzle;
    if (!nz.length) grey(sel, (S.arm === 'in' ? 'In' : 'Out') + ' Arm：C++ 沒有列出 Enable 的 Z'); else ungrey(sel);
    $('armCellArmNote').textContent = (ac.why ? '⚠ ' + ac.why : '') + (ac.why && ac.note ? '\n' : '') + (ac.note || '');
    var box = $('armCellAreas'), list = areasOfArm();
    box.innerHTML = '';
    if (!areaOf(S.area)) {                                                       // first time / the other arm: its first usable area (a greyed one stays selectable to read why)
      var first = list.filter(function (a) { return a.usable; })[0] || list[0];
      S.area = first ? first.id : '';
    }
    list.forEach(function (a) {
      var b = mk('button', { 'class': 'btn3d armCellArea', id: 'armCellArea_' + a.id, title: 'armCellArea_' + a.id + ' : Arm Cell area（' + TAG + '）' },
        a.label + (a.cols && a.rows ? '  ' + a.cols + '×' + a.rows : ''));
      if (!a.usable) grey(b, a.why || (a.label + '：不能用'));
      if (a.id === S.area) b.classList.add('armCellSel');
      b.addEventListener('click', function () {
        var w = greyWhy(b);
        if (w) { status(w, 'warn'); info(w, 'warn'); return; }
        if (S.area !== a.id) { S.area = a.id; S.col = 0; S.row = 0; }
        render();
      });
      box.appendChild(b);
    });
    renderCell();
    renderGo();
  }
  function fillSel(sel, n, cur) {
    sel.innerHTML = '';
    for (var i = 0; i < n; i++) sel.appendChild(mk('option', { value: String(i) }, String(i + 1)));
    sel.value = String(cur);
  }
  function renderCell() {
    var a = areaOf(S.area), boxT = $('armCellTrayBox'), cols = a ? a.cols : 0, rows = a ? a.rows : 0;
    if (S.col >= cols) S.col = 0;
    if (S.row >= rows) S.row = 0;
    fillSel($('armCellCol'), cols, S.col);
    fillSel($('armCellRow'), rows, S.row);
    boxT.innerHTML = '';
    if (a && cols > 0 && rows > 0 && window.HTWidgets && HTWidgets.makeMyTray) {
      var cw = Math.max(8, Math.min(60, Math.floor(540 / cols) - 2)), ch = Math.max(6, Math.min(44, Math.floor(380 / rows) - 2));
      var t = HTWidgets.makeMyTray({ name: 'armCellTray', xitem: cols, yitem: rows, cellW: cw, cellH: ch, showFont: true,
        onCellClick: function (x, y) { if (S.locked) return; S.col = x; S.row = y; renderCell(); renderGo(); } });
      t.setCell(S.col, S.row, 1, '●');
      boxT.appendChild(t);
    }
    var cn = $('armCellCellNote');
    cn.textContent = a ? (a.label + '：' + cols + ' 欄 × ' + rows + ' 列；選的格子 (' + (S.col + 1) + ', ' + (S.row + 1) + ')' +
      '\nZ：' + (a.zKind === 'pick' ? 'pick Z（取料高度）' : a.zKind === 'place' ? 'place Z（放料高度）' : '—') +
      (a.zDownWhy ? '\n' + a.zDownWhy : '') + (a.usable ? '' : '\n⚠ ' + a.why)) : '先選一區';
  }
  function renderGo() {
    var a = areaOf(S.area), ac = armCat() || {}, z = $('armCellZDown'), go = $('armCellGo');
    var why = S.paneWhy || (ac.why ? ac.why : '') || (!S.nozzle ? '這支手臂沒有可用的吸嘴' : '') ||
      (!a ? '先選一區' : (!a.usable ? a.why : '')) || (a && (a.cols <= 0 || a.rows <= 0) ? '這一區沒有格子' : '');
    if (why) grey(go, why); else ungrey(go);
    var zw = $('armCellZNote');
    if (a && a.zDownAllowed === false) { grey(z, a.zDownWhy || '這一區不能降 Z'); z.checked = false; zw.textContent = a.zDownWhy || ''; }
    else {
      ungrey(z);
      if (!S.zDownTouched && S.cat) z.checked = !!S.cat.zDownDefault;
      zw.textContent = (S.cat && !S.cat.zDownDefault ? '預設不勾：' + (S.cat.zDownWhy || '') : '') + (a && a.zDownWhy ? (S.cat && !S.cat.zDownDefault ? '\n' : '') + a.zDownWhy : '');
    }
  }
  function planText(t) { var p = $('armCellPlan'); if (p) p.textContent = t; }
  function showPlan(pl, note) {
    if (!pl) return;
    var L = [];
    L.push(pl.label + ' (' + (pl.col + 1) + ', ' + (pl.row + 1) + ')  ' + pl.nozzle + '：X ' + pl.x.motor + ' → ' + pl.x.target + '，Y ' + pl.y.motor + ' → ' + pl.y.target +
      '，Z ' + pl.z.motor + ' → ' + (pl.zDown ? pl.z.target + '（' + pl.z.kind + '）' : '不降（停在 ZSafePos ' + pl.zSafe + '）') + '；先抬 ' + (pl.zLift || []).join('、') + ' 到 ZSafePos ' + pl.zSafe);
    L.push('教導點 X ' + pl.teach.x + ' Y ' + pl.teach.y + ' Z ' + pl.teach.z + '；基準格（照 Tech 重算）X ' + pl.base.x + ' Y ' + pl.base.y +
      (pl.prod ? '；Prod 現值 X ' + pl.prod.x + ' Y ' + pl.prod.y : '') + '；補償前 X ' + pl.cell.x + ' Y ' + pl.cell.y);
    if (pl.d2) L.push('D2：' + pl.d2.what + '（' + pl.d2.motor + ' 要在 ' + pl.d2.target + '）');
    (pl.notes || []).forEach(function (n) { L.push('• ' + n); });
    if (note && (pl.notes || []).indexOf(note) < 0) L.push('• ' + note);
    planText(L.join('\n'));
  }

  // ---- the job (runtime) ----
  function lockOn() {
    if (!window.HTMotorAccess || !HTMotorAccess.lockAll) return;
    HTMotorAccess.lockAll('armCellStop');
    S.locked = true;
  }
  function lockOff() {
    if (S.locked && window.HTMotorAccess && HTMotorAccess.unlockAll) HTMotorAccess.unlockAll();
    S.locked = false;
  }
  //AI(W906-ARMCELL) 20261002: review m6 -- never busy while the window is closed: the token hold and the 60 s keepAlive stop there,
  //  so the recipe client's idle release returns the token like the other teach pages after a close (the C++ page-close edge
  //  has already stopped the job; a released token is the operator-gone cancel on top of it).
  function busy() { return !S.winClosed && (!!S.pendingSeq || (!!S.myJob && !S.inactive && S.lastActive === 'yes')); }
  function holdToken() {
    if (S.holdSet || !window.HT9045Recipe || !HT9045Recipe.setTokenHold) return;
    S.holdSet = true;
    HT9045Recipe.setTokenHold(function () { return busy(); });
    S.keepTimer = setInterval(function () { if (busy() && HT9045Recipe.keepAlive) HT9045Recipe.keepAlive().catch(function () {}); }, 60000);
  }
  function jobText(j) {
    return (j.label || j.area || '?') + ' (' + ((j.col | 0) + 1) + ', ' + ((j.row | 0) + 1) + ')，' + (j.stepName || '');
  }
  function onRuntime(rt) {
    if (!rt) return;                                                             // unknown: keep everything (the lock too)
    var ac = rt.armCell;
    var src = window.teachMotorSrc;
    if (!ac || typeof ac !== 'object') {
      paneGrey(src && src !== 'C++' ? 'Arm Cell 需要 wb_serve（C++）的目錄；這頁現在的馬達資料來源是「' + src + '」（file:// 或 C++ 讀不到）—— 整頁灰掉，不退回靜態快照（W5B-7）'
                                    : '這版 C++ 的 /api/struct/motor/runtime 沒有 armCell 區塊（舊版 wb_serve）—— Arm Cell 不能用');
      return;
    }
    var cat = ac.catalog || {};
    if (cat.ok !== true) paneGrey('C++ 沒有 Arm Cell 目錄：' + (cat.why || '（沒有原因）'));
    else if (ac.catalogRev !== S.catRev || S.paneWhy) {
      if (!S.locked) { S.cat = cat; S.catRev = ac.catalogRev; paneUngrey(); render(); }
    }
    var j = ac.job || {};
    // the job this page started: follow it; another one (another tab / an older run): only say so
    if (S.myJob && j.jobId === S.myJob) {
      if (j.active) {
        S.inactive = 0; S.lastActive = 'yes';
        status('Arm Cell 進行中：' + jobText(j) + '（job ' + j.jobId + '）', 'warn');
        if (j.activeMotor && j.activeMotor !== S.lastActiveMotor && window.teachSelectMotor) { S.lastActiveMotor = j.activeMotor; window.teachSelectMotor(j.activeMotor, true); }   // golden ActiveMotorIndex (S2) -- like teachEchoFrom's active=
        if (!S.locked && !S.stopped) lockOn();                                   // not again after this page's STOP (the runtime read may predate it)
      } else {
        if (rt !== S.lastRt) S.inactive++;
        S.lastActive = 'no';
        var key = j.jobId + ':' + j.result;
        if (key !== S.lastResultKey) {
          S.lastResultKey = key;
          if (j.result === 'arrived') {
            var f = j.final || {}, nv = function (p) { return (p && p.cmdPos != null) ? String(p.cmdPos) : '—'; };
            status('到位：' + (j.label || j.area) + ' (' + ((j.col | 0) + 1) + ', ' + ((j.row | 0) + 1) + ')  X ' + nv(f.x) + ' Y ' + nv(f.y) + ' Z ' + nv(f.z) + '（Z 不自動抬起：要抬按 In／Out Z All Up）', 'ok');
          } else {
            var stopBad = !!(j.stop && j.stop.refused > 0);                        // review N5: a refused stop is an error, never a calm yellow
            status('Arm Cell ' + (j.result === 'cancelled' ? '已中止' : j.result === 'timeout' ? '逾時' : j.result === 'failed' ? '失敗' : j.result) + '（' + (j.stepName || '') + '）：' + (j.why || ''), (j.result === 'cancelled' && !stopBad) ? 'warn' : 'err');   // AI(W906-ARMCELL2) 20261002: cancelled = 已中止 (was 取消; NB2 R165 review)
          }
        }
        if (S.inactive >= 2) { lockOff(); S.myJob = 0; }                          // two reads, like teachSyncHome
      }
    } else if (j.active && !S.pendingSeq) {
      status('另一個 Arm Cell 進行中（job ' + j.jobId + '：' + jobText(j) + '）—— 不是這一頁送的', 'warn');
    }
    S.lastRt = rt;
  }
  // AI(W906-ARMCELL2) 20261002: NB2 R165 item 3 -- HT160 btnSaAllZUp = Teach's own In / Out Z All Up for the chosen arm (file head)
  function zAllUpSrc() { return S.arm === 'out' ? 'btnOutZAllUp' : 'btnInZAllUp'; }
  function zAllUp() {
    var b = zAllUpSrc(), el = $(b), cap = S.arm === 'out' ? 'Out Z All Up' : 'In Z All Up';
    if (!el) { status('All Z Up：頁面上找不到 ' + b + '（Axle Control 頁的「' + cap + '」）—— 什麼都沒按', 'err'); return; }
    var w = el.getAttribute('data-unwired');
    if (w) { status('All Z Up 按不了：' + w, 'warn'); info(w, 'warn'); return; }   // the same gate the Teach button has (greyed there = greyed here)
    if (el.disabled) { status('All Z Up：「' + cap + '」現在按不到（畫面鎖住：有動作在進行，先按 STOP 或等它結束）', 'warn'); return; }
    el.click();                                                                  // ht9045_teach_zallup_c.js onClick -> HTMotorAccess.send(b) -> C++ teachZAllUp (its gates)
    status('All Z Up：已按下 Axle Control 頁的「' + cap + '」（同一顆鈕的程式：那一臂每一支 Z 歸零後到 ZSafePos；C++ 拒絕時理由在右下角狀態列）', 'warn');
  }
  function tick() {
    if (!pane || S.winClosed) return;                                            // window closed: the page's runtime read stops too (stale data must not re-lock)
    var zb = $('armCellZAllUp'), zs = $(zAllUpSrc());                           // AI(W906-ARMCELL2): the source button's golden caption ("Homeing..." while its Z home)
    if (zb) { var zt = (zs && zs.textContent === 'Homeing...') ? 'Homeing...' : ZUP_CAP; if (zb.textContent !== zt) zb.textContent = zt; }
    if (window.teachMotorRuntime) onRuntime(window.teachMotorRuntime);
    var src = window.teachMotorSrc;
    if (!S.cat && !window.teachMotorRuntime && src && src !== 'C++' && src !== 'none')
      paneGrey('Arm Cell 需要 wb_serve（C++）的目錄；這頁現在的馬達資料來源是「' + src + '」（file:// 或 C++ 讀不到）—— 整頁灰掉，不退回靜態快照（W5B-7）');
  }

  // ---- buttons ----
  function wire() {
    function on(id, fn) { var e = $(id); if (e) e.addEventListener('click', function (ev) { var w = greyWhy(e); if (w) { ev.preventDefault(); status(w, 'warn'); info(w, 'warn'); return; } fn(ev); }); }
    on('armCellArmIn', function () { if (S.arm !== 'in') { S.arm = 'in'; S.area = ''; S.col = 0; S.row = 0; render(); } });
    on('armCellArmOut', function () { if (S.arm !== 'out') { S.arm = 'out'; S.area = ''; S.col = 0; S.row = 0; render(); } });
    $('armCellNozzle').addEventListener('change', function () { S.nozzle = this.value; renderGo(); });
    $('armCellCol').addEventListener('change', function () { S.col = parseInt(this.value, 10) || 0; renderCell(); renderGo(); });
    $('armCellRow').addEventListener('change', function () { S.row = parseInt(this.value, 10) || 0; renderCell(); renderGo(); });
    $('armCellZDown').addEventListener('click', function (ev) { var w = greyWhy(this); if (w) { ev.preventDefault(); status(w, 'warn'); return; } S.zDownTouched = true; });
    on('armCellGo', go);
    on('armCellZAllUp', zAllUp);                                                 // AI(W906-ARMCELL2) 20261002
    var stop = $('armCellStop');
    stop.addEventListener('click', function () {
      if (!window.HTMotorAccess) { status('STOP：motor-access.js 還沒載入 —— 請按右邊的 STOP', 'err'); return; }
      S.stopAfterSeq = S.lastSeq; S.stopped = true;                              // a GO ack / a runtime read that comes after this STOP must not lock again
      HTMotorAccess.send('btnStop');                                             // motor.stop -> C++ DoStop -> CancelAllJobs (the job stops its own axes)
      lockOff();
      status('STOP 已送出：Arm Cell 由 C++ 取消並停軸', 'warn');
    });
    window.addEventListener('message', function (ev) {                          // the frame closes / minimizes / reopens the window (HW.teach.html :777's rule)
      var d = ev.data;
      if (!d || d.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
      winEdge(!!d.open);                                                         // = HW.teach.html teachWinShown: minimized sends open:true and keeps polling (golden: no FormClose)
    });
  }
  // the frame's HT_WIN edge (also called by the probe): closed -> unlock and forget the job (the C++ page-close edge stops it,
  //   WebMotorAccess.cpp WSLINK-B); busy() is false from here on, so the token is no longer held / renewed (review m6)
  function winEdge(open) {
    S.winClosed = !open;
    if (open) return;
    lockOff();
    S.pendingSeq = 0; S.myJob = 0; S.inactive = 0; S.lastActive = ''; S.stopped = false;
  }
  function go() {
    if (!window.HTMotorAccess) { status('motor-access.js 還沒載入', 'err'); return; }
    if (S.pendingSeq || S.locked) { status('Arm Cell 已經在走 —— 先按 STOP 或等它走完', 'warn'); return; }
    var ac = armCat() || {}, a = areaOf(S.area);
    if (!a || !S.nozzle) return;
    var z = $('armCellZDown'), zDown = !!(z && z.checked && !greyWhy(z));
    holdToken();
    var motors = [ac.x && ac.x.motor, ac.y && ac.y.motor, S.nozzle].filter(function (m) { return !!m; });
    var req = HTMotorAccess.send('armCellGo', { arm: S.arm, nozzle: S.nozzle, area: S.area, col: S.col, row: S.row, zDown: zDown, speedEvent: false }, motors);
    if (!req) return;                                                            // busy / no catalog row: motor-access.js said why
    S.pendingSeq = req.seq; S.lastSeq = req.seq; S.myJob = 0; S.inactive = 0; S.lastResultKey = ''; S.lastActiveMotor = ''; S.stopped = false;
    status('送出 ' + a.label + ' (' + (S.col + 1) + ', ' + (S.row + 1) + ')' + (zDown ? '，降 Z' : '，不降 Z') + ' —— 等 C++ 受理', 'warn');
  }
  // HW.teach.html teachOnAck hands every moveToTrayCell reply here (same line as its speedEvent line).
  function onAck(req, ack, err) {
    if (!req || req.seq !== S.pendingSeq) return;                                // an older GO
    S.pendingSeq = 0;
    if (!ack) { status('Arm Cell 被拒（什麼都沒動）：' + ((err && err.message) || '?'), 'err'); return; }
    if (ack.result !== 'cellMoving' || !ack.cellActive) { status('Arm Cell：C++ 回 ' + (ack.result || '?') + ' —— ' + (ack.message || ''), 'err'); return; }
    showPlan(ack.plan, ack.note);
    S.myJob = ack.cellSeq; S.inactive = 0; S.lastActive = 'yes';                // tracked even after a STOP: the runtime tells what really happened
    if (S.stopAfterSeq >= req.seq) { status('STOP 已按：這筆受理比 STOP 晚回來，不上鎖 —— 結果看 runtime（還在走就再按一次 STOP）', 'warn'); return; }
    status('受理（還沒到位）：' + (ack.message || ''), 'warn');
    setTimeout(function () { if (S.myJob === ack.cellSeq && S.lastActive === 'yes' && S.stopAfterSeq < req.seq) lockOn(); }, 0);   // after motor-access.js finish() -> unlockAll
  }

  window.HTArmCell = { onAck: onAck, busy: busy, tick: tick, winEdge: winEdge, state: function () { return S; } };
  build();
  setInterval(tick, 500);
})();
