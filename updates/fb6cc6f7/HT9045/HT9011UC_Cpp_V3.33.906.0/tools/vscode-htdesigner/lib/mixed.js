'use strict';
// AI(W906-HTDESIGNER) 20260929: several selected -- which fields of the properties panel have
// different values among them (WPF shows those empty). Plain Node (no vscode).

const LAYOUT = ['left', 'top', 'width', 'height'];
const LOOK = ['visible', 'enabled', 'autoSize', 'alignment', 'fontName', 'fontSize', 'bold', 'italic', 'color', 'background',
  'underline', 'strikeout', 'wordWrap', 'readOnly', 'maxLength', 'checked', 'tabOrder',
  // (1009 second review (surface #8): a TLabeledEdit's label and a TTrackBar's range -- the primary's shown as everyone's)
  'editLabel', 'range.min', 'range.max', 'range.value'];   // (0.152)
// AI(W906-HTDESIGNER) 20261001: the IO lamp / panel button's own (the grid's io.* fields) -- three lamps of different
// shapes selected showed the primary one's LEDStyle as if all were the same (WPF / the Object Inspector: shown empty)
const IO = ['io.ledStyle', 'io.value', 'io.blink', 'io.trueColor', 'io.falseColor', 'io.flat', 'io.down', 'io.trueFontColor', 'io.falseFontColor'];

function valueOf(it, f) {
  if (LAYOUT.includes(f)) return it.lay ? it.lay[f] : undefined;
  if (f === 'caption') return it.cap ? it.cap.value : undefined;
  if (f.indexOf('io.') === 0) return it.look && it.look.io ? it.look.io[f.slice(3)] : undefined;
  if (f.indexOf('.') > 0) { const p = f.split('.'); return it.look && it.look[p[0]] && typeof it.look[p[0]] === 'object' ? it.look[p[0]][p[1]] : undefined; }
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
  for (const f of LAYOUT.concat(['caption'], LOOK, IO)) {
    const vals = list.map(it => valueOf(it, f)).filter(v => v !== undefined && v !== null);
    if (vals.length < 2) continue;
    // (1009 second review (edit core #8): a caption's case counts -- "Start" and "START" are two)
    const s = new Set(vals.map(v => (typeof v === 'string' ? (f === 'caption' ? v.trim() : v.trim().toLowerCase()) : String(v))));
    if (s.size > 1) out[f] = true;
  }
  return out;
}

module.exports = { mixedFields, FIELDS: LAYOUT.concat(['caption'], LOOK, IO) };
