'use strict';
// AI(W906-HTDESIGNER) 20261001 (EastSun: "新增各種元件上去 或是新增事件 各種更改 都需要9050 可以編譯過 並且執行"):
// a new event handler is declared with the VCL 6 signature (vclevents.SIG). The port is not VCL: vclcompat has
// TObject / TShiftState / TMouseButton / AnsiString ..., but NOT TPoint, TRect, TDragState, TDragObject,
// TOwnerDrawState, TGridDrawState, TCloseAction; TWinControl only inside two headers no form includes.
// e2e_build.ps1 measured it: OnContextPopup / OnDragOver / OnStartDrag / OnDrawItem / OnMeasureItem added to
// TfHotPlate = fHotPlate.h does not compile, the whole 9050 build fails at F5.
// So before a handler is added: every type its parameters name must be declared in what the form's header
// includes (followed through the port's quoted #includes). A missing one = not added, said why.
const fs = require('fs');
const path = require('path');
const { mask } = require('./cppstub');

// always there: C++ itself, and the Win32 types the port's headers get through <windows.h>
const BUILTIN = new Set(['void', 'bool', 'char', 'short', 'int', 'long', 'float', 'double', 'unsigned', 'signed', 'const', 'volatile',
  'struct', 'class', 'enum', 'auto', 'size_t', 'wchar_t', 'WORD', 'DWORD', 'BYTE', 'BOOL', 'UINT', 'LONG', 'HWND', 'std', 'string']);

/** The type names a parameter list uses ("TObject *Sender, TPoint &MousePos, bool &Handled" -> [TObject, TPoint]). */
function typesOf(params) {
  const out = [];
  for (const raw of String(params || '').split(',')) {
    const p = raw.replace(/=.*$/, '').trim();
    if (!p) continue;
    const ids = p.match(/[A-Za-z_]\w*/g) || [];
    // the last identifier is the parameter's name when there are two or more ("int X"); "TObject*" alone = a type
    const names = ids.length >= 2 ? ids.slice(0, -1) : ids;
    for (const n of names) if (!BUILTIN.has(n) && !out.includes(n)) out.push(n);
  }
  return out;
}

/** Is `name` declared as a type in this (masked) text: class / struct / union / enum / typedef / using / #define. */
function declares(masked, name) {
  const n = name.replace(/[$]/g, '\\$&');
  const res = [
    new RegExp('\\b(?:class|struct|union)\\s+(?:PACKAGE\\s+)?' + n + '\\b'),
    new RegExp('\\benum\\s+(?:class\\s+|struct\\s+)?' + n + '\\b'),
    new RegExp('\\btypedef\\b[^;]*\\b' + n + '\\s*(?:\\[[^\\]]*\\]\\s*)?;'),
    new RegExp('\\busing\\s+' + n + '\\s*='),
    new RegExp('^[ \\t]*#[ \\t]*define[ \\t]+' + n + '\\b', 'm'),
  ];
  return res.some(r => r.test(masked));
}

/**
 * The texts the header sees: itself and every quoted #include it reaches inside the port (each resolved from the
 * port's root, then from the including file's folder -- the port's CMake include dirs). <...> includes are not read.
 * -> [{ file, masked }]
 */
function includeClosure(hFile, portRoot, max) {
  const seen = new Set();
  const out = [];
  const todo = [path.resolve(hFile)];
  const lim = max || 400;
  while (todo.length && out.length < lim) {
    const f = todo.shift();
    const k = f.toLowerCase();
    if (seen.has(k)) continue;
    seen.add(k);
    let t;
    try { t = fs.readFileSync(f, 'latin1'); } catch (e) { continue; }
    const masked = mask(t);
    out.push({ file: f, masked, text: t });
    const re = /^[ \t]*#[ \t]*include[ \t]*"([^"]+)"/gm;
    let m;
    while ((m = re.exec(t))) {
      // (an #include inside a comment is gone in the masked text)
      if (!/#\s*include/.test(masked.slice(m.index, m.index + m[0].length))) continue;
      for (const base of [portRoot, path.dirname(f)]) {
        if (!base) continue;
        const c = path.resolve(base, m[1]);
        if (fs.existsSync(c)) { todo.push(c); break; }
      }
    }
  }
  return out;
}

/** The parameters' types the header cannot see. [] = it compiles as far as types go. */
function missingTypes(params, closure) {
  return typesOf(params).filter(n => !closure.some(x => declares(x.masked, n)));
}

module.exports = { typesOf, declares, includeClosure, missingTypes, BUILTIN };
