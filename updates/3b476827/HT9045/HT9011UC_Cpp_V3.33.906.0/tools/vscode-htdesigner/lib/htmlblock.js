'use strict';
// AI(W906-HTDESIGNER) 20260929: whole elements of the page source, for the WPF-style
// Delete / Copy / Cut / Paste. The designer re-draws the page from the new source, so
// everything here is text: find an element's whole range (start tag .. its end tag),
// rename the ids of a copied block so none collides, move it a little, and say where
// a pasted block goes. Plain Node (no vscode).
const he = require('./htmledit');

/** Name of a start tag ('<div …>' -> 'div'). */
function nameOf(tagText) {
  const m = /^<([A-Za-z][\w-]*)/.exec(tagText);
  return m ? m[1].toLowerCase() : '';
}

/**
 * [start, end) of the whole element whose start tag is `tag` ({ start, end, text }):
 * through its matching end tag. A void or self-closing element is its start tag.
 * null when the end is not found (unbalanced markup).
 */
function elementRange(text, tag) {
  const nm = nameOf(tag.text);
  if (!nm) return null;
  if (he.VOID.has(nm) || /\/>$/.test(tag.text)) return [tag.start, tag.end];
  if (nm === 'script' || nm === 'style') {
    // its content is not markup ("a < b" in the code): straight to its end tag
    const endTag = text.toLowerCase().indexOf('</' + nm, tag.end);
    const ee = endTag < 0 ? -1 : he.tagEnd(text, endTag);
    return ee < 0 ? null : [tag.start, ee + 1];
  }
  let i = tag.end;
  let depth = 0;
  const n = text.length;
  while (i < n) {
    const lt = text.indexOf('<', i);
    if (lt < 0) return null;
    if (text.startsWith('<!--', lt)) { const e = text.indexOf('-->', lt); if (e < 0) return null; i = e + 3; continue; }
    const e = he.tagEnd(text, lt);
    if (e < 0) return null;
    const t = text.slice(lt, e + 1);
    const close = /^<\/([A-Za-z][\w-]*)/.exec(t);
    if (close) {
      if (depth === 0) return close[1].toLowerCase() === nm ? [tag.start, e + 1] : null;
      depth--;
      i = e + 1;
      continue;
    }
    const open = /^<([A-Za-z][\w-]*)/.exec(t);
    if (open) {
      const on = open[1].toLowerCase();
      if (on === 'script' || on === 'style') {
        const endTag = text.toLowerCase().indexOf('</' + on, e + 1);
        if (endTag < 0) return null;
        const ee = he.tagEnd(text, endTag);
        i = ee < 0 ? n : ee + 1;
        continue;
      }
      if (!he.VOID.has(on) && !/\/>$/.test(t)) depth++;
    }
    i = e + 1;
  }
  return null;
}

/**
 * The unit the designer moves: the element itself, or -- when its position lives on
 * the <span> around it (an <input>, target 'parent') -- that span.
 * { start, end, html, tag } or null.
 */
function unitOf(text, id, target) {
  const tag = he.startTagOf(text, id);
  if (!tag) return null;
  const outer = target === 'parent' ? he.wrapperTagOf(text, tag.start) : null;
  const t = outer || tag;
  const r = elementRange(text, t);
  if (!r) return null;
  return { start: r[0], end: r[1], html: text.slice(r[0], r[1]), tag: t };
}

