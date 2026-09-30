/* AI(W906-HTDESIGNER) 20260930: CSV 表格 -- a .csv as a table, edited like Excel.
 * EastSun: "你外掛軟體可以包含開啟CSV時可以像excel有表格可以編輯嗎?"
 * The extension parses the file and writes every change as a replacement of only the cells that
 * changed (lib/csvtable.js), into VS Code's document: Ctrl+S saves, Ctrl+Z undoes.
 * Keys like Excel: arrows / Tab / Enter move, typing or F2 edits, Esc cancels, Delete clears,
 * Ctrl+C / Ctrl+V copy / paste (tab separated, to and from Excel), Ctrl+A all, Ctrl+F find.
 * Only the rows in sight are drawn (a log with 100 000 lines stays quick). */
(function () {
  'use strict';
  var api = acquireVsCodeApi();
  var st = api.getState() || {};
  var RH = 22;                 /* row height */
  var RNW = 52;                /* the row-number column */
  var D = { rows: [], readOnly: false, why: '', file: '', delim: ',' };
  var header = st.header;      /* the first row is the column names (frozen): undefined = guess */
  var widths = st.widths || {};
  var cur = { r: 0, c: 0 }, anchor = { r: 0, c: 0 };
  var editing = null;          /* { r, c, input } */
  var find = { q: '', hits: [], i: -1 };
  var ncols = 1;

  var root = document.getElementById('root');
  root.innerHTML =
    '<div class="bar">' +
      '<span id="name" class="name"></span><span id="size" class="size"></span>' +
      '<label class="chk" title="第一列是欄位名稱：固定在上面，捲動時一直看得到"><input type="checkbox" id="hdr">第一列是標題</label>' +
      '<button id="insRow" title="在目前這列上面插入一列（Ctrl+Shift+＋）">插入列</button>' +
      '<button id="delRow" title="刪除選取的列（Ctrl+－）">刪除列</button>' +
      '<span class="sp"></span>' +
      '<input id="find" type="text" placeholder="尋找（Ctrl+F，Enter 下一個）" spellcheck="false">' +
      '<span id="findN" class="findN"></span>' +
      '<button id="asText" title="用文字編輯器開這個檔">以文字開啟</button>' +
    '</div>' +
    '<div id="warn" class="warn" hidden></div>' +
    '<div id="wrap" class="wrap" tabindex="0"><div id="sizer" class="sizer"><div id="grid"></div></div></div>' +
    '<div id="status" class="status"></div>';
  var wrap = document.getElementById('wrap'), sizer = document.getElementById('sizer'), grid = document.getElementById('grid');
  var hdrBox = document.getElementById('hdr'), findBox = document.getElementById('find');

  function colName(c) { var s = ''; c++; while (c > 0) { var m = (c - 1) % 26; s = String.fromCharCode(65 + m) + s; c = Math.floor((c - 1) / 26); } return s; }
  function cell(r, c) { var row = D.rows[r]; return row && c < row.length ? row[c] : ''; }
  function firstBody() { return header ? 1 : 0; }
  function nrows() { return D.rows.length; }
  function colW(c) { return widths[c] || 80; }
  function colX(c) { var x = RNW; for (var i = 0; i < c; i++) x += colW(i); return x; }
  function totalW() { var x = RNW; for (var i = 0; i < ncols; i++) x += colW(i); return x; }
  function rowY(r) { return (r - firstBody()) * RH + RH * (header ? 2 : 1); }
  function guessHeader() {
    if (D.rows.length < 2) return false;
    var a = D.rows[0], b = D.rows[1];
    var textual = a.filter(function (v) { return v !== ''; }).every(function (v) { return !/^[-+]?\d+(\.\d+)?$/.test(v); });
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
  function save() { api.setState({ header: header, widths: widths, cur: cur }); }

  /* --- drawing: only what is in sight --- */
  var drawn = { a: -1, b: -1 };
  function draw() {
    var top = wrap.scrollTop, h = wrap.clientHeight || 400;
    var fb = firstBody();
    var a = Math.max(fb, fb + Math.floor((top - RH * (header ? 2 : 1)) / RH) - 8);
    var b = Math.min(nrows() - 1, a + Math.ceil(h / RH) + 16);
    sizer.style.height = (rowY(nrows()) + RH) + 'px';
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
    if (editing) placeEditor();
    status();
  }
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }
  function inSel(r, c) {
    return r >= Math.min(cur.r, anchor.r) && r <= Math.max(cur.r, anchor.r) && c >= Math.min(cur.c, anchor.c) && c <= Math.max(cur.c, anchor.c);
  }
  function inSelC(c) { return c >= Math.min(cur.c, anchor.c) && c <= Math.max(cur.c, anchor.c); }
  function inSelR(r) { return r >= Math.min(cur.r, anchor.r) && r <= Math.max(cur.r, anchor.r); }
  function rowHtml(r, y, frozen) {
    var s = ['<div class="row' + (frozen ? ' frozen' : '') + '" style="top:' + y + 'px;width:' + totalW() + 'px">'];
    s.push('<div class="rn' + (inSelR(r) ? ' on' : '') + '" data-row="' + r + '" style="left:' + wrap.scrollLeft + 'px">' + (r + 1) + '</div>');
    var row = D.rows[r] || [];
    for (var c = 0; c < ncols; c++) {
      var v = c < row.length ? row[c] : '';
      var cls = 'cell' + (c >= row.length ? ' none' : '') + (inSel(r, c) ? ' sel' : '') + (r === cur.r && c === cur.c ? ' cur' : '') +
        (find.q && v.toLowerCase().indexOf(find.q) >= 0 ? ' hit' : '') + (/^[-+]?\d+(\.\d+)?$/.test(v) ? ' num' : '');
      s.push('<div class="' + cls + '" data-r="' + r + '" data-c="' + c + '" style="left:' + colX(c) + 'px;width:' + colW(c) + 'px" title="' + esc(v).replace(/"/g, '&quot;') + '">' + esc(v) + '</div>');
    }
    s.push('</div>');
    return s.join('');
  }
  function status() {
    var el = document.getElementById('status');
    var n = (Math.abs(cur.r - anchor.r) + 1) * (Math.abs(cur.c - anchor.c) + 1);
    var name = header && D.rows[0] && D.rows[0][cur.c] ? '（' + D.rows[0][cur.c] + '）' : '';
    el.textContent = colName(cur.c) + (cur.r + 1) + name + '　' + (n > 1 ? '選了 ' + n + ' 格　' : '') +
      (D.readOnly ? '唯讀' : '雙擊或直接打字＝編輯，Enter 確定，Esc 取消；Ctrl+S 存檔，Ctrl+Z 復原');
  }

  /* --- moving --- */
  function clampCur() {
    cur.r = Math.max(0, Math.min(nrows() - 1, cur.r));
    cur.c = Math.max(0, Math.min(ncols - 1, cur.c));
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
  function go(r, c, extend) {
    cur = { r: r, c: c };
    clampCur();
    if (!extend) anchor = { r: cur.r, c: cur.c };
    scrollTo(cur.r, cur.c);
    draw();
    save();
  }

  /* --- editing --- */
  function send(m) { api.postMessage(m); }
  function setCells(changes) {
    if (D.readOnly) { showWarn(D.why || '這個檔案現在不能編輯。'); return; }
    changes = changes.filter(function (ch) { return cell(ch.r, ch.c) !== ch.v || !D.rows[ch.r] || ch.c >= D.rows[ch.r].length; });
    if (!changes.length) return;
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
  function startEdit(initial) {
    if (D.readOnly) { showWarn(D.why || '這個檔案現在不能編輯。'); return; }
    var inp = document.createElement('input');
    inp.className = 'ed';
    inp.spellcheck = false;
    inp.value = initial != null ? initial : cell(cur.r, cur.c);
    editing = { r: cur.r, c: cur.c, input: inp, orig: cell(cur.r, cur.c) };
    placeEditor();
    inp.focus();
    if (initial == null) inp.select(); else inp.setSelectionRange(inp.value.length, inp.value.length);
    inp.addEventListener('keydown', function (e) {
      if (e.key === 'Enter') { e.preventDefault(); e.stopPropagation(); commit(); go(cur.r + (e.shiftKey ? -1 : 1), cur.c); }
      else if (e.key === 'Tab') { e.preventDefault(); e.stopPropagation(); commit(); go(cur.r, cur.c + (e.shiftKey ? -1 : 1)); }
      else if (e.key === 'Escape') { e.preventDefault(); e.stopPropagation(); cancel(); }
      else if ((e.key === 'ArrowUp' || e.key === 'ArrowDown') && initial != null) {
        /* typed over (not F2): an arrow commits and moves, like Excel */
        e.preventDefault(); e.stopPropagation(); commit(); go(cur.r + (e.key === 'ArrowUp' ? -1 : 1), cur.c);
      } else e.stopPropagation();
    });
    inp.addEventListener('blur', function () { if (editing && editing.input === inp) commit(); });
  }
  function placeEditor() {
    if (!editing) return;
    var y = header && editing.r === 0 ? wrap.scrollTop + RH : rowY(editing.r);
    var s = editing.input.style;
    s.left = colX(editing.c) + 'px'; s.top = y + 'px';
    s.width = Math.max(colW(editing.c), 120) + 'px'; s.height = RH + 'px';
    if (editing.input.parentNode !== sizer) sizer.appendChild(editing.input);
  }
  function commit() {
    if (!editing) return;
    var e = editing;
    editing = null;
    var v = e.input.value;
    if (e.input.parentNode) e.input.parentNode.removeChild(e.input);
    if (v !== e.orig) setCells([{ r: e.r, c: e.c, v: v }]);
    else draw();
    wrap.focus();
  }
  function cancel() {
    if (!editing) return;
    var e = editing;
    editing = null;
    if (e.input.parentNode) e.input.parentNode.removeChild(e.input);
    draw();
    wrap.focus();
  }
  function selRect() {
    return { r0: Math.min(cur.r, anchor.r), r1: Math.max(cur.r, anchor.r), c0: Math.min(cur.c, anchor.c), c1: Math.max(cur.c, anchor.c) };
  }
  function clearSel() {
    var s = selRect(), ch = [];
    for (var r = s.r0; r <= s.r1; r++) for (var c = s.c0; c <= s.c1; c++) if (D.rows[r] && c < D.rows[r].length && D.rows[r][c] !== '') ch.push({ r: r, c: c, v: '' });
    setCells(ch);
  }
  function copySel() {
    var s = selRect(), grid = [];
    for (var r = s.r0; r <= s.r1; r++) { var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(cell(r, c)); grid.push(line); }
    send({ type: 'copy', grid: grid });
  }
  function pasteText(t) {
    send({ type: 'paste', text: t, r: Math.min(cur.r, anchor.r), c: Math.min(cur.c, anchor.c) });
  }

  /* --- find --- */
  function runFind(q, next) {
    find.q = String(q || '').toLowerCase();
    find.hits = [];
    if (find.q) {
      for (var r = 0; r < nrows(); r++) {
        var row = D.rows[r];
        for (var c = 0; c < row.length; c++) if (row[c].toLowerCase().indexOf(find.q) >= 0) find.hits.push({ r: r, c: c });
      }
    }
    var n = document.getElementById('findN');
    if (!find.hits.length) { find.i = -1; n.textContent = find.q ? '找不到' : ''; draw(); return; }
    if (next) {
      var at = -1;
      for (var i = 0; i < find.hits.length; i++) { var h = find.hits[i]; if (h.r > cur.r || (h.r === cur.r && h.c > cur.c)) { at = i; break; } }
      find.i = at < 0 ? 0 : at;
      go(find.hits[find.i].r, find.hits[find.i].c);
    } else draw();
    n.textContent = find.hits.length + ' 個' + (find.i >= 0 ? '（第 ' + (find.i + 1) + ' 個）' : '');
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
    if (t.getAttribute('data-rs') != null) { rsCol = +t.getAttribute('data-rs'); rsX = e.clientX; rsW = colW(rsCol); e.preventDefault(); return; }
    if (editing && t === editing.input) return;
    if (editing) commit();
    if (t.getAttribute('data-all')) { anchor = { r: 0, c: 0 }; cur = { r: nrows() - 1, c: ncols - 1 }; draw(); return; }
    if (t.getAttribute('data-col') != null) { var cc = +t.getAttribute('data-col'); anchor = { r: 0, c: e.shiftKey ? anchor.c : cc }; cur = { r: nrows() - 1, c: cc }; draw(); return; }
    if (t.getAttribute('data-row') != null) { var rr = +t.getAttribute('data-row'); anchor = { r: e.shiftKey ? anchor.r : rr, c: 0 }; cur = { r: rr, c: ncols - 1 }; draw(); return; }
    if (t.getAttribute('data-r') == null) return;
    e.preventDefault();
    wrap.focus();
    cur = { r: +t.getAttribute('data-r'), c: +t.getAttribute('data-c') };
    if (!e.shiftKey) anchor = { r: cur.r, c: cur.c };
    dragSel = true;
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
  sizer.addEventListener('dblclick', function (e) {
    var t = e.target;
    if (t.getAttribute('data-r') == null) return;
    cur = anchor = { r: +t.getAttribute('data-r'), c: +t.getAttribute('data-c') };
    startEdit(null);
  });
  var raf = 0;
  wrap.addEventListener('scroll', function () { if (drawn.a >= 0 && !raf) raf = requestAnimationFrame(function () { raf = 0; draw(); }); });
  window.addEventListener('resize', draw);

  /* --- keys --- */
  wrap.addEventListener('keydown', function (e) {
    if (editing) return;
    var k = e.key, ctrl = e.ctrlKey || e.metaKey, sh = e.shiftKey;
    var page = Math.max(1, Math.floor(wrap.clientHeight / RH) - 2);
    var mv = function (dr, dc) { e.preventDefault(); go(cur.r + dr, cur.c + dc, sh); };
    if (k === 'ArrowUp') return mv(ctrl ? -cur.r : -1, 0);
    if (k === 'ArrowDown') return mv(ctrl ? nrows() : 1, 0);
    if (k === 'ArrowLeft') return mv(0, ctrl ? -cur.c : -1);
    if (k === 'ArrowRight') return mv(0, ctrl ? ncols : 1);
    if (k === 'PageUp') return mv(-page, 0);
    if (k === 'PageDown') return mv(page, 0);
    if (k === 'Home') { e.preventDefault(); return go(ctrl ? 0 : cur.r, 0, sh); }
    if (k === 'End') { e.preventDefault(); return go(ctrl ? nrows() - 1 : cur.r, ncols - 1, sh); }
    if (k === 'Tab') { e.preventDefault(); return go(cur.r, cur.c + (sh ? -1 : 1)); }
    if (k === 'Enter') { e.preventDefault(); return go(cur.r + (sh ? -1 : 1), cur.c); }
    if (k === 'F2') { e.preventDefault(); return startEdit(null); }
    if (k === 'Delete' || k === 'Backspace') { e.preventDefault(); return clearSel(); }
    if (ctrl && (k === 'c' || k === 'C')) { e.preventDefault(); return copySel(); }
    if (ctrl && (k === 'x' || k === 'X')) { e.preventDefault(); copySel(); return clearSel(); }
    if (ctrl && (k === 'a' || k === 'A')) { e.preventDefault(); anchor = { r: 0, c: 0 }; cur = { r: nrows() - 1, c: ncols - 1 }; return draw(); }
    if (ctrl && (k === 'f' || k === 'F')) { e.preventDefault(); findBox.focus(); findBox.select(); return; }
    if (k === 'F3') { e.preventDefault(); return runFind(findBox.value, true); }
    if (ctrl && sh && (k === '+' || k === '=')) { e.preventDefault(); return insRow(); }
    if (ctrl && !sh && k === '-') { e.preventDefault(); return delRow(); }
    /* (Ctrl+Z / Ctrl+Y / Ctrl+S: VS Code's own -- the undo stack and the saving are the document's) */
    /* a character: start typing over the cell (Excel) */
    if (!ctrl && !e.altKey && k.length === 1) { e.preventDefault(); return startEdit(k); }
  });
  /* Ctrl+C when VS Code does the copy (it runs the webview's copy): the selection as tab separated text */
  document.addEventListener('copy', function (e) {
    if (editing || document.activeElement === findBox || !e.clipboardData) return;
    var s = selRect(), rows = [];
    for (var r = s.r0; r <= s.r1; r++) { var line = []; for (var c = s.c0; c <= s.c1; c++) line.push(cell(r, c)); rows.push(line.join('\t')); }
    e.clipboardData.setData('text/plain', rows.join('\r\n'));
    e.preventDefault();
  });
  /* Ctrl+V: the clipboard text (from Excel: tab separated) goes in from the selected cell */
  document.addEventListener('paste', function (e) {
    if (editing || document.activeElement === findBox) return;
    var t = e.clipboardData ? e.clipboardData.getData('text/plain') : '';
    if (!t) return;
    e.preventDefault();
    pasteText(t);
  });

  function insRow() {
    if (D.readOnly) { showWarn(D.why || '這個檔案現在不能編輯。'); return; }
    var s = selRect();
    send({ type: 'insertRows', r: s.r0, count: s.r1 - s.r0 + 1 });
  }
  function delRow() {
    if (D.readOnly) { showWarn(D.why || '這個檔案現在不能編輯。'); return; }
    var s = selRect();
    send({ type: 'deleteRows', r: s.r0, count: s.r1 - s.r0 + 1 });
  }
  document.getElementById('insRow').addEventListener('click', insRow);
  document.getElementById('delRow').addEventListener('click', delRow);
  document.getElementById('asText').addEventListener('click', function () { send({ type: 'openText' }); });
  hdrBox.addEventListener('change', function () { header = hdrBox.checked; save(); draw(); });
  findBox.addEventListener('input', function () { runFind(findBox.value, false); });
  findBox.addEventListener('keydown', function (e) {
    if (e.key === 'Enter' || e.key === 'F3') { e.preventDefault(); runFind(findBox.value, true); }
    else if (e.key === 'Escape') { e.preventDefault(); findBox.value = ''; runFind('', false); wrap.focus(); }
  });

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
      if (m.first && st.cur) { cur = { r: st.cur.r, c: st.cur.c }; anchor = { r: cur.r, c: cur.c }; }
      clampCur();
      if (find.q) runFind(find.q, false);
      draw();
      if (m.first) { scrollTo(cur.r, cur.c); draw(); wrap.focus(); }
    } else if (m.type === 'select') {
      cur = { r: m.r | 0, c: m.c | 0 };
      anchor = { r: m.r1 != null ? m.r1 | 0 : cur.r, c: m.c1 != null ? m.c1 | 0 : cur.c };
      clampCur();
      scrollTo(cur.r, cur.c);
      draw();
    }
  });
  /* for the tests */
  window.__htdCsv = {
    state: function () { return { cur: cur, anchor: anchor, header: !!header, rows: nrows(), cols: ncols, editing: !!editing, drawnRows: drawn.b - drawn.a + 1 }; },
    cellText: function (r, c) { var el = sizer.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]'); return el ? el.textContent : null; }
  };
  send({ type: 'ready' });
})();
