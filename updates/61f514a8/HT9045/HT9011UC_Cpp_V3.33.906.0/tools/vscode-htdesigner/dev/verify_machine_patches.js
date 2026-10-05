// AI(W906-HTDESIGNER) 20261005 (ES02): after taking in machine (MC01) patches, run this: every line a patch added and no later
// patch removed must be here. (0143's .claude skip had been half taken in and nothing else showed it.)
// Usage: node dev/verify_machine_patches.js [fromPatchNo]   (PUSH = a GitHub HT9050 clone with origin/machine/integ-ioweb)
// Lines the machine still has at the end (added by patch N, not removed by any later patch) that our tree lacks.
const fs = require('fs'), path = require('path'), cp = require('child_process');
const PUSH = process.env.HTD_PUSH || 'D:/HT9050/htd_push';
const ROOT = require('path').resolve(__dirname, '..', '..', '..');
const from = +(process.argv[2] || 100);
const list = cp.execSync('git ls-tree -r --name-only origin/machine/integ-ioweb', { cwd: PUSH, encoding: 'utf8' })
  .split('\n').filter(f => /^tools\/0\d{3}-.*\.patch$/.test(f)).sort();
const live = {};   // rel -> Map(line -> patchNo)
for (const f of list) {
  const n = +/^tools\/(\d{4})/.exec(f)[1];
  const p = cp.execSync('git show "origin/machine/integ-ioweb:' + f + '"', { cwd: PUSH, encoding: 'utf8', maxBuffer: 1 << 28 }).replace(/\r\n/g, '\n');
  let cur = null;
  for (const l of p.split('\n')) {
    let m = /^\+\+\+ b\/(.*)$/.exec(l); if (m) { cur = m[1]; live[cur] = live[cur] || new Map(); continue; }
    if (/^\+\+\+ /.test(l) || /^--- /.test(l)) continue;
    if (/^diff --git/.test(l)) { cur = null; continue; }
    if (!cur) continue;
    if (l.startsWith('-')) live[cur].delete(l.slice(1));
    else if (l.startsWith('+')) live[cur].set(l.slice(1), n);
  }
}
let miss = 0;
for (const [rel, mp] of Object.entries(live)) {
  if (/(CHANGELOG|WPF_DIFF_LOG|HANDOVER|README|CHEATSHEET)\.md$|package-lock|\/dev\//.test(rel)) continue;
  const fp = path.join(ROOT, rel);
  if (!fs.existsSync(fp)) { console.log('MISSING FILE ' + rel); continue; }
  const t = fs.readFileSync(fp, 'utf8').replace(/\r\n/g, '\n');
  const ms = [...mp].filter(([l, n]) => n >= from && l.trim().length > 3 && !t.includes(l));
  if (ms.length) { miss += ms.length; console.log(rel + ': ' + ms.length); ms.forEach(([l, n]) => console.log('   ' + n + ' | ' + l.trim().slice(0, 120))); }
}
console.log('still-live machine lines missing here: ' + miss);
