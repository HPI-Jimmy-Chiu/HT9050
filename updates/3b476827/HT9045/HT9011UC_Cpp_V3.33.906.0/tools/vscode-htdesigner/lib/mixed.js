'use strict';
// AI(W906-HTDESIGNER) 20260929: several selected -- which fields of the properties panel have
// different values among them (WPF shows those empty). Plain Node (no vscode).

const LAYOUT = ['left', 'top', 'width', 'height'];
const LOOK = ['visible', 'enabled', 'autoSize', 'alignment', 'fontName', 'fontSize', 'bold', 'italic', 'color', 'background'];

function valueOf(it, f) {
  if (LAYOUT.includes(f)) return it.lay ? it.lay[f] : undefined;
  if (f === 'caption') return it.cap ? it.cap.value : undefined;
  return it.look ? it.look[f] : undefined;
}

/**
 * items: the probe's lookAll answer for the selected ones -- [{ id, lay, cap, look }]
 * -> { field: true } for each panel field (the data-field names) whose value is not the
 * same on all of them; a component that has no such field (no caption ...) does not count.
 */
function mixedFields(items) {
  const out = {};
  const list = (items || []).filter(Boolean);
  if (list.length < 2) return out;
  for (const f of LAYOUT.concat(['caption'], LOOK)) {
    const vals = list.map(it => valueOf(it, f)).filter(v => v !== undefined && v !== null);
    if (vals.length < 2) continue;
    const s = new Set(vals.map(v => (typeof v === 'string' ? v.trim().toLowerCase() : String(v))));
    if (s.size > 1) out[f] = true;
  }
  return out;
}

module.exports = { mixedFields, FIELDS: LAYOUT.concat(['caption'], LOOK) };
