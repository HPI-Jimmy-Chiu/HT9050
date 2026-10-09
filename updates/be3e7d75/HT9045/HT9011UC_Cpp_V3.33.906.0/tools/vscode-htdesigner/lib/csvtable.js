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

/**
 * The separator: the one that splits the first lines into the most equal numbers of fields -- counted OUTSIDE quotes
 * (a quoted "a;b" or "\t" is a value, not a separator), over all the lines sampled (1006 audit: only the first line
 * used to count -- a ";" typed into row 1, or row 1 deleted / sorted away, switched a file to another separator and
 * every later edit wrote the wrong bytes). The table decides it once per open (CsvTableEditor), not on every parse.
 */
function detectDelim(text) {
  const t = String(text || '');
  const lines = [];
  let cur = { ',': 0, '\t': 0, ';': 0, '|': 0 }, any = false, inQ = false;
  for (let i = 0; i < t.length && lines.length < 50; i++) {
    const ch = t[i];
    if (ch === '"') { inQ = !inQ; any = true; continue; }
    if (!inQ && (ch === '\n' || ch === '\r')) {
      if (any) lines.push(cur);
      cur = { ',': 0, '\t': 0, ';': 0, '|': 0 }; any = false;
      if (ch === '\r' && t[i + 1] === '\n') i++;
      continue;
    }
    any = true;
    if (!inQ && ch in cur) cur[ch]++;
  }
  if (any && lines.length < 50) lines.push(cur);
  let best = ',', score = -1;
  for (const d of [',', '\t', ';', '|']) {
    const counts = lines.map(c => c[d]).filter(c => c > 0);
    if (!counts.length) continue;
    // the commonest count among the lines that have it, and how many lines have exactly that
    const freq = new Map();
    for (const c of counts) freq.set(c, (freq.get(c) || 0) + 1);
    let mode = 0, modeN = 0;
    for (const [c, n] of freq) if (n > modeN || (n === modeN && c > mode)) { mode = c; modeN = n; }
    const sc = modeN * 1000 + counts.length * 10 + Math.min(mode, 9);
    if (sc > score) { score = sc; best = d; }
  }
  return best;
}

