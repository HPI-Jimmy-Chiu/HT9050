'use strict';
// AI(W906-HTDESIGNER) 20261005 (ES02, EastSun: "圖片上按鈕我希望都有作用 現在好像有些是假的"): every button of the two
// toolbars pressed in a REAL VS Code, and what it did checked -- not the command it is wired to:
//   editor title bar: save / save all / back / forward / ▶ / continue / pause / stop / restart / step over / into / out /
//                     模擬 / Debug;   方案總管 toolbar: the same run buttons + ◎ sync / ↻ refresh / ⊟ collapse.
// ▶ really builds and starts wb_serve (模擬 + Debug = build_dbg_nonoracle, under gdb), with the tree's own launch entry
// (its W906_* paths = the runcfg sandbox, not the machine's settings). Launched by test\vscode_run_it.ps1.
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
  const X = id => vscode.commands.executeCommand(id);
  try {
    out.push('vscode ' + vscode.version);
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    const api = await ext.activate();
    const hub = api.hub, rb = hub.runBar;
    const tree = rb.plan().tree;
    out.push('      tree ' + tree + '  plan ' + JSON.stringify(rb.plan()));
    const mine = async () => rb.treeProcs(await rb.listProcs());
    ok((await mine()).length === 0, 'nothing of this tree running before the test', JSON.stringify(await mine()));

    // ---- save / save all ----
    const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'htd_btn_'));
    const fA = path.join(tmp, 'a.txt'), fB = path.join(tmp, 'b.txt');
    fs.writeFileSync(fA, 'A0\n'); fs.writeFileSync(fB, 'B0\n');
    const dA = await vscode.workspace.openTextDocument(fA), dB = await vscode.workspace.openTextDocument(fB);
    await vscode.window.showTextDocument(dB, { preview: false });
    const eA = await vscode.window.showTextDocument(dA, { preview: false });
    await eA.edit(e => e.insert(new vscode.Position(0, 0), 'x'));
    await X('ht9045Designer.saveFile');
    const savedA = fs.readFileSync(fA, 'utf8');
    ok(savedA === 'xA0\n' && !dA.isDirty, '💾 save: the file in front written to disk', JSON.stringify(savedA));
    const wsEd = new vscode.WorkspaceEdit();
    wsEd.insert(dA.uri, new vscode.Position(0, 0), 'y'); wsEd.insert(dB.uri, new vscode.Position(0, 0), 'z');
    await vscode.workspace.applyEdit(wsEd);
    await X('ht9045Designer.saveAllFiles');
    const sA2 = fs.readFileSync(fA, 'utf8'), sB2 = fs.readFileSync(fB, 'utf8');
    ok(sA2 === 'yxA0\n' && sB2 === 'zB0\n', '💾💾 save all: both changed files written', JSON.stringify([sA2, sB2]));

    // ---- back / forward ----
    const cpp = path.join(tree, 'forms', 'fHotPlate.cpp'), hh = path.join(tree, 'forms', 'fHotPlate.h');
    await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(cpp), { preview: false });
    await sleep(300);
    await vscode.window.showTextDocument(await vscode.workspace.openTextDocument(hh), { preview: false });
    await sleep(300);
    const act = () => { const e = vscode.window.activeTextEditor; return e ? path.basename(e.document.uri.fsPath) : '-'; };
    await X('ht9045Designer.navBack');
    const back = await waitFor(() => act() === 'fHotPlate.cpp' && act(), 3000) || act();
    await X('ht9045Designer.navForward');
    const fwd = await waitFor(() => act() === 'fHotPlate.h' && act(), 3000) || act();
    ok(back === 'fHotPlate.cpp' && fwd === 'fHotPlate.h', '← back / → forward: the editor goes to the previous file and back again', back + ' / ' + fwd);

    // ---- 方案總管: ◎ sync (a text file, then the designer in front), ⊟ collapse, ↻ refresh ----
    const n1 = await X('ht9045Designer.solutionReveal');
    ok(!!(n1 && n1.path && path.resolve(n1.path).toLowerCase() === path.resolve(hh).toLowerCase()), '◎ sync: 方案總管 selects the file in front (a C++ file)', n1 && n1.path);
    const page = path.join(path.dirname(tree), 'web', 'page', 'Setup.HotPlate.html');
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(page), 'ht9045Designer.editor');
    await waitFor(() => hub.active && hub.active.treeData.length, 30000);
    const n2 = await X('ht9045Designer.solutionReveal');
    ok(!!(n2 && n2.path && path.resolve(n2.path).toLowerCase() === path.resolve(page).toLowerCase()), '◎ sync with the designer in front (no text editor): the page is selected', n2 ? n2.path : 'null');
    const sp = hub.solPanel;
    const openBefore = Array.from(sp.open.values()).filter(Boolean).length;
    await X('ht9045Designer.solutionCollapseAll');
    const openAfter = Array.from(sp.items.entries()).filter(([id, x]) => x.n.type !== 'sln' && sp.open.get(id)).length;
    ok(openBefore > 0 && openAfter === 0, '⊟ collapse: every open folder of 方案總管 folded', openBefore + ' open -> ' + openAfter);
    let refreshed = 0; const r0 = hub.solution.refresh.bind(hub.solution);
    hub.solution.refresh = (...a) => { refreshed++; return r0(...a); };
    await X('ht9045Designer.solutionRefresh');
    hub.solution.refresh = r0;
    ok(refreshed > 0, '↻ refresh: 方案總管 read again (its cache cleared)', 'refresh calls ' + refreshed);

    // ---- 模擬 / Debug ----
    const c = () => vscode.workspace.getConfiguration('ht9045Designer');
    const sim0 = rb.opt('run.simulation'), dbg0 = rb.opt('run.debug');
    await X(sim0 ? 'ht9045Designer.run.simOn' : 'ht9045Designer.run.simOff');
    const sim1 = rb.opt('run.simulation');
    await X(sim1 ? 'ht9045Designer.run.simOn' : 'ht9045Designer.run.simOff');
    await X(dbg0 ? 'ht9045Designer.run.dbgOn' : 'ht9045Designer.run.dbgOff');
    const dbg1 = rb.opt('run.debug');
    await X(dbg1 ? 'ht9045Designer.run.dbgOn' : 'ht9045Designer.run.dbgOff');
    ok(sim1 === !sim0 && rb.opt('run.simulation') === sim0 && dbg1 === !dbg0 && rb.opt('run.debug') === dbg0 && c().get('run.simulation') !== undefined,
      '模擬 / Debug buttons: each press switches its setting (and back)', [sim0, sim1, dbg0, dbg1].join(','));
    // (Debug needs a gdb that can start a program: ES02's endpoint security kills it on any program. The check ▶ asks
    //  first is checked against that fact here; then the run goes in the mode this PC can do -- Release on ES02 -- and
    //  pause / the steps, which need the debugger, are tested only where gdb works, said SKIP otherwise.)
    const gdbPath = (rb.launchFor(rb.plan()).cfg || {}).miDebuggerPath;
    const gw = await rb.gdbWorks(gdbPath);
    const gdbRaw = await new Promise(res => require('child_process').execFile(String(gdbPath).replace(/\$\{env:([^}]+)\}/g, (m, k) => process.env[k] || ''),
      ['-nx', '--batch', '-ex', 'starti', '-ex', 'kill', '--args', path.join(process.env.SystemRoot || 'C:\\Windows', 'SysWOW64', 'whoami.exe')], { timeout: 30000, windowsHide: true }, e => res(e ? (e.code >>> 0).toString(16) : '0')));
    ok(gw.ok === (gdbRaw === '0'), 'the gdb check ▶ asks before Debug tells the truth on this PC', 'check=' + JSON.stringify(gw) + ' gdb starti whoami exit=0x' + gdbRaw);
    if (!gw.ok) await X('ht9045Designer.run.dbgOn');   // (the Debug button pressed: -> Release)
    const DBG = rb.plan().dbg;
    const dbgSnap = (rb.snapshot().b || {}).dbg || {};
    ok(/(Debug|Release)/.test(rb.items.dbg.text) && rb.items.dbg.text.indexOf(DBG ? 'Debug' : 'Release') >= 0 && rb.snapshot().dbg === DBG && !!dbgSnap.cmd,
      'the Debug button says what it is now (Debug <-> Release), pressing it switches', rb.items.dbg.text);
    out.push('      run mode: ' + rb.plan().dir + (DBG ? ' (Debug, gdb)' : ' (Release: this PC\'s gdb cannot start a program)'));

    // ---- ▶ build + start ----
    // (what the debugger says -- its output, the exit code: why a session ended, in the report)
    const dap = [];
    const trk = vscode.debug.registerDebugAdapterTrackerFactory('*', { createDebugAdapterTracker: s => ({
      onDidSendMessage: m => { if (m.type === 'event' && /^(output|exited|terminated|stopped)$/.test(m.event)) dap.push(m.event + ' ' + JSON.stringify(m.body || {}).slice(0, 300)); },
      onError: e => dap.push('error ' + (e && e.message)), onExit: (c, sig) => dap.push('adapter exit ' + c + ' ' + (sig || '')),
    }) });
    const sesLog = [];
    const sS = vscode.debug.onDidStartDebugSession(s => sesLog.push('start ' + s.name + ' noDebug=' + !!s.configuration.noDebug));
    const sT = vscode.debug.onDidTerminateDebugSession(s => sesLog.push('end ' + s.name));
    const ctx = () => rb.curState();
    ok(ctx() === 'idle', 'idle before ▶');
    const t0 = Date.now();
    const bs = await X('ht9045Designer.run.buildAndStart');
    ok(!!(bs && bs.built && bs.started), '▶ build + start: built, then started', JSON.stringify(bs) + ' ' + Math.round((Date.now() - t0) / 1000) + ' s');
    const procs1 = await waitFor(async () => { const p = await mine(); return p.length ? p : null; }, 60000, 1000);
    ok(!!procs1 && path.resolve(procs1[0].path).toLowerCase() === path.join(tree, rb.plan().dir, 'wb_serve.exe').toLowerCase(), '▶: wb_serve.exe of ' + rb.plan().dir + ' is running', JSON.stringify(procs1));
    out.push('      sessions: ' + sesLog.join(' | '));
    dap.slice(0, 60).forEach(l => out.push('      dap: ' + l.replace(/\s+/g, ' ')));
    ok(await waitFor(() => ctx() === 'running', 30000), 'state running (pause / stop / restart lit)', ctx());
    // (gdb reads 120 MB of symbols first: give it time before the interrupt)
    await sleep(8000);
    if (!DBG) {
      ok(!rb.items.pause.command && !rb.items.over.command, '⏸ / steps grey in a Release run (no debugger to stop it -- like Visual Studio without one)');
      out.push('SKIP  ⏸ pause / ↷ ↓ ↑ steps / |▷ continue: need a gdb that can start a program (not on this PC)');
    }

    // ---- ⏸ pause / step / ▶ continue ----
    if (DBG) await X('ht9045Designer.run.pause');
    const paused = DBG ? await waitFor(() => ctx() === 'paused', 30000) : null;
    const fr = vscode.debug.activeStackItem;
    if (DBG) ok(!!paused, '⏸ pause: the program stops in the debugger (a stack frame in focus)', ctx() + ' ' + (fr && fr.frameId !== undefined ? 'frame ' + fr.frameId : ''));
    if (paused) {
      for (const [id, name] of [['stepOver', '↷ step over'], ['stepInto', '↓ step into'], ['stepOut', '↑ step out']]) {
        let changed = false;
        const sub = vscode.debug.onDidChangeActiveStackItem(() => { changed = true; });
        await X('ht9045Designer.run.' + id);
        const again = await waitFor(() => changed && ctx() === 'paused', 20000);
        sub.dispose();
        ok(!!again || ctx() === 'running', name + ': the debugger moved and stopped again (or ran on, when the step left the frame)', ctx());
        if (ctx() === 'running') { await X('ht9045Designer.run.pause'); await waitFor(() => ctx() === 'paused', 20000); }
      }
      await X('ht9045Designer.run.continue');
      ok(!!(await waitFor(() => ctx() === 'running', 20000)), '|▷ continue: running again', ctx());
    }

    // ---- ↺ restart = stop, build, start ----
    const pid1 = procs1 ? procs1[0].pid : 0;
    const rs = await X('ht9045Designer.run.restart');
    const procs2 = await waitFor(async () => { const p = await mine(); return p.length && p[0].pid !== pid1 ? p : null; }, 120000, 1000);
    ok(!!(rs && rs.built && rs.started) && !!procs2, '↺ restart: the old wb_serve stopped, built again, a new one started', 'pid ' + pid1 + ' -> ' + (procs2 ? procs2[0].pid : '-') + ' ' + JSON.stringify(rs));
    await waitFor(() => ctx() === 'running', 30000);

    // ---- ⏹ stop ----
    const st = await X('ht9045Designer.run.stopAll');
    const gone = await waitFor(async () => (await mine()).length === 0 && ctx() === 'idle', 30000, 1000);
    ok(!!gone, '⏹ stop: the session ended and no wb_* of this tree is left', JSON.stringify(st) + ' left ' + JSON.stringify(await mine()));
    try { fs.rmSync(tmp, { recursive: true, force: true }); } catch (e) { /* temp */ }
  } catch (e) {
    ok(false, 'CRASH ' + (e && e.stack || e));
  } finally {
    try { await vscode.commands.executeCommand('ht9045Designer.run.stopAll'); } catch (e) { /* end */ }
    out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
    if (report) fs.writeFileSync(report, out.join('\n') + '\n', 'utf8');
  }
};
