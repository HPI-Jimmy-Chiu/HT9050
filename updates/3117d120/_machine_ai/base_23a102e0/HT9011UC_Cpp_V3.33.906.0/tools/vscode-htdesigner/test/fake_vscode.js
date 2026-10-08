'use strict';
// AI(W906-HTDESIGNER) 20260929: the small slice of the `vscode` API extension.js uses,
// recorded instead of performed, so the extension can be exercised under plain Node.
const path = require('path');

const calls = [];
const rec = (name, args) => { calls.push({ name, args }); };
const disp = () => ({ dispose() {} });

class Uri {
  constructor(scheme, fsPath) { this.scheme = scheme; this.fsPath = fsPath; }
  static file(p) { return new Uri('file', path.normalize(p)); }
  static joinPath(u, ...parts) { return new Uri(u.scheme, path.join(u.fsPath, ...parts)); }
  with(o) { return new Uri(o.scheme || this.scheme, this.fsPath); }
  toString() { return this.scheme + ':///' + this.fsPath.replace(/\\/g, '/'); }
}
class EventEmitter {
  constructor() { this.fns = []; this.event = fn => { this.fns.push(fn); return disp(); }; }
  fire(v) { this.fns.forEach(f => f(v)); }
}
class Position { constructor(line, character) { this.line = line; this.character = character; } }
class Range { constructor(a, b, c, d) { if (a instanceof Position) { this.start = a; this.end = b; } else { this.start = new Position(a, b); this.end = new Position(c, d); } } }
class Selection extends Range {}
class TreeItem { constructor(label, state) { this.label = label; this.collapsibleState = state; } }
class ThemeIcon { constructor(id, color) { this.id = id; this.color = color; } }
class ThemeColor { constructor(id) { this.id = id; } }
class DataTransferItem { constructor(value) { this.value = value; } asString() { return Promise.resolve(typeof this.value === 'string' ? this.value : JSON.stringify(this.value)); } }
class Diagnostic { constructor(range, message, severity) { this.range = range; this.message = message; this.severity = severity; } }
class CodeAction { constructor(title, kind) { this.title = title; this.kind = kind; } }
class DiagnosticCollection {
  constructor() { this._m = new Map(); }
  set(uri, d) { this._m.set(uri.toString(), d); }
  delete(uri) { this._m.delete(uri.toString()); }
  has(uri) { return this._m.has(uri.toString()); }
  get(uri) { return this._m.get(uri.toString()); }
  clear() { this._m.clear(); }
  dispose() {}
}
class DataTransfer { constructor() { this._m = new Map(); } set(k, v) { this._m.set(k, v); } get(k) { return this._m.get(k); } }
class RelativePattern { constructor(base, glob) { this.base = base; this.glob = glob; } }
class Disposable { constructor(fn) { this.fn = fn; } dispose() { if (this.fn) this.fn(); } }
class CodeLens { constructor(range, command) { this.range = range; this.command = command; } }
class WorkspaceEdit {
  constructor() { this.ops = []; this.renames = []; }
  replace(uri, range, text) { this.ops.push({ uri, range, text }); }
  insert(uri, pos, text) { this.ops.push({ uri, range: { start: pos, end: pos }, text }); }
  // (1008: a rename is done on disk ONLY under the OS temp folder -- the tests' own files; anything else is refused)
  renameFile(from, to) { this.renames.push({ from, to }); }
}
const FileSystemError = {
  FileNotFound: u => Object.assign(new Error('FileNotFound ' + u), { code: 'FileNotFound' }),
  NoPermissions: u => Object.assign(new Error('NoPermissions ' + u), { code: 'NoPermissions' }),
};

const config = {};
const state = {
  visibleTextEditors: [],
  quickPick: items => items[0],
  opened: [],
};

