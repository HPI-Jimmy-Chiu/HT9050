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
    // (1008 EastSun「做在 上面一排」: the run strip -- its own group on top of the editor area, made at start-up; the WPF rows go
    //  under it: the design in 2, the HTML in 3, a C++ file = the strip + one area)
    const stripG = () => groups().find(g => g.tabs.some(t => /執行與除錯/.test(t.label)));
    const strip0 = await waitFor(() => stripG() || null, 15000);
    ok(!!strip0 && strip0.viewColumn === 1, 'the run strip is made at start-up: a group of its own on top (group 1)', desc());
    // (the strip is sized right after it is made -- a moment later than it appears)
    const lay0 = await waitFor(async () => { const l = await vscode.commands.executeCommand('vscode.getEditorLayout'); return l && l.groups && l.groups[0].size <= 90 ? l : null; }, 5000) || await vscode.commands.executeCommand('vscode.getEditorLayout');
    ok(!!lay0 && lay0.orientation === 1 && lay0.groups && lay0.groups[0].size <= 90, 'the strip is a thin row (about 78 px) above the editors', JSON.stringify(lay0));
    const work = () => groups().filter(g => g !== stripG());
    const stripOk = () => { const s = stripG(); return !!s && s.viewColumn === 1; };
    // (1) a page: two rows under the strip, the design surface above (column 2), its HTML below (column 3)
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pA), 'ht9045Designer.editor');
    const two = await waitFor(() => {
      const d = tabsOf(/Setup\.HotPlate\.html$/i).find(x => x.t.input.viewType), h = tabsOf(/Setup\.HotPlate\.html$/i).find(x => !x.t.input.viewType);
      return work().length === 2 && stripOk() && d && h && d.col === 2 && h.col === 3 ? true : null;
    }, 20000);
    ok(!!two, 'a page opens as two rows under the strip: the design above (2), its HTML below (3)', desc());
    // (2) another page: the first one's tabs close, the new one in the same two rows
    await sleep(1200);
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pB), 'ht9045Designer.editor');
    const swapped = await waitFor(() => (!tabsOf(/Setup\.HotPlate\.html$/i).length && tabsOf(/Setup\.Speed\.html$/i).length === 2 && work().length === 2 && stripOk() ? true : null), 20000);
    ok(!!swapped, 'another page: the first one closes (one page at a time), the new one above / below', desc());
    // (3) a C++ file: every .html tab closes, the area is the strip + one group
    await sleep(1200);
    const cpp = path.join(port, 'forms', 'fHotPlate.cpp');
    await vscode.window.showTextDocument(vscode.Uri.file(cpp), { preview: false });
    const one = await waitFor(() => (!tabsOf(/\.html?$/i).length && work().length === 1 && stripOk() ? true : null), 10000);
    ok(!!one && /fHotPlate\.cpp$/i.test((vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri.fsPath) || ''),
      'a C++ file: every .html tab closes, one editor area under the strip, the C++ in front', desc());
    // (1008: a file opened while the strip is the active group goes below it -- the strip's group is locked)
    await vscode.commands.executeCommand('workbench.action.focusFirstEditorGroup');
    await vscode.window.showTextDocument(vscode.Uri.file(path.join(port, 'forms', 'fHotPlate.h')), { preview: false });
    await sleep(500);
    const sg = stripG();
    ok(!!sg && sg.tabs.length === 1 && tabsOf(/fHotPlate\.h$/i).every(x => x.col !== 1), 'a file opened with the strip focused goes below it (the strip group holds only the strip)', desc());
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
    // (1008 EastSun「點擊 方案總管檔案 怎檔案直接出現在右邊了」: every file closed = VS Code drops the area under the strip; it is
    //  put back, and a file opened from 方案總管 lands under the strip -- not in a new group to its right)
    for (const g of groups()) { if (g === stripG()) continue; await vscode.window.tabGroups.close(g.tabs, true); }
    await sleep(1200);
    const back = await waitFor(async () => { const l = await vscode.commands.executeCommand('vscode.getEditorLayout'); return groups().length === 2 && l.orientation === 1 && l.groups[0].size <= 90 ? l : null; }, 5000);
    await hub.solution.open({ path: cpp, p: { area: 'port' } });
    await sleep(1200);
    const layR = await vscode.commands.executeCommand('vscode.getEditorLayout');
    const cppTab = tabsOf(/fHotPlate\.cpp$/i)[0];
    ok(!!back && layR.orientation === 1 && cppTab && cppTab.col === 2 && stripOk(), 'every file closed: the area under the strip comes back; a file from 方案總管 opens under the strip, not to its right', desc() + ' ' + JSON.stringify(layR));
    // (1008 EastSun「我畫面 按鈕不見了」: the strip's tab ended up in the files' group, behind asendic_Loader.cpp -- it is put back
    //  on top, alone, by itself; the side bars draw their buttons only while it stands right)
    const sg2 = stripG();
    let moved = '';
    if (sg2 && hub.runStrip.panel) {
      hub.runStrip.repairing = true;   // (held off a moment, so the bad shape can be seen first)
      hub.runStrip.panel.reveal(vscode.ViewColumn.One, false);
      await sleep(300);
      await vscode.commands.executeCommand('workbench.action.unlockEditorGroup');
      await vscode.commands.executeCommand('workbench.action.moveEditorToBelowGroup');
      await sleep(600);
      moved = desc();
      hub.runStrip.repairing = false;
      await hub.runStrip.keepShape();
    }
    const fixed = await waitFor(() => { const s = stripG(); return s && s.viewColumn === 1 && s.tabs.length === 1 && tabsOf(/fHotPlate\.cpp$/i).length === 1 && tabsOf(/fHotPlate\.cpp$/i).every(x => x.col !== 1) && groups().length === 2 ? true : null; }, 8000);
    const wasWrong = /^(?!.*1:\[▶ 執行與除錯\(design\)\]( |$)).*執行與除錯/.test(moved);
    ok(wasWrong && !!fixed && hub.runStrip.isOn() && hub.runStrip.closedByUser() === false, 'the strip moved into the files\' group (behind a file): put back on top by itself, alone; not taken as closed by the user', moved + ' -> ' + desc());
    // (1008 EastSun「上方按鈕排怎又不見了 我剛剛把 有個東西關掉 直接整個消失」: its tab closed (×) = made again at once, on top, not
    //  hidden for good; the side bars leave the buttons to it (「這邊功能重複了吧」); closed three times in a minute = left
    //  closed for this window, the side bars draw the buttons, 顯示上方執行列 brings it back)
    const closeStrip = async () => { const s = stripG(); if (s) await vscode.window.tabGroups.close(s.tabs, true); };
    await closeStrip();
    const again = await waitFor(() => (stripOk() && hub.runStrip.isOn() ? true : null), 8000);
    const okAgain = !!again && hub.runStrip.closedByUser() === false && hub.runStrip.covers() === true;
    await sleep(800);
    await closeStrip();
    await waitFor(() => (stripOk() ? true : null), 8000);
    await sleep(800);
    await closeStrip();
    await sleep(1500);
    const off = !stripG() && hub.runStrip.closedByUser() === true && hub.runStrip.covers() === false;
    await vscode.commands.executeCommand('ht9045Designer.runStrip.show');
    const shown = await waitFor(() => (stripOk() && hub.runStrip.isOn() && hub.runStrip.closedByUser() === false ? true : null), 8000);
    ok(okAgain && off && !!shown, 'the strip\'s tab closed: back on top at once (side bars leave the buttons to it); three closes in a minute: left closed for this window; 顯示上方執行列 brings it back', JSON.stringify({ okAgain, off, shown: !!shown }) + ' ' + desc());
  } catch (e) {
    ok(false, 'the test ran', String(e && e.stack || e));
  }
  out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  flush();
  await vscode.commands.executeCommand('workbench.action.closeWindow');
};
