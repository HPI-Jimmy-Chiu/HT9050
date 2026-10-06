'use strict';
// AI(W906-HTDESIGNER) 20260930: CSV 表格 -- a .csv shown as a table and edited like Excel
// (EastSun: "開啟CSV時可以像excel有表格可以編輯嗎?"). The machine's settings are CSV
// (D:\HT9045\system\Mot_Table.csv, IO_Table.csv ...), so an edit must never re-write the file:
// every change replaces only the text of the cells it changes -- line ends, quoting, the empty
// columns at the end of a line all stay as they were. Plain Node (no vscode).
//
// parse(text) -> { delim, eol, rows: [ { s, e, cells: [ { v, s, e, q } ] } ] }
//   s / e: offsets in the text (a row: its first field's start .. its last field's end, the line
//   end not included); q: the field was in quotes.

/** The separator: the one that splits the first lines into the most equal fields. */
function detectDelim(text) {
  const lines = String(text || '').split(/\r\n|\n|\r/).filter(l => l.length).slice(0, 20);
  let best = ',', score = -1;
  for (const d of [',', '\t', ';', '|']) {
    const counts = lines.map(l => l.split(d).length - 1);
    if (!counts.length || !counts[0]) continue;
    const same = counts.filter(c => c === counts[0]).length;
    const s = same * 1000 + counts[0];
    if (s > score) { score = s; best = d; }
  }
  return best;
}

function parse(text, delim) {
  const t = String(text || '');
  const d = delim || detectDelim(t);
  const crlf = (t.match(/\r\n/g) || []).length, lf = (t.match(/(^|[^\r])\n/g) || []).length;
  const eol = crlf >= lf && crlf ? '\r\n' : '\n';
  const rows = [];
  let i = 0;
  const n = t.length;
  // a text that ends with a line end has no empty last row
  while (i < n) {
    const cells = [];
    const rs = i;
    for (;;) {
      const fs = i;
      let v = '', q = false;
      if (t[i] === '"') {
        q = true;
        i++;
        for (;;) {
          if (i >= n) break;
          if (t[i] === '"') {
            if (t[i + 1] === '"') { v += '"'; i += 2; continue; }
            i++;
            break;
          }
          v += t[i++];
        }
        // (text after the closing quote up to the separator belongs to the field, like Excel)
        while (i < n && t[i] !== d && t[i] !== '\r' && t[i] !== '\n') v += t[i++];
      } else {
        while (i < n && t[i] !== d && t[i] !== '\r' && t[i] !== '\n') v += t[i++];
      }
      cells.push({ v, s: fs, e: i, q });
      if (i < n && t[i] === d) { i++; continue; }
      break;
    }
    const re = i;
    if (t[i] === '\r' && t[i + 1] === '\n') i += 2;
    else if (t[i] === '\r' || t[i] === '\n') i++;
    rows.push({ s: rs, e: re, cells });
  }
  return { delim: d, eol, rows };
}

