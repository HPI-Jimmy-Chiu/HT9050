'use strict';
// AI(W906-HTDESIGNER) 20261001 (0.137, EastSun: "我沒辦法整個專案查詢關鍵字並列出來"): 專案搜尋 -- one keyword over the
// whole project: the C++ port tree, the web tree and the BCB6 golden tree, every hit listed (area -> file -> line).
// VS Code's own Find in Files reads every file as UTF-8, so the golden tree (Big5: 186 of its 194 .cpp are not valid
// UTF-8) comes out garbled and a Chinese keyword never matches there. Here each file is decoded the way it is written:
// the golden tree as Big5, the others as UTF-8 -- and a file of them that is not valid UTF-8 as Big5 too.
// Plain Node (tests run it without VS Code).
const fsp = require('fs/promises');
const path = require('path');

// what is text worth searching (sources, pages, forms, settings, notes)
const EXT = new Set(['.cpp', '.h', '.hpp', '.c', '.cc', '.cxx', '.inc', '.inl', '.dfm', '.bpr', '.rc', '.js', '.mjs', '.cjs',
  '.html', '.htm', '.css', '.json', '.ini', '.csv', '.txt', '.md', '.ps1', '.bat', '.cmd', '.cmake', '.xml', '.def', '.tsv']);
const NAMES = new Set(['CMakeLists.txt', 'Makefile']);
// build output, other people's code, archives, version control, generated IR
const SKIP_DIR = /^(build.*|third_party|\.git|\.svn|\.vs|\.vscode|node_modules|__pycache__|_archive.*|_backup.*|ir_out|scratchpad|dist|out|obj|Debug|Release|\.npm-cache)$/i;
const MAX_FILE = 8 * 1024 * 1024;

const utf8 = new TextDecoder('utf-8', { fatal: true });
let big5 = null;
/** -> { text, enc: 'utf8' | 'big5' } -- golden = Big5; else UTF-8, or Big5 when the bytes are not valid UTF-8. */
function decode(buf, kind) {
  if (!big5) big5 = new TextDecoder('big5');
  if (kind === 'golden') return { text: big5.decode(buf), enc: 'big5' };
  try { return { text: utf8.decode(buf).replace(/^﻿/, ''), enc: 'utf8' }; } catch (e) { return { text: big5.decode(buf), enc: 'big5' }; }
}

function escRe(s) { return String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }

/**
 * The search as a RegExp: plain text by default (VS Code's Find in Files: case-insensitive, any part of a word);
 * opts { caseSensitive, wholeWord, regex }. -> { re } | { error }
 */
function makeRe(q, opts) {
  const o = opts || {};
  const s = String(q || '');
  if (!s.trim()) return { error: '沒有輸入要找的字' };
  let src = o.regex ? s : escRe(s);
  // (a whole word: not inside a longer identifier -- \b does not know CJK, so letters / digits / _ on each side)
  if (o.wholeWord) src = '(?<![A-Za-z0-9_])(?:' + src + ')(?![A-Za-z0-9_])';
  try { return { re: new RegExp(src, o.caseSensitive ? 'g' : 'gi') }; } catch (e) { return { error: '正規式寫錯了：' + e.message }; }
}

/** The files to search under one root (sorted, so a run is repeatable). */
async function listFiles(root) {
  const out = [];
  const stack = [root];
  while (stack.length) {
    const d = stack.pop();
    let ents;
    try { ents = await fsp.readdir(d, { withFileTypes: true }); } catch (e) { continue; }
    for (const e of ents) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (!SKIP_DIR.test(e.name)) stack.push(p); } else if (e.isFile() && (EXT.has(path.extname(e.name).toLowerCase()) || NAMES.has(e.name))) out.push(p);
    }
  }
  return out.sort((a, b) => a.localeCompare(b));
}

/** The hits in one text: [{ line, col, len, text }] (line / col 1-based; text = the line, trimmed to 300 chars around). */
function hitsIn(text, re, max) {
  const out = [];
  re.lastIndex = 0;
  let m, ls = 0, ln = 1, scan = 0;
  while ((m = re.exec(text)) && out.length < max) {
    if (m[0].length === 0) { re.lastIndex++; continue; }
    // (the line of the hit: counted forward from the last one)
    for (let i = text.indexOf('\n', scan); i >= 0 && i < m.index; i = text.indexOf('\n', i + 1)) { ln++; ls = i + 1; }
    scan = m.index;
    const le = text.indexOf('\n', m.index);
    let line = text.slice(ls, le < 0 ? text.length : le).replace(/\r$/, '');
    let col = m.index - ls;
    // a long line (generated files): the part around the hit
    if (line.length > 300) { const a = Math.max(0, col - 120); line = (a ? '…' : '') + line.slice(a, a + 300); col = col - a + (a ? 1 : 0); }
    out.push({ line: ln, col: col + 1, len: m[0].length, text: line });
  }
  return out;
}

