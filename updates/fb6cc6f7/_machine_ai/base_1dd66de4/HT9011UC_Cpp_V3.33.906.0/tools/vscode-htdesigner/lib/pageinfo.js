'use strict';
// AI(W906-HTDESIGNER) 20260929: which DFM a page came from, and that DFM's IR
// (tools\dfm2rc\ir_out\*.dfm.ir.json: every control's class, properties, events).
// Plain Node (no vscode).

const fs = require('fs');
const path = require('path');

/**
 * The generated pages put the source form in <title>:
 *   "Hot Plate（cHotPlate.dfm / fHotPlate : TfHotPlate）"
 *   "HT-9132 主視窗（main.dfm / TMainForm）"   "AOA Info（main.dfm ts1）"
 */
function parseTitle(html) {
  const m = /<title[^>]*>([^<]*)<\/title>/i.exec(html || '');
  if (!m) return null;
  const title = m[1].trim();
  const d = /([A-Za-z_][\w]*)\.dfm\b/i.exec(title);
  const c = /:\s*(T\w+)/.exec(title) || /\/\s*(T\w+)\s*[）)]/.exec(title);
  return { title, dfm: d ? d[1] : null, cls: c ? c[1] : null };
}

/** Every static id="..." in the page, in order. */
function collectIds(html) {
  const out = [];
  const re = /\bid\s*=\s*"([A-Za-z_][\w]*)"/g;
  let m;
  while ((m = re.exec(html || ''))) out.push(m[1]);
  return out;
}

/** <script src> of the page that exist on disk, resolved against the page directory. */
function listScripts(html, pageDir) {
  const out = [];
  const seen = new Set();
  const re = /<script\b[^>]*\bsrc\s*=\s*["']([^"']+)["']/gi;
  let m;
  while ((m = re.exec(html || ''))) {
    const src = m[1].split('?')[0].split('#')[0];
    if (/^[a-z][\w+.-]*:/i.test(src) || src.startsWith('//')) continue;
    const p = path.resolve(pageDir, src.replace(/\//g, path.sep));
    const k = p.toLowerCase();
    if (seen.has(k)) continue;
    seen.add(k);
    try { if (fs.statSync(p).isFile()) out.push(p); } catch (e) { /* missing: skip */ }
  }
  return out;
}

// Names every form has; they say nothing about which DFM a page came from.
const GENERIC_ID = /^(Label|Panel|GroupBox|Button|BitBtn|SpeedButton|Image|Edit|Shape|Bevel|CheckBox|ComboBox|RadioButton|RadioGroup|TabSheet|PageControl|Memo|Timer|StaticText|StringGrid|ListBox|Splitter|ScrollBox)\d+$/;

class IrStore {
  constructor(root) {
    this.root = root;
    this._files = null;
    this._cache = new Map();
    this._names = null;
  }

  files() {
    if (this._files) return this._files;
    const out = [];
    const walk = d => {
      let ents = [];
      try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; }
      for (const e of ents) {
        const p = path.join(d, e.name);
        if (e.isDirectory()) walk(p);
        else if (/\.dfm\.ir\.json$/i.test(e.name)) out.push(p);
      }
    };
    if (this.root) walk(this.root);
    this._files = out;
    return out;
  }

  /** IR file for "cHotPlate" (+ optional class to break ties). */
  find(dfmBase, cls) {
    if (!dfmBase) return null;
    const want = dfmBase.toLowerCase() + '.dfm.ir.json';
    const c = this.files().filter(f => path.basename(f).toLowerCase() === want);
    if (c.length <= 1) return c[0] || null;
    if (cls) for (const f of c) { const d = this.load(f); if (d && d.formClass === cls) return f; }
    return c[0];
  }

  load(file) {
    let st;
    try { st = fs.statSync(file); } catch (e) { return null; }
    const hit = this._cache.get(file);
    if (hit && hit.mtime === st.mtimeMs) return hit.data;
    let j;
    try { j = JSON.parse(fs.readFileSync(file, 'utf8')); } catch (e) { return null; }
    const byName = new Map();
    let root = null;
    for (const n of j.nodes || []) {
      if (!root && (n.kind === 'ROOT' || n.depth === 0)) root = n;
      if (n.name && !byName.has(n.name)) byName.set(n.name, n);
    }
    const data = {
      file,
      formClass: j.form_class || (root && root.class) || null,
      formName: j.form_name || (root && root.name) || null,
      sourceDfm: j.source_dfm || null,
      root,
      byName,
      count: (j.nodes || []).length,
    };
    this._cache.set(file, { mtime: st.mtimeMs, data });
    return data;
  }

  /** For pages whose title names no DFM: the IR whose control names cover most page ids. */
  guess(ids) {
    const want = Array.from(new Set(ids || [])).filter(id => !GENERIC_ID.test(id));
    if (want.length < 3) return null;
    if (!this._names) {
      this._names = new Map();
      for (const f of this.files()) {
        let t = '';
        try { t = fs.readFileSync(f, 'utf8'); } catch (e) { continue; }
        const s = new Set();
        const re = /"name":\s*"((?:[^"\\]|\\.)*)"/g;
        let m;
        while ((m = re.exec(t))) s.add(m[1]);
        this._names.set(f, s);
      }
    }
    let best = null;
    for (const [f, s] of this._names) {
      let hits = 0;
      for (const id of want) if (s.has(id)) hits++;
      if (!best || hits > best.hits) best = { file: f, hits, score: hits / want.length };
    }
    if (!best || best.hits < 5 || best.score < 0.3) return null;
    return best;
  }
}

