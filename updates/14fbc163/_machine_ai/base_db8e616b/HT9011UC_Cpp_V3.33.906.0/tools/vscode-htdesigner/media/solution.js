/* AI(W906-HTDESIGNER) 20261001 (0.148): 方案總管 as one piece -- the search box right under the title, the tree under it
 * (EastSun: "方案總管整個重做成一格"; Visual Studio's Solution Explorer). The extension (SolutionPanel) builds the rows --
 * what is open, the filter, the icons -- this page only draws them and says what was clicked / typed.
 * Keys as in VS Code's tree: ↑ ↓ Home End, → open / into, ← close / up, Enter / Space = open the file (or open / close
 * the folder); Ctrl+F = the box. In the box: typing filters (200 ms), Esc clears, Enter / ↓ = into the tree. */
(function () {
  'use strict';
  var api = acquireVsCodeApi();
  var st = api.getState() || {};
  var CH = JSON.parse(document.getElementById('root').getAttribute('data-ch') || '{}');
  var root = document.getElementById('root');
  // 0.151 (EastSun: "在圖片上面 做我剛剛說的 啟動軟體開關 暫停 關掉 ... 風格可以仿造圖片上的風格 顏色偏黑色"): Visual Studio's
  // tool window -- a frame, the toolbar row (▶ 啟動 ⏸ ⏹ ↻ | ☑ 模擬 ☑ Debug | 同步 重新整理 全部摺疊), the search box, the tree
  root.innerHTML =
    '<div class="win"><div id="tb" class="tb" role="toolbar" aria-label="執行與方案總管"></div>' +
    '<div class="top"><div class="box"><span class="ci" aria-hidden="true"></span>' +
    '<input id="q" type="text" spellcheck="false" autocomplete="off" placeholder="搜尋方案總管（檔名、路徑、頁面、元件）" aria-label="搜尋方案總管">' +
    '<button id="x" type="button" class="ci" title="清除（Esc）" aria-label="清除"></button></div></div>' +
    '<div id="msg" class="msg"></div>' +
    '<div id="list" class="list" tabindex="0" role="tree" aria-label="方案總管"></div></div>';
  var tb = document.getElementById('tb');
  var run = null;
  function drawTb() {
    tb.textContent = '';
    var s = run || { st: 'idle', sim: true, dbg: true, b: {} };
    var B = s.b || {};
    var btn = function (key, icon, text, cls, cmd, tip) {
      var b = document.createElement('button');
      b.type = 'button';
      b.className = 'tbb' + (cls ? ' ' + cls : '');
      b.dataset.key = key;
      var i = document.createElement('span'); i.className = 'ci'; i.textContent = String.fromCharCode(CH[icon] || 0x3F); b.appendChild(i);
      if (text) { var t = document.createElement('span'); t.className = 'tx'; t.textContent = text; b.appendChild(t); }
      b.title = tip || '';
      if (cmd) b.dataset.cmd = cmd; else b.disabled = true;
      tb.appendChild(b);
      return b;
    };
    var sep = function () { var d = document.createElement('span'); d.className = 'sep'; tb.appendChild(d); };
    var r = function (k) { return B[k] || {}; };
    var startText = s.st === 'building' ? '建置中 ' + (s.pct || 0) + '%' : s.st === 'paused' ? '繼續' : s.st === 'running' ? '執行中' : '啟動';
    // (1003, EastSun: "請用不同顏色 不要反灰": building / running in their own colour, not greyed out)
    btn('start', s.st === 'paused' ? 'debug-continue' : 'debug-start', startText, 'go' + (s.st === 'building' ? ' busy state' : s.st === 'running' ? ' live state' : ''), r('start').cmd, r('start').tip);
    btn('pause', 'debug-pause', '', '', r('pause').cmd, r('pause').tip);
    btn('stop', 'debug-stop', '', 'red', r('stop').cmd, r('stop').tip);
    btn('restart', 'debug-restart', '', '', r('restart').cmd, r('restart').tip);
    sep();
    btn('sim', s.sim ? 'pass-filled' : 'circle-large-outline', '模擬', 'chk' + (s.sim ? ' on' : ''), r('sim').cmd, r('sim').tip);
    btn('dbg', s.dbg ? 'pass-filled' : 'circle-large-outline', 'Debug', 'chk' + (s.dbg ? ' on' : ''), r('dbg').cmd, r('dbg').tip);
    sep();
    btn('sync', 'target', '', '', 'ht9045Designer.solutionReveal', '與作用中文件同步（在方案總管選取目前的檔案）');
    btn('refresh', 'refresh', '', '', 'ht9045Designer.solutionRefresh', '重新整理');
    btn('collapse', 'collapse-all', '', '', 'ht9045Designer.solutionCollapseAll', '全部摺疊');
  }
  tb.addEventListener('click', function (e) {
    var b = e.target.closest && e.target.closest('.tbb');
    if (b && !b.disabled && b.dataset.cmd) api.postMessage({ type: 'cmd', id: b.dataset.cmd });
  });
  drawTb();
  var q = document.getElementById('q');
  var x = document.getElementById('x');
  var msg = document.getElementById('msg');
  var list = document.getElementById('list');
  root.querySelector('.box .ci').textContent = String.fromCharCode(CH.search || 0xEA6D);
  x.textContent = String.fromCharCode(CH.close || 0xEA76);
  q.title = '打字就篩選下面的方案總管：檔名、路徑，網頁的頁面和元件也找。空白隔開＝每個字都要有；含 / 的字比對路徑（例如 forms .h、web/page main）。Enter 或 ↓ 到清單，Esc 清除。';
  var rows = [];
  var sel = null;
  var timer = null;
  var sent = null;

  function idx(id) { for (var i = 0; i < rows.length; i++) if (rows[i].id === id) return i; return -1; }

  function label(el, r) {
    var t = r.l || '';
    var hs = (r.h || []).slice().sort(function (a, b) { return a[0] - b[0]; });
    var at = 0;
    hs.forEach(function (h) {
      if (h[0] < at || h[1] <= h[0]) return;
      if (h[0] > at) el.appendChild(document.createTextNode(t.slice(at, h[0])));
      var m = document.createElement('mark');
      m.textContent = t.slice(h[0], h[1]);
      el.appendChild(m);
      at = h[1];
    });
    if (at < t.length) el.appendChild(document.createTextNode(t.slice(at)));
  }

  function draw() {
    var top = list.scrollTop;
    list.textContent = '';
    var frag = document.createDocumentFragment();
    rows.forEach(function (r) {
      var row = document.createElement('div');
      row.className = 'row' + (r.id === sel ? ' sel' : '');
      row.setAttribute('role', 'treeitem');
      row.dataset.id = r.id;
      row.style.paddingLeft = (4 + r.d * 8) + 'px';
      if (r.tip) row.title = r.tip;
      if (r.tw) row.setAttribute('aria-expanded', r.tw === 2 ? 'true' : 'false');
      // (right-click: VS Code's menu for this row -- webview/context, keyed by webviewSection)
      row.setAttribute('data-vscode-context', JSON.stringify({ webviewSection: r.ctx || 'htdSlnNone', htdId: r.id, preventDefaultContextMenuItems: true }));
      var tw = document.createElement('span');
      tw.className = 'tw ci';
      tw.textContent = r.tw ? String.fromCharCode(r.tw === 2 ? (CH['chevron-down'] || 0xEAB4) : (CH['chevron-right'] || 0xEAB6)) : '';
      row.appendChild(tw);
      var ic = document.createElement('span');
      ic.className = 'ic ' + (r.ic && r.ic.t === 's' ? 'si' : 'ci');
      if (r.ic && r.ic.ch) ic.textContent = String.fromCharCode(r.ic.ch);
      if (r.ic && r.ic.col) ic.style.color = r.ic.col;
      row.appendChild(ic);
      var lb = document.createElement('span');
      lb.className = 'lb';
      label(lb, r);
      row.appendChild(lb);
      if (r.ds) { var ds = document.createElement('span'); ds.className = 'ds'; ds.textContent = r.ds; row.appendChild(ds); }
      frag.appendChild(row);
    });
    list.appendChild(frag);
    list.scrollTop = top;
  }

  function show(id) {
    var el = id && list.querySelector('.row[data-id="' + (window.CSS && CSS.escape ? CSS.escape(id) : id) + '"]');
    if (el && el.scrollIntoView) el.scrollIntoView({ block: 'nearest' });
  }
  function select(id, tell) {
    sel = id;
    Array.prototype.forEach.call(list.querySelectorAll('.row.sel'), function (e) { e.classList.remove('sel'); });
    var el = list.querySelector('.row[data-id="' + (window.CSS && CSS.escape ? CSS.escape(id) : id) + '"]');
    if (el) el.classList.add('sel');
    show(id);
    if (tell) api.postMessage({ type: 'select', id: id });
  }
  function toggle(r) { api.postMessage({ type: 'toggle', id: r.id }); }
  function activate(r) { if (r.cmd) api.postMessage({ type: 'activate', id: r.id }); else if (r.tw) toggle(r); }

  // Ctrl+F / Ctrl+; anywhere here = the box. (A webview view does not set VS Code's focusedView and forwards every key
  // it does not stop to the workbench -- Ctrl+F would open the editor's find; stopped here, it stays ours.)
  document.addEventListener('keydown', function (e) {
    if ((e.ctrlKey || e.metaKey) && !e.altKey && !e.shiftKey && (e.key === 'f' || e.key === 'F' || e.key === ';')) {
      e.preventDefault(); e.stopPropagation(); q.focus(); q.select();
    }
  });
  list.addEventListener('click', function (e) {
    var el = e.target.closest && e.target.closest('.row');
    if (!el) return;
    // (the second click of a double click: the first one did it -- a folder would open and close again)
    if (e.detail > 1) return;
    var r = rows[idx(el.dataset.id)];
    if (!r) return;
    select(r.id, true);
    if (e.target.classList.contains('tw') && r.tw) toggle(r);
    else activate(r);
  });
  list.addEventListener('contextmenu', function (e) {
    var el = e.target.closest && e.target.closest('.row');
    if (el) select(el.dataset.id, true);
  });
  list.addEventListener('keydown', function (e) {
    if (e.ctrlKey || e.metaKey || e.altKey) return;
    var i = idx(sel), r = rows[i];
    // (a key handled here is not passed on to VS Code's keybindings)
    var go = function (j) { if (j >= 0 && j < rows.length) select(rows[j].id, true); e.preventDefault(); e.stopPropagation(); };
    if (/^(ArrowRight|ArrowLeft|Enter| )$/.test(e.key)) e.stopPropagation();
    if (e.key === 'ArrowDown') go(i < 0 ? 0 : i + 1);
    else if (e.key === 'ArrowUp') go(i < 0 ? 0 : i - 1);
    else if (e.key === 'Home') go(0);
    else if (e.key === 'End') go(rows.length - 1);
    else if (e.key === 'ArrowRight' && r) {
      e.preventDefault();
      if (r.tw === 1) toggle(r); else if (r.tw === 2 && rows[i + 1] && rows[i + 1].d > r.d) select(rows[i + 1].id, true);
    } else if (e.key === 'ArrowLeft' && r) {
      e.preventDefault();
      if (r.tw === 2) toggle(r);
      else for (var j = i - 1; j >= 0; j--) { if (rows[j].d < r.d) { select(rows[j].id, true); break; } }
    } else if ((e.key === 'Enter' || e.key === ' ') && r) { e.preventDefault(); activate(r); }
  });

  function boxState() { x.style.visibility = q.value ? 'visible' : 'hidden'; }
  function send(now) {
    clearTimeout(timer);
    var fire = function () {
      var v = q.value.trim();
      st.q = q.value; api.setState(st);
      if (v === sent) return;
      sent = v;
      api.postMessage({ type: 'query', q: v });
    };
    if (now) fire(); else timer = setTimeout(fire, 200);
  }
  q.value = st.q || '';
  boxState();
  q.addEventListener('input', function () { boxState(); send(false); });
  q.addEventListener('keydown', function (e) {
    // (typing stays in the box: not passed on to VS Code's keybindings)
    if (!e.ctrlKey && !e.metaKey) e.stopPropagation();
    if (e.key === 'Escape') { e.preventDefault(); q.value = ''; boxState(); send(true); }
    else if (e.key === 'Enter' || e.key === 'ArrowDown') {
      e.preventDefault(); send(true); list.focus();
      if (rows.length && idx(sel) < 0) select(rows[0].id, true);
    }
  });
  x.addEventListener('click', function () { q.value = ''; boxState(); send(true); q.focus(); });

  window.addEventListener('message', function (ev) {
    var m = ev.data || {};
    if (m.type === 'rows') {
      rows = m.rows || [];
      // (the page keeps its own selection -- the arrow keys may be ahead of the rows; the extension sets it on a reveal)
      if (m.sel !== undefined && m.sel !== null) sel = m.sel;
      msg.textContent = m.msg || '';
      draw();
      if (m.reveal) show(m.reveal);
    } else if (m.type === 'setq') {        // set from elsewhere (a command, the title bar's clear button)
      q.value = m.q || ''; sent = q.value.trim(); st.q = q.value; api.setState(st); boxState();
    } else if (m.type === 'run') { run = m.s || null; drawTb(); }
    else if (m.type === 'focus') { q.focus(); q.select(); }
    else if (m.type === 'focusList') { list.focus(); }
  });
  api.postMessage({ type: 'ready', q: q.value.trim() });
})();
