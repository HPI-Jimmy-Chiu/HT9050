'use strict';
// AI(W906-HTDESIGNER) 20260930: 有新版 -- a VS Code window keeps running the version it loaded;
// a newer one installed later (code --install-extension) only takes over after a reload.
// EastSun: "你不能加個按鈕讓我 reload 嗎" -- so the extension looks whether a newer version of
// itself is installed and offers a button. Plain Node (no vscode).
const fs = require('fs');
const path = require('path');

/** a < b: -1, a = b: 0, a > b: 1 ("0.9.0" < "0.10.0") */
function cmpVer(a, b) {
  const pa = String(a || '').split('.').map(n => parseInt(n, 10) || 0);
  const pb = String(b || '').split('.').map(n => parseInt(n, 10) || 0);
  for (let i = 0; i < Math.max(pa.length, pb.length); i++) {
    const d = (pa[i] || 0) - (pb[i] || 0);
    if (d) return d < 0 ? -1 : 1;
  }
  return 0;
}

/**
 * The version of extension `id` VS Code has registered in the extensions folder `extDir`
 * (its extensions.json), or null (not found / not an extensions folder -- e.g. a development
 * copy run from the source tree).
 */
function installedVersion(extDir, id) {
  if (!extDir || !id) return null;
  let list;
  try { list = JSON.parse(fs.readFileSync(path.join(extDir, 'extensions.json'), 'utf8')); } catch (e) { return null; }
  if (!Array.isArray(list)) return null;
  const want = String(id).toLowerCase();
  let best = null;
  for (const e of list) {
    const eid = e && e.identifier && String(e.identifier.id || '').toLowerCase();
    if (eid !== want || !e.version) continue;
    if (!best || cmpVer(e.version, best) > 0) best = String(e.version);
  }
  return best;
}

module.exports = { cmpVer, installedVersion };
