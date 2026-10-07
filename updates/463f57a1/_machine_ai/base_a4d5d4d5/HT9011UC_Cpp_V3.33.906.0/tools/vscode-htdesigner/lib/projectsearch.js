'use strict';
// AI(W906-HTDESIGNER) 20261001 (0.137, EastSun: "我沒辦法整個專案查詢關鍵字並列出來"): 專案搜尋 -- one keyword over the
// whole project: the C++ port tree, the web tree and the BCB6 golden tree, every hit listed (area -> file -> line).
// VS Code's own Find in Files reads every file as UTF-8, so the golden tree (Big5: 186 of its 194 .cpp are not valid
// UTF-8) comes out garbled and a Chinese keyword never matches there. Here each file is decoded the way it is written:
// the golden tree as Big5, the others as UTF-8 -- and a file of them that is not valid UTF-8 as Big5 too.
// Plain Node (tests run it without VS Code).
const fsp = require('fs/promises');
const path = require('path');
const { mask } = require('./cppstub');

// what is text worth searching (sources, pages, forms, settings, notes)
const EXT = new Set(['.cpp', '.h', '.hpp', '.c', '.cc', '.cxx', '.inc', '.inl', '.dfm', '.bpr', '.rc', '.js', '.mjs', '.cjs',
  '.html', '.htm', '.css', '.json', '.ini', '.csv', '.txt', '.md', '.ps1', '.bat', '.cmd', '.cmake', '.xml', '.def', '.tsv']);
const NAMES = new Set(['CMakeLists.txt', 'Makefile']);
// build output, other people's code, archives, version control, generated IR
// AI(W906-HTDESIGNER) 20261002 (machine): .claude too -- agents' git worktrees in <tree>\.claude\worktrees\ are whole
// copies of the tree (every hit found again in them; the Solution Explorer listed them)
const SKIP_DIR = /^(build.*|third_party|\.git|\.svn|\.vs|\.vscode|\.claude|node_modules|__pycache__|_archive.*|_backup.*|ir_out|scratchpad|dist|out|obj|Debug|Release|\.npm-cache)$/i;
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
  // (1006 audit: 正規式 ^ / $ = the start / end of a LINE, as in Visual Studio -- without m they meant the whole file)
  try { return { re: new RegExp(src, (o.caseSensitive ? 'g' : 'gi') + (o.regex ? 'm' : '')) }; } catch (e) { return { error: '正規式寫錯了：' + e.message }; }
}

// AI(W906-HTDESIGNER) 20261003 (machine, EastSun: "你搜尋關鍵字 是用並列運算 去搜檔案嗎 怎感覺很慢"): it was one file at a
// time -- 4,364 files / 217 MB took 12 s, nearly all of it waiting for the disk (decoding + matching them all: 0.3 s).
// Now this many reads / directory listings are in flight at once (32 at once: 2.9 s here).
const PARALLEL = 32;
/** Run fn(item, i) over items, `n` at a time; stops taking new ones once stop() says so. */
async function pool(items, n, fn, stop) {
  let next = 0;
  const worker = async () => { while (next < items.length && !(stop && stop())) { const i = next++; await fn(items[i], i); } };
  await Promise.all(Array.from({ length: Math.min(n, items.length) }, worker));
}

/** The files to search under one root (sorted, so a run is repeatable). */
async function listFiles(root, stop) {
  const out = [];
  let level = [root];
  // (one directory level at a time, its directories listed together)
  while (level.length && !(stop && stop())) {
    const nextLevel = [];
    await pool(level, PARALLEL, async d => {
      let ents;
      try { ents = await fsp.readdir(d, { withFileTypes: true }); } catch (e) { return; }
      for (const e of ents) {
        const p = path.join(d, e.name);
        if (e.isDirectory()) { if (!SKIP_DIR.test(e.name)) nextLevel.push(p); } else if (e.isFile() && (EXT.has(path.extname(e.name).toLowerCase()) || NAMES.has(e.name))) out.push(p);
      }
    }, stop);
    level = nextLevel;
  }
  return out.sort((a, b) => a.localeCompare(b));
}

/**
 * The hits in one text: [{ line, col, len, text, at, lc }] (line / col 1-based; text = the line, trimmed to 300 chars
 * around; at = the offset in the text; lc = the column in the whole line, 0-based). More than max: out.more = true.
 * 1006 audit: a lone CR ends a line too (the editor counts it -- the line numbers were off after one).
 */
