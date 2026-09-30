'use strict';
// AI(W906-HTDESIGNER) 20260929: turn a page's HTML into what the designer webview shows.
//
// The page is shown as-is, with four things put right after <head>:
//   1. a Content-Security-Policy that allows ONLY the webview's own local files
//      (`src` = webview.cspSource). No ws:, no http(s) to anywhere, no frames, no
//      form posts, no workers. This is what makes it impossible for a preview to
//      reach wb_gateway / wb_serve (8045 / 8046) or anything else on the network.
//   2. <base href> = the page's own folder, so img/…, ../JSON/…, theme.css resolve
//      exactly as they do in the real browser.
//   3. a white default background (the webview otherwise shows the editor theme,
//      and pages that set no background would render black-on-dark).
//   4. the probe script (media/probe.js), before every page script.
// Plain Node (no vscode) so the headless-browser test builds the very same page.

function cspFor(src) {
  return [
    "default-src 'none'",
    'img-src ' + src + ' data: blob:',
    'media-src ' + src + ' data: blob:',
    'font-src ' + src + ' data:',
    'style-src ' + src + " 'unsafe-inline'",
    'script-src ' + src + " 'unsafe-inline' 'unsafe-eval'",
    'connect-src ' + src,
    "frame-src 'none'",
    "child-src 'none'",
    "worker-src 'none'",
    "object-src 'none'",
    "form-action 'none'",
  ].join('; ');
}

function attr(s) {
  return String(s).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;');
}

/** Data for the probe as a JSON <script> (not run; `<` escaped so no text can close the tag). */
function dataScript(id, obj) {
  return '<script type="application/json" id="' + attr(id) + '">' + JSON.stringify(obj).replace(/</g, '\\u003c') + '</script>';
}

/**
 * opt: { cspSource, baseHref, probeSrc, mode, preScript?, live? }
 * preScript: raw <script>…</script> put before the probe (tests use it to stub
 * acquireVsCodeApi); never used by the extension.
 * live: { machine, docs: { gerneral: {...}, config: {...} } } -- 機種與機台設定 (lib/liveconfig.js), read by the probe
 */
function buildPageHtml(html, opt) {
  const inject =
    '<meta http-equiv="Content-Security-Policy" content="' + attr(cspFor(opt.cspSource)) + '">' +
    '<base href="' + attr(opt.baseHref) + '">' +
    '<style id="__htd_base">html{background:#fff;color:#000;}</style>' +
    (opt.preScript || '') +
    (opt.live ? dataScript('__htd_live', opt.live) : '') +
    '<script src="' + attr(opt.probeSrc) + '" data-mode="' + attr(opt.mode || 'design') + '"></script>';
  const text = String(html || '');
  const head = /<head\b[^>]*>/i.exec(text);
  if (head) {
    const at = head.index + head[0].length;
    return text.slice(0, at) + inject + text.slice(at);
  }
  const root = /<html\b[^>]*>/i.exec(text);
  if (root) {
    const at = root.index + root[0].length;
    return text.slice(0, at) + '<head>' + inject + '</head>' + text.slice(at);
  }
  return '<!DOCTYPE html><html><head>' + inject + '</head><body>' + text + '</body></html>';
}

/** The page has <iframe>s: the preview will not load them (frame-src 'none'). */
function hasIframe(html) {
  return /<iframe\b/i.test(String(html || ''));
}

module.exports = { cspFor, buildPageHtml, hasIframe };
