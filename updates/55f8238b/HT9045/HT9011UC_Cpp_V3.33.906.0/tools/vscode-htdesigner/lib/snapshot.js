'use strict';
// AI(W906-HTDESIGNER) 20261006 (EastSun: 設計畫面匯出成圖片): a page as a PNG, the way the machine shows it (its own
// visibility, not the designer's show-all), drawn by Edge headless from a copy of the page's text (unsaved changes
// included) -- the copy in %TEMP% with the page's folder as <base> and a CSP that allows no network at all (the page's
// scripts may run, a WebSocket to the machine's gateway never opens). Plain Node; the extension runs Edge.
const fs = require('fs');
const path = require('path');
const { pathToFileURL } = require('url');

/** the form's size from <div class="form" style="width:…px;height:…px"> (the generator's), else the screen's */
function formSize(html) {
  const m = /<div\b[^>]*\bclass\s*=\s*["']form["'][^>]*>/i.exec(String(html || ''));
  const st = m ? (/\bstyle\s*=\s*"([^"]*)"/i.exec(m[0]) || [])[1] || '' : '';
  const px = k => { const x = new RegExp('(?:^|;)\\s*' + k + '\\s*:\\s*(\\d+)px', 'i').exec(st); return x ? +x[1] : null; };
  const w = px('width'), h = px('height');
  const out = { w: w && w > 40 ? Math.min(w, 4000) : 1280, h: h && h > 40 ? Math.min(h, 4000) : 1024, fromForm: !!(w && h) };
  // (1009 review (export S1): a form wider / taller than 4000 px was cut without a word -- only a measured page said so)
  if ((w || 0) > 4000 || (h || 0) > 4000) out.cut = 4000;
  return out;
}

