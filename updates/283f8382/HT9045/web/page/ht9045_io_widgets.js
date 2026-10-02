/* ht9045_io_widgets.js -- the IO lamps / panel buttons of any page, bound by their Alias
 * ---------------------------------------------------------------------------
 * AI(W906-IOWIDGET) 20261001：新檔。EastSun 20261001「把舊版的功能架構仿造到現有架構」。
 *
 * golden 的架構：元件（TMyLedLane／TMyLed／TBtnPanelLane／TBtnPanel）本身只帶屬性（Alias、
 * In/Out 位址、Value、Down、顏色），真正接 IO 的是 IO 畫面提供的三個共用函式，別的畫面直接借用：
 *
 *     fiosetview->SetCompomentIO(form)    Alias -> IO 位址（iosetview.cpp:2085；uteach.cpp:1598 借用）
 *     fiosetview->SetCompomentHint(form)  提示「(Lane:x,IP:y,Port:z,Bit:b) Alias」（iosetview.cpp:2671）
 *     fiosetview->ScanLed(form)           計時器讀輸入、依 InType 換算、Value＝亮／滅（iosetview.cpp:2741；uteach.cpp:1372 借用）
 *     BtnPanelClick / BtnPanelLane1Click  Down 反相 -> IOBitOn/Off（iosetview.cpp:1030；uteach.cpp:3074）
 *
 * 網頁版照同一個分工：這個檔＝SetCompomentIO＋SetCompomentHint＋ScanLed（讀 wb_serve 的
 * /api/struct/io/config＋runtime，依 Alias 上色、每 pollIntervalMs 重讀）；按鈕送出沿用
 * ht9045_io_do.js（io.btnPanelClick -> JsonBridge/IoBtnPanelClick.cpp，golden 的 BtnPanelClick）。
 *
 * 用法（頁面自己決定要不要用 —— golden 也只有 IO 畫面和 teach 畫面用這套；GroundMan 的燈是警報記錄、
 * VacuumUnit 的按鈕是自己寫 1203，那些頁不要載）：
 *
 *     <script src="ht9045_recipe_client.js"></script>
 *     <script src="ht9045_io_widgets.js"></script>   <- 一定要在 ht9045_io_do.js 前面（它包 window.ioBindStatus）
 *     <script src="ht9045_io_do.js"></script>
 *
 * 規則（跟 HW.IoSetView.html 的 inline 版一致，那一版是同事的檔、不動它）：
 *   * 頁面已經有自己的 ioBindStatus（HW.IoSetView.html）就不重複讀、不重複上色、不加樣式 —— 只在它寫的提示前面加上 golden 的格式。
 *   * 資料只從 wb_serve 來（http 開的頁）；file:// 或讀不到就全部畫成 no-data，不拿舊 JSON 快照假裝。
 *   * null 不是 off：沒有卡／不是 1203 的點畫成 unknown（ChanIoPoints.cpp 的 quality=nosource）。
 *   * 顏色照 HW.IoSetView.html（EastSun 20260925 定的：亮＝藍燈綠框、沒觸發＝原本的燈加紅框；按鈕用自己的 TrueColor／FalseColor）。
 * ------------------------------------------------------------------------- */
