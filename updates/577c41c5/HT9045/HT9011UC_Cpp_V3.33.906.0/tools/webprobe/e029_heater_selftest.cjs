// AI(W906-FRW-E029) 20261002 [W906] (St01): ctest E029_HeaterPages -- the page half of todo E-029 / Q72 (node, offline, nothing written
//   outside the temp folder). Steven 1002: 「溫控器少了 DTM」「這邊選不到DTM」「還有EJ1N也選不到」「這些全部都要可以選,所以數量也是少了」
//   「使用 EJ1N 或是 DTM 應該是要設定兩個變數」. Server half: ctest HSys_HeaterMix (FileRW/HSys.cpp, rules in FileRW/HSys_Heater.h E029).
//   [A] files: D:\HT9045\web\page\ht9045_hsys_heater_c.js and ht9045_temperfrom_strip.js compile, no BOM, one EOL style.
//   [B] heater_c.js rules (window.HT9045HSysHeater.rules, the same rules as the C++ HmLinkIndex / HmBrandSame / HmDefaultUnitCh):
//       Index area EJ1N / DTM -> USE_16_HEATER of the same count (16 -> 2 / 5, 32 -> 3 / 6, 4 Heaters -> 16); back to a COM brand ->
//       1 / 4; Index Heater Counts -> the Heater tab (whole area, KT4H when leaving); one channel = the whole area; Head1-4 follow
//       "other" when Index is EJ1N / DTM; golden wiring (iTempCode) for the default unit / station + CH; Heater Type keeps the area.
//   [C] heater_c.js page glue in a small fake DOM: the auto-link changes the ORIGINAL #rgHeater by a real click (click(), never only
//       .checked); a click on #rgHeater moves the Heater tab; it never touches elements with data-hst-src.
//   [D] ht9045_temperfrom_strip.js addresses for codes 5 / 6 (main-screen-display.md §9.1 text rules: "EJ1N CHx-x", "DTM CHx-x"), the
//       "per heater" station keys, and the old KT4H / TC401 / E5DC / No Heater / zone texts.
//   Steven 1002 17:1x: 全機相同 -> EJ1N / DTM only in the Index drop-down (Other = golden 5); 各溫控器不同 -> 7 per channel; Head1-4 vs Ax / Bx
//       and Socket vs DUT1-4 are exclusive, shown live from #rgHeater / #rgUse4DUT ([B] rowShown, [C] row display).
//   Control: broken copies (heater: 32-heater count forgotten; strip: code 6 not handled) must be red.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = path.resolve(process.env.W906_E029_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page'));
const HEATER = 'ht9045_hsys_heater_c.js', STRIP = 'ht9045_temperfrom_strip.js';
let failed = 0, passed = 0;
function check(name, fn) {
  try { fn(); passed++; console.log('PASS ' + name); }
  catch (e) { failed++; console.log('FAIL ' + name + ' -- ' + (e && e.message || e)); }
}
function eq(a, b, what) { if (JSON.stringify(a) !== JSON.stringify(b)) throw new Error(what + ': got ' + JSON.stringify(a) + ', want ' + JSON.stringify(b)); }
function ok(c, what) { if (!c) throw new Error(what); }

