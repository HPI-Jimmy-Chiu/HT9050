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
    if (st && ses) { try { await ses.customRequest('continue', { threadId: st.threadId }); } catch (e) { out.push('      continue: ' + e.message); } }
    const gone = await Promise.race([ended.then(() => true), sleep(30000).then(() => false)]);
    ok(gone && exited === 0, 'Continue runs it to its end (exit 0)', JSON.stringify({ gone, exited }));
    if (!gone) { try { await vscode.debug.stopDebugging(); } catch (e) { /* gone */ } }
    tf.dispose(); sd.dispose();
    vscode.debug.removeBreakpoints(vscode.debug.breakpoints);
  } catch (e) {
    ok(false, 'the test ran', String(e && e.stack || e));
  }
  out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  if (report) fs.writeFileSync(report, out.join('\n'), 'utf8');
  await vscode.commands.executeCommand('workbench.action.closeWindow');
};
