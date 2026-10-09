'use strict';
// AI(W906-HTDESIGNER) 20261009 (second review, debug #6): run once by VS Code after the extension is uninstalled
// (package.json "vscode:uninstall"; no vscode API here) -- the designer's own entries taken out of the user
// keybindings.json (every profile). The CMake Tools removals (-cmake.build F7 ...) carry no "when" and outlived the
// extension: CMake Tools' keys dead in every project. Only lines this extension wrote (lib/bcbkeys.js remove); the
// user's own entries stay. Errors are ignored: nothing to tell, nobody to tell it to.

const fs = require('fs');
const path = require('path');
const bk = require('../lib/bcbkeys');

const users = [];
const ad = process.env.APPDATA;
if (ad) for (const n of ['Code', 'Code - Insiders', 'VSCodium']) users.push(path.join(ad, n, 'User'));
if (process.env.VSCODE_PORTABLE) users.push(path.join(process.env.VSCODE_PORTABLE, 'user-data', 'User'));
for (const u of users) {
  const files = [path.join(u, 'keybindings.json')];
  try { for (const p of fs.readdirSync(path.join(u, 'profiles'))) files.push(path.join(u, 'profiles', p, 'keybindings.json')); } catch (e) { /* no profiles */ }
  for (const f of files) {
    let t;
    try { t = fs.readFileSync(f, 'utf8'); } catch (e) { continue; }
    let n;
    try { n = bk.remove(t); } catch (e) { continue; }
    if (n !== t) { try { fs.writeFileSync(f, n); } catch (e) { /* read-only */ } }
  }
}
