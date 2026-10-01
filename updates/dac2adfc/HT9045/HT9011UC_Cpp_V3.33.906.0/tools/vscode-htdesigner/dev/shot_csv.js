// AI(W906-HTDESIGNER) 20261001: a screenshot of the CSV table (media/csv.js) in headless Edge, in VS Code's dark
// colours, with a small Mot_Table-like file -- optionally with the right-click menu open (Down twice: an item lit).
// usage (VS Code's Electron as Node):  set ELECTRON_RUN_AS_NODE=1 & Code.exe dev\shot_csv.js <out.png> [menu]
// Writes only under %TEMP% and <out.png>.
'use strict';
const fs = require('fs'), path = require('path'), os = require('os'), cp = require('child_process');
const media = path.join(__dirname, '..', 'media'), outPng = process.argv[2], withMenu = process.argv[3] === 'menu';
const edgeExe = ['C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'].find(p => fs.existsSync(p));
if (!edgeExe || !outPng) { console.error('usage: shot_csv.js <out.png> [menu]  (needs msedge.exe)'); process.exit(2); }
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'htd_csv_shot_'));
const u = p => 'file:///' + path.resolve(p).replace(/\\/g, '/');
const vars = '--vscode-font-family:"Segoe UI","Microsoft JhengHei",sans-serif;--vscode-font-size:13px;--vscode-foreground:#cccccc;' +
  '--vscode-descriptionForeground:#9d9d9d;--vscode-editor-background:#1e1e1e;--vscode-editor-font-family:Consolas,monospace;' +
  '--vscode-input-background:#3c3c3c;--vscode-input-foreground:#cccccc;--vscode-input-border:#3c3c3c;--vscode-focusBorder:#007fd4;' +
  '--vscode-panel-border:#3c3c3c;--vscode-button-secondaryBackground:#3a3d41;--vscode-button-secondaryForeground:#cccccc;' +
  '--vscode-menu-background:#252526;--vscode-menu-foreground:#cccccc;--vscode-menu-border:#454545;--vscode-menu-selectionBackground:#04395e;' +
  '--vscode-menu-selectionForeground:#ffffff;--vscode-menu-separatorBackground:#454545;--vscode-list-hoverBackground:#2a2d2e;';
const rows = [['Motorname', 'Alias', 'Acc', 'Rate'], ['M00', 'MInArmX', '100000', '1'], ['M01', 'MInArmY', '200000', '2'], ['M02', 'MInArmZ', '300000', '']];
const page = path.join(work, 'csv.html');
fs.writeFileSync(page, '<!DOCTYPE html><html><head><meta charset="UTF-8"><style>:root{' + vars + '} body{margin:0;background:#1e1e1e;color:#cccccc;' +
  'font-family:var(--vscode-font-family);font-size:13px;}</style><link rel="stylesheet" href="' + u(path.join(media, 'csv.css')) + '"></head><body><div id="root"></div>' +
  '<script>window.acquireVsCodeApi=function(){return{postMessage:function(){},getState:function(){return null;},setState:function(){}};};</script>' +
  '<script src="' + u(path.join(media, 'csv.js')) + '"></script>' +
  '<script>setTimeout(function(){window.postMessage({type:"data",rows:' + JSON.stringify(rows) + ',delim:",",readOnly:false,file:"Mot_Table.csv",first:true},"*");},50);</script></body></html>');
const prof = path.join(work, 'prof');
const edge = cp.spawn(edgeExe, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--user-data-dir=' + prof,
  '--allow-file-access-from-files', '--remote-debugging-port=0', '--window-size=900,600', 'about:blank'], { stdio: 'ignore' });
const sleep = ms => new Promise(r => setTimeout(r, ms));
const end = code => { try { edge.kill(); } catch (e) { } setTimeout(() => { try { fs.rmSync(work, { recursive: true, force: true }); } catch (e) { } process.exit(code); }, 400); };
setTimeout(() => { console.error('timeout'); end(3); }, 30000);
(async () => {
  let port = 0;
  for (let i = 0; i < 150 && !port; i++) { await sleep(100); try { port = +fs.readFileSync(path.join(prof, 'DevToolsActivePort'), 'utf8').split('\n')[0]; } catch (e) { } }
  if (!port) throw new Error('no DevToolsActivePort');
  const t = await (await fetch('http://127.0.0.1:' + port + '/json/new?' + encodeURI(u(page)), { method: 'PUT' })).json();
  const ws = new WebSocket(t.webSocketDebuggerUrl);
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
  let id = 0;
  const wait = new Map();
  ws.onmessage = ev => { const m = JSON.parse(ev.data); if (m.id && wait.has(m.id)) { wait.get(m.id)(m); wait.delete(m.id); } };
  const call = (method, params) => new Promise(res => { const i = ++id; wait.set(i, res); ws.send(JSON.stringify({ id: i, method, params: params || {} })); });
  const ev = async expr => { const r = await call('Runtime.evaluate', { expression: expr, returnByValue: true }); return r.result && r.result.result ? r.result.result.value : null; };
  // (AI 20261001: Edge 154 headless stalls a tab nobody talks to -- to the front, then poll; see test\csv_realinput.js)
  await call('Page.bringToFront');
  for (let i = 0; i < 200; i++) { await sleep(100); if (await ev('document.readyState === "complete" && !!document.querySelector(".cell")')) break; }
  if (withMenu) {
    const p = await ev('(function(){var b=document.querySelector(".cell[data-r=\\"2\\"][data-c=\\"1\\"]").getBoundingClientRect();return {x:b.left+b.width/2,y:b.top+b.height/2};})()');
    await call('Input.dispatchMouseEvent', { type: 'mousePressed', x: p.x, y: p.y, button: 'right', buttons: 2, clickCount: 1 });
    await call('Input.dispatchMouseEvent', { type: 'mouseReleased', x: p.x, y: p.y, button: 'right', buttons: 0, clickCount: 1 });
    await sleep(150);
    for (let i = 0; i < 2; i++) {
      await call('Input.dispatchKeyEvent', { type: 'rawKeyDown', key: 'ArrowDown', code: 'ArrowDown', windowsVirtualKeyCode: 40 });
      await call('Input.dispatchKeyEvent', { type: 'keyUp', key: 'ArrowDown', code: 'ArrowDown', windowsVirtualKeyCode: 40 });
    }
    await sleep(100);
  }
  const shot = await call('Page.captureScreenshot', { format: 'png', clip: { x: 0, y: 0, width: 900, height: withMenu ? 460 : 220, scale: 1 } });
  fs.writeFileSync(outPng, Buffer.from(shot.result.data, 'base64'));
  ws.close();
  end(0);
})().catch(e => { console.error(String(e && e.stack || e)); end(1); });
