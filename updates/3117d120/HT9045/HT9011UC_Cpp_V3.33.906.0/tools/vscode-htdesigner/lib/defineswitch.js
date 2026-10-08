'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #22, BCB6 Project > Options -- in this tree's own way: EastSun 20260908「通常都是
// 透過#define來做開關」): the on / off switches of a header like MachineType.h -- a line that is only "#define NAME" (and
// a comment), on when it is live, off when it is commented out with //. The include guard and the switches inside an
// #if 0 block (toggling those changes nothing) are left out. Plain Node (no vscode).

const deadcode = require('./deadcode');

const LINE = /^([ \t]*)(\/\/[ \t]*)?#define[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*(\/\/.*|\/\*.*)?$/;

/** -> [{ line, name, on, note, indent }] (0-based line) */
function list(text) {
  const t = String(text || '');
  const lines = t.split('\n');
  const dead = deadcode.deadRanges(t);
  const guard = (/^\s*#ifndef\s+([A-Za-z_]\w*)\s*\r?\n\s*#define\s+\1\b/m.exec(t) || [])[1];
  const out = [];
  lines.forEach((raw, i) => {
    const l = raw.replace(/\r$/, '');
    const m = LINE.exec(l);
    if (!m || m[3] === guard) return;
    if (deadcode.deadAt(dead, i)) return;
    out.push({ line: i, name: m[3], on: !m[2], note: (m[4] || '').replace(/^\/\/\s*|^\/\*\s*|\s*\*\/$/g, '').trim(), indent: m[1] });
  });
  return out;
}

/** The edit that turns switch `sw` (from list) on or off, the rest of its line kept. -> { s, e, text } | null (already so) */
function toggle(text, sw, on) {
  const t = String(text || '');
  if (!!sw.on === !!on) return null;
  let s = 0;
  for (let i = 0; i < sw.line; i++) { s = t.indexOf('\n', s) + 1; if (s <= 0) return null; }
  let e = t.indexOf('\n', s); if (e < 0) e = t.length;
  const raw = t.slice(s, e), cr = /\r$/.test(raw) ? '\r' : '';
  const l = raw.replace(/\r$/, '');
  const m = LINE.exec(l);
  if (!m || m[3] !== sw.name) return null;
  const body = l.slice(m[1].length + (m[2] ? m[2].length : 0));
  return { s, e, text: m[1] + (on ? '' : '//') + body + cr };
}

module.exports = { list, toggle };
