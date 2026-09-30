// AI(W906-STREAM-F2) 20261001: the Contact CT and Observer page polls follow the stage-2E rule (RULINGS_20260930 #12):
//   the frame's WIN_STATE decides -- open / minimized = poll, closed / never = stop. The old test (frameElement
//   getClientRects) also stopped a MINIMIZED window; golden keeps both forms running when minimized (TfContactCT bShow and
//   TfObserver Timer1 change only in FormShow / FormClose). St01 1001 00:39 FYI 3.
//   Cuts the real visible() out of web/page/ht9045_contactct_wire.js and ht9045_observer_wire.js into a node vm with a
//   fake window / frameElement / parent.WIN_STATE. Also pins background.html's early windowState call (FYI 2).
//   Control: W906_S2E_PAGE_DIR (or argv[2]) -> the pre-change page directory must be red.
//   Offline: no wb_serve, no machine files. Usage: through ctest (Stream2E_St01Polls).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_S2E_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let passed = 0, failed = 0;
function check(name, cond, detail) {
  if (cond) { passed++; console.log('PASS ' + name); }
  else { failed++; console.log('FAIL ' + name + (detail ? ' -- ' + detail : '')); }
}

function cutVisible(file) {
  const t = fs.readFileSync(path.join(PAGE_DIR, file), 'utf8').replace(/\r/g, '');
  const s = t.indexOf('  function visible() {');
  if (s < 0) throw new Error('visible() not found in ' + file);
  const e = t.indexOf('\n  }\n', s);
  if (e < 0) throw new Error('end of visible() not found in ' + file);
  return t.slice(s, e + 4);
}

// state: undefined = no frame (standalone page); 'nowin' = a frame outside any .win; 'throw' = parent unreadable;
//   else the WIN_STATE of the window around the iframe. display:none (so getClientRects() = []) unless open.
function run(code, state, hidden) {
  const ctx = { document: { hidden: !!hidden }, String, Object };
  ctx.window = ctx;
  if (state === 'throw') {
    Object.defineProperty(ctx, 'frameElement', { get() { throw new Error('SecurityError'); } });
  } else if (state === 'nowin') {
    ctx.frameElement = { closest: () => null, getClientRects: () => [] };
    ctx.parent = {};
  } else if (state !== undefined) {
    const win = { id: 'win-contactct' };
    ctx.frameElement = { closest: (sel) => (sel === '.win' ? win : null), getClientRects: () => (state === 'open' ? [{}] : []) };
    ctx.parent = { WIN_STATE: { contactct: state } };
  }
  vm.createContext(ctx);
  vm.runInContext(code + '\n;this.__v = visible();', ctx);
  return ctx.__v;
}

for (const file of ['ht9045_contactct_wire.js', 'ht9045_observer_wire.js']) {
  let code;
  try { code = cutVisible(file); } catch (e) { check(file + ': visible() found', false, e.message); continue; }
  check(file + ': open -> polls', run(code, 'open') === true);
  check(file + ': minimized -> polls (golden keeps the form running)', run(code, 'minimized') === true);
  check(file + ': closed -> stops', run(code, 'closed') === false);
  check(file + ': never opened -> stops', run(code, 'never') === false);
  check(file + ': browser tab in the background -> stops', run(code, 'open', true) === false);
  check(file + ': standalone page (no frame) -> polls', run(code, undefined) === true);
  check(file + ': a frame outside any .win, display:none -> stops (old display test)', run(code, 'nowin') === false);
  check(file + ': parent not readable -> polls, nothing throws', run(code, 'throw') === true);
}

const bg = fs.readFileSync(path.join(PAGE_DIR, '..', 'background.html'), 'utf8');
const line = bg.split(/\r?\n/).find((l) => l.indexOf("WIN_STATE[cfg.id]=cfg.hidden?'never':'open';") >= 0) || '';
const code = line.split('//')[0];
check('background.html: the WIN_STATE start line tells the hub at iframe creation (before the first // on that line)',
      code.indexOf('HT9045Link.windowState(fWS.contentWindow, !cfg.hidden)') >= 0);
const sd = fs.readFileSync(path.join(PAGE_DIR, 'ht9045_smartdiag_web.js'), 'utf8');
check('ht9045_smartdiag_web.js keeps golden SmartDiagnosticTimer (runs while recording, not while shown)',
      sd.indexOf("tick = setInterval(function () { if (!document.hidden) op({ act: 'timer' }, true); }, 1000);") >= 0);

console.log('\n' + passed + ' passed, ' + failed + ' failed');
process.exit(failed ? 1 : 0);
