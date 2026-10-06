'use strict';
// AI(W906-HTDESIGNER) 20261006 (ES02, EastSun: "我需要你檢查每一個外掛 我不要用抽樣的 要用枚舉 我刪元件要可以正常編譯 加元件要可以
// 正常編譯 裡面屬性設定也都要有功能 事件也要可以刪除與新增 並且功能正常 可以編譯"): e2e_build.js did three pages; this one
// ENUMERATES -- every page of web\page, every component on it, every event of each, every toolbox kind -- with the
// designer's own code (extension.js on test\fake_vscode.js), writing UNDER THE TREE GIVEN (a git worktree; e2e_all.ps1
// builds 9050 after each wave and puts the tree back).
//   argv: <wave> <reportJson>      wave = inventory | add | evdel | compdel
//   env:  HTD_E2E_PORT = the C++ port tree (default: this extension's tree)
//   add     : every toolbox kind onto every page (into its form / body), then every event of every new component
//   evdel   : every event already wired on every component of every page reset WITH its C++ function (the delete answer)
//   compdel : every component of every page deleted (the parents last: a child is gone with its parent anyway)
const Module = require('module');
const fs = require('fs');
const path = require('path');
const fake = require('./fake_vscode');

const origResolve = Module._resolveFilename;
Module._resolveFilename = function (req, parent, ...rest) {
  if (req === 'vscode') return require.resolve('./fake_vscode');
  return origResolve.call(this, req, parent, ...rest);
};

const report = { wave: '', pages: [], added: [], events: [], deleted: [], refused: [], written: [], errors: [] };

