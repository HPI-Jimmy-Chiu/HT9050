'use strict';
// AI(W906-HTDESIGNER) 20260930: Alias -- the IO a component of an IO page stands for. The generator writes it into
// the title ("BtnPanelLane3 : TBtnPanelLane｜Alias=C_LoaderEdgePush") and the page's JS reads it from there
// (ht9045_io_do.js / HW.IoSetView.html: title.match(/Alias=([^｜|\s]+)/)), so changing it is changing that part of
// the title. EastSun 20260930: "名稱都要讓我改 alias". Plain Node.
const fs = require('fs');
const csvtable = require('./csvtable');

const RE = /([｜|]?)Alias=([^｜|\s"]*)/;
const OK = /^[^\s｜|"'<>&]+$/;

/** The alias in a title ('' = none). */
function aliasOf(title) {
  const m = RE.exec(String(title || ''));
  return m ? m[2] : '';
}

/** The title with `alias` ('' = the Alias part taken out). -> { title } or { error } */
function withAlias(title, alias) {
  const t = String(title || '');
  const a = String(alias == null ? '' : alias).trim();
  if (a && !OK.test(a)) return { error: 'Alias 不能有空白、｜、引號、< > &' };
  const m = RE.exec(t);
  if (m) {
    if (!a) return { title: (t.slice(0, m.index) + t.slice(m.index + m[0].length)).replace(/\s+$/, '') };
    return { title: t.slice(0, m.index) + m[1] + 'Alias=' + a + t.slice(m.index + m[0].length) };
  }
  if (!a) return { title: t };
  return { title: t + '｜Alias=' + a };
}

/** The Alias column of an IO table (IO_Table.csv, read only): [alias, ...] in the file's order, or []. */
function ioAliases(file) {
  let text;
  try { text = fs.readFileSync(file, 'utf8'); } catch (e) { return []; }
  const p = csvtable.parse(text);
  if (!p.rows.length) return [];
  const col = p.rows[0].cells.findIndex(c => c.v.trim().toLowerCase() === 'alias');
  if (col < 0) return [];
  const out = [];
  const seen = new Set();
  for (let r = 1; r < p.rows.length; r++) {
    const c = p.rows[r].cells[col];
    const v = c ? c.v.trim() : '';
    // AI(W906-HTDESIGNER) 20261001: a "#..." row is the table's own section marker (HT9050's IO_Table.csv has
    // ",#NEW_FROM_9050_DRAWING_20260923,..." since 09-23), not an IO -- it is not offered as an Alias
    if (v && v.charAt(0) !== '#' && !seen.has(v)) { seen.add(v); out.push(v); }
  }
  return out;
}

module.exports = { aliasOf, withAlias, ioAliases };
