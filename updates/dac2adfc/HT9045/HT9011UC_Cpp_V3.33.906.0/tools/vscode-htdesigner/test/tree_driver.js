/* AI(W906-HTDESIGNER) 20261001: the component tree on a page with nested PageControls (HW.IoSetView.html) -- WPF's
   Document Outline / BCB6's Object TreeView: each TTabSheet a node under its PageControl, the sheet's controls under
   the sheet; a sheet picked in the tree = that tab shown; a control on another tab is not "hidden".
   Writes "HTDTEST{json}HTDEND" into the DOM (tree_test.ps1 reads it). */
(function () {
  var R = {};
  function msgs(t) { return (window.__htdOut || []).filter(function (m) { return m.type === t; }); }
  function finish() {
    var pre = document.createElement('pre');
    pre.id = 'HTDTEST';
    pre.textContent = 'HTDTEST' + JSON.stringify(R) + 'HTDEND';
    document.body.appendChild(pre);
  }
  setTimeout(function () {
    try {
      var tree = msgs('tree').pop();
      var nodes = tree ? tree.nodes : [];
      var byKey = {}, byId = {};
      nodes.forEach(function (n) { byKey[n[0]] = n; if (!byId[n[2]]) byId[n[2]] = n; });
      var kids = function (k) { return nodes.filter(function (n) { return n[1] === k; }); };
      R.count = nodes.length;
      var pgc = byId.pgcStack1;
      R.pgc = !!pgc;
      R.pgcKids = pgc ? kids(pgc[0]).map(function (n) { return n[2] + ':' + n[4] + ':' + n[5]; }) : [];
      R.sheets = nodes.filter(function (n) { return n[4] === 'TTabSheet'; }).length;
      var cas = byId.tsStack1_Cassette;
      R.cassette = cas ? { parent: byKey[cas[1]] ? byKey[cas[1]][2] : '', cap: cas[5], hidden: cas[6], kids: kids(cas[0]).length } : null;
      /* every control under some tab sheet: its tree parent chain reaches that sheet */
      var dirtyKids = 0;
      if (pgc) kids(pgc[0]).forEach(function (n) { if (n[4] !== 'TTabSheet') dirtyKids++; });
      R.pgcNonSheetKids = dirtyKids;
      /* a control on another (not active) tab is not marked hidden */
      var otherTab = cas ? kids(cas[0]) : [];
      R.otherTabHidden = otherTab.filter(function (n) { return n[6]; }).map(function (n) { return n[2]; });
      R.otherTabCount = otherTab.length;
      /* pick the sheet in the tree */
      var before = msgs('select').length;
      window.__htdProbe.message({ type: 'selectId', id: 'tsStack1_Cassette', origin: 'tree' });
      setTimeout(function () {
        try {
          var s = msgs('select').slice(before).pop();
          R.selId = s && s.info ? s.info.id : null;
          var pane = null, ps = document.querySelectorAll('.pcPane');
          for (var i = 0; i < ps.length; i++) {
            var w = ps[i].parentElement && ps[i].parentElement.parentElement;
            if (w && w.id === 'pgcStack1' && ps[i].getAttribute('data-p') === '2') pane = ps[i];
          }
          R.paneShown = !!(pane && getComputedStyle(pane).display !== 'none');
          var act = document.querySelector('#pgcStack1 > .pcTabs > .tab.act');
          R.activeTab = act ? act.textContent : '';
          /* a tool put down on the sheet (0.135): the targets name the sheet, with the point in it */
          if (pane) {
            var pr = pane.getBoundingClientRect();
            var tg = window.__htdProbe.placeTargetsAt(Math.round(pr.left + pr.width - 6), Math.round(pr.top + pr.height - 6)) || [];
            var hit = tg.filter(function (t) { return t && t.pane; })[0];
            R.placePane = hit ? hit.id + '@' + hit.x + ',' + hit.y : 'none: ' + JSON.stringify(tg).slice(0, 200);
          }
          /* an id that is not a sheet still works as before */
          var b2 = msgs('select').length;
          window.__htdProbe.message({ type: 'selectId', id: 'pnlStack1', origin: 'tree' });
          setTimeout(function () {
            var s2 = msgs('select').slice(b2).pop();
            R.plainSel = s2 && s2.info ? s2.info.id : null;
            finish();
          }, 300);
        } catch (e) { R.error = String(e && e.stack || e); finish(); }
      }, 400);
    } catch (e) { R.error = String(e && e.stack || e); finish(); }
  }, 2500);
})();
