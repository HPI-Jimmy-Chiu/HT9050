/* ht9045_teach_formshow_c.js -- Teach: golden TfTeach::FormShow's screen half, computed by C++ (hand-written, not generated).
 *
 * AI(W906-TEACH-FORMSHOW) 20261002: EastSun 20261001「請檢查每個頁面元件」「不是只有檢查按鈕喔 我說的是所有元件」——
 *   the every-component check found that this page applied none of golden FormShow's ~150 Visible / TabVisible / Caption /
 *   Color / Enabled / ActivePage assignments (uteach.cpp:1413-2012): every OCR / Preciser / laser / Rotate Kit / Auto Clean /
 *   16-picker / Bin Box / X-Y pitch panel showed as drawn in uteach.dfm whatever this machine's options are.
 *   C++ runs those golden lines on the globals it really loaded (FileRW/TeachFormShow_File.cpp, S12 second-type bridge) and
 *   serves the result as GET /api/form/HW.teach.html; this file applies it:
 *     * once when the page loads, and again at every window open (golden FormShow runs at every Show; a minimise is not a
 *       Show -- background.html sends open:true for it, so only a closed -> open transition re-runs, same as teach STREAM-S2E);
 *     * visible:false -> class teach-fs-hidden (display:none !important); visible:true -> that class off and an inline
 *       display:none from the dfm cleared (golden sets Visible=true on dfm-hidden groups, e.g. grpInShLtcPos :1960).
 *       A class another rule owns (teach-axis-hidden: this machine has no such axis; rule-hide: View-rules.json) still wins;
 *     * tabVisible: the .tab header and its .pcPane; a hidden active tab moves to the first visible one (VCL does the same);
 *     * caption: label / button text, GroupBox legend, tab header; color "#rrggbb": background (edits / panels);
 *       enabled:false: disabled; activePage / activePageIndex: the PageControl switches to that sheet (golden :1599-1600 opens
 *       on Axle Control; :1875 forces TabSheet10 when Index Y find-phase differs);
 *     * "<parent>/children:TSpeedButton,TEdit": every button / edit inside <parent> (golden :1785-1803 loops Controls[]).
 *   Text is NOT applied: the data cells come from the C path (FileRW/Teach.cpp), and the bridge sends none.
 *   No C++ or an old wb_serve (404): nothing changes, the page stays as before.
 */
