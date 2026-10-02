'use strict';
// =============================================================================
//  tools/webprobe/kb_machine_setting_selftest.cjs -- ctest KB_MachineSetting.  AI(W906-C2) 20261002 (St02-E);
//  AI(W906-C8) 20261002: + Setup.TrayForm; AI(W906-C8) 20261002 (St02-E helper): each setting follows golden's own value.
//
//  web/page/ht9045_kb_generic.js keeps golden 906_0625_Steven's two branches where a MACHINE setting chooses, picked when
//  the keyboard opens; unknown (read failed / not answered yet) -> the narrower range (the safe side):
//    Main.CommView edtSetZ1 / edtSetZ2   main.cpp:26175-26185  INDEX_DRIVER_TYPE: Mitsubishi (1) 0..100, else 0..300
//        Gerneral.ini [IndexDriver] through HT9045System.read('gerneral'); golden rs232.cpp:103 default Panasonic_DRIVER
//        (key missing or not an integer -> 0..300, common.cpp:1444-1455)
//    Setup.TrayForm Tp1/2/3TickUp        cTrayForm.cpp:797-800 IniConfig.bC03UseCatchTray: on 60..73, off 30..110
//        C++'s own value: this page's C route (editlist.get UserDefForm_File = golden TfTrayForm::FormShow) sends
//        Lab1/2/3XPickup captions set from it (cTrayForm.cpp:256-267), read through HT9045Page.golden(); the LastSet.ini
//        file is NOT read (golden cConfiguration.cpp:1054-1066 fixes it on an auto-retest machine).  Section 5 pins the
//        C++ sources that carry it, so a change there turns this red instead of silently falling to the safe side.
//  Offline: a fake window / document; the REAL wire data registers through a stub HT9045Wire.register (the stub
//  HT9045Page.golden() answers like ht9045_wire_engine.js), then the REAL ht9045_kb_generic.js runs with a stub
//  HT9045System.read.  Every async case waits with settle() after load() and after resolve().  CommView reads only
//  'gerneral', TrayForm and other pages read nothing; the Q41 TF-2 / TF-3 fields on Setup.TrayForm are left to
//  ht9045_trayform_q41.js; load order wire -> q41 -> kb_generic is pinned.
//  argv[2] = web/page.  CONTROL: W906_KB_GENERIC pointing at a file without the listener (e.g. an empty .js) must be red.
// =============================================================================
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const pageDir = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const cppDir = path.join(__dirname, '..', '..');                     // HT9011UC_Cpp_V3.33.906.0
const kbFile = process.env.W906_KB_GENERIC || path.join(pageDir, 'ht9045_kb_generic.js');
let pass = 0, fail = 0;
function check(c, m) { if (c) { pass++; console.log('  ok   ' + m); } else { fail++; console.log('  FAIL ' + m); } }
const same = (a, b) => JSON.stringify(a) === JSON.stringify(b);

// golden = a function standing in for HT9045Page.golden() (ht9045_wire_engine.js:2175); omitted -> the page has none
function load(readImpl, wireFile, golden) {
  const listeners = [], reads = [];
  const sb = {
    console: { log() {}, info() {}, warn() {}, error() {} },
    document: { readyState: 'loading', addEventListener(t, fn, cap) { listeners.push({ t, fn, cap: !!cap }); }, getElementById() { return null; } },
    Promise
  };
  sb.window = sb;
  sb.Event = function (type, init) { this.type = type; this.bubbles = !!(init && init.bubbles); };
  let cfg = null;
  sb.HT9045Wire = { register(c) { cfg = c; sb.HT9045Page = { cfg: c }; if (golden) sb.HT9045Page.golden = golden; }, say() {} };
  sb.HT9045System = { read(name) { reads.push(name); return readImpl(name); } };
  vm.createContext(sb);
  vm.runInContext(fs.readFileSync(path.join(pageDir, wireFile), 'utf8'), sb, { filename: wireFile });
  const before = cfg && cfg.kb ? JSON.parse(JSON.stringify(cfg.kb)) : null;
  vm.runInContext(fs.readFileSync(kbFile, 'utf8'), sb, { filename: path.basename(kbFile) });
  return { sb, cfg, before, listeners, reads };
}
function fire(v, id) {
  const shows = [];
  v.sb.HTQwerty = { N: { INTEGER: 1, DOUBLE: 2 }, show(el, flags, o) { shows.push({ id: el.id, flags, o }); } };
  const md = v.listeners.filter(l => l.t === 'mousedown' && l.cap);
  const fired = [];
  const el = { id, disabled: false, dispatchEvent(e) { fired.push(e.type); } };
  const ev = { target: el, pd: false, sp: false, preventDefault() { this.pd = true; }, stopPropagation() { this.sp = true; } };
  md.forEach(l => l.fn(ev));
  return { shows, ev, fired, n: md.length };
}
const settle = () => new Promise(r => setTimeout(r, 0));
const ini = (sec, key, v) => () => Promise.resolve({ sections: { [sec]: { [key]: { raw: String(v), value: String(v) } } } });
const opened = (r, flags, dp, min, max) => r.n === 1 && r.shows.length === 1 && r.shows[0].flags === flags && r.shows[0].o.dp === dp &&
  r.shows[0].o.checkRange === true && r.shows[0].o.min === min && r.shows[0].o.max === max && r.ev.pd && r.ev.sp;
