/* AI(W906-HTDESIGNER) 20260929: HTML 視覺設計工具 -- the in-page probe.
 *
 * Loaded as the first script of the previewed page (the extension injects it right
 * after <head>, behind a Content-Security-Policy that allows no network at all
 * except the webview's own local files). It must run before any page script so it
 * can record every event listener the page attaches.
 *
 * What it does:
 *   - records addEventListener / on* property handlers per element, with the call
 *     stack at the moment of binding (that is how "which JS handles this button" is
 *     answered for real, not guessed from text);
 *   - in DESIGN mode, swallows pointer events so the page's own handlers never run
 *     (a click selects the component instead); tabs still switch, Ctrl+click
 *     operates once;
 *   - draws hover / selection boxes in a closed shadow root;
 *   - reports the component tree and the selected component to the extension.
 *
 * Safety: the CSP is the wall (no ws:, no http: to 127.0.0.1:8045/8046, no
 * frames, no form posts). What is here are belts on top: links never navigate,
 * forms never submit, window.open is a no-op.
 */
(function () {
  'use strict';
  if (window.__htdProbe) return;

  var api = null;
  try { api = acquireVsCodeApi(); } catch (e) { api = null; }
  function post(m) {
    m.__htd = 1;
    if ((m.type === 'edit' || m.type === 'select') && typeof tabCache !== 'undefined') tabCache = null;   // (an edit / a tab switch may change the Tab stops)
    try {
      if (api) api.postMessage(m);
      else (window.__htdOut = window.__htdOut || []).push(JSON.parse(JSON.stringify(m)));
    } catch (e) { /* ignore */ }
  }

  var me = document.currentScript;
  var PROBE_URL = (me && me.src) || '';
  var mode = (me && me.getAttribute('data-mode')) || 'design';
  var passNext = false;

  /* 1. VS Code puts its own default styles (body padding 0 20px, img max-width...)
        in front of ours; the real browser has none of that. Remove them. */
  try {
    var ss = document.querySelectorAll('style');
    for (var si = 0; si < ss.length; si++) {
      var st = ss[si];
      if (me && !(st.compareDocumentPosition(me) & Node.DOCUMENT_POSITION_FOLLOWING)) continue;
      if (st.id === '_defaultStyles' || /--vscode-/.test(st.textContent || '')) st.parentNode.removeChild(st);
    }
  } catch (e) { /* ignore */ }

  /* 2. belts: no popups */
  try { window.open = function () { return null; }; } catch (e) { /* ignore */ }

  /* 2b. 機種與機台設定 (READ-ONLY copies the extension put in #__htd_live): the machine F5 opens the HMI
         for goes where background.html puts it (the launch options), and GET /api/system/<name> is
         answered from the copies -- so a page decides what to show the way it does on the machine
         (HW.IoSetView's Above9050 tab). Only those GETs; everything else still has no network. */
  var LIVE = null;
  try { var liveEl = document.getElementById('__htd_live'); if (liveEl) LIVE = JSON.parse(liveEl.textContent || 'null'); } catch (e) { LIVE = null; }
  window.__htdLiveAnswered = [];
  window.__htdLiveMoved = [];
  if (LIVE) {
    if (LIVE.machine) {
      var LAUNCH = 'ht9xxx-launch-options';
      var launchWith = function (s) {
        var o = {};
        try { o = JSON.parse(s || '{}') || {}; } catch (e2) { o = {}; }
        o.machine = LIVE.machine;
        return JSON.stringify(o);
      };
      try { sessionStorage.setItem(LAUNCH, launchWith(sessionStorage.getItem(LAUNCH))); } catch (e) { /* no storage here: answered below */ }
      try {
        var sGet = Storage.prototype.getItem;
        Storage.prototype.getItem = function (k) {
          var v = sGet.apply(this, arguments);
          return k === LAUNCH ? launchWith(v) : v;
        };
      } catch (e) { /* ignore */ }
    }
    var liveDoc = function (method, url) {
      if (String(method || 'GET').toUpperCase() !== 'GET') return null;
      var m = /\/api\/system\/([A-Za-z0-9_]+)\/?(?:[?#]|$)/.exec(String(url || ''));
      if (!m || !LIVE.docs || !Object.prototype.hasOwnProperty.call(LIVE.docs, m[1])) return null;
      return { name: m[1], body: JSON.stringify(LIVE.docs[m[1]]) };
    };
    /* a page that picks its JSON folder by its address (/page/ in the path -> ../JSON/, else JSON/) takes
       itself for the web root here (the designer's address has no /page/): its JSON/… goes to the web root's */
    var fixFrom = null, fixTo = null;
    try { if (LIVE.pathFix) { fixFrom = new URL(LIVE.pathFix.from).href; fixTo = new URL(LIVE.pathFix.to).href; } } catch (e) { fixFrom = null; }
    var liveMove = function (url) {
      if (!fixFrom || !fixTo) return url;
      var abs;
      try { abs = new URL(String(url), document.baseURI).href; } catch (e) { return url; }
      if (abs.indexOf(fixFrom) !== 0) return url;
      var to = fixTo + abs.slice(fixFrom.length);
      window.__htdLiveMoved.push(to);
      return to;
    };
    try {
      var xOpen = XMLHttpRequest.prototype.open, xSend = XMLHttpRequest.prototype.send;
      XMLHttpRequest.prototype.open = function (method, url) {
        this.__htdLive = liveDoc(method, url);
        if (!this.__htdLive && arguments.length >= 2) {
          var args = Array.prototype.slice.call(arguments);
          args[1] = liveMove(url);
          return xOpen.apply(this, args);
        }
        return xOpen.apply(this, arguments);
      };
      XMLHttpRequest.prototype.send = function () {
        var d = this.__htdLive, x = this;
        if (!d) return xSend.apply(this, arguments);
        window.__htdLiveAnswered.push(d.name);
        var def = function (k, v) { try { Object.defineProperty(x, k, { value: v, configurable: true }); } catch (e) { /* ignore */ } };
        setTimeout(function () {
          def('readyState', 4); def('status', 200); def('statusText', 'OK'); def('responseText', d.body); def('response', d.body);
          // (the on* handlers run from these events, like a real answer)
          ['readystatechange', 'load', 'loadend'].forEach(function (t) { try { x.dispatchEvent(new Event(t)); } catch (e) { /* ignore */ } });
        }, 0);
      };
      var pageFetch = window.fetch;
      if (pageFetch) window.fetch = function (input, init) {
        var url = typeof input === 'string' ? input : (input && input.url) || '';
        var method = (init && init.method) || (input && typeof input === 'object' && input.method) || 'GET';
        var d = liveDoc(method, url);
        if (!d) {
          var moved = liveMove(url);
          return moved === url || typeof input !== 'string' ? pageFetch.apply(this, arguments) : pageFetch.call(this, moved, init);
        }
        window.__htdLiveAnswered.push(d.name);
        return Promise.resolve(new Response(d.body, { status: 200, headers: { 'Content-Type': 'application/json' } }));
      };
    } catch (e) { /* ignore */ }
  }

  /* 3. listener recording ---------------------------------------------------- */
  var REC = new WeakMap();
  var origAdd = EventTarget.prototype.addEventListener;
  var origRemove = EventTarget.prototype.removeEventListener;

  function stackFrames() {
    var s = '';
    try { s = new Error().stack || ''; } catch (e) { s = ''; }
    var lines = s.split('\n');
    var out = [];
    for (var i = 1; i < lines.length && out.length < 5; i++) {
      var l = lines[i];
      var m = /\(([^()]*?):(\d+):(\d+)\)\s*$/.exec(l) || /\bat\s+([^\s()]+?):(\d+):(\d+)\s*$/.exec(l);
      if (!m) continue;
      if (PROBE_URL && m[1] === PROBE_URL) continue;
      var fm = /^\s*at\s+(.*?)\s+\(/.exec(l);
      out.push({ url: m[1], line: +m[2], col: +m[3], fn: fm ? fm[1] : '' });
    }
    return out;
  }
  function capOf(opt) { return !!(opt === true || (opt && typeof opt === 'object' && opt.capture)); }
  function record(target, type, fn, opt, via) {
    if (!fn || !target) return;
    var list = REC.get(target);
    if (!list) { list = []; REC.set(target, list); }
    var cap = capOf(opt);
    if (via === 'add') {
      for (var i = 0; i < list.length; i++) {
        if (list[i].via === 'add' && list[i].type === type && list[i].fn === fn && list[i].cap === cap) return;
      }
    }
    list.push({ type: String(type), fn: fn, cap: cap, via: via, frames: stackFrames() });
  }
  EventTarget.prototype.addEventListener = function (type, fn, opt) {
    try { record(this, type, fn, opt, 'add'); } catch (e) { /* ignore */ }
    return origAdd.apply(this, arguments);
  };
  EventTarget.prototype.removeEventListener = function (type, fn, opt) {
    try {
      var list = REC.get(this);
      if (list) {
        var cap = capOf(opt);
        for (var i = 0; i < list.length; i++) {
          if (list[i].via === 'add' && list[i].type === String(type) && list[i].fn === fn && list[i].cap === cap) { list.splice(i, 1); break; }
        }
      }
    } catch (e) { /* ignore */ }
    return origRemove.apply(this, arguments);
  };

  var ON_PROPS = ['onclick', 'ondblclick', 'onmousedown', 'onmouseup', 'onmousemove', 'onmouseover',
    'onmouseout', 'onmouseenter', 'onmouseleave', 'onchange', 'oninput', 'onkeydown', 'onkeypress',
    'onkeyup', 'onfocus', 'onblur', 'oncontextmenu', 'onpointerdown', 'onpointerup', 'onsubmit',
    'onwheel', 'ontouchstart', 'ontouchend', 'onscroll', 'onload'];
  function patchProps(obj) {
    ON_PROPS.forEach(function (p) {
      var d = Object.getOwnPropertyDescriptor(obj, p);
      if (!d || !d.set || !d.get || !d.configurable) return;
      Object.defineProperty(obj, p, {
        configurable: true,
        enumerable: d.enumerable,
        get: function () { return d.get.call(this); },
        set: function (v) {
          try {
            var list = REC.get(this);
            var type = p.slice(2);
            if (list) for (var i = list.length - 1; i >= 0; i--) if (list[i].via === 'prop' && list[i].type === type) list.splice(i, 1);
            if (typeof v === 'function') record(this, type, v, false, 'prop');
          } catch (e) { /* ignore */ }
          return d.set.call(this, v);
        },
      });
    });
  }
  try { patchProps(HTMLElement.prototype); } catch (e) { /* ignore */ }
  try { patchProps(Document.prototype); } catch (e) { /* ignore */ }
  try { patchProps(SVGElement.prototype); } catch (e) { /* ignore */ }
  try { patchProps(window); } catch (e) { /* ignore */ }

  function fnInfo(fn) {
    var f = fn;
    if (f && typeof f === 'object' && typeof f.handleEvent === 'function') f = f.handleEvent;
    if (typeof f !== 'function') return { text: '', name: '', len: 0 };
    var t = '';
    try { t = Function.prototype.toString.call(f); } catch (e) { t = ''; }
    return { text: t.length > 6000 ? t.slice(0, 6000) : t, name: f.name || '', len: t.length };
  }
  function serList(list, types, max) {
    var out = [];
    if (!list) return out;
    for (var i = 0; i < list.length && out.length < max; i++) {
      var r = list[i];
      if (types && types.indexOf(r.type) < 0) continue;
      var f = fnInfo(r.fn);
      out.push({ type: r.type, cap: r.cap, via: r.via, fnText: f.text, fnName: f.name, fnLen: f.len, frames: r.frames });
    }
    return out;
  }

  /* 4. keys, component picking --------------------------------------------- */
  var keys = new WeakMap();
  var els = new Map();
  var nextKey = 1;
  function keyOf(el) {
    if (!el) return 0;
    var k = keys.get(el);
    if (!k) { k = nextKey++; keys.set(el, k); els.set(k, el); }
    return k;
  }
  var formRootEl;
  function formRoot() {
    if (formRootEl === undefined || (formRootEl && !formRootEl.isConnected)) {
      formRootEl = (document.body && document.body.querySelector(':scope > .form')) || null;
    }
    return formRootEl;
  }
  function idOf(el) {
    if (!el) return '';
    if (el.id) return el.id;
    if (isPane(el)) return paneName(el);
    return el === formRoot() ? '@form' : '';
  }
  /* AI(W906-HTDESIGNER) 20261001 (EastSun: the tree did not branch out a PageControl's sheets -- "Above Coveyor /
     Under Coveyor / Cassette / Manual Track"): a TTabSheet is <div class="pcPane" data-p="N" title="tsName"> with no
     id; its name and caption are on its tab (<div class="tab" data-t="N" title="tsName : TTabSheet">Caption</div>).
     WPF's Document Outline / BCB6's Object TreeView list each sheet under its PageControl, the sheet's controls
     under it -- so the tree does too, under the sheet's .dfm name. */
  function isPane(el) { return !!(el && el.classList && el.classList.contains('pcPane')); }
  function paneTab(el) {
    var wrap = el.parentElement && el.parentElement.parentElement;
    if (!wrap) return null;
    var tabs = wrap.querySelectorAll(':scope > .pcTabs > .tab');
    for (var j = 0; j < tabs.length; j++) if (tabs[j].getAttribute('data-t') === el.getAttribute('data-p')) return tabs[j];
    return null;
  }
  /* (the page moves title to data-htitle when it runs -- no tooltips -- so either one) */
  function titleAttr(el) { return (el && el.getAttribute && (el.getAttribute('title') || el.getAttribute('data-htitle'))) || ''; }
  function paneName(el) {
    var tab = paneTab(el);
    var m = /^\s*([A-Za-z_][\w]*)\s*:\s*TTabSheet/.exec(titleAttr(tab));
    if (m) return m[1];
    var t = String(titleAttr(el)).trim();
    return /^[A-Za-z_][\w]*$/.test(t) ? t : '';
  }
  function pick(t, exact) {
    var el = t && t.nodeType === 1 ? t : (t && t.parentElement) || null;
    if (!el || el === host) return null;
    if (el === document.documentElement || el === document.body) return null;
    if (exact) return el;
    var fr = formRoot();
    for (var x = el; x && x !== document.body && x !== document.documentElement; x = x.parentElement) {
      if (x.id || x === fr) return x;
    }
    return el;
  }
  /* Blend: "Select an object underneath another object: hold down Alt and click once for each layer of objects" --
     every element under the point (innermost / topmost first, the page's own layers too); the first Alt+click takes
     the top one (as Alt+click always did: the innermost element, with or without an id), each next one the one below
     the current selection, round again at the bottom */
  function altPick(x, y) {
    var hd = host ? host.style.display : '';
    if (host) host.style.display = 'none';
    var list = [];
    try { list = document.elementsFromPoint(x, y) || []; } catch (e) { list = []; }
    if (host) host.style.display = hd;
    var fr = formRoot();
    var stack = Array.prototype.filter.call(list, function (el) {
      return el && el !== host && el !== document.body && el !== document.documentElement && (!fr || el === fr || fr.contains(el));
    });
    if (!stack.length) return null;
    var i = selEl ? stack.indexOf(selEl) : -1;
    return i < 0 ? stack[0] : stack[(i + 1) % stack.length];
  }
  /* AI(W906-HTDESIGNER) 20261001: Blend's "Set Current Selection" -- the components under the last right-click, the
     top one first (Z order), the form last; only ones that can be selected by name (an id, or the form) */
  var lastCtx = null;
  function stackAt(x, y) {
    var hd = host ? host.style.display : '';
    if (host) host.style.display = 'none';
    var list = [];
    try { list = document.elementsFromPoint(x, y) || []; } catch (e) { list = []; }
    if (host) host.style.display = hd;
    var fr = formRoot(), seen = [], out = [];
    for (var i = 0; i < list.length; i++) {
      var c = pick(list[i], false);
      if (!c || c === host || seen.indexOf(c) >= 0 || (fr && c !== fr && !fr.contains(c))) continue;
      seen.push(c);
      var nm = c === fr ? '@form' : c.id;
      if (!nm) continue;
      out.push({ id: nm, cls: vclOf(c).cls || '', tag: c.tagName.toLowerCase(), sel: c === selEl });
    }
    if (fr && !out.some(function (o) { return o.id === '@form'; })) out.push({ id: '@form', cls: vclOf(fr).cls || '', tag: fr.tagName.toLowerCase(), sel: fr === selEl });
    return out;
  }
  function closestSafe(t, sel) {
    var el = t && t.nodeType === 1 ? t : (t && t.parentElement);
    try { return el ? el.closest(sel) : null; } catch (e) { return null; }
  }
  function isTab(t) { return !!closestSafe(t, '.pcTabs > .tab, .tabs > .tab, [role="tab"]'); }

  var CLS = {};   /* name -> VCL class, from the IR (sent with 'init') */
  var EVS = {};   /* name -> ["OnClick", ...], from the IR (sent with 'init') */
  function vclOf(el) {
    var t = (el && el.getAttribute && el.getAttribute('title')) || '';
    var m = /^\s*([A-Za-z_@][\w]*)\s*:\s*(T\w+)/.exec(t);
    if (m) return { name: m[1], cls: m[2] };
    var id = idOf(el);
    return { name: id, cls: (id && Object.prototype.hasOwnProperty.call(CLS, id)) ? CLS[id] : '' };
  }
  function clip(s, n) {
    s = String(s || '').replace(/\s+/g, ' ').trim();
    return s.length > n ? s.slice(0, n) + '…' : s;
  }
  function caption(el) {
    try {
      var tn = el.tagName;
      if (tn === 'FIELDSET') { var lg = el.querySelector(':scope > legend'); return lg ? clip(lg.textContent, 40) : ''; }
      if (tn === 'INPUT' || tn === 'SELECT' || tn === 'TEXTAREA' || tn === 'IMG') return '';
      if (el.childElementCount > 4) { var cp = el.querySelector(':scope > .pnlCap'); return cp ? clip(cp.textContent, 40) : ''; }
      return clip(el.textContent, 40);
    } catch (e) { return ''; }
  }
  function hidden(el) {
    return !(el.offsetWidth || el.offsetHeight || el.getClientRects().length);
  }
  /* the tree's 「隱藏」: hidden by itself or by a container up to its tab sheet -- not just because its sheet is not
     the active tab (BCB6 / WPF do not call a control on another tab hidden) */
  function treeHidden(el) {
    if (!hidden(el)) return false;
    for (var x = el; x && x !== document.body; x = x.parentElement) {
      if (isPane(x)) return false;
      var cs = getComputedStyle(x);
      if (cs.display === 'none' || cs.visibility === 'hidden') return true;
    }
    return true;
  }

  /* 5. overlay (shadow root, custom tag so page selectors never match it). Open, so
        the window-level handler can tell which resize handle was pressed. */
  var host = null, sh = null, bH, bS, tH, tS, badge, handles = [], gX = null, gY = null, gpX = null, gpY = null, extraBoxes = [], grEl = null, mqEl = null;
  var mgX = null, mgY = null, mgTX = null, mgTY = null, mgX2 = null, mgY2 = null, mgTX2 = null, mgTY2 = null;   /* 0.161 + right / bottom */   /* the margin adorners (WPF): lines + numbers to the container's left / top */
  /* WPF rubber-band selection: a drag that starts on the form's own background */
  var marquee = null;
  function drawMarquee() {
    if (!mqEl || !marquee) return;
    if (!marquee.on) { mqEl.style.display = 'none'; return; }
    var L = Math.min(marquee.x0, marquee.x1), T = Math.min(marquee.y0, marquee.y1);
    mqEl.style.display = 'block';
    mqEl.style.left = L + 'px'; mqEl.style.top = T + 'px';
    mqEl.style.width = Math.abs(marquee.x1 - marquee.x0) + 'px'; mqEl.style.height = Math.abs(marquee.y1 - marquee.y0) + 'px';
  }
  /* the components entirely inside the band -- the outermost of them -- become the selection */
  function finishMarquee() {
    var m = marquee;
    marquee = null;
    if (mqEl) mqEl.style.display = 'none';
    if (m && !m.on && m.shiftEl) { toggleSel(m.shiftEl); scheduleDraw(); return 0; }   // (a Shift+click, no drag)
    if (!m || !m.on) { scheduleDraw(); return 0; }
    justDragged = true;
    setTimeout(function () { justDragged = false; }, 0);
    var L = Math.min(m.x0, m.x1), R = Math.max(m.x0, m.x1), T = Math.min(m.y0, m.y1), B = Math.max(m.y0, m.y1);
    var fr = formRoot();
    var inside = allComps().filter(function (x) {
      if (x === fr || !x.getClientRects().length || (x.closest && x.closest('[data-htd-hide]'))) return false;
      var r = x.getBoundingClientRect();
      if (!r.width && !r.height) return false;
      return r.left >= L && r.right <= R && r.top >= T && r.bottom <= B;
    });
    var outer = inside.filter(function (x) { return !inside.some(function (o) { return o !== x && o.contains(x); }); });
    if (!outer.length) { scheduleDraw(); return 0; }
    multi = outer.slice(1);
    selEl = null;
    select(outer[0], 'multi', false, true);
    return outer.length;
  }
  /* WPF / Blend's hand: hold Space and drag = pan the view; a middle-button drag too. While
     Space is held a shield covers the page, so nothing below is picked, moved or clicked. */
  var PAN = { space: false, drag: null }, panEl = null;
  /* Blend: Ctrl+Space held = a click zooms in there, Ctrl+Alt+Space = zooms out (ZSP.out); the same shield as the hand */
  var ZSP = null;
  function panShield(on) {
    if (!panEl) return;
    panEl.style.display = on || ZSP ? 'block' : 'none';
    panEl.className = ZSP ? (ZSP.out ? 'pan zout' : 'pan zin') : PAN.drag ? 'pan on' : 'pan';
  }
  /* one zoom step in (dir 1) / out (-1), the page point under (x, y) kept under it */
  function zoomAt(x, y, dir) {
    var z0 = zoom, z = z0, i;
    if (dir > 0) { for (i = 0; i < ZOOMS.length; i++) if (ZOOMS[i] > z0 + 1e-6) { z = ZOOMS[i]; break; } }
    else { for (i = ZOOMS.length - 1; i >= 0; i--) if (ZOOMS[i] < z0 - 1e-6) { z = ZOOMS[i]; break; } }
    var px = (x + window.scrollX) / z0, py = (y + window.scrollY) / z0;
    setZoom(z);
    try { window.scrollTo(Math.max(0, px * z - x), Math.max(0, py * z - y)); } catch (e) { /* ignore */ }
    return z;
  }
  function startPan(e) {
    PAN.drag = { x: e.clientX, y: e.clientY, sx: window.scrollX, sy: window.scrollY, moved: false };
    panShield(true);
  }
  function movePan(e) {
    var p = PAN.drag, dx = e.clientX - p.x, dy = e.clientY - p.y;
    if (Math.abs(dx) + Math.abs(dy) >= 2) p.moved = true;
    window.scrollTo(p.sx - dx, p.sy - dy);
  }
  function endPan() {
    var p = PAN.drag;
    PAN.drag = null;
    if (p && p.moved) { justDragged = true; setTimeout(function () { justDragged = false; }, 0); }
    panShield(PAN.space);
  }
  /* 工具箱放置 (WinForms / WPF): a tool picked in the toolbox -- a crosshair over the page; a
     click puts it there, a drag gives its size too; Esc / a right-click gives up. The host
     is told the id'd elements under the point (innermost first, the form last), each with
     the point in its own child coordinates, and picks the first container. */
  var PLACE = null, plEl = null;
  /* 0.156 (the WPF gap list G7; WinForms View > Tab Order): Tab order by clicks -- each click numbers the component
     under it (tabindex = next, then next + 1); Esc / right-click = done. null = off */
  var TABSET = null;
  function tabSetClick(el) {
    if (!el || el === formRoot()) return;
    var n = TABSET.next;
    var ch = lookChange(el, 'tabOrder', n);
    if (!ch) return;
    TABSET.next = n + 1;
    TABSET.done.push(idOf(el) || '?');
    sendEdit(el, ch);
    tabCache = null;
    drawBadge();
    scheduleDraw();
  }
  function endTabSet(tell) {
    if (!TABSET) return;
    var done = TABSET.done;
    TABSET = null;
    drawBadge();
    scheduleDraw();
    if (tell) post({ type: 'tabOrderSetEnd', done: done });
  }
  function placeShield(on) { if (plEl) plEl.style.display = on ? 'block' : 'none'; }
  function placeTargets(x, y, skip) {
    /* the whole overlay out of the way while looking: a resize handle under the point is ours, not the page;
       skip = nodes being dragged (they are under the pointer themselves) */
    var hd = host ? host.style.display : '';
    if (host) host.style.display = 'none';
    var vis = (skip || []).map(function (n) { var v = n.style.visibility; n.style.visibility = 'hidden'; return v; });
    var under = document.elementFromPoint(x, y);
    (skip || []).forEach(function (n, i) { n.style.visibility = vis[i]; });
    if (host) host.style.display = hd;
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var fr = formRoot(), out = [];
    for (var e = under; e && e !== document.body && e !== document.documentElement; e = e.parentElement) {
      if (e === host) break;
      /* a tab sheet: by its .dfm name now (0.135, WPF: a control put on a TabItem goes into it); none = as before */
      if (!e.id && isPane(e)) {
        var pn = paneName(e);
        if (!pn) { out.push({ id: null, pane: true }); continue; }
        var rp = e.getBoundingClientRect();
        out.push({ id: pn, pane: true, x: Math.round((x - rp.left) / z - e.clientLeft), y: Math.round((y - rp.top) / z - e.clientTop) });
        continue;
      }
      var nm = e === fr ? '@form' : e.id;
      if (!nm) continue;
      /* a GroupBox keeps its children in <div class="cli">: that is where their left/top count from */
      var org = (e.querySelector && e.querySelector(':scope > div.cli')) || e;
      var r = org.getBoundingClientRect();
      out.push({ id: nm, x: Math.round((x - r.left) / z - org.clientLeft), y: Math.round((y - r.top) / z - org.clientTop) });
      if (e === fr) break;
    }
    if (fr && !out.some(function (o) { return o.id === '@form'; })) {
      var rf = fr.getBoundingClientRect();
      out.push({ id: '@form', x: Math.round((x - rf.left) / z - fr.clientLeft), y: Math.round((y - rf.top) / z - fr.clientTop) });
    }
    return out;
  }
  function placeBand() {
    if (!mqEl) return;
    var p = PLACE && PLACE.drag;
    if (!p || Math.abs(p.x1 - p.x0) + Math.abs(p.y1 - p.y0) < 4) { mqEl.style.display = 'none'; return; }
    mqEl.style.display = 'block';
    mqEl.style.left = Math.min(p.x0, p.x1) + 'px'; mqEl.style.top = Math.min(p.y0, p.y1) + 'px';
    mqEl.style.width = Math.abs(p.x1 - p.x0) + 'px'; mqEl.style.height = Math.abs(p.y1 - p.y0) + 'px';
  }
  function finishPlace() {
    var pl = PLACE, p = pl && pl.drag;
    PLACE = null;
    placeShield(false);
    if (mqEl) mqEl.style.display = 'none';
    drawBadge();
    if (!p) return;
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var x = Math.min(p.x0, p.x1), y = Math.min(p.y0, p.y1);
    var w = Math.abs(p.x1 - p.x0) / z, h = Math.abs(p.y1 - p.y0) / z;
    var sized = w >= 4 && h >= 4;
    justDragged = true;
    setTimeout(function () { justDragged = false; }, 0);
    var tgs = placeTargets(x, y);
    /* WPF: "If snapping to gridlines is enabled, an element tends to align with the closest gridlines when you drag it
       onto the artboard" -- the point in each container, and the size (AI 20261001: it landed at 37,53) */
    if (GRID.on) {
      tgs.forEach(function (o) { if (typeof o.x === 'number') { o.x = snapG(o.x); o.y = snapG(o.y); } });
      if (sized) { w = Math.max(GRID.size, snapG(w)); h = Math.max(GRID.size, snapG(h)); }
    }
    post({ type: 'place', cls: pl.cls, targets: tgs, w: sized ? Math.round(w) : 0, h: sized ? Math.round(h) : 0 });
  }
  function cancelPlace() {
    if (!PLACE) return;
    PLACE = null;
    placeShield(false);
    if (mqEl) mqEl.style.display = 'none';
    drawBadge();
    post({ type: 'placeCancel' });
  }
  /* WPF "snap to gridlines": positions and sizes land on multiples of GRID.size (Alt = free) */
  /* on = snapping to it, show = drawn (WPF: two buttons, "Show/Hide snap grid" and "snapping to gridlines") */
  var GRID = { on: false, show: false, size: 8 };
  function gridOf(g) {
    var on = !!g.on;
    return { on: on, show: typeof g.show === 'boolean' ? g.show : on, size: Math.max(2, Math.min(64, Math.round(+g.size || 8))) };
  }
  function snapG(v) { return Math.round(v / GRID.size) * GRID.size; }
  /* 0.155 (the WPF gap list G8: d:DesignWidth / DesignHeight, Blend's device frame): the machine's screen drawn from the
     form's top left -- what falls outside it is off the screen on the machine. null = not drawn */
  var SCREEN = null;
  var scEl = null, scTag = null;
  function screenOf(v) { return v && +v.w > 0 && +v.h > 0 ? { w: Math.round(+v.w), h: Math.round(+v.h) } : null; }
  function placeScreen() {
    if (!scEl) return;
    var f = SCREEN && mode === 'design' ? formRoot() : null;
    if (!f) { scEl.style.display = 'none'; scTag.style.display = 'none'; return; }
    var r = f.getBoundingClientRect();
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    scEl.style.display = 'block';
    scEl.style.left = r.left + 'px'; scEl.style.top = r.top + 'px';
    scEl.style.width = (SCREEN.w * z) + 'px'; scEl.style.height = (SCREEN.h * z) + 'px';
    scTag.style.display = 'block';
    scTag.textContent = '機台螢幕 ' + SCREEN.w + ' × ' + SCREEN.h;
    scTag.style.left = (r.left + SCREEN.w * z - 2) + 'px'; scTag.style.top = (r.top + SCREEN.h * z + 2) + 'px';
  }
  /* the grid over the container the selection moves in (else the form), aligned to its origin */
  function placeGrid() {
    if (!grEl) return;
    if (!GRID.show || mode !== 'design') { grEl.style.display = 'none'; return; }
    var bx = selEl ? boxOf(selEl) : null;
    var c = bx && !bx.root ? bx.node.offsetParent : null;
    if (!c || c === document.body || c === document.documentElement) c = formRoot();
    if (!c) { grEl.style.display = 'none'; return; }
    var r = c.getBoundingClientRect();
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var step = GRID.size * z;
    grEl.style.display = 'block';
    grEl.style.left = (r.left + c.clientLeft * z) + 'px';
    grEl.style.top = (r.top + c.clientTop * z) + 'px';
    grEl.style.width = Math.max(0, c.clientWidth * z) + 'px';
    grEl.style.height = Math.max(0, c.clientHeight * z) + 'px';
    grEl.style.backgroundSize = step + 'px ' + step + 'px';
    grEl.style.backgroundPosition = (-step / 2) + 'px ' + (-step / 2) + 'px';
  }
  var hoverEl = null, selEl = null;
  var multi = [];   /* the other selected components (WPF multi-select); selEl is the primary */
  function allSel() {
    var out = selEl ? [selEl] : [];
    for (var i = 0; i < multi.length; i++) if (multi[i].isConnected && multi[i] !== selEl) out.push(multi[i]);
    return out;
  }
  var DIRS = ['nw', 'n', 'ne', 'e', 'se', 's', 'sw', 'w'];
  function ensureOverlay() {
    if (host || !document.documentElement) return;
    host = document.createElement('htd-overlay');
    host.setAttribute('style', 'all:initial;position:fixed;left:0;top:0;width:0;height:0;z-index:2147483647;pointer-events:none;');
    sh = host.attachShadow({ mode: 'open' });
    var hs = '';
    for (var i = 0; i < DIRS.length; i++) hs += '<div class="hd" data-dir="' + DIRS[i] + '"></div>';
    sh.innerHTML = '<style>' +
      '.b{position:fixed;display:none;box-sizing:border-box;pointer-events:none;}' +
      '.h{outline:1px dashed #1e90ff;background:rgba(30,144,255,.08);}' +
      '.s{outline:2px solid #ff8c00;background:rgba(255,140,0,.10);}' +
      '.t{position:fixed;display:none;pointer-events:none;font:11px/16px Consolas,"Courier New",monospace;' +
      'padding:0 5px;border-radius:2px;white-space:nowrap;color:#fff;}' +
      '.th{background:#1e5fa8;} .ts{background:#b35c00;}' +
      '.m{position:fixed;right:6px;bottom:6px;pointer-events:none;font:11px/18px "Microsoft JhengHei",sans-serif;' +
      'background:rgba(0,0,0,.66);color:#fff;padding:0 8px;border-radius:3px;}' +
      '.hd{position:fixed;display:none;width:7px;height:7px;margin:-4px 0 0 -4px;box-sizing:border-box;' +
      'background:#fff;border:1px solid #b35c00;pointer-events:auto;}' +
      '.hd[data-dir=nw],.hd[data-dir=se]{cursor:nwse-resize;} .hd[data-dir=ne],.hd[data-dir=sw]{cursor:nesw-resize;}' +
      '.hd[data-dir=n],.hd[data-dir=s]{cursor:ns-resize;} .hd[data-dir=e],.hd[data-dir=w]{cursor:ew-resize;}' +
      '.g{position:fixed;display:none;pointer-events:none;background:#e5172f;}' +
      '.g.bl{background:none;border-top:1px dashed #e5172f;}' +
      '.gp{position:fixed;display:none;pointer-events:none;background:#1e90ff;box-shadow:0 0 0 1px rgba(255,255,255,.7);}' +
      '.s2{outline:1px solid #ff8c00;background:rgba(255,140,0,.06);}' +
      '.gr{position:fixed;display:none;pointer-events:none;background-image:radial-gradient(circle,rgba(40,40,40,.5) .8px,transparent 1.2px);}' +
      '.mq{position:fixed;display:none;pointer-events:none;box-sizing:border-box;border:1px dashed #1e90ff;background:rgba(30,144,255,.10);}' +
      '.wm{position:fixed;display:none;pointer-events:none;box-sizing:border-box;border:2px solid;}' +
      '.nt{position:fixed;display:none;pointer-events:none;font:10px/12px Consolas,"Courier New",monospace;padding:0 2px;white-space:nowrap;' +
      'background:rgba(255,255,215,.92);color:#333;border:1px solid rgba(0,0,0,.25);border-radius:2px;}' +
      '.gh{position:fixed;display:none;pointer-events:none;box-sizing:border-box;border:1px dashed rgba(192,38,211,.9);background:rgba(192,38,211,.05);}' +
      '.gh.sel{border:2px dashed #c026d3;background:rgba(192,38,211,.10);}' +
      '.pan{position:fixed;left:0;top:0;right:0;bottom:0;display:none;pointer-events:auto;cursor:grab;} .pan.on{cursor:grabbing;}' +
      '.pan.zin{cursor:zoom-in;} .pan.zout{cursor:zoom-out;}' +
      '.plc{position:fixed;left:0;top:0;right:0;bottom:0;display:none;pointer-events:auto;cursor:crosshair;}' +
      '.tb{position:fixed;display:none;pointer-events:none;font:bold 10px/14px Consolas,"Courier New",monospace;padding:0 4px;border-radius:7px;' +
      'white-space:nowrap;color:#fff;background:#1e5fa8;box-shadow:0 0 0 1px rgba(255,255,255,.8);} .tb.bad{background:#d11a2a;} .tb.x{background:#808080;}' +
      '.ght{position:fixed;display:none;pointer-events:none;font:10px/14px Consolas,"Courier New",monospace;padding:0 4px;white-space:nowrap;' +
      'background:#a21caf;color:#fff;border-radius:2px;}' +
      '.wm.ok{border-color:rgba(46,160,67,.85);} .wm.mid{border-color:rgba(214,140,20,.95);} .wm.bad{border-color:rgba(229,23,47,.95);background:rgba(229,23,47,.07);}' +
      /* WPF's artboard toolbar (bottom left): zoom, fit, grid, snaplines */
      '.atb{position:fixed;left:6px;bottom:6px;display:none;pointer-events:auto;font:12px/20px "Microsoft JhengHei",sans-serif;' +
      'background:rgba(30,30,30,.86);color:#fff;border-radius:3px;padding:1px 2px;white-space:nowrap;user-select:none;}' +
      '.atb b{display:inline-block;min-width:18px;padding:0 5px;margin:0 1px;border-radius:2px;cursor:pointer;font-weight:normal;text-align:center;}' +
      '.atb b:hover{background:rgba(255,255,255,.2);} .atb b.on{background:#1e5fa8;} .atb i{display:inline-block;width:1px;height:14px;margin:0 3px;vertical-align:-2px;background:rgba(255,255,255,.3);}' +
      '.atm{position:fixed;left:6px;bottom:30px;display:none;pointer-events:auto;font:12px/22px "Microsoft JhengHei",sans-serif;' +
      'background:rgba(30,30,30,.94);color:#fff;border-radius:3px;padding:3px 0;min-width:120px;user-select:none;}' +
      '.atm b{display:block;padding:0 12px;cursor:pointer;font-weight:normal;} .atm b:hover{background:#1e5fa8;} .atm b.on{color:#9cdcfe;}' +
      /* WPF's information bar (top of Design view): the page's problems -- here its JS errors */
      '.ib{position:fixed;left:0;right:0;top:0;display:none;pointer-events:auto;font:12px/22px "Microsoft JhengHei",sans-serif;' +
      'background:#fff4ce;color:#3b2f00;border-bottom:1px solid #d9b300;padding:0 8px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;}' +
      '.ib b{cursor:pointer;font-weight:normal;text-decoration:underline;margin-left:10px;} .ib b.x{text-decoration:none;float:right;margin-left:12px;}' +
      /* F2: the control's text edited right on it (WPF's Edit control text) */
      '.txe{position:fixed;box-sizing:border-box;margin:0;padding:0 2px;pointer-events:auto;border:1px solid #b35c00;' +
      'outline:2px solid rgba(179,92,0,.35);background:#fffbe6;color:#000;}' +
      /* WPF's margin adorners: the selected one's distance to its container's left / top edge */
      '.mg{position:fixed;display:none;pointer-events:none;background:rgba(179,92,0,.75);}' +
      '.mgt{position:fixed;display:none;pointer-events:none;font:10px/13px Consolas,"Courier New",monospace;padding:0 3px;white-space:nowrap;' +
      'color:#fff;background:rgba(179,92,0,.88);border-radius:2px;}' +
      /* 0.155 the machine's screen frame */
      '.scr{position:fixed;display:none;pointer-events:none;box-sizing:border-box;border:2px dashed rgba(229,23,47,.85);}' +
      '.scrt{position:fixed;display:none;pointer-events:none;transform:translateX(-100%);font:11px/15px "Microsoft JhengHei",sans-serif;padding:0 4px;color:#fff;background:rgba(229,23,47,.85);border-radius:2px;white-space:nowrap;}' +
      '</style><div class="mg mgx"></div><div class="mg mgy"></div><div class="mgt mgtx"></div><div class="mgt mgty"></div><div class="mg mgx2"></div><div class="mg mgy2"></div><div class="mgt mgtx2"></div><div class="mgt mgty2"></div><div class="ib"></div><div class="atb"></div><div class="atm"></div><div class="gr"></div><div class="scr"></div><div class="scrt"></div><div class="b h"></div><div class="b s"></div><div class="t th"></div><div class="t ts"></div><div class="m"></div><div class="mq"></div>' +
      '<div class="g gx"></div><div class="g gy"></div><div class="gp gpx"></div><div class="gp gpy"></div><div class="ght"></div>' + hs + '<div class="pan"></div><div class="plc"></div>';
    gX = sh.querySelector('.gx'); gY = sh.querySelector('.gy');
    gpX = sh.querySelector('.gpx'); gpY = sh.querySelector('.gpy');
    mgX = sh.querySelector('.mgx'); mgY = sh.querySelector('.mgy'); mgTX = sh.querySelector('.mgtx'); mgTY = sh.querySelector('.mgty');
    mgX2 = sh.querySelector('.mgx2'); mgY2 = sh.querySelector('.mgy2'); mgTX2 = sh.querySelector('.mgtx2'); mgTY2 = sh.querySelector('.mgty2');
    ghTag = sh.querySelector('.ght');
    panEl = sh.querySelector('.pan');
    plEl = sh.querySelector('.plc');
    grEl = sh.querySelector('.gr');
    scEl = sh.querySelector('.scr'); scTag = sh.querySelector('.scrt');
    mqEl = sh.querySelector('.mq');
    bH = sh.querySelector('.h'); bS = sh.querySelector('.s');
    tH = sh.querySelector('.th'); tS = sh.querySelector('.ts');
    badge = sh.querySelector('.m');
    atbEl = sh.querySelector('.atb');
    atmEl = sh.querySelector('.atm');
    ibEl = sh.querySelector('.ib');
    drawInfoBar();
    handles = Array.prototype.slice.call(sh.querySelectorAll('.hd'));
    document.documentElement.appendChild(host);
    drawBadge();
  }
  /* WPF's artboard toolbar: zoom out / the zoom list / in, fit all, fit selection, the snap grid, snaplines.
     Built with textContent; its clicks are ours (handled with the overlay's other events, never the page's). */
  var atbEl, atmEl;          // (no initialiser: the overlay may be built before this line runs)
  var SNAPL = true;
  /* WPF's "Toggle artboard background": the page's own background <-> a dark one around the form (the design
     surface only: the page's html / body get it inline, their own values come back when it is off) */
  var ABG = false, ABG0 = null, ABGWANT = false;
  function setArtboard(on) {
    on = !!on;
    if (on === ABG) return;
    var h = document.documentElement, b = document.body;
    if (on) {
      ABG0 = { h: h.style.getPropertyValue('background'), hp: h.style.getPropertyPriority('background'),
        b: b ? b.style.getPropertyValue('background') : '', bp: b ? b.style.getPropertyPriority('background') : '' };
      h.style.setProperty('background', '#2d2d30', 'important');
      if (b) b.style.setProperty('background', '#2d2d30', 'important');
    } else if (ABG0) {
      if (ABG0.h) h.style.setProperty('background', ABG0.h, ABG0.hp); else h.style.removeProperty('background');
      if (b) { if (ABG0.b) b.style.setProperty('background', ABG0.b, ABG0.bp); else b.style.removeProperty('background'); }
      ABG0 = null;
    }
    ABG = on;
  }
  /* 1005 (WPF audit: Visual Studio / Blend's "show element bounds"): every component's box outlined (dashed, an outline --
     nothing moves, nothing is written; the source is untouched). Remembered for every page (workspace state), off while
     operating. Components with no border of their own (a Label, an empty Panel) can be seen and hit. */
  var BOUNDS = false, BOUNDSWANT = false;
  function setBounds(on) {
    on = !!on;
    if (on && !document.getElementById('__htd_bounds_css')) {
      var st = document.createElement('style');
      st.id = '__htd_bounds_css';
      st.textContent = 'html[data-htd-bounds] body [id]:not([id^="__htd"]):not(script):not(style):not(link):not(meta):not(template)' +
        '{outline:1px dashed rgba(0,150,255,.65)!important;outline-offset:-1px!important}';
      (document.head || document.documentElement).appendChild(st);
    }
    if (on) document.documentElement.setAttribute('data-htd-bounds', '1'); else document.documentElement.removeAttribute('data-htd-bounds');
    BOUNDS = on;
  }
  /* WPF's information bar: the page's JS errors (the first one's text; all of them in the output panel) */
  var ibEl, ibErrs, ibClosedAt;
  function drawInfoBar() {
    if (!ibEl) return;
    var n = ibErrs ? ibErrs.length : 0;
    if (!n || (ibClosedAt || 0) >= n) { ibEl.style.display = 'none'; return; }
    ibEl.textContent = '';
    var x = document.createElement('b');
    x.className = 'x'; x.setAttribute('data-atb', 'errorsClose'); x.textContent = '×'; x.title = '關掉（有新的錯誤會再出現）';
    ibEl.appendChild(x);
    ibEl.appendChild(document.createTextNode('⚠ 這一頁有 ' + n + ' 個 JS 錯誤：' + ibErrs[0].msg + (ibErrs[0].src ? '（' + String(ibErrs[0].src).split('/').pop() + ':' + ibErrs[0].line + '）' : '')));
    var all = document.createElement('b');
    all.setAttribute('data-atb', 'errors'); all.textContent = '看全部';
    ibEl.appendChild(all);
    ibEl.title = ibErrs.map(function (e) { return e.msg; }).join('\n');
    ibEl.style.display = 'block';
  }
  function drawToolbar() {
    if (!atbEl) return;
    if (mode !== 'design') { atbEl.style.display = 'none'; atmEl.style.display = 'none'; return; }
    atbEl.textContent = '';
    var add = function (act, text, tip, on) {
      var b = document.createElement('b');
      b.setAttribute('data-atb', act);
      b.textContent = text;
      b.title = tip;
      if (on) b.className = 'on';
      atbEl.appendChild(b);
    };
    var sep = function () { atbEl.appendChild(document.createElement('i')); };
    add('zoomOut', '－', '縮小（' + wheelKeyText() + '、Ctrl+-；最小 12.5%）');
    add('zoomMenu', pct(zoom), '縮放比例（WPF 的 Zoom，12.5%～800%）：點一下選');
    add('zoomIn', '＋', '放大（' + wheelKeyText() + '、Ctrl+=；最大 800%）');
    sep();
    add('fitAll', '全部', '符合視窗：整個表單放進畫面（Fit all）');
    add('fitSel', '選取', '縮放到選取的元件（Fit selection）');
    sep();
    add('grid', '格線', '顯示／隱藏格線（WPF 的 Show/Hide snap grid）', GRID.show);
    add('gsnap', '吸附', '拖曳、改大小時對齊格線（WPF 的 snapping to gridlines；按住 Alt 暫時不吸）', GRID.on);
    add('snap', '對齊線', '拖曳時跟其他元件的邊緣、中心對齊（snaplines）', SNAPL);
    add('screen', '螢幕', '機台螢幕大小的框線（WPF 的 DesignWidth／DesignHeight）：框外的元件在機台上看不到；按一下選大小', !!SCREEN);
    add('bounds', '邊界', '顯示所有元件的邊界（Visual Studio／Blend 的 Show element bounds）：每個元件畫一圈虛線，沒有框的標籤、空面板也看得到、點得到；只影響設計畫面，原始碼沒改', BOUNDS);
    add('artboard', '背景', '切換畫面背景（WPF 的 Toggle artboard background）：頁面自己的 ↔ 暗色；只影響設計畫面，原始碼沒改', ABG);
    atbEl.style.display = 'block';
  }
  function zoomMenu(open) {
    if (!atmEl) return;
    if (!open) { atmEl.style.display = 'none'; return; }
    atmEl.textContent = '';
    var item = function (act, text, on) {
      var b = document.createElement('b');
      b.setAttribute('data-atb', act);
      b.textContent = text;
      if (on) b.className = 'on';
      atmEl.appendChild(b);
    };
    [0.125, 0.25, 0.5, 0.75, 1, 1.25, 1.5, 2, 3, 4, 6, 8].forEach(function (z) { item('z:' + z, pct(z), Math.abs(z - zoom) < 1e-6); });
    item('fitAll', '符合視窗（全部）', false);
    item('fitSel', '符合選取的元件', false);
    atmEl.style.display = 'block';
  }
  function toolbarAct(act) {
    var i, z;
    /* the information bar */
    if (act === 'errors') { post({ type: 'showErrors' }); return; }
    if (act === 'errorsClose') { ibClosedAt = ibErrs ? ibErrs.length : 0; drawInfoBar(); return; }
    if (act !== 'zoomMenu') zoomMenu(false);
    if (act === 'zoomMenu') { zoomMenu(atmEl.style.display !== 'block'); return; }
    if (act === 'zoomOut' || act === 'zoomIn') {
      z = zoom;
      if (act === 'zoomIn') { for (i = 0; i < ZOOMS.length; i++) if (ZOOMS[i] > zoom + 1e-6) { z = ZOOMS[i]; break; } }
      else { for (i = ZOOMS.length - 1; i >= 0; i--) if (ZOOMS[i] < zoom - 1e-6) { z = ZOOMS[i]; break; } }
      setZoom(z);
    } else if (/^z:/.test(act)) setZoom(parseFloat(act.slice(2)) || 1);
    else if (act === 'fitAll') zoomFit();
    else if (act === 'fitSel') { if (selEl) zoomToSel(); else zoomFit(); }
    else if (act === 'grid') post({ type: 'toolbar', cmd: 'gridShow' });
    else if (act === 'screen') post({ type: 'toolbar', cmd: 'screen' });
    else if (act === 'gsnap') post({ type: 'toolbar', cmd: 'gridSnap' });
    else if (act === 'snap') { SNAPL = !SNAPL; post({ type: 'toolbar', cmd: 'snap', on: SNAPL }); }
    else if (act === 'bounds') { BOUNDSWANT = !BOUNDS; setBounds(BOUNDSWANT); post({ type: 'toolbar', cmd: 'bounds', on: BOUNDS }); }
    else if (act === 'artboard') { ABGWANT = !ABG; setArtboard(ABGWANT); post({ type: 'toolbar', cmd: 'artboard', dark: ABG }); }
    drawToolbar();
  }
  function placeHandles() {
    /* a locked component shows no resize handles (WPF) */
    var bx = mode === 'design' && selEl && !drag && !lockedOf(selEl) ? boxOf(selEl) : null;
    var r = bx ? bx.node.getBoundingClientRect() : null;
    for (var i = 0; i < handles.length; i++) {
      var h = handles[i];
      if (!r || (!r.width && !r.height)) { h.style.display = 'none'; continue; }
      var d = h.getAttribute('data-dir');
      if (bx.root && !/^(e|s|se)$/.test(d)) { h.style.display = 'none'; continue; }   // the form: size only
      var x = d.indexOf('w') >= 0 ? r.left : d.indexOf('e') >= 0 ? r.right : r.left + r.width / 2;
      var y = d.indexOf('n') >= 0 ? r.top : d.indexOf('s') >= 0 ? r.bottom : r.top + r.height / 2;
      h.style.display = 'block';
      h.style.left = x + 'px';
      h.style.top = y + 'px';
    }
  }
  function labelOf(el, r) {
    var v = vclOf(el);
    var id = idOf(el);
    var name = id || el.tagName.toLowerCase();
    var ev = id && Object.prototype.hasOwnProperty.call(EVS, id) ? EVS[id] : null;
    var evs = ev && ev.length ? '  · ' + ev.slice(0, 3).join(', ') + (ev.length > 3 ? ' …' : '') : '';
    var n = el === selEl ? allSel().length : 0;
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;   // the page's own px, not the zoomed screen's
    if (drag && drag.active && drag.el === el && drag.bx) {
      /* while it moves / resizes: the numbers going into its style (WPF's live tip) */
      var st = drag.bx.node.style;
      var num = function (v) { var p = pxOf(v); return p === null ? '—' : String(Math.round(p)); };
      return name + '   ' + (drag.how === 'move' ? 'left ' + num(st.left) + '　top ' + num(st.top)
        : Math.round(r.width / z) + ' × ' + Math.round(r.height / z) + (/[wn]/.test(drag.how) ? '　left ' + num(st.left) + '　top ' + num(st.top) : ''));
    }
    return (lockedOf(el) ? '🔒 ' : '') + name + (v.cls ? ' : ' + v.cls : '') + evs + '   ' + Math.round(r.width / z) + '×' + Math.round(r.height / z) + (n > 1 ? '   （共選 ' + n + ' 個）' : '');
  }
  function place(box, tag, el) {
    if (!el || !el.isConnected) { box.style.display = 'none'; tag.style.display = 'none'; return; }
    var r = el.getBoundingClientRect();
    if (!r.width && !r.height) { box.style.display = 'none'; tag.style.display = 'none'; return; }
    box.style.display = 'block';
    box.style.left = r.left + 'px'; box.style.top = r.top + 'px';
    box.style.width = r.width + 'px'; box.style.height = r.height + 'px';
    tag.textContent = labelOf(el, r);
    tag.style.display = 'block';
    tag.style.left = Math.max(0, r.left) + 'px';
    tag.style.top = (r.top >= 17 ? r.top - 17 : r.bottom + 1) + 'px';
  }
  var drawPending = false;
  var ADORN = true;   /* F9 */
  function draw() {
    drawPending = false;
    if (!host) return;
    placeGrid();
    placeScreen();
    placeGhosts();
    placeWireMarks();
    placeNameTags();
    placeTabOrder();
    place(bS, tS, selEl);
    if (mode === 'design' && hoverEl && hoverEl !== selEl && !drag) place(bH, tH, hoverEl);
    else { bH.style.display = 'none'; tH.style.display = 'none'; }
    placeHandles();
    placeMargins();
    /* F9 (WPF: show / hide element handles): the selection's boxes, its tag and the handles go; clicks still select */
    if (!ADORN) {
      [bS, tS, bH, tH, gX, gY, gpX, gpY, mgX, mgY, mgTX, mgTY, mgX2, mgY2, mgTX2, mgTY2].forEach(function (x) { if (x) x.style.display = 'none'; });
      for (var ah = 0; ah < handles.length; ah++) handles[ah].style.display = 'none';
      for (var ab = 0; ab < extraBoxes.length; ab++) extraBoxes[ab].style.display = 'none';
      return;
    }
    /* the other selected components: thinner boxes */
    var others = allSel().slice(1);
    for (var i = 0; i < others.length || i < extraBoxes.length; i++) {
      if (i >= extraBoxes.length) {
        var nb = document.createElement('div');
        nb.className = 'b s2';
        sh.appendChild(nb);
        extraBoxes.push(nb);
      }
      var bxd = extraBoxes[i];
      var o = others[i];
      var rr = o ? o.getBoundingClientRect() : null;
      if (!rr || (!rr.width && !rr.height)) { bxd.style.display = 'none'; continue; }
      bxd.style.display = 'block';
      bxd.style.left = rr.left + 'px'; bxd.style.top = rr.top + 'px';
      bxd.style.width = rr.width + 'px'; bxd.style.height = rr.height + 'px';
    }
    var g = drag && drag.active && drag.guide ? drag.guide : null;
    var box = drag && drag.lines ? drag.lines.box : null;
    if (g && g.x != null && box) {
      gX.style.display = 'block'; gX.style.left = Math.round(g.x) + 'px'; gX.style.top = box.top + 'px';
      gX.style.width = '1px'; gX.style.height = box.height + 'px';
    } else gX.style.display = 'none';
    gY.classList.toggle('bl', !!(drag && drag.baseline));   // (a text baseline: dashed)
    if (g && g.y != null && box) {
      gY.style.display = 'block'; gY.style.top = Math.round(g.y) + 'px'; gY.style.left = box.left + 'px';
      gY.style.height = '1px'; gY.style.width = box.width + 'px';
    } else gY.style.display = 'none';
    /* the spacing snaplines: a short blue line across the gap to the neighbour */
    var gp = drag && drag.active && drag.gap ? drag.gap : null;
    if (gp && gp.x) {
      gpX.style.display = 'block'; gpX.style.left = Math.round(gp.x.from) + 'px'; gpX.style.width = Math.max(1, Math.round(gp.x.to - gp.x.from)) + 'px';
      gpX.style.top = Math.round(gp.x.mid) + 'px'; gpX.style.height = '2px';
    } else gpX.style.display = 'none';
    if (gp && gp.y) {
      gpY.style.display = 'block'; gpY.style.top = Math.round(gp.y.from) + 'px'; gpY.style.height = Math.max(1, Math.round(gp.y.to - gp.y.from)) + 'px';
      gpY.style.left = Math.round(gp.y.mid) + 'px'; gpY.style.width = '2px';
    } else gpY.style.display = 'none';
  }
  /* WPF's margin adorners: the selected component (one) -- a thin line from its container's left edge to it and one
     from the top edge, each with the distance (its Left / Top, as the properties say); they follow a drag */
  function placeMargins() {
    /* 0.161 (the WPF gap list G11): all four sides -- right / bottom to the container's inner right / bottom edge too */
    var hide = function () { [mgX, mgY, mgTX, mgTY, mgX2, mgY2, mgTX2, mgTY2].forEach(function (x) { if (x) x.style.display = 'none'; }); };
    if (!mgX || mode !== 'design' || !selEl || TXT || PLACE || selEl === formRoot() || allSel().length !== 1) return hide();
    var bx = boxOf(selEl);
    var node = bx && !bx.root ? bx.node : null;
    var par = node ? node.offsetParent : null;
    if (!par || par === document.body || par === document.documentElement || !node.getClientRects().length) return hide();
    var r = node.getBoundingClientRect(), pr = par.getBoundingClientRect();
    var z = node.offsetWidth ? r.width / node.offsetWidth : 1;
    var x0 = pr.left + par.clientLeft * z, y0 = pr.top + par.clientTop * z;
    var cx = r.left + r.width / 2, cy = r.top + r.height / 2;
    var x1 = x0 + par.clientWidth * z, y1 = y0 + par.clientHeight * z;
    /* from = where the line starts (the container's edge or the component's), len = its length on the screen */
    var put = function (ln, tg, horiz, from, len) {
      if (!ln || !tg) return;
      if (len < 1) { ln.style.display = 'none'; tg.style.display = 'none'; return; }
      ln.style.display = 'block';
      if (horiz) { ln.style.left = Math.round(from) + 'px'; ln.style.top = Math.round(cy) + 'px'; ln.style.width = Math.round(len) + 'px'; ln.style.height = '1px'; }
      else { ln.style.left = Math.round(cx) + 'px'; ln.style.top = Math.round(from) + 'px'; ln.style.width = '1px'; ln.style.height = Math.round(len) + 'px'; }
      tg.textContent = String(Math.round(len / z));
      tg.style.display = 'block';
      if (horiz) { tg.style.left = Math.round(from + len / 2 - tg.offsetWidth / 2) + 'px'; tg.style.top = Math.round(cy - 15) + 'px'; }
      else { tg.style.left = Math.round(cx + 3) + 'px'; tg.style.top = Math.round(from + len / 2 - 6) + 'px'; }
    };
    put(mgX, mgTX, true, x0, r.left - x0);
    put(mgY, mgTY, false, y0, r.top - y0);
    put(mgX2, mgTX2, true, r.right, x1 - r.right);
    put(mgY2, mgTY2, false, r.bottom, y1 - r.bottom);
  }
  function scheduleDraw() {
    if (drawPending) return;
    drawPending = true;
    requestAnimationFrame(draw);
  }
  function drawBadge() {
    if (!badge) return;
    drawToolbar();
    var z = typeof zoom === 'number' && zoom !== 1 ? '　［縮放 ' + Math.round(zoom * 100) + '%，' + wheelKeyText() + '］' : '';
    var w = WIRE ? '　［接線：綠＝網頁有接　橘＝部分／只有提到　紅＝網頁沒接］' : '';
    var gh = GHOST ? '　［紫色虛線框＝BCB6 .dfm 的位置（拖過去會吸附）］' : '';
    gh += TABO ? '　［Tab 順序：藍＝跟 DFM 一樣　紅 3≠5＝網頁第 3、DFM 第 5　灰＝DFM 沒有］' : '';
    if (TABSET) badge.textContent = '設定 Tab 順序：依序點元件，下一個是 ' + TABSET.next + '　Esc／右鍵＝完成';
    else if (PLACE) badge.textContent = '放置 ' + PLACE.label + '：點一下＝放在那裡　拖一個框＝連大小　Esc／右鍵＝取消';
    else badge.textContent = (mode === 'design'
      ? '設計模式：點＝選取　Ctrl/Shift+點＝多選　拖曳＝移動　控制點＝改大小　方向鍵＝微調（Shift＝10）　雙擊＝預設事件（空的＝新增）　F2＝改文字　Enter／F7＝程式碼　Shift+F7＝HTML　Esc＝上層　Tab＝下一個　空白鍵＋拖曳＝平移'
      : '操作模式：點擊交給頁面（網路仍全部封鎖）') + z + w + gh;
    /* never over the artboard toolbar (bottom left, WPF keeps it in view): the hint keeps to the right of it and wraps
       upwards when the view is narrow */
    var tbR = atbEl && atbEl.style.display === 'block' ? atbEl.getBoundingClientRect().right : 0;
    var vw = document.documentElement && document.documentElement.clientWidth || window.innerWidth;   /* (without a scrollbar) */
    badge.style.boxSizing = 'border-box';   /* (the max width with its padding) */
    badge.style.maxWidth = Math.max(120, vw - tbR - 18) + 'px';
  }
  /* DFM 位置: a dashed frame where the .dfm puts each control whose position / size differs.
     The values are in the control's own coordinates (what its style would say), so the frame
     stays at the DFM place while the control moves -- put it back on the frame and they agree.
     [{ id, left?, top?, width?, height? }] (a side not given = where it is now) */
  var GHOST = null, ghostBoxes = [], ghTag = null;
  function ghostRect(it) {
    var el = byId(it.id);
    var bx = el ? boxOf(el) : null;
    if (!bx || bx.root || !bx.node.getClientRects().length) return null;
    var r = bx.node.getBoundingClientRect(), cb = curBox(bx.node);
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var L = it.left != null ? it.left : cb.left, T = it.top != null ? it.top : cb.top;
    var W = it.width != null ? it.width : cb.width, H = it.height != null ? it.height : cb.height;
    return { el: el, left: r.left + (L - cb.left) * z, top: r.top + (T - cb.top) * z, width: W * z, height: H * z, L: L, T: T, W: W, H: H };
  }
  function ghostOf(el) {
    for (var i = 0; GHOST && i < GHOST.length; i++) if (GHOST[i].id === el.id) return ghostRect(GHOST[i]);
    return null;
  }
  function placeGhosts() {
    var n = 0, selG = null;
    for (var i = 0; GHOST && mode === 'design' && i < GHOST.length && n < 3000; i++) {
      var g = ghostRect(GHOST[i]);
      if (!g || (g.el.closest && g.el.closest('[data-htd-hide]'))) continue;
      if (n >= ghostBoxes.length) { var nb = document.createElement('div'); sh.insertBefore(nb, sh.firstChild.nextSibling); ghostBoxes.push(nb); }
      var b = ghostBoxes[n++];
      b.className = 'gh' + (g.el === selEl ? ' sel' : '');
      b.style.display = 'block';
      b.style.left = g.left + 'px'; b.style.top = g.top + 'px';
      b.style.width = g.width + 'px'; b.style.height = g.height + 'px';
      if (g.el === selEl) selG = g;
    }
    for (var j = n; j < ghostBoxes.length; j++) ghostBoxes[j].style.display = 'none';
    if (ghTag) {
      if (selG) {
        ghTag.textContent = 'DFM　left ' + Math.round(selG.L) + '　top ' + Math.round(selG.T) + '　' + Math.round(selG.W) + '×' + Math.round(selG.H);
        ghTag.style.display = 'block';
        ghTag.style.left = Math.max(0, selG.left) + 'px';
        ghTag.style.top = (selG.top + selG.height + 1) + 'px';
      } else ghTag.style.display = 'none';
    }
    return n;
  }
  /* 顯示所有元件名稱: a small tag with its name at every visible component's top-left corner */
  var NAMES = false, nameTags = [];
  function placeNameTags() {
    var n = 0;
    if (NAMES) {
      var fr = formRoot();
      var list = allComps();
      for (var i = 0; i < list.length && n < 1500; i++) {
        var el = list[i];
        if (el === fr || !el.id || !el.getClientRects().length || (el.closest && el.closest('[data-htd-hide]'))) continue;
        var r = el.getBoundingClientRect();
        if ((!r.width && !r.height) || r.bottom < 0 || r.right < 0 || r.top > window.innerHeight || r.left > window.innerWidth) continue;
        if (n >= nameTags.length) { var nt = document.createElement('div'); nt.className = 'nt'; sh.appendChild(nt); nameTags.push(nt); }
        var t = nameTags[n++];
        t.textContent = el.id;
        t.style.display = 'block';
        t.style.left = r.left + 'px';
        t.style.top = r.top + 'px';
      }
    }
    for (var j = n; j < nameTags.length; j++) nameTags[j].style.display = 'none';
    return n;
  }
  /* Tab 順序 (BCB6's Edit > Tab Order): a badge on every place the page's Tab key stops, numbered
     in its order among the controls the .dfm has too. TABO = those names in the .dfm's order
     (null = off). Blue: the same place as in the .dfm; red "3≠5": 3rd on the page, 5th in
     the .dfm; grey "·": the page stops there, the .dfm has no such stop. */
  var TABO = null, tabBadges = [];
  function pageTabOrder() {
    var all = document.querySelectorAll('input,select,textarea,button,a[href],[tabindex]');
    var list = [];
    for (var i = 0; i < all.length; i++) {
      var x = all[i];
      if (x.disabled || x.tabIndex < 0 || x.type === 'hidden' || !x.getClientRects().length) continue;
      var cs = getComputedStyle(x);
      if (cs.visibility === 'hidden' || cs.display === 'none') continue;
      /* hidden when the page runs (shown now only because we are designing): no stop then */
      if (x.closest && x.closest('[data-htd-show],[data-htd-vis]')) continue;
      list.push(x);
    }
    var pos = list.filter(function (x) { return x.tabIndex > 0; }).sort(function (a, b) { return a.tabIndex - b.tabIndex; });
    return pos.concat(list.filter(function (x) { return x.tabIndex === 0; }));
  }
  /* the component a Tab stop belongs to: its own id, or -- a generated TCheckBox / TRadioButton
     is <label class="ckb" id="name"><input type="checkbox"> -- the label's around it */
  function stopName(x) {
    if (x.id) return x.id;
    var p = x.parentElement;
    return p && p.tagName === 'LABEL' && p.id ? p.id : '';
  }
  function tabModel() {
    var seq = pageTabOrder();
    var dfm = TABO || [];
    var inDfm = {}, onPage = {};
    for (var i = 0; i < dfm.length; i++) inDfm[dfm[i]] = 1;
    for (var j = 0; j < seq.length; j++) { var sn = stopName(seq[j]); if (sn) onPage[sn] = 1; }
    var dRank = {}, k = 0;
    for (var d = 0; d < dfm.length; d++) if (onPage[dfm[d]]) dRank[dfm[d]] = ++k;
    var items = [], p = 0, bad = [], counted = {};
    for (var s = 0; s < seq.length; s++) {
      var id = stopName(seq[s]);
      if (id && inDfm[id] && !counted[id]) {
        counted[id] = 1;
        p++;
        items.push({ el: seq[s], page: p, dfm: dRank[id] });
        if (p !== dRank[id]) bad.push(id + '（網頁第 ' + p + '、DFM 第 ' + dRank[id] + '）');
      } else items.push({ el: seq[s], page: 0, dfm: 0 });
    }
    return { items: items, stops: seq.length, common: p, bad: bad };
  }
  /* the stops change only with an edit / a redraw: read them again at most every 0.7 s (a big
     page has hundreds, and the overlay is drawn on every mouse move); the badges follow each draw */
  var tabCache = null;
  function tabModelCached() {
    var now = Date.now();
    if (!tabCache || now - tabCache.at > 700) tabCache = { at: now, model: tabModel() };
    return tabCache.model;
  }
  function placeTabOrder() {
    var n = 0;
    if (TABO && mode === 'design') {
      var md = tabModelCached();
      for (var i = 0; i < md.items.length && n < 3000; i++) {
        var it = md.items[i];
        var r = it.el.getBoundingClientRect();
        if ((!r.width && !r.height) || r.bottom < 0 || r.right < 0 || r.top > window.innerHeight || r.left > window.innerWidth) continue;
        if (n >= tabBadges.length) { var nb = document.createElement('div'); sh.appendChild(nb); tabBadges.push(nb); }
        var b = tabBadges[n++];
        b.className = 'tb' + (!it.page ? ' x' : it.page !== it.dfm ? ' bad' : '');
        b.textContent = !it.page ? '·' : it.page !== it.dfm ? it.page + '≠' + it.dfm : String(it.page);
        b.style.display = 'block';
        b.style.left = (r.left - 4) + 'px';
        b.style.top = (r.top - 6) + 'px';
      }
    }
    for (var j = n; j < tabBadges.length; j++) tabBadges[j].style.display = 'none';
    return n;
  }
  /* 在畫面上標出接線狀態: a coloured frame per component with DFM events (id -> ok | mid | bad) */
  var WIRE = null, wireBoxes = [];
  function placeWireMarks() {
    var ids = WIRE ? Object.keys(WIRE) : [];
    var n = 0;
    for (var i = 0; i < ids.length; i++) {
      var el = byId(ids[i]);
      if (!el || !el.getClientRects().length) continue;
      var r = el.getBoundingClientRect();
      if (!r.width && !r.height) continue;
      if (n >= wireBoxes.length) { var nb = document.createElement('div'); sh.appendChild(nb); wireBoxes.push(nb); }
      var b = wireBoxes[n++];
      b.className = 'wm ' + WIRE[ids[i]];
      b.style.display = 'block';
      b.style.left = r.left + 'px'; b.style.top = r.top + 'px';
      b.style.width = r.width + 'px'; b.style.height = r.height + 'px';
    }
    for (var j = n; j < wireBoxes.length; j++) wireBoxes[j].style.display = 'none';
    return n;
  }

  /* 6. describing a component ---------------------------------------------- */
  var INHERIT_TYPES = ['click', 'dblclick', 'mousedown', 'mouseup', 'pointerdown', 'pointerup', 'change',
    'input', 'keydown', 'keyup', 'keypress', 'contextmenu', 'touchstart', 'touchend', 'focusin', 'focusout', 'submit'];
  var CS_KEYS = ['display', 'position', 'visibility', 'color', 'background-color', 'font-family', 'font-size',
    'font-weight', 'z-index', 'cursor', 'overflow'];
  function cssPath(el) {
    var parts = [];
    for (var x = el; x && x.nodeType === 1 && x !== document.documentElement; x = x.parentElement) {
      if (x.id) { parts.unshift('#' + x.id); break; }
      var i = 1, s = x;
      while ((s = s.previousElementSibling)) if (s.tagName === x.tagName) i++;
      parts.unshift(x.tagName.toLowerCase() + ':nth-of-type(' + i + ')');
    }
    return parts.join(' > ');
  }
  function describe(el) {
    var d = {
      key: keyOf(el), id: idOf(el), tag: el.tagName.toLowerCase(),
      cls: el.getAttribute('class') || '', title: el.getAttribute('title') || '',
      attrs: [], style: [], inline: [], computed: [], chain: [], cssPath: el.id ? '' : cssPath(el),
      text: '', visible: !hidden(el), geom: null, listeners: [], inherited: [],
    };
    var i;
    for (i = 0; i < el.attributes.length; i++) {
      var a = el.attributes[i];
      var v = a.value.length > 300 ? a.value.slice(0, 300) + '…' : a.value;
      if (/^on/i.test(a.name)) d.inline.push([a.name, v]);
      else if (a.name !== 'style') d.attrs.push([a.name, v]);
    }
    for (i = 0; i < el.style.length; i++) {
      var n = el.style[i];
      d.style.push([n, el.style.getPropertyValue(n) + (el.style.getPropertyPriority(n) ? ' !important' : '')]);
    }
    try {
      var cs = getComputedStyle(el);
      CS_KEYS.forEach(function (k) { d.computed.push([k, cs.getPropertyValue(k)]); });
    } catch (e) { /* ignore */ }
    var r = el.getBoundingClientRect();
    var fr = formRoot();
    var fb = fr && fr !== el && fr.contains(el) ? fr.getBoundingClientRect() : null;
    d.geom = {
      left: Math.round(fb ? r.left - fb.left : r.left + window.scrollX),
      top: Math.round(fb ? r.top - fb.top : r.top + window.scrollY),
      width: Math.round(r.width), height: Math.round(r.height), rel: fb ? 'form' : 'page',
    };
    if (!/^(INPUT|SELECT|TEXTAREA)$/.test(el.tagName)) d.text = clip(el.textContent, 200);
    else d.text = clip(el.value, 200);
    for (var x = el.parentElement; x && x !== document.documentElement; x = x.parentElement) {
      var xid = idOf(x);
      if (xid) d.chain.unshift(xid);
    }
    d.listeners = serList(REC.get(el), null, 40);
    var budget = 30;
    for (var y = el.parentElement; y && budget > 0; y = y.parentElement) {
      var l = serList(REC.get(y), INHERIT_TYPES, budget);
      if (l.length) { d.inherited.push({ label: idOf(y) ? '#' + idOf(y) : '<' + y.tagName.toLowerCase() + '>', list: l }); budget -= l.length; }
    }
    [[document, 'document'], [window, 'window']].forEach(function (p) {
      var l2 = serList(REC.get(p[0]), INHERIT_TYPES, budget);
      if (l2.length) { d.inherited.push({ label: p[1], list: l2 }); budget -= l2.length; }
    });
    if (el === selEl) {
      var ms = allSel().slice(1);
      if (ms.length) d.multi = ms.map(function (x) { return idOf(x) || x.tagName.toLowerCase(); });
    }
    try { d.layout = layoutOf(el); } catch (e) { d.layout = null; }
    try { d.caption = captionOf(el); } catch (e) { d.caption = null; }
    try { d.look = lookOf(el); } catch (e) { d.look = null; }
    return d;
  }

  /* 6b. WPF-style editing --------------------------------------------------------
     The DOM is changed here (instant feedback); the extension then writes the same
     change into the page source ('edit' messages). Only inline px values are edited;
     anything else (%, auto, a class rule) is refused rather than guessed. */
  var drag = null;
  /* zoom (WPF: Ctrl+wheel). CSS zoom on <body> only -- the overlay lives on <html>, so
     it stays in plain viewport px. Screen deltas are divided by the zoom before they
     become CSS px in the source. */
  var zoom = 1;
  /* WPF's zoom: 12.5% to 800%; fit / fit selection pick a step up to 400%, as before */
  var ZOOMS = [0.125, 0.25, 0.33, 0.5, 0.67, 0.75, 0.9, 1, 1.1, 1.25, 1.5, 2, 3, 4, 6, 8];
  /* the wheel that zooms: 'ctrl' (Ctrl+wheel) / 'wheel' (the wheel alone) / 'alt' (Alt+wheel) -- the zoomWheel setting */
  var WHEELZ = 'ctrl';
  function wheelKeyText() { return WHEELZ === 'wheel' ? '滾輪' : WHEELZ === 'alt' ? 'Alt+滾輪' : 'Ctrl+滾輪'; }
  var FITZ = ZOOMS.filter(function (z) { return z <= 4; });
  function pct(z) { return Math.round(z * 1000) / 10 + '%'; }
  function setZoom(z) {
    if (TXT) endTextEdit(true);   // (the box would stay where the text was)
    zoom = Math.max(0.125, Math.min(8, z));
    if (document.body) document.body.style.zoom = zoom === 1 ? '' : String(zoom);
    drawBadge();
    scheduleDraw();
    post({ type: 'zoom', zoom: zoom });
  }
  /* 符合視窗: the largest zoom step at which the whole form fits the view */
  function zoomFit() {
    var fr = formRoot() || document.body;
    var r = fr.getBoundingClientRect();
    var w = r.width / zoom, h = r.height / zoom;            // the form's own px
    if (!w || !h) return zoom;
    var want = Math.min((window.innerWidth - 16) / w, (window.innerHeight - 16) / h);
    var z = FITZ[0];
    for (var i = 0; i < FITZ.length; i++) if (FITZ[i] <= want + 1e-9) z = FITZ[i];
    setZoom(z);
    try { window.scrollTo(0, 0); } catch (x) { /* ignore */ }
    return z;
  }
  /* 縮放到選取的元件 (Blend's Zoom to selection): the largest step at which the whole selection
     fits the view with a margin (400% at most), scrolled so it sits in the middle */
  function zoomToSel(seq) {
    var list = allSel().filter(function (x) { return x.getClientRects().length; });
    if (!list.length) { if (seq) post({ type: 'zoomSel', seq: seq, zoom: zoom, none: true }); return zoom; }
    var L = Infinity, T = Infinity, R = -Infinity, B = -Infinity;
    for (var i = 0; i < list.length; i++) {
      var r = list[i].getBoundingClientRect();
      L = Math.min(L, r.left); T = Math.min(T, r.top); R = Math.max(R, r.right); B = Math.max(B, r.bottom);
    }
    var w = Math.max(1, (R - L) / zoom), h = Math.max(1, (B - T) / zoom);   /* the page's own px */
    var want = Math.min((window.innerWidth - 60) / w, (window.innerHeight - 60) / h);
    var z = FITZ[0];
    for (var k = 0; k < FITZ.length; k++) if (FITZ[k] <= want + 1e-9) z = FITZ[k];
    var cx = ((L + R) / 2 + window.scrollX) / zoom, cy = ((T + B) / 2 + window.scrollY) / zoom;
    setZoom(z);
    try { window.scrollTo(Math.max(0, cx * z - window.innerWidth / 2), Math.max(0, cy * z - window.innerHeight / 2)); } catch (x) { /* ignore */ }
    scheduleDraw();
    /* (asked with a seq: what it went by, so the step can be checked) */
    if (seq) post({ type: 'zoomSel', seq: seq, zoom: z, w: w, h: h, vw: window.innerWidth, vh: window.innerHeight });
    return z;
  }
  /* the element that actually carries the position: itself, or the generated
     <span style="position:absolute…"> wrapper around an <input> */
  function boxOf(el) {
    if (!el || el === document.body) return null;
    /* the form itself: resizable (inline width/height), not movable -- like a WPF Window */
    if (el === formRoot()) return el.style && (el.style.width || el.style.height) ? { node: el, target: 'self', root: true } : null;
    if (el.style && el.style.position === 'absolute') return { node: el, target: 'self' };
    var p = el.parentElement;
    if (p && !p.id && p !== document.body && p.style && p.style.position === 'absolute' && p.children.length === 1) return { node: p, target: 'parent' };
    return null;
  }
  function pxOf(v) {
    var m = /^\s*(-?\d+(?:\.\d+)?)px\s*$/.exec(v || '');
    return m ? parseFloat(m[1]) : null;
  }
  function snap(bx) {
    var s = bx.node.style;
    return { left: s.left, top: s.top, width: s.width, height: s.height, right: s.right, bottom: s.bottom,
      ow: bx.node.offsetWidth, oh: bx.node.offsetHeight };
  }
  /* new inline values for a move/resize by (dL, dT, dW, dH), from snapshot s0;
     null when a value that must change is not an inline px number */
  function deltaStyle(s0, dL, dT, dW, dH) {
    var ch = {};
    function axis(a, b, size, osize, dA, dS) {
      if (!dA && !dS) return true;
      /* a left / top handle (the far edge stays): past the opposite edge the size stops at 2 px -- and so does the
         moving edge (AI 20261001: the size stopped, the left went on, the component slid away) */
      var S00 = pxOf(s0[size]);
      if (S00 == null) S00 = osize;
      if (dA && dS && Math.abs(dA + dS) < 1e-9 && S00 + dS < 2) { dS = 2 - S00; dA = -dS; }
      var A = pxOf(s0[a]), B = pxOf(s0[b]), S = pxOf(s0[size]);
      var hasA = s0[a] !== '', hasB = s0[b] !== '', hasS = s0[size] !== '';
      if (hasA) { if (A == null) return false; if (dA) ch[a] = Math.round(A + dA) + 'px'; }
      if (hasS) {
        if (S == null) return false;
        if (dS) ch[size] = Math.max(2, Math.round(S + dS)) + 'px';
        if (!hasA && hasB) { if (B == null) return false; ch[b] = Math.round(B - dA - dS) + 'px'; }
      } else if (hasB) {
        if (B == null) return false;
        ch[b] = Math.round(B - dA - dS) + 'px';
        if (!hasA) return true;
      } else if (dS) {
        ch[size] = Math.max(2, Math.round(osize + dS)) + 'px';
      }
      if (!hasA && !hasB && dA) return false;   // not positioned on this axis
      return true;
    }
    if (!axis('left', 'right', 'width', s0.ow, dL, dW)) return null;
    if (!axis('top', 'bottom', 'height', s0.oh, dT, dH)) return null;
    return ch;
  }
  function applyStyle(node, ch) { for (var k in ch) if (Object.prototype.hasOwnProperty.call(ch, k)) node.style[k] = ch[k]; }
  /* where the box is: as drawn (outer size, like the DFM's Width/Height); a box that
     is not drawn (a hidden tab or panel) measures 0, so then the px its style says */
  function curBox(n) {
    if (n.getClientRects().length) return { left: n.offsetLeft, top: n.offsetTop, width: n.offsetWidth, height: n.offsetHeight, rendered: true };
    var s = n.style;
    return { left: pxOf(s.left), top: pxOf(s.top), width: pxOf(s.width), height: pxOf(s.height), rendered: false };
  }
  /* how far the box's coordinate origin is from its DFM parent's: a wrapper the
     generator adds (span.lled around an LED and its label) shifts it. The walk stops
     at the element that stands for the DFM parent: one with an id, a tab sheet
     (<div class="pcPane"> -- a TTabSheet has no id on the page; its tab bar and body
     offsets belong to the TPageControl), or a container titled with a bare component
     name (the page's JS strips most titles at run time, so that is only a fallback) */
  function originShift(n, el) {
    var cp = compParent(el) || formRoot();
    var dx = 0, dy = 0;
    var drawn = n.getClientRects().length > 0;
    for (var p = n.parentElement; p && p !== cp && p !== document.body && p !== document.documentElement; p = p.parentElement) {
      if (p.classList && p.classList.contains('pcPane')) break;
      /* (a wrapper's title is "name : TClass…", never a bare name) */
      var tt = p.getAttribute('title');
      if (tt && (Object.prototype.hasOwnProperty.call(CLS, tt) || /^[A-Za-z_]\w*$/.test(tt))) break;
      var pcs = getComputedStyle(p);
      if (pcs.position === 'static') continue;
      if (drawn) { dx += p.offsetLeft; dy += p.offsetTop; }
      else { dx += pxOf(pcs.left) || 0; dy += pxOf(pcs.top) || 0; }
    }
    return { x: dx, y: dy };
  }
  function layoutOf(el) {
    var bx = boxOf(el);
    if (!bx) return null;
    var s = bx.node.style;
    var cb = curBox(bx.node);
    var sh = bx.root ? { x: 0, y: 0 } : originShift(bx.node, el);
    return {
      target: bx.target, root: !!bx.root, rendered: cb.rendered, ox: sh.x, oy: sh.y,
      left: cb.left, top: cb.top, width: cb.width, height: cb.height,
      raw: { left: s.left, top: s.top, width: s.width, height: s.height, right: s.right, bottom: s.bottom },
    };
  }
  /* the element's caption and where it lives: value / legend / pnlCap / text */
  function captionOf(el) {
    var tn = el.tagName;
    if (tn === 'INPUT') {
      var ty = (el.getAttribute('type') || 'text').toLowerCase();
      if (/^(checkbox|radio|button|submit|image|file|hidden|range|color)$/.test(ty)) return null;
      return { kind: 'value', value: el.getAttribute('value') || '' };
    }
    if (/^(SELECT|TEXTAREA|IMG|TABLE|CANVAS|SVG)$/.test(tn)) return null;
    if (tn === 'FIELDSET') { var lg = el.querySelector(':scope > legend'); return lg ? { kind: 'legend', value: lg.textContent } : null; }
    var cap = el.querySelector(':scope > .pnlCap');
    if (cap) return { kind: 'pnlCap', value: cap.textContent };
    for (var n = el.firstChild; n; n = n.nextSibling) {
      if (n.nodeType === 3 && /\S/.test(n.nodeValue)) return { kind: 'text', value: n.nodeValue.replace(/^\s+|\s+$/g, '') };
    }
    if (!el.children.length) return { kind: 'text', value: '' };
    return null;
  }
  function setCaptionDom(el, kind, value) {
    if (kind === 'value') { el.setAttribute('value', value); el.value = value; return; }
    if (kind === 'legend') { var lg = el.querySelector(':scope > legend'); if (lg) lg.textContent = value; return; }
    if (kind === 'pnlCap') { var cap = el.querySelector(':scope > .pnlCap'); if (cap) cap.textContent = value; return; }
    for (var n = el.firstChild; n; n = n.nextSibling) {
      if (n.nodeType === 3 && /\S/.test(n.nodeValue)) { n.nodeValue = value; return; }
    }
    el.appendChild(document.createTextNode(value));
  }
  /* WPF (XAML Designer keyboard shortcuts): F2 = edit the control's text right on the design surface; Enter or
     leaving it = done (the same edit as the properties panel's Caption), Esc = never mind. While it is open the
     design keys leave the keyboard alone, and the keys it gets go no further (VS Code's shortcuts stay out of it). */
  var TXT = null;
  function startTextEdit() {
    if (TXT || mode !== 'design' || !selEl || !host) return false;
    var el = selEl;
    var lk = lockedOf(el);
    if (lk) { post({ type: 'editRefused', why: lockedWhy(lk) }); return false; }
    var c = captionOf(el);
    if (!c) { post({ type: 'editRefused', why: '這個元件沒有可以直接改的文字（其他屬性在屬性面板改）' }); return false; }
    var r = el.getBoundingClientRect();
    if (!r.width && !r.height) return false;
    var cs = getComputedStyle(el);
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var inp = document.createElement('input');
    inp.className = 'txe';
    inp.type = 'text';
    inp.spellcheck = false;
    inp.value = c.value;
    inp.style.left = r.left + 'px';
    inp.style.top = r.top + 'px';
    inp.style.width = Math.max(60, r.width) + 'px';
    inp.style.height = Math.max(20, r.height) + 'px';
    inp.style.fontFamily = cs.fontFamily;
    inp.style.fontSize = Math.max(9, (parseFloat(cs.fontSize) || 12) * z) + 'px';
    inp.style.fontWeight = cs.fontWeight;
    inp.style.textAlign = /center|right/.test(cs.textAlign) ? cs.textAlign : 'left';
    TXT = { el: el, kind: c.kind, orig: c.value, inp: inp };
    /* (its Enter / Esc: the window's keydown listener, which runs first) */
    origAdd.call(inp, 'blur', function () { endTextEdit(true); });
    sh.appendChild(inp);
    post({ type: 'textEdit', open: true });
    try { inp.focus(); inp.select(); } catch (x) { /* ignore */ }
    return true;
  }
  function endTextEdit(commit) {
    var t = TXT;
    if (!t) return false;
    TXT = null;   // (first: removing the input blurs it, and that must not end it twice)
    var v = t.inp.value;
    if (t.inp.parentNode) t.inp.parentNode.removeChild(t.inp);
    post({ type: 'textEdit', open: false });
    if (!commit || v === t.orig || !t.el.isConnected) return false;
    setCaptionDom(t.el, t.kind, v);
    sendEdit(t.el, { what: 'caption', capKind: t.kind, value: v });
    selEl = null; select(t.el, 'edit', false);
    return true;
  }
  function hexOf(c) {
    var m = /^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*([\d.]+))?\)$/.exec(c || '');
    if (!m) return '';
    if (m[4] !== undefined && parseFloat(m[4]) === 0) return '';
    return '#' + [m[1], m[2], m[3]].map(function (x) { return ('0' + (+x).toString(16)).slice(-2); }).join('');
  }
  /* where a control's Font lives: a TPanel's on its caption <span class="pnlCap"> (the
     generator writes the Font there), a TGroupBox's on its <legend>, else on itself */
  function fontHost(el) {
    var pc = el.querySelector(':scope > .pnlCap');
    if (pc) return { el: pc, target: 'pnlCap' };
    if (el.tagName === 'FIELDSET') { var lg = el.querySelector(':scope > legend'); if (lg) return { el: lg, target: 'legend' }; }
    return { el: el, target: 'self' };
  }
  function lookOf(el) {
    var cs = getComputedStyle(el);
    var fh = fontHost(el);
    var fs = fh.el === el ? cs : getComputedStyle(fh.el);
    var form = /^(INPUT|SELECT|TEXTAREA|BUTTON)$/.test(el.tagName);
    var par = el.parentElement;
    return {
      /* BCB6 Visible: its own style says display:none (what Visible=False becomes). While designing it shows
         anyway; when the page runs it is hidden. A hidden tab around it does not count. */
      visible: el.style.display !== 'none',
      /* hidden when the page runs, shown now only because we are designing: 'self' = its Visible=False,
         'rule' = a page rule or the page's JS (a machine's "-only" class, an option rule) */
      runHidden: runHidden(el) ? (el.style.display === 'none' ? 'self' : 'rule') : null,
      enabled: form ? !el.disabled : null,
      fontOn: fh.target,
      fontSize: Math.round(parseFloat(fs.fontSize) || 0),
      fontName: String(fs.fontFamily || '').split(',')[0].replace(/^["']|["']$/g, '').trim(),
      fontFamilies: String(fs.fontFamily || '').split(',').map(function (f) { return f.trim().replace(/^["']|["']$/g, ''); }).filter(Boolean),
      /* false = the font family is inherited (the form's UI font), not given for this control */
      fontOwn: !par || !!el.style.fontFamily || !!fh.el.style.fontFamily || fs.fontFamily !== getComputedStyle(par).fontFamily,
      bold: (parseInt(fs.fontWeight, 10) || 400) >= 600 || fs.fontWeight === 'bold',
      italic: fs.fontStyle === 'italic',
      /* 0.152 (the WPF gap list G1: the DFM-only rows editable): Font.Underline / StrikeOut, a label's WordWrap, an
         edit's ReadOnly / MaxLength, a check box's Checked, TabOrder (tabindex) -- null = this control has none */
      underline: /underline/.test(String(fs.textDecorationLine || fs.textDecoration || '')),
      strikeout: /line-through/.test(String(fs.textDecorationLine || fs.textDecoration || '')),
      wordWrap: isLabel(el) ? wraps(el) : null,
      readOnly: textInput(el) ? !!el.readOnly : null,
      maxLength: textInput(el) && el.tagName === 'INPUT' ? (el.maxLength > 0 ? el.maxLength : 0) : null,
      checked: checkInput(el) ? !!checkInput(el).checked : null,
      tabOrder: form ? (el.hasAttribute('tabindex') ? el.tabIndex : null) : null,
      /* 0.158 a TImage's Picture (WPF Image.Source): the <img src> as written (relative to the page), null = not an image */
      src: el.tagName === 'IMG' ? String(el.getAttribute('src') || '') : null,
      color: hexOf(fs.color),
      background: hexOf(cs.backgroundColor),
      /* a label's AutoSize: its width is "auto" (the generator writes "width:136px; … width:auto");
         null = not a label */
      autoSize: isLabel(el) ? autoSized(el) : null,
      /* the caption's Alignment in VCL's words: a label's own text, a generated panel's .pnlCap */
      alignment: isLabel(el) ? taOf(cs.textAlign) : panelCap(el) ? taOf(getComputedStyle(panelCap(el)).textAlign) : null,
      /* the IO lamp / panel button's own properties (hwidgets.js: TALed family = span.aled, TBtnPanel family =
         div.btnpanel), null for any other control */
      io: ioLookOf(el),
    };
  }
  var LED_STYLES = ['LEDSmall', 'LEDLarge', 'LEDSqSmall', 'LEDSqLarge', 'LEDVertical', 'LEDHorizontal'];
  function cssVarOf(el, name) { var v = String(el.style.getPropertyValue(name) || '').trim(); return v ? (hexOf(v) || v.toLowerCase()) : ''; }
  /* TALed: LEDStyle (its shape, a class), Value (lit: "on"), Blink, TrueColor / FalseColor (--led-on / --led-off);
     TBtnPanel: Style (tsFlatButtons = "flat"), Down ("down"), TrueColor / FalseColor / TrueFontColor / FalseFontColor
     (--bp-true / --bp-false / --bp-true-font / --bp-false-font) */
  function ioLookOf(el) {
    var cl = el.classList;
    if (!cl) return null;
    if (cl.contains('aled')) {
      var ls = LED_STYLES.filter(function (c) { return cl.contains(c); })[0] || 'LEDSmall';
      return { kind: 'led', ledStyle: ls, value: cl.contains('on'), blink: cl.contains('blink'),
        trueColor: cssVarOf(el, '--led-on'), falseColor: cssVarOf(el, '--led-off') };
    }
    if (cl.contains('btnpanel')) {
      return { kind: 'btn', flat: cl.contains('flat'), down: cl.contains('down'),
        trueColor: cssVarOf(el, '--bp-true'), falseColor: cssVarOf(el, '--bp-false'),
        trueFontColor: cssVarOf(el, '--bp-true-font'), falseFontColor: cssVarOf(el, '--bp-false-font') };
    }
    return null;
  }
  /* one IO property change: applied to the DOM, returned as the edit (null = not one, or not this kind) */
  var IO_VARS = { led: { trueColor: '--led-on', falseColor: '--led-off' },
    btn: { trueColor: '--bp-true', falseColor: '--bp-false', trueFontColor: '--bp-true-font', falseFontColor: '--bp-false-font' } };
  function ioChange(el, prop, value) {
    var io = ioLookOf(el);
    if (!io || !/^io\./.test(prop)) return null;
    var p = prop.slice(3);
    var cls = function (add, rem) {
      for (var i = 0; i < rem.length; i++) el.classList.remove(rem[i]);
      for (var j = 0; j < add.length; j++) el.classList.add(add[j]);
      return { what: 'class', target: 'self', add: add, remove: rem };
    };
    if (IO_VARS[io.kind][p]) {
      var v = String(value || '').trim();
      if (!/^#[0-9a-f]{6}$/i.test(v)) return null;
      var st = {};
      st[IO_VARS[io.kind][p]] = v.toLowerCase();
      el.style.setProperty(IO_VARS[io.kind][p], v.toLowerCase());
      return { what: 'style', target: 'self', style: st };
    }
    if (io.kind === 'led' && p === 'ledStyle' && LED_STYLES.indexOf(value) >= 0)
      return cls([value], LED_STYLES.filter(function (c) { return c !== value; }));
    if (io.kind === 'led' && (p === 'value' || p === 'blink')) return value ? cls([p === 'value' ? 'on' : 'blink'], []) : cls([], [p === 'value' ? 'on' : 'blink']);
    if (io.kind === 'btn' && (p === 'down' || p === 'flat')) return value ? cls([p], []) : cls([], [p]);
    return null;
  }
  function taOf(v) { return v === 'center' ? 'taCenter' : /^(right|end)$/.test(v) ? 'taRightJustify' : 'taLeftJustify'; }
  /* a generated TPanel: <div class="pnl"><span class="pnlCap"> -- its caption span, or null */
  function panelCap(el) {
    if (!el.classList || !el.classList.contains('pnl')) return null;
    for (var c = el.firstElementChild; c; c = c.nextElementSibling) if (c.classList && c.classList.contains('pnlCap')) return c;
    return null;
  }
  function isLabel(el) { return !!(el.classList && el.classList.contains('lb')) || vclOf(el).cls === 'TLabel'; }
  /* 0.152: a TEdit / TMemo (input text-like, textarea); a TCheckBox / TRadioButton's box (itself, or the <input> in its <label>) */
  function textInput(el) { return el.tagName === 'TEXTAREA' || (el.tagName === 'INPUT' && !/^(checkbox|radio|button|submit|image|file|color|range|hidden)$/i.test(el.type || '')); }
  function checkInput(el) {
    if (el.tagName === 'INPUT' && /^(checkbox|radio)$/i.test(el.type || '')) return el;
    return el.tagName === 'LABEL' && el.querySelector ? el.querySelector('input[type=checkbox],input[type=radio]') : null;
  }
  /* BCB6 AutoSize on the page: a size that follows the text -- its width is auto, or (a label that
     wraps, WordWrap: a fixed width, the height follows) its height is auto */
  function autoSized(el) { var b = boxOf(el); return !!b && (b.node.style.width === 'auto' || b.node.style.height === 'auto'); }
  function wraps(el) { try { return !/^(nowrap|pre)$/.test(getComputedStyle(el).whiteSpace); } catch (x) { return false; } }
  /* a Visible / Enabled / Font / Color change: applied to the DOM, returned as the edit to
     write (null = not a look property). A font goes where the control's Font lives. */
  function lookChange(el, prop, value, size) {
    var st = null, at = null;
    if (/^io\./.test(String(prop))) return ioChange(el, prop, value);
    if (prop === 'alignment') {
      /* BCB6 Alignment: a label's own text (it shows with a fixed width, AutoSize off), or a
         generated panel's caption span (its class centres it: taCenter = no declaration) */
      var pc = panelCap(el);
      if (!isLabel(el) && !pc) return null;
      var tav = pc
        ? (value === 'taLeftJustify' ? 'left' : value === 'taRightJustify' ? 'right' : null)
        : (value === 'taCenter' ? 'center' : value === 'taRightJustify' ? 'right' : null);   /* the default: no declaration */
      var tel = pc || el;
      if (tav) tel.style.setProperty('text-align', tav); else tel.style.removeProperty('text-align');
      return { what: 'style', target: pc ? 'pnlCap' : 'self', style: { 'text-align': tav } };
    }
    if (prop === 'autoSize') {
      /* BCB6 AutoSize: on = the size follows the text (auto), off = it keeps the size it has now --
         or, from the DFM (size), the .dfm's own */
      var ab = boxOf(el);
      if (!ab || ab.root || !isLabel(el)) return null;   /* (only a label has it; several selected: the others are skipped) */
      if (lockedOf(el)) { post({ type: 'editRefused', why: lockedWhy(lockedOf(el)) }); return null; }
      var hasSize = size && +size.width > 0 && +size.height > 0;
      if (!value && !hasSize && !ab.node.getClientRects().length) {
        /* not drawn (hidden, a tab not shown): its size cannot be measured -- 0 x 0 would be written */
        post({ type: 'editRefused', why: '「' + (idOf(el) || el.tagName.toLowerCase()) + '」現在沒有顯示（隱藏，或在沒切到的分頁裡），量不到大小；先讓它顯示再關 AutoSize' });
        return null;
      }
      var sw = hasSize ? Math.round(+size.width) : ab.node.offsetWidth, shh = hasSize ? Math.round(+size.height) : ab.node.offsetHeight;
      /* a label that wraps (WordWrap) keeps its width: only its height follows the text */
      var ast = value ? (wraps(el) ? { height: 'auto' } : { width: 'auto', height: 'auto' }) : { width: sw + 'px', height: shh + 'px' };
      for (var ak in ast) ab.node.style.setProperty(ak, ast[ak]);
      return { what: 'style', target: ab.target, style: ast };
    }
    /* 0.152 the rows that were DFM-only */
    if (prop === 'underline' || prop === 'strikeout') {
      var dh = fontHost(el);
      var cur = String(getComputedStyle(dh.el).textDecorationLine || '');
      var u = prop === 'underline' ? !!value : /underline/.test(cur), s = prop === 'strikeout' ? !!value : /line-through/.test(cur);
      var dv = [u ? 'underline' : '', s ? 'line-through' : ''].filter(Boolean).join(' ') || null;
      if (dv) dh.el.style.setProperty('text-decoration', dv); else dh.el.style.removeProperty('text-decoration');
      return { what: 'style', target: dh.target, style: { 'text-decoration': dv } };
    }
    if (prop === 'wordWrap') {
      if (!isLabel(el)) return null;
      var ws = value ? 'normal' : 'nowrap';
      el.style.setProperty('white-space', ws);
      return { what: 'style', target: 'self', style: { 'white-space': ws } };
    }
    if (prop === 'readOnly') {
      if (!textInput(el)) return null;
      el.readOnly = !!value;
      return { what: 'attr', attr: { readonly: value ? true : null } };
    }
    if (prop === 'maxLength') {
      if (!textInput(el) || el.tagName !== 'INPUT') return null;
      var ml = Math.max(0, Math.round(+value || 0));
      if (ml) el.maxLength = ml; else el.removeAttribute('maxlength');
      return { what: 'attr', attr: { maxlength: ml ? String(ml) : null } };
    }
    if (prop === 'checked') {
      var ci = checkInput(el);
      if (!ci) return null;
      ci.checked = !!value;
      if (value) ci.setAttribute('checked', ''); else ci.removeAttribute('checked');
      return { what: 'attr', target: ci === el ? 'self' : 'input', attr: { checked: value ? true : null } };
    }
    if (prop === 'src') {
      if (el.tagName !== 'IMG') return null;
      var sv = String(value == null ? '' : value).trim();
      if (/^(javascript|vbscript):/i.test(sv)) return null;
      el.setAttribute('src', sv);
      return { what: 'attr', attr: { src: sv } };
    }
    if (prop === 'tabOrder') {
      var to = String(value == null ? '' : value).trim();
      if (to === '') { el.removeAttribute('tabindex'); return { what: 'attr', attr: { tabindex: null } }; }
      var tn = Math.max(-1, Math.round(+to));
      if (isNaN(tn)) return null;
      el.tabIndex = tn;
      return { what: 'attr', attr: { tabindex: String(tn) } };
    }
    if (prop === 'visible') st = { display: value ? null : 'none' };
    else if (prop === 'enabled') at = { disabled: value ? null : true };
    else if (prop === 'fontSize') st = { 'font-size': Math.max(1, Math.round(+value || 0)) + 'px' };
    else if (prop === 'bold') st = { 'font-weight': value ? 'bold' : 'normal' };
    else if (prop === 'italic') st = { 'font-style': value ? 'italic' : 'normal' };
    else if (prop === 'fontName') {
      var fn = String(value || '').replace(/["';{}<>]/g, '').trim();
      st = { 'font-family': fn ? (/\s/.test(fn) ? "'" + fn + "'" : fn) : null };
    }
    else if (prop === 'color') st = { color: String(value) };
    else if (prop === 'background') st = { 'background-color': String(value) };
    if (st) {
      var fh = /^(fontSize|bold|italic|fontName|color)$/.test(prop) ? fontHost(el) : { el: el, target: 'self' };
      for (var k in st) { if (st[k] == null) fh.el.style.removeProperty(k); else fh.el.style.setProperty(k, st[k]); }
      return { what: 'style', target: fh.target, style: st };
    }
    if (at) {
      el.disabled = !value;
      return { what: 'attr', attr: at };
    }
    return null;
  }
  /* a caption change: applied, returned as the edit (null = it has no caption to change) */
  function captionChange(el, value) {
    var c = captionOf(el);
    if (!c) return null;
    var v = String(value == null ? '' : value);
    setCaptionDom(el, c.kind, v);
    return { what: 'caption', capKind: c.kind, value: v };
  }
  /* a Left / Top / Width / Height from the panel: the new inline values (not applied yet),
     or why it cannot be done */
  function layoutPlan(el, m) {
    if (lockedOf(el)) return lockedWhy(lockedOf(el));
    var bx = boxOf(el);
    if (!bx) return '「' + (idOf(el) || el.tagName.toLowerCase()) + '」不是用 position:absolute 定位的，位置／大小不能改';
    var asz = autoSizeBlocks(bx, (m.width != null ? 'e' : '') + (m.height != null ? 's' : ''));
    if (asz) return asz;
    var s = snap(bx), cb = curBox(bx.node);
    if ((m.left != null && cb.left == null) || (m.top != null && cb.top == null) ||
      (m.width != null && cb.width == null) || (m.height != null && cb.height == null))
      return '「' + (idOf(el) || el.tagName.toLowerCase()) + '」目前沒有顯示，而它的位置／大小不是寫在自己 style 裡的 px 數值';
    var ch = deltaStyle(s, m.left != null ? m.left - cb.left : 0, m.top != null ? m.top - cb.top : 0,
      m.width != null ? m.width - cb.width : 0, m.height != null ? m.height - cb.height : 0);
    if (!ch) return '「' + (idOf(el) || el.tagName.toLowerCase()) + '」的位置／大小不是寫在自己 style 裡的 px 數值';
    /* force (改回 DFM): these px values are written even where this page already shows them -- the source may say
       something else (a move just made that the page has not been drawn again with) */
    if (m.force) ['left', 'top', 'width', 'height'].forEach(function (k) {
      if (m[k] != null && ch[k] == null && pxOf(s[k]) != null) ch[k] = Math.round(m[k]) + 'px';
    });
    return { bx: bx, ch: ch };
  }
  /* the properties panel with several selected (WPF: the grid sets the value on every selected
     one): the same change on each, ONE batch edit = one undo step. A position / size change
     is all or nothing (a locked one = none); a caption skips the ones that have none. */
  function editAllSel(m) {
    var sel = allSel(), batch = [], plans = [], skipped = 0;
    if (m.type === 'setLayout') {
      for (var i = 0; i < sel.length; i++) {
        var pl = layoutPlan(sel[i], m);
        if (typeof pl === 'string') { post({ type: 'editRefused', why: pl + '，這次 ' + sel.length + ' 個都沒有改' }); return 0; }
        plans.push(pl);
      }
      for (var j = 0; j < sel.length; j++) {
        applyStyle(plans[j].bx.node, plans[j].ch);
        if (Object.keys(plans[j].ch).length) batch.push({ what: 'style', target: plans[j].bx.target, style: plans[j].ch, id: idOf(sel[j]), key: keyOf(sel[j]) });
      }
    } else {
      for (var k = 0; k < sel.length; k++) {
        var p2 = m.type === 'setCaption' ? captionChange(sel[k], m.value) : lookChange(sel[k], m.prop, m.value);
        if (!p2) { skipped++; continue; }
        p2.id = idOf(sel[k]);
        p2.key = keyOf(sel[k]);
        batch.push(p2);
      }
    }
    if (batch.length === 1) { var b1 = batch[0]; b1.type = 'edit'; post(b1); }
    else if (batch.length) post({ type: 'edit', what: 'batch', edits: batch });
    if (skipped) post({ type: 'editRefused', why: '有 ' + skipped + ' 個選取的元件沒有可以直接改的' + (m.type === 'setCaption' ? '文字' : '這個屬性') + '，那些沒有改' });
    var prim = selEl;
    selEl = null;
    select(prim, 'edit', false);
    return batch.length;
  }
  function sendEdit(el, payload) {
    payload.type = 'edit';
    payload.id = idOf(el);
    payload.key = keyOf(el);
    post(payload);
  }
  function commitLayout(el, bx, ch) {
    var keys = Object.keys(ch);
    if (!keys.length) return;
    sendEdit(el, { what: 'style', target: bx.target, style: ch });
  }

  /* 7. selection ------------------------------------------------------------- */
  function revealTabs(el) {
    var panes = [];
    for (var x = el; x && x !== document.body; x = x.parentElement) {
      if (x.classList && x.classList.contains('pcPane')) panes.unshift(x);
    }
    for (var i = 0; i < panes.length; i++) {
      var pane = panes[i];
      if (getComputedStyle(pane).display !== 'none') continue;
      var wrap = pane.parentElement && pane.parentElement.parentElement;
      if (!wrap) continue;
      var tabs = wrap.querySelectorAll(':scope > .pcTabs > .tab');
      for (var j = 0; j < tabs.length; j++) {
        if (tabs[j].getAttribute('data-t') === pane.getAttribute('data-p')) {
          passNext = true;
          try { tabs[j].click(); } finally { passNext = false; }
          break;
        }
      }
    }
  }
  function select(el, origin, scroll, keepMulti) {
    /* another component selected while F2's text box is open: the text is done first */
    if (TXT && el !== TXT.el) endTextEdit(true);
    /* a plain selection ends a multi-selection; an edit or a Ctrl/Shift toggle keeps it */
    if (!keepMulti && origin !== 'edit' && origin !== 'multi') multi = [];
    if (!el) {
      selEl = null; scheduleDraw();
      post({ type: 'select', info: null, origin: origin });
      return;
    }
    if (el === selEl && origin === 'click') { scheduleDraw(); return; }
    selEl = el;
    if (scroll) {
      if (hidden(el)) revealTabs(el);
      try { el.scrollIntoView({ block: 'nearest', inline: 'nearest' }); } catch (e) { /* ignore */ }
    }
    scheduleDraw();
    var info = null;
    try { info = describe(el); } catch (e) { info = { key: keyOf(el), id: idOf(el), tag: el.tagName.toLowerCase(), error: String(e) }; }
    post({ type: 'select', info: info, origin: origin });
  }

  /* 8. event blocking (design mode) and hover -------------------------------- */
  var justDragged = false;
  /* snaplines (WPF): the edges and centres of the siblings in the same container and
     of the container itself, in viewport px, collected once when a drag starts */
  function snapLines(bx, el, skip) {
    var node = bx.node, par = node.offsetParent || document.body;
    /* snaplines off (the artboard toolbar): nothing to snap to, nothing drawn */
    if (!SNAPL) return { xs: [], ys: [], box: par.getBoundingClientRect(), sibs: [] };
    var xs = [], ys = [], sibs = [];
    var pr = par.getBoundingClientRect();
    var pl = pr.left + par.clientLeft, pt = pr.top + par.clientTop;
    xs.push(pl, pl + par.clientWidth, pl + par.clientWidth / 2);
    ys.push(pt, pt + par.clientHeight, pt + par.clientHeight / 2);
    /* its own DFM place (DFM 位置 on): dragged near it, it lands on it */
    var gh = el && GHOST ? ghostOf(el) : null;
    if (gh) {
      xs.push(gh.left, gh.left + gh.width, gh.left + gh.width / 2);
      ys.push(gh.top, gh.top + gh.height, gh.top + gh.height / 2);
    }
    var all = par.querySelectorAll('*');
    var bys = [];
    for (var i = 0; i < all.length && i < 4000; i++) {
      var o = all[i];
      if (o === node || node.contains(o) || o.offsetParent !== par || !o.style || o.style.position !== 'absolute') continue;
      if (skip && skip.indexOf(o) >= 0) continue;   // (a group move: the others move along, their old places mean nothing)
      var r = o.getBoundingClientRect();
      if (!r.width && !r.height) continue;
      xs.push(r.left, r.right, r.left + r.width / 2);
      ys.push(r.top, r.bottom, r.top + r.height / 2);
      sibs.push(r);
      if (bys.length < 400) { var ob = baselineOf(o); if (ob !== null) bys.push(ob); }
    }
    /* WPF: "when the edges and the text of some controls are aligned" -- the text baselines too: bo = where the
       dragged one's text baseline is below its top (null: no text) */
    var own = el ? baselineOf(el) : null;
    var bo = own !== null ? own - node.getBoundingClientRect().top : null;
    return { xs: xs, ys: ys, box: pr, sibs: sibs, bys: bys, bo: bo };
  }
  /* the viewport y of a control's first text baseline, or null: its own text (a label, a button), a panel's caption,
     a text box's value (drawn in the middle of its content box) */
  var BLC = null;
  function baselineOf(el) {
    try {
      var c = captionOf(el);
      if (!c) return null;
      var tn = null, host0 = el;
      if (c.kind === 'pnlCap') host0 = el.querySelector(':scope > .pnlCap');
      else if (c.kind === 'legend') host0 = el.querySelector(':scope > legend');
      var cs = getComputedStyle(c.kind === 'value' || !host0 ? el : host0);
      BLC = BLC || document.createElement('canvas');
      var cx = BLC.getContext('2d');
      cx.font = [cs.fontStyle, cs.fontWeight, cs.fontSize, cs.fontFamily].join(' ');
      var m = cx.measureText('Hg');
      var asc = m.fontBoundingBoxAscent, desc = m.fontBoundingBoxDescent;
      if (!(asc > 0) || !(desc >= 0)) return null;
      var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
      if (c.kind === 'value') {
        if (!c.value) return null;
        var r0 = el.getBoundingClientRect();
        var top = r0.top + (el.clientTop + (parseFloat(cs.paddingTop) || 0)) * z;
        var h = (el.clientHeight - (parseFloat(cs.paddingTop) || 0) - (parseFloat(cs.paddingBottom) || 0)) * z;
        return top + h / 2 + (asc - desc) / 2 * z;
      }
      for (var n = host0 ? host0.firstChild : null; n; n = n.nextSibling) if (n.nodeType === 3 && /\S/.test(n.nodeValue)) { tn = n; break; }
      if (!tn) return null;
      var rg = document.createRange();
      rg.selectNodeContents(tn);
      var rs = rg.getClientRects();
      if (!rs.length || !rs[0].height) return null;
      return rs[0].top + rs[0].height * asc / (asc + desc);
    } catch (x) { return null; }
  }
  var SNAP = 5;
  /* WinForms / WPF spacing snaplines: GAP page px (BCB6's grid step) from a neighbour on the same
     row (hz) / column -- the moving edge that faces it lands there. -> { d, from, to, mid } or null */
  var GAP = 8;   /* the snapSpacing setting (WPF's snapping spacing); 0 = no spacing snaplines */
  function gapSnap(sibs, h, L, R, T, B, hz) {
    if (!(GAP > 0)) return null;
    var g = GAP * (typeof zoom === 'number' && zoom > 0 ? zoom : 1), best = null;
    var mvA = h === 'move' || h.indexOf(hz ? 'w' : 'n') >= 0;   /* its left / top edge moves */
    var mvB = h === 'move' || h.indexOf(hz ? 'e' : 's') >= 0;   /* its right / bottom edge moves */
    for (var i = 0; sibs && i < sibs.length; i++) {
      var s = sibs[i];
      var lo = hz ? Math.max(T, s.top) : Math.max(L, s.left), hi = hz ? Math.min(B, s.bottom) : Math.min(R, s.right);
      if (hi - lo <= 0) continue;   /* not on the same row / column */
      var mid = (lo + hi) / 2;
      if (mvA) { var a = hz ? s.right : s.bottom, dA = a + g - (hz ? L : T); if (Math.abs(dA) <= SNAP && (!best || Math.abs(dA) < Math.abs(best.d))) best = { d: dA, from: a, to: a + g, mid: mid }; }
      if (mvB) { var b = hz ? s.left : s.top, dB = b - g - (hz ? R : B); if (Math.abs(dB) <= SNAP && (!best || Math.abs(dB) < Math.abs(best.d))) best = { d: dB, from: b - g, to: b, mid: mid }; }
    }
    return best;
  }
  /* best correction so that one of `edges` lands on one of `lines` (within SNAP) */
  function snapTo(edges, lines) {
    var best = null;
    for (var i = 0; i < edges.length; i++) {
      for (var j = 0; j < lines.length; j++) {
        var d = lines[j] - edges[i];
        if (Math.abs(d) <= SNAP && (best === null || Math.abs(d) < Math.abs(best.d))) best = { d: d, at: lines[j] };
      }
    }
    return best;
  }
  /* locked in the designer (WPF: the lock in the document outline): it and what is
     inside it cannot be moved or resized here; selecting and its properties still work */
  var LOCK = {};
  function lockedOf(el) {
    var fr = formRoot();
    for (var x = el; x && x !== document.body && x !== document.documentElement; x = x.parentElement) {
      var nm = x === fr ? '@form' : x.id;
      if (nm && LOCK[nm]) return nm;
    }
    return '';
  }
  function lockedWhy(name) { return '「' + name + '」鎖定了（元件樹上的鎖頭），在設計畫面不能移動或改大小'; }
  /* BCB6: an AutoSize label cannot be resized (its size follows the text) -- the reason, or '' */
  function autoSizeBlocks(bx, how) {
    var s = bx.node.style;
    if ((/[ew]/.test(how) && s.width === 'auto') || (/[ns]/.test(how) && s.height === 'auto')) {
      return '「' + (idOf(bx.node) || idOf(bx.node.firstElementChild) || bx.node.tagName.toLowerCase()) + '」的 AutoSize 開著（大小跟著文字），先在屬性面板把 AutoSize 關掉才能改大小（跟 BCB6 一樣）';
    }
    return '';
  }
  function startDrag(el, bx, how, e, group, copy) {
    /* (a copy leaves the originals where they are: a lock does not stop it) */
    var lk = copy ? null : lockedOf(el);
    for (var gi = 0; !lk && !copy && group && gi < group.length; gi++) lk = lockedOf(group[gi].el);
    if (lk) { drag = { locked: lk, el: el, x: e.clientX, y: e.clientY, active: false, told: false }; return; }
    if (how !== 'move') {
      var asb = autoSizeBlocks(bx, how);
      for (var ga = 0; !asb && group && ga < group.length; ga++) asb = autoSizeBlocks(group[ga].bx, how);
      if (asb) { post({ type: 'editRefused', why: asb }); return; }
    }
    drag = { el: el, bx: bx, how: how, s0: snap(bx), x: e.clientX, y: e.clientY, active: how !== 'move', last: null,
      r0: bx.node.getBoundingClientRect(), lines: null, items: null, collapse: null, copy: !!copy, dL: 0, dT: 0,
      /* (Alt not held at some moment of the drag: an Alt at the release is Blend's reparent; held from the press it is
         only VS's 'no snapping' -- AI 20261001: the two meant the same key, a free move landed inside a Panel) */
      altUp: !e.altKey };
    /* moving / resizing a multi-selection changes every selected component by the same delta */
    if (group && group.length > 1) {
      drag.items = group.map(function (g) { return { el: g.el, bx: g.bx, s0: snap(g.bx), last: null }; });
    }
  }
  /* Ctrl/Shift+click: add to / remove from the selection; the last one clicked is primary */
  function toggleSel(el) {
    if (!el) return;
    if (el === selEl) {
      if (multi.length) { var np = multi.pop(); selEl = null; select(np, 'multi', false); }
      return;
    }
    var i = multi.indexOf(el);
    if (i >= 0) { multi.splice(i, 1); var p = selEl; selEl = null; select(p, 'multi', false); return; }
    if (selEl) multi.push(selEl);
    selEl = null;
    select(el, 'multi', false);
  }
  /* several components' edits as ONE message = one source edit = one undo step */
  function commitGroup(items) {
    var edits = [];
    for (var i = 0; i < items.length; i++) {
      var it = items[i];
      if (it.last && Object.keys(it.last).length) edits.push({ what: 'style', id: idOf(it.el), key: keyOf(it.el), target: it.bx.target, style: it.last });
    }
    if (edits.length === 1) post({ type: 'edit', what: 'style', id: edits[0].id, key: edits[0].key, target: edits[0].target, style: edits[0].style });
    else if (edits.length) post({ type: 'edit', what: 'batch', edits: edits });
    return edits.length;
  }
  /* WPF / BCB6 Format > Horizontal / Vertical Spacing > Make Equal (3 or more: the outer two
     stay, the gaps between neighbours become the same) and Center in Window (the selection as
     one block, centred in its container). Here the primary is no reference: any may move.
     A locked one or one without its own px position: nothing moves, and it is said why. */
  /* AI(W906-HTDESIGNER) 20261001: + WinForms' / BCB6's Format menu (learn.microsoft.com "Format menu", the VBA forms
     designer "Increase, to increase the space between controls by one grid block"; "Remove ... immediately adjacent";
     "The object with focus does not move"): Align to Grid / Size to Grid (every selected one, on GRID.size), and
     Horizontal / Vertical Spacing Increase / Decrease (one grid block per gap; decrease never past touching) / Remove
     (side by side) -- the primary one stays, the others move away from / towards it */
  var ARRANGE = { hspace: 1, vspace: 1, hcenterIn: 1, vcenterIn: 1, gridPos: 1, gridSize: 1,
    hspaceInc: 1, hspaceDec: 1, hspaceRemove: 1, vspaceInc: 1, vspaceDec: 1, vspaceRemove: 1 };
  function toGrid(how) {
    var sel = allSel(), items = [], lk = '', bad = '';
    for (var i = 0; i < sel.length; i++) {
      var bx = boxOf(sel[i]);
      if (!bx || bx.root) continue;   /* (the form: no position of its own) */
      var l = lockedOf(sel[i]);
      if (l) { lk = lk || l; continue; }
      var s = snap(bx), dL = 0, dT = 0, dW = 0, dH = 0;
      if (how === 'gridPos') {
        var A = pxOf(s.left), T = pxOf(s.top);
        if (A !== null) dL = snapG(A) - A;
        if (T !== null) dT = snapG(T) - T;
      } else {
        var W = pxOf(s.width), H = pxOf(s.height);
        if (W !== null) dW = Math.max(GRID.size, snapG(W)) - W;
        if (H !== null) dH = Math.max(GRID.size, snapG(H)) - H;
      }
      if (!dL && !dT && !dW && !dH) continue;
      var ch = deltaStyle(s, dL, dT, dW, dH);
      if (!ch) { bad = bad || (idOf(sel[i]) || sel[i].tagName.toLowerCase()); continue; }
      applyStyle(bx.node, ch);
      items.push({ el: sel[i], bx: bx, last: ch });
    }
    var done = items.length ? commitGroup(items) : 0;
    if (bad) post({ type: 'editRefused', why: '「' + bad + '」的位置／大小不是寫在自己 style 裡的 px 數值，它沒有對齊格線' });
    else if (lk) post({ type: 'editRefused', why: lockedWhy(lk) + '，它沒有對齊格線' });
    else if (!done) post({ type: 'editRefused', why: (how === 'gridPos' ? '位置' : '大小') + '已經在格線上（' + GRID.size + ' px），沒有要改的' });
    var prim = selEl;
    selEl = null;
    select(prim, 'edit', false);
    return done;
  }
  function arrange(how) {
    if (how === 'gridPos' || how === 'gridSize') return toGrid(how);
    var sel = allSel();
    if (!sel.length) return 0;
    var z = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
    var list = [];
    for (var i = 0; i < sel.length; i++) {
      var bx = boxOf(sel[i]);
      if (!bx || bx.root || !bx.node.getClientRects().length) {
        post({ type: 'editRefused', why: '「' + (idOf(sel[i]) || sel[i].tagName.toLowerCase()) + '」的位置不是寫在自己 style 裡的 px 數值（或現在沒顯示），這次沒有調整' });
        return 0;
      }
      var lk = lockedOf(sel[i]);
      if (lk) { post({ type: 'editRefused', why: lockedWhy(lk) + '，這次沒有調整' }); return 0; }
      list.push({ el: sel[i], bx: bx, r: bx.node.getBoundingClientRect(), last: null });
    }
    var h = how.charAt(0) === 'h';
    var moves = [];   /* screen px per item, same order as list */
    var step = /^[hv]space(Inc|Dec|Remove)$/.exec(how);
    if (step) {
      if (list.length < 2) { post({ type: 'editRefused', why: '間距要選 2 個以上的元件（Ctrl＋點）' }); return 0; }
      var ord = list.slice().sort(function (a, b) { return h ? a.r.left - b.r.left : a.r.top - b.r.top; });
      var pi = 0;
      for (var oi = 0; oi < ord.length; oi++) if (ord[oi].el === selEl) pi = oi;
      var size = function (it) { return h ? it.r.width : it.r.height; };
      var lo0 = function (it) { return h ? it.r.left : it.r.top; }, hi0 = function (it) { return h ? it.r.right : it.r.bottom; };
      var G = GRID.size * z, acc, k3;
      if (step[1] === 'Remove') {
        /* side by side, from the primary one outwards */
        var edge = hi0(ord[pi]);
        for (k3 = pi + 1; k3 < ord.length; k3++) { ord[k3].d = edge - lo0(ord[k3]); edge += size(ord[k3]); }
        edge = lo0(ord[pi]);
        for (k3 = pi - 1; k3 >= 0; k3--) { ord[k3].d = edge - hi0(ord[k3]); edge -= size(ord[k3]); }
      } else {
        /* each gap one grid block more / less (less: never past touching); the k-th one away moves k gaps' worth */
        var inc = step[1] === 'Inc';
        var by = function (gap) { return inc ? G : -Math.min(G, Math.max(0, gap)); };
        for (acc = 0, k3 = pi + 1; k3 < ord.length; k3++) { acc += by(lo0(ord[k3]) - hi0(ord[k3 - 1])); ord[k3].d = acc; }
        for (acc = 0, k3 = pi - 1; k3 >= 0; k3--) { acc += by(lo0(ord[k3 + 1]) - hi0(ord[k3])); ord[k3].d = -acc; }
      }
      for (var k4 = 0; k4 < list.length; k4++) moves.push(list[k4].d || 0);
    } else if (how === 'hspace' || how === 'vspace') {
      if (list.length < 3) { post({ type: 'editRefused', why: '等距要選 3 個以上的元件（Ctrl＋點）' }); return 0; }
      var order = list.slice().sort(function (a, b) { return h ? a.r.left - b.r.left : a.r.top - b.r.top; });
      var first = order[0].r, lastR = order[order.length - 1].r, sum = 0;
      for (var j = 0; j < order.length; j++) sum += h ? order[j].r.width : order[j].r.height;
      var gap = ((h ? lastR.right - first.left : lastR.bottom - first.top) - sum) / (order.length - 1);
      var at = h ? first.right : first.bottom;
      for (var k = 1; k < order.length - 1; k++) {
        order[k].d = at + gap - (h ? order[k].r.left : order[k].r.top);
        at += gap + (h ? order[k].r.width : order[k].r.height);
      }
      for (var k2 = 0; k2 < list.length; k2++) moves.push(list[k2].d || 0);
    } else {
      var c = list[0].bx.node.offsetParent;
      for (var q = 1; q < list.length; q++) if (list[q].bx.node.offsetParent !== c) {
        post({ type: 'editRefused', why: '選取的元件不在同一個容器裡，沒有置中' });
        return 0;
      }
      if (!c || c === document.body || c === document.documentElement) c = formRoot();
      if (!c) return 0;
      var cr = c.getBoundingClientRect();
      var lo = Infinity, hi = -Infinity;
      for (var q2 = 0; q2 < list.length; q2++) {
        lo = Math.min(lo, h ? list[q2].r.left : list[q2].r.top);
        hi = Math.max(hi, h ? list[q2].r.right : list[q2].r.bottom);
      }
      var mid = h ? cr.left + (c.clientLeft + c.clientWidth / 2) * z : cr.top + (c.clientTop + c.clientHeight / 2) * z;
      var dc = mid - (lo + hi) / 2;
      for (var q3 = 0; q3 < list.length; q3++) moves.push(dc);
    }
    var items = [];
    for (var n = 0; n < list.length; n++) {
      if (Math.abs(moves[n] / z) < 0.5) continue;
      var ch = deltaStyle(snap(list[n].bx), h ? moves[n] / z : 0, h ? 0 : moves[n] / z, 0, 0);
      if (!ch) {
        post({ type: 'editRefused', why: '「' + (idOf(list[n].el) || list[n].el.tagName.toLowerCase()) + '」的位置不是寫在自己 style 裡的 px 數值，這次沒有調整' });
        return 0;
      }
      items.push({ el: list[n].el, bx: list[n].bx, last: ch });
    }
    for (var a = 0; a < items.length; a++) applyStyle(items[a].bx.node, items[a].last);
    var done = items.length ? commitGroup(items) : 0;
    if (!done) post({ type: 'editRefused', why: step ? (step[1] === 'Remove' ? '已經緊靠在一起了，沒有要改的' : '間距已經不能再縮小了') : how === 'hspace' || how === 'vspace' ? '已經是等距了，沒有要改的' : '已經置中了，沒有要改的' });
    var prim = selEl;
    selEl = null;
    select(prim, 'edit', false);
    return done;
  }
  function onBlock(e) {
    var t = e.type;
    if (TABSET && mode === 'design') {
      /* Tab order by clicks: the press numbers the one under it; right = done; the page never sees it */
      if (t === 'mousedown') {
        if (e.button === 0) tabSetClick(pick(e.target, false));
        else if (e.button === 2) endTabSet(true);
      }
      e.stopImmediatePropagation();
      e.preventDefault();
      return;
    }
    if (PLACE && mode === 'design') {
      /* a toolbox tool in the hand: the press starts it, the release puts it down; right = give up */
      if (t === 'mousedown') {
        if (e.button === 0) PLACE.drag = { x0: e.clientX, y0: e.clientY, x1: e.clientX, y1: e.clientY };
        else if (e.button === 2) cancelPlace();
      } else if (t === 'mouseup' && PLACE.drag) finishPlace();
      e.stopImmediatePropagation();
      e.preventDefault();
      return;
    }
    if (PAN.drag && /^(mouseup|pointerup|click|auxclick)$/.test(t)) {
      /* the end of a pan: the page never sees it */
      if (t === 'mouseup') endPan();
      e.stopImmediatePropagation();
      e.preventDefault();
      return;
    }
    if ((drag || marquee) && t === 'mouseup') endDrag(e);
    if (host && e.target === host) {
      /* our own overlay: a resize handle (or the pan shield), never the page's business */
      var real = e.composedPath ? e.composedPath()[0] : null;
      /* F2's text box: its own clicks (the caret, a selection), nothing else sees them */
      if (TXT && real === TXT.inp) { e.stopImmediatePropagation(); return; }
      /* the artboard toolbar / its zoom list / the information bar: a click does its job, nothing else sees it */
      var ab = real && real.closest ? real.closest('[data-atb]') : null;
      if (ab || (real && real.closest && real.closest('.atb, .atm, .ib'))) {
        if (ab && t === 'click') toolbarAct(ab.getAttribute('data-atb'));
        e.stopImmediatePropagation();
        e.preventDefault();
        return;
      }
      if (mode === 'design') {
        if (t === 'mousedown' && atmEl && atmEl.style.display === 'block') zoomMenu(false);
        var dir = real && real.getAttribute ? real.getAttribute('data-dir') : null;
        if (ZSP && t === 'mousedown' && e.button === 0) zoomAt(e.clientX, e.clientY, ZSP.out ? -1 : 1);   // Blend: Ctrl(+Alt)+Space+click
        else if (t === 'mousedown' && (PAN.space ? e.button === 0 : e.button === 1)) startPan(e);
        else if (t === 'mousedown' && e.button === 0 && dir && selEl) {
          var hb = boxOf(selEl);
          /* WinForms / WPF: with several selected a handle resizes all of them by the same amount
             (as Ctrl+arrows do); the form resizes alone */
          var rg = hb && !hb.root ? allSel().map(function (x) { return { el: x, bx: boxOf(x) }; }).filter(function (g) { return g.bx && !g.bx.root; }) : [];
          if (hb) startDrag(selEl, hb, dir, e, rg.length > 1 ? rg : null);
        }
        e.stopImmediatePropagation();
        e.preventDefault();
      }
      return;
    }
    if (t === 'click' || t === 'auxclick') {
      var a = closestSafe(e.target, 'a[href]');
      if (a) {
        var h = a.getAttribute('href') || '';
        if (h && h.charAt(0) !== '#' && !/^javascript:/i.test(h)) e.preventDefault();
      }
    }
    if (t === 'submit') e.preventDefault();
    if (passNext || mode !== 'design') return;
    if (t === 'contextmenu') {
      /* WPF: right-click selects (unless it is already in the selection) and opens the
         designer's menu. The page never sees it; VS Code shows our webview/context menu,
         so no preventDefault here. */
      e.stopImmediatePropagation();
      lastCtx = { x: e.clientX, y: e.clientY };   /* (選取這裡的元件…: what is under this point) */
      var cm = pick(e.target, e.altKey);
      if (cm && cm !== selEl && multi.indexOf(cm) < 0) select(cm, 'click', false);
      return;
    }
    if (t === 'mousedown' && atmEl && atmEl.style.display === 'block') zoomMenu(false);   // (a click elsewhere closes the zoom list)
    if (t === 'mousedown' && e.button === 1) { e.stopImmediatePropagation(); e.preventDefault(); startPan(e); return; }   // middle button: pan
    if (t === 'mousedown' && e.button !== 0) { e.stopImmediatePropagation(); return; }
    var toggle = e.ctrlKey || e.metaKey || e.shiftKey;
    if (isTab(e.target) && !e.altKey && !toggle) {
      if (t === 'click') select(pick(e.target, false), 'click', false);
      return;
    }
    e.stopImmediatePropagation();
    if (t !== 'touchstart' || e.cancelable) e.preventDefault();
    if (t === 'mousedown' && e.button === 0) {
      var md = e.altKey && !toggle ? (altPick(e.clientX, e.clientY) || pick(e.target, true)) : pick(e.target, e.altKey);
      if (toggle && e.ctrlKey && !e.shiftKey && !e.metaKey && md && md !== formRoot()) {
        /* WinForms / WPF: Ctrl+drag = a copy of the selection dropped where it is let go (the originals stay);
           Ctrl+click without moving = add to / take out of the selection, as before. Pressed on one not
           selected: it joins now (as the click would); on a selected one: it leaves on the release, if no drag */
        var wasIn = md === selEl || multi.indexOf(md) >= 0;
        if (!wasIn) toggleSel(md);
        var cb = boxOf(md);
        if (cb && !cb.root) {
          var cg = allSel().map(function (x) { return { el: x, bx: boxOf(x) }; }).filter(function (g) { return g.bx && !g.bx.root; });
          startDrag(md, cb, 'move', e, cg, true);
          if (drag) drag.toggleOnClick = wasIn ? md : null;
        } else if (wasIn) toggleSel(md);
        return;
      }
      if (toggle && e.shiftKey && !e.ctrlKey && !e.metaKey && md && md !== formRoot()) {
        /* Blend: "Select by drawing a marquee: hold down Shift and drag" -- from anywhere, on a component too; a Shift+click
           without a drag still adds / takes it out (finishMarquee) */
        marquee = { x0: e.clientX, y0: e.clientY, x1: e.clientX, y1: e.clientY, on: false, shiftEl: md };
        return;
      }
      if (toggle) { toggleSel(md); return; }            // WPF: Ctrl/Shift+click = multi-select
      if (!md || md === formRoot()) {
        /* pressed on the form's own background (it gets selected) or outside the form (nothing is):
           a drag from here is a rubber band */
        select(md, 'click', false);
        marquee = { x0: e.clientX, y0: e.clientY, x1: e.clientX, y1: e.clientY, on: false };
        return;
      }
      /* WPF: pressing selects, and dragging from there moves (the whole selection
         when it is pressed on one of its members) */
      var inSel = !!md && (md === selEl || multi.indexOf(md) >= 0);
      if (!inSel) select(md, 'click', false);
      else if (md !== selEl) { multi.splice(multi.indexOf(md), 1); if (selEl) multi.push(selEl); selEl = null; select(md, 'multi', false); }
      var bx = md ? boxOf(md) : null;
      if (bx && !bx.root) {
        var group = allSel().map(function (x) { return { el: x, bx: boxOf(x) }; }).filter(function (g) { return g.bx && !g.bx.root; });
        startDrag(md, bx, 'move', e, group);
        if (inSel && multi.length) drag.collapse = md;   // a click (no move) on a member keeps only it
      }
    } else if (t === 'click') {
      if (toggle) return;
      if (e.altKey) return;   // (Alt: the press already picked the layer)
      if (justDragged) { justDragged = false; return; }
      select(pick(e.target, e.altKey), 'click', false);
    } else if (t === 'dblclick') {
      if (toggle) return;
      var el = pick(e.target, e.altKey);
      select(el, 'click', false);
      if (el) post({ type: 'dblclick', key: keyOf(el) });
    }
  }
  ['pointerdown', 'pointerup', 'mousedown', 'mouseup', 'click', 'dblclick', 'auxclick', 'contextmenu',
    'touchstart', 'touchend', 'dragstart', 'submit'].forEach(function (t) {
    origAdd.call(window, t, onBlock, { capture: true, passive: false });
  });
  origAdd.call(window, 'mousemove', function (e) {
    if (PLACE && PLACE.drag) { PLACE.drag.x1 = e.clientX; PLACE.drag.y1 = e.clientY; placeBand(); e.preventDefault(); e.stopImmediatePropagation(); return; }
    if (PAN.drag) { movePan(e); e.preventDefault(); e.stopImmediatePropagation(); return; }
    if (marquee) {
      marquee.x1 = e.clientX;
      marquee.y1 = e.clientY;
      if (!marquee.on && Math.abs(marquee.x1 - marquee.x0) + Math.abs(marquee.y1 - marquee.y0) >= 4) marquee.on = true;
      drawMarquee();
      e.preventDefault();
      e.stopImmediatePropagation();
      return;
    }
    if (drag) {
      var dx = e.clientX - drag.x, dy = e.clientY - drag.y;
      if (!e.altKey) drag.altUp = true;
      if (drag.locked) {
        /* pressed on a locked component: nothing moves; said once, when it would have */
        if (!drag.told && Math.abs(dx) + Math.abs(dy) >= 3) { drag.told = true; post({ type: 'editRefused', why: lockedWhy(drag.locked) }); }
        e.preventDefault();
        e.stopImmediatePropagation();
        return;
      }
      if (!drag.active) {
        if (Math.abs(dx) + Math.abs(dy) < 3) return;   // a click, not a drag (yet)
        drag.active = true;
      }
      var h = drag.how;
      /* snap the moving edges to the siblings' / container's lines (Alt = no snapping) */
      var gx = null, gy = null, gpx = null, gpy = null;
      if (!e.altKey && !GRID.on) {       // (with the grid on, the grid is what it snaps to)
        if (!drag.lines) drag.lines = snapLines(drag.bx, drag.el, drag.items ? drag.items.map(function (it) { return it.bx.node; }) : null);
        var r0 = drag.r0;
        var L = r0.left + dx, R = r0.right + dx, T = r0.top + dy, B = r0.bottom + dy;
        var ex, ey;
        if (h === 'move') { ex = [L, R, (L + R) / 2]; ey = [T, B, (T + B) / 2]; }
        else {
          ex = h.indexOf('e') >= 0 ? [R] : h.indexOf('w') >= 0 ? [L] : [];
          ey = h.indexOf('s') >= 0 ? [B] : h.indexOf('n') >= 0 ? [T] : [];
        }
        var sx = snapTo(ex, drag.lines.xs), sy = snapTo(ey, drag.lines.ys);
        /* the text baseline on a neighbour's text baseline (a move only), when that is nearer than an edge line */
        var sbl = h === 'move' && drag.lines.bo !== null && drag.lines.bo !== undefined && drag.lines.bys && drag.lines.bys.length
          ? snapTo([T + drag.lines.bo], drag.lines.bys) : null;
        drag.baseline = false;
        if (sbl && (!sy || Math.abs(sbl.d) < Math.abs(sy.d))) { sy = sbl; drag.baseline = true; }
        /* the nearer of an edge line and a spacing line wins */
        /* (for a handle only the sides it moves have moved: the row / column test uses the others as they are) */
        var mv = h === 'move';
        var gL = r0.left + (mv || h.indexOf('w') >= 0 ? dx : 0), gR = r0.right + (mv || h.indexOf('e') >= 0 ? dx : 0);
        var gT = r0.top + (mv || h.indexOf('n') >= 0 ? dy : 0), gB = r0.bottom + (mv || h.indexOf('s') >= 0 ? dy : 0);
        var qx = gapSnap(drag.lines.sibs, h, gL, gR, gT, gB, true), qy = gapSnap(drag.lines.sibs, h, gL, gR, gT, gB, false);
        if (qx && (!sx || Math.abs(qx.d) < Math.abs(sx.d))) { dx += qx.d; gpx = qx; }
        else if (sx) { dx += sx.d; gx = sx.at; }
        if (qy && (!sy || Math.abs(qy.d) < Math.abs(sy.d))) { dy += qy.d; gpy = qy; }
        else if (sy) { dy += sy.d; gy = sy.at; }
      }
      drag.guide = { x: gx, y: gy };
      drag.gap = { x: gpx, y: gpy };
      dx /= zoom;      // screen px -> CSS px of the page
      dy /= zoom;
      var dL = 0, dT = 0, dW = 0, dH = 0;
      if (h === 'move') { dL = dx; dT = dy; }
      else {
        if (h.indexOf('e') >= 0) dW = dx;
        if (h.indexOf('w') >= 0) { dL = dx; dW = -dx; }
        if (h.indexOf('s') >= 0) dH = dy;
        if (h.indexOf('n') >= 0) { dT = dy; dH = -dy; }
      }
      if (GRID.on && !e.altKey) {
        /* the moving edges land on the grid (in the element's own px: its style's left/top/width/height) */
        var A = pxOf(drag.s0.left), Tg = pxOf(drag.s0.top), Wg = pxOf(drag.s0.width), Hg = pxOf(drag.s0.height);
        if (h === 'move') {
          if (A !== null) dL = snapG(A + dL) - A;
          if (Tg !== null) dT = snapG(Tg + dT) - Tg;
        } else {
          if (h.indexOf('e') >= 0 && Wg !== null) dW = snapG(Wg + dW) - Wg;
          if (h.indexOf('w') >= 0 && A !== null) { dL = snapG(A + dL) - A; dW = -dL; }
          if (h.indexOf('s') >= 0 && Hg !== null) dH = snapG(Hg + dH) - Hg;
          if (h.indexOf('n') >= 0 && Tg !== null) { dT = snapG(Tg + dT) - Tg; dH = -dT; }
        }
      }
      /* WPF / Blend: Shift on a corner handle keeps the proportions (the size it had when the drag started) */
      if (e.shiftKey && h.length === 2) {
        var W0 = pxOf(drag.s0.width), H0 = pxOf(drag.s0.height);
        if (W0 === null) W0 = drag.s0.ow;
        if (H0 === null) H0 = drag.s0.oh;
        if (W0 > 0 && H0 > 0) {
          var fw = (W0 + dW) / W0, fh = (H0 + dH) / H0;
          var f = Math.abs(fw - 1) >= Math.abs(fh - 1) ? fw : fh;
          dW = Math.max(2, W0 * f) - W0;
          dH = Math.max(2, H0 * f) - H0;
          if (h.indexOf('w') >= 0) dL = -dW;
          if (h.indexOf('n') >= 0) dT = -dH;
        }
      }
      drag.dL = dL;
      drag.dT = dT;
      if (drag.items) {
        for (var gi = 0; gi < drag.items.length; gi++) {
          var it = drag.items[gi];
          var gch = deltaStyle(it.s0, dL, dT, dW, dH);
          if (gch) { applyStyle(it.bx.node, gch); it.last = gch; } else drag.refused = true;
        }
        drag.last = drag.items[0] ? drag.items[0].last : null;
      } else {
        var ch = deltaStyle(drag.s0, dL, dT, dW, dH);
        if (ch) { applyStyle(drag.bx.node, ch); drag.last = ch; }
        else drag.refused = true;
      }
      e.preventDefault();
      e.stopImmediatePropagation();
      scheduleDraw();
      return;
    }
    if (mode !== 'design') { if (hoverEl) { hoverEl = null; scheduleDraw(); } return; }
    var el = pick(e.target, e.altKey);
    if (el !== hoverEl) { hoverEl = el; scheduleDraw(); }
  }, true);
  /* called from onBlock (which stops mouseup for the page) and, as a fallback, here */
  function endDrag(ev) {
    if (PAN.drag) { endPan(); return; }
    if (marquee) { finishMarquee(); return; }
    if (!drag) return;
    var d = drag;
    drag = null;
    if (d.active && !d.copy && !d.locked && d.how === 'move' && ev && ev.altKey && d.altUp && typeof ev.clientX === 'number') {
      /* Blend: "Reparent an object: drag the object over a layout panel and press Alt" (before letting go) -- the
         originals go back, the extension moves them into the container under the pointer, where they were let go */
      var rItems = d.items || [{ el: d.el, bx: d.bx, s0: d.s0 }];
      var zr = typeof zoom === 'number' && zoom > 0 ? zoom : 1;
      var offs = rItems.map(function (it) {
        var rr = it.bx.node.getBoundingClientRect();
        return { id: idOf(it.el), dx: Math.round((rr.left - ev.clientX) / zr), dy: Math.round((rr.top - ev.clientY) / zr) };
      });
      var tg = placeTargets(ev.clientX, ev.clientY, rItems.map(function (it) { return it.bx.node; }));
      for (var ri = 0; ri < rItems.length; ri++) {
        var r0s = rItems[ri].s0;
        applyStyle(rItems[ri].bx.node, { left: r0s.left, top: r0s.top, right: r0s.right, bottom: r0s.bottom, width: r0s.width, height: r0s.height });
      }
      justDragged = true;
      setTimeout(function () { justDragged = false; }, 0);
      if (offs.some(function (o) { return !o.id; })) post({ type: 'editRefused', why: '有元件沒有名稱（id），不能換容器' });
      else post({ type: 'reparentDrop', offs: offs, targets: tg });
      scheduleDraw();
      return;
    }
    if (d.locked) {
      if (d.told) { justDragged = true; setTimeout(function () { justDragged = false; }, 0); }
      scheduleDraw();
      return;
    }
    if (d.copy) {
      /* Ctrl+drag: the originals go back where they were; the extension makes the copies there (ONE edit) */
      var cItems = d.items || [{ el: d.el, bx: d.bx, s0: d.s0 }];
      for (var ci = 0; ci < cItems.length; ci++) {
        var c0 = cItems[ci].s0;
        applyStyle(cItems[ci].bx.node, { left: c0.left, top: c0.top, right: c0.right, bottom: c0.bottom, width: c0.width, height: c0.height });
      }
      if (d.active) {
        justDragged = true;
        setTimeout(function () { justDragged = false; }, 0);
        var cdx = Math.round(d.dL), cdy = Math.round(d.dT);
        if (d.refused) post({ type: 'editRefused', why: '有元件的位置不是寫在自己 style 裡的 px 數值，沒有複製' });
        else if (cdx || cdy) post({ type: 'copyDrop', ids: cItems.map(function (it) { return idOf(it.el); }).filter(Boolean), dx: cdx, dy: cdy });
      } else if (d.toggleOnClick) toggleSel(d.toggleOnClick);
      scheduleDraw();
      return;
    }
    if (d.active) {
      justDragged = true;
      setTimeout(function () { justDragged = false; }, 0);
      if (d.items) { commitGroup(d.items); selEl = null; select(d.el, 'edit', false); }
      else if (d.last) { commitLayout(d.el, d.bx, d.last); selEl = null; select(d.el, 'edit', false); }
      if (d.refused) post({ type: 'editRefused', why: '有元件的位置／大小不是寫在自己 style 裡的 px 數值，那個沒有移動' });
    } else if (d.collapse) {
      selEl = null;
      select(d.collapse, 'click', false);   // clears the multi-selection
    }
    scheduleDraw();
  }
  /* Esc while dragging (Windows' rule for any drag: the drag is cancelled): everything back where it was, nothing written */
  function cancelDrag() {
    if (marquee) { marquee = null; if (mqEl) mqEl.style.display = 'none'; }
    var d = drag;
    drag = null;
    if (d && !d.locked) {
      (d.items || [{ bx: d.bx, s0: d.s0 }]).forEach(function (it) {
        var s = it.s0;
        applyStyle(it.bx.node, { left: s.left, top: s.top, right: s.right, bottom: s.bottom, width: s.width, height: s.height });
      });
      if (d.active) post({ type: 'note', text: '已取消（Esc）：放回原位，沒有改' });
    }
    justDragged = true;
    setTimeout(function () { justDragged = false; }, 0);
    scheduleDraw();
  }
  origAdd.call(window, 'mouseup', endDrag, true);
  origAdd.call(document, 'mouseleave', function () { hoverEl = null; scheduleDraw(); }, true);
  origAdd.call(window, 'scroll', scheduleDraw, { capture: true, passive: true });
  origAdd.call(window, 'wheel', function (e) {
    /* WPF's "Zoom by using" (Options > XAML Designer): Ctrl+wheel (the default), the wheel alone, or Alt+wheel */
    var zk = WHEELZ === 'wheel' ? !e.ctrlKey && !e.altKey && !e.shiftKey : WHEELZ === 'alt' ? e.altKey && !e.ctrlKey : e.ctrlKey;
    if (!zk) {
      /* (Ctrl+wheel when it is not the zoom: a scroll -- never the webview's own page zoom) */
      if (e.ctrlKey) { e.preventDefault(); e.stopImmediatePropagation(); try { window.scrollBy(e.deltaX || 0, e.deltaY || 0); } catch (x) { /* ignore */ } }
      return;                            // otherwise the wheel scrolls as usual
    }
    e.preventDefault();
    e.stopImmediatePropagation();
    /* about the pointer: what is under it stays under it (VS / Blend; AI 20261001 -- it used to drift away) */
    zoomAt(e.clientX, e.clientY, e.deltaY < 0 ? 1 : -1);
  }, { capture: true, passive: false });
  origAdd.call(window, 'resize', function () { drawBadge(); scheduleDraw(); }, true);   /* (the hint's room beside the toolbar too) */
  /* keyboard, design mode, a component selected -- the WPF designer's keys:
       arrows = nudge 1px (Shift = 10px)      Ctrl+arrows = resize 1px (Ctrl+Shift = 10px)
       Esc = select the parent                Tab / Shift+Tab = next / previous component
       Enter = open the default event's code
     Every other key (Ctrl+Z, Ctrl+S, VS Code shortcuts) is left alone. */
  function compParent(el) {
    var fr = formRoot();
    for (var p = el.parentElement; p && p !== document.body && p !== document.documentElement; p = p.parentElement) {
      if (p.id || p === fr) return p;
    }
    return null;
  }
  function allComps() {
    var out = [];
    var fr = formRoot();
    if (fr) out.push(fr);
    var all = document.body ? document.body.querySelectorAll('[id]') : [];
    for (var i = 0; i < all.length; i++) if (!/^(SCRIPT|STYLE|TEMPLATE|LINK|META)$/.test(all[i].tagName)) out.push(all[i]);
    return out;
  }
  /* WPF Ctrl+A: every component in the same container as the selection (with the form or
     nothing selected: the form's own); the selection stays the primary when it is one of them */
  /* the container: an ancestor with an id, the form, or a tab sheet (a .pcPane has no id) */
  function contOf(x) {
    var fr = formRoot();
    for (var p = x.parentElement; p && p !== document.body && p !== document.documentElement; p = p.parentElement) {
      if (p.id || p === fr || (p.classList && p.classList.contains('pcPane'))) return p;
    }
    return null;
  }
  /* 選取同型別: every component of the selection's VCL class -- on the whole page, or only in
     the selection's container (Ctrl+A's). Not the ones hidden in the designer (the eye). The
     selection stays the primary; with R30 one change in the panel then goes to all of them. */
  function selectSameType(scope) {
    if (!selEl) return 0;
    var fr = formRoot();
    var cls = vclOf(selEl).cls;
    if (!cls || selEl === fr) {
      post({ type: 'editRefused', why: selEl === fr ? '選的是表單本身，沒有同型別的元件' : '不知道「' + (idOf(selEl) || selEl.tagName.toLowerCase()) + '」的型別（DFM 裡沒有它），沒有選' });
      return 0;
    }
    var par = scope === 'container' ? contOf(selEl) : null;
    var same = allComps().filter(function (x) {
      return x !== fr && x.id && vclOf(x).cls === cls && (!par || contOf(x) === par) && !(x.closest && x.closest('[data-htd-hide]'));
    });
    var prim = selEl;
    multi = same.filter(function (x) { return x !== prim; });
    selEl = null;
    select(prim, 'multi', false, true);
    post({ type: 'note', text: '選取了 ' + same.length + ' 個 ' + cls + (par ? '（同一層）' : '（整頁）') });
    return same.length;
  }
  function selectAllSiblings() {
    var fr = formRoot();
    var par = !selEl || selEl === fr ? fr : contOf(selEl);
    /* (not the ones hidden in the designer -- the eye -- nor the locked ones, as the rubber band: AI 20261001, one locked
       member stopped the arrows for all of them, the hidden ones were moved / deleted unseen) */
    var sibs = allComps().filter(function (x) { return x !== fr && contOf(x) === par && !(x.closest && x.closest('[data-htd-hide]')) && !lockedOf(x); });
    if (!sibs.length) return 0;
    var prim = selEl && sibs.indexOf(selEl) >= 0 ? selEl : sibs[0];
    multi = sibs.filter(function (x) { return x !== prim; });
    selEl = null;
    select(prim, 'multi', false, true);
    return sibs.length;
  }
  var nudge = null, nudgeTimer = 0;
  function flushNudge() {
    clearTimeout(nudgeTimer);
    var n = nudge;
    nudge = null;
    if (!n || !n.items.some(function (it) { return it.last; })) return;
    if (n.items.length > 1) commitGroup(n.items);
    else commitLayout(n.items[0].el, n.items[0].bx, n.items[0].last);
    // refresh the panel -- but never take the selection back if it has moved on
    if (selEl === n.el) { selEl = null; select(n.el, 'edit', false); }
  }
  var ARROWS = { ArrowLeft: [-1, 0], ArrowRight: [1, 0], ArrowUp: [0, -1], ArrowDown: [0, 1] };
  /* WPF Cut / Copy / Paste / Delete of whole components: the extension does them on
     the source. Ctrl+C / X / V arrive as clipboard events (VS Code runs its clipboard
     commands inside the webview); one of a kind per 300 ms, as the browser's own
     default action may fire it too -- a doubled paste would make two copies. */
  var lastCmd = { cmd: '', t: 0 };
  function postCmd(cmd) {
    var now = Date.now();
    if (lastCmd.cmd === cmd && now - lastCmd.t < 300) return;
    lastCmd = { cmd: cmd, t: now };
    post({ type: 'cmd', cmd: cmd });
  }
  ['copy', 'cut', 'paste'].forEach(function (evn) {
    origAdd.call(document, evn, function (e) {
      if (mode !== 'design' || TXT) return;   // (in F2's text box: the text's own copy / paste)
      if (evn !== 'paste' && !selEl) return;
      e.preventDefault();
      postCmd(evn);
    }, true);
  });
  origAdd.call(window, 'keyup', function (e) {
    if (TXT) { if ((e.composedPath ? e.composedPath()[0] : null) === TXT.inp) e.stopImmediatePropagation(); return; }
    if ((e.key === ' ' || e.code === 'Space') && ZSP) {
      ZSP = null;
      panShield(PAN.space);
      e.preventDefault();
      e.stopImmediatePropagation();
      if (!PAN.space) return;
    }
    if ((e.key === ' ' || e.code === 'Space') && PAN.space) {
      PAN.space = false;
      if (!PAN.drag) panShield(false);
      e.preventDefault();
      e.stopImmediatePropagation();
    }
  }, true);
  origAdd.call(window, 'blur', function () { PAN.space = false; ZSP = null; if (PAN.drag) endPan(); else panShield(false); }, true);
  origAdd.call(window, 'keydown', function (e) {
    if (TXT) {
      /* F2's text box has the keyboard: its Enter / Esc here (this runs before the page's own key handlers -- a page
         may take Esc for itself), and its keys go no further (neither the page nor VS Code's shortcuts see them) */
      if ((e.composedPath ? e.composedPath()[0] : null) === TXT.inp) {
        if (e.key === 'Enter') { e.preventDefault(); endTextEdit(true); }
        else if (e.key === 'Escape') { e.preventDefault(); endTextEdit(false); }
        e.stopImmediatePropagation();
      }
      return;
    }
    if (TABSET && e.key === 'Escape') { e.preventDefault(); e.stopImmediatePropagation(); endTabSet(true); return; }
    if (PLACE && e.key === 'Escape') { e.preventDefault(); e.stopImmediatePropagation(); cancelPlace(); return; }
    if (e.key === 'Escape' && (drag || marquee)) { e.preventDefault(); e.stopImmediatePropagation(); cancelDrag(); return; }
    if ((e.key === ' ' || e.code === 'Space') && mode === 'design' && e.ctrlKey && !e.metaKey &&
      !(e.target && (/^(INPUT|TEXTAREA|SELECT)$/.test(e.target.tagName) || e.target.isContentEditable))) {
      /* Blend: Ctrl+Space held = the zoom-in pointer (a click zooms in there), Ctrl+Alt+Space = zoom out */
      e.preventDefault();
      e.stopImmediatePropagation();
      ZSP = { out: !!e.altKey };
      panShield(true);
      return;
    }
    if ((e.key === ' ' || e.code === 'Space') && mode === 'design' && !e.ctrlKey && !e.altKey && !e.metaKey &&
      !(e.target && (/^(INPUT|TEXTAREA|SELECT)$/.test(e.target.tagName) || e.target.isContentEditable))) {
      /* Space held = the hand (pan); the page does not see it, the view does not page down */
      e.preventDefault();
      e.stopImmediatePropagation();
      if (!PAN.space) { PAN.space = true; panShield(true); }
      return;
    }
    if (mode === 'design' && e.ctrlKey && e.shiftKey && !e.altKey && !e.metaKey && (e.key === 'a' || e.key === 'A')) {
      /* WPF: Ctrl+Shift+A = clear all selections */
      e.preventDefault();
      flushNudge();
      select(null, 'key', false);
      return;
    }
    if (mode === 'design' && e.ctrlKey && !e.altKey && !e.shiftKey && !e.metaKey && (e.key === 'a' || e.key === 'A')) {
      e.preventDefault();
      flushNudge();
      selectAllSiblings();
      return;
    }
    if (mode === 'design' && e.key === 'F9' && !e.ctrlKey && !e.altKey && !e.shiftKey && !e.metaKey) {
      /* WPF: F9 = show / hide the element handles */
      e.preventDefault();
      ADORN = !ADORN;
      scheduleDraw();
      post({ type: 'note', text: ADORN ? '控制點：顯示' : '控制點：隱藏（F9 再顯示；點照樣會選取）' });
      return;
    }
    if (mode !== 'design' || !selEl || e.altKey || e.metaKey) return;
    var t = e.target;
    if (t && /^(INPUT|TEXTAREA|SELECT)$/.test(t.tagName)) return;
    var k = e.key;
    if (k === 'Escape' && !e.ctrlKey) {
      e.preventDefault();
      flushNudge();
      var p = compParent(selEl);
      if (p) { selEl = null; select(p, 'key', true); } else select(null, 'key', false);
      return;
    }
    if (k === 'Tab' && !e.ctrlKey) {
      e.preventDefault();
      e.stopImmediatePropagation();
      flushNudge();
      /* (the ones hidden in the designer are skipped: they cannot be seen) */
      var list = allComps().filter(function (x) { return x === selEl || !(x.closest && x.closest('[data-htd-hide]')); });
      var i = list.indexOf(selEl);
      var nx = list[(i + (e.shiftKey ? -1 : 1) + list.length) % list.length];
      if (nx) { selEl = null; select(nx, 'key', true); }
      return;
    }
    if (k === 'Enter' && !e.ctrlKey && !e.shiftKey) {
      e.preventDefault();
      flushNudge();
      /* Enter = View Code: opens code, never adds a handler (only a real double-click does, like WPF) */
      post({ type: 'dblclick', key: keyOf(selEl), viewOnly: true });
      return;
    }
    if (k === 'Delete' && !e.ctrlKey && !e.shiftKey) {
      /* WPF Delete: the extension removes it from the source, the page is drawn again */
      e.preventDefault();
      flushNudge();
      postCmd('delete');
      return;
    }
    if (!ARROWS[k]) return;
    e.preventDefault();
    e.stopImmediatePropagation();
    var lkK = '';
    allSel().forEach(function (x) { if (!lkK) lkK = lockedOf(x); });
    if (lkK) { post({ type: 'editRefused', why: lockedWhy(lkK) }); return; }
    var group = allSel().map(function (x) { return { el: x, bx: boxOf(x) }; })
      .filter(function (g) { return g.bx && (e.ctrlKey || !g.bx.root); });   // the form resizes, never moves
    if (!group.length) { post({ type: 'editRefused', why: '這個元件不是用 position:absolute 定位的，不能用方向鍵移動' }); return; }
    var step = e.shiftKey ? 10 : 1;
    if (!nudge || nudge.el !== selEl || nudge.items.length !== group.length) {
      flushNudge();
      nudge = { el: selEl, dL: 0, dT: 0, dW: 0, dH: 0,
        items: group.map(function (g) { return { el: g.el, bx: g.bx, s0: snap(g.bx), last: null }; }) };
    }
    if (e.ctrlKey) { nudge.dW += ARROWS[k][0] * step; nudge.dH += ARROWS[k][1] * step; }
    else { nudge.dL += ARROWS[k][0] * step; nudge.dT += ARROWS[k][1] * step; }
    var refused = false;
    for (var ni = 0; ni < nudge.items.length; ni++) {
      var it = nudge.items[ni];
      var ch = deltaStyle(it.s0, nudge.dL, nudge.dT, nudge.dW, nudge.dH);
      if (!ch) { refused = true; continue; }
      applyStyle(it.bx.node, ch);
      it.last = ch;
    }
    if (refused) post({ type: 'editRefused', why: '有元件的位置／大小不是寫在自己 style 裡的 px 數值，那個沒有移動' });
    scheduleDraw();
    clearTimeout(nudgeTimer);
    nudgeTimer = setTimeout(flushNudge, 400);   // one source edit (one undo step) per burst of keys
  }, true);
  setInterval(function () { if (selEl) scheduleDraw(); }, 400);

  /* 9. what the page tried and could not do / what broke --------------------- */
  var seenBlock = {};
  origAdd.call(document, 'securitypolicyviolation', function (e) {
    var dir = e.effectiveDirective || e.violatedDirective || '';
    var uri = String(e.blockedURI || '');
    if (uri.indexOf('htd-netcheck') >= 0) return;   /* our own self-check, not the page */
    var k = dir + ' ' + uri;
    if (seenBlock[k]) return;
    seenBlock[k] = 1;
    post({ type: 'blocked', dir: dir, uri: uri });
  });
  var errCount = 0;
  origAdd.call(window, 'error', function (e) {
    if (!(e instanceof ErrorEvent)) return;
    if (errCount++ >= 20) return;
    post({ type: 'pageError', msg: String(e.message || ''), src: String(e.filename || ''), line: e.lineno || 0 });
    (ibErrs = ibErrs || []).push({ msg: String(e.message || ''), src: String(e.filename || ''), line: e.lineno || 0 });
    drawInfoBar();
  }, true);

  /* hidden in the designer only (WPF: the eye in the document outline) -- a
     visibility rule of our own, never written to the source; a click goes through
     to what is underneath (pages stack many panels at one place) */
  var hideStyle = null;
  function setDesignHidden(ids) {
    tabCache = null;   // (a hidden one is no Tab stop)
    if (!hideStyle) {
      hideStyle = document.createElement('style');
      hideStyle.setAttribute('data-htd', 'hide');
      hideStyle.textContent = '[data-htd-hide],[data-htd-hide] *{visibility:hidden!important;}';
      (document.head || document.documentElement).appendChild(hideStyle);
    }
    var want = {};
    (Array.isArray(ids) ? ids : []).forEach(function (id) { want[String(id)] = 1; });
    var now = document.querySelectorAll('[data-htd-hide]');
    for (var i = 0; i < now.length; i++) if (!want[idOf(now[i])]) now[i].removeAttribute('data-htd-hide');
    for (var id in want) {
      var e = byId(id);
      if (e && e !== document.body) e.setAttribute('data-htd-hide', '1');
    }
    hoverEl = null;
    scheduleDraw();
  }

  /* 10. component tree -------------------------------------------------------- */
  function buildTree() {
    var out = [];
    var fr = formRoot();
    /* [key, parent key, id, tag, class, caption, hidden now, is the form, hidden when the page runs (shown only while designing)] */
    if (fr) out.push([keyOf(fr), 0, '@form', fr.tagName.toLowerCase(), '', '', hidden(fr) ? 1 : 0, 1, 0]);
    /* (the tab sheets too -- in document order, so each comes before its controls) */
    var all = document.body ? document.body.querySelectorAll('[id], .pcPane') : [];
    for (var i = 0; i < all.length; i++) {
      var el = all[i];
      var tn = el.tagName;
      if (tn === 'SCRIPT' || tn === 'STYLE' || tn === 'TEMPLATE' || tn === 'LINK' || tn === 'META') continue;
      var pane = !el.id && isPane(el);
      if (pane && !paneName(el)) continue;
      var p = el.parentElement, pk = 0;
      while (p && p !== document.body) {
        if (p.id || p === fr || (isPane(p) && paneName(p))) { pk = keyOf(p); break; }
        p = p.parentElement;
      }
      if (pane) {
        /* a sheet not showing is not "hidden" (BCB6 / WPF: only its tab is not the active one) */
        var tab = paneTab(el);
        out.push([keyOf(el), pk, paneName(el), 'div', 'TTabSheet', tab ? clip(tab.textContent, 40) : '', 0, 0, 0]);
        continue;
      }
      out.push([keyOf(el), pk, el.id, tn.toLowerCase(), vclOf(el).cls, caption(el), treeHidden(el) ? 1 : 0, 0, runHidden(el) ? 1 : 0]);
    }
    return out;
  }
  var lastSig = '';
  function sendTree(force) {
    var nodes;
    try { nodes = buildTree(); } catch (e) { return; }
    var sig = nodes.length + '|' + nodes.map(function (n) { return n[2] + (n[6] ? '-' : '+') + (n[8] ? 'r' : ''); }).join(',');
    if (!force && sig === lastSig) return;
    lastSig = sig;
    post({ type: 'tree', nodes: nodes });
  }
  var treeTimer = 0;
  function scheduleTree() {
    if (treeTimer) return;
    treeTimer = setTimeout(function () { treeTimer = 0; sendTree(false); }, 1000);
  }

  /* 11. messages from the extension ------------------------------------------ */
  function byId(id) {
    if (id === '@form') return formRoot();
    if (!id) return null;
    var el = document.getElementById(id);
    if (el) return el;
    /* a tab sheet: by its .dfm name (it has no id on the page) */
    var ps = document.querySelectorAll('.pcPane');
    for (var i = 0; i < ps.length; i++) if (paneName(ps[i]) === id) return ps[i];
    return null;
  }
  origAdd.call(window, 'message', function (e) { onHostMessage(e.data); });
  /* 10b. 設計時全部顯示 (the BCB6 / WinForms form designer; EastSun: "編輯的時候我需要全部都顯示不要隱藏，
          當軟體啟動時才判定隱藏還是顯示"). In DESIGN mode every component and every tab shows, whatever
          hides it when the page runs: its own Visible=False (display:none in its style), a page rule (a
          machine's "-only" class, an option rule), the page's JS. A tab sheet's body still follows its tab,
          and what the user hid in the designer (the eye) stays hidden. 操作模式 shows the page as it runs.
          How: the page's own display:none rules (the ones it can reach: <style> in the page) are switched
          off while designing, and each component still hidden gets data-htd-show="<its display>" -- the
          page's source is never touched (the style attribute is read, never kept changed). */
  var SHOWALL = { on: false, rules: [], vals: {}, css: null, obs: null, t: 0, busy: false };
  function showAllTargets() {
    var fr = formRoot();
    var out = [];
    if (!fr) return out;
    var all = fr.querySelectorAll('[id], .pcTabs > .tab');
    for (var i = 0; i < all.length; i++) {
      var e = all[i];
      if (/^(SCRIPT|STYLE|TEMPLATE|LINK|META)$/.test(e.tagName)) continue;
      if (e.classList.contains('pcPane')) continue;   /* a tab sheet's body: its tab decides */
      out.push(e);
    }
    return out;
  }
  function showAllRules(on) {
    if (!on) {
      SHOWALL.rules.forEach(function (r) { try { r.rule.style.setProperty(r.prop, r.value, r.prio); } catch (e) { /* ignore */ } });
      SHOWALL.rules = [];
      return;
    }
    var fr = formRoot();
    var inForm = function (x) { return !!fr && fr.contains(x) && !(host && (x === host || host.contains(x))); };
    var walk = function (list) {
      for (var i = 0; i < list.length; i++) {
        var r = list[i];
        if (r.cssRules && !r.selectorText) { try { walk(r.cssRules); } catch (e) { /* ignore */ } continue; }
        if (!r.style || !r.selectorText) continue;
        ['display', 'visibility'].forEach(function (prop) {
          var v = r.style.getPropertyValue(prop);
          if (!(prop === 'display' ? v === 'none' : v === 'hidden')) return;
          var hits;
          try { hits = document.querySelectorAll(r.selectorText); } catch (e) { return; }
          /* only a rule for components: one that also hides a popup / a menu is left alone */
          if (!hits.length) return;
          for (var k = 0; k < hits.length; k++) if (!inForm(hits[k])) return;
          SHOWALL.rules.push({ rule: r, prop: prop, value: v, prio: r.style.getPropertyPriority(prop) });
          r.style.removeProperty(prop);
        });
      }
    };
    var sheets = document.styleSheets;
    for (var s = 0; s < sheets.length; s++) {
      var sh0 = sheets[s];
      if (sh0.ownerNode === SHOWALL.css || (hideStyle && sh0.ownerNode === hideStyle)) continue;
      var rl;
      try { rl = sh0.cssRules; } catch (e) { continue; }   /* another origin's sheet: cannot be read */
      if (rl) walk(rl);
    }
  }
  function showAllCss(val) {
    if (!SHOWALL.css) {
      SHOWALL.css = document.createElement('style');
      SHOWALL.css.id = '__htd_showall';
      (document.head || document.documentElement).appendChild(SHOWALL.css);
    }
    if (val && !SHOWALL.vals[val]) {
      SHOWALL.vals[val] = 1;
      SHOWALL.css.textContent += ':root [data-htd-show="' + val + '"]{display:' + val + '!important;}';
    }
    if (!SHOWALL.css.textContent || SHOWALL.css.textContent.indexOf('data-htd-vis') < 0)
      SHOWALL.css.textContent += ':root [data-htd-vis]:not([data-htd-hide]):not([data-htd-hide] *){visibility:visible!important;}';
  }
  function showAllPass() {
    if (SHOWALL.busy) return;
    SHOWALL.busy = true;
    try {
      var marked = document.querySelectorAll('[data-htd-show],[data-htd-vis]');
      for (var i = 0; i < marked.length; i++) { marked[i].removeAttribute('data-htd-show'); marked[i].removeAttribute('data-htd-vis'); }
      showAllCss(null);
      showAllRules(true);
      var t = showAllTargets(), hid = [];
      for (var j = 0; j < t.length; j++) {
        var cs = getComputedStyle(t[j]);
        if (cs.display === 'none' || cs.visibility === 'hidden') hid.push({ el: t[j], d: cs.display === 'none', v: cs.visibility === 'hidden' });
      }
      for (var k = 0; k < hid.length; k++) {
        var h = hid[k], el = h.el;
        if (el.closest('[data-htd-hide]')) continue;   /* hidden in the designer by the user */
        if (h.d) {
          /* its own display: what it has without the display:none (read, and the style put back as it was) */
          var nat = 'block';
          if (el.style.display === 'none') {
            var old = el.getAttribute('style');
            el.style.removeProperty('display');
            var nd = getComputedStyle(el).display;
            if (old == null) el.removeAttribute('style'); else el.setAttribute('style', old);
            if (nd && nd !== 'none') nat = nd;
          }
          showAllCss(nat);
          el.setAttribute('data-htd-show', nat);
        }
        if (h.v && el.parentElement && getComputedStyle(el.parentElement).visibility !== 'hidden') el.setAttribute('data-htd-vis', '1');
      }
    } catch (e) { /* ignore */ }
    SHOWALL.busy = false;
    if (SHOWALL.obs) SHOWALL.obs.takeRecords();   /* (our own style reads-and-restores are no change) */
  }
  function setShowAll(on) {
    SHOWALL.on = !!on;
    clearTimeout(SHOWALL.t);
    if (SHOWALL.on) {
      showAllPass();
      if (!SHOWALL.obs) {
        try {
          /* the page's JS may hide something later (a gate, an option rule): shown again */
          SHOWALL.obs = new MutationObserver(function (list) {
            if (!SHOWALL.on || SHOWALL.busy) return;
            var fr = formRoot();
            for (var i = 0; i < list.length; i++) {
              var r = list[i];
              if (r.target === host || (host && host.contains(r.target))) continue;
              /* (a status line outside the form ticking every 200 ms is no reason; a <style> the page adds is) */
              if (fr && !fr.contains(r.target) && r.target !== document.documentElement && r.target !== document.head && !(r.target.closest && r.target.closest('head'))) continue;
              clearTimeout(SHOWALL.t);
              SHOWALL.t = setTimeout(function () { if (SHOWALL.on) { showAllPass(); scheduleTree(); scheduleDraw(); } }, 120);
              return;
            }
          });
          SHOWALL.obs.observe(document.documentElement, { childList: true, subtree: true, attributes: true, attributeFilter: ['style', 'class'] });
        } catch (e) { SHOWALL.obs = null; }
      }
    } else {
      var marked = document.querySelectorAll('[data-htd-show],[data-htd-vis]');
      for (var i = 0; i < marked.length; i++) { marked[i].removeAttribute('data-htd-show'); marked[i].removeAttribute('data-htd-vis'); }
      showAllRules(false);
    }
    tabCache = null;
  }
  /* hidden when the page runs: shown only because we are designing */
  function runHidden(el) { return !!(el && el.hasAttribute && (el.hasAttribute('data-htd-show') || el.hasAttribute('data-htd-vis'))); }

  function onHostMessage(m) {
    if (!m || m.__htd !== 1) return;
    switch (m.type) {
      case 'init':
        if (m.classes && typeof m.classes === 'object') { CLS = m.classes; lastSig = ''; scheduleTree(); }
        if (m.events && typeof m.events === 'object') EVS = m.events;
        if (typeof m.zoom === 'number' && m.zoom !== 1) setZoom(m.zoom);
        if (m.mode) { mode = m.mode; drawBadge(); }
        if (SHOWALL.on !== (mode === 'design')) { setShowAll(mode === 'design'); scheduleTree(); }
        if (Array.isArray(m.designHidden) && m.designHidden.length) setDesignHidden(m.designHidden);
        if (Array.isArray(m.designLocked)) { LOCK = {}; m.designLocked.forEach(function (id) { LOCK[String(id)] = 1; }); }
        if (m.grid && typeof m.grid === 'object') GRID = gridOf(m.grid);
        if (m.screen !== undefined) SCREEN = screenOf(m.screen);
        if (typeof m.snapLines === 'boolean') SNAPL = m.snapLines;
        if (typeof m.artboardDark === 'boolean') { ABGWANT = m.artboardDark; setArtboard(ABGWANT && mode === 'design'); }
        if (typeof m.showBounds === 'boolean') { BOUNDSWANT = m.showBounds; setBounds(BOUNDSWANT && mode === 'design'); }
        if (m.wireMarks && typeof m.wireMarks === 'object') { WIRE = m.wireMarks; drawBadge(); }
        if (Array.isArray(m.dfmGhosts) && m.dfmGhosts.length) { GHOST = m.dfmGhosts; drawBadge(); }
        if (Array.isArray(m.tabOrder)) { TABO = m.tabOrder.filter(function (x) { return typeof x === 'string'; }); drawBadge(); }
        if (m.showNames) NAMES = true;
        if (m.wheelZoom) { WHEELZ = m.wheelZoom === 'wheel' || m.wheelZoom === 'alt' ? m.wheelZoom : 'ctrl'; drawBadge(); }
        if (typeof m.snapSpacing === 'number') GAP = Math.max(0, Math.min(64, Math.round(m.snapSpacing)));
        if (m.scroll) { try { window.scrollTo(m.scroll.x || 0, m.scroll.y || 0); } catch (x) { /* ignore */ } }
        var el0 = m.selectId ? byId(m.selectId) : null;
        if (el0) select(el0, 'init', !m.scroll);
        /* WPF: nothing picked yet = the root (the Window) is what the Properties window shows. The one asked for not
           on this page (yet -- a rename redraws the page for its script edits before the HTML's): the root is shown,
           'initRoot' = the extension keeps the one it asked for, the next drawing selects it */
        else if (mode === 'design' && !selEl && formRoot()) select(formRoot(), m.selectId ? 'initRoot' : 'init', false);
        /* WPF's "Default zoom setting: Fit All" -- a page just opened fits the view (after it is laid out) */
        if (m.fitOnOpen) setTimeout(function () { zoomFit(); }, 0);
        break;
      case 'setMode':
        if (TXT) endTextEdit(true);
        mode = m.mode === 'operate' ? 'operate' : 'design';
        setArtboard(ABGWANT && mode === 'design');   // (operating: the page as it is)
        setBounds(BOUNDSWANT && mode === 'design');
        setShowAll(mode === 'design');
        scheduleTree();
        PAN.space = false; PAN.drag = null; panShield(false);
        PLACE = null; placeShield(false);
        hoverEl = null; drawBadge(); scheduleDraw();
        break;
      case 'selectKey': {
        var el1 = els.get(m.key);
        if (el1 && el1.isConnected) { selEl = null; select(el1, m.origin || 'tree', true); }
        break;
      }
      case 'setZoom':
        setZoom(typeof m.zoom === 'number' ? m.zoom : 1);
        break;
      case 'selectAll':
        flushNudge();
        selectAllSiblings();
        break;
      case 'selectSameType':
        flushNudge();
        selectSameType(m.scope === 'container' ? 'container' : 'page');
        break;
      case 'zoomFit':
        zoomFit();
        break;
      case 'zoomSel':
        if (selEl) zoomToSel(m.seq); else { zoomFit(); if (m.seq) post({ type: 'zoomSel', seq: m.seq, zoom: zoom, none: true }); }
        break;
      case 'zoomStep':
        /* Blend: Ctrl+= / Ctrl+- (the toolbar's + / -) */
        toolbarAct(m.dir > 0 ? 'zoomIn' : 'zoomOut');
        break;
      case 'editText':
        /* F2 (WPF: Edit control text) */
        startTextEdit();
        break;
      case 'textEditEnd':
        endTextEdit(m.commit === true);
        break;
      case 'textUndo':
        /* Ctrl+Z / Ctrl+Y in F2's text box: the typing, not the page (VS Code sends those keys to its own undo) */
        if (TXT) { try { TXT.inp.focus(); document.execCommand(m.redo ? 'redo' : 'undo'); } catch (x) { /* ignore */ } }
        break;
      case 'setBounds':
        BOUNDSWANT = !!m.on;
        setBounds(BOUNDSWANT && mode === 'design');
        drawToolbar();
        break;
      case 'setArtboard':
        /* another page's toolbar toggled the artboard background: every page follows */
        ABGWANT = !!m.dark;
        setArtboard(ABGWANT && mode === 'design');
        drawToolbar();
        break;
      case 'placeArm':
        /* the toolbox picked a tool (cls null = put it back) */
        PLACE = m.cls && mode === 'design' ? { cls: String(m.cls), label: String(m.label || m.cls), drag: null } : null;
        placeShield(!!PLACE);
        if (!PLACE && mqEl) mqEl.style.display = 'none';
        drawBadge();
        break;
      case 'tabOrderSet':
        /* on: numbering by clicks from m.start (the badges shown: TABO is on too); off: done */
        if (m.on && mode === 'design') { TABSET = { next: Math.max(0, Math.round(+m.start || 1)), done: [] }; drawBadge(); scheduleDraw(); }
        else endTabSet(false);
        break;
      case 'tabOrder': {
        TABO = Array.isArray(m.dfm) ? m.dfm.filter(function (x) { return typeof x === 'string'; }) : null;
        tabCache = null;
        drawBadge();
        scheduleDraw();
        var tm = TABO ? tabModel() : null;
        post({ type: 'tabOrderInfo', seq: m.seq, on: !!TABO, stops: tm ? tm.stops : 0, common: tm ? tm.common : 0, bad: tm ? tm.bad : [] });
        break;
      }
      case 'dfmGhosts':
        GHOST = Array.isArray(m.items) && m.items.length ? m.items.filter(function (g) { return g && typeof g.id === 'string'; }) : null;
        drawBadge();
        scheduleDraw();
        break;
      case 'wireMarks':
        WIRE = m.marks && typeof m.marks === 'object' ? m.marks : null;
        drawBadge();
        scheduleDraw();
        break;
      case 'showNames':
        NAMES = !!m.on;
        scheduleDraw();
        break;
      case 'selectParent': {
        var pp = selEl ? compParent(selEl) : null;
        if (pp) { selEl = null; select(pp, 'key', true); }
        break;
      }
      case 'selectIds': {
        /* a multi-selection by ids: the first is the primary */
        var list6 = (m.ids || []).map(byId).filter(function (x) { return !!x; });
        if (!list6.length) break;
        multi = list6.slice(1);
        selEl = null;
        select(list6[0], m.origin === 'tree' ? 'tree' : 'multi', true, true);
        break;
      }
      case 'wheelZoom':
        /* the setting changed: which wheel zooms from now on */
        WHEELZ = m.how === 'wheel' || m.how === 'alt' ? m.how : 'ctrl';
        drawBadge();   // (the toolbar's and the hint's texts name the key)
        break;
      case 'snapSpacing':
        /* the setting changed: the spacing snaplines' distance from now on (0 = none) */
        GAP = Math.max(0, Math.min(64, Math.round(+m.px || 0)));
        break;
      case 'selectId': {
        var el2 = byId(m.id);
        if (el2) { selEl = null; select(el2, m.origin || 'editor', true); }
        break;
      }
      case 'refreshTree':
        sendTree(true);
        break;
      case 'designHidden':
        /* the whole set each time (ids), so it can never drift from the extension's */
        setDesignHidden(m.ids);
        break;
      case 'setGrid':
        GRID = gridOf(m);
        scheduleDraw();
        drawToolbar();
        break;
      case 'setScreen':
        SCREEN = screenOf(m.screen);
        scheduleDraw();
        break;
      case 'designLocked':
        LOCK = {};
        (Array.isArray(m.ids) ? m.ids : []).forEach(function (id) { LOCK[String(id)] = 1; });
        scheduleDraw();
        break;
      /* edits from the properties panel: change the DOM, then report the same change
         as an 'edit' for the extension to write into the source */
      case 'setLayout': {
        /* (by id: from the DFM difference list, for a control that is not selected) */
        var el3 = m.id ? byId(m.id) : els.get(m.key);
        if (!el3 || !el3.isConnected) break;
        if (m.all && !m.id && el3 === selEl && multi.length) { editAllSel(m); break; }
        var pl3 = layoutPlan(el3, m);
        if (typeof pl3 === 'string') { post({ type: 'editRefused', why: pl3 }); break; }
        applyStyle(pl3.bx.node, pl3.ch);
        commitLayout(el3, pl3.bx, pl3.ch);
        selEl = null; select(el3, 'edit', false);
        break;
      }
      case 'setCaption': {
        if (m.all && !m.id && els.get(m.key) === selEl && multi.length) { editAllSel(m); break; }
        var el4 = m.id ? byId(m.id) : els.get(m.key);
        if (!el4 || !el4.isConnected) break;
        var c4 = captionOf(el4);
        if (!c4) { post({ type: 'editRefused', why: '這個元件沒有可以直接改的文字' }); break; }
        var v4 = String(m.value == null ? '' : m.value);
        setCaptionDom(el4, c4.kind, v4);
        sendEdit(el4, { what: 'caption', capKind: c4.kind, value: v4 });
        selEl = null; select(el4, 'edit', false);
        break;
      }
      case 'setLook': {
        var el5 = m.id ? byId(m.id) : els.get(m.key);
        if (!el5 || !el5.isConnected) break;
        if (m.all && !m.id && el5 === selEl && multi.length) { editAllSel(m); break; }
        var e5 = lookChange(el5, m.prop, m.value, m.size);
        if (e5) sendEdit(el5, e5);
        selEl = null; select(el5, 'edit', false);
        break;
      }
      case 'editMany': {
        /* one look / caption change on many controls (the DFM difference list's "全部改回"):
           applied to each, reported as ONE batch edit = one source edit = one undo step */
        var batch = [], refusedM = [];
        (Array.isArray(m.items) ? m.items : []).forEach(function (it) {
          var e = byId(it.id);
          if (!e || !e.isConnected) return;
          var pl = null;
          if (it.type === 'setLayout') {
            /* (改回 DFM: a position / size; one that cannot is left, and said) */
            var lp = layoutPlan(e, it);
            if (typeof lp === 'string') { refusedM.push(lp); return; }
            applyStyle(lp.bx.node, lp.ch);
            if (Object.keys(lp.ch).length) pl = { what: 'style', target: lp.bx.target, style: lp.ch };
          } else pl = it.type === 'setCaption' ? captionChange(e, it.value) : it.type === 'setLook' ? lookChange(e, it.prop, it.value, it.size) : null;
          if (!pl) return;
          pl.id = idOf(e);
          pl.key = keyOf(e);
          batch.push(pl);
        });
        if (batch.length === 1) { var b1 = batch[0]; b1.type = 'edit'; post(b1); }
        else if (batch.length) post({ type: 'edit', what: 'batch', edits: batch });
        if (refusedM.length) post({ type: 'editRefused', why: refusedM[0] + (refusedM.length > 1 ? '（還有 ' + (refusedM.length - 1) + ' 個一樣）' : '') + '，那些的位置／大小沒有改' });
        /* the selection was among them: the panel shows the new values */
        if (selEl && batch.some(function (b) { return b.key === keyOf(selEl); })) { var pM = selEl; selEl = null; select(pM, 'edit', false); }
        scheduleDraw();
        break;
      }
      case 'setStyleRaw': {
        /* one declaration of the element's own style="…" from the grid (value null = remove) */
        var el8 = els.get(m.key);
        if (!el8 || !el8.isConnected || !/^(--)?[a-z][a-z0-9-]*$/.test(String(m.name || ''))) break;
        if (m.value != null && /[;{}<>\\]/.test(String(m.value))) break;
        var st8 = {};
        st8[m.name] = m.value == null ? null : String(m.value);
        if (st8[m.name] == null) el8.style.removeProperty(m.name); else el8.style.setProperty(m.name, st8[m.name]);
        sendEdit(el8, { what: 'style', target: 'self', style: st8 });
        selEl = null; select(el8, 'edit', false);
        break;
      }
      case 'setAttrRaw': {
        /* a text-like attribute from the grid (never the id or an on* handler) */
        var el9 = els.get(m.key);
        if (!el9 || !el9.isConnected || !/^(title|class|placeholder|tabindex|alt|data-[\w-]+|aria-[\w-]+)$/.test(String(m.name || ''))) break;
        var at9 = {};
        at9[m.name] = m.value == null ? null : String(m.value);
        if (at9[m.name] == null) el9.removeAttribute(m.name); else el9.setAttribute(m.name, at9[m.name]);
        sendEdit(el9, { what: 'attr', attr: at9 });
        selEl = null; select(el9, 'edit', false);
        break;
      }
      case 'align': {
        /* WPF Format > Align / Make Same Size: the primary selection is the reference */
        if (!selEl) break;
        flushNudge();
        if (ARRANGE[m.how]) { arrange(m.how); break; }
        var ref = selEl.getBoundingClientRect();
        var rest = allSel().slice(1);
        var items = [];
        var bad = false;
        var lockedA = '';
        for (var ai = 0; ai < rest.length; ai++) {
          var ae = rest[ai], abx = boxOf(ae);
          /* a locked one stays where it is (the primary is only the reference, it never moves) */
          if (lockedOf(ae)) { lockedA = lockedA || lockedOf(ae); continue; }
          if (!abx) { bad = true; continue; }
          var ar = ae.getBoundingClientRect();
          var aL = 0, aT = 0, aW = 0, aH = 0;
          switch (m.how) {   /* (rects are screen px; divided by the zoom below) */
            case 'left': aL = ref.left - ar.left; break;
            case 'right': aL = ref.right - ar.right; break;
            case 'top': aT = ref.top - ar.top; break;
            case 'bottom': aT = ref.bottom - ar.bottom; break;
            case 'hcenter': aL = (ref.left + ref.width / 2) - (ar.left + ar.width / 2); break;
            case 'vcenter': aT = (ref.top + ref.height / 2) - (ar.top + ar.height / 2); break;
            case 'width': aW = ref.width - ar.width; break;
            case 'height': aH = ref.height - ar.height; break;
            case 'size': aW = ref.width - ar.width; aH = ref.height - ar.height; break;
            default: break;
          }
          var s0a = snap(abx);
          var ach = deltaStyle(s0a, aL / zoom, aT / zoom, aW / zoom, aH / zoom);
          if (!ach) { bad = true; continue; }
          applyStyle(abx.node, ach);
          items.push({ el: ae, bx: abx, last: ach });
        }
        if (items.length) commitGroup(items);
        if (bad) post({ type: 'editRefused', why: '有元件的位置／大小不是寫在自己 style 裡的 px 數值，那個沒有對齊' });
        else if (lockedA) post({ type: 'editRefused', why: lockedWhy(lockedA) + '，它沒有跟著對齊' });
        var prim = selEl;
        selEl = null;
        select(prim, 'edit', false);
        break;
      }
      case 'listenersAll': {
        /* for the wiring overview: every control's own listener types (+ inline on*) */
        var map = {};
        var all = document.body ? document.body.querySelectorAll('[id]') : [];
        for (var li = 0; li < all.length; li++) {
          var le = all[li];
          var types = [];
          var rl = REC.get(le);
          if (rl) for (var lj = 0; lj < rl.length; lj++) if (types.indexOf(rl[lj].type) < 0) types.push(rl[lj].type);
          for (var la = 0; la < le.attributes.length; la++) {
            var an = le.attributes[la].name;
            if (/^on/i.test(an) && types.indexOf(an.slice(2).toLowerCase()) < 0) types.push(an.slice(2).toLowerCase());
          }
          if (types.length) map[le.id] = types;
        }
        post({ type: 'listenersAll', seq: m.seq, map: map });
        break;
      }
      case 'stackAt':
        /* 選取這裡的元件…: under the last right-click (or the point asked) */
        post({ type: 'stackAt', seq: m.seq, items: lastCtx || typeof m.x === 'number' ? stackAt(typeof m.x === 'number' ? m.x : lastCtx.x, typeof m.y === 'number' ? m.y : lastCtx.y) : [] });
        break;
      case 'lookAll': {
        /* for the DFM difference list: position / caption / look of the controls asked for */
        var want = Array.isArray(m.ids) ? new Set(m.ids) : null;
        var items7 = [];
        var all7 = document.body ? document.body.querySelectorAll('[id]') : [];
        for (var i7 = 0; i7 < all7.length; i7++) {
          var e7 = all7[i7];
          if (want && !want.has(e7.id)) continue;
          if (/^(SCRIPT|STYLE|TEMPLATE|LINK|META)$/.test(e7.tagName)) continue;
          var l7 = layoutOf(e7);
          var b7 = l7 ? boxOf(e7) : null;
          items7.push({
            id: e7.id,
            /* cl / ct: its border (children count from inside it) -- for "ungroup" */
            lay: l7 ? { left: l7.left, top: l7.top, width: l7.width, height: l7.height, rendered: l7.rendered, root: l7.root, target: l7.target,
              raw: l7.raw, ox: l7.ox, oy: l7.oy, cl: b7 ? b7.node.clientLeft : 0, ct: b7 ? b7.node.clientTop : 0 } : null,
            cap: captionOf(e7),
            look: lookOf(e7),
          });
        }
        post({ type: 'lookAll', seq: m.seq, items: items7 });
        break;
      }
      case 'netcheck': {
        /* prove the wall from inside the page: try ws / http / xhr to 127.0.0.1:9
           (a closed port, never the machine) and report what the CSP did */
        var got = [];
        var onV = function (ev) { got.push((ev.effectiveDirective || ev.violatedDirective) + ' ' + ev.blockedURI); };
        origAdd.call(document, 'securitypolicyviolation', onV);
        var res = { ws: '', fetch: '', xhr: '' };
        try { new WebSocket('ws://127.0.0.1:9/htd-netcheck'); res.ws = 'constructed'; } catch (x) { res.ws = 'threw ' + x.name; }
        try {
          fetch('http://127.0.0.1:9/htd-netcheck').then(function () { res.fetch = 'RESOLVED'; }, function (x) { res.fetch = 'rejected ' + x.name; });
        } catch (x) { res.fetch = 'threw ' + x.name; }
        try {
          var xr = new XMLHttpRequest();
          xr.open('GET', 'http://127.0.0.1:9/htd-netcheck');
          xr.onerror = function () { res.xhr = 'error'; };
          xr.onload = function () { res.xhr = 'LOADED'; };
          xr.send();
        } catch (x) { res.xhr = 'threw ' + x.name; }
        setTimeout(function () {
          origRemove.call(document, 'securitypolicyviolation', onV);
          post({ type: 'netcheck', violations: got, ws: res.ws, fetch: res.fetch, xhr: res.xhr });
        }, 800);
        break;
      }
      case 'diag': {
        /* did the page's own resources load through <base href>? (for the output log) */
        var imgs = document.images, loaded = 0, broken = 0;
        for (var ii = 0; ii < imgs.length; ii++) {
          if (imgs[ii].complete && imgs[ii].naturalWidth > 0) loaded++;
          else if (imgs[ii].complete) broken++;
        }
        var bs = null;
        try { bs = getComputedStyle(document.body); } catch (x) { bs = null; }
        post({
          type: 'diag', images: imgs.length, imagesLoaded: loaded, imagesBroken: broken,
          sheets: document.styleSheets.length, scripts: document.scripts.length,
          bodyBg: bs ? bs.backgroundColor : '', bodyPadding: bs ? bs.padding : '',
          vscodeDefaultsLeft: !!document.getElementById('_defaultStyles'),
          designHidden: document.querySelectorAll('[data-htd-hide]').length,
          designLocked: Object.keys(LOCK).length,
          grid: GRID.on ? GRID.size : 0,
          wireMarks: WIRE ? Object.keys(WIRE).length : 0,
          ghosts: GHOST && sh ? placeGhosts() : 0,
          tabBadges: TABO && sh ? placeTabOrder() : 0,
          placing: PLACE ? PLACE.cls : '',
          names: NAMES && sh ? placeNameTags() : 0,
          base: document.baseURI,
        });
        break;
      }
      default:
        break;
    }
  }

  /* 12. start --------------------------------------------------------------- */
  var lastScroll = 0;
  origAdd.call(window, 'scroll', function () {
    var now = Date.now();
    if (now - lastScroll < 400) return;
    lastScroll = now;
    post({ type: 'scroll', x: window.scrollX, y: window.scrollY });
  }, { passive: true });

  function start() {
    ensureOverlay();
    /* the context for VS Code's webview/context menu (our designer commands only) */
    try {
      document.documentElement.setAttribute('data-vscode-context', JSON.stringify({ webviewSection: 'htdDesign', preventDefaultContextMenuItems: true }));
    } catch (e) { /* ignore */ }
    if (mode === 'design') setShowAll(true);
    post({ type: 'ready', url: location.href, title: document.title, mode: mode });
    setTimeout(function () { sendTree(true); }, 60);
    try {
      new MutationObserver(function (list) {
        for (var i = 0; i < list.length; i++) {
          var r = list[i];
          if (r.target === host || (host && host.contains(r.target))) continue;
          scheduleTree();
          return;
        }
      }).observe(document.documentElement, { childList: true, subtree: true, attributes: true, attributeFilter: ['id', 'style', 'class'] });
    } catch (e) { /* ignore */ }
  }
  if (document.readyState === 'loading') origAdd.call(document, 'DOMContentLoaded', start);
  else start();
  origAdd.call(window, 'load', function () { if (SHOWALL.on) showAllPass(); sendTree(false); scheduleDraw(); });

  window.__htdProbe = {
    version: 1,
    mode: function () { return mode; },
    selectById: function (id) { var el = byId(id); if (el) { selEl = null; select(el, 'test', true); } return !!el; },
    drawNow: function () { draw(); },   /* tests: headless virtual time may not run animation frames */
    textBox: function () { return TXT ? TXT.inp : null; },   /* tests: F2's text box */
    hasSelection: function () { return !!selEl; },
    message: function (m) { m.__htd = 1; onHostMessage(m); },   /* tests: a host message, synchronously */
    placeTargetsAt: function (x, y) { return placeTargets(x, y); },   /* tests: where a tool put down at x, y would go (0.135: tab sheets by name) */
  };
})();
