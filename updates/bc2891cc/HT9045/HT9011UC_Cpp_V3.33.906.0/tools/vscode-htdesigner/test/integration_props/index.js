'use strict';
// AI(W906-HTDESIGNER) 20261006 (ES02, EastSun: "裡面屬性設定也都要有功能 ... 我不要用抽樣的 要用枚舉"): every property row of the
// 屬性 panel, for every component class on the pages, in a REAL VS Code (the real probe in the real webview, the real
// extension). Launched by test\vscode_props_it.ps1. Two modes (env HTD_PROPS_MODE):
//   dump   : one component of each class selected; the panel's data (what props.js draws its rows from) -> <out>\<cls>.json
//   replay : the messages each row sends (found by test\props_rows.ps1 rendering props.js in Edge) sent again here, one by
//            one: the page's source must change, the panel's data after it must differ there; the source is put back
//            after each (nothing is saved -- the page files on disk are never written).
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

const sleep = ms => new Promise(r => setTimeout(r, ms));
async function waitFor(fn, ms, step) {
  const end = Date.now() + ms;
  for (;;) {
    let v = null;
    try { v = await fn(); } catch (e) { v = null; }
    if (v) return v;
    if (Date.now() > end) return null;
    await sleep(step || 100);
  }
}

exports.run = async function () {
  const out = [];
  let pass = 0, fail = 0;
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); };
  const report = process.env.HTD_IT_REPORT;
  const mode = process.env.HTD_PROPS_MODE || 'dump';
  const dir = process.env.HTD_PROPS_DIR;
  const saves = [];
  const saveSub = vscode.workspace.onDidSaveTextDocument(doc => { if (!/[\\/]htd_vscode_it_/i.test(doc.uri.fsPath)) saves.push(doc.uri.fsPath); });
  try {
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    const api = await ext.activate();
    const hub = api.hub;
    const pages = path.join(String(vscode.workspace.getConfiguration('ht9045Designer').get('webRoot')), 'page');
    let open = null;
    const openPage = async (page) => {
      if (open === page && hub.active && path.basename(hub.active.file) === page) return hub.active;
      await vscode.commands.executeCommand('workbench.action.closeAllEditors');
      await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(path.join(pages, page)), 'ht9045Designer.editor');
      const d = await waitFor(() => (hub.active && path.basename(hub.active.file) === page && hub.active.treeData.length ? hub.active : null), 30000);
      open = d ? page : null;
      return d;
    };
    const select = async (d, id) => {
      hub.props.data = null;
      d.post({ type: 'selectId', id, origin: 'editor' });
      return waitFor(() => { const x = hub.props.data; return x && x.comp && (x.comp.htmlId === id || (id === '@form' && x.comp.isForm)) ? x : null; }, 10000);
    };

    if (mode === 'dump') {
      const plan = JSON.parse(fs.readFileSync(path.join(dir, 'plan.json'), 'utf8'));   // [{ cls, page, id }]
      for (const p of plan) {
        const d = await openPage(p.page);
        if (!d) { ok(false, p.cls + ': ' + p.page + ' opened'); continue; }
        const data = await select(d, p.id);
        ok(!!data, p.cls + ': ' + p.id + ' on ' + p.page + ' selected, the panel has its data');
        if (data) fs.writeFileSync(path.join(dir, p.cls + '.json'), JSON.stringify(Object.assign({ __page: p.page, __id: p.id }, data)), 'utf8');
      }
    } else {
      const plan = JSON.parse(fs.readFileSync(path.join(dir, 'replay.json'), 'utf8'));   // [{ cls, page, id, rows: [{ row, msg }] }]
      const results = [];
      for (const p of plan) {
        const d = await openPage(p.page);
        if (!d) { ok(false, p.cls + ': ' + p.page + ' opened'); continue; }
        // (round 2 showed it: putting the text back after EACH row made the designer reload the whole page, and the next
        //  row's message went to a page still loading -- lost. The rows of one component now go one after another on the
        //  page as it is, each change on top of the last, and the text is put back once, after the last row)
        const tClass = d.doc.getText();
        for (const r of p.rows) {
          const before = await select(d, r.msg.type === 'rename' ? p.id : p.id);
          if (!before) { results.push({ cls: p.cls, row: r.row, msg: r.msg.type, ok: false, why: 'not selected' }); continue; }
          const t0 = d.doc.getText();
          const b0 = JSON.stringify(before.edit || {}) + JSON.stringify(before.comp || {});
          let res = null, err = '';
          const l0 = (hub.logLines || []).length;
          // (a rename of a .dfm name asks first -- a test VS Code refuses every dialog: the rename as confirmed)
          try { res = r.msg.type === 'rename' ? await hub.cmdRename(r.msg.name, true) : await hub.onPropsMessage(r.msg); } catch (e) { err = String(e && e.message || e); }
          const changed = await waitFor(() => d.doc.getText() !== t0, 12000);
          const said = (hub.logLines || []).slice(l0).join(' / ').slice(0, 400);
          const t1 = d.doc.getText();
          // the panel's data after the change (the probe re-reads the element)
          const after = changed ? await select(d, r.msg.type === 'rename' && r.msg.name ? r.msg.name : p.id) : null;
          const a1 = after ? JSON.stringify(after.edit || {}) + JSON.stringify(after.comp || {}) : '';
          const diffAt = changed ? (() => { let i = 0; while (i < t0.length && t0[i] === t1[i]) i++; return t1.slice(Math.max(0, i - 40), i + 80).replace(/\s+/g, ' '); })() : '';
          results.push({ cls: p.cls, id: p.id, row: r.row, msg: r.msg, changed: !!changed, readBack: !!after && a1 !== b0, err, diffAt, said });
          fs.writeFileSync(path.join(dir, 'replay_result.json'), JSON.stringify(results, null, 1), 'utf8');
        }
        // back as it was, once: the whole text replaced (one edit); the designer reloads the page -- waited for
        if (d.doc.getText() !== tClass) {
          const we = new vscode.WorkspaceEdit();
          we.replace(d.doc.uri, new vscode.Range(d.doc.positionAt(0), d.doc.positionAt(d.doc.getText().length)), tClass);
          await vscode.workspace.applyEdit(we);
          await waitFor(() => d.doc.getText() === tClass, 5000);
          await sleep(3000);
        }
        // (the page left unsaved and unchanged)
        if (d.doc.isDirty) { try { await vscode.commands.executeCommand('workbench.action.files.revert'); } catch (e) { /* (the text is back already) */ } }
      }
      fs.writeFileSync(path.join(dir, 'replay_result.json'), JSON.stringify(results, null, 1), 'utf8');
      for (const x of results) ok(x.changed && x.readBack && !x.err, x.cls + ' · ' + x.row + ' (' + x.msg.type + (x.msg.prop ? ':' + x.msg.prop : '') + ')', x.err || (x.changed ? (x.readBack ? '' : 'source changed, panel data did not') : 'source not changed' + (x.said ? ' -- ' + x.said : '')) + (x.diffAt ? '   …' + x.diffAt.slice(0, 120) : ''));
    }
    ok(saves.length === 0, 'NO-SAVES: nothing was saved by the test', saves.join(', '));
  } catch (e) {
    ok(false, 'CRASH ' + (e && e.stack || e));
  } finally {
    saveSub.dispose();
    out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
    if (report) fs.writeFileSync(report, out.join('\n') + '\n', 'utf8');
  }
};