// what the engine's HT9045Page.golden() returns after editlist.get UserDefForm_File (FileRW/_EditList.cpp:58 captions)
const ON = 'Catch Tray Range:60-73', OFF = 'Tray Range:30-110';         // golden cTrayForm.cpp:258-260 / :264-266
const tfAnswer = (caps, struct) => ({ struct: struct || 'UserDefForm_File', page: { proxies: {
  Lab1XPickup: { visible: true, caption: caps[0] }, Lab2XPickup: { visible: true, caption: caps[1] }, Lab3XPickup: { visible: true, caption: caps[2] },
  Tp1TickUp: { visible: true, text: '65.000' } } }, kinds: {}, lastSave: null });
const fileSays = (v) => ini('Tray', 'bC03UseCatchTray', v);            // the raw LastSet.ini -- must not be read
const TF_IDS = ['Tp1TickUp', 'Tp2TickUp', 'Tp3TickUp'];

(async () => {
  const CV = 'ht9045_wire_maincommview.js', TF = 'ht9045_wire_trayform.js';
  console.log('-- 1. CommView, golden main.cpp:26183: not Mitsubishi -> INTEGER 0..300');
  for (const t of [2, 0]) {
    const v = load(ini('IndexDriver', 'INDEX_DRIVER_TYPE', t), CV);
    await settle();
    check(same(v.reads, ['gerneral']) && v.sb.HT9045KbGeneric.driver().known && v.sb.HT9045KbGeneric.driver().type === t,
          'INDEX_DRIVER_TYPE ' + t + ' read from gerneral [IndexDriver]');
    for (const id of ['edtSetZ1', 'edtSetZ2']) {
      const r = fire(v, id);
      check(opened(r, 1, 0, 0, 300), id + ' with driver ' + t + ': INTEGER 0..300, the engine handler stopped');
      if (r.shows[0] && r.shows[0].o.onCommit) r.shows[0].o.onCommit();
      check(same(r.fired, ['input', 'change']), id + ': OK fires input + change');
    }
    const o = fire(v, 'edtReadZ1');
    check(o.shows.length === 0 && !o.ev.pd, 'other fields untouched');
    check(same(v.cfg.kb, v.before), 'the kb table itself is unchanged');
  }
  console.log('-- 1b. CommView, golden rs232.cpp:103 default Panasonic_DRIVER (common.cpp:1444-1455) -> INTEGER 0..300');
  const cvDefault = [['no [IndexDriver] section', () => Promise.resolve({ sections: {} })],
                     ['[IndexDriver] without the key', () => Promise.resolve({ sections: { IndexDriver: { USE_HP_COM_CARD: { raw: '0' } } } })],
                     ['an empty value', ini('IndexDriver', 'INDEX_DRIVER_TYPE', '')],
                     ['a value that is not an integer (ReadInteger -> the default)', ini('IndexDriver', 'INDEX_DRIVER_TYPE', 'abc')],
                     ['"1 ;x" (StrToIntDef fails -> the default)', ini('IndexDriver', 'INDEX_DRIVER_TYPE', '1 ;x')],
                     ['an unknown driver id 3 (not Mitsubishi)', ini('IndexDriver', 'INDEX_DRIVER_TYPE', 3)]];
  for (const [what, impl] of cvDefault) {
    const v = load(impl, CV);
    await settle();
    check(v.sb.HT9045KbGeneric.driver().known && opened(fire(v, 'edtSetZ1'), 1, 0, 0, 300) && opened(fire(v, 'edtSetZ2'), 1, 0, 0, 300),
          'CommView ' + what + ': INTEGER 0..300');
  }
  console.log('-- 2. CommView, golden main.cpp:26179 / the safe side: INTEGER 0..100');
  const cvCases = [['Mitsubishi (1)', ini('IndexDriver', 'INDEX_DRIVER_TYPE', 1)],
                   ['Mitsubishi " 1 " (bcb "1", what BCB6 reads)', () => Promise.resolve({ sections: { IndexDriver: { INDEX_DRIVER_TYPE: { raw: ' 1 ', value: 1, bcb: '1' } } } })],
                   ['Mitsubishi as hex "0x1" (ReadInteger)', ini('IndexDriver', 'INDEX_DRIVER_TYPE', '0x1')],
                   ['Mitsubishi as hex "$01"', ini('IndexDriver', 'INDEX_DRIVER_TYPE', '$01')],
                   ['Mitsubishi with other letter case [indexdriver] index_driver_type', ini('indexdriver', 'index_driver_type', 1)],
                   ['read rejected', () => Promise.reject(new Error('http 500'))],
                   ['file not available', () => Promise.resolve({ available: false })],
                   ['no answer body', () => Promise.resolve(null)]];
  for (const [what, impl] of cvCases) {
    const v = load(impl, CV);
    await settle();
    check(opened(fire(v, 'edtSetZ1'), 1, 0, 0, 100), 'CommView ' + what + ': INTEGER 0..100');
  }
  for (const [later, max] of [[{ sections: { IndexDriver: { INDEX_DRIVER_TYPE: { raw: '2' } } } }, 300], [{ sections: {} }, 300],
                              [{ sections: { IndexDriver: { INDEX_DRIVER_TYPE: { raw: '1' } } } }, 100]]) {
    let resolve;
    const v = load(() => new Promise(r => { resolve = r; }), CV);
    await settle();                                   // the read starts one microtask after load (Promise.resolve().then)
    check(typeof resolve === 'function' && !v.sb.HT9045KbGeneric.driver().known, 'CommView: the read has started and is still pending');
    check(opened(fire(v, 'edtSetZ2'), 1, 0, 0, 100), 'CommView not answered yet: 0..100 (the safe side)');
    resolve(later);
    await settle(); await settle();
    check(v.sb.HT9045KbGeneric.driver().known && opened(fire(v, 'edtSetZ2'), 1, 0, 0, max),
          'CommView answered later (' + JSON.stringify(later.sections) + '): the next opening gets 0..' + max);
  }
  // Sections 3 / 4: golden cConfiguration.cpp:1054-1066 is decided in C++ (pinned in section 5); each case is named after
  // the machine it stands for, and the stub answers the captions C++ sends for it.  The raw LastSet.ini is never read.
  console.log('-- 3. TrayForm, golden cTrayForm.cpp:800: C++ IniConfig.bC03UseCatchTray off -> DOUBLE 3, 30..110');
  for (const [what, file] of [['an auto-retest 9046AU (cConfiguration.cpp:1058 fixed 0), the file says 1', '1'],
                              ['no auto retest, the file says 0 (cConfiguration.cpp:1066)', '0'],
                              ['no auto retest, no key, the lastdata.dat default off (cConfiguration.cpp:1066)', null]]) {
    const v = load(file === null ? () => Promise.resolve({ sections: {} }) : fileSays(file), TF, () => tfAnswer([OFF, OFF, OFF]));
    await settle();
    check(v.reads.length === 0 && v.sb.HT9045KbGeneric.setting().known && v.sb.HT9045KbGeneric.setting().value === false,
          what + ': C route captions "' + OFF + '" -> off; LastSet.ini not read');
    for (const id of TF_IDS) {
      const r = fire(v, id);
      check(opened(r, 2, 3, 30, 110), id + ' with the catch tray off: DOUBLE 3, 30..110, the engine handler stopped');
      if (r.shows[0] && r.shows[0].o.onCommit) r.shows[0].o.onCommit();
      check(same(r.fired, ['input', 'change']), id + ': OK fires input + change');
    }
    const o = fire(v, 'Tp1TickDown');
    check(o.shows.length === 0 && !o.ev.pd, 'TrayForm: other fields untouched');
    check(same(v.cfg.kb, v.before), 'TrayForm: the kb table itself is unchanged');
  }
  console.log('-- 4. TrayForm, golden cTrayForm.cpp:798 / the safe side: DOUBLE 3, 60..73');
  const tfCases = [
    ['an auto-retest machine (cConfiguration.cpp:1062 fixed 1), the file says 0', () => tfAnswer([ON, ON, ON]), fileSays(0)],
    ['an auto-retest HT9045 with SubModel 3 (database.cpp:527-529 SubMachineType None -> :1062 fixed 1), the file says 0',
     () => tfAnswer([ON, ON, ON]), fileSays(0)],
    ['no auto retest, the file says 1 (cConfiguration.cpp:1066)', () => tfAnswer([ON, ON, ON]), fileSays(1)],
    ['no auto retest, no key, the lastdata.dat default on (cConfiguration.cpp:1066)', () => tfAnswer([ON, ON, ON]), () => Promise.resolve({ sections: {} })],
    ['the C route has not answered yet (golden() page null)', () => ({ struct: 'UserDefForm_File', page: null, kinds: {}, lastSave: null }), fileSays(0)],
    ['the page has no HT9045Page.golden()', null, fileSays(0)],
    ['HT9045Page.golden() throws', () => { throw new Error('boom'); }, fileSays(0)],
    ['another C route struct answered', () => tfAnswer([OFF, OFF, OFF], 'IniConfig'), fileSays(0)],
    ['the three captions disagree', () => tfAnswer([OFF, ON, OFF]), fileSays(0)],
    ['one caption missing', () => tfAnswer([OFF, OFF, undefined]), fileSays(0)],
    ['the DFM caption (FormShow not applied)', () => tfAnswer(['Range:', 'Range:', 'Range:']), fileSays(0)],
    ['no proxies in the answer', () => ({ struct: 'UserDefForm_File', page: {}, kinds: {}, lastSave: null }), fileSays(0)]];
  for (const [what, golden, impl] of tfCases) {
    const v = load(impl, TF, golden || undefined);
    await settle();
    check(v.reads.length === 0, 'TrayForm ' + what + ': LastSet.ini not read');
    for (const id of TF_IDS)                          // the three fields share one rule (one golden handler)
      check(opened(fire(v, id), 2, 3, 60, 73), 'TrayForm ' + what + ': ' + id + ' DOUBLE 3, 60..73');
  }
  {                                                   // Q41 TF-2 / TF-3 (ht9045_trayform_q41.js) keep working
    const v = load(fileSays(0), TF, () => tfAnswer([OFF, OFF, OFF]));
    await settle();
    for (const id of ['XPitch1', 'YPitch1', 'XCT1', 'YCT1', 'cbCopyFrom', 'spbCopy']) {
      const r = fire(v, id);
      check(r.shows.length === 0 && !r.ev.pd && !r.ev.sp, 'TrayForm ' + id + ' (Q41 TF-2 / TF-3): not touched');
    }
    const kbSrc = fs.readFileSync(kbFile, 'utf8');
    check(kbSrc.indexOf('stopImmediatePropagation') < 0,
          'ht9045_kb_generic.js never calls stopImmediatePropagation (q41.js\'s own capture listener on document still runs)');
    const q41 = fs.readFileSync(path.join(pageDir, 'ht9045_trayform_q41.js'), 'utf8');
    check(q41.indexOf("document.addEventListener('mousedown', pitchIntercept, true)") >= 0, 'q41.js still intercepts XPitch1 / YPitch1 on document (capture)');
  }
  {                                                   // the value is taken at each opening: not answered, then off, then on
    let ans = { struct: 'UserDefForm_File', page: null, kinds: {}, lastSave: null };
    const v = load(fileSays(1), TF, () => ans);
    await settle();
    check(!v.sb.HT9045KbGeneric.setting().known && opened(fire(v, 'Tp3TickUp'), 2, 3, 60, 73), 'TrayForm not answered yet: 60..73 (the safe side)');
    ans = tfAnswer([OFF, OFF, OFF]);
    await settle();
    check(opened(fire(v, 'Tp3TickUp'), 2, 3, 30, 110), 'TrayForm answered later (off): the next opening gets 30..110');
    ans = tfAnswer([ON, ON, ON]);                     // e.g. the engine's re-read after a save
    await settle();
    check(opened(fire(v, 'Tp3TickUp'), 2, 3, 60, 73), 'TrayForm read again (on): the next opening gets 60..73');
    check(v.reads.length === 0, 'TrayForm: still no file read');
  }
  console.log('-- 5. the C++ sources that carry IniConfig.bC03UseCatchTray to this page (pins)');
  {
    const rd = (p) => { try { return fs.readFileSync(path.join(cppDir, p), 'utf8'); } catch (e) { return ''; } };
    const add = (fixed, def) => 'cbLastSet->Add\\(EL<TCheckBox>\\("TfConfiguration", "cbC03"\\),\\s*&IniConfig\\.bC03UseCatchTray,\\s*ECBool,\\s*' +
      '"Tray",\\s*"bC03UseCatchTray",\\s*bShow,\\s*' + (fixed ? 'bDisable,\\s*bFixedValue,\\s*' : 'bEnable,\\s*bReadFromFile,\\s*') + def + '\\);';
    const ic = rd('FileRW/IniConfig.gen.inc');
    check(new RegExp('if\\(USE_AUTO_RETEST==eartInstall\\)\\s*\\{\\s*if\\(\\(SubMachineType==Type_HT9046AU\\)\\)[^\\n]*\\n\\s*\\{\\s*' + add(true, '0') +
                     '\\s*\\}\\s*else\\s*\\{\\s*' + add(true, '1') + '\\s*\\}\\s*\\}\\s*else\\s*' + add(false, 'LastSet\\.bC03UseCatchTray\\?"1":"0"')).test(ic),
          'FileRW/IniConfig.gen.inc: golden cConfiguration.cpp:1054-1066 (auto retest: fixed, 9046AU 0 / else 1; otherwise the file, LastSet default)');
    const db = rd('database.cpp');
    check(/if\(MachineTypeChoice==Type_HT9045 \|\|\s*MachineTypeChoice==Type_HT502\)[^\n]*\n\s*SubMachineType=Type_None;\s*else\s*SubMachineType=CheckAndReadIniDataGeneral\("Version", "SubModel", Type_None\);/.test(db) &&
          /USE_AUTO_RETEST\s*=CheckAndReadIniDataGeneral\("System", "USE_AUTO_RETEST",\s*\(int\)eartUninstall\);/.test(db),
          'database.cpp: SubMachineType (None on HT9045 / HT502, else [Version] SubModel) and [System] USE_AUTO_RETEST (default eartUninstall)');
    const boot = rd('FileRW/IniConfig.cpp'), b0 = boot.indexOf('void FileRW_IniConfig_Boot()');
    const bI = boot.indexOf('IC_InitConfigEdtList();', b0), bR = boot.indexOf('ReadLastSetIni();', b0);
    check(b0 >= 0 && bI > b0 && bR > bI, 'FileRW/IniConfig.cpp: FileRW_IniConfig_Boot registers the edit list, then ReadLastSetIni reads LastSet.ini');
    const ws = rd('tools/wb_serve.cpp'), wL = ws.indexOf('if (!LoadMachineConfig())'), wB = ws.indexOf('FileRW_IniConfig_Boot(); }');
    check(wL >= 0 && wB > wL, 'tools/wb_serve.cpp: FileRW_IniConfig_Boot runs at boot, after LoadMachineConfig (USE_AUTO_RETEST, SubMachineType, lastdata.dat)');
    const uf = rd('FileRW/UserDefForm_File.gen.inc'), f0 = uf.search(/static void TF_FormShow\(\)\s*\{/), f1 = uf.search(/static void TF_DoIniDataToForm\(\)\s*\{/);   // the bodies, not the forward declarations
    const lab = (n, t) => '\\s*EL<TLabel>\\("TfTrayForm", "Lab' + n + 'XPickup"\\)->Caption="' + t + '";';
    const m = new RegExp('if\\(IniConfig\\.bC03UseCatchTray\\)[^\\n]*\\n\\s*\\{' + lab(1, ON) + lab(2, ON) + lab(3, ON) + '\\s*\\}\\s*else\\s*\\{' +
                         lab(1, OFF) + lab(2, OFF) + lab(3, OFF) + '\\s*\\}').exec(uf);
    check(!!m && f0 >= 0 && m.index > f0 && (f1 < 0 || m.index < f1),
          'FileRW/UserDefForm_File.gen.inc TF_FormShow: golden cTrayForm.cpp:256-267 sets Lab1/2/3XPickup from IniConfig.bC03UseCatchTray');
    const ud = rd('FileRW/UserDefForm_File.cpp');
    check(/"UserDefForm_File", "TfTrayForm", "Setup\.TrayForm\.html"/.test(ud) && ud.indexOf('&TF_FormShow') >= 0,
          'FileRW/UserDefForm_File.cpp: editlist.get UserDefForm_File runs TF_FormShow for Setup.TrayForm.html');
    const ep = rd('FileRW/_EditPage.cpp'), eS = ep.indexOf('d.formShow();'), eP = ep.indexOf('w.Key("proxies").RawValue(ProxyStateJson(d.form));');
    check(eS >= 0 && eP > eS, 'FileRW/_EditPage.cpp: the proxies are sent after golden FormShow ran');
    check(rd('FileRW/_EditList.cpp').indexOf('if (TLabel* x = dynamic_cast<TLabel*>(c)) { if (!x->Caption.IsEmpty()) w.Key("caption").String(x->Caption.c_str()); return; }') >= 0,
          'FileRW/_EditList.cpp: a TLabel proxy sends its caption');
    const en = fs.readFileSync(path.join(pageDir, 'ht9045_wire_engine.js'), 'utf8');
    check(en.indexOf("'Setup.TrayForm.html': 'UserDefForm_File'") >= 0 && en.indexOf('golden: function () { return { struct: gbStruct(), page: GB') >= 0,
          'ht9045_wire_engine.js: Setup.TrayForm is C route UserDefForm_File and HT9045Page.golden() returns the last editlist.get answer');
  }
  console.log('-- 6. other pages, load order');
  {
    const w = load(ini('Tray', 'bC03UseCatchTray', 0), 'ht9045_wire_setupsetup.js', () => tfAnswer([OFF, OFF, OFF]));
    await settle();
    check(w.listeners.filter(l => l.t === 'mousedown').length === 0 && w.reads.length === 0, 'another page (Setup.SetUp): no listener, no read');
    const c = load(ini('IndexDriver', 'INDEX_DRIVER_TYPE', 2), CV, () => tfAnswer([OFF, OFF, OFF]));
    await settle();
    check(fire(c, 'Tp1TickUp').shows.length === 0 && same(c.reads, ['gerneral']), 'CommView: a TrayForm id is not handled there; only gerneral is read');
  }
  for (const [html, wire] of [['Main.CommView.html', CV], ['Setup.TrayForm.html', TF]]) {
    const t = fs.readFileSync(path.join(pageDir, html), 'utf8');
    const iE = t.indexOf('src="ht9045_wire_engine.js"'), iW = t.indexOf('src="' + wire + '"'), iK = t.indexOf('src="ht9045_kb_generic.js"');
    check(iE >= 0 && iW > iE && iK > iW, html + ': engine, the wire data, then ht9045_kb_generic.js');
  }
  {
    const t = fs.readFileSync(path.join(pageDir, 'Setup.TrayForm.html'), 'utf8');
    const iW = t.indexOf('src="' + TF + '"'), iQ = t.indexOf('src="ht9045_trayform_q41.js"'), iK = t.indexOf('src="ht9045_kb_generic.js"');
    check(iW >= 0 && iQ > iW && iK > iQ, 'Setup.TrayForm.html: the wire data, then ht9045_trayform_q41.js, then ht9045_kb_generic.js');
  }
  console.log((fail ? 'FAIL' : 'PASS') + ': ' + pass + '/' + (pass + fail) + ' checks');
  process.exit(fail ? 1 : 0);
})();
