// AI(W906-HTDESIGNER) 20261003: the 尋找 window (EastSun 1003: "你搜尋關鍵字的視窗 可以額外的視窗嗎 不要用內建的 並且我可以看到我之前搜尋了什麼"):
// Visual Studio's Find in Files dialog + its Find Results -- the keyword, 搜尋範圍, the three options, the searches made
// before (a click searches again), the results (a click opens the line). The extension does the searching.
(function () {
  const vscode = acquireVsCodeApi();
  const $ = id => document.getElementById(id);
  const esc = s => String(s == null ? '' : s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  let st = { scopes: [], scope: '', opts: {}, history: [], res: null, busy: false };
  document.body.innerHTML =
    '<div id="top">' +
    '<div class="row"><label for="q">尋找目標</label><input id="q" type="text" spellcheck="false" placeholder="要找的字，例如 DoInArm、spbSave、吸嘴"><button id="go" class="pri">尋找</button></div>' +
    '<div class="row"><label for="scope">搜尋範圍</label><select id="scope"></select></div>' +
    '<div class="row opts"><label><input type="checkbox" id="caseSensitive"> 大小寫要相同</label><label><input type="checkbox" id="wholeWord"> 全字</label><label><input type="checkbox" id="regex"> 正規式</label></div>' +
    '</div>' +
    '<div id="split"><section id="hist"><h3>之前搜尋的<button id="clr" class="lnk" title="清除全部紀錄">清除</button></h3><ul id="hl"></ul></section>' +
    '<section id="out"><h3 id="sum">結果</h3><div id="rl"></div></section></div>';
  const q = $('q');
  const send = m => vscode.postMessage(m);
  const find = () => {
    const v = q.value;
    if (!v.trim()) { q.focus(); return; }
    send({ type: 'find', q: v, scope: $('scope').value, opts: { caseSensitive: $('caseSensitive').checked, wholeWord: $('wholeWord').checked, regex: $('regex').checked } });
  };
  $('go').addEventListener('click', find);
  q.addEventListener('keydown', e => { if (e.key === 'Enter') { e.preventDefault(); find(); } });
  $('clr').addEventListener('click', () => send({ type: 'clearHistory' }));
  // (Ctrl+F in this window: its own box, not VS Code's find)
  document.addEventListener('keydown', e => {
    if (e.ctrlKey && !e.altKey && !e.shiftKey && (e.key === 'f' || e.key === 'F')) { e.preventDefault(); e.stopPropagation(); q.focus(); q.select(); }
  }, true);

  function drawScopes() {
    $('scope').innerHTML = st.scopes.map(s => '<option value="' + esc(s.key) + '"' + (s.key === st.scope ? ' selected' : '') + '>' + esc(s.label + (s.description ? '　' + s.description : '')) + '</option>').join('');
    for (const k of ['caseSensitive', 'wholeWord', 'regex']) $(k).checked = !!st.opts[k];
  }
  function drawHistory() {
    const hl = $('hl');
    hl.innerHTML = st.history.length ? st.history.map((h, i) =>
      '<li data-i="' + i + '" title="按一下＝再找一次"><span class="hq">' + esc(h.q) + '</span><span class="hm">' + esc(h.scopeLabel || '') +
      (h.opts ? ['caseSensitive', 'wholeWord', 'regex'].filter(k => h.opts[k]).map(k => ({ caseSensitive: '　Aa', wholeWord: '　全字', regex: '　正規式' }[k])).join('') : '') +
      (h.n != null ? '　' + h.n + ' 筆' : '') + '　' + esc(h.when || '') + '</span><button class="x" data-x="' + i + '" title="刪掉這筆紀錄">×</button></li>').join('')
      : '<li class="none">還沒有搜尋過</li>';
  }
  $('hl').addEventListener('click', e => {
    const x = e.target.closest('[data-x]');
    if (x) { e.stopPropagation(); send({ type: 'delHistory', i: +x.dataset.x }); return; }
    const li = e.target.closest('li[data-i]');
    if (!li) return;
    const h = st.history[+li.dataset.i];
    if (!h) return;
    q.value = h.q;
    send({ type: 'find', q: h.q, scope: h.scope, opts: h.opts || {} });
  });
  function drawResults() {
    const r = st.res, rl = $('rl');
    if (st.busy) { $('sum').textContent = '搜尋中…'; return; }
    if (!r) { $('sum').textContent = '結果'; rl.innerHTML = ''; return; }
    $('sum').textContent = '「' + r.q + '」' + r.n + (r.truncated ? '+' : '') + ' 筆、' + r.files + ' 個檔' + (r.scopeLabel ? '　在 ' + r.scopeLabel : '') + '（' + r.ms + ' ms）';
    if (!r.hits.length) { rl.innerHTML = '<div class="none">沒有找到</div>'; return; }
    let html = '', last = '';
    for (const h of r.hits) {
      const k = h.area + '|' + h.rel;
      if (k !== last) { html += '<div class="f" title="' + esc(h.file) + '">' + esc(h.rel) + '<span class="fa">' + esc(h.areaLabel || '') + '</span></div>'; last = k; }
      const a = Math.max(0, h.col - 1), t = h.text;
      html += '<div class="h" data-i="' + h.i + '"><span class="ln">' + h.line + '</span>' + esc(t.slice(0, a)) + '<mark>' + esc(t.slice(a, a + h.len)) + '</mark>' + esc(t.slice(a + h.len)) + '</div>';
    }
    if (r.shown < r.n) html += '<div class="none">只列前 ' + r.shown + ' 筆（打長一點、或開「全字」）</div>';
    rl.innerHTML = html;
  }
  $('rl').addEventListener('click', e => {
    const h = e.target.closest('.h');
    if (!h) return;
    for (const o of document.querySelectorAll('.h.sel')) o.classList.remove('sel');
    h.classList.add('sel');
    send({ type: 'open', i: +h.dataset.i });
  });
  window.addEventListener('message', ev => {
    const m = ev.data || {};
    // (m.data: the same state from the panels test harness)
    if (m.type === 'state' && !m.s && m.data) m.s = m.data;
    if (m.type === 'state') {
      st = Object.assign(st, m.s);
      if (m.s.scopes || m.s.opts) drawScopes();
      if (m.s.history) drawHistory();
      if ('res' in m.s || 'busy' in m.s) drawResults();
      if (typeof m.s.q === 'string') q.value = m.s.q;
      if (m.focus) { q.focus(); q.select(); }
    }
  });
  send({ type: 'ready' });
})();
