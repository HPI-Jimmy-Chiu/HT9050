/* AI(W906-HTDESIGNER) 20260930: 搜尋頁面 -- the search box above the 頁面 list (like the one on top of
 * Visual Studio's Solution Explorer). EastSun: "我在頁面這邊需要一個可以搜尋關鍵字的輸入點".
 * Typing filters the list below it (the pages, and under each the components that have the words);
 * Enter opens the best one, Esc clears, ↓ goes into the list. The searching is done by the extension. */
(function () {
  'use strict';
  var api = acquireVsCodeApi();
  var st = api.getState() || {};
  var root = document.getElementById('root');
  root.innerHTML =
    '<div class="box"><span class="ico" aria-hidden="true">&#x1F50D;&#xFE0E;</span>' +
    '<input id="q" type="text" spellcheck="false" autocomplete="off" placeholder="搜尋頁面、元件名稱、畫面上的文字" aria-label="搜尋頁面">' +
    '<button id="x" type="button" title="清除（Esc）" aria-label="清除">&#x2715;</button></div>' +
    '<div id="sum" class="sum"></div>';
  var q = document.getElementById('q');
  var x = document.getElementById('x');
  var sum = document.getElementById('sum');
  var TIP = '打字就會篩選下面的「頁面」：頁面名稱、元件名稱、型別、畫面上的文字都找。空白隔開＝每個字都要有。Enter 開第一個，↓ 到清單。';
  var timer = null;
  var sent = null;

  function show(s) {
    x.style.visibility = q.value ? 'visible' : 'hidden';
    if (!q.value) { sum.textContent = TIP; sum.className = 'sum tip'; return; }
    if (!s || s.query !== q.value.trim()) return;
    sum.textContent = s.text;
    sum.className = 'sum' + (s.none ? ' none' : '');
  }
  function send(now) {
    clearTimeout(timer);
    var go = function () {
      var v = q.value.trim();
      api.setState({ q: q.value });
      if (v === sent) return;
      sent = v;
      api.postMessage({ type: 'query', q: v });
    };
    if (now) go(); else timer = setTimeout(go, 200);
  }

  q.value = st.q || '';
  show(null);
  q.addEventListener('input', function () { show(null); send(false); });
  q.addEventListener('keydown', function (e) {
    if (e.key === 'Enter') { e.preventDefault(); send(true); api.postMessage({ type: 'open' }); }
    else if (e.key === 'Escape') { e.preventDefault(); q.value = ''; show(null); send(true); }
    else if (e.key === 'ArrowDown') { e.preventDefault(); send(true); api.postMessage({ type: 'toList' }); }
  });
  x.addEventListener('click', function () { q.value = ''; show(null); send(true); q.focus(); });
  window.addEventListener('focus', function () { q.focus(); });
  window.addEventListener('message', function (ev) {
    var m = ev.data || {};
    if (m.type === 'result') show(m);
    else if (m.type === 'set') {           // set from elsewhere (a command, the list's clear button)
      q.value = m.q || '';
      sent = q.value.trim();
      api.setState({ q: q.value });
      show(m.summary || null);
    } else if (m.type === 'focus') { q.focus(); q.select(); }
  });
  api.postMessage({ type: 'ready', q: q.value.trim() });
})();
