/* ht9045_trayedit.js -- HW.TrayEdit.html <-> golden TTrayEditForm (906_0625_Steven uTrayEditForm.cpp / .dfm / .h)
 * ---------------------------------------------------------------------------
 * AI(W906-S10-TRAYEDIT) 20260929 (St02-E helper)
 * Hand-written page file. Every piece of logic and every guard runs in C++ (TrayEditForm.cpp / WebTrayEdit.cpp);
 * this file only draws the state C++ returns and sends operator operations. It never decides whether an edit is allowed.
 *
 * Command: WS `act.trayEdit`, value = JSON string. EVERY reply has the shape
 *   {"executed":bool, "op":"...", "guard":"...", "detail":"...", "state":{...}}   (the form is always under "state")
 *   state = {open, motor, motorName, hasMap, xItem, yItem, xbItem, ybItem, xbWidth, ybWidth,
 *            color[], number[]        indexed y*xItem + x (x = column, y = row), capped at 100 in each direction,
 *            colorMap{"<idx>":"#RRGGBB"}  overrides of the default palette (golden SetColorMap, e.g. HAS_OCR_OK -> clRed),
 *            sel{x0,y0,x1,y1}, caption, binCount{visible,items[],value}, updateVisible, noSave}
 *   ops  {"op":"state"}                              poll, every POLL_MS while the window is visible (C++ runs golden Timer1Timer in it)
 *        {"op":"select","x0","y0","x1","y1","moved"} golden MouseDown(x0,y0) -> MouseUp(x1,y1) -> SetTray. `moved` is extra
 *                                                    (true when at least one MouseMove landed on a cell = golden ShowTray ran);
 *                                                    C++ may ignore it.
 *        {"op":"binCount","value":"n"}               cbBinCount (iHasMap == 2)
 *        {"op":"noSave","value":bool}                CheckBox1
 *        {"op":"fill","x":"...","y":"..."}           edtXPos / edtYPos + Button1 (golden Button1Click)
 *        {"op":"update"} / {"op":"cancel"}           spbUpdate / SpeedButton2 (golden spbUpdateClick / SpeedButton2Click)
 *        {"op":"snapshot"}                           Button2 (golden Button2Click -> SaveJPG("Enter"))
 *   A golden refusal ("system-running", "not-authorized", "fifo", "plate", "not-open", "no-tray", "customer", "no-handler",
 *   "already-open", "bad-value", "out-of-grid", "not-editable", "bad-payload", "unknown-op") comes back with executed:false:
 *   the page shows guard + detail in the status line and NEVER retries it.
 *
 * Sending (pattern of ht9045_sortct_wire.js): HT9045Recipe.rawCmd(name, {value: JSON.stringify(...)}), unwrap / parseErr as
 *   there; control.acquire before every write op. One request at a time (the poll waits for a user op and the other way round).
 *   Server WebCmdGuard answers "busy:" to an identical command within 400 ms: the poll `state` is identical every time, so the
 *   next poll goes out POLL_MS (500) after the previous one completed. A busy reply to the poll = try the next tick; a busy
 *   reply to a write op = shown as HT9045Busy.NOTE (not an error, not resent). Identical write ops inside the page cool-down
 *   (HT9045Busy.coolMs, 400 ms after the ack) or already queued / in flight are dropped (double-click rule, Steven 20260926).
 *   If the server refuses `state` with not-operator, the next poll does control.acquire first (once per refusal).
 *
 * Polling stops when the window is hidden (document.hidden, or background.html set the frame to display:none), when
 *   background.html says HT_WIN open:false, and when state.open is false (after Update / Abort / Manual Input C++ closes the
 *   form; the window itself is closed by the page table). It restarts on HT_WIN open:true, on a hidden -> visible edge, or
 *   with the "重新讀取" button.
 *
 * Drawing = golden TTMyTray (vclcompat/component_src/HTray.cpp; golden elec\myvcl\HTray.cpp), same geometry formulas:
 *   CaculateTrayParameter :262-270 (LineWidth 1, EdgeWidth 1), DrawSingleIC :230-259 (block gap = x / (XItem / XBlock) * XBlockWidth),
 *   SetBlockXItem :143 clamps XBlock to 1..2, SetBlockYItem :165 clamps YBlock to 1..5, SetX/YBlockWidth <= 0 -> 1,
 *   DrawTray :272 (white tray, black frame, csLeftTop direction mark of DirectWidth 10), ColorMap default :23-27
 *   (= ht9045_mv_trays.js PALETTE; index 11..31 = clBlack; index < 0 or >= 32 is ignored by SetCellColorIndex :382-388).
 *   Page choices (not golden): a cell with no value in color[] is drawn grey (#C0C0C0 = unknown); cell text is white on dark
 *   fills (golden always draws clWindowText); pixel -> cell uses the drawn geometry including block gaps (golden
 *   ConvertIndexCells :437 ignores the gaps -- C++ receives cell indexes, so that quirk does not reach it).
 * Drag preview = golden ShowTray :376-456 colour index 2 inside the rectangle. Outside the rectangle golden repaints from
 *   BackTray (e.g. OCR cells -> 0, iHasMap 1 value 2 -> colour 3); the page keeps the last C++ colours there because BackTray
 *   is not in the state. The C++ reply after the drop is what stays on screen.
 * Golden MouseDown :257-314 checks plate / SIGURD_HUKOU / security BEFORE the drag starts; here those checks run in C++ when
 *   the select arrives, so a refused drag shows a preview first and then the guard in the status line.
 *
 * window.HT9045TrayEdit: state(), last(), lastError(), busy(), stats(), poll(), select(x0,y0,x1,y1), update(), cancel(),
 *   fill(x,y), snapshot(), noSave(bool), binCount(v) -- for probes.
 * ---------------------------------------------------------------------------
 */
