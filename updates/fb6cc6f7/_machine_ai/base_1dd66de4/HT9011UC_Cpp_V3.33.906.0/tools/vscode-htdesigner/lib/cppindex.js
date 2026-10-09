'use strict';
// AI(W906-HTDESIGNER) 20260929: find "TfHotPlate::spbSaveClick" in a source tree.
//
// One pass over the tree records every "T<Name>::<member>" (the VCL form classes all
// start with T). A lookup then reads only the few files that hit and classifies each
// hit: definition / comment mention / other, and (port tree only) inside #if 0.
// Also: find a command string literal ("io.btnPanelClick") for the web -> C++ hop.
// Plain Node (no vscode).

const fs = require('fs');
const fsp = require('fs/promises');
const path = require('path');
const lex = require('./cpplex');

const EXT = new Set(['.cpp', '.h', '.hpp', '.c', '.cc', '.cxx', '.inc', '.inl']);
// (the orders of findString / findFuncDefs -- also used when one file's hits are put back after a save)
const STR_RANK = h => (h.dispatch ? (h.weak ? 0.5 : 0) : h.comment ? 2 : 1) + (h.dead ? 3 : 0) + (h.test ? 6 : 0);
const STR_ORDER = (a, b) => STR_RANK(a) - STR_RANK(b) || a.file.localeCompare(b.file) || a.line - b.line;
const FN_ORDER = (a, b) => (a.dead - b.dead) || (a.test - b.test) || a.file.localeCompare(b.file) || a.line - b.line;
// AI(W906-HTDESIGNER) 20261002 (machine): .claude too -- agents' git worktrees live in <tree>\.claude\worktrees\ (whole
// copies of the tree: 2,376 more .cpp / .h on 20261001 22:25), every function found twice and the index 10x slower
const SKIP_DIR = /^(build.*|third_party|\.git|\.svn|\.vs|\.vscode|\.claude|node_modules|__pycache__|_archive.*|_backup.*|ir_out|scratchpad|ship|dist|web-overlay|vscode-htdesigner)$/i;

let big5 = null, utf8 = null;
/**
 * A golden (BCB6) file's text: Big5 -- unless the bytes are valid UTF-8 (1008 review: golden 912's cSiteUseManager.cpp is
 * UTF-8, "→" in it, and showed as garbage). A Big5 file is not valid UTF-8 (its second bytes break the sequences), and an
 * all-ASCII one reads the same either way.
 */
function decodeBig5(buf) {
  if (!big5) big5 = new TextDecoder('big5');
  if (!utf8) utf8 = new TextDecoder('utf-8', { fatal: true });
  try { return utf8.decode(buf).replace(/^﻿/, ''); } catch (e) { return big5.decode(buf); }
}

async function walk(root) {
  const out = [];
  const stack = [root];
  while (stack.length) {
    const d = stack.pop();
    let ents;
    try { ents = await fsp.readdir(d, { withFileTypes: true }); } catch (e) { continue; }
    for (const e of ents) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (!SKIP_DIR.test(e.name)) stack.push(p); }
      else if (e.isFile() && EXT.has(path.extname(e.name).toLowerCase())) out.push(p);
    }
  }
  return out;
}

async function pool(items, n, fn) {
  let i = 0;
  const run = async () => { while (i < items.length) { const k = i++; await fn(items[k], k); } };
  await Promise.all(Array.from({ length: Math.min(n, items.length) }, run));
}

class SourceTree {
  /** kind: 'port' (UTF-8, gets #if 0 analysis) or 'golden' (Big5, read-only). */
  constructor(root, kind) {
    this.root = root;
    this.kind = kind;
    this.files = [];
    this.qual = new Map();      // "Cls::member" -> [{ file, off }]
    this.fileKeys = new Map();  // file -> ["Cls::member", ...]
    this.ready = null;
    this.buildMs = 0;
    this.clsFiles = new Map();  // "TfHotPlate" -> Set(files that implement or declare it)
    this.fileCls = new Map();   // file -> ["TfHotPlate", ...]
    this._buf = new Map();      // small LRU of file buffers used by lookups
    this._an = new Map();       // file -> { mtime, an }
    this._str = new Map();      // string literal cache
    this._fn = new Map();       // free-function definition cache
  }