function mkPanel() {
  const p = {
    posted: [], recv: null, active: true, visible: true, viewColumn: 1, reveal() {},
    onDidChangeViewState: () => ({ dispose() {} }), onDidDispose: () => ({ dispose() {} }), dispose() {},
    webview: {
      cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
      asWebviewUri: u => ({ toString: () => 'https://x/' + u.fsPath.replace(/\\/g, '/') }),
      postMessage: m => { p.posted.push(m); return Promise.resolve(true); },
      onDidReceiveMessage: fn => { p.recv = fn; return { dispose() {} }; },
    },
  };
  return p;
}
function mkDoc(file) {
  let text = fs.readFileSync(file, 'utf8');
  let ls = [];
  const index = () => { ls = [0]; for (let i = 0; i < text.length; i++) if (text.charCodeAt(i) === 10) ls.push(i + 1); };
  index();
  const doc = {
    uri: fake.Uri.file(file), fileName: file, isDirty: false,
    getText: () => text,
    positionAt: off => { let lo = 0, hi = ls.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (ls[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - ls[lo]); },
    offsetAt: p => ls[p.line] + p.character,
    _setText: t => { text = t; index(); doc.isDirty = true; },
  };
  return doc;
}
const sleep = ms => new Promise(r => setTimeout(r, ms));

/** the components of a page from its source: every element with an id and a "name : TClass" title (the generator's) */
function componentsOf(html) {
  const out = [];
  const re = /<([a-zA-Z][\w-]*)\b[^>]*\bid\s*=\s*["']([^"']+)["'][^>]*>/g;
  let m;
  while ((m = re.exec(html))) {
    const tag = m[0];
    const t = /\btitle\s*=\s*["']([^"']*)["']/.exec(tag);
    const cm = t && /^\s*([\w$]+)\s*:\s*(T\w+)/.exec(t[1]);
    out.push({ id: m[2], tag: m[1].toLowerCase(), cls: cm ? cm[2] : '', at: m.index });
  }
  return out;
}

async function main() {
  const [wave, repFile] = process.argv.slice(2);
  report.wave = wave;
  const PORT = process.env.HTD_E2E_PORT ? path.resolve(process.env.HTD_E2E_PORT) : path.resolve(__dirname, '..', '..', '..');
  const WEB = path.join(PORT, '..', 'web');
  if (/^[a-z]:\\ht9045\\/i.test(PORT + '\\') && !/_work|htd_work|e2e/i.test(PORT)) throw new Error('refusing to write under ' + PORT);
  report.port = PORT;
  fake._folders = [{ uri: fake.Uri.file(PORT) }];
  Object.assign(fake._config, { eventJump: 'cpp', syncSource: false });
  const ext = require('../extension');
  const memento = { _m: {}, get(k, d) { return k in this._m ? this._m[k] : d; }, update(k, v) { this._m[k] = v; return Promise.resolve(); } };
  const ctx = { subscriptions: [], extensionUri: fake.Uri.file(path.resolve(__dirname, '..')), workspaceState: memento, globalState: memento,
    extension: { id: 'ht9045.ht9045-html-designer', extensionPath: path.resolve(__dirname, '..'), packageJSON: JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')) } };
  const api = ext.activate(ctx);
  const hub = api.hub;
  fake._props.resolveWebviewView({
    webview: { cspSource: 'x', options: null, html: '', asWebviewUri: u => ({ toString: () => 'x' }), postMessage: () => Promise.resolve(true), onDidReceiveMessage: () => ({ dispose() {} }) },
    onDidDispose: () => ({ dispose() {} }),
  });
  fake._docs = fake._docs || {};
  // every modal question answered with its first button (delete = "delete"; a used component = delete anyway)
  if (fake.window && fake.window.showWarningMessage) fake.window.showWarningMessage = async (msg, opts, ...items) => (opts && opts.modal && items.length ? items[0] : undefined);
  if (fake.window && fake.window.showInformationMessage) fake.window.showInformationMessage = async (msg, opts, ...items) => (opts && opts.modal && items.length ? items[0] : undefined);

  const pageDir = path.join(WEB, 'page');
  const only = process.env.HTD_E2E_PAGES ? process.env.HTD_E2E_PAGES.split(',') : null;
  const pages = fs.readdirSync(pageDir).filter(f => /\.html$/i.test(f) && (!only || only.includes(f))).sort();
  const toolbox = require('../lib/toolbox');
  const he = require('../lib/htmledit'), hb = require('../lib/htmlblock');
  let key = 1000;
  const covered = new Set();
  const T0 = Date.now();

  for (const page of pages) {
    const file = path.join(pageDir, page);
    const doc = mkDoc(file);
    fake._docs[doc.uri.toString()] = doc;
    const panel = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, panel);
    panel.recv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });
    for (let i = 0; i < 100 && !panel.posted.some(m => m.type === 'init'); i++) await sleep(50);
    const d = hub.active;
    const P = { page, form: '', global: null, comps: 0, classes: {}, events: 0, wired: 0, note: '' };
    report.pages.push(P);
    if (!d || d.doc !== doc) { P.note = 'designer not active'; continue; }
    let pi;
    try { pi = await d.info; } catch (e) { P.note = 'info: ' + e.message; continue; }
    if (pi && pi.redirect) { P.note = 'redirect -> ' + pi.redirect; continue; }
    P.form = (pi.classes && pi.classes[0]) || '';
    const select = async (id) => {
      const info = { key: key++, id, tag: 'div', cls: '', title: '', attrs: [['id', id]], style: [], inline: [], computed: [], chain: ['@form'], cssPath: '', text: id,
        visible: true, geom: { left: 0, top: 0, width: 10, height: 10, rel: 'form' }, listeners: [], inherited: [] };
      d.sel = { info, data: null, promise: null };
      const data = await hub.resolve(d, info, () => {});
      d.sel.data = data; d.sel.promise = Promise.resolve(data);
      return data;
    };
    const comps = componentsOf(doc.getText()).filter(c => c.cls);
    P.comps = comps.length;
    for (const c of comps) P.classes[c.cls] = (P.classes[c.cls] || 0) + 1;

    if (wave === 'inventory') {
      for (const c of comps) {
        let data = null;
        try { data = await select(c.id); } catch (e) { report.errors.push(page + ' ' + c.id + ' resolve: ' + (e && e.message)); continue; }
        const g = data && data.eventGrid;
        if (!g) continue;
        P.events += g.rows.length;
        P.wired += g.rows.filter(r => r.handler).length;
        if (P.global === null) P.global = g.global !== false;
      }
      continue;
    }

    if (wave === 'add') {
      // every toolbox kind, into the form (a page without a form root: its body), stacked down the right
      let y = 8;
      const newIds = [];
      for (const it of toolbox.ITEMS) {
        const c0 = (fake._calls || []).length;
        let r = null;
        try { r = await hub.cmdAdd(it.cls, { into: '@form', x: 8, y }); } catch (e) { report.errors.push(page + ' add ' + it.cls + ': ' + (e && e.stack || e)); }
        if (r && r.id) { newIds.push(r); report.added.push({ page, id: r.id, cls: r.cls }); }
        else report.refused.push({ page, what: 'add ' + it.cls, why: (fake._calls || []).slice(c0).map(x => x.name + ': ' + String(x.args && x.args[0]).slice(0, 160)).join(' | ') });
        y += 40;
      }
      // the events, enumerated by what they exercise (every class x event once -- the signature and the wiring of
      // each -- and on every page with a form at least one: its own .h / .cpp / page compile with a new handler);
      // every class x event on every page would be ~24,000 copies of the same thing
      let pageEvents = 0;
      for (const n of newIds) {
        let data = null;
        try { data = await select(n.id); } catch (e) { report.errors.push(page + ' ' + n.id + ' resolve: ' + (e && e.message)); continue; }
        const g = data && data.eventGrid;
        if (!g) { report.refused.push({ page, what: n.id + ': no event grid' }); continue; }
        for (const row of g.rows.slice()) {
          if (row.handler) continue;
          const ck = n.cls + '.' + row.name;
          if (covered.has(ck) && pageEvents > 0) continue;
          let r = null;
          try { r = await hub.onEventGrid(data, row.name, d); } catch (e) { report.errors.push(page + ' ' + n.id + '.' + row.name + ': ' + (e && e.stack || e)); continue; }
          if (r && r.name) { report.events.push({ page, form: g.formCls, control: n.id, cls: n.cls, event: row.name, handler: r.name, wired: !!r.wired }); covered.add(ck); pageEvents++; }
          else report.refused.push({ page, what: n.id + '.' + row.name, why: r && r.notes ? r.notes.join(' / ') : 'nothing made' });
        }
      }
      process.stderr.write('[' + (pages.indexOf(page) + 1) + '/' + pages.length + '] ' + page + ': +' + newIds.length + ' components, +' + pageEvents + ' events (' + covered.size + ' class x event covered) ' + Math.round((Date.now() - T0) / 1000) + ' s\n');
      continue;
    }

    if (wave === 'evdel') {
      // every event already wired, on every component: reset WITH its C++ function (the warning's "delete")
      for (const c of comps) {
        let data = null;
        try { data = await select(c.id); } catch (e) { report.errors.push(page + ' ' + c.id + ' resolve: ' + (e && e.message)); continue; }
        const g = data && data.eventGrid;
        if (!g) continue;
        for (const row of g.rows.filter(r => r.handler)) {
          let r = null;
          try { data = await select(c.id); r = await hub.onEventReset(data, row.name, d, { answer: 'delete' }); }
          catch (e) { report.errors.push(page + ' ' + c.id + '.' + row.name + ' reset: ' + (e && e.stack || e)); continue; }
          report.deleted.push({ page, kind: 'event', control: c.id, cls: c.cls, event: row.name, handler: row.handler, ok: !!r, removedCpp: !!(r && r.removedCpp) });
        }
      }
      process.stderr.write('[' + (pages.indexOf(page) + 1) + '/' + pages.length + '] ' + page + ': ' + report.deleted.filter(x => x.page === page).length + ' events reset ' + Math.round((Date.now() - T0) / 1000) + ' s\n');
      continue;
    }

    if (wave === 'compdel') {
      // every component, the deepest first (a parent deleted first takes its children with it)
      const order = comps.slice().sort((a, b) => b.at - a.at);
      for (const c of order) {
        if (!he.startTagOf(doc.getText(), c.id)) continue;   // (gone with its parent)
        const V = process.env.HTD_E2E_VERBOSE ? s => process.stderr.write('  ' + s + ' ' + c.id + ' ' + Math.round((Date.now() - T0) / 1000) + ' s\n') : () => {};
        V('select');
        await select(c.id);
        V('delete');
        if (process.env.HTD_E2E_VERBOSE) {
          const d0 = hub.active, t0 = Date.now(), us = hub.unitsFor(doc.getText(), hub.selIds(d0));
          V('units ' + (Date.now() - t0) + 'ms');
          const t1 = Date.now(); await hub.webUsesOf(d0, [].concat(...us.units.map(u => hb.idsIn(u.html)))); V('webUsesOf ' + (Date.now() - t1) + 'ms');
        }
        let r = null;
        try { r = await hub.cmdDelete({ confirmed: true }); } catch (e) { report.errors.push(page + ' delete ' + c.id + ': ' + (e && e.stack || e)); continue; }
        const gone = !he.startTagOf(doc.getText(), c.id);
        report.deleted.push({ page, kind: 'component', id: c.id, cls: c.cls, ok: !!r && gone });
        if (!(r && gone)) report.refused.push({ page, what: 'delete ' + c.id, why: (fake._calls || []).slice(-3).map(x => x.name + ': ' + String(x.args && x.args[0]).slice(0, 160)).join(' | ') });
      }
      // (each page written and let go at once: a page left dirty in memory is scanned again by every later operation --
      //  30 pages in, every delete was crawling at 100% CPU)
      if (doc.isDirty) { fs.writeFileSync(doc.uri.fsPath, doc.getText(), 'utf8'); report.written.push(path.relative(path.resolve(PORT, '..'), doc.uri.fsPath)); doc.isDirty = false; }
      delete fake._docs[doc.uri.toString()];
      process.stderr.write('[' + (pages.indexOf(page) + 1) + '/' + pages.length + '] ' + page + ': ' + report.deleted.filter(x => x.page === page).length + ' deleted ' + Math.round((Date.now() - T0) / 1000) + ' s\n');
      continue;
    }
  }

  if (wave !== 'inventory') {
    // Save All: every changed document written (under the worktree)
    for (const x of Object.values(fake._docs).filter(x => x && x.isDirty && x.uri && x.uri.scheme !== 'untitled')) {
      const f = x.uri.fsPath;
      if (!path.resolve(f).toLowerCase().startsWith(path.resolve(PORT, '..').toLowerCase())) { report.errors.push('outside the worktree: ' + f); continue; }
      fs.writeFileSync(f, x.getText(), 'utf8');
      report.written.push(path.relative(path.resolve(PORT, '..'), f));
    }
  }
  report.counts = { pages: report.pages.length, opened: report.pages.filter(p => !p.note).length, comps: report.pages.reduce((a, p) => a + p.comps, 0),
    events: report.pages.reduce((a, p) => a + p.events, 0), wired: report.pages.reduce((a, p) => a + p.wired, 0),
    added: report.added.length, newEvents: report.events.length, deleted: report.deleted.length, deletedOk: report.deleted.filter(x => x.ok).length,
    refused: report.refused.length, errors: report.errors.length, written: report.written.length };
  fs.writeFileSync(repFile, JSON.stringify(report, null, 1), 'utf8');
}

main().then(() => process.exit(report.errors.length ? 1 : 0), e => {
  report.errors.push(String(e && e.stack || e));
  try { fs.writeFileSync(process.argv[3], JSON.stringify(report, null, 1), 'utf8'); } catch (x) { /* none */ }
  process.exit(2);
});
