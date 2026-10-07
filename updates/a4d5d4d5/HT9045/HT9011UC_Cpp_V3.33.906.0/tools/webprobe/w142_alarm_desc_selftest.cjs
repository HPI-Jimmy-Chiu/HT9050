// w142_alarm_desc_selftest.cjs -- ctest St02_W142AlarmDescPage (node, offline): what Alert.Note.html shows in the message line
// (ShowMessageEdit1) and the description box (reDescription) after web/page/ht9045_alarm_motionview.js handles an alarm request.
// AI(W906-W142) 20261007 (St02-E): laptop card W-142 (EastSun 1007「之後畫面不要只顯示錯誤碼 要顯示錯誤碼對應的說明」).
//   description: C++'s golden .dat text (display.description) -> web/JSON/Alarm-description.json by display.descriptionKey
//                ("MOT<k>", a motor note) or by code -> the message line (W-142 item 4, laptop default)
//   message:     C++'s golden text stays; the bare code (+ " : "+errPart) C++ sends for a code missing from the live
//                AlarmCodeList.txt gets the index text in front of it
// The script runs as on the page: dialog-page.js has already put display.message / display.description into the two boxes
// (renderAlarm, dialog-page.js:63-64), then the HT_DIALOG_REQUEST message reaches the motionview script.
// Usage: node w142_alarm_desc_selftest.cjs <web/page dir>.  Control: W906_ALARM_MV_JS=<the pre-W-142 file> must make it red.
// Fake DOM rule (St02 workflow skill section 5 item 20): every property the script reads is set when the DOM is built.
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE = process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
const SRC = process.env.W906_ALARM_MV_JS || path.join(PAGE, 'ht9045_alarm_motionview.js');
const code = fs.readFileSync(SRC, 'utf8');

let pass = 0, fail = 0;
function check(cond, what, got) { if (cond) { pass++; console.log('  ok   ' + what + '   [' + got + ']'); } else { fail++; console.log('  FAIL ' + what + '   [got ' + got + ']'); } }
async function settle() { for (let i = 0; i < 20; i++) await new Promise((r) => setImmediate(r)); }

const JSONS = {
  'Alarm-unit-map': { units: {}, panels: {}, defaultPanel: 'palSys', codeUnits: { '01': { name: 'InArm' }, '24': { name: 'Motor' } } },
  'Alarm-description': { text: { WAR0102: { English: 'static WAR0102' }, MOT3: { English: 'static MOT3' } } },
  'AlarmCodeList-index': { codes: { MES16441: 'Index text MES16441', WAR0102: 'Index WAR0102', WAR0109: 'Index WAR0109' } },
};

function mkEl(id) {
  return { id, value: '', textContent: '', tagName: 'TEXTAREA', style: {}, classList: { add() {}, remove() {} },
           appendChild(c) { return c; }, remove() {}, setAttribute() {}, getAttribute() { return null; } };
}
const els = { reDescription: mkEl('reDescription'), reBigDescription: mkEl('reBigDescription'),
              ShowMessageEdit1: Object.assign(mkEl('ShowMessageEdit1'), { tagName: 'INPUT' }), edUnitName: Object.assign(mkEl('edUnitName'), { tagName: 'INPUT' }) };
const onMessage = [];   // the script registers more than one 'message' listener (its HT_LANG refill too)
const sb = {
  console: { info() {}, warn() {}, error() {}, log() {} },
  setTimeout, clearTimeout, Promise, JSON, Date, Math, Object, Array, String, Number, Error, RegExp,
  fetch(url) {
    const name = String(url).replace(/^.*\//, '').replace(/\.json$/, '');
    if (!(name in JSONS)) return Promise.resolve({ ok: false, json() { return Promise.resolve(null); } });
    return Promise.resolve({ ok: true, json() { return Promise.resolve(JSON.parse(JSON.stringify(JSONS[name]))); } });
  },
  location: { pathname: '/web/page/Alert.Note.html', search: '' },
  localStorage: { getItem() { return null; }, setItem() {}, removeItem() {} },
  document: {
    getElementById(id) { return els[id] || null; },
    querySelectorAll() { return []; },
    createElement() { return mkEl(''); },
    head: { appendChild(c) { return c; } },
    body: { classList: { add() {}, remove() {} } },
  },
  addEventListener(t, f) { if (t === 'message') onMessage.push(f); },
};
sb.window = sb;
sb.parent = {};
sb.global = sb;
vm.createContext(sb);
vm.runInContext(code, sb, { filename: SRC });

let seq = 1;
async function alarm(c, display) {
  // dialog-page.js renderAlarm first (:63-64)
  els.ShowMessageEdit1.value = display.message || '';
  els.reDescription.value = display.description || '';
  els.reBigDescription.value = '';
  const ev = { data: { type: 'HT_DIALOG_REQUEST', kind: 'alarm', request: { requestId: 'q' + seq, seq: seq++, arguments: { code: c, position: 0, kCode: 4 }, display } } };
  onMessage.slice().forEach((f) => f(ev));
  await settle();
  return { msg: els.ShowMessageEdit1.value, desc: els.reDescription.value, big: els.reBigDescription.value };
}

(async () => {
  console.log('St02_W142AlarmDescPage (' + path.basename(SRC) + ')');
  if (!onMessage.length) { console.log('  FAIL the script did not register its message handler'); process.exit(1); }
  let r = await alarm('WAR0102', { message: 'Golden WAR0102 message', description: 'LIVE .dat text', unitName: 'InArm' });
  check(r.desc === 'LIVE .dat text', '1. C++ sent the golden .dat text -> it stays (the static JSON must not overwrite it)', r.desc);
  r = await alarm('WAR0102', { message: 'Golden WAR0102 message', description: '', unitName: 'InArm' });
  check(r.desc === 'static WAR0102' && r.big === 'static WAR0102', '2. no live text -> the static Alarm-description.json row by code', r.desc);
  r = await alarm('WAR24013', { message: 'Golden motor text', description: '', descriptionKey: 'MOT3', unitName: 'Motor' });
  check(r.desc === 'static MOT3', '3. a motor note -> the static row by descriptionKey MOT3 (golden MOT<k>.dat)', r.desc);
  r = await alarm('WAR0109', { message: 'Golden WAR0109 message : part', description: '', unitName: 'InArm' });
  check(r.desc === 'Golden WAR0109 message : part', '4. no description anywhere -> the message line (W-142 item 4)', r.desc);
  check(r.msg === 'Golden WAR0109 message : part', '4. the golden message line stays even when the index text differs', r.msg);
  r = await alarm('MES16441', { message: 'MES16441 : extra', description: '', unitName: '' });
  check(r.msg === 'Index text MES16441 : extra', '5. C++ could send only the code (+ errPart) -> the index text in front of it', r.msg);
  check(r.desc === 'Index text MES16441 : extra', '5. ... and the description fallback reads that final message line', r.desc);
  console.log('St02_W142AlarmDescPage: ' + pass + ' passed, ' + fail + ' failed');
  process.exit(fail ? 1 : 0);
})().catch((e) => { console.log('  FAIL exception ' + (e && e.stack || e)); process.exit(1); });
