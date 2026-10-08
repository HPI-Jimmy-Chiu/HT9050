'use strict';
// AI(W906-HTDESIGNER) 20261007 (EastSun "驗證功能"): the 0.245 C++ features in a REAL VS Code -- launched by
// test\vscode_cpp_it.ps1 (a throw-away instance; nothing of the user's is touched):
//   1. 異常字元: a temp .cpp with Big5 bytes, a full-width ';', a NBSP, curly quotes -> Error diagnostics on those places
//      (an Error = the red underline and the red mark in the scroll bar, VS Code's own drawing);
//   2. 編譯器檢查: a real .cpp of the port tree, a broken line added in the EDITOR only (never saved), the tree's own g++
//      with the build's flags -> an Error on that line; the file reverted, byte for byte as before;
//   3. 建置錯誤: real g++ output for a broken temp file -> 問題 entries; F8 (next problem) puts the cursor on that line;
//   4. 方案總管 新增: a .h and a UI page made in a temp folder, opened.
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const os = require('os');

const sleep = ms => new Promise(r => setTimeout(r, ms));
async function waitFor(fn, ms, step) {
  const end = Date.now() + ms;
  for (;;) {
    let v = null;
    try { v = await fn(); } catch (e) { v = null; }
    if (v) return v;
    if (Date.now() > end) return null;
    await sleep(step || 250);
  }
}

