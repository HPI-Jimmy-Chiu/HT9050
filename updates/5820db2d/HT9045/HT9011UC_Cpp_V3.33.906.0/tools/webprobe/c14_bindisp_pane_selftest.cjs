'use strict';
// =============================================================================
//  tools/webprobe/c14_bindisp_pane_selftest.cjs -- ctest C14_BinDispPanePage.  AI(W906-ST02-C14) 20261002 (St02-E helper).
//
//  web/page/ht9045_bindisp_status.js = Status.ShowBinSelect.html's "Bin Display Status" tab (golden 906_0625_Steven
//  TfShowBinSelect tsUnloadMap, cShowBinSelect.dfm:1361-2790; ChangeBinDispStatus cShowBinSelect.cpp:208-386), drawn from the
//  C++ tags binsel.disp.* (HT9011UC_Cpp_V3.33.906.0/WebBinDispStatus_St02.cpp).  Offline, in a node vm with a tiny fake DOM:
//    1. offline (no HT9045Tags): nothing throws, the grid is built from the dfm layout (36 cells, 33 bin lists), every value "---".
//    2. model(): the TColor -> CSS map (golden 906 ColorMap :218: gray, red, green, orange 0x000080FF -> #FF8000, black; clBtnFace),
//       captions, wrong lengths = unknown, bin.visible + groups -> the group boxes / panels, tabVisible.
//    3. render() with live tags: cell text / background, the status bar, the hidden tab (FormShow :760-764) leaves to Test Bin.
//    4. golden :272's jump: jumpSeq up -> the tab is clicked; not on the first value, not when already shown, not when hidden.
//    5. page pins: Status.ShowBinSelect.html loads the js exactly once, still 147 lines, the pane has #bdMap / #sbRunStatus; no
//       BOM, no mixed CRLF / LF in either file.
//  argv[2] = web/page.  CONTROL: W906_C14_PANE_JS pointing at a file without the renderer (e.g. an empty .js) must be red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const jsFile = process.env.W906_C14_PANE_JS || path.join(pageDir, 'ht9045_bindisp_status.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }

// ---- a tiny DOM: enough for document.getElementById / createElement / querySelector('#PageControl1 > .tab[data-tab="x"]') ----
function makeDom() {
  const byId = {};
  function El(tag) {
    this.tagName = String(tag).toUpperCase(); this.children = []; this.style = {}; this.title = ''; this.className = '';
    this._text = ''; this._id = ''; this.clicks = 0;
    const self = this;
    this.classList = {
      contains(c) { return (' ' + self.className + ' ').indexOf(' ' + c + ' ') >= 0; },
      add(c) { if (!this.contains(c)) self.className = (self.className + ' ' + c).trim(); },
      remove(c) { self.className = (' ' + self.className + ' ').replace(' ' + c + ' ', ' ').trim(); }
    };
  }
  Object.defineProperty(El.prototype, 'id', { get() { return this._id; }, set(v) { this._id = v; if (v) byId[v] = this; } });
  Object.defineProperty(El.prototype, 'textContent', {
    get() { return this._text + this.children.map(c => c.textContent).join(''); },
    set(v) { this.children.forEach(c => { if (c._id && byId[c._id] === c) delete byId[c._id]; }); this.children = []; this._text = String(v); }
  });
  El.prototype.appendChild = function (c) { this.children.push(c); return c; };
  El.prototype.click = function () { this.clicks++; if (dom.onClick) dom.onClick(this); };
  const dom = { byId, El, onClick: null };
  const tabs = {};
  ['testbin', 'catinfo', 'uph', 'bindisp', 'index'].forEach(n => { const t = new El('span'); t.className = 'tab' + (n === 'testbin' ? ' act' : ''); t.dataTab = n; tabs[n] = t; });
  dom.tabs = tabs;
  dom.onClick = (t) => { if (t.dataTab) { Object.keys(tabs).forEach(k => tabs[k].classList.remove('act')); t.classList.add('act'); } };   // tabs.js
  const map = new El('div'); map.id = 'bdMap'; map.textContent = '---';
  const sb = new El('div'); sb.id = 'sbRunStatus'; sb.textContent = '---';
  const head = new El('head'); dom.head = head;   // St01 1003 05:44: section 1 :119 reads dom.head (TypeError on node v22.20.0 without it)
  dom.document = {
    readyState: 'complete', head,
    addEventListener() {},
    getElementById(id) { return byId[id] || null; },
    createElement(t) { return new El(t); },
    querySelector(sel) { const m = /data-tab="([^"]+)"/.exec(sel); return m ? (tabs[m[1]] || null) : null; }
  };
  return dom;
}

