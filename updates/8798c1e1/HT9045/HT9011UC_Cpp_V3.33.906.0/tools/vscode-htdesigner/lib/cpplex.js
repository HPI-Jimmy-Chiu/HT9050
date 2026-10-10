'use strict';
// AI(W906-HTDESIGNER) 20260929: tell a C++ hit apart: in a comment? inside #if 0?
//
// Why a lexer and not a line regex: this tree has preprocessor directives inside
// comments (a "/* ... #endif */" in csystem.cpp) and inside string literals. A naive
// #if-depth counter goes wrong at the first one and stays wrong for the rest of a
// 30,000-line file. So comments and literals are blanked first, then directives
// are read from what is left.
//
// Only "#if 0" / "#if 1 ... #else" are treated as dead. #ifdef SOFT_SIMULTE and the
// like depend on the build line, so they are reported as live (the honest answer
// without a compiler). tools\live_lines.ps1 is the authority when it matters.
//
// Input is the file decoded as latin1 (1 char = 1 byte). That is safe for UTF-8
// sources (multi-byte sequences never contain ASCII bytes). It is NOT safe for the
// Big5 golden tree (Big5 trail bytes include '\' and quote-range bytes), so the
// golden tree is never run through this.

// (1009 review (C++ nav #3): a lone CR ends a line too, as in VS Code -- Setup.Configuration.html has one ("\r\r\n") and
//  every hit after it was a line short; lib/deadcode.js and the search count lines this way already)
function lineStartsOf(text) {
  const a = [0];
  for (let i = 0; i < text.length; i++) {
    const c = text.charCodeAt(i);
    if (c === 10 || (c === 13 && text.charCodeAt(i + 1) !== 10)) a.push(i + 1);
  }
  return a;
}

function lineIndexOf(starts, off) {
  let lo = 0, hi = starts.length - 1;
  while (lo < hi) {
    const mid = (lo + hi + 1) >> 1;
    if (starts[mid] <= off) lo = mid; else hi = mid - 1;
  }
  return lo;
}

function isIdentChar(c) {
  return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c === 95;
}

/**
 * Returns { starts, comments:[s,e,...], masked, dead:Uint8Array(lines) }.
 * masked = text with comments and literal contents replaced by spaces (newlines kept).
 */
function analyze(text) {
  const n = text.length;
  const comments = [];
  const blanks = []; // [s,e) ranges to blank (comments + literal bodies)
  let i = 0;
  while (i < n) {
    const c = text.charCodeAt(i);
    if (c === 47 && text.charCodeAt(i + 1) === 47) { // line comment (with \ continuation)
      const s = i;
      i += 2;
      while (i < n) {
        if (text.charCodeAt(i) === 10) {
          let k = i - 1;
          if (k >= s && text.charCodeAt(k) === 13) k--;
          while (k >= s && (text.charCodeAt(k) === 32 || text.charCodeAt(k) === 9)) k--;   // (review of 0.423: "\  " then the line end, as mask() / g++)
          if (k >= s && text.charCodeAt(k) === 92) { i++; continue; }
          break;
        }
        i++;
      }
      comments.push(s, i);
      blanks.push(s, i);
      continue;
    }
    if (c === 47 && text.charCodeAt(i + 1) === 42) { // block comment
      const s = i;
      const e = text.indexOf('*/', i + 2);
      i = e < 0 ? n : e + 2;
      comments.push(s, i);
      blanks.push(s, i);
      continue;
    }
    if (c === 34 || c === 39) { // "..." or '...'
      // raw string R"delim( ... )delim"
      // (review of 0.422 C++ #3: LR" / u8R" / uR" / UR" too, as cppstub.mask)
      if (c === 34 && i > 0 && /(?:^|[^\w])(?:u8|[uUL])?R$/.test(text.slice(Math.max(0, i - 4), i))) {
        const p = text.indexOf('(', i + 1);
        if (p > 0 && p - i <= 17) {
          const delim = ')' + text.slice(i + 1, p) + '"';
          const e = text.indexOf(delim, p + 1);
          const end = e < 0 ? n : e + delim.length;
          blanks.push(i + 1, end - 1);
          i = end;
          continue;
        }
      }
      // digit separator 1'000'000
      if (c === 39 && i > 0 && isIdentChar(text.charCodeAt(i - 1)) && isIdentChar(text.charCodeAt(i + 1))) {
        const prev = text.charCodeAt(i - 1);
        if (prev >= 48 && prev <= 57) { i++; continue; }
      }
      const s = i;
      i++;
      while (i < n) {
        const ch = text.charCodeAt(i);
        if (ch === 92) { i += 2; continue; }
        if (ch === c) { i++; break; }
        if (ch === 10) break; // unterminated: stop at the line end
        i++;
      }
      if (i - s > 2) blanks.push(s + 1, i - 1);
      continue;
    }
    i++;
  }

  // masked text
  let masked = '';
  let last = 0;
  for (let k = 0; k < blanks.length; k += 2) {
    const s = blanks[k], e = blanks[k + 1];
    if (s < last) continue;
    masked += text.slice(last, s) + text.slice(s, e).replace(/[^\n]/g, ' ');
    last = e;
  }
  masked += text.slice(last);

  const starts = lineStartsOf(text);
  // CRLF files: the '\r' must go, or "#endif\r" never matches (JS '.' stops at '\r')
  const lines = masked.split(/\r\n|\r|\n/);   // (the same line ends as lineStartsOf)
  const dead = new Uint8Array(lines.length);
  // frame: dead = this branch is dead; taken = an earlier branch was the live one;
  // unknown = the condition depends on the build (every branch reported live)
  const stack = [];
  let deadCount = 0;
  const ZERO = /^\(?\s*(0|false)\s*\)?$/;
  const ONE = /^\(?\s*(1|true)\s*\)?$/;
  for (let L = 0; L < lines.length; L++) {
    let line = lines[L];
    // join backslash continuations for the directive test
    let span = 0;
    while (/\\\s*$/.test(line) && L + span + 1 < lines.length) {
      span++;
      line = line.replace(/\\\s*$/, ' ') + lines[L + span];
    }
    const m = /^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b([\s\S]*)$/.exec(line);
    if (!m) {
      dead[L] = deadCount > 0 ? 1 : 0;
      continue;
    }
    const kw = m[1];
    const rest = m[2].trim();
    // a directive line is dead only when an ENCLOSING frame is dead
    const top = stack[stack.length - 1];
    const enclosing = kw === 'if' || kw === 'ifdef' || kw === 'ifndef' ? deadCount
      : deadCount - (top && top.dead ? 1 : 0);
    dead[L] = enclosing > 0 ? 1 : 0;
    if (kw === 'if') {
      const f = ZERO.test(rest) ? { dead: true, taken: false, unknown: false }
        : ONE.test(rest) ? { dead: false, taken: true, unknown: false }
          : { dead: false, taken: false, unknown: true };
      stack.push(f);
      if (f.dead) deadCount++;
    } else if (kw === 'ifdef' || kw === 'ifndef') {
      stack.push({ dead: false, taken: false, unknown: true });
    } else if (kw === 'elif' || kw === 'else') {
      const f = stack[stack.length - 1];
      if (f) {
        const was = f.dead;
        let now = false;
        if (!f.unknown) {
          if (f.taken) now = true;
          else if (kw === 'else') { now = false; f.taken = true; }
          else if (ZERO.test(rest)) now = true;
          else if (ONE.test(rest)) { now = false; f.taken = true; }
          else { now = false; f.unknown = true; }
        } else if (kw === 'elif' && ZERO.test(rest)) now = true;   // (1009 review (C++ nav #7): "#if X ... #elif 0" -- that part never compiles whatever X is, as lib/deadcode.js says)
        if (was !== now) deadCount += now ? 1 : -1;
        f.dead = now;
      }
    } else if (kw === 'endif') {
      const f = stack.pop();
      if (f && f.dead) deadCount--;
    }
    for (let k = 1; k <= span; k++) dead[L + k] = dead[L];
    L += span;
  }
  return { starts, comments, masked, dead };
}

