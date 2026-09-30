'use strict';
// AI(W906-HTDESIGNER) 20260929: 在所有頁面搜尋 -- a component by its name or by the text
// it shows, over every page of the web folder at once. Plain Node (no vscode).
//   hit = { file, page, id, cls, caption, line, how: 'id' | 'text' | 'class' }
const fs = require('fs');
const path = require('path');
const pagelist = require('./pagelist');

function decode(s) {
  return String(s).replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&quot;/g, '"').replace(/&#39;/g, "'").replace(/&nbsp;/g, ' ').replace(/&amp;/g, '&');
}

/** Every element with an id in one page's source: { id, cls, caption, at }. */
function elementsOf(text) {
  const out = [];
  const re = /<([A-Za-z][\w-]*)\b([^>]*?)\sid\s*=\s*(["'])([^"']+)\3([^>]*)>/g;
  let m;
  while ((m = re.exec(text))) {
    const attrs = m[2] + ' ' + m[5];
    const t = /\stitle\s*=\s*(["'])([^"']*)\1/.exec(attrs);
    const cm = t ? /^\s*[\w@]+\s*:\s*(T\w+)/.exec(t[2]) : null;
    // the caption: a container's (TPanel / TGroupBox) is its caption child only; an input's is
    // its value; anything else shows the text up to its own end tag, tags left out
    // (<button><img …>Save</button> -> "Save")
    const tag = m[1].toLowerCase();
    // not components: a <style id> / <script id> (their text is CSS / JS) and the like
    if (/^(style|script|template|link|meta|title|head|html|body)$/.test(tag)) continue;
    const after = text.slice(m.index + m[0].length, m.index + m[0].length + 800);
    let cap = '';
    const c = /^\s*<(?:legend|span class="pnlCap")[^>]*>([^<]*)</.exec(after);
    if (c) cap = c[1];
    else if (tag === 'input') { const v = /\svalue\s*=\s*(["'])([^"']*)\1/.exec(attrs); cap = v ? v[2] : ''; }
    else if (tag !== 'div' && tag !== 'fieldset') {
      const end = after.toLowerCase().indexOf('</' + tag);
      cap = (end >= 0 ? after.slice(0, end) : /^([^<]*)/.exec(after)[1]).replace(/<[^>]*>/g, ' ');
    } else cap = /^([^<]*)/.exec(after)[1];
    out.push({ id: m[4], cls: cm ? cm[1] : '', caption: decode(cap).replace(/\s+/g, ' ').trim(), at: m.index });
  }
  return out;
}

const wordsOf = q => String(q || '').trim().toLowerCase().split(/\s+/).filter(Boolean);

// a page's elements (with their line), kept while the file is unchanged: the search box
// above the 頁面 list searches as one types
const cache = new Map();
function pageEntry(file) {
  let st;
  try { st = fs.statSync(file); } catch (e) { return null; }
  const c = cache.get(file);
  if (c && c.mtime === st.mtimeMs && c.size === st.size) return c;
  let text;
  try { text = fs.readFileSync(file, 'utf8'); } catch (e) { return null; }
  const els = elementsOf(text);
  // elementsOf goes front to back: one walk gives every line
  let line = 1, pos = 0;
  for (const e of els) {
    for (; pos < e.at; pos++) if (text.charCodeAt(pos) === 10) line++;
    e.line = line;
  }
  // the text the page shows (a tab's label has no id, so it is no component): no scripts, styles, tags
  const shown = decode(text.replace(/<(script|style|template)\b[^>]*>[\s\S]*?<\/\1\s*>/gi, ' ').replace(/<!--[\s\S]*?-->/g, ' ')
    .replace(/<[^>]*>/g, ' ')).replace(/\s+/g, ' ').toLowerCase();
  const ent = { mtime: st.mtimeMs, size: st.size, els, shown };
  cache.set(file, ent);
  return ent;
}
function pageElements(file) { const e = pageEntry(file); return e ? e.els : null; }

/**
 * Search every page of `webRoot` for `query` (words separated by blanks must all be
 * in the name, the class or the text; case-blind). Best first: name hits, then text.
 */
function searchPages(webRoot, query, limit, groups) {
  const words = wordsOf(query);
  if (!words.length || !webRoot) return [];
  const hits = [];
  for (const g of groups || pagelist.listPages(webRoot)) {
    for (const p of g.pages) {
      if (p.redirect) continue;
      const els = pageElements(p.file);
      if (!els) continue;
      for (const e of els) {
        const idL = e.id.toLowerCase(), capL = e.caption.toLowerCase(), clsL = e.cls.toLowerCase();
        if (!words.every(w => idL.includes(w) || capL.includes(w) || clsL.includes(w))) continue;
        const how = words.every(w => idL.includes(w)) ? 'id' : words.every(w => capL.includes(w) || idL.includes(w)) ? 'text' : 'class';
        hits.push({ file: p.file, page: p.name, group: g.name, id: e.id, cls: e.cls, caption: e.caption, line: e.line, how,
          exact: idL === words.join(' ') });
      }
    }
  }
  const rank = h => (h.exact ? 0 : h.how === 'id' ? 1 : h.how === 'text' ? 2 : 3);
  hits.sort((a, b) => rank(a) - rank(b) || a.page.localeCompare(b.page) || a.line - b.line);
  return hits.slice(0, limit || 500);
}

/**
 * 搜尋頁面 (the box above the 頁面 list, like the search box of Visual Studio's Solution Explorer):
 * the pages whose name / title has every word, and the components that have them, by page, in the
 * list's order; a page with neither but whose shown text has them (a tab's label) is there too (textHit).
 * null for an empty query.
 *   { words, groups: [{ name, pages: [{ ...page, nameHit, textHit, hits }] }], pages, hits, capped }
 */
function filterPages(webRoot, query, limit) {
  const words = wordsOf(query);
  if (!words.length || !webRoot) return null;
  const max = limit || 2000;
  const list = pagelist.listPages(webRoot);
  const all = searchPages(webRoot, query, max + 1, list);
  const capped = all.length > max;
  const byFile = new Map();
  for (const h of all.slice(0, max)) {
    const k = h.file.toLowerCase();
    if (!byFile.has(k)) byFile.set(k, []);
    byFile.get(k).push(h);
  }
  const groups = [];
  let pages = 0, hits = 0;
  for (const g of list) {
    const ps = [];
    for (const p of g.pages) {
      const hay = [p.name, p.label, p.title].join(' ').toLowerCase();
      const nameHit = words.every(w => hay.includes(w));
      const ph = byFile.get(p.file.toLowerCase()) || [];
      let textHit = false;
      if (!nameHit && !ph.length) {
        const ent = p.redirect ? null : pageEntry(p.file);
        textHit = !!(ent && words.every(w => ent.shown.includes(w)));
        if (!textHit) continue;
      }
      ps.push(Object.assign({}, p, { nameHit, textHit, hits: ph }));
      hits += ph.length;
    }
    if (ps.length) { groups.push({ name: g.name, pages: ps }); pages += ps.length; }
  }
  return { words, groups, pages, hits, capped };
}

/** Where the words are in `label` (case-blind), merged: [[start, end], ...] for a TreeItem's highlights. */
function highlightsOf(label, words) {
  const s = String(label || '').toLowerCase();
  const r = [];
  for (const w of words || []) {
    if (!w) continue;
    for (let i = s.indexOf(w); i >= 0; i = s.indexOf(w, i + w.length)) r.push([i, i + w.length]);
  }
  r.sort((a, b) => a[0] - b[0]);
  const out = [];
  for (const x of r) {
    if (out.length && x[0] <= out[out.length - 1][1]) out[out.length - 1][1] = Math.max(out[out.length - 1][1], x[1]);
    else out.push(x.slice());
  }
  return out;
}

module.exports = { elementsOf, searchPages, filterPages, highlightsOf, wordsOf };
