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
  var widths = st.widths || {};
  /* the selection is the block anchor..cur; act = the active cell inside it (Excel's white cell) */
  var cur = { r: 0, c: 0 }, anchor = { r: 0, c: 0 }, act = { r: 0, c: 0 };
  var editing = null;          /* { r, c, orig, typed } -- the editor is `kb`; typed = Enter mode */
  var find = { q: '', hits: [], i: -1, whole: false, rect: null };
  var cut = null;              /* { r0, c0, r1, c1, text }: Ctrl+X, moved when pasted */
  var tabStart = null;         /* the column a Tab..Tab run started in (Enter goes back to it) */
  var ncols = 1;

  var root = document.getElementById('root');
  root.innerHTML =
    '<div class="bar">' +
      '<input id="nameBox" class="namebox" type="text" spellcheck="false" autocomplete="off" aria-label="名稱方塊" ' +
        'title="名稱方塊（Excel）：目前的格子。打 C12、B2:D5 或列號（12）再按 Enter＝跳過去並選取；Ctrl+G 也會到這裡">' +
      '<span id="name" class="name"></span><span id="size" class="size"></span>' +
      '<label class="chk" title="第一列是欄位名稱：固定在上面，捲動時一直看得到"><input type="checkbox" id="hdr">第一列是標題</label>' +
      '<button id="insRow" title="在目前這列上面插入一列（Ctrl+Shift+＋）">插入列</button>' +
      '<button id="delRow" title="刪除選取的列（選整列後 Ctrl+－）">刪除列</button>' +
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
  function colX(c) { var x = RNW; for (var i = 0; i < c; i++) x += colW(i); return x; }
  function totalW() { var x = RNW; for (var i = 0; i < ncols; i++) x += colW(i); return x; }
  function rowY(r) { return (r - firstBody()) * RH + RH * (header ? 2 : 1); }
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
  function save() { api.setState({ header: header, widths: widths, cur: act }); }
  function warnRo() { showWarn(D.why || '這個檔案現在不能編輯。'); }

  /* --- drawing: only what is in sight --- */
  var drawn = { a: -1, b: -1 };
  function draw() {
    var top = wrap.scrollTop, h = wrap.clientHeight || 400;
    var fb = firstBody();
    var a = Math.max(fb, fb + Math.floor((top - RH * (header ? 2 : 1)) / RH) - 8);
    var b = Math.min(lastRow(), a + Math.ceil(h / RH) + 16);
    sizer.style.height = (rowY(Math.max(lastRow(), 0) + 1) + RH) + 'px';
    sizer.style.width = (totalW() + 40) + 'px';
    var html = [];
    /* the column letters (and the frozen first row) stay on top */
    html.push('<div class="hrow letters" style="top:' + top + 'px;width:' + totalW() + 'px">');
    html.push('<div class="rn corner" style="left:' + wrap.scrollLeft + 'px" data-all="1"></div>');
    for (var c = 0; c < ncols; c++) html.push('<div class="hc' + (inSelC(c) ? ' on' : '') + '" data-col="' + c + '" style="left:' + colX(c) + 'px;width:' + colW(c) + 'px">' + colName(c) + '<span class="rs" data-rs="' + c + '"></span></div>');
    html.push('</div>');
    if (header && nrows()) html.push(rowHtml(0, top + RH, true));
    for (var r = a; r <= b; r++) html.push(rowHtml(r, rowY(r), false));
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
  function rowHtml(r, y, frozen) {
    var phantom = r >= nrows();
    var s = ['<div class="row' + (frozen ? ' frozen' : '') + (phantom ? ' phantom' : '') + '" style="top:' + y + 'px;width:' + totalW() + 'px">'];
    s.push('<div class="rn' + (inSelR(r) ? ' on' : '') + '" data-row="' + r + '" style="left:' + wrap.scrollLeft + 'px"' +
      (phantom ? ' title="最後一列下面的空白列：在這裡打字＝新增一列"' : '') + '>' + (phantom ? '＋' : r + 1) + '</div>');
    var row = D.rows[r] || [];
    for (var c = 0; c < ncols; c++) {
      var v = c < row.length ? row[c] : '';
      var cls = 'cell' + (c >= row.length ? ' none' : '') + (inSel(r, c) ? ' sel' : '') + (r === act.r && c === act.c ? ' cur' : '') +
        (inRect(cut, r, c) ? ' cut' : '') + (find.q && match(v) ? ' hit' : '') + (isNum(v) ? ' num' : '');
      s.push('<div class="' + cls + '" data-r="' + r + '" data-c="' + c + '" style="left:' + colX(c) + 'px;width:' + colW(c) + 'px" title="' + esc(v).replace(/"/g, '&quot;') + '">' + esc(v) + '</div>');
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
      (cut ? '剪下了（貼上＝搬過去，Esc 取消）　' : '') + hint;
  }

  /* --- moving --- */
  function clamp(x, a, b) { return Math.max(a, Math.min(b, x)); }
  function clampCur() {
    var R = Math.max(0, lastRow()), C = Math.max(0, ncols - 1);
    [cur, anchor, act].forEach(function (p) { p.r = clamp(p.r, 0, R); p.c = clamp(p.c, 0, C); });
  }
  function scrollTo(r, c) {
    var y = rowY(r), top = wrap.scrollTop, hh = RH * (header ? 2 : 1);
    if (!(header && r === 0)) {
      if (y - hh < top) wrap.scrollTop = y - hh;
      else if (y + RH > top + wrap.clientHeight) wrap.scrollTop = y + RH - wrap.clientHeight;
    }
    var x = colX(c), left = wrap.scrollLeft;
    if (x - RNW < left) wrap.scrollLeft = x - RNW;
    else if (x + colW(c) > left + wrap.clientWidth) wrap.scrollLeft = x + colW(c) - wrap.clientWidth;
  }
  /** One cell (the selection and the active cell there), or (extend) the block from the anchor to it. */
  function go(r, c, extend) {
    cur = { r: r, c: c };
    if (!extend) { anchor = { r: r, c: c }; act = { r: r, c: c }; }
    clampCur();
    scrollTo(cur.r, cur.c);
    draw();
    save();
  }
  function setRect(r0, c0, r1, c1, ar, ac) {
    anchor = { r: r0, c: c0 }; cur = { r: r1, c: c1 }; act = { r: ar == null ? r0 : ar, c: ac == null ? c0 : ac };
    clampCur();
  }
  function multi() { return cur.r !== anchor.r || cur.c !== anchor.c; }
  /* Excel: Enter / Tab inside a selected block move the active cell and keep the block (down then the next
     column / right then the next row; Shift = back; round at the end) */
  function stepInSel(byRow, back) {
    var s = selRect(), r = act.r, c = act.c, d = back ? -1 : 1;
    if (byRow) {
      c += d;
      if (c > s.c1) { c = s.c0; r = r + 1 > s.r1 ? s.r0 : r + 1; }
      else if (c < s.c0) { c = s.c1; r = r - 1 < s.r0 ? s.r1 : r - 1; }
    } else {
      r += d;
      if (r > s.r1) { r = s.r0; c = c + 1 > s.c1 ? s.c0 : c + 1; }
      else if (r < s.r0) { r = s.r1; c = c - 1 < s.c0 ? s.c1 : c - 1; }
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
    go(act.r + (back ? -1 : 1), c);
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
    var inb = function (y, x) { return y >= 0 && y <= R && x >= 0 && x <= C; };
    var filled = function (y, x) { return cell(y, x) !== ''; };
    var y = r + dr, x = c + dc;
    if (!inb(y, x)) return { r: clamp(r, 0, Math.max(R, 0)), c: c };
    if (filled(r, c) && filled(y, x)) {
      while (inb(y + dr, x + dc) && filled(y + dr, x + dc)) { y += dr; x += dc; }
      return { r: y, c: x };
    }
    while (!filled(y, x) && inb(y + dr, x + dc)) { y += dr; x += dc; }
    return { r: y, c: x };
  }

  /* --- editing --- */
  function send(m) { api.postMessage(m); }
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
    s.left = (right ? colX(c) + colW(c) - w : colX(c)) + 'px'; s.top = (header && r === 0 ? wrap.scrollTop + RH : rowY(r)) + 'px';
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
    if (ctrl && (k === 'z' || k === 'Z' || k === 'y' || k === 'Y')) { e.preventDefault(); return cancel(true); }
    if (k === 'F2') { e.preventDefault(); editing.typed = !editing.typed; placeKb(); return status(); }
    if (editing.typed && /^Arrow/.test(k)) {
      /* Enter mode (typed over): an arrow writes it and moves, like Excel */
      e.preventDefault();
      commit();
      tabStart = null;
      return go(act.r + (k === 'ArrowUp' ? -1 : k === 'ArrowDown' ? 1 : 0), act.c + (k === 'ArrowLeft' ? -1 : k === 'ArrowRight' ? 1 : 0));
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
    for (var r = s.r0; r <= s.r1; r++) for (var c = s.c0; c <= s.c1; c++) ch.push({ r: r, c: c, v: v });
    setCells(ch);
    draw();
  }
  function cancel(byUndoKey) {
    if (!editing) return;
    editing = null;
    toIdle(true, byUndoKey);
    draw();
  }
  function selRect() {
    return { r0: Math.min(cur.r, anchor.r), r1: Math.max(cur.r, anchor.r), c0: Math.min(cur.c, anchor.c), c1: Math.max(cur.c, anchor.c) };
  }
  function clearSel() {
    var s = selRect(), ch = [];
    for (var r = s.r0; r <= s.r1; r++) for (var c = s.c0; c <= s.c1; c++) if (D.rows[r] && c < D.rows[r].length && D.rows[r][c] !== '') ch.push({ r: r, c: c, v: '' });
    setCells(ch);
  }
  /* the block as Excel's clipboard text: tab between cells, CRLF between rows, quoted when it has to be */
  function q(v) { return /[\t\r\n"]/.test(v) ? '"' + String(v).replace(/"/g, '""') + '"' : String(v); }
  function tsvOf(s) {
    var rows = [];
    for (var r = s.r0; r <= s.r1; r++) { var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(q(cell(r, c))); rows.push(line.join('\t')); }
    return rows.join('\r\n');
  }
  function gridOf(s) {
    var g = [];
    for (var r = s.r0; r <= s.r1; r++) { var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(cell(r, c)); g.push(line); }
    return g;
  }
  function copySel() { cut = null; send({ type: 'copy', grid: gridOf(selRect()) }); draw(); }
  /* Ctrl+X: copied now, moved when pasted (Excel's "moving border"), Esc forgets it */
  function cutSel() {
    var s = selRect();
    send({ type: 'copy', grid: gridOf(s) });
    if (D.readOnly) { cut = null; warnRo(); return; }
    cut = { r0: s.r0, c0: s.c0, r1: s.r1, c1: s.c1, text: tsvOf(s) };
    draw();
  }
  var norm = function (t) { return String(t || '').replace(/\r\n|\r/g, '\n').replace(/\n$/, ''); };
  function pasteText(t) {
    var s = selRect();
    var m = { type: 'paste', text: t, r: s.r0, c: s.c0, r1: s.r1, c1: s.c1 };
    if (cut && norm(t) === norm(cut.text)) m.cut = { r0: cut.r0, c0: cut.c0, r1: cut.r1, c1: cut.c1 };
    cut = null;
    send(m);
  }
  /* Ctrl+D / Ctrl+R: the first row (column) of the block into the rest; one row (column) = from the one above (left) */
  function fill(down) {
    var s = selRect(), ch = [];
    var one = down ? s.r0 === s.r1 : s.c0 === s.c1;
    var src = down ? (one ? s.r0 - 1 : s.r0) : (one ? s.c0 - 1 : s.c0);
    if (src < 0) { showWarn(down ? '上面沒有列可以往下填' : '左邊沒有欄可以往右填'); return; }
    if (down && header && src === 0) { showWarn('上面是標題列，不往下填'); return; }
    if (down) { for (var r = one ? s.r0 : s.r0 + 1; r <= s.r1; r++) for (var c = s.c0; c <= s.c1; c++) ch.push({ r: r, c: c, v: cell(src, c) }); }
    else { for (var r2 = s.r0; r2 <= s.r1; r2++) for (var c2 = one ? s.c0 : s.c0 + 1; c2 <= s.c1; c2++) ch.push({ r: r2, c: c2, v: cell(r2, src) }); }
    setCells(ch);
  }
  function selectRows() { var s = selRect(); setRect(s.r0, 0, s.r1, ncols - 1, act.r, act.c); draw(); }
  /* (the frozen title row is not part of a column: a Ctrl+Enter on a column never writes over the column names) */
  function colTop() { return firstBody() < nrows() ? firstBody() : 0; }
  function selectCols() { var s = selRect(); setRect(colTop(), s.c0, Math.max(nrows() - 1, 0), s.c1, Math.max(act.r, colTop()), act.c); draw(); }

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
    var ch = find.hits.map(function (h) { return { r: h.r, c: h.c, v: replaced(cell(h.r, h.c), by) }; });
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
    var nIns = s.r1 - s.r0 + 1, nDel = Math.min(s.r1, nrows() - 1) - s.r0 + 1;
    var items = [
      ['剪下', 'Ctrl+X', !ro, cutSel],
      ['複製', 'Ctrl+C', true, copySel],
      ['貼上', 'Ctrl+V', !ro, function () { send({ type: 'readClip' }); }],
      null,
      ['插入 ' + nIns + ' 列（在上面）', 'Ctrl+Shift++', !ro, insRow],
      ['刪除 ' + Math.max(nDel, 0) + ' 列', 'Ctrl+－', !ro && nDel > 0, function () { delRow(false); }],
      ['清除內容', 'Delete', !ro, clearSel],
      null,
      ['選取整列', 'Shift+空白鍵', true, selectRows],
      ['選取整欄', 'Ctrl+空白鍵', true, selectCols],
      null,
      ['尋找…', 'Ctrl+F', true, function () { openFind(false); }],
      ['取代…', 'Ctrl+H', !ro, function () { openFind(true); }]
    ];
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
    if (k === 'ArrowDown' && e.altKey && !ctrl) { e.preventDefault(); return openPick(); }
    var page = Math.max(1, Math.floor(wrap.clientHeight / RH) - 2);
    /* plain: from the active cell; Shift: the far corner of the block moves */
    var from = sh ? cur : act;
    var mv = function (dr, dc) {
      e.preventDefault();
      tabStart = null;
      var to = ctrl ? edge(from.r, from.c, dr, dc) : { r: from.r + dr, c: from.c + dc };
      go(to.r, to.c, sh);
    };
    if (k === 'ArrowUp') return mv(-1, 0);
    if (k === 'ArrowDown') return mv(1, 0);
    if (k === 'ArrowLeft') return mv(0, -1);
    if (k === 'ArrowRight') return mv(0, 1);
    if (k === 'PageUp') { e.preventDefault(); return go(from.r - page, from.c, sh); }
    if (k === 'PageDown') { e.preventDefault(); return go(from.r + page, from.c, sh); }
    if (k === 'Home') { e.preventDefault(); return go(ctrl ? 0 : from.r, 0, sh); }
    if (k === 'End') { e.preventDefault(); return go(ctrl ? Math.max(nrows() - 1, 0) : from.r, ncols - 1, sh); }
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
    if (ctrl && sh && (k === '+' || k === '=')) { e.preventDefault(); return insRow(); }
    if (ctrl && !sh && k === '-') { e.preventDefault(); return delRow(true); }
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

  function insRow() {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    send({ type: 'insertRows', r: s.r0, count: s.r1 - s.r0 + 1 });
  }
  /* (by the key only with whole rows selected: Excel asks what to delete otherwise -- here it says how) */
  function delRow(byKey) {
    if (D.readOnly) { warnRo(); return; }
    var s = selRect();
    if (byKey && !(s.c0 === 0 && s.c1 === ncols - 1)) { showWarn('要刪除整列：先點列號（或按 Shift+空白鍵）選整列，再按 Ctrl+－。只選格子時不會刪。'); return; }
    if (s.r0 >= nrows()) return;
    send({ type: 'deleteRows', r: s.r0, count: Math.min(s.r1, nrows() - 1) - s.r0 + 1 });
  }
  document.getElementById('insRow').addEventListener('click', insRow);
  document.getElementById('delRow').addEventListener('click', function () { delRow(false); });
  document.getElementById('asText').addEventListener('click', function () { send({ type: 'openText' }); });
  document.getElementById('replTog').addEventListener('click', function () { if (replBar.hidden) openFind(true); else { replBar.hidden = true; setScope(false); wholeBox.checked = false; runFind(findBox.value, 0); focusKb(); } });
  document.getElementById('replOne').addEventListener('click', replaceOne);
  document.getElementById('replAll').addEventListener('click', replaceAll);
  wholeBox.addEventListener('change', function () { runFind(findBox.value, 0); });
  inSelBox.addEventListener('change', function () { setScope(inSelBox.checked); runFind(findBox.value, 0); });
  hdrBox.addEventListener('change', function () { header = hdrBox.checked; save(); draw(); });
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
      D = { rows: m.rows || [], readOnly: !!m.readOnly, why: m.why || '', file: m.file || '', delim: m.delim || ',' };
      ncols = 1;
      D.rows.forEach(function (r) { if (r.length > ncols) ncols = r.length; });
      if (header === undefined || header === null) header = guessHeader();
      hdrBox.checked = !!header;
      autoWidths();
      document.getElementById('name').textContent = D.file;
      document.getElementById('size').textContent = nrows() + ' 列 × ' + ncols + ' 欄　分隔：' + (D.delim === '\t' ? 'Tab' : D.delim);
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
    state: function () { return { cur: cur, anchor: anchor, act: act, header: !!header, rows: nrows(), cols: ncols, editing: !!editing, typed: !!(editing && editing.typed), cut: !!cut, drawnRows: drawn.b - drawn.a + 1 }; },
    cellText: function (r, c) { var el = sizer.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]'); return el ? el.textContent : null; },
    status: function () { return document.getElementById('status').textContent; },
    cutText: function () { return cut ? cut.text : null; }
  };
  send({ type: 'ready' });
})();
