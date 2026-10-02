/* ht9045_iosetview_formshow_c.js -- HW.IoSetView: golden Tfiosetview::FormShow's screen half, computed by C++ (hand-written,
 * not generated).
 *
 * AI(W906-IOSV-FORMSHOW) 20261002: EastSun 20261001「請檢查每個頁面元件」「不是只有檢查按鈕喔 我說的是所有元件」—— the every-
 *   component check (compK ioS) found that this page applied none of golden FormShow's Visible / TabVisible / Caption / Enabled
 *   assignments (iosetview.cpp:236-1023, + ShowSuckMode :1313, ShowShuttleSensor :1671, LabSiteMap :2861, Hide_1032_IO :3907):
 *   every group showed as drawn in iosetview.dfm (Color tray, 2nd loader, AOI, OTD, catch tray, Neg. air, ATC, TTL map, the
 *   index sucker names ...) whatever this machine's options are. C++ runs those golden lines on the globals it really loaded
 *   (FileRW/IoSetViewFormShow_File.cpp, S12 second-type bridge) and serves GET /api/form/HW.IoSetView.html; this file applies it
 *   the way page/ht9045_teach_formshow_c.js does for Teach:
 *     * on load, and again at every window open (golden FormShow runs at every Show; a minimise is not a Show);
 *     * visible:false -> class iosv-fs-hidden (display:none !important); visible:true -> that class off and an inline
 *       display:none from the .dfm cleared (golden shows dfm-hidden ones, e.g. lbTrayArmSafePos :797). A rule another owner
 *       applies still wins: data-ht9050-hidden (EastSun 20260930, IO9050HIDE), the HT9050 tab CSS, rule-hide (View-rules.json);
 *     * tabVisible:false -> only the tab header is hidden (VCL: a sheet whose TabVisible is false still shows its content when it
 *       is the ActivePage -- golden Hide_1032_IO :3909-3916 hides all of Stack 1's tabs and then sets ActivePage :3926);
 *       hiding the active header moves to the first visible tab, as VCL does;
 *     * activePage / activePageIndex: that sheet's pane is shown even when its header is hidden (VCL). NOT on HT9050 for
 *       pgcStack1 / pgcStack2: there the page shows the Above9050 sheet (W906-IOWEB-P28, machine engineer 20260925) --
 *       golden V906 has no such sheet, its ActivePage would land on a tab the machine does not use;
 *     * caption: label / panel / button text, GroupBox legend; color "#rrggbb": background; checked: check boxes;
 *     * enabled: track bars and plain buttons only. NOT the IO panel buttons (.btnpanel): golden FormShow's Enabled on them
 *       (:404-441 IndexHasIC / EP_Install) is a lock, and EastSun 20260929「IO畫面一律不要卡控，讓我測試」turned those locks
 *       off in C++ (JsonBridge/IoBtnPanelClick.cpp W906_IO_PAGE_NO_GUARDS); ht9045_io_do.js / C++ decide those buttons.
 *   No C++ or an old wb_serve (404): nothing changes, the page stays as before (the reason goes to window.__IOSV_FORMSHOW__).
 */
