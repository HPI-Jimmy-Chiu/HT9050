'use strict';
// AI(W906-HTDESIGNER) 20261001 (EastSun: "新增各種元件上去 或是新增事件 各種更改 都需要9050 可以編譯過 並且執行"):
// the designer's own code (extension.js with test\fake_vscode.js) does what a user does --
//   * the toolbox: every kind of component added to each page
//   * the events: every event of every new component (and new events on a .dfm component) added -- the handler in
//     the form's .h / .cpp, the page's htdCpp line, HtdEvents.gen.cpp, CMakeLists.txt, wb_serve.cpp's htd.event branch
//   * other changes: a handler renamed, an event reset, an existing handler picked for another event
// -- then "Save All": every changed document written to disk, UNDER THE TREE GIVEN (a throw-away worktree: never the
// machine's own tree). Each new handler body gets one marker line (the user's code), so a run can show it was called.
// e2e_build.ps1 then builds 9050 and runs wb_serve against what this wrote.
//   argv: <reportJson>   env: HTD_E2E_PORT = the C++ port tree (default: this extension's tree)
const Module = require('module');
const fs = require('fs');
const path = require('path');
const fake = require('./fake_vscode');

const origResolve = Module._resolveFilename;
Module._resolveFilename = function (req, parent, ...rest) {
  if (req === 'vscode') return require.resolve('./fake_vscode');
  return origResolve.call(this, req, parent, ...rest);
};

const MARK = 'HTD-E2E';
const report = { pages: [], added: [], events: [], changes: [], refused: [], written: [], errors: [] };

// pages and what to add (every toolbox class goes on the first page; a few on the others)
const PLAN = [
  { page: 'Setup.HotPlate.html', tools: 'ALL', dfmEvents: [['spbSave', ['OnMouseUp', 'OnDblClick']]] },
  // (main.html has no form root: '@form' = its <body> since 0.135)
  { page: 'main.html', tools: ['TSpeedButton', 'TEdit', 'TComboBox', 'TCheckBox'], dfmEvents: [] },
  // into a tab sheet (0.135, WPF: a control put on a TabItem goes into it): HW.IoSetView's tsStack1_Cassette
  { page: 'HW.IoSetView.html', tools: ['TSpeedButton', 'TLabel', 'TMyLedLane', 'TBtnPanelLane'], into: 'tsStack1_Cassette', dfmEvents: [],
    moveInto: 'tsStack1_Under' },   // (0.136: a component moved into another tab sheet -- Blend's reparent / the outline's drag)
];