(function () {
  'use strict';
  if (window.__ht9045IoWidgets) return;
  window.__ht9045IoWidgets = true;
  // HW.IoSetView.html：讀 IO、上色用它自己那一份（同事的檔，不動）；這裡只在上面加 golden 的提示（SetCompomentHint）
  var pageOwn = typeof window.ioBindStatus === 'function' ? window.ioBindStatus : null;

  var MOTION_NET = 0, PCI1203 = 3;
  var live = { cfg: null, timer: null, busy: false, fails: 0 };
  var noDataWhy = '';   /* set when wb_serve could not be read: every component says that, not "not in the IO table" */

  // ------------------------------------------------------------------ style（頁面沒有才加，值同 HW.IoSetView.html:52-60）
  (function () {
    if (pageOwn || document.getElementById('ioWidgetStyle')) return;   /* (the IO page has these rules itself) */
    var st = document.createElement('style');
    st.id = 'ioWidgetStyle';
    st.textContent =
      '.io-on{box-shadow:0 0 0 1px #228b22 inset,0 0 6px rgba(34,139,34,.45);}' +
      '.io-off{box-shadow:0 0 0 1px #b22222 inset,0 0 6px rgba(178,34,34,.35);}' +
      '.aled.io-on{background-color:#1e6fff !important;background-image:radial-gradient(ellipse at 50% 45%,#e8f4ff 0%,#8cc4ff 25%,#3a8cff 55%,#1e6fff 80%,#1552c8 100%) !important;border-color:#228b22 !important;box-shadow:0 0 0 2px #228b22 inset;}' +   /* (EastSun 20260925: the light inside the frame only, no outer glow) */
      '.aled.io-off{box-shadow:0 0 0 1px #b22222 inset,0 0 6px rgba(178,34,34,.35);}' +
      '.btnpanel.io-on{background-color:var(--bp-true,#228b22) !important;border-color:var(--bp-true,#228b22) !important;color:var(--bp-true-font,#fff) !important;box-shadow:0 0 0 1px var(--bp-true,#228b22) inset;}' +
      '.btnpanel.io-off{background-color:var(--bp-false,#b22222) !important;border-color:var(--bp-false,#b22222) !important;color:var(--bp-false-font,#fff) !important;box-shadow:0 0 0 1px var(--bp-false,#b22222) inset;}' +
      '.io-unknown{box-shadow:0 0 0 1px #666 inset;filter:saturate(.35);}' +
      '.io-stale{outline:1px dashed #ff8c00;outline-offset:1px;}' +
      '.io-nodata{outline:1px dashed #8b4513;outline-offset:1px;}';
    (document.head || document.documentElement).appendChild(st);
  })();

  // ------------------------------------------------------------------ helpers
  function aliasOf(el) {
    var t = el.getAttribute('data-io-title') || el.getAttribute('title') || el.getAttribute('data-htitle') || '';
    var m = t.match(/Alias=([^｜|\s]+)/);
    return m ? m[1].trim() : '';
  }

  function statusOf(pt, runtime) {
    var st = pt && pt.status ? pt.status : null;
    var state = 'unknown', stale = true;
    if (st) {
      if (st.isOn === true) state = 'on';
      else if (st.isOn === false) state = 'off';
      else if (st.state === 'on' || st.state === 'off') state = st.state;
      if (st.updatedAt) {
        var t = Date.parse(st.updatedAt);
        if (!isNaN(t)) {
          var interval = (runtime && runtime.pollIntervalMs) ? runtime.pollIntervalMs : 200;
          stale = (Date.now() - t) > Math.max(interval * 5, 2000);
        }
      }
      if (st.quality === 'good') stale = false;
      if (st.quality === 'stale' || st.quality === 'bad') stale = true;
    }
    return { state: state, stale: stale };
  }

  function hex3(n) { var s = (n >>> 0).toString(16).toUpperCase(); while (s.length < 3) s = ' ' + s; return s; }

  /* golden SetCompomentHint（iosetview.cpp:2671-2737）：MotionNet／1203 的點
     「(Lane:%d,IP:%d,Port:%d,Bit:%d) Alias」，其他卡「(%3X0%d) Alias」 */
  function hintOf(pt, alias) {
    var a = (pt && pt.address) || {};
    var isa = pt && pt.hw ? pt.hw.isaBase : null;
    if (a.port == null || a.bit == null) return alias;
    if (isa === MOTION_NET || isa === PCI1203)
      return '(Lane:' + a.lane + ',IP:' + a.ip + ',Port:' + a.port + ',Bit:' + a.bit + ') ' + alias;
    return '(' + hex3(a.port) + '0' + a.bit + ') ' + alias;
  }

  function mark(el, cls, on) { if (on) el.classList.add(cls); else el.classList.remove(cls); }

  /* AI(W906-IOWIDGET-3) 20261001: golden SetCompomentIO's colours for a bound panel button (iosetview.cpp:2244-2258
     TBtnPanelLane, :2322-2330 TBtnPanel) -- the .dfm's colours are only what it has before the form binds it:
       an output that needs the safe door closed while idle (bIdleNeedCheckSafeDoor -> hw.needSafeDoor, C++
       ChanIoPoints.cpp)  FalseColor 0x001865EA / TrueColor 0x0003DFFD, black text  = orange / yellow
       any other bound one                FalseColor 0x009D4701 / TrueColor 0x00DCB505, white text  = blue / light blue
     not bound / not enabled = grey (golden clSilver; ht9045_io_do.js greys it). Set on the element only, never the source. */
  var SAFE_DOOR = { '--bp-false': '#ea6518', '--bp-true': '#fddf03', '--bp-false-font': '#000000', '--bp-true-font': '#000000' };
  var BOUND = { '--bp-false': '#01479d', '--bp-true': '#05b5dc', '--bp-false-font': '#ffffff', '--bp-true-font': '#ffffff' };
  function colorButton(el, pt) {
    if (!el.classList || !el.classList.contains('btnpanel') || !pt || pt.direction !== 'output' || !pt.hw || pt.hw.enable !== 1) return;
    var c = pt.hw.needSafeDoor ? SAFE_DOOR : BOUND;
    for (var k in c) if (c.hasOwnProperty(k) && String(el.style.getPropertyValue(k)).trim().toLowerCase() !== c[k]) el.style.setProperty(k, c[k]);
    var tAttr = el.hasAttribute('title') ? 'title' : 'data-htitle';
    var t = el.getAttribute(tAttr) || '';
    if (pt.hw.needSafeDoor && t.indexOf('待機時要先關安全門') < 0) el.setAttribute(tAttr, t + '\n（待機時要先關安全門才能動：門開著按了會被擋下）');
  }

  /* one component: its IO state as a class, the golden hint in its tooltip (the original title kept) */
  function paint(el, pt, runtime) {
    mark(el, 'io-on', false); mark(el, 'io-off', false); mark(el, 'io-unknown', false);
    mark(el, 'io-stale', false); mark(el, 'io-nodata', false);
    var tAttr = el.hasAttribute('title') ? 'title' : 'data-htitle';   // (theme.js's release mode moves title to data-htitle)
    var base = el.getAttribute('data-io-title');
    if (!base) { base = el.getAttribute(tAttr) || ''; el.setAttribute('data-io-title', base); }
    var alias = aliasOf(el);
    if (!pt) {
      mark(el, 'io-nodata', true);
      el.setAttribute(tAttr, alias + '（' + (noDataWhy || 'IO 表沒有這個 Alias') + '）\n' + base);
      return;
    }
    var s = statusOf(pt, runtime);
    mark(el, s.state === 'on' ? 'io-on' : s.state === 'off' ? 'io-off' : 'io-unknown', true);
    if (s.stale || s.state === 'unknown') mark(el, 'io-stale', true);
    el.setAttribute(tAttr, hintOf(pt, alias) + '\n' + base + '｜state=' + s.state + (s.stale ? '/stale' : ''));
  }

  function byAliasOf(data) {
    var out = {}, pts = (data && data.points) || [], byId = {}, i;
    for (i = 0; i < pts.length; i++) byId[pts[i].id] = pts[i];
    var idx = data && data.index && data.index.byAlias;
    if (idx) for (var k in idx) if (idx.hasOwnProperty(k) && idx[k] && idx[k].length && byId[idx[k][0]]) out[k] = byId[idx[k][0]];
    // (no index: the first row of an Alias, as C++'s mapIOTable)
    if (!Object.keys(out).length) for (i = 0; i < pts.length; i++) if (pts[i].alias && !out[pts[i].alias]) out[pts[i].alias] = pts[i];
    return out;
  }

  /* the page binds and paints by itself (HW.IoSetView.html): golden's hint put in front of the tooltip it writes,
     every time it writes it (it rewrites from the original title each poll, so nothing piles up) */
  if (pageOwn) {
    window.ioBindStatus = function (data) {
      var r = pageOwn.apply(this, arguments);
      try {
        var map = byAliasOf(data || {});
        var nodes = document.querySelectorAll('[data-io-title*="Alias="]');
        for (var i = 0; i < nodes.length; i++) {
          var el = nodes[i], a = aliasOf(el), pt = a && map[a];
          if (!pt) continue;
          var tAttr = el.hasAttribute('title') ? 'title' : 'data-htitle';
          var h = hintOf(pt, a), cur = el.getAttribute(tAttr) || '';
          if (cur.indexOf(h) !== 0) el.setAttribute(tAttr, h + '\n' + cur);
          colorButton(el, pt);
        }
      } catch (e) { /* a hint that cannot be added never stops the page's own painting */ }
      return r;
    };
    window.HT9045IoWidgets = { bind: window.ioBindStatus, hintOf: hintOf, aliasOf: aliasOf, last: null, why: 'the page binds by itself; hints only' };
    return;
  }

  /* golden ScanLed + the Down part of SetCompomentIO: every Alias-bearing component of the page.
     Global name on purpose: ht9045_io_do.js wraps window.ioBindStatus to repaint the buttons' Down. */
  function ioBindStatus(data) {
    var runtime = (data && data.runtime) || {};
    var map = byAliasOf(data || {});
    var nodes = document.querySelectorAll('[title*="Alias="],[data-htitle*="Alias="],[data-io-title*="Alias="]');
    var hit = 0, miss = 0;
    for (var i = 0; i < nodes.length; i++) {
      var a = aliasOf(nodes[i]);
      if (!a) continue;                        // (Alias= empty: the page's own button, e.g. Alert.Note's)
      paint(nodes[i], map[a], runtime);
      colorButton(nodes[i], map[a]);
      if (map[a]) hit++; else miss++;
    }
    window.HT9045IoWidgets.last = { mapped: hit, noData: miss, connected: runtime.connected === true, provider: runtime.provider || '' };
  }
  window.ioBindStatus = ioBindStatus;   // (called as window.ioBindStatus below: ht9045_io_do.js wraps it after this file)

  // ------------------------------------------------------------------ data（wb_serve only）
  function getJson(url) {
    if (window.fetch) return fetch(url, { cache: 'no-store' }).then(function (r) { if (!r.ok) throw new Error('HTTP ' + r.status); return r.json(); });
    return new Promise(function (ok, bad) {
      var x = new XMLHttpRequest();
      x.open('GET', url, true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        if (x.status >= 200 && x.status < 300) { try { ok(JSON.parse(x.responseText)); } catch (e) { bad(e); } }
        else bad(new Error('HTTP ' + x.status));
      };
      x.onerror = function () { bad(new Error('network error')); };
      x.send();
    });
  }

  /* golden SetCompomentIO: the IO table's row per Alias (address, ISABase, InType, Enable), the card's state */
  function merge(cfg, rt) {
    var st = {}, rtp = (rt && rt.points) || [], cp = (cfg && cfg.points) || [], i;
    for (i = 0; i < rtp.length; i++) if (rtp[i] && rtp[i].ioId) st[rtp[i].ioId] = rtp[i];
    var pts = [];
    for (i = 0; i < cp.length; i++) {
      var c = cp[i], id = c.ioId || c.id, s = st[id];
      pts.push({ id: id, alias: c.alias, ioType: c.ioType, direction: c.direction, address: c.address, hw: c.hw,
        status: s ? { isOn: s.isOn, isOff: s.isOff, state: s.state, updatedAt: s.updatedAt, source: s.source, quality: s.quality }
                  : { isOn: null, isOff: null, state: 'unknown', updatedAt: null, source: 'merge-default', quality: 'stale' } });
    }
    return { runtime: (rt && rt.runtime) || { connected: false, pollIntervalMs: 200 }, points: pts, index: (cfg && cfg.index) || { byAlias: {} } };
  }

  function apiBase() { return (location.protocol === 'http:' || location.protocol === 'https:') ? '/api/struct/io/' : null; }

  /* wb_serve could not be read: every component says so (dashed no-data) -- on the machine, lamps that do not follow
     the card must not look like they do */
  function nothing(why) {
    window.HT9045IoWidgets.why = why;
    noDataWhy = 'IO 讀不到：' + why;
    window.ioBindStatus({ runtime: { connected: false }, points: [], index: { byAlias: {} } });
  }

  function start() {
    var api = apiBase();
    // (not opened from wb_serve -- a file, the HTML designer: the page stays as drawn, nothing marked)
    if (!api) { window.HT9045IoWidgets.why = '不是從 wb_serve 開的頁（' + location.protocol + '）'; return; }
    var q = '?_=' + Date.now();
    Promise.all([getJson(api + 'config' + q), getJson(api + 'runtime' + q)]).then(function (p) {
      if (!p[0] || !p[0].points || !p[1] || !p[1].points) throw new Error('bad shape');
      live.cfg = p[0];
      window.ioBindStatus(merge(p[0], p[1]));
      var ms = Math.max(200, (p[1].runtime && p[1].runtime.pollIntervalMs) || 500);
      live.timer = setInterval(function () {
        if (live.busy || document.hidden) return;   // (not stacked; paused while the page is hidden)
        live.busy = true;
        getJson(api + 'runtime?_=' + Date.now()).then(function (rt) {
          live.busy = false; live.fails = 0;
          window.ioBindStatus(merge(live.cfg, rt));
        }, function (e) {
          live.busy = false; live.fails++;
          window.HT9045IoWidgets.why = 'runtime 讀取失敗 x' + live.fails + '（' + (e && e.message) + '）';
        });
      }, ms);
    }).catch(function (e) { nothing('/api/struct/io 讀不到（' + ((e && e.message) || e) + '）'); });
  }

  window.HT9045IoWidgets = { bind: ioBindStatus, hintOf: hintOf, aliasOf: aliasOf, last: null, why: '' };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
})();
