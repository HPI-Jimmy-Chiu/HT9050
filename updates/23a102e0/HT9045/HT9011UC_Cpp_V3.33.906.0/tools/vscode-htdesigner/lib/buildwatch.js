'use strict';
// AI(W906-HTDESIGNER) 20261002 (machine), EastSun: 「編譯進度可以有個進度條嗎?」
// F5's preLaunchTask builds through tools\build_with_status.ps1 (the port tree's), which every 0.5 s writes
// <web>\JSON\runtime\boot_build.js:  window.__HT_BUILD={"state":"building"|"done"|"failed","pct":N,"file":"...","n":N,
// "errors":N,"lastError":"...","target":"wb_serve","dir":"build_...","elapsed":S,"t":<ms since 1970>};
// The boot wait page reads it; this reads the same file so VS Code shows a progress bar too. READ ONLY: nothing here
// starts, stops or changes a build.
const fs = require('fs');
const path = require('path');

/** The status file of a web root (null without one). */
function statusFile(webRoot) {
  return webRoot ? path.join(webRoot, 'JSON', 'runtime', 'boot_build.js') : null;
}

/** The status object from the file's text, or null. */
function parse(text) {
  const s = String(text || '');
  const a = s.indexOf('{'), b = s.lastIndexOf('}');
  if (a < 0 || b <= a) return null;
  try {
    const o = JSON.parse(s.slice(a, b + 1));
    if (!o || typeof o.state !== 'string') return null;
    return o;
  } catch (e) { return null; }
}

/** The status in `file`, or null (no file, half-written, not ours). */
function read(file) {
  if (!file) return null;
  try { return parse(fs.readFileSync(file, 'utf8')); } catch (e) { return null; }
}

/**
 * What the progress bar does now: 'open' (a build is running and no bar yet), 'update', 'close-done', 'close-failed',
 * 'close-stale' (the writer stopped -- killed, or VS Code was closed), or 'none'.
 * `shown` = a bar is open; `now` = Date.now(). A status older than `staleMs` is not a running build (the file stays
 * on disk after every build: an old "building" must not open a bar when VS Code starts).
 */
function decide(st, shown, now, staleMs) {
  const stale = staleMs || 15000;
  const fresh = !!(st && typeof st.t === 'number' && now - st.t < stale);
  if (!shown) return st && st.state === 'building' && fresh ? 'open' : 'none';
  if (!st) return 'close-stale';
  if (st.state === 'done') return 'close-done';
  if (st.state === 'failed') return 'close-failed';
  return fresh ? 'update' : 'close-stale';
}

/** The bar's text: "45%　cmydef.cpp　(120 個檔，2 分 05 秒)" -- and the errors when there are some. */
function message(st) {
  if (!st) return '';
  const el = Math.max(0, st.elapsed | 0);
  const mm = Math.floor(el / 60), ss = el % 60;
  const t = mm ? mm + ' 分 ' + String(ss).padStart(2, '0') + ' 秒' : ss + ' 秒';
  return (st.pct | 0) + '%' + (st.file ? '　' + st.file : '') + '　（' + (st.n | 0) + ' 個檔，' + t + '）' +
    (st.errors ? '　錯誤 ' + st.errors : '');
}

/**
 * CMake's own progress (the Makefile generators -- MinGW / Unix Makefiles): at the start of a build
 * `cmake -E cmake_progress_start <dir>/CMakeFiles <count>` makes CMakeFiles\Progress\ with count.txt (the steps of the
 * target); every finished step drops one more file in it (make's "[ 45%]" = those files / count); a build that ENDS
 * removes the folder (`cmake_progress_start ... 0`); one that fails or is killed leaves it. Nothing in the build is
 * changed to get it: F5's tasks stay exactly as they are (EastSun 20261002 09:3x: a wrapper on the F5 path stopped F5).
 * -> { dir, pct, steps, total, since } or null.
 */
function cmakeProgress(buildDir) {
  const p = path.join(buildDir, 'CMakeFiles', 'Progress');
  let names, cnt;
  try {
    cnt = fs.statSync(path.join(p, 'count.txt'));
    names = fs.readdirSync(p);
  } catch (e) { return ninjaProgress(buildDir); }
  let total = 0;
  try { total = parseInt(fs.readFileSync(path.join(p, 'count.txt'), 'utf8'), 10) || 0; } catch (e) { total = 0; }
  const steps = names.filter(n => n !== 'count.txt').length;
  return { dir: buildDir, pct: total ? Math.min(100, Math.floor(steps * 100 / total)) : 0, steps, total, since: cnt.mtimeMs };
}

/**
 * 1008 (the full test: a fresh PC's folders are Ninja, which has no CMakeFiles\Progress): the F5 mode build's own log
 * (media\htd_f5_mode.ps1 tees it to <dir>\htd_build.log, deleted when a build starts) -- its last "[n/m]". A build that
 * reached m/m is done (null, like the Progress folder removed); one that stopped short stays (= "not finished").
 */
function ninjaProgress(buildDir) {
  if (!fs.existsSync(path.join(buildDir, 'build.ninja'))) return null;
  const f = path.join(buildDir, 'htd_build.log');
  let st, text;
  try { st = fs.statSync(f); } catch (e) { return null; }
  try {
    const fd = fs.openSync(f, 'r');
    const n = Math.min(st.size, 65536), buf = Buffer.alloc(n);
    fs.readSync(fd, buf, 0, n, st.size - n);
    fs.closeSync(fd);
    text = buf.toString('utf8');
  } catch (e) { return null; }
  const re = /^\s*\[(\d+)\/(\d+)\]/gm;
  let m, last = null;
  while ((m = re.exec(text))) last = m;
  if (!last) return null;
  const steps = +last[1], total = +last[2];
  if (total && steps >= total) return null;
  return { dir: buildDir, pct: total ? Math.min(100, Math.floor(steps * 100 / total)) : 0, steps, total, since: st.birthtimeMs || st.ctimeMs };
}

/** The build folders right under `root` (build*) whose CMake progress started at or after `sinceMs`, newest first. */
function runningCMake(root, sinceMs) {
  let names = [];
  try { names = fs.readdirSync(root); } catch (e) { return []; }
  const out = [];
  for (const n of names) {
    if (!/^build/i.test(n)) continue;
    const pr = cmakeProgress(path.join(root, n));
    if (pr && pr.since >= (sinceMs || 0)) out.push(pr);
  }
  return out.sort((a, b) => b.since - a.since);
}

module.exports = { statusFile, parse, read, decide, message, cmakeProgress, runningCMake };
