'use strict';
// AI(W906-HTDESIGNER) 20261006 (EastSun: "有些元件是多分頁的 需要在元件點選右鍵新增分頁"): a TPageControl's sheets in the
// page source, as the generator writes them --
//   <div class="pcWrap" id="pgc…" title="pgc… : TPageControl">
//     <div class="tabs pcTabs"><div class="tab act" data-t="0" title="tsA : TTabSheet">Caption</div>…</div>
//     <div class="pcBody"><div class="pcPane" data-p="0" title="tsA" style="display:block;">…</div>…</div>
//   </div>
// -- a tab and its pane go together by data-t = data-p; the sheet's .dfm name is on the tab's title (the pane has no id).
// Add / delete a sheet, change a tab's caption: text edits [{ range, repl }] for one WorkspaceEdit (one undo).
// Plain Node (no vscode).
const he = require('./htmledit');
const hb = require('./htmlblock');

const hasClass = (tagText, c) => new RegExp('\\bclass\\s*=\\s*["\'][^"\']*\\b' + c + '\\b').test(tagText);
const attr = (tagText, a) => { const m = new RegExp('\\b' + a + '\\s*=\\s*"([^"]*)"').exec(tagText); return m ? m[1] : null; };
const escRe = s => String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
const escHtml = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
const unHtml = s => String(s).replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&quot;/g, '"').replace(/&#39;/g, "'").replace(/&amp;/g, '&');

/** the element whose start tag begins at `at`: { tag, range } (null: unbalanced) */
function elemAt(text, at) {
  const e = he.tagEnd(text, at);
  if (e < 0) return null;
  const tag = { start: at, end: e + 1, text: text.slice(at, e + 1) };
  const range = hb.elementRange(text, tag);
  return range ? { tag, range } : null;
}

/** where a sheet's tab starts (by its .dfm name), or -1 */
function tabStartOf(text, name) {
  const m = new RegExp('title\\s*=\\s*"' + escRe(name) + '\\s*:\\s*TTabSheet"').exec(text);
  return m ? text.lastIndexOf('<', m.index) : -1;
}

/**
 * The PageControl that `id` means: the PageControl itself, a sheet of it (its .dfm name), or a component on one of its
 * sheets (the nearest PageControl around it). { tag, range } or null.
 */
function wrapFor(text, id) {
  if (!id || id === '@form') return null;
  const st = he.startTagOf(text, id);
  if (st && hasClass(st.text, 'pcWrap')) return elemAt(text, st.start);
  const at = st ? st.start : tabStartOf(text, id);
  if (at < 0) return null;
  const stack = hb.openStackAt(text, at) || [];
  for (let k = stack.length - 1; k >= 0; k--) if (stack[k].name === 'div' && hasClass(stack[k].text, 'pcWrap')) return elemAt(text, stack[k].start);
  return null;
}

/** a PageControl's sheets: { id, tabs, body, list: [{ n, name, caption, act, tab: [s,e], tabTag, pane: [s,e]|null, paneTag }] } */
function sheetsOf(text, wrap) {
  const kids = hb.childrenOf(text, wrap);
  const tk = kids.find(c => c.name === 'div' && hasClass(c.text, 'pcTabs'));
  const bk = kids.find(c => c.name === 'div' && hasClass(c.text, 'pcBody'));
  if (!tk || !bk) return null;
  const tabs = { tag: { start: tk.start, end: tk.start + tk.text.length, text: tk.text }, range: [tk.start, tk.end] };
  const body = { tag: { start: bk.start, end: bk.start + bk.text.length, text: bk.text }, range: [bk.start, bk.end] };
  const panes = hb.childrenOf(text, body).filter(c => hasClass(c.text, 'pcPane'));
  const list = hb.childrenOf(text, tabs).filter(c => hasClass(c.text, 'tab')).map(c => {
    const n = attr(c.text, 'data-t');
    const nm = /^\s*([A-Za-z_]\w*)\s*:\s*TTabSheet/.exec(attr(c.text, 'title') || '');
    const p = panes.find(x => attr(x.text, 'data-p') === n);
    const inner = text.slice(c.start + c.text.length, text.lastIndexOf('</', c.end - 1));
    return { n, name: nm ? nm[1] : '', caption: unHtml(inner.replace(/<[^>]*>/g, '')), act:   // (1006 audit: not trimmed -- a caption's own spaces survive a round trip)
      hasClass(c.text, 'act'),
      tab: [c.start, c.end], tabTag: c.text, pane: p ? [p.start, p.end] : null, paneTag: p ? p.text : '' };
  });
  return { id: attr(wrap.tag.text, 'id') || '', tabs, body, list };
}

/** the sheet `id` means: a sheet's name, or the sheet a component is on (its nearest). null = none */
function sheetFor(text, id) {
  const wrap = wrapFor(text, id);
  const s = wrap ? sheetsOf(text, wrap) : null;
  if (!s) return null;
  const byName = s.list.find(x => x.name === id);
  if (byName) return { wrap, sheets: s, sheet: byName };
  const st = he.startTagOf(text, id);
  if (!st || hasClass(st.text, 'pcWrap')) return { wrap, sheets: s, sheet: null };
  const on = s.list.find(x => x.pane && st.start >= x.pane[0] && st.start < x.pane[1]);
  return { wrap, sheets: s, sheet: on || null };
}

/** every name the page already uses: ids and the sheets' names */
function usedNames(text) {
  const used = new Set(hb.idsIn(text));
  const re = /title\s*=\s*"([A-Za-z_]\w*)\s*:\s*TTabSheet"/g;
  let m;
  while ((m = re.exec(text))) used.add(m[1]);
  return used;
}

/**
 * A new sheet at the end of the PageControl `id` means (BCB6 / WinForms: New Page / Add Tab): its tab and an empty pane.
 * The new one is not made the shown one (the page opens on the sheet it did). name / caption: given (tests) or
 * TabSheetN. { parts, name, caption, n, wrapId } or { error }.
 */
function addSheet(text, id, opts) {
  opts = opts || {};
  const wrap = wrapFor(text, id);
  if (!wrap) return { error: '「' + id + '」不是多分頁元件（TPageControl），也不在分頁裡' };
  const s = sheetsOf(text, wrap);
  if (!s) return { error: '「' + (attr(wrap.tag.text, 'id') || id) + '」的分頁列或內容區在原始碼裡找不到（不是產生器的格式）' };
  // (1009 second review (edit core #6): the names the designer keeps for the page too (opts.taken: the .dfm's, the hidden /
  //  locked / noted ones), and without case -- a new TabSheet3 took a .dfm component's name and its events)
  const used0 = usedNames(text);
  const lc = new Set(Array.from(used0).concat(Array.from(opts.taken || [])).map(x => String(x).toLowerCase()));
  const used = { has: x => lc.has(String(x).toLowerCase()) };
  let name = opts.name ? String(opts.name) : '';
  if (name && (!/^[A-Za-z_]\w*$/.test(name) || used.has(name))) return { error: '分頁名稱「' + name + '」' + (used.has(name) ? '已經有人用了' : '不能當名稱（英文字母開頭，只能有英數字和 _）') };
  // (the toolbox names a new PageControl's sheets PageControl1Sheet1, …: more of them the same way; else BCB6's TabSheetN)
  const base = s.id && s.list.some(x => new RegExp('^' + escRe(s.id) + 'Sheet\\d+$').test(x.name)) ? s.id + 'Sheet' : 'TabSheet';
  for (let k = s.list.length + 1; !name; k++) if (!used.has(base + k)) name = base + k;
  const caption = opts.caption != null && String(opts.caption).trim() !== '' ? String(opts.caption) : name;
  const n = s.list.reduce((mx, x) => Math.max(mx, isFinite(+x.n) ? +x.n : -1), -1) + 1;
  const first = !s.list.length;
  const tab = '<div class="tab' + (first ? ' act' : '') + '" data-t="' + n + '" title="' + name + ' : TTabSheet">' + escHtml(caption) + '</div>';
  const pane = '<div class="pcPane" data-p="' + n + '" title="' + name + '" style="display:' + (first ? 'block' : 'none') + ';"></div>';
  const ta = text.lastIndexOf('</', s.tabs.range[1] - 1), ba = text.lastIndexOf('</', s.body.range[1] - 1);
  return { parts: [{ range: [ta, ta], repl: tab }, { range: [ba, ba], repl: pane }], name, caption, n, wrapId: s.id };
}

/**
 * The sheet `name` taken out: its tab and its pane (with what is on it). The last one stays (a PageControl with none
 * makes no sense to the page's JS). The shown one gone = the first left is shown. Data-t / data-p are not renumbered
 * (they only pair a tab with its pane). { parts, ids: the components that go with it, left } or { error }.
 */
function removeSheet(text, name) {
  const f = sheetFor(text, name);
  if (!f || !f.sheet || f.sheet.name !== name) return { error: '找不到分頁「' + name + '」' };
  const list = f.sheets.list;
  if (list.length < 2) return { error: '「' + f.sheets.id + '」只剩這一個分頁，不能刪（要整個拿掉就刪除那個分頁元件）' };
  const sh = f.sheet;
  const parts = [{ range: sh.tab, repl: '' }];
  if (sh.pane) parts.push({ range: sh.pane, repl: '' });
  const ids = sh.pane ? hb.idsIn(text.slice(sh.pane[0], sh.pane[1])) : [];
  const shown = sh.act || (sh.paneTag && /display\s*:\s*block/.test(sh.paneTag));
  if (shown) {
    const nx = list.find(x => x !== sh);
    const tg = nx.tabTag.replace(/\bclass\s*=\s*"([^"]*)"/, (m0, c) => 'class="' + (/\bact\b/.test(c) ? c : c + ' act') + '"');
    parts.push({ range: [nx.tab[0], nx.tab[0] + nx.tabTag.length], repl: tg });
    if (nx.pane) {
      const pt = /display\s*:\s*none/.test(nx.paneTag) ? nx.paneTag.replace(/display\s*:\s*none/, 'display:block')
        : /style\s*=\s*"/.test(nx.paneTag) ? nx.paneTag : nx.paneTag.replace(/>$/, ' style="display:block;">');
      if (pt !== nx.paneTag) parts.push({ range: [nx.pane[0], nx.pane[0] + nx.paneTag.length], repl: pt });
    }
  }
  parts.sort((a, b) => a.range[0] - b.range[0]);
  return { parts, ids, left: list.length - 1, wrapId: f.sheets.id };
}