(function () {
  'use strict';
  var PAGE = 'HW.teach.html';
  var last = null;                                       // last /api/form answer (probe / debugging)
  function css() {
    if (document.getElementById('teachFsCss')) return;
    var st = document.createElement('style'); st.id = 'teachFsCss';
    st.textContent = '.teach-fs-hidden{display:none !important;}';
    document.head.appendChild(st);
  }
  function byTitle(name) {
    return document.querySelector('[title^="' + name + ' :"],[data-htitle^="' + name + ' :"]');
  }
  function ttl(e) { return e.getAttribute('title') || e.getAttribute('data-htitle') || ''; }   // the hover-tip code moves title to data-htitle
  function tabOf(name) {                                 // TTabSheet: the .tab header (title "<name> : TTabSheet") + its pane
    var t = document.querySelector('.tab[title^="' + name + ' :"],.tab[data-htitle^="' + name + ' :"]');
    if (!t) return null;
    var wrap = t.closest('.pcWrap'), pane = null;
    if (wrap) {
      var ps = wrap.querySelectorAll('.pcPane');
      for (var i = 0; i < ps.length; i++) if (ps[i].closest('.pcWrap') === wrap && ps[i].getAttribute('data-p') === t.getAttribute('data-t')) { pane = ps[i]; break; }
    }
    return { tab: t, pane: pane, wrap: wrap };
  }
  function show(el, on) {
    if (!el) return;
    if (on) { el.classList.remove('teach-fs-hidden'); if (el.style.display === 'none') el.style.display = ''; }
    else el.classList.add('teach-fs-hidden');
  }
  function ownTabs(wrap) {
    return Array.prototype.filter.call(wrap.querySelectorAll('.tab'), function (t) { return t.closest('.pcWrap') === wrap; });
  }
  function shown(e) { return !!e && getComputedStyle(e).display !== 'none'; }
  function setTabVisible(name, on) {
    var x = tabOf(name); if (!x) return false;
    show(x.tab, on);
    if (!on && x.tab.classList.contains('act') && x.wrap) {              // VCL: hiding the active sheet moves to the next visible one
      var v = ownTabs(x.wrap).filter(shown);
      if (v.length) v[0].click();
    }
    if (!on && x.pane) x.pane.classList.add('teach-fs-hidden'); else if (x.pane) x.pane.classList.remove('teach-fs-hidden');
    return true;
  }
  function setCaption(el, text) {
    if (el.tagName === 'FIELDSET') { var lg = el.querySelector('legend'); if (lg) { lg.textContent = text; return; } }
    if (!el.children.length) { el.textContent = text; return; }
    for (var n = el.firstChild; n; n = n.nextSibling) if (n.nodeType === 3 && n.nodeValue.trim()) { n.nodeValue = text; return; }
    var sp = el.querySelector('span'); if (sp && !sp.children.length) sp.textContent = text;
  }
  function activate(pcName, sheet, index) {
    var pc = document.getElementById(pcName) || byTitle(pcName); if (!pc) return false;
    var wrap = pc.classList.contains('pcWrap') ? pc : (pc.querySelector('.pcWrap') || pc);
    var tabs = ownTabs(wrap), t = null;
    if (sheet) t = tabs.filter(function (x) { return ttl(x).indexOf(sheet + ' :') === 0; })[0] || null;
    if (!t && index !== undefined) t = tabs.filter(function (x) { return x.getAttribute('data-t') === String(index); })[0] || null;
    if (t && shown(t)) { t.click(); return true; }
    return false;
  }
  function apply(d) {
    last = d;
    if (!d || d.available === false || !d.widgets) return { applied: 0, why: d && (d.why || d.note) };
    css();
    var n = 0, miss = [], pages = [];
    Object.keys(d.widgets).forEach(function (id) {
      var w = d.widgets[id];
      var ch = /^([^/]+)\/children:/.exec(id);
      if (ch) {                                                           // golden :1785-1803
        var par = document.getElementById(ch[1]) || byTitle(ch[1]);
        if (!par) { miss.push(id); return; }
        if (w.visible !== undefined) par.querySelectorAll('button, input, .btnpanel').forEach(function (c) { show(c, !!w.visible); });
        n++; return;
      }
      var el = document.getElementById(id) || byTitle(id);
      if (w.activePage !== undefined || w.activePageIndex !== undefined) { pages.push([id, w.activePage, w.activePageIndex]); n++; if (!el) return; }
      if (w.tabVisible !== undefined) { if (setTabVisible(id, !!w.tabVisible)) n++; else miss.push(id); }
      if (!el) { if (w.tabVisible === undefined && w.activePage === undefined && w.activePageIndex === undefined) miss.push(id);
                 else if (w.caption !== undefined) { var tx = tabOf(id); if (tx) setCaption(tx.tab, w.caption); }
                 return; }
      if (w.visible !== undefined) show(el, !!w.visible);
      if (w.caption !== undefined) setCaption(el, w.caption);
      if (typeof w.color === 'string' && w.color.charAt(0) === '#') el.style.backgroundColor = w.color;
      if (w.enabled === false) { if ('disabled' in el) el.disabled = true; el.setAttribute('aria-disabled', 'true'); el.setAttribute('data-fs-disabled', '1'); }
      else if (w.enabled === true && el.getAttribute('data-fs-disabled')) { if ('disabled' in el) el.disabled = false; el.removeAttribute('aria-disabled'); el.removeAttribute('data-fs-disabled'); }
      el.setAttribute('data-fs', '1');
      n++;
    });
    pages.forEach(function (p) { activate(p[0], p[1], p[2]); });          // last: golden's order, PageControl2 after its sheets
    var r = { applied: n, missing: miss, todo: d.todo || [], form: d.form };
    window.__TEACH_FORMSHOW__ = r;
    if (miss.length && window.console) console.warn('[teach formshow] not on this page:', miss);
    return r;
  }
  function run() {
    var get = (window.HT9045Recipe && HT9045Recipe.form) ? HT9045Recipe.form(PAGE)
      : (location.protocol === 'http:' || location.protocol === 'https:')
        ? fetch('/api/form/' + encodeURIComponent(PAGE), { cache: 'no-store' }).then(function (r) { if (!r.ok) throw new Error('HTTP ' + r.status); return r.json(); })
        : Promise.reject(new Error('no server'));
    return get.then(apply, function (e) { window.__TEACH_FORMSHOW__ = { applied: 0, error: String(e && e.message || e) }; return null; });
  }
  var shownWin = false, hosted = false;
  window.addEventListener('message', function (ev) {
    var d = ev && ev.data;
    if (!d || d.type !== 'HT_WIN' || ev.source !== window.parent || window.parent === window) return;
    var was = shownWin; hosted = true; shownWin = !!d.open;
    if (shownWin && !was) run();                                          // closed -> open = golden FormShow
  });
  window.HT9045TeachFormShow = { run: run, apply: apply, last: function () { return last; } };
  // AI(W906-TEACH-ALLCOMP) 20261002: golden Label2Click (uteach.cpp:4295-4298) -- clicking the "Active Motor" caption shows the
  //   Axle Control sheet (PageControl2->ActivePage=tsAxleCtrl); the page had no handler (every-component check)
  function bindLabel2() {
    var l = document.getElementById('Label2') || byTitle('Label2');
    if (!l || l.getAttribute('data-l2')) return;
    l.setAttribute('data-l2', '1'); l.style.cursor = 'pointer';
    l.addEventListener('click', function () { activate('PageControl2', 'tsAxleCtrl'); });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', bindLabel2); else bindLabel2();
  // first load (golden: the form is created and shown); the C-path data and the axis hiding load in parallel and do not
  // depend on this
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', function () { if (!hosted) run(); });
  else if (!hosted) run();
})();
