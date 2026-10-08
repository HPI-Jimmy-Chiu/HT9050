'use strict';
// AI(W906-HTDESIGNER) 20261008 (EastSun「我需要保證debug 模式下中斷後 按F8 F7 F9 功能都跟BCB一樣」): C++ Builder 6's debugger
// keys while the program is stopped at a breakpoint -- written into the USER keybindings.json, because only that level
// always wins: an extension's own keybindings tie with other extensions' (CMake Tools binds F7 to cmake.build whenever a
// CMake project is open -- measured on ES02), and which one VS Code picks then depends on load order.
// AI(W906-HTDESIGNER) 20261008 (full test, audit B1-B4): the designer's entries are found by STRUCTURE -- a single-pass
// JSONC scan (strings and comments together), every object of the top array whose "when" names
// config.ht9045Designer.bcbDebugKeys is ours, removed whole with its comma -- not by "a line with the marker": a formatted
// file (one object over several lines) or an entry VS Code's UI appended after ours used to be cut in half or deleted.
// Plain Node (no vscode).

const MARK = '// ht9045Designer.bcbDebugKeys';
const OWN = 'config.ht9045Designer.bcbDebugKeys';
const WHEN = "inDebugMode && debugState == 'stopped' && " + OWN;
const KEYS = [
  { key: 'f8', command: 'workbench.action.debug.stepOver', when: WHEN },                               // Step Over
  { key: 'f7', command: 'workbench.action.debug.stepInto', when: WHEN },                               // Trace Into
  { key: 'f9', command: 'workbench.action.debug.continue', when: WHEN },                               // Run
  { key: 'f4', command: 'editor.debug.action.runToCursor', when: WHEN + ' && editorTextFocus' },        // Run to Cursor
  { key: 'ctrl+f2', command: 'workbench.action.debug.stop', when: 'inDebugMode && ' + OWN },           // Program Reset
  // (1008 review, EastSun「不用等我決定一律選最合理」: BCB6's F5 = Toggle Breakpoint -- while stopped VS Code's F5 is Continue,
  //  so a BCB hand setting a breakpoint ran the machine program on. While stopped only: not debugging, F5 still builds
  //  and starts, as every day)
  { key: 'f5', command: 'editor.debug.action.toggleBreakpoint', when: WHEN + ' && editorTextFocus' },   // Toggle Breakpoint
  // (1008 gap list #3-#5: the rest of BCB6's Run menu while stopped)
  { key: 'shift+f8', command: 'workbench.action.debug.stepOut', when: WHEN },                          // Run Until Return
  { key: 'shift+f7', command: 'workbench.action.debug.stepInto', when: WHEN },                         // Trace to Next Source Line
  { key: 'ctrl+f7', command: 'editor.debug.action.selectionToRepl', when: 'inDebugMode && editorTextFocus && ' + OWN },   // Evaluate/Modify
  { key: 'ctrl+f5', command: 'editor.debug.action.selectionToWatch', when: 'inDebugMode && editorTextFocus && ' + OWN },  // Add Watch
  { key: 'alt+f5', command: 'editor.debug.action.showDebugHover', when: 'inDebugMode && editorTextFocus && ' + OWN },     // Inspect
  // (gap list #1: Make -- BCB Ctrl+F9; Visual Studio's Ctrl+Shift+B in the C++ tree, whose default build task needs
  //  C:\MinGW, absent on a PC without the oracle compiler)
  { key: 'ctrl+f9', command: 'ht9045Designer.run.make', when: '!inDebugMode && ' + OWN },
  { key: 'ctrl+shift+b', command: 'ht9045Designer.run.make', when: '!inDebugMode && ht9045Designer.portTree && ' + OWN },
  // (gap list #8: BCB6's View > Debug Windows -- not in the designer, whose Ctrl+Alt+V / T are its own)
  { key: 'ctrl+alt+b', command: 'workbench.debug.action.focusBreakpointsView', when: "activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+s', command: 'workbench.debug.action.focusCallStackView', when: "activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+w', command: 'workbench.debug.action.focusWatchView', when: "activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+l', command: 'workbench.debug.action.focusVariablesView', when: "activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+c', command: 'debug.action.openDisassemblyView', when: 'inDebugMode && editorTextFocus && ' + OWN },
  // (gap list #20: the rest of BCB6's Debug Windows that VS Code has -- FPU = the Registers of the Variables view, Threads =
  //  the Call Stack (threads listed), Event Log = the Debug Console; while debugging, not in the designer (its own Ctrl+Alt+T
  //  / V). Modules: no such view in VS Code; Memory: needs a hex editor extension -- not mapped)
  { key: 'ctrl+alt+f', command: 'workbench.debug.action.focusVariablesView', when: "inDebugMode && activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+t', command: 'workbench.debug.action.focusCallStackView', when: "inDebugMode && activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  { key: 'ctrl+alt+v', command: 'workbench.panel.repl.view.focus', when: "inDebugMode && activeCustomEditorId != 'ht9045Designer.editor' && " + OWN },
  // (gap list #23: Code Templates -- BCB6's Ctrl+J, the snippets of this tree)
  { key: 'ctrl+j', command: 'editor.action.insertSnippet', when: "editorTextFocus && !editorReadonly && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  // (gap list #10: Compile Unit -- the real compiler on this file, BCB6's Alt+F9)
  { key: 'alt+f9', command: 'ht9045Designer.cpp.compileUnit', when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  // (gap list #9: the .h <-> .cpp of this file, BCB6's Ctrl+F6)
  { key: 'ctrl+f6', command: 'C_Cpp.SwitchHeaderSource', when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  // (gap list #15 / #24: BCB6's editor keys in a C / C++ editor -- Alt+G Go to Line Number, Ctrl+E Incremental Search,
  //  Ctrl+R Replace, Ctrl+Shift+I / U Indent / Unindent Block; VS Code's own Ctrl+E / Ctrl+R / Ctrl+Shift+U stay elsewhere)
  { key: 'alt+g', command: 'workbench.action.gotoLine', when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  { key: 'ctrl+e', command: 'actions.find', when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  { key: 'ctrl+r', command: 'editor.action.startFindReplaceAction', when: "editorTextFocus && !editorReadonly && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  { key: 'ctrl+shift+i', command: 'editor.action.indentLines', when: "editorTextFocus && !editorReadonly && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  { key: 'ctrl+shift+u', command: 'editor.action.outdentLines', when: "editorTextFocus && !editorReadonly && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  // (gap list #18: To-Do -- Ctrl+Shift+T adds "// TODO: " above the line, as BCB6's Add To-Do Item)
  { key: 'ctrl+shift+t', command: 'ht9045Designer.todo.add', when: "editorTextFocus && !editorReadonly && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN },
  // (gap list #11: BCB6's bookmarks in a C / C++ editor -- Ctrl+Shift+0..9 set / clear, Ctrl+0..9 go)
  ...[0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map(n => ({ key: 'ctrl+shift+' + n, command: 'ht9045Designer.bookmark.toggle', args: n, when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN })),
  ...[0, 1, 2, 3, 4, 5, 6, 7, 8, 9].map(n => ({ key: 'ctrl+' + n, command: 'ht9045Designer.bookmark.goto', args: n, when: "editorTextFocus && (editorLangId == 'cpp' || editorLangId == 'c') && " + OWN })),
  // (gap list #2, safety: CMake Tools' keys -- F7 builds the build\ folder (CLAUDE.md: never touched), Shift+F7 a target of
  //  it, Shift+F5 / Ctrl+Shift+F5 start wb_serve without the W906_* settings (the machine's own files written). Removed.)
  { key: 'f7', command: '-cmake.build' },
  { key: 'shift+f7', command: '-cmake.buildWithTarget' },
  { key: 'shift+f5', command: '-cmake.debugTarget' },
  { key: 'ctrl+shift+f5', command: '-cmake.launchTarget' },
];

const line = k => '  { "key": ' + JSON.stringify(k.key) + ', "command": ' + JSON.stringify(k.command) + (k.args !== undefined ? ', "args": ' + JSON.stringify(k.args) : '') + (k.when ? ', "when": ' + JSON.stringify(k.when) : '') + ' }';
// (a removal rule cannot carry our when -- it would then remove nothing: ours by its exact key + command)
const NEG = new Set(KEYS.filter(k => !k.when).map(k => k.key + '|' + k.command));

/**
 * One pass over JSONC text: the top-level array's [start, end] and each top-level object in it { s, e, text } -- strings
 * (with escapes) and // / block comments skipped together, so a quote in a comment or a bracket in a string misleads
 * nothing. -> { open, close, objs } or null when there is no top array.
 */
function scan(text) {
  const t = String(text || '');
  let i = 0, depth = 0, open = -1, close = -1, objStart = -1;
  const objs = [];
  while (i < t.length) {
    const c = t[i];
    if (c === '"') { i++; while (i < t.length && t[i] !== '"') { if (t[i] === '\\') i++; i++; } i++; continue; }
    if (c === '/' && t[i + 1] === '/') { while (i < t.length && t[i] !== '\n') i++; continue; }
    if (c === '/' && t[i + 1] === '*') { const e = t.indexOf('*/', i + 2); i = e < 0 ? t.length : e + 2; continue; }
    if (c === '[' || c === '{') {
      if (depth === 0 && c === '[' && open < 0) open = i;
      else if (depth === 1 && c === '{' && open >= 0 && close < 0) objStart = i;
      depth++;
    } else if (c === ']' || c === '}') {
      depth--;
      if (depth === 1 && c === '}' && objStart >= 0) { objs.push({ s: objStart, e: i + 1, text: t.slice(objStart, i + 1) }); objStart = -1; }
      if (depth === 0 && c === ']' && open >= 0 && close < 0) close = i;
    }
    i++;
  }
  if (open < 0 || close < 0) return null;
  return { open, close, objs };
}

const isOurs = o => {
  if (o.text.indexOf(OWN) >= 0) return true;
  try { const j = JSON.parse(o.text); return !!j && !j.when && NEG.has(String(j.key) + '|' + String(j.command)); } catch (e) { return false; }
};

/** keybindings.json text without the designer's entries (each removed whole, with the comma after / before it). */
function remove(text) {
  let t = String(text || '');
  const sc = scan(t);
  if (!sc) return t.split(/\r?\n/).filter(l => l.indexOf(MARK) < 0).join(/\r\n/.test(t) ? '\r\n' : '\n');
  for (const o of sc.objs.filter(isOurs).reverse()) {
    let s = o.s, e = o.e;
    // (its comma: the one after it, else the one before it; then the rest of its line when only blanks / our old marker)
    const after = /^\s*,/.exec(t.slice(e));
    if (after) e += after[0].length;
    else { const before = /,\s*$/.exec(t.slice(sc.open + 1, s)); if (before) s -= before[0].length; }
    const rest = /^[ \t]*(\/\/ ht9045Designer\.bcbDebugKeys)?[ \t]*(\r?\n)?/.exec(t.slice(e));
    if (rest) e += rest[0].length;
    const lead = /(^|\n)[ \t]*$/.exec(t.slice(0, s));
    if (lead && rest && rest[2]) s -= lead[0].length - lead[1].length;
    t = t.slice(0, s) + t.slice(e);
  }
  return t;
}

/**
 * keybindings.json text -> the text with the designer's five entries (replacing older ones), or null when it already has
 * exactly them. No file / no array (comments only) = an array added after what is there. -> string | null | { error }
 */
function ensure(text) {
  const t0 = String(text || '').replace(/^﻿/, '');
  const eol = /\r\n/.test(t0) ? '\r\n' : '\n';
  const want = KEYS.map(line);
  const sc0 = scan(t0);
  if (sc0) {
    const have = sc0.objs.filter(isOurs).map(o => o.text.replace(/\s+/g, ' ').trim());
    if (have.length === want.length && want.every((w, i) => have[i] === w.trim().replace(/\s+/g, ' '))) return null;
  }
  const t = remove(t0);
  const sc = scan(t);
  if (!sc) {
    if (/[[{]/.test(t.replace(/"(?:[^"\\]|\\.)*"|\/\/[^\n]*|\/\*[\s\S]*?\*\//g, ''))) return { error: 'keybindings.json 的格式認不得（找不到 [ ... ]）' };
    // (comments only -- old bindings kept commented out stay as they are)
    return (t.trim() ? t.replace(/\s*$/, '') + eol : '// Place your key bindings in this file to override the defaults' + eol) + '[' + eol + want.join(',' + eol) + eol + ']' + eol;
  }
  // (after the last entry: a comma there when it has none; ours separated by commas, no trailing comma)
  let head = t.slice(0, sc.close), tail = t.slice(sc.close);
  const last = sc.objs[sc.objs.length - 1];
  if (last && !/^\s*,/.test(t.slice(last.e, sc.close))) head = t.slice(0, last.e) + ',' + t.slice(last.e, sc.close);
  head = head.replace(/[ \t]*$/, '');
  if (!/\n$/.test(head)) head += eol;
  return head + want.join(',' + eol) + eol + tail;
}

module.exports = { MARK, OWN, KEYS, ensure, remove, scan };
