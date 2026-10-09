'use strict';
// AI(W906-HTDESIGNER) 20260929: find the four places the designer reads from.
//   webRoot    web\          (page\*.html, js\, JSON\)
//   portRoot   the C++ port: a folder with tools\dfm2rc and forms\ (HT9045: HT9011UC_Cpp_*)
//   irRoot     <portRoot>\tools\dfm2rc\ir_out   (DFM -> JSON, properties + events)
//   goldenRoot the BCB6 source: a folder with main.dfm / main.cpp (HT9045: HT9011UC_Code_V*), Big5, read-only
// 20260930: found by their shape, not by the HT9045 folder names -- any software built the
// same way (BCB6 -> web HMI + C++) is found; a name like *_Cpp_* / *_Code_V* only comes first.
// Everything here is plain Node (no vscode) so test\run_tests.js can call it.

const fs = require('fs');
const path = require('path');

function isDir(p) {
  try { return fs.statSync(p).isDirectory(); } catch (e) { return false; }
}
function isFile(p) {
  try { return fs.statSync(p).isFile(); } catch (e) { return false; }
}
function listDirs(p) {
  try {
    return fs.readdirSync(p, { withFileTypes: true }).filter(d => d.isDirectory()).map(d => d.name);
  } catch (e) { return []; }
}

function looksLikeWebRoot(d) {
  return isDir(path.join(d, 'page'));
}
function looksLikePortRoot(d) {
  return !!d && isDir(path.join(d, 'tools', 'dfm2rc')) && isDir(path.join(d, 'forms'));
}
function looksLikeGoldenRoot(d) {
  return !!d && (isFile(path.join(d, 'main.dfm')) || isFile(path.join(d, 'main.cpp')));
}

/** web root for a page file: the nearest ancestor named "web" or holding a page\ dir. */
function findWebRoot(pageFile) {
  const pageDir = path.dirname(pageFile);
  let dir = pageDir;
  for (let i = 0; i < 6; i++) {
    if (path.basename(dir).toLowerCase() === 'web' && looksLikeWebRoot(dir)) return dir;
    const up = path.dirname(dir);
    if (up === dir) break;
    dir = up;
  }
  // web\page\x.html is the normal shape; a page directly in web\ is also fine
  if (path.basename(pageDir).toLowerCase() === 'page') return path.dirname(pageDir);
  return pageDir;
}

/** Port root: a workspace folder that looks like it, else a sibling of the web root. */
function findPortRoot(candidates, webRoot) {
  for (const c of candidates || []) {
    if (looksLikePortRoot(c)) return c;
  }
  const bases = [];
  if (webRoot) bases.push(path.dirname(webRoot));
  for (const c of candidates || []) if (c) bases.push(path.dirname(c));
  // (named like a port tree first -- HT9011UC_Cpp_* -- then any folder with its shape)
  const rank = n => (/_Cpp_/i.test(n) ? 0 : 1);
  for (const b of bases) {
    const sib = listDirs(b)
      .filter(n => !/_noBuild$/i.test(n) && !/^(build|_archive|_backup|node_modules|\.)/i.test(n))
      .map(n => ({ n, p: path.join(b, n) }))
      .filter(x => looksLikePortRoot(x.p))
      .sort((a, c) => rank(a.n) - rank(c.n) || (a.n < c.n ? -1 : a.n > c.n ? 1 : 0));
    if (sib.length) return sib[0].p;
  }
  return null;
}

/** Golden root: walk up from the port root, look for the BCB6 source next to it (named *_Code_V* first, newest). */
function findGoldenRoot(portRoot, webRoot) {
  const starts = [portRoot, webRoot].filter(Boolean);
  const rank = n => (/_Code_V/i.test(n) ? 0 : 1);
  for (const s of starts) {
    let dir = path.dirname(s);
    for (let i = 0; i < 4; i++) {
      const hits = listDirs(dir)
        .filter(n => !/^(build|_archive|_backup|node_modules|web|\.)/i.test(n))
        .map(n => ({ n, p: path.join(dir, n) }))
        .filter(x => looksLikeGoldenRoot(x.p) && !looksLikePortRoot(x.p) && x.p !== portRoot)
        .sort((a, c) => rank(a.n) - rank(c.n) || (a.n < c.n ? 1 : a.n > c.n ? -1 : 0))
        .map(x => x.p);
      if (hits.length) return hits[0];
      const up = path.dirname(dir);
      if (up === dir) break;
      dir = up;
    }
  }
  return viaLinks(portRoot || webRoot, p => findGoldenRoot(p, null));
}