// ---------------- a very small DOM (only what the two scripts touch) ----------------
function makeDom() {
  const byId = {};
  function node(tag) {
    const n = { tagName: String(tag).toUpperCase(), children: [], parentNode: null, attrs: {}, style: {}, listeners: {}, textContent: '',
      id: '', type: '', checked: false, disabled: false, value: '', options: [], selectedIndex: -1, placeholder: '', clicks: 0,
      setAttribute(k, v) { this.attrs[k] = String(v); if (k === 'id') { this.id = String(v); byId[this.id] = this; } if (k === 'type') this.type = String(v); },
      getAttribute(k) { return Object.prototype.hasOwnProperty.call(this.attrs, k) ? this.attrs[k] : null; },
      removeAttribute(k) { delete this.attrs[k]; },
      hasAttribute(k) { return Object.prototype.hasOwnProperty.call(this.attrs, k); },
      appendChild(c) { c.parentNode = this; this.children.push(c); if (this.tagName === 'SELECT' && c.tagName === 'OPTION') { this.options.push(c); if (this.selectedIndex < 0) this.selectedIndex = 0; } return c; },
      removeChild(c) { this.children = this.children.filter(x => x !== c); c.parentNode = null; return c; },
      addEventListener(t, f) { (this.listeners[t] = this.listeners[t] || []).push(f); },
      dispatch(t, target) { let n2 = this; const ev = { type: t, target: target || this }; while (n2) { (n2.listeners[t] || []).forEach(f => f(ev)); n2 = n2.parentNode; } },
      click() { if (this.disabled) return; this.clicks++; if (this.type === 'radio') { const g = this.attrs.name; all().forEach(x => { if (x.type === 'radio' && x.attrs.name === g) x.checked = false; }); this.checked = true; }
                this.dispatch('click'); this.dispatch('change'); },
      closest(sel) { let n2 = this; while (n2) { if (match(n2, sel)) return n2; n2 = n2.parentNode; } return null; },
      querySelector(sel) { return this.querySelectorAll(sel)[0] || null; },
      querySelectorAll(sel) { const out = []; (function walk(x) { x.children.forEach(c => { if (match(c, sel)) out.push(c); walk(c); }); })(this); return out; },
    };
    Object.defineProperty(n, 'className', { get() { return this.attrs['class'] || ''; }, set(v) { this.attrs['class'] = v; } });
    return n;
  }
  function all() { const out = []; (function walk(x) { x.children.forEach(c => { out.push(c); walk(c); }); })(doc.body); return out; }
  function match(n, sel) {
    return sel.split(',').some(s => {
      s = s.trim();
      let m;
      if ((m = /^#([\w-]+) input\[type="radio"\]$/.exec(s))) { if (!(n.tagName === 'INPUT' && n.type === 'radio')) return false; let p = n.parentNode; while (p) { if (p.id === m[1]) return true; p = p.parentNode; } return false; }
      if (s === 'input[type="radio"]') return n.tagName === 'INPUT' && n.type === 'radio';
      if (s === 'label') return n.tagName === 'LABEL';
      if (s === 'tr') return n.tagName === 'TR';
      if (s === '.cli') return (n.attrs['class'] || '').split(' ').indexOf('cli') >= 0;
      if ((m = /^\.([\w-]+)$/.exec(s))) return (n.attrs['class'] || '').split(' ').indexOf(m[1]) >= 0;
      if ((m = /^\[([\w-]+)="([^"]*)"\]$/.exec(s))) return n.attrs[m[1]] === m[2];
      if ((m = /^\[([\w-]+)\]$/.exec(s))) return Object.prototype.hasOwnProperty.call(n.attrs, m[1]);
      return false;
    });
  }
  const doc = { readyState: 'complete', body: node('body'), createElement: node, createTextNode: t => { const n = node('#text'); n.textContent = t; return n; },
    getElementById: id => byId[id] || null, addEventListener() {}, querySelectorAll: sel => doc.body.querySelectorAll(sel), querySelector: sel => doc.body.querySelector(sel) };
  function radioGroup(id, items) {
    const fs2 = node('fieldset'); fs2.setAttribute('id', id); fs2.setAttribute('class', 'gbx rg'); doc.body.appendChild(fs2);
    items.forEach((t, i) => { const lb = node('label'); const r = node('input'); r.setAttribute('type', 'radio'); r.setAttribute('name', 'rg_' + id); lb.appendChild(r); const tx = node('#text'); tx.textContent = t; lb.appendChild(tx); lb.textContent = t; fs2.appendChild(lb); if (i === 0) r.checked = true; });
    return fs2;
  }
  return { doc, node, byId, radioGroup };
}

function loadHeater(src, withDom) {
  const D = makeDom();
  const win = { document: D.doc, console: { warn() {}, log() {}, error() {} }, setTimeout: (f) => f(), HT9045Recipe: null };
  let getResolve = null;
  if (withDom) {
    win.HT9045Recipe = { editlistGet: () => ({ then: (f) => f(withDom) }) };
    const g = D.node('fieldset'); g.setAttribute('id', 'grpHeater'); D.doc.body.appendChild(g);
    const cli = D.node('div'); cli.setAttribute('class', 'cli'); g.appendChild(cli);
    D.radioGroup('rgHeaterType', ['TC401', 'Panasonic KT4H', 'Omron E5DC', 'No Heater', 'DTK4848']);
    D.radioGroup('rgUse4DUT', ['1 EA', '4 EA', '2 EA']);                     // golden eDut1ea 0 / eDut4ea 1 / eDut2ea 2
    D.radioGroup('rgHeater', ['4 Heaters', '16 Heaters', '16 Heaters with Omron EJ1N', '32 Heaters with Omron EJ1N', '32 Heaters with KT4H',
                              '16 Heaters with DTME08', '32 Heaters with DTME08']);
    const proxy = D.node('select'); proxy.setAttribute('data-hst-src', 'rgHeater'); proxy.setAttribute('id', 'tableProxyRgHeater'); D.doc.body.appendChild(proxy);
    ['cbComTemp', 'cbComTempOmron'].forEach((id, k) => { const s = D.node('select'); s.setAttribute('id', id); const o = D.node('option'); o.text = k ? 'COM13' : 'COM12'; s.appendChild(o); D.doc.body.appendChild(s); });
  }
  win.window = win;
  vm.createContext(win);
  vm.runInContext(src, win, { filename: HEATER });
  return { win, D };
}
function loadStrip(src) {
  const D = makeDom();
  const win = { document: D.doc, localStorage: { getItem: () => 'en' }, addEventListener() {}, console: { warn() {}, log() {} } };
  win.window = win;
  vm.createContext(win);
  vm.runInContext(src, win, { filename: STRIP });
  return win.HT9045TemperStrip;
}

// ---------------- [B] rules ----------------
function rulesVerdicts(R) {
  const N = 71, EJ1N = 5, DTM = 6, KT4H = 1, E5DC = 2;
  const st = (mode, I, O, u, fill) => ({ mode, I, O, u, ch: Array.from({ length: N }, (_, ti) => fill(ti)) });
  const zones = (s, a, b) => s.ch.slice(a, b + 1);
  const V = [];
  V.push(['[B] u16For: EJ1N / DTM keep the count, 4 Heaters -> 16, back to COM -> 1 / 4', () => {
    eq([0, 1, 2, 3, 4, 5, 6].map(u => R.u16For(EJ1N, u)), [2, 2, 2, 3, 3, 2, 3], 'EJ1N');
    eq([0, 1, 2, 3, 4, 5, 6].map(u => R.u16For(DTM, u)), [5, 5, 5, 6, 6, 5, 6], 'DTM');
    eq([0, 1, 2, 3, 4, 5, 6].map(u => R.u16For(-1, u)), [0, 1, 1, 4, 4, 1, 4], 'COM');
  }]);
  V.push(['[B] same mode: Index EJ1N / DTM / KT4H moves USE_16_HEATER', () => {
    eq(R.linkTab(st(0, EJ1N, E5DC, 1, () => 1)).u, 2, '16 KT4H + Index EJ1N');
    eq(R.linkTab(st(0, EJ1N, E5DC, 4, () => 1)).u, 3, '32 KT4H + Index EJ1N');
    eq(R.linkTab(st(0, DTM, E5DC, 0, () => 1)).u, 5, '4 Heaters + Index DTM');
    eq(R.linkTab(st(0, DTM, E5DC, 4, () => 1)).u, 6, '32 KT4H + Index DTM');
    eq(R.linkTab(st(0, KT4H, E5DC, 3, () => 5)).u, 4, '32 EJ1N + Index KT4H');
    eq(R.linkTab(st(0, KT4H, E5DC, 5, () => 6)).u, 1, '16 DTME08 + Index KT4H');
    eq(R.linkTab(st(0, E5DC, E5DC, 1, () => 1)).u, 1, 'COM -> COM: no change');
  }]);
  V.push(['[B] different mode: one Index channel EJ1N = the whole area; back to COM = the whole area + USE_16_HEATER 1 / 4', () => {
    let s = st(1, KT4H, KT4H, 1, () => KT4H); s.ch[12] = EJ1N;           // Ab1
    let r = R.linkTab(s, 12);
    eq(r.u, 2, 'u'); eq(zones(r, 11, 26), Array(16).fill(EJ1N), 'Aa1..Bd2'); eq(zones(r, 33, 48), Array(16).fill(KT4H), 'Ae1..Bh2 untouched'); eq(r.ch[4], KT4H, 'Head1');
    eq(r.I, EJ1N, 'the same-mode Index drop-down follows too');
    s = st(1, EJ1N, KT4H, 2, ti => (ti >= 11 && ti <= 26) ? EJ1N : KT4H); s.ch[11] = 4;   // Aa1 -> DTK4848
    r = R.linkTab(s, 11);
    eq(r.u, 1, 'u back to 1'); eq(zones(r, 11, 26), Array(16).fill(4), 'whole area -> DTK4848'); eq(r.I, KT4H, 'the same-mode Index leaves EJ1N (KT4H)');
    s = st(1, KT4H, KT4H, 4, () => KT4H); s.ch[40] = DTM;             // Bh1 in a 32 area
    r = R.linkTab(s, 40);
    eq(r.u, 6, '32 KT4H + Bh1 DTM'); eq(zones(r, 11, 26).concat(zones(r, 33, 48)), Array(32).fill(DTM), 'all 32 zones DTM');
    s = st(1, KT4H, KT4H, 0, () => KT4H); s.ch[11] = DTM;
    r = R.linkTab(s, 11);
    eq(r.u, 5, '4 Heaters + Aa1 DTM -> 16 DTME08'); eq(zones(r, 11, 26), Array(16).fill(DTM), 'Aa1..Bd2 DTM');
    s = st(1, KT4H, KT4H, 1, () => KT4H); s.ch[2] = DTM;              // Shuttle1 (not Index): no link
    r = R.linkTab(s, 2);
    eq([r.u, r.ch[2]], [1, DTM], 'a non-Index channel on DTM does not touch USE_16_HEATER');
  }]);
  V.push(['[B] Index Heater Counts -> Heater tab', () => {
    let r = R.linkU16(st(1, KT4H, KT4H, 6, () => KT4H), 1);
    eq(zones(r, 11, 26).concat(zones(r, 33, 48)), Array(32).fill(DTM), '1 -> 6: 32 zones DTM'); eq(r.I, DTM, 'Index');
    r = R.linkU16(st(1, EJ1N, KT4H, 4, ti => R.isZone(ti) ? EJ1N : KT4H), 3);
    eq(zones(r, 11, 26).concat(zones(r, 33, 48)), Array(32).fill(KT4H), '3 -> 4: EJ1N -> KT4H'); eq(r.I, KT4H, 'Index -> KT4H');
    r = R.linkU16(st(1, EJ1N, KT4H, 1, ti => R.isZone(ti) ? EJ1N : KT4H), 3);
    eq(zones(r, 33, 48), Array(16).fill(KT4H), '3 -> 1: the old 32 area is cleaned too');
    r = R.linkU16(st(0, KT4H, E5DC, 2, () => KT4H), 1);
    eq([r.I, r.O], [EJ1N, E5DC], 'same mode: 1 -> 2 sets Index = EJ1N, Other stays');
  }]);
  V.push(['[B] same mode brands: Head1-4 stay on the COM port (follow Other) when Index is EJ1N / DTM', () => {
    eq([4, 7, 11, 26, 33, 0, 29].map(ti => R.planSame(ti, EJ1N, E5DC, 2)), [E5DC, E5DC, EJ1N, EJ1N, E5DC, E5DC, E5DC], 'Index EJ1N, 16');
    eq([4, 33, 48].map(ti => R.planSame(ti, DTM, E5DC, 6)), [E5DC, DTM, DTM], 'Index DTM, 32');
    eq([4, 11, 33, 0].map(ti => R.planSame(ti, KT4H, E5DC, 1)), [KT4H, KT4H, KT4H, E5DC], 'Index KT4H: all 36 Index-group channels');
  }]);
  V.push(['[B] golden wiring defaults (iTempCode; main-screen-display.md §3)', () => {
    eq(R.TEMPCODE, [11, 15, 12, 16, 13, 17, 14, 18, 19, 23, 20, 24, 21, 25, 22, 26, 33, 37, 34, 38, 35, 39, 36, 40, 41, 45, 42, 46, 43, 47, 44, 48], 'TEMPCODE');
    const d = (ti, b) => { const x = R.defaultAddr(ti, b); return [x.kind, x.st, x.ch]; };
    eq(d(16, EJ1N), ['ej1n', 1, 4], 'Bb1 EJ1N unit 1 CH4');
    eq(d(13, EJ1N), ['ej1n', 2, 1], 'Ac1 EJ1N unit 2 CH1');
    eq(d(48, EJ1N), ['ej1n', 8, 4], 'Bh2 EJ1N unit 8 CH4');
    eq(d(12, DTM), ['dtm', 0, 3], 'Ab1 DTM station 0 CH3');
    eq(d(19, DTM), ['dtm', 1, 1], 'Aa2 DTM station 1 CH1');
    eq(d(48, DTM), ['dtm', 3, 8], 'Bh2 DTM station 3 CH8');
    eq(d(0, DTM), ['dtm', null, null], 'HotPlate1 DTM: no golden default');
    eq(d(4, 0), ['tc401', 2, 0], 'Head1 TC401 unit 2 channel 0');
    eq(d(4, KT4H), ['com', 5, null], 'Head1 KT4H station 5');
  }]);
  V.push(['[B] exclusive pairs (Steven 1002 17:1x): Head1-4 vs Ax / Bx by Index Heater Counts, Socket vs DUT1-4 by Dut Heater Count', () => {
    const show = (u, dut) => [4, 7, 11, 26, 33, 48, 8, 29, 30, 31, 32, 0, 10, 70].map(ti => R.rowShown(ti, u, dut) ? 1 : 0);
    eq(show(0, 0), [1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1], '4 Heaters + 1 EA: Head1-4, Socket');
    eq(show(2, 2), [0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1], '16 EJ1N + 2 EA: Aa1..Bd2, DUT1-2');
    eq(show(6, 1), [0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1], '32 DTME08 + 4 EA: Aa1..Bh2, DUT1-4');
    eq(show(4, 1), show(3, 1), '32 KT4H = 32 EJ1N for the rows');
  }]);
  V.push(['[B] Heater Type keeps an EJ1N / DTME08 Index area (golden does not touch USE_16_HEATER)', () => {
    const r = R.heaterType(st(1, KT4H, KT4H, 2, () => KT4H), E5DC);
    eq([r.mode, r.I, r.O], [0, EJ1N, E5DC], 'u=2');
    const r2 = R.heaterType(st(1, KT4H, KT4H, 1, () => KT4H), E5DC);
    eq([r2.I, r2.O], [E5DC, E5DC], 'u=1');
  }]);
  return V;
}

// ---------------- [C] page glue ----------------
function fakeExtra() {
  const rows = [];
  const names = ['HotPlate1', 'HotPlate2', 'Shuttle1', 'Shuttle2', 'Head1', 'Head2', 'Head3', 'Head4', 'Socket', 'Chamber', 'CCD',
    'Aa1', 'Ab1', 'Ac1', 'Ad1', 'Ba1', 'Bb1', 'Bc1', 'Bd1', 'Aa2', 'Ab2', 'Ac2', 'Ad2', 'Ba2', 'Bb2', 'Bc2', 'Bd2', 'HeatGun1', 'HeatGun2',
    'DUT1', 'DUT2', 'DUT3', 'DUT4', 'Ae1', 'Af1', 'Ag1', 'Ah1', 'Be1', 'Bf1', 'Bg1', 'Bh1', 'Ae2', 'Af2', 'Ag2', 'Ah2', 'Be2', 'Bf2', 'Bg2', 'Bh2',
    '2D', 'LB', 'IndexESD', 'CCD_2', 'ATCHotAir1', 'ATCHotAir2', 'OutSht1', 'OutSht2', 'Base1', 'Base2', 'Base3', 'Base4', 'Base5', 'Base6',
    'HotPlate3', 'HotPlate4', 'Shuttle3', 'Shuttle4', 'Door1', 'Door2', 'LBUp', 'LBDown'];
  names.forEach((n, ti) => rows.push({ typeIdx: ti, name: n, saveName: 'HeaterInsOpt_' + n, addrKey: 'HeaterInsAddr_' + n, chKey: 'HeaterInsCh_' + n,
    cb: 'cbHeaterInsOpt_' + n, lb: '', addr: 'edHeaterInsAddr_' + n, chEd: 'edHeaterInsCh_' + n, golden: ti <= 10, goldenShow: ti <= 9,
    zone: (ti >= 11 && ti <= 26) ? 1 : (ti >= 33 && ti <= 48) ? 2 : 0, indexGroup: (ti >= 4 && ti <= 7) || (ti >= 11 && ti <= 26) || (ti >= 33 && ti <= 48) }));
  return { extra: { heater: { options: ['TC401', 'Panasonic KT4H', 'Omron E5DC', 'No Heater', 'DTK4848', 'Omron EJ1N', 'Delta DTM'], mix: {
    modeProxy: 'rgHeaterInsMode', indexProxy: 'cbHeaterInsIndexOpt', otherProxy: 'cbHeaterInsOtherOpt', u16Proxy: 'rgHeater', mode: 0,
    options: ['TC401', 'Panasonic KT4H', 'Omron E5DC', 'No Heater', 'DTK4848', 'Omron EJ1N', 'Delta DTM'], indexGroup: [], use16Heater: 1,
    otherOptions: ['TC401', 'Panasonic KT4H', 'Omron E5DC', 'No Heater', 'DTK4848'], dutProxy: 'rgUse4DUT', use4Dut: 0,
    hint: '目前溫控只看 HEATER_CTRL_TYPE', stationMax: 247, stationMaxE5DC: 99, ej1nUnitMin: 1, ej1nUnitMax: 15, ej1nChMax: 4, dtmStationMin: 0,
    dtmStationMax: 3, dtmChMax: 8, conn: { comTempKey: '[TempCtrl] COM_PORT', comTempWidget: 'cbComTemp', comOmronKey: '[TempCtrl] COM_PORT_OMRON',
    comOmronWidget: 'cbComTempOmron', dtm: { file: 'X', exists: true, address: '172.16.8.50', port: '502' } }, rows } } } };
}
function glueVerdicts(src) {
  const ex = fakeExtra();
  const { win, D } = loadHeater(src, ex);
  const $ = id => D.byId[id];
  const radios = id => $(id).querySelectorAll('input[type="radio"]');
  const ci = id => { const r = radios(id); for (let i = 0; i < r.length; i++) if (r[i].checked) return i; return -1; };
  radios('rgHeater')[1].checked = true; radios('rgHeater')[0].checked = false;   // the original #rgHeater = 1 (16 Heaters) when the page opens
  win.HT9045Recipe.editlistGet('HSys');                                         // build (setTimeout runs at once in the fake)
  radios('rgHeaterInsMode')[0].checked = true;                                  // then the engine applies the proxies (no events)
  $('cbHeaterInsIndexOpt').selectedIndex = 1; $('cbHeaterInsOtherOpt').selectedIndex = 2;
  win.HT9045HSysHeater.refresh();
  const V = [];
  V.push(['[C] build: 71 rows with brand / station / CH boxes, 7 options', () => {
    eq(['cbHeaterInsOpt_LBDown', 'edHeaterInsAddr_OutSht1', 'edHeaterInsCh_Bh2'].map(id => !!$(id)), [true, true, true], 'ids exist');
    eq($('cbHeaterInsOpt_Aa1').options.length, 7, 'per-channel options');
    eq([$('cbHeaterInsIndexOpt').options.length, $('cbHeaterInsOtherOpt').options.length], [7, 5], 'same mode: Index 7 (EJ1N / DTM), Other 5 golden');
    ok(/COM12/.test($('hsysHeaterConn').textContent) && /COM13/.test($('hsysHeaterConn').textContent) && /172\.16\.8\.50:502/.test($('hsysHeaterConn').textContent),
       'connection line shows COM_PORT, COM_PORT_OMRON and the DTM address (read-only)');
  }]);
  V.push(['[C] rows follow #rgHeater / #rgUse4DUT live (exclusive pairs)', () => {
    const disp = n => $('cbHeaterInsOpt_' + n).closest('tr').style.display;
    eq(['Head1', 'Aa1', 'Ae1', 'Socket', 'DUT1', 'CCD'].map(disp), ['none', '', 'none', '', 'none', ''], '16 Heaters + 1 EA');
    ok(/這台沒裝/.test($('cbHeaterInsOpt_CCD').closest('tr').querySelector('.hsys-inst').textContent), 'golden-hidden CCD listed, marked 這台沒裝');
    radios('rgUse4DUT')[1].click();                                             // 4 EA
    eq(['Socket', 'DUT1', 'DUT4'].map(disp), ['none', '', ''], '4 EA: DUT1-4, no Socket');
    radios('rgUse4DUT')[2].click();                                             // 2 EA
    eq(['Socket', 'DUT2', 'DUT3'].map(disp), ['none', '', 'none'], '2 EA: DUT1-2');
    radios('rgUse4DUT')[0].click();
    radios('rgHeater')[0].click();                                              // 4 Heaters
    eq(['Head1', 'Head4', 'Aa1', 'Bd2'].map(disp), ['', '', 'none', 'none'], '4 Heaters: Head1-4, no zones');
    radios('rgHeater')[1].click();
  }]);
  V.push(['[C] Index -> EJ1N clicks the ORIGINAL #rgHeater (real click), table proxy untouched', () => {
    const s = $('cbHeaterInsIndexOpt'); s.selectedIndex = 5; s.dispatch('change');
    eq(ci('rgHeater'), 2, 'rgHeater = 16 EJ1N');
    eq(radios('rgHeater')[2].clicks, 1, 'by click()');
    eq($('tableProxyRgHeater').clicks + ($('tableProxyRgHeater').attrs.touched ? 1 : 0), 0, 'data-hst-src proxy not touched');
    eq($('cbHeaterInsOpt_Aa1').selectedIndex, 5, 'Aa1 shows EJ1N (same mode, computed)');
    eq($('cbHeaterInsOpt_Head1').selectedIndex, 2, 'Head1 shows Other (E5DC)');
    ok(/Head1～4 不顯示/.test($('hsysHeaterHeadNote').textContent), 'Head1-4 note shown');
  }]);
  V.push(['[C] a click on #rgHeater (operator / table) moves the Heater tab', () => {
    radios('rgHeater')[6].click();                                              // 32 DTME08
    eq($('cbHeaterInsIndexOpt').selectedIndex, 6, 'Index = DTM');
    eq([$('cbHeaterInsOpt_Ae1').selectedIndex, $('cbHeaterInsOpt_Bh2').selectedIndex, $('cbHeaterInsOpt_Head4').selectedIndex], [6, 6, 2], 'Ae1 / Bh2 DTM, Head4 Other');
    radios('rgHeater')[4].click();                                              // 32 KT4H
    eq($('cbHeaterInsIndexOpt').selectedIndex, 1, 'leaving DTM -> Index KT4H');
  }]);
  V.push(['[C] different mode: one Index channel DTM -> whole area + #rgHeater clicked', () => {
    const m = radios('rgHeaterInsMode'); m[1].click();
    const s = $('cbHeaterInsOpt_Ab1'); s.selectedIndex = 6; s.dispatch('change');
    eq(ci('rgHeater'), 6, '32 KT4H + Ab1 DTM -> 32 DTME08');
    eq(['Aa1', 'Bd2', 'Ae1', 'Bh2'].map(n => $('cbHeaterInsOpt_' + n).selectedIndex), [6, 6, 6, 6], 'whole 32 area DTM');
    eq($('edHeaterInsCh_Ab1').style.display, '', 'CH box shown for DTM');
    eq($('edHeaterInsCh_HotPlate1').style.display, 'none', 'CH box hidden for a COM brand');
    ok(/站 0 CH3/.test($('cbHeaterInsOpt_Ab1').closest('tr').querySelector('.hsys-def').textContent), 'default shows golden station 0 CH3');
    const h = $('cbHeaterInsOpt_HotPlate1'); h.selectedIndex = 6; h.dispatch('change');
    ok(/未指定/.test(h.closest('tr').querySelector('.hsys-def').textContent), 'HotPlate1 DTM: no golden default -> 未指定');
    eq(ci('rgHeater'), 6, 'non-Index DTM does not move USE_16_HEATER');
    eq($('hsysHeaterHint').style.display, '', 'D-9a hint shown');
  }]);
  return V;
}

// ---------------- [D] strip addresses ----------------
function stripVerdicts(S) {
  const V = [];
  const L16 = u => S.layoutHT9045({ USE_16_HEATER: String(u), SocketBasedAdd4Temp: '1' });
  const find = (L, tc) => { for (const g of L.groups) for (const c of g.chans) if (c.tc === tc) return c; return null; };
  const A = (u, tc, t) => { const L = L16(u); const c = find(L, tc); ok(c, 'no ' + tc + ' in the layout'); return S.addressOf(c, L, t); };
  V.push(['[D] golden texts unchanged (KT4H / TC401 / E5DC / No Heater / -9999 / zones)', () => {
    eq(A(1, 'tcHotPlate1', { HEATER_CTRL_TYPE: '1' }), 'KT4H #1', 'KT4H');
    eq(A(1, 'tcHotPlate2', { HEATER_CTRL_TYPE: '0' }), 'TC401 CH1-1', 'TC401');
    eq(A(1, 'tcShuttle1', { HEATER_CTRL_TYPE: '1', HeaterInsOpt_Shuttle1: '2' }), 'E5DC #3', 'E5DC per channel');
    eq(A(1, 'tcShuttle2', { HEATER_CTRL_TYPE: '1', HeaterInsOpt_Shuttle2: '3' }), 'No Heater', 'No Heater');
    eq(A(1, 'tcChamber', { HEATER_CTRL_TYPE: '2', HeaterInsOpt_Chamber: '-9999' }), 'E5DC #10', '-9999 = HEATER_CTRL_TYPE');
    eq(A(2, 'tcAc1', { HEATER_CTRL_TYPE: '1' }), 'EJ1N CH2-1', 'EJ1N zone (golden bthermo.cpp:3903)');
    eq(A(6, 'tcBh2', { HEATER_CTRL_TYPE: '1' }), 'DTM CH3-8', 'DTM zone');
    eq(A(5, 'tcAb1', { HEATER_CTRL_TYPE: '1' }), 'DTM CH0-3', 'DTM zone Ab1');
  }]);
  V.push(['[D] codes 5 / 6 outside the Index area: "EJ1N CHx-x" / "DTM CHx-x" from the per-heater keys', () => {
    const t = { HEATER_CTRL_TYPE: '1', HeaterInsMode: '1', HeaterInsOpt_HotPlate1: '6', HeaterInsAddr_HotPlate1: '2', HeaterInsCh_HotPlate1: '1',
                HeaterInsOpt_Shuttle1: '5', HeaterInsAddr_Shuttle1: '3', HeaterInsCh_Shuttle1: '2', HeaterInsOpt_DUT1: '6', HeaterInsAddr_DUT1: '0', HeaterInsCh_DUT1: '5' };
    eq(A(1, 'tcHotPlate1', t), 'DTM CH2-1', 'HotPlate1 DTM');
    eq(A(1, 'tcShuttle1', t), 'EJ1N CH3-2', 'Shuttle1 EJ1N');
    eq(A(1, 'tcDUT1', t), 'DTM CH0-5', 'DUT1 DTM station 0 (host)');
    eq(A(1, 'tcHotPlate2', { HEATER_CTRL_TYPE: '1', HeaterInsMode: '1', HeaterInsOpt_HotPlate2: '5' }), 'EJ1N Station not set', 'no keys -> not set');
    eq(A(1, 'tcHotPlate2', { HEATER_CTRL_TYPE: '1', HeaterInsMode: '0', HeaterInsOpt_HotPlate2: '6', HeaterInsAddr_HotPlate2: '2', HeaterInsCh_HotPlate2: '2' }),
       'DTM Station not set', 'same mode: station keys not used');
  }]);
  V.push(['[D] per-heater station keys for the COM brands and zones', () => {
    eq(A(1, 'tcHotPlate1', { HEATER_CTRL_TYPE: '1', HeaterInsMode: '1', HeaterInsAddr_HotPlate1: '50' }), 'KT4H #50', 'KT4H station 50');
    eq(A(1, 'tcHotPlate1', { HEATER_CTRL_TYPE: '1', HeaterInsMode: '0', HeaterInsAddr_HotPlate1: '50' }), 'KT4H #1', 'same mode: default');
    eq(A(1, 'tcHotPlate2', { HEATER_CTRL_TYPE: '0', HeaterInsMode: '1', HeaterInsAddr_HotPlate2: '3' }), 'TC401 CH3-1', 'TC401 unit 3');
    eq(A(2, 'tcAa1', { HEATER_CTRL_TYPE: '1', HeaterInsMode: '1', HeaterInsAddr_Aa1: '5', HeaterInsCh_Aa1: '1' }), 'EJ1N CH5-1', 'EJ1N zone override');
  }]);
  return V;
}

function run(tag, heaterSrc, stripSrc) {
  const out = [];
  let heater = null, strip = null;
  try { heater = loadHeater(heaterSrc).win.HT9045HSysHeater; } catch (e) { out.push([tag + ' heater_c.js loads', () => { throw e; }]); }
  try { strip = loadStrip(stripSrc); } catch (e) { out.push([tag + ' strip loads', () => { throw e; }]); }
  if (heater) out.push(...rulesVerdicts(heater.rules));
  try { out.push(...glueVerdicts(heaterSrc)); } catch (e) { out.push(['[C] page glue runs', () => { throw e; }]); }
  if (strip) out.push(...stripVerdicts(strip));
  return out;
}

function main() {
  const files = { [HEATER]: fs.readFileSync(path.join(PAGE_DIR, HEATER)), [STRIP]: fs.readFileSync(path.join(PAGE_DIR, STRIP)) };
  for (const f of Object.keys(files)) check('[A] ' + f + ': compiles, no BOM, one EOL style', () => {
    const b = files[f], t = b.toString('utf8');
    ok(!(b[0] === 0xEF && b[1] === 0xBB && b[2] === 0xBF), 'has a UTF-8 BOM');
    const crlf = (t.match(/\r\n/g) || []).length, lf = (t.match(/\n/g) || []).length;
    ok(crlf === 0 || crlf === lf, 'mixed EOL: ' + crlf + ' CRLF of ' + lf + ' LF');
    new vm.Script(t, { filename: f });
  });
  const hs = files[HEATER].toString('utf8'), ss = files[STRIP].toString('utf8');
  check('[C] heater_c.js never sets .checked on #rgHeater and never touches data-hst-src', () => {
    ok(!/data-hst-src/.test(hs.replace(/\/\*[\s\S]*?\*\//g, '').replace(/\/\/.*$/gm, '')), 'data-hst-src used in code');
    ok(/r\.click\(\)/.test(hs), 'no real click on the original radio');
  });
  for (const [name, fn] of run('', hs, ss)) check(name, fn);

  // Control: broken copies must be red
  check('control: broken copies are red', () => {
    const goodH = 'if (code === EJ1N) return b32 ? 3 : 2;', goodS = "var zoneCtl = !c.arm ? null : (L.u16 === 2 || L.u16 === 3) ? 5 : (L.u16 === 5 || L.u16 === 6) ? 6 : (b === 5 || b === 6) ? b : null;";
    ok(hs.includes(goodH), 'control anchor not found in ' + HEATER + ' (update the control)');
    ok(ss.includes(goodS), 'control anchor not found in ' + STRIP + ' (update the control)');
    const bad = run('broken', hs.replace(goodH, 'if (code === EJ1N) return 2;'),
                    ss.replace(goodS, "var zoneCtl = !c.arm ? null : (L.u16 === 2 || L.u16 === 3) ? 5 : null;").replace("if (b === 5 || b === 6) {", "if (b === 5) {"));
    const red = bad.filter(([, fn]) => { try { fn(); return false; } catch (e) { return true; } }).map(([n]) => n);
    ok(red.some(n => n.indexOf('u16For') >= 0), 'the broken heater copy passed u16For (red: ' + JSON.stringify(red) + ')');
    ok(red.some(n => n.indexOf('[D] codes 5 / 6') >= 0), 'the broken strip copy passed the 5 / 6 check (red: ' + JSON.stringify(red) + ')');
    console.log('     control red: ' + red.length + ' check(s): ' + red.join(' | '));
  });
  void os;
  console.log((failed ? 'FAILED ' : 'OK ') + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
}
main();
