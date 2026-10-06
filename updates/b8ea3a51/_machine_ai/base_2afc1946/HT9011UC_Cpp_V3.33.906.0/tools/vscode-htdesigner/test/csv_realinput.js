// AI(W906-HTDESIGNER) 20261001: the CSV table (media/csv.js) in a headless Edge driven with REAL input --
// the DevTools protocol's Input.* (a real double click, real keys, an IME composition) -- not synthetic
// events. The synthetic test (csv_driver.js) passed while EastSun could not write a cell (20261001
// "表格我沒辦法寫入"): a dispatched dblclick hits the cell, a real one never did; a dispatched keydown
// does not need the IME. Run by test\probe_test.ps1 with VS Code's Electron as Node (global fetch / WebSocket).
// usage: Code.exe csv_realinput.js <media dir> <out.json>   (with ELECTRON_RUN_AS_NODE=1)
// Writes only under %TEMP% (the page and the Edge profile) and <out.json>.
'use strict';
const fs = require('fs'), path = require('path'), os = require('os'), cp = require('child_process');
const media = process.argv[2], outFile = process.argv[3];
const R = {};
const done = () => { try { fs.writeFileSync(outFile, JSON.stringify(R)); } catch (e) { } };
const edgeExe = ['C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'].find(p => fs.existsSync(p));
if (!edgeExe) { R.error = 'msedge.exe not found'; done(); process.exit(2); }
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'htd_csv_real_'));
const u = p => 'file:///' + path.resolve(p).replace(/\\/g, '/');
const page = path.join(work, 'csv.html');
fs.writeFileSync(page, '<!DOCTYPE html><html><head><meta charset="UTF-8">' +
  '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src file: \'unsafe-inline\'; script-src file: \'unsafe-inline\'">' +
  '<link rel="stylesheet" href="' + u(path.join(media, 'csv.css')) + '"></head><body><div id="root"></div>' +
  '<script>window.__out=[];window.acquireVsCodeApi=function(){return Object.freeze({postMessage:function(m){window.__out.push(JSON.parse(JSON.stringify(m)));},getState:function(){return null;},setState:function(){}});};</script>' +
  '<script src="' + u(path.join(media, 'csv.js')) + '"></script>' +
  '<script>setTimeout(function(){window.postMessage({type:"data",rows:[["Motorname","Alias","Acc","Rate"],["M00","MInArmX","100000","1"],["M01","MInArmY","200000","2"],["M02","MInArmZ","300000"]],delim:",",readOnly:false,file:"Mot_Table.csv",first:true},"*");},50);</script>' +
  '</body></html>');
const prof = path.join(work, 'prof');
const edge = cp.spawn(edgeExe, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--user-data-dir=' + prof,
  '--allow-file-access-from-files', '--remote-debugging-port=0', '--window-size=1000,700', 'about:blank'], { stdio: 'ignore' });
