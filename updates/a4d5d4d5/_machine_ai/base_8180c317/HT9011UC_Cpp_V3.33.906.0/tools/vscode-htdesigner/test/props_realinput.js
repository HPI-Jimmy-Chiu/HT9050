// AI(W906-HTDESIGNER) 20261001: the properties panel (media/props.js) in a headless Edge with REAL input (the DevTools
// protocol's Input.*), on the data layer 3 wrote (%TEMP%\htdesigner_panels\props_spbSave.json, an Alignment drop-down
// added): what synthetic events cannot show --
//   the wheel over a focused number field changed it (Left 54 -> 52, written on leaving);
//   Up / Down on a focused drop-down changed it and wrote it (Alignment -> taCenter);
//   a real double click on a True / False box.
// usage: Code.exe props_realinput.js <media dir> <props_spbSave.json> <out.json>   (with ELECTRON_RUN_AS_NODE=1)
// Writes only under %TEMP% (the page and the Edge profile) and <out.json>.
'use strict';
const fs = require('fs'), path = require('path'), os = require('os'), cp = require('child_process');
const [media, dataFile, outFile] = process.argv.slice(2);
const R = {};
const done = () => { try { fs.writeFileSync(outFile, JSON.stringify(R)); } catch (e) { } };
const edgeExe = ['C:/Program Files (x86)/Microsoft/Edge/Application/msedge.exe', 'C:/Program Files/Microsoft/Edge/Application/msedge.exe'].find(p => fs.existsSync(p));
if (!edgeExe) { R.error = 'msedge.exe not found'; done(); process.exit(2); }
const work = fs.mkdtempSync(path.join(os.tmpdir(), 'htd_props_real_'));
const u = p => 'file:///' + path.resolve(p).replace(/\\/g, '/');
const d = JSON.parse(fs.readFileSync(dataFile, 'utf8'));
const ed = d.edit || (d.data && d.data.edit);
if (ed && ed.look && !ed.look.alignment) ed.look.alignment = 'taLeftJustify';   // (a drop-down to try the arrows on)
const data = JSON.stringify(d).replace(/<\//g, '<\\/');
const page = path.join(work, 'props.html');
fs.writeFileSync(page, '<!DOCTYPE html><html><head><meta charset="UTF-8"><link rel="stylesheet" href="' + u(path.join(media, 'props.css')) + '">' +
  '<script>window.__out=[];window.__err=[];window.addEventListener("error",function(e){window.__err.push(String(e.message));});' +
  'window.acquireVsCodeApi=function(){return Object.freeze({postMessage:function(m){window.__out.push(JSON.parse(JSON.stringify(m)));},getState:function(){return null;},setState:function(){}});};</script>' +
  '</head><body><div id="root"></div><script src="' + u(path.join(media, 'props.js')) + '"></script>' +
  '<script>var DATA=' + data + ';window.postMessage({type:"show",data:DATA},"*");</script></body></html>');
const prof = path.join(work, 'prof');
const edge = cp.spawn(edgeExe, ['--headless=new', '--disable-gpu', '--no-first-run', '--no-default-browser-check', '--user-data-dir=' + prof,
  '--allow-file-access-from-files', '--remote-debugging-port=0', '--window-size=520,900', 'about:blank'], { stdio: 'ignore' });
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
  const center = sel => ev('(function(){var e=document.querySelector(' + JSON.stringify(sel) + ');if(!e)return null;e.scrollIntoView({block:"center"});var b=e.getBoundingClientRect();return {x:b.left+b.width/2,y:b.top+b.height/2};})()');
  const press = async (p, n) => {
    await call('Input.dispatchMouseEvent', { type: 'mousePressed', x: p.x, y: p.y, button: 'left', buttons: 1, clickCount: n || 1 });
    await call('Input.dispatchMouseEvent', { type: 'mouseReleased', x: p.x, y: p.y, button: 'left', buttons: 0, clickCount: n || 1 });
    await sleep(70);
  };
  const key = async (k, code, vk) => { await call('Input.dispatchKeyEvent', { type: 'rawKeyDown', key: k, code, windowsVirtualKeyCode: vk }); await call('Input.dispatchKeyEvent', { type: 'keyUp', key: k, code, windowsVirtualKeyCode: vk }); await sleep(80); };
  const sent = n0 => ev('window.__out.slice(' + n0 + ').map(function(m){return m.type+(m.prop?":"+m.prop:"")+(m.left!=null?":left="+m.left:"")+(m.value!=null?"="+m.value:"");})');
  const field = f => 'document.querySelector(\'[data-field="' + f + '"]\')';
  const focusedField = () => ev('document.activeElement && document.activeElement.getAttribute("data-field")');
  await sleep(1200);
  // 1. the wheel over the focused Left field: the panel scrolls, the value stays, nothing written
  const lf = await center('input[data-field="left"]');
  if (lf) {
    await press(lf);
    const v0 = await ev(field('left') + '.value'), n0 = await ev('window.__out.length');
    for (let i = 0; i < 2; i++) { await call('Input.dispatchMouseEvent', { type: 'mouseWheel', x: lf.x, y: lf.y, deltaX: 0, deltaY: 120 }); await sleep(150); }
    await sleep(500);
    const v1 = await ev(field('left') + '.value');
    await ev(field('left') + '.blur()');
    await sleep(300);
    R.wheel = { before: v0, after: v1, sent: await sent(n0) };
    R.wheelOk = v0 === v1 && R.wheel.sent.length === 0;
  }
  // 2. Up / Down on the focused Alignment drop-down: to the row below / above, the value stays, nothing written
  if (await ev('!!' + field('alignment'))) {
    await center('select[data-field="alignment"]');
    await ev(field('alignment') + '.focus()');
    const s0 = await ev(field('alignment') + '.value'), n1 = await ev('window.__out.length');
    await key('ArrowDown', 'ArrowDown', 40);
    const f1 = await focusedField();
    await ev(field('alignment') + '.focus()');
    await key('ArrowUp', 'ArrowUp', 38);
    const f2 = await focusedField();
    const s1 = await ev(field('alignment') + '.value');
    R.selectArrow = { before: s0, after: s1, sent: await sent(n1), down: f1, up: f2 };
    // (Font.Name below keeps Up / Down for its suggestion list: it is only where Down lands here)
    R.selectArrowOk = s0 === s1 && R.selectArrow.sent.length === 0 && !!f1 && !!f2 && f1 !== 'alignment' && f2 !== 'alignment' && f1 !== f2;
  }
  // 3. a real double click on the Bold box: switched once
  const bp = await center('input[data-field="bold"]');
  if (bp) {
    const b0 = await ev(field('bold') + '.checked'), n2 = await ev('window.__out.length');
    await press(bp, 1);
    await press(bp, 2);
    await sleep(200);
    const bs = (await sent(n2)).filter(x => /^setLook:bold/.test(x));
    R.dblBold = { before: b0, after: await ev(field('bold') + '.checked'), sent: bs };
    R.dblBoldOk = bs.length === 1 && R.dblBold.after !== b0;
  }
  R.pageErrors = await ev('window.__err');
  ws.close();
  await finish(0);
})().catch(async e => { R.error = String(e && e.stack || e); await finish(1); });