exports.run = async function () {
  const out = [];
  let pass = 0, fail = 0;
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); };
  const report = process.env.HTD_IT_REPORT;
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'htd_cpp_it_'));
  try {
    out.push('vscode ' + vscode.version);
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    const api = await ext.activate();
    const hub = api.hub, ck = hub.cppChecks;
    ok(!!ck, 'C++ checks are running in this VS Code', ck ? 'CppChecks' : 'none');
    const diagOf = (uri, src) => vscode.languages.getDiagnostics(uri).filter(d => !src || d.source === src);

    // ---- 1. 異常字元 ----
    const f1 = path.join(tmp, 'bad_chars.cpp');
    const big5 = Buffer.from([0xa7, 0x6c, 0xbc, 0x4c]);   // 吸嘴 in Big5
    fs.writeFileSync(f1, Buffer.concat([
      Buffer.from('// ok: 中文註解\r\nconst char* s = "中文字串";\r\nint a = 1；\r\nint b = 2;\r\nconst char* q = “x”;\r\n', 'utf8'),
      Buffer.from('int c = 3; // '), big5, Buffer.from('\r\n'),
    ]));
    const d1 = await vscode.workspace.openTextDocument(f1);
    await vscode.window.showTextDocument(d1, { preview: false });
    const g1 = await waitFor(() => { const g = diagOf(d1.uri, '異常字元'); return g.length >= 4 ? g : null; }, 8000);
    const lines1 = (g1 || []).map(d => d.range.start.line + ':' + d.range.start.character).join(',');
    const allErr = !!g1 && g1.every(d => d.severity === vscode.DiagnosticSeverity.Error);
    ok(!!g1 && allErr && /2:9/.test(lines1) && /3:3/.test(lines1) && /4:16/.test(lines1) && /5:/.test(lines1) && !g1.some(d => d.range.start.line <= 1),
      '1. 異常字元: full-width ; (line 3), NBSP (line 4), curly quotes (line 5), Big5 bytes (line 6) marked as Errors (red underline + scroll bar); the Chinese comment and string (lines 1-2) not',
      lines1 + ' / ' + (g1 || []).map(d => d.message.slice(0, 18)).join(' | '));
    const ci = ck.info();
    out.push('      the checks follow: ' + JSON.stringify(ci));

    // ---- 2. 編譯器檢查 (the editor's text, never saved) ----
    const tree = hub.runBar.plan().tree;
    const real = path.join(tree, 'Public', 'cBootLog.cpp');
    const before = fs.readFileSync(real);
    const d2 = await vscode.workspace.openTextDocument(real);
    const e2 = await vscode.window.showTextDocument(d2, { preview: false });
    const ln = d2.lineCount;
    await e2.edit(e => e.insert(new vscode.Position(ln - 1, d2.lineAt(ln - 1).text.length), '\r\nint htdBrokenOnPurpose = ;\r\n'));
    const t2 = Date.now();
    const ps2 = await ck.syntaxCheck(d2);
    const ms2 = Date.now() - t2;
    const g2 = diagOf(d2.uri, '編譯器');
    const hit2 = g2.find(d => d.severity === vscode.DiagnosticSeverity.Error && d.range.start.line === ln);
    ok(!!ps2 && !!hit2, '2. 編譯器檢查: the real g++ (the build\'s flags) finds the broken line added in the editor -- line ' + (ln + 1),
      (hit2 ? hit2.message : JSON.stringify(g2.map(d => [d.range.start.line, d.message]).slice(0, 3))) + '  (' + ms2 + ' ms)');
    // (1008 gap list #10: Alt+F9 Compile Unit -- the same check on demand, the unsaved broken line found)
    const cu = await vscode.commands.executeCommand('ht9045Designer.cpp.compileUnit');
    ok(!!cu && cu.errors >= 1, '2. Compile Unit (Alt+F9): the real compiler on the open file now, unsaved text too -- the broken line is an error', JSON.stringify(cu));
    await vscode.commands.executeCommand('workbench.action.files.revert');
    await sleep(500);
    const ps2b = await ck.syntaxCheck(d2);
    const g2b = diagOf(d2.uri, '編譯器').filter(d => d.severity === vscode.DiagnosticSeverity.Error);
    ok(!!ps2b && g2b.length === 0 && !d2.isDirty, '2. 編譯器檢查: the file as it is (reverted) has no compiler error', g2b.map(d => d.message).join(' | '));
    ok(Buffer.compare(before, fs.readFileSync(real)) === 0, '2. the real file is byte for byte as before (nothing was saved)');
    await vscode.commands.executeCommand('workbench.action.closeActiveEditor');

    // ---- 3. 建置錯誤: real g++ output -> 問題, F8 goes there ----
    const f3 = path.join(tmp, 'broken.cpp');
    fs.writeFileSync(f3, 'int ok1 = 1;\r\nint ok2 = 2;\r\nint broken = ;\r\nvoid f() { undeclared_thing(); }\r\n');
    const cmp = (ci && ci.dir) ? require('../../lib/builderrors').compileCommand(ci.dir, path.join(tree, 'Public', 'cBootLog.cpp')) : null;
    const gxx = cmp ? cmp.compiler : 'g++';
    const env = Object.assign({}, process.env, { PATH: path.dirname(gxx) + path.delimiter + (process.env.PATH || '') });
    const real3 = await new Promise(res => require('child_process').execFile(gxx, ['-fsyntax-only', f3], { env, windowsHide: true, timeout: 60000 }, (e, so, se) => res(String(se || '') + String(so || ''))));
    out.push('      g++ said: ' + real3.split(/\r?\n/).filter(l => /error/.test(l)).join(' / ').slice(0, 300));
    const ps3 = ck.fromBuildOutput(real3, tmp, tmp, true);
    const d3 = await vscode.workspace.openTextDocument(f3);
    const g3 = await waitFor(() => { const g = diagOf(d3.uri, '建置'); return g.length >= 2 ? g : null; }, 5000);
    ok(!!g3 && g3.some(d => d.range.start.line === 2) && g3.some(d => d.range.start.line === 3) && ps3.length >= 2,
      '3. 建置錯誤: the compiler\'s errors are 問題 entries on their lines (3 and 4)', (g3 || []).map(d => (d.range.start.line + 1) + ': ' + d.message).join(' | '));
    // (a click in the 問題 panel = go to that problem; F8 = the same navigation, from the top of the file)
    const e3 = await vscode.window.showTextDocument(d3, { preview: false, selection: new vscode.Range(0, 0, 0, 0) });
    await vscode.commands.executeCommand('editor.action.marker.next');
    await sleep(400);
    const at3 = vscode.window.activeTextEditor ? vscode.window.activeTextEditor.selection.active.line : -1;
    ok(at3 === 2, '3. 建置錯誤: going to the next problem (F8 / a click in 問題) puts the cursor on the error line', 'line ' + (at3 + 1));
    void e3;
    // (1008 gap list #11: BCB bookmarks -- Ctrl+Shift+3 on line 2, the cursor moved away, Ctrl+3 comes back)
    const ed3 = vscode.window.activeTextEditor;
    if (ed3) {
      ed3.selection = new vscode.Selection(1, 0, 1, 0);
      const bt = await vscode.commands.executeCommand('ht9045Designer.bookmark.toggle', 3);
      ed3.selection = new vscode.Selection(3, 0, 3, 0);
      const bg = await vscode.commands.executeCommand('ht9045Designer.bookmark.goto', 3);
      const atB = vscode.window.activeTextEditor ? vscode.window.activeTextEditor.selection.active.line : -1;
      const bt2 = await vscode.commands.executeCommand('ht9045Designer.bookmark.toggle', 3);
      ok(!!bt && bt.on && !!bg && atB === 1 && bt2 && !bt2.on, '3b. bookmarks: Ctrl+Shift+3 set on line 2, Ctrl+3 back there from line 4, Ctrl+Shift+3 again clears it', JSON.stringify({ bt, atB, bt2 }));
    }

    // ---- 5. IntelliSense from the build (1008 gap list #7) ----
    const cppExt = vscode.extensions.getExtension('ms-vscode.cpptools');
    if (cppExt) {
      const prov = await waitFor(() => ck.cppProvider || null, 30000);
      const realH = path.join(tree, 'Public', 'cBootLog.h');
      const cfgs = prov ? await prov.provideConfigurations([vscode.Uri.file(real), vscode.Uri.file(realH)]) : [];
      const c0 = cfgs[0] && cfgs[0].configuration, c1 = cfgs[1] && cfgs[1].configuration;
      const prop = vscode.workspace.getConfiguration('C_Cpp').get('default.configurationProvider');
      ok(!!prov && !!c0 && c0.includePath.some(p => path.resolve(p).toLowerCase() === path.resolve(tree).toLowerCase()) && /^c\+\+1[47]$/.test(c0.standard) && /g\+\+\.exe$/i.test(c0.compilerPath) && !!c1 && prop === 'ht9045.ht9045-html-designer',
        '5. IntelliSense: the provider is registered with C/C++, a .cpp and its .h get the build\'s include paths / standard / compiler, C/C++ is told to use it',
        JSON.stringify({ prov: !!prov, std: c0 && c0.standard, inc: c0 && c0.includePath.length, defs: c0 && c0.defines.length, comp: c0 && path.basename(c0.compilerPath || ''), h: !!c1, prop }));
    } else out.push('      (C/C++ extension not in this instance: 5. skipped)');

    // ---- 4. 方案總管 新增 ----
    const nh = await hub.cmdNewHeader(null, { dir: tmp, name: 'MyNewHeader' });
    const hText = nh ? fs.readFileSync(nh.file, 'utf8') : '';
    const act = vscode.window.activeTextEditor;
    ok(!!nh && /#ifndef \w*MYNEWHEADER_H_/.test(hText) && act && path.basename(act.document.uri.fsPath) === 'MyNewHeader.h',
      '4. 新增 C++ 標頭檔: made with an include guard and opened', nh ? path.basename(nh.file) : 'none');
    const np = await hub.cmdNewPage(null, { dir: tmp, name: 'Setup.ItTest', title: 'It Test', w: 320, h: 240 });
    await sleep(1500);
    const tab = vscode.window.tabGroups && vscode.window.tabGroups.activeTabGroup.activeTab;
    const vt = tab && tab.input && tab.input.viewType;
    const pText = np ? fs.readFileSync(np.file, 'utf8') : '';
    ok(!!np && /width:320px;height:240px/.test(pText) && /<title>It Test<\/title>/.test(pText) && /ht9045Designer\.editor/.test(String(vt || '')),
      '4. 新增 UI 頁面: an empty form of the size asked, its title, opened in the designer', 'tab ' + (tab ? tab.label + ' / ' + vt : 'none'));
    const pkg = JSON.parse(fs.readFileSync(path.join(__dirname, '..', '..', 'package.json'), 'utf8'));
    const menu = (pkg.contributes.menus['webview/context'] || []).filter(m => /ht9045Designer\.solution'/.test(m.when)).map(m => m.command);
    ok(menu.includes('ht9045Designer.solutionNewHeader') && menu.includes('ht9045Designer.solutionNewPage'), '4. both are on the Solution Explorer right-click menu');
  } catch (e) {
    fail++;
    out.push('FAIL  exception: ' + (e && e.stack || e));
  } finally {
    try { await vscode.commands.executeCommand('workbench.action.closeAllEditors'); } catch (e) { /* closing */ }
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { /* left in %TEMP% */ }
    out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
    if (report) fs.writeFileSync(report, out.join('\n'), 'utf8');
  }
};