(function () {
  'use strict';
  var PAGE = 'HW.IoSetView.html';
  var STACK_9050 = { pgcStack1: 1, pgcStack2: 1 };     // W906-IOWEB-P28: on HT9050 these show Above9050 (ht9050Gate)
  var last = null;
  function css() {
    if (document.getElementById('iosvFsCss')) return;
    var st = document.createElement('style'); st.id = 'iosvFsCss';
    st.textContent = '.iosv-fs-hidden{display:none !important;}';
    document.head.appendChild(st);
  }
  function ttl(e) { return e.getAttribute('title') || e.getAttribute('data-htitle') || ''; }   // theme.js release mode moves title
  function byTitle(name) { return document.querySelector('[title^="' + name + ' :"],[data-htitle^="' + name + ' :"]'); }
  function ownTabs(wrap) { return Array.prototype.filter.call(wrap.querySelectorAll('.tab'), function (t) { return t.closest('.pcWrap') === wrap; }); }
  function ownPanes(wrap) { return Array.prototype.filter.call(wrap.querySelectorAll('.pcPane'), function (p) { return p.closest('.pcWrap') === wrap; }); }
  function shown(e) { return !!e && getComputedStyle(e).display !== 'none'; }
  function tabOf(name) {                               // TTabSheet: the .tab header (title "<name> : TTabSheet") + its pane
    var t = document.querySelector('.tab[title^="' + name + ' :"],.tab[data-htitle^="' + name + ' :"]');
    if (!t) return null;
    var wrap = t.closest('.pcWrap'), pane = null;
    if (wrap) ownPanes(wrap).forEach(function (p) { if (p.getAttribute('data-p') === t.getAttribute('data-t')) pane = p; });
    return { tab: t, pane: pane, wrap: wrap };
  }
  function show(el, on) {
    if (!el) return;
    if (on) { el.classList.remove('iosv-fs-hidden'); if (el.style.display === 'none') el.style.display = ''; }
    else el.classList.add('iosv-fs-hidden');
  }
  function setTabVisible(name, on) {
    var x = tabOf(name); if (!x) return false;
    show(x.tab, on);
    if (!on && x.tab.classList.contains('act') && x.wrap) {            // VCL: hiding the active sheet's tab moves to the next visible one
      var v = ownTabs(x.wrap).filter(shown);
      if (v.length) v[0].click();
    }
    return true;
  }
  function setCaption(el, text) {
    if (el.tagName === 'FIELDSET') { var lg = el.querySelector('legend'); if (lg) { lg.textContent = text; return; } }
    var cap = el.querySelector(':scope > .pnlCap, :scope > .lledCap');
    if (cap) { cap.textContent = text; return; }
    if (!el.children.length) { el.textContent = text; return; }
    for (var n = el.firstChild; n; n = n.nextSibling) if (n.nodeType === 3 && n.nodeValue.trim()) { n.nodeValue = text; return; }
    var sp = el.querySelector('span'); if (sp && !sp.children.length) sp.textContent = text;
  }
  function machine() { return document.documentElement.getAttribute('data-machine'); }   // set by the page's ht9050Gate
  // VCL ActivePage: show that sheet's pane even when its tab header is hidden; the header (if shown) is marked active
  function activate(pcName, sheet, index, tries) {
    var pc = document.getElementById(pcName) || byTitle(pcName); if (!pc) return false;
    if (STACK_9050[pcName]) {
      var m = machine();
      if (m === null && (tries || 0) < 15) { setTimeout(function () { activate(pcName, sheet, index, (tries || 0) + 1); }, 300); return true; }
      if (m === 'HT9050' && pc.querySelector(':scope > .pcTabs > .tab.ht9050-only')) return false;   // W906-IOWEB-P28 (file header)
    }
    var wrap = pc.classList.contains('pcWrap') ? pc : (pc.querySelector('.pcWrap') || pc);
    var tabs = ownTabs(wrap), t = null;
    if (sheet) t = tabs.filter(function (x) { return ttl(x).indexOf(sheet + ' :') === 0; })[0] || null;
    if (!t && index !== undefined) t = tabs.filter(function (x) { return x.getAttribute('data-t') === String(index); })[0] || null;
    if (!t) return false;
    if (shown(t)) { t.click(); return true; }
    tabs.forEach(function (x) { x.classList.remove('act'); });
    ownPanes(wrap).forEach(function (p) { p.style.display = (p.getAttribute('data-p') === t.getAttribute('data-t')) ? 'block' : 'none'; });
    return true;
  }
  function apply(d, part) {                            // part: an event's "changed" map (AI(W906-IOSV-SHOWALL)), not a FormShow
    if (!part) last = d;
    if (!d || d.available === false || !d.widgets) { if (!part) window.__IOSV_FORMSHOW__ = { applied: 0, why: d && (d.why || d.error || d.note) }; return null; }
    css();
    var n = 0, miss = [], pages = [];
    Object.keys(d.widgets).forEach(function (id) {
      var w = d.widgets[id];
      var el = document.getElementById(id) || byTitle(id);
      if (w.activePage !== undefined || w.activePageIndex !== undefined) { pages.push([id, w.activePage, w.activePageIndex]); n++; }
      if (w.tabVisible !== undefined) { if (setTabVisible(id, !!w.tabVisible)) n++; else miss.push(id); }
      if (!el) {
        if (w.tabVisible === undefined && w.activePage === undefined && w.activePageIndex === undefined) miss.push(id);
        else if (w.caption !== undefined) { var tx = tabOf(id); if (tx) setCaption(tx.tab, w.caption); }
        return;
      }
      if (el.classList.contains('tab')) return;                          // a TTabSheet found by title: header handled above
      if (w.visible !== undefined) show(el, !!w.visible);
      if (w.caption !== undefined) setCaption(el, w.caption);
      if (typeof w.color === 'string' && w.color.charAt(0) === '#') el.style.backgroundColor = w.color;
      if (w.checked !== undefined) {
        var cb = el.tagName === 'INPUT' ? el : el.querySelector('input[type="checkbox"]');
        if (cb) cb.checked = !!w.checked;
      }
      if (w.enabled !== undefined && !el.classList.contains('btnpanel')) {   // not the IO panel buttons (file header)
        if (w.enabled === false) { if ('disabled' in el) el.disabled = true; el.setAttribute('aria-disabled', 'true'); el.setAttribute('data-fs-disabled', '1'); }
        else if (el.getAttribute('data-fs-disabled')) { if ('disabled' in el) el.disabled = false; el.removeAttribute('aria-disabled'); el.removeAttribute('data-fs-disabled'); }
      }
      el.setAttribute('data-fs', '1');
      n++;
    });
    pages.forEach(function (p) { activate(p[0], p[1], p[2]); });          // last: golden's order (Hide_1032_IO sets them after the tabs)
    var r = { applied: n, missing: miss, todo: d.todo || [], form: d.form };
    if (part) window.__IOSV_SHOWALL__ = r; else window.__IOSV_FORMSHOW__ = r;
    if (miss.length && window.console) console.warn('[iosetview formshow] not on this page:', miss);
    return r;
  }
  // AI(W906-IOSV-SHOWALL) 20261002: golden chkShowIndexAllClick (iosetview.cpp:1760-1766) -- every Index / InArm / OutArm sucker
  //   panel drawn "in use" with the Index sucker names (ShowSuckMode(-1) :1318-1344) and the box hides itself (:1765). It had no
  //   web handler. C++ runs the golden lines (WS form.event, FileRW/IoSetViewFormShow_File.cpp E_chkShowIndexAllClick) and the
  //   answer is applied like the FormShow one. Colours and captions only -- nothing on the machine changes. No confirm (golden none).
  function unwrap(m) {
    if (m && typeof m.value === 'string') { try { return JSON.parse(m.value); } catch (e) { return m; } }
    return m;
  }
  function hookShowAll() {
    var box = document.getElementById('chkShowIndexAll'); if (!box || box.__iosvAll) return;
    var cb = box.tagName === 'INPUT' ? box : box.querySelector('input[type="checkbox"]'); if (!cb) return;
    box.__iosvAll = true;
    cb.addEventListener('change', function (ev) {
      if (ev && ev.isTrusted === false) return;                         // only the operator's click (golden OnClick)
      var R = window.HT9045Recipe;
      if (!R || !R.rawCmd) { window.__IOSV_SHOWALL__ = { error: 'no ht9045_recipe_client.js' }; return; }
      var v = { form: 'Tfiosetview', control: 'chkShowIndexAll', event: 'click', checked: !!cb.checked };
      var pre = (R.status && R.keepAlive && !R.status().holdsToken) ? R.keepAlive().catch(function () {}) : Promise.resolve();
      pre.then(function () { return R.rawCmd('form.event', { tag: PAGE, value: JSON.stringify(v) }); }).then(function (m) {
        var a = unwrap(m) || {};
        apply({ widgets: a.changed || {} }, true);
      }, function (e) {
        var msg = String(e && e.message || e);
        window.__IOSV_SHOWALL__ = { error: msg };
        cb.checked = !cb.checked;                                         // not run: the box shows what golden would still show
        box.setAttribute('title', (box.getAttribute('title') || '').replace(/\n（沒有送到）.*$/, '') + '\n（沒有送到）form.event：' + msg);
        if (window.console) console.warn('[iosetview chkShowIndexAll] form.event failed:', msg);
      });
    });
  }
  function run() {
    var get = (window.HT9045Recipe && HT9045Recipe.form) ? HT9045Recipe.form(PAGE)
      : (location.protocol === 'http:' || location.protocol === 'https:')
        ? fetch('/api/form/' + encodeURIComponent(PAGE), { cache: 'no-store' }).then(function (r) { if (!r.ok) throw new Error('HTTP ' + r.status); return r.json(); })
        : Promise.reject(new Error('no server'));
    return get.then(apply, function (e) { window.__IOSV_FORMSHOW__ = { applied: 0, error: String(e && e.message || e) }; return null; });
  }
  var shownWin = false, hosted = false;
  window.addEventListener('message', function (ev) {
    var d = ev && ev.data;
    if (!d || d.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
    var was = shownWin; hosted = true; shownWin = !!d.open;
    if (shownWin && !was) run();                                          // closed -> open = golden FormShow
  });
  // AI(W906-IOSV-GEOM) 20261002: golden ctor iosetview.cpp:55-62 puts each 16-sucker Index panel exactly where its 8-sucker twin
  //   is (palArm*_X_16->Top / Left = palArm*_X->Top / Left); the .dfm (and so the page) draws them side by side, so on a machine
  //   that hides the 8-sucker ones (FormShow :258-282) the web showed the 16-sucker panels half a window to the right.
  function ctorGeometry() {
    [['palArm2_A_16', 'palArm2_A'], ['palArm2_B_16', 'palArm2_B'], ['palArm1_A_16', 'palArm1_A'], ['palArm1_B_16', 'palArm1_B']].forEach(function (p) {
      var a = document.getElementById(p[0]), b = document.getElementById(p[1]);
      if (a && b) { a.style.top = b.style.top; a.style.left = b.style.left; }
    });
  }
  window.HT9045IoSetViewFormShow = { run: run, apply: apply, last: function () { return last; } };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', function () { ctorGeometry(); hookShowAll(); if (!hosted) run(); });
  else { ctorGeometry(); hookShowAll(); if (!hosted) run(); }
})();
