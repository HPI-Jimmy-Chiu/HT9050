// tools/webprobe/w152_contact_reread_selftest.cjs -- W-152 (B): the Contact page re-reads itself after a contact / auto-height run.
// AI(W906-W152) 20261007 (St02-E).  ctest St02_W152ContactRereadPage.  argv[2] = web/page.  Read-only, no browser, no server.
// Runs the W-152 block at the end of ht9045_contact_ev.js against fake HT9045Tags / HT9045Wire / HT9045EvB3Contact:
//   [1] the first contact.runResultSeq value (the snapshot when the page opens) does not re-read;
//   [2] a new value re-reads exactly once through HT9045Wire.reload and tells the operator (not saved);
//   [3] the same value again does nothing;
//   [4] while a form.event is queued / in flight the re-read waits, then runs once when the page is idle;
//   [5] source pins: the engine exports reload = its own load(); wb_serve stages the tag only while the Contact page is open.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let pass = 0, fail = 0;
function check(ok, what) { if (ok) { pass++; console.log('  PASS: ' + what); } else { fail++; console.log('  FAIL: ' + what); } }

const ev = fs.readFileSync(path.join(pageDir, 'ht9045_contact_ev.js'), 'utf8');
const marker = "// AI(W906-W152) 20261007 (St02-E; stand-in claim for St01";
const at = ev.indexOf(marker);
check(at > 0, '[0] ht9045_contact_ev.js carries the W-152 block');
const block = at > 0 ? ev.slice(ev.lastIndexOf('\n', at - 2) + 1) : '';

// fake globals
const subs = {};
let reloads = 0, says = [], queue = [], inflight = 0;
const timers = [];
const win = {
  HT9045Tags: { on: (tag, fn) => { (subs[tag] = subs[tag] || []).push(fn); return () => {}; } },
  HT9045Wire: { reload: () => { reloads++; return Promise.resolve({}); }, say: (t) => { says.push(t); } },
  HT9045EvB3Contact: { queue: () => queue.slice(), inflight: () => inflight },
  console: console,
};
const ctx = vm.createContext({ window: win, Promise, console, setTimeout: (fn) => { timers.push(fn); return timers.length; } });
vm.runInContext(block, ctx, { filename: 'ht9045_contact_ev.js#W152' });
function fire(v) { (subs['contact.runResultSeq'] || []).forEach((f) => f(v)); }
function flushTimers() { const due = timers.splice(0, timers.length); due.forEach((fn) => fn()); }   // one 300 ms step: a busy page re-arms
const tick = () => new Promise((r) => setImmediate(r));

(async () => {
  check((subs['contact.runResultSeq'] || []).length === 1, '[0] subscribed to tag contact.runResultSeq once');
  fire(4);
  await tick();
  check(reloads === 0, '[1] the first value (page-open snapshot) does not re-read');
  fire(5);
  await tick();
  check(reloads === 1 && says.some((s) => s.indexOf('尚未存檔') >= 0), '[2] a new run result re-reads once through HT9045Wire.reload and says it is not saved');
  fire(5);
  await tick();
  check(reloads === 1, '[3] the same value again does nothing');
  queue = [{ control: 'rbAutoHeight' }];
  fire(6);
  await tick();
  check(reloads === 1 && timers.length === 1, '[4] a form.event is queued -> the re-read waits');
  queue = []; inflight = 1;
  flushTimers();
  await tick();
  check(reloads === 1, '[4] still in flight -> still waiting');
  inflight = 0;
  flushTimers();
  await tick();
  check(reloads === 2, '[4] idle -> exactly one re-read');

  const eng = fs.readFileSync(path.join(pageDir, 'ht9045_wire_engine.js'), 'utf8');
  check(/window\.HT9045Wire = \{ register: register, say: say, reload: function \(\) \{ return load\(\); \} \};/.test(eng),
        '[5] the engine exports reload = its own load()');
  const ws = fs.readFileSync(path.join(pageDir, '..', '..', 'HT9011UC_Cpp_V3.33.906.0', 'tools', 'wb_serve.cpp'), 'utf8');
  check(ws.indexOf('{ if (::W906_PageStreamWanted("contact")) { snap.stage("contact.runResultSeq", webbridge::TagValue::makeInt(::W906_ContactRunResultSeq())); ++n; } }') > 0,
        '[5] wb_serve stages contact.runResultSeq only while the Contact page is open (webId "contact")');
  console.log('St02_W152ContactRereadPage: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})();
