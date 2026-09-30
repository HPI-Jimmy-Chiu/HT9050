'use strict';
// AI(W906-HTDESIGNER) 20260930: 新增事件處理函式 -- what BCB6 does on a double-click on an empty event: the
// declaration in the form's class (.h) and an empty body (.cpp). EastSun 20260930 chose the C++ port tree
// for them. These only compute the text and where it goes; the extension applies it as an edit of the
// documents (nothing is saved). Plain Node.

/** The text with comments, strings and character literals blanked (same length: offsets stay). */
function mask(text) {
  const t = String(text || '');
  let out = '';
  let i = 0;
  while (i < t.length) {
    const c = t[i], d = t[i + 1];
    if (c === '/' && d === '/') { const e = t.indexOf('\n', i); const j = e < 0 ? t.length : e; out += ' '.repeat(j - i); i = j; continue; }
    if (c === '/' && d === '*') { const e = t.indexOf('*/', i + 2); const j = e < 0 ? t.length : e + 2; out += t.slice(i, j).replace(/[^\n]/g, ' '); i = j; continue; }
    if (c === '"' || c === "'") {
      let j = i + 1;
      while (j < t.length && t[j] !== c && t[j] !== '\n') j += t[j] === '\\' ? 2 : 1;
      j = Math.min(t.length, j + 1);
      out += c + ' '.repeat(Math.max(0, j - i - 2)) + (j - i >= 2 ? c : '');
      i = j;
      continue;
    }
    out += c;
    i++;
  }
  return out;
}

/** The body of `class cls { ... };` in a header: { open, close } (offsets of the braces), or null. */
function classBody(text, cls) {
  const m = mask(text);
  const re = new RegExp('\\bclass\\s+(?:PACKAGE\\s+)?' + cls + '\\b[^;{]*\\{', 'g');
  const hit = re.exec(m);
  if (!hit) return null;
  const open = hit.index + hit[0].length - 1;
  let depth = 0;
  for (let i = open; i < m.length; i++) {
    if (m[i] === '{') depth++;
    else if (m[i] === '}') { depth--; if (!depth) return { open, close: i }; }
  }
  return null;
}

/** Is `name(` declared in the class (outside comments)? */
function declares(text, cls, name) {
  const b = classBody(text, cls);
  if (!b) return false;
  return new RegExp('\\b' + name + '\\s*\\(').test(mask(text).slice(b.open, b.close));
}

function eolOf(text) { return /\r\n/.test(String(text || '')) ? '\r\n' : '\n'; }

/**
 * The declaration to add: after the last event handler of the class (a `void X(TObject *Sender…);` line),
 * with its indentation; without one, before `private:` / the closing brace. `note`: a whole comment
 * ("//AI(...) ..."), put after the declaration / above the body.
 * -> { at, text } or null (no such class)
 */
function declEdit(hText, cls, name, params, note) {
  const b = classBody(hText, cls);
  if (!b) return null;
  const eol = eolOf(hText);
  const m = mask(hText);
  const body = m.slice(b.open + 1, b.close);
  const re = /^([ \t]*)void\s+\w+\s*\(\s*TObject\s*\*\s*Sender[^;]*\)\s*;[^\n]*$/gm;
  let last = null, x;
  while ((x = re.exec(body))) last = x;
  const decl = 'void ' + name + '(' + params + ');' + (note ? '   ' + note : '');
  if (last) {
    // the end of that line in the real text (its comment included)
    const lineEnd = b.open + 1 + last.index + last[0].length;
    return { at: lineEnd, text: eol + last[1] + decl };
  }
  const priv = /^([ \t]*)(private|protected)\s*:/m.exec(body);
  if (priv) {
    const at = b.open + 1 + priv.index;
    return { at, text: '    ' + decl + eol };
  }
  // before the closing brace (at the start of its line)
  let at = b.close;
  while (at > 0 && hText[at - 1] !== '\n') at--;
  return { at, text: '    ' + decl + eol };
}

/** The empty handler, at the end of the .cpp. -> { at, text, bodyLine (0-based line of its first statement) } */
function defEdit(cppText, cls, name, params, note) {
  const t = String(cppText || '');
  const eol = eolOf(t);
  const lead = t.length && !/\n$/.test(t) ? eol : '';
  const usesSender = /^\s*TObject\s*\*\s*Sender\b/.test(params);
  const lines = [
    '',
    note,
    'void ' + cls + '::' + name + '(' + params + ')',
    '{',
    usesSender ? '    (void)Sender;' : '    ',
    '    // TODO',
    '}',
    '',
  ];
  const text = lead + lines.join(eol);
  const full = t + text;
  const todo = full.indexOf('    // TODO', t.length);
  let bodyLine = 0;
  for (let i = 0; i < todo; i++) if (full.charCodeAt(i) === 10) bodyLine++;
  return { at: t.length, text, bodyLine };
}

/**
 * Rename a handler the designer added: `old(` declared in the class body (.h), `cls::old` in the .cpp (comments and
 * strings left alone). -> { h: [{ s, e, text }], cpp: [...] }
 */
function renameEdits(hText, cppText, cls, oldName, newName) {
  const out = { h: [], cpp: [] };
  const b = classBody(hText, cls);
  if (b) {
    const m = mask(hText);
    const re = new RegExp('\\b' + oldName + '(?=\\s*\\()', 'g');
    let x;
    re.lastIndex = b.open;
    while ((x = re.exec(m)) && x.index < b.close) out.h.push({ s: x.index, e: x.index + oldName.length, text: newName });
  }
  const mc = mask(cppText);
  const rc = new RegExp('\\b' + cls + '\\s*::\\s*' + oldName + '\\b', 'g');
  let y;
  while ((y = rc.exec(mc))) {
    const s = y.index + y[0].length - oldName.length;
    out.cpp.push({ s, e: s + oldName.length, text: newName });
  }
  return out;
}

/**
 * WPF "navigates to the existing handler" with the caret in its body: from the line a function starts on (0-based),
 * the first '{' within 30 lines (masked: not in a comment or a string) -> { line, col } of the first statement in the
 * body (the line after the brace, at its first non-blank), or just after the brace when the body is empty; null when
 * there is no brace near.
 */
function bodyStart(text, line0) {
  const m = mask(text);
  const starts = [0];
  for (let i = 0; i < text.length; i++) if (text[i] === '\n') starts.push(i + 1);
  if (line0 < 0 || line0 >= starts.length) return null;
  const end = starts[Math.min(starts.length - 1, line0 + 30)] || text.length;
  const at = m.indexOf('{', starts[line0]);
  if (at < 0 || at > end) return null;
  const lineOf = off => { let lo = 0, hi = starts.length - 1; while (lo < hi) { const mid = (lo + hi + 1) >> 1; if (starts[mid] <= off) lo = mid; else hi = mid - 1; } return lo; };
  const bl = lineOf(at);
  const next = bl + 1 < starts.length ? text.slice(starts[bl + 1], (starts[bl + 2] || text.length + 1) - 1).replace(/\r$/, '') : '';
  const rest = text.slice(at + 1, (starts[bl + 1] || text.length + 1) - 1).replace(/\r$/, '');
  if (rest.trim() || bl + 1 >= starts.length || /^\s*}/.test(next)) return { line: bl, col: at - starts[bl] + 1 };
  return { line: bl + 1, col: (/^\s*/.exec(next) || [''])[0].length };
}

module.exports = { mask, classBody, declares, declEdit, defEdit, renameEdits, bodyStart };
