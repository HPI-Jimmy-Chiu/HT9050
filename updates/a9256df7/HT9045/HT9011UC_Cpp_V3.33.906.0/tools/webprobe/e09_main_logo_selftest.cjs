'use strict';
// =============================================================================
//  tools/webprobe/e09_main_logo_selftest.cjs -- ctest E09_MainLogo.  AI(W906-E09) 20261004 (St02-E).
//
//  Card E-09 (TO_ES02.md section 3; St02 since Jimmy 1004 20:3x), batch 1: on the main screen the moved logo block covered the
//  run-mode combo.  golden main.dfm:2198 imgLogo (TImage 143,314 202x111) and cbRunStartMode (TComboBox 189,227 156x24) on
//  palSetting never overlap, and a TComboBox paints above a TImage anyway.  On the web, ht9xxx-layout-base.js moves the whole
//  #imgLogo block up 156 px and its IMG back down 56 px: the picture sits where golden has it, but the block's empty area lies
//  over cbRunStartMode and takes its clicks (E-09 layout probe, docs/handoff/ST02_E09_LAYOUT_20261004.md).  Fix:
//  web/page/main.html:45 `#imgLogo{pointer-events:none;}` -- layout only (golden imgLogo has no OnClick; the web binds none).
//  Offline source checks (no browser):
//   1. main.html's <style> has exactly one `#imgLogo{pointer-events:none;}` rule
//   2. the run-mode combo and the logo block are both still there (ids cbRunStartMode / imgLogo)
//   3. nothing in web/page binds a click / mouse handler to imgLogo (the rule must not swallow a real action)
//  argv[2] = web/page.  CONTROL: W906_MAIN_HTML pointing at main.html before the fix must turn check 1 red.
// =============================================================================
const fs = require('fs');
const path = require('path');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const mainHtml = process.env.W906_MAIN_HTML || path.join(pageDir, 'main.html');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }

const html = fs.readFileSync(mainHtml, 'utf8');
const styles = (html.match(/<style[\s\S]*?<\/style>/gi) || []).join('\n');
const rule = /#imgLogo\s*\{\s*pointer-events\s*:\s*none\s*;?\s*\}/g;
console.log('-- 1. the logo block does not take the clicks of what lies under it');
check((styles.match(rule) || []).length === 1, 'main.html <style>: one #imgLogo{pointer-events:none;}');
console.log('-- 2. both elements still there');
check(/id="cbRunStartMode"/.test(html) && /<select[^>]*id="cbRunStartMode"/.test(html), 'main.html: <select id="cbRunStartMode">');
check((html.match(/id="imgLogo"/g) || []).length === 1, 'main.html: one element id="imgLogo"');
console.log('-- 3. no click / mouse handler on the logo anywhere in web/page');
const bad = [];
for (const f of fs.readdirSync(pageDir)) {
  if (!/\.(js|html)$/i.test(f)) continue;
  const t = fs.readFileSync(path.join(pageDir, f), 'utf8');
  const re = /imgLogo[\s\S]{0,120}?(addEventListener\(\s*['"](click|mousedown|mouseup|dblclick|pointerdown)|\.onclick\s*=|\.onmousedown\s*=)/g;
  if (re.test(t)) bad.push(f);
  if (/id="imgLogo"[^>]*\son(click|mousedown|dblclick)=/i.test(t)) bad.push(f + ' (inline)');
}
check(bad.length === 0, 'no handler bound to imgLogo' + (bad.length ? ': ' + bad.join(', ') : ''));

console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
process.exit(fail ? 1 : 0);
