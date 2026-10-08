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
/*
 * 1007 audit (toolbox, performance): one structural command asks the same ranges / parents again and again -- the form's
 * range (the whole page) 22-38 ms each, parentOf scanning from offset 0 each time; drop / group / add-next-to on
 * HW.IoSetView took up to 0.7 s. The answers are kept for the text they were worked out on (the same text = the same
 * answers; another text = all forgotten).
 */
let memoText = null, memoRange = new Map(), memoStack = new Map();
function memoFor(text) {
  if (text !== memoText) { memoText = text; memoRange = new Map(); memoStack = new Map(); }
}
function elementRange(text, tag) {
  memoFor(text);
  const key = tag.start + ':' + tag.end;
  if (memoRange.has(key)) { const r = memoRange.get(key); return r ? r.slice() : null; }
  const r = elementRange0(text, tag);
  memoRange.set(key, r ? r.slice() : null);
  return r;
}
function elementRange0(text, tag) {
  const nm = nameOf(tag.text);
  if (!nm) return null;
  if (he.VOID.has(nm) || /\/>$/.test(tag.text)) return [tag.start, tag.end];
  if (nm === 'script' || nm === 'style') {
    // its content is not markup ("a < b" in the code): straight to its end tag
    const endTag = he.findCI(text, '</' + nm, tag.end);
    const ee = endTag < 0 ? -1 : he.tagEnd(text, endTag);
    return ee < 0 ? null : [tag.start, ee + 1];
  }
  let i = tag.end;
  // (1006 audit: the open elements by name -- a close tag closes the nearest one of its name and any left open inside
  //  it, as a browser does; one <div> left unclosed inside used to make every ancestor's range null: "找不到它的上層元素")
  const stack = [];
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
      const cn = close[1].toLowerCase();
      const k = stack.lastIndexOf(cn);
      if (k >= 0) { stack.length = k; i = e + 1; continue; }
      if (cn === nm) return [tag.start, e + 1];
      // (a close tag of nothing open in here: a stray one -- or this element's parent ending it, unclosed: no range)
      if (!stack.length) return null;
      i = e + 1;
      continue;
    }
    const open = /^<([A-Za-z][\w-]*)/.exec(t);
    if (open) {
      const on = open[1].toLowerCase();
      if (on === 'script' || on === 'style') {
        const endTag = he.findCI(text, '</' + on, e + 1);
        if (endTag < 0) return null;
        const ee = he.tagEnd(text, endTag);
        i = ee < 0 ? n : ee + 1;
        continue;
      }
      if (!he.VOID.has(on) && !/\/>$/.test(t)) stack.push(on);
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

/**
 * 1007 audit (toolbox #1): the start tag right before `tag` is ITS wrapper -- an id-less position:absolute element
 * (<span> round an input, span.lled round a labeled LED) that holds no other named element. A GroupBox's
 * <div class="cli" style="position:absolute;inset:0"> before its first child was taken for one: Delete / Cut / drag of
 * that child took all its siblings along (974 components on 30+ pages).
 */
function wrapsOnly(text, tag) {
  const w = he.wrapperTagOf(text, tag.start);
  if (!w || /\sid\s*=/.test(w.text) || !/position\s*:\s*absolute/i.test(w.text)) return false;
  if (/\sclass\s*=\s*["'][^"']*\bcli\b/i.test(w.text)) return false;
  const wr = elementRange(text, w), er = elementRange(text, tag);
  if (!wr || !er) return false;
  // (every named element in the wrapper is this one or inside it)
  return idsIn(text.slice(wr[0], wr[1])).length === idsIn(text.slice(er[0], er[1])).length;
}

/**
 * 1007 audit (surface #5): the unit of a named component -- the element, its one-child wrapper (wrapsOnly), or the
 * id-less position:absolute <span> of a TLabeledEdit / labeled LED whose other child is the caption (.elab / .lledCap):
 * z-order moved the bare input, swapping it with its own EditLabel. -> unitOf's result or null
 */
function componentUnit(text, id) {
  const tag = he.startTagOf(text, id);
  if (!tag) return null;
  if (wrapsOnly(text, tag)) return unitOf(text, id, 'parent');
  const par = parentOf(text, tag.start);
  if (par && !/\sid\s*=/.test(par.tag.text) && /position\s*:\s*absolute/i.test(par.tag.text) && !/\sclass\s*=\s*["'][^"']*\bcli\b/i.test(par.tag.text)) {
    const kids = childrenOf(text, par);
    const er = elementRange(text, tag);
    const capOk = kids.length === 2 && kids.some(k => k.start === tag.start) && kids.some(k => /\bclass\s*=\s*["'][^"']*\b(elab|lledCap)\b/.test(k.text));
    if (capOk && er && idsIn(text.slice(par.range[0], par.range[1])).length === idsIn(text.slice(er[0], er[1])).length) {
      return { start: par.range[0], end: par.range[1], html: text.slice(par.range[0], par.range[1]), tag: par.tag };
    }
  }
  return unitOf(text, id, 'self');
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
function renameIds(html, used, opts) {
  const map = new Map();
  const keepFree = !!(opts && opts.keepFree);
  // (1006 audit, performance: the next free number per base name -- each id used to count up from _2 again)
  const nextK = new Map();
  // (1007 audit, toolbox #4: a TPageControl's sheet names -- tab title="ts : TTabSheet", pane title="ts" -- are no ids;
  //  a copy kept them, so the page had two sheets of one name and an add into the copy's sheet went into the original)
  const sheets = [];
  { const re = /\stitle\s*=\s*["']([^"'\s:]+)\s*:\s*TTabSheet\b/g; let m; while ((m = re.exec(html))) sheets.push(m[1]); }
  for (const id of idsIn(html).concat(sheets)) {
    if (map.has(id)) continue;
    // (1007 audit, toolbox #5: Cut + Paste -- a name no longer on the page is kept, as BCB6 / WPF do)
    // (1009 review: opts.keepIn = the names that count for that -- the page's own ids; the DFM's names are in `used` (a new
    //  name must not take them) and every DFM component's cut + paste became name_2, moved 8 px, cut off from its events)
    if (keepFree && !(opts.keepIn || used).has(id)) { used.add(id); map.set(id, id); continue; }
    const base = String(id).replace(/_\d+$/, '');
    let k = nextK.get(base) || 2, nu;
    while (used.has(nu = base + '_' + k)) k++;
    nextK.set(base, k + 1);
    used.add(nu);
    map.set(id, nu);
  }
  // (1006 audit, performance: ONE pass over the block -- three whole-block regex replaces per id took 25 s to copy
  //  HW.IoSetView's biggest panel) -- id="…", for="…", and title="name : TClass…"
  // (1007 audit, toolbox #4: and what points at a renamed one -- a radio group's name="rg_X" / name="X", list="X",
  //  data-*="X", a sheet pane's title="ts": the copy's radio buttons shared the original's group, a click on one cleared
  //  the other)
  const ref = v => (map.has(v) ? map.get(v) : /^rg_/.test(v) && map.has(v.slice(3)) ? 'rg_' + map.get(v.slice(3)) : null);
  const out = String(html).replace(/(\s(?:id|for)\s*=\s*["'])([^"']+)(["'])|(\stitle\s*=\s*["'])([^"'\s:]+)(\s*:|["'])|(\s(?:name|list|data-[\w-]+)\s*=\s*["'])([^"']+)(["'])/g,
    (m0, a1, a2, a3, b1, b2, b3, c1, c2, c3) => {
      if (a1 !== undefined) return map.has(a2) ? a1 + map.get(a2) + a3 : m0;
      if (b1 !== undefined) return map.has(b2) ? b1 + map.get(b2) + b3 : m0;
      const r = ref(c2);
      return r !== null ? c1 + r + c3 : m0;
    });
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
/**
 * 1009 review (design #8): where a component's removal starts when the comments right above it are about it -- each
 * <!-- ... --> directly before (only blanks between) that names one of `ids` as a word goes with it; Delete left the
 * generator's / an AI's note about a component that was no longer there. A comment about something else stays.
 */
function leadNotes(text, start, ids) {
  const words = (ids || []).filter(Boolean).map(id => new RegExp('(^|[^A-Za-z0-9_])' + String(id).replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + '($|[^A-Za-z0-9_])'));
  if (!words.length) return start;
  let p = start;
  for (;;) {
    let w = p;
    while (w > 0 && /\s/.test(text[w - 1])) w--;
    if (w < 3 || text.slice(w - 3, w) !== '-->') break;
    const cs = text.lastIndexOf('<!--', w - 3);
    if (cs < 0) break;
    const body = text.slice(cs + 4, w - 3);
    if (!words.some(re => re.test(body))) break;
    // (on its own lines only: a comment sharing its line with other markup is that markup's)
    const ls = text.lastIndexOf('\n', cs - 1) + 1;
    if (text.slice(ls, cs).trim()) break;
    p = cs;
  }
  return p;
}

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
  memoFor(text);
  if (memoStack.has(at)) { const st = memoStack.get(at); return st ? st.slice() : null; }
  const st = openStackAt0(text, at);
  memoStack.set(at, st ? st.slice() : null);
  return st;
}
function openStackAt0(text, at) {
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
        const endTag = he.findCI(text, '</' + nm, e + 1);
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
  // (1006 audit: the comments right before an element -- its <!-- AI(…) --> note -- belong to it and move with it)
  const leadOf = i => {
    const floor = i > 0 ? kids[i - 1].end : parent.tag.end;
    let p = kids[i].start;
    for (;;) {
      let w = p;
      while (w > floor && /\s/.test(text[w - 1])) w--;
      if (w - 3 < floor || text.slice(w - 3, w) !== '-->') break;
      const cs = text.lastIndexOf('<!--', w - 3);
      if (cs < floor) break;
      p = cs;
    }
    return p;
  };
  // (1006 audit: one step = the two swap their exact places and whatever is between them stays -- text in a flow layout
  //  ("<select> Baud") used to jump to the other side; forward then backward is the page again, byte for byte)
  if (Math.abs(to - idx) === 1) {
    const a = Math.min(idx, to), b = Math.max(idx, to);
    const aS = leadOf(a), bS = leadOf(b);
    const A = text.slice(aS, kids[a].end), B = text.slice(bS, kids[b].end), mid = text.slice(kids[a].end, bS);
    return { parts: [{ range: [aS, kids[b].end], repl: B + mid + A }], from: idx, to, of: kids.length };
  }
  const uStart = leadOf(idx);
  const html = text.slice(uStart, unit.end);
  const cut = removeRange(text, uStart, unit.end).range;
  const moved = text.slice(cut[0], cut[1]);
  const wholeLine = moved.endsWith('\n') && moved.trim() === html.trim();
  const tgt = kids[to];
  const tLs = text.lastIndexOf('\n', tgt.start - 1) + 1;
  let tLe = text.indexOf('\n', tgt.end);
  if (tLe < 0) tLe = text.length;
  const tgtOwnLine = !text.slice(tLs, tgt.start).trim() && !text.slice(tgt.end, tLe).replace(/\r$/, '').trim() && tLe < text.length;
  let ins;
  if (wholeLine && tgtOwnLine) ins = { at: to > idx ? tLe + 1 : tLs, repl: moved };        // line with line
  else ins = { at: to > idx ? tgt.end : leadOf(to), repl: html };
  const del = wholeLine && tgtOwnLine ? { range: cut, repl: '' } : { range: [uStart, unit.end], repl: '' };
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
  // (1008 review: a radio group's own buttons, name="rg_OLD" / name="OLD" inside it, follow -- as renameIds does for a
  //  copy. Left, a new RadioGroup1 added later took the free name and both groups shared one name: a click on one
  //  cleared the other)
  const er = elementRange(text, { start, end: tagEnd, text: m[0] });
  if (er) {
    const nre = new RegExp('(\\sname\\s*=\\s*["\'])(rg_)?' + esc(oldId) + '(["\'])', 'g');
    const inner = text.slice(tagEnd, er[1]);
    let q;
    while ((q = nre.exec(inner))) {
      const s = tagEnd + q.index + q[1].length + (q[2] ? q[2].length : 0);
      edits.push({ range: [s, s + oldId.length], repl: newId });
    }
  }
  // (1009 review (toolbox #9): a labeled lamp -- its <span class="lled" title="OLD : TMyLabeledLedLane…｜Caption=OLD…"> and,
  //  as BCB6 does for a caption still showing the name, its .lledCap text: the frame kept the old name, and a second
  //  MyLabeledLedLane1 added later had the same one)
  const par = parentOf(text, start);
  if (par && par.tag && /\sclass\s*=\s*["'][^"']*\blled\b/.test(par.tag.text) && !/\sid\s*=/.test(par.tag.text)) {
    const pt = par.tag.text;
    const tre = new RegExp('(\\stitle\\s*=\\s*["\'])' + esc(oldId) + '(\\s*:)');
    const tm = tre.exec(pt);
    if (tm) { const s = par.tag.start + tm.index + tm[1].length; edits.push({ range: [s, s + oldId.length], repl: newId }); }
    if (opts && opts.caption) {
      const cm = new RegExp('Caption=' + esc(oldId) + '(?=[\\s｜"\'])').exec(pt);
      if (cm) { const s = par.tag.start + cm.index + 'Caption='.length; edits.push({ range: [s, s + oldId.length], repl: newId }); }
      const pr = par.range || elementRange(text, par.tag);
      if (pr) {
        const body = text.slice(par.tag.end, pr[1]);
        const lm = /(<span\b[^>]*\bclass\s*=\s*["'][^"']*\blledCap\b[^>]*>)([^<]*)</.exec(body);
        if (lm && lm[2] === oldId) { const s = par.tag.end + lm.index + lm[1].length; edits.push({ range: [s, s + oldId.length], repl: newId }); }
        // (the toolbox's caption label "lblOLD : TLabel" -- its name follows)
        const lt = lm ? new RegExp('(\\stitle\\s*=\\s*["\'])lbl' + esc(oldId) + '(\\s*:)').exec(lm[1]) : null;
        if (lt) { const s = par.tag.end + lm.index + lt.index + lt[1].length + 3; edits.push({ range: [s, s + oldId.length], repl: newId }); }
      }
    }
  }
  return edits.sort((a, b) => a.range[0] - b.range[0]);
}

module.exports = { leadNotes, nameOf, elementRange, unitOf, wrapsOnly, componentUnit, idsIn, freshName, renameIds, renameEdits, offsetBlock, firstPx, indentAt, insideEnd, removeRange, parentOf, openStackAt, childrenOf, reorder };
