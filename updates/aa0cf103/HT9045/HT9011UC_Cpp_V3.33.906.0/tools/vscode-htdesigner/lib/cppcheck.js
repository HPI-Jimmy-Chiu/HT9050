'use strict';
// AI(W906-HTDESIGNER) 20261007 (EastSun: "可能造成c++編譯失敗的code 我需要在當前開啟的頁面 可以看到 字的底線加入紅色 右邊滑條
// 紅色部分顯示那個段落有 異常編碼"): what in a C / C++ file breaks the build before the compiler even reads the code --
// bytes that are not UTF-8 (a Big5 line pasted in), the replacement character a bad decode left, and outside comments /
// strings: full-width punctuation (；（）＝), an ideographic space, invisible characters (zero width, no-break space),
// curly quotes, and any other non-ASCII character (g++ 6.3, the oracle, takes no non-ASCII identifiers).
// Plain Node (no vscode). Results: [{ line, col, len, msg, kind }] (0-based line / col, in the text's UTF-16 units).

const { mask } = require('./cppstub');

/** The byte ranges of a buffer that are not valid UTF-8: [{ at, len }] (a BOM at the start is fine). */
function badUtf8(buf) {
  const out = [];
  const n = buf.length;
  let i = 0;
  while (i < n) {
    const b = buf[i];
    if (b < 0x80) { i++; continue; }
    let need = 0, min = 0;
    if (b >= 0xc2 && b <= 0xdf) { need = 1; min = 0x80; }
    else if (b >= 0xe0 && b <= 0xef) { need = 2; min = 0x800; }
    else if (b >= 0xf0 && b <= 0xf4) { need = 3; min = 0x10000; }
    else { push(i, 1); i++; continue; }
    let cp = b & (need === 1 ? 0x1f : need === 2 ? 0x0f : 0x07), ok = i + need < n;
    for (let k = 1; ok && k <= need; k++) { const c = buf[i + k]; if ((c & 0xc0) !== 0x80) ok = false; else cp = (cp << 6) | (c & 0x3f); }
    if (!ok || cp < min || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) { push(i, 1); i++; continue; }
    i += need + 1;
  }
  return out;
  function push(at, len) {
    const last = out[out.length - 1];
    if (last && last.at + last.len === at) last.len += len; else out.push({ at, len });
  }
}

/** Line / column (0-based, UTF-16) of each bad byte range: the bytes before it decoded as the editor shows them. */
function bytePositions(buf, ranges) {
  const out = [];
  if (!ranges.length) return out;
  // (1008 audit C2: a BOM is not in the editor's text -- line 1's columns counted it, the underline one to the right)
  const bom = buf.length >= 3 && buf[0] === 0xef && buf[1] === 0xbb && buf[2] === 0xbf ? 3 : 0;
  let line = 0, lineStartByte = bom, scan = bom;
  for (const r of ranges) {
    for (let i = scan; i < r.at; i++) if (buf[i] === 0x0a) { line++; lineStartByte = i + 1; }
    scan = r.at;
    // (the column: what the editor shows before it -- valid UTF-8 decoded, each bad byte one U+FFFD)
    const col = buf.subarray(lineStartByte, r.at).toString('utf8').length;
    out.push({ line, col, len: Math.max(1, buf.subarray(r.at, r.at + r.len).toString('utf8').length) });
  }
  return out;
}

const NAMES = [
  [/[！-～]/, '全形符號「$」：C++ 只認半形（例如 ；→ ;　（）→ ()　＝ → =）'],
  [/　/, '全形空白：C++ 只認半形空白'],
  [/[​-‍⁠﻿]/, '看不見的零寬字元：編譯器會當成不認得的字'],
  [/[  ]/, '不換行空白（NBSP）：看起來像空白，編譯器不認'],
  [/[‘’“”]/, '彎引號「$」：C++ 的引號是直的 \' 或 "'],
  [/�/, '亂碼（�）：這裡原本的位元組不是 UTF-8，存檔時已經壞掉了'],
];

/**
 * Problems in the text of a C / C++ file: outside comments and string / char literals, every non-ASCII character
 * (named when it is a known look-alike); everywhere, U+FFFD. -> [{ line, col, len, msg, kind }]
 */
