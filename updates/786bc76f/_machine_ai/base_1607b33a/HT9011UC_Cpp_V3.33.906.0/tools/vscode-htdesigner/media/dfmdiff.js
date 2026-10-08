/* AI(W906-HTDESIGNER) 20260929: 與 DFM 的差異 -- every property of the page that is not
   what the BCB6 .dfm says. textContent only; click a row = select the control in the
   designer, ↺ = write the DFM value back (one undo step). The same difference on many
   controls of one class (the page's common style) is folded into one pattern line. */
(function () {
  'use strict';
  var vscode = acquireVsCodeApi();
  var root = document.getElementById('root');
  var data = null;
  var st = vscode.getState() || { group: 'all', q: '', pattern: null, showPat: false };
  var GROUP = { layout: '位置／大小', text: '文字', look: '外觀' };

  function el(tag, cls, text) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text != null) e.textContent = String(text);
    return e;
  }
  function save() { vscode.setState(st); }
  function val(prop, v) {
    var td = el('td', 'fn');
    if (/Color$/.test(prop) && /^#[0-9a-f]{6}$/i.test(v)) {
      var sw = el('span', 'sw');
      sw.style.background = v;
      td.appendChild(sw);
    }
    td.appendChild(document.createTextNode(v === '' ? '（空）' : v));
    return td;
  }
  function keep(r) {
    if (st.group !== 'all' && r.group !== st.group) return false;
    if (st.pattern) { if (r.pattern !== st.pattern) return false; }
    else if (!st.showPat && r.pattern) return false;
    var q = (st.q || '').trim().toLowerCase();
    if (q && (r.id + ' ' + r.cls + ' ' + r.prop + ' ' + r.page + ' ' + r.dfm).toLowerCase().indexOf(q) < 0) return false;
    return true;
  }

  function render() {
    root.textContent = '';
    if (!data) { root.appendChild(el('p', 'dim', '比對中…')); return; }
    var pats = data.patterns || [];
    if (st.pattern && !pats.some(function (p) { return p.key === st.pattern; })) st.pattern = null;
    var h = el('div', 'head');
    h.appendChild(el('h1', null, '與 DFM 的差異：' + data.page));
    var inPat = data.rows.length - (data.single == null ? data.rows.length : data.single);
    h.appendChild(el('div', 'sub', 'DFM ' + data.dfm + '　表單類別 ' + data.formClass + '　比了 ' + data.controls + ' 個元件、' +
      data.compared + ' 項，不一樣的 ' + data.rows.length + ' 項：個別 ' + (data.rows.length - inPat) + ' 項' +
      (pats.length ? '、同一種差異 ' + pats.length + ' 種（' + inPat + ' 項）' : '') + (data.dirty ? '　（頁面有未存檔的修改）' : '')));
    var chips = el('div', 'chips');
    ['layout', 'text', 'look'].forEach(function (g) {
      chips.appendChild(el('span', 'chip ' + (data.byGroup[g] ? 'mid' : 'ok'), GROUP[g] + ' ' + data.byGroup[g]));
    });
    h.appendChild(chips);
    var bar = el('div', 'bar');
    [['all', '全部'], ['layout', GROUP.layout], ['text', GROUP.text], ['look', GROUP.look]].forEach(function (f) {
      var b = el('button', st.group === f[0] ? 'on' : '', f[1]);
      b.setAttribute('data-group', f[0]);
      b.addEventListener('click', function () { st.group = f[0]; save(); render(); });
      bar.appendChild(b);
    });
    if (pats.length && !st.pattern) {
      var sp = el('button', st.showPat ? 'on' : '', st.showPat ? '也列出同一種差異' : '只列個別差異');
      sp.setAttribute('data-act', 'showPat');
      sp.title = '同一種差異＝同一型別的很多元件都一樣（多半是網頁的統一樣式）';
      sp.addEventListener('click', function () { st.showPat = !st.showPat; save(); render(); });
      bar.appendChild(sp);
    }
    var q = el('input', 'q');
    q.placeholder = '篩選元件／屬性／值…';
    q.value = st.q || '';
    q.addEventListener('input', function () { st.q = q.value; save(); fill(); });
    bar.appendChild(q);
    var rf = el('button', null, '重新比對');
    rf.title = '頁面在別處被改過的話，按這個重新比對';
    rf.addEventListener('click', function () { vscode.postMessage({ type: 'refresh' }); });
    bar.appendChild(rf);
    var gb = el('button', data.ghostsOn ? 'on' : '', data.ghostsOn ? '畫面上的 DFM 位置：開' : '在畫面上畫出 DFM 位置');
    gb.setAttribute('data-act', 'ghosts');
    gb.title = '設計畫面上用紫色虛線框畫出 BCB6 .dfm 的位置（只畫位置／大小不同的），拖過去會吸附；再按一次關掉';
    gb.addEventListener('click', function () { vscode.postMessage({ type: 'ghosts' }); });
    bar.appendChild(gb);
    h.appendChild(bar);
    root.appendChild(h);

    if (!data.rows.length) {
      root.appendChild(el('p', 'pad', '全部和 DFM 一樣（DFM 有寫、網頁也有給的屬性）。'));
      return;
    }

    /* the page's common style: one line per pattern */
    if (pats.length) {
      var det = el('details', 'pats');
      det.open = !!st.patOpen || !!st.pattern;
      det.addEventListener('toggle', function () { st.patOpen = det.open; save(); });
      det.appendChild(el('summary', null, '同一種差異 ' + pats.length + ' 種（同一型別至少 5 個元件都一樣，多半是網頁的統一樣式；點一列看是哪些元件）'));
      var pt = el('table', 'grid');
      var ph = el('tr');
      ['型別', '屬性', '網頁', 'DFM', '個數', ''].forEach(function (t) { ph.appendChild(el('th', null, t)); });
      var pthead = el('thead');
      pthead.appendChild(ph);
      pt.appendChild(pthead);
      var ptb = el('tbody');
      pats.forEach(function (p) {
        var r = el('tr', 'patrow' + (st.pattern === p.key ? ' on' : ''));
        r.setAttribute('data-key', p.key);
        r.appendChild(el('td', 'cls', p.cls));
        r.appendChild(el('td', 'ev', p.prop));
        r.appendChild(val(p.prop, p.page));
        r.appendChild(val(p.prop, p.dfm));
        r.appendChild(el('td', 'fn', p.count));
        /* a look / text pattern: every one back to the DFM value at once (asks first; one Ctrl+Z) */
        var tdb = el('td');
        if (p.group !== 'layout') {
          var ab = el('button', 'reset', '全部改回');
          ab.setAttribute('data-act', 'resetPattern');
          ab.title = '把這 ' + p.count + ' 個都改回 DFM 的值（會先問；一個 Ctrl+Z 全部復原）';
          ab.addEventListener('click', function (e) { e.stopPropagation(); vscode.postMessage({ type: 'resetPattern', key: p.key }); });
          tdb.appendChild(ab);
        }
        r.appendChild(tdb);
        r.title = '點一下：只看這一種差異的元件';
        r.addEventListener('click', function () { st.pattern = st.pattern === p.key ? null : p.key; save(); render(); });
        ptb.appendChild(r);
      });
      pt.appendChild(ptb);
      det.appendChild(pt);
      root.appendChild(det);
    }
    if (st.pattern) {
      var pp = pats.filter(function (p) { return p.key === st.pattern; })[0];
      var line = el('div', 'bar');
      line.appendChild(el('span', 'dim', '只看：' + pp.cls + ' 的 ' + pp.prop + '（網頁 ' + (pp.page || '（空）') + '、DFM ' + (pp.dfm || '（空）') + '）共 ' + pp.count + ' 個'));
      var back = el('button', null, '回到個別差異');
      back.setAttribute('data-act', 'back');
      back.addEventListener('click', function () { st.pattern = null; save(); render(); });
      line.appendChild(back);
      root.appendChild(line);
    }

    var table = el('table', 'grid');
    var thead = el('thead');
    var tr = el('tr');
    ['元件', '型別', '屬性', '網頁', 'DFM', ''].forEach(function (t) { tr.appendChild(el('th', null, t)); });
    thead.appendChild(tr);
    table.appendChild(thead);
    var tbody = el('tbody');
    table.appendChild(tbody);
    root.appendChild(table);
    var count = el('div', 'dim pad');
    root.appendChild(count);

    function fill() {
      tbody.textContent = '';
      var list = data.rows.filter(keep);
      var i = 0;
      while (i < list.length) {
        var j = i;
        while (j < list.length && list[j].id === list[i].id) j++;
        for (var k = i; k < j; k++) {
          (function (r, first, span) {
            var row = el('tr', 'diffrow');
            row.setAttribute('data-id', r.id);
            row.setAttribute('data-prop', r.prop);
            if (first) {
              var n = el('td', 'name', r.id);
              n.rowSpan = span;
              row.appendChild(n);
              var c = el('td', 'cls', r.cls);
              c.rowSpan = span;
              row.appendChild(c);
            }
            var p = el('td', 'ev', r.prop);
            if (r.note) { p.title = r.note; p.appendChild(el('span', 'mark dim', ' *')); }
            row.appendChild(p);
            row.appendChild(val(r.prop, r.page));
            row.appendChild(val(r.prop, r.dfm));
            var td = el('td');
            var b = el('button', 'reset', '↺');
            b.disabled = !r.inSource;
            b.title = r.inSource ? '重設為 DFM 值（' + r.dfm + '）' : '這個元件是頁面 JS 產生的，HTML 原始碼裡沒有它，不能改';
            b.addEventListener('click', function (e) {
              e.stopPropagation();
              vscode.postMessage({ type: 'reset', id: r.id, reset: r.reset });
            });
            td.appendChild(b);
            row.appendChild(td);
            row.title = '點一下：在設計檢視選取 ' + r.id;
            row.addEventListener('click', function () { vscode.postMessage({ type: 'select', id: r.id }); });
            tbody.appendChild(row);
          })(list[k], k === i, j - i);
        }
        i = j;
      }
      if (!list.length && !st.pattern && pats.length && !st.showPat) {
        var tr0 = el('tr');
        var td0 = el('td', 'dim', '沒有個別差異 —— 全部都是上面的「同一種差異」。');
        td0.colSpan = 6;
        tr0.appendChild(td0);
        tbody.appendChild(tr0);
      }
      count.textContent = '顯示 ' + list.length + ' / ' + data.rows.length + ' 項。' +
        '不一樣不代表錯 —— 網頁作者常常是刻意調的；要不要改回去由你決定，↺ 一次改一項（Ctrl+Z 可以復原）。' +
        '只比 DFM 有寫、而且網頁原始碼也有給的屬性：系統色（clBtnFace…）、沿用表單字型的字型名稱、由文字決定的大小都不比。' +
        '字型大小照網頁產生器的換算（Font.Height −13 → 11px）。標 * 的滑鼠移上去看說明（沒有顯示的元件、座標原點換算）。';
    }
    fill();
  }

  window.addEventListener('message', function (e) {
    var m = e.data;
    if (m && m.type === 'data') { data = m.data; render(); }
  });
  render();
  vscode.postMessage({ type: 'ready' });
})();
