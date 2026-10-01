'use strict';
// AI(W906-HTDESIGNER) 20261001 (0.140, EastSun's screenshot of Visual Studio's navigation bar: 專案 ▾ | 類別 ▾ | 成員 ▾):
// the classes and members of a C++ file, for VS Code's breadcrumbs (its navigation bar above the editor) and the
// 類別 / 成員 pickers. A light reading of the text, not a compiler: comments and strings blanked (cppstub.mask), then
//   class / struct NAME [: bases] { ... }        -> a class, its methods and fields (one level deep)
//   [type] NAME::member(...) [const] { ... }     -> a member defined outside its class (a .cpp), grouped by class
//   [type] name(...) { ... } at the top level    -> a free function
// Offsets are in the text given (lines / columns are made by the caller from them). Plain Node.
const { mask } = require('./cppstub');

const KW = new Set(['if', 'for', 'while', 'switch', 'return', 'sizeof', 'catch', 'else', 'do', 'case', 'new', 'delete', 'throw', 'defined', 'decltype', 'alignof', 'static_assert']);

/** The offset of the brace that closes the one at `open` (masked text), or -1. */
function closeOf(m, open) {
  let depth = 0;
  for (let i = open; i < m.length; i++) {
    const c = m[i];
    if (c === '{') depth++;
    else if (c === '}') { depth--; if (!depth) return i; }
  }
  return -1;
}

