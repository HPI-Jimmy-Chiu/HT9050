'use strict';
// AI(W906-HTDESIGNER) 20261001: the run half of e2e_build -- against a running wb_serve (the 9050 build that has
// what e2e_build.js added), do what the page's htdCpp line does on a click / key / change: WS htd.event with the
// page's tag and {form, handler, control, event, ...}. Each ack is recorded; e2e_build.ps1 then matches the
// handlers' marker lines in wb_serve's stderr ("HTD-E2E TfHotPlate::Edit1KeyDown").
//   argv: <port> <e2eReport.json> <out.json>
// Needs Node 22+ (WebSocket): run with VS Code's Electron (ELECTRON_RUN_AS_NODE=1).
const fs = require('fs');

const [port, repFile, outFile] = process.argv.slice(2);
const R = JSON.parse(fs.readFileSync(repFile, 'utf8'));
const out = { port: +port, sent: [], acks: [], errors: [] };
const sleep = ms => new Promise(r => setTimeout(r, ms));

async function main() {
  let ws = null;
  for (let i = 0; i < 60 && !ws; i++) {
    try {
      ws = await new Promise((res, rej) => {
        const s = new WebSocket('ws://127.0.0.1:' + port + '/ht9045');
        s.onopen = () => res(s);
        s.onerror = e => rej(e);
      });
    } catch (e) { ws = null; await sleep(1000); }
  }
  if (!ws) throw new Error('no WS on port ' + port);
  let nextId = 1;
  const pending = new Map();
  ws.onmessage = ev => {
    let m;
    try { m = JSON.parse(ev.data); } catch (e) { return; }
    if (m && m.type === 'ack' && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
  };
  const cmd = (name, extra) => new Promise(res => {
    const id = nextId++;
    const msg = Object.assign({ type: 'cmd', id, cmd: name }, extra || {});
    pending.set(id, res);
    ws.send(JSON.stringify(msg));
    setTimeout(() => { if (pending.has(id)) { pending.delete(id); res({ id, ok: false, error: 'no ack in 8 s' }); } }, 8000);
  });
  out.acquire = await cmd('control.acquire');
  const evs = (R.events || []).filter(e => e.wired && !e.reset);
  for (const e of evs) {
    const v = { form: e.form, handler: e.handler, control: e.control, event: e.event, button: 0, x: 3, y: 4,
      shift: false, ctrl: false, alt: false, dbl: /DblClick$/.test(e.event), key: /^OnKey/.test(e.event) ? 65 : 0, chr: e.event === 'OnKeyPress' ? 97 : 0 };
    const a = await cmd('htd.event', { tag: e.tag, value: JSON.stringify(v) });
    out.sent.push({ form: e.form, handler: e.handler, control: e.control, event: e.event, reused: !!e.reused });
    out.acks.push({ handler: e.form + '::' + e.handler, control: e.control, event: e.event, ok: !!a.ok, error: a.error || '', value: typeof a.value === 'string' ? a.value.slice(0, 160) : '' });
    await sleep(30);   // (the server's WebCmdGuard: no burst)
  }
  try { ws.close(); } catch (e) { /* closing */ }
}

main().then(() => { fs.writeFileSync(outFile, JSON.stringify(out, null, 1), 'utf8'); process.exit(0); }, e => {
  out.errors.push(String(e && e.stack || e));
  fs.writeFileSync(outFile, JSON.stringify(out, null, 1), 'utf8');
  process.exit(2);
});
