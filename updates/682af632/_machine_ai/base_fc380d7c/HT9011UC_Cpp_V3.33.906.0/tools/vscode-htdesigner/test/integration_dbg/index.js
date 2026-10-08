'use strict';
// AI(W906-HTDESIGNER) 20261006 (ES02, EastSun: "debug 就是可以攔中斷的模式喔"): Debug in a REAL VS Code with the real C++
// debugger extension (cpptools): a launch with gdb on this PC goes through the extension's provider, which -- gdb unable to
// start a program here -- hands it to LLDB (lldb-dap, the ht9045-lldb debugger); a breakpoint on the line after ten threads ran must stop, a variable
// must read 10000, and Continue must run the program to its end. Launched by test\vscode_dbg_it.ps1.
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
  // (written after every step: a step that hangs still leaves what came before it)
  const flush = () => { if (report) try { fs.writeFileSync(report, out.join('\n'), 'utf8'); } catch (e) { /* next time */ } };
  const step = t => { out.push('      .. ' + t); flush(); };
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); flush(); };
  try {
    out.push('vscode ' + vscode.version);
    const ext = vscode.extensions.getExtension('ht9045.ht9045-html-designer');
    const api = await ext.activate();
    const exe = process.env.HTD_DBG_EXE, src = process.env.HTD_DBG_SRC, gdb = process.env.HTD_DBG_GDB;
    const line = fs.readFileSync(src, 'utf8').split(/\r?\n/).findIndex(x => /HTD-STOP-HERE/.test(x));
    out.push('      program ' + exe + '  stop at ' + path.basename(src) + ':' + (line + 1) + '  gdb ' + gdb);
    step('gdb check');
    const g = await api.hub.runBar.gdbWorks(gdb);
    out.push('      gdb on this PC: ' + (g.ok ? 'works' : 'cannot (' + g.why + ')'));
    // what the debug adapter says: stops, the program's end
    const stops = [], trace = [];
    const dumpTrace = () => { out.push('      ---- adapter trace (' + trace.length + ')'); trace.slice(0, 120).forEach(t => out.push('      | ' + t)); flush(); };
    let exited = null, started = null;
    const tf = vscode.debug.registerDebugAdapterTrackerFactory('*', {
      createDebugAdapterTracker: () => ({
        onDidSendMessage: m => {
          if (m && m.type === 'event' && m.event === 'stopped') stops.push(m.body || {});
          if (m && m.type === 'event' && m.event === 'output' && m.body && trace.length < 400) { trace.push(String(m.body.output || '').trim().slice(0, 240)); }
          if (m && m.type === 'response' && trace.length < 400) trace.push('<- ' + m.command + ' ' + (m.success ? 'ok' : 'FAILED ' + (m.message || '')));
          if (m && m.type === 'event' && m.event === 'exited') exited = m.body ? m.body.exitCode : -1;
        },
      }),
    });
    const ended = new Promise(r => { const d = vscode.debug.onDidTerminateDebugSession(s => { d.dispose(); r(s); }); });
    const sd = vscode.debug.onDidStartDebugSession(s => { started = s; });
    vscode.debug.addBreakpoints([new vscode.SourceBreakpoint(new vscode.Location(vscode.Uri.file(src), new vscode.Position(line, 0)))]);
    // a launch as launch.json has it: gdb (the extension decides whether it can run here)
    const t0 = Date.now();
    const via = await api.hub.runBar.debugVia({ type: 'cppdbg', program: exe, MIMode: 'gdb', miDebuggerPath: gdb });
    step('debugVia: ' + via.via + ' ' + (via.cfg ? (via.cfg.lldbDap || via.cfg.miDebuggerPath) : via.why) + ' (' + (Date.now() - t0) + ' ms)');
    step('startDebugging');
    const dumpLate = setTimeout(dumpTrace, 25000);   // (the adapter's own words, when it has not stopped by then)
    const okStart = await vscode.debug.startDebugging(undefined, {
      type: 'cppdbg', request: 'launch', name: 'HTD debug test', program: exe, args: [], cwd: path.dirname(exe), stopAtEntry: false,
      environment: [{ name: 'HTD_DBG_ENV', value: '1' }], externalConsole: false, MIMode: 'gdb', miDebuggerPath: gdb,
      setupCommands: [{ text: '-enable-pretty-printing', ignoreFailures: true }],
      logging: { engineLogging: true, trace: true, traceResponse: true },
    });
    step('started: ' + okStart);
    const ses = await waitFor(() => started, 20000);
    const cfg = ses ? ses.configuration : {};
    ok(okStart && !!ses && (g.ok ? cfg.type === 'cppdbg' && cfg.MIMode === 'gdb' : cfg.type === 'ht9045-lldb' && /lldb-dap\.exe$/i.test(cfg.lldbDap || '') && cfg.env && cfg.env.HTD_DBG_ENV === '1'),
      'Debug started -- with gdb where gdb works, else through LLDB (lldb-dap, the environment carried over)', JSON.stringify({ okStart, type: cfg.type, MIMode: cfg.MIMode, dap: cfg.lldbDap, env: cfg.env }));
    step('waiting for the breakpoint');
    const st = await waitFor(() => stops.find(s => s.reason === 'breakpoint'), 60000);
    if (st) clearTimeout(dumpLate);
    ok(!!st, 'the breakpoint stops the program (after its ten threads ran)', JSON.stringify(stops));
    let at = null, val = null;
    if (st && ses) {
      const tr = await ses.customRequest('stackTrace', { threadId: st.threadId, startFrame: 0, levels: 1 });
      const fr = tr && tr.stackFrames && tr.stackFrames[0];
      at = fr ? path.basename((fr.source && fr.source.path) || '') + ':' + fr.line : null;
      const ev = fr ? await ses.customRequest('evaluate', { expression: 'n', frameId: fr.id, context: 'watch' }) : null;
      val = ev ? String(ev.result) : null;
    }
    ok(at === path.basename(src) + ':' + (line + 1), 'stopped on that very line', String(at));
    ok(val === '10000', 'a variable reads right there (n = 10000)', String(val));
    // (1008, EastSun「debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」): the commands the BCB keys are bound to, in the real
    //  debugger -- F8 Step Over twice (the second one over a call: it must NOT go in), F7 Trace Into (it MUST go in), F9 Run
    const srcLines = fs.readFileSync(src, 'utf8').split(/\r?\n/);
    const lineOf = tag => srcLines.findIndex(x => x.indexOf(tag) >= 0) + 1;
    const where = async () => {
      const s = stops[stops.length - 1];
      const tr = await ses.customRequest('stackTrace', { threadId: s.threadId, startFrame: 0, levels: 1 });
      const fr = tr && tr.stackFrames && tr.stackFrames[0];
      return fr ? { line: fr.line, fn: fr.name } : null;
    };
    const keyStep = async (cmd, label) => {
      const n0 = stops.length;
      await vscode.commands.executeCommand(cmd);
      const s = await waitFor(() => stops.length > n0, 20000);
      return s ? await where() : null;
    };
    // the keys the extension wrote into this VS Code's user keybindings.json (it does that when it starts)
    const kbFile = path.join(path.dirname(process.env.HTD_IT_REPORT || '.'), 'user', 'User', 'keybindings.json');
    const kbText = await waitFor(() => fs.existsSync(kbFile) && fs.readFileSync(kbFile, 'utf8'), 10000);
    const kb = String(kbText || '');
    const esc = x => x.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    const has = (k, c) => new RegExp('"key": "' + esc(k) + '", "command": "' + esc(c) + '", "when": "inDebugMode && debugState == \'stopped\'').test(kb);
    // (1008: every command the BCB keys name is a real command of this VS Code / its C/C++ extension -- a wrong id is silent)
    const allCmds = new Set(await vscode.commands.getCommands(true));
    const bk = require(path.join(__dirname, '..', '..', 'lib', 'bcbkeys.js'));
    const missingCmds = bk.KEYS.filter(k => !/^-/.test(k.command) && !allCmds.has(k.command)).map(k => k.key + '=' + k.command);
    ok(missingCmds.length === 0, 'every BCB key names a command that exists here (' + bk.KEYS.length + ' keys)', missingCmds.join(', '));
    ok(has('f8', 'workbench.action.debug.stepOver') && has('f7', 'workbench.action.debug.stepInto') && has('f9', 'workbench.action.debug.continue'),
      'the user keybindings (the level that beats every extension) map F8 / F7 / F9 to Step Over / Trace Into / Run while stopped', kbFile);
    if (st && ses) {
      // (1008 gap list #13: Show Execution Point -- from another file, back to the stopped line)
      await vscode.window.showTextDocument(await vscode.workspace.openTextDocument({ content: 'elsewhere', language: 'plaintext' }));
      const ep = await vscode.commands.executeCommand('ht9045Designer.debug.showExecPoint');
      const ae = vscode.window.activeTextEditor;
      const epLine = ae ? ae.selection.active.line + 1 : -1, epFile = ae ? path.basename(ae.document.uri.fsPath) : '';
      ok(!!ep && epFile === 'thr.c' && epLine === line + 1,'Show Execution Point: from another editor back to the stopped line', JSON.stringify({ ep, epFile, epLine }));
      const a1 = await keyStep('workbench.action.debug.stepOver', 'F8');
      ok(a1 && a1.line === lineOf('HTD-CALL1'), 'F8 (Step Over): the next line', JSON.stringify(a1));
      const a2 = await keyStep('workbench.action.debug.stepOver', 'F8');
      ok(a2 && a2.line === lineOf('HTD-CALL2') && !/add2/.test(a2.fn || ''), 'F8 over a call: the function runs, the next line of main (not inside it)', JSON.stringify(a2));
      const a3 = await keyStep('workbench.action.debug.stepInto', 'F7');
      ok(a3 && /add2/.test(a3.fn || '') && a3.line === lineOf('HTD-IN-FUNC'), 'F7 (Trace Into): inside the called function, its first line', JSON.stringify(a3));
      // (1008 gap list #3: Shift+F8 Run Until Return -- out of the function, back in main)
      const a4 = await keyStep('workbench.action.debug.stepOut', 'Shift+F8');
      ok(a4 && /main/.test(a4.fn || '') && (a4.line === lineOf('HTD-CALL2') || a4.line === lineOf('HTD-CALL2') + 1), 'Shift+F8 (Run Until Return): the function finishes, back in main', JSON.stringify(a4));
      ok(has('shift+f8', 'workbench.action.debug.stepOut') && /"key": "f7", "command": "-cmake\.build"/.test(kb) && /"key": "ctrl\+f9", "command": "ht9045Designer\.run\.make"/.test(kb),
        'the user keybindings also have Shift+F8, Ctrl+F9 Make, and CMake Tools\' F7 build removed', '');
      try { await vscode.commands.executeCommand('workbench.action.debug.continue'); } catch (e) { out.push('      continue: ' + e.message); }
    }
    const gone = await Promise.race([ended.then(() => true), sleep(30000).then(() => false)]);
    ok(gone && exited === 0, 'F9 (Run): continues to the end (exit 0)', JSON.stringify({ gone, exited }));
    if (!gone) { try { await vscode.debug.stopDebugging(); } catch (e) { /* gone */ } }
    vscode.debug.removeBreakpoints(vscode.debug.breakpoints);
    // (1008 gap list #19, Attach to Process: "thr wait" already running, the debugger attached -- a breakpoint in its loop
    //  stops it there; detached afterwards, the program is ended by the test)
    {
      const cp = require('child_process').spawn(exe, ['wait'], { detached: false, stdio: 'ignore', windowsHide: true });
      await sleep(800);
      const wl = fs.readFileSync(src, 'utf8').split(/\r?\n/).findIndex(x => /HTD-ATTACH-LOOP/.test(x));
      vscode.debug.addBreakpoints([new vscode.SourceBreakpoint(new vscode.Location(vscode.Uri.file(src), new vscode.Position(wl, 0)))]);
      const n0 = stops.length;
      step('attach to PID ' + cp.pid);
      const at = await vscode.commands.executeCommand('ht9045Designer.run.attach', { pid: cp.pid, program: exe, gdb });
      const hit = await waitFor(() => stops.length > n0, 20000);
      let aw = null;
      if (hit && vscode.debug.activeDebugSession) {
        try {
          const st = await vscode.debug.activeDebugSession.customRequest('stackTrace', { threadId: stops[stops.length - 1].threadId, startFrame: 0, levels: 5 });
          const f = st && st.stackFrames && st.stackFrames[0];
          aw = f ? { line: f.line, fn: f.name, file: f.source && path.basename(f.source.path || '') } : null;
        } catch (e) { aw = { err: e.message }; }
      }
      const stillRunning = cp.exitCode === null;
      ok(!!at && at.ok && at.cfg.request === 'attach' && !!aw && aw.line === wl + 1 && /waitLoop/.test(aw.fn || '') && stillRunning,
        'Attach to Process: the running "thr wait" attached (' + (at && at.cfg ? at.cfg.type : '?') + '), stopped in its loop on the breakpoint, not restarted', JSON.stringify({ ok: at && at.ok, type: at && at.cfg && at.cfg.type, aw, stillRunning }));
      try { await vscode.debug.stopDebugging(); } catch (e) { /* gone */ }
      await sleep(500);
      try { cp.kill(); } catch (e) { /* gone */ }
      vscode.debug.removeBreakpoints(vscode.debug.breakpoints);
    }
    tf.dispose(); sd.dispose();
  } catch (e) {
    ok(false, 'the test ran', String(e && e.stack || e));
  }
  out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  if (report) fs.writeFileSync(report, out.join('\n'), 'utf8');
  await vscode.commands.executeCommand('workbench.action.closeWindow');
};
