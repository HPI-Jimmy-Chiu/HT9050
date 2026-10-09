/* AI(W906-HTDESIGNER) 20261001 (0.157, the WPF gap list G5: the Items collection editor / the String Collection Editor):
 * a list control's items written into the page as the generator writes them --
 *   TComboBox / TListBox  <select ...><option>a</option><option>b</option></select>  (the selected one kept by its text)
 *   TRadioGroup           <fieldset class="gbx rg" ...><legend>..</legend><div class="cli" ...>
 *                           <label class="rgi"><input type="radio" name="rg_ID" checked>a</label>...</div></fieldset>
 *                         (ItemIndex kept: the checked one by its text, else the first)
 *   TMemo                 <textarea ...>line 1\nline 2</textarea>  (Lines)
 *   TStringGrid           <div class="sgd" ...><table><tr><th>a</th>..</tr><tr><td>b</td>..</tr></table></div>  (Cells: 1008,
 *                         feature gap #4 -- one row a line, the cells parted by a Tab; the first row the heading <th>)
 * itemsOf(text, id) -> { kind, items } | null; itemsEdit(text, id, items) -> { range, repl, kind } | { error }. Pure. */
'use strict';
const he = require('./htmledit');
const hb = require('./htmlblock');

// (1006 audit: every entity, numeric ones too -- "Don&#x27;t" used to come back as "Don&amp;#x27;t" -- and a no-break
//  space stays one)
const unesc = t => he.decodeEnt(String(t).replace(/<[^>]*>/g, ''));
// (an item's text: trimmed only when the markup broke it over lines -- "Af1 " keeps its space)
const itemText = raw => { const v = unesc(raw); return /[\r\n]/.test(v) ? v.trim() : v; };

