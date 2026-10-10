'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #22, BCB6 Project > Options -- in this tree's own way: EastSun 20260908「通常都是
// 透過#define來做開關」): the on / off switches of a header like MachineType.h -- a line that is only "#define NAME" (and
// a comment), on when it is live, off when it is commented out with //. The include guard and the switches inside an
// #if 0 block (toggling those changes nothing) are left out. Plain Node (no vscode).

const deadcode = require('./deadcode');

// (1009 second review (toolbox #6): a /* comment after it must close on that line -- "#define X /* a" with "b */" on the
//  next line, turned off, left "b */" as code)
const LINE = /^([ \t]*)(\/\/[ \t]*)?#define[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*(\/\/.*|\/\*.*?\*\/[ \t]*)?$/;
const { mask } = require('./cppstub');

// the text with only its block comments blanked (// ones kept: "// #define X" is a switch turned off) -- same length
function blockMask(t) {
  let out = '', i = 0;
  while (i < t.length) {
    const c = t[i], d = t[i + 1];
    if (c === '/' && d === '/') { let e = t.indexOf('\n', i); if (e < 0) e = t.length; out += t.slice(i, e); i = e; continue; }
    if (c === '/' && d === '*') { const e = t.indexOf('*/', i + 2); const j = e < 0 ? t.length : e + 2; out += t.slice(i, j).replace(/[^\r\n]/g, ' '); i = j; continue; }
    if (c === '"') { let j = i + 1; while (j < t.length && t[j] !== '"' && t[j] !== '\n') j += t[j] === '\\' ? 2 : 1; j = Math.min(t.length, j + 1); out += t.slice(i, j); i = j; continue; }
    out += c; i++;
  }
  return out;
}
const SPLIT = /\r\n|\r|\n/;

/** -> [{ line, name, on, note, indent }] (0-based line) */
function list(text) {
  const t = String(text || '');
  // (1009 second review (toolbox #1): the #if / #endif depth from the text with its comments blanked -- MachineType.h:185-201
  //  "/*#ifdef HiSilicon ... #endif*/" counted 3 #ifdef and 4 #endif, one level too low from there on; a #define inside a
  //  /* */ block is no switch. Lines as deadcode counts them (a lone CR too))
  const lines = blockMask(t).split(SPLIT);
  const rl = t.split(SPLIT);
  let ml;
  try { ml = mask(t).split(SPLIT); } catch (e) { ml = lines; }
  const dead = deadcode.deadRanges(t);
  const guard = (/^\s*#ifndef\s+([A-Za-z_]\w*)\s*\r?\n\s*#define\s+\1\b/m.exec(t) || [])[1];
  const out = [];
  // (1008 review: only the switches at the top level -- one inside #if / #ifdef / #ifndef is set by that condition, not by
  //  hand (MachineType.h:181 DEBUG_HANGUP_NO_HOME under #ifdef SOFT_SIMULTE). Listed too, its name matched the top-level
  //  switch of the same name, and OK with nothing changed turned DEBUG_HANGUP_NO_HOME / DEBUG_ATC on for every build)
  const base = guard ? 1 : 0;
  let depth = 0;
  lines.forEach((raw, i) => {
    const l = raw.replace(/\r$/, '');
    const lm = ml[i] || '';
    if (/^\s*#\s*if(n?def)?\b/.test(lm)) { depth++; return; }
    if (/^\s*#\s*endif\b/.test(lm)) { depth = Math.max(0, depth - 1); return; }
    // (and the line as written: "#define X /* a" whose comment runs on is no switch; 1009 regression review #7: its note
    //  read from the line as written -- the masked one had the /* note */ blanked)
    if (!LINE.test(l)) return;
    const m = LINE.exec(rl[i] || '');
    if (!m || m[3] === guard) return;
    if (depth > base) return;
    if (deadcode.deadAt(dead, i)) return;
    out.push({ line: i, name: m[3], on: !m[2], note: (m[4] || '').replace(/^\/\/\s*|^\/\*\s*|\s*\*\/$/g, '').trim(), indent: m[1] });
  });
  return out;
}

/** The edit that turns switch `sw` (from list) on or off, the rest of its line kept. -> { s, e, text } | null (already so) */
function toggle(text, sw, on) {
  const t = String(text || '');
  if (!!sw.on === !!on) return null;
  // (the lines as list() counts them)
  const starts = [0];
  const re = /\r\n|\r|\n/g;
  let x;
  while ((x = re.exec(t))) starts.push(x.index + x[0].length);
  if (sw.line >= starts.length) return null;
  const s = starts[sw.line];
  let e = sw.line + 1 < starts.length ? starts[sw.line + 1] : t.length;
  if (e > s && t[e - 1] === '\n') e--;   // (its \r stays in the range and is written back, as before)
  const raw = t.slice(s, e), cr = /\r$/.test(raw) ? '\r' : '';
  const l = raw.replace(/\r$/, '');
  const m = LINE.exec(l);
  if (!m || m[3] !== sw.name) return null;
  const body = l.slice(m[1].length + (m[2] ? m[2].length : 0));
  return { s, e, text: m[1] + (on ? '' : '//') + body + cr };
}

module.exports = { list, toggle };
