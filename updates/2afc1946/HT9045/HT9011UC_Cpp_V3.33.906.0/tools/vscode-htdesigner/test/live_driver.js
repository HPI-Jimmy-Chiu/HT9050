// AI(W906-HTDESIGNER) 20260930: HW.IoSetView in a headless Edge, for test\probe_test.ps1 --
//   機種與機台設定: after the page's own scripts ran, what it decided (html.m-ht9050, data-machine, the
//   Above9050 tab opened);
//   設計時全部顯示: in DESIGN mode every tab and every component shows (grpLoaderFunc is Visible=False,
//   the golden tabs are hidden on a 9050); in OPERATE mode the page is as it runs.
// Written into <pre id="HTDTEST"> as { design: {...}, operate: {...} }.
(function () {
  var errors = [];
  window.addEventListener('error', function (e) { errors.push(String(e.message || e)); });
  function shown(el) { return !!el && el.getBoundingClientRect().width > 0; }
  function snap() {
    var h = document.documentElement;
    var tab = document.querySelector('#pgcStack1 > .pcTabs > .tab.ht9050-only');
    var tabs = document.querySelectorAll('#pgcStack1 > .pcTabs > .tab');
    var func = document.getElementById('grpLoaderFunc');
    return {
      mHt9050: h.classList.contains('m-ht9050'),
      dataMachine: h.getAttribute('data-machine') || '',
      aboveTab: !!tab,
      aboveActive: !!(tab && tab.classList.contains('act')),
      aboveShown: shown(tab),
      tabsAll: tabs.length,
      tabsShown: Array.prototype.filter.call(tabs, shown).length,
      funcDisplay: func ? getComputedStyle(func).display : 'missing',
      funcOwnNone: !!func && func.style.display === 'none',
      funcMarked: !!func && func.hasAttribute('data-htd-show'),
      /* 1006: hidden by a page RULE (the Above9050 tab, .ht9050-only, without HT9050): shown while designing AND marked
         執行時隱藏 -- and so is what is on its sheet (grpLoader_9050: the tree row's last column) */
      aboveMarked: !!tab && tab.hasAttribute('data-htd-show'),
      g9050Row: (function () { var a = (window.__htdOut || []).filter(function (m) { return m.type === 'tree'; }).pop(); var r = a && (a.rows || a.nodes || []).filter(function (x) { return x[2] === 'grpLoader_9050'; })[0]; return r ? r[8] : null; })(),
      answered: (window.__htdLiveAnswered || []).slice(),
      moved: (window.__htdLiveMoved || []).map(function (u) { return u.replace(/[?#].*$/, '').replace(/^.*\//, ''); }),
      errors: errors.slice()
    };
  }
  function lastEdit() {
    var o = (window.__htdOut || []).filter(function (m) { return m.type === 'edit'; });
    return o.length ? o[o.length - 1] : null;
  }
  function send(m) { m.__htd = 1; window.postMessage(m, '*'); }
  setTimeout(function () {
    var res = { design: snap() };
    // the Visible property while designing: off = display:none written, the component still shows
    var g = document.getElementById('grpLoader2');
    send({ type: 'setLook', id: 'grpLoader2', prop: 'visible', value: false });
    setTimeout(function () {
      var e1 = lastEdit();
      res.visOff = {
        edit: e1 ? e1.id + ' ' + JSON.stringify(e1.style) : null,
        own: g ? g.style.display : 'missing', display: g ? getComputedStyle(g).display : '', marked: !!g && g.hasAttribute('data-htd-show')
      };
      send({ type: 'setLook', id: 'grpLoader2', prop: 'visible', value: true });
      setTimeout(function () {
        var e2 = lastEdit();
        res.visOn = {
          edit: e2 ? e2.id + ' ' + JSON.stringify(e2.style) : null,
          own: g ? g.style.display : 'missing', display: g ? getComputedStyle(g).display : '', marked: !!g && g.hasAttribute('data-htd-show')
        };
        send({ type: 'setMode', mode: 'operate' });
        setTimeout(function () {
          res.operate = snap();
          var pre = document.createElement('pre');
          pre.id = 'HTDTEST';
          pre.textContent = 'HTDTEST' + JSON.stringify(res) + 'HTDEND';
          document.body.appendChild(pre);
        }, 400);
      }, 400);
    }, 400);
  }, 2500);
})();
