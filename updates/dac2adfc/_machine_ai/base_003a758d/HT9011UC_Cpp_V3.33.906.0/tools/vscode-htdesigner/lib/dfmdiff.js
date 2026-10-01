'use strict';
// AI(W906-HTDESIGNER) 20260929: 與 DFM 的差異 -- the whole page against the BCB6 .dfm.
// The same comparison the property grid makes for one control (media/props.js), for
// every control of the page at once. Plain Node (no vscode).
const fmt = require('./format');

/** Same value? numbers rounded, booleans as booleans, text trimmed and case-blind (#RRGGBB). */
function same(a, b) {
  if (typeof a === 'number' || typeof b === 'number') return Math.round(+a) === Math.round(+b);
  if (typeof a === 'boolean' || typeof b === 'boolean') return !!a === !!b;
  return String(a == null ? '' : a).trim().toLowerCase() === String(b == null ? '' : b).trim().toLowerCase();
}

function shown(v) {
  if (v === null || v === undefined) return '';
  if (typeof v === 'boolean') return v ? 'True' : 'False';
  return String(v);
}

const GROUP = { layout: '位置／大小', text: '文字', look: '外觀' };

/**
 * Does the page's source give this size? A width the source does not set (an
 * AutoSize TLabel, a box sized by its text) is the text's -- in another font than
 * BCB6's -- so it is not compared. Only a px size counts: the generated pages write
 * an AutoSize label as "width:136px; … width:auto" (the later one wins). Anchored
 * on both sides (left + right in px) counts as set.
 *   raw: the positioning box's inline style { left, top, width, height, right, bottom }
 */
const PX = /^\s*-?\d+(\.\d+)?px\s*$/;
function sizeInSource(raw, k) {
  if (!raw) return true;
  if (k === 'width') return PX.test(raw.width || '') || (PX.test(raw.left || '') && PX.test(raw.right || ''));
  if (k === 'height') return PX.test(raw.height || '') || (PX.test(raw.top || '') && PX.test(raw.bottom || ''));
  return true;
}

/**
 * items: the probe's lookAll answer -- [{ id, lay, cap, look }]
 * ir:    the page's IR (IrStore.load) -- byName, root
 * Returns { rows: [{ id, cls, group, prop, page, dfm, reset, note }], compared, controls }
 * reset = the message that writes the DFM value back ({ type:'setLayout', left:54 } ...).
 */