function mkPanel() {
  const p = {
    posted: [], recv: null, active: true, visible: true, viewColumn: 1, reveal() {},
    onDidChangeViewState: () => ({ dispose() {} }), onDidDispose: () => ({ dispose() {} }),
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

async function main() {
  const [repFile] = process.argv.slice(2);
  const PORT = process.env.HTD_E2E_PORT ? path.resolve(process.env.HTD_E2E_PORT) : path.resolve(__dirname, '..', '..', '..');
  const WEB = path.join(PORT, '..', 'web');
  // never the machine's own trees
  if (/^[a-z]:\\ht9045\\/i.test(PORT + '\\') && !/_work|htd_work|e2e/i.test(PORT)) throw new Error('refusing to write under ' + PORT);
  report.port = PORT;
  fake._folders = [{ uri: fake.Uri.file(PORT) }];
  Object.assign(fake._config, { eventJump: 'cpp', syncSource: false });
  const ext = require('../extension');
  const memento = { _m: {}, get(k, d) { return k in this._m ? this._m[k] : d; }, update(k, v) { this._m[k] = v; return Promise.resolve(); } };
  const ctx = { subscriptions: [], extensionUri: fake.Uri.file(path.resolve(__dirname, '..')), workspaceState: memento,
    extension: { id: 'ht9045.ht9045-html-designer', extensionPath: path.resolve(__dirname, '..'), packageJSON: JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')) } };
  const api = ext.activate(ctx);
  const hub = api.hub;
  fake._props.resolveWebviewView({
    webview: { cspSource: 'x', options: null, html: '', asWebviewUri: u => ({ toString: () => 'x' }), postMessage: () => Promise.resolve(true), onDidReceiveMessage: () => ({ dispose() {} }) },
    onDidDispose: () => ({ dispose() {} }),
  });
  fake._docs = fake._docs || {};
  // a modal question (rename of a .dfm name ...) is answered with its first button
  const answer = (orig) => async (msg, opts, ...items) => (opts && opts.modal && items.length ? items[0] : undefined);
  if (fake.window && fake.window.showWarningMessage) fake.window.showWarningMessage = answer(fake.window.showWarningMessage);

  let key = 1000;
  for (const step of PLAN) {
    const file = path.join(WEB, 'page', step.page);
    if (!fs.existsSync(file)) { report.errors.push('no page ' + file); continue; }
    const doc = mkDoc(file);
    fake._docs[doc.uri.toString()] = doc;
    const panel = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, panel);
    panel.recv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });
    for (let i = 0; i < 100 && !panel.posted.some(m => m.type === 'init'); i++) await sleep(50);
    const d = hub.active;
    if (!d || d.doc !== doc) { report.errors.push(step.page + ': designer not active'); continue; }
    const pi = await d.info;
    const formCls = (pi.classes && pi.classes[0]) || '';
    report.pages.push({ page: step.page, form: formCls });

    // 1. the toolbox
    const toolbox = require('../lib/toolbox');
    const tools = step.tools === 'ALL' ? toolbox.ITEMS.map(i => i.cls) : step.tools;
    const newIds = [];
    let x = 900, y = 8;
    // a page without a form root (main.html is hand-made: no <div class="form">) -- put them into a real container
    // of the page, as a user would (select a Panel, then the toolbox)
    const into = step.into || '@form';
    if (step.into || !/<div\b[^>]*\bclass\s*=\s*["']form["']/.test(doc.getText())) x = 8;
    report.pages[report.pages.length - 1].into = into;
    const he = require('../lib/htmledit'), hb = require('../lib/htmlblock');
    for (const cls of tools) {
      const c0 = (fake._calls || []).length;
      const r = await hub.cmdAdd(cls, { into, x, y });
      if (r && r.id) {
        // where it landed: inside the container asked for (the tab sheet's pane / the body / the form)
        const t = doc.getText(), ct = he.containerTagOf(t, into), ce = ct ? hb.insideEnd(t, ct) : -1;
        const at = t.search(new RegExp('\\bid\\s*=\\s*["\']' + r.id + '["\']'));
        newIds.push({ id: r.id, cls: r.cls });
        report.added.push({ page: step.page, id: r.id, cls: r.cls, into, kind: ct && ct.kind, inside: !!(ct && at > ct.end && at < ce) });
      }
      else report.refused.push({ page: step.page, what: 'toolbox ' + cls, why: (fake._calls || []).slice(c0).map(c => c.name + ': ' + String(c.args && c.args[0]).slice(0, 200)).join(' | ') });
      y += 40;
    }

    // 1b. a component moved into another container (a tab sheet)
    if (step.moveInto && newIds.length) {
      const mv = newIds[0].id;
      const c0 = (fake._calls || []).length;
      const r = await hub.cmdReparent([mv], { id: step.moveInto }, { [mv]: { left: 16, top: 16 } });
      const t = doc.getText(), ct = he.containerTagOf(t, step.moveInto), ce = ct ? hb.insideEnd(t, ct) : -1;
      const at = t.search(new RegExp('\\bid\\s*=\\s*["\']' + mv + '["\']'));
      const inside = !!(r && ct && at > ct.end && at < ce);
      report.changes.push({ page: step.page, what: 'move into sheet', id: mv, into: step.moveInto, inside,
        why: r ? '' : (fake._calls || []).slice(c0).map(c => String(c.args && c.args[0]).slice(0, 160)).join(' | ') });
      const a = report.added.find(x => x.page === step.page && x.id === mv);
      if (a) { a.into = step.moveInto; a.kind = ct && ct.kind; a.inside = inside; }
    }

    // 2. the events
    const select = async (id) => {
      const info = { key: key++, id, tag: 'div', cls: '', title: '', attrs: [['id', id]], style: [], inline: [], computed: [], chain: ['@form'], cssPath: '', text: id,
        visible: true, geom: { left: 0, top: 0, width: 10, height: 10, rel: 'form' }, listeners: [], inherited: [] };
      d.sel = { info, data: null, promise: null };
      const data = await hub.resolve(d, info, () => {});
      d.sel.data = data;
      d.sel.promise = Promise.resolve(data);
      return data;
    };
    const addEvents = async (id, only) => {
      const data = await select(id);
      const g = data && data.eventGrid;
      if (!g) { report.refused.push({ page: step.page, what: id + ': no event grid' }); return []; }
      const made = [];
      for (const row of g.rows.slice()) {
        if (only && !only.includes(row.name)) continue;
        if (row.handler) continue;
        let r = null;
        try { r = await hub.onEventGrid(data, row.name, d); } catch (e) { report.errors.push(step.page + ' ' + id + '.' + row.name + ': ' + (e && e.stack || e)); continue; }
        if (r && r.name) {
          const e = { page: step.page, tag: step.page.replace(/\.html?$/i, ''), form: g.formCls, control: id, event: row.name, handler: r.name, h: r.h, cpp: r.cpp, wired: !!r.wired, notes: r.notes || [] };
          report.events.push(e); made.push(e);
        } else report.refused.push({ page: step.page, what: id + '.' + row.name, why: r && r.notes ? r.notes.join(' / ') : 'nothing made' });
      }
      return made;
    };
    for (const n of newIds) await addEvents(n.id);
    for (const [id, evs] of step.dfmEvents) await addEvents(id, evs);

    // 3. other changes: rename one new handler, reset one, give one event an existing handler
    // (0.162: how many rows the generated table has before / after each -- a change must take off only its own row)
    const genRows = () => { const g = hub.htdGen(d.r.portRoot); return g && g.text ? require('../lib/cppbridge').rowsOf(g.text).length : -1; };
    report.genRows = report.genRows || [];
    report.genRows.push(step.page + ' before: ' + genRows());
    const mine = report.events.filter(e => e.page === step.page && e.wired);
    if (mine.length >= 3) {
      const ren = mine[0];
      const data = await select(ren.control);
      const nu = ren.handler + 'Renamed';
      try {
        await hub.onEventName(data, ren.event, nu, d);
        report.changes.push({ page: step.page, what: 'rename', from: ren.handler, to: nu });
        report.genRows.push(step.page + ' after rename: ' + genRows());
        ren.handler = nu;
      } catch (e) { report.errors.push('rename: ' + (e && e.stack || e)); }
      const rs = mine[1];
      try {
        const data2 = await select(rs.control);
        const r = await hub.onEventReset(data2, rs.event, d, { answer: 'delete' });   // (0.162: with its C++ function -- the build below must still pass)
        if (r) { report.changes.push({ page: step.page, what: 'reset', control: rs.control, event: rs.event, handler: rs.handler }); rs.reset = true; }
        report.genRows.push(step.page + ' after delete: ' + genRows());
      } catch (e) { report.errors.push('reset: ' + (e && e.stack || e)); }
      // an existing handler (same signature) for another control's event: the first new one's OnClick-like handler
      const src = mine.find(e => /OnClick$/.test(e.event) && !e.reset);
      // (the same id can be on two pages -- Edit1 on HotPlate and on main: always this page's)
      const dst = src && report.added.find(a => a.page === step.page && a.id !== src.control && report.events.some(e => e.page === step.page && e.control === a.id && e.event === src.event));
      if (src && dst) {
        try {
          const data3 = await select(dst.id);
          const row = data3.eventGrid.rows.find(r => r.name === src.event);
          const old = row && row.handler;
          await hub.onEventName(data3, src.event, src.handler, d);
          report.changes.push({ page: step.page, what: 'use existing', control: dst.id, event: src.event, handler: src.handler, was: old });
          report.genRows.push(step.page + ' after use: ' + genRows());
          const e = report.events.find(x => x.page === step.page && x.control === dst.id && x.event === src.event);
          if (e) { e.handler = src.handler; e.reused = true; }
        } catch (e) { report.errors.push('use existing: ' + (e && e.stack || e)); }
      }
    }
  }

  // 4. Save All -- plus the user's code: one marker line in each new handler body
  const docs = Object.values(fake._docs).filter(x => x && x.isDirty && x.uri && x.uri.scheme !== 'untitled');
  const bodies = new Map();
  for (const e of report.events) if (!e.reset && !e.reused) { if (!bodies.has(e.cpp)) bodies.set(e.cpp, []); bodies.get(e.cpp).push(e); }
  for (const x of docs) {
    let t = x.getText();
    const f = x.uri.fsPath;
    const evs = bodies.get(f) || [];
    for (const e of evs) {
      const re = new RegExp('(void\\s+' + e.form + '::' + e.handler + '\\s*\\([^)]*\\)\\s*\\r?\\n?\\s*\\{)');
      const m = re.exec(t);
      if (!m) { report.errors.push('no body for ' + e.form + '::' + e.handler + ' in ' + path.basename(f)); continue; }
      const eol = /\r\n/.test(t) ? '\r\n' : '\n';
      const line = eol + '    std::fprintf(stderr, "' + MARK + ' ' + e.form + '::' + e.handler + '\\n"); std::fflush(stderr);   // e2e_build: the user\'s code';
      t = t.slice(0, m.index + m[0].length) + line + t.slice(m.index + m[0].length);
      e.marked = true;
    }
    if (evs.length && !/#include\s*<cstdio>/.test(t)) t = '#include <cstdio>   // e2e_build' + (/\r\n/.test(t) ? '\r\n' : '\n') + t;
    fs.writeFileSync(f, t, 'utf8');
    report.written.push(path.relative(PORT, f));
  }
  report.counts = { added: report.added.length, events: report.events.length, wired: report.events.filter(e => e.wired).length, changes: report.changes.length, refused: report.refused.length, errors: report.errors.length };
  fs.writeFileSync(repFile, JSON.stringify(report, null, 1), 'utf8');
}

main().then(() => process.exit(report.errors.length ? 1 : 0), e => {
  report.errors.push(String(e && e.stack || e));
  try { fs.writeFileSync(process.argv[2], JSON.stringify(report, null, 1), 'utf8'); } catch (x) { /* none */ }
  process.exit(2);
});