function load(tags) {
  const dom = makeDom();
  const subs = [];
  const sb = { console: { log() {}, info() {}, warn() {}, error() {} }, document: dom.document, Promise };
  sb.window = sb;
  if (tags) {
    sb.HT9045Tags = {
      has(k) { return k in tags; }, get(k) { return (k in tags) ? tags[k] : null; },
      subscribe(fn) { subs.push(fn); return () => {}; }, connect() { return Promise.resolve(true); }
    };
  }
  vm.createContext(sb);
  let err = null;
  try { vm.runInContext(fs.readFileSync(jsFile, 'utf8'), sb, { filename: path.basename(jsFile) }); } catch (e) { err = e; }
  const push = (changed) => { Object.assign(tags, changed); subs.forEach(f => f(changed, tags)); };
  return { sb, dom, err, api: sb.HT9045BinDispStatus, push };
}

const DISP = ['Loader', 'Empty', 'Color', 'Auto1', 'Auto2', 'Auto3', 'Fix1', 'Fix2', 'Fix3', 'Fix4', 'Fix5', 'Fix6', 'BinBox',
              'Mag1', 'Mag2', 'Mag3', 'Mag4', 'Mag5', 'Mag6', 'Mag7', 'Mag8', 'Mag9', 'Mag10', 'Mag11', 'Mag12', 'Mag13', 'Mag14',
              'Auto4', 'Auto5', 'Auto6', 'Fix7', 'Fix8', 'Fix9', 'Fix10', 'Fix11', 'Fix12'];
const TRAY = ['Auto1', 'Auto2', 'Auto3', 'Auto4', 'Auto5', 'Auto6', 'Fix1', 'Fix2', 'Fix3', 'Fix4', 'Fix5', 'Fix6', 'Fix7', 'Fix8', 'Fix9',
              'Fix10', 'Fix11', 'Fix12', 'BinBox', 'Mag1', 'Mag2', 'Mag3', 'Mag4', 'Mag5', 'Mag6', 'Mag7', 'Mag8', 'Mag9', 'Mag10', 'Mag11',
              'Mag12', 'Mag13', 'Mag14'];
const GRAY = 0x00808080, RED = 0x000000FF, GREEN = 0x00008000, ORANGE = 0x000080FF, BLACK = 0, BTNFACE = 0x8000000F;
function liveTags() {
  const cap = DISP.map(() => 'X'), col = DISP.map(() => GRAY);
  cap[0] = 'L'; col[0] = ORANGE; cap[3] = '1'; col[3] = GREEN; cap[4] = '5'; col[4] = RED; cap[5] = '7'; col[5] = BLACK;
  const inst = DISP.map((n, i) => (i < 12 ? '1' : '0')).join(''), err = DISP.map((n, i) => (i === 5 ? '1' : '0')).join('');
  const lbl = TRAY.map(() => ''), lcol = TRAY.map(() => 0);
  lbl[1] = '1 2'; lcol[1] = GREEN;
  return {
    'binsel.disp.panelType': 3, 'binsel.disp.tabVisible': true, 'binsel.disp.ctrl': true,
    'binsel.disp.caption': cap.join('\t'), 'binsel.disp.color': col.join(','),
    'binsel.disp.inst': inst, 'binsel.disp.err': err,
    'binsel.disp.colorNow': DISP.map(() => 1).join(','), 'binsel.disp.binNow': DISP.map(() => 0).join(','),
    'binsel.disp.lbl': lbl.join('\t'), 'binsel.disp.lblColor': lcol.join(','), 'binsel.disp.groups': '101',
    'binsel.disp.status': 'Bin display got error!!', 'binsel.disp.statusColor': RED, 'binsel.disp.jumpSeq': 0,
    'bin.visible': TRAY.map((n, i) => (i === 5 ? '0' : '1')).join('')
  };
}

