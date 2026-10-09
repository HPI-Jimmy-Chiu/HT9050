// AI(W906-HTDESIGNER) 20261003: the 尋找 window (EastSun 1003: "你搜尋關鍵字的視窗 可以額外的視窗嗎 不要用內建的 並且我可以看到我之前搜尋了什麼"):
// Visual Studio's Find in Files dialog + its Find Results -- the keyword, 搜尋範圍, the three options, the searches made
// before (a click searches again), the results (a click opens the line). The extension does the searching.
(function () {
  const vscode = acquireVsCodeApi();
  const $ = id => document.getElementById(id);
  const esc = s => String(s == null ? '' : s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  let st = { scopes: [], scope: '', opts: {}, history: [], res: null, busy: false };
  // (1003, EastSun: "可以加入讓我勾選篩選條件嗎? 我想加入 被賦予值 或是被當成判斷式 或是是函式 的篩選")
  const KINDS = [['assign', '被賦予值', '賦值'], ['cond', '被當成判斷式', '判斷'], ['func', '是函式', '函式']];
  document.body.innerHTML =
    '<div id="top">' +
    '<div class="row"><label for="q">尋找目標</label><input id="q" type="text" spellcheck="false" placeholder="要找的字，例如 DoInArm、spbSave、吸嘴"><button id="go" class="pri">尋找</button></div>' +
    // (1003, EastSun: "我搜尋視窗 要有可以替換關鍵字的功能": Visual Studio's Replace in Files)
    '<div class="row"><label for="rq">取代為</label><input id="rq" type="text" spellcheck="false" placeholder="換成這個字（正規式時可用 $1、$2）"><button id="rp1" title="只換結果裡點選的那一筆" disabled>取代</button><button id="rpa" title="把列出來的結果全部換掉（不會存檔：要按「全部儲存」；Ctrl+Z 可以復原；BCB6 原始碼不改）" disabled>全部取代</button></div>' +
    '<div class="row"><label for="scope">搜尋範圍</label><select id="scope"></select></div>' +
    '<div class="row opts"><label><input type="checkbox" id="caseSensitive"> 大小寫要相同</label><label><input type="checkbox" id="wholeWord"> 全字</label><label><input type="checkbox" id="regex"> 正規式</label></div>' +
    '<div class="row opts" title="勾了就只列出這幾種用法（任一種），註解和字串裡的不算；都不勾＝全部"><span class="ol">篩選</span>' + KINDS.map(k => '<label><input type="checkbox" id="k_' + k[0] + '"> ' + k[1] + '</label>').join('') + '<label title="// 和 /* */ 註解裡的、#if 0 區塊裡的不列（字串裡的照列）"><input type="checkbox" id="noComment"> 不含註解</label><label title="只搜 C／C++ 原始檔：.cpp、.h（.c .cc .cxx .hpp .hh .hxx 也算）；網頁、CSV、文件都不找"><input type="checkbox" id="onlyCpp"> 只找 .cpp／.h</label></div>' +
    '</div>' +
    '<div id="split"><section id="hist"><h3>之前搜尋的<button id="clr" class="lnk" title="清除全部紀錄">清除</button></h3><ul id="hl"></ul></section>' +
    '<section id="out"><h3 id="sum">結果</h3><div id="rl"></div></section></div>';
  const q = $('q');
  const send = m => vscode.postMessage(m);
  const find = () => {
    const v = q.value;
    if (!v.trim()) { q.focus(); return; }
    const kinds = {};
    for (const k of KINDS) kinds[k[0]] = $('k_' + k[0]).checked;
    send({ type: 'find', q: v, scope: $('scope').value, opts: { caseSensitive: $('caseSensitive').checked, wholeWord: $('wholeWord').checked, regex: $('regex').checked, noComment: $('noComment').checked, onlyCpp: $('onlyCpp').checked, kinds } });
  };
  $('go').addEventListener('click', find);
  // (1005, EastSun: "我搜尋視窗 搜尋時 搜尋範圍都會重製 你可以記憶住嗎? 我其他選項也要記憶住": every change of 搜尋範圍, the options,
  //  the filters and 取代為 is told to the extension at once -- kept there (and across a restart), not only when searching)
  const curOpts = () => { const kinds = {}; for (const k of KINDS) kinds[k[0]] = $('k_' + k[0]).checked; return { caseSensitive: $('caseSensitive').checked, wholeWord: $('wholeWord').checked, regex: $('regex').checked, noComment: $('noComment').checked, onlyCpp: $('onlyCpp').checked, kinds }; };
  const prefs = () => { st.scope = $('scope').value; st.opts = curOpts(); send({ type: 'prefs', scope: st.scope, opts: st.opts, rq: $('rq').value }); };
  $('scope').addEventListener('change', prefs);
  for (const id of ['caseSensitive', 'wholeWord', 'regex', 'noComment', 'onlyCpp'].concat(KINDS.map(k => 'k_' + k[0]))) $(id).addEventListener('change', prefs);
  $('rq').addEventListener('change', prefs);
  let selI = null;
  // (1009 review #1 / #2: off while a search runs (the list is the new one's, a replace would use the last one's) and while
  //  a replace runs (a second click placed by the same old text wrote into the first one's result))
  let replacing = false;
  const rpState = () => { const any = !st.busy && !replacing && !!(st.res && st.res.hits && st.res.hits.length); $('rpa').disabled = !any; $('rp1').disabled = !(any && selI != null); };
  const startReplace = m => { replacing = true; rpState(); $('sum').textContent = '取代中…'; send(m); };
  $('rp1').addEventListener('click', () => { if (selI != null && !replacing) startReplace({ type: 'replace', all: false, i: selI, text: $('rq').value }); });
  $('rpa').addEventListener('click', () => { if (!replacing) startReplace({ type: 'replace', all: true, text: $('rq').value }); });
  q.addEventListener('keydown', e => { if (e.key === 'Enter') { e.preventDefault(); find(); } });
  $('clr').addEventListener('click', () => send({ type: 'clearHistory' }));
  // (Ctrl+F in this window: its own box, not VS Code's find)
  document.addEventListener('keydown', e => {
    if (e.ctrlKey && !e.altKey && !e.shiftKey && (e.key === 'f' || e.key === 'F')) { e.preventDefault(); e.stopPropagation(); q.focus(); q.select(); }
  }, true);

  function drawScopes() {
    $('scope').innerHTML = st.scopes.map(s => '<option value="' + esc(s.key) + '"' + (s.key === st.scope ? ' selected' : '') + '>' + esc(s.label + (s.description ? '　' + s.description : '')) + '</option>').join('');
    for (const k of ['caseSensitive', 'wholeWord', 'regex', 'noComment', 'onlyCpp']) $(k).checked = !!st.opts[k];
    for (const k of KINDS) $('k_' + k[0]).checked = !!(st.opts.kinds && st.opts.kinds[k[0]]);
  }
  function drawHistory() {
    const hl = $('hl');
    hl.innerHTML = st.history.length ? st.history.map((h, i) =>
      '<li data-i="' + i + '" title="按一下＝再找一次"><span class="hq">' + esc(h.q) + '</span><span class="hm">' + esc(h.scopeLabel || '') +
      (h.opts ? ['caseSensitive', 'wholeWord', 'regex', 'noComment', 'onlyCpp'].filter(k => h.opts[k]).map(k => ({ caseSensitive: '　Aa', wholeWord: '　全字', regex: '　正規式', noComment: '　不含註解', onlyCpp: '　.cpp/.h' }[k])).join('') : '') +
      (h.opts && h.opts.kinds ? KINDS.filter(k => h.opts.kinds[k[0]]).map(k => '　' + k[2]).join('') : '') +
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
    // (1007 audit: the file / project the entry was made on goes along -- a 目前檔案 entry searched whatever file was current)
    send({ type: 'find', q: h.q, scope: h.scope, file: h.file || null, scopeLabel: h.scopeLabel || '', opts: h.opts || {} });
  });
  // (1008, EastSun「可以先把搜尋到的列上去? 不用等到全部搜尋完成」: while the search runs, the hits found so far are listed
  //  as they come -- a click opens one already; the final list, in file order, replaces them at the end)
  let partN = 0, partKey = '', partGroup = null;
  function appendPartial(pt) {
    if (!st.busy || !pt || !pt.add) return;
    const rl = $('rl');
    if (!partN) rl.innerHTML = '';
    for (const h of pt.add) {
      const k = h.area + '|' + h.rel;
      if (k !== partKey || !partGroup) {
        partGroup = document.createElement('div');
        partGroup.className = 'fg';
        partGroup.innerHTML = '<div class="f" title="' + esc(h.file) + '">' + esc(h.rel) + '<span class="fa">' + esc(h.areaLabel || '') + '</span></div>';
        rl.appendChild(partGroup);
        partKey = k;
      }
      const a = Math.max(0, h.col - 1), t = h.text;
      partGroup.insertAdjacentHTML('beforeend', '<div class="h" data-p="' + h.p + '"><span class="ln">' + h.line + '</span>' + esc(t.slice(0, a)) + '<mark>' + esc(t.slice(a, a + h.len)) + '</mark>' + esc(t.slice(a + h.len)) + '</div>');
      partN++;
    }
    $('sum').textContent = '搜尋中…「' + pt.q + '」已找到 ' + partN + ' 筆（還在找，可以先點）';
  }
  function drawResults() {
    const r = st.res, rl = $('rl');
    if (st.busy) { partN = 0; partKey = ''; partGroup = null; rl.innerHTML = ''; $('sum').textContent = '搜尋中…'; selI = null; rpState(); return; }
    selI = null;
    rpState();
    if (!r) { $('sum').textContent = '結果'; rl.innerHTML = ''; return; }
    $('sum').textContent = '「' + r.q + '」' + r.n + (r.truncated ? '+' : '') + ' 筆、' + r.files + ' 個檔' + (r.scopeLabel ? '　在 ' + r.scopeLabel : '') + '（' + r.ms + ' ms）';
    if (!r.hits.length) { rl.innerHTML = '<div class="none">沒有找到</div>'; return; }
    // (one block per file: its name stays on top only while its own lines scroll -- names never stack on each other)
    let html = '', last = '';
    for (const h of r.hits) {
      const k = h.area + '|' + h.rel;
      if (k !== last) { html += (last ? '</div>' : '') + '<div class="fg"><div class="f" title="' + esc(h.file) + '">' + esc(h.rel) + '<span class="fa">' + esc(h.areaLabel || '') + '</span></div>'; last = k; }
      const a = Math.max(0, h.col - 1), t = h.text;
      // (the use it is, before the line -- a long line is cut at the right)
      const kd = h.kinds && h.kinds.length ? '<span class="kd">' + KINDS.filter(k => h.kinds.indexOf(k[0]) >= 0).map(k => k[2]).join('・') + '</span>' : '';
      html += '<div class="h" data-i="' + h.i + '"><span class="ln">' + h.line + '</span>' + kd + esc(t.slice(0, a)) + '<mark>' + esc(t.slice(a, a + h.len)) + '</mark>' + esc(t.slice(a + h.len)) + '</div>';
    }
    if (last) html += '</div>';
    if (r.shown < r.n) html += '<div class="none">只列前 ' + r.shown + ' 筆（打長一點、或開「全字」）</div>';
    rl.innerHTML = html;
  }
  $('rl').addEventListener('click', e => {
    const h = e.target.closest('.h');
    if (!h) return;
    for (const o of document.querySelectorAll('.h.sel')) o.classList.remove('sel');
    h.classList.add('sel');
    // (a hit listed while the search still runs: by its place in what was sent so far -- 取代 waits for the final list)
    if (h.dataset.p !== undefined) { send({ type: 'openPartial', p: +h.dataset.p }); return; }
    selI = +h.dataset.i;
    rpState();
    send({ type: 'open', i: +h.dataset.i });
  });
  window.addEventListener('message', ev => {
    const m = ev.data || {};
    // (m.data: the same state from the panels test harness)
    if (m.type === 'state' && !m.s && m.data) m.s = m.data;
    if (m.type === 'state') {
      if (!m.s || typeof m.s !== 'object') return;   // (1007 audit: a state without s threw)
      st = Object.assign(st, m.s);
      if (m.s.scopes || m.s.opts) drawScopes();
      if (m.s.history) drawHistory();
      if ('res' in m.s || 'busy' in m.s) drawResults();
      if (m.s.partial) appendPartial(m.s.partial);
      if (m.s.replaced) {
        const x = m.s.replaced;
        replacing = false;
        rpState();
        const why = [x.multiline ? x.multiline + ' 處跨行' : '', x.enc ? x.enc + ' 處檔案不是 UTF-8' : '', x.changed ? x.changed + ' 處檔案已經改過' : ''].filter(Boolean).join('、');
        $('sum').textContent = x.busy ? '還在搜尋或取代中，等它完成再按'
          : x.failed ? 'VS Code 沒有接受這次取代（' + x.failed + ' 處：檔案唯讀或位置重疊），什麼都沒改'
          : '取代了 ' + x.done + ' 處（' + x.files + ' 個檔，還沒存檔）' + (x.skipped ? '，略過 ' + x.skipped + ' 處' + (why ? '（' + why + '）' : '') : '') + (x.golden ? '，BCB6 原始碼 ' + x.golden + ' 處不改' : '') +
            '　　剩 ' + (st.res ? st.res.n : 0) + ' 筆' + (x.more ? '　⚠ 搜尋結果沒有列完整（每檔 500、總共 5000 筆，或中途取消），檔案裡可能還有，再搜一次' : '');
      }
      // (1009 review #10: a search another one took over -- the list is the last one's)
      if (m.s.note) { $('sum').textContent += '　（' + m.s.note + '）'; st.note = null; }
      if (typeof m.s.q === 'string') q.value = m.s.q;
      if (typeof m.s.rq === 'string') $('rq').value = m.s.rq;
      if (m.focus) { q.focus(); q.select(); }
    }
  });
  send({ type: 'ready' });
})();
