// AI(W906-HTDESIGNER) 20260930: CSV 表格 in a headless Edge (test\probe_test.ps1): media/csv.js with a
// stubbed acquireVsCodeApi; what it sends to the extension is checked. Writes <pre id="HTDTEST">.
(function () {
  var R = {};
  function out(t) { return (window.__out || []).filter(function (m) { return m.type === t; }); }
  function send(m) { window.postMessage(m, '*'); }
  function key(k, o) {
    var w = document.getElementById('wrap');
    var ev = new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true, ctrlKey: !!(o && o.ctrlKey), shiftKey: !!(o && o.shiftKey) });
    w.dispatchEvent(ev);
  }
  function edKey(k) {
    var inp = document.querySelector('.ed');
    if (inp) inp.dispatchEvent(new KeyboardEvent('keydown', { key: k, bubbles: true, cancelable: true }));
  }
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
        var inp = document.querySelector('.ed');
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
        R.pasteOk = !!ps && ps.text === 'a\tb\r\nc\td' && ps.r === 1 && ps.c === 1;
        /* F2 then Esc: nothing written */
        var nE2 = out('edit').length;
        key('F2');
        var inp2 = document.querySelector('.ed');
        if (inp2) inp2.value = 'zzz';
        edKey('Escape');
        R.escOk = !!inp2 && !document.querySelector('.ed') && out('edit').length === nE2;
        /* Tab moves right, Home goes to the first column */
        key('Tab');
        var s1 = window.__htdCsv.state().cur;
        key('Home');
        var s2 = window.__htdCsv.state().cur;
        R.moveOk = s1.c === 2 && s2.c === 0;
        /* find ("X10" is in row 13 -- X10, X100, X1000... -- the first one after the current cell) */
        var fb = document.getElementById('find');
        fb.value = 'X10';
        fb.dispatchEvent(new Event('input'));
        fb.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
        var s3 = window.__htdCsv.state().cur;
        R.find = s3.r + ',' + s3.c + ' ' + document.getElementById('findN').textContent;
        R.findOk = s3.r === 13 && s3.c === 1 && /第 1 個/.test(document.getElementById('findN').textContent);
        /* read only (a Big5 file read as UTF-8): typing does nothing but say why */
        send({ type: 'data', rows: rows.slice(0, 3), delim: ',', readOnly: true, why: 'not UTF-8', file: 'x.csv', first: false });
      } catch (e) { R.driverError = String(e && e.stack || e); finish(); return; }
      setTimeout(function () {
        try {
          var nE3 = out('edit').length;
          document.getElementById('wrap').focus();
          key('7');
          R.roOk = out('edit').length === nE3 && !document.querySelector('.ed') && !document.getElementById('warn').hidden &&
            /UTF-8/.test(document.getElementById('warn').textContent);
        } catch (e) { R.driverError = String(e); }
        finish();
      }, 150);
    }, 250);
  }, 300);
})();
