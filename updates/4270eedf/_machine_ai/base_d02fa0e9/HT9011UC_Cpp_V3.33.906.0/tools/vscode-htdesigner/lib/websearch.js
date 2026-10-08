'use strict';
// AI(W906-HTDESIGNER) 20260929: find web-side code: where a listener's function lives,
// which lines mention an element id, which command strings a handler sends.
// Plain Node (no vscode).

const fs = require('fs');
const path = require('path');
const { fileURLToPath } = require('url');
const { lineStartsOf, lineIndexOf } = require('./cpplex');

function escRe(s) { return String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }

/** Inline <script> bodies of an HTML page (not <script src>). */
function scriptRanges(html) {
  const out = [];
  const open = /<script\b[^>]*>/gi;
  const close = /<\/script\s*>/gi;
  let m;
  while ((m = open.exec(html))) {
    const s = m.index + m[0].length;
    close.lastIndex = s;
    const c = close.exec(html);
    const e = c ? c.index : html.length;
    if (!/\bsrc\s*=/i.test(m[0])) out.push(s, e);
    open.lastIndex = c ? c.index + c[0].length : html.length;
  }
  return out;
}

class Source {
  /** scriptOnly: an .html file, where only inline <script> bodies count as code. */
  constructor(file, text, scriptOnly) {
    this.file = file;
    this.text = text;
    this.ranges = scriptOnly ? scriptRanges(text) : null;
    this._ls = null;
  }
  get starts() { return this._ls || (this._ls = lineStartsOf(this.text)); }
  /** Inside a JS comment? (// … and /* … *\/; strings and template literals skipped.) */
  inComment(off) {
    if (!this._cm) {
      const t = this.text;
      const n = t.length;
      const cm = [];
      const scopes = this.ranges || [0, n];
      for (let r = 0; r < scopes.length; r += 2) {
        for (let i = scopes[r], end = scopes[r + 1]; i < end; i++) {
          const c = t[i];
          if (c === '/' && t[i + 1] === '/') { const e = t.indexOf('\n', i); const s = i; i = e < 0 || e > end ? end : e; cm.push(s, i); continue; }
          if (c === '/' && t[i + 1] === '*') { const e = t.indexOf('*/', i + 2); const s = i; i = e < 0 ? end : Math.min(end, e + 2); cm.push(s, i); i--; continue; }
          if (c === '"' || c === "'" || c === '`') {
            for (i++; i < end; i++) {
              if (t[i] === '\\') { i++; continue; }
              if (t[i] === c) break;
              if (c !== '`' && t[i] === '\n') break;
            }
          }
        }
      }
      this._cm = cm;
    }
    const cm = this._cm;
    let lo = 0, hi = (cm.length >> 1) - 1;
    while (lo <= hi) {
      const mid = (lo + hi) >> 1;
      if (off < cm[mid * 2]) hi = mid - 1;
      else if (off >= cm[mid * 2 + 1]) lo = mid + 1;
      else return true;
    }
    return false;
  }
  inScope(off) {
    if (!this.ranges) return true;
    for (let i = 0; i < this.ranges.length; i += 2) {
      if (off >= this.ranges[i] && off < this.ranges[i + 1]) return true;
    }
    return false;
  }
  hitAt(off) {
    const li = lineIndexOf(this.starts, off);
    const s = this.starts[li];
    let e = this.text.indexOf('\n', s);
    if (e < 0) e = this.text.length;
    let line = this.text.slice(s, e).replace(/\r$/, '');
    const c = off - s;
    if (line.length > 170) {
      const a = Math.max(0, c - 50);
      line = (a > 0 ? '…' : '') + line.slice(a, a + 160) + (a + 160 < line.length ? '…' : '');
    }
    return { file: this.file, line: li + 1, col: c + 1, snippet: line.trim() };
  }
}

const _fileCache = new Map();
/** Read a text file with a small mtime cache. */
function readText(file) {
  let st;
  try { st = fs.statSync(file); } catch (e) { return null; }
  const hit = _fileCache.get(file);
  if (hit && hit.mtime === st.mtimeMs) return hit.text;
  let text;
  try { text = fs.readFileSync(file, 'utf8'); } catch (e) { return null; }
  if (text.charCodeAt(0) === 0xfeff) text = text.slice(1);
  _fileCache.set(file, { mtime: st.mtimeMs, text });
  if (_fileCache.size > 200) _fileCache.delete(_fileCache.keys().next().value);
  return text;
}

