'use strict';
// AI(W906-HTDESIGNER) 20260929: C++ -> web. One pass over web\**\*.js and the pages'
// inline <script>s (~6 MB, about a second) collects:
//   commands  "io.btnPanelClick"          -> where the web sends it
//   fields    [Hotplate Form] "X Start"   -> the control that edits it (XST1: [...] rows)
// and, per script, the pages that load it (and which controls each page has).
// Plain Node (no vscode).

const fs = require('fs');
const path = require('path');
const web = require('./websearch');
const { listScripts, collectIds } = require('./pageinfo');

// JSON\ is data (and JSON\js the same data as <script> shims), not code
const SKIP = /^(img|shot|ScreenShot|_partials|node_modules|\.git|JSON)$/i;

class WebCmdIndex {
  constructor(webRoot) {
    this.root = webRoot;
    this.sends = new Map();    // cmd -> [{ file, line, col, snippet }]
    this.fields = new Map();   // "section\0key" -> [{ id, section, key, note, file, line, col, snippet }]
    this.fieldsById = new Map(); // id -> [row]
    this.tags = new Map();     // "machine.state" -> [{ tag, id, prop, file, line, col, snippet }]
    this.tagsById = new Map(); // id -> [row]
    this.pagesOf = new Map();  // script file (lower) -> [page files]
    this.pageIds = new Map();  // page file (lower) -> Set(ids)
    this.built = false;
    this.buildMs = 0;
    this.files = 0;
  }

  build() {
    if (this.built) return this;
    const t0 = Date.now();
    const files = [];
    const walk = d => {
      let ents = [];
      try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; }
      for (const e of ents) {
        const p = path.join(d, e.name);
        if (e.isDirectory()) { if (!SKIP.test(e.name)) walk(p); }
        else if (/\.(js|html?)$/i.test(e.name)) files.push(p);
      }
    };
    if (this.root) walk(this.root);
    for (const f of files) {
      const text = web.readText(f);
      if (text == null) continue;
      const isHtml = /\.html?$/i.test(f);
      const src = new web.Source(f, text, isHtml);
      for (const s of web.sendsIn(src)) {
        if (!this.sends.has(s.cmd)) this.sends.set(s.cmd, []);
        this.sends.get(s.cmd).push(s.hit);
      }
      for (const r of web.fieldRowsIn(src)) {
        const row = Object.assign({ id: r.id, doc: r.doc, section: r.section, key: r.key, note: r.note }, r.hit);
        const k = r.section + '\u0000' + r.key;
        if (!this.fields.has(k)) this.fields.set(k, []);
        this.fields.get(k).push(row);
        if (!this.fieldsById.has(r.id)) this.fieldsById.set(r.id, []);
        this.fieldsById.get(r.id).push(row);
      }
      for (const r of web.tagRowsIn(src)) {
        const row = Object.assign({ tag: r.tag, id: r.id, prop: r.prop }, r.hit);
        if (!this.tags.has(r.tag)) this.tags.set(r.tag, []);
        this.tags.get(r.tag).push(row);
        if (!this.tagsById.has(r.id)) this.tagsById.set(r.id, []);
        this.tagsById.get(r.id).push(row);
      }
      if (isHtml) {
        this.pageIds.set(f.toLowerCase(), new Set(collectIds(text)));
        for (const sc of listScripts(text, path.dirname(f))) {
          const k = sc.toLowerCase();
          if (!this.pagesOf.has(k)) this.pagesOf.set(k, []);
          this.pagesOf.get(k).push(f);
        }
      }
    }
    this.files = files.length;
    this.built = true;
    this.buildMs = Date.now() - t0;
    return this;
  }

  pagesRunning(file) {
    return /\.html?$/i.test(file) ? [file] : (this.pagesOf.get(file.toLowerCase()) || []);
  }

  /** Where the web sends `cmd`, each with the pages that run it. */
  senders(cmd) {
    this.build();
    return (this.sends.get(cmd) || []).map(h => Object.assign({ pages: this.pagesRunning(h.file) }, h));
  }

  /** The controls that edit [section] key, each with the pages that really show it. */
  fieldControls(section, key) {
    this.build();
    return (this.fields.get(section + '\u0000' + key) || []).map(r => Object.assign({
      pages: this.pagesRunning(r.file).filter(p => { const ids = this.pageIds.get(p.toLowerCase()); return !ids || ids.has(r.id); }),
    }, r));
  }

  /** The controls that show tag `tag`, each with the pages that really have it. */
  tagControls(tag) {
    this.build();
    return (this.tags.get(tag) || []).map(r => Object.assign({
      pages: this.pagesRunning(r.file).filter(p => { const ids = this.pageIds.get(p.toLowerCase()); return !ids || ids.has(r.id); }),
    }, r));
  }

  /** The tag rows for control `id` in scripts that `page` runs. */
  tagsOf(page, id) {
    this.build();
    const pk = page.toLowerCase();
    return (this.tagsById.get(id) || []).filter(r => this.pagesRunning(r.file).some(p => p.toLowerCase() === pk));
  }

  /** The field rows for control `id` in scripts that `page` runs. */
  fieldsOf(page, id) {
    this.build();
    const pk = page.toLowerCase();
    return (this.fieldsById.get(id) || []).filter(r => this.pagesRunning(r.file).some(p => p.toLowerCase() === pk));
  }
}

module.exports = { WebCmdIndex };