/** the copy Edge draws: <base> = the page's folder, no network (CSP connect-src 'none'), the page as it is */
function snapshotHtml(html, pageDir, opt) {
  const base = pathToFileURL(pageDir.replace(/[\\/]?$/, path.sep)).href;
  const inject = '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; img-src file: data:; style-src file: \'unsafe-inline\'; ' +
    'script-src file: \'unsafe-inline\'; font-src file: data:; connect-src \'none\'">' +
    // (1009 review (export S3): & too -- a folder "x&copy_y" was read back as "x©_y")
    '<base href="' + base.replace(/&/g, '&amp;').replace(/"/g, '%22') + '">' +
    // (the wire scripts' status bar -- 讀取中 / 讀取失敗, only about the server: not part of the page's look)
    '<style>html,body{overflow:hidden!important;}#ht9045WireBar{display:none!important;}</style>' +
    // (the page's own fetch / XHR / WebSocket wait for ever instead of failing: a failed read would draw the page's red
    //  "讀取失敗 … HTTP 0" box into the picture; the CSP still lets nothing out)
    '<script>(function(){try{window.fetch=function(){return new Promise(function(){});};' +
    'var X=window.XMLHttpRequest;if(X){X.prototype.send=function(){};}' +
    'window.WebSocket=function(){var w={readyState:0,send:function(){},close:function(){},addEventListener:function(){},removeEventListener:function(){}};return w;};' +
    'window.WebSocket.CONNECTING=0;window.WebSocket.OPEN=1;window.WebSocket.CLOSING=2;window.WebSocket.CLOSED=3;' +
    'window.EventSource=function(){return{close:function(){},addEventListener:function(){}};};}catch(e){}})();</script>' +
    // (a page with no generated .form -- main.html, the MotionView pages: its drawn size measured once it has run, read
    //  back from the dumped DOM; see measureArgs / parseSize)
    (opt && opt.measure ? '<script>window.addEventListener("load",function(){setTimeout(function(){var d=document.documentElement,b=document.body||d;' +
      'var m=document.createElement("meta");m.name="htd-size";m.content=Math.max(d.scrollWidth,b.scrollWidth)+"x"+Math.max(d.scrollHeight,b.scrollHeight);' +
      'document.head.appendChild(m);},800);});</script>' : '');
  const text = String(html || '');
  // (1009 review (export S2): the first <head> outside comments -- "<!-- <head> -->" took the CSP, the base and the
  //  network stubs into the comment: the page drew with broken paths and its "讀取失敗" box)
  const noCm = text.replace(/<!--[\s\S]*?(?:-->|$)/g, m => ' '.repeat(m.length));
  const head = /<head\b[^>]*>/i.exec(noCm);
  if (head) return text.slice(0, head.index + head[0].length) + inject + text.slice(head.index + head[0].length);
  return '<!DOCTYPE html><html><head>' + inject + '</head><body>' + text + '</body></html>';
}

/** msedge.exe, or null */
function edgePath(env) {
  env = env || process.env;
  const c = [path.join(env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)', 'Microsoft', 'Edge', 'Application', 'msedge.exe'),
    path.join(env.ProgramFiles || 'C:\\Program Files', 'Microsoft', 'Edge', 'Application', 'msedge.exe')];
  return c.find(p => { try { return fs.statSync(p).isFile(); } catch (e) { return false; } }) || null;
}

/** Edge's arguments: headless, its own profile (a running Edge is not touched), the size of the form, a moment for the scripts */
/*
 * 1007 audit (export #1): no network at the BROWSER level too -- the CSP is in the copy only; a page that redirects
 * (Setup.Configuration -> Config.Configuration.html) loaded the real page, which tried ws://ht9045/ (this PC runs the
 * machine). Every connection goes to a dead proxy, loopback included (<-loopback>); no background requests.
 */
const NO_NET = ['--proxy-server=127.0.0.1:9', '--proxy-bypass-list=<-loopback>', '--disable-background-networking', '--disable-sync',
  '--disable-component-update', '--no-pings', '--disable-domain-reliability'];
/** 1007 audit (export #1): a redirect stub page (location.replace / assign / href to another .html) -> that file name, else null */
function redirectOf(html) {
  const t = String(html || '');
  // (a page that location.replace()s at load -- Setup.Configuration, 500 KB, goes to Config.Configuration.html; an href /
  //  assign only in a small stub: a big page's buttons navigate that way too)
  // (1009 review (export S4): only in the <head> -- where the three redirect pages have it, run at load; a button's
  //  onclick="location.replace('main.html')" anywhere in a big page made it draw main.html instead)
  const hEnd = t.search(/<\/head\s*>|<body\b/i);
  const rp = /location\s*\.\s*replace\s*\(\s*(['"])([^'"?#]+\.html?)\1/i.exec(hEnd >= 0 ? t.slice(0, hEnd) : t.slice(0, 4000));
  if (rp) return rp[2];
  if (t.length > 4000) return null;
  const m = /location\s*(?:\.\s*(?:replace|assign)\s*\(\s*|\.\s*href\s*=\s*)(['"])([^'"?#]+\.html?)/i.exec(t);
  return m ? m[2] : null;
}
function edgeArgs(o) {
  return ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--disable-extensions', '--hide-scrollbars'].concat(NO_NET, [
    '--force-device-scale-factor=1', '--user-data-dir=' + o.profile, '--allow-file-access-from-files', '--virtual-time-budget=3000',
    '--window-size=' + o.w + ',' + o.h, '--screenshot=' + o.out, pathToFileURL(o.file).href]);
}

/** the measuring pass: the DOM dumped after the page has run, in a 1280 x 1024 window (as the machine's screen) */
function measureArgs(o) {
  return ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--disable-extensions', '--hide-scrollbars'].concat(NO_NET, [
    '--force-device-scale-factor=1', '--user-data-dir=' + o.profile, '--allow-file-access-from-files', '--virtual-time-budget=3000',
    '--window-size=1280,1024', '--dump-dom', pathToFileURL(o.file).href]);
}
/** the size the measuring pass found (320..4000 each), or null */
function parseSize(dom) {
  const m = /<meta\b[^>]*name="htd-size"[^>]*content="(\d+)x(\d+)"/.exec(String(dom || '')) || /<meta\b[^>]*content="(\d+)x(\d+)"[^>]*name="htd-size"/.exec(String(dom || ''));
  if (!m) return null;
  const c = v => Math.max(320, Math.min(4000, +v));
  const out = { w: c(m[1]), h: c(m[2]) };
  if (+m[1] > 4000 || +m[2] > 4000) out.cut = 4000;   // (1007 audit, export #3: said, not cut silently)
  return out;
}

/** a default name: the page's name + the time (Setup.HotPlate_20261006_1830.png) */
function defaultName(pageFile, now) {
  const d = now || new Date();
  const p2 = n => ('0' + n).slice(-2);
  return path.basename(pageFile).replace(/\.html?$/i, '') + '_' + d.getFullYear() + p2(d.getMonth() + 1) + p2(d.getDate()) + '_' + p2(d.getHours()) + p2(d.getMinutes()) + '.png';
}

module.exports = { NO_NET, redirectOf, formSize, snapshotHtml, edgePath, edgeArgs, measureArgs, parseSize, defaultName };
