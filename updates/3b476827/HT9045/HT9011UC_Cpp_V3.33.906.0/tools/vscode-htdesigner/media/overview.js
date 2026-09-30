/* AI(W906-HTDESIGNER) 20260929: 接線總覽 -- one page's DFM events and their state.
   textContent only; click a row = select the control in the designer, click a
   location = open it. */
(function () {
  'use strict';
  var vscode = acquireVsCodeApi();
  var root = document.getElementById('root');
  var data = null;
  var st = vscode.getState() || { filter: 'all', q: '' };

  function el(tag, cls, text) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text != null) e.textContent = String(text);
    return e;
  }
  function base(p) { return String(p || '').split(/[\\/]/).pop(); }

  var WEB = {
    yes: ['✔', '網頁有綁這個事件的監聽器', 'ok'],
    field: ['✔', '由 wire 引擎的欄位對照處理（小鍵盤／寫回）', 'ok'],
    weak: ['△', '網頁程式有提到這個元件，但沒有直接綁這個事件（多半是事件委派）', 'mid'],
    no: ['✗', '網頁沒有處理', 'bad'],
    absent: ['—', '這個元件不在網頁上', 'dim'],
    na: ['—', '表單事件', 'dim'],
  };
  var PORT = {
    live: ['✔', 'C++ 移植樹有定義', 'ok'],
    dead: ['⚠', '只有 #if 0 裡的定義（不會編譯）', 'mid'],
    mention: ['△', '只在註解裡提到（實作可能搬到別的函式）', 'mid'],
    none: ['✗', 'C++ 移植樹沒有同名函式（可能沒做，也可能網頁改走命令到別的函式）', 'bad'],
  };

  function cell(map, s, at, kind) {
    var td = el('td', 'st');
    var d = map[s] || ['?', s, 'dim'];
    var b = el('span', 'mark ' + d[2], d[0]);
    b.title = d[1];
    td.appendChild(b);
    if (at) {
      var a = el('a', 'loc', base(at.file) + ':' + at.line);
      a.href = '#';
      a.title = at.file + ':' + at.line;
      a.addEventListener('click', function (e) {
        e.preventDefault(); e.stopPropagation();
        vscode.postMessage({ type: 'open', kind: kind, file: at.file, line: at.line, col: at.col || 1 });
      });
      td.appendChild(a);
    }
    return td;
  }

  function keep(row, ev) {
    var f = st.filter;
    if (f === 'webno' && !(ev.web === 'no' || ev.web === 'weak')) return false;
    if (f === 'portno' && ev.port.state === 'live') return false;
    if (f === 'absent' && row.present) return false;
    var q = (st.q || '').trim().toLowerCase();
    if (q && (row.id + ' ' + row.cls + ' ' + ev.name + ' ' + ev.handler).toLowerCase().indexOf(q) < 0) return false;
    return true;
  }

  function render() {
    root.textContent = '';
    if (!data) { root.appendChild(el('p', 'dim', '整理中…')); return; }
    var s = data.summary;
    var h = el('div', 'head');
    h.appendChild(el('h1', null, '接線總覽：' + data.page));
    h.appendChild(el('div', 'sub', 'DFM ' + data.dfm + '　表單類別 ' + data.formClass + '　' + s.controls + ' 個有事件的元件、' + s.events + ' 個事件' +
      (s.notOnPage ? '（其中 ' + s.notOnPage + ' 個元件不在網頁上）' : '')));
    var chips = el('div', 'chips');
    function chip(label, n, cls, tip) { var c = el('span', 'chip ' + cls, label + ' ' + n); c.title = tip; chips.appendChild(c); }
    chip('網頁有接', s.web.yes + s.web.field, 'ok', '直接綁了監聽器，或由欄位對照處理');
    chip('只有提到', s.web.weak, 'mid', '網頁程式提到這個元件，但沒有直接綁這個事件');
    chip('網頁未接', s.web.no, 'bad', '網頁完全沒處理');
    chip('C++ 有同名函式', s.port.live, 'ok', 'C++ 移植樹有會編譯的 ' + data.formClass + '::函式');
    chip('只在 #if 0', s.port.dead, 'mid', '定義在 #if 0 裡，不會編譯');
    chip('只在註解', s.port.mention, 'mid', '只在註解裡提到，實作可能搬到別的函式');
    chip('C++ 沒有同名函式', s.port.none, 'bad', '可能沒做，也可能改走命令到別的 C++ 函式');
    h.appendChild(chips);
    var bar = el('div', 'bar');
    [['all', '全部'], ['webno', '網頁未接'], ['portno', 'C++ 沒有同名函式'], ['absent', '不在網頁上的元件']].forEach(function (f) {
      var b = el('button', st.filter === f[0] ? 'on' : '', f[1]);
      b.addEventListener('click', function () { st.filter = f[0]; vscode.setState(st); render(); });
      bar.appendChild(b);
    });
    var q = el('input', 'q');
    q.placeholder = '篩選元件／事件／函式…';
    q.value = st.q || '';
    q.addEventListener('input', function () { st.q = q.value; vscode.setState(st); fill(); });
    bar.appendChild(q);
    h.appendChild(bar);
    root.appendChild(h);

    var table = el('table', 'grid');
    var thead = el('thead');
    var tr = el('tr');
    [['元件', ''], ['型別', ''], ['事件', ''], ['函式', ''], ['網頁', '預覽時網頁實際綁上的監聽器／欄位對照'],
      ['C++ 移植（' + data.formClass + '::函式）', '移植樹裡有沒有同名的事件函式'], ['BCB6', '原版的事件函式']].forEach(function (t) {
      var th = el('th', null, t[0]);
      if (t[1]) th.title = t[1];
      tr.appendChild(th);
    });
    thead.appendChild(tr);
    table.appendChild(thead);
    var tbody = el('tbody');
    table.appendChild(tbody);
    root.appendChild(table);
    var count = el('div', 'dim pad');
    root.appendChild(count);

    function fill() {
      tbody.textContent = '';
      var shown = 0;
      data.rows.forEach(function (row) {
        var evs = row.events.filter(function (ev) { return keep(row, ev); });
        evs.forEach(function (ev, i) {
          var r = el('tr', row.present ? '' : 'absent');
          if (i === 0) {
            var n = el('td', 'name', row.id === '@form' ? '（表單）' : row.id);
            n.rowSpan = evs.length;
            r.appendChild(n);
            var c = el('td', 'cls', row.cls);
            c.rowSpan = evs.length;
            r.appendChild(c);
          }
          r.appendChild(el('td', 'ev', ev.name));
          r.appendChild(el('td', 'fn', ev.handler));
          r.appendChild(cell(WEB, ev.web, null, 'web'));
          r.appendChild(cell(PORT, ev.port.state, ev.port.at, 'port'));
          r.appendChild(cell({ live: ['✔', 'BCB6 有定義', 'ok'], none: ['✗', 'BCB6 找不到', 'bad'] }, ev.golden.state, ev.golden.at, 'golden'));
          r.title = row.present ? '點一下：在設計檢視選取 ' + row.id : '這個元件不在網頁上';
          r.addEventListener('click', function () { if (row.present) vscode.postMessage({ type: 'select', id: row.id }); });
          tbody.appendChild(r);
          shown++;
        });
      });
      count.textContent = '顯示 ' + shown + ' / ' + s.events + ' 個事件。' +
        '「網頁」來自預覽時實際綁上的監聽器（只在連線後才綁的看不到）。' +
        '「C++ 移植」只看有沒有同名的 ' + data.formClass + '::函式 —— 網頁常改走命令（例如 act.main.*）到別的 C++ 函式，' +
        '所以 ✗ 不一定代表功能沒做；點那個元件，看屬性面板的「送到 C++ 的命令」。';
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
