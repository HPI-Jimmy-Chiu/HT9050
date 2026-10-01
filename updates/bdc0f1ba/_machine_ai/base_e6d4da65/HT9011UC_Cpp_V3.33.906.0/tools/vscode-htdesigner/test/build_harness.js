'use strict';
// AI(W906-HTDESIGNER) 20260929: build a file:// copy of a page exactly as the designer
// webview would show it (lib/pagehtml.js), plus a stubbed acquireVsCodeApi and the
// test driver, so a headless browser can check the probe.
//   argv: <pageFile> <outHtml> <driverJs> [liveJson]
//   liveJson: 機種與機台設定 as the extension gives it ({ machine, docs }); pathFix is added the way the
//   extension does (the page folder's JSON/ -> the web root's, the page being in web\page)
const fs = require('fs');
const path = require('path');
const { pathToFileURL } = require('url');
const { buildPageHtml } = require('../lib/pagehtml');

const [page, outFile, driver, liveFile] = process.argv.slice(2);
const html = fs.readFileSync(page, 'utf8');
const base = pathToFileURL(path.dirname(page) + path.sep).href;
const probe = pathToFileURL(path.join(__dirname, '..', 'media', 'probe.js')).href;
const stub = '<script>window.acquireVsCodeApi=function(){return{postMessage:function(m){' +
  '(window.__htdOut=window.__htdOut||[]).push(JSON.parse(JSON.stringify(m)));}};};</script>';
let live;
if (liveFile) {
  live = JSON.parse(fs.readFileSync(liveFile, 'utf8'));
  const dir = path.dirname(page);
  live.pathFix = { from: pathToFileURL(path.join(dir, 'JSON') + path.sep).href, to: pathToFileURL(path.join(dir, '..', 'JSON') + path.sep).href };
}
let out = buildPageHtml(html, { cspSource: 'file:', baseHref: base, probeSrc: probe, mode: 'design', preScript: stub, live });
const drv = fs.readFileSync(driver, 'utf8');
const i = out.toLowerCase().lastIndexOf('</body>');
const tag = '<script>' + drv + '</script>';
out = i >= 0 ? out.slice(0, i) + tag + out.slice(i) : out + tag;
fs.writeFileSync(outFile, out, 'utf8');