function checkText(text, opts) {
  const t = String(text || '');
  // (1007, EastSun "要照c++版本": a letter / CJK character in an identifier is fine for GCC 10 and later (C++ extended
  //  identifiers) and an error for older ones -- g++ 6.3, the oracle. Look-alike punctuation, invisible characters and
  //  bad bytes break every version)
  const gccMajor = opts && opts.gccMajor ? opts.gccMajor : 0;
  const identOk = gccMajor >= 10;
  const out = [];
  let m0 = null;
  // (1008 audit D8: what mask() does not know is blanked first, same offsets -- a raw string R"x( ... )x" (its " and line
  //  breaks), a // comment that a trailing \ carries onto the next line, and the lines an #if 0 never compiles)
  // (1008 full test, audit C3: the prefixed ones too -- u8R"( )", LR"( )", uR / UR)
  let pre = t.replace(/\b(u8|[uUL])?R"([^(\s\\]{0,16})\(([\s\S]*?)\)\2"/g, (all, p, d, body) => (p || '') + 'R"' + d + '(' + body.replace(/[^\r\n]/g, ' ') + ')' + d + '"');
  // (1008 full test, audit C4: only a // outside a string -- #define URL "http://x" \ continues CODE, not a comment)
  pre = pre.replace(/\/\/[^\r\n]*\\\r?\n[^\r\n]*/g, (s0, at, all) => {
    const ls = all.lastIndexOf('\n', at - 1) + 1;
    const before = all.slice(ls, at).replace(/\\./g, '');
    if (((before.match(/"/g) || []).length % 2) === 1) return s0;
    return s0.replace(/(\\\r?\n)([^\r\n]*)$/, (x, nl, rest) => nl + rest.replace(/./g, ' '));
  });
  try { m0 = mask(pre); } catch (e) { m0 = pre; }
  try {
    // (1008 audit C7: an "#endif" line inside a raw string is not one -- the whole literal blanked, its quotes too:
    //  deadcode reads a lone `R"(` quote as an ordinary string running on and swallowing the #if after it)
    const forDead = pre.replace(/\b(u8|[uUL])?R"[^(\s\\]{0,16}\([\s\S]*?\)[^()\s\\"]{0,16}"/g, s => s.replace(/[^\r\n]/g, ' '));
    const dead = require('./deadcode').deadRanges(forDead);
    if (dead.length) {
      // (by the TEXT's line offsets -- mask() turns a multi-line string's line breaks into spaces, so m0's own lines are
      //  shifted after one; blanking m0's line N used to blank the wrong line, found by the full test)
      const st = [0];
      for (let i = 0; i < t.length; i++) if (t.charCodeAt(i) === 10) st.push(i + 1);
      const a = m0.split('');
      for (const r of dead) {
        const s = st[r.from], e = r.to + 1 < st.length ? st[r.to + 1] : t.length;
        if (s == null) continue;
        for (let i = s; i < e && i < a.length; i++) if (t[i] !== '\n' && t[i] !== '\r') a[i] = ' ';
      }
      m0 = a.join('');
    }
  } catch (e) { /* as it was */ }
  const starts = [0];
  for (let i = 0; i < t.length; i++) if (t.charCodeAt(i) === 10) starts.push(i + 1);
  const lineOf = off => { let lo = 0, hi = starts.length - 1; while (lo < hi) { const mid = (lo + hi + 1) >> 1; if (starts[mid] <= off) lo = mid; else hi = mid - 1; } return lo; };
  const re = /[^\x00-\x7F]+/g;
  let m;
  while ((m = re.exec(t))) {
    let i = m.index;
    const end = i + m[0].length;
    // (1008 audit C5: by code point -- a CJK Extension B character (U+20000 and up) is two UTF-16 units; tested one unit at
    //  a time it was no letter (an error under g++ 16) and the message showed half a character)
    const at = k => String.fromCodePoint(t.codePointAt(k));
    while (i < end) {
      const ch = at(i), w = ch.length;
      const inCode = m0.slice(i, i + w) === ch;   // (mask blanks comments / literals: what stayed is code)
      if (i === 0 && ch === '﻿') { i += w; continue; }   // (a BOM)
      let msg = null, kind = 'nonascii';
      for (const [rx, text0] of NAMES) if (rx.test(ch)) { msg = text0.replace('$', ch); kind = 'lookalike'; break; }
      if (ch === '�') kind = 'replacement';
      if (!inCode && kind !== 'replacement') { i += w; continue; }
      if (identOk && kind === 'nonascii' && /[\p{L}\p{N}\p{Mn}\p{Mc}]/u.test(ch)) { i += w; continue; }
      if (!msg) msg = '程式碼裡（註解和字串以外）有非 ASCII 字「' + ch + '」：' + (gccMajor ? 'g++ ' + gccMajor + ' 不接受' + (gccMajor < 10 ? '（g++ 10 以後的版本才允許非 ASCII 的名稱）' : '（不是字母或數字）') : 'g++ 10 以前的版本（例如 oracle 的 6.3）不接受');
      // (a run of the same kind = one problem)
      let j = i + w;
      // (1008 audit D9: a look-alike right after a name -- 變數； -- is a problem of its own, with its own explanation)
      while (j < end) {
        const c2 = at(j);
        if ((m0.slice(j, j + c2.length) === c2) !== inCode || (c2 === '�') !== (ch === '�') || NAMES.some(([rx]) => rx.test(c2))) break;
        j += c2.length;
      }
      if (kind === 'lookalike') j = i + w;
      const line = lineOf(i);
      out.push({ line, col: i - starts[line], len: j - i, msg, kind });
      i = j;
    }
  }
  return out;
}

/** Text + the file's bytes on disk (when they are what the editor shows): every problem, the bad UTF-8 first. */
function check(text, buf, opts) {
  const out = [];
  if (buf) {
    for (const p of bytePositions(buf, badUtf8(buf))) out.push(Object.assign(p, { kind: 'badutf8', msg: '這裡的位元組不是 UTF-8（可能是 Big5 中文貼進來，或用別的編碼存過）：編譯器會讀成亂碼。用 UTF-8 重新輸入這段' }));
  }
  // (1008 audit C1: a bad-byte range is several U+FFFD in the editor, each its own "亂碼" problem -- every one inside the
  //  range is the same problem; only its start used to be matched: one Big5 character = two problems)
  const bad = out.slice();
  const covered = p => bad.some(b => b.line === p.line && p.col >= b.col && p.col < b.col + b.len);
  for (const p of checkText(text, opts)) if (!(p.kind === 'replacement' && covered(p))) out.push(p);
  out.sort((a, b) => a.line - b.line || a.col - b.col);
  return out;
}

module.exports = { badUtf8, bytePositions, checkText, check };
