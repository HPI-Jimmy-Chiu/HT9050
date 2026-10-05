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
// AI(W906-HTDESIGNER) 20261002 (machine): .claude too -- agents' git worktrees live in <tree>\.claude\worktrees\ (whole
// copies of the tree: 2,376 more .cpp / .h on 20261001 22:25), every function found twice and the index 10x slower
const SKIP_DIR = /^(build.*|third_party|\.git|\.svn|\.vs|\.vscode|\.claude|node_modules|__pycache__|_archive.*|_backup.*|ir_out|scratchpad|ship|dist|web-overlay|vscode-htdesigner)$/i;

let big5 = null;
function decodeBig5(buf) {
  if (!big5) big5 = new TextDecoder('big5');
  return big5.decode(buf);
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
  async rescan(file) {
    if (!this.ready) return;
    await this.ready;
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
    this._an.delete(file);
    this._str.clear();
    this._fn.clear();
    let exists = false;
    try { exists = fs.statSync(file).isFile(); } catch (e) { exists = false; }
    if (exists) {
      if (!this.files.includes(file)) this.files.push(file);
      await this._scan(file);
    } else {
      this.files = this.files.filter(f => f !== file);
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
    if (hit && hit.len === text.length) return hit.an;
    const an = lex.analyze(text);
    this._an.set(file, { len: text.length, an });
    return an;
  }

  _decode(buf) {
    return this.kind === 'golden' ? decodeBig5(buf) : buf.toString('utf8');
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
    const lineText = this._decode(buf.subarray(ls, le)).replace(/\r?\n$/, '').replace(/\r$/, '');
    const col = this._decode(buf.subarray(ls, off)).length + 1;
    let comment, dead = false;
    if (an) {
      comment = lex.inRanges(an.comments, off);
      dead = !!an.dead[li];
    } else {
      comment = text.slice(ls, off).includes('//');
    }
    const def = !comment && len != null && lex.isDefinitionAt(an ? an.masked : text, off + len);
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
      try {
        const d = await this.describe(r.file, r.off, null);
        // a dispatch compares THIS literal: == "x", case "x", strcmp(c, "x"), or a
        // command table entry { "x", … } -- not a line that merely contains it
        const text = (await this._read(r.file)).toString('latin1');
        const before = text.slice(text.lastIndexOf('\n', r.off - 1) + 1, r.off);
        d.dispatch = !d.comment && (/(==|!=)\s*$/.test(before) || /\bcase\s*$/.test(before) ||
          /\b(strcmp|stricmp|strcasecmp|_stricmp|compare|equals|Equals)\s*\([^()]*,\s*$/.test(before) ||
          /\{\s*$/.test(before));
        out.push(d);
      } catch (e) { /* skip */ }
    }
    const rank = h => (h.dispatch ? 0 : h.comment ? 2 : 1) + (h.dead ? 3 : 0) + (h.test ? 6 : 0);
    out.sort((a, b) => rank(a) - rank(b) || a.file.localeCompare(b.file) || a.line - b.line);
    this._str.set(str, out);
    return out;
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
      const text = buf.toString('latin1');
      re.lastIndex = 0;
      let m;
      while ((m = re.exec(text)) && out.length < 20) {
        const off = m.index + m[1].length;
        try {
          const d = await this.describe(f, off, name.length);
          if (d.def) out.push(d);
        } catch (x) { /* skip */ }
      }
    });
    out.sort((a, b) => (a.dead - b.dead) || (a.test - b.test) || a.file.localeCompare(b.file) || a.line - b.line);
    this._fn.set(name, out);
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
    const next = rest.search(/==\s*"/);           // the next branch on the same line
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
