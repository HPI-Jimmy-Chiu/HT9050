// AI(W906-E021-OB1) 20261002 [W906] (St01): todo E-021, the page half (node, offline).
//   golden V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cObserver.cpp: BtnExitClick :697-706, the Record tab handlers
//   :4683-4984, pgcMessageChange :4997-5024, lstTimeDataClick :5072-5075, btnBackupLogYearClick :5602-5622, btnClearTimeClick :5624-5630,
//   dfm btnTimeData OnClick=btnQueryEventLogTxtClick (:1813-1822).  The C++ half is ctest E021_Observer (tests/test_e021_observer.cpp).
//   [A] D:\HT9045\web\page\ht9045_observer_ev.js in a node vm with a fake DOM / HT9045Recipe / HT9045Observer (+ the real
//       ht9045_busy_util.js): compiles, one EOL style, no BOM; control.acquire -> act.observer.<op> -> control.release; no widgets before
//       the first dataRecord, all Record-tab inputs after it; dataRecord drawn (tab visibility, edits, combos incl. a text outside the
//       list, memos, panels, the focused box kept); executed / refused / busy / not-big5 (the typed text stays) on the status line; a change -> one debounced sync; Exit
//       stopped before .exitbtn, closes on close:true only, an old wb_serve (unknown-action) still closes, busy does not; pgcMessage tabs
//       -> msgTab {arg}; lstTimeData -> timeFile; Backup Log sends the year text; Clear Time Data refreshes; btnTimeData -> observer.get query
//   [B] ht9045_observer_wire.js (cut: render(), wire()): dataRecord goes to HT9045ObserverEv.render; Backup Log's "not wired" listener is
//       bound only when ht9045_observer_ev.js is absent
//   [C] Data.Observer.html loads ht9045_observer_ev.js after ht9045_observer_wire.js (outside comments)
//   Control: W906_E021_PAGE_DIR -> the pre-change page directory must be red.
//   No wb_serve, no machine file (reads .js / .html under web\page).  Use: only through ctest (E021_ObserverPage).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_E021_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let failed = 0, passed = 0;
async function check(name, fn) {
  try { await fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function ok(c, what) { if (!c) throw new Error(what); }
const flush = async (ms) => { if (ms) await new Promise(r => setTimeout(r, ms)); for (let i = 0; i < 12; i++) await new Promise(r => setImmediate(r)); };
function read(f) { return fs.readFileSync(path.join(PAGE_DIR, f)); }

function makeEl(id, tag, cls) {
  const attrs = {}, listeners = {}, classes = new Set(cls ? cls.split(' ') : []);
  const el = {
    id, tagName: tag || 'DIV', nodeType: 1, disabled: false, title: '', style: {}, textContent: '', innerHTML: '', value: '', children: [],
    selectedIndex: -1, options: null,
    classList: { contains: c => classes.has(c), add: c => classes.add(c), remove: c => classes.delete(c) },
    getAttribute: k => (k in attrs ? attrs[k] : null),
    setAttribute: (k, v) => { attrs[k] = String(v); },
    hasAttribute: k => k in attrs,
    addEventListener: (t, fn) => { (listeners[t] = listeners[t] || []).push(fn); },
    listeners: t => (listeners[t] || []).length,
    fire: (t, ev) => { (listeners[t] || []).forEach(fn => fn(Object.assign({ type: t, preventDefault() {} }, ev || {}))); },
    click: () => el.fire('click'),
    appendChild: c => { el.children.push(c); return c; },
    querySelector: sel => (sel === ':scope > .pnlCap' ? (el.children.find(c => c.className === 'pnlCap') || null) : null),
    querySelectorAll: () => [],
    closest: () => null,
  };
  return el;
}

const EDITS = ['edPrecautionRecordDocumentNo', 'edNoteContents', 'edApprovedManager', 'edWatchmakers', 'edFinishName', 'edPromptDay',
               'edMajorMaintenanceCheckNo', 'edMajorMaintenancePersonnel', 'edMajorMaintenanceCheckPersonnel'];
const COMBOS = ['cobPRFinishType', 'cobNoteContents', 'cobHandlerPrecautionRecord', 'cobMajorMaintenanceClassType',
                'cobUndesirablePhenomenon', 'cobCountermeasure', 'cobMajorMaintenanceSearch', 'cobSearchPrecautionLog'];
const MEMOS = ['MemoHandlerPrecautionRecord', 'MemoUndesirablePhenomenon', 'MemoCountermeasure', 'MemoNoteLog'];
const BUTTONS = { cobNoteContentsSet: 'prNoteSet', sbHandlerPrecautionRecordSet: 'prRecordSet', sbHandlerPrecautionRecordClear: 'prRecordClear',
  sbHandlerPrecautionFormShow: 'prFormShow', sbPrecautionSave: 'prSave', sbPRStartDate: 'prStartDate', sbPRFinishDate: 'prFinishDate',
  sbMajorMaintenanceDate: 'mmDate', sbMajorMaintenanceStartTime: 'mmStart', sbMajorMaintenanceEndTime: 'mmEnd',
  sbUndesirablePhenomenon: 'mmPhenAdd', sbCountermeasure: 'mmCmAdd', sbUndesirablePhenomenonClear: 'mmPhenClear',
  sbCountermeasureClear: 'mmCmClear', sbMajorMaintenanceSave: 'mmSave', sbMajorMaintenanceSearch: 'mmSearch',
  sbSearchPrecautionLog: 'prLogSearch', btnClearTime: 'clearTime', btnBackupLogYear: 'backupLogYear' };

function dataRecord(over) {
  const d = {
    b01: true, b02: true,
    tabVisible: { tsDataRecord: true, tsPrecautionsRecord: true, tsHanderMajorMaintenance: true, tsPrecautionLog: true },
    edits: {}, combos: {}, memos: {}, panels: { pnPrecautionStartTime: '2026/10/02', pnPRSpecificationNO: 'SPEC-1' },
  };
  EDITS.forEach(id => { d.edits[id] = 'v_' + id; });
  COMBOS.forEach(id => { d.combos[id] = { items: ['a', 'b'], itemIndex: 0, text: 'a' }; });
  d.combos.cobPRFinishType = { items: ['By MO', 'By Day'], itemIndex: 1, text: 'By Day' };
  d.combos.cobMajorMaintenanceClassType = { items: ['A', 'B', 'C', 'D', 'E'], itemIndex: -1, text: 'F' };
  MEMOS.forEach(id => { d.memos[id] = ['l1_' + id, 'l2']; });
  return Object.assign(d, over || {});
}

function makeEnv(script, busyScript) {
  const els = {};
  const add = (id, tag, cls) => { els[id] = makeEl(id, tag, cls); return els[id]; };
  EDITS.forEach(id => add(id, 'INPUT'));
  COMBOS.forEach(id => add(id, 'SELECT'));
  MEMOS.forEach(id => add(id, 'TEXTAREA'));
  ['pnPrecautionStartTime', 'pnPRSpecificationNO'].forEach(id => add(id, 'DIV', 'pnl'));
  Object.keys(BUTTONS).forEach(id => add(id, 'BUTTON'));
  add('btnTimeData', 'BUTTON'); add('btExit', 'BUTTON', 'btn3d exitbtn'); add('obsStatus'); add('lstTimeData', 'DIV', 'lbx'); add('strngrdTimeData', 'DIV', 'sgd');
  const year = add('cbbEventLogYear', 'SELECT');
  year.options = [{ text: '2025' }, { text: '2026' }]; year.selectedIndex = 1;
  const tabMain = makeEl('', 'DIV', 'tab'); tabMain.setAttribute('data-t', '7');
  const prTabs = [0, 1, 2].map(n => { const t = makeEl('', 'DIV', 'tab' + (n === 0 ? ' act' : '')); t.setAttribute('data-t', String(n)); return t; });
  prTabs.forEach((t, n) => { t.click = () => { prTabs.forEach(x => x.classList.remove('act')); t.classList.add('act'); t.clicked = (t.clicked || 0) + 1; }; });
  const msgTabs = [1, 2, 3].map(n => { const t = makeEl('', 'DIV', 'tab'); t.setAttribute('data-t', String(n)); return t; });
  const sent = [], posted = [], replies = [], obsSent = [];
  let ticks = 0;
  const docListeners = [];
  const R = {
    rawCmd: (cmd, extra) => {
      sent.push({ cmd, value: extra && extra.value ? JSON.parse(extra.value) : undefined });
      if (cmd === 'control.acquire' || cmd === 'control.release') return Promise.resolve({});
      const r = replies.shift() || { ok: false, v: { executed: false, guard: 'test-no-reply' } };
      if (r.busy) return Promise.reject(new Error('busy: same command in progress or just done (' + cmd + ', 10 ms ago)'));
      return r.ok ? Promise.resolve(r.v) : Promise.reject(new Error(JSON.stringify(r.v)));
    },
  };
  const doc = {
    readyState: 'complete', activeElement: null,
    getElementById: id => els[id] || null,
    createElement: tag => makeEl('', String(tag).toUpperCase()),
    addEventListener: (t, fn, cap) => { docListeners.push({ t, fn, cap: !!cap }); },
    querySelector: sel => (sel === '#pgcObserv > .pcTabs > .tab[data-t="7"]' ? tabMain :
      (/^#pgcPrecautions > \.pcTabs > \.tab\[data-t="(\d)"\]$/.test(sel) ? prTabs[+RegExp.$1] : null)),
    querySelectorAll: sel => (sel === '#pgcMessage > .pcTabs > .tab' ? msgTabs : []),
  };
  const win = vm.createContext({
    HT9045Recipe: R,
    HT9045Observer: { send: v => { obsSent.push(v); return Promise.resolve(null); }, tick: () => { ticks++; } },
    console: { info() {}, log() {}, error() {} },
    parent: { postMessage: (m, o) => posted.push({ m, o }) },
    document: doc, setTimeout, clearTimeout, Promise, JSON, Object, Array, String, Date, Error, RegExp, Number, Math,
  });
  vm.runInContext('var window = this;', win);
  if (busyScript) busyScript.runInContext(win);
  script.runInContext(win);
  const exitClick = () => {
    let stopped = false;
    const ev = { type: 'click', target: { closest: s => (s === '#btExit' ? els.btExit : null) }, stopImmediatePropagation() { stopped = true; }, preventDefault() {} };
    docListeners.filter(l => l.t === 'click' && l.cap).forEach(l => l.fn(ev));
    if (!stopped) els.btExit.fire('click');          // the bubbling .exitbtn listener (Data.Observer.html) -> closeMe
    return stopped;
  };
  els.btExit.addEventListener('click', () => posted.push({ m: { closeMe: 1 }, o: '*', byExitbtn: true }));
  return { win, els, sent, posted, replies, obsSent, doc, tabMain, prTabs, msgTabs, exitClick, ticks: () => ticks };
}
const acts = env => env.sent.filter(s => s.cmd !== 'control.acquire' && s.cmd !== 'control.release');

(async () => {
  let evText = '', evBytes = null, evScript = null, busyScript = null;
  await check('[A] read ht9045_observer_ev.js', () => { evBytes = read('ht9045_observer_ev.js'); evText = evBytes.toString('utf8'); });
  await check('[A] no BOM, one EOL style', () => {
    ok(!(evBytes[0] === 0xEF && evBytes[1] === 0xBB && evBytes[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (evText.match(/\r\n/g) || []).length, lf = (evText.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
  });
  await check('[A] compiles', () => { ok(evBytes, 'not read'); evScript = new vm.Script(evText, { filename: 'ht9045_observer_ev.js' }); });
  await check('[A] ht9045_busy_util.js compiles', () => { busyScript = new vm.Script(read('ht9045_busy_util.js').toString('utf8'), { filename: 'ht9045_busy_util.js' }); });

  if (evScript) {
    // A1 before the first dataRecord: no widgets
    {
      const env = makeEnv(evScript, busyScript);
      env.replies.push({ ok: true, v: { executed: true, op: 'prSave', result: 'ran' } });
      env.els.sbPrecautionSave.click();
      await flush();
      await check('[A1] control.acquire -> act.observer.prSave -> control.release; no widgets before the first dataRecord', () => {
        ok(env.sent.length === 3 && env.sent[0].cmd === 'control.acquire' && env.sent[1].cmd === 'act.observer.prSave' && env.sent[2].cmd === 'control.release', JSON.stringify(env.sent));
        ok(env.sent[1].value && env.sent[1].value.widgets === undefined, JSON.stringify(env.sent[1].value));
      });
      await check('[A1] every Record-tab button is bound to its op', () => {
        Object.keys(BUTTONS).forEach(id => ok(env.els[id].listeners('click') === 1, id + ' has ' + env.els[id].listeners('click') + ' click listeners'));
      });
    }
    // A2 render
    {
      const env = makeEnv(evScript, busyScript);
      env.doc.activeElement = env.els.edWatchmakers;
      env.els.edWatchmakers.value = 'typing';
      env.win.HT9045ObserverEv.render(dataRecord({ tabVisible: { tsDataRecord: true, tsPrecautionsRecord: false, tsHanderMajorMaintenance: true, tsPrecautionLog: true } }));
      await check('[A2] edits / memos / panels drawn; the focused box is kept', () => {
        ok(env.els.edNoteContents.value === 'v_edNoteContents', env.els.edNoteContents.value);
        ok(env.els.edWatchmakers.value === 'typing', 'focused box overwritten: ' + env.els.edWatchmakers.value);
        ok(env.els.MemoNoteLog.value === 'l1_MemoNoteLog\nl2' && env.els.MemoNoteLog.textContent === 'l1_MemoNoteLog\nl2', env.els.MemoNoteLog.value);
        const cap = env.els.pnPRSpecificationNO.children.find(c => c.className === 'pnlCap');
        ok(cap && cap.textContent === 'SPEC-1', 'panel caption ' + JSON.stringify(cap));
      });
      await check('[A2] combos: items, selection, a text outside the list = an extra selected option', () => {
        ok(/<option>By MO<\/option><option>By Day<\/option>/.test(env.els.cobPRFinishType.innerHTML) && env.els.cobPRFinishType.selectedIndex === 1, env.els.cobPRFinishType.innerHTML);
        ok(/data-extra="1">F</.test(env.els.cobMajorMaintenanceClassType.innerHTML) && env.els.cobMajorMaintenanceClassType.selectedIndex === 5, env.els.cobMajorMaintenanceClassType.innerHTML);
        ok(env.els.cobPRFinishType.getAttribute('data-items') === '2', 'data-items');
      });
      await check('[A2] tab visibility: Record shown; the hidden active Precautions Record sub-tab moves to the first visible one', () => {
        ok(env.tabMain.style.display === '', 'Record tab ' + env.tabMain.style.display);
        ok(env.prTabs[0].style.display === 'none' && env.prTabs[1].style.display === '' && env.prTabs[1].clicked === 1, JSON.stringify(env.prTabs.map(t => [t.style.display, t.clicked])));
      });
      env.win.HT9045ObserverEv.render(dataRecord({ tabVisible: { tsDataRecord: false } }));
      await check('[A2] tsDataRecord false -> the Record tab is hidden (golden FormShow :591-593)', () => { ok(env.tabMain.style.display === 'none', env.tabMain.style.display); });
      // A3 widgets after the first render
      env.doc.activeElement = null;
      env.els.MemoHandlerPrecautionRecord.value = 'x1\r\nx2\n';
      env.replies.push({ ok: true, v: { executed: true, op: 'prRecordSet', result: 'ran', dataRecord: dataRecord() } });
      env.els.sbHandlerPrecautionRecordSet.click();
      await flush();
      const v = acts(env)[0] && acts(env)[0].value;
      await check('[A3] after the first dataRecord every input is sent: edits, combos {itemIndex}, memo lines', () => {
        ok(v && v.widgets, JSON.stringify(v));
        EDITS.forEach(id => ok(typeof v.widgets[id] === 'string', id));
        COMBOS.forEach(id => ok(v.widgets[id] && typeof v.widgets[id].itemIndex === 'number', id));
        ok(v.widgets.cobPRFinishType.itemIndex === 1 && v.widgets.cobMajorMaintenanceClassType.itemIndex === -1, 'extra option -> -1');
        ok(JSON.stringify(v.widgets.MemoHandlerPrecautionRecord) === '["x1","x2"]', JSON.stringify(v.widgets.MemoHandlerPrecautionRecord));
        ok(!('MemoNoteLog' in v.widgets) && !('pnPrecautionStartTime' in v.widgets), 'outputs are not sent');
      });
      await check('[A4] executed -> the reply dataRecord is drawn, status ✓', () => {
        ok(env.els.MemoHandlerPrecautionRecord.value === 'l1_MemoHandlerPrecautionRecord\nl2', env.els.MemoHandlerPrecautionRecord.value);
        ok(/prRecordSet ✓/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '', env.els.obsStatus.textContent);
      });
      env.replies.push({ ok: false, v: { executed: false, op: 'prSave', guard: 'record-name', detail: 'port refused: ..', dataRecord: dataRecord() } });
      await flush(450);
      env.els.sbPrecautionSave.click();
      await flush();
      await check('[A4] port refusal (record-name) -> red status "沒有存檔"', () => {
        ok(/沒有存檔/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '#c00', env.els.obsStatus.textContent);
      });
      env.replies.push({ busy: true });
      await flush(450);
      env.els.sbMajorMaintenanceDate.click();
      await flush();
      await check('[A4] busy: is not an error (HT9045Busy.NOTE, normal colour)', () => {
        ok(env.els.obsStatus.style.color === '' && /同一個指令剛送過/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
      });
      // AI(W906-E021-B5) 20261002: not-big5 (no dataRecord in the reply) -> red Big5 message, the typed text stays
      await flush(450);
      env.els.edNoteContents.value = '简体';
      env.replies.push({ ok: false, v: { executed: false, op: 'prSave', guard: 'not-big5', detail: 'widgets: edNoteContents: 简 (U+7B80) has no Big5 (cp950) form' } });
      env.els.sbPrecautionSave.click();
      await flush();
      await check('[A4] not-big5 -> red "不是 Big5" status; the operator\'s text is not overwritten', () => {
        ok(/不是 Big5/.test(env.els.obsStatus.textContent) && env.els.obsStatus.style.color === '#c00', env.els.obsStatus.textContent);
        ok(env.els.edNoteContents.value === '简体', 'overwritten: ' + env.els.edNoteContents.value);
      });
      // A5 change -> debounced sync
      await flush(450);
      const before = acts(env).length;
      env.replies.push({ ok: true, v: { executed: true, op: 'sync', result: 'widgets applied', dataRecord: dataRecord() } });
      env.els.edNoteContents.value = 'typed';
      env.els.edNoteContents.fire('change');
      env.els.edNoteContents.fire('change');
      await flush(260);
      await check('[A5] two changes -> one act.observer.sync with the typed text', () => {
        const a = acts(env).slice(before);
        ok(a.length === 1 && a[0].cmd === 'act.observer.sync' && a[0].value.widgets.edNoteContents === 'typed', JSON.stringify(a));
      });
    }
    // A6 Exit
    {
      const env = makeEnv(evScript, busyScript);
      env.win.HT9045ObserverEv.render(dataRecord());
      env.replies.push({ ok: true, v: { executed: true, op: 'exit', close: false, result: 'golden refusal :699-704' } });
      const stopped = env.exitClick();
      await flush();
      await check('[A6] Exit is stopped before .exitbtn and sends act.observer.exit with the widgets', () => {
        ok(stopped, 'not stopped');
        const a = acts(env);
        ok(a.length === 1 && a[0].cmd === 'act.observer.exit' && a[0].value.widgets, JSON.stringify(a));
      });
      await check('[A6] close:false (golden BtnExitClick refused) -> the window stays, status says so', () => {
        ok(env.posted.length === 0, JSON.stringify(env.posted));
        ok(/沒有關窗/.test(env.els.obsStatus.textContent), env.els.obsStatus.textContent);
      });
      await flush(450);
      env.replies.push({ ok: true, v: { executed: true, op: 'exit', close: true } });
      env.exitClick();
      await flush();
      await check('[A6] close:true -> postMessage({closeMe:1}) once', () => { ok(env.posted.length === 1 && env.posted[0].m.closeMe === 1 && !env.posted[0].byExitbtn, JSON.stringify(env.posted)); });
      await flush(450);
      env.replies.push({ ok: false, v: { executed: false, guard: 'unknown-action', detail: 'act.observer.exit' } });
      env.exitClick();
      await flush();
      await check('[A6] an old wb_serve (unknown-action) -> closes anyway (the pre-E-021 behaviour)', () => { ok(env.posted.length === 2, JSON.stringify(env.posted)); });
      await flush(450);
      env.replies.push({ busy: true });
      env.exitClick();
      await flush();
      await check('[A6] busy: -> no close', () => { ok(env.posted.length === 2, JSON.stringify(env.posted)); });
    }
    // A7..A11
    {
      const env = makeEnv(evScript, busyScript);
      env.replies.push({ ok: true, v: { executed: true, op: 'msgTab', timeData: { root: 'R', files: ['D:\\x\\a.csv', 'D:\\x\\b.csv'], itemIndex: 1,
        grid: { rows: 2, cols: 2, fixedRows: 1, fixedCols: 1, cells: [['H1', 'H2'], ['r1', 'v<1>']] } } } });
      env.msgTabs[1].click();
      await flush();
      await check('[A7] a pgcMessage tab -> act.observer.msgTab {arg: data-t}; timeData drawn (list with data-i, the selected one, the grid)', () => {
        const a = acts(env);
        ok(a.length === 1 && a[0].cmd === 'act.observer.msgTab' && a[0].value.arg === 2, JSON.stringify(a));
        ok(/data-i="0"/.test(env.els.lstTimeData.innerHTML) && /data-i="1" class="sel"/.test(env.els.lstTimeData.innerHTML), env.els.lstTimeData.innerHTML);
        ok(/<th>H1<\/th>/.test(env.els.strngrdTimeData.innerHTML) && /<td>v&lt;1&gt;<\/td>/.test(env.els.strngrdTimeData.innerHTML), env.els.strngrdTimeData.innerHTML);
      });
      await flush(450);
      env.replies.push({ ok: true, v: { executed: true, op: 'timeFile' } });
      const item = makeEl('', 'DIV'); item.setAttribute('data-i', '1');
      env.els.lstTimeData.fire('click', { target: { closest: s => (s === '[data-i]' ? item : null) } });
      env.els.lstTimeData.fire('click', { target: { closest: () => null } });
      await flush();
      await check('[A8] a Time Data file -> act.observer.timeFile {arg:1}; a dfm placeholder row sends nothing', () => {
        const a = acts(env).slice(1);
        ok(a.length === 1 && a[0].cmd === 'act.observer.timeFile' && a[0].value.arg === 1, JSON.stringify(a));
      });
      await flush(450);
      env.replies.push({ ok: true, v: { executed: true, op: 'backupLogYear', result: 'year 2026', bat: [], ran: true } });
      env.els.btnBackupLogYear.click();
      await flush();
      await check('[A9] Backup Log -> act.observer.backupLogYear {year: the cbbEventLogYear text}', () => {
        const a = acts(env).slice(2);
        ok(a.length === 1 && a[0].cmd === 'act.observer.backupLogYear' && a[0].value.year === '2026', JSON.stringify(a));
      });
      await flush(450);
      env.replies.push({ ok: true, v: { executed: true, op: 'clearTime', result: 'ran' } });
      env.els.btnClearTime.click();
      await flush();
      await check('[A10] Clear Time Data executed -> HT9045Observer.tick() (the captions refresh)', () => { ok(env.ticks() === 1, 'ticks ' + env.ticks()); });
      env.els.btnTimeData.click();
      await flush();
      await check('[A11] btnTimeData (Query) -> observer.get query (golden dfm :1813-1822 OnClick=btnQueryEventLogTxtClick)', () => {
        ok(env.obsSent.length === 1 && env.obsSent[0].act === 'query', JSON.stringify(env.obsSent));
      });
    }
  }

  // ---- [B] ht9045_observer_wire.js, cut ----------------------------------------------------------------------------------------
  let wire = '';
  await check('[B] read ht9045_observer_wire.js', () => { wire = read('ht9045_observer_wire.js').toString('utf8'); });
  function cut(src, head) {
    const a = src.indexOf(head);
    if (a < 0) throw new Error('no ' + head);
    let depth = 0;
    for (let i = src.indexOf('{', a); i < src.length; i++) {
      if (src[i] === '{') depth++;
      else if (src[i] === '}' && --depth === 0) return src.slice(a, i + 1);
    }
    throw new Error('unbalanced ' + head);
  }
  await check('[B] render(): a full reply hands dataRecord to HT9045ObserverEv.render; a reply without it does not', () => {
    const got = [];
    const ctx = vm.createContext({ window: { HT9045ObserverEv: { render: d => got.push(d) } }, JSON, Object, Date,
      st: { renders: 0, same: 0 }, sig: {}, lastNote: null, errShown: false, say() {}, renderCaptions() {}, renderCategory() {},
      renderEventLog() {}, renderCounter() {}, renderYield() {}, renderTestInfo() {} });
    vm.runInContext(cut(wire, 'function render(d, isTick)') + '; this.render = render;', ctx);
    ctx.render({ full: true, act: 'open', dataRecord: { b01: true } }, false);
    ctx.render({ full: false, act: 'timer' }, true);
    ok(got.length === 1 && got[0].b01 === true, JSON.stringify(got));
  });
  await check('[B] wire(): Backup Log\'s "not wired" listener only without ht9045_observer_ev.js', () => {
    function run(withEv) {
      const els = {};
      const $ = id => (els[id] = els[id] || makeEl(id, 'BUTTON'));
      const ctx = vm.createContext({ window: withEv ? { HT9045ObserverEv: {} } : {}, $, document: { querySelectorAll: () => [] }, say() {},
        send() {}, yieldAct() {}, opened: false });
      vm.runInContext(cut(wire, 'function wire()') + '; this.wire = wire;', ctx);
      ctx.wire();
      return els.btnBackupLogYear.listeners('click');
    }
    ok(run(true) === 0 && run(false) === 1, 'with ev ' + run(true) + ', without ' + run(false));
  });

  // ---- [C] Data.Observer.html -------------------------------------------------------------------------------------------------
  await check('[C] Data.Observer.html loads ht9045_observer_ev.js after ht9045_observer_wire.js (outside comments)', () => {
    const html = read('Data.Observer.html').toString('utf8').replace(/<!--[\s\S]*?-->/g, '');
    const a = html.indexOf('<script src="ht9045_observer_wire.js"></script>'), b = html.indexOf('<script src="ht9045_observer_ev.js"></script>');
    ok(a >= 0 && b > a, 'wire at ' + a + ', ev at ' + b);
  });

  console.log('\nE021_ObserverPage: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
})();