/** the sheet `name`'s tab caption = `caption`. { parts } or { error } */
function setCaption(text, name, caption) {
  const f = sheetFor(text, name);
  if (!f || !f.sheet || f.sheet.name !== name) return { error: '找不到分頁「' + name + '」' };
  const c = String(caption == null ? '' : caption).replace(/[\r\n]+/g, ' ');
  if (!c.trim()) return { error: '分頁標題不能空白' };
  const sh = f.sheet;
  const s0 = sh.tab[0] + sh.tabTag.length, s1 = text.lastIndexOf('</', sh.tab[1] - 1);
  return { parts: [{ range: [s0, s1], repl: escHtml(c) }] };
}

/**
 * 1006 the sheet `name`'s tab one place left (-1) / right (+1) among the tabs (WinForms' TabPages editor ↑↓, BCB6
 * PageIndex). Only the tabs' order changes -- a tab still goes with its pane by data-t = data-p. { parts } or { error }.
 */
function moveSheet(text, name, dir) {
  const f = sheetFor(text, name);
  if (!f || !f.sheet || f.sheet.name !== name) return { error: '找不到分頁「' + name + '」' };
  const list = f.sheets.list, i = list.indexOf(f.sheet), j = i + (dir < 0 ? -1 : 1);
  if (j < 0 || j >= list.length) return { error: '分頁「' + name + '」已經在最' + (dir < 0 ? '左' : '右') + '邊了' };
  const a = list[Math.min(i, j)], b = list[Math.max(i, j)];
  const between = text.slice(a.tab[1], b.tab[0]);
  return { parts: [{ range: [a.tab[0], b.tab[1]], repl: text.slice(b.tab[0], b.tab[1]) + between + text.slice(a.tab[0], a.tab[1]) }] };
}

