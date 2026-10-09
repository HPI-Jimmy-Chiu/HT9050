'use strict';
// AI(W906-HTDESIGNER) 20261006 (EastSun 「繼續」: 兩個頁面比較 / 跨頁面批次改文字): a page's components as data -- id, VCL class,
// caption, place, size, shown or not -- read from the source (not the running page), and two pages compared by id; the
// captions of all pages searched and replaced (text only, never a script, a style or an attribute but an input's value).
// Plain Node (no vscode).
const he = require('./htmledit');

const px = v => { const m = /^\s*(-?\d+(?:\.\d+)?)px\s*$/.exec(String(v || '')); return m ? Math.round(+m[1]) : null; };

/** where a component's caption is in the source: [s, e] (the text, decoded = its caption) or null (it has none) */
function captionAt(text, tag) {
  const name = (/^<([A-Za-z][\w-]*)/.exec(tag.text) || [])[1] || '';
  if (/^input$/i.test(name)) {
    const ty = (he.attrTokens(tag.text).find(a => a.name === 'type') || null);
    const tv = ty && ty.q !== null ? tag.text.slice(ty.vs, ty.ve).toLowerCase() : 'text';
    if (/^(checkbox|radio|range|hidden|file|image|color)$/.test(tv)) return null;
    const v = he.attrTokens(tag.text).find(a => a.name === 'value' && a.q !== null);
    return v ? [tag.start + v.vs, tag.start + v.ve] : null;
  }
  if (/^(select|textarea|img|svg|canvas|table|script|style)$/i.test(name)) return null;
  for (const kind of ['pnlCap', 'legend', 'text']) {
    const r = he.captionRange(text, tag, kind);
    if (r && (kind !== 'text' || r[1] > r[0])) return r;
  }
  return null;
}

/** every component (an element with an id on a real tag) -> Map id -> { tag, cls, caption, capAt, left, top, width, height, hidden } */
// (1007 audit, replace #6: every replace parsed all 82 pages again -- 3.6 s; the same text = the same answer, kept for
//  the last 120 texts)
const MEMO = new Map();
let memoChars = 0;
function componentsOf(text) {
  const key = String(text);
  if (MEMO.has(key)) { const v = MEMO.get(key); MEMO.delete(key); MEMO.set(key, v); return v; }
  const v = componentsOf0(key);
  MEMO.set(key, v);
  // (1009 second review (lint #7): at most 20 MB of page text kept, not 120 whole pages (up to ~920 KB each))
  memoChars += key.length;
  while (MEMO.size > 1 && (MEMO.size > 120 || memoChars > 20 * 1024 * 1024)) { const k0 = MEMO.keys().next().value; memoChars -= k0.length; MEMO.delete(k0); }
  return v;
}
function componentsOf0(text) {
  const out = new Map();
  const re = /\sid\s*=\s*["']([^"'<>]+)["']/g;
  let m;
  while ((m = re.exec(text))) {
    const id = m[1];
    if (out.has(id)) continue;
    const tag = he.startTagOf(text, id);
    if (!tag || tag.start > m.index || tag.end < m.index) continue;
    const title = (he.attrTokens(tag.text).find(a => a.name === 'title' && a.q !== null) || null);
    const tv = title ? he.decodeEnt(tag.text.slice(title.vs, title.ve)) : '';
    const cm = /^\s*[A-Za-z_@][\w]*\s*:\s*(T\w+)/.exec(tv);
    // (an <input> placed by the <span style="position:absolute"> around it: its place and size are on that span)
    const w = he.wrapperTagOf(text, tag.start);
    const sts = [he.styleOf(tag.text)].concat(w ? [he.styleOf(w.text)] : []);
    const d = n => { for (const st of sts) { const x = st.decls.filter(y => String(y.name).toLowerCase() === n).pop(); if (x && px(x.value) !== null || (x && n === 'display')) return String(x.value).trim(); } return null; };
    const capAt = captionAt(text, tag);
    out.set(id, {
      tag: tag.name, cls: cm ? cm[1] : '', caption: capAt ? he.decodeEnt(text.slice(capAt[0], capAt[1])) : null, capAt,
      left: px(d('left')), top: px(d('top')), width: px(d('width')), height: px(d('height')), hidden: /^none\b/i.test(d('display') || ''),
    });
  }
  return out;
}

/**
 * Two pages compared by component id: { onlyA: [{ id, cls, caption }], onlyB: [...], diff: [{ id, cls, fields: [{ f, a, b }] }],
 * same: n }. Fields: 型別 (class), 文字 (caption), 位置 (left, top), 大小 (width, height), 顯示 (hidden).
 */
