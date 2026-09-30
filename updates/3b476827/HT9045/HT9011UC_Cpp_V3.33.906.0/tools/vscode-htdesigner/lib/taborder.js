'use strict';
// AI(W906-HTDESIGNER) 20260929: Tab 順序 -- the order the .dfm gives the Tab key (BCB6's
// Edit > Tab Order), to compare with the order the page's Tab key really takes.
// Plain Node (no vscode).

/*
 * VCL's own rule (TWinControl.GetTabOrderList + FindNextControl): a container's windowed
 * children in TabOrder, each one followed by its own children -- depth first; a control
 * the Tab key stops at is one with TabStop that can take the focus. A graphic control
 * (TLabel, TSpeedButton, TImage ...) is never one; an invisible or disabled one is not,
 * and neither is anything inside it.
 */

// the classes whose TabStop is True unless the .dfm says otherwise (VCL's defaults)
const TABSTOP_DEFAULT = /^T(Edit|Memo|Button|BitBtn|CheckBox|RadioButton|ComboBox|ListBox|CheckListBox|SpinEdit|CSpinEdit|MaskEdit|LabeledEdit|RichEdit|StringGrid|DrawGrid|TrackBar|DateTimePicker|MonthCalendar|ListView|TreeView|PageControl|TabControl|HotKey|ScrollBar|ColorBox|ValueListEditor|DBEdit|DBComboBox|DBMemo)$/;

function prop(n, k) {
  const p = n && n.properties ? n.properties[k] : null;
  return p ? p.value : undefined;
}
function isFalse(v) { return v === false || String(v) === 'False'; }

/** Does the Tab key stop at this control (by the .dfm)? */
function tabStopOf(n) {
  const ts = prop(n, 'TabStop');
  if (ts !== undefined) return !isFalse(ts);
  return TABSTOP_DEFAULT.test(n.class || '');
}

/**
 * The names of the form's controls in the order the Tab key visits them, by the .dfm.
 *   ir: IrStore.load() -- { root, byName } with nodes { name, class, path, parent_path,
 *       tab_order, sibling_index, is_graphic_control, properties }
 */
function dfmTabOrder(ir) {
  if (!ir || !ir.root || !ir.byName) return [];
  const kids = new Map();
  for (const n of ir.byName.values()) {
    if (n === ir.root) continue;
    const pp = n.parent_path || '';
    if (!kids.has(pp)) kids.set(pp, []);
    kids.get(pp).push(n);
  }
  const num = (v, d) => (typeof v === 'number' ? v : d);
  const out = [];
  const seen = new Set();
  const walk = p => {
    if (seen.has(p)) return;
    seen.add(p);
    const list = (kids.get(p) || []).filter(n => !n.is_graphic_control)
      .sort((a, b) => (num(a.tab_order, 1e6) - num(b.tab_order, 1e6)) || (num(a.sibling_index, 0) - num(b.sibling_index, 0)));
    for (const n of list) {
      if (isFalse(prop(n, 'Visible')) || isFalse(prop(n, 'Enabled'))) continue;   // nor anything inside it
      if (tabStopOf(n)) out.push(n.name);
      if (n.path) walk(n.path);
    }
  };
  walk(ir.root.path || ir.root.name);
  return out;
}

/**
 * The page's Tab order against the .dfm's, over the controls both have:
 *   page: ids in the page's Tab order; dfm: dfmTabOrder()
 * -> { common: [{ id, page, dfm }] (1-based ranks among the common ones), mismatched: n }
 */
function compareTabOrder(page, dfm) {
  const inDfm = new Set(dfm || []);
  const inPage = new Set(page || []);
  const pc = (page || []).filter(id => inDfm.has(id));
  const dc = (dfm || []).filter(id => inPage.has(id));
  const dRank = new Map(dc.map((id, i) => [id, i + 1]));
  const common = pc.map((id, i) => ({ id, page: i + 1, dfm: dRank.get(id) }));
  return { common, mismatched: common.filter(c => c.page !== c.dfm).length };
}

module.exports = { dfmTabOrder, compareTabOrder, tabStopOf, TABSTOP_DEFAULT };
