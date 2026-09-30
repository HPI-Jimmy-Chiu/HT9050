'use strict';
// AI(W906-HTDESIGNER) 20260929: code -> designer. Which page and which control does a
// C++ / BCB6 handler (TfHotPlate::spbSaveClick) belong to?
//
//   page\*.html <title> names its .dfm  ->  that DFM's IR  ->  every node's events
//   => "TfHotPlate::spbSaveClick" -> [{ page, node: "spbSave", event: "OnClick" }]
//
// Built once, lazily (the ~40 IR files that have a page, ~20 MB of JSON, well under
// a second). Plain Node (no vscode).

const fs = require('fs');
const path = require('path');
const { parseTitle, collectIds } = require('./pageinfo');

function readHead(file, n) {
  let fd = null;
  try {
    fd = fs.openSync(file, 'r');
    const buf = Buffer.alloc(n);
    const len = fs.readSync(fd, buf, 0, n, 0);
    return buf.subarray(0, len).toString('utf8');
  } catch (e) {
    return '';
  } finally {
    if (fd !== null) try { fs.closeSync(fd); } catch (e) { /* ignore */ }
  }
}

class ReverseIndex {
  /** irStore: pageinfo.IrStore; pageDirs: folders holding the .html pages */
  constructor(irStore, pageDirs) {
    this.ir = irStore;
    this.pageDirs = pageDirs || [];
    this.handlers = new Map();  // "TfHotPlate::spbSaveClick" -> [entry]
    this.controls = new Map();  // "TfHotPlate#spbSave"        -> [entry]
    this.classes = new Set();   // form classes that have a page
    this.pages = [];            // [{ page, irFile, formClass }]
    this.built = false;
    this.buildMs = 0;
  }

  build() {
    if (this.built) return this;
    const t0 = Date.now();
    const byIr = new Map();
    for (const dir of this.pageDirs) {
      let names = [];
      try { names = fs.readdirSync(dir).filter(n => /\.html?$/i.test(n)); } catch (e) { names = []; }
      for (const n of names.sort()) {
        const page = path.join(dir, n);
        const t = parseTitle(readHead(page, 8192));
        if (!t || !t.dfm) continue;
        const irFile = this.ir.find(t.dfm, t.cls);
        if (!irFile) continue;
        const k = irFile.toLowerCase();
        if (!byIr.has(k)) byIr.set(k, { irFile, pages: [] });
        // several pages can share one DFM (main.dfm: main.html, Main.AOAInfo.html, …),
        // each showing only part of it -- remember which controls each page really has
        let ids;
        try { ids = new Set(collectIds(fs.readFileSync(page, 'utf8'))); } catch (e) { ids = new Set(); }
        byIr.get(k).pages.push({ page, ids });
      }
    }
    for (const { irFile, pages } of byIr.values()) {
      const d = this.ir.load(irFile);
      if (!d || !d.formClass) continue;
      const cls = d.formClass;
      this.classes.add(cls);
      for (const p of pages) this.pages.push({ page: p.page, irFile, formClass: cls });
      for (const node of d.byName.values()) {
        const isRoot = node === d.root;
        const name = isRoot ? '@form' : node.name;
        // the form itself: only its "own" pages (the one whose ids cover most of it)
        const where = isRoot ? pages.slice().sort((a, b) => b.ids.size - a.ids.size).slice(0, 1) : pages.filter(p => p.ids.has(node.name));
        for (const { page } of where) {
          const base = { page, irFile, formClass: cls, node: name, nodeClass: node.class, path: node.path, dfmLine: node.line || 0 };
          const ck = cls + '#' + node.name;
          if (!this.controls.has(ck)) this.controls.set(ck, []);
          this.controls.get(ck).push(base);
          for (const [ev, h] of Object.entries(node.events || {})) {
            const hk = cls + '::' + h;
            if (!this.handlers.has(hk)) this.handlers.set(hk, []);
            this.handlers.get(hk).push(Object.assign({ event: ev, handler: h }, base));
          }
        }
      }
    }
    this.built = true;
    this.buildMs = Date.now() - t0;
    return this;
  }

  handler(cls, method) { return this.build().handlers.get(cls + '::' + method) || []; }
  control(cls, name) { return this.build().controls.get(cls + '#' + name) || []; }
  hasClass(cls) { return this.build().classes.has(cls); }
}

/**
 * The C++ function definition the offset sits in: "TfHotPlate::spbSaveClick".
 * Looks back for the nearest "TClass::name(" whose "(…)" is followed by "{".
 */
function enclosingMethod(text, off, isDefinitionAt) {
  const { braceEnd } = require('./websearch');
  const base = Math.max(0, off - 200000);
  const head = text.slice(base, Math.min(text.length, off + 400));
  const re = /\b(T[A-Za-z_]\w*)\s*::\s*(~?[A-Za-z_]\w*)\s*\(/g;
  let m, best = null;
  while ((m = re.exec(head))) {
    const at = base + m.index;
    if (at > off + 200) break;
    const lineStart = text.lastIndexOf('\n', at) + 1;
    if (text.slice(lineStart, at).includes('//')) continue;
    if (!isDefinitionAt(text, at + m[0].length - 1)) continue;
    // the cursor must be inside this definition (signature line or body)
    const open = text.indexOf('{', at);
    if (open < 0) continue;
    const end = braceEnd(text, open);
    if (off <= end) best = { cls: m[1], method: m[2], off: at };
  }
  return best;
}

module.exports = { ReverseIndex, enclosingMethod };