function compare(textA, textB) {
  const A = componentsOf(textA), B = componentsOf(textB);
  const onlyA = [], onlyB = [], diff = [];
  let same = 0;
  for (const [id, a] of A) {
    const b = B.get(id);
    if (!b) { onlyA.push({ id, cls: a.cls, caption: a.caption }); continue; }
    const fields = [];
    if ((a.cls || a.tag) !== (b.cls || b.tag)) fields.push({ f: '型別', a: a.cls || '<' + a.tag + '>', b: b.cls || '<' + b.tag + '>' });
    if ((a.caption || '') !== (b.caption || '')) fields.push({ f: '文字', a: a.caption || '', b: b.caption || '' });
    if (a.left !== b.left || a.top !== b.top) fields.push({ f: '位置', a: a.left + ', ' + a.top, b: b.left + ', ' + b.top });
    if (a.width !== b.width || a.height !== b.height) fields.push({ f: '大小', a: a.width + ' × ' + a.height, b: b.width + ' × ' + b.height });
    if (a.hidden !== b.hidden) fields.push({ f: '顯示', a: a.hidden ? '隱藏' : '顯示', b: b.hidden ? '隱藏' : '顯示' });
    if (fields.length) diff.push({ id, cls: a.cls || b.cls, fields }); else same++;
  }
  for (const [id, b] of B) if (!A.has(id)) onlyB.push({ id, cls: b.cls, caption: b.caption });
  return { onlyA, onlyB, diff, same, countA: A.size, countB: B.size };
}

/** the comparison as Markdown (a preview the user reads and can save) */
function compareMarkdown(nameA, nameB, r) {
  const esc = v => String(v == null ? '' : v).replace(/\|/g, '\\|').replace(/\r?\n/g, ' ');
  const L = ['# 頁面比較：' + nameA + ' ↔ ' + nameB, '',
    '- ' + nameA + '：' + r.countA + ' 個元件；' + nameB + '：' + r.countB + ' 個元件',
    '- 一樣：' + r.same + '；不一樣：' + r.diff.length + '；只有 ' + nameA + ' 有：' + r.onlyA.length + '；只有 ' + nameB + ' 有：' + r.onlyB.length, ''];
  const only = (title, list) => {
    L.push('## ' + title + '（' + list.length + '）', '');
    if (!list.length) { L.push('（沒有）', ''); return; }
    L.push('| 元件 | 型別 | 文字 |', '|---|---|---|');
    for (const x of list) L.push('| ' + esc(x.id) + ' | ' + esc(x.cls) + ' | ' + esc(x.caption) + ' |');
    L.push('');
  };
  L.push('## 兩邊都有、但不一樣（' + r.diff.length + '）', '');
  if (!r.diff.length) L.push('（沒有）', '');
  else {
    L.push('| 元件 | 型別 | 項目 | ' + esc(nameA) + ' | ' + esc(nameB) + ' |', '|---|---|---|---|---|');
    for (const x of r.diff) for (const f of x.fields) L.push('| ' + esc(x.id) + ' | ' + esc(x.cls) + ' | ' + f.f + ' | ' + esc(f.a) + ' | ' + esc(f.b) + ' |');
    L.push('');
  }
  only('只有 ' + nameA + ' 有', r.onlyA);
  only('只有 ' + nameB + ' 有', r.onlyB);
  return L.join('\n');
}

/**
 * 跨頁面取代文字: the components whose caption contains `find` (exact case; whole = the whole caption only) on every page
 * given -> [{ file, id, cls, at: [s, e], old, now }] -- `now` the caption with every `find` replaced.
 */
function captionHits(pages, find, repl, whole) {
  const out = [];
  if (!find) return out;
  for (const p of pages) {
    for (const [id, c] of componentsOf(p.text)) {
      if (c.caption == null || !c.capAt) continue;
      // (1007 audit, replace #2: <title id="mvDocumentTitle"> is the page's own title, not a component's caption)
      if (/^<title\b/i.test(p.text.slice(p.text.lastIndexOf('<', c.capAt[0]), c.capAt[0]))) continue;
      const hit = whole ? c.caption === find : c.caption.indexOf(find) >= 0;
      if (!hit) continue;
      const now = whole ? repl : c.caption.split(find).join(repl);
      if (now === c.caption) continue;
      out.push({ file: p.file, id, cls: c.cls, at: c.capAt, old: c.caption, now });
    }
  }
  return out;
}

module.exports = { componentsOf, compare, compareMarkdown, captionHits, captionAt };
