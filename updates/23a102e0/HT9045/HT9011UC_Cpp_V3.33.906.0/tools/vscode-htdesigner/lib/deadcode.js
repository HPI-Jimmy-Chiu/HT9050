'use strict';
// AI(W906-HTDESIGNER) 20261008 (feature gap #16 -- Visual Studio says "The breakpoint will not currently be hit"; CLAUDE.md:
// a breakpoint in an #if 0 block is moved by gdb to the next live line without a word, and it looks like it bound):
// the lines a C / C++ file never compiles, as far as the text itself decides -- #if 0 / #if false blocks, the #else of
// #if 1 / #if true -- nested and with comments masked first (a "#endif" written inside /* ... */ is not one: the trap that
// broke two hand-written scanners of csystem.cpp). Conditions on macros (#ifdef SOFT_SIMULTE ...) are not decided here.
// -> [{ from, to, why }] (0-based lines, inclusive; the directive lines themselves not counted)
// Plain Node (no vscode).

const { mask } = require('./cppstub');

// (1008 audit D1) a condition that is only 0 / false (or 1 / true), maybe in parentheses, maybe a comment after it
const WHOLE0 = /^\(?\s*(0|false)\s*\)?\s*(\/\/.*|\/\*.*)?$/;
const WHOLE1 = /^\(?\s*(1|true)\s*\)?\s*(\/\/.*|\/\*.*)?$/;

function deadRanges(text) {
  const t = String(text || '');
  let m0;
  try { m0 = mask(t); } catch (e) { m0 = t; }
  // (a CRLF line keeps its \r here -- "." does not match it, and "#endif\r" was no directive: whole files read as dead)
  const lines = m0.split('\n').map(l => l.replace(/\r$/, ''));
  const out = [];
  const stack = [];   // { state: 'dead' | 'live' | '?', parentDead, start, why, taken }
  const deadNow = () => stack.some(s => s.state === 'dead');
  let openStart = -1, openWhy = '';
  const open = (ln, why) => { if (openStart < 0) { openStart = ln; openWhy = why; } };
  const close = ln => { if (openStart >= 0) { if (ln - 1 >= openStart) out.push({ from: openStart, to: ln - 1, why: openWhy }); openStart = -1; } };
  for (let i = 0; i < lines.length; i++) {
    const d = /^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$/.exec(lines[i]);
    if (!d) continue;
    const kind = d[1], arg = d[2].trim();
    const wasDead = deadNow();
    if (kind === 'if' || kind === 'ifdef' || kind === 'ifndef') {
      let st = '?';
      // (1008 full test, audit D1: "#if 0 || defined(X)" is not dead -- the WHOLE condition must be 0 / false)
      if (kind === 'if') { if (WHOLE0.test(arg)) st = 'dead'; else if (WHOLE1.test(arg)) st = 'live'; }
      stack.push({ state: st, line: i, why: '#' + kind + ' ' + arg.split(/\s+/)[0], taken: st === 'live' });
    } else if (kind === 'elif') {
      const s = stack[stack.length - 1];
      if (!s) continue;
      if (s.taken) s.state = 'dead';
      else if (WHOLE0.test(arg)) s.state = 'dead';
      else if (WHOLE1.test(arg)) { s.state = 'live'; s.taken = true; }
      else s.state = '?';
      s.why = '#elif ' + arg.split(/\s+/)[0];
    } else if (kind === 'else') {
      const s = stack[stack.length - 1];
      if (!s) continue;
      s.state = s.taken ? 'dead' : s.state === 'dead' ? 'live' : '?';
      s.why = '#else of ' + s.why;
    } else if (kind === 'endif') {
      stack.pop();
    }
    const isDead = deadNow();
    if (!wasDead && isDead) open(i + 1, (stack.find(s => s.state === 'dead') || {}).why || '#if 0');
    else if (wasDead && !isDead) close(i);
    else if (wasDead && isDead && kind !== 'if' && kind !== 'ifdef' && kind !== 'ifndef' && kind !== 'endif') { close(i); open(i + 1, (stack.find(s => s.state === 'dead') || {}).why); }
  }
  close(lines.length);
  return out;
}

/** the dead range a 0-based line is in, or null */
function deadAt(ranges, line) { return (ranges || []).find(r => line >= r.from && line <= r.to) || null; }

module.exports = { deadRanges, deadAt };