(function () {
  'use strict';

  var CMD = 'act.trayEdit';
  var POLL_MS = 500;               // >= WebCmdGuard 400 ms for the identical `state` command
  var SLOW_MS = 3000;              // back-off after a transport error / missing dispatch
  var WATCH_MS = 1000;             // local visibility watcher (no network)
  var GOLD_W = 491, GOLD_H = 567;  // dfm mtLoaderBuffer Width / Height
  var DFM_XITEM = 5, DFM_YITEM = 8;
  var MAX_CELLS = 100;             // C++ caps color[] / number[] at 100 per direction (golden BackTray[100][100])
  var MAX_COLOR_INDEX = 32;        // HTray.h MAX_COLOR_INDEX
  var PREVIEW_INDEX = 2;           // golden ShowTray :409
  var MIN_CELL_PX = 3;
  var DIRECT_WIDTH = 10;           // TTMyTray FDirectWidth default (csLeftTop)
  var MAX_QUEUE = 8;
  // TTMyTray default ColorMap (HTray.cpp:23-27) as CSS RGB -- same array as ht9045_mv_trays.js:44-45
  var PALETTE = ['#FFFFFF', '#008000', '#FFFF00', '#00FF00', '#FF0000', '#8080FF', '#BE34DC', '#0000FF', '#FF80FF',
                 '#8000FF', '#80FF00'];
  var UNKNOWN_CSS = '#C0C0C0';
  var HEX = /^#[0-9A-Fa-f]{6}$/;

  function $(id) { return document.getElementById(id); }

  var S = null;                    // the state object currently drawn (reply.state)
  var lastReply = null, lastError = '';
  var inflight = null;             // {kind:'poll'|'op', key, op}
  var queue = [];                  // user ops waiting: {req, key, label}
  var doneAt = {};                 // op key -> ms of its last ack (page cool-down)
  var pollTimer = null, pollDelay = POLL_MS, lastPollDone = 0;
  var winOpen = null;              // HT_WIN from background.html; null = never told (page opened on its own)
  var closedByState = false;       // state.open was false -> no polling until the window is reopened
  var wasVisible = false, needToken = false;
  var drag = null;                 // {x0,y0,x1,y1,moved,id}
  var G = null;                    // geometry of the last draw
  var lastSig = '', rafPending = false;
  var st = { polls: 0, ops: 0, busy: 0, dropped: 0, errors: 0, renders: 0 };

  // --- status lines --------------------------------------------------------------------------
  function put(id, msg, kind) {
    var el = $(id);
    if (!el) return;
    el.textContent = msg || '';
    el.className = kind ? ('st-' + kind) : '';
  }
  function sayOp(msg, kind) { put('teStatus', msg, kind); }
  function sayPoll(msg, kind, reload) {
    put('tePoll', msg, kind);
    if (!reload) return;
    var el = $('tePoll'), b = document.createElement('button');
    b.type = 'button'; b.className = 'btn3d'; b.id = 'teReload'; b.textContent = '重新讀取';
    b.addEventListener('click', function () { closedByState = false; pollDelay = POLL_MS; pollNow(); });
    el.appendChild(b);
  }
  function busyNote() { return (window.HT9045Busy && HT9045Busy.NOTE) || '同一個指令剛送過，這一下略過'; }
  function coolMs() { return (window.HT9045Busy && HT9045Busy.coolMs) ? HT9045Busy.coolMs() : 400; }
  function guardText(r) {
    if (!r) return '沒有回應';
    return (r.guard || '?') + (r.detail ? '：' + r.detail : '');
  }

  // --- transport (ht9045_sortct_wire.js pattern) ---------------------------------------------
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }
  function parseErr(e) {
    var t = (e && e.message) || String(e);
    try { var j = JSON.parse(t); if (j && typeof j === 'object') return j; } catch (x) {}
    return { executed: false, guard: 'transport', detail: t };
  }
  function cmd(name, extra) {
    if (!window.HT9045Recipe || !HT9045Recipe.rawCmd) return Promise.reject(new Error('ht9045_recipe_client.js 沒有載入'));
    return HT9045Recipe.rawCmd(name, extra).then(unwrap);
  }
  function send(req, withToken) {
    var pre = withToken ? cmd('control.acquire').catch(function () { /* held already or by someone else: the command itself answers */ })
                        : Promise.resolve();
    return pre.then(function () { return cmd(CMD, { value: JSON.stringify(req) }); })
      .then(function (r) { return r; }, function (e) { return parseErr(e); });
  }
  // '' = a real reply from C++; otherwise what went wrong on the way
  function transportKind(r) {
    if (!r || typeof r !== 'object') return 'transport';
    if (r.guard !== 'transport') return '';
    var d = String(r.detail || '');
    if ((window.HT9045Busy && HT9045Busy.is(r)) || /^busy:/.test(d)) return 'busy';
    if (d === 'not-operator') return 'token';
    if (d === 'modal-pending') return 'modal';
    if (/unknown cmd/.test(d)) return 'unwired';
    return 'transport';
  }

  // --- visibility / window state ---------------------------------------------------------------
  function visible() {
    if (document.hidden) return false;
    try {
      var fe = window.frameElement;                                   // background.html iframe; null when opened directly
      if (fe && fe.getClientRects().length === 0) return false;       // frame display:none (minimised / closed)
    } catch (e) { /* no frameElement access: treat as visible */ }
    return true;
  }
  function active() { return visible() && winOpen !== false && !closedByState; }

  // --- poll loop -------------------------------------------------------------------------------
  function stopPoll() { if (pollTimer) { clearTimeout(pollTimer); pollTimer = null; } }
  function schedulePoll() {
    stopPoll();
    if (!active()) return;
    var wait = Math.max(0, lastPollDone + pollDelay - Date.now());
    pollTimer = setTimeout(function () { pollTimer = null; pollOnce(); }, wait);
  }
  function pollNow() { stopPoll(); if (!inflight) pollOnce(); }
  function pollOnce() {
    if (inflight) return;                                             // pump() reschedules when it is done
    if (queue.length) { pump(); return; }
    if (!active()) { stopPoll(); return; }
    if (Date.now() - lastPollDone < POLL_MS) { schedulePoll(); return; }   // identical command: keep >= 500 ms apart
    inflight = { kind: 'poll', key: '', op: 'state' };
    var withToken = needToken; needToken = false;
    st.polls++;
    send({ op: 'state' }, withToken).then(function (r) {
      inflight = null; lastPollDone = Date.now();
      try { onPollReply(r); } catch (e) { lastError = String(e && e.message || e); }
      pump();
    });
  }
  function onPollReply(r) {
    var k = transportKind(r);
    if (k === 'busy') { st.busy++; pollDelay = POLL_MS; return; }    // try the next tick
    if (k === 'token') { needToken = true; pollDelay = POLL_MS; sayPoll('讀取表單狀態需要控制權（not-operator），下一次先取得控制權', 'note'); return; }
    if (k === 'modal') { pollDelay = POLL_MS; sayPoll('機台訊息框等待回答中，回答之後才會更新', 'note'); return; }
    if (k === 'unwired') {
      pollDelay = SLOW_MS; st.errors++;
      lastError = '伺服器還沒有 act.trayEdit 的分派（tools/wb_serve.cpp），畫面不會更新';
      sayPoll(lastError, 'bad'); return;
    }
    if (k === 'transport') {
      pollDelay = SLOW_MS; st.errors++;
      lastError = '連線問題：' + ((r && r.detail) || '沒有回應');
      sayPoll(lastError + '（' + (SLOW_MS / 1000) + ' 秒後再讀）', 'bad'); return;
    }
    lastReply = r;
    if (r.state && typeof r.state === 'object') {
      pollDelay = POLL_MS;
      applyState(r.state);
      if (S && S.open === false) sayPoll('Tray Edit 表單目前沒有開啟（golden Close() 之後），已停止讀取；視窗再開啟時會重新讀取', 'note', true);
      else if (r.executed === false && r.guard) sayPoll('state：' + guardText(r), 'note');
      else sayPoll('');
    } else {
      pollDelay = SLOW_MS;
      lastError = 'state 沒有回表單：' + guardText(r);
      sayPoll(lastError, 'bad');
    }
  }

  // --- user ops --------------------------------------------------------------------------------
  function pump() {
    if (inflight) return;
    if (queue.length) { runOp(queue.shift()); return; }
    schedulePoll();
  }
  function enqueue(req, label) {
    var key = JSON.stringify(req), i;
    var dup = (inflight && inflight.key === key) || (Date.now() - (doneAt[key] || 0) < coolMs());
    for (i = 0; !dup && i < queue.length; i++) if (queue[i].key === key) dup = true;
    if (dup) { st.dropped++; sayOp(label + '：' + busyNote(), 'note'); return false; }
    if (queue.length >= MAX_QUEUE) { sayOp(label + '：前面還有 ' + queue.length + ' 個操作在等，這一下沒有送出', 'bad'); return false; }
    queue.push({ req: req, key: key, label: label });
    markBusy();
    if (!inflight) { stopPoll(); pump(); }
    return true;
  }
  function runOp(item) {
    inflight = { kind: 'op', key: item.key, op: item.req.op };
    st.ops++;
    sayOp(item.label + '…');
    send(item.req, true).then(function (r) {
      inflight = null; doneAt[item.key] = Date.now();
      try { onOpReply(item, r); } catch (e) { lastError = String(e && e.message || e); }
      markBusy();
      pump();
    });
  }
  function onOpReply(item, r) {
    var k = transportKind(r);
    if (k === 'busy') { st.busy++; sayOp(item.label + '：' + busyNote(), 'note'); return; }   // not a failure, not resent
    if (k) {
      st.errors++;
      lastError = item.label + ' 沒有執行：' + (k === 'token' ? '沒有控制權（not-operator），可能別的畫面正持有控制權'
        : k === 'modal' ? '機台訊息框等待回答中（modal-pending）'
        : k === 'unwired' ? '伺服器還沒有 act.trayEdit 的分派（tools/wb_serve.cpp）'
        : '連線問題：' + ((r && r.detail) || '沒有回應'));
      sayOp(lastError, 'bad');
      return;
    }
    lastReply = r;
    if (r.state && typeof r.state === 'object') {
      applyState(r.state);
      lastPollDone = Date.now();                                      // fresh state just arrived: the next poll waits a full tick
    }
    if (r.executed === true) {
      lastError = '';
      var closed = (S && S.open === false);
      sayOp(item.label + ' 完成' + (r.detail ? '（' + r.detail + '）' : '') + (closed ? '，表單已關閉' : ''));
    } else {
      lastError = item.label + ' 沒有執行：' + guardText(r);
      sayOp(lastError, 'bad');
    }
    if (S && S.open === false && queue.length) {                      // the form is gone: the rest would only hit "not-open"
      st.dropped += queue.length; queue.length = 0;
    }
  }
  function pendingOp(op) {
    if (inflight && inflight.op === op && inflight.kind === 'op') return true;
    for (var i = 0; i < queue.length; i++) if (queue[i].req.op === op) return true;
    return false;
  }
  function markBusy() {
    ['spbUpdate', 'SpeedButton2', 'Button1', 'Button2'].forEach(function (id) {
      var b = $(id), op = { spbUpdate: 'update', SpeedButton2: 'cancel', Button1: 'fill', Button2: 'snapshot' }[id];
      if (!b) return;
      if (pendingOp(op)) b.setAttribute('aria-busy', 'true'); else b.removeAttribute('aria-busy');
    });
  }

  // --- state -> controls -----------------------------------------------------------------------
  function toInt(v) { var n = Number(v); return isFinite(n) ? Math.trunc(n) : NaN; }
  function clampInt(v, lo, hi, dflt) { var n = toInt(v); if (isNaN(n)) n = dflt; return n < lo ? lo : (n > hi ? hi : n); }

  function applyState(s) {
    S = s;
    if (s.open === false) closedByState = true;
    else if (s.open === true) closedByState = false;
    if (drag && s.open !== true) drag = null;
    renderControls();
    requestDraw();
  }
  function captionText() {
    if (drag) return '(' + drag.x0 + ',' + drag.y0 + ')...(' + drag.x1 + ',' + drag.y1 + ')';   // golden Timer1Timer shows the live iStart/iEnd
    if (S && typeof S.caption === 'string' && S.caption !== '') return S.caption;
    return 'TrayEditForm';                                                                      // dfm Caption
  }
  function renderCaption() { var c = $('teCaption'); if (c) c.textContent = captionText(); }

  function renderControls() {
    var s = S || {}, open = (s.open === true);
    var m = $('teMotor');
    if (m) {
      m.textContent = (typeof s.motorName === 'string' && s.motorName) ? s.motorName : (S ? ('motor ' + (s.motor != null ? s.motor : '---')) : '---');
      m.title = 'EditTray(MotorIndexIndex=' + (s.motor != null ? s.motor : '---') + ', iHasMap=' + (s.hasMap != null ? s.hasMap : '---') + ')';
    }
    renderCaption();

    var up = $('spbUpdate');
    if (up) up.style.display = (s.updateVisible === false) ? 'none' : '';
    var b2 = $('Button2');
    if (b2) b2.hidden = (s.snapshotVisible !== true);               // dfm Button2 Visible=False

    var cb = $('CheckBox1');
    if (cb && !pendingOp('noSave') && typeof s.noSave === 'boolean') cb.checked = s.noSave;

    var bc = s.binCount || null, sel = $('cbBinCount');
    if (sel) {
      sel.hidden = !(bc && bc.visible === true);
      if (bc && !pendingOp('binCount')) fillBinCount(sel, bc);
    }

    ['spbUpdate', 'SpeedButton2', 'Button1', 'Button2', 'CheckBox1', 'cbBinCount', 'edtXPos', 'edtYPos'].forEach(function (id) {
      var el = $(id); if (el) el.disabled = !open;
    });

    var ov = $('teOverlay');
    if (ov) {
      if (!S) { ov.hidden = false; ov.textContent = '等待 C++ 回應…'; }
      else if (!open) { ov.hidden = false; ov.textContent = 'Tray Edit 表單目前沒有開啟（golden EditTray 由機台流程叫出）'; }
      else ov.hidden = true;
    }
  }
  function fillBinCount(sel, bc) {
    var items = (bc.items && bc.items.length) ? bc.items.map(String) : [];
    var val = (bc.value == null) ? '' : String(bc.value);
    var want = items.slice();
    if (want.indexOf(val) < 0) want.push(val);                       // golden cbBinCount is csDropDown: Text may be outside Items
    var have = Array.prototype.map.call(sel.options, function (o) { return o.value; });
    if (have.join('\u0001') !== want.join('\u0001')) {
      sel.innerHTML = '';
      want.forEach(function (t) {
        var o = document.createElement('option');
        o.value = t; o.textContent = (t === '' ? '（空白）' : t);
        sel.appendChild(o);
      });
    }
    sel.value = val;
  }

  // --- drawing (golden TTMyTray) ---------------------------------------------------------------
  function geometry(s, W, H) {
    var nx = clampInt(s.xItem, 1, MAX_CELLS, DFM_XITEM), ny = clampInt(s.yItem, 1, MAX_CELLS, DFM_YITEM);
    var xb = clampInt(s.xbItem, 1, 2, 1), yb = clampInt(s.ybItem, 1, 5, 1);            // SetBlockXItem :143 / SetBlockYItem :165
    var xbw = toInt(s.xbWidth), ybw = toInt(s.ybWidth);
    if (!(xbw > 0)) xbw = 1;                                                              // SetXBlockWidth :187
    if (!(ybw > 0)) ybw = 1;
    var cw = Math.trunc((W - 2 - (nx - 1) - 2 - (xb - 1) * xbw) / nx);                   // CaculateTrayParameter :264
    var ch = Math.trunc((H - 2 - (ny - 1) - 2 - (yb - 1) * ybw) / ny);                   // :265
    if (cw < 1) cw = 1;
    if (ch < 1) ch = 1;
    return {
      nx: nx, ny: ny, cw: cw, ch: ch, W: W, H: H, xbw: xbw, ybw: ybw,
      sx: Math.trunc((W - cw * nx - (nx - 1) - (xb - 1) * xbw) / 2),                     // :266
      sy: Math.trunc((H - ch * ny - (ny - 1) - (yb - 1) * ybw) / 2),                     // :267
      px: cw + 1, py: ch + 1,                                                             // :268-269 pitch = width + LineWidth
      bsx: Math.max(1, Math.trunc(nx / xb)), bsy: Math.max(1, Math.trunc(ny / yb))        // DrawSingleIC :241-242 X / (FXItem / FXBlock)
    };
  }
  function cellLeft(g, x) { return g.sx + x * g.px + Math.trunc(x / g.bsx) * g.xbw; }     // DrawSingleIC :245
  function cellTop(g, y) { return g.sy + y * g.py + Math.trunc(y / g.bsy) * g.ybw; }      // :246

  function canvasSize(s) {
    var nx = clampInt(s.xItem, 1, MAX_CELLS, DFM_XITEM), ny = clampInt(s.yItem, 1, MAX_CELLS, DFM_YITEM);
    var xb = clampInt(s.xbItem, 1, 2, 1), yb = clampInt(s.ybItem, 1, 5, 1);
    var xbw = toInt(s.xbWidth) > 0 ? toInt(s.xbWidth) : 1, ybw = toInt(s.ybWidth) > 0 ? toInt(s.ybWidth) : 1;
    var minW = nx * (MIN_CELL_PX + 1) + 4 + xb * xbw, minH = ny * (MIN_CELL_PX + 1) + 4 + yb * ybw;
    var wrap = $('teWrap'), below = $('teBelow');
    var availW = (wrap && wrap.clientWidth) || GOLD_W;
    var top = wrap ? wrap.getBoundingClientRect().top : 0;
    var availH = Math.floor((window.innerHeight || 0) - top - (below ? below.offsetHeight : 0) - 16);
    var W = Math.max(minW, Math.floor(availW));
    var H = Math.round(W * GOLD_H / GOLD_W);                          // golden 491 x 567 proportions when there is room
    if (availH > 0 && H > availH) H = availH;
    if (H < Math.max(minH, 120)) H = Math.max(minH, 120);
    return { W: W, H: H };
  }
  function paletteOf(s) {
    var p = [], i;
    for (i = 0; i < MAX_COLOR_INDEX; i++) p.push(i < PALETTE.length ? PALETTE[i] : '#000000');
    var cm = s && s.colorMap;
    if (cm && typeof cm === 'object') {
      Object.keys(cm).forEach(function (k) {
        var n = toInt(k);
        if (n >= 0 && n < MAX_COLOR_INDEX && typeof cm[k] === 'string' && HEX.test(cm[k])) p[n] = cm[k];
      });
    }
    return p;
  }
  function darkFill(css) {
    var r = parseInt(css.substr(1, 2), 16), g = parseInt(css.substr(3, 2), 16), b = parseInt(css.substr(5, 2), 16);
    return (0.299 * r + 0.587 * g + 0.114 * b) < 110;
  }
  function inDrag(x, y) {
    if (!drag) return false;
    return x >= Math.min(drag.x0, drag.x1) && x <= Math.max(drag.x0, drag.x1) &&
           y >= Math.min(drag.y0, drag.y1) && y <= Math.max(drag.y0, drag.y1);
  }

  function requestDraw() {
    if (rafPending) return;
    rafPending = true;
    (window.requestAnimationFrame || function (f) { return setTimeout(f, 16); })(function () { rafPending = false; draw(); });
  }
  function draw() {
    var cv = $('mtLoaderBuffer');
    if (!cv || !cv.getContext) return;
    var s = S || { xItem: DFM_XITEM, yItem: DFM_YITEM };
    var sz = canvasSize(s), g = geometry(s, sz.W, sz.H);
    if (drag && (drag.x0 >= g.nx || drag.x1 >= g.nx || drag.y0 >= g.ny || drag.y1 >= g.ny)) drag = null;   // tray changed under the drag
    var cwPx = Math.max(sz.W, cellLeft(g, g.nx - 1) + g.cw + 1);     // golden odd block counts can reach past Width
    var chPx = Math.max(sz.H, cellTop(g, g.ny - 1) + g.ch + 1);
    var dpr = window.devicePixelRatio || 1;
    var colors = (S && S.color && S.color.length != null) ? S.color : null;
    var texts = (S && S.number && S.number.length != null) ? S.number : null;
    var sig = [cwPx, chPx, dpr, g.nx, g.ny, g.xbw, g.ybw, g.bsx, g.bsy, JSON.stringify(colors), JSON.stringify(texts),
               JSON.stringify(S && S.colorMap), S ? 1 : 0, drag ? [drag.x0, drag.y0, drag.x1, drag.y1].join(',') : ''].join('|');
    G = g; G.cwPx = cwPx; G.chPx = chPx;
    if (sig === lastSig) return;
    lastSig = sig;
    st.renders++;

    if (cv._w !== cwPx || cv._h !== chPx || cv._dpr !== dpr) {
      cv.width = Math.round(cwPx * dpr); cv.height = Math.round(chPx * dpr);
      cv.style.width = cwPx + 'px'; cv.style.height = chPx + 'px';
      cv._w = cwPx; cv._h = chPx; cv._dpr = dpr;
    }
    var ctx = cv.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.fillStyle = '#FFFFFF';                                        // FTrayColor = clWhite
    ctx.fillRect(0, 0, cwPx, chPx);
    ctx.lineWidth = 1;
    ctx.strokeStyle = '#000000';                                      // FFrameColor = clBlack
    ctx.strokeRect(0.5, 0.5, sz.W - 1, sz.H - 1);                     // DrawTray Rectangle(0, 0, Width, Height)
    ctx.beginPath();                                                  // csLeftTop direction mark (DrawTray :284-295)
    ctx.moveTo(0, 0); ctx.lineTo(DIRECT_WIDTH, 0); ctx.lineTo(0, DIRECT_WIDTH); ctx.closePath();
    ctx.fillStyle = '#000000'; ctx.fill();

    var pal = paletteOf(S), stride = g.nx;
    var fs = Math.min(Math.round(11 * Math.min(1.6, Math.max(1, sz.W / GOLD_W))), g.ch - 2);   // dfm Font.Height = -11
    var withText = !!texts && fs >= 6;
    if (withText) { ctx.font = fs + 'px Tahoma, "MS Sans Serif", sans-serif'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'; }
    for (var y = 0; y < g.ny; y++) {
      var T = cellTop(g, y);
      for (var x = 0; x < g.nx; x++) {
        var L = cellLeft(g, x), i = y * stride + x, fill;
        if (inDrag(x, y)) fill = pal[PREVIEW_INDEX];                  // golden ShowTray :409
        else if (!S) fill = pal[0];
        else {
          var c = colors ? colors[i] : null;
          if (typeof c === 'number' && c === Math.floor(c) && c >= 0 && c < MAX_COLOR_INDEX) fill = pal[c];
          else fill = UNKNOWN_CSS;                                    // no value from C++ (golden never paints outside 0..31)
        }
        ctx.fillStyle = fill;                                         // DrawSingleIC Rectangle: brush = ColorMap[idx], pen = frame
        ctx.fillRect(L, T, g.cw, g.ch);
        if (g.cw >= 2 && g.ch >= 2) ctx.strokeRect(L + 0.5, T + 0.5, g.cw - 1, g.ch - 1);
        if (withText) {
          var t = texts[i];
          if (t != null && t !== '') {
            ctx.save();
            ctx.beginPath(); ctx.rect(L + 1, T + 1, g.cw - 2, g.ch - 2); ctx.clip();   // DrawText clips to the cell
            ctx.fillStyle = darkFill(fill) ? '#FFFFFF' : '#000000';
            ctx.fillText(String(t), L + g.cw / 2, T + g.ch / 2 + 0.5);
            ctx.restore();
            ctx.strokeStyle = '#000000';
          }
        }
      }
    }
  }

  // --- pointer -> cell ---------------------------------------------------------------------------
  function axisAt(p, n, startOf, pitch) {
    for (var i = 0; i < n; i++) {
      var a = startOf(i);
      if (p < a) return -1;                                           // before the cell = block gap or outside
      if (p < a + pitch) return i;                                    // the cell plus its grid line
    }
    return -1;
  }
  function cellFromEvent(ev) {
    var cv = $('mtLoaderBuffer');
    if (!cv || !G) return null;
    var r = cv.getBoundingClientRect();
    if (!r.width || !r.height) return null;
    var px = (ev.clientX - r.left) * (G.cwPx / r.width), py = (ev.clientY - r.top) * (G.chPx / r.height);
    var x = axisAt(px, G.nx, function (i) { return cellLeft(G, i); }, G.px);
    var y = axisAt(py, G.ny, function (i) { return cellTop(G, i); }, G.py);
    return (x < 0 || y < 0) ? null : { x: x, y: y };
  }

  function bindGrid() {
    var cv = $('mtLoaderBuffer');
    if (!cv) return;
    cv.addEventListener('contextmenu', function (ev) { ev.preventDefault(); });
    cv.addEventListener('pointerdown', function (ev) {                // golden mtLoaderBufferMouseDown :257 (any button)
      if (drag || !S || S.open !== true) return;
      var c = cellFromEvent(ev);
      if (!c) return;                                                 // :302-303 outside the grid -> nothing
      ev.preventDefault();
      drag = { x0: c.x, y0: c.y, x1: c.x, y1: c.y, moved: false, id: ev.pointerId };
      try { cv.setPointerCapture(ev.pointerId); } catch (e) { /* capture is only a convenience */ }
      renderCaption(); requestDraw();
    });
    cv.addEventListener('pointermove', function (ev) {                // :316-331
      if (!drag || ev.pointerId !== drag.id) return;
      var c = cellFromEvent(ev);
      if (!c) return;                                                 // :325-326 outside -> the end cell stays
      drag.moved = true;
      if (c.x !== drag.x1 || c.y !== drag.y1) { drag.x1 = c.x; drag.y1 = c.y; renderCaption(); requestDraw(); }
    });
    cv.addEventListener('pointerup', function (ev) {                  // :333-344 -> SetTray in C++
      if (!drag || ev.pointerId !== drag.id) return;
      var d = drag; drag = null;
      try { cv.releasePointerCapture(ev.pointerId); } catch (e) {}
      renderCaption(); requestDraw();
      enqueue({ op: 'select', x0: d.x0, y0: d.y0, x1: d.x1, y1: d.y1, moved: d.moved },
              'Select (' + d.x0 + ',' + d.y0 + ')...(' + d.x1 + ',' + d.y1 + ')');
    });
    cv.addEventListener('pointercancel', function (ev) {
      if (!drag || ev.pointerId !== drag.id) return;
      drag = null; renderCaption(); requestDraw();
      sayOp('拖曳被中斷，沒有送出', 'note');
    });
  }

  // --- buttons ---------------------------------------------------------------------------------
  function bindControls() {
    function on(id, evn, fn) { var el = $(id); if (el) el.addEventListener(evn, fn); }
    on('spbUpdate', 'click', function () { enqueue({ op: 'update' }, 'Update'); });
    on('SpeedButton2', 'click', function () { enqueue({ op: 'cancel' }, 'Abort'); });
    on('Button2', 'click', function () { enqueue({ op: 'snapshot' }, 'Button2（存畫面）'); });
    on('Button1', 'click', function () {
      var ex = $('edtXPos'), ey = $('edtYPos');
      enqueue({ op: 'fill', x: ex ? ex.value : '', y: ey ? ey.value : '' }, 'Manual Input');
    });
    on('CheckBox1', 'change', function (ev) { enqueue({ op: 'noSave', value: !!ev.target.checked }, 'No Save Image'); });
    on('cbBinCount', 'change', function (ev) { enqueue({ op: 'binCount', value: String(ev.target.value) }, 'cbBinCount = ' + ev.target.value); });
    ['edtXPos', 'edtYPos'].forEach(function (id) {                   // golden edtXPosClick :664-667 ShowQwertyKey(N_INTEGER, 0, true, 100, 0)
      on(id, 'click', function (ev) {
        if (ev.target.disabled || !window.HTQwerty) return;
        HTQwerty.show(ev.target, HTQwerty.N.INTEGER, { dp: 0, checkRange: true, min: 100, max: 0 });
      });
    });
  }

  // --- window lifecycle ------------------------------------------------------------------------
  window.addEventListener('message', function (ev) {                  // background.html postWinState
    var m = ev && ev.data;
    if (!m || m.type !== 'HT_WIN') return;
    var prev = winOpen;
    winOpen = !!m.open;
    if (winOpen && prev !== true) { closedByState = false; pollDelay = POLL_MS; pollNow(); }
    if (!winOpen) { if (drag) { drag = null; requestDraw(); } stopPoll(); }
  });
  function watch() {
    var v = visible();
    if (v && !wasVisible) { closedByState = false; pollDelay = POLL_MS; pollNow(); requestDraw(); }   // shown again = reopened
    if (!v && wasVisible) { if (drag) { drag = null; requestDraw(); } stopPoll(); }
    wasVisible = v;
  }
  document.addEventListener('visibilitychange', watch);
  window.addEventListener('resize', requestDraw);

  function start() {
    bindGrid();
    bindControls();
    renderControls();
    draw();
    wasVisible = visible();
    setInterval(watch, WATCH_MS);
    if (active()) pollNow();
  }

  window.HT9045TrayEdit = {
    state: function () { return S; },
    last: function () { return lastReply; },
    lastError: function () { return lastError; },
    busy: function () { return !!inflight || queue.length > 0; },
    stats: function () { var o = {}, k; for (k in st) o[k] = st[k]; o.polling = !!pollTimer; o.closedByState = closedByState; o.winOpen = winOpen; return o; },
    poll: function () { closedByState = false; pollNow(); },
    select: function (x0, y0, x1, y1) { return enqueue({ op: 'select', x0: x0, y0: y0, x1: x1, y1: y1, moved: !(x0 === x1 && y0 === y1) }, 'Select (' + x0 + ',' + y0 + ')...(' + x1 + ',' + y1 + ')'); },
    update: function () { return enqueue({ op: 'update' }, 'Update'); },
    cancel: function () { return enqueue({ op: 'cancel' }, 'Abort'); },
    fill: function (x, y) { return enqueue({ op: 'fill', x: String(x), y: String(y) }, 'Manual Input'); },
    snapshot: function () { return enqueue({ op: 'snapshot' }, 'Button2（存畫面）'); },
    noSave: function (b) { return enqueue({ op: 'noSave', value: !!b }, 'No Save Image'); },
    binCount: function (v) { return enqueue({ op: 'binCount', value: String(v) }, 'cbBinCount = ' + v); }
  };

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();
