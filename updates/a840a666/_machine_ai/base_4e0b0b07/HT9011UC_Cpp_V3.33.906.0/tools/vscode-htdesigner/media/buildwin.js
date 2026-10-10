// AI(W906-HTDESIGNER) 20261003: the build window (EastSun 1003: "請你用個獨立建置框 並且要放大 讓我看清楚"): F5's build, big --
// the %, a thick bar, steps / total, the time; done = green, not finished = red. The extension sends { type: 'build', ... }.
(function () {
  const vscode = acquireVsCodeApi();
  document.body.innerHTML =
    '<div id="title"></div>' +
    '<div id="pct">0%</div>' +
    '<div class="bar"><div id="fill"></div></div>' +
    '<div id="info"></div>' +
    '<div id="state"></div>';
  const $ = id => document.getElementById(id);
  window.addEventListener('message', ev => {
    let m = ev.data || {};
    // (m.data: the same message from the panels test harness)
    if (m.type === 'build' && m.data) m = Object.assign({ type: 'build' }, m.data);
    if (m.type !== 'build') return;
    document.body.className = m.state;
    $('title').textContent = m.title || '建置';
    $('pct').textContent = (m.pct | 0) + '%';
    $('fill').style.width = (m.pct | 0) + '%';
    $('info').textContent = (m.steps ? m.steps + (m.total ? ' / ' + m.total : '') + ' 步　·　' : '') + (m.time || '') + (m.file ? '　·　' + m.file : '');
    $('state').textContent = m.state === 'done' ? '建置完成（這個視窗幾秒後自己關）' :
      m.state === 'stopped' ? '建置已停止（寫到一半的 exe 已刪掉，下次建置會重新連結）' :
      m.state === 'failed' ? '建置沒有完成' + (m.errors ? '：錯誤 ' + m.errors + ' 個' : '') + (m.lastError ? '，第一個：' + m.lastError : '') + '。原因在終端機和「問題」。' :
      '建置中…　輸出在終端機；要停就按工具列的 ⏹';
  });
  vscode.postMessage({ type: 'ready' });
})();
