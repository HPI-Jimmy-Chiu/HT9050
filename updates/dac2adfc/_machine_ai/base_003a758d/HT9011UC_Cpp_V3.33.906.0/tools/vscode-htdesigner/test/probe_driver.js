/* AI(W906-HTDESIGNER) 20260929: probe checks, run inside the harness page by a headless
   browser. Writes one line "HTDTEST{json}HTDEND" into the DOM for probe_test.ps1. */
(function () {
  var R = { page: document.title };
  var vio = [];
  document.addEventListener('securitypolicyviolation', function (e) {
    vio.push((e.effectiveDirective || e.violatedDirective) + ' ' + e.blockedURI);
  });
  function msgs(t) { return (window.__htdOut || []).filter(function (m) { return m.type === t; }); }
  function finish() {
    R.violations = vio;
    R.blockedMsgs = msgs('blocked').map(function (m) { return m.dir + ' ' + m.uri; });
    /* (the information bar test's own error does not count) */
    R.pageErrors = msgs('pageError').filter(function (m) { return !/htd-infobar-test/.test(m.msg); }).map(function (m) { return m.msg + ' @' + String(m.src).split('/').pop() + ':' + m.line; });
    var pre = document.createElement('pre');
    pre.id = 'HTDTEST';
    pre.textContent = 'HTDTEST' + JSON.stringify(R) + 'HTDEND';
    document.body.appendChild(pre);
  }
  setTimeout(function () {
    try {
      R.ready = msgs('ready').length > 0;
      R.mode = window.__htdProbe && window.__htdProbe.mode();
      var tree = msgs('tree').pop();
      R.treeCount = tree ? tree.nodes.length : 0;
      R.treeHasForm = !!(tree && tree.nodes.some(function (n) { return n[2] === '@form'; }));
      /* WPF: nothing picked yet = the root is the selection (the Properties window shows the Window) -- a first init
         without selectId selects the form */
      var selBefore = msgs('select').length;
      var hadSel = !!(window.__htdProbe && window.__htdProbe.hasSelection && window.__htdProbe.hasSelection());
      if (window.__htdProbe && !hadSel) window.__htdProbe.message({ type: 'init', classes: {} });
      var selInit = msgs('select').slice(selBefore).pop();
      R.initSelectsForm = hadSel || !document.querySelector('div.form') || !!(selInit && selInit.info && selInit.info.id === '@form');
      R.initSelectsFormTried = hadSel ? 'had a selection already' : (selInit && selInit.info ? selInit.info.id : 'no select');

      /* a button with an id: prefer the save button of the generated pages */
      var btn = document.getElementById('spbSave') || document.querySelector('button[id]');
      R.button = btn ? btn.id : null;
      if (btn) {
        /* registered after the probe on window/capture: runs only if the probe lets
           the click through (page scripts may stop it further down, so the probe is
           judged here, not by a listener on the button) */
        var ran = 0;
        window.addEventListener('click', function htdTestHandler(e) { if (e.target === btn) ran++; }, true);
        btn.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true }));
        R.blockedInDesign = ran === 0;
        var sel = msgs('select').pop();
        R.selId = sel && sel.info ? sel.info.id : null;
        R.selVcl = sel && sel.info ? sel.info.title : null;
        R.selListeners = sel && sel.info ? sel.info.listeners.map(function (l) {
          var f = l.frames[0];
          return l.type + '@' + (f ? String(f.url).split('/').pop() + ':' + f.line : '?') + (l.fnName ? ' ' + l.fnName : '');
        }) : [];
        R.selInherited = sel && sel.info ? sel.info.inherited.map(function (a) {
          return a.label + ': ' + a.list.map(function (l) {
            var f = l.frames[0];
            return l.type + '@' + (f ? String(f.url).split('/').pop() + ':' + f.line : '?');
          }).join(', ');
        }) : [];
        R.selGeom = sel && sel.info ? sel.info.geom : null;
        R.selInfo = sel ? sel.info : null;           /* full, for test\smoke_extension.js */
        R.treeNodes = tree ? tree.nodes : [];
        /* operate mode hands clicks to the page (the network is still walled off) */
        window.postMessage({ __htd: 1, type: 'setMode', mode: 'operate' }, '*');
        R._ranBefore = ran;
        setTimeout(function () {
          btn.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true }));
          R.operateModeOperates = ran === R._ranBefore + 1;
          window.postMessage({ __htd: 1, type: 'setMode', mode: 'design' }, '*');
        }, 50);

        /* Enter = open the default event, View Code only: never adds one (arrows/Esc/Tab are checked in editTests) */
        var before = msgs('dblclick').length;
        window.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true }));
        var ents = msgs('dblclick');
        R.keyEnterOpens = ents.length === before + 1 && ents[ents.length - 1].viewOnly === true;
      }

      /* a component inside a tab page that is not showing */
      var pane = Array.prototype.find.call(document.querySelectorAll('.pcPane'), function (p) {
        return getComputedStyle(p).display === 'none' && p.querySelector('[id]');
      });
      if (pane) {
        var inner = pane.querySelector('[id]');
        R.hiddenTarget = inner.id;
        window.__htdProbe.selectById(inner.id);
        R.hiddenTabRevealed = getComputedStyle(pane).display !== 'none';
      }
      /* clicking a tab still switches it in design mode */
      var tab = Array.prototype.find.call(document.querySelectorAll('.pcTabs > .tab'), function (t) {
        return !t.classList.contains('act');
      });
      if (tab) {
        tab.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true }));
        R.tabClickSwitches = tab.classList.contains('act');
      }

      /* the wall: nothing may leave the page (closed port 9, never the machine) */
      try { new WebSocket('ws://127.0.0.1:9/htdtest'); R.wsConstructor = 'no throw'; } catch (e) { R.wsConstructor = String(e.name); }
      try { fetch('http://127.0.0.1:9/htdtest').then(function () { R.fetchResult = 'resolved'; }, function (e) { R.fetchResult = 'rejected ' + e.name; }); } catch (e) { R.fetchResult = 'threw ' + e.name; }
      try {
        var x = new XMLHttpRequest();
        x.open('GET', 'http://127.0.0.1:9/htdtest');
        x.onerror = function () { R.xhrResult = 'error'; };
        x.send();
      } catch (e) { R.xhrResult = 'threw ' + e.name; }
      R.iframeCount = document.querySelectorAll('iframe').length;
    } catch (e) {
      R.driverError = String(e && e.stack || e);
    }
    setTimeout(editTests, 300);
  }, 1500);

  /* WPF-style editing (DOM only here; the extension writes the source) */
  function editTests() {
    try {
      var btn = document.getElementById('spbSave');
      if (!btn) { setTimeout(finish, 100); return; }
      var edits = function () { return msgs('edit'); };
      var mouse = function (type, target, x, y, extra) {
        var o = { bubbles: true, cancelable: true, composed: true, clientX: x, clientY: y, button: 0 };
        for (var k in extra || {}) o[k] = extra[k];
        target.dispatchEvent(new MouseEvent(type, o));
      };
      var r = btn.getBoundingClientRect();
      var cx = r.left + r.width / 2, cy = r.top + r.height / 2;
      var l0 = btn.style.left, t0 = btn.style.top;
      /* drag the Save button by (+20, +10), Alt held = no snapping, exact deltas */
      var ALT = { altKey: true };
      var n0 = edits().length;
      mouse('mousedown', btn, cx, cy);
      mouse('mousemove', btn, cx + 10, cy + 5, ALT);
      mouse('mousemove', btn, cx + 20, cy + 10, ALT);
      mouse('mouseup', btn, cx + 20, cy + 10);
      var e1 = edits()[n0];
      R.dragEdit = e1 ? JSON.stringify(e1.style) + ' ' + e1.target : null;
      R.dragOk = !!e1 && e1.what === 'style' && e1.id === 'spbSave' && e1.style.left === (parseFloat(l0) + 20) + 'px' && e1.style.top === (parseFloat(t0) + 10) + 'px' && btn.style.left === e1.style.left;
      /* a click without moving is not a drag */
      var n1 = edits().length;
      mouse('mousedown', btn, cx + 20, cy + 10);
      mouse('mouseup', btn, cx + 20, cy + 10);
      R.clickNoEdit = edits().length === n1;
      /* resize from the south-east handle (open shadow root) */
      var host = document.querySelector('htd-overlay');
      var se = host && host.shadowRoot ? host.shadowRoot.querySelector('.hd[data-dir="se"]') : null;
      if (se) {
        var hr = se.getBoundingClientRect();
        var w0 = btn.style.width, h0 = btn.style.height;
        var n2 = edits().length;
        mouse('mousedown', se, hr.left + 3, hr.top + 3);
        mouse('mousemove', btn, hr.left + 3 + 15, hr.top + 3 + 6, ALT);
        mouse('mouseup', btn, hr.left + 3 + 15, hr.top + 3 + 6);
        var e2 = edits()[n2];
        R.resizeEdit = e2 ? JSON.stringify(e2.style) : null;
        R.resizeOk = !!e2 && e2.style.width === (parseFloat(w0) + 15) + 'px' && e2.style.height === (parseFloat(h0) + 6) + 'px' && !('left' in e2.style);
        /* WPF / Blend: Shift on a corner handle keeps the proportions (30 right, 2 down -> both grow by the same factor) */
        var w1 = parseFloat(btn.style.width), h1 = parseFloat(btn.style.height);
        var n2b = edits().length;
        mouse('mousedown', se, hr.left + 3, hr.top + 3);
        mouse('mousemove', btn, hr.left + 3 + 30, hr.top + 3 + 2, { altKey: true, shiftKey: true });
        mouse('mouseup', btn, hr.left + 3 + 30, hr.top + 3 + 2);
        var e2b = edits()[n2b];
        R.shiftResize = e2b ? JSON.stringify(e2b.style) + ' from ' + w1 + 'x' + h1 : null;
        R.shiftResizeOk = !!e2b && parseFloat(e2b.style.width) > w1 + 20 && parseFloat(e2b.style.height) > h1 + 2 &&
          Math.abs(parseFloat(e2b.style.width) / parseFloat(e2b.style.height) - w1 / h1) < 0.15;
        btn.style.width = w1 + 'px'; btn.style.height = h1 + 'px';   // (the page as the next tests expect it)
      }
      /* the positioning wrapper of an input */
      var inp = document.getElementById('XST1');
      if (inp) {
        var ir = inp.getBoundingClientRect();
        var n3 = edits().length;
        mouse('mousedown', inp, ir.left + 5, ir.top + 5);
        mouse('mousemove', inp, ir.left + 5 + 7, ir.top + 5, ALT);
        mouse('mouseup', inp, ir.left + 5 + 7, ir.top + 5);
        var e3 = edits()[n3];
        R.wrapperEdit = e3 ? e3.target + ' ' + JSON.stringify(e3.style) : null;
        R.wrapperOk = !!e3 && e3.id === 'XST1' && e3.target === 'parent' && /px$/.test(e3.style.left);
      }
      /* multi-select (Ctrl+click), group drag = ONE batch edit, align = ONE batch edit */
      var ex0 = document.getElementById('sbtExit');
      if (ex0) {
        window.__htdProbe.selectById('spbSave');
        var er = ex0.getBoundingClientRect();
        mouse('mousedown', ex0, er.left + 5, er.top + 5, { ctrlKey: true });
        mouse('mouseup', ex0, er.left + 5, er.top + 5, { ctrlKey: true });
        mouse('click', ex0, er.left + 5, er.top + 5, { ctrlKey: true });
        var sm = msgs('select').pop();
        R.multiSel = sm && sm.info ? sm.info.id + '+' + (sm.info.multi || []).join(',') : null;
        R.multiOk = !!sm && sm.info.id === 'sbtExit' && (sm.info.multi || []).indexOf('spbSave') >= 0;
        var bl0 = btn.style.left, xl0 = ex0.style.left;
        var nb = edits().length;
        mouse('mousedown', ex0, er.left + 5, er.top + 5);
        mouse('mousemove', ex0, er.left + 5 + 4, er.top + 5, ALT);
        mouse('mousemove', ex0, er.left + 5 + 8, er.top + 5, ALT);
        mouse('mouseup', ex0, er.left + 5 + 8, er.top + 5);
        var eb = edits()[nb];
        R.groupEdit = eb ? eb.what + ' ' + (eb.edits || []).map(function (x) { return x.id + JSON.stringify(x.style); }).join(' ') : null;
        R.groupOk = !!eb && eb.what === 'batch' && eb.edits.length === 2 &&
          btn.style.left === (parseFloat(bl0) + 8) + 'px' && ex0.style.left === (parseFloat(xl0) + 8) + 'px';
        var na = edits().length;
        window.__htdProbe.message({ type: 'align', how: 'top' });   // primary = sbtExit, other = spbSave
        var ea = edits()[na];
        var aTopDiff = Math.abs(btn.getBoundingClientRect().top - ex0.getBoundingClientRect().top);
        R.alignEdit = ea ? ea.what + ' ' + ea.id + JSON.stringify(ea.style) + ' topDiff=' + aTopDiff.toFixed(1) : null;
        R.alignOk = !!ea && ea.id === 'spbSave' && aTopDiff < 0.5;
        /* WinForms / WPF: Ctrl+drag = a copy dropped there -- the originals go back where they were, ONE copyDrop
           with both names and the move (no edit); then Ctrl+click (no move) on a selected one takes it out, as before */
        var bl1 = btn.style.left, xl1 = ex0.style.left, bt1 = btn.style.top, nc = edits().length;
        var er2 = ex0.getBoundingClientRect();
        mouse('mousedown', ex0, er2.left + 5, er2.top + 5, { ctrlKey: true });
        mouse('mousemove', ex0, er2.left + 5 + 6, er2.top + 5, { ctrlKey: true, altKey: true });
        R.copyMoving = ex0.style.left !== xl1;
        mouse('mousemove', ex0, er2.left + 5 + 20, er2.top + 5 + 10, { ctrlKey: true, altKey: true });
        mouse('mouseup', ex0, er2.left + 5 + 20, er2.top + 5 + 10, { ctrlKey: true });
        mouse('click', ex0, er2.left + 5 + 20, er2.top + 5 + 10, { ctrlKey: true });
        var cdm = msgs('copyDrop').pop();
        R.copyDrop = cdm ? JSON.stringify(cdm) : null;
        var sm1 = msgs('select').pop();
        R.copyDropOk = !!cdm && cdm.ids.indexOf('spbSave') >= 0 && cdm.ids.indexOf('sbtExit') >= 0 && cdm.dx === 20 && cdm.dy === 10 &&
          edits().length === nc && btn.style.left === bl1 && btn.style.top === bt1 && ex0.style.left === xl1 && R.copyMoving &&
          !!sm1 && sm1.info.id === 'sbtExit' && (sm1.info.multi || []).indexOf('spbSave') >= 0;
        mouse('mousedown', ex0, er2.left + 5, er2.top + 5, { ctrlKey: true });
        mouse('mouseup', ex0, er2.left + 5, er2.top + 5, { ctrlKey: true });
        mouse('click', ex0, er2.left + 5, er2.top + 5, { ctrlKey: true });
        var sm2 = msgs('select').pop();
        R.ctrlClickOff = !!sm2 && sm2.info.id === 'spbSave' && (sm2.info.multi || []).indexOf('sbtExit') < 0 && msgs('copyDrop').length === 1;
        /* Blend: dragged out of its panel and let go with Alt held = into the container under the pointer -- the
           button goes back where it was (the extension moves it in the source), the targets under the pointer and
           where its top-left is from the pointer are sent, nothing written here */
        window.__htdProbe.selectById('spbSave');
        var rpP = btn.offsetParent, rpF = document.querySelector('body > .form');
        if (rpP && rpF && rpP !== rpF) {
          var rpB = btn.getBoundingClientRect();
          /* a small box of its own on the form, on top of everything: the place it is let go */
          var rpBox = document.createElement('div');
          rpBox.id = 'htdRpBox';
          rpBox.style.cssText = 'position:absolute;left:4px;top:4px;width:40px;height:30px;z-index:99999;';
          rpF.appendChild(rpBox);
          var rpBr = rpBox.getBoundingClientRect();
          var rpX = Math.round(rpBr.left + 20), rpY = Math.round(rpBr.top + 15);
          var rpL0 = btn.style.left, rpT0 = btn.style.top, nRp = edits().length;
          mouse('mousedown', btn, rpB.left + 5, rpB.top + 5);
          mouse('mousemove', btn, rpB.left + 9, rpB.top + 9, { altKey: true });
          mouse('mousemove', btn, rpX, rpY, { altKey: true });
          mouse('mouseup', btn, rpX, rpY, { altKey: true });
          var rpm = msgs('reparentDrop').pop();
          R.reparentDrop = rpm ? JSON.stringify(rpm).slice(0, 240) : null;
          R.reparentDropOk = !!rpm && rpm.offs.length === 1 && rpm.offs[0].id === 'spbSave' && Math.abs(rpm.offs[0].dx + 5) <= 1 && Math.abs(rpm.offs[0].dy + 5) <= 1 &&
            ((rpm.targets.length === 2 && rpm.targets[0].id === 'htdRpBox' && Math.abs(rpm.targets[0].x - 20) <= 1 && Math.abs(rpm.targets[0].y - 15) <= 1) ||
              (rpm.targets.length === 1)) &&   // (a page that stacks its own layers over the box: the form under the pointer)
            rpm.targets[rpm.targets.length - 1].id === '@form' && btn.style.left === rpL0 && btn.style.top === rpT0 && edits().length === nRp;
          rpF.removeChild(rpBox);
        } else R.reparentDropOk = 'no panel';
        /* without Alt the same drag is a plain move (no reparentDrop) */
        R.reparentPlain = msgs('reparentDrop').length;
      }
      /* right-click: selects, the page does not see it, VS Code's menu is not blocked */
      var exr = document.getElementById('sbtExit');
      if (exr) {
        window.__htdProbe.selectById('spbSave');
        var pageSaw = 0;
        var seen = function () { pageSaw++; };
        exr.addEventListener('contextmenu', seen);
        var rr0 = exr.getBoundingClientRect();
        var cmEv = new MouseEvent('contextmenu', { bubbles: true, cancelable: true, composed: true, button: 2, clientX: rr0.left + 4, clientY: rr0.top + 4 });
        exr.dispatchEvent(cmEv);
        var sc = msgs('select').pop();
        R.ctxSelects = !!sc && sc.info.id === 'sbtExit';
        R.ctxNotBlocked = !cmEv.defaultPrevented && pageSaw === 0;
        R.ctxAttr = /htdDesign/.test(document.documentElement.getAttribute('data-vscode-context') || '');
        exr.removeEventListener('contextmenu', seen);
        window.__htdProbe.message({ type: 'selectParent' });
        var spar = msgs('select').pop();
        R.selectParent = spar && spar.info ? spar.info.id : null;
      }
      /* snaplines: drag the Save button up so its top comes within 5px of lines of the
         Exit button / the panel -- it must land ON a line, and the guide is drawn */
      var ex = document.getElementById('sbtExit');
      if (ex) {
        window.__htdProbe.selectById('spbSave');     // a single selection again
        btn.style.top = '30px';                     // away from every line first (DOM only)
        window.__htdProbe.drawNow();
        var br = btn.getBoundingClientRect(), xr = ex.getBoundingClientRect();
        var want = xr.top + 2 - br.top;                 // 2px below the Exit button's top
        var sx0 = br.left + 5, sy0 = br.top + 5;
        var ns = edits().length;
        mouse('mousedown', btn, sx0, sy0);
        mouse('mousemove', btn, sx0, sy0 + want / 2);
        mouse('mousemove', btn, sx0, sy0 + want);
        window.__htdProbe.drawNow();
        var gy = host && host.shadowRoot ? host.shadowRoot.querySelector('.gy') : null;
        R.snapGuide = !!gy && gy.style.display === 'block';
        mouse('mouseup', btn, sx0, sy0 + want);
        var es = edits()[ns];
        var topNow = btn.getBoundingClientRect().top;
        R.snapEdit = es ? JSON.stringify(es.style) + ' offBy=' + (topNow - (br.top + want)).toFixed(1) : null;
        R.snapOk = !!es && Math.abs(topNow - (br.top + want)) >= 0.5 && Math.abs(topNow - (br.top + want)) <= 5;
      }
      /* WPF: the text baselines are snaplines too -- in a box of their own, a small text put 2 px below a big text's
         baseline and dragged sideways lands with its baseline on it (the dashed guide), not on an edge line */
      (function () {
        var fr0 = document.querySelector('body > .form') || document.body;
        var bx0 = document.createElement('div');
        bx0.id = 'htdBlBox';
        bx0.style.cssText = 'position:absolute;left:10px;top:10px;width:420px;height:300px;';
        var mk = function (id, fs, left, top) {
          var s = document.createElement('span');
          s.id = id; s.textContent = 'Hg';
          s.style.cssText = 'position:absolute;left:' + left + 'px;top:' + top + 'px;font-size:' + fs + 'px;line-height:normal;white-space:nowrap;font-family:Arial;';
          bx0.appendChild(s);
          return s;
        };
        fr0.appendChild(bx0);
        var bA = mk('htdBlA', 40, 20, 20), bB = mk('htdBlB', 12, 300, 0);
        var blOf = function (el) {
          var rg = document.createRange(); rg.selectNodeContents(el.firstChild);
          var rr = rg.getClientRects()[0];
          var cv = document.createElement('canvas').getContext('2d');
          var cs = getComputedStyle(el);
          cv.font = [cs.fontStyle, cs.fontWeight, cs.fontSize, cs.fontFamily].join(' ');
          var mm = cv.measureText('Hg');
          return rr.top + rr.height * mm.fontBoundingBoxAscent / (mm.fontBoundingBoxAscent + mm.fontBoundingBoxDescent);
        };
        var yA = blOf(bA), offB = blOf(bB) - bB.getBoundingClientRect().top;
        var top0 = Math.round(yA + 2 - offB - bx0.getBoundingClientRect().top);
        bB.style.top = top0 + 'px';
        window.__htdProbe.selectById('htdBlB');
        var rb = bB.getBoundingClientRect(), nb = edits().length;
        mouse('mousedown', bB, rb.left + 3, rb.top + 3);
        mouse('mousemove', bB, rb.left + 5, rb.top + 3);
        mouse('mousemove', bB, rb.left + 8, rb.top + 3);
        window.__htdProbe.drawNow();
        var gy2 = host && host.shadowRoot ? host.shadowRoot.querySelector('.gy') : null;
        var dashed = !!gy2 && gy2.style.display === 'block' && gy2.classList.contains('bl');
        mouse('mouseup', bB, rb.left + 8, rb.top + 3);
        var eb2 = edits()[nb];
        var off = blOf(bB) - blOf(bA);   // (both now: selecting may have scrolled the page)
        R.baselineSnap = 'top ' + top0 + ' -> ' + bB.style.top + ', baseline off by ' + off.toFixed(2) + ', dashed ' + dashed;
        R.baselineSnapOk = !!eb2 && Math.abs(off) < 1 && dashed && parseFloat(bB.style.top) < top0;
        fr0.removeChild(bx0);
        window.__htdProbe.selectById('spbSave');
      })();
      /* keyboard: Esc = parent, Tab = next, arrows = nudge (one edit per burst) */
      window.__htdProbe.selectById('spbSave');
      var key = function (k, extra) {
        var o = { key: k, bubbles: true, cancelable: true };
        for (var q in extra || {}) o[q] = extra[q];
        window.dispatchEvent(new KeyboardEvent('keydown', o));
        var s = msgs('select').pop();
        return s && s.info ? s.info.id : null;
      };
      R.escParent = key('Escape');
      window.__htdProbe.selectById('spbSave');
      R.tabNext = key('Tab');
      window.__htdProbe.selectById('spbSave');
      var n4 = edits().length;
      var lBefore = parseFloat(btn.style.left);
      key('ArrowRight'); key('ArrowRight'); key('ArrowRight', { shiftKey: true });
      R.nudgeDom = parseFloat(btn.style.left) - lBefore;
      setTimeout(function () {
        /* the 8 resize handles are drawn around the selection (next animation frame) */
        window.__htdProbe.drawNow();
        var hs = host && host.shadowRoot ? host.shadowRoot.querySelectorAll('.hd') : [];
        R.handlesShown = Array.prototype.filter.call(hs, function (h) { return h.style.display === 'block'; }).length;
        var e4 = edits().slice(n4);
        R.nudgeEdits = e4.length;
        R.nudgeOk = e4.length === 1 && e4[0].style.left === (lBefore + 12) + 'px';
        /* from the properties panel: caption */
        var sel = msgs('select').pop();
        window.postMessage({ __htd: 1, type: 'setCaption', key: sel.info.key, value: 'Save2' }, '*');
        setTimeout(function () {
          var e5 = edits().pop();
          R.captionEdit = e5 ? e5.what + ' ' + e5.capKind + ' ' + e5.value : null;
          R.captionOk = !!e5 && e5.what === 'caption' && e5.capKind === 'text' && e5.value === 'Save2' && /Save2/.test(btn.textContent);
          var s2 = msgs('select').pop();
          R.selLayout = s2 && s2.info && s2.info.layout ? s2.info.layout.target + ' ' + s2.info.layout.width + 'x' + s2.info.layout.height : null;
          R.selCaption = s2 && s2.info && s2.info.caption ? s2.info.caption.kind + ':' + s2.info.caption.value : null;
          /* WPF: F2 = the control's text edited right on it -- a box with its text; Enter = the same caption edit as the
             panel's; while it is open the design keys leave it alone and a press in it is not stopped (the caret);
             Esc = nothing written */
          window.__htdProbe.selectById(btn.id);
          var nT = edits().length;
          window.__htdProbe.message({ type: 'editText' });
          var tb1 = window.__htdProbe.textBox();
          R.textBoxOpen = !!tb1 && tb1.value === 'Save2' && msgs('textEdit').some(function (m) { return m.open === true; });
          R.textBoxKeysAlone = false; R.textBoxClick = false;
          if (tb1) {
            var lBefore2 = btn.style.left;
            window.dispatchEvent(new KeyboardEvent('keydown', { key: 'ArrowLeft', bubbles: true, cancelable: true }));
            R.textBoxKeysAlone = btn.style.left === lBefore2 && edits().length === nT;
            var md2 = new MouseEvent('mousedown', { bubbles: true, cancelable: true, composed: true, button: 0 });
            tb1.dispatchEvent(md2);
            R.textBoxClick = !md2.defaultPrevented && !!window.__htdProbe.textBox();
            tb1.value = 'Save3';
            tb1.dispatchEvent(new KeyboardEvent('keydown', { key: 'Enter', bubbles: true, cancelable: true, composed: true }));
          }
          var e6 = edits()[nT];
          R.textBoxEdit = e6 ? e6.what + ' ' + e6.capKind + ' ' + e6.value : null;
          var te6 = msgs('textEdit').pop();
          R.textBoxOk = R.textBoxOpen && R.textBoxKeysAlone && R.textBoxClick && !!e6 && e6.what === 'caption' && e6.value === 'Save3' &&
            /Save3/.test(btn.textContent) && !window.__htdProbe.textBox() && !!te6 && te6.open === false;
          var nT2 = edits().length;
          window.__htdProbe.message({ type: 'editText' });
          var tb2 = window.__htdProbe.textBox();
          var escEv = new KeyboardEvent('keydown', { key: 'Escape', bubbles: true, cancelable: true, composed: true });
          if (tb2) { tb2.value = 'zzz'; tb2.dispatchEvent(escEv); }
          R.textBoxEscPrevented = escEv.defaultPrevented + ' same=' + (tb2 === window.__htdProbe.textBox()) + ' connected=' + (tb2 ? tb2.isConnected : '-');
          R.textBoxEsc = 'box=' + !!tb2 + ' edits=' + (edits().length - nT2) + ' text=' + btn.textContent.trim().slice(0, 20) + ' open=' + !!window.__htdProbe.textBox();
          R.textBoxEscOk = !!tb2 && edits().length === nT2 && /Save3/.test(btn.textContent) && !window.__htdProbe.textBox();
          window.__htdProbe.message({ type: 'setCaption', key: msgs('select').pop().info.key, value: 'Save2' });   // (as the next tests expect)
          /* anchored on both sides (left+right, top+bottom, no width/height): moving keeps
             the size (both edges move), resizing moves only the far edge */
          var gb2 = document.getElementById('GroupBox2');
          if (gb2 && gb2.style.right !== '' && gb2.style.width === '') {
            window.__htdProbe.selectById('GroupBox2');
            var w2 = gb2.offsetWidth, l2 = parseFloat(gb2.style.left), r2 = parseFloat(gb2.style.right), b2 = parseFloat(gb2.style.bottom);
            var ne = edits().length;
            key('ArrowRight');
            key('ArrowDown');
            window.__htdProbe.message({ type: 'noop' });
            R._anch = { ne: ne, w2: w2, l2: l2, r2: r2, b2: b2 };
            R.anchMoveDom = gb2.style.left + ' ' + gb2.style.right + ' ' + gb2.style.bottom + ' w=' + gb2.offsetWidth;
            R.anchMoveOk = gb2.style.left === (l2 + 1) + 'px' && gb2.style.right === (r2 - 1) + 'px' && gb2.style.bottom === (b2 - 1) + 'px' && gb2.offsetWidth === w2;
            key('ArrowLeft', { ctrlKey: true });        // 1px narrower: only the right edge moves
            R.anchResizeDom = gb2.style.left + ' ' + gb2.style.right + ' w=' + gb2.offsetWidth;
            R.anchResizeOk = gb2.style.left === (l2 + 1) + 'px' && gb2.style.right === (r2 - 1 + 1) + 'px' && gb2.offsetWidth === w2 - 1 && gb2.style.width === '';
          }

          /* the form itself: Ctrl+arrow resizes it (like a WPF Window), plain arrows do not move it */
          var frm = document.querySelector('body > .form');
          if (frm && frm.style.width) {
            window.__htdProbe.selectById('@form');
            var fw0 = parseFloat(frm.style.width);
            var nf = edits().length;
            key('ArrowRight');                               // no move for the form
            key('ArrowRight', { ctrlKey: true });            // +1 width
            window.__htdProbe.message({ type: 'noop' });
            R._formAt = nf;
            R.formResizeDom = frm.style.width + ' left=' + (frm.style.left || '-');
            R.formResizeOk = frm.style.width === (fw0 + 1) + 'px' && !frm.style.left;
            var sf = msgs('select').pop();
            R.formLayoutRoot = !!(sf && sf.info && sf.info.layout && sf.info.layout.root);
            /* fonts from the properties panel */
            window.__htdProbe.selectById('spbSave');
            var sk2 = msgs('select').pop().info.key;
            var nfb = edits().length;
            window.__htdProbe.message({ type: 'setLook', key: sk2, prop: 'bold', value: true });
            window.__htdProbe.message({ type: 'setLook', key: sk2, prop: 'fontName', value: 'Microsoft JhengHei' });
            var fb = edits().slice(nfb);
            R.fontEdits = fb.map(function (x) { return JSON.stringify(x.style); }).join(' ');
            R.fontOk = fb.length === 2 && fb[0].style['font-weight'] === 'bold' && fb[1].style['font-family'] === "'Microsoft JhengHei'" &&
              getComputedStyle(btn).fontWeight >= 600;
            var sl = msgs('select').pop();
            R.lookBold = !!(sl && sl.info && sl.info.look && sl.info.look.bold);
          }

          /* from the properties panel / the DFM difference list: setLayout by key, and by id
             for a control that is not drawn (measured by its style then, not as 0) */
          var ex2 = document.getElementById('sbtExit');
          if (ex2 && /px$/.test(ex2.style.left)) {
            window.__htdProbe.selectById('spbSave');
            var sk3 = msgs('select').pop().info.key;
            var nl = edits().length;
            window.__htdProbe.message({ type: 'setLayout', key: sk3, left: 61 });
            var el1 = edits()[nl];
            R.setLayoutKey = el1 ? JSON.stringify(el1.style) : 'no edit';
            R.setLayoutKeyOk = !!el1 && el1.style.left === '61px' && btn.style.left === '61px';
            var xl = parseFloat(ex2.style.left);
            var disp = ex2.style.display;
            ex2.style.display = 'none';                     /* DOM only: as if in a hidden panel */
            window.__htdProbe.message({ type: 'lookAll', seq: 7, ids: ['sbtExit', 'spbSave'] });
            var la = msgs('lookAll').pop();
            var lx = la && la.items.filter(function (x) { return x.id === 'sbtExit'; })[0];
            R.lookAllHidden = lx ? JSON.stringify({ lay: lx.lay && { left: lx.lay.left, rendered: lx.lay.rendered }, visible: lx.look.visible }) : null;
            R.lookAllOk = !!la && la.seq === 7 && la.items.length === 2 && !!lx && lx.lay && lx.lay.rendered === false && lx.lay.left === xl &&
              lx.look.visible === false && Array.isArray(lx.look.fontFamilies);
            var nh = edits().length;
            window.__htdProbe.message({ type: 'setLayout', id: 'sbtExit', left: xl + 5 });
            var eh = edits()[nh];
            R.setLayoutHidden = eh ? eh.id + JSON.stringify(eh.style) : 'no edit';
            R.setLayoutHiddenOk = !!eh && eh.id === 'sbtExit' && eh.style.left === (xl + 5) + 'px';
            ex2.style.display = disp;
          }

          /* hidden in the designer only: invisible, a click at its place reaches what is
             underneath; showing it again restores it (nothing written: no 'edit') */
          var hpnl = btn.parentElement && btn.parentElement.closest ? btn.parentElement.closest('[id]') : null;
          if (hpnl && hpnl !== document.body && hpnl.contains(btn)) {
            var ne2 = edits().length;
            window.__htdProbe.message({ type: 'designHidden', ids: [hpnl.id] });
            var br2 = btn.getBoundingClientRect();
            var hit = document.elementFromPoint(br2.left + br2.width / 2, br2.top + br2.height / 2);
            R.designHideTarget = hpnl.id + ' hit=' + (hit ? (hit.id || hit.tagName) : '-') + ' vis=' + getComputedStyle(btn).visibility;
            R.designHideOk = hpnl.getAttribute('data-htd-hide') === '1' && getComputedStyle(btn).visibility === 'hidden' && !!hit && !hpnl.contains(hit);
            window.__htdProbe.message({ type: 'designHidden', ids: [] });
            R.designShowOk = !hpnl.hasAttribute('data-htd-hide') && getComputedStyle(btn).visibility === 'visible' && edits().length === ne2;
          }

          /* "全部改回" of a pattern: one look change on several controls = ONE batch edit */
          var other2 = Array.prototype.filter.call(document.querySelectorAll('button[id]'), function (b) { return b !== btn; })[0];
          if (other2) {
            var nM = edits().length;
            window.__htdProbe.message({ type: 'editMany', items: [
              { id: btn.id, type: 'setLook', prop: 'italic', value: true },
              { id: other2.id, type: 'setLook', prop: 'italic', value: true },
              { id: 'no-such-id', type: 'setLook', prop: 'italic', value: true },
            ] });
            var eM = edits().slice(nM);
            R.editMany = eM.map(function (x) { return x.what + ':' + (x.edits || []).map(function (y) { return y.id; }).join('+'); }).join(' ');
            R.editManyOk = eM.length === 1 && eM[0].what === 'batch' && eM[0].edits.length === 2 &&
              getComputedStyle(btn).fontStyle === 'italic' && getComputedStyle(other2).fontStyle === 'italic';

            /* 改回 DFM sends position / size too: in the same ONE batch; a locked one keeps its place (said) */
            var lL0 = btn.style.left, lW0 = btn.style.width;
            var nR = edits().length, rR = msgs('editRefused').length;
            window.__htdProbe.message({ type: 'editMany', items: [
              { id: btn.id, type: 'setLayout', left: btn.offsetLeft + 5, width: btn.offsetWidth + 4 },
              { id: other2.id, type: 'setLook', prop: 'bold', value: true },
            ] });
            var eR = edits().slice(nR);
            var layOk = eR.length === 1 && eR[0].what === 'batch' && eR[0].edits.length === 2 && btn.style.left === (parseFloat(lL0) + 5) + 'px' && btn.style.width !== lW0;
            window.__htdProbe.message({ type: 'designLocked', ids: [btn.id] });
            var nR2 = edits().length;
            window.__htdProbe.message({ type: 'editMany', items: [
              { id: btn.id, type: 'setLayout', left: 3 },
              { id: other2.id, type: 'setLook', prop: 'bold', value: false },
            ] });
            var eR2 = edits().slice(nR2);
            window.__htdProbe.message({ type: 'designLocked', ids: [] });
            R.editManyLayout = eR.map(function (x) { return x.what + (x.edits || []).length; }).join(',') + ' | locked: ' + eR2.map(function (x) { return x.what + ':' + x.id; }).join(',') +
              ' refused ' + (msgs('editRefused').length - rR);
            R.editManyLayoutOk = layOk && eR2.length === 1 && eR2[0].id === other2.id && btn.style.left === (parseFloat(lL0) + 5) + 'px' && msgs('editRefused').length === rR + 1;
            btn.style.left = lL0; btn.style.width = lW0;
            /* force (改回 DFM): the page already shows the value asked for, the source may not -- the edit is still made */
            var nF = edits().length;
            window.__htdProbe.message({ type: 'editMany', items: [{ id: btn.id, type: 'setLayout', left: parseFloat(lL0), force: true }] });
            var eF = edits().slice(nF);
            var nF2 = edits().length;
            window.__htdProbe.message({ type: 'editMany', items: [{ id: btn.id, type: 'setLayout', left: parseFloat(lL0) }] });
            R.editManyForceOk = eF.length === 1 && eF[0].style && eF[0].style.left === lL0 && edits().length === nF2 && btn.style.left === lL0;
          }

          /* 水平／垂直等距 (the outer two stay, the gaps come out equal, the primary moves too) and
             容器中置中 (one or a block of several); locked / too few = nothing. Put back afterwards. */
          (function () {
            var cands = Array.prototype.filter.call(document.querySelectorAll('[id]'), function (x) {
              return x.style && x.style.position === 'absolute' && /px$/.test(x.style.left) && /px$/.test(x.style.top) &&
                x.getClientRects().length && x.offsetWidth > 0 && x.offsetParent && x.offsetParent !== document.body;
            });
            var groups = new Map();
            cands.forEach(function (x) { var p = x.offsetParent; if (!groups.has(p)) groups.set(p, []); groups.get(p).push(x); });
            function pos(x, hz) { return hz ? x.offsetLeft : x.offsetTop; }
            function gapOf(a, b, hz) { return hz ? b.offsetLeft - a.offsetLeft - a.offsetWidth : b.offsetTop - a.offsetTop - a.offsetHeight; }
            function trio(hz) {
              var found = null;
              groups.forEach(function (arr) {
                if (found || arr.length < 3) return;
                var s = arr.slice().sort(function (a, b) { return pos(a, hz) - pos(b, hz); });
                for (var i = 0; i + 2 < s.length && !found; i++) {
                  if (pos(s[i + 1], hz) - pos(s[i], hz) < 5 || pos(s[i + 2], hz) - pos(s[i + 1], hz) < 5) continue;
                  if (Math.abs(gapOf(s[i], s[i + 1], hz) - gapOf(s[i + 1], s[i + 2], hz)) > 3) found = [s[i], s[i + 1], s[i + 2]];
                }
              });
              return found;
            }
            var saved = cands.map(function (x) { return [x, x.style.left, x.style.top]; });
            function spaced(hz) {
              var t = trio(hz);
              if (!t) return null;
              var o0 = t[0].style.cssText, o2 = t[2].style.cssText;
              var n0 = edits().length;
              window.__htdProbe.message({ type: 'selectIds', ids: [t[1].id, t[0].id, t[2].id] });   /* the middle one is primary */
              window.__htdProbe.message({ type: 'align', how: hz ? 'hspace' : 'vspace' });
              /* (an arrow nudge still pending from an earlier check is sent first -- not counted) */
              var ids = t.map(function (x) { return x.id; }), e = [];
              edits().slice(n0).forEach(function (x) { (x.what === 'batch' ? x.edits : [x]).forEach(function (y) { if (ids.indexOf(y.id) >= 0) e.push(y); }); });
              var g1 = gapOf(t[0], t[1], hz), g2 = gapOf(t[1], t[2], hz);
              return { what: ids.join(',') + ' gaps ' + g1 + '/' + g2 + ' edits ' + e.length,
                ok: e.length === 1 && e[0].id === t[1].id && Math.abs(g1 - g2) <= 1 && t[0].style.cssText === o0 && t[2].style.cssText === o2,
                t: t };
            }
            var hs = spaced(true), vs = spaced(false);
            R.spaceTried = (hs ? 'H ' + hs.what : 'H none') + ' | ' + (vs ? 'V ' + vs.what : 'V none');
            R.spaceOk = !!(hs || vs) && (!hs || hs.ok) && (!vs || vs.ok);
            var t3 = (hs || vs) && (hs || vs).t;
            if (t3) {
              /* too few / locked: no edit, said once each */
              var nr = msgs('editRefused').length, ne = edits().length;
              window.__htdProbe.message({ type: 'selectIds', ids: [t3[0].id, t3[1].id] });
              window.__htdProbe.message({ type: 'align', how: 'hspace' });
              window.__htdProbe.message({ type: 'designLocked', ids: [t3[1].id] });
              window.__htdProbe.message({ type: 'selectIds', ids: [t3[0].id, t3[1].id, t3[2].id] });
              window.__htdProbe.message({ type: 'align', how: 'vcenterIn' });
              window.__htdProbe.message({ type: 'designLocked', ids: [] });
              R.spaceRefusedOk = edits().length === ne && msgs('editRefused').length === nr + 2;
              /* one in its container, then a block of two: centred, the two keep their distance */
              var a = t3[0], b = t3[1], P = a.offsetParent;
              window.__htdProbe.message({ type: 'selectIds', ids: [a.id] });
              window.__htdProbe.message({ type: 'align', how: 'hcenterIn' });
              var cx = a.offsetLeft + a.offsetWidth / 2 - P.clientWidth / 2;
              var dy0 = b.offsetTop - a.offsetTop, dx0 = b.offsetLeft - a.offsetLeft, nc = edits().length;
              window.__htdProbe.message({ type: 'selectIds', ids: [b.id, a.id] });
              window.__htdProbe.message({ type: 'align', how: 'vcenterIn' });
              var top = Math.min(a.offsetTop, b.offsetTop), bot = Math.max(a.offsetTop + a.offsetHeight, b.offsetTop + b.offsetHeight);
              var cy = (top + bot) / 2 - P.clientHeight / 2;
              var ec = edits().slice(nc);
              R.centerTried = a.id + ' dx ' + cx.toFixed(1) + ' | ' + a.id + '+' + b.id + ' dy ' + cy.toFixed(1) + ' edits ' + ec.map(function (x) { return x.what; }).join(',');
              R.centerOk = Math.abs(cx) <= 1 && Math.abs(cy) <= 1 && b.offsetTop - a.offsetTop === dy0 && b.offsetLeft - a.offsetLeft === dx0 &&
                ec.length === 1 && (ec[0].what === 'batch' ? ec[0].edits.length === 2 : true);
            }
            saved.forEach(function (s) { s[0].style.left = s[1]; s[0].style.top = s[2]; });
          })();

          /* a resize handle with 2 selected: both get the same +12 / +6 (WinForms / WPF), ONE batch edit */
          if (other2) {
            var gs0 = [btn.style.width, btn.style.height, other2.style.width, other2.style.height];
            var gw0 = [btn.offsetWidth, btn.offsetHeight, other2.offsetWidth, other2.offsetHeight];
            window.__htdProbe.message({ type: 'selectIds', ids: [btn.id, other2.id] });
            window.__htdProbe.drawNow();
            var gse = host && host.shadowRoot ? host.shadowRoot.querySelector('.hd[data-dir="se"]') : null;
            var ngr = edits().length;
            if (gse) {
              var ghr = gse.getBoundingClientRect(), gzz = btn.getBoundingClientRect().width / btn.offsetWidth;
              mouse('mousedown', gse, ghr.left + 3, ghr.top + 3);
              mouse('mousemove', btn, ghr.left + 3 + 12 * gzz, ghr.top + 3 + 6 * gzz, ALT);
              mouse('mouseup', btn, ghr.left + 3 + 12 * gzz, ghr.top + 3 + 6 * gzz);
            }
            var egr = edits().slice(ngr);
            R.groupResize = egr.map(function (x) { return x.what + (x.edits ? x.edits.length : ''); }).join(',') + ' sizes ' +
              [btn.offsetWidth - gw0[0], btn.offsetHeight - gw0[1], other2.offsetWidth - gw0[2], other2.offsetHeight - gw0[3]].join('/');
            R.groupResizeOk = !!gse && egr.length === 1 && egr[0].what === 'batch' && egr[0].edits.length === 2 &&
              btn.offsetWidth - gw0[0] === 12 && btn.offsetHeight - gw0[1] === 6 && other2.offsetWidth - gw0[2] === 12 && other2.offsetHeight - gw0[3] === 6;
            btn.style.width = gs0[0]; btn.style.height = gs0[1]; other2.style.width = gs0[2]; other2.style.height = gs0[3];
          }

          /* the properties panel with 2 selected (all: true): bold / width on both = ONE batch edit
             each; a width with one locked = nothing; without all = the primary only */
          if (other2) {
            var w0b = btn.style.width, w0o = other2.style.width;
            window.__htdProbe.message({ type: 'selectIds', ids: [btn.id, other2.id] });
            var ak = msgs('select').pop().info.key;
            var na0 = edits().length;
            window.__htdProbe.message({ type: 'setLook', key: ak, prop: 'bold', value: false, all: true });
            var ea1 = edits().slice(na0);
            window.__htdProbe.message({ type: 'setLayout', key: ak, width: 120, all: true });
            var ea2 = edits().slice(na0 + ea1.length);
            window.__htdProbe.message({ type: 'designLocked', ids: [other2.id] });
            var nr1 = msgs('editRefused').length, na1 = edits().length;
            window.__htdProbe.message({ type: 'setLayout', key: ak, width: 130, all: true });
            var lockedNone = edits().length === na1 && msgs('editRefused').length === nr1 + 1 && btn.offsetWidth === 120;
            window.__htdProbe.message({ type: 'designLocked', ids: [] });
            var na2 = edits().length;
            window.__htdProbe.message({ type: 'setLook', key: ak, prop: 'italic', value: false });
            var ea3 = edits().slice(na2);
            var stillMulti = (msgs('select').pop().info.multi || []).indexOf(other2.id) >= 0;
            R.allEdit = [ea1, ea2, ea3].map(function (a) { return a.map(function (x) { return x.what + (x.edits ? x.edits.length : ':' + x.id); }).join(','); }).join(' | ') +
              ' w ' + btn.offsetWidth + '/' + other2.offsetWidth + ' locked ' + lockedNone + ' multi ' + stillMulti;
            R.allOk = ea1.length === 1 && ea1[0].what === 'batch' && ea1[0].edits.length === 2 &&
              getComputedStyle(btn).fontWeight === '400' && getComputedStyle(other2).fontWeight === '400' &&
              ea2.length === 1 && ea2[0].what === 'batch' && ea2[0].edits.length === 2 && btn.offsetWidth === 120 && other2.offsetWidth === 120 &&
              lockedNone && ea3.length === 1 && ea3[0].id === btn.id && stillMulti;
            btn.style.width = w0b; other2.style.width = w0o;
          }

          /* DFM 位置: a dashed frame 30px right of the button (in its own px), the selected one
             labelled; a drag of 28px snaps onto it (left + 30 exactly); off = gone */
          (function () {
            window.__htdProbe.selectById(btn.id);
            var gl0 = btn.style.left, gz = btn.getBoundingClientRect().width / btn.offsetWidth;
            window.__htdProbe.message({ type: 'dfmGhosts', items: [{ id: btn.id, left: parseFloat(gl0) + 30 }, { id: 'no-such-id', left: 1 }] });
            window.__htdProbe.drawNow();
            var gbx = host && host.shadowRoot ? host.shadowRoot.querySelector('.gh.sel') : null;
            var gtg = host && host.shadowRoot ? host.shadowRoot.querySelector('.ght') : null;
            var br = btn.getBoundingClientRect();
            var gdx = gbx ? parseFloat(gbx.style.left) - br.left : null;
            var shownOk = !!gbx && gbx.style.display === 'block' && Math.abs(gdx - 30 * gz) < 1 && Math.abs(parseFloat(gbx.style.width) - br.width) < 1 &&
              !!gtg && gtg.style.display === 'block' && /DFM/.test(gtg.textContent) && gtg.textContent.indexOf('left ' + (parseFloat(gl0) + 30)) >= 0;
            var ng = edits().length;
            mouse('mousedown', btn, br.left + 5, br.top + 5);
            mouse('mousemove', btn, br.left + 5 + 14 * gz, br.top + 5);
            mouse('mousemove', btn, br.left + 5 + 28 * gz, br.top + 5);
            mouse('mouseup', btn, br.left + 5 + 28 * gz, br.top + 5);
            var snapped = btn.style.left;
            window.__htdProbe.message({ type: 'dfmGhosts', items: null });
            window.__htdProbe.drawNow();
            var gone = Array.prototype.every.call(host.shadowRoot.querySelectorAll('.gh'), function (x) { return x.style.display === 'none'; }) && gtg.style.display === 'none';
            R.ghostTried = 'dx ' + (gdx === null ? '-' : gdx.toFixed(1)) + ' zoom ' + gz.toFixed(2) + ' tag "' + (gtg ? gtg.textContent : '') + '" snapped ' + gl0 + '->' + snapped + ' edits ' + (edits().length - ng);
            R.ghostOk = shownOk && snapped === (parseFloat(gl0) + 30) + 'px' && edits().length === ng + 1 && gone;
            btn.style.left = gl0;
          })();

          /* spacing snaplines: dragged so its right edge is 2 px short of "8 px before the next one on the
             same row" -> it lands exactly 8 px before it, a blue line across the gap while dragging */
          (function () {
            var par = btn.offsetParent;
            var br = btn.getBoundingClientRect(), z = br.width / btn.offsetWidth;
            var sib = null;
            Array.prototype.forEach.call(par.querySelectorAll('*'), function (o) {
              if (o === btn || o.offsetParent !== par || !o.style || o.style.position !== 'absolute' || !o.getClientRects().length) return;
              var r = o.getBoundingClientRect();
              if (r.left <= br.right || Math.min(r.bottom, br.bottom) - Math.max(r.top, br.top) <= 0) return;
              if (!sib || r.left < sib.getBoundingClientRect().left) sib = o;
            });
            if (!sib) { R.gapTried = 'no neighbour on the right (skipped)'; R.gapOk = true; return; }
            var sr = sib.getBoundingClientRect();
            var dxs = (sr.left - 8 * z) - br.right - 2 * z;
            var l0 = btn.style.left, t0 = btn.style.top;
            window.__htdProbe.selectById(btn.id);
            mouse('mousedown', btn, br.left + 5, br.top + 5);
            mouse('mousemove', btn, br.left + 5 + dxs / 2, br.top + 5);
            mouse('mousemove', btn, br.left + 5 + dxs, br.top + 5);
            window.__htdProbe.drawNow();
            var gpx = host.shadowRoot.querySelector('.gpx');
            var shown = !!gpx && gpx.style.display === 'block' && Math.abs(parseFloat(gpx.style.width) - 8 * z) <= 1;
            mouse('mouseup', btn, br.left + 5 + dxs, br.top + 5);
            window.__htdProbe.drawNow();
            var gapNow = sib.getBoundingClientRect().left - btn.getBoundingClientRect().right;
            R.gapTried = 'next to ' + sib.id + ': gap ' + (gapNow / z).toFixed(1) + ' px, blue line while dragging ' + shown + ' (' + (gpx ? gpx.style.width : '-') + '), after ' + (gpx ? gpx.style.display : '-');
            R.gapOk = shown && Math.abs(gapNow / z - 8) < 0.6 && gpx.style.display === 'none';
            btn.style.left = l0; btn.style.top = t0;
          })();

          /* AutoSize (a label whose width is auto): the look says so; its resize handle is refused with the
             reason; AutoSize off = its measured size written; on again = auto */
          (function () {
            var lab = Array.prototype.filter.call(document.querySelectorAll('span.lb[id]'), function (x) { return x.style.width === 'auto' && x.getClientRects().length; })[0];
            if (!lab) { R.autoSizeTried = 'no auto label (skipped)'; R.autoSizeOk = true; return; }
            var sv = lab.style.cssText;
            window.__htdProbe.selectById(lab.id);
            window.__htdProbe.drawNow();
            var info = msgs('select').pop().info;
            var ne = edits().length, nr = msgs('editRefused').length;
            var hse = host.shadowRoot.querySelector('.hd[data-dir="se"]');
            var hr = hse.getBoundingClientRect();
            mouse('mousedown', hse, hr.left + 3, hr.top + 3);
            mouse('mousemove', lab, hr.left + 20, hr.top + 10, ALT);
            mouse('mouseup', lab, hr.left + 20, hr.top + 10);
            var refused = msgs('editRefused').slice(nr).pop();
            var noEdit = edits().length === ne && lab.style.width === 'auto';
            var w0 = lab.offsetWidth, h0 = lab.offsetHeight;
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'autoSize', value: false });
            var eOff = edits()[ne];
            var offOk = !!eOff && eOff.style && eOff.style.width === w0 + 'px' && eOff.style.height === h0 + 'px' && lab.style.width === w0 + 'px';
            var lookOff = msgs('select').pop().info.look.autoSize;
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'autoSize', value: true });
            var eOn = edits()[ne + 1];
            R.autoSizeTried = lab.id + ' look ' + (info.look && info.look.autoSize) + ' refused "' + (refused ? refused.why.slice(0, 30) : '') + '" off ' + (eOff ? JSON.stringify(eOff.style) : '-') + ' then ' + lookOff +
              ' on ' + (eOn ? JSON.stringify(eOn.style) : '-');
            R.autoSizeOk = info.look && info.look.autoSize === true && !!refused && /AutoSize/.test(refused.why) && noEdit && offOk && lookOff === false &&
              !!eOn && eOn.style.width === 'auto' && eOn.style.height === 'auto' && lab.style.width === 'auto';
            lab.style.cssText = sv;
          })();

          /* a label that wraps (WordWrap: a fixed width, height:auto) reads AutoSize on; turning it on keeps
             the width (only height:auto); a hidden label's AutoSize off is refused, never 0 x 0 */
          (function () {
            var lab = Array.prototype.filter.call(document.querySelectorAll('span.lb[id]'), function (x) { return x.getClientRects().length; })[0];
            if (!lab) { R.wrapTried = 'no label (skipped)'; R.wrapOk = true; return; }
            var sv = lab.style.cssText;
            lab.style.width = '120px'; lab.style.height = 'auto'; lab.style.whiteSpace = 'normal';
            window.__htdProbe.selectById(lab.id);
            var info = msgs('select').pop().info;
            var ne = edits().length, nr = msgs('editRefused').length;
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'autoSize', value: false });
            var eOff = edits()[ne];
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'autoSize', value: true });
            var eOn = edits()[ne + 1];
            lab.style.display = 'none';
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'autoSize', value: false });
            var hiddenRefused = edits().length === ne + 2 && msgs('editRefused').length === nr + 1;
            R.wrapTried = lab.id + ' look ' + info.look.autoSize + ' off ' + (eOff ? JSON.stringify(eOff.style) : '-') + ' on ' + (eOn ? JSON.stringify(eOn.style) : '-') + ' hidden refused ' + hiddenRefused;
            R.wrapOk = info.look.autoSize === true && !!eOff && eOff.style.width === '120px' && !!eOn && eOn.style.height === 'auto' && !('width' in eOn.style) && hiddenRefused;
            lab.style.cssText = sv;
          })();

          /* a resize handle dragged right with the mouse drifting well below: the row test uses the box's real
             top / bottom (an 'e' handle moves neither), so it still lands 8 px before the neighbour */
          (function () {
            var par = btn.offsetParent;
            window.__htdProbe.selectById(btn.id);
            window.__htdProbe.drawNow();
            var br = btn.getBoundingClientRect(), z = br.width / btn.offsetWidth;
            var sib = null;
            Array.prototype.forEach.call(par.querySelectorAll('*'), function (o) {
              if (o === btn || o.offsetParent !== par || !o.style || o.style.position !== 'absolute' || !o.getClientRects().length) return;
              var r = o.getBoundingClientRect();
              if (r.left <= br.right || Math.min(r.bottom, br.bottom) - Math.max(r.top, br.top) <= 0) return;
              if (!sib || r.left < sib.getBoundingClientRect().left) sib = o;
            });
            var he = host.shadowRoot.querySelector('.hd[data-dir="e"]');
            if (!sib || !he) { R.gapResizeTried = 'no neighbour / handle (skipped)'; R.gapResizeOk = true; return; }
            var w0 = btn.style.width;
            var hr = he.getBoundingClientRect();
            var dxs = (sib.getBoundingClientRect().left - 8 * z) - br.right - 2 * z, drift = br.height + 30 * z;
            mouse('mousedown', he, hr.left + 3, hr.top + 3);
            mouse('mousemove', btn, hr.left + 3 + dxs / 2, hr.top + 3 + drift / 2);
            mouse('mousemove', btn, hr.left + 3 + dxs, hr.top + 3 + drift);
            mouse('mouseup', btn, hr.left + 3 + dxs, hr.top + 3 + drift);
            var gapNow = (sib.getBoundingClientRect().left - btn.getBoundingClientRect().right) / z;
            R.gapResizeTried = 'right handle towards ' + sib.id + ' with the mouse ' + Math.round(drift / z) + ' px below: gap ' + gapNow.toFixed(1);
            R.gapResizeOk = Math.abs(gapNow - 8) < 0.6;
            btn.style.width = w0;
          })();

          /* Alignment of a label: taCenter -> text-align:center written, the look says taCenter; AutoSize
             off with the .dfm's size (from the DFM reset) -> exactly that size, not the text's */
          (function () {
            var lab = Array.prototype.filter.call(document.querySelectorAll('span.lb[id]'), function (x) { return x.getClientRects().length; })[0];
            if (!lab) { R.alignTried = 'no label (skipped)'; R.alignOk = true; return; }
            var sv = lab.style.cssText;
            window.__htdProbe.selectById(lab.id);
            var key = msgs('select').pop().info.key;
            var ne = edits().length;
            window.__htdProbe.message({ type: 'setLook', key: key, prop: 'alignment', value: 'taCenter' });
            var eA = edits()[ne];
            var lookA = msgs('select').pop().info.look.alignment;
            window.__htdProbe.message({ type: 'setLook', key: key, prop: 'autoSize', value: false, size: { width: 150, height: 30 } });
            var eS = edits()[ne + 1];
            R.alignTried = lab.id + ' ' + (eA ? JSON.stringify(eA.style) : '-') + ' look ' + lookA + ' | ' + (eS ? JSON.stringify(eS.style) : '-') + ' now ' + lab.offsetWidth + 'x' + lab.offsetHeight;
            R.alignOk = !!eA && eA.style['text-align'] === 'center' && lookA === 'taCenter' && !!eS && eS.style.width === '150px' && eS.style.height === '30px' &&
              lab.offsetWidth === 150 && lab.offsetHeight === 30;
            lab.style.cssText = sv;
          })();

          /* a generated panel's Alignment lives on its .pnlCap (centred by its class): right -> text-align:right
             there (target pnlCap), back to taCenter -> the declaration removed */
          (function () {
            var pn = Array.prototype.filter.call(document.querySelectorAll('div.pnl[id]'), function (x) { return x.getClientRects().length && x.querySelector(':scope > .pnlCap'); })[0];
            if (!pn) { R.panelAlignTried = 'no generated panel (skipped)'; R.panelAlignOk = true; return; }
            var cap = pn.querySelector(':scope > .pnlCap'), sv = cap.style.cssText;
            window.__htdProbe.selectById(pn.id);
            var info = msgs('select').pop().info;
            var ne = edits().length;
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'alignment', value: 'taRightJustify' });
            var eR = edits()[ne], taR = getComputedStyle(cap).textAlign;
            window.__htdProbe.message({ type: 'setLook', key: info.key, prop: 'alignment', value: 'taCenter' });
            var eC = edits()[ne + 1], taC = getComputedStyle(cap).textAlign;
            R.panelAlignTried = pn.id + ' look ' + (info.look && info.look.alignment) + ' | right ' + (eR ? eR.target + JSON.stringify(eR.style) : '-') + ' ' + taR + ' | centre ' + (eC ? JSON.stringify(eC.style) : '-') + ' ' + taC;
            R.panelAlignOk = info.look && info.look.alignment === 'taCenter' && !!eR && eR.target === 'pnlCap' && eR.style['text-align'] === 'right' && taR === 'right' &&
              !!eC && eC.style['text-align'] === null && taC === 'center';
            cap.style.cssText = sv;
          })();

          /* 縮放到選取的元件: the Save button -> the largest step it fits with a margin, and in view */
          (function () {
            window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
            window.scrollTo(0, 0);
            window.__htdProbe.selectById(btn.id);
            var bw = btn.offsetWidth, bh = btn.offsetHeight;
            var want = Math.min((window.innerWidth - 60) / bw, (window.innerHeight - 60) / bh);
            var steps = [0.125, 0.25, 0.33, 0.5, 0.67, 0.75, 0.9, 1, 1.1, 1.25, 1.5, 2, 3, 4], exp = 0.125;
            steps.forEach(function (s) { if (s <= want + 1e-9) exp = s; });
            window.__htdProbe.message({ type: 'zoomSel' });
            var zm = msgs('zoom').pop();
            var zr = btn.getBoundingClientRect();
            R.zoomSelTried = 'zoom ' + (zm ? zm.zoom : '-') + ' (expected ' + exp + ') rect ' + Math.round(zr.left) + ',' + Math.round(zr.top) + ' ' + Math.round(zr.width) + 'x' + Math.round(zr.height) +
              ' view ' + window.innerWidth + 'x' + window.innerHeight;
            R.zoomSelOk = !!zm && zm.zoom === exp && exp > 1 && zr.left >= 0 && zr.top >= 0 && zr.right <= window.innerWidth && zr.bottom <= window.innerHeight;
            window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
            window.scrollTo(0, 0);
          })();

          /* the hand: Space held + drag pans the view (a shield covers the page), a middle-button
             drag pans too; nothing gets selected, moved or written */
          (function () {
            window.__htdProbe.message({ type: 'setZoom', zoom: 2 });
            window.scrollTo(0, 0);
            var shield = host && host.shadowRoot ? host.shadowRoot.querySelector('.pan') : null;
            var np = edits().length, ns = msgs('select').length;
            var kd = function (type) { document.body.dispatchEvent(new KeyboardEvent(type, { key: ' ', code: 'Space', bubbles: true, cancelable: true })); };
            kd('keydown');
            var shown = !!shield && shield.style.display === 'block';
            var pm = function (type, x, y) { shield.dispatchEvent(new MouseEvent(type, { bubbles: true, cancelable: true, composed: true, clientX: x, clientY: y, button: 0, buttons: 1 })); };
            if (shield) { pm('mousedown', 300, 300); pm('mousemove', 250, 280); pm('mousemove', 200, 250); pm('mouseup', 200, 250); pm('click', 200, 250); }
            var sx1 = window.scrollX, sy1 = window.scrollY;
            kd('keyup');
            var hid = !!shield && shield.style.display === 'none';
            var pb = btn.getBoundingClientRect();
            mouse('mousedown', btn, pb.left + 5, pb.top + 5, { button: 1 });
            mouse('mousemove', btn, pb.left - 55, pb.top + 5, { button: 1 });
            mouse('mouseup', btn, pb.left - 55, pb.top + 5, { button: 1 });
            var sx2 = window.scrollX;
            var maxX = document.documentElement.scrollWidth - document.documentElement.clientWidth;   /* (the page's edge stops it) */
            R.panTried = 'space ' + sx1 + ',' + sy1 + ' shown ' + shown + ' hidden after ' + hid + ' | middle ' + sx2 + ' (max ' + maxX + ')' +
              ' | edits ' + (edits().length - np) + ' selects ' + (msgs('select').length - ns);
            R.panOk = shown && hid && sx1 === 100 && sy1 === 50 && Math.abs(sx2 - Math.min(160, maxX)) <= 1 && sx2 > sx1 && edits().length === np && msgs('select').length === ns;
            window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
            window.scrollTo(0, 0);
            /* Blend: Ctrl+Space held = the zoom-in pointer, a click there zooms in a step; Ctrl+Alt+Space = zoom out;
               nothing selected or written */
            var zk = function (type, alt) { document.body.dispatchEvent(new KeyboardEvent(type, { key: ' ', code: 'Space', ctrlKey: true, altKey: !!alt, bubbles: true, cancelable: true })); };
            var nz = edits().length, nzs = msgs('select').length;
            zk('keydown');
            var zinShown = !!shield && shield.style.display === 'block' && shield.classList.contains('zin');
            if (shield) { pm('mousedown', 120, 90); pm('mouseup', 120, 90); pm('click', 120, 90); }
            var zIn = (msgs('zoom').pop() || {}).zoom;
            zk('keyup');
            var zHid = !!shield && shield.style.display === 'none';
            zk('keydown', true);
            var zoutShown = !!shield && shield.classList.contains('zout');
            if (shield) { pm('mousedown', 120, 90); pm('mouseup', 120, 90); }
            var zOut = (msgs('zoom').pop() || {}).zoom;
            zk('keyup', true);
            R.clickZoom = 'in ' + zinShown + ' ' + zIn + ' hidden ' + zHid + ' out ' + zoutShown + ' ' + zOut;
            R.clickZoomOk = zinShown && zIn === 1.1 && zHid && zoutShown && zOut === 1 && edits().length === nz && msgs('select').length === nzs;
            window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
            window.scrollTo(0, 0);
          })();

          /* 工具箱放置: a tool in the hand -> a crosshair shield; a drag from a point on the Save button
             posts place with the button, its container (the point in its own px) and the form, and the
             dragged size; a click = no size; Esc gives up (placeCancel), nothing written */
          (function () {
            var shield = host && host.shadowRoot ? host.shadowRoot.querySelector('.plc') : null;
            var par = btn.offsetParent;
            /* (the button selected: its top-left resize handle is right under the point -- ours, not the page) */
            window.__htdProbe.selectById(btn.id);
            window.__htdProbe.drawNow();
            var ne = edits().length, ns = msgs('select').length;
            var sp = function (type, x, y, b) { shield.dispatchEvent(new MouseEvent(type, { bubbles: true, cancelable: true, composed: true, clientX: x, clientY: y, button: b || 0 })); };
            window.__htdProbe.message({ type: 'placeArm', cls: 'TSpeedButton', label: 'SpeedButton' });
            var shown = !!shield && shield.style.display === 'block';
            var br = btn.getBoundingClientRect(), pz = br.width / btn.offsetWidth;
            var nP = msgs('place').length;
            if (shield) { sp('mousedown', br.left + 2 * pz, br.top + 2 * pz); window.dispatchEvent(new MouseEvent('mousemove', { clientX: br.left + 42 * pz, clientY: br.top + 22 * pz, bubbles: true }));
              sp('mouseup', br.left + 42 * pz, br.top + 22 * pz); sp('click', br.left + 42 * pz, br.top + 22 * pz); }
            var pm = msgs('place')[nP];
            var tIds = pm ? pm.targets.map(function (g) { return g.id; }) : [];
            var tPar = pm ? pm.targets.filter(function (g) { return g.id === par.id; })[0] : null;
            var hidden = !!shield && shield.style.display === 'none';
            window.__htdProbe.message({ type: 'placeArm', cls: 'TLabel', label: 'Label' });
            if (shield) { sp('mousedown', br.left + 5 * pz, br.top + 5 * pz); sp('mouseup', br.left + 5 * pz, br.top + 5 * pz); }
            var pc = msgs('place')[nP + 1];
            var nC = msgs('placeCancel').length;
            window.__htdProbe.message({ type: 'placeArm', cls: 'TLabel', label: 'Label' });
            document.body.dispatchEvent(new KeyboardEvent('keydown', { key: 'Escape', bubbles: true, cancelable: true }));
            var cancelled = msgs('placeCancel').length === nC + 1 && shield.style.display === 'none';
            R.placeTried = 'targets ' + tIds.join('>') + (tPar ? ' in ' + tPar.id + ' at ' + tPar.x + ',' + tPar.y : '') + ' size ' + (pm ? pm.w + 'x' + pm.h : '-') +
              ' | click ' + (pc ? pc.cls + ' ' + pc.w + 'x' + pc.h : '-') + ' | esc ' + cancelled + ' | edits ' + (edits().length - ne) + ' selects ' + (msgs('select').length - ns);
            R.placeOk = shown && hidden && !!pm && pm.cls === 'TSpeedButton' && tIds[0] === btn.id && tIds[tIds.length - 1] === '@form' && !!tPar &&
              Math.abs(tPar.x - (btn.offsetLeft + 2)) <= 1 && Math.abs(tPar.y - (btn.offsetTop + 2)) <= 1 && pm.w === 40 && pm.h === 20 &&
              !!pc && pc.cls === 'TLabel' && pc.w === 0 && pc.h === 0 && cancelled && edits().length === ne && msgs('select').length === ns;
          })();

          /* Tab 順序: the .dfm's order = the page's first three stops with the first two swapped ->
             "1≠2", "2≠1", "3" and grey for the rest; the reply counts them; off = no badge */
          (function () {
            var stops = Array.prototype.filter.call(document.querySelectorAll('input,select,textarea,button,a[href],[tabindex]'), function (x) {
              if (x.disabled || x.tabIndex < 0 || x.type === 'hidden' || !x.getClientRects().length) return false;
              /* (hidden when the page runs, shown only while designing: no stop) */
              if (x.closest('[data-htd-show],[data-htd-vis]')) return false;
              var cs = getComputedStyle(x);
              return cs.visibility !== 'hidden' && cs.display !== 'none';
            });
            /* (a checkbox's name is on its <label class="ckb" id>, the <input> has none) */
            var nm = function (x) { if (x.id) return x.id; var pp = x.parentElement; return pp && pp.tagName === 'LABEL' && pp.id ? pp.id : ''; };
            var ids = [];
            stops.forEach(function (x) { var n0 = nm(x); if (x.tabIndex === 0 && n0 && ids.indexOf(n0) < 0) ids.push(n0); });
            if (ids.length < 4 || stops.some(function (x) { return x.tabIndex > 0; })) { R.tabTried = 'page has ' + ids.length + ' stops (skipped)'; R.tabOk = true; return; }
            var ck = stops.filter(function (x) { return x.type === 'checkbox' && !x.id && nm(x) && ids.indexOf(nm(x)) > 2; })[0];
            var dfmL = [ids[1], ids[0], ids[2], 'no-such-id'].concat(ck ? [nm(ck)] : []);
            /* (badges are drawn for what is in view: the first stops are at the top -- an earlier step
               may have scrolled the page, which is taller while designing: everything shows) */
            window.scrollTo(0, 0);
            var nTI = msgs('tabOrderInfo').length;
            window.__htdProbe.message({ type: 'tabOrder', dfm: dfmL, seq: 777 });
            window.__htdProbe.drawNow();
            var rep = msgs('tabOrderInfo')[nTI];
            var tbs = Array.prototype.filter.call(host.shadowRoot.querySelectorAll('.tb'), function (b) { return b.style.display === 'block'; });
            var txt = tbs.map(function (b) { return b.textContent; });
            var reds = tbs.filter(function (b) { return /\bbad\b/.test(b.className); }).length;
            var greys = tbs.filter(function (b) { return /\bx\b/.test(b.className); }).length;
            window.__htdProbe.message({ type: 'tabOrder', dfm: null });
            window.__htdProbe.drawNow();
            var left = Array.prototype.filter.call(host.shadowRoot.querySelectorAll('.tb'), function (b) { return b.style.display === 'block'; }).length;
            var expC = 3 + (ck ? 1 : 0);
            var where = function (id) { var e0 = document.getElementById(id); if (!e0) return id + ':none'; var q = e0.getBoundingClientRect(); return id + '@' + Math.round(q.left) + ',' + Math.round(q.top) + ' ' + Math.round(q.width) + 'x' + Math.round(q.height); };
            R.tabTried = 'badges ' + txt.slice(0, 6).join(' ') + ' (' + tbs.length + ', red ' + reds + ', grey ' + greys + ') reply ' + (rep ? rep.seq + ' stops ' + rep.stops + ' common ' + rep.common + ' bad ' + rep.bad.length : '-') +
              ' checkbox ' + (ck ? nm(ck) : '-') + ' after off ' + left + ' | first ' + where(ids[0]) + ' ' + where(ids[1]) + ' view ' + window.innerWidth + 'x' + window.innerHeight +
              ' scroll ' + Math.round(window.scrollX) + ',' + Math.round(window.scrollY);
            /* a checkbox named by its label counts like the others (4th, blue), not grey */
            R.tabOk = !!rep && rep.seq === 777 && rep.common === expC && rep.bad.length === 2 && txt.indexOf('1≠2') >= 0 && txt.indexOf('2≠1') >= 0 && txt.indexOf('3') >= 0 &&
              reds === 2 && tbs.length - greys === expC && (!ck || txt.indexOf('4') >= 0) && left === 0;
          })();

          /* 選取同型別: every button made a "TSpeedButton" -> the page scope selects them all
             (the Save button stays primary), the container scope only those beside it */
          (function () {
            var buttons = Array.prototype.filter.call(document.querySelectorAll('button[id]'), function (b) { return !b.closest('[data-htd-hide]'); });
            var cmap = {};
            buttons.forEach(function (b) { cmap[b.id] = 'TSpeedButton'; });
            window.__htdProbe.message({ type: 'init', classes: cmap });
            window.__htdProbe.selectById(btn.id);
            window.__htdProbe.message({ type: 'selectSameType', scope: 'page' });
            var s1 = msgs('select').pop(), n1 = msgs('note').pop();
            var pageIds = [s1.info.id].concat(s1.info.multi || []);
            window.__htdProbe.message({ type: 'selectSameType', scope: 'container' });
            var s2 = msgs('select').pop();
            var hereIds = [s2.info.id].concat(s2.info.multi || []);
            var cont = function (x) { for (var p = x.parentElement; p && p !== document.body; p = p.parentElement) if (p.id || (p.classList && p.classList.contains('pcPane'))) return p; return null; };
            var expHere = buttons.filter(function (b) { return cont(b) === cont(btn); }).length;
            R.sameTypeTried = 'page ' + pageIds.length + '/' + buttons.length + ' here ' + hereIds.length + '/' + expHere + ' note "' + (n1 ? n1.text : '') + '"';
            R.sameTypeOk = s1.info.id === btn.id && s2.info.id === btn.id && pageIds.length === buttons.length && hereIds.length === expHere &&
              hereIds.length >= 1 && !!n1 && /TSpeedButton/.test(n1.text);
            window.__htdProbe.message({ type: 'init', classes: {} });
            window.__htdProbe.selectById(btn.id);
          })();

          /* the grid's HTML rows: a style declaration / a text attribute, written as an edit; an on* never */
          window.__htdProbe.selectById(btn.id);
          var skR = msgs('select').pop().info.key;
          var nR = edits().length;
          window.__htdProbe.message({ type: 'setStyleRaw', key: skR, name: 'z-index', value: '5' });
          window.__htdProbe.message({ type: 'setAttrRaw', key: skR, name: 'title', value: 'Save it' });
          window.__htdProbe.message({ type: 'setAttrRaw', key: skR, name: 'onclick', value: 'x()' });
          window.__htdProbe.message({ type: 'setStyleRaw', key: skR, name: 'z-index', value: null });
          var eR = edits().slice(nR);
          R.rawEdits = eR.map(function (x) { return x.what + JSON.stringify(x.style || x.attr); }).join(' ');
          R.rawOk = eR.length === 3 && eR[0].style['z-index'] === '5' && eR[1].attr.title === 'Save it' && eR[2].style['z-index'] === null &&
            btn.getAttribute('title') === 'Save it' && !btn.hasAttribute('onclick') && btn.style.zIndex === '';

          /* Ctrl+A: every component in the button's container, the button still the primary */
          window.__htdProbe.selectById(btn.id);
          window.dispatchEvent(new KeyboardEvent('keydown', { key: 'a', ctrlKey: true, bubbles: true, cancelable: true }));
          var sa = msgs('select').pop();
          var contOf = function (x) {
            for (var p = x.parentElement; p && p !== document.body; p = p.parentElement) {
              if (p.id || p.classList.contains('form') || p.classList.contains('pcPane')) return p;
            }
            return null;
          };
          var saMulti = sa && sa.info && sa.info.multi ? sa.info.multi : [];
          R.selAll = sa && sa.info ? sa.info.id + ' + ' + saMulti.length : null;
          R.selAllOk = !!sa && sa.info.id === btn.id && saMulti.length >= 1 && saMulti.indexOf(btn.id) < 0 &&
            saMulti.every(function (id) { var x = document.getElementById(id); return x && contOf(x) === contOf(btn); });
          /* Blend: Alt+click = the layer below the selection, once per click (the form selected: round to the top one,
             the button; then its panel, then the form), the page never sees those clicks */
          window.__htdProbe.selectById('@form');
          var acR = btn.getBoundingClientRect(), acX = acR.left + acR.width / 2, acY = acR.top + acR.height / 2, acIds = [];
          for (var ac = 0; ac < 4; ac++) {
            mouse('mousedown', btn, acX, acY, { altKey: true });
            mouse('mouseup', btn, acX, acY, { altKey: true });
            mouse('click', btn, acX, acY, { altKey: true });
            var acS = msgs('select').pop();
            acIds.push(acS && acS.info ? (acS.info.id || acS.info.tag) : 'none');
          }
          var acPar = btn.offsetParent;
          R.altCycle = acIds.join(' > ');
          R.altCycleOk = acIds[0] === btn.id && !!acPar && acIds[1] === (acPar.id || acPar.tagName.toLowerCase()) && acIds.indexOf('@form') > 1;
          window.__htdProbe.selectById(btn.id);
          /* Blend: Shift+drag = a marquee from anywhere -- started on box A, it selects box B it encloses (A only partly
             in it); a Shift+click (no drag) on A then adds A */
          (function () {
            var sfr = document.querySelector('body > .form') || document.body;
            var sBox = document.createElement('div');
            sBox.style.cssText = 'position:absolute;left:10px;top:10px;width:300px;height:200px;z-index:99998;';
            var mkb = function (id, l, t) { var b = document.createElement('div'); b.id = id; b.style.cssText = 'position:absolute;left:' + l + 'px;top:' + t + 'px;width:40px;height:20px;background:#ddd;'; sBox.appendChild(b); return b; };
            sfr.appendChild(sBox);
            var sA = mkb('htdShA', 10, 10), sB = mkb('htdShB', 120, 80);
            var ar = sA.getBoundingClientRect(), bR = sB.getBoundingClientRect();
            window.__htdProbe.selectById(btn.id);
            var nsm = edits().length;
            mouse('mousedown', sA, ar.left + 20, ar.top + 10, { shiftKey: true });
            window.dispatchEvent(new MouseEvent('mousemove', { clientX: ar.left + 40, clientY: ar.top + 30, shiftKey: true, bubbles: true }));
            window.dispatchEvent(new MouseEvent('mousemove', { clientX: bR.right + 3, clientY: bR.bottom + 3, shiftKey: true, bubbles: true }));
            mouse('mouseup', sA, bR.right + 3, bR.bottom + 3, { shiftKey: true });
            mouse('click', sA, bR.right + 3, bR.bottom + 3, { shiftKey: true });
            var sm1 = msgs('select').pop();
            var mq1 = sm1 && sm1.info ? [sm1.info.id].concat(sm1.info.multi || []) : [];
            mouse('mousedown', sA, ar.left + 20, ar.top + 10, { shiftKey: true });
            mouse('mouseup', sA, ar.left + 20, ar.top + 10, { shiftKey: true });
            mouse('click', sA, ar.left + 20, ar.top + 10, { shiftKey: true });
            var sm2 = msgs('select').pop();
            var mq2 = sm2 && sm2.info ? [sm2.info.id].concat(sm2.info.multi || []) : [];
            R.shiftMarquee = mq1.join(',') + ' | then ' + mq2.join(',');
            R.shiftMarqueeOk = mq1.join(',') === 'htdShB' && mq2.indexOf('htdShA') >= 0 && mq2.indexOf('htdShB') >= 0 && edits().length === nsm;
            sfr.removeChild(sBox);
            window.__htdProbe.selectById(btn.id);
          })();
          /* WPF: Ctrl+Shift+A = clear all selections (the multi-selection too) */
          window.dispatchEvent(new KeyboardEvent('keydown', { key: 'A', ctrlKey: true, shiftKey: true, bubbles: true, cancelable: true }));
          var sn = msgs('select').pop();
          R.clearSelOk = !!sn && sn.info === null;
          /* the one asked for is not on the page (yet -- a rename redraws for its script edits before the HTML's): the
             form shows, said 'initRoot' so the extension keeps asking for the other one */
          var nIr = msgs('select').length;
          window.__htdProbe.message({ type: 'init', classes: {}, selectId: 'htdNoSuchOne' });
          var sIr = msgs('select').slice(nIr).pop();
          R.initRoot = sIr ? (sIr.info ? sIr.info.id : 'null') + '/' + sIr.origin : 'no select';
          R.initRootOk = !document.querySelector('div.form') || R.initRoot === '@form/initRoot';
          /* WPF: F9 = hide the element handles (the selection box and its tag too), F9 again = back; a click still selects */
          window.__htdProbe.selectById(btn.id);
          var f9Host = document.querySelector('htd-overlay'), f9Sr = f9Host && f9Host.shadowRoot;
          var f9Vis = function () {
            var shown = 0;
            if (f9Sr) Array.prototype.forEach.call(f9Sr.querySelectorAll('.hd'), function (x) { if (x.style.display === 'block') shown++; });
            return shown;
          };
          window.__htdProbe.drawNow();
          R.f9Before = f9Vis();
          window.dispatchEvent(new KeyboardEvent('keydown', { key: 'F9', bubbles: true, cancelable: true }));
          window.__htdProbe.drawNow();
          R.f9Off = f9Vis();
          R.f9SelBox = !!(f9Sr && f9Sr.querySelector('.b.s') && f9Sr.querySelector('.b.s').style.display === 'none');
          window.dispatchEvent(new KeyboardEvent('keydown', { key: 'F9', bubbles: true, cancelable: true }));
          window.__htdProbe.drawNow();
          R.f9On = f9Vis();
          R.f9Ok = R.f9Before === 8 && R.f9Off === 0 && R.f9SelBox && R.f9On === 8 && msgs('note').some(function (m) { return /F9/.test(m.text); });

          /* Del / Ctrl+C / Ctrl+V reach the extension as commands; a doubled paste event counts once */
          window.__htdProbe.selectById(btn.id);
          var nc = msgs('cmd').length;
          window.dispatchEvent(new KeyboardEvent('keydown', { key: 'Delete', bubbles: true, cancelable: true }));
          document.dispatchEvent(new ClipboardEvent('copy', { bubbles: true, cancelable: true }));
          document.dispatchEvent(new ClipboardEvent('paste', { bubbles: true, cancelable: true }));
          document.dispatchEvent(new ClipboardEvent('paste', { bubbles: true, cancelable: true }));
          R.cmds = msgs('cmd').slice(nc).map(function (m) { return m.cmd; }).join(',');
          R.cmdsOk = R.cmds === 'delete,copy,paste' && document.getElementById(btn.id) === btn;

          /* the live tip while dragging: the label shows the left / top going into the style */
          window.__htdProbe.selectById(btn.id);
          var lr0 = btn.getBoundingClientRect();
          mouse('mousedown', btn, lr0.left + 6, lr0.top + 6);
          mouse('mousemove', btn, lr0.left + 12, lr0.top + 8, ALT);
          mouse('mousemove', btn, lr0.left + 18, lr0.top + 10, ALT);
          window.__htdProbe.drawNow();
          var tsEl = host && host.shadowRoot ? host.shadowRoot.querySelector('.ts') : null;
          R.dragTip = tsEl ? tsEl.textContent : null;
          R.dragTipOk = !!tsEl && new RegExp('left ' + Math.round(parseFloat(btn.style.left)) + '　top ' + Math.round(parseFloat(btn.style.top))).test(tsEl.textContent);
          mouse('mouseup', btn, lr0.left + 18, lr0.top + 10);
          /* every component's name as a tag; off again */
          window.__htdProbe.message({ type: 'showNames', on: true });
          window.__htdProbe.drawNow();
          var tags = host && host.shadowRoot ? Array.prototype.filter.call(host.shadowRoot.querySelectorAll('.nt'), function (t) { return t.style.display === 'block'; }) : [];
          R.nameTags = tags.length;
          R.nameTagsOk = tags.length >= 3 && tags.some(function (t) { return t.textContent === btn.id; });
          window.__htdProbe.message({ type: 'showNames', on: false });
          window.__htdProbe.drawNow();
          R.nameTagsOffOk = tags.every(function (t) { return t.style.display === 'none'; });

          /* snap to gridlines (8 px): a free drag of (+13, +5) lands on multiples of 8, so does a
             resize from the south-east handle; the grid is drawn; off again afterwards */
          window.__htdProbe.message({ type: 'setGrid', on: true, size: 8 });
          window.__htdProbe.selectById(btn.id);
          var gr = btn.getBoundingClientRect();
          var ng = edits().length;
          mouse('mousedown', btn, gr.left + 6, gr.top + 6);
          mouse('mousemove', btn, gr.left + 12, gr.top + 8);
          mouse('mousemove', btn, gr.left + 19, gr.top + 11);
          mouse('mouseup', btn, gr.left + 19, gr.top + 11);
          var eg = edits()[ng];
          var gl = parseFloat(btn.style.left), gt = parseFloat(btn.style.top);
          window.__htdProbe.drawNow();
          var gse = host && host.shadowRoot ? host.shadowRoot.querySelector('.hd[data-dir="se"]') : null;
          var gw = NaN, gh = NaN;
          if (gse) {
            var ghr = gse.getBoundingClientRect();
            mouse('mousedown', gse, ghr.left + 3, ghr.top + 3);
            mouse('mousemove', btn, ghr.left + 7, ghr.top + 4);
            mouse('mousemove', btn, ghr.left + 10, ghr.top + 6);
            mouse('mouseup', btn, ghr.left + 10, ghr.top + 6);
            gw = parseFloat(btn.style.width); gh = parseFloat(btn.style.height);
          }
          var grd = host && host.shadowRoot ? host.shadowRoot.querySelector('.gr') : null;
          R.gridTried = 'left ' + gl + ' top ' + gt + ' w ' + gw + ' h ' + gh + ' grid ' + (grd ? grd.style.display + ' ' + grd.style.backgroundSize : '-');
          R.gridOk = !!eg && gl % 8 === 0 && gt % 8 === 0 && gw % 8 === 0 && gh % 8 === 0 && !!grd && grd.style.display === 'block' && /^8px 8px$/.test(grd.style.backgroundSize);
          window.__htdProbe.message({ type: 'setGrid', on: false, size: 8 });
          window.__htdProbe.drawNow();
          R.gridOffOk = !!grd && grd.style.display === 'none';
          /* WPF's two buttons apart: snapping without the grid drawn (still on multiples of 8), the grid drawn without
             snapping (a free move of 5 stays 5) */
          window.__htdProbe.message({ type: 'setGrid', on: true, show: false, size: 8 });
          window.__htdProbe.drawNow();
          var gsHidden = !!grd && grd.style.display === 'none';
          var gr2 = btn.getBoundingClientRect(), ng2 = edits().length;
          mouse('mousedown', btn, gr2.left + 6, gr2.top + 6);
          mouse('mousemove', btn, gr2.left + 12, gr2.top + 8);
          mouse('mousemove', btn, gr2.left + 19, gr2.top + 9);
          mouse('mouseup', btn, gr2.left + 19, gr2.top + 9);
          var gsSnapped = !!edits()[ng2] && parseFloat(btn.style.left) % 8 === 0;
          window.__htdProbe.message({ type: 'setGrid', on: false, show: true, size: 8 });
          window.__htdProbe.drawNow();
          var gsShown = !!grd && grd.style.display === 'block';
          var gl3 = parseFloat(btn.style.left), gr3 = btn.getBoundingClientRect();
          mouse('mousedown', btn, gr3.left + 6, gr3.top + 6);
          mouse('mousemove', btn, gr3.left + 9, gr3.top + 6, { altKey: true });
          mouse('mousemove', btn, gr3.left + 11, gr3.top + 6, { altKey: true });
          mouse('mouseup', btn, gr3.left + 11, gr3.top + 6);
          var gsFree = parseFloat(btn.style.left) === gl3 + 5;
          R.gridApart = 'hidden+snap ' + gsHidden + '/' + gsSnapped + ' shown+free ' + gsShown + '/' + gsFree + ' (' + gl3 + '->' + btn.style.left + ')';
          R.gridApartOk = gsHidden && gsSnapped && gsShown && gsFree;
          window.__htdProbe.message({ type: 'setGrid', on: false, show: false, size: 8 });

          /* the lock: the button of a locked panel moves neither by drag nor by arrows (said once
             each), shows no resize handles; unlocked again afterwards */
          if (hpnl && hpnl !== document.body) {
            window.__htdProbe.message({ type: 'designLocked', ids: [hpnl.id] });
            window.__htdProbe.selectById(btn.id);
            var nl0 = edits().length, nr0 = msgs('editRefused').length, lb0 = btn.style.left;
            var lr = btn.getBoundingClientRect();
            mouse('mousedown', btn, lr.left + 5, lr.top + 5);
            mouse('mousemove', btn, lr.left + 25, lr.top + 5, ALT);
            mouse('mousemove', btn, lr.left + 45, lr.top + 5, ALT);
            mouse('mouseup', btn, lr.left + 45, lr.top + 5);
            key('ArrowRight');
            window.__htdProbe.drawNow();
            var hs2 = host && host.shadowRoot ? host.shadowRoot.querySelectorAll('.hd') : [];
            R.lockTried = 'edits ' + (edits().length - nl0) + ' refused ' + (msgs('editRefused').length - nr0) + ' left ' + lb0 + '->' + btn.style.left;
            R.lockOk = edits().length === nl0 && btn.style.left === lb0 && msgs('editRefused').length === nr0 + 2 &&
              Array.prototype.every.call(hs2, function (h) { return h.style.display !== 'block'; });
            window.__htdProbe.message({ type: 'designLocked', ids: [] });
          }

          /* 接線標示: a red frame around the button, exactly on it; gone when switched off */
          window.__htdProbe.message({ type: 'wireMarks', marks: (function () { var o = {}; o[btn.id] = 'bad'; return o; })() });
          window.__htdProbe.drawNow();
          var wmb = host && host.shadowRoot ? host.shadowRoot.querySelector('.wm.bad') : null;
          var br3 = btn.getBoundingClientRect();
          R.wireMark = wmb ? wmb.style.display + ' ' + wmb.style.left + ',' + wmb.style.top : 'none';
          R.wireMarkOk = !!wmb && wmb.style.display === 'block' && Math.abs(parseFloat(wmb.style.left) - br3.left) < 1 && Math.abs(parseFloat(wmb.style.width) - br3.width) < 1;
          window.__htdProbe.message({ type: 'wireMarks', marks: null });
          window.__htdProbe.drawNow();
          R.wireMarkOffOk = !!wmb && wmb.style.display === 'none';

          /* 符合視窗: the whole form fits the view afterwards (a zoom step) */
          window.__htdProbe.message({ type: 'zoomFit' });
          var zf = msgs('zoom').pop();
          var frm2 = document.querySelector('body > .form');
          var fr2 = frm2 ? frm2.getBoundingClientRect() : null;
          R.zoomFit = zf ? zf.zoom : null;
          R.zoomFitOk = !!zf && !!fr2 && (zf.zoom === 0.125 || (fr2.width <= window.innerWidth && fr2.height <= window.innerHeight));
          /* (the rubber band below runs at this zoom: fitted, there is room around the form) */

          /* rubber band: from an empty spot (outside the form, or its own background) past its
             top-left corner -- the components entirely inside become the selection, outermost only */
          if (frm2) {
            var fb = frm2.getBoundingClientRect(), spot = null;
            var outsideAt = function (x, y) {
              if (x < 0 || y < 0 || x >= window.innerWidth || y >= window.innerHeight) return false;
              var h0 = document.elementFromPoint(x, y);
              return !!h0 && (h0 === document.body || h0 === document.documentElement);
            };
            if (outsideAt(fb.right + 6, fb.bottom + 6)) spot = { x: fb.right + 6, y: fb.bottom + 6 };
            for (var gy2 = fb.bottom - 3; !spot && gy2 > fb.top + 20; gy2 -= 7) {
              for (var gx2 = fb.right - 3; !spot && gx2 > fb.left + 20; gx2 -= 7) {
                var hitE = document.elementFromPoint(gx2, gy2);
                var up = hitE;
                while (up && up !== document.body && !up.id && up !== frm2) up = up.parentElement;
                if (up === frm2) spot = { x: gx2, y: gy2 };
              }
            }
            R.marqueeSpot = spot ? Math.round(spot.x) + ',' + Math.round(spot.y) : 'none';
            if (spot) {
              var tgtEl = document.elementFromPoint(spot.x, spot.y);
              var nsel = msgs('select').length;
              mouse('mousedown', tgtEl, spot.x, spot.y);
              mouse('mousemove', tgtEl, spot.x - 10, spot.y - 10);
              mouse('mousemove', tgtEl, fb.left + 1, fb.top + 1);
              mouse('mouseup', tgtEl, fb.left + 1, fb.top + 1);
              var ms2 = msgs('select').pop();
              var ids2 = ms2 && ms2.info ? [ms2.info.id].concat(ms2.info.multi || []) : [];
              var L2 = fb.left + 1, T2 = fb.top + 1, R2 = spot.x, B2 = spot.y;
              R.marquee = ids2.length + ' selected from ' + R.marqueeSpot + ' (' + ids2.slice(0, 4).join(',') + ')';
              R.marqueeOk = msgs('select').length > nsel && ids2.length >= 1 && ids2.indexOf('@form') < 0 && ids2.every(function (id) {
                var x = document.getElementById(id); if (!x) return false;
                var r = x.getBoundingClientRect(); return r.left >= L2 - 0.5 && r.top >= T2 - 0.5 && r.right <= R2 + 0.5 && r.bottom <= B2 + 0.5;
              });
            }
          }

          window.__htdProbe.message({ type: 'setZoom', zoom: 1 });

          /* zoom 200%: a 20px drag on screen is 10px in the page */
          window.__htdProbe.message({ type: 'setZoom', zoom: 2 });
          window.__htdProbe.selectById('spbSave');
          var zr = btn.getBoundingClientRect();
          var zl0 = parseFloat(btn.style.left);
          var nz = edits().length;
          var zx = zr.left + 6, zy = zr.top + 6;
          mouse('mousedown', btn, zx, zy);
          mouse('mousemove', btn, zx + 10, zy, ALT);
          mouse('mousemove', btn, zx + 20, zy, ALT);
          mouse('mouseup', btn, zx + 20, zy);
          var ez = edits()[nz];
          R.zoomEdit = ez ? JSON.stringify(ez.style) : null;
          R.zoomDragOk = !!ez && ez.style.left === (zl0 + 10) + 'px';
          /* Ctrl+wheel up = zoom in one step */
          var nzm = msgs('zoom').length;
          window.dispatchEvent(new WheelEvent('wheel', { deltaY: -100, ctrlKey: true, bubbles: true, cancelable: true }));
          var zm = msgs('zoom').slice(nzm).pop();
          R.wheelZoom = zm ? zm.zoom : null;
          R.wheelZoomOk = !!zm && zm.zoom === 3;
          window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
          /* WPF's artboard toolbar (bottom left): zoom out / the zoom list / in, fit, grid, snaplines; its clicks are
             ours (the page's selection does not change) */
          var ovH = document.querySelector('htd-overlay');
          var srt = ovH && ovH.shadowRoot;
          var tbq = function (a) { return srt ? srt.querySelector('.atb [data-atb="' + a + '"], .atm [data-atb="' + a + '"]') : null; };
          var tbc = function (el) { if (el) el.dispatchEvent(new MouseEvent('click', { bubbles: true, cancelable: true, composed: true })); };
          R.tbButtons = srt ? srt.querySelectorAll('.atb [data-atb]').length : 0;
          R.tbShown = !!(srt && srt.querySelector('.atb') && srt.querySelector('.atb').style.display === 'block');
          var tbSel0 = msgs('select').length, tbZ0 = msgs('zoom').length;
          tbc(tbq('zoomIn'));
          R.tbZoomIn = (msgs('zoom').slice(tbZ0).pop() || {}).zoom;
          tbc(tbq('zoomMenu'));
          R.tbMenu = srt ? srt.querySelectorAll('.atm [data-atb]').length : 0;
          tbc(tbq('z:0.5'));
          R.tbZoomHalf = (msgs('zoom').slice(tbZ0).pop() || {}).zoom;
          R.tbMenuClosed = !!(srt && srt.querySelector('.atm').style.display === 'none');
          R.tbLabel = tbq('zoomMenu') ? tbq('zoomMenu').textContent : '';
          var tbT0 = msgs('toolbar').length;
          tbc(tbq('grid'));
          tbc(tbq('gsnap'));
          tbc(tbq('snap'));
          R.tbPosts = msgs('toolbar').slice(tbT0).map(function (m) { return m.cmd + (m.cmd === 'snap' ? ':' + m.on : ''); }).join(',');
          R.tbSnapOff = !!(tbq('snap') && tbq('snap').className !== 'on');
          tbc(tbq('snap'));
          /* WPF's zoom goes 12.5% to 800%: the list has both ends, + stops at 800%, the label says 12.5% */
          tbc(tbq('zoomMenu'));
          R.tbMenuEnds = !!(tbq('z:0.125') && tbq('z:8'));
          tbc(tbq('z:8'));
          tbc(tbq('zoomIn'));
          R.tbZoomMax = (msgs('zoom').slice(tbZ0).pop() || {}).zoom;
          tbc(tbq('zoomMenu'));
          tbc(tbq('z:0.125'));
          R.tbZoomMin = (msgs('zoom').slice(tbZ0).pop() || {}).zoom;
          R.tbLabelMin = tbq('zoomMenu') ? tbq('zoomMenu').textContent : '';
          window.__htdProbe.message({ type: 'setZoom', zoom: 0.05 });
          R.tbZoomClamp = (msgs('zoom').pop() || {}).zoom;
          window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
          /* Blend: Ctrl+= / Ctrl+- = the toolbar's + / - (the extension sends zoomStep) */
          window.__htdProbe.message({ type: 'zoomStep', dir: 1 });
          var zsIn = (msgs('zoom').pop() || {}).zoom;
          window.__htdProbe.message({ type: 'zoomStep', dir: -1 });
          R.zoomStepOk = zsIn === 1.1 && (msgs('zoom').pop() || {}).zoom === 1;
          /* WPF's "Toggle artboard background": dark around the form, then the page's own again (inline, put back) */
          var abH0 = document.documentElement.style.getPropertyValue('background'), abB0 = document.body.style.getPropertyValue('background');
          var abT0 = msgs('toolbar').length;
          tbc(tbq('artboard'));
          R.tbArtDark = /45, 45, 48|2d2d30/i.test(getComputedStyle(document.documentElement).backgroundColor + ' ' + document.documentElement.style.getPropertyValue('background')) &&
            !!(tbq('artboard') && tbq('artboard').className === 'on');
          tbc(tbq('artboard'));
          R.tbArtBack = document.documentElement.style.getPropertyValue('background') === abH0 && document.body.style.getPropertyValue('background') === abB0;
          R.tbArtPosts = msgs('toolbar').slice(abT0).map(function (m) { return m.cmd + ':' + m.dark; }).join(',');
          R.tbPageBlind = msgs('select').length === tbSel0;
          R.toolbarOk = R.tbButtons >= 9 && R.tbShown && R.tbZoomIn === 1.1 && R.tbMenu >= 9 && R.tbZoomHalf === 0.5 && R.tbMenuClosed &&
            R.tbLabel === '50%' && R.tbPosts === 'gridShow,gridSnap,snap:false' && R.tbSnapOff && R.tbPageBlind;
          R.zoomRangeOk = R.tbMenuEnds && R.tbZoomMax === 8 && R.tbZoomMin === 0.125 && R.tbLabelMin === '12.5%' && R.tbZoomClamp === 0.125;
          R.artboardOk = R.tbArtDark && R.tbArtBack && R.tbArtPosts === 'artboard:true,artboard:false';
          /* WPF's information bar: a JS error of the page -> the bar at the top says so; 看全部 asks for the list, × hides
             it (until another error) */
          var ibq = function () { return srt ? srt.querySelector('.ib') : null; };
          var ibBefore = ibq() && ibq().style.display === 'block';
          window.dispatchEvent(new ErrorEvent('error', { message: 'htd-infobar-test', filename: 'http://x/test.js', lineno: 7 }));
          var ibShown = !!(ibq() && ibq().style.display === 'block' && /htd-infobar-test/.test(ibq().textContent) && /test\.js:7/.test(ibq().textContent));
          var ibS0 = msgs('showErrors').length;
          tbc(srt ? srt.querySelector('.ib [data-atb="errors"]') : null);
          var ibAsked = msgs('showErrors').length === ibS0 + 1;
          tbc(srt ? srt.querySelector('.ib [data-atb="errorsClose"]') : null);
          var ibHidden = !!(ibq() && ibq().style.display === 'none');
          R.infoBar = { before: ibBefore, shown: ibShown, asked: ibAsked, hidden: ibHidden };
          R.infoBarOk = ibShown && ibAsked && ibHidden && R.tbPageBlind && msgs('select').length === tbSel0;
          window.__htdProbe.message({ type: 'setZoom', zoom: 1 });
          finish();
        }, 100);
      }, 600);
    } catch (e) {
      R.editError = String(e && e.stack || e);
      finish();
    }
  }
})();
