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
  // (1007 audit, events P1: [^\r\n] -- with [^\n] the line's CR was taken into it, the new line went in between CR and LF:
  //  "…\r\r\n" before it and a bare LF after it, and a delete never gave the header back byte for byte)
  const re = /^([ \t]*)void\s+\w+\s*\(\s*TObject\s*\*\s*Sender[^;]*\)\s*;[^\r\n]*\r?$/gm;
  // (1007 audit, events P2: the section each handler is in -- after the last one under private: / protected: the new one
  //  could never be wired (the web calls it from outside the class): only public / __published ones count; a class
  //  starts private, a struct public)
  const kw = /\b(class|struct)\b(?![\s\S]*\b(?:class|struct)\b)/.exec(m.slice(Math.max(0, b.open - 400), b.open));
  const secs = [];
  { const sr = /^[ \t]*(public|private|protected|__published)\s*:/gm; let y; while ((y = sr.exec(body))) secs.push({ at: y.index, end: y.index + y[0].length, name: y[1] }); }
  const secAt = i => { let n = kw && kw[1] === 'struct' ? 'public' : 'private'; for (const z of secs) if (z.at <= i) n = z.name; return n; };
  const open = n => n === 'public' || n === '__published';
  let last = null, x;
  while ((x = re.exec(body))) if (open(secAt(x.index))) last = x;
  const decl = 'void ' + name + '(' + params + ');' + (note ? '   ' + note : '');
  if (!last) {
    // no open handler: right after the last public: / __published: line
    const ps = secs.filter(z => open(z.name)).pop();
    if (ps) {
      let e = b.open + 1 + ps.end;
      while (e < hText.length && hText[e] !== '\n' && hText[e] !== '\r') e++;
      return { at: e, text: eol + '    ' + decl };
    }
  }
  if (last) {
    // the end of that line in the real text (its comment included)
    let lineEnd = b.open + 1 + last.index + last[0].length;
    // (the line end in the REAL text: mask() blanks a comment and the CR after it alike)
    while (lineEnd > b.open && (hText[lineEnd - 1] === '\r' || hText[lineEnd - 1] === '\n')) lineEnd--;
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

/**
 * AI(W906-HTDESIGNER) 20261003 (machine, EastSun: "你用事件轉跳到程式碼時 我希望你用不同底色 標出轉跳到的程式碼"):
 * the lines of the function that starts on `line0` (0-based): its header line to the line of its closing brace ->
 * { start, end } (0-based lines). The first '{' within 30 lines, with no ';' before it (masked: comments / strings do
 * not count) -- else it is not a function there (a declaration, a statement) -> null. C++ and JS alike.
 */
function funcRange(text, line0) {
  const t = String(text || '');
  const m = mask(t);
  const starts = [0];
  for (let i = 0; i < t.length; i++) if (t[i] === '\n') starts.push(i + 1);
  if (line0 < 0 || line0 >= starts.length) return null;
  const end = starts[Math.min(starts.length - 1, line0 + 30)] || t.length;
  const at = m.indexOf('{', starts[line0]);
  if (at < 0 || at > end) return null;
  const semi = m.indexOf(';', starts[line0]);
  if (semi >= 0 && semi < at) return null;
  let depth = 0, close = -1;
  for (let i = at; i < m.length; i++) {
    if (m[i] === '{') depth++;
    else if (m[i] === '}') { depth--; if (!depth) { close = i; break; } }
  }
  if (close < 0) return null;
  let lo = 0, hi = starts.length - 1;
  while (lo < hi) { const mid = (lo + hi + 1) >> 1; if (starts[mid] <= close) lo = mid; else hi = mid - 1; }
  return { start: line0, end: lo };
}

/**
 * 0.162 (EastSun: "刪除事件時 需要連動刪除 要可以編譯成功 刪除事件時要跳出提醒視窗"): what deleting an event handler takes
 * out -- its declaration line in the class (.h) and its whole function in the .cpp (the designer's note line above it
 * and one blank line with it), and whether that is safe:
 *   { h: { s, e } | null, cpp: { s, e } | null, body (the code inside the braces), empty (only the designer's stub:
 *     "(void)Sender;" / "// TODO" / nothing), ours (the designer's note is on it), usedElsewhere: [where it is still
 *     named outside its declaration and definition -- then taking it out would not compile] }
 */
function removeEdits(hText, cppText, cls, name) {
  const out = { h: null, cpp: null, body: '', empty: false, ours: false, usedElsewhere: [] };
  const lineStart = (t, i) => { while (i > 0 && t[i - 1] !== '\n') i--; return i; };
  const lineEnd = (t, i) => { const e = t.indexOf('\n', i); return e < 0 ? t.length : e + 1; };
  // the declaration: `name(` inside the class body, its whole line (one line, ends with ';')
  const b = classBody(hText, cls);
  const mh = mask(hText);
  if (b) {
    const re = new RegExp('\\b' + name + '\\s*\\(', 'g');
    re.lastIndex = b.open;
    const x = re.exec(mh);
    if (x && x.index < b.close) {
      const s = lineStart(hText, x.index), e = lineEnd(hText, x.index);
      if (/;/.test(mh.slice(x.index, e))) {
        out.h = { s, e };
        if (/AI\(W906-HTDESIGNER\)/.test(hText.slice(s, e))) out.ours = true;
      }
    }
  }
  // the definition: `cls::name(` ... `{ ... }`
  const mc = mask(cppText);
  const rd = new RegExp('\\b' + cls + '\\s*::\\s*' + name + '\\s*\\(', 'g');
  const y = rd.exec(mc);
  if (y) {
    const open = mc.indexOf('{', y.index);
    let depth = 0, close = -1;
    for (let i = open; open >= 0 && i < mc.length; i++) {
      if (mc[i] === '{') depth++;
      else if (mc[i] === '}') { depth--; if (!depth) { close = i; break; } }
    }
    if (close > 0) {
      let s = lineStart(cppText, y.index);
      // the designer's note just above it (//AI(W906-HTDESIGNER) ...), and the blank line above that
      const prevLine = p => { const e = p; let st = e - 1; if (st < 0) return null; st = lineStart(cppText, st - 0); return { s: st, e, text: cppText.slice(st, e) }; };
      let pl = prevLine(s);
      if (pl && /^\s*\/\/\s*AI\(W906-HTDESIGNER\)/.test(pl.text)) { out.ours = true; s = pl.s; pl = prevLine(s); }
      if (pl && /^\s*$/.test(pl.text)) s = pl.s;
      let e = lineEnd(cppText, close);
      out.cpp = { s, e };
      out.body = cppText.slice(open + 1, close);
      out.empty = out.body.replace(/\(void\)\s*Sender\s*;/g, '').replace(/\/\/\s*TODO[^\n]*/g, '').trim() === '';
    }
  }
  // still named elsewhere? (a call, a pointer, another declaration) -- the text without the two parts
  const cut = (t, r) => (r ? t.slice(0, r.s) + ' '.repeat(r.e - r.s) + t.slice(r.e) : t);
  const word = new RegExp('\\b' + name + '\\b', 'g');
  if ((cut(mh, out.h).match(word) || []).length) out.usedElsewhere.push('.h');
  if ((cut(mc, out.cpp).match(word) || []).length) out.usedElsewhere.push('.cpp');
  return out;
}

/**
 * (1008, feature gap #10): WPF renames x:Name's uses in the code-behind -- a component renamed on the page: the form
 * class's member of that name and its uses, whole words only, in the class's .h (inside the class body) and its .cpp.
 * Comments and strings are left alone (masked). Not renamed: a use through another object (`fOther->spbSave`,
 * `x.spbSave`) unless it is `this` or the form's own global (`extern PACKAGE TfHotPlate *fHotPlate;` -> fHotPlate),
 * another class's `TOther::spbSave`, a `spbSave::` scope; and (1008 audit G1) in the .cpp a BARE use outside the class's
 * own member functions (`TfX::f(...) { ... }`, its init list included) -- a free function's or another class's local.
 *   -> { h: [{ s, e, text }], cpp: [...], clash: true when the class (or the .cpp) already names newName, selfs: [...] }
 */
function renameMemberEdits(hText, cppText, cls, oldName, newName) {
  const out = { h: [], cpp: [], clash: false, selfs: ['this'] };
  const ht = String(hText || ''), ct = String(cppText || '');
  if (!/^[A-Za-z_]\w*$/.test(String(oldName)) || !/^[A-Za-z_]\w*$/.test(String(newName)) || oldName === newName) return out;
  const b = classBody(ht, cls);
  if (!b) return out;
  const mh = mask(ht), mc = mask(ct);
  // the form's own global instance(s): `TfX *fX;` outside the class body (BCB6's extern PACKAGE line)
  const outside = mh.slice(0, b.open) + ' '.repeat(b.close - b.open) + mh.slice(b.close);
  const gr = new RegExp('\\b' + cls + '\\s*\\*\\s*([A-Za-z_]\\w*)\\s*[;=,]', 'g');
  let g;
  while ((g = gr.exec(outside))) if (!out.selfs.includes(g[1])) out.selfs.push(g[1]);
  const word = n => new RegExp('(?<![\\w$])' + n + '(?![\\w$])', 'g');
  if (word(newName).test(mh.slice(b.open, b.close)) || word(newName).test(mc)) out.clash = true;
  // is the use at i (masked text m) one of THIS class's member?
  const ours = (m, i) => {
    let j = i - 1;
    while (j >= 0 && /\s/.test(m[j])) j--;
    const k = i + oldName.length;
    let a = k;
    while (a < m.length && /\s/.test(m[a])) a++;
    if (m[a] === ':' && m[a + 1] === ':') return false;   // spbSave:: -- a scope, not the member
    const objBefore = end => { let e = end; while (e >= 0 && /\s/.test(m[e])) e--; let s0 = e; while (s0 >= 0 && /[\w$]/.test(m[s0])) s0--; return m.slice(s0 + 1, e + 1); };
    if (m[j] === '>' && m[j - 1] === '-') return out.selfs.includes(objBefore(j - 2));
    if (m[j] === '.' && !/\d/.test(m[j - 1] || '')) return out.selfs.includes(objBefore(j - 1));
    if (m[j] === ':' && m[j - 1] === ':') return objBefore(j - 2) === cls;
    return true;
  };
  // (audit G1) the .cpp's member functions of cls: from `cls::name(` to the closing brace of its body
  const members = [];
  const hr = new RegExp('(?<![\\w$])' + cls + '\\s*::\\s*~?[A-Za-z_]\\w*\\s*\\(', 'g');
  let hm;
  while ((hm = hr.exec(mc))) {
    let i = hr.lastIndex, d = 1;
    while (i < mc.length && d) { if (mc[i] === '(') d++; else if (mc[i] === ')') d--; i++; }
    const at = mc.indexOf('{', i), semi = mc.indexOf(';', i);
    if (at < 0 || (semi >= 0 && semi < at)) continue;          // a declaration / a call, not a definition
    let depth = 0, close = -1;
    for (let k = at; k < mc.length; k++) { if (mc[k] === '{') depth++; else if (mc[k] === '}') { depth--; if (!depth) { close = k; break; } } }
    if (close < 0) continue;
    members.push([hm.index, close]);
    hr.lastIndex = close;
  }
  const inMember = i => members.some(r => i > r[0] && i < r[1]);
  const bare = (m, i) => { let j = i - 1; while (j >= 0 && /\s/.test(m[j])) j--; return !((m[j] === '>' && m[j - 1] === '-') || (m[j] === '.' && !/\d/.test(m[j - 1] || '')) || (m[j] === ':' && m[j - 1] === ':')); };
  const collect = (m, from, to, into, cpp) => {
    const re = word(oldName);
    re.lastIndex = from;
    let x;
    while ((x = re.exec(m)) && x.index < to) if (ours(m, x.index) && !(cpp && bare(m, x.index) && !inMember(x.index))) into.push({ s: x.index, e: x.index + oldName.length, text: newName });
  };
  collect(mh, b.open, b.close, out.h);
  collect(mc, 0, mc.length, out.cpp, true);
  return out;
}

module.exports = { mask, classBody, declares, declEdit, defEdit, renameEdits, bodyStart, funcRange, removeEdits, renameMemberEdits };
