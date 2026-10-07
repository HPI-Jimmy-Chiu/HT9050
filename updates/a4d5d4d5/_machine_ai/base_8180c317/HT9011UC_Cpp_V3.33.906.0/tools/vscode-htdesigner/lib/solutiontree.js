'use strict';
// AI(W906-HTDESIGNER) 20261001 (0.138, EastSun's screenshot of Visual Studio's 方案總管 / Solution Explorer): the
// project as Visual Studio shows a solution -- 方案 -> projects (the C++ port tree, the web tree, the BCB6 golden
// tree) -> folders -> files, with "搜尋方案總管 (Ctrl+;)": typed words keep only the files whose name / path has every
// one of them, with their folders. Plain Node (tests run it without VS Code).
const fsp = require('fs/promises');
const path = require('path');
const { SKIP_DIR } = require('./projectsearch');

/** One folder's entries as Solution Explorer lists them: folders first, then files, each A->Z (case-insensitive). */
async function entries(dir) {
  let ents;
  try { ents = await fsp.readdir(dir, { withFileTypes: true }); } catch (e) { return []; }
  const dirs = [], files = [];
  for (const e of ents) {
    if (e.isDirectory()) { if (!SKIP_DIR.test(e.name)) dirs.push(e.name); } else if (e.isFile()) files.push(e.name);
  }
  const by = (a, b) => a.localeCompare(b, undefined, { sensitivity: 'base' });
  return dirs.sort(by).map(n => ({ dir: true, name: n, path: path.join(dir, n) })).concat(files.sort(by).map(n => ({ dir: false, name: n, path: path.join(dir, n) })));
}

/** Every file under root (all kinds -- Solution Explorer shows them all), build / VCS / archive folders skipped. */
async function allFiles(root, max) {
  const out = [];
  const stack = [root];
  const lim = max || 200000;
  while (stack.length && out.length < lim) {
    const d = stack.pop();
    let ents;
    try { ents = await fsp.readdir(d, { withFileTypes: true }); } catch (e) { continue; }
    for (const e of ents) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (!SKIP_DIR.test(e.name)) stack.push(p); } else if (e.isFile()) out.push(p);
    }
  }
  return out;
}

/** The words of a query ("fHot plate .h" -> ['fhot', 'plate', '.h']); none = no filter. */
function words(q) { return String(q || '').toLowerCase().split(/\s+/).filter(Boolean); }

/**
 * The files of `files` (under `root`) that match every word -- in the name, or in the path from the root (a word with
 * a / or \ looks at the path). -> { files: Set, dirs: Set (each match's folders up to the root, the root not in it),
 * count, truncated }
 */
function filter(root, files, q, max) {
  const ws = words(q);
  const out = { files: new Set(), dirs: new Set(), count: 0, truncated: false };
  if (!ws.length) return out;
  const lim = max || 2000;
  const rootR = path.resolve(root);
  for (const f of files) {
    const rel = path.relative(rootR, f).toLowerCase();
    const name = path.basename(f).toLowerCase();
    const relN = rel.replace(/\\/g, '/');
    if (!ws.every(w => (/[\\/]/.test(w) ? relN.includes(w.replace(/\\/g, '/')) : name.includes(w) || relN.includes(w)))) continue;
    if (out.count >= lim) { out.truncated = true; break; }
    out.files.add(path.resolve(f).toLowerCase());
    out.count++;
    for (let d = path.dirname(path.resolve(f)); d.toLowerCase() !== rootR.toLowerCase() && d.length > rootR.length; d = path.dirname(d)) out.dirs.add(d.toLowerCase());
  }
  return out;
}

module.exports = { entries, allFiles, words, filter };