function diffPage(items, ir) {
  const rows = [];
  let compared = 0;
  let controls = 0;
  const rootName = ir && ir.root ? ir.root.name : null;
  for (const it of items || []) {
    const node = ir && ir.byName ? ir.byName.get(it.id) : null;
    if (!node || node.name === rootName) continue;   // the form: its DFM Width/Height include the window frame
    const dv = fmt.dfmEditValues(node);
    if (!dv) continue;
    controls++;
    const cls = node.class || '';
    const add = (group, prop, page, dfm, reset, note, dfmText) => {
      compared++;
      if (same(dfm, page)) return;
      rows.push({ id: it.id, cls, group, prop, page: shown(page), dfm: dfmText || shown(dfm), reset, note: note || '' });
    };
    const lay = it.lay;
    if (lay && !lay.root) {
      // a wrapper the generator adds (span.lled around an LED + its label) moves the
      // origin: compare in the DFM's coordinates, reset in the element's own
      const shift = { left: lay.ox || 0, top: lay.oy || 0, width: 0, height: 0 };
      for (const k of ['left', 'top', 'width', 'height']) {
        if (dv[k] === null || lay[k] === null || lay[k] === undefined) continue;
        if (!sizeInSource(lay.raw, k)) continue;
        const r = { type: 'setLayout' };
        r[k] = dv[k] - shift[k];
        const notes = [];
        if (lay.rendered === false) notes.push('目前沒有顯示（隱藏的分頁或面板），用原始碼 style 裡的數值比對');
        if (shift[k]) notes.push('外層包裝讓座標原點位移 ' + shift[k] + ' px，已換算成 DFM 的座標');
        add('layout', k.charAt(0).toUpperCase() + k.slice(1), lay[k] + shift[k], dv[k], r, notes.join('；'));
      }
    }
    const cap = it.cap;
    if (cap) {
      const want = cap.kind === 'value' ? dv.text : dv.caption;
      if (want !== null) add('text', cap.kind === 'value' ? 'Text' : 'Caption', cap.value, want, { type: 'setCaption', value: want });
    }
    const lk = it.look;
    if (lk) {
      const look = (prop, label, page, dfm, note, dfmText) => {
        if (dfm === null || dfm === undefined) return;
        add('look', label, page, dfm, { type: 'setLook', prop, value: dfm }, note, dfmText);
      };
      look('visible', 'Visible', lk.visible, dv.visible);
      if (lk.autoSize !== null && lk.autoSize !== undefined && dv.autoSize !== null) {
        // AutoSize = False in BCB6: the label has the .dfm's own size -- the reset gives it that
        // size (not the one its text makes now), so one reset makes it match
        const size = dv.autoSize === false && dv.width !== null && dv.height !== null ? { width: dv.width, height: dv.height } : null;
        compared++;
        if (!same(dv.autoSize, lk.autoSize)) {
          rows.push({ id: it.id, cls, group: 'look', prop: 'AutoSize', page: shown(lk.autoSize), dfm: shown(dv.autoSize),
            reset: Object.assign({ type: 'setLook', prop: 'autoSize', value: dv.autoSize }, size ? { size } : {}), note: size ? 'DFM 的大小 ' + size.width + ' × ' + size.height : '' });
        }
      }
      if (lk.alignment !== null && lk.alignment !== undefined) look('alignment', 'Alignment', lk.alignment, dv.alignment);
      if (lk.enabled !== null && lk.enabled !== undefined) look('enabled', 'Enabled', lk.enabled, dv.enabled);
      // a font family the page does not give for this control is the form's UI font (the
      // generator's choice for labels and buttons): not compared
      if (dv.fontName !== null && lk.fontOwn !== false) {
        // the page's font list naming the DFM font counts as the same font
        const fams = Array.isArray(lk.fontFamilies) ? lk.fontFamilies : [lk.fontName];
        const hit = fams.some(f => String(f).toLowerCase() === dv.fontName.toLowerCase());
        look('fontName', 'Font.Name', hit ? dv.fontName : lk.fontName, dv.fontName);
      }
      // DFM Font.Height -13 -> the 11px a generated page has (|Height| - 2)
      look('fontSize', 'Font.Size', lk.fontSize, dv.fontSize, '', dv.fontHeight !== null ? dv.fontSize + '（Height ' + dv.fontHeight + '）' : '');
      look('bold', 'Font.Bold', lk.bold, dv.bold);
      look('italic', 'Font.Italic', lk.italic, dv.italic);
      look('color', 'Font.Color', lk.color, dv.color);          // system colours are null here: not compared
      look('background', 'Color', lk.background, dv.background);
    }
  }
  // the same difference on many controls of one class (every TBtnPanelLane bold, every
  // GroupBox caption in the theme colour) is the page's common style, not a one-off
  // change: those are folded into one pattern line
  const count = new Map();
  const keyOf = r => r.cls + '|' + r.prop + '|' + r.page + '|' + r.dfm;
  for (const r of rows) count.set(keyOf(r), (count.get(keyOf(r)) || 0) + 1);
  const patterns = [];
  const seen = new Set();
  for (const r of rows) {
    const k = keyOf(r);
    if (count.get(k) < PATTERN_MIN) continue;
    r.pattern = k;
    if (!seen.has(k)) { seen.add(k); patterns.push({ key: k, cls: r.cls, group: r.group, prop: r.prop, page: r.page, dfm: r.dfm, count: count.get(k) }); }
  }
  patterns.sort((a, b) => b.count - a.count);
  const byGroup = { layout: 0, text: 0, look: 0 };
  let single = 0;
  for (const r of rows) { byGroup[r.group]++; if (!r.pattern) single++; }
  return { rows, compared, controls, byGroup, patterns, single };
}

/** How many controls of one class must share a difference to count as the page's common style. */
const PATTERN_MIN = 5;

/**
 * DFM 位置 on the design surface: for each control whose position / size differs, where
 * the .dfm puts it -- in the control's own coordinates (the values its style would get,
 * the wrapper shift already taken off), only the sides that differ:
 *   [{ id, left?, top?, width?, height? }]
 */
function ghostsOf(rows) {
  const by = new Map();
  for (const r of rows || []) {
    if (r.group !== 'layout' || !r.reset || r.reset.type !== 'setLayout') continue;
    const g = by.get(r.id) || { id: r.id };
    for (const k of ['left', 'top', 'width', 'height']) if (typeof r.reset[k] === 'number') g[k] = r.reset[k];
    by.set(r.id, g);
  }
  return Array.from(by.values());
}

/**
 * 改回 DFM: the difference rows as the probe's editMany items -- one setLayout per control
 * (its differing sides merged), one setLook / setCaption per property.
 */
function resetItemsOf(rows) {
  const items = [];
  const lay = new Map();
  for (const r of rows || []) {
    const rs = r.reset;
    if (!rs) continue;
    if (rs.type === 'setLayout') {
      let it = lay.get(r.id);
      // (force: the page writes these values even where it already shows them -- the source is what counts)
      if (!it) { it = { id: r.id, type: 'setLayout', force: true }; lay.set(r.id, it); items.push(it); }
      for (const k of ['left', 'top', 'width', 'height']) if (typeof rs[k] === 'number') it[k] = rs[k];
    } else if (rs.type === 'setLook') items.push(Object.assign({ id: r.id, type: 'setLook', prop: rs.prop, value: rs.value }, rs.size ? { size: rs.size } : {}));
    else if (rs.type === 'setCaption') items.push({ id: r.id, type: 'setCaption', value: rs.value });
  }
  return items;
}

module.exports = { same, sizeInSource, diffPage, ghostsOf, resetItemsOf, GROUP, PATTERN_MIN };