/**
 * Line of every property of one DFM object. `lines` = the .dfm split into lines,
 * `nodeLine` = the IR's 1-based line of "object Name: TClass". Nested objects and
 * collection items (item … end) are skipped so their properties are not taken.
 * Returns { "Caption": 355, "Font.Name": 360, ... } (1-based).
 */
function dfmPropLines(lines, nodeLine) {
  const out = {};
  let depth = 0;
  for (let i = nodeLine; i < lines.length; i++) {
    const t = lines[i].trim();
    if (/^(object|inherited|inline|item)\b/i.test(t)) { depth++; continue; }
    if (/^end\b/i.test(t) || /^end>\s*$/i.test(t)) { if (depth === 0) break; depth--; continue; }
    if (/^item\s*$/i.test(t)) { depth++; continue; }
    if (depth === 0) {
      const m = /^([A-Za-z_][\w.]*)\s*=/.exec(t);
      if (m && !Object.prototype.hasOwnProperty.call(out, m[1])) out[m[1]] = i + 1;
    }
  }
  return out;
}

/**
 * A page that only forwards to another one (an empty <body> plus location.replace /
 * location.href = / meta refresh): the target file name, else null.
 *   Setup.BinSelNormal.html: location.replace('Setup.BinSel.html' + location.search)
 */
function redirectTarget(html) {
  const text = String(html || '');
  // AI(W906-HTDESIGNER) 20261002 (machine): a <head> script whose FIRST statement sends the page on forwards it, body or
  // not -- the browser leaves before the body is drawn (Setup.Configuration.html -> Config.Configuration.html,
  // Setup.DIOInterFaceCFG.html -> Config.DIOInterFaceCFG.html keep their old body; the page list, which reads only a
  // page's first 8 KB, already called them forwarding pages and this said no)
  const bodyAt = text.search(/<body[\s>]/i);
  const headPart = bodyAt >= 0 ? text.slice(0, bodyAt) : text;
  const hs = /<script[^>]*>([\s\S]*?)<\/script>/gi;
  let hm;
  while ((hm = hs.exec(headPart))) {
    const first = /^\s*(?:\/\*[\s\S]*?\*\/\s*|\/\/[^\n]*\n\s*)*location\.replace\(\s*['"]([^'"?#]+\.html?)/i.exec(hm[1]);
    if (first) return first[1];
  }
  const body = /<body[^>]*>([\s\S]*?)<\/body>/i.exec(text);
  const rest = body ? body[1].replace(/<script[\s\S]*?<\/script>/gi, '').replace(/<!--[\s\S]*?-->/g, '').trim() : '';
  if (rest) return null;
  const m = /location\.(?:replace|assign)\(\s*['"]([^'"?#]+\.html?)/i.exec(text) ||
    /location(?:\.href)?\s*=\s*['"]([^'"?#]+\.html?)/i.exec(text) ||
    /<meta[^>]+http-equiv\s*=\s*["']?refresh["']?[^>]*url\s*=\s*([^"'>\s;]+\.html?)/i.exec(text);
  return m ? m[1] : null;
}

/**
 * 1009 second review (DFM #3): the part of the form a page is -- "AOA Info（main.dfm ts1）", "Logs（main.dfm tsLogs＋tsMNetLog）",
 * "Control Buttons（main.dfm / gbControlBtn）" -> the IR paths of those components; null = the whole form
 * ("main.dfm / TMainForm" names the class, no component).
 */
function scopeOf(title, ir) {
  if (!title || !ir || !ir.byName) return null;
  const m = /\.dfm\b([^）)]*)/i.exec(String(title));
  if (!m) return null;
  const out = [];
  for (const n of m[1].match(/[A-Za-z_]\w*/g) || []) {
    const nd = ir.byName.get(n);
    if (nd && nd !== ir.root && nd.path && !out.some(x => x.name === n)) out.push({ name: n, path: nd.path });
  }
  return out.length ? out : null;
}

module.exports = { parseTitle, collectIds, listScripts, IrStore, GENERIC_ID, dfmPropLines, redirectTarget, scopeOf };
