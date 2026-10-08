'use strict';
// AI(W906-HTDESIGNER) 20261008 (feature gap #5, Visual Studio's Solution Explorer: Add > New Item puts a .cpp in the
// project): the tree's CMakeLists.txt lists every source of a target inside its add_library( ... ) / add_executable( ... ).
//   targets(text)            -> [{ name, kind, start, end }]   (the call's own text range, comments skipped for the parens)
//   targetsFor(text, relDir) -> the targets that already have sources of that folder, most first:
//                               [{ name, count, insertAt, indent }]  (insertAt = after the last line of that folder)
//   insertSource(text, t, rel, note) -> { at, text }   the line to add
//   refsOf(text, rel)        -> [{ s, e }]   where a source path is written (to rename / warn on delete)
// Plain Node (no vscode).

/** add_library / add_executable calls: their name and range (a ')' inside a comment or a string does not close one). */
function targets(text) {
  const t = String(text || '');
  const out = [];
  const re = /^[ \t]*(add_library|add_executable)\s*\(\s*([A-Za-z_][\w.-]*)/gm;
  let m;
  while ((m = re.exec(t))) {
    let i = t.indexOf('(', m.index), depth = 0, end = -1;
    for (; i < t.length; i++) {
      const c = t[i];
      if (c === '#') { const nl = t.indexOf('\n', i); i = nl < 0 ? t.length : nl; continue; }
      if (c === '"') { const q = t.indexOf('"', i + 1); i = q < 0 ? t.length : q; continue; }
      if (c === '(') depth++;
      else if (c === ')' && !--depth) { end = i + 1; break; }
    }
    if (end < 0) continue;
    out.push({ name: m[2], kind: m[1], start: m.index, end });
    re.lastIndex = end;
  }
  return out;
}

const escRe = s => String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

/** the targets with sources directly in relDir ('' = the tree's root), most sources first */
function targetsFor(text, relDir) {
  const t = String(text || '');
  const dir = String(relDir || '').replace(/\\/g, '/').replace(/\/$/, '');
  const lineRe = new RegExp('^([ \\t]*)' + (dir ? escRe(dir) + '/' : '') + '[^/\\s#()]+\\.(?:c|cc|cpp|cxx)\\b[^\\r\\n]*', 'gmi');
  const out = [];
  for (const tg of targets(t)) {
    const body = t.slice(tg.start, tg.end);
    let m, count = 0, last = null;
    lineRe.lastIndex = 0;
    while ((m = lineRe.exec(body))) { count++; last = m; }
    if (!count) continue;
    out.push({ name: tg.name, count, insertAt: tg.start + last.index + last[0].length, indent: last[1] });
  }
  return out.sort((a, b) => b.count - a.count);
}

/** the line that adds `rel` (path from the tree, '/') after the target's last source of that folder */
function insertSource(text, tg, rel, note) {
  const eol = /\r\n/.test(String(text || '')) ? '\r\n' : '\n';
  return { at: tg.insertAt, text: eol + tg.indent + String(rel).replace(/\\/g, '/') + (note ? '    # ' + note : '') };
}

/** where `rel` is written as a source (a whole word, not part of a longer path) */
function refsOf(text, rel) {
  const t = String(text || '');
  const r = String(rel).replace(/\\/g, '/');
  const re = new RegExp('(^|[\\s("])' + escRe(r) + '(?=[\\s)"#]|$)', 'gmi');
  const out = [];
  let m;
  while ((m = re.exec(t))) out.push({ s: m.index + m[1].length, e: m.index + m[1].length + r.length });
  return out;
}

module.exports = { targets, targetsFor, insertSource, refsOf };
