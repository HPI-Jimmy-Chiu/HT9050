'use strict';
// AI(W906-HTDESIGNER) 20260929: 與 DFM 的差異 -- the whole page against the BCB6 .dfm.
// The same comparison the property grid makes for one control (media/props.js), for
// every control of the page at once. Plain Node (no vscode).
const fmt = require('./format');

/** Same value? numbers rounded, booleans as booleans, text trimmed and case-blind (#RRGGBB, a font name); exact = a
 *  caption / text: the case counts. Line ends as the browser has them (\r\n -> \n) either way. */
function same(a, b, exact) {
  if (typeof a === 'number' || typeof b === 'number') return Math.round(+a) === Math.round(+b);
  if (typeof a === 'boolean' || typeof b === 'boolean') return !!a === !!b;
  // (1009 review (DFM #1 / #8): a .dfm caption's \r\n is \n in the page -- every multi-line caption differed for ever and
  //  its ↺ wrote \r\n back, to differ again after a reload; and "Show 0X Bin" vs "Show 0X bin" was no difference)
  const t = v => String(v == null ? '' : v).replace(/\r\n?/g, '\n').trim();
  return exact ? t(a) === t(b) : t(a).toLowerCase() === t(b).toLowerCase();
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
function diffPage(items, ir, opts) {
  const cut = (opts && opts.cut) || null;
  const rows = [];
  let compared = 0;
  let controls = 0;
  const rootName = ir && ir.root ? ir.root.name : null;
  // (the common UI fonts the generator puts every label / button in -- a DFM Arial / Tahoma there is that choice; an
  //  OpenSymbol / Courier New / Batang / Times New Roman is the control's own look and is compared)
  const UI_FONTS = new Set(['ms sans serif', 'microsoft sans serif', 'arial', 'tahoma', 'segoe ui', 'verdana', 'system', 'ms shell dlg', 'ms shell dlg 2', '新細明體', 'pmingliu', '細明體', 'mingliu', '微軟正黑體', 'microsoft jhenghei', 'microsoft jhenghei ui']);
  // (review of 0.419 #1: the form's UI font -- a control's own DFM font (OpenSymbol, Courier New) is compared even when the
  //  page gives it none: the browser's Arial is drawn instead)
  const rootFont = String(((ir && ir.root ? fmt.dfmEditValues(ir.root) : null) || {}).fontName || 'MS Sans Serif').toLowerCase();
  const uiFont = n => { const l = String(n || '').toLowerCase(); return !l || l === rootFont || UI_FONTS.has(l); };
  // (1010 review of 0.394: the containers this page has; review of 0.395 A: the caller's list when it asked for a few
  //  only -- 改回 DFM of the selected one asked for it alone, the parent check never fired and the DFM numbers went into the
  //  new container)
  const onPage = opts && opts.pageIds ? new Set(opts.pageIds) : new Set((items || []).map(x => x && x.id).filter(Boolean));
  const isSheet = x => { const nd = ir && ir.byName ? ir.byName.get(x) : null; return !!(nd && /^TTabSheet$/i.test(nd.class || '')); };
  for (const it of items || []) {
    const node = ir && ir.byName ? ir.byName.get(it.id) : null;
    if (!node || node.name === rootName) continue;   // the form: its DFM Width/Height include the window frame
    const dv = fmt.dfmEditValues(node);
    if (!dv) continue;
    controls++;
    const cls = node.class || '';
    // (1009 second review (DFM #2): the generator's unified buttons -- a TPanel / TButton of the .dfm made a btn3d
    //  TSpeedButton on purpose ("統一 exit 樣式，原 TPanel"): their text / look are not put back; their place is compared)
    const changedKind = !!(it.cls && cls && it.cls !== cls);
    // (1009 second review (DFM #4): a start tag whose style is cut by a quote inside it -- the browser dropped the rest
    //  (font-weight:700 …): a look difference there is that cut, and ↺ is refused at the source until it is fixed)
    const isCut = !!(cut && cut.has(it.id));
    const add = (group, prop, page, dfm, reset, note, dfmText) => {
      compared++;
      if (same(dfm, page, group === 'text')) return;
      if (changedKind && group !== 'layout') { reset = null; note = (note ? note + '；' : '') + '頁面刻意改成 ' + it.cls + '（DFM 是 ' + cls + '），文字和外觀不改回'; }
      else if (isCut && group === 'look') { reset = null; note = (note ? note + '；' : '') + '這個元件的 style 在原始碼被引號切斷，瀏覽器讀不到後面的值——先在頁面檢查修正它'; }
      rows.push({ id: it.id, cls, group, prop, page: shown(page), dfm: dfmText || shown(dfm), reset, note: note || '' });
    };
    const lay = it.lay;
    // (1010 toolbox walkthrough #1: another parent than the .dfm's (grouped into a new Panel, moved with Alt / the tree) -- its
    //  numbers are in another container's coordinates: said, not put back (↺ moved it to the DFM numbers inside the new one))
    // (tab sheets are no element with an id on the page: the nearest DFM container that is not a sheet, against the nearest
    //  component around it on the page; the form itself = none)
    const dChain = String(node.parent_path || '').split('.').filter(x => x && x !== rootName && !isSheet(x));
    const dPar = dChain.length ? dChain[dChain.length - 1] : '';
    // (1010 review of 0.394: the page's tab panes may have an id too -- skipped on that side as well; and only where the
    //  .dfm's container IS on this page and is not the one around it (a fragment page, a renamed or flattened container is
    //  no move: 517 controls of untouched pages were flagged))
    const pPar = Array.isArray(it.par) ? (it.par.find(x => x !== rootName && !isSheet(x)) || '') : null;
    const otherParent = pPar !== null && !!pPar && !!dPar && onPage.has(dPar) && pPar !== dPar;   // (… and inside ANOTHER component: a flat page layout is no move -- 65 pages, 16583 controls: 1 left, a stale IR)
    if (otherParent) { compared++; rows.push({ id: it.id, cls, group: 'layout', prop: 'Parent', page: pPar || '（表單）', dfm: dPar || '（表單）', reset: null, note: '上層容器跟 DFM 不同：位置是新容器的座標，不能直接改回' }); }
    if (lay && !lay.root && !otherParent) {
      // a wrapper the generator adds (span.lled around an LED + its label) moves the
      // origin: compare in the DFM's coordinates, reset in the element's own
      const shift = { left: lay.ox || 0, top: lay.oy || 0, width: 0, height: 0 };
      // (1009 second review (DFM #1): a labeled lamp -- the box is its <span class="lled"> (the lamp AND its caption, two
      //  components in the .dfm): the LAMP's own place / size compared; its place put back by moving the wrapper; its size
      //  not (the wrapper's size would shrink the caption away))
      const lamp = lay.lamp || null;
      for (const k of ['left', 'top', 'width', 'height']) {
        if (dv[k] === null || lay[k] === null || lay[k] === undefined) continue;
        if (!lamp && !sizeInSource(lay.raw, k)) continue;
        const r = { type: 'setLayout' };
        const inner = lamp ? (k === 'left' ? lamp.dl : k === 'top' ? lamp.dt : 0) : 0;
        r[k] = dv[k] - shift[k] - inner;
        const notes = [];
        if (lay.rendered === false) notes.push('目前沒有顯示（隱藏的分頁或面板），用原始碼 style 裡的數值比對');
        if (shift[k]) notes.push('外層包裝讓座標原點位移 ' + shift[k] + ' px，已換算成 DFM 的座標');
        const pageV = lamp && (k === 'width' || k === 'height') ? (k === 'width' ? lamp.w : lamp.h) : lay[k] + shift[k] + inner;
        if (lamp && (k === 'width' || k === 'height')) { if (pageV === null || pageV === undefined) continue; notes.push('燈號和文字包在一起：燈號大小請在原始碼改'); add('layout', k.charAt(0).toUpperCase() + k.slice(1), pageV, dv[k], null, notes.join('；')); continue; }
        add('layout', k.charAt(0).toUpperCase() + k.slice(1), pageV, dv[k], r, notes.join('；'));
      }
    }
    const cap = it.cap;
    if (cap) {
      const want = cap.kind === 'value' ? dv.text : dv.caption;
      // (1009 review (DFM #1): a caption of several lines -- the page breaks it with <br> / more text nodes; ↺ replaced the
      //  first line only and the others were there twice. Said, not reset from here)
      const multi = want !== null && /[\r\n]/.test(String(want));
      // (review of 0.418 DFM #2: the text split into elements -- compared whole, blanks collapsed; not reset from here)
      const coll = v => String(v == null ? '' : v).replace(/[ \t\n\r\f\u00a0]+/g, '');   // (review of 0.419 #2: blanks do not count)
      // (review of 0.418 DFM #4: the page's "---" is its live value's placeholder (said while the feed is down) -- the DFM's
      //  design-time text is no value to write there)
      const live = String(cap.value).trim() === '---';
      if (want !== null && typeof cap.full === 'string') { if (coll(cap.full) !== coll(want)) add('text', 'Caption', cap.full, want, null, '文字分在好幾個元素裡（例如單位另外放）：請在 HTML 原始碼改'); }
      else if (want !== null) add('text', cap.kind === 'value' ? 'Text' : 'Caption', cap.value, want, multi || live ? null : { type: 'setCaption', value: want }, multi ? '好幾行的文字：請在 HTML 原始碼改（頁面用 <br> 分行，這裡不能一次換掉）' : live ? '「---」是執行時才填的數值（沒連線時顯示）：DFM 的設計時文字不改回' : '');
    }
    const lk = it.look;
    if (lk) {
      const look = (prop, label, page, dfm, note, dfmText) => {
        if (dfm === null || dfm === undefined) return;
        add('look', label, page, dfm, { type: 'setLook', prop, value: dfm }, note, dfmText);
      };
      // (1009 review (DFM #5): Visible / Enabled not written in the .dfm = True (BCB6 writes only False) -- 64 controls hidden
      //  on the page but shown by BCB6 were never listed)
      const yes = v => (v === null || v === undefined ? true : v);
      if (lk.visible !== null && lk.visible !== undefined) look('visible', 'Visible', lk.visible, yes(dv.visible));
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
      if (lk.enabled !== null && lk.enabled !== undefined) look('enabled', 'Enabled', lk.enabled, yes(dv.enabled));
      // a font family the page does not give for this control is the form's UI font (the
      // generator's choice for labels and buttons): not compared
      if (dv.fontName !== null && (lk.fontOwn !== false || !uiFont(dv.fontName))) {
        // the page's font list naming the DFM font counts as the same font
        const fams = Array.isArray(lk.fontFamilies) ? lk.fontFamilies : [lk.fontName];
        const hit = fams.some(f => String(f).toLowerCase() === dv.fontName.toLowerCase());
        look('fontName', 'Font.Name', hit ? dv.fontName : lk.fontName, dv.fontName);
      }
      // DFM Font.Height -13 -> the 11px a generated page has (|Height| - 2)
      look('fontSize', 'Font.Size', lk.fontSize, dv.fontSize, '', dv.fontHeight !== null ? dv.fontSize + '（Height ' + dv.fontHeight + '）' : '');
      look('bold', 'Font.Bold', lk.bold, dv.bold);
      look('italic', 'Font.Italic', lk.italic, dv.italic);
      // the IO lamp / panel button's own (the same names the grid shows; only what the .dfm writes)
      const io = lk.io, dio = dv.io || {};
      // (1009 review (IO #1): a lamp's / panel button's colours ARE its True / False(Font)Color -- the page draws them from
      //  --led-* / --bp-* and its on / down class. Font.Color / Color compared here gave a difference on a down button
      //  (white text from --bp-true-font) and 改回 DFM wrote an inline color over the class rules: Down / TrueColor never
      //  showed again. Not compared on them)
      if (!io) {
        look('color', 'Font.Color', lk.color, dv.color);          // system colours are null here: not compared
        look('background', 'Color', lk.background, dv.background);
      }
      // (1009 review (IO #4): Value / Blink / Down not written in the .dfm = False (the 1,600 lamps' .dfm only ever write
      //  "True") -- a ticked one was no difference and 全部改回 left it on)
      const off = v => (v === null || v === undefined ? false : v);
      if (io && io.kind === 'led') {
        look('io.ledStyle', 'LEDStyle', io.ledStyle, dio.ledStyle);
        look('io.value', 'Value', io.value, off(dio.value));
        look('io.blink', 'Blink', io.blink, off(dio.blink));
        look('io.trueColor', 'TrueColor', io.trueColor || null, dio.trueColor);
        look('io.falseColor', 'FalseColor', io.falseColor || null, dio.falseColor);
      } else if (io && io.kind === 'btn') {
        if (dio.flat !== null && dio.flat !== undefined) look('io.flat', 'Style', io.flat, dio.flat, '', dio.flat ? 'tsFlatButtons' : 'tsButtons');
        look('io.down', 'Down', io.down, off(dio.down));
        for (const [k, n] of [['trueColor', 'TrueColor'], ['falseColor', 'FalseColor'], ['trueFontColor', 'TrueFontColor'], ['falseFontColor', 'FalseFontColor']]) look('io.' + k, n, io[k] || null, dio[k]);
      }
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
