'use strict';
// AI(W906-HTDESIGNER) 20261008 (EastSun「是不是有可能開啟兩個一樣的專案 一個有紅色點可以中斷的 一個沒有紅色點 其他電腦有發生此事
// 可以都統一 開一個檔案嗎?」): one source file, one editor --
//   (1) a program built from ANOTHER copy of the tree (its build folder's CMakeCache.txt names that copy as
//       CMAKE_HOME_DIRECTORY): the debugger is told the copy -> the tree that is open (cppdbg sourceFileMap, lldb-dap
//       sourceMap), so breakpoints bind in the file the user has open and a stop shows there, not in a second tab;
//   (2) the same file reached by two paths (a junction, a symlink, another spelling): one key -- its real path.
// Plain Node (no vscode).

const fs = require('fs');
const path = require('path');

/** The source tree a program was built from: CMAKE_HOME_DIRECTORY of the CMakeCache.txt beside it (or up to 3 folders up) */
function compileRoot(program, rd) {
  const read = rd || (f => fs.readFileSync(f, 'utf8'));
  let d = path.dirname(String(program || ''));
  for (let i = 0; i < 4 && d; i++) {
    let t = null;
    try { t = read(path.join(d, 'CMakeCache.txt')); } catch (e) { t = null; }
    if (t) { const m = /^CMAKE_HOME_DIRECTORY:\w+=(.+)$/m.exec(t); return m ? m[1].trim() : null; }
    const up = path.dirname(d);
    if (up === d) break;
    d = up;
  }
  return null;
}

/** a path's key: its real path (junctions / symlinks resolved), back slashes, lower case */
function key(p, real) {
  let r = String(p || '');
  try { r = (real || fs.realpathSync.native)(r); } catch (e) { /* not there: as written */ }
  return path.resolve(r).replace(/\//g, '\\').replace(/\\+$/, '').toLowerCase();
}

/**
 * The launch with the copy it was built from mapped to the open tree (1). Nothing to do (same tree, unknown, an attach
 * to a program built here) -> the same object. -> { cfg, from, to } (from / to null when not mapped)
 */
function mapLaunch(cfg, tree, opts) {
  opts = opts || {};
  const none = { cfg, from: null, to: null };
  if (!cfg || !tree || typeof cfg.program !== 'string') return none;
  const from = compileRoot(cfg.program, opts.read);
  if (!from) return none;
  if (key(from, opts.real) === key(tree, opts.real)) return none;
  const to = path.resolve(tree);
  const fwd = from.replace(/\\/g, '/'), back = from.replace(/\//g, '\\');
  const out = Object.assign({}, cfg);
  if (cfg.type === 'cppdbg') {
    out.sourceFileMap = Object.assign({}, cfg.sourceFileMap || {});
    out.sourceFileMap[fwd] = to;
    if (back !== fwd) out.sourceFileMap[back] = to;
  } else if (cfg.type === 'ht9045-lldb' || cfg.type === 'lldb-dap') {
    // (a breakpoint is set through the REVERSE of this map, by prefix and case-sensitively: VS Code sends "c:\..." (its drive
    //  letter lower case) -- measured 1008, a map to "C:\..." left the breakpoint unbound. Both cases of the drive letter)
    const sm = Array.isArray(cfg.sourceMap) ? cfg.sourceMap.slice() : [];
    const drives = [to].concat(/^[a-z]:/i.test(to) ? [to[0].toLowerCase() + to.slice(1), to[0].toUpperCase() + to.slice(1)] : []).filter((x, i, a) => a.indexOf(x) === i);
    for (const t of drives) { sm.push([fwd, t]); if (back !== fwd) sm.push([back, t]); }
    out.sourceMap = sm;
  } else return none;
  return { cfg: out, from, to };
}

module.exports = { compileRoot, key, mapLaunch };
