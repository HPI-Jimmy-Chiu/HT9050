// AI(W906-S98) 20261001 [W906] St01: offline selftest for todo E-003 (1), the page half (ctest S98_TrayPlatePage).
//   web/page/ht9045_config_trayplate.js drives the Configuration page's Tray / Hot Plate tabs (golden V912 cConfiguration.cpp:6945-7217)
//   through WS cfgtrayplate.op (C++ body FileRW/CfgTrayPlate.cpp, tested by S98_CfgTrayPlate). Here: nothing is sent at load; the first get
//   follows the engine's own editlist.get IniConfig (= the HT_WIN open edge when hosted, stage F); grid fill; cell click -> select; double
//   click / Modify Data -> keypad in two steps (HTQwerty OK / Abort); Add / Delete / Load Data / Save round trips against a fake server
//   model; disabled buttons from operable; refusals (busy retry, reload page, not-operable, unknown cmd); one request in flight; tab click.
//   Loads the REAL ht9045_config_trayplate.js into a node vm with a fake DOM, a fake HT9045Recipe, a fake HTQwerty and a fake clock.
//   Config.Configuration.html is read only (ids + load-order ratchet). No socket, no wb_serve, no file written.
//   Usage: node tools/webprobe/s98_trayplate_selftest.cjs [<web/page dir>]
//   Control run (must go red): W906_CONFIG_TRAYPLATE_JS=<a pre-change stand-in, e.g. an empty file>.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_CONFIG_TRAYPLATE_JS || path.join(PAGE, 'ht9045_config_trayplate.js');
let code = '';
try { code = fs.readFileSync(SRC, 'utf8'); } catch (e) { code = ''; }
const html = fs.readFileSync(path.join(PAGE, 'Config.Configuration.html'), 'utf8');

let pass = 0, fail = 0;
function check(cond, what) { if (cond) { pass++; console.log('  PASS ' + what); } else { fail++; console.log('  FAIL ' + what); } }

// ---------------------------------------------------------------- fake clock (same as ta5_selftest.cjs)
let clock = 0, timers = [], tid = 1;
function fakeSetTimeout(fn, ms) { const id = tid++; timers.push({ id, due: clock + Math.max(0, ms | 0), fn }); return id; }
function fakeClearTimeout(id) { timers = timers.filter((t) => t.id !== id); }
async function settle() { for (let i = 0; i < 16; i++) await new Promise((r) => setImmediate(r)); }
async function advance(ms) {
  const end = clock + ms;
  await settle();
  for (;;) {
    timers.sort((a, b) => a.due - b.due || a.id - b.id);
    const t = timers[0];
    if (!t || t.due > end) break;
    timers.shift();
    clock = t.due;
    t.fn();
    await settle();
  }
  clock = end;
  await settle();
}

// ---------------------------------------------------------------- fake DOM
const byId = {};
class El {
  constructor(tag, id) {
    this.tagName = tag; this.children = []; this.parentNode = null; this.style = {}; this.attrs = {}; this.listeners = {};
    this._text = ''; this.className = ''; this.disabled = false; this.value = ''; this.qsa = {};
    if (id) { this.id = id; byId[id] = this; }
  }
  get firstChild() { return this.children[0] || null; }
  get textContent() { return this.children.length ? this.children.map((c) => c.textContent).join('') : this._text; }
  set textContent(v) { this.children = []; this._text = String(v); }
  appendChild(c) { if (c.parentNode) c.parentNode.removeChild(c); c.parentNode = this; this.children.push(c); if (c.id) byId[c.id] = c; return c; }
  removeChild(c) { const i = this.children.indexOf(c); if (i >= 0) this.children.splice(i, 1); c.parentNode = null; return c; }
  setAttribute(k, v) { this.attrs[k] = String(v); }
  getAttribute(k) { return k in this.attrs ? this.attrs[k] : null; }
  removeAttribute(k) { delete this.attrs[k]; }
  addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); }
  removeEventListener(t, f) { this.listeners[t] = (this.listeners[t] || []).filter((x) => x !== f); }
  querySelector() { return null; }
  querySelectorAll(sel) { return this.qsa[sel] || []; }
  get classList() { const self = this; return { contains(c) { return (' ' + self.className + ' ').indexOf(' ' + c + ' ') >= 0; }, add(c) { self.className += ' ' + c; }, remove() {} }; }
}
function fire(target, type, props) {
  const ev = Object.assign({ type, target, isTrusted: true, preventDefault() {}, stopPropagation() { this.stopped = true; } }, props || {});
  for (let n = target; n && !ev.stopped; n = n.parentNode) (n.listeners[type] || []).slice().forEach((f) => { ev.currentTarget = n; f(ev); });
  return ev;
}
const head = new El('HEAD');
const document = {
  readyState: 'complete',
  head,
  documentElement: head,
  getElementById: (id) => byId[id] || null,
  querySelector: () => null,
  querySelectorAll: () => [],
  createElement: (t) => new El(t.toUpperCase()),
  addEventListener() {},
};
// the page: the two tab panes as Config.Configuration.html:70 has them (static placeholder table in each grid div, the ten buttons)
const body = new El('BODY');
['strngrdTray', 'strngrdHP'].forEach((id) => { const d = body.appendChild(new El('DIV', id)); d.appendChild(new El('TABLE')); });
['btnAddTray', 'btnDeleteTray', 'btnModifyTray', 'sbUpdateTray', 'sbtReloadTray', 'btnAddHP', 'btnDeleteHP', 'btnModifyHP', 'sbUpdateHP', 'sbtReloadHP']
  .forEach((id) => body.appendChild(new El('BUTTON', id)));
