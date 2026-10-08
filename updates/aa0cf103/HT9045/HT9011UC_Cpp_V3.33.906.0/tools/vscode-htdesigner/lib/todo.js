'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #18, BCB6 View > To-Do List / Ctrl+Shift+T Add To-Do Item): the TODO / FIXME / XXX
// comments of the C++ sources -- in a comment only (a "TODO" in a string is not one), // and /* */ alike. Plain Node.

const fs = require('fs');
const path = require('path');
// (at the start of a comment, or of one of its lines: "// TODO", "/* FIXME", " * XXX")
const RE = /^(?:\/\/+|\/\*+|[ \t]*\*?)[ \t]*(TODO|FIXME|XXX)\b[ \t:：-]*([^\r\n]*)/gm;

/** the comments of a C / C++ text: [[start, end)] -- strings and character literals skipped */
function comments(t) {
  const out = [];
  let i = 0;
  while (i < t.length) {
    const c = t[i], d = t[i + 1];
    if (c === '/' && d === '/') { const e = t.indexOf('\n', i); const j = e < 0 ? t.length : e; out.push([i, j]); i = j; continue; }
    if (c === '/' && d === '*') { const e = t.indexOf('*/', i + 2); const j = e < 0 ? t.length : e + 2; out.push([i, j]); i = j; continue; }
    if (c === '"' || c === "'") { let j = i + 1; while (j < t.length && t[j] !== c && t[j] !== '\n') j += t[j] === '\\' ? 2 : 1; i = j + 1; continue; }
    i++;
  }
  return out;
}

/** -> [{ line, col, kind, text }] (0-based line) */
function scan(text) {
  const t = String(text || '');
  if (!/TODO|FIXME|XXX/.test(t)) return [];
  const out = [];
  let starts = null;
  for (const [s, e] of comments(t)) {
    const body = t.slice(s, e);
    if (!/TODO|FIXME|XXX/.test(body)) continue;
    RE.lastIndex = 0;
    let x;
    while ((x = RE.exec(body))) {
      if (!x[0]) { RE.lastIndex++; continue; }
      const at = s + x.index + x[0].indexOf(x[1]);
      if (!starts) { starts = [0]; for (let i = 0; i < t.length; i++) if (t[i] === '\n') starts.push(i + 1); }
      let lo = 0, hi = starts.length - 1;
      while (lo < hi) { const mid = (lo + hi + 1) >> 1; if (starts[mid] <= at) lo = mid; else hi = mid - 1; }
      let txt = x[2].replace(/\s*\*\/.*$/, '').trim(), owner = '';
      const o = /^\(([^)]*)\)[ \t:：-]*/.exec(txt);   // (BCB's "TODO(owner): text")
      if (o) { owner = o[1].trim(); txt = txt.slice(o[0].length); }
      out.push({ line: lo, col: at - starts[lo], kind: x[1], owner, text: txt });
    }
  }
  return out;
}

/** every .cpp / .h / .hpp / .c under root (not build*, third_party, vendor, node_modules, dot / _ folders) -> [{ file, line, col, kind, text }] */
function scanTree(root, opts) {
  opts = opts || {};
  const max = opts.max || 5000;
  const out = [];
  const walk = (d, depth) => {
    let ents = [];
    try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; }
    for (const e of ents) {
      if (out.length >= max) return;
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (depth < 6 && !/^(build|third_party$|vendor$|node_modules$|\.|_)/i.test(e.name)) walk(p, depth + 1); continue; }
      if (!/\.(cpp|c|h|hpp)$/i.test(e.name)) continue;
      let t = '';
      try { t = fs.readFileSync(p, 'utf8'); } catch (x) { continue; }
      for (const r of scan(t)) out.push(Object.assign({ file: p }, r));
    }
  };
  walk(root, 0);
  return out;
}

/** The line Ctrl+Shift+T inserts above `lineText`: its indent + "// TODO: " (the caret goes after it) */
function addLine(lineText, who) {
  const ind = (/^[ \t]*/.exec(String(lineText || '')) || [''])[0];
  return ind + '// TODO' + (who ? '(' + who + ')' : '') + ': ';
}

module.exports = { scan, scanTree, addLine };
