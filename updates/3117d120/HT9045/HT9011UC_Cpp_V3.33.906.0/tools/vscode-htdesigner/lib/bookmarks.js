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
function shift(slots, file, startLine, endLine, newLines) {
  const s = {};
  const delta = (newLines - 1) - (endLine - startLine);
  for (const k of Object.keys(slots || {})) {
    const b = slots[k];
    if (!sameFile(b.file, file) || b.line < startLine || delta === 0) { s[k] = b; continue; }
    if (b.line > endLine) s[k] = { file: b.file, line: b.line + delta };
    else s[k] = { file: b.file, line: Math.min(b.line, startLine + Math.max(0, newLines - 1)) };
  }
  return s;
}

module.exports = { toggle, inFile, shift, sameFile };