const pc = body.appendChild(new El('DIV', 'PageControl1'));
const tabs = [0, 1, 2, 3, 4].map((n) => { const t = new El('DIV'); t.attrs['data-t'] = String(n); t.className = n === 2 ? 'tab act' : 'tab'; return t; });
pc.qsa[':scope > .pcTabs > .tab'] = tabs;
let progTabClicks = 0;
tabs.forEach((t) => { t.click = function () { progTabClicks++; tabs.forEach((x) => { x.className = 'tab'; }); this.className = 'tab act'; fire(this, 'click', { isTrusted: false }); }; });
pc.querySelector = (sel) => {                                // the two selectors the page script uses (':scope > .pcTabs > .tab.act' / '.tab[data-t="n"]')
  if (sel === ':scope > .pcTabs > .tab.act') return tabs.find((t) => / act/.test(t.className)) || null;
  const m = /\.tab\[data-t="(\d+)"\]$/.exec(sel);
  return m ? tabs[+m[1]] || null : null;
};
const actTab = () => { const t = tabs.find((x) => / act/.test(x.className)); return t ? +t.attrs['data-t'] : -1; };

// ---------------------------------------------------------------- fake server model (the page's view of cfgtrayplate.op; golden logic is
//   S98_CfgTrayPlate's job) + fake recipe client
const HEAD16 = ['Package Type', 'X Start Pos', 'Y Start Pos', 'X Pitch', 'Y Pitch', 'Columns (X)', 'Rows (Y)', 'X Width', 'Y Width',
  'Z Tray Tickness', 'Group', 'Memo', 'BlockNumberX', 'BlockNumberY', 'BlockPitchX', 'BlockPitchY'];
