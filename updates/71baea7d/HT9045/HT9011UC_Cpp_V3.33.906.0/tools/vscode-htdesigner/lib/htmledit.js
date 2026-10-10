'use strict';
// AI(W906-HTDESIGNER) 20260929: WPF-style edits written back into the page source.
// The designer changes the page like the WPF designer changes XAML: every edit is a
// text edit of the .html (undo/redo/save are VS Code's). This file finds the exact
// characters to change -- one start tag's style / attribute, or one text node -- and
// never touches anything else. Plain Node (no vscode).

const VOID = new Set(['area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'param', 'source', 'track', 'wbr']);

function escRe(s) { return String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }
/** 1006 audit (performance): where `needle` is next, any case, from `from` -- without lower-casing the whole text (a
 *  500 KB page lower-cased once per <script> passed was most of the time of every parent / range lookup) */
function findCI(text, needle, from) {
  const re = new RegExp(escRe(needle), 'gi');
  re.lastIndex = from || 0;
  const m = re.exec(text);
  return m ? m.index : -1;
}

/** End of the tag that starts at `s` ('<'), quote-aware: index of its '>' (or -1). */
function tagEnd(text, s) {
  // (review of 0.412 lint #1 / of 0.413 tagEnd: the browser's tokenizer -- a quote opens a value only in "before the value"
  //  (after the = that follows a NAME): alt=Don't, style="a:"b;", href=?q= "x>y", b=="x>y", <a ="x>y"> all end at the
  //  browser's > now. <! / <? (doctype, comment, processing instruction): the first >)
  const n = text.length;
  if (text[s + 1] === '!' || text[s + 1] === '?') { const k = text.indexOf('>', s + 1); return k; }
  const sp = c => c === ' ' || c === '\t' || c === '\n' || c === '\r' || c === '\f';
  // TN tag name, BN before a name, N name, AN after a name, BV before a value, UV unquoted value, Q quoted, AQ after quoted
  let st = 'TN', q = null;
  for (let i = s + (text[s + 1] === '/' ? 2 : 1); i < n; i++) {
    const c = text[i];
    if (st === 'Q') { if (c === q) st = 'AQ'; continue; }
    if (c === '>') return i;
    switch (st) {
      case 'TN': if (sp(c) || c === '/') st = 'BN'; break;
      case 'BN': if (!sp(c) && c !== '/') st = 'N'; break;
      case 'N': if (sp(c)) st = 'AN'; else if (c === '/') st = 'BN'; else if (c === '=') st = 'BV'; break;
      case 'AN': if (c === '/') st = 'BN'; else if (c === '=') st = 'BV'; else if (!sp(c)) st = 'N'; break;
      case 'BV': if (c === '"' || c === "'") { st = 'Q'; q = c; } else if (!sp(c)) st = 'UV'; break;
      case 'UV': if (sp(c)) st = 'BN'; break;
      case 'AQ': if (sp(c) || c === '/') st = 'BN'; else st = 'N'; break;
    }
  }
  return -1;
}

/** [start, end) of the comments and the script / style contents of a text (cached for the last text) -- not markup */
let deadCache = { text: null, ranges: [] };
function deadRanges(text) {
  if (deadCache.text === text) return deadCache.ranges;
  const r = [];
  const re = /<!--|<script\b|<style\b/gi;
  let m;
  while ((m = re.exec(text))) {
    const s0 = m.index;
    if (m[0] === '<!--') { const e = text.indexOf('-->', s0 + 4); const end = e < 0 ? text.length : e + 3; r.push([s0, end]); re.lastIndex = end; continue; }
    const te = tagEnd(text, s0);
    if (te < 0) break;
    const ce = new RegExp('</' + m[0].slice(1), 'gi');
    ce.lastIndex = te + 1;
    const cm = ce.exec(text);
    const end = cm ? cm.index : text.length;
    r.push([te + 1, end]);
    re.lastIndex = Math.max(end, te + 1);
  }
  deadCache = { text, ranges: r };
  return r;
}
function inRanges(r, p) {
  let lo = 0, hi = r.length - 1;
  while (lo <= hi) { const md = (lo + hi) >> 1; if (p < r[md][0]) hi = md - 1; else if (p >= r[md][1]) lo = md + 1; else return true; }
  return false;
}
/**
 * 1006 audit: a start tag's attributes, quote-aware -- [{ name, ws, ns, ne, vs, ve, q, end }]: ws = where its leading
 * whitespace starts, ns..ne its name, vs..ve its value (inside the quotes), q = '"' / "'" / '' (unquoted) / null (none),
 * end = just after it. (A regex used to find "disabled" inside a title, or value="…" inside a placeholder.)
 */
function attrTokens(tagText) {
  const out = [];
  const t = String(tagText);
  const m0 = /^<[A-Za-z][\w:-]*/.exec(t);
  if (!m0) return out;
  const n = t.length;
  const sp = c => c === ' ' || c === '\t' || c === '\n' || c === '\r' || c === '\f';
  let i = m0[0].length;
  for (;;) {
    const ws = i;
    while (i < n && sp(t[i])) i++;
    if (i >= n || t[i] === '>' || (t[i] === '/' && t[i + 1] === '>')) break;
    if (t[i] === '/') { i++; continue; }
    const ns = i;
    while (i < n && !sp(t[i]) && t[i] !== '=' && t[i] !== '>' && !(t[i] === '/' && t[i + 1] === '>')) i++;
    const ne = i;
    if (ne === ns) { i++; continue; }
    const tok = { name: t.slice(ns, ne).toLowerCase(), ws, ns, ne, vs: -1, ve: -1, q: null, end: ne };
    let j = i;
    while (j < n && sp(t[j])) j++;
    if (t[j] === '=') {
      j++;
      while (j < n && sp(t[j])) j++;
      if (t[j] === '"' || t[j] === "'") {
        const k = t.indexOf(t[j], j + 1), ke = k < 0 ? n - 1 : k;
        tok.q = t[j]; tok.vs = j + 1; tok.ve = ke; tok.end = ke + 1; i = ke + 1;
      } else {
        let k = j;
        while (k < n && !sp(t[k]) && t[k] !== '>') k++;
        tok.q = ''; tok.vs = j; tok.ve = k; tok.end = k; i = k;
      }
    }
    out.push(tok);
  }
  return out;
}
// (review of 0.445 #1: no prototype -- "&toString;" read as a function's source)
const NAMED_ENT = Object.assign(Object.create(null), { amp: '&', lt: '<', gt: '>', quot: '"', apos: "'", nbsp: '\u00a0', hellip: '\u2026', larr: '\u2190', rarr: '\u2192',
  uarr: '\u2191', darr: '\u2193', harr: '\u2194', mdash: '\u2014', ndash: '\u2013', middot: '\u00b7', bull: '\u2022', copy: '\u00a9',
  reg: '\u00ae', trade: '\u2122', deg: '\u00b0', times: '\u00d7', divide: '\u00f7', plusmn: '\u00b1', laquo: '\u00ab', raquo: '\u00bb',
  lsquo: '\u2018', rsquo: '\u2019', ldquo: '\u201c', rdquo: '\u201d', micro: '\u00b5', sup2: '\u00b2', sup3: '\u00b3', ohm: '\u03a9',
  le: '\u2264', ge: '\u2265', ne: '\u2260', infin: '\u221e', check: '\u2713', cross: '\u2717', ensp: '\u2002', emsp: '\u2003', thinsp: '\u2009' });
// (review of 0.445 #2: the legacy upper-case spellings HTML itself takes -- only these; "&rArr;" (⇒) is not "&rarr;" (→))
const NAMED_UPPER = Object.assign(Object.create(null), { AMP: '&', LT: '<', GT: '>', QUOT: '"', COPY: '\u00a9', REG: '\u00ae' });
/**
 * 1010 review of props values #1: why a CSS value would break the declarations after it ('' = fine) -- a quote not closed
 * ("'abc" ran to the end of the style), parentheses not balanced ("calc(10px", "Arial)"), a comment opened ("Arial /* x")
 */
function cssValueWhy(v) {
  const s = String(v == null ? '' : v);
  let q = null, depth = 0;
  for (let i = 0; i < s.length; i++) {
    const c = s[i];
    if (q) { if (c === '\\') { i++; continue; } if (c === q) q = null; continue; }
    if (c === '"' || c === "'") { q = c; continue; }
    if (c === '/' && s[i + 1] === '*') return '不能有 /*（註解）';
    if (c === '(') depth++;
    else if (c === ')' && --depth < 0) return '括號 ) 多了';
  }
  if (q) return '引號 ' + q + ' 沒有收尾';
  if (depth > 0) return '括號 ( 沒有收尾';
  return '';
}
/** an attribute's / a text's characters as the browser reads them (the entities the generator writes, and numeric ones) */
function decodeEnt(v) {
  // (1010 review of props values #5: the common named ones too -- "starting&hellip;" edited became "&amp;hellip;")
  return String(v).replace(/&(#x[0-9a-f]+|#\d+|[a-z][a-z0-9]*);/gi, (m0, e) => {   // (review of 0.446: digits in a name too -- &sup2; / &sup3;)
    const k = e[0] === '#' ? e.toLowerCase() : e;
    if (k[0] === '#') { const c = k[1] === 'x' ? parseInt(k.slice(2), 16) : parseInt(k.slice(1), 10); return isFinite(c) ? String.fromCodePoint(c) : m0; }
    const t = NAMED_ENT[k] !== undefined ? NAMED_ENT[k] : NAMED_UPPER[k];
    return t !== undefined ? t : m0;
  });
}

/** Start tag of the element with id="…" ('@form' = the generated <div class="form">). */
function startTagOf(text, id) {
  let at;
  if (id === '@form') {
    // (1007 audit, props K: "form" as one class among others -- class="form X" made every later form edit refused)
    const m = /<div\b[^>]*\bclass\s*=\s*["'](?:[^"']*\s)?form(?:\s[^"']*)?["'][^>]*>/.exec(text);
    if (!m) return null;
    at = m.index;
  } else {
    // (1006 audit, every id of every page: the first regex hit used to be taken -- an id written inside the page's
    //  <script> ('<span id="teachHandPos">' in a JS string, or a JS "<" comparison before id="…") gave a fake tag, and an
    //  edit was written into the JavaScript; "data-id" counted as "id". Now: an id attribute of a real start tag, outside
    //  comments and script / style content, with its own quote.)
    at = -1;
    const dead = deadRanges(text);
    // (1009 review (edit core #2): an id is case-sensitive -- 'gi' found <span id="label1"> for Label1, and every edit of
    //  the later of the two landed on the earlier)
    const re = new RegExp('\\sid\\s*=\\s*(?:"' + escRe(id) + '"|\'' + escRe(id) + '\')', 'g');
    let m;
    while ((m = re.exec(text))) {
      const p = m.index;
      if (inRanges(dead, p)) continue;
      const ls = text.lastIndexOf('<', p);
      if (ls < 0 || !/^<[A-Za-z]/.test(text.slice(ls, ls + 2)) || inRanges(dead, ls)) continue;
      const le = tagEnd(text, ls);
      if (le < p) continue;
      const tok = attrTokens(text.slice(ls, le + 1)).find(x => x.name === 'id');
      // (the id attribute of THIS tag at exactly this place -- not "id=…" written inside another attribute's value)
      if (!tok || ls + tok.ns !== p + m[0].search(/[^\s]/)) continue;
      at = ls;
      break;
    }
    if (at < 0) return null;
  }
  const end = tagEnd(text, at);
  if (end < 0) return null;
  const name = /^<([A-Za-z][\w-]*)/.exec(text.slice(at, at + 40))[1].toLowerCase();
  return { start: at, end: end + 1, name, text: text.slice(at, end + 1) };
}

/**
 * The start tag right before `tagStart`, when it is the element's direct wrapper:
 * <span style="position:absolute;…"><input id="XST1" …> (only whitespace between).
 */
function wrapperTagOf(text, tagStart) {
  let i = tagStart - 1;
  while (i >= 0 && /\s/.test(text[i])) i--;
  // (1007 audit, toolbox #8: <span …><!-- note --><input id=…> -- the comment skipped, the span is still the wrapper)
  for (let guard = 0; guard < 20 && i >= 2 && text.slice(i - 2, i + 1) === '-->'; guard++) {
    const c = text.lastIndexOf('<!--', i - 2);
    if (c < 0) return null;
    i = c - 1;
    while (i >= 0 && /\s/.test(text[i])) i--;
  }
  if (i < 0 || text[i] !== '>') return null;
  // 1006 (the enumeration -- every component of every page deleted: deleting HW.home's Panel2 hung the extension at
  // 100% CPU): right before it was a comment's "-->" -- no tag ends there, the walk back reached the start, and
  // lastIndexOf('<', -1) is 0 again (a negative position counts as 0), so s stayed 0 for ever. A comment's end is no
  // wrapper; the walk stops at the start of the text.
  if (text.slice(Math.max(0, i - 2), i + 1) === '-->') return null;
  // walk back to that tag's '<' (quote-aware enough for generated markup)
  let s = text.lastIndexOf('<', i);
  while (s >= 0 && tagEnd(text, s) !== i) s = s > 0 ? text.lastIndexOf('<', s - 1) : -1;
  if (s < 0) return null;
  const t = text.slice(s, i + 1);
  if (/^<\//.test(t) || /^<!/.test(t)) return null;
  const name = /^<([A-Za-z][\w-]*)/.exec(t);
  if (!name || VOID.has(name[1].toLowerCase())) return null;
  return { start: s, end: i + 1, name: name[1].toLowerCase(), text: t };
}

/**
 * style="…" of a start tag: { decls:[{ name, value, s, e, vs, ve }], range:[s,e] of the
 * value, quote }. s/e = the whole declaration incl. its ';', vs/ve = the value only
 * (offsets in the tag text).
 */
function styleOf(tagText) {
  // (1009 review: by the attribute tokens -- the regex took " style='…'" inside a title / data-* value for the style, and
  //  did not see style=left:3px written without quotes)
  const a = attrTokens(tagText).find(x => x.name === 'style' && x.q !== null);
  if (!a) return { decls: [], range: null, quote: '"' };
  const base = a.vs;
  const v = String(tagText).slice(a.vs, a.ve);
  const m = [null, a.q || '"'];
  const decls = [];
  // split on ';' outside parentheses and quotes (font-family:"MS Sans Serif",sans-serif)
  let depth = 0, q = null, from = 0;
  const cut = (a, b, withSemi) => {
    const part = v.slice(a, b);
    const k = part.indexOf(':');
    if (k < 0 || !part.slice(0, k).trim()) return;
    const lead = part.search(/\S/);
    const rawVal = part.slice(k + 1);
    const vLead = rawVal.search(/\S/);
    const vs = a + k + 1 + (vLead < 0 ? 0 : vLead);
    const ve = a + k + 1 + rawVal.replace(/\s+$/, '').length;
    decls.push({ name: part.slice(0, k).trim().toLowerCase(), value: v.slice(vs, ve), s: base + a + lead, e: base + b + (withSemi ? 1 : 0), vs: base + vs, ve: base + Math.max(vs, ve) });
  };
  for (let i = 0; i < v.length; i++) {
    const c = v[i];
    // (1009 second review (edit core #2): an entity is one unit -- the ";" of &quot; ended the declaration
    //  (font-family:&quot;Arial&quot;,sans-serif changed to Tahoma left Arial&quot;,sans-serif behind, and its " swallowed
    //  font-weight:700); &quot; / &#39; are quotes of the CSS)
    if (c === '&') {
      const em = /^&(#\d+|#x[0-9a-f]+|[a-z]\w*);/i.exec(v.slice(i, i + 12));
      if (em) {
        // (1009 regression review #8: &quot; / &#34; / &#x22; are the same quote, &apos; / &#39; / &#x27; the other)
        const e0 = em[0].toLowerCase();
        const ent = /^&(quot|#34|#x22);$/.test(e0) ? '&quot;' : /^&(apos|#39|#x27);$/.test(e0) ? '&apos;' : e0;
        const isQ = ent === '&quot;' || ent === '&apos;';
        if (q && isQ && q === ent) q = null; else if (!q && isQ) q = ent;
        i += em[0].length - 1;
        continue;
      }
    }
    if (q) { if (c === q) q = null; continue; }
    if (c === '"' || c === "'") { q = c; continue; }
    if (c === '(') depth++;
    if (c === ')') depth--;
    if (c === ';' && depth === 0) { cut(from, i, true); from = i + 1; }
  }
  if (v.slice(from).trim()) cut(from, v.length, false);
  return { decls, range: [base, base + v.length], quote: m[1] };
}

/**
 * New start tag with style properties changed. changes: { left: '12px', display: null }
 * (null = remove). Only the characters of the changed declarations move: an existing
 * one gets its value replaced in place (the LAST occurrence -- the one CSS uses; earlier
 * duplicates are removed), a new one is appended, a removed one is cut out.
 */
function setStyle(tagText, changes, opts) {
  // (1009 review: an unquoted style= gets its quotes first -- "left:3px;top:4px" cannot be written into it bare; a bare
  //  "style" with no value goes, else a second style attribute was added after it and the browser used the first)
  const sa = attrTokens(tagText).find(x => x.name === 'style');
  if (sa && sa.q === '') return setStyle(tagText.slice(0, sa.vs) + '"' + tagText.slice(sa.vs, sa.ve).replace(/"/g, "'") + '"' + tagText.slice(sa.ve), changes, opts);
  if (sa && sa.q === null) return setStyle(tagText.slice(0, sa.ws) + tagText.slice(sa.end), changes, opts);
  const st = styleOf(tagText);
  // (opts.after { name: the declaration it goes after, '^' = first } -- 1007 audit, props L: a declaration taken away
  //  and given back goes where it was, not to the end)
  const after = (opts && opts.after) || {};
  const q = st.quote;
  // (review of 0.410 props #3: an & typed is written as &amp; -- the grid shows the values decoded)
  // (1010 review of props values #2: the attribute's own quote written as its entity (styleOf reads &quot; / &#39; as CSS
  //  quotes) -- swapped to the other kind, "Tom's Font" became 'Tom's Font' and cut the declarations after it)
  const clean = s => { const a = String(s).replace(/&/g, '&amp;'); return q === '"' ? a.replace(/"/g, '&quot;') : a.replace(/'/g, '&#39;'); };
  const edits = [];   // [start, end, replacement] in tag-text offsets
  const append = [];
  for (const [name, value] of Object.entries(changes)) {
    const key = name.toLowerCase();
    const hits = st.decls.filter(d => d.name === key);
    if (value == null) {
      for (const d of hits) {
        let s0 = d.s;
        // (1007 audit, props B2: the last declaration written without its ';' -- appended after a ';' this function
        //  added -- takes that ';' with it, so the style is as it was)
        if (st.range && d.e === st.range[1] && tagText[d.e - 1] !== ';') { let k = s0 - 1; while (k >= st.range[0] && /\s/.test(tagText[k])) k--; if (k >= st.range[0] && tagText[k] === ';') s0 = k; }
        edits.push([s0, d.e, '']);
      }
      continue;
    }
    if (!hits.length) {
      const af = after[key];
      const pd = af && af !== '^' ? st.decls.filter(d => d.name === af).pop() : null;
      if (st.range && af === '^' && st.decls.length) { edits.push([st.decls[0].s, st.decls[0].s, key + ':' + clean(value) + ';']); continue; }
      if (st.range && pd) {
        const semi = tagText[pd.e - 1] === ';';
        edits.push([pd.e, pd.e, semi ? key + ':' + clean(value) + ';' : ';' + key + ':' + clean(value)]);
        continue;
      }
      append.push(key + ':' + clean(value));
      continue;
    }
    // (1007 audit, props D: the generator writes a label as width:136px;…;width:auto -- the earlier ones were deleted, so
    //  AutoSize off and on lost width:136px. Only the last (the one that counts) is changed now)
    const last = hits[hits.length - 1];
    edits.push([last.vs, last.ve, clean(value)]);
  }
  let t = tagText;
  if (!st.range) {
    if (!append.length) return t;
    const close = /\s*\/?>$/.exec(t);
    const at = close ? close.index : t.length - 1;
    return t.slice(0, at) + ' style=' + q + append.map(x => x + ';').join('') + q + t.slice(at);
  }
  if (append.length) {
    // after the value's last char; a style that ended without ';' gets ";decl" (no ';' after: removing it again leaves
    // the style as it was), one that ended with ';' gets "decl;"
    const valEnd = st.range[1];
    const lastDecl = st.decls[st.decls.length - 1];
    const needSemi = lastDecl && lastDecl.e === valEnd && tagText[valEnd - 1] !== ';';
    edits.push([valEnd, valEnd, needSemi ? ';' + append.join(';') : append.map(x => x + ';').join('')]);
  }
  edits.sort((a, b) => b[0] - a[0] || b[1] - a[1]);
  for (const [s, e, r] of edits) t = t.slice(0, s) + r + t.slice(e);
  // (1007 audit, props B: Visible / Underline off and on again left style="" on a tag that had no style before -- a style
  //  emptied by this edit goes, attribute and all)
  const was = tagText.slice(st.range[0], st.range[1]).trim();
  const now = styleOf(t);
  if (was && now.range && !t.slice(now.range[0], now.range[1]).trim()) {
    const a = attrTokens(t).find(x => x.name === 'style');
    if (a) { let s0 = a.ns; while (s0 > 0 && /\s/.test(t[s0 - 1])) s0--; t = t.slice(0, s0) + t.slice(a.ve + (a.q ? 1 : 0)); }
  }
  return t;
}

/**
 * 1007 audit (props L): two CSS values that mean the same -- #000 / #000000 / rgb(0,0,0), font-weight 400 / normal and
 * 700 / bold, a font family with or without its quotes, 12px / 12.0px. Used to keep the source's own spelling.
 */
function cssSame(name, a, b) {
  if (a == null || b == null) return a == b;
  const k = String(name).toLowerCase(), x = String(a).trim().toLowerCase(), y = String(b).trim().toLowerCase();
  if (x === y) return true;
  const rgb = v => {
    let m = /^#([0-9a-f]{3})$/.exec(v);
    if (m) return m[1].split('').map(c => parseInt(c + c, 16)).join();
    m = /^#([0-9a-f]{6})$/.exec(v);
    if (m) return [0, 2, 4].map(i => parseInt(m[1].slice(i, i + 2), 16)).join();
    m = /^rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)$/.exec(v);
    return m ? [m[1], m[2], m[3]].map(Number).join() : null;
  };
  if (/color$/.test(k)) { const p = rgb(x), q = rgb(y); return p !== null && p === q; }
  if (k === 'font-weight') { const w = v => ({ normal: '400', bold: '700' }[v] || v); return w(x) === w(y); }
  if (k === 'font-family') { const f = v => v.split(',').map(s => s.trim().replace(/^["']|["']$/g, '')).join(','); return f(x) === f(y); }
  const n = v => { const m = /^(-?\d+(?:\.\d+)?)(px|%|em|pt)?$/.exec(v); return m ? parseFloat(m[1]) + (m[2] || '') : null; };
  return n(x) !== null && n(x) === n(y);
}

function escAttr(s) { return String(s).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;'); }
function escText(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }

/** New start tag with an attribute set (value string), made boolean (true), or removed (null). */
// (1006 audit, every attribute of every page: it used to cut the attribute out and add it again before ">" -- every
//  edit moved it, lost the line break before it, re-encoded "&" -- and its regex was not quote-aware. Now the value is
//  changed where it is, in its own quotes; the same value = the tag unchanged.)
/* (after: 1007 audit, props L -- a new attribute goes right after this one ('^' = right after the tag name), where it
   was before it was taken away; else before '>') */
function setAttr(tagText, name, value, after) {
  const t = String(tagText);
  const a = attrTokens(t).find(x => x.name === String(name).toLowerCase());
  if (value == null || value === false) return a ? t.slice(0, a.ws) + t.slice(a.end) : t;
  if (value === true) {
    if (a) return t;
  } else if (a) {
    const v = String(value);
    if (a.q === null) return t.slice(0, a.ne) + '="' + escAttr(v) + '"' + t.slice(a.ne);
    if (decodeEnt(t.slice(a.vs, a.ve)) === v) return t;
    if (a.q === "'") return t.slice(0, a.vs) + v.replace(/&/g, '&amp;').replace(/'/g, '&#39;').replace(/</g, '&lt;') + t.slice(a.ve);
    if (a.q === '"') return t.slice(0, a.vs) + escAttr(v) + t.slice(a.ve);
    return t.slice(0, a.vs) + '"' + escAttr(v) + '"' + t.slice(a.ve);
  }
  const close = /\s*\/?>$/.exec(t);
  let at = close ? close.index : t.length - 1;
  if (after === '^') { const nm = /^<[A-Za-z][\w-]*/.exec(t); if (nm) at = nm[0].length; }
  else if (after) { const p = attrTokens(t).find(x => x.name === String(after).toLowerCase()); if (p) at = p.end; }
  const piece = value === true ? ' ' + name : ' ' + name + '="' + escAttr(value) + '"';
  return t.slice(0, at) + piece + t.slice(at);
}

/**
 * Where the caption text of an element lives in the source, as [start, end) of the
 * raw text (possibly empty, an insertion point). kind:
 *   'text'   first non-blank text directly inside the element (button, label, span)
 *   'legend' the text of its first <legend>            (TGroupBox caption)
 *   'pnlCap' the text of its first <span class="pnlCap"> (TPanel caption)
 * Returns null when the element's end is not found.
 */
function captionRange(text, tag, kind) {
  // (1006 audit: an <input> / <img> has no content -- the walk used to go on to a sibling's text or the parent's end)
  const tn = (/^<([A-Za-z][\w-]*)/.exec(tag.text || '') || [])[1];
  if (!tn || VOID.has(tn.toLowerCase()) || /\/>$/.test(tag.text)) return null;
  let i = tag.end;
  let depth = 0;
  const n = text.length;
  // (HTML's whitespace, not JavaScript's \s: a caption that is just a no-break space is text, as the probe reads it)
  const NWS = /[^ \t\n\r\f]/;
  while (i < n) {
    const lt = text.indexOf('<', i);
    const segEnd = lt < 0 ? n : lt;
    if (depth === 0 && kind === 'text' && segEnd > i && NWS.test(text.slice(i, segEnd))) {
      const raw = text.slice(i, segEnd);
      const a = i + raw.search(NWS);
      const b = i + raw.replace(/[ \t\n\r\f]+$/, '').length;
      return [a, b];
    }
    if (lt < 0) break;
    if (text.startsWith('<!--', lt)) { const e = text.indexOf('-->', lt); i = e < 0 ? n : e + 3; continue; }
    const e = tagEnd(text, lt);
    if (e < 0) break;
    const t = text.slice(lt, e + 1);
    const close = /^<\/([A-Za-z][\w-]*)/.exec(t);
    if (close) {
      if (depth === 0) {
        // end of our element: an empty 'text' caption is inserted here
        return kind === 'text' ? [lt, lt] : null;
      }
      depth--;
      i = e + 1;
      continue;
    }
    const open = /^<([A-Za-z][\w-]*)/.exec(t);
    if (open) {
      const nm = open[1].toLowerCase();
      if (depth === 0 && ((kind === 'legend' && nm === 'legend') || (kind === 'pnlCap' && /\bclass\s*=\s*["'][^"']*\bpnlCap\b/.test(t)))) {
        const inner = text.indexOf('<', e + 1);
        return inner < 0 ? null : [e + 1, inner];
      }
      if (nm === 'script' || nm === 'style') {
        const ce = new RegExp('</' + nm, 'gi');
        ce.lastIndex = e + 1;
        const cm = ce.exec(text);
        i = cm ? cm.index : n;
        continue;
      }
      if (!VOID.has(nm) && !/\/>$/.test(t)) depth++;
    }
    i = e + 1;
  }
  return null;
}

/**
 * The start tag of the child that carries an element's caption -- and so its font:
 * kind 'legend' (TGroupBox) or 'pnlCap' (TPanel: the generator writes the panel's
 * Font on <span class="pnlCap" style="font-size:…">). { start, end, text } or null.
 */
function captionHostTag(text, tag, kind) {
  if (kind !== 'legend' && kind !== 'pnlCap') return null;
  const r = captionRange(text, tag, kind);          // [just after the child's start tag, …)
  if (!r) return null;
  const lt = text.lastIndexOf('<', r[0] - 1);
  if (lt < tag.end) return null;
  const e = tagEnd(text, lt);
  if (e < 0 || e + 1 !== r[0]) return null;
  return { start: lt, end: e + 1, text: text.slice(lt, e + 1) };
}

/**
 * 0.152: the start tag of the check box inside a <label> (TCheckBox / TRadioButton: <label class="ckb"><input type=checkbox>):
 * its Checked is written there. Only before the label's own end -- not some later <input>.
 */
function innerInputTag(text, tag) {
  // (1006 audit: for any other element it found an <input> anywhere later -- in another element, or a JS string)
  if (!/^<label\b/i.test(tag.text || '')) return null;
  const rc = /<\/label/gi; rc.lastIndex = tag.end;
  const cm = rc.exec(text);
  const ri = /<input\b/gi; ri.lastIndex = tag.end;
  const im = ri.exec(text);
  const close = cm ? cm.index : -1, i = im ? im.index : -1;
  if (i < 0 || (close >= 0 && i > close)) return null;
  const e = tagEnd(text, i);
  if (e < 0) return null;
  return { start: i, end: e + 1, text: text.slice(i, e + 1) };
}

/** The attributes a start tag writes, in order: [[name, value], ...] (a bare one = ''). */
function attrsOf(tagText) {
  const out = [];
  const body = String(tagText).replace(/^<[A-Za-z][\w-]*/, '').replace(/\/?>$/, '');
  const re = /\s*([^\s=/>"']+)(?:\s*=\s*("([^"]*)"|'([^']*)'|([^\s>]+)))?/g;
  let m;
  while ((m = re.exec(body))) {
    if (!m[1]) break;
    out.push([m[1].toLowerCase(), m[3] !== undefined ? m[3] : m[4] !== undefined ? m[4] : m[5] !== undefined ? m[5] : '']);
    if (m[0].length === 0) re.lastIndex++;
  }
  return out;
}

/**
 * Where a property (its VCL name, as the properties grid shows it) is written in the element's markup -- the style
 * declaration's value, the attribute, the caption text: [start, end) in `text`, or null (not written there).
 * Left / Top / Width / Height also on the positioning wrapper <span>; a panel's font / alignment also on its caption
 * <span class="pnlCap">. (EastSun 20260930: "和 wpf 改 code 一樣方便有效率" -- a double-click on a property's name goes
 * to it in the markup.)
 */
const PROP_CSS = { Left: ['left'], Top: ['top'], Width: ['width'], Height: ['height'], AutoSize: ['width', 'height'],
  'Font.Name': ['font-family'], 'Font.Size': ['font-size'], 'Font.Bold': ['font-weight'], 'Font.Italic': ['font-style'],
  'Font.Color': ['color'], Color: ['background-color', 'background'], Alignment: ['text-align'], Visible: ['display', 'visibility'],
  // the IO lamp (TALed family: span.aled) / panel button (TBtnPanel family: div.btnpanel) colours are CSS variables
  TrueColor: ['--led-on', '--bp-true'], FalseColor: ['--led-off', '--bp-false'], TrueFontColor: ['--bp-true-font'], FalseFontColor: ['--bp-false-font'] };
// (the IO components' properties that are classes: LEDStyle / Value / Blink / Down / Style -> the class attribute)
const PROP_CLASS = { LEDStyle: 1, Value: 1, Blink: 1, Down: 1, Style: 1 };
function propRange(text, id, prop) {
  const tag = startTagOf(text, id);
  if (!tag) return null;
  const css = PROP_CSS[prop];
  if (css) {
    const tags = [tag];
    if (/^(Left|Top|Width|Height|AutoSize)$/.test(prop)) { const w = wrapperTagOf(text, tag.start); if (w) tags.push(w); }
    if (/^(Font\.|Alignment$)/.test(prop)) { const c = captionHostTag(text, tag, 'pnlCap'); if (c) tags.push(c); }
    for (const t of tags) {
      const st = styleOf(t.text);
      for (const n of css) { const dc = st.decls.find(x => x.name === n); if (dc) return [t.start + dc.vs, t.start + dc.ve]; }
    }
    return null;
  }
  const attr = name => {
    const a = attrTokens(tag.text).find(x => x.name === name);
    if (!a) return null;
    return a.q !== null ? [tag.start + a.vs, tag.start + a.ve] : [tag.start + a.ns, tag.start + a.ne];
  };
  if (prop === 'Enabled') return attr('disabled');
  if (prop === 'Alias') return attr('title');
  if (PROP_CLASS[prop]) return attr('class');
  if (prop === 'Caption' || prop === 'Text') {
    if (/^<input\b/i.test(tag.text)) return attr('value');
    for (const kind of ['pnlCap', 'legend', 'text']) { const r = captionRange(text, tag, kind); if (r) return r; }
  }
  return null;
}

/**
 * New start tag with classes added / removed IN PLACE (only the class value's characters move -- setAttr would move the
 * whole attribute to the end of the tag). A class already there is not added twice; no class attribute and something to
 * add -> one is appended. (The IO components' LEDStyle / Value / Down / Style are classes: "aled LEDHorizontal on".)
 */
function setClass(tagText, add, remove) {
  // (1010 review of webview messages F4: 'x"onmouseover="…' left the class value -- only a class name's characters)
  const okTok = c => typeof c === 'string' && /^-?[A-Za-z_\u00a0-\uffff][\w\u00a0-\uffff-]*$/.test(c);
  const addL = (add || []).filter(okTok), remL = (remove || []).filter(Boolean);
  // (1009 review: class=lbl without quotes was not seen -- a second class="…" was added, which the browser ignores)
  const a = attrTokens(tagText).find(x => x.name === 'class' && x.q !== null);
  if (!a) return addL.length ? setAttr(tagText, 'class', addL.join(' ')) : tagText;
  if (a.q === '') return setClass(tagText.slice(0, a.vs) + '"' + tagText.slice(a.vs, a.ve) + '"' + tagText.slice(a.ve), add, remove);
  const val = tagText.slice(a.vs, a.ve);
  let toks = val.split(/\s+/).filter(Boolean).filter(c => !remL.includes(c));
  for (const c of addL) if (!toks.includes(c)) toks.push(c);
  const nv = toks.join(' ');
  if (nv === val.trim() && val === val.trim()) return tagText;
  return tagText.slice(0, a.vs) + nv + tagText.slice(a.ve);
}

/**
 * AI(W906-HTDESIGNER) 20261001 (0.135): the start tag of a place a new component can go into, also for the two that
 * startTagOf cannot name --
 *   a tab sheet by its .dfm name: <div class="pcPane" data-p="N" title="tsName"> (no id); else the pane whose data-p is
 *     the data-t of the tab titled "tsName : TTabSheet" in the same PageControl
 *   '@form' on a page without <div class="form"> (main.html is hand-made): its <body> (WPF: the Window's root)
 * -> { start, end, name, text, kind: 'id' | 'form' | 'pane' | 'body' } | null
 */
function containerTagOf(text, id) {
  const t = startTagOf(text, id);
  if (t) return Object.assign(t, { kind: id === '@form' ? 'form' : 'id' });
  const at = s => {
    const end = tagEnd(text, s);
    if (end < 0) return null;
    const name = /^<([A-Za-z][\w-]*)/.exec(text.slice(s, s + 40))[1].toLowerCase();
    return { start: s, end: end + 1, name, text: text.slice(s, end + 1) };
  };
  if (id === '@form') {
    const b = /<body\b[^>]*>/i.exec(text);
    const r = b ? at(b.index) : null;
    return r ? Object.assign(r, { kind: 'body' }) : null;
  }
  if (!/^[A-Za-z_]\w*$/.test(String(id || ''))) return null;
  const paneRe = /<div\b[^>]*\bclass\s*=\s*["'][^"']*\bpcPane\b[^"']*["'][^>]*>/g;
  let m;
  const panes = [];
  while ((m = paneRe.exec(text))) panes.push({ s: m.index, tag: m[0] });
  const titled = panes.find(p => new RegExp('\\btitle\\s*=\\s*["\']' + escRe(id) + '["\']').test(p.tag));
  if (titled) { const r = at(titled.s); return r ? Object.assign(r, { kind: 'pane' }) : null; }
  // by the tab: <div class="tab" data-t="N" title="tsName : TTabSheet"> -> the next pane with data-p="N" after it
  const tabRe = new RegExp('<div\\b[^>]*\\bdata-t\\s*=\\s*["\'](\\d+)["\'][^>]*\\btitle\\s*=\\s*["\']' + escRe(id) + '\\s*:\\s*TTabSheet', 'g');
  const tm = tabRe.exec(text);
  if (!tm) return null;
  const p = panes.find(x => x.s > tm.index && new RegExp('\\bdata-p\\s*=\\s*["\']' + tm[1] + '["\']').test(x.tag));
  const r = p ? at(p.s) : null;
  return r ? Object.assign(r, { kind: 'pane' }) : null;
}

/**
 * 1009 review (live #6): what the SOURCE says of every component, for the designer -- the page's scripts change the DOM
 * (a "---" where a value is shown, a caption span they make, display:none they set) and F2 / the properties read that.
 * -> { caps: { id: { k: 'pnlCap' | 'legend' | 'text' | null, v } }, hid: [ids with display:none in their own / wrapper's style] }
 */
function srcInfo(text) {
  const t = String(text || '');
  const caps = {}, hid = [];
  const dead = deadRanges(t);
  const re = /<([A-Za-z][\w-]*)\b[^>]*?\sid\s*=\s*["']([A-Za-z_][\w]*)["']/g;
  let m;
  while ((m = re.exec(t))) {
    const id = m[2];
    if (Object.prototype.hasOwnProperty.call(caps, id) || inRanges(dead, m.index)) continue;
    const e = tagEnd(t, m.index);
    if (e < 0) continue;
    const tag = { start: m.index, end: e + 1, text: t.slice(m.index, e + 1) };
    const none = tt => styleOf(tt).decls.some(d => d.name.toLowerCase() === 'display' && /^\s*none\b/i.test(d.value));
    const w = wrapperTagOf(t, m.index);
    if (none(tag.text) || (w && !attrTokens(w.text).some(x => x.name === 'id') && styleOf(w.text).decls.some(d => d.name.toLowerCase() === 'position' && /absolute/i.test(d.value)) && none(w.text))) hid.push(id);
    let cap = { k: null, v: '' };
    if (!/^input$/i.test(m[1])) {
      for (const kind of ['pnlCap', 'legend', 'text']) {
        const r = captionRange(t, tag, kind);
        if (r) { cap = { k: kind, v: decodeEnt(t.slice(r[0], r[1])) }; break; }
      }
    }
    caps[id] = cap;
    re.lastIndex = e + 1;
  }
  return { caps, hid };
}

module.exports = { cssValueWhy, srcInfo, deadRanges, inRanges, cssSame, VOID, tagEnd, findCI, attrTokens, decodeEnt, startTagOf, containerTagOf, wrapperTagOf, styleOf, setStyle, setAttr, setClass, attrsOf, captionRange, captionHostTag, innerInputTag, escText, escAttr, propRange };
