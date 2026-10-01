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

const unesc = t => String(t).replace(/<[^>]*>/g, '').replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&quot;/g, '"').replace(/&#39;/g, "'").replace(/&nbsp;/g, ' ').replace(/&amp;/g, '&');

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
  const re = h.kind === 'select' ? /<option\b([^>]*)>([\s\S]*?)<\/option>/gi : /<label\b[^>]*class\s*=\s*["'][^"']*\brgi\b[^"']*["'][^>]*>\s*<input\b([^>]*)>([\s\S]*?)<\/label>/gi;
  let m;
  while ((m = re.exec(body))) {
    if (h.kind === 'select' ? /\sselected\b/i.test(' ' + m[1]) : /\schecked\b/i.test(' ' + m[1])) selected = items.length;
    items.push(unesc(m[2]).trim());
  }
  return { kind: h.kind, items, selected };
}

/** The new inner markup for `items` (the one selected now kept by its text, a radio group's first otherwise). */
function itemsEdit(text, id, items) {
  const h = hostOf(text, id);
  if (!h) return { error: '「' + id + '」不是下拉選單（select）、單選群組（RadioGroup）或多行輸入框（Memo），沒有項目可以改' };
  const list = (Array.isArray(items) ? items : []).map(x => String(x == null ? '' : x).replace(/[\r\n]+/g, ' '));
  const now = itemsOf(text, id) || { items: [], selected: -1 };
  const keepText = now.selected >= 0 ? now.items[now.selected] : null;
  let sel = keepText != null ? list.indexOf(keepText) : -1;
  let repl;
  if (h.kind === 'memo') repl = he.escText(list.join('\n'));
  else if (h.kind === 'select') repl = list.map((t, i) => '<option' + (i === sel ? ' selected' : '') + '>' + he.escText(t) + '</option>').join('');
  else {
    if (sel < 0 && list.length) sel = 0;
    const nameM = /<input\b[^>]*\sname\s*=\s*["']([^"']+)["']/i.exec(text.slice(h.inner[0], h.inner[1]));
    const nm = nameM ? nameM[1] : 'rg_' + id;
    repl = list.map((t, i) => '<label class="rgi"><input type="radio" name="' + he.escAttr(nm) + '"' + (i === sel ? ' checked' : ' ') + '>' + he.escText(t) + '</label>').join('');
  }
  return { range: [h.inner[0], h.inner[1]], repl, kind: h.kind, selected: sel };
}

module.exports = { itemsOf, itemsEdit };
