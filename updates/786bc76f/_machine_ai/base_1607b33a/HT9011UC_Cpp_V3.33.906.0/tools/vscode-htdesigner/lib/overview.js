'use strict';
// AI(W906-HTDESIGNER) 20260929: the wiring overview of one page -- every DFM event of
// every control, and for each: is it wired on the web page, implemented in the C++
// port, where is it in BCB6. The FW campaign's "what is left on this page" view.
// Plain Node (no vscode).

const { domTypesOf } = require('./format');

// events the wire engine covers through its field map (keyboard pop-up, value
// write-back) rather than a listener of that exact DOM type
const FIELD_EVENTS = new Set(['OnChange', 'OnKeyPress', 'OnKeyDown', 'OnKeyUp', 'OnMouseDown', 'OnMouseUp', 'OnExit', 'OnEnter', 'OnClick', 'OnDblClick']);

/** Every quoted name in the page's code ('spbSave', "#XST1", `x`) -- one pass. */
function quotedNames(sources) {
  const out = new Set();
  const re = /(['"`])#?([A-Za-z_]\w*)\1/g;
  for (const s of sources) {
    re.lastIndex = 0;
    let m;
    while ((m = re.exec(s.text))) if (s.inScope(m.index)) out.add(m[2]);
  }
  return out;
}

/**
 * opt: { ir, pageIds:Set, listeners:{id:[types]}, fieldIds:Set, tagIds:Set,
 *        quoted:Set, classes:[...], port:SourceTree|null, gold:SourceTree|null }
 */
async function buildOverview(opt) {
  const ir = opt.ir;
  const rows = [];
  const sum = {
    controls: 0, events: 0, notOnPage: 0,
    web: { yes: 0, field: 0, weak: 0, no: 0 },
    port: { live: 0, dead: 0, mention: 0, none: 0 },
    golden: { live: 0, none: 0 },
  };
  const cache = new Map();
  const state = async (tree, member) => {
    if (!tree || !opt.classes.length) return { state: 'none', hit: null };
    const k = tree.kind + '|' + member;
    if (!cache.has(k)) cache.set(k, tree.defState(opt.classes, member));
    return cache.get(k);
  };
  for (const node of ir.byName.values()) {
    const evs = Object.entries(node.events || {});
    if (!evs.length) continue;
    const isRoot = node === ir.root;
    const id = isRoot ? '@form' : node.name;
    const present = isRoot || opt.pageIds.has(node.name);
    const row = { id, name: node.name, cls: node.class, path: node.path, present, events: [] };
    sum.controls++;
    if (!present) sum.notOnPage++;
    const own = (opt.listeners && opt.listeners[node.name]) || [];
    for (const [ev, handler] of evs) {
      sum.events++;
      let web;
      if (isRoot) web = 'na';
      else if (!present) web = 'absent';
      else if (domTypesOf(ev).some(t => own.includes(t))) web = 'yes';
      else if (opt.fieldIds.has(node.name) && FIELD_EVENTS.has(ev)) web = 'field';
      else if (opt.quoted.has(node.name) || (opt.tagIds && opt.tagIds.has(node.name))) web = 'weak';
      else web = 'no';
      if (web in sum.web) sum.web[web]++;
      const p = await state(opt.port, handler);
      const g = await state(opt.gold, handler);
      sum.port[p.state]++;
      if (g.state === 'live') sum.golden.live++; else sum.golden.none++;
      const loc = s => (s.hit ? { file: s.hit.file, line: s.hit.line, col: s.hit.col } : null);
      row.events.push({
        name: ev, handler, web,
        port: { state: p.state, at: loc(p) },
        golden: { state: g.state === 'live' ? 'live' : 'none', at: loc(g) },
      });
    }
    rows.push(row);
  }
  // controls on the page first, in DFM order; the absent ones last
  rows.sort((a, b) => (a.present === b.present ? 0 : a.present ? -1 : 1));
  return { rows, summary: sum };
}

module.exports = { buildOverview, quotedNames, FIELD_EVENTS };
