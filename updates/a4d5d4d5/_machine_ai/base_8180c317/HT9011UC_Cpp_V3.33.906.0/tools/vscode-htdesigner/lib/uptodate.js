'use strict';
// AI(W906-HTDESIGNER) 20261005 (machine, EastSun: "如果已經編譯過了 請按下F5的時候 就直接啟動軟體"): is a built exe newer than
// every source that goes into it? Then F5 can skip its build. Plain Node (tests run it without VS Code). Conservative:
// anything it cannot tell = "build" (a missing / tiny exe, a build folder whose CMake files are newer than the exe -- the
// configuration changed --, a source newer than the exe).
const fs = require('fs');
const path = require('path');

// what a C++ build reads: sources, headers, generated includes, resources, CMake
const EXT = new Set(['.cpp', '.h', '.hpp', '.hh', '.c', '.cc', '.cxx', '.inc', '.inl', '.def', '.rc', '.cmake']);
const NAMES = new Set(['CMakeLists.txt']);
// build output, version control, editors, agents' worktree copies, tests (not in the program), notes, the web tree
const SKIP = /^(build.*|\.git|\.svn|\.vs|\.vscode|\.claude|node_modules|__pycache__|dist|out|obj|Debug|Release|tests|docs|_archive.*|_backup.*)$/i;

/** The newest source under `root`: { mtime, file, count }. */
function newestSource(root) {
  let best = { mtime: 0, file: null, count: 0 };
  const stack = [root];
  while (stack.length) {
    const d = stack.pop();
    let ents;
    try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { continue; }
    for (const e of ents) {
      const p = path.join(d, e.name);
      if (e.isDirectory()) { if (!SKIP.test(e.name)) stack.push(p); continue; }
      if (!e.isFile() || !(EXT.has(path.extname(e.name).toLowerCase()) || NAMES.has(e.name))) continue;
      let st;
      try { st = fs.statSync(p); } catch (x) { continue; }
      best.count++;
      if (st.mtimeMs > best.mtime) best = { mtime: st.mtimeMs, file: p, count: best.count };
    }
  }
  return best;
}

/**
 * check(exe, root) -> { upToDate, why, exeTime, newest } -- `exe` the program F5 runs (in its build folder), `root` the
 * source tree. upToDate only when the exe is there (> 64 KB), newer than every source and than its build folder's
 * CMakeCache.txt / Makefile / build.ninja.
 */
function check(exe, root) {
  let st;
  try { st = fs.statSync(exe); } catch (e) { return { upToDate: false, why: '還沒有 ' + path.basename(exe) }; }
  if (st.size < 64 * 1024) return { upToDate: false, why: path.basename(exe) + ' 太小（' + st.size + ' bytes，可能是沒連結完的）' };
  const dir = path.dirname(exe);
  for (const n of ['CMakeCache.txt', 'Makefile', 'build.ninja']) {
    try { const c = fs.statSync(path.join(dir, n)); if (c.mtimeMs > st.mtimeMs) return { upToDate: false, why: '建置設定改過（' + n + ' 比 exe 新）' }; } catch (e) { /* none */ }
  }
  const nw = newestSource(root);
  if (!nw.count) return { upToDate: false, why: '找不到原始碼（' + root + '）' };
  if (nw.mtime > st.mtimeMs) return { upToDate: false, why: path.relative(root, nw.file) + ' 比 exe 新', exeTime: st.mtimeMs, newest: nw };
  return { upToDate: true, why: '原始碼都沒有比 exe 新（' + nw.count + ' 個檔）', exeTime: st.mtimeMs, newest: nw };
}

module.exports = { newestSource, check, EXT, SKIP };
