/* AI(W906-HTDESIGNER) 20260930: CSV 表格 -- a .csv as a table, edited like Excel.
 * EastSun: "你外掛軟體可以包含開啟CSV時可以像excel有表格可以編輯嗎?"
 * The extension parses the file and writes every change as a replacement of only the cells that
 * changed (lib/csvtable.js), into VS Code's document: Ctrl+S saves, Ctrl+Z undoes.
 * Keys like Excel: arrows / Tab / Enter move, typing or F2 edits, Esc cancels, Delete clears,
 * Ctrl+C / Ctrl+V copy / paste (tab separated, to and from Excel), Ctrl+A all, Ctrl+F find.
 * Only the rows in sight are drawn (a log with 100 000 lines stays quick).
 *
 * AI(W906-HTDESIGNER) 20261001 (EastSun: "你看一下網路上 手冊還有哪些操作…目前操作後的結果跟網路上結果不一樣"):
 * checked against Excel's own help (support.microsoft.com "Keyboard shortcuts in Excel", "Enter data manually",
 * "Edit cell contents", "Find or replace", "Move or copy cells"):
 *   - the ACTIVE cell is where the selection started (Excel), not its far corner: typing, F2 and Ctrl+Enter go
 *     there; Enter / Tab move it inside a selected block; Tab..Tab then Enter goes back to the first column
 *   - Ctrl+Enter = the typed value into every selected cell; Ctrl+D / Ctrl+R = fill down / right
 *   - a copied cell pasted on a block fills the block (a block a whole number of times: repeated)
 *   - Ctrl+X moves the cells when they are pasted (not cleared at once); Esc forgets the cut
 *   - typed over a cell: all four arrows write it and move (Enter mode); F2: the caret at the end, F2 again
 *     toggles Enter / Edit mode; a double click puts the caret where it was clicked
 *   - Shift+Space / Ctrl+Space = the whole rows / columns (a Shift+Space used to type a space into the cell)
 *   - Ctrl+- deletes rows only when whole rows are selected (it used to take the row of any one cell)
 *   - Backspace = edit the active cell from empty (Esc gives it back); Delete clears the selection
 *   - Ctrl+arrow = the edge of the data (stops at empty cells); Ctrl+H = replace (whole cell, in the selection)
 *   - an empty row under the last one: typing there adds a row
 *   - the status bar: the row's first two columns (which axis), sum / min / max of the numbers selected */