/** The element, its inner range and its kind ('select' | 'radio' | 'memo'), or null. */
function hostOf(text, id) {
  const tag = he.startTagOf(text, id);
  if (!tag) return null;
  const r = hb.elementRange(text, tag);
  if (!r) return null;
  const nm = (/^<([a-z]+)/i.exec(tag.text) || [])[1];
  const name = nm ? nm.toLowerCase() : '';
  const endTag = text.lastIndexOf('</', r[1] - 1);
  if (name === 'select') return { kind: 'select', tag, inner: [tag.end, endTag], all: r };
  if (name === 'textarea') return { kind: 'memo', tag, inner: [tag.end, endTag], all: r };
  // (1009 second review (props #4): a TListBox as the generator and the toolbox write it -- <div class="lbx"> with one
  //  <div> a row; its Items could not be edited at all)
  if (name === 'div' && /\bclass\s*=\s*["'][^"']*\blbx\b/.test(tag.text)) return { kind: 'lbx', tag, inner: [tag.end, endTag], all: r };
  if (name === 'div' && /\bclass\s*=\s*["'][^"']*\bsgd\b/.test(tag.text)) {
    // (the cells' box: the <table> inside -- its <tbody> when it has one)
    const low = text.toLowerCase();
    let box = null;
    for (const nm2 of ['<table', '<tbody']) {
      const from = box ? box[0] : tag.end, to = box ? box[1] : r[1];
      const i = low.indexOf(nm2, from);
      if (i < 0 || i >= to) { if (!box) return null; break; }
      const e = he.tagEnd(text, i);
      if (e < 0) return null;
      const cr = hb.elementRange(text, { start: i, end: e + 1, text: text.slice(i, e + 1) });
      if (!cr) return null;
      box = [e + 1, text.lastIndexOf('</', cr[1] - 1)];
    }
    return { kind: 'grid', tag, inner: box, all: r };
  }
  if (name === 'fieldset' && /\bclass\s*=\s*["'][^"']*\brg\b/.test(tag.text)) {
    // the items' box: the first <div class="cli"> inside
    const low = text.toLowerCase();
    let i = tag.end;
    for (;;) {
      i = low.indexOf('<div', i);
      if (i < 0 || i >= r[1]) return null;
      const e = he.tagEnd(text, i);
      if (e < 0) return null;
      const t = text.slice(i, e + 1);
      if (/\bclass\s*=\s*["'][^"']*\bcli\b/.test(t)) {
        const cr = hb.elementRange(text, { start: i, end: e + 1, text: t });
        if (!cr) return null;
        return { kind: 'radio', tag, inner: [e + 1, text.lastIndexOf('</', cr[1] - 1)], all: r, cliTag: { start: i, end: e + 1, text: t } };
      }
      i = e + 1;
    }
  }
  return null;
}

/** The items the page has now: { kind, items, selected } (selected: index, -1 = none) or null. */
function itemsOf(text, id) {
  const h = hostOf(text, id);
  if (!h) return null;
  const body = text.slice(h.inner[0], h.inner[1]);
  if (h.kind === 'memo') {
    // (1009 review (toolbox #10): the browser drops ONE line break right after <textarea> -- read as it shows)
    const t = unesc(body).replace(/^\r?\n/, '');
    return { kind: 'memo', items: t === '' ? [] : t.replace(/\r\n/g, '\n').split('\n'), selected: -1 };
  }
  if (h.kind === 'grid') return { kind: 'grid', items: rowsOf(body).map(rw => rw.cells.map(c => cellText(body.slice(c.ts, c.te))).join('\t')), selected: -1 };
  if (h.kind === 'lbx') return { kind: 'lbx', items: lbxRows(body).map(x => { const v = itemText(body.slice(x.ts, x.te)); return v === '\u00a0' ? '' : v; }), selected: -1 };
  const items = [];
  let selected = -1;
  for (const u of unitsOf(h.kind, body)) {
    if (u.on) selected = items.length;
    items.push(itemText(body.slice(u.ts, u.te)));
  }
  return { kind: h.kind, items, selected };
}

/**
 * 1006 audit: the items as they are written -- [{ s, e, os, oe, ts, te, on }] in `body`: the whole item, the start tag
 * that carries selected / checked (the <option>, a radio's <input>), its text. Comments and whitespace between items are
 * not items (they stay where they are).
 */
function unitsOf(kind, body) {
  const out = [];
  const re = kind === 'select' ? /<option\b/gi : /<label\b[^>]*\bclass\s*=\s*["'][^"']*\brgi\b[^"']*["'][^>]*>/gi;
  let m;
  while ((m = re.exec(body))) {
    const s0 = m.index;
    if (kind === 'select') {
      const oe = he.tagEnd(body, s0);
      if (oe < 0) break;
      const ce = he.findCI(body, '</option', oe + 1);
      if (ce < 0) break;
      const ee = he.tagEnd(body, ce);
      const os = s0, ot = body.slice(os, oe + 1);
      out.push({ s: s0, e: ee + 1, os, oe: oe + 1, ts: oe + 1, te: ce, on: he.attrTokens(ot).some(x => x.name === 'selected') });
      re.lastIndex = ee + 1;
    } else {
      const le = s0 + m[0].length;
      const im = /^\s*<input\b/i.exec(body.slice(le));
      if (!im) continue;
      const is = le + im[0].length - '<input'.length;
      const ie = he.tagEnd(body, is);
      if (ie < 0) break;
      const ce = he.findCI(body, '</label', ie + 1);
      if (ce < 0) break;
      const ee = he.tagEnd(body, ce);
      const it = body.slice(is, ie + 1);
      out.push({ s: s0, e: ee + 1, os: is, oe: ie + 1, ts: ie + 1, te: ce, on: he.attrTokens(it).some(x => x.name === 'checked') });
      re.lastIndex = ee + 1;
    }
  }
  return out;
}

/** a list box's rows: each <div …>text</div> right inside it -> [{ s, e, ts, te }] */
function lbxRows(body) {
  const out = [];
  const re = /<div\b/gi;
  let m;
  while ((m = re.exec(body))) {
    const oe = he.tagEnd(body, m.index);
    if (oe < 0) break;
    const ce = he.findCI(body, '</div', oe + 1);
    if (ce < 0) break;
    const ee = he.tagEnd(body, ce);
    out.push({ s: m.index, e: ee + 1, ts: oe + 1, te: ce });
    re.lastIndex = ee + 1;
  }
  return out;
}

// (a cell's text: no Tab or line break in it -- they part the cells and rows in the editor)
const cellText = raw => itemText(raw).replace(/[\t\r\n]+/g, ' ');

/**
 * 1008 (feature gap #4): a grid's rows as they are written -- [{ s, e, os, oe, ce, cells: [{ s, e, ts, te, th }] }] in
 * `body`: the whole <tr>, its start tag, where its content ends; each <th> / <td> and its text.
 */
function rowsOf(body) {
  const out = [];
  const re = /<tr\b/gi;
  let m;
  while ((m = re.exec(body))) {
    const os = m.index, oe = he.tagEnd(body, os);
    if (oe < 0) break;
    let ce = he.findCI(body, '</tr', oe + 1);
    const nx = he.findCI(body, '<tr', oe + 1);
    // (a <tr> left open ends where the next starts)
    const open = ce < 0 || (nx >= 0 && nx < ce);
    if (open) ce = nx >= 0 ? nx : body.length;
    const e = open ? ce : he.tagEnd(body, ce) + 1;
    const cells = [];
    const cre = /<t([hd])\b/gi;
    const inner = body.slice(oe + 1, ce);
    let c;
    while ((c = cre.exec(inner))) {
      const cs = oe + 1 + c.index, ct = he.tagEnd(body, cs);
      if (ct < 0 || ct >= ce) break;
      let cc = body.slice(ct + 1, ce).search(/<\/t[hd]\b|<t[hd]\b/i);
      cc = cc < 0 ? ce : ct + 1 + cc;
      const closed = /^<\/t[hd]/i.test(body.slice(cc, cc + 4));
      const cend = closed ? he.tagEnd(body, cc) + 1 : cc;
      cells.push({ s: cs, e: cend, ts: ct + 1, te: cc, th: c[1].toLowerCase() === 'h', os: cs, oe: ct + 1 });
      cre.lastIndex = cend - (oe + 1);
    }
    out.push({ s: os, e, os, oe: oe + 1, ce, cells });
    re.lastIndex = e;
  }
  return out;
}

/** 1008: a grid's new inner markup -- each row kept as written when its cells are the same, a changed cell's text alone
 *  rewritten (its tag, colspan and all, kept); new cells / rows like the ones before them (<th> in the first row). */
function gridEdit(body, list) {
  const rows = rowsOf(body);
  const want = list.map(l => String(l).split('\t'));
  const sep = rows.length > 1 && !/\S/.test(body.slice(rows[0].e, rows[1].s)) ? body.slice(rows[0].e, rows[1].s) : '';
  const freshCell = (t, th) => '<t' + (th ? 'h' : 'd') + '>' + he.escText(t) + '</t' + (th ? 'h' : 'd') + '>';
  const freshRow = (cells, first) => '<tr>' + cells.map(t => freshCell(t, first)).join('') + '</tr>';
  if (!rows.length) return want.map((c, i) => freshRow(c, i === 0)).join('');
  let out = '', pos = 0;
  rows.forEach((rw, i) => {
    if (i >= want.length) { const gap = body.slice(pos, rw.s); out += /\S/.test(gap) ? gap.replace(/\s+$/, '') : ''; pos = rw.e; return; }
    const w = want[i];
    let r = body.slice(pos, rw.oe), p = rw.oe;
    rw.cells.forEach((c, k) => {
      if (k >= w.length) { r += body.slice(p, c.s).replace(/\s+$/, ''); p = c.e; return; }
      const cur = body.slice(c.ts, c.te);
      // (the cells of a row now more or fewer: a colspan -- the generator's one heading over every column -- no longer fits)
      const tg = body.slice(c.os, c.oe);
      r += body.slice(p, c.os) + (w.length !== rw.cells.length && /\bcolspan\b/i.test(tg) ? he.setAttr(tg, 'colspan', null) : tg) + (cellText(cur) === w[k] ? cur : he.escText(w[k]));
      p = c.te;
      if (k === rw.cells.length - 1) {
        r += body.slice(p, c.e); p = c.e;
        for (let q = rw.cells.length; q < w.length; q++) r += freshCell(w[q], c.th);
      }
    });
    if (!rw.cells.length) r += w.map(t => freshCell(t, i === 0)).join('');
    out += r + body.slice(p, rw.e);
    pos = rw.e;
    if (i === Math.min(rows.length, want.length) - 1) for (let q = rows.length; q < want.length; q++) out += sep + freshRow(want[q], false);
  });
  return out + body.slice(pos);
}

/** The new inner markup for `items` (the one selected now kept by its text, a radio group's first otherwise). */
function itemsEdit(text, id, items) {
  const h = hostOf(text, id);
  if (!h) return { error: '「' + id + '」不是下拉選單（select）、清單（ListBox）、單選群組（RadioGroup）、多行輸入框（Memo）或表格（StringGrid），沒有項目可以改' };
  if (h.kind === 'lbx') {
    const rowsIn = (Array.isArray(items) ? items : []).map(x => String(x == null ? '' : x).replace(/[\r\n]+/g, ' '));
    const body = text.slice(h.inner[0], h.inner[1]);
    const now = itemsOf(text, id).items;
    const same = rowsIn.length === now.length && rowsIn.every((x, i) => x === now[i]);
    // (an empty row keeps its height: &nbsp;)
    return { range: [h.inner[0], h.inner[1]], repl: same ? body : rowsIn.map(t => '<div>' + (t === '' ? '&nbsp;' : he.escText(t)) + '</div>').join(''), kind: 'lbx', selected: -1, same: same || undefined };
  }
  if (h.kind === 'grid') {
    const rowsIn = (Array.isArray(items) ? items : []).map(x => String(x == null ? '' : x).replace(/[\r\n]+/g, ' '));
    const body = text.slice(h.inner[0], h.inner[1]);
    const now = itemsOf(text, id).items;
    const same = rowsIn.length === now.length && rowsIn.every((x, i) => x === now[i]);
    return { range: [h.inner[0], h.inner[1]], repl: same ? body : gridEdit(body, rowsIn), kind: 'grid', selected: -1, same: same || undefined };
  }
  const list = (Array.isArray(items) ? items : []).map(x => String(x == null ? '' : x).replace(/[\r\n]+/g, ' '));
  const now = itemsOf(text, id) || { items: [], selected: -1 };
  const keepText = now.selected >= 0 ? now.items[now.selected] : null;
  // (1009 review (toolbox #12): the same text twice -- the same occurrence of it (the 2nd "A" stays the selected one; it
  //  moved to the first)
  let sel = -1;
  if (keepText != null) {
    const nth = now.items.slice(0, now.selected).filter(x => x === keepText).length;
    for (let i = 0, k = 0; i < list.length; i++) if (list[i] === keepText) { if (k === nth) { sel = i; break; } k++; }
    if (sel < 0) sel = list.lastIndexOf(keepText);
  }
  // (the selected one renamed: it stays selected -- the same place, as an ItemIndex)
  if (sel < 0 && now.selected >= 0 && now.selected < list.length && h.kind !== 'memo') sel = now.selected;
  const body = text.slice(h.inner[0], h.inner[1]);
  if (h.kind !== 'memo' && h.kind !== 'select' && sel < 0 && list.length) sel = 0;
  // (1006 audit: the same items = the same text -- it used to rewrite them all, losing <option value>, comments, titles)
  if (list.length === now.items.length && list.every((x, i) => x === now.items[i]) && sel === now.selected) return { range: [h.inner[0], h.inner[1]], repl: body, kind: h.kind, selected: sel, same: true };
  let repl;
  // (1009 review (toolbox #10): a first line that is empty needs the break the browser drops written once more)
  if (h.kind === 'memo') { repl = he.escText(list.join('\n')); if (/^\r?\n/.test(repl)) repl = '\n' + repl; }
  else {
    const nameM = /<input\b[^>]*\sname\s*=\s*["']([^"']+)["']/i.exec(body);
    const nm = nameM ? nameM[1] : 'rg_' + id;
    const fresh = (t, on) => h.kind === 'select' ? '<option' + (on ? ' selected' : '') + '>' + he.escText(t) + '</option>'
      : '<label class="rgi"><input type="radio" name="' + he.escAttr(nm) + '"' + (on ? ' checked' : ' ') + '>' + he.escText(t) + '</label>';
    const units = unitsOf(h.kind, body);
    if (!units.length) repl = list.map((t, i) => fresh(t, i === sel)).join('');
    else {
      // each kept item edited where it is (its own tag's attributes kept, only the selected / checked mark and the text
      // changed when they change); the ones past the new end removed; new ones after the last, spaced like the others
      const flag = h.kind === 'select' ? 'selected' : 'checked';
      let out = '', pos = 0;
      const sep = units.length > 1 && !/\S/.test(body.slice(units[0].e, units[1].s)) ? body.slice(units[0].e, units[1].s) : '';
      // (1008 review: an item moved / sorted / removed takes ITS OWN tag along -- <option value="1">, disabled, data-*. The
      //  tag used to stay at its place and only the words moved: sorting s-demoType (GPIB=1 / RS232=2 / TTL=0 / TCP=3)
      //  gave every option another one's value. Each place: the unused old item of the same words; else the one that was
      //  there (renamed in place); else a new one)
      const textOf = u => itemText(body.slice(u.ts, u.te));
      const src = new Array(list.length).fill(-1), used = new Set();
      list.forEach((t, i) => { if (i < units.length && textOf(units[i]) === t) { src[i] = i; used.add(i); } });
      list.forEach((t, i) => { if (src[i] >= 0) return; const j = units.findIndex((u, k) => !used.has(k) && textOf(u) === t); if (j >= 0) { src[i] = j; used.add(j); } });
      list.forEach((t, i) => { if (src[i] < 0 && i < units.length && !used.has(i) && !list.includes(textOf(units[i]))) { src[i] = i; used.add(i); } });
      units.forEach((u, i) => {
        // (a removed item takes the blank space before it along -- a comment there stays)
        if (i >= list.length) { const gap = body.slice(pos, u.s); out += /\S/.test(gap) ? gap.replace(/\s+$/, '') : ''; pos = u.e; return; }
        const j = src[i];
        if (j < 0) { out += body.slice(pos, u.s) + fresh(list[i], i === sel); pos = u.e; }
        else {
          const v = units[j];
          const tag = he.setAttr(body.slice(v.os, v.oe), flag, i === sel ? true : null);
          const curT = body.slice(v.ts, v.te);
          const txt = itemText(curT) === list[i] ? curT : he.escText(list[i]);
          out += body.slice(pos, u.s) + body.slice(v.s, v.os) + tag + txt + body.slice(v.te, v.e);
          pos = u.e;
        }
        if (i === Math.min(units.length, list.length) - 1 && list.length > units.length) {
          for (let k = units.length; k < list.length; k++) out += sep + fresh(list[k], k === sel);
        }
      });
      repl = out + body.slice(pos);
    }
  }
  // (1008 review: a radio group's rows follow its number of items -- the generator writes rows = ceil(items / Columns);
  //  with the old count left, a fourth item of a one-column group went into a second column, and removed ones left
  //  empty rows. Columns worked out from the old count and rows)
  if (h.kind === 'radio' && h.cliTag && list.length && list.length !== now.items.length) {
    const rm = /(grid-template-rows\s*:\s*repeat\(\s*)(\d+)(\s*,)/i.exec(h.cliTag.text);
    if (rm) {
      const oldRows = Math.max(1, +rm[2]), cols = Math.max(1, Math.ceil(now.items.length / oldRows));
      const rows = Math.max(1, Math.ceil(list.length / cols));
      if (rows !== oldRows) {
        const nt = h.cliTag.text.slice(0, rm.index) + rm[1] + rows + rm[3] + h.cliTag.text.slice(rm.index + rm[0].length);
        return { range: [h.cliTag.start, h.inner[1]], repl: nt + text.slice(h.cliTag.end, h.inner[0]) + repl, kind: h.kind, selected: sel };
      }
    }
  }
  return { range: [h.inner[0], h.inner[1]], repl, kind: h.kind, selected: sel };
}

module.exports = { itemsOf, itemsEdit };