function hitsIn(text, re, max) {
  const out = [];
  re.lastIndex = 0;
  let m, ls = 0, ln = 1;
  let nl = text.indexOf('\n'), cr = text.indexOf('\r');
  // (the next line break at / after from: [its start, the start of the next line] or null)
  const brk = from => {
    if (nl >= 0 && nl < from) nl = text.indexOf('\n', from);
    if (cr >= 0 && cr < from) cr = text.indexOf('\r', from);
    const b = nl < 0 ? cr : cr < 0 ? nl : Math.min(nl, cr);
    if (b < 0) return null;
    return [b, b === cr && text[b + 1] === '\n' ? b + 2 : b + 1];
  };
  while ((m = re.exec(text))) {
    if (m[0].length === 0) { re.lastIndex++; continue; }
    if (out.length >= max) { out.more = true; break; }
    // (the line of the hit: counted forward from the last one)
    for (let b = brk(ls); b && b[0] < m.index; b = brk(ls)) { ln++; ls = b[1]; }
    const e = brk(m.index);
    let line = text.slice(ls, e ? e[0] : text.length);
    let col = m.index - ls;
    const lc = col;
    // a long line (generated files): the part around the hit
    if (line.length > 300) { const a = Math.max(0, col - 120); line = (a ? '…' : '') + line.slice(a, a + 300); col = col - a + (a ? 1 : 0); }
    out.push({ line: ln, col: col + 1, len: m[0].length, text: line, at: m.index, lc });
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
  // (1006 audit: listing a big tree can be cancelled too)
  const stopList = () => !!(o.cancelled && o.cancelled());
  for (const r of roots) if (r && r.root) lists.push({ r, files: Array.isArray(r.files) ? r.files.slice() : await listFiles(r.root, stopList) });
  const total = lists.reduce((n, l) => n + l.files.length, 0);
  // (read PARALLEL files at once; each file's hits kept in its own slot, put together in file order at the end, so the
  //  list is the same as one-at-a-time -- the order of the roots, then of the files)
  const jobs = [];
  for (const { r, files } of lists) for (const f of files) jobs.push({ r, f });
  const slots = new Array(jobs.length);
  let scanned = 0, found = 0, cancelled = stopList(), capped = 0;
  // (each match is its own RegExp: lastIndex is per search)
  const reOf = () => new RegExp(re.source, re.flags);
  await pool(jobs, PARALLEL, async ({ r, f }, i) => {
    if (o.cancelled && o.cancelled()) { cancelled = true; return; }
    let buf;
    try {
      const st = await fsp.stat(f);
      if (st.size > MAX_FILE) { scanned++; return; }
      buf = await fsp.readFile(f);
    } catch (e) { scanned++; return; }
    scanned++;
    if (o.progress && scanned % 200 === 0) o.progress(scanned, total);
    if (buf.indexOf(0) >= 0 && buf.indexOf(0) < 4096) return;   // binary
    const { text, enc } = decode(buf, r.kind);
    let hs = hitsIn(text, reOf(), o.perFile);
    // (1006 audit: a file with more than perFile hits said nothing -- the list now says "+")
    if (hs.more) capped++;
    if (!hs.length) return;
    // (the filters -- assigned / a condition / a function: any one checked = those uses only, comments and strings out;
    //  each hit says which it is)
    const kinds = o.kinds && (o.kinds.assign || o.kinds.cond || o.kinds.func) ? o.kinds : null;
    if (kinds) {
      const masked = mask(text);
      hs = hs.filter(h => {
        const c = classify(masked, text, h.at, h.len);
        if (!c.code) return false;
        h.kinds = ['assign', 'cond', 'func'].filter(k => c[k]);
        return h.kinds.some(k => kinds[k]);
      });
      if (!hs.length) return;
    }
    slots[i] = { r, f, enc, hs };
    found += hs.length;
  }, () => cancelled || found >= o.limit || !!(o.cancelled && o.cancelled() && (cancelled = true)));
  const hits = [];
  let withHits = 0, truncated = cancelled || capped > 0;
  for (const s of slots) {
    if (!s) continue;
    if (hits.length >= o.limit) { truncated = true; break; }
    withHits++;
    const rel = path.relative(s.r.root, s.f);
    for (const h of s.hs) {
      if (hits.length >= o.limit) { truncated = true; break; }
      hits.push(Object.assign({ area: s.r.area, file: s.f, rel, enc: s.enc }, h));
    }
  }
  // (the limit reached while files were still unread: more may be there)
  if (found >= o.limit && scanned < jobs.length) truncated = true;
  const res = { hits, files: withHits, scanned, total, truncated, ms: Date.now() - t0 };
  if (capped) res.capped = capped;
  if (cancelled) res.cancelled = true;
  return res;
}

/**
 * AI(W906-HTDESIGNER) 20261003 (machine, EastSun: "可以加入讓我勾選篩選條件嗎? 我想加入 被賦予值 或是被當成判斷式 或是是函式 的篩選"):
 * how the name at offset `at` (length `len`) is used, from the text with comments / strings blanked (`masked`, same
 * offsets -- cppstub.mask) -> { code: false } in a comment / string, else { code: true, assign, cond, func }:
 *   assign: `x = …` (not ==), `x += …` (any compound), `x++` / `++x` / `x--` / `--x` -- an index / member after the
 *           name is skipped first (x[i] = …, x.y = …);
 *   cond:   inside the ( ) of if / while / for / switch, or next to == != <= >= < > && || or a ! before it, or a ? after;
 *   func:   a ( right after the name (a call, a definition, a declaration).
 */
function classify(masked, text, at, len) {
  const m = masked, n = m.length;
  if (m[at] !== text[at]) return { code: false };
  const isSp = c => c === ' ' || c === '\t' || c === '\r' || c === '\n';
  let e = at + len;
  // (the name may go on past the hit: "iHome" found inside "iHomeLed" is not iHome)
  while (e < n && /[A-Za-z0-9_]/.test(m[e])) e++;
  let j = e;
  while (j < n && isSp(m[j])) j++;
  const func = m[j] === '(';
  // past an index / member: x[i] = ..., x.y = ..., x->y = ...
  let k = j;
  for (let guard = 0; guard < 20; guard++) {
    if (m[k] === '[') { let d = 0; for (; k < n; k++) { if (m[k] === '[') d++; else if (m[k] === ']') { d--; if (!d) { k++; break; } } } }
    else if (m[k] === '.' || (m[k] === '-' && m[k + 1] === '>')) { k += m[k] === '.' ? 1 : 2; while (k < n && /[A-Za-z0-9_]/.test(m[k])) k++; }
    else break;
    while (k < n && isSp(m[k])) k++;
  }
  const after2 = m.slice(k, k + 3);
  let b = at - 1;
  while (b >= 0 && isSp(m[b])) b--;
  const before2 = m.slice(Math.max(0, b - 1), b + 1);
  const assign = (/^=[^=]/.test(after2) || /^(\+|-|\*|\/|%|&|\||\^)=/.test(after2) || /^(<<|>>)=/.test(after2) || /^(\+\+|--)/.test(after2) ||
    before2 === '++' || before2 === '--');
  let cond = /^(==|!=|<=|>=|&&|\|\|)/.test(after2) || (/^[<>][^<>=]/.test(after2) && !func) || /^\?/.test(after2) ||
    /(==|!=|<=|>=|&&|\|\|)$/.test(before2) || (m[b] === '!' && m[b + 1] !== '=') || ((m[b] === '<' || m[b] === '>') && m[b - 1] !== m[b] && m[b - 1] !== '-');
  if (!cond) {
    // the ( ) it sits in, back to the start of its statement: if / while / for / switch before it
    let d = 0;
    for (let i = at - 1; i >= 0 && at - i < 4000; i--) {
      const c = m[i];
      if (c === ')') d++;
      else if (c === '(') {
        if (d) { d--; continue; }
        let w = i - 1;
        while (w >= 0 && isSp(m[w])) w--;
        let s = w;
        while (s >= 0 && /[A-Za-z0-9_]/.test(m[s])) s--;
        const word = m.slice(s + 1, w + 1);
        if (/^(if|while|for|switch)$/.test(word)) { cond = true; break; }
      } else if ((c === '{' || c === '}') && !d) break;   // (not at ';': for (a; b; c) has them inside)
    }
  }
  return { code: true, assign: !!assign, cond: !!cond, func: !!func };
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

/** 1006 audit: a file of a BCB6 source tree (HT9011UC_Code_*, the Big5 golden / 899 / 912 ones) -- never replaced in. */
function isBcb6(file) { return /[\\/]HT9011UC_Code_[^\\/]*[\\/]/i.test(String(file || '')); }
/** 1006 audit: a replacement text with $1 / $<name> / $& / $$ filled from one match (as String.replace does). */
function expand(text, mm) {
  return String(text).replace(/\$(\$|&|<([^>]*)>|(\d{1,2}))/g, (all, k, name, num) => {
    if (k === '$') return '$';
    if (k === '&') return mm[0];
    if (name !== undefined) return mm.groups && mm.groups[name] != null ? mm.groups[name] : (mm.groups ? '' : all);
    let n = +num;
    if (n >= mm.length && num.length === 2 && +num[0] < mm.length) return (mm[+num[0]] || '') + num[1];
    return n > 0 && n < mm.length ? (mm[n] == null ? '' : mm[n]) : all;
  });
}

module.exports = { decode, makeRe, listFiles, hitsIn, search, classify, asText, areaOf, scopeRoots, isBcb6, expand, EXT, SKIP_DIR };