const vscode = {
  Uri, EventEmitter, Position, Range, Selection, TreeItem, ThemeIcon, ThemeColor, RelativePattern, DataTransfer, DataTransferItem,
  Diagnostic, CodeAction, DiagnosticSeverity: { Error: 0, Warning: 1, Information: 2, Hint: 3 }, CodeActionKind: { QuickFix: 'quickfix' },
  Disposable, CodeLens, WorkspaceEdit, FileSystemError, FileType: { File: 1, Directory: 2 }, FilePermission: { Readonly: 1 },
  ConfigurationTarget: { Global: 1, Workspace: 2, WorkspaceFolder: 3 },
  languages: {
    registerCodeLensProvider: (sel, p) => { vscode._lens = p; return disp(); },
    createDiagnosticCollection: () => (vscode._diag = new DiagnosticCollection()),
    registerCodeActionsProvider: (sel, p) => { vscode._codeActions = p; return disp(); },
  },
  StatusBarAlignment: { Left: 1, Right: 2 },
  ProgressLocation: { SourceControl: 1, Window: 10, Notification: 15 },
  TreeItemCollapsibleState: { None: 0, Collapsed: 1, Expanded: 2 },
  ViewColumn: { Active: -1, Beside: -2, One: 1, Two: 2, Three: 3 },
  TextEditorSelectionChangeKind: { Keyboard: 1, Mouse: 2, Command: 3 },
  TextEditorRevealType: { Default: 0, InCenter: 1, InCenterIfOutsideViewport: 2, AtTop: 3 },
  window: {
    createOutputChannel: () => ({ lines: [], appendLine(s) { this.lines.push(s); }, show() {}, dispose() {} }),
    createTreeView: (id, o) => {
      const tv = { id, visible: false, reveal: () => Promise.resolve(), dispose() {}, _sel: null,
        onDidChangeSelection: fn => { tv._sel = fn; return disp(); },
        onDidChangeVisibility: fn => { tv._vis = fn; return disp(); } };
      // _tree / _treeView = the component tree (the tests' old name); every view by id in _views
      (vscode._views = vscode._views || {})[id] = { provider: o.treeDataProvider, view: tv, dnd: o.dragAndDropController || null };
      if (id === 'ht9045Designer.components') { vscode._tree = o.treeDataProvider; vscode._treeView = tv; }
      return tv;
    },
    createStatusBarItem: () => ({ show() { this.shown = true; }, hide() { this.shown = false; }, dispose() {} }),
    // _props = the 屬性與事件 panel (the tests' old name); every webview view by id in _webviewViews
    registerWebviewViewProvider: (id, p) => {
      rec('registerWebviewViewProvider', [id]);
      (vscode._webviewViews = vscode._webviewViews || {})[id] = p;
      if (id === 'ht9045Designer.properties') vscode._props = p;
      return disp();
    },
    // _editor = the designer (the tests' old name); every custom editor by view type in _editors
    registerCustomEditorProvider: (id, p) => {
      rec('registerCustomEditorProvider', [id]);
      (vscode._editors = vscode._editors || {})[id] = p;
      if (id === 'ht9045Designer.editor') vscode._editor = p;
      return disp();
    },
    onDidChangeTextEditorSelection: () => disp(),
    get visibleTextEditors() { return state.visibleTextEditors; },
    get activeTextEditor() { return null; },
    showInformationMessage: (m) => { rec('info', [m]); return Promise.resolve(); },
    showErrorMessage: (m) => { rec('error', [m]); return Promise.resolve(); },
    // a modal question: answered with vscode._answer (undefined = closed without an answer)
    showWarningMessage: (m, ...rest) => { rec('warning', [m]); return Promise.resolve(vscode._answer); },
    setStatusBarMessage: (m) => { rec('status', [m]); return disp(); },
    showQuickPick: (items, o) => { rec('quickPick', [items.map(i => i.label + ' ' + i.description), o && o.placeHolder]); return Promise.resolve(state.quickPick(items)); },
    showTextDocument: (doc, o) => {
      state.opened.push({ uri: doc.uri, selection: o && o.selection, viewColumn: o && o.viewColumn, text: doc.text, bom: doc.bom, readonly: doc.readonly });
      return Promise.resolve({ revealRange() {}, selection: null });
    },
    showOpenDialog: () => Promise.resolve(undefined),
    withProgress: (o, fn) => Promise.resolve(fn({ report() {} })),
    createWebviewPanel: (viewType, title) => {
      const p = {
        viewType, title, posted: [], recv: null, disposed: false,
        webview: {
          cspSource: 'https://*.vscode-cdn.net', html: '', options: null,
          asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
          postMessage: m => { p.posted.push(m); return Promise.resolve(true); },
          onDidReceiveMessage: fn => { p.recv = fn; return disp(); },
        },
        reveal() {}, onDidDispose: () => disp(),
      };
      (vscode._panels = vscode._panels || []).push(p);
      return p;
    },
  },
  workspace: {
    // (1008: deleting only under the OS temp folder -- the tests' own files)
    fs: { delete: async (uri, o) => { const p = uri.fsPath; if (!p.toLowerCase().startsWith(require('os').tmpdir().toLowerCase())) throw new Error('fake fs.delete outside temp: ' + p); require('fs').rmSync(p, { recursive: !!(o && o.recursive), force: true }); rec('fsDelete', [p, !!(o && o.useTrash)]); } },
    get workspaceFolders() { return vscode._folders || []; },
    getConfiguration: () => ({ get: k => config[k] }),
    // a setting changed: vscode._configChanged('zoomWheel') fires it (the key without "ht9045Designer.")
    onDidChangeConfiguration: fn => { (vscode._cfgFns = vscode._cfgFns || []).push(fn); return disp(); },
    registerFileSystemProvider: (s, p, o) => { vscode._fs = { scheme: s, p, o }; return disp(); },
    createFileSystemWatcher: () => ({ onDidChange() {}, onDidCreate() {}, onDidDelete() {}, dispose() {} }),
    // (listeners kept; fired only by a test: vscode._fireDocChange(doc) = a change made elsewhere)
    onDidChangeTextDocument: fn => { (vscode._chgFns = vscode._chgFns || []).push(fn); return disp(); },
    get textDocuments() { return Object.values(vscode._docs || {}); },
    // edits go to documents registered in vscode._docs (uri string -> { offsetAt, _setText, getText })
    applyEdit: async we => {
      rec('applyEditCall', [we.ops.length]);
      for (const r of we.renames || []) {
        const tmp = require('os').tmpdir().toLowerCase();
        if (!r.from.fsPath.toLowerCase().startsWith(tmp) || !r.to.fsPath.toLowerCase().startsWith(tmp)) return false;
        require('fs').renameSync(r.from.fsPath, r.to.fsPath);
        rec('renameFile', [r.from.fsPath, r.to.fsPath]);
      }
      // apply from the end so earlier offsets stay valid (like VS Code applies a WorkspaceEdit)
      const ops = we.ops.slice().sort((a, b) => b.range.start.line - a.range.start.line || b.range.start.character - a.range.start.character);
      for (const op of ops) {
        const doc = (vscode._docs || {})[op.uri.toString()];
        if (!doc) return false;
        const t = doc.getText();
        const s = doc.offsetAt(op.range.start), e = doc.offsetAt(op.range.end);
        doc._setText(t.slice(0, s) + op.text + t.slice(e));
        rec('applyEdit', [op.uri.toString(), s, e, op.text]);
      }
      return true;
    },
    openTextDocument: async (uri) => {
      if (vscode._fs && uri.scheme === vscode._fs.scheme) {
        const b = Buffer.from(vscode._fs.p.readFile(uri));
        const hasBom = b[0] === 0xef && b[1] === 0xbb && b[2] === 0xbf;
        return { uri, text: b.subarray(hasBom ? 3 : 0).toString('utf8'), bom: hasBom, readonly: !!(vscode._fs.o && vscode._fs.o.isReadonly) };
      }
      // a file: an in-memory document (edits change only this copy -- nothing is ever written)
      const key = uri.toString();
      if (vscode._docs && vscode._docs[key]) return vscode._docs[key];
      // (untitled with a path: a new empty document, nothing on disk)
      let t = uri.scheme === 'untitled' ? '' : require('fs').readFileSync(uri.fsPath, 'utf8'), ls = [0];
      const idx = () => { ls = [0]; for (let i = 0; i < t.length; i++) if (t.charCodeAt(i) === 10) ls.push(i + 1); };
      idx();
      const d = {
        uri, fileName: uri.fsPath, isDirty: false,
        get text() { return t; },
        getText: () => t,
        positionAt: off => { let lo = 0, hi = ls.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (ls[m] <= off) lo = m; else hi = m - 1; } return new Position(lo, off - ls[lo]); },
        offsetAt: p => ls[p.line] + p.character,
        _setText: x => { t = x; idx(); d.isDirty = true; },
      };
      (vscode._docs = vscode._docs || {})[key] = d;
      return d;
    },
  },
  commands: {
    registerCommand: (id, fn) => { (vscode._cmds = vscode._cmds || {})[id] = fn; return disp(); },
    executeCommand: (id, ...a) => { rec('exec', [id].concat(a.map(String))); return Promise.resolve(); },
  },
  env: { clipboard: { writeText: (t) => { state.clip = String(t); return Promise.resolve(); }, readText: () => Promise.resolve(state.clip || '') } },
  _calls: calls, _state: state, _config: config,
  _configChanged: key => (vscode._cfgFns || []).forEach(fn => fn({ affectsConfiguration: s => s === 'ht9045Designer.' + key || s === 'ht9045Designer' })),
};

vscode._fireDocChange = doc => { for (const fn of vscode._chgFns || []) { try { fn({ document: doc, contentChanges: [{ text: '' }], reason: undefined }); } catch (e) { /* a listener of another test */ } } };
module.exports = vscode;