/**
 * Search the roots. roots: [{ area, label, root, kind }] (kind 'golden' = Big5). opts { limit, perFile, cancelled(),
 * progress(done, total) }. -> { hits: [{ area, file, rel, enc, line, col, len, text }], files, scanned, truncated, ms }
 */
async function search(roots, re, opts) {
  const o = Object.assign({ limit: 5000, perFile: 500 }, opts || {});
  const t0 = Date.now();
  const lists = [];
  // (a root with files: only those -- the "current file" scope)
  for (const r of roots) if (r && r.root) lists.push({ r, files: Array.isArray(r.files) ? r.files.slice() : await listFiles(r.root) });
  const total = lists.reduce((n, l) => n + l.files.length, 0);
  const hits = [];
  let scanned = 0, withHits = 0, truncated = false;
  for (const { r, files } of lists) {
    for (const f of files) {
      if (o.cancelled && o.cancelled()) return { hits, files: withHits, scanned, truncated: true, cancelled: true, ms: Date.now() - t0 };
      scanned++;
      if (o.progress && scanned % 200 === 0) o.progress(scanned, total);
      let buf;
      try {
        const st = await fsp.stat(f);
        if (st.size > MAX_FILE) continue;
        buf = await fsp.readFile(f);
      } catch (e) { continue; }
      if (buf.indexOf(0) >= 0 && buf.indexOf(0) < 4096) continue;   // binary
      const { text, enc } = decode(buf, r.kind);
      const hs = hitsIn(text, re, Math.min(o.perFile, o.limit - hits.length));
      if (!hs.length) continue;
      withHits++;
      const rel = path.relative(r.root, f);
      for (const h of hs) hits.push(Object.assign({ area: r.area, file: f, rel, enc }, h));
      if (hits.length >= o.limit) { truncated = true; break; }
    }
    if (truncated) break;
  }
  return { hits, files: withHits, scanned, total, truncated, ms: Date.now() - t0 };
}

/** The results as plain text (one line per hit), for the clipboard / the output panel. */
function asText(q, res, roots) {
  const L = ['專案搜尋「' + q + '」：' + res.hits.length + (res.truncated ? '+' : '') + ' 筆，' + res.files + ' 個檔（掃了 ' + res.scanned + ' 個，' + res.ms + ' ms）'];
  for (const r of roots) {
    const hs = res.hits.filter(h => h.area === r.area);
    if (!hs.length) continue;
    L.push('', '== ' + r.label + '（' + r.root + '）' + hs.length + ' 筆');
    for (const h of hs) L.push(h.rel + ':' + h.line + ':' + h.col + '  ' + h.text.trim());
  }
  return L.join('\n');
}

/** Which of the roots holds `file` (the deepest one: the port tree's tools\ are not the web tree's). -> root entry | null */
function areaOf(roots, file) {
  const f = path.resolve(String(file || '')).toLowerCase();
  let best = null;
  for (const r of roots) {
    if (!r || !r.root) continue;
    const rr = path.resolve(r.root).toLowerCase();
    if ((f === rr || f.startsWith(rr + path.sep)) && (!best || rr.length > path.resolve(best.root).length)) best = r;
  }
  return best;
}

/**
 * The roots for a scope (Visual Studio's Find in Files "Look in"): 'solution' = all, 'project' = the one holding
 * `file`, 'file' = only `file` (in its root, so it is decoded the way that root is). -> { roots, label } | { error }
 */
function scopeRoots(roots, scope, file) {
  if (scope === 'solution' || !scope) return { roots, label: '整個方案' };
  const a = file ? areaOf(roots, file) : null;
  if (scope === 'project') return a ? { roots: [a], label: a.label } : { error: '目前的檔案不在任何一個專案（C++ 移植樹、網頁、BCB6 原始碼）裡' };
  // (a folder of a project -- the Solution Explorer's folder: decoded as its project)
  if (scope === 'folder') return a ? { roots: [Object.assign({}, a, { root: file })], label: a.label + ' ' + path.basename(file) + '\\' } : { error: '這個資料夾不在任何一個專案裡' };
  if (scope === 'file') {
    if (!file) return { error: '沒有開著的檔案' };
    const r = a || { area: 'other', label: '其他', root: path.dirname(file), kind: /\.(cpp|h|dfm|bpr)$/i.test(file) ? 'port' : 'web' };
    return { roots: [Object.assign({}, r, { files: [file] })], label: path.basename(file) };
  }
  return { error: '不認得的範圍：' + scope };
}

module.exports = { decode, makeRe, listFiles, hitsIn, search, asText, areaOf, scopeRoots, EXT, SKIP_DIR };