/** How a value is written into a field: quoted when it was, or when it has to be. */
function fieldText(value, delim, wasQuoted) {
  const v = String(value == null ? '' : value);
  const must = v.indexOf(delim) >= 0 || /["\r\n]/.test(v);
  return must || wasQuoted ? '"' + v.replace(/"/g, '""') + '"' : v;
}

/**
 * The text replacements for setting cells: changes = [{ r, c, v }] (rows / columns from 0).
 * A cell past the end of its row gets the separators it needs; a row past the end of the text
 * is added (with the file's line end). Unchanged values give no replacement.
 * -> [{ s, e, text }] sorted, never overlapping (the ones for one row end are merged).
 */
function cellEdits(p, changes) {
  const out = [];
  const d = p.delim;
  const byRowEnd = new Map();   // appended cells of one row: one insert at its end
  let tailRows = [];            // rows after the last one
  const lastRow = p.rows.length - 1;
  for (const ch of changes || []) {
    const r = ch.r | 0, c = ch.c | 0;
    if (r < 0 || c < 0) continue;
    const v = String(ch.v == null ? '' : ch.v);
    if (r <= lastRow) {
      const row = p.rows[r];
      const cell = row.cells[c];
      if (cell) {
        if (cell.v === v) continue;
        out.push({ s: cell.s, e: cell.e, text: fieldText(v, d, cell.q) });
      } else {
        if (!byRowEnd.has(r)) byRowEnd.set(r, new Map());
        byRowEnd.get(r).set(c, v);
      }
    } else {
      const k = r - lastRow - 1;
      while (tailRows.length <= k) tailRows.push(new Map());
      tailRows[k].set(c, v);
    }
  }
  for (const [r, cols] of byRowEnd) {
    const row = p.rows[r];
    const max = Math.max(...cols.keys());
    let text = '';
    for (let c = row.cells.length; c <= max; c++) text += d + (cols.has(c) ? fieldText(cols.get(c), d, false) : '');
    out.push({ s: row.e, e: row.e, text });
  }
  if (tailRows.length) {
    const width = p.rows.length ? p.rows[0].cells.length : 1;
    const lines = tailRows.map(cols => {
      const max = Math.max(width - 1, cols.size ? Math.max(...cols.keys()) : 0);
      const f = [];
      for (let c = 0; c <= max; c++) f.push(cols.has(c) ? fieldText(cols.get(c), d, false) : '');
      return f.join(d);
    });
    const endsWithEol = p.textLength > 0 && p.lastEol;
    const at = p.textLength || 0;
    out.push({ s: at, e: at, text: (endsWithEol || !p.rows.length ? '' : p.eol) + lines.join(p.eol) + (endsWithEol ? p.eol : '') });
  }
  out.sort((a, b) => a.s - b.s || a.e - b.e);
  return out;
}

/**
 * 1006 (Excel's Sort A to Z / Z to A): rows from..last ordered by column `col` -- numbers by value first, then text A-Z,
 * the empty ones last (desc: the reverse, the empty ones still last); equal ones keep their order. Each row's own text
 * is moved as it is (its quoting, its other cells untouched). -> [{ s, e, text }] (one edit) or [] (already in order)
 */
function sortRows(p, text, from, col, desc) {
  const t = String(text || '');
  const rows = p.rows;
  const a = Math.max(0, from | 0);
  if (rows.length - a < 2) return [];
  const isN = v => v.trim() !== '' && isFinite(+v);
  const keyOf = r => { const c = r.cells[col]; return c ? c.v : ''; };
  const idx = [];
  for (let i = a; i < rows.length; i++) idx.push(i);
  const cmp = (i, j) => {
    const x = keyOf(rows[i]), y = keyOf(rows[j]);
    if ((x === '') !== (y === '')) return x === '' ? 1 : -1;
    let d;
    const xn = isN(x), yn = isN(y);
    if (xn && yn) d = (+x) - (+y);
    else if (xn !== yn) d = xn ? -1 : 1;
    else d = x.localeCompare(y);
    if (desc && x !== '' && y !== '') d = -d;
    return d || i - j;
  };
  const order = idx.slice().sort(cmp);
  if (order.every((v, k) => v === idx[k])) return [];
  const s = rows[a].s, e = rows[rows.length - 1].e;
  return [{ s, e, text: order.map(i => t.slice(rows[i].s, rows[i].e)).join(p.eol) }];
}

/** parse() plus what cellEdits() needs to add rows at the end. */
function parseDoc(text, delim) {
  const p = parse(text, delim);
  const t = String(text || '');
  p.textLength = t.length;
  p.lastEol = /(\r\n|\n|\r)$/.test(t);
  return p;
}

/** Insert `count` empty rows before row r (r = rows.length: at the end). -> [{ s, e, text }] */
function insertRows(p, r, count) {
  const n = Math.max(1, count | 0);
  const width = p.rows.length ? Math.max(1, (p.rows[Math.min(r, p.rows.length - 1)] || p.rows[0]).cells.length) : 1;
  const blank = new Array(width).join(p.delim);
  const lines = new Array(n).fill(blank);
  if (r < p.rows.length) return [{ s: p.rows[r].s, e: p.rows[r].s, text: lines.join(p.eol) + p.eol }];
  const at = p.textLength || 0;
  return [{ s: at, e: at, text: (p.lastEol || !p.rows.length ? '' : p.eol) + lines.join(p.eol) + (p.lastEol ? p.eol : '') }];
}

/** Delete rows r .. r+count-1 (with their line ends). -> [{ s, e, text: '' }] */
function deleteRows(p, r, count) {
  const a = Math.max(0, r | 0), b = Math.min(p.rows.length - 1, a + Math.max(1, count | 0) - 1);
  if (a > b) return [];
  // rows after them: the lines go with their line ends
  if (b + 1 < p.rows.length) return [{ s: p.rows[a].s, e: p.rows[b + 1].s, text: '' }];
  // the last rows: from the end of the row before them (the file's last line end, if any, stays)
  if (a > 0) return [{ s: p.rows[a - 1].e, e: p.rows[b].e, text: '' }];
  return [{ s: 0, e: p.textLength, text: '' }];
}

/** Rows x columns as text for the clipboard (Excel's: tab between cells, CRLF between rows). */
function toTsv(grid) {
  return grid.map(r => r.map(v => (/[\t\r\n"]/.test(v) ? '"' + String(v).replace(/"/g, '""') + '"' : String(v))).join('\t')).join('\r\n');
}

/** Clipboard text from Excel (or anything tab / line separated) -> rows x columns. */
function fromTsv(text) {
  const t = String(text || '').replace(/(\r\n|\n|\r)$/, '');
  if (!t.length) return [['']];
  return parse(t, '\t').rows.map(r => r.cells.map(c => c.v));
}

/**
 * A paste, as Excel does it (AI 20261001: "一整欄改成同一個值" pasted only the first cell): the clipboard block goes
 * in from the selection's top-left; a selection bigger than the block that the block fits a whole number of times
 * (one cell into a whole column, a 1x2 into a 3x4...) is filled with it, repeated. A cut (Ctrl+X) block: its cells
 * outside where it lands are emptied (only cells that exist). One list = one edit = one Ctrl+Z.
 * sel / cut: { r0, c0, r1, c1 }. -> { changes: [{ r, c, v }], rect: { r0, c0, r1, c1 }, tiled }
 */
function pasteChanges(p, grid, sel, cut) {
  const g = grid && grid.length ? grid : [['']];
  const gh = g.length, gw = Math.max(1, ...g.map(x => x.length));
  const h = sel.r1 - sel.r0 + 1, w = sel.c1 - sel.c0 + 1;
  const tiled = (h > gh || w > gw) && h % gh === 0 && w % gw === 0;
  const H = tiled ? h : gh, W = tiled ? w : gw;
  const changes = [];
  for (let i = 0; i < H; i++) {
    const row = g[i % gh];
    for (let j = 0; j < W; j++) if (j % gw < row.length) changes.push({ r: sel.r0 + i, c: sel.c0 + j, v: row[j % gw] });
  }
  const rect = { r0: sel.r0, c0: sel.c0, r1: sel.r0 + H - 1, c1: sel.c0 + W - 1 };
  if (cut) {
    for (let r = cut.r0; r <= cut.r1; r++) for (let c = cut.c0; c <= cut.c1; c++) {
      if (r >= rect.r0 && r <= rect.r1 && c >= rect.c0 && c <= rect.c1) continue;
      const pr = p && p.rows[r];
      if (pr && c < pr.cells.length && pr.cells[c].v !== '') changes.push({ r, c, v: '' });
    }
  }
  return { changes, rect, tiled };
}

/**
 * Could the text VS Code decoded be written back without damage? A file that is not UTF-8 (a Big5
 * one) comes in with U+FFFD in place of what it could not read: saving it would destroy those bytes.
 */
function decodeSafe(bytes, text) {
  if (!bytes) return { ok: true };
  const hasBad = String(text || '').indexOf('\uFFFD') >= 0;
  if (!hasBad) return { ok: true };
  let utf8 = true;
  try { new TextDecoder('utf-8', { fatal: true }).decode(bytes); } catch (e) { utf8 = false; }
  return utf8 ? { ok: true } : { ok: false, why: '這個檔案不是 UTF-8（多半是 Big5），VS Code 用 UTF-8 讀，看不懂的字變成了 �。這樣存檔會把那些字毀掉，所以不能編輯。要編輯：按右下角的「UTF-8」→「以編碼重新開啟」→ 選「Traditional Chinese (Big5)」。' };
}

module.exports = { detectDelim, parse, parseDoc, fieldText, cellEdits, insertRows, deleteRows, sortRows, toTsv, fromTsv, pasteChanges, decodeSafe };
