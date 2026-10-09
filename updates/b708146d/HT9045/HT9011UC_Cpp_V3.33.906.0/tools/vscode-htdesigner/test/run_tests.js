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
  // (1006 audit: a minimal WHOLE PE file of n bytes -- MZ, e_lfanew, PE signature, one section whose raw data ends at the
  //  file's end -- lib/uptodate.js now refuses a cut / zeroed exe, a buffer of zeros no longer passes as built)
  const fakePE = n => { const b = Buffer.alloc(n); b.write('MZ', 0, 'latin1'); b.writeUInt32LE(64, 0x3c); b.writeUInt32LE(0x00004550, 64); b.writeUInt16LE(0x14c, 68); b.writeUInt16LE(1, 70); b.writeUInt16LE(0, 84); b.writeUInt32LE(n - 512, 88 + 16); b.writeUInt32LE(512, 88 + 20); return b; };
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
  // (1007 audit, events P2: a handler the DESIGNER added -- the page's htdCpp line, sent or kept as a comment -- is found too)
  {
    const je = require('../lib/jsevents');
    const tmp = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_rev_'));
    let pt = fs.readFileSync(path.join(WEB, 'page', 'Setup.HotPlate.html'), 'utf8');
    const apE = (t, e) => { for (const x of e.slice().sort((p, q) => q.s - p.s)) t = t.slice(0, x.s) + x.text + t.slice(x.e); return t; };
    pt = apE(pt, je.setCpp(pt, 'Panel1', 'none', 'TfHotPlate', 'Panel1CanResizeX', 'OnCanResize').edits);
    pt = apE(pt, je.setCpp(pt, 'spbSave', 'mouseup', 'TfHotPlate', 'spbSaveMouseUpX', 'OnMouseUp').edits);
    fs.writeFileSync(path.join(tmp, 'Setup.HotPlate.html'), pt);
    const rv2 = new ReverseIndex(ir, [tmp]);
    const a1 = rv2.handler('TfHotPlate', 'Panel1CanResizeX'), a2 = rv2.handler('TfHotPlate', 'spbSaveMouseUpX'), a3 = rv2.handler('TfHotPlate', 'spbSaveClick');
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { }
    ok(a1.length === 1 && a1[0].node === 'Panel1' && a1[0].event === 'OnCanResize' && a2.length === 1 && a2[0].node === 'spbSave' && a3.length === 1,
      'reverse: handlers the designer added (htdCpp lines, a commented one too) are found like the .dfm ones', JSON.stringify({ a1: a1.map(e => e.node + '.' + e.event), a2: a2.map(e => e.node + '.' + e.event), a3: a3.length }));
  }
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
  {
    // 1009 second review (DFM #3): a Main.* page is one sheet / group of main.dfm -- only what is under it
    const mst = new pageinfo.IrStore(path.join(__dirname, '..', '..', 'dfm2rc', 'ir_out'));
    const mir = mst.load(mst.find('main'));
    const sc1 = pageinfo.scopeOf('AOA Info（main.dfm ts1）', mir), sc2 = pageinfo.scopeOf('Logs（main.dfm tsLogs＋tsMNetLog）', mir);
    const sc3 = pageinfo.scopeOf('Control Buttons（main.dfm / gbControlBtn）', mir), sc4 = pageinfo.scopeOf('HT-9132 主視窗（main.dfm / TMainForm）', mir);
    const ovAll = await ovm.buildOverview({ ir: mir, pageIds: new Set(), listeners: {}, fieldIds: new Set(), tagIds: new Set(), quoted: new Set(), classes: [], port: null, gold: null });
    const ovBtn = await ovm.buildOverview({ ir: mir, scope: sc3, pageIds: new Set(), listeners: {}, fieldIds: new Set(), tagIds: new Set(), quoted: new Set(), classes: [], port: null, gold: null });
    ok(sc1 && sc1.length === 1 && sc1[0].name === 'ts1' && sc2 && sc2.length === 2 && sc3 && sc3[0].name === 'gbControlBtn' && sc4 === null &&
      ovBtn.rows.length > 0 && ovBtn.rows.length < ovAll.rows.length && ovBtn.rows.every(r => r.id !== '@form' && /\.gbControlBtn(\.|$)/.test(r.path)),
      'scopeOf: a Main.* title names its sheets / group (the class = the whole form); the overview counts only those',
      JSON.stringify({ sc1, sc2: sc2 && sc2.map(x => x.name), all: ovAll.rows.length, btn: ovBtn.rows.length }));
  }
  {
    // 1009 second review (search #10): a regex literal in the page's code is no string / comment start
    const WS = require('../lib/websearch');
    const t = String.raw`x.replace(/[&<>"]/g, f); var a = "btnX"; y = a / 2; // btnZ` + '\n' + String.raw`z(/\/\//, "btnY"); w = b / c / "btnW"`;
    const s = new WS.Source('a.js', t, false);
    const r = ['btnX', 'btnZ', 'btnY', 'btnW'].map(w => s.inComment(t.indexOf(w)));
    ok(r.join() === 'false,true,false,false', 'websearch Source.inComment: /[&<>"]/g and /\\/\\// are regex literals (no string / comment), a / b division is not', JSON.stringify(r));
  }
  {
    // 1009 second review (debug #5 #6 #8, csv #9)
    const fe = require('../lib/finderror'), bk = require('../lib/bcbkeys'), ct = require('../lib/csvtable');
    const ev = 'Faulting application name: wb_serve.exe\nFaulting module name: wb_serve.exe, version: 0.0.0.0\nFault offset: 0x0012a3b4\nFaulting application start time: 0x01db1a2b3c4d5e6f\nReport Id: 1a2b3c4d-1111-2222-3333-444455556666';
    const a1 = fe.parseAddrs(ev, 0x400000n, 'wb_serve.exe'), a2 = fe.parseAddrs('Faulting module name: ntdll.dll\nFault offset: 0x1234', 0x400000n, 'wb_serve.exe');
    const oldF2 = '[\n  { "key": "ctrl+f2", "command": "ht9045Designer.run.stopAll", "when": "ht9045Designer.runCanStop && config.ht9045Designer.bcbDebugKeys" }\n]\n';
    const up = bk.ensure(oldF2);
    const f2 = bk.KEYS.find(k => k.key === 'ctrl+f2');
    const wide = new Map(); for (let i = 0; i < 200000; i++) wide.set(i, 'x');
    let wideOk = true; try { ct.cellEdits(ct.parseDoc('a,b\n', ','), Array.from({ length: 150000 }, (x, i) => ({ r: 0, c: i, v: '' })).slice(149990)); } catch (e) { wideOk = false; }
    const hk = ct.decodeSafe(Buffer.from('ID,Name\n1,AB\n'), 'ID,Name\n1,AB\n', 'big5hkscs', 'D:\\HT9045\\system\\x.csv');
    ok(a1.addrs.length === 1 && a1.addrs[0] === '0x52a3b4' && /不是這支程式/.test(a2.error || '') &&
      typeof up === 'string' && (up.match(/ctrl\+f2/g) || []).length === 1 && /editorTextFocus/.test(f2.when) && wideOk && hk.big5 === true,
      '1009 second review: Event Log "Fault offset" of this exe (start time / Report Id not addresses; another module refused); the old Ctrl+F2 line replaced by the one not taking an editor; a wide row; Big5-HKSCS checked as Big5',
      JSON.stringify({ a1, a2, f2: f2.when, hk }));
  }
  {
    // 1009 second review (C++ nav #3 #4 #7 #10): an apostrophe in an #if 0 note, a raw string, a brace open in one branch,
    // the build's \" defines
    const st = require('../lib/cppstub'), dc = require('../lib/deadcode'), cs = require('../lib/cppsymbols'), be = require('../lib/builderrors');
    const dr = dc.deadRanges("#if 0\nit's a note\n#endif\nint live;\n");
    const raw = 'int a = 1;\nconst char* r = R"x(line1\n"q" line2\n)x";\nint b;\n';
    const ms = st.mask(raw);
    const sy = cs.symbolsOf('void A(){\n#if 0\n if(x){\n#else\n if(y){\n#endif\n }\n}\nclass TfX {\n#if 0\n void Old(int a){\n#else\n void New(int a){\n#endif\n }\n};\n')
      .map(s => s.kind + ':' + s.name + '[' + (s.children || []).map(c => c.name).join(',') + ']').join(' ');
    const sp = be.split('-DA=\\"D:/x\\" -DB=1 "-IC:/a b"');
    ok(dr.length === 1 && dr[0].from === 1 && dr[0].to === 1 && ms.length === raw.length && ms.split('\n').length === raw.split('\n').length && !/q/.test(ms) &&
      sy === 'function:A[] class:TfX[New]' && sp[0] === '-DA="D:/x"' && sp[2] === '-IC:/a b',
      '1009 second review (C++ nav): an unclosed quote keeps its line break (the code after #endif not dead); a raw string keeps its lines; #if 0 copies out of the outline (a brace open in one branch no longer drops the class); \\" defines unescaped',
      JSON.stringify({ dr, sy, sp }));
  }
  {
    // 1009 second review (solution #1 #2 #4, wiring #6 #7)
    const cl = require('../lib/cmakelists'), je = require('../lib/jsevents'), ws = require('../lib/websearch');
    const cm = 'add_subdirectory(tests)\ntarget_link_libraries(x PUBLIC y)\nadd_executable(wb_serve tools/a.cpp $<$<PLATFORM_ID:Windows>:tools/wb_serve.rc>)\nadd_library(p OBJECT ${DEV} E/R.cpp)\n';
    const tr = cl.refsUnder(cm, 'tests', '').map(r => cm.slice(r.s, r.e));
    const pr = cl.refsUnder(cm, 'PUBLIC', '').length;
    const rc = cl.refsAll(cm, 'tools/wb_serve.rc', '').length;
    const em = cl.removeSource(cm, 'E/R.cpp', '').empties;
    const pg = '<html><body><script>function save(){go()}</script><button id="b"></button></body></html>';
    const jb = je.setBinding(pg, 'b', 'click', 'save');
    const tc = ws.traceCmds([], "// rawCmd('io.old')\ncmd('x.y');", 2).map(x => x.cmd);
    ok(tr.join() === 'tests' && pr === 0 && rc === 1 && em.join() === 'p' && jb.existing === true && jb.fnAdded === null && !/function save/.test(jb.edits[0].text) && tc.join() === 'x.y',
      '1009 second review: add_subdirectory(tests) follows the folder (PUBLIC still not a folder); a path in $<…:…> found; a target left with only ${VAR} is empty; the page\'s own function bound to (no stub over it); a command in a comment is not the event\'s',
      JSON.stringify({ tr, pr, rc, em, jb, tc }));
  }
  {
    // 1009 second review (edit core #2 #3 #4 #5 #8)
    const he2 = require('../lib/htmledit'), hb2 = require('../lib/htmlblock'), mx2 = require('../lib/mixed');
    const st = he2.setStyle('<span class="pnlCap" style="left:1px;font-family:&quot;Arial&quot;,sans-serif;font-weight:700;">', { 'font-family': 'Tahoma' });
    const ids = hb2.idsIn('<span title="A->B" id="L1"><b id=\'x\'>');
    const pg = '<html><body>\n<div id="A" style="position:absolute"></div>\n<div id="B" style="position:absolute"></div>\n<script src="x.js"></script>\n</body></html>';
    const r = hb2.reorder(pg, hb2.unitOf(pg, 'A', 'self'), 'front');
    const n = '<div id="P">\n  <!-- note about A -->\n  <div id="A"></div>\n  <div id="B"></div>\n  <div id="C"></div>\n</div>';
    const r2 = hb2.reorder(n, hb2.unitOf(n, 'C', 'self'), 'back');
    let o = n; for (const p of r2.parts.slice().sort((a, b) => b.range[0] - a.range[0])) o = o.slice(0, p.range[0]) + p.repl + o.slice(p.range[1]);
    const mxd = mx2.mixedFields([{ id: 'a', cap: { value: 'Start' } }, { id: 'b', cap: { value: 'START' } }]);
    ok(/font-family:Tahoma;font-weight:700;/.test(st) && ids.join() === 'L1,x' && r.parts && !/<\/script>/.test(r.parts[0].repl) &&
      /<!-- note about A -->\s*<div id="A">/.test(o) && /id="C"[\s\S]*note about A/.test(o) && mxd.caption === true,
      '1009 second review (edit core): &quot; in a style is one unit (bold kept); an id after "->" in a title found; Bring to Front stays before the page\'s scripts; a moved one does not split a note from its element; caption case counts as mixed',
      JSON.stringify({ st, ids, r, o, mxd }));
  }
  {
    // 1009 second review (toolbox #1 #6, lint #2 #8)
    const ds = require('../lib/defineswitch'), pl = require('../lib/pagelint'), ck = require('../lib/cppcheck');
    const sw = ds.list('#ifndef G\n#define G\n/*#ifdef A\n#endif\n#endif*/\n#ifdef S\n#define FOO\n#endif\n#define TOP\n#define X /* a\nb */\n//#define OFF // note\n').map(x => x.name + ':' + x.on);
    const ckE = ck.check('#error 中文\nint a;\n', null, {}).filter(p => p.kind === 'nonascii').length;
    ok(sw.join() === 'TOP:true,OFF:false' && ckE === 0,
      '1009 second review: #define switches -- a /* #ifdef … #endif */ block does not shift the depth, a #define in a block comment or with a comment running on is no switch; the text of #error is not code',
      JSON.stringify({ sw, ckE }));
  }
  {
    // 1009 regression review (#2 #6 #7 #8 #9, classBody)
    const st = require('../lib/cppstub'), ds = require('../lib/defineswitch'), he3 = require('../lib/htmledit'), be3 = require('../lib/builderrors'), bk = require('../lib/bookmarks');
    const raw = 'int a; auto s = R"x(abc';
    const note = ds.list('#define FOO /* the foo switch */\n')[0];
    const ent = he3.setStyle('<div style="font-family:&quot;Arial&#34;;color:red">', { color: 'blue' });
    const sp = be3.split(String.raw`-I\\srv\inc -DA=\"x\" "-IC:/a b"`);
    const sh = bk.shift({ 1: { file: 'a', line: 5, text: 'x' } }, 'a', 0, 0, 2, 0);
    const cb = st.classBody('#if 0\nclass TA { int old; };\n#endif\nclass TA { int live; };\n', 'TA');
    ok(st.mask(raw).length === raw.length && note && note.note === 'the foo switch' && /color:blue">$/.test(ent) && !/color:red/.test(ent) &&
      sp[0] === String.raw`-I\\srv\inc` && sp[1] === '-DA="x"' && sh[1].text === 'x' && sh[1].line === 6 && cb && cb.open > 30,
      '1009 regression review: an open raw string keeps the length; a /* note */ of a switch kept; &quot; and &#34; are one quote; only \\" unescaped (\\\\server kept); a moved bookmark keeps its text; classBody takes the live class',
      JSON.stringify({ n: note, ent, sp, sh, cb }));
  }
  {
    // 1009 second review (toolbox #7): ini names without case, the first one only (as TIniStore / kernel32 read them)
    const lc = require('../lib/liveconfig');
    const f = path.join(require('os').tmpdir(), 'htd_ini_case_' + process.pid + '.ini');
    fs.writeFileSync(f, '[System]\nModel=A\nmodel=B\n[SYSTEM]\nX=1\n[Other]\nY=2\n');
    const sec = lc.iniToDoc(f).sections;
    try { fs.unlinkSync(f); } catch (e) { /* temp */ }
    ok(Object.keys(sec).join() === 'System,Other' && Object.keys(sec.System).join() === 'Model' && sec.System.Model.value === 'A',
      'liveconfig: [System] then [SYSTEM], Model= then model= -- the first one only, names compared without case', JSON.stringify(sec));
  }

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
  // 1006 (deleting HW.home's Panel2 hung the extension): a comment ending right before the tag -- and a '>' no tag ends
  // at, in a text that starts with '<' -- must come back at once (lastIndexOf('<', -1) is 0 again: s stayed 0 for ever)
  {
    const w1t = '<!DOCTYPE html><div><!-- Panel2->Visible=false --><div id="P2" style="position:absolute">x</div></div>';
    const w2t = '<p>a -> b><div id="Q" style="position:absolute"></div>';
    const tw0 = Date.now();
    const w1 = he.wrapperTagOf(w1t, he.startTagOf(w1t, 'P2').start);
    const w2 = he.wrapperTagOf(w2t, he.startTagOf(w2t, 'Q').start);
    // (1007 audit, toolbox #8: the comment is skipped now -- the tag before it is found; that plain <div> is no wrapper
    //  for a structural command (htmlblock.wrapsOnly: not position:absolute). A "-->" with no "<!--" = none, at once)
    const w3t = 'x --><div id="R" style="position:absolute"></div>';
    const w3 = he.wrapperTagOf(w3t, he.startTagOf(w3t, 'R').start);
    const hbw = require('../lib/htmlblock');
    ok(w1 && w1.text === '<div>' && !hbw.wrapsOnly(w1t, he.startTagOf(w1t, 'P2')) && w2 === null && w3 === null && Date.now() - tw0 < 100,
      'wrapperTagOf: past a comment to the tag before it (no wrapper for a structural command unless position:absolute); a stray ">" or "-->" walks back to the start and stops (it looped for ever)', JSON.stringify({ w1, w2, w3, ms: Date.now() - tw0 }));
  }
  const hid = he.setStyle(x1.text, { display: 'none' });
  ok(/display:none;/.test(hid) && he.setStyle(hid, { display: null }) === x1.text, 'setStyle add then remove display:none restores the tag byte for byte');
  // hand-written style with spaces and no final ';': only the changed value moves
  const hw = '<div id="a" style="position: absolute; left: 10px;  top : 5px">';
  ok(he.setStyle(hw, { left: '12px' }) === '<div id="a" style="position: absolute; left: 12px;  top : 5px">', 'setStyle edits one value in place (spaces kept)');
  ok(he.setStyle(hw, { width: '30px' }) === '<div id="a" style="position: absolute; left: 10px;  top : 5px;width:30px">' && he.setStyle(he.setStyle(hw, { width: '30px' }), { width: null }) === hw, 'setStyle appends a new property after a missing ";" (as ";decl": removed again = byte for byte, 1007)');
  ok(he.setStyle('<p id="b">', { left: '1px' }) === '<p id="b" style="left:1px;">', 'setStyle adds a style attribute when there is none');
  ok(he.setStyle('<p id="c" style="left:1px;top:2px;left:3px;">', { left: '9px' }) === '<p id="c" style="left:1px;top:2px;left:9px;">', 'setStyle edits the last of duplicates and keeps the others (1007: the generator writes width:136px;…;width:auto)');
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
  // 0.152 the rows that were DFM-only: Font.Underline / StrikeOut, WordWrap, ReadOnly, MaxLength, Checked, TabOrder (VCL defaults)
  {
    const e1 = fmt.dfmEditValues({ class: 'TEdit', geometry: {}, properties: { 'Font.Style': { type: 'SET', value: ['fsUnderline'] }, MaxLength: { type: 'SCALAR', value: 8 }, TabOrder: { type: 'SCALAR', value: 3 }, ReadOnly: { type: 'SCALAR', value: 'True' } } });
    const e2 = fmt.dfmEditValues({ class: 'TCheckBox', geometry: {}, properties: {} });
    const e3 = fmt.dfmEditValues({ class: 'TLabel', geometry: {}, properties: { WordWrap: { type: 'SCALAR', value: 'True' } } });
    const he = require('../lib/htmledit');
    const lt = '<div><label class="ckb" id="CB1" title="CB1 : TCheckBox"><input type="checkbox" >CB1</label><input id="X" type="checkbox"></div>';
    const lab = he.startTagOf(lt, 'CB1'), inn = lab ? he.innerInputTag(lt, lab) : null;
    const set = inn ? lt.slice(0, inn.start) + he.setAttr(inn.text, 'checked', true) + lt.slice(inn.end) : '';
    const lt2 = '<label class="ckb" id="CB2">no box</label><input id="Y" type="checkbox">';
    const none = he.innerInputTag(lt2, he.startTagOf(lt2, 'CB2'));
    const got = { e1: [e1.underline, e1.strikeout, e1.maxLength, e1.tabOrder, e1.readOnly, e1.checked, e1.wordWrap], e2: [e2.checked, e2.readOnly, e2.maxLength], e3: e3.wordWrap, set, none };
    ok(JSON.stringify(got.e1) === '[true,false,8,3,true,null,null]' && JSON.stringify(got.e2) === '[false,null,null]' && got.e3 === true &&
      /<label class="ckb" id="CB1" title="CB1 : TCheckBox"><input type="checkbox" checked ?>CB1<\/label><input id="X" type="checkbox">/.test(set) && none === null,
      'dfmEditValues 0.152: Underline / StrikeOut / MaxLength / TabOrder / ReadOnly / Checked / WordWrap with VCL defaults per class; innerInputTag = the <input> in a check box\'s <label> only (not one after it)',
      JSON.stringify(got));
  }

  // --- the IO lamp / panel button (EastSun 20261001: the old components' properties in the designer)
  const iol = fmt.dfmEditValues({ class: 'TMyLedLane', geometry: {}, properties: { LEDStyle: { type: 'SCALAR', value: 'LEDHorizontal' },
    TrueColor: { type: 'SCALAR', value: 'clLime' }, FalseColor: { type: 'SCALAR', value: 12632256 }, Value: { type: 'SCALAR', value: 'True' } } }).io;
  const iob = fmt.dfmEditValues({ class: 'TBtnPanelLane', geometry: {}, properties: { Style: { type: 'SCALAR', value: 'tsFlatButtons' },
    TrueColor: { type: 'SCALAR', value: 14464261 }, FalseColor: { type: 'SCALAR', value: 10307329 }, TrueFontColor: { type: 'SCALAR', value: 'clWhite' },
    FalseFontColor: { type: 'SCALAR', value: 'clBtnText' } } }).io;
  ok(iol.ledStyle === 'LEDHorizontal' && iol.trueColor === '#00ff00' && iol.falseColor === '#c0c0c0' && iol.value === true && iol.blink === null &&
    iob.flat === true && iob.trueColor === '#05b5dc' && iob.falseColor === '#01479d' && iob.trueFontColor === '#ffffff' && iob.falseFontColor === null && iob.down === null,
    'dfmEditValues io: LEDStyle / TrueColor / FalseColor / Value of a lamp, Style=tsFlatButtons / the four colours of a button (a system colour not compared, Down not written = null)',
    JSON.stringify({ iol, iob }));
  ok(he.setClass('<span class="aled LEDHorizontal" id="l1" style="--led-on:#00ff00;">', ['LEDSqLarge'], ['LEDHorizontal', 'LEDSmall']) ===
    '<span class="aled LEDSqLarge" id="l1" style="--led-on:#00ff00;">' &&
    he.setClass('<div class="btnpanel flat" id="b1">', ['down'], []) === '<div class="btnpanel flat down" id="b1">' &&
    he.setClass('<div class="btnpanel flat down" id="b1">', [], ['down']) === '<div class="btnpanel flat" id="b1">' &&
    he.setClass('<div class="btnpanel down" id="b1">', ['down'], []) === '<div class="btnpanel down" id="b1">' &&
    he.setClass('<div id="b2">', ['on'], []) === '<div id="b2" class="on">',
    'setClass: in place (the id / style stay where they are), no duplicates, removed, a class attribute added when there is none');
  {
    // 1009 review (dispatch table / page events)
    const cbr = require('../lib/cppbridge'), jev = require('../lib/jsevents');
    const hA = 'class TfA : public TForm {\npublic:\n  TPanel *p1, *p2, *p3;\n  TButton* b{nullptr};\n  void __fastcall bClick(TObject *Sender);\n};\nextern PACKAGE TfA *fA;\n';
    const hP = 'class TfPad : public TForm {\npublic:\n  void x();\n};\n#define fPad (W906_PadObject())\n';
    const cP = 'void f() {\n    TfPad* p=fPad;\n}\n';
    const cL = 'static TfLoc *g_l = 0;\nvoid g() {\n  TfLoc *m = 0;\n}\n';
    const gotCb = { gA: cbr.globalOf(hA, '', 'TfA'), gP: cbr.globalOf(hP, cP, 'TfPad'), gL: cbr.globalOf('class TfLoc {};', cL, 'TfLoc'),
      mem: ['p1', 'p2', 'p3', 'b'].map(n => cbr.hasMember(hA, 'TfA', n)).join(','), fc: !!cbr.declOf(hA, 'TfA', 'bClick'),
      sel: !!cbr.callOf('TObject *Sender, int ACol, int ARow, bool &CanSelect', 'o', 'f', 's').error, row: cbr.rowsOf('// htd-row TfX hClick btn-save OnClick\n').length };
    const hN = 'class TfX : public TForm {\nprivate:\n  struct Inner { public: int a; };\n  void hClick(TObject *Sender);\npublic:\n  void pClick(TObject *Sender);\n};\n';
    ok(cbr.declOf(hN, 'TfX', 'hClick').access === 'private' && cbr.declOf(hN, 'TfX', 'pClick').access === 'public',
      '1009 review (dispatch table #12): a nested struct\'s "public:" does not make a private handler after it public');
    const rn1 = cbr.runName({ form: 'TfA', handler: 'aClick', control: 'btn-a', event: 'OnClick' }), rn2 = cbr.runName({ form: 'TfA', handler: 'aClick', control: 'btn_a', event: 'OnClick' });
    ok(/^HtdGenRun_TfA_aClick_btn_a_OnClick_[0-9a-f]+$/.test(rn1) && rn1 !== rn2 && rn1 === cbr.runName({ form: 'TfA', handler: 'aClick', control: 'btn-a', event: 'OnClick' }),
      '1009 review (dispatch table #4): a row\'s run function is named by its form / handler / control / event (+ hash) -- a part file left from before can no longer give another row its handler', rn1 + ' / ' + rn2);
    ok(gotCb.gA === 'fA' && gotCb.gP === 'fPad' && gotCb.gL === null && gotCb.mem === 'true,true,true,true' && gotCb.fc && gotCb.sel && gotCb.row === 1,
      '1009 review (dispatch table): extern PACKAGE / a #define global; a local or static in the .cpp is no global; several members on one line and {nullptr}; void __fastcall; ACol / ARow refused; a control id with "-" read back',
      JSON.stringify(gotCb));
    const pg = '<html><body><label class="ckb" id="ck1"><input type="checkbox">x</label><select id="cb1"></select><button id="bt1">b</button></body></html>';
    const e1 = jev.setCpp(pg, 'bt1', 'none', 'TfA', 'bClick', 'OnClick').edits;
    let pg2 = pg; for (const x of e1.slice().sort((a, b) => b.s - a.s)) pg2 = pg2.slice(0, x.s) + x.text + pg2.slice(x.e);
    const e2 = jev.setCpp(pg2, 'bt1', 'click', 'TfA', 'bClick', 'OnClick').edits;
    let pg3 = pg2; for (const x of e2.slice().sort((a, b) => b.s - a.s)) pg3 = pg3.slice(0, x.s) + x.text + pg3.slice(x.e);
    const ren = jev.renameCppEdits("<script id=\"htdEvents\">\nhtdCpp('ed1', 'change', 'TfA', 'change', 'OnChange');\n</script>", 'TfA', 'change', 'NewName');
    const gotJe = { helper: /function htdCpp/.test(pg3), ck: jev.domTypeFor(pg, 'ck1', 'OnClick'), cb: jev.domTypeFor(pg, 'cb1', 'OnSelect'), bt: jev.domTypeFor(pg, 'bt1', 'OnClick'), en: jev.domTypeFor(pg, 'bt1', 'OnEnter'),
      ren: ren.length === 1 ? "htdCpp('ed1', 'change', 'TfA', 'change', 'OnChange');".slice(0, ren[0].s - 22) : '-' };
    {
      const ap2 = (t0, es) => { let t1 = t0; for (const x of es.slice().sort((a, b) => b.s - a.s)) t1 = t1.slice(0, x.s) + x.text + t1.slice(x.e); return t1; };
      const p0 = '<html><body>\n<button id="b1">x</button>\n</body></html>';
      let p1 = ap2(p0, jev.setCpp(p0, 'b1', 'click', 'TfA', 'bClick', 'OnClick').edits);
      const bs1 = jev.findBlock(p1).start;
      p1 = p1.slice(0, bs1) + '<script src="c.js" data-htd-client></script>\n' + p1.slice(bs1);
      const p2 = ap2(p1, jev.setCpp(p1, 'b1', 'click', 'TfA', '', 'OnClick').edits);
      ok(p2 === p0, '1009 review (web events #9): the command client the designer added (data-htd-client) goes with the block -- added and taken off = the page byte for byte', JSON.stringify(p2));
    }
    ok(gotJe.helper && gotJe.ck === 'change' && gotJe.cb === 'change' && gotJe.bt === null && gotJe.en === 'focusin' && ren.length === 1 && ren[0].e - ren[0].s === 6,
      '1009 review (page events): a commented line made live brings the helper; a check box / ComboBox OnClick / OnSelect = change, OnEnter = focusin; a handler named "change" renamed in its own field',
      JSON.stringify(gotJe));
  }
  {
    const wsf = require('../lib/websearch'), hed = require('../lib/htmledit');
    const pt = '<div class="pcWrap" id="pg1"><div class="pcPane" data-p="0" title="tsA">x</div></div><script>if(i<q.length) el.id="pg1";</script>';
    const fa = wsf.findIdAttr(pt, 'tsA');
    const inScript = hed.inRanges(hed.deadRanges(pt), pt.indexOf('i<q') + 1);
    ok(!!fa && pt.slice(fa.start, fa.start + 4) === '<div' && inScript,
      '1009 review (sync #1 / #4): a tab sheet (pcPane, no id) is found by its title; a place inside <script> is no markup', JSON.stringify({ fa, inScript }));
  }
  {
    const hbl = require('../lib/htmlblock'), tbl = require('../lib/toolbox');
    const lh = '<div class="form">' + tbl.itemOf('TMyLabeledLedLane').html('MyLabeledLedLane1', 10, 20) + '</div>';
    let ls2 = lh;
    for (const e of hbl.renameEdits(lh, 'MyLabeledLedLane1', 'ledReady', { caption: true }).sort((a, b) => b.range[0] - a.range[0])) ls2 = ls2.slice(0, e.range[0]) + e.repl + ls2.slice(e.range[1]);
    ok(!ls2.includes('MyLabeledLedLane1') && (ls2.match(/ledReady/g) || []).length === 6 && /<span class="lled[^>]*title="ledReady : TMyLabeledLedLane｜Caption=ledReady /.test(ls2),
      '1009 review (toolbox #9): a labeled lamp renamed -- the outer span\'s title and Caption=, the caption text and its lbl name follow', ls2.slice(0, 200));
  }
  {
    const hbn = require('../lib/htmlblock');
    const tn = '<div>\n  <!-- other note -->\n  <!-- AI(W906) btnA: moved -->\n  <button id="btnA">a</button>\n  <i>x</i><!-- btnB note -->\n  <button id="btnB">b</button>\n</div>';
    const ia = tn.indexOf('<button id="btnA"'), ib = tn.indexOf('<button id="btnB"');
    const na = tn.slice(hbn.leadNotes(tn, ia, ['btnA']), ia), nb = tn.slice(hbn.leadNotes(tn, ib, ['btnB']), ib);
    ok(na === '<!-- AI(W906) btnA: moved -->\n  ' && nb === '',
      '1009 review (design #8): Delete takes the comments right above a component that name it (own lines only); another note / one sharing a line stays', JSON.stringify({ na, nb }));
  }
  ok(he.setClass('<span class=lbl id=x>', ['ro'], []) === '<span class="lbl ro" id=x>' &&
    he.setStyle('<span title="a style=\'left:1px\'" id=x>', { left: '5px' }) === '<span title="a style=\'left:1px\'" id=x style="left:5px;">' &&
    he.setStyle('<span style=left:1px;top:2px id=x>', { left: '5px' }) === '<span style="left:5px;top:2px" id=x>' &&
    he.setStyle('<span style id=x>', { left: '5px' }) === '<span id=x style="left:5px;">' &&
    he.styleOf('<i data-a=" style=\'x:1\'">').range === null,
    '1009 review: class= / style= without quotes are found (quoted first), a "style=" inside another attribute is not the style, a bare "style" goes');
  const iohtml = '<p><span class="aled LEDHorizontal" id="l1" style="--led-on:#00ff00;--led-off:#c0c0c0;"></span><div class="btnpanel flat" id="b1" style="--bp-true:#05b5dc;--bp-false-font:#ffffff;">x</div></p>';
  const rngT = he.propRange(iohtml, 'l1', 'FalseColor'), rngS = he.propRange(iohtml, 'l1', 'LEDStyle'), rngF = he.propRange(iohtml, 'b1', 'FalseFontColor');
  ok(rngT && iohtml.slice(rngT[0], rngT[1]) === '#c0c0c0' && rngS && iohtml.slice(rngS[0], rngS[1]) === 'aled LEDHorizontal' && rngF && iohtml.slice(rngF[0], rngF[1]) === '#ffffff',
    'propRange for the IO properties: FalseColor -> --led-off\'s value, LEDStyle -> the class, FalseFontColor -> --bp-false-font');

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
  {
    // 1009 review (IO #1 / #4): a panel button's / lamp's colours are its True / False(Font)Color -- Font.Color / Color not
    //   compared (改回 DFM wrote an inline color over the down rule); Value not written in the .dfm = False
    const irDir = path.join(__dirname, '..', '..', 'dfm2rc', 'ir_out');
    const st = new pageinfo.IrStore(irDir);
    const io = st.load(path.join(irDir, 'iosetview.dfm.ir.json'));
    const bLook = Object.assign({}, look0, { color: '#ffffff', background: '#05b5dc', io: { kind: 'btn', flat: true, down: true, trueColor: null, falseColor: null, trueFontColor: null, falseFontColor: null } });
    const br = dd.diffPage([{ id: 'btnC_OutputRotateKIT', lay: null, cap: null, look: bLook }], io).rows.map(x => x.prop);
    const ledName = Array.from(io.byName.keys()).find(k => { const c = io.byName.get(k); return c && /^(TMyLedLane|TALed|TMyLed)$/.test(c.cls || c.class || '') && !(c.properties && c.properties.Value); });
    const lLook = Object.assign({}, look0, { io: { kind: 'led', ledStyle: null, value: true, blink: false, trueColor: null, falseColor: null } });
    const lr = ledName ? dd.diffPage([{ id: ledName, lay: null, cap: null, look: lLook }], io).rows.map(x => x.prop + ' ' + x.page + '/' + x.dfm) : [];
    ok(!br.includes('Font.Color') && !br.includes('Color') && lr.includes('Value True/False'),
      '1009 review (IO): a panel button / lamp has no Font.Color / Color difference (its True / False colours are); a lamp ticked Value whose .dfm writes none differs from False',
      JSON.stringify({ br, ledName, lr }));
  }
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
  {
    // 1009 second review (DFM #1 #2 #4): a labeled lamp's own place / size; a control made another kind on purpose; a cut style
    const lampR = dd.diffPage([{ id: 'spbSave', lay: { left: 51, top: 6, width: 300, height: 30, rendered: true, lamp: { dl: 3, dt: 2, w: 20, h: 40 } }, cap: null, look: Object.assign({}, look0, { bold: false }) }], d).rows;
    const lw = lampR.find(r => r.prop === 'Width');
    const kindR = dd.diffPage([{ id: 'spbSave', cls: 'TZzOther', lay: { left: 60, top: 8, width: 227, height: 40, rendered: true }, cap: { kind: 'text', value: 'Save' }, look: look0 }], d).rows;
    const cutR = dd.diffPage([{ id: 'spbSave', lay: null, cap: null, look: look0 }], d, { cut: new Set(['spbSave']) }).rows;
    const pl = require('../lib/pagelint');
    const cids = pl.cutIds('<div id="p1" style="font-family:"Arial";font-weight:700"><span class="pnlCap" style="font:"X";color:red">c</span></div><div id="p2" style="left:1px">');
    ok(!lampR.some(r => r.prop === 'Left' || r.prop === 'Top' || r.prop === 'Height') && lw && lw.page === '20' && lw.reset === null &&
      kindR.find(r => r.prop === 'Left').reset && kindR.find(r => r.prop === 'Font.Bold').reset === null && /刻意改成/.test(kindR.find(r => r.prop === 'Font.Bold').note) &&
      cutR.length === 1 && cutR[0].reset === null && /引號切斷/.test(cutR[0].note) && cids.has('p1') && !cids.has('p2'),
      '1009 second review: a lamp compared by its own offsets / size (size not reset); another kind keeps place resets only; a cut style refuses ↺; cutIds',
      JSON.stringify({ lampR, kindR, cutR, cids: Array.from(cids) }));
  }
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
  ok(mx1.left && mx1.bold && mx1.color && !mx1.top && !mx1.width && !mx1.fontName && !mx1.fontSize && mx1.caption && !mxd.mixedFields([mxA, Object.assign({}, mxB, { cap: { value: 'Save ' } })]).caption && Object.keys(mx2).length === 0 && Object.keys(mxd.mixedFields([mxA])).length === 0,
    'mixedFields: left / bold / color differ; a font name in another case is the same, a caption in another case is not (1009 second review: as WinForms / BCB6), trailing blanks aside; a missing caption does not count', JSON.stringify(mx1));
  // AI(W906-HTDESIGNER) 20261001: the IO lamp's own -- two lamps of different shapes: LEDStyle mixed, the same TrueColor not;
  // a lamp with a panel button: the button's Down alone is not "mixed"
  const mxL1 = { id: 'L1', look: { io: { kind: 'led', ledStyle: 'LEDSmall', trueColor: '#00ff00' } } };
  const mxL2 = { id: 'L2', look: { io: { kind: 'led', ledStyle: 'LEDLarge', trueColor: '#00ff00' } } };
  const mxB1 = { id: 'B1', look: { io: { kind: 'btn', down: true } } };
  const mxIo = mxd.mixedFields([mxL1, mxL2]), mxIo2 = mxd.mixedFields([mxL1, mxB1]);
  ok(mxIo['io.ledStyle'] === true && !mxIo['io.trueColor'] && !mxIo2['io.down'] && mxd.FIELDS.indexOf('io.ledStyle') >= 0,
    'mixedFields: the IO lamp / panel button fields (io.*) -- different shapes = mixed, the same colour not, one kind alone not', JSON.stringify([mxIo, mxIo2]));

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
    const u = hbMod.componentUnit(h, 'X1');   // (1008: the component's unit -- a TLabeledEdit / labeled LED span too)
    const hostR = hbMod.elementRange(h, he.startTagOf(h, 'host'));
    return !t0 || !u || !hostR || hostR[1] !== h.length || !new RegExp('title="X1 : ' + it.cls + '(｜[^"]*)?"').test(h) || !/left:8px;top:16px/.test(u.html);
  }).map(it => it.cls);
  ok(tbMod.ITEMS.length === 31 && tbBad.length === 0 && tbMod.CATS.every(c => tbMod.ITEMS.some(i => tbMod.catOf(i) === c)) && tbMod.catOf(tbMod.itemOf('TPageControl')) === '容器' && tbMod.catOf(tbMod.itemOf('TMyLed')) === 'IO 元件',
    'toolbox: 31 templates (1008: + LabeledEdit / ListBox / StringGrid / TrackBar / ScrollBar / DateTimePicker / ScrollBox / MyTray / MyLabeledLedLane / LabeledALed; 0.153: + Button / BitBtn / RadioButton / Memo / RadioGroup / PageControl / Image / Shape / Bevel) in 4 categories, each one balanced element with id, title "name : TClass" (an IO one with "｜Alias="), left/top', 'bad: ' + tbBad.join(','));
  const tbLed = tbMod.itemOf('TMyLedLane').html('MyLedLane1', 5, 6), tbBp = tbMod.itemOf('TBtnPanelLane').html('BtnPanelLane1', 5, 6);
  ok(/^<span class="aled LEDHorizontal" id="MyLedLane1"[^>]*width:22px;height:14px;--led-on:#00ff00;--led-off:#c0c0c0;/.test(tbLed) && /title="MyLedLane1 : TMyLedLane｜Alias="/.test(tbLed) &&
    /^<div class="btnpanel flat" id="BtnPanelLane1"[^>]*--bp-true:#05b5dc;--bp-false:#01479d;/.test(tbBp) && />BtnPanelLane1<\/div>$/.test(tbBp),
    'toolbox IO components: the generator markup (a lamp span.aled with --led-on / --led-off, a panel button div.btnpanel flat with --bp-*), Alias left empty');
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
  {
    const lk = s => lnMod.lintPage(s).map(i => i.kind + (i.fix ? '>' + i.fix.repl : '')).join(' | ');
    const g = [lk('<div class="a"id="b"></div>'), lk('<span style="font-family:"MS Sans Serif" ;color:red">x</span>'),
      lk('<ul><li id=x>a<li id=y>b</ul><select><option id=o1>A<option id=o2>B</select>'), lk('<div title="see id=\'q\'" id="z"></div><span id="q"></span>')];
    ok(g[0] === '' && g[1] === "quote-cut>font-family:'MS Sans Serif' ;color:red" && g[2] === '' && g[3] === '',
      '1009 review (lint #5-#8): class="a"id="b" is two attributes; a cut value is fixed up to its real end; <li> / <option> without their end tag are not unclosed; an "id=" inside a title is no id',
      g.join(' / '));
  }

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
  // (1006 audit: "a, b" -> c is written plain -- this file quotes no value that does not need it; a file that quotes
  //  plain values keeps doing so)
  const cvQ = 'A,B\r\n"x","y"\r\n', cvQP = cv.parseDoc(cvQ);
  ok(cvQP.quotesPlain && applyReps(cvQ, cv.cellEdits(cvQP, [{ r: 1, c: 0, v: 'z' }])) === 'A,B\r\n"z","y"\r\n' && !cvP.quotesPlain &&
    cv.detectDelim('a;b\r\n"x\ty",1,2\r\n3,4,5\r\n6,7,8\r\n') === ',' && cv.detectDelim('N\tA\r\n"a,b"\t2\r\n') === '\t' && cv.detectDelim('x\r\n1,2\r\n3,4\r\n') === ',',
    'csv quoting follows the file (quotes plain values or not); the separator counted outside quotes over all lines (not only line 1)');
  ok(cvE1 === cvText.replace('MInArmX', 'MInArmY') && cvE2 === cvText.replace('"a, b"', 'c').replace('M01,,', 'M01,"x,y",') &&
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
  ok(cvTsv === 'a\t"b\tc"\r\n1\t\r\n' && JSON.stringify(cvFrom) === '[["x","y"],["1","2\\t3"]]' && cv.detectDelim('a;b;c\n1;2;3') === ';' &&
    cv.decodeSafe(cvBig5, 'A,\uFFFD\uFFFD\r\n').ok === false && cv.decodeSafe(Buffer.from('A,B'), 'A,B').ok && cv.decodeSafe(cvBig5, 'A,中\r\n').ok,
    'csv clipboard: tab separated both ways (Excel\'s); ; detected; a Big5 file read as UTF-8 (U+FFFD) is not editable, read right it is');
  ok(cv.decodeSafe(cvBig5, 'A,中\r\n').big5 === true && !cv.decodeSafe(Buffer.from('A,中'), 'A,中').big5 &&
    JSON.stringify(cv.big5Missing('馬達 吸嘴 简 😀 ① ア Ж 碁 €')) === '["简","😀","①","ア","Ж"]' && cv.big5Missing('X_AXIS 1,2').length === 0,
    '1009: a file read as Big5 is marked big5; the characters Big5 has none for (VS Code saves "?") found -- simplified, emoji; 1009 review (csv #5): ① ア Ж are not cp950\'s (Windows writes "?"; measured), 碁 € are');
  // AI(W906-HTDESIGNER) 20261001: a paste as Excel does it -- one cell on a column fills it, a block that fits a whole
  // number of times repeats, one that does not = the block once; a cut block moves (its other cells emptied)
  const pcP = cv.parseDoc('N,A,B\r\nm0,1,2\r\nm1,3,4\r\nm2,5\r\n');
  const pcOne = cv.pasteChanges(pcP, [['9']], { r0: 1, c0: 1, r1: 3, c1: 1 });
  const pcRep = cv.pasteChanges(pcP, [['a', 'b']], { r0: 1, c0: 1, r1: 2, c1: 2 });
  const pcOdd = cv.pasteChanges(pcP, [['a', 'b']], { r0: 1, c0: 0, r1: 1, c1: 2 });   // 1x2 into 1x3: not a whole number of times
  const pcCut = cv.pasteChanges(pcP, [['m0', '1']], { r0: 3, c0: 0, r1: 3, c1: 0 }, { r0: 1, c0: 0, r1: 1, c1: 1 });
  const pcText = cv.cellEdits(pcP, pcCut.changes).reduceRight((t, e) => t.slice(0, e.s) + e.text + t.slice(e.e), 'N,A,B\r\nm0,1,2\r\nm1,3,4\r\nm2,5\r\n');
  ok(JSON.stringify(pcOne.changes) === '[{"r":1,"c":1,"v":"9"},{"r":2,"c":1,"v":"9"},{"r":3,"c":1,"v":"9"}]' && pcOne.tiled && pcOne.rect.r1 === 3 &&
    pcRep.tiled && pcRep.changes.length === 4 && pcRep.changes.every(x => x.v === (x.c === 1 ? 'a' : 'b')) &&
    !pcOdd.tiled && pcOdd.changes.length === 2 && pcOdd.rect.c1 === 1 &&
    pcText === 'N,A,B\r\n,,2\r\nm1,3,4\r\nm0,1\r\n' && pcCut.rect.r0 === 3 && pcCut.rect.c1 === 1,
    'csv paste (Excel): one cell on a column = every cell; a block that fits repeats, one that does not = once; a cut block moves (old cells emptied), one edit',
    JSON.stringify([pcOne.changes.length, pcRep.changes.length, pcOdd.changes.length, pcText]));
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
  // (1008, feature gap #10): a component renamed on the page -- the form class's member of that name and its uses follow,
  // whole words only; comments, strings, another object's member, another class's scope and longer names stay
  {
    const mrH = ['class TfDemo : public TForm', '{', '__published:', '    TSpeedButton *spbSave;   // spbSave: the save button', '    TSpeedButton *spbSaveAll;',
      '    void __fastcall spbSaveClick(TObject *Sender);', '    void Paint() { spbSave->Caption = "spbSave"; }', '};', 'class TfOther { TSpeedButton *spbSave; };',
      'extern PACKAGE TfDemo *fDemo;', ''].join('\r\n');
    const mrC = ['#include "fDemo.h"', '// spbSave is set up in FormShow', 'void TfDemo::FormShow(TObject *Sender)', '{', '    spbSave->Caption = "Save";',
      '    this->spbSave->Enabled = true;', '    fDemo->spbSave->Visible = true;', '    fOther->spbSave->Visible = false;', '    TfDemo::spbSave = 0; TfOther::spbSave = 0;',
      '    ShowMessage("spbSave"); spbSaveAll->Down = 1; x.spbSave = 2;', '    /* spbSave */ spbSave->Tag = 3;', '}', 'static int spbSave = 0;',
      'void Helper() { int spbSave = 1; fDemo->spbSave->Tag = spbSave; }', 'void TfOther::Go() { spbSave->Tag = 4; }', 'TfDemo::TfDemo(TComponent *O) : TForm(O)', '{ spbSave = 0; }', ''].join('\r\n');
    const mr = cstub.renameMemberEdits(mrH, mrC, 'TfDemo', 'spbSave', 'btnSave');
    const mrApply = (t, eds) => { let o = t; for (const x of eds.slice().sort((a, b) => b.s - a.s)) o = o.slice(0, x.s) + x.text + o.slice(x.e); return o; };
    const mrH2 = mrApply(mrH, mr.h), mrC2 = mrApply(mrC, mr.cpp);
    const mrWantH = mrH.replace('TSpeedButton *spbSave;   //', 'TSpeedButton *btnSave;   //').replace('{ spbSave->Caption', '{ btnSave->Caption');
    const mrWantC = mrC.replace('    spbSave->Caption = "Save"', '    btnSave->Caption = "Save"').replace('this->spbSave', 'this->btnSave')
      .replace('fDemo->spbSave', 'fDemo->btnSave').replace('TfDemo::spbSave = 0', 'TfDemo::btnSave = 0').replace('*/ spbSave->Tag', '*/ btnSave->Tag')
      .replace('fDemo->spbSave->Tag = spbSave', 'fDemo->btnSave->Tag = spbSave').replace('{ spbSave = 0; }', '{ btnSave = 0; }');
    const mrClash = cstub.renameMemberEdits(mrH, mrC, 'TfDemo', 'spbSave', 'spbSaveAll');
    const mrNone = cstub.renameMemberEdits(mrH, mrC, 'TfNope', 'spbSave', 'btnSave');
    ok(mr.h.length === 2 && mr.cpp.length === 7 && mrH2 === mrWantH && mrC2 === mrWantC && !mr.clash && mr.selfs.join(',') === 'this,fDemo' &&
      mrClash.clash && !mrNone.h.length && !mrNone.cpp.length && /\r\n/.test(mrC2) && !/\r\r|[^\r]\n/.test(mrC2),
      'cppstub.renameMemberEdits: spbSave -> btnSave in TfDemo\'s body (member + inline use) and the .cpp (bare in TfDemo:: bodies + ctor, this->, fDemo-> anywhere, TfDemo::); file-scope / free-function / TfOther:: bare names (audit G1), comments, strings, spbSaveAll, fOther->, x., TfOther:: and the other class unchanged; a name the class has = clash; no class = nothing; CRLF kept',
      JSON.stringify({ h: mr.h.length, cpp: mr.cpp.length, selfs: mr.selfs, clash: mrClash.clash, hOk: mrH2 === mrWantH, cOk: mrC2 === mrWantC }));
  }
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
  {
    // (1010 keys walkthrough #1 #4 #5: removing ours never breaks the file (a comment ending in a comma), a user's own copy
    //  of a line of ours is kept, ours in another order is no change)
    const bk = require('../lib/bcbkeys');
    const full = bk.ensure('');
    const L = full.split('\n'); const ix = L.findIndex(l => l.includes('-cmake.launchTarget')); L[ix - 1] = L[ix - 1].replace(/^(\s*)/, '$1// ');
    const r1 = bk.remove(L.join('\n'));
    const t2 = '[\n  { "key":"x","command":"y" }, // note,\n  { "key": "f7", "command": "-cmake.build" }\n]\n';
    const r2 = bk.remove(t2);
    const t3 = '[\n  { "key": "f7", "command": "-cmake.build" }\n]\n';
    const r3 = bk.remove(t3, { keep: new Set(bk.oursPresent(t3)) });
    const objs = bk.scan(full).objs.map(o => o.text); [objs[0], objs[1]] = [objs[1], objs[0]];
    const r5 = bk.ensure('[\n  ' + objs.join(',\n  ') + '\n]\n');
    ok(!!bk.scan(r1) && !!bk.scan(r2) && /"x"/.test(r2) && !/cmake\.build/.test(r2) && r3 === t3 && r5 === null,
      'bcbkeys: removing ours never breaks keybindings.json (a // comment ending in a comma), a hand-written copy is kept, another order is no change', JSON.stringify({ r2 }));
  }
  {
    // (1010 operate walkthrough #5: the text inside <textarea> / <title> is no markup)
    const pl0 = require('../lib/pagelint');
    const iss = pl0.lintPage('<html><head><title>a <b> c</title></head><body><textarea id="t1"><div id="z"></textarea><div id="z"></div></body></html>', {});
    ok(!iss.some(x => /unclosed|dup/i.test(String(x.code || x.kind || x.type || x.rule || ''))),
      'pagelint: the text inside <textarea> / <title> is not taken as tags (no unclosed / duplicate id)', JSON.stringify(iss.map(x => x.code || x.kind || x.type || x.rule)));
    // (1010 review of 0.395 B: a <title> with no end tag swallows the page -- said, not "no problems")
    const iss2 = pl0.lintPage('<html><head><title>abc</head><body><div id="a"></div><div id="a"></div></body></html>', {});
    ok(iss2.some(x => x.kind === 'unclosed' && /title/.test(x.msg || '')), 'pagelint: a <title> with no end tag is reported (the rest of the page is its text)', JSON.stringify(iss2.map(x => x.kind)));
  }
  {
    // (1009 CSV walkthrough #3 #4: a sort keeps every line end where it was; a machine table read in a single-byte encoding is read only)
    const ct = require('../lib/csvtable');
    const app = (t, r) => (r.length ? t.slice(0, r[0].s) + r[0].text + t.slice(r[0].e) : t);
    const s1 = 'h\r\nb\na\r\n', s2 = 'h\r\nb\r\nc\na';
    const o1 = app(s1, ct.sortRows(ct.parseDoc ? ct.parseDoc(s1) : ct.parse(s1), s1, 1, 0, false));
    const o2 = app(s2, ct.sortRows(ct.parseDoc ? ct.parseDoc(s2) : ct.parse(s2), s2, 1, 0, true));
    const b = Buffer.from('a,b\r\n');
    const m1 = ct.decodeSafe(b, 'a,b\r\n', 'windows1252', 'E:/x/Mot_Table.csv', true), m2 = ct.decodeSafe(b, 'a,b\r\n', 'windows1252', 'E:/x/notes.csv', false), m3 = ct.decodeSafe(b, 'a,b\r\n', 'utf8', 'E:/x/Mot_Table.csv', true);
    const m4 = ct.decodeSafe(Buffer.concat([Buffer.from([0xEF, 0xBB, 0xBF]), b]), 'a,b\r\n', 'utf8bom', 'E:/x/Mot_Table.csv', true);   // (a BOM is not text)
    ok(o1 === 'h\r\na\nb\r\n' && o2 === 'h\r\nc\r\nb\na' && m1.ok === false && m2.ok === true && m3.ok === true && m3.asciiOnly === true && m4.ok === true && m4.asciiOnly === true,
      'csvtable: sorting keeps each line end in its place (mixed LF / CRLF); a machine table (by the caller\'s answer) read in windows1252 is read only, in UTF-8 ASCII-only', JSON.stringify({ o1, o2, m1: m1.ok, m3 }));
  }
  {
    // (1009 event-wiring walkthrough #4: rows remember the table they came from, not enumerable -- kept by filter, not copied)
    const t1 = '// htd-row TfA aClick btnA OnClick\n// htd-row TfB bClick btnB OnClick\n';
    const r1 = cbr.rowsOf(t1);
    const kept = r1.filter(r => r.form === 'TfB');
    const copied = r1.map(r => Object.assign({}, r));
    ok(r1.length === 2 && cbr.srcOf(r1) === t1 && cbr.srcOf(kept) === t1 && cbr.srcOf(copied) === undefined && cbr.srcOf([]) === undefined &&
      JSON.stringify(r1[0]) === '{"form":"TfA","handler":"aClick","control":"btnA","event":"OnClick"}' && Object.keys(r1[0]).length === 4,
      'cppbridge.rowsOf / srcOf: each row remembers its table text (not enumerable, not in JSON); filtered rows keep it, copies do not');
  }
  const cbServer =fs.readFileSync(path.join(PORT, 'tools', 'wb_serve.cpp'), 'utf8');
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
  // (1003: form.event's branch got `{ extern void W906_FormEventRunAfterAck(bool ok); W906_FormEventRunAfterAck(feOk); }`
  //  -- form.event's own queue, not copied into htd.event; another unknown FormEvent call = refused, not guessed)
  const cbFx = 'a\n} else if (wc.cmd == "form.event") { extern bool W906_FormEvent(const std::string& tag); bool feOk = W906_FormEvent(wc.tag); server.CompleteCommand(1, feOk, ""); { extern void W906_FormEventRunAfterAck(bool ok); W906_FormEventRunAfterAck(feOk); } } else if (wc.cmd == "x") { }\n';
  const cbFa = cbr.serverHook(cbFx, 's'), cbFb = cbr.serverHook(cbFx.replace('{ extern void W906_FormEventRunAfterAck(bool ok); W906_FormEventRunAfterAck(feOk); }', 'W906_FormEventMore();'), 's');
  ok(cbFa.edit && /W906_HtdEvent\(wc\.tag\); server\.CompleteCommand\(1, feOk, ""\); $/.test(cbFa.edit.text) && !/FormEvent/.test(cbFa.edit.text) && cbFb.error && /W906_FormEventMore/.test(cbFb.error),
    'cppbridge.serverHook: form.event\'s after-ack queue call is left out of the htd.event copy; an unknown form.event-only call = refused (not guessed)',
    JSON.stringify({ a: cbFa.edit && cbFa.edit.text.slice(-120), b: cbFb.error }));
  const cbCm = fs.readFileSync(path.join(PORT, 'CMakeLists.txt'), 'utf8');
  const cbC = cbr.cmakeHook(cbCm, 'tools/wb_serve.cpp', cbr.GEN_REL);
  const cbCm2 = cbC.edit ? jeApply(cbCm, [cbC.edit]) : cbCm;
  const cbWbs = /add_executable\(wb_serve[\s\S]*?\)\s*#/.exec(cbCm2.replace(/#[^\n]*/g, '#'));
  // (1006: the generated code is several files -- the table listed, the parts by a glob in front of the add_executable)
  const cbAt = cbC.edit ? cbCm2.indexOf('HtdEvents/HtdEvents.gen.cpp  ${HTD_GEN_PARTS})') : -1;
  const cbGl = cbCm2.indexOf('file(GLOB HTD_GEN_PARTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/HtdEvents/part/*.gen.cpp")');
  const cbOld = cbC.edit ? cbCm.slice(0, cbC.edit.e) + '  ' + cbr.GEN_REL + cbCm.slice(cbC.edit.e) : '';
  const cbUp = cbOld ? cbr.cmakeHook(cbOld, 'tools/wb_serve.cpp', cbr.GEN_REL) : null;
  ok(!!(cbC.has || (cbC.edit && nlOf(cbCm2) === nlOf(cbCm) + 1 && cbAt > 0 && cbCm2.slice(cbAt - 30, cbAt).includes('MainStateRecord.cpp') && cbGl > 0 && cbGl < cbAt &&
    /add_executable\(wb_serve/.test(cbCm2.slice(cbGl, cbAt)) &&
    cbr.cmakeHook(cbCm2, 'tools/wb_serve.cpp', cbr.GEN_REL).has && !cbr.cmakeHook(cbCm, 'tools/nothere.cpp', cbr.GEN_REL).edit &&
    cbUp && cbUp.edit && /HTD_GEN_PARTS/.test(cbUp.edit.text) && (cbUp.edit.text.match(/HtdEvents\.gen\.cpp/g) || []).length === 1)),
    'cppbridge.cmakeHook: the table goes in wb_serve\'s add_executable in front of its ")", the parts by a glob on the line before (one line more); a second time = already there; a tree with the table only gets the glob (the table not twice); no such executable = nothing',
    cbC.error || (cbC.has ? 'already listed' : cbCm2.slice(Math.max(0, cbGl), cbGl + 160) + ' … ' + cbCm2.slice(cbAt - 40, cbAt + 60)));
  const cbCall = cbr.callOf('TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y', 'fHotPlate', 'spbSaveMouseUp', 'nullptr');
  const cbKey = cbr.callOf('TObject *Sender, WORD &Key, TShiftState Shift', 'fHotPlate', 'XST1KeyDown', 'HtdSender(fHotPlate->XST1)');
  const cbBad = cbr.callOf('TObject *Sender, TPoint &MousePos, bool &Handled', 'fX', 'a', 'nullptr');
  // (1006: OnEndDock has VCL's own signature now -- the same as OnEndDrag, and the web can call it)
  const cbDock = cbr.callOf(require('../lib/vclevents').signatureOf('OnEndDock'), 'fX', 'pnlEndDock', 'nullptr');
  ok(cbDock.call === 'fX->pnlEndDock(nullptr, nullptr, a.x, a.y);', 'OnEndDock: VCL\'s signature (Sender, Target, X, Y), called from the web with the position', cbDock.call || cbDock.error);
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
  // AI(W906-HTDESIGNER) 20261001 (an unsaved generated file broke the build): a row calls its handler only when the
  // class has it -- C<i>(..., int) by a decltype of the call (a WORD& local = std::declval), else C<i>(..., long) = 0
  // ("no-handler"), so a form .h not saved yet still compiles; the server's note is ASCII; spliceBytes keeps every
  // other byte (a file that is not all valid UTF-8 stays as it was)
  const sfGen = cbr.genFile([{ form: 'TfHotPlate', handler: 'XST1KeyDown', control: 'XST1', event: 'OnKeyDown', header: 'forms/fHotPlate.h', obj: 'fHotPlate', call: cbKey }],
    { fn: 'W906_HtdEvent', cjson: 'Public/cJSON.h', running: ['SystemStart'], shiftHeader: 'vclcompat/ShiftState.h' }, '\n');
  const sfB = Buffer.from([0x61, 0xe4, 0xb8, 0xad, 0x0a, 0xff, 0x62]);   // a, UTF-8 中, LF, a byte that is not UTF-8, b
  const sfB2 = cbr.spliceBytes(sfB, { s: 4, e: 4, text: 'XY' });
  const sfNote = cbS.edit ? /\/\*AI\(W906-HTDESIGNER\) 20260930: [\x20-\x7e]*\*\//.test(cbHead) : true;
  ok(/template <class T> auto C0\(T\* o, const HtdIn& a, int\) -> decltype\(\(void\)o->XST1KeyDown\(HtdSender\(o->XST1\), std::declval<WORD&>\(\), HtdShift\(a\)\), 1\)/.test(sfGen) &&
    /\{ \(void\)a; if \(!o\) return -1; WORD k0 = \(WORD\)a\.key; o->XST1KeyDown\(HtdSender\(o->XST1\), k0, HtdShift\(a\)\); return 1; \}/.test(sfGen) &&
    /template <class T> int C0\(T\*, const HtdIn&, long\) \{ return 0; \}/.test(sfGen) && /int E0\(const HtdIn& a\) \{ return C0\(fHotPlate, a, 0\); \}/.test(sfGen) &&
    /int \(\*run\)\(const HtdIn&\)/.test(sfGen) && /ran = r->run\(a\);/.test(sfGen) && /"no-handler: "/.test(sfGen) && /#include <utility>/.test(sfGen) &&
    !!sfB2 && sfB2.toString('hex') === '61e4b8ad58590aff62' && cbr.spliceBytes(sfB, { s: 0, e: 0, text: '中' }) === null && sfNote,
    'cppbridge (20261001): a generated row calls its handler only when the class has it (else 0 = no-handler, still compiles); the server note ASCII; spliceBytes = every other byte kept',
    sfGen.split('\n').filter(l => /^template|^int E/.test(l)).join(' | ').slice(0, 260));
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

  // --- cpptypes (AI 20261001): a new handler's parameter types must be seen by the form's header, else 9050 does not
  // build (e2e_build.ps1 measured: OnContextPopup / OnDragOver / OnStartDrag / OnDrawItem / OnMeasureItem broke it)
  {
    const ct = require('../lib/cpptypes');
    const vev = require('../lib/vclevents');
    const hpH = path.join(PORT, 'forms', 'fHotPlate.h');
    const clo = fs.existsSync(hpH) ? ct.includeClosure(hpH, PORT) : [];
    const miss = ev => ct.missingTypes(vev.signatureOf(ev), clo).join(',');
    const got = {
      types: ct.typesOf('TObject *Sender, TPoint &MousePos, bool &Handled').join(','),
      typesConst: ct.typesOf('TWinControl *Control, int Index, const TRect &Rect, TOwnerDrawState State').join(','),
      decl: [ct.declares('class TPoint { };', 'TPoint'), ct.declares('typedef int TKey;', 'TKey'), ct.declares('enum class TDragState { a };', 'TDragState'),
        ct.declares('using TRect = int;', 'TRect'), ct.declares(require('../lib/cppstub').mask('// class TWinControl\n'), 'TWinControl')].join(','),
      click: miss('OnClick'), mouseDown: miss('OnMouseDown'), keyDown: miss('OnKeyDown'), keyPress: miss('OnKeyPress'),
      popup: miss('OnContextPopup'), dragOver: miss('OnDragOver'), startDrag: miss('OnStartDrag'), drawItem: miss('OnDrawItem'), measure: miss('OnMeasureItem'),
      files: clo.length,
    };
    ok(got.types === 'TObject,TPoint' && got.typesConst === 'TWinControl,TRect,TOwnerDrawState' && got.decl === 'true,true,true,true,false' &&
      clo.length >= 2 && !got.click && !got.mouseDown && !got.keyDown && !got.keyPress &&
      got.popup === 'TPoint' && got.dragOver === 'TDragState' && got.startDrag === 'TDragObject' && got.drawItem === 'TWinControl,TRect,TOwnerDrawState' && got.measure === 'TWinControl',
      'cpptypes: the parameter types a new handler needs, against what fHotPlate.h really includes -- OnClick / OnMouseDown / OnKeyDown / OnKeyPress fine; OnContextPopup (TPoint), OnDragOver (TDragState), OnStartDrag (TDragObject), OnDrawItem / OnMeasureItem (TWinControl ...) not in the port: refused before anything is written',
      JSON.stringify(got));
  }

  // --- projectsearch, 1006 audit: lone CR lines, the column in the whole line, the per-file cap, ^ $ per line, $1, BCB6
  {
    const ps = require('../lib/projectsearch');
    const cr = ps.hitsIn('a\rb KEY\r\nc KEY\nKEY', ps.makeRe('KEY').re, 10);
    const cap = ps.hitsIn('K K K', ps.makeRe('K').re, 2), noCap = ps.hitsIn('K K', ps.makeRe('K').re, 2);
    const ml = ps.hitsIn('ab\nab\r\nab', ps.makeRe('^ab$', { regex: true }).re, 10);
    const mm = /(\w+)-(?<n>\d+)/.exec('M-01');
    const ex = [ps.expand('$2_$1', mm), ps.expand('$<n>/$&/$$/$9', mm), ps.expand('x$10', mm)];
    const lc = ps.hitsIn('y'.repeat(500) + 'KEY', ps.makeRe('KEY').re, 5)[0];
    const got = { cr: cr.map(h => h.line + ':' + h.col).join(','), cap: [cap.length, !!cap.more, !!noCap.more], ml: ml.length, ex, lc: lc.lc,
      bcb: [ps.isBcb6('D:\\HT9045\\HT9011UC_Code_V3.33.899.0_x\\a.cpp'), ps.isBcb6('D:/a/HT9011UC_Cpp_V3.33.906.0/a.cpp')] };
    ok(got.cr === '2:3,3:3,4:1' && got.cap.join() === '2,true,false' && got.ml === 3 && ex.join('|') === '01_M|01/M-01/$/$9|xM0' && got.lc === 500 && got.bcb.join() === 'true,false',
      'projectsearch: a lone CR ends a line, lc = the column in the whole line, more than perFile marked, regex ^ $ per line, $1 / $<n> / $& / $$ filled, a BCB6 tree path known', got);
  }
  // --- projectsearch.classify (1007, audit #14): a template's "<" is no comparison
  {
    const ps = require('../lib/projectsearch'), cpp = require('../lib/cppstub');
    const c = (t, w) => { const k = ps.classify(cpp.mask(t), t, t.indexOf(w), w.length); return (k.cond ? 'c' : '') + (k.func ? 'f' : '') || '-'; };
    const got = [c('std::vector<int> v;', 'vector'), c('TList<TObject*> *p = 0;', 'TList'), c('map<string, int> m;', 'int'), c('f(vector<int>(3));', 'vector'),
      c('if (a < b) x=1;', 'a'), c('if (a < b && c > d) x;', 'b'), c('y = a<b;', 'a')].join(' ');
    ok(got === '- - - f c c c', 'projectsearch.classify: vector<int> / TList<TObject*> / map<a, b> are no conditions (vector<int>(3) a call); a < b, a < b && c > d still are', got);
  }
  // --- events audit (1007): a declaration in a CRLF header and back byte for byte, in a public / __published section; a
  //   handler the web cannot send kept as a commented htdCpp line (found again, renamed, removed); the last line out = the
  //   block out (the page back byte for byte); reserved words / read-only globals refused as JS function names
  {
    const cs = require('../lib/cppstub'), je = require('../lib/jsevents');
    const ap = (t, e) => { for (const x of e.slice().sort((p, q) => q.s - p.s)) t = t.slice(0, x.s) + x.text + t.slice(x.e); return t; };
    const h = 'class TfA : public TForm\r\n{\r\n__published:\r\n    void AClick(TObject *Sender);   // GATE (I-6)\r\nprivate:\r\n    void BClick(TObject *Sender);\r\npublic:\r\n    TfA();\r\n};\r\n';
    const de = cs.declEdit(h, 'TfA', 'CClick', 'TObject *Sender');
    const h2 = h.slice(0, de.at) + de.text + h.slice(de.at);
    const rm = cs.removeEdits(h2, '', 'TfA', 'CClick');
    const back = rm.h ? h2.slice(0, rm.h.s) + h2.slice(rm.h.e) : null;
    const hp = 'class TfB : public TForm\r\n{\r\nprivate:\r\n    void BClick(TObject *Sender);\r\npublic:\r\n    TfB();\r\n};\r\n';
    const dp = cs.declEdit(hp, 'TfB', 'CClick', 'TObject *Sender');
    const pubOk = /public:\r\n    void CClick\(TObject \*Sender\);\r\n    TfB\(\);/.test(hp.slice(0, dp.at) + dp.text + hp.slice(dp.at));
    const pg = '<html><body>\r\n<div id="P1"></div>\r\n</body></html>\r\n';
    const p1 = ap(pg, je.setCpp(pg, 'P1', 'none', 'TfA', 'P1CanResize', 'OnCanResize').edits);
    const ln = je.cppLines(p1)[0];
    const p2 = ap(p1, je.renameCppEdits(p1, 'TfA', 'P1CanResize', 'P1Rn'));
    const p3 = ap(p2, je.setCpp(p2, 'P1', 'none', 'TfA', '', 'OnCanResize').edits);
    const w1 = ap(pg, je.setCpp(pg, 'P1', 'click', 'TfA', 'P1Click', 'OnClick').edits), w0 = ap(w1, je.setCpp(w1, 'P1', 'click', 'TfA', '', 'OnClick').edits);
    const j1 = ap(pg, je.setBinding(pg, 'P1', 'click', 'P1_click').edits), j0 = ap(j1, je.setBinding(j1, 'P1', 'click', '').edits);
    const resv = ['delete', 'class', 'document', 'top', 'undefined'].every(n => !!je.setBinding(pg, 'P1', 'click', n).error) && !je.setBinding(pg, 'P1', 'click', 'okFn').error;
    const got = { crlf: !/\r\r\n/.test(h2) && back === h, pubOk, only: !!(ln && ln.only && ln.handler === 'P1CanResize' && /^\/\/ htdCpp/.test(p1.slice(ln.s, ln.e))),
      renamed: je.cppLines(p2)[0].handler === 'P1Rn', gone: p3 === pg && w0 === pg && j0 === pg, resv };
    ok(Object.values(got).every(Boolean), 'events audit: a declaration in a CRLF header and back byte for byte (no \\r\\r\\n), in a public section when the last handler is private; an event the web cannot send kept as // htdCpp (found, renamed, removed); the last line out takes the block (page back byte for byte); reserved / global names refused', got);
  }
  // --- 1007 audit (export / lint / replace): redirect pages, no network at the browser level, a cut size said; an
  //   unterminated tag no longer ends the lint; a bad % escape does not throw; a page's <title> is no caption
  {
    const sn = require('../lib/snapshot'), pl = require('../lib/pagelint'), pc = require('../lib/pagecompare');
    const pages = fs.readdirSync(path.join(WEB, 'page')).filter(x => /\.html$/i.test(x));
    const red = pages.map(x => [x, sn.redirectOf(fs.readFileSync(path.join(WEB, 'page', x), 'utf8'))]).filter(x => x[1]).map(x => x[0]).sort().join();
    const args = sn.edgeArgs({ file: path.join(require('os').tmpdir(), 'a.html'), out: 'o.png', w: 10, h: 10, profile: 'p' }).concat(sn.measureArgs({ file: path.join(require('os').tmpdir(), 'a.html'), profile: 'p' }));
    const noNet = args.filter(x => x === '--proxy-server=127.0.0.1:9').length === 2 && args.filter(x => x === '--proxy-bypass-list=<-loopback>').length === 2;
    const cut = sn.parseSize('<meta name="htd-size" content="800x37752">');
    const li = pl.lintPage('<html><body><div id="a" title="a"b=c"></div><div id="x"></div><div id="x"></div><img src="%zz.png"></body></html>', { dir: require('os').tmpdir() }).map(i => i.kind).join();
    const ttl = pc.captionHits([{ file: 'p', text: '<html><head><title id="mvDocumentTitle">Motion</title></head><body><div class="form"><span id="s1" title="s1 : TLabel">Motion</span></div></body></html>' }], 'Motion', 'M2', false).map(h => h.id).join();
    const got = { red, noNet, cut: cut && cut.cut, li, ttl };
    ok(red === 'Setup.BinSelNormal.html,Setup.Configuration.html,Setup.DIOInterFaceCFG.html' && noNet && got.cut === 4000 && li === 'unterminated,dup-id,missing' && ttl === 's1',
      'export / lint / replace audit: the 3 redirect pages found (no other), the network closed at the browser level, a page cut at 4000px said; an unterminated tag reported and the rest still checked, a bad % escape no crash; <title> is not a caption', got);
  }
  // --- 1007 (EastSun: C++ 異常字元 / 編譯錯誤清單, "要照c++版本"): cppcheck by the compiler version, builderrors parse /
  //   compile command / compiler info; every .cpp / .h of the port tree checked (no crash; the counts logged)
  {
    const cc = require('../lib/cppcheck'), be = require('../lib/builderrors');
    const t = 'int a = 1\uFF1B\r\n// \u8A3B\u89E3 OK\r\nconst char* s = "\u4E2D\u6587";\r\nint\u00A0b;\r\nif (x == \u201Cy\u201D) {}\r\nint \u8B8A = 2;\r\n';
    const k6 = cc.checkText(t, { gccMajor: 6 }).map(p => p.line + ':' + p.kind).join(), k16 = cc.checkText(t, { gccMajor: 16 }).map(p => p.line + ':' + p.kind).join();
    const buf = Buffer.concat([Buffer.from('ok\r\nab'), Buffer.from([0xa7, 0x6c]), Buffer.from('cd\r\n')]);
    const bad = cc.check(buf.toString('utf8'), buf).map(p => p.line + ':' + p.col + ':' + p.kind).join();
    const ps = be.parse('D:/x/a.cpp:12:5: error: expected \';\'\nIn file included from b.h:3:1,\n../c.h:7:2: warning: unused\nD:/x/a.cpp:(.text+0x12): undefined reference to `foo()\'', 'D:/b');
    const pOk = ps.length === 3 && ps[0].line === 12 && ps[0].col === 5 && ps[0].severity === 'error' && /c\.h$/.test(ps[1].file) && ps[1].severity === 'warning' && /undefined reference/.test(ps[2].msg);
    let files = 0, found = 0, ms = Date.now();
    const walk = d => { for (const e of fs.readdirSync(d, { withFileTypes: true })) { const p = path.join(d, e.name); if (e.isDirectory()) { if (!/^(build|\.git|node_modules|third_party|Obj)/i.test(e.name)) walk(p); } else if (/\.(cpp|h|hpp|c)$/i.test(e.name)) { files++; const b = fs.readFileSync(p); found += cc.check(b.toString('utf8'), b, { gccMajor: 16 }).length; } } };
    walk(PORT);
    ms = Date.now() - ms;
    log('C++ 異常字元：' + files + ' 個檔，' + found + ' 處（' + ms + ' ms）');
    ok(k6 === '0:lookalike,3:lookalike,4:lookalike,4:lookalike,5:nonascii' && k16 === '0:lookalike,3:lookalike,4:lookalike,4:lookalike' && bad === '1:2:badutf8' && pOk && files > 500,
      'C++ checks (1007): full-width ; / NBSP / curly quotes found outside comments and strings (Chinese in them is fine), a Chinese name an error for g++ 6 and fine for g++ 16; Big5 bytes placed; g++ / ld output parsed; every port file checked', { k6, k16, bad, pOk, files, found, ms });
  }
  // --- 1008 (C++ checks audit): link errors of a Release build (object / archive member + source, no line), "cannot find
  //   -lX", the compiler's own fatal error, GCC's colour codes; every target of the build indexed (tests\ too)
  {
    const be = require('../lib/builderrors');
    const out = ["C:/tc/ld.exe: c1n.o:c1.cpp:(.text+0x14): undefined reference to `ext()'",
      "C:/tc/ld.exe: libht9045_sm.a(csystem.cpp.obj):csystem.cpp:(.text+0x1): undefined reference to `x'",
      'C:/tc/ld.exe: cannot find -lfoo', 'collect2.exe: error: ld returned 1 exit status', 'cc1plus.exe: fatal error: zz.cpp: No such file or directory',
      '\x1b[01m\x1b[KD:/x/a.cpp:5:3:\x1b[m\x1b[K \x1b[01;31m\x1b[Kerror: \x1b[m\x1b[Kbad', 'mingw32-make[2]: *** [x] Error 1'].join('\n');
    const ps = be.parse(out, 'D:/b');
    const got = ps.map(p => (p.src || (p.file ? path.basename(p.file) : '-')) + ':' + p.line + ':' + p.severity).join(',');
    let idx = 0, tst = null;
    for (const d of be.buildDirs(PORT)) { const ix = be.sourceIndex(d); if (ix.size > idx) { idx = ix.size; const c = be.compileCommand(d, path.join(PORT, 'language.cpp')); tst = c ? path.basename(c.cwd) : null; } }
    ok(got === 'c1.cpp:0:error,csystem.cpp:0:error,-:0:error,-:0:info,-:0:error,a.cpp:5:error' && (idx === 0 || (idx > 700 && tst === 'tests')),
      'builderrors (1008): Release link errors (object / archive member -> its source), cannot find -lX, the compiler\'s fatal error, colour codes; make\'s own line not taken; tests\ targets indexed (language.cpp from test_ga1_language, cwd tests)', { got, idx, tst });
    const be2 = require('../lib/builderrors').parse([
      'C:/W/bin/ld.exe: cannot open output file wb_serve.exe: Permission denied', 'ld.exe: final link failed: file truncated',
      'cc1plus.exe: out of memory allocating 65536 bytes', 'D:/T/x.cpp:5:1: internal compiler error: Segmentation fault',
      'D:/T/y.cpp:6:1: sorry, unimplemented: thing', "ninja: error: 'D:/T/gone.cpp', needed by 'x.obj', missing and no known rule to make it",
      "mingw32-make[2]: *** No rule to make target 'gone.cpp', needed by 'x.obj'.  Stop.", 'windres.exe: D:/T/wb_serve.rc:12: syntax error',
      "mingw32-make[1]: *** [CMakeFiles/x.dir/all] Error 2"].join('\n'), 'D:/B');
    ok(be2.filter(p => p.severity === 'error').length === 8 && be2.length === 8,
      '1009 review (build #1): the failures without a source line are errors too -- exe locked, final link, out of memory, ICE, sorry, ninja / make missing file, windres; make\'s "*** Error 2" summary not taken',
      be2.map(p => p.severity + ' ' + p.msg.slice(0, 40)));
    const be3 = require('../lib/builderrors').parse("D:/W/bin/ld.exe: libht9045_sm.a(fRotate.cpp.obj):fRotate.cpp:(.text+0x12): undefined reference to `Foo()'", 'D:/B')[0];
    ok(be3 && be3.archive === 'ht9045_sm' && be3.src === 'fRotate.cpp' && be3.dir === 'D:/B',
      '1009 review (build #3): a Release link error keeps its archive (ht9045_sm) -- two built fRotate.cpp are told apart by their target', JSON.stringify(be3));
  }
  // --- 1008 (feature gap #5): CMakeLists.txt -- the targets, the ones that build a folder, a source line added in the right
  //   place (the file read only), where a source is named
  {
    const cm = require('../lib/cmakelists');
    const t = fs.readFileSync(path.join(PORT, 'CMakeLists.txt'), 'utf8');
    const ts = cm.targets(t);
    const pub = cm.targetsFor(t, 'Public');
    const ins = pub.length ? cm.insertSource(t, pub[0], 'Public/HtdNew.cpp', 'x') : null;
    const t2 = ins ? t.slice(0, ins.at) + ins.text + t.slice(ins.at) : t;
    const after = cm.targetsFor(t2, 'Public');
    const got = { ts: ts.length, pub: pub.map(x => x.name + ':' + x.count).join(), afterN: after.length ? after[0].count : 0, same: cm.targets(t2).length === ts.length,
      noBlank: ins ? !/\r\r\n/.test(t2.slice(ins.at - 4, ins.at + ins.text.length + 4)) : false, refs: cm.refsOf(t, 'Public/cBootLog.cpp').length, partial: cm.refsOf(t, 'cBootLog.cpp').length };
    ok(got.ts > 10 && pub.length >= 1 && got.afterN === pub[0].count + 1 && got.same && got.noBlank && got.refs >= 1 && got.partial === 0,
      'cmakelists (1008): the add_library / add_executable targets, the ones that build a folder (most first), a new source added after that folder\'s last line (no blank CR), a source found by its whole path', got);
  }
  // --- 1008 (feature gap #16): the lines #if 0 never compiles -- nested, "#endif" inside a comment is not one, CRLF, the
  //   #else of #if 1; csystem.cpp measured (checked against g++ -E once: 0 of its 2,668 dead lines compiled)
  {
    const dc = require('../lib/deadcode');
    const t = ['a;', '#if 0', 'dead1;', '/* #endif */', 'dead2;', '#endif', 'b;', '#if 1', 'live;', '#else', 'dead3;', '#endif', '#ifdef X', 'unk;', '#endif',
      '#if 0', '#if 1', 'd4;', '#endif', 'd5;', '#else', 'live2;', '#endif'].join('\r\n');
    const r = dc.deadRanges(t).map(x => (x.from + 1) + '-' + (x.to + 1)).join(',');
    const cs = fs.readFileSync(path.join(PORT, 'csystem.cpp'), 'utf8');
    const t0 = Date.now(), rc = dc.deadRanges(cs), ms = Date.now() - t0;
    const n = rc.reduce((k, x) => k + x.to - x.from + 1, 0);
    ok(r === '3-5,11-11,17-20' && rc.length > 50 && n > 500 && ms < 2000 && !!dc.deadAt(dc.deadRanges(t), 2) && !dc.deadAt(dc.deadRanges(t), 6),
      'deadcode (1008): #if 0 blocks nested, a #endif inside a comment ignored, CRLF lines, the #else of #if 1 dead, #ifdef undecided; csystem.cpp ' + rc.length + ' blocks / ' + n + ' lines in ' + ms + ' ms', r);
  }
  // --- 1008 (C++ checks audit D8 / D9): no false marks in a raw string, a // comment carried on by \, an #if 0 block; a
  //   look-alike right after a name is its own problem
  {
    const cc = require('../lib/cppcheck');
    const k = t => cc.checkText(t, { gccMajor: 6 }).map(p => p.line + ':' + p.kind).join(',');
    const got = {
      raw: k('const char* r = R"(he said "\u4E2D\u6587")";\nint a;\n'),
      rawD: k('auto s = R"xy(\u591A\u884C\n\u7B2C\u4E8C ") x )xy";\nint b\uFF1B\n'),
      cont: k('// x \\\n\u7E8C\u884C\nint c\uFF1B\n'),
      dead: k('#if 0\nint d\uFF1B\n#endif\nint e\uFF1B\n'),
      name: k('int \u8B8A\u6578\uFF1B\n'),
    };
    ok(got.raw === '' && got.rawD === '2:lookalike' && got.cont === '2:lookalike' && got.dead === '3:lookalike' && got.name === '0:nonascii,0:lookalike',
      'cppcheck (1008): nothing marked inside a raw string (its " and line breaks too), a // comment continued by \\, an #if 0 block; "\u8B8A\u6578\uFF1B" = a name problem and a full-width ; problem', got);
  }
  // --- liveconfig.stripJsonc (1007, audit L4): trailing commas dropped outside strings only
  {
    const lc = require('../lib/liveconfig');
    let got = null;
    try { got = JSON.parse(lc.stripJsonc('{"a":"x, ]", /* c, } */ "b":[1,2, // c\n], "u":"http://h/x",}')); } catch (e) { got = String(e); }
    ok(got && got.a === 'x, ]' && got.b.join() === '1,2' && got.u === 'http://h/x', 'liveconfig.stripJsonc: comments out, trailing commas dropped, a ", ]" inside a string kept', got);
  }
  // --- searchworker (1007, audit #5): the search in a worker thread; a regex stuck in one match is ended on cancel
  {
    const ps = require('../lib/projectsearch'), sw = require('../lib/searchworker');
    const dir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_sw_'));
    fs.writeFileSync(path.join(dir, 'a.cpp'), 'int KEY = 1;\nKEY++;\n');
    fs.writeFileSync(path.join(dir, 'b.js'), 'a'.repeat(40) + '!\n');
    const roots = [{ area: 'port', label: 'p', root: dir, kind: 'port' }];
    const inProc = await ps.search(roots, ps.makeRe('KEY').re, {});
    const inW = await sw.searchInWorker(roots, ps.makeRe('KEY').re, {});
    const t0 = Date.now();
    let cancel = false;
    setTimeout(() => { cancel = true; }, 300);
    const stuck = await sw.searchInWorker(roots, ps.makeRe('(a+)+$', { regex: true }).re, { cancelled: () => cancel });
    const ms = Date.now() - t0;
    try { fs.rmSync(dir, { recursive: true, force: true }); } catch (e) { }
    const got = { same: JSON.stringify(inProc.hits) === JSON.stringify(inW.hits), n: inW.hits.length, stuck: !!stuck.stuck, cancelled: !!stuck.cancelled, ms };
    ok(got.same && got.n === 2 && got.stuck && got.cancelled && ms < 5000,
      'searchworker: the same hits as in-process; a regex stuck in one match (a+)+$ ended on cancel within seconds, the host not blocked', got);
  }
  // --- 1009 review of the find window: an open unsaved text over the disk's, whole word at punctuation, .py / .vscode,
  //     golden UTF-8 first, a lone CR in #if 0 lines
  {
    const ps = require('../lib/projectsearch'), sw = require('../lib/searchworker'), dc = require('../lib/deadcode');
    const dir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_ov_'));
    const fa = path.join(dir, 'a.cpp');
    fs.writeFileSync(fa, 'int Foo = 1;\n');
    fs.mkdirSync(path.join(dir, '.vscode')); fs.writeFileSync(path.join(dir, '.vscode', 'tasks.json'), '"Foo"');
    fs.writeFileSync(path.join(dir, 'x.py'), 'Foo = 2');
    fs.writeFileSync(path.join(dir, 'g.cpp'), Buffer.from('// → Foo\n', 'utf8'));
    const roots = [{ area: 'port', label: 'p', root: dir, kind: 'port' }];
    const mk = ps.makeRe('Foo', { caseSensitive: true });
    const ov = {}; ov[path.resolve(fa).toLowerCase()] = 'int FooEx = 1;\n';
    const disk = await ps.search(roots, mk.re, { plain: true, core: mk.core });
    const inW = await sw.searchInWorker(roots, mk.re, { plain: true, core: mk.core, overlay: ov });
    const gold = await ps.search([{ area: 'golden', label: 'g', root: dir, kind: 'golden', files: [path.join(dir, 'g.cpp')] }], ps.makeRe('→').re, {});
    try { fs.rmSync(dir, { recursive: true, force: true }); } catch (e) { }
    const n = (q, o, t) => ps.hitsIn(t, ps.makeRe(q, o).re, 9).length;
    const got = { disk: disk.hits.map(h => path.basename(h.file)).join(','), ov: inW.hits.filter(h => /a\.cpp$/.test(h.file)).map(h => h.col + ':' + h.text.trim()).join('|'),
      gold: gold.hits.length ? gold.hits[0].col : null,
      ww: [n('->Run', { wholeWord: true }, 'p->Run(); p->RunX();'), n('.Caption', { wholeWord: true }, 'a.Caption a.Captions'), n('Run', { wholeWord: true }, 'Run RunX xRun')].join(','),
      dead: dc.deadRanges('a\rb\n#if 0\nfoo\n#endif\nfoo').map(x => x.from + '-' + x.to).join(',') };
    ok(got.disk === 'tasks.json,a.cpp,g.cpp,x.py' && got.ov === '5:int FooEx = 1;' && got.gold === 4 && got.ww === '1,1,1' && got.dead === '3-3',
      '1009 review: an open unsaved document searched as the editor has it (in the worker too); .vscode and .py searched; whole word only checks an edge the keyword has a letter at (->Run, .Caption); a golden file valid as UTF-8 read as UTF-8; #if 0 after a lone CR on the right line', JSON.stringify(got));
  }
  // --- projectsearch (AI 20261001, 0.137): one keyword over the port / web / golden trees; golden read as Big5
  {
    const ps = require('../lib/projectsearch');
    const big5 = Buffer.from([0xa7, 0x6c, 0xbc, 0x4c]);   // 吸嘴 in Big5
    const dg = ps.decode(big5, 'golden'), dpU = ps.decode(Buffer.from('﻿a吸嘴', 'utf8'), 'port'), dpB = ps.decode(Buffer.concat([Buffer.from('x '), big5]), 'port');
    const hs = ps.hitsIn('line one\n  call DoInArm(1); DoInArm(2)\nDoInArmX\n', ps.makeRe('doinarm').re, 10);
    const ww = ps.hitsIn('spbSave spbSaveClick xspbSave spbSave_1 spbSave\n', ps.makeRe('spbSave', { wholeWord: true }).re, 10);
    const cs = ps.hitsIn('Save save SAVE', ps.makeRe('Save', { caseSensitive: true }).re, 10);
    const rx = ps.hitsIn('E74 E075 E7a', ps.makeRe('E\\d+', { regex: true }).re, 10);
    const badRx = ps.makeRe('(', { regex: true });
    const long = ps.hitsIn('x'.repeat(1000) + 'KEY' + 'y'.repeat(1000), ps.makeRe('KEY').re, 5)[0];
    const gold = r.goldenRoot;   // (the golden tree found at the top of this test)
    let real = null;
    if (gold) {
      const areas = [{ area: 'golden', label: 'golden', root: gold, kind: 'golden' }, { area: 'port', label: 'port', root: PORT, kind: 'port' }];
      real = await ps.search(areas, ps.makeRe('吸嘴').re, { limit: 200000 });
    }
    const got = {
      dg: dg.text + '/' + dg.enc, dpU: dpU.text + '/' + dpU.enc, dpB: dpB.text + '/' + dpB.enc,
      hs: hs.map(h => h.line + ':' + h.col + ':' + h.len).join(','), ww: ww.map(h => h.col).join(','), cs: cs.map(h => h.col).join(','),
      rx: rx.map(h => h.col + '/' + h.len).join(','), badRx: !!badRx.error, long: long && long.text.length <= 302 && long.text.slice(long.col - 1, long.col + 2) === 'KEY',
      real: real ? { golden: real.hits.filter(h => h.area === 'golden').length, goldenBig5: real.hits.filter(h => h.area === 'golden').every(h => h.enc === 'big5' && /吸嘴/.test(h.text)), port: real.hits.filter(h => h.area === 'port').length } : null,
    };
    // the scopes (Visual Studio's "Look in"): a file under the port's tools\ is the port's (the deepest root)
    const RR = [{ area: 'port', label: 'C++', root: 'D:\\X\\port' }, { area: 'web', label: 'web', root: 'D:\\X\\web' }, { area: 'golden', label: 'g', root: 'D:\\X\\gold' }];
    const sF = ps.scopeRoots(RR, 'file', 'D:\\X\\gold\\main.cpp'), sP = ps.scopeRoots(RR, 'project', 'D:\\X\\web\\page\\a.html'), sS = ps.scopeRoots(RR, 'solution');
    const sN = ps.scopeRoots(RR, 'project', 'C:\\elsewhere\\a.cpp');
    const scopes = [sF.roots.length + ':' + sF.roots[0].area + ':' + (sF.roots[0].files || []).join('|') + ':' + sF.label, sP.roots.map(x => x.area).join(',') + ':' + sP.label, sS.roots.length, !!sN.error,
      (ps.areaOf(RR, 'd:\\x\\PORT\\tools\\a.js') || {}).area].join(' / ');
    ok(scopes === '1:golden:D:\\X\\gold\\main.cpp:main.cpp / web:web / 3 / true / port',
      'projectsearch scopes: current file (decoded as its tree: golden = Big5) / current project / entire solution; a file outside every tree has no project', scopes);
    ok(got.dg === '吸嘴/big5' && got.dpU === 'a吸嘴/utf8' && got.dpB === 'x 吸嘴/big5' && got.hs === '2:8:7,2:20:7,3:1:7' && got.ww === '1,41' && got.cs === '1' &&
      got.rx === '1/3,5/4,10/2' && got.badRx && got.long && (!got.real || (got.real.golden > 100 && got.real.goldenBig5)),
      'projectsearch: golden decoded as Big5 (吸嘴 found there -- VS Code\'s Find in Files reads it as UTF-8 and never matches), UTF-8 elsewhere (a non-UTF-8 file falls back to Big5); case-insensitive by default, whole word, case, regex (a bad one said); line / column; a long line cut around the hit',
      JSON.stringify(got));
    // (1003 machine: files read 32 at a time -- the list is still in file order, roots in their order; the limit cuts
    //  in that order; a cancel stops it) -- a throw-away tree of 120 files in nested folders
    const pDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_par_'));
    const pA = path.join(pDir, 'a'), pB = path.join(pDir, 'b');
    for (let i = 0; i < 120; i++) {
      const d = path.join(i % 2 ? pB : pA, 'd' + (i % 5), 'e' + (i % 3));
      fs.mkdirSync(d, { recursive: true });
      // (bigger files first in name order: a slow read must not move its hits later)
      fs.writeFileSync(path.join(d, 'f' + String(i).padStart(3, '0') + '.txt'), 'x'.repeat(i < 10 ? 200000 : 10) + '\nKEY ' + i + '\nKEY again\n');
    }
    const pR = [{ area: 'one', root: pA, kind: 'port' }, { area: 'two', root: pB, kind: 'port' }];
    const pAll = await ps.search(pR, ps.makeRe('KEY').re, {});
    const pWant = [].concat(...await Promise.all(pR.map(async x => (await ps.listFiles(x.root)).map(f => [x.area, f]))));
    const pOrder = pAll.hits.filter((h, i) => i % 2 === 0).map(h => h.area + '|' + h.file).join('\n') === pWant.map(w => w.join('|')).join('\n');
    const pLim = await ps.search(pR, ps.makeRe('KEY').re, { limit: 7 });
    const pLimOk = pLim.hits.length === 7 && pLim.truncated && pLim.hits.map(h => h.file).join() === pAll.hits.slice(0, 7).map(h => h.file).join();
    let pN = 0;
    const pCx = await ps.search(pR, ps.makeRe('KEY').re, { cancelled: () => ++pN > 5 });
    try { fs.rmSync(pDir, { recursive: true, force: true }); } catch (e) { /* temp */ }
    ok(pAll.hits.length === 240 && pAll.files === 120 && pAll.scanned === 120 && pOrder && pLimOk && pCx.cancelled && pCx.truncated && pCx.scanned < 120,
      'projectsearch reads 32 files at once: the hits in file order (roots in order, a big file first keeps its place), the limit cut in that order, a cancel stops it',
      JSON.stringify({ n: pAll.hits.length, files: pAll.files, pOrder, lim: pLim.hits.length, limT: pLim.truncated, cx: { c: pCx.cancelled, s: pCx.scanned } }));
  }

  // --- filelog (1003 machine, EastSun: "你要寫LOG紀錄 才能查異常" + "並且定時清理LOG"): a file per day, lines with the time and
  //     the level (a stack's lines indented); cleanup: older than keepDays out, then the oldest while over maxMB; today's stays
  {
    const fl = require('../lib/filelog');
    const ld = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_log_'));
    let clk = new Date(2026, 9, 3, 21, 5, 9, 7);
    const lg = fl.create(ld, { keepDays: 14, maxMB: 1, now: () => clk });
    lg.write('INFO', 'start');
    lg.write('ERROR', 'boom\n  at x.js:1');
    const today = fs.readFileSync(path.join(ld, 'htdesigner_20261003.log'), 'utf8');
    // old files: 20 days old (out by days); 10 days old (kept by days -- but over the size cap the oldest go first, so it goes,
    // then the big one 5 days old); today and a file not ours stay
    fs.writeFileSync(path.join(ld, 'htdesigner_20260913.log'), 'old\n');
    fs.writeFileSync(path.join(ld, 'htdesigner_20260923.log'), 'ten\n');
    fs.writeFileSync(path.join(ld, 'htdesigner_20260928.log'), 'x'.repeat(1100 * 1024));
    fs.writeFileSync(path.join(ld, 'notours.txt'), 'keep');
    const cr = lg.cleanup();
    const left = fs.readdirSync(ld).sort().join(',');
    fs.rmSync(ld, { recursive: true, force: true });
    ok(/^2026-10-03 21:05:09\.007 \[INFO\] start\n2026-10-03 21:05:09\.007 \[ERROR\] boom\n {4} {2}at x\.js:1\n$/.test(today) &&
      cr.removed.join(',') === 'htdesigner_20260913.log,htdesigner_20260923.log,htdesigner_20260928.log' && left === 'htdesigner_20261003.log,notours.txt',
      'filelog: one file per day, time + level per line (a stack indented); cleanup: older than keepDays out, then the oldest over maxMB, today and other files stay',
      JSON.stringify({ today, removed: cr.removed, left }));
  }

  // --- uptodate (1005 machine, EastSun: "如果已經編譯過了 請按下F5的時候 就直接啟動軟體"): the exe newer than every source and its
  //     build folder's CMake files = no build; a newer source / CMakeCache / a missing or tiny exe = build; build* / tests skipped
  {
    const ut = require('../lib/uptodate');
    const ud = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_ut_'));
    const T = s => new Date(Date.now() - s * 1000);
    const put = (rel, body, ageS) => { const p = path.join(ud, rel); fs.mkdirSync(path.dirname(p), { recursive: true }); fs.writeFileSync(p, body); fs.utimesSync(p, T(ageS), T(ageS)); return p; };
    put('a.cpp', 'x', 600); put('sub/b.h', 'x', 500); put('CMakeLists.txt', 'x', 700);
    put('tests/t.cpp', 'x', 10);                     // (tests: not in the program)
    put('build_x/CMakeCache.txt', 'x', 400);
    const exe = put('build_x/wb.exe', fakePE(70 * 1024), 100);
    const r1 = ut.check(exe, ud);
    put('sub/b.h', 'y', 50);                         // a header edited after the build
    const r2 = ut.check(exe, ud);
    put('sub/b.h', 'y', 500);
    put('build_x/CMakeCache.txt', 'y', 20);          // configured again
    const r3 = ut.check(exe, ud);
    put('build_x/CMakeCache.txt', 'y', 400);
    const small = put('build_x/cut.exe', 'tiny', 1);
    const r4 = ut.check(small, ud), r5 = ut.check(path.join(ud, 'build_x', 'none.exe'), ud);
    fs.rmSync(ud, { recursive: true, force: true });
    const got = [r1.upToDate, r2.upToDate, /b\.h/.test(r2.why), r3.upToDate, /CMakeCache/.test(r3.why), r4.upToDate, r5.upToDate, r1.newest && r1.newest.count].join(',');
    ok(got === 'true,false,true,false,true,false,false,3', 'uptodate: exe newer than every source = no build (tests / build folders not counted); a newer header, a newer CMakeCache, a tiny or missing exe = build, with the reason', got);
  }

  // --- buildmem (1008 machine, EastSun「可以自動偵測該電腦記憶體 編譯自動配置?」): -j from the free memory, never 0, capped by
  //     cores - 2 and the maximum; the machine's own numbers (7.7 GB, 28 cores) give 1-2 jobs where the old rule gave 6
  {
    const bm = require('../lib/buildmem');
    const P = o => bm.plan(Object.assign({ totalMB: 7900, cores: 28, perJobMB: 700, reserveMB: 1200, maxJobs: 6 }, o));
    const a = P({ availMB: 1500 }), b = P({ availMB: 3000 }), c = P({ availMB: 60000 }), d = P({ availMB: 60000, cores: 4 }),
      e = P({ availMB: 500 }), f = P({ availMB: 3000, perJobMB: 350 }), g = bm.plan({}), h = P({ availMB: 6000 });
    const got = [a.jobs, b.jobs, c.jobs, d.jobs, e.jobs, f.jobs, g.jobs >= 1 && g.availMB > 0 && g.totalMB > 0, /同時 1 個編譯/.test(b.why) && /保留 2370 MB/.test(b.why), /上限/.test(c.why), /CPU/.test(d.why), h.jobs].join(',');
    ok(got === '1,1,6,2,1,1,true,true,true,true,5', 'buildmem (1009 second review: never past 70 % of all memory -- 30 % kept back at least): -j = (free - reserve) / per job, at least 1, at most cores - 2 and the maximum; the reason said; this PC read when nothing is given', got);
  }

  // --- htd_f5_mode.ps1 -PlanOnly (1008 machine): EastSun's F5 "閃退" = build_f5_ship_release__* failing at wb_serve.rc --
  //     its cache had CMAKE_RC_COMPILER=windres (bare) and the toolchain bin was not on PATH ('gcc' is not recognized).
  //     The fix seeds CMAKE_RC_COMPILER and puts the baseline compiler's bin first on PATH; HTD_BUILD_JOBS sets -j.
  {
    const cp = require('child_process'), osm = require('os');
    const td = fs.mkdtempSync(path.join(osm.tmpdir(), 'htd_f5m_'));
    const tc = path.join(td, 'tc', 'bin'), base = path.join(td, 'build_base'), dir = path.join(td, 'build_f5_x');
    fs.mkdirSync(tc, { recursive: true }); fs.mkdirSync(base, { recursive: true });
    fs.writeFileSync(path.join(tc, 'g++.exe'), '');
    fs.writeFileSync(path.join(base, 'CMakeCache.txt'), ['CMAKE_GENERATOR:INTERNAL=MinGW Makefiles', 'CMAKE_CXX_COMPILER:FILEPATH=' + tc.replace(/\\/g, '/') + '/g++.exe',
      'CMAKE_C_COMPILER:FILEPATH=' + tc.replace(/\\/g, '/') + '/gcc.exe', 'CMAKE_RC_COMPILER:FILEPATH=windres', ''].join('\n'));
    let plan = null, err = '';
    try {
      const out = cp.execFileSync('powershell.exe', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', path.join(__dirname, '..', 'media', 'htd_f5_mode.ps1'),
        '-Tree', td, '-Dir', dir, '-Baseline', base, '-Sim', '0', '-Dbg', '0', '-PlanOnly'], { encoding: 'utf8', env: Object.assign({}, process.env, { HTD_BUILD_JOBS: '3' }), timeout: 60000 });
      const line = out.split(/\r?\n/).find(l => /^\{/.test(l.trim()));
      plan = line ? JSON.parse(line) : null;
    } catch (e) { err = String(e.message || e).slice(0, 200); }
    try { fs.rmSync(td, { recursive: true, force: true }); } catch (e) { /* left in %TEMP% */ }
    const cfg = plan && plan.configure || [], bld = plan && plan.build || [];
    const got = { rc: cfg.includes('-DCMAKE_RC_COMPILER=windres'), cxx: cfg.some(a => /^-DCMAKE_CXX_COMPILER=.*\/g\+\+\.exe$/.test(a)),
      path: !!plan && path.resolve(String(plan.pathHead || '')).toLowerCase() === path.resolve(tc).toLowerCase(), jobs: bld[bld.length - 1], err };
    ok(got.rc && got.cxx && got.path && got.jobs === '3',
      'htd_f5_mode (1008): the baseline\'s CMAKE_RC_COMPILER seeded into the mode build, its compiler\'s bin first on PATH (windres finds gcc), HTD_BUILD_JOBS = the -j',
      JSON.stringify(got));
  }

  // --- the 尋找 window's filters (1003 machine, EastSun: "可以加入讓我勾選篩選條件嗎? 我想加入 被賦予值 或是被當成判斷式 或是是函式
  //     的篩選"): each use of the name classified; comments / strings are none of them
  {
    const ps = require('../lib/projectsearch');
    const { mask } = require('../lib/cppstub');
    const src = [
      'iHome = 1;', 'iHome==0', 'if (x && iHome) y();', 'a[iHome] = 2;', 'iHome[3] = 4;', 'iHome++;', '--iHome;', 'iHome += 2;',
      'for (i = 0; iHome < 3; i++)', 'DoHome(1);', 'void DoHome(int a)', '// iHome = 9', 's = "iHome = 1";', 'while (!iHome)', 'p = iHome ? 1 : 2;',
      'x.iHome = 5;', 'q = iHomeLed;', 'iHome.Down = true;',
    ];
    const txt = src.join('\n'), mk = mask(txt);
    const res = src.map((l, i) => {
      const off = src.slice(0, i).reduce((s, x) => s + x.length + 1, 0);
      const w = /DoHome/.test(l) ? 'DoHome' : 'iHome';
      const c = ps.classify(mk, txt, off + l.indexOf(w), w.length);
      return c.code ? ['assign', 'cond', 'func'].filter(k => c[k]).join('+') || 'other' : 'comment';
    }).join(' | ');
    const want = 'assign | cond | cond | other | assign | assign | assign | assign | cond | func | func | comment | comment | cond | cond | assign | other | assign';
    ok(res === want, 'find filters: = / += / ++ / -- / x[i] = / x.y = -> assigned; if / while / for ( ), == < && ! ? -> a condition; name( -> a function; an index / a longer name -> other; in a comment / string -> none', res);
  }

  // --- funcRange (1003 machine, EastSun: "你用事件轉跳到程式碼時 我希望你用不同底色 標出轉跳到的程式碼"): the lines of the
  //     function a jump lands on -- header to closing brace; braces in comments / strings do not count; a statement or
  //     a declaration line is not a function
  {
    const cst = require('../lib/cppstub');
    const src = [
      'void __fastcall TfA::B(TObject *Sender)',   // 0
      '{',                                          // 1
      '  if (x) { y("}"); } // }',                  // 2
      '  /* { */ z();',                             // 3
      '}',                                          // 4
      'int decl(int a);',                           // 5
      'function jsFn(e) {',                         // 6
      '  return 1;',                                // 7
      '}',                                          // 8
      '  call();',                                  // 9
    ].join('\r\n');
    const fr = [0, 5, 6, 9, 4].map(l => { const r = cst.funcRange(src, l); return r ? r.start + '-' + r.end : '-'; }).join(' ');
    ok(fr === '0-4 - 6-8 - -', 'funcRange: a C++ handler (brace on the next line, braces in strings / comments skipped) and a JS function -> header to closing brace; a declaration / statement / closing line -> none', fr);
  }

  // --- cppsymbols (AI 20261001, 0.140): Visual Studio's navigation bar (類別 ▾ | 成員 ▾) for the breadcrumbs
  {
    const cs = require('../lib/cppsymbols');
    const src = [
      '#include "x.h"', '// class Fake { int no; };', 'class TfA : public TForm {', 'public:',
      '    TEdit *Edit1 = new TEdit();', '    int iCount;', '    char name[20];', '    void __fastcall Go(TObject *Sender);',
      '    int Get() const { return iCount; }', '    TfA();', '};', 'struct S { int a; };',
      'void TfA::Go(TObject *Sender)', '{', '    if (iCount) Get();', '    Run(1);', '}', 'TfA::TfA()', '{', '}',
      'static int Free(int a, int b)', '{', '    return a + b;', '}', 'int g = Free(1, 2);', '',
    ].join('\n');
    const sy = cs.symbolsOf(src);
    const flat = s => s.kind + ':' + s.name + (s.children && s.children.length ? '[' + s.children.map(flat).join(',') + ']' : '');
    const got = sy.map(flat).join(' ');
    const goAt = src.indexOf('Run(1)');
    const chain = cs.chainAt(sy, goAt).map(s => s.name).join('>');
    const hpH = path.join(PORT, 'forms', 'fHotPlate.h'), hpC = path.join(PORT, 'forms', 'fHotPlate.cpp');
    const real = fs.existsSync(hpH) ? { h: cs.symbolsOf(fs.readFileSync(hpH, 'utf8')), c: cs.symbolsOf(fs.readFileSync(hpC, 'utf8')) } : null;
    const realGot = real ? {
      hClass: real.h.filter(s => s.kind === 'class').map(s => s.name).join(','),
      hFields: real.h[0] ? real.h[0].children.filter(c => c.kind === 'field').map(c => c.name).slice(0, 3).join(',') : '',
      hNoTEdit: real.h[0] ? !real.h[0].children.some(c => c.name === 'TEdit') : false,
      cDefs: real.c.filter(s => s.kind === 'class' && s.defs).map(s => s.name + ':' + s.children.length).join(','),
    } : null;
    ok(got === 'class:TfA[field:Edit1,field:iCount,field:name,method:Go,method:Get,method:TfA] struct:S[field:a] class:TfA[method:Go,method:TfA] function:Free' &&
      chain === 'TfA>Go' && (!realGot || (realGot.hClass === 'TfHotPlate' && /HotPlateName/.test(realGot.hFields) && realGot.hNoTEdit && /^TfHotPlate:\d+$/.test(realGot.cDefs))),
      'cppsymbols: classes with their fields (an = new X() initializer is a field) and methods, out-of-class definitions grouped by class, free functions; calls / comments / #include are not symbols; the chain at the cursor = class > member; the real fHotPlate.h / .cpp',
      got + ' | ' + chain + ' | ' + JSON.stringify(realGot));
  }

  // --- solutiontree (AI 20261001, 0.138): Visual Studio's Solution Explorer -- folders first A->Z, then files; the
  // search keeps the files with every word in the name / path, and their folders
  {
    const st = require('../lib/solutiontree');
    const ps = require('../lib/projectsearch');
    const tmp = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-sln-'));
    for (const d of ['b', 'A', 'build_x', 'A/sub']) fs.mkdirSync(path.join(tmp, d), { recursive: true });
    for (const f of ['z.cpp', 'a.h', 'A/fHotPlate.h', 'A/sub/fHotPlate.cpp', 'b/other.cpp', 'build_x/fHotPlate.o']) fs.writeFileSync(path.join(tmp, f), 'x');
    const top = (await st.entries(tmp)).map(e => (e.dir ? '/' : '') + e.name).join(',');
    const files = await st.allFiles(tmp);
    const fl = st.filter(tmp, files, 'fhotplate');
    const fl2 = st.filter(tmp, files, 'FHOT .cpp');
    const fl3 = st.filter(tmp, files, 'a/sub');
    const rel = s => Array.from(s).map(x => path.relative(tmp.toLowerCase(), x).replace(/\\/g, '/')).sort().join(',');
    const sFo = ps.scopeRoots([{ area: 'port', label: 'C++', root: tmp }], 'folder', path.join(tmp, 'A'));
    const sFoHits = (await ps.search(sFo.roots, ps.makeRe('x').re, {})).hits.map(h => h.rel.replace(/\\/g, '/')).sort().join(',');
    fs.rmSync(tmp, { recursive: true, force: true });
    const got = { top, n: files.length, fl: rel(fl.files) + ' | ' + rel(fl.dirs), fl2: rel(fl2.files), fl3: rel(fl3.files), none: st.filter(tmp, files, '').count, sFo: sFo.label + ' ' + sFoHits };
    ok(got.top === '/A,/b,a.h,z.cpp' && got.n === 5 && got.fl === 'a/fhotplate.h,a/sub/fhotplate.cpp | a,a/sub' && got.fl2 === 'a/sub/fhotplate.cpp' && got.fl3 === 'a/sub/fhotplate.cpp' &&
      got.none === 0 && /A\\/.test(sFo.label) && got.sFo.endsWith('fHotPlate.h,sub/fHotPlate.cpp'),
      'solutiontree: folders first then files, A->Z, build* hidden (Solution Explorer); search = every word in the name or path (a word with / looks at the path), with the folders up to the project; the folder scope searches only that folder',
      JSON.stringify(got));
  }

  // --- items (AI 20261001, 0.157, WPF's Items collection editor): a select's options, a radio group's items, a memo's
  // lines read and written as the generator writes them; the selected one kept by its text
  {
    const it = require('../lib/items');
    const t = '<div><select class="ed" id="C1" title="C1 : TComboBox"><option>a</option><option selected>b &amp; c</option></select>' +
      '<fieldset class="gbx rg" id="R1" title="R1 : TRadioGroup"><legend>Mode</legend><div class="cli" style="x"><label class="rgi"><input type="radio" name="rg_R1" checked>No message</label><label class="rgi"><input type="radio" name="rg_R1" >Show</label></div></fieldset>' +
      '<textarea class="ed" id="M1" title="M1 : TMemo">l1\nl2</textarea><span id="L1">x</span></div>';
    const ap = (x, e) => x.slice(0, e.range[0]) + e.repl + x.slice(e.range[1]);
    const r0 = [it.itemsOf(t, 'C1'), it.itemsOf(t, 'R1'), it.itemsOf(t, 'M1'), it.itemsOf(t, 'L1')];
    const c1 = ap(t, it.itemsEdit(t, 'C1', ['x', 'b & c', 'y']));
    const r1 = ap(t, it.itemsEdit(t, 'R1', ['Show', 'Other']));
    const m1 = ap(t, it.itemsEdit(t, 'M1', ['a<b', 'c']));
    const bad = it.itemsEdit(t, 'L1', ['x']);
    ok(JSON.stringify(r0) === '[{"kind":"select","items":["a","b & c"],"selected":1},{"kind":"radio","items":["No message","Show"],"selected":0},{"kind":"memo","items":["l1","l2"],"selected":-1},null]' &&
      /<select class="ed" id="C1" title="C1 : TComboBox"><option>x<\/option><option selected>b &amp; c<\/option><option>y<\/option><\/select>/.test(c1) &&
      /<legend>Mode<\/legend><div class="cli" style="x"><label class="rgi"><input type="radio" name="rg_R1" checked ?>Show<\/label><label class="rgi"><input type="radio" name="rg_R1" >Other<\/label><\/div>/.test(r1) &&
      /<textarea class="ed" id="M1" title="M1 : TMemo">a&lt;b\nc<\/textarea><span id="L1">x<\/span>/.test(m1) && !!bad.error,
      'items 0.157: select / radio group / memo read; written as the generator does (the selected kept by its text, a radio group\'s first otherwise, the memo escaped); another element refused',
      JSON.stringify(r0));
    const m2 = ap(t, it.itemsEdit(t, 'M1', ['', 'line2']));
    const m2r = it.itemsOf(m2, 'M1');
    ok(/<textarea[^>]*>\n\nline2<\/textarea>/.test(m2) && JSON.stringify(m2r.items) === '["","line2"]',
      '1009 review (toolbox #10): a memo whose first line is empty -- the break the browser drops after <textarea> written once more, read back the same', JSON.stringify(m2r));
    const td = '<select id="C2" title="C2 : TComboBox"><option>A</option><option selected>A</option></select>';
    const ed2 = it.itemsEdit(td, 'C2', ['A', 'A', 'B']);
    ok(ed2.selected === 1 && /<option>A<\/option><option selected>A<\/option><option>B<\/option>/.test(ed2.repl),
      '1009 review (toolbox #12): the same text twice -- the selected one stays the 2nd "A"', ed2.repl);
  }

  // --- items 1008 (feature gap #4): a StringGrid's cells -- one row a line, Tab between cells; the generator's one
  //   heading over every column (colspan) dropped once the row has more cells; an unchanged row kept as written; <tbody>
  {
    const it = require('../lib/items');
    const ap = (x, e) => x.slice(0, e.range[0]) + e.repl + x.slice(e.range[1]);
    const t = '<div class="sgd" id="G1" title="G1 : TStringGrid" style="x"><table><tr><th colspan="4">G1\uFF08\u57F7\u884C\u671F\uFF09</th></tr><tr><td>--</td><td>a&amp;b</td></tr></table></div><p>';
    const r0 = it.itemsOf(t, 'G1');
    const same = it.itemsEdit(t, 'G1', r0.items);
    const g1 = ap(t, it.itemsEdit(t, 'G1', ['H1\tH2', '--\ta&b\t3', 'x\ty']));
    const g2 = ap(t, it.itemsEdit(t, 'G1', [r0.items[0]]));
    const tb = '<div class="sgd" id="G2"><table><tbody>\n<tr><th>A</th></tr>\n<tr><td>1</td></tr>\n</tbody></table></div>';
    const g3 = ap(tb, it.itemsEdit(tb, 'G2', ['A', '1', '2']));
    const got = { r0, same: same.same, g1, g2, g3 };
    ok(JSON.stringify(r0) === '{"kind":"grid","items":["G1\uFF08\u57F7\u884C\u671F\uFF09","--\\ta&b"],"selected":-1}' && same.same === true &&
      g1 === '<div class="sgd" id="G1" title="G1 : TStringGrid" style="x"><table><tr><th>H1</th><th>H2</th></tr><tr><td>--</td><td>a&amp;b</td><td>3</td></tr><tr><td>x</td><td>y</td></tr></table></div><p>' &&
      g2 === '<div class="sgd" id="G1" title="G1 : TStringGrid" style="x"><table><tr><th colspan="4">G1\uFF08\u57F7\u884C\u671F\uFF09</th></tr></table></div><p>' &&
      g3 === '<div class="sgd" id="G2"><table><tbody>\n<tr><th>A</th></tr>\n<tr><td>1</td></tr>\n<tr><td>2</td></tr>\n</tbody></table></div>' &&
      JSON.stringify(it.itemsOf(g1, 'G1').items) === '["H1\\tH2","--\\ta&b\\t3","x\\ty"]',
      'items 1008: a StringGrid read as rows (Tab between cells); the same = the same text; cells / rows added like their neighbours, the colspan heading dropped when the row grows; a row removed; <tbody> kept', got);
  }

  // --- 1008 (the full test): a Ninja build folder -- a fresh PC's -- read like a Makefiles one: the source's compile command
  //   (DEFINES / INCLUDES / FLAGS of its build.ninja edge, cwd = the folder) and the F5 build's progress from htd_build.log
  {
    const be = require('../lib/builderrors'), bw = require('../lib/buildwatch');
    const nd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-ninja-'));
    const src = path.join(nd, 'src', 'a b.cpp');
    fs.mkdirSync(path.dirname(src)); fs.writeFileSync(src, 'int x;');
    const esc = src.replace(/\\/g, '/').replace(/:/g, '$:').replace(/ /g, '$ ');
    fs.writeFileSync(path.join(nd, 'CMakeCache.txt'), 'CMAKE_CXX_COMPILER:FILEPATH=C:/tc/bin/g++.exe\n');
    fs.writeFileSync(path.join(nd, 'build.ninja'), 'build CMakeFiles/lib1.dir/src/a_b.cpp.obj: CXX_COMPILER__lib1_unscanned_Debug ' + esc + ' || cmake_object_order\n  DEFINES = -DX=1 -DY\n  FLAGS = -g -std=c++14 -Wall -O2\n  INCLUDES = -IC:/inc\n  OBJECT_DIR = CMakeFiles\\lib1.dir\n\nbuild other: phony\n');
    const cc = be.compileCommand(nd, src);
    fs.writeFileSync(path.join(nd, 'htd_build.log'), '[F5 mode] Debug\n[3/40] Building CXX object a.obj\n[10/40] Building CXX object b.obj\n');
    const p1 = bw.cmakeProgress(nd);
    fs.appendFileSync(path.join(nd, 'htd_build.log'), '[40/40] Linking CXX executable wb_serve.exe\n');
    const p2 = bw.cmakeProgress(nd);
    fs.rmSync(nd, { recursive: true, force: true });
    const got = { cc: cc && { compiler: cc.compiler, cwd: cc.cwd === nd, args: cc.args.join(' '), target: cc.target }, p1: p1 && p1.pct, p2 };
    ok(cc && cc.compiler === 'C:/tc/bin/g++.exe' && cc.cwd === nd && cc.target === 'lib1' && cc.args.join(' ') === '-DX=1 -DY -IC:/inc -std=c++14 -fsyntax-only -w -fmax-errors=50 ' + path.resolve(src) &&
      p1 && p1.pct === 25 && p1.steps === 10 && p1.total === 40 && p2 === null,
      'Ninja build folders (1008): the compile command of a source (a path with a space, $-escaped) from build.ninja; the F5 build log\'s [n/m] = progress, m/m = done', got);
  }

  // --- bcbkeys 1008 (EastSun「debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」): the user keybindings.json gets the five lines --
  //   a new file, after the user's own entries (comma added, comments kept), idempotent, updated when they change, removed
  {
    const bk = require('../lib/bcbkeys');
    const strip = t => JSON.parse(t.replace(/\/\/[^\n]*/g, '').replace(/,(\s*\])/g, '$1'));
    const a = bk.ensure('');
    const user = '// mine\n[\n  { "key": "ctrl+k", "command": "x" } // keep\n]\n';
    const b = bk.ensure(user);
    const again = bk.ensure(b);
    const old = b.replace('workbench.action.debug.continue', 'workbench.action.debug.start');
    // (1009 review (keys #1): an older version's line is ours only when it is known -- what was written before)
    const oldLine = old.split(/\r?\n/).find(l => /debug\.start/.test(l)).replace(/,\s*$/, '');
    const upd = bk.ensure(old, { known: new Set([bk.norm(oldLine)]) });
    const notMine = bk.ensure(old);
    const rem = bk.remove(b);
    const got = { a: strip(a).map(k => k.key + '=' + k.command).join(','), b: strip(b).length, keep: /\/\/ keep/.test(b) && /\/\/ mine/.test(b), again, upd: upd && strip(upd).some(k => k.command === 'workbench.action.debug.continue') && !/debug\.start/.test(upd), notMine: !!notMine && /debug.start/.test(notMine), rem: JSON.stringify(strip(rem)) === JSON.stringify(strip(user)) && /\/\/ keep/.test(rem) && rem.indexOf(bk.MARK) < 0, bad: !!(bk.ensure('{ "x": 1 }') || {}).error };
    ok(got.a.startsWith('f8=workbench.action.debug.stepOver,f7=workbench.action.debug.stepInto,f9=workbench.action.debug.continue,f4=editor.debug.action.runToCursor,ctrl+f2=ht9045Designer.run.stopAll,') && /f7=-cmake\.build/.test(got.a) && /ctrl\+f9=ht9045Designer\.run\.make/.test(got.a) &&
      got.b === bk.KEYS.length + 1 && got.keep && got.again === null && got.upd && got.notMine && got.rem && got.bad,
      'bcbkeys (1008): F8 Step Over / F7 Trace Into / F9 Run / F4 Run to Cursor / Ctrl+F2 Program Reset written to the user keybindings (new file, after the user\'s entries, comments kept), once, updated, removed', got);
  }

  // --- 1008 full test (audit D1 / C3 / H1): "#if 0 || defined(X)" is not dead, "#if 0 // why" and "#if (0)" are; a u8R / LR raw
  //   string is skipped like R; gdb's SIGSEGV and a 64-bit lock offset (0x24) are named as an all-zero lock
  {
    const dc = require('../lib/deadcode'), cc = require('../lib/cppcheck'), ch = require('../lib/crashhint');
    const t = ['#if 0 || defined(X)', 'live1;', '#endif', '#if 0 // GATE', 'dead1;', '#endif', '#if (0)', 'dead2;', '#endif', '#if 0 + 1', 'live2;', '#endif'].join('\n');
    const dr = dc.deadRanges(t).map(x => (x.from + 1) + '-' + (x.to + 1)).join(',');
    const raw = cc.checkText('auto a = u8R"(\u4E2D "x")";\nauto b = LR"y(\uFF1B)y";\nint c\uFF1B\n', { gccMajor: 6 }).map(p => p.line + ':' + p.kind).join(',');
    const F = (name, p, line) => ({ name, path: p, line });
    const lockStack = [F('webbridge::WbMutex::lock()', 'D:/t/WebBridge/Sync.h', 60), F('X::y()', 'D:/t/X.cpp', 9)];
    const s1 = ch.hintOf('Program received signal SIGSEGV, Segmentation fault.', lockStack);
    const s2 = ch.hintOf('Exception 0xc0000005: Access violation writing location 0x00000024', lockStack);
    const got = { dr, raw, s1: s1.title, s2: s2.title };
    ok(dr === '5-5,8-8' && raw === '2:lookalike' && /鎖/.test(s1.title) && /鎖/.test(s2.title),
      'audit D1 / C3 / H1 (1008): only a whole 0 condition is dead; prefixed raw strings skipped; SIGSEGV / 0x24 in a lock = an all-zero lock', got);
  }

  // --- cppconfig 1008 (gap list #7): IntelliSense from a compile command -- @rsp read (relative to the build folder), -I /
  //   -isystem / "-I path" absolute, -D kept, -std mapped; the real build_nonoracle's cBootLog.cpp
  {
    const cc = require('../lib/cppconfig'), be = require('../lib/builderrors');
    const nd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-cfg-'));
    fs.writeFileSync(path.join(nd, 'inc.rsp'), '-ID:/a "-ID:/b c" -I rel/x\n');
    const a = cc.fromArgs(['-DX=1', '-D', 'Y', '@inc.rsp', '-isystem', 'D:/sys', '-std=gnu++1z', '-m32', '-Wall'], nd);
    fs.rmSync(nd, { recursive: true, force: true });
    const real = be.compileCommand(path.join(PORT, 'build_nonoracle'), path.join(PORT, 'Public', 'cBootLog.cpp'));
    const r = real ? cc.fromArgs(real.args.slice(0, -1), real.cwd) : null;
    const got = { inc: a.includePath.map(p => path.basename(p)).join(','), def: a.defines.join(','), std: a.standard, args: a.compilerArgs.join(','), real: r && { n: r.includePath.length, std: r.standard, def: r.defines.slice(0, 3) } };
    ok(got.inc === 'a,b c,x,sys' && got.def === 'X=1,Y' && got.std === 'c++17' && got.args === '-m32' && (!real || (r.includePath.some(p => /HT9011UC_Cpp/i.test(p)) && r.defines.length > 0)),
      'cppconfig (1008): a compile command -> IntelliSense (rsp, absolute includes, defines, standard); the tree\'s own build_nonoracle read', got);
  }

  // --- 1008 full test (audit C4 / C7 / E3): "http://x" \ continues code (its next line checked); an #endif inside a raw
  //   string is not a directive; "required from here" is a place of the problem before it
  {
    const cc = require('../lib/cppcheck'), be = require('../lib/builderrors');
    const c4 = cc.checkText('#define URL "http://x" \\\nint a\uFF1B\n// note \\\n\u4E2D\n', { gccMajor: 6 }).map(p => p.line + ':' + p.kind).join(',');
    const c7 = cc.checkText('auto s = R"(\n#endif\n)";\n#if 0\nint d\uFF1B\n#endif\nint e\uFF1B\n', { gccMajor: 6 }).map(p => p.line + ':' + p.kind).join(',');
    const e3 = be.parse(['D:/t/a.h:5:3: error: no match', 'D:/t/a.cpp:20:7:   required from here', 'D:/t/b.cpp:1:1: error: x'].join('\n'), 'D:/t');
    const got = { c4, c7, e3: e3.map(p => p.line + '+' + p.related.length).join(',') };
    ok(c4 === '1:lookalike' && c7 === '6:lookalike' && got.e3 === '5+1,1+0',
      'audit C4 / C7 / E3 (1008): a // in a string is not a comment; #endif in a raw string ignored; "required from here" joins its problem', got);
  }

  // --- finderror 1008 (gap list #21): the addresses typed (a bootsample line, 0x..., module+offset), addr2line's frames, the
  //   program's own first frame; the ImageBase of the tree's built wb_serve.exe
  {
    const fe = require('../lib/finderror');
    const a1 = fe.parseAddrs('main: 016078cb 0160850b 0130b23d', null).addrs.join(',');
    const a2 = fe.parseAddrs('wb_serve.exe+0x1234', 0x400000n).addrs.join(',');
    const a3 = fe.parseAddrs('+1234', null).error;
    const fr = fe.frames('_ZNSt6vectorE\nC:/tc/mingw32/include/c++/16.2.0/bits/vector.tcc:131\nTMyX::Run()\nD:/t/tree/Foo.cpp:42 (discriminator 3)\n??\n??:?\n');
    const fo = fe.focus(fr, 'D:/t/tree');
    const exe = path.join(PORT, 'build_nonoracle', 'wb_serve.exe');
    const ib = fs.existsSync(exe) ? fe.imageBase(exe) : 0x400000n;
    const got = { a1, a2, a3: !!a3, fr: fr.map(f => f.fn + '@' + f.line).join(','), fo: fo && fo.fn, ib: ib && ib.toString(16), dm: fe.needsDemangle('ZN9vclcompat3FooEv') };
    ok(a1 === '0x16078cb,0x160850b,0x130b23d' && a2 === '0x401234' && got.a3 && got.fr === '_ZNSt6vectorE@131,TMyX::Run()@42,??@0' && got.fo === 'TMyX::Run()' && got.ib === '400000' && got.dm,
      'finderror (1008): bootsample addresses, module+offset (ImageBase), addr2line frames, the program\'s own first frame', got);
  }

  // --- snippets 1008 (gap list #23): the C++ code templates -- valid JSON, each with a prefix and a body; the AI note in the
  //   tree's format (//AI(W906-<tag>) YYYYMMDD: ...); contributed for cpp and c
  {
    const sn = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'snippets', 'cpp.json'), 'utf8'));
    const pk = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
    const all = Object.values(sn);
    const ai = sn['AI change note (W906)'];
    const got = { n: all.length, ok: all.every(x => x.prefix && Array.isArray(x.body) && x.body.length), ai: ai && ai.body[0], langs: (pk.contributes.snippets || []).map(x => x.language).join(',') };
    ok(got.n >= 8 && got.ok && /^\/\/AI\(W906-\$\{1:TAG\}\) \$\{CURRENT_YEAR\}\$\{CURRENT_MONTH\}\$\{CURRENT_DATE\}: /.test(got.ai) && got.langs === 'cpp,c',
      'snippets (1008): the C++ templates (Ctrl+J) are valid, the AI note in the tree\'s format, contributed for cpp and c', got);
  }

  // --- defineswitch 1008 (gap list #22): the #define switches -- on / off by the leading //, the include guard and an #if 0
  //   block left out, a toggle keeps the comment and the CRLF; the tree's own MachineType.h read
  {
    const ds = require('../lib/defineswitch');
    const t = '#ifndef MT_H\r\n#define MT_H\r\n#define A   // note a\r\n//#define B\r\n// #define C /* c */\r\n#define D 5\r\n#if 0\r\n#define E\r\n#endif\r\n';
    const l = ds.list(t);
    const on = ds.toggle(t, l.find(x => x.name === 'B'), true), off = ds.toggle(t, l.find(x => x.name === 'A'), false), same = ds.toggle(t, l.find(x => x.name === 'A'), true);
    const mt = ds.list(fs.readFileSync(path.join(PORT, 'MachineType.h'), 'utf8'));
    const got = { l: l.map(x => x.name + (x.on ? '+' : '-')).join(','), on: on && on.text, off: off && off.text, same, mt: mt.length, guard: mt.some(x => x.name === 'MachineTypeH') };
    ok(got.l === 'A+,B-,C-' && got.on === '#define B\r' && got.off === '//#define A   // note a\r' && same === null && got.mt > 40 && !got.guard,
      'defineswitch (1008): #define switches on / off (guard, values, #if 0 left out), a toggle keeps the comment and CRLF; MachineType.h has ' + got.mt, got);
  }

  // --- bookmarks 1008 (gap list #11): set / same line again = clear / another slot on that line goes; lines follow an edit
  //   above (inserted / deleted lines), a bookmark inside a deleted range stays at its start
  {
    const bm = require('../lib/bookmarks');
    let s = bm.toggle({}, 1, 'D:/a.cpp', 10);
    s = bm.toggle(s, 2, 'D:\\A.CPP', 20);
    const cleared = bm.toggle(s, 1, 'D:/a.cpp', 10);
    const moved = bm.toggle(s, 3, 'D:/a.cpp', 20);
    const ins = bm.shift(s, 'D:/a.cpp', 5, 5, 3);      // (2 lines inserted at 5)
    const del = bm.shift(s, 'D:/a.cpp', 8, 15, 1);     // (lines 8..15 removed)
    const got = { s: JSON.stringify(s), cleared: Object.keys(cleared).join(','), moved: Object.keys(moved).join(','), ins: ins['1'].line + '/' + ins['2'].line, del: del['1'].line + '/' + del['2'].line, inFile: bm.inFile(s, 'd:\\a.cpp').map(x => x.n).join(',') };
    ok(got.cleared === '2' && got.moved === '1,3' && got.ins === '12/22' && got.del === '8/13' && got.inFile === '1,2',
      'bookmarks (1008): toggle / clear / one per line, lines follow inserts and deletes above them', got);
    const b10 = { 1: { file: 'x', line: 10 } };
    const L = (...a) => bm.shift(b10, 'x', ...a)[1].line;
    const g2 = [L(10, 10, 2, 0), L(10, 10, 4, 0), L(10, 10, 2, 5), L(9, 10, 1, 0)].join(',');
    ok(g2 === '11,13,10,9', '1009 review (C++ nav #4): Enter / a 3-line paste at column 0 of the bookmark\'s line moves it with its text; Enter mid-line keeps it; a line joined up follows', g2);
    const lx = require('../lib/cpplex');
    const ls = JSON.stringify(lx.lineStartsOf('a\r\r\nb\nc\rd')), dl = Array.from(lx.analyze('a\rb\n#if 0\nfoo\n#endif\nx').dead).join('');
    // 1009 review (C++ nav #1 / #2): a golden file valid UTF-8 at the top and Big5 further down -- its definitions found as
    //   definitions; a folder renamed = its files' entries follow
    const { SourceTree } = require('../lib/cppindex');
    const gd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_mix_'));
    const top = Buffer.from('// �� old note\r\nvoid TfX::A()\r\n{\r\n}\r\n', 'utf8');
    const big = Buffer.concat([Buffer.from('// '), Buffer.from([0xa7, 0x6c, 0xbc, 0x4c]), Buffer.from('\r\nvoid TfX::B()\r\n{\r\n}\r\n')]);
    fs.writeFileSync(path.join(gd, 'm.cpp'), Buffer.concat([top, big]));
    fs.mkdirSync(path.join(gd, 'sub')); fs.writeFileSync(path.join(gd, 'sub', 'c.cpp'), 'void TfX::C()\r\n{\r\n}\r\n');
    const gt = new SourceTree(gd, 'golden'); await gt.ensure();
    const defs = [];
    for (const k of ['TfX::A', 'TfX::B']) for (const o of gt.qual.get(k) || []) { const dd2 = await gt.describe(o.file, o.off, o.len); defs.push(k + ':' + dd2.line + (dd2.def ? 'd' : '')); }
    fs.renameSync(path.join(gd, 'sub'), path.join(gd, 'sub2'));
    await gt.rescan(path.join(gd, 'sub')); await gt.rescan(path.join(gd, 'sub2'));
    const cAt = (gt.qual.get('TfX::C') || []).map(o => path.relative(gd, o.file)).join(',');
    try { fs.rmSync(gd, { recursive: true, force: true }); } catch (e) { }
    ok(defs.join(' ') === 'TfX::A:2d TfX::B:6d' && cAt === path.join('sub2', 'c.cpp'),
      '1009 review (C++ nav #1 / #2): a mixed golden file (UTF-8 top, Big5 below) -- both definitions are definitions; a folder renamed moves its entries', JSON.stringify({ defs, cAt }));
    ok(ls === '[0,2,4,6,8]' && dl === '000100', '1009 review (C++ nav #3): a lone CR ends a line in cpplex too (lines as VS Code numbers them; #if 0 after one on the right line)', ls + ' ' + dl);
  }

  // --- cppcheck / builderrors 1008 (full test, audit C1 / C2 / C5 / E4): a Big5 character = ONE problem (not one per
  //   U+FFFD); a BOM does not shift line 1's columns; a CJK Extension B character is one character (a name for g++ 16,
  //   shown whole for g++ 6); the same error from a second unit -- its note is not a stand-alone entry
  {
    const cc = require('../lib/cppcheck'), be = require('../lib/builderrors');
    const buf = Buffer.concat([Buffer.from([0xef, 0xbb, 0xbf]), Buffer.from('int a; '), Buffer.from([0xa4, 0xa4]), Buffer.from(';\n')]);
    const c1 = cc.check(buf.subarray(3).toString('utf8'), buf, { gccMajor: 16 }).map(p => [p.line, p.col, p.len, p.kind].join(':')).join('|');
    const ext = String.fromCodePoint(0x20000);
    const c5a = cc.checkText('int ' + ext + 'x = 1;\n', { gccMajor: 16 }).length;
    const c5b = cc.checkText('int ' + ext + 'x = 1;\n', { gccMajor: 6 }).map(p => p.col + ':' + p.len + ':' + p.msg.includes('「' + ext + '」')).join('|');
    const out = 'D:/x/a.h:3:5: error: bad\nD:/x/a.h:1:1: note: here\nD:/x/a.h:3:5: error: bad\nD:/x/a.h:1:1: note: here\n';
    const e4 = be.parse(out, 'D:/b').map(p => p.severity + ':' + p.line + ':' + (p.related || []).length).join('|');
    ok(c1 === '0:7:2:badutf8' && c5a === 0 && c5b === '4:2:true' && e4 === 'error:3:1',
      'cppcheck / builderrors (1008 audit C1/C2/C5/E4): one problem per Big5 character, BOM-free columns, Extension B whole, a repeated error\'s note swallowed',
      { c1, c5a, c5b, e4 });
  }

  // --- srcmap 1008 (EastSun「可以都統一 開一個檔案嗎?」): the copy a program was built from (its CMakeCache.txt) mapped to the
  //   open tree -- cppdbg sourceFileMap, lldb-dap sourceMap; the same tree (another spelling) = nothing
  {
    const sm = require('../lib/srcmap');
    const caches = { 'D:\\B\\build_x\\CMakeCache.txt': 'CMAKE_HOME_DIRECTORY:INTERNAL=D:/A/Tree\n' };
    const read = f => { const t = caches[path.resolve(f)]; if (t == null) throw new Error('no'); return t; };
    const real = p => p;
    const root = sm.compileRoot('D:\\B\\build_x\\wb_serve.exe', read);
    const g = sm.mapLaunch({ type: 'cppdbg', program: 'D:\\B\\build_x\\wb_serve.exe', sourceFileMap: { 'x': 'y' } }, 'D:\\B', { read, real });
    const l = sm.mapLaunch({ type: 'ht9045-lldb', program: 'D:\\B\\build_x\\wb_serve.exe' }, 'D:\\B', { read, real });
    const same = sm.mapLaunch({ type: 'cppdbg', program: 'D:\\B\\build_x\\wb_serve.exe' }, 'd:/a/tree/', { read, real });
    const none = sm.mapLaunch({ type: 'cppdbg', program: 'D:\\C\\x.exe' }, 'D:\\B', { read, real });
    const got = { root, g: g.cfg.sourceFileMap, l: l.cfg.sourceMap, same: same.from, none: none.from, k: sm.key('D:/A/Tree/', real) };
    ok(root === 'D:/A/Tree' && got.g['D:/A/Tree'] === 'D:\\B' && got.g['D:\\A\\Tree'] === 'D:\\B' && got.g.x === 'y' &&
      JSON.stringify(got.l) === JSON.stringify([['D:/A/Tree', 'D:\\B'], ['D:\\A\\Tree', 'D:\\B'], ['D:/A/Tree', 'd:\\B'], ['D:\\A\\Tree', 'd:\\B']]) && got.same === null && got.none === null && got.k === 'd:\\a\\tree',
      'srcmap (1008): the build\'s source copy (CMakeCache) mapped to the open tree for gdb and LLDB; the same tree in another spelling / no CMakeCache = nothing', got);
    // (1009 review (debug B1): the tree opened through a junction -- the same real folder, another name: mapped)
    const realJ = p => (/^d:\\j(\\|$)/i.test(path.resolve(p)) ? path.resolve(p).replace(/^d:\\j/i, 'D:\\A\\Tree') : p);
    const j = sm.mapLaunch({ type: 'ht9045-lldb', program: 'D:\\B\\build_x\\wb_serve.exe' }, 'D:\\J', { read, real: realJ });
    ok(j.from === 'D:/A/Tree' && j.to === 'D:\\J' && j.cfg.sourceMap.some(x => x[0] === 'D:/A/Tree' && x[1] === 'D:\\J'),
      '1009 review (debug B1): a tree opened through a junction (same real folder, another name) is mapped for LLDB', JSON.stringify({ from: j.from, to: j.to }));
  }
  {
    const lc = require('../lib/liveconfig'), rt = require('../lib/roots');
    const td = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-lh-'));
    fs.mkdirSync(path.join(td, '.vscode'));
    fs.writeFileSync(path.join(td, '.vscode', 'launch.json'), JSON.stringify({ configurations: [
      { name: 'a', environment: [{ name: 'W906_HMI_URL', value: 'http://x/background.html?mode=debug&machine=HT9045' }, { name: 'W906_GENERAL_INI_PATH', value: 'D:\\HT9045\\system\\A.ini' }] },
      { name: 'b', environment: [{ name: 'W906_HMI_URL', value: 'http://x/background.html?machine=HT9050' }, { name: 'W906_GENERAL_INI_PATH', value: 'D:\\HT9045\\system\\B.ini' }] }] }));
    const h = lc.launchHints([td]);
    const w1 = path.join(td, 'w1'), w2 = path.join(td, 'w2');
    for (const w of [w1, w2]) { fs.mkdirSync(path.join(w, 'page'), { recursive: true }); fs.mkdirSync(path.join(w, 'css'), { recursive: true }); fs.writeFileSync(path.join(w, 'page', 'P.html'), '<div class="form"></div>'); }
    const rIn = rt.resolveRoots(path.join(w1, 'page', 'P.html'), [], { webRoot: w1 });
    const rOut = rt.resolveRoots(path.join(w2, 'page', 'P.html'), [], { webRoot: w1 });
    try { fs.rmSync(td, { recursive: true, force: true }); } catch (e) { }
    ok(h.mode === 'debug' && h.byMachine.HT9050.generalIni === 'D:\\HT9045\\system\\B.ini' && h.byMachine.HT9045.generalIni === 'D:\\HT9045\\system\\A.ini' && h.byMachine.HT9045.mode === 'debug' &&
      rIn.webRoot === w1 && rOut.webRoot !== w1,
      '1009 review (live #2 #5 #7): each machine\'s settings from a configuration that opens it, F5\'s ?mode=; the webRoot setting only for a page inside it',
      JSON.stringify({ mode: h.mode, by: h.byMachine, rIn: rIn.webRoot, rOut: rOut.webRoot }));
    const he = require('../lib/htmledit');
    const si = he.srcInfo('<div class="pnl" id="P1"><span class="pnlCap">Cap &amp; X</span></div><div class="pnl" id="P2"></div>' +
      '<span style="position:absolute;left:1px;display:none"><input id="E1" value="3"></span><button id="B1" style="display: none">Go</button>' +
      '<fieldset id="G1"><legend>Grp</legend></fieldset><label id="L1">Text</label>\n#if 0\n<span id="D1">dead</span>\n#endif\n');
    ok(si.caps.P1.k === 'pnlCap' && si.caps.P1.v === 'Cap & X' && si.caps.P2.k !== 'pnlCap' && si.caps.G1.k === 'legend' && si.caps.L1.v === 'Text' &&
      si.caps.E1.k === null && si.hid.join() === 'E1,B1',
      '1009 review (live #6): srcInfo -- each component\'s caption (pnlCap / legend / text) and Visible=False (its own or its wrapper span\'s display:none) from the source',
      JSON.stringify(si));
    const bk2 = require('../lib/bcbkeys');
    const userKb = '[\n  { "key":"f7","command":"-cmake.build" }, // I never want CMake on F7\n' +
      '  { "key": "f8", "command": "workbench.action.debug.stepOver", "when": "inDebugMode && debugState == \'stopped\' && config.ht9045Designer.bcbDebugKeys && !myFlag" },\n' +
      '  { "key": "f10", "command": "x.y", "when": "config.ht9045Designer.bcbDebugKeys", }\n]\n';
    const kw1 = bk2.ensure(userKb), off1 = bk2.remove(kw1);
    const onlyNeg = '[\n' + bk2.KEYS.filter(k => !k.when).map(k => '  { "key": ' + JSON.stringify(k.key) + ', "command": ' + JSON.stringify(k.command) + ' }').join(',\n') + '\n]\n';
    const offNeg = bk2.scan(bk2.remove(onlyNeg)).objs.length;
    ok(/I never want CMake on F7/.test(off1) && /-cmake\.build" \}, \/\/ I never/.test(off1) && /!myFlag/.test(off1) && /"f10"/.test(off1) && bk2.scan(off1).objs.length === 3 && offNeg === 0 &&
      bk2.present(kw1).length === bk2.KEYS.length,
      '1009 review (keys #1 #3): only the exact lines written are ours -- the user\'s own -cmake.build, an entry of ours he edited and one with a trailing comma stay; off takes the CMake removals out too',
      JSON.stringify({ off1, offNeg }));
    {
      const he2 = require('../lib/htmledit'), hb2 = require('../lib/htmlblock');
      const tx = '<!-- was <span id="L1" x> --><span id="L1" title="L1 : TLabel a->b">L1</span><span id="label2">x</span><label for="L1">';
      const re2 = hb2.renameEdits(tx, 'L1', 'L9', { caption: true }) || [];
      const cs = new hb2.CiSet(['Label1']);
      ok(re2.length === 3 && re2[0].range[0] === tx.indexOf('<span id="L1" title') && /title="L9 : TLabel a->b">$/.test(re2[0].repl) && he2.startTagOf(tx, 'Label2') === null && !!he2.startTagOf(tx, 'label2') &&
        cs.has('label1') && hb2.freshName('label1', cs) === 'label1_2',
        '1009 review (edit core #2 #3): rename edits the real start tag (not an id in a comment; a title with "->" whole); ids are case-sensitive, names taken are not',
        JSON.stringify(re2.map(e => e.range)));
    }
    {
      // (EastSun 1009「搜尋我需要功能只搜尋 .cpp .h」: 只找 .cpp／.h)
      const ps2 = require('../lib/projectsearch');
      const dd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-oc-'));
      for (const n of ['a.cpp', 'b.h', 'c.html', 'd.js', 'e.hpp']) fs.writeFileSync(path.join(dd, n), 'KEYX here\n');
      const rts = [{ area: 'port', label: 'p', root: dd, kind: 'port', files: ['a.cpp', 'b.h', 'c.html', 'd.js', 'e.hpp'].map(n => path.join(dd, n)) }];
      const allR = await ps2.search(rts, ps2.makeRe('KEYX').re, {});
      const cppR = await ps2.search(rts, ps2.makeRe('KEYX').re, { onlyCpp: true });
      const fl = r => Array.from(new Set((r.hits || r.results || []).map(h => path.basename(h.file || h.f || '')))).sort().join();
      try { fs.rmSync(dd, { recursive: true, force: true }); } catch (e) { }
      ok(fl(cppR) === 'a.cpp,b.h,e.hpp' && fl(allR).split(',').length === 5, 'EastSun 1009: 只找 .cpp／.h -- the C / C++ sources only', JSON.stringify({ all: fl(allR), cpp: fl(cppR) }));
    }
    {
      const cvR = require('../lib/csvtable'), clR = require('../lib/cmakelists');
      const m1 = cvR.decodeSafe(Buffer.from('a,b\r\n'), 'a,b\r\n', 'utf8', 'D:\\HT9045\\system\\IO_Table.csv');
      const m2 = cvR.decodeSafe(Buffer.from('a,b\r\n'), 'a,b\r\n', 'utf8', 'D:\\HT9045\\HT9045_fromMachine\\x\\t.csv');
      const m3 = cvR.decodeSafe(Buffer.from('a,b\r\n'), 'a,b\r\n', 'utf8', 'D:\\HP9050\\HT9045\\system\\Mot_Table.csv');
      const ts = clR.removeSource('add_library(m STATIC\n  a.cpp\n  b.cpp\n)\ntarget_sources(m PRIVATE c.cpp)\n', 'c.cpp');
      ok(m1.asciiOnly && !m2.asciiOnly && m3.asciiOnly && ts.length === 1 && !ts.empties.length,
        '1009 regression review #6 #7: a machine table = the machine folders only (not a repo with HT9045 in its path); a one-file target_sources is no "only source"',
        JSON.stringify({ m1, m2, m3, empties: ts.empties }));
    }
    {
      const itR = require('../lib/items'), tbR = require('../lib/toolbox');
      const lb = '<div class="lbx" id="L1" style="x"><div>a</div><div>&nbsp;</div><div>b &amp; c</div></div>';
      const lo = itR.itemsOf(lb, 'L1'), le = itR.itemsEdit(lb, 'L1', ['x', '', 'y<z']);
      const pc = tbR.itemOf('TPageControl').html('P9', 1, 2, 0, 0, { sheets: ['TabSheet5', 'TabSheet6'] });
      ok(lo && lo.kind === 'lbx' && lo.items.join('|') === 'a||b & c' && le.repl === '<div>x</div><div>&nbsp;</div><div>y&lt;z</div>' &&
        /title="TabSheet5 : TTabSheet">TabSheet5</.test(pc) && /class="pcPane" data-p="1" title="TabSheet6" style="position:absolute;[^"]*display:none;"/.test(pc) && /class="pcBody" style="position:absolute;/.test(pc),
        '1009 second review (props #1 #2 #4): a ListBox\'s Items (div.lbx rows) read and written; a new PageControl places itself and its sheets get free names',
        JSON.stringify({ lo, le: le.repl }));
    }
    const ae = require('../lib/aliasedit');
    const ae1 = ae.withAlias('X : T NoteAlias=Q｜Alias=A', 'B').title, ae2 = ae.withAlias('Alias=A｜Foo', '').title;
    const ae3 = ae.withAlias('X : T', 'B,C'), ae4 = ae.withAlias('X : T', '#S');
    const ck = ae.checkTableEdit([['Name', 'Alias'], ['a', 'SnA'], ['b', 'SnB'], ['c', '#SEC']], [{ r: 2, c: 1, v: 'SnA' }]);
    const ck2 = ae.checkTableEdit([['Name', 'Alias'], ['a', 'SnA'], ['b', 'SnB']], [{ r: 1, c: 1, v: 'SnC' }, { r: 2, c: 0, v: 'x' }]);
    const ck3 = ae.checkTableEdit([['Name', 'Alias'], ['a', 'SnA']], [{ r: 1, c: 1, v: 'Load Y' }]);
    ok(ae1 === 'X : T NoteAlias=Q｜Alias=B' && ae2 === 'Foo' && ae3.error && ae4.error && ck.problems.length === 1 && !ck2.problems.length && ck2.renamed.join() === 'SnA,SnC' && ck3.problems.length === 1,
      '1009 review (alias #3 #5 #6 #7): Alias= as a word of its own, its "｜" goes with it at the start; no comma / leading #; the IO table\'s Alias column: a duplicate or a blank refused, a rename said',
      JSON.stringify({ ae1, ae2, ck, ck2, ck3 }));
    const mr = require('../lib/machineroot');
    const rcIn = mr.mapPath('D:\\HT9045\\HT9045_fromMachine\\HT9011UC_Cpp_V3.33.906.0\\..\\runcfg\\system\\teach.ini', 'D:\\HP9050');
    const rcOut = mr.mapPath('D:\\HT9050\\htd_work\\runcfg\\SetUp.inf', 'D:\\HP9050');
    ok(rcIn === 'D:\\HP9050\\runcfg\\system\\teach.ini' && rcOut === 'D:\\HP9050\\runcfg\\SetUp.inf' &&
      mr.runsMachine({ program: 'D:/t/build/tests/test_automation.exe' }) && mr.runsMachine({ program: 'D:\\t\\pci1203_linkprobe.exe' }) && mr.runsMachine({ program: 'D:/t/wb_serve.exe' }) && !mr.runsMachine({ program: 'D:/t/foo.exe' }),
      '1009 review (F5 #4 #7): a C++ tree under D:\\HT9045 has its runcfg moved for a 9050 run too; the tests and probes switch the folders like wb_*',
      JSON.stringify({ rcIn, rcOut }));
  }

  // --- projectsearch 1008 (EastSun「搜尋速度可以在快點」「可以先把搜尋到的列上去」): the byte pre-check (plain) gives the same
  //   hits as decoding every file, a Big5 file and a regex included; the hits come file by file before the end -- in the
  //   worker too (sent in batches before its result)
  {
    const ps = require('../lib/projectsearch'), sw = require('../lib/searchworker');
    const tmp = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-ps-'));
    fs.writeFileSync(path.join(tmp, 'a.cpp'), 'int DoInArm();\r\n// doinarm 吸嘴\r\n');
    fs.writeFileSync(path.join(tmp, 'b.h'), 'nothing here\r\n');
    fs.writeFileSync(path.join(tmp, 'c.cpp'), Buffer.concat([Buffer.from('x = '), Buffer.from([0xa7, 0x6c, 0xbc, 0x4c]), Buffer.from(' DoInArm;\r\n')]));   // (Big5 吸嘴 + the keyword)
    const roots = [{ area: 'port', root: tmp }];
    const sig = r => r.hits.map(h => path.basename(h.file) + ':' + h.line + ':' + h.col).join('|');
    const got = {};
    const mk = ps.makeRe('DoInArm', {}), mkW = ps.makeRe('doinarm', { wholeWord: true, caseSensitive: true }), mkR = ps.makeRe('D.InArm', { regex: true });
    const runAll = async m => { const a = await ps.search(roots, m.re, {}); const b = await ps.search(roots, m.re, { plain: true }); return [sig(a), sig(b)]; };
    const p1 = await runAll(mk), p2 = await runAll(mkW), p3 = await runAll(mkR);
    got.same = [p1[0] === p1[1], p2[0] === p2[1], p3[0] === p3[1]].join(',');
    got.hits = p1[0];
    const streamed = [];
    const r1 = await ps.search(roots, mk.re, { plain: true, onHits: b => streamed.push(b.map(h => path.basename(h.file)).join('+')) });
    got.streamed = streamed.sort().join(' ');
    const wb = [];
    let wbBeforeEnd = false;
    const rw = await sw.searchInWorker(roots, mk.re, { plain: true, onHits: b => { wb.push(b.length); } }).then(r => { wbBeforeEnd = wb.length > 0; return r; });
    got.worker = wb.reduce((s, n) => s + n, 0) + '/' + rw.hits.length;
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { }
    ok(got.same === 'true,true,true' && got.hits === 'a.cpp:1:5|a.cpp:2:4|c.cpp:1:8' && got.streamed === 'a.cpp+a.cpp c.cpp' && r1.hits.length === 3 && got.worker === '3/3' && wbBeforeEnd,
      'projectsearch (1008): the byte pre-check = the same hits (plain / whole word / regex, a Big5 file); hits streamed file by file, in the worker too, before the end', JSON.stringify(got));
  }

  // --- defineswitch 1008 review: a #define inside #ifdef X (set by that condition) is not a switch -- listed, its name
  //   matched the top-level switch of the same name and OK with nothing changed turned that one on
  {
    const ds = require('../lib/defineswitch');
    const t = '#ifndef GH\r\n#define GH\r\n//#define DEBUG_HANGUP_NO_HOME\r\n#define SOFT_SIMULTE\r\n#ifdef SOFT_SIMULTE\r\n    #ifndef DEBUG_HANGUP_NO_HOME\r\n        #define DEBUG_HANGUP_NO_HOME\r\n    #endif\r\n#endif\r\n//#define DEBUG_ATC\r\n#endif\r\n';
    const l = ds.list(t);
    const got = { names: l.map(s => (s.line + 1) + ':' + s.name + (s.on ? '=on' : '')).join(' '), same: l.map(s => ds.toggle(t, s, s.on)).filter(Boolean).length };
    let real = null;
    try {
      const mt = fs.readFileSync(path.join(__dirname, '..', '..', '..', 'MachineType.h'), 'latin1');
      const rl = ds.list(mt), names = rl.map(s => s.name);
      real = { n: rl.length, dup: names.filter((n, i) => names.indexOf(n) !== i).length, same: rl.map(s => ds.toggle(mt, s, s.on)).filter(Boolean).length };
    } catch (e) { real = null; }
    ok(got.names === '3:DEBUG_HANGUP_NO_HOME 4:SOFT_SIMULTE=on 10:DEBUG_ATC' && got.same === 0 && (!real || (real.dup === 0 && real.same === 0)),
      'machine switches (1008 review): only the top-level #defines (one inside #ifdef is the condition\'s); names unique; nothing changed = no edit (MachineType.h too)', JSON.stringify({ got, real }));
  }

  // --- cmakelists 1008 review: a name in a # comment is not a source; taking out the only file of a
  //   set_source_files_properties(... PROPERTIES ...) takes the call (an empty file list breaks configure)
  {
    const c = require('../lib/cmakelists');
    const s = 'add_library(x\n  a.cpp\n  b.cpp   # b.cpp old\n)\n# a.cpp was here\nset(N "x#a.cpp")\nset_source_files_properties(a.cpp PROPERTIES COMPILE_FLAGS -O0)\nset_source_files_properties(a.cpp b.cpp PROPERTIES COMPILE_FLAGS -O1)\n';
    const ed = c.removeSource(s, 'a.cpp');
    let o = s; for (const e of ed.slice().reverse()) o = o.slice(0, e.s) + o.slice(e.e);
    const got = { refsB: c.refsOf(s, 'b.cpp').length, refsA: c.refsOf(s, 'a.cpp').length, out: o };
    ok(got.refsB === 2 && got.refsA === 3 && o === 'add_library(x\n  b.cpp   # b.cpp old\n)\n# a.cpp was here\nset(N "x#a.cpp")\nset_source_files_properties(b.cpp PROPERTIES COMPILE_FLAGS -O1)\n',
      'CMake 移出建置 (1008 review): names in # comments are not sources (kept); the only file of set_source_files_properties takes the whole call', JSON.stringify(got));
  }

  // --- cppstub 1008 review: a component renamed with 一起改 C++ -- other files' "fMain->lbA" follow (comments, strings and
  //   a longer name left); the global object from "extern [PACKAGE] TfMain *fMain;"
  {
    const c = require('../lib/cppstub');
    const g = c.globalsOf('class TfMain : public TForm {};\r\nextern PACKAGE TfMain *fMain;\r\n// extern TfMain *fOld;\r\nextern TfOther *fOther;\r\n', 'TfMain');
    const t = 'a = fMain->lbA; // fMain->lbA\ns="fMain->lbA"; fMain -> lbAB; fMain ->  lbA->Caption; fOther->lbA;';
    const u = c.outsideUses(t, g, 'lbA', 'lbB');
    let o = t; for (const x of u.slice().reverse()) o = o.slice(0, x.s) + x.text + o.slice(x.e);
    ok(JSON.stringify(g) === '["fMain"]' && o === 'a = fMain->lbB; // fMain->lbA\ns="fMain->lbA"; fMain -> lbAB; fMain ->  lbB->Caption; fOther->lbA;',
      'rename 一起改 C++ (1008 review): other files\' fMain->lbA follow; comments, strings, a longer name and another form\'s object left', JSON.stringify({ g, o }));
  }

  // --- cmakelists 1008 review (third): a folder renamed -- its include directory follows; # comments left; a bare top
  //   folder name does not match CMake's keywords (Public vs PUBLIC)
  {
    const c = require('../lib/cmakelists');
    const s = 'add_library(x\n  Common/PickPlanner/a.cpp\n)\n# Common/PickPlanner/old.cpp\ntarget_include_directories(x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Common/PickPlanner)\ntarget_link_libraries(x PUBLIC y)\n';
    const r = c.refsUnder(s, 'Common/PickPlanner', '');
    const p = c.refsUnder('target_link_libraries(x PUBLIC y)\nadd_library(z Public/b.cpp)\n', 'Public', '');
    const got = { r: r.map(x => x.path + (x.folder ? '*' : '')).join(','), to: r.map(x => x.folder ? x.of('Common/Planner') : x.of('Common/Planner/' + x.path.slice(x.prefix.length))).join(','), p: p.map(x => x.path).join(',') };
    ok(got.r === 'Common/PickPlanner/a.cpp,${CMAKE_CURRENT_SOURCE_DIR}/Common/PickPlanner*' && got.to === 'Common/Planner/a.cpp,${CMAKE_CURRENT_SOURCE_DIR}/Common/Planner' && got.p === 'Public/b.cpp',
      'CMake folder rename (1008 review): the include directory follows, # comments left, PUBLIC is not the folder Public', JSON.stringify(got));
  }

  // --- cppstub 1008 review (third): a nested struct's "public:" inside the private part does not make the handler after it
  //   public -- the new handler goes after the class's own public one
  {
    const c = require('../lib/cppstub');
    const h = 'class TfX : public TForm\n{\npublic:\n    void Btn1Click(TObject *Sender);\nprivate:\n    struct S {\n    public:\n        int a;\n    };\n    void Btn2Click(TObject *Sender);\n};\n';
    const e = c.declEdit(h, 'TfX', 'Btn3Click', 'TObject *Sender');
    const L = (h.slice(0, e.at) + e.text + h.slice(e.at)).split('\n');
    const i3 = L.findIndex(l => /Btn3Click/.test(l)), iP = L.findIndex(l => /^private:/.test(l));
    ok(i3 > 0 && i3 < iP, 'new handler declaration (1008 review): a nested struct\'s public: in the private part is not the class\'s -- the handler goes into public', JSON.stringify({ i3, iP }));
  }

  // --- items 1008 review (fourth): an option moved / sorted / removed keeps ITS value (and attributes) -- the tag used to
  //   stay at its place and only the words moved (testercomm's s-demoType GPIB=1 / RS232=2 / TTL=0 / TCP=3)
  {
    const it = require('../lib/items');
    const pg = '<select id="s"><option value="1" selected>GPIB</option><option value="2">RS232</option><option value="0" disabled>TTL</option></select>';
    const ap = items => { const r = it.itemsEdit(pg, 's', items); return pg.slice(0, r.range[0]) + r.repl + pg.slice(r.range[1]); };
    const got = { sort: ap(['GPIB', 'RS232', 'TTL'].slice().sort().reverse()), del: ap(['GPIB', 'TTL']), ren: ap(['GPIB', 'RS-232', 'TTL']) };
    ok(got.sort === '<select id="s"><option value="0" disabled>TTL</option><option value="2">RS232</option><option value="1" selected>GPIB</option></select>' &&
      got.del === '<select id="s"><option value="1" selected>GPIB</option><option value="0" disabled>TTL</option></select>' &&
      got.ren === '<select id="s"><option value="1" selected>GPIB</option><option value="2">RS-232</option><option value="0" disabled>TTL</option></select>',
      'Items (1008 review): an option sorted / removed keeps its own value and attributes; one renamed in place keeps the one there', JSON.stringify(got));
  }

  // --- 1008 review (sixth): a radio group renamed -- its own buttons' name="rg_OLD" follow (another group outside left);
  //   its items' count changed -- grid-template-rows follows rows = ceil(items / columns)
  {
    const hb = require('../lib/htmlblock'), it = require('../lib/items');
    const t = '<fieldset class="gbx rg" id="RadioGroup1" title="RadioGroup1 : TRadioGroup"><legend>RadioGroup1</legend><div class="cli" style="display:grid;grid-template-rows:repeat(3,1fr);grid-auto-flow:column;"><label class="rgi"><input type="radio" name="rg_RadioGroup1" checked>A</label><label class="rgi"><input type="radio" name="rg_RadioGroup1">B</label><label class="rgi"><input type="radio" name="rg_RadioGroup1">C</label></div></fieldset><input type="radio" name="rg_RadioGroup1">';
    const e = hb.renameEdits(t, 'RadioGroup1', 'rgMode', { caption: true });
    let o = t; for (const x of e.slice().reverse()) o = o.slice(0, x.range[0]) + x.repl + o.slice(x.range[1]);
    const ap = l => { const r = it.itemsEdit(t, 'RadioGroup1', l); return (t.slice(0, r.range[0]) + r.repl + t.slice(r.range[1])).match(/repeat\((\d+)/)[1]; };
    const got = { inside: (o.match(/name="rg_rgMode"/g) || []).length, outside: (o.match(/name="rg_RadioGroup1"/g) || []).length, four: ap(['A', 'B', 'C', 'D']), two: ap(['A', 'B']), same: ap(['A', 'B', 'X']) };
    ok(got.inside === 3 && got.outside === 1 && got.four === '4' && got.two === '2' && got.same === '3',
      'RadioGroup (1008 review): renamed -- its own buttons\' group name follows, another\'s left; items added / removed -- its rows follow', JSON.stringify(got));
  }

  // --- reverse 1008 review (eighth): the cursor in a handler's last lines is in THAT handler, not the next one 200 chars on
  {
    const rv = require('../lib/reverse');
    const t = 'void __fastcall TfX::spbSaveClick(TObject *Sender)\n{\n    SaveAll();\n    Close();\n}\nvoid __fastcall TfX::spbLoadClick(TObject *Sender)\n{\n    Load();\n}\n';
    const isDef = (text, at) => /^\([^)]*\)\s*\{/.test(text.slice(at, at + 80)) || /^\([^)]*\)\s*\n\s*\{/.test(text.slice(at, at + 80));
    const got = ['SaveAll', 'Close', '}\nvoid', 'Load()'].map(w => { const r = rv.enclosingMethod(t, t.indexOf(w), isDef); return r && r.method; });
    ok(JSON.stringify(got) === '["spbSaveClick","spbSaveClick","spbSaveClick","spbLoadClick"]', 'code -> designer (1008 review): the last lines of a handler belong to it, not to the next one', JSON.stringify(got));
  }

  // --- csvtable 1008 review (CSV module): written as the MACHINE reads it (BCB6 CommaText ends an unquoted item at any char
  //   <= ' '): "Load Y" quoted; a line break in a value becomes a blank; a copied block's empty last cells survive the
  //   clipboard; an empty line is not touched by Insert Column; a quote the machine reads otherwise = quoteTrouble
  {
    const cv = require('../lib/csvtable');
    const t = 'Name,Alias,Lane\r\nCylinder_On,Old,0\r\n';
    const p = cv.parse(t);
    let o = t; for (const x of cv.cellEdits(p, [{ r: 1, c: 1, v: 'Load Y' }]).slice().reverse()) o = o.slice(0, x.s) + x.text + o.slice(x.e);
    const ins = (() => { const s = 'a,b\r\n\r\nc,d\r\n'; let r = s; for (const x of cv.insertCols(cv.parse(s), 0, 1).slice().sort((a, b) => b.s - a.s)) r = r.slice(0, x.s) + x.text + r.slice(x.e); return r; })();
    const got = { o, nl: cv.fieldText('a\r\nb', ',', false), rt: JSON.stringify(cv.fromTsv(cv.toTsv([['5'], [''], ['']]))), ins,
      q1: cv.parse('"x,1\r\nc,d\r\n').quoteTrouble, q2: cv.parse('a,"b,c",d\r\n').quoteTrouble };
    ok(got.o === 'Name,Alias,Lane\r\nCylinder_On,"Load Y",0\r\n' && got.nl === '"a b"' && got.rt === '[["5"],[""],[""]]' && got.ins === ',a,b\r\n\r\n,c,d\r\n' &&
      got.q1 === 'unclosed' && got.q2 === false,
      'CSV (1008 review): quoted as BCB6 CommaText reads it; no line break in a value; empty last cells survive copy / paste; empty lines untouched; unclosed quote = read-only', JSON.stringify(got));
  }

  // --- srcmap 1008 review #6: the build's tree and the open one inside each other = not mapped (a prefix map would move
  //   the inner one's files); two separate trees still mapped
  {
    const sm = require('../lib/srcmap');
    const rd = f => { if (/B.sub.build/i.test(f) || /C.build/i.test(f)) return 'CMAKE_HOME_DIRECTORY:INTERNAL=D:/B'; throw new Error('none'); };
    const id = p => p;
    const got = [sm.mapLaunch({ type: 'cppdbg', program: 'D:/B/sub/build/a.exe' }, 'D:/B/sub', { read: rd, real: id }).from,
      sm.mapLaunch({ type: 'cppdbg', program: 'D:/B/sub/build/a.exe' }, 'D:/', { read: rd, real: id }).from,
      sm.mapLaunch({ type: 'cppdbg', program: 'D:/C/build/a.exe' }, 'D:/C', { read: rd, real: id }).from];
    ok(got[0] === null && got[1] === null && got[2] === 'D:/B', 'srcmap (1008 review): trees inside each other not mapped; separate trees mapped', JSON.stringify(got));
  }

  // --- projectsearch 1008 review #9: a JS regex literal holding // is not a comment (the rest of its line stays code);
  //   a division followed by // is; a .cpp file keeps the C rules
  {
    const ps = require('../lib/projectsearch');
    const t = 'var r = /\\/\\/Loader/; Loader();\nvar q = a / b; // Loader\nif (x) return /[//]Loader/.test(s) && Loader;\n';
    const re = ps.makeRe('Loader', { caseSensitive: true }).re;
    const hs = ps.hitsIn(t, new RegExp(re.source, re.flags), 99);
    const js = ps.dropCommented('a.js', t, hs).map(h => h.line + ':' + h.col).join(' ');
    const all = hs.map(h => h.line + ':' + h.col).join(' ');
    ok(js === '1:14 1:23 3:20 3:39' && all === '1:14 1:23 2:19 3:20 3:39', 'projectsearch 不含註解 (1008 review): a JS regex literal with // is code, a division then // is a comment', JSON.stringify({ js, all }));
  }

  // --- projectsearch 1008 review #2: whole word right after a Big5 character whose second byte is a letter (0xA4 0x42) --
  //   the byte pre-check read "B" + DoInArm as one word and skipped the file
  {
    const ps = require('../lib/projectsearch');
    const tmp = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-ww5-'));
    fs.writeFileSync(path.join(tmp, 'd.cpp'), Buffer.concat([Buffer.from([0xa4, 0x42]), Buffer.from('DoInArm();\r\nxDoInArm;\r\n')]));
    const re = ps.makeRe('DoInArm', { wholeWord: true }).re;
    const full = await ps.search([{ area: 'golden', root: tmp }], re, {});
    const pre = await ps.search([{ area: 'golden', root: tmp }], re, { plain: true });
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { }
    const got = { full: full.hits.map(h => h.line + ':' + h.col).join(' '), pre: pre.hits.map(h => h.line + ':' + h.col).join(' ') };
    ok(got.full === '1:2' && got.pre === got.full,
      'projectsearch whole word (1008 review): a hit right after a Big5 character whose 2nd byte is a letter is found with the byte pre-check too', JSON.stringify(got));
  }

  // --- projectsearch 1008 (EastSun「被註解掉的不要搜尋」): 不含註解 -- // and /* */ comments and #if 0 lines out, a string's
  //   "//" stays code; a .md file is not filtered; through search() with noComment
  {
    const ps = require('../lib/projectsearch');
    const t = 'int a = Loader; // Loader\r\n/* Loader */ x = "Loader // not";\r\n#if 0\r\nLoader();\r\n#endif\r\nLoader2;\r\n';
    const re = ps.makeRe('Loader', {}).re;
    const hs = ps.hitsIn(t, new RegExp(re.source, re.flags), 99);
    const kept = ps.dropCommented('a.cpp', t, hs).map(h => h.line + ':' + h.col).join(' ');
    const md = ps.dropCommented('a.md', t, hs).length;
    const tmp = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-nc-'));
    fs.writeFileSync(path.join(tmp, 'a.cpp'), t);
    const all = await ps.search([{ area: 'port', root: tmp }], re, { plain: true });
    const nc = await ps.search([{ area: 'port', root: tmp }], re, { plain: true, noComment: true });
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { }
    const got = { kept, md, all: all.hits.length, nc: nc.hits.length };
    ok(kept === '1:9 2:19 6:1' && md === 6 && got.all === 6 && got.nc === 3,
      'projectsearch 不含註解 (1008): // /* */ comments and #if 0 lines out, a string kept; .md not filtered; search({ noComment })', JSON.stringify(got));
  }

  // --- htd_hp9050_sync.ps1 1008: given the C++ tree (a folder INSIDE the repository, as the extension passes it) it still
  //   finds machines/HT9050/snapshot -- it said "no machines/HT9050/snapshot" and synced nothing; into a temp root, no fetch
  {
    const tmpR = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-hp9050-'));
    const r = require('child_process').spawnSync('powershell', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', path.join(__dirname, '..', 'media', 'htd_hp9050_sync.ps1'),
      '-Repo', PORT, '-Root', tmpR, '-NoFetch', '-NoProcessCheck'], { encoding: 'utf8', timeout: 180000, windowsHide: true });
    const snap = (() => { try { return fs.readFileSync(path.join(tmpR, '.htd_snapshot'), 'utf8').split(/\r?\n/)[0]; } catch (e) { return ''; } })();
    const gen = fs.existsSync(path.join(tmpR, 'HT9045', 'system'));
    try { fs.rmSync(tmpR, { recursive: true, force: true }); } catch (e) { }
    ok(r.status === 0 && /^[0-9a-f]{40}$/.test(snap) && gen, 'htd_hp9050_sync (1008): given the C++ tree inside the repository, the snapshot is found and written (its commit recorded)',
      JSON.stringify({ code: r.status, snap, gen, out: String(r.stdout || '').split(/\r?\n/).filter(Boolean).slice(-2) }));
  }

  // --- classexp 1008 (gap list #17, ClassExplorer): classes (the brace on the next line too, a forward declaration not),
  //   their members -- functions / fields, access, a field with "= new X()" a field, a struct public; the real tree's TfHotPlate
  {
    const ce = require('../lib/classexp');
    const t = 'class TFoo;\r\nclass PACKAGE TfDemo : public TForm\r\n{\r\n__published:\r\n    TSpeedButton *spbSave, *spbExit;\r\n    void __fastcall spbSaveClick(TObject *Sender);\r\nprivate:\r\n    int n[4];\r\n    TEdit *e = new TEdit();\r\n    bool Ok() const { return n[0] > 0; }\r\npublic:\r\n    virtual ~TfDemo();\r\n};\r\nstruct S\r\n{ int a; bool g(int x = 3); };\r\n';
    const cl = ce.classesIn(t).map(c => c.name + '@' + c.line + (c.base ? ':' + c.base : '')).join(' ');
    const md = ce.members(t, 'TfDemo').map(m => m.kind[0] + ':' + m.name + '/' + m.access).join(' ');
    const ms = ce.members(t, 'S').map(m => m.kind[0] + ':' + m.name + '/' + m.access).join(' ');
    const all = ce.scanClasses(PORT);
    const hp = all.find(c => c.name === 'TfHotPlate');
    const hpm = hp ? ce.members(fs.readFileSync(hp.file, 'utf8'), 'TfHotPlate') : [];
    const got = { cl, md, ms, n: all.length, hp: hp && path.basename(hp.file), hpm: hpm.length, xst: hpm.some(m => m.name === 'XST1' && m.kind === 'field') };
    ok(cl === 'TfDemo@1:TForm S@13' && md === 'f:spbSave/__published f:spbExit/__published m:spbSaveClick/__published f:n/private f:e/private m:Ok/private m:~TfDemo/public' &&
      ms === 'f:a/public m:g/public' && got.n > 300 && got.hp === 'fHotPlate.h' && got.hpm > 10 && got.xst,
      'classexp (1008, ClassExplorer): classes and their functions / fields (access, initialised fields, struct public); the tree\'s classes, TfHotPlate in forms\\fHotPlate.h', JSON.stringify(got));
  }

  // --- todo 1008 (gap list #18): TODO / FIXME / XXX at the start of a comment (// and /* */, a " * " line of a block),
  //   the owner of "TODO(x):" split off; one in a string or in the middle of a comment's text is not one; the add line
  {
    const td = require('../lib/todo');
    const r = td.scan('int a; // TODO: fix this\r\nputs("// TODO not");\r\n/* FIXME later */ x;\r\n  //XXX-why\r\n// TODOS no\r\n/*\r\n * TODO(es02): two\r\n */\r\n// see the TODO above\r\n');
    const got = { r: r.map(x => x.line + ':' + x.col + ':' + x.kind + ':' + x.owner + ':' + x.text).join('|'), add: td.addLine('\t  x = 1;'), add2: td.addLine('y;', 'ES02') };
    ok(got.r === '0:10:TODO::fix this|2:3:FIXME::later|3:4:XXX::why|6:3:TODO:es02:two' && got.add === '\t  // TODO: ' && got.add2 === '// TODO(ES02): ',
      'todo (1008): TODO / FIXME / XXX at a comment\'s start (// /* and " * "), owner split; not in a string, not mid-text, not TODOS; the add line keeps the indent', got);
  }

  // --- crashhint.exitHint 1008 (EastSun「我軟體又開不了了」): exit codes in words -- the endpoint security's kill, heap
  //   corruption, a missing DLL, a negative int as Windows hands it over; 0 = nothing to say
  {
    const ch = require('../lib/crashhint');
    const got = [0xE0000027, -536870873, 0xC0000374, -1073740940, 0xC0000135, 0, 2, 0xC0001234].map(c => { const h = ch.exitHint(c); return h.hex + ':' + (h.title || '-'); });
    ok(/資安/.test(got[0]) && got[1] === got[0] && /heap/.test(got[2]) && got[3] === got[2] && /DLL/.test(got[4]) && got[5] === '0x00000000:-' && /exit 2/.test(got[6]) && /0xC0001234/.test(got[7]),
      'exitHint (1008): 0xE0000027 = the endpoint security, 0xC0000374 = heap, 0xC0000135 = a DLL, negative ints read the same, 0 = normal', got);
  }

  // --- builderrors 1008 (full test, audit E2): CMake's own errors (real CMake 4 output, a deleted source) -> problems at their
  //   CMakeLists line, the indented lines joined; "CMake Error:" with no place said too; the compiler's errors still read
  {
    const be = require('../lib/builderrors');
    const txt = ['-- Configuring done', 'CMake Error at CMakeLists.txt:3 (add_library):', '  Cannot find source file:', '', '    missing_on_purpose.cpp', '',
      'CMake Error at CMakeLists.txt:3 (add_library):', '  No SOURCES given to target: a', '', 'CMake Error: Could not create named generator X', 'CMake Generate step failed.',
      'D:/t/a.cpp:5:3: error: expected \';\' before \'}\' token'].join('\r\n');
    const ps = be.parse(txt, 'D:\\t\\build', 'D:\\t\\tree');
    const got = ps.map(p => (p.file ? path.basename(path.dirname(p.file)) + '/' + path.basename(p.file) : '-') + ':' + p.line + ' ' + p.severity + ' ' + p.msg.slice(0, 50));
    ok(ps.length === 4 && ps[0].file === path.resolve('D:\\t\\tree', 'CMakeLists.txt') && ps[0].line === 3 && /Cannot find source file: missing_on_purpose\.cpp/.test(ps[0].msg) &&
      /No SOURCES/.test(ps[1].msg) && ps[2].noPlace && /named generator/.test(ps[2].msg) && /a\.cpp$/.test(ps[3].file) && ps[3].line === 5,
      'builderrors (1008 full test): CMake errors at their CMakeLists line (indented lines joined), a placeless CMake Error, the compiler\'s after them', got);
  }

  // --- cmakelists 1008 (full test, audit F1): a folder whose last source shares its line with the call's ")" -- the tree's
  //   own JsonBridge/actions and JsonBridge -- the new source goes inside the call (in memory; nothing written)
  {
    const cl = require('../lib/cmakelists');
    const cm = fs.readFileSync(path.join(PORT, 'CMakeLists.txt'), 'utf8');
    const got = {};
    for (const dir of ['JsonBridge/actions', 'JsonBridge', 'Public']) {
      const tg = cl.targetsFor(cm, dir)[0];
      if (!tg) { got[dir] = 'no target'; continue; }
      const ins = cl.insertSource(cm, tg, dir + '/HtdNew.cpp', 'test');
      const nt = cm.slice(0, ins.at) + ins.text + cm.slice(ins.at);
      const call = cl.targets(nt).find(x => x.name === tg.name);
      const body = call ? nt.slice(call.start, call.end) : '';
      got[dir] = (call && body.indexOf(dir + '/HtdNew.cpp') >= 0 ? 'inside' : 'OUTSIDE') + (tg.before ? '/before' : '/after');
    }
    ok(got['JsonBridge/actions'] === 'inside/before' && /^inside/.test(got.JsonBridge) && /^inside/.test(got.Public),
      'cmakelists (1008 full test): a new .cpp lands inside its target call, also where the last source shares its line with ")"', got);
  }

  // --- cmakelists.removeSource 1008 (gap list #16): Remove from Project on the tree's real CMakeLists.txt (in memory) -- the
  //   source gone from every target, the calls still parse; a source sharing a line with ")" leaves the ")"
  {
    const cl = require('../lib/cmakelists');
    const cm = fs.readFileSync(path.join(PORT, 'CMakeLists.txt'), 'utf8');
    const apply = (t, es) => { let s = t; for (const e of es.slice().reverse()) s = s.slice(0, e.s) + s.slice(e.e); return s; };
    const r1 = apply(cm, cl.removeSource(cm, 'Public/cBootLog.cpp'));
    const r2 = apply(cm, cl.removeSource(cm, 'JsonBridge/actions/MainStateRecord.cpp'));
    const n0 = cl.targets(cm).length;
    const tiny = 'add_library(a\n  x/a.cpp\n  x/b.cpp)  # end\n';
    const r3 = apply(tiny, cl.removeSource(tiny, 'x/b.cpp'));
    const got = { b1: cl.refsOf(r1, 'Public/cBootLog.cpp').length, t1: cl.targets(r1).length === n0, b2: cl.refsOf(r2, 'JsonBridge/actions/MainStateRecord.cpp').length, t2: cl.targets(r2).length === n0, r3 };
    ok(cl.refsOf(cm, 'Public/cBootLog.cpp').length > 0 && got.b1 === 0 && got.t1 && got.b2 === 0 && got.t2 && r3 === 'add_library(a\n  x/a.cpp\n  )  # end\n',
      'cmakelists.removeSource (1008): a source out of the real CMakeLists.txt (every target still parses), a ")"-sharing one leaves the ")"', got);
  }

  // --- cmakelists spellings 1008 (full test, audit F2): the tree's own tests\CMakeLists.txt names sources as "../Public/x.cpp"
  //   and "${CMAKE_SOURCE_DIR}/vclcompat/x.cpp" -- both found, a new name written the same way, a folder's sources found
  {
    const cl = require('../lib/cmakelists');
    const t = fs.readFileSync(path.join(PORT, 'tests', 'CMakeLists.txt'), 'utf8');
    const a = cl.refsAll(t, 'Public/cBootLog.cpp', 'tests'), b = cl.refsAll(t, 'vclcompat/TrayCore.cpp', 'tests');
    const u = cl.refsUnder(t, 'JsonBridge', 'tests');
    const got = { a: a.length, aNew: a[0] && a[0].of('Public/Boot2.cpp'), b: b.length, bNew: b[0] && b[0].of('vclcompat/T2.cpp'), u: u.length, uNew: u[0] && u[0].of('JB2/' + u[0].path.slice(u[0].prefix.length)),
      root: cl.spellings('Public/x.cpp', '').map(s => s.spell).join('|') };
    ok(got.a >= 1 && got.aNew === '../Public/Boot2.cpp' && got.b >= 1 && got.bNew === '${CMAKE_SOURCE_DIR}/vclcompat/T2.cpp' && got.u > 10 && /^(\.\.\/|\$\{CMAKE_SOURCE_DIR\}\/)JB2\//.test(got.uNew) && /Public\/x\.cpp\|/.test(got.root + '|'),
      'cmakelists spellings (1008 full test): tests\\CMakeLists.txt\'s "../" and "${CMAKE_SOURCE_DIR}/" forms found and rewritten the same way; a folder\'s sources', got);
  }

  // --- bcbkeys 1008 (full test, audit B1-B4): the keybindings.json edge cases -- the file formatted (one object over several
  //   lines), an entry VS Code's UI added after ours, a quote in a comment, a ] in a string, comments only, a BOM: always
  //   valid JSONC, the user's entries and comments kept, ours exactly once, removed whole
  {
    const bk = require('../lib/bcbkeys');
    const jsonc = t => { const sc = bk.scan(t); if (!sc) throw new Error('no array'); return sc.objs.map(o => JSON.parse(o.text)); };
    const valid = t => { try { const a = jsonc(t); return a.every(o => o && typeof o.key === 'string'); } catch (e) { return false; } };
    const user = '[\n  { "key": "ctrl+k", "command": "x", "args": "a]b // c" } // press "x\n]\n';
    const one = bk.ensure(user);
    // VS Code's 格式化文件: each object over several lines
    const formatted = one.replace(/\{ ("key")/g, '{\n    $1').replace(/" \}/g, '"\n  }');
    const f2 = bk.ensure(formatted);
    // the UI appends an entry after ours
    const appended = one.replace(/\n\]\n$/, ',\n  { "key": "ctrl+u", "command": "mine.ui" }\n]\n');
    const offA = bk.remove(appended);
    const comments = '// old ones\n// [ { "key": "f1" } ]\n';
    const c1 = bk.ensure(comments);
    const bom = bk.ensure('\uFEFF[]');
    const got = {
      one: valid(one) && jsonc(one).length === bk.KEYS.length + 1 && /\/\/ press "x/.test(one),
      formattedAgain: f2 === null || (valid(f2) && jsonc(f2).length === bk.KEYS.length + 1),
      offFormatted: (() => { const r = bk.remove(formatted); return valid(r) && jsonc(r).length === 1 && jsonc(r)[0].key === 'ctrl+k'; })(),
      appendedKept: valid(offA) && jsonc(offA).map(o => o.key).join(',') === 'ctrl+k,ctrl+u',
      appendedEnsure: (() => { const r = bk.ensure(appended); return r === null || (valid(r) && jsonc(r).filter(o => o.command === 'mine.ui').length === 1); })(),
      commentsKept: /\/\/ old ones/.test(c1) && /\/\/ \[ \{ "key": "f1" \} \]/.test(c1) && valid(c1) && jsonc(c1).length === bk.KEYS.length,
      bom: valid(bom) && jsonc(bom).length === bk.KEYS.length,
      idem: bk.ensure(one) === null,
    };
    ok(Object.values(got).every(Boolean),
      'bcbkeys (1008 full test): formatted file, a UI entry after ours, a quote in a comment, ] in a string, comments only, BOM -- valid JSONC, the user\'s kept, ours once, removed whole', got);
  }

  // --- machineroot 1008 (EastSun「9050 都改到 D:\HP9050」): a 9050 launch's path env moved under the root, laid out like the
  //   machine; other values and other launches untouched
  {
    const mr = require('../lib/machineroot');
    const env = { W906_GENERAL_INI_PATH: 'D:\\HT9045\\system\\Gerneral.ini', W906_SETUPINF_PATH: 'd:\\HT9050\\htd_work\\HT9011UC_Cpp_V3.33.906.0\\..\\runcfg\\SetUp.inf',
      W906_EVENTLOG_ROOT: 'D:\\HT9045_Log\\EventLogTxt', W906_X: 'd:/GPIB9045/system/general.ini', W906_HMI_URL: 'http://127.0.0.1:8055/background.html?mode=debug&machine=HT9050',
      W906_F5_OPERATOR_CLOSE: '1', OTHER: 'D:\\HT9045\\x' };
    const r = mr.remapEnv(env, 'D:\\HP9050\\');
    const got = { g: r.env.W906_GENERAL_INI_PATH, s: r.env.W906_SETUPINF_PATH, e: r.env.W906_EVENTLOG_ROOT, x: r.env.W906_X, u: r.env.W906_HMI_URL, o: r.env.OTHER, n: r.changed.length, is: mr.is9050(env), not: mr.is9050({ W906_HMI_URL: 'http://x/?machine=HT9045' }) };
    ok(got.g === 'D:\\HP9050\\HT9045\\system\\Gerneral.ini' && got.s === 'D:\\HP9050\\runcfg\\SetUp.inf' && got.e === 'D:\\HP9050\\HT9045_Log\\EventLogTxt' &&
      got.x === 'D:\\HP9050\\GPIB9045\\system\\general.ini' && got.u === env.W906_HMI_URL && got.o === 'D:\\HT9045\\x' && got.n === 4 && got.is && !got.not,
      'machineroot (1008): D:\\HT9045 / HT9045_Log / GPIB9045 / ..\\runcfg under the root; URLs, switches, non-W906 names untouched; only machine=HT9050 launches', got);
  }

  // --- crashhint 1008 (EastSun「你不能自己偵測喔?」): the ES02 crash -- WbMutex::lock / EnterCriticalSection writing 0x14 = a
  //   lock whose memory is zeros; before main / at exit / running read from the frames; the focus = the program's own frame
  {
    const ch = require('../lib/crashhint');
    const F = (name, p, line) => ({ name, path: p, line });
    const csStack = [F('ntdll!RtlEnterCriticalSection', null, 0), F('webbridge::WbMutex::lock()', 'D:/t/WebBridge/Sync.h', 60),
      F('webbridge::WbGuard::WbGuard(webbridge::WbMutex&)', 'D:/t/WebBridge/Sync.h', 75), F('Foo::Bar()', 'D:/t/Foo.cpp', 12),
      F('_GLOBAL__sub_I_Foo', 'D:/t/Foo.cpp', 99), F('__do_global_ctors', null, 0), F('__main', null, 0)];
    const h1 = ch.hintOf('Exception 0xc0000005 encountered at address 0x77a0abc6: Access violation writing location 0x00000014', csStack);
    const h2 = ch.hintOf('Access violation reading location 0x00000008', [F('X::Y()', 'D:/t/X.cpp', 5), F('main', 'D:/t/m.cpp', 3)]);
    const h3 = ch.hintOf('Access violation writing location 0x00000014', [F('ntdll!RtlEnterCriticalSection', null, 0), F('webbridge::WbMutex::lock()', 'D:/t/WebBridge/Sync.h', 60), F('Z::z()', 'D:/t/Z.cpp', 7), F('__tcf_0', null, 0), F('exit', null, 0)]);
    // (the ES02 case itself: the ELA worker's stack looks like "running", main is in exit on another thread)
    const h4 = ch.hintOf('Access violation writing location 0x00000014', [F('webbridge::WbMutex::lock()', 'D:/t/WebBridge/Sync.h', 60), F('simnet::Mask::ElaValue()', 'D:/t/SimNet/SimNetMask.cpp', 476), F('ela::Hub::Entry(void*)', 'D:/t/EventLogAnalysis/ElaHub.cpp', 646)],
      [[F('__tcf_3', null, 0), F('exit', null, 0), F('main', 'D:/t/tools/wb_serve.cpp', 4185)]]);
    // (EastSun「怎閃退了?」: no recipe -> which file / recipe)
    const files = { 'D:\\HT9045\\SetUp.inf': 'R1\r\n', 'D:\\x\\SetUp.inf': 'NOPE\n', 'D:\\HT9045\\IniData\\Data\\R1': '' };
    const io = { exists: p => Object.prototype.hasOwnProperty.call(files, p), read: p => files[p] };
    const r1 = ch.recipeHint({ W906_SETUPINF_PATH: 'D:\\y\\SetUp.inf' }, io), r2 = ch.recipeHint({ W906_SETUPINF_PATH: 'D:\\x\\SetUp.inf' }, io), r3 = ch.recipeHint({}, io);
    const recOk = /找不到 SetUp\.inf/.test(r1.title) && r1.fixFrom === 'D:\\HT9045\\SetUp.inf' && /NOPE/.test(r2.title) && r3.recipe === 'R1' && !r3.fixFrom;
    const got = { recOk, t1: h1.title, p1: h1.phase, f1: h1.focus && h1.focus.name, t2: h2.title, p2: h2.phase, p3: h3.phase, f3: h3.focus && h3.focus.name, p4: h4.phase };
    ok(/鎖/.test(h1.title) && h1.phase === 'init' && got.f1 === 'Foo::Bar()' && /Foo\.cpp:12/.test(h1.text) && /#3 Foo::Bar\(\)/.test(h1.text) &&
      /NULL/.test(h2.title) && h2.phase === 'run' && h3.phase === 'exit' && got.f3 === 'Z::z()' && recOk && h4.phase === 'exit' && /背景執行緒/.test(h4.why) && h4.focus.name === 'simnet::Mask::ElaValue()',
      'crashhint (1008): an all-zero lock (EnterCriticalSection writing 0x14) named, before main / at exit / running, the first frame of the program\'s own code', got);
  }

  // --- icons (AI 20261001, 0.148): the 方案總管 the extension draws uses VS Code's own icons -- a file's Seti icon by its
  // name / extension / language (light first in a light theme), the default when nothing fits; codicons from VS Code's table
  {
    const ic = require('../lib/icons');
    const st = { theme: {
      iconDefinitions: { _c: { fontCharacter: '\\E01A', fontColor: '#519aba' }, _h: { fontCharacter: '\\E00C', fontColor: '#a074c4' },
        _pkg: { fontCharacter: '\\E055', fontColor: '#cbcb41' }, _def: { fontCharacter: '\\E023', fontColor: '#d4d7d6' }, _hl: { fontCharacter: '\\E00C', fontColor: '#000000' } },
      file: '_def', fileExtensions: { h: '_h' }, fileNames: { 'package.json': '_pkg' }, languageIds: { cpp: '_c' },
      light: { fileExtensions: { h: '_hl' } },
    } };
    const g = n => { const r = ic.fileIcon(st, n, false); return r ? r.char.toString(16) + r.color : '-'; };
    const got = { cpp: g('fHotPlate.cpp'), h: g('a.H'), pkg: g('Package.json'), dfm: g('x.dfm'), none: g('Makefile'), hl: ic.fileIcon(st, 'a.h', true).color,
      noTheme: ic.fileIcon(null, 'a.cpp'), cm: ic.codicons('Z:\\no-vscode-here')['symbol-folder'] };
    ok(got.cpp === 'e01a#519aba' && got.h === 'e00c#a074c4' && got.pkg === 'e055#cbcb41' && got.dfm === 'e023#d4d7d6' && got.none === 'e023#d4d7d6' && got.hl === '#000000' &&
      got.noTheme === null && got.cm === 60035,
      'icons: Seti by file name, extension, language (.cpp), the default; the light theme first in light; no theme = none; codicons fall back to the built-in table',
      JSON.stringify(got));
  }

  // --- containerTagOf (AI 20261001, 0.135): where a new / pasted component goes -- a tab sheet by its .dfm name, the body
  // of a page without a form root (main.html)
  {
    const heC = require('../lib/htmledit');
    const webP = path.join(PORT, '..', 'web', 'page');
    const rd = n => { try { return fs.readFileSync(path.join(webP, n), 'utf8'); } catch (e) { return ''; } };
    const io = rd('HW.IoSetView.html'), mn = rd('main.html'), hp = rd('Setup.HotPlate.html');
    const cas = io ? heC.containerTagOf(io, 'tsStack1_Cassette') : null;
    const byTab = heC.containerTagOf('<div class="pcWrap" id="pg"><div class="tabs"><div class="tab" data-t="0" title="tsA : TTabSheet">A</div>' +
      '<div class="tab" data-t="1" title="tsB : TTabSheet">B</div></div><div class="pcBody"><div class="pcPane" data-p="0"></div><div class="pcPane" data-p="1"></div></div></div>', 'tsB');
    const body = mn ? heC.containerTagOf(mn, '@form') : null;
    const form = hp ? heC.containerTagOf(hp, '@form') : null;
    const got = { cas: cas && cas.kind + ' ' + /data-p="(\d+)"/.exec(cas.text)[1] + ' ' + /title="(\w+)"/.exec(cas.text)[1], byTab: byTab && byTab.kind + ' ' + byTab.text,
      body: body && body.kind + ' ' + body.name, form: form && form.kind + ' ' + /class="(\w+)"/.exec(form.text)[1], none: heC.containerTagOf(io || 'x', 'tsNoSuchSheet'), id: io ? heC.containerTagOf(io, 'pnlStack1').kind : '' };
    ok(got.cas === 'pane 2 tsStack1_Cassette' && got.byTab === 'pane <div class="pcPane" data-p="1">' && got.body === 'body body' && got.form === 'form form' && got.none === null && got.id === 'id',
      'containerTagOf: a tab sheet by its .dfm name (its pane\'s title, or the tab "tsB : TTabSheet" -> data-p of the same number); \'@form\' on main.html (no form root) = <body>; an ordinary id as before',
      JSON.stringify(got));
  }

  // --- Alias: the part of the title the page's JS reads (title.match(/Alias=([^｜|\s]+)/)), the IO table's names
  const al = require('../lib/aliasedit');
  const alT = 'BtnPanelLane3 : TBtnPanelLane｜Alias=C_LoaderEdgePush';
  const alIo = 'D:\\HT9045\\system\\IO_Table.csv';
  const alNames = fs.existsSync(alIo) ? al.ioAliases(alIo) : null;
  ok(al.aliasOf(alT) === 'C_LoaderEdgePush' && al.withAlias(alT, 'C_Other').title === 'BtnPanelLane3 : TBtnPanelLane｜Alias=C_Other' &&
    al.withAlias(alT, '').title === 'BtnPanelLane3 : TBtnPanelLane' && al.withAlias('X : TButton', 'Y1').title === 'X : TButton｜Alias=Y1' &&
    // (AI 20261001: any machine's IO table -- not one machine's name: C_OTD_Valve_On is not on every 9045 / 9050)
    !!al.withAlias(alT, 'a b').error && (alNames === null || (alNames.length > 100 && alNames.every(n => /^[A-Za-z_][\w.]*$/.test(n)) && new Set(alNames).size === alNames.length)),
    'aliasedit: read / change / take out / add the Alias of a title; the IO table\'s Alias column (read only)', alNames ? alNames.length + ' IO names' : 'no IO table');
  // AI(W906-HTDESIGNER) 20261001: the IO table's "#..." section marker row (HT9050's ",#NEW_FROM_9050_DRAWING_20260923,")
  // is not an IO -- not offered as an Alias; a name twice is listed once
  const alDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-alias-'));
  const alTmp = path.join(alDir, 'IO_Table.csv');
  fs.writeFileSync(alTmp, 'IOName,Alias,Port\r\nX0,C_A,1\r\n,#NEW_FROM_DRAWING,\r\nX1,C_B,2\r\nX2,C_A,3\r\n');
  const alMk = al.ioAliases(alTmp);
  fs.rmSync(alDir, { recursive: true, force: true });
  ok(JSON.stringify(alMk) === '["C_A","C_B"]', 'aliasedit: the IO table\'s "#..." marker row is not offered as an Alias; each name once', JSON.stringify(alMk));
  // AI(W906-HTDESIGNER) 20261002 (machine): agents' git worktrees in <tree>\.claude\worktrees\ (whole copies of the tree)
  // are not part of the project -- not indexed, not searched, not in the Solution Explorer
  {
    const wtDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-wt-'));
    fs.mkdirSync(path.join(wtDir, 'src'), { recursive: true });
    fs.mkdirSync(path.join(wtDir, '.claude', 'worktrees', 'agent-x', 'src'), { recursive: true });
    fs.writeFileSync(path.join(wtDir, 'src', 'a.cpp'), 'void A() {}\n');
    fs.writeFileSync(path.join(wtDir, '.claude', 'worktrees', 'agent-x', 'src', 'a.cpp'), 'void A() {}\n');
    const rel = l => l.map(f => path.relative(wtDir, f).split(path.sep).join('/')).sort();
    const wtIdx = rel(await require('../lib/cppindex').walk(wtDir));
    const wtSrch = rel(await require('../lib/projectsearch').listFiles(wtDir));
    const wtSol = rel(await require('../lib/solutiontree').allFiles(wtDir, 100));
    const wtTop = (await require('../lib/solutiontree').entries(wtDir));
    fs.rmSync(wtDir, { recursive: true, force: true });
    ok(JSON.stringify(wtIdx) === '["src/a.cpp"]' && JSON.stringify(wtSrch) === '["src/a.cpp"]' && JSON.stringify(wtSol) === '["src/a.cpp"]' &&
      !JSON.stringify(wtTop).includes('.claude'),
      '.claude\\worktrees (agents\' copies of the tree) is skipped by the C++ index, the project search and the Solution Explorer', JSON.stringify({ idx: wtIdx, search: wtSrch, sol: wtSol }));
  }
  // AI(W906-HTDESIGNER) 20261002 (machine): F5's build status (tools\build_with_status.ps1 -> boot_build.js) -> the bar
  {
    const bw = require('../lib/buildwatch');
    const now = 1000000;
    const st = bw.parse('window.__HT_BUILD={"state":"building","pct":45,"file":"cmydef.cpp","n":120,"errors":0,"lastError":"","target":"wb_serve","dir":"build_x","elapsed":125,"t":' + (now - 1000) + '};');
    const old = Object.assign({}, st, { t: now - 60000 });
    ok(!!st && st.pct === 45 && bw.parse('garbage') === null && bw.parse('') === null &&
      bw.decide(st, false, now) === 'open' && bw.decide(old, false, now) === 'none' && bw.decide(st, true, now) === 'update' &&
      bw.decide(Object.assign({}, st, { state: 'done' }), true, now) === 'close-done' && bw.decide(Object.assign({}, st, { state: 'failed' }), true, now) === 'close-failed' &&
      bw.decide(old, true, now) === 'close-stale' && bw.decide(null, true, now) === 'close-stale' && bw.decide(Object.assign({}, st, { state: 'done' }), false, now) === 'none' &&
      bw.message(st) === '45%　cmydef.cpp　（120 個檔，2 分 05 秒）' && /錯誤 3/.test(bw.message(Object.assign({}, st, { errors: 3 }))) &&
      /JSON[\\/]runtime[\\/]boot_build\.js$/.test(bw.statusFile('W')) && bw.statusFile(null) === null,
      'buildwatch: F5\'s build status file -> a bar opens only for a fresh "building", updates, closes on done / failed / a writer gone silent; its text',
      bw.message(st));
    // CMake's own progress folder (what make's "[ 45%]" is counted from): 9 of 20 steps = 45%; a folder without it ignored
    const cmRoot = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-cmp-'));
    const cmProg = path.join(cmRoot, 'build_a', 'CMakeFiles', 'Progress');
    fs.mkdirSync(cmProg, { recursive: true });
    fs.mkdirSync(path.join(cmRoot, 'build_b', 'CMakeFiles'), { recursive: true });
    fs.mkdirSync(path.join(cmRoot, 'src', 'CMakeFiles', 'Progress'), { recursive: true });
    fs.writeFileSync(path.join(cmRoot, 'src', 'CMakeFiles', 'Progress', 'count.txt'), '5\n');
    fs.writeFileSync(path.join(cmProg, 'count.txt'), '20\n');
    for (let i = 1; i <= 9; i++) fs.writeFileSync(path.join(cmProg, String(i)), '');
    const cmOne = bw.cmakeProgress(path.join(cmRoot, 'build_a'));
    const cmRun = bw.runningCMake(cmRoot, 0), cmLater = bw.runningCMake(cmRoot, Date.now() + 60000);
    fs.rmSync(cmRoot, { recursive: true, force: true });
    ok(!!cmOne && cmOne.pct === 45 && cmOne.steps === 9 && cmOne.total === 20 && cmRun.length === 1 && path.basename(cmRun[0].dir) === 'build_a' && cmLater.length === 0 &&
      bw.cmakeProgress(path.join(cmRoot, 'nope')) === null,
      'buildwatch: CMake\'s CMakeFiles\\Progress (count.txt + one file per step) -> 9 / 20 = 45%; only build* folders; an older one than the task ignored',
      JSON.stringify({ one: cmOne && cmOne.pct, run: cmRun.length, later: cmLater.length }));
  }

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
  // 1006 csvtable.sortRows (Excel's Sort): numbers by value first, then text, the empty last; desc reverses all but the
  // empty; the rows before `from` stay; a quoted cell moves with its row as it is; in order = no edit
  {
    const ct = require('../lib/csvtable');
    const t = 'Name,N\r\nb,10\r\na,2\r\nc,\r\nd,"3"\r\ne,x\r\n';
    const p = ct.parseDoc(t);
    const run = (col, desc) => { const e = ct.sortRows(p, t, 1, col, desc); return e.length ? t.slice(0, e[0].s) + e[0].text + t.slice(e[0].e) : t; };
    const up = run(1, false), dn = run(1, true), byName = run(0, false);
    const same = ct.sortRows(ct.parseDoc(byName), byName, 1, 0, false);
    ok(up === 'Name,N\r\na,2\r\nd,"3"\r\nb,10\r\ne,x\r\nc,\r\n' && dn === 'Name,N\r\ne,x\r\nb,10\r\nd,"3"\r\na,2\r\nc,\r\n' && byName.indexOf('Name,N\r\na,2\r\nb,10') === 0 && same.length === 0,
      'csvtable.sortRows (Excel Sort): numbers by value, then text, the empty last; desc reverses (the empty still last); the title row stays; in order = no edit',
      JSON.stringify({ up, dn }));
  }
  // 1006 csvtable.insertCols / deleteCols (Excel's Insert / Delete sheet columns): every row, a quoted field with the
  // delimiter in it kept whole, a short row filled / left
  {
    const ct = require('../lib/csvtable');
    const t = 'A,B,C\r\n1,"x,y",3\r\n4\r\n7,8,9\r\n';
    const p = ct.parseDoc(t);
    const ap = es => { let s = t; es.slice().sort((a, b) => b.s - a.s).forEach(e => { s = s.slice(0, e.s) + e.text + s.slice(e.e); }); return s; };
    const ins = ap(ct.insertCols(p, 1, 2)), d1 = ap(ct.deleteCols(p, 1, 1)), dEnd = ap(ct.deleteCols(p, 1, 2)), d0 = ap(ct.deleteCols(p, 0, 0));
    ok(ins === 'A,,,B,C\r\n1,,,"x,y",3\r\n4,,\r\n7,,,8,9\r\n' && d1 === 'A,C\r\n1,3\r\n4\r\n7,9\r\n' && dEnd === 'A\r\n1\r\n4\r\n7\r\n' && d0 === 'B,C\r\n"x,y",3\r\n\r\n8,9\r\n',
      'csvtable insert / delete columns (Excel): every row, a quoted "x,y" kept whole, the delimiters right at the ends',
      JSON.stringify({ ins, d1, dEnd, d0 }));
  }
  // 1006: is a Claude session working? (the transcript's last user / assistant entry; its title = the latest ai-title)
  {
    const ca = require('../lib/claudeactivity');
    const J = o => JSON.stringify(o);
    const asst = sr => J({ type: 'assistant', message: { stop_reason: sr, content: [] }, timestamp: '2026-10-06T06:00:00Z' });
    const base = [J({ type: 'ai-title', aiTitle: 'Old' }), J({ type: 'user', message: { content: 'hi' } }), J({ type: 'ai-title', aiTitle: 'HT9050' })];
    const done = ca.stateOf(base.concat([asst('end_turn'), J({ type: 'last-prompt', lastPrompt: 'x' })]).join('\n'));
    const tool = ca.stateOf(base.concat([asst('tool_use'), J({ type: 'attachment' })]).join('\n'));
    const res = ca.stateOf(base.concat([asst('tool_use'), J({ type: 'user', message: { content: [{ type: 'tool_result', content: 'ok' }] } })]).join('\n'));
    const esc = ca.stateOf(base.concat([asst('tool_use'), J({ type: 'user', message: { content: [{ type: 'text', text: '[Request interrupted by user]' }] } })]).join('\n'));
    const side = ca.stateOf(base.concat([asst('end_turn'), J({ type: 'user', isSidechain: true, message: { content: 'sub' } })]).join('\n'));
    const cust = ca.stateOf(base.concat([J({ type: 'custom-title', customTitle: 'Mine' }), asst('end_turn')]).join('\n'));
    // scan: a fresh file and an old one (outside the window), only the tail read
    const cd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-ca-'));
    fs.mkdirSync(path.join(cd, 'p1'));
    const fNew = path.join(cd, 'p1', 'a.jsonl'), fOld = path.join(cd, 'p1', 'b.jsonl');
    fs.writeFileSync(fNew, 'x'.repeat(400000) + '\n' + base.concat([asst('tool_use')]).join('\n') + '\n');
    fs.writeFileSync(fOld, base.concat([asst('tool_use')]).join('\n') + '\n');
    const old = (Date.now() - 3600 * 1000) / 1000;
    fs.utimesSync(fOld, old, old);
    const sc = ca.scan(cd, Date.now(), 15 * 60 * 1000);
    fs.rmSync(cd, { recursive: true, force: true });
    ok(!done.busy && done.title === 'HT9050' && tool.busy && res.busy && !esc.busy && !side.busy && cust.title === 'Mine' &&
      sc.length === 1 && sc[0].busy && sc[0].title === 'HT9050',
      'Claude activity: a finished answer = idle, a tool call / tool result = working, Esc = idle, sub-agent lines ignored; title = latest ai-title (custom first); scan = recent files only, from their tail',
      J({ done, tool: tool.busy, res: res.busy, esc: esc.busy, side: side.busy, cust: cust.title, sc: sc.map(x => x.title + ':' + x.busy) }));
    {
      const d2 = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_ca2_')), p2 = path.join(d2, 'proj');
      fs.mkdirSync(p2);
      fs.writeFileSync(path.join(p2, 'a.jsonl'), [J({ type: 'ai-title', aiTitle: 'T1' }), J({ type: 'assistant', message: { stop_reason: 'tool_use', content: [] } }),
        J({ type: 'user', message: { content: [{ type: 'tool_result', is_error: true, content: "The user doesn't want to take this action right now. STOP" }] } })].join('\n') + '\n');
      fs.writeFileSync(path.join(p2, 'b.jsonl'), [J({ type: 'ai-title', aiTitle: 'T2' }), J({ type: 'user', message: { content: 'x'.repeat(600000) } })].join('\n') + '\n');
      const r2 = ca.scan(d2, Date.now(), 60000).map(x => path.basename(x.file) + ':' + x.title + ':' + x.busy).sort().join(' ');
      try { fs.rmSync(d2, { recursive: true, force: true }); } catch (e) { }
      ok(r2 === 'a.jsonl:T1:false b.jsonl:T2:true',
        '1009 review: a tool the user said No to = not working (it spun 15 minutes); a last line longer than the tail (a pasted image) = still read, working, with its title', r2);
    }
  }

  // --- 1006 (EastSun: "多分頁元件 右鍵新增分頁"): a TPageControl's sheets in the real Alert.Note page (pgcNote, 12 sheets)
  {
    const pc = require('../lib/pagecontrol');
    const J = JSON.stringify;
    const t = fs.readFileSync(path.join(WEB, 'page', 'Alert.Note.html'), 'utf8');
    const ap = (s, parts) => { for (const p of parts.slice().sort((a, b) => b.range[0] - a.range[0])) s = s.slice(0, p.range[0]) + p.repl + s.slice(p.range[1]); return s; };
    const sheets = s => pc.sheetsOf(s, pc.wrapFor(s, 'pgcNote')).list;
    const l0 = sheets(t);
    ok(l0.length === 12 && l0[0].name === 'tsHandler' && l0[0].act && l0[1].caption === 'Red Alarm' && !!pc.wrapFor(t, 'tsRedAlarm') && !!pc.wrapFor(t, 'PanelMain6'),
      'page control: pgcNote has 12 sheets (names, captions, the shown one); found from itself, a sheet, a component on a sheet', l0.length + ' ' + (l0[0] && l0[0].name));
    const a = pc.addSheet(t, 'PanelMain6');
    const t2 = ap(t, a.parts), l2 = sheets(t2), last = l2[l2.length - 1];
    const r = pc.removeSheet(t2, a.name);
    ok(a.name === 'TabSheet13' && a.n === 12 && l2.length === 13 && last.name === 'TabSheet13' && !last.act && /display:none/.test(last.paneTag) && ap(t2, r.parts) === t,
      'page control: a new sheet at the end (TabSheet13, its tab + an empty pane, not shown); deleting it again = the page byte for byte', J({ name: a.name, n: a.n, len: l2.length }));
    const r0 = pc.removeSheet(t, 'tsHandler'), l3 = sheets(ap(t, r0.parts));
    const one = '<div class="pcWrap" id="pg1"><div class="tabs pcTabs"><div class="tab act" data-t="0" title="tsA : TTabSheet">A</div></div><div class="pcBody"><div class="pcPane" data-p="0" title="tsA" style="display:block;"></div></div></div>';
    ok(l3.length === 11 && l3[0].name === 'tsRedAlarm' && l3[0].act && /display:block/.test(l3[0].paneTag) && r0.ids.length > 0 && !!pc.removeSheet(one, 'tsA').error,
      'page control: deleting the shown sheet shows the next one; what was on it is counted (to ask); the last sheet stays', J({ len: l3.length, ids: r0.ids.length }));
    const c = pc.setCaption(t, 'tsRedAlarm', '紅燈 <A>');
    ok(sheets(ap(t, c.parts))[1].caption === '紅燈 <A>' && /紅燈 &lt;A&gt;/.test(ap(t, c.parts)) && !!pc.addSheet(t, '@form').error && !!pc.addSheet(t, 'PanelMain6', { name: 'tsHandler' }).error,
      'page control: a tab caption changed (escaped); the form / a name taken = refused');
    const mv = ap(t, pc.moveSheet(t, 'tsRedAlarm', -1).parts);
    const act = ap(t, pc.setActiveSheet(t, 'tsATC').parts), la = sheets(act);
    ok(sheets(mv).slice(0, 2).map(x => x.name).join() === 'tsRedAlarm,tsHandler' && ap(mv, pc.moveSheet(mv, 'tsRedAlarm', 1).parts) === t && !!pc.moveSheet(t, 'tsHandler', -1).error &&
      la.filter(x => x.act).map(x => x.name).join() === 'tsATC' && la.filter(x => /display:block/.test(x.paneTag)).map(x => x.name).join() === 'tsATC' &&
      !pc.setActiveSheet(act, 'tsATC').parts.length && ap(act, pc.setActiveSheet(act, 'tsHandler').parts) === t,
      'page control: a tab moved left / back (byte for byte), not past the end; the sheet the page opens on (act + display) set and back');
  }
  // --- 1007 audit (props B / K / L): an emptied style attribute goes; "form" as one class among others; equivalent CSS
  {
    const he = require('../lib/htmledit');
    const t1 = '<span id="a">', t2 = '<i id="b" style="color:red">';
    const b1 = he.setStyle(he.setStyle(t1, { display: 'none' }), { display: null }) === t1;
    const b2 = he.setStyle(t2, { color: null }) === '<i id="b">';
    const kf = !!he.startTagOf('<body><div class="form X" id="q">', '@form') && !he.startTagOf('<div class="formX">', '@form');
    const eq = he.cssSame('color', '#000', 'rgb(0, 0, 0)') && he.cssSame('color', '#000', '#000000') && he.cssSame('font-weight', '400', 'normal') &&
      he.cssSame('font-family', "'MS Sans Serif'", '"MS Sans Serif"') && he.cssSame('font-size', '12px', '12.0px') && !he.cssSame('color', '#000', '#001');
    const pt = '<div id="a" style="left:1px;display:none;top:2px;">', pu = '<input id="b" disabled value="x">';
    const posOk = he.setStyle(he.setStyle(pt, { display: null }), { display: 'none' }, { after: { display: 'left' } }) === pt && he.setAttr(he.setAttr(pu, 'disabled', null), 'disabled', true, 'id') === pu;
    ok(b1 && b2 && kf && eq && posOk, 'props audit (display / disabled given back where they were): display off / on leaves no style=""; a style emptied goes; class="form X" is the form; #000 = #000000 = rgb(0,0,0), 400 = normal, quoted font names, 12px = 12.0px', { b1, b2, kf, eq, posOk });
  }
  // --- 1007 audit (toolbox #1 / #8 / #3 / #4 / #5, enumerated): over EVERY named element of EVERY real page, the unit a
  //   structural command moves holds that element only (a GroupBox's div.cli was taken for its first child's wrapper:
  //   974 components took their siblings along); a span wrapper is found past a comment; removeRange of an element
  //   pasted with the page's line end gives the page back byte for byte; a copy's radio group / list / sheet names are
  //   its own; Cut + Paste keeps the names
  {
    const he = require('../lib/htmledit'), hb = require('../lib/htmlblock');
    const pages = fs.readdirSync(path.join(WEB, 'page')).filter(f => /\.html$/i.test(f));
    let n = 0, wrapped = 0, lled = 0, crlfPages = 0, rt = 0, elabN = 0;
    const bad = [], rtBad = [];
    for (const f of pages) {
      const t = fs.readFileSync(path.join(WEB, 'page', f), 'utf8');
      if (/\r\n/.test(t)) crlfPages++;
      const nl = /\r\n/.test(t) ? '\r\n' : '\n';
      const ids = new Set();
      { const re = /\sid\s*=\s*["']([^"'<>]+)["']/g; let m; while ((m = re.exec(t))) ids.add(m[1]); }
      for (const id of ids) {
        const tg = he.startTagOf(t, id);
        if (!tg) continue;
        const er = hb.elementRange(t, tg);
        if (!er) continue;
        const w = hb.wrapsOnly(t, tg);
        // (1007 audit, surface #5: componentUnit -- a TLabeledEdit's span with its .elab caption is the unit too)
        const u = hb.componentUnit(t, id);
        if (!w && u && u.start < tg.start) elabN++;
        if (!u) continue;
        n++;
        if (w) { wrapped++; if (/\blled\b/.test(u.tag.text)) lled++; }
        if (hb.idsIn(u.html).join() !== hb.idsIn(t.slice(er[0], er[1])).join() && bad.length < 8) bad.push(f + ':' + id);
        // (paste next to itself with the page's line end, then delete the copy: the page back)
        if (n % 7 === 0) {
          rt++;
          const ins = t.slice(0, u.end) + nl + hb.indentAt(t, u.start) + u.html + t.slice(u.end);
          const cs = u.end + nl.length + hb.indentAt(t, u.start).length;
          const rm = hb.removeRange(ins, cs, cs + u.html.length);
          if (rm.text !== t && rtBad.length < 5) rtBad.push(f + ':' + id);
        }
      }
    }
    const sp = '<div id="P" style="position:absolute"><span style="position:absolute;left:4px"><!-- x --><input id="E1"></span></div>';
    const spOk = hb.wrapsOnly(sp, he.startTagOf(sp, 'E1')) && hb.unitOf(sp, 'E1', 'parent').tag.text.startsWith('<span');
    const cp = '<div id="rg" title="rg : TRadioGroup"><input type="radio" id="r1" name="rg_rg"><span title="ts : TTabSheet">A</span><div title="ts"><input id="e" list="dl"><datalist id="dl"></datalist></div></div>';
    const r = hb.renameIds(cp, new Set(['rg', 'r1', 'ts', 'e', 'dl']));
    const refsOk = /name="rg_rg_2"/.test(r.html) && /title="ts_2 : TTabSheet"/.test(r.html) && /title="ts_2"/.test(r.html) && /list="dl_2"/.test(r.html);
    const keepOk = hb.renameIds(cp, new Set(['other']), { keepFree: true }).html === cp;
    ok(n > 15000 && !bad.length && lled > 200 && elabN > 40 && crlfPages === pages.length && rt > 2000 && !rtBad.length && spOk && refsOk && keepOk,
      'structural unit on every named element of every page (' + n + ', ' + wrapped + ' with a wrapper, ' + lled + ' labeled LEDs, ' + elabN + ' TLabeledEdit spans): only that element (no GroupBox .cli taken for a wrapper); a wrapper past a comment; paste + delete with the page\'s CRLF byte for byte (' + rt + '); a copy\'s radio group / list / sheet names its own; Cut + Paste keeps the names',
      JSON.stringify({ bad, rtBad, crlfPages, pages: pages.length, spOk, refsOk, keepOk }));
  }
  // --- 1006 audit (the lib enumeration, kept as a test): over EVERY element of EVERY real page --
  //   setAttr(an attribute, its own value) = the tag unchanged (it used to move the attribute and re-encode it);
  //   startTagOf never answers with a "tag" inside a comment / script / style (it wrote edits into the page's JS);
  //   itemsEdit(the same items) = no change (it lost <option value>, comments, &#x27;);
  //   captionRange on an <input> / <img> = none (it went on to a sibling's text)
  {
    const he = require('../lib/htmledit'), itm = require('../lib/items');
    const pages = fs.readdirSync(path.join(WEB, 'page')).filter(f => /\.html$/i.test(f));
    let tags = 0, attrN = 0, attrBad = [], fake = [], itemsN = 0, itemsBad = [], voidN = 0, voidBad = [];
    for (const f of pages) {
      const t = fs.readFileSync(path.join(WEB, 'page', f), 'utf8');
      const dead = [];
      { const re = /<!--|<script\b|<style\b/gi; let m; while ((m = re.exec(t))) { const s0 = m.index; if (m[0] === '<!--') { const e = t.indexOf('-->', s0); dead.push([s0, e < 0 ? t.length : e + 3]); re.lastIndex = e < 0 ? t.length : e + 3; continue; } const te = he.tagEnd(t, s0); const ce = t.toLowerCase().indexOf('</' + m[0].slice(1).toLowerCase(), te); dead.push([te + 1, ce < 0 ? t.length : ce]); re.lastIndex = ce < 0 ? t.length : ce; } }
      const inDead = p => dead.some(r => p >= r[0] && p < r[1]);
      const ids = new Set();
      { const re = /\sid\s*=\s*["']([^"'<>]+)["']/g; let m; while ((m = re.exec(t))) ids.add(m[1]); }
      for (const id of ids) {
        const tg = he.startTagOf(t, id);
        if (!tg) continue;
        if (inDead(tg.start)) { fake.push(f + ':' + id); continue; }
        tags++;
        for (const a of he.attrTokens(tg.text)) {
          if (a.q === null) continue;
          attrN++;
          const v = he.decodeEnt(tg.text.slice(a.vs, a.ve));
          if (he.setAttr(tg.text, a.name, v) !== tg.text && attrBad.length < 5) attrBad.push(f + ':' + id + '.' + a.name);
        }
        const nm = (/^<([a-z]+)/i.exec(tg.text) || [])[1];
        if (nm && he.VOID.has(nm.toLowerCase())) { voidN++; if (he.captionRange(t, tg, 'text') && voidBad.length < 5) voidBad.push(f + ':' + id); }
        const io = itm.itemsOf(t, id);
        if (io && io.kind !== 'memo') {
          itemsN++;
          const e = itm.itemsEdit(t, id, io.items);
          if ((t.slice(0, e.range[0]) + e.repl + t.slice(e.range[1])) !== t && itemsBad.length < 5) itemsBad.push(f + ':' + id);
        }
      }
    }
    ok(tags > 15000 && attrN > 50000 && !attrBad.length && !fake.length && itemsN > 300 && !itemsBad.length && voidN > 100 && !voidBad.length,
      'lib on every element of every page (' + pages.length + ' pages, ' + tags + ' tags, ' + attrN + ' attributes, ' + itemsN + ' item lists, ' + voidN + ' void elements): setAttr(same) unchanged, no fake tag inside script / comments, itemsEdit(same) unchanged, no caption inside a void element',
      JSON.stringify({ attrBad, fake: fake.slice(0, 5), itemsBad, voidBad }));
    const nt = '<p><input title="e1 (value 1)" placeholder="type a value here" value="abc" id="e1"> <span data-id="dx">a</span><script>var s=\'<b id="sx">\';</script></p>';
    const e1 = he.startTagOf(nt, 'e1');
    ok(he.setAttr(e1.text, 'value', 'NEW') === '<input title="e1 (value 1)" placeholder="type a value here" value="NEW" id="e1">' &&
      he.setAttr('<a title="x disabled y" disabled>', 'disabled', null) === '<a title="x disabled y">' && !he.startTagOf(nt, 'dx') && !he.startTagOf(nt, 'sx') &&
      he.decodeEnt('Don&#x27;t &amp; &nbsp;') === 'Don\'t &  ',
      'setAttr changes the value in place, quote-aware (a "value" / "disabled" inside another attribute untouched); data-id is not id; an id in a script is not a tag');
  }
  // --- 1006 (EastSun 「繼續」: 兩個頁面比較 / 跨頁面批次改文字): lib/pagecompare on the real pages
  {
    const pcm = require('../lib/pagecompare');
    const ta = fs.readFileSync(path.join(WEB, 'page', 'Alert.MotionView.html'), 'utf8'), tb = fs.readFileSync(path.join(WEB, 'page', 'Alert.MotionView9050.html'), 'utf8');
    const r = pcm.compare(ta, tb), self = pcm.compare(ta, ta);
    const hp = pcm.componentsOf(fs.readFileSync(PAGE, 'utf8'));
    const sv = hp.get('spbSave'), x1 = hp.get('XST1');
    const md = pcm.compareMarkdown('A.html', 'B.html', r);
    const hits = pcm.captionHits([{ file: 'hp', text: fs.readFileSync(PAGE, 'utf8') }], 'Save', 'Keep', true);
    const sub = pcm.captionHits([{ file: 'x', text: '<div class="form"><button id="b1" style="left:1px">Save all</button><input id="e1" value="Save"><script>var s="Save";</script></div>' }], 'Save', 'Keep', false);
    ok(r.countA > 0 && r.countB > 0 && r.same + r.diff.length + r.onlyA.length === r.countA && self.diff.length === 0 && self.onlyA.length === 0 &&
      sv && sv.cls === 'TSpeedButton' && sv.caption === 'Save' && sv.left === 54 && sv.width === 227 && x1 && x1.left === 129 && x1.width === 61 &&
      /^# 頁面比較：A\.html ↔ B\.html/.test(md) && hits.length === 1 && hits[0].id === 'spbSave' && hits[0].now === 'Keep' &&
      sub.map(h => h.id + '=' + h.now).join() === 'b1=Keep all,e1=Keep',
      'pagecompare: components by id (class, caption, place / size incl. a wrapper span), two pages compared (a page with itself = no difference), Markdown; caption hits whole / part (an input value too, never a script)',
      JSON.stringify({ counts: [r.countA, r.countB, r.same, r.diff.length, r.onlyA.length, r.onlyB.length], sub: sub.map(h => h.id) }));
  }
  // --- 1006 audit (the run bar, build integrity): a cut / zero-headed exe is not "built" (F5 used to skip the build and
  // run it -- the CLAUDE.md "27 cut exes" case)
  {
    const up = require('../lib/uptodate');
    const os = require('os');
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'htd-pe-'));
    const whole = path.join(dir, 'w.exe'), cut = path.join(dir, 'c.exe'), zero = path.join(dir, 'z.exe');
    const pe = fakePE(80 * 1024);
    fs.writeFileSync(whole, pe);
    fs.writeFileSync(cut, pe.subarray(0, 60 * 1024));
    const z = Buffer.from(pe); z.fill(0, 0, 4096); fs.writeFileSync(zero, z);
    const r = [up.peBroken(whole), up.peBroken(cut), up.peBroken(zero)];
    fs.rmSync(dir, { recursive: true, force: true });
    ok(r[0] === null && /截斷/.test(r[1] || '') && /MZ/.test(r[2] || ''),
      'uptodate.peBroken: a whole PE passes, one cut short or with a zeroed header does not', JSON.stringify(r));
  }
  // --- 1006 (EastSun: 設計畫面匯出成圖片): lib/snapshot on every real page -- the form's size, the copy that cannot reach the network
  {
    const sn = require('../lib/snapshot');
    const pages = fs.readdirSync(path.join(WEB, 'page')).filter(f => /\.html$/i.test(f));
    let sized = 0, bad = [];
    for (const f of pages) {
      const t = fs.readFileSync(path.join(WEB, 'page', f), 'utf8');
      const z = sn.formSize(t);
      if (z.fromForm) sized++;
      const h = sn.snapshotHtml(t, path.join(WEB, 'page'));
      if (!/connect-src 'none'/.test(h) || !/<base href="file:\/\/\//.test(h) || !/window\.fetch=function\(\)\{return new Promise/.test(h) || h.indexOf(t.slice(-200)) < 0) bad.push(f);
    }
    const hp = sn.formSize(fs.readFileSync(PAGE, 'utf8'));
    const args = sn.edgeArgs({ file: 'C:\\t\\p.html', out: 'C:\\t\\o.png', w: 606, h: 531, profile: 'C:\\t\\prof' });
    const mh = sn.snapshotHtml('<html><head></head><body></body></html>', path.join(WEB, 'page'), { measure: true });
    const ps = sn.parseSize('<head><meta name="htd-size" content="1256x932"></head>');
    ok(hp.w === 606 && hp.h === 531 && sized > 30 && !bad.length && /htd-size/.test(mh) && ps && ps.w === 1256 && ps.h === 932 && !sn.parseSize('<p>') && args.includes('--window-size=606,531') && args.includes('--screenshot=C:\\t\\o.png') &&
      /^file:\/\/\/C:\/t\/p\.html$/.test(args[args.length - 1]) && sn.defaultName('D:\\w\\Setup.HotPlate.html', new Date(2026, 9, 6, 18, 5)) === 'Setup.HotPlate_20261006_1805.png',
      'snapshot: the form\'s size (HotPlate 606 x 531; ' + sized + ' / ' + pages.length + ' pages give one, the rest are measured by a first pass), every page\'s copy has no network (CSP + fetch / WebSocket that wait) and its folder as base; Edge args; the default name',
      bad.slice(0, 5).join(','));
    const big = sn.formSize('<div class="form" style="width:5000px;height:300px">');
    const cm = sn.snapshotHtml('<html><!-- <head> --><head><title>t</title></head><body>x</body></html>', 'D:\\x&copy_y');
    ok(big.w === 4000 && big.cut === 4000 && cm.indexOf('<!-- <head> -->') >= 0 && /<\/title>/.test(cm) && cm.indexOf('<head><meta') > cm.indexOf('-->') && /x&amp;copy_y/.test(cm),
      '1009 review (export S1-S3): a form over 4000 px says it is cut; the <head> inside a comment is not the head; & in the folder escaped in <base>',
      JSON.stringify({ big, at: cm.slice(0, 60) }));
    const rdBtn = sn.redirectOf('<html><head></head><body>' + 'x'.repeat(5000) + '<button onclick="location.replace(\'main.html\')">b</button></body></html>');
    const rdHead = sn.redirectOf('<html><head><script>location.replace(\'B.html\' + location.search);</script></head><body>' + 'x'.repeat(5000) + '</body></html>');
    ok(rdBtn === null && rdHead === 'B.html',
      '1009 review (export S4): a redirect is a location.replace in the <head> only -- a big page\'s button doing it is no redirect', JSON.stringify({ rdBtn, rdHead }));
  }
  {
    const cl = require('../lib/cmakelists');
    const apply = (t, es) => { for (const e of es.slice().reverse()) t = t.slice(0, e.s) + t.slice(e.e); return t; };
    const one = 'add_library(x STATIC\n  a.cpp\n)\ntarget_sources(y PRIVATE b.cpp c.cpp)\n';
    const e1 = cl.removeSource(one, 'a.cpp'), e2 = cl.removeSource(one, 'b.cpp');
    const q = 'add_library(z\n  "m/a b.cpp"\n  m/c.cpp\n)\n';
    const rq = apply(q, cl.removeSource(q, 'm/a b.cpp'));
    const sp = 'set_source_files_properties(a.cpp # (old flags)\n  PROPERTIES COMPILE_FLAGS -O0)\nadd_library(w a.cpp b.cpp)\n';
    const rs = apply(sp, cl.removeSource(sp, 'a.cpp'));
    ok(e1.empties.join() === 'x' && !e2.empties.length && rq === 'add_library(z\n  m/c.cpp\n)\n' && rs === 'add_library(w b.cpp)\n',
      '1009 review (new item #2 #6 #7): a target\'s last source is flagged (empties), the quotes of "m/a b.cpp" go with it, a "(" in a # comment is no paren',
      JSON.stringify({ e1: e1.empties, e2: e2.empties, rq, rs }));
    const own = cl.refsUnder('add_executable(t a.cpp)\ntarget_link_libraries(t PRIVATE x)\n', 'tests', 'tests');
    const ld = cl.refsAll('include(${CMAKE_CURRENT_LIST_DIR}/ui/native/N.cmake)\n', 'ui/native/N.cmake', '');
    ok(own.length === 0 && ld.length === 1,
      '1009 review (solution #1 #3): a CMakeLists.txt\'s own folder renamed does not match its every word; ${CMAKE_CURRENT_LIST_DIR}/ is a spelling',
      JSON.stringify({ own: own.length, ld: ld.length }));
    const cv = require('../lib/csvtable');
    const u = Buffer.from('名稱,值\r\n中文,1\r\n', 'utf8');
    const asCp = new (require('util').TextDecoder)('big5').decode(u);   // (the UTF-8 bytes as VS Code shows them read in Big5)
    const d1 = cv.decodeSafe(u, asCp, 'cp950'), d2 = cv.decodeSafe(u, asCp), d3 = cv.decodeSafe(u, u.toString('utf8'), 'utf8');
    const d4 = cv.decodeSafe(Buffer.from('Motorname,Alias\r\n'), 'Motorname,Alias\r\n', 'utf8', 'D:\\HT9045\\system\\Mot_Table.csv');
    const d5 = cv.decodeSafe(Buffer.from([0x41, 0x2c, 0xa4, 0xa4]), 'A,中', 'cp950');
    const p = cv.parse('a,b\r\n0x10,1\r\n9,2\r\n1e1,3\r\n');
    const so = cv.sortRows(p, 'a,b\r\n0x10,1\r\n9,2\r\n1e1,3\r\n', 1, 0, false);
    ok(d1.ok === false && d2.ok === false && d3.ok && !d3.big5 && d4.ok && d4.asciiOnly && d5.ok && d5.big5 && so.length > 0,
      '1009 review (csv #2 #4 #8 #9): a UTF-8 file read as Big5 is read-only (with or without TextDocument.encoding); an all-ASCII machine table read as UTF-8 is marked; cp950 = big5; 0x10 / 1e1 sort as text',
      JSON.stringify({ d1, d2, d3, d4, d5 }));
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