function mkTable(rows) { return { cells: rows.map((r) => r.slice()), fixedRows: 1, fixedCols: 0, cursor: { col: 0, row: 1 }, sel: { col: 0, row: 0 }, buttons: true, grid: true }; }
const M = {
  tray: mkTable([HEAD16, ['QFN-48', '10.5', '20', '6.35', '6.35', '8', '12', '3', '3', '5', '', '', '1', '1', '0', '0'],
    ['BGA', '11', '21', '7', '7', '10', '20', '4', '4', '6', '', '', '2', '2', '0', '0']]),
  hp: mkTable([HEAD16, ['HP-A', '1', '2', '3', '4', '5', '6', '7', '', '', '', '', '', '', '', '']]),
  files: {},
  keypad: null,
};
M.files.tray = M.tray.cells.map((r) => r.slice());
M.files.hp = M.hp.cells.map((r) => r.slice());
function snapOf(t) {
  return { rowCount: t.cells.length, colCount: 16, fixedRows: t.fixedRows, fixedCols: t.fixedCols, defaultColWidth: 80,
    colWidths: [200].concat(new Array(15).fill(80)), cursor: Object.assign({}, t.cursor), sel: Object.assign({}, t.sel),
    cells: t.cells.map((r) => r.slice()), operable: { grid: t.grid, buttons: t.buttons, reload: t.buttons }, onTab: true, file: 'x', fileExists: true };
}
function serve(v) {
  const t = v.table ? M[v.table] : null;
  const reply = { op: v.op, ran: v.op !== 'get' };
  if (v.op === 'select') { t.sel = { col: v.col, row: v.row }; t.cursor = { col: v.col, row: v.row }; }
  if (v.op === 'modify' && v.step === 0) {
    if (t.sel.row > 0) { M.keypad = { table: v.table, row: t.sel.row, col: t.sel.col }; reply.keypad = { open: true, flags: 2, kind: 'double', dp: 2, checkRange: true, min: 0, max: 1000, current: t.cells[t.sel.row][t.sel.col], header: t.cells[0][t.sel.col] }; }
    else reply.keypad = { open: false };
  }
  if (v.op === 'modify' && v.step === 1) {
    if (!M.keypad) throw new Error('keypad: no Modify Data keypad is open for this table (send step 0 first)');
    if (!v.cancel) t.cells[M.keypad.row][M.keypad.col] = String(parseFloat(v.text));
    M.keypad = null;
  }
  if (v.op === 'add') { t.cells.push(new Array(16).fill('')); t.sel.row = t.cells.length - 1; t.cursor.row = t.sel.row; }
  if (v.op === 'delete' && t.sel.row > 0) { t.cells.splice(t.sel.row, 1); }
  if (v.op === 'save') { M.files[v.table] = t.cells.map((r) => r.slice()); reply.wrote = 'D:/sandbox/' + v.table + '.csv'; }
  if (v.op === 'reload') { t.cells = M.files[v.table].map((r) => r.slice()); }
  reply.tray = snapOf(M.tray);
  reply.hp = snapOf(M.hp);
  return reply;
}
const cmds = [], says = [];
const script = [];                                           // forced answers for the next rawCmd: 'err:<message>' | 'hold' (resolve later)
let held = null, pageLoads = 0, keepAlives = 0;
const R = {
  status() { return { connected: true, holdsToken: true }; },
  keepAlive() { keepAlives++; return Promise.resolve(); },
  nextProxies: {},
  editlistGet(st) { cmds.push({ name: 'editlist.get', st, at: clock }); return Promise.resolve({ struct: st, proxies: R.nextProxies }); },
  editlistSave() { return Promise.resolve({}); },
  rawCmd(name, extra) {
    const v = JSON.parse(extra.value);
    cmds.push({ name, v, at: clock });
    const next = script.length ? script.shift() : '';
    if (next.indexOf('err:') === 0) return Promise.reject(new Error(next.slice(4)));
    if (next === 'hold') return new Promise((res) => { held = () => res(Object.assign({ type: 'ack', ok: true }, serve(v))); });
    try { return Promise.resolve(Object.assign({ type: 'ack', ok: true }, serve(v))); } catch (e) { return Promise.reject(e); }
  },
};
const shows = [];
const sb = {
  console: { info() {}, warn() {}, log() {}, error(...a) { console.error(...a); } },
  setTimeout: fakeSetTimeout, clearTimeout: fakeClearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp, isNaN, parseInt,
  HT9045Recipe: R,
  HT9045Page: { load() { pageLoads++; return R.editlistGet('IniConfig'); } },
  HT9045Wire: { say(msg) { says.push(msg); } },
  HTQwerty: { N: { INTEGER: 1, DOUBLE: 2, NO_SYMBOL: 4 }, show(target, flags, opt) { shows.push({ target, flags, opt }); } },
  document,
};
sb.window = sb;
vm.createContext(sb);
let loadErr = null;
try { vm.runInContext(code, sb, { filename: 'ht9045_config_trayplate.js' }); } catch (e) { loadErr = e; }

const ops = () => cmds.filter((c) => c.name === 'cfgtrayplate.op');
const last = () => ops()[ops().length - 1];
const grid = (key) => byId[key === 'hp' ? 'strngrdHP' : 'strngrdTray'];
const td = (key, r, c) => { const t = grid(key).children[0]; return t && t.children[r] && t.children[r].children[c]; };

