// AI(W906-HTDESIGNER) 20261008 (feature gap #14): 工具箱 floated into a window of its own (Visual Studio's floating
// Toolbox). The rows are the side view's own tree items (Pointer, 常用 / 容器 / IO 元件 / 其他, 我的範本: the same
// labels, notes and tooltips -- the extension sends them as { type: 'rows' }), and a row does what it does there:
// a click = the item's own command (pick the tool up / put the template in), two quick clicks = added at once (the
// extension's double-click, the side view's), Enter = added (the side view's Enter), Shift+click = 連續放置 (the side
// view's right-click), Esc = 指標. The extension says which tool is in the hand: { type: 'sel', cls }.
(function () {
  const vscode = acquireVsCodeApi();
  let rows = [], sel = '@pointer', focusKey = null, q = '';
  const shut = new Set();
  document.body.innerHTML = '<input id="q" type="search" placeholder="搜尋工具箱" spellcheck="false"><div id="list" role="listbox" tabindex="0"></div><div id="tip"></div>';
  const $ = id => document.getElementById(id);
  const list = $('list');
  const esc = s => String(s == null ? '' : s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  const hit = r => !q || (r.label + ' ' + r.desc).toLowerCase().indexOf(q) >= 0;
  // the rows shown: a closed category hides its items; a search shows the items that match (their category with them)
  function shown() {
    const out = [];
    let grp = null, grpShown = false;
    for (const r of rows) {
      if (r.group) { grp = r; grpShown = false; if (!q) out.push(r); continue; }
      if (r.depth === 0) { if (hit(r)) out.push(r); grp = null; continue; }
      if (q) { if (hit(r)) { if (grp && !grpShown) { out.push(grp); grpShown = true; } out.push(r); } continue; }
      if (!(grp && shut.has(grp.key))) out.push(r);
    }
    return out;
  }
  function draw() {
    const s = shown();
    list.innerHTML = s.map(r => r.group
      ? '<div class="grp' + (shut.has(r.key) && !q ? ' shut' : '') + '" data-k="' + esc(r.key) + '" title="' + esc(r.tip || '') + '"><span class="tw"></span>' + esc(r.label) + '<span class="d">' + esc(r.desc) + '</span></div>'
      : '<div class="it' + (r.cls && r.cls === sel ? ' sel' : '') + (r.key === focusKey ? ' foc' : '') + '" role="option" data-k="' + esc(r.key) + '" style="padding-left:' + (8 + r.depth * 14) + 'px" title="' + esc(r.tip || '') + '">' +
        '<span class="l">' + esc(r.label) + '</span><span class="d">' + esc(r.desc) + '</span></div>').join('');
  }
  const rowOf = k => rows.find(r => r.key === k);
  function run(r, shift) {
    if (!r || r.group) return;
    focusKey = r.key;
    if (r.cls) sel = r.cls;
    draw();
    vscode.postMessage({ type: 'click', cmd: r.cmd, arg: r.arg, shift: !!shift });
  }
  list.addEventListener('click', e => {
    const el = e.target.closest('[data-k]');
    if (!el) return;
    const r = rowOf(el.getAttribute('data-k'));
    if (!r) return;
    if (r.group) { if (shut.has(r.key)) shut.delete(r.key); else shut.add(r.key); draw(); return; }
    run(r, e.shiftKey);
  });
  list.addEventListener('keydown', e => {
    const items = shown().filter(r => !r.group);
    let i = items.findIndex(r => r.key === focusKey);
    if (e.key === 'ArrowDown' || e.key === 'ArrowUp') {
      e.preventDefault();
      i = Math.max(0, Math.min(items.length - 1, i + (e.key === 'ArrowDown' ? 1 : -1)));
      focusKey = items[i] ? items[i].key : null;
      draw();
    } else if (e.key === 'Enter') {
      e.preventDefault();
      const r = items[i];
      if (r) { if (r.cls) sel = '@pointer'; draw(); vscode.postMessage({ type: 'add', cmd: r.cmd, arg: r.arg }); }
    } else if (e.key === 'Escape') {
      e.preventDefault();
      const p = rows.find(r => r.cls === '@pointer');
      if (p) run(p, false);
    }
  });
  $('q').addEventListener('input', () => { q = $('q').value.trim().toLowerCase(); draw(); });
  $('q').addEventListener('keydown', e => { if (e.key === 'ArrowDown') { e.preventDefault(); list.focus(); list.dispatchEvent(new KeyboardEvent('keydown', { key: 'ArrowDown' })); } });
  window.addEventListener('message', ev => {
    const m = ev.data || {};
    if (m.type === 'rows') { rows = Array.isArray(m.rows) ? m.rows : []; if (m.sel) sel = m.sel; draw(); }
    else if (m.type === 'sel') { sel = m.cls || '@pointer'; draw(); }
  });
  vscode.postMessage({ type: 'ready' });
})();
