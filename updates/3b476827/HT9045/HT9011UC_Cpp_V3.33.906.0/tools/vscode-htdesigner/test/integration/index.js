'use strict';
// AI(W906-HTDESIGNER) 20260929: integration test inside a REAL VS Code window
// (--extensionTestsPath). Launched by test\vscode_it.ps1 with a throw-away
// user-data-dir, so the user's own VS Code settings and window are not touched.
// It checks what the Edge + fake-vscode layers cannot: the real webview
// (base href to vscode-resource URLs, the CSP, stack-frame URLs, the Big5
// virtual document, read-only-ness).
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const web2 = require('../../lib/websearch');

const sleep = ms => new Promise(r => setTimeout(r, ms));
async function waitFor(fn, ms) {
  const end = Date.now() + ms;
  for (;;) {
    let v = null;
    try { v = fn(); } catch (e) { v = null; }
    if (v) return v;
    if (Date.now() > end) return null;
    await sleep(100);
  }
}

exports.run = async function () {
  const out = [];
  let pass = 0, fail = 0;
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); };
  const report = process.env.HTD_IT_REPORT;
  const page = process.env.HTD_IT_PAGE;
  // every save this VS Code makes during the test: must be none (the launcher trusts this line
  // when another session changes a source file while the test runs)
  const saves = [];
  // (the throw-away user settings of this test instance, %TEMP%\htd_vscode_it_*\user, are not a project file)
  const saveSub = vscode.workspace.onDidSaveTextDocument(doc => {
    if (!/[\\/]htd_vscode_it_[^\\/]+[\\/]/i.test(doc.uri.fsPath)) saves.push(doc.uri.fsPath);
  });
  try {
    out.push('vscode ' + vscode.version + '  page ' + page);
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    ok(!!ext, 'extension found');
    const api = await ext.activate();
    const hub = api && api.hub;
    ok(!!hub, 'extension activated');

    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(page), 'ht9045Designer.editor');
    const d = await waitFor(() => (hub.active && hub.active.treeData.length ? hub.active : null), 30000);
    ok(!!d, 'designer opened; the probe ran inside the webview and sent the tree', d ? d.treeData.length + ' nodes' : 'timeout');
    if (!d) return;
    out.push('      roots: ' + JSON.stringify(d.r));

    const diag = await waitFor(() => d.diag, 8000);
    ok(diag && diag.images > 0 && diag.imagesLoaded === diag.images, 'page images load through <base href> (vscode-resource)',
      diag ? diag.imagesLoaded + '/' + diag.images + ' broken=' + diag.imagesBroken + ' sheets=' + diag.sheets + ' base=' + diag.base : 'no diag');
    ok(diag && diag.sheets >= 2, 'page stylesheets load', diag ? String(diag.sheets) : '');
    ok(diag && !diag.vscodeDefaultsLeft && /^0px/.test(diag.bodyPadding), 'VS Code default webview styles removed (body padding 0)', diag ? diag.bodyPadding : '');
    // the wall, checked from inside the real webview (closed port 9, never the machine)
    d.post({ type: 'netcheck' });
    const nc = await waitFor(() => d.netcheck, 8000);
    ok(nc && nc.ok, 'CSP blocks WebSocket / fetch / XHR from inside the page',
      nc ? 'violations=' + nc.violations.join(', ') + ' ws=' + nc.ws + ' fetch=' + nc.fetch + ' xhr=' + nc.xhr : 'no answer');
    out.push('      the page\'s own blocked attempts: ' + (d.blocked.length ? d.blocked.map(b => b.dir + ' ' + b.uri).join(', ') : '(none -- this page did not try to connect here)'));
    ok(d.errors.length === 0, 'no page JS errors', d.errors.map(e => e.msg + ' @' + path.basename(e.src) + ':' + e.line).join(' / '));

    // select the Save button as the HTML editor would
    d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
    const sel = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 10000);
    ok(!!sel, 'probe selected spbSave');
    if (!sel) return;
    const rawFrames = [];
    (sel.info.inherited || []).forEach(a => a.list.forEach(l => { if (l.frames[0]) rawFrames.push(l.type + ' ' + l.frames[0].url + ':' + l.frames[0].line); }));
    out.push('      raw frame URLs (first 3): ' + rawFrames.slice(0, 3).join(' | '));
    const data = await sel.promise;
    ok(!!data && !data.cppPending, 'selection resolved (C++ / BCB6 index built)');
    if (!data) return;
    const T = i => data.targets[i];
    const where = i => T(i).kind + ' ' + path.basename(T(i).file) + ':' + T(i).line;
    const click = data.events.find(e => e.name === 'OnClick');
    ok(click && click.handler === 'spbSaveClick', 'OnClick -> spbSaveClick');
    out.push('      OnClick targets: ' + (click ? click.targets.map(where).join(' | ') : '-'));
    ok(click && click.targets.some(i => T(i).kind === 'web' && /ht9045_hotplate_wire\.js$/i.test(T(i).file)),
      'web target found from REAL webview stack frames (vscode-resource URL -> file)');
    ok(click && click.targets.some(i => T(i).kind === 'port'), 'C++ port target');
    ok(click && click.targets.some(i => T(i).kind === 'golden' && T(i).line === 440), 'BCB6 target cHotPlate.cpp:440');
    const ci = data.events.indexOf(click);

    // properties view got it
    const pv = await waitFor(() => (hub.props.view && hub.props.data && hub.props.data.comp && hub.props.data.comp.htmlId === 'spbSave' ? hub.props.view : null), 5000);
    ok(!!pv, 'properties panel resolved and showing spbSave');
    // compare with DFM: the page's look (real webview) next to the .dfm values
    const ed0 = pv ? hub.props.data.edit : null;
    // Font.Height -16 is 14px on a generated page (|Height| - 2), and the page has 14px
    ok(ed0 && ed0.dfm && ed0.dfm.left === ed0.layout.left && ed0.dfm.fontSize === 14 && ed0.dfm.fontHeight === -16 && ed0.look && ed0.look.fontSize === 14 &&
      Array.isArray(ed0.look.fontFamilies) && ed0.look.fontFamilies.length >= 1,
      'panel data: DFM values beside the page values (Left same; Font.Size 14 = |Height -16| - 2 = the page)',
      ed0 ? 'dfm.fontSize=' + (ed0.dfm && ed0.dfm.fontSize) + ' page=' + (ed0.look && ed0.look.fontSize) + ' families=' + (ed0.look && (ed0.look.fontFamilies || []).join('|')) : '-');

    // BCB6: open (eventJump=golden set by the launcher), Big5, read-only
    await hub.openEvent(data, ci, d);
    const ged = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && e.document.uri.scheme === 'ht9045-golden' ? e : null; }, 5000);
    ok(!!ged && ged.selection.active.line === 439, 'BCB6 opens at cHotPlate.cpp:440', ged ? ged.document.uri.toString() + ' line ' + (ged.selection.active.line + 1) : 'not opened');
    if (ged) {
      const text = ged.document.getText();
      ok(/TfHotPlate::spbSaveClick/.test(text) && /[一-鿿]/.test(text), 'BCB6 text decoded from Big5');
      // what protects the Big5 original: the scheme is a read-only file system (the
      // editor is read-only, typing is refused) and every write through it fails.
      // (An extension API edit could still change the in-memory buffer -- VS Code
      // allows that on read-only editors -- but saving it goes through writeFile.)
      ok(vscode.workspace.fs.isWritableFileSystem('ht9045-golden') === false, 'BCB6 scheme is a read-only file system');
      let wrote = 'no error';
      try { await vscode.workspace.fs.writeFile(ged.document.uri, Buffer.from('x')); } catch (e) { wrote = String(e.code || e.name || e.message); }
      ok(wrote !== 'no error', 'BCB6 file system refuses writes', wrote);
      ok(ged.document.languageId === 'cpp', 'BCB6 document gets C++ highlighting', ged.document.languageId);
    }

    // web: direct jump
    await vscode.workspace.getConfiguration('ht9045Designer').update('eventJump', 'web', vscode.ConfigurationTarget.Global);
    await hub.openEvent(data, ci, d);
    const wed = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && /ht9045_hotplate_wire\.js$/i.test(e.document.uri.fsPath) ? e : null; }, 5000);
    ok(!!wed, 'eventJump=web opens the wire script', wed ? path.basename(wed.document.uri.fsPath) + ':' + (wed.selection.active.line + 1) : 'not opened');
    ok(!!wed && wed.viewColumn !== d.panel.viewColumn, 'code opens beside the designer, not over it', wed ? 'designer=' + d.panel.viewColumn + ' code=' + wed.viewColumn : '');

    // DFM definition: .dfm opens read-only (Big5) at "object spbSave: TSpeedButton"
    await vscode.commands.executeCommand('ht9045Designer.revealDfm');
    const ded = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && /cHotPlate\.dfm$/i.test(e.document.uri.path) ? e : null; }, 5000);
    ok(!!ded && ded.document.uri.scheme === 'ht9045-golden' && /object spbSave: TSpeedButton/.test(ded.document.lineAt(ded.selection.active.line).text),
      'revealDfm opens cHotPlate.dfm at "object spbSave: TSpeedButton"', ded ? 'line ' + (ded.selection.active.line + 1) : 'not opened');
    ok(!!data.cmds.find(c => c.cmd === 'recipe.doc.put' && c.dispatch.length), 'command recipe.doc.put traced to its C++ dispatch',
      data.cmds.map(c => c.cmd + '(' + c.dispatch.length + ')').join(' '));

    // 事件表 (the Object Inspector's Events tab): every TSpeedButton event; OnClick set; a double-click on an empty
    // one adds the handler to the C++ port (forms/fHotPlate.h / .cpp) -- in memory, put back, never saved
    const eg = data.eventGrid;
    const egNames = eg ? eg.rows.map(r => r.name).join(',') : '';
    const egClick = eg && eg.rows.find(r => r.name === 'OnClick');
    const egDbl = eg && eg.rows.find(r => r.name === 'OnDblClick');
    let egMade = null, egH = '', egC = '', egBack = false, egWire = {}, egRow2 = null, egListen = false;
    const egPort = d.r.portRoot;
    const egSrvF = path.join(egPort, 'tools', 'wb_serve.cpp'), egCmF = path.join(egPort, 'CMakeLists.txt'), egGenF = path.join(egPort, 'HtdEvents', 'HtdEvents.gen.cpp');
    const egGenBefore = fs.existsSync(egGenF);
    if (egDbl && !egDbl.handler) {
      const egR0 = d.readyCount || 0;
      egMade = await hub.onEventGrid(data, 'OnDblClick', d);
      if (egMade) {
        const find = f => vscode.workspace.textDocuments.find(x => x.uri.fsPath.toLowerCase() === f.toLowerCase());
        const hd = find(egMade.h), cd = find(egMade.cpp), sd = find(egSrvF), md = find(egCmF), gd = find(egGenF);
        egH = hd ? hd.getText() : '';
        egC = cd ? cd.getText() : '';
        egWire = { wired: egMade.wired, notes: egMade.notes, page: /htdCpp\('spbSave', 'dblclick', 'TfHotPlate', 'spbSaveDblClick', 'OnDblClick'\);/.test(d.doc.getText()),
          server: !!(sd && sd.isDirty && /\.cmd == "htd\.event"/.test(sd.getText())), cmake: !!(md && md.isDirty && /HtdEvents\/HtdEvents\.gen\.cpp\)/.test(md.getText())),
          gen: !!(gd && gd.uri.scheme === 'untitled' && /htd-row TfHotPlate spbSaveDblClick spbSave OnDblClick/.test(gd.getText())), genOnDisk: fs.existsSync(egGenF) };
        // the redrawn page binds it (the helper's listener), and the 事件表 shows the one function from the NOT-saved documents
        await waitFor(() => ((d.readyCount || 0) > egR0 ? true : null), 10000);
        await sleep(400);
        const sb = d.sel;
        d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
        const s2 = await waitFor(() => (d.sel && d.sel !== sb && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 10000);
        const dd2 = s2 ? await s2.promise : null;
        egRow2 = dd2 && dd2.eventGrid ? dd2.eventGrid.rows.find(r => r.name === 'OnDblClick') : null;
        if (egRow2) egRow2 = { via: egRow2.via, mainName: egRow2.mainName, main: egRow2.main != null && dd2.targets[egRow2.main] ? path.basename(dd2.targets[egRow2.main].file) : null,
          server: egRow2.server != null && dd2.targets[egRow2.server] ? path.basename(dd2.targets[egRow2.server].file) : null, web: egRow2.web != null };
        egListen = !!(dd2 && dd2.listeners.filter(l => l.type === 'dblclick').length > data.listeners.filter(l => l.type === 'dblclick').length);
        // put everything back (nothing was saved): the documents reverted, the untitled one closed without saving
        for (const doc of [hd, cd, sd, md, d.doc]) {
          if (!doc || !doc.isDirty) continue;
          await vscode.window.showTextDocument(doc, { preview: false });
          await vscode.commands.executeCommand('workbench.action.files.revert');
        }
        if (gd) { await vscode.window.showTextDocument(gd, { preview: false }); await vscode.commands.executeCommand('workbench.action.revertAndCloseActiveEditor'); }
        await waitFor(() => (![hd, cd, sd, md, d.doc].some(x => x && x.isDirty) ? true : null), 5000);
        egBack = !!(hd && cd && !hd.isDirty && !cd.isDirty && hd.getText() === fs.readFileSync(egMade.h, 'utf8') && cd.getText() === fs.readFileSync(egMade.cpp, 'utf8') &&
          (!sd || (!sd.isDirty && sd.getText() === fs.readFileSync(egSrvF, 'utf8'))) && (!md || (!md.isDirty && md.getText() === fs.readFileSync(egCmF, 'utf8'))) &&
          !d.doc.isDirty && fs.existsSync(egGenF) === egGenBefore);
      }
    }
    ok(!!(eg && eg.vcl === 'TSpeedButton' && egNames === 'OnClick,OnDblClick,OnMouseDown,OnMouseMove,OnMouseUp' && egClick && egClick.handler === 'spbSaveClick' &&
      egClick.idx >= 0 && egMade && /fHotPlate\.h$/i.test(egMade.h) && /fHotPlate\.cpp$/i.test(egMade.cpp) &&
      /\n\s*void spbSaveDblClick\(TObject \*Sender\);\s+\/\/AI\(W906-HTDESIGNER\)/.test(egH) && /void TfHotPlate::spbSaveDblClick\(TObject \*Sender\)\r?\n\{/.test(egC) && egBack),
      '事件表: every TSpeedButton event, OnClick = spbSaveClick; a double-click on the empty OnDblClick adds TfHotPlate::spbSaveDblClick to fHotPlate.h / .cpp (put back, not saved)',
      egNames + ' | made ' + (egMade ? path.basename(egMade.h) + '+' + path.basename(egMade.cpp) : '-') + ' | back ' + egBack);
    // 自動接線 (EastSun 20260930: "如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增")
    ok(!!(egWire.wired && egWire.page && egWire.server && egWire.cmake && egWire.gen && !egWire.genOnDisk && egListen &&
      egRow2 && egRow2.via === 'htd' && egRow2.mainName === 'spbSaveDblClick' && /fHotPlate\.cpp$/i.test(egRow2.main || '') && /wb_serve\.cpp$/i.test(egRow2.server || '') && egRow2.web && egBack),
      '事件表: ... and wired in the same edit: the page\'s htdCpp line (the redrawn page binds dblclick), wb_serve.cpp\'s htd.event branch, CMakeLists.txt, HtdEvents.gen.cpp (untitled, not on disk); ' +
      'the row = spbSaveDblClick from the not-saved documents; all put back', JSON.stringify({ wire: egWire, row: egRow2, listen: egListen, back: egBack }));

    // 網頁事件: a typed function name -> the page's htdEvents block; the redrawn page really binds it (the probe sees
    // the listener); put back with a revert
    const jsR0 = d.readyCount || 0;
    const jsMade = await hub.cmdJsEvent(d, data, 'mouseup', 'spbSaveUpJs');
    await waitFor(() => ((d.readyCount || 0) > jsR0 ? true : null), 10000);
    await sleep(400);
    // (a selection made AFTER the redraw -- the one before it has the old listeners)
    const jsSelBefore = d.sel;
    d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
    const jsSel = await waitFor(() => (d.sel && d.sel !== jsSelBefore && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 10000);
    const jsData = jsSel ? await jsSel.promise : null;
    const jsBound = !!(jsData && jsData.listeners.some(l => l.type === 'mouseup' && l.fnName === 'spbSaveUpJs'));
    const jsText = d.doc.getText();
    await vscode.window.showTextDocument(d.doc, { preview: false });
    await vscode.commands.executeCommand('workbench.action.files.revert');
    const jsBack = !d.doc.isDirty && d.doc.getText() === fs.readFileSync(d.file, 'utf8');
    ok(!!(jsMade && jsMade.added === 'spbSaveUpJs' && /<script id="htdEvents">[\s\S]*addEventListener\('mouseup', spbSaveUpJs\)/.test(jsText) && jsBound && jsBack),
      '網頁事件: spbSaveUpJs written into the page (htdEvents block), the redrawn page binds it (the probe records mouseup -> spbSaveUpJs); reverted',
      JSON.stringify({ made: jsMade && jsMade.added, bound: jsBound, back: jsBack }));
    d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
    await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 5000);
    ok(data.uses && data.uses.golden.length > 0, 'uses of spbSave found in BCB6', data.uses ? data.uses.golden.length + ' golden, ' + data.uses.port.length + ' port' : '-');

    // code -> designer: CodeLens on the BCB6 handler, then clicking it
    const gUri = vscode.Uri.file(path.join(d.r.goldenRoot, 'cHotPlate.cpp')).with({ scheme: 'ht9045-golden' });
    d.post({ type: 'selectId', id: 'XST1', origin: 'editor' });           // move the selection away first
    await waitFor(() => d.sel && d.sel.info && d.sel.info.id === 'XST1', 5000);
    let lenses = [];
    for (let i = 0; i < 40; i++) {
      lenses = (await vscode.commands.executeCommand('vscode.executeCodeLensProvider', gUri)) || [];
      if (lenses.some(l => l.command && l.command.command === 'ht9045Designer.showInDesigner')) break;
      await sleep(250);
    }
    const lens = lenses.find(l => l.command && /spbSave\.OnClick/.test(l.command.title));
    ok(!!lens && lens.range.start.line === 439, 'CodeLens above BCB6 TfHotPlate::spbSaveClick', lens ? lens.command.title : lenses.length + ' lenses');
    if (lens) {
      await vscode.commands.executeCommand(lens.command.command, ...(lens.command.arguments || []));
      const back = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 8000);
      ok(!!back, 'clicking it selects spbSave in the designer');
    }

    // C++ -> web: lens above the server's dispatch shows where the web sends it
    const wsUri = vscode.Uri.file(path.join(d.r.portRoot, 'tools', 'wb_serve.cpp'));
    await vscode.workspace.openTextDocument(wsUri);   // executeCodeLensProvider needs a loaded model
    let wl = [];
    for (let i = 0; i < 40; i++) {
      wl = (await vscode.commands.executeCommand('vscode.executeCodeLensProvider', wsUri)) || [];
      if (wl.some(l => l.command && l.command.command === 'ht9045Designer.openWebSenders')) break;
      await sleep(250);
    }
    const ioLens = wl.find(l => l.command && l.command.command === 'ht9045Designer.openWebSenders' && l.command.arguments[0] === 'io.btnPanelClick');
    ok(!!ioLens && /HW\.IoSetView\.html/.test(ioLens.command.title), 'wb_serve.cpp: CodeLens "網頁送出 io.btnPanelClick"',
      ioLens ? ioLens.command.title + ' @' + (ioLens.range.start.line + 1) : wl.length + ' lenses');

    // HTML source reveal
    await hub.revealSource(d, 'spbSave', true);
    const hed = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && e.document === d.doc ? e : null; }, 5000);
    ok(!!hed && /id="spbSave"/.test(hed.document.getText(hed.selection)), 'HTML 原始碼 selects id="spbSave"', hed ? JSON.stringify(hed.document.getText(hed.selection)) : '');

    // wiring overview with the REAL probe's listener list
    d.panel.reveal(d.panel.viewColumn, false);
    await waitFor(() => hub.active === d, 3000);
    const ov = await vscode.commands.executeCommand('ht9045Designer.pageOverview');
    ok(ov && ov.listenersOk && ov.summary.events === 19, 'page overview (real probe): HotPlate 19 events',
      ov ? 'web ' + JSON.stringify(ov.summary.web) + ' port ' + JSON.stringify(ov.summary.port) : 'none');
    const exitEv = ov && ov.rows.find(r => r.id === 'sbtExit');
    ok(exitEv && exitEv.events[0].web === 'yes', 'overview: sbtExit.OnClick has a real web listener', exitEv ? exitEv.events[0].web : '-');

    // WPF-style editing in the real designer: edit -> dirty -> undo -> clean. Never saved.
    const original = d.doc.getText();
    d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
    const s3 = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? d.sel : null), 5000);
    ok(!!(s3 && s3.info.layout && s3.info.layout.target === 'self' && s3.info.caption && s3.info.caption.value === 'Save'),
      'real probe: spbSave layout + caption are editable', s3 && s3.info.layout ? JSON.stringify(s3.info.layout.raw) : '-');
    if (s3) {
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.post({ type: 'setLayout', key: s3.info.key, left: 60 });
      const moved = await waitFor(() => (d.doc.isDirty && /id="spbSave"[^>]*left:60px;/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!moved, 'setLayout Left=60 -> the source says left:60px, the page is dirty (not saved)',
        moved ? '' : 'dirty=' + d.doc.isDirty + ' tag=' + ((/<button[^>]*id="spbSave"[^>]*>/.exec(d.doc.getText()) || [''])[0]) +
          ' visible=' + d.panel.visible + ' log: ' + (hub.logLines || []).slice(-4).join(' / '));
      d.post({ type: 'setCaption', key: s3.info.key, value: 'Save It' });
      const capd = await waitFor(() => (/>Save It<\/button>/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!capd, 'setCaption -> <button id="spbSave"> text in the source');
      // Ctrl+Z, twice, with the designer active
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      const undone = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
      ok(!!undone, 'undo twice (Ctrl+Z) restores the source exactly', undone ? '' : 'isDirty=' + d.doc.isDirty);
      if (!undone || d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      ok(!d.doc.isDirty && d.doc.getText() === original, 'page clean again, nothing saved');
      // the undo re-rendered the page from the source: the tree comes back
      const back = await waitFor(() => (d.treeData.length ? true : null), 8000);
      ok(!!back, 'the page re-renders after undo');

      // multi-select + align top = ONE undo step
      await sleep(800);
      d.post({ type: 'selectIds', ids: ['sbtExit', 'spbSave'], origin: 'tree' });   // as the outline sends it
      const ms = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'sbtExit' && d.sel.info.multi ? d.sel : null), 5000);
      ok(!!ms && ms.info.multi.indexOf('spbSave') >= 0, 'multi-select sbtExit + spbSave (from the tree)', ms ? JSON.stringify(ms.info.multi) : '-');
      d.post({ type: 'align', how: 'height' });
      const al = await waitFor(() => (d.doc.isDirty && d.doc.getText() !== original ? true : null), 5000);
      ok(!!al, 'align (same height) changed the source');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      const un2 = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
      ok(!!un2, 'ONE undo restores the whole alignment');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      ok(!d.doc.isDirty && d.doc.getText() === original, 'clean again, nothing saved');
      await waitFor(() => (d.treeData.length ? true : null), 8000);

      // 容器中水平置中 by the real command: the pair moves as one block (their distance kept), ONE undo
      await sleep(800);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.post({ type: 'selectIds', ids: ['sbtExit', 'spbSave'], origin: 'tree' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'sbtExit' && d.sel.info.multi ? true : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.align.hcenterIn');
      const leftOf = (t, id) => { const m = new RegExp('id="' + id + '"[^>]*?left:(-?\\d+)px').exec(t); return m ? +m[1] : null; };
      const cz = await waitFor(() => (d.doc.isDirty && d.doc.getText() !== original ? d.doc.getText() : null), 5000);
      const xl = cz ? leftOf(cz, 'sbtExit') : null, sl = cz ? leftOf(cz, 'spbSave') : null;
      ok(!!cz && xl !== leftOf(original, 'sbtExit') && sl !== leftOf(original, 'spbSave') &&
        xl - sl === leftOf(original, 'sbtExit') - leftOf(original, 'spbSave') && Math.abs(sl - 54 + 10) <= 3,   // block 54..573 in 606 px: about 10 px left
        '容器中水平置中: sbtExit + spbSave centred in Panel2 as one block', 'spbSave ' + sl + ' sbtExit ' + xl);
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo takes the centring back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);

      // 與 DFM 的差異 (real probe, whole page): reset spbSave Font.Bold -> normal, the list compares again, ONE undo
      await sleep(800);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      const df = await vscode.commands.executeCommand('ht9045Designer.dfmDiff');
      const dfs = df && df.rows.find(r => r.id === 'spbSave' && r.prop === 'Font.Bold');
      ok(df && df.controls >= 20 && dfs && dfs.page === 'True' && dfs.dfm === 'False' &&
        !df.rows.some(r => r.id === 'spbSave' && /^(Left|Top|Width|Height|Caption|Font\.Size)$/.test(r.prop)),
        'DFM difference list (real probe): spbSave Font.Bold True/False; position, caption, Font.Size (14 = |-16|-2) equal the DFM',
        df ? df.controls + ' controls, ' + df.compared + ' compared, ' + df.rows.length + ' differ ' + JSON.stringify(df.byGroup) : 'none');
      if (df) df.rows.filter(r => r.group === 'layout').slice(0, 20).forEach(r => out.push('        pos ' + r.id + '.' + r.prop + ' page ' + r.page + ' dfm ' + r.dfm + (r.note ? ' *' : '')));
      // HotPlate's only size differences were its 8 AutoSize labels ("width:136px; … width:auto")
      ok(df && df.byGroup.layout === 0, 'AutoSize labels (width:auto in the source) are not compared: HotPlate has no position/size difference',
        df ? JSON.stringify(df.byGroup) : '-');
      if (dfs && d.dfmDiff) {
        d.dfmDiff.onMessage({ type: 'reset', id: 'spbSave', reset: dfs.reset });
        const rs = await waitFor(() => (d.doc.isDirty && /id="spbSave"[^>]*font-weight:normal/.test(d.doc.getText()) ? true : null), 5000);
        ok(!!rs, 'reset from the list -> the source says font-weight:normal on spbSave (not saved)');
        const re = await waitFor(() => (d.dfmDiff && d.dfmDiff.data && !d.dfmDiff.data.rows.some(r => r.id === 'spbSave' && r.prop === 'Font.Bold') ? true : null), 5000);
        ok(!!re, 'the list compares again after the edit: spbSave Font.Bold is gone');
        d.panel.reveal(d.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        const un3 = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
        ok(!!un3, 'ONE undo restores the reset');
        if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        ok(!d.doc.isDirty && d.doc.getText() === original, 'clean again, nothing saved');
        await waitFor(() => (d.treeData.length ? true : null), 8000);
        const back3 = await waitFor(() => (d.dfmDiff && d.dfmDiff.data && d.dfmDiff.data.rows.some(r => r.id === 'spbSave' && r.prop === 'Font.Bold') ? true : null), 8000);
        ok(!!back3, 'after the undo re-draw the list shows spbSave Font.Bold again');
      }
    }

    // right-click menu commands (the menu itself cannot be opened from a test)
    d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
    await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
    await vscode.commands.executeCommand('ht9045Designer.selectParent');
    const par = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'Panel2' ? true : null), 5000);
    ok(!!par, 'menu: 選取上層 -> Panel2');
    const cmds = await vscode.commands.getCommands(true);
    ok(['viewCode', 'selectParent', 'align.left', 'align.height', 'align.size', 'align.hspace', 'align.vspace', 'align.hcenterIn', 'align.vcenterIn', 'help', 'resetZoom'].every(c => cmds.includes('ht9045Designer.' + c)),
      'menu / palette commands registered');

    // mode toggle reaches the probe (no error)
    await vscode.commands.executeCommand('ht9045Designer.toggleMode');
    ok(d.mode === 'operate', 'toggle -> operate mode');
    await vscode.commands.executeCommand('ht9045Designer.toggleMode');
    ok(d.mode === 'design', 'toggle -> design mode');

    // the page file itself was not modified
    ok(!d.doc.isDirty, 'page document not modified');

    // hidden in the designer only (the eye): the real probe hides Panel2, it stays hidden after the page
    // is drawn again (Reload), "show all" brings it back -- and the page is never dirty
    {
      const diagNow = async () => { d.diag = null; d.post({ type: 'diag' }); return waitFor(() => d.diag, 5000); };
      await vscode.commands.executeCommand('ht9045Designer.designHide', { id: 'Panel2', isForm: false });
      const g1 = await diagNow();
      ok(g1 && g1.designHidden === 1 && !d.doc.isDirty, 'eye: Panel2 hidden in the real designer, page not dirty', JSON.stringify(g1 && g1.designHidden));
      await vscode.commands.executeCommand('ht9045Designer.reload');
      await sleep(1500);
      await waitFor(() => (d.treeData.length ? true : null), 10000);
      const g2 = await diagNow();
      ok(g2 && g2.designHidden === 1, 'still hidden after the page is drawn again (Reload)', JSON.stringify(g2 && g2.designHidden));
      await vscode.commands.executeCommand('ht9045Designer.designUnhideAll');
      const g3 = await diagNow();
      ok(g3 && g3.designHidden === 0 && !d.doc.isDirty && hub.loadDesignHidden(d.file).length === 0, 'show all: back, nothing kept, page not dirty');
      // the lock: Panel2 locked -> deleting spbSave (inside it) is refused even when confirmed; unlock all
      await vscode.commands.executeCommand('ht9045Designer.designLock', { id: 'Panel2', isForm: false });
      const g4 = await diagNow();
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const lkBefore = d.doc.getText();
      const lkDel = await vscode.commands.executeCommand('ht9045Designer.deleteComponent', { confirmed: true });
      ok(g4 && g4.designLocked === 1 && lkDel === null && d.doc.getText() === lkBefore && !d.doc.isDirty,
        'lock Panel2 (real probe knows it): deleting spbSave inside it is refused, page untouched', JSON.stringify(g4 && g4.designLocked));
      await vscode.commands.executeCommand('ht9045Designer.designUnlockAll');
      const g5 = await diagNow();
      ok(g5 && g5.designLocked === 0 && hub.loadDesignSet('htd.designLocked', d.file).length === 0, 'unlock all: nothing locked, nothing kept');
      // 接線標示 with the real probe's listeners: sbtExit green; off again
      const wm = await vscode.commands.executeCommand('ht9045Designer.toggleWireMarks', true);
      const gw = await diagNow();
      await vscode.commands.executeCommand('ht9045Designer.toggleWireMarks', false);
      const gw2 = await diagNow();
      ok(wm && wm.on && wm.marks && wm.marks.sbtExit === 'ok' && gw && gw.wireMarks > 0 && gw2 && gw2.wireMarks === 0,
        'wiring marks (real probe): sbtExit green, ' + (wm ? JSON.stringify(wm.count) : '-') + '; off again');
      // every component's name on the real design surface
      await vscode.commands.executeCommand('ht9045Designer.toggleNames', true);
      const gn = await diagNow();
      await vscode.commands.executeCommand('ht9045Designer.toggleNames', false);
      const gn2 = await diagNow();
      ok(gn && gn.names >= 10 && gn2 && gn2.names === 0, 'name tags (real probe): ' + (gn && gn.names) + ' shown; off again');
      // snap to gridlines: the real probe gets it (and again after a Reload), off again
      const gon = await vscode.commands.executeCommand('ht9045Designer.toggleGrid', true);
      const g6 = await diagNow();
      await vscode.commands.executeCommand('ht9045Designer.reload');
      await sleep(1500);
      await waitFor(() => (d.treeData.length ? true : null), 10000);
      const g7 = await diagNow();
      await vscode.commands.executeCommand('ht9045Designer.toggleGrid', false);
      const g8 = await diagNow();
      ok(gon && gon.on && g6 && g6.grid === 8 && g7 && g7.grid === 8 && g8 && g8.grid === 0 && !d.doc.isDirty,
        'grid on: the real probe snaps at 8 px (still after Reload); off again; page not dirty', [g6, g7, g8].map(g => g && g.grid).join(','));
    }

    // WPF Copy / Paste / Delete in the real designer: the page is drawn again from the source each time
    {
      const original = d.doc.getText();
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const cp = await vscode.commands.executeCommand('ht9045Designer.copyComponent');
      ok(cp && cp.ids[0] === 'spbSave', 'copy spbSave (real designer)');
      const ps = await vscode.commands.executeCommand('ht9045Designer.pasteComponent');
      const drawn = await waitFor(() => (/id="spbSave_2"/.test(d.doc.getText()) && d.treeData.some(r => r[2] === 'spbSave_2') ? true : null), 10000);
      ok(ps && ps.made[0] === 'spbSave_2' && !!drawn, 'paste: spbSave_2 in the source AND drawn on the page (the tree has it)');
      const selNew = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave_2' ? true : null), 8000);
      ok(!!selNew, 'the pasted button is selected after the re-draw');
      const del = await vscode.commands.executeCommand('ht9045Designer.deleteComponent');
      const gone = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
      ok(del && del.removed[0] === 'spbSave_2' && !!gone, 'delete spbSave_2 (no script uses it): the source is byte-identical to before the paste');
      await waitFor(() => (d.treeData.length && !d.treeData.some(r => r[2] === 'spbSave_2') ? true : null), 10000);
      // a component the page's JS uses: removed only when confirmed (here: the argument), Ctrl+Z brings it back
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const del2 = await vscode.commands.executeCommand('ht9045Designer.deleteComponent', { confirmed: true });
      const gone2 = await waitFor(() => (!/id="spbSave"/.test(d.doc.getText()) ? true : null), 5000);
      ok(del2 && del2.uses.indexOf('spbSave') >= 0 && !!gone2, 'delete spbSave (the wire script uses it; confirmed): gone from the source', del2 ? JSON.stringify(del2) : '-');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      const back = await waitFor(() => (/id="spbSave"/.test(d.doc.getText()) && !/id="spbSave_2"/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!back, 'Ctrl+Z brings spbSave back');
      if (d.doc.getText() !== original || d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      ok(d.doc.getText() === original && !d.doc.isDirty, 'clean again, nothing saved');
      await waitFor(() => (d.treeData.some(r => r[2] === 'spbSave') ? true : null), 10000);

      // WPF Order (Ctrl+Shift+]): spbSave to the front -- the re-drawn tree lists it after sbtExit; Ctrl+Z
      await sleep(800);
      const idxOf = id => d.treeData.findIndex(r => r[2] === id);
      const was = idxOf('spbSave') < idxOf('sbtExit');
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const of = await vscode.commands.executeCommand('ht9045Designer.order.front');
      const redrawn = await waitFor(() => (d.doc.getText() !== original && idxOf('spbSave') > idxOf('sbtExit') && idxOf('sbtExit') >= 0 ? true : null), 10000);
      ok(was && of && !!redrawn, 'order front: spbSave moved after sbtExit, the re-drawn component tree follows', of ? JSON.stringify(of) : '-');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      const unOrd = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
      ok(!!unOrd, 'ONE undo puts it back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (idxOf('spbSave') >= 0 && idxOf('spbSave') < idxOf('sbtExit') ? true : null), 10000);

      // document outline drag and drop (the real DataTransfer): spbSave onto GroupBox1 -> the re-drawn
      // tree has it under GroupBox1; one undo puts it back
      await sleep(800);
      {
        const nodeOf = id => Array.from(hub.tree.byKey.values()).find(n => n.id === id);
        const dt = new vscode.DataTransfer();
        hub.treeDnd.handleDrag([nodeOf('spbSave')], dt);
        const mv = await hub.treeDnd.handleDrop(nodeOf('GroupBox1'), dt);
        const keyOfId = id => { const r = d.treeData.find(x => x[2] === id); return r ? r[0] : null; };
        const under = await waitFor(() => { const r = d.treeData.find(x => x[2] === 'spbSave'); return r && r[1] === keyOfId('GroupBox1') ? true : null; }, 10000);
        ok(mv && mv.into === 'GroupBox1' && !!under, 'tree drag: spbSave into GroupBox1 -- the re-drawn tree has it there', mv ? JSON.stringify(mv) : '-');
        d.panel.reveal(d.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        const back = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
        ok(!!back, 'ONE undo puts it back in Panel2');
        if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        await waitFor(() => { const r = d.treeData.find(x => x[2] === 'spbSave'); return r && r[1] === keyOfId('Panel2') ? true : null; }, 10000);
      }

      // Group Into a new panel (the real probe measures) / Ungroup Panel2 -- each undone again
      await sleep(800);
      {
        const keyOfId = id => { const r = d.treeData.find(x => x[2] === id); return r ? r[0] : null; };
        const parentIdOf = id => { const r = d.treeData.find(x => x[2] === id); const p = r ? d.treeData.find(x => x[0] === r[1]) : null; return p ? (p[7] ? '@form' : p[2]) : null; };
        const p2Parent = parentIdOf('Panel2');
        d.post({ type: 'selectIds', ids: ['spbSave', 'sbtExit'], origin: 'tree' });
        await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && d.sel.info.multi && d.sel.info.multi.indexOf('sbtExit') >= 0 ? true : null), 5000);
        const gp = await vscode.commands.executeCommand('ht9045Designer.groupIntoPanel');
        const inNew = gp ? await waitFor(() => (parentIdOf('spbSave') === gp.panel && parentIdOf('sbtExit') === gp.panel && parentIdOf(gp.panel) === 'Panel2' ? true : null), 10000) : null;
        ok(gp && !!inNew && gp.box.width > 0 && gp.box.height > 0, 'group: a new panel in Panel2 holds spbSave + sbtExit (the re-drawn tree)', gp ? JSON.stringify(gp) : '-');
        d.panel.reveal(d.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo takes the new panel away');
        if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        await waitFor(() => (parentIdOf('spbSave') === 'Panel2' ? true : null), 10000);
        d.post({ type: 'selectId', id: 'Panel2', origin: 'editor' });
        await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'Panel2' ? true : null), 5000);
        const ug = await vscode.commands.executeCommand('ht9045Designer.ungroup');
        const outOk = ug ? await waitFor(() => (!keyOfId('Panel2') && parentIdOf('spbSave') === p2Parent && parentIdOf('sbtExit') === p2Parent ? true : null), 10000) : null;
        ok(ug && ug.removed === 'Panel2' && !!outOk, 'ungroup Panel2: its buttons now in Panel2\'s parent, Panel2 gone', ug ? JSON.stringify(ug) : '-');
        d.panel.reveal(d.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo brings Panel2 back');
        if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        await waitFor(() => (parentIdOf('spbSave') === 'Panel2' ? true : null), 10000);
      }

      // 工具箱: a new TLabel into Panel2 -- drawn, inside Panel2 in the tree, selected; Ctrl+Z removes it
      await sleep(800);
      d.post({ type: 'selectId', id: 'Panel2', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'Panel2' ? true : null), 5000);
      const tb = await vscode.commands.executeCommand('ht9045Designer.toolboxAdd', 'TLabel');
      const p2Key = () => { const r = d.treeData.find(x => x[2] === 'Panel2'); return r ? r[0] : null; };
      const tbDrawn = tb ? await waitFor(() => { const r = d.treeData.find(x => x[2] === tb.id); return r && r[1] === p2Key() ? true : null; }, 10000) : null;
      const tbSel = tb ? await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === tb.id ? true : null), 8000) : null;
      ok(tb && tb.into === 'Panel2' && !!tbDrawn && !!tbSel, 'toolbox: a new label drawn inside Panel2 (the tree says so) and selected', tb ? JSON.stringify(tb) : '-');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      const tbGone = await waitFor(() => (d.doc.getText() === original ? true : null), 5000);
      ok(!!tbGone, 'ONE undo removes it');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length && (!tb || !d.treeData.some(x => x[2] === tb.id)) ? true : null), 10000);

      // WinForms / WPF placing: a toolbox click hands the real probe the tool (crosshair); put down in Panel2
      // at 400,10 with 60x24 -> drawn inside Panel2; ONE undo
      hub.lastArm = null;
      // (the toolbox showing: once the tool is put down its selection is Pointer again, WPF)
      try { await vscode.commands.executeCommand('ht9045Designer.toolbox.focus'); } catch (e) { /* no view */ }
      await waitFor(() => (hub.toolboxView && hub.toolboxView.visible ? true : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.toolboxArm', 'TSpeedButton');
      d.diag = null; d.post({ type: 'diag' });
      const pgA = await waitFor(() => d.diag, 5000);
      d.post({ type: 'placeArm', cls: null });
      const tp = await hub.cmdPlace(d, { cls: 'TSpeedButton', w: 60, h: 24, targets: [{ id: 'Panel2', x: 400, y: 10 }, { id: '@form', x: 400, y: 485 }] });
      const tpDrawn = tp ? await waitFor(() => { const r = d.treeData.find(x => x[2] === tp.id); return r && r[1] === p2Key() ? true : null; }, 10000) : null;
      ok(!!(pgA && pgA.placing === 'TSpeedButton' && tp && tp.into === 'Panel2' && tpDrawn && new RegExp('id="' + tp.id + '"[^>]*left:400px;top:10px;width:60px;height:24px;').test(d.doc.getText())),
        'toolbox placing (real): the probe holds the tool; put down into Panel2 at 400,10, 60x24, drawn there', tp ? JSON.stringify(tp) : '-');
      const tpPtr = await waitFor(() => { const s = hub.toolboxView && hub.toolboxView.selection; return s && s[0] && s[0].cls === '@pointer' ? true : null; }, 5000);
      ok(!!tpPtr && !d.armed, 'toolbox (real): after the tool is put down its selection is Pointer again (WPF), nothing picked up',
        'visible=' + !!(hub.toolboxView && hub.toolboxView.visible) + ' sel=' + JSON.stringify(((hub.toolboxView && hub.toolboxView.selection) || []).map(x => x.cls)));
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo removes the placed one');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length && (!tp || !d.treeData.some(x => x[2] === tp.id)) ? true : null), 10000);

      // WinForms / WPF Ctrl+drag (the probe's copyDrop): a copy of spbSave 40 px to the right, drawn in Panel2 and
      // selected, the original where it was; ONE undo
      const cdHe = require(path.join(__dirname, '..', '..', 'lib', 'htmledit.js'));
      const cdLeft = id => { const b = cdHe.startTagOf(d.doc.getText(), id); const mm = b ? /left:(-?\d+)px/.exec(b.text) : null; return mm ? +mm[1] : null; };
      const cdL0 = cdLeft('spbSave');
      const cdr = await hub.cmdCopyDrop(d, { ids: ['spbSave'], dx: 40, dy: 0 });
      const cdId = cdr && cdr.made[0];
      const cdDrawn = cdId ? await waitFor(() => { const r = d.treeData.find(x => x[2] === cdId); return r && r[1] === p2Key() ? true : null; }, 10000) : null;
      const cdSel = cdId ? await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === cdId ? true : null), 8000) : null;
      ok(!!(cdId && cdDrawn && cdSel && cdL0 !== null && cdLeft(cdId) === cdL0 + 40 && cdLeft('spbSave') === cdL0),
        'Ctrl+drag copy (real): ' + cdId + ' 40 px right of spbSave, drawn in Panel2 and selected; spbSave where it was', cdr ? JSON.stringify(cdr) : '-');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo removes the copy');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length && (!cdId || !d.treeData.some(x => x[2] === cdId)) ? true : null), 10000);

      // WPF Document Outline Ctrl+H / Shift+Ctrl+H (the command the keys run): spbSave hidden on the surface, shown again
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const dkh = await vscode.commands.executeCommand('ht9045Designer.designKey', { cmd: 'hide' });
      const dkHidden = d.designHidden.has('spbSave');
      const dks = await vscode.commands.executeCommand('ht9045Designer.designKey', { cmd: 'show' });
      ok(!!(dkh && dkHidden && dks && !d.designHidden.has('spbSave') && d.doc.getText() === original),
        'Ctrl+H / Shift+Ctrl+H (real): spbSave hidden in the designer, shown again; the source untouched', JSON.stringify({ dkh, dks }));
      // WPF Alt+arrow (Duplicate), the command Shift+Alt+Right runs: a copy 10 px right, drawn in Panel2; ONE undo
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const dpr = await vscode.commands.executeCommand('ht9045Designer.duplicate', { dx: 10, dy: 0 });
      const dpId = dpr && dpr.made[0];
      const dpDrawn = dpId ? await waitFor(() => { const r = d.treeData.find(x => x[2] === dpId); return r && r[1] === p2Key() ? true : null; }, 10000) : null;
      ok(!!(dpId && dpDrawn && cdLeft(dpId) === cdL0 + 10), 'Shift+Alt+Right (real): ' + dpId + ' 10 px right of spbSave, drawn in Panel2', dpr ? JSON.stringify(dpr) : '-');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo removes the duplicate');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length && (!dpId || !d.treeData.some(x => x[2] === dpId)) ? true : null), 10000);
      // WPF F2 (Edit control text), the command F2 runs: the real page opens its text box on spbSave (the designer's keys
      // stay out while it is open); closed without writing -> the source untouched
      d.panel.reveal(d.panel.viewColumn, false);
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      const f2r = await vscode.commands.executeCommand('ht9045Designer.editText');
      const f2Open = await waitFor(() => (d.textEditing ? true : null), 5000);
      d.post({ type: 'textEditEnd', commit: false });
      const f2Closed = await waitFor(() => (d.textEditing === false ? true : null), 5000);
      ok(!!(f2r && f2Open && f2Closed && d.doc.getText() === original && !d.doc.isDirty),
        'F2 (real): the text box opens on spbSave (textEditing on), closed without writing, the source untouched', JSON.stringify({ f2r, f2Open, f2Closed }));
      // Blend reparent (Alt at the release), what the probe sends: spbSave let go over the form -> out of Panel2 into the form
      // (the real tree says so) at the drop place; ONE undo
      const rpd = await hub.cmdReparentDrop(d, { offs: [{ id: 'spbSave', dx: 0, dy: 0 }], targets: [{ id: '@form', x: 20, y: 440 }] });
      const formKey = () => { const r = d.treeData.find(x => x[7]); return r ? r[0] : null; };
      const rpdTree = rpd ? await waitFor(() => { const r = d.treeData.find(x => x[2] === 'spbSave'); return r && r[1] === formKey() ? true : null; }, 10000) : null;
      const rpdRow = d.treeData.find(x => x[2] === 'spbSave');
      const rpdPar = rpdRow ? d.treeData.find(x => x[0] === rpdRow[1]) : null;
      ok(!!(rpd && rpd.into === '@form' && rpdTree && cdLeft('spbSave') === 20),
        'Blend reparent (real): spbSave out of Panel2 into the form at 20, 440 (the tree says so)', (rpd ? JSON.stringify(rpd) : '-') +
          ' tree=' + !!rpdTree + ' parent=' + (rpdPar ? rpdPar[2] : '-') + ' left=' + cdLeft('spbSave') + ' tag=' + ((cdHe.startTagOf(d.doc.getText(), 'spbSave') || {}).text || '').slice(0, 160));
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === original ? true : null), 5000)), 'ONE undo puts spbSave back into Panel2');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => { const r = d.treeData.find(x => x[2] === 'spbSave'); return r && r[1] === p2Key() ? true : null; }, 10000);
    }

    // 全選 (right-click menu) and the title bar's undo button
    {
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.selectAll');
      const all = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && (d.sel.info.multi || []).indexOf('sbtExit') >= 0 ? true : null), 5000);
      ok(!!all, '全選: spbSave + the rest of Panel2 (sbtExit among them)', d.sel && d.sel.info ? JSON.stringify(d.sel.info.multi) : '-');
      const orig2 = d.doc.getText();
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      const s9 = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && !(d.sel.info.multi || []).length ? d.sel : null), 5000);
      if (s9) d.post({ type: 'setLayout', key: s9.info.key, left: 61 });
      const mv9 = await waitFor(() => (d.doc.isDirty && d.doc.getText() !== orig2 ? true : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.undo');
      const un9 = await waitFor(() => (d.doc.getText() === orig2 ? true : null), 5000);
      ok(!!mv9 && !!un9, 'the title bar\'s undo button takes the edit back');
      // zoom to fit (the real webview reports its zoom), back to 100%; the Visual Studio keys' targets exist
      d.zoom = null;
      await vscode.commands.executeCommand('ht9045Designer.zoomFit');
      const zf = await waitFor(() => (typeof d.zoom === 'number' ? d.zoom : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.resetZoom');
      await waitFor(() => (d.zoom === 1 ? true : null), 5000);
      const cmdsNow = await vscode.commands.getCommands(true);
      ok(typeof zf === 'number' && zf >= 0.125 && zf <= 4 &&['ht9045Designer.properties.focus', 'ht9045Designer.toolbox.focus', 'ht9045Designer.components.focus'].every(c => cmdsNow.includes(c)),
        'zoom to fit: the webview picks a step (' + zf + '); F4 / Ctrl+Alt+X / Ctrl+Alt+T have their views');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
    }

    // the grid's HTML rows: z-index 5 on spbSave from the properties panel -> the source has it; undo
    {
      const orig3 = d.doc.getText();
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (hub.props.data && hub.props.data.comp && hub.props.data.comp.htmlId === 'spbSave' && hub.propsDesigner === d ? true : null), 8000);
      hub.onPropsMessage({ type: 'setStyleProp', name: 'z-index', value: '5' });
      const zi = await waitFor(() => (/id="spbSave"[^>]*z-index:5/.test(d.doc.getText()) ? true : null), 5000);
      const zp = await waitFor(() => (hub.props.data && hub.props.data.html && (hub.props.data.html.srcStyle || []).some(s => s[0] === 'z-index' && s[1] === '5') ? true : null), 5000);
      ok(!!zi && !!zp, 'HTML style row: z-index:5 written into spbSave\'s style, the panel shows it');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === orig3 ? true : null), 5000)), 'ONE undo takes it back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
    }

    // 選取同型別 with the real IR classes: spbSave -> every TSpeedButton of HotPlate (sbtExit too), said in the status line
    {
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      // (spbSave must be the selection first: if the page has not answered yet, sbtExit may still be selected and
      // become the primary of the same-type selection)
      const stFirst = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && !(d.sel.info.multi || []).length ? true : null), 10000);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.lastNote = '';
      await vscode.commands.executeCommand('ht9045Designer.selectSameType');
      // (10 s: the page answers the multi-select a little later on a busy machine)
      const st = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && (d.sel.info.multi || []).length ? d.sel.info : null), 10000);
      ok(!!st && st.multi.indexOf('sbtExit') >= 0 && /TSpeedButton/.test(d.lastNote || ''), 'select same type (real): spbSave + ' + (st ? st.multi.join(',') : '-'),
        (d.lastNote || '') + ' ' + JSON.stringify({ first: !!stFirst, now: d.sel && d.sel.info ? { id: d.sel.info.id, multi: d.sel.info.multi || [] } : null }));
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && !d.sel.info.multi ? true : null), 5000);
    }

    // 改名稱 (real): spbSave -> btnSaveX (confirmed): the source and the tree have the new name, it is selected; ONE undo
    {
      const orig6 = d.doc.getText();
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && !d.sel.info.multi ? true : null), 5000);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      const rn = await vscode.commands.executeCommand('ht9045Designer.rename', 'btnSaveX', true);
      const rnTree = await waitFor(() => (d.treeData.some(x => x[2] === 'btnSaveX') && !d.treeData.some(x => x[2] === 'spbSave') ? true : null), 10000);
      const rnSel = await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'btnSaveX' ? true : null), 8000);
      const rnSrc = /id="btnSaveX" title="btnSaveX : TSpeedButton/.test(d.doc.getText());
      ok(!!(rn && rn.to === 'btnSaveX' && rnSrc && rnTree && rnSel),
        'rename (real): spbSave -> btnSaveX in the source and the tree, selected', (rn ? JSON.stringify(rn) : '-') + ' src=' + rnSrc + ' tree=' + !!rnTree + ' sel=' + !!rnSel +
        ' now=' + (d.sel && d.sel.info ? d.sel.info.id : '-') + ' last=' + d.lastSelId + ' pending=' + (d.pendingSel && d.pendingSel.info ? d.pendingSel.info.id : '-') +
        ' active=' + (hub.active === d) + '/' + d.panel.active + ' renders=' + d.renderN);
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === orig6 ? true : null), 5000)), 'ONE undo takes the name back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.some(x => x[2] === 'spbSave') ? true : null), 10000);
      // 縮放到選取的元件 (real): spbSave -> zoomed in; back to 100%
      await sleep(800);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.sel = null;
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' ? true : null), 5000);
      // (the step depends on how wide this test window's designer column is: checked by the probe's own numbers)
      const zr = await d.request({ type: 'zoomSel' }, 5000);
      const steps = [0.125, 0.25, 0.33, 0.5, 0.67, 0.75, 0.9, 1, 1.1, 1.25, 1.5, 2, 3, 4];
      const zwant = zr && !zr.none ? Math.min((zr.vw - 60) / zr.w, (zr.vh - 60) / zr.h) : 0;
      const zexp = steps.filter(s => s <= zwant + 1e-9).pop() || 0.125;
      const zsel = await waitFor(() => (zr && d.zoom === zr.zoom ? d.zoom : null), 5000);
      await vscode.commands.executeCommand('ht9045Designer.resetZoom');
      const zback = await waitFor(() => (d.zoom === 1 ? true : null), 5000);
      ok(!!(zr && !zr.none && zr.zoom === zexp && Math.round(zr.w) === 227 && zsel && zback),
        'zoom to selection (real): spbSave (' + (zr ? Math.round(zr.w) + 'x' + Math.round(zr.h) : '-') + ' in a ' + (zr ? zr.vw + 'x' + zr.vh : '-') + ' view) -> ' + (zr ? zr.zoom : '-') + 'x; back to 100%');
    }

    // AutoSize (real): Label1 is AutoSize (width:auto); off from the panel -> its measured px in the source, the autos gone; ONE undo
    {
      const orig7 = d.doc.getText();
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      d.post({ type: 'selectId', id: 'Label1', origin: 'editor' });
      const asD = await waitFor(() => (hub.props.data && hub.props.data.comp && hub.props.data.comp.htmlId === 'Label1' && hub.props.data.edit && hub.props.data.edit.look ? hub.props.data : null), 8000);
      const asOn = asD && asD.edit.look.autoSize;
      const asDfm = asD && asD.edit.dfm ? asD.edit.dfm.autoSize : null;
      hub.onPropsMessage({ type: 'setLook', prop: 'autoSize', value: false });
      const lab1 = () => { const m = /<span[^>]*id="Label1"[^>]*>/.exec(d.doc.getText()); return m ? m[0] : ''; };
      const asW = await waitFor(() => (/width:\d+px/.test(lab1()) && !/width:auto/.test(lab1()) ? lab1() : null), 5000);
      ok(!!(asOn === true && asDfm === true && asW && /height:\d+px/.test(asW) && !/height:auto/.test(asW)),
        'AutoSize (real): Label1 on (the DFM too); off -> its measured width / height written, the autos gone', asW ? asW.slice(0, 160) : 'look ' + asOn + ' dfm ' + asDfm);
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === orig7 ? true : null), 5000)), 'ONE undo gives AutoSize back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
    }

    // Tab 順序 (real): HotPlate's stops numbered by the real probe against the .dfm's order; off = none
    {
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      const tOn = await vscode.commands.executeCommand('ht9045Designer.toggleTabOrder', true);
      d.diag = null; d.post({ type: 'diag' });
      const tg1 = await waitFor(() => d.diag, 5000);
      await vscode.commands.executeCommand('ht9045Designer.toggleTabOrder', false);
      d.diag = null; d.post({ type: 'diag' });
      const tg2 = await waitFor(() => d.diag, 5000);
      // (8 before the checkboxes were named by their <label>: now they count too)
      ok(!!(tOn && tOn.on && tOn.stops > 0 && tOn.common > 8 && tOn.dfm[0] === 'HotPlateName' && tg1 && tg1.tabBadges > 0 && tg2 && tg2.tabBadges === 0),
        'Tab 順序 (real): ' + (tOn ? tOn.stops + ' stops, ' + tOn.common + ' in the .dfm, ' + tOn.bad.length + ' out of order' : '-') + '; badges drawn, off = none',
        tOn && tOn.bad ? tOn.bad.slice(0, 3).join(' | ') : '');
    }

    // 改回 DFM (real): spbSave is bold on the page, not in the DFM -> the source gets font-weight:normal; ONE undo
    {
      const orig5 = d.doc.getText();
      d.post({ type: 'selectId', id: 'spbSave', origin: 'editor' });
      await waitFor(() => (d.sel && d.sel.info && d.sel.info.id === 'spbSave' && !d.sel.info.multi ? true : null), 5000);
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      // first move it 6 px (a second difference on the same tag: Left + Font.Bold -- both must be written)
      d.post({ type: 'setLayout', key: d.sel.info.key, left: 60 });
      await waitFor(() => (/id="spbSave"[^>]*left:60px/.test(d.doc.getText()) ? true : null), 5000);
      // (the page must report the move before the reset measures it -- a busy machine redraws slowly)
      let rdSeen = null;
      for (let i = 0; i < 40; i++) {
        const lk = await d.request({ type: 'lookAll', ids: ['spbSave'] }, 2000);
        const it0 = lk && lk.items && lk.items[0];
        rdSeen = it0 && it0.lay ? it0.lay.left : rdSeen;
        if (rdSeen === 60) break;
        await sleep(200);
      }
      await sleep(300);
      // WPF Layout > Reset one part (real): the position only -- left back to 54, the bold not touched; undone again
      const rpp = await vscode.commands.executeCommand('ht9045Designer.resetLayout.pos');
      const rppOk = await waitFor(() => (/id="spbSave"[^>]*left:54px/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!(rpp && rpp.only === 'pos' && rppOk && !/id="spbSave"[^>]*font-weight:normal/.test(d.doc.getText()) && rpp.items.every(i => i.type === 'setLayout')),
        'Layout > Reset position (real): spbSave left back to 54, the bold not touched', JSON.stringify(rpp && rpp.items));
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      await waitFor(() => (/id="spbSave"[^>]*left:60px/.test(d.doc.getText()) ? true : null), 5000);
      await sleep(300);
      const rd = await vscode.commands.executeCommand('ht9045Designer.resetToDfm');
      const rdOk = await waitFor(() => (/id="spbSave"[^>]*font-weight:normal/.test(d.doc.getText()) && /id="spbSave"[^>]*left:54px/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!(rd && rd.count >= 2 && rdOk), 'reset to DFM (real): spbSave moved + bold -> both back to its .dfm values in the source (' + (rd ? rd.count : 0) + ' items), not saved',
        JSON.stringify({ seenLeft: rdSeen, items: rd && rd.items ? rd.items.map(x => x.type + ':' + (x.prop || Object.keys(x).filter(k => k !== 'id' && k !== 'type').join('/'))) : null,
          src: (/id="spbSave"[^>]*/.exec(d.doc.getText()) || [''])[0].slice(0, 200) }));
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === orig5 ? true : null), 5000)), 'two undos (the reset, the move) give the page back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
    }

    // the properties panel with sbtExit + spbSave selected: italic on -> both in the source, ONE undo
    {
      const orig4 = d.doc.getText();
      d.post({ type: 'selectIds', ids: ['sbtExit', 'spbSave'], origin: 'tree' });
      await waitFor(() => (hub.props.data && hub.props.data.comp && hub.props.data.comp.htmlId === 'sbtExit' && hub.props.data.edit &&
        (hub.props.data.edit.multi || []).indexOf('spbSave') >= 0 && hub.propsDesigner === d ? true : null), 8000);
      // the values that differ between the two (left 332 / 54, width 241 / 227) are marked mixed, measured by the real probe
      const mxR = await waitFor(() => (hub.props.data && hub.props.data.edit && hub.props.data.edit.mixed ? hub.props.data.edit.mixed : null), 5000);
      ok(!!(mxR && mxR.left && mxR.width && !mxR.fontSize), 'several selected (real): left / width differ and are marked, the font size is the same', JSON.stringify(mxR));
      hub.onPropsMessage({ type: 'setLook', prop: 'italic', value: true });
      const it2 = await waitFor(() => (/id="sbtExit"[^>]*font-style:italic/.test(d.doc.getText()) && /id="spbSave"[^>]*font-style:italic/.test(d.doc.getText()) ? true : null), 5000);
      ok(!!it2, 'panel with 2 selected: Font.Italic on -> sbtExit and spbSave both italic in the source');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === orig4 ? true : null), 5000)), 'ONE undo takes both back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
    }

    // 頁面檢查 in the real Problems panel: HotPlate's cut style, VS Code offers our fix, fix-all fixes it; undo
    {
      d.panel.reveal(d.panel.viewColumn, false);
      await waitFor(() => hub.active === d, 3000);
      const sumL = await vscode.commands.executeCommand('ht9045Designer.lintPage');
      const ours = () => vscode.languages.getDiagnostics(d.doc.uri).filter(x => x.source === 'HTML 頁面檢查');
      const dgs = ours();
      ok(sumL && sumL.error === 1 && dgs.length === 1 && dgs[0].code === 'quote-cut', 'lint: HotPlate\'s cut style in the real Problems panel',
        dgs.map(x => x.code + '@' + (x.range.start.line + 1)).join(','));
      const hpPage = hub.pages.byFile.get(page.toLowerCase());
      const hpDesc = hpPage ? hub.pages.getTreeItem(hpPage).description : '';
      ok(/⚠ 1(\s|$)/.test(hpDesc), 'the 頁面 list shows it: HotPlate "⚠ 1"', hpDesc);
      const acts = dgs.length ? await vscode.commands.executeCommand('vscode.executeCodeActionProvider', d.doc.uri, dgs[0].range) : [];
      ok(Array.isArray(acts) && acts.some(a => /修正被切斷的屬性/.test(a.title)), 'VS Code offers the fix (the bulb)', (acts || []).map(a => a.title).join(' | '));
      const origL = d.doc.getText();
      const nFix = await vscode.commands.executeCommand('ht9045Designer.lintFixAll', d.doc.uri);
      const fixed = await waitFor(() => (/font-family:'MS Sans Serif',sans-serif;font-weight:400;"/.test(d.doc.getText()) ? true : null), 5000);
      const gone = await waitFor(() => (ours().length === 0 ? true : null), 5000);
      ok(nFix === 1 && !!fixed && !!gone, 'fix all: the inner quotes are single now, the problem gone, not saved');
      d.panel.reveal(d.panel.viewColumn, false);
      await sleep(300);
      await vscode.commands.executeCommand('undo');
      ok(!!(await waitFor(() => (d.doc.getText() === origL ? true : null), 5000)), 'ONE undo takes the fix back');
      if (d.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
      await waitFor(() => (d.treeData.length ? true : null), 8000);
      hub.lintDiag.clear();
    }

    // 在所有頁面搜尋: spbSave on another page -> that page opens and spbSave is selected there
    {
      const hits = await vscode.commands.executeCommand('ht9045Designer.searchPages', 'spbSave');
      const other = hits && hits.find(h => !web2.samePath(h.file, page) && /[\\/]page[\\/]/i.test(h.file));
      ok(hits && hits.length >= 5 && !!other, 'search all pages: spbSave on ' + (hits ? hits.length : 0) + ' pages', other ? path.basename(other.file) : '-');
      if (other) {
        const od = await hub.openPageAt(other.file, other.id);
        const sel = od ? await waitFor(() => (od.sel && od.sel.info && od.sel.info.id === other.id ? true : null), 10000) : null;
        ok(!!od && !!sel && hub.active === od, 'openPageAt: ' + path.basename(other.file) + ' opened in the designer, spbSave selected there');
        if (od) { await vscode.commands.executeCommand('workbench.action.closeActiveEditor'); await sleep(300); }
      }
    }

    // "全部改回" of a real pattern on Setup.Speed.html: one edit for all of them, one undo
    {
      const speedFile = path.join(path.dirname(page), 'Setup.Speed.html');
      const sd = await hub.openPageAt(speedFile, null);
      if (sd) {
        await sleep(800);
        const df = await vscode.commands.executeCommand('ht9045Designer.dfmDiff');
        const pat = df && df.patterns.find(p => p.group !== 'layout');
        const before = sd.doc.getText();
        const res = pat && sd.dfmDiff ? await sd.dfmDiff.resetPattern(pat.key, true) : null;
        const changed = res ? await waitFor(() => (sd.doc.isDirty && sd.doc.getText() !== before ? true : null), 8000) : null;
        ok(pat && res && res.count === pat.count && !!changed, 'Setup.Speed: 全部改回 ' + (pat ? pat.cls + ' ' + pat.prop + ' x' + pat.count : '-') + ' written into the source');
        sd.panel.reveal(sd.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        ok(!!(await waitFor(() => (sd.doc.getText() === before ? true : null), 5000)), 'ONE undo takes all of them back');
        if (sd.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        // DFM 位置 on Speed (it has position differences): frames drawn by the real probe; off = none
        await waitFor(() => (sd.treeData.length ? true : null), 8000);
        await waitFor(() => hub.active === sd, 3000);
        const sdDiag = async () => { sd.diag = null; sd.post({ type: 'diag' }); return waitFor(() => sd.diag, 5000); };
        const gs = await vscode.commands.executeCommand('ht9045Designer.toggleDfmGhosts', true);
        const gd1 = await sdDiag();
        await vscode.commands.executeCommand('ht9045Designer.toggleDfmGhosts', false);
        const gd2 = await sdDiag();
        ok(!!(gs && gs.on && gs.items.length > 0 && gs.items.length <= df.byGroup.layout),
          'DFM 位置 on Speed: ' + (gs ? gs.items.length : '-') + ' frames for ' + df.byGroup.layout + ' position differences', '');
        ok(gd1 && gd1.ghosts > 0 && gd2 && gd2.ghosts === 0 && !sd.doc.isDirty, 'the real probe draws them (' + (gd1 ? gd1.ghosts : '-') + ' shown); off = none; the page untouched');
        await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
        await sleep(300);
      } else ok(false, 'Setup.Speed opens for the pattern test');
    }

    // a real porting gap: Setup.Contact's labDelayStatus is AutoSize=False, 273 x 25, taCenter in BCB6 but drawn
    // width:auto, left-aligned on the page -- 改回 DFM gives it the .dfm's size and centring in one step; ONE undo
    {
      const ctFile = path.join(path.dirname(page), 'Setup.Contact.html');
      const cd = fs.existsSync(ctFile) ? await hub.openPageAt(ctFile, 'labDelayStatus') : null;
      if (cd) {
        await waitFor(() => (cd.sel && cd.sel.info && cd.sel.info.id === 'labDelayStatus' ? true : null), 8000);
        cd.panel.reveal(cd.panel.viewColumn, false);
        await waitFor(() => hub.active === cd, 3000);
        const ctBefore = cd.doc.getText();
        const ctRes = await vscode.commands.executeCommand('ht9045Designer.resetToDfm');
        const ctTag = () => { const m = /<span[^>]*id="labDelayStatus"[^>]*>/.exec(cd.doc.getText()); return m ? m[0] : ''; };
        const ctOk = await waitFor(() => (/width:273px/.test(ctTag()) && /height:25px/.test(ctTag()) && /text-align:center/.test(ctTag()) && !/width:auto/.test(ctTag()) ? ctTag() : null), 6000);
        ok(!!(ctRes && ctOk), 'Setup.Contact labDelayStatus (AutoSize=False, taCenter in the DFM): 改回 DFM -> 273 x 25, centred, in the source',
          (ctRes ? JSON.stringify(ctRes.items) : '-') + ' | ' + (ctOk || ctTag()).slice(0, 200));
        cd.panel.reveal(cd.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        ok(!!(await waitFor(() => (cd.doc.getText() === ctBefore ? true : null), 5000)), 'ONE undo gives Setup.Contact back');
        // lblWarning1 wraps (WordWrap): a fixed width, height:auto -- that is AutoSize on, as in its .dfm (not a difference)
        await waitFor(() => (cd.treeData.length ? true : null), 8000);
        cd.post({ type: 'selectId', id: 'lblWarning1', origin: 'editor' });
        const wwD = await waitFor(() => (hub.props.data && hub.props.data.comp && hub.props.data.comp.htmlId === 'lblWarning1' && hub.props.data.edit && hub.props.data.edit.look ? hub.props.data : null), 8000);
        ok(!!(wwD && wwD.edit.look.autoSize === true && wwD.edit.dfm && wwD.edit.dfm.autoSize === true),
          'a WordWrap label (lblWarning1: fixed width, height:auto) reads AutoSize on, the same as its .dfm', wwD ? JSON.stringify({ page: wwD.edit.look.autoSize, dfm: wwD.edit.dfm && wwD.edit.dfm.autoSize }) : '-');
        if (cd.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
        await sleep(300);
      } else ok(false, 'Setup.Contact opens for the AutoSize / Alignment reset');
    }

    // 頁面檢查 without opening the page: Setup.Contact's "differs from the .dfm" notes in the real Problems panel
    // (information), VS Code offers the bulb, the page-wide fix writes labDelayStatus's 273 x 25 (not saved; reverted)
    let gStep = 'start';
    try {
      const ctFile2 = path.join(path.dirname(page), 'Setup.Contact.html');
      const ctUri = vscode.Uri.file(ctFile2);
      gStep = 'lintFile';
      hub.lintFile(ctFile2);
      gStep = 'getDiagnostics';
      const gd = vscode.languages.getDiagnostics(ctUri).filter(x => x.source === 'HTML 頁面檢查' && x.code === 'dfm-gap');
      const gdLab = gd.find(x => /labDelayStatus/.test(x.message));
      gStep = 'codeActions';
      await vscode.workspace.openTextDocument(ctUri);
      const gActs = gdLab ? await vscode.commands.executeCommand('vscode.executeCodeActionProvider', ctUri, gdLab.range) : [];
      ok(!!(gdLab && gdLab.severity === vscode.DiagnosticSeverity.Information && (gActs || []).some(a => /改成 BCB6 \.dfm 的大小與對齊/.test(a.title))),
        'differs from the .dfm (real Problems panel): ' + gd.length + ' notes on Setup.Contact, labDelayStatus among them, the bulb offered', (gActs || []).map(a => a.title).slice(0, 3).join(' | '));
      gStep = 'fixAll';
      const nFixed = await vscode.commands.executeCommand('ht9045Designer.lintFixDfmGaps', ctUri);
      gStep = 'check';
      const ctDoc = vscode.workspace.textDocuments.find(td => td.uri.fsPath.toLowerCase() === ctFile2.toLowerCase());
      const ctFixedTag = ctDoc ? (/<span[^>]*id="labDelayStatus"[^>]*>/.exec(ctDoc.getText()) || [''])[0] : '';
      ok(!!(nFixed === gd.length && /width:273px/.test(ctFixedTag) && /text-align:center/.test(ctFixedTag) && ctDoc.isDirty), 'fix the page: all ' + nFixed + ' written at once, not saved', ctFixedTag.slice(0, 160));
      gStep = 'revert';
      if (ctDoc) {
        await vscode.window.showTextDocument(ctDoc);
        await vscode.commands.executeCommand('workbench.action.files.revert');
        await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
      }
      ok(!!(ctDoc && !ctDoc.isDirty), 'reverted, nothing saved');
      hub.lintDiag.delete(ctUri);
    } catch (e) { ok(false, 'differs from the .dfm (real): failed at ' + gStep, String(e && e.stack || e).slice(0, 300)); }

    // another gap: Data.Observer's palCustomer is a TPanel with taLeftJustify in BCB6, its caption centred on the
    // page (the .pnlCap class) -- 改回 DFM writes text-align:left on the caption span; ONE undo
    {
      const obFile = path.join(path.dirname(page), 'Data.Observer.html');
      const od = fs.existsSync(obFile) ? await hub.openPageAt(obFile, 'palCustomer') : null;
      if (od) {
        await waitFor(() => (od.sel && od.sel.info && od.sel.info.id === 'palCustomer' ? true : null), 8000);
        od.panel.reveal(od.panel.viewColumn, false);
        await waitFor(() => hub.active === od, 3000);
        const obBefore = od.doc.getText();
        const obRes = await vscode.commands.executeCommand('ht9045Designer.resetToDfm');
        const obCap = () => { const m = /<div class="pnl" id="palCustomer"[^>]*>\s*(<span class="pnlCap"[^>]*>)/.exec(od.doc.getText()); return m ? m[1] : ''; };
        const obOk = await waitFor(() => (/text-align:left/.test(obCap()) ? obCap() : null), 6000);
        ok(!!(obRes && obOk), 'Data.Observer palCustomer (taLeftJustify in the DFM): 改回 DFM -> text-align:left on its caption span',
          (obRes ? JSON.stringify(obRes.items) : '-') + ' | ' + (obOk || obCap()).slice(0, 160));
        od.panel.reveal(od.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        ok(!!(await waitFor(() => (od.doc.getText() === obBefore ? true : null), 5000)), 'ONE undo gives Data.Observer back');
        if (od.doc.isDirty) await vscode.commands.executeCommand('workbench.action.files.revert');
        await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
        await sleep(300);
      } else ok(false, 'Data.Observer opens for the panel Alignment reset');
    }

    // 頁面 list (the real tree view): HotPlate is the current page; a click switches to Setup.Speed.html
    {
      const pgs = hub.pages.getChildren();
      const hpNode = hub.pages.byFile.get(page.toLowerCase());
      const hpItem = hpNode ? hub.pages.getTreeItem(hpNode) : null;
      ok(pgs.length >= 5 && hpItem && /● 目前/.test(hpItem.description), '頁面 list: HotPlate marked as the current page',
        pgs.map(g => g.name + ' ' + g.children.length).join(', '));
      const speed = path.join(path.dirname(page), 'Setup.Speed.html');
      const spNode = hub.pages.byFile.get(speed.toLowerCase());
      ok(!!spNode && spNode.label === 'Speed', '頁面 list has Setup › Speed');
      if (spNode) {
        const it = hub.pages.getTreeItem(spNode);
        await vscode.commands.executeCommand(it.command.command, ...it.command.arguments);   // what a click runs
        const sd = await waitFor(() => (hub.active && /Setup\.Speed\.html$/i.test(hub.active.file) && hub.active.treeData.length ? hub.active : null), 30000);
        ok(!!sd, 'a click in the 頁面 list opens that page in the designer');
        const it2 = hub.pages.getTreeItem(spNode);
        const it3 = hub.pages.getTreeItem(hpNode);
        ok(/● 目前/.test(it2.description) && /○ 已開啟/.test(it3.description), 'the list follows: Speed is current, HotPlate still open',
          it2.description + ' | ' + it3.description);
        if (sd) { await vscode.commands.executeCommand('workbench.action.closeActiveEditor'); await sleep(300); }
      }
    }

    // sweep: other kinds of page (generated / hand-written / JS-built / iframe)
    const sweep = String(process.env.HTD_IT_SWEEP || '').split(';').filter(Boolean);
    const sweepBad = [];
    if (sweep.length) {
      out.push('');
      out.push('sweep (nodes / DFM / images / JS errors / blocked / net wall / a component resolves):');
      for (const p of sweep) {
        const file = path.join(path.dirname(page), p);
        const t0 = Date.now();
        await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(file), 'ht9045Designer.editor');
        const dd = await waitFor(() => {
          const a = hub.active;
          return a && a.file.toLowerCase() === file.toLowerCase() && (a.treeData.length || (a.redirect && a.readyCount)) ? a : null;
        }, 30000);
        if (dd && dd.redirect) {
          ok(true, 'sweep ' + p, 'a forwarding page -> ' + dd.redirect + ' (shown as a notice, not followed)');
          await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
          await sleep(300);
          continue;
        }
        if (!dd) {
          const any = Array.from(hub.designers).find(x => x.file.toLowerCase() === file.toLowerCase());
          ok(false, 'sweep ' + p + ': tree arrives', 'timeout' + (any ? ' (designer exists: ready=' + (any.readyCount || 0) + ' msgs=' + (any.msgCount || 0) +
            ' last=' + (any.lastMsg || '-') + ' tree=' + any.treeData.length + ' active=' + (hub.active === any) + ')' : ' (no designer)'));
          (hub.logLines || []).slice(-8).forEach(l => out.push('        log ' + l));
          await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
          continue;
        }
        const dg = await waitFor(() => dd.diag, 8000);
        dd.post({ type: 'netcheck' });
        const n2 = await waitFor(() => dd.netcheck, 8000);
        const pi = await dd.info;
        // resolve the first component that has DFM events (or any id)
        const ids = dd.treeData.map(r => r[2]).filter(id => id && id !== '@form');
        const withEv = pi.ir ? ids.find(id => { const n = pi.ir.byName.get(id); return n && n.events && Object.keys(n.events).length; }) : null;
        const pickId = withEv || ids[0];
        let res = '-';
        if (pickId) {
          dd.sel = null;
          dd.post({ type: 'selectId', id: pickId, origin: 'editor' });
          const s2 = await waitFor(() => (dd.sel && dd.sel.info && dd.sel.info.id === pickId ? dd.sel : null), 8000);
          const r2 = s2 ? await s2.promise : null;
          if (r2) {
            const evs = r2.events.map(e => e.name + '(' + e.targets.map(i => r2.targets[i].kind[0]).join('') + ')');
            res = pickId + ' ' + (evs.join(' ') || 'no DFM events') + ' listeners=' + r2.listeners.length;
          } else res = pickId + ' did not resolve';
        }
        let ovs = '-';
        if (pi.ir) {
          const t1 = Date.now();
          const o = await vscode.commands.executeCommand('ht9045Designer.pageOverview');
          ovs = o ? 'overview ' + o.summary.events + ' ev, web ✔' + (o.summary.web.yes + o.summary.web.field) + ' △' + o.summary.web.weak + ' ✗' + o.summary.web.no +
            ', C++ ✔' + o.summary.port.live + ' ✗' + o.summary.port.none + ' (' + (Date.now() - t1) + ' ms)' : 'overview failed';
          if (dd.overview) dd.overview.panel.dispose();   // the overview tab (opened without focus)
          // the whole page against the DFM (read-only: nothing is reset here)
          dd.panel.reveal(dd.panel.viewColumn, false);
          await waitFor(() => hub.active === dd, 3000);
          const t2 = Date.now();
          const df2 = await vscode.commands.executeCommand('ht9045Designer.dfmDiff');
          let top2 = '';
          if (df2) {
            const byProp = {};
            for (const r of df2.rows) byProp[r.prop] = (byProp[r.prop] || 0) + 1;
            top2 = Object.keys(byProp).sort((a, b) => byProp[b] - byProp[a]).slice(0, 4).map(k => k + ' ' + byProp[k]).join(', ');
          }
          ovs += df2 ? ' | DFM diff ' + df2.controls + ' ctl / ' + df2.compared + ' cmp: pos ' + df2.byGroup.layout + ' text ' + df2.byGroup.text +
            ' look ' + df2.byGroup.look + ', single ' + df2.single + ', patterns ' + df2.patterns.length + ' [' + top2 + '] (' + (Date.now() - t2) + ' ms)' : ' | DFM diff failed';
          if (dd.dfmDiff) dd.dfmDiff.panel.dispose();
          if (!df2) sweepBad.push(p + ': DFM diff failed');
        }
        ok(!!(n2 && n2.ok) && !sweepBad.some(s => s.indexOf(p + ':') === 0), 'sweep ' + p, dd.treeData.length + ' nodes | ' + ovs + ' | ' +
          (pi.ir ? path.basename(pi.ir.file).replace('.dfm.ir.json', '') + '(' + (pi.dfm && pi.dfm.how) + ')' : 'no DFM') +
          ' | img ' + (dg ? dg.imagesLoaded + '/' + dg.images : '?') +
          ' | err ' + dd.errors.length + (dd.errors.length ? ' [' + dd.errors[0].msg.slice(0, 80) + ']' : '') +
          ' | blocked ' + dd.blocked.length + (dd.blocked.length ? ' [' + dd.blocked.map(b => b.uri).slice(0, 2).join(' ') + ']' : '') +
          ' | ' + (Date.now() - t0) + ' ms | ' + res);
        await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
        await sleep(300);
      }
    }
    // the side bar's 頁面 list brought back by a command (it may have been hidden); the names people see are generic
    const psShown = await vscode.commands.executeCommand('ht9045Designer.showPages');
    const psCmds = await vscode.commands.getCommands(true);
    const extVer = (vscode.extensions.getExtension('ht9045.ht9045-html-designer') || { packageJSON: {} }).packageJSON.version;
    ok(!!(psShown === true && hub.pageView.visible && psCmds.includes('ht9045Designer.showPages') && hub.pageStatus && /\$\(file-code\)/.test(hub.pageStatus.text) && hub.out.name === 'HTML 視覺設計' &&
      extVer && hub.pageView.description === 'v' + extVer),
      'the 頁面 list shown by its command (its title says v' + extVer + '); the page switcher in the status bar; the output channel "HTML 視覺設計"', hub.pageStatus && hub.pageStatus.text);
    // 有新版: run from the source tree nothing is found; a newer version in an extensions folder -> the status
    // bar button and the title; nothing newer -> gone. (The reload itself is not run: it would end this test.)
    const upPkg = (vscode.extensions.getExtension('ht9045.ht9045-html-designer') || { packageJSON: {} }).packageJSON;
    const upSrc = hub.checkUpdate();
    const upDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-upd-it-'));
    fs.writeFileSync(path.join(upDir, 'extensions.json'), JSON.stringify([{ identifier: { id: 'ht9045.ht9045-html-designer' }, version: '99.0.0' }]));
    const upNew = hub.checkUpdate(upDir);
    const upText = hub.updateItem.text, upDesc = hub.pageView.description;
    fs.writeFileSync(path.join(upDir, 'extensions.json'), JSON.stringify([{ identifier: { id: 'ht9045.ht9045-html-designer' }, version: extVer }]));
    const upGone = hub.checkUpdate(upDir);
    fs.rmSync(upDir, { recursive: true, force: true });
    const upMenus = ((upPkg.contributes || {}).menus || {})['view/title'] || [];
    ok(!!(upSrc === null && upNew === '99.0.0' && /v99\.0\.0/.test(upText) && hub.updateItem.command === 'ht9045Designer.reloadWindow' && /新版 v99\.0\.0/.test(upDesc) &&
      upGone === null && hub.pageView.description === 'v' + extVer && psCmds.includes('ht9045Designer.reloadWindow') &&
      upMenus.some(m => m.command === 'ht9045Designer.reloadWindow' && /ht9045Designer\.pages/.test(m.when))),
      'a newer version installed: the status bar button (reload) and the 頁面 title; the reload button in the 頁面 title bar', upText + ' / ' + upDesc);

    // 搜尋頁面 + 機種: find grpLoader_9050 from the box's command, open it from the list; the page is shown for the
    // machine F5 opens the HMI for (launch.json) -- so the HT9050-only group is visible -- and with 不指定 it is not
    const snS = await vscode.commands.executeCommand('ht9045Designer.filterPages', 'grpLoader_9050');
    const snHw = hub.pages.getChildren().find(n => n.name === 'HW');
    const snIo = snHw && hub.pages.getChildren(snHw).find(n => n.name === 'HW.IoSetView.html');
    const snHit = snIo && hub.pages.getChildren(snIo).find(n => n.kind === 'hit' && n.hit.id === 'grpLoader_9050');
    const snCmd = snHit && hub.pages.getTreeItem(snHit).command;
    if (snCmd) await vscode.commands.executeCommand(snCmd.command, ...snCmd.arguments);
    const snHints = require(path.join(__dirname, '..', '..', 'lib', 'liveconfig.js')).launchHints([hub.active ? hub.active.r.portRoot : '']);
    const rowOf = id => { const d = hub.active; const r = d && d.treeData.find(x => x[2] === id); return r || null; };
    let snRow = null;
    for (let i = 0; i < 100; i++) {
      snRow = rowOf('grpLoader_9050');
      if (hub.active && /HW\.IoSetView\.html$/.test(hub.active.file) && snRow && snRow[6] === 0) break;
      await sleep(200);
    }
    const snLive = hub.active && hub.active.live;
    // 設計時全部顯示: grpLoaderFunc is Visible=False (display:none in its style) -- the tree says 執行時隱藏
    let snFunc = null;
    for (let i = 0; i < 50; i++) { snFunc = rowOf('grpLoaderFunc'); if (snFunc && snFunc[8] === 1) break; await sleep(200); }
    const snFuncNode = snFunc && Array.from(hub.tree.byKey.values()).find(n => n.id === 'grpLoaderFunc');
    const snFuncDesc = snFuncNode ? hub.tree.getTreeItem(snFuncNode).description : '';
    ok(!!(snFunc && snFunc[8] === 1 && /執行時隱藏/.test(snFuncDesc)),
      'design mode shows everything: grpLoaderFunc (Visible=False) is marked hidden-when-running, the tree says 執行時隱藏', snFuncDesc);
    // 事件表 on a component the .dfm does not have (BtnPanelLane3, in the 9050 group): its click is wired through the
    // page's JS -> WS io.btnPanelClick -> C++ (EastSun 20260930: "應該有click吧? 他是連到C++的")
    let bpRow = null, bpData = null;
    if (hub.active && /HW\.IoSetView\.html$/.test(hub.active.file)) {
      const dio = hub.active;
      dio.post({ type: 'selectId', id: 'BtnPanelLane3', origin: 'editor' });
      const bpSel = await waitFor(() => (dio.sel && dio.sel.info && dio.sel.info.id === 'BtnPanelLane3' ? dio.sel : null), 10000);
      bpData = bpSel ? await bpSel.promise : null;
      bpRow = bpData && bpData.eventGrid && bpData.eventGrid.rows.find(r => r.name === 'OnClick');
    }
    const bpT = i => (bpData && i != null ? bpData.targets[i] : null);
    ok(!!(bpRow && bpRow.cmd === 'io.btnPanelClick' && /DispatchIoClick/.test(bpRow.handler) && bpT(bpRow.web) && bpT(bpRow.web).kind === 'web' &&
      bpT(bpRow.server) && /wb_serve\.cpp$/i.test(bpT(bpRow.server).file) && bpT(bpRow.port) && bpT(bpRow.port).kind === 'port'),
      '事件表: BtnPanelLane3 (not in the .dfm) OnClick = the page\'s listener -> io.btnPanelClick -> C++ ' + (bpRow ? bpRow.handler : '-'),
      bpRow ? JSON.stringify({ cmd: bpRow.cmd, handler: bpRow.handler, web: bpT(bpRow.web) && path.basename(bpT(bpRow.web).file), server: bpT(bpRow.server) && path.basename(bpT(bpRow.server).file) + ':' + bpT(bpRow.server).line }) : 'no row');
    // ONLY the function that runs (EastSun 20260930: "我需要你只對應到W906_Main_CloseProgramOp，其他的事件也都是一樣")
    ok(!!(bpRow && bpRow.mainName === 'W906_DispatchIoClick' && bpT(bpRow.main) && bpT(bpRow.main).kind === 'port' &&
      /W906_DispatchIoClick/.test(bpT(bpRow.main).snippet || '')),
      '事件表: BtnPanelLane3 OnClick -> ONE C++ function, W906_DispatchIoClick (not the guard, not the dispatch line)',
      bpRow ? bpRow.mainName + ' @ ' + (bpT(bpRow.main) ? path.basename(bpT(bpRow.main).file) + ':' + bpT(bpRow.main).line : '-') : 'no row');
    // 事件表 on main.html's Exit button: OnClick = BCB6's sbCloseProgramClick AND the port's C++ it runs --
    // ht9045_main_close.js raw(CMD) with CMD = 'act.main.closeProgram' -> W906_Main_CloseProgramOp (FileRW/MainClose.cpp)
    // (EastSun 20260930: "BCB 就可以對應到 TfMain::sbCloseProgramClick，你這邊怎不行對應到?")
    let exRow = null, exData = null;
    const mainPage = path.join(hub.pages.root() || '', 'page', 'main.html');
    if (fs.existsSync(mainPage)) {
      const dm = await hub.openPageAt(mainPage, 'sbCloseProgram');
      if (dm) {
        const exSel = await waitFor(() => (dm.sel && dm.sel.info && dm.sel.info.id === 'sbCloseProgram' ? dm.sel : null), 15000);
        exData = exSel ? await exSel.promise : null;
        exRow = exData && exData.eventGrid && exData.eventGrid.rows.find(r => r.name === 'OnClick');
      }
    }
    const exT = i => (exData && i != null ? exData.targets[i] : null);
    const exEv = exRow && exRow.idx >= 0 ? exData.events[exRow.idx] : null;
    const exPort = exEv ? exEv.targets.map(exT).filter(t => t && t.kind === 'port' && /W906_Main_CloseProgramOp/.test(t.note || '')) : [];
    ok(!!(exRow && exRow.handler === 'sbCloseProgramClick' && exRow.dfm && exRow.cmd === 'act.main.closeProgram' && exT(exRow.port) &&
      /MainClose\.cpp$/i.test(exT(exRow.port).file) && exPort.length && exEv.targets.some(k => exT(k) && exT(k).kind === 'golden')),
      '事件表: main.html sbCloseProgram OnClick = sbCloseProgramClick (BCB6) and its C++: act.main.closeProgram -> W906_Main_CloseProgramOp',
      exRow ? JSON.stringify({ handler: exRow.handler, cmd: exRow.cmd, port: exT(exRow.port) && path.basename(exT(exRow.port).file) + ':' + exT(exRow.port).line }) : 'no row');
    // ... and its ONE function: W906_Main_CloseProgramOp; a double-click opens it straight away (no list)
    const exMain = exRow ? exT(exRow.main) : null;
    ok(!!(exRow && exRow.mainName === 'W906_Main_CloseProgramOp' && exMain && /MainClose\.cpp$/i.test(exMain.file) &&
      /W906_Main_CloseProgramOp/.test(exMain.snippet || '')),
      '事件表: sbCloseProgram OnClick -> ONE C++ function, W906_Main_CloseProgramOp in FileRW\\MainClose.cpp',
      exRow ? exRow.mainName + ' @ ' + (exMain ? path.basename(exMain.file) + ':' + exMain.line : '-') : 'no row');
    if (exRow && exMain) {
      await hub.onEventGrid(exData, 'OnClick', hub.active);
      const exEd = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && /MainClose\.cpp$/i.test(e.document.uri.fsPath) ? e : null; }, 8000);
      // (WPF: the caret inside the handler's body, ready to type -- not on its name)
      const exBody = exEd ? require(path.join(__dirname, '..', '..', 'lib', 'cppstub.js')).bodyStart(exEd.document.getText(), exMain.line - 1) : null;
      ok(!!(exEd && exBody && exBody.line >= exMain.line - 1 && exEd.selection.active.line === exBody.line && exEd.selection.active.character === exBody.col),
        'double-click on sbCloseProgram OnClick opens W906_Main_CloseProgramOp at once, the caret inside its body (MainClose.cpp:' + exMain.line + ')',
        exEd ? path.basename(exEd.document.uri.fsPath) + ':' + (exEd.selection.active.line + 1) + ':' + exEd.selection.active.character + ' body ' + JSON.stringify(exBody) : 'not opened');
    }
    if (exData) {
      // (its tabs by name -- which editor is active after a code tab opened beside it is not certain)
      const tabsOf = re => vscode.window.tabGroups.all.flatMap(g => g.tabs).filter(t => t.input && t.input.uri && re.test(t.input.uri.fsPath));
      const mcTabs = tabsOf(/MainClose\.cpp$/i).filter(t => !t.isDirty);
      if (mcTabs.length) await vscode.window.tabGroups.close(mcTabs);
      const mhTabs = tabsOf(/[\\/]page[\\/]main\.html$/i).filter(t => !t.isDirty);
      if (mhTabs.length) await vscode.window.tabGroups.close(mhTabs);
      else await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
      // (the IO page's designer tab, in ITS group -- openWith without a column would open a second designer in the active one)
      const io = tabsOf(/HW\.IoSetView\.html$/).find(t => t.input.viewType === 'ht9045Designer.editor');
      if (io && !(hub.active && /HW\.IoSetView\.html$/.test(hub.active.file))) {
        try { await vscode.commands.executeCommand('vscode.openWith', io.input.uri, 'ht9045Designer.editor', { viewColumn: io.group.viewColumn, preserveFocus: false }); }
        catch (e) { /* the wait below says */ }
      }
      // (back on the IO page: the checks after this use it)
      await waitFor(() => (hub.active && /HW\.IoSetView\.html$/.test(hub.active.file) ? true : null), 8000);
    }

    // Alias: ONE change -> the page's title changes and the panel shows the NEW Alias at once (it read the source
    // text before the edit reached it, showed the old one, and it took a second Enter)
    let alShown = null, alMade = null, alBack = false, alWhy = {};
    if (bpData && hub.active && /HW\.IoSetView\.html$/.test(hub.active.file)) {
      const dio = hub.active;
      alMade = hub.cmdSetAlias(dio, bpData, 'C_HtdAliasTest');
      const pd = await waitFor(() => { const x = hub.props.data; return x && x.comp && x.comp.htmlId === 'BtnPanelLane3' && x.edit && x.edit.alias === 'C_HtdAliasTest' ? x : null; }, 8000);
      alShown = pd ? pd.edit.alias : (hub.props.data && hub.props.data.edit ? hub.props.data.edit.alias : null);
      const srcOk = await waitFor(() => (/id="BtnPanelLane3"[^>]*Alias=C_HtdAliasTest/.test(dio.doc.getText()) ? true : null), 4000);
      await vscode.window.showTextDocument(dio.doc, { preview: false });
      await vscode.commands.executeCommand('workbench.action.files.revert');
      // (the revert reaches the document a moment later)
      await waitFor(() => (!dio.doc.isDirty ? true : null), 4000);
      const eqDisk = dio.doc.getText() === fs.readFileSync(dio.file, 'utf8');
      alBack = !!srcOk && !dio.doc.isDirty && eqDisk;
      alWhy = { src: !!srcOk, dirty: dio.doc.isDirty, eqDisk };
      const tt = vscode.window.tabGroups.all.flatMap(g => g.tabs).find(t => t.input && t.input.uri && /HW\.IoSetView\.html$/.test(t.input.uri.fsPath) && !t.input.viewType && !t.isDirty);
      if (tt) await vscode.window.tabGroups.close(tt);
    }
    ok(!!(alMade && alMade.from === 'C_LoaderEdgePush' && alShown === 'C_HtdAliasTest' && alBack),
      'Alias: one change -> the title in the source, and the panel shows the new Alias at once (reverted)', JSON.stringify({ made: alMade, shown: alShown, back: alBack, why: alWhy }));
    const snNone = await vscode.commands.executeCommand('ht9045Designer.pickMachine', '-');
    let snRowNone = null;
    for (let i = 0; i < 100; i++) {
      await sleep(200);
      snRowNone = rowOf('grpLoader_9050');
      if (snRowNone && snRowNone[6] === 1) break;
    }
    await vscode.commands.executeCommand('ht9045Designer.pickMachine', '');
    await vscode.commands.executeCommand('ht9045Designer.clearPageFilter');
    ok(!!(snS && snS.pages >= 1 && snCmd && snCmd.command === 'ht9045Designer.openPageAt' && hub.active && /HW\.IoSetView\.html$/.test(hub.active.file) &&
      (snHints.machine !== 'HT9050' || (snLive && snLive.machine === 'HT9050' && snRow && snRow[6] === 0)) &&
      snNone && snNone.machine === null && snRowNone && snRowNone[6] === 1 && hub.pages.getChildren().length > 3),
      'search "grpLoader_9050" -> opened from the list; shown for ' + (snHints.machine || '(no machine in launch.json)') + ' like F5 (visible), not with 不指定 (hidden)',
      JSON.stringify({ machine: snLive && snLive.machine, docs: snLive && Object.keys(snLive.docs || {}), row: snRow && snRow[6], none: snRowNone && snRowNone[6] }));
    // CSV 表格 on a COPY of the machine's Mot_Table.csv (the real file is never opened): one cell changed =
    // only its bytes; VS Code's undo takes it back; nothing saved
    const mtReal = 'D:\\HT9045\\system\\Mot_Table.csv';
    if (fs.existsSync(mtReal)) {
      const mtDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-csv-it-'));
      const mtCopy = path.join(mtDir, 'Mot_Table.csv');
      fs.copyFileSync(mtReal, mtCopy);
      const mt0 = fs.readFileSync(mtCopy, 'utf8');
      await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(mtCopy), 'ht9045Designer.csv');
      let mtT = null;
      for (let i = 0; i < 60 && !mtT; i++) { mtT = Array.from(hub.csv.open).find(x => x.doc.uri.fsPath.toLowerCase() === mtCopy.toLowerCase() && x.posted > 0) || null; if (!mtT) await sleep(200); }
      const mtP = require(path.join(__dirname, '..', '..', 'lib', 'csvtable.js')).parseDoc(mt0);
      const mtCell = mtP.rows[1].cells[23];
      let mtEdited = null, mtDirty = false, mtUndone = null, mtDirtyAfter = true;
      if (mtT) {
        await hub.csv.onMessage(mtT, { type: 'edit', changes: [{ r: 1, c: 23, v: '123' }] });
        mtEdited = mtT.doc.getText();
        mtDirty = mtT.doc.isDirty;
        mtT.panel.reveal(mtT.panel.viewColumn, false);
        await sleep(300);
        await vscode.commands.executeCommand('undo');
        for (let i = 0; i < 20 && mtT.doc.getText() !== mt0; i++) await sleep(150);
        mtUndone = mtT.doc.getText();
        mtDirtyAfter = mtT.doc.isDirty;
      }
      ok(!!(mtT && mtEdited === mt0.slice(0, mtCell.s) + '123' + mt0.slice(mtCell.e) && mtDirty && mtUndone === mt0 && !mtDirtyAfter &&
        fs.readFileSync(mtCopy, 'utf8') === mt0),
        'CSV 表格 (a copy of Mot_Table.csv): one cell = only its bytes, not saved; VS Code\'s undo takes it back exactly',
        JSON.stringify({ opened: !!mtT, cell: mtCell && mtCell.v, dirty: mtDirty, undone: mtUndone === mt0, dirtyAfter: mtDirtyAfter }));
      await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
      // opened the ordinary way (vscode.open, like a click in the explorer): the table, not text
      await vscode.commands.executeCommand('vscode.open', vscode.Uri.file(mtCopy));
      let otTab = null;
      for (let i = 0; i < 30; i++) {
        const at = vscode.window.tabGroups.activeTabGroup.activeTab;
        if (at && at.input instanceof vscode.TabInputCustom && at.input.viewType === 'ht9045Designer.csv') { otTab = at; break; }
        await sleep(200);
      }
      await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
      // a text tab of it (as one from before the update): switched to the table by itself, the text tab closes
      hub.csvAsked.delete(mtCopy.toLowerCase());
      const otDoc = await vscode.workspace.openTextDocument(mtCopy);
      await vscode.window.showTextDocument(otDoc);
      const otShown = true;
      let otText = -1, otTable = false;
      for (let i = 0; i < 20; i++) {
        const tabs = vscode.window.tabGroups.all.reduce((a, g) => a.concat(g.tabs), []);
        otText = tabs.filter(t => t.input instanceof vscode.TabInputText && t.input.uri.fsPath.toLowerCase() === mtCopy.toLowerCase()).length;
        otTable = tabs.some(t => t.input instanceof vscode.TabInputCustom && t.input.viewType === 'ht9045Designer.csv' && t.input.uri.fsPath.toLowerCase() === mtCopy.toLowerCase());
        if (otText === 0 && otTable) break;
        await sleep(200);
      }
      ok(!!(otTab && otShown && otText === 0 && otTable && fs.readFileSync(mtCopy, 'utf8') === mt0),
        'a .csv opened the ordinary way is the table; a text tab of it switches to the table by itself (the text tab closes)',
        JSON.stringify({ openedAsTable: !!otTab, button: otShown, textTabs: otText, table: otTable }));
      await vscode.commands.executeCommand('workbench.action.closeActiveEditor');
      try { fs.rmSync(mtDir, { recursive: true, force: true }); } catch (e) { /* ignore */ }
    }
    // Shift+F7 (WPF View Designer): the HotPlate page's HTML with the cursor in spbSave's caption -> the designer, spbSave
    // selected; nothing changed
    {
      const vdDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(page));
      const vdText = vdDoc.getText();
      const vdTag = vdText.search(/<[a-z]+[^>]*\sid="spbSave"/i);
      const vdEd = await vscode.window.showTextDocument(vdDoc, { preview: false });
      const vdPos = vdDoc.positionAt(vdText.indexOf('>', vdTag) + 1);
      vdEd.selection = new vscode.Selection(vdPos, vdPos);
      const vdR = await vscode.commands.executeCommand('ht9045Designer.viewDesigner');
      // (that page's designer -- which designer VS Code calls active a moment later is not the point)
      const vdOf = () => Array.from(hub.designers).find(x => web2.samePath(x.file, page)) || null;
      const vdD = await waitFor(() => { const x = vdOf(); return x && x.sel && x.sel.info && x.sel.info.id === 'spbSave' ? x : null; }, 15000);
      const vdX = vdOf();
      ok(!!(vdR && vdR.id === 'spbSave' && vdD && !vdDoc.isDirty),
        'Shift+F7 from the page\'s HTML (WPF View Designer): the designer with spbSave (the cursor in its caption) selected',
        JSON.stringify({ r: vdR, selected: vdX && vdX.sel && vdX.sel.info ? vdX.sel.info.id : null, lastSelId: vdX ? vdX.lastSelId : null,
          active: hub.active ? path.basename(hub.active.file) : null }));
      // 移到事件處理函式 (WPF Navigate to Event Handler) from the same place: spbSave's OnClick code opens (nothing added)
      const geEd = await vscode.window.showTextDocument(vdDoc, { preview: false });
      geEd.selection = new vscode.Selection(vdPos, vdPos);
      const geR = await vscode.commands.executeCommand('ht9045Designer.gotoEventHandler');
      const geCode = await waitFor(() => { const e = vscode.window.activeTextEditor; return e && /\.(cpp|h)$/i.test(e.document.uri.fsPath) ? e : null; }, 15000);
      ok(!!(geR && geR.id === 'spbSave' && geCode && !vdDoc.isDirty),
        'Navigate to Event Handler from the page\'s HTML (WPF): the cursor in spbSave -> its OnClick\'s C++ opens, nothing changed',
        JSON.stringify({ r: geR, opened: geCode ? path.basename(geCode.document.uri.fsPath) + ':' + (geCode.selection.active.line + 1) : null }));
      const vdTabs = vscode.window.tabGroups.all.flatMap(g => g.tabs).filter(t => t.input instanceof vscode.TabInputText && t.input.uri.fsPath.toLowerCase() === page.toLowerCase() && !t.isDirty);
      if (vdTabs.length) await vscode.window.tabGroups.close(vdTabs);
    }
    out.push('');
    out.push('output channel:');
    out.push('      ' + hub.out.name);
  } catch (e) {
    fail++;
    out.push('CRASH ' + (e && e.stack || e));
  } finally {
    saveSub.dispose();
    ok(saves.length === 0, 'NO-SAVES: the test saved no file', saves.join(', '));
    out.push('');
    out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
    if (report) fs.writeFileSync(report, out.join('\n') + '\n', 'utf8');
  }
  if (fail) throw new Error(fail + ' check(s) failed');
};