// AI(W906-HTDESIGNER) 20261005 (machine): the tree moved to another disk and left a junction where it was (here
// D:\HT9045\_integ_ioweb -> C:\HT9045_ssd\_integ_ioweb), the BCB6 source staying beside the junction -- walking up
// from the tree's real path never sees it. So, when the walk finds nothing: a junction / symbolic link in the first two
// levels of any drive that points at the tree (or a folder above it), and the same walk from where the link sits.
// Cached for 60 s (a few hundred folder names are listed).
const linkCache = new Map();
function viaLinks(start, findFrom) {
  if (!start || !path.isAbsolute(start)) return null;
  const key = path.resolve(start).toLowerCase();
  const hit = linkCache.get(key);
  if (hit && Date.now() - hit.at < 60000) return hit.v;
  let real;
  try { real = fs.realpathSync.native(start); } catch (e) { real = start; }
  const realL = path.resolve(real).toLowerCase();
  const SYS = /^(\$recycle\.bin|system volume information|windows|program files.*|programdata|users|recovery|msocache|perflogs|\$.*)$/i;
  let found = null;
  outer:
  for (let c = 67; c <= 90 && !found; c++) {   // C: .. Z:
    const drive = String.fromCharCode(c) + ':\\';
    if (!isDir(drive)) continue;
    // (folders and links both: a junction's entry is not a directory entry -- listDirs leaves it out)
    const entries = p => { try { return fs.readdirSync(p, { withFileTypes: true }).filter(e => e.isDirectory() || e.isSymbolicLink()).map(e => e.name); } catch (e) { return []; } };
    const lv1 = entries(drive).filter(n => !SYS.test(n)).map(n => path.join(drive, n));
    const lv2 = [];
    for (const d of lv1) { let isLink = false; try { isLink = fs.lstatSync(d).isSymbolicLink(); } catch (e) { /* gone */ } if (!isLink) for (const n of entries(d).slice(0, 400)) lv2.push(path.join(d, n)); }
    for (const d of lv1.concat(lv2)) {
      let st;
      try { st = fs.lstatSync(d); } catch (e) { continue; }
      if (!st.isSymbolicLink()) continue;
      let tgt;
      try { tgt = path.resolve(fs.realpathSync.native(d)).toLowerCase(); } catch (e) { continue; }
      // the link points at the tree or a folder above it: the same place seen from where the link sits
      if (realL === tgt || realL.startsWith(tgt + path.sep)) {
        const seen = path.join(d, path.relative(tgt, realL));
        if (path.resolve(seen).toLowerCase() === key) continue;
        const r = findFrom(seen);
        if (r) { found = r; break outer; }
      }
    }
  }
  linkCache.set(key, { at: Date.now(), v: found });
  return found;
}

/** Web root when no page is known yet (command palette): sibling "web" of the port root. */
function findWebRootFromPort(portRoot, candidates) {
  const bases = [];
  if (portRoot) bases.push(path.dirname(portRoot));
  for (const c of candidates || []) if (c) { bases.push(c); bases.push(path.dirname(c)); }
  for (const b of bases) {
    const w = path.join(b, 'web');
    if (looksLikeWebRoot(w)) return w;
  }
  return null;
}

/**
 * Resolve every root for one page. `over` holds the user's settings (empty string = auto).
 * Returns { webRoot, portRoot, irRoot, goldenRoot } with null for anything not found.
 */
function resolveRoots(pageFile, workspaceFolders, over) {
  over = over || {};
  const webRoot = (over.webRoot && isDir(over.webRoot)) ? over.webRoot
    : (pageFile ? findWebRoot(pageFile) : findWebRootFromPort(findPortRoot(workspaceFolders, null), workspaceFolders));
  const portRoot = (over.portRoot && isDir(over.portRoot)) ? over.portRoot : findPortRoot(workspaceFolders, webRoot);
  const goldenRoot = (over.goldenRoot && isDir(over.goldenRoot)) ? over.goldenRoot : findGoldenRoot(portRoot, webRoot);
  const ir = portRoot ? path.join(portRoot, 'tools', 'dfm2rc', 'ir_out') : null;
  return {
    webRoot: webRoot || null,
    portRoot: portRoot || null,
    irRoot: ir && isDir(ir) ? ir : null,
    goldenRoot: goldenRoot || null,
  };
}

module.exports = {
  isDir, isFile, findWebRoot, findPortRoot, findGoldenRoot, findWebRootFromPort, resolveRoots,
};
