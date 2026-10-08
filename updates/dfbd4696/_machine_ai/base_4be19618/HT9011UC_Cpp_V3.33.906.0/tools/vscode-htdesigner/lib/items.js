/* AI(W906-HTDESIGNER) 20261001 (0.157, the WPF gap list G5: the Items collection editor / the String Collection Editor):
 * a list control's items written into the page as the generator writes them --
 *   TComboBox / TListBox  <select ...><option>a</option><option>b</option></select>  (the selected one kept by its text)
 *   TRadioGroup           <fieldset class="gbx rg" ...><legend>..</legend><div class="cli" ...>
 *                           <label class="rgi"><input type="radio" name="rg_ID" checked>a</label>...</div></fieldset>
 *                         (ItemIndex kept: the checked one by its text, else the first)
 *   TMemo                 <textarea ...>line 1\nline 2</textarea>  (Lines)
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
        return { kind: 'radio', tag, inner: [e + 1, text.lastIndexOf('</', cr[1] - 1)], all: r };
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
    const t = unesc(body);
    return { kind: 'memo', items: t === '' ? [] : t.replace(/\r\n/g, '\n').split('\n'), selected: -1 };
  }
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

/** The new inner markup for `items` (the one selected now kept by its text, a radio group's first otherwise). */
function itemsEdit(text, id, items) {
  const h = hostOf(text, id);
  if (!h) return { error: '「' + id + '」不是下拉選單（select）、單選群組（RadioGroup）或多行輸入框（Memo），沒有項目可以改' };
  const list = (Array.isArray(items) ? items : []).map(x => String(x == null ? '' : x).replace(/[\r\n]+/g, ' '));
  const now = itemsOf(text, id) || { items: [], selected: -1 };
  const keepText = now.selected >= 0 ? now.items[now.selected] : null;
  let sel = keepText != null ? list.indexOf(keepText) : -1;
  // (the selected one renamed: it stays selected -- the same place, as an ItemIndex)
  if (sel < 0 && now.selected >= 0 && now.selected < list.length && h.kind !== 'memo') sel = now.selected;
  const body = text.slice(h.inner[0], h.inner[1]);
  if (h.kind !== 'memo' && h.kind !== 'select' && sel < 0 && list.length) sel = 0;
  // (1006 audit: the same items = the same text -- it used to rewrite them all, losing <option value>, comments, titles)
  if (list.length === now.items.length && list.every((x, i) => x === now.items[i]) && sel === now.selected) return { range: [h.inner[0], h.inner[1]], repl: body, kind: h.kind, selected: sel, same: true };
  let repl;
  if (h.kind === 'memo') repl = he.escText(list.join('\n'));
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
      units.forEach((u, i) => {
        // (a removed item takes the blank space before it along -- a comment there stays)
        if (i >= list.length) { const gap = body.slice(pos, u.s); out += /\S/.test(gap) ? gap.replace(/\s+$/, '') : ''; pos = u.e; return; }
        const tag = he.setAttr(body.slice(u.os, u.oe), flag, i === sel ? true : null);
        const curT = body.slice(u.ts, u.te);
        const txt = itemText(curT) === list[i] ? curT : he.escText(list[i]);
        out += body.slice(pos, u.s) + body.slice(u.s, u.os) + tag + txt + body.slice(u.te, u.e);
        pos = u.e;
        if (i === Math.min(units.length, list.length) - 1 && list.length > units.length) {
          for (let k = units.length; k < list.length; k++) out += sep + fresh(list[k], k === sel);
        }
      });
      repl = out + body.slice(pos);
    }
  }
  return { range: [h.inner[0], h.inner[1]], repl, kind: h.kind, selected: sel };
}

module.exports = { itemsOf, itemsEdit };
