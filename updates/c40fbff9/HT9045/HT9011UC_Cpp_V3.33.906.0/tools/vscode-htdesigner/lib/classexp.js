'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #17, BCB6 View > ClassExplorer): the classes of the C++ tree and their members --
// class -> its functions and fields, each to its line. A command with two pick lists, not a view: the side bar is not
// made fuller (EastSun「不要全擠在左邊」). Plain Node.

const fs = require('fs');
const path = require('path');
const { maskLive: mask } = require('./cppstub');   // (1009 second review (C++ nav #3): #if 0 copies not taken)

const CLASS_RE = /^[ \t]*(?:class|struct)[ \t]+(?:PACKAGE[ \t]+|__declspec\([^)]*\)[ \t]+)?([A-Za-z_]\w*)\s*(?:final\s*)?(?::\s*([^{;]*))?\s*\{/gm;

/** the classes defined in a header's text: [{ name, line, base }] (a forward declaration "class X;" is not one) */
function classesIn(text) {
  const t = String(text || '');
  const m0 = mask(t);
  const out = [];
  CLASS_RE.lastIndex = 0;
  let x;
  while ((x = CLASS_RE.exec(m0))) {
    const line = m0.slice(0, x.index).split('\n').length - 1;
    const base = (x[2] || '').replace(/\b(public|protected|private|virtual)\b/g, '').replace(/\s+/g, ' ').trim();
    out.push({ name: x[1], line, base });
  }
  return out;
}

/** every class of the tree's headers (not build*, third_party, vendor, dot / _ folders) -> [{ name, file, line, base }] */
function scanClasses(root, opts) {
  const max = (opts && opts.max) || 20000;
  const out = [];
  const walk = (d, depth) => {
    let ents = [];
    try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; }
    for (const e of ents) {
      if (out.length >= max) return;
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (depth < 6 && !/^(build|third_party$|vendor$|node_modules$|\.|_)/i.test(e.name)) walk(p, depth + 1); continue; }
      if (!/\.(h|hpp|hxx)$/i.test(e.name)) continue;
      let t = '';
      try { t = fs.readFileSync(p, 'utf8'); } catch (x) { continue; }
      if (!/\b(class|struct)\b/.test(t)) continue;
      for (const c of classesIn(t)) out.push(Object.assign({ file: p }, c));
    }
  };
  walk(root, 0);
  return out.sort((a, b) => a.name.localeCompare(b.name) || a.file.localeCompare(b.file));
}

/**
 * The members of class `cls` in a header's text: [{ name, kind: 'method' | 'field', line, sig, access }] -- the class's
 * own body only (a nested class's members are left out), in their order.
 */
function members(text, cls) {
  const t = String(text || '');
  const m0 = mask(t);
  const b = bodyOf(m0, cls);
  if (!b) return [];
  const starts = [0];
  for (let i = 0; i < t.length; i++) if (t[i] === '\n') starts.push(i + 1);
  const lineOf = off => { let lo = 0, hi = starts.length - 1; while (lo < hi) { const mid = (lo + hi + 1) >> 1; if (starts[mid] <= off) lo = mid; else hi = mid - 1; } return lo; };
  const out = [];
  let access = b.struct ? 'public' : 'private';
  let depth = 0, stmt = '', stmtAt = -1;
  for (let i = b.open + 1; i < b.close; i++) {
    const c = m0[i];
    if (c === '{') { if (depth === 0 && stmt.trim()) { take(stmt, stmtAt, true); stmt = ''; stmtAt = -1; } depth++; continue; }
    if (c === '}') { depth--; if (depth === 0) { stmt = ''; stmtAt = -1; } continue; }
    if (depth > 0) continue;
    if (c === ';') { take(stmt, stmtAt, false); stmt = ''; stmtAt = -1; continue; }
    if (c === ':' && /^\s*(public|protected|private|__published)\s*$/.test(stmt) && m0[i + 1] !== ':') { access = stmt.trim(); stmt = ''; stmtAt = -1; continue; }
    if (stmtAt < 0 && /\S/.test(c)) stmtAt = i;
    stmt += c;
  }
  return out;
  function take(s, at, body) {
    const one = s.replace(/\s+/g, ' ').trim();
    if (!one || /^(typedef|using|friend|enum|class|struct|union|template)\b/.test(one) || /^#/.test(one)) return;
    // (an "=" before the first "(": a field with its first value -- TEdit *X = new TEdit(); -- not a function)
    const eq = one.search(/(^|[^=!<>])=(?!=)/), par = one.indexOf('(');
    const fm = eq >= 0 && (par < 0 || eq < par) ? null : /([~A-Za-z_][\w]*|operator\s*\S+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*(const)?[^()]*$/.exec(one);
    if (fm && !/=\s*[^=]*\(/.test(one.slice(0, fm.index))) {
      out.push({ name: fm[1].replace(/\s+/g, ''), kind: 'method', line: lineOf(at), sig: one.replace(/\s*=\s*0$/, ' = 0').slice(0, 160), access });
      return;
    }
    if (body) return;
    // fields: the last name before [ ] / = / , (one declaration may name several)
    const decl = one.replace(/\[[^\]]*\]/g, '').replace(/=.*$/, '');
    const names = decl.split(',').map(x => (/([A-Za-z_]\w*)\s*$/.exec(x.trim()) || [])[1]).filter(Boolean);
    const typeOk = /^[A-Za-z_][\w:<>,\s*&]*\s[*&\s]*[A-Za-z_]\w*/.test(decl.trim());
    if (typeOk) for (const n of names) out.push({ name: n, kind: 'field', line: lineOf(at), sig: one.slice(0, 160), access });
  }
}

/** { open, close } of the first definition of `cls` (class or struct) in the masked text, or null */
function bodyOf(m0, cls) {
  CLASS_RE.lastIndex = 0;
  let x;
  while ((x = CLASS_RE.exec(m0))) {
    if (x[1] !== cls) continue;
    const open = x.index + x[0].length - 1, struct = /^\s*struct\b/.test(x[0]);
    let d = 0;
    for (let i = open; i < m0.length; i++) { if (m0[i] === '{') d++; else if (m0[i] === '}') { d--; if (!d) return { open, close: i, struct }; } }
    return null;
  }
  return null;
}

module.exports = { classesIn, scanClasses, members };
