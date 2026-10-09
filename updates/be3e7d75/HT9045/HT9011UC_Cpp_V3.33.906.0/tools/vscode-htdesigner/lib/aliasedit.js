'use strict';
// AI(W906-HTDESIGNER) 20260930: Alias -- the IO a component of an IO page stands for. The generator writes it into
// the title ("BtnPanelLane3 : TBtnPanelLane｜Alias=C_LoaderEdgePush") and the page's JS reads it from there
// (ht9045_io_do.js / HW.IoSetView.html: title.match(/Alias=([^｜|\s]+)/)), so changing it is changing that part of
// the title. EastSun 20260930: "名稱都要讓我改 alias". Plain Node.
const fs = require('fs');
const csvtable = require('./csvtable');

// (1009 review (alias #6): "Alias=" as a word of its own -- "NoteAlias=Q｜Alias=A" had its NoteAlias changed)
const RE = /(^|[｜|\s])Alias=([^｜|\s"]*)/;
// (1009 review (alias #5): no "," (BCB's CommaText splits the CSV value there) and no leading "#" (the table's section rows))
const OK = /^[^\s｜|"'<>&,#][^\s｜|"'<>&,]*$/;

/** The alias in a title ('' = none). */
function aliasOf(title) {
  const m = RE.exec(String(title || ''));
  return m ? m[2] : '';
}

/** The title with `alias` ('' = the Alias part taken out). -> { title } or { error } */
function withAlias(title, alias) {
  const t = String(title || '');
  const a = String(alias == null ? '' : alias).trim();
  if (a && !OK.test(a)) return { error: 'Alias 不能有空白、｜、引號、逗號、< > &，也不能用 # 開頭' };
  const m = RE.exec(t);
  if (m) {
    if (!a) {
      // (1009 review (alias #7): at the start of the title its "｜" after it goes too -- "Alias=A｜Foo" left "｜Foo")
      let rest = t.slice(m.index + m[0].length);
      const keep = /\s/.test(m[1]) ? m[1] : '';
      if (!t.slice(0, m.index) && !m[1]) rest = rest.replace(/^[｜|]/, '');
      return { title: (t.slice(0, m.index) + keep + rest).replace(/\s+$/, '') };
    }
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
  const dups = [];
  for (let r = 1; r < p.rows.length; r++) {
    const c = p.rows[r].cells[col];
    const v = c ? c.v.trim() : '';
    // AI(W906-HTDESIGNER) 20261001: a "#..." row is the table's own section marker (HT9050's IO_Table.csv has
    // ",#NEW_FROM_9050_DRAWING_20260923,..." since 09-23), not an IO -- it is not offered as an Alias
    if (!v || v.charAt(0) === '#') continue;
    if (seen.has(v)) { if (!dups.includes(v)) dups.push(v); continue; }
    seen.add(v); out.push(v);
  }
  // (1009 review (alias #3): the names in two rows or more -- the machine refuses the table ("IO %s alias is duplicated!"))
  out.dups = dups;
  return out;
}

/**
 * 1009 review (alias #3): the IO table's Alias column changed in the CSV table -- what is wrong with the new values:
 * a name another row has (the machine refuses the table), a blank / comma / ｜ (no page title can name it).
 * rows: [[cell, ...]] (the table now), changes: [{ r, c, v }] -> { col, problems: [text], renamed: [[old, new]] }
 */
function checkTableEdit(rows, changes) {
  const head = (rows && rows[0]) || [];
  const col = head.findIndex(c => String(c == null ? '' : c).trim().toLowerCase() === 'alias');
  const out = { col, problems: [], renamed: [] };
  if (col < 0) return out;
  const mine = (changes || []).filter(c => c && c.c === col && c.r > 0);
  if (!mine.length) return out;
  const after = rows.map(r => (r || []).slice());
  for (const c of mine) { while (after.length <= c.r) after.push([]); after[c.r][col] = c.v; }
  const count = new Map();
  for (let r = 1; r < after.length; r++) { const v = String(after[r][col] == null ? '' : after[r][col]).trim(); if (v && v[0] !== '#') count.set(v, (count.get(v) || 0) + 1); }
  for (const c of mine) {
    const v = String(c.v == null ? '' : c.v).trim();
    const old = String((rows[c.r] || [])[col] == null ? '' : rows[c.r][col]).trim();
    if (v && v[0] !== '#' && count.get(v) > 1) out.problems.push('「' + v + '」已經是別列的 Alias（機台讀表時會說 alias is duplicated）');
    if (v && /[\s,｜|]/.test(v)) out.problems.push('「' + v + '」有空白、逗號或｜（頁面的 title 對不到它）');
    if (old && old[0] !== '#' && v !== old) out.renamed.push([old, v]);
  }
  out.problems = Array.from(new Set(out.problems));
  return out;
}

module.exports = { aliasOf, withAlias, ioAliases, checkTableEdit };