/** Every id="…" in a block, in order. */
function idsIn(html) {
  const out = [];
  const re = /<[A-Za-z][\w-]*\b[^>]*?\sid\s*=\s*(["'])([^"']+)\1/g;
  let m;
  while ((m = re.exec(html))) out.push(m[2]);
  return out;
}

/** A name not in `used`: spbSave -> spbSave_2, spbSave_2 -> spbSave_3. */
function freshName(id, used) {
  const base = String(id).replace(/_\d+$/, '');
  for (let k = 2; ; k++) {
    const c = base + '_' + k;
    if (!used.has(c)) return c;
  }
}

/**
 * A copy of `html` whose ids are all new (not in `used`, which gets them added):
 * id="…", the generated title="name : TClass…" and <label for="…">. { html, map }.
 */
function renameIds(html, used) {
  const map = new Map();
  for (const id of idsIn(html)) {
    if (map.has(id)) continue;
    const nu = freshName(id, used);
    used.add(nu);
    map.set(id, nu);
  }
  const esc = s => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  let out = html;
  for (const [a, b] of map) {
    out = out.replace(new RegExp('(\\sid\\s*=\\s*["\'])' + esc(a) + '(["\'])', 'g'), '$1' + b + '$2');
    out = out.replace(new RegExp('(\\stitle\\s*=\\s*["\'])' + esc(a) + '(\\s*:)', 'g'), '$1' + b + '$2');
    out = out.replace(new RegExp('(\\sfor\\s*=\\s*["\'])' + esc(a) + '(["\'])', 'g'), '$1' + b + '$2');
  }
  return { html: out, map };
}

/** The block with its first start tag moved by (dx, dy) px (left/top in px only). */
function offsetBlock(html, dx, dy) {
  const e = he.tagEnd(html, 0);
  if (e < 0) return html;
  const tag = html.slice(0, e + 1);
  const st = he.styleOf(tag);
  const px = name => {
    const d = st.decls.filter(x => x.name === name).pop();
    const m = d ? /^(-?\d+(?:\.\d+)?)px$/.exec(d.value) : null;
    return m ? parseFloat(m[1]) : null;
  };
  const ch = {};
  const l = px('left'), t = px('top');
  if (l !== null && dx) ch.left = Math.round(l + dx) + 'px';
  if (t !== null && dy) ch.top = Math.round(t + dy) + 'px';
  if (!Object.keys(ch).length) return html;
  return he.setStyle(tag, ch) + html.slice(e + 1);
}

/** left / top / width / height (px numbers, null when not px) of a block's first start tag. */
function firstPx(html) {
  const e = he.tagEnd(html, 0);
  if (e < 0) return null;
  const st = he.styleOf(html.slice(0, e + 1));
  const px = name => {
    const d = st.decls.filter(x => x.name === name).pop();
    const m = d ? /^(-?\d+(?:\.\d+)?)px$/.exec(d.value) : null;
    return m ? parseFloat(m[1]) : null;
  };
  return { left: px('left'), top: px('top'), width: px('width'), height: px('height') };
}

/** The indentation of the line `at` is on. */
function indentAt(text, at) {
  const ls = text.lastIndexOf('\n', at - 1) + 1;
  const m = /^[ \t]*/.exec(text.slice(ls, at));
  return m ? m[0] : '';
}

/**
 * Where a block pasted INTO a container goes: just before the container's end tag,
 * or -- a generated TGroupBox / TPanel keeps its children in <div class="cli"> --
 * before the end of that. Offset or -1.
 */
function insideEnd(text, tag) {
  const r = elementRange(text, tag);
  if (!r || he.VOID.has(nameOf(tag.text))) return -1;
  // a DIRECT <div class="cli"> child (the form holds GroupBoxes with their own: those are not the form's)
  //AI(W906-HTDESIGNER) 20260930: the first cli anywhere inside was taken -- a paste / a move into the form landed in a GroupBox
  const cliRe = /^<div\b[^>]*\bclass\s*=\s*["'][^"']*\bcli\b[^"']*["']/;
  const cli = childrenOf(text, { tag, range: r }).find(c => c.name === 'div' && cliRe.test(c.text));
  if (cli) return text.lastIndexOf('</', cli.end - 1);
  return text.lastIndexOf('</', r[1] - 1);
}

/**
 * The text with [start, end) removed, and the line break that came with it:
 *   alone on its line          -> the whole line goes;
 *   starts a line, more after  -> the break (and indentation) before it goes.
 * So a pasted block (inserted as "\n" + indent + block) deleted again leaves the
 * page byte-identical. { text, range:[s,e] } -- the range actually removed.
 */
function removeRange(text, start, end) {
  let s = start, e = end;
  const ls = text.lastIndexOf('\n', s - 1) + 1;
  let le = text.indexOf('\n', e);
  if (le < 0) le = text.length;
  const blankBefore = !text.slice(ls, s).trim();
  const blankAfter = !text.slice(e, le).replace(/\r$/, '').trim();
  if (blankBefore && blankAfter) { s = ls; e = le < text.length ? le + 1 : le; }
  else if (blankBefore && ls > 0) { s = ls - 1; if (s > 0 && text[s - 1] === '\r') s--; }
  return { text: text.slice(0, s) + text.slice(e), range: [s, e] };
}

/**
 * The element that directly contains offset `at` (a start tag's '<'): { tag, range },
 * or null at the top. One pass with a stack of the open elements.
 */
function parentOf(text, at) {
  const stack = openStackAt(text, at);
  if (!stack) return null;
  const top = stack[stack.length - 1];
  if (!top) return null;
  const range = elementRange(text, top);
  return range ? { tag: top, range } : null;
}

/** The elements open at offset `at`, outermost first: [{ name, start, end, text }] (its start tags), or null. */
function openStackAt(text, at) {
  const stack = [];
  let i = 0;
  const n = text.length;
  while (i < n && i < at) {
    const lt = text.indexOf('<', i);
    if (lt < 0 || lt >= at) break;
    if (text.startsWith('<!--', lt)) { const e = text.indexOf('-->', lt); if (e < 0) return null; i = e + 3; continue; }
    const e = he.tagEnd(text, lt);
    if (e < 0) return null;
    const t = text.slice(lt, e + 1);
    const close = /^<\/([A-Za-z][\w-]*)/.exec(t);
    if (close) {
      const nm = close[1].toLowerCase();
      // back to the matching open element (tolerates an unclosed <p> and the like)
      for (let k = stack.length - 1; k >= 0; k--) if (stack[k].name === nm) { stack.length = k; break; }
      i = e + 1;
      continue;
    }
    const open = /^<([A-Za-z][\w-]*)/.exec(t);
    if (open) {
      const nm = open[1].toLowerCase();
      if (nm === 'script' || nm === 'style') {
        const endTag = text.toLowerCase().indexOf('</' + nm, e + 1);
        const ee = endTag < 0 ? -1 : he.tagEnd(text, endTag);
        i = ee < 0 ? n : ee + 1;
        continue;
      }
      if (!he.VOID.has(nm) && !/\/>$/.test(t)) stack.push({ name: nm, start: lt, end: e + 1, text: t });
    }
    i = e + 1;
  }
  return stack;
}

/** The element children of a parent ({ tag, range }), in order: [{ start, end, name, text }]. */
function childrenOf(text, parent) {
  const out = [];
  let i = parent.tag.end;
  const stop = text.lastIndexOf('</', parent.range[1] - 1);
  while (i < stop) {
    const lt = text.indexOf('<', i);
    if (lt < 0 || lt >= stop) break;
    if (text.startsWith('<!--', lt)) { const e = text.indexOf('-->', lt); i = e < 0 ? stop : e + 3; continue; }
    const e = he.tagEnd(text, lt);
    if (e < 0) break;
    const t = text.slice(lt, e + 1);
    if (/^<\//.test(t)) { i = e + 1; continue; }
    const r = elementRange(text, { start: lt, end: e + 1, text: t });
    if (!r) break;
    out.push({ start: r[0], end: r[1], name: nameOf(t), text: t });
    i = r[1];
  }
  return out;
}

/**
 * WPF Order (Bring to Front / Forward / Send Backward / to Back) = where the element
 * stands among its siblings: later in the source is drawn on top. A fieldset's
 * <legend> and a panel's caption <span class="pnlCap"> stay first.
 *   how: 'front' | 'forward' | 'backward' | 'back'
 * Returns { parts: [{ range, repl }] } (one WorkspaceEdit), { same: true } when it is
 * already there, or { error }.
 */
function reorder(text, unit, how) {
  const parent = parentOf(text, unit.start);
  if (!parent) return { error: '找不到它的上層元素' };
  const kids = childrenOf(text, parent);
  const idx = kids.findIndex(k => k.start === unit.start && k.end === unit.end);
  if (idx < 0) return { error: '在上層元素裡找不到它（原始碼結構不完整）' };
  let fixed = 0;
  while (fixed < kids.length && (kids[fixed].name === 'legend' || /\bclass\s*=\s*["'][^"']*\bpnlCap\b/.test(kids[fixed].text))) fixed++;
  if (idx < fixed) return { error: '標題元素不能移動' };
  const last = kids.length - 1;
  const to = how === 'front' ? last : how === 'forward' ? Math.min(last, idx + 1) : how === 'backward' ? Math.max(fixed, idx - 1) : fixed;
  if (to === idx) return { same: true };
  const html = text.slice(unit.start, unit.end);
  const cut = removeRange(text, unit.start, unit.end).range;
  const moved = text.slice(cut[0], cut[1]);
  const wholeLine = moved.endsWith('\n') && moved.trim() === html.trim();
  const tgt = kids[to];
  const tLs = text.lastIndexOf('\n', tgt.start - 1) + 1;
  let tLe = text.indexOf('\n', tgt.end);
  if (tLe < 0) tLe = text.length;
  const tgtOwnLine = !text.slice(tLs, tgt.start).trim() && !text.slice(tgt.end, tLe).replace(/\r$/, '').trim() && tLe < text.length;
  let ins;
  if (wholeLine && tgtOwnLine) ins = { at: to > idx ? tLe + 1 : tLs, repl: moved };        // line with line
  else ins = { at: to > idx ? tgt.end : tgt.start, repl: html };
  const del = wholeLine && tgtOwnLine ? { range: cut, repl: '' } : { range: [unit.start, unit.end], repl: '' };
  return { parts: [del, { range: [ins.at, ins.at], repl: ins.repl }], from: idx, to, of: kids.length };
}

/**
 * 改名稱 (WPF's Name / the BCB6 Object Inspector's Name) of one component, on the whole page:
 * its id and the generated title="name : TClass…" (one edit of its start tag), every
 * <label for="old"> -- and, as BCB6 does, its caption when the caption still is the old
 * name (a new Label1 shows "Label1"). -> [{ range: [start, end], repl }] or null (not found).
 */
function renameEdits(text, oldId, newId, opts) {
  const esc = s => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const m = new RegExp('<[A-Za-z][\\w-]*\\b[^>]*\\sid\\s*=\\s*(["\'])' + esc(oldId) + '\\1[^>]*>').exec(text);
  if (!m) return null;
  const start = m.index, tagEnd = m.index + m[0].length;
  const tag = m[0]
    .replace(new RegExp('(\\sid\\s*=\\s*["\'])' + esc(oldId) + '(["\'])'), '$1' + newId + '$2')
    .replace(new RegExp('(\\stitle\\s*=\\s*["\'])' + esc(oldId) + '(\\s*:)'), '$1' + newId + '$2');
  const edits = [{ range: [start, tagEnd], repl: tag }];
  if (opts && opts.caption) {
    const r = elementRange(text, { start, end: tagEnd, text: m[0] });
    if (r) {
      const inner = text.slice(tagEnd, r[1]);
      const k = inner.indexOf('>' + oldId + '<');
      // the element's own text right after its start tag, or its first text before any child
      // with a name of its own (a legend, a panel's caption span) -- never a child's caption
      const own = k >= 0 && !/\sid\s*=/.test(inner.slice(0, k));
      const at = inner.slice(0, oldId.length + 1) === oldId + '<' ? 0 : own ? k + 1 : -1;
      if (at >= 0) edits.push({ range: [tagEnd + at, tagEnd + at + oldId.length], repl: newId });
    }
  }
  const fre = new RegExp('(\\sfor\\s*=\\s*["\'])' + esc(oldId) + '(["\'])', 'g');
  let f;
  while ((f = fre.exec(text))) {
    const s = f.index + f[1].length;
    if (s >= start && s < tagEnd) continue;
    edits.push({ range: [s, s + oldId.length], repl: newId });
  }
  return edits.sort((a, b) => a.range[0] - b.range[0]);
}

module.exports = { nameOf, elementRange, unitOf, idsIn, freshName, renameIds, renameEdits, offsetBlock, firstPx, indentAt, insideEnd, removeRange, parentOf, openStackAt, childrenOf, reorder };
