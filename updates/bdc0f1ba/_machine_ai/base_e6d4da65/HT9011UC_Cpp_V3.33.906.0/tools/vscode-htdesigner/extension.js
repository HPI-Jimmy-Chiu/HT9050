'use strict';
// AI(W906-HTDESIGNER) 20260929: HTML 視覺設計工具 (first built for HT9045; for every BCB6 -> web HMI + C++ software) -- VS Code extension entry.
//
// A WPF-designer-like view of the web HMI pages (web\page\*.html):
//   * the page is rendered in a webview behind a no-network CSP (lib/pagehtml.js);
//   * clicking a component selects it (the page's own handlers do not run);
//   * the 元件 tree and the 屬性與事件 panel show the DFM control, its properties
//     (tools\dfm2rc\ir_out) and its events;
//   * double-clicking an event opens its code: the web JS listener that really
//     runs in the browser, the C++ port (TfXxx::Handler), or the BCB6 original
//     (Big5, opened read-only through a virtual document so it can never be
//     re-saved as UTF-8).
// Nothing here writes to any file, and nothing here can reach the machine.

const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const roots = require('./lib/roots');
const pageinfo = require('./lib/pageinfo');
const { SourceTree, decodeBig5, SKIP_DIR } = require('./lib/cppindex');
const web = require('./lib/websearch');
const fmt = require('./lib/format');
const { buildPageHtml, hasIframe } = require('./lib/pagehtml');
const { ReverseIndex, enclosingMethod } = require('./lib/reverse');
const { WebCmdIndex } = require('./lib/webcmds');
const overview = require('./lib/overview');
const dfmdiff = require('./lib/dfmdiff');
const taborder = require('./lib/taborder');
const mixed = require('./lib/mixed');
const pagelist = require('./lib/pagelist');
const htmlblock = require('./lib/htmlblock');
const toolbox = require('./lib/toolbox');
const pagesearch = require('./lib/pagesearch');
const pagelint = require('./lib/pagelint');
const update = require('./lib/update');
const liveconfig = require('./lib/liveconfig');
const csvtable = require('./lib/csvtable');
const vclevents = require('./lib/vclevents');
const cppstub = require('./lib/cppstub');
const cpptypes = require('./lib/cpptypes');
const projectsearch = require('./lib/projectsearch');
const solutiontree = require('./lib/solutiontree');
const icons = require('./lib/icons');
const cppsymbols = require('./lib/cppsymbols');
const jsevents = require('./lib/jsevents');
const aliasedit = require('./lib/aliasedit');
const cppbridge = require('./lib/cppbridge');
const CSV_VIEW = 'ht9045Designer.csv';
const LINT_SOURCE = 'HTML 頁面檢查';
const htmledit = require('./lib/htmledit');
const lex = require('./lib/cpplex');

const sleep = ms => new Promise(r => setTimeout(r, ms));

const attrsOf = t => htmledit.attrsOf(t);
/** Attributes the grid may change: text-like ones. Never the id (the page's JS finds it by it) or on* handlers. */
const ATTR_EDIT = /^(title|class|placeholder|tabindex|alt|data-[\w-]+|aria-[\w-]+)$/;
/** A CSS property name / value the grid writes into a style="…" (no way out of the declaration;
    a quote is fine: htmledit.setStyle turns it into the other kind). */
const CSS_NAME = /^(--)?[a-z][a-z0-9-]*$/;
const CSS_BAD = /[;{}<>\\]/;

const VIEW_TYPE = 'ht9045Designer.editor';
const GOLDEN_SCHEME = 'ht9045-golden';
const CONTAINER = 'workbench.view.extension.ht9045-designer';

const KIND = {
  web: { icon: '$(globe)', label: '網頁 JS' },
  port: { icon: '$(symbol-class)', label: 'C++ 移植樹' },
  golden: { icon: '$(history)', label: 'BCB6 原始碼（唯讀）' },
  dfm: { icon: '$(symbol-structure)', label: 'BCB6 表單 .dfm（唯讀）' },
  html: { icon: '$(code)', label: 'HTML 原始碼' },
};

let hub = null;

function activate(context) {
  hub = new Hub(context);
  // exposed for test\integration (vscode.extensions.getExtension(...).exports)
  return { hub };
}

function deactivate() {
  hub = null;
}

// ---------------------------------------------------------------------------
class Hub {
  constructor(ctx) {
    this.ctx = ctx;
    this.out = vscode.window.createOutputChannel('HTML 視覺設計');
    this.designers = new Set();
    this.active = null;
    this.irStores = new Map();
    this.trees = new Map();
    this.watchers = new Map();
    this.selSeq = 0;
    this.propsDesigner = null;
    this.warnedOperate = false;
    // AI(W906-HTDESIGNER) 20261001: the designer's own build files (HtdEvents.gen.cpp, the server's htd.event branch,
    // the CMake entry) go to disk at once, through here -- never left unsaved for a build to trip on (EastSun 1001:
    // "你這邊建置時 可能有問題"). The tests put an in-memory one in its place.
    this.disk = {
      read: f => { try { return fs.readFileSync(f); } catch (e) { return null; } },
      write: (f, buf) => {
        fs.mkdirSync(path.dirname(f), { recursive: true });
        const tmp = f + '.htd-' + process.pid + '.tmp';
        fs.writeFileSync(tmp, buf);
        fs.renameSync(tmp, f);
      },
    };
    this.tree = new ComponentTree(this);
    // WPF document outline: drag a component onto a container to move it in there
    this.treeDnd = new ComponentDnd(this);
    this.treeView = vscode.window.createTreeView('ht9045Designer.components', {
      treeDataProvider: this.tree, showCollapseAll: true, canSelectMany: true, dragAndDropController: this.treeDnd,
    });
    // WPF document outline: Ctrl/Shift+click in the tree = multi-select on the page
    // (a single click is the item's command; the tree's own reveal is ignored here)
    this.treeView.onDidChangeSelection(e => {
      if (this.tree.revealing || !this.active) return;
      const sel = (e.selection || []).filter(n => n && n.key);
      if (sel.length < 2) return;
      const ids = sel.map(n => (n.isForm ? '@form' : n.id)).filter(Boolean);
      const focus = this.tree.lastClicked && sel.find(n => n.key === this.tree.lastClicked);
      if (focus) ids.unshift(ids.splice(ids.indexOf(focus.isForm ? '@form' : focus.id), 1)[0]);
      this.active.post({ type: 'selectIds', ids, origin: 'tree' });
    });
    // 頁面: all pages of the web folder, a click switches the designer to one
    this.pages = new PageTree(this);
    this.pageView = vscode.window.createTreeView('ht9045Designer.pages', { treeDataProvider: this.pages, showCollapseAll: true });
    if (this.pageView.onDidChangeVisibility) this.pageView.onDidChangeVisibility(e => { if (e.visible) this.pages.revealActive(); });
    // the version this window runs, in sight: after installing a new one, a window not reloaded still runs the old
    this.version = (ctx.extension && ctx.extension.packageJSON && ctx.extension.packageJSON.version) || '';
    if (this.version) this.pageView.description = 'v' + this.version;
    // 工具箱: new components (the generator's markup)
    this.toolboxView = vscode.window.createTreeView('ht9045Designer.toolbox', { treeDataProvider: new ToolboxTree() });
    // 專案搜尋 (0.137): one keyword over the port / web / golden trees, every hit listed
    this.projSearch = new ProjectSearch(this);
    this.projSearchView = vscode.window.createTreeView('ht9045Designer.projectSearch', { treeDataProvider: this.projSearch, showCollapseAll: true });
    this.projSearch.view = this.projSearchView;
    // 方案總管 (0.138): Visual Studio's Solution Explorer over the same three trees
    this.solution = new SolutionTree(this);
    // (0.148: drawn by the extension -- the search box right under the title, the tree under it, one view)
    this.solPanel = new SolutionPanel(this);
    this.solutionView = this.solPanel.adapter;
    this.applyTabScrollbar();
    this.applySectionFrames();
    this.solution.view = this.solutionView;
    if (this.version) { this.solution.versionText = 'v' + this.version; this.solution.setDescription(); }
    this.props = new PropsView(this);
    // 搜尋頁面: the search box above the 頁面 list
    this.pageSearch = new PageSearchView(this);
    this.status = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 40);
    this.status.command = 'ht9045Designer.toggleMode';
    // the page switcher, always in sight (the 頁面 list can be collapsed or hidden in the side bar)
    this.pageStatus = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 41);
    this.pageStatus.command = 'ht9045Designer.openPage';
    this.pageStatus.text = '$(file-code) 選擇頁面';
    this.pageStatus.tooltip = '選一個 HTML 頁面，用設計檢視開啟';
    this.pageStatus.show();
    // 有新版: a newer install runs only after the window is reloaded -- found by itself, one button to take it
    // (EastSun: "你不能加個按鈕讓我 reload 嗎 ... 就算我接受別人也沒辦法接受")
    this.updateItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 42);
    this.updateItem.command = 'ht9045Designer.reloadWindow';
    this.newerVersion = null;
    const updTimer = setInterval(() => this.checkUpdate(), 60000);
    if (updTimer && updTimer.unref) updTimer.unref();
    ctx.subscriptions.push({ dispose: () => clearInterval(updTimer) });
    if (vscode.window.onDidChangeWindowState) ctx.subscriptions.push(vscode.window.onDidChangeWindowState(e => { if (e.focused) this.checkUpdate(); }));
    const updFirst = setTimeout(() => this.checkUpdate(), 3000);
    if (updFirst && updFirst.unref) updFirst.unref();
    // CSV 表格: a .csv open as text -- a tab from before the update comes back as text (VS Code keeps the kind
    // of editor a tab had): the table one click away, and asked once per file
    // (EastSun 20260930: "依照圖片 表格沒有出現阿 我已經有更新過")
    this.csvItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 43);
    this.csvItem.command = 'ht9045Designer.openCsvTable';
    this.csvItem.text = '$(table) 用表格開啟';
    this.csvAsked = new Set();
    if (vscode.window.onDidChangeActiveTextEditor) ctx.subscriptions.push(vscode.window.onDidChangeActiveTextEditor(ed => this.csvTextEditor(ed)));
    const csvFirst = setTimeout(() => this.csvTextEditor(vscode.window.activeTextEditor), 1500);
    if (csvFirst && csvFirst.unref) csvFirst.unref();
    // 執行列 (0.142, EastSun: "我需要在固定的地方 可以讓我啟動 暫停 停止 該軟體"): Visual Studio's ▶ ▾ ⏸ ⏹ ↻ step buttons, always
    // at the left of the status bar (VS Code's own debug toolbar shows only while debugging)
    this.runBar = new RunBar(ctx);
    // the zoomWheel / snapSpacing setting changed: every open designer takes it at once (no reopening)
    if (vscode.workspace.onDidChangeConfiguration) ctx.subscriptions.push(vscode.workspace.onDidChangeConfiguration(e => {
      if (e.affectsConfiguration('ht9045Designer.zoomWheel')) {
        const how = this.wheelZoom();
        for (const d of this.designers) d.post({ type: 'wheelZoom', how });
        this.updateStatus();
      }
      if (e.affectsConfiguration('ht9045Designer.snapSpacing')) {
        const px = this.snapSpacing();
        for (const d of this.designers) d.post({ type: 'snapSpacing', px });
      }
    }));
    const reg = (id, fn) => vscode.commands.registerCommand(id, fn);
    ctx.subscriptions.push(
      this.out, this.treeView, this.pageView, this.toolboxView, this.status, this.pageStatus, this.updateItem, this.csvItem,
      reg('ht9045Designer.reloadWindow', () => this.cmdReloadWindow()),
      // C++ navigation (0.140): the breadcrumbs' classes / members, the 專案 / 類別 / 成員 pickers
      ...(vscode.languages.registerDocumentSymbolProvider ? [vscode.languages.registerDocumentSymbolProvider([{ language: 'cpp' }, { language: 'c' }], this.cppNav = new CppNav(this), { label: 'HTML 設計工具：C++ 導覽' })] : []),
      reg('ht9045Designer.navProject', () => (this.cppNav || (this.cppNav = new CppNav(this))).cmdNav('project')),
      reg('ht9045Designer.navClass', () => (this.cppNav || (this.cppNav = new CppNav(this))).cmdNav('class')),
      reg('ht9045Designer.navMember', () => (this.cppNav || (this.cppNav = new CppNav(this))).cmdNav('member')),
      // 分頁 (0.141, EastSun: "可以調整左右的 可以看其他方案 並且有選項可以關閉其他方案"): the list of the open ones (Visual
      // Studio's ▾ at the end of the tab row), close the others -- VS Code's own commands. 0.142 (EastSun: "左右可以滾輪 不是選擇
      // 檔案"): the 0.141 ◀ ▶ switched the file; no API scrolls the tab row, so the row gets the large scrollbar instead
      // (the wheel over the row scrolls it too) -- tabScrollbar toggles it
      reg('ht9045Designer.tabScrollbar', () => this.cmdTabScrollbar()),
      reg('ht9045Designer.sectionFrames', () => this.cmdSectionFrames()),
      // 尋找 (0.144): Visual Studio's Find in Files -- the keyword, the scope, the options in one box (Ctrl+Shift+F)
      reg('ht9045Designer.find', a => this.projSearch.cmdFind(a)),
      reg('ht9045Designer.tabList', () => vscode.commands.executeCommand('workbench.action.showAllEditorsByMostRecentlyUsed')),
      reg('ht9045Designer.tabCloseOthers', () => vscode.commands.executeCommand('workbench.action.closeOtherEditors')),
      // 上下版面 (0.140): a C++ file opened closes the page
      ...(vscode.window.onDidChangeActiveTextEditor ? [vscode.window.onDidChangeActiveTextEditor(ed => this.onActiveEditor(ed))] : []),
      // 方案總管 (0.138)
      // (0.148: one webview view -- the box and the tree; right-click items get { htdId } -> solPanel.nodeOf)
      vscode.window.registerWebviewViewProvider('ht9045Designer.solution', this.solPanel, { webviewOptions: { retainContextWhenHidden: true } }),
      reg('ht9045Designer.solutionFilter', q => this.solution.cmdFilter(typeof q === 'string' ? q : undefined)),
      reg('ht9045Designer.solutionClearFilter', () => this.solution.setFilter('')),
      reg('ht9045Designer.solutionRefresh', () => { this.solution.cache.clear(); return this.solution.q ? this.solution.setFilter(this.solution.q) : this.solution.refresh(); }),
      reg('ht9045Designer.solutionCollapseAll', () => this.solPanel.collapseAll()),
      reg('ht9045Designer.solutionOpen', n => this.solution.open(this.solPanel.nodeOf(n))),
      reg('ht9045Designer.solutionReveal', f => this.solution.cmdReveal(typeof f === 'string' ? f : undefined)),
      reg('ht9045Designer.solutionSearchIn', a => { const n = this.solPanel.nodeOf(a); return n && n.path ? this.projSearch.cmdSearchHere({ q: n.q, fromTree: true, scope: n.type === 'file' ? 'file' : n.type === 'dir' ? 'folder' : 'project', file: n.path }) : null; }),
      reg('ht9045Designer.solutionCopyPath', a => { const n = this.solPanel.nodeOf(a); return n && n.path ? Promise.resolve(vscode.env.clipboard.writeText(n.path)).then(() => n.path) : null; }),
      reg('ht9045Designer.solutionRevealInOS', a => { const n = this.solPanel.nodeOf(a); return n && n.path ? vscode.commands.executeCommand('revealFileInOS', vscode.Uri.file(n.path)) : null; }),
      // 專案搜尋 (0.137)
      this.projSearchView,
      reg('ht9045Designer.projectSearchRun', q => this.projSearch.cmdSearch(typeof q === 'string' ? q : undefined)),
      reg('ht9045Designer.projectSearchOpen', h => this.projSearch.open(h)),
      reg('ht9045Designer.projectSearchHere', a => this.projSearch.cmdSearchHere(a && typeof a === 'object' && !a.scheme ? a : undefined)),
      reg('ht9045Designer.projectSearchCopy', () => this.projSearch.cmdCopy()),
      reg('ht9045Designer.projectSearchClear', () => this.projSearch.cmdClear()),
      reg('ht9045Designer.projectSearchCase', () => this.projSearch.toggle('caseSensitive')),
      reg('ht9045Designer.projectSearchCaseOn', () => this.projSearch.toggle('caseSensitive')),
      reg('ht9045Designer.projectSearchWord', () => this.projSearch.toggle('wholeWord')),
      reg('ht9045Designer.projectSearchWordOn', () => this.projSearch.toggle('wholeWord')),
      reg('ht9045Designer.projectSearchRegex', () => this.projSearch.toggle('regex')),
      reg('ht9045Designer.projectSearchRegexOn', () => this.projSearch.toggle('regex')),
      vscode.window.registerWebviewViewProvider('ht9045Designer.pageSearch', this.pageSearch),
      // CSV 表格: a .csv as a table, edited like Excel (only the changed cells' text is replaced)
      vscode.window.registerCustomEditorProvider(CSV_VIEW, this.csv = new CsvTableEditor(this), { webviewOptions: { retainContextWhenHidden: true } }),
      reg('ht9045Designer.openCsvTable', uri => this.cmdOpenCsvTable(uri)),
      // 搜尋頁面: filter the 頁面 list (the box, or a command with the words); '' = every page again
      reg('ht9045Designer.filterPages', q => this.filterPages(typeof q === 'string' ? q : '')),
      reg('ht9045Designer.clearPageFilter', () => this.filterPages('')),
      // (0.139: 搜尋頁面 is 方案總管's search now -- Ctrl+; there)
      reg('ht9045Designer.focusPageSearch', () => this.solution.cmdFilter()),
      reg('ht9045Designer.openPageAt', (file, id) => (typeof file === 'string' && file ? this.openPageAt(file, id) : null)),
      // 機種: which machine the pages are shown for (machine-only tabs / components)
      reg('ht9045Designer.pickMachine', arg => this.cmdPickMachine(arg)),
      vscode.window.registerWebviewViewProvider('ht9045Designer.properties', this.props, {
        webviewOptions: { retainContextWhenHidden: true },
      }),
      vscode.window.registerCustomEditorProvider(VIEW_TYPE, {
        resolveCustomTextEditor: (doc, panel) => this.createDesigner(doc, panel),
      }, { webviewOptions: { retainContextWhenHidden: true }, supportsMultipleEditorsPerDocument: false }),
      // golden (Big5) through a READ-ONLY file system: the editor cannot change it at
      // all, so it can never be saved back as UTF-8 (a content provider still lets
      // the API edit the buffer, and "Save As" could then land on the original path)
      vscode.workspace.registerFileSystemProvider(GOLDEN_SCHEME, new GoldenFs(), {
        isReadonly: true, isCaseSensitive: false,
      }),
      reg('ht9045Designer.open', uri => this.cmdOpen(uri)),
      reg('ht9045Designer.openPage', () => this.cmdOpenPage()),
      reg('ht9045Designer.showPages', () => this.cmdShowPages()),
      // the 頁面 list: open a page in the designer / as HTML text; read the folder again
      reg('ht9045Designer.openPageFile', file => {
        if (typeof file === 'string' && file) return vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(file), VIEW_TYPE);
      }),
      reg('ht9045Designer.openPageText', n => {
        const f = n && n.file ? n.file : (this.active ? this.active.file : null);
        if (f) return vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(f), 'default', vscode.ViewColumn.Beside);
      }),
      reg('ht9045Designer.refreshPages', () => this.pages.refresh()),
      reg('ht9045Designer.searchPages', q => this.cmdSearchPages(typeof q === 'string' ? q : undefined)),
      // hidden in the designer only (the eye in the component tree / the design surface's menu)
      reg('ht9045Designer.designHide', n => this.designHide(n, true)),
      reg('ht9045Designer.designUnhide', n => this.designHide(n, false)),
      reg('ht9045Designer.designToggle', n => this.designHide(n)),
      reg('ht9045Designer.designUnhideAll', () => { if (this.active) this.applyDesignHidden(this.active, []); }),
      // locked in the designer only (the lock in the component tree / the design surface's menu)
      reg('ht9045Designer.designLock', n => this.designLock(n, true)),
      reg('ht9045Designer.designUnlock', n => this.designLock(n, false)),
      reg('ht9045Designer.designLockToggle', n => this.designLock(n)),
      reg('ht9045Designer.designUnlockAll', () => { if (this.active) this.applyDesignLocked(this.active, []); }),
      reg('ht9045Designer.designLockAll', () => this.designLockAll()),
      // WPF Document Outline: Ctrl+H hide, Shift+Ctrl+H show, Ctrl+L lock, Shift+Ctrl+L unlock -- the selection
      // (from the component tree: its selected rows)
      reg('ht9045Designer.designKey', a => this.cmdDesignKey(a && a.cmd, !!(a && a.from === 'tree'))),
      // WPF: F2 on the design surface = edit the control's text right there (renaming: the Name box, F2 in the tree)
      reg('ht9045Designer.editText', () => {
        const d = this.active;
        if (!d) return null;
        if (!(d.sel && d.sel.info && d.sel.info.id)) { vscode.window.setStatusBarMessage('$(info) 先選一個元件，再按 F2 改它的文字', 4000); return null; }
        d.post({ type: 'editText' });
        return { asked: d.sel.info.id };
      }),
      // Ctrl+Z / Ctrl+Y while that text box is open: its typing, never the page's undo
      reg('ht9045Designer.textUndo', a => { const d = this.active; if (d && d.textEditing) d.post({ type: 'textUndo', redo: !!(a && a.redo) }); return !!(d && d.textEditing); }),
      // AI(W906-HTDESIGNER) 20261001: the CSV table's own Excel keys -- the table handles them; VS Code's binding of the
      // same key must not run too (Ctrl+R = Open Recent, Ctrl+- / Ctrl+Shift+= = zoom, Ctrl+Z while a cell is edited
      // = undo of the FILE). package.json binds them to this in the table only.
      reg('ht9045Designer.csvKey', () => true),
      // WPF: Alt+arrow = duplicate the selection (the copy moved 1 px that way; Shift = 10 px) -- the Ctrl+drag copy
      reg('ht9045Designer.duplicate', a => {
        const d = this.active;
        if (!d) return null;
        return this.cmdCopyDrop(d, { ids: this.selIds(d), dx: Math.round(+(a && a.dx) || 0), dy: Math.round(+(a && a.dy) || 0) });
      }),
      // WPF Delete / Cut / Copy / Paste of whole components (the page is drawn again from the source)
      // (from the Document Outline's right-click menu the row comes along: that component is selected first -- onNode)
      reg('ht9045Designer.deleteComponent', async arg => { await this.onNode(arg); return this.cmdDelete(arg && typeof arg === 'object' && 'confirmed' in arg ? arg : null); }),
      reg('ht9045Designer.cutComponent', async arg => { await this.onNode(arg); return this.cmdCut(arg && typeof arg === 'object' && 'confirmed' in arg ? arg : null); }),
      reg('ht9045Designer.copyComponent', async n => { await this.onNode(n); return this.cmdCopy(); }),
      // the Document Outline's own keys (WPF: the outline does what the design surface does): F2, Ctrl+C / X / V, Delete on
      // the tree's selected row -- the arrow keys move the tree's selection without the page knowing, so it is selected
      // first (several rows: the page already has them all)
      reg('ht9045Designer.outlineKey', async a => {
        const sel = this.treeView && this.treeView.selection ? this.treeView.selection.filter(n => n && n.key != null) : [];
        if (sel.length === 1) await this.onNode(sel[0]);
        const run = { rename: () => this.cmdRename(), copy: () => this.cmdCopy(), cut: () => this.cmdCut(null), paste: () => this.cmdPaste(), delete: () => this.cmdDelete(null) }[a && a.cmd];
        return run ? run() : null;
      }),
      reg('ht9045Designer.pasteComponent', async n => { await this.onNode(n); return this.cmdPaste(); }),
      // WPF Order
      ...['front', 'forward', 'backward', 'back'].map(how => reg('ht9045Designer.order.' + how, async n => { await this.onNode(n); return this.cmdOrder(how); })),
      // 工具箱 (WPF Toolbox): a click on an item adds one
      reg('ht9045Designer.toolboxAdd', cls => this.cmdAdd(typeof cls === 'string' ? cls : cls && cls.cls)),
      reg('ht9045Designer.toolboxArm', cls => this.toolboxArm(typeof cls === 'string' ? cls : cls && cls.cls)),
      // WPF: "select an element in the Toolbox and press Enter" = added (like a double-click)
      // (the arrow keys move the tree's focus, Enter selects: VS Code selects the focused item first -- its own Enter --
      // and that item's click command, pick-up, is skipped for the moment)
      reg('ht9045Designer.toolboxAddSelected', async () => {
        this.toolboxEnter = Date.now();
        try { await vscode.commands.executeCommand('list.select'); } catch (e) { /* the selection as it is */ }
        await new Promise(r => setTimeout(r, 80));
        const it = this.toolboxView && this.toolboxView.selection && this.toolboxView.selection[0];
        if (!it) return null;
        if (it.cls === POINTER.cls) return this.toolboxPointer();
        this.lastArm = null;
        this.toolboxBackToPointer();
        if (this.active) { this.active.armed = null; this.active.post({ type: 'placeArm', cls: null }); }
        return this.cmdAdd(it.cls);
      }),
      reg('ht9045Designer.rename', async (name, confirmed) => { await this.onNode(name); return this.cmdRename(typeof name === 'string' ? name : undefined, confirmed === true); }),
      reg('ht9045Designer.addComponent', () => this.cmdAddPick()),
      // Blend Group Into (a new panel) / Ungroup
      reg('ht9045Designer.groupIntoPanel', async n => { await this.onNode(n); return this.cmdGroup(); }),
      reg('ht9045Designer.ungroup', async n => { await this.onNode(n); return this.cmdUngroup(); }),
      // WPF "snap to gridlines"
      reg('ht9045Designer.toggleGrid', on => this.cmdToggleGrid(typeof on === 'boolean' ? on : undefined)),
      reg('ht9045Designer.toggleMode', () => this.cmdToggleMode()),
      reg('ht9045Designer.reload', () => { if (this.active) this.active.render(); }),
      reg('ht9045Designer.showSource', () => this.cmdShowSource()),
      reg('ht9045Designer.revealSource', node => this.cmdRevealSource(node)),
      reg('ht9045Designer.selectKey', key => {
        this.tree.lastClicked = key;
        if (this.active) this.active.post({ type: 'selectKey', key, origin: 'tree' });
      }),
      reg('ht9045Designer.showInfo', () => this.cmdShowInfo()),
      reg('ht9045Designer.revealDfm', node => this.cmdRevealDfm(node)),
      reg('ht9045Designer.findComponent', () => this.cmdFindComponent()),
      reg('ht9045Designer.showInDesigner', entries => this.cmdShowInDesigner(entries)),
      reg('ht9045Designer.showCodeInDesigner', () => this.cmdShowCodeInDesigner()),
      reg('ht9045Designer.viewDesigner', () => this.cmdViewDesigner()),
      reg('ht9045Designer.gotoEventHandler', () => this.cmdGotoEventHandler()),
      reg('ht9045Designer.openWebSenders', (cmd, hits) => this.cmdOpenWebSenders(cmd, hits)),
      reg('ht9045Designer.pageOverview', () => this.cmdPageOverview()),
      reg('ht9045Designer.toggleWireMarks', on => this.cmdWireMarks(typeof on === 'boolean' ? on : undefined)),
      reg('ht9045Designer.toggleDfmGhosts', on => this.cmdDfmGhosts(typeof on === 'boolean' ? on : undefined)),
      reg('ht9045Designer.resetToDfm', async n => { await this.onNode(n); return this.cmdResetToDfm(); }),
      // WPF Layout > Reset <property> / Reset All: only the position, only the size, the whole layout
      ...['pos', 'size', 'layout'].map(only => reg('ht9045Designer.resetLayout.' + only, async n => { await this.onNode(n); return this.cmdResetToDfm(null, only); })),
      reg('ht9045Designer.toggleTabOrder', on => this.cmdTabOrder(typeof on === 'boolean' ? on : undefined)),
      // every component's name on the design surface (on / off per designer)
      reg('ht9045Designer.toggleNames', on => {
        const d = this.active;
        if (!d) return null;
        d.showNames = typeof on === 'boolean' ? on : !d.showNames;
        d.post({ type: 'showNames', on: d.showNames });
        this.updateStatus();
        return { on: d.showNames };
      }),
      reg('ht9045Designer.dfmDiff', () => this.cmdDfmDiff()),
      reg('ht9045Designer.resetZoom', () => { if (this.active) this.active.post({ type: 'setZoom', zoom: 1 }); }),
      reg('ht9045Designer.zoomFit', () => { if (this.active) this.active.post({ type: 'zoomFit' }); }),
      reg('ht9045Designer.zoomSelection', () => { if (this.active) this.active.post({ type: 'zoomSel' }); }),
      // Blend: Ctrl+= / Ctrl+- zoom in / out one step (the artboard toolbar's + / -)
      reg('ht9045Designer.zoomIn', () => { if (this.active) this.active.post({ type: 'zoomStep', dir: 1 }); return !!this.active; }),
      reg('ht9045Designer.zoomOut', () => { if (this.active) this.active.post({ type: 'zoomStep', dir: -1 }); return !!this.active; }),
      reg('ht9045Designer.help', () => vscode.commands.executeCommand('markdown.showPreview', vscode.Uri.joinPath(ctx.extensionUri, 'README.md'))),
      reg('ht9045Designer.cheatsheet', () => vscode.commands.executeCommand('markdown.showPreview', vscode.Uri.joinPath(ctx.extensionUri, 'CHEATSHEET.md'))),
      // the design surface's right-click menu (webview/context)
      // (F7 / View Code only opens code -- WPF's View Code opens the code-behind; a double-click creates the default event)
      reg('ht9045Designer.viewCode', async n => { await this.onNode(n); if (this.active) return this.onDesignerDblClick(this.active, true); }),
      reg('ht9045Designer.selectParent', () => { if (this.active) this.active.post({ type: 'selectParent' }); }),
      reg('ht9045Designer.selectHere', () => this.cmdSelectHere()),
      reg('ht9045Designer.selectAll', () => { if (this.active) this.active.post({ type: 'selectAll' }); }),
      reg('ht9045Designer.selectSameType', () => { if (this.active) this.active.post({ type: 'selectSameType', scope: 'page' }); }),
      reg('ht9045Designer.selectSameTypeHere', () => { if (this.active) this.active.post({ type: 'selectSameType', scope: 'container' }); }),
      // undo / redo from the designer's title bar (the page source's own undo stack)
      reg('ht9045Designer.undo', () => { if (this.active) { this.active.panel.reveal(this.active.panel.viewColumn, false); return vscode.commands.executeCommand('undo'); } }),
      reg('ht9045Designer.redo', () => { if (this.active) { this.active.panel.reveal(this.active.panel.viewColumn, false); return vscode.commands.executeCommand('redo'); } }),
      // (AI 20261001: + WinForms' / BCB6's Format menu -- Align to Grid, Size to Grid, Horizontal / Vertical Spacing
      //  Increase / Decrease / Remove)
      ...['left', 'right', 'top', 'bottom', 'hcenter', 'vcenter', 'width', 'height', 'size', 'hspace', 'vspace', 'hcenterIn', 'vcenterIn',
        'gridPos', 'gridSize', 'hspaceInc', 'hspaceDec', 'hspaceRemove', 'vspaceInc', 'vspaceDec', 'vspaceRemove'].map(how =>
        reg('ht9045Designer.align.' + how, () => { if (this.active) this.active.post({ type: 'align', how }); })),
      vscode.languages.registerCodeLensProvider([{ language: 'cpp' }, { language: 'c' }], this.lens = new HandlerLens(this)),
      // 頁面檢查: the Problems panel + the quick fix of a cut attribute; a checked page is checked again as it changes
      this.lintDiag = vscode.languages.createDiagnosticCollection('ht9045Designer'),
      vscode.languages.registerCodeActionsProvider([{ language: 'html' }, { scheme: 'file', pattern: '**/*.html' }], new LintFixes(this),
        { providedCodeActionKinds: [vscode.CodeActionKind.QuickFix] }),
      vscode.workspace.onDidChangeTextDocument(e => {
        if (!this.lintDiag || !this.lintDiag.has(e.document.uri)) return;
        clearTimeout(this._lintT);
        this._lintT = setTimeout(() => this.lintFile(e.document.uri.fsPath), 700);
      }),
      reg('ht9045Designer.lintPage', () => this.cmdLint('page')),
      reg('ht9045Designer.lintAll', () => this.cmdLint('all')),
      // from the 頁面 list: one page, without opening it
      reg('ht9045Designer.lintPageFile', n => {
        if (!n || !n.file) return null;
        const issues = this.lintFile(n.file);
        if (issues.length) vscode.commands.executeCommand('workbench.actions.view.problems').then(() => {}, () => {});
        vscode.window.setStatusBarMessage('$(checklist) ' + path.basename(n.file) + '：' + (issues.length ? issues.length + ' 個問題（看「問題」面板）' : '沒有問題'), 6000);
        return issues.length;
      }),
      reg('ht9045Designer.lintFixAll', uri => this.cmdLintFixAll(uri)),
      reg('ht9045Designer.lintFixDfmGaps', uri => this.cmdLintFixDfmGaps(uri)),
      reg('ht9045Designer.netCheck', () => {
        const d = this.active;
        if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return; }
        d.netcheckNotify = true;
        d.post({ type: 'netcheck' });
      }),
      vscode.window.onDidChangeTextEditorSelection(e => this.onEditorSelection(e)),
    );
  }

  log(s) {
    const t = new Date();
    const hh = String(t.getHours()).padStart(2, '0') + ':' + String(t.getMinutes()).padStart(2, '0') + ':' + String(t.getSeconds()).padStart(2, '0');
    this.out.appendLine('[' + hh + '] ' + s);
    this.logLines = this.logLines || [];
    this.logLines.push('[' + hh + '] ' + s);
    if (this.logLines.length > 300) this.logLines.shift();
  }

  cfg() { return vscode.workspace.getConfiguration('ht9045Designer'); }
  // 0.142 the tab row's scrollbar: 'large' once when the user has never set it (an explicit value -- even 'default' -- stays)
  applyTabScrollbar() {
    const c = vscode.workspace.getConfiguration('workbench.editor');
    if (!c.inspect || !c.update) return false;
    const i = c.inspect('titleScrollbarSizing');
    if (!i || i.globalValue !== undefined || i.workspaceValue !== undefined) return false;
    c.update('titleScrollbarSizing', 'large', vscode.ConfigurationTarget.Global);
    return true;
  }
  // 0.147 (EastSun: "每個項目 你可以像 visual studio 外框有個跟底部一樣顏色的外框 這樣比較好辨識嗎?"): no API draws a frame around
  // a side bar section, so its title bar gets Visual Studio's tool-window band (a solid background + a border line) --
  // workbench.colorCustomizations, only for the theme in use, only keys the user has not set, once (then the toggle)
  sectionFrameColors() {
    const kind = vscode.window.activeColorTheme ? vscode.window.activeColorTheme.kind : 2;
    return kind === 1 || kind === 4
      ? { 'sideBarSectionHeader.background': '#dcdce4', 'sideBarSectionHeader.border': '#b4b4c0', 'sideBarSectionHeader.foreground': '#1e1e1e' }
      : { 'sideBarSectionHeader.background': '#2d2d30', 'sideBarSectionHeader.border': '#4b4b55', 'sideBarSectionHeader.foreground': '#f1f1f1' };
  }
  async setSectionFrames(on) {
    const wb = vscode.workspace.getConfiguration('workbench');
    if (!wb.update) return false;
    const theme = '[' + (wb.get('colorTheme') || 'Default Dark Modern') + ']';
    const cc = Object.assign({}, (wb.inspect && (wb.inspect('colorCustomizations') || {}).globalValue) || {});
    const mine = Object.assign({}, cc[theme] || {});
    const want = this.sectionFrameColors();
    let changed = false;
    for (const k of Object.keys(want)) {
      if (on && mine[k] === undefined) { mine[k] = want[k]; changed = true; }
      if (!on && mine[k] === want[k]) { delete mine[k]; changed = true; }
    }
    if (!changed) return false;
    if (Object.keys(mine).length) cc[theme] = mine; else delete cc[theme];
    await wb.update('colorCustomizations', cc, vscode.ConfigurationTarget.Global);
    return true;
  }
  applySectionFrames() {
    const gs = this.ctx && this.ctx.globalState;
    if (!gs || gs.get('htd.sectionFrames')) return false;
    gs.update('htd.sectionFrames', 1);
    this.setSectionFrames(true);
    return true;
  }
  async cmdSectionFrames() {
    const wb = vscode.workspace.getConfiguration('workbench');
    const theme = '[' + (wb.get('colorTheme') || 'Default Dark Modern') + ']';
    const cc = (wb.inspect && (wb.inspect('colorCustomizations') || {}).globalValue) || {};
    const on = !(cc[theme] && cc[theme]['sideBarSectionHeader.background']);
    await this.setSectionFrames(on);
    vscode.window.setStatusBarMessage(on ? '側欄分區外框：開（標題列有底色和框線）' : '側欄分區外框：關', 4000);
    return on;
  }
  async cmdTabScrollbar() {
    const c = vscode.workspace.getConfiguration('workbench.editor');
    if (!c.update) return;
    const big = c.get('titleScrollbarSizing') === 'large';
    await c.update('titleScrollbarSizing', big ? 'default' : 'large', vscode.ConfigurationTarget.Global);
    vscode.window.setStatusBarMessage(big ? '分頁列捲軸：恢復細的' : '分頁列捲軸：加粗（滑鼠在分頁列上滾輪也能左右捲）', 4000);
  }
  over() {
    const c = this.cfg();
    return { webRoot: c.get('webRoot') || '', portRoot: c.get('portRoot') || '', goldenRoot: c.get('goldenRoot') || '' };
  }
  wsFolders() { return (vscode.workspace.workspaceFolders || []).map(f => f.uri.fsPath); }
  rootsFor(file) { return roots.resolveRoots(file, this.wsFolders(), this.over()); }

  irStore(root) {
    if (!this.irStores.has(root)) this.irStores.set(root, new pageinfo.IrStore(root));
    return this.irStores.get(root);
  }

  /** Shared index of a source tree; built once in the background. */
  sourceTree(root, kind) {
    if (!root) return null;
    const key = kind + '|' + root.toLowerCase();
    let t = this.trees.get(key);
    if (!t) {
      t = new SourceTree(root, kind);
      this.trees.set(key, t);
      this.log('建立索引：' + (kind === 'port' ? 'C++ 移植樹 ' : 'BCB6 原始碼 ') + root);
      t.ensure().then(() => {
        this.log('索引完成：' + root + '（' + t.files.length + ' 檔，' + t.qual.size + ' 個 Class::名稱，' + t.buildMs + ' ms）');
      }, e => this.log('索引失敗：' + root + ' ' + (e && e.stack || e)));
      if (kind === 'port') {
        this.watch(root, '**/*.{cpp,h,hpp,c,cc,cxx,inc,inl}', uri => {
          const rel = path.relative(root, uri.fsPath);
          if (rel.split(path.sep).slice(0, -1).some(seg => SKIP_DIR.test(seg))) return;
          t.rescan(uri.fsPath).catch(() => {});
        });
      }
    }
    return t;
  }

  watch(root, glob, fn) {
    const key = root.toLowerCase() + '|' + glob;
    if (this.watchers.has(key)) return;
    try {
      const w = vscode.workspace.createFileSystemWatcher(new vscode.RelativePattern(vscode.Uri.file(root), glob));
      w.onDidChange(fn); w.onDidCreate(fn); w.onDidDelete(fn);
      this.ctx.subscriptions.push(w);
      this.watchers.set(key, w);
    } catch (e) {
      this.log('無法監看 ' + root + '：' + e);
    }
  }

  // --- designers -----------------------------------------------------------
  createDesigner(doc, panel) {
    const d = new Designer(this, doc, panel);
    this.designers.add(d);
    this.sourceTree(d.r.portRoot, 'port');
    this.sourceTree(d.r.goldenRoot, 'golden');
    if (d.r.webRoot) {
      this.watch(d.r.webRoot, '**/*.{js,css}', uri => this.onWebAssetChanged(uri));
    }
    this.log('開啟 ' + d.file + '\n    web=' + d.r.webRoot + '\n    port=' + d.r.portRoot + '\n    ir=' + d.r.irRoot + '\n    golden=' + d.r.goldenRoot);
    this.setActive(d);
    if (!this.treeView.visible) {
      vscode.commands.executeCommand(CONTAINER).then(() => panel.reveal(panel.viewColumn, false), () => {});
    }
  }

  onWebAssetChanged(uri) {
    const f = uri.fsPath;
    clearTimeout(this._assetTimer);
    this._assetTimer = setTimeout(() => {
      for (const d of this.designers) {
        d.info.then(pi => {
          const hit = pi.scripts.some(s => web.samePath(s, f)) ||
            (/\.css$/i.test(f) && web.samePath(path.dirname(f), d.pageDir));
          if (hit) { this.log('重新載入（' + path.basename(f) + ' 改了）：' + path.basename(d.file)); d.render(); }
        }, () => {});
      }
    }, 600);
  }

  removeDesigner(d) {
    this.designers.delete(d);
    this.pages.mark();
    if (this.active === d) {
      const next = Array.from(this.designers).find(x => x.panel.visible) || null;
      this.setActive(next);
    }
  }

  setActive(d) {
    if (this.active === d) return;
    this.active = d;
    this.tree.set(d);
    vscode.commands.executeCommand('setContext', 'ht9045Designer.anyDesignHidden', !!(d && d.designHidden && d.designHidden.size));
    vscode.commands.executeCommand('setContext', 'ht9045Designer.anyDesignLocked', !!(d && d.designLocked && d.designLocked.size));
    vscode.commands.executeCommand('setContext', 'ht9045Designer.gridOn', this.gridState().on);
    this.pages.mark();
    this.pages.revealActive();
    vscode.commands.executeCommand('setContext', 'ht9045Designer.active', !!d);
    this.updateStatus();
    if (d && d.pendingSel) {             // it changed selection while in the background
      const p = d.pendingSel;
      d.pendingSel = null;
      this.onSelect(d, p.info, p.origin);
      return;
    }
    this.showProps(d, d && d.sel ? d.sel.data : null);
  }

  // --- hidden in the designer only (the eye; the workspace state, never the source) ---
  loadDesignHidden(file) { return this.loadDesignSet('htd.designHidden', file); }

  /** A per-page list of names kept in the workspace state (hidden / locked in the designer). */
  loadDesignSet(key, file) {
    const all = this.ctx.workspaceState ? this.ctx.workspaceState.get(key, {}) : {};
    const v = all && all[String(file).toLowerCase()];
    return Array.isArray(v) ? v.filter(x => typeof x === 'string' && x) : [];
  }

  saveDesignSet(key, file, list) {
    if (!this.ctx.workspaceState) return;
    const all = Object.assign({}, this.ctx.workspaceState.get(key, {}));
    if (list.length) all[String(file).toLowerCase()] = list; else delete all[String(file).toLowerCase()];
    this.ctx.workspaceState.update(key, all);
  }

  /** Locked in the designer (the lock): it or a component around it. The locked name, or ''. */
  lockedName(d, id) {
    if (!d || !d.designLocked || !d.designLocked.size || !id) return '';
    if (d.designLocked.has(id)) return id;
    let row = d.treeData.find(r => r[2] === id);
    const seen = new Set();
    while (row && !seen.has(row[0])) {
      seen.add(row[0]);
      const name = row[7] ? '@form' : row[2];
      if (d.designLocked.has(name)) return name;
      row = row[1] ? d.treeData.find(r => r[0] === row[1]) : null;
    }
    return '';
  }

  applyDesignLocked(d, ids) {
    if (!d) return;
    d.designLocked = new Set(ids.filter(x => typeof x === 'string' && x && x !== '@form'));
    const list = Array.from(d.designLocked);
    this.saveDesignSet('htd.designLocked', d.file, list);
    d.post({ type: 'designLocked', ids: list });
    if (this.active === d) {
      this.tree.refresh();
      vscode.commands.executeCommand('setContext', 'ht9045Designer.anyDesignLocked', list.length > 0);
      // the properties panel: its position / size fields follow the lock
      if (d.sel && d.sel.info) this.onSelect(d, d.sel.info, 'lock');
    }
  }

  /**
   * 全部鎖定 (WinForms / BCB6 Format > Lock Controls -- "locks the form's size as well"; the pair of 全部解除鎖定):
   * every component right under the form, so all that is inside them too (the form itself is never locked here).
   */
  designLockAll() {
    const d = this.active;
    if (!d || !d.treeData) return null;
    const form = d.treeData.find(r => r[7]);
    const top = form ? d.treeData.filter(r => r[1] === form[0] && r[2] && !r[7]).map(r => r[2]) : [];
    if (!top.length) { vscode.window.setStatusBarMessage('$(info) 表單上沒有元件可以鎖定', 4000); return null; }
    this.applyDesignLocked(d, Array.from(new Set([...(d.designLocked || []), ...top])));
    vscode.window.setStatusBarMessage('$(lock) 全部鎖定：表單上的 ' + top.length + ' 個元件（和它們裡面的）在設計畫面不能移動、改大小、刪除；「全部解除鎖定」解開（原始碼沒改）', 6000);
    return { locked: top };
  }

  designLock(n, lock) {
    const d = this.active;
    if (!d) return;
    const id = n && typeof n.id === 'string' ? (n.isForm ? '' : n.id) : (d.sel && d.sel.info ? d.sel.info.id : '');
    if (!id || id === '@form') { vscode.window.setStatusBarMessage('$(info) 表單本身不能鎖定', 4000); return; }
    const ids = new Set(d.designLocked);
    if (lock === undefined) lock = !ids.has(id);
    if (lock) ids.add(id); else ids.delete(id);
    this.applyDesignLocked(d, Array.from(ids));
    vscode.window.setStatusBarMessage('$(' + (lock ? 'lock' : 'unlock') + ') ' + id + (lock ? ' 鎖定了：在設計畫面不能移動、改大小、刪除（裡面的元件也一樣；原始碼沒改）' : ' 解除鎖定'), 5000);
  }

  /**
   * WPF Document Outline keys: hide / show / lock / unlock every selected component (the design
   * surface's selection, or the tree's selected rows); the form itself is left out.
   */
  cmdDesignKey(cmd, fromTree) {
    const d = this.active;
    if (!d || !/^(hide|show|lock|unlock)$/.test(cmd || '')) return null;
    const ids = (fromTree
      ? (this.treeView && this.treeView.selection || []).filter(n => n && typeof n.id === 'string' && !n.isForm).map(n => n.id)
      : this.selIds(d)).filter(x => x && x !== '@form');
    const hide = cmd === 'hide' || cmd === 'show';
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選元件（表單本身不能' + (hide ? '隱藏' : '鎖定') + '）', 4000); return null; }
    const on = cmd === 'hide' || cmd === 'lock';
    const set = new Set(hide ? d.designHidden : d.designLocked);
    for (const id of ids) { if (on) set.add(id); else set.delete(id); }
    if (hide) this.applyDesignHidden(d, Array.from(set)); else this.applyDesignLocked(d, Array.from(set));
    const what = { hide: '在設計檢視隱藏了（只影響畫面，原始碼沒改；Shift+Ctrl+H 顯示）', show: '顯示回來了',
      lock: '鎖定了（在設計畫面不能移動、改大小、刪除；Shift+Ctrl+L 解鎖）', unlock: '解除鎖定' }[cmd];
    vscode.window.setStatusBarMessage('$(' + { hide: 'eye-closed', show: 'eye', lock: 'lock', unlock: 'unlock' }[cmd] + ') ' + ids.join('、') + ' ' + what, 5000);
    return { cmd, ids };
  }

  /** Refused when one of the names is locked: true (and the status says so). */
  refuseLocked(d, ids, verb) {
    for (const id of ids) {
      const lk = this.lockedName(d, id);
      if (lk) { this.refuseEdit(d, '「' + lk + '」鎖定了（元件樹上的鎖頭），不能' + verb + (lk !== id ? '（' + id + ' 在它裡面）' : '') + '。'); return true; }
    }
    return false;
  }

  /** ids = the names to hide (a toggle adds / removes one; [] shows everything again). */
  applyDesignHidden(d, ids) {
    if (!d) return;
    d.designHidden = new Set(ids.filter(x => typeof x === 'string' && x && x !== '@form'));
    const list = Array.from(d.designHidden);
    if (this.ctx.workspaceState) {
      const all = Object.assign({}, this.ctx.workspaceState.get('htd.designHidden', {}));
      if (list.length) all[d.file.toLowerCase()] = list; else delete all[d.file.toLowerCase()];
      this.ctx.workspaceState.update('htd.designHidden', all);
    }
    d.post({ type: 'designHidden', ids: list });
    if (this.active === d) {
      this.tree.refresh();
      vscode.commands.executeCommand('setContext', 'ht9045Designer.anyDesignHidden', list.length > 0);
    }
  }

  designHide(n, hide) {
    const d = this.active;
    if (!d) return;
    // from the tree: its node; from the design surface's menu: the selection
    const id = n && typeof n.id === 'string' ? (n.isForm ? '' : n.id) : (d.sel && d.sel.info ? d.sel.info.id : '');
    if (!id || id === '@form') { vscode.window.setStatusBarMessage('$(info) 表單本身不能隱藏', 4000); return; }
    const ids = new Set(d.designHidden);
    if (hide === undefined) hide = !ids.has(id);
    if (hide) ids.add(id); else ids.delete(id);
    this.applyDesignHidden(d, Array.from(ids));
    vscode.window.setStatusBarMessage('$(eye' + (hide ? '-closed' : '') + ') ' + id + (hide ? ' 在設計檢視隱藏了（只影響畫面，原始碼沒改）' : ' 顯示回來了'), 5000);
  }

  /** WPF "snap to gridlines": on / off for every page (the workspace state); the size is a setting. */
  gridState() {
    const on = !!(this.ctx.workspaceState && this.ctx.workspaceState.get('htd.gridOn', false));
    // WPF: "Show/hide snap grid" and "snapping to gridlines" are two buttons -- shown (never set: as the snapping)
    const sv = this.ctx.workspaceState ? this.ctx.workspaceState.get('htd.gridShow', undefined) : undefined;
    const show = typeof sv === 'boolean' ? sv : on;
    const size = Math.max(2, Math.min(64, Math.round(+vscode.workspace.getConfiguration('ht9045Designer').get('gridSize') || 8)));
    return { on, show, size };
  }

  /** WPF's "Zoom by using" (Options > XAML Designer): which wheel zooms the design surface -- 'ctrl' (Ctrl+wheel, the
   *  default), 'wheel' (the wheel alone), 'alt' (Alt+wheel). */
  wheelZoom() {
    const v = String(vscode.workspace.getConfiguration('ht9045Designer').get('zoomWheel') || 'ctrl');
    return v === 'wheel' || v === 'alt' ? v : 'ctrl';
  }

  /** WPF's "Default zoom setting": a page opens at the zoom it had last time ('last', WPF's Last Used -- the default),
   *  fitting the view ('fit', Fit All), or at 100% ('100'). */
  defaultZoom() {
    const v = String(vscode.workspace.getConfiguration('ht9045Designer').get('defaultZoom') || 'last');
    return v === 'fit' || v === '100' ? v : 'last';
  }
  /** The spacing snaplines' distance (WPF's snapping spacing / margin; BCB6's grid step 8 by default; 0 = none). */
  snapSpacing() {
    const v = vscode.workspace.getConfiguration('ht9045Designer').get('snapSpacing');
    const n = Math.round(+(v == null ? 8 : v));
    return isFinite(n) ? Math.max(0, Math.min(64, n)) : 8;
  }
  /** WPF's "Default document view": a page opens as the design surface alone ('design', the default) or with its
   *  HTML beside it ('split', WPF's Split view). */
  defaultView() {
    // (0.140: 'wpf' = the design above, the page's source below, closed when a C++ file is opened -- EastSun's default)
    const v = String(vscode.workspace.getConfiguration('ht9045Designer').get('defaultView') || 'wpf');
    return v === 'split' || v === 'design' ? v : 'wpf';
  }

  /**
   * AI(W906-HTDESIGNER) 20261001 (0.140, EastSun: "如果點擊頁面 跟 WPF 一樣 上面是畫面 下面是對應原始碼；我切到 C++ 時要自動關閉
   * 該頁面"; "編輯區希望只出現 C++ 原始碼"): the editor area in two rows -- the design surface above, the page's HTML
   * below (WPF's designer over its XAML). The page before it (if any) is closed first: one page at a time.
   */
  async openWpf(d) {
    const prev = this.wpfSession;
    if (prev && !web.samePath(prev.file, d.file)) await this.closePageSession('another page');
    this.wpfSession = { file: d.file, d, at: Date.now() };
    try {
      await vscode.commands.executeCommand('vscode.setEditorLayout', { orientation: 1, groups: [{ size: 0.62 }, { size: 0.38 }] });
      if (d.panel.viewColumn && d.panel.viewColumn !== vscode.ViewColumn.One && d.panel.reveal) d.panel.reveal(vscode.ViewColumn.One, false);
      if (!vscode.window.visibleTextEditors.some(e => e.document === d.doc)) {
        await vscode.window.showTextDocument(d.doc, { viewColumn: vscode.ViewColumn.Two, preview: false, preserveFocus: true });
      }
    } catch (e) { this.log('上下版面：' + (e && e.message || e)); }
  }

  /**
   * A C++ file became the active editor: the pages close, the area back to one (EastSun 1001: the HTML tabs left over
   * should close by themselves too -- 0.141: every .html tab, the designer's and the text ones, not only the last page;
   * a tab with unsaved changes stays). Only in the 'wpf' view.
   */
  async onActiveEditor(ed) {
    if (!ed || !ed.document || !ed.document.uri) return;
    const f = ed.document.uri.fsPath || '';
    if (!/\.(cpp|cc|cxx|c|h|hpp|hxx|inc|inl)$/i.test(f)) return;
    if (this.defaultView() !== 'wpf') return;
    const s = this.wpfSession;
    if (s && Date.now() - s.at < 800) return;   // (the page's own opening moments)
    await this.closePageSession('C++', { allHtml: true });
  }

  /** Close the page of the session: its design surface and its HTML tab (not when it has unsaved changes), then one group. */
  /**
   * Close the page of the session -- or, opts.allHtml, every .html tab (designer or text) -- not one with unsaved
   * changes; then the area back to one group. -> { closed, kept: [names left open] }
   */
  async closePageSession(why, opts) {
    const s = this.wpfSession;
    this.wpfSession = null;
    const all = !!(opts && opts.allHtml);
    if (!s && !all) return { closed: 0, kept: [] };
    let closed = 0;
    const kept = [];
    const tg = vscode.window.tabGroups;
    if (tg && tg.all) {
      const tabs = [];
      for (const g of tg.all) for (const t of g.tabs) {
        const u = t.input && t.input.uri;
        if (!u || !u.fsPath) continue;
        if (all ? !/\.html?$/i.test(u.fsPath) : !(s && web.samePath(u.fsPath, s.file))) continue;
        if (t.isDirty) { if (!kept.includes(path.basename(u.fsPath))) kept.push(path.basename(u.fsPath)); continue; }
        tabs.push(t);
      }
      if (tabs.length) { try { await tg.close(tabs, true); closed = tabs.length; } catch (e) { this.log('關頁面：' + (e && e.message || e)); } }
    } else {
      // (no tab API: the designers this extension holds)
      const ds = all ? Array.from(this.designers || []) : (s && s.d ? [s.d] : []);
      for (const d of ds) {
        if (!d || !d.panel || !d.panel.dispose) continue;
        if (d.doc && d.doc.isDirty) { kept.push(path.basename(d.file)); continue; }
        d.panel.dispose();
        closed++;
      }
    }
    // (one editor area again -- an unsaved page that stayed just moves into it)
    if (closed) { try { await vscode.commands.executeCommand('workbench.action.joinAllGroups'); } catch (e) { /* one group already */ } }
    if (kept.length) vscode.window.setStatusBarMessage('$(info) 沒有關（還沒存檔）：' + kept.join('、'), 8000);
    this.log('上下版面：關了 ' + closed + ' 個頁面分頁（' + why + '）' + (kept.length ? '；還沒存檔沒關：' + kept.join('、') : ''));
    return { closed, kept };
  }
  /** The zoom a page had last (kept per page in the workspace state; 1 = none kept). */
  loadZoom(file) {
    const all = this.ctx.workspaceState ? this.ctx.workspaceState.get('htd.zoom', {}) : {};
    const z = all && +all[String(file).toLowerCase()];
    return z >= 0.125 && z <= 8 ? z : 1;
  }
  saveZoom(file, z) {
    if (!this.ctx.workspaceState) return;
    const all = Object.assign({}, this.ctx.workspaceState.get('htd.zoom', {}) || {});
    const k = String(file).toLowerCase();
    if (z === 1) delete all[k]; else all[k] = z;
    this.ctx.workspaceState.update('htd.zoom', all);
  }

  /** The artboard toolbar's two grid buttons (WPF): part 'show' = the grid drawn, 'snap' = snapping to it. */
  async cmdGridPart(part, on) {
    const g = this.gridState();
    const snap = part === 'snap';
    const nu = typeof on === 'boolean' ? on : !(snap ? g.on : g.show);
    if (this.ctx.workspaceState) await this.ctx.workspaceState.update(snap ? 'htd.gridOn' : 'htd.gridShow', nu);
    const g2 = this.gridState();
    for (const d of this.designers) d.post({ type: 'setGrid', on: g2.on, show: g2.show, size: g2.size });
    vscode.commands.executeCommand('setContext', 'ht9045Designer.gridOn', g2.on);
    this.updateStatus();
    vscode.window.setStatusBarMessage('$(symbol-numeric) ' + (snap ? '對齊格線 ' + (nu ? '開（' + g2.size + 'px；按住 Alt 暫時不吸）' : '關') : '顯示格線 ' + (nu ? '開' : '關')), 4000);
    return g2;
  }

  /** WPF "Toggle artboard background": dark around the form, or the page's own (every page; the workspace state). */
  artboardDark() { return !!(this.ctx.workspaceState && this.ctx.workspaceState.get('htd.artboardDark', false)); }

  setArtboard(dark, from) {
    if (this.ctx.workspaceState) this.ctx.workspaceState.update('htd.artboardDark', !!dark);
    for (const d of this.designers) if (d !== from) d.post({ type: 'setArtboard', dark: !!dark });
    vscode.window.setStatusBarMessage('$(color-mode) 畫面背景：' + (dark ? '暗色' : '頁面自己的') + '（只影響設計畫面）', 3000);
    return { dark: !!dark };
  }

  async cmdToggleGrid(on) {
    const g = this.gridState();
    const nu = typeof on === 'boolean' ? on : !g.on;
    // (the one command / menu item: the grid shown and snapped to together)
    if (this.ctx.workspaceState) { await this.ctx.workspaceState.update('htd.gridOn', nu); await this.ctx.workspaceState.update('htd.gridShow', nu); }
    for (const d of this.designers) d.post({ type: 'setGrid', on: nu, show: nu, size: g.size });
    vscode.commands.executeCommand('setContext', 'ht9045Designer.gridOn', nu);
    this.updateStatus();
    vscode.window.setStatusBarMessage('$(symbol-numeric) 對齊格線 ' + (nu ? '開（' + g.size + 'px；按住 Alt 暫時不吸）' : '關'), 4000);
    return { on: nu, size: g.size };
  }

  updateStatus() {
    const d = this.active;
    if (this.pageStatus) {
      this.pageStatus.text = '$(file-code) ' + (d ? path.basename(d.file) : '選擇頁面') + ' $(chevron-down)';
      this.pageStatus.tooltip = (d ? '目前：' + d.file + '\n' : '') + '按這裡：換一頁（選一個 HTML 頁面，用設計檢視開啟）' +
        (this.version ? '\nHTML 視覺設計工具 v' + this.version : '');
    }
    if (!d) { this.status.hide(); return; }
    const g = this.gridState();
    this.status.text = (d.mode === 'design' ? '$(pencil) 設計模式' : '$(play) 操作模式') +
      (d.zoom && d.zoom !== 1 ? '　' + Math.round(d.zoom * 100) + '%' : '') + (g.on ? '　格線 ' + g.size + 'px' : '') + (d.wireMarks ? '　接線標示' : '') + (d.dfmGhosts ? '　DFM 位置' : '') + (d.tabOrder ? '　Tab 順序' : '') + (d.showNames ? '　名稱' : '') +
      (d.live && d.live.machine ? '　' + d.live.machine : '');
    this.status.tooltip = (d.mode === 'design'
      ? '點＝選元件，雙擊＝跳到事件程式碼。按這裡切到操作模式。'
      : '點擊交給頁面自己的程式（網路仍全部封鎖）。按這裡切回設計模式。') +
      '\n' + ({ wheel: '滾輪', alt: 'Alt＋滾輪' }[this.wheelZoom()] || 'Ctrl＋滾輪') + '縮放（設定 ht9045Designer.zoomWheel）；「HTML設計: 設計檢視縮放回 100%」還原。' +
      (d.live ? '\n機種：' + (d.live.machine || '不指定') + (d.live.docs && Object.keys(d.live.docs).length ? '（機台設定檔唯讀：' + Object.keys(d.live.docs).join('、') + '）' : '') +
        '　右鍵「顯示」→「設計檢視的機種」可以改' : '');
    this.status.show();
  }

  /**
   * A newer version of this extension installed (in the extensions folder this copy came from, or `extDir`)?
   * Then: the status bar says so with a button, the 頁面 title too, and once a message with「現在更新」.
   * Returns the newer version or null. A copy run from the source tree (no extensions folder) finds nothing.
   */
  checkUpdate(extDir) {
    const ext = this.ctx.extension;
    const dir = extDir || (ext && ext.extensionPath ? path.dirname(ext.extensionPath) : null);
    const inst = this.version && dir && ext && ext.id ? update.installedVersion(dir, ext.id) : null;
    const newer = inst && update.cmpVer(inst, this.version) > 0 ? inst : null;
    if (newer === this.newerVersion) return newer;
    this.newerVersion = newer;
    vscode.commands.executeCommand('setContext', 'ht9045Designer.updateReady', !!newer);
    if (this.version) this.pageView.description = 'v' + this.version + (newer ? '（新版 v' + newer + ' 已裝好）' : '');
    // (0.139: the 頁面 list is in 方案總管 now -- the version shows there)
    if (this.version && this.solution) { this.solution.versionText = 'v' + this.version + (newer ? '（新版 v' + newer + ' 已裝好）' : ''); this.solution.setDescription(); }
    if (!newer) { this.updateItem.hide(); return null; }
    this.updateItem.text = '$(debug-restart) 設計工具新版 v' + newer + '：按這裡更新';
    this.updateItem.tooltip = 'HTML 視覺設計工具 v' + newer + ' 已經裝好了，這個視窗還在跑 v' + this.version + '。\n' +
      '按一下＝重新載入視窗，換成新版（開著的頁面、還沒存的檔案都會留著）。';
    this.updateItem.backgroundColor = new vscode.ThemeColor('statusBarItem.warningBackground');
    this.updateItem.show();
    this.log('有新版 v' + newer + ' 已裝好（這個視窗跑 v' + this.version + '）');
    vscode.window.showInformationMessage('HTML 視覺設計工具的新版 v' + newer + ' 已經裝好了（這個視窗還在跑 v' + this.version +
      '）。按「現在更新」換成新版，開著的頁面會自己再打開。', '現在更新')
      .then(c => { if (c === '現在更新') this.cmdReloadWindow(); }, () => {});
    return newer;
  }

  /**
   * CSV 表格 for `uri` (default: the active text editor's): the table opens, and the text tab of the same
   * file closes (it is the same document: nothing unsaved is lost, the table shows it).
   */
  async cmdOpenCsvTable(uri) {
    const u = uri instanceof vscode.Uri ? uri : (vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri);
    if (!u) return false;
    await vscode.commands.executeCommand('vscode.openWith', u, CSV_VIEW);
    try {
      const tg = vscode.window.tabGroups;
      if (tg && vscode.TabInputText) {
        const texts = tg.all.reduce((a, g) => a.concat(g.tabs), []).filter(t => t.input instanceof vscode.TabInputText && web.samePath(t.input.uri.fsPath, u.fsPath));
        if (texts.length) await tg.close(texts, true);
      }
    } catch (e) { /* the text tab stays: harmless */ }
    if (this.csvItem) this.csvItem.hide();
    this.csvShown = false;
    return true;
  }

  /** The active editor is a .csv shown as text: the status bar offers the table; asked once per file. */
  csvTextEditor(ed) {
    const doc = ed && ed.document;
    const isCsv = !!(doc && doc.uri && doc.uri.scheme === 'file' && /\.csv$/i.test(doc.uri.fsPath || ''));
    this.csvShown = isCsv;
    if (!isCsv) { this.csvItem.hide(); return false; }
    const name = path.basename(doc.uri.fsPath);
    this.csvItem.tooltip = name + ' 現在是文字畫面。按這裡用「CSV 表格」開（像 Excel 一樣編輯，只改你改的格子）';
    this.csvItem.show();
    const k = doc.uri.fsPath.toLowerCase();
    if (!this.csvAsked.has(k)) {
      this.csvAsked.add(k);
      // a text tab of it (one from before the update comes back as text, and a click in the explorer only brings
      // that tab forward -- EastSun 20260930: "我圖片上開CSV 怎還是一樣的畫面?"): switched to the table by itself.
      // Not a diff view, not one opened as text from the table on purpose (csvAsked), not with the setting off.
      let plainTab = true;
      try {
        const at = vscode.window.tabGroups && vscode.window.tabGroups.activeTabGroup.activeTab;
        plainTab = !at || !vscode.TabInputText || at.input instanceof vscode.TabInputText;
      } catch (e) { plainTab = true; }
      if (plainTab && this.cfg().get('csvAutoTable') !== false) {
        this.cmdOpenCsvTable(doc.uri).then(() => {
          vscode.window.setStatusBarMessage('$(table) ' + name + ' 改用表格開了（要看文字：表格右上「以文字開啟」；不要自動改：設定 ht9045Designer.csvAutoTable）', 8000);
        }, () => {});
        return true;
      }
      vscode.window.showInformationMessage(name + ' 現在是用文字開的。要用表格看嗎？', '用表格開', '不用')
        .then(c => { if (c === '用表格開') this.cmdOpenCsvTable(doc.uri); }, () => {});
    }
    return true;
  }

  /** 搜尋頁面 from a command: filter the 頁面 list by `q` ('' = every page) and put the words in the box. */
  filterPages(q) {
    const s = this.pages.setFilter(q);
    this.pageSearch.set(this.pages.query, s);
    // (0.139: 方案總管 shows the same search -- its files too; the pages part is already done)
    if (this.solution && String(q || '').trim() !== this.solution.q) this.solution.setFilter(q, { pagesDone: true });
    return s;
  }

  /**
   * 機種與機台設定 for pages of roots `r`: the machine (chosen, else the one F5 opens the HMI for in
   * launch.json), and READ-ONLY copies of the settings F5 gives wb_serve (Gerneral.ini, config.ini) in
   * wb_serve's /api/system shape. Choice (workspace): '' = like F5, '-' = none (the "no server" face), else an id.
   */
  machineState(r) {
    const pick = (this.ctx.workspaceState && this.ctx.workspaceState.get('htd.machine', '')) || '';
    const dirs = [];
    for (const x of [r && r.portRoot, r && r.webRoot && path.dirname(r.webRoot)].concat(this.wsFolders())) {
      if (x && !dirs.some(y => web.samePath(y, x))) dirs.push(x);
    }
    const hints = liveconfig.launchHints(dirs);
    const cfg = vscode.workspace.getConfiguration('ht9045Designer');
    const off = pick === '-';
    const machine = off ? null : pick || hints.machine || null;
    const iniOf = (key, auto) => (cfg.get(key) || auto || null);
    const generalIni = off ? null : iniOf('generalIni', hints.generalIni);
    const configIni = off ? null : iniOf('configIni', hints.configIni);
    const docs = {};
    this._iniCache = this._iniCache || new Map();
    for (const [name, f] of [['gerneral', generalIni], ['config', configIni]]) {
      if (!f) continue;
      let st = null;
      try { st = fs.statSync(f); } catch (e) { continue; }
      const k = f.toLowerCase();
      const c = this._iniCache.get(k);
      if (c && c.mtime === st.mtimeMs && c.size === st.size) { docs[name] = c.doc; continue; }
      const doc = liveconfig.iniToDoc(f);
      this._iniCache.set(k, { mtime: st.mtimeMs, size: st.size, doc });
      docs[name] = doc;
    }
    return { pick, machine, auto: !pick && !!hints.machine, hints, generalIni, configIni, docs: Object.keys(docs).length ? docs : null };
  }

  /** What a designer's page gets (lib/pagehtml.js `live`), null = nothing. */
  liveFor(d) {
    const s = this.machineState(d && d.r);
    return s.machine || s.docs ? { machine: s.machine, docs: s.docs || {} } : null;
  }

  /** 機種: which machine the pages are shown for; every designer draws its page again. */
  async cmdPickMachine(arg) {
    const d = this.active;
    const r = d ? d.r : roots.resolveRoots(null, this.wsFolders(), this.over());
    const s = this.machineState(r);
    let pick = arg;
    if (typeof pick !== 'string') {
      const prof = liveconfig.machineProfiles(r.webRoot);
      const ids = prof.ids.slice();
      if (s.hints.machine && !ids.includes(s.hints.machine)) ids.unshift(s.hints.machine);
      const items = [{ label: '$(debug-start) 跟 F5 一樣' + (s.hints.machine ? '（' + s.hints.machine + '）' : '（launch.json 沒有指定機種）'), pick: '',
        description: s.pick === '' ? '● 目前' : '', detail: 'F5 開機台畫面用的機種（launch.json 的 W906_HMI_URL machine=），設定檔也用 F5 給 wb_serve 的那幾個（唯讀）' }]
        .concat(ids.map(id => ({ label: '$(server-environment) ' + id, pick: id, description: (s.pick === id ? '● 目前　' : '') + (prof.label[id] || ''),
          detail: '頁面照 ' + id + ' 顯示（機台設定檔唯讀）' })))
        .concat([{ label: '$(circle-slash) 不指定', pick: '-', description: s.pick === '-' ? '● 目前' : '',
          detail: '跟沒有伺服器時一樣：機種專用的東西不顯示，設定檔也不讀' }]);
      const it = await vscode.window.showQuickPick(items, { placeHolder: '設計檢視的機種（機種專用的頁籤、元件照它顯示）' });
      if (!it) return null;
      pick = it.pick;
    }
    if (this.ctx.workspaceState) await this.ctx.workspaceState.update('htd.machine', pick === '' ? undefined : pick);
    const now = this.machineState(r);
    this.log('設計檢視的機種：' + (now.machine || '不指定') + (now.auto ? '（跟 F5）' : '') +
      (now.docs ? '；設定檔（唯讀）' + [now.generalIni, now.configIni].filter(Boolean).join('、') : ''));
    for (const x of this.designers) x.render();
    this.updateStatus();
    return { machine: now.machine, auto: now.auto, docs: now.docs ? Object.keys(now.docs) : [] };
  }

  /** Reload the window (the only way a window takes a newly installed version). Unsaved files: VS Code keeps
      them over a reload (hot exit) -- unless files.hotExit is off; then ask to save first. */
  async cmdReloadWindow() {
    const dirty = vscode.workspace.textDocuments.filter(t => t.isDirty);
    const hot = vscode.workspace.getConfiguration('files').get('hotExit');
    if (dirty.length && hot === 'off') {
      const c = await vscode.window.showWarningMessage('有 ' + dirty.length + ' 個檔案還沒存（例如 ' + path.basename(dirty[0].fileName) +
        '）。重新載入前要先存檔嗎？', { modal: true }, '全部存檔後重新載入', '不存，直接重新載入');
      if (!c) return false;
      if (c === '全部存檔後重新載入' && !(await vscode.workspace.saveAll(false))) return false;
    }
    await vscode.commands.executeCommand('workbench.action.reloadWindow');
    return true;
  }

  showProps(d, data) {
    this.propsDesigner = d;
    this.props.show(data || null);
  }

  /** Page-level facts: which DFM, which form class, which scripts. */
  async analyzePage(d) {
    const html = d.doc.getText();
    const r = d.r;
    const title = pageinfo.parseTitle(html);
    const store = r.irRoot ? this.irStore(r.irRoot) : null;
    let ir = null, dfm = null;
    if (store && title && title.dfm) {
      const f = store.find(title.dfm, title.cls);
      if (f) { ir = store.load(f); dfm = { how: 'title' }; }
    }
    if (store && !ir) {
      await new Promise(res => setImmediate(res));
      const g = store.guess(pageinfo.collectIds(html));
      if (g) { ir = store.load(g.file); dfm = { how: 'guess', hits: g.hits, score: g.score }; }
    }
    const classes = [];
    if (ir && ir.formClass) classes.push(ir.formClass);
    if (title && title.cls && !classes.includes(title.cls)) classes.push(title.cls);
    d.formLabel = ir ? (ir.formName || '表單') + ' : ' + (ir.formClass || '') : '表單（.form）';
    d.ir = ir;
    if (this.active === d) this.tree.refresh();
    return { title, ir, dfm, classes, scripts: pageinfo.listScripts(html, d.pageDir), iframe: hasIframe(html) };
  }

  // --- selection -----------------------------------------------------------
  onSelect(d, info, origin) {
    if (this.active !== d) {
      // a page in the background (it reloaded and restored its selection) must not
      // take the tree and the panel away from the page the user is looking at
      if (!d.panel.active) { d.pendingSel = { info, origin }; return; }
      this.setActive(d);
    }
    d.pendingSel = null;
    const seq = ++this.selSeq;
    if (!info) { d.sel = null; this.showProps(d, null); return; }
    if (origin !== 'tree') this.tree.reveal(info.key);
    if (origin === 'click' || origin === 'tree' || origin === 'find' || origin === 'key' || origin === 'edit') this.syncSource(d, info.id);
    const sel = { info, data: null, promise: null };
    d.sel = sel;
    sel.promise = this.resolve(d, info, partial => {
      if (seq === this.selSeq) { sel.data = partial; this.showProps(d, partial); }
    }).then(async data => {
      // several selected: which fields differ among them (WPF shows those empty)
      if (data && data.edit && Array.isArray(info.multi) && info.multi.length && seq === this.selSeq) {
        const rep = await d.request({ type: 'lookAll', ids: [info.id].concat(info.multi) }, 3000);
        if (rep) data.edit.mixed = mixed.mixedFields(rep.items);
      }
      if (seq === this.selSeq) { sel.data = data; this.showProps(d, data); }
      return data;
    }, e => {
      this.log('解析元件失敗：' + (e && e.stack || e));
      return null;
    });
  }

  async onDesignerDblClick(d, viewOnly) {
    const sel = d.sel;
    if (!sel) return;
    const data = await sel.promise;
    if (!data) return;
    // WPF / BCB6: its default event -- the code when set, a new handler (wired to the page) when empty
    const g = data.eventGrid;
    const def = g && g.rows && g.rows.length ? vclevents.defaultEventOf(g.vcl, g.isForm, g.rows.map(r => r.name)) : null;
    const defRow = def ? g.rows.find(r => r.name === def) : null;
    if (defRow && (defRow.handler || !viewOnly)) return this.onEventGrid(data, def, d);
    if (viewOnly && g && g.formCls) {
      // View Code with no default event: the form class's code (WPF: the code-behind)
      const pf = await this.portClassFiles(d, g.formCls);
      if (pf) return this.openTarget({ kind: 'port', file: pf.cppFile, line: 1, col: 1 }, d);
    }
    const i = fmt.defaultEvent(data.events, data.comp.isForm);
    if (i >= 0) return this.openEvent(data, i, d);
    return this.revealSource(d, data.comp.htmlId, true, data.comp.html != null ? data.targets[data.comp.html] : null);
  }

  webSources(d, pi) {
    const list = [new web.Source(d.file, d.doc.getText(), true)];
    for (const s of pi.scripts) {
      const t = web.readText(s);
      if (t != null) list.push(new web.Source(s, t, false));
    }
    return list;
  }

  /** The BCB6 .dfm a page's IR came from (golden tree), or null. */
  dfmFileOf(d, ir) {
    if (!ir || !ir.sourceDfm || !d.r.goldenRoot) return null;
    const f = path.join(d.r.goldenRoot, ir.sourceDfm.replace(/[\\/]/g, path.sep));
    return fs.existsSync(f) ? f : null;
  }

  /** A golden (Big5) file as lines, cached by mtime. */
  goldenLines(file) {
    let st;
    try { st = fs.statSync(file); } catch (e) { return null; }
    this._goldenLines = this._goldenLines || new Map();
    const hit = this._goldenLines.get(file);
    if (hit && hit.mtime === st.mtimeMs) return hit.lines;
    let lines;
    try { lines = decodeBig5(fs.readFileSync(file)).split('\n').map(l => l.replace(/\r$/, '')); } catch (e) { return null; }
    this._goldenLines.set(file, { mtime: st.mtimeMs, lines });
    if (this._goldenLines.size > 20) this._goldenLines.delete(this._goldenLines.keys().next().value);
    return lines;
  }

  lineSnippet(file, line) {
    const t = web.readText(file);
    if (t == null) return '';
    const lines = t.split('\n');
    const s = (lines[line - 1] || '').replace(/\r$/, '').trim();
    return s.length > 170 ? s.slice(0, 170) + '…' : s;
  }

  cppTarget(kind, h, forCmd) {
    let note;
    if (forCmd) note = h.dispatch ? '命令分派' : h.comment ? '註解提到' : '出現此字串';
    else if (kind === 'golden') note = h.def ? '定義（BCB6 原版）' : h.comment ? '註解提到' : '引用';
    else note = h.def ? (h.dead ? '定義，但在 #if 0 裡（不會編譯）' : '定義') : h.comment ? '註解提到（實作可能搬到這裡）' : '引用';
    if (h.test) note += '（測試）';
    return {
      kind, file: h.file, line: h.line, col: h.col, snippet: h.snippet, note,
      warn: !!h.dead, weak: forCmd ? !h.dispatch : !(h.def && !h.dead),
    };
  }

  /**
   * Everything the 屬性與事件 panel shows for one component. Calls onPartial first
   * with the web/DFM part (instant), then resolves with the C++ part added.
   */
  async resolve(d, info, onPartial) {
    const pi = await d.info;
    const targets = [];
    const T = t => { targets.push(t); return targets.length - 1; };
    const isForm = info.id === '@form';
    const ir = pi.ir;
    const node = ir ? (isForm ? ir.root : (info.id ? ir.byName.get(info.id) : null)) : null;
    const html = d.doc.getText();
    const v = fmt.parseVclTitle(info.title || web.titleOf(html, info.id));

    const comp = {
      key: info.key, htmlId: info.id || '', isForm,
      name: isForm ? ((ir && ir.formName) || '表單') : (info.id || '<' + info.tag + '>'),
      cls: node ? node.class : (v ? v.cls : ''),
      note: v && v.note ? v.note : '',
      tag: info.tag, path: node ? node.path : '', chain: info.chain || [], cssPath: info.cssPath || '',
      inIr: !!node, html: null,
    };
    if (info.id) {
      const at = web.findIdAttr(html, info.id);
      if (at) {
        const p = d.doc.positionAt(at.start);
        comp.html = T({ kind: 'html', file: d.file, line: p.line + 1, col: p.character + 1, snippet: '', note: 'HTML 原始碼', range: [at.start, at.end] });
      }
    }
    // the control's own lines in the BCB6 .dfm (Big5): the IR knows "object Name: TClass"
    const dfmFile = this.dfmFileOf(d, ir);
    const dfmLines = dfmFile ? this.goldenLines(dfmFile) : null;
    comp.dfm = null;
    if (node && node.line && dfmLines) {
      comp.dfm = T({ kind: 'dfm', file: dfmFile, line: node.line, col: 1, snippet: (dfmLines[node.line - 1] || '').trim(), note: 'DFM 定義' });
    }
    const propLines = node && node.line && dfmLines ? pageinfo.dfmPropLines(dfmLines, node.line) : {};

    // web: listeners recorded at runtime, then where their code is
    const sources = this.webSources(d, pi);
    const idRe = info.id && !isForm ? web.idMentionRe(info.id) : null;
    const mentions = idRe ? web.findAll(sources, idRe, 12) : [];
    const mentionLines = mentions.map(m => ({ file: m.file, line: m.line }));
    // the generated pages' layout classes say nothing about which handler is meant
    const LAYOUT_CLS = /^(form|pnl|pnlCap|gbx|cli|lb|ed|ckb|rgi|btn3d|lled|lledCap|tab|act|dim|sgd|imgph|trk|elab|down)$/;
    const classes = String(info.cls || '').split(/\s+/).filter(c => c.length >= 3 && !LAYOUT_CLS.test(c));
    const listeners = [];
    const near = loc => loc && mentionLines.some(m => web.samePath(m.file, loc.file) && loc.line >= m.line - 3 && loc.line <= m.line + 25);
    const addL = (l, on, onLabel) => {
      const f0 = l.frames && l.frames[0];
      const bindFile = f0 ? web.mapFrameUrl(f0.url) : null;
      let bind = null, bindT = null;
      if (bindFile && fs.existsSync(bindFile)) {
        bind = { kind: 'web', file: bindFile, line: f0.line, col: f0.col, snippet: this.lineSnippet(bindFile, f0.line), note: '綁定位置' };
        if (!sources.some(s => web.samePath(s.file, bindFile))) {
          const tx = web.readText(bindFile);
          if (tx != null) sources.push(new web.Source(bindFile, tx, /\.html?$/i.test(bindFile)));
        }
      }
      const h = web.locateFn(sources, l.fnText, bindFile);
      const handler = h ? Object.assign({ kind: 'web', note: h.approx ? '處理函式（依第一行比對）' : '處理函式' }, h) : null;
      const text = l.fnText || '';
      const relevant = on === 'self' ||
        (info.id && !isForm && text.includes(info.id)) ||
        classes.some(c => web.mentionsClass(text, c)) ||
        near(bind) || near(handler);
      if (bind) bindT = T(bind);
      const hT = handler ? T(handler) : null;
      // follow the handler into what it calls, to the command it finally sends
      const traced = relevant ? web.traceCmds(sources, text, 3, handler ? handler.file : bindFile) : [];
      listeners.push({
        type: l.type, on, onLabel: onLabel || '', via: l.via, cap: !!l.cap, fnName: l.fnName || '',
        handler: hT, bind: bindT, cmds: traced.map(t => t.cmd), traced, relevant: !!relevant,
      });
    };
    (info.listeners || []).forEach(l => addL(l, 'self'));
    (info.inherited || []).forEach(a => a.list.forEach(l => addL(l, 'up', a.label)));
    (info.inline || []).forEach(pair => {
      const traced = web.traceCmds(sources, pair[1], 3, d.file);
      listeners.push({
        type: String(pair[0]).replace(/^on/i, ''), on: 'attr', onLabel: '', via: 'attr', cap: false, fnName: '',
        handler: comp.html, bind: null, code: pair[1], cmds: traced.map(t => t.cmd), traced, relevant: true,
      });
    });
    const mentionT = mentions.map(m => T(Object.assign({ kind: 'web', note: '提到此 id' }, m)));

    // DFM events -> web targets now, C++ targets below
    const events = [];
    const evs = node && node.events ? Object.entries(node.events) : [];
    for (const [name, handlerName] of evs) {
      const types = fmt.domTypesOf(name);
      const tl = [];
      const push = x => { if (x != null && !tl.includes(x)) tl.push(x); };
      for (const l of listeners) if (l.on === 'self' && types.includes(l.type)) push(l.handler != null ? l.handler : l.bind);
      for (const l of listeners) if (l.on !== 'self' && l.relevant && types.includes(l.type)) push(l.handler != null ? l.handler : l.bind);
      if (!tl.length) {
        web.findAll(sources, web.identRe(handlerName), 3).forEach(h => push(T(Object.assign({ kind: 'web', note: '同名函式' }, h))));
      }
      events.push({ name, handler: handlerName, targets: tl, cppPending: true });
    }

    // commands the relevant handlers send (traced through their calls), and via what
    const cmdVia = new Map();
    for (const l of listeners) {
      if (!l.relevant) continue;
      for (const t of l.traced) {
        if (!cmdVia.has(t.cmd)) cmdVia.set(t.cmd, [l.type + ' 處理函式'].concat(t.via.map(n => n + '()')));
      }
    }
    for (const m of mentions) {
      for (const c of web.extractCmds(m.snippet)) if (!cmdVia.has(c)) cmdVia.set(c, ['提到此 id 的那一行']);
    }

    // WPF-style editing: what can be changed, and whether it can be written back
    const edTag = info.id ? htmledit.startTagOf(html, info.id) : null;
    const edWrap = edTag && info.layout && info.layout.target === 'parent' ? htmledit.wrapperTagOf(html, edTag.start) : null;
    // locked in the designer (the lock): position / size cannot be changed, text and look can
    const locked = this.lockedName(d, isForm ? '@form' : info.id) || null;
    const edit = {
      layout: info.layout || null, caption: info.caption || null, look: info.look || null,
      inSource: !!edTag,
      locked,
      layoutInSource: !locked && !!(edTag && info.layout && (info.layout.target === 'self' || edWrap)),
      dirty: !!d.doc.isDirty,
      multi: Array.isArray(info.multi) && info.multi.length ? info.multi : null,
      // (the form's DFM Width/Height are the whole window incl. caption and borders,
      // not the page's client area: nothing comparable, so no DFM values for it)
      dfm: node && !isForm ? fmt.dfmEditValues(node) : null,
    };
    // Name / Alias (the IO it stands for, kept in the title) and the page's own JS handlers (網頁事件): editable
    const titleNow = edTag ? ((attrsOf(edTag.text).find(a => a[0] === 'title') || [])[1] || '') : '';
    edit.aliasOn = !isForm && !!edTag && /Alias=/.test(html);
    // (from the page as it is now -- the probe changed the title before it asked again -- not from the source text,
    // which the edit may not have reached yet: the field showed the old Alias and it took a second Enter)
    edit.alias = aliasedit.aliasOf(info.title ? info.title : titleNow);
    edit.ioAliases = edit.aliasOn ? this.ioAliasList(d) : null;
    edit.jsEvents = !isForm && edTag ? { types: jsevents.TYPES, handlers: jsevents.handlersOf(html, info.id) } : null;

    const data = {
      comp, events, listeners, mentions: mentionT, cmds: [], uses: null, fields: null, tags: null, edit,
      props: node ? fmt.formatProps(node.properties).map(p => p.concat([propLines[p[0]] || 0])) : [],
      dfmFile: dfmFile || null,
      html: {
        geom: info.geom || null, dfmGeom: node ? node.geometry : null, attrs: info.attrs || [],
        style: info.style || [], computed: info.computed || [], text: info.text || '', visible: info.visible !== false,
        // what the SOURCE's start tag says (the grid edits these, like WPF edits the XAML):
        // the declarations as written, not the browser's longhands; the attributes as written
        srcStyle: edTag ? htmledit.styleOf(edTag.text).decls.map(x => [x.name, x.value]) : null,
        srcAttrs: edTag ? attrsOf(edTag.text).filter(a => a[0] !== 'style').map(a => a.concat([ATTR_EDIT.test(a[0]) ? 1 : 0])) : null,
      },
      page: this.pageSummary(d, pi),
      targets, cppPending: true,
    };
    onPartial(data);

    const port = this.sourceTree(d.r.portRoot, 'port');
    const gold = this.sourceTree(d.r.goldenRoot, 'golden');

    // web command -> C++: the dispatch branch in the server, and the function it calls
    const cmdRes = new Map();
    for (const [c, via] of Array.from(cmdVia).slice(0, 8)) {
      const send = web.findCmdSend(sources, c).map(h => T(Object.assign({ kind: 'web', note: '送出命令' }, h)));
      const r = { cmd: c, via: via.join(' → '), send, dispatch: [], handlers: [], handlerNames: [], handlerAt: {}, other: [] };
      if (port) {
        const hits = (await port.findString(c)).filter(h => !h.test);
        const disp = hits.filter(h => h.dispatch && !h.comment && !h.dead).slice(0, 3);
        const called = [];
        for (const h of disp) called.push(await port.calledFuncsAt(h.file, h.off));
        r.dispatch = disp.map((h, k) => {
          const t = this.cppTarget('port', h, true);
          if (!called[k].length && /\{\s*$/.test(h.snippet)) t.note = '命令分派（處理程式就寫在這個區塊裡）';
          return T(t);
        });
        const seenFn = new Set();
        for (let k = 0; k < Math.min(2, disp.length); k++) {
          for (const fnName of called[k]) {
            if (seenFn.has(fnName)) continue;
            seenFn.add(fnName);
            for (const dd of (await port.findFuncDefs(fnName)).slice(0, 2)) {
              const t = this.cppTarget('port', dd);
              t.note = '處理函式 ' + fnName + (dd.dead ? '（在 #if 0 裡，不會編譯）' : '');
              const ti = T(t);
              r.handlers.push(ti);
              if (!r.handlerNames.includes(fnName)) r.handlerNames.push(fnName);
              if (r.handlerAt[fnName] == null && !dd.dead) r.handlerAt[fnName] = ti;
            }
          }
        }
        r.other = hits.filter(h => !disp.includes(h)).slice(0, 3).map(h => T(this.cppTarget('port', h, true)));
      }
      cmdRes.set(c, r);
      data.cmds.push(r);
    }

    for (const ev of events) {
      // what really runs when the web page's button is used: its command's C++ path
      // (per command: its C++ handler functions, else its dispatch line; at most 4 in all,
      // shallowest command first -- the full list is in the 送到 C++ 的命令 section)
      const types = fmt.domTypesOf(ev.name);
      let extra = 0;
      for (const l of listeners) {
        if (!(l.on === 'self' || l.relevant) || !types.includes(l.type)) continue;
        for (const c of l.cmds) {
          const r = cmdRes.get(c);
          if (!r) continue;
          const pick = r.handlers.length ? r.handlers.slice(0, 2) : r.dispatch.slice(0, 1);
          for (const i of pick) {
            if (extra >= 4 || ev.targets.includes(i)) continue;
            data.targets[i] = Object.assign({}, data.targets[i], { note: '網頁命令 ' + c + ' → ' + data.targets[i].note });
            ev.targets.push(i);
            extra++;
          }
        }
      }
      if (port && pi.classes.length) {
        const hits = await port.lookup(pi.classes, ev.handler);
        const defs = hits.filter(h => h.def && !h.test).slice(0, 6);
        const ments = hits.filter(h => h.comment && !h.test).slice(0, 5);
        const other = hits.filter(h => !h.def && !h.comment && !h.test).slice(0, 2);
        const pick = defs.concat(ments, other);
        (pick.length ? pick : hits.slice(0, 3)).forEach(h => ev.targets.push(T(this.cppTarget('port', h))));
      }
      if (gold && pi.classes.length) {
        const hits = await gold.lookup(pi.classes, ev.handler);
        const defs = hits.filter(h => h.def).slice(0, 3);
        (defs.length ? defs : hits.slice(0, 1)).forEach(h => ev.targets.push(T(this.cppTarget('golden', h))));
      }
      ev.cppPending = false;
    }

    // 事件表 (the Events tab of BCB6's Object Inspector / WPF's Properties window): every event of the class,
    // the handler of the ones that are set, and where each is wired -- the page sends it (its *_ev.js CTLS
    // list), the server has it (the generated form.event table), the C++ port has the function, BCB6 has it
    {
      const vcl = isForm ? 'TForm' : comp.cls || '';
      const formCls = pi.classes && pi.classes.length ? pi.classes[0] : '';
      const name = isForm ? '' : info.id || '';
      const dfmEv = node && node.events ? node.events : {};
      const table = port && d.r.portRoot ? this.serverEvents(d.r.portRoot) : [];
      // the C++ events the designer wired on this page (htdCpp lines), the server's htd.event branch, the generated table
      const pageText = d.doc.getText();
      const htdLines = name ? jsevents.cppLines(pageText).filter(x => x.id === name) : [];
      let htdDisp = null, htdRows = [];
      if (htdLines.length && port) {
        const hh = (await port.findString(cppbridge.CMD)).filter(h => h.dispatch && !h.dead && !h.test && !h.comment);
        if (hh.length) htdDisp = T(Object.assign(this.cppTarget('port', hh[0], true), { note: '伺服器分派 ' + cppbridge.CMD + '（設計工具的事件）' }));
        // (written a moment ago: the file on disk has it before the source index does)
        if (!htdDisp) {
          const sv = await this.htdServer(port);
          const at = sv ? /\.cmd\s*==\s*"htd\.event"/.exec(sv.text) : null;
          if (at) htdDisp = T({ kind: 'port', file: sv.file, line: sv.text.slice(0, at.index).split('\n').length, col: 1, snippet: cppbridge.CMD + ' → ' + (cppbridge.serverHook(sv.text, '').fn || ''),
            note: '伺服器分派 ' + cppbridge.CMD + '（設計工具的事件）', weak: false });
        }
        const gd = d.r.portRoot ? this.htdGen(d.r.portRoot) : { text: null };
        htdRows = gd.text ? cppbridge.rowsOf(gd.text) : [];
      }
      const rows = [];
      for (const evName of vclevents.eventsOf(vcl, Object.keys(dfmEv))) {
        const dfmH = dfmEv[evName] || '';
        const conv = vclevents.handlerName(name, evName, isForm);
        const fe = vclevents.formEventOf(evName);
        const row = { name: evName, handler: dfmH, dfm: !!dfmH, idx: dfmH ? events.findIndex(e => e.name === evName) : -1, fe, conv,
          port: null, server: null, web: null };
        const srv = fe && name ? table.find(r => pi.classes.includes(r.cls) && r.ctl === name && r.ev === fe) : null;
        if (srv) {
          row.server = T({ kind: 'port', file: srv.file, line: srv.line, col: 1, snippet: srv.snippet, note: '伺服器的 form.event 事件表：' + srv.golden, weak: false });
          if (!row.handler) row.handler = srv.meth;
        }
        if (port && pi.classes.length) {
          const st = await port.defState(pi.classes, dfmH || conv);
          if (st.state === 'live' || st.state === 'dead') {
            row.port = T(this.cppTarget('port', st.hit));
            row.portState = st.state;
            if (!row.handler) row.handler = dfmH || conv;
          }
        }
        if (fe && name) {
          const re = new RegExp('\\[\\s*[\'"]' + name.replace(/[^\w]/g, '\\$&') + '[\'"]\\s*,\\s*[\'"]' + fe + '[\'"]\\s*\\]', 'g');
          const h = web.findAll(sources, re, 1)[0];
          if (h) row.web = T(Object.assign({ kind: 'web', note: '網頁會送 form.event（這一頁的事件清單）' }, h));
        }
        // wired through the page's JS: its listener of this DOM event sends a WS command the server dispatches to
        // C++ (EastSun 20260930 on BtnPanelLane3: "應該有click吧? 他是連到C++的" -- io.btnPanelClick ->
        // W906_DispatchIoClick); a component the .dfm does not have (added for the 9050) is wired this way too
        // (a .dfm one keeps its handler name -- sbCloseProgram.OnClick = sbCloseProgramClick -- and gets the marks and
        // the port's function, W906_Main_CloseProgramOp through act.main.closeProgram)
        // a C++ event the designer wired (its htdCpp line): THAT handler -- not the htd.event entry point the page's
        // listener sends to
        const dl = htdLines.find(x => x.event === evName);
        if (dl) {
          row.cmd = cppbridge.CMD;
          row.via = 'htd';
          if (!row.handler) row.handler = dl.handler;
          row.web = T({ kind: 'web', file: d.file, line: pageText.slice(0, dl.s).split('\n').length, col: 1, snippet: pageText.slice(dl.s, dl.e).trim(),
            note: '這一頁送出（設計工具的 htdCpp）', weak: false });
          row.server = htdDisp && htdRows.some(r => r.form === formCls && r.handler === dl.handler) ? htdDisp : null;
          if (port && pi.classes.length) {
            const st = dl.handler === (dfmH || conv) && row.portState ? { state: row.portState, hit: null } : await port.defState(pi.classes, dl.handler);
            if (st.state === 'live') {
              if (st.hit) row.port = T(this.cppTarget('port', st.hit));
              row.portState = 'live';
              row.main = row.port;
              row.mainName = dl.handler;
            } else {
              // added and not saved yet: its body in the open .cpp
              const od = this.dirtyHit(d.r.portRoot, new RegExp('\\b' + formCls + '\\s*::\\s*' + dl.handler + '\\s*\\('), /\.cpp$/i);
              if (od) {
                row.port = T({ kind: 'port', file: od.file, line: od.line, col: 1, snippet: od.snippet, note: '定義（還沒存檔）', weak: false });
                row.portState = 'new';
                row.main = row.port;
                row.mainName = dl.handler;
              }
            }
          }
        }
        if (!isForm && !dl) {
          const types = fmt.domTypesOf(evName);
          let via = null;
          // (the operator-token handshake the sender does first -- control.takeover / control.release -- is not it;
          // nor htd.event, the designer's own entry point: its handler is the row's htdCpp line)
          for (const pass of [0, 1]) {
            for (const l of listeners) {
              if (via || !(l.on === 'self' || l.relevant) || !types.includes(l.type)) continue;
              for (const c of l.cmds) {
                if (c === cppbridge.CMD) continue;
                if (pass === 0 && /^control\./.test(c)) continue;
                const r = cmdRes.get(c);
                if (r && (r.dispatch.length || r.handlers.length)) { via = { l, r }; break; }
              }
            }
          }
          if (via) {
            row.cmd = via.r.cmd;
            // THE function that runs (EastSun 20260930: "我需要你只對應到W906_Main_CloseProgramOp，其他的事件也都是一樣"):
            // the handler the dispatch calls, not a guard / helper around it (W906_DispatchIoClick over
            // W906_IoPageNoGuards; W906_Main_CloseProgramOp)
            const hn = via.r.handlerNames.filter(n => via.r.handlerAt[n] != null);
            const helper = n => /Guard|Lock|Log|Json|Printf|Ack/i.test(n);
            const best = hn.find(n => /(Op|Click|Change|Dispatch\w*|Event|Handle\w*|Run\w*)$/.test(n) && !helper(n)) || hn.find(n => !helper(n)) || hn[0];
            if (best) { row.main = via.r.handlerAt[best]; row.mainName = best; }
            else if (via.r.dispatch.length) { row.main = via.r.dispatch[0]; row.mainName = via.r.cmd + '（wb_serve 的分派區塊）'; }
            if (!row.handler) row.handler = best || via.r.cmd;
            if (row.web == null) row.web = via.l.handler != null ? via.l.handler : via.l.bind;
            if (row.server == null && via.r.dispatch.length) row.server = via.r.dispatch[0];
            if (row.port == null && row.main != null) row.port = row.main;
            row.via = 'cmd';
          }
        }
        // no command to follow: the function the server's form.event table runs (B_cbSelectHPFromDBChange, the golden
        // handler's translation), else the port's own definition of it (live), else that table row
        if (row.main == null && srv && srv.fnLine) {
          row.main = T({ kind: 'port', file: srv.file, line: srv.fnLine, col: 1, snippet: srv.fnSnippet, note: '伺服器的 form.event 跑的函式（' + srv.golden + ' 的翻譯）', weak: false });
          row.mainName = srv.fn;
        }
        if (row.main == null && row.port != null && row.portState === 'live') { row.main = row.port; row.mainName = dfmH || conv; }
        if (row.main == null && row.server != null) { row.main = row.server; row.mainName = (srv ? srv.meth : '') + '（form.event 事件表）'; }
        rows.push(row);
      }
      // the value box's list (Windows Forms: "all methods that have a compatible method signature"): the form class's
      // public methods whose parameter types are the event's, with a body that is compiled (a GATE's has none)
      if (port && formCls && !isForm && name) {
        const pf = await this.portClassFiles(d, formCls);
        let hT = '';
        try { hT = pf ? fs.readFileSync(pf.hFile, 'utf8') : ''; } catch (e) { hT = ''; }
        const ms = hT ? cppbridge.methodsOf(hT, formCls).filter(m => m.access === 'public') : [];
        const live = new Map();
        for (const rw of rows) {
          const key = cppbridge.typesKey(vclevents.signatureOf(rw.name));
          rw.pick = [];
          for (const m of ms) {
            if (cppbridge.typesKey(m.params) !== key) continue;
            if (!live.has(m.name)) live.set(m.name, (await port.defState(pi.classes, m.name)).state === 'live');
            if (live.get(m.name) && !rw.pick.includes(m.name)) rw.pick.push(m.name);
          }
        }
      }
      // AI(W906-HTDESIGNER) 20261001: several selected (BCB6's Object Inspector, Windows Forms' Events tab): only the
      // events they ALL have; one whose handlers differ shows empty; a name typed there = that handler for all of them
      const others = !isForm && Array.isArray(info.multi) ? info.multi.filter(x => x && x !== '@form' && x !== name) : [];
      let multi = null;
      if (others.length) {
        const lines = jsevents.cppLines(pageText);
        const os = others.map(id => {
          const n = ir ? ir.byName.get(id) : null;
          const vv = n ? null : fmt.parseVclTitle(web.titleOf(pageText, id));
          return { id, vcl: n ? n.class : (vv ? vv.cls : ''), dfm: n && n.events ? n.events : {} };
        });
        const common = [];
        for (const rw of rows) {
          if (!os.every(o => vclevents.eventsOf(o.vcl, Object.keys(o.dfm)).includes(rw.name))) continue;
          const hs = [rw.handler || ''];
          let ro = !!rw.dfm || !!(rw.cmd && rw.via !== 'htd');
          const fe = rw.fe;
          for (const o of os) {
            const dh = o.dfm[rw.name] || '';
            if (dh) ro = true;
            const li = lines.find(x => x.id === o.id && x.event === rw.name);
            let h = dh || (li ? li.handler : '');
            if (!h && fe) { const sr = table.find(r => pi.classes.includes(r.cls) && r.ctl === o.id && r.ev === fe); if (sr) h = sr.meth; }
            if (!h && port && pi.classes.length) {
              const cv = vclevents.handlerName(o.id, rw.name, false);
              const st = await port.defState(pi.classes, cv);
              if (st.state === 'live' || st.state === 'dead') h = cv;
            }
            hs.push(h);
          }
          rw.mixed = hs.some(h => h !== hs[0]);
          rw.multiRo = ro;
          common.push(rw);
        }
        rows.splice(0, rows.length, ...common);
        multi = others;
      }
      data.eventGrid = { vcl, formCls, name, isForm, rows, multi };
    }

    // the setting/recipe field this control edits (wire field map XST1: ['Hotplate Form',
    // 'X Start']) and the C++ / BCB6 lines that read or write it
    data.fields = [];
    const widx = info.id && !isForm && d.r.webRoot ? this.webCmdIndex(d.r.webRoot) : null;
    if (widx) {
      const rwNote = (h, withSec) => (/\b\w*(Write|Save|Put|Set)\w*\s*\(/.test(h.snippet) ? '寫入' : /\b\w*(Read|Load|Get)\w*\s*\(/.test(h.snippet) ? '讀取' : '出現') +
        (withSec ? '' : '（只比對到 key，section 可能是變數）') + (h.dead ? '（在 #if 0 裡）' : '');
      const cppFor = async (tree, kind, r) => {
        if (!tree) return [];
        const hits = (await tree.findString(r.key)).filter(h => !h.test && !h.comment);
        const both = hits.filter(h => h.snippet.includes('"' + r.section + '"'));
        return (both.length ? both : hits).slice(0, 5).map(h => T(Object.assign(this.cppTarget(kind, h, true), {
          note: rwNote(h, both.includes(h)), weak: !both.includes(h),
        })));
      };
      // the same field is often mapped twice (hand-written wire + generated wire): one entry
      const groups = new Map();
      for (const r of widx.fieldsOf(d.file, info.id)) {
        const k = r.section + '\u0000' + r.key;
        if (!groups.has(k)) groups.set(k, []);
        groups.get(k).push(r);
      }
      for (const rows of Array.from(groups.values()).slice(0, 4)) {
        const r = rows[0];
        data.fields.push({
          section: r.section, key: r.key,
          note: rows.map(x => x.note).filter(Boolean).join('；') || (rows.find(x => x.doc) || {}).doc || '',
          web: rows.map(x => T({ kind: 'web', file: x.file, line: x.line, col: x.col, snippet: x.snippet, note: '欄位對照' + (x.doc ? '（文件 ' + x.doc + '）' : '') })),
          port: await cppFor(port, 'port', r),
          golden: await cppFor(gold, 'golden', r),
        });
      }
    }

    // live data the control shows: wire tag map 'machine.state': ['palMainStatus', 'text'],
    // and the C++ lines that publish (or use) that tag
    data.tags = [];
    if (widx) {
      const score = h => (/WebBridge|wb_serve|JsonBridge/i.test(h.file) ? 0 : 2) +
        (/\b\w*(Set|Publish|Put|Emit|Update|Tag|tag)\w*\s*\(/.test(h.snippet) ? 0 : 1);
      for (const r of widx.tagsOf(d.file, info.id).slice(0, 6)) {
        const hits = port ? (await port.findString(r.tag)).filter(h => !h.test && !h.comment) : [];
        hits.sort((a, b) => score(a) - score(b) || a.file.localeCompare(b.file) || a.line - b.line);
        data.tags.push({
          tag: r.tag, prop: r.prop,
          web: T({ kind: 'web', file: r.file, line: r.line, col: r.col, snippet: r.snippet, note: '標籤對照（顯示為 ' + r.prop + '）' }),
          port: hits.slice(0, 5).map(h => T(Object.assign(this.cppTarget('port', h, true), {
            note: (h.dead ? '在 #if 0 裡' : '發布／使用這個標籤'), weak: !!h.dead,
          }))),
        });
      }
    }

    // where the form's code touches this control (spbSave->Enabled = …, the declaration)
    if (info.id && !isForm && pi.classes.length) {
      const uses = { port: [], portMore: 0, golden: [], goldenMore: 0 };
      const useNote = h => (h.decl ? '宣告' : h.comment ? '註解' : '使用') + (h.dead ? '（在 #if 0 裡）' : '') + (h.test ? '（測試）' : '');
      if (port) {
        const u = await port.findMemberUses(pi.classes, info.id, 80);
        uses.port = u.slice(0, 15).map(h => T(Object.assign(this.cppTarget('port', h), { note: useNote(h), weak: h.comment || h.dead })));
        uses.portMore = Math.max(0, u.length - 15);
      }
      if (gold) {
        const u = await gold.findMemberUses(pi.classes, info.id, 80);
        uses.golden = u.slice(0, 15).map(h => T(Object.assign(this.cppTarget('golden', h), { note: useNote(h), weak: h.comment })));
        uses.goldenMore = Math.max(0, u.length - 15);
      }
      data.uses = uses;
    }
    data.cppPending = false;
    data.page = this.pageSummary(d, pi);
    return data;
  }

  pageSummary(d, pi) {
    const notes = [];
    if (pi.ir) {
      const how = pi.dfm && pi.dfm.how === 'guess'
        ? '（依元件名稱推測：吻合 ' + pi.dfm.hits + ' 個，' + Math.round(pi.dfm.score * 100) + '%）'
        : '（依頁面標題）';
      notes.push('DFM：' + (pi.ir.sourceDfm || path.basename(pi.ir.file)) + how + '，表單類別 ' + (pi.ir.formClass || '?'));
    } else {
      notes.push('找不到對應的 DFM：這頁只能顯示 HTML 屬性和網頁 JS。');
    }
    if (pi.iframe) notes.push('這頁有內嵌 iframe，設計檢視不載入它們（避免繞過網路封鎖）。');
    if (d.blocked.length) {
      notes.push('已擋下頁面的連線 ' + d.blocked.length + ' 種：' + d.blocked.slice(0, 3).map(b => b.uri || b.dir).join('、') + '（設計檢視不連機台）');
    }
    if (d.errors.length) notes.push('頁面 JS 錯誤 ' + d.errors.length + ' 筆，第一筆：' + d.errors[0].msg);
    if (!d.r.portRoot) notes.push('找不到 C++ 移植樹，可在設定 ht9045Designer.portRoot 指定。');
    if (!d.r.goldenRoot) notes.push('找不到 BCB6 原始碼資料夾，可在設定 ht9045Designer.goldenRoot 指定。');
    return { file: d.file, name: path.basename(d.file), mode: d.mode, notes };
  }

  // --- opening code --------------------------------------------------------
  codeColumn(d) {
    const dcol = d && d.panel.viewColumn;
    const eds = vscode.window.visibleTextEditors.filter(e => e.viewColumn && e.viewColumn !== dcol);
    const other = eds.find(e => !d || e.document !== d.doc);
    if (other) return other.viewColumn;
    const same = eds.find(e => d && e.document === d.doc);
    if (same) return Math.min(9, Math.max(same.viewColumn, dcol || 1) + 1);
    return dcol ? Math.min(9, dcol + 1) : vscode.ViewColumn.Beside;
  }

  sourceColumn(d) {
    const ex = vscode.window.visibleTextEditors.find(e => e.document === d.doc && e.viewColumn);
    if (ex) return ex.viewColumn;
    const dcol = d.panel.viewColumn;
    return dcol ? Math.min(9, dcol + 1) : vscode.ViewColumn.Beside;
  }

  async openTarget(t, d, intoBody) {
    if (!t) return;
    if (t.kind === 'html') { if (d) await this.revealSource(d, null, true, t); return; }
    try {
      const big5 = t.kind === 'golden' || t.kind === 'dfm';
      const uri = big5 ? vscode.Uri.file(t.file).with({ scheme: GOLDEN_SCHEME }) : vscode.Uri.file(t.file);
      const doc = await vscode.workspace.openTextDocument(uri);
      let pos = new vscode.Position(Math.max(0, (t.line || 1) - 1), Math.max(0, (t.col || 1) - 1));
      // WPF: "navigates to the existing handler" -- the caret inside its body, ready to type (not on its name)
      if (intoBody && !big5) pos = bodyPos(doc, pos.line) || pos;
      const ed = await vscode.window.showTextDocument(doc, {
        viewColumn: this.codeColumn(d), preview: true, selection: new vscode.Range(pos, pos),
      });
      ed.revealRange(new vscode.Range(pos, pos), vscode.TextEditorRevealType.InCenter);
    } catch (e) {
      vscode.window.showErrorMessage('開不了 ' + t.file + '：' + (e && e.message || e));
    }
  }

  /**
   * The server's WS form.event tables (generated: FileRW/*.gen.inc, FileRW/*_File.cpp, JsonBridge -- rows
   * {"control", "click"|"change", "<golden cpp>:<line> TfXxx::Method", &handler}). Read again after 20 s.
   */
  serverEvents(portRoot) {
    const c = this._srvEv;
    if (c && c.root === portRoot && Date.now() - c.at < 20000) return c.rows;
    const rows = [];
    // (the handler pointer after it -- &B_cbSelectHPFromDBChange -- is the function the server really runs)
    const re = /\{\s*"(\w+)"\s*,\s*"(click|change)"\s*,\s*"([^"]*?(T\w+)::(\w+))"(?:\s*,\s*&\s*([A-Za-z_][\w:]*))?/g;
    for (const sub of ['FileRW', 'JsonBridge']) {
      let names = [];
      try { names = fs.readdirSync(path.join(portRoot, sub)); } catch (e) { continue; }
      for (const n of names) {
        if (!/\.(cpp|inc|h)$/i.test(n)) continue;
        const f = path.join(portRoot, sub, n);
        let t;
        try { t = fs.readFileSync(f, 'utf8'); } catch (e) { continue; }
        if (t.indexOf('"click"') < 0 && t.indexOf('"change"') < 0) continue;
        re.lastIndex = 0;
        let m;
        while ((m = re.exec(t))) {
          let line = 1;
          for (let i = 0; i < m.index; i++) if (t.charCodeAt(i) === 10) line++;
          const row = { ctl: m[1], ev: m[2], golden: m[3], cls: m[4], meth: m[5], file: f, line, snippet: m[0].trim() };
          if (m[6]) {
            const fn = m[6].replace(/^.*::/, '');
            const dm = new RegExp('(^|\\n)([^\\n;{}]*\\b' + fn + '\\s*\\([^;{)]*\\)\\s*)\\{').exec(t);
            if (dm) {
              const at = dm.index + dm[1].length;
              row.fn = fn;
              row.fnLine = t.slice(0, at).split('\n').length;
              row.fnSnippet = dm[2].trim();
            }
          }
          rows.push(row);
        }
      }
    }
    this._srvEv = { root: portRoot, at: Date.now(), rows };
    return rows;
  }

  /** A double-click in the 事件表: a set event -> its code; an empty one -> a new handler (like BCB6). */
  async onEventGrid(data, evName, d) {
    const g = data && data.eventGrid;
    const row = g && g.rows.find(r => r.name === evName);
    if (!row) return null;
    // several selected and not one handler of them all: a double-click makes one for all (Windows Forms / BCB6)
    if (g.multi && g.multi.length && (!row.handler || row.mixed)) return this.cmdEventMany(d, data, row, row.conv);
    // the one C++ function that runs, straight away (no list to pick from)
    if (row.main != null && data.targets[row.main]) return this.openTarget(data.targets[row.main], d, true);
    if (row.idx >= 0) return this.openEvent(data, row.idx, d);
    const t = [row.port, row.server, row.web].filter(x => x != null)[0];
    if (t != null) return this.openTarget(data.targets[t], d);
    return this.cmdCreateEvent(d, data, row);
  }

  /**
   * 新增事件處理函式 (EastSun 20260930: in the C++ port tree): the declaration in the form's class (.h), an empty
   * body at the end of its .cpp -- one edit of the documents, nothing saved -- then the .cpp opens there.
   * What the page and the server still need (the page's *_ev.js CTLS list, the generated form.event table)
   * is said, not written: those belong to the event-porting pipeline (tools/editlist/*.py, gen_editlist.py).
   */
  /** The IO table's aliases (READ only; launch.json's W906_IOTABLE_PATH, else IO_Table.csv beside Gerneral.ini), cached by mtime. */
  ioAliasList(d) {
    const s = this.machineState(d && d.r);
    const f = s.hints.ioTable || (s.generalIni ? path.join(path.dirname(s.generalIni), 'IO_Table.csv') : null);
    if (!f) return null;
    let st;
    try { st = fs.statSync(f); } catch (e) { return null; }
    const c = this._ioAl;
    if (c && c.file === f && c.mtime === st.mtimeMs) return c.list;
    const list = aliasedit.ioAliases(f);
    this._ioAl = { file: f, mtime: st.mtimeMs, list: { file: f, names: list } };
    return this._ioAl.list;
  }

  /** Alias (EastSun: "名稱都要讓我改 alias"): the Alias part of the component's title, which the page's JS reads. */
  cmdSetAlias(d, data, value) {
    const id = data.comp.htmlId;
    const tag = id ? htmledit.startTagOf(d.doc.getText(), id) : null;
    if (!tag) { this.refuseEdit(d, '原始碼裡找不到「' + id + '」，Alias 沒有改。'); return null; }
    const title = (attrsOf(tag.text).find(a => a[0] === 'title') || [])[1] || '';
    const r = aliasedit.withAlias(title, value);
    if (r.error) { this.refuseEdit(d, r.error + '，沒有改。'); return null; }
    if (r.title === title) return null;
    const v = String(value || '').trim();
    const io = data.edit && data.edit.ioAliases;
    if (v && io && io.names.length && !io.names.includes(v)) {
      vscode.window.showWarningMessage('Alias「' + v + '」不在 IO 表（' + path.basename(io.file) + '）裡——這個元件會對不到任何 IO。還是照改了，Ctrl+Z 可以復原。');
    }
    d.post({ type: 'setAttrRaw', key: data.comp.key, name: 'title', value: r.title });
    vscode.window.setStatusBarMessage('$(edit) ' + id + ' 的 Alias：' + (aliasedit.aliasOf(title) || '（沒有）') + ' → ' + (v || '（拿掉）') + '（Ctrl+Z 復原）', 6000);
    return { from: aliasedit.aliasOf(title), to: v, title: r.title };
  }

  /**
   * A handler name typed into the 事件表: an empty event -> a new C++ handler of that name; one the designer (or
   * the port) has that the .dfm does not name -> renamed in the class (.h) and every TfXxx::name of its .cpp.
   * The .dfm's are BCB6's (golden, read only): not renamed.
   */
  async onEventName(data, evName, value, d) {
    const g = data && data.eventGrid;
    const row = g && g.rows.find(r => r.name === evName);
    if (!row) return null;
    const nu = String(value || '').trim();
    if (g.multi && g.multi.length) return this.cmdEventMany(d, data, row, nu);
    if (row.dfm) { this.refuseEdit(d, evName + ' 的 ' + row.handler + ' 是 BCB6 .dfm 指定的（golden，唯讀），名稱不在這裡改。'); return null; }
    // cleared (WPF: the name deleted = the event not attached any more; the code stays)
    if (!nu) return row.handler ? this.onEventReset(data, evName, d) : null;
    if (!/^[A-Za-z_]\w*$/.test(nu)) { this.refuseEdit(d, '「' + nu + '」不能當 C++ 函式名稱（英文字母、數字、_，不能用數字開頭），沒有改。'); return null; }
    if (!row.handler) return this.cmdCreateEvent(d, data, row, nu);
    if (row.handler === nu) return null;
    // a method the class already has (picked from the list): the event is attached to it instead (Windows Forms'
    // selection box); a new name: the handler is renamed
    const cls = data.eventGrid.formCls;
    const pf = await this.portClassFiles(d, cls);
    if (pf) {
      const hd = await vscode.workspace.openTextDocument(vscode.Uri.file(pf.hFile));
      if (cppstub.declares(hd.getText(), cls, nu)) return this.cmdUseHandler(d, data, row, cls, nu, hd);
    }
    return this.renameCppHandler(d, data, row, nu);
  }

  /**
   * 事件 with several selected (BCB6's Object Inspector, Windows Forms' Events tab): the name typed = THAT function for
   * every one of them -- made once (or the class's own one used), then each control's wiring (its page line, its row of
   * the generated table); '' = each one's designer wiring taken off. Nothing is renamed (a name typed with several
   * selected attaches, it does not rename). BCB6's .dfm ones (golden) and another page script's are not changed.
   */
  async cmdEventMany(d, data, row, nu) {
    const g = data.eventGrid;
    const n = g.multi.length + 1;
    if (row.multiRo) {
      this.refuseEdit(d, row.name + '：選取的元件裡有 BCB6 .dfm 指定的（golden，唯讀）或網頁 JS 自己送的處理函式，' + n + ' 個不能一起改——一個一個選來改。');
      return null;
    }
    if (!nu) return this.resetMany(d, data, row, [g.name].concat(g.multi));
    if (!/^[A-Za-z_]\w*$/.test(nu)) { this.refuseEdit(d, '「' + nu + '」不能當 C++ 函式名稱（英文字母、數字、_，不能用數字開頭），沒有改。'); return null; }
    const cls = g.formCls;
    const pf = await this.portClassFiles(d, cls);
    if (!pf) { vscode.window.showInformationMessage('移植樹裡找不到 ' + cls + ' 的 .h／.cpp，事件沒有接。'); return null; }
    const hDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(pf.hFile));
    // the primary one: the one-selected path (made, or the class's own used) -- unless it is that one already
    let first = { name: nu, wired: true, skipped: true };
    if (!(row.handler === nu && row.via === 'htd')) {
      first = cppstub.declares(hDoc.getText(), cls, nu) ? await this.cmdUseHandler(d, data, row, cls, nu, hDoc) : await this.cmdCreateEvent(d, data, row, nu);
      // (not added -- e.g. a parameter type the port lacks: the others are not wired to a handler that is not there)
      if (!first || first.refused) return first || null;
    }
    const dc = cppbridge.declOf(hDoc.getText(), cls, nu);
    if (!dc) { vscode.window.showInformationMessage(cls + ' 沒有宣告 ' + nu + '，其他 ' + g.multi.length + ' 個沒有接。'); return first; }
    // the others: wiring only, each in turn (each one's page line and table row from the text as it is by then)
    const done = [], notes = [];
    for (const id of g.multi) {
      const w = await this.wirePlan(d, { name: id, isForm: false }, { name: row.name }, cls, nu, dc.params, dc.access, this.stamp());
      this.htdWriteWire(w);
      if (!w.wired) { notes.push(id + '：' + w.notes.join('；')); continue; }
      const we = new vscode.WorkspaceEdit();
      for (const op of w.ops) we.replace(op.doc.uri, new vscode.Range(op.doc.positionAt(op.s), op.doc.positionAt(op.e)), op.text);
      if (!(await vscode.workspace.applyEdit(we))) { notes.push(id + '：沒有寫進去'); continue; }
      done.push(id);
    }
    if (!notes.length) row.mixed = false;
    if (this.props && this.props.data === data) this.props.show(data);
    this.log('事件 ' + row.name + '（多選 ' + n + ' 個）＝' + cls + '::' + nu + '：' + (first.skipped ? g.name + ' 本來就是' : g.name) + (done.length ? '、' + done.join('、') : '') +
      '（網頁未存檔）' + (notes.length ? '；沒有接：' + notes.join('；') : ''));
    vscode.window.showInformationMessage(row.name + ' 接到 ' + cls + '::' + nu + '：' + [g.name].concat(done).join('、') + '（' + (done.length + 1) + ' / ' + n + ' 個，網頁還沒存檔）。' +
      (notes.length ? '沒有接上：' + notes.join('；') + '。' : ''));
    return { name: nu, first, done, notes };
  }

  /** 重設 with several selected: each one's designer wiring of the event taken off (the C++ function stays). */
  async resetMany(d, data, row, ids) {
    const cls = data.eventGrid.formCls, ev = row.name;
    const rng = (doc, x) => new vscode.Range(doc.positionAt(x.s), doc.positionAt(x.e));
    // the generated table first, straight to disk (the build's file, never left unsaved)
    const gen = d.r.portRoot ? this.htdGen(d.r.portRoot) : { text: null };
    const rows0 = gen.text ? cppbridge.rowsOf(gen.text) : [];
    const hit = r => r.form === cls && r.event === ev && ids.includes(r.control);
    const saved = [];
    let genWhy = '';
    if (rows0.some(hit)) {
      if (gen.busy) genWhy = path.basename(gen.file) + ' 在編輯器裡有沒存的分頁，它的列沒有拿掉（關掉那個分頁、不存，再重設一次）';
      else {
        const genText = await this.htdRegen(d, rows0.filter(r => !hit(r)));
        const wr = genText != null ? this.htdWriteDisk([{ file: gen.file, buf: Buffer.from(genText, 'utf8') }]) : null;
        if (wr && wr.error) genWhy = wr.error;
        else if (wr) saved.push(path.basename(cppbridge.GEN_REL) + ' ' + rows0.filter(hit).length + ' 列');
      }
    }
    const done = [];
    for (const id of ids) {
      const pt = d.doc.getText();
      const line = jsevents.cppLines(pt).find(x => x.id === id && x.event === ev);
      if (!line) continue;
      const we = new vscode.WorkspaceEdit();
      for (const x of jsevents.setCpp(pt, id, line.type, line.form, '', ev).edits) we.replace(d.doc.uri, rng(d.doc, x), x.text);
      if (await vscode.workspace.applyEdit(we)) done.push(id);
    }
    if (!done.length && !saved.length) {
      vscode.window.showInformationMessage(ev + '：選取的 ' + ids.length + ' 個都沒有設計工具接的連線可以拿掉。' + (genWhy ? genWhy + '。' : ''));
      return null;
    }
    const old = row.handler;
    if (done.includes(data.eventGrid.name)) Object.assign(row, { handler: '', main: null, mainName: null, cmd: null, via: null, web: null, server: null, port: null, portState: null });
    if (done.length === ids.length) row.mixed = false;
    if (this.props && this.props.data === data) this.props.show(data);
    this.log('重設事件 ' + ev + '（多選 ' + ids.length + ' 個）：' + done.join('、') + '（網頁未存檔）' + (saved.length ? '；' + saved.join('、') + ' 直接存檔' : '') + '；C++ 函式沒有刪' + (genWhy ? '；' + genWhy : ''));
    vscode.window.showInformationMessage('已重設 ' + ev + '：' + (done.length ? done.join('、') + ' 的網頁連線（還沒存檔）' : '') + (saved.length ? (done.length ? '、' : '') + saved.join('、') + '（已經直接存檔）' : '') +
      '。C++ 函式' + (old ? ' ' + old : '') + '沒有刪。' + (genWhy ? genWhy + '。' : ''));
    return { reset: ev, ids: done, saved };
  }

  /** The class's .h and .cpp in the port tree (null when not found). */
  async portClassFiles(d, cls) {
    const port = d && d.r.portRoot ? this.sourceTree(d.r.portRoot, 'port') : null;
    if (!port || !cls) return null;
    await port.ensure();
    const files = Array.from(port.clsFiles.get(cls) || []).filter(f => !/[\\/]tests?[\\/]/i.test(f));
    const read = f => { try { return fs.readFileSync(f, 'utf8'); } catch (e) { return null; } };
    const hFile = files.filter(f => /\.h(pp)?$/i.test(f)).find(f => { const t = read(f); return t && cppstub.classBody(t, cls); });
    if (!hFile) return null;
    const base = path.basename(hFile).replace(/\.h(pp)?$/i, '');
    const cppFile = files.find(f => /\.cpp$/i.test(f) && path.basename(f).replace(/\.cpp$/i, '').toLowerCase() === base.toLowerCase()) ||
      files.filter(f => /\.cpp$/i.test(f)).sort((a, b) => (port.fileKeys.get(b) || []).length - (port.fileKeys.get(a) || []).length)[0];
    return cppFile ? { port, hFile, cppFile } : null;
  }

  async renameCppHandler(d, data, row, nu) {
    const cls = data.eventGrid.formCls;
    const pf = await this.portClassFiles(d, cls);
    if (!pf) { vscode.window.showInformationMessage('移植樹裡找不到 ' + cls + ' 的 .h／.cpp，名稱沒有改。'); return null; }
    const hDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(pf.hFile));
    const cDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(pf.cppFile));
    if (cppstub.declares(hDoc.getText(), cls, nu)) { this.refuseEdit(d, cls + ' 已經有「' + nu + '」了，沒有改。'); return null; }
    const r = cppstub.renameEdits(hDoc.getText(), cDoc.getText(), cls, row.handler, nu);
    if (!r.h.length && !r.cpp.length) { this.refuseEdit(d, '在 ' + path.basename(pf.hFile) + '／' + path.basename(pf.cppFile) + ' 找不到 ' + row.handler + '，沒有改。'); return null; }
    const we = new vscode.WorkspaceEdit();
    const rng = (doc, x) => new vscode.Range(doc.positionAt(x.s), doc.positionAt(x.e));
    for (const x of r.h) we.replace(hDoc.uri, rng(hDoc, x), x.text);
    for (const x of r.cpp) we.replace(cDoc.uri, rng(cDoc, x), x.text);
    // its wiring follows: the page's htdCpp line(s), the generated table's row
    const pageR = jsevents.renameCppEdits(d.doc.getText(), cls, row.handler, nu);
    for (const x of pageR) we.replace(d.doc.uri, rng(d.doc, x), x.text);
    let genN = 0, genText = null, genWhy = '';
    const gen = d.r.portRoot ? this.htdGen(d.r.portRoot) : { text: null };
    const genRows = gen.text ? cppbridge.rowsOf(gen.text) : [];
    if (genRows.some(x => x.form === cls && x.handler === row.handler)) {
      const port = this.sourceTree(d.r.portRoot, 'port');
      const sv = await this.htdServer(port);
      const sh = sv ? cppbridge.serverHook(sv.text, this.stamp()) : { error: 'no server' };
      const ctx = sh.fn ? await this.htdCtx(port, sh.fn) : null;
      const full = [];
      let eol = '\n';
      for (const x of genRows) {
        const renamed = x.form === cls && x.handler === row.handler;
        const dc = renamed ? cppbridge.declOf(hDoc.getText(), cls, row.handler) : null;
        const c = await this.htdRowCode(d, renamed ? Object.assign({}, x, { handler: nu }) : x, dc ? dc.params : undefined);
        if (c.error) continue;
        eol = c.eol;
        full.push(Object.assign({}, x, renamed ? { handler: nu } : {}, c));
        if (renamed) genN++;
      }
      if (gen.busy) { genN = 0; genWhy = path.basename(gen.file) + ' 在編輯器裡有沒存的分頁，它那一列沒有跟著改（關掉那個分頁、不存，再改一次名稱）'; }
      else if (ctx && ctx.cjson) genText = cppbridge.genFile(full, ctx, eol);
      else genN = 0;
    }
    // (the generated table goes to disk first: its row calls the new name only once the class has it -- until the
    //  .h / .cpp are saved it still compiles, see lib/cppbridge.js)
    if (genText != null) {
      const wr = this.htdWriteDisk([{ file: gen.file, buf: Buffer.from(genText, 'utf8') }]);
      if (wr.error) { genN = 0; genWhy = wr.error; }
    }
    if (!(await vscode.workspace.applyEdit(we))) { vscode.window.showWarningMessage('改名沒有寫進去（檔案可能是唯讀的）。'); return null; }
    // other files that name it (saved ones: the source index) -- said, not changed
    const others = (await pf.port.lookup([cls], row.handler)).filter(h => !web.samePath(h.file, pf.hFile) && !web.samePath(h.file, pf.cppFile));
    const old = row.handler;
    row.handler = nu;
    if (this.props && this.props.data === data) this.props.show(data);
    this.log('事件處理函式改名 ' + cls + '::' + old + ' → ' + nu + '：' + path.basename(pf.hFile) + ' ' + r.h.length + ' 處、' + path.basename(pf.cppFile) + ' ' + r.cpp.length + ' 處（未存檔）' +
      (genN ? '；' + cppbridge.GEN_REL + ' 已直接寫好' : '') + (genWhy ? '；' + genWhy : ''));
    const wired = pageR.length ? '、網頁 ' + pageR.length + ' 處' : '';
    vscode.window.showInformationMessage(cls + '::' + old + ' 改名為 ' + nu + '（' + path.basename(pf.hFile) + ' ' + r.h.length + ' 處、' + path.basename(pf.cppFile) + ' ' + r.cpp.length + ' 處' + wired + '；還沒存檔，Ctrl+Z 可以復原）。' +
      (genN ? path.basename(cppbridge.GEN_REL) + ' 那一列已經直接改好存檔（建置不會因為還沒存檔而壞）。' : '') + (genWhy ? genWhy + '。' : '') +
      (others.length ? '另外 ' + others.length + ' 個地方也寫到 ' + old + '（例如 ' + path.basename(others[0].file) + ':' + others[0].line + '），那些沒有改。' : ''));
    return { from: old, to: nu, h: r.h.length, cpp: r.cpp.length, page: pageR.length, gen: genN, others: others.length };
  }

  /**
   * 網頁事件 (EastSun: "網路js監聽器也要讓我填function"): the page's own JS handler of a DOM event, kept in the page's
   * <script id="htdEvents"> block (lib/jsevents.js) -- a name adds the function and its binding, a new one renames
   * it, '' takes the binding out. One edit of the page (nothing saved); the new function then opens.
   */
  async cmdJsEvent(d, data, ev, fn) {
    const id = data.comp.htmlId;
    if (!id || data.comp.isForm) { this.refuseEdit(d, '網頁事件要選一個有 id 的元件。'); return null; }
    const text = d.doc.getText();
    const r = jsevents.setBinding(text, id, String(ev || ''), fn);
    if (r.error) { this.refuseEdit(d, r.error + '，沒有改。'); return null; }
    if (!r.edits.length) return null;
    const we = new vscode.WorkspaceEdit();
    for (const x of r.edits) we.replace(d.doc.uri, new vscode.Range(d.doc.positionAt(x.s), d.doc.positionAt(x.e)), x.text);
    if (!(await vscode.workspace.applyEdit(we))) { vscode.window.showWarningMessage('網頁事件沒有寫進去。'); return null; }
    const now = jsevents.handlersOf(d.doc.getText(), id);
    if (data.edit && data.edit.jsEvents) { data.edit.jsEvents.handlers = now; if (this.props && this.props.data === data) this.props.show(data); }
    this.log('網頁事件 ' + id + '.' + ev + '：' + (String(fn || '').trim() || '（拿掉）') + (r.renamed ? '（' + r.renamed + ' 改名）' : '') + '（' + path.basename(d.file) + '，未存檔）');
    if (r.fnAdded || r.renamed) await this.openJsEvent(d, id, ev);
    else vscode.window.setStatusBarMessage('$(edit) ' + id + ' 的 ' + ev + '：' + (String(fn || '').trim() || '拿掉了') + '（Ctrl+Z 復原）', 6000);
    return { id, ev, fn: String(fn || '').trim(), added: r.fnAdded || null, renamed: r.renamed || null, edits: r.edits.length };
  }

  /** The function of a web event in the page's HTML, beside the designer. */
  async openJsEvent(d, id, ev) {
    const h = jsevents.handlersOf(d.doc.getText(), id)[ev];
    if (!h) return null;
    return this.revealSource(d, id, true, { range: [h.at, h.at] });
  }

  async cmdCreateEvent(d, data, row, nameOverride) {
    const g = data.eventGrid;
    const cls = g.formCls;
    const port = d && d.r.portRoot ? this.sourceTree(d.r.portRoot, 'port') : null;
    if (!port || !cls) { vscode.window.showInformationMessage('找不到這一頁的 C++ 表單類別（移植樹），不能新增事件處理函式。'); return null; }
    await port.ensure();
    const files = Array.from(port.clsFiles.get(cls) || []).filter(f => !/[\\/]tests?[\\/]/i.test(f));
    const read = f => { try { return fs.readFileSync(f, 'utf8'); } catch (e) { return null; } };
    const hFile = files.filter(f => /\.h(pp)?$/i.test(f)).find(f => { const t = read(f); return t && cppstub.classBody(t, cls); });
    if (!hFile) { vscode.window.showInformationMessage('移植樹裡找不到 class ' + cls + ' 的宣告（.h），不能新增事件處理函式。'); return null; }
    const base = path.basename(hFile).replace(/\.h(pp)?$/i, '');
    const cppFile = files.find(f => /\.cpp$/i.test(f) && path.basename(f).replace(/\.cpp$/i, '').toLowerCase() === base.toLowerCase()) ||
      files.filter(f => /\.cpp$/i.test(f)).sort((a, b) => (port.fileKeys.get(b) || []).length - (port.fileKeys.get(a) || []).length)[0];
    if (!cppFile) { vscode.window.showInformationMessage('移植樹裡找不到 ' + cls + ' 的 .cpp，不能新增事件處理函式。'); return null; }
    const hDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(hFile));
    const cDoc = await vscode.workspace.openTextDocument(vscode.Uri.file(cppFile));
    const name = nameOverride || row.conv;
    // the class has it already: the event uses it (Windows Forms' selection box: an existing handler)
    if (cppstub.declares(hDoc.getText(), cls, name)) return this.cmdUseHandler(d, data, row, cls, name, hDoc);
    const params = vclevents.signatureOf(row.name);
    // AI(W906-HTDESIGNER) 20261001: the build first -- a parameter type the form's header cannot see (TPoint, TRect,
    // TDragObject ... are not in the port's vclcompat) would make the whole 9050 build fail at F5: not added, said why
    const miss = cpptypes.missingTypes(params, cpptypes.includeClosure(hFile, d.r.portRoot));
    if (miss.length) {
      const why = row.name + ' 的參數用到 ' + miss.join('、') + '，移植樹裡 ' + path.basename(hFile) + ' 看不到它們的宣告（vclcompat 沒有）——新增的話 9050 會編不過，所以沒有新增。' +
        '要用這個事件，先在移植樹（vclcompat）補上這些型別，再新增一次。';
      this.log('沒有新增 ' + cls + '::' + name + '（' + row.name + '）：' + why);
      vscode.window.showInformationMessage(why);
      return { name: null, refused: true, missingTypes: miss, notes: [why] };
    }
    const stamp = this.stamp();
    const note = '//AI(W906-HTDESIGNER) ' + stamp + ': 設計檢視新增的事件處理函式（' + (g.name || '表單') + '.' + row.name + '）';
    const de = cppstub.declEdit(hDoc.getText(), cls, name, params, note);
    const fe = cppstub.defEdit(cDoc.getText(), cls, name, params, note);
    if (!de) { vscode.window.showInformationMessage('在 ' + path.basename(hFile) + ' 找不到 class ' + cls + ' 的本體。'); return null; }
    // 自動接線 (EastSun 20260930: "如果我有需要新增事件 其他連結 理應外掛程式自動幫我新增"): the page's sender in the
    // same edit as the handler; the build's part (the generated C++ table, the CMake entry, the server's htd.event
    // branch) straight to disk first (AI 20261001: an unsaved one of them broke the build)
    const hb = cppstub.classBody(hDoc.getText(), cls);
    const access = hb ? cppbridge.accessAt(cppstub.mask(hDoc.getText()), hb, de.at) : 'public';
    const wire = await this.wirePlan(d, g, row, cls, name, params, access, stamp);
    this.htdWriteWire(wire);
    const we = new vscode.WorkspaceEdit();
    const at = (doc, off) => new vscode.Range(doc.positionAt(off), doc.positionAt(off));
    we.replace(hDoc.uri, at(hDoc, de.at), de.text);
    we.replace(cDoc.uri, at(cDoc, fe.at), fe.text);
    for (const op of wire.ops) we.replace(op.doc.uri, new vscode.Range(op.doc.positionAt(op.s), op.doc.positionAt(op.e)), op.text);
    if (!(await vscode.workspace.applyEdit(we))) { vscode.window.showWarningMessage('新增 ' + name + ' 沒有寫進去（檔案可能是唯讀的）。'); return null; }
    this.log('新增事件處理函式 ' + cls + '::' + name + '（' + row.name + '）：' + path.basename(hFile) + ' 宣告、' + path.basename(cppFile) + ' 本體（未存檔）' +
      (wire.done.length ? '；自動接線 ' + wire.done.join('、') + '（未存檔）' : '') + (wire.saved.length ? '；直接存檔 ' + wire.saved.join('、') : '') +
      (wire.notes.length ? '；沒有接：' + wire.notes.join('；') : ''));
    const pos = new vscode.Position(fe.bodyLine, 4);
    await vscode.window.showTextDocument(cDoc, { preview: false, selection: new vscode.Range(pos, new vscode.Position(fe.bodyLine, 11)) });
    // the 事件表 shows it at once (the source index sees it when the files are saved)
    data.targets.push({ kind: 'port', file: cppFile, line: fe.bodyLine - 2, col: 1, snippet: 'void ' + cls + '::' + name + '(' + params + ')',
      note: '定義（剛新增，還沒存檔）', weak: false });
    row.port = data.targets.length - 1;
    row.main = row.port;
    row.mainName = name;
    row.portState = 'new';
    row.handler = name;
    if (wire.wired) {
      row.cmd = cppbridge.CMD;
      row.via = 'htd';
      data.targets.push({ kind: 'web', file: d.file, line: wire.webLine, col: 1, snippet: wire.webSnippet, note: '這一頁送出（設計工具的 htdCpp，剛新增）', weak: false });
      row.web = data.targets.length - 1;
      data.targets.push({ kind: 'port', file: wire.serverFile, line: wire.serverLine, col: 1, snippet: cppbridge.CMD + ' → ' + wire.fn, note: '伺服器分派 ' + cppbridge.CMD + '（剛新增）', weak: false });
      row.server = data.targets.length - 1;
    }
    if (this.props && this.props.data === data) this.props.show(data);
    const left = wire.wired ? [] : wire.notes;
    vscode.window.showInformationMessage('已新增 ' + cls + '::' + name + '（' + path.basename(hFile) + ' 宣告、' + path.basename(cppFile) + ' 空的本體）。' +
      (wire.wired ? '網頁也接好了：' + wire.done.join('、') + '——網頁 → WS ' + cppbridge.CMD + ' → 伺服器 → 這個函式。' : '') +
      '表單的 .h／.cpp' + (wire.wired ? '和網頁' : '') + '還沒存檔（不要就不存，或 Ctrl+Z）。' +
      (wire.saved.length ? '建置要用的 ' + wire.saved.join('、') + ' 已經直接存檔——不會讓建置壞掉（表單還沒存檔時，網頁按下去會回「函式還不在」）。' : '') +
      (wire.wired ? '存檔後重建、重開 ' + path.basename(wire.serverFile).replace(/\.\w+$/, '') + '，網頁按下去才會跑到它。' : '') +
      (left.length ? '網頁沒有接上：' + left.join('；') + '。' : ''));
    return { h: hFile, cpp: cppFile, name, left, wired: wire.wired, done: wire.done, saved: wire.saved, notes: wire.notes };
  }

  /**
   * An existing method of the class for the event (Windows Forms: "Use the selection box to choose an existing
   * handler"): only the wiring is added (page line, generated row; the server branch / CMake entry once) -- no code
   * written. A method declared without a compiled body (a GATE) is refused.
   */
  async cmdUseHandler(d, data, row, cls, name, hDoc) {
    const g = data.eventGrid;
    const hText = hDoc.getText();
    const dc = cppbridge.declOf(hText, cls, name);
    const port = this.sourceTree(d.r.portRoot, 'port');
    const st = port ? await port.defState([cls], name) : { state: 'none' };
    const od = st.state === 'live' ? null : this.dirtyHit(d.r.portRoot, new RegExp('\\b' + cls + '\\s*::\\s*' + name + '\\s*\\('), /\.cpp$/i);
    if (!dc || (st.state !== 'live' && !od)) {
      vscode.window.showInformationMessage(cls + '::' + name + ' 宣告了，但沒有會編譯的本體（可能是刻意沒有本體的 GATE），不能接到事件。');
      await vscode.window.showTextDocument(hDoc, { preview: false });
      return null;
    }
    const wire = await this.wirePlan(d, g, row, cls, name, dc.params, dc.access, this.stamp());
    this.htdWriteWire(wire);
    if (!wire.wired) { vscode.window.showInformationMessage(g.name + '.' + row.name + ' 沒有接到 ' + cls + '::' + name + '：' + wire.notes.join('；') + '。'); return { name, used: true, wired: false, notes: wire.notes }; }
    const we = new vscode.WorkspaceEdit();
    for (const op of wire.ops) we.replace(op.doc.uri, new vscode.Range(op.doc.positionAt(op.s), op.doc.positionAt(op.e)), op.text);
    if (!(await vscode.workspace.applyEdit(we))) { vscode.window.showWarningMessage('事件的連線沒有寫進去（檔案可能是唯讀的）。'); return null; }
    const def = st.state === 'live' && st.hit ? this.cppTarget('port', st.hit) : { kind: 'port', file: od.file, line: od.line, col: 1, snippet: od.snippet, note: '定義（還沒存檔）', weak: false };
    data.targets.push(def);
    row.port = data.targets.length - 1;
    row.main = row.port;
    row.mainName = name;
    row.portState = st.state === 'live' ? 'live' : 'new';
    row.handler = name;
    row.cmd = cppbridge.CMD;
    row.via = 'htd';
    data.targets.push({ kind: 'web', file: d.file, line: wire.webLine, col: 1, snippet: wire.webSnippet, note: '這一頁送出（設計工具的 htdCpp，剛新增）', weak: false });
    row.web = data.targets.length - 1;
    data.targets.push({ kind: 'port', file: wire.serverFile, line: wire.serverLine, col: 1, snippet: cppbridge.CMD + ' → ' + wire.fn, note: '伺服器分派 ' + cppbridge.CMD, weak: false });
    row.server = data.targets.length - 1;
    if (this.props && this.props.data === data) this.props.show(data);
    this.log('事件 ' + g.name + '.' + row.name + ' 用已經有的 ' + cls + '::' + name + '：' + wire.done.join('、') + '（未存檔）' + (wire.saved.length ? '；直接存檔 ' + wire.saved.join('、') : ''));
    await this.openTarget(def, d);
    vscode.window.showInformationMessage(g.name + '.' + row.name + ' 接到已經有的 ' + cls + '::' + name + '（沒有新增程式碼）：' + wire.done.join('、') +
      '（還沒存檔）。' + (wire.saved.length ? '建置要用的 ' + wire.saved.join('、') + ' 已經直接存檔。' : '') +
      '網頁存檔後重建、重開 ' + path.basename(wire.serverFile).replace(/\.\w+$/, '') + '，網頁按下去才會跑到它。');
    return { name, used: true, wired: true, done: wire.done, saved: wire.saved };
  }

  /**
   * 重設 (Windows Forms: right-click the event -> Reset; WPF: the name deleted): the event is not attached any more --
   * the page's htdCpp line and the generated row go; the C++ function stays ("you can't just delete handler code").
   * BCB6's .dfm ones and another page script's are not the designer's to take off.
   */
  async onEventReset(data, evName, d) {
    const g = data && data.eventGrid;
    const row = g && g.rows.find(r => r.name === evName);
    if (row && g.multi && g.multi.length) return this.cmdEventMany(d, data, row, '');
    if (!row || !row.handler) return null;
    if (row.dfm) { this.refuseEdit(d, evName + ' 的 ' + row.handler + ' 是 BCB6 .dfm 指定的（golden，唯讀），不能重設。'); return null; }
    if (row.cmd && row.via !== 'htd') { this.refuseEdit(d, evName + ' 是這一頁的 JS 自己送 WS 命令 ' + row.cmd + '（不是設計工具接的）；要拿掉請改網頁的 JS。'); return null; }
    const cls = g.formCls, id = g.name;
    const pt = d.doc.getText();
    const we = new vscode.WorkspaceEdit();
    const rng = (doc, x) => new vscode.Range(doc.positionAt(x.s), doc.positionAt(x.e));
    const done = [];
    const line = jsevents.cppLines(pt).find(x => x.id === id && x.event === evName);
    if (line) {
      for (const x of jsevents.setCpp(pt, id, line.type, line.form, '', evName).edits) we.replace(d.doc.uri, rng(d.doc, x), x.text);
      done.push(path.basename(d.file) + ' 那一行');
    }
    // the generated table's row: straight to disk (the build's file, never left unsaved)
    const gen = d.r.portRoot ? this.htdGen(d.r.portRoot) : { text: null };
    const rows0 = gen.text ? cppbridge.rowsOf(gen.text) : [];
    let genText = null, genWhy = '';
    if (rows0.some(r => r.form === cls && r.control === id && r.event === evName)) {
      if (gen.busy) genWhy = path.basename(gen.file) + ' 在編輯器裡有沒存的分頁，它那一列沒有拿掉（關掉那個分頁、不存，再重設一次）';
      else genText = await this.htdRegen(d, rows0.filter(r => !(r.form === cls && r.control === id && r.event === evName)));
    }
    if (!done.length && genText == null) {
      vscode.window.showInformationMessage(id + '.' + evName + ' 沒有設計工具接的網頁連線可以拿掉；C++ 的 ' + cls + '::' + row.handler + ' 還在（要刪請直接改程式碼）。' + (genWhy ? genWhy + '。' : ''));
      return null;
    }
    const saved = [];
    if (genText != null) {
      const wr = this.htdWriteDisk([{ file: gen.file, buf: Buffer.from(genText, 'utf8') }]);
      if (wr.error) genWhy = wr.error; else saved.push(path.basename(cppbridge.GEN_REL) + ' 那一列');
    }
    if (done.length && !(await vscode.workspace.applyEdit(we))) { vscode.window.showWarningMessage('重設沒有寫進去。'); return null; }
    const old = row.handler;
    Object.assign(row, { handler: '', main: null, mainName: null, cmd: null, via: null, web: null, server: null, port: null, portState: null });
    if (this.props && this.props.data === data) this.props.show(data);
    this.log('重設事件 ' + id + '.' + evName + '（' + cls + '::' + old + '）：' + done.concat(saved).join('、') + '（' + (done.length ? done.join('、') + ' 未存檔' : '') +
      (saved.length ? (done.length ? '；' : '') + saved.join('、') + ' 直接存檔' : '') + '；C++ 函式沒有刪）' + (genWhy ? '；' + genWhy : ''));
    vscode.window.showInformationMessage('已重設 ' + id + '.' + evName + '：拿掉 ' + done.concat(saved).join('、') + '（' + (done.length ? done.join('、') + '還沒存檔' : '') +
      (saved.length ? (done.length ? '，' : '') + saved.join('、') + '已經直接存檔' : '') + '）。C++ 的 ' + cls + '::' + old + ' 沒有刪——不要的話自己刪。' + (genWhy ? genWhy + '。' : ''));
    return { reset: evName, handler: old, done, saved };
  }

  /** The generated file's text for `rows` (each one's call made from the port again). null: no form.event to take the rules from. */
  async htdRegen(d, rows) {
    const port = this.sourceTree(d.r.portRoot, 'port');
    const sv = await this.htdServer(port);
    const sh = sv ? cppbridge.serverHook(sv.text, this.stamp()) : null;
    if (!sh || !sh.fn) return null;
    const ctx = await this.htdCtx(port, sh.fn);
    if (!ctx.cjson) return null;
    const full = [];
    let eol = '\r\n';
    for (const r of rows) {
      const c = await this.htdRowCode(d, r);
      if (c.error) continue;
      eol = c.eol;
      full.push(Object.assign({}, r, c));
    }
    return cppbridge.genFile(full, ctx, eol);
  }

  /** 所有找到的程式碼 (right-click): every place found for the event, to pick one. */
  async onEventAll(data, evName, d) {
    const g = data && data.eventGrid;
    const row = g && g.rows.find(r => r.name === evName);
    if (!row) return null;
    const ks = [];
    const add = k => { if (k != null && data.targets[k] && !ks.includes(k)) ks.push(k); };
    add(row.main);
    if (row.idx >= 0 && data.events[row.idx]) data.events[row.idx].targets.forEach(add);
    [row.web, row.server, row.port].forEach(add);
    if (!ks.length) { vscode.window.showInformationMessage(evName + '：沒有找到程式碼。'); return null; }
    const items = ks.map(k => { const t = data.targets[k]; return { label: KIND[t.kind].icon + ' ' + KIND[t.kind].label + (t.note ? '　' + t.note : ''), description: this.relName(t.file, d) + ':' + t.line, detail: t.snippet || '', t }; });
    const pick = await vscode.window.showQuickPick(items, { placeHolder: evName + '：要跳到哪一份程式碼？', matchOnDescription: true, matchOnDetail: true });
    if (pick) await this.openTarget(pick.t, d);
    return pick ? pick.t : null;
  }

  stamp() {
    const day = new Date();
    return day.getFullYear() + String(day.getMonth() + 1).padStart(2, '0') + String(day.getDate()).padStart(2, '0');
  }

  /**
   * The designer's generated C++ event table, ON DISK (AI 20261001: it used to be an untitled document until saved,
   * while CMakeLists.txt already named it -- the build failed). -> { file, text (null: not there yet), exists, busy }
   * busy: an editor holds unsaved text of it (or an old untitled tab that a save would write there).
   */
  htdGen(portRoot) {
    const f = path.join(portRoot, ...cppbridge.GEN_REL.split('/'));
    const b = this.disk.read(f);
    return { file: f, text: b ? b.toString('utf8') : null, exists: !!b, busy: this.unsavedDoc(f) };
  }

  /** An editor holds unsaved text of the file (a changed document, or an untitled one with that path). */
  unsavedDoc(f) {
    return vscode.workspace.textDocuments.some(x => x.uri && (x.isDirty || x.uri.scheme === 'untitled') && web.samePath(x.uri.fsPath, f));
  }

  /** The server's file (the one with form.event's dispatch), its bytes and its text read as latin1 (offsets = bytes). */
  async htdServer(port) {
    const disp = port ? (await port.findString('form.event')).filter(h => h.dispatch && !h.dead && !h.test && !h.comment) : [];
    if (!disp.length) return null;
    const f = disp[0].file, b = this.disk.read(f);
    return b ? { file: f, buf: b, text: b.toString('latin1'), busy: this.unsavedDoc(f) } : null;
  }

  /** Files to disk in this order (each one whole, through a temporary): -> { written: [file], error? } */
  htdWriteDisk(list) {
    const written = [];
    for (const w of list) {
      try { this.disk.write(w.file, w.buf); written.push(w.file); } catch (e) {
        return { written, error: path.basename(w.file) + ' 寫不進去（' + (e && e.message || e) + '）' + (written.length ? '；已寫好的：' + written.map(f => path.basename(f)).join('、') : '') };
      }
    }
    return { written };
  }

  /**
   * The build's part of a wire plan to disk -- the generated table first, then the CMake entry, then the server's
   * branch: stopped anywhere, the tree still builds (the table alone is not compiled; with its CMake entry it only adds
   * an unused function; the server calls it last). A failure = not wired, said. Sets wire.saved.
   */
  htdWriteWire(wire) {
    wire.saved = [];
    if (!wire.wired || !wire.disk || !wire.disk.length) return wire;
    const wr = this.htdWriteDisk(wire.disk);
    wire.saved = wire.disk.filter(w => wr.written.includes(w.file)).map(w => w.what);
    if (wr.error) { wire.wired = false; wire.ops = []; wire.notes.push(wr.error); }
    return wire;
  }

  /** What the generated file takes from the form.event implementation: its JSON header, form lock, running flags. */
  async htdCtx(port, fn) {
    const root = port.root;
    const rel = f => path.relative(root, f).split(path.sep).join('/');
    const read = f => { try { return fs.readFileSync(f, 'utf8'); } catch (e) { return ''; } };
    const ctx = { fn, running: [] };
    const impl = (await port.findFuncDefs(fn.replace(/HtdEvent$/, 'FormEvent'))).find(x => !x.dead && !x.test);
    const it = impl ? read(impl.file) : '';
    const incs = [];
    const ire = /^[ \t]*#[ \t]*include[ \t]+"([^"]+)"/gm;
    let m;
    while ((m = ire.exec(it))) incs.push(m[1]);
    const cj = incs.find(i => /(^|\/)cJSON\.h$/i.test(i)) || (() => { const f = port.files.find(x => /[\\/]cJSON\.h$/i.test(x)); return f ? rel(f) : null; })();
    ctx.cjson = cj;
    const lk = /([\w:]*FormLock)\s*\(\s*\)\s*;/.exec(it), ul = /([\w:]*FormUnlock)\s*\(\s*\)\s*;/.exec(it);
    const lh = incs.find(i => /\bFormLock\s*\(/.test(read(path.join(root, i))));
    if (lk && ul && lh) { ctx.lockHeader = lh; ctx.lock = lk[1]; ctx.unlock = ul[1]; }
    const rre = /^[ \t]*extern[ \t]+bool[ \t]+(\w+)[ \t]*;/gm;
    while ((m = rre.exec(it))) ctx.running.push(m[1]);
    const sh = Array.from(port.clsFiles.get('TShiftState') || []).find(f => /\.h(pp)?$/i.test(f));
    ctx.shiftHeader = sh ? rel(sh) : null;
    return ctx;
  }

  /** One row of the generated table, made from the port: its form's header and the call. -> { header, call } | { error } */
  async htdRowCode(d, r, params) {
    const pf = await this.portClassFiles(d, r.form);
    if (!pf) return { error: '移植樹裡找不到 ' + r.form + ' 的 .h／.cpp' };
    const hText = (await vscode.workspace.openTextDocument(vscode.Uri.file(pf.hFile))).getText();
    const cText = (await vscode.workspace.openTextDocument(vscode.Uri.file(pf.cppFile))).getText();
    const obj = cppbridge.globalOf(hText, cText, r.form);
    if (!obj) return { error: r.form + ' 沒有全域物件（像 extern ' + r.form + ' *fXxx;），網頁叫不到它的函式' };
    let ps = params;
    if (ps == null) {
      const dc = cppbridge.declOf(hText, r.form, r.handler);
      if (!dc) return { error: r.form + '::' + r.handler + ' 沒有宣告在 ' + path.basename(pf.hFile) };
      if (dc.access !== 'public') return { error: r.form + '::' + r.handler + ' 宣告在 ' + dc.access + ' 裡，伺服器叫不到' };
      ps = dc.params;
    }
    const sender = cppbridge.hasMember(hText, r.form, r.control) ? 'HtdSender(' + obj + '->' + r.control + ')' : 'nullptr';
    const call = cppbridge.callOf(ps, obj, r.handler, sender);
    if (call.error) return { error: r.form + '::' + r.handler + '：' + call.error };
    return { header: path.relative(d.r.portRoot, pf.hFile).split(path.sep).join('/'), obj, call, eol: /\r\n/.test(hText) ? '\r\n' : '\n' };
  }

  /** The page's script that sends WS commands (a .js with rawCmd), loaded or to be loaded. -> { src } (to add) | null (loaded) | { error } */
  htdClient(d) {
    const t = d.doc.getText();
    const dir = path.dirname(d.file);
    const has = f => { try { return /\brawCmd\s*[:=]/.test(fs.readFileSync(f, 'utf8')); } catch (e) { return false; } };
    const re = /<script\b[^>]*\bsrc\s*=\s*["']([^"']+)["']/gi;
    let m;
    while ((m = re.exec(t))) { if (!/^[a-z]+:/i.test(m[1]) && has(path.resolve(dir, m[1].split(/[?#]/)[0]))) return null; }
    const cands = [];
    for (const base of [dir, d.r.webRoot, d.r.webRoot && path.join(d.r.webRoot, 'js')]) {
      if (!base) continue;
      try { for (const n of fs.readdirSync(base).sort()) if (/\.js$/i.test(n)) cands.push(path.join(base, n)); } catch (e) { /* none */ }
    }
    const hit = cands.find(has);
    if (!hit) return { error: '找不到送命令的程式（有 rawCmd 的 .js）' };
    return { src: path.relative(dir, hit).split(path.sep).join('/') };
  }

  /**
   * 自動接線: the edits that make the page's control reach the new handler (lib/cppbridge.js explains the chain).
   * Everything is checked first; anything missing -> nothing wired and the reason said (the handler is still added).
   * -> { ops: [{ doc, s, e, text }], done, notes, wired, ... }
   */
  async wirePlan(d, g, row, cls, handler, params, access, stamp) {
    const out = { ops: [], disk: [], saved: [], done: [], notes: [], wired: false };
    const DOM = ['click', 'dblclick', 'mousedown', 'mouseup', 'mousemove', 'mouseenter', 'mouseleave', 'keydown', 'keyup', 'keypress', 'change', 'focus', 'blur', 'contextmenu'];
    const type = fmt.domTypesOf(row.name).find(x => DOM.includes(x));
    if (g.isForm || !g.name) { out.notes.push('表單本身的事件網頁不會送'); return out; }
    if (!type) { out.notes.push(row.name + ' 在網頁上沒有對應的事件'); return out; }
    if (access !== 'public') { out.notes.push(cls + '::' + handler + ' 會宣告在 ' + access + ' 裡，伺服器叫不到'); return out; }
    const port = this.sourceTree(d.r.portRoot, 'port');
    const code = await this.htdRowCode(d, { form: cls, handler, control: g.name, event: row.name }, params);
    if (code.error) { out.notes.push(code.error); return out; }
    const client = this.htdClient(d);
    if (client && client.error) { out.notes.push(client.error); return out; }
    const pt = d.doc.getText();
    const pe = jsevents.setCpp(pt, g.name, type, cls, handler, row.name);
    if (pe.error) { out.notes.push(pe.error); return out; }
    // the build's part -- the server's branch, the CMake entry, the generated table -- is read and written ON DISK, byte
    // for byte (the server's file is not all valid UTF-8: through an editor's UTF-8 save its other bytes would change)
    const sv = await this.htdServer(port);
    if (!sv) { out.notes.push('伺服器程式裡找不到 form.event 的分派'); return out; }
    const sh = cppbridge.serverHook(sv.text, stamp);
    if (sh.error) { out.notes.push(sh.error); return out; }
    const ctx = await this.htdCtx(port, sh.fn);
    if (!ctx.cjson) { out.notes.push('移植樹裡找不到 cJSON.h（伺服器要用它讀網頁送來的值）'); return out; }
    const cm = path.join(d.r.portRoot, 'CMakeLists.txt');
    const cmB = this.disk.read(cm);
    const relS = path.relative(d.r.portRoot, sv.file).split(path.sep).join('/');
    const ch = cmB ? cppbridge.cmakeHook(cmB.toString('latin1'), relS, cppbridge.GEN_REL) : { error: '找不到 CMakeLists.txt' };
    if (ch.error) { out.notes.push(ch.error); return out; }
    const gen = this.htdGen(d.r.portRoot);
    // the designer writes them now: an editor with unsaved text of one would later overwrite it (or be overwritten)
    const busy = [gen.busy && gen.file, sh.edit && sv.busy && sv.file, ch.edit && this.unsavedDoc(cm) && cm].filter(Boolean);
    if (busy.length) {
      out.notes.push(busy.map(f => path.basename(f)).join('、') + ' 在編輯器裡有還沒存的修改（或沒存的分頁）——設計工具要直接寫它：先存檔，或關掉那個分頁（不存），再新增一次');
      return out;
    }
    const srvB = sh.edit ? cppbridge.spliceBytes(sv.buf, sh.edit) : null;
    const cmB2 = ch.edit ? cppbridge.spliceBytes(cmB, ch.edit) : null;
    if ((sh.edit && !srvB) || (ch.edit && !cmB2)) { out.notes.push('伺服器分派或 CMake 那一行有不能逐位元組寫入的字'); return out; }
    // (one row per control + event; the same handler may serve several events)
    const keep = cppbridge.rowsOf(gen.text || '').filter(r => !(r.form === cls && r.control === g.name && r.event === row.name));
    const rows = [];
    for (const r of keep) {
      const c = await this.htdRowCode(d, r);
      if (c.error) out.notes.push('（' + c.error + '：產生檔裡這一列拿掉）'); else rows.push(Object.assign({}, r, c));
    }
    rows.push({ form: cls, handler, control: g.name, event: row.name, header: code.header, call: code.call });
    // the page: the htdCpp line (and the helper, and the client script when the page does not load it)
    const block = jsevents.findBlock(pt);
    const tag = client ? '<script src="' + client.src + '"></script>' + (/\r\n/.test(pt) ? '\r\n' : '\n') : '';
    for (const x of pe.edits) out.ops.push({ doc: d.doc, s: x.s, e: x.e, text: (!block && tag ? tag : '') + x.text });
    if (block && tag) out.ops.push({ doc: d.doc, s: block.start, e: block.start, text: tag });
    out.done.push(path.basename(d.file) + '（htdCpp' + (client ? '＋載入 ' + path.basename(client.src) : '') + '）');
    // to disk, in this order (htdWriteWire): the table, the CMake entry, the server's branch
    out.disk = [{ file: gen.file, buf: Buffer.from(cppbridge.genFile(rows, ctx, code.eol), 'utf8'), what: cppbridge.GEN_REL + (gen.exists ? '' : '（新檔）') }];
    if (cmB2) out.disk.push({ file: cm, buf: cmB2, what: 'CMakeLists.txt（' + path.basename(sv.file).replace(/\.\w+$/, '') + ' 的來源清單）' });
    if (srvB) out.disk.push({ file: sv.file, buf: srvB, what: path.basename(sv.file) + ' 的 ' + cppbridge.CMD + ' 分支' });
    // where the page line / the server branch will be (for the 事件表)
    const after = this.applyOps(pt, out.ops.filter(o => o.doc === d.doc));
    const li = jsevents.cppLines(after).find(x => x.id === g.name && x.event === row.name);
    out.webLine = li ? after.slice(0, li.s).split('\n').length : 1;
    out.webSnippet = li ? after.slice(li.s, li.e).trim() : '';
    const hookAt = sh.edit ? sh.edit.s : (/\.cmd\s*==\s*"htd\.event"/.exec(sv.text) || { index: 0 }).index;
    out.serverFile = sv.file;
    out.serverLine = sv.text.slice(0, hookAt).split('\n').length;
    out.fn = sh.fn;
    out.wired = true;
    return out;
  }

  /** The first match of `re` in an open, not-saved document under `root` (a change the source index cannot see yet). */
  dirtyHit(root, re, fileRe) {
    if (!root) return null;
    for (const doc of vscode.workspace.textDocuments) {
      if (!doc.isDirty || !doc.uri || (fileRe && !fileRe.test(doc.uri.fsPath))) continue;
      const rel = path.relative(root, doc.uri.fsPath);
      if (!rel || rel.startsWith('..') || path.isAbsolute(rel)) continue;
      const t = doc.getText();
      const m = re.exec(t);
      if (!m) continue;
      const ls = t.lastIndexOf('\n', m.index) + 1, le = t.indexOf('\n', m.index);
      return { file: doc.uri.fsPath, line: t.slice(0, m.index).split('\n').length, snippet: t.slice(ls, le < 0 ? t.length : le).trim().slice(0, 200) };
    }
    return null;
  }

  /** The text after non-overlapping edits { s, e, text } (for positions after an edit). */
  applyOps(text, ops) {
    let t = String(text || '');
    for (const o of ops.slice().sort((a, b) => b.s - a.s)) t = t.slice(0, o.s) + o.text + t.slice(o.e);
    return t;
  }

  async openEvent(data, i, d) {
    let ev = data.events[i];
    if (!ev) return;
    if (data.cppPending && d && d.sel && d.sel.promise) {
      const full = await d.sel.promise;
      if (full && full.events[i] && full.events[i].name === ev.name) { data = full; ev = full.events[i]; }
    }
    const seen = new Set();
    const ts = [];
    for (const k of ev.targets) {
      const t = data.targets[k];
      if (!t) continue;
      const key = t.kind + '|' + t.file.toLowerCase() + '|' + t.line;
      if (seen.has(key)) continue;
      seen.add(key);
      ts.push(t);
    }
    if (!ts.length) {
      vscode.window.showInformationMessage(ev.name + ' → ' + ev.handler + '：網頁 JS、C++ 移植樹、BCB6 原始碼都找不到這個事件的程式碼。');
      return;
    }
    if (ts.length === 1) return this.openTarget(ts[0], d);
    const pref = this.cfg().get('eventJump') || 'ask';
    if (pref !== 'ask') {
      const kind = pref === 'cpp' ? 'port' : pref;
      const t = ts.find(x => x.kind === kind && !x.weak) || ts.find(x => x.kind === kind);
      if (t) return this.openTarget(t, d);
    }
    const items = ts.map(t => ({
      label: KIND[t.kind].icon + ' ' + KIND[t.kind].label + (t.note ? '　' + t.note : ''),
      description: this.relName(t.file, d) + ':' + t.line,
      detail: t.snippet || '',
      t,
    }));
    const pick = await vscode.window.showQuickPick(items, {
      placeHolder: ev.name + ' → ' + ev.handler + '：要跳到哪一份程式碼？',
      matchOnDescription: true, matchOnDetail: true,
    });
    if (pick) return this.openTarget(pick.t, d);
  }

  relName(file, d) {
    const bases = d ? [d.r.webRoot, d.r.portRoot, d.r.goldenRoot].filter(Boolean) : [];
    for (const b of bases) {
      const r = path.relative(b, file);
      if (!r.startsWith('..') && !path.isAbsolute(r)) return r;
    }
    return file;
  }

  async revealSource(d, id, focus, t) {
    if (!d) return;
    let range = t && t.range ? t.range : null;
    if (!range) {
      const at = web.findIdAttr(d.doc.getText(), id);
      if (!at) {
        vscode.window.showInformationMessage('HTML 原始碼裡找不到 id="' + id + '"（可能是頁面 JS 動態產生的元件）。');
        return;
      }
      range = [at.start, at.end];
    }
    const r = new vscode.Range(d.doc.positionAt(range[0]), d.doc.positionAt(range[1]));
    const ed = await vscode.window.showTextDocument(d.doc, {
      viewColumn: this.sourceColumn(d), preview: false, preserveFocus: !focus, selection: r,
    });
    ed.revealRange(r, vscode.TextEditorRevealType.InCenter);
  }

  /** Designer -> source: move an already visible HTML editor, never open or focus one. */
  syncSource(d, id) {
    if (!id || !this.cfg().get('syncSource')) return;
    const ed = vscode.window.visibleTextEditors.find(e => e.document === d.doc);
    if (!ed) return;
    const at = web.findIdAttr(d.doc.getText(), id);
    if (!at) return;
    const r = new vscode.Range(d.doc.positionAt(at.start), d.doc.positionAt(at.end));
    ed.selection = new vscode.Selection(r.start, r.end);
    ed.revealRange(r, vscode.TextEditorRevealType.InCenterIfOutsideViewport);
  }

  /** Source -> designer: the cursor sits in a start tag with an id. */
  onEditorSelection(e) {
    const K = vscode.TextEditorSelectionChangeKind;
    if (e.kind !== K.Mouse && e.kind !== K.Keyboard) return;
    const d = Array.from(this.designers).find(x => x.doc === e.textEditor.document);
    if (!d) return;
    clearTimeout(this._edTimer);
    this._edTimer = setTimeout(() => {
      const off = d.doc.offsetAt(e.selections[0].active);
      // WPF's split view: the cursor anywhere in an element's markup (its tag, its text, a child without an id)
      const id = this.idAtOffset(d.doc.getText(), off);
      if (id && id !== d.lastSelId) d.post({ type: 'selectId', id, origin: 'editor' });
    }, 200);
  }

  onPropsMessage(m) {
    const d = this.propsDesigner;
    const data = this.props.data;
    switch (m && m.type) {
      case 'ready': this.props.show(this.props.data); break;
      case 'revealProp':
        // a double-click on a property's name: where it is written in the HTML, the value selected (else the element)
        if (d && data && data.comp && typeof m.prop === 'string') {
          const pid = data.comp.isForm ? '@form' : data.comp.htmlId;
          const pr = pid ? htmledit.propRange(d.doc.getText(), pid, m.prop) : null;
          this.revealSource(d, pid, true, pr ? { range: pr } : (data.comp.html != null ? data.targets[data.comp.html] : null));
          return { prop: m.prop, range: pr };
        }
        break;
      case 'open': if (data) this.openTarget(data.targets[m.i], d); break;
      case 'openEvent': if (data) this.openEvent(data, m.i, d); break;
      case 'eventGrid': if (data) this.onEventGrid(data, m.name, d); break;
      case 'openDfmLine':
        if (data && data.dfmFile && m.line > 0) this.openTarget({ kind: 'dfm', file: data.dfmFile, line: m.line, col: 1 }, d);
        break;
      case 'revealSource':
        if (d && data) this.revealSource(d, data.comp.htmlId, true, data.comp.html != null ? data.targets[data.comp.html] : null);
        break;
      case 'copy':
        vscode.env.clipboard.writeText(String(m.text || ''));
        vscode.window.setStatusBarMessage('已複製：' + m.text, 2000);
        break;
      case 'setLayout':
      case 'setCaption':
      case 'setLook':
      case 'align':
        // several selected: WPF's grid sets the value on every one of them
        if (d && data && data.comp) this.forwardEdit(d, data.comp.key, m, !!(data.edit && data.edit.multi && data.edit.multi.length));
        break;
      case 'resetToDfm':
        // the panel's 全部改回 DFM: the selection (all of it) back to the .dfm, one undo step
        if (d) return this.cmdResetToDfm(d);
        break;
      case 'resetProp':
        // AI(W906-HTDESIGNER) 20261001: one property's Reset with several selected = EACH back to its OWN .dfm value
        // (WPF / the Object Inspector); it used to write the primary one's value into all of them
        if (d && typeof m.prop === 'string') return this.cmdResetToDfm(d, null, m.prop);
        break;
      case 'rename':
        // the pencil by the name at the top of the panel, or the Name field (WPF's) with the new name
        if (d && this.active === d) return this.cmdRename(typeof m.name === 'string' ? m.name : undefined);
        break;
      case 'setAlias':
        if (d && data && data.comp) return this.cmdSetAlias(d, data, m.value);
        break;
      case 'eventName':
        if (d && data) return this.onEventName(data, m.event, m.value, d);
        break;
      case 'eventReset':
        if (d && data) return this.onEventReset(data, m.name, d);
        break;
      case 'selectId':
        // the tag navigator: a parent picked in the panel's path
        if (d && typeof m.id === 'string' && m.id) { d.lastSelId = m.id; d.post({ type: 'selectId', id: m.id, origin: 'editor' }); }
        break;
      case 'eventAll':
        if (d && data) return this.onEventAll(data, m.name, d);
        break;
      case 'jsEvent':
        if (d && data && data.comp) return this.cmdJsEvent(d, data, m.ev, m.fn);
        break;
      case 'openJsEvent':
        if (d && data && data.comp) return this.openJsEvent(d, data.comp.htmlId, m.ev);
        break;
      case 'setStyleProp': {
        // one style declaration of the source's start tag, as written (value null / '' = remove it)
        if (!d || !data || !data.comp) break;
        const name = String(m.name || '').trim().toLowerCase();
        const value = m.value == null || String(m.value).trim() === '' ? null : String(m.value).trim();
        if (!CSS_NAME.test(name) || (value !== null && CSS_BAD.test(value))) {
          this.refuseEdit(d, '樣式「' + name + (value ? ': ' + value : '') + '」不能寫（名稱只能是英文小寫與 -，值不能有 ; { } < > \\）。');
          break;
        }
        if (/^(left|top|right|bottom|width|height|position|inset)$/.test(name) && this.refuseLocked(d, [data.comp.htmlId], '改位置')) break;
        d.post({ type: 'setStyleRaw', key: data.comp.key, name, value });
        break;
      }
      case 'setAttrProp': {
        if (!d || !data || !data.comp) break;
        const name = String(m.name || '').trim().toLowerCase();
        if (!ATTR_EDIT.test(name)) { this.refuseEdit(d, '屬性「' + name + '」不在這裡改（id 是網頁程式找元件用的；on… 是事件）。'); break; }
        d.post({ type: 'setAttrRaw', key: data.comp.key, name, value: m.value == null ? null : String(m.value) });
        break;
      }
      default: break;
    }
  }

  // --- commands ------------------------------------------------------------
  async cmdOpen(uri) {
    if (!(uri instanceof vscode.Uri)) {
      const ed = vscode.window.activeTextEditor;
      if (ed && /\.html?$/i.test(ed.document.fileName)) uri = ed.document.uri;
    }
    if (!uri) return this.cmdOpenPage();
    const ed = vscode.window.activeTextEditor;
    const fromEditor = ed && ed.document.uri.toString() === uri.toString();
    await vscode.commands.executeCommand('vscode.openWith', uri, VIEW_TYPE, fromEditor ? vscode.ViewColumn.Beside : undefined);
  }

  async cmdOpenPage() {
    const r = roots.resolveRoots(null, this.wsFolders(), this.over());
    let webRoot = r.webRoot;
    if (!webRoot) {
      const pick = await vscode.window.showOpenDialog({ canSelectFolders: true, canSelectFiles: false, openLabel: '這是 web 資料夾' });
      if (!pick || !pick.length) return;
      webRoot = pick[0].fsPath;
    }
    // first: the side bar's 頁面 list (it may be hidden); the current page marked
    const cur = this.active ? this.active.file.toLowerCase() : '';
    const items = [{ label: '$(list-tree) 在方案總管顯示頁面清單', description: '（網頁 → 頁面；被收起或隱藏時叫回來）', showList: true }];
    for (const dir of [path.join(webRoot, 'page'), webRoot]) {
      let names = [];
      try { names = fs.readdirSync(dir).filter(n => /\.html?$/i.test(n)).sort(); } catch (e) { names = []; }
      for (const n of names) {
        const f = path.join(dir, n);
        let head = '';
        try {
          const fd = fs.openSync(f, 'r');
          const buf = Buffer.alloc(4096);
          const len = fs.readSync(fd, buf, 0, 4096, 0);
          fs.closeSync(fd);
          head = buf.subarray(0, len).toString('utf8');
        } catch (e) { head = ''; }
        const t = pageinfo.parseTitle(head);
        const isCur = f.toLowerCase() === cur;
        items.push({ label: (isCur ? '$(eye) ' : '') + n, description: (isCur ? '● 目前　' : '') + (t ? t.title : ''), detail: path.relative(webRoot, f), file: f });
      }
    }
    const pick = await vscode.window.showQuickPick(items, { placeHolder: '選一個頁面，用設計檢視開啟（打字可以篩選；' + webRoot + '）', matchOnDescription: true });
    if (pick && pick.showList) return this.cmdShowPages();
    if (pick) await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(pick.file), VIEW_TYPE);
  }

  /** The side bar's 頁面 list shown (VS Code's view focus command also brings back a hidden view). */
  async cmdShowPages() {
    // (0.139: the pages are in 方案總管, under 網頁 -> 頁面)
    try { await vscode.commands.executeCommand('ht9045Designer.solution.focus'); } catch (e) { /* ignore */ }
    if (this.solution) await this.solution.revealPages();
    return !!(this.solutionView && this.solutionView.visible);
  }

  cmdToggleMode() {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return; }
    d.mode = d.mode === 'design' ? 'operate' : 'design';
    d.post({ type: 'setMode', mode: d.mode });
    this.updateStatus();
    if (d.mode === 'operate' && !this.warnedOperate) {
      this.warnedOperate = true;
      vscode.window.showInformationMessage('操作模式：點擊會交給頁面自己的程式處理（例如切分頁、展開選單）。網路仍然全部封鎖，不會送任何命令到機台。');
    }
  }

  async cmdShowSource() {
    const d = this.active;
    if (!d) return;
    await vscode.window.showTextDocument(d.doc, { viewColumn: this.sourceColumn(d), preview: false, preserveFocus: true });
  }

  cmdRevealSource(node) {
    const d = this.active;
    if (!d) return;
    if (node && node.key) {
      if (node.isForm) return this.revealSource(d, '@form', true);
      return this.revealSource(d, node.id, true);
    }
    if (d.sel && d.sel.info) return this.revealSource(d, d.sel.info.id, true);
  }

  /** Open the BCB6 .dfm at "object Name: TClass" (tree node, or the current selection). */
  async cmdRevealDfm(node) {
    const d = this.active;
    if (!d) return;
    const pi = await d.info;
    const id = node && node.key ? (node.isForm ? '@form' : node.id) : (d.sel && d.sel.info ? d.sel.info.id : null);
    if (!id) { vscode.window.showInformationMessage('先選一個元件。'); return; }
    const n = pi.ir ? (id === '@form' ? pi.ir.root : pi.ir.byName.get(id)) : null;
    const f = this.dfmFileOf(d, pi.ir);
    if (!n || !n.line || !f) {
      vscode.window.showInformationMessage(!pi.ir ? '這一頁沒有對應的 DFM。' : !f ? '找不到 BCB6 的 .dfm 檔（' + (pi.ir.sourceDfm || '?') + '）。' : 'DFM 裡沒有「' + id + '」（可能是網頁自己加的元件）。');
      return;
    }
    return this.openTarget({ kind: 'dfm', file: f, line: n.line, col: 1 }, d);
  }

  // --- WPF-style editing: the probe changed the DOM, now write it into the source ---
  refuseEdit(d, why) {
    vscode.window.setStatusBarMessage('$(warning) ' + why, 6000);
    this.log('[' + path.basename(d.file) + '] 沒有寫回：' + why);
  }

  /**
   * m = { what:'style'|'attr'|'caption', id, target:'self'|'parent', style:{…}, attr:{…},
   *       capKind, value }. One WorkspaceEdit = one undo step; nothing is saved to disk.
   */
  /** The characters one edit changes in `text`: { range:[s,e], repl } or { error }. */
  editRange(text, m) {
    const tag = m.id ? htmledit.startTagOf(text, m.id) : null;
    if (!tag) return { error: '原始碼裡找不到 id="' + (m.id || '') + '"（可能是頁面 JS 產生的元件），只改到畫面、沒有寫回。' };
    if (m.what === 'style' || m.what === 'attr') {
      // 'pnlCap' / 'legend': the child that carries a TPanel's / TGroupBox's caption font
      const target = m.target === 'parent' ? htmledit.wrapperTagOf(text, tag.start)
        : m.target === 'pnlCap' || m.target === 'legend' ? htmledit.captionHostTag(text, tag, m.target) : tag;
      if (!target && (m.target === 'pnlCap' || m.target === 'legend')) {
        return { error: '找不到「' + m.id + '」的標題元素（' + (m.target === 'legend' ? '<legend>' : '.pnlCap') + '）在原始碼的位置，沒有寫回。' };
      }
      if (!target) return { error: '「' + m.id + '」的位置由外層元素決定，但原始碼裡找不到緊鄰的外層標籤，沒有寫回。' };
      // (fn: the same change applied to a tag's text -- a batch may change one tag twice)
      const fn = t0 => {
        let t = t0;
        if (m.what === 'style') t = htmledit.setStyle(t, m.style || {});
        else for (const [k, v] of Object.entries(m.attr || {})) t = htmledit.setAttr(t, k, v);
        return t;
      };
      return { range: [target.start, target.end], repl: fn(target.text), fn };
    }
    if (m.what === 'class') {
      // (an IO lamp / panel button's LEDStyle / Value / Blink / Down / Style: its classes, changed in place)
      const fnc = t0 => htmledit.setClass(t0, m.add || [], m.remove || []);
      return { range: [tag.start, tag.end], repl: fnc(tag.text), fn: fnc };
    }
    if (m.what === 'caption') {
      if (m.capKind === 'value') {
        const fnv = t0 => htmledit.setAttr(t0, 'value', String(m.value));
        return { range: [tag.start, tag.end], repl: fnv(tag.text), fn: fnv };
      }
      const r = htmledit.captionRange(text, tag, m.capKind);
      if (!r) return { error: '找不到「' + m.id + '」的文字在原始碼的位置，沒有寫回。' };
      return { range: r, repl: htmledit.escText(m.value) };
    }
    return { error: '不認得的修改：' + m.what };
  }

  /** One edit, or a batch (group drag, align) as ONE WorkspaceEdit = one undo step. */
  async applySourceEdit(d, m) {
    const text = d.doc.getText();
    const list = m.what === 'batch' ? (m.edits || []) : [m];
    const all = [];
    for (const one of list) {
      const r = this.editRange(text, one);
      if (r.error) { this.refuseEdit(d, r.error); d.render(); return false; }
      // two changes of the same tag (改回 DFM: Left and Font.Bold of one button) = one edit of it
      const same = r.fn ? all.find(p => p.fn && p.range[0] === r.range[0] && p.range[1] === r.range[1]) : null;
      if (same) { same.repl = r.fn(same.repl); continue; }
      all.push(r);
    }
    const parts = all.filter(r => text.slice(r.range[0], r.range[1]) !== r.repl);
    if (!parts.length) return true;
    parts.sort((a, b) => a.range[0] - b.range[0]);
    for (let i = 1; i < parts.length; i++) {
      if (parts[i].range[0] < parts[i - 1].range[1]) { this.refuseEdit(d, '同一段原始碼被改了兩次（重疊），沒有寫回。'); d.render(); return false; }
    }
    const we = new vscode.WorkspaceEdit();
    for (const p of parts) we.replace(d.doc.uri, new vscode.Range(d.doc.positionAt(p.range[0]), d.doc.positionAt(p.range[1])), p.repl);
    d.selfEdits++;
    const ok = await vscode.workspace.applyEdit(we);
    if (!ok) { d.selfEdits--; this.refuseEdit(d, '寫回原始碼失敗。'); d.render(); return false; }
    this.noteFirstEdit(d);
    return true;
  }

  /** The first change of the session: say where it went, how to undo / save, and sync_web.py. */
  noteFirstEdit(d) {
    if (this.warnedEdit) return;
    this.warnedEdit = true;
    const auto = vscode.workspace.getConfiguration('files', d.doc.uri).get('autoSave');
    const autoNote = auto && auto !== 'off' ? '⚠ 你開了自動存檔（files.autoSave = ' + auto + '），改動會自動寫到磁碟。' : '還沒存檔；Ctrl+Z 復原、Ctrl+S 存檔。';
    vscode.window.showInformationMessage(
      '已改到 ' + path.basename(d.file) + ' 的 HTML 原始碼。' + autoNote +
      '這一頁由 sync_web.py 從網頁作者那邊同步，--apply 會整份覆蓋：存檔前請確認，並告訴網頁作者。');
  }

  // --- WPF Delete / Copy / Cut / Paste: whole elements; the page is drawn again from the source ---
  /**
   * The VCL class of a name: the DFM's, else the component tree's, else the source's
   * generated title="name : TClass" (a component added in this session is in neither yet).
   */
  classOf(d, pi, id, text) {
    if (id === '@form') return 'form';
    const n = pi && pi.ir ? pi.ir.byName.get(id) : null;
    if (n && n.class) return n.class;
    const row = d.treeData.find(r => r[2] === id);
    if (row && row[4]) return row[4];
    const tag = htmledit.startTagOf(text || d.doc.getText(), id);
    const t = tag ? /\stitle\s*=\s*["']\s*[\w@]+\s*:\s*(T\w+)/.exec(tag.text) : null;
    return t ? t[1] : '';
  }

  /** The names a structural command works on: the selection (+ the multi-selection), never the form. */
  selIds(d) {
    const info = d && d.sel && d.sel.info;
    if (!info) return [];
    return [info.id].concat(Array.isArray(info.multi) ? info.multi : []).filter(id => id && id !== '@form');
  }

  /** The unit of one name: the element, or the <span style="position:absolute"> around it (an input). */
  unitFor(text, id) {
    const tag = htmledit.startTagOf(text, id);
    if (!tag) return null;
    const w = htmledit.wrapperTagOf(text, tag.start);
    const wrap = !!(w && !/\sid\s*=/.test(w.text) && /position\s*:\s*absolute/i.test(w.text));
    const u = htmlblock.unitOf(text, id, wrap ? 'parent' : 'self');
    if (u) u.id = id;
    return u;
  }

  /** Units of these names; one inside another counts once. { units } or { error }. */
  unitsFor(text, ids) {
    const units = [];
    for (const id of ids) {
      const u = this.unitFor(text, id);
      if (!u) return { error: '原始碼裡找不到「' + id + '」（可能是頁面 JS 產生的元件，或標籤沒有正常結束），沒有動。' };
      units.push(u);
    }
    const outer = units.filter(u => !units.some(o => o !== u && o.start <= u.start && u.end <= o.end));
    outer.sort((a, b) => a.start - b.start);
    return { units: outer };
  }

  /** A structural change as ONE WorkspaceEdit (one undo step); the designer re-draws from it. */
  async applyStructural(d, parts) {
    parts.sort((a, b) => a.range[0] - b.range[0]);
    for (let i = 1; i < parts.length; i++) {
      if (parts[i].range[0] < parts[i - 1].range[1]) { this.refuseEdit(d, '同一段原始碼被改了兩次（重疊），沒有改。'); return false; }
    }
    const we = new vscode.WorkspaceEdit();
    for (const p of parts) we.replace(d.doc.uri, new vscode.Range(d.doc.positionAt(p.range[0]), d.doc.positionAt(p.range[1])), p.repl);
    // until the page is drawn again, a late 'select' from the old page must not change what
    // the new one selects (d.lastSelId, set by the caller: the new / renamed component)
    d.structuralPending = true;
    const ok = await vscode.workspace.applyEdit(we);
    if (!ok) { d.structuralPending = false; this.refuseEdit(d, '改原始碼失敗。'); return false; }
    this.noteFirstEdit(d);
    return true;
  }

  /** The page's JS that uses these names (getElementById('x'), '#x' ...): [{ id, hits }]. */
  async webUsesOf(d, names) {
    const pi = await d.info;
    const sources = this.webSources(d, pi);
    const out = [];
    for (const id of names) {
      const hits = web.findAll(sources, web.idMentionRe(id), 5);
      if (hits.length) out.push({ id, hits });
    }
    return out;
  }

  /** Del / Cut: remove the selected elements from the source (asks first when the page's JS uses them). */
  async cmdDelete(arg, verb) {
    const d = this.active;
    verb = verb || '刪除';
    if (!d) return null;
    const ids = this.selIds(d);
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選一個元件（表單本身不能' + verb + '）', 5000); return null; }
    if (this.refuseLocked(d, ids, verb)) return null;
    const text = d.doc.getText();
    const us = this.unitsFor(text, ids);
    if (us.error) { this.refuseEdit(d, us.error); return null; }
    const names = [].concat(...us.units.map(u => htmlblock.idsIn(u.html)));
    const uses = await this.webUsesOf(d, names);
    if (uses.length && !(arg && arg.confirmed)) {
      const list = uses.slice(0, 6).map(u => u.id + '（' + u.hits.map(h => path.basename(h.file) + ':' + h.line).join('、') + '）').join('\n');
      const pick = await vscode.window.showWarningMessage(
        '網頁程式有用到要' + verb + '的元件，' + verb + '之後那些程式會找不到它：\n' + list + (uses.length > 6 ? '\n…共 ' + uses.length + ' 個' : '') +
        '\n\n確定要' + verb + '嗎？（Ctrl+Z 可以復原）', { modal: true }, verb);
      if (pick !== verb) return null;
    }
    // afterwards the parent is selected (WPF)
    const row = d.treeData.find(r => r[2] === ids[0]);
    const prow = row ? d.treeData.find(r => r[0] === row[1]) : null;
    d.lastSelId = prow ? prow[2] : null;
    const parts = us.units.map(u => { const r = htmlblock.removeRange(text, u.start, u.end); return { range: r.range, repl: '' }; });
    const ok = await this.applyStructural(d, parts);
    if (ok) vscode.window.setStatusBarMessage('$(trash) 已' + verb + ' ' + us.units.map(u => u.id).join('、') + '（Ctrl+Z 復原）', 6000);
    return ok ? { removed: us.units.map(u => u.id), uses: uses.map(u => u.id) } : null;
  }

  /**
   * WPF Order: Bring to Front / Bring Forward / Send Backward / Send to Back = the
   * element's place among its siblings in the source (later = drawn on top); the
   * page is drawn again and the component tree follows. how: front|forward|backward|back
   */
  async cmdOrder(how) {
    const d = this.active;
    if (!d) return null;
    const ids = this.selIds(d);
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選一個元件（表單本身沒有前後順序）', 5000); return null; }
    if (ids.length > 1) return this.cmdOrderMany(d, ids, how);
    if (this.refuseLocked(d, [ids[0]], '調整順序')) return null;
    const text = d.doc.getText();
    const u = this.unitFor(text, ids[0]);
    if (!u) { this.refuseEdit(d, '原始碼裡找不到「' + ids[0] + '」（可能是頁面 JS 產生的元件），沒有動。'); return null; }
    const r = htmlblock.reorder(text, u, how);
    const WORD = { front: '最上層', forward: '上一層', backward: '下一層', back: '最下層' };
    if (r.error) { this.refuseEdit(d, '「' + ids[0] + '」不能調整順序：' + r.error); return null; }
    if (r.same) { vscode.window.setStatusBarMessage('$(info) ' + ids[0] + ' 已經在' + (how === 'front' || how === 'forward' ? '最上層' : '最下層'), 4000); return { same: true }; }
    d.lastSelId = ids[0];
    const ok = await this.applyStructural(d, r.parts);
    if (ok) {
      vscode.window.setStatusBarMessage('$(layers) ' + ids[0] + ' 移到' + WORD[how] + '（同一層第 ' + (r.to + 1) + ' / ' + r.of + ' 個；Ctrl+Z 復原）' +
        (ids.length > 1 ? '　多選時只移動主要選取' : ''), 6000);
    }
    return ok ? { id: ids[0], from: r.from, to: r.to, of: r.of } : null;
  }

  /**
   * AI(W906-HTDESIGNER) 20261001: Order with several selected (WPF / Blend "Bring the selected object to the front" --
   * the selection, not only its primary one, as it used to be): each among its own siblings, their order among
   * themselves kept (front / backward: the earliest first; back / forward: the latest first), ONE edit.
   */
  async cmdOrderMany(d, ids, how) {
    if (this.refuseLocked(d, ids, '調整順序')) return null;
    const orig = d.doc.getText();
    let text = orig;
    const startOf = id => { const u = this.unitFor(text, id); return u ? u.start : -1; };
    const missing = ids.filter(id => startOf(id) < 0);
    const asc = ids.filter(id => startOf(id) >= 0).sort((a, b) => startOf(a) - startOf(b));
    const seq = how === 'front' || how === 'backward' ? asc : asc.slice().reverse();
    const moved = [], skipped = [], blocked = new Set();
    const idOfKid = k => (/\bid\s*=\s*["']([^"']+)["']/.exec(k.text.slice(0, k.text.indexOf('>') + 1)) || [])[1] || '';
    for (const id of seq) {
      const u = this.unitFor(text, id);
      let r = u ? htmlblock.reorder(text, u, how) : { error: '找不到' };
      // one step (forward / backward) never jumps over another selected one that could not move: the order kept
      if (!r.error && !r.same && (how === 'forward' || how === 'backward')) {
        const kids = htmlblock.childrenOf(text, htmlblock.parentOf(text, u.start));
        if (kids[r.to] && blocked.has(idOfKid(kids[r.to]))) r = { same: true };
      }
      if (r.error || r.same) { blocked.add(id); skipped.push(id + (r.error ? '（' + r.error + '）' : '')); continue; }
      for (const p of r.parts.slice().sort((a, b) => b.range[0] - a.range[0])) text = text.slice(0, p.range[0]) + p.repl + text.slice(p.range[1]);
      moved.push(id);
    }
    const WORD = { front: '最上層', forward: '上一層', backward: '下一層', back: '最下層' };
    if (text === orig) {
      vscode.window.setStatusBarMessage('$(info) 選取的 ' + ids.length + ' 個都已經在' + (how === 'front' || how === 'forward' ? '最上層' : '最下層') + (missing.length ? '（原始碼裡找不到：' + missing.join('、') + '）' : ''), 5000);
      return { same: true, moved: [], skipped, missing };
    }
    // ONE replacement: the part of the source that changed (one Ctrl+Z)
    let a = 0;
    while (a < orig.length && a < text.length && orig[a] === text[a]) a++;
    let b = 0;
    while (b < orig.length - a && b < text.length - a && orig[orig.length - 1 - b] === text[text.length - 1 - b]) b++;
    d.lastSelId = ids[0];
    const ok = await this.applyStructural(d, [{ range: [a, orig.length - b], repl: text.slice(a, text.length - b) }]);
    if (ok) vscode.window.setStatusBarMessage('$(layers) 選取的 ' + moved.length + ' 個移到' + WORD[how] + '（各自在自己的那一層，彼此的前後不變；Ctrl+Z 復原）' +
      (skipped.length ? '　已經在那裡／不能移：' + skipped.join('、') : ''), 6000);
    return ok ? { moved, skipped, missing } : null;
  }

  /**
   * 選取這裡的元件… (Blend "Set Current Selection": "displays all objects in the hierarchy, starting with the object
   * that was right-clicked"; Blend 2012: "a list of all objects, in Z order ... under the location that you clicked"):
   * the components under the last right-click, the top one first, the form last -- one picked = selected.
   */
  async cmdSelectHere() {
    const d = this.active;
    if (!d) return null;
    const rep = await d.request({ type: 'stackAt' }, 3000);
    const items = rep && Array.isArray(rep.items) ? rep.items : [];
    if (!items.length) { vscode.window.setStatusBarMessage('$(info) 先在設計畫面上按右鍵（那一點底下的元件會列出來）', 5000); return null; }
    const pick = await vscode.window.showQuickPick(items.map((x, i) => ({
      label: (i ? '' : '$(arrow-right) ') + (x.id === '@form' ? '表單' : x.id), description: x.cls || x.tag || '', detail: x.sel ? '（目前選取的）' : undefined, x,
    })), { placeHolder: '選取這裡的元件（最上面的在前面）' });
    if (!pick) return null;
    d.post({ type: 'selectId', id: pick.x.id, origin: 'editor' });
    return { id: pick.x.id, of: items.map(x => x.id) };
  }

  /**
   * WPF document outline drag and drop: `ids` onto `target` (a component tree node;
   * null = the empty area = the form). Onto a container (TPanel / TGroupBox /
   * TTabSheet / the form) they move into it -- at its end, so on top; onto any other
   * component they move next to it, into its parent. Their left/top numbers stay
   * (they now count from the new container). One WorkspaceEdit, the page re-drawn.
   */
  async cmdReparent(ids, target, pos) {
    const d = this.active;
    if (!d) return null;
    ids = ids.filter(id => id && id !== '@form');
    if (!ids.length) return null;
    const tId = !target ? '@form' : target.isForm ? '@form' : target.id;
    if (!tId) return null;
    if (this.refuseLocked(d, ids, '移動')) return null;
    // never into itself or into something inside it
    const rowOf = id => d.treeData.find(r => (id === '@form' ? r[7] : r[2] === id));
    for (let row = rowOf(tId); row; row = row[1] ? d.treeData.find(r => r[0] === row[1]) : null) {
      if (!row[7] && ids.includes(row[2])) { this.refuseEdit(d, '不能把「' + row[2] + '」放進它自己裡面。'); return null; }
    }
    const pi = await d.info;
    const container = tId === '@form' || toolbox.CONTAINERS.test(this.classOf(d, pi, tId));
    const text = d.doc.getText();
    const us = this.unitsFor(text, ids);
    if (us.error) { this.refuseEdit(d, us.error); return null; }
    let at, lineOf;
    if (container) {
      // (AI 20261001, 0.136: into a tab sheet by its .dfm name -- WPF / Blend reparent into a TabItem; the body of a page
      // without a form root)
      const tag = htmledit.containerTagOf(text, tId);
      at = tag ? htmlblock.insideEnd(text, tag) : -1;
      lineOf = at;
    } else {
      const tu = this.unitFor(text, tId);
      at = tu ? tu.end : -1;
      lineOf = tu ? tu.start : -1;
    }
    if (at < 0) { this.refuseEdit(d, '找不到「' + tId + '」在原始碼的位置（或沒有正常結束），沒有移動。'); return null; }
    if (us.units.some(u => u.start < at && at < u.end)) { this.refuseEdit(d, '不能把元件放進它自己裡面。'); return null; }
    const parts = us.units.map(u => ({ range: htmlblock.removeRange(text, u.start, u.end).range, repl: '' }));
    // pos (a drop on the design surface): where each one goes in the new container, { id: { left, top } }
    const htmlOf = u => {
      const p = pos && pos[u.id], at0 = p ? htmlblock.firstPx(u.html) : null;
      return p && at0 && typeof at0.left === 'number' && typeof at0.top === 'number'
        ? htmlblock.offsetBlock(u.html, Math.round(p.left) - at0.left, Math.round(p.top) - at0.top) : u.html;
    };
    const ls = text.lastIndexOf('\n', at - 1) + 1;
    let repl;
    if (container && !text.slice(ls, at).trim()) {
      const ind = htmlblock.indentAt(text, at) + '  ';
      repl = us.units.map(u => ind + htmlOf(u) + '\n').join('');
      at = ls;
    } else {
      const ind = htmlblock.indentAt(text, lineOf) + (container ? '  ' : '');
      repl = us.units.map(u => '\n' + ind + htmlOf(u)).join('');
    }
    // the insertion must not fall inside what is taken out
    if (parts.some(p => p.range[0] < at && at < p.range[1])) { this.refuseEdit(d, '放的位置在要移走的那一段裡面，沒有移動。'); return null; }
    parts.push({ range: [at, at], repl });
    d.lastSelId = ids[0];
    const ok = await this.applyStructural(d, parts);
    if (ok) {
      vscode.window.setStatusBarMessage('$(move) ' + us.units.map(u => u.id).join('、') + (container ? ' 移進 ' + (tId === '@form' ? '表單' : tId) : ' 移到 ' + tId + ' 旁邊') +
        (pos ? '（放在放開的地方；Ctrl+Z 復原）' : '（left/top 數值沒變，現在從新的上層算起；Ctrl+Z 復原）'), 7000);
    }
    return ok ? { moved: us.units.map(u => u.id), into: container ? tId : null, after: container ? null : tId } : null;
  }

  /**
   * Blend: "Reparent an object: drag the object over a layout panel and press Alt" -- the design surface let the
   * selection go with Alt held: targets = the id'd elements under the pointer (innermost first, the form last, each
   * with the pointer in its own child coordinates), offs = where each one's top-left is from the pointer. The first
   * container takes them, at the place they were let go; the container they are already in = just that move.
   */
  async cmdReparentDrop(d, m) {
    if (this.active !== d) this.setActive(d);
    const offs = Array.isArray(m && m.offs) ? m.offs.filter(o => o && typeof o.id === 'string' && o.id && o.id !== '@form') : [];
    const ids = offs.map(o => o.id);
    if (!ids.length || !Array.isArray(m.targets)) return null;
    const pi = await d.info;
    const text = d.doc.getText();
    const t = m.targets.find(g => g && !(typeof g.id === 'string' && ids.includes(g.id)) &&
      (g.pane || g.id === '@form' || (typeof g.id === 'string' && toolbox.CONTAINERS.test(this.classOf(d, pi, g.id, text))))) || null;
    if (!t) { this.refuseEdit(d, '放開的地方沒有可以放進去的容器（Panel、GroupBox 或表單），沒有換。'); return null; }
    // (0.136: a sheet the page names (its .dfm name) takes them like a Panel; only an unnamed one is refused)
    if (t.pane && !t.id) { this.refuseEdit(d, '分頁（TabSheet）在 HTML 裡沒有名稱，不能用拖的換進去：在元件樹把它拖到那個分頁裡的元件上。'); return null; }
    const pos = {};
    for (const o of offs) pos[o.id] = { left: Math.round((+t.x || 0) + (+o.dx || 0)), top: Math.round((+t.y || 0) + (+o.dy || 0)) };
    const parentOf = id => {
      const row = d.treeData.find(r => r[2] === id);
      const pr = row && row[1] != null ? d.treeData.find(r => r[0] === row[1]) : null;
      return pr ? (pr[7] ? '@form' : pr[2]) : null;
    };
    if (ids.every(id => parentOf(id) === t.id)) {
      // already in it: only the move (ONE batch edit, from the source's positions)
      d.post({ type: 'editMany', items: ids.map(id => ({ id, type: 'setLayout', left: pos[id].left, top: pos[id].top, force: true })) });
      return { same: true, into: t.id, pos };
    }
    const r = await this.cmdReparent(ids, t.id === '@form' ? null : { id: t.id }, pos);
    return r ? Object.assign({ pos }, r) : null;
  }

  /**
   * WPF / Blend "Group Into": the selected components (all in one parent) get a new
   * TPanel around them -- no border, no background, exactly their bounding box -- and
   * count from it: on the screen nothing moves. One WorkspaceEdit, the panel selected.
   */
  async cmdGroup() {
    const d = this.active;
    if (!d) return null;
    const ids = this.selIds(d);
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選要放進新 Panel 的元件（可以多選）', 5000); return null; }
    if (this.refuseLocked(d, ids, '放進新 Panel')) return null;
    const text = d.doc.getText();
    const us = this.unitsFor(text, ids);
    if (us.error) { this.refuseEdit(d, us.error); return null; }
    const parents = us.units.map(u => htmlblock.parentOf(text, u.start));
    if (!parents.every(p => p && parents[0] && p.tag.start === parents[0].tag.start)) {
      this.refuseEdit(d, '要放進同一個新 Panel 的元件必須在同一層（同一個上層容器裡）。'); return null;
    }
    // drawn sizes (a label without a width is as wide as its text): the probe measures
    const reply = await d.request({ type: 'lookAll', ids: us.units.map(u => u.id) }, 5000);
    const lay = new Map(((reply && reply.items) || []).map(it => [it.id, it.lay]));
    let minL = Infinity, minT = Infinity, maxR = -Infinity, maxB = -Infinity;
    for (const u of us.units) {
      const p = htmlblock.firstPx(u.html);
      const l = lay.get(u.id);
      const w = p && p.width !== null ? p.width : l && typeof l.width === 'number' ? l.width : null;
      const h = p && p.height !== null ? p.height : l && typeof l.height === 'number' ? l.height : null;
      if (!p || p.left === null || p.top === null || w === null || h === null) {
        this.refuseEdit(d, '「' + u.id + '」的位置不是寫在自己 style 裡的 px 數值（或量不到大小），不能放進新 Panel。'); return null;
      }
      minL = Math.min(minL, p.left); minT = Math.min(minT, p.top);
      maxR = Math.max(maxR, p.left + w); maxB = Math.max(maxB, p.top + h);
    }
    const used = new Set(pageinfo.collectIds(text).concat(d.treeData.map(r => r[2]).filter(Boolean)));
    const pid = toolbox.newName('Panel', used);
    const inner = us.units.map(u => htmlblock.offsetBlock(u.html, -minL, -minT)).join('');
    const html = '<div class="pnl" id="' + pid + '" style="position:absolute;left:' + minL + 'px;top:' + minT + 'px;width:' + Math.round(maxR - minL) + 'px;height:' +
      Math.round(maxB - minT) + 'px;" title="' + pid + ' : TPanel"><span class="pnlCap" style="font-size:11px;"></span>' + inner + '</div>';
    // the panel takes the first one's place in the source; the others go
    const first = us.units[0];
    const parts = [{ range: [first.start, first.end], repl: html }].concat(
      us.units.slice(1).map(u => ({ range: htmlblock.removeRange(text, u.start, u.end).range, repl: '' })));
    d.lastSelId = pid;
    const ok = await this.applyStructural(d, parts);
    if (ok) vscode.window.setStatusBarMessage('$(group-by-ref-type) ' + us.units.map(u => u.id).join('、') + ' 放進新的 ' + pid + '（畫面上沒動；Ctrl+Z 復原）', 6000);
    return ok ? { panel: pid, moved: us.units.map(u => u.id), box: { left: minL, top: minT, width: Math.round(maxR - minL), height: Math.round(maxB - minT) } } : null;
  }

  /**
   * Blend "Ungroup": the selected TPanel / TGroupBox's children move out into its
   * parent (their left/top plus the container's place and border: on the screen
   * nothing moves) and the container goes. One WorkspaceEdit.
   */
  async cmdUngroup() {
    const d = this.active;
    if (!d) return null;
    const id = this.selIds(d)[0];
    const pi = await d.info;
    const cls = id ? this.classOf(d, pi, id) : '';
    if (!id || !/^(TPanel|TGroupBox)$/.test(cls)) { vscode.window.setStatusBarMessage('$(info) 先選一個 Panel 或 GroupBox（它裡面的元件會移到外面）', 5000); return null; }
    if (this.refuseLocked(d, [id], '解除群組')) return null;
    const text = d.doc.getText();
    const u = this.unitFor(text, id);
    const tag = htmledit.startTagOf(text, id);
    if (!u || !tag) { this.refuseEdit(d, '原始碼裡找不到「' + id + '」，沒有動。'); return null; }
    // the children live in it, or in its <div class="cli">
    let holder = { tag, range: [u.start, u.end] };
    const cliAt = htmlblock.insideEnd(text, tag);
    const cli = htmlblock.parentOf(text, cliAt);
    if (cli && cli.tag.start !== tag.start && cli.tag.start > tag.start && /\bclass\s*=\s*["'][^"']*\bcli\b/.test(cli.tag.text)) holder = cli;
    const kids = htmlblock.childrenOf(text, holder).filter(k => k.name !== 'legend' && !/\bclass\s*=\s*["'][^"']*\bpnlCap\b/.test(k.text) &&
      !/\bclass\s*=\s*["'][^"']*\bcli\b/.test(k.text));
    if (!kids.length) { vscode.window.setStatusBarMessage('$(info) ' + id + ' 裡面沒有元件', 5000); return null; }
    const own = htmlblock.firstPx(u.html);
    if (!own || own.left === null || own.top === null) { this.refuseEdit(d, '「' + id + '」的位置不是 px 數值，不能解除群組。'); return null; }
    const reply = await d.request({ type: 'lookAll', ids: [id] }, 5000);
    const l = reply && reply.items && reply.items[0] ? reply.items[0].lay : null;
    const cl = l && typeof l.cl === 'number' ? l.cl : 0, ct = l && typeof l.ct === 'number' ? l.ct : 0;
    const blocks = kids.map(k => text.slice(k.start, k.end));
    const bad = blocks.find(b => { const p = htmlblock.firstPx(b); return !p || p.left === null || p.top === null; });
    if (bad) { this.refuseEdit(d, '裡面有元件的位置不是 px 數值，不能解除群組。'); return null; }
    const out = blocks.map(b => htmlblock.offsetBlock(b, own.left + cl, own.top + ct));
    const ind = htmlblock.indentAt(text, u.start);
    const repl = out.join('\n' + ind);
    const firstIds = htmlblock.idsIn(blocks[0]);
    d.lastSelId = firstIds[0] || null;
    const ok = await this.applyStructural(d, [{ range: [u.start, u.end], repl }]);
    if (ok) vscode.window.setStatusBarMessage('$(ungroup-by-ref-type) ' + id + ' 解除群組：' + blocks.length + ' 個元件移到外面（畫面上沒動；Ctrl+Z 復原）', 6000);
    return ok ? { removed: id, out: [].concat(...blocks.map(b => htmlblock.idsIn(b).slice(0, 1))), shift: { x: own.left + cl, y: own.top + ct } } : null;
  }

  /**
   * 工具箱 (WPF Toolbox): a new component of class `cls` (the generator's markup, a
   * BCB6-style name Label1 ...): into the selected container (TPanel / TGroupBox /
   * TTabSheet / the form) at 8,8 -- moved on while that place is taken --, else just
   * below the selected component, next to it in the source (same tab sheet / group).
   */
  async cmdAdd(cls, place) {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    const item = toolbox.itemOf(cls);
    if (!item) return null;
    const text = d.doc.getText();
    const pi = await d.info;
    const info = d.sel && d.sel.info;
    // (place: from the design surface -- { into, x, y, w, h }, the container and the point it was put at)
    const selId = place ? place.into : info && info.id ? info.id : '@form';
    const container = !!place || selId === '@form' || toolbox.CONTAINERS.test(this.classOf(d, pi, selId, text));
    const used = new Set(pageinfo.collectIds(text).concat(d.treeData.map(r => r[2]).filter(Boolean)));
    const id = toolbox.newName(item.base, used);
    let at, x = 8, y = 8, lineOf, into = null, atEnd = false;
    if (container) {
      // (AI 20261001, 0.135: a tab sheet by its .dfm name, and '@form' on a page without a form root = its body)
      const tag = htmledit.containerTagOf(text, selId);
      at = tag ? htmlblock.insideEnd(text, tag) : -1;
      lineOf = at;
      into = selId;
      if (place) { x = Math.max(0, Math.round(place.x)); y = Math.max(0, Math.round(place.y)); }
      // 8,8 unless a child already stands there: then 16 px further down-right
      const inner = tag && at > 0 ? text.slice(tag.end, at) : '';
      for (let k = 0; !place && k < 20 && new RegExp('left:' + x + 'px;top:' + y + 'px').test(inner); k++) { x += 16; y += 16; }
    } else {
      const u = this.unitFor(text, selId);
      // WPF: "the element is automatically placed in front of other elements in the active container element" -- the
      // end of the selected one's container (AI 20261001: it went right after the selected one, under its later siblings);
      // where: under the selected one, as before
      const par = u ? htmlblock.parentOf(text, u.start) : null;
      const pEnd = par && par.tag ? htmlblock.insideEnd(text, par.tag) : -1;
      atEnd = !!(u && pEnd > u.end);
      at = atEnd ? pEnd : (u ? u.end : -1);
      lineOf = u ? u.start : -1;
      const lay = info && info.layout;
      if (lay && typeof lay.left === 'number' && typeof lay.top === 'number') { x = Math.round(lay.left); y = Math.round(lay.top + (lay.height || 0) + 8); }
    }
    if (at < 0) { this.refuseEdit(d, '找不到要放到哪裡（「' + selId + '」在原始碼裡找不到或沒有正常結束），沒有新增。'); return null; }
    const w = place && place.w >= 4 ? Math.round(place.w) : 0, h = place && place.h >= 4 ? Math.round(place.h) : 0;
    const html = item.html(id, x, y, w, h);
    let repl;
    const ls = text.lastIndexOf('\n', at - 1) + 1;
    if ((container || atEnd) && !text.slice(ls, at).trim()) {
      // (on its own line before the container's end tag: indented as the selected one, a sibling)
      repl = (container ? htmlblock.indentAt(text, at) + '  ' : htmlblock.indentAt(text, lineOf)) + html + '\n';
      at = ls;
    } else {
      repl = '\n' + htmlblock.indentAt(text, lineOf) + (container ? '  ' : '') + html;
    }
    d.lastSelId = id;
    const ok = await this.applyStructural(d, [{ range: [at, at], repl }]);
    if (ok) vscode.window.setStatusBarMessage('$(add) 新增了 ' + id + ' : ' + item.cls + (into ? '（在 ' + into + ' 裡）' : '（在 ' + selId + ' 下面）') + '（Ctrl+Z 復原）', 6000);
    return ok ? { id, cls: item.cls, into, after: into ? null : selId, x, y, w, h } : null;
  }

  /**
   * 改名稱 (WPF's Name field, F2): the selected component's id on the page -- its start tag,
   * the generated title, <label for> -- and, like BCB6, its caption when that still is the
   * old name. A name the .dfm has, or one the page's JS uses, is asked about first: those
   * links are not renamed (the DFM, the C++ handlers, the web code stay as they are).
   *   arg: the new name (tests / other code; else an input box), confirmed: skip the question
   */
  async cmdRename(arg, confirmed) {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    const info = d.sel && d.sel.info;
    const id = info && info.id;
    if (!id || id === '@form') { vscode.window.setStatusBarMessage('$(info) 先選一個元件（表單本身的名稱不在這裡改）', 5000); return null; }
    const text = d.doc.getText();
    const used = new Set(pageinfo.collectIds(text).concat(d.treeData.map(r => r[2]).filter(Boolean)));
    const bad = v => {
      const s = String(v || '').trim();
      if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(s)) return '名稱只能用英文字母、數字和 _，而且不能用數字開頭（BCB6 的規則）';
      if (s !== id && used.has(s)) return '這一頁已經有「' + s + '」了';
      return null;
    };
    let nu = typeof arg === 'string' ? arg.trim() : await vscode.window.showInputBox({
      prompt: '「' + id + '」的新名稱（id）', value: id, validateInput: v => bad(v),
    });
    if (nu == null) return null;
    nu = String(nu).trim();
    if (nu === id) return null;
    const why = bad(nu);
    if (why) { this.refuseEdit(d, why + '，沒有改。'); return null; }
    const pi = await d.info;
    const inDfm = !!(pi.ir && pi.ir.byName.has(id));
    const uses = await this.webUsesOf(d, [id]);
    const nUse = uses.length ? uses[0].hits.length : 0;
    if ((inDfm || nUse) && confirmed !== true) {
      const msg = '把「' + id + '」改成「' + nu + '」？\n\n' +
        (inDfm ? '・BCB6 的 .dfm 裡有「' + id + '」：改名後這個元件就對不到 DFM（與 DFM 的差異、事件、C++ 程式都對不起來）。\n' : '') +
        (nUse ? '・網頁程式提到「' + id + '」' + (nUse >= 5 ? ' 5 次以上' : ' ' + nUse + ' 次') + '（例如 ' + uses[0].hits[0].file.split(/[\\/]/).pop() + ':' + uses[0].hits[0].line + '）：那些不會跟著改。\n' : '') +
        '\n只改這一頁 HTML 裡的名稱，Ctrl+Z 可以復原。';
      const pick = await vscode.window.showWarningMessage(msg, { modal: true }, '改名稱');
      if (pick !== '改名稱') return null;
    }
    const edits = htmlblock.renameEdits(text, id, nu, { caption: true });
    if (!edits) { this.refuseEdit(d, '原始碼裡找不到「' + id + '」（可能是頁面 JS 產生的元件），沒有改。'); return null; }
    // the page's designer-kept event block (網頁事件): its getElementById('old') follow
    for (const x of jsevents.renameIdEdits(text, id, nu)) edits.push({ range: [x.s, x.e], repl: x.text });
    d.lastSelId = nu;
    const ok = await this.applyStructural(d, edits.map(e => ({ range: e.range, repl: e.repl })));
    if (ok) {
      // the designer's own marks (the eye, the lock) follow the name
      if (d.designHidden && d.designHidden.has(id)) this.applyDesignHidden(d, Array.from(d.designHidden).map(x => (x === id ? nu : x)));
      if (d.designLocked && d.designLocked.has(id)) this.applyDesignLocked(d, Array.from(d.designLocked).map(x => (x === id ? nu : x)));
      vscode.window.setStatusBarMessage('$(edit) 「' + id + '」改名為「' + nu + '」（Ctrl+Z 復原）', 6000);
    }
    return ok ? { from: id, to: nu, edits: edits.length, inDfm, webUses: nUse } : null;
  }

  /**
   * WinForms / WPF: a click on a toolbox item picks the tool (the design surface then shows a
   * crosshair: a click puts it there, a drag also gives its size, Esc gives up); a second
   * click on the same item within a moment ("double-click") adds it at once, as before.
   */
  toolboxArm(cls) {
    // (Enter in the toolbox adds it -- the select that Enter makes is not a pick-up)
    if (this.toolboxEnter && Date.now() - this.toolboxEnter < 500) return null;
    if (cls === POINTER.cls) return this.toolboxPointer();
    const item = toolbox.itemOf(cls);
    const d = this.active;
    if (!item) return null;
    const now = Date.now();
    const dbl = this.lastArm && this.lastArm.cls === cls && now - this.lastArm.t < 450;
    this.lastArm = dbl ? null : { cls, t: now };
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    if (dbl) { d.post({ type: 'placeArm', cls: null }); this.toolboxBackToPointer(); return this.cmdAdd(cls); }
    d.armed = cls;
    d.post({ type: 'placeArm', cls, label: item.label });
    d.panel.reveal(d.panel.viewColumn, false);
    vscode.window.setStatusBarMessage('$(add) 放置 ' + item.cls + '：在設計畫面上點一下放在那裡（拖一個框＝連大小），Esc 取消；連點兩下工具箱＝直接加', 8000);
    return { armed: cls };
  }

  /** WPF Toolbox "Pointer": the picked-up tool is put back; the surface selects again. */
  toolboxPointer() {
    this.lastArm = null;
    const d = this.active;
    const had = !!(d && d.armed);
    if (d) { d.armed = null; d.post({ type: 'placeArm', cls: null }); }
    vscode.window.setStatusBarMessage('$(inspect) 指標：' + (had ? '放下拿起的工具，' : '') + '設計畫面回到選取', 5000);
    return { pointer: true, had };
  }

  /**
   * WPF: once a tool is put down (or given up), the Toolbox's selection is Pointer again. Only when
   * the toolbox is showing (a reveal would open it) -- its selection is not a click, nothing runs.
   */
  toolboxBackToPointer() {
    const tv = this.toolboxView;
    if (!tv || !tv.visible) return;
    try { Promise.resolve(tv.reveal(POINTER, { select: true, focus: false })).catch(() => {}); } catch (e) { /* not showing */ }
  }

  /**
   * The design surface put the armed tool down: targets = the id'd elements under the point,
   * innermost first, each with the point in its own child coordinates ('@form' last). The
   * first container (Panel / GroupBox / tab sheet ... or the form) takes it.
   */
  async cmdPlace(d, m) {
    const item = toolbox.itemOf(m && m.cls);
    d.armed = null;
    this.toolboxBackToPointer();
    // cmdAdd works on the active designer: the one it was put down on
    if (this.active !== d) this.setActive(d);
    if (!item || !Array.isArray(m.targets) || !m.targets.length) return null;
    const pi = await d.info;
    const text = d.doc.getText();
    const t = m.targets.find(g => g && (g.pane || g.id === '@form' || (typeof g.id === 'string' && toolbox.CONTAINERS.test(this.classOf(d, pi, g.id, text))))) || null;
    if (!t) { this.refuseEdit(d, '這裡沒有可以放元件的容器（Panel、GroupBox 或表單），沒有新增。'); return null; }
    // a generated tab sheet has no id: by its .dfm name (0.135; WPF puts a control dropped on a TabItem into it);
    // a sheet the page does not name (no TTabSheet title) still cannot be told apart
    if (t.pane && !t.id) { this.refuseEdit(d, '分頁（TabSheet）裡不能用點的放：先選那個分頁上的一個元件，再連點兩下工具箱（新的會放在它下面、同一個分頁裡）。'); return null; }
    return this.cmdAdd(item.cls, { into: t.id, x: +t.x || 0, y: +t.y || 0, w: +m.w || 0, h: +m.h || 0 });
  }

  /** The toolbox as a quick pick (the command palette / the design surface's menu). */
  async cmdAddPick() {
    const pick = await vscode.window.showQuickPick(toolbox.ITEMS.map(i => ({ label: i.label, description: i.cls + '　' + i.note, cls: i.cls })),
      { placeHolder: '新增哪一種元件？（放進選取的容器，或放在選取元件的下面）' });
    return pick ? this.cmdAdd(pick.cls) : null;
  }

  /** Ctrl+C: the selected elements' HTML, for Paste (also on the system clipboard as text). */
  async cmdCopy() {
    const d = this.active;
    if (!d) return null;
    const ids = this.selIds(d);
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選一個元件（表單本身不能複製）', 5000); return null; }
    const us = this.unitsFor(d.doc.getText(), ids);
    if (us.error) { this.refuseEdit(d, us.error); return null; }
    this.clip = { blocks: us.units.map(u => u.html), ids: us.units.map(u => u.id), file: d.file };
    try { await vscode.env.clipboard.writeText(this.clip.blocks.join('\n')); } catch (e) { /* the internal copy is enough */ }
    vscode.window.setStatusBarMessage('$(copy) 已複製 ' + this.clip.ids.join('、') + '，Ctrl+V 貼上', 5000);
    return this.clip;
  }

  /** Ctrl+X: copy, then delete (asks like Delete when the page's JS uses them). */
  async cmdCut(arg) {
    if (this.active && this.refuseLocked(this.active, this.selIds(this.active), '剪下')) return null;
    const c = await this.cmdCopy();
    if (!c) return null;
    return this.cmdDelete(arg, '剪下');
  }

  /**
   * Ctrl+V: the copied elements as new ones -- every id made new (spbSave -> spbSave_2),
   * into the selected container (TPanel / TGroupBox / TTabSheet / the form), else next
   * to the selected element; 8 px down-right when pasted on the page they came from.
   */
  async cmdPaste() {
    const d = this.active;
    if (!d) return null;
    if (!this.clip || !this.clip.blocks.length) { vscode.window.setStatusBarMessage('$(info) 還沒有複製元件（先在設計檢視選元件按 Ctrl+C）', 5000); return null; }
    const text = d.doc.getText();
    const pi = await d.info;
    const info = d.sel && d.sel.info;
    const selId = info && info.id ? info.id : '@form';
    const container = selId === '@form' || toolbox.CONTAINERS.test(this.classOf(d, pi, selId, text));
    let at;
    let lineOf;
    if (container) {
      // (AI 20261001, 0.135: a tab sheet by its .dfm name, and '@form' on a page without a form root = its body)
      const tag = htmledit.containerTagOf(text, selId);
      at = tag ? htmlblock.insideEnd(text, tag) : -1;
      lineOf = at;
    } else {
      const u = this.unitFor(text, selId);
      at = u ? u.end : -1;
      lineOf = u ? u.start : -1;
    }
    if (at < 0) { this.refuseEdit(d, '找不到要貼到哪裡（「' + selId + '」在原始碼裡找不到或沒有正常結束），沒有貼上。'); return null; }
    const used = new Set(pageinfo.collectIds(text).concat(d.treeData.map(r => r[2]).filter(Boolean)));
    const same = web.samePath(this.clip.file, d.file);
    const made = [];
    const blocks = this.clip.blocks.map(b => {
      const r = htmlblock.renameIds(b, used);
      made.push(...r.map.values());
      return same ? htmlblock.offsetBlock(r.html, 8, 8) : r.html;
    });
    let repl;
    const ls = text.lastIndexOf('\n', at - 1) + 1;
    if (container && !text.slice(ls, at).trim()) {
      // the container's end tag starts its line: whole lines, just before it
      const ind = htmlblock.indentAt(text, at) + '  ';
      repl = blocks.map(b => ind + b + '\n').join('');
      at = ls;
    } else {
      // "\n" + indent + block: Delete takes the same break away again (htmlblock.removeRange)
      const ind = htmlblock.indentAt(text, lineOf) + (container ? '  ' : '');
      repl = blocks.map(b => '\n' + ind + b).join('');
    }
    // afterwards the first new element is selected
    d.lastSelId = made[0] || d.lastSelId;
    const ok = await this.applyStructural(d, [{ range: [at, at], repl }]);
    if (ok) vscode.window.setStatusBarMessage('$(clippy) 已貼上 ' + made.join('、') + (container ? '（在 ' + selId + ' 裡）' : '') + '（Ctrl+Z 復原）', 6000);
    return ok ? { made, into: container ? selId : null, after: container ? null : selId } : null;
  }

  /**
   * WinForms / WPF: Ctrl+drag on the design surface = the selection copied, the copies where it was let
   * go (dx / dy px from the originals, which stay); each copy right after its original -- the same
   * container, drawn over it -- with new names (spbSave -> spbSave_2); the first copy selected; ONE undo.
   */
  async cmdCopyDrop(d, m) {
    if (this.active !== d) this.setActive(d);
    const ids = Array.isArray(m && m.ids) ? m.ids.filter(x => typeof x === 'string' && x && x !== '@form') : [];
    const dx = Math.round(+(m && m.dx) || 0), dy = Math.round(+(m && m.dy) || 0);
    if (!ids.length) { this.refuseEdit(d, '沒有可以複製的元件（先選元件；表單本身、沒有名稱（id）的不能複製）。'); return null; }
    if (!dx && !dy) return null;
    const text = d.doc.getText();
    const us = this.unitsFor(text, ids);
    if (us.error) { this.refuseEdit(d, us.error); return null; }
    const used = new Set(pageinfo.collectIds(text).concat(d.treeData.map(r => r[2]).filter(Boolean)));
    const made = [];
    const parts = us.units.map(u => {
      const r = htmlblock.renameIds(u.html, used);
      made.push(...r.map.values());
      // "\n" + indent + block, like Paste next to an element (Delete takes the same break away again)
      return { range: [u.end, u.end], repl: '\n' + htmlblock.indentAt(text, u.start) + htmlblock.offsetBlock(r.html, dx, dy) };
    });
    d.lastSelId = made[0] || d.lastSelId;
    const ok = await this.applyStructural(d, parts);
    if (ok) vscode.window.setStatusBarMessage('$(copy) 複製了 ' + made.join('、') + '（位移 ' + dx + ', ' + dy + '；Ctrl+Z 復原）', 6000);
    return ok ? { made, dx, dy } : null;
  }

  /** From the properties panel: ask the probe to apply (it then reports an 'edit').
   *  all: the panel shows a multi-selection -- the probe sets it on every selected one. */
  forwardEdit(d, key, m, all) {
    if (!d || key == null) return;
    all = all === true;
    if (m.type === 'setLayout') d.post({ type: 'setLayout', key, left: m.left, top: m.top, width: m.width, height: m.height, all });
    else if (m.type === 'setCaption') d.post({ type: 'setCaption', key, value: m.value, all });
    else if (m.type === 'setLook') {
      const size = m.size && typeof m.size.width === 'number' && typeof m.size.height === 'number' ? { width: m.size.width, height: m.size.height } : undefined;
      d.post({ type: 'setLook', key, prop: m.prop, value: m.value, all, size });
    }
    else if (m.type === 'align') d.post({ type: 'align', how: m.how });
  }

  // --- code -> designer ------------------------------------------------------
  /** Roots when no page is involved (a C++ file): settings, else the workspace. */
  globalRoots() {
    return roots.resolveRoots(null, this.wsFolders(), this.over());
  }

  /** The code -> page index for the current roots (built lazily, once). */
  reverseIndex() {
    const r = this.globalRoots();
    if (!r.irRoot || !r.webRoot) return null;
    const key = (r.irRoot + '|' + r.webRoot).toLowerCase();
    if (!this._rev || this._rev.key !== key) {
      this._rev = { key, idx: new ReverseIndex(this.irStore(r.irRoot), [path.join(r.webRoot, 'page'), r.webRoot]) };
    }
    return this._rev.idx;
  }

  /** C++ -> web for commands: which web script sends "x.y" (built lazily, once). */
  webCmdIndex(webRoot) {
    const root = webRoot || this.globalRoots().webRoot;
    if (!root) return null;
    this._wcis = this._wcis || new Map();
    const key = root.toLowerCase();
    let e = this._wcis.get(key);
    if (!e) {
      e = { idx: new WebCmdIndex(root), timer: null };
      this._wcis.set(key, e);
      // JSON\ holds data the running system may rewrite often: not code, ignore it
      this.watch(root, '**/*.{js,html}', uri => {
        if (/[\\/]JSON[\\/]/i.test(uri.fsPath)) return;
        clearTimeout(e.timer);
        e.timer = setTimeout(() => { e.idx = new WebCmdIndex(root); this.lens._em.fire(); }, 1500);
      });
    }
    return e.idx;
  }

  async cmdOpenWebSenders(cmd, hits) {
    const list = Array.isArray(hits) ? hits : [];
    if (!list.length) return;
    let pick = list[0];
    if (list.length > 1) {
      const it = await vscode.window.showQuickPick(list.map(h => ({
        label: '$(globe) ' + path.basename(h.file) + ':' + h.line,
        description: h.pages && h.pages.length ? h.pages.map(p => path.basename(p)).slice(0, 3).join('、') + (h.pages.length > 3 ? '…' : '') : '',
        detail: h.snippet, h,
      })), { placeHolder: '網頁在這些地方送出「' + cmd + '」', matchOnDescription: true, matchOnDetail: true });
      if (!it) return;
      pick = it.h;
    }
    return this.openTarget({ kind: 'web', file: pick.file, line: pick.line, col: pick.col }, null);
  }

  /** Open a page in the designer (or bring it forward) and select a control. */
  async openInDesigner(e) {
    let d = Array.from(this.designers).find(x => web.samePath(x.file, e.page));
    if (d) {
      d.panel.reveal(d.panel.viewColumn, false);
    } else {
      await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(e.page), VIEW_TYPE, vscode.ViewColumn.Beside);
      for (let i = 0; i < 100 && !d; i++) {
        d = Array.from(this.designers).find(x => web.samePath(x.file, e.page));
        if (!d) await sleep(100);
      }
    }
    if (!d) return null;
    await Promise.race([d.readyP, sleep(15000)]);
    d.post({ type: 'selectId', id: e.node, origin: 'code' });
    return d;
  }

  async cmdShowInDesigner(entries) {
    const list = Array.isArray(entries) ? entries : entries ? [entries] : [];
    if (!list.length) return;
    if (list.length === 1) return this.openInDesigner(list[0]);
    const pick = await vscode.window.showQuickPick(list.map(e => ({
      label: (e.node === '@form' ? '表單' : e.node) + (e.event ? '.' + e.event : ''),
      description: path.basename(e.page) + (e.nodeClass ? '　' + e.nodeClass : ''),
      detail: e.path || undefined,
      e,
    })), { placeHolder: '這段程式對應好幾個元件，要看哪一個？', matchOnDescription: true });
    if (pick) return this.openInDesigner(pick.e);
  }

  /** Editor context menu: the handler the cursor is in, or the control name under it. */
  async cmdShowCodeInDesigner() {
    const ed = vscode.window.activeTextEditor;
    const idx = this.reverseIndex();
    if (!ed || !idx) { vscode.window.showInformationMessage('找不到 web 或 ir_out 資料夾（看「HTML設計: 顯示偵測到的資料夾」）。'); return; }
    const doc = ed.document;
    const text = doc.getText();
    const off = doc.offsetAt(ed.selection.active);
    const wr = doc.getWordRangeAtPosition(ed.selection.active, /[A-Za-z_]\w*/);
    const word = wr ? doc.getText(wr) : '';
    let entries = [];
    // TfHotPlate::spbSaveClick written at the cursor, or the definition it sits in
    const q = wr ? /\b(T[A-Za-z_]\w*)\s*::\s*$/.exec(text.slice(Math.max(0, doc.offsetAt(wr.start) - 80), doc.offsetAt(wr.start))) : null;
    if (q) entries = idx.handler(q[1], word);
    if (!entries.length && word && /^T/.test(word)) {
      const after = /^\s*::\s*([A-Za-z_]\w*)/.exec(text.slice(doc.offsetAt(wr.end), doc.offsetAt(wr.end) + 80));
      if (after) entries = idx.handler(word, after[1]);
    }
    const encl = enclosingMethod(text, off, lex.isDefinitionAt);
    // a control name (spbSave->Enabled) of the form this code belongs to
    if (!entries.length && word) {
      const cls = encl ? [encl.cls] : this.formClassesOf(text, idx);
      for (const c of cls) entries = entries.concat(idx.control(c, word));
    }
    if (!entries.length && encl) entries = idx.handler(encl.cls, encl.method);
    if (!entries.length) {
      vscode.window.showInformationMessage(
        (encl ? encl.cls + '::' + encl.method : word || '這裡') + '：找不到對應的網頁元件（只認得有網頁、而且網頁標題寫了 .dfm 的表單）。');
      return;
    }
    return this.cmdShowInDesigner(entries);
  }

  /** Form classes a C++ text belongs to (the most used TClass:: prefixes that have a page). */
  formClassesOf(text, idx) {
    const count = new Map();
    const re = /\b(T[A-Za-z_]\w*)\s*::/g;
    let m;
    while ((m = re.exec(text))) if (idx.hasClass(m[1])) count.set(m[1], (count.get(m[1]) || 0) + 1);
    return Array.from(count.entries()).sort((a, b) => b[1] - a[1]).slice(0, 2).map(x => x[0]);
  }

  /** 接線總覽: every DFM event of the page -- wired on the web? implemented in C++? */
  async cmdPageOverview() {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return; }
    const pi = await d.info;
    if (!pi.ir) { vscode.window.showInformationMessage('這一頁沒有對應的 DFM，沒有事件可以總覽。'); return; }
    return vscode.window.withProgress({ location: vscode.ProgressLocation.Notification, title: '整理接線總覽：' + path.basename(d.file) + '…' }, async () => {
      const ov = await this.computeOverview(d, pi);
      OverviewPanel.show(this, d, ov);
      return ov;
    });
  }

  /**
   * 頁面檢查 (WPF: the Error List): problems in the page sources, into VS Code's Problems
   * panel -- a click there opens the line; a cut attribute has a quick fix (the bulb).
   * scope: 'page' (the active page) | 'all' (every page of the web folder).
   */
  async cmdLint(scope) {
    let files = [];
    if (scope === 'all') {
      const webRoot = (this.active && this.active.r.webRoot) || roots.resolveRoots(null, this.wsFolders(), this.over()).webRoot;
      if (!webRoot) { vscode.window.showInformationMessage('找不到 web 資料夾（設定 ht9045Designer.webRoot）。'); return null; }
      files = [].concat(...pagelist.listPages(webRoot).map(g => g.pages.map(p => p.file)));
    } else {
      const f = this.active ? this.active.file : (vscode.window.activeTextEditor && /\.html?$/i.test(vscode.window.activeTextEditor.document.fileName) ? vscode.window.activeTextEditor.document.fileName : null);
      if (!f) { vscode.window.showInformationMessage('先開一頁（設計檢視或 HTML），或用「檢查所有頁面」。'); return null; }
      files = [f];
    }
    const sum = { pages: files.length, withIssues: 0, error: 0, warn: 0, info: 0, byKind: {} };
    for (const f of files) {
      const n = this.lintFile(f);
      if (n.some(i => i.level !== 'info')) sum.withIssues++;
      for (const i of n) { sum[i.level === 'error' ? 'error' : i.level === 'info' ? 'info' : 'warn']++; sum.byKind[i.kind] = (sum.byKind[i.kind] || 0) + 1; }
    }
    this.log('頁面檢查（' + (scope === 'all' ? '所有頁面' : path.basename(files[0])) + '）：' + sum.pages + ' 頁，' + sum.withIssues + ' 頁有問題；錯誤 ' + sum.error + '、警告 ' + sum.warn +
      '、跟 DFM 不同（資訊）' + sum.info + ' ' + JSON.stringify(sum.byKind));
    if (sum.error + sum.warn + sum.info) vscode.commands.executeCommand('workbench.actions.view.problems').then(() => {}, () => {});
    vscode.window.setStatusBarMessage('$(checklist) 頁面檢查：' + (sum.error + sum.warn ? '錯誤 ' + sum.error + '、警告 ' + sum.warn : '沒有錯誤') +
      (sum.info ? '；跟 BCB6 .dfm 不同 ' + sum.info + ' 個（資訊）' : '') + (sum.error + sum.warn + sum.info ? '（看「問題」面板）' : ''), 8000);
    return sum;
  }

  /** The .dfm form a page's title names (IR), or null -- for the checks, only a named one (no guessing). */
  irForFile(file, text) {
    try {
      const r = this.rootsFor(file);
      const store = r && r.irRoot ? this.irStore(r.irRoot) : null;
      const title = pageinfo.parseTitle(text);
      if (!store || !title || !title.dfm) return null;
      const f = store.find(title.dfm, title.cls);
      return f ? store.load(f) : null;
    } catch (e) { return null; }
  }

  /** One page into the Problems panel (the open document's text when it is open). [issues] */
  lintFile(file) {
    const uri = vscode.Uri.file(file);
    const open = vscode.workspace.textDocuments.find(td => td.uri.scheme === 'file' && web.samePath(td.uri.fsPath, file));
    let text = '';
    try { text = open ? open.getText() : fs.readFileSync(file, 'utf8'); } catch (e) { return []; }
    const issues = pagelint.lintPage(text, { dir: path.dirname(file) });
    // what the generator left out of the form (AutoSize / Alignment against the page's .dfm): information
    if (this.cfg().get('lintDfmGaps') !== false) {
      const ir = this.irForFile(file, text);
      if (ir) { issues.push(...pagelint.dfmGaps(text, ir)); issues.sort((a, b) => a.at - b.at); }
    }
    const starts = [0];
    for (let i = 0; i < text.length; i++) if (text.charCodeAt(i) === 10) starts.push(i + 1);
    const pos = off => {
      let lo = 0, hi = starts.length - 1;
      while (lo < hi) { const m = (lo + hi + 1) >> 1; if (starts[m] <= off) lo = m; else hi = m - 1; }
      return new vscode.Position(lo, off - starts[lo]);
    };
    const diags = issues.map(i => {
      const dg = new vscode.Diagnostic(new vscode.Range(pos(i.at), pos(i.at + i.len)), i.msg,
        i.level === 'error' ? vscode.DiagnosticSeverity.Error : i.level === 'info' ? vscode.DiagnosticSeverity.Information : vscode.DiagnosticSeverity.Warning);
      dg.source = LINT_SOURCE;
      dg.code = i.kind;
      return dg;
    });
    if (diags.length) this.lintDiag.set(uri, diags); else this.lintDiag.delete(uri);
    // the 頁面 list shows each page's count ("⚠ 93"): once after a burst of checks
    clearTimeout(this._pmT);
    this._pmT = setTimeout(() => { if (this.pages) this.pages.mark(); }, 200);
    return issues;
  }

  /** Every cut attribute of a page fixed at once (one WorkspaceEdit). */
  async cmdLintFixAll(uri0) {
    const file = uri0 instanceof vscode.Uri ? uri0.fsPath : this.active ? this.active.file : vscode.window.activeTextEditor ? vscode.window.activeTextEditor.document.fileName : null;
    if (!file) return null;
    const doc = await vscode.workspace.openTextDocument(vscode.Uri.file(file));
    const issues = pagelint.lintPage(doc.getText(), { dir: path.dirname(file) }).filter(i => i.fix && i.kind === 'quote-cut');
    if (!issues.length) { vscode.window.setStatusBarMessage('$(check) ' + path.basename(file) + ' 沒有被切斷的屬性', 4000); return 0; }
    const we = new vscode.WorkspaceEdit();
    for (const i of issues) we.replace(doc.uri, new vscode.Range(doc.positionAt(i.fix.at), doc.positionAt(i.fix.at + i.fix.len)), i.fix.repl);
    const ok = await vscode.workspace.applyEdit(we);
    if (ok) {
      if (this.active && web.samePath(this.active.file, file)) this.noteFirstEdit(this.active);
      vscode.window.setStatusBarMessage('$(wrench) ' + path.basename(file) + '：修正了 ' + issues.length + ' 個被切斷的屬性（還沒存檔；Ctrl+Z 復原）', 7000);
      this.lintFile(file);
    }
    return ok ? issues.length : null;
  }

  /**
   * Every "differs from the .dfm" note of a page fixed at once (one WorkspaceEdit): the labels'
   * AutoSize=False size and alignment, the panel captions' alignment -- the .dfm's values.
   */
  async cmdLintFixDfmGaps(uri0) {
    const file = uri0 instanceof vscode.Uri ? uri0.fsPath : this.active ? this.active.file : vscode.window.activeTextEditor ? vscode.window.activeTextEditor.document.fileName : null;
    if (!file) return null;
    const doc = await vscode.workspace.openTextDocument(vscode.Uri.file(file));
    const text = doc.getText();
    const ir = this.irForFile(file, text);
    const gaps = ir ? pagelint.dfmGaps(text, ir) : [];
    if (!gaps.length) { vscode.window.setStatusBarMessage('$(check) ' + path.basename(file) + (ir ? ' 的 Label／Panel 跟 DFM 一樣' : ' 的標題沒有寫是哪個 .dfm'), 4000); return 0; }
    const we = new vscode.WorkspaceEdit();
    for (const g of gaps) we.replace(doc.uri, new vscode.Range(doc.positionAt(g.fix.at), doc.positionAt(g.fix.at + g.fix.len)), g.fix.repl);
    const ok = await vscode.workspace.applyEdit(we);
    if (ok) {
      if (this.active && web.samePath(this.active.file, file)) this.noteFirstEdit(this.active);
      vscode.window.setStatusBarMessage('$(wrench) ' + path.basename(file) + '：' + gaps.length + ' 個 Label／Panel 改成 BCB6 .dfm 的大小與對齊（還沒存檔；Ctrl+Z 復原）', 7000);
      this.lintFile(file);
    }
    return ok ? gaps.length : null;
  }

  /** Every DFM event of the page: wired on the web? implemented in C++? (the overview's data) */
  async computeOverview(d, pi) {
    const t0 = Date.now();
    const reply = await d.request({ type: 'listenersAll' }, 5000);
    const sources = this.webSources(d, pi);
    const widx = d.r.webRoot ? this.webCmdIndex(d.r.webRoot) : null;
    const fieldIds = new Set();
    const tagIds = new Set();
    if (widx) {
      widx.build();
      for (const id of widx.fieldsById.keys()) if (widx.fieldsOf(d.file, id).length) fieldIds.add(id);
      for (const id of widx.tagsById.keys()) if (widx.tagsOf(d.file, id).length) tagIds.add(id);
    }
    const ov = await overview.buildOverview({
      ir: pi.ir,
      pageIds: new Set(pageinfo.collectIds(d.doc.getText()).concat(d.treeData.map(r => r[2]))),
      listeners: reply ? reply.map : {},
      fieldIds, tagIds,
      quoted: overview.quotedNames(sources),
      classes: pi.classes,
      port: this.sourceTree(d.r.portRoot, 'port'),
      gold: this.sourceTree(d.r.goldenRoot, 'golden'),
    });
    ov.page = path.basename(d.file);
    ov.dfm = pi.ir.sourceDfm || path.basename(pi.ir.file);
    ov.formClass = pi.ir.formClass || '';
    ov.listenersOk = !!reply;
    this.log('接線總覽 ' + ov.page + '：' + ov.summary.events + ' 個事件（' + (Date.now() - t0) + ' ms）' + (reply ? '' : '，⚠ 探針沒回應，網頁監聽器一欄不準'));
    return ov;
  }

  /**
   * 在畫面上標出接線狀態: every component with DFM events gets a coloured frame on the
   * design surface -- green: the web handles all of them (a listener or the wire engine's
   * field map), orange: some / only mentioned (delegated), red: none. On / off per designer.
   */
  async cmdWireMarks(on) {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    const nu = typeof on === 'boolean' ? on : !d.wireMarks;
    if (!nu) {
      d.wireMarks = null;
      d.post({ type: 'wireMarks', marks: null });
      this.updateStatus();
      return { on: false };
    }
    const pi = await d.info;
    if (!pi.ir) { vscode.window.showInformationMessage('這一頁沒有對應的 DFM，沒有事件可以標示。'); return null; }
    const ov = await this.computeOverview(d, pi);
    const marks = {};
    const count = { ok: 0, mid: 0, bad: 0 };
    for (const row of ov.rows) {
      if (!row.present || row.id === '@form' || !row.events.length) continue;
      const good = row.events.filter(e => e.web === 'yes' || e.web === 'field').length;
      const none = row.events.filter(e => e.web === 'no').length;
      const st = good === row.events.length ? 'ok' : none === row.events.length ? 'bad' : 'mid';
      marks[row.id] = st;
      count[st]++;
    }
    d.wireMarks = marks;
    d.post({ type: 'wireMarks', marks });
    this.updateStatus();
    vscode.window.setStatusBarMessage('$(plug) 接線標示：綠 ' + count.ok + '（網頁有接）、橘 ' + count.mid + '（部分／只有提到）、紅 ' + count.bad + '（網頁沒接）', 8000);
    return { on: true, marks, count, listenersOk: ov.listenersOk };
  }

  /**
   * DFM 位置: a purple dashed frame on the design surface where the BCB6 .dfm puts each
   * control whose position / size differs from it. The frame stays at the DFM place while
   * the control is dragged, and a drag near it snaps onto it. On / off per designer; while
   * on, it follows every edit (compared again shortly after).
   */
  async cmdDfmGhosts(on, d0, quiet) {
    const d = d0 instanceof Designer ? d0 : this.active;
    if (!d) { if (!quiet) vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    // (a click while the first "on" still waits for the page counts as on already: the second turns it off)
    const nu = typeof on === 'boolean' ? on : !(d.dfmGhosts || d.ghostsComing);
    if (!nu) {
      d.dfmGhosts = null;
      d.ghostsComing = false;
      d.ghostGen = (d.ghostGen || 0) + 1;   // a comparison already on its way must not turn them back on
      clearTimeout(d.ghostTimer);
      d.post({ type: 'dfmGhosts', items: null });
      this.updateStatus();
      return { on: false };
    }
    const gen = d.ghostGen || 0;
    if (!d.dfmGhosts) d.ghostsComing = true;
    const pi = await d.info;
    if (!pi.ir) { d.ghostsComing = false; if (!quiet) vscode.window.showInformationMessage('這一頁沒有對應的 DFM，沒有位置可以畫。'); return null; }
    const reply = await d.request({ type: 'lookAll', ids: Array.from(pi.ir.byName.keys()) }, 8000);
    if ((d.ghostGen || 0) !== gen) return { on: false };   // turned off while it compared
    d.ghostsComing = false;
    if (!reply) { if (!quiet) vscode.window.showWarningMessage('設計檢視沒有回應（頁面還在載入？），請稍後再試。'); return null; }
    const items = dfmdiff.ghostsOf(dfmdiff.diffPage(reply.items || [], pi.ir).rows);
    d.dfmGhosts = items;
    d.post({ type: 'dfmGhosts', items });
    this.updateStatus();
    if (!quiet) vscode.window.setStatusBarMessage('$(target) DFM 位置：' + (items.length ? items.length + ' 個元件的位置／大小跟 BCB6 .dfm 不同（紫色虛線框＝DFM 的位置）' : '所有元件的位置／大小都跟 DFM 一樣'), 8000);
    return { on: true, items };
  }

  /**
   * 改回 DFM: the selected component(s) back to what the .dfm says -- position / size,
   * caption and look, every difference the DFM list would show for them -- in ONE edit
   * (one undo step). A locked one keeps its place (and it is said); a control the page's JS
   * builds is not in the source, so it is left alone.
   */
  async cmdResetToDfm(d0, only, prop) {
    const d = d0 instanceof Designer ? d0 : this.active;
    // WPF Layout > Reset Width / Height / ... / Reset All: only = 'pos' (Left, Top), 'size' (Width, Height),
    // 'layout' (the four); none = everything that differs (the text and the look too)
    const ONLY = { pos: /^(Left|Top)$/, size: /^(Width|Height)$/, layout: /^(Left|Top|Width|Height)$/ };
    const onlyRe = only && ONLY[only] ? ONLY[only] : null;
    const what = { pos: '位置', size: '大小', layout: '版面' }[only] || (prop ? prop + ' ' : '');
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    const ids = this.selIds(d);
    if (!ids.length) { vscode.window.setStatusBarMessage('$(info) 先選取元件（表單本身沒有可以改回的）', 5000); return null; }
    const pi = await d.info;
    if (!pi.ir) { vscode.window.showInformationMessage('這一頁沒有對應的 DFM，沒有東西可以改回。'); return null; }
    const known = ids.filter(id => pi.ir.byName.has(id));
    if (!known.length) { vscode.window.setStatusBarMessage('$(info) DFM 裡沒有「' + ids[0] + '」，沒有東西可以改回', 5000); return null; }
    const reply = await d.request({ type: 'lookAll', ids: known }, 8000);
    if (!reply) { vscode.window.showWarningMessage('設計檢視沒有回應（頁面還在載入？），請稍後再試。'); return null; }
    const srcText = d.doc.getText();
    const inSource = new Set(pageinfo.collectIds(srcText));
    // position / size: what the SOURCE says now (a move just made, the page not drawn again yet, still counts)
    for (const it of reply.items || []) {
      const lay = it && it.lay;
      if (!lay || lay.root || lay.target !== 'self') continue;
      const tg = htmledit.startTagOf(srcText, it.id);
      if (!tg) continue;
      for (const dc of htmledit.styleOf(tg.text).decls) {
        const k = String(dc.name || '').trim().toLowerCase();
        const px = /^(-?\d+(?:\.\d+)?)px$/.exec(String(dc.value || '').trim());
        if (!px || !/^(left|top|width|height)$/.test(k)) continue;
        lay[k] = Math.round(+px[1]);
        if (lay.raw) lay.raw[k] = px[0];
      }
    }
    const rows = dfmdiff.diffPage(reply.items || [], pi.ir).rows.filter(r => inSource.has(r.id) && (!onlyRe || (r.group === 'layout' && onlyRe.test(r.prop))) && (!prop || r.prop === prop));
    const items = dfmdiff.resetItemsOf(rows);
    if (!items.length) {
      vscode.window.setStatusBarMessage('$(check) ' + (known.length > 1 ? '這 ' + known.length + ' 個' : '「' + known[0] + '」') + '的' + (what || '') + '跟 DFM 一樣，沒有要改回的', 5000);
      return { count: 0, items, only: only || null };
    }
    d.post({ type: 'editMany', items });
    const ctl = new Set(rows.map(r => r.id)).size;
    vscode.window.setStatusBarMessage('$(discard) ' + (what ? what + '改回 DFM：' : '改回 DFM：') + ctl + ' 個元件、' + rows.length + ' 項（一個 Ctrl+Z 可以全部復原）', 8000);
    return { count: rows.length, controls: ctl, items, only: only || null };
  }

  /**
   * Tab 順序 (BCB6's Edit > Tab Order): a badge on every place the page's Tab key stops,
   * numbered, compared with the order the .dfm gives the Tab key (TabOrder in each
   * container, depth first -- lib/taborder.js). Red = not where the .dfm has it. On / off
   * per designer; the ones out of order go to the output panel.
   */
  async cmdTabOrder(on, d0) {
    const d = d0 instanceof Designer ? d0 : this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return null; }
    const nu = typeof on === 'boolean' ? on : !d.tabOrder;
    if (!nu) {
      d.tabOrder = null;
      d.post({ type: 'tabOrder', dfm: null });
      this.updateStatus();
      return { on: false };
    }
    const pi = await d.info;
    const dfm = pi.ir ? taborder.dfmTabOrder(pi.ir) : [];
    d.tabOrder = dfm;
    const rep = await d.request({ type: 'tabOrder', dfm }, 5000);
    this.updateStatus();
    if (!rep) { vscode.window.showWarningMessage('設計檢視沒有回應（頁面還在載入？），請稍後再試。'); return { on: true, dfm }; }
    const bad = Array.isArray(rep.bad) ? rep.bad : [];
    vscode.window.setStatusBarMessage('$(list-ordered) Tab 順序：網頁 ' + rep.stops + ' 個 Tab 停點；跟 DFM 比 ' + rep.common + ' 個' +
      (pi.ir ? (bad.length ? '，' + bad.length + ' 個順序不同（紅色）' : '，順序都一樣') : '（這一頁沒有 DFM）'), 10000);
    if (bad.length) this.log('Tab 順序 ' + path.basename(d.file) + '：跟 DFM 不同的 ' + bad.length + ' 個 —— ' + bad.slice(0, 40).join('、') + (bad.length > 40 ? ' …' : ''));
    return { on: true, dfm, stops: rep.stops, common: rep.common, bad };
  }

  /** DFM 位置 is on: compare again a moment after an edit / a re-draw (one at a time). */
  scheduleGhosts(d) {
    if (!d || !d.dfmGhosts) return;
    clearTimeout(d.ghostTimer);
    d.ghostTimer = setTimeout(() => { if (d.dfmGhosts) this.cmdDfmGhosts(true, d, true).catch(() => null); }, 700);
  }

  /** 與 DFM 的差異: every control of the page against the .dfm (position, text, look). */
  async cmdDfmDiff(d0, quiet) {
    // (from a menu VS Code passes a Uri / the webview context as the first argument)
    const d = d0 instanceof Designer ? d0 : this.active;
    quiet = d0 instanceof Designer && quiet === true;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return; }
    const pi = await d.info;
    if (!pi.ir) { if (!quiet) vscode.window.showInformationMessage('這一頁沒有對應的 DFM，沒有東西可以比對。'); return; }
    const t0 = Date.now();
    const ids = Array.from(pi.ir.byName.keys());
    const reply = await d.request({ type: 'lookAll', ids }, 8000);
    if (!reply) { if (!quiet) vscode.window.showWarningMessage('設計檢視沒有回應（頁面還在載入？），請稍後再試。'); return; }
    const res = dfmdiff.diffPage(reply.items || [], pi.ir);
    res.page = path.basename(d.file);
    res.dfm = pi.ir.sourceDfm || path.basename(pi.ir.file);
    res.formClass = pi.ir.formClass || '';
    // a control the page's JS builds is not in the HTML source: nothing to write back into
    const inSource = new Set(pageinfo.collectIds(d.doc.getText()));
    for (const r of res.rows) r.inSource = inSource.has(r.id);
    res.dirty = !!d.doc.isDirty;
    // DFM 位置 on: the frames follow this comparison (no second look at the page)
    if (d.dfmGhosts) { clearTimeout(d.ghostTimer); d.dfmGhosts = dfmdiff.ghostsOf(res.rows); d.post({ type: 'dfmGhosts', items: d.dfmGhosts }); }
    res.ghostsOn = !!d.dfmGhosts;
    this.log('與 DFM 的差異 ' + res.page + '：' + res.controls + ' 個元件、比了 ' + res.compared + ' 項、不同 ' + res.rows.length +
      ' 項（位置／大小 ' + res.byGroup.layout + '、文字 ' + res.byGroup.text + '、外觀 ' + res.byGroup.look + '；' + (Date.now() - t0) + ' ms）');
    DfmDiffPanel.show(this, d, res, quiet);
    return res;
  }

  /**
   * 在所有頁面搜尋: a component by its name, its class or the text it shows, over every
   * page of the web folder; the pick opens that page and selects it there. With a
   * string argument (tests, other code) the hits are returned without asking.
   */
  async cmdSearchPages(query) {
    const webRoot = (this.active && this.active.r.webRoot) || roots.resolveRoots(null, this.wsFolders(), this.over()).webRoot;
    if (!webRoot) { vscode.window.showInformationMessage('找不到 web 資料夾（設定 ht9045Designer.webRoot）。'); return null; }
    const direct = typeof query === 'string';
    const q = direct ? query : await vscode.window.showInputBox({
      prompt: '在所有頁面找元件：名稱、型別或畫面上的文字（空白隔開＝每個字都要有）',
      placeHolder: '例如 spbSave、Save、TGroupBox、存檔', value: this.lastPageQuery || '',
    });
    if (!q || !q.trim()) return null;
    this.lastPageQuery = q;
    const t0 = Date.now();
    const hits = pagesearch.searchPages(webRoot, q, 500);
    this.log('在所有頁面搜尋「' + q + '」：' + hits.length + ' 個（' + (Date.now() - t0) + ' ms）');
    if (direct) return hits;
    if (!hits.length) { vscode.window.showInformationMessage('所有頁面都沒有找到「' + q + '」。'); return hits; }
    const HOW = { id: '名稱', text: '文字', class: '型別' };
    const pick = await vscode.window.showQuickPick(hits.map(h => ({
      label: h.id,
      description: (h.cls ? h.cls + '　' : '') + (h.caption ? '「' + h.caption.slice(0, 40) + (h.caption.length > 40 ? '…' : '') + '」' : ''),
      detail: h.page + ':' + h.line + '　（找到：' + HOW[h.how] + '）',
      hit: h,
    })), { placeHolder: '找到 ' + hits.length + (hits.length >= 500 ? '+' : '') + ' 個「' + q + '」— 選一個就開那一頁並選取它', matchOnDescription: true, matchOnDetail: true });
    if (pick) await this.openPageAt(pick.hit.file, pick.hit.id);
    return hits;
  }

  /** Open `file` in the designer and select `id` once its page is drawn (a hidden tab opens). */
  /**
   * WPF: "Right-click the element in the Document Outline window or the artboard" -- the same commands. A command run
   * from the component tree's menu gets that row: the component is selected first (unless it already is, alone).
   * Anything else as the argument (nothing, a name, { confirmed }) is left alone.
   */
  async onNode(n) {
    const d = this.active;
    if (!d || !n || typeof n !== 'object' || n.key == null || 'confirmed' in n || typeof n.id === 'undefined') return;
    const cur = d.sel && d.sel.info;
    if (cur && cur.key === n.key && !(cur.multi || []).length) return;
    this.tree.lastClicked = n.key;
    d.post({ type: 'selectKey', key: n.key, origin: 'tree' });
    for (let i = 0; i < 30; i++) {
      if (d.sel && d.sel.info && d.sel.info.key === n.key) return;
      await sleep(100);
    }
  }

  /**
   * Shift+F7 from code (WPF: View Designer, "Shift+F7 toggles to the designer"): a page's HTML -> its designer with the
   * element at the cursor selected; a C++ file -> the component its code at the cursor is for (showCodeInDesigner).
   * (In the designer, Shift+F7 goes the other way: the selected component's HTML -- revealSource.)
   */
  async cmdViewDesigner() {
    const ed = vscode.window.activeTextEditor;
    if (!ed) return null;
    const doc = ed.document;
    if (/\.(cpp|cc|cxx|h|hpp|c)$/i.test(doc.fileName)) return this.cmdShowCodeInDesigner();
    if (!/\.html?$/i.test(doc.fileName)) return null;
    const id = this.idAtOffset(doc.getText(), doc.offsetAt(ed.selection.active));
    if (!id) { await this.cmdOpen(doc.uri); return { file: doc.fileName, id: null }; }
    const d = await this.openPageAt(doc.fileName, id);
    return d ? { file: doc.fileName, id } : null;
  }

  /**
   * 移到事件處理函式 from the page's HTML (WPF, XAML view: right-click the event -> "Navigate to Event Handler"):
   * on a designer event's htdCpp(...) line -> that C++ function; anywhere in an element -> its default event's code
   * (as F7 does -- nothing is added).
   */
  async cmdGotoEventHandler() {
    const ed = vscode.window.activeTextEditor;
    if (!ed || !/\.html?$/i.test(ed.document.fileName)) return null;
    const doc = ed.document;
    const text = doc.getText();
    const off = doc.offsetAt(ed.selection.active);
    const line = jsevents.cppLines(text).find(x => off >= x.s && off <= x.e);
    if (line) {
      const r = this.rootsFor(doc.fileName);
      const port = r.portRoot ? this.sourceTree(r.portRoot, 'port') : null;
      const st = port ? await port.defState([line.form], line.handler) : { state: 'none' };
      if (st.state === 'live' && st.hit) { await this.openTarget(this.cppTarget('port', st.hit), null); return { form: line.form, handler: line.handler }; }
      const od = r.portRoot ? this.dirtyHit(r.portRoot, new RegExp('\\b' + line.form + '\\s*::\\s*' + line.handler + '\\s*\\('), /\.cpp$/i) : null;
      if (od) { await this.openTarget({ kind: 'port', file: od.file, line: od.line, col: 1 }, null); return { form: line.form, handler: line.handler, unsaved: true }; }
      vscode.window.showInformationMessage('移植樹裡找不到 ' + line.form + '::' + line.handler + ' 的本體。');
      return null;
    }
    const id = this.idAtOffset(text, off);
    if (!id) { vscode.window.setStatusBarMessage('$(info) 游標不在任何元件裡', 4000); return null; }
    const d = await this.openPageAt(doc.fileName, id);
    if (!d) return null;
    for (let i = 0; i < 100 && !(d.sel && d.sel.info && d.sel.info.id === id); i++) await sleep(100);
    if (!(d.sel && d.sel.info && d.sel.info.id === id)) return null;
    await this.onDesignerDblClick(d, true);
    return { id };
  }

  /**
   * The id of the element whose start tag -- or body -- holds `off` (the innermost one with an id; the generated
   * <div class="form"> = '@form'), or null (the head, a script, outside the form).
   */
  idAtOffset(text, off) {
    const idOf = tag => {
      const m = /\sid\s*=\s*["']([^"']+)["']/.exec(tag);
      if (m) return m[1];
      return /^<div\b[^>]*\bclass\s*=\s*["']form["']/.test(tag) ? '@form' : null;
    };
    let at = off;
    // the start tag the cursor is in
    const lt = text.lastIndexOf('<', at);
    const gt = lt >= 0 ? text.indexOf('>', lt) : -1;
    if (lt >= 0 && gt >= off && text[lt + 1] !== '/' && text[lt + 1] !== '!') {
      const own = idOf(text.slice(lt, gt + 1));
      if (own) return own;
      at = lt;
    }
    // the elements open there, innermost first (one pass over the page)
    const stack = htmlblock.openStackAt(text, at) || [];
    for (let i = stack.length - 1; i >= 0; i--) {
      const got = idOf(stack[i].text);
      if (got) return got;
    }
    return null;
  }

  async openPageAt(file, id) {
    await vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(file), VIEW_TYPE);
    let d = null;
    for (let i = 0; i < 200 && !d; i++) {
      d = Array.from(this.designers).find(x => web.samePath(x.file, file) && x.treeData.length) || null;
      if (!d) await sleep(100);
    }
    if (!d) return null;
    d.lastSelId = id;
    d.post({ type: 'selectId', id, origin: 'search' });
    return d;
  }

  /** Quick pick over every component of the page (big pages have thousands). */
  async cmdFindComponent() {
    const d = this.active;
    if (!d) { vscode.window.showInformationMessage('先開啟一個設計檢視。'); return; }
    const pi = await d.info;
    const items = d.treeData.map(r => {
      const key = r[0], id = r[2], tag = r[3], cls = r[4], cap = r[5], hidden = r[6], isForm = r[7];
      const node = pi.ir && !isForm ? pi.ir.byName.get(id) : null;
      return {
        label: isForm ? d.formLabel : id,
        description: (cls || (node && node.class) || '<' + tag + '>') + (cap ? '　「' + cap + '」' : '') + (hidden ? '　（隱藏）' : ''),
        detail: node && node.path ? node.path : undefined,
        key,
      };
    });
    const pick = await vscode.window.showQuickPick(items, {
      placeHolder: '輸入元件名稱、型別或畫面上的文字（這頁共 ' + items.length + ' 個）',
      matchOnDescription: true, matchOnDetail: true,
    });
    if (pick) d.post({ type: 'selectKey', key: pick.key, origin: 'find' });
  }

  cmdShowInfo() {
    const r = this.active ? this.active.r : roots.resolveRoots(null, this.wsFolders(), this.over());
    this.out.appendLine('');
    this.out.appendLine('=== HTML 視覺設計：偵測結果 ===' + (this.version ? '（這個視窗跑的是 v' + this.version + '）' : ''));
    this.out.appendLine('web     ' + r.webRoot);
    this.out.appendLine('port    ' + r.portRoot);
    this.out.appendLine('ir      ' + r.irRoot);
    this.out.appendLine('golden  ' + r.goldenRoot);
    for (const [k, t] of this.trees) {
      this.out.appendLine('index   ' + k + '  ' + (t.buildMs ? t.files.length + ' 檔，' + t.qual.size + ' 名稱，' + t.buildMs + ' ms' : '建立中…'));
    }
    for (const d of this.designers) {
      this.out.appendLine('page    ' + d.file + '  mode=' + d.mode + '  blocked=' + d.blocked.length + '  errors=' + d.errors.length);
    }
    this.out.show(true);
  }
}

// ---------------------------------------------------------------------------
class Designer {
  constructor(hub, doc, panel) {
    this.hub = hub;
    this.doc = doc;
    this.panel = panel;
    this.id = ++Designer.seq;
    this.mode = 'design';
    this.treeData = [];
    this.lastSelId = null;
    this.scroll = null;
    this.sel = null;
    this.blocked = [];
    this.errors = [];
    // hidden in the designer only (WPF: the eye in the document outline); kept per
    // page in the workspace state, never in the source
    this.designHidden = new Set(hub.loadDesignHidden(doc.uri.fsPath));
    this.designLocked = new Set(hub.loadDesignSet('htd.designLocked', doc.uri.fsPath));
    this.formLabel = '表單';
    this.disposed = false;
    this.renderN = 0;
    this.readyP = new Promise(res => { this._ready = res; });   // first component tree arrived
    this.selfEdits = 0;   // our own source edits: the DOM already shows them, no reload
    this.file = doc.uri.fsPath;
    // WPF's "Default zoom setting": the zoom it had last time (Last Used) / fitting the view once drawn (Fit All) / 100%
    const dz = hub.defaultZoom();
    this.zoom = dz === 'last' ? hub.loadZoom(this.file) : 1;
    this.fitOnOpen = dz === 'fit';
    this.pageDir = path.dirname(this.file);
    this.r = hub.rootsFor(this.file);
    const res = [vscode.Uri.file(this.pageDir), vscode.Uri.joinPath(hub.ctx.extensionUri, 'media')];
    if (this.r.webRoot) res.push(vscode.Uri.file(this.r.webRoot));
    panel.webview.options = { enableScripts: true, enableCommandUris: false, localResourceRoots: res };
    this.subs = [
      panel.webview.onDidReceiveMessage(m => this.onMessage(m)),
      panel.onDidChangeViewState(() => { if (panel.active) hub.setActive(this); }),
      panel.onDidDispose(() => this.dispose()),
      vscode.workspace.onDidChangeTextDocument(e => {
        if (e.document !== doc || !e.contentChanges.length) return;
        if (this.selfEdits > 0) { this.selfEdits--; return; }   // a designer edit: the page already shows it
        this.scheduleRender();                                   // typing in the HTML, undo/redo, reload
      }),
    ];
    this.render();
    // 0.140 'wpf' (the default): the design above, the page's HTML below; closed when a C++ file is opened
    if (hub.defaultView() === 'wpf') Promise.resolve().then(() => hub.openWpf(this)).catch(e => hub.log('上下版面：' + (e && e.message || e)));
    // WPF's "Default document view: Split": the HTML opens beside the design surface (the focus stays on the design)
    else if (hub.defaultView() === 'split' && !vscode.window.visibleTextEditors.some(e => e.document === doc)) {
      Promise.resolve().then(() => vscode.window.showTextDocument(doc, { viewColumn: hub.sourceColumn(this), preview: false, preserveFocus: true }))
        .catch(e => hub.log('分割檢視：開不了 HTML（' + (e && e.message || e) + '）'));
    }
  }

  render() {
    this.info = this.hub.analyzePage(this);
    this.info.catch(e => this.hub.log('分析頁面失敗：' + (e && e.stack || e)));
    this.blocked = [];
    this.errors = [];
    const w = this.panel.webview;
    const base = w.asWebviewUri(vscode.Uri.file(this.pageDir)).toString().replace(/\/?$/, '/');
    const probe = w.asWebviewUri(vscode.Uri.joinPath(this.hub.ctx.extensionUri, 'media', 'probe.js')).toString();
    this.renderN++;
    // a forwarding page would navigate the designer away from itself: say so instead
    const target = pageinfo.redirectTarget(this.doc.getText());
    this.redirect = target;
    if (target) {
      const nonce = crypto.randomBytes(16).toString('base64');
      const esc = s => String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/"/g, '&quot;');
      w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
        '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src \'unsafe-inline\'; script-src \'nonce-' + nonce + '\';">' +
        '<style>body{font-family:var(--vscode-font-family);color:var(--vscode-foreground);padding:24px;line-height:1.7}' +
        'button{font:inherit;color:var(--vscode-button-foreground);background:var(--vscode-button-background);border:none;padding:4px 14px;cursor:pointer}' +
        'code{font-family:var(--vscode-editor-font-family)}</style></head><body>' +
        '<p><code>' + esc(path.basename(this.file)) + '</code> 只是一個轉址頁：一打開就會轉到 <code>' + esc(target) + '</code>，本身沒有畫面。</p>' +
        '<p><button id="go">用設計檢視開啟 ' + esc(target) + '</button></p>' +
        '<script nonce="' + nonce + '">(function(){var api=acquireVsCodeApi();' +
        'document.getElementById("go").addEventListener("click",function(){api.postMessage({__htd:1,type:"openRedirect"});});' +
        'api.postMessage({__htd:1,type:"ready",redirect:true});})();</script></body></html><!-- htd-render ' + this.renderN + ' -->';
      return;
    }
    // 機種與機台設定: the page decides what to show like on the machine (read-only copies); and a page that
    // picks its JSON folder by its address gets the web root's (the designer's address has no /page/)
    this.live = this.hub.liveFor(this);
    const jsonHere = path.join(this.pageDir, 'JSON'), jsonRoot = this.r.webRoot ? path.join(this.r.webRoot, 'JSON') : '';
    if (jsonRoot && !web.samePath(this.pageDir, this.r.webRoot) && !fs.existsSync(jsonHere) && fs.existsSync(jsonRoot)) {
      const u = p => w.asWebviewUri(vscode.Uri.file(p)).toString().replace(/\/?$/, '/');
      this.live = Object.assign({ machine: null, docs: {} }, this.live, { pathFix: { from: u(jsonHere), to: u(jsonRoot) } });
    }
    w.html = buildPageHtml(this.doc.getText(), {
      cspSource: w.cspSource, baseHref: base, probeSrc: probe, mode: this.mode, live: this.live,
    }) + '\n<!-- htd-render ' + this.renderN + ' -->';
  }

  scheduleRender() {
    clearTimeout(this.renderTimer);
    this.renderTimer = setTimeout(() => { if (!this.disposed) this.render(); }, 500);
  }

  post(m) {
    if (this.disposed) return;
    this.panel.webview.postMessage(Object.assign({ __htd: 1 }, m));
  }

  /** Ask the probe something and wait for the reply of the same type (null on timeout). */
  request(m, ms) {
    this._reqSeq = (this._reqSeq || 0) + 1;
    const seq = this._reqSeq;
    this._pending = this._pending || new Map();
    return new Promise(res => {
      const timer = setTimeout(() => { this._pending.delete(seq); res(null); }, ms || 5000);
      this._pending.set(seq, v => { clearTimeout(timer); res(v); });
      this.post(Object.assign({}, m, { seq }));
    });
  }

  onMessage(m) {
    if (!m || m.__htd !== 1) return;
    const name = path.basename(this.file);
    this.msgCount = (this.msgCount || 0) + 1;
    this.lastMsg = m.type;
    if (m.type === 'ready') { this.readyCount = (this.readyCount || 0) + 1; this.structuralPending = false; }
    if (m.seq && this._pending && this._pending.has(m.seq)) {
      const done = this._pending.get(m.seq);
      this._pending.delete(m.seq);
      done(m);
      return;
    }
    switch (m.type) {
      case 'openRedirect':
        if (this.redirect) {
          const f = path.resolve(this.pageDir, this.redirect);
          vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(f), VIEW_TYPE);
        }
        break;
      case 'ready': {
        if (m.redirect) { if (this._ready) { this._ready(); this._ready = null; } break; }
        // page scripts may strip the generated title="name : TClass" attributes at
        // runtime, so the probe gets the VCL class of every name from the IR
        const send = pi => {
          const classes = {};
          const events = {};
          if (pi && pi.ir) {
            for (const [n, node] of pi.ir.byName) {
              classes[n] = node.class;
              const ev = Object.keys(node.events || {});
              if (ev.length && node !== pi.ir.root) events[n] = ev;
            }
            if (pi.ir.formClass) classes['@form'] = pi.ir.formClass;
            if (pi.ir.root && pi.ir.root.events) events['@form'] = Object.keys(pi.ir.root.events);
          }
          this.post({ type: 'init', mode: this.mode, selectId: this.lastSelId, scroll: this.scroll, classes, events, zoom: this.zoom || 1,
            designHidden: Array.from(this.designHidden), designLocked: Array.from(this.designLocked), grid: this.hub.gridState(),
            wireMarks: this.wireMarks || null, showNames: !!this.showNames, dfmGhosts: this.dfmGhosts || null, tabOrder: this.tabOrder || null,
            snapLines: this.snapLines !== false, artboardDark: this.hub.artboardDark(), wheelZoom: this.hub.wheelZoom(), snapSpacing: this.hub.snapSpacing(),
            fitOnOpen: !!this.fitOnOpen });
          this.fitOnOpen = false;   // (once: a redraw keeps the zoom it has)
        };
        this.info.then(send, () => send(null));
        clearTimeout(this.diagTimer);
        this.diagTimer = setTimeout(() => this.post({ type: 'diag' }), 1500);
        break;
      }
      case 'netcheck': {
        const v = Array.isArray(m.violations) ? m.violations : [];
        const need = ['ws://127.0.0.1:9', 'http://127.0.0.1:9'];
        const allBlocked = need.every(u => v.some(x => x.indexOf(u) >= 0)) && m.fetch !== 'RESOLVED' && m.xhr !== 'LOADED';
        this.netcheck = { ok: allBlocked, violations: v, ws: m.ws, fetch: m.fetch, xhr: m.xhr };
        this.hub.log('[' + name + '] 網路封鎖檢查：' + (allBlocked ? '通過' : '⚠ 沒有全部擋下') +
          '（WebSocket/HTTP/XHR 試連 127.0.0.1:9；被擋 ' + v.length + ' 次：' + v.join('、') + '）');
        if (this.netcheckNotify) {
          this.netcheckNotify = false;
          if (allBlocked) vscode.window.showInformationMessage('網路封鎖檢查通過：預覽裡的 WebSocket、HTTP、XHR 全部被擋下（試連的是 127.0.0.1:9 這個關閉的埠）。');
          else vscode.window.showWarningMessage('網路封鎖檢查沒有通過，請看輸出面板「HTML 視覺設計」。');
        }
        break;
      }
      case 'diag':
        this.diag = m;
        this.hub.log('[' + name + '] 載入狀況：圖片 ' + m.imagesLoaded + '/' + m.images + '（壞 ' + m.imagesBroken + '）、樣式表 ' +
          m.sheets + '、script ' + m.scripts + '、背景 ' + m.bodyBg + (m.vscodeDefaultsLeft ? '、⚠ VS Code 預設樣式沒移除' : ''));
        break;
      case 'tree':
        this.treeData = Array.isArray(m.nodes) ? m.nodes : [];
        if (this.hub.active === this) this.hub.tree.set(this);
        // the page was (re)drawn -- e.g. after an undo: the difference list compares again
        if (this.dfmDiff && !this.dfmDiff.disposed) this.dfmDiff.scheduleRefresh();
        this.hub.scheduleGhosts(this);
        if (this._ready) { this._ready(); this._ready = null; }
        break;
      case 'select':
        // (a late answer of the page before a structural edit: the redrawn page decides)
        // ('initRoot': the page showed the form because the one asked for is not there yet -- keep asking for it)
        if (m.origin !== 'initRoot' && (!this.structuralPending || m.origin === 'init')) this.lastSelId = m.info ? m.info.id : null;
        this.hub.onSelect(this, m.info, m.origin);
        break;
      case 'toolbar':
        // the artboard toolbar (WPF): the grid is the hub's setting (every designer); snaplines this designer's
        if (m.cmd === 'toggleGrid') this.hub.cmdToggleGrid();
        else if (m.cmd === 'gridShow' || m.cmd === 'gridSnap') this.hub.cmdGridPart(m.cmd === 'gridSnap' ? 'snap' : 'show');
        else if (m.cmd === 'snap') { this.snapLines = m.on !== false; vscode.window.setStatusBarMessage('$(symbol-ruler) 對齊線：' + (this.snapLines ? '開' : '關'), 3000); }
        else if (m.cmd === 'artboard') this.hub.setArtboard(!!m.dark, this);
        break;
      case 'dblclick':
        this.hub.onDesignerDblClick(this, !!m.viewOnly);
        break;
      case 'edit':
        // one at a time: each edit is computed on the text the previous one left
        this.editChain = (this.editChain || Promise.resolve())
          .then(() => this.hub.applySourceEdit(this, m))
          .catch(e => this.hub.log('寫回失敗：' + (e && e.stack || e)))
          .then(() => { if (this.dfmDiff && !this.dfmDiff.disposed) this.dfmDiff.scheduleRefresh(); this.hub.scheduleGhosts(this); });
        break;
      case 'editRefused':
        this.hub.refuseEdit(this, String(m.why || '不能改'));
        break;
      case 'place':
        // the armed toolbox tool was put down on the surface
        this.hub.cmdPlace(this, m).catch(e => this.hub.log('放置失敗：' + (e && e.stack || e)));
        break;
      case 'placeCancel':
        this.armed = null;
        this.hub.toolboxBackToPointer();
        break;
      case 'textEdit':
        // F2's text box on the surface opened / closed: the designer's keys stay out while it is open
        this.textEditing = !!m.open;
        vscode.commands.executeCommand('setContext', 'ht9045Designer.textEditing', this.textEditing);
        break;
      case 'reparentDrop':
        // Alt held when a drag was let go (Blend): into the container under the pointer
        this.hub.cmdReparentDrop(this, m).catch(e => this.hub.log('換容器失敗：' + (e && e.stack || e)));
        break;
      case 'copyDrop':
        // Ctrl+drag on the surface: copies of the selection where it was let go
        this.hub.cmdCopyDrop(this, m).catch(e => this.hub.log('複製失敗：' + (e && e.stack || e)));
        break;
      case 'note':
        // a plain status line from the design surface (e.g. "選取了 12 個 TLabel")
        this.lastNote = String(m.text || '').slice(0, 200);
        vscode.window.setStatusBarMessage('$(info) ' + this.lastNote, 6000);
        break;
      case 'cmd': {
        // Del / Ctrl+C / Ctrl+X / Ctrl+V on the design surface (this designer has the focus)
        if (this.hub.active !== this) this.hub.setActive(this);
        const run = { delete: () => this.hub.cmdDelete(null), copy: () => this.hub.cmdCopy(), cut: () => this.hub.cmdCut(null), paste: () => this.hub.cmdPaste() }[m.cmd];
        if (run) Promise.resolve(run()).catch(e => this.hub.log('指令失敗（' + m.cmd + '）：' + (e && e.stack || e)));
        break;
      }
      case 'zoom':
        this.zoom = typeof m.zoom === 'number' ? m.zoom : 1;
        this.hub.saveZoom(this.file, this.zoom);   // (for Last Used: the next time this page opens)
        if (this.hub.active === this) this.hub.updateStatus();
        break;
      case 'scroll':
        this.scroll = { x: m.x || 0, y: m.y || 0 };
        break;
      case 'blocked':
        this.blocked.push({ dir: String(m.dir || ''), uri: String(m.uri || '') });
        this.hub.log('[' + name + '] 已擋下連線：' + m.dir + ' ' + m.uri);
        break;
      case 'pageError':
        this.errors.push({ msg: String(m.msg || ''), src: String(m.src || ''), line: m.line || 0 });
        this.hub.log('[' + name + '] 頁面 JS 錯誤：' + m.msg + '（' + m.src + ':' + m.line + '）');
        break;
      case 'showErrors':
        // the information bar's 看全部: every JS error of this page, in the output panel
        this.hub.log('[' + name + '] 這一頁的 JS 錯誤（' + this.errors.length + ' 個）：');
        this.errors.forEach((x, i) => this.hub.log('  ' + (i + 1) + '. ' + x.msg + '（' + path.basename(x.src || '') + ':' + x.line + '）'));
        this.hub.out.show(true);
        break;
      default:
        break;
    }
  }

  dispose() {
    if (this.disposed) return;
    this.disposed = true;
    clearTimeout(this.renderTimer);
    clearTimeout(this.diagTimer);
    clearTimeout(this.ghostTimer);
    this.dfmGhosts = null;
    for (const s of this.subs) { try { s.dispose(); } catch (e) { /* ignore */ } }
    // its side tabs only make sense with it
    for (const p of [this.overview, this.dfmDiff]) { if (p && !p.disposed) { try { p.panel.dispose(); } catch (e) { /* ignore */ } } }
    this.hub.removeDesigner(this);
  }
}
Designer.seq = 0;

// ---------------------------------------------------------------------------
/**
 * ht9045-golden:/d:/HT9045/HT9011UC_Code_V…/x.cpp -> that file, decoded from Big5 and
 * handed to VS Code as UTF-8 with a BOM (the BOM wins over any files.encoding the user
 * has set). Registered with isReadonly, and every write path refuses.
 */
class GoldenFs {
  constructor() {
    this._em = new vscode.EventEmitter();
    this.onDidChangeFile = this._em.event;
  }
  real(uri) { return uri.with({ scheme: 'file' }).fsPath; }
  watch() { return new vscode.Disposable(() => {}); }
  stat(uri) {
    let st;
    try { st = fs.statSync(this.real(uri)); } catch (e) { throw vscode.FileSystemError.FileNotFound(uri); }
    return {
      type: st.isDirectory() ? vscode.FileType.Directory : vscode.FileType.File,
      ctime: st.ctimeMs, mtime: st.mtimeMs, size: st.size,
      permissions: vscode.FilePermission ? vscode.FilePermission.Readonly : undefined,
    };
  }
  readFile(uri) {
    let buf;
    try { buf = fs.readFileSync(this.real(uri)); } catch (e) { throw vscode.FileSystemError.FileNotFound(uri); }
    return Buffer.concat([Buffer.from([0xef, 0xbb, 0xbf]), Buffer.from(decodeBig5(buf), 'utf8')]);
  }
  readDirectory() { return []; }
  createDirectory(uri) { throw vscode.FileSystemError.NoPermissions(uri); }
  writeFile(uri) { throw vscode.FileSystemError.NoPermissions(uri); }
  delete(uri) { throw vscode.FileSystemError.NoPermissions(uri); }
  rename(uri) { throw vscode.FileSystemError.NoPermissions(uri); }
}

// ---------------------------------------------------------------------------
/**
 * Above every C++ / BCB6 event handler that a web page's control uses:
 *   $(preview) 設計檢視：Setup.HotPlate.html › spbSave.OnClick
 * The index is built in the background on first use; the lenses appear when it is.
 */
class HandlerLens {
  constructor(hub) {
    this.hub = hub;
    this._em = new vscode.EventEmitter();
    this.onDidChangeCodeLenses = this._em.event;
    this.building = false;
  }

  provideCodeLenses(doc) {
    if (this.hub.cfg().get('codeLens') === false) return [];
    const idx = this.hub.reverseIndex();
    const widx = this.hub.webCmdIndex();
    if (!idx && !widx) return [];
    if ((idx && !idx.built) || (widx && !widx.built)) {
      if (!this.building) {
        this.building = true;
        setTimeout(() => {
          try {
            if (idx) { idx.build(); this.hub.log('反查索引完成：' + idx.pages.length + ' 頁，' + idx.handlers.size + ' 個事件函式（' + idx.buildMs + ' ms）'); }
            if (widx) { widx.build(); this.hub.log('網頁命令索引完成：' + widx.files + ' 個檔，' + widx.sends.size + ' 種命令（' + widx.buildMs + ' ms）'); }
          } catch (e) { this.hub.log('索引失敗：' + e); }
          this.building = false;
          this._em.fire();
        }, 50);
      }
      return [];
    }
    const text = doc.getText();
    const out = [];
    if (widx) this.commandLenses(doc, text, widx, out);
    if (widx) this.fieldLenses(doc, text, widx, out);
    if (widx) this.tagLenses(doc, text, widx, out);
    if (!idx) return out;
    const re = /\b(T[A-Za-z_]\w*)\s*::\s*([A-Za-z_]\w*)\s*\(/g;
    let m;
    while ((m = re.exec(text)) && out.length < 400) {
      if (!idx.hasClass(m[1])) continue;
      const entries = idx.handler(m[1], m[2]);
      if (!entries.length) continue;
      const lineStart = text.lastIndexOf('\n', m.index) + 1;
      if (text.slice(lineStart, m.index).includes('//')) continue;
      if (!lex.isDefinitionAt(text, m.index + m[0].length - 1)) continue;
      const pos = doc.positionAt(m.index);
      const range = new vscode.Range(pos.line, 0, pos.line, 0);
      const pages = Array.from(new Set(entries.map(e => path.basename(e.page))));
      const first = entries[0];
      let title = '$(preview) 設計檢視：' + pages[0] + ' › ' + (first.node === '@form' ? '表單' : first.node) + '.' + first.event;
      if (entries.length > 1) title += ' 等 ' + entries.length + ' 個元件' + (pages.length > 1 ? '（' + pages.length + ' 頁）' : '');
      out.push(new vscode.CodeLens(range, { title, command: 'ht9045Designer.showInDesigner', arguments: [entries], tooltip: '打開網頁的設計檢視並選取用這個函式的元件' }));
    }
    return out;
  }

  /** Above a line that publishes "machine.state": the web control that shows it. */
  tagLenses(doc, text, widx, out) {
    const re = /"([a-z][\w]*\.[\w.]+)"/g;
    const seen = new Set();
    let m;
    while ((m = re.exec(text)) && out.length < 900) {
      if (!widx.tags.has(m[1])) continue;
      const lineStart = text.lastIndexOf('\n', m.index) + 1;
      const before = text.slice(lineStart, m.index);
      if (before.includes('//') || /(==|!=)\s*$/.test(before)) continue;   // comments; command compares
      const rows = widx.tagControls(m[1]).filter(r => r.pages.length);
      if (!rows.length) continue;
      const pos = doc.positionAt(m.index);
      const key = pos.line + '|' + m[1];
      if (seen.has(key)) continue;
      seen.add(key);
      const entries = [];
      const had = new Set();
      for (const r of rows) {
        for (const p of r.pages) {
          const k = p.toLowerCase() + '|' + r.id;
          if (had.has(k)) continue;
          had.add(k);
          entries.push({ page: p, node: r.id, path: 'tag ' + r.tag + '（顯示為 ' + r.prop + '）' });
        }
      }
      let title = '$(eye) 網頁顯示 ' + m[1] + '：' + path.basename(entries[0].page) + ' › ' + entries[0].node;
      if (entries.length > 1) title += ' 等 ' + entries.length + ' 處';
      out.push(new vscode.CodeLens(new vscode.Range(pos.line, 0, pos.line, 0), {
        title, command: 'ht9045Designer.showInDesigner', arguments: [entries], tooltip: '打開網頁的設計檢視並選取顯示這個標籤的元件',
      }));
    }
  }

  /** Above ReadIniData(…, "Hotplate Form", "X Start", …): the web control that edits it. */
  fieldLenses(doc, text, widx, out) {
    const re = /"([^"\\\n]{1,80})"\s*,\s*"([^"\\\n]{1,80})"/g;
    const seen = new Set();
    let m;
    while ((m = re.exec(text)) && out.length < 800) {
      re.lastIndex = m.index + 1 + m[1].length;     // pairs can chain: "a", "b", "c"
      const rows = widx.fieldControls(m[1], m[2]).filter(r => r.pages.length);
      if (!rows.length) continue;
      const lineStart = text.lastIndexOf('\n', m.index) + 1;
      if (text.slice(lineStart, m.index).includes('//')) continue;
      const pos = doc.positionAt(m.index);
      const key = pos.line + '|' + m[1] + '|' + m[2];
      if (seen.has(key)) continue;
      seen.add(key);
      const entries = [];
      const had = new Set();
      for (const r of rows) {
        for (const p of r.pages) {
          const k = p.toLowerCase() + '|' + r.id;          // hand-written + generated wire map the same control
          if (had.has(k)) continue;
          had.add(k);
          entries.push({ page: p, node: r.id, path: '[' + r.section + '] ' + r.key + (r.note ? '　' + r.note : '') });
        }
      }
      const pages = Array.from(new Set(entries.map(e => path.basename(e.page))));
      let title = '$(symbol-field) 網頁欄位 [' + m[1] + '] ' + m[2] + '：' + pages[0] + ' › ' + entries[0].node;
      if (entries.length > 1) title += ' 等 ' + entries.length + ' 處';
      out.push(new vscode.CodeLens(new vscode.Range(pos.line, 0, pos.line, 0), {
        title, command: 'ht9045Designer.showInDesigner', arguments: [entries], tooltip: '打開網頁的設計檢視並選取編輯這個欄位的元件',
      }));
    }
  }

  /** Above a server dispatch (== "io.btnPanelClick"): where the web sends that command. */
  commandLenses(doc, text, widx, out) {
    const re = /(==|\bcase)\s*"([a-z][\w]*\.[\w.:-]+)"/g;
    const seen = new Set();
    let m;
    while ((m = re.exec(text)) && out.length < 600) {
      const cmd = m[2];
      const lineStart = text.lastIndexOf('\n', m.index) + 1;
      if (text.slice(lineStart, m.index).includes('//')) continue;
      const pos = doc.positionAt(m.index);
      const key = pos.line + '|' + cmd;
      if (seen.has(key)) continue;
      seen.add(key);
      const hits = widx.senders(cmd);
      if (!hits.length) continue;
      const pages = Array.from(new Set([].concat(...hits.map(h => h.pages)).map(p => path.basename(p))));
      let title = '$(globe) 網頁送出 ' + cmd + '：' + path.basename(hits[0].file) + ':' + hits[0].line;
      if (hits.length > 1) title += ' 等 ' + hits.length + ' 處';
      if (pages.length) title += '（' + pages.slice(0, 2).join('、') + (pages.length > 2 ? ' 等 ' + pages.length + ' 頁' : '') + '）';
      out.push(new vscode.CodeLens(new vscode.Range(pos.line, 0, pos.line, 0), {
        title, command: 'ht9045Designer.openWebSenders', arguments: [cmd, hits], tooltip: '跳到網頁送出這個命令的地方',
      }));
    }
  }
}

// ---------------------------------------------------------------------------
function iconFor(n) {
  if (n.isForm) return 'window';
  const c = (n.cls || '').toLowerCase();
  const t = n.tag;
  if (/speedbutton|bitbtn|button|btnpanel/.test(c) || t === 'button') return 'symbol-event';
  if (/edit|memo|maskedit/.test(c) || t === 'input' || t === 'textarea') return 'symbol-string';
  if (/checkbox/.test(c)) return 'check';
  if (/radio/.test(c)) return 'circle-large-outline';
  if (/combobox|listbox/.test(c) || t === 'select') return 'list-selection';
  if (/groupbox/.test(c) || t === 'fieldset') return 'group-by-ref-type';
  if (/pagecontrol/.test(c)) return 'files';
  if (/tabsheet/.test(c)) return 'file';
  if (/image/.test(c) || t === 'img') return 'file-media';
  if (/grid/.test(c) || t === 'table') return 'table';
  if (/led|lamp/.test(c)) return 'lightbulb';
  if (/label|statictext/.test(c)) return 'symbol-text';
  if (/panel|scrollbox|bevel|shape/.test(c)) return 'symbol-namespace';
  return 'symbol-field';
}

class ComponentTree {
  constructor(hub) {
    this.hub = hub;
    this._em = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._em.event;
    this.d = null;
    this.roots = [];
    this.byKey = new Map();
  }

  set(d) {
    this.d = d;
    this.roots = [];
    this.byKey = new Map();
    const rows = d ? d.treeData : [];
    for (const r of rows) {
      const n = {
        key: r[0], pk: r[1], id: r[2], tag: r[3], cls: r[4], cap: r[5], hidden: !!r[6], isForm: !!r[7], runHidden: !!r[8],
        children: [], parent: null, depth: 0, tid: '',
      };
      this.byKey.set(n.key, n);
    }
    const seen = new Map();
    for (const n of this.byKey.values()) {
      const p = n.pk ? this.byKey.get(n.pk) : null;
      if (p) { n.parent = p; p.children.push(n); } else this.roots.push(n);
      const base = n.isForm ? '@form' : n.id;
      const c = (seen.get(base) || 0) + 1;
      seen.set(base, c);
      n.tid = (d ? d.id : 0) + ':' + base + '#' + c;
    }
    const setDepth = (list, dep) => { for (const n of list) { n.depth = dep; setDepth(n.children, dep + 1); } };
    setDepth(this.roots, 0);
    this._em.fire();
  }

  refresh() { this._em.fire(); }

  getChildren(n) { return n ? n.children : this.roots; }
  getParent(n) { return n.parent || undefined; }

  getTreeItem(n) {
    const S = vscode.TreeItemCollapsibleState;
    const label = n.isForm ? (this.d ? this.d.formLabel : '表單') : n.id;
    const item = new vscode.TreeItem(label, n.children.length ? (n.depth < 2 ? S.Expanded : S.Collapsed) : S.None);
    item.id = n.tid;
    if (!n.cls && !n.isForm && this.d && this.d.ir) {
      const node = this.d.ir.byName.get(n.id);
      if (node) n.cls = node.class;
    }
    const dh = !n.isForm && !!(this.d && this.d.designHidden && this.d.designHidden.has(n.id));
    const lk = !n.isForm && !!(this.d && this.d.designLocked && this.d.designLocked.has(n.id));
    const desc = [];
    if (lk) desc.push('🔒');
    if (!n.isForm) desc.push(n.cls || '<' + n.tag + '>');
    if (n.cap) desc.push('「' + n.cap + '」');
    if (dh) desc.push('（設計時隱藏）');
    else if (n.runHidden) desc.push('（執行時隱藏）');
    else if (n.hidden) desc.push('（隱藏）');
    item.description = desc.join(' ');
    item.tooltip = label + (n.cls ? ' : ' + n.cls : '') + '\n<' + n.tag + '>' + (n.cap ? '\n' + n.cap : '') +
      (dh ? '\n在設計檢視暫時隱藏（按右邊的眼睛顯示回來；原始碼沒改）'
        : n.runHidden ? '\n執行時隱藏（Visible=False，或頁面依機種／選配決定）；設計時照樣顯示，像 BCB6 的設計畫面'
        : n.hidden ? '\n目前不可見（在未顯示的分頁裡，或被隱藏）' : '') +
      (lk ? '\n鎖定：在設計畫面不能移動、改大小、刪除（裡面的元件也一樣）；按右邊的鎖頭解除' : '');
    item.iconPath = new vscode.ThemeIcon(dh ? 'eye-closed' : iconFor(n), dh || n.hidden ? new vscode.ThemeColor('disabledForeground') : undefined);
    item.command = { command: 'ht9045Designer.selectKey', title: '選取', arguments: [n.key] };
    // 'component' + '.hidden' / '.locked': the eye and the lock pick their icon from it; the form has neither
    item.contextValue = n.isForm ? 'componentForm' : 'component' + (dh ? '.hidden' : '') + (lk ? '.locked' : '');
    return item;
  }

  reveal(key) {
    const n = this.byKey.get(key);
    if (!n || !this.hub.treeView.visible) return;
    // the page selected it: mirror that in the tree without the tree answering back
    this.revealing = true;
    const done = () => { setTimeout(() => { this.revealing = false; }, 50); };
    this.hub.treeView.reveal(n, { select: true, focus: false }).then(done, done);
  }
}

// ---------------------------------------------------------------------------
/** 頁面: every page of the web folder by group -- a click opens it in the designer. */
class PageTree {
  constructor(hub) {
    this.hub = hub;
    this._em = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._em.event;
    this.groups = null;
    this.byFile = new Map();
    this.webRoot = null;
    // 搜尋頁面 (the box above this list): only what has every word, and the components that have them
    this.query = '';
    this.found = null;
    this.gen = 0;
  }

  /**
   * Filter the list by `q` ('' = every page again): the pages whose name / title has every word, and
   * under each the components that have them (name, class, text). keep: the same query again (a page
   * changed), the groups stay as they were opened. Returns what was found, for the box.
   */
  setFilter(q, keep) {
    const query = String(q || '').trim();
    if (!keep) this.gen++;
    this.query = query;
    const t0 = Date.now();
    const f = query ? pagesearch.filterPages(this.root(), query, 2000) : null;
    this.found = null;
    this.foundByFile = new Map();
    if (f) {
      const PER_PAGE = 100;
      const open = f.hits <= 200;
      const nodes = f.groups.map(g => {
        const gn = { kind: 'group', name: g.name, children: [], parent: null, filtered: true };
        gn.children = g.pages.map(p => {
          const pn = Object.assign({}, p, { kind: 'page', parent: gn, filtered: true, open, children: [] });
          pn.children = p.hits.slice(0, PER_PAGE).map(h => ({ kind: 'hit', parent: pn, hit: h }));
          if (p.hits.length > PER_PAGE) pn.children.push({ kind: 'more', parent: pn, n: p.hits.length - PER_PAGE });
          this.foundByFile.set(p.file.toLowerCase(), pn);
          return pn;
        });
        return gn;
      });
      const best = f.groups.length ? (pagesearch.searchPages(this.root(), query, 1)[0] || null) : null;
      let firstPage = null;
      for (const g of nodes) { firstPage = firstPage || g.children.find(p => p.nameHit) || null; }
      this.found = { query, words: f.words, nodes, pages: f.pages, hits: f.hits, capped: f.capped, best,
        firstPage: firstPage || (nodes.length ? nodes[0].children[0] : null) };
    }
    const ms = Date.now() - t0;
    const s = this.summary(ms);
    if (this.hub.pageView) this.hub.pageView.message = s.text || undefined;
    vscode.commands.executeCommand('setContext', 'ht9045Designer.pageFilterOn', !!query);
    if (query && !keep) this.hub.log('搜尋頁面「' + query + '」：' + s.pages + ' 頁、' + s.hits + ' 個元件（' + ms + ' ms）');
    this._em.fire();
    return s;
  }

  /** What the filter found, in words (the list's message, the box's line under it). */
  summary(ms) {
    const f = this.found;
    if (!this.query) return { query: '', pages: 0, hits: 0, text: '', none: false, ms: ms || 0 };
    if (!f || !f.pages) return { query: this.query, pages: 0, hits: 0, text: '找不到「' + this.query + '」', none: true, ms: ms || 0 };
    return { query: this.query, pages: f.pages, hits: f.hits, capped: f.capped, none: false, ms: ms || 0,
      text: '「' + this.query + '」：' + f.pages + ' 頁' + (f.hits ? '、' + f.hits + (f.capped ? '+' : '') + ' 個元件' : '') };
  }

  /**
   * Enter in the box: a component of exactly that name (its page opens, it is selected); else a page whose
   * name / title has the words; else the best component found.
   */
  openFirst() {
    const f = this.found;
    if (!f) return null;
    const openPage = p => vscode.commands.executeCommand('vscode.openWith', vscode.Uri.file(p.file), VIEW_TYPE);
    if (f.best && f.best.exact) return this.hub.openPageAt(f.best.file, f.best.id);
    if (f.firstPage && f.firstPage.nameHit) return openPage(f.firstPage);
    if (f.best) return this.hub.openPageAt(f.best.file, f.best.id);
    if (f.firstPage) return openPage(f.firstPage);
    return null;
  }

  /** The web folder: the active designer's, else the one the settings / workspace name. */
  root() {
    if (this.hub.active && this.hub.active.r.webRoot) return this.hub.active.r.webRoot;
    return roots.resolveRoots(null, this.hub.wsFolders(), this.hub.over()).webRoot || null;
  }

  build() {
    const webRoot = this.root();
    if (this.groups && webRoot === this.webRoot) return this.groups;
    this.webRoot = webRoot;
    this.byFile = new Map();
    this.groups = pagelist.listPages(webRoot).map(g => {
      const gn = { kind: 'group', name: g.name, children: [], parent: null };
      gn.children = g.pages.map(p => {
        const pn = Object.assign({ kind: 'page', parent: gn }, p);
        this.byFile.set(p.file.toLowerCase(), pn);
        return pn;
      });
      return gn;
    });
    if (webRoot) {
      // a page added / removed / renamed: the list follows (debounced)
      this.hub.watch(webRoot, '**/*.html', () => {
        clearTimeout(this._t);
        this._t = setTimeout(() => this.refresh(), 500);
      });
    }
    return this.groups;
  }

  refresh() {
    this.groups = null;
    if (this.query) { const s = this.setFilter(this.query, true); this.hub.pageSearch.show(s); return; }
    this._em.fire();
  }
  /** The open / active marks changed (no need to read the folder again). */
  mark() { this._em.fire(); }
  /** How many 頁面檢查 problems this page has in the Problems panel (0 = none or not checked). */
  problemsOf(file) {
    const c = this.hub.lintDiag;
    if (!c) return 0;
    const l = c.get(vscode.Uri.file(file));
    // (errors and warnings; the "differs from the .dfm" notes are information, not problems)
    return l ? l.filter(dg => dg.severity !== vscode.DiagnosticSeverity.Information).length : 0;
  }

  getChildren(n) {
    if (n) return n.children || [];
    if (this.found) return this.found.nodes;
    // a query that found nothing: an empty list (the message above it says so)
    return this.query ? [] : this.build();
  }
  getParent(n) { return n.parent || undefined; }

  getTreeItem(n) {
    const S = vscode.TreeItemCollapsibleState;
    // filtered: the ids change with each query, so every group / page opens as the query wants
    const q = n.filtered || n.kind === 'hit' || n.kind === 'more' ? 'q' + this.gen + ':' : '';
    const words = this.found ? this.found.words : [];
    if (n.kind === 'hit') {
      const h = n.hit;
      const item = new vscode.TreeItem({ label: h.id, highlights: pagesearch.highlightsOf(h.id, words) }, S.None);
      item.id = q + 'hit:' + h.file.toLowerCase() + '#' + h.id + '@' + h.line;
      item.description = (h.cls || '') + (h.caption ? (h.cls ? '　' : '') + '「' + h.caption.slice(0, 40) + (h.caption.length > 40 ? '…' : '') + '」' : '');
      item.tooltip = h.page + ':' + h.line + '\n' + h.id + (h.cls ? ' : ' + h.cls : '') + (h.caption ? '\n「' + h.caption + '」' : '') +
        '\n\n點一下：開這一頁並選取它';
      item.iconPath = new vscode.ThemeIcon(h.how === 'class' ? 'symbol-class' : h.how === 'text' ? 'symbol-text' : 'symbol-field');
      item.command = { command: 'ht9045Designer.openPageAt', title: '開啟並選取', arguments: [h.file, h.id] };
      item.contextValue = 'pageHit';
      return item;
    }
    if (n.kind === 'more') {
      const item = new vscode.TreeItem('…還有 ' + n.n + ' 個（多打幾個字縮小範圍）', S.None);
      item.id = q + 'more:' + n.parent.file.toLowerCase();
      return item;
    }
    if (n.kind === 'group') {
      const act = this.hub.active;
      const has = act && n.children.some(p => web.samePath(p.file, act.file));
      const item = new vscode.TreeItem(n.name, n.filtered || has ? S.Expanded : S.Collapsed);
      item.id = q + 'grp:' + n.name;
      const probs = n.children.reduce((s, p) => s + this.problemsOf(p.file), 0);
      item.description = String(n.children.length) + (probs ? '　⚠ ' + probs : '');
      item.iconPath = new vscode.ThemeIcon(n.name === pagelist.ROOT ? 'root-folder' : 'symbol-folder');   // (not 'folder': see SolutionTree's dir)
      item.contextValue = 'pageGroup';
      return item;
    }
    const act = this.hub.active;
    const isActive = !!(act && web.samePath(act.file, n.file));
    const isOpen = Array.from(this.hub.designers).some(d => web.samePath(d.file, n.file));
    const nHits = n.filtered ? n.hits.length : 0;
    const item = new vscode.TreeItem(n.filtered ? { label: n.label, highlights: pagesearch.highlightsOf(n.label, words) } : n.label,
      nHits ? (n.open ? S.Expanded : S.Collapsed) : S.None);
    item.id = q + 'page:' + n.file.toLowerCase();
    const desc = [];
    if (nHits) desc.push(nHits + ' 個元件');
    else if (n.filtered && n.textHit) desc.push('頁面上的文字');
    const probs = this.problemsOf(n.file);
    if (probs) desc.push('⚠ ' + probs);
    if (isActive) desc.push('● 目前');
    else if (isOpen) desc.push('○ 已開啟');
    if (n.redirect) desc.push('→ ' + n.redirect);
    else if (n.title) desc.push(n.title);
    item.description = desc.join('  ');
    item.tooltip = n.rel + (n.title ? '\n' + n.title : '') + (n.redirect ? '\n轉址頁 → ' + n.redirect : '') +
      (probs ? '\n頁面檢查：' + probs + ' 個問題（看「問題」面板）' : '') + '\n\n點一下：用設計檢視開啟';
    item.iconPath = new vscode.ThemeIcon(isActive ? 'eye' : n.redirect ? 'arrow-right' : 'file-code',
      isActive ? new vscode.ThemeColor('charts.orange') : undefined);
    item.command = { command: 'ht9045Designer.openPageFile', title: '開啟', arguments: [n.file] };
    item.contextValue = 'page';
    return item;
  }

  /** Select the active page in the list (its group opens). */
  revealActive() {
    const act = this.hub.active;
    const tv = this.hub.pageView;
    if (!act || !tv || !tv.visible) return;
    this.build();
    // after the list has drawn what it was just told (a new filter, the first load): revealing an item it
    // does not have yet is an error in VS Code's log ("No tree item with id ...")
    clearTimeout(this._revealT);
    this._revealT = setTimeout(() => {
      const map = this.query ? this.foundByFile : this.byFile;
      const n = map && act.file && map.get(act.file.toLowerCase());
      if (n && tv.visible) tv.reveal(n, { select: true, focus: false, expand: true }).then(() => {}, () => {});
    }, 250);
  }
}

// ---------------------------------------------------------------------------
/** The bulb on a cut attribute (頁面檢查): its inner quotes become the other kind; or the whole page at once. */
class LintFixes {
  constructor(hub) { this.hub = hub; }

  provideCodeActions(document, range, context) {
    const out = [];
    // "differs from the .dfm": the .dfm's size / alignment for this one, or the whole page
    const gapsHere = (context.diagnostics || []).filter(dg => dg.source === LINT_SOURCE && dg.code === 'dfm-gap');
    if (gapsHere.length && this.hub) {
      const text = document.getText();
      const ir = this.hub.irForFile(document.uri.fsPath, text);
      const gaps = ir ? pagelint.dfmGaps(text, ir) : [];
      for (const dg of gapsHere) {
        const g = gaps.find(x => x.at === document.offsetAt(dg.range.start));
        if (!g) continue;
        const a = new vscode.CodeAction('改成 BCB6 .dfm 的大小與對齊（' + g.id + '）', vscode.CodeActionKind.QuickFix);
        a.edit = new vscode.WorkspaceEdit();
        a.edit.replace(document.uri, new vscode.Range(document.positionAt(g.fix.at), document.positionAt(g.fix.at + g.fix.len)), g.fix.repl);
        a.diagnostics = [dg];
        a.isPreferred = true;
        out.push(a);
      }
      const allG = new vscode.CodeAction('這一頁所有跟 DFM 不同的 Label／Panel 都改成 DFM 的', vscode.CodeActionKind.QuickFix);
      allG.command = { command: 'ht9045Designer.lintFixDfmGaps', title: allG.title, arguments: [document.uri] };
      out.push(allG);
    }
    const ours = (context.diagnostics || []).filter(dg => dg.source === LINT_SOURCE && dg.code === 'quote-cut');
    for (const dg of ours) {
      const value = document.getText(dg.range);
      const start = document.offsetAt(dg.range.start);
      const q = start > 0 ? document.getText().charAt(start - 1) : '"';
      const other = q === '"' ? "'" : '"';
      const a = new vscode.CodeAction('把裡面的 ' + q + ' 改成 ' + other + '（修正被切斷的屬性）', vscode.CodeActionKind.QuickFix);
      a.edit = new vscode.WorkspaceEdit();
      a.edit.replace(document.uri, dg.range, value.split(q).join(other));
      a.diagnostics = [dg];
      a.isPreferred = true;
      out.push(a);
    }
    if (ours.length) {
      const all = new vscode.CodeAction('修正這一頁所有被切斷的屬性', vscode.CodeActionKind.QuickFix);
      all.command = { command: 'ht9045Designer.lintFixAll', title: '修正這一頁所有被切斷的屬性', arguments: [document.uri] };
      out.push(all);
    }
    return out;
  }
}

// ---------------------------------------------------------------------------
/** Drag and drop in the component tree (WPF document outline): the names travel, the drop moves them. */
const TREE_MIME = 'application/vnd.code.tree.ht9045designer.components';
class ComponentDnd {
  constructor(hub) {
    this.hub = hub;
    this.dragMimeTypes = [TREE_MIME];
    this.dropMimeTypes = [TREE_MIME];
  }
  handleDrag(source, dataTransfer) {
    const ids = (source || []).filter(n => n && !n.isForm && n.id).map(n => n.id);
    if (ids.length) dataTransfer.set(TREE_MIME, new vscode.DataTransferItem(ids));
  }
  async handleDrop(target, dataTransfer) {
    const item = dataTransfer.get(TREE_MIME);
    if (!item) return null;
    let ids = item.value;
    if (typeof ids === 'string') { try { ids = JSON.parse(ids); } catch (e) { ids = []; } }
    if (!Array.isArray(ids) || !ids.length || !ids.every(x => typeof x === 'string')) return null;
    return this.hub.cmdReparent(ids, target || null);
  }
}

// ---------------------------------------------------------------------------
/**
 * WPF / WinForms Toolbox: "Pointer" is the first item -- a click on it puts the picked-up tool
 * back (the surface is for selecting again), and the toolbox goes back to it after a tool is put down.
 */
const POINTER = { cls: '@pointer', label: '指標', note: '回到選取（放下拿起的工具）', icon: 'inspect' };

/** WPF: the caret inside a handler's body (the first statement, or just after an empty body's brace), or null. */
function bodyPos(doc, line0) {
  const b = cppstub.bodyStart(doc.getText(), line0);
  return b ? new vscode.Position(b.line, b.col) : null;
}

/** 工具箱 (WPF Toolbox): the component classes a page can get; a click adds one. */
class ToolboxTree {
  getChildren(n) { return n ? [] : [POINTER, ...toolbox.ITEMS]; }
  // (reveal -- back to Pointer -- asks for the parent: every item is at the top)
  getParent() { return null; }
  getTreeItem(it) {
    if (it === POINTER) {
      const p = new vscode.TreeItem(it.label, vscode.TreeItemCollapsibleState.None);
      p.id = 'tool:@pointer';
      p.description = it.note;
      p.tooltip = '指標（WPF 工具箱的 Pointer）：放下拿起的工具，設計畫面回到選取（同 Esc）。\n放好一個元件後工具箱也會回到這一項。';
      p.iconPath = new vscode.ThemeIcon(it.icon);
      p.command = { command: 'ht9045Designer.toolboxArm', title: '指標', arguments: [it.cls] };
      p.contextValue = 'toolPointer';
      return p;
    }
    const item = new vscode.TreeItem(it.label, vscode.TreeItemCollapsibleState.None);
    item.id = 'tool:' + it.cls;
    item.description = it.note + '　' + it.cls;
    item.tooltip = it.cls + '：' + it.note + '（名稱 ' + it.base + '1、' + it.base + '2…）\n\n' +
      '點一下＝拿起這個工具：到設計畫面上點一下放在那裡，或拖一個框（連大小）；Esc 取消\n' +
      '連點兩下＝直接新增：選了 Panel／GroupBox／分頁／表單 → 放進去（左上 8,8）；選了其他元件 → 放在它下面\n\n寫進 HTML 原始碼，Ctrl+Z 復原。';
    item.iconPath = new vscode.ThemeIcon(it.icon);
    item.command = { command: 'ht9045Designer.toolboxArm', title: '放置', arguments: [it.cls] };
    item.contextValue = 'tool';
    return item;
  }
}

// ---------------------------------------------------------------------------
/** 接線總覽 tab (one per designer; re-running the command refreshes it). */
class OverviewPanel {
  static show(hub, d, data) {
    if (d.overview && !d.overview.disposed) { d.overview.set(data); d.overview.panel.reveal(undefined, true); return d.overview; }
    d.overview = new OverviewPanel(hub, d, data);
    return d.overview;
  }

  constructor(hub, d, data) {
    this.hub = hub;
    this.d = d;
    this.data = data;
    this.disposed = false;
    const media = vscode.Uri.joinPath(hub.ctx.extensionUri, 'media');
    this.panel = vscode.window.createWebviewPanel('ht9045Designer.overview', '接線總覽：' + path.basename(d.file),
      { viewColumn: vscode.ViewColumn.Beside, preserveFocus: true },
      { enableScripts: true, enableCommandUris: false, localResourceRoots: [media], retainContextWhenHidden: true });
    const w = this.panel.webview;
    const nonce = crypto.randomBytes(16).toString('base64');
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src ' + w.cspSource + '; script-src \'nonce-' + nonce + '\';">' +
      '<link rel="stylesheet" href="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'overview.css')) + '"></head><body><div id="root"></div>' +
      '<script nonce="' + nonce + '" src="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'overview.js')) + '"></script></body></html>';
    w.onDidReceiveMessage(m => this.onMessage(m));
    this.panel.onDidDispose(() => { this.disposed = true; if (d.overview === this) d.overview = null; });
  }

  set(data) {
    this.data = data;
    this.panel.webview.postMessage({ type: 'data', data });
  }

  onMessage(m) {
    switch (m && m.type) {
      case 'ready': this.panel.webview.postMessage({ type: 'data', data: this.data }); break;
      case 'select':
        if (!this.d.disposed) {
          this.d.panel.reveal(this.d.panel.viewColumn, true);
          this.d.post({ type: 'selectId', id: String(m.id || ''), origin: 'overview' });
        }
        break;
      case 'open':
        this.hub.openTarget({ kind: m.kind === 'golden' ? 'golden' : m.kind === 'port' ? 'port' : 'web', file: String(m.file), line: m.line | 0, col: m.col | 0 }, this.d);
        break;
      default: break;
    }
  }
}

// ---------------------------------------------------------------------------
/** 與 DFM 的差異 tab (one per designer; refreshed after every edit). */
class DfmDiffPanel {
  static show(hub, d, data, quiet) {
    if (d.dfmDiff && !d.dfmDiff.disposed) {
      d.dfmDiff.set(data);
      if (!quiet) d.dfmDiff.panel.reveal(undefined, true);
      return d.dfmDiff;
    }
    if (quiet) return null;
    d.dfmDiff = new DfmDiffPanel(hub, d, data);
    return d.dfmDiff;
  }

  constructor(hub, d, data) {
    this.hub = hub;
    this.d = d;
    this.data = data;
    this.disposed = false;
    const media = vscode.Uri.joinPath(hub.ctx.extensionUri, 'media');
    this.panel = vscode.window.createWebviewPanel('ht9045Designer.dfmDiff', '與 DFM 的差異：' + path.basename(d.file),
      { viewColumn: vscode.ViewColumn.Beside, preserveFocus: true },
      { enableScripts: true, enableCommandUris: false, localResourceRoots: [media], retainContextWhenHidden: true });
    const w = this.panel.webview;
    const nonce = crypto.randomBytes(16).toString('base64');
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src ' + w.cspSource + '; script-src \'nonce-' + nonce + '\';">' +
      '<link rel="stylesheet" href="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'overview.css')) + '"></head><body><div id="root"></div>' +
      '<script nonce="' + nonce + '" src="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'dfmdiff.js')) + '"></script></body></html>';
    w.onDidReceiveMessage(m => this.onMessage(m));
    this.panel.onDidDispose(() => { this.disposed = true; clearTimeout(this.timer); if (d.dfmDiff === this) d.dfmDiff = null; });
  }

  set(data) {
    this.data = data;
    this.panel.webview.postMessage({ type: 'data', data });
  }

  scheduleRefresh() {
    clearTimeout(this.timer);
    this.timer = setTimeout(() => { if (!this.disposed && !this.d.disposed) this.hub.cmdDfmDiff(this.d, true); }, 400);
  }

  onMessage(m) {
    if (this.d.disposed) return;
    switch (m && m.type) {
      case 'ready': this.panel.webview.postMessage({ type: 'data', data: this.data }); break;
      case 'refresh': this.hub.cmdDfmDiff(this.d, true); break;
      case 'ghosts':
        // the design surface's DFM frames on / off; the button then says which
        return this.hub.cmdDfmGhosts(undefined, this.d).then(r => this.hub.cmdDfmDiff(this.d, true).then(() => r));
      case 'select':
        this.d.panel.reveal(this.d.panel.viewColumn, true);
        this.d.post({ type: 'selectId', id: String(m.id || ''), origin: 'dfmdiff' });
        break;
      case 'reset': {
        // write one DFM value back: the probe changes the page and reports the edit
        const r = m.reset || {};
        const id = String(m.id || '');
        if (!id || !/^(setLayout|setCaption|setLook)$/.test(r.type)) break;
        const msg = { type: r.type, id };
        for (const k of ['left', 'top', 'width', 'height']) if (typeof r[k] === 'number') msg[k] = r[k];
        if (r.type === 'setCaption') msg.value = String(r.value == null ? '' : r.value);
        if (r.type === 'setLook') { msg.prop = String(r.prop || ''); msg.value = r.value; }
        // (AutoSize = False: the .dfm's own size comes along)
        if (r.type === 'setLook' && r.size && typeof r.size.width === 'number' && typeof r.size.height === 'number') msg.size = { width: r.size.width, height: r.size.height };
        this.d.panel.reveal(this.d.panel.viewColumn, true);
        this.d.post(msg);
        break;
      }
      case 'resetPattern': return this.resetPattern(String(m.key || ''), m.confirmed === true);
      default: break;
    }
  }

  /**
   * "全部改回" on a pattern line: the same look / text difference on many controls -- every
   * one written back to the DFM value in ONE edit (one Ctrl+Z), after a question.
   * Position / size patterns are not offered: their DFM values differ control by control.
   */
  async resetPattern(key, confirmed) {
    const data = this.data;
    const pat = data && (data.patterns || []).find(p => p.key === key);
    if (!pat) return null;
    const rows = data.rows.filter(r => r.pattern === key && r.inSource && r.reset && /^(setLook|setCaption)$/.test(r.reset.type));
    if (!rows.length) { vscode.window.setStatusBarMessage('$(info) 這一種差異沒有可以改回的元件（位置／大小請一個一個改）', 5000); return null; }
    if (!confirmed) {
      const pick = await vscode.window.showWarningMessage(
        '把 ' + rows.length + ' 個 ' + pat.cls + ' 的 ' + pat.prop + ' 從「' + (pat.page || '（空）') + '」改回 DFM 的「' + (pat.dfm || '（空）') + '」？\n' +
        '（寫進 HTML 原始碼，一個 Ctrl+Z 全部復原；要存檔才寫入）', { modal: true }, '改回');
      if (pick !== '改回') return null;
    }
    // (AutoSize = False: each label's own .dfm size comes along -- lib/dfmdiff.resetItemsOf does the same)
    const items = rows.map(r => Object.assign({ id: r.id, type: r.reset.type, prop: r.reset.prop, value: r.reset.value }, r.reset.size ? { size: r.reset.size } : {}));
    this.d.panel.reveal(this.d.panel.viewColumn, true);
    this.d.post({ type: 'editMany', items });
    vscode.window.setStatusBarMessage('$(discard) ' + rows.length + ' 個 ' + pat.cls + ' 的 ' + pat.prop + ' 改回 DFM 的值（Ctrl+Z 復原）', 6000);
    return { count: rows.length, items };
  }
}

// ---------------------------------------------------------------------------
class PropsView {
  constructor(hub) {
    this.hub = hub;
    this.view = null;
    this.data = null;
  }

  resolveWebviewView(view) {
    this.view = view;
    const media = vscode.Uri.joinPath(this.hub.ctx.extensionUri, 'media');
    const w = view.webview;
    w.options = { enableScripts: true, enableCommandUris: false, localResourceRoots: [media] };
    const nonce = crypto.randomBytes(16).toString('base64');
    const css = w.asWebviewUri(vscode.Uri.joinPath(media, 'props.css'));
    const js = w.asWebviewUri(vscode.Uri.joinPath(media, 'props.js'));
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src ' + w.cspSource +
      '; script-src \'nonce-' + nonce + '\'; img-src ' + w.cspSource + ' data:;">' +
      '<meta name="viewport" content="width=device-width, initial-scale=1">' +
      '<link rel="stylesheet" href="' + css + '"></head><body><div id="root"></div>' +
      '<script nonce="' + nonce + '" src="' + js + '"></script></body></html>';
    w.onDidReceiveMessage(m => this.hub.onPropsMessage(m));
    view.onDidDispose(() => { this.view = null; });
  }

  show(data) {
    this.data = data;
    if (this.view) this.view.webview.postMessage({ type: 'show', data });
  }
}

// ---------------------------------------------------------------------------
/** 搜尋頁面: the search box above the 頁面 list (media/search.js); what it types filters that list. */
class PageSearchView {
  constructor(hub) {
    this.hub = hub;
    this.view = null;
  }

  resolveWebviewView(view) {
    this.view = view;
    const media = vscode.Uri.joinPath(this.hub.ctx.extensionUri, 'media');
    const w = view.webview;
    w.options = { enableScripts: true, enableCommandUris: false, localResourceRoots: [media] };
    const nonce = crypto.randomBytes(16).toString('base64');
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src ' + w.cspSource + '; script-src \'nonce-' + nonce + '\';">' +
      '<meta name="viewport" content="width=device-width, initial-scale=1">' +
      '<link rel="stylesheet" href="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'search.css')) + '"></head><body><div id="root"' +
      (this.attrs ? Object.keys(this.attrs).map(k => ' data-' + k + '="' + htmlesc(this.attrs[k]) + '"').join('') : '') + '></div>' +
      '<script nonce="' + nonce + '" src="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'search.js')) + '"></script></body></html>';
    w.onDidReceiveMessage(m => this.onMessage(m));
    view.onDidDispose(() => { this.view = null; });
  }

  post(m) { if (this.view) this.view.webview.postMessage(m); }

  onMessage(m) {
    if (!m) return;
    const pages = this.hub.pages;
    if (m.type === 'ready') {
      // the box was drawn again (moved, hidden and shown): it shows what the list is filtered by
      if (pages.query) this.post({ type: 'set', q: pages.query, summary: pages.summary() });
      else if (m.q) this.show(pages.setFilter(m.q));
    } else if (m.type === 'query') {
      this.show(pages.setFilter(m.q));
    } else if (m.type === 'open') {
      return pages.openFirst();
    } else if (m.type === 'toList') {
      return vscode.commands.executeCommand('ht9045Designer.pages.focus');
    }
  }

  show(s) { this.post(Object.assign({ type: 'result' }, s)); }
  /** Put `q` in the box (a command set the filter). */
  set(q, s) { this.post({ type: 'set', q, summary: s }); }
}
// (an attribute value in the box's page)
function htmlesc(t) { return String(t).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }


// ---------------------------------------------------------------------------
/**
 * AI(W906-HTDESIGNER) 20261001 (0.140, EastSun's screenshot of Visual Studio's navigation bar 專案 ▾ | 類別 ▾ | 成員 ▾):
 * a C++ file's classes and members for VS Code's breadcrumbs (the bar above the editor: each part a drop-down) and
 * the 類別 / 成員 pickers in the editor's title bar. lib/cppsymbols.js reads them (a light reading, not a compiler).
 */
// 執行列 (0.142): ▶ 啟動／繼續, ▾ the configuration, ⏸ 暫停, ⏹ 停止, ↻ 重新啟動, the steps -- VS Code's debug commands on
// status bar items that stay; a button that cannot act now is grey and does nothing
class RunBar {
  constructor(ctx) {
    const B = [
      ['start', '$(debug-start)'], ['pick', '$(triangle-down)'], ['pause', '$(debug-pause)'], ['stop', '$(debug-stop)'],
      ['restart', '$(debug-restart)'], ['over', '$(debug-step-over)'], ['into', '$(debug-step-into)'], ['out', '$(debug-step-out)'],
    ];
    this.items = {};
    B.forEach(([k, t], i) => {
      const it = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 1000 - i);
      it.text = t;
      this.items[k] = it;
      ctx.subscriptions.push(it);
    });
    const d = vscode.debug || {};
    for (const ev of ['onDidStartDebugSession', 'onDidTerminateDebugSession', 'onDidChangeActiveDebugSession', 'onDidChangeActiveStackItem'])
      if (d[ev]) ctx.subscriptions.push(d[ev](() => this.update()));
    this.update();
  }
  // what the buttons can do now: nothing running / running / stopped at a breakpoint (a stack frame in focus)
  state() {
    const d = vscode.debug || {};
    if (!d.activeDebugSession) return 'idle';
    const s = d.activeStackItem;
    return s && s.frameId !== undefined ? 'paused' : 'running';
  }
  update() {
    const st = this.state(), grey = vscode.ThemeColor ? new vscode.ThemeColor('disabledForeground') : undefined;
    const set = (k, cmd, tip) => {
      const it = this.items[k];
      it.command = cmd || undefined;
      it.color = cmd ? undefined : grey;
      it.tooltip = tip + (cmd ? '' : '（現在不能用）');
      it.show();
    };
    const on = (want, cmd) => (want ? cmd : null);
    set('start', st === 'idle' ? 'workbench.action.debug.start' : on(st === 'paused', 'workbench.action.debug.continue'),
      st === 'paused' ? '繼續 (F5)' : st === 'running' ? '執行中' : '啟動 (F5)：用「執行與偵錯」選好的設定');
    set('pick', on(st === 'idle', 'workbench.action.debug.selectandstart'), '選擇要啟動的設定，然後啟動');
    set('pause', on(st === 'running', 'workbench.action.debug.pause'), '暫停 (F6)');
    set('stop', on(st !== 'idle', 'workbench.action.debug.stop'), '停止 (Shift+F5)');
    set('restart', on(st !== 'idle', 'workbench.action.debug.restart'), '重新啟動 (Ctrl+Shift+F5)');
    set('over', on(st === 'paused', 'workbench.action.debug.stepOver'), '不進入函式 (F10)');
    set('into', on(st === 'paused', 'workbench.action.debug.stepInto'), '逐步執行 (F11)');
    set('out', on(st === 'paused', 'workbench.action.debug.stepOut'), '跳離函式 (Shift+F11)');
    return st;
  }
}

class CppNav {
  constructor(hub) {
    this.hub = hub;
    this.cache = new Map();   // uri -> { version, syms }
  }

  symsOf(doc) {
    const k = doc.uri.toString();
    const c = this.cache.get(k);
    if (c && c.version === doc.version) return c.syms;
    const syms = cppsymbols.symbolsOf(doc.getText());
    this.cache.set(k, { version: doc.version, syms });
    if (this.cache.size > 50) this.cache.delete(this.cache.keys().next().value);
    return syms;
  }

  kindOf(s) {
    const K = vscode.SymbolKind;
    return s.kind === 'struct' ? K.Struct : s.kind === 'class' ? K.Class : s.kind === 'method' ? (s.detail === '建構式' ? K.Constructor : K.Method) : s.kind === 'field' ? K.Field : K.Function;
  }

  /** VS Code's DocumentSymbolProvider: the breadcrumbs and the Outline. */
  provideDocumentSymbols(doc) {
    if (vscode.workspace.getConfiguration('ht9045Designer').get('cppNavigation') === false) return [];
    const mk = s => {
      const r = new vscode.Range(doc.positionAt(s.start), doc.positionAt(s.end));
      const sel = new vscode.Range(doc.positionAt(s.selStart), doc.positionAt(Math.max(s.selStart, s.selEnd)));
      const ds = new vscode.DocumentSymbol(s.name, (s.detail || '') + (s.sig ? ' ' + s.sig : ''), this.kindOf(s), r, r.contains(sel) ? sel : r);
      ds.children = (s.children || []).map(mk);
      return ds;
    };
    return this.symsOf(doc).map(mk);
  }

  /** 專案 ▾ / 類別 ▾ / 成員 ▾ (Visual Studio's navigation bar): which = 'project' | 'class' | 'member'. */
  async cmdNav(which) {
    const ed = vscode.window.activeTextEditor;
    if (!ed) { vscode.window.showInformationMessage('先開一個 C++ 檔。'); return null; }
    const doc = ed.document;
    if (which === 'project') return this.hub.solution.cmdReveal(doc.uri.fsPath).then(n => { if (n) vscode.commands.executeCommand('ht9045Designer.solution.focus'); return n; });
    const syms = this.symsOf(doc);
    const off = doc.offsetAt(ed.selection.active);
    const chain = cppsymbols.chainAt(syms, off);
    const classes = syms.filter(s => s.kind === 'class' || s.kind === 'struct');
    let items;
    if (which === 'class') {
      items = classes.map(s => ({ label: '$(symbol-class) ' + s.name, description: (s.defs ? '定義' : s.kind) + '　' + s.children.length + ' 個成員', s }))
        .concat(syms.filter(s => s.kind === 'function').length ? [{ label: '$(symbol-function) （全域範圍）', description: syms.filter(s => s.kind === 'function').length + ' 個函式', global: true }] : []);
    } else {
      const cur = chain[0] && (chain[0].kind === 'class' || chain[0].kind === 'struct') ? chain[0] : null;
      const sameName = cur ? syms.filter(s => (s.kind === 'class' || s.kind === 'struct') && s.name === cur.name) : [];
      const list = cur ? [].concat(...sameName.map(s => s.children)) : syms.filter(s => s.kind === 'function').concat(...classes.map(c => c.children));
      items = list.map(s => ({ label: (s.kind === 'field' ? '$(symbol-field) ' : '$(symbol-method) ') + s.name, description: (s.sig || '') + (s.detail ? '　' + s.detail : ''), s }));
    }
    if (!items.length) { vscode.window.showInformationMessage('這個檔沒有找到' + (which === 'class' ? '類別' : '成員') + '。'); return null; }
    const here = which === 'class' ? chain[0] : chain[chain.length - 1];
    const act = items.find(i => i.s === here);
    const pick = await vscode.window.showQuickPick(items, { placeHolder: (which === 'class' ? '類別' : '成員') + '（' + path.basename(doc.uri.fsPath) + '）——選一個就跳過去', matchOnDescription: true, activeItems: act ? [act] : undefined });
    if (!pick) return null;
    if (pick.global) return this.cmdNavGlobal(ed, syms);
    const pos = doc.positionAt(pick.s.selStart);
    ed.selection = new vscode.Selection(pos, doc.positionAt(pick.s.selEnd));
    ed.revealRange(new vscode.Range(pos, pos), vscode.TextEditorRevealType.InCenterIfOutsideViewport);
    return pick.s;
  }

  async cmdNavGlobal(ed, syms) {
    const fs1 = syms.filter(s => s.kind === 'function');
    const pick = await vscode.window.showQuickPick(fs1.map(s => ({ label: '$(symbol-function) ' + s.name, description: s.sig || '', s })), { placeHolder: '全域範圍的函式' });
    if (!pick) return null;
    const pos = ed.document.positionAt(pick.s.selStart);
    ed.selection = new vscode.Selection(pos, ed.document.positionAt(pick.s.selEnd));
    ed.revealRange(new vscode.Range(pos, pos), vscode.TextEditorRevealType.InCenterIfOutsideViewport);
    return pick.s;
  }
}

// ---------------------------------------------------------------------------
/**
 * AI(W906-HTDESIGNER) 20261001 (0.138, EastSun's screenshot of Visual Studio's 方案總管): the project as a solution --
 * 方案 -> projects (the C++ port tree, the web tree, the BCB6 golden tree: the 專案搜尋 roots) -> folders -> files, in
 * Visual Studio's order (folders first, A->Z). A click opens a file (golden: read-only, decoded from Big5).
 * 搜尋方案總管 (Ctrl+; as in Visual Studio): only the files whose name / path has every typed word, with their folders,
 * all expanded. 與作用中文件同步: the open editor's file selected in the tree.
 */
class SolutionTree {
  constructor(hub) {
    this.hub = hub;
    this._em = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._em.event;
    this.view = null;
    this.q = '';
    this.flt = null;        // { byRoot: Map(root -> filter result), count, truncated }
    this.cache = new Map(); // root -> { at, files } (the file lists, for the filter)
    this.nodes = new Map(); // id -> node (getParent / reveal)
    this.versionText = '';
    this.pageSummary = null;
    // the 頁面 part follows the 頁面 list (a page added / removed, the open / active marks)
    if (hub.pages && hub.pages.onDidChangeTreeData) hub.pages.onDidChangeTreeData(() => this._em.fire());
  }

  // 0.148 (EastSun: "BCB 原始碼就不用出現了 編譯用不到的東西"): only what the build uses -- the C++ port tree and the web;
  // the BCB6 golden tree stays in 尋找 / 專案搜尋
  projects() { return this.hub.projSearch.areas().filter(p => p.area !== 'golden'); }

  node(n) { this.nodes.set(n.id, n); return n; }

  async fileList(root) {
    const c = this.cache.get(root);
    if (c && Date.now() - c.at < 60000) return c.files;
    const files = await solutiontree.allFiles(root);
    this.cache.set(root, { at: Date.now(), files });
    return files;
  }

  /**
   * 搜尋方案總管: q = the words ('' = off) -- the files (name / path) AND, under 網頁 -> 頁面, the pages and their
   * components (the old 搜尋頁面, 0.139). opts.pagesDone: the 頁面 part was just filtered. -> { count, truncated, pages }
   */
  async setFilter(q, opts) {
    this.q = String(q || '').trim();
    let ps = null;
    if (!(opts && opts.pagesDone) && this.hub.pages && this.hub.pages.query !== this.q) {
      ps = this.hub.pages.setFilter(this.q);
      if (this.hub.pageSearch) this.hub.pageSearch.set(this.hub.pages.query, ps);
    }
    this.pageSummary = this.q && this.hub.pages ? this.hub.pages.summary() : null;
    // (0.146: the box on top shows the words -- unless they came from it: it may hold more typing by now)
    const done = r => {
      this.lastResult = r;
      if (!(opts && opts.fromBox) && this.hub.solPanel) this.hub.solPanel.set(this.q);
      return r;
    };
    if (!this.q) { this.flt = null; this.refresh(); return done({ count: 0 }); }
    const byRoot = new Map();
    let count = 0, truncated = false;
    for (const p of this.projects()) {
      const r = solutiontree.filter(p.root, await this.fileList(p.root), this.q, 2000 - count);
      byRoot.set(p.root, r);
      count += r.count;
      truncated = truncated || r.truncated;
    }
    this.flt = { byRoot, count, truncated };
    this.refresh();
    const pg = this.pageSummary || { pages: 0, hits: 0 };
    return done({ count, truncated, pages: pg.pages, hits: pg.hits });
  }

  /** The title line: the version (the old 頁面 list showed it), and what the search found. */
  setDescription() {
    if (!this.view) return;
    const pg = this.pageSummary;
    const found = this.q ? '「' + this.q + '」' + (this.flt ? this.flt.count + (this.flt.truncated ? '+' : '') + ' 個檔' : '') +
      (pg && pg.pages ? '、' + pg.pages + ' 頁' + (pg.hits ? '、' + pg.hits + (pg.capped ? '+' : '') + ' 個元件' : '') : '') : '';
    this.view.description = [found, this.versionText || ''].filter(Boolean).join('　');
    this.view.message = this.q && this.flt && !this.flt.count && !(pg && pg.pages) ? '沒有檔名、路徑、頁面或元件含「' + this.q + '」' : undefined;
  }

  refresh() {
    this.nodes.clear();
    this.setDescription();
    vscode.commands.executeCommand('setContext', 'ht9045Designer.solutionFiltered', !!this.q);
    this._em.fire();
  }

  /** The 頁面 node (網頁 -> 頁面) selected and opened, with the active page in it when one is open. */
  async revealPages() {
    const web = this.projects().find(p => p.area === 'web');
    if (!web || !this.view || !this.view.reveal) return null;
    const sln = this.node({ type: 'sln', id: 'sln' });
    const proj = this.node({ type: 'proj', id: 'sln|web', p: web, path: web.root, parentId: sln.id });
    const pg = this.node({ type: 'pgroot', id: 'sln|web|@pages', p: web, parentId: proj.id });
    try { await this.view.reveal(pg, { select: true, focus: false, expand: true }); } catch (e) { /* not drawn yet */ }
    return pg;
  }

  /** A node of the 頁面 list (PageTree) inside this tree. */
  wrap(inner, parent) {
    return this.node({ type: 'pg', inner, p: parent.p, parentId: parent.id, id: 'sln|pg|' + this.pageKey(inner) });
  }

  pageKey(n) {
    if (n.kind === 'group') return 'grp:' + n.name + (n.filtered ? '?' : '');
    if (n.kind === 'page') return 'page:' + String(n.file || '').toLowerCase() + (n.filtered ? '?' : '');
    if (n.kind === 'hit') return 'hit:' + String(n.hit.file).toLowerCase() + '#' + n.hit.id + '@' + n.hit.line;
    return 'more:' + (n.parent && n.parent.file ? String(n.parent.file).toLowerCase() : '');
  }

  async getChildren(n) {
    if (!n) return [this.node({ type: 'sln', id: 'sln' })];
    if (n.type === 'sln') {
      // (searching: a project with a matching file -- or 網頁 with a matching page / component)
      return this.projects().filter(p => !this.flt || (this.flt.byRoot.get(p.root) || { count: 0 }).count || (p.area === 'web' && this.pageSummary && this.pageSummary.pages))
        .map(p => this.node({ type: 'proj', id: 'sln|' + p.area, p, path: p.root, parentId: 'sln' }));
    }
    // 網頁 -> 頁面 (0.139): the old 頁面 list, as it was (groups -> pages; filtered: -> components)
    if (n.type === 'pgroot') return this.hub.pages ? this.hub.pages.getChildren().map(x => this.wrap(x, n)) : [];
    if (n.type === 'pg') return (this.hub.pages.getChildren(n.inner) || []).map(x => this.wrap(x, n));
    const p = n.p;
    const f = this.flt ? this.flt.byRoot.get(p.root) : null;
    const es = await solutiontree.entries(n.path);
    const out = es.filter(e => !f || (e.dir ? f.dirs.has(path.resolve(e.path).toLowerCase()) : f.files.has(path.resolve(e.path).toLowerCase())))
      .map(e => this.node({ type: e.dir ? 'dir' : 'file', id: 'sln|' + p.area + '|' + path.resolve(e.path).toLowerCase(), p, path: e.path, name: e.name, parentId: n.id }));
    if (n.type === 'proj' && p.area === 'web' && this.hub.pages) {
      const pgs = this.hub.pages.getChildren();
      if (!this.q || pgs.length) out.unshift(this.node({ type: 'pgroot', id: 'sln|web|@pages', p, parentId: n.id }));
    }
    return out;
  }

  getParent(n) { return n && n.parentId ? this.nodes.get(n.parentId) : undefined; }

  getTreeItem(n) {
    const S = vscode.TreeItemCollapsibleState;
    const open = this.q ? S.Expanded : S.Collapsed;
    if (n.type === 'pgroot') {
      const it = new vscode.TreeItem('頁面（依畫面）', this.q ? S.Expanded : S.Collapsed);
      const pg = this.pageSummary;
      it.description = this.q && pg ? pg.pages + ' 頁' + (pg.hits ? '、' + pg.hits + (pg.capped ? '+' : '') + ' 個元件' : '') : '點一下頁面＝設計檢視';
      it.tooltip = 'web\\page 的每一頁，照畫面分類（以前的「頁面」清單）。點一下＝用設計檢視開；搜尋方案總管時也找頁面標題和元件。';
      it.iconPath = new vscode.ThemeIcon('layout');
      it.id = n.id + (this.q ? '?' : '');
      return it;
    }
    if (n.type === 'pg') {
      // (the 頁面 list's own item: label, highlights, the page's marks, its command -- its id made unique here)
      const it = this.hub.pages.getTreeItem(n.inner);
      it.id = n.id + (this.q ? '?' + this.hub.pages.gen : '');
      return it;
    }
    if (n.type === 'sln') {
      const ps = this.projects();
      const it = new vscode.TreeItem('方案 \'' + this.solutionName() + '\'（' + ps.length + ' 個專案）', S.Expanded);
      it.iconPath = new vscode.ThemeIcon('folder-library');
      it.id = n.id;
      it.tooltip = ps.map(p => p.label + '：' + p.root).join('\n');
      return it;
    }
    if (n.type === 'proj') {
      const it = new vscode.TreeItem(n.p.label, this.q ? S.Expanded : S.Collapsed);
      it.description = path.basename(n.p.root);
      it.tooltip = n.p.root;
      it.iconPath = new vscode.ThemeIcon(n.p.area === 'golden' ? 'history' : n.p.area === 'web' ? 'globe' : 'project');
      it.id = n.id;
      return it;
    }
    if (n.type === 'dir') {
      const it = new vscode.TreeItem(n.name, open);
      it.resourceUri = vscode.Uri.file(n.path);
      // 0.142 (EastSun: "資料夾也需要圖示"): ThemeIcon.Folder follows the file icon theme, and Seti draws no folder. 0.143: so does ANY
      // ThemeIcon whose id is 'folder' (or 'file') -- VS Code checks the id, so 0.142's new ThemeIcon('folder') was still blank; symbol-folder always draws
      it.iconPath = new vscode.ThemeIcon('symbol-folder', vscode.ThemeColor ? new vscode.ThemeColor('charts.yellow') : undefined);
      it.id = n.id;
      it.contextValue = 'htdSlnDir';
      return it;
    }
    const it = new vscode.TreeItem(n.name, S.None);
    it.resourceUri = vscode.Uri.file(n.path);
    it.iconPath = vscode.ThemeIcon.File;
    it.id = n.id;
    it.tooltip = n.path + (n.p.area === 'golden' ? '\n（BCB6 原始碼：唯讀、照 Big5 開）' : '');
    it.contextValue = 'htdSlnFile';
    it.command = { command: 'ht9045Designer.solutionOpen', title: '開啟', arguments: [n] };
    return it;
  }

  /** The solution's name: the folder that holds the projects (D:\HT9050\htd_work -> htd_work), else HT9045. */
  solutionName() {
    const ps = this.projects();
    const dirs = Array.from(new Set(ps.map(p => path.dirname(path.resolve(p.root)).toLowerCase())));
    return dirs.length === 1 ? path.basename(path.dirname(path.resolve(ps[0].root))) : 'HT9045';
  }

  async open(n) {
    if (!n || !n.path) return null;
    // a page: as text (the designer has its own open command); golden: read-only, Big5
    const uri = n.p && n.p.area === 'golden' ? vscode.Uri.file(n.path).with({ scheme: GOLDEN_SCHEME }) : vscode.Uri.file(n.path);
    try {
      const doc = await vscode.workspace.openTextDocument(uri);
      await vscode.window.showTextDocument(doc, { preview: true });
      return uri;
    } catch (e) {
      // (not text: let VS Code open it its own way)
      try { await vscode.commands.executeCommand('vscode.open', vscode.Uri.file(n.path)); return vscode.Uri.file(n.path); } catch (x) { return null; }
    }
  }

  async cmdFilter(arg) {
    // 0.146: Ctrl+F / Ctrl+; / the title bar's filter = into the box of 方案總管 (an input box only if it cannot show);
    // 0.148: the box is in 方案總管 itself
    if (typeof arg !== 'string' && this.hub.solPanel) {
      try {
        await vscode.commands.executeCommand('ht9045Designer.solution.focus');
        this.hub.solPanel.focusBox();
        return { box: true };
      } catch (e) { /* the box is not there: ask below */ }
    }
    const q = typeof arg === 'string' ? arg : await vscode.window.showInputBox({
      prompt: '搜尋方案總管：檔名或路徑（空白隔開＝每個字都要有；含 / 的字比對路徑）', placeHolder: '例如 fHotPlate、forms .h、web/page main', value: this.q,
    });
    if (q == null) return null;
    const r = await this.setFilter(q);
    if (this.view && this.q) { try { await vscode.commands.executeCommand('ht9045Designer.solution.focus'); } catch (e) { /* not showing */ } }
    return r;
  }

  /** 與作用中文件同步 (Visual Studio "Sync with Active Document"): the open file selected in the tree. */
  async cmdReveal(fileArg) {
    const ed = vscode.window.activeTextEditor;
    const file = typeof fileArg === 'string' ? fileArg : ed && ed.document && ed.document.uri ? ed.document.uri.fsPath : null;
    if (!file) { vscode.window.showInformationMessage('沒有開著的檔案。'); return null; }
    const p = projectsearch.areaOf(this.projects(), file);
    if (!p) { vscode.window.showInformationMessage(path.basename(file) + ' 不在這個方案的專案裡（' + this.projects().map(x => x.label).join('、') + '）。'); return null; }
    // build the chain sln -> proj -> dirs -> file (the same ids getChildren gives)
    const sln = this.node({ type: 'sln', id: 'sln' });
    let parent = this.node({ type: 'proj', id: 'sln|' + p.area, p, path: p.root, parentId: sln.id });
    const rel = path.relative(p.root, file).split(path.sep);
    let cur = p.root, last = parent;
    for (let i = 0; i < rel.length; i++) {
      cur = path.join(cur, rel[i]);
      last = this.node({ type: i === rel.length - 1 ? 'file' : 'dir', id: 'sln|' + p.area + '|' + path.resolve(cur).toLowerCase(), p, path: cur, name: rel[i], parentId: parent.id });
      parent = last;
    }
    if (this.view && this.view.reveal) { try { await this.view.reveal(last, { select: true, focus: false, expand: false }); } catch (e) { /* hidden by the filter */ } }
    return last;
  }
}

// ---------------------------------------------------------------------------
/**
 * AI(W906-HTDESIGNER) 20261001 (0.148, EastSun: "方案總管整個重做成一格", with a picture: the search box right under
 * 方案總管's title, the tree under it, what does not match hidden): 方案總管 drawn by the extension in one webview view.
 * VS Code puts a title bar over every view and a tree view cannot hold an input, so the box and the tree are one page.
 * The data stays SolutionTree's (getChildren / getTreeItem / setFilter / reveal -- the same nodes, ids, commands and
 * marks); this class turns the open part of it into rows (media/solution.js draws them) with VS Code's own icons
 * (lib/icons.js: the codicon font and the Seti file icons VS Code ships). SolutionTree.view is this.adapter.
 */
class SolutionPanel {
  constructor(hub) {
    this.hub = hub;
    this.view = null;
    this.open = new Map();   // node id -> open (the user's); not there = the item's own state
    this.sel = null;
    this.items = new Map();  // node id -> { n, it } of the rows drawn
    this.rows = [];
    this.description = '';
    this.message = '';
    this.timer = null;
    this.lastQ = '';
    const self = this;
    this.adapter = {
      get description() { return self.description; },
      set description(v) { self.description = v || ''; if (self.view) self.view.description = self.description; },
      get message() { return self.message; },
      set message(v) { self.message = v || ''; self.schedule(); },
      get visible() { return !!(self.view && self.view.visible); },
      reveal: (n, o) => self.reveal(n, o),
    };
    hub.solution.onDidChangeTreeData(() => this.schedule());
    // (the Seti colours are the theme's: light / dark again when the theme changes)
    if (vscode.window.onDidChangeActiveColorTheme) hub.ctx.subscriptions.push(vscode.window.onDidChangeActiveColorTheme(() => this.schedule()));
  }

  appRoot() { return (vscode.env && vscode.env.appRoot) || ''; }

  resolveWebviewView(view) {
    this.view = view;
    view.description = this.description;
    const media = vscode.Uri.joinPath(this.hub.ctx.extensionUri, 'media');
    const app = this.appRoot();
    const cf = app ? icons.codiconFont(app) : null, se = app ? icons.seti(app) : null;
    const w = view.webview;
    const roots = [media];
    if (cf) roots.push(vscode.Uri.file(path.dirname(cf)));
    if (se) roots.push(vscode.Uri.file(se.dir));
    w.options = { enableScripts: true, enableCommandUris: false, localResourceRoots: roots };
    const nonce = crypto.randomBytes(16).toString('base64');
    const cm = app ? icons.codicons(app) : icons.FALLBACK;
    const ch = { search: cm.search, close: cm.close, 'chevron-right': cm['chevron-right'], 'chevron-down': cm['chevron-down'] };
    const font = (fam, f, fmt) => f ? '@font-face{font-family:\'' + fam + '\';src:url("' + w.asWebviewUri(vscode.Uri.file(f)) + '") format("' + fmt + '");}' : '';
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; font-src ' + w.cspSource + '; style-src ' + w.cspSource + ' \'nonce-' + nonce + '\'; script-src \'nonce-' + nonce + '\';">' +
      '<meta name="viewport" content="width=device-width, initial-scale=1">' +
      '<link rel="stylesheet" href="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'solution.css')) + '">' +
      '<style nonce="' + nonce + '">' + font('htd-codicon', cf, 'truetype') + font('htd-seti', se && se.font, 'woff') + '</style>' +
      '</head><body><div id="root" data-ch="' + htmlesc(JSON.stringify(ch)) + '"></div>' +
      '<script nonce="' + nonce + '" src="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'solution.js')) + '"></script></body></html>';
    w.onDidReceiveMessage(m => this.onMessage(m));
    if (view.onDidChangeVisibility) view.onDidChangeVisibility(() => { if (view.visible) this.schedule(); });
    view.onDidDispose(() => { this.view = null; });
  }

  post(m) { if (this.view) this.view.webview.postMessage(m); }

  schedule() {
    clearTimeout(this.timer);
    this.timer = setTimeout(() => { this.render().catch(e => this.hub.log('方案總管：' + (e && e.message || e))); }, 30);
  }

  /** The rows: the tree from the top, into what is open (the user's, or the item's own -- searching opens everything). */
  async build(g) {
    const sol = this.hub.solution;
    if (sol.q !== this.lastQ) { this.open.clear(); this.lastQ = sol.q; }
    const app = this.appRoot();
    const cm = app ? icons.codicons(app) : icons.FALLBACK;
    const se = app ? icons.seti(app) : null;
    const light = vscode.window.activeColorTheme && (vscode.window.activeColorTheme.kind === 1 || vscode.window.activeColorTheme.kind === 4);
    const rows = [], items = new Map(), MAX = 5000;
    const walk = async (n, d) => {
      if (rows.length >= MAX) return;
      const it = sol.getTreeItem(n);
      const coll = it.collapsibleState || 0;
      const isOpen = coll ? (this.open.has(n.id) ? this.open.get(n.id) : coll === 2) : false;
      items.set(n.id, { n, it });
      rows.push(this.row(n, it, d, coll ? (isOpen ? 2 : 1) : 0, cm, se, light));
      if (isOpen) for (const c of (await sol.getChildren(n)) || []) await walk(c, d + 1);
    };
    for (const r of (await sol.getChildren()) || []) { if (g !== undefined && g !== this.gen) return null; await walk(r, 0); }
    return { rows, items };
  }

  row(n, it, d, tw, cm, se, light) {
    const lab = typeof it.label === 'string' ? { label: it.label } : (it.label || { label: n.name || '' });
    const tip = typeof it.tooltip === 'string' ? it.tooltip : it.tooltip && it.tooltip.value ? it.tooltip.value : '';
    let ic = null;
    const ip = it.iconPath;
    const fileName = n.type === 'file' ? n.name : null;
    if (fileName || (ip && ip.id === 'file')) {
      const f = icons.fileIcon(se, fileName || (it.resourceUri ? path.basename(it.resourceUri.fsPath) : ''), light);
      if (f) ic = { t: 's', ch: f.char, col: f.color };
    } else if (ip && ip.id && cm[ip.id]) {
      ic = { t: 'c', ch: cm[ip.id], col: ip.color && ip.color.id ? 'var(--vscode-' + ip.color.id.replace(/\./g, '-') + ')' : null };
    }
    return {
      id: n.id, d, l: lab.label, h: lab.highlights || null, ds: typeof it.description === 'string' ? it.description : '',
      tip, ic, tw, ctx: it.contextValue || ('htdSln_' + n.type), cmd: !!it.command,
    };
  }

  async render(reveal) {
    if (!this.view) return null;
    // (renders overlap -- the timer, a toggle, a reveal: only the newest one posts and keeps its rows)
    const g = this.gen = (this.gen || 0) + 1;
    const built = await this.build(g);
    if (!built || g !== this.gen) return null;
    const { rows, items } = built;
    this.items = items;
    this.rows = rows;
    if (this.sel && !this.items.has(this.sel)) this.sel = null;
    const more = rows.length >= 5000 ? '（只列前 5000 列：打長一點的字縮小範圍）' : '';
    // (the selection goes with a reveal only: the page's own arrow keys may be ahead of these rows)
    this.post({ type: 'rows', rows, sel: reveal ? this.sel : null, msg: [this.message, more].filter(Boolean).join(' '), reveal: reveal || null });
    return rows;
  }

  async onMessage(m) {
    if (!m) return null;
    const sol = this.hub.solution;
    if (m.type === 'ready') {
      if (sol.q) this.post({ type: 'setq', q: sol.q });
      else if (m.q) { await sol.setFilter(m.q, { fromBox: true }); }
      return this.render();
    }
    if (m.type === 'query') return sol.setFilter(m.q, { fromBox: true });
    if (m.type === 'select') { this.sel = m.id; return null; }
    const x = this.items.get(m.id);
    if (!x) return null;
    if (m.type === 'toggle') {
      const cur = this.open.has(m.id) ? this.open.get(m.id) : x.it.collapsibleState === 2;
      this.open.set(m.id, !cur);
      this.sel = m.id;
      return this.render();
    }
    if (m.type === 'activate') {
      this.sel = m.id;
      const c = x.it.command;
      if (c) return vscode.commands.executeCommand(c.command, ...(c.arguments || []));
    }
    return null;
  }

  /** A command's argument from the webview's right-click menu ({ htdId }) -> the node; a node stays a node. */
  nodeOf(a) {
    if (a && a.htdId) { const x = this.items.get(a.htdId); return x ? x.n : null; }
    return a;
  }

  /** Select a node: everything above it opened, scrolled to (Sync with Active Document, the 頁面 node). */
  async reveal(n, o) {
    if (!n) return null;
    const sol = this.hub.solution;
    for (let p = sol.getParent(n); p; p = sol.getParent(p)) this.open.set(p.id, true);
    if (o && o.expand) this.open.set(n.id, true);
    if (o && o.select !== false) this.sel = n.id;
    if (o && o.focus) this.post({ type: 'focusList' });
    return this.render(n.id);
  }

  collapseAll() {
    for (const [id, x] of this.items) if (x.n.type !== 'sln' && (x.it.collapsibleState || 0)) this.open.set(id, false);
    return this.render();
  }

  /** The words into the box (a command set the filter). */
  set(q) { this.post({ type: 'setq', q }); }
  focusBox() { this.post({ type: 'focus' }); }
}

// ---------------------------------------------------------------------------
/**
 * AI(W906-HTDESIGNER) 20261001 (0.137, EastSun: "我沒辦法整個專案查詢關鍵字並列出來"): 專案搜尋 -- a keyword over the
 * C++ port tree, the web tree and the BCB6 golden tree (Big5 read as Big5: VS Code's Find in Files garbles it), every
 * hit in this tree: area -> file -> line (VS Code's Search view's layout). A click opens the line (golden: read-only,
 * decoded from Big5, like every golden file the designer opens). Options = Find in Files' three: Aa, whole word, .*.
 */
class ProjectSearch {
  constructor(hub) {
    this.hub = hub;
    this._em = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._em.event;
    this.q = '';
    this.opts = { caseSensitive: false, wholeWord: false, regex: false };
    this.res = null;
    this.roots = [];
    this.view = null;
    this.busy = false;
    this.cancel = false;
  }

  /** The three trees, from the open designer or the workspace (the same roots the designer finds). */
  areas() {
    const d = this.hub.active;
    const r = d && d.r ? d.r : roots.resolveRoots(null, this.hub.wsFolders(), this.hub.over());
    return [
      { area: 'port', label: 'C++ 移植樹', root: r.portRoot, kind: 'port' },
      { area: 'web', label: '網頁', root: r.webRoot, kind: 'web' },
      { area: 'golden', label: 'BCB6 原始碼（golden，Big5）', root: r.goldenRoot, kind: 'golden' },
    ].filter(x => x.root && fs.existsSync(x.root));
  }

  optsText() {
    return [this.opts.caseSensitive ? 'Aa' : '', this.opts.wholeWord ? '全字' : '', this.opts.regex ? '正規式' : ''].filter(Boolean).join('、');
  }

  setDescription() {
    if (!this.view) return;
    const o = this.optsText();
    this.view.description = this.res ? '「' + this.q + '」' + this.res.hits.length + (this.res.truncated ? '+' : '') + ' 筆／' + this.res.files + ' 檔' +
      (this.scopeLabel && this.scope !== 'solution' ? '　在 ' + this.scopeLabel : '') + (o ? '（' + o + '）' : '') : (o ? '選項：' + o : '');
    vscode.commands.executeCommand('setContext', 'ht9045Designer.projSearch.case', this.opts.caseSensitive);
    vscode.commands.executeCommand('setContext', 'ht9045Designer.projSearch.word', this.opts.wholeWord);
    vscode.commands.executeCommand('setContext', 'ht9045Designer.projSearch.regex', this.opts.regex);
  }

  toggle(k) {
    this.opts[k] = !this.opts[k];
    this.setDescription();
    // (the same keyword again with the new option, in the same scope, like Find in Files)
    if (this.q && !this.busy) return this.run(this.q, this.scope, this.scopeFile);
    return null;
  }

  /** Ask for the keyword (the selected text of the editor first), then search. arg: the keyword (tests / other code). */
  async cmdSearch(arg) {
    let q = typeof arg === 'string' ? arg : null;
    if (q == null) {
      const ed = vscode.window.activeTextEditor;
      const sel = ed && !ed.selection.isEmpty ? ed.document.getText(ed.selection).split(/\r?\n/)[0].slice(0, 200) : '';
      q = await vscode.window.showInputBox({
        prompt: '在整個專案找（C++ 移植樹、網頁、BCB6 原始碼）' + (this.optsText() ? '　選項：' + this.optsText() : '　大小寫、全字、正規式在清單標題列切換'),
        placeHolder: '例如 DoInArm、spbSave、吸嘴、Gerneral.ini', value: sel || this.q,
      });
    }
    if (q == null || !String(q).trim()) return null;
    return this.run(String(q));
  }

  /**
   * From the code (Visual Studio: Find in Files with "Look in" = current document / current project / entire
   * solution): the selection -- or the word at the cursor -- searched in the scope picked. arg: { q, scope } (tests).
   */
  async cmdSearchHere(arg) {
    const ed = vscode.window.activeTextEditor;
    const file = ed && ed.document && ed.document.uri ? ed.document.uri.fsPath : (arg && arg.file) || null;
    let q = arg && typeof arg.q === 'string' ? arg.q : '';
    // (from the Solution Explorer's menu: ask -- the editor's word is not what was clicked)
    if (!q && ed && !(arg && arg.fromTree)) {
      const s = ed.selection;
      if (s && !s.isEmpty) q = ed.document.getText(s).split(/\r?\n/)[0].slice(0, 200);
      else if (ed.document.getWordRangeAtPosition) { const w = ed.document.getWordRangeAtPosition(s.active); if (w) q = ed.document.getText(w); }
    }
    if (!q) q = await vscode.window.showInputBox({ prompt: '要找的字', value: this.q }) || '';
    if (!q.trim()) return null;
    const areas = this.areas();
    const here = file ? projectsearch.areaOf(areas, file) : null;
    let scope = arg && arg.scope;
    if (!scope) {
      const items = [
        file ? { label: '$(file) 目前檔案', description: path.basename(file), scope: 'file' } : null,
        here ? { label: '$(folder) 目前專案', description: here.label, scope: 'project' } : null,
        { label: '$(folder-library) 整個方案', description: areas.map(a => a.label).join('、'), scope: 'solution' },
      ].filter(Boolean);
      const pick = await vscode.window.showQuickPick(items, { placeHolder: '在哪裡找「' + q + '」？（Visual Studio 的「搜尋範圍」）' });
      if (!pick) return null;
      scope = pick.scope;
    }
    return this.run(q, scope, file);
  }

  /**
   * 0.144 尋找 (EastSun: "我希望可以搜尋所有專案的關鍵字 可以選擇"; Visual Studio's Find in Files: 尋找目標 + 搜尋範圍 +
   * options in one box): the keyword typed above, the scope picked below (目前檔案 / 目前專案 / 整個方案 / one project),
   * Aa / 全字 / 正規式 as the box's buttons; Enter = search, the results in 專案搜尋. The last scope is picked again.
   */
  findScopes(file, inEditor) {
    const areas = this.areas();
    const here = file ? projectsearch.areaOf(areas, file) : null;
    return [
      // 0.145 (EastSun: "你不能直接取代ctrl+F 就好了嗎?"): Ctrl+F opens this box, so VS Code's own find in the file is its first scope
      // -- every match marked as typed, F3 / Shift+F3 (Visual Studio's 快速尋找 "目前文件")
      inEditor ? { label: '$(search) 這個檔案（即時標示，F3 下一個）', description: file ? path.basename(file) : '', scope: 'inline', file } : null,
      file ? { label: '$(file) 目前檔案', description: path.basename(file), scope: 'file', file } : null,
      here ? { label: '$(project) 目前專案', description: here.label, scope: 'project', file } : null,
      { label: '$(folder-library) 整個方案', description: areas.map(a => a.label).join('、'), scope: 'solution', file: null },
      ...areas.map(a => ({ label: '$(' + (a.area === 'golden' ? 'history' : a.area === 'web' ? 'globe' : 'symbol-class') + ') ' + a.label,
        description: path.basename(a.root), scope: 'project', file: a.root, area: a.area })),
    ].filter(Boolean);
  }
  findButtons() {
    const o = this.opts, b = (k, icon, name) => ({ k, iconPath: new vscode.ThemeIcon(o[k] ? 'pass-filled' : icon), tooltip: name + '（目前：' + (o[k] ? '開' : '關') + '）' });
    return [b('caseSensitive', 'case-sensitive', '大小寫要相同'), b('wholeWord', 'whole-word', '全字'), b('regex', 'regex', '正規式')];
  }
  async cmdFind(arg) {
    const ed = vscode.window.activeTextEditor;
    const file = ed && ed.document && ed.document.uri && ed.document.uri.scheme !== 'untitled' ? ed.document.uri.fsPath : null;
    let q = '';
    if (ed && ed.selection) {
      const sl = ed.selection;
      if (!sl.isEmpty) q = ed.document.getText(sl).split(/\r?\n/)[0].slice(0, 200);
      else if (ed.document.getWordRangeAtPosition) { const w = ed.document.getWordRangeAtPosition(sl.active); if (w) q = ed.document.getText(w); }
    }
    const scopes = this.findScopes(file, !!ed);
    const key = it => it.scope + '|' + (it.area || '');
    const last = scopes.find(it => key(it) === this.findLast) || scopes.find(it => it.scope === 'inline') || scopes.find(it => it.scope === 'solution');
    const go = (v, it) => {
      this.findLast = key(it);
      if (it.scope !== 'inline') return this.run(v, it.scope, it.file);
      // (VS Code's find widget with the keyword and the box's options)
      return Promise.resolve(vscode.commands.executeCommand('editor.actions.findWithArgs', {
        searchString: v, isCaseSensitive: this.opts.caseSensitive, matchWholeWord: this.opts.wholeWord, isRegex: this.opts.regex,
      })).then(() => ({ inline: true, q: v }));
    };
    // (tests / other code: { q, pick } = the keyword and the scope's key, no box)
    if (arg && typeof arg.q === 'string') return go(arg.q, scopes.find(x => key(x) === arg.pick) || last);
    if (!vscode.window.createQuickPick) return this.cmdSearchHere();
    const qp = vscode.window.createQuickPick();
    qp.title = '尋找　打字＝要找的字；↑↓ 選範圍（第一個＝這個檔案即時標示，其他＝列出每一筆）；Enter＝找';
    qp.placeholder = '要找的字，例如 DoInArm、spbSave、吸嘴';
    qp.value = q || this.q || '';
    qp.items = scopes;
    qp.activeItems = [last];
    qp.matchOnDescription = false;
    qp.buttons = this.findButtons();
    // (typing must not filter the scope list away: every scope stays, the typed text is only the keyword)
    const keep = () => { qp.items = scopes.map(it => Object.assign({}, it, { alwaysShow: true })); const a = qp.items.find(it => key(it) === key(last)); if (a) qp.activeItems = [a]; };
    keep();
    return new Promise(resolve => {
      let done = false, accepted = false;
      const fin = v => { if (!done) { done = true; resolve(v); } };
      qp.onDidTriggerButton(b => { this.toggleOpt(b.k); qp.buttons = this.findButtons(); });
      qp.onDidAccept(async () => {
        const it = qp.activeItems[0] || last, v = qp.value;
        if (!v.trim()) { qp.validationMessage = '先打要找的字'; return; }
        accepted = true;
        qp.hide();
        fin(await go(v, it));
      });
      // (hide() above fires this too -- then the search's result is the answer, not null)
      qp.onDidHide(() => { qp.dispose(); if (!accepted) fin(null); });
      this.findBox = qp;
      qp.show();
    });
  }
  // (an option changed in the box: only remembered -- the box's Enter searches with it)
  toggleOpt(k) { this.opts[k] = !this.opts[k]; this.setDescription(); }

  async run(q, scope, file) {
    const mk = projectsearch.makeRe(q, this.opts);
    if (mk.error) { vscode.window.showWarningMessage(mk.error); return null; }
    const all = this.areas();
    if (!all.length) { vscode.window.showInformationMessage('找不到要搜尋的資料夾（C++ 移植樹、web、BCB6 原始碼）：開一個含它們的資料夾，或在設定 ht9045Designer.portRoot／webRoot／goldenRoot 指定。'); return null; }
    const sc = projectsearch.scopeRoots(all, scope || 'solution', file);
    if (sc.error) { vscode.window.showInformationMessage(sc.error + '。'); return null; }
    const areas = sc.roots;
    this.scope = scope || 'solution';
    this.scopeLabel = sc.label;
    this.scopeFile = file || null;
    if (this.busy) { this.cancel = true; for (let i = 0; i < 100 && this.busy; i++) await new Promise(r => setTimeout(r, 20)); }
    this.busy = true;
    this.cancel = false;
    this.q = q;
    let res = null;
    try {
      res = await vscode.window.withProgress({ location: vscode.ProgressLocation.Notification, title: '專案搜尋「' + q + '」', cancellable: true }, async (prog, token) => {
        return projectsearch.search(areas, mk.re, {
          cancelled: () => this.cancel || !!(token && token.isCancellationRequested),
          progress: (n, total) => prog.report({ message: n + '／' + total + ' 個檔' }),
        });
      });
    } finally { this.busy = false; }
    this.res = res;
    this.roots = areas;
    this.setDescription();
    this._em.fire();
    this.hub.log('專案搜尋「' + q + '」' + (this.optsText() ? '（' + this.optsText() + '）' : '') + '：' + res.hits.length + (res.truncated ? '+' : '') + ' 筆、' + res.files + ' 個檔（掃了 ' + res.scanned + ' 個，' + res.ms + ' ms）' + (res.cancelled ? '，中途取消' : ''));
    if (this.view) { try { await vscode.commands.executeCommand('ht9045Designer.projectSearch.focus'); } catch (e) { /* not showing */ } }
    if (!res.hits.length) vscode.window.showInformationMessage('整個專案都沒有找到「' + q + '」（' + areas.map(a => a.label).join('、') + '，' + res.scanned + ' 個檔）。');
    else if (res.truncated && !res.cancelled) vscode.window.showInformationMessage('「' + q + '」超過 ' + res.hits.length + ' 筆，只列前 ' + res.hits.length + ' 筆：打長一點、或開「全字」。');
    return res;
  }

  /** The results as text to the clipboard (every hit: file:line:col  the line). */
  async cmdCopy() {
    if (!this.res || !this.res.hits.length) { vscode.window.showInformationMessage('還沒有搜尋結果。'); return null; }
    const t = projectsearch.asText(this.q, this.res, this.roots);
    await vscode.env.clipboard.writeText(t);
    vscode.window.setStatusBarMessage('$(copy) 專案搜尋的 ' + this.res.hits.length + ' 筆結果複製到剪貼簿了', 5000);
    return t;
  }

  cmdClear() {
    this.res = null;
    this.q = '';
    this.setDescription();
    this._em.fire();
  }

  /** Open a hit's line, the keyword selected (golden: read-only Big5). */
  async open(h) {
    if (!h) return null;
    // (only the golden tree through the read-only Big5 view; a port / web file with a few stray bytes opens as it is)
    const t = { kind: h.area === 'golden' ? 'golden' : 'port', file: h.file, line: h.line, col: h.col };
    try {
      const uri = t.kind === 'golden' ? vscode.Uri.file(h.file).with({ scheme: GOLDEN_SCHEME }) : vscode.Uri.file(h.file);
      const doc = await vscode.workspace.openTextDocument(uri);
      const ln = Math.max(0, h.line - 1);
      // (the line shown may be the part around the hit of a long line: find the keyword in the real line)
      const real = doc.lineAt ? doc.lineAt(Math.min(ln, doc.lineCount - 1)).text : '';
      const mk = projectsearch.makeRe(this.q, this.opts);
      let c = Math.max(0, h.col - 1), len = h.len;
      if (mk.re && real) { mk.re.lastIndex = 0; const m = mk.re.exec(real); if (m) { c = m.index; len = m[0].length; } }
      const sel = new vscode.Range(new vscode.Position(ln, c), new vscode.Position(ln, c + len));
      const ed = await vscode.window.showTextDocument(doc, { preview: true, selection: sel });
      if (ed && ed.revealRange) ed.revealRange(sel, vscode.TextEditorRevealType.InCenter);
      return t;
    } catch (e) {
      vscode.window.showErrorMessage('開不了 ' + h.file + '：' + (e && e.message || e));
      return null;
    }
  }

  getChildren(n) {
    const res = this.res;
    if (!res) return [];
    if (!n) {
      return this.roots.map(r => ({ type: 'area', r, hits: res.hits.filter(h => h.area === r.area) })).filter(a => a.hits.length);
    }
    if (n.type === 'area') {
      const by = new Map();
      for (const h of n.hits) { if (!by.has(h.file)) by.set(h.file, []); by.get(h.file).push(h); }
      return Array.from(by, ([file, hs]) => ({ type: 'file', file, rel: hs[0].rel, area: n.r.area, hits: hs }));
    }
    if (n.type === 'file') return n.hits.map(h => ({ type: 'hit', h }));
    return [];
  }

  getTreeItem(n) {
    const S = vscode.TreeItemCollapsibleState;
    if (n.type === 'area') {
      const it = new vscode.TreeItem(n.r.label, S.Expanded);
      it.description = n.hits.length + ' 筆';
      it.tooltip = n.r.root;
      it.iconPath = new vscode.ThemeIcon(n.r.area === 'golden' ? 'history' : n.r.area === 'web' ? 'globe' : 'symbol-class');
      it.id = 'ps:' + n.r.area;
      return it;
    }
    if (n.type === 'file') {
      const it = new vscode.TreeItem(path.basename(n.file), S.Collapsed);
      const dir = path.dirname(n.rel);
      it.description = (dir && dir !== '.' ? dir + '　' : '') + n.hits.length + ' 筆' + (n.hits[0].enc === 'big5' ? '　Big5' : '');
      it.tooltip = n.file;
      it.resourceUri = vscode.Uri.file(n.file);
      it.id = 'ps:' + n.area + ':' + n.file;
      return it;
    }
    const h = n.h;
    const lead = h.text.length - h.text.replace(/^\s+/, '').length;
    const shown = h.text.trim();
    const label = h.line + ': ' + shown;
    const at = String(h.line).length + 2 + Math.max(0, h.col - 1 - lead);
    // (a TreeItemLabel: the keyword highlighted, as in VS Code's Search view)
    const it = new vscode.TreeItem({ label, highlights: [[at, Math.min(label.length, at + h.len)]] }, S.None);
    it.tooltip = h.rel + ':' + h.line + ':' + h.col + '\n' + shown;
    it.command = { command: 'ht9045Designer.projectSearchOpen', title: '開啟', arguments: [h] };
    it.id = 'ps:' + h.area + ':' + h.file + ':' + h.line + ':' + h.col;
    return it;
  }
}

// ---------------------------------------------------------------------------
/**
 * CSV 表格: a .csv as a table, edited like Excel (media/csv.js). Every change is a replacement of only
 * the text of the cells it changes (lib/csvtable.js) in VS Code's document -- the machine's settings are
 * CSV (Mot_Table.csv, IO_Table.csv), so nothing else of the file may move. Saving (Ctrl+S) and undo are
 * VS Code's. A file VS Code could not read right (Big5 read as UTF-8) is shown, not edited.
 */
class CsvTableEditor {
  constructor(hub) {
    this.hub = hub;
    this.open = new Set();
  }

  resolveCustomTextEditor(doc, panel) {
    const media = vscode.Uri.joinPath(this.hub.ctx.extensionUri, 'media');
    const w = panel.webview;
    w.options = { enableScripts: true, enableCommandUris: false, localResourceRoots: [media] };
    const nonce = crypto.randomBytes(16).toString('base64');
    // style-src 'unsafe-inline': the table places every cell with style="left:…;top:…" (only the rows in sight
    // are drawn); without it VS Code drops those and every cell lands in the corner (EastSun 20260930's screenshot)
    w.html = '<!DOCTYPE html><html lang="zh-Hant"><head><meta charset="UTF-8">' +
      '<meta http-equiv="Content-Security-Policy" content="default-src \'none\'; style-src ' + w.cspSource + ' \'unsafe-inline\'; script-src \'nonce-' + nonce + '\';">' +
      '<meta name="viewport" content="width=device-width, initial-scale=1">' +
      '<link rel="stylesheet" href="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'csv.css')) + '"></head><body><div id="root"></div>' +
      '<script nonce="' + nonce + '" src="' + w.asWebviewUri(vscode.Uri.joinPath(media, 'csv.js')) + '"></script></body></html>';
    const t = { doc, panel, p: null, first: true, timer: null, safe: null, posted: 0 };
    this.open.add(t);
    const subs = [
      w.onDidReceiveMessage(m => this.onMessage(t, m)),
      vscode.workspace.onDidChangeTextDocument(e => {
        if (e.document !== doc || !e.contentChanges.length) return;
        clearTimeout(t.timer);
        t.timer = setTimeout(() => this.send(t), 60);
      }),
    ];
    panel.onDidDispose(() => { clearTimeout(t.timer); subs.forEach(s => s.dispose()); this.open.delete(t); if (t.editing) vscode.commands.executeCommand('setContext', 'ht9045Designer.csvEditing', false); });
    // (another editor in front: no cell of this one is being edited for VS Code's keys)
    subs.push(panel.onDidChangeViewState(e => { if (!e.webviewPanel.active && t.editing) { t.editing = false; vscode.commands.executeCommand('setContext', 'ht9045Designer.csvEditing', false); } }));
  }

  /** Whether the text can be written back: once per open (the bytes on disk vs what VS Code decoded). */
  safety(t) {
    if (t.safe) return t.safe;
    let bytes = null;
    try { if (t.doc.uri.scheme === 'file') bytes = fs.readFileSync(t.doc.uri.fsPath); } catch (e) { bytes = null; }
    t.safe = csvtable.decodeSafe(bytes, t.doc.getText());
    if (t.doc.uri.scheme !== 'file' && t.doc.uri.scheme !== 'untitled') t.safe = { ok: false, why: '這個檔案是唯讀的（' + t.doc.uri.scheme + '）。' };
    return t.safe;
  }

  send(t) {
    t.p = csvtable.parseDoc(t.doc.getText());
    const s = this.safety(t);
    t.posted++;
    t.panel.webview.postMessage({
      type: 'data', rows: t.p.rows.map(r => r.cells.map(c => c.v)), delim: t.p.delim,
      readOnly: !s.ok, why: s.why || '', file: path.basename(t.doc.uri.fsPath || t.doc.fileName || ''), first: t.first,
    });
    t.first = false;
  }

  /** Replacements [{ s, e, text }] on the document as ONE edit (= one undo step). */
  async apply(t, reps, what) {
    if (!reps.length) return true;
    const s = this.safety(t);
    if (!s.ok) { vscode.window.showWarningMessage(s.why); this.send(t); return false; }
    const we = new vscode.WorkspaceEdit();
    for (const r of reps) we.replace(t.doc.uri, new vscode.Range(t.doc.positionAt(r.s), t.doc.positionAt(r.e)), r.text);
    const ok = await vscode.workspace.applyEdit(we);
    if (!ok) { vscode.window.showWarningMessage('CSV 表格：' + what + '沒有寫進去（檔案可能是唯讀的）。'); this.send(t); }
    return ok;
  }

  async onMessage(t, m) {
    if (!m || typeof m.type !== 'string') return null;
    // the offsets of the text as it is NOW (two quick edits: the second must not use the first's positions)
    if (/^(edit|paste|insertRows|deleteRows)$/.test(m.type)) t.p = csvtable.parseDoc(t.doc.getText());
    switch (m.type) {
      case 'ready':
        this.send(t);
        return null;
      case 'edit': {
        const ch = (Array.isArray(m.changes) ? m.changes : []).filter(c => c && Number.isInteger(c.r) && Number.isInteger(c.c) && c.r >= 0 && c.c >= 0)
          .map(c => ({ r: c.r, c: c.c, v: String(c.v == null ? '' : c.v) }));
        return this.apply(t, csvtable.cellEdits(t.p, ch), '修改');
      }
      case 'paste': {
        // Excel's clipboard (tab separated) from the top-left cell of the selection; a bigger selection the block fits
        // a whole number of times is filled with it; a cut block moves (lib/csvtable.js pasteChanges) -- one edit
        const grid = csvtable.fromTsv(m.text);
        const int = (x, d) => (Number.isInteger(x) && x >= 0 ? x : d);
        const r0 = int(m.r, 0), c0 = int(m.c, 0);
        const sel = { r0, c0, r1: Math.max(r0, int(m.r1, r0)), c1: Math.max(c0, int(m.c1, c0)) };
        const cut = m.cut && Number.isInteger(m.cut.r0) ? { r0: int(m.cut.r0, 0), c0: int(m.cut.c0, 0), r1: int(m.cut.r1, 0), c1: int(m.cut.c1, 0) } : null;
        const pc = csvtable.pasteChanges(t.p, grid, sel, cut);
        const ok = await this.apply(t, csvtable.cellEdits(t.p, pc.changes), cut ? '搬移' : '貼上');
        if (ok) t.panel.webview.postMessage({ type: 'select', r: pc.rect.r0, c: pc.rect.c0, r1: pc.rect.r1, c1: pc.rect.c1 });
        return ok;
      }
      case 'editing':
        // a cell is being edited: VS Code's Ctrl+Z (the FILE's undo) is held off -- the table takes back the typing
        t.editing = !!m.on;
        return vscode.commands.executeCommand('setContext', 'ht9045Designer.csvEditing', t.editing).then(() => true, () => false);
      case 'copy':
        return vscode.env.clipboard.writeText(csvtable.toTsv(Array.isArray(m.grid) ? m.grid : [])).then(() => true, () => false);
      case 'readClip':
        // the table's right-click 貼上: the clipboard back to it, pasted there like Ctrl+V (a webview cannot read it)
        return vscode.env.clipboard.readText().then(tx => { t.panel.webview.postMessage({ type: 'clip', text: String(tx || '') }); return true; }, () => false);
      case 'insertRows':
        return this.apply(t, csvtable.insertRows(t.p, Math.max(0, m.r | 0), m.count), '插入列');
      case 'deleteRows':
        return this.apply(t, csvtable.deleteRows(t.p, Math.max(0, m.r | 0), m.count), '刪除列');
      case 'openText':
        // (asked for on purpose: no "use the table?" question for it)
        if (this.hub.csvAsked) this.hub.csvAsked.add(t.doc.uri.fsPath.toLowerCase());
        return vscode.commands.executeCommand('vscode.openWith', t.doc.uri, 'default');
    }
    return null;
  }
}

module.exports = { activate, deactivate };
