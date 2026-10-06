'use strict';
// AI(W906-HTDESIGNER) 20261006: the 'wpf' view (0.140, EastSun's default -- "如果點擊頁面 跟 WPF 一樣 上面是畫面 下面是對應原始碼；
// 我切到 C++ 時要自動關閉該頁面") in a REAL VS Code: a page opens as two rows (the design above, its HTML below); another
// page closes the first; a C++ file closes every .html tab and the area is one again; a page with unsaved changes stays.
// Nothing is saved (the one edit is reverted). Launched by test\vscode_wpf_it.ps1.
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
    await sleep(step || 200);
  }
}

exports.run = async function () {
  const out = [];
  let pass = 0, fail = 0;
  const report = process.env.HTD_IT_REPORT;
  const flush = () => { if (report) try { fs.writeFileSync(report, out.join('\n'), 'utf8'); } catch (e) { /* next time */ } };
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); flush(); };
  const groups = () => vscode.window.tabGroups.all;
  const tabsOf = re => groups().flatMap(g => g.tabs.map(t => ({ t, col: g.viewColumn }))).filter(x => x.t.input && x.t.input.uri && re.test(x.t.input.uri.fsPath));
  const desc = () => groups().map(g => g.viewColumn + ':[' + g.tabs.map(t => (t.input && t.input.uri ? path.basename(t.input.uri.fsPath) : t.label) + (t.input && t.input.viewType ? '(design)' : '')).join(',') + ']').join(' ');
  try {
    out.push('vscode ' + vscode.version);
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    const api = await ext.activate();
    const hub = api.hub;
    const web = process.env.HTD_WEB_PAGE_DIR, port = process.env.HTD_PORT_ROOT;
    ok(hub.defaultView() === 'wpf', 'the view is wpf (the default)', hub.defaultView());
    const pA = path.join(web, 'Setup.HotPlate.html'), pB = path.join(web, 'Setup.Speed.html');
    // (1) a page: two rows, the design surface above (column 1), its HTML below (column 2)
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pA), 'ht9045Designer.editor');
    const two = await waitFor(() => {
      const d = tabsOf(/Setup\.HotPlate\.html$/i).find(x => x.t.input.viewType), h = tabsOf(/Setup\.HotPlate\.html$/i).find(x => !x.t.input.viewType);
      return groups().length === 2 && d && h && d.col === 1 && h.col === 2 ? true : null;
    }, 20000);
    ok(!!two, 'a page opens as two rows: the design above (1), its HTML below (2)', desc());
    // (2) another page: the first one's tabs close, the new one in the same two rows
    await sleep(1200);
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pB), 'ht9045Designer.editor');
    const swapped = await waitFor(() => (!tabsOf(/Setup\.HotPlate\.html$/i).length && tabsOf(/Setup\.Speed\.html$/i).length === 2 && groups().length === 2 ? true : null), 20000);
    ok(!!swapped, 'another page: the first one closes (one page at a time), the new one above / below', desc());
    // (3) a C++ file: every .html tab closes, the area is one group
    await sleep(1200);
    const cpp = path.join(port, 'forms', 'fHotPlate.cpp');
    await vscode.window.showTextDocument(vscode.Uri.file(cpp), { preview: false });
    const one = await waitFor(() => (!tabsOf(/\.html?$/i).length && groups().length === 1 ? true : null), 10000);
    ok(!!one && /fHotPlate\.cpp$/i.test((vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri.fsPath) || ''),
      'a C++ file: every .html tab closes, one editor area, the C++ in front', desc());
    // (4) a page with unsaved changes stays when the C++ comes to the front (then reverted -- nothing saved)
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pA), 'ht9045Designer.editor');
    await waitFor(() => (tabsOf(/Setup\.HotPlate\.html$/i).length === 2 ? true : null), 20000);
    await sleep(1200);
    const doc = vscode.workspace.textDocuments.find(x => /Setup\.HotPlate\.html$/i.test(x.uri.fsPath));
    const before = doc ? doc.getText() : null;
    if (doc) { const we = new vscode.WorkspaceEdit(); we.insert(doc.uri, doc.positionAt(doc.getText().length), ' '); await vscode.workspace.applyEdit(we); }
    await vscode.window.showTextDocument(vscode.Uri.file(cpp), { preview: false });
    await sleep(2500);
    const kept = tabsOf(/Setup\.HotPlate\.html$/i).filter(x => x.t.isDirty).length;
    ok(!!doc && doc.isDirty && kept >= 1, 'a page with unsaved changes is not closed by the C++ (its tab stays)', desc());
    if (doc) {
      await vscode.window.showTextDocument(doc, { preview: false });
      await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (!doc.isDirty ? true : null), 5000);
    }
    ok(!!doc && !doc.isDirty && doc.getText() === before && fs.readFileSync(pA, 'utf8') === before, 'put back: the page as it was, nothing saved');
  } catch (e) {
    ok(false, 'the test ran', String(e && e.stack || e));
  }
  out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  flush();
  await vscode.commands.executeCommand('workbench.action.closeWindow');
};
