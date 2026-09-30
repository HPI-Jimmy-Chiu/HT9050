'use strict';
// AI(W906-HTDESIGNER) 20260929: tests for lib\*.js against the real trees on this machine.
// No npm here, so run it with VS Code's own Electron as Node:
//   powershell -File tools\vscode-htdesigner\test\run_tests.ps1
// Read-only: nothing below writes anywhere except the report file given as argv[2].

const fs = require('fs');
const path = require('path');
const roots = require('../lib/roots');
const pageinfo = require('../lib/pageinfo');
const lex = require('../lib/cpplex');
const { SourceTree } = require('../lib/cppindex');
const web = require('../lib/websearch');
const fmt = require('../lib/format');

const out = [];
let pass = 0, fail = 0;
function log(s) { out.push(s); }
function ok(cond, name, extra) {
  if (cond) { pass++; log('PASS  ' + name + (extra ? '   ' + extra : '')); }
  else { fail++; log('FAIL  ' + name + (extra ? '   ' + extra : '')); }
}
function rel(p, base) { return base ? path.relative(base, p) : p; }

async function main() {
  const PORT = path.resolve(__dirname, '..', '..', '..');
  const WEB = path.resolve(PORT, '..', 'web');
  const PAGE = path.join(WEB, 'page', 'Setup.HotPlate.html');

  // --- roots
  const r = roots.resolveRoots(PAGE, [PORT], {});
  log('roots: ' + JSON.stringify(r));
  ok(web.samePath(r.webRoot, WEB), 'webRoot of a page');
  ok(web.samePath(r.portRoot, PORT), 'portRoot from workspace');
  ok(!!r.irRoot && fs.existsSync(r.irRoot), 'irRoot exists');
  ok(!!r.goldenRoot && /HT9011UC_Code_V/i.test(r.goldenRoot), 'goldenRoot found', r.goldenRoot);
  const r2 = roots.resolveRoots(null, [PORT], {});
  ok(web.samePath(r2.webRoot, WEB), 'webRoot without a page (command palette)', r2.webRoot);
  // another software built the same way, with names of its own: found by the folders' shape
  const oth = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_roots_'));
  try {
    const mk = p => fs.mkdirSync(path.join(oth, p), { recursive: true });
    mk('Proj/web/page'); mk('Proj/Mach7_Port/tools/dfm2rc/ir_out'); mk('Proj/Mach7_Port/forms'); mk('Proj/Mach7_BCB6'); mk('Proj/notes');
    fs.writeFileSync(path.join(oth, 'Proj/web/page/Main.html'), '<html></html>');
    fs.writeFileSync(path.join(oth, 'Proj/Mach7_BCB6/main.dfm'), 'object fMain: TfMain\nend\n');
    const ro = roots.resolveRoots(path.join(oth, 'Proj/web/page/Main.html'), [], {});
    ok(ro.webRoot && /Proj[\\/]web$/.test(ro.webRoot) && ro.portRoot && /Mach7_Port$/.test(ro.portRoot) && ro.irRoot && ro.goldenRoot && /Mach7_BCB6$/.test(ro.goldenRoot),
      'roots of another software (Mach7_Port / Mach7_BCB6, no HT9011UC names): found by their shape', JSON.stringify(ro));
  } finally { try { fs.rmSync(oth, { recursive: true, force: true }); } catch (e) { /* ignore */ } }

  // --- title / IR
  const html = fs.readFileSync(PAGE, 'utf8');
  const t = pageinfo.parseTitle(html);
  ok(t && t.dfm === 'cHotPlate' && t.cls === 'TfHotPlate', 'parseTitle HotPlate', JSON.stringify(t));
  const tm = pageinfo.parseTitle(fs.readFileSync(path.join(WEB, 'page', 'main.html'), 'utf8'));
  ok(tm && tm.dfm === 'main' && tm.cls === 'TMainForm', 'parseTitle main.html', JSON.stringify(tm));
  const ta = pageinfo.parseTitle(fs.readFileSync(path.join(WEB, 'page', 'Main.AOAInfo.html'), 'utf8'));
  ok(ta && ta.dfm === 'main', 'parseTitle Main.AOAInfo (main.dfm ts1)', JSON.stringify(ta));

  const ir = new pageinfo.IrStore(r.irRoot);
  const f = ir.find('cHotPlate', 'TfHotPlate');
  ok(!!f, 'IR file for cHotPlate', f && rel(f, r.irRoot));
  const d = ir.load(f);
  const sp = d.byName.get('spbSave');
  ok(d.formClass === 'TfHotPlate' && sp && sp.events.OnClick === 'spbSaveClick', 'IR spbSave OnClick = spbSaveClick');
  ok(d.root && d.root.events && d.root.events.OnShow === 'FormShow', 'IR root OnShow = FormShow');

  const scripts = pageinfo.listScripts(html, path.dirname(PAGE));
  ok(scripts.some(s => /ht9045_hotplate_wire\.js$/i.test(s)), 'listScripts has the wire script', scripts.map(s => path.basename(s)).join(', '));

  const mv = fs.readFileSync(path.join(WEB, 'page', 'Main.MotionView.html'), 'utf8');
  const t0 = Date.now();
  const g = ir.guess(pageinfo.collectIds(mv));
  log('guess Main.MotionView.html -> ' + (g ? rel(g.file, r.irRoot) + ' hits=' + g.hits + ' score=' + g.score.toFixed(2) : 'none') + ' (' + (Date.now() - t0) + ' ms incl. name index)');

  // --- cpplex
  const sample = [
    'int a;',                  // 1
    '#if 0',                   // 2
    'void dead1() {}',         // 3
    '/* #endif inside a comment */', // 4
    'const char* s = "#endif";',     // 5
    'void dead2() {}',         // 6
    '#else',                   // 7
    'void live1() {}',         // 8
    '#endif',                  // 9
    '#if 1',                   // 10
    'void live2() {}',         // 11
    '#else',                   // 12
    'void dead3() {}',         // 13
    '#endif',                  // 14
    '#ifdef SOFT_SIMULTE',     // 15
    'void cond() {}',          // 16
    '#endif',                  // 17
    'void after() {}',         // 18
  ].join('\n');
  const an = lex.analyze(sample);
  const deadLines = [];
  for (let i = 0; i < an.dead.length; i++) if (an.dead[i]) deadLines.push(i + 1);
  ok(deadLines.join(',') === '3,4,5,6,13', 'cpplex dead lines (comment/string #endif ignored)', deadLines.join(','));
  const anCr = lex.analyze(sample.replace(/\n/g, '\r\n'));
  const deadCr = [];
  for (let i = 0; i < anCr.dead.length; i++) if (anCr.dead[i]) deadCr.push(i + 1);
  ok(deadCr.join(',') === '3,4,5,6,13', 'cpplex dead lines, CRLF file', deadCr.join(','));

  // wb_serve.cpp (CRLF): a live function after a closed #if 0 must stay live
  const wbs = path.join(PORT, 'tools', 'wb_serve.cpp');
  if (fs.existsSync(wbs)) {
    const wt = fs.readFileSync(wbs).toString('latin1');
    const wa = lex.analyze(wt);
    const at = wt.indexOf('void W906_DispatchIoClick(webbridge::WebBridgeServer& server');
    const wl = lex.lineIndexOf(wa.starts, at);
    let deadTotal = 0;
    for (let i = 0; i < wa.dead.length; i++) deadTotal += wa.dead[i];
    ok(at > 0 && !wa.dead[wl], 'wb_serve.cpp (CRLF): W906_DispatchIoClick definition is live', 'line ' + (wl + 1) + ', dead lines in file ' + deadTotal + '/' + wa.dead.length);
  }

  const csys = path.join(PORT, 'csystem.cpp');
  if (fs.existsSync(csys)) {
    const txt = fs.readFileSync(csys).toString('latin1');
    const a2 = lex.analyze(txt);
    // definitions only ("void DoAllProcess()" with no ';' -- the forward declaration
    // near the top is live and must not be counted; before the CRLF fix it was, and
    // that hid the live definition being reported dead)
    const defs = [];
    const re = /^[ \t]*void[ \t]+DoAllProcess[ \t]*\([^)]*\)[ \t]*\r?$/gm;
    let m;
    while ((m = re.exec(txt))) {
      const li = lex.lineIndexOf(a2.starts, m.index);
      defs.push((li + 1) + (a2.dead[li] ? ':dead' : ':live'));
    }
    ok(defs.filter(x => x.endsWith(':live')).length === 1 && defs.some(x => x.endsWith(':dead')),
      'csystem.cpp DoAllProcess: exactly one live definition, the golden copy dead', defs.join(' '));
  }

  // --- C++ index (port)
  const port = new SourceTree(r.portRoot, 'port');
  let t1 = Date.now();
  await port.ensure();
  log('port index: ' + port.files.length + ' files, ' + port.qual.size + ' qualified names, ' + (Date.now() - t1) + ' ms');
  const ex = await port.lookup(['TfHotPlate'], 'sbtExitClick');
  const exDef = ex.find(h => h.def && !h.dead);
  ok(exDef && /forms[\\/]fHotPlate\.cpp$/i.test(exDef.file), 'port sbtExitClick -> forms\\fHotPlate.cpp (live def)',
    exDef ? rel(exDef.file, PORT) + ':' + exDef.line + '  ' + exDef.snippet : JSON.stringify(ex.slice(0, 3)));
  const sv = await port.lookup(['TfHotPlate'], 'spbSaveClick');
  const svm = sv.find(h => h.comment && /HotPlateForm_File\.cpp$/i.test(h.file));
  ok(!!svm, 'port spbSaveClick -> comment mention in FileRW\\HotPlateForm_File.cpp',
    sv.slice(0, 4).map(h => rel(h.file, PORT) + ':' + h.line + (h.def ? ' def' : '') + (h.comment ? ' comment' : '') + (h.dead ? ' dead' : '')).join(' | '));

  // --- C++ index (golden, Big5)
  const gold = new SourceTree(r.goldenRoot, 'golden');
  t1 = Date.now();
  await gold.ensure();
  log('golden index: ' + gold.files.length + ' files, ' + gold.qual.size + ' qualified names, ' + (Date.now() - t1) + ' ms');
  const gs = await gold.lookup(['TfHotPlate'], 'spbSaveClick');
  const gdef = gs.find(h => h.def);
  ok(gdef && /cHotPlate\.cpp$/i.test(gdef.file) && gdef.line === 440, 'golden spbSaveClick -> cHotPlate.cpp:440 def',
    gdef ? rel(gdef.file, r.goldenRoot) + ':' + gdef.line + '  ' + gdef.snippet : JSON.stringify(gs.slice(0, 3)));
  const gshow = await gold.lookup(['TfHotPlate'], 'FormShow');
  ok(gshow.some(h => h.def), 'golden FormShow found', gshow.filter(h => h.def).map(h => rel(h.file, r.goldenRoot) + ':' + h.line).join(' '));
  // Big5 decode of a line with Chinese (any golden hit whose snippet has CJK)
  const anyCjk = gshow.concat(gs).find(h => /[\u4e00-\u9fff]/.test(h.snippet));
  log('golden snippet sample: ' + (anyCjk ? anyCjk.snippet : '(no CJK on these lines)'));

  // --- command string -> C++
  t1 = Date.now();
  const cs = await port.findString('io.btnPanelClick');
  ok(cs.length && cs[0].dispatch && /wb_serve\.cpp$/i.test(cs[0].file), 'cmd io.btnPanelClick -> wb_serve.cpp dispatch first',
    cs.slice(0, 3).map(h => rel(h.file, PORT) + ':' + h.line).join(' | ') + ' (' + (Date.now() - t1) + ' ms)');

  // --- web search
  const srcs = [new web.Source(PAGE, html, true)].concat(scripts.map(s => new web.Source(s, web.readText(s), false)));
  const men = web.findAll(srcs, web.idMentionRe('spbSave'), 20);
  ok(men.some(h => /ht9045_hotplate_wire\.js$/i.test(h.file)), 'id mention spbSave in wire script',
    men.slice(0, 4).map(h => path.basename(h.file) + ':' + h.line).join(' | '));
  ok(!men.some(h => h.file === PAGE && /<button/.test(h.snippet)), 'id mention ignores the element markup itself');
  // locateFn: take a real function from the page's inline script
  const fnSrc = "function(b){\n  b.style.cursor='pointer';";
  const inl = html.replace(/\r\n/g, '\n');
  const at = inl.indexOf(fnSrc);
  if (at >= 0) {
    const end = inl.indexOf('\n});', at);
    const fnText = inl.slice(at, end + 2);
    const loc = web.locateFn(srcs, fnText, null);
    ok(loc && loc.file === PAGE, 'locateFn finds an inline-script function (LF vs CRLF)', loc ? path.basename(loc.file) + ':' + loc.line : 'null');
  }
  ok(web.extractCmds("R.rawCmd('io.btnPanelClick', {tag:a})").join() === 'io.btnPanelClick', 'extractCmds rawCmd');
  ok(web.mapFrameUrl('https://file%2B.vscode-resource.vscode-cdn.net/d%3A/HT9045/_integ_ioweb/web/page/x.js') === path.normalize('d:/HT9045/_integ_ioweb/web/page/x.js'), 'mapFrameUrl vscode-resource');
  ok(web.mapFrameUrl('vscode-webview://abc/index.html?id=1') === null, 'mapFrameUrl webview doc = inline');
  const oi = html.indexOf('id="XST1"');
  ok(web.idAtOffset(html, oi + 3) === 'XST1', 'idAtOffset inside a tag');
  ok(web.idAtOffset(html, html.indexOf('class="form"') + 2) === '@form', 'idAtOffset form root');

  // --- DFM property lines (golden .dfm, Big5)
  const dfmFile = path.join(r.goldenRoot, d.sourceDfm);
  const dfmLines = require('../lib/cppindex').decodeBig5(fs.readFileSync(dfmFile)).split('\n').map(l => l.replace(/\r$/, ''));
  ok(/^\s*object spbSave: TSpeedButton/.test(dfmLines[sp.line - 1]), 'IR line -> "object spbSave: TSpeedButton" in the .dfm', 'line ' + sp.line);
  const pl = pageinfo.dfmPropLines(dfmLines, sp.line);
  ok(pl.Caption && /^\s*Caption\s*=/.test(dfmLines[pl.Caption - 1]) && pl.OnClick && /spbSaveClick/.test(dfmLines[pl.OnClick - 1]),
    'dfmPropLines: Caption / OnClick lines', 'Caption=' + pl.Caption + ' OnClick=' + pl.OnClick);
  const g1 = d.byName.get('GroupBox1');
  const pg = pageinfo.dfmPropLines(dfmLines, g1.line);
  ok(pg.Caption && !Object.prototype.hasOwnProperty.call(pg, 'Picture.Data'), 'dfmPropLines skips nested objects', 'GroupBox1.Caption=' + pg.Caption);

  // --- web: follow the save handler to the command it sends
  const wire = srcs.find(s => /ht9045_hotplate_wire\.js$/i.test(s.file));
  const hStart = wire.text.indexOf("document.addEventListener('click', function (ev)");
  const hOpen = wire.text.indexOf('{', wire.text.indexOf('function (ev)', hStart));
  const hText = wire.text.slice(wire.text.indexOf('function (ev)', hStart), web.braceEnd(wire.text, hOpen));
  const tr = web.traceCmds(srcs, hText, 3, wire.file);
  ok(tr.some(t => t.cmd === 'recipe.doc.put') && !tr.some(t => t.cmd === 'editlist.save'),
    'traceCmds: save handler -> own save() -> ... -> cmd(recipe.doc.put), not other pages\' save()',
    tr.map(t => t.cmd + ' via ' + t.via.join('>')).join(' | '));
  ok(web.extractCmds("return cmd('form.save', {tag:p}); raw('control.acquire'); run('act.main.x'); x('a.html')").join() === 'form.save,control.acquire,act.main.x',
    'extractCmds: cmd( raw( run(, file names ignored');
  const sends = web.findCmdSend(srcs, 'recipe.doc.put');
  ok(sends.some(h => /ht9045_recipe_client\.js$/i.test(h.file)), 'findCmdSend recipe.doc.put -> recipe client', sends.map(h => path.basename(h.file) + ':' + h.line).join(' '));

  // --- C++: dispatch line -> handler function -> its definition
  const disp = (await port.findString('io.btnPanelClick')).find(h => h.dispatch && /W906_DispatchIoClick/.test(h.snippet));
  const called = disp ? await port.calledFuncsAt(disp.file, disp.off) : [];
  ok(called.includes('W906_DispatchIoClick'), 'calledFuncsAt: dispatch branch -> W906_DispatchIoClick', called.join(','));
  const fdef = await port.findFuncDefs('W906_DispatchIoClick');
  ok(fdef.length && fdef[0].def && !fdef[0].dead && /wb_serve\.cpp$/i.test(fdef[0].file), 'findFuncDefs W906_DispatchIoClick (live, in wb_serve.cpp)',
    fdef.map(h => rel(h.file, PORT) + ':' + h.line + (h.dead ? ' dead' : '')).join(' '));
  const rp = (await port.findString('recipe.doc.put')).find(h => h.dispatch);
  const rpCalled = rp ? await port.calledFuncsAt(rp.file, rp.off) : [];
  ok(rp && rpCalled.length === 0, 'calledFuncsAt: block-style dispatch (recipe.doc.put) -> no "handler" (the block is the handler)',
    (rp ? rel(rp.file, PORT) + ':' + rp.line : '-') + ' calls [' + rpCalled.join(',') + ']');
  // like the extension: the first three dispatch hits (an outer grouping "if" first,
  // then the inner "else if (wc.cmd == "counterclear.click") ccRes = W906_X(…)")
  const ccHits = (await port.findString('counterclear.click')).filter(h => h.dispatch && !h.test).slice(0, 3);
  const ccCalled = [];
  for (const h of ccHits) ccCalled.push(...(await port.calledFuncsAt(h.file, h.off)));
  ok(ccCalled.includes('W906_CounterClearClick'), 'calledFuncsAt: "else if (…) r = W906_X(…)" -> W906_X',
    ccHits.map(h => rel(h.file, PORT) + ':' + h.line).join(' ') + ' -> ' + ccCalled.join(','));
  const ar = (await port.findString('act.main.clearRecord')).find(h => h.dispatch);
  const arCalled = ar ? await port.calledFuncsAt(ar.file, ar.off) : [];
  ok(arCalled[0] === 'DoClearRecordAction', 'calledFuncsAt: "if (…) return DoX(p);" -> DoX', arCalled.join(','));
  // several branches on one long line: only this command's branch
  const multi = (await port.findString('io.btnPanelClick')).find(h => h.dispatch && /W906_SimDiCommand/.test(h.snippet));
  if (multi) {
    const mc = await port.calledFuncsAt(multi.file, multi.off);
    ok(!mc.includes('W906_SimDiCommand'), 'calledFuncsAt: stops at the other branches of a long line', rel(multi.file, PORT) + ':' + multi.line + ' -> ' + mc.join(','));
  }

  // --- where the form code uses a control
  const gu = await gold.findMemberUses(['TfHotPlate'], 'spbSave', 50);
  ok(gu.some(h => !h.decl && /spbSave\s*->/.test(h.snippet)) && gu.some(h => h.decl), 'golden uses of spbSave (use + declaration)',
    gu.slice(0, 4).map(h => path.basename(h.file) + ':' + h.line + (h.decl ? ' decl' : '')).join(' | '));
  const pu = await port.findMemberUses(['TfHotPlate'], 'XST1', 50);
  ok(pu.length > 0, 'port uses of XST1', pu.slice(0, 4).map(h => rel(h.file, PORT) + ':' + h.line + (h.comment ? ' comment' : '') + (h.dead ? ' dead' : '')).join(' | '));

  // --- code -> designer (reverse index)
  const { ReverseIndex, enclosingMethod } = require('../lib/reverse');
  const rev = new ReverseIndex(ir, [path.join(WEB, 'page'), WEB]);
  t1 = Date.now();
  rev.build();
  log('reverse index: ' + rev.pages.length + ' pages, ' + rev.handlers.size + ' handler keys, ' + rev.controls.size + ' controls, ' + (Date.now() - t1) + ' ms');
  const rh = rev.handler('TfHotPlate', 'spbSaveClick');
  ok(rh.length === 1 && /Setup\.HotPlate\.html$/.test(rh[0].page) && rh[0].node === 'spbSave' && rh[0].event === 'OnClick',
    'reverse: TfHotPlate::spbSaveClick -> Setup.HotPlate.html spbSave.OnClick', rh.map(e => path.basename(e.page) + ' ' + e.node + '.' + e.event).join(' | '));
  const rk = rev.handler('TfHotPlate', 'XST1KeyPress');
  ok(rk.length >= 3 && rk.every(e => e.event === 'OnKeyPress'), 'reverse: a handler shared by several controls', rk.map(e => e.node).join(','));
  const rs = rev.handler('TfHotPlate', 'FormShow');
  ok(rs.length && rs[0].node === '@form' && rs[0].event === 'OnShow', 'reverse: FormShow -> the form itself');
  ok(rev.control('TfHotPlate', 'XST1').length === 1, 'reverse: control TfHotPlate#XST1');
  // main.dfm is shared by several pages: every entry's page must really have that control
  let checked = 0, wrong = [];
  const pageText = new Map();
  for (const [k, list] of rev.handlers) {
    if (!k.startsWith('TfMain::')) continue;
    for (const e of list) {
      if (e.node === '@form') continue;
      if (!pageText.has(e.page)) pageText.set(e.page, fs.readFileSync(e.page, 'utf8'));
      checked++;
      if (!pageText.get(e.page).includes('id="' + e.node + '"')) wrong.push(path.basename(e.page) + ':' + e.node);
    }
  }
  const mainPages = new Set();
  for (const [k, list] of rev.handlers) if (k.startsWith('TfMain::')) list.forEach(e => mainPages.add(path.basename(e.page)));
  ok(checked > 0 && wrong.length === 0, 'reverse: TfMain entries only point at pages that contain the control',
    checked + ' entries over ' + mainPages.size + ' pages (' + Array.from(mainPages).join(', ') + ')' + (wrong.length ? ' WRONG ' + wrong.slice(0, 5).join(' ') : ''));
  const gtxt = require('../lib/cppindex').decodeBig5(fs.readFileSync(path.join(r.goldenRoot, 'cHotPlate.cpp')));
  const inSave = gtxt.indexOf('\n', gtxt.indexOf('TfHotPlate::spbSaveClick')) + 40;
  const em = enclosingMethod(gtxt, inSave, lex.isDefinitionAt);
  ok(em && em.cls === 'TfHotPlate' && em.method === 'spbSaveClick', 'enclosingMethod: cursor inside the BCB6 spbSaveClick body', em ? em.cls + '::' + em.method : 'null');
  const ptxt = fs.readFileSync(path.join(PORT, 'forms', 'fHotPlate.cpp'), 'utf8');
  const inExit = ptxt.indexOf('sbtExit->Down = false;');
  const em2 = enclosingMethod(ptxt, inExit, lex.isDefinitionAt);
  ok(em2 && em2.method === 'sbtExitClick', 'enclosingMethod: cursor inside the port sbtExitClick body', em2 ? em2.cls + '::' + em2.method : 'null');
  const em3 = enclosingMethod(ptxt, ptxt.length - 5, lex.isDefinitionAt);
  log('      enclosingMethod at end of fHotPlate.cpp: ' + (em3 ? em3.cls + '::' + em3.method : 'none'));

  // --- C++ -> web for commands
  const { WebCmdIndex } = require('../lib/webcmds');
  const wci = new WebCmdIndex(WEB);
  t1 = Date.now();
  wci.build();
  log('web command index: ' + wci.files + ' files, ' + wci.sends.size + ' commands, ' + (Date.now() - t1) + ' ms');
  const ws1 = wci.senders('io.btnPanelClick');
  ok(ws1.some(h => /ht9045_io_do\.js$/i.test(h.file) && h.pages.some(p => /HW\.IoSetView\.html$/i.test(p))) &&
    !ws1.some(h => /ht9045_io_do\.js$/i.test(h.file) && h.line === 9),
    'web senders of io.btnPanelClick: ht9045_io_do.js, loaded by HW.IoSetView.html (not the doc comment at :9)',
    ws1.map(h => path.basename(h.file) + ':' + h.line + ' [' + h.pages.map(p => path.basename(p)).join(',') + ']').join(' | '));
  const ws2 = wci.senders('recipe.doc.put');
  ok(ws2.some(h => h.pages.some(p => /Setup\.HotPlate\.html$/i.test(p))), 'web senders of recipe.doc.put include a script HotPlate loads',
    ws2.map(h => path.basename(h.file) + ':' + h.line + ' (' + h.pages.length + ' pages)').join(' | '));

  // --- field maps (XST1: ['Hotplate Form', 'X Start'])
  log('field maps: ' + wci.fields.size + ' distinct [section] keys, ' + wci.fieldsById.size + ' controls');
  const fx = wci.fieldsOf(PAGE, 'XST1');
  ok(fx.length >= 2 && fx.every(r => r.section === 'Hotplate Form' && r.key === 'X Start') && fx.some(r => r.doc === 'hotPlate'),
    'fieldsOf(HotPlate, XST1) -> [Hotplate Form] X Start in both the hand-written and the generated (doc, section, key) wire',
    fx.map(r => (r.doc ? r.doc + ' ' : '') + '[' + r.section + '] ' + r.key + ' @' + path.basename(r.file) + ':' + r.line).join(' | '));
  const fc = wci.fieldControls('Hotplate Form', 'X Start');
  ok(fc.some(r => r.id === 'XST1' && r.pages.some(p => /Setup\.HotPlate\.html$/i.test(p))), 'fieldControls([Hotplate Form] X Start) -> XST1 on Setup.HotPlate.html',
    fc.map(r => r.id + ' [' + r.pages.map(p => path.basename(p)).join(',') + ']').join(' | '));
  const gx = (await gold.findString('X Start')).filter(h => h.snippet.includes('"Hotplate Form"'));
  ok(gx.some(h => /cHotPlate\.cpp$/i.test(h.file) && /WriteIniData|ReadIniData/.test(h.snippet)), 'BCB6 reads/writes [Hotplate Form] X Start in cHotPlate.cpp',
    gx.slice(0, 3).map(h => path.basename(h.file) + ':' + h.line).join(' | '));

  // --- tag maps ('machine.state': ['palMainStatus', 'text'])
  log('tag maps: ' + wci.tags.size + ' tags, ' + wci.tagsById.size + ' controls');
  const MAIN = path.join(WEB, 'page', 'main.html');
  const tg = wci.tagsOf(MAIN, 'palMainStatus');
  ok(tg.some(r => r.tag === 'machine.state' && r.prop === 'text'), 'tagsOf(main.html, palMainStatus) -> machine.state (text)',
    tg.map(r => r.tag + '/' + r.prop + ' @' + path.basename(r.file) + ':' + r.line).join(' | '));
  const tc = wci.tagControls('machine.state');
  ok(tc.some(r => r.id === 'palMainStatus' && r.pages.some(p => /main\.html$/i.test(p))), 'tagControls(machine.state) -> palMainStatus on main.html',
    tc.map(r => r.id + ' [' + r.pages.map(p => path.basename(p)).join(',') + ']').join(' | '));
  const tp = (await port.findString('machine.state')).filter(h => !h.test && !h.comment);
  ok(tp.length > 0, 'C++ lines with "machine.state"', tp.slice(0, 4).map(h => rel(h.file, PORT) + ':' + h.line).join(' | '));

  // --- wiring overview (HotPlate, no runtime listeners: static signals only)
  const ovm = require('../lib/overview');
  const fieldIdsHP = new Set(Array.from(wci.fieldsById.keys()).filter(id => wci.fieldsOf(PAGE, id).length));
  t1 = Date.now();
  const ov = await ovm.buildOverview({
    ir: d, pageIds: new Set(pageinfo.collectIds(html)), listeners: { sbtExit: ['click'] },
    fieldIds: fieldIdsHP, tagIds: new Set(), quoted: ovm.quotedNames(srcs), classes: ['TfHotPlate'], port, gold,
  });
  const evOf = (id, ev) => { const r = ov.rows.find(x => x.id === id); return r && r.events.find(e => e.name === ev); };
  log('overview HotPlate: ' + JSON.stringify(ov.summary) + ' (' + (Date.now() - t1) + ' ms)');
  const e1 = evOf('sbtExit', 'OnClick'), e2 = evOf('spbSave', 'OnClick'), e3 = evOf('XST1', 'OnKeyPress'), e4 = evOf('@form', 'OnShow');
  ok(e1 && e1.web === 'yes' && e1.port.state === 'live' && /fHotPlate\.cpp$/i.test(e1.port.at.file), 'overview sbtExit.OnClick: web ✔ (listener), C++ ✔ forms\\fHotPlate.cpp',
    e1 ? e1.web + ' / ' + e1.port.state + ' ' + path.basename(e1.port.at.file) + ':' + e1.port.at.line : '-');
  ok(e2 && e2.web === 'weak' && e2.port.state === 'mention' && e2.golden.state === 'live', 'overview spbSave.OnClick: web △ (delegated), C++ only a comment (moved), BCB6 ✔',
    e2 ? e2.web + ' / ' + e2.port.state + ' / ' + e2.golden.state : '-');
  ok(e3 && e3.web === 'field', 'overview XST1.OnKeyPress: web ✔ via the field map', e3 ? e3.web : '-');
  ok(e4 && e4.web === 'na' && e4.golden.state === 'live', 'overview form OnShow: n/a on the web, BCB6 ✔');

  // --- source edits (WPF-style), on the real HotPlate markup, in memory only
  const he = require('../lib/htmledit');
  const bt = he.startTagOf(html, 'spbSave');
  ok(bt && bt.name === 'button' && /id="spbSave"/.test(bt.text), 'startTagOf spbSave', bt ? bt.text.slice(0, 60) : '-');
  const moved = he.setStyle(bt.text, { left: '60px', top: '9px' });
  ok(/left:60px;/.test(moved) && /top:9px;/.test(moved) && /width:227px;/.test(moved) && moved.length - bt.text.length < 4,
    'setStyle moves spbSave, keeps the rest', moved.slice(moved.indexOf('style='), moved.indexOf('style=') + 90));
  const x1 = he.startTagOf(html, 'XST1');
  const wr = he.wrapperTagOf(html, x1.start);
  ok(wr && wr.name === 'span' && /position:absolute;left:129px/.test(wr.text), 'wrapperTagOf XST1 -> the positioning <span>', wr ? wr.text : '-');
  const hid = he.setStyle(x1.text, { display: 'none' });
  ok(/display:none;/.test(hid) && he.setStyle(hid, { display: null }) === x1.text, 'setStyle add then remove display:none restores the tag byte for byte');
  // hand-written style with spaces and no final ';': only the changed value moves
  const hw = '<div id="a" style="position: absolute; left: 10px;  top : 5px">';
  ok(he.setStyle(hw, { left: '12px' }) === '<div id="a" style="position: absolute; left: 12px;  top : 5px">', 'setStyle edits one value in place (spaces kept)');
  ok(he.setStyle(hw, { width: '30px' }) === '<div id="a" style="position: absolute; left: 10px;  top : 5px;width:30px;">', 'setStyle appends a new property after a missing ";"');
  ok(he.setStyle('<p id="b">', { left: '1px' }) === '<p id="b" style="left:1px;">', 'setStyle adds a style attribute when there is none');
  ok(he.setStyle('<p id="c" style="left:1px;top:2px;left:3px;">', { left: '9px' }) === '<p id="c" style="top:2px;left:9px;">', 'setStyle collapses duplicates onto the last one');
  const dis = he.setAttr(x1.text, 'disabled', true);
  ok(/ disabled>$/.test(dis) || / disabled\s*\/?>$/.test(dis), 'setAttr disabled', dis.slice(-30));
  ok(he.setAttr(dis, 'disabled', null) === x1.text, 'setAttr remove restores the tag byte for byte');
  const vset = he.setAttr(x1.text, 'value', '12.5');
  ok(/value="12\.5"/.test(vset) && !/value=""/.test(vset), 'setAttr value replaces the old value', vset.slice(vset.indexOf('value'), vset.indexOf('value') + 14));
  const cr = he.captionRange(html, bt, 'text');
  ok(cr && html.slice(cr[0], cr[1]) === 'Save', 'captionRange(spbSave) = "Save" (after the <img>)', cr ? JSON.stringify(html.slice(cr[0], cr[1])) : '-');
  const gb = he.startTagOf(html, 'GroupBox1');
  const lr = he.captionRange(html, gb, 'legend');
  ok(lr && html.slice(lr[0], lr[1]) === 'Plate of Use', 'captionRange(GroupBox1, legend) = "Plate of Use"', lr ? JSON.stringify(html.slice(lr[0], lr[1])) : '-');
  const ck = he.startTagOf(html, 'cbEnableHP2');
  const kr = he.captionRange(html, ck, 'text');
  ok(kr && html.slice(kr[0], kr[1]) === 'Plate 2', 'captionRange(checkbox label) = "Plate 2"', kr ? JSON.stringify(html.slice(kr[0], kr[1])) : '-');
  const pn = he.startTagOf(html, 'Panel1');
  const pr = he.captionRange(html, pn, 'pnlCap');
  ok(pr && pr[0] === pr[1], 'captionRange(Panel1, pnlCap) = empty insertion point', pr ? JSON.stringify(pr) : '-');
  const fr = he.startTagOf(html, '@form');
  ok(fr && /class="form"/.test(fr.text), 'startTagOf @form');

  // --- forwarding pages
  const bsn = fs.readFileSync(path.join(WEB, 'page', 'Setup.BinSelNormal.html'), 'utf8');
  ok(pageinfo.redirectTarget(bsn) === 'Setup.BinSel.html', 'redirectTarget: Setup.BinSelNormal.html -> Setup.BinSel.html');
  ok(pageinfo.redirectTarget(html) === null, 'redirectTarget: a real page is not a forwarder');
  ok(pageinfo.redirectTarget('<html><head><meta http-equiv="refresh" content="0; url=main.html"></head><body></body></html>') === 'main.html', 'redirectTarget: meta refresh');
  let fwd = 0;
  for (const n of fs.readdirSync(path.join(WEB, 'page')).filter(x => /\.html$/i.test(x))) {
    if (pageinfo.redirectTarget(fs.readFileSync(path.join(WEB, 'page', n), 'utf8'))) fwd++;
  }
  log('forwarding pages in web\\page: ' + fwd);

  // --- format
  ok(fmt.tcolor(12761254) === '#a6b8c2', 'TColor int -> #rrggbb (BGR)', fmt.tcolor(12761254));
  ok(fmt.tcolor('clBtnFace') === '#d4d0c8', 'TColor clBtnFace');
  ok(fmt.tcolor(-2147483633) === '#d4d0c8', 'TColor system colour index');
  ok(fmt.formatProp('Font.Height', { type: 'SCALAR', value: -11 })[3] === '約 8 pt', 'Font.Height hint');
  const v = fmt.parseVclTitle('spbSave : TSpeedButton（統一 save 樣式）');
  ok(v && v.cls === 'TSpeedButton' && v.note === '統一 save 樣式', 'parseVclTitle', JSON.stringify(v));
  ok(fmt.formatProps(sp.properties).every(p => !/^On[A-Z]/.test(p[0])), 'formatProps drops On* events');

  // --- compare with DFM / reset to DFM
  const dv = fmt.dfmEditValues(sp);
  ok(dv && dv.left === 54 && dv.top === 8 && dv.width === 227 && dv.height === 40 && dv.caption === 'Save',
    'dfmEditValues(spbSave): geometry + Caption', JSON.stringify(dv));
  const syn = fmt.dfmEditValues({ geometry: {}, properties: {
    Caption: { type: 'STR', value: '&Save && Exit' }, Color: { type: 'SCALAR', value: 'clBtnFace' },
    'Font.Color': { type: 'SCALAR', value: 255 }, 'Font.Style': { type: 'SET', value: ['fsBold'] },
    'Font.Height': { type: 'SCALAR', value: -13 }, Visible: { type: 'SCALAR', value: 'False' } } });
  ok(syn.caption === 'Save & Exit' && syn.background === null && syn.backgroundSys === 'clBtnFace' && syn.color === '#ff0000' &&
    syn.colorSys === null && syn.bold === true && syn.italic === false && syn.fontSize === 11 && syn.fontHeight === -13 && syn.visible === false &&
    syn.enabled === null && syn.left === null && syn.fontName === null,
    'dfmEditValues: & accelerator dropped, system colour by name only, Font.Height -13 -> 11px (the generator), only what the DFM says', JSON.stringify(syn));
  ok(fmt.sysColorName(-2147483633) === 'clBtnFace' && fmt.sysColorName('$8000000F') === 'clBtnFace' &&
    fmt.sysColorName('clRed') === null && fmt.sysColorName(255) === null, 'sysColorName: system vs fixed colours');
  ok(fmt.dfmEditValues(null) === null, 'dfmEditValues(null)');

  // --- the whole page against the DFM (what the probe's lookAll would answer)
  const dd = require('../lib/dfmdiff');
  const look0 = { visible: true, enabled: true, fontSize: 14, fontName: 'Arial', fontFamilies: ['Arial'], bold: true, italic: false, color: '#222222', background: '#ece9d8' };
  const pd = dd.diffPage([
    { id: 'spbSave', lay: { left: 54, top: 8, width: 227, height: 40, rendered: true, root: false }, cap: { kind: 'text', value: 'Save' }, look: look0 },
    { id: 'spbSave_not_in_dfm', lay: { left: 1, top: 1, width: 1, height: 1 }, cap: null, look: look0 },
    { id: d.root.name, lay: { left: 0, top: 0, width: 10, height: 10, root: true }, cap: null, look: look0 },
  ], d);
  // the page's 14px IS what the generator makes of Font.Height -16; bold is the save style
  ok(pd.controls === 1 && pd.rows.map(r => r.prop).join(',') === 'Font.Bold',
    'diffPage(spbSave): only Font.Bold (Font.Size 14 = |-16|-2); the form and unknown ids skipped', JSON.stringify(pd.rows.map(r => r.prop + ' ' + r.page + '/' + r.dfm)));
  const fbd = pd.rows[0];
  ok(fbd && fbd.page === 'True' && fbd.dfm === 'False' && fbd.group === 'look' && fbd.reset.type === 'setLook' && fbd.reset.prop === 'bold' && fbd.reset.value === false,
    'diffPage row: page True, DFM False, reset = setLook bold false', JSON.stringify(fbd));
  const pdSize = dd.diffPage([{ id: 'spbSave', lay: null, cap: null, look: Object.assign({}, look0, { fontSize: 16, bold: false }) }], d);
  ok(pdSize.rows.length === 1 && pdSize.rows[0].prop === 'Font.Size' && pdSize.rows[0].dfm === '14（Height -16）' && pdSize.rows[0].reset.value === 14,
    'Font.Size 16 on the page vs DFM Height -16 -> shown as "14（Height -16）", reset 14', JSON.stringify(pdSize.rows));
  const pdHidden = dd.diffPage([{ id: 'spbSave', lay: { left: 60, top: 8, width: 227, height: 40, rendered: false }, cap: { kind: 'text', value: 'Save' },
    look: Object.assign({}, look0, { fontSize: 14, bold: false }) }], d);
  ok(pdHidden.rows.length === 1 && pdHidden.rows[0].prop === 'Left' && pdHidden.rows[0].reset.left === 54 && /沒有顯示/.test(pdHidden.rows[0].note) && pdHidden.byGroup.layout === 1,
    'diffPage: a moved, hidden control -> Left 60/54, reset setLayout left 54, measured by style', JSON.stringify(pdHidden.rows));
  ok(dd.same(14, '14') && dd.same('#AABBCC', '#aabbcc') && !dd.same(true, false) && dd.same(' Save ', 'Save'), 'dfmdiff.same');
  // an AutoSize label: the source gives no width/height -> its size is the text's, not compared
  const raw0 = { left: '54px', top: '8px', width: '', height: '', right: '', bottom: '' };
  const pdAuto = dd.diffPage([{ id: 'spbSave', lay: { left: 54, top: 8, width: 126, height: 22, rendered: true, raw: raw0 }, cap: { kind: 'text', value: 'Save' },
    look: Object.assign({}, look0, { fontSize: 14, bold: false }) }], d);
  ok(pdAuto.rows.length === 0 && !dd.sizeInSource(raw0, 'width') && dd.sizeInSource(Object.assign({}, raw0, { right: '3px' }), 'width') &&
    dd.sizeInSource(Object.assign({}, raw0, { height: '40px' }), 'height') && !dd.sizeInSource(Object.assign({}, raw0, { width: 'auto' }), 'width'),
    'a size the source does not give (AutoSize label) is not compared; left+right anchoring counts as given', JSON.stringify(pdAuto.rows));
  // a wrapper (span.lled) moves the origin by (10, 37): compared in the DFM's coordinates, reset in the element's own
  const pdShift = dd.diffPage([{ id: 'spbSave', lay: { left: 44, top: -29, width: 227, height: 40, rendered: true, ox: 10, oy: 37 }, cap: null,
    look: Object.assign({}, look0, { fontSize: 14, bold: false }) }], d);
  const pdShift2 = dd.diffPage([{ id: 'spbSave', lay: { left: 50, top: -29, width: 227, height: 40, rendered: true, ox: 10, oy: 37 }, cap: null,
    look: Object.assign({}, look0, { fontSize: 14, bold: false }) }], d);
  ok(pdShift.rows.length === 0 && pdShift2.rows.length === 1 && pdShift2.rows[0].page === '60' && pdShift2.rows[0].reset.left === 44 && /位移 10/.test(pdShift2.rows[0].note),
    'origin shift: 44+10 = DFM 54 is the same; 50+10 = 60 differs, reset writes 54-10 = 44', JSON.stringify(pdShift2.rows));
  // the form's UI font, not given for this control: Font.Name not compared
  const pdFont = dd.diffPage([{ id: 'spbSave', lay: null, cap: null, look: Object.assign({}, look0, { fontSize: 14, bold: false, fontName: 'Microsoft JhengHei', fontFamilies: ['Microsoft JhengHei'], fontOwn: false }) },
  ], d);
  const pdFont2 = dd.diffPage([{ id: 'spbSave', lay: null, cap: null, look: Object.assign({}, look0, { fontSize: 14, bold: false, fontName: 'Segoe UI', fontFamilies: ['Segoe UI'], fontOwn: true }) },
  ], d);
  ok(pdFont.rows.length === 0 && pdFont2.rows.length === 1 && pdFont2.rows[0].prop === 'Font.Name',
    'Font.Name: inherited (the form UI font) not compared; its own font compared', JSON.stringify(pdFont2.rows));
  // the same difference on >= 5 controls of one class = one pattern line
  const many = [];
  for (const n of d.byName.values()) {
    if (n.class !== 'TLabel' || !n.properties['Font.Height']) continue;
    const dv0 = fmt.dfmEditValues(n);
    many.push({ id: n.name, lay: null, cap: null, look: Object.assign({}, look0, { fontSize: dv0.fontSize, bold: !dv0.bold, italic: !!dv0.italic, color: dv0.color || '#222222', fontOwn: false }) });
  }
  const pdPat = dd.diffPage(many, d);
  ok(many.length >= 5 && pdPat.patterns.length >= 1 && pdPat.patterns[0].cls === 'TLabel' && pdPat.patterns[0].prop === 'Font.Bold' && pdPat.single < pdPat.rows.length,
    'the same Font.Bold difference on every TLabel of HotPlate -> one pattern', many.length + ' labels, patterns ' + JSON.stringify(pdPat.patterns.map(p => p.cls + ' ' + p.prop + ' ' + p.page + '/' + p.dfm + ' x' + p.count)));
  // AutoSize: a TLabel is True unless the .dfm says False; other classes have none; the list compares it
  const asLabel = Array.from(d.byName.values()).find(n => n.class === 'TLabel' && !(n.properties || {}).AutoSize);
  const asNo = fmt.dfmEditValues({ class: 'TLabel', properties: { AutoSize: { value: 'False' } }, geometry: {} });
  const asRows = asLabel ? dd.diffPage([{ id: asLabel.name, lay: null, cap: null, look: Object.assign({}, look0, { autoSize: false, fontOwn: false }) }], d).rows.filter(r => r.prop === 'AutoSize') : [];
  ok(asLabel && fmt.dfmEditValues(asLabel).autoSize === true && asNo.autoSize === false && fmt.dfmEditValues(d.byName.get('spbSave')).autoSize === null &&
    asRows.length === 1 && asRows[0].page === 'False' && asRows[0].dfm === 'True' && asRows[0].reset.prop === 'autoSize' && asRows[0].reset.value === true,
    'AutoSize: a TLabel without it = True (VCL default), "False" = false, a button none; a label drawn at a fixed size differs from the DFM', asLabel ? asLabel.name : '-');

  // Alignment (TLabel default taLeftJustify) and an AutoSize=False label: the reset brings the .dfm's size
  const alNode = { name: 'labX', class: 'TLabel', geometry: { left: 0, top: 0, width: 273, height: 25 },
    properties: { AutoSize: { value: 'False' }, Alignment: { value: 'taCenter' } } };
  const alIr = { root: d.root, byName: new Map([['labX', alNode]]) };
  const alRows = dd.diffPage([{ id: 'labX', lay: null, cap: null, look: Object.assign({}, look0, { autoSize: true, alignment: 'taLeftJustify', fontOwn: false }) }], alIr).rows;
  const alA = alRows.find(r => r.prop === 'Alignment'), alS = alRows.find(r => r.prop === 'AutoSize');
  const alItems = dd.resetItemsOf(alRows);
  ok(asLabel && fmt.dfmEditValues(asLabel).alignment === 'taLeftJustify' && fmt.dfmEditValues(alNode).alignment === 'taCenter' && fmt.dfmEditValues(d.byName.get('spbSave')).alignment === null &&
    alA && alA.page === 'taLeftJustify' && alA.dfm === 'taCenter' && alS && alS.reset.value === false && alS.reset.size && alS.reset.size.width === 273 && alS.reset.size.height === 25 &&
    alItems.some(i => i.prop === 'autoSize' && i.size && i.size.width === 273) && alItems.some(i => i.prop === 'alignment' && i.value === 'taCenter'),
    'Alignment: TLabel default taLeftJustify; a centred AutoSize=False label drawn auto -> Alignment + AutoSize rows, the reset brings its 273 x 25', JSON.stringify(alItems));
  const alPanel = Array.from(d.byName.values()).find(n => n.class === 'TPanel' && !(n.properties || {}).Alignment);
  ok(alPanel && fmt.dfmEditValues(alPanel).alignment === 'taCenter' && fmt.dfmEditValues({ class: 'TPanel', properties: { Alignment: { value: 'taLeftJustify' } }, geometry: {} }).alignment === 'taLeftJustify',
    'Alignment: a TPanel without it = taCenter (VCL default), a written one as written', alPanel ? alPanel.name : '-');

  // 頁面檢查 dfm-gap: what the generator left out, from the source against the page's own .dfm
  const plg = require('../lib/pagelint');
  const gapIr = file => { const t = fs.readFileSync(file, 'utf8'); const ti = pageinfo.parseTitle(t); const f0 = ti && ti.dfm ? ir.find(ti.dfm, ti.cls) : null; return { t, ir: f0 ? ir.load(f0) : null }; };
  const gCt = gapIr(path.join(WEB, 'page', 'Setup.Contact.html'));
  const gObs = gapIr(path.join(WEB, 'page', 'Data.Observer.html'));
  const gHp = gapIr(path.join(WEB, 'page', 'Setup.HotPlate.html'));
  const gC = gCt.ir ? plg.dfmGaps(gCt.t, gCt.ir) : [];
  const gLab = gC.find(g => g.id === 'labDelayStatus');
  const gLabFixed = gLab ? gCt.t.slice(0, gLab.fix.at) + gLab.fix.repl + gCt.t.slice(gLab.fix.at + gLab.fix.len) : '';
  const gO = gObs.ir ? plg.dfmGaps(gObs.t, gObs.ir) : [];
  const gPal = gO.find(g => g.id === 'palCustomer');
  const gAgain = gLab ? plg.dfmGaps(gLabFixed, gCt.ir).filter(g => g.id === 'labDelayStatus') : [1];
  ok(gLab && gLab.level === 'info' && /width:273px/.test(gLab.fix.repl) && /height:25px/.test(gLab.fix.repl) && /text-align:center/.test(gLab.fix.repl) && !/width:auto/.test(gLab.fix.repl) &&
    gAgain.length === 0 && !gC.some(g => g.id === 'lblWarning1') && gPal && /pnlCap/.test(gPal.fix.repl) && /text-align:left/.test(gPal.fix.repl) &&
    gHp.ir && plg.dfmGaps(gHp.t, gHp.ir).length === 0,
    'dfm-gap: Setup.Contact labDelayStatus (273 x 25, centred; fixed once, not again), the WordWrap lblWarning1 not flagged, Data.Observer palCustomer left; HotPlate none',
    gC.length + ' on Contact, ' + gO.length + ' on Observer');

  // DFM 位置: per control only the sides that differ, in its own coordinates (the shift taken off); look rows ignored
  const ghA = dd.ghostsOf(pdShift2.rows.concat(pdHidden.rows, dd.diffPage([{ id: 'spbSave', lay: { left: 54, top: 8, width: 200, height: 30, rendered: true }, cap: null,
    look: Object.assign({}, look0, { fontSize: 14, bold: true }) }], d).rows));
  ok(ghA.length === 1 && ghA[0].id === 'spbSave' && ghA[0].left === 54 && ghA[0].width === 227 && ghA[0].height === 40 && !('top' in ghA[0]) &&
    dd.ghostsOf([]).length === 0 && dd.ghostsOf(pd.rows).length === 0,
    'ghostsOf: one frame per control, only its differing sides (left / width / height), no frame for a look difference', JSON.stringify(ghA));
  // Tab 順序 by the .dfm: HotPlate's real IR; a made-up form for VCL's rules
  const tob = require('../lib/taborder');
  const hpTab = tob.dfmTabOrder(d);
  ok(hpTab[0] === 'HotPlateName' && hpTab.indexOf('XST1') > 0 && hpTab.indexOf('spbSave') < 0 && hpTab.indexOf('Label1') < 0,
    'dfmTabOrder(HotPlate): HotPlateName first, XST1 in it; no TSpeedButton, no TLabel', hpTab.join(','));
  const mk = (name, cls, parent, tab, props, graphic) => ({ name, class: cls, path: parent + '.' + name, parent_path: parent, tab_order: tab,
    sibling_index: 0, is_graphic_control: !!graphic, properties: props || {} });
  const fNodes = [
    { name: 'F', class: 'TF', path: 'F', parent_path: null, properties: {} },
    mk('P2', 'TPanel', 'F', 1), mk('P1', 'TPanel', 'F', 0),
    mk('eB', 'TEdit', 'F.P1', 1), mk('eA', 'TEdit', 'F.P1', 0), mk('eNo', 'TEdit', 'F.P1', 2, { TabStop: { value: 'False' } }),
    mk('lbl', 'TLabel', 'F.P1', null, {}, true),
    mk('eC', 'TEdit', 'F.P2', 0), mk('Hid', 'TPanel', 'F', 2, { Visible: { value: 'False' } }), mk('eHid', 'TEdit', 'F.Hid', 0),
    mk('bOk', 'TButton', 'F', 3),
  ];
  const fIr = { root: fNodes[0], byName: new Map(fNodes.map(n => [n.name, n])) };
  const fTab = tob.dfmTabOrder(fIr);
  const cmpT = tob.compareTabOrder(['eB', 'eA', 'extra', 'eC', 'bOk'], fTab);
  ok(fTab.join(',') === 'eA,eB,eC,bOk' && cmpT.common.length === 4 && cmpT.mismatched === 2 && cmpT.common[0].id === 'eB' && cmpT.common[0].dfm === 2 && cmpT.common[2].page === 3 && cmpT.common[2].dfm === 3,
    'dfmTabOrder: panels by TabOrder, depth first; TabStop=False, a TLabel, an invisible panel and what is in it left out; compare: eB/eA swapped = 2 out of order',
    fTab.join(',') + ' ' + JSON.stringify(cmpT.common));

  // 改名稱: id + title in one edit of the start tag, <label for>, the caption only when it still is the name
  const hbR = require('../lib/htmlblock');
  const rnApply = (t, eds) => { let o = t; for (const e of eds.slice().sort((a, b) => b.range[0] - a.range[0])) o = o.slice(0, e.range[0]) + e.repl + o.slice(e.range[1]); return o; };
  const rnHp = fs.readFileSync(path.join(WEB, 'page', 'Setup.HotPlate.html'), 'utf8');
  const rnE1 = hbR.renameEdits(rnHp, 'spbSave', 'btnSave', { caption: true });
  const rnT1 = rnE1 ? rnApply(rnHp, rnE1) : '';
  const rnCk = '<div><label class="ckb" id="CheckBox1" title="CheckBox1 : TCheckBox"><input type="checkbox" >CheckBox1</label><label for="CheckBox1">x</label></div>';
  const rnT2 = rnApply(rnCk, hbR.renameEdits(rnCk, 'CheckBox1', 'chkAuto', { caption: true }));
  const rnPn = '<div id="Panel1" title="Panel1 : TPanel"><span class="pnlCap">Panel1</span><button id="B1">Panel1</button></div>';
  const rnT3 = rnApply(rnPn, hbR.renameEdits(rnPn, 'Panel1', 'pnlTop', { caption: true }));
  const rnP2 = '<div id="P" title="P : TPanel"><button id="Q">P</button></div>';
  const rnT4 = rnApply(rnP2, hbR.renameEdits(rnP2, 'P', 'P2', { caption: true }));
  ok(rnE1 && rnE1.length === 1 && /id="btnSave" title="btnSave : TSpeedButton/.test(rnT1) && rnT1.indexOf('id="spbSave"') < 0 && />Save</.test(rnT1) &&
    rnT2.indexOf('id="chkAuto" title="chkAuto : TCheckBox"') > 0 && rnT2.indexOf('>chkAuto</label>') > 0 && rnT2.indexOf('for="chkAuto"') > 0 &&
    rnT3.indexOf('>pnlTop</span>') > 0 && rnT3.indexOf('<button id="B1">Panel1</button>') > 0 && rnT4.indexOf('<button id="Q">P</button>') > 0 && rnT4.indexOf('id="P2"') > 0 &&
    hbR.renameEdits(rnHp, 'noSuch', 'x', {}) === null,
    'renameEdits: spbSave -> btnSave (its caption Save stays); CheckBox1 -> chkAuto with its caption and <label for>; a panel caption yes, a child\'s text no', rnT3);

  // several selected: which fields differ (a component without the field does not count)
  const mxd = require('../lib/mixed');
  const mxA = { id: 'a', lay: { left: 54, top: 8, width: 227, height: 40 }, cap: { value: 'Save' }, look: { bold: true, fontName: 'Arial', fontSize: 14, color: '#000000' } };
  const mxB = { id: 'b', lay: { left: 332, top: 8, width: 227, height: 40 }, cap: { value: 'save ' }, look: { bold: false, fontName: 'arial', fontSize: 14, color: '#FF0000' } };
  const mxC = { id: 'c', lay: { left: 54, top: 8, width: 227, height: 40 }, cap: null, look: { bold: true, fontName: 'Arial', fontSize: 14, color: '#000000' } };
  const mx1 = mxd.mixedFields([mxA, mxB]), mx2 = mxd.mixedFields([mxA, mxC]);
  ok(mx1.left && mx1.bold && mx1.color && !mx1.top && !mx1.width && !mx1.fontName && !mx1.fontSize && !mx1.caption && Object.keys(mx2).length === 0 && Object.keys(mxd.mixedFields([mxA])).length === 0,
    'mixedFields: left / bold / color differ; the same text in another case is the same; a missing caption does not count', JSON.stringify(mx1));

  // 改回 DFM: one setLayout per control (its sides merged), one setLook / setCaption per property
  const riRows = [
    { id: 'A', reset: { type: 'setLayout', left: 54 } }, { id: 'A', reset: { type: 'setLayout', width: 227 } },
    { id: 'A', reset: { type: 'setLook', prop: 'bold', value: false } }, { id: 'B', reset: { type: 'setCaption', value: 'Save' } },
    { id: 'B', reset: null }, { id: 'B', reset: { type: 'evil' } },
  ];
  const ri = dd.resetItemsOf(riRows);
  ok(ri.length === 3 && ri[0].type === 'setLayout' && ri[0].left === 54 && ri[0].width === 227 && !('top' in ri[0]) && ri[0].force === true &&
    ri[1].type === 'setLook' && ri[1].prop === 'bold' && ri[1].value === false && ri[2].type === 'setCaption' && ri[2].value === 'Save' && dd.resetItemsOf(null).length === 0,
    'resetItemsOf: A -> one setLayout {left, width} + setLook bold; B -> setCaption; nothing else', JSON.stringify(ri));

  // the font of a TPanel lives on its caption span, a TGroupBox's on its legend
  const hpHtml = fs.readFileSync(path.join(WEB, 'page', 'Setup.HotPlate.html'), 'utf8');
  const pc1 = he.captionHostTag(hpHtml, he.startTagOf(hpHtml, 'Panel1'), 'pnlCap');
  const lg1 = he.captionHostTag(hpHtml, he.startTagOf(hpHtml, 'GroupBox1'), 'legend');
  ok(pc1 && /^<span class="pnlCap"/.test(pc1.text) && /font-size:14px/.test(pc1.text) && lg1 && /^<legend\b/.test(lg1.text) &&
    /font-size:16px/.test(he.setStyle(pc1.text, { 'font-size': '16px' })),
    'captionHostTag: Panel1 -> its <span class="pnlCap">, GroupBox1 -> its <legend>; a font edit lands there', pc1 ? pc1.text.slice(0, 70) : '-');

  // --- whole elements (Delete / Copy / Paste): HotPlate's Panel2 holds spbSave + sbtExit
  const hbMod = require('../lib/htmlblock');
  const hbHtml = fs.readFileSync(path.join(WEB, 'page', 'Setup.HotPlate.html'), 'utf8');
  const hbP2 = hbMod.unitOf(hbHtml, 'Panel2', 'self');
  ok(hbP2 && /^<div[^>]*id="Panel2"/.test(hbP2.html) && /<\/div>$/.test(hbP2.html) && hbMod.idsIn(hbP2.html).join(',').indexOf('spbSave') >= 0 &&
    hbMod.idsIn(hbP2.html).indexOf('sbtExit') >= 0 && hbMod.idsIn(hbP2.html)[0] === 'Panel2',
    'elementRange: Panel2 from its start tag through its own </div>, with spbSave and sbtExit inside', hbMod.idsIn(hbP2 ? hbP2.html : '').join(','));
  const hbX = hbMod.unitOf(hbHtml, 'XST1', 'parent');
  ok(hbX && /^<span\b/.test(hbX.html) && /id="XST1"/.test(hbX.html) && /<\/span>$/.test(hbX.html), 'unitOf(XST1, parent): the positioning <span> with the input in it', hbX ? hbX.html.slice(0, 80) : '-');
  const hbUsed = new Set(pageinfo.collectIds(hbHtml));
  const hbRn = hbMod.renameIds(hbP2.html, hbUsed);
  ok(hbRn.map.get('Panel2') === 'Panel2_2' && hbRn.map.get('spbSave') === 'spbSave_2' && !/id="(Panel2|spbSave|sbtExit)"/.test(hbRn.html) &&
    /title="spbSave_2 : TSpeedButton/.test(hbRn.html) && hbUsed.has('sbtExit_2'),
    'renameIds: every id of the copy new (Panel2_2, spbSave_2, sbtExit_2), the generated titles too', JSON.stringify(Array.from(hbRn.map)));
  const hbOff = hbMod.offsetBlock('<div id="a" style="position:absolute;left:10px;top:20px;width:5px;"><b></b></div>', 8, 8);
  ok(/left:18px;top:28px;width:5px;/.test(hbOff) && /<b><\/b><\/div>$/.test(hbOff), 'offsetBlock: only the first tag, left/top +8', hbOff);
  const hbFp = hbMod.firstPx('<span class="lb" id="q" style="position:absolute;left:12px;top:3px;width:auto;"><i style="left:99px"></i></span>');
  ok(hbFp && hbFp.left === 12 && hbFp.top === 3 && hbFp.width === null && hbFp.height === null, 'firstPx: the first tag\'s px values, auto = null', JSON.stringify(hbFp));
  const hbDoc = 'a\n  <div id="x">1</div><span id="y"></span>\n  <i id="z"></i>\nb';
  const hbT1 = hbMod.unitOf(hbDoc, 'z', 'self');
  const hbR1 = hbMod.removeRange(hbDoc, hbT1.start, hbT1.end);
  const hbT2 = hbMod.unitOf(hbDoc, 'x', 'self');
  const hbR2 = hbMod.removeRange(hbDoc, hbT2.start, hbT2.end);
  ok(hbR1.text === 'a\n  <div id="x">1</div><span id="y"></span>\nb' && hbR2.text === 'a<span id="y"></span>\n  <i id="z"></i>\nb',
    'removeRange: alone on its line -> the line goes; starts a line with more after -> the break before it goes', JSON.stringify([hbR1.text, hbR2.text]));
  const hbIns = 'P' + '\n    ' + '<i id="n"></i>' + '<q></q>';
  const hbT3 = hbMod.unitOf(hbIns, 'n', 'self');
  ok(hbMod.removeRange(hbIns, hbT3.start, hbT3.end).text === 'P<q></q>', 'a block pasted as "\\n" + indent + block, deleted again: byte-identical');
  const hbCli = '<fieldset id="g"><legend>L</legend><div class="cli" style="inset:0"><i id="k"></i></div></fieldset>';
  const hbAt = hbMod.insideEnd(hbCli, he.startTagOf(hbCli, 'g'));
  ok(hbCli.slice(hbAt, hbAt + 6) === '</div>' && hbCli.slice(0, hbAt).endsWith('<i id="k"></i>'), 'insideEnd: a GroupBox keeps its children in <div class="cli">: paste goes there');
  // (the form holds a GroupBox with its own cli: into the FORM = the form's end, not that GroupBox's cli)
  const hbForm = '<div class="form"><div id="p"></div>' + hbCli + '<b id="z"></b></div>';
  const hbFAt = hbMod.insideEnd(hbForm, he.startTagOf(hbForm, '@form'));
  ok(hbForm.slice(hbFAt) === '</div>' && hbForm.slice(0, hbFAt).endsWith('<b id="z"></b>'),
    'insideEnd: into the form = the form\'s own end, not a GroupBox\'s <div class="cli"> inside it', hbForm.slice(Math.max(0, hbFAt - 20), hbFAt + 6));
  ok(hbMod.elementRange('<div id="u"><p>', he.startTagOf('<div id="u"><p>', 'u')) === null, 'elementRange: unbalanced markup -> null (nothing is cut)');

  // --- WPF Order: the place among the siblings (later = on top); a legend / pnlCap stays first
  const orApply = (t, parts) => parts.slice().sort((a, b) => b.range[0] - a.range[0]).reduce((s, p) => s.slice(0, p.range[0]) + p.repl + s.slice(p.range[1]), t);
  const orIds = t => hbMod.idsIn(t).join(',');
  const orDo = (t, id, how) => { const u = hbMod.unitOf(t, id, 'self'); const r = hbMod.reorder(t, u, how); return r.parts ? orApply(t, r.parts) : r; };
  const orFs = '<fieldset id="g"><legend>L</legend><i id="a"></i><i id="b"></i><i id="c"></i></fieldset>';
  ok(orIds(orDo(orFs, 'c', 'back')) === 'g,c,a,b' && /<legend>L<\/legend><i id="c">/.test(orDo(orFs, 'c', 'back')) &&
    orIds(orDo(orFs, 'a', 'front')) === 'g,b,c,a' && orIds(orDo(orFs, 'a', 'forward')) === 'g,b,a,c' && orIds(orDo(orFs, 'c', 'backward')) === 'g,a,c,b' &&
    orDo(orFs, 'a', 'backward').same === true && orDo(orFs, 'c', 'front').same === true,
    'reorder: back (after the legend) / front / forward / backward; already there = same', [orDo(orFs, 'c', 'back'), orDo(orFs, 'a', 'front')].join(' | '));
  const orLines = '<div id="p">\n  <i id="a"></i>\n  <i id="b"></i>\n</div>\n';
  ok(orDo(orLines, 'a', 'front') === '<div id="p">\n  <i id="b"></i>\n  <i id="a"></i>\n</div>\n' && orDo(orDo(orLines, 'a', 'front'), 'a', 'back') === orLines,
    'reorder: a line moves as a line; front then back = byte-identical', JSON.stringify(orDo(orLines, 'a', 'front')));
  const orHp = hbHtml;
  const orP2 = t => { const u = hbMod.unitOf(t, 'Panel2', 'self'); return u.html; };
  const orHp2 = orDo(orHp, 'spbSave', 'front');
  const orHp3 = typeof orHp2 === 'string' ? orDo(orHp2, 'spbSave', 'back') : null;
  const orBefore = (t, a, b) => orP2(t).indexOf('id="' + a + '"') < orP2(t).indexOf('id="' + b + '"');
  ok(typeof orHp2 === 'string' && orBefore(orHp2, 'sbtExit', 'spbSave') && /^<div[^>]*id="Panel2"[^>]*>\s*<span class="pnlCap"/.test(orP2(orHp2)) &&
    typeof orHp3 === 'string' && orBefore(orHp3, 'spbSave', 'sbtExit') && /^<div[^>]*id="Panel2"[^>]*>\s*<span class="pnlCap"/.test(orP2(orHp3)),
    'HotPlate Panel2: spbSave to the front goes after sbtExit, to the back before it; the caption span stays first', hbMod.idsIn(orP2(orHp2)).join(','));

  // --- 工具箱: every template is one balanced element with its id, its class in the title
  const tbMod = require('../lib/toolbox');
  const tbBad = tbMod.ITEMS.filter(it => {
    const h = '<div id="host">' + it.html('X1', 8, 16) + '</div>';
    const t0 = he.startTagOf(h, 'X1');
    const u = hbMod.unitOf(h, 'X1', it.cls === 'TEdit' ? 'parent' : 'self');
    const hostR = hbMod.elementRange(h, he.startTagOf(h, 'host'));
    return !t0 || !u || !hostR || hostR[1] !== h.length || !new RegExp('title="X1 : ' + it.cls + '"').test(h) || !/left:8px;top:16px/.test(u.html);
  }).map(it => it.cls);
  ok(tbMod.ITEMS.length === 7 && tbBad.length === 0, 'toolbox: 7 templates, each one balanced element with id, title "name : TClass", left/top', 'bad: ' + tbBad.join(','));
  ok(tbMod.newName('Label', new Set(['Label1', 'Label2', 'Label4'])) === 'Label3' && tbMod.newName('Panel', new Set()) === 'Panel1',
    'toolbox names: the first free BCB6-style name (Label3, Panel1)');
  const tbSized = tbMod.itemOf('TSpeedButton').html('SB9', 3, 4, 100, 30), tbDef = tbMod.itemOf('TSpeedButton').html('SB9', 3, 4);
  const tbGb = tbMod.itemOf('TGroupBox').html('GB9', 0, 0, 200, 120);
  ok(/left:3px;top:4px;width:100px;height:30px;/.test(tbSized) && /width:75px;height:25px;/.test(tbDef) && /width:200px;height:120px;/.test(tbGb),
    'toolbox templates take a size (a drawn box); without one, the default', tbSized.slice(0, 120));

  // --- 頁面檢查: the generator's cut style attribute (HotPlate Panel1's caption span), and the other kinds
  const lnMod = require('../lib/pagelint');
  const lnHp = lnMod.lintPage(hbHtml, { dir: path.join(WEB, 'page') });
  const lnQ = lnHp.filter(i => i.kind === 'quote-cut');
  ok(lnQ.length === 1 && lnQ[0].fix && /font-family:'MS Sans Serif',sans-serif;font-weight:400;$/.test(lnQ[0].fix.repl) &&
    hbHtml.slice(lnQ[0].fix.at, lnQ[0].fix.at + lnQ[0].fix.len).indexOf('"MS Sans Serif"') > 0 && lnHp.every(i => i.kind === 'quote-cut'),
    'lintPage(HotPlate): the one cut style (Panel1\'s caption span); the fix keeps font-weight:400', lnQ[0] ? lnQ[0].fix.repl : '-');
  const lnSyn = lnMod.lintPage('<div id="a"></div><div id="a"></div><span id="b"><i></i><img src="nope.png"><script id="s">if (a < b) x();</script>',
    { dir: '/x', exists: () => false });
  ok(lnSyn.map(i => i.kind).join(',') === 'dup-id,unclosed,missing' && lnSyn[1].id === 'b',
    'lintPage: a second id="a", an unclosed <span id="b">, a missing image; the script\'s "a < b" is not markup', lnSyn.map(i => i.kind + ':' + (i.id || '')).join(','));
  ok(lnMod.cutValues('<p title="say "hi" now" class="x">').length === 1 && lnMod.cutValues('<p title=\'say "hi"\' class="x">').length === 0,
    'cutValues: a double quote inside "…" cuts it; inside \'…\' it does not');

  // --- the grid's HTML rows: the attributes the source's start tag writes, as written
  const atSave = he.attrsOf(he.startTagOf(hbHtml, 'spbSave').text);
  const atBare = he.attrsOf('<input type="checkbox" checked data-x=\'a b\' tabindex=3>');
  ok(atSave.map(a => a[0]).join(',') === 'class,id,title,style' && atSave[1][1] === 'spbSave' && /^spbSave : TSpeedButton/.test(atSave[2][1]) &&
    JSON.stringify(atBare) === JSON.stringify([['type', 'checkbox'], ['checked', ''], ['data-x', 'a b'], ['tabindex', '3']]),
    'attrsOf: spbSave\'s class / id / title / style as written; bare and unquoted ones too', JSON.stringify(atSave.map(a => a[0])));

  // --- 在所有頁面搜尋: a name, the shown text, a class -- over every page
  const psMod = require('../lib/pagesearch');
  const psIds = psMod.searchPages(WEB, 'spbSave');
  const psHp = psIds.find(h => h.page === 'Setup.HotPlate.html');
  ok(psIds.length >= 5 && psIds[0].exact && psHp && psHp.cls === 'TSpeedButton' && psHp.caption === 'Save' && psHp.how === 'id' && psHp.line > 0,
    'searchPages spbSave: on many pages, exact name first; HotPlate\'s is a TSpeedButton showing "Save" (the text after its <img>)', psIds.length + ' hits');
  const psText = psMod.searchPages(WEB, 'plate of use');
  ok(psText.some(h => h.page === 'Setup.HotPlate.html' && h.id === 'GroupBox1' && h.how === 'text'), 'searchPages by the text a group box shows (its legend): GroupBox1',
    psText.slice(0, 3).map(h => h.page + ' ' + h.id).join(', '));
  const psNone = psMod.searchPages(WEB, 'zz-no-such-thing-qq');
  const psEls = psMod.elementsOf('<span style="position:absolute"><input class="ed" id="e1" title="e1 : TEdit" value="12.5"></span><label class="ckb" id="c1" title="c1 : TCheckBox"><input type="checkbox" >On</label>');
  ok(psNone.length === 0 && psEls.length === 2 && psEls[0].caption === '12.5' && psEls[1].caption === 'On' && psEls[1].cls === 'TCheckBox',
    'searchPages: no hit = empty; an edit shows its value, a check box its label', JSON.stringify(psEls));

  // --- the 頁面 list: every page of web\page and web\, by the name before the first dot
  const plMod = require('../lib/pagelist');
  const plGroups = plMod.listPages(WEB);
  const plFlat = [].concat(...plGroups.map(g => g.pages));
  const plHp = plFlat.find(p => p.name === 'Setup.HotPlate.html');
  const plDisk = fs.readdirSync(path.join(WEB, 'page')).filter(n => /\.html?$/i.test(n)).length + fs.readdirSync(WEB).filter(n => /\.html?$/i.test(n)).length;
  ok(plGroups[0].name === 'Main' && plGroups[plGroups.length - 1].name === plMod.ROOT && plFlat.length === plDisk && plHp && plHp.label === 'HotPlate' && /Hot Plate/.test(plHp.title),
    'listPages: Main first, web root last, every .html once; Setup.HotPlate.html -> Setup / HotPlate (title from the page)',
    plGroups.map(g => g.name + ' ' + g.pages.length).join(', '));
  const plOther = plGroups.find(g => g.name === plMod.OTHER);
  const plInPage = [].concat(...plGroups.filter(g => g.name !== plMod.ROOT).map(g => g.pages));
  ok(plOther && plOther.pages.some(p => p.name === 'main.html') && plInPage.filter(p => p.redirect).length === fwd,
    'names without a Group. prefix go to 其他; forwarding pages carry their target', 'forwarding ' + plFlat.filter(p => p.redirect).map(p => p.name + '->' + p.redirect).join(', '));

  // --- 有新版: the version VS Code registered in the extensions folder, compared as numbers
  const upd = require('../lib/update');
  const updDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-upd-'));
  fs.writeFileSync(path.join(updDir, 'extensions.json'), JSON.stringify([
    { identifier: { id: 'ht9045.ht9045-html-designer' }, version: '0.9.0' },
    { identifier: { id: 'HT9045.HT9045-html-designer' }, version: '0.10.0' },
    { identifier: { id: 'someone.else' }, version: '9.0.0' }, null, { identifier: {} }]));
  const updV = upd.installedVersion(updDir, 'ht9045.ht9045-html-designer');
  const updNone = [upd.installedVersion(updDir, 'nobody.here'), upd.installedVersion(path.join(updDir, 'missing'), 'ht9045.ht9045-html-designer'), upd.installedVersion(null, 'x')];
  fs.writeFileSync(path.join(updDir, 'extensions.json'), '{ broken');
  updNone.push(upd.installedVersion(updDir, 'ht9045.ht9045-html-designer'));
  fs.rmSync(updDir, { recursive: true, force: true });
  ok(upd.cmpVer('0.9.0', '0.10.0') === -1 && upd.cmpVer('0.62.0', '0.61.9') === 1 && upd.cmpVer('1.0', '1.0.0') === 0 && updV === '0.10.0' && updNone.every(v => v === null),
    'update: versions compared as numbers (0.9 < 0.10), the newest registered one (id case ignored), null when not there / not readable', updV + ' ' + JSON.stringify(updNone));

  // --- 搜尋頁面: the pages with every word (name / title, or a component), their components, the highlights
  const fpMod = require('../lib/pagesearch');
  const fpA = fpMod.filterPages(WEB, 'grpLoader_9050');
  const fpIo = fpA && fpA.groups.find(g => g.name === 'HW');
  const fpIoPage = fpIo && fpIo.pages.find(p => p.name === 'HW.IoSetView.html');
  // a tab's label has no id: the page is found by the text it shows
  const fpTab = fpMod.filterPages(WEB, 'Above9050');
  const fpTabPage = fpTab && [].concat(...fpTab.groups.map(g => g.pages)).find(p => p.name === 'HW.IoSetView.html');
  ok(fpTabPage && fpTabPage.textHit && !fpTabPage.nameHit && fpTabPage.hits.every(h => h.id !== 'ht9050OnlyCss'),
    'filterPages: "Above9050" (a tab label, no id) finds HW.IoSetView by the text it shows; its <style id> is no hit',
    fpTabPage && JSON.stringify({ textHit: fpTabPage.textHit, hits: fpTabPage.hits.map(h => h.id) }));
  const fpName = fpMod.filterPages(WEB, 'hotplate');
  const fpHp = fpName && [].concat(...fpName.groups.map(g => g.pages)).find(p => p.name === 'Setup.HotPlate.html');
  const fpNone = fpMod.filterPages(WEB, 'zz-no-such-thing-qq');
  const fpEmpty = fpMod.filterPages(WEB, '   ');
  const fpHl = fpMod.highlightsOf('SetupHotPlate hot', ['hot', 'plate']);
  const fpStyle = fpMod.elementsOf('<style id="css9050">.a{}</style><script id="s1">var x</script><span id="real">Above</span>');
  ok(fpStyle.length === 1 && fpStyle[0].id === 'real', 'elementsOf: a <style id> / <script id> is not a component', JSON.stringify(fpStyle.map(e => e.id)));
  ok(fpIoPage && fpIoPage.hits.length >= 1 && fpA.pages === fpA.groups.reduce((s, g) => s + g.pages.length, 0) && fpHp && fpHp.nameHit &&
    fpNone && fpNone.pages === 0 && fpNone.groups.length === 0 && fpEmpty === null && JSON.stringify(fpHl) === '[[5,13],[14,17]]',
    'filterPages: "grpLoader_9050" -> HW / IoSetView with its component; "hotplate" -> the page by its name; nothing -> 0 pages; blank -> null; highlights merged',
    (fpA ? fpA.pages + ' pages ' + fpA.hits + ' hits' : '-') + ' ' + JSON.stringify(fpHl));

  // --- 機種與機台設定 (read only): launch.json's machine / settings files, an ini in wb_serve's /api/system shape
  const lcMod = require('../lib/liveconfig');
  const lcDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-live-'));
  fs.mkdirSync(path.join(lcDir, '.vscode'));
  fs.writeFileSync(path.join(lcDir, '.vscode', 'launch.json'), '{\n  // a comment with "quotes" and a // inside\n  "configurations": [\n' +
    '    { "name": "a", /* block */ "environment": [ { "name": "W906_HMI_URL", "value": "http://127.0.0.1:8055/background.html?mode=debug&machine=HT9050" },\n' +
    '      { "name": "W906_GENERAL_INI_PATH", "value": "${workspaceFolder}\\\\sys\\\\Gerneral.ini" }, { "name": "W906_AUTH_PATH", "value": "${workspaceFolder}\\\\cfg\\\\" }, ] },\n' +
    '    { "name": "b", "env": { "W906_HMI_URL": "http://x/?machine=HT9045" } },\n    { "name": "c", "env": { "W906_HMI_URL": "http://x/?machine=HT9050&a=1" } },\n  ],\n}\n');
  const lcHints = lcMod.launchHints([lcDir, path.join(lcDir, 'nothing-here')]);
  const lcIni = path.join(lcDir, 'g.ini');
  fs.writeFileSync(lcIni, Buffer.concat([Buffer.from('; top comment\r\nkey=before any section\r\n[Version]\r\n   \tModel=HT-9050  \r\nModel=second\r\n' +
    '# hash comment\r\n[ System ]\r\nN=12\r\nF=2.50\r\nQ= " a " \r\nE=\r\nH=$FF\r\nnoequals\r\n[System]\r\nN=99\r\nZ=1\r\n[Text]\r\n', 'latin1'),
    Buffer.from([0x54, 0x3d, 0xa4, 0xa4, 0xa4, 0xe5, 0x0d, 0x0a])]));   // T=中文 in Big5
  const lcDoc = lcMod.iniToDoc(lcIni);
  const lcS = lcDoc.sections;
  const lcMiss = lcMod.iniToDoc(path.join(lcDir, 'missing.ini'));
  fs.rmSync(lcDir, { recursive: true, force: true });
  ok(lcHints.machine === 'HT9050' && lcHints.machines.HT9050 === 2 && lcHints.machines.HT9045 === 1 &&
    lcHints.generalIni === path.join(lcDir, 'sys', 'Gerneral.ini') && lcHints.configIni === path.join(lcDir, 'cfg', 'config.ini'),
    'launchHints: the machine most F5 configurations open the HMI for; W906_GENERAL_INI_PATH / W906_AUTH_PATH + config.ini (${workspaceFolder}, comments, trailing commas)',
    JSON.stringify(lcHints));
  ok(lcDoc.available && lcS.Version.Model.value === 'HT-9050' && lcS.Version.Model.raw === 'HT-9050  ' && !lcS.key &&
    lcS.System.N.value === 12 && lcS.System.N.type === 'int' && lcS.System.F.type === 'float' && lcS.System.F.value === 2.5 &&
    lcS.System.Q.value === ' a ' && lcS.System.E.value === '' && lcS.System.E.type === 'string' && lcS.System.H.type === 'string' &&
    !lcS.System.Z && !lcS.System.noequals && lcS.Text && lcS.Text.T.value === '中文' && lcMiss.available === false,
    'iniToDoc: wb_serve\'s shape; the first section / key wins, trimmed, one quote pair dropped, "12" int / "2.50" float / "$FF" string, Big5 read, a missing file available:false',
    JSON.stringify(lcS.System));

  // --- CSV 表格: parse with offsets; an edit replaces only the text of the cells it changes
  const cv = require('../lib/csvtable');
  const applyReps = (t, reps) => { let o = t; for (const r of reps.slice().sort((a, b) => b.s - a.s)) o = o.slice(0, r.s) + r.text + o.slice(r.e); return o; };
  const cvText = 'Name,Alias,Note,\r\nM00,MInArmX,"a, b",\r\nM01,,"say ""hi""\r\nnext line",x\r\n';
  const cvP = cv.parseDoc(cvText);
  ok(cvP.delim === ',' && cvP.eol === '\r\n' && cvP.rows.length === 3 && cvP.rows[0].cells.length === 4 && cvP.rows[0].cells[3].v === '' &&
    cvP.rows[1].cells[2].v === 'a, b' && cvP.rows[1].cells[2].q && cvP.rows[2].cells[2].v === 'say "hi"\r\nnext line' && cvP.rows[2].cells[3].v === 'x' &&
    cvText.slice(cvP.rows[1].cells[1].s, cvP.rows[1].cells[1].e) === 'MInArmX',
    'csv parse: CRLF, a trailing empty column, quoted commas / quotes / a line break inside, offsets exact', JSON.stringify(cvP.rows.map(r => r.cells.map(c => c.v))));
  const cvE1 = applyReps(cvText, cv.cellEdits(cvP, [{ r: 1, c: 1, v: 'MInArmY' }]));
  const cvE2 = applyReps(cvText, cv.cellEdits(cvP, [{ r: 1, c: 2, v: 'c' }, { r: 2, c: 1, v: 'x,y' }, { r: 1, c: 0, v: 'M00' }]));
  const cvE3 = applyReps(cvText, cv.cellEdits(cvP, [{ r: 0, c: 5, v: 'Z' }, { r: 4, c: 1, v: 'new' }]));
  ok(cvE1 === cvText.replace('MInArmX', 'MInArmY') && cvE2 === cvText.replace('"a, b"', '"c"').replace('M01,,', 'M01,"x,y",') &&
    cvE3 === 'Name,Alias,Note,,,Z\r\nM00,MInArmX,"a, b",\r\nM01,,"say ""hi""\r\nnext line",x\r\n,,,\r\n,new,,\r\n' &&
    cv.cellEdits(cvP, [{ r: 1, c: 0, v: 'M00' }]).length === 0,
    'csv edits: one cell = only its text; a quoted one stays quoted, a comma gets quotes; past the row end / past the last row added; unchanged = nothing',
    JSON.stringify(cvE3));
  const cvIns = applyReps(cvText, cv.insertRows(cvP, 1, 1));
  const cvDelMid = applyReps(cvText, cv.deleteRows(cvP, 1, 1));
  const cvDelLast = applyReps(cvText, cv.deleteRows(cvP, 2, 1));
  const cvNoEol = 'a,b\nc,d';
  const cvP2 = cv.parseDoc(cvNoEol);
  const cvDelLast2 = applyReps(cvNoEol, cv.deleteRows(cvP2, 1, 1));
  const cvAdd2 = applyReps(cvNoEol, cv.cellEdits(cvP2, [{ r: 2, c: 0, v: 'e' }]));
  ok(cvIns === 'Name,Alias,Note,\r\n,,,\r\nM00,MInArmX,"a, b",\r\nM01,,"say ""hi""\r\nnext line",x\r\n' &&
    cvDelMid === 'Name,Alias,Note,\r\nM01,,"say ""hi""\r\nnext line",x\r\n' && cvDelLast === 'Name,Alias,Note,\r\nM00,MInArmX,"a, b",\r\n' &&
    cvP2.eol === '\n' && cvDelLast2 === 'a,b' && cvAdd2 === 'a,b\nc,d\ne,',
    'csv rows: insert before a row (same width, the file\'s line end), delete in the middle / at the end (with / without a last line end)',
    JSON.stringify([cvDelLast2, cvAdd2]));
  const cvTsv = cv.toTsv([['a', 'b\tc'], ['1', '']]);
  const cvFrom = cv.fromTsv('x\ty\r\n1\t"2\t3"\r\n');
  const cvBig5 = Buffer.from([0x41, 0x2c, 0xa4, 0xa4, 0x0d, 0x0a]);
  ok(cvTsv === 'a\t"b\tc"\r\n1\t' && JSON.stringify(cvFrom) === '[["x","y"],["1","2\\t3"]]' && cv.detectDelim('a;b;c\n1;2;3') === ';' &&
    cv.decodeSafe(cvBig5, 'A,\uFFFD\uFFFD\r\n').ok === false && cv.decodeSafe(Buffer.from('A,B'), 'A,B').ok && cv.decodeSafe(cvBig5, 'A,中\r\n').ok,
    'csv clipboard: tab separated both ways (Excel\'s); ; detected; a Big5 file read as UTF-8 (U+FFFD) is not editable, read right it is');
  // --- 事件表: the events of a VCL class (the Object Inspector's list), BCB6's handler names and signatures
  const ve = require('../lib/vclevents');
  const veBtn = ve.eventsOf('TButton');
  ok(ve.eventsOf('TSpeedButton').join(',') === 'OnClick,OnDblClick,OnMouseDown,OnMouseMove,OnMouseUp' && veBtn.includes('OnEnter') && !veBtn.includes('OnDblClick') &&
    ve.eventsOf('TMyLedLane', ['OnFoo', 'bad']).join(',') === 'OnClick,OnContextPopup,OnDblClick,OnDragDrop,OnDragOver,OnEndDrag,OnFoo,OnMouseDown,OnMouseMove,OnMouseUp,OnStartDrag' &&
    ve.eventsOf('TForm').includes('OnShow') && ve.handlerName('spbSave', 'OnClick') === 'spbSaveClick' && ve.handlerName('', 'OnShow', true) === 'FormShow' &&
    ve.formEventOf('OnClick') === 'click' && ve.formEventOf('OnChange') === 'change' && ve.formEventOf('OnDblClick') === null &&
    ve.signatureOf('OnKeyDown') === 'TObject *Sender, WORD &Key, TShiftState Shift' && ve.signatureOf('OnClick') === 'TObject *Sender',
    'vclevents: a class\'s events sorted (a custom one = TControl\'s + the .dfm\'s), BCB6 names (spbSaveClick, FormShow), form.event click / change, VCL signatures');
  ok(ve.defaultEventOf('TSpeedButton') === 'OnClick' && ve.defaultEventOf('TEdit') === 'OnChange' && ve.defaultEventOf('TComboBox') === 'OnChange' &&
    ve.defaultEventOf('TTimer') === 'OnTimer' && ve.defaultEventOf('', true) === 'OnCreate' && ve.defaultEventOf('TLabel') === 'OnClick' &&
    ve.defaultEventOf('TBevel') === null && ve.defaultEventOf('TMyLedLane') === 'OnClick',
    'vclevents.defaultEventOf: what a double-click in the designer opens / creates (Button1Click, Edit1Change, Timer1Timer, FormCreate)');
  // --- 新增事件處理函式: the declaration after the class's last event handler, an empty body at the end (the real
  //     forms/fHotPlate.h / .cpp, READ only, in memory)
  const cstub = require('../lib/cppstub');
  const csH = fs.readFileSync(path.join(PORT, 'forms', 'fHotPlate.h'), 'utf8');
  const csC = fs.readFileSync(path.join(PORT, 'forms', 'fHotPlate.cpp'), 'utf8');
  const csB = cstub.classBody(csH, 'TfHotPlate');
  const csNote = '//AI(W906-HTDESIGNER) 20260930: test';
  const csDe = cstub.declEdit(csH, 'TfHotPlate', 'spbTestClick', 'TObject *Sender', csNote);
  const csH2 = csDe ? csH.slice(0, csDe.at) + csDe.text + csH.slice(csDe.at) : '';
  const csFe = cstub.defEdit(csC, 'TfHotPlate', 'spbTestClick', 'TObject *Sender', csNote);
  const csC2 = csC + csFe.text;
  const csEol = /\r\n/.test(csH) ? '\r\n' : '\n';
  const csLines2 = csC2.split('\n');
  const csFake = 'class TfX {\n  // a { in a comment\n  const char* s = "}";\npublic:\n    void aClick(TObject *Sender);\nprivate:\n    int n;\n};\n';
  const csFD = cstub.declEdit(csFake, 'TfX', 'bClick', 'TObject *Sender', '');
  ok(!!(csB && cstub.declares(csH, 'TfHotPlate', 'sbtExitClick') && !cstub.declares(csH, 'TfHotPlate', 'spbTestClick') && csDe &&
    csH2.indexOf('void spbTestClick(TObject *Sender);   ' + csNote) > 0 && cstub.declares(csH2, 'TfHotPlate', 'spbTestClick') &&
    csH2.slice(0, csDe.at) === csH.slice(0, csDe.at) && csH2.slice(csDe.at + csDe.text.length) === csH.slice(csDe.at) && csDe.text.indexOf(csEol) === 0 &&
    csDe.at < csB.close && csC2.slice(0, csC.length) === csC && /void TfHotPlate::spbTestClick\(TObject \*Sender\)\r?\n\{\r?\n    \(void\)Sender;\r?\n    \/\/ TODO/.test(csC2) &&
    /^\s*\/\/ TODO/.test(csLines2[csFe.bodyLine]) && csFD && csFake.slice(0, csFD.at) + csFD.text + csFake.slice(csFD.at) === 'class TfX {\n  // a { in a comment\n  const char* s = "}";\npublic:\n    void aClick(TObject *Sender);\n    void bClick(TObject *Sender);\nprivate:\n    int n;\n};\n'),
    'cppstub: TfHotPlate found in fHotPlate.h (braces in comments / strings skipped); the declaration after its last handler, the file\'s line ends; the body at the end of fHotPlate.cpp; nothing else moves',
    csDe ? JSON.stringify(csDe.text) : '-');

  // --- the command trace follows the page's own send(...) (a bare call), not socket.send(...) (a method)
  const wsT = require('../lib/websearch');
  const wsSrc = [new wsT.Source('C:\\x\\io.js', 'function onClick(el){ send(el, 1); }\nfunction send(el, v){ R.rawCmd(\'control.takeover\'); R.rawCmd(\'io.btnPanelClick\', {tag:1}); }\n', false)];
  const wsTr = wsT.traceCmds(wsSrc, 'function (e) { ws.send("x"); onClick(e.target); }', 3, 'C:\\x\\io.js');
  ok(wsTr.map(t => t.cmd).join(',') === 'control.takeover,io.btnPanelClick' && wsTr[1].via.join('>') === 'onClick>send',
    'traceCmds: a listener -> onClick -> the page\'s own send() -> io.btnPanelClick (ws.send stays a builtin)', JSON.stringify(wsTr));
  // a command held in a name (ht9045_main_close.js: var CMD = 'act.main.closeProgram'; raw(CMD, …))
  const wsSrc2 = [new wsT.Source('C:\\x\\close.js', "var CMD = 'act.main.closeProgram';\nfunction raw(name, extra) { return R.rawCmd(name, extra); }\nfunction send(step) { return raw(CMD, { value: { step: step } }); }\n", false)];
  const wsTr2 = wsT.traceCmds(wsSrc2, 'function () { send(0); }', 3, 'C:\\x\\close.js');
  ok(wsTr2.some(t => t.cmd === 'act.main.closeProgram' && t.via.join('>') === 'send'),
    'traceCmds: raw(CMD, …) with var CMD = \'act.main.closeProgram\' -> the command (the Exit button\'s)', JSON.stringify(wsTr2));

  // --- a C++ handler the designer added, renamed: the declaration in the class and cls::name in the .cpp only
  const csRn = cstub.renameEdits(csH2, csC2, 'TfHotPlate', 'spbTestClick', 'spbTest2Click');
  const csApply = (t, eds) => { let o = t; for (const x of eds.slice().sort((a, b) => b.s - a.s)) o = o.slice(0, x.s) + x.text + o.slice(x.e); return o; };
  const csH3 = csApply(csH2, csRn.h), csC3 = csApply(csC2, csRn.cpp);
  ok(csRn.h.length === 1 && csRn.cpp.length === 1 && csH3 === csH2.replace('void spbTestClick(', 'void spbTest2Click(') &&
    csC3 === csC2.replace('TfHotPlate::spbTestClick(', 'TfHotPlate::spbTest2Click('),
    'cppstub.renameEdits: the declaration in the class, TfHotPlate::name in the .cpp; nothing else', JSON.stringify([csRn.h.length, csRn.cpp.length]));
  // a double-click on a property's name: where it is written -- a style value (on the wrapper for an input's Left), the
  // caption text, a panel's caption font, an input's value, the disabled attribute; not written = null
  const heP = require('../lib/htmledit');
  const prSrc = '<div class="pnl" id="P1" style="left:1px;top:2px;"><span class="pnlCap" style="font-size:12px;text-align:left">Cap</span></div>' +
    '<button id="B1" style="position:absolute;left:54px;top:8px;font-weight:bold;" disabled>Save</button>' +
    '<span style="position:absolute;left:7px;top:9px"><input id="I1" value="12" style="width:40px"></span>';
  const prOf = (id, p) => { const r = heP.propRange(prSrc, id, p); return r ? prSrc.slice(r[0], r[1]) : null; };
  const prGot = [prOf('B1', 'Left'), prOf('B1', 'Caption'), prOf('B1', 'Font.Bold'), prOf('B1', 'Enabled'), prOf('I1', 'Left'), prOf('I1', 'Width'), prOf('I1', 'Text'),
    prOf('P1', 'Caption'), prOf('P1', 'Font.Size'), prOf('P1', 'Alignment'), prOf('B1', 'Font.Name')];
  ok(JSON.stringify(prGot) === JSON.stringify(['54px', 'Save', 'bold', 'disabled', '7px', '40px', '12', 'Cap', '12px', 'left', null]),
    'htmledit.propRange: Left / Caption / Font.Bold / Enabled of a button, an input\'s Left on its wrapper, its Width and value, a panel\'s caption / font / alignment on its pnlCap; not written = null',
    JSON.stringify(prGot));
  // WPF "navigates to the existing handler" with the caret in its body: the first statement's line and indent; an empty
  // body = just after its brace; a brace in a comment on the signature line does not count; a brace on its own line too
  const bsSrc = ['void TfHotPlate::spbSaveClick(TObject *Sender) // {not this', '{', '    DoSave();', '}', '', 'void TfHotPlate::X(TObject *Sender) {}',
    'void TfHotPlate::Y(TObject *Sender) {', '', '}'].join('\r\n');
  const bs1 = cstub.bodyStart(bsSrc, 0), bs2 = cstub.bodyStart(bsSrc, 5), bs3 = cstub.bodyStart(bsSrc, 6), bs4 = cstub.bodyStart('int a;', 0);
  ok(JSON.stringify(bs1) === '{"line":2,"col":4}' && JSON.stringify(bs2) === '{"line":5,"col":37}' && JSON.stringify(bs3) === '{"line":7,"col":0}' && bs4 === null,
    'cppstub.bodyStart: the caret on the first statement (2:4), just after an empty body\'s brace (5:37), the empty line inside (7:0); no brace = null',
    JSON.stringify([bs1, bs2, bs3, bs4]));

  // --- 網頁事件: the page's own JS handlers in its <script id="htdEvents"> block
  const je = require('../lib/jsevents');
  const jeApply = (t, eds) => { let o = t; for (const x of eds.slice().sort((a, b) => b.s - a.s)) o = o.slice(0, x.s) + x.text + o.slice(x.e); return o; };
  const jeP0 = '<html><body>\r\n<button id="b1">x</button>\r\n</body></html>';
  const je1 = je.setBinding(jeP0, 'b1', 'click', 'b1Click');
  const jeP1 = jeApply(jeP0, je1.edits);
  const je2 = je.setBinding(jeP1, 'b1', 'mouseup', 'b1Up');
  const jeP2 = jeApply(jeP1, je2.edits);
  const jeP2b = jeP2.replace('function b1Click(e) {\r\n  // TODO', 'function b1Click(e) {\r\n  alert(1);');
  const je3 = je.setBinding(jeP2b, 'b1', 'click', 'b1Go');        // renamed: its code stays
  const jeP3 = jeApply(jeP2b, je3.edits);
  const je4 = je.setBinding(jeP3, 'b1', 'mouseup', '');           // cleared: the empty function goes too
  const jeP4 = jeApply(jeP3, je4.edits);
  const je5 = je.setBinding(jeP4, 'b1', 'click', '');             // cleared: a function with code stays
  const jeP5 = jeApply(jeP4, je5.edits);
  const jeRid = jeApply(jeP3, je.renameIdEdits(jeP3, 'b1', 'b2'));
  const jeBad = je.setBinding(jeP0, 'b1', 'click', '1x');
  ok(je1.fnAdded === 'b1Click' && /<script id="htdEvents">\r\n\/\*[^\n]*\*\/\r\nfunction b1Click\(e\) \{\r\n  \/\/ TODO\r\n\}\r\ndocument\.getElementById\('b1'\)\.addEventListener\('click', b1Click\);\r\n<\/script>\r\n<\/body>/.test(jeP1) &&
    jeP1.indexOf('<button id="b1">x</button>') > 0 && JSON.stringify(je.handlersOf(jeP2, 'b1')).indexOf('"mouseup":{"fn":"b1Up"') > 0 &&
    je3.renamed === 'b1Click' && /function b1Go\(e\) \{\r\n  alert\(1\);/.test(jeP3) && /addEventListener\('click', b1Go\)/.test(jeP3) && !/b1Click/.test(jeP3) &&
    !/b1Up/.test(jeP4) && /function b1Go/.test(jeP4) && !/addEventListener\('click'/.test(jeP5) && /function b1Go\(e\) \{\r\n  alert\(1\);/.test(jeP5) &&
    /getElementById\('b2'\)\.addEventListener\('click', b1Go\)/.test(jeRid) && /getElementById\('b2'\)\.addEventListener\('mouseup', b1Up\)/.test(jeRid) &&
    jeBad.error && !jeBad.edits.length,
    'jsevents: a name = the function + addEventListener in the page\'s htdEvents block (its line ends); renamed = its code stays; cleared = the binding out (an empty function too); a renamed component\'s bindings follow',
    JSON.stringify(jeP3.slice(jeP3.indexOf('<script')).slice(0, 160)));

  // --- 自動接線 (EastSun 20260930: "如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增"): the page's htdCpp line,
  //     the server's htd.event branch made from form.event's, the CMake entry, the generated table (real files, READ only)
  const cbr = require('../lib/cppbridge');
  const cbServer = fs.readFileSync(path.join(PORT, 'tools', 'wb_serve.cpp'), 'utf8');
  const cbS = cbr.serverHook(cbServer, '20260930');
  const cbS2 = cbS.edit ? jeApply(cbServer, [cbS.edit]) : cbServer;
  const nlOf = s => (s.match(/\n/g) || []).length;
  const cbHead = cbS.edit ? cbS2.slice(cbS.edit.s, cbS.edit.s + cbS.edit.text.length) : '';
  ok(!!((cbS.has && cbS.fn === 'W906_HtdEvent') || (cbS.edit && cbS.fn === 'W906_HtdEvent' && nlOf(cbS2) === nlOf(cbServer) &&
    /^\} else if \(wc\.cmd == "htd\.event"\) \{ \/\*AI\(W906-HTDESIGNER\) 20260930/.test(cbHead) && /extern bool W906_HtdEvent\(const std::string& tag/.test(cbHead) &&
    /server\.CompleteCommand\(/.test(cbHead) && !/"form\.event"|W906_FormEvent/.test(cbHead) && cbS2.slice(cbS.edit.s + cbS.edit.text.length).startsWith('} else if (wc.cmd == "form.event")') &&
    cbr.serverHook(cbS2, 'x').has)),
    'cppbridge.serverHook: the htd.event branch = form.event\'s checks + W906_HtdEvent + CompleteCommand, in front of it on the same line (no line moves); a second time = already there',
    cbS.error || (cbS.has ? 'already hooked' : cbHead.slice(0, 120)));
  const cbCm = fs.readFileSync(path.join(PORT, 'CMakeLists.txt'), 'utf8');
  const cbC = cbr.cmakeHook(cbCm, 'tools/wb_serve.cpp', cbr.GEN_REL);
  const cbCm2 = cbC.edit ? jeApply(cbCm, [cbC.edit]) : cbCm;
  const cbWbs = /add_executable\(wb_serve[\s\S]*?\)\s*#/.exec(cbCm2.replace(/#[^\n]*/g, '#'));
  ok(!!(cbC.has || (cbC.edit && nlOf(cbCm2) === nlOf(cbCm) && cbCm2.slice(cbC.edit.s - 40, cbC.edit.s + 40).includes('MainStateRecord.cpp  HtdEvents/HtdEvents.gen.cpp)') &&
    cbr.cmakeHook(cbCm2, 'tools/wb_serve.cpp', cbr.GEN_REL).has && !cbr.cmakeHook(cbCm, 'tools/nothere.cpp', cbr.GEN_REL).edit)),
    'cppbridge.cmakeHook: the generated file goes in wb_serve\'s add_executable, in front of its ")" (same line); a second time = already there; no such executable = nothing',
    cbC.error || (cbC.has ? 'already listed' : cbCm2.slice(cbC.edit.s - 40, cbC.edit.s + 40)));
  const cbCall = cbr.callOf('TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y', 'fHotPlate', 'spbSaveMouseUp', 'nullptr');
  const cbKey = cbr.callOf('TObject *Sender, WORD &Key, TShiftState Shift', 'fHotPlate', 'XST1KeyDown', 'HtdSender(fHotPlate->XST1)');
  const cbBad = cbr.callOf('TObject *Sender, TPoint &MousePos, bool &Handled', 'fX', 'a', 'nullptr');
  const cbRows = [{ form: 'TfHotPlate', handler: 'spbSaveMouseUp', control: 'spbSave', event: 'OnMouseUp', header: 'forms/fHotPlate.h', call: cbCall }];
  const cbGen = cbr.genFile(cbRows, { fn: 'W906_HtdEvent', cjson: 'Public/cJSON.h', lockHeader: 'JsonBridge/FormJson.h', lock: 'ht9045::formjson::FormLock',
    unlock: 'ht9045::formjson::FormUnlock', running: ['SystemStart', 'SoftStart'], shiftHeader: 'vclcompat/ShiftState.h' }, '\r\n');
  ok(cbCall.call === 'fHotPlate->spbSaveMouseUp(nullptr, HtdButton(a), HtdShift(a), a.x, a.y);' && cbCall.usesShift &&
    cbKey.pre[0] === 'WORD k0 = (WORD)a.key;' && cbKey.call === 'fHotPlate->XST1KeyDown(HtdSender(fHotPlate->XST1), k0, HtdShift(a));' && !!cbBad.error &&
    cbr.globalOf(csH, csC, 'TfHotPlate') === 'fHotPlate' && cbr.hasMember(csH, 'TfHotPlate', 'sbtExit') && !cbr.hasMember(csH, 'TfHotPlate', 'spbSave') &&
    cbr.declOf(csH, 'TfHotPlate', 'sbtExitClick').access === 'public' && cbr.declOf(csH, 'TfHotPlate', 'LoadImage').access === 'private' &&
    JSON.stringify(cbr.rowsOf(cbGen)) === JSON.stringify([{ form: 'TfHotPlate', handler: 'spbSaveMouseUp', control: 'spbSave', event: 'OnMouseUp' }]) &&
    /bool W906_HtdEvent\(const std::string& tag, const std::string& valueJson, std::string\* ack, std::string\* err\)/.test(cbGen) &&
    /if \(SystemStart \|\| SoftStart\)/.test(cbGen) && /ht9045::formjson::FormLock\(\);[\s\S]*r->run\(a\);[\s\S]*ht9045::formjson::FormUnlock\(\);/.test(cbGen) &&
    /#include "forms\/fHotPlate\.h"/.test(cbGen) && /#include "vclcompat\/ShiftState\.h"/.test(cbGen) && !/\n(?!\r)/.test(cbGen.replace(/\r\n/g, '')),
    'cppbridge: the call from the declared parameters (mouse / key / a parameter the page cannot give = refused); the global (fHotPlate), members, access; the generated file (its rows read back, running check, lock, headers)',
    JSON.stringify([cbCall.call, cbKey.pre, cbBad.error]));
  const jc0 = '<html><body>\r\n<button id="b1">x</button>\r\n</body></html>';
  const jc1 = jeApply(jc0, je.setCpp(jc0, 'b1', 'mouseup', 'TfX', 'b1MouseUp', 'OnMouseUp').edits);
  const jc2 = jeApply(jc1, je.setCpp(jc1, 'b1', 'click', 'TfX', 'b1Click', 'OnClick').edits);
  const jcAgain = je.setCpp(jc2, 'b1', 'click', 'TfX', 'b1Click', 'OnClick');
  const jc3 = jeApply(jc2, je.renameCppEdits(jc2, 'TfX', 'b1Click', 'b1Go'));
  const jc4 = jeApply(jc3, je.renameIdEdits(jc3, 'b1', 'b2'));
  const jcWithJs = jeApply(jc2, je.setBinding(jc2, 'b1', 'click', 'b1Js').edits);
  const jcBlk = je.findBlock(jc2);
  let jcParses = false;
  try { new Function(jc2.slice(jcBlk.innerStart, jcBlk.innerEnd)); jcParses = true; } catch (e) { /* no */ }
  ok(JSON.stringify(je.cppLines(jc2).map(x => [x.id, x.type, x.form, x.handler, x.event])) === JSON.stringify([['b1', 'mouseup', 'TfX', 'b1MouseUp', 'OnMouseUp'], ['b1', 'click', 'TfX', 'b1Click', 'OnClick']]) &&
    (jc2.match(/function htdCpp\(/g) || []).length === 1 && jcParses && !jcAgain.edits.length && /rawCmd\('htd\.event'/.test(jc2) && !/\n(?!\r)/.test(jc2.replace(/\r\n/g, '')) &&
    /htdCpp\('b1', 'click', 'TfX', 'b1Go', 'OnClick'\)/.test(jc3) && !/b1Click/.test(jc3) && (jc4.match(/htdCpp\('b2'/g) || []).length === 2 &&
    je.cppLines(jcWithJs).length === 2 && je.bindings(jcWithJs).length === 1 && je.bindings(jc2).length === 0,
    'jsevents C++ lines: htdCpp(id, type, form, handler, event) in the htdEvents block + ONE helper (it parses, sends htd.event); the same again = nothing; a renamed handler / component follows; the page\'s own JS bindings apart',
    JSON.stringify(jc2.slice(jc2.lastIndexOf('}\r\nhtdCpp') + 3).slice(0, 140)));

  // --- Alias: the part of the title the page's JS reads (title.match(/Alias=([^｜|\s]+)/)), the IO table's names
  const al = require('../lib/aliasedit');
  const alT = 'BtnPanelLane3 : TBtnPanelLane｜Alias=C_LoaderEdgePush';
  const alIo = 'D:\\HT9045\\system\\IO_Table.csv';
  const alNames = fs.existsSync(alIo) ? al.ioAliases(alIo) : null;
  ok(al.aliasOf(alT) === 'C_LoaderEdgePush' && al.withAlias(alT, 'C_Other').title === 'BtnPanelLane3 : TBtnPanelLane｜Alias=C_Other' &&
    al.withAlias(alT, '').title === 'BtnPanelLane3 : TBtnPanelLane' && al.withAlias('X : TButton', 'Y1').title === 'X : TButton｜Alias=Y1' &&
    !!al.withAlias(alT, 'a b').error && (alNames === null || (alNames.length > 100 && alNames.includes('C_OTD_Valve_On'))),
    'aliasedit: read / change / take out / add the Alias of a title; the IO table\'s Alias column (read only)', alNames ? alNames.length + ' IO names' : 'no IO table');

  // the real Mot_Table.csv (READ only, in memory): one value changed = only that value differs
  const mtFile = 'D:\\HT9045\\system\\Mot_Table.csv';
  if (fs.existsSync(mtFile)) {
    const mt = fs.readFileSync(mtFile, 'utf8');
    const mtP = cv.parseDoc(mt);
    const mtCell = mtP.rows[1].cells[23];
    const mtNew = applyReps(mt, cv.cellEdits(mtP, [{ r: 1, c: 23, v: '123' }]));
    const mtBack = applyReps(mtNew, cv.cellEdits(cv.parseDoc(mtNew), [{ r: 1, c: 23, v: mtCell.v }]));
    ok(mtP.delim === ',' && mtP.eol === '\r\n' && mtP.rows.length >= 2 && mtNew.length === mt.length - mtCell.v.length + 3 &&
      mtNew.slice(0, mtCell.s) === mt.slice(0, mtCell.s) && mtNew.slice(mtCell.s + 3) === mt.slice(mtCell.e) && mtBack === mt,
      'csv on the real Mot_Table.csv (read only, in memory): one value changed = only those bytes differ; changed back = the same file',
      mtP.rows.length + ' rows, row 2 col 24 "' + mtCell.v + '"');
  }
}

main().catch(e => { fail++; log('CRASH ' + (e && e.stack || e)); }).then(() => {
  log('');
  log('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  const text = out.join('\n') + '\n';
  if (process.argv[2]) fs.writeFileSync(process.argv[2], text, 'utf8');
  else process.stdout.write(text);
  process.exitCode = fail ? 1 : 0;
});
