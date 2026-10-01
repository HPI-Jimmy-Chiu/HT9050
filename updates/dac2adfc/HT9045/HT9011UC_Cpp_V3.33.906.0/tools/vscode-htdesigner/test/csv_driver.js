// AI(W906-HTDESIGNER) 20260930: CSV 表格 in a headless Edge (test\probe_test.ps1): media/csv.js with a
// stubbed acquireVsCodeApi; what it sends to the extension is checked. Writes <pre id="HTDTEST">.
(function () {
  var R = {};
  function out(t) { return (window.__out || []).filter(function (m) { return m.type === t; }); }
  function send(m) { window.postMessage(m, '*'); }
  /* a key into the focused input (the table's one: idle or editing), as a browser does it: a
   * character not taken by the keydown lands in the input and fires 'input' */
  function key(k, o) {
    var w = document.getElementById('kb');
    var ev = new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true, ctrlKey: !!(o && o.ctrlKey), shiftKey: !!(o && o.shiftKey) });
    var go = w.dispatchEvent(ev);
    if (go && k.length === 1 && !(o && o.ctrlKey)) { w.value += k; w.dispatchEvent(new Event('input', { bubbles: true })); }
  }
  function edKey(k) { key(k); }
  function press(r, c, n, shift) {
    var el = document.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]');
    el.dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, detail: n || 1, shiftKey: !!shift }));
    window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true }));
  }
  function editor() { return document.querySelector('.ed:not(.idle)'); }
  function finish() {
    var pre = document.createElement('pre');
    pre.id = 'HTDTEST';
    pre.textContent = 'HTDTEST' + JSON.stringify(R) + 'HTDEND';
    document.body.appendChild(pre);
  }
  setTimeout(function () {
    try {
      R.ready = out('ready').length === 1;
      var rows = [['Motorname', 'Alias', 'Acc'], ['M00', 'MInArmX', '100000'], ['M01', 'MInArmY', '200000']];
      for (var i = 0; i < 5000; i++) rows.push(['M' + (i + 2), 'X' + i, String(i)]);
      send({ type: 'data', rows: rows, delim: ',', readOnly: false, file: 'Mot_Table.csv', first: true });
    } catch (e) { R.driverError = String(e); finish(); return; }
    setTimeout(function () {
      try {
        var st = window.__htdCsv.state();
        R.header = st.header;
        /* laid out for real (under the page's policy): columns side by side, rows one under another, the header on top */
        var box = function (r, c) { var e = document.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]'); return e ? e.getBoundingClientRect() : null; };
        var h0 = box(0, 0), a0 = box(1, 0), a1 = box(1, 1), b0 = box(2, 0);
        R.layout = [h0, a0, a1, b0].map(function (q) { return q ? Math.round(q.left) + ',' + Math.round(q.top) + ' ' + Math.round(q.width) : '-'; }).join(' | ');
        R.layoutOk = !!(h0 && a0 && a1 && b0 && a1.left >= a0.right - 1 && b0.top >= a0.bottom - 1 && a0.top >= h0.bottom - 1 && a0.width > 20 &&
          document.querySelector('.hc[data-col="1"]').getBoundingClientRect().left > document.querySelector('.hc[data-col="0"]').getBoundingClientRect().left + 20);
        R.virtual = st.rows + ' rows, ' + st.drawnRows + ' drawn';
        R.virtualOk = st.rows === 5003 && st.drawnRows > 5 && st.drawnRows < 120 && document.querySelectorAll('.cell').length < 120 * 3 + 3;
        /* click a cell, type over it, Enter */
        var c = document.querySelector('.cell[data-r="1"][data-c="2"]');
        c.dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true }));
        window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true }));
        key('5');
        var inp = editor();
        R.editorOpen = !!inp && inp.value === '5';
        if (inp) inp.value = '555';
        edKey('Enter');
        var e1 = out('edit').pop();
        R.edit = e1 ? JSON.stringify(e1.changes) : null;
        R.editOk = !!e1 && e1.changes.length === 1 && e1.changes[0].r === 1 && e1.changes[0].c === 2 && e1.changes[0].v === '555';
        st = window.__htdCsv.state();
        R.movedDown = st.cur.r === 2 && st.cur.c === 2 && !st.editing;
        R.shownNow = window.__htdCsv.cellText(1, 2) === '555';
        /* Shift+arrows: a block; Ctrl+C: tab separated rows */
        key('ArrowLeft', { shiftKey: true });
        key('ArrowUp', { shiftKey: true });
        key('c', { ctrlKey: true });
        var cp = out('copy').pop();
        R.copy = cp ? JSON.stringify(cp.grid) : null;
        R.copyOk = !!cp && R.copy === '[["MInArmX","555"],["MInArmY","200000"]]';
        /* Delete: the block cleared, one edit */
        var nE = out('edit').length;
        key('Delete');
        var e2 = out('edit').pop();
        R.clearOk = out('edit').length === nE + 1 && e2.changes.length === 4 && e2.changes.every(function (x) { return x.v === ''; });
        /* paste (from Excel): the text and the top-left cell go to the extension */
        var pe = new Event('paste', { bubbles: true, cancelable: true });
        Object.defineProperty(pe, 'clipboardData', { value: { getData: function () { return 'a\tb\r\nc\td'; } } });
        document.getElementById('wrap').dispatchEvent(pe);
        var ps = out('paste').pop();
        R.pasteOk = !!ps && ps.text === 'a\tb\r\nc\td' && ps.r === 1 && ps.c === 1 && ps.r1 === 2 && ps.c1 === 2 && !ps.cut;
        /* Ctrl+X: copied now, moved when pasted -- the paste of that same text carries where it came from (20261001, Excel) */
        key('x', { ctrlKey: true });
        var cutT = window.__htdCsv.cutText();
        var nE5 = out('edit').length;
        var pe2 = new Event('paste', { bubbles: true, cancelable: true });
        Object.defineProperty(pe2, 'clipboardData', { value: { getData: function () { return cutT; } } });
        document.getElementById('wrap').dispatchEvent(pe2);
        var ps2 = out('paste').pop();
        R.cutOk = !!cutT && out('edit').length === nE5 && !!ps2 && !!ps2.cut && ps2.cut.r0 === 1 && ps2.cut.c1 === 2 && !window.__htdCsv.state().cut;
        /* F2 then Esc: nothing written */
        var nE2 = out('edit').length;
        key('F2');
        var inp2 = editor();
        if (inp2) inp2.value = 'zzz';
        edKey('Escape');
        R.escOk = !!inp2 && !editor() && out('edit').length === nE2;
        /* one cell: Tab moves right, Home goes to the first column */
        press(1, 1, 1);
        key('Tab');
        var s1 = window.__htdCsv.state().act;
        key('Home');
        var s2 = window.__htdCsv.state().act;
        R.moveOk = s1.c === 2 && s2.c === 0;
        /* find ("X10" is in row 13 -- X10, X100, X1000... -- the first one after the current cell) */
        var fb = document.getElementById('find');
        fb.value = 'X10';
        fb.dispatchEvent(new Event('input'));
        fb.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
        var s3 = window.__htdCsv.state().cur;
        R.find = s3.r + ',' + s3.c + ' ' + document.getElementById('findN').textContent;
        R.findOk = s3.r === 13 && s3.c === 1 && /第 1 個/.test(document.getElementById('findN').textContent);
        /* a double click = the editor, its text selected: told by the second press's click count (20261001:
         * the browser's own dblclick never reached the cell -- the first press redraws it) */
        press(5, 1, 1); press(5, 1, 2);
        var de = editor();
        R.dbl = de ? de.value + ' ' + de.selectionStart + '-' + de.selectionEnd + ' ' + (document.activeElement === de) : null;
        R.dblOk = !!de && de.value === 'X2' && de.selectionStart === de.selectionEnd && document.activeElement === de;
        edKey('Escape');
        /* the IME (注音): a word started = that cell's editor; the IME's own Enter (isComposing) picks the
         * word, it does not write the cell; then Enter writes it */
        press(6, 1, 1);
        var kbi = document.getElementById('kb'), nE4 = out('edit').length;
        R.kbFocused = document.activeElement === kbi;
        kbi.dispatchEvent(new KeyboardEvent('keydown', { key: 'Process', keyCode: 229, bubbles: true, cancelable: true }));
        kbi.dispatchEvent(new CompositionEvent('compositionstart', { bubbles: true, data: '' }));
        var imeOpen = !!editor();
        kbi.value = '中';
        kbi.dispatchEvent(new InputEvent('input', { bubbles: true, isComposing: true, data: '中' }));
        kbi.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', keyCode: 229, isComposing: true, bubbles: true, cancelable: true }));
        var stillEd = !!editor() && out('edit').length === nE4;
        kbi.dispatchEvent(new CompositionEvent('compositionend', { bubbles: true, data: '中' }));
        edKey('Enter');
        var e4 = out('edit').pop();
        R.ime = imeOpen + ' ' + stillEd + ' ' + (e4 ? JSON.stringify(e4.changes) : null);
        R.imeOk = R.kbFocused && imeOpen && stillEd && out('edit').length === nE4 + 1 && e4.changes[0].r === 6 && e4.changes[0].c === 1 && e4.changes[0].v === '中';
        /* AI(W906-HTDESIGNER) 20261001 -- the right-click menu (Excel's): inside the block it keeps the block, each item
         * with its key; 清除內容 = the block emptied (one edit); outside = that cell selected; Esc closes it; Shift+F10 opens
         * it at the active cell, Down / Enter runs an item; 貼上 asks the extension for the clipboard */
        /* (the press redraws the grid: the contextmenu goes to the cell drawn now, as a real one does) */
        var cellAt = function (r, c) { return document.querySelector('.cell[data-r="' + r + '"][data-c="' + c + '"]'); };
        var rclick = function (r, c) {
          cellAt(r, c).dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, button: 2 }));
          window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true, button: 2 }));
          cellAt(r, c).dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 50, clientY: 60 }));
          return document.querySelector('.ctxmenu');
        };
        var items = function (m) { return m ? Array.prototype.map.call(m.querySelectorAll('.ctxitem'), function (x) { return x.getAttribute('data-item') + (x.classList.contains('off') ? '(off)' : ''); }) : []; };
        press(8, 0, 1); press(9, 1, 1, true);
        var mk = rclick(9, 0);
        var stK = window.__htdCsv.state();
        R.menu = items(mk).join('|');
        R.menuOk = !!mk && R.menu === '剪下|複製|貼上|插入 2 列（在上面）|刪除 2 列|清除內容|選取整列|選取整欄|尋找…|取代…' &&
          mk.querySelector('.ctxkey').textContent === 'Ctrl+X' && stK.anchor.r === 8 && stK.anchor.c === 0 && stK.cur.r === 9 && stK.cur.c === 1 && !stK.editing;
        var nE5 = out('edit').length;
        var clr = mk && mk.querySelector('.ctxitem[data-item="清除內容"]');
        if (clr) clr.click();
        var e5 = out('edit').pop();
        R.menuClear = out('edit').length === nE5 + 1 && e5.changes.length === 4 && e5.changes.every(function (x) { return x.v === ''; }) && !document.querySelector('.ctxmenu');
        var mo = rclick(12, 2);
        var stO = window.__htdCsv.state();
        R.menuMove = !!mo && stO.anchor.r === 12 && stO.anchor.c === 2 && stO.cur.r === 12 && stO.cur.c === 2 && !!mo.querySelector('.ctxitem[data-item="插入 1 列（在上面）"]');
        key('Escape');
        R.menuEsc = !document.querySelector('.ctxmenu') && window.__htdCsv.state().cur.r === 12;
        key('F10', { shiftKey: true });
        var mf = document.querySelector('.ctxmenu');
        var nC = out('copy').length;
        key('ArrowDown'); key('ArrowDown');
        var hotNow = mf && mf.querySelector('.ctxitem.hot');
        key('Enter');
        var c6 = out('copy').pop();
        R.menuKeys = !!mf && !!hotNow && hotNow.getAttribute('data-item') === '複製' && out('copy').length === nC + 1 && JSON.stringify(c6.grid) === '[["9"]]' &&
          !document.querySelector('.ctxmenu') && document.activeElement === document.getElementById('kb');
        /* 20261001 -- the Name Box (Excel): the active cell's address; Ctrl+G = into it; B5:C7 + Enter = that block (B5
         * active), the keys back with the table; a place not in the table = said, nothing moved; Esc = back as it was */
        var nb = document.getElementById('nameBox');
        R.nbShows = nb.value;
        key('g', { ctrlKey: true });
        var nbFocus = document.activeElement === nb;
        var nbType = function (v) { nb.value = v; nb.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true })); };
        nbType('b5:c7');
        var sG = window.__htdCsv.state();
        var nbBack = document.activeElement === document.getElementById('kb') && nb.value === 'B5';
        nb.focus(); nbType('ZZ9');
        var sBad = window.__htdCsv.state(), badSaid = !document.getElementById('warn').hidden && /ZZ9/.test(document.getElementById('warn').textContent);
        nb.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true, cancelable: true }));
        var nbEsc = document.activeElement === document.getElementById('kb') && nb.value === 'B5';
        nb.focus(); nbType('12');
        var sRow = window.__htdCsv.state();
        R.nameBox = { shows: R.nbShows, g: sG.anchor.r + ',' + sG.anchor.c + '-' + sG.cur.r + ',' + sG.cur.c + ' act ' + sG.act.r + ',' + sG.act.c, row: sRow.act.r + ',' + sRow.act.c };
        R.nameBoxOk = R.nbShows === 'C13' && nbFocus && sG.anchor.r === 4 && sG.anchor.c === 1 && sG.cur.r === 6 && sG.cur.c === 2 && sG.act.r === 4 && sG.act.c === 1 && nbBack &&
          badSaid && sBad.act.r === 4 && sBad.cur.r === 6 && nbEsc && sRow.act.r === 11 && sRow.act.c === 1 && sRow.cur.r === 11;
        /* AutoFit (Excel): a column dragged narrow, then a double click on its boundary = as wide as its longest value */
        var rsOf = function (c) { return document.querySelector('.hc[data-col="' + c + '"] .rs'); };
        var hw = function (c) { return Math.round(document.querySelector('.hc[data-col="' + c + '"]').getBoundingClientRect().width); };
        var rs1 = rsOf(1), b1 = rs1.getBoundingClientRect();
        rs1.dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, detail: 1, clientX: b1.left, clientY: b1.top + 2 }));
        window.dispatchEvent(new MouseEvent('mousemove', { bubbles: true, clientX: b1.left - 40, clientY: b1.top + 2 }));
        window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true }));
        var narrow = hw(1);
        var over = function (c) { return Array.prototype.filter.call(document.querySelectorAll('.cell[data-c="' + c + '"]'), function (x) { return x.scrollWidth > x.clientWidth + 1; }).length; };
        var overNarrow = over(1);
        rsOf(1).dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, detail: 1 }));
        window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true }));
        rsOf(1).dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, detail: 2 }));
        window.dispatchEvent(new MouseEvent('mouseup', { bubbles: true }));
        var fitted = hw(1);
        R.autoFit = narrow + ' -> ' + fitted + ' (over ' + overNarrow + ' -> ' + over(1) + ')';
        R.autoFitOk = overNarrow > 0 && over(1) === 0 && fitted > narrow && fitted < 200;
        var mp = rclick(12, 2), nR = out('readClip').length;
        var pi = mp && mp.querySelector('.ctxitem[data-item="貼上"]');
        if (pi) pi.click();
        R.menuReadClip = out('readClip').length === nR + 1;
        send({ type: 'clip', text: 'a\tb' });
        /* read only (a Big5 file read as UTF-8): typing does nothing but say why */
        send({ type: 'data', rows: rows.slice(0, 3), delim: ',', readOnly: true, why: 'not UTF-8', file: 'x.csv', first: false });
      } catch (e) { R.driverError = String(e && e.stack || e); finish(); return; }
      setTimeout(function () {
        try {
          var nE3 = out('edit').length;
          document.getElementById('wrap').focus();
          key('7');
          R.roOk = out('edit').length === nE3 && !editor() && document.getElementById('kb').value === '' && !document.getElementById('warn').hidden &&
            /UTF-8/.test(document.getElementById('warn').textContent);
          /* the menu's 貼上: the clipboard the extension sent back went in at the selected cell, like Ctrl+V */
          R.menuPaste = out('paste').some(function (m) { return m.text === 'a\tb' && m.r === 12 && m.c === 2; });
          /* read only: what would change the file is off in the menu */
          document.querySelector('.cell[data-r="1"][data-c="0"]').dispatchEvent(new MouseEvent('mousedown', { bubbles: true, cancelable: true, button: 2 }));
          document.querySelector('.cell[data-r="1"][data-c="0"]').dispatchEvent(new MouseEvent('contextmenu', { bubbles: true, cancelable: true, clientX: 30, clientY: 30 }));
          var mro = document.querySelector('.ctxmenu');
          R.menuRo = mro ? Array.prototype.filter.call(mro.querySelectorAll('.ctxitem.off'), function () { return true; }).map(function (x) { return x.getAttribute('data-item').charAt(0); }).join('') : null;
          R.menuRoOk = R.menuRo === '剪貼插刪清取';
        } catch (e) { R.driverError = String(e); }
        finish();
      }, 150);
    }, 250);
  }, 300);
})();
