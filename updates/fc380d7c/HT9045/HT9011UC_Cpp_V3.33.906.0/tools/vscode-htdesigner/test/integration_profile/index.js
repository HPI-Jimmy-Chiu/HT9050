'use strict';
// AI(W906-HTDESIGNER) 20261008 (full test, audit B5): in a real VS Code started with or without --profile, the BCB keys
// go into the keybindings.json of the profile the window runs (an extension's globalStorage is the default profile's
// even in another one), and the probe value is gone from the settings afterwards. Launched by test\vscode_profile_it.ps1.
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

exports.run = async function () {
  const out = [];
  let pass = 0, fail = 0;
  const ok = (c, n, x) => { if (c) pass++; else fail++; out.push((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); };
  try {
    out.push('vscode ' + vscode.version + '  profile ' + (process.env.HTD_PROFILE || '(default)'));
    const api = await vscode.extensions.getExtension('ht9045.ht9045-html-designer').activate();
    const cc = api.hub.cppChecks;
    for (let i = 0; i < 60 && !cc.profileDirDone; i++) await new Promise(r => setTimeout(r, 250));
    await new Promise(r => setTimeout(r, 2500));
    const user = path.join(process.env.HTD_USER_DIR, 'User');
    const kbFiles = [];
    const walk = d => { let es = []; try { es = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; } for (const e of es) { const p = path.join(d, e.name); if (e.isDirectory()) walk(p); else if (e.name === 'keybindings.json') kbFiles.push(p); } };
    walk(user);
    const probe = vscode.workspace.getConfiguration('ht9045Designer').inspect('profileProbe');
    const want = process.env.HTD_PROFILE ? /[\\/]User[\\/]profiles[\\/][^\\/]+[\\/]keybindings\.json$/i : /[\\/]User[\\/]keybindings\.json$/i;
    const rel = kbFiles.map(f => path.relative(user, f));
    ok(kbFiles.length === 1 && want.test(kbFiles[0]) && /ht9045Designer\.bcbDebugKeys/.test(fs.readFileSync(kbFiles[0], 'utf8')),
      'the BCB keys are in the keybindings.json of the profile this window runs, and only there', rel.join(', ') + '  userDir=' + path.relative(user, cc.userDir || ''));
    ok(!probe || probe.globalValue === undefined, 'the profile probe is gone from the user settings again', JSON.stringify(probe && probe.globalValue));
  } catch (e) {
    ok(false, 'the test ran', String(e && e.stack || e));
  }
  out.push('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  fs.writeFileSync(process.env.HTD_IT_REPORT, out.join('\n'), 'utf8');
  await vscode.commands.executeCommand('workbench.action.closeWindow');
};