function inRanges(ranges, off) {
  let lo = 0, hi = (ranges.length >> 1) - 1;
  while (lo <= hi) {
    const mid = (lo + hi) >> 1;
    const s = ranges[mid * 2], e = ranges[mid * 2 + 1];
    if (off < s) hi = mid - 1;
    else if (off >= e) lo = mid + 1;
    else return true;
  }
  return false;
}

/**
 * Is `Cls::name` at `pos` (just after the name) the head of a definition?
 * Looks for "( ... )" followed by "{" or a ctor-initialiser ":" (not "::").
 */
function isDefinitionAt(text, pos) {
  const n = Math.min(text.length, pos + 6000);
  let i = pos;
  while (i < n && /\s/.test(text[i])) i++;
  if (text[i] !== '(') return false;
  let depth = 0;
  for (; i < n; i++) {
    const c = text[i];
    if (c === '(') depth++;
    else if (c === ')') { depth--; if (depth === 0) { i++; break; } }
    else if (c === ';' || c === '{' || c === '}') return false;
  }
  for (;;) {
    while (i < n && /\s/.test(text[i])) i++;
    if (text.startsWith('//', i)) { const e = text.indexOf('\n', i); i = e < 0 ? n : e + 1; continue; }
    if (text.startsWith('/*', i)) { const e = text.indexOf('*/', i + 2); i = e < 0 ? n : e + 2; continue; }
    const q = /^(const|noexcept|override|final|volatile|__fastcall|throw\s*\([^)]*\))\b/.exec(text.slice(i, i + 40));
    if (q) { i += q[0].length; continue; }
    break;
  }
  const c = text[i];
  if (c === '{') return true;
  if (c !== ':' || text[i + 1] === ':') return false;
  // (1009 review (C++ nav #8): ") :" is a constructor's initialiser list only where a definition can start -- in
  //  "live ? TagValue::makeString(v) : ..." the call is an operand: before its name comes ? = ( , or an operator / return)
  let b = pos - 1;
  while (b >= 0 && /[\w:~]/.test(text[b])) b--;
  while (b >= 0 && /\s/.test(text[b])) b--;
  if (b >= 0 && /[?=(,!&|+\-*/<>%^]/.test(text[b])) return false;
  if (b >= 5 && /\breturn$/.test(text.slice(Math.max(0, b - 6), b + 1))) return false;
  return true;
}

module.exports = { analyze, inRanges, isDefinitionAt, lineStartsOf, lineIndexOf };
