'use strict';
// AI(W906-HTDESIGNER) 20261007 (EastSun: "編譯如果出現問題 下面須條列出 編譯出錯的code在哪裡 並且可以點選跳到異常程式碼" +
// "vs code的異常碼規範不完全"): what the REAL compiler says, as a list a click can follow --
//   1. parse(): g++ / ld output -> [{ file, line, col, severity, msg }] (file absolute; line / col 1-based, 0 = unknown);
//   2. compileCommand(): how the tree's own build compiles one .cpp (its target's flags.make, includes .rsp, compiler),
//      so a file can be checked with -fsyntax-only -- nothing written, the build folder not touched.
// Plain Node (no vscode).

const fs = require('fs');
const path = require('path');

// (1009 review #1: "internal compiler error" / "sorry, unimplemented" are errors too -- a build that failed with one listed
//  nothing)
const GCC = /^(.*?):(\d+):(?:(\d+):)?\s+(fatal error|internal compiler error|sorry, unimplemented|error|warning|note):\s*(.*)$/;
const LD = /^(?:.*?ld(?:\.exe)?:\s*)?(.*?\.(?:c|cc|cpp|cxx|h|hpp|obj|o))(?::(\d+))?(?::\(\.\w[^)]*\))?:\s+(undefined reference to .*|multiple definition of .*)$/i;
// (1008 audit D1: ld without -g names the object, then the source with no line: "ld.exe: c1n.o:c1.cpp:(.text+0x14): undefined
//  reference", "CMakeFiles\x.dir\a.cpp.obj:a.cpp:(...)", "libht9045_sm.a(csystem.cpp.obj):csystem.cpp:(...)")
// (1009 review (build #3): the archive's name kept -- two built fRotate.cpp (RotateKit -> ht9045_sm, forms -> ht9045_forms):
//  the file name alone took the first one found, often the wrong one)
const LDOBJ = /^(?:.*?ld(?:\.exe)?:\s*)?(?:(?:[^\s(]*?[\\/])?(?:lib)?([^\s(\\/]*?)\.a\()?([^\s():]*?\.(?:obj|o))\)?:([^:\s()]+\.\w+):\([^)]*\):\s+(undefined reference to .*|multiple definition of .*)$/i;
// (and the lines with no place at all: "ld.exe: cannot find -lX", "cc1plus.exe: fatal error: ...", "g++.exe: error: ...")
// (1009 review #1: the build tools too -- ninja / make / windres: "ninja: error: 'x.cpp', needed by ..., missing", "make[2]: ***
//  No rule to make target", "windres.exe: wb_serve.rc:12: syntax error")
const NOPLACE = /^(?:.*?[\\/])?((?:ld|collect2|cc1plus|cc1|g\+\+|gcc|ninja|mingw32-make|make|windres)(?:\.exe)?(?:\[\d+\])?):\s+(?:(fatal error|error):\s+)?(cannot find .*|.+?)$/i;

/** g++ / ld lines -> problems. cwd: what relative paths are relative to (the build folder). */
function parse(text, cwd, srcDir) {
  const out = [];
  const seen = new Set();
  // (1008 full test, audit E2: CMake's own errors -- "CMake Error at CMakeLists.txt:898 (add_library):" and the indented
  //  lines after it (Cannot find source file: ...) -- a .cpp deleted / renamed in 方案總管 used to fail the configure with
  //  nothing in 問題)
  let cm = null;
  const cmDone = () => { if (cm) { const k = cm.file + '|' + cm.line + '|' + cm.msg; if (!seen.has(k)) { seen.add(k); out.push(cm); } cm = null; } };
  const cmFile = f => { const p = String(f).trim(); return /^[A-Za-z]:[\\/]|^[\\/]/.test(p) ? path.normalize(p) : path.resolve(srcDir || cwd || '.', p); };
  const abs = f => {
    let p = String(f).trim().replace(/^"(.*)"$/, '$1');
    if (/^[A-Za-z]:[\\/]|^[\\/]/.test(p)) return path.normalize(p);
    return cwd ? path.resolve(cwd, p) : p;
  };
  // (1008, feature gap #9 -- Visual Studio's Error List shows where an error came from: "In file included from a.cpp:3,"
  //  and "from b.h:7:" lines before it, and the notes after it ("required from here", "declared here"), are kept as the
  //  problem's related places instead of being dropped / listed as problems of their own)
  let chain = [], lastMain = null;
  const INC = /^\s*(?:In file included from|from)\s+(.+?):(\d+)(?::\d+)?[,:]?\s*$/;
  for (const raw of String(text || '').split(/\r?\n/)) {
    // (1008 audit D7: every ANSI control -- GCC's colour also sends \x1b[K)
    const line = raw.replace(/\x1b\[[0-9;]*[A-Za-z]/g, '');
    if (cm) { if (/^\s/.test(line) || line === '') { const tx = line.trim(); if (tx) cm.msg += (/[:：]$/.test(cm.msg) ? ' ' : '；') + tx; continue; } cmDone(); }
    const cme = /^CMake (Error|Warning)(?: \(dev\))? at (.+?):(\d+)(?: \((\w+)\))?:\s*(.*)$/.exec(line);
    if (cme) { cm = { file: cmFile(cme[2]), line: +cme[3], col: 0, severity: cme[1] === 'Error' ? 'error' : 'warning', msg: 'CMake' + (cme[4] ? '（' + cme[4] + '）' : '') + '：' + cme[5].trim(), related: [] }; continue; }
    const cmn = /^CMake Error:\s*(.+)$/.exec(line);
    if (cmn) { const p = { file: '', line: 0, col: 0, severity: 'error', msg: 'CMake：' + cmn[1].trim(), noPlace: true }; if (!seen.has(p.msg)) { seen.add(p.msg); out.push(p); } continue; }
    // (1008 audit E3: "x.cpp:20:7:   required from here" / "required by" -- no severity word: a place of the problem before it)
    const req = /^(.+?):(\d+):(?:\d+:)?\s+((?:recursively )?required (?:from|by) .*|required from here)$/.exec(line);
    if (req && lastMain) { lastMain.related.push({ file: abs(req[1]), line: +req[2], msg: req[3].trim() }); continue; }
    const inc = INC.exec(line);
    if (inc) { chain.push({ file: abs(inc[1]), line: +inc[2], msg: '從這裡 include 進來' }); continue; }
    let m = LDOBJ.exec(line);
    if (m) {
      const p = { file: '', obj: abs(m[2]), src: m[3], archive: m[1] || '', dir: cwd || '', line: 0, col: 0, severity: 'error', msg: '連結錯誤：' + m[4].trim() };
      const k = p.obj + '|' + p.src + '|' + p.msg;
      if (!seen.has(k)) { seen.add(k); out.push(p); }
      continue;
    }
    m = GCC.exec(line);
    if (m && !/^\s*In file included from/.test(line)) {
      const sev = /error|sorry/.test(m[4]) ? 'error' : m[4] === 'warning' ? 'warning' : 'info';
      // (a note belongs to the problem before it)
      if (sev === 'info' && lastMain) { lastMain.related.push({ file: abs(m[1]), line: +m[2], msg: m[5].trim() }); continue; }
      const msg = (/internal compiler error|sorry/.test(m[4]) ? m[4] + '：' : '') + m[5].trim();
      const w = /\[(-W[\w=+-]+)\]\s*$/.exec(msg);
      const p = { file: abs(m[1]), line: +m[2], col: m[3] ? +m[3] : 0, severity: sev, msg, related: chain, code: w ? w[1] : '' };
      chain = [];
      const k = p.file + '|' + p.line + '|' + p.col + '|' + p.msg;
      // (1008 audit E4: the same problem again from another unit -- its notes are the first one's again: swallowed, they
      //  used to become stand-alone info entries)
      if (!seen.has(k)) { seen.add(k); out.push(p); lastMain = p; } else lastMain = { related: [] };
      continue;
    }
    // (a "In function 'x':" / "At global scope:" line opens the next problem's context; anything else ends a chain)
    if (!/:\s*(In (?:member )?function|At global scope|In instantiation of|In constructor|In destructor|In lambda)/.test(line)) chain = [];
    m = LD.exec(line);
    if (m) {
      const f = abs(m[1]);
      const p = { file: /\.(o|obj)$/i.test(f) ? '' : f, obj: /\.(o|obj)$/i.test(f) ? f : '', line: m[2] ? +m[2] : 0, col: 0, severity: 'error', msg: '連結錯誤：' + m[3].trim() };
      const k = p.file + p.obj + '|' + p.line + '|' + p.msg;
      if (!seen.has(k)) { seen.add(k); out.push(p); }
      continue;
    }
    m = NOPLACE.exec(line);
    // (1009 review #1: and what fails a link / a build without the word "error": the exe locked by the program still running
    //  ("cannot open output file ... Permission denied"), "final link failed", "out of memory", make's "***")
    if (m && (m[2] || /^cannot find /i.test(m[3]) || /^(collect2|ld)/i.test(m[1]) && /returned \d+ exit status|error/i.test(m[3]) ||
      /cannot open output file|final link failed|out of memory|virtual memory exhausted|No rule to make target|missing and no known rule|syntax error/i.test(m[3]))) {
      // (collect2's "ld returned 1 exit status" only sums up the errors above it)
      const p = { file: '', line: 0, col: 0, severity: /returned \d+ exit status/i.test(m[3]) ? 'info' : 'error', msg: m[1] + '：' + m[3].trim(), noPlace: true };
      const k = '|' + p.msg;
      if (!seen.has(k)) { seen.add(k); out.push(p); }
    }
  }
  cmDone();
  return out;
}

/** The build folders of a tree, newest configured first (their CMakeCache.txt). */
function buildDirs(tree) {
  const cands = [];
  const add = d => { try { const st = fs.statSync(path.join(d, 'CMakeCache.txt')); cands.push({ dir: d, t: st.mtimeMs }); } catch (e) { /* not one */ } };
  for (const base of [tree, path.resolve(tree, '..', 'Obj', 'V906')]) {
    let names = [];
    try { names = fs.readdirSync(base); } catch (e) { names = []; }
    for (const n of names) if (/^build/i.test(n) || base !== tree) add(path.join(base, n));
  }
  return cands.sort((a, b) => b.t - a.t).map(c => c.dir);
}

/**
 * How the build in `dir` compiles `file` (a .c / .cpp): { compiler, cwd, args } with -fsyntax-only -w, or null when no
 * target of that build has it. The target is found in its DependInfo.cmake (the source, absolute, quoted).
 */
function compileCommand(dir, file) {
  let compiler = null;
  try {
    const cache = fs.readFileSync(path.join(dir, 'CMakeCache.txt'), 'utf8');
    const m = /^CMAKE_CXX_COMPILER:\w+=(.*)$/m.exec(cache);
    if (m) compiler = m[1].trim();
  } catch (e) { return null; }
  if (!compiler) return null;
  const want = path.resolve(file).replace(/\\/g, '/').toLowerCase();
  const hit = sourceIndex(dir).get(want);
  if (!hit) return null;
  if (hit.ninja) {
    // (1008: Ninja -- the object's own variables; Ninja runs the compiler in the build folder)
    const v = hit.vars || {};
    const lang = /\.c$/i.test(file) ? 'C' : 'CXX';
    const args = [].concat(split(v.DEFINES || ''), split(v.INCLUDES || ''), split(v.FLAGS || '').filter(a => !/^-g\d?$|^-W|^-O/.test(a)), ['-fsyntax-only', '-w', '-fmax-errors=50', path.resolve(file)]);
    return { compiler: lang === 'C' ? compiler.replace(/g\+\+(\.exe)?$/i, 'gcc$1') : compiler, cwd: dir, args, target: hit.t.replace(/\.dir$/, ''), buildDir: dir };
  }
  {
    const cm = hit.cm, t = hit.t;
    let flags = '';
    try { flags = fs.readFileSync(path.join(cm, t, 'flags.make'), 'utf8'); } catch (e) { return null; }
    const get = k => { const x = new RegExp('^' + k + ' = (.*)$', 'm').exec(flags); return x ? x[1].trim() : ''; };
    const lang = /\.c$/i.test(file) ? 'C' : 'CXX';
    const args = [].concat(split(get(lang + '_DEFINES')), split(get(lang + '_INCLUDES')),
      split(get(lang + '_FLAGS')).filter(a => !/^-g\d?$|^-W|^-O/.test(a)), ['-fsyntax-only', '-w', '-fmax-errors=50', path.resolve(file)]);
    // (cwd = the folder that owns that CMakeFiles: its @CMakeFiles\...\includes_CXX.rsp is relative to it)
    return { compiler: lang === 'C' ? compiler.replace(/g\+\+(\.exe)?$/i, 'gcc$1') : compiler, cwd: path.dirname(cm), args, target: t.replace(/\.dir$/, ''), buildDir: dir };
  }
}

/**
 * 1008 audit D3: every source a build compiles -> { cm (its CMakeFiles folder), t (target dir name) } -- every CMakeFiles
 * folder of the build (tests\, third_party\... are targets of their own), read once per configure (CMakeCache.txt's time).
 */
const INDEX = new Map();
function sourceIndex(dir) {
  let stamp = 0;
  try { stamp = fs.statSync(path.join(dir, 'CMakeCache.txt')).mtimeMs; } catch (e) { stamp = 0; }
  const c = INDEX.get(dir);
  if (c && c.stamp === stamp) return c.map;
  const map = new Map();
  // (1008, the full test: a Ninja build folder -- what a fresh PC configures now -- has no DependInfo.cmake / flags.make;
  //  every object's source and its DEFINES / INCLUDES / FLAGS are in build.ninja, `build <obj>: C*_COMPILER__... <src>`)
  let ninja = '';
  try { ninja = fs.readFileSync(path.join(dir, 'build.ninja'), 'utf8'); } catch (e) { ninja = ''; }
  if (ninja) {
    const lines = ninja.split(/\r?\n/);
    for (let i = 0; i < lines.length; i++) {
      const m = /^build ([^:]+?\.obj): (C|CXX)_COMPILER__(\S+) (.+?)(?: \|\|| \||$)/.exec(lines[i]);
      if (!m) continue;
      const src = m[4].trim().replace(/\$:/g, ':').replace(/\$ /g, ' ').replace(/\\/g, '/');
      const vars = {};
      for (let j = i + 1; j < lines.length && /^ {2}\S/.test(lines[j]); j++) { const v = /^ {2}(\w+) = (.*)$/.exec(lines[j]); if (v) vars[v[1]] = v[2]; }
      const t = (/CMakeFiles[\\/]([^\\/]+\.dir)/.exec(m[1]) || [])[1] || m[3];
      const k = src.toLowerCase();
      if (/^[a-z]:\//.test(k) && !map.has(k)) map.set(k, { ninja: true, lang: m[2], vars, t, cm: path.join(dir, 'CMakeFiles') });
    }
    INDEX.set(dir, { stamp, map });
    return map;
  }
  const walk = (d, depth) => {
    let ents = [];
    try { ents = fs.readdirSync(d, { withFileTypes: true }); } catch (e) { return; }
    for (const e of ents) {
      if (!e.isDirectory()) continue;
      const p = path.join(d, e.name);
      if (e.name === 'CMakeFiles') {
        let ts = [];
        try { ts = fs.readdirSync(p).filter(n => /\.dir$/.test(n)); } catch (x) { ts = []; }
        for (const t of ts) {
          let di = '';
          try { di = fs.readFileSync(path.join(p, t, 'DependInfo.cmake'), 'utf8'); } catch (x) { continue; }
          const re = /"([A-Za-z]:\/[^"]+\.(?:c|cc|cpp|cxx))"/g;
          let m;
          while ((m = re.exec(di))) { const k = m[1].toLowerCase(); if (!map.has(k)) map.set(k, { cm: p, t }); }
        }
      } else if (depth < 5 && !/^(Testing|\.|_deps$)/.test(e.name)) walk(p, depth + 1);
    }
  };
  walk(dir, 0);
  INDEX.set(dir, { stamp, map });
  return map;
}

/**
 * 1007 (EastSun "要照c++版本"): the compiler and the C++ standard a build folder really uses -- from CMake's own record
 * (CMakeFiles\<ver>\CMakeCXXCompiler.cmake) and the cache (HT9045_CXX_STANDARD / CMAKE_CXX_STANDARD; else -std= of the
 * flags). -> { id, version, major, std, dir } or null
 */
function compilerInfo(dir) {
  let cache = '';
  try { cache = fs.readFileSync(path.join(dir, 'CMakeCache.txt'), 'utf8'); } catch (e) { return null; }
  let id = '', version = '';
  try {
    for (const n of fs.readdirSync(path.join(dir, 'CMakeFiles'))) {
      if (!/^\d/.test(n)) continue;
      let t = '';
      try { t = fs.readFileSync(path.join(dir, 'CMakeFiles', n, 'CMakeCXXCompiler.cmake'), 'utf8'); } catch (e) { continue; }
      const v = /set\(CMAKE_CXX_COMPILER_VERSION "([^"]+)"\)/.exec(t), i = /set\(CMAKE_CXX_COMPILER_ID "([^"]+)"\)/.exec(t);
      if (v) { version = v[1]; id = i ? i[1] : ''; break; }
    }
  } catch (e) { /* no CMakeFiles */ }
  const st = /^HT9045_CXX_STANDARD:\w+=(\d+)/m.exec(cache) || /^CMAKE_CXX_STANDARD:\w+=(\d+)/m.exec(cache) || /-std=(?:c|gnu)\+\+(\w+)/.exec(cache);
  const std = st ? String(st[1]).replace(/^1z$/, '17').replace(/^2a$/, '20') : '';
  return { id, version, major: version ? parseInt(version, 10) : 0, std, dir };
}

/** A flags.make value split into arguments (double quotes kept together). */
function split(s) {
  const out = [];
  const re = /"([^"]*)"|(\S+)/g;
  let m;
  while ((m = re.exec(String(s || '')))) out.push(m[1] !== undefined ? m[1] : m[2]);
  return out;
}

module.exports = { parse, buildDirs, compileCommand, compilerInfo, sourceIndex, split };