  ensure() {
    if (!this.ready) this.ready = this._build();
    return this.ready;
  }

  async _build() {
    const t0 = Date.now();
    this.files = await walk(this.root);
    await pool(this.files, 16, f => this._scan(f));
    this.buildMs = Date.now() - t0;
  }

  async _scan(file) {
    let buf;
    try { buf = await fsp.readFile(file); } catch (e) { return; }
    const text = buf.toString('latin1');
    const re = /\b(T[A-Za-z_]\w*)\s*::\s*(~?[A-Za-z_]\w*)/g;
    const keys = [];
    const cls = new Set();
    let m;
    while ((m = re.exec(text))) {
      const k = m[1] + '::' + m[2];
      let a = this.qual.get(k);
      if (!a) { a = []; this.qual.set(k, a); }
      a.push({ file, off: m.index, len: m[0].length });
      keys.push(k);
      cls.add(m[1]);
    }
    const cre = /\bclass\s+(?:PACKAGE\s+)?(T[A-Za-z_]\w*)\s*[:{]/g;
    while ((m = cre.exec(text))) cls.add(m[1]);
    for (const c of cls) {
      let s = this.clsFiles.get(c);
      if (!s) { s = new Set(); this.clsFiles.set(c, s); }
      s.add(file);
    }
    this.fileKeys.set(file, keys);
    this.fileCls.set(file, Array.from(cls));
  }

  /** A file changed on disk (from a file watcher). */
  async rescan(file0) {
    if (!this.ready) return;
    await this.ready;
    // (1008 review: the file in the index's own spelling -- the watcher's "d:\…" against a portRoot set as "D:\…" never
    //  matched, the old entries stayed and the file was in twice, one pointing at old lines. And one file's rescans one
    //  after another: a save firing change + create at once added its definitions twice)
    const k = path.resolve(String(file0)).toLowerCase();
    // (1009 review (C++ nav #2): a FOLDER renamed / deleted / made -- one event for the folder: the files indexed under it
    //  are looked at again (gone = out of the index; they were still "the definition" from the cached bytes), and a folder
    //  there now is walked for the files it brings)
    if (!/\.[^\\/.]+$/.test(path.basename(k)) || (() => { try { return fs.statSync(file0).isDirectory(); } catch (e) { return false; } })()) {
      let isDir = false;
      try { isDir = fs.statSync(file0).isDirectory(); } catch (e) { isDir = false; }
      const under = this.files.filter(f => path.resolve(f).toLowerCase().startsWith(k + path.sep));
      if (under.length || isDir) {
        for (const f of under) await this.rescan(f);
        if (isDir) {
          const have = new Set(this.files.map(f => path.resolve(f).toLowerCase()));
          for (const f of await walk(file0)) if (!have.has(path.resolve(f).toLowerCase())) await this.rescan(f);
        }
        return;
      }
    }
    // (1009 second review (C++ nav #9): a file with no C / C++ extension (the folder watcher's "**/*") -- not indexed, and
    //  the caches not cleared for it)
    if (!EXT.has(path.extname(k))) return;
    const file = this.files.find(f => path.resolve(f).toLowerCase() === k) || file0;
    if (!this._rs) this._rs = new Map();
    const run = (this._rs.get(k) || Promise.resolve()).then(() => this._rescan(file), () => this._rescan(file));
    this._rs.set(k, run);
    try { await run; } finally { if (this._rs.get(k) === run) this._rs.delete(k); }
  }

  async _rescan(file) {
    const old = this.fileKeys.get(file);
    if (old) {
      for (const k of new Set(old)) {
        const a = this.qual.get(k);
        if (!a) continue;
        const b = a.filter(o => o.file !== file);
        if (b.length) this.qual.set(k, b); else this.qual.delete(k);
      }
      this.fileKeys.delete(file);
    }
    for (const c of this.fileCls.get(file) || []) {
      const s = this.clsFiles.get(c);
      if (s) { s.delete(file); if (!s.size) this.clsFiles.delete(c); }
    }
    this.fileCls.delete(file);
    this._buf.delete(file);
    this._an.delete(file); this._an.delete(file + '|text');
    if (this._gt) this._gt.delete(file);
    let exists = false;
    try { exists = fs.statSync(file).isFile(); } catch (e) { exists = false; }
    if (exists) {
      if (!this.files.includes(file)) this.files.push(file);
      await this._scan(file);
    } else {
      this.files = this.files.filter(f => f !== file);
    }
    // (1009 second review (C++ nav #1): the cached searches brought up to date for THIS file only -- they were cleared, and
    //  the next click on a control read the whole tree (88 MB) again 15-20 times)
    await this._patchCaches(file, exists);
  }
  async _patchCaches(file, exists) {
    const k = path.resolve(file).toLowerCase();
    const same = f => path.resolve(f).toLowerCase() === k;
    let buf = null;
    if (exists) { try { buf = await fsp.readFile(file); } catch (e) { buf = null; } }
    for (const [str, arr] of Array.from(this._str)) {
      const keep = arr.filter(d => !same(d.file));
      if (buf) {
        const needle = Buffer.from('"' + str + '"', 'latin1');
        let i = buf.indexOf(needle);
        while (i >= 0 && keep.length < 300) { try { keep.push(await this._strHit(file, i, needle)); } catch (e) { /* skip */ } i = buf.indexOf(needle, i + 1); }
      }
      keep.sort(STR_ORDER);
      this._str.set(str, keep);
    }
    for (const [name, arr] of Array.from(this._fn)) {
      const keep = arr.filter(d => !same(d.file));
      if (buf) for (const d of await this._fnHits(file, name, buf)) if (keep.length < 20) keep.push(d);
      keep.sort(FN_ORDER);
      this._fn.set(name, keep);
    }
  }

  async _read(file) {
    if (this._buf.has(file)) {
      const b = this._buf.get(file);
      this._buf.delete(file);
      this._buf.set(file, b);
      return b;
    }
    const b = await fsp.readFile(file);
    this._buf.set(file, b);
    while (this._buf.size > 40) this._buf.delete(this._buf.keys().next().value);
    return b;
  }

  _analysis(file, text) {
    const hit = this._an.get(file);
    // (1008 review: the most recently used 150 kept -- one per file of the tree was ~110 MB, only a rescan let one go)
    if (hit && hit.len === text.length) { this._an.delete(file); this._an.set(file, hit); return hit.an; }
    const an = lex.analyze(text);
    this._an.set(file, { len: text.length, an });
    while (this._an.size > 150) this._an.delete(this._an.keys().next().value);
    return an;
  }

  _decode(buf) {
    return this.kind === 'golden' ? decodeBig5(buf) : buf.toString('utf8');
  }
  /**
   * 1009 review (C++ nav #1): ONE decoder for every slice of a file -- the whole file's encoding. Each slice decided its own
   * (UTF-8 first): in a Big5 file whose first part happens to be valid UTF-8 (899 uhome.cpp: U+FFFD bytes near the top) the
   * prefix was read as UTF-8, its length did not match the whole text's, and 8 of 9 definitions were judged wrong.
   */
  _decoderFor(file, buf) {
    if (this.kind !== 'golden') return b => b.toString('utf8');
    if (!this._enc) this._enc = new Map();
    const k = this._enc.get(file);
    let isU = k && k.len === buf.length ? k.u : null;
    if (isU === null) {
      try { new TextDecoder('utf-8', { fatal: true }).decode(buf); isU = true; } catch (e) { isU = false; }
      this._enc.set(file, { len: buf.length, u: isU });
      while (this._enc.size > 500) this._enc.delete(this._enc.keys().next().value);
    }
    // (TextDecoder drops a leading BOM by itself -- the whole text and every prefix from 0 alike)
    const td = new TextDecoder(isU ? 'utf-8' : 'big5');
    return b => td.decode(b);
  }

  /** Describe the hit at byte offset `off` in `file`. */
  async describe(file, off, len) {
    const buf = await this._read(file);
    const text = buf.toString('latin1');
    let starts, an = null;
    if (this.kind === 'port') { an = this._analysis(file, text); starts = an.starts; }
    else starts = lex.lineStartsOf(text);
    const li = lex.lineIndexOf(starts, off);
    const ls = starts[li];
    const le = li + 1 < starts.length ? starts[li + 1] : buf.length;
    const dec = this._decoderFor(file, buf);
    const lineText = dec(buf.subarray(ls, le)).replace(/\r?\n$/, '').replace(/\r$/, '');
    const col = dec(buf.subarray(ls, off)).length + 1;
    let comment, dead = false, def;
    if (an) {
      comment = lex.inRanges(an.comments, off);
      dead = !!an.dead[li];
      def = !comment && len != null && lex.isDefinitionAt(an.masked, off + len);
    } else {
      // (1008 review: golden -- the decoded text analysed (Big5 second bytes can be "\", so the bytes as latin1 cannot be):
      //  a definition inside /* */ (12 in golden 912: TTrayMotor::HasIC's first one, mymotor.cpp:1053) was taken as THE
      //  definition, the jump went to the commented-out copy. Byte offsets turned into character offsets of that text)
      try {
        // (1009 second review (C++ nav #6): the whole file decoded ONCE (kept while its size is the same), and a hit's
        //  character offset = its line's start + the line's part before it -- every hit decoded the file twice)
        if (!this._gt) this._gt = new Map();
        let gt = this._gt.get(file);
        if (!gt || gt.len !== buf.length) { gt = { len: buf.length, text: dec(buf) }; this._gt.set(file, gt); while (this._gt.size > 20) this._gt.delete(this._gt.keys().next().value); }
        const ga = this._analysis(file + '|text', gt.text);
        const co = (ga.starts && ga.starts[li] !== undefined ? ga.starts[li] + dec(buf.subarray(ls, off)).length : dec(buf.subarray(0, off)).length);
        const cl = len != null ? dec(buf.subarray(off, off + len)).length : 0;
        comment = lex.inRanges(ga.comments, co);
        dead = !!ga.dead[li];
        def = !comment && len != null && lex.isDefinitionAt(ga.masked, co + cl);
      } catch (e) {
        comment = text.slice(ls, off).includes('//');
        def = !comment && len != null && lex.isDefinitionAt(text, off + len);
      }
    }
    let snippet = lineText.trim();
    if (snippet.length > 180) snippet = snippet.slice(0, 180) + '…';
    return {
      file, off, line: li + 1, col, snippet, comment, dead, def,
      test: /(^|[\\/])tests?[\\/]/i.test(path.relative(this.root, file)),
    };
  }

  /**
   * All hits of Cls::member for any of `classes`, best first:
   * live definitions, then #if 0 definitions, comment mentions, other uses.
   */
  async lookup(classes, member) {
    await this.ensure();
    const occ = [];
    for (const c of classes || []) {
      for (const o of this.qual.get(c + '::' + member) || []) occ.push(o);
    }
    const out = [];
    for (const o of occ) {
      try { out.push(await this.describe(o.file, o.off, o.len)); } catch (e) { /* file vanished */ }
    }
    const rank = h => (h.def ? (h.dead ? 1 : 0) : h.comment ? 2 : 3) + (h.test ? 4 : 0);
    out.sort((a, b) => rank(a) - rank(b) || a.file.localeCompare(b.file) || a.line - b.line);
    return out;
  }

  /**
   * Is Cls::member implemented? 'live' (a definition that compiles), 'dead' (only
   * inside #if 0), 'mention' (only named in comments -- often a moved body), 'none'.
   * Stops at the first live definition, so it is cheap enough for a whole form.
   */
  async defState(classes, member) {
    await this.ensure();
    let dead = false, mention = false;
    for (const c of classes || []) {
      for (const o of this.qual.get(c + '::' + member) || []) {
        let h;
        try { h = await this.describe(o.file, o.off, o.len); } catch (e) { continue; }
        if (h.test) continue;
        if (h.def && !h.dead) return { state: 'live', hit: h };
        if (h.def) dead = h;
        else if (h.comment && !mention) mention = h;
      }
    }
    if (dead) return { state: 'dead', hit: dead };
    if (mention) return { state: 'mention', hit: mention };
    return { state: 'none', hit: null };
  }

  /** "cmd" string literal hits; dispatch-looking lines (==, case, strcmp) first. */
  async findString(str) {
    if (this._str.has(str)) return this._str.get(str);
    await this.ensure();
    const needle = Buffer.from('"' + str + '"', 'latin1');
    const raw = [];
    await pool(this.files, 16, async f => {
      let buf;
      try { buf = await fsp.readFile(f); } catch (e) { return; }
      let i = buf.indexOf(needle);
      while (i >= 0 && raw.length < 300) { raw.push({ file: f, off: i }); i = buf.indexOf(needle, i + 1); }
    });
    const out = [];
    for (const r of raw) {
      try { out.push(await this._strHit(r.file, r.off, needle)); } catch (e) { /* skip */ }
    }
    out.sort(STR_ORDER);
    this._str.set(str, out);
    return out;
  }
  /** one hit of findString, described */
  async _strHit(file, off, needle) {
    const r = { file, off };
    {
      {
        const d = await this.describe(r.file, r.off, null);
        // a dispatch compares THIS literal: == "x", case "x", strcmp(c, "x"), or a
        // command table entry { "x", … } -- not a line that merely contains it
        const text = (await this._read(r.file)).toString('latin1');
        const before = text.slice(text.lastIndexOf('\n', r.off - 1) + 1, r.off);
        const le = text.indexOf('\n', r.off);
        const after = text.slice(r.off + needle.length, le < 0 ? text.length : le);
        // (1008 review: "!=" is not where it is handled (an exclusion list); a whitelist line -- if (c=="x") return true; or
        //  a || chain -- compares it but does nothing with it: "weak", after the real dispatch. MainClose.cpp's
        //  CmdAllowedWhileClosing came first by name for pause.run / auth.login / motor.stop)
        d.dispatch = !d.comment && (/==\s*$/.test(before) || /\bcase\s*$/.test(before) ||
          /\b(strcmp|stricmp|strcasecmp|_stricmp|compare|equals|Equals)\s*\([^()]*,\s*$/.test(before) ||
          /\{\s*$/.test(before));
        d.weak = d.dispatch && (/^\s*\)*\s*(return\s+(true|false)\b|\|\||&&)/.test(after) || /\|\|\s*$/.test(before.replace(/[^|]*==\s*$/, '')));
        return d;
      }
    }
  }

  /**
   * Where the form's code touches a component: "spbSave->Enabled = …" and the
   * declaration "TSpeedButton *spbSave;". Only files that implement or declare one
   * of `classes`, so generic names (Label1) do not drag in every other form.
   */
  async findMemberUses(classes, name, limit) {
    await this.ensure();
    const files = new Set();
    for (const c of classes || []) for (const f of this.clsFiles.get(c) || []) files.add(f);
    const needle = Buffer.from(name, 'latin1');
    const e = String(name).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    const re = new RegExp('\\b' + e + '\\s*->|\\*\\s*' + e + '\\s*;', 'g');
    const out = [];
    for (const f of Array.from(files).sort()) {
      let buf;
      try { buf = await this._read(f); } catch (x) { continue; }
      if (buf.indexOf(needle) < 0) continue;
      const text = buf.toString('latin1');
      re.lastIndex = 0;
      let m;
      while ((m = re.exec(text)) && out.length < (limit || 80)) {
        const d = await this.describe(f, m.index, null);
        d.decl = m[0].charAt(0) === '*';
        out.push(d);
      }
    }
    const rank = h => (h.comment ? 4 : 0) + (h.dead ? 2 : 0) + (h.decl ? 1 : 0) + (h.test ? 8 : 0);
    out.sort((a, b) => rank(a) - rank(b) || a.file.localeCompare(b.file) || a.line - b.line);
    return out;
  }

  /** Definitions of a free function (W906_CounterClearClick, DoClearRecordAction, ...). */
  async findFuncDefs(name) {
    if (this._fn.has(name)) return this._fn.get(name);
    await this.ensure();
    const needle = Buffer.from(name, 'latin1');
    const e = String(name).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    const re = new RegExp('(^|[^\\w:.>])' + e + '\\s*\\(', 'g');
    const out = [];
    await pool(this.files, 16, async f => {
      let buf;
      try { buf = await fsp.readFile(f); } catch (x) { return; }
      if (buf.indexOf(needle) < 0) return;
      for (const d of await this._fnHits(f, name, buf)) if (out.length < 20) out.push(d);
    });
    out.sort(FN_ORDER);
    this._fn.set(name, out);
    return out;
  }
  /** the definitions of free function `name` in one file (its bytes given) */
  async _fnHits(f, name, buf) {
    const needle = Buffer.from(name, 'latin1');
    if (buf.indexOf(needle) < 0) return [];
    const e = String(name).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    const re = new RegExp('(^|[^\\w:.>])' + e + '\\s*\\(', 'g');
    const text = buf.toString('latin1');
    const out = [];
    let m;
    while ((m = re.exec(text)) && out.length < 20) {
      const off = m.index + m[1].length;
      try {
        const d = await this.describe(f, off, name.length);
        if (d.def) out.push(d);
      } catch (x) { /* skip */ }
    }
    return out;
  }

  /**
   * The function a dispatch branch hands the command to. `off` = byte offset of the
   * command string literal ("io.btnPanelClick"). Only the rest of that branch on the
   * same line counts, up to the next branch when several share one line:
   *   if (cmd == "x") return DoX(p);                  -> ["DoX"]
   *   else if (wc.cmd == "x") r = W906_X(tag, &ok);   -> ["W906_X"]
   *   } else if (wc.cmd == "x") {   (code follows)    -> []  (the block IS the handler;
   *                                                         its helpers are not)
   * Member calls (v.isString(), p->Foo()) are never a handler.
   */
  async calledFuncsAt(file, off) {
    const buf = await this._read(file);
    const text = buf.toString('latin1');
    const src = this.kind === 'port' ? this._analysis(file, text).masked : text;
    let e = src.indexOf('\n', off);
    if (e < 0) e = src.length;
    const line = src.slice(off, e);
    const q = line.indexOf('"', 1);               // past the literal itself
    const rest = q > 0 ? line.slice(q + 1) : line;
    // the next branch on the same line -- (1008 review) also "} else", "case", "break;", ".compare(": act.home.abort at
    // wb_serve.cpp:671 listed the else arm's W906_MsgBoxModelessAnswer / W906_SimDiCommand as its handlers
    const next = rest.search(/==\s*"|\}\s*else\b|\belse\b|\bcase\b|\bbreak\s*;|\.compare\s*\(/);
    const region = next >= 0 ? rest.slice(0, next) : rest;
    const out = [];
    const re = /(^|[^\w.>])([A-Za-z_]\w*)\s*\(/g;
    let m;
    while ((m = re.exec(region)) && out.length < 3) {
      const n = m[2];
      if (m[1] === ':' && region[m.index - 1] !== ':') continue;   // a::b ok, a:b no
      if (CPP_SKIP.has(n) || out.includes(n) || n.length < 4) continue;
      if (!/^W906_|[A-Z]/.test(n)) continue;   // project functions are CamelCase / W906_*
      out.push(n);
    }
    return out;
  }
}

const CPP_SKIP = new Set(('if while for switch return sizeof static_cast reinterpret_cast const_cast ' +
  'dynamic_cast catch throw new delete std string wstring to_string printf sprintf snprintf fprintf strcmp ' +
  'strncmp stricmp memcpy memset strlen atoi atof strtol strtod min max abs size empty c_str find substr ' +
  'append push_back emplace_back begin end insert erase at front back reset get make_shared make_unique move ' +
  'forward lock unlock AnsiString UnicodeString StrToInt IntToStr FloatToStr StrToFloat Format ShowMessage ' +
  'assert Key String Int Uint Int64 Bool Double Null StartObject EndObject StartArray EndArray Writer ' +
  'GetString GetInt GetBool HasMember IsString IsInt IsBool IsObject IsArray FindMember Parse ' +
  'ToString TRUE FALSE NULL DWORD BOOL WORD BYTE LOBYTE HIBYTE').split(/\s+/));

module.exports = { SourceTree, walk, decodeBig5, SKIP_DIR };
