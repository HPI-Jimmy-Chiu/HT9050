'use strict';
// AI(W906-HTDESIGNER) 20261008 (feature gap #5, Visual Studio's Solution Explorer: Add > New Item puts a .cpp in the
// project): the tree's CMakeLists.txt lists every source of a target inside its add_library( ... ) / add_executable( ... ).
//   targets(text)            -> [{ name, kind, start, end }]   (the call's own text range, comments skipped for the parens)
//   targetsFor(text, relDir) -> the targets that already have sources of that folder, most first:
//                               [{ name, count, insertAt, indent }]  (insertAt = after the last line of that folder)
//   insertSource(text, t, rel, note) -> { at, text }   the line to add
//   refsOf(text, rel)        -> [{ s, e }]   where a source path is written (to rename / warn on delete)
// Plain Node (no vscode).

/** add_library / add_executable calls: their name and range (a ')' inside a comment or a string does not close one). */
function targets(text) {
  const t = String(text || '');
  const out = [];
  const re = /^[ \t]*(add_library|add_executable)\s*\(\s*([A-Za-z_][\w.-]*)/gm;
  let m;
  while ((m = re.exec(t))) {
    let i = t.indexOf('(', m.index), depth = 0, end = -1;
    for (; i < t.length; i++) {
      const c = t[i];
      if (c === '#') { const nl = t.indexOf('\n', i); i = nl < 0 ? t.length : nl; continue; }
      if (c === '"') { const q = t.indexOf('"', i + 1); i = q < 0 ? t.length : q; continue; }
      if (c === '(') depth++;
      else if (c === ')' && !--depth) { end = i + 1; break; }
    }
    if (end < 0) continue;
    out.push({ name: m[2], kind: m[1], start: m.index, end });
    re.lastIndex = end;
  }
  return out;
}

const escRe = s => String(s).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

/** the targets with sources directly in relDir ('' = the tree's root), most sources first */
function targetsFor(text, relDir) {
  const t = String(text || '');
  const dir = String(relDir || '').replace(/\\/g, '/').replace(/\/$/, '');
  const lineRe = new RegExp('^([ \\t]*)' + (dir ? escRe(dir) + '/' : '') + '[^/\\s#()]+\\.(?:c|cc|cpp|cxx)\\b[^\\r\\n]*', 'gmi');
  const out = [];
  for (const tg of targets(t)) {
    const body = t.slice(tg.start, tg.end);
    let m, count = 0, last = null;
    lineRe.lastIndex = 0;
    while ((m = lineRe.exec(body))) { count++; last = m; }
    if (!count) continue;
    // (1008 full test, audit F1: the last source on the same line as the call's closing ")" -- CMakeLists.txt:3452
    //  `JsonBridge/actions/MainStateRecord.cpp)   # ...` -- the new line goes BEFORE it, else it lands outside the call)
    const code = last[0].replace(/#.*$/, '');
    if (/\)/.test(code)) out.push({ name: tg.name, count, insertAt: tg.start + last.index, indent: last[1], before: true });
    else out.push({ name: tg.name, count, insertAt: tg.start + last.index + last[0].length, indent: last[1] });
  }
  return out.sort((a, b) => b.count - a.count);
}

/** the line that adds `rel` (path from the tree, '/') after the target's last source of that folder */
function insertSource(text, tg, rel, note) {
  const eol = /\r\n/.test(String(text || '')) ? '\r\n' : '\n';
  // (1008 review: a path with a blank or ";" quoted -- CMake takes "Motor/a b.cpp" bare as two sources)
  const r0 = String(rel).replace(/\\/g, '/');
  const ln = tg.indent + (/[\s;]/.test(r0) ? '"' + r0 + '"' : r0) + (note ? '    # ' + note : '');
  return { at: tg.insertAt, text: tg.before ? ln + eol : eol + ln };
}

/** the # comments of a CMake text (a # inside "..." is not one): [[start, end)] */
function commentsOf(t) {
  const out = [];
  let q = false;
  for (let i = 0; i < t.length; i++) {
    const c = t[i];
    if (c === '\\' && q) { i++; continue; }
    if (c === '"') { q = !q; continue; }
    if (c === '#' && !q) { let e = t.indexOf('\n', i); if (e < 0) e = t.length; out.push([i, e]); i = e; continue; }
  }
  return out;
}

/** where `rel` is written as a source (a whole word, not part of a longer path) */
function refsOf(text, rel) {
  const t = String(text || '');
  const r = String(rel).replace(/\\/g, '/');
  const re = new RegExp('(^|[\\s("])' + escRe(r) + '(?=[\\s)"#]|$)', 'gmi');
  // (1008 review: not in a # comment -- 471 of the names in this tree's two CMakeLists.txt are in comments; "移出建置"
  //  deleted them from the comments and "加入建置" refused a file only a comment names as "already there")
  const cm = commentsOf(t);
  const inC = at => cm.some(x => at >= x[0] && at < x[1]);
  const out = [];
  let m;
  while ((m = re.exec(t))) { const s = m.index + m[1].length; if (!inC(s)) out.push({ s, e: s + r.length }); }
  return out;
}

/** the command call around position `at`: { name, open, close } (open = its "(", close = its ")") | null */
// (1009 review (new item #7): a "(" or ")" in a # comment or a "string" is not a paren -- a comment "# (old)" between the
//  sources made the file's call look like none, and its set_source_files_properties was left half)
function quietMask(t) {
  const m = new Uint8Array(t.length);
  let q = false;
  for (let i = 0; i < t.length; i++) {
    const c = t[i];
    if (q) { m[i] = 1; if (c === '\\') { if (i + 1 < t.length) m[++i] = 1; continue; } if (c === '"') q = false; continue; }
    if (c === '"') { m[i] = 1; q = true; continue; }
    if (c === '#') { let e = t.indexOf('\n', i); if (e < 0) e = t.length; for (; i < e; i++) m[i] = 1; i--; continue; }
  }
  return m;
}
function callAt(t, at, mask) {
  const mk = mask || quietMask(t);
  let d = 0, i = at;
  for (; i >= 0; i--) { if (mk[i]) continue; const c = t[i]; if (c === ')') d++; else if (c === '(') { if (d === 0) break; d--; } }
  if (i < 0) return null;
  const nm = /([A-Za-z_][A-Za-z0-9_]*)\s*$/.exec(t.slice(Math.max(0, i - 80), i));
  let e = at, dd = 0;
  for (; e < t.length; e++) { if (mk[e]) continue; const c = t[e]; if (c === '(') dd++; else if (c === ')') { if (dd === 0) break; dd--; } }
  return nm ? { name: nm[1], open: i, close: e, nameAt: i - nm[0].length + nm[0].search(/\S/) } : null;
}

/**
 * 1008 (gap list #16, BCB6 Project > Remove from Project): the edits that take `rel` out of every target, the file kept --
 * a line that is only that source (and a comment) goes whole, line break too; a source sharing its line (with ")" or
 * other sources) loses just its word and the blanks after it. -> [{ s, e }] in text order
 */
function removeSource(text, rel, cmDir) {
  const t = String(text || '');
  const out = [];
  const refs = cmDir === undefined ? refsOf(t, rel) : refsAll(t, rel, cmDir);
  // (1008 review: set_source_files_properties(<only this file> PROPERTIES ...) -- the file taken out left
  //  "set_source_files_properties(PROPERTIES ...)", an error at the next configure; that call goes whole)
  const whole = new Map();
  const mask = quietMask(t);
  for (const r of refs) {
    const c = callAt(t, r.s, mask);
    if (!c || !/^set_source_files_properties$/i.test(c.name)) continue;
    const args = t.slice(c.open + 1, c.close).replace(/#[^\n]*/g, '');
    const files = (args.split(/\bPROPERTIES\b/i)[0].match(/"[^"]*"|[^\s"]+/g) || []);
    const mine = refs.filter(x => x.s > c.open && x.s < c.close).length;
    if (files.length <= mine) whole.set(c.open, c);
  }
  for (const c of whole.values()) {
    const ls = t.lastIndexOf('\n', c.nameAt - 1) + 1;
    let le = t.indexOf('\n', c.close); if (le < 0) le = t.length;
    const only = !/\S/.test(t.slice(ls, c.nameAt)) && /^\)\s*(#.*)?\r?$/.test(t.slice(c.close, le));
    out.push(only ? { s: ls, e: le < t.length ? le + 1 : le } : { s: c.nameAt, e: c.close + 1 });
  }
  // (1009 review (new item #2): a target left with no source at all -- add_library(x STATIC ) is an error at the next
  //  configure; the caller refuses: out.empties = [target names])
  const empties = [];
  { const per = new Map();
    for (const r of refs) {
      const c = callAt(t, r.s, mask);
      // (1009 regression review #6: add_library / add_executable only -- a one-file target_sources(x PRIVATE a.cpp) beside
      //  the target's own list is no "only source"; an empty target_sources call is fine for CMake)
      if (!c || !/^(add_library|add_executable)$/i.test(c.name)) continue;
      if (!per.has(c.open)) per.set(c.open, { c, n: 0 });
      per.get(c.open).n++;
    }
    for (const { c, n } of per.values()) {
      const args = (t.slice(c.open + 1, c.close).replace(/#[^\n]*/g, '').match(/"[^"]*"|[^\s"]+/g) || []);
      const name = args.shift();
      const files = args.filter(a => !/^(STATIC|SHARED|MODULE|OBJECT|INTERFACE|UNKNOWN|EXCLUDE_FROM_ALL|WIN32|MACOSX_BUNDLE|PRIVATE|PUBLIC|IMPORTED|GLOBAL|ALIAS)$/.test(a));
      if (files.length <= n) empties.push(String(name || '').replace(/^"|"$/g, ''));
    }
  }
  for (const r0 of refs) {
    if ([...whole.values()].some(c => r0.s > c.open && r0.s < c.close)) continue;
    // (1009 review (new item #6): "Motor/a b.cpp" -- its quotes go with it; "" was left, an empty source at configure)
    const r = t[r0.s - 1] === '"' && t[r0.e] === '"' ? { s: r0.s - 1, e: r0.e + 1 } : r0;
    const ls = t.lastIndexOf('\n', r.s - 1) + 1;
    let le = t.indexOf('\n', r.e); if (le < 0) le = t.length;
    const before = t.slice(ls, r.s), after = t.slice(r.e, le).replace(/\r$/, '');
    if (!/\S/.test(before) && /^\s*(#.*)?$/.test(after)) out.push({ s: ls, e: le < t.length ? le + 1 : le });
    else { const sp = /^[ \t]*/.exec(t.slice(r.e))[0].length; out.push({ s: r.s, e: r.e + (/^[ \t]*\)/.test(t.slice(r.e)) ? 0 : sp) }); }
  }
  out.sort((a, b) => a.s - b.s);
  out.empties = empties;
  return out;
}

/**
 * 1008 (full test, audit F2: tests\CMakeLists.txt names the tree's sources as "../Public/cBootLog.cpp" and
 * "${CMAKE_SOURCE_DIR}/vclcompat/TrayCore.cpp" -- a rename / delete that looked at the root file only left them broken):
 * every way a CMakeLists.txt in `cmDir` (from the tree, '' = the root) may write the tree path `rel`.
 * -> [{ spell, of(newRel) }] -- of() writes a new path the same way
 */
function spellings(rel, cmDir) {
  const r = String(rel).replace(/\\/g, '/').replace(/^\.\//, '');
  const d = String(cmDir || '').replace(/\\/g, '/').replace(/\/$/, '');
  const up = d ? d.split('/').map(() => '..').join('/') + '/' : '';
  const out = [];
  const add = (spell, of) => { if (spell && !out.some(x => x.spell === spell)) out.push({ spell, of }); };
  for (const v of ['${CMAKE_SOURCE_DIR}/', '${PROJECT_SOURCE_DIR}/']) add(v + r, n => v + n);
  // (1009 review (solution #3): ${CMAKE_CURRENT_LIST_DIR} as well -- the root's include(${CMAKE_CURRENT_LIST_DIR}/ui/native/...))
  const CUR = ['${CMAKE_CURRENT_SOURCE_DIR}/', '${CMAKE_CURRENT_LIST_DIR}/'];
  if (!d) { add(r, n => n); for (const v of CUR) add(v + r, n => v + n); }
  else if (r.startsWith(d + '/')) {
    const inner = r.slice(d.length + 1);
    add(inner, n => (n.startsWith(d + '/') ? n.slice(d.length + 1) : up + n));
    for (const v of CUR) add(v + inner, n => v + (n.startsWith(d + '/') ? n.slice(d.length + 1) : up + n));
  } else {
    add(up + r, n => up + n);
    for (const v of CUR) add(v + up + r, n => v + up + n);
  }
  return out;
}

/** every place a CMakeLists.txt in cmDir names `rel` (any spelling) -> [{ s, e, spell, of }] in text order */
function refsAll(text, rel, cmDir) {
  const out = [];
  for (const sp of spellings(rel, cmDir)) for (const r of refsOf(text, sp.spell)) out.push(Object.assign({}, r, { spell: sp.spell, of: sp.of }));
  return out.sort((a, b) => a.s - b.s).filter((r, i, a) => !i || r.s >= a[i - 1].e);
}

/** the places that name a source UNDER folder `relDir` (any spelling) -> [{ s, e, path }] (for a folder renamed / deleted) */
function refsUnder(text, relDir, cmDir) {
  const t = String(text || '');
  const out = [];
  // (1008 review: the folder itself too -- include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/PickPlanner) was left
  //  when the folder was renamed, and the next build could not find its headers; and not in # comments: 12 of Public's 26
  //  "uses" were comments, counted in the delete warning and rewritten by a rename)
  const cm = commentsOf(t);
  const inC = at => cm.some(x => at >= x[0] && at < x[1]);
  for (const sp of spellings(String(relDir).replace(/\/$/, '') + '/__x__', cmDir)) {
    const pre = sp.spell.slice(0, -'__x__'.length);
    // (1009 review (solution #1): the CMakeLists.txt's own folder (tests\ renamed) -- an empty prefix matched all 10,035 words)
    if (!pre) continue;
    const re = new RegExp('(^|[\\s("])(' + escRe(pre) + '[^\\s)"#]+)', 'gmi');
    let m;
    while ((m = re.exec(t))) { const s = m.index + m[1].length; if (!inC(s)) out.push({ s, e: s + m[2].length, path: m[2], prefix: pre, of: sp.of }); }
    const bare = pre.replace(/\/$/, '');
    // (with a path only ("${CMAKE_CURRENT_SOURCE_DIR}/Public", "Common/PickPlanner") and in its own case: a bare top
    //  folder "Public" matched CMake's PUBLIC keyword 45 times)
    if (!bare || !bare.includes('/')) continue;
    const reB = new RegExp('(^|[\\s("])(' + escRe(bare) + ')(?=[\\s)"]|$)', 'gm');
    while ((m = reB.exec(t))) { const s = m.index + m[1].length; if (!inC(s)) out.push({ s, e: s + m[2].length, path: m[2], prefix: m[2], of: sp.of, folder: true }); }
  }
  return out.sort((a, b) => a.s - b.s).filter((r, i, a) => !i || r.s >= a[i - 1].e);
}

module.exports = { targets, targetsFor, insertSource, refsOf, removeSource, spellings, refsAll, refsUnder, callAt };