/** Blank #... lines too (macros, #if 0 bodies are still read -- a light reading). */
function maskPre(m) {
  return m.replace(/^[ \t]*#[^\n]*(\\\r?\n[^\n]*)*/gm, s => s.replace(/[^\n]/g, ' '));
}

/**
 * -> [{ kind: 'class'|'struct'|'method'|'field'|'function', name, detail, start, end, selStart, selEnd, children }]
 * Classes hold their members; the out-of-class definitions of one class that follow each other are grouped under a
 * symbol named after the class (kind 'class', detail '定義').
 */
function symbolsOf(text) {
  const m = maskPre(mask(String(text || '')));
  const out = [];
  const taken = [];   // [start, end] of class bodies (the definitions scan skips them)
  // 1. classes / structs (with a body)
  const cre = /\b(class|struct)\s+(?:PACKAGE\s+)?([A-Za-z_]\w*)\s*(?:final\s*)?(?::[^{;()]*)?\{/g;
  let x;
  while ((x = cre.exec(m))) {
    const open = m.indexOf('{', x.index + x[0].length - 1);
    const close = closeOf(m, open);
    if (close < 0) continue;
    if (taken.some(([a, b]) => x.index > a && x.index < b)) continue;   // (a nested one: inside its outer's members)
    const sym = { kind: x[1], name: x[2], detail: x[1], start: x.index, end: close + 1, selStart: x.index + x[0].indexOf(x[2], x[1].length), selEnd: 0, children: [] };
    sym.selEnd = sym.selStart + x[2].length;
    sym.children = membersOf(m, open + 1, close, x[2]);
    out.push(sym);
    taken.push([x.index, close + 1]);
  }
  // 2. definitions outside a class: NAME::member( ... ) { and free functions name( ... ) {
  const dre = /(^|[;}\n])([ \t]*(?:[A-Za-z_][\w:<>,*& \t]*?[\s*&]+)?)(~?[A-Za-z_]\w*(?:::~?[A-Za-z_]\w*)*)\s*\(/g;
  const defs = [];
  while ((x = dre.exec(m))) {
    const nameAt = x.index + x[1].length + x[2].length;
    const full = x[3];
    const last = full.split('::').pop().replace(/^~/, '');
    if (KW.has(last) || KW.has(full)) continue;
    if (taken.some(([a, b]) => nameAt > a && nameAt < b)) continue;
    // the parameter list, then (const / noexcept / override / an initializer list) then { -- a body, not a call / declaration
    const po = m.indexOf('(', nameAt + full.length);
    const pc = parenClose(m, po);
    if (pc < 0) continue;
    const after = /^\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:->[^{;]*)?(?::[^{;]*)?\{/.exec(m.slice(pc + 1, pc + 4000));
    if (!after) continue;
    const open = pc + 1 + after[0].length - 1;
    const close = closeOf(m, open);
    if (close < 0) continue;
    // (inside another function's body: a lambda or a call -- not a definition)
    if (defs.some(d => nameAt > d.start && nameAt < d.end)) continue;
    const parts = full.split('::');
    const cls = parts.length > 1 ? parts.slice(0, -1).join('::') : '';
    defs.push({ kind: cls ? 'method' : 'function', cls, name: parts[parts.length - 1], detail: cls ? cls + '::' : '函式',
      start: nameAt - x[2].trimStart().length, end: close + 1, selStart: nameAt, selEnd: nameAt + full.length, children: [], sig: oneLine(m.slice(po, pc + 1)) });
    dre.lastIndex = close + 1;
  }
  // group the definitions of one class that follow each other
  let run = null;
  for (const d of defs) {
    if (!d.cls) { run = null; out.push(d); continue; }
    if (run && run.name === d.cls) { run.children.push(d); run.end = d.end; continue; }
    run = { kind: 'class', name: d.cls, detail: '定義', start: d.start, end: d.end, selStart: d.selStart, selEnd: d.selStart + d.cls.length, children: [d], defs: true };
    out.push(run);
  }
  return out.sort((a, b) => a.start - b.start);
}

function parenClose(m, open) {
  if (open < 0) return -1;
  let depth = 0;
  for (let i = open; i < m.length && i < open + 20000; i++) {
    if (m[i] === '(') depth++;
    else if (m[i] === ')') { depth--; if (!depth) return i; }
    else if ((m[i] === ';' || m[i] === '{' || m[i] === '}') && depth <= 1 && m[i] !== '{') return -1;
  }
  return -1;
}

function oneLine(s) { return s.replace(/\s+/g, ' ').trim(); }

/** The members of a class body [from, to): methods (declared or defined inline) and fields, one level deep. */
function membersOf(m, from, to, cls) {
  const out = [];
  let i = from;
  while (i < to) {
    // the next statement at this level: up to ; or a { ... } body
    let j = i, depth = 0;
    while (j < to) {
      const c = m[j];
      if (c === '(') depth++;
      else if (c === ')') depth--;
      else if (c === '{' && depth === 0) break;
      else if (c === ';' && depth === 0) break;
      j++;
    }
    if (j >= to) break;
    let end = j + 1;
    if (m[j] === '{') { const cl = closeOf(m, j); if (cl < 0) break; end = cl + 1; if (m[end] === ';') end++; }
    const stmt = m.slice(i, j);
    const s = stmt.replace(/^\s*(?:(?:public|private|protected|__published)\s*:\s*)+/, '');
    const lead = stmt.length - s.length;
    let fn = /(~?[A-Za-z_]\w*|operator\s*[^\s(]+)\s*\(([^()]*(?:\([^()]*\)[^()]*)*)\)\s*(?:const)?[^()]*$/.exec(s);
    // (TEdit *X = new TEdit(); -- an = before the parentheses: a field with its initializer, not a method)
    const eq = s.search(/[^=!<>]=(?!=)/);
    if (fn && eq >= 0 && eq < fn.index && !/\boperator\b/.test(s.slice(0, fn.index + 9))) fn = null;
    const nested = /^\s*(?:class|struct|enum|union|typedef|friend|using)\b/.test(s);
    if (fn && !nested && !/^\s*(return|if|while)\b/.test(s)) {
      const at = i + lead + fn.index;
      const name = fn[1];
      out.push({ kind: 'method', name, detail: name === cls ? '建構式' : name === '~' + cls ? '解構式' : '', start: i + lead + (s.length - s.trimStart().length), end, selStart: at, selEnd: at + name.length, children: [], sig: oneLine('(' + fn[2] + ')') });
    } else if (!nested && m[j] === ';') {
      // a field: the last identifier before ; (or before [ / = / :)
      const s2 = s.replace(/\s+$/, '');
      const eq2 = s2.search(/[^=!<>]=(?!=)/);
      const head = eq2 >= 0 ? s2.slice(0, eq2 + 1) : s2;   // (the name is before the initializer)
      const fl = /([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*(?::\s*\d+\s*)?$/.exec(head.replace(/\s+$/, ''));
      if (fl && /[A-Za-z_]/.test(s.slice(0, fl.index))) {
        const at = i + lead + fl.index;
        out.push({ kind: 'field', name: fl[1], detail: '', start: i + lead + (s.length - s.trimStart().length), end, selStart: at, selEnd: at + fl[1].length, children: [] });
      }
    }
    i = end;
  }
  return out;
}

/** The symbol chain at an offset: [class, member] (the innermost last). */
function chainAt(syms, off) {
  const out = [];
  let list = syms;
  for (;;) {
    const s = list.find(x => off >= x.start && off <= x.end);
    if (!s) break;
    out.push(s);
    list = s.children || [];
  }
  return out;
}

module.exports = { symbolsOf, chainAt, membersOf, closeOf };