(function () {
  'use strict';
  var api = acquireVsCodeApi();
  var st = api.getState() || {};
  var RH = 22;                 /* row height */
  var RNW = 52;                /* the row-number column */
  var D = { rows: [], readOnly: false, why: '', file: '', delim: ',' };
  var header = st.header;      /* the first row is the column names (frozen): undefined = guess */
  /* 1006 (Excel's Freeze Panes): the first FREEZE columns stay at the left when it scrolls sideways (Mot_Table: the axis name) */
  var FREEZE = Math.max(0, st.freeze | 0);
  var widths = st.widths || {};
  /* the selection is the block anchor..cur; act = the active cell inside it (Excel's white cell) */
  var cur = { r: 0, c: 0 }, anchor = { r: 0, c: 0 }, act = { r: 0, c: 0 };
  var editing = null;          /* { r, c, orig, typed } -- the editor is `kb`; typed = Enter mode */
  var find = { q: '', hits: [], i: -1, whole: false, rect: null };
  var cut = null;              /* { r0, c0, r1, c1, text }: Ctrl+X, moved when pasted */
  var tabStart = null;         /* the column a Tab..Tab run started in (Enter goes back to it) */
  var ncols = 1;
  /* 1006 AutoFilter (Excel: Data > Filter, Alt+Down on a title, Ctrl+Shift+L): flt[c] = { v: values let through };
     V = the body rows in sight in order (null = no filter; the empty row under the last one is in it), P = row -> its
     place in V. keep = rows held in sight although they no longer pass (Excel filters again only when asked: an edit
     does not make its row vanish; Ctrl+Alt+L = filter again); shifts = the row inserts / deletes on their way, so the
     rows held follow them */
  var flt = {}, V = null, P = null, keep = null, shifts = null, noCarry = false;
  /* 1006 (Excel: Conditional Formatting > Duplicate Values): DUP = { c, n: value -> how many rows have it, groups, cells }
     of one column; the cells whose value is in two rows or more are marked (an empty one never) */
  var DUP = null;
  function dupOf(c) {
    var n = Object.create(null), groups = 0, cells = 0;
    for (var r = firstBody(); r < nrows(); r++) { var v = cell(r, c); if (v.trim() !== '') n[v] = (n[v] || 0) + 1; }
    for (var k in n) if (n[k] > 1) { groups++; cells += n[k]; }
    return { c: c, n: n, groups: groups, cells: cells };
  }
  function toggleDup(c) { DUP = DUP && DUP.c === c ? null : dupOf(c); draw(); }
  /* 1006 (Excel's error checking, the green triangle): in a column whose values are numbers (4 or more filled, 80 % of
     them numbers) the filled cells that are not are flagged -- an orange corner, a tooltip, a count in the status bar;
     the menu's 下一個可疑的格子 goes to them in turn. A hint only: nothing is changed. ODD = { at: 'r,c' -> the
     column's number count, list: [{ r, c }] row by row } */
  var ODD = { at: Object.create(null), list: [] };
  function oddOf() {
    var at = Object.create(null), list = [], r0 = firstBody(), r1 = Math.min(nrows(), r0 + 20000);
    for (var c = 0; c < ncols; c++) {
      var k = 0, nn = 0, bad = [];
      for (var r = r0; r < r1; r++) { var v = cell(r, c).trim(); if (v === '') continue; k++; if (isNum(v)) nn++; else bad.push(r); }
      if (k < 4 || !bad.length || nn < k * 0.8) continue;
      bad.forEach(function (r) { at[r + ',' + c] = nn; list.push({ r: r, c: c }); });
    }
    list.sort(function (a, b) { return a.r - b.r || a.c - b.c; });
    return { at: at, list: list };
  }
  function nextOdd() {
    if (!ODD.list.length) return;
    /* (1006 audit: the ones a filter hides are skipped -- it used to stop on the first hidden one) */
    var vis = ODD.list.filter(function (o) { return !V || V.indexOf(o.r) >= 0; });
    if (!vis.length) { showWarn('可疑的 ' + ODD.list.length + ' 格都被篩選藏起來了'); return; }
    var n = vis.filter(function (o) { return o.r > act.r || (o.r === act.r && o.c > act.c); })[0] || vis[0];
    go(n.r, n.c);
  }

  var root = document.getElementById('root');
  root.innerHTML =
    '<div class="bar">' +
      '<input id="nameBox" class="namebox" type="text" spellcheck="false" autocomplete="off" aria-label="名稱方塊" ' +
        'title="名稱方塊（Excel）：目前的格子。打 C12、B2:D5 或列號（12）再按 Enter＝跳過去並選取；Ctrl+G 也會到這裡">' +
      '<span id="name" class="name"></span><span id="size" class="size"></span>' +
      '<label class="chk" title="第一列是欄位名稱：固定在上面，捲動時一直看得到"><input type="checkbox" id="hdr">第一列是標題</label>' +
      '<button id="insRow" title="在目前這列上面插入一列（Ctrl+Shift+＋）">插入列</button>' +
      '<button id="delRow" title="刪除選取的列（選整列後 Ctrl+－）">刪除列</button>' +
      '<button id="fltClr" hidden title="清除所有欄的篩選，全部的列都顯示（Ctrl+Shift+L）">清除篩選</button>' +
      '<span class="sp"></span>' +
      '<input id="find" type="text" placeholder="尋找（Ctrl+F，Enter 下一個，Shift+Enter 上一個）" spellcheck="false">' +
      '<span id="findN" class="findN"></span>' +
      '<button id="replTog" title="取代（Ctrl+H）">取代…</button>' +
      '<button id="asText" title="用文字編輯器開這個檔">以文字開啟</button>' +
    '</div>' +
    '<div id="repl" class="bar repl" hidden>' +
      '<input id="replBox" type="text" placeholder="取代為" spellcheck="false">' +
      '<label class="chk" title="只找整格就是這個值的（取代「1」不會改到 10、21）"><input type="checkbox" id="whole">整格相符</label>' +
      '<label class="chk" id="inSelL" title="只在選取的範圍裡找、取代"><input type="checkbox" id="inSel">只在選取範圍<span id="inSelR"></span></label>' +
      '<button id="replOne" title="取代目前這一格，再找下一個">取代</button>' +
      '<button id="replAll" title="一次取代全部（一個 Ctrl+Z 全部復原）">全部取代</button>' +
      '<span id="replN" class="findN"></span>' +
    '</div>' +
    '<div id="warn" class="warn" hidden></div>' +
    '<div id="wrap" class="wrap" tabindex="-1"><div id="sizer" class="sizer"><div id="grid"></div>' +
      '<input id="kb" class="ed idle" type="text" spellcheck="false" autocomplete="off" aria-label="儲存格"></div></div>' +
    '<div id="status" class="status"></div>';
  var wrap = document.getElementById('wrap'), sizer = document.getElementById('sizer'), grid = document.getElementById('grid');
  var hdrBox = document.getElementById('hdr'), findBox = document.getElementById('find');
  var replBar = document.getElementById('repl'), replBox = document.getElementById('replBox');
  var wholeBox = document.getElementById('whole'), inSelBox = document.getElementById('inSel');
  var nameBox = document.getElementById('nameBox');
  /* AI(W906-HTDESIGNER) 20261001: ONE input takes every key (EastSun 20261001 "表格我沒辦法寫入").
   * Idle it sits, invisible, on the active cell and has the focus; a character typed -- or the IME
   * (注音／倉頡) starting a word -- goes into it, and that makes it the cell's editor (Excel: typing
   * replaces the cell), so a Chinese word goes into a cell too and the first key is never lost.
   * Before, the grid (a div) had the focus: a div takes no IME, so with a Chinese input method on
   * typing did nothing at all. */
  var kb = document.getElementById('kb');
  var composing = false;

  function colName(c) { var s = ''; c++; while (c > 0) { var m = (c - 1) % 26; s = String.fromCharCode(65 + m) + s; c = Math.floor((c - 1) / 26); } return s; }
  function cell(r, c) { var row = D.rows[r]; return row && c < row.length ? row[c] : ''; }
  function firstBody() { return header ? 1 : 0; }
  function nrows() { return D.rows.length; }
  /* the last row one can go to: the empty one under the data (typing there adds a row), none when read only */
  function lastRow() { return D.readOnly ? nrows() - 1 : nrows(); }
  function colW(c) { return widths[c] || 80; }
  /* (1006 audit, performance: during a draw the columns' places come from one table -- every cell used to add up the
     widths from column A: a 1002-column file took 260-380 ms a scroll step) */
  var COLX = null;
  function colX(c) {
    if (COLX) return COLX[Math.max(0, Math.min(c, COLX.length - 1))];
    var x = RNW; for (var i = 0; i < c; i++) x += colW(i); return x;
  }
  /* the columns to draw: the frozen ones and the ones in sight (+ a margin) -- not all of them */
  function colsInSight() {
    var out = [], i;
    for (i = 0; i < Math.min(FREEZE, ncols); i++) out.push(i);
    var x0 = wrap.scrollLeft - 400, x1 = wrap.scrollLeft + (wrap.clientWidth || 1200) + 400;
    var lo = 0, hi = ncols;   /* (the first column whose right edge is past x0) */
    while (lo < hi) { var md = (lo + hi) >> 1; if (COLX[md + 1] <= x0) lo = md + 1; else hi = md; }
    for (i = Math.max(lo, FREEZE); i < ncols && COLX[i] < x1; i++) out.push(i);
    return out;
  }
  /* where column c is drawn: a frozen one moves along with the sideways scroll (it stays in sight) */
  function cellX(c) { return colX(c) + (c < FREEZE ? wrap.scrollLeft : 0); }
  function frozenW() { var w = 0; for (var i = 0; i < Math.min(FREEZE, ncols); i++) w += colW(i); return w; }
  function setFreeze(n) { FREEZE = Math.max(0, n | 0); save(); scrollTo(act.r, act.c); draw(); }
  function totalW() { if (COLX) return COLX[COLX.length - 1]; var x = RNW; for (var i = 0; i < ncols; i++) x += colW(i); return x; }
  /* the place of row r among the rows drawn (a row filtered out: the place of the next one in sight) */
  function idxOf(r) {
    if (!V) return r - firstBody();
    if (P[r] != null) return P[r];
    var lo = 0, hi = V.length;
    while (lo < hi) { var mid = (lo + hi) >> 1; if (V[mid] < r) lo = mid + 1; else hi = mid; }
    return lo;
  }
  function rowAt(i) { return V ? (V.length ? V[clamp(i, 0, V.length - 1)] : firstBody()) : i + firstBody(); }
  function viewLen() { return V ? V.length : Math.max(0, lastRow() - firstBody() + 1); }
  function rowY(r) { return idxOf(r) * RH + RH * (header ? 2 : 1); }
  /** row r is in sight (not filtered out); the title row always is */
  function shown(r) { return !V || (header && r === 0) || P[r] != null; }
  /** d rows on from r, counting only the rows in sight (no filter: r + d, clamped later) */
  function stepRow(r, d) {
    if (!V) return r + d;
    if (header && r === 0) return d <= 0 ? 0 : V.length ? rowAt(d - 1) : 0;
    var i = (P[r] != null ? P[r] : d > 0 ? idxOf(r) - 1 : idxOf(r)) + d;
    if (i < 0) return header ? 0 : rowAt(0);
    return rowAt(Math.min(i, V.length - 1));
  }
  /* the first / last row in sight inside s (s.r0 / s.r1 when none is) */
  function firstIn(s) { if (shown(s.r0)) return s.r0; var x = stepRow(s.r0, 1); return x >= s.r0 && x <= s.r1 ? x : s.r0; }
  function lastIn(s) { if (shown(s.r1)) return s.r1; var x = stepRow(s.r1, -1); return x >= s.r0 && x <= s.r1 ? x : s.r1; }
  /** rows r0..r1 of the data have one filtered out */
  function hiddenIn(r0, r1) {
    if (!V) return false;
    for (var r = Math.max(r0, firstBody()); r <= Math.min(r1, nrows() - 1); r++) if (P[r] == null) return true;
    return false;
  }
  function passes(r) { for (var k in flt) if (flt[k].v[cell(r, +k)] !== 1) return false; return true; }
  function rebuildView() {
    if (!Object.keys(flt).length) { V = null; P = null; keep = null; sizeText(); return; }
    V = []; P = {};
    for (var r = firstBody(); r < nrows(); r++) if (passes(r) || (keep && keep[r])) { P[r] = V.length; V.push(r); }
    V.n = nrows();
    if (!D.readOnly) { P[nrows()] = V.length; V.push(nrows()); }
    sizeText();
  }
  /* new data (an edit came back): the rows in sight stay in sight -- moved by the inserts / deletes this table sent */
  function carryKeep() {
    if (!V) return;
    var oldN = V.n, k = {}, sh = shifts || [];
    shifts = null;
    /* (the inserts / deletes did not land -- not written, or something else changed the file: no shifting) */
    if (sh.length && oldN + sh.reduce(function (a, s) { return a + s.n; }, 0) !== nrows()) sh = [];
    if (!sh.length && nrows() < oldN) { keep = null; return; }   /* (rows gone by an undo / outside: filtered afresh) */
    V.forEach(function (r) {
      if (r >= oldN) return;
      for (var i = 0; i < sh.length; i++) {
        var s = sh[i];
        if (s.n > 0) { if (r >= s.r) r += s.n; }
        else if (r >= s.r && r < s.r - s.n) return;
        else if (r >= s.r - s.n) r += s.n;
      }
      k[r] = 1;
    });
    sh.forEach(function (s) { if (s.n > 0) for (var j = 0; j < s.n; j++) k[s.r + j] = 1; });
    if (!sh.length) for (var n = oldN; n < nrows(); n++) k[n] = 1;   /* typed / pasted under the last row */
    keep = k;
  }
  function sizeText() {
    var el = document.getElementById('size');
    if (!el) return;
    var body = Math.max(0, nrows() - firstBody());
    var n = V ? V.filter(function (r) { return r < nrows(); }).length : body;
    el.textContent = nrows() + ' 列 × ' + ncols + ' 欄　分隔：' + (D.delim === '\t' ? 'Tab' : D.delim) +
      (V ? '　｜篩選：顯示 ' + n + ' / ' + body + ' 列（' + Object.keys(flt).map(function (c) { return colName(+c); }).join('、') + ' 欄）' : '');
    var b = document.getElementById('fltClr');
    if (b) b.hidden = !V;
  }
  function refilter() { rebuildView(); clampCur(); scrollTo(act.r, act.c); draw(); }
  function isNum(v) { return /^[-+]?\d+(\.\d+)?$/.test(v); }
  function guessHeader() {
    if (D.rows.length < 2) return false;
    var a = D.rows[0], b = D.rows[1];
    var textual = a.filter(function (v) { return v !== ''; }).every(function (v) { return !isNum(v); });
    return textual && a.some(function (v) { return v !== ''; }) && b.some(function (v) { return /^[-+]?\d/.test(v); });
  }
  function autoWidths() {
    var n = Math.min(D.rows.length, 400);
    for (var c = 0; c < ncols; c++) {
      if (widths[c]) continue;
      var mx = colName(c).length + 1;
      for (var r = 0; r < n; r++) { var v = cell(r, c); if (v.length > mx) mx = v.length; }
      widths[c] = Math.max(44, Math.min(320, Math.round(mx * 7.2 + 18)));
    }
  }
  function save() { api.setState({ header: header, widths: widths, cur: act, freeze: FREEZE }); }
  function warnRo() { showWarn(D.why || '這個檔案現在不能編輯。'); }

  /* --- drawing: only what is in sight --- */
  var drawn = { a: -1, b: -1 };
  function draw() {
    COLX = [RNW];
    for (var ci = 0; ci < ncols; ci++) COLX.push(COLX[ci] + colW(ci));
    try { drawRows(); } finally { COLX = null; }
  }
  function drawRows() {
    var top = wrap.scrollTop, h = wrap.clientHeight || 400;
    var COLS = colsInSight();
    /* (a, b: places among the rows drawn -- with a filter on, only the rows in sight have one) */
    var hh = RH * (header ? 2 : 1), L = viewLen();
    var a = Math.max(0, Math.floor((top - hh) / RH) - 8);
    var b = Math.min(L - 1, a + Math.ceil(h / RH) + 16);
    sizer.style.height = (hh + Math.max(L, 1) * RH + RH) + 'px';
    sizer.style.width = (totalW() + 40) + 'px';
    var html = [];
    /* the column letters (and the frozen first row) stay on top */
    html.push('<div class="hrow letters" style="top:' + top + 'px;width:' + totalW() + 'px">');
    html.push('<div class="rn corner" style="left:' + wrap.scrollLeft + 'px" data-all="1"></div>');
    COLS.forEach(function (c) { html.push('<div class="hc' + (inSelC(c) ? ' on' : '') + (flt[c] ? ' flt' : '') + (c < FREEZE ? ' fz' + (c === FREEZE - 1 ? ' fzl' : '') : '') + '" data-col="' + c + '" style="left:' + cellX(c) + 'px;width:' + colW(c) + 'px">' + colName(c) +
      '<span class="fb' + (flt[c] ? ' on' : '') + '" data-fb="' + c + '" title="' + (flt[c] ? '這一欄有篩選——點這裡改或清除' : '篩選這一欄（Alt+↓）') + '">' + (flt[c] ? '▼' : '▾') + '</span>' +
      '<span class="rs" data-rs="' + c + '"></span></div>'); });
    html.push('</div>');
    if (header && nrows()) html.push(rowHtml(0, top + RH, true, COLS));
    for (var i = a; i <= b; i++) { var r = rowAt(i); html.push(rowHtml(r, rowY(r), false, COLS)); }
    grid.innerHTML = html.join('');   /* (the cell editor lives beside the grid: a redraw keeps it) */
    drawn = { a: a, b: b };
    placeKb();
    status();
  }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }
  function inSel(r, c) {
    return r >= Math.min(cur.r, anchor.r) && r <= Math.max(cur.r, anchor.r) && c >= Math.min(cur.c, anchor.c) && c <= Math.max(cur.c, anchor.c);
  }
  function inSelC(c) { return c >= Math.min(cur.c, anchor.c) && c <= Math.max(cur.c, anchor.c); }
  function inSelR(r) { return r >= Math.min(cur.r, anchor.r) && r <= Math.max(cur.r, anchor.r); }
  function inRect(s, r, c) { return !!s && r >= s.r0 && r <= s.r1 && c >= s.c0 && c <= s.c1; }
  function rowHtml(r, y, frozen, COLS) {
    var phantom = r >= nrows();
    var s = ['<div class="row' + (frozen ? ' frozen' : '') + (phantom ? ' phantom' : '') + '" style="top:' + y + 'px;width:' + totalW() + 'px">'];
    s.push('<div class="rn' + (inSelR(r) ? ' on' : '') + (V && !frozen ? ' flt' : '') + '" data-row="' + r + '" style="left:' + wrap.scrollLeft + 'px"' +
      (phantom ? ' title="最後一列下面的空白列：在這裡打字＝新增一列"' : '') + '>' + (phantom ? '＋' : r + 1) + '</div>');
    var row = D.rows[r] || [];
    var cs = COLS || null;
    for (var ck = 0; ck < (cs ? cs.length : ncols); ck++) {
      var c = cs ? cs[ck] : ck;
      var v = c < row.length ? row[c] : '';
      var cls = 'cell' + (c >= row.length ? ' none' : '') + (inSel(r, c) ? ' sel' : '') + (r === act.r && c === act.c ? ' cur' : '') +
        (inRect(cut, r, c) ? ' cut' : '') + (find.q && match(v) ? ' hit' : '') + (isNum(v) ? ' num' : '') +
        (DUP && c === DUP.c && r >= firstBody() && v.trim() !== '' && DUP.n[v] > 1 ? ' dup' : '') +
        (ODD.at[r + ',' + c] ? ' odd' : '');
      if (c < FREEZE) cls += ' fz' + (c === FREEZE - 1 ? ' fzl' : '');
      s.push('<div class="' + cls + '" data-r="' + r + '" data-c="' + c + '" style="left:' + cellX(c) + 'px;width:' + colW(c) + 'px" title="' + esc((ODD.at[r + ',' + c] ? '⚠ 這一欄其他 ' + ODD.at[r + ',' + c] + ' 格都是數字，這格不是——是不是打錯？\n' : '') + v).replace(/"/g, '&quot;') + '">' + esc(v) + '</div>');
    }
    s.push('</div>');
    return s.join('');
  }
  function fmtNum(x) { return Math.round(x * 1e6) / 1e6 + ''; }
  function status() {
    var el = document.getElementById('status');
    var s = selRect(), n = (s.r1 - s.r0 + 1) * (s.c1 - s.c0 + 1);
    var name = header && D.rows[0] && D.rows[0][act.c] ? '（' + D.rows[0][act.c] + '）' : '';
    /* which row it is: its first two columns (Mot_Table: Motorname + Alias -- the axis) */
    var who = act.r >= firstBody() && act.r < nrows() ? [cell(act.r, 0), cell(act.r, 1)].filter(Boolean).join(' ') : '';
    if (who.length > 40) who = who.slice(0, 40) + '…';
    var sum = '';
    if (n > 1 && n <= 200000) {
      var k = 0, tot = 0, mn = Infinity, mx = -Infinity;
      for (var r = s.r0; r <= s.r1; r++) for (var c = s.c0; c <= s.c1; c++) {
        if (!shown(r)) break;   /* (Excel: the numbers of the rows in sight) */
        var v = cell(r, c);
        if (isNum(v)) { var x = +v; k++; tot += x; if (x < mn) mn = x; if (x > mx) mx = x; }
      }
      if (k > 1) sum = '　數字 ' + k + ' 個：加總 ' + fmtNum(tot) + '　平均 ' + fmtNum(tot / k) + '　最小 ' + fmtNum(mn) + '　最大 ' + fmtNum(mx);
    }
    var hint = D.readOnly ? '唯讀' : act.r >= nrows() ? '最後一列下面的空白列：在這裡打字＝新增一列' :
      editing ? (editing.typed ? '［輸入］方向鍵＝寫入並移動，F2＝改成在格子裡移游標' : '［編輯］方向鍵＝在格子裡移游標，F2＝改回輸入') + '；Enter 確定，Ctrl+Enter＝填入選取的格子，Esc 取消' :
      '雙擊、F2 或直接打字（中文輸入法也可以）＝編輯；Ctrl+Enter／Ctrl+D／Ctrl+R＝填入；Ctrl+H 取代；Ctrl+S 存檔，Ctrl+Z 復原';
    /* the Name Box shows the active cell (Excel), unless one is typing a place into it */
    if (document.activeElement !== nameBox) nameBox.value = colName(act.c) + (act.r + 1);
    el.textContent = colName(act.c) + (act.r + 1) + name + (who ? '｜' + who : '') + '　' + (n > 1 ? '選了 ' + n + ' 格' + sum + '　' : '') +
      (cut ? '剪下了（貼上＝搬過去，Esc 取消）　' : '') +
      (ODD.list.length ? '⚠ 可疑 ' + ODD.list.length + ' 格（數字欄裡的文字，橘色角；右鍵「下一個可疑的格子」）　' : '') +
      (DUP ? colName(DUP.c) + ' 欄重複：' + (DUP.groups ? DUP.groups + ' 組（' + DUP.cells + ' 格，紅底）' : '沒有') + '　' : '') + hint;
  }

  /* --- moving --- */
  function clamp(x, a, b) { return Math.max(a, Math.min(b, x)); }
  function clampCur() {
    var R = Math.max(0, lastRow()), C = Math.max(0, ncols - 1);
    [cur, anchor, act].forEach(function (p) { p.r = clamp(p.r, 0, R); p.c = clamp(p.c, 0, C); });
    /* (the active cell is never a filtered-out one: the next row in sight) */
    if (!shown(act.r)) act.r = rowAt(idxOf(act.r));
    if (cur.r === anchor.r && cur.c === anchor.c && !shown(cur.r)) { cur.r = act.r; anchor.r = act.r; }
  }
  function scrollTo(r, c) {
    var y = rowY(r), top = wrap.scrollTop, hh = RH * (header ? 2 : 1);
    if (!(header && r === 0)) {
      if (y - hh < top) wrap.scrollTop = y - hh;
      else if (y + RH > top + wrap.clientHeight) wrap.scrollTop = y + RH - wrap.clientHeight;
    }
    var x = colX(c), left = wrap.scrollLeft;
    var fw = c >= FREEZE ? frozenW() : 0;
    if (c < FREEZE) { /* (a frozen column is always in sight) */ }
    else if (x - RNW - fw < left) wrap.scrollLeft = x - RNW - fw;
    else if (x + colW(c) > left + wrap.clientWidth) wrap.scrollLeft = x + colW(c) - wrap.clientWidth;
  }
  /** One cell (the selection and the active cell there), or (extend) the block from the anchor to it. */
  function go(r, c, extend) {
    COLSEL = false;
    cur = { r: r, c: c };
    if (!extend) { anchor = { r: r, c: c }; act = { r: r, c: c }; }
    clampCur();
    scrollTo(cur.r, cur.c);
    draw();
    save();
  }
  /* (1006 audit: whole columns = picked as columns -- the letters, Ctrl+Space; in a file with one data row a single cell
     used to count as a whole column and Ctrl+- deleted it) */
  var COLSEL = false;
  function setRect(r0, c0, r1, c1, ar, ac) {
    COLSEL = false;
    anchor = { r: r0, c: c0 }; cur = { r: r1, c: c1 }; act = { r: ar == null ? r0 : ar, c: ac == null ? c0 : ac };
    clampCur();
  }
  function multi() { return cur.r !== anchor.r || cur.c !== anchor.c; }
  /* Excel: Enter / Tab inside a selected block move the active cell and keep the block (down then the next
     column / right then the next row; Shift = back; round at the end) */
  function stepInSel(byRow, back) {
    var s = selRect(), r = act.r, c = act.c, d = back ? -1 : 1;
    /* (the next row of the block in sight; past its end = round to the other end) */
    var nextR = function (y, dd) { var n = stepRow(y, dd); return n > s.r1 || n < s.r0 || n === y ? null : n; };
    if (byRow) {
      c += d;
      if (c > s.c1) { c = s.c0; var n1 = nextR(r, 1); r = n1 == null ? firstIn(s) : n1; }
      else if (c < s.c0) { c = s.c1; var n2 = nextR(r, -1); r = n2 == null ? lastIn(s) : n2; }
    } else {
      var n3 = nextR(r, d);
      if (n3 != null) r = n3;
      else if (d > 0) { r = firstIn(s); c = c + 1 > s.c1 ? s.c0 : c + 1; }
      else { r = lastIn(s); c = c - 1 < s.c0 ? s.c1 : c - 1; }
    }
    act = { r: r, c: c };
    scrollTo(r, c);
    draw();
    save();
  }
  /** Enter / Tab from the active cell: inside a block, or to the next cell (Tab..Tab, Enter = back to the first column). */
  function enterMove(back) {
    if (multi()) return stepInSel(false, back);
    var c = tabStart != null && !back ? tabStart : act.c;
    tabStart = null;
    go(stepRow(act.r, back ? -1 : 1), c);
  }
  function tabMove(back) {
    if (multi()) return stepInSel(true, back);
    if (tabStart == null) tabStart = act.c;
    var r = act.r, c = act.c + (back ? -1 : 1);
    go(r, c);
  }
  /* Excel's Ctrl+arrow: to the edge of the data -- the last filled cell of this run, or the next filled one */
  function edge(r, c, dr, dc) {
    var R = nrows() - 1, C = ncols - 1;
    var inb = function (y, x) { return y >= 0 && y <= R && x >= 0 && x <= C && shown(y); };
    var filled = function (y, x) { return cell(y, x) !== ''; };
    /* (up / down: the rows in sight only -- a filter's hidden rows are stepped over, as Excel does) */
    var ny = function (y) { return dr ? stepRow(y, dr) : y; };
    var moved = function (y0, y1) { return !dr || y1 !== y0; };
    var y = ny(r), x = c + dc;
    if (!inb(y, x) || !moved(r, y)) return { r: clamp(r, 0, Math.max(R, 0)), c: c };
    if (filled(r, c) && filled(y, x)) {
      for (;;) { var y2 = ny(y); if (!moved(y, y2) || !inb(y2, x + dc) || !filled(y2, x + dc)) break; y = y2; x += dc; }
      return { r: y, c: x };
    }
    for (;;) { if (filled(y, x)) break; var y3 = ny(y); if (!moved(y, y3) || !inb(y3, x + dc)) break; y = y3; x += dc; }
    return { r: y, c: x };
  }

  /* --- editing --- */
  /* (1006 audit: every message says which version of the file this view shows -- a write from a view that a change
     made elsewhere has overtaken is refused by the extension, not applied to the wrong row) */
  function send(m) { if (D && D.ver != null && m && typeof m === 'object') m.ver = D.ver; api.postMessage(m); }
  function setCells(changes) {
    if (D.readOnly) { warnRo(); return; }
    changes = changes.filter(function (ch) { return cell(ch.r, ch.c) !== ch.v || !D.rows[ch.r] || ch.c >= D.rows[ch.r].length; })
      .filter(function (ch) { return ch.v !== '' || (D.rows[ch.r] && ch.c < D.rows[ch.r].length); });
    if (!changes.length) return;
    cut = null;   /* (Excel: an edit ends the cut) */
    /* shown at once; the document's own change comes back and redraws */
    changes.forEach(function (ch) {
      while (D.rows.length <= ch.r) D.rows.push([]);
      var row = D.rows[ch.r];
      while (row.length <= ch.c) row.push('');
      row[ch.c] = ch.v;
      if (ch.c >= ncols) ncols = ch.c + 1;
    });
    send({ type: 'edit', changes: changes });
    draw();
  }
  function focusKb() { try { kb.focus({ preventScroll: true }); } catch (e) { kb.focus(); } }
  /** The input on its cell: the editing one's, or (idle) the active one's -- the IME's word box opens there. */
  function placeKb() {
    var r = editing ? editing.r : act.r, c = editing ? editing.c : act.c;
    var s = kb.style;
    var w = editing ? Math.max(colW(c), 120) : colW(c);
    var right = !!editing && isNum(editing.orig) && !editing.typed;   /* a number edits right-aligned, as the cell shows it */
    s.left = (right ? cellX(c) + colW(c) - w : cellX(c)) + 'px'; s.top = (header && r === 0 ? wrap.scrollTop + RH : rowY(r)) + 'px';
    s.width = w + 'px'; s.height = RH + 'px';
    s.textAlign = right ? 'right' : '';
  }
  /* the extension holds VS Code's own Ctrl+Z off while a cell is edited (that one takes back this cell's typing);
     after a Ctrl+Z it says so a moment later -- VS Code reads the same key right after this page did */
  var offTimer = 0;
  function toIdle(refocus, late) {
    kb.value = '';
    kb.classList.add('idle');
    placeKb();
    clearTimeout(offTimer);
    if (late) offTimer = setTimeout(function () { if (!editing) send({ type: 'editing', on: false }); }, 400);
    else send({ type: 'editing', on: false });
    if (refocus) focusKb();
  }
  /** The input becomes the active cell's editor; `text` null = keep what was typed into it. */
  function begin(text, typed) {
    if (D.readOnly) { kb.value = ''; warnRo(); return false; }
    var orig = cell(act.r, act.c);
    /* (an <input> holds one line: a cell with a line break would lose it -- that one is edited as text) */
    if (!typed && /[\r\n]/.test(orig)) { kb.value = ''; showWarn('這一格有換行：在表格裡改會把換行弄掉。請按「以文字開啟」改這一格。'); return false; }
    editing = { r: act.r, c: act.c, orig: orig, typed: typed };
    clearTimeout(offTimer);
    if (text != null) kb.value = text;
    kb.classList.remove('idle');
    placeKb();
    focusKb();
    send({ type: 'editing', on: true });
    status();
    return true;
  }
  /** F2 / a double click (initial null): the cell's text, the caret at its end (Excel); else `initial` typed over it. */
  function startEdit(initial) {
    if (!begin(initial != null ? initial : cell(act.r, act.c), initial != null)) return false;
    kb.setSelectionRange(kb.value.length, kb.value.length);
    return true;
  }
  /* a double click: the caret where it was clicked (measured on the input's own font) */
  var mctx = null;
  function caretAt(clientX) {
    var b = kb.getBoundingClientRect(), cs = getComputedStyle(kb);
    if (!mctx) mctx = document.createElement('canvas').getContext('2d');
    mctx.font = cs.font;
    var t = kb.value, pad = (parseFloat(cs.paddingLeft) || 0) + (parseFloat(cs.borderLeftWidth) || 0);
    var full = mctx.measureText(t).width;
    var x0 = kb.style.textAlign === 'right' ? b.right - pad - full : b.left + pad;
    var x = clientX - x0, best = x <= 0 ? 0 : t.length;
    for (var i = 1; i <= t.length; i++) {
      var w = mctx.measureText(t.slice(0, i)).width;
      if (w >= x) { best = w - x > x - mctx.measureText(t.slice(0, i - 1)).width ? i - 1 : i; break; }
    }
    kb.setSelectionRange(best, best);
  }
  function editKey(e) {
    var k = e.key, ctrl = e.ctrlKey || e.metaKey;
    if (pickKey(e)) return;
    if (k === 'ArrowDown' && e.altKey && !ctrl) { e.preventDefault(); return openPick(); }
    if (k === 'Enter' && e.altKey) { e.preventDefault(); showWarn('表格的格子不能換行（機台的 CSV 一格一行）'); return; }
    if (k === 'Enter' && ctrl) { e.preventDefault(); return commitFill(); }
    if (k === 'Enter') { e.preventDefault(); commit(); return enterMove(e.shiftKey); }
    if (k === 'Tab') { e.preventDefault(); commit(); return tabMove(e.shiftKey); }
    if (k === 'Escape') { e.preventDefault(); return cancel(); }
    /* Ctrl+Z while editing = this cell's typing undone (VS Code's undo of the file is held off meanwhile) */
    /* (1006 audit: Ctrl+Y / Ctrl+Shift+Z while editing = nothing -- it used to throw the typing away like Ctrl+Z) */
    if (ctrl && (k === 'y' || k === 'Y' || ((k === 'z' || k === 'Z') && e.shiftKey))) { e.preventDefault(); return; }
    if (ctrl && (k === 'z' || k === 'Z')) { e.preventDefault(); return cancel(true); }
    if (k === 'F2') { e.preventDefault(); editing.typed = !editing.typed; placeKb(); return status(); }
    if (editing.typed && /^Arrow/.test(k)) {
      /* Enter mode (typed over): an arrow writes it and moves, like Excel */
      e.preventDefault();
      commit();
      tabStart = null;
      return go(stepRow(act.r, k === 'ArrowUp' ? -1 : k === 'ArrowDown' ? 1 : 0), act.c + (k === 'ArrowLeft' ? -1 : k === 'ArrowRight' ? 1 : 0));
    }
  }
  function commit(refocus) {
    if (!editing) return;
    var e = editing;
    editing = null;
    var v = kb.value;
    toIdle(refocus !== false);
    if (v !== e.orig) setCells([{ r: e.r, c: e.c, v: v }]);
    else draw();
  }
  /* Ctrl+Enter: the value into every selected cell (ONE edit); the block and the active cell stay */
  function commitFill() {
    if (!editing) return;
    var v = kb.value;
    editing = null;
    toIdle(true);
    var s = selRect(), ch = [];
    for (var r = bodyFrom(s); r <= fillLast(s); r++) if (shown(r)) for (var c = s.c0; c <= s.c1; c++) ch.push({ r: r, c: c, v: v });
    setCells(ch);
    draw();
  }
  /* 1008 review: the last row a fill / Ctrl+Enter / Ctrl+D reaches -- not the "+" row under the data (that was a new row
     at the end of the file: a duplicate No / Alias, "Motor名稱重複"), unless the block starts on it */
  function fillLast(s) { return s.r0 >= nrows() ? s.r1 : Math.min(s.r1, nrows() - 1); }
  function cancel(byUndoKey) {
    if (!editing) return;
    editing = null;
    toIdle(true, byUndoKey);
    draw();
  }
  function selRect() {
    return { r0: Math.min(cur.r, anchor.r), r1: Math.max(cur.r, anchor.r), c0: Math.min(cur.c, anchor.c), c1: Math.max(cur.c, anchor.c) };
  }
  /* (1006 audit: with 第一列是標題 on, a selection reaching past the title row leaves the names alone -- Ctrl+A then
     Delete used to clear Mot_Table's column names; the title row selected alone can still be changed) */
  function bodyFrom(s) { return header && s.r0 === 0 && s.r1 > 0 ? 1 : s.r0; }
  function clearSel() {
    var s = selRect(), ch = [];
    for (var r = bodyFrom(s); r <= s.r1; r++) if (shown(r)) for (var c = s.c0; c <= s.c1; c++) if (D.rows[r] && c < D.rows[r].length && D.rows[r][c] !== '') ch.push({ r: r, c: c, v: '' });
    setCells(ch);
  }
  /* the block as Excel's clipboard text: tab between cells, CRLF between rows, quoted when it has to be */
  function q(v) { return /[\t\r\n"]/.test(v) ? '"' + String(v).replace(/"/g, '""') + '"' : String(v); }
  function tsvOf(s) {
    var rows = [];
    for (var r = s.r0; r <= s.r1; r++) { if (!shown(r)) continue; var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(q(cell(r, c))); rows.push(line.join('\t')); }
    /* 1008 review: each row ends with CRLF, as Excel -- a block ending in empty cells kept its empty rows */
    return rows.map(function (x) { return x + '\r\n'; }).join('');
  }
  /* (with a filter on: the rows in sight only -- Excel copies what is shown) */
  function gridOf(s) {
    var g = [];
    for (var r = s.r0; r <= s.r1; r++) { if (!shown(r)) continue; var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(cell(r, c)); g.push(line); }
    return g;
  }
  function copySel() { cut = null; send({ type: 'copy', grid: gridOf(selRect()) }); draw(); }
  /* Ctrl+X: copied now, moved when pasted (Excel's "moving border"), Esc forgets it */
  function cutSel() {
    var s = selRect();
    if (hiddenIn(s.r0, s.r1)) { showWarn('篩選中，選取的範圍裡有被篩選掉的列：不能剪下（Excel 也不行）。要複製請按 Ctrl+C（只複製看得到的列），或先清除篩選。'); return; }
    send({ type: 'copy', grid: gridOf(s) });
    if (D.readOnly) { cut = null; warnRo(); return; }
    cut = { r0: s.r0, c0: s.c0, r1: s.r1, c1: s.c1, text: tsvOf(s) };
    draw();
  }
  var norm = function (t) { return String(t || '').replace(/\r\n|\r/g, '\n').replace(/\n$/, ''); };
  function pasteText(t) {
    var s = selRect();
    /* (a paste goes into the rows one after another: over a filtered-out row it would write where nobody sees) */
    var tall = norm(t).split('\n').length;
    if (hiddenIn(s.r0, Math.max(s.r1, s.r0 + tall - 1))) { showWarn('篩選中：貼上的範圍會跨過被篩選掉的列（寫到看不到的地方）。請先清除篩選再貼，或選一段中間沒有隱藏列的範圍。'); return; }
    /* 1008 review: a selection that takes in the header row with rows below it (Ctrl+A, A1:N50) pastes from the first data
       row -- the header was overwritten and the machine refused the whole table (its columns are found by name). The
       header alone selected = still pasted there, on purpose. A cut block that held the header does not clear it either */
    var r0p = bodyFrom(s);
    var m = { type: 'paste', text: t, r: r0p, c: s.c0, r1: Math.max(r0p, fillLast(s)), c1: s.c1 };
    if (cut && norm(t) === norm(cut.text)) m.cut = { r0: bodyFrom(cut), c0: cut.c0, r1: cut.r1, c1: cut.c1 };
    cut = null;
    send(m);
  }
  /* Ctrl+D / Ctrl+R: the first row (column) of the block into the rest; one row (column) = from the one above (left) */
  /* 1006 (Excel's Fill > Series / Auto Fill): the block's first two values of each column give the step (one value = +1),
     the rest of the block filled on from them -- a number, or text ending in one (X0 X1 ..., M02 M03 ..., the digits kept);
     one row with several columns = to the right. The rows in sight only. ONE edit. */
  /* (1006 audit: a whole number with decimals is a number -- "0.5, 1.0" went on 0.6, 0.7 and "1.9" became 1.10; in text
     only the digits at the end count -- "M-01" took its "-" for a minus sign and went M00, M01) */
  function seriesParts(v) {
    var t = String(v), d = /^(-?)(\d+)(?:\.(\d+))?$/.exec(t);
    if (d) return { pre: '', num: parseFloat(t), dec: d[3] ? d[3].length : 0, width: d[2].length, pad: /^0\d/.test(d[2]), suf: '' };
    var m = /^(.*?)(\d+)(\D*)$/.exec(t);
    if (!m) return null;
    return { pre: m[1], num: parseInt(m[2], 10), dec: 0, width: m[2].length, pad: /^0\d/.test(m[2]), suf: m[3] };
  }
  function seriesText(p, n) {
    var s = p.dec ? Math.abs(n).toFixed(p.dec) : String(Math.round(Math.abs(n)));
    if (p.pad) { var ip = s.split('.'); while (ip[0].length < p.width) ip[0] = '0' + ip[0]; s = ip.join('.'); }
    return p.pre + (n < 0 ? '-' : '') + s + p.suf;
  }
  function fillSeries() {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect(), across = s.r0 === s.r1 && s.c1 > s.c0, ch = [], why = '';
    var lines = across ? [s.r0] : [];
    if (!across) for (var c = s.c0; c <= s.c1; c++) lines.push(c);
    lines.forEach(function (ln) {
      var at = [];
      if (across) for (var x = s.c0; x <= s.c1; x++) at.push({ r: ln, c: x });
      else for (var y = s.r0; y <= fillLast(s); y++) if (shown(y) && !(header && y === 0)) at.push({ r: y, c: ln });
      if (at.length < 2) return;
      var p0 = seriesParts(cell(at[0].r, at[0].c));
      if (!p0) { why = why || '第一格不是數字，也不是以數字結尾（例如 X0、M02）'; return; }
      var p1 = at.length > 2 ? seriesParts(cell(at[1].r, at[1].c)) : null;
      var two = p1 && p1.pre === p0.pre && p1.suf === p0.suf;
      var step = two ? p1.num - p0.num : 1;
      /* (decimals: as many as the two values have, the steps added without float noise) */
      var pp = { pre: p0.pre, suf: p0.suf, pad: p0.pad, width: p0.width, dec: Math.max(p0.dec, two ? p1.dec : 0) };
      for (var i = two ? 2 : 1; i < at.length; i++) ch.push({ r: at[i].r, c: at[i].c, v: seriesText(pp, +(p0.num + step * i).toFixed(pp.dec + 6)) });
    });
    if (!ch.length) { showWarn(why || '選一段範圍（一欄往下，或一列往右），第一格放起始值（第二格放第二個值＝步長），再按「填滿數列」'); return; }
    setCells(ch);
  }
  function fill(down) {
    var s = selRect(), ch = [];
    var one = down ? s.r0 === s.r1 : s.c0 === s.c1;
    /* (with a filter on: from the row in sight above / the block's first row in sight, into the rows in sight) */
    var src = down ? (one ? stepRow(s.r0, -1) : firstIn(s)) : (one ? s.c0 - 1 : s.c0);
    if (down && one && src >= s.r0) src = -1;
    if (src < 0) { showWarn(down ? '上面沒有列可以往下填' : '左邊沒有欄可以往右填'); return; }
    if (down && header && src === 0) { showWarn('上面是標題列，不往下填'); return; }
    if (down) { for (var r = one ? s.r0 : src + 1; r <= fillLast(s); r++) if (shown(r)) for (var c = s.c0; c <= s.c1; c++) ch.push({ r: r, c: c, v: cell(src, c) }); }
    else { for (var r2 = s.r0; r2 <= fillLast(s); r2++) if (shown(r2)) for (var c2 = one ? s.c0 : s.c0 + 1; c2 <= s.c1; c2++) ch.push({ r: r2, c: c2, v: cell(r2, src) }); }
    setCells(ch);
  }
  function selectRows() { var s = selRect(); setRect(s.r0, 0, s.r1, ncols - 1, act.r, act.c); draw(); }
  /* (the frozen title row is not part of a column: a Ctrl+Enter on a column never writes over the column names) */
  function colTop() { return firstBody() < nrows() ? firstBody() : 0; }
  function selectCols() { var s = selRect(); setRect(colTop(), s.c0, Math.max(nrows() - 1, 0), s.c1, Math.max(act.r, colTop()), act.c); COLSEL = true; draw(); }

  /* --- find / replace --- */
  function match(v) {
    v = String(v).toLowerCase();
    return find.whole ? v === find.q : v.indexOf(find.q) >= 0;
  }
  function inScope(r, c) { return !find.rect || inRect(find.rect, r, c); }
  function runFind(qs, dir) {
    find.q = String(qs || '').toLowerCase();
    find.whole = wholeBox.checked;
    find.hits = [];
    if (find.q) {
      for (var r = 0; r < nrows(); r++) {
        var row = D.rows[r];
        if (!shown(r)) continue;   /* (with a filter on: the rows in sight) */
        for (var c = 0; c < row.length; c++) if (inScope(r, c) && match(row[c])) find.hits.push({ r: r, c: c });
      }
    }
    var n = document.getElementById('findN');
    if (!find.hits.length) { find.i = -1; n.textContent = find.q ? '找不到' : ''; draw(); return; }
    if (dir) {
      var at = -1, i;
      if (dir > 0) { for (i = 0; i < find.hits.length; i++) { var h = find.hits[i]; if (h.r > act.r || (h.r === act.r && h.c > act.c)) { at = i; break; } } if (at < 0) at = 0; }
      else { for (i = find.hits.length - 1; i >= 0; i--) { var g = find.hits[i]; if (g.r < act.r || (g.r === act.r && g.c < act.c)) { at = i; break; } } if (at < 0) at = find.hits.length - 1; }
      find.i = at;
      /* (inside the searched block: the block stays, only the active cell moves) */
      if (find.rect) { setRect(find.rect.r0, find.rect.c0, find.rect.r1, find.rect.c1, find.hits[at].r, find.hits[at].c); scrollTo(act.r, act.c); draw(); save(); }
      else go(find.hits[at].r, find.hits[at].c);
    } else draw();
    n.textContent = find.hits.length + ' 個' + (find.i >= 0 ? '（第 ' + (find.i + 1) + ' 個）' : '');
  }
  function replaced(v, by) {
    if (find.whole) return by;
    return String(v).replace(new RegExp(find.q.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'), 'gi'), function () { return by; });
  }
  function replaceOne() {
    if (D.readOnly) return warnRo();
    runFind(findBox.value, 0);
    if (!find.q) return;
    var v = cell(act.r, act.c), done = 0;
    if (act.r < nrows() && inScope(act.r, act.c) && match(v)) { setCells([{ r: act.r, c: act.c, v: replaced(v, replBox.value) }]); done = 1; }
    runFind(findBox.value, 1);
    document.getElementById('replN').textContent = done ? '取代了 1 格' : '';
  }
  function replaceAll() {
    if (D.readOnly) return warnRo();
    runFind(findBox.value, 0);
    if (!find.q || !find.hits.length) { document.getElementById('replN').textContent = '沒有可以取代的'; return; }
    var by = replBox.value;
    /* (1007 audit D5: the title row is left alone -- Delete, Ctrl+A, Ctrl+D already do -- unless the search is inside a
       block of the title row itself) */
    var hits = find.hits.filter(function (h) { return !(header && h.r === 0 && !(find.rect && find.rect.r0 === 0 && find.rect.r1 === 0)); });
    if (!hits.length) { document.getElementById('replN').textContent = '只有標題列有，標題列不取代（要改標題：只選標題列那一格再取代）'; return; }
    var ch = hits.map(function (h) { return { r: h.r, c: h.c, v: replaced(cell(h.r, h.c), by) }; });
    var n = ch.filter(function (x) { return x.v !== cell(x.r, x.c); }).length;
    setCells(ch);   /* ONE edit: one Ctrl+Z takes them all back */
    document.getElementById('replN').textContent = '取代了 ' + n + ' 格';
    runFind(findBox.value, 0);
  }
  function setScope(on) {
    inSelBox.checked = on;
    var s = selRect();
    find.rect = on ? { r0: s.r0, c0: s.c0, r1: s.r1, c1: s.c1 } : null;
    document.getElementById('inSelR').textContent = on ? '（' + colName(s.c0) + (s.r0 + 1) + ':' + colName(s.c1) + (s.r1 + 1) + '）' : '';
  }
  function openFind(withReplace) {
    if (withReplace) replBar.hidden = false;
    /* Excel: several cells selected = they are what is searched */
    setScope(multi());
    (withReplace && findBox.value ? replBox : findBox).focus();
    (withReplace && findBox.value ? replBox : findBox).select();
  }

  function showWarn(t) {
    var w = document.getElementById('warn');
    w.hidden = !t;
    w.textContent = t || '';
  }

  /* --- mouse --- */
  var dragSel = false, rsCol = -1, rsX = 0, rsW = 0;
  sizer.addEventListener('mousedown', function (e) {
    var t = e.target;
    if (t.getAttribute('data-rs') != null) {
      e.preventDefault();
      /* (a double click on the boundary = AutoFit, told by the click count as the cells' double click) */
      if (e.detail >= 2 && e.button === 0) { rsCol = -1; fitFrom(+t.getAttribute('data-rs')); focusKb(); return; }
      rsCol = +t.getAttribute('data-rs'); rsX = e.clientX; rsW = colW(rsCol);
      return;
    }
    if (t.getAttribute('data-fb') != null) { e.preventDefault(); if (editing) commit(); openFilter(+t.getAttribute('data-fb')); return; }
    if (editing && t === kb) return;
    if (editing) commit();
    tabStart = null;
    /* a right click inside the selection keeps it (Excel) -- on a cell, or on the header of a selected row / column */
    if (e.button === 2 && t.getAttribute('data-r') != null && inSel(+t.getAttribute('data-r'), +t.getAttribute('data-c'))) { e.preventDefault(); focusKb(); return; }
    if (e.button === 2) {
      var s2 = selRect(), rh = t.getAttribute('data-row'), ch2 = t.getAttribute('data-col');
      if (rh != null && s2.c0 === 0 && s2.c1 === ncols - 1 && +rh >= s2.r0 && +rh <= s2.r1) { e.preventDefault(); focusKb(); return; }
      if (ch2 != null && +ch2 >= s2.c0 && +ch2 <= s2.c1 && s2.r0 <= colTop() && s2.r1 >= nrows() - 1) { e.preventDefault(); focusKb(); return; }
    }
    if (t.getAttribute('data-all')) { e.preventDefault(); focusKb(); setRect(0, 0, Math.max(nrows() - 1, 0), ncols - 1, act.r, act.c); draw(); return; }
    if (t.getAttribute('data-col') != null) {
      e.preventDefault(); focusKb();
      var cc = +t.getAttribute('data-col');
      var c0 = e.shiftKey ? anchor.c : cc;
      setRect(colTop(), c0, Math.max(nrows() - 1, 0), cc, e.shiftKey ? act.r : colTop(), e.shiftKey ? act.c : cc);
      COLSEL = true;
      draw(); return;
    }
    if (t.getAttribute('data-row') != null) {
      e.preventDefault(); focusKb();
      var rr = +t.getAttribute('data-row');
      setRect(e.shiftKey ? anchor.r : rr, 0, rr, ncols - 1, e.shiftKey ? act.r : rr, e.shiftKey ? act.c : 0);
      draw(); return;
    }
    if (t.getAttribute('data-r') == null) return;
    e.preventDefault();
    focusKb();
    var p = { r: +t.getAttribute('data-r'), c: +t.getAttribute('data-c') };
    cur = p;
    if (!e.shiftKey) { anchor = { r: p.r, c: p.c }; act = { r: p.r, c: p.c }; }
    /* a double click = edit, told by the click count right here: the browser's own dblclick never
     * reaches the cell -- this mousedown redraws the grid, so the second click's target is a new
     * element and Chromium aims the dblclick at what the two have in common (the grid) */
    if (e.detail >= 2 && !e.shiftKey && e.button === 0) { dragSel = false; draw(); save(); if (startEdit(null)) caretAt(e.clientX); return; }
    dragSel = e.button === 0;
    draw();
    save();
  });
  window.addEventListener('mousemove', function (e) {
    if (rsCol >= 0) { widths[rsCol] = Math.max(30, rsW + e.clientX - rsX); draw(); return; }
    if (!dragSel) return;
    var t = document.elementFromPoint(e.clientX, e.clientY);
    if (t && t.getAttribute && t.getAttribute('data-r') != null) {
      var r = +t.getAttribute('data-r'), c = +t.getAttribute('data-c');
      if (r !== cur.r || c !== cur.c) { cur = { r: r, c: c }; draw(); }
    }
  });
  window.addEventListener('mouseup', function () { if (rsCol >= 0) save(); rsCol = -1; dragSel = false; });

  /* --- the right-click menu (AI(W906-HTDESIGNER) 20261001, Excel's cell menu): the same as the keys, each with its key
     beside it; Shift+F10 / the menu key open it at the active cell; Up / Down / Enter / Esc in it (VS Code's menus) --- */
  var ctx = null;
  function closeMenu() {
    if (!ctx) return;
    if (ctx.el.parentNode) ctx.el.parentNode.removeChild(ctx.el);
    document.removeEventListener('mousedown', ctx.away, true);
    document.removeEventListener('keydown', ctx.keys, true);
    ctx = null;
  }
  function openMenu(x, y) {
    closeMenu();
    if (editing) commit();
    var ro = D.readOnly, s = selRect();
    var nIns = s.r1 - s.r0 + 1, nDel = shownRuns(s).reduce(function (a, x) { return a + x.count; }, 0);
    var items = [
      ['剪下', 'Ctrl+X', !ro, cutSel],
      ['複製', 'Ctrl+C', true, copySel],
      ['貼上', 'Ctrl+V', !ro, function () { send({ type: 'readClip' }); }],
      null,
      ['插入 ' + nIns + ' 列（在上面）', 'Ctrl+Shift++', !ro, insRow],
      ['刪除 ' + Math.max(nDel, 0) + ' 列', 'Ctrl+－', !ro && nDel > 0, function () { delRow(false); }],
      ['清除內容', 'Delete', !ro, clearSel],
      ['插入 ' + (s.c1 - s.c0 + 1) + ' 欄（在左邊）', '', !ro, insCol],
      ['刪除 ' + (s.c1 - s.c0 + 1) + ' 欄', '', !ro && wholeCols(s), delCol],
      ['填滿數列（1、2、3… 或 X0、X1…）', '', !ro, fillSeries],
      null,
      ['選取整列', 'Shift+空白鍵', true, selectRows],
      ['選取整欄', 'Ctrl+空白鍵', true, selectCols],
      null,
      ['篩選這一欄…', 'Alt+↓（標題列）', true, function () { openFilter(act.c); }],
      [DUP && DUP.c === act.c ? '取消標出重複值' : '標出這一欄的重複值', '', true, function () { toggleDup(act.c); }],
      [FREEZE && act.c === FREEZE - 1 ? '取消凍結欄' : '凍結到這一欄（' + colName(0) + (act.c > 0 ? '～' + colName(act.c) : '') + ' 固定在左邊）', '', true, function () { setFreeze(FREEZE && act.c === FREEZE - 1 ? 0 : act.c + 1); }],
      ['依這一格的值篩選', '', true, function () { filterByCell(); }],
      null,
      ['尋找…', 'Ctrl+F', true, function () { openFind(false); }],
      ['取代…', 'Ctrl+H', !ro, function () { openFind(true); }]
    ];
    /* (清除所有篩選 only while a filter is on) */
    /* (1006 audit: in the filter group, after 依這一格的值篩選 -- indexOf(null, 11) put it among the selection items) */
    if (V) { var fAt = 0; items.forEach(function (it, ix) { if (it && it[0] === '依這一格的值篩選') fAt = ix + 1; }); items.splice(fAt || items.length, 0, ['清除所有篩選', 'Ctrl+Shift+L', true, clearFilters]); }
    /* (only while there are some) */
    if (ODD.list.length) items.push(null, ['下一個可疑的格子（數字欄裡的文字，共 ' + ODD.list.length + ' 格）', '', true, nextOdd]);
    var m = document.createElement('div');
    m.className = 'ctxmenu';
    var live = [], at = -1;
    var hot = function (i) { at = i; live.forEach(function (b, j) { b.classList.toggle('hot', j === i); }); };
    items.forEach(function (it) {
      if (!it) { var sp = document.createElement('div'); sp.className = 'ctxsep'; m.appendChild(sp); return; }
      var b = document.createElement('div');
      b.className = 'ctxitem' + (it[2] ? '' : ' off');
      b.setAttribute('data-item', it[0]);
      var l = document.createElement('span'); l.textContent = it[0]; b.appendChild(l);
      var k = document.createElement('span'); k.className = 'ctxkey'; k.textContent = it[1]; b.appendChild(k);
      if (it[2]) {
        live.push(b);
        b.addEventListener('mouseenter', function () { hot(live.indexOf(b)); });
        b.addEventListener('click', function () { closeMenu(); focusKb(); it[3](); });
      }
      m.appendChild(b);
    });
    document.body.appendChild(m);
    m.style.left = Math.max(0, Math.min(x, window.innerWidth - m.offsetWidth - 4)) + 'px';
    m.style.top = Math.max(0, Math.min(y, window.innerHeight - m.offsetHeight - 4)) + 'px';
    var away = function (e) { if (!m.contains(e.target)) closeMenu(); };
    var keys = function (e) {
      var k = e.key;
      if (k === 'Escape') { e.preventDefault(); e.stopPropagation(); closeMenu(); focusKb(); }
      else if (k === 'ArrowDown' || k === 'ArrowUp') {
        e.preventDefault(); e.stopPropagation();
        if (live.length) hot(at < 0 ? (k === 'ArrowDown' ? 0 : live.length - 1) : (at + (k === 'ArrowDown' ? 1 : live.length - 1)) % live.length);
      } else if (k === 'Enter') { e.preventDefault(); e.stopPropagation(); if (at >= 0) live[at].click(); }
      else if (k !== 'Shift' && k !== 'Control' && k !== 'Alt') closeMenu();   // (any other key: the menu goes, the key works as usual)
    };
    document.addEventListener('mousedown', away, true);
    document.addEventListener('keydown', keys, true);
    ctx = { el: m, away: away, keys: keys };
    return m;
  }
  /* (the cell editor's own text keeps the browser's text menu) */
  sizer.addEventListener('contextmenu', function (e) {
    var t = e.target;
    if (t === kb) return;
    e.preventDefault();
    if (t.getAttribute('data-r') == null && t.getAttribute('data-row') == null && t.getAttribute('data-col') == null && !t.getAttribute('data-all')) return;
    openMenu(e.clientX, e.clientY);
  });
  function menuAtCell() {
    var c = sizer.querySelector('.cell[data-r="' + act.r + '"][data-c="' + act.c + '"]');
    var b = c ? c.getBoundingClientRect() : { left: 40, bottom: 40 };
    openMenu(b.left + 8, b.bottom - 4);
  }
  /* --- the Name Box / Go To (AI(W906-HTDESIGNER) 20261001, Excel: a reference typed into the Name Box + Enter goes
     there; Ctrl+G = Go To): C12 = that cell, B2:D5 = that block (its top-left the active cell), 12 = row 12 in this
     column, C = column C in this row; $ signs are ignored --- */
  function colIndex(s) { var n = 0; for (var i = 0; i < s.length; i++) n = n * 26 + (s.charCodeAt(i) - 64); return n - 1; }
  function parseRef(t) {
    t = String(t || '').trim().toUpperCase().replace(/\$/g, '');
    var m = /^([A-Z]{1,3})(\d{1,7})(?::([A-Z]{1,3})(\d{1,7}))?$/.exec(t);
    if (m) {
      var r0 = +m[2] - 1, c0 = colIndex(m[1]), r1 = m[3] ? +m[4] - 1 : r0, c1 = m[3] ? colIndex(m[3]) : c0;
      return { r0: Math.min(r0, r1), c0: Math.min(c0, c1), r1: Math.max(r0, r1), c1: Math.max(c0, c1) };
    }
    if ((m = /^(\d{1,7})$/.exec(t))) return { r0: +m[1] - 1, c0: act.c, r1: +m[1] - 1, c1: act.c };
    if ((m = /^([A-Z]{1,3})$/.exec(t))) return { r0: act.r, c0: colIndex(m[1]), r1: act.r, c1: colIndex(m[1]) };
    return null;
  }
  function goRef(t) {
    var p = parseRef(t);
    if (!p || p.r0 < 0 || p.c0 < 0) { showWarn('「' + String(t).trim() + '」不是格子的位置：打 C12、B2:D5 或列號（12）。'); return false; }
    var R = Math.max(0, lastRow()), C = Math.max(0, ncols - 1);
    if (p.r1 > R || p.c1 > C) { showWarn('沒有「' + String(t).trim().toUpperCase() + '」：這個表格到 ' + colName(C) + (R + 1) + '（' + nrows() + ' 列 × ' + ncols + ' 欄）。'); return false; }
    showWarn(D.readOnly ? D.why : '');
    tabStart = null;
    setRect(p.r0, p.c0, p.r1, p.c1, p.r0, p.c0);
    scrollTo(p.r0, p.c0);
    draw();
    save();
    return true;
  }
  nameBox.addEventListener('focus', function () { nameBox.select(); });
  nameBox.addEventListener('keydown', function (e) {
    if (e.isComposing || e.keyCode === 229) return;
    e.stopPropagation();
    if (e.key === 'Enter') { e.preventDefault(); if (goRef(nameBox.value)) focusKb(); else nameBox.select(); }
    else if (e.key === 'Escape') { e.preventDefault(); focusKb(); }
  });
  nameBox.addEventListener('blur', function () { status(); });

  /* AutoFit (Excel: a double click on the boundary of a column heading): the width of its longest value, every row,
     measured in the table's own font (the title row in its bold); with whole columns selected and the boundary one of
     theirs: each of them */
  var meas = null;
  function fitCols(cs) {
    if (!meas) meas = document.createElement('canvas').getContext('2d');
    var body = sizer.querySelector('.row:not(.frozen) .cell') || sizer.querySelector('.cell'), head = sizer.querySelector('.row.frozen .cell');
    if (!body) return;
    var fontOf = function (x) { var s = getComputedStyle(x); return s.fontStyle + ' ' + s.fontWeight + ' ' + s.fontSize + ' ' + s.fontFamily; };
    var bs = getComputedStyle(body), pad = (parseFloat(bs.paddingLeft) || 0) + (parseFloat(bs.paddingRight) || 0) + 8;
    var fBody = fontOf(body), fHead = head ? fontOf(head) : fBody;
    var wOf = function (v, f) { meas.font = f; return meas.measureText(v.length > 400 ? v.slice(0, 400) : v).width; };
    cs.forEach(function (c) {
      var mx = wOf(colName(c), fBody);
      for (var r = 0; r < nrows(); r++) {
        var v = cell(r, c);
        if (!v) continue;
        var w = wOf(v, header && r === 0 ? fHead : fBody);
        if (w > mx) mx = w;
      }
      widths[c] = Math.max(30, Math.min(800, Math.ceil(mx + pad)));
    });
    save();
    draw();
  }
  function fitFrom(c) {
    var s = selRect(), whole = s.r0 <= colTop() && s.r1 >= nrows() - 1 && c >= s.c0 && c <= s.c1;
    var cs = [];
    if (whole) for (var i = s.c0; i <= s.c1; i++) cs.push(i); else cs.push(c);
    fitCols(cs);
  }

  var raf = 0;
  wrap.addEventListener('scroll', function () { if (drawn.a >= 0 && !raf) raf = requestAnimationFrame(function () { raf = 0; draw(); }); });
  window.addEventListener('resize', draw);
  /* the keys always go to the input: a click on the table's empty part / its scroll bar, or VS Code
   * focusing the editor, hands the focus back to it */
  wrap.addEventListener('focus', function (e) { if (e.target === wrap) focusKb(); });
  window.addEventListener('focus', function () { var a = document.activeElement; if (!a || a === document.body || a === wrap) focusKb(); });

  /* --- keys --- */
  kb.addEventListener('keydown', function (e) {
    if (e.isComposing || e.keyCode === 229) return;   /* the IME's own keys: its Enter picks the word, not the next cell */
    if (editing) return editKey(e);
    navKey(e);
  });
  /* a character typed into the idle input (or an IME word started in it): that cell's editor now */
  kb.addEventListener('input', function (e) {
    if (editing || composing || e.isComposing || kb.value === '') return;
    begin(null, true);
  });
  kb.addEventListener('compositionstart', function () { composing = true; if (!editing) begin(null, true); });
  kb.addEventListener('compositionend', function () { composing = false; if (!editing) kb.value = ''; });
  kb.addEventListener('blur', function () { if (editing) commit(false); });
  /* 1005 (WPF / Excel audit: Excel's "Pick From Drop-down List", Alt+Down): the values already in this column (each once,
   * A-Z, the header row left out), under the active cell; Up / Down / Enter or a click puts one in, Esc closes. While
   * typing, the list keeps the ones that start with / contain what is typed. */
  var pick = null;   /* { el, items, i } */
  function columnValues(c) {
    var seen = {}, out = [], n = nrows();
    for (var r = header ? 1 : 0; r < n && out.length < 2000; r++) {
      var v = cell(r, c);
      if (v === '' || seen[v]) continue;
      seen[v] = 1; out.push(v);
    }
    /* (numbers first, by value; then the rest A-Z -- one rule for every pair, so a mixed column sorts the same way) */
    var isN = function (v) { return v.trim() !== '' && isFinite(+v); };
    return out.sort(function (a, b) { var x = isN(a), y = isN(b); return x && y ? (+a) - (+b) : x !== y ? (x ? -1 : 1) : a.localeCompare(b); });
  }
  function closePick() { if (pick) { pick.el.remove(); pick = null; } }
  function openPick() {
    closePick();
    if (D.readOnly) { warnRo(); return; }
    var typed = editing ? kb.value : '';
    var all = columnValues(act.c).filter(function (v) { return v !== cell(act.r, act.c) || typed; });
    var t = typed.toLowerCase();
    var items = t ? all.filter(function (v) { return v.toLowerCase().indexOf(t) === 0; }).concat(all.filter(function (v) { var x = v.toLowerCase(); return x.indexOf(t) > 0; })) : all;
    var el = document.createElement('div');
    el.className = 'pick';
    if (!items.length) el.innerHTML = '<div class="none">' + (all.length ? '這一欄沒有以「' + typed.replace(/[&<>]/g, '') + '」開頭的值' : '這一欄還沒有別的值') + '</div>';
    items.slice(0, 500).forEach(function (v, i) {
      var o = document.createElement('div');
      o.className = 'po'; o.textContent = v === '' ? '（空白）' : v; o.setAttribute('data-i', i);
      o.addEventListener('mousedown', function (ev) { ev.preventDefault(); choosePick(i); });
      el.appendChild(o);
    });
    var b = kb.getBoundingClientRect();
    el.style.left = Math.round(b.left) + 'px';
    el.style.top = Math.round(b.bottom) + 'px';
    el.style.minWidth = Math.max(120, Math.round(b.width)) + 'px';
    document.body.appendChild(el);
    pick = { el: el, items: items.slice(0, 500), i: items.length ? 0 : -1 };
    markPick();
  }
  function markPick() {
    if (!pick) return;
    Array.prototype.forEach.call(pick.el.querySelectorAll('.po'), function (o) { o.classList.toggle('on', +o.getAttribute('data-i') === pick.i); });
    var on = pick.el.querySelector('.po.on');
    if (on && on.scrollIntoView) on.scrollIntoView({ block: 'nearest' });
  }
  function choosePick(i) {
    if (!pick || i < 0 || i >= pick.items.length) return closePick();
    var v = pick.items[i];
    closePick();
    if (editing) { kb.value = v; commit(); return; }
    setCells([{ r: act.r, c: act.c, v: v }]);
  }
  /** a key while the list is open: true = it was the list's */
  function pickKey(e) {
    if (!pick) return false;
    var k = e.key;
    if (k === 'ArrowDown' || k === 'ArrowUp') { e.preventDefault(); if (pick.items.length) { pick.i = (pick.i + (k === 'ArrowDown' ? 1 : -1) + pick.items.length) % pick.items.length; markPick(); } return true; }
    if (k === 'Enter' || k === 'Tab') { e.preventDefault(); choosePick(pick.i); return true; }
    if (k === 'Escape') { e.preventDefault(); closePick(); return true; }
    closePick();
    return false;
  }
  document.addEventListener('mousedown', function (e) { if (pick && !(e.target.closest && e.target.closest('.pick'))) closePick(); }, true);
  function navKey(e) {
    var k = e.key, ctrl = e.ctrlKey || e.metaKey, sh = e.shiftKey;
    if (pickKey(e)) return;
    /* Alt+Down: on the title row = the column's filter (Excel's AutoFilter button); in the data = pick a value */
    if (k === 'ArrowDown' && e.altKey && !ctrl) { e.preventDefault(); return header && act.r === 0 ? openFilter(act.c) : openPick(); }
    /* Ctrl+Shift+L (Excel: filters on / off): every filter cleared, or (none) the active column's filter opened;
       Ctrl+Alt+L (Excel: Reapply): the rows held in sight after edits are filtered again */
    if (ctrl && (k === 'l' || k === 'L') && sh && !e.altKey) { e.preventDefault(); return V ? clearFilters() : openFilter(act.c); }
    if (ctrl && (k === 'l' || k === 'L') && e.altKey) { e.preventDefault(); keep = null; return refilter(); }
    var page = Math.max(1, Math.floor(wrap.clientHeight / RH) - 2);
    /* plain: from the active cell; Shift: the far corner of the block moves */
    var from = sh ? cur : act;
    var mv = function (dr, dc) {
      e.preventDefault();
      tabStart = null;
      var to = ctrl ? edge(from.r, from.c, dr, dc) : { r: stepRow(from.r, dr), c: from.c + dc };
      go(to.r, to.c, sh);
    };
    if (k === 'ArrowUp') return mv(-1, 0);
    if (k === 'ArrowDown') return mv(1, 0);
    if (k === 'ArrowLeft') return mv(0, -1);
    if (k === 'ArrowRight') return mv(0, 1);
    if (k === 'PageUp') { e.preventDefault(); return go(stepRow(from.r, -page), from.c, sh); }
    if (k === 'PageDown') { e.preventDefault(); return go(stepRow(from.r, page), from.c, sh); }
    if (k === 'Home') { e.preventDefault(); return go(ctrl ? (shown(0) ? 0 : rowAt(0)) : from.r, 0, sh); }
    if (k === 'End') { e.preventDefault(); return go(ctrl ? lastIn({ r0: 0, r1: Math.max(nrows() - 1, 0) }) : from.r, ncols - 1, sh); }
    if (k === 'Tab') { e.preventDefault(); return tabMove(sh); }
    if (k === 'Enter' && !ctrl && !e.altKey) { e.preventDefault(); return enterMove(sh); }
    if (k === 'F2') { e.preventDefault(); return startEdit(null); }
    if (k === 'ContextMenu' || (k === 'F10' && sh && !ctrl)) { e.preventDefault(); return menuAtCell(); }
    if (ctrl && !sh && (k === 'g' || k === 'G')) { e.preventDefault(); return nameBox.focus(); }
    if (k === 'Escape') { if (cut) { e.preventDefault(); cut = null; draw(); } return; }
    if (k === 'Delete') { e.preventDefault(); return clearSel(); }
    /* Excel: Backspace = the active cell edited from empty (Esc gives it back) */
    if (k === 'Backspace') { e.preventDefault(); if (act.r < nrows() || !D.readOnly) begin('', true); return; }
    if (k === ' ' && sh && !ctrl) { e.preventDefault(); return selectRows(); }
    if (k === ' ' && ctrl && !sh) { e.preventDefault(); return selectCols(); }
    if (ctrl && (k === 'c' || k === 'C')) { e.preventDefault(); return copySel(); }
    if (ctrl && (k === 'x' || k === 'X')) { e.preventDefault(); return cutSel(); }
    if (ctrl && (k === 'a' || k === 'A')) { e.preventDefault(); setRect(0, 0, Math.max(nrows() - 1, 0), ncols - 1, act.r, act.c); return draw(); }
    if (ctrl && (k === 'd' || k === 'D')) { e.preventDefault(); return fill(true); }
    if (ctrl && (k === 'r' || k === 'R')) { e.preventDefault(); return fill(false); }
    if (ctrl && (k === 'f' || k === 'F')) { e.preventDefault(); return openFind(false); }
    if (ctrl && (k === 'h' || k === 'H')) { e.preventDefault(); return openFind(true); }
    if (k === 'F3') { e.preventDefault(); return runFind(findBox.value, sh ? -1 : 1); }
    /* (1009 review: Ctrl+NumpadAdd too, without Shift -- Excel's insert; it zoomed VS Code) */
    if (ctrl && ((sh && (k === '+' || k === '=')) || (!sh && e.code === 'NumpadAdd'))) { e.preventDefault(); var s9 = selRect(); return wholeCols(s9) && !(s9.c0 === 0 && s9.c1 === ncols - 1) ? insCol() : insRow(); }
    if (ctrl && !sh && k === '-') { e.preventDefault(); var s8 = selRect(); return wholeCols(s8) && !(s8.c0 === 0 && s8.c1 === ncols - 1) ? delCol() : delRow(true); }
    /* (Ctrl+Z / Ctrl+Y / Ctrl+S: VS Code's own -- the undo stack and the saving are the document's) */
    /* a character: it goes into the input, whose 'input' starts typing over the cell (Excel) */
    if (!ctrl && !e.altKey && k.length === 1 && D.readOnly) { e.preventDefault(); warnRo(); }
  }
  /* Ctrl+C when VS Code does the copy (it runs the webview's copy): the same text as the extension's copy */
  document.addEventListener('copy', function (e) {
    if (editing || document.activeElement === findBox || document.activeElement === replBox || document.activeElement === nameBox || !e.clipboardData) return;
    e.clipboardData.setData('text/plain', tsvOf(selRect()));
    e.preventDefault();
  });
  /* Ctrl+V: the clipboard text (from Excel: tab separated) goes in from the selected cell -- on a bigger block,
     a block that fits into it a whole number of times fills it (Excel) */
  document.addEventListener('paste', function (e) {
    if (editing || document.activeElement === findBox || document.activeElement === replBox || document.activeElement === nameBox) return;
    var t = e.clipboardData ? e.clipboardData.getData('text/plain') : '';
    if (!t) return;
    e.preventDefault();
    if (D.readOnly) return warnRo();
    pasteText(t);
  });

  /* 1006 (Excel's Insert / Delete sheet columns): whole columns selected -- as many columns (left of them) / those columns;
     the widths, filters, the duplicate marks and the frozen columns follow */
  function wholeCols(s) { return (COLSEL || nrows() - colTop() >= 2) && s.r0 <= colTop() && s.r1 >= nrows() - 1; }
  function shiftCols(at, n) {
    var mv = function (obj) { var o = {}; Object.keys(obj).forEach(function (k) { var i = +k; if (n < 0 && i >= at && i < at - n) return; o[i >= at ? i + n : i] = obj[k]; }); return o; };
    widths = mv(widths); flt = mv(flt);
    if (DUP) { if (n < 0 && DUP.c >= at && DUP.c < at - n) DUP = null; else if (DUP.c >= at) DUP.c += n; }
    if (FREEZE > at) FREEZE = Math.max(at, FREEZE + n);
    save();
  }
  function insCol() {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    shiftCols(s.c0, s.c1 - s.c0 + 1);
    send({ type: 'insertCols', c: s.c0, count: s.c1 - s.c0 + 1 });
  }
  function delCol() {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    if (!wholeCols(s)) { showWarn('要刪除整欄：先點欄字母（或按 Ctrl+空白鍵）選整欄，再按 Ctrl+－。'); return; }
    shiftCols(s.c0, -(s.c1 - s.c0 + 1));
    send({ type: 'deleteCols', c0: s.c0, c1: s.c1 });
  }
  function insRow() {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    if (V) shifts = [{ r: s.r0, n: s.r1 - s.r0 + 1 }];   /* (the new rows stay in sight, the rows held move down) */
    send({ type: 'insertRows', r: s.r0, count: s.r1 - s.r0 + 1 });
  }
  /* the runs of data rows in sight inside s, bottom first: [{ r, count }] */
  function shownRuns(s) {
    var out = [], last = Math.min(s.r1, nrows() - 1), r0 = -1;
    for (var r = bodyFrom(s); r <= last + 1; r++) {
      var on = r <= last && shown(r);
      if (on && r0 < 0) r0 = r;
      if (!on && r0 >= 0) { out.push({ r: r0, count: r - r0 }); r0 = -1; }
    }
    return out.reverse();
  }
  /* (by the key only with whole rows selected: Excel asks what to delete otherwise -- here it says how) */
  function delRow(byKey) {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    if (byKey && !(s.c0 === 0 && s.c1 === ncols - 1)) { showWarn('要刪除整列：先點列號（或按 Shift+空白鍵）選整列，再按 Ctrl+－。只選格子時不會刪。'); return; }
    if (s.r0 >= nrows()) return;
    /* (with a filter on: only the rows in sight go -- Excel's "Delete Row" on a filtered list -- one edit) */
    if (V) {
      var runs = shownRuns(s);
      if (!runs.length) return;
      shifts = runs.map(function (x) { return { r: x.r, n: -x.count }; });
      send({ type: 'deleteRows', r: runs[runs.length - 1].r, count: runs[0].r + runs[0].count - runs[runs.length - 1].r, runs: runs });
      return;
    }
    /* 1008 review: not the header -- the menu counted from the first data row (shownRuns) but this took row 0 too: one row
       more than said, and the machine refused the table without its header. Many rows at once (a whole column selected =
       every data row) = asked first, on the extension's side (ask) */
    var rd = bodyFrom(s), last = Math.min(s.r1, nrows() - 1);
    if (rd > last) return;
    send({ type: 'deleteRows', r: rd, count: last - rd + 1, ask: last - rd + 1 >= 20 });
  }
  /* --- the filter list (1006, Excel's AutoFilter): the column's values in the rows the OTHER filters let through,
     each with its count, a search box, (全選), 確定 / 取消 / 清除這一欄的篩選; Enter = 確定, Esc = 取消 --- */
  var fm = null;
  function closeFilter() {
    if (!fm) return;
    if (fm.el.parentNode) fm.el.parentNode.removeChild(fm.el);
    document.removeEventListener('mousedown', fm.away, true);
    fm = null;
  }
  function filterValues(c) {
    var cnt = Object.create(null), out = [];
    for (var r = firstBody(); r < nrows(); r++) {
      var ok = true;
      for (var k in flt) if (+k !== c && flt[k].v[cell(r, +k)] !== 1) { ok = false; break; }
      if (!ok) continue;
      var v = cell(r, c);
      if (!Object.prototype.hasOwnProperty.call(cnt, v)) { cnt[v] = 0; out.push(v); }
      cnt[v]++;
    }
    /* (numbers by value, then the rest A-Z, the empty one last -- Excel's list) */
    var isN = function (v) { return v.trim() !== '' && isFinite(+v); };
    out.sort(function (a, b) {
      if ((a === '') !== (b === '')) return a === '' ? 1 : -1;
      var x = isN(a), y = isN(b); return x && y ? (+a) - (+b) : x !== y ? (x ? -1 : 1) : a.localeCompare(b);
    });
    return { vals: out, cnt: cnt };
  }
  function clearFilters() { closeFilter(); flt = {}; keep = null; refilter(); focusKb(); }
  /* the right-click 依這一格的值篩選 (Excel: Filter > Filter by Selected Cell's Value) */
  function filterByCell() {
    if (header && act.r === 0) { openFilter(act.c); return; }
    var v = Object.create(null); v[cell(act.r, act.c)] = 1;
    flt[act.c] = { v: v }; keep = null; refilter(); focusKb();
  }
  function openFilter(c) {
    closeFilter(); closePick(); closeMenu();
    if (editing) commit();
    if (c < 0 || c >= ncols) return;
    var fv = filterValues(c), was = flt[c];
    var checked = Object.create(null);
    fv.vals.forEach(function (v) { checked[v] = !was || was.v[v] === 1; });
    var el = document.createElement('div');
    el.className = 'fmenu';
    var title = header && D.rows[0] && D.rows[0][c] ? '（' + esc(D.rows[0][c]) + '）' : '';
    el.innerHTML = '<div class="ft">篩選 ' + colName(c) + ' 欄' + title + '</div>' +
      '<div class="fsort"><button class="fasc" title="整個檔案的列依這一欄由小到大排（會先問；Ctrl+Z 復原）">由小到大排序</button><button class="fdesc" title="整個檔案的列依這一欄由大到小排（會先問；Ctrl+Z 復原）">由大到小排序</button></div>' +
      '<input class="fq" type="text" placeholder="搜尋（只留下含這些字的值）" spellcheck="false">' +
      '<div class="fl"></div>' +
      '<div class="fbtn"><button class="fclr"' + (was ? '' : ' disabled') + '>清除這一欄的篩選</button><span class="sp"></span>' +
      '<button class="fok">確定</button><button class="fcan">取消</button></div>';
    var list = el.querySelector('.fl'), qBox = el.querySelector('.fq'), okB = el.querySelector('.fok');
    var listed = function () { var t = qBox.value.toLowerCase(); return t ? fv.vals.filter(function (v) { return v.toLowerCase().indexOf(t) >= 0; }) : fv.vals; };
    var vs = [];
    var render = function () {
      vs = listed().slice(0, 3000);
      var all = vs.length > 0 && vs.every(function (v) { return checked[v]; });
      var h = ['<label class="fo fall"><input type="checkbox" data-fall="1"' + (all ? ' checked' : '') + '>（全選' + (qBox.value ? '搜尋結果' : '') + '）</label>'];
      vs.forEach(function (v, i) {
        h.push('<label class="fo"><input type="checkbox" data-fi="' + i + '"' + (checked[v] ? ' checked' : '') + '><span class="fv">' + (v === '' ? '（空白）' : esc(v)) + '</span><span class="fc">' + fv.cnt[v] + '</span></label>');
      });
      if (!vs.length) h.push('<div class="none">' + (fv.vals.length ? '沒有含「' + esc(qBox.value) + '」的值' : '這一欄沒有值') + '</div>');
      var top = list.scrollTop;
      list.innerHTML = h.join('');
      list.scrollTop = top;
      okB.disabled = !vs.some(function (v) { return checked[v]; });
    };
    var apply = function () {
      if (okB.disabled) return;
      /* with a search typed: the checked values among those found (Excel) */
      var from = qBox.value ? vs : fv.vals, allow = Object.create(null), n = 0;
      from.forEach(function (v) { if (checked[v]) { allow[v] = 1; n++; } });
      if (!n) return;
      if (!qBox.value && n === fv.vals.length) delete flt[c]; else flt[c] = { v: allow };
      keep = null;
      closeFilter();
      refilter();
      focusKb();
    };
    list.addEventListener('change', function (e) {
      var t = e.target;
      if (t.getAttribute('data-fall')) vs.forEach(function (v) { checked[v] = t.checked; });
      else if (t.getAttribute('data-fi') != null) checked[vs[+t.getAttribute('data-fi')]] = t.checked;
      render();
    });
    qBox.addEventListener('input', function () {
      /* (Excel: a search starts with everything it finds checked) */
      if (qBox.value) listed().forEach(function (v) { checked[v] = true; });
      render();
    });
    el.addEventListener('keydown', function (e) {
      if (e.isComposing || e.keyCode === 229) return;
      e.stopPropagation();
      if (e.key === 'Escape') { e.preventDefault(); closeFilter(); focusKb(); }
      else if (e.key === 'Enter' && !(e.target && e.target.tagName === 'BUTTON')) { e.preventDefault(); apply(); }
    });
    okB.addEventListener('click', apply);
    /* 1006 (Excel's Sort A to Z / Z to A, on the same button): the extension asks first -- the rows' order in the file changes */
    var sortBy = function (desc) {
      if (D.readOnly) { closeFilter(); warnRo(); return; }
      closeFilter();
      noCarry = true;   /* (the order changes: the rows held in sight are filtered afresh) */
      send({ type: 'sortRows', from: firstBody(), col: c, desc: desc, colName: colName(c) + ' 欄' + (header && D.rows[0] && D.rows[0][c] ? '（' + D.rows[0][c] + '）' : '') });
      focusKb();
    };
    el.querySelector('.fasc').addEventListener('click', function () { sortBy(false); });
    el.querySelector('.fdesc').addEventListener('click', function () { sortBy(true); });
    el.querySelector('.fcan').addEventListener('click', function () { closeFilter(); focusKb(); });
    el.querySelector('.fclr').addEventListener('click', function () { delete flt[c]; keep = null; closeFilter(); refilter(); focusKb(); });
    render();
    document.body.appendChild(el);
    var hcEl = sizer.querySelector('.hc[data-col="' + c + '"]');
    var b = hcEl ? hcEl.getBoundingClientRect() : { left: 60, bottom: 60 };
    el.style.left = Math.max(0, Math.min(b.left, window.innerWidth - el.offsetWidth - 4)) + 'px';
    el.style.top = Math.max(0, Math.min(b.bottom, window.innerHeight - el.offsetHeight - 4)) + 'px';
    var away = function (e) { if (!el.contains(e.target)) closeFilter(); };
    document.addEventListener('mousedown', away, true);
    fm = { el: el, away: away, c: c };
    qBox.focus();
    return el;
  }
  document.getElementById('fltClr').addEventListener('click', clearFilters);
  document.getElementById('insRow').addEventListener('click', insRow);
  document.getElementById('delRow').addEventListener('click', function () { delRow(false); });
  document.getElementById('asText').addEventListener('click', function () { send({ type: 'openText' }); });
  document.getElementById('replTog').addEventListener('click', function () { if (replBar.hidden) openFind(true); else { replBar.hidden = true; setScope(false); wholeBox.checked = false; runFind(findBox.value, 0); focusKb(); } });
  document.getElementById('replOne').addEventListener('click', replaceOne);
  document.getElementById('replAll').addEventListener('click', replaceAll);
  wholeBox.addEventListener('change', function () { runFind(findBox.value, 0); });
  inSelBox.addEventListener('change', function () { setScope(inSelBox.checked); runFind(findBox.value, 0); });
  /* (1006 audit: the duplicates counted again too -- the title row is in or out of them now) */
  hdrBox.addEventListener('change', function () { header = hdrBox.checked; save(); keep = null; ODD = oddOf(); if (DUP) DUP = dupOf(DUP.c); rebuildView(); clampCur(); draw(); });
  findBox.addEventListener('input', function () { runFind(findBox.value, 0); });
  var findKeys = function (e) {
    if (e.isComposing || e.keyCode === 229) return;
    if (e.key === 'Enter' || e.key === 'F3') { e.preventDefault(); if (e.target === replBox && e.key === 'Enter') return replaceOne(); runFind(findBox.value, e.shiftKey ? -1 : 1); }
    else if (e.key === 'Escape') { e.preventDefault(); findBox.value = ''; replBar.hidden = true; setScope(false); wholeBox.checked = false; runFind('', 0); focusKb(); }
    else if ((e.ctrlKey || e.metaKey) && (e.key === 'h' || e.key === 'H')) { e.preventDefault(); openFind(true); }
  };
  findBox.addEventListener('keydown', findKeys);
  replBox.addEventListener('keydown', findKeys);

  window.addEventListener('message', function (ev) {
    var m = ev.data || {};
    if (m.type === 'data') {
      /* 1008 review: the file changed while a cell was being typed in (the machine saved its parameters, git, another tool)
         -- the cell's row number may be another row now, and Enter went with the NEW version, past the version check:
         the value landed on another axis. The typing is dropped, said */
      var wasEditing = !!(editing && D && typeof m.ver === 'number' && D.ver !== null && m.ver !== D.ver && !m.mine);
      if (wasEditing) { editing = null; toIdle(true); }
      D = { rows: m.rows || [], readOnly: !!m.readOnly, why: m.why || '', file: m.file || '', delim: m.delim || ',', ver: typeof m.ver === 'number' ? m.ver : null };
      if (wasEditing) setTimeout(function () { showWarn('檔案剛被別的地方改了：正在輸入的那一格沒有寫入（列可能移動了），請重新輸入。'); }, 0);
      ncols = 1;
      D.rows.forEach(function (r) { if (r.length > ncols) ncols = r.length; });
      if (header === undefined || header === null) header = guessHeader();
      hdrBox.checked = !!header;
      autoWidths();
      document.getElementById('name').textContent = D.file;
      /* (a filter stays on over an edit; the rows in sight stay in sight) */
      Object.keys(flt).forEach(function (k) { if (+k >= ncols) delete flt[k]; });
      if (noCarry) { noCarry = false; keep = null; shifts = null; } else carryKeep();
      if (DUP) DUP = DUP.c < ncols ? dupOf(DUP.c) : null;   /* (the duplicates counted again on the new data) */
      /* 1008 review: a cut whose cells are not what was cut any more (a row inserted / deleted above, Ctrl+Z) is forgotten
         -- the paste used to clear the cells now at those row numbers, another row's, and leave the cut ones in place */
      if (cut) { try { if (tsvOf({ r0: cut.r0, c0: cut.c0, r1: cut.r1, c1: cut.c1 }) !== cut.text) cut = null; } catch (e) { cut = null; } }
      ODD = oddOf();
      rebuildView();
      showWarn(D.readOnly ? D.why : '');
      if (m.first && st.cur) { cur = { r: st.cur.r, c: st.cur.c }; anchor = { r: cur.r, c: cur.c }; act = { r: cur.r, c: cur.c }; }
      clampCur();
      if (find.q) runFind(find.q, 0);
      draw();
      if (m.first) { scrollTo(act.r, act.c); draw(); focusKb(); }
    } else if (m.type === 'select') {
      setRect(m.r | 0, m.c | 0, m.r1 != null ? m.r1 | 0 : m.r | 0, m.c1 != null ? m.c1 | 0 : m.c | 0);
      scrollTo(act.r, act.c);
      draw();
    } else if (m.type === 'say') showWarn(m.text || '');
    else if (m.type === 'clip') {
      /* the menu's 貼上: the clipboard read by the extension (a webview cannot read it itself) */
      if (!m.text) showWarn('剪貼簿是空的，沒有東西可以貼上。');
      else if (D.readOnly) warnRo();
      else pasteText(m.text);
    }
  });
  /* for the tests */
  window.__htdCsv = {
    state: function () { return { cur: cur, anchor: anchor, act: act, header: !!header, rows: nrows(), cols: ncols, editing: !!editing, typed: !!(editing && editing.typed), cut: !!cut, drawnRows: drawn.b - drawn.a + 1,
      filtered: V ? V.filter(function (r) { return r < nrows(); }) : null, filterCols: Object.keys(flt).map(Number), filterOpen: fm ? fm.c : null }; },
    drawnRowNums: function () { return Array.prototype.map.call(sizer.querySelectorAll('.row:not(.frozen) .rn'), function (x) { return x.textContent; }); },
    cellText: function (r, c) { var el = sizer.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]'); return el ? el.textContent : null; },
    status: function () { return document.getElementById('status').textContent; },
    cutText: function () { return cut ? cut.text : null; }
  };
  send({ type: 'ready' });
})();