console.log('-- 1. offline: no HT9045Tags');
{
  const v = load(null);
  check(!v.err && v.api && typeof v.api.model === 'function', 'the file runs and exports HT9045BinDispStatus.model' + (v.err ? ' (' + v.err.message + ')' : ''));
  const ids = Object.keys(v.dom.byId);
  check(DISP.every(n => ids.indexOf('pnl' + n) >= 0), 'the 36 golden cells pnlLoader..pnlFix12 are built');
  check(TRAY.every(n => ids.indexOf('lbl' + n) >= 0) && ['Loader', 'Empty', 'Color'].every(n => ids.indexOf('lbl' + n) < 0),
        '33 bin lists lblAuto1..lblMag14, none under Loader / Empty / Color (dfm)');
  check(['pnlLoad', 'pnlFix123', 'pnlFix789', 'pnlAuto123', 'pnlAuto456', 'pnlMag123'].every(n => ids.indexOf(n) >= 0), 'the six dfm panels');
  check(v.dom.byId.pnlFix1.textContent === '---' && !v.dom.byId.pnlFix1.style.background && v.dom.byId.sbRunStatus.textContent === '---',
        'unknown = "---", no colour');
  const legend = v.dom.byId.gbMag1 && v.dom.byId.gbMag1.children[0];
  check(legend && legend.textContent === 'Mag.1' && v.dom.byId.gbBinBox.children[0].textContent === 'Bulkbox', 'group captions are the dfm ones (Mag.1, Bulkbox)');
  check(v.dom.head.children.length === 1 && v.dom.head.children[0].id === 'bdStyle', 'one style element');
}

console.log('-- 2. model(): golden 906 ColorMap :218 and the rest');
{
  const v = load(null), t = v.api.tcolor;
  check(t(GRAY) === '#808080' && t(RED) === '#FF0000' && t(GREEN) === '#008000' && t(ORANGE) === '#FF8000' && t(BLACK) === '#000000',
        'clGray / clRed / clGreen / 0x000080FF / clBlack -> #808080 / #FF0000 / #008000 / #FF8000 / #000000');
  check(t(BTNFACE) === '#f0f0f0' && t(-2147483633) === '#f0f0f0' && t(null) === '' && t('') === '', 'clBtnFace (either sign) and unknown');
  const tags = liveTags();
  const m = v.api.model(k => (k in tags) ? tags[k] : null);
  check(m.pnl.Loader.text === 'L' && m.pnl.Loader.bg === '#FF8000' && m.pnl.Auto1.text === '1' && m.pnl.Auto1.bg === '#008000' &&
        m.pnl.Auto2.bg === '#FF0000' && m.pnl.Fix12.text === 'X' && m.pnl.Fix12.bg === '#808080', 'cells: caption and colour from the facade');
  check(/GerErrNow=是/.test(m.pnl.Auto3.title) && /GetColorNow=1（clRed）/.test(m.pnl.Auto3.title), 'the raw unit state is in the tooltip');
  check(m.lbl.Auto2.text === '1 2' && m.lbl.Auto2.color === '#008000', 'bin list text and colour');
  check(m.gb.Auto6 === false && m.gb.Auto1 === true && m.grp.pnlMag123 === true && m.grp.pnlFix789 === false && m.grp.pnlAuto456 === true,
        'bin.visible -> the station boxes, groups "101" -> pnlMag123 / pnlFix789 / pnlAuto456');
  check(m.status.text === 'Bin display got error!!' && m.status.bg === '#FF0000' && m.tab === true && m.jumpSeq === 0, 'status bar, tab, jumpSeq');
  const short = Object.assign({}, tags, { 'binsel.disp.caption': 'L\tE', 'bin.visible': null });
  const m2 = v.api.model(k => (k in short) ? short[k] : null);
  check(m2.pnl.Loader.text === '---' && m2.pnl.Loader.bg === '' && m2.gb.Auto6 === true && m2.grp.pnlFix789 === true,
        'a caption list of the wrong length = unknown; bin.visible unknown = everything listed');
  const off = Object.assign({}, tags, { 'binsel.disp.tabVisible': false });
  check(v.api.model(k => (k in off) ? off[k] : null).tab === false, 'NUMBER_PANEL_TYPE not 3 / 4 -> tab false');
}

