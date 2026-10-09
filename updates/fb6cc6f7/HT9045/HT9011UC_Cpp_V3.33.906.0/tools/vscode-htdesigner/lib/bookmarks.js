'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #11, BCB6's bookmarks: Ctrl+Shift+0..9 set / clear, Ctrl+0..9 go): the numbered
// bookmarks of the C++ sources -- ten slots, each one file and line; setting a slot that is already on that line clears
// it (BCB toggles), a slot moves when set elsewhere. Lines follow edits made above them. Plain Node (no vscode).

/** slots: { '0'..'9': { file, line } } -> the new slots after Ctrl+Shift+n on file:line */
function toggle(slots, n, file, line) {
  const s = Object.assign({}, slots || {});
  const k = String(n);
  const cur = s[k];
  if (cur && sameFile(cur.file, file) && cur.line === line) { delete s[k]; return s; }
  // (one number on a line: another slot on that same line goes)
  for (const j of Object.keys(s)) if (j !== k && sameFile(s[j].file, file) && s[j].line === line) delete s[j];
  s[k] = { file, line };
  return s;
}

const sameFile = (a, b) => String(a || '').toLowerCase().replace(/\//g, '\\') === String(b || '').toLowerCase().replace(/\//g, '\\');

/** the slots of one file: [{ n, line }] */
function inFile(slots, file) {
  return Object.keys(slots || {}).filter(k => sameFile(slots[k].file, file)).map(k => ({ n: k, line: slots[k].line })).sort((a, b) => a.line - b.line);
}

/**
 * An edit of `file` -- lines startLine..endLine (0-based, inclusive, the old text) replaced by text of `newLines` lines --
 * applied to the slots: below it they move by the difference; inside a deleted range they stay at its start.
 */
function shift(slots, file, startLine, endLine, newLines, endChar) {
  const s = {};
  const delta = (newLines - 1) - (endLine - startLine);
  // (1008 review: the same object back when no bookmark moves -- every key typed in any file saved the bookmarks and
  //  repainted them all)
  let moved = false;
  for (const k of Object.keys(slots || {})) {
    const b = slots[k];
    if (!sameFile(b.file, file) || b.line < startLine || delta === 0) { s[k] = b; continue; }
    // (1009 review (C++ nav #4): an edit ending at column 0 of the bookmark's line (Enter / a paste at the line's start)
    //  leaves that line's text whole, pushed down -- the bookmark goes with it; it stayed on the new empty line)
    const below = b.line > endLine || (b.line === endLine && endChar === 0 && delta > 0);
    const line = below ? b.line + delta : Math.min(b.line, startLine + Math.max(0, newLines - 1));
    if (line === b.line) { s[k] = b; continue; }
    s[k] = Object.assign({}, b, { line });   // (1009 regression review #2: its line's text kept)
    moved = true;
  }
  return moved ? s : slots;
}

module.exports = { toggle, inFile, shift, sameFile };