(async () => {
  console.log('s98_trayplate_selftest -- ' + SRC);
  check(!loadErr && code.length > 0, 'the page script loads' + (loadErr ? ' (' + loadErr.message + ')' : ''));

  console.log('[0] Config.Configuration.html (read-only): ids, load order');
  ['strngrdTray', 'strngrdHP', 'btnAddTray', 'btnDeleteTray', 'btnModifyTray', 'sbUpdateTray', 'sbtReloadTray', 'btnAddHP', 'btnDeleteHP',
    'btnModifyHP', 'sbUpdateHP', 'sbtReloadHP'].forEach((id) => check(html.indexOf('id="' + id + '"') >= 0, 'html has #' + id));
  const iQ = html.indexOf('<script src="qwerty.js">'), iEng = html.indexOf('<script src="ht9045_wire_engine.js">'),
    iEv = html.indexOf('<script src="ht9045_config_st01_ev.js">'), iTp = html.indexOf('<script src="ht9045_config_trayplate.js">');
  check(iQ >= 0 && iEng > iQ && iEv > iEng && iTp > iEv, 'ht9045_config_trayplate.js loads after qwerty.js, the engine and ht9045_config_st01_ev.js');
  // AI(W906-ST02-2C) 20261006 (St02-E): Frank FR-PR1 2C #19 -- no pinned html:138: the one line that loads ht9045_config_trayplate.js
  //   also loads ht9045_config_st01_ev.js (the S98 same-line insert), wherever other edits move that line.
  const hl = html.split('\n'), tpAt = [];
  hl.forEach((l, i) => { if (l.indexOf('<script src="ht9045_config_trayplate.js">') >= 0) tpAt.push(i); });
  const lTp = tpAt.length === 1 ? hl[tpAt[0]] : '';
  check(lTp.indexOf('<script src="ht9045_config_st01_ev.js">') >= 0,
    'same line as ht9045_config_st01_ev.js (html:' + (tpAt.length === 1 ? tpAt[0] + 1 : '?') + ', loaded once; the S98 same-line insert)');

  console.log('[1] load: nothing sent until the engine\'s own editlist.get IniConfig (the open edge)');
  await advance(5000);
  check(ops().length === 0, 'nothing sent at load (a closed hosted window reads nothing, stage F)');
  await sb.HT9045Recipe.editlistGet('TrayForm');
  await advance(100);
  check(ops().length === 0, 'another page structure\'s editlist.get does not trigger anything');
  const p = sb.HT9045Recipe.editlistGet('IniConfig');         // the wrapped get the engine calls on the open edge
  await p;
  check(ops().length === 0, 'not in the same tick (the engine applies its values in the same then; the get waits a setTimeout 0)');
  await advance(0);
  check(ops().length === 1 && last().v.op === 'get' && Object.keys(last().v).length === 1, 'then exactly one {"op":"get"}');

  console.log('[2] grid fill');
  const tray = grid('tray').children[0];
  check(tray && tray.tagName === 'TABLE' && tray.className === 's98g' && tray.children.length === 3 && tray.children[0].children.length === 16,
    'strngrdTray: the placeholder table replaced by 3 x 16 cells');
  check(td('tray', 0, 0).textContent === 'Package Type' && td('tray', 1, 0).textContent === 'QFN-48' && td('tray', 2, 1).textContent === '11',
    'cell texts from the server snapshot');
  check(td('tray', 0, 3).className === 'fx' && td('tray', 1, 0).className === 'cur' && td('tray', 2, 3).className === '',
    'fixed row 0 styled fixed, the VCL current cell (0,1) highlighted');
  check(td('tray', 1, 0).style.width === '200px' && td('tray', 1, 1).style.width === '80px', 'column widths 200 / 80 (golden :7045-7046)');
  check(grid('hp').children[0].children.length === 2, 'strngrdHP filled too');
  check(!byId.btnAddTray.disabled && !byId.sbUpdateHP.disabled, 'buttons enabled (operable)');

  console.log('[3] cell click -> select');
  fire(td('tray', 2, 3), 'mousedown');
  await advance(0);
  check(ops().length === 2 && last().v.op === 'select' && last().v.table === 'tray' && last().v.col === 3 && last().v.row === 2,
    'mousedown on (3,2) -> {"op":"select","table":"tray","col":3,"row":2}');
  check(td('tray', 2, 3).className === 'cur' && td('tray', 1, 0).className === '', 'the reply moves the highlight');
  fire(td('tray', 0, 5), 'mousedown');
  await advance(0);
  check(ops().length === 2, 'a click on the fixed header row sends nothing (VCL does not select fixed cells)');
  fire(td('tray', 2, 3), 'mousedown', { isTrusted: false });
  await advance(0);
  check(ops().length === 2, 'a synthetic (isTrusted=false) click sends nothing');

  console.log('[4] double click -> Modify Data keypad, two steps');
  fire(td('tray', 2, 3), 'dblclick');
  await advance(0);
  check(last().v.op === 'modify' && last().v.step === 0 && last().v.via === 'dblclick', 'dblclick -> {"op":"modify","step":0,"via":"dblclick"}');
  check(shows.length === 1 && shows[0].flags === 2 && shows[0].opt.checkRange === true && shows[0].opt.min === 0 && shows[0].opt.max === 1000 &&
    shows[0].opt.dp === 2 && shows[0].target.value === '7', 'HTQwerty.show(box="7", N_DOUBLE, checkRange 0..1000, dp 2 = the server\'s golden step index)');
  const n4 = ops().length;
  fire(byId.btnAddTray, 'click');
  fire(td('tray', 1, 1), 'mousedown');
  await advance(0);
  check(ops().length === n4, 'while the keypad is open nothing else is sent (golden keypad is modal)');
  shows[0].opt.onCommit('12.5');
  await advance(0);
  check(last().v.op === 'modify' && last().v.step === 1 && last().v.text === '12.5' && last().v.cancel === undefined, 'OK -> {"step":1,"text":"12.5"}');
  check(td('tray', 2, 3).textContent === '12.5', 'the reply re-renders the cell');
  fire(byId.btnModifyTray, 'click');
  await advance(0);
  check(last().v.op === 'modify' && last().v.via === 'button' && shows.length === 2, 'Modify Data button -> step 0 via button -> keypad');
  shows[1].opt.onAbort();
  shows[1].opt.onAbort();
  await advance(0);
  check(last().v.step === 1 && last().v.cancel === true && ops().filter((c) => c.v.cancel).length === 1, 'Abort -> {"step":1,"cancel":true}, once');

  console.log('[5] Add / Delete / Save / Load Data round trip');
  fire(byId.btnAddTray, 'click');
  await advance(0);
  check(last().v.op === 'add' && last().v.table === 'tray' && grid('tray').children[0].children.length === 4 && td('tray', 3, 3).className === 'cur',
    'Add -> 4 rows, the new row highlighted');
  fire(byId.btnDeleteTray, 'click');
  await advance(0);
  check(last().v.op === 'delete' && grid('tray').children[0].children.length === 3, 'Delete -> 3 rows');
  fire(byId.sbUpdateTray, 'click');
  await advance(0);
  check(last().v.op === 'save' && M.files.tray[2][3] === '12.5' && says.some((s) => /寫入/.test(s)), 'Save -> the fake file has 12.5, status line says written');
  M.tray.cells[2][3] = 'dirty';                              // an unsaved change on the server side
  fire(byId.sbtReloadTray, 'click');
  await advance(0);
  check(last().v.op === 'reload' && td('tray', 2, 3).textContent === '12.5', 'Load Data -> the file value again');
  fire(byId.btnAddHP, 'click');
  await advance(0);
  check(last().v.op === 'add' && last().v.table === 'hp' && grid('hp').children[0].children.length === 3, 'HP Add goes to table "hp"');

  console.log('[6] operable -> buttons disabled / enabled');
  M.tray.buttons = false;
  fire(td('tray', 1, 1), 'mousedown');
  await advance(0);
  check(byId.btnAddTray.disabled && byId.sbUpdateTray.disabled && byId.sbtReloadTray.disabled && byId.btnAddTray.getAttribute('data-s98-dis') === '1' &&
    !byId.btnAddHP.disabled, 'operable.buttons=false -> the five Tray buttons disabled (own mark), HP untouched');
  const n6 = ops().length;
  fire(byId.btnAddTray, 'click');
  await advance(0);
  check(ops().length === n6, 'a disabled button sends nothing');
  M.tray.buttons = true;
  fire(td('tray', 1, 2), 'mousedown');
  await advance(0);
  check(!byId.btnAddTray.disabled && byId.btnAddTray.getAttribute('data-s98-dis') === null, 'operable again -> enabled');
  byId.btnDeleteHP.disabled = true;                          // the engine's own disable (not ours): left alone
  M.hp.buttons = true;
  fire(td('hp', 1, 1), 'mousedown');
  await advance(0);
  check(byId.btnDeleteHP.disabled, 'a button disabled by someone else stays disabled');
  byId.btnDeleteHP.disabled = false;

  console.log('[7] refusals');
  script.push('err:busy: cfgtrayplate.op same command within 400 ms');
  fire(td('tray', 2, 2), 'mousedown');
  await advance(0);
  const nb = ops().length;
  await advance(450);
  check(ops().length === nb + 1 && last().v.op === 'select' && last().v.col === 2, 'busy -> resent after 450 ms');
  script.push('err:bad-payload: reload page: open the Configuration page (editlist.get IniConfig = golden FormShow) with the current access level');
  fire(byId.btnAddTray, 'click');
  await advance(0);
  check(pageLoads === 1, 'reload page -> HT9045Page.load() (the engine reads, this file follows with a get)');
  await advance(0);
  check(last().v.op === 'get', '... and the get after that editlist.get');
  script.push('err:not-operable: pnlTray (or a container) is disabled or hidden after golden FormShow');
  fire(byId.btnDeleteTray, 'click');
  await advance(0);
  check(says.some((s) => /not-operable/.test(s)) && last().v.op === 'get', 'not-operable -> status line + a get to refresh the buttons');
  script.push('err:not-operator');
  const ka = keepAlives;
  fire(byId.btnAddTray, 'click');
  await advance(0);
  check(keepAlives === ka + 1 && last().v.op === 'add', 'not-operator -> keepAlive, sent again once');

  console.log('[8] one request in flight');
  script.push('hold');
  fire(td('tray', 1, 3), 'mousedown');
  await advance(0);
  const nh = ops().length;
  fire(byId.btnAddTray, 'click');
  fire(byId.btnAddTray, 'click');
  fire(td('tray', 1, 3), 'mousedown');
  await advance(0);
  check(ops().length === nh, 'nothing else goes out while one is in flight');
  held();
  await advance(0);
  check(ops().length === nh + 1 && last().v.op === 'add' && ops().filter((c, i) => i >= nh - 1 && c.v.op === 'add').length === 1,
    'after the reply: the queued Add once (the second Add and the repeated click were double clicks)');

  console.log('[9] tab click -> get after 300 ms; older generation replies are dropped');
  fire(tabs[3], 'click');
  await advance(299);
  const nt = ops().length;
  await advance(1);
  check(ops().length === nt + 1 && last().v.op === 'get', 'Tray tab click -> one get 300 ms later (after st01_ev\'s PageControl1 form.event)');
  fire(tabs[2], 'click');
  await advance(400);
  check(ops().length === nt + 1, 'Config tab click -> nothing');
  script.push('hold');
  fire(td('tray', 1, 4), 'mousedown');
  await advance(0);
  const before = td('tray', 1, 4).className;
  await sb.HT9045Recipe.editlistGet('IniConfig');            // the page reopens while the select is in flight
  held();
  await settle();                                            // microtasks only: the new generation's get (setTimeout 0) has not run yet
  check(td('tray', 1, 4).className === before, 'the reply of the older generation is not applied');
  await advance(0);
  check(last().v.op === 'get', 'the new generation reads again');

  console.log('[11] open edge: the page tab follows the PageControl1 of the server (golden FormShow :4645 ActivePage=tsConfig)');
  fire(tabs[3], 'click');                                    // the user was on the Tray tab (a user click: switch it by hand here)
  tabs.forEach((x) => { x.className = 'tab'; }); tabs[3].className = 'tab act';
  await advance(400);
  const n11 = ops().length;
  R.nextProxies = { PageControl1: { activePageIndex: 2 } };
  const p11 = sb.HT9045Recipe.editlistGet('IniConfig');      // Save = golden FormClose, the engine reads again (FormShow)
  await p11;
  check(actTab() === 2 && progTabClicks === 1, 'server activePageIndex 2 -> the page switched to the Config tab (programmatic click), in the then (before the setTimeout of st01_ev)');
  await advance(400);
  check(ops().length === n11 + 1 && last().v.op === 'get', 'the programmatic tab click sent nothing (isTrusted=false); only the open-edge get');
  R.nextProxies = { PageControl1: { activePageIndex: 2 } };
  await sb.HT9045Recipe.editlistGet('IniConfig');
  check(progTabClicks === 1, 'already on the tab of the server -> no click');
  await advance(0);
  R.nextProxies = {};

  console.log('[10] unknown cmd (old wb_serve) -> off');
  script.push('err:unknown cmd: cfgtrayplate.op');
  fire(byId.btnAddTray, 'click');
  await advance(0);
  const no = ops().length;
  check(byId.btnAddTray.disabled && byId.sbtReloadHP.disabled, 'all ten buttons disabled');
  fire(td('tray', 1, 1), 'mousedown');
  await advance(0);
  check(ops().length === no, 'nothing sent any more');

  console.log('s98_trayplate_selftest: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.error(e); process.exit(2); });
