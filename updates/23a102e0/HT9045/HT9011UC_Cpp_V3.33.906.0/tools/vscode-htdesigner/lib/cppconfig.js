'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #7, the full test: IntelliSense was not the build -- c_cpp_properties.json names
// C:\MinGW\bin\g++.exe (not on every PC), C++17 (the WinLibs lines build C++14) and none of CMake's switches
// (SOFT_SIMULTE / W906_NO_SOFT_SIMULTE ...), so Ctrl+click, F12 and the greyed #if regions disagreed with the compiler):
// a file's IntelliSense configuration made from the compile command the build itself uses (lib/builderrors.compileCommand:
// Makefiles' flags.make or Ninja's build.ninja). Plain Node (no vscode).

const fs = require('fs');
const path = require('path');

/** The words of a response file (@file): GCC's quoting -- "..." / '...' / backslash before a quote or a space. */
function rspWords(text) {
  const out = [];
  const re = /"((?:[^"\\]|\\.)*)"|'([^']*)'|(\S+)/g;
  let m;
  while ((m = re.exec(String(text || '')))) out.push(m[1] != null ? m[1].replace(/\\(["\\])/g, '$1') : m[2] != null ? m[2] : m[3]);
  return out;
}

/**
 * args (a compile command's, as lib/builderrors gives them) -> { includePath, defines, standard, compilerArgs }
 * @file arguments read (relative to cwd); -I / -isystem / -iquote paths made absolute; -D kept as NAME or NAME=VALUE.
 */
function fromArgs(args, cwd) {
  const words = [];
  for (const a of args || []) {
    if (/^@/.test(a)) { try { words.push(...rspWords(fs.readFileSync(path.resolve(cwd || '.', a.slice(1)), 'utf8'))); } catch (e) { /* not there */ } }
    else words.push(String(a));
  }
  const includePath = [], defines = [], compilerArgs = [];
  let standard = '';
  const abs = p => path.resolve(cwd || '.', String(p).replace(/^"(.*)"$/, '$1'));
  for (let i = 0; i < words.length; i++) {
    const w = words[i];
    let m;
    if ((m = /^-(I|isystem|iquote|idirafter)(.*)$/.exec(w))) { const p = m[2] || words[++i]; if (p) includePath.push(abs(p)); continue; }
    if ((m = /^-D(.*)$/.exec(w))) { const d = m[1] || words[++i]; if (d) defines.push(d); continue; }
    if ((m = /^-std=(?:gnu|c)\+\+(\w+)$/.exec(w))) { standard = 'c++' + m[1].replace(/^1z$/, '17').replace(/^2a$/, '20').replace(/^1y$/, '14'); continue; }
    if ((m = /^-std=(?:gnu|c)(\d+)$/.exec(w))) { standard = 'c' + m[1]; continue; }
    if (/^-(m32|m64|march=.*|fexcess-precision=.*|funsigned-char|fsigned-char|fshort-enums)$/.test(w)) compilerArgs.push(w);
  }
  const uniq = a => Array.from(new Set(a));
  return { includePath: uniq(includePath), defines: uniq(defines), standard, compilerArgs };
}

/** The IntelliSense mode for a compiler: 32-bit i686 MinGW or x86_64 by its folder / name. */
function modeOf(compiler) {
  return /x86_64|mingw64|x64/i.test(String(compiler || '')) ? 'windows-gcc-x64' : 'windows-gcc-x86';
}

module.exports = { fromArgs, rspWords, modeOf };
