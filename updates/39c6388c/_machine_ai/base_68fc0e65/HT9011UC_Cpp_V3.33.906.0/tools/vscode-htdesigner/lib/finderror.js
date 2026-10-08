'use strict';
// AI(W906-HTDESIGNER) 20261008 (gap list #21, BCB6 Search > Find Error): an address the program crashed at -- from a crash
// dialog, the event log, a bootsample line ("main: 016078cb 0160850b ...") -- to its function and source line, through
// the exe's own debug information (addr2line -f -i, the inline chain too; names demangled with c++filt -- MinGW i686
// symbols carry one leading underscore addr2line -C does not take off). Plain Node (no vscode).

const fs = require('fs');

/** The PE's ImageBase (where its addresses start), or null. */
function imageBase(exe) {
  try {
    const fd = fs.openSync(exe, 'r');
    const h = Buffer.alloc(4096);
    fs.readSync(fd, h, 0, 4096, 0);
    fs.closeSync(fd);
    if (h.readUInt16LE(0) !== 0x5a4d) return null;
    const pe = h.readUInt32LE(0x3c);
    if (h.readUInt32LE(pe) !== 0x4550) return null;
    const opt = pe + 24, magic = h.readUInt16LE(opt);
    if (magic === 0x10b) return BigInt(h.readUInt32LE(opt + 28));
    if (magic === 0x20b) return h.readBigUInt64LE(opt + 24);
    return null;
  } catch (e) { return null; }
}

/**
 * What was typed -> the addresses to look up (hex strings 0x...). "016078cb", "0x52ba88", several at once (a bootsample
 * line), "wb_serve.exe+0x1234" / "+1234" (an offset into the module = ImageBase + offset). -> { addrs: [..], error }
 */
function parseAddrs(input, base) {
  const s = String(input || '').trim();
  if (!s) return { addrs: [], error: '沒有位址' };
  const out = [];
  const off = /(?:^|[\s,;])(?:[\w.-]+\.(?:exe|dll))?\+\s*(?:0x)?([0-9a-f]+)\b/gi;
  let m, any = false;
  while ((m = off.exec(s))) {
    any = true;
    if (base == null) return { addrs: [], error: '讀不到 exe 的 ImageBase，沒辦法換算「+位移」' };
    out.push('0x' + (base + BigInt('0x' + m[1])).toString(16));
  }
  if (!any) {
    const re = /\b(?:0x)?([0-9a-f]{5,16})\b/gi;
    while ((m = re.exec(s))) out.push('0x' + m[1].toLowerCase().replace(/^0+(?=.)/, ''));
  }
  if (!out.length) return { addrs: [], error: '認不出位址（例如 016078cb、0x52ba88、wb_serve.exe+0x1234）' };
  return { addrs: Array.from(new Set(out)) };
}

/** addr2line -f -i output (function, file:line pairs; "??" = unknown) -> [{ fn, file, line }] innermost first */
function frames(out) {
  const ls = String(out || '').split(/\r?\n/).filter(l => l !== '');
  const res = [];
  for (let i = 0; i + 1 < ls.length; i += 2) {
    const fn = ls[i].trim(), loc = ls[i + 1].trim();
    const m = /^(.*):(\d+|\?)(?: \(discriminator \d+\))?$/.exec(loc);
    res.push({ fn, file: m && m[1] !== '??' ? m[1] : '', line: m && m[2] !== '?' ? +m[2] : 0 });
  }
  return res;
}

/** a mangled name addr2line left as it is ("ZN9vclcompat..." -- the underscore MinGW added was taken as the prefix) */
const needsDemangle = n => /^_{0,2}Z/.test(String(n || ''));

/** the first frame in the program's own sources (not the compiler's headers / runtime) */
function focus(fr, tree) {
  const t = String(tree || '').replace(/\\/g, '/').toLowerCase();
  return (fr || []).find(f => f.file && f.line && !/[\\/](mingw32|mingw64|include[\\/]c\+\+|bits)[\\/]/i.test(f.file) && (!t || f.file.replace(/\\/g, '/').toLowerCase().startsWith(t))) ||
    (fr || []).find(f => f.file && f.line) || null;
}

module.exports = { imageBase, parseAddrs, frames, needsDemangle, focus };
