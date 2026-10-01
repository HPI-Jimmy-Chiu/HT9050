// AI(W906-HTDESIGNER) 20261001: a syntax check of every .js of the extension (and package.json) -- with VS Code's
// Electron as Node: Code.exe syn.js <extension folder> <out file> (ELECTRON_RUN_AS_NODE=1; dev\syn.ps1 runs it).
// Each file is compiled (new Function), never run. One line per file: "ok <file>" or "SYNTAX <file> <message>".
'use strict';
const fs = require('fs'), path = require('path');
const root = process.argv[2], outFile = process.argv[3];
const out = [];
const list = ['extension.js'];
for (const dir of ['lib', 'media', 'test', 'test/integration', 'dev']) {
  let names = [];
  try { names = fs.readdirSync(path.join(root, dir)); } catch (e) { continue; }
  for (const n of names.sort()) if (/\.js$/i.test(n)) list.push(dir + '/' + n);
}
for (const f of list) {
  try { new Function(fs.readFileSync(path.join(root, f), 'utf8').replace(/^#!.*/, '')); out.push('ok ' + f); }
  catch (e) { out.push('SYNTAX ' + f + ' ' + e.message); }
}
try { JSON.parse(fs.readFileSync(path.join(root, 'package.json'), 'utf8')); out.push('ok package.json'); } catch (e) { out.push('JSON package.json ' + e.message); }
fs.writeFileSync(outFile, out.join('\n'));