/**
 * 1006 the sheet `name` = the one the page opens on (BCB6 ActivePage): its tab "act" and its pane display:block, every
 * other tab without "act" and its pane display:none. { parts } (none when it already is) or { error }.
 */
function setActiveSheet(text, name) {
  const f = sheetFor(text, name);
  if (!f || !f.sheet || f.sheet.name !== name) return { error: '找不到分頁「' + name + '」' };
  const parts = [];
  for (const x of f.sheets.list) {
    const on = x === f.sheet;
    const tg = x.tabTag.replace(/\bclass\s*=\s*"([^"]*)"/, (m0, c) => {
      const cl = c.split(/\s+/).filter(k => k && k !== 'act');
      if (on) cl.push('act');
      return 'class="' + cl.join(' ') + '"';
    });
    if (tg !== x.tabTag) parts.push({ range: [x.tab[0], x.tab[0] + x.tabTag.length], repl: tg });
    if (x.pane) {
      const want = on ? 'block' : 'none';
      let pt = x.paneTag;
      if (/display\s*:\s*\w+/.test(pt)) pt = pt.replace(/display\s*:\s*\w+/, 'display:' + want);
      else if (/style\s*=\s*"/.test(pt)) pt = pt.replace(/style\s*=\s*"/, 'style="display:' + want + ';');
      else pt = pt.replace(/>$/, ' style="display:' + want + ';">');
      if (pt !== x.paneTag) parts.push({ range: [x.pane[0], x.pane[0] + x.paneTag.length], repl: pt });
    }
  }
  parts.sort((p, q) => p.range[0] - q.range[0]);
  return { parts };
}

module.exports = { wrapFor, sheetsOf, sheetFor, addSheet, removeSheet, setCaption, moveSheet, setActiveSheet };