const sleep = ms => new Promise(r => setTimeout(r, ms));
let finished = false;
const finish = async code => {
  if (finished) return;
  finished = true;
  done();
  try { edge.kill(); } catch (e) { }
  await sleep(400);
  try { fs.rmSync(work, { recursive: true, force: true }); } catch (e) { }
  process.exit(code);
};
setTimeout(() => { R.error = R.error || 'timeout'; finish(3); }, 40000);
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
  const center = async (sel) => ev('(function(){var e=document.querySelector(' + JSON.stringify(sel) + ');if(!e)return null;var b=e.getBoundingClientRect();return {x:b.left+b.width/2,y:b.top+b.height/2};})()');
  const mouse = (p, type, n) => call('Input.dispatchMouseEvent', { type, x: p.x, y: p.y, button: 'left', buttons: type === 'mousePressed' ? 1 : 0, clickCount: n });
  const click = async (p, n) => { await mouse(p, 'mousePressed', n); await mouse(p, 'mouseReleased', n); await sleep(60); };
  // modifiers: 1 Alt, 2 Ctrl, 8 Shift
  const keyPress = async (key, code, vk, text, modifiers) => {
    const mod = modifiers || 0;
    await call('Input.dispatchKeyEvent', Object.assign({ type: text ? 'keyDown' : 'rawKeyDown', key, code, windowsVirtualKeyCode: vk, modifiers: mod }, text ? { text } : {}));
    await call('Input.dispatchKeyEvent', { type: 'keyUp', key, code, windowsVirtualKeyCode: vk, modifiers: mod });
    await sleep(40);
  };
  const ch = async c => keyPress(c, /\d/.test(c) ? 'Digit' + c : 'Key' + c.toUpperCase(), c.toUpperCase().charCodeAt(0), c);
  const clickCell = async (r, c, shift) => {
    const p = await center('.cell[data-r="' + r + '"][data-c="' + c + '"]');
    await call('Input.dispatchMouseEvent', { type: 'mousePressed', x: p.x, y: p.y, button: 'left', buttons: 1, clickCount: 1, modifiers: shift ? 8 : 0 });
    await call('Input.dispatchMouseEvent', { type: 'mouseReleased', x: p.x, y: p.y, button: 'left', buttons: 0, clickCount: 1, modifiers: shift ? 8 : 0 });
    await sleep(60);
  };
  const stateOf = () => ev('window.__htdCsv.state()');
  const lastMsg = type => ev('(function(){var a=window.__out.filter(function(m){return m.type===' + JSON.stringify(type) + ';});return a.length?a[a.length-1]:null;})()');
  const count = type => ev('window.__out.filter(function(m){return m.type===' + JSON.stringify(type) + ';}).length');
  const editorState = () => ev('(function(){var e=document.querySelector(".ed:not(.idle)");return e?{value:e.value,s:e.selectionStart,e:e.selectionEnd,focused:document.activeElement===e}:null;})()');
  const edits = () => ev('window.__out.filter(function(m){return m.type==="edit";}).map(function(m){return m.changes;})');
  // AI(W906-HTDESIGNER) 20261001: Edge 154 headless leaves a new tab nobody talks to at readyState "loading"
  // (csv.css / csv.js never fetched): a fixed 700 ms sleep then found no table. To the front first, then poll.
  await call('Page.bringToFront');
  for (let i = 0; i < 200; i++) { await sleep(100); if (await ev('document.readyState === "complete" && !!document.querySelector(".cell")')) break; }
  const c22 = await center('.cell[data-r="2"][data-c="2"]');
  if (!c22) throw new Error('the table was not drawn');
  await mouse(c22, 'mouseMoved', 0);
  // 1. a real double click opens the cell's editor, its text selected
  await click(c22, 1);
  await click(c22, 2);
  await sleep(100);
  R.dbl = await editorState();
  R.dblOk = !!(R.dbl && R.dbl.value === '200000' && R.dbl.s === R.dbl.e && R.dbl.focused);   // (Excel: the caret where it was clicked, nothing selected)
  await keyPress('Escape', 'Escape', 27);
  // 2. a click, then real keys: typed over the cell; Enter writes it (one edit), the next row
  await click(c22, 1);
  await keyPress('7', 'Digit7', 55, '7');
  await keyPress('8', 'Digit8', 56, '8');
  R.typed = await editorState();
  await keyPress('Enter', 'Enter', 13, '\r');
  R.typedEdits = await edits();
  R.typedOk = !!(R.typed && R.typed.value === '78' && JSON.stringify(R.typedEdits) === '[[{"r":2,"c":2,"v":"78"}]]' &&
    await ev('!document.querySelector(".ed:not(.idle)") && document.activeElement && document.activeElement.id === "kb"'));
  // 3. the IME (a Chinese input method): the key is "Process" (229), a word is composed, then committed
  const c31 = await center('.cell[data-r="3"][data-c="1"]');
  await click(c31, 1);
  await call('Input.dispatchKeyEvent', { type: 'rawKeyDown', key: 'Process', code: 'KeyJ', windowsVirtualKeyCode: 229 });
  await call('Input.imeSetComposition', { text: '\u3128', selectionStart: 1, selectionEnd: 1 });
  await call('Input.dispatchKeyEvent', { type: 'keyUp', key: 'Process', code: 'KeyJ', windowsVirtualKeyCode: 229 });
  await sleep(60);
  R.imeComposing = await editorState();
  await call('Input.insertText', { text: '\u4e2d' });
  await sleep(60);
  R.imeCommitted = await editorState();
  const nBefore = (await edits()).length;
  await keyPress('Enter', 'Enter', 13, '\r');
  const ed3 = (await edits()).slice(nBefore);
  R.imeEdits = ed3;
  R.imeOk = !!(R.imeComposing && R.imeCommitted && R.imeCommitted.value === '\u4e2d' && ed3.length === 1 &&
    JSON.stringify(ed3[0]) === JSON.stringify([{ r: 3, c: 1, v: '\u4e2d' }]));
  // 4. a click on the table's empty part: the keys still go to the input
  const below = await ev('(function(){var w=document.getElementById("wrap").getBoundingClientRect();return {x:w.left+300,y:w.bottom-20};})()');
  await click(below, 1);
  R.emptyClickFocus = await ev('document.activeElement && (document.activeElement.id || document.activeElement.tagName)');
  R.emptyClickOk = R.emptyClickFocus === 'kb';
  // 5. Excel (20261001): Ctrl+Enter = the typed value into every selected cell, the block and the active cell stay
  await clickCell(1, 3); await clickCell(3, 3, true);
  await ch('7');
  await keyPress('Enter', 'Enter', 13, '\r', 2);
  const ce = await lastMsg('edit'), cs = await stateOf();
  R.ctrlEnterOk = !!(ce && ce.changes.length === 3 && ce.changes.every(x => x.c === 3 && x.v === '7') && cs.act.r === 1 && cs.act.c === 3 && cs.cur.r === 3 && !cs.editing);
  // 6. Ctrl+D = the block's first row into the rest
  await clickCell(1, 1); await clickCell(2, 1, true);
  await keyPress('d', 'KeyD', 68, '', 2);
  const fd = await lastMsg('edit');
  R.fillDownOk = !!(fd && fd.changes.length === 1 && fd.changes[0].r === 2 && fd.changes[0].c === 1 && fd.changes[0].v === 'MInArmX');
  // 7. Shift+Space = the whole row (it used to type a space into the cell); 8. Ctrl+- deletes it only then
  await clickCell(2, 0);
  await keyPress(' ', 'Space', 32, ' ', 8);
  const sr = await stateOf();
  const kbVal = await ev('document.getElementById("kb").value');
  const del0 = await count('deleteRows');
  await keyPress('-', 'Minus', 189, '', 2);
  const del1 = await count('deleteRows'), dm = await lastMsg('deleteRows');
  await clickCell(1, 0);
  await keyPress('-', 'Minus', 189, '', 2);
  const del2 = await count('deleteRows');
  const warned = await ev('!document.getElementById("warn").hidden && /整列/.test(document.getElementById("warn").textContent)');
  R.rowKeysOk = !!(sr.anchor.r === 2 && sr.anchor.c === 0 && sr.cur.r === 2 && sr.cur.c === 3 && !sr.editing && kbVal === '' &&
    del1 === del0 + 1 && dm && dm.r === 2 && dm.count === 1 && del2 === del1 && warned);
  // 9. typed over: an arrow writes it and moves (all four, Enter mode)
  await clickCell(1, 0);
  await ch('q');
  await keyPress('ArrowRight', 'ArrowRight', 39);
  const ta = await lastMsg('edit'), ts = await stateOf();
  R.typedArrowOk = !!(ta && ta.changes.length === 1 && ta.changes[0].r === 1 && ta.changes[0].c === 0 && ta.changes[0].v === 'q' && ts.act.r === 1 && ts.act.c === 1 && !ts.editing);
  // 10. F2 = the caret at the end (not all selected); F2 again = Enter mode / Edit mode
  await clickCell(1, 2);
  await keyPress('F2', 'F2', 113);
  const f2 = await editorState(), f2s = await stateOf();
  await keyPress('F2', 'F2', 113);
  const f2t = await stateOf();
  await keyPress('Escape', 'Escape', 27);
  R.f2Ok = !!(f2 && f2.value === '100000' && f2.s === 6 && f2.e === 6 && f2s.editing && !f2s.typed && f2t.typed);
  // 11. Backspace = the active cell from empty; Esc gives it back (nothing written)
  const e11 = await count('edit');
  await keyPress('Backspace', 'Backspace', 8);
  const bs = await editorState();
  await keyPress('Escape', 'Escape', 27);
  R.backspaceOk = !!(bs && bs.value === '' && bs.focused && (await count('edit')) === e11 && !(await editorState()));
  // 12. Enter inside a selected block: the active cell moves (down, then the next column), the block stays
  await clickCell(1, 0); await clickCell(2, 1, true);
  await keyPress('Enter', 'Enter', 13, '\r');
  const en1 = await stateOf();
  await keyPress('Enter', 'Enter', 13, '\r');
  const en2 = await stateOf();
  R.enterInBlockOk = en1.act.r === 2 && en1.act.c === 0 && en2.act.r === 1 && en2.act.c === 1 && en2.anchor.r === 1 && en2.cur.r === 2 && en2.cur.c === 1;
  // 13. the empty row under the last one: typing there adds a row
  await clickCell(3, 3);
  await keyPress('ArrowDown', 'ArrowDown', 40);
  const ph = await stateOf();
  await ch('n');
  await keyPress('Enter', 'Enter', 13, '\r');
  const pe = await lastMsg('edit');
  R.newRowOk = !!(ph.act.r === 4 && pe && pe.changes.length === 1 && pe.changes[0].r === 4 && pe.changes[0].c === 3 && pe.changes[0].v === 'n');
  // 14. Ctrl+H: replace -- whole cell (7 -> X: the three 7s, not 100000 / 1), ONE edit
  await clickCell(1, 0);
  await keyPress('h', 'KeyH', 72, '', 2);
  const replShown = await ev('!document.getElementById("repl").hidden && document.activeElement && document.activeElement.id === "find"');
  await ch('7');
  await ev('(function(){var r=document.getElementById("replBox");r.focus();r.value="X";var w=document.getElementById("whole");w.checked=true;w.dispatchEvent(new Event("change"));})()');
  const e14 = await count('edit');
  await ev('document.getElementById("replAll").click()');
  const ra = await lastMsg('edit');
  R.replaceOk = !!(replShown && (await count('edit')) === e14 + 1 && ra && ra.changes.length === 3 && ra.changes.every(x => x.c === 3 && x.v === 'X'));
  // 15. the right-click menu (20261001, Excel's cell menu): a REAL right click opens it with that cell selected; real
  // Down, Down, Enter = Copy; a real click on 清除內容 = one edit; real Shift+F10 opens it, Esc closes it
  const rclick = async (r, c) => {
    const p = await center('.cell[data-r="' + r + '"][data-c="' + c + '"]');
    await call('Input.dispatchMouseEvent', { type: 'mousePressed', x: p.x, y: p.y, button: 'right', buttons: 2, clickCount: 1 });
    await call('Input.dispatchMouseEvent', { type: 'mouseReleased', x: p.x, y: p.y, button: 'right', buttons: 0, clickCount: 1 });
    await sleep(80);
  };
  const menuN = () => ev('(function(){var m=document.querySelector(".ctxmenu");return m?m.querySelectorAll(".ctxitem").length:0;})()');
  await rclick(2, 1);
  const rm = await menuN(), rs = await stateOf(), cp0 = await count('copy');
  await keyPress('ArrowDown', 'ArrowDown', 40); await keyPress('ArrowDown', 'ArrowDown', 40); await keyPress('Enter', 'Enter', 13, '\r');
  const cpm = await lastMsg('copy'), cp1 = await count('copy'), gone1 = await ev('!document.querySelector(".ctxmenu")');
  await rclick(2, 1);
  const e15 = await count('edit');
  const clr = await center('.ctxitem[data-item="清除內容"]');
  if (clr) await click(clr, 1);
  const ce15 = await lastMsg('edit');
  await keyPress('F10', 'F10', 121, '', 8);
  const sf = await menuN();
  await keyPress('Escape', 'Escape', 27);
  const sfGone = await ev('!document.querySelector(".ctxmenu") && document.activeElement && document.activeElement.id === "kb"');
  R.menuReal = { items: rm, act: rs.act, copy: cpm && cpm.grid, clear: ce15 && ce15.changes, shiftF10: sf };
  // (MInArmX: step 6's Ctrl+D put it there, the table shows its edits at once)
  R.menuRealOk = !!(rm === 12 && rs.act.r === 2 && rs.act.c === 1 && rs.anchor.r === 2 && cp1 === cp0 + 1 && cpm && JSON.stringify(cpm.grid) === '[["MInArmX"]]' && gone1 &&
    (await count('edit')) === e15 + 1 && ce15 && JSON.stringify(ce15.changes) === '[{"r":2,"c":1,"v":""}]' && sf === 12 && sfGone);
  // 16. the Name Box (Excel): real Ctrl+G, real keys "c3", Enter = C3 active, the keys back with the table
  await keyPress('g', 'KeyG', 71, '', 2);
  const nbF = await ev('document.activeElement && document.activeElement.id');
  await ch('c'); await ch('3');
  await keyPress('Enter', 'Enter', 13, '\r');
  const nbS = await stateOf(), nbAfter = await ev('document.activeElement && document.activeElement.id'), nbVal = await ev('document.getElementById("nameBox").value');
  R.nameBoxReal = { focus: nbF, act: nbS.act, after: nbAfter, value: nbVal };
  R.nameBoxRealOk = nbF === 'nameBox' && nbS.act.r === 2 && nbS.act.c === 2 && nbS.cur.r === 2 && nbAfter === 'kb' && nbVal === 'C3';
  // 17. AutoFit: a REAL double click on column B's boundary = as wide as its longest value (MInArmX / MInArmY)
  const bnd = await center('.hc[data-col="1"] .rs');
  const wB0 = await ev('Math.round(document.querySelector(\'.hc[data-col="1"]\').getBoundingClientRect().width)');
  await mouse(bnd, 'mouseMoved', 0);
  await click(bnd, 1); await click(bnd, 2);
  await sleep(80);
  const wB1 = await ev('Math.round(document.querySelector(\'.hc[data-col="1"]\').getBoundingClientRect().width)');
  const overB = await ev('Array.prototype.filter.call(document.querySelectorAll(\'.cell[data-c="1"]\'), function (x) { return x.scrollWidth > x.clientWidth + 1; }).length');
  R.autoFitReal = { before: wB0, after: wB1, over: overB };
  R.autoFitRealOk = typeof wB1 === 'number' && wB1 > 30 && wB1 < 200 && overB === 0;
  R.pageErrors = await ev('window.__errs || []');
  ws.close();
  await finish(0);
})().catch(async e => { R.error = String(e && e.stack || e); await finish(1); });
