'use strict';
// AI(W906-HTDESIGNER) 20260929: run extension.js against test\fake_vscode.js with a real
// selection recorded by the probe in headless Edge (probe_test.ps1 writes
// %TEMP%\htdesigner_probe\<page>.result.json). Checks the whole path:
// page -> DFM -> events -> web / C++ / BCB6 targets -> opening them.
//   argv: <result.json> <reportFile>
const Module = require('module');
const fs = require('fs');
const path = require('path');
const fake = require('./fake_vscode');
const he2 = require('../lib/htmledit');
const hb2 = require('../lib/htmlblock');

const origResolve = Module._resolveFilename;
Module._resolveFilename = function (req, parent, ...rest) {
  if (req === 'vscode') return require.resolve('./fake_vscode');
  return origResolve.call(this, req, parent, ...rest);
};

const out = [];
let pass = 0, fail = 0;
const log = s => out.push(s);
const ok = (c, n, x) => { if (c) pass++; else fail++; log((c ? 'PASS  ' : 'FAIL  ') + n + (x ? '   ' + x : '')); };

async function main() {
  const [resultFile] = process.argv.slice(2);
  const R = JSON.parse(fs.readFileSync(resultFile, 'utf8'));
  const PORT = path.resolve(__dirname, '..', '..', '..');
  const PAGE = path.join(PORT, '..', 'web', 'page', 'Setup.HotPlate.html');
  fake._folders = [{ uri: fake.Uri.file(PORT) }];
  // (defaultView 'design' for the run: 0.140's default 'wpf' opens the HTML below every designer -- its own test below)
  Object.assign(fake._config, { eventJump: 'ask', syncSource: true, defaultView: 'design' });

  const ext = require('../extension');
  const memento = { _m: {}, get(k, d) { return k in this._m ? this._m[k] : d; }, update(k, v) { this._m[k] = v; return Promise.resolve(); } };
  const ctx = { subscriptions: [], extensionUri: fake.Uri.file(path.resolve(__dirname, '..')), workspaceState: memento, extension: { id: 'ht9045.ht9045-html-designer', extensionPath: path.resolve(__dirname, '..'), packageJSON: JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')) } };
  const api = ext.activate(ctx);
  ok(!!fake._editor && !!fake._props, 'activate registers the custom editor and the properties view');

  // properties view
  const posted = [];
  let propsRecv = null;
  fake._props.resolveWebviewView({
    webview: {
      cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
      asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
      postMessage: m => { posted.push(m); return Promise.resolve(true); },
      onDidReceiveMessage: fn => { propsRecv = fn; return { dispose() {} }; },
    },
    onDidDispose: () => ({ dispose() {} }),
  });

  // designer on the real page
  const ORIGINAL = fs.readFileSync(PAGE, 'utf8');
  let text = ORIGINAL;
  let lineStarts = [];
  const indexLines = () => { lineStarts = [0]; for (let i = 0; i < text.length; i++) if (text.charCodeAt(i) === 10) lineStarts.push(i + 1); };
  indexLines();
  const doc = {
    uri: fake.Uri.file(PAGE), fileName: PAGE, isDirty: false,
    getText: () => text,
    positionAt: off => { let lo = 0, hi = lineStarts.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (lineStarts[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - lineStarts[lo]); },
    offsetAt: p => lineStarts[p.line] + p.character,
    _setText: t => { text = t; indexLines(); doc.isDirty = true; },
  };
  fake._docs = { [doc.uri.toString()]: doc };
  const pagePosted = [];
  let pageRecv = null;
  const panel = {
    active: true, visible: true, viewColumn: 1,
    reveal() {},
    onDidChangeViewState: () => ({ dispose() {} }),
    onDidDispose: () => ({ dispose() {} }),
    webview: {
      cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
      asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
      postMessage: m => { pagePosted.push(m); return Promise.resolve(true); },
      onDidReceiveMessage: fn => { pageRecv = fn; return { dispose() {} }; },
    },
  };
  fake._editor.resolveCustomTextEditor(doc, panel);
  const html = panel.webview.html;
  ok(/Content-Security-Policy/.test(html) && /connect-src https:\/\/\*\.vscode-cdn\.net;/.test(html) && /frame-src 'none'/.test(html),
    'webview html carries the no-network CSP');
  ok(html.indexOf('probe.js') > 0 && html.indexOf('probe.js') < html.indexOf('theme.js'), 'probe loads before the page scripts');

  pageRecv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });
  await new Promise(r => setTimeout(r, 50));   // init waits for the page analysis
  const init = pagePosted.find(m => m.type === 'init');
  ok(init && init.mode === 'design' && init.classes && init.classes.spbSave === 'TSpeedButton' && init.classes['@form'] === 'TfHotPlate',
    'ready -> init(mode=design, IR classes for the probe)');
  // WPF's "Zoom by using" setting: Ctrl+wheel by default; changed -> the open designer is told at once, the status tip says it
  const wzN = pagePosted.length;
  fake._config.zoomWheel = 'alt';
  fake._configChanged('zoomWheel');
  const wzAlt = pagePosted.slice(wzN).filter(m => m.type === 'wheelZoom').map(m => m.how).join(',');
  const wzTip = String(fake._status ? fake._status.tooltip : (api.hub.status && api.hub.status.tooltip) || '');
  delete fake._config.zoomWheel;
  fake._configChanged('zoomWheel');
  const wzBack = pagePosted.slice(wzN).filter(m => m.type === 'wheelZoom').map(m => m.how).join(',');
  ok(init && init.wheelZoom === 'ctrl' && wzAlt === 'alt' && wzBack === 'alt,ctrl' && /Alt＋滾輪/.test(wzTip),
    'zoomWheel setting (WPF "Zoom by using"): init says ctrl; set to alt -> the designer told at once (then back to ctrl), the status tip says Alt+wheel',
    'init=' + (init && init.wheelZoom) + ' alt=' + wzAlt + ' back=' + wzBack + ' tip=' + wzTip.slice(0, 80));
  // the snapSpacing setting (WPF's snapping spacing): 8 by default; changed -> told at once; nonsense / out of range clamped
  const ssN = pagePosted.length;
  fake._config.snapSpacing = 12; fake._configChanged('snapSpacing');
  fake._config.snapSpacing = 500; fake._configChanged('snapSpacing');
  fake._config.snapSpacing = 'x'; fake._configChanged('snapSpacing');
  delete fake._config.snapSpacing; fake._configChanged('snapSpacing');
  const ssSent = pagePosted.slice(ssN).filter(m => m.type === 'snapSpacing').map(m => m.px).join(',');
  ok(init && init.snapSpacing === 8 && ssSent === '12,64,8,8', 'snapSpacing setting: init says 8; 12 / 500 (-> 64) / nonsense (-> 8) / back to the default, each told at once',
    'init=' + (init && init.snapSpacing) + ' sent=' + ssSent);
  // WPF's "Default zoom setting": Last Used (the default) -- the page opens again at the zoom it had; Fit All -- the
  // page is asked to fit once (a redraw keeps its zoom); the kept zoom goes when it is back at 100%
  {
    const mkPanel = () => {
      const p = { posted: [], recv: null, disp: null, active: false, visible: true, viewColumn: 2, reveal() {},
        onDidChangeViewState: () => ({ dispose() {} }), onDidDispose: fn => { p.disp = fn; return { dispose() {} }; },
        webview: { cspSource: 'https://*.vscode-cdn.net', options: null, html: '', asWebviewUri: u => ({ toString: () => 'https://x/' + u.fsPath.replace(/\\/g, '/') }),
          postMessage: m => { p.posted.push(m); return Promise.resolve(true); }, onDidReceiveMessage: fn => { p.recv = fn; return { dispose() {} }; } } };
      return p;
    };
    const initOf = async (p, n) => { for (let i = 0; i < 150 && p.posted.filter(m => m.type === 'init').length < n; i++) await new Promise(r => setTimeout(r, 20)); return p.posted.filter(m => m.type === 'init'); };
    const dzDef = api.hub.defaultZoom();
    fake._config.defaultZoom = 'nonsense';
    const dzBad = api.hub.defaultZoom();
    delete fake._config.defaultZoom;
    pageRecv({ __htd: 1, type: 'zoom', zoom: 1.5 });   // this page zoomed to 150%
    const dzKept = api.hub.loadZoom(PAGE);
    const p2 = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, p2);
    p2.recv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });
    const i2 = (await initOf(p2, 1))[0];
    if (p2.disp) p2.disp();
    fake._config.defaultZoom = 'fit';
    const p3 = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, p3);
    p3.recv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });
    await initOf(p3, 1);
    p3.recv({ __htd: 1, type: 'ready', url: 'vscode-webview://x/index.html' });   // (a redraw)
    const i3 = await initOf(p3, 2);
    if (p3.disp) p3.disp();
    delete fake._config.defaultZoom;
    pageRecv({ __htd: 1, type: 'zoom', zoom: 1 });
    const dzGone = api.hub.loadZoom(PAGE) === 1 && !Object.keys(memento.get('htd.zoom', {}) || {}).length;
    ok(dzDef === 'last' && dzBad === 'last' && dzKept === 1.5 && i2 && i2.zoom === 1.5 && !i2.fitOnOpen &&
      i3.length === 2 && i3[0].zoom === 1 && i3[0].fitOnOpen === true && !i3[1].fitOnOpen && dzGone,
      'defaultZoom setting (WPF "Default zoom"): Last Used by default -- the page opens again at 150%; Fit All -- fit once, a redraw keeps it; 100% again = nothing kept',
      JSON.stringify({ dzDef, dzBad, dzKept, i2: i2 && [i2.zoom, i2.fitOnOpen], i3: i3.map(m => [m.zoom, m.fitOnOpen]), dzGone }));
    // WPF's "Default document view": design alone by default; split -> the HTML opens beside it, the focus stays
    const dvDef = api.hub.defaultView();
    const nOpen = fake._state.opened.length;
    const p4 = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, p4);
    await new Promise(r => setTimeout(r, 30));
    const dvNone = fake._state.opened.length === nOpen;
    if (p4.disp) p4.disp();
    fake._config.defaultView = 'split';
    const p5 = mkPanel();
    fake._editor.resolveCustomTextEditor(doc, p5);
    await new Promise(r => setTimeout(r, 30));
    const dvOpened = fake._state.opened.slice(nOpen);
    if (p5.disp) p5.disp();
    // 0.140 'wpf' (the package's default now): the design above, the HTML below (row 2); a C++ file opened closes the page
    delete fake._config.defaultView;
    const wDef = api.hub.defaultView();
    const pkgDef = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.configuration.properties['ht9045Designer.defaultView'].default;
    const wCalls = fake._calls.length, wOpen0 = fake._state.opened.length;
    const p6 = mkPanel();
    p6.dispose = () => { p6.disposed = true; if (p6.disp) p6.disp(); };
    fake._editor.resolveCustomTextEditor(doc, p6);
    await new Promise(r => setTimeout(r, 60));
    const wLayout = fake._calls.slice(wCalls).find(c => c.name === 'exec' && c.args[0] === 'vscode.setEditorLayout');
    const wOpened = fake._state.opened.slice(wOpen0);
    const wSess = !!api.hub.wpfSession;
    // (a .cpp right away is the page's own opening moments: ignored; later = closes)
    await api.hub.onActiveEditor({ document: { uri: fake.Uri.file(path.join(PORT, 'forms', 'fHotPlate.cpp')) } });
    const wEarly = !!api.hub.wpfSession;
    if (api.hub.wpfSession) api.hub.wpfSession.at = 0;
    await api.hub.onActiveEditor({ document: { uri: fake.Uri.file(path.join(PORT, '..', 'web', 'page', 'main.html')) } });
    const wHtmlKeeps = !!api.hub.wpfSession;
    // 0.141 (EastSun's screenshot: Alert.MotionView / Data.LotInfo / Data.CounterClear / Data.ContactCT left open
    // beside a .cpp): EVERY .html tab closes -- the designer's and the text ones -- not one with unsaved changes; the
    // .cpp stays. (A fake tab list: the fallback without one would close every designer of this run.)
    const tabOf = (f, dirty, custom) => ({ input: custom ? { uri: fake.Uri.file(f), viewType: 'ht9045Designer.editor' } : { uri: fake.Uri.file(f) }, isDirty: !!dirty, label: path.basename(f) });
    const WP = path.join(PORT, '..', 'web', 'page');
    const tabs = [tabOf(path.join(PORT, 'forms', 'fHotPlate.cpp')), tabOf(PAGE, false, true), tabOf(PAGE), tabOf(path.join(WP, 'Alert.MotionView.html')),
      tabOf(path.join(WP, 'Data.LotInfo.html')), tabOf(path.join(WP, 'Data.CounterClear.html'), true), tabOf(path.join(WP, 'Data.ContactCT.html'))];
    let closedTabs = [];
    fake.window.tabGroups = { all: [{ tabs }], close: async t => { closedTabs = (Array.isArray(t) ? t : [t]).map(x => x.label + (x.input.viewType ? '(設計)' : '')); return true; } };
    await api.hub.onActiveEditor({ document: { uri: fake.Uri.file(path.join(PORT, 'forms', 'fHotPlate.cpp')) } });
    delete fake.window.tabGroups;
    const keptMsg = fake._calls.slice(wCalls).some(c => /沒有關（還沒存檔）：Data\.CounterClear\.html/.test(String(c.args && c.args[0])));
    const wJoin = fake._calls.slice(wCalls).some(c => c.name === 'exec' && c.args[0] === 'workbench.action.joinAllGroups');
    const wGone = !api.hub.wpfSession && closedTabs.join(',') === 'Setup.HotPlate.html(設計),Setup.HotPlate.html,Alert.MotionView.html,Data.LotInfo.html,Data.ContactCT.html' && keptMsg;
    if (p6.disp) p6.disp();
    fake._config.defaultView = 'design';
    ok(dvDef === 'design' && dvNone && dvOpened.length === 1 && dvOpened[0].uri.toString() === doc.uri.toString() && dvOpened[0].viewColumn === 3 &&
      pkgDef === 'wpf' && wDef === 'wpf' && wLayout && wOpened.length === 1 && wOpened[0].viewColumn === 2 &&
      wSess && wEarly && wHtmlKeeps && wJoin && wGone,
      'defaultView (WPF "Default document view"): design = alone; split = the HTML beside (column 3); 0.140 wpf (the default) = the design above, the HTML below (row 2), a C++ file opened closes the page and joins the area to one (an .html does not); 0.141: every .html tab closes (designer and text), an unsaved one stays (said), the .cpp stays',
      JSON.stringify({ dvDef, dvNone, opened: dvOpened.map(o => [path.basename(o.uri.fsPath), o.viewColumn]), pkgDef, wDef, wLayout: !!wLayout, wOpened: wOpened.map(o => o.viewColumn), wSess, wEarly, wHtmlKeeps, wJoin, wGone }));
    fake._state.opened.splice(nOpen);
    // 0.141 the tab buttons (EastSun: "可以調整左右的 可以看其他方案 並且有選項可以關閉其他方案"): VS Code's own commands.
    // 0.142 (EastSun: "左右可以滾輪 不是選擇檔案"): no ◀ ▶ (they switched the file); the tab row's scrollbar is large instead
    const tbC = fake._calls.length;
    for (const c of ['tabList', 'tabCloseOthers']) await fake._cmds['ht9045Designer.' + c]();
    const tbRan = fake._calls.slice(tbC).filter(c => c.name === 'exec').map(c => c.args[0]).join(',');
    const arrPkgTop = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
    const tbMenu = (arrPkgTop.contributes.menus['editor/title'] || []).filter(m => /^ht9045Designer\.tab/.test(m.command)).map(m => m.command.split('.')[1]).join(',');
    const tbGone = !fake._cmds['ht9045Designer.tabPrev'] && !fake._cmds['ht9045Designer.tabNext'];
    // the workbench.editor settings: a stand-in with inspect / update
    const wbSet = {}, wbUpd = [], gc0 = fake.workspace.getConfiguration;
    fake.workspace.getConfiguration = sec => sec !== 'workbench.editor' ? gc0(sec) : {
      get: k => wbSet[k], inspect: k => ({ globalValue: wbSet[k] }), update: async (k, v, t) => { wbUpd.push([k, v, t]); wbSet[k] = v; },
    };
    const sbFirst = api.hub.applyTabScrollbar(), sbVal1 = wbSet.titleScrollbarSizing;
    wbSet.titleScrollbarSizing = 'default';
    const sbKeep = !api.hub.applyTabScrollbar() && wbSet.titleScrollbarSizing === 'default';
    await fake._cmds['ht9045Designer.tabScrollbar'](); const sbOn = wbSet.titleScrollbarSizing;
    await fake._cmds['ht9045Designer.tabScrollbar'](); const sbOff = wbSet.titleScrollbarSizing;
    fake.workspace.getConfiguration = gc0;
    const sbOk = sbFirst && sbVal1 === 'large' && sbKeep && sbOn === 'large' && sbOff === 'default' && wbUpd.every(u => u[0] === 'titleScrollbarSizing' && u[2] === 1);
    // 0.142 (EastSun: "元件 工具箱 屬性與事件 可以加個圖示" / "資料夾也需要圖示")
    const vws = arrPkgTop.contributes.views['ht9045-designer'].filter(v => v.when !== 'false');
    const vwOk = vws.every(v => v.icon && /^\S+ /.test(v.name)) && vws.map(v => v.name.split(' ')[1]).join(',') === '方案總管,元件,工具箱,屬性與事件';
    const tsol = api.hub.solution;
    const tProj = (await tsol.getChildren((await tsol.getChildren())[0])).find(p => p.p.area === 'port');
    const tDir = tProj && (await tsol.getChildren(tProj)).find(k => k.type === 'dir');
    const tDirIcon = tDir && tsol.getTreeItem(tDir).iconPath;
    const dirOk = !!tDirIcon && tDirIcon.id === 'symbol-folder';   // 0.143: never 'folder' / 'file' (VS Code draws those from the icon theme, Seti has no folder)
    const pgFolderIds = (await api.hub.pages.getChildren()).map(g => api.hub.pages.getTreeItem(g).iconPath).filter(Boolean).map(i => i.id);
    const pgFolderOk = pgFolderIds.length > 0 && !pgFolderIds.some(id => id === 'folder' || id === 'file');
    ok(tbRan === 'workbench.action.showAllEditorsByMostRecentlyUsed,workbench.action.closeOtherEditors' && tbMenu === 'tabList,tabCloseOthers' && tbGone && sbOk && vwOk && dirOk && pgFolderOk,
      'tab buttons on every editor\'s title bar: the open ones, close the others (0.142: no ◀ ▶); the tab row\'s scrollbar large once when never set (an explicit value stays), tabScrollbar toggles it; the side bar\'s views have icons; 方案總管 folders have a folder icon',
      JSON.stringify({ tbRan, tbMenu, tbGone, sbFirst, sbVal1, sbKeep, sbOn, sbOff, vws: vws.map(v => v.name + ' ' + v.icon), dir: tDir && tDir.name, icon: tDirIcon && tDirIcon.id, pgFolderIds: [...new Set(pgFolderIds)] }));
    // 0.147 側欄分區外框 (EastSun: "每個項目 你可以像 visual studio 外框 ... 這樣比較好辨識嗎?"): the section title bars' colours,
    // only for the theme in use, never over the user's own, once; the toggle takes only ours away
    {
      const h = api.hub, gc1 = fake.workspace.getConfiguration, ctx0 = h.ctx;
      const wb = { colorTheme: 'Default Dark Modern', colorCustomizations: { '[Default Dark Modern]': { 'sideBarSectionHeader.foreground': '#123456', 'sideBarSectionHeader.background': '#2d2d30' }, '[Other]': { x: 1 } } };   // (0.149: #2d2d30 = 0.147's own colour -> replaced)
      fake.workspace.getConfiguration = sec => sec !== 'workbench' ? gc1(sec) : {
        get: k => wb[k], inspect: k => ({ globalValue: wb[k] }), update: async (k, v) => { wb[k] = JSON.parse(JSON.stringify(v)); },
      };
      const gsv = {};
      h.ctx = Object.assign({}, ctx0, { globalState: { get: k => gsv[k], update: (k, v) => { gsv[k] = v; return Promise.resolve(); } } });
      const fr1 = h.applySectionFrames();
      await new Promise(r => setTimeout(r, 10));
      const t1 = JSON.stringify(wb.colorCustomizations['[Default Dark Modern]']);
      const fr2 = h.applySectionFrames();
      const off = await fake._cmds['ht9045Designer.sectionFrames']();
      const t2 = JSON.stringify(wb.colorCustomizations['[Default Dark Modern]']);
      const on = await fake._cmds['ht9045Designer.sectionFrames']();
      const t3 = wb.colorCustomizations['[Default Dark Modern]'];
      h.ctx = ctx0; fake.workspace.getConfiguration = gc1;
      ok(fr1 && !fr2 && t1 === JSON.stringify({ 'sideBarSectionHeader.foreground': '#123456', 'sideBarSectionHeader.background': '#2d2d30', 'sideBarSectionHeader.border': '#3f3f46' }) &&
        off === false && t2 === JSON.stringify({ 'sideBarSectionHeader.foreground': '#123456' }) && on === true && t3['sideBarSectionHeader.background'] === '#2d2d30' &&
        wb.colorCustomizations['[Other]'].x === 1,
        'section frames: the title bars get a band and a border for the theme in use, once; the user\'s own colour stays; the toggle takes only ours away and back; other themes untouched',
        JSON.stringify({ fr1, fr2, t1, off, t2, on, t3 }));
    }
    // 0.148 方案總管 as one piece (EastSun: "方案總管整個重做成一格" -- the box right under the title, the tree under it,
    // what does not match hidden; "BCB 原始碼就不用出現了 編譯用不到的東西")
    {
      const pnl = api.hub.solPanel, prov = fake._webviewViews && fake._webviewViews['ht9045Designer.solution'];
      // (VS Code's own icons: the app folder of the VS Code on this PC, when there is one)
      const vsDir = path.join(process.env.LOCALAPPDATA || '', 'Programs', 'Microsoft VS Code');
      let appRoot = '';
      try { for (const d of fs.readdirSync(vsDir)) { const a = path.join(vsDir, d, 'resources', 'app'); if (fs.existsSync(path.join(a, 'out', 'media', 'codicon.ttf'))) { appRoot = a; break; } } } catch (e) { /* none */ }
      const env0 = fake.env.appRoot;
      if (appRoot) fake.env.appRoot = appRoot;
      let pHtml = '', pRecv = null; const pPosts = [];
      if (prov) prov.resolveWebviewView({
        webview: { options: {}, cspSource: 'csp', asWebviewUri: u => u, postMessage: m => { pPosts.push(m); return Promise.resolve(true); },
          set html(h) { pHtml = h; }, get html() { return pHtml; }, onDidReceiveMessage: fn => { pRecv = fn; return { dispose() {} }; } },
        visible: true, onDidChangeVisibility: () => ({ dispose() {} }), onDidDispose: () => ({ dispose() {} }),
      });
      const lastRows = () => { for (let i = pPosts.length - 1; i >= 0; i--) if (pPosts[i].type === 'rows') return pPosts[i]; return null; };
      // (the newest rows: a render of its own after the timer -- a fixed wait lost to a busy PC)
      const settle = async () => { await new Promise(r => setTimeout(r, 40)); await pnl.render(); };
      await fake._cmds['ht9045Designer.solutionClearFilter']();
      if (pRecv) await pRecv({ type: 'ready', q: '' });
      await settle();
      const r0 = lastRows() || { rows: [] };
      // 0.151 the toolbar: the run bar's state comes with ready; a button = a command (only this extension's / VS Code's debug)
      const runMsg = pPosts.filter(m => m.type === 'run').pop();
      const tbRun0 = !!(runMsg && runMsg.s && runMsg.s.b && runMsg.s.b.start && runMsg.s.b.start.cmd === 'ht9045Designer.run.buildAndStart' && typeof runMsg.s.sim === 'boolean');
      const cx0 = fake._calls.length;
      await pRecv({ type: 'cmd', id: 'ht9045Designer.solutionCollapseAll' });
      await pRecv({ type: 'cmd', id: 'workbench.action.files.delete' });
      const cxRan = fake._calls.slice(cx0).filter(c => c.name === 'exec').map(c => c.args[0]).join(',');
      const tbOk = tbRun0 && cxRan === 'ht9045Designer.solutionCollapseAll';
      const top = r0.rows.map(r => r.l).slice(0, 3).join(' | ');
      const portRow = r0.rows.find(r => r.d === 1 && /C\+\+/.test(r.l));
      const noGold = !r0.rows.some(r => /BCB6/.test(r.l));
      // open C++ 移植樹: its folders, a folder with VS Code's folder icon (codicon), closed
      if (portRow) await pRecv({ type: 'toggle', id: portRow.id });
      await settle();
      const r1 = lastRows() || { rows: [] };
      const formsRow = r1.rows.find(r => r.d === 2 && r.l === 'forms');
      const kids1 = r1.rows.filter(r => r.d === 2).length;
      // type in the box: only fHotPlate's files and their folders, open; a file with its Seti icon
      pPosts.length = 0;
      await pRecv({ type: 'query', q: 'fHotPlate' });
      await settle();
      const r2 = lastRows() || { rows: [] };
      const r2Files = r2.rows.filter(r => r.ctx === 'htdSlnFile').map(r => r.l);
      const hpCpp = r2.rows.find(r => r.l === 'fHotPlate.cpp');
      const noSetq = !pPosts.some(m => m.type === 'setq');
      // (the rows for panels_render.ps1: media/solution.js draws them in Edge)
      try { const dd = path.join(require('os').tmpdir(), 'htdesigner_panels'); fs.mkdirSync(dd, { recursive: true }); fs.writeFileSync(path.join(dd, 'solution_rows.json'), JSON.stringify({ rows: r2.rows }), 'utf8'); } catch (e) { /* the panels layer says so */ }
      const desc2 = pnl.description;
      // a click on the file = it opens
      // (the item's own command -- solutionOpen with the node; the fake records it, VS Code runs it)
      const ac0 = fake._calls.length;
      if (hpCpp) await pRecv({ type: 'activate', id: hpCpp.id });
      const acCall = fake._calls.slice(ac0).find(c => c.name === 'exec' && c.args[0] === 'ht9045Designer.solutionOpen');
      const opened = acCall ? { uri: { fsPath: hpCpp.l } } : null;
      // right-click "在這裡搜尋…" etc. get { htdId }
      const ctxNode = hpCpp ? pnl.nodeOf({ webviewSection: 'htdSlnFile', htdId: hpCpp.id }) : null;
      await fake._cmds['ht9045Designer.solutionCopyPath']({ htdId: hpCpp && hpCpp.id });
      const clip = await fake.env.clipboard.readText();
      // set elsewhere -> the box shows it; Ctrl+F / Ctrl+; = into the box
      pPosts.length = 0;
      await fake._cmds['ht9045Designer.solutionFilter']('cHotPlate');
      const setq = pPosts.find(m => m.type === 'setq');
      const fc0 = fake._calls.length;
      pPosts.length = 0;
      const boxR = await fake._cmds['ht9045Designer.solutionFilter']();
      const focusCmd = fake._calls.slice(fc0).some(c => c.name === 'exec' && c.args[0] === 'ht9045Designer.solution.focus') && pPosts.some(m => m.type === 'focus');
      await fake._cmds['ht9045Designer.solutionClearFilter']();
      await settle();
      // sync with the active document: the folders above it open, it selected
      const port1 = api.hub.solution.projects().find(p => p.area === 'port');
      const rvF = port1 ? path.join(port1.root, 'forms', 'fHotPlate.cpp') : '';
      pPosts.length = 0;
      const rv = rvF ? await fake._cmds['ht9045Designer.solutionReveal'](rvF) : null;
      await new Promise(r => setTimeout(r, 40));
      const r3 = lastRows() || { rows: [] };
      const rvRow = r3.rows.find(r => r.id === (rv && rv.id));
      const rvSel = r3.sel === (rv && rv.id) && !!rvRow;
      await fake._cmds['ht9045Designer.solutionCollapseAll']();
      await settle();
      const r4 = lastRows() || { rows: [] };
      const collapsed = !r4.rows.some(r => r.d >= 2);
      fake.env.appRoot = env0;
      const pkgP = arrPkgTop.contributes.views['ht9045-designer'];
      const pView = pkgP.find(v => v.id === 'ht9045Designer.solution');
      const ctxMenu = (arrPkgTop.contributes.menus['webview/context'] || []).filter(m => /webviewId == 'ht9045Designer\.solution'/.test(m.when)).map(m => m.command.split('.')[1]).join(',');
      const icOk = !appRoot || (!!(formsRow && formsRow.ic && formsRow.ic.t === 'c' && formsRow.ic.ch === 60035) && !!(hpCpp && hpCpp.ic && hpCpp.ic.t === 's' && hpCpp.ic.ch > 57000) && /htd-codicon/.test(pHtml) && /htd-seti/.test(pHtml));
      const got = { tbOk, cxRan, mark: hpCpp && hpCpp.h, prov: !!prov, html: /solution\.js/.test(pHtml) && /data-ch=/.test(pHtml), top, noGold, kids1, forms: formsRow && formsRow.tw, r2Files, noSetq, desc2,
        opened: opened ? path.basename(opened.uri.fsPath) : null, ctx: ctxNode && ctxNode.name, clip: path.basename(clip || ''), setq: setq && setq.q, box: boxR, focusCmd,
        rvSel, collapsed, view: pView && pView.type, noOldBox: !pkgP.some(v => v.id === 'ht9045Designer.solutionSearch'), ctxMenu, icOk, appRoot: !!appRoot };
      ok(got.prov && got.html && /^方案 '.+'（2 個專案） \| C\+\+ 移植樹 \| 網頁$/.test(top) && noGold && kids1 > 20 && got.forms === 1 &&
        r2Files.includes('fHotPlate.cpp') && r2Files.includes('fHotPlate.h') && !!hpCpp && JSON.stringify(hpCpp.h) === '[[0,9]]' && !r2Files.some(f => /fMain/i.test(f)) && noSetq && /「fHotPlate」\d+ 個檔/.test(desc2) &&
        got.opened === 'fHotPlate.cpp' && got.ctx === 'fHotPlate.cpp' && got.clip === 'fHotPlate.cpp' && got.setq === 'cHotPlate' && boxR && boxR.box && focusCmd &&
        tbOk && rvSel && collapsed && got.view === 'webview' && got.noOldBox && ctxMenu === 'solutionOpen,solutionSearchIn,solutionCopyPath,solutionRevealInOS' && icOk,
        '方案總管 in one piece: the box right under the title, the tree under it (C++ 移植樹 / 網頁 -- no BCB6); typing hides what does not match (fHotPlate: its files, open); a click opens; right-click items; Ctrl+F = the box; sync selects; collapse all; VS Code\'s own folder / file icons',
        JSON.stringify(got));
    }
    // 0.149 屬性與事件 with nothing selected: a button's panel, grey (EastSun: "還沒選到元件時 請用 button 參數 然後全部反灰")
    {
      const pv = api.hub.props, smp = pv && pv.sample ? pv.sample() : null;
      const txt = smp ? JSON.stringify(smp) : '';
      const posted = [];
      const pvView0 = pv ? pv.view : null;   // (the harness's own view: back after this)
      if (pv) pv.resolveWebviewView({
        webview: { options: {}, cspSource: 'csp', asWebviewUri: u => u, postMessage: m => { posted.push(m); return Promise.resolve(true); }, set html(h) {}, onDidReceiveMessage: () => ({ dispose() {} }) },
        onDidDispose: () => ({ dispose() {} }),
      });
      const sent = posted.find(m => m.type === 'sample');
      if (pv) pv.view = pvView0;
      ok(!!(smp && smp.sample && smp.comp && smp.comp.cls === 'TSpeedButton' && smp.events && smp.events.length && !/[A-Za-z]:\\\\/.test(txt) && !/htd_work/i.test(txt) && sent && sent.data === smp),
        '屬性與事件 with nothing selected: the panel gets a button\'s data (TSpeedButton, its events) to show grey; no path of this PC in it',
        JSON.stringify({ cls: smp && smp.comp && smp.comp.cls, events: smp && smp.events && smp.events.length, sent: !!sent }));
    }
    // 0.144 尋找 (EastSun: "我希望可以搜尋所有專案的關鍵字 可以選擇"): one box -- the keyword, the scope list, Aa / 全字 / 正規式
    {
      fake._config['find.window'] = false;   // (this block: the box; the window is the next block)
      const ps = api.hub.projSearch, cq0 = fake.window.createQuickPick, ae0 = Object.getOwnPropertyDescriptor(fake.window, 'activeTextEditor');
      const port0 = ps.areas().find(a => a.area === 'port');
      const cppF = port0 ? path.join(port0.root, 'forms', 'fHotPlate.cpp') : '';
      Object.defineProperty(fake.window, 'activeTextEditor', { configurable: true, get: () => ({ document: { uri: fake.Uri.file(cppF), getText: () => 'spbSave', getWordRangeAtPosition: () => ({}) }, selection: { isEmpty: true, active: {} } }) });
      let qp = null;
      fake.window.createQuickPick = () => (qp = { _fn: {}, show() { this.shown = true; }, hide() { this._fn.hide && this._fn.hide(); }, dispose() {},
        onDidTriggerButton(f) { this._fn.btn = f; }, onDidAccept(f) { this._fn.acc = f; }, onDidHide(f) { this._fn.hide = f; } });
      const pr = fake._cmds['ht9045Designer.find']();
      await new Promise(r => setTimeout(r, 10));
      const fScopes = qp.items.map(i => i.label.replace(/^\$\([^)]*\) /, '')).join(',');
      const fVal = qp.value, fAct = qp.activeItems[0] && qp.activeItems[0].scope, fAlways = qp.items.every(i => i.alwaysShow);
      const opt0 = ps.opts.wholeWord;
      qp._fn.btn(qp.buttons[1]);
      const fBtn = ps.opts.wholeWord === !opt0 && qp.buttons[1].iconPath.id === 'pass-filled';
      qp._fn.btn(qp.buttons[1]);
      qp.activeItems = [qp.items.find(i => i.scope === 'file')];
      await qp._fn.acc();
      const fRes = await pr;
      const fFiles = fRes ? [...new Set(fRes.hits.map(h => path.basename(h.file)))].join(',') : '-';
      // the last scope comes back; { q, pick } = no box (the golden project only)
      const r2 = await fake._cmds['ht9045Designer.find']({ q: 'spbSave', pick: 'project|golden' });
      const fGold = r2 ? [...new Set(r2.hits.map(h => h.area))].join(',') : '-';
      // 0.145 (EastSun: "你不能直接取代ctrl+F 就好了嗎?"): the first scope = VS Code's find in this file, with the box's options
      const ic0 = fake._calls.length;
      ps.opts.wholeWord = true;
      // (the fake records a command's arguments as String(): spy on the call itself for the object)
      const ex0 = fake.commands.executeCommand, icArgs = [];
      fake.commands.executeCommand = (id, ...a) => { if (id === 'editor.actions.findWithArgs') icArgs.push(a[0]); return ex0(id, ...a); };
      const r3 = await fake._cmds['ht9045Designer.find']({ q: 'XST1', pick: 'inline|' });
      fake.commands.executeCommand = ex0;
      ps.opts.wholeWord = false;
      const ic = icArgs[0];
      const fInline = !!(r3 && r3.inline && ic && ic.searchString === 'XST1' && ic.matchWholeWord === true && ic.isRegex === false && fake._calls.length > ic0);
      fake.window.createQuickPick = cq0;
      if (ae0) Object.defineProperty(fake.window, 'activeTextEditor', ae0); else delete fake.window.activeTextEditor;
      const pkgF = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const kb = pkgF.contributes.keybindings || [];
      const fKey = kb.some(k => k.command === 'ht9045Designer.find' && k.key === 'ctrl+shift+f') &&
        kb.some(k => k.command === 'ht9045Designer.find' && k.key === 'ctrl+f' && /editorFocus/.test(k.when) && /!findInputFocussed/.test(k.when) && /config\.ht9045Designer\.ctrlFFind/.test(k.when)) &&
        pkgF.contributes.configuration.properties['ht9045Designer.ctrlFFind'].default === true;
      const fMenu = (pkgF.contributes.menus['editor/title'] || []).some(m => m.command === 'ht9045Designer.find');
      ok(fScopes === '這個檔案（即時標示，F3 下一個）,目前檔案,目前專案,整個方案,C++ 移植樹,網頁,BCB6 原始碼（golden，Big5）' && fVal === 'spbSave' && fAct === 'inline' && fAlways && fBtn &&
        fFiles === 'fHotPlate.cpp' && fGold === 'golden' && fInline && ps.findLast === 'inline|' && fKey && fMenu,
        '尋找 (Ctrl+F / Ctrl+Shift+F): the word at the cursor, the scopes (this file live = VS Code\'s find, first and the default / file / project / solution / each project) always listed; Aa / 全字 / 正規式 buttons (also into VS Code\'s find); Enter = search in the scope picked; one project only; Ctrl+F in an editor, not in the find input, a setting turns it off',
        JSON.stringify({ fScopes, fVal, fAct, fAlways, fBtn, fFiles, last: ps.findLast, fGold, fInline, fKey, fMenu }));
      delete fake._config['find.window'];
    }
    // 尋找 as its own window (EastSun 1003: "你搜尋關鍵字的視窗 可以額外的視窗嗎 不要用內建的 並且我可以看到我之前搜尋了什麼"):
    // a webview window (floated), the word at the cursor, the scopes; a search -> its results in the window + the history
    // (kept, newest first, one per keyword+scope); a history row searches again; a result opens in the editor group
    // the user came from; Ctrl+F again = the same window
    {
      const ps = api.hub.projSearch, h = api.hub, ctx0 = h.ctx, ae0 = Object.getOwnPropertyDescriptor(fake.window, 'activeTextEditor');
      const gsv = {};
      h.ctx = Object.assign({}, ctx0, { globalState: { get: k => gsv[k], update: (k, v) => { gsv[k] = v; return Promise.resolve(); } } });
      const port0 = ps.areas().find(a => a.area === 'port');
      const cppF = port0 ? path.join(port0.root, 'forms', 'fHotPlate.cpp') : '';
      Object.defineProperty(fake.window, 'activeTextEditor', { configurable: true, get: () => ({ viewColumn: 2, document: { uri: fake.Uri.file(cppF), getText: () => 'spbSave', getWordRangeAtPosition: () => ({}) }, selection: { isEmpty: true, active: {} } }) });
      const ex0 = fake.commands.executeCommand, cmds = [];
      fake.commands.executeCommand = (id, ...a) => { cmds.push(id); return ex0(id, ...a); };
      const n0 = (fake._panels || []).length;
      const w1 = await fake._cmds['ht9045Designer.find']();
      const pn = (fake._panels || [])[n0];
      const floated = cmds.includes('workbench.action.moveEditorToNewWindow') && cmds.indexOf('closeFindWidget') >= 0;
      const html = pn ? pn.webview.html : '';
      await pn.recv({ type: 'ready' });
      const st0 = pn.posted.find(m => m.type === 'state');
      const sc0 = st0 && st0.s.scopes.map(s => s.label).join(',');
      const r1 = await pn.recv({ type: 'find', q: 'spbSaveClick', scope: 'file|', opts: { wholeWord: true } });
      const last1 = pn.posted[pn.posted.length - 1].s;
      // (1003 filters: only the uses checked; each hit says which; kept in the options it posts back)
      // (spbSaveClick is only in a comment there: the filter leaves it out -- strncpy( is a call)
      const rK0 = await pn.recv({ type: 'find', q: 'spbSaveClick', scope: 'file|', opts: { kinds: { func: true } } });
      const rK = await pn.recv({ type: 'find', q: 'strncpy', scope: 'file|', opts: { kinds: { func: true } } });
      const kOk = !!(rK0 && rK0.n === 0 && rK && rK.n >= 1 && rK.hits.every(h => h.kinds && h.kinds.includes('func')) && pn.posted[pn.posted.length - 1].s.opts.kinds.func === true);
      ps.opts.kinds = null;
      await pn.recv({ type: 'find', q: 'cHotPlate', scope: 'solution|', opts: {} });
      await pn.recv({ type: 'find', q: 'spbSaveClick', scope: 'file|', opts: {} });   // (again: moves to the top, not twice)
      const hist = (gsv['htd.findHistory'] || []).map(x => x.q + '@' + x.scope).join(' ');
      // (the window's whole state after the searches: for panels_render.ps1 to draw it in Edge)
      try { const lastS = pn.posted[pn.posted.length - 1].s; const dir = path.join(require('os').tmpdir(), 'htdesigner_panels'); fs.mkdirSync(dir, { recursive: true }); fs.writeFileSync(path.join(dir, 'findwin.json'), JSON.stringify(Object.assign({}, st0.s, lastS, { q: 'spbSaveClick' }))); } catch (e) { /* the render test says */ }
      const op0 = fake._state ? fake._state.opened.length : 0;
      const opened0 = (fake.window._opened || null);
      await pn.recv({ type: 'open', i: 0 });
      const lastOpen = fake._state && fake._state.opened[fake._state.opened.length - 1];
      await pn.recv({ type: 'delHistory', i: 0 });
      const histDel = (gsv['htd.findHistory'] || []).map(x => x.q).join(' ');
      const w2 = await fake._cmds['ht9045Designer.find']();
      const one = (fake._panels || []).length === n0 + 1 && w2 && w2.reused;
      // 1005 (EastSun: "我搜尋視窗 搜尋時 搜尋範圍都會重製 你可以記憶住嗎? 我其他選項也要記憶住"): a change of 搜尋範圍 / the options /
      // the filters / 取代為 is kept at once (globalState); a search sends back the scope it searched; the window opened again
      // -- and the extension started again -- come back with all of it
      {
        await pn.recv({ type: 'prefs', scope: 'solution|', opts: { regex: true, kinds: { cond: true } }, rq: 'NEWNAME' });
        const pf = gsv['htd.findPrefs'] || {};
        const pfOk = pf.scope === 'solution|' && pf.opts && pf.opts.regex === true && pf.opts.caseSensitive === false && pf.opts.kinds.cond === true && pf.opts.kinds.func === false && pf.rq === 'NEWNAME';
        await pn.recv({ type: 'find', q: 'cHotPlate', scope: 'solution|', opts: { regex: true, kinds: { cond: true } } });
        const back = pn.posted[pn.posted.length - 1].s;
        const backOk = back.scope === 'solution|' && back.opts.regex === true;
        const nP = pn.posted.length;
        await fake._cmds['ht9045Designer.find']();
        const again = (pn.posted.slice(nP).find(m => m.type === 'state') || {}).s || {};
        const againOk = again.scope === 'solution|' && again.opts && again.opts.regex === true && again.opts.kinds.cond === true && again.rq === 'NEWNAME';
        const ps2 = new ps.constructor(h);
        const restartOk = ps2.findLast === 'solution|' && ps2.opts.regex === true && ps2.opts.kinds.cond === true && ps2.fwRq === 'NEWNAME';
        ok(pfOk && backOk && againOk && restartOk, '尋找 window remembers 搜尋範圍, the options, the filters and 取代為: kept when changed (no search needed), the same after a search, when it opens again and after a restart',
          JSON.stringify({ pfOk, backOk, againOk, restartOk, pf, back: { scope: back.scope }, again: { scope: again.scope, rq: again.rq } }));
      }
      fake.commands.executeCommand = ex0;
      if (ae0) Object.defineProperty(fake.window, 'activeTextEditor', ae0); else delete fake.window.activeTextEditor;
      ps.fw = null; ps.opts = { caseSensitive: false, wholeWord: false, regex: false };
      h.ctx = ctx0;
      const pkgW = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const cfgW = pkgW.contributes.configuration.properties;
      const got = { win: w1 && w1.window, floated, view: pn && pn.viewType, csp: /script-src 'nonce-/.test(html) && /findwin\.js/.test(html) && /findwin\.css/.test(html),
        st0: st0 && { q: st0.s.q, scope: st0.s.scope, focus: st0.focus }, sc0, r1: r1 && { n: r1.n, file: r1.hits[0] && path.basename(r1.hits[0].file), line: r1.hits[0] && r1.hits[0].line },
        last1: last1 && { busy: last1.busy, hist: last1.history.length, ww: last1.opts.wholeWord }, hist, kOk, open: lastOpen && { f: path.basename(lastOpen.uri.fsPath), col: lastOpen.viewColumn }, histDel, one,
        cfg: cfgW['ht9045Designer.find.window'] && cfgW['ht9045Designer.find.window'].default === true && cfgW['ht9045Designer.find.newWindow'].default === true };
      ok(!!(got.win && floated && got.view === 'ht9045Designer.findWindow' && got.csp && got.st0 && got.st0.q === 'spbSave' && got.st0.scope === 'file|' && got.st0.focus &&
        /^這個檔案|^目前檔案/.test(sc0) && !/即時標示/.test(sc0) && got.r1 && got.r1.n >= 1 && got.r1.file === 'fHotPlate.cpp' && got.last1.busy === false && got.last1.ww === true &&
        hist === 'spbSaveClick@file| cHotPlate@solution| strncpy@file|' && kOk && got.open && got.open.f === 'fHotPlate.cpp' && got.open.col === 2 && histDel === 'cHotPlate strncpy' && one && got.cfg),
        '尋找 window: its own floated webview (not the built-in box; VS Code\'s find closed), the word at the cursor, the scopes (no VS Code live find); a search -> the results in it and the history (kept, newest first, one per keyword + scope); a result opens in the group the user came from; a row deleted; Ctrl+F again = the same window',
        JSON.stringify(got));
    }
    // 1003 (EastSun: "你用事件轉跳到程式碼時 我希望你用不同底色 標出轉跳到的程式碼"): a jump marks the code it went to in its own
    // background colour -- the whole function (header to closing brace), else the line; the next jump moves the mark;
    // a tab shown again gets it back
    {
      const jd = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-jump-'));
      const jf = path.join(jd, 'a.cpp');
      fs.writeFileSync(jf, 'void __fastcall TfA::B(TObject *Sender)\n{\n  x();\n  if (y) { z(); }\n}\n  call();\nint k;\n');
      const cd0 = fake.window.createTextEditorDecorationType, vis0 = fake._state.visibleTextEditors;
      const types = [], sets = [];
      fake.window.createTextEditorDecorationType = o => { const dt = { o, dispose() {} }; types.push(dt); return dt; };
      const ed = { document: { uri: fake.Uri.file(jf), lineCount: 8 }, setDecorations: (dt, rs) => sets.push(rs.map(r => r.start.line + '-' + r.end.line).join(',')) };
      fake._state.visibleTextEditors = [ed];
      await api.hub.openTarget({ kind: 'port', file: jf, line: 1, col: 1 }, null, true);
      const j1 = sets[sets.length - 1];
      await api.hub.openTarget({ kind: 'port', file: jf, line: 6, col: 3 }, null);
      const j2 = sets[sets.length - 1];
      api.hub.applyJump();
      const j3 = sets[sets.length - 1];
      const other = { document: { uri: fake.Uri.file(path.join(jd, 'b.cpp')), lineCount: 3 }, setDecorations: (dt, rs) => sets.push('other:' + rs.length) };
      fake._state.visibleTextEditors = [ed, other];
      api.hub.applyJump();
      const j4 = sets.slice(-2).join(' ');
      const colour = types[0] && types[0].o.backgroundColor && types[0].o.backgroundColor.id;
      const pkgJ = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const cdef = (pkgJ.contributes.colors || []).find(c => c.id === 'ht9045Designer.jumpTargetBackground');
      fake.window.createTextEditorDecorationType = cd0; fake._state.visibleTextEditors = vis0;
      fs.rmSync(jd, { recursive: true, force: true });
      const got = { j1, j2, j3, j4, types: types.length, colour, whole: types[0] && types[0].o.isWholeLine, cdef: !!(cdef && cdef.defaults && cdef.defaults.dark && cdef.defaults.light) };
      ok(j1 === '0-4' && j2 === '5-5' && j3 === '5-5' && j4 === '5-5 other:0' && got.types === 1 && colour === 'ht9045Designer.jumpTargetBackground' && got.whole && got.cdef,
        'a jump marks the code in its own colour (a theme colour, dark / light defaults): the whole function (0-4), a statement = its line; the next jump moves it (one decoration type); shown again = back; other editors none',
        JSON.stringify(got));
    }    // 1003 (EastSun: "可以轉跳我上次程式碼位置的按鈕" / "可以轉跳到下個程式碼的按鈕", then "按鈕可以不要加圖片位置? 加在上方可以嗎? ... 列一條
    // 工具列 把工具列都列上去 下面藍色顯示資訊就好"): one toolbar on the editor's title bar -- back / forward, ▶, continue, pause,
    // stop, restart, the steps, 模擬 / Debug (an icon each for on / off) -- greyed by "enablement", never hidden; the status
    // bar has none of them
    {
      const ex0 = fake.commands.executeCommand, ran = [];
      fake.commands.executeCommand = (id, ...a) => { ran.push(id); return ex0(id, ...a); };
      for (const c of ['saveFile', 'saveAllFiles', 'navBack', 'navForward', 'run.continue', 'run.pause', 'run.stepOver', 'run.stepInto', 'run.stepOut']) await fake._cmds['ht9045Designer.' + c]();
      fake.commands.executeCommand = ex0;
      // (the on / off icons both switch their setting: the fake's settings cannot be written -- the switch itself is seen)
      const rbT = api.hub.runBar, tg0 = rbT.toggle, tgs = [];
      rbT.toggle = k => { tgs.push(k); return Promise.resolve(); };
      for (const c of ['simOn', 'simOff', 'dbgOn', 'dbgOff']) await fake._cmds['ht9045Designer.run.' + c]();
      rbT.toggle = tg0;
      const toggles = tgs.join(',');
      const pkgN = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const tm = pkgN.contributes.menus['editor/title'] || [];
      const want = ['saveFile', 'saveAllFiles', 'navBack', 'navForward', 'run.buildAndStart', 'run.continue', 'run.pause', 'run.stopAll', 'run.restart', 'run.stepOver', 'run.stepInto', 'run.stepOut', 'run.simOn', 'run.simOff', 'run.dbgOn', 'run.dbgOff'];
      const bar = tm.filter(m => want.includes(m.command.replace('ht9045Designer.', ''))).map(m => m.command.replace('ht9045Designer.', ''));
      const cmdOf = id => pkgN.contributes.commands.find(c => c.command === 'ht9045Designer.' + id) || {};
      const allIcon = want.every(id => /^\$\(/.test(cmdOf(id).icon || ''));
      const grey = ['saveFile', 'saveAllFiles', 'navBack', 'navForward', 'run.buildAndStart', 'run.continue', 'run.pause', 'run.stopAll', 'run.restart', 'run.stepOver'].every(id => !!cmdOf(id).enablement);
      const notHidden = tm.filter(m => /saveFile|saveAllFiles|navBack|navForward|run\.(buildAndStart|continue|pause|stopAll|restart|step)/.test(m.command)).every(m => !m.when);
      const got = { bar: bar.join(','), ran: ran.filter(x => /navigate|debug\.|files\.save/.test(x)).map(x => x.replace(/^workbench\.action\.(debug\.|files\.)?/, '')).join(','), toggles, allIcon, grey, notHidden,
        status: !api.hub.navItems };
      ok(got.bar === want.join(',') && got.ran === 'save,saveAll,navigateBack,navigateForward,continue,pause,stepOver,stepInto,stepOut' && toggles === 'run.simulation,run.simulation,run.debug,run.debug' &&
        allIcon && grey && notHidden && got.status,
        'one toolbar on the editor title bar: back / forward, build+start, continue, pause, stop, restart, steps, 模擬 / Debug (on / off icons) -- greyed when they cannot act, never hidden; none in the status bar',
        JSON.stringify(got));
    }    // 1003 (EastSun: "檔案和按鈕請分成兩行" + "如果是claude 的視窗 請分成第三行"): a Claude tab that opens in front is pinned (VS Code's
    // pinned-tabs row = Claude's row); a file tab, a pinned one, one not in front are left alone; a setting turns it off
    {
      const ex0 = fake.commands.executeCommand, ran = [];
      fake.commands.executeCommand = (id, ...a) => { ran.push(id); return ex0(id, ...a); };
      const g = { isActive: true }, g2 = { isActive: false };
      const cl = (o) => Object.assign({ input: { viewType: 'mainThreadWebview-claudeVSCodePanel' }, isPinned: false, isActive: true, group: g }, o);
      const n1 = api.hub.pinClaudeTabs({ opened: [cl({}), cl({ isPinned: true }), cl({ isActive: false }), cl({ group: g2 }), { input: { uri: fake.Uri.file('a.cpp') }, isPinned: false, isActive: true, group: g }] });
      fake._config.pinClaudeTabs = false;
      const n2 = api.hub.pinClaudeTabs({ opened: [cl({})] });
      delete fake._config.pinClaudeTabs;
      fake.commands.executeCommand = ex0;
      const pkgC = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const got = { n1, n2, pins: ran.filter(x => x === 'workbench.action.pinEditor').length, act: (pkgC.activationEvents || []).includes('onWebviewPanel:claudeVSCodePanel'),
        cfg: pkgC.contributes.configuration.properties['ht9045Designer.pinClaudeTabs'].default === true };
      ok(n1 === 1 && n2 === 0 && got.pins === 1 && got.act && got.cfg, 'Claude tabs on a row of their own: one opening in front is pinned (only that one: not a file, a pinned one, one behind); the setting turns it off; it starts with Claude\'s restored tabs', JSON.stringify(got));
    }    // 1003 (EastSun: "我明明把程式關掉了 圖片上停止鈕 卻又還是可以點"): only this tree's wb_* light ⏹ (another folder's do not);
    // a state left behind is worked out again on the next 3 s tick; the tooltip says why it is lit
    {
      const rb = api.hub.runBar, dbg0 = fake.debug;
      clearInterval(rb.procTimer);
      const tree = rb.plan().tree;
      rb.procPoller = cb => cb(true);
      rb.procLister = async () => [{ pid: 5, path: path.join(require('os').tmpdir(), 'elsewhere', 'wb_serve.exe') }];
      rb.procsAlive = false; rb.procsNames = '';
      // (a real 3 s poll may still be on its way: not this test's)
      const poll = () => { rb.procPolling = false; rb.pollProcs(); };
      fake.debug = {}; rb.update();
      poll(); await new Promise(r => setTimeout(r, 30));
      const otherLit = !!rb.items.stop.command;
      rb.procLister = async () => [{ pid: 6, path: path.join(tree, 'build_integ_ship_x86', 'wb_serve.exe') }];
      poll(); await new Promise(r => setTimeout(r, 30));
      const mineLit = rb.items.stop.command === 'ht9045Designer.run.stopAll', tipProc = /現在亮著是因為：還在跑：wb_serve\.exe/.test(rb.items.stop.tooltip || '');
      rb.procPoller = cb => cb(false);
      poll(); await new Promise(r => setTimeout(r, 30));
      const goneGrey = !rb.items.stop.command;
      // a session that ended without its event reaching update(): the next tick puts it right
      fake.debug = { activeDebugSession: { name: 'IOWEB(這台): wb_serve', configuration: {} } }; rb.update();
      const tipSes = /偵錯工作階段：IOWEB\(這台\): wb_serve/.test(rb.items.stop.tooltip || '');
      fake.debug = {};   // (ended -- no event)
      const before = !!rb.items.stop.command;
      poll(); await new Promise(r => setTimeout(r, 30));
      const healed = !rb.items.stop.command;
      delete rb.procPoller; delete rb.procLister; rb.procsAlive = false; rb.procsNames = '';
      fake.debug = dbg0; rb.update();
      const got = { otherLit, mineLit, tipProc, goneGrey, tipSes, before, healed };
      ok(!otherLit && mineLit && tipProc && goneGrey && tipSes && before && healed,
        'stop: another folder\'s wb_serve does not light it, this tree\'s does (the tooltip says which); gone = grey; a session ended without its event = grey on the next tick; the tooltip names the session',
        JSON.stringify(got));
    }    // 1003 (EastSun: "我要在編譯過程中可以按下圖片中停止 來停止編譯"): during F5's build ⏹ is lit and the title bar says building;
    // ⏹ ends that task and its process tree by its pid, deletes the exes this build wrote (a half-linked one would pass
    // for up to date), leaves an older exe alone, and the build window then says stopped. + "開軟體就自動啟用好 ... 沒按
    // ctrl+F 請先隱藏": starts with VS Code; a 尋找 / 建置 window brought back from the last session is closed
    {
      const rb = api.hub.runBar, hub = api.hub;
      clearInterval(rb.procTimer); rb.procsAlive = false;
      const dir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-f5stop-'));
      const t0 = Date.now();
      fs.writeFileSync(path.join(dir, 'wb_serve.exe'), 'half');
      fs.writeFileSync(path.join(dir, 'wb_publish.exe'), 'old');
      fs.utimesSync(path.join(dir, 'wb_publish.exe'), new Date(t0 - 3600000), new Date(t0 - 3600000));
      const tasks0 = fake.tasks;
      let terminated = 0;
      fake.tasks = Object.assign({}, tasks0 || {}, { taskExecutions: [{ task: { name: 'IOWEB(這台): build wb_serve' }, terminate() { terminated++; } }, { task: { name: 'other' }, terminate() { terminated += 100; } }] });
      const killed = [];
      rb.spawn = (cmd, args) => { if (cmd === 'taskkill') killed.push(args.join(' ')); return { on() {} }; };
      rb.stopWaitMs = 0;
      // (the hub's 1 s build tick may clear hub.bwTask meanwhile -- the folder is not under the tree: the object is kept here)
      const bt = { name: 'IOWEB(這台): build wb_serve', t: t0, ended: false, code: null, dir, pid: 4321 };
      const tick0 = hub.buildWatchTick; hub.buildWatchTick = () => null;
      hub.bwTask = bt;
      rb.update();
      const lit = rb.items.stop.command === 'ht9045Designer.run.stopAll', stBuilding = rb.curState() === 'building', startOff = !rb.items.start.command;
      const sa = await rb.stopAll();
      const halfGone = !fs.existsSync(path.join(dir, 'wb_serve.exe')), oldKept = fs.existsSync(path.join(dir, 'wb_publish.exe'));
      const cancelled = !!bt.cancelled;
      bt.ended = true; hub.bwTask = bt;
      rb.update();
      const greyAfter = !rb.items.stop.command;
      hub.bwTask = null;
      fake.tasks = tasks0; delete rb.spawn; delete rb.stopWaitMs;
      hub.buildWatchTick = tick0;
      fs.rmSync(dir, { recursive: true, force: true });
      const pkgS = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).activationEvents || [];
      const got = { lit, stBuilding, startOff, did: sa && sa.did.join('|'), killed, terminated, halfGone, oldKept, cancelled, greyAfter,
        act: ['onStartupFinished', 'onWebviewPanel:ht9045Designer.findWindow', 'onWebviewPanel:ht9045Designer.buildWindow'].every(x => pkgS.includes(x)) };
      ok(lit && stBuilding && startOff && /F5 建置（刪掉寫到一半的 wb_serve\.exe/.test(got.did) && killed.includes('/PID 4321 /T /F') && terminated === 1 && halfGone && oldKept && cancelled && greyAfter && got.act,
        'stop during F5\'s build: lit, the title bar says building; it ends that task (not another) and its tree by pid, deletes the exe this build wrote, keeps an older one; grey after; starts with VS Code, restored windows closed',
        JSON.stringify(got));
    }    // 1003 (EastSun: "我需要的是隱藏 有需要搜尋在出現 focus到別的視窗 就隱藏 不要讓使用者看到 也點不出來"): the find window closes
    // when the focus goes elsewhere (not while it is the active one, not in its first moments), Ctrl+F brings it back with
    // the last results; a click on a result keeps the focus in it
    {
      const ps = api.hub.projSearch, cw0 = fake.window.createWebviewPanel;
      ps.fw = null; ps.fwSettleMs = 0;
      const made = [];
      fake.window.createWebviewPanel = (...a) => {
        const p = cw0(...a);
        p.disposed = false;
        p.onDidChangeViewState = fn => { p.vs = fn; return { dispose() {} }; };
        p.onDidDispose = fn => { p.dd = fn; return { dispose() {} }; };
        p.dispose = () => { p.disposed = true; if (p.dd) p.dd(); };
        made.push(p);
        return p;
      };
      await fake._cmds['ht9045Designer.find']();
      const p1 = made[0];
      p1.vs({ webviewPanel: { active: true } });
      const keptActive = !p1.disposed;
      p1.vs({ webviewPanel: { active: false } });
      const hid = p1.disposed && ps.fw === null;
      ps.fwLastRes = { q: 'spbSave', n: 1, files: 1, ms: 1, shown: 1, hits: [] };
      await fake._cmds['ht9045Designer.find']();
      const p2 = made[1];
      if (p2 && p2.recv) await p2.recv({ type: 'ready' });
      const st2 = p2 && p2.posted.filter(m => m.type === 'state')[0];
      const back = !!(st2 && st2.s.res && st2.s.res.q === 'spbSave');
      const open0 = ps.open, args = [];
      ps.open = (...x) => { args.push(x); return Promise.resolve(null); };
      ps.res = { hits: [{ file: 'a.cpp', line: 1, col: 1, len: 1 }] };
      await p2.recv({ type: 'open', i: 0 });
      ps.open = open0;
      const keepFocus = args[0] && args[0][2] === true;
      fake.window.createWebviewPanel = cw0;
      if (p2) p2.dispose();
      ps.fw = null; delete ps.fwSettleMs; ps.fwLastRes = null; ps.res = null;
      const cfg = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.configuration.properties['ht9045Designer.find.hideOnBlur'];
      const got = { keptActive, hid, back, keepFocus, cfg: !!(cfg && cfg.default === true) };
      ok(keptActive && hid && back && keepFocus && got.cfg,
        'find window hides itself: stays while active, closes when the focus goes elsewhere (off the task bar); Ctrl+F brings it back with the last results; a result click keeps the focus in it',
        JSON.stringify(got));
    }    // 1003 (EastSun: "為啥我都已經關掉程式了 我還需要按按鈕停止 才可以編譯"): before F5's build the program about to be rebuilt is
    // ended -- its sessions and its process (that exe path only: another folder's / another exe are left) -- and waited for
    {
      const rb = api.hub.runBar, tree = rb.plan().tree;
      const prog = path.join(tree, 'build_integ_ship_x86', 'wb_serve.exe');
      let alive = [{ pid: 11, path: prog }, { pid: 12, path: path.join(require('os').tmpdir(), 'x', 'wb_serve.exe') }, { pid: 13, path: path.join(tree, 'build_integ_ship_x86', 'wb_publish.exe') }];
      const killed = [];
      rb.procLister = async () => alive.slice();
      rb.spawn = (cmd, args) => { if (cmd === 'taskkill') { killed.push(args[1]); alive = alive.filter(p => String(p.pid) !== args[1]); } return { on() {} }; };
      rb.stopWaitMs = 0;
      const stopped = [];
      const dbg0 = fake.debug, ses0 = rb.sessions;
      const sMine = { configuration: { program: prog } }, sOther = { configuration: { program: path.join(tree, 'other.exe') } };
      rb.sessions = new Set([sMine, sOther]);
      fake.debug = { stopDebugging: async s => { stopped.push(s === sMine ? 'mine' : 'other'); } };
      const pids = await rb.freeProgram(prog);
      const none = await rb.freeProgram(path.join(tree, 'build_integ_ship_x86', 'nothing.exe'));
      fake.debug = dbg0; rb.sessions = ses0;
      delete rb.procLister; delete rb.spawn; delete rb.stopWaitMs;
      const cfg = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.configuration.properties['ht9045Designer.run.freeBeforeBuild'];
      const got = { pids: pids.join(','), killed: killed.join(','), stopped: stopped.join(','), none: none.length, cfg: !!(cfg && cfg.default === true) };
      ok(got.pids === '11' && got.killed === '11' && got.stopped === 'mine' && got.none === 0 && got.cfg,
        'F5 frees the program before its build: its session stopped and its process ended (that exe only -- not another folder\'s, not wb_publish), waited for; nothing running = nothing done',
        JSON.stringify(got));
    }    // 1003 (EastSun: "我搜尋視窗 要有可以替換關鍵字的功能"): replace one hit / all hits in the documents (not saved); a hit whose
    // place no longer has the match is skipped; the golden tree never; the hits after a replaced one in its file move with it;
    // regex groups ($1); + "永遠0%": F5's % reaches 方案總管's toolbar
    {
      const ps = api.hub.projSearch;
      const repDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-rep-'));
      const rf = path.join(repDir, 'a.cpp');
      fs.writeFileSync(rf, 'aa FOO bb FOO\ncc FOO\n');
      const keep = { q: ps.q, opts: ps.opts, res: ps.res };
      ps.q = 'FOO'; ps.opts = { caseSensitive: true, wholeWord: false, regex: false };
      const H = at => ({ area: 'port', file: rf, rel: 'a.cpp', enc: 'utf8', at, len: 3, line: 1, col: 1, text: '' });
      ps.res = { hits: [H(3), H(10), H(17), Object.assign(H(0), { area: 'golden', file: path.join(repDir, 'g.cpp') })] };
      const r1 = await ps.replaceHits([ps.res.hits[0]], 'LONGER');
      const doc = await fake.workspace.openTextDocument(fake.Uri.file(rf));
      const t1 = doc.getText(), left1 = ps.res.hits.map(h => h.at).join(',');
      const r2 = await ps.replaceHits(ps.res.hits, 'Z');
      const t2 = doc.getText();
      // regex groups; the second hit's place no longer has the match
      const bf = path.join(repDir, 'b.cpp');
      fs.writeFileSync(bf, 'x = Val12;\n');
      ps.q = 'Val(\\d+)'; ps.opts = { caseSensitive: true, wholeWord: false, regex: true };
      ps.res = { hits: [{ area: 'port', file: bf, rel: 'b.cpp', enc: 'utf8', at: 4, len: 5, line: 1, col: 5, text: '' }, { area: 'port', file: bf, rel: 'b.cpp', enc: 'utf8', at: 0, len: 5, line: 1, col: 1, text: '' }] };
      const r3 = await ps.replaceHits(ps.res.hits, 'Num$1');
      const t3 = (await fake.workspace.openTextDocument(fake.Uri.file(bf))).getText();
      const onDisk = fs.readFileSync(rf, 'utf8');
      Object.assign(ps, keep);
      fs.rmSync(repDir, { recursive: true, force: true });
      // F5's % on 方案總管's toolbar
      const rb = api.hub.runBar, hub = api.hub;
      hub.bwTask = { name: 'b', t: Date.now(), ended: false }; hub.buildBar = { pct: 42 };
      const snapPct = rb.snapshot().pct, snapSt = rb.snapshot().st;   // (1003: 方案總管 said 啟動 while F5 built -- it reads snapshot())
      hub.bwTask = null; hub.buildBar = null;
      const got = { r1, t1, left1, r2, t2, r3, t3, unsaved: onDisk === 'aa FOO bb FOO\ncc FOO\n', snapPct, snapSt };
      ok(r1.done === 1 && t1 === 'aa LONGER bb FOO\ncc FOO\n' && left1 === '13,20,0' && r2.done === 2 && r2.golden === 1 && t2 === 'aa LONGER bb Z\ncc Z\n' &&
        r3.done === 1 && r3.skipped === 1 && t3 === 'x = Num12;\n' && got.unsaved && snapPct === 42 && snapSt === 'building',
        'replace: one hit, then all (the others moved by the length difference, the golden one never), regex groups ($1), a hit no longer at its place skipped; documents only (nothing saved); F5\'s % on 方案總管\'s toolbar',
        JSON.stringify(got));
    }
    // 1003 (EastSun: "你要寫LOG紀錄 才能查異常" + "並且定時清理LOG"): the hub writes its log lines, events and errors (with the stack)
    // to today's file; the settings and the open-folder command are there
    {
      const hub = api.hub;
      const lf = hub.flog ? hub.flog.file(new Date()) : null;
      const before = lf && fs.existsSync(lf) ? fs.readFileSync(lf, 'utf8').length : 0;
      hub.log('smoke-log-line');
      hub.logEv('smoke-event');
      hub.logErr('smoke.where', new Error('smoke-boom'));
      const added = lf && fs.existsSync(lf) ? fs.readFileSync(lf, 'utf8').slice(before) : '';
      const pkgL = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8'));
      const pr = pkgL.contributes.configuration.properties;
      const got = { file: lf && path.basename(lf), info: /\[INFO\] smoke-log-line/.test(added), ev: /\[EVENT\] smoke-event/.test(added),
        err: /\[ERROR\] smoke\.where：smoke-boom\n {4}Error: smoke-boom/.test(added), cmd: typeof fake._cmds['ht9045Designer.openLogFolder'] === 'function',
        cfg: pr['ht9045Designer.log.keepDays'].default === 14 && pr['ht9045Designer.log.maxMB'].default === 50 };
      ok(got.info && got.ev && got.err && got.cmd && got.cfg && /^htdesigner_\d{8}\.log$/.test(got.file || ''),
        'log file: the hub\'s lines, events and errors (with the stack) in today\'s file; keepDays / maxMB settings; 開啟 LOG 資料夾',
        JSON.stringify(got));
    }
    // 1005 (EastSun: "如果已經編譯過了 請按下F5的時候 就直接啟動軟體"): F5 with the program up to date -> its compound preLaunchTask
    // becomes its last part that is not a build (the wait page); with a newer source the build stays; a plain build task -> none
    {
      const hub = api.hub, rb = hub.runBar, plan0 = rb.plan;
      const ud = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-f5skip-'));
      const T = s => new Date(Date.now() - s * 1000);
      const put = (rel, body, ageS) => { const p = path.join(ud, rel); fs.mkdirSync(path.dirname(p), { recursive: true }); fs.writeFileSync(p, body); fs.utimesSync(p, T(ageS), T(ageS)); return p; };
      put('a.cpp', 'x', 600);
      const exe = put('build_s/wb_serve.exe', Buffer.alloc(80 * 1024), 100);
      put('.vscode/tasks.json', JSON.stringify({ version: '2.0.0', tasks: [
        { label: 'check old', type: 'process', command: 'powershell', args: ['check_wb_serve_not_running.ps1'] },
        { label: 'wait page', type: 'process', command: 'powershell', args: ['open_boot_wait.ps1'] },
        { label: 'build wb_serve', type: 'process', command: 'cmake', args: ['--build', 'build_s'] },
        { label: 'check + wait + build', dependsOrder: 'sequence', dependsOn: ['check old', 'wait page', 'build wb_serve'] },
      ] }), 1000);
      rb.plan = () => Object.assign(plan0.call(rb), { tree: ud });
      const folder = { uri: fake.Uri.file(ud) };
      const c1 = { program: exe, preLaunchTask: 'check + wait + build' };
      const s1 = hub.skipBuildIfUpToDate(folder, c1);
      put('a.cpp', 'y', 10);
      const c2 = { program: exe, preLaunchTask: 'check + wait + build' };
      const s2 = hub.skipBuildIfUpToDate(folder, c2);
      put('a.cpp', 'y', 600);
      const c3 = { program: exe, preLaunchTask: 'build wb_serve' };
      const s3 = hub.skipBuildIfUpToDate(folder, c3);
      fake._config['run.skipBuildWhenUpToDate'] = false;
      const c4 = { program: exe, preLaunchTask: 'check + wait + build' };
      const s4 = hub.skipBuildIfUpToDate(folder, c4);
      delete fake._config['run.skipBuildWhenUpToDate'];
      rb.plan = plan0;
      fs.rmSync(ud, { recursive: true, force: true });
      const got = { s1: s1.skipped, t1: c1.preLaunchTask, s2: s2.skipped, t2: c2.preLaunchTask, s3: s3.skipped, t3: 'preLaunchTask' in c3, s4: s4.skipped, t4: c4.preLaunchTask };
      ok(got.s1 && got.t1 === 'wait page' && !got.s2 && got.t2 === 'check + wait + build' && got.s3 && got.t3 === false && !got.s4 && got.t4 === 'check + wait + build',
        'F5 skips the build when the program is newer than every source: a compound task keeps its last non-build part (the wait page), a plain build task none; a newer source = the build stays; the setting turns it off',
        JSON.stringify(got));
    }
    // 1005 (EastSun: "claude 在上 C++ 檔案顯示在下 你有全部加進外掛工具嗎? 怎別台電腦沒有"): the extension sets the layout settings
    // itself, once, at the user level -- only the ones the user has not set; never again after; off by a setting
    {
      const h = api.hub, ctx0 = h.ctx, gc0 = fake.workspace.getConfiguration;
      const run = async (userSet, offSetting) => {
        const gsv = {}, writes = [];
        h.ctx = Object.assign({}, ctx0, { globalState: { get: k => gsv[k], update: (k, v) => { gsv[k] = v; return Promise.resolve(); } } });
        fake.workspace.getConfiguration = sec => (sec === 'workbench.editor' || sec === 'debug') ? {
          get: k => undefined, inspect: k => ({ globalValue: userSet[sec + '.' + k] }), update: async (k, v, where) => { writes.push(sec + '.' + k + '=' + v + '@' + where); },
        } : gc0(sec);
        if (offSetting) fake._config['layout.apply'] = false;
        const first = h.applyLayout(), again = h.applyLayout();
        delete fake._config['layout.apply'];
        return { first, again, writes };
      };
      const a = await run({ 'debug.toolBarLocation': 'floating' }, false);
      const b = await run({}, true);
      fake.workspace.getConfiguration = gc0; h.ctx = ctx0;
      const G = (fake.ConfigurationTarget || {}).Global;
      const got = { first: a.first.join(','), again: a.again.length, writes: a.writes.join(' '), off: b.first.length + b.writes.length };
      ok(got.first === 'workbench.editor.editorActionsLocation,workbench.editor.pinnedTabsOnSeparateRow' && got.again === 0 &&
        got.writes === 'workbench.editor.editorActionsLocation=titleBar@' + G + ' workbench.editor.pinnedTabsOnSeparateRow=true@' + G && got.off === 0,
        'layout on any PC: the buttons on the title bar and pinned (Claude) tabs on their own row set at the user level, once; a setting the user made himself (the debug toolbar here) left alone; layout.apply=false = nothing',
        JSON.stringify(got));
    }
    // 0.142 執行列 (EastSun: "我需要在固定的地方 可以讓我啟動 暫停 停止 該軟體"): the commands per state.
    // 0.149 (EastSun: "勾選開關 ... 模擬模式 (SOFT_SIMULTE) ... debug 模式 ... 綠色箭頭 啟動軟體並編譯 ... 暫停的按鈕給我中斷",
    // "編譯的時候 需要有視窗顯示在編譯 ... 按鈕取消編譯", "我編譯視窗要有進度條"): ☑ 模擬 / ☑ Debug, the green ▶ = build + start
    {
      const rb = api.hub.runBar, rbCmds = () => Object.keys(rb.items).filter(k => k !== 'sim' && k !== 'dbg').map(k => {
        const c = rb.items[k].command; return k + '=' + (c ? (typeof c === 'string' ? c : c.command) : '-').replace('workbench.action.debug.', '').replace('ht9045Designer.run.', 'run.');
      }).join(' ');
      const dbg0 = fake.debug;
      // (not the machine's real process list: a wb_serve running here would light ⏹ in the idle check)
      clearInterval(rb.procTimer); rb.procsAlive = false;
      // (this PC's real gdb is not asked here: the gdb check has its own test below)
      rb.gdbCheck = async () => ({ ok: true });
      const exC = fake.commands.executeCommand, ctxK = {};
      fake.commands.executeCommand = (id, ...a) => { if (id === 'setContext') ctxK[a[0]] = a[1]; return exC(id, ...a); };
      fake.debug = {}; const rbIdle = [rb.update(), rbCmds()];
      const ctxIdle = ctxK['ht9045Designer.runState'] + '/' + ctxK['ht9045Designer.runCanStop'];
      // a wb_* exe running with no session (a terminal, a session that left it): ⏹ is on, idle or not
      rb.procsAlive = true; rb.update(); const stopLeft = rb.items.stop.command === 'ht9045Designer.run.stopAll' && rb.items.restart.command === 'ht9045Designer.run.restart' && ctxK['ht9045Designer.runCanRestart'] === true; rb.procsAlive = false;
      fake.debug = { activeDebugSession: { configuration: {} } }; const rbRun = [rb.update(), rbCmds()];
      const ctxRun = ctxK['ht9045Designer.runState'] + '/' + ctxK['ht9045Designer.runCanStop'] + '/' + ctxK['ht9045Designer.runNoDbg'];
      const pauseNoDbg = (() => { fake.debug = { activeDebugSession: { configuration: { noDebug: true } } }; rb.update(); return rb.items.pause.command; })();
      fake.debug = { activeDebugSession: { configuration: {} }, activeStackItem: { frameId: 1 } }; const rbPause = [rb.update(), rbCmds()];
      const ctxPause = ctxK['ht9045Designer.runState'];
      fake.commands.executeCommand = exC;
      fake.debug = {}; rb.update();
      // (1003: the run buttons left the status bar -- the title bar has them; the items only hold the state)
      const rbShown = Object.values(rb.items).every(it => !it.shown);
      const green = rb.items.start.color && rb.items.start.color.id === 'testing.iconPassed';
      // the four choices -> the four build dirs (the tree's own F5 dirs); the checkboxes' text
      const c0 = { sim: fake._config['run.simulation'], dbg: fake._config['run.debug'] };
      const plans = [];
      for (const [s, d] of [[true, true], [true, false], [false, true], [false, false]]) {
        fake._config['run.simulation'] = s; fake._config['run.debug'] = d; rb.update();
        plans.push(rb.plan().dir + ':' + rb.items.sim.text.split(' ')[0].replace(/\$\(|\)/g, '') + '/' + rb.items.dbg.text.split(' ')[0].replace(/\$\(|\)/g, ''));
      }
      // ▶ with a stand-in build: the bar follows make's %, the terminal gets the output, then the start (Debug: with gdb)
      fake._config['run.simulation'] = true; fake._config['run.debug'] = true;
      const keep = { tasks: fake.tasks, Task: fake.Task, CE: fake.CustomExecution, TS: fake.TaskScope, TR: fake.TaskRevealKind, TP: fake.TaskPanelKind, wp: fake.window.withProgress, sd: fake.debug.startDebugging };
      let ptyOut = '', ranArgs = null, started = null;
      const reports = [];
      fake.TaskScope = { Workspace: 2 }; fake.TaskRevealKind = { Always: 1 }; fake.TaskPanelKind = { Dedicated: 2 };
      fake.CustomExecution = class { constructor(cb) { this.cb = cb; } };
      fake.Task = class { constructor(def, scope, name, source, exec, pm) { Object.assign(this, { def, scope, name, source, exec, pm }); } };
      fake.tasks = { executeTask: async t => { const pty = await t.exec.cb(); pty.onDidWrite(d => { ptyOut += d; }); pty.open(); return { task: t }; } };
      fake.window.withProgress = (o, fn) => Promise.resolve(fn({ report: r => reports.push(r) }, { onCancellationRequested() {} }));
      fake.debug = { startDebugging: async (f, cfg, o) => { started = { cfg, o }; return true; } };
      rb.checkRunning = async () => false;   // (no wb_serve.exe running: tasklist stand-in)
      const { EventEmitter } = require('events');
      rb.spawn = (cmd, args) => {
        ranArgs = [cmd].concat(args);
        const ch = new EventEmitter(); ch.stdout = new EventEmitter(); ch.stderr = new EventEmitter(); ch.pid = 4242;
        setTimeout(() => { ch.stdout.emit('data', Buffer.from('[ 10%] Building CXX object a.obj\n[ 55%] Building CXX object b.obj\n')); ch.stdout.emit('data', Buffer.from('[100%] Linking CXX executable wb_serve.exe\n')); ch.emit('close', 0); }, 20);
        return ch;
      };
      const bs = await fake._cmds['ht9045Designer.run.buildAndStart']();
      const incs = reports.filter(r => r.increment).map(r => r.increment);
      const barOk = incs.join(',') === '10,45,45' && reports.some(r => /100%/.test(r.message || ''));
      const termOk = /\[ 55%\] Building CXX object b\.obj\r\n/.test(ptyOut);
      const argsOk = ranArgs && ranArgs[0] === 'powershell.exe' && /htd_build\.ps1$/.test(ranArgs[ranArgs.indexOf('-File') + 1]) && ranArgs[ranArgs.indexOf('-Dir') + 1] === 'build_dbg_nonoracle' &&
        ranArgs[ranArgs.indexOf('-Sim') + 1] === '1' && ranArgs[ranArgs.indexOf('-Dbg') + 1] === '1';
      const startOk = started && /build_dbg_nonoracle[\\/]wb_serve\.exe$/.test(started.cfg.program) && started.cfg.noDebug === false && started.o.noDebug === false && !started.cfg.preLaunchTask && started.cfg.type === 'cppdbg' && started.cfg.environment.some(e => e.name === 'W906_AUTH_PATH' && e.value.indexOf('${workspaceFolder}') < 0 && /runcfg/.test(e.value)) && /出貨組態（1203 唯讀）/.test(bs && bs.from || '');
      // a failed build does not start; Debug off = no debugger
      fake._config['run.debug'] = false; started = null; reports.length = 0;
      rb.spawn = () => { const ch = new EventEmitter(); ch.stdout = new EventEmitter(); ch.stderr = new EventEmitter(); setTimeout(() => ch.emit('close', 2), 10); return ch; };
      const bf = await fake._cmds['ht9045Designer.run.buildAndStart']();
      const failOk = bf && bf.built === false && bf.code === 2 && started === null;
      // cancel: the whole tree killed (taskkill /T), the half-made exe deleted -- in a throw-away tree (never a real build dir)
      const plan0 = rb.plan;
      const cxTree = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd_cancel_'));
      fs.mkdirSync(path.join(cxTree, 'build_nonoracle'));
      fs.writeFileSync(path.join(cxTree, 'build_nonoracle', 'wb_serve.exe'), 'cut');
      rb.plan = () => Object.assign(plan0.call(rb), { tree: cxTree });
      // (no launch.json there: refused before any build -- wb_serve would start without its W906_* paths)
      let spawned = 0; rb.spawn = () => { spawned++; throw new Error('no'); };
      const noLaunch = await fake._cmds['ht9045Designer.run.buildAndStart']();
      const noLaunchOk = noLaunch && noLaunch.refused === 'launch' && spawned === 0;
      fs.mkdirSync(path.join(cxTree, '.vscode'));
      fs.copyFileSync(path.join(plan0.call(rb).tree, '.vscode', 'launch.json'), path.join(cxTree, '.vscode', 'launch.json'));
      // (a wb_serve.exe still running: refused)
      // (1003: what still runs is stopped first -- one that is still there after that is another folder's: refused)
      rb.checkRunning = async () => true; rb.stopWaitMs = 0; rb.procLister = async () => [];
      const busyRun = await fake._cmds['ht9045Designer.run.buildAndStart']();
      delete rb.procLister; delete rb.stopWaitMs;
      const busyRunOk = busyRun && busyRun.refused === 'running' && spawned === 0;
      rb.checkRunning = async () => false;
      const killed = [];
      let hold = null;
      rb.spawn = (cmd, args) => {
        if (cmd === 'taskkill') { killed.push(args.join(' ')); setTimeout(() => hold && hold.emit('close', 1), 5); return new EventEmitter(); }
        const ch = new EventEmitter(); ch.stdout = new EventEmitter(); ch.stderr = new EventEmitter(); ch.pid = 777; hold = ch; return ch;
      };
      const pr = fake._cmds['ht9045Designer.run.buildAndStart']();
      await new Promise(r => setTimeout(r, 30));
      const busy = rb.items.start.text.indexOf('sync~spin') >= 0 && rb.items.stop.command === 'ht9045Designer.run.stopAll';
      // (⏹ = 全部停止: the build cancelled, then this tree's wb_* exes still alive killed -- not another folder's)
      rb.stopWaitMs = 0;
      rb.procLister = async () => [{ pid: 901, path: path.join(cxTree, 'build_x64', 'wb_publish.exe') }, { pid: 902, path: path.join(require('os').tmpdir(), 'other', 'wb_serve.exe') }];
      const sa = await rb.stopAll();
      delete rb.procLister; delete rb.stopWaitMs;
      const bc = await pr;
      await new Promise(r => setTimeout(r, 1700));
      const cutGone = !fs.existsSync(path.join(cxTree, 'build_nonoracle', 'wb_serve.exe'));
      const cancelOk = noLaunchOk && busyRunOk && busy && killed[0] === '/PID 777 /T /F' && killed[1] === '/PID 901 /T /F' && killed.length === 2 && sa && sa.killed.join() === '901' && bc && bc.built === false && bc.code === undefined && !rb.building && cutGone;
      rb.plan = plan0;
      try { fs.rmSync(cxTree, { recursive: true, force: true }); } catch (e) { /* temp */ }
      // (the two error messages above were asked for: not "an error shown" for the check at the end)
      for (let i = fake._calls.length - 1; i >= 0; i--) if (fake._calls[i].name === 'error' && /^建置(失敗|取消)/.test(String(fake._calls[i].args[0]))) fake._calls.splice(i, 1);
      delete rb.spawn; delete rb.checkRunning;
      for (let i = fake._calls.length - 1; i >= 0; i--) if (fake._calls[i].name === 'error' && /^不啟動：/.test(String(fake._calls[i].args[0]))) fake._calls.splice(i, 1);
      Object.assign(fake, { tasks: keep.tasks, Task: keep.Task, CustomExecution: keep.CE, TaskScope: keep.TS, TaskRevealKind: keep.TR, TaskPanelKind: keep.TP });
      fake.window.withProgress = keep.wp;
      fake._config['run.simulation'] = c0.sim; fake._config['run.debug'] = c0.dbg;
      // VS Code's own ⏹ ends one session of F5's pair: the other of this tree is stopped too, another folder's is not
      const trR = rb.plan().tree, sA = { configuration: { program: path.join(trR, 'build_x64', 'wb_publish.exe') } };
      const sB = { configuration: { program: path.join(trR, 'build_x64', 'wb_gateway.exe') } }, sC = { configuration: { program: path.join(require('os').tmpdir(), 'x', 'wb_serve.exe') } };
      const stoppedS = [], ses0 = rb.sessions;
      rb.sessions = new Set([sA, sB, sC]);
      fake.debug = { stopDebugging: async s => { stoppedS.push(s); } };
      const fN = rb.followStop(sA);
      const followOk = fN === 1 && stoppedS.length === 1 && stoppedS[0] === sB && rb.sessions.has(sC) && rb.followStop(sC) === 0;
      rb.sessions = ses0;
      fake.debug = dbg0; rb.update();
      // 1005 (EastSun: "圖片上按鈕我希望都有作用 現在好像有些是假的"): ↺ = Visual Studio's Restart -- stop everything, build, start
      // (VS Code's debug.restart did not rebuild); a wb_* exe left running lights it too; during a build it does nothing
      const sa0 = rb.stopAll, bs0 = rb.buildAndStart, rsSeq = [];
      rb.stopAll = async () => { rsSeq.push('stop'); return { did: [] }; };
      rb.buildAndStart = async () => { rsSeq.push('build+start'); return { built: true, started: true }; };
      rb.procsAlive = true; await fake._cmds['ht9045Designer.run.restart']();
      rb.procsAlive = false; rsSeq.push('|'); await fake._cmds['ht9045Designer.run.restart']();
      rb.building = { run: {}, p: {} }; rsSeq.push('|'); await fake._cmds['ht9045Designer.run.restart'](); rb.building = null;
      rb.stopAll = sa0; rb.buildAndStart = bs0; rb.update();
      const restartOk = rsSeq.join(',') === 'stop,build+start,|,build+start,|';
      // 1005 (ES02: this PC's gdb dies on any program it starts -- Debug's ▶ said "started" and nothing ran): with Debug,
      // gdb is asked first; it cannot -> said why, nothing built; "改用 Release 啟動" -> Release, then ▶ again
      {
        const realBs = rb.buildAndStart, tg1 = rb.toggle, a0 = fake._answer, tgl = [];
        let bsN = 0;
        rb.buildAndStart = function (...x) { bsN++; return bsN === 1 ? realBs.apply(rb, x) : Promise.resolve({ again: true }); };
        rb.toggle = async k => { tgl.push(k); };
        rb.gdbCheck = async () => ({ ok: false, why: 'exit 0xE0000027' });
        fake._config['run.debug'] = true;
        fake._answer = '取消';
        const g1 = await rb.buildAndStart();
        const warn = fake._calls.filter(c => c.name === 'warning').pop();
        bsN = 0; fake._answer = '改用 Release 啟動';
        const g2 = await rb.buildAndStart();
        rb.buildAndStart = realBs; rb.toggle = tg1; fake._answer = a0; rb.gdbCheck = async () => ({ ok: true });
        fake._config['run.debug'] = c0.dbg;
        for (let i = fake._calls.length - 1; i >= 0; i--) if (fake._calls[i].name === 'warning' && /^Debug 啟動不了/.test(String(fake._calls[i].args[0]))) fake._calls.splice(i, 1);
        ok(g1 && g1.refused === 'gdb' && !rb.building && warn && /gdb/.test(warn.args[0]) && /0xE0000027/.test(warn.args[0]) && g2 && g2.again && tgl.join() === 'run.debug',
          'Debug ▶ when gdb cannot start a program here: said why (nothing built, not "started"); 改用 Release 啟動 = Release, then ▶ again', JSON.stringify({ g1, g2, tgl, warn: warn && warn.args[0].slice(0, 50) }));
      }
      ok(restartOk, '↺ restart (like Visual Studio): stop all -> rebuild -> start; lit while this tree\'s wb_* runs; nothing during a build', rsSeq.join(','));
      // 1005 (WPF / Blend audit): Ctrl+D = duplicate the selection (10, 10 down-right), in the designer only (the CSV table keeps its own Ctrl+D)
      {
        const kb = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.keybindings.filter(k => k.key === 'ctrl+d');
        const dz = kb.find(k => k.command === 'ht9045Designer.duplicate'), cz = kb.find(k => k.command === 'ht9045Designer.csvKey');
        ok(!!(dz && dz.args && dz.args.dx === 10 && dz.args.dy === 10 && /ht9045Designer\.editor/.test(dz.when) && /textEditing/.test(dz.when) && cz && /ht9045Designer\.csv/.test(cz.when)),
          'Ctrl+D duplicates in the designer (like Visual Studio / Blend), not while typing; the CSV table\'s Ctrl+D stays', JSON.stringify(kb.map(k => k.command)));
      }
      const got = { followOk, stopLeft, ctx: ctxIdle + ' ' + ctxRun + ' ' + ctxPause, idle: rbIdle.join(' '), run: rbRun.join(' '), pause: rbPause.join(' '), pauseNoDbg: pauseNoDbg || null, shown: rbShown, green, plans: plans.join(' '),
        barOk, incs, termOk, argsOk, startOk, from: bs && bs.from, failOk, cancelOk, noLaunchOk, busyRunOk, killed };
      ok(got.idle === 'idle start=run.buildAndStart pick=selectandstart pause=- stop=- restart=- over=- into=- out=-' &&
        got.run === 'running start=- pick=- pause=pause stop=run.stopAll restart=run.restart over=- into=- out=-' && !pauseNoDbg &&
        got.pause === 'paused start=continue pick=- pause=- stop=run.stopAll restart=run.restart over=stepOver into=stepInto out=stepOut' && rbShown && green &&
        got.plans === 'build_dbg_nonoracle:pass-filled/bug build_nonoracle:pass-filled/rocket build_integ_dbg_x86:circle-large-outline/bug build_integ_ship_x86:circle-large-outline/rocket' &&
        barOk && termOk && argsOk && startOk && failOk && cancelOk && followOk && got.stopLeft && got.ctx === 'idle/false running/true/false paused',
        'the run bar: ☑ 模擬 / ☑ Debug -> the four build dirs; the green ▶ builds (the bar follows make\'s %, the output in a terminal) then starts (Debug = gdb, no preLaunchTask); a failed build does not start; ⏹ cancels (taskkill /T); ⏸ only with a debugger',
        JSON.stringify(got));
    }
  }
  pageRecv({ __htd: 1, type: 'tree', nodes: R.treeNodes });
  await new Promise(r => setTimeout(r, 50));
  const roots = fake._tree.getChildren();
  const formItem = roots.length ? fake._tree.getTreeItem(roots[0]) : null;
  const kids = roots.length ? fake._tree.getChildren(roots[0]).map(n => fake._tree.getTreeItem(n)) : [];
  ok(formItem && /fHotPlate : TfHotPlate/.test(formItem.label) && kids.length >= 3 && kids.some(k => /^TPanel/.test(k.description)),
    'component tree: form root + children, VCL class from the IR',
    (formItem ? formItem.label : '-') + ' -> ' + kids.map(k => k.label + ' ' + k.description).join(', '));

  // selection recorded in Edge: frames are file:// URLs of the real scripts
  pageRecv({ __htd: 1, type: 'select', info: R.selInfo, origin: 'click' });
  // wait for resolve (index build ~1-2 s alone; more while the other test layers run)
  let full = null;
  const fullT0 = Date.now();
  for (let i = 0; i < 600 && !full; i++) {
    await new Promise(r => setTimeout(r, 50));
    const last = posted.filter(m => m.type === 'show' && m.data).pop();
    if (last && !last.data.cppPending) full = last.data;
  }
  ok(!!full, 'properties view receives the full result', (Date.now() - fullT0) + ' ms');
  if (!full) return;
  const T = i => full.targets[i];
  const where = i => { const t = T(i); return t ? t.kind + ' ' + path.basename(t.file) + ':' + t.line + (t.note ? ' (' + t.note + ')' : '') : '?'; };
  ok(full.comp.name === 'spbSave' && full.comp.cls === 'TSpeedButton' && /Panel2\.spbSave$/.test(full.comp.path),
    'component = spbSave : TSpeedButton, DFM path', full.comp.path);
  ok(full.comp.note === '統一 save 樣式', 'note from the page source title (stripped at runtime)', full.comp.note);
  const click = full.events.find(e => e.name === 'OnClick');
  ok(click && click.handler === 'spbSaveClick', 'DFM event OnClick -> spbSaveClick');
  log('      OnClick targets: ' + click.targets.map(where).join(' | '));
  ok(click.targets.some(i => T(i).kind === 'web' && /ht9045_hotplate_wire\.js$/.test(T(i).file)), 'OnClick has the web target in the wire script');
  ok(click.targets.some(i => T(i).kind === 'port' && /HotPlateForm_File\.cpp$/.test(T(i).file)), 'OnClick has the C++ port target');
  ok(click.targets.some(i => T(i).kind === 'golden' && /cHotPlate\.cpp$/.test(T(i).file) && T(i).line === 440), 'OnClick has the BCB6 target cHotPlate.cpp:440');
  ok(full.props.length > 10 && full.props.some(p => p[0] === 'Caption'), 'DFM properties listed', full.props.length + ' props');
  ok(full.comp.html != null && T(full.comp.html).kind === 'html', 'HTML source location known', where(full.comp.html));
  log('      listeners: ' + full.listeners.map(l => l.type + '/' + l.on + (l.relevant ? '*' : '') + ' ' + (l.handler != null ? where(l.handler) : l.bind != null ? where(l.bind) : '-')).join(' | '));

  // DFM definition, per-property .dfm lines
  ok(full.comp.dfm != null && T(full.comp.dfm).kind === 'dfm' && T(full.comp.dfm).line === 351 && /cHotPlate\.dfm$/i.test(T(full.comp.dfm).file),
    'DFM definition -> cHotPlate.dfm:351', full.comp.dfm != null ? where(full.comp.dfm) + ' ' + T(full.comp.dfm).snippet : '-');
  const cap = full.props.find(p => p[0] === 'Caption');
  ok(cap && cap[4] === 356, 'DFM property Caption -> .dfm line 356', cap ? String(cap[4]) : '-');

  // web command -> C++ dispatch -> handler function
  const rdp = full.cmds.find(c => c.cmd === 'recipe.doc.put');
  ok(rdp && /save\(\)/.test(rdp.via) && rdp.send.length && rdp.dispatch.length && /wb_serve\.cpp$/i.test(T(rdp.dispatch[0]).file),
    'command recipe.doc.put: traced from the handler, sent from the recipe client, dispatched in wb_serve.cpp',
    rdp ? rdp.via + ' | send ' + rdp.send.map(where).join(',') + ' | dispatch ' + rdp.dispatch.map(where).join(',') + ' | handlers ' + rdp.handlers.map(where).join(',') : full.cmds.map(c => c.cmd).join(','));
  ok(click.targets.length <= 12, 'OnClick jump list stays short', click.targets.length + ' targets');

  // where the form code uses the control
  ok(full.uses && full.uses.golden.some(i => /cHotPlate\.cpp$/i.test(T(i).file)) && full.uses.golden.some(i => /cHotPlate\.h$/i.test(T(i).file) && T(i).note === '宣告'),
    'uses of spbSave in BCB6: code + declaration', full.uses ? full.uses.golden.map(where).join(' | ') : '-');
  log('      uses (port): ' + (full.uses ? full.uses.port.map(where).join(' | ') || '(none)' : '-'));
  log('      page notes: ' + full.page.notes.join(' / '));

  // double-click the event in the panel -> quick pick (3 kinds) -> open golden (virtual Big5 doc)
  fake._state.quickPick = items => items.find(i => /BCB6/.test(i.label));
  const ci = full.events.indexOf(click);
  propsRecv({ type: 'openEvent', i: ci });
  await new Promise(r => setTimeout(r, 300));
  const qp = fake._calls.filter(c => c.name === 'quickPick').pop();
  ok(!!qp && qp.args[0].length >= 3, 'double-click event -> quick pick with every location', qp ? qp.args[0].join(' || ') : 'none');
  const opened = fake._state.opened.pop();
  ok(opened && opened.uri.scheme === 'ht9045-golden' && opened.selection.start.line === 439, 'BCB6 opens read-only (virtual doc) at line 440');
  ok(opened && /TfHotPlate::spbSaveClick/.test(opened.text) && /[一-鿿]/.test(opened.text), 'BCB6 text decoded from Big5 (has CJK)');
  ok(opened && opened.readonly && opened.bom, 'BCB6 served by a read-only file system, UTF-8 with BOM');
  let refused = false;
  try { fake._fs.p.writeFile(opened.uri, Buffer.from('x')); } catch (e) { refused = e.code === 'NoPermissions'; }
  ok(refused, 'BCB6 file system refuses writes');

  // double-click a DFM property -> the .dfm opens read-only at that property's line
  propsRecv({ type: 'openDfmLine', line: 356 });
  await new Promise(r => setTimeout(r, 200));
  const od = fake._state.opened.pop();
  ok(od && od.uri.scheme === 'ht9045-golden' && /cHotPlate\.dfm$/i.test(od.uri.fsPath) && od.selection.start.line === 355 && /Caption/.test(od.text.split('\n')[355]),
    'property double-click opens cHotPlate.dfm:356 (Big5, read-only)', od ? od.uri.fsPath + ':' + (od.selection.start.line + 1) : 'none');

  // tree multi-select (Ctrl+click in the outline) -> selectIds to the page, clicked one first
  const kids2 = fake._tree.getChildren(fake._tree.getChildren()[0]);
  const p2 = kids2.find(n => n.id === 'Panel2');
  const pn2 = p2 ? fake._tree.getChildren(p2) : [];
  const nSave = pn2.find(n => n.id === 'spbSave'), nExit = pn2.find(n => n.id === 'sbtExit');
  if (nSave && nExit) {
    pagePosted.length = 0;
    await fake._cmds['ht9045Designer.selectKey'](nExit.key);           // the item's click command
    fake._treeView._sel({ selection: [nSave, nExit] });                 // then the tree's selection
    const si = pagePosted.find(m => m.type === 'selectIds');
    ok(si && si.ids.join(',') === 'sbtExit,spbSave' && si.origin === 'tree', 'tree Ctrl+click multi-select -> selectIds, clicked one primary', si ? JSON.stringify(si.ids) : 'none');
  }

  // find component (quick pick) -> selectKey to the page
  fake._state.quickPick = items => items.find(i => i.label === 'XST1');
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.findComponent']();
  const sk = pagePosted.find(m => m.type === 'selectKey');
  const xnode = R.treeNodes.find(n => n[2] === 'XST1');
  ok(sk && xnode && sk.key === xnode[0], 'find component -> selects XST1 in the page');

  // eventJump=web: direct jump without asking
  fake._config.eventJump = 'web';
  const before = fake._calls.filter(c => c.name === 'quickPick').length;
  propsRecv({ type: 'openEvent', i: ci });
  await new Promise(r => setTimeout(r, 200));
  const o2 = fake._state.opened.pop();
  ok(fake._calls.filter(c => c.name === 'quickPick').length === before && o2 && /ht9045_hotplate_wire\.js$/i.test(o2.uri.fsPath),
    'eventJump=web opens the web JS directly', o2 ? path.basename(o2.uri.fsPath) + ':' + (o2.selection.start.line + 1) : 'none');

  // double-click on the component in the page = default event
  fake._config.eventJump = 'cpp';
  pageRecv({ __htd: 1, type: 'dblclick', key: R.selInfo.key });
  await new Promise(r => setTimeout(r, 200));
  const o3 = fake._state.opened.pop();
  ok(o3 && /\.cpp$/i.test(o3.uri.fsPath) && o3.uri.scheme === 'file' && o3.uri.fsPath.toLowerCase().startsWith(PORT.toLowerCase()),
    'double-click component (eventJump=cpp) opens C++ in the port tree', o3 ? path.basename(o3.uri.fsPath) + ':' + (o3.selection.start.line + 1) : 'none');

  // a double-click on a property's name in the grid: the page's HTML opens with that value selected (spbSave's Left = 54px,
  // its Caption = Save); a property not written there (Font.Name) = the element itself
  const rpOff = (t, p) => { const ls = t.split('\n'); let o = 0; for (let i = 0; i < p.line; i++) o += ls[i].length + 1; return o + p.character; };
  const rpSelText = o => (o && o.selection) ? text.slice(rpOff(text, o.selection.start), rpOff(text, o.selection.end)) : null;   // (the page's text: this test's doc)
  const rpGot = [];
  for (const pr of ['Left', 'Caption', 'Font.Name']) {
    propsRecv({ type: 'revealProp', prop: pr });
    await new Promise(r => setTimeout(r, 120));
    const orp = fake._state.opened.pop();
    rpGot.push(orp && /Setup\.HotPlate\.html$/i.test(orp.uri.fsPath) ? rpSelText(orp) : 'not opened');
  }
  ok(rpGot[0] === '54px' && rpGot[1] === 'Save' && /id="spbSave"|spbSave/.test(rpGot[2] || ''),
    'a double-click on a property name (WPF-quick code): the HTML opens with the value selected -- Left 54px, Caption Save; Font.Name (not written) = the element', JSON.stringify(rpGot));

  // code -> designer: CodeLens above the BCB6 handler, and it selects the control
  // (AI 20261001: the BCB6 tree the extension found -- 906 on the machine, 912 on another PC -- not a fixed path)
  const gfile = path.join(api.hub.active.r.goldenRoot || path.join('D:\\HT9045', 'HT9011UC_Code_V3.33.906.0_20260618'), 'cHotPlate.cpp');
  const gtext = require('../lib/cppindex').decodeBig5(fs.readFileSync(gfile));
  const gstarts = [0];
  for (let i = 0; i < gtext.length; i++) if (gtext.charCodeAt(i) === 10) gstarts.push(i + 1);
  const gdoc = {
    uri: fake.Uri.file(gfile).with({ scheme: 'ht9045-golden' }), getText: () => gtext,
    positionAt: off => { let lo = 0, hi = gstarts.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (gstarts[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - gstarts[lo]); },
  };
  let lenses = fake._lens.provideCodeLenses(gdoc);
  for (let i = 0; i < 100 && !lenses.length; i++) { await new Promise(r => setTimeout(r, 50)); lenses = fake._lens.provideCodeLenses(gdoc); }
  const saveLens = lenses.find(l => /spbSave\.OnClick/.test(l.command.title));
  ok(!!saveLens && saveLens.range.start.line === 439, 'CodeLens above BCB6 TfHotPlate::spbSaveClick (line 440)',
    lenses.length + ' lenses; ' + (saveLens ? saveLens.command.title + ' @' + (saveLens.range.start.line + 1) : lenses.slice(0, 3).map(l => l.command.title).join(' / ')));
  const kpLens = lenses.find(l => /OnKeyPress 等/.test(l.command.title));
  ok(!!kpLens, 'CodeLens for a shared handler says how many controls', kpLens ? kpLens.command.title : '');
  pagePosted.length = 0;
  await fake._cmds[saveLens.command.command].apply(null, saveLens.command.arguments);
  const sel2 = pagePosted.find(m => m.type === 'selectId');
  ok(sel2 && sel2.id === 'spbSave' && sel2.origin === 'code', 'clicking the CodeLens selects spbSave in the designer', sel2 ? JSON.stringify(sel2) : 'nothing posted');

  // C++ -> web: lens above the server's dispatch of io.btnPanelClick
  const wsFile = path.join(PORT, 'tools', 'wb_serve.cpp');
  const wsText = fs.readFileSync(wsFile, 'utf8');
  const wsStarts = [0];
  for (let i = 0; i < wsText.length; i++) if (wsText.charCodeAt(i) === 10) wsStarts.push(i + 1);
  const wsDoc = {
    uri: fake.Uri.file(wsFile), getText: () => wsText,
    positionAt: off => { let lo = 0, hi = wsStarts.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (wsStarts[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - wsStarts[lo]); },
  };
  let wl = fake._lens.provideCodeLenses(wsDoc);
  for (let i = 0; i < 100 && !wl.some(l => l.command.command === 'ht9045Designer.openWebSenders'); i++) { await new Promise(r => setTimeout(r, 50)); wl = fake._lens.provideCodeLenses(wsDoc); }
  const ioLens = wl.find(l => l.command.command === 'ht9045Designer.openWebSenders' && l.command.arguments[0] === 'io.btnPanelClick');
  ok(!!ioLens && /HW\.IoSetView\.html/.test(ioLens.command.title), 'CodeLens above wb_serve.cpp dispatch: web sender of io.btnPanelClick',
    wl.filter(l => l.command.command === 'ht9045Designer.openWebSenders').length + ' command lenses; ' + (ioLens ? ioLens.command.title + ' @' + (ioLens.range.start.line + 1) : '-'));
  fake._state.quickPick = items => items[0];
  await fake._cmds['ht9045Designer.openWebSenders'].apply(null, ioLens.command.arguments);
  const ow = fake._state.opened.pop();
  ok(ow && /\.js$/i.test(ow.uri.fsPath), 'clicking it opens the web script at the send', ow ? path.basename(ow.uri.fsPath) + ':' + (ow.selection.start.line + 1) : 'none');

  // data field of an edit box: XST1 -> [Hotplate Form] X Start -> C++ / BCB6 read/write lines
  pageRecv({ __htd: 1, type: 'select', info: { key: 77, id: 'XST1', tag: 'input', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'click' });
  let xs = null;
  for (let i = 0; i < 200 && !xs; i++) {
    await new Promise(r => setTimeout(r, 30));
    const last = posted.filter(m => m.type === 'show' && m.data).pop();
    if (last && last.data.comp.htmlId === 'XST1' && !last.data.cppPending) xs = last.data;
  }
  const xf = xs && xs.fields && xs.fields.find(f => f.section === 'Hotplate Form' && f.key === 'X Start');
  ok(xf && xf.golden.some(i => /cHotPlate\.cpp$/i.test(xs.targets[i].file)) && xf.port.length > 0 &&
    xs.fields.filter(f => f.key === 'X Start').length === 1 && xf.web.length >= 2,
    'XST1 data field [Hotplate Form] X Start (one entry, both wire maps), with C++ and BCB6 read/write lines',
    xf ? 'web ' + xf.web.map(i => path.basename(xs.targets[i].file) + ':' + xs.targets[i].line).join(',') + ' | port ' + xf.port.map(i => path.basename(xs.targets[i].file) + ':' + xs.targets[i].line + '(' + xs.targets[i].note + ')').join(',') +
      ' | golden ' + xf.golden.map(i => path.basename(xs.targets[i].file) + ':' + xs.targets[i].line + '(' + xs.targets[i].note + ')').join(',') : (xs ? JSON.stringify(xs.fields) : 'no data'));
  // and back: a lens above the BCB6 WriteIniData(…, "Hotplate Form", "X Start", …)
  const fl = fake._lens.provideCodeLenses(gdoc).find(l => /網頁欄位 \[Hotplate Form\] X Start/.test(l.command.title));
  ok(!!fl && /XST1/.test(fl.command.title) && !/等/.test(fl.command.title), 'CodeLens above the BCB6 INI line: 網頁欄位 → XST1 (one control, not counted twice)',
    fl ? fl.command.title + ' @' + (fl.range.start.line + 1) : '-');

  // wiring overview of HotPlate: the probe answers listenersAll, the panel gets the table
  const origPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'listenersAll') setTimeout(() => pageRecv({ __htd: 1, type: 'listenersAll', seq: m.seq, map: { sbtExit: ['click'] } }), 10);
    return origPM(m);
  };
  pageRecv({ __htd: 1, type: 'select', info: R.selInfo, origin: 'click' });   // make HotPlate the active designer again
  await new Promise(r => setTimeout(r, 100));
  const ov = await fake._cmds['ht9045Designer.pageOverview']();
  const opanel = (fake._panels || []).find(p => p.viewType === 'ht9045Designer.overview');
  ok(ov && ov.listenersOk && ov.summary.events === 19 && opanel, 'page overview: 19 events on HotPlate, probe answered, panel opened',
    ov ? JSON.stringify(ov.summary.web) + ' ' + JSON.stringify(ov.summary.port) : 'none');
  // real panel data for test\panels_render.ps1 (renders media/props.js and overview.js in Edge)
  const dumpDir = path.join(require('os').tmpdir(), 'htdesigner_panels');
  fs.mkdirSync(dumpDir, { recursive: true });
  // 1005 (WPF audit): the instance list -- every component of the page, the form first, in page order, by depth
  {
    const cl = (full && full.comps) || [];
    const sp = cl.find(x => x.id === 'spbSave'), p2 = cl.find(x => x.id === 'Panel2');
    ok(cl.length >= 20 && cl[0].form && cl[0].id === '@form' && sp && p2 && sp.depth > p2.depth && cl.indexOf(p2) < cl.indexOf(sp) && cl[0].name === '表單' && typeof sp.cls === 'string',
      'props panel: the instance list (BCB6 Object Inspector / WPF) -- the page\'s components, the form first, page order, nested deeper', JSON.stringify({ n: cl.length, first: cl[0], sp, p2 }));
  }
  fs.writeFileSync(path.join(dumpDir, 'props_spbSave.json'), JSON.stringify(full), 'utf8');
  // the same with sbtExit also selected (2 selected: the spacing buttons need 3)
  if (full && full.edit) fs.writeFileSync(path.join(dumpDir, 'props_multi.json'), JSON.stringify(Object.assign({}, full, { edit: Object.assign({}, full.edit,
    { multi: ['sbtExit'], mixed: { left: true, fontSize: true, bold: true, color: true } }) })), 'utf8');
  if (xs) fs.writeFileSync(path.join(dumpDir, 'props_XST1.json'), JSON.stringify(xs), 'utf8');
  if (ov) fs.writeFileSync(path.join(dumpDir, 'overview_HotPlate.json'), JSON.stringify(ov), 'utf8');
  if (opanel) {
    opanel.recv({ type: 'ready' });
    ok(opanel.posted.some(m => m.type === 'data' && m.data.rows.length === 11), 'overview panel receives the rows');
    pagePosted.length = 0;
    opanel.recv({ type: 'select', id: 'XST1' });
    ok(pagePosted.some(m => m.type === 'selectId' && m.id === 'XST1' && m.origin === 'overview'), 'clicking a row selects the control in the designer');
  }

  // 與 DFM 的差異: the probe answers lookAll with the page's real look of spbSave (from the probe run)
  const realLook = full.edit && full.edit.look;
  const realLay = full.edit && full.edit.layout;
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') {
      setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, askedIds: m.ids.length, items: [
        { id: 'spbSave', lay: realLay, cap: full.edit.caption, look: realLook },
      ] }), 10);
    }
    return origPM(m);
  };
  const dfr = await fake._cmds['ht9045Designer.dfmDiff']();
  const dpanel = (fake._panels || []).find(p => p.viewType === 'ht9045Designer.dfmDiff');
  // Font.Size 14 is what the generator makes of Font.Height -16; the save style's bold is not in the DFM
  ok(dfr && dpanel && dfr.controls === 1 && dfr.rows.map(r => r.prop).join(',') === 'Font.Bold' && dfr.rows.every(r => r.inSource),
    'DFM difference list (spbSave): only Font.Bold, panel opened, writable', dfr ? JSON.stringify(dfr.rows.map(r => r.prop + ' ' + r.page + '/' + r.dfm)) : 'none');
  if (dfr) fs.writeFileSync(path.join(dumpDir, 'dfmdiff_HotPlate.json'), JSON.stringify(dfr), 'utf8');
  // from the right-click menu / title bar VS Code passes the webview context / the Uri first
  const dfm2 = await fake._cmds['ht9045Designer.dfmDiff']({ webview: 'ht9045Designer.editor', webviewSection: 'htdDesign' });
  const dfm3 = await fake._cmds['ht9045Designer.dfmDiff'](fake.Uri.file(PAGE));
  ok(dfm2 && dfm3 && dfm2.rows.length === dfr.rows.length && dfm3.rows.length === dfr.rows.length,
    'DFM difference list from a menu (context / Uri argument) works the same');
  if (dpanel) {
    pagePosted.length = 0;
    dpanel.recv({ type: 'reset', id: 'spbSave', reset: dfr.rows.find(r => r.prop === 'Font.Bold').reset });
    ok(pagePosted.some(m => m.type === 'setLook' && m.id === 'spbSave' && m.prop === 'bold' && m.value === false), 'reset posts setLook bold=false for spbSave (by id)',
      JSON.stringify(pagePosted.filter(m => m.type !== 'diag')));
    pagePosted.length = 0;
    dpanel.recv({ type: 'reset', id: 'spbSave', reset: { type: 'evil', value: 1 } });
    ok(!pagePosted.some(m => m.type === 'evil'), 'the panel can only ask for setLayout / setCaption / setLook');
  }
  panel.webview.postMessage = origPM;

  // DFM 位置: spbSave 10px right of the DFM (54) -> one frame at left 54; the list's button turns it off / on
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') {
      setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: [
        { id: 'spbSave', lay: Object.assign({}, realLay, { left: 64 }), cap: full.edit.caption, look: realLook },
      ] }), 10);
    }
    return origPM(m);
  };
  pagePosted.length = 0;
  const ghOn = await fake._cmds['ht9045Designer.toggleDfmGhosts']();
  const ghMsg = pagePosted.filter(m => m.type === 'dfmGhosts').pop();
  ok(ghOn && ghOn.on && ghOn.items.length === 1 && ghOn.items[0].id === 'spbSave' && ghOn.items[0].left === 54 && !('width' in ghOn.items[0]) &&
    ghMsg && ghMsg.items === ghOn.items && /DFM 位置/.test(api.hub.status.text),
    'DFM 位置: spbSave at 64 -> a frame at the DFM left 54 (only the side that differs); the status bar says so', JSON.stringify(ghOn && ghOn.items));
  let ghOff = null, ghData = null, ghBack = null;
  if (dpanel) {
    pagePosted.length = 0;
    ghOff = await dpanel.recv({ type: 'ghosts' });
    ghData = dpanel.posted.filter(m => m.type === 'data').pop();
    const offMsg = pagePosted.filter(m => m.type === 'dfmGhosts').pop();
    const offOk = ghOff && ghOff.on === false && offMsg && offMsg.items === null && ghData && ghData.data.ghostsOn === false;
    ghBack = await dpanel.recv({ type: 'ghosts' });
    const backData = dpanel.posted.filter(m => m.type === 'data').pop();
    ok(offOk && ghBack && ghBack.on && backData && backData.data.ghostsOn === true && /DFM 位置/.test(api.hub.status.text),
      'the list\'s button: off (frames gone, the button says off), on again (the list says on)');
  }
  const ghEnd = await fake._cmds['ht9045Designer.toggleDfmGhosts'](false);
  ok(ghEnd && ghEnd.on === false && !/DFM 位置/.test(api.hub.status.text), 'DFM 位置 off');
  // turned off while a comparison is still on its way (the page answers late): it stays off
  const ghFastPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: [
      { id: 'spbSave', lay: Object.assign({}, realLay, { left: 64 }), cap: full.edit.caption, look: realLook }] }), 80);
    return origPM(m);
  };
  pagePosted.length = 0;
  const ghLate = fake._cmds['ht9045Designer.toggleDfmGhosts'](true);
  await new Promise(r => setTimeout(r, 10));
  await fake._cmds['ht9045Designer.toggleDfmGhosts'](false);
  const ghLateRes = await ghLate;
  const ghLast = pagePosted.filter(m => m.type === 'dfmGhosts').pop();
  ok(ghLateRes && ghLateRes.on === false && !(api.hub.active && api.hub.active.dfmGhosts) && ghLast && ghLast.items === null && !/DFM 位置/.test(api.hub.status.text),
    'DFM 位置 turned off while it compared: the late answer does not turn it back on', JSON.stringify(ghLateRes));
  // the toggle clicked twice quickly (the first "on" still waiting): the second one means off
  pagePosted.length = 0;
  const ghT1 = fake._cmds['ht9045Designer.toggleDfmGhosts']();
  await new Promise(r => setTimeout(r, 10));
  const ghT2 = await fake._cmds['ht9045Designer.toggleDfmGhosts']();
  const ghT1r = await ghT1;
  ok(ghT2 && ghT2.on === false && ghT1r && ghT1r.on === false && !(api.hub.active && api.hub.active.dfmGhosts),
    'DFM 位置 clicked twice quickly: off in the end', JSON.stringify([ghT1r, ghT2]));
  panel.webview.postMessage = ghFastPM;
  // Tab 順序: the .dfm's order goes to the probe; its reply (2 stops out of order) -> status bar + output panel
  const toPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'tabOrder' && m.dfm) setTimeout(() => pageRecv({ __htd: 1, type: 'tabOrderInfo', seq: m.seq, on: true, stops: 13, common: 11, bad: ['XST1（網頁第 5、DFM 第 4）', 'cbEnableHP1（網頁第 4、DFM 第 5）'] }), 5);
    return toPM(m);
  };
  pagePosted.length = 0;
  const toOn = await fake._cmds['ht9045Designer.toggleTabOrder']();
  const toMsg = pagePosted.filter(m => m.type === 'tabOrder').pop();
  ok(!!(toOn && toOn.on && toMsg && Array.isArray(toMsg.dfm) && toMsg.dfm[0] === 'HotPlateName' && toOn.bad.length === 2 && toOn.common === 11 && /Tab 順序/.test(api.hub.status.text)),
    'Tab 順序: the .dfm order (HotPlateName first) to the probe; 2 out of order reported; the status bar says so', toMsg ? toMsg.dfm.slice(0, 4).join(',') : '-');
  const toOff = await fake._cmds['ht9045Designer.toggleTabOrder']();
  ok(toOff && toOff.on === false && pagePosted.filter(m => m.type === 'tabOrder').pop().dfm === null && !/Tab 順序/.test(api.hub.status.text), 'Tab 順序 off');
  panel.webview.postMessage = toPM;

  // 改回 DFM on the selection (spbSave: moved to 64, bold; the DFM says 54, not bold) -> ONE editMany with both
  // (the position is read from the source: the move is in it, whatever the page has drawn so far)
  pageRecv({ __htd: 1, type: 'select', info: R.selInfo, origin: 'click' });
  await new Promise(r => setTimeout(r, 30));
  const rdText0 = text;
  doc._setText(text.replace(/(id="spbSave"[^>]*?left:)54px/, '$164px'));
  pagePosted.length = 0;
  const rdRes = await fake._cmds['ht9045Designer.resetToDfm']();
  const rdMsg = pagePosted.filter(m => m.type === 'editMany');
  ok(rdRes && rdRes.controls === 1 && rdMsg.length === 1 && rdMsg[0].items.some(i => i.type === 'setLayout' && i.id === 'spbSave' && i.left === 54) &&
    rdMsg[0].items.some(i => i.type === 'setLook' && i.prop === 'bold' && i.value === false),
    'reset to DFM: spbSave -> one editMany {setLayout left 54, setLook bold false}', JSON.stringify(rdMsg[0] && rdMsg[0].items));
  // WPF Layout > Reset one part: the position only (left 54, not the bold), the size only (the same as the DFM: nothing),
  // the whole layout (the position, no look)
  pagePosted.length = 0;
  const rpPos = await fake._cmds['ht9045Designer.resetLayout.pos']();
  const rpPosMsg = pagePosted.filter(m => m.type === 'editMany').pop();
  pagePosted.length = 0;
  const rpSize = await fake._cmds['ht9045Designer.resetLayout.size']();
  const rpSizeMsgs = pagePosted.filter(m => m.type === 'editMany').length;
  const rpLay = await fake._cmds['ht9045Designer.resetLayout.layout']();
  ok(!!(rpPos && rpPos.only === 'pos' && rpPosMsg && rpPosMsg.items.length === 1 && rpPosMsg.items[0].type === 'setLayout' && rpPosMsg.items[0].left === 54 &&
    rpPosMsg.items[0].width === undefined && rpSize && rpSize.count === 0 && rpSizeMsgs === 0 &&
    rpLay && rpLay.items.every(i => i.type === 'setLayout') && rpLay.items.some(i => i.left === 54)),
    'Layout > Reset (WPF): position only = left 54 without the bold; size only = nothing (same as the DFM); whole layout = setLayout only',
    JSON.stringify({ pos: rpPosMsg && rpPosMsg.items, size: rpSize && rpSize.count, lay: rpLay && rpLay.items }));
  // the page not drawn again yet (it still says 54): the source's 64 counts
  const rdPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') { setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: [{ id: 'spbSave', lay: Object.assign({}, realLay, { left: 54 }), cap: full.edit.caption, look: realLook }] }), 10); return Promise.resolve(true); }
    return rdPM(m);
  };
  pagePosted.length = 0;
  const rdStale = await fake._cmds['ht9045Designer.resetToDfm']();
  panel.webview.postMessage = rdPM;
  ok(!!(rdStale && rdStale.items.some(i => i.type === 'setLayout' && i.id === 'spbSave' && i.left === 54)),
    'reset to DFM right after a move (the page still shows the old place): the move is in the source, so it is reset too', JSON.stringify(rdStale && rdStale.items));
  // AI(W906-HTDESIGNER) 20261001: one property's Reset with several selected (the panel's resetProp) = each one back to
  // ITS OWN .dfm value, that property only (it used to write the primary one's value into all of them)
  pagePosted.length = 0;
  await propsRecv({ type: 'resetProp', prop: 'Font.Bold' });
  const rpProp = pagePosted.filter(m => m.type === 'editMany').pop();
  ok(!!(rpProp && rpProp.items.length === 1 && rpProp.items[0].type === 'setLook' && rpProp.items[0].prop === 'bold' && rpProp.items[0].value === false && rpProp.items[0].id === 'spbSave'),
    'one property\'s Reset (resetProp Font.Bold) = only that property, from each one\'s own .dfm (the move is left alone)', JSON.stringify(rpProp && rpProp.items));
  doc._setText(rdText0);
  pagePosted.length = 0;
  await propsRecv({ type: 'resetToDfm' });
  ok(pagePosted.filter(m => m.type === 'editMany').length === 1, 'the properties panel\'s 全部改回 DFM does the same');
  panel.webview.postMessage = origPM;

  // "全部改回" of a pattern: every TLabel bold on the page, none in the DFM -> one line; the button sends them all
  const ptLabels = Array.from(require('../lib/pageinfo').collectIds(text)).filter(id => /^Label\d+$/.test(id));
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') {
      setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: ptLabels.map(id => ({
        id, lay: null, cap: null, look: { visible: true, enabled: null, fontSize: 0, fontName: 'x', fontFamilies: ['x'], fontOwn: false, bold: true, italic: false, color: '', background: '' },
      })) }), 5);
    }
    return origPM(m);
  };
  const ptDiff = await fake._cmds['ht9045Designer.dfmDiff']();
  panel.webview.postMessage = origPM;
  const ptPat = ptDiff && ptDiff.patterns.find(p => p.cls === 'TLabel' && p.prop === 'Font.Bold');
  if (ptDiff) fs.writeFileSync(path.join(dumpDir, 'dfmdiff_pattern.json'), JSON.stringify(ptDiff), 'utf8');
  pagePosted.length = 0;
  const ptRes = ptPat && dpanel ? await dpanel.recv({ type: 'resetPattern', key: ptPat.key, confirmed: true }) : null;
  const ptSent = pagePosted.filter(m => m.type === 'editMany').pop();
  ok(ptPat && ptPat.count >= 5 && ptRes && ptSent && ptSent.items.length === ptPat.count && ptSent.items.every(i => i.type === 'setLook' && i.prop === 'bold' && i.value === false),
    'pattern "every TLabel bold": 全部改回 sends all ' + (ptPat ? ptPat.count : 0) + ' to the probe in one message (bold = DFM\'s false)');
  const ptLay = ptDiff && ptDiff.patterns.find(p => p.group === 'layout');
  ok(!ptLay || (await dpanel.recv({ type: 'resetPattern', key: ptLay.key, confirmed: true })) === null, 'a position pattern is not reset in bulk');
  // an AutoSize=False pattern: 全部改回 gives every label its own .dfm size (not the one its text makes)
  const asPanel = api.hub.active && api.hub.active.dfmDiff;
  if (asPanel) {
    const asKey = 'TLabel|AutoSize|True|False';
    const asData0 = asPanel.data;
    asPanel.data = { patterns: [{ key: asKey, cls: 'TLabel', group: 'look', prop: 'AutoSize', page: 'True', dfm: 'False', count: 2 }], rows: [
      { id: 'Label1', cls: 'TLabel', group: 'look', prop: 'AutoSize', page: 'True', dfm: 'False', pattern: asKey, inSource: true, reset: { type: 'setLook', prop: 'autoSize', value: false, size: { width: 136, height: 24 } } },
      { id: 'Label2', cls: 'TLabel', group: 'look', prop: 'AutoSize', page: 'True', dfm: 'False', pattern: asKey, inSource: true, reset: { type: 'setLook', prop: 'autoSize', value: false, size: { width: 39, height: 16 } } },
    ] };
    pagePosted.length = 0;
    await dpanel.recv({ type: 'resetPattern', key: asKey, confirmed: true });
    const asSent = pagePosted.filter(m => m.type === 'editMany').pop();
    ok(!!(asSent && asSent.items.length === 2 && asSent.items[0].size && asSent.items[0].size.width === 136 && asSent.items[1].size.width === 39),
      'an AutoSize=False pattern: 全部改回 sends each label with its own .dfm size', JSON.stringify(asSent && asSent.items));
    asPanel.data = asData0;
  } else ok(false, 'the DFM list is open for the AutoSize pattern check');

  // 頁面 list: every page by group (Main first), HotPlate marked as the current page, a click opens it
  const pv = fake._views && fake._views['ht9045Designer.pages'];
  const pgroups = pv ? pv.provider.getChildren() : [];
  const psetup = pgroups.find(g => g.name === 'Setup');
  const php = psetup && psetup.children.find(p => p.name === 'Setup.HotPlate.html');
  const phpItem = php ? pv.provider.getTreeItem(php) : null;
  ok(pgroups.length >= 5 && pgroups[0].name === 'Main' && php && php.label === 'HotPlate' && phpItem && /● 目前/.test(phpItem.description) &&
    phpItem.command.command === 'ht9045Designer.openPageFile',
    '頁面 list: groups (Main first), Setup › HotPlate marked as the current page', pgroups.map(g => g.name + ' ' + g.children.length).join(', '));
  const psItem = psetup ? pv.provider.getTreeItem(psetup) : null;
  const pmItem = pv ? pv.provider.getTreeItem(pgroups[0]) : null;
  ok(psItem && psItem.collapsibleState === fake.TreeItemCollapsibleState.Expanded && pmItem && pmItem.collapsibleState === fake.TreeItemCollapsibleState.Collapsed,
    'the group of the current page is open, the others closed');
  const nExec = fake._calls.length;
  if (php) await fake._cmds['ht9045Designer.openPageFile'](php.file);
  ok(fake._calls.slice(nExec).some(c => c.name === 'exec' && c.args[0] === 'vscode.openWith' && /Setup\.HotPlate\.html$/i.test(c.args[1]) && c.args[2] === 'ht9045Designer.editor'),
    'a click on a page runs vscode.openWith(…, the designer)', JSON.stringify(fake._calls.slice(nExec).map(c => c.args)));
  const all = pgroups.reduce((n, g) => n + g.children.length, 0);
  const onDisk = fs.readdirSync(path.dirname(PAGE)).filter(n => /\.html?$/i.test(n)).length + fs.readdirSync(path.join(path.dirname(PAGE), '..')).filter(n => /\.html?$/i.test(n)).length;
  ok(all === onDisk && pgroups[pgroups.length - 1].name === 'web 根目錄', 'every .html of web\\page and web\\ is listed (web root last)', all + ' listed, ' + onDisk + ' on disk');

  // hidden in the designer only (the eye): the probe gets the whole set, the tree shows it, the workspace state keeps it
  const nCallsEye = fake._calls.length;
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.designHide']({ id: 'Panel2', isForm: false });
  const eyeMsg = pagePosted.filter(m => m.type === 'designHidden').pop();
  const eyeNode = Array.from(fake._tree.byKey.values()).find(n => n.id === 'Panel2');
  const eyeItem = eyeNode ? fake._tree.getTreeItem(eyeNode) : null;
  ok(eyeMsg && JSON.stringify(eyeMsg.ids) === '["Panel2"]' && eyeItem && eyeItem.contextValue === 'component.hidden' && /設計時隱藏/.test(eyeItem.description) &&
    JSON.stringify(api.hub.loadDesignHidden(PAGE)) === '["Panel2"]',
    'eye: Panel2 hidden in the designer (probe told, tree marks it, kept for the page)', JSON.stringify(eyeMsg) + ' | ' + (eyeItem && eyeItem.description));
  await fake._cmds['ht9045Designer.designHide']({ id: '@form', isForm: true });
  ok(JSON.stringify(Array.from(api.hub.active.designHidden)) === '["Panel2"]', 'the form itself cannot be hidden');
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.designUnhideAll']();
  const eyeMsg2 = pagePosted.filter(m => m.type === 'designHidden').pop();
  ok(eyeMsg2 && eyeMsg2.ids.length === 0 && api.hub.loadDesignHidden(PAGE).length === 0 && fake._tree.getTreeItem(eyeNode).contextValue === 'component',
    'show all: nothing hidden, nothing kept, the tree is back');
  // the source was never touched by any of it
  ok(!fake._calls.slice(nCallsEye).some(c => c.name === 'applyEditCall'), 'hiding in the designer writes nothing into the page (no WorkspaceEdit)');

  // WPF Copy / Paste / Delete / Cut of whole components -- text of the source, one WorkspaceEdit each
  const edHub = api.hub;
  const edD = edHub.active;
  const edBefore = text;
  const selAs = async (id, tag) => {
    pageRecv({ __htd: 1, type: 'select', info: { key: 900 + id.length, id, tag, title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'click' });
    await new Promise(r => setTimeout(r, 30));
  };
  await selAs('spbSave', 'button');
  const edClip = await fake._cmds['ht9045Designer.copyComponent']();
  ok(edClip && edClip.ids[0] === 'spbSave' && /^<button[^>]*id="spbSave"/.test(edClip.blocks[0]) && /<\/button>$/.test(edClip.blocks[0]),
    'copy: the whole <button id="spbSave"> … </button>', edClip ? edClip.blocks[0].slice(0, 60) + ' … ' + edClip.blocks[0].slice(-12) : '-');
  const edCalls = fake._calls.length;
  const edPos = /id="spbSave"[^>]*left:(\d+)px;top:(\d+)px/.exec(edBefore);
  const edWant = edPos ? 'left:' + (+edPos[1] + 8) + 'px;top:' + (+edPos[2] + 8) + 'px' : '?';
  const edPaste = await fake._cmds['ht9045Designer.pasteComponent']();
  ok(edPaste && edPaste.made[0] === 'spbSave_2' && new RegExp('id="spbSave_2"[^>]*' + edWant).test(text) && text.split('id="spbSave"').length === 2 &&
    /title="spbSave_2 : TSpeedButton/.test(text) && edD.lastSelId === 'spbSave_2' &&
    fake._calls.slice(edCalls).filter(c => c.name === 'applyEditCall').length === 1,
    'paste: a new spbSave_2 next to it, 8 px down-right, ONE WorkspaceEdit, selected afterwards', edPaste ? JSON.stringify(edPaste) : '-');
  // the page's JS uses "spbSave": Delete asks first; closed without an answer = nothing removed
  fake._answer = undefined;
  const edNo = await fake._cmds['ht9045Designer.deleteComponent']();
  const edAsked = fake._calls.filter(c => c.name === 'warning').pop();
  ok(edNo === null && /id="spbSave"/.test(text) && edAsked && /spbSave/.test(edAsked.args[0]) && /hotplate_wire\.js/i.test(edAsked.args[0]),
    'delete asks first when the page JS uses it (names the script); no answer = nothing removed', edAsked ? edAsked.args[0].split('\n')[1] : '-');
  // spbSave_2 is used by no script: removed at once, and the page is exactly as before
  await selAs('spbSave_2', 'button');
  const edDel = await fake._cmds['ht9045Designer.deleteComponent']();
  ok(edDel && edDel.removed[0] === 'spbSave_2' && edDel.uses.length === 0 && text === edBefore, 'delete spbSave_2: no question, the page is byte-identical to before the paste');
  // paste INTO a container (Panel2 is a TPanel): inside it, the ids new again
  await selAs('Panel2', 'div');
  const edIn = await fake._cmds['ht9045Designer.pasteComponent']();
  const p2Range = (() => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; })();
  ok(edIn && edIn.into === 'Panel2' && edIn.made[0] === 'spbSave_2' && p2Range.indexOf('id="spbSave_2"') > 0, 'paste into Panel2 (a TPanel): the new button is inside it',
    edIn ? JSON.stringify(edIn) : '-');
  // Cut = copy + delete, the question answered "剪下"
  await selAs('spbSave_2', 'button');
  fake._answer = '剪下';
  const edCut = await fake._cmds['ht9045Designer.cutComponent']();
  ok(edCut && edCut.removed[0] === 'spbSave_2' && text === edBefore && edHub.clip.ids[0] === 'spbSave_2', 'cut: copied, then removed; the page as before');
  fake._answer = undefined;
  doc._setText(edBefore);
  await selAs('spbSave', 'button');

  // WPF Order: spbSave's place among Panel2's children (later in the source = drawn on top)
  const orP2Of = () => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; };
  const orFront = await fake._cmds['ht9045Designer.order.front']();
  ok(orFront && orFront.to === orFront.of - 1 && orP2Of().indexOf('id="sbtExit"') < orP2Of().indexOf('id="spbSave"'),
    'order front: spbSave last in Panel2 (drawn on top)', JSON.stringify(orFront));
  // 1005 (WPF audit): the move put spbSave after sbtExit -- two buttons, so Tab goes the other way now: said (not silent);
  // a move among things Tab does not stop at says nothing
  const tnSay = api.hub.tabOrderNote('<div><button id="a"></button><button id="b"></button></div>', '<div><button id="b"></button><button id="a"></button></div>');
  const tnQuiet = api.hub.tabOrderNote('<div><span id="a"></span><button id="b"></button><span id="c"></span></div>', '<div><span id="c"></span><button id="b"></button><span id="a"></span></div>');
  ok(orFront.tabNote === true && /Tab 順序/.test(tnSay) && tnQuiet === '', 'order: when the move changes the Tab order (no tabindex: Tab = source order) the status line says so; a move of labels / panels says nothing',
    JSON.stringify({ tabNote: orFront.tabNote, tnSay, tnQuiet }));
  const orAgain = await fake._cmds['ht9045Designer.order.front']();
  ok(orAgain && orAgain.same === true, 'front again: already on top, nothing changed');
  const orBack = await fake._cmds['ht9045Designer.order.back']();
  ok(orBack && orBack.to === 1 && orP2Of().indexOf('id="spbSave"') < orP2Of().indexOf('id="sbtExit"') && /^<div[^>]*id="Panel2"[^>]*>\s*<span class="pnlCap"/.test(orP2Of()),
    'order back: first after the caption span (which stays first)', JSON.stringify(orBack));
  doc._setText(edBefore);
  // AI(W906-HTDESIGNER) 20261001: Order with two selected = both, each among its siblings, their order kept, ONE edit
  // (it used to move the primary one only)
  await selAs('spbSave', 'button');
  const omInfo = edD.sel.info, omMulti0 = omInfo.multi;
  // (spbSave in Panel2, Label1 in another container: each goes on top of its own siblings, in ONE edit)
  omInfo.multi = ['Label1'];
  const omKidsOf = id => hb2.childrenOf(text, hb2.parentOf(text, api.hub.unitFor(text, id).start)).map(k => (/\bid="([^"]+)"/.exec(k.text.slice(0, k.text.indexOf('>') + 1)) || [])[1] || '?');
  const omCalls = fake._calls.length;
  const omP0 = omKidsOf('spbSave'), omL0 = omKidsOf('Label1');
  const omFront = await fake._cmds['ht9045Designer.order.front']();
  const omP1 = omKidsOf('spbSave'), omL1 = omKidsOf('Label1');
  const omEdits = fake._calls.slice(omCalls).filter(c => c.name === 'applyEditCall').length;
  doc._setText(edBefore);
  // one step back with both of Panel2's selected, the first already at the bottom: the other does not jump over it
  omInfo.multi = ['sbtExit'];
  const omText0 = text;
  const omBack = await fake._cmds['ht9045Designer.order.backward']();
  const omBackSame = text === omText0;
  omInfo.multi = omMulti0;
  doc._setText(edBefore);
  ok(!!(omFront && omFront.moved.length === 2 && omP1[omP1.length - 1] === 'spbSave' && omL1[omL1.length - 1] === 'Label1' && omEdits === 1 &&
    omBack && omBack.same && omBackSame),
    'order with 2 selected: each on top of its own siblings in ONE edit (not only the primary one); one step back never makes one jump over the other',
    JSON.stringify({ panel: [omP0, omP1], label: [omL0, omL1], edits: omEdits, back: omBack }));
  // 選取這裡的元件… (Blend's Set Current Selection): the list under the right-click from the probe, one picked = selected
  const shPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'stackAt') { setTimeout(() => pageRecv({ __htd: 1, type: 'stackAt', seq: m.seq, items: [{ id: 'spbSave', cls: 'TSpeedButton' }, { id: 'Panel2', cls: 'TPanel' }, { id: '@form', cls: 'TfHotPlate' }] }), 10); return Promise.resolve(true); }
    return shPM(m);
  };
  const shQp = fake._state.quickPick;
  fake._state.quickPick = items => items[1];
  pagePosted.length = 0;
  const shR = await fake._cmds['ht9045Designer.selectHere']();
  fake._state.quickPick = shQp;
  panel.webview.postMessage = shPM;
  const shSel = pagePosted.filter(m => m.type === 'selectId').pop();
  ok(!!(shR && shR.id === 'Panel2' && JSON.stringify(shR.of) === '["spbSave","Panel2","@form"]' && shSel && shSel.id === 'Panel2'),
    'select here (Blend Set Current Selection): the components under the right-click listed top first, the one picked selected', JSON.stringify(shR));

  // the lock: Panel2 locked -> spbSave (inside it) cannot be deleted / cut / reordered; the tree and the probe know
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.designLock']({ id: 'Panel2', isForm: false });
  const lkMsg = pagePosted.filter(m => m.type === 'designLocked').pop();
  const lkNode = Array.from(fake._tree.byKey.values()).find(n => n.id === 'Panel2');
  const lkItem = lkNode ? fake._tree.getTreeItem(lkNode) : null;
  ok(lkMsg && JSON.stringify(lkMsg.ids) === '["Panel2"]' && lkItem && lkItem.contextValue === 'component.locked' && /🔒/.test(lkItem.description) &&
    JSON.stringify(api.hub.loadDesignSet('htd.designLocked', PAGE)) === '["Panel2"]' && api.hub.lockedName(edD, 'spbSave') === 'Panel2',
    'lock Panel2: probe told, tree marks it, kept for the page; spbSave inside it counts as locked', lkItem ? lkItem.description : '-');
  await selAs('spbSave', 'button');
  const lkText = text;
  const lkDel = await fake._cmds['ht9045Designer.deleteComponent']({ confirmed: true });
  const lkCut = await fake._cmds['ht9045Designer.cutComponent']({ confirmed: true });
  const lkOrd = await fake._cmds['ht9045Designer.order.front']();
  ok(lkDel === null && lkCut === null && lkOrd === null && text === lkText, 'locked: delete / cut / order refused, the page untouched');
  await fake._cmds['ht9045Designer.designUnlockAll']();
  ok(api.hub.loadDesignSet('htd.designLocked', PAGE).length === 0 && api.hub.lockedName(edD, 'spbSave') === '' && fake._tree.getTreeItem(lkNode).contextValue === 'component',
    'unlock all: nothing locked, nothing kept');
  // AI(W906-HTDESIGNER) 20261001: 全部鎖定 (WinForms / BCB6 Format > Lock Controls), the pair of 全部解除鎖定: what is right
  // under the form, so all inside it too; the form itself never
  const laR = await fake._cmds['ht9045Designer.designLockAll']();
  const laSpb = api.hub.lockedName(edD, 'spbSave'), laForm = api.hub.lockedName(edD, '@form');
  await fake._cmds['ht9045Designer.designUnlockAll']();
  ok(!!(laR && laR.locked.length >= 1 && laR.locked.indexOf('@form') < 0 && laSpb && api.hub.lockedName(edD, 'spbSave') === ''),
    'lock all: every component right under the form (spbSave inside one is locked too), not the form; unlock all frees them', JSON.stringify({ locked: laR && laR.locked, spb: laSpb }));
  // WPF Document Outline keys: Ctrl+H hide / Shift+Ctrl+H show / Ctrl+L lock / Shift+Ctrl+L unlock -- the design surface's
  // selection (spbSave + sbtExit), or the tree's selected rows (Panel2 + sbtExit); bound for both, the surface's not in the tree
  // (F2 too: in the tree the tree's rename wins)
  await selAs('spbSave', 'button');
  const dkInfo = edD.sel.info, dkMulti0 = dkInfo.multi;
  dkInfo.multi = ['sbtExit'];
  const dkH = await fake._cmds['ht9045Designer.designKey']({ cmd: 'hide' });
  const dkHid = Array.from(edD.designHidden).sort().join(',');
  const dkS = await fake._cmds['ht9045Designer.designKey']({ cmd: 'show' });
  const dkHid2 = Array.from(edD.designHidden).length;
  dkInfo.multi = dkMulti0;
  const dkTv0 = api.hub.treeView.selection;
  api.hub.treeView.selection = ['Panel2', 'sbtExit'].map(id => Array.from(fake._tree.byKey.values()).find(n => n.id === id));
  const dkL = await fake._cmds['ht9045Designer.designKey']({ cmd: 'lock', from: 'tree' });
  const dkLk = Array.from(edD.designLocked).sort().join(',');
  const dkU = await fake._cmds['ht9045Designer.designKey']({ cmd: 'unlock', from: 'tree' });
  const dkLk2 = Array.from(edD.designLocked).length;
  api.hub.treeView.selection = dkTv0;
  const dkKbs = require('../package.json').contributes.keybindings;
  const dkKb = (key, tree) => dkKbs.filter(k => k.key === key && k.command === 'ht9045Designer.designKey' && ((k.args || {}).from === 'tree') === tree);
  const dkKbOk = ['ctrl+h', 'ctrl+shift+h', 'ctrl+l', 'ctrl+shift+l'].every(k => dkKb(k, false).length === 1 && dkKb(k, true).length === 1 &&
    /focusedView != ht9045Designer\.components/.test(dkKb(k, false)[0].when) && /focusedView == ht9045Designer\.components/.test(dkKb(k, true)[0].when));
  const dkF2 = dkKbs.find(k => k.key === 'f2' && k.command === 'ht9045Designer.editText');
  ok(!!(dkH && dkHid === 'sbtExit,spbSave' && dkS && dkHid2 === 0 && dkL && dkLk === 'Panel2,sbtExit' && dkU && dkLk2 === 0 && dkKbOk &&
    dkF2 && /focusedView != ht9045Designer\.components/.test(dkF2.when)),
    'Document Outline keys (WPF): Ctrl+H / Shift+Ctrl+H hide and show spbSave + sbtExit; Ctrl+L / Shift+Ctrl+L on the tree rows lock and unlock Panel2 + sbtExit; the surface\'s keys (and F2) not in the tree',
    JSON.stringify({ dkHid, dkLk, dkKbOk }));
  // WPF: F2 on the design surface = edit the control's text there -- the page is asked; its text box open = the designer's keys
  // stay out (every surface rule), Ctrl+Z / Ctrl+Y = the box's own undo, not the page's
  await selAs('spbSave', 'button');
  pagePosted.length = 0;
  const etCalls = fake._calls.length;
  const et = await fake._cmds['ht9045Designer.editText']();
  const etAsked = pagePosted.some(m => m.type === 'editText');
  edD.onMessage({ __htd: 1, type: 'textEdit', open: true });
  const etCtx = fake._calls.slice(etCalls).some(c => c.name === 'exec' && c.args[0] === 'setContext' && c.args[1] === 'ht9045Designer.textEditing' && String(c.args[2]) === 'true');
  pagePosted.length = 0;
  const etUndo = await fake._cmds['ht9045Designer.textUndo']({ redo: true });
  const etUndoMsg = pagePosted.filter(m => m.type === 'textUndo').pop();
  edD.onMessage({ __htd: 1, type: 'textEdit', open: false });
  pagePosted.length = 0;
  const etUndo2 = await fake._cmds['ht9045Designer.textUndo']();
  const etKbs = require('../package.json').contributes.keybindings;
  const etSurface = etKbs.filter(k => /activeCustomEditorId == 'ht9045Designer\.editor'/.test(k.when || '') && !/ht9045Designer\.textEditing &&/.test(k.when || '') && k.command !== 'ht9045Designer.textUndo');
  const etUnguarded = etSurface.filter(k => !/!ht9045Designer\.textEditing/.test(k.when) && !/^(f7|f4|ctrl\+alt\+o|ctrl\+alt\+shift\+o|ctrl\+alt\+x|ctrl\+alt\+t|shift\+f7)$/.test(k.key));
  const etZ = etKbs.find(k => k.key === 'ctrl+z' && k.command === 'ht9045Designer.textUndo');
  ok(!!(et && et.asked === 'spbSave' && etAsked && etCtx && edD.textEditing === false && etUndo === true && etUndoMsg && etUndoMsg.redo === true &&
    etUndo2 === false && !pagePosted.some(m => m.type === 'textUndo') && etUnguarded.length === 0 && etZ && /^ht9045Designer\.textEditing/.test(etZ.when) &&
    etKbs.some(k => k.key === 'f2' && k.command === 'ht9045Designer.editText')),
    'F2 on the surface (WPF Edit control text): the page asked; its text box open -> the context set, the surface keys out, Ctrl+Z / Ctrl+Y its own undo',
    JSON.stringify({ et, unguarded: etUnguarded.map(k => k.key) }));

  // the grid's HTML rows: the source's own style / attributes; a change goes to the probe (checked first)
  const hmS = full.html && full.html.srcStyle, hmA = full.html && full.html.srcAttrs;
  ok(Array.isArray(hmS) && hmS.some(s => s[0] === 'left' && s[1] === '54px') && Array.isArray(hmA) && hmA.some(a => a[0] === 'id' && a[2] === 0) &&
    hmA.some(a => a[0] === 'title' && a[2] === 1) && !hmA.some(a => a[0] === 'style'),
    'panel data: style as the source writes it (left 54px), attributes with id not editable, title editable', JSON.stringify(hmS && hmS.slice(0, 3)));
  pagePosted.length = 0;
  propsRecv({ type: 'setStyleProp', name: 'z-index', value: '5' });
  propsRecv({ type: 'setStyleProp', name: 'color;x', value: 'red' });
  propsRecv({ type: 'setStyleProp', name: 'color', value: 'red;background:url(x)' });
  propsRecv({ type: 'setAttrProp', name: 'title', value: 'Save it' });
  propsRecv({ type: 'setAttrProp', name: 'onclick', value: 'alert(1)' });
  propsRecv({ type: 'setAttrProp', name: 'id', value: 'other' });
  const hmSent = pagePosted.filter(m => /^set(Style|Attr)Raw$/.test(m.type)).map(m => m.type + ':' + m.name + '=' + m.value);
  ok(hmSent.join(' | ') === 'setStyleRaw:z-index=5 | setAttrRaw:title=Save it', 'only a clean style / a text attribute reaches the probe (no ";", no on…, no id)', hmSent.join(' | '));

  // select all / undo / redo from the menus and the title bar
  pagePosted.length = 0;
  const uxCalls = fake._calls.length;
  await fake._cmds['ht9045Designer.selectAll']();
  await fake._cmds['ht9045Designer.undo']();
  await fake._cmds['ht9045Designer.redo']();
  const uxExec = fake._calls.slice(uxCalls).filter(c => c.name === 'exec').map(c => c.args[0]);
  ok(pagePosted.some(m => m.type === 'selectAll') && uxExec.indexOf('undo') >= 0 && uxExec.indexOf('redo') > uxExec.indexOf('undo'),
    'select all goes to the probe; the title bar\'s undo / redo run VS Code\'s undo / redo', uxExec.join(','));
  await fake._cmds['ht9045Designer.zoomFit']();
  ok(pagePosted.some(m => m.type === 'zoomFit'), 'zoom to fit goes to the probe');
  await fake._cmds['ht9045Designer.zoomSelection']();
  ok(pagePosted.some(m => m.type === 'zoomSel'), 'zoom to selection goes to the probe');
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.selectSameType']();
  await fake._cmds['ht9045Designer.selectSameTypeHere']();
  ok(pagePosted.filter(m => m.type === 'selectSameType').map(m => m.scope).join(',') === 'page,container', 'select same type (page / here) goes to the probe');
  // 同大小 / 等距 / 容器中置中: commands and the 對齊與間距 menu, each to the probe as an align
  const arrHows = ['size', 'hspace', 'vspace', 'hcenterIn', 'vcenterIn'];
  pagePosted.length = 0;
  for (const h of arrHows) await fake._cmds['ht9045Designer.align.' + h]();
  const arrPkg = JSON.parse(require('fs').readFileSync(require('path').join(__dirname, '..', 'package.json'), 'utf8'));
  const arrMenu = (arrPkg.contributes.menus['ht9045Designer.alignMenu'] || []).map(x => x.command);
  ok(arrHows.every(h => pagePosted.some(m => m.type === 'align' && m.how === h) && arrMenu.includes('ht9045Designer.align.' + h)),
    'same size / equal spacing / centre in container: in the menu, each goes to the probe', pagePosted.map(m => m.how).join(','));
  // the right-click menu: the view toggles under 顯示, the page checks under 檢查與比對; every
  // command any menu names is declared and registered (a menu item that does nothing is a bug)
  const mnMenus = arrPkg.contributes.menus;
  // (the designer's menu: 0.148's 方案總管 rows have their own, webviewId == 'ht9045Designer.solution')
  const mnTop = mnMenus['webview/context'].filter(x => !/webviewId == 'ht9045Designer\.solution'/.test(x.when || '')).map(x => x.command || x.submenu);
  const mnShow = (mnMenus['ht9045Designer.showMenu'] || []).map(x => x.command);
  const mnCheck = (mnMenus['ht9045Designer.checkMenu'] || []).map(x => x.command);
  const mnDeclared = new Set(arrPkg.contributes.commands.map(c => c.command));
  const mnAll = [].concat(...Object.values(mnMenus).map(list => list.map(x => x.command).filter(Boolean)));
  const mnBad = mnAll.filter(c => /^ht9045Designer\./.test(c) && !/\.focus$/.test(c) && (!mnDeclared.has(c) || typeof fake._cmds[c] !== 'function'));
  ok(['toggleNames', 'toggleWireMarks', 'toggleDfmGhosts', 'toggleTabOrder', 'toggleGrid', 'zoomFit'].every(c => mnShow.includes('ht9045Designer.' + c) && !mnTop.includes('ht9045Designer.' + c)) &&
    ['pageOverview', 'dfmDiff', 'lintPage', 'lintFixAll'].every(c => mnCheck.includes('ht9045Designer.' + c) && !mnTop.includes('ht9045Designer.' + c)) &&
    mnTop.includes('ht9045Designer.showMenu') && mnTop.includes('ht9045Designer.checkMenu') && mnTop.length <= 27 && mnBad.length === 0,   // (27: + 選取這裡的元件…, top level as Blend's Set Current Selection, 20261001)
    'right-click menu: 顯示 / 檢查與比對 submenus, ' + mnTop.length + ' top-level items (33 before); every menu command declared and registered', mnBad.join(','));
  // the panel with several selected: a change is asked for all of them (all: true); one selected: not
  const mlInfo = multi => ({ key: 960, id: 'spbSave', multi, tag: 'button', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] });
  const mlPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    // the two selected: left and bold differ, the rest the same
    if (m.type === 'lookAll') setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: m.ids.map(id => ({ id,
      lay: { left: id === 'spbSave' ? 54 : 332, top: 8, width: 227, height: 40 }, cap: { value: 'X' }, look: { bold: id === 'spbSave', fontSize: 14 } })) }), 5);
    return mlPM(m);
  };
  pageRecv({ __htd: 1, type: 'select', info: mlInfo(['sbtExit']), origin: 'multi' });
  const smWait = async (fn, ms) => { const t0 = Date.now(); while (Date.now() - t0 < ms) { const v = fn(); if (v) return v; await new Promise(r => setTimeout(r, 20)); } return null; };
  const mlMx = await smWait(() => (api.hub.props.data && api.hub.props.data.edit && api.hub.props.data.edit.mixed ? api.hub.props.data.edit.mixed : null), 3000);
  panel.webview.postMessage = mlPM;
  ok(!!(mlMx && mlMx.left && mlMx.bold && !mlMx.top && !mlMx.caption), 'several selected: the panel data says which fields differ (left, bold), asked of the page', JSON.stringify(mlMx));
  pagePosted.length = 0;
  propsRecv({ type: 'setLook', prop: 'bold', value: false });
  propsRecv({ type: 'setLayout', width: 120 });
  const mlAll = pagePosted.filter(m => /^set(Look|Layout)$/.test(m.type));
  pageRecv({ __htd: 1, type: 'select', info: mlInfo(undefined), origin: 'click' });
  await new Promise(r => setTimeout(r, 30));
  pagePosted.length = 0;
  propsRecv({ type: 'setLook', prop: 'bold', value: false });
  const mlOne = pagePosted.filter(m => m.type === 'setLook');
  ok(mlAll.length === 2 && mlAll.every(m => m.all === true && m.key === 960) && mlOne.length === 1 && mlOne[0].all === false,
    'properties panel: with 2 selected a change goes out for all of them; with one, only that one', JSON.stringify(mlAll.concat(mlOne).map(m => m.type + ':' + m.all)));
  if (R.selInfo) { pageRecv({ __htd: 1, type: 'select', info: R.selInfo, origin: 'click' }); await new Promise(r => setTimeout(r, 30)); }

  // 操作一覽: the cheat sheet opens as a markdown preview (and ships with the extension)
  const csCalls = fake._calls.length;
  await fake._cmds['ht9045Designer.cheatsheet']();
  const csExec = fake._calls.slice(csCalls).find(c => c.name === 'exec' && c.args[0] === 'markdown.showPreview');
  const csText = fs.readFileSync(path.join(__dirname, '..', 'CHEATSHEET.md'), 'utf8');
  const csPack = fs.readFileSync(path.join(__dirname, '..', 'pack.ps1'), 'utf8');
  ok(csExec && /CHEATSHEET\.md$/.test(csExec.args[1]) && /Ctrl\+Alt\+Shift\+O/.test(csText) && /'CHEATSHEET\.md'/.test(csPack),
    'the cheat sheet: opens as a preview, lists the keys, is packed');
  // 更新紀錄: packed, and its newest entry is this version (a round that forgets it fails here)
  const clText = fs.readFileSync(path.join(__dirname, '..', 'CHANGELOG.md'), 'utf8');
  const clTop = (/^## (\d+\.\d+\.\d+)/m.exec(clText) || [])[1];
  ok(/'CHANGELOG\.md'/.test(csPack) && clTop === arrPkg.version, 'CHANGELOG.md is packed and starts with this version (' + arrPkg.version + ')', 'top entry ' + clTop);

  // the page switcher: the status bar names this page and opens the picker; the picker offers the side-bar
  // list first (it can be hidden) and marks this page; picking that first item focuses the 頁面 view
  const swItem = api.hub.pageStatus;
  ok(api.hub.pageView.description === 'v' + ctx.extension.packageJSON.version, 'the 頁面 view shows the version this window runs', api.hub.pageView.description);
  // 有新版: a newer install in the extensions folder -> the status bar button (reload), the title, one message;
  // a copy run from the source tree (no extensions.json next to it) finds nothing
  const upVer = ctx.extension.packageJSON.version;
  const upSrc = api.hub.checkUpdate();
  const upDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-upd-'));
  fs.writeFileSync(path.join(upDir, 'extensions.json'), JSON.stringify([
    { identifier: { id: 'HT9045.ht9045-html-designer' }, version: '0.9.0' },
    { identifier: { id: 'ht9045.ht9045-html-designer' }, version: '99.1.0' },
    { identifier: { id: 'someone.other' }, version: '100.0.0' }]));
  const upCalls = fake._calls.length;
  const upNew = api.hub.checkUpdate(upDir);
  const upItem = api.hub.updateItem;
  const upInfo = fake._calls.slice(upCalls).filter(c => c.name === 'info' && /新版 v99\.1\.0/.test(c.args[0]));
  const upCtx = fake._calls.slice(upCalls).find(c => c.name === 'exec' && c.args[0] === 'setContext' && c.args[1] === 'ht9045Designer.updateReady');
  api.hub.checkUpdate(upDir);
  const upInfo2 = fake._calls.slice(upCalls).filter(c => c.name === 'info').length;
  ok(upSrc === null && upNew === '99.1.0' && upItem.shown && upItem.command === 'ht9045Designer.reloadWindow' && /v99\.1\.0/.test(upItem.text) &&
    /新版 v99\.1\.0/.test(api.hub.pageView.description) && upInfo.length === 1 && upInfo2 === 1 && upCtx && upCtx.args[2] === 'true',
    'a newer version installed: the status bar button (reload), the 頁面 title, one message; none from the source tree', upItem.text + ' / ' + api.hub.pageView.description);
  const rlCalls = fake._calls.length;
  const rlOk = await fake._cmds['ht9045Designer.reloadWindow']();
  const rlExec = fake._calls.slice(rlCalls).some(c => c.name === 'exec' && c.args[0] === 'workbench.action.reloadWindow');
  // files.hotExit off + an unsaved file: asked first; closing the question reloads nothing
  fake._config.hotExit = 'off';
  fake._docs = fake._docs || {};
  fake._docs['file:///rl-dirty.html'] = { isDirty: true, fileName: 'C:\\x\\rl-dirty.html', getText: () => '', offsetAt: () => 0, _setText() {} };
  fake._answer = undefined;
  const rl2Calls = fake._calls.length;
  const rlNo = await fake._cmds['ht9045Designer.reloadWindow']();
  const rlAsked = fake._calls.slice(rl2Calls).some(c => c.name === 'warning' && /個檔案還沒存/.test(c.args[0]));
  const rlNoExec = !fake._calls.slice(rl2Calls).some(c => c.name === 'exec' && c.args[0] === 'workbench.action.reloadWindow');
  delete fake._docs['file:///rl-dirty.html'];
  delete fake._config.hotExit;
  fs.writeFileSync(path.join(upDir, 'extensions.json'), JSON.stringify([{ identifier: { id: 'ht9045.ht9045-html-designer' }, version: upVer }]));
  const upGone = api.hub.checkUpdate(upDir);
  fs.rmSync(upDir, { recursive: true, force: true });
  ok(rlOk === true && rlExec && rlNo === false && rlAsked && rlNoExec && upGone === null && !upItem.shown && api.hub.pageView.description === 'v' + upVer,
    'the reload button: reloads the window; asks first when an unsaved file would be lost (hotExit off); the button goes when nothing is newer',
    JSON.stringify({ rlOk, rlExec, rlNo, rlAsked, rlNoExec, upGone, shown: upItem.shown, desc: api.hub.pageView.description }));

  // 搜尋頁面: the box above the 頁面 list; what it types filters the list (pages, and their components under them)
  const sbView = (fake._webviewViews || {})['ht9045Designer.pageSearch'];
  const sbPosted = [];
  let sbRecv = null;
  if (sbView) sbView.resolveWebviewView({
    webview: {
      cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
      asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
      postMessage: m => { sbPosted.push(m); return Promise.resolve(true); },
      onDidReceiveMessage: fn => { sbRecv = fn; return { dispose() {} }; },
    },
    onDidDispose: () => ({ dispose() {} }),
  });
  const sbPkg = arrPkg.contributes.views['ht9045-designer'];
  // (right above 頁面, wherever that is: 0.138 put 方案總管 first, as Visual Studio's Solution Explorer)
  const sbAt = sbPkg.findIndex(v => v.id === 'ht9045Designer.pageSearch');
  ok(!!(sbView && sbRecv && sbAt >= 0 && sbPkg[sbAt].type === 'webview' && sbPkg[sbAt + 1] && sbPkg[sbAt + 1].id === 'ht9045Designer.pages' &&
    fs.existsSync(path.join(__dirname, '..', 'media', 'search.js')) && fs.existsSync(path.join(__dirname, '..', 'media', 'search.css'))),
    'the search box: a view right above 頁面 (its page and style ship)');
  const pt = api.hub.pages;
  // a tab label (no id): the page is listed by the text it shows
  sbRecv({ type: 'query', q: 'Above9050' });
  const sbTabHw = pt.getChildren().find(n => n.name === 'HW');
  const sbTabIo = sbTabHw && pt.getChildren(sbTabHw).find(n => n.name === 'HW.IoSetView.html');
  const sbTabItem = sbTabIo && pt.getTreeItem(sbTabIo);
  ok(!!(sbTabItem && /頁面上的文字/.test(sbTabItem.description) && sbTabItem.collapsibleState === 0),
    'typing "Above9050" (a tab label): HW / IoSetView listed, "頁面上的文字"', sbTabItem && sbTabItem.description);
  sbRecv({ type: 'query', q: 'grpLoader_9050' });
  const sbRes = sbPosted.filter(m => m.type === 'result').pop();
  const sbRoots = pt.getChildren();
  const sbHw = sbRoots.find(n => n.name === 'HW');
  const sbIo = sbHw && pt.getChildren(sbHw).find(n => n.name === 'HW.IoSetView.html');
  const sbIoItem = sbIo && pt.getTreeItem(sbIo);
  const sbHit = sbIo && pt.getChildren(sbIo)[0];
  const sbHitItem = sbHit && pt.getTreeItem(sbHit);
  const sbGrpItem = sbHw && pt.getTreeItem(sbHw);
  ok(!!(sbRes && sbRes.pages >= 1 && /grpLoader_9050/.test(sbRes.text) && sbIoItem && sbIoItem.collapsibleState === 2 && /個元件/.test(sbIoItem.description) &&
    sbGrpItem.collapsibleState === 2 && sbHitItem && sbHitItem.command.command === 'ht9045Designer.openPageAt' && sbHitItem.command.arguments[0] === sbIo.file &&
    sbHitItem.command.arguments[1] === 'grpLoader_9050' && JSON.stringify(sbHitItem.label.highlights) === '[[0,14]]' && /grpLoader_9050/.test(api.hub.pageView.message)),
    'typing "grpLoader_9050": the list shows HW / IoSetView open, the component under it highlighted (a click opens it there); the count above the list',
    (sbRes && sbRes.text) + ' / ' + (sbHitItem && sbHitItem.label.label));
  sbRecv({ type: 'query', q: 'zz-no-such-thing-qq' });
  const sbNone = sbPosted.filter(m => m.type === 'result').pop();
  const sbNoneRoots = pt.getChildren();
  const sbNoneMsg = api.hub.pageView.message;
  const sbSet = await fake._cmds['ht9045Designer.filterPages']('hotplate');
  const sbSetMsg = sbPosted.filter(m => m.type === 'set').pop();
  const sbHp = [].concat(...pt.getChildren().map(g => pt.getChildren(g))).find(p => p.name === 'Setup.HotPlate.html');
  const sbHpItem = sbHp && pt.getTreeItem(sbHp);
  ok(!!(sbNone && sbNone.none && sbNoneRoots.length === 0 && /找不到/.test(sbNoneMsg) && sbSet && sbSet.pages >= 1 && sbSetMsg && sbSetMsg.q === 'hotplate' &&
    sbHpItem && sbHpItem.label.highlights.length === 1 && /● 目前/.test(sbHpItem.description)),
    'nothing found: an empty list and 找不到; the command puts its words in the box; a page found by its name is highlighted and still marked 目前',
    sbHpItem && JSON.stringify(sbHpItem.label));
  const sbCalls = fake._calls.length;
  sbRecv({ type: 'query', q: 'Setup.Speed' });
  await sbRecv({ type: 'open' });
  const sbOpen = fake._calls.slice(sbCalls).find(c => c.name === 'exec' && c.args[0] === 'vscode.openWith');
  await fake._cmds['ht9045Designer.clearPageFilter']();
  const sbAll = pt.getChildren();
  const sbCtx = fake._calls.slice(sbCalls).filter(c => c.name === 'exec' && c.args[0] === 'setContext' && c.args[1] === 'ht9045Designer.pageFilterOn').pop();
  ok(!!(sbOpen && /Setup\.Speed\.html/.test(sbOpen.args[1]) && sbAll.length > 3 && sbAll.some(g => g.name === 'Main') && !api.hub.pageView.message && sbCtx && sbCtx.args[2] === 'false'),
    'Enter opens the first page found; clearing shows every page again', sbOpen && sbOpen.args[1]);

  // 設計時全部顯示: a component hidden when the page runs (tree row [8]) says so in the tree
  const rhNodes = R.treeNodes.map(r => r.slice());
  const rhRow = rhNodes.find(r => r[2] === 'spbSave');
  if (rhRow) rhRow[8] = 1;
  pageRecv({ __htd: 1, type: 'tree', nodes: rhNodes });
  await new Promise(r => setTimeout(r, 50));
  const rhNode = Array.from(fake._tree.byKey.values()).find(n => n.id === 'spbSave');
  const rhItem = rhNode && fake._tree.getTreeItem(rhNode);
  pageRecv({ __htd: 1, type: 'tree', nodes: R.treeNodes });
  await new Promise(r => setTimeout(r, 50));
  const rhBack = Array.from(fake._tree.byKey.values()).find(n => n.id === 'spbSave');
  ok(!!(rhItem && /執行時隱藏/.test(rhItem.description) && /設計時照樣顯示/.test(rhItem.tooltip) && rhBack && !/執行時隱藏/.test(fake._tree.getTreeItem(rhBack).description)),
    'a component hidden when the page runs: the tree says 執行時隱藏 (it shows while designing)', rhItem && rhItem.description);

  // CSV 表格: a .csv as a table; each change = the text of only the cells it changes, one edit each
  const csvProv = (fake._editors || {})['ht9045Designer.csv'];
  const csvPkg = arrPkg.contributes.customEditors.find(e => e.viewType === 'ht9045Designer.csv');
  const csvDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-csv-'));
  const mkCsv = (name, bytes, text) => {
    const f = path.join(csvDir, name);
    fs.writeFileSync(f, bytes);
    let t = text, ls = [0];
    const idx = () => { ls = [0]; for (let i = 0; i < t.length; i++) if (t.charCodeAt(i) === 10) ls.push(i + 1); };
    idx();
    const d = {
      uri: fake.Uri.file(f), fileName: f, isDirty: false, getText: () => t,
      positionAt: off => { let lo = 0, hi = ls.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (ls[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - ls[lo]); },
      offsetAt: p => ls[p.line] + p.character, _setText: x => { t = x; idx(); },
    };
    fake._docs[d.uri.toString()] = d;
    const posted = [];
    const box = { recv: null, posted, doc: d };
    const pnl = {
      webview: {
        cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
        asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
        postMessage: m => { posted.push(m); return Promise.resolve(true); },
        onDidReceiveMessage: fn => { box.recv = fn; return { dispose() {} }; },
      },
      onDidDispose: () => ({ dispose() {} }), onDidChangeViewState: () => ({ dispose() {} }),
    };
    csvProv.resolveCustomTextEditor(d, pnl);
    box.html = pnl.webview.html;
    return box;
  };
  const csv0 = 'Name,Acc,\r\nM00,100000,\r\nM01,"a,b",\r\n';
  const cb = csvProv ? mkCsv('Mot.csv', csv0, csv0) : null;
  if (cb) await cb.recv({ type: 'ready' });
  const cbData = cb && cb.posted.filter(m => m.type === 'data').pop();
  ok(!!(csvProv && csvPkg && csvPkg.priority === 'default' && cbData && cbData.rows.length === 3 && cbData.rows[2][1] === 'a,b' && !cbData.readOnly &&
    /csv\.js/.test(cb.html) && /Content-Security-Policy/.test(cb.html) && /style-src [^;"]*'unsafe-inline'/.test(cb.html)),
    'CSV 表格: .csv opens as a table (the default), the rows go to it', cbData && JSON.stringify(cbData.rows));
  const cbCalls = fake._calls.length;
  await cb.recv({ type: 'edit', changes: [{ r: 1, c: 1, v: '123' }] });
  const cbAfter1 = cb.doc.getText();
  await cb.recv({ type: 'edit', changes: [{ r: 2, c: 0, v: 'M02' }] });   // right after: the new positions
  const cbAfter2 = cb.doc.getText();
  await cb.recv({ type: 'paste', text: 'x\ty\r\nz\tw', r: 1, c: 0 });
  const cbAfter3 = cb.doc.getText();
  const cbSel = cb.posted.filter(m => m.type === 'select').pop();
  const cbEditCalls = fake._calls.slice(cbCalls).filter(c => c.name === 'applyEditCall').length;
  ok(cbAfter1 === csv0.replace('100000', '123') && cbAfter2 === cbAfter1.replace('M01', 'M02') && cbAfter3 === 'Name,Acc,\r\nx,y,\r\nz,"w",\r\n' &&
    cbEditCalls === 3 && cbSel && cbSel.r === 1 && cbSel.r1 === 2 && cbSel.c1 === 1,
    'CSV 表格: an edit = only that cell\'s text; a second one right after lands right; a paste (Excel) = one edit, the block selected', JSON.stringify(cbAfter3));
  await cb.recv({ type: 'insertRows', r: 1, count: 1 });
  const cbIns = cb.doc.getText();
  await cb.recv({ type: 'deleteRows', r: 1, count: 1 });
  const cbDel = cb.doc.getText();
  const cbClip0 = fake._calls.length;
  let cbClip = null;
  const clipW = fake.env.clipboard.writeText;
  fake.env.clipboard.writeText = s => { cbClip = s; return Promise.resolve(); };
  await cb.recv({ type: 'copy', grid: [['a', 'b'], ['1', '2']] });
  fake.env.clipboard.writeText = clipW;
  ok(cbIns === 'Name,Acc,\r\n,,\r\nx,y,\r\nz,"w",\r\n' && cbDel === cbAfter3 && cbClip === 'a\tb\r\n1\t2' && cbClip0 >= 0,
    'CSV 表格: insert / delete a row (the file\'s line end), copy = tab separated to the clipboard', JSON.stringify(cbIns));
  // AI(W906-HTDESIGNER) 20261001 (Excel): one copied cell pasted on a selected column = every cell of it, ONE edit;
  // a block that fits a whole number of times = repeated; a cut block = moved (its old cells emptied) in the same edit;
  // a cell being edited = VS Code's Ctrl+Z held off (the context the keybinding reads)
  const tl0 = 'Name,Acc,Dec\r\nM00,1,2\r\nM01,3,4\r\nM02,5,6\r\n';
  const tl = mkCsv('Tile.csv', tl0, tl0);
  await tl.recv({ type: 'ready' });
  const tlCalls = fake._calls.length;
  await tl.recv({ type: 'paste', text: '9\r\n', r: 1, c: 1, r1: 3, c1: 1 });
  const tlCol = tl.doc.getText(), tlSel = tl.posted.filter(m => m.type === 'select').pop();
  await tl.recv({ type: 'paste', text: 'a\tb', r: 1, c: 1, r1: 2, c1: 2 });   // 1x2 into 2x2: repeated
  const tlRep = tl.doc.getText();
  await tl.recv({ type: 'paste', text: 'M00\ta', r: 3, c: 0, r1: 3, c1: 0, cut: { r0: 1, c0: 0, r1: 1, c1: 1 } });   // M00,a moved to row 3
  const tlCut = tl.doc.getText();
  const tlEdits = fake._calls.slice(tlCalls).filter(c => c.name === 'applyEditCall').length;
  const tlEx0 = fake._calls.length;
  await tl.recv({ type: 'editing', on: true });
  await tl.recv({ type: 'editing', on: false });
  const tlCtx = fake._calls.slice(tlEx0).filter(c => c.name === 'exec' && c.args[0] === 'setContext' && c.args[1] === 'ht9045Designer.csvEditing').map(c => c.args[2]);
  ok(tlCol === 'Name,Acc,Dec\r\nM00,9,2\r\nM01,9,4\r\nM02,9,6\r\n' && tlSel && tlSel.r === 1 && tlSel.r1 === 3 && tlSel.c1 === 1 &&
    tlRep === 'Name,Acc,Dec\r\nM00,a,b\r\nM01,a,b\r\nM02,9,6\r\n' && tlCut === 'Name,Acc,Dec\r\n,,b\r\nM01,a,b\r\nM00,a,6\r\n' && tlEdits === 3 &&
    JSON.stringify(tlCtx) === '["true","false"]',
    'CSV 表格 (Excel): one cell pasted on a column fills it; a 1x2 on a 2x2 repeats; a cut block moves (old cells emptied) -- one edit each; editing = csvEditing context on / off',
    JSON.stringify({ col: tlCol, rep: tlRep, cut: tlCut, edits: tlEdits, ctx: tlCtx }));
  // AI(W906-HTDESIGNER) 20261001: the table's right-click 貼上 -- the extension reads the clipboard and hands it back
  // (a webview cannot read it), nothing written by that alone
  await fake.env.clipboard.writeText('x\ty');
  const rcText0 = tl.doc.getText();
  await tl.recv({ type: 'readClip' });
  const rcMsg = tl.posted.filter(m => m.type === 'clip').pop();
  ok(!!(rcMsg && rcMsg.text === 'x\ty' && tl.doc.getText() === rcText0), 'CSV 表格: the right-click 貼上 asks for the clipboard -- the extension gives the text back, nothing written by that',
    JSON.stringify(rcMsg || null));
  // the keys VS Code has its own command for (Ctrl+G = Go to Line, Shift+F10) are the table's while it is active
  const csvKeys = (require(path.join(__dirname, '..', 'package.json')).contributes.keybindings || [])
    .filter(k => k.command === 'ht9045Designer.csvKey' && /activeCustomEditorId == 'ht9045Designer\.csv'/.test(k.when || '')).map(k => k.key);
  ok(['ctrl+g', 'shift+f10', 'ctrl+h', 'ctrl+d', 'ctrl+r'].every(k => csvKeys.includes(k)) && !csvKeys.includes('f5'),
    'CSV 表格: Ctrl+G (the Name Box) and Shift+F10 (the menu) are the table\'s, not VS Code\'s Go to Line; F5 is left to VS Code (it would start a debug run)', csvKeys.join(','));
  // a Big5 file VS Code read as UTF-8: shown, never edited (saving would destroy those bytes)
  const cbBad = mkCsv('Lang.csv', Buffer.from([0x41, 0x2c, 0xa4, 0xa4, 0x0d, 0x0a]), 'A,��\r\n');
  await cbBad.recv({ type: 'ready' });
  const cbBadData = cbBad.posted.filter(m => m.type === 'data').pop();
  const cbBadCalls = fake._calls.length;
  await cbBad.recv({ type: 'edit', changes: [{ r: 0, c: 0, v: 'B' }] });
  const cbBadEdit = fake._calls.slice(cbBadCalls).some(c => c.name === 'applyEditCall');
  const cbBadWarn = fake._calls.slice(cbBadCalls).some(c => c.name === 'warning' && /Big5/.test(c.args[0]));
  fs.rmSync(csvDir, { recursive: true, force: true });
  ok(!!(cbBadData && cbBadData.readOnly && /Big5/.test(cbBadData.why) && !cbBadEdit && cbBadWarn && cbBad.doc.getText() === 'A,��\r\n'),
    'CSV 表格: a Big5 file read as UTF-8 is read only (it says how to reopen it), an edit is refused', cbBadData && cbBadData.why.slice(0, 40));

  // 事件表: every event of the class; a double-click on an empty one adds the handler to the C++ port
  // (the fake's documents are in memory: nothing is written)
  if (R.selInfo) pageRecv({ __htd: 1, type: 'select', info: R.selInfo, origin: 'click' });
  let egData = null;
  for (let i = 0; i < 150 && !egData; i++) {
    const pd = api.hub.props.data;
    if (pd && pd.comp && pd.comp.htmlId === 'spbSave' && pd.eventGrid) egData = pd;
    else await new Promise(r => setTimeout(r, 50));
  }
  const egG = egData && egData.eventGrid;
  const egNames = egG ? egG.rows.map(r => r.name).join(',') : '';
  const egClickRow = egG && egG.rows.find(r => r.name === 'OnClick');
  const egUp = egG && egG.rows.find(r => r.name === 'OnMouseUp');
  ok(!!(egG && egG.vcl === 'TSpeedButton' && egG.formCls === 'TfHotPlate' && egNames === 'OnClick,OnDblClick,OnMouseDown,OnMouseMove,OnMouseUp' &&
    egClickRow && egClickRow.handler === 'spbSaveClick' && egClickRow.dfm && egClickRow.idx >= 0 && egUp && !egUp.handler && egUp.conv === 'spbSaveMouseUp'),
    '事件表: every TSpeedButton event (the Object Inspector\'s list), OnClick = spbSaveClick from the .dfm, OnMouseUp empty (would be spbSaveMouseUp)', egNames);
  const egCalls = fake._calls.length;
  const egPage = api.hub.active && api.hub.active.doc;
  const egPage0 = egPage ? egPage.getText() : '';
  const egPort = api.hub.active && api.hub.active.r.portRoot;
  const egSrvF = egPort ? path.join(egPort, 'tools', 'wb_serve.cpp') : '', egCmF = egPort ? path.join(egPort, 'CMakeLists.txt') : '';
  const egGenF = egPort ? path.join(egPort, 'HtdEvents', 'HtdEvents.gen.cpp') : '';
  const egSrv0 = egSrvF && fs.existsSync(egSrvF) ? fs.readFileSync(egSrvF, 'utf8') : '', egCm0 = egCmF && fs.existsSync(egCmF) ? fs.readFileSync(egCmF, 'utf8') : '';
  const egSrvB0 = egSrvF && fs.existsSync(egSrvF) ? fs.readFileSync(egSrvF) : Buffer.alloc(0), egCmB0 = egCmF && fs.existsSync(egCmF) ? fs.readFileSync(egCmF) : Buffer.alloc(0);
  // AI(W906-HTDESIGNER) 20261001: the build's part (generated table, CMake entry, server branch) goes to disk at once --
  // here into memory instead: nothing is ever written under the real tree by this test
  const egMem = new Map(), egKey = f => path.resolve(f).toLowerCase(), egRealDisk = api.hub.disk, egWrites = [];
  api.hub.disk = { read: f => (egMem.has(egKey(f)) ? egMem.get(egKey(f)) : egRealDisk.read(f)), write: (f, b) => { egWrites.push(path.basename(f)); egMem.set(egKey(f), Buffer.from(b)); } };
  const egMemT = (f, enc) => (f && egMem.has(egKey(f)) ? egMem.get(egKey(f)).toString(enc || 'utf8') : '');
  const egMade = egData ? await api.hub.onEventGrid(egData, 'OnMouseUp', api.hub.active) : null;
  const egH = egMade && fake._docs[fake.Uri.file(egMade.h).toString()];
  const egC = egMade && fake._docs[fake.Uri.file(egMade.cpp).toString()];
  const egHText = egH ? egH.getText() : '', egCText = egC ? egC.getText() : '';
  const egDisk = egMade ? fs.readFileSync(egMade.h, 'utf8') : '';
  const egOpened = fake._state.opened.slice(-1)[0];
  const egInfo = fake._calls.slice(egCalls).find(c => c.name === 'info' && /已新增 TfHotPlate::spbSaveMouseUp/.test(c.args[0]));
  const egRow = egData && egData.eventGrid.rows.find(r => r.name === 'OnMouseUp');
  ok(!!(egMade && egHText.length > egDisk.length &&
    /void spbSaveMouseUp\(TObject \*Sender, TMouseButton Button, TShiftState Shift, int X, int Y\);\s+\/\/AI\(W906-HTDESIGNER\)/.test(egHText) &&
    /void TfHotPlate::spbSaveMouseUp\(TObject \*Sender, TMouseButton Button, TShiftState Shift, int X, int Y\)\r?\n\{\r?\n\s+\(void\)Sender;/.test(egCText) &&
    egOpened && /fHotPlate\.cpp$/i.test(egOpened.uri.fsPath) && egInfo &&
    egRow && egRow.handler === 'spbSaveMouseUp' && egRow.portState === 'new' && egRow.main === egRow.port && egRow.mainName === 'spbSaveMouseUp' &&
    fs.readFileSync(egMade.h, 'utf8') === egDisk),
    '事件表: a double-click on the empty OnMouseUp adds TfHotPlate::spbSaveMouseUp (VCL signature) to fHotPlate.h / .cpp, opens the .cpp there; nothing written to disk',
    egInfo ? egInfo.args[0].slice(0, 90) : '-');
  // an event the server's form.event table runs: the function it really calls (HotPlate cbSelectHPFromDB.OnChange ->
  // &B_cbSelectHPFromDBChange, the golden handler's translation in FileRW\HotPlateForm_File.cpp)
  const egSrvRows = egPort ? api.hub.serverEvents(egPort) : [];
  const egHp = egSrvRows.find(r => r.cls === 'TfHotPlate' && r.ctl === 'cbSelectHPFromDB' && r.ev === 'change');
  const egHpText = egHp ? fs.readFileSync(egHp.file, 'utf8').split('\n') : [];
  ok(!!(egHp && egHp.fn === 'B_cbSelectHPFromDBChange' && /HotPlateForm_File\.cpp$/i.test(egHp.file) &&
    /^static void B_cbSelectHPFromDBChange\(FormState& J\)/.test(egHpText[egHp.fnLine - 1] || '') && egHp.fnLine !== egHp.line),
    '事件表: a form.event table row -> the function the server runs: cbSelectHPFromDB change -> B_cbSelectHPFromDBChange (its definition, not the forward declaration)',
    egHp ? path.basename(egHp.file) + ':' + egHp.fnLine + ' ' + egHp.fnSnippet : 'no row');
  // ... and wires it (自動接線): the page's htdCpp line (in memory, with the handler); the build's part STRAIGHT TO DISK
  // (here the in-memory one), the generated table first, the server's branch last, each byte of the server's and
  // CMake's other text unchanged; no untitled document; the real files untouched
  const egDoc = f => fake._docs[fake.Uri.file(f).toString()] || fake._docs[fake.Uri.file(f).with({ scheme: 'untitled' }).toString()];
  const egSrvT = egMemT(egSrvF, 'latin1'), egCmT = egMemT(egCmF, 'latin1');
  const egGenT = egMemT(egGenF);
  const egPageT = egPage ? egPage.getText() : '';
  // (an insert of the branch only: the bytes before and after it are the original ones)
  const egOnlyInsert = (b0, b1) => {
    if (!b1 || b1.length <= b0.length) return false;
    let i = 0;
    while (i < b0.length && b0[i] === b1[i]) i++;
    return Buffer.compare(b1.slice(i + b1.length - b0.length), b0.slice(i)) === 0;
  };
  const egSrvB1 = egMem.get(egKey(egSrvF)), egCmB1 = egMem.get(egKey(egCmF));
  ok(!!(egMade && egMade.wired && /htdCpp\('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveMouseUp', 'OnMouseUp'\);/.test(egPageT) && /function htdCpp\(/.test(egPageT) &&
    /\} else if \(wc\.cmd == "htd\.event"\) \{ \/\*AI\(W906-HTDESIGNER\) \d{8}: [\x20-\x7e]*\*\/[^\n]*W906_HtdEvent\(/.test(egSrvT) && egSrvT.split('\n').length === egSrv0.split('\n').length &&
    egOnlyInsert(egSrvB0, egSrvB1) && egOnlyInsert(egCmB0, egCmB1) &&
    /HtdEvents\/HtdEvents\.gen\.cpp\)/.test(egCmT) && egCmT.split('\n').length === egCm0.split('\n').length &&
    /^\/\/ htd-row TfHotPlate spbSaveMouseUp spbSave OnMouseUp\r?$/m.test(egGenT) &&
    /decltype\(\(void\)o->spbSaveMouseUp\(nullptr, HtdButton\(a\), HtdShift\(a\), a\.x, a\.y\), 1\)[^\n]*o->spbSaveMouseUp\(nullptr, HtdButton\(a\), HtdShift\(a\), a\.x, a\.y\); return 1; \}/.test(egGenT) &&
    /int E0\(const HtdIn& a\) \{ return C0\(fHotPlate, a, 0\); \}/.test(egGenT) && /no-handler: /.test(egGenT) &&
    JSON.stringify(egWrites) === JSON.stringify(['HtdEvents.gen.cpp', 'CMakeLists.txt', 'wb_serve.cpp']) && (egMade.saved || []).length === 3 &&
    !egDoc(egGenF) && !(egDoc(egSrvF) && egDoc(egSrvF).isDirty) && !(egDoc(egCmF) && egDoc(egCmF).isDirty) &&
    fs.readFileSync(egSrvF, 'utf8') === egSrv0 && fs.readFileSync(egCmF, 'utf8') === egCm0 && !fs.existsSync(egGenF) &&
    egRow.cmd === 'htd.event' && egRow.via === 'htd' && egRow.web != null && egRow.server != null && /網頁也接好了/.test(egInfo.args[0]) && /已經直接存檔/.test(egInfo.args[0])),
    '事件表: ... and wired: the page\'s htdCpp line (+ its helper) unsaved with the handler; the build\'s part to disk at once -- HtdEvents.gen.cpp first (calls the handler only when the class has it), then CMakeLists.txt, then wb_serve.cpp\'s htd.event branch (ASCII note, same line count, every other byte kept); the real tree untouched',
    egMade ? JSON.stringify({ done: egMade.done, saved: egMade.saved, writes: egWrites, notes: egMade.notes }) : '-');
  // the next event: the CMake entry and the server's branch are there now -- only the table is written again
  const egWrites0 = egWrites.length;
  // a second double-click on it: its code (no second copy)
  const egAgain = egData ? await api.hub.onEventGrid(egData, 'OnMouseUp', api.hub.active) : null;
  const egHText2 = egH ? egH.getText() : '';
  ok(!(egAgain && egAgain.h) && egHText2 === egHText && (egHText2.match(/spbSaveMouseUp/g) || []).length === 1,
    '事件表: the new handler is set now -- a second double-click opens it, no second copy');
  // a typed name: renames the handler the designer added (.h and .cpp follow); the .dfm's is not renamed
  const egRen = egData ? await api.hub.onEventName(egData, 'OnMouseUp', 'spbSaveUp', api.hub.active) : null;
  const egHR = egH ? egH.getText() : '', egCR = egC ? egC.getText() : '';
  const egDfmCalls = fake._calls.length;
  const egDfmRen = egData ? await api.hub.onEventName(egData, 'OnClick', 'spbSaveGo', api.hub.active) : null;
  const egDfmRefused = fake._calls.slice(egDfmCalls).some(c => (c.name === 'warning' || c.name === 'info' || c.name === 'error') && /golden|唯讀/.test(c.args[0])) ||
    pagePosted.some(m => m.type === 'refused');
  const egTyped = egData ? await api.hub.onEventName(egData, 'OnMouseDown', 'spbSavePress', api.hub.active) : null;
  const egHT = egH ? egH.getText() : '';
  ok(!!(egRen && egRen.from === 'spbSaveMouseUp' && egRen.to === 'spbSaveUp' && /void spbSaveUp\(TObject \*Sender, TMouseButton/.test(egHR) && !/spbSaveMouseUp/.test(egHR) &&
    /void TfHotPlate::spbSaveUp\(/.test(egCR) && !/spbSaveMouseUp/.test(egCR) && !egDfmRen &&
    egTyped && egTyped.name === 'spbSavePress' && /void spbSavePress\(TObject \*Sender, TMouseButton Button, TShiftState Shift, int X, int Y\);/.test(egHT)),
    '事件表: a typed name renames the added handler in fHotPlate.h / .cpp; an empty one gets the typed name; the .dfm\'s OnClick is not renamed',
    JSON.stringify({ ren: egRen, dfm: egDfmRen, typed: egTyped && egTyped.name }));
  // the rename's wiring followed: the page's line, the generated table's row; the typed one is wired too
  const egPageR = egPage ? egPage.getText() : '', egGenR = egMemT(egGenF);
  ok(!!(egRen && egRen.page === 1 && egRen.gen === 1 && /htdCpp\('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveUp', 'OnMouseUp'\);/.test(egPageR) && !/spbSaveMouseUp/.test(egPageR) &&
    /^\/\/ htd-row TfHotPlate spbSaveUp spbSave OnMouseUp\r?$/m.test(egGenR) && /^\/\/ htd-row TfHotPlate spbSavePress spbSave OnMouseDown\r?$/m.test(egGenR) &&
    /o->spbSaveUp\(nullptr, HtdButton\(a\)/.test(egGenR) && /return C\d+\(fHotPlate, a, 0\);/.test(egGenR) && !/spbSaveMouseUp/.test(egGenR) && /htdCpp\('spbSave', 'mousedown', 'TfHotPlate', 'spbSavePress', 'OnMouseDown'\);/.test(egPageR) &&
    (egPageR.match(/function htdCpp\(/g) || []).length === 1),
    '事件表: the renamed handler\'s page line and generated row follow (spbSaveUp); the typed spbSavePress is wired too (one helper on the page)',
    JSON.stringify({ page: egRen && egRen.page, gen: egRen && egRen.gen }));
  // WPF / Windows Forms: the value's list = the class's methods that fit (with a compiled body: sbtExitClick yes, the
  // GATE spbSaveClick no); Reset takes the page line and the generated row off, the C++ stays; an existing method picked
  // for another event = only the wiring (the same handler serves two events)
  const egPick = egClickRow && egClickRow.pick || [];
  const egResetR = egData ? await api.hub.onEventReset(egData, 'OnMouseDown', api.hub.active, { answer: 'keep' }) : null;
  const egPageX = egPage ? egPage.getText() : '', egGenX = egMemT(egGenF), egHX = egH ? egH.getText() : '';
  const egDownRow = egData && egData.eventGrid.rows.find(r => r.name === 'OnMouseDown');
  const egDownAfterReset = egDownRow ? egDownRow.handler : null;   // (the same row object is used again below)
  const egUse = egData ? await api.hub.onEventName(egData, 'OnMouseDown', 'spbSaveUp', api.hub.active) : null;
  const egPageU = egPage ? egPage.getText() : '', egGenU = egMemT(egGenF), egHU = egH ? egH.getText() : '';
  const egDfmReset = egData ? await api.hub.onEventReset(egData, 'OnClick', api.hub.active, { answer: 'keep' }) : null;
  const egWf = {
    pick: egPick.includes('sbtExitClick') && !egPick.includes('spbSaveClick') && !egPick.includes('FormShow'),
    reset: !!(egResetR && egResetR.handler === 'spbSavePress'), resetPage: !/'spbSavePress'/.test(egPageX), resetGen: !/htd-row TfHotPlate spbSavePress/.test(egGenX),
    keptCpp: /void spbSavePress\(/.test(egHX), rowEmpty: egDownAfterReset === '' && egDownRow.handler === 'spbSaveUp',
    use: !!(egUse && egUse.used && egUse.wired), usePage: /htdCpp\('spbSave', 'mousedown', 'TfHotPlate', 'spbSaveUp', 'OnMouseDown'\);/.test(egPageU) && /htdCpp\('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveUp', 'OnMouseUp'\);/.test(egPageU),
    useGen: /^\/\/ htd-row TfHotPlate spbSaveUp spbSave OnMouseDown\r?$/m.test(egGenU) && /^\/\/ htd-row TfHotPlate spbSaveUp spbSave OnMouseUp\r?$/m.test(egGenU),
    noCode: egHU === egHX, dfmKept: !egDfmReset,
  };
  ok(Object.keys(egWf).every(k => egWf[k]),
    '事件表 like Windows Forms: the list = fitting methods with a body (sbtExitClick, not the GATE spbSaveClick); Reset = the page line + generated row off, the C++ kept; an existing method for another event = wiring only (one handler, two events); a .dfm one cannot be reset',
    JSON.stringify(Object.keys(egWf).filter(k => !egWf[k])) + ' ' + JSON.stringify({ pick: egPick, use: egUse && (egUse.notes || egUse.done) }));
  // 0.162 刪除事件 (EastSun: "刪除事件時 需要連動刪除 要可以編譯成功 刪除事件時要跳出提醒視窗"): a new spbSaveMove, then
  // delete it -- the warning lists what goes; 刪除事件和函式 = the page line, the generated row, the .h line, the .cpp function
  {
    const dm = egData ? await api.hub.onEventName(egData, 'OnMouseMove', 'spbSaveMove', api.hub.active) : null;
    const hasH = egH ? /void spbSaveMove\(/.test(egH.getText()) : false;
    const cDoc = Object.values(fake._docs || {}).find(x => /fHotPlate\.cpp$/i.test(x.uri ? x.uri.fsPath : ''));
    const hasC = cDoc ? /TfHotPlate::spbSaveMove\(/.test(cDoc.getText()) : false;
    const w0 = fake._calls.length;
    fake._answer = '刪除事件和函式';
    const del = egData ? await api.hub.onEventReset(egData, 'OnMouseMove', api.hub.active) : null;
    fake._answer = undefined;
    const warn = fake._calls.slice(w0).filter(c => c.name === 'warning').map(c => c.args[0]).join(' ');
    const hT = egH ? egH.getText() : '', cT = cDoc ? cDoc.getText() : '', pT = egPage ? egPage.getText() : '', gT = egMemT(egGenF);
    // a function something else still calls: kept, and said
    const kept = egData ? await api.hub.onEventName(egData, 'OnMouseMove', 'spbSaveMove2', api.hub.active) : null;
    if (cDoc && cDoc._setText) cDoc._setText(cDoc.getText() + '\nvoid TfHotPlate::Helper152() { spbSaveMove2(nullptr, TShiftState(), 0, 0); }\n');
    const w1 = fake._calls.length;
    fake._answer = '刪除事件（保留函式）';
    const del2 = egData ? await api.hub.onEventReset(egData, 'OnMouseMove', api.hub.active) : null;
    fake._answer = undefined;
    const warn2 = fake._calls.slice(w1).filter(c => c.name === 'warning').map(c => c.args[0]).join(' ');
    const got = { made: !!dm, hasH, hasC, del: del && { removedFn: del.removedFn, done: del.done }, del2: del2 && { removedFn: del2.removedFn, keepWhy: del2.keepWhy } };
    ok(!!(dm && hasH && hasC && /刪除事件 spbSave\.OnMouseMove（spbSaveMove）/.test(warn) && /TfHotPlate::spbSaveMove/.test(warn) && del && del.removedFn &&
      !/spbSaveMove\b\(/.test(hT) && !/TfHotPlate::spbSaveMove\(/.test(cT) && !/'spbSaveMove'/.test(pT) && !/htd-row TfHotPlate spbSaveMove spbSave/.test(gT) &&
      kept && del2 && del2.removedFn === false && /編不過/.test(del2.keepWhy || '') && /保留/.test(warn2) && /void spbSaveMove2\(/.test(egH.getText())),
      'delete an event (0.162): the warning lists what goes; with the function = the page line, the generated row, the .h line and the .cpp function gone; one still called elsewhere is kept (said: it would not compile)',
      JSON.stringify(got));
  }
  // 移到事件處理函式 (WPF XAML: right-click the event -> Navigate to Event Handler): the cursor on a designer event's
  // htdCpp(...) line -> that C++ function (here not saved yet: the open .cpp)
  const geText = egPage ? egPage.getText() : '';
  const geAt = geText.indexOf("htdCpp('spbSave', 'mouseup'");
  const geOpen0 = fake._state.opened.length;
  Object.defineProperty(fake.window, 'activeTextEditor', { configurable: true, get: () => (geAt >= 0 ? { document: egPage, selection: { active: egPage.positionAt(geAt + 3) } } : null) });
  const geR = geAt >= 0 ? await fake._cmds['ht9045Designer.gotoEventHandler']() : null;
  Object.defineProperty(fake.window, 'activeTextEditor', { configurable: true, get: () => null });
  const geOpened = fake._state.opened.slice(geOpen0).pop();
  ok(!!(geR && geR.handler === 'spbSaveUp' && geOpened && /fHotPlate\.cpp$/i.test(geOpened.uri.fsPath)),
    'Navigate to Event Handler (WPF): on the htdCpp line of spbSave mouseup -> TfHotPlate::spbSaveUp in fHotPlate.cpp', JSON.stringify({ r: geR, opened: geOpened && path.basename(geOpened.uri.fsPath) }));
  // Shift+F7 from the HTML (WPF: View Designer): the element at the cursor -- in its start tag, in its text, or in a
  // child without an id -- is the one selected in the designer
  const vdT = egPage0;
  const vdTag = vdT.search(/<[a-z]+[^>]*\sid="spbSave"/i);
  const vdIn = vdTag >= 0 ? vdT.indexOf('>', vdTag) + 1 : -1;
  const vdCap = vdIn > 0 ? vdT.indexOf('Save', vdIn) : -1;
  ok(!!(vdTag >= 0 && api.hub.idAtOffset(vdT, vdTag + 3) === 'spbSave' && (vdCap < 0 || api.hub.idAtOffset(vdT, vdCap + 1) === 'spbSave') &&
    api.hub.idAtOffset(vdT, vdT.indexOf('<head') + 2) === null),
    'Shift+F7 (View Designer): the element at the cursor -- spbSave in its start tag and in its caption; none in <head>',
    JSON.stringify({ tag: vdTag >= 0 ? api.hub.idAtOffset(vdT, vdTag + 3) : '-', cap: vdCap > 0 ? api.hub.idAtOffset(vdT, vdCap + 1) : '-' }));
  // WPF's split view: the cursor in the page's HTML beside the designer (its caption text, the form's own div) selects that
  const svForm = vdT.search(/<div\b[^>]*\bclass="form"/);
  const svAt = async off => {
    pagePosted.length = 0;
    api.hub.onEditorSelection({ kind: fake.TextEditorSelectionChangeKind.Keyboard, textEditor: { document: egPage }, selections: [{ active: egPage.positionAt(off) }] });
    await new Promise(r => setTimeout(r, 320));
    const m = pagePosted.filter(x => x.type === 'selectId').pop();
    return m ? m.id : null;
  };
  // (it posts only when the element differs from the one selected last)
  const svLast = api.hub.active.lastSelId;
  api.hub.active.lastSelId = null;
  const svCap = vdCap > 0 ? await svAt(vdCap + 1) : null;
  api.hub.active.lastSelId = null;
  const svFm = svForm >= 0 ? await svAt(svForm + 5) : null;
  api.hub.active.lastSelId = svLast;
  ok(svCap === 'spbSave' && (svForm < 0 || svFm === '@form'),
    'split view (WPF): the cursor in spbSave\'s caption text selects spbSave; inside <div class="form"> the form', JSON.stringify({ cap: svCap, form: svFm }));
  // WPF: a double-click on the component = its default event; an empty one is created (Label1, a TLabel: OnClick ->
  // Label1Click, wired); F7 (View Code) with an empty default event only opens the form's code, nothing added
  await selAs('Label1', 'span');
  const dcSel = api.hub.active.sel;
  const dcData = dcSel ? await dcSel.promise : null;
  const dcRow0 = dcData && dcData.eventGrid ? dcData.eventGrid.rows.find(r => r.name === 'OnClick') : null;
  const dcWasEmpty = !!(dcRow0 && !dcRow0.handler);
  const dcMade = await api.hub.onDesignerDblClick(api.hub.active);
  const dcH = egH ? egH.getText() : '';
  await selAs('Label2', 'span');
  const vcH0 = egH ? egH.getText() : '';
  const vcOpen0 = fake._state.opened.length;
  await api.hub.onDesignerDblClick(api.hub.active, true);
  const vcOpened = fake._state.opened.slice(vcOpen0).pop();
  ok(!!(dcWasEmpty && dcMade && dcMade.name === 'Label1Click' && dcMade.wired && /void Label1Click\(TObject \*Sender\);/.test(dcH) &&
    egH.getText() === vcH0 && !/Label2Click/.test(egH.getText()) && vcOpened && /fHotPlate\.cpp$/i.test(vcOpened.uri.fsPath)),
    'WPF: a double-click on Label1 (OnClick empty) creates Label1Click and wires it; F7 (View Code) on Label2 only opens fHotPlate.cpp',
    JSON.stringify({ empty: dcWasEmpty, made: dcMade && dcMade.name, wired: dcMade && dcMade.wired, opened: vcOpened && path.basename(vcOpened.uri.fsPath) }));
  // AI(W906-HTDESIGNER) 20261001 -- BCB6's Object Inspector / Windows Forms with several selected: only the events they all
  // have; handlers that differ = empty; a name typed = THAT function for all of them (made once, each one wired); a .dfm
  // one among them = read only; Reset = every one's designer wiring off, the C++ kept
  pageRecv({ __htd: 1, type: 'select', info: { key: 961, id: 'spbSave', multi: ['sbtExit'], tag: 'button', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'multi' });
  await new Promise(r => setTimeout(r, 30));
  const mvSel = api.hub.active.sel;
  const mvData = mvSel ? await mvSel.promise : null;
  const mvG = mvData && mvData.eventGrid;
  const mvRow = n => (mvG ? mvG.rows.find(r => r.name === n) : null);
  const mvUp = mvRow('OnMouseUp'), mvClick = mvRow('OnClick'), mvDbl = mvRow('OnDblClick');
  const mvShape = {
    multi: !!(mvG && JSON.stringify(mvG.multi) === '["sbtExit"]'),
    names: !!(mvG && mvG.rows.map(r => r.name).join(',') === 'OnClick,OnDblClick,OnMouseDown,OnMouseMove,OnMouseUp'),
    upMixed: !!(mvUp && mvUp.handler === 'spbSaveUp' && mvUp.mixed === true && !mvUp.multiRo),
    clickRo: !!(mvClick && mvClick.multiRo === true && mvClick.mixed === true),
    dblSame: !!(mvDbl && !mvDbl.handler && !mvDbl.mixed && !mvDbl.multiRo),
  };
  const mvH0 = egH ? egH.getText() : '';
  const mvUse = mvData ? await api.hub.onEventName(mvData, 'OnMouseUp', 'spbSaveUp', api.hub.active) : null;
  const mvPageU = egPage ? egPage.getText() : '', mvGenU = egMemT(egGenF), mvHU = egH ? egH.getText() : '';
  const mvNew = mvData ? await api.hub.onEventName(mvData, 'OnDblClick', 'BothDbl', api.hub.active) : null;
  const mvPageN = egPage ? egPage.getText() : '', mvGenN = egMemT(egGenF), mvHN = egH ? egH.getText() : '';
  const mvRoCalls = fake._calls.length;
  const mvRo = mvData ? await api.hub.onEventName(mvData, 'OnClick', 'BothClick', api.hub.active) : null;
  const mvRoSaid = pagePosted.some(m => m.type === 'refused') || fake._calls.slice(mvRoCalls).some(c => /golden|唯讀/.test(String(c.args && c.args[0])));
  const mvPageRo = egPage ? egPage.getText() : '';
  const mvReset = mvData ? await api.hub.onEventReset(mvData, 'OnMouseUp', api.hub.active, { answer: 'keep' }) : null;
  const mvPageX = egPage ? egPage.getText() : '', mvGenX = egMemT(egGenF), mvHX = egH ? egH.getText() : '';
  const mvOk = Object.assign({}, mvShape, {
    use: !!(mvUse && mvUse.first && mvUse.first.skipped && JSON.stringify(mvUse.done) === '["sbtExit"]' && !mvUse.notes.length && mvH0 === mvHU),
    usePage: /htdCpp\('sbtExit', 'mouseup', 'TfHotPlate', 'spbSaveUp', 'OnMouseUp'\);/.test(mvPageU) && /htdCpp\('spbSave', 'mouseup', 'TfHotPlate', 'spbSaveUp', 'OnMouseUp'\);/.test(mvPageU),
    useGen: /^\/\/ htd-row TfHotPlate spbSaveUp sbtExit OnMouseUp\r?$/m.test(mvGenU) && /^\/\/ htd-row TfHotPlate spbSaveUp spbSave OnMouseUp\r?$/m.test(mvGenU) && !mvUp.mixed,
    made: !!(mvNew && mvNew.first && mvNew.first.name === 'BothDbl' && JSON.stringify(mvNew.done) === '["sbtExit"]') && (mvHN.match(/void BothDbl\(TObject \*Sender\);/g) || []).length === 1,
    madePage: /htdCpp\('spbSave', 'dblclick', 'TfHotPlate', 'BothDbl', 'OnDblClick'\);/.test(mvPageN) && /htdCpp\('sbtExit', 'dblclick', 'TfHotPlate', 'BothDbl', 'OnDblClick'\);/.test(mvPageN),
    madeGen: /^\/\/ htd-row TfHotPlate BothDbl spbSave OnDblClick\r?$/m.test(mvGenN) && /^\/\/ htd-row TfHotPlate BothDbl sbtExit OnDblClick\r?$/m.test(mvGenN),
    roRefused: !mvRo && mvRoSaid && mvPageRo === mvPageN && !/BothClick/.test(mvHN),
    reset: !!(mvReset && JSON.stringify(mvReset.ids) === '["spbSave","sbtExit"]') && !/'mouseup'/.test(mvPageX) && !/OnMouseUp\r?$/m.test(mvGenX) && /BothDbl sbtExit OnDblClick/.test(mvGenX) &&
      /void spbSaveUp\(/.test(mvHX) && mvUp.handler === '' && !mvUp.mixed,
  });
  ok(Object.keys(mvOk).every(k => mvOk[k]),
    '事件表, several selected (BCB6 / Windows Forms): only the events both have; OnMouseUp spbSaveUp vs none = mixed; OnClick (.dfm) read only; spbSaveUp typed = sbtExit wired to it too; BothDbl typed = made once, both wired; Reset = both off, the C++ kept',
    JSON.stringify(Object.keys(mvOk).filter(k => !mvOk[k])) + ' ' + JSON.stringify({ use: mvUse && { done: mvUse.done, notes: mvUse.notes }, made: mvNew && { done: mvNew.done, notes: mvNew.notes }, reset: mvReset }));
  if (egMade) { delete fake._docs[fake.Uri.file(egMade.h).toString()]; delete fake._docs[fake.Uri.file(egMade.cpp).toString()]; }
  for (const f of [egSrvF, egCmF]) if (f) delete fake._docs[fake.Uri.file(f).toString()];
  if (egGenF) { delete fake._docs[fake.Uri.file(egGenF).toString()]; delete fake._docs[fake.Uri.file(egGenF).with({ scheme: 'untitled' }).toString()]; }
  // after the first event only the generated table is rewritten (the CMake entry and the server's branch are there)
  const egLater = egWrites.slice(egWrites0);
  ok(egLater.length > 0 && egLater.every(w => w === 'HtdEvents.gen.cpp'),
    '事件表: after the first event only HtdEvents.gen.cpp is written again (rename / reset / another event); CMakeLists.txt and wb_serve.cpp once', JSON.stringify(egLater));
  // an editor holding unsaved text of the table (an old untitled tab of it): nothing is wired, said -- a save of that
  // tab would put its old text over the designer's
  const bzDoc = await fake.workspace.openTextDocument(fake.Uri.file(egGenF).with({ scheme: 'untitled' }));
  const bzW = egWrites.length, bzCalls = fake._calls.length;
  const bzMade = dcData ? await api.hub.cmdCreateEvent(api.hub.active, dcData, dcData.eventGrid.rows.find(r => r.name === 'OnMouseDown'), 'Label1Down') : null;
  const bzSaid = fake._calls.slice(bzCalls).find(c => c.name === 'info' && /HtdEvents\.gen\.cpp 在編輯器裡有還沒存的修改/.test(c.args[0]));
  ok(!!(bzDoc && bzMade && !bzMade.wired && egWrites.length === bzW && bzSaid && (bzMade.notes || []).some(n => /沒存的分頁/.test(n))),
    '事件表: an unsaved tab of HtdEvents.gen.cpp open = nothing written to disk, not wired, the reason said (save it or close it without saving)',
    bzMade ? JSON.stringify(bzMade.notes) : '-');
  delete fake._docs[fake.Uri.file(egGenF).with({ scheme: 'untitled' }).toString()];
  if (bzMade) delete fake._docs[fake.Uri.file(bzMade.h).toString()], delete fake._docs[fake.Uri.file(bzMade.cpp).toString()];
  api.hub.disk = egRealDisk;
  if (egPage && egPage._setText) { egPage._setText(egPage0); egPage.isDirty = false; }

  // Name / Alias / 網頁事件 from the panel
  const nmCalls = pagePosted.length;
  const alRes = egData ? api.hub.cmdSetAlias(api.hub.active, egData, 'C_Test_Alias') : null;
  const alPost = pagePosted.slice(nmCalls).find(m => m.type === 'setAttrRaw' && m.name === 'title');
  const jsDoc = api.hub.active.doc;
  const jsBefore = jsDoc.getText();
  const jsRes = egData ? await api.hub.cmdJsEvent(api.hub.active, egData, 'click', 'spbSaveJsClick') : null;
  const jsAfter = jsDoc.getText();
  const jsRen = egData ? await api.hub.cmdJsEvent(api.hub.active, egData, 'click', 'spbSaveGoJs') : null;
  const jsAfter2 = jsDoc.getText();
  ok(!!(alRes && alRes.to === 'C_Test_Alias' && alPost && /｜Alias=C_Test_Alias$/.test(alPost.value) && /^spbSave : TSpeedButton/.test(alPost.value) &&
    jsRes && jsRes.added === 'spbSaveJsClick' && /<script id="htdEvents">[\s\S]*function spbSaveJsClick\(e\) \{[\s\S]*addEventListener\('click', spbSaveJsClick\);[\s\S]*<\/script>\s*<\/body>/.test(jsAfter) &&
    jsAfter.slice(0, jsAfter.indexOf('<script id="htdEvents">')) === jsBefore.slice(0, jsAfter.indexOf('<script id="htdEvents">')) &&
    jsRen && jsRen.renamed === 'spbSaveJsClick' && /function spbSaveGoJs\(e\)/.test(jsAfter2) && !/spbSaveJsClick/.test(jsAfter2) &&
    egData.edit.jsEvents.handlers.click && egData.edit.jsEvents.handlers.click.fn === 'spbSaveGoJs'),
    'Alias written into the title (the rest kept); 網頁事件: a name = the function + addEventListener at the end of the page, a new name renames it',
    alPost ? alPost.value : '-');
  // (the page's text back as it was for the tests after this)
  if (jsDoc._setText) { jsDoc._setText(jsBefore); jsDoc.isDirty = false; }

  // a .csv shown as text (a tab from before the update): the status bar offers the table, asked once per file
  // (by itself: a text tab of a .csv becomes the table -- EastSun: "開CSV 怎還是一樣的畫面?")
  const ctAuto0 = fake._calls.length;
  api.hub.csvTextEditor({ document: { uri: fake.Uri.file('C:\\x\\Mot_Table.csv') } });
  await new Promise(r => setTimeout(r, 30));
  const ctAuto = fake._calls.slice(ctAuto0).find(c => c.name === 'exec' && c.args[0] === 'vscode.openWith' && /Mot_Table\.csv/.test(c.args[1]) && c.args[2] === 'ht9045Designer.csv');
  const ctAutoAsk = fake._calls.slice(ctAuto0).some(c => c.name === 'info');
  ok(!!(ctAuto && !ctAutoAsk), 'a .csv shown as text: switched to the table by itself (no question)', ctAuto && ctAuto.args.join(' '));
  // with the setting off: the status bar button and one question
  fake._config.csvAutoTable = false;
  const ctCalls = fake._calls.length;
  const ctUri = fake.Uri.file('C:\\x\\IO_Table.csv');
  const ctOn = api.hub.csvTextEditor({ document: { uri: ctUri } });
  const ctItemShown = api.hub.csvItem.shown;
  api.hub.csvTextEditor({ document: { uri: ctUri } });
  const ctAsk = fake._calls.slice(ctCalls).filter(c => c.name === 'info' && /IO_Table\.csv.*表格/.test(c.args[0])).length;
  delete fake._config.csvAutoTable;
  const ctOff = api.hub.csvTextEditor(undefined);
  const ctHtml = api.hub.csvTextEditor({ document: { uri: fake.Uri.file('C:\\x\\a.html') } });
  const ctOpen0 = fake._calls.length;
  await fake._cmds['ht9045Designer.openCsvTable'](ctUri);
  const ctOpenExec = fake._calls.slice(ctOpen0).find(c => c.name === 'exec' && c.args[0] === 'vscode.openWith');
  ok(!!(ctOn && ctItemShown && api.hub.csvItem.command === 'ht9045Designer.openCsvTable' && ctAsk === 1 && ctOff === false && ctHtml === false && !api.hub.csvItem.shown &&
    ctOpenExec && ctOpenExec.args[2] === 'ht9045Designer.csv'),
    'a .csv open as text: the status bar button (用表格開啟) and one question per file; the button opens the table', ctOpenExec && ctOpenExec.args.join(' '));

  // 機種: which machine the pages are shown for -- the page gets it (and read-only copies of the settings)
  const lcHintsSm = require('../lib/liveconfig').launchHints([api.hub.active.r.portRoot]);
  const mcQp0 = fake._state.quickPick;
  let mcItems = null;
  fake._state.quickPick = items => { mcItems = items; return items.find(i => i.pick === 'HT9050'); };
  const mcPick = await fake._cmds['ht9045Designer.pickMachine']();
  fake._state.quickPick = mcQp0;
  const mcHtml = panel.webview.html;
  const mcStatus = api.hub.status.text;
  const mcNone = await fake._cmds['ht9045Designer.pickMachine']('-');
  const mcNoneHtml = panel.webview.html;
  const mcAuto = await fake._cmds['ht9045Designer.pickMachine']('');
  const mcAutoHtml = panel.webview.html;
  ok(!!(mcItems && mcItems.some(i => i.pick === '') && mcItems.some(i => i.pick === 'HT9045') && mcItems.some(i => i.pick === '-') &&
    mcPick && mcPick.machine === 'HT9050' && /id="__htd_live"[^>]*>\{"machine":"HT9050"/.test(mcHtml) && mcHtml.indexOf('__htd_live') < mcHtml.indexOf('probe.js') &&
    /HT9050/.test(mcStatus) && /"pathFix":\{"from":"[^"]*\/web\/page\/JSON\/","to":"[^"]*\/web\/JSON\/"/.test(mcHtml)),
    'machine picked from the list (like F5 / HT9045 / HT9050 / none): the page gets HT9050 before any script; the status bar says it; the JSON folder fix',
    mcStatus);
  ok(!!(mcNone && mcNone.machine === null && mcNone.docs.length === 0 && !/"machine":"HT9050"/.test(mcNoneHtml) &&
    mcAuto && mcAuto.auto === !!lcHintsSm.machine && mcAuto.machine === (lcHintsSm.machine || null) &&
    (!lcHintsSm.generalIni || !fs.existsSync(lcHintsSm.generalIni) || (mcAuto.docs.includes('gerneral') && /"gerneral":\{"path"/.test(mcAutoHtml)))),
    'none: no machine and no settings; like F5: the machine launch.json opens the HMI for, its settings file (read only)',
    JSON.stringify({ none: mcNone, auto: mcAuto, hints: lcHintsSm.machine }));
  ok(!!(swItem && swItem.shown && swItem.command === 'ht9045Designer.openPage' && /Setup\.HotPlate\.html/.test(swItem.text)),
    'status bar: the page switcher shows Setup.HotPlate.html and opens the picker', swItem && swItem.text);
  const swQp0 = fake._state.quickPick;
  fake._state.quickPick = items => items[0];
  const swCalls = fake._calls.length;
  await fake._cmds['ht9045Designer.openPage']();
  fake._state.quickPick = swQp0;
  const swQp = fake._calls.slice(swCalls).find(c => c.name === 'quickPick');
  const swExec = fake._calls.slice(swCalls).filter(c => c.name === 'exec').map(c => c.args[0]);
  ok(!!(swQp && /頁面.*清單/.test(swQp.args[0][0]) && swQp.args[0].some(l => /Setup\.HotPlate\.html ● 目前/.test(l)) && swExec.includes('ht9045Designer.solution.focus')),
    'the picker: "show the 頁面 list" first (-> 方案總管 focused, 0.139: the list is in it), this page marked 目前', swQp ? swQp.args[0].slice(0, 2).join(' | ') : '-');

  // every component's name on the surface: told to the probe, shown in the status bar
  pagePosted.length = 0;
  const nmOn = await fake._cmds['ht9045Designer.toggleNames']();
  const nmMsg = pagePosted.filter(m => m.type === 'showNames').pop();
  const nmStatus = api.hub.status.text;
  const nmOff = await fake._cmds['ht9045Designer.toggleNames']();
  ok(nmOn && nmOn.on && nmMsg && nmMsg.on === true && /名稱/.test(nmStatus) && nmOff && nmOff.on === false && !/　名稱/.test(api.hub.status.text),
    'name tags: on (the probe told, the status bar), off again', nmStatus);

  // 頁面檢查: the Problems panel gets HotPlate's cut style; the bulb offers the fix; all pages: 297
  const lnSum = await fake._cmds['ht9045Designer.lintPage']();
  const lnDiags = fake._diag.get(doc.uri) || [];
  ok(lnSum && lnSum.error === 1 && lnDiags.length === 1 && lnDiags[0].code === 'quote-cut' && lnDiags[0].source === 'HTML 頁面檢查' && lnDiags[0].range.start.line === 55,
    'lint this page: one cut style in the Problems panel, on line 56', lnDiags.map(x => x.code + '@' + (x.range.start.line + 1)).join(','));
  const lnActs = fake._codeActions.provideCodeActions(Object.assign({}, doc, {
    getText: r => (r ? text.slice(doc.offsetAt(r.start), doc.offsetAt(r.end)) : text),
  }), lnDiags[0].range, { diagnostics: lnDiags });
  const lnFix = lnActs.find(a => a.edit);
  ok(lnActs.length === 2 && lnFix && /font-family:'MS Sans Serif'/.test(lnFix.edit.ops[0].text) && lnFix.isPreferred && lnActs[1].command.command === 'ht9045Designer.lintFixAll',
    'the bulb: this one fixed (the inner " become \'), or the whole page', lnActs.map(a => a.title).join(' | '));
  // "differs from the .dfm" (information): Setup.Contact's labDelayStatus -- in the panel, with its own bulb
  const ctPath = path.join(PORT, '..', 'web', 'page', 'Setup.Contact.html');
  const ctIssues = api.hub.lintFile(ctPath);
  const ctDiags = fake._diag.get(fake.Uri.file(ctPath)) || [];
  const ctGap = ctDiags.find(x => x.code === 'dfm-gap' && /labDelayStatus/.test(x.message));
  const ctText = fs.readFileSync(ctPath, 'utf8');
  const ctLines = [0]; for (let i = 0; i < ctText.length; i++) if (ctText.charCodeAt(i) === 10) ctLines.push(i + 1);
  const ctDoc = { uri: fake.Uri.file(ctPath), getText: r => (r ? ctText.slice(ctDoc.offsetAt(r.start), ctDoc.offsetAt(r.end)) : ctText),
    offsetAt: p => ctLines[p.line] + p.character,
    positionAt: o => { let l = 0; while (l + 1 < ctLines.length && ctLines[l + 1] <= o) l++; return new fake.Position(l, o - ctLines[l]); } };
  const ctActs = ctGap ? fake._codeActions.provideCodeActions(ctDoc, ctGap.range, { diagnostics: [ctGap] }) : [];
  const ctFix = ctActs.find(a => a.edit);
  ok(ctGap && ctGap.severity === fake.DiagnosticSeverity.Information && ctIssues.some(i => i.kind === 'dfm-gap') && ctFix && /width:273px/.test(ctFix.edit.ops[0].text) &&
    ctActs.some(a => a.command && a.command.command === 'ht9045Designer.lintFixDfmGaps') && api.hub.pages.problemsOf(ctPath) === ctDiags.filter(x => x.severity !== fake.DiagnosticSeverity.Information).length,
    'differs from the .dfm (information): labDelayStatus in the Problems panel, the bulb gives its 273 x 25; the 頁面 list counts only the problems', ctGap ? ctGap.message.slice(0, 60) : '-');
  const lnAll = await fake._cmds['ht9045Designer.lintAll']();
  // AI(W906-HTDESIGNER) 20261002 (machine): the cut attributes are counted here over the same page list, the same text
  // (an open document's), not pinned to a number -- other sessions fix pages (HW.home.html 34 -> 1 at 20261001 23:43,
  // all pages 297 -> 264) and a fixed ">= 290" failed with nothing wrong in the designer
  const lnPl = require(path.join(__dirname, '..', 'lib', 'pagelint.js')), lnList = require(path.join(__dirname, '..', 'lib', 'pagelist.js'));
  const lnFiles = [].concat(...lnList.listPages(path.join(PORT, '..', 'web')).map(g => g.pages.map(p => p.file)));
  let lnCut = 0, lnCutPages = 0;
  for (const f of lnFiles) {
    const od = (fake.workspace.textDocuments || []).find(td => td.uri && td.uri.scheme === 'file' && path.resolve(td.uri.fsPath).toLowerCase() === path.resolve(f).toLowerCase());
    let tx = ''; try { tx = od ? od.getText() : fs.readFileSync(f, 'utf8'); } catch (e) { tx = ''; }
    const k = lnPl.lintPage(tx, { dir: path.dirname(f) }).filter(i => i.kind === 'quote-cut').length;
    if (k) { lnCut += k; lnCutPages++; }
  }
  ok(lnAll && lnCut > 0 && lnAll.byKind['quote-cut'] === lnCut && lnAll.withIssues >= lnCutPages && lnAll.info > 0 && lnAll.info === lnAll.byKind['dfm-gap'],
    'lint all pages: ' + (lnAll ? lnAll.byKind['quote-cut'] + ' cut attributes on ' + lnAll.withIssues + ' pages; ' + lnAll.info + ' labels / panels differ from their .dfm (information)' : '-'));
  // AI(W906-HTDESIGNER) 20261002 (machine; EastSun「編譯進度可以有個進度條嗎?」): F5's build status file -> a progress bar
  // that follows make's %, closes when the build ends; nothing else is touched (a temporary file, not the real one)
  {
    const bwDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-bw-'));
    const bwF = path.join(bwDir, 'boot_build.js');
    const bwW = (o) => fs.writeFileSync(bwF, 'window.__HT_BUILD=' + JSON.stringify(Object.assign({ target: 'wb_serve', dir: 'build_t', n: 3, errors: 0, lastError: '', elapsed: 4 }, o)) + ';');
    const bwReps = [], bwTitles = [];
    let bwEnded = false;
    const bwWP = fake.window.withProgress;
    fake._config['build.window'] = false;   // (these two: the notification; the build window is the next block)
    fake.window.withProgress = (o, fn) => { bwTitles.push(o.title + (o.cancellable ? ' (cancellable)' : '')); return Promise.resolve(fn({ report: r => bwReps.push(r) })).then(v => { bwEnded = true; return v; }); };
    const now = Date.now();
    bwW({ state: 'building', pct: 10, file: 'a.cpp', t: now - 60000 });
    const bwOld = api.hub.buildWatchTick(bwF, now);
    bwW({ state: 'building', pct: 10, file: 'a.cpp', t: now });
    const bwOpen = api.hub.buildWatchTick(bwF, now);
    bwW({ state: 'building', pct: 55, file: 'b.cpp', t: now + 1000 });
    const bwUpd = api.hub.buildWatchTick(bwF, now + 1000);
    bwW({ state: 'done', pct: 100, file: 'link wb_serve.exe', t: now + 2000 });
    const bwDone = api.hub.buildWatchTick(bwF, now + 2000);
    await new Promise(r => setTimeout(r, 20));
    const bwAfter = api.hub.buildWatchTick(bwF, now + 3000);
    fake.window.withProgress = bwWP;
    fs.rmSync(bwDir, { recursive: true, force: true });
    const bwInc = bwReps.reduce((s, r) => s + (r.increment || 0), 0);
    ok(bwOld === 'none' && bwOpen === 'open' && bwUpd === 'update' && bwDone === 'close-done' && bwAfter === 'none' && bwEnded &&
      bwTitles.length === 1 && /建置 wb_serve（build_t）$/.test(bwTitles[0]) && bwInc === 55 && /55%　b\.cpp/.test((bwReps[1] || {}).message || '') && !api.hub.buildBar,
      'F5 build progress bar: an old status file opens nothing; a fresh "building" opens one bar (no Cancel: F5\'s build is VS Code\'s task), it follows make\'s % (10 -> 55), closes when done',
      JSON.stringify({ old: bwOld, open: bwOpen, upd: bwUpd, done: bwDone, after: bwAfter, titles: bwTitles, inc: bwInc }));
    // the F5 path as it is (its task calls cmake directly): while the build task runs, CMake's progress folder; the
    // folder gone = done; still there when the task ends = failed
    const cmRoot = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-bwc-'));
    const cmProg = path.join(cmRoot, 'build_integ_ship_x86', 'CMakeFiles', 'Progress');
    const cmNoFile = path.join(cmRoot, 'no_status.js');
    const cmReps = [], cmErr0 = fake._calls.length;
    fake.window.withProgress = (o, fn) => Promise.resolve(fn({ report: r => cmReps.push(r) }));
    const cmT = Date.now();
    api.hub.bwTask = { name: 'IOWEB(這台): build wb_serve（出貨組態, WinLibs 32-bit）', t: cmT, ended: false, code: null, dir: null };
    const cmNone = api.hub.buildWatchTick(cmNoFile, cmT + 500, cmRoot);
    fs.mkdirSync(cmProg, { recursive: true });
    fs.writeFileSync(path.join(cmProg, 'count.txt'), '40\n');
    for (let i = 1; i <= 10; i++) fs.writeFileSync(path.join(cmProg, String(i)), '');
    const cmOpen = api.hub.buildWatchTick(cmNoFile, cmT + 1000, cmRoot);
    for (let i = 11; i <= 30; i++) fs.writeFileSync(path.join(cmProg, String(i)), '');
    const cmUpd = api.hub.buildWatchTick(cmNoFile, cmT + 2000, cmRoot);
    fs.rmSync(path.join(cmRoot, 'build_integ_ship_x86', 'CMakeFiles', 'Progress'), { recursive: true, force: true });
    const cmDone = api.hub.buildWatchTick(cmNoFile, cmT + 3000, cmRoot);
    // (1003: F5 starts the extension (onDebug) -- F5 right after opening VS Code had no bar: nothing had started it;
    //  later the same day onStartupFinished too: EastSun asked for the find window ready at start)
    const cmStart = (JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).activationEvents || []).includes('onDebug');
    // a second build that fails: the task ends, the folder stays
    api.hub.bwTask = { name: 'b2', t: cmT + 4000, ended: false, code: null, dir: null };
    fs.mkdirSync(cmProg, { recursive: true });
    fs.writeFileSync(path.join(cmProg, 'count.txt'), '40\n');
    fs.writeFileSync(path.join(cmProg, '1'), '');
    const cmOpen2 = api.hub.buildWatchTick(cmNoFile, cmT + 5000, cmRoot);
    api.hub.bwTask.ended = true; api.hub.bwTask.code = 2;
    const cmFail = api.hub.buildWatchTick(cmNoFile, cmT + 6000, cmRoot);
    const cmErr = fake._calls.slice(cmErr0).find(c => c.name === 'error' && /建置沒有完成（build_integ_ship_x86）/.test(c.args[0]));
    // (this one is expected: out of the record, so the final "no error messages shown" still means what it says)
    if (cmErr) fake._calls.splice(fake._calls.indexOf(cmErr), 1);
    fake.window.withProgress = bwWP;
    fs.rmSync(cmRoot, { recursive: true, force: true });
    const cmInc = cmReps.slice(0, 2).reduce((s, r) => s + (r.increment || 0), 0);
    ok(cmNone === 'none' && cmOpen === 'open' && cmUpd === 'update' && cmDone === 'close-done' && cmInc === 75 && cmStart && cmOpen2 === 'open' && cmFail === 'close-failed' && !!cmErr &&
      !api.hub.bwTask && !api.hub.buildBar,
      'F5 build progress bar from CMake\'s own progress folder (F5 unchanged): opens when the build starts, 25% -> 75%, the folder gone = done; a task ending with the folder left = "not finished", said',
      JSON.stringify({ none: cmNone, open: cmOpen, upd: cmUpd, done: cmDone, inc: cmInc, open2: cmOpen2, fail: cmFail, err: !!cmErr }));
  }
  // 1003 (EastSun: "請你用個獨立建置框 並且要放大 讓我看清楚"): the build in a window of its own -- opened when the build
  // starts (floated), the % / steps / time sent to it, done = green and closed by itself, not finished = red and it stays
  {
    fake._config['build.window'] = true;   // (1003: off by default now -- EastSun "視窗不要了"; the window itself still tested)
    const bwDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'htd-bww-'));
    const bwF = path.join(bwDir, 'boot_build.js');
    const bwW = (o) => fs.writeFileSync(bwF, 'window.__HT_BUILD=' + JSON.stringify(Object.assign({ target: 'wb_serve', dir: 'build_w', n: 3, total: 40, errors: 0, lastError: '', elapsed: 65 }, o)) + ';');
    const ex0 = fake.commands.executeCommand, ran = [];
    fake.commands.executeCommand = (id, ...a) => { ran.push(id); return ex0(id, ...a); };
    const n0 = (fake._panels || []).length;
    api.hub.buildWinCloseMs = 0;
    const now = Date.now();
    bwW({ state: 'building', pct: 10, file: 'a.cpp', t: now });
    const wOpen = api.hub.buildWatchTick(bwF, now);
    const pn = (fake._panels || [])[n0];
    let disposed = 0;
    if (pn) pn.dispose = () => { disposed++; };
    bwW({ state: 'building', pct: 75, n: 30, file: 'b.cpp', t: now + 1000 });
    const wUpd = api.hub.buildWatchTick(bwF, now + 1000);
    const m1 = pn && pn.posted.filter(m => m.type === 'build').pop();
    try { const dir = path.join(require('os').tmpdir(), 'htdesigner_panels'); fs.mkdirSync(dir, { recursive: true }); fs.writeFileSync(path.join(dir, 'buildwin.json'), JSON.stringify(m1)); } catch (e) { /* the render test says */ }
    bwW({ state: 'done', pct: 100, file: 'link', t: now + 2000 });
    const wDone = api.hub.buildWatchTick(bwF, now + 2000);
    const m2 = pn && pn.posted.filter(m => m.type === 'build').pop();
    await new Promise(r => setTimeout(r, 20));
    const closedDone = disposed;
    // a build that does not finish: red, and it stays
    bwW({ state: 'building', pct: 20, t: now + 5000 });
    api.hub.buildWatchTick(bwF, now + 5000);
    const pn2 = (fake._panels || [])[n0 + 1];
    let disposed2 = 0;
    if (pn2) pn2.dispose = () => { disposed2++; };
    bwW({ state: 'failed', pct: 20, errors: 2, lastError: 'x.cpp:3: error', t: now + 6000 });
    const wFail = api.hub.buildWatchTick(bwF, now + 6000);
    await new Promise(r => setTimeout(r, 20));
    const m3 = pn2 && pn2.posted.filter(m => m.type === 'build').pop();
    fake.commands.executeCommand = ex0;
    delete api.hub.buildWinCloseMs;
    delete fake._config['build.window'];
    fs.rmSync(bwDir, { recursive: true, force: true });
    const pkgB = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.configuration.properties;
    const got = { wOpen, wUpd, wDone, wFail, view: pn && pn.viewType, floated: ran.includes('workbench.action.moveEditorToNewWindow'),
      m1: m1 && { s: m1.state, pct: m1.pct, steps: m1.steps, total: m1.total, time: m1.time }, m2: m2 && m2.state, closedDone, m3: m3 && { s: m3.state, e: m3.errors }, disposed2,
      html: !!(pn && /buildwin\.js/.test(pn.webview.html) && /buildwin\.css/.test(pn.webview.html)), noStatus: !api.hub.buildItem,
      cfg: pkgB['ht9045Designer.build.window'].default === false && pkgB['ht9045Designer.build.newWindow'].default === true };
    ok(wOpen === 'open' && wUpd === 'update' && wDone === 'close-done' && wFail === 'close-failed' && got.view === 'ht9045Designer.buildWindow' && got.floated && got.html &&
      got.m1 && got.m1.s === 'building' && got.m1.pct === 75 && got.m1.steps === 30 && got.m1.total === 40 && got.m1.time === '1 分 05 秒' && got.m2 === 'done' && closedDone === 1 &&
      got.m3 && got.m3.s === 'failed' && got.m3.e === 2 && disposed2 === 0 && got.noStatus && got.cfg,
      'build window: opened (floated) when the build starts, the % / steps / time sent to it; done = green and it closes by itself; not finished = red and it stays; no status bar item',
      JSON.stringify(got));
  }  // the 頁面 list shows the counts: a page "⚠ 1", its group the sum
  const lnGroups = pv.provider.getChildren();
  const lnSetup = lnGroups.find(g => g.name === 'Setup'), lnAlert = lnGroups.find(g => g.name === 'Alert');
  const lnHpNode = lnSetup.children.find(p => p.name === 'Setup.HotPlate.html');
  const lnHpItem = pv.provider.getTreeItem(lnHpNode), lnAlertItem = pv.provider.getTreeItem(lnAlert);
  ok(/⚠ 1(\s|$)/.test(lnHpItem.description) && /⚠ 99$/.test(lnAlertItem.description) && /1 個問題/.test(lnHpItem.tooltip),
    'the 頁面 list: HotPlate "⚠ 1", the Alert group "⚠ 99" (6 + 93)', lnHpItem.description + ' | ' + lnAlertItem.description);
  const lnOne = await fake._cmds['ht9045Designer.lintPageFile'](lnHpNode);
  ok(lnOne === 1, 'right-click a page > 檢查這一頁: 1 problem');

  // 接線標示: from the overview's data (the probe answers listenersAll: sbtExit has a click listener)
  const wmPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'listenersAll') setTimeout(() => pageRecv({ __htd: 1, type: 'listenersAll', seq: m.seq, map: { sbtExit: ['click'] } }), 5);
    return wmPM(m);
  };
  pagePosted.length = 0;
  const wmOn = await fake._cmds['ht9045Designer.toggleWireMarks']();
  const wmMsg = pagePosted.filter(m => m.type === 'wireMarks').pop();
  ok(wmOn && wmOn.on && wmMsg && wmMsg.marks && wmMsg.marks.sbtExit === 'ok' && wmMsg.marks.spbSave === 'mid' && /接線標示/.test(api.hub.status.text),
    'wiring marks: sbtExit green (a click listener), spbSave orange (only delegated); the status bar says so', wmMsg ? JSON.stringify(wmOn.count) : '-');
  const wmOff = await fake._cmds['ht9045Designer.toggleWireMarks']();
  ok(wmOff && wmOff.on === false && pagePosted.filter(m => m.type === 'wireMarks').pop().marks === null && !/接線標示/.test(api.hub.status.text), 'wiring marks off');
  panel.webview.postMessage = wmPM;

  // 在所有頁面搜尋: with a string the command returns the hits (the input box / pick are for people)
  const psHits = await fake._cmds['ht9045Designer.searchPages']('spbSave');
  ok(Array.isArray(psHits) && psHits.length >= 5 && psHits.some(h => /Setup\.HotPlate\.html$/.test(h.file) && h.id === 'spbSave'),
    'search all pages for spbSave: many pages, HotPlate among them', psHits ? psHits.length + ' hits' : '-');

  // snap to gridlines: on / off for every designer, kept in the workspace state, shown in the status bar
  pagePosted.length = 0;
  const grOn = await fake._cmds['ht9045Designer.toggleGrid']();
  const grMsg = pagePosted.filter(m => m.type === 'setGrid').pop();
  ok(grOn && grOn.on === true && grOn.size === 8 && grMsg && grMsg.on === true && grMsg.size === 8 && memento.get('htd.gridOn') === true && /格線 8px/.test(api.hub.status.text),
    'grid on: the designer told (8 px), kept, the status bar says so', api.hub.status.text);
  const grOff = await fake._cmds['ht9045Designer.toggleGrid']();
  ok(grOff && grOff.on === false && memento.get('htd.gridOn') === false && !/格線/.test(api.hub.status.text), 'grid off again');
  // WPF's two toolbar buttons apart: 吸附 = snapping only (the grid not drawn), 格線 = drawn only; kept, every page told
  pagePosted.length = 0;
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'gridSnap' });
  await new Promise(r => setTimeout(r, 30));
  const gpSnap = pagePosted.filter(m => m.type === 'setGrid').pop();
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'gridSnap' });
  await new Promise(r => setTimeout(r, 30));
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'gridShow' });
  await new Promise(r => setTimeout(r, 30));
  const gpShow = pagePosted.filter(m => m.type === 'setGrid').pop();
  const gpState = api.hub.gridState();
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'gridShow' });
  await new Promise(r => setTimeout(r, 30));
  ok(!!(gpSnap && gpSnap.on === true && gpSnap.show === false && gpShow && gpShow.on === false && gpShow.show === true &&
    gpState.show === true && gpState.on === false && memento.get('htd.gridShow') === false && memento.get('htd.gridOn') === false),
    'grid toolbar (WPF): 吸附 = snapping without the grid drawn, 格線 = drawn without snapping; kept, every page told', JSON.stringify({ gpSnap, gpShow }));
  // WPF "Toggle artboard background": the toolbar's button -> kept for every page (the other designers told), in the next init
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'artboard', dark: true });
  const abOn = memento.get('htd.artboardDark') === true && api.hub.artboardDark() === true;
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'artboard', dark: false });
  ok(abOn && memento.get('htd.artboardDark') === false && api.hub.artboardDark() === false,
    'artboard background (WPF toolbar): dark kept for every page, then back to the page\'s own');
  // 1005 (Visual Studio / Blend show element bounds): 邊界 kept for every page, the next page's init carries it
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'bounds', on: true });
  const bdKept = memento.get('htd.showBounds') === true && api.hub.showBounds() === true;
  edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'bounds', on: false });
  ok(bdKept && memento.get('htd.showBounds') === false && api.hub.showBounds() === false && /showBounds: this\.hub\.showBounds\(\)/.test(fs.readFileSync(path.join(__dirname, '..', 'extension.js'), 'utf8')),
    'show element bounds (VS / Blend toolbar 邊界): kept for every page (workspace state), sent with each page\'s init; off again');
  // 0.158 the Picture picker (WPF Image.Source): an image inside the page's folder -> its relative src to the probe (no
  // copy -- only one outside the page's folder is copied into img\, not done here: that would write the real web\page\img)
  {
    const od0 = fake.window.showOpenDialog;
    const pageImg = path.join(path.dirname(edD.file), 'img', 'HonPrec.png');
    let asked = null;
    fake.window.showOpenDialog = async o => { asked = o; return fs.existsSync(pageImg) ? [fake.Uri.file(pageImg)] : undefined; };
    const p0 = pagePosted.length;
    const pr = await api.hub.cmdPickImage(edD, 42);
    fake.window.showOpenDialog = od0;
    const sentP = pagePosted.slice(p0).filter(m => m.type === 'setLook');
    ok(!!(pr && pr.src === 'img/HonPrec.png' && pr.copied === false && sentP.length === 1 && sentP[0].key === 42 && sentP[0].prop === 'src' && sentP[0].value === 'img/HonPrec.png' &&
      asked && asked.filters && asked.filters['圖片'].includes('png') && /[\\/]img$/.test(asked.defaultUri.fsPath)),
      'Picture picker (0.158): opens in the page\'s img folder, an image there = its relative src to the probe, nothing copied', JSON.stringify({ pr, sentP }));
  }
  // 0.157 the Items collection editor (WPF): HotPlate's cbSelectHPFromDB gets three options from the panel's setItems
  {
    const he7 = require('../lib/htmledit');
    const it7 = require('../lib/items');
    const before7 = text;
    const has = !!he7.startTagOf(text, 'cbSelectHPFromDB');
    const r7 = has ? await api.hub.cmdSetItems(edD, 'cbSelectHPFromDB', ['A', 'B', 'C']) : null;
    const now7 = it7.itemsOf(text, 'cbSelectHPFromDB');
    const bad7 = await api.hub.cmdSetItems(edD, 'spbSave', ['x']);
    // the panel's data for a select carries its items (the editor's rows)
    const dat7 = api.hub.props && api.hub.props.data;
    ok(!!(has && r7 && r7.kind === 'select' && r7.n === 3 && now7 && now7.items.join(',') === 'A,B,C' && bad7 === null && text.indexOf('id="spbSave"') >= 0),
      'Items editor (0.157): a select\'s options written from the panel (one edit); a button refused (no items)', JSON.stringify({ r7, now7 }));
    doc._setText(before7);
  }
  // 0.156 Tab order by clicks (WinForms View > Tab Order): the command shows the badges and starts the numbering at 1;
  // the page's "done" is said; false = stop
  {
    const t0 = pagePosted.length;
    const ts = await fake._cmds['ht9045Designer.tabOrderSet']();
    const sentT = pagePosted.slice(t0).filter(m => m.type === 'tabOrderSet' || m.type === 'tabOrder').map(m => m.type + (m.type === 'tabOrderSet' ? ':' + m.on + ':' + m.start : '')).join(',');
    const st0 = fake._calls.length;
    edD.onMessage({ __htd: 1, type: 'tabOrderSetEnd', done: ['XST1', 'XPitch1'] });
    const said = fake._calls.slice(st0).filter(c => c.name === 'status').map(c => c.args[0]).join(' ');
    const off = await fake._cmds['ht9045Designer.tabOrderSet'](false);
    const menus = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.menus['ht9045Designer.showMenu'] || [];
    ok(!!(ts && ts.on && ts.start === 1 && /tabOrder/.test(sentT) && /tabOrderSet:true:1/.test(sentT) && /Tab 順序設定完成：2 個（XST1、XPitch1）/.test(said) && off && off.on === false &&
      menus.some(x => x.command === 'ht9045Designer.tabOrderSet')),
      'Tab order by clicks (0.156): the badges on, then numbering from 1; the page\'s done said with the names; false stops; in 顯示',
      JSON.stringify({ ts, sentT, said }));
    await fake._cmds['ht9045Designer.toggleTabOrder'](false);
  }
  // 0.155 the machine's screen frame (WPF d:DesignWidth / DesignHeight): the command tells every designer; '' = none;
  // the toolbar's 螢幕 = the list (the stand-in picks the first: 不畫)
  {
    const sc0 = pagePosted.length;
    const s1 = await fake._cmds['ht9045Designer.screenSize']('1280x1024');
    const s2 = await fake._cmds['ht9045Designer.screenSize']('');
    const qpN = fake._calls.filter(c => c.name === 'quickPick').length;
    edD.onMessage({ __htd: 1, type: 'toolbar', cmd: 'screen' });
    await new Promise(r => setTimeout(r, 20));
    const qpAsked = fake._calls.filter(c => c.name === 'quickPick').length > qpN;
    const sent = pagePosted.slice(sc0).filter(m => m.type === 'setScreen').map(m => (m.screen ? m.screen.w + 'x' + m.screen.h : 'none')).join(',');
    fake._config.screenSize = ' 1920 × 1080 ';
    const cfgRead = api.hub.screenSize();
    delete fake._config.screenSize;
    const pkgS = JSON.parse(fs.readFileSync(path.join(__dirname, '..', 'package.json'), 'utf8')).contributes.configuration.properties['ht9045Designer.screenSize'];
    ok(!!(s1 && s1.w === 1280 && s1.h === 1024 && s2 === null && qpAsked && /^1280x1024,none,none$/.test(sent) && cfgRead && cfgRead.w === 1920 && cfgRead.h === 1080 && pkgS && pkgS.default === ''),
      'machine screen frame (0.155): the command tells the designers (1280x1024, then none), the toolbar button opens the list, the setting read leniently; off by default',
      JSON.stringify({ s1, s2, qpAsked, sent, cfgRead }));
  }

  // 工具箱: the view lists the 7 templates; a click adds one -- into the selected Panel2, or below spbSave
  const tbView = fake._views && fake._views['ht9045Designer.toolbox'];
  // (0.153: Pointer, then the categories with the components in them)
  const tbTop = tbView ? tbView.provider.getChildren() : [];
  const tbItems = [tbTop[0]].concat(...tbTop.slice(1).map(g => tbView.provider.getChildren(g)));
  const tbCats = tbTop.slice(1).map(g => tbView.provider.getTreeItem(g).label).join(',');
  const tbBtn = tbItems.find(i => i.cls === 'TSpeedButton');
  const tbItem = tbBtn ? tbView.provider.getTreeItem(tbBtn) : null;
  const tbPtr = tbItems[0] ? tbView.provider.getTreeItem(tbItems[0]) : null;
  ok(tbItems.length === 22 && tbCats === '常用,容器,IO 元件,其他' && tbItem && tbItem.command.command === 'ht9045Designer.toolboxArm' && tbItem.command.arguments[0] === 'TSpeedButton' &&
    tbPtr && tbPtr.label === '指標' && tbPtr.id === 'tool:@pointer' && tbPtr.command.arguments[0] === '@pointer' && tbView.provider.getParent(tbItems[0]) === null && tbView.provider.getParent(tbTop[1]) === null && tbView.provider.getParent(tbBtn) === tbTop[1],
    'toolbox view: Pointer first (WPF), then 4 categories (常用 / 容器 / IO 元件 / 其他) with 21 items, a click picks the tool (toolboxArm(class))', tbCats);
  const tbBefore = text;
  await selAs('Panel2', 'div');
  const tbAdd1 = await fake._cmds['ht9045Designer.toolboxAdd']('TSpeedButton');
  const tbP2 = (() => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; })();
  ok(tbAdd1 && tbAdd1.id === 'SpeedButton1' && tbAdd1.into === 'Panel2' && /<button class="btn3d" id="SpeedButton1" title="SpeedButton1 : TSpeedButton" style="position:absolute;left:8px;top:8px;/.test(tbP2) &&
    edD.lastSelId === 'SpeedButton1', 'toolbox: a SpeedButton1 into Panel2 at 8,8, selected afterwards', JSON.stringify(tbAdd1));
  const tbAdd2 = await fake._cmds['ht9045Designer.toolboxAdd']('TSpeedButton');
  ok(tbAdd2 && tbAdd2.id === 'SpeedButton2' && tbAdd2.x === 24 && tbAdd2.y === 24, 'the next one: SpeedButton2, 16 px further (8,8 is taken)', JSON.stringify(tbAdd2));
  await selAs('spbSave', 'button');
  const tbAdd3 = await fake._cmds['ht9045Designer.toolboxAdd']('TLabel');
  const tbSave = hb2.unitOf(text, 'spbSave', 'self');
  // (AI 20261001, WPF: "placed in front of other elements in the active container": the LAST child of spbSave's container,
  //  where: under spbSave)
  const tbKids = hb2.childrenOf(text, hb2.parentOf(text, tbSave.start)).map(k => (/\bid="([^"]+)"/.exec(k.text.slice(0, k.text.indexOf('>') + 1)) || [])[1] || '?');
  ok(tbAdd3 && /^Label\d+$/.test(tbAdd3.id) && !tbAdd3.into && tbAdd3.after === 'spbSave' && tbSave && tbKids[tbKids.length - 1] === tbAdd3.id && tbKids.indexOf('spbSave') >= 0,
    'toolbox with spbSave selected: a new label on top of its container (its last child, same parent as spbSave), a free LabelN name', JSON.stringify({ add: tbAdd3, kids: tbKids }));
  // 0.153 the new toolbox components (the WPF gap list G2): a PageControl (two tab sheets), a RadioGroup, a Memo into Panel2
  {
    await selAs('Panel2', 'div');
    const a1 = await fake._cmds['ht9045Designer.toolboxAdd']('TPageControl');
    await selAs('Panel2', 'div');
    const a2 = await fake._cmds['ht9045Designer.toolboxAdd']('TRadioGroup');
    await selAs('Panel2', 'div');
    const a3 = await fake._cmds['ht9045Designer.toolboxAdd']('TMemo');
    const p2 = (() => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; })();
    ok(a1 && a1.id === 'PageControl1' && a1.into === 'Panel2' && /<div class="pcWrap" id="PageControl1"[^>]*title="PageControl1 : TPageControl">.*title="PageControl1Sheet1 : TTabSheet".*class="pcPane" data-p="1"/.test(p2) &&
      a2 && a2.id === 'RadioGroup1' && /<fieldset class="gbx rg" id="RadioGroup1".*name="rg_RadioGroup1" checked>Item1/.test(p2) &&
      a3 && a3.id === 'Memo1' && /<textarea class="ed" id="Memo1" title="Memo1 : TMemo"/.test(p2),
      'toolbox 0.153: PageControl1 (two tab sheets), RadioGroup1 (two items), Memo1 into Panel2 with the generator\'s markup', JSON.stringify([a1, a2, a3]));
  }
  // the IO components from the toolbox (EastSun 20261001): MyLedLane1 / BtnPanelLane1 into Panel2, the generator's markup
  await selAs('Panel2', 'div');
  const tbLed = await fake._cmds['ht9045Designer.toolboxAdd']('TMyLedLane');
  await selAs('Panel2', 'div');
  const tbBp = await fake._cmds['ht9045Designer.toolboxAdd']('TBtnPanelLane');
  const tbP2b = (() => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; })();
  ok(tbLed && tbLed.id === 'MyLedLane1' && tbLed.into === 'Panel2' && tbBp && tbBp.id === 'BtnPanelLane1' &&
    /<span class="aled LEDHorizontal" id="MyLedLane1" style="position:absolute;[^"]*--led-on:#00ff00;[^"]*" title="MyLedLane1 : TMyLedLane｜Alias="><\/span>/.test(tbP2b) &&
    /<div class="btnpanel flat" id="BtnPanelLane1" [^>]*title="BtnPanelLane1 : TBtnPanelLane｜Alias=">BtnPanelLane1<\/div>/.test(tbP2b),
    'toolbox: an IO lamp (MyLedLane1) and an IO panel button (BtnPanelLane1) into Panel2, as the generator writes them, Alias to be picked',
    JSON.stringify({ tbLed, tbBp }));
  doc._setText(tbBefore);
  // 改名稱 (F2): spbSave is in the DFM -> asked first; closed = nothing; "改名稱" = renamed (id, title), selected under the new name
  await selAs('spbSave', 'button');
  const rnBefore = text;
  fake._answer = undefined;
  const rnNo = await fake._cmds['ht9045Designer.rename']('btnSave');
  const rnAsked = fake._calls.filter(c => c.name === 'warning').pop();
  ok(rnNo === null && text === rnBefore && rnAsked && /\.dfm 裡有「spbSave」/.test(rnAsked.args[0]), 'rename spbSave: in the DFM -> asked; closed = nothing changed');
  fake._answer = '改名稱';
  api.hub.applyDesignLocked(edD, ['spbSave']);
  const rnYes = await fake._cmds['ht9045Designer.rename']('btnSave');
  fake._answer = undefined;
  ok(rnYes && rnYes.to === 'btnSave' && rnYes.inDfm && /id="btnSave" title="btnSave : TSpeedButton/.test(text) && text.indexOf('id="spbSave"') < 0 && edD.lastSelId === 'btnSave' &&
    edD.designLocked.has('btnSave') && !edD.designLocked.has('spbSave'),
    'answered 改名稱: the source says btnSave (id + title), the new name is selected, its lock follows the name', JSON.stringify(rnYes));
  // a redraw for the scripts' edits before the HTML's: the page shows the form, said 'initRoot' -- btnSave still wanted
  // (the real-VS Code rename flaked on this: the form's init answer took the place of the new name)
  edD.onMessage({ __htd: 1, type: 'select', info: { key: 1, id: '@form', tag: 'div', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'initRoot' });
  const rnKeep = edD.lastSelId;
  edD.onMessage({ __htd: 1, type: 'select', info: { key: 1, id: '@form', tag: 'div', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'init' });
  const rnInit = edD.lastSelId;
  edD.lastSelId = 'btnSave';
  ok(rnKeep === 'btnSave' && rnInit === '@form', 'the page showing the form because the new name is not drawn yet (initRoot) keeps it wanted; a plain init answer is taken',
    'initRoot -> ' + rnKeep + ', init -> ' + rnInit);
  api.hub.applyDesignLocked(edD, []);
  doc._setText(rnBefore);
  await selAs('spbSave', 'button');
  const rnBad1 = await fake._cmds['ht9045Designer.rename']('9lives', true);
  const rnBad2 = await fake._cmds['ht9045Designer.rename']('sbtExit', true);
  ok(rnBad1 === null && rnBad2 === null && text === rnBefore, 'rename: "9lives" (not a name) and "sbtExit" (taken) refused, nothing changed');
  // 0.154 (WPF: renaming x:Name renames its uses): the page's JS that names spbSave -- a list, the scripts only this page
  // loads ticked, the shared ones not; what is ticked changes too (the fake's documents are in memory: nothing is written)
  {
    await selAs('spbSave', 'button');
    // (the rename above changed the in-memory hotplate wire to btnSave: back to the file's own text first)
    const wd0 = Object.values(fake._docs || {}).find(x => /ht9045_hotplate_wire\.js$/i.test(x.uri ? x.uri.fsPath : ''));
    if (wd0 && wd0._setText) wd0._setText(fs.readFileSync(wd0.uri.fsPath, 'utf8'));
    const qp0 = fake._state.quickPick;
    let shown = null;
    fake._state.quickPick = items => { shown = items; return items.filter(i => i.picked); };
    fake._answer = '改名稱';
    const rr = await fake._cmds['ht9045Designer.rename']('btnSave3');
    fake._answer = undefined;
    fake._state.quickPick = qp0;
    const labels = (shown || []).map(i => (i.picked ? '+' : '-') + i.label.replace(/^\$\([^)]*\) /, ''));
    const wireDoc = Object.values(fake._docs || {}).find(x => /ht9045_hotplate_wire\.js$/i.test(x.uri ? x.uri.fsPath : ''));
    const wireT = wireDoc ? wireDoc.getText() : '';
    const sharedPicked = (shown || []).some(i => i.picked && i.ref.pages > 1);
    const got = { rr, labels: labels.slice(0, 8), n: labels.length, wire: /btnSave3/.test(wireT) && !/['"]spbSave['"]/.test(wireT), sharedPicked };
    ok(!!(rr && rr.to === 'btnSave3' && rr.webEdited > 0 && rr.chosen.some(c => /ht9045_hotplate_wire\.js/.test(c)) && got.wire && !sharedPicked && rr.shared > 0 && /id="btnSave3"/.test(text)),
      'rename 0.154: the page\'s JS that names spbSave listed; only-this-page scripts ticked (ht9045_hotplate_wire.js changed to btnSave3), shared ones not ticked',
      JSON.stringify(got));
    if (wireDoc && wireDoc._setText) wireDoc._setText(wireT.split('btnSave3').join('spbSave'));
    doc._setText(rnBefore);
  }

  // WinForms / WPF placing: a click picks the tool (the page shows a crosshair), the page puts it down
  // in the first container under the point (at that point, with the dragged size); a quick second click adds at once
  pagePosted.length = 0;
  api.hub.lastArm = null;
  const tpArm = await fake._cmds['ht9045Designer.toolboxArm']('TSpeedButton');
  const tpMsg = pagePosted.filter(m => m.type === 'placeArm').pop();
  ok(tpArm && tpArm.armed === 'TSpeedButton' && tpMsg && tpMsg.cls === 'TSpeedButton' && edD.armed === 'TSpeedButton', 'toolbox click: the tool is picked, the page is told (crosshair)');
  // WPF Toolbox "Pointer": the tool put back, the page told (no crosshair), nothing added; once a tool is put
  // down (or given up) the toolbox's selection goes back to Pointer (a reveal, only when the view is showing)
  const tpText0 = text;
  const tpPtr = await fake._cmds['ht9045Designer.toolboxArm']('@pointer');
  const tpPtrMsg = pagePosted.filter(m => m.type === 'placeArm').pop();
  ok(tpPtr && tpPtr.pointer && tpPtr.had === true && !edD.armed && tpPtrMsg && tpPtrMsg.cls === null && text === tpText0 && api.hub.lastArm === null,
    'toolbox Pointer (WPF): the picked-up tool put back, the page told, nothing added', JSON.stringify(tpPtr));
  api.hub.toolboxView.selection = [tbItems[0]];
  const tpPtrEnter = await fake._cmds['ht9045Designer.toolboxAddSelected']();
  api.hub.toolboxEnter = 0;
  ok(tpPtrEnter && tpPtrEnter.pointer && text === tpText0, 'toolbox Enter on Pointer: nothing added (Pointer is not a component)', JSON.stringify(tpPtrEnter));
  const tpRevealed = [];
  const tpTv = api.hub.toolboxView, tpRev0 = tpTv.reveal, tpVis0 = tpTv.visible;
  tpTv.reveal = (el, o) => { tpRevealed.push({ cls: el && el.cls, select: o && o.select, focus: o && o.focus }); return Promise.resolve(); };
  tpTv.visible = false;
  edD.onMessage({ __htd: 1, type: 'placeCancel' });
  const tpHidden = tpRevealed.length;
  tpTv.visible = true;
  edD.onMessage({ __htd: 1, type: 'placeCancel' });
  ok(tpHidden === 0 && tpRevealed.length === 1 && tpRevealed[0].cls === '@pointer' && tpRevealed[0].select === true && tpRevealed[0].focus === false,
    'Esc on the surface: the toolbox back to Pointer (only when it is showing; the focus stays)', JSON.stringify(tpRevealed));
  await fake._cmds['ht9045Designer.toolboxArm']('TSpeedButton');
  const tpPlaced = await api.hub.cmdPlace(edD, { cls: 'TSpeedButton', w: 40, h: 20,
    targets: [{ id: 'spbSave', x: 3, y: 3 }, { id: 'Panel2', x: 57, y: 11 }, { id: '@form', x: 57, y: 486 }] });
  ok(tpRevealed.length === 2 && tpRevealed[1].cls === '@pointer', 'a tool put down: the toolbox back to Pointer (WPF)', JSON.stringify(tpRevealed));
  tpTv.reveal = tpRev0; tpTv.visible = tpVis0;
  const tpP2 = (() => { const t0 = he2.startTagOf(text, 'Panel2'); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; })();
  ok(tpPlaced && tpPlaced.into === 'Panel2' && new RegExp('id="' + tpPlaced.id + '"[^>]*left:57px;top:11px;width:40px;height:20px;').test(tpP2) && !edD.armed,
    'put down over spbSave: into Panel2 (spbSave is no container) at 57,11, 40x20', JSON.stringify(tpPlaced));
  doc._setText(tbBefore);
  const tpText = text;
  const tpPane = await api.hub.cmdPlace(edD, { cls: 'TLabel', w: 0, h: 0, targets: [{ id: null, pane: true }, { id: '@form', x: 5, y: 5 }] });
  ok(tpPane === null && text === tpText, 'put down in a tab sheet (no id): refused, not dropped on the form');
  api.hub.lastArm = null;
  await fake._cmds['ht9045Designer.toolboxArm']('TLabel');
  await selAs('Panel2', 'div');
  const tpDbl = await fake._cmds['ht9045Designer.toolboxArm']('TLabel');
  ok(tpDbl && /^Label\d+$/.test(tpDbl.id) && tpDbl.into === 'Panel2' && pagePosted.filter(m => m.type === 'placeArm').pop().cls === null,
    'a quick second click on the same tool: added at once (as before), the crosshair put away', JSON.stringify(tpDbl));
  doc._setText(tbBefore);
  // WPF: "select an element in the Toolbox and press Enter" = added at once; the select Enter makes (its click
  // command, pick-up) does not pick the tool up
  await selAs('Panel2', 'div');
  api.hub.lastArm = null;
  api.hub.toolboxView.selection = [require('../lib/toolbox').itemOf('TLabel')];
  const teCalls = fake._calls.length;
  const tpEnter = await fake._cmds['ht9045Designer.toolboxAddSelected']();
  const teArm = await fake._cmds['ht9045Designer.toolboxArm']('TLabel');
  ok(!!(tpEnter && /^Label\d+$/.test(tpEnter.id) && tpEnter.into === 'Panel2' && teArm === null && !edD.armed &&
    fake._calls.slice(teCalls).some(c => c.name === 'exec' && c.args[0] === 'list.select')),
    'toolbox Enter (WPF): the selected tool is added at once into Panel2; the select it makes does not pick the tool up', JSON.stringify(tpEnter));
  api.hub.toolboxEnter = 0;
  doc._setText(tbBefore);

  // WinForms / WPF: Ctrl+drag = a copy of the selection dropped there -- the originals stay, each copy right after its
  // original (same container) moved by the drop's dx / dy, new names, the copy selected; ONE edit
  const cdPos = (t, id) => { const tg = he2.startTagOf(t, id); const m = tg ? /left:(-?\d+)px;top:(-?\d+)px/.exec(tg.text) : null; return m ? [+m[1], +m[2]] : null; };
  const cdEnd = (t, id) => { const t0 = he2.startTagOf(t, id); const r = t0 ? hb2.elementRange(t, t0) : null; return r ? r[1] : -1; };
  const cdBefore = text;
  const cdP0 = cdPos(cdBefore, 'spbSave');
  const cd = await api.hub.cmdCopyDrop(edD, { ids: ['spbSave', 'sbtExit'], dx: 30, dy: -12 });
  const cdN1 = cd && cd.made[0], cdN2 = cd && cd.made[1];
  const cdQ = cdN1 ? cdPos(text, cdN1) : null;
  const cdAfter = cdN1 ? text.slice(cdEnd(text, 'spbSave'), (he2.startTagOf(text, cdN1) || {}).start || 0) : null;
  ok(!!(cd && cd.made.length === 2 && cdP0 && cdQ && cdQ[0] === cdP0[0] + 30 && cdQ[1] === cdP0[1] - 12 &&
    JSON.stringify(cdPos(text, 'spbSave')) === JSON.stringify(cdP0) && cdAfter !== null && !cdAfter.trim() &&
    cdN2 && he2.startTagOf(text, cdN2) && cdN1 !== 'spbSave' && edD.lastSelId === cdN1),
    'Ctrl+drag (WinForms / WPF): spbSave + sbtExit copied, each copy right after its original, 30 right / 12 up; the originals stay; the copy selected',
    JSON.stringify({ cd, p0: cdP0, q: cdQ }));
  doc._setText(cdBefore);
  const cdNone = await api.hub.cmdCopyDrop(edD, { ids: ['spbSave'], dx: 0, dy: 0 });
  const cdGone = await api.hub.cmdCopyDrop(edD, { ids: ['noSuchThing'], dx: 8, dy: 8 });
  ok(cdNone === null && cdGone === null && text === cdBefore, 'Ctrl+drag with no move / of a name not in the source: nothing copied');
  edD.onMessage({ __htd: 1, type: 'copyDrop', ids: ['spbSave'], dx: 16, dy: 0 });
  for (let i = 0; i < 40 && text === cdBefore; i++) await new Promise(r => setTimeout(r, 50));
  ok(text !== cdBefore && /id="spbSave_\d+"/.test(text), 'the design surface\'s copyDrop message: copied');
  doc._setText(cdBefore);
  // Blend reparent (let go with Alt): over the form, outside Panel2 -> spbSave moved out of Panel2 into the form, where it was
  // let go; over Panel2 (where it already is) -> only that move (a batch setLayout, nothing restructured); over no container -> nothing
  const bpBefore = text;
  const bpIn = (t, id, cont) => { const c0 = he2.startTagOf(t, cont); const rc = c0 ? hb2.elementRange(t, c0) : null; const e0 = he2.startTagOf(t, id); return !!(rc && e0 && e0.start > rc[0] && e0.start < rc[1]); };
  const bpWas = bpIn(bpBefore, 'spbSave', 'Panel2');
  const bpR = await api.hub.cmdReparentDrop(edD, { offs: [{ id: 'spbSave', dx: -5, dy: -6 }], targets: [{ id: '@form', x: 300, y: 250 }] });
  const bpPos = cdPos(text, 'spbSave');
  ok(!!(bpWas && bpR && bpR.into === '@form' && !bpIn(text, 'spbSave', 'Panel2') && !bpIn(text, 'spbSave', 'GroupBox1') && bpPos && bpPos[0] === 295 && bpPos[1] === 244),
    'Blend reparent (Alt at the release): spbSave out of Panel2 into the form, at the place it was let go (295, 244)', JSON.stringify({ bpR, bpPos }));
  doc._setText(bpBefore);
  pagePosted.length = 0;
  const bpSame = await api.hub.cmdReparentDrop(edD, { offs: [{ id: 'spbSave', dx: 0, dy: 0 }], targets: [{ id: 'Panel2', x: 70, y: 9 }, { id: '@form', x: 70, y: 490 }] });
  const bpSameMsg = pagePosted.filter(m => m.type === 'editMany').pop();
  const bpNone = await api.hub.cmdReparentDrop(edD, { offs: [{ id: 'spbSave', dx: 0, dy: 0 }], targets: [{ id: 'sbtExit', x: 1, y: 1 }] });
  ok(!!(bpSame && bpSame.same && text === bpBefore && bpSameMsg && bpSameMsg.items.length === 1 && bpSameMsg.items[0].id === 'spbSave' &&
    bpSameMsg.items[0].left === 70 && bpSameMsg.items[0].top === 9 && bpNone === null && text === bpBefore),
    'Blend reparent over its own Panel2: just the move (setLayout 70, 9); over no container: nothing', JSON.stringify({ bpSame, items: bpSameMsg && bpSameMsg.items }));
  // WPF keyboard shortcuts: Alt+arrow = duplicate (1 px that way, Shift = 10 px), Ctrl+N = create an element -- bound only
  // while the design surface has the focus (not the side bar / panel: Alt+Left there is VS Code's Go Back)
  await selAs('spbSave', 'button');
  const dpR = await fake._cmds['ht9045Designer.duplicate']({ dx: 10, dy: 0 });
  const dpQ = dpR && dpR.made[0] ? cdPos(text, dpR.made[0]) : null;
  const dpKbs = require('../package.json').contributes.keybindings;
  const dpKeys = ['alt+left', 'alt+right', 'alt+up', 'alt+down', 'shift+alt+left', 'shift+alt+right', 'shift+alt+up', 'shift+alt+down']
    .map(k => dpKbs.find(x => x.key === k && x.command === 'ht9045Designer.duplicate'));
  const dpN = dpKbs.find(x => x.key === 'ctrl+n' && x.command === 'ht9045Designer.addComponent');
  const dpWhen = k => !!k && /activeCustomEditorId == 'ht9045Designer\.editor'/.test(k.when) && /!sideBarFocus/.test(k.when) && /!panelFocus/.test(k.when);
  ok(!!(dpR && dpQ && cdP0 && dpQ[0] === cdP0[0] + 10 && dpQ[1] === cdP0[1] && dpKeys.every(dpWhen) && dpKeys[0].args.dx === -1 && dpKeys[3].args.dy === 1 &&
    dpKeys[5].args.dx === 10 && dpWhen(dpN)),
    'Alt+arrow (WPF Duplicate): a copy of spbSave 10 px right (Shift+Alt+Right); the 8 keys and Ctrl+N (create an element) only on the design surface',
    JSON.stringify({ made: dpR && dpR.made, q: dpQ }));
  // Blend keyboard shortcuts: Ctrl+Shift+1 / 2 / 9 = same width / height / size, Ctrl+= / Ctrl+- = a zoom step, Ctrl+0 / Ctrl+9 =
  // fit the selection, Ctrl+1 = actual size -- only while the design surface has the focus, not while typing in F2's box
  pagePosted.length = 0;
  await fake._cmds['ht9045Designer.zoomIn']();
  await fake._cmds['ht9045Designer.zoomOut']();
  const bkSteps = pagePosted.filter(m => m.type === 'zoomStep').map(m => m.dir).join(',');
  const bkWant = { 'ctrl+shift+1': 'ht9045Designer.align.width', 'ctrl+shift+2': 'ht9045Designer.align.height', 'ctrl+shift+9': 'ht9045Designer.align.size',
    'ctrl+=': 'ht9045Designer.zoomIn', 'ctrl+-': 'ht9045Designer.zoomOut', 'ctrl+0': 'ht9045Designer.zoomSelection', 'ctrl+9': 'ht9045Designer.zoomSelection',
    'ctrl+1': 'ht9045Designer.resetZoom' };
  const bkMiss = Object.keys(bkWant).filter(k => !dpKbs.some(x => x.key === k && x.command === bkWant[k] && dpWhen(x) && /!ht9045Designer\.textEditing/.test(x.when)));
  ok(bkSteps === '1,-1' && bkMiss.length === 0, 'Blend keys: Ctrl+Shift+1 / 2 / 9 same width / height / size, Ctrl+= / Ctrl+- zoom a step, Ctrl+0 / 9 fit the selection, Ctrl+1 100% (design surface only)',
    JSON.stringify({ bkSteps, bkMiss }));
  doc._setText(cdBefore);

  // WPF document outline drag and drop: the names travel in the DataTransfer, the drop moves them
  const dnd = fake._views['ht9045Designer.components'].dnd;
  const dnNode = id => Array.from(fake._tree.byKey.values()).find(n => n.id === id);
  const dnRange = id => { const t0 = he2.startTagOf(text, id); const r = t0 ? hb2.elementRange(text, t0) : null; return r ? text.slice(r[0], r[1]) : ''; };
  const dnBefore = text;
  const dnT1 = new fake.DataTransfer();
  dnd.handleDrag([dnNode('spbSave')], dnT1);
  const dn1 = await dnd.handleDrop(dnNode('GroupBox1'), dnT1);
  ok(dnd.dragMimeTypes[0] === dnd.dropMimeTypes[0] && dn1 && dn1.into === 'GroupBox1' && dnRange('GroupBox1').indexOf('id="spbSave"') > 0 &&
    dnRange('Panel2').indexOf('id="spbSave"') < 0 && text.split('id="spbSave"').length === 2 && edD.lastSelId === 'spbSave',
    'tree drag: spbSave dropped on GroupBox1 moves into it, out of Panel2, once; selected', JSON.stringify(dn1));
  doc._setText(dnBefore);
  const dnT2 = new fake.DataTransfer();
  dnd.handleDrag([dnNode('Panel2')], dnT2);
  const dn2 = await dnd.handleDrop(dnNode('spbSave'), dnT2);
  ok(dn2 === null && text === dnBefore, 'tree drag: Panel2 onto spbSave (inside it) is refused, nothing moves');
  const dnT3 = new fake.DataTransfer();
  dnd.handleDrag([dnNode('spbSave')], dnT3);
  const dn3 = await dnd.handleDrop(dnNode('XST1'), dnT3);
  const dnX = hb2.unitOf(text, 'XST1', 'parent');
  ok(dn3 && dn3.after === 'XST1' && dnX && text.slice(dnX.end).replace(/^\s+/, '').indexOf('<button class="btn3d" id="spbSave"') === 0 && dnRange('Panel2').indexOf('id="spbSave"') < 0,
    'tree drag onto XST1 (an edit, not a container): moved right after it, into its parent', JSON.stringify(dn3));
  doc._setText(dnBefore);
  // WPF: "Right-click the element in the Document Outline window or the artboard" -- the same commands; the row's
  // component is selected first (the page answers the selection here the way the probe does)
  await selAs('spbSave', 'button');
  const orPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m && m.type === 'selectKey') {
      const n = Array.from(fake._tree.byKey.values()).find(x => x.key === m.key);
      setTimeout(() => pageRecv({ __htd: 1, type: 'select', info: { key: m.key, id: n ? n.id : null, tag: 'div', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'tree' }), 20);
    }
    return orPM(m);
  };
  const orClip = await fake._cmds['ht9045Designer.copyComponent'](dnNode('Panel2'));
  const orSel = edD.sel && edD.sel.info ? edD.sel.info.id : null;
  const orSame = await fake._cmds['ht9045Designer.copyComponent'](dnNode('Panel2'));
  // the outline's own keys (F2 / Ctrl+C / X / V / Delete): the tree's selected row -- moved there with the arrow keys,
  // the page still has spbSave -- is the one acted on
  await selAs('spbSave', 'button');
  api.hub.treeView.selection = [dnNode('Panel2')];
  const okClip = await fake._cmds['ht9045Designer.outlineKey']({ cmd: 'copy' });
  const okSel = edD.sel && edD.sel.info ? edD.sel.info.id : null;
  api.hub.treeView.selection = [];
  panel.webview.postMessage = orPM;
  const okKeys = require('../package.json').contributes.keybindings.filter(k => k.command === 'ht9045Designer.outlineKey' && /focusedView == ht9045Designer\.components/.test(k.when));
  ok(!!(okClip && okClip.ids[0] === 'Panel2' && okSel === 'Panel2' &&
    ['delete', 'f2', 'ctrl+c', 'ctrl+x', 'ctrl+v'].every(key => okKeys.some(k => k.key === key))),
    'Document Outline keys (WPF): F2 / Ctrl+C / X / V / Delete bound in the tree; Ctrl+C on the tree\'s row (Panel2) copies Panel2, not the page\'s spbSave',
    JSON.stringify({ ids: okClip && okClip.ids, sel: okSel, keys: okKeys.map(k => k.key) }));
  ok(!!(orClip && orClip.ids[0] === 'Panel2' && orSel === 'Panel2' && orSame && orSame.ids[0] === 'Panel2'),
    'Document Outline right-click (WPF): Copy on the Panel2 row while spbSave is selected -> Panel2 selected and copied', JSON.stringify({ ids: orClip && orClip.ids, sel: orSel }));
  const orMenus = require('../package.json').contributes.menus['view/item/context'].filter(x => /ht9045Designer\.components/.test(x.when) && !/^inline/.test(x.group || ''));
  const orHas = c => orMenus.some(x => x.command === c || x.submenu === c);
  ok(['ht9045Designer.cutComponent', 'ht9045Designer.copyComponent', 'ht9045Designer.pasteComponent', 'ht9045Designer.deleteComponent', 'ht9045Designer.rename',
    'ht9045Designer.viewCode', 'ht9045Designer.orderMenu', 'ht9045Designer.groupIntoPanel', 'ht9045Designer.ungroup', 'ht9045Designer.layoutMenu'].every(orHas),
    'Document Outline right-click menu: cut / copy / paste / delete / rename, view code, Order, Group Into, Ungroup, Layout reset (like the design surface)', orMenus.length + ' items');
  // WPF Layout > Reset <property> / Reset All: the submenu on both right-click menus
  const lyMenus = require('../package.json').contributes.menus;
  const lyItems = (lyMenus['ht9045Designer.layoutMenu'] || []).map(x => x.command);
  ok(JSON.stringify(lyItems) === JSON.stringify(['ht9045Designer.resetLayout.pos', 'ht9045Designer.resetLayout.size', 'ht9045Designer.resetLayout.layout', 'ht9045Designer.resetToDfm']) &&
    lyMenus['webview/context'].some(x => x.submenu === 'ht9045Designer.layoutMenu') && !lyMenus['webview/context'].some(x => x.command === 'ht9045Designer.resetToDfm'),
    'Layout reset submenu (WPF): position / size / whole layout / everything, on the design surface and the outline', lyItems.join(','));

  // Group Into a new panel / Ungroup (the probe measures: here answered from the styles, borders 0)
  const grPM = panel.webview.postMessage;
  panel.webview.postMessage = m => {
    if (m.type === 'lookAll') {
      setTimeout(() => pageRecv({ __htd: 1, type: 'lookAll', seq: m.seq, items: (m.ids || []).map(id => {
        const u = hb2.unitOf(text, id, 'self');
        const p = u ? hb2.firstPx(u.html) : null;
        return { id, lay: p ? { left: p.left, top: p.top, width: p.width === null ? 50 : p.width, height: p.height === null ? 20 : p.height, cl: 0, ct: 0 } : null };
      }) }), 5);
    }
    return grPM(m);
  };
  const grBefore = text;
  const grPos = id => { const u = hb2.unitOf(text, id, 'self'); return u ? hb2.firstPx(u.html) : null; };
  const grS0 = grPos('spbSave'), grE0 = grPos('sbtExit');
  pageRecv({ __htd: 1, type: 'select', info: { key: 960, id: 'spbSave', multi: ['sbtExit'], tag: 'button', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'multi' });
  await new Promise(r => setTimeout(r, 30));
  const grp = await fake._cmds['ht9045Designer.groupIntoPanel']();
  const grIn = grp ? dnRange(grp.panel) : '';
  const grS1 = grPos('spbSave');
  ok(grp && grp.panel === 'Panel3' && grIn.indexOf('id="spbSave"') > 0 && grIn.indexOf('id="sbtExit"') > 0 && grp.box.left === Math.min(grS0.left, grE0.left) &&
    grS1.left === grS0.left - grp.box.left && grS1.top === grS0.top - grp.box.top && text.split('id="spbSave"').length === 2 && edD.lastSelId === 'Panel3',
    'group: spbSave + sbtExit into a new Panel3 at their bounding box, counting from it (nothing moves on screen)', JSON.stringify(grp));
  await selAs('Panel3', 'div');
  const ugr = await fake._cmds['ht9045Designer.ungroup']();
  const grS2 = grPos('spbSave'), grE2 = grPos('sbtExit');
  ok(ugr && ugr.removed === 'Panel3' && !/id="Panel3"/.test(text) && grS2.left === grS0.left && grS2.top === grS0.top && grE2.left === grE0.left && grE2.top === grE0.top &&
    dnRange('Panel2').indexOf('id="spbSave"') > 0, 'ungroup Panel3: both back in Panel2 at their old left/top, Panel3 gone', JSON.stringify(ugr));
  // 0.160 Group Into a GroupBox (WPF Group Into > ...): the box round them (8px, 16px for the caption), the children in
  // its .cli counting from inside its 2px border -- on the screen nothing moves
  {
    pageRecv({ __htd: 1, type: 'select', info: { key: 961, id: 'spbSave', multi: ['sbtExit'], tag: 'button', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'multi' });
    await new Promise(r => setTimeout(r, 30));
    const gb = await fake._cmds['ht9045Designer.groupInto']({ kind: 'TGroupBox' });
    const gbIn = gb ? dnRange(gb.panel) : '';
    const gS = grPos('spbSave');
    const scrOk = gb && gS && gb.box.left + 2 + gS.left === grS0.left && gb.box.top + 2 + gS.top === grS0.top;
    ok(!!(gb && gb.kind === 'TGroupBox' && /^GroupBox\d+$/.test(gb.panel) && /^<fieldset class="gbx" id="GroupBox\d+"[^>]*title="GroupBox\d+ : TGroupBox"><legend[^>]*>GroupBox\d+<\/legend><div class="cli"/.test(gbIn) &&
      gbIn.indexOf('id="spbSave"') > 0 && gbIn.indexOf('id="sbtExit"') > 0 && scrOk && edD.lastSelId === gb.panel),
      'group into a GroupBox (0.160): the generator\'s fieldset + legend + .cli, the two inside, their place on the screen kept (box + 2px border + own left/top)', JSON.stringify(gb));
    doc._setText(grBefore);
  }
  await selAs('spbSave', 'button');
  const ugNo = await fake._cmds['ht9045Designer.ungroup']();
  ok(ugNo === null, 'ungroup needs a Panel / GroupBox selected');
  panel.webview.postMessage = grPM;
  doc._setText(grBefore);

  // live data tag: main.html palMainStatus <- machine.state <- WebBridgeTags.cpp
  const MAINP = path.join(PORT, '..', 'web', 'page', 'main.html');
  const mtext = fs.readFileSync(MAINP, 'utf8');
  const mstarts = [0];
  for (let i = 0; i < mtext.length; i++) if (mtext.charCodeAt(i) === 10) mstarts.push(i + 1);
  const mdoc = {
    uri: fake.Uri.file(MAINP), fileName: MAINP, text: mtext, getText: () => mtext,
    positionAt: off => { let lo = 0, hi = mstarts.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (mstarts[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - mstarts[lo]); },
    offsetAt: p => mstarts[p.line] + p.character,
  };
  let mRecv = null;
  const mpanel = {
    active: true, visible: true, viewColumn: 1, reveal() {},
    onDidChangeViewState: () => ({ dispose() {} }), onDidDispose: () => ({ dispose() {} }),
    webview: {
      cspSource: 'https://*.vscode-cdn.net', options: null, html: '',
      asWebviewUri: u => ({ toString: () => 'https://file+.vscode-resource.vscode-cdn.net/' + u.fsPath.replace(/\\/g, '/') }),
      postMessage: () => Promise.resolve(true),
      onDidReceiveMessage: fn => { mRecv = fn; return { dispose() {} }; },
    },
  };
  fake._editor.resolveCustomTextEditor(mdoc, mpanel);
  mRecv({ __htd: 1, type: 'ready', url: 'vscode-webview://y/index.html' });
  mRecv({ __htd: 1, type: 'select', info: { key: 5, id: 'palMainStatus', tag: 'div', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'click' });
  let ms = null;
  for (let i = 0; i < 200 && !ms; i++) {
    await new Promise(r => setTimeout(r, 30));
    const last = posted.filter(m => m.type === 'show' && m.data).pop();
    if (last && last.data.comp.htmlId === 'palMainStatus' && !last.data.cppPending) ms = last.data;
  }
  const mt = ms && ms.tags && ms.tags.find(t => t.tag === 'machine.state');
  ok(mt && mt.port.some(i => /WebBridgeTags\.cpp$/i.test(ms.targets[i].file)), 'palMainStatus shows tag machine.state, published in WebBridgeTags.cpp',
    mt ? mt.port.map(i => path.basename(ms.targets[i].file) + ':' + ms.targets[i].line).join(', ') : (ms ? JSON.stringify(ms.tags) : 'no data'));
  // and back: lens above the C++ line that publishes it
  const wbtT = mt && mt.port.map(i => ms.targets[i]).find(t => /WebBridgeTags\.cpp$/i.test(t.file));
  const wbt = wbtT ? wbtT.file : null;
  ok(!!wbt, 'the publishing C++ file is known', wbt ? path.relative(PORT, wbt) : '-');
  if (wbt) {
    const wtext = fs.readFileSync(wbt, 'utf8');
    const wst = [0];
    for (let i = 0; i < wtext.length; i++) if (wtext.charCodeAt(i) === 10) wst.push(i + 1);
    const wdoc = { uri: fake.Uri.file(wbt), getText: () => wtext, positionAt: off => { let lo = 0, hi = wst.length - 1; while (lo < hi) { const m = (lo + hi + 1) >> 1; if (wst[m] <= off) lo = m; else hi = m - 1; } return new fake.Position(lo, off - wst[lo]); } };
    const tl = fake._lens.provideCodeLenses(wdoc).find(l => /網頁顯示 machine\.state/.test(l.command.title));
    ok(!!tl && /palMainStatus/.test(tl.command.title), 'CodeLens in WebBridgeTags.cpp: 網頁顯示 machine.state → main.html › palMainStatus',
      tl ? tl.command.title + ' @' + (tl.range.start.line + 1) : '-');
  }

  // form root: OnShow -> FormShow
  pageRecv({ __htd: 1, type: 'select', info: { key: 1, id: '@form', tag: 'div', title: '', attrs: [], style: [], inline: [], computed: [], chain: [], listeners: [], inherited: [] }, origin: 'click' });
  let fr = null;
  for (let i = 0; i < 100 && !fr; i++) {
    await new Promise(r => setTimeout(r, 30));
    const last = posted.filter(m => m.type === 'show' && m.data).pop();
    if (last && last.data.comp.isForm && !last.data.cppPending) fr = last.data;
  }
  const show = fr && fr.events.find(e => e.name === 'OnShow');
  ok(show && show.handler === 'FormShow' && show.targets.some(i => fr.targets[i].kind === 'golden'), 'form root: OnShow -> FormShow with a BCB6 target',
    show ? show.targets.map(i => fr.targets[i].kind + ' ' + path.basename(fr.targets[i].file) + ':' + fr.targets[i].line).join(' | ') : '');
  ok(!fake._calls.some(c => c.name === 'error'), 'no error messages shown', fake._calls.filter(c => c.name === 'error').map(c => c.args[0]).join(' / '));

  // --- WPF-style edits written into the source (in memory; nothing is saved) ---
  ok(full.edit && full.edit.layout && full.edit.layout.target === 'self' && full.edit.inSource && full.edit.layoutInSource &&
    full.edit.caption && full.edit.caption.kind === 'text' && full.edit.caption.value === 'Save',
    'panel data: spbSave is editable (layout self, caption "Save")', full.edit ? JSON.stringify({ l: full.edit.layout && full.edit.layout.raw, c: full.edit.caption }) : '-');
  const tagNow = id => require('../lib/htmledit').startTagOf(text, id).text;
  const wait = ms => new Promise(r => setTimeout(r, ms));
  pageRecv({ __htd: 1, type: 'edit', what: 'style', id: 'spbSave', target: 'self', style: { left: '74px', top: '18px' } });
  await wait(50);
  ok(/left:74px;top:18px;width:227px/.test(tagNow('spbSave')) && doc.isDirty, 'drag edit -> <button id="spbSave"> style left/top rewritten, document dirty',
    tagNow('spbSave').slice(tagNow('spbSave').indexOf('style'), tagNow('spbSave').indexOf('style') + 60));
  // an IO lamp / panel button's LEDStyle / Value / Down / Style is a class: written IN PLACE (the attribute stays where it is)
  const clsBefore = tagNow('spbSave');
  pageRecv({ __htd: 1, type: 'edit', what: 'class', id: 'spbSave', target: 'self', add: ['down'], remove: [] });
  await wait(50);
  const clsAdded = tagNow('spbSave');
  pageRecv({ __htd: 1, type: 'edit', what: 'class', id: 'spbSave', target: 'self', add: [], remove: ['down'] });
  await wait(50);
  const clsBack = tagNow('spbSave');
  const clsAt = s => s.indexOf('class=');
  ok(/class="[^"]*\bdown\b[^"]*"/.test(clsAdded) && clsAt(clsAdded) === clsAt(clsBefore) && clsBack === clsBefore,
    'a class edit (IO Down / LEDStyle / Style) -> the class attribute changed in place, taken out again = the tag as it was',
    clsAdded.slice(0, 80));
  pageRecv({ __htd: 1, type: 'edit', what: 'style', id: 'XST1', target: 'parent', style: { left: '136px' } });
  await wait(50);
  const xs1 = require('../lib/htmledit').startTagOf(text, 'XST1');
  const wr1 = require('../lib/htmledit').wrapperTagOf(text, xs1.start);
  ok(/left:136px;/.test(wr1.text) && !/left:136px/.test(xs1.text), 'wrapper edit -> the <span> around XST1 moved, the input untouched', wr1.text);
  pageRecv({ __htd: 1, type: 'edit', what: 'caption', id: 'spbSave', capKind: 'text', value: 'Save <2>' });
  pageRecv({ __htd: 1, type: 'edit', what: 'caption', id: 'GroupBox1', capKind: 'legend', value: 'Plates' });
  pageRecv({ __htd: 1, type: 'edit', what: 'caption', id: 'XST1', capKind: 'value', value: '1.5' });
  pageRecv({ __htd: 1, type: 'edit', what: 'attr', id: 'XST1', attr: { disabled: true } });
  await wait(100);
  ok(/>Save &lt;2&gt;<\/button>/.test(text), 'caption edit -> button text, HTML-escaped');
  ok(/<legend[^>]*>Plates<\/legend>/.test(text), 'caption edit -> GroupBox legend');
  ok(/id="XST1"[^>]*value="1\.5"[^>]*disabled>/.test(text), 'value + disabled edits on <input id="XST1"> (queued, applied in order)', tagNow('XST1').slice(-60));
  // a group drag / align: several tags in ONE WorkspaceEdit (one undo step)
  const callsBefore = fake._calls.filter(c => c.name === 'applyEditCall').length;
  pageRecv({ __htd: 1, type: 'edit', what: 'batch', edits: [
    { what: 'style', id: 'spbSave', target: 'self', style: { top: '9px' } },
    { what: 'style', id: 'sbtExit', target: 'self', style: { top: '9px' } },
    { what: 'style', id: 'Panel2', target: 'self', style: { top: '475px' } },   // unchanged: not written
  ] });
  await wait(50);
  const calls = fake._calls.filter(c => c.name === 'applyEditCall').slice(callsBefore);
  ok(calls.length === 1 && calls[0].args[0] === 2 && /id="spbSave"[^>]*top:9px;/.test(text) && /id="sbtExit"[^>]*top:9px;/.test(text),
    'batch edit (align top) -> 2 changed tags in ONE WorkspaceEdit, the unchanged one skipped', JSON.stringify(calls.map(c => c.args[0])));
  // 改回 DFM sends Left and Font.Bold of ONE button: both land in its one start tag (not "overlapping, nothing written")
  const sameBefore = fake._calls.filter(c => c.name === 'applyEditCall').length;
  const sameText0 = text;
  pageRecv({ __htd: 1, type: 'edit', what: 'batch', edits: [
    { what: 'style', id: 'spbSave', target: 'self', style: { left: '61px' } },
    { what: 'style', id: 'spbSave', target: 'self', style: { 'font-weight': 'normal' } },
    { what: 'attr', id: 'spbSave', attr: { 'data-x': '1' } },
  ] });
  await wait(50);
  const sameCalls = fake._calls.filter(c => c.name === 'applyEditCall').slice(sameBefore);
  ok(sameCalls.length === 1 && sameCalls[0].args[0] === 1 && /id="spbSave"[^>]*left:61px;/.test(text) && /id="spbSave"[^>]*font-weight:normal/.test(text) &&
    /id="spbSave"[^>]*data-x="1"/.test(text),
    'batch with three changes of ONE tag (style, style, attribute) -> one edit of that tag, all three in it', tagNow('spbSave').slice(0, 160));
  doc._setText(sameText0);
  const textBefore = text;
  const refusedBefore = fake._calls.length;
  pageRecv({ __htd: 1, type: 'edit', what: 'style', id: 'noSuchControl', target: 'self', style: { left: '1px' } });
  await wait(50);
  ok(text === textBefore, 'an id that is not in the source is refused, text untouched');
  // everything outside the edited tags is byte-identical to the file on disk
  const strip = s => s.replace(/<button class="btn3d" id="spbSave"[^>]*>[\s\S]*?<\/button>/, '')
    .replace(/<button class="btn3d exitbtn" id="sbtExit"[^>]*>/, '')
    .replace(/<legend[^>]*>(Plate of Use|Plates)<\/legend>/, '')
    .replace(/<span style="position:absolute;left:1\d\dpx;top:58px;width:61px;height:24px;"><input class="ed" id="XST1"[^>]*>/, '');
  ok(strip(text) === strip(ORIGINAL), 'nothing else in the page changed');
  ok(!fake._calls.slice(refusedBefore).some(c => c.name === 'error'), 'refusal is a status message, not an error dialog');

  // 專案搜尋 (AI 20261001, 0.137; EastSun: "我沒辦法整個專案查詢關鍵字並列出來"): the command -> the tree (area -> file ->
  // line) -> a click opens the line; a golden hit opens read-only, decoded from Big5; the results to the clipboard
  {
    const psv = fake._views && fake._views['ht9045Designer.projectSearch'];
    const res = await fake._cmds['ht9045Designer.projectSearchRun']('spbSave');
    const prov = api.hub.projSearch;
    const areas = prov.getChildren();
    const gArea = areas.find(a => a.r.area === 'golden'), pArea = areas.find(a => a.r.area === 'port'), wArea = areas.find(a => a.r.area === 'web');
    const gFile = gArea && prov.getChildren(gArea).find(f => /cHotPlate\.cpp$/i.test(f.file));
    const gHit = gFile && prov.getChildren(gFile).find(n => n.h.line === 440);
    const gItem = gHit && prov.getTreeItem(gHit);
    const op0 = fake._state.opened.length;
    if (gItem) await fake._cmds[gItem.command.command](...gItem.command.arguments);
    const gOpen = fake._state.opened[op0];
    const wFile = wArea && prov.getChildren(wArea).find(f => /Setup\.HotPlate\.html$/i.test(f.file));
    const wHit = wFile && prov.getChildren(wFile)[0];
    if (wHit) await fake._cmds['ht9045Designer.projectSearchOpen'](wHit.h);
    const wOpen = fake._state.opened[op0 + 1];
    const copied = await fake._cmds['ht9045Designer.projectSearchCopy']();
    // whole word: spbSaveClick no longer counted
    await fake._cmds['ht9045Designer.projectSearchWord']();
    const wwRes = prov.res;
    await fake._cmds['ht9045Designer.projectSearchWord']();
    const lab = gItem && gItem.label;
    const got = {
      view: !!psv, hits: res && res.hits.length, areas: areas.map(a => a.r.area + ':' + a.hits.length).join(','),
      gItem: lab ? (typeof lab === 'object' ? lab.label + ' ' + JSON.stringify(lab.highlights) : lab) : null,
      gOpen: gOpen ? gOpen.uri.scheme + ' ' + path.basename(gOpen.uri.fsPath) + ' L' + (gOpen.selection && gOpen.selection.start.line + 1) + ' ro=' + gOpen.readonly : null,
      wOpen: wOpen ? wOpen.uri.scheme + ' ' + path.basename(wOpen.uri.fsPath) + ' L' + (wOpen.selection && wOpen.selection.start.line + 1) : null,
      copied: copied ? copied.split('\n').length : 0, clip: fake._state.clip === copied,
      ww: wwRes && res ? wwRes.hits.length < res.hits.length && !wwRes.hits.some(h => /spbSave\w/.test(h.text.slice(h.col - 1, h.col + 7))) : false,
      desc: psv && psv.view ? String(psv.view.description || '') : '',
    };
    ok(!!(got.view && got.hits > 50 && gArea && pArea && wArea && gHit && lab && typeof lab === 'object' && lab.highlights && lab.highlights.length === 1 &&
      gOpen && gOpen.uri.scheme === 'ht9045-golden' && /cHotPlate\.cpp$/i.test(gOpen.uri.fsPath) && gOpen.selection.start.line === 439 && gOpen.readonly &&
      wOpen && wOpen.uri.scheme === 'file' && /Setup\.HotPlate\.html$/.test(wOpen.uri.fsPath) && got.copied > got.hits && got.clip && got.ww && /spbSave/.test(got.desc)),
      '專案搜尋: spbSave over C++ / web / BCB6 -> the tree by area, file, line (the keyword highlighted); a golden hit opens read-only (Big5) at that line, a web one as the file; the list copied; whole word narrows it',
      JSON.stringify(got));
    // from the code, with a scope (VS "Look in"): the golden cHotPlate.cpp alone / its project (golden) / everything
    const scF = gFile ? await fake._cmds['ht9045Designer.projectSearchHere']({ q: 'spbSave', scope: 'file', file: gFile.file }) : null;
    const scFfiles = scF ? Array.from(new Set(scF.hits.map(h => h.file))) : [];
    const scP = gFile ? await fake._cmds['ht9045Designer.projectSearchHere']({ q: 'spbSave', scope: 'project', file: gFile.file }) : null;
    const scPareas = scP ? Array.from(new Set(scP.hits.map(h => h.area))) : [];
    const scDesc = psv && psv.view ? String(psv.view.description || '') : '';
    const scS = await fake._cmds['ht9045Designer.projectSearchHere']({ q: 'spbSave', scope: 'solution', file: gFile && gFile.file });
    ok(!!(scF && scF.hits.length > 0 && scFfiles.length === 1 && scFfiles[0] === gFile.file && scP && scPareas.join() === 'golden' && scP.hits.length > scF.hits.length &&
      /在 /.test(scDesc) && scS && scS.hits.length === res.hits.length),
      '專案搜尋 from the code, VS "Look in": the current file only / its project only (golden) / the entire solution (= the plain search); the list says where',
      JSON.stringify({ file: scF && scF.hits.length, files: scFfiles.length, project: scP && scP.hits.length, areas: scPareas, desc: scDesc, solution: scS && scS.hits.length }));
  }

  // 方案總管 (AI 20261001, 0.138; EastSun's screenshot of Visual Studio's Solution Explorer): 方案 -> 3 projects ->
  // folders first; 搜尋方案總管 keeps the matching files with their folders, all expanded; sync with the active document;
  // a golden file opens read-only (Big5)
  {
    const sv = api.hub.solPanel ? { view: api.hub.solPanel.adapter } : null;   // (0.148: a webview view; its title line is the adapter's)
    const sol = api.hub.solution;
    // (0.139: the page searches above also searched 方案總管 -- the same search now; start from none)
    await fake._cmds['ht9045Designer.solutionClearFilter']();
    const [sln] = await sol.getChildren();
    const slnItem = sol.getTreeItem(sln);
    const projs = await sol.getChildren(sln);
    const port = projs.find(p => p.p.area === 'port'), gold = projs.find(p => p.p.area === 'golden');
    const portKids = port ? await sol.getChildren(port) : [];
    const firstFile = portKids.findIndex(k => k.type === 'file'), lastDir = portKids.map(k => k.type).lastIndexOf('dir');
    const fr = await fake._cmds['ht9045Designer.solutionFilter']('fHotPlate');
    const fProjs = await sol.getChildren(sln);
    const fPort = fProjs.find(p => p.p.area === 'port');
    const fPortKids = fPort ? await sol.getChildren(fPort) : [];
    const fForms = fPortKids.find(k => k.type === 'dir' && /^forms$/i.test(k.name));
    const fFormsKids = fForms ? await sol.getChildren(fForms) : [];
    const fFormsItem = fForms ? sol.getTreeItem(fForms) : null;
    const desc = sv && sv.view ? String(sv.view.description || '') : '';
    await fake._cmds['ht9045Designer.solutionClearFilter']();
    const after = port ? (await sol.getChildren((await sol.getChildren((await sol.getChildren())[0])).find(p => p.p.area === 'port'))).length : 0;
    // sync with the active document: the port's forms\fHotPlate.cpp (0.148: no BCB6 tree in 方案總管)
    const gf = port ? path.join(port.p.root, 'forms', 'fHotPlate.cpp') : '';
    const rv = gf ? await fake._cmds['ht9045Designer.solutionReveal'](gf) : null;
    const rvChain = []; for (let n = rv; n; n = sol.getParent(n)) rvChain.unshift(n.type + ':' + (n.name || ''));
    const op0 = fake._state.opened.length;
    if (rv) await fake._cmds['ht9045Designer.solutionOpen'](rv);
    const op = fake._state.opened[op0];
    const got = {
      view: !!sv, sln: slnItem.label, projs: projs.map(p => p.p.area).join(','), order: firstFile > lastDir, kids: portKids.length,
      filter: fr && fr.count, fProjs: fProjs.map(p => p.p.area).join(','), fForms: fFormsKids.map(k => k.name).join(','), expanded: fFormsItem && fFormsItem.collapsibleState === 2, desc,
      cleared: after === portKids.length, chain: rvChain.join(' > '), open: op ? op.uri.scheme + ' ' + path.basename(op.uri.fsPath) + ' ro=' + op.readonly : null,
    };
    // 0.139: 頁面 and 搜尋頁面 are in 方案總管 -- 網頁's first node; the search finds pages and their components
    const wProj = (await sol.getChildren((await sol.getChildren())[0])).find(p => p.p.area === 'web');
    const wKids = wProj ? await sol.getChildren(wProj) : [];
    const pgRoot = wKids[0] && wKids[0].type === 'pgroot' ? wKids[0] : null;
    const pgGroups = pgRoot ? await sol.getChildren(pgRoot) : [];
    await fake._cmds['ht9045Designer.solutionFilter']('spbSave');
    const wProj2 = (await sol.getChildren((await sol.getChildren())[0])).find(p => p.p.area === 'web');
    const pgRoot2 = wProj2 ? (await sol.getChildren(wProj2)).find(k => k.type === 'pgroot') : null;
    let hpPage = null, hpHit = null;
    for (const g of pgRoot2 ? await sol.getChildren(pgRoot2) : []) {
      for (const pg of await sol.getChildren(g)) {
        if (pg.inner.kind === 'page' && /Setup\.HotPlate\.html$/i.test(pg.inner.file)) { hpPage = pg; hpHit = await sol.getChildren(pg); }
      }
    }
    // (1005, EastSun: "可以不要顯示元件了嗎? 因為下方就有元件 列表可以看了": the page found by its component's name is listed,
    //  with no component rows under it, nothing to open, no "N 個元件" in its line)
    const hpItem = hpPage ? sol.getTreeItem(hpPage) : null;
    const noComps = !!(hpHit && hpHit.length === 0 && hpItem && hpItem.collapsibleState === fake.TreeItemCollapsibleState.None && !/個元件/.test(String(hpItem.description || '')));
    const hitItem = null;
    const pgDesc = sv && sv.view ? String(sv.view.description || '') : '';
    await fake._cmds['ht9045Designer.solutionClearFilter']();
    const pgGot = { pgRoot: !!pgRoot, groups: pgGroups.length, first: pgGroups[0] ? sol.getTreeItem(pgGroups[0]).label : '', hp: !!hpPage,
      noComps, hpLine: hpItem ? String(hpItem.description || '') : null, desc: pgDesc,
      oldHidden: (arrPkg.contributes.views['ht9045-designer'].find(v => v.id === 'ht9045Designer.pages') || {}).when === 'false' &&
        (arrPkg.contributes.views['ht9045-designer'].find(v => v.id === 'ht9045Designer.pageSearch') || {}).when === 'false',
      findPanel: !!(arrPkg.contributes.views['ht9045-find'] || []).find(v => v.id === 'ht9045Designer.projectSearch') };
    ok(!!(pgGot.pgRoot && pgGot.groups > 3 && pgGot.hp && pgGot.noComps && /頁/.test(pgGot.desc) && !/個元件/.test(pgGot.desc) && pgGot.oldHidden && pgGot.findPanel),
      '方案總管 0.139: 網頁 -> 頁面 (the old 頁面 list, by screen); its search finds the pages -- 1005: a page found by a component name (spbSave on Setup.HotPlate) without its components (the 元件 list shows them); the old 頁面 / 搜尋頁面 views hidden; 專案搜尋 in the bottom panel',
      JSON.stringify(pgGot));
    // (the report line is cut at the quote of 方案 'x': the whole of it in a file too)
    try { fs.writeFileSync(path.join(require('os').tmpdir(), 'htd_smoke_solution.json'), JSON.stringify(got, null, 1), 'utf8'); } catch (e) { /* only for a look */ }
    ok(!!(got.view && /^方案 '.+'（2 個專案）$/.test(got.sln) && got.projs === 'port,web' && !gold && got.order && got.kids > 20 &&
      got.filter > 0 && /port/.test(got.fProjs) && /fHotPlate\.h/.test(got.fForms) && /fHotPlate\.cpp/.test(got.fForms) && !/fMain/.test(got.fForms) && got.expanded && /fHotPlate/.test(got.desc) &&
      got.cleared && got.chain === 'sln: > proj: > dir:forms > file:fHotPlate.cpp' && /^file fHotPlate\.cpp/.test(got.open || '')),
      '方案總管: 方案 (2 projects: C++ / web -- 0.148 no BCB6) -> folders before files; 搜尋 fHotPlate = only those files with their folders, expanded; cleared = all again; sync with the active document selects forms\\fHotPlate.cpp, it opens',
      JSON.stringify(got));
  }
}

main().catch(e => { fail++; log('CRASH ' + (e && e.stack || e)); }).then(() => {
  log('');
  log('RESULT: ' + pass + ' pass, ' + fail + ' fail');
  const text = out.join('\n') + '\n';
  if (process.argv[3]) fs.writeFileSync(process.argv[3], text, 'utf8'); else process.stdout.write(text);
  process.exit(fail ? 1 : 0);
});
