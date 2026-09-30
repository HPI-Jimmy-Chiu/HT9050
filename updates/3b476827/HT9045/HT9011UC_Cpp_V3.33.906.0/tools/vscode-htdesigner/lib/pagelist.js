'use strict';
// AI(W906-HTDESIGNER) 20260929: 頁面 -- every page of the web folder, grouped by the
// part of the name before the first dot (Setup.HotPlate.html -> Setup / HotPlate),
// for the page list in the side bar. Plain Node (no vscode).
const fs = require('fs');
const path = require('path');
const pageinfo = require('./pageinfo');

/** The order the groups are shown in; the rest follow alphabetically, then 其他, then web root. */
const ORDER = ['Main', 'Setup', 'HW', 'Data', 'Status', 'Alert', 'Config', 'IDE'];
const OTHER = '其他';
const ROOT = 'web 根目錄';

function head(file) {
  try {
    const fd = fs.openSync(file, 'r');
    const buf = Buffer.alloc(8192);
    const len = fs.readSync(fd, buf, 0, buf.length, 0);
    fs.closeSync(fd);
    return buf.subarray(0, len).toString('utf8');
  } catch (e) {
    return '';
  }
}

/**
 * [{ name, pages: [{ file, name, label, rel, title, redirect }] }] in display order.
 *   webRoot: the web folder (its page\ and the .html files directly in it)
 */
function listPages(webRoot) {
  const groups = new Map();
  const put = (g, p) => { if (!groups.has(g)) groups.set(g, []); groups.get(g).push(p); };
  const one = (dir, name, group) => {
    const file = path.join(dir, name);
    const h = head(file);
    const t = pageinfo.parseTitle(h);
    let title = t ? t.title : '';
    if (!title) { const m = /<title>([^<]*)<\/title>/i.exec(h); title = m ? m[1].trim() : ''; }
    const m = /^([A-Z][A-Za-z0-9]*)\.(.+)\.html?$/.exec(name);
    const g = group || (m ? m[1] : OTHER);
    put(g, {
      file, name, rel: path.relative(webRoot, file),
      label: !group && m ? m[2] : name.replace(/\.html?$/i, ''),
      title, redirect: pageinfo.redirectTarget(h) || '',
    });
  };
  const ls = dir => { try { return fs.readdirSync(dir).filter(n => /\.html?$/i.test(n)).sort((a, b) => a.localeCompare(b, 'en', { sensitivity: 'base' })); } catch (e) { return []; } };
  if (!webRoot) return [];
  for (const n of ls(path.join(webRoot, 'page'))) one(path.join(webRoot, 'page'), n, null);
  for (const n of ls(webRoot)) one(webRoot, n, ROOT);
  const names = Array.from(groups.keys());
  const rank = g => (g === ROOT ? 3 : g === OTHER ? 2 : ORDER.includes(g) ? 0 : 1);
  names.sort((a, b) => rank(a) - rank(b) || (rank(a) === 0 ? ORDER.indexOf(a) - ORDER.indexOf(b) : a.localeCompare(b)));
  return names.map(g => ({ name: g, pages: groups.get(g) }));
}

module.exports = { listPages, ORDER, OTHER, ROOT };