console.log('-- 3. render() with live tags');
{
  const tags = liveTags();
  const v = load(tags);
  const d = v.dom.byId;
  check(d.pnlLoader.textContent === 'L' && d.pnlLoader.style.background === '#FF8000' && d.pnlAuto2.style.background === '#FF0000',
        'cells painted from the tags');
  check(d.sbRunStatus.textContent === 'Bin display got error!!' && d.sbRunStatus.style.background === '#FF0000', 'status bar');
  check(d.gbAuto6.style.display === 'none' && d.pnlFix789.style.display === 'none' && d.gbAuto1.style.display === '', 'visibility');
  check(d.lblAuto1.textContent === ' ', 'an empty bin list keeps its line (nbsp)');
  v.push({ 'binsel.disp.color': tags['binsel.disp.color'].replace(/^[0-9]+/, String(BLACK)) });
  check(d.pnlLoader.style.background === '#000000', 'a pushed change repaints (the black / red flash arrives this way)');
  v.dom.tabs.bindisp.click();
  check(v.dom.tabs.bindisp.classList.contains('act'), '(the tab is shown)');
  v.push({ 'binsel.disp.tabVisible': false });
  check(v.dom.tabs.bindisp.style.display === 'none' && v.dom.tabs.testbin.classList.contains('act'), 'tabVisible false: the tab hides and Test Bin opens');
}

console.log('-- 4. golden :272 jump');
{
  const tags = liveTags();
  tags['binsel.disp.jumpSeq'] = 5;
  const v = load(tags);
  const bd = v.dom.tabs.bindisp;
  check(bd.clicks === 0, 'no jump on the first value');
  v.push({ 'binsel.disp.jumpSeq': 6 });
  check(bd.clicks === 1 && bd.classList.contains('act'), 'jumpSeq 5 -> 6: the Bin Display Status tab is opened');
  v.push({ 'binsel.disp.jumpSeq': 7 });
  check(bd.clicks === 1, 'already shown: no second click');
  v.dom.tabs.testbin.click();
  v.push({ 'binsel.disp.jumpSeq': 8 });
  check(bd.clicks === 2 && bd.classList.contains('act'), 'on another tab: the next jump opens it again (906 jumps every second)');
  v.dom.tabs.testbin.click();
  v.push({ 'binsel.disp.tabVisible': false, 'binsel.disp.jumpSeq': 9 });
  check(bd.clicks === 2 && v.dom.tabs.testbin.classList.contains('act'), 'hidden tab: no jump');
}

console.log('-- 5. page pins');
{
  const page = fs.readFileSync(path.join(pageDir, 'Status.ShowBinSelect.html'));
  const js = fs.readFileSync(jsFile);
  const txt = page.toString('utf8');
  const count = (s, n) => s.split(n).length - 1;
  check(count(txt, '<script src="ht9045_bindisp_status.js"></script>') === 1, 'the page loads ht9045_bindisp_status.js exactly once');
  // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C (low) -- the "still 147 lines" pin is gone; the tag / anchor / order checks stay
  check(count(txt, 'id="bdMap"') === 1 && count(txt, 'id="sbRunStatus"') === 1, 'the pane has #bdMap and #sbRunStatus');
  { const at = (f) => txt.indexOf('<script src="' + f + '"></script>');   // the real tags, not the file names in the pane's comments (:107 / :110) -- St01 1003 06:15
    check(at('ht9045_bindisp_status.js') > 0 && at('ht9045_recipe_client.js') > 0 && at('tabs.js') > 0 &&
          at('ht9045_recipe_client.js') < at('ht9045_bindisp_status.js') && at('tabs.js') < at('ht9045_bindisp_status.js'),
          'loaded after tabs.js and ht9045_recipe_client.js (HT9045Tags)'); }
  for (const [name, b] of [['Status.ShowBinSelect.html', page], [path.basename(jsFile), js]]) {
    const s = b.toString('latin1');
    const crlf = count(s, '\r\n'), lf = count(s, '\n');
    check(!(b[0] === 0xEF && b[1] === 0xBB && b[2] === 0xBF), name + ': no BOM');
    check(crlf === 0 || crlf === lf, name + ': one line ending (CRLF ' + crlf + ' / LF ' + lf + ')');
  }
}

console.log('C14_BinDispPanePage: ' + pass + ' ok, ' + fail + ' failed');
process.exit(fail === 0 ? 0 : 1);
