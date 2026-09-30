'use strict';
// AI(W906-HTDESIGNER) 20260929: WPF-style edits written back into the page source.
// The designer changes the page like the WPF designer changes XAML: every edit is a
// text edit of the .html (undo/redo/save are VS Code's). This file finds the exact
// characters to change -- one start tag's style / attribute, or one text node -- and
// never touches anything else. Plain Node (no vscode).

const VOID = new Set(['area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input', 'link', 'meta', 'param', 'source', 'track', 'wbr']);

function escRe(s) { return String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }

/** End of the tag that starts at `s` ('<'), quote-aware: index of its '>' (or -1). */
function tagEnd(text, s) {
  let q = null;
  for (let i = s + 1; i < text.length; i++) {
    const c = text[i];
    if (q) { if (c === q) q = null; continue; }
    if (c === '"' || c === "'") { q = c; continue; }
    if (c === '>') return i;
  }
  return -1;
}

/** Start tag of the element with id="…" ('@form' = the generated <div class="form">). */
function startTagOf(text, id) {
  let at;
  if (id === '@form') {
    const m = /<div\b[^>]*\bclass\s*=\s*["']form["'][^>]*>/.exec(text);
    if (!m) return null;
    at = m.index;
  } else {
    const re = new RegExp('<[A-Za-z][\\w-]*\\b[^>]*?\\bid\\s*=\\s*["\']' + escRe(id) + '["\']');
    const m = re.exec(text);
    if (!m) return null;
    at = m.index;
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
  if (i < 0 || text[i] !== '>') return null;
  // walk back to that tag's '<' (quote-aware enough for generated markup)
  let s = text.lastIndexOf('<', i);
  while (s >= 0 && tagEnd(text, s) !== i) s = text.lastIndexOf('<', s - 1);
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
  const m = /\sstyle\s*=\s*(["'])([\s\S]*?)\1/i.exec(tagText);
  if (!m) return { decls: [], range: null, quote: '"' };
  const base = m.index + m[0].indexOf(m[1]) + 1;
  const v = m[2];
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
function setStyle(tagText, changes) {
  const st = styleOf(tagText);
  const q = st.quote;
  const clean = s => (q === '"' ? String(s).replace(/"/g, "'") : String(s).replace(/'/g, '"'));
  const edits = [];   // [start, end, replacement] in tag-text offsets
  const append = [];
  for (const [name, value] of Object.entries(changes)) {
    const key = name.toLowerCase();
    const hits = st.decls.filter(d => d.name === key);
    if (value == null) { for (const d of hits) edits.push([d.s, d.e, '']); continue; }
    if (!hits.length) { append.push(key + ':' + clean(value) + ';'); continue; }
    const last = hits[hits.length - 1];
    for (const d of hits.slice(0, -1)) edits.push([d.s, d.e, '']);
    edits.push([last.vs, last.ve, clean(value)]);
  }
  let t = tagText;
  if (!st.range) {
    if (!append.length) return t;
    const close = /\s*\/?>$/.exec(t);
    const at = close ? close.index : t.length - 1;
    return t.slice(0, at) + ' style=' + q + append.join('') + q + t.slice(at);
  }
  if (append.length) {
    // after the value's last char; add a ';' first if the last declaration had none
    const valEnd = st.range[1];
    const lastDecl = st.decls[st.decls.length - 1];
    const needSemi = lastDecl && lastDecl.e === valEnd && tagText[valEnd - 1] !== ';';
    edits.push([valEnd, valEnd, (needSemi ? ';' : '') + append.join('')]);
  }
  edits.sort((a, b) => b[0] - a[0] || b[1] - a[1]);
  for (const [s, e, r] of edits) t = t.slice(0, s) + r + t.slice(e);
  return t;
}

function escAttr(s) { return String(s).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;'); }
function escText(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }

/** New start tag with an attribute set (value string), made boolean (true), or removed (null). */
function setAttr(tagText, name, value) {
  const re = new RegExp('\\s' + escRe(name) + '(\\s*=\\s*("[^"]*"|\'[^\']*\'|[^\\s>]+))?(?=[\\s/>])', 'i');
  const m = re.exec(tagText);
  let t = m ? tagText.slice(0, m.index) + tagText.slice(m.index + m[0].length) : tagText;
  if (value == null || value === false) return t;
  const close = /\s*\/?>$/.exec(t);
  const at = close ? close.index : t.length - 1;
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
  let i = tag.end;
  let depth = 0;
  let firstText = null;
  const n = text.length;
  while (i < n) {
    const lt = text.indexOf('<', i);
    const segEnd = lt < 0 ? n : lt;
    if (depth === 0 && kind === 'text' && segEnd > i && /\S/.test(text.slice(i, segEnd))) {
      const raw = text.slice(i, segEnd);
      const a = i + raw.search(/\S/);
      const b = i + raw.replace(/\s+$/, '').length;
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
        return kind === 'text' ? (firstText || [lt, lt]) : null;
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
        const endTag = text.toLowerCase().indexOf('</' + nm, e + 1);
        i = endTag < 0 ? n : endTag;
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
  'Font.Color': ['color'], Color: ['background-color', 'background'], Alignment: ['text-align'], Visible: ['display', 'visibility'] };
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
    const m = new RegExp('\\s(' + name + ')(\\s*=\\s*("[^"]*"|\'[^\']*\'|[^\\s>]+))?', 'i').exec(tag.text);
    if (!m) return null;
    const q = m[3] ? m[0].lastIndexOf(m[3]) : -1;
    return q >= 0 && /^["']/.test(m[3]) ? [tag.start + m.index + q + 1, tag.start + m.index + q + m[3].length - 1] : [tag.start + m.index + 1, tag.start + m.index + m[0].length];
  };
  if (prop === 'Enabled') return attr('disabled');
  if (prop === 'Alias') return attr('title');
  if (prop === 'Caption' || prop === 'Text') {
    if (/^<input\b/i.test(tag.text)) return attr('value');
    for (const kind of ['pnlCap', 'legend', 'text']) { const r = captionRange(text, tag, kind); if (r) return r; }
  }
  return null;
}

module.exports = { VOID, tagEnd, startTagOf, wrapperTagOf, styleOf, setStyle, setAttr, attrsOf, captionRange, captionHostTag, escText, escAttr, propRange };
