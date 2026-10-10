/* AI(W906-HTDESIGNER) 20261010 (review of webview messages F1 F2, review of 0.435 B1): the probe as it is IN VS Code --
   no __htdTestEvents, so only the user's own input and the host's messages count. Run by probe_test.ps1 on
   Data.LotInfo.html (hand-written .tab / .tabPane sheets). Writes one line "HTDTEST{json}HTDEND". */
(function () {
  var R = {};
  function msgs(t) { return (window.__htdOut || []).filter(function (m) { return m.type === t; }); }
  function shown(el) { return !!el && getComputedStyle(el).display !== 'none'; }
  function finish() {
    var pre = document.createElement('pre');
    pre.id = 'HTDTEST';
    pre.textContent = 'HTDTEST' + JSON.stringify(R) + 'HTDEND';
    document.body.appendChild(pre);
  }
  setTimeout(function () {
    try {
      var P = window.__htdProbe;
      R.ready = !!P && P.mode() === 'design';
      // B1: a component on a hidden hand-written tab, selected from the outline -- the probe's own tab.click() reaches the page
      var pane = document.querySelector('.tabPane[data-pane="atc"]');
      R.paneHiddenBefore = !shown(pane);
      R.selected = P.selectById('palATCWorkingTemp');
      R.paneShownAfter = shown(pane);
      // F1: a page script's dblclick / Delete / copy -- not a designer gesture, and in design mode the page does not get it
      var btn = document.getElementById('btTesterTCPShow');
      var pageSaw = 0;
      document.addEventListener('dblclick', function () { pageSaw++; });
      var nD = msgs('dblclick').length, nC = msgs('cmd').length, nE = msgs('edit').length;
      btn.dispatchEvent(new MouseEvent('mousedown', { bubbles: true, button: 0 }));
      btn.dispatchEvent(new MouseEvent('dblclick', { bubbles: true }));
      window.dispatchEvent(new KeyboardEvent('keydown', { key: 'Delete', bubbles: true }));
      document.dispatchEvent(new Event('copy', { bubbles: true }));
      R.madeDblclick = msgs('dblclick').length - nD;
      R.madeCmd = msgs('cmd').length - nC;
      R.madeEdit = msgs('edit').length - nE;
      R.pageSawDblclick = pageSaw;
      // F2: the page's own postMessage and the tests' hook -- not the host's
      window.postMessage({ __htd: 1, type: 'setMode', mode: 'operate' }, '*');
      P.message({ type: 'setMode', mode: 'operate' });
      setTimeout(function () {
        R.modeAfterForged = P.mode();
        finish();
      }, 300);
    } catch (e) { R.error = String(e && e.stack || e); finish(); }
  }, 1500);
})();