function samePath(a, b) {
  return !!a && !!b && path.normalize(a).toLowerCase() === path.normalize(b).toLowerCase();
}

/** Every hit of `re` (global) across sources, in scope only. */
function findAll(sources, re, limit) {
  const out = [];
  for (const s of sources) {
    re.lastIndex = 0;
    let m;
    while ((m = re.exec(s.text))) {
      if (s.inScope(m.index)) {
        out.push(s.hitAt(m.index));
        if (out.length >= limit) return out;
      }
      if (m[0].length === 0) re.lastIndex++;
    }
  }
  return out;
}

/** Lines that name the element id as a string or a #selector. */
function idMentionRe(id) {
  const e = escRe(id);
  return new RegExp('([\'"`])#?' + e + '\\1|#' + e + '(?![\\w-])|\\[id=[\'"]?' + e + '[\'"]?\\]', 'g');
}

function identRe(name) {
  return new RegExp('\\b' + escRe(name) + '\\b', 'g');
}

/**
 * Where a listener function is written, from its source text (Function.toString).
 * V8 returns the exact source slice, so an exact indexOf finds it; line endings
 * may differ (the page's inline scripts go through the HTML parser: CRLF -> LF).
 */
function locateFn(sources, fnText, preferFile) {
  if (!fnText || /\{\s*\[native code\]\s*\}\s*$/.test(fnText)) return null;
  const variants = [];
  const add = v => { if (v && !variants.includes(v)) variants.push(v); };
  add(fnText);
  add(fnText.replace(/\r?\n/g, '\r\n'));
  add(fnText.replace(/\r\n/g, '\n'));
  if (fnText.length > 300) {
    const head = fnText.slice(0, 300);
    add(head); add(head.replace(/\r?\n/g, '\r\n')); add(head.replace(/\r\n/g, '\n'));
  }
  const order = preferFile
    ? sources.filter(s => samePath(s.file, preferFile)).concat(sources.filter(s => !samePath(s.file, preferFile)))
    : sources;
  for (const v of variants) {
    for (const s of order) {
      const i = s.text.indexOf(v);
      if (i >= 0 && s.inScope(i)) return s.hitAt(i);
    }
  }
  const first = fnText.split(/\r?\n/)[0].trim();
  if (first.length >= 16) {
    for (const s of order) {
      const i = s.text.indexOf(first);
      if (i >= 0 && s.inScope(i)) return Object.assign(s.hitAt(i), { approx: true });
    }
  }
  return null;
}

// The page scripts send commands to C++ through these helpers (measured 20260929 over
// web\page: cmd( 36, rawCmd( 20, raw( 19, run( 4) plus {cmd:'x.y'} envelopes.
const SENDERS = ['cmd', 'rawCmd', 'raw', 'run', 'sendCmd', 'sendCommand'];
const SEND_RE_SRC = '\\b(?:' + SENDERS.join('|') + ')\\s*\\(\\s*([\'"])([\\w.:-]+)\\1';

