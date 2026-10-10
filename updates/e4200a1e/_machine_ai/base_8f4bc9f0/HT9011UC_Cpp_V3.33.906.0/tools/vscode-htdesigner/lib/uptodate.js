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
/**
 * 1006 audit (the build-integrity check of the run bar): whether a PE file is whole -- its own structure says how long it
 * must be: every section's raw data, and the COFF symbol table + its string table, inside the file; "MZ", e_lfanew and
 * "PE\0\0" where they belong (tools/pe_truncation_check.ps1 asks the same). A linker killed mid-write leaves an exe that
 * is cut short (it runs, half of it missing) or whose header is all zeros (it does not load) -- both used to count as
 * built, and F5 skipped the build and ran it. -> null = whole, else why not.
 */
function peBroken(file) {
  let fd = null;
  try {
    fd = fs.openSync(file, 'r');
    const size = fs.fstatSync(fd).size;
    const rd = (pos, len) => { const b = Buffer.alloc(len); const n = fs.readSync(fd, b, 0, len, pos); return n === len ? b : null; };
    const h = rd(0, 64);
    if (!h || h[0] !== 0x4d || h[1] !== 0x5a) return '開頭不是 MZ（可能是寫到一半的檔、整段是 0）';
    const lfanew = h.readUInt32LE(0x3c);
    if (lfanew < 64 || lfanew + 24 > size) return 'PE 檔頭位置不對（e_lfanew ' + lfanew + '）';
    const ph = rd(lfanew, 24);
    if (!ph || ph.readUInt32LE(0) !== 0x00004550) return '找不到 PE 簽章';
    const nSec = ph.readUInt16LE(6), symPtr = ph.readUInt32LE(12), nSym = ph.readUInt32LE(16), optSize = ph.readUInt16LE(20);
    const secAt = lfanew + 24 + optSize;
    const sec = rd(secAt, nSec * 40);
    if (!sec) return '節區表不完整';
    let need = secAt + nSec * 40;
    for (let i = 0; i < nSec; i++) {
      const raw = sec.readUInt32LE(i * 40 + 16), ptr = sec.readUInt32LE(i * 40 + 20);
      if (raw && ptr + raw > need) need = ptr + raw;
    }
    if (symPtr && nSym) {
      const strAt = symPtr + nSym * 18;
      const sl = strAt + 4 <= size ? rd(strAt, 4) : null;
      if (!sl) return '符號表不完整（檔案在 ' + size + ' bytes 斷掉）';
      need = Math.max(need, strAt + sl.readUInt32LE(0));
    }
    if (need > size) return '檔案被截斷：結構要求 ' + need + ' bytes，只有 ' + size;
    return null;
  } catch (e) {
    return '讀不了（' + (e.code || e.message) + '）';
  } finally {
    if (fd !== null) try { fs.closeSync(fd); } catch (e) { /* closed */ }
  }
}

function check(exe, root) {
  let st;
  try { st = fs.statSync(exe); } catch (e) { return { upToDate: false, why: '還沒有 ' + path.basename(exe) }; }
  if (st.size < 64 * 1024) return { upToDate: false, why: path.basename(exe) + ' 太小（' + st.size + ' bytes，可能是沒連結完的）' };
  const broken = /\.exe$/i.test(exe) ? peBroken(exe) : null;
  if (broken) return { upToDate: false, why: path.basename(exe) + ' 不完整（' + broken + '），要重新連結' };
  const dir = path.dirname(exe);
  for (const n of ['CMakeCache.txt', 'Makefile', 'build.ninja']) {
    try { const c = fs.statSync(path.join(dir, n)); if (c.mtimeMs > st.mtimeMs) return { upToDate: false, why: '建置設定改過（' + n + ' 比 exe 新）' }; } catch (e) { /* none */ }
  }
  // (1009 second review (build #2), safety: how the build is made -- the tree's .vscode\tasks.json (its cmake -D options:
  //  SOFT_SIMULTE, the build type) and the .bat / .ps1 at its top (build_nonoracle.bat ...) -- changed after the exe = build;
  //  the old SIM / shipping exe (they differ in the safety-door interlock) was started without one)
  {
    const how = [path.join(root, '.vscode', 'tasks.json')];
    try { for (const n of fs.readdirSync(root)) if (/\.(bat|cmd|ps1)$/i.test(n)) how.push(path.join(root, n)); } catch (e) { /* none */ }
    // (1009 regression review #3: against the exe AND the build folder's CMakeCache.txt -- a build that changed nothing (no
    //  relink) left the exe older than tasks.json, and every F5 after it built again, for ever; a configure rewrites the cache)
    // (1009 regression review 2 #7: against a stamp a build that ENDED FINE left (htd_how.stamp, by the extension) -- not the
    //  CMakeCache a configure rewrote before a compile that failed or was stopped)
    let seen = st.mtimeMs;
    try { seen = Math.max(seen, fs.statSync(path.join(path.dirname(exe), 'htd_how.stamp')).mtimeMs); } catch (e) { /* no stamp */ }
    for (const f of how) { try { if (fs.statSync(f).mtimeMs > seen) return { upToDate: false, why: path.relative(root, f) + ' 比 exe 新（建置方式改過）' }; } catch (e) { /* none */ } }
  }
  const nw = newestSource(root);
  if (!nw.count) return { upToDate: false, why: '找不到原始碼（' + root + '）' };
  if (nw.mtime > st.mtimeMs) return { upToDate: false, why: path.relative(root, nw.file) + ' 比 exe 新', exeTime: st.mtimeMs, newest: nw };
  return { upToDate: true, why: '原始碼都沒有比 exe 新（' + nw.count + ' 個檔）', exeTime: st.mtimeMs, newest: nw };
}

module.exports = { newestSource, check, peBroken, EXT, SKIP };
