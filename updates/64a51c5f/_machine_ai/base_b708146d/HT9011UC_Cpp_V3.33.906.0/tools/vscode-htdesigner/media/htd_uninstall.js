'use strict';
// AI(W906-HTDESIGNER) 20261009 (second review, debug #6): run once by VS Code after the extension is uninstalled
// (package.json "vscode:uninstall"; no vscode API here) -- the designer's own entries taken out of the user
// keybindings.json (every profile). The CMake Tools removals (-cmake.build F7 ...) carry no "when" and outlived the
// extension: CMake Tools' keys dead in every project. Only lines this extension wrote (lib/bcbkeys.js remove); the
// user's own entries stay. Errors are ignored: nothing to tell, nobody to tell it to.

const fs = require('fs');
const path = require('path');
const bk = require('../lib/bcbkeys');

// (1010 keys walkthrough #2: only the VS Code this extension was installed in -- uninstalled from Insiders it emptied Stable's
//  keys too, and Stable then took them all as "deleted by the user" for good)
const users = [];
const ad = process.env.APPDATA;
const here = __dirname.toLowerCase();
if (process.env.VSCODE_PORTABLE && here.startsWith(path.resolve(process.env.VSCODE_PORTABLE).toLowerCase())) users.push(path.join(process.env.VSCODE_PORTABLE, 'user-data', 'User'));
else if (ad) users.push(path.join(ad, /[\\/]\.vscode-insiders[\\/]/.test(here) ? 'Code - Insiders' : /[\\/]\.vscode-oss[\\/]/.test(here) ? 'VSCodium' : 'Code', 'User'));
// (… the user's own copies of our lines (there before our first write), as the extension recorded them)
let keep = null;
try { const id = (require('../package.json').publisher + '.' + require('../package.json').name).toLowerCase(); for (const u0 of users) { const kf = path.join(u0, 'globalStorage', id, 'bcbkeys_keep.json'); if (fs.existsSync(kf)) keep = new Set(JSON.parse(fs.readFileSync(kf, 'utf8'))); } } catch (e) { keep = null; }
for (const u of users) {
  const files = [path.join(u, 'keybindings.json')];
  try { for (const p of fs.readdirSync(path.join(u, 'profiles'))) files.push(path.join(u, 'profiles', p, 'keybindings.json')); } catch (e) { /* no profiles */ }
  for (const f of files) {
    let t;
    try { t = fs.readFileSync(f, 'utf8'); } catch (e) { continue; }
    let n;
    try { n = bk.remove(t, { keep }); } catch (e) { continue; }
    if (n !== t) { try { bk.writeAtomic(fs, path, f, n); } catch (e) { /* read-only */ } }
  }
}