/** Command names a piece of JS sends: cmd('recipe.doc.put'), rawCmd('io.btnPanelClick'), {cmd:'x.y'} */
function extractCmds(text) {
  const out = [];
  if (!text) return out;
  const res = [new RegExp(SEND_RE_SRC, 'g'), /\bcmd\s*:\s*(['"])([\w.:-]+)\1/g];
  for (const re of res) {
    let m;
    while ((m = re.exec(text))) {
      const c = m[2];
      if (c.includes('.') && !/\.(html?|js|json|css|png|cpp|ini|csv|txt)$/i.test(c) && !out.includes(c)) out.push(c);
    }
  }
  return out;
}

// names that are never the page's own helpers
const JS_SKIP = new Set(('if for while switch return function catch typeof new delete void do else case ' +
  'Array Object String Number Boolean JSON Math Date Promise RegExp Map Set WeakMap Error Symbol ' +
  'parseInt parseFloat isNaN isFinite setTimeout setInterval clearTimeout clearInterval requestAnimationFrame ' +
  'console log warn error info debug push pop shift unshift slice splice concat join split indexOf lastIndexOf ' +
  'includes map filter forEach reduce some every find findIndex sort reverse keys values entries assign create ' +
  'stringify parse then finally resolve reject all race call apply bind toString trim replace replaceAll match ' +
  'test exec toFixed toLowerCase toUpperCase charAt charCodeAt substring substr startsWith endsWith padStart ' +
  'padEnd getElementById querySelector querySelectorAll closest contains matches addEventListener ' +
  'removeEventListener preventDefault stopPropagation stopImmediatePropagation getAttribute setAttribute ' +
  'removeAttribute hasAttribute appendChild removeChild insertBefore replaceChild createElement createTextNode ' +
  'cloneNode add remove toggle focus blur click select dispatchEvent getBoundingClientRect getComputedStyle ' +
  'scrollIntoView scrollTo fromCharCode isArray max min round floor ceil abs pow sqrt random now alert ' +
  'confirm prompt encodeURIComponent decodeURIComponent fetch open send postMessage hasOwnProperty ' +
  'getItem setItem removeItem has get set delete clear item defineProperty freeze').split(/\s+/));

// a bare call of these is still never a page helper (keywords, globals); a bare call of a DOM method NAME is:
// AI(W906-HTDESIGNER) 20260930: the IO page's own send(el, a, down) (ht9045_io_do.js:173) -- skipped as if it were
// WebSocket.send, so its click never traced to io.btnPanelClick (EastSun: "應該有click吧? 他是連到C++的")
const FREE_SKIP = new Set(('if for while switch return function catch typeof new delete void do else case ' +
  'Array Object String Number Boolean JSON Math Date Promise RegExp Map Set WeakMap Error Symbol ' +
  'parseInt parseFloat isNaN isFinite setTimeout setInterval clearTimeout clearInterval requestAnimationFrame ' +
  'alert confirm prompt encodeURIComponent decodeURIComponent fetch').split(/\s+/));

/** Functions a piece of JS calls, in order: save(), R.put(), … (JS builtins and DOM methods left out). */
function calledNames(text) {
  const out = [];
  if (!text) return out;
  const re = /(^|[^\w$])([A-Za-z_$][\w$]*)\s*\(/g;
  let m;
  while ((m = re.exec(text)) && out.length < 25) {
    const name = m[2];
    const method = m[1] === '.';
    if ((method ? JS_SKIP.has(name) : FREE_SKIP.has(name)) || name.length < 2 || out.includes(name)) continue;
    out.push(name);
  }
  return out;
}

/** Offset just past the "}" matching the "{" at `open` (strings, comments, templates skipped). */
function braceEnd(text, open) {
  let depth = 0;
  const n = Math.min(text.length, open + 60000);
  for (let i = open; i < n; i++) {
    const c = text[i];
    if (c === '/' && text[i + 1] === '/') { const e = text.indexOf('\n', i); i = e < 0 ? n : e; continue; }
    if (c === '/' && text[i + 1] === '*') { const e = text.indexOf('*/', i + 2); i = e < 0 ? n : e + 1; continue; }
    if (c === '"' || c === "'" || c === '`') {
      for (i++; i < n; i++) {
        if (text[i] === '\\') { i++; continue; }
        if (text[i] === c) break;
        if (c !== '`' && text[i] === '\n') break;
      }
      continue;
    }
    if (c === '{') depth++;
    else if (c === '}') { depth--; if (depth === 0) return i + 1; }
  }
  return n;
}

/**
 * Where `name` is defined as a function in the page sources: [{ source, off, body }].
 * With `preferFile`: a definition in that file wins (JS scope: the caller's own
 * `function save()` is the one called, not another page script's `save`).
 */
function findFnDefs(sources, name, limit, preferFile) {
  if (preferFile) {
    const own = findFnDefs(sources.filter(s => samePath(s.file, preferFile)), name, limit, null);
    if (own.length) return own;
  }
  const e = escRe(name);
  const re = new RegExp(
    '\\bfunction\\s+' + e + '\\s*\\(' +                                     // function name(
    '|(?:^|[^\\w$])' + e + '\\s*[:=]\\s*(?:async\\s+)?function\\b' +       // name: function / X.name = function
    '|(?:^|[^\\w$])' + e + '\\s*[:=]\\s*(?:async\\s*)?(?:\\([^()]*\\)|[A-Za-z_$][\\w$]*)\\s*=>' + // arrow
    '|^[ \\t]*(?:async\\s+)?' + e + '\\s*\\([^()]*\\)\\s*\\{',              // method shorthand
    'gm');
  const out = [];
  for (const s of sources) {
    re.lastIndex = 0;
    let m;
    while ((m = re.exec(s.text)) && out.length < (limit || 4)) {
      if (!s.inScope(m.index)) continue;
      const open = s.text.indexOf('{', m.index + m[0].length - 1);
      if (open < 0 || open - m.index > 400) continue;
      const end = braceEnd(s.text, open);
      out.push({ source: s, off: m.index + (m[0].length - m[0].trimStart().length), body: s.text.slice(open, end) });
    }
  }
  return out;
}

/**
 * Follow a handler into the functions it calls (depth-limited) and collect the
 * command names that end up being sent. Returns [{ cmd, via: ['save', 'putDoc'] }].
 */
/**
 * Commands a sender gets through a name: raw(CMD, …) with `var CMD = 'act.main.closeProgram'` in the page's scripts
 * (ht9045_main_close.js:40 -- the Exit button's C++ was never found: EastSun 20260930 "BCB 就可以對應到
 * TfMain::sbCloseProgramClick，你這邊怎不行對應到?"). The script of the text first, then the others.
 */
function namedCmds(sources, text, file) {
  const out = [];
  const re = new RegExp('\\b(?:' + SENDERS.join('|') + ')\\s*\\(\\s*([A-Za-z_$][\\w$]*)\\s*[,)]', 'g');
  const names = [];
  let m;
  while ((m = re.exec(String(text || '')))) if (!names.includes(m[1])) names.push(m[1]);
  if (!names.length) return out;
  const ordered = file ? sources.filter(s => samePath(s.file, file)).concat(sources.filter(s => !samePath(s.file, file))) : sources;
  for (const name of names) {
    const def = new RegExp('\\b(?:var|let|const)\\s+' + escRe(name) + '\\s*=\\s*([\'"])([\\w.:-]+)\\1');
    for (const s of ordered) {
      const d = def.exec(s.text);
      if (d && d[2].includes('.') && !out.includes(d[2])) { out.push(d[2]); break; }
    }
  }
  return out;
}

function traceCmds(sources, rootText, maxDepth, rootFile) {
  const found = [];
  const seen = new Set();
  const visit = (text, via, depth, file) => {
    for (const c of extractCmds(text).concat(namedCmds(sources, text, file))) {
      if (!found.some(f => f.cmd === c)) found.push({ cmd: c, via: via.slice() });
    }
    if (depth >= (maxDepth || 3) || found.length >= 12) return;
    for (const name of calledNames(text)) {
      const key = name + '|' + (file || '');
      if (SENDERS.includes(name) || seen.has(key)) continue;
      seen.add(key);
      for (const d of findFnDefs(sources, name, 3, file)) visit(d.body, via.concat([name]), depth + 1, d.source.file);
    }
  };
  visit(rootText || '', [], 0, rootFile || null);
  return found;
}

/** The line that sends `cmd` (cmd('x.y' / rawCmd('x.y' / {cmd:'x.y'}), for "go to". */
function findCmdSend(sources, cmd) {
  const out = [];
  for (const s of sources) {
    for (const x of sendsIn(s)) {      // comments skipped
      if (x.cmd === cmd) out.push(x.hit);
      if (out.length >= 3) return out;
    }
  }
  return out;
}

/**
 * Stack-frame URL -> local file. Webview resources look like
 *   https://file+.vscode-resource.vscode-cdn.net/d%3A/HT9045/.../x.js
 * Anything else (the webview document itself) is the page's inline script -> null.
 */
function mapFrameUrl(url) {
  if (!url) return null;
  try {
    const u = new URL(url);
    if (u.protocol === 'file:') return fileURLToPath(u);
    if (/vscode-resource/i.test(u.hostname)) {
      let p = decodeURIComponent(u.pathname);
      if (/^\/[a-zA-Z]:[\\/]/.test(p)) p = p.slice(1);
      return path.normalize(p);
    }
  } catch (e) { /* not a URL */ }
  return null;
}

/** id of the start tag the cursor sits in ("@form" for the generated .form root). */
function idAtOffset(text, off) {
  const s = text.lastIndexOf('<', off);
  if (s < 0) return null;
  const g = text.indexOf('>', s);
  if (g < 0 || off > g) return null;
  const tag = text.slice(s, g + 1);
  if (/^<\//.test(tag) || /^<!/.test(tag)) return null;
  const m = /\bid\s*=\s*["']([^"']+)["']/.exec(tag);
  if (m) return m[1];
  if (/^<div\b[^>]*\bclass\s*=\s*["']form["']/.test(tag)) return '@form';
  return null;
}

/** Offset range of id="X" in the page (or the .form root), for "go to HTML". */
function findIdAttr(text, id) {
  if (id === '@form') {
    const m = /<div\b[^>]*\bclass\s*=\s*["']form["']/.exec(text);
    return m ? { start: m.index, end: m.index + 4 } : null;
  }
  const re = new RegExp('\\bid\\s*=\\s*["\']' + escRe(id) + '["\']');
  const m = re.exec(text);
  return m ? { start: m.index, end: m.index + m[0].length } : null;
}

/** title="…" of the element's start tag in the page source (page JS may strip it at runtime). */
function titleOf(text, id) {
  if (!id) return '';
  const at = findIdAttr(text, id);
  if (!at) return '';
  const s = text.lastIndexOf('<', at.start);
  const e = text.indexOf('>', at.end);
  if (s < 0 || e < 0) return '';
  const m = /\btitle\s*=\s*"([^"]*)"/.exec(text.slice(s, e + 1));
  if (!m) return '';
  return m[1].replace(/&quot;/g, '"').replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&amp;/g, '&');
}

/** Does handler source text refer to this CSS class (".exitbtn", 'btnpanel', …)? */
function mentionsClass(text, cls) {
  if (!text || !cls) return false;
  return text.includes('.' + cls) || text.includes("'" + cls + "'") || text.includes('"' + cls + '"');
}

/** Every command a source sends, with where: [{ cmd, hit }] (hit = file/line/col/snippet). */
function sendsIn(source) {
  const out = [];
  const res = [new RegExp(SEND_RE_SRC, 'g'), /\bcmd\s*:\s*(['"])([\w.:-]+)\1/g];
  for (const re of res) {
    let m;
    while ((m = re.exec(source.text))) {
      const c = m[2];
      if (!c.includes('.') || /\.(html?|js|json|css|png|cpp|ini|csv|txt)$/i.test(c)) continue;
      if (!source.inScope(m.index) || source.inComment(m.index)) continue;
      out.push({ cmd: c, hit: source.hitAt(m.index) });
    }
  }
  return out;
}

/**
 * The wire scripts' field maps (2,504 rows, 20260929). Two shapes:
 *   hand-written  XST1: ['Hotplate Form', 'X Start'(, 'note')]      [section, key, note]
 *   generated     XST1: ['hotPlate', 'Hotplate Form', 'X Start']     [doc, section, key]
 *                 (files that call HT9045Wire.register -- gen_wire.py output)
 * Returns [{ id, doc, section, key, note, hit }] (comments skipped).
 */
function fieldRowsIn(source) {
  const out = [];
  const generated = /\b[A-Z][A-Za-z0-9_]*Wire\.register\s*\(/.test(source.text);   // (HT9045Wire, or the same library under another name)
  const re = /(^|[\s{,])([A-Za-z_]\w*)\s*:\s*\[\s*(['"])([^'"\n]{1,80})\3\s*,\s*(['"])([^'"\n]{1,80})\5\s*(?:,\s*(['"])([^'"\n]{0,120})\7)?/g;
  let m;
  while ((m = re.exec(source.text))) {
    const at = m.index + m[1].length;
    if (!source.inScope(at) || source.inComment(at)) continue;
    const three = m[7] !== undefined;
    const row = generated && three
      ? { id: m[2], doc: m[4], section: m[6], key: m[8], note: '' }
      : { id: m[2], doc: '', section: m[4], key: m[6], note: three ? m[8] : '' };
    if (!row.key) continue;
    row.hit = source.hitAt(at);
    out.push(row);
  }
  return out;
}

/**
 * The wire scripts' tag maps: `'machine.state': ['palMainStatus', 'text'],` = the value
 * C++ publishes as tag machine.state is shown in control palMainStatus (as text).
 * (94 rows, 20260929: text / site / chip / map / select / led.)
 * Returns [{ tag, id, prop, hit }] (comments skipped).
 */
function tagRowsIn(source) {
  const out = [];
  const re = /(['"])([a-z][\w]*\.[\w.]+)\1\s*:\s*\[\s*(['"])([A-Za-z_]\w*)\3\s*,\s*(['"])(\w+)\5/g;
  let m;
  while ((m = re.exec(source.text))) {
    if (!source.inScope(m.index) || source.inComment(m.index)) continue;
    out.push({ tag: m[2], id: m[4], prop: m[6], hit: source.hitAt(m.index) });
  }
  return out;
}

module.exports = {
  Source, scriptRanges, readText, samePath, findAll, idMentionRe, identRe, locateFn,
  extractCmds, calledNames, findFnDefs, traceCmds, findCmdSend, sendsIn, fieldRowsIn, tagRowsIn, braceEnd,
  mapFrameUrl, idAtOffset, findIdAttr, titleOf, mentionsClass, escRe, SENDERS,
};