function parse(text, delim) {
  const t = String(text || '');
  const d = delim || detectDelim(t);
  const dc = d.charCodeAt(0);
  const crlf = (t.match(/\r\n/g) || []).length, lf = (t.match(/(^|[^\r])\n/g) || []).length;
  const eol = crlf >= lf && crlf ? '\r\n' : '\n';
  const rows = [];
  let quoteTrouble = false;
  let i = 0;
  const n = t.length;
  // a text that ends with a line end has no empty last row
  while (i < n) {
    const cells = [];
    const rs = i;
    for (;;) {
      const fs = i;
      let v = '', q = false;
      const plainEnd = j => { while (j < n) { const ch = t.charCodeAt(j); if (ch === dc || ch === 13 || ch === 10) break; j++; } return j; };
      if (t[i] === '"') {
        q = true;
        i++;
        for (;;) {
          if (i >= n) break;
          const k = t.indexOf('"', i);
          if (k < 0) { v += t.slice(i); i = n; quoteTrouble = quoteTrouble || 'unclosed'; break; }
          v += t.slice(i, k);
          if (t[k + 1] === '"') { v += '"'; i = k + 2; continue; }
          i = k + 1;
          break;
        }
        // (text after the closing quote up to the separator belongs to the field, like Excel)
        const j2 = plainEnd(i);
        v += t.slice(i, j2); i = j2;
      } else {
        const j1 = plainEnd(i);
        v = t.slice(i, j1); i = j1;
      }
      if (q && /[\r\n]/.test(v)) quoteTrouble = quoteTrouble || 'newline';
      cells.push({ v, s: fs, e: i, q });
      if (i < n && t[i] === d) { i++; continue; }
      break;
    }
    const re = i;
    if (t[i] === '\r' && t[i + 1] === '\n') i += 2;
    else if (t[i] === '\r' || t[i] === '\n') i++;
    rows.push({ s: rs, e: re, cells });
  }
  // (1006 audit: a value that needs no quotes keeps quotes only in a file that quotes such values -- the cell's last
  //  state used to decide: once "a,b" had been in a cell, putting back IOType wrote "IOType")
  const needs = v => v.indexOf(d) >= 0 || /["\r\n]/.test(v);
  let quotesPlain = false;
  for (let r = 0; r < rows.length && !quotesPlain; r++) for (const c of rows[r].cells) if (c.q && !needs(c.v)) { quotesPlain = true; break; }
  // (1008 review: quoteTrouble = a quote this table reads otherwise than the machine -- one never closed (the rest of the
  //  file one cell) or a line break inside quotes: the machine (BCB6 LoadFromFile + CommaText) splits LINES first, so the
  //  rows seen here are not its rows. The table is read-only then, said; the text editor fixes it)
  return { delim: d, eol, rows, quotesPlain, quoteTrouble };
}

/** How a value is written into a field: quoted when it was, or when it has to be. */
function fieldText(value, delim, wasQuoted) {
  // (1008 review: a line break cannot be in a value -- the machine splits the file into lines FIRST (LoadFromFile), so a
  //  quoted "a\r\nb" became two rows; it goes as a blank (the caller says so: hasLineBreak))
  const v = String(value == null ? '' : value).replace(/\r\n|\r|\n/g, ' ');
  // (1008 review: quoted the way the MACHINE reads it -- BCB6 CommaText (vclcompat/TStringList.cpp, pinned by a real BCB6
  //  run) ends an unquoted item at ANY char <= ' ': "Load Y" typed as an IO_Table Alias was read as two items and every
  //  column after it moved one to the right. Quoted when it has a char <= ' ', a quote or the separator -- as BCB writes it)
  const must = v.indexOf(delim) >= 0 || /[\x00-\x20"]/.test(v);
  return must || wasQuoted ? '"' + v.replace(/"/g, '""') + '"' : v;
}
/** a value that will lose its line breaks when written (fieldText) */
function hasLineBreak(value) { return /[\r\n]/.test(String(value == null ? '' : value)); }

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
    // (1007 audit D2: a line break inside a value in the file's own line end -- VS Code turned a pasted LF into CRLF
    //  anyway, so what was written was not what the table showed)
    const v = String(ch.v == null ? '' : ch.v).replace(/\r\n|\r|\n/g, p.eol || '\r\n');
    if (r <= lastRow) {
      const row = p.rows[r];
      const cell = row.cells[c];
      if (cell) {
        if (cell.v === v) continue;
        out.push({ s: cell.s, e: cell.e, text: fieldText(v, d, cell.q && p.quotesPlain !== false) });
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
    // (1007 audit D7: a file that quotes its plain values gets its new ones quoted too)
    for (let c = row.cells.length; c <= max; c++) text += d + (cols.has(c) ? fieldText(cols.get(c), d, p.quotesPlain === true) : '');
    out.push({ s: row.e, e: row.e, text });
  }
  if (tailRows.length) {
    const width = p.rows.length ? p.rows[0].cells.length : 1;
    const lines = tailRows.map(cols => {
      const max = Math.max(width - 1, cols.size ? Math.max(...cols.keys()) : 0);
      const f = [];
      for (let c = 0; c <= max; c++) f.push(cols.has(c) ? fieldText(cols.get(c), d, p.quotesPlain === true) : '');
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
  // (1009 review (csv #9): a number written as one -- "0x10" / "1e1" are text, not 16 / 10)
  const isN = v => /^\s*[-+]?(\d+(\.\d*)?|\.\d+)\s*$/.test(v);
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
  // (1007 audit D3: empty rows sort to the end -- in a file without a final line end the last one read as that line end and
  //  a row was lost; it gets its own)
  const lastEmpty = rows[order[order.length - 1]].s === rows[order[order.length - 1]].e;
  return [{ s, e, text: order.map(i => t.slice(rows[i].s, rows[i].e)).join(p.eol) + (lastEmpty && !p.lastEol ? p.eol : '') }];
}

/**
 * 1006 (Excel's Insert / Delete sheet columns): `count` empty fields before column c in every row that reaches it (a row
 * that ends before c is left as it is; c = its length = at its end). -> [{ s, e, text }] (one edit)
 */
function insertCols(p, c, count) {
  const n = Math.max(1, count | 0), col = Math.max(0, c | 0), d = p.delim, out = [];
  for (const row of p.rows) {
    const cs = row.cells;
    // (1008 review: an empty line is left as it is -- it became "," : a line not touched rewritten, a row of empty fields)
    if (cs.length === 1 && cs[0].s === cs[0].e) continue;
    if (col < cs.length) out.push({ s: cs[col].s, e: cs[col].s, text: d.repeat(n) });
    else if (col === cs.length && cs.length) out.push({ s: row.e, e: row.e, text: d.repeat(n) });
  }
  return out;
}
/** Columns c0..c1 out of every row (with their delimiters); a row shorter than c0 is left as it is. -> [{ s, e, text: '' }] */
function deleteCols(p, c0, c1) {
  const a = Math.max(0, c0 | 0), b = Math.max(a, c1 | 0), out = [];
  for (const row of p.rows) {
    const cs = row.cells;
    if (a >= cs.length) continue;
    const last = Math.min(b, cs.length - 1);
    if (last + 1 < cs.length) out.push({ s: cs[a].s, e: cs[last + 1].s, text: '' });   // the fields and the delimiter after them
    else if (a > 0) out.push({ s: cs[a - 1].e, e: cs[last].e, text: '' });              // up to the end: from the delimiter before
    else out.push({ s: cs[0].s, e: cs[last].e, text: '' });                              // every field: an empty line stays
  }
  return out;
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
  // (1007 audit D3: an empty new last row -- a 1-column file, or an empty file -- would read as the file's final line end:
  //  nothing added. It gets a line end of its own)
  const eolAfter = p.lastEol || !p.rows.length || blank === '' ? (p.eol || '\r\n') : '';
  return [{ s: at, e: at, text: (p.lastEol || !p.rows.length ? '' : p.eol) + lines.join(p.eol) + eolAfter }];
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
  // (1008 review: every row ends with CRLF, as Excel writes it -- without the last one, a block whose last cells are empty
  //  ("5", "", "") came back as one row and was pasted as 5,5,5)
  return grid.map(r => r.map(v => (/[\t\r\n"]/.test(v) ? '"' + String(v).replace(/"/g, '""') + '"' : String(v))).join('\t') + '\r\n').join('');
}

/** Clipboard text from Excel (or anything tab / line separated) -> rows x columns. */
function fromTsv(text) {
  const t = String(text || '').replace(/(\r\n|\n|\r)$/, '');
  if (!t.length) return [['']];
  // (1007 audit D4: Excel quotes a field only when it has to, and then the closing quote is followed by a tab, a line end or
  //  the end. Text from Notepad / a log with a cell that merely starts with " ("5 in) swallowed the rest of the clipboard
  //  into one cell: such text is read line by line, cell by cell, as it is)
  if (!quotedOk(t)) return t.split(/\r\n|\n|\r/).map(l => l.split('\t'));
  const rows = parse(t, '\t').rows.map(r => r.cells.map(c => c.v));
  // (1008 review: parse takes a final line break as the end of the last row, not one more empty row -- the clipboard's own
  //  last break is gone already, so one still there IS an empty last row)
  if (/(\r\n|\n|\r)$/.test(t)) rows.push(['']);
  return rows;
}
/** every field that starts with " is a whole quoted field (its closing quote followed by a tab, a line end or the end) */
function quotedOk(t) {
  let i = 0;
  const n = t.length;
  while (i <= n) {
    if (t[i] === '"') {
      let j = i + 1;
      for (;;) {
        const q = t.indexOf('"', j);
        if (q < 0) return false;
        if (t[q + 1] === '"') { j = q + 2; continue; }
        const nx = t[q + 1];
        if (!(nx === undefined || nx === '\t' || nx === '\r' || nx === '\n')) return false;
        i = q + 1;
        break;
      }
    } else {
      while (i < n && t[i] !== '\t' && t[i] !== '\r' && t[i] !== '\n') i++;
    }
    if (i >= n) return true;
    if (t[i] === '\r' && t[i + 1] === '\n') i += 2; else i++;
  }
  return true;
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
  const gh = g.length, gw = g.reduce((m, x) => Math.max(m, x.length), 1);   // (no spread: 200k lines overflowed the stack)
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
const REOPEN_UTF8 = '這個檔案是 UTF-8，但 VS Code 用別的編碼（多半是 Big5）讀，中文變成了亂碼。這樣存檔會把那些字毀掉，所以不能編輯。要編輯：按右下角的編碼 →「以編碼重新開啟」→ 選「UTF-8」。';
// (1009 regression review #7: the machine's own folders -- not the development repos that have "HT9045" in their path)
const MACHINE = /^[A-Za-z]:[\\/]((HT9045|GPIB9045)[\\/](system|config|IniData)|HP90\d\d)[\\/]/i;
/**
 * 1009 review (csv #2 #4 #8): `enc` = the encoding VS Code read the document with (TextDocument.encoding: 'utf8',
 * 'utf8bom', 'cp950', 'big5hkscs', 'utf16le' ...; undefined on an older VS Code = guessed from the bytes as before);
 * `file` = its path (a machine table: BCB6 reads it in cp950). -> { ok, why?, big5?, asciiOnly?, enc? }
 */
function decodeSafe(bytes, text, enc, file) {
  if (!bytes) return { ok: true };
  const tx = String(text || '');
  const hasBad = tx.indexOf('\uFFFD') >= 0;
  let utf8 = true, u8 = null;
  try { u8 = new TextDecoder('utf-8', { fatal: true }).decode(bytes); } catch (e) { utf8 = false; }
  if (u8 !== null && u8.charCodeAt(0) === 0xFEFF) u8 = u8.slice(1);
  const e = String(enc || '').toLowerCase();
  // (a machine table with no character above 0x7f yet: whatever is typed in must be what the machine (cp950) reads)
  const ascii = !bytes.some(b => b > 0x7f);
  const machine = !!(file && MACHINE.test(String(file)));
  if (e) {
    if (/^utf8/.test(e)) {
      if (!utf8 && hasBad) return { ok: false, why: '這個檔案不是 UTF-8（多半是 Big5），VS Code 用 UTF-8 讀，看不懂的字變成了 �。這樣存檔會把那些字毀掉，所以不能編輯。要編輯：按右下角的「UTF-8」→「以編碼重新開啟」→ 選「Traditional Chinese (Big5)」。' };
      return ascii && machine ? { ok: true, asciiOnly: true, enc: e } : { ok: true, enc: e };
    }
    // (read in another encoding while the bytes are UTF-8 with non-ASCII: the text is mojibake)
    if (utf8 && !ascii && u8 !== tx) return { ok: false, why: REOPEN_UTF8 };
    if (hasBad) return { ok: false, why: '這個檔案用「' + enc + '」讀，有看不懂的字變成了 �。這樣存檔會把那些字毀掉，所以不能編輯。請用「以編碼重新開啟」選對的編碼。' };
    if (/^(cp950|big5)$/.test(e)) return { ok: true, big5: true, enc: e };
    return { ok: true, enc: e };
  }
  // (no TextDocument.encoding: guessed. 1009 review (csv #2): bytes that are valid UTF-8 but a text that is not their
  //  UTF-8 reading = VS Code read them in another encoding (files.encoding cp950 in D:\HT9045\.vscode))
  if (utf8 && !ascii && u8 !== tx) return { ok: false, why: REOPEN_UTF8 };
  // (1009: read without a single U+FFFD from bytes that are not UTF-8 = VS Code opened it in Big5 (reopened with that
  //  encoding, or files.encoding): it is saved in Big5 too -- big5 tells the editor to check what is typed in)
  if (!hasBad) return utf8 ? (ascii && machine ? { ok: true, asciiOnly: true } : { ok: true }) : { ok: true, big5: true };
  return utf8 ? { ok: true } : { ok: false, why: '這個檔案不是 UTF-8（多半是 Big5），VS Code 用 UTF-8 讀，看不懂的字變成了 �。這樣存檔會把那些字毀掉，所以不能編輯。要編輯：按右下角的「UTF-8」→「以編碼重新開啟」→ 選「Traditional Chinese (Big5)」。' };
}

/**
 * 1009: the characters of `text` Big5 has no code for -- VS Code writes "?" in their place when it saves a Big5 file
 * (a Mot_Table / IO table opened as Big5 and a name typed with such a character: the machine read "?"). The set is
 * every character the Big5 decoder gives for a two-byte code, built once. -> [unique characters] (empty = all fine)
 */
let BIG5SET = null;
function big5Missing(text) {
  const t = String(text || '');
  if (!/[^\x00-\x7f]/.test(t)) return [];
  if (!BIG5SET) {
    BIG5SET = new Set();
    const dec = new TextDecoder('big5');
    const b = new Uint8Array(2);
    // (1009 review (csv #5): Windows' cp950 -- what VS Code writes -- not the WHATWG decoder's whole table: its HKSCS
    //  rows (lead 0x81-0xA0, 0xFA-0xFE) and 0xC6A1-0xC8FE (circled numbers, kana, Cyrillic) are "?" in cp950)
    for (let hi = 0xa1; hi <= 0xf9; hi++) {
      for (let lo = 0x40; lo <= 0xfe; lo++) {
        if (lo > 0x7e && lo < 0xa1) continue;
        if ((hi === 0xc6 && lo >= 0xa1) || hi === 0xc7 || hi === 0xc8) continue;
        b[0] = hi; b[1] = lo;
        const ch = dec.decode(b);
        if (ch && ch !== '\uFFFD') BIG5SET.add(ch);
      }
    }
  }
  const out = [];
  for (const ch of t) if (ch.charCodeAt(0) > 0x7f && !BIG5SET.has(ch) && out.indexOf(ch) < 0) out.push(ch);
  return out;
}

module.exports = { big5Missing, detectDelim, parse, parseDoc, fieldText, hasLineBreak, cellEdits, insertRows, deleteRows, sortRows, insertCols, deleteCols, toTsv, fromTsv, pasteChanges, decodeSafe };
