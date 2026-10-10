'use strict';
// AI(W906-HTDESIGNER) 20261010 (review of 0.427 live #1): the designer's preview answers /api/system/<csv> as wb_serve does.
// csvdoc.js -- wb_serve's GET /api/system/<csv name> body, reproduced in plain Node (READ ONLY; never writes).
//
// Source of truth (tools/wb_serve.cpp, read 20261010):
//   CsvToJson            :1407-1455   lines split on '\n' only; last piece kept only if non-empty
//   SplitCsv             :1356-1375   split on ',' (no quoting), every '\r' AND '\n' byte dropped
//   CsvKeyColumn         :1391-1396   ioTable -> "Alias", motTable -> "Motorname", other csv -> column 0
//   CsvKeyIndex          :1400-1405   exact BYTE compare of the header cell (a UTF-8 BOM breaks the match)
//   row skip             :1442-1446   empty line; or key cell missing/empty (or keyColumn not found -> ALL rows skipped)
//   short row            :1449        missing cells -> ""; extra cells beyond the header dropped
//   empty file           :1422-1426   {path, kind, columns:[], rows:[]}  (no keyColumn key)
//   unreadable file      :1409/2192   "" -> HTTP 404 text/plain "404 file not readable: <path>\n"   (csvToDoc returns null)
//   > 2 MiB JSON         :978/2198    HTTP 413 (csvToDoc reports it in a non-enumerable property, see below)
// Strings (path, column names, cell values) each go through JsonQuote -> SanitizeToUtf8 (WebBridge/JsonWriter.cpp:128-146)
// PER STRING:  (a) valid UTF-8 (overlong / surrogate / >U+10FFFF rejected) -> as is, a BOM is kept as U+FEFF;
//              (b) else kernel32 MultiByteToWideChar(950, MB_ERR_INVALID_CHARS) on the whole string;
//              (c) else per-byte repair: keep each well-formed UTF-8 sequence, every other byte -> U+FFFD.
// (b) here = Node's big5 decoder in fatal mode, which was measured equal to kernel32 cp950 on all 19,910
//     single/double-byte units (scratchpad gen_cp950.ps1 + diff_cp950.js) except one: byte 0xFF (Node U+F8F8,
//     kernel32 invalid) -- handled below.
// Duplicate header names: wb_serve writes the key twice in one object; JSON.parse keeps the LAST -- so do we.
const fs = require('fs');
const path = require('path');

const UTF8 = new TextDecoder('utf-8', { fatal: true, ignoreBOM: true });   // ignoreBOM: wb_serve keeps the BOM bytes
const BIG5 = new TextDecoder('big5', { fatal: true });

// JsonWriter.cpp:26-59 Utf8SequenceLen
function seqLen(b, i) {
  const n = b.length, c0 = b[i];
  if (c0 < 0x80) return 1;
  if (c0 < 0xC2) return 0;
  let need, cp;
  if (c0 < 0xE0) { need = 1; cp = c0 & 0x1F; } else if (c0 < 0xF0) { need = 2; cp = c0 & 0x0F; } else if (c0 < 0xF5) { need = 3; cp = c0 & 0x07; } else return 0;
  if (i + need >= n) return 0;
  for (let k = 1; k <= need; k++) { const cc = b[i + k]; if ((cc & 0xC0) !== 0x80) return 0; cp = (cp << 6) | (cc & 0x3F); }
  if ((need === 1 && cp < 0x80) || (need === 2 && cp < 0x800) || (need === 3 && cp < 0x10000)) return 0;
  if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return 0;
  return need + 1;
}

/** JsonWriter.cpp:128 SanitizeToUtf8, for one string's bytes (Buffer) -> JS string. Also tells which rule fired. */
function sanitize(b, stats) {
  if (b.length === 0) return '';
  try { const s = UTF8.decode(b); if (stats) stats.utf8++; return s; } catch (e) { /* not UTF-8 */ }
  if (!b.includes(0xFF)) {                       // 0xFF: Node big5 -> U+F8F8, kernel32 cp950 -> invalid
    try { const s = BIG5.decode(b); if (stats) stats.cp950++; return s; } catch (e) { /* not cp950 */ }
  }
  if (stats) stats.repaired++;
  let out = '';
  for (let i = 0; i < b.length;) {
    const len = seqLen(b, i);
    if (len === 0) { out += '\uFFFD'; i++; } else { out += b.toString('utf8', i, i + len); i += len; }
  }
  return out;
}

// wb_serve.cpp:1356 SplitCsv on raw bytes (',' never occurs as a Big5 trail byte, trail >= 0x40)
function splitCsv(line) {
  const out = []; let cur = [];
  for (const c of line) {
    if (c === 0x0D || c === 0x0A) continue;
    if (c === 0x2C) { out.push(Buffer.from(cur)); cur = []; } else cur.push(c);
  }
  out.push(Buffer.from(cur));
  return out;
}

const KEY_COLUMN = { ioTable: 'Alias', motTable: 'Motorname' };   // wb_serve.cpp:1391-1396 (other csv names: column 0)

function setLast(obj, k, v) { Object.defineProperty(obj, k, { value: v, enumerable: true, writable: true, configurable: true }); }

/**
 * csvToDoc(file, name?) -> the object JSON.parse would give for wb_serve's body, or null where wb_serve answers 404.
 * name: the /api/system name (ioTable / motTable / trayForm / plateForm); if omitted it is guessed from the file name.
 * Non-enumerable extras on the result: _jsonBytes (size of wb_serve's body, approx.), _tooLarge (wb_serve -> 413),
 * _decode {utf8, cp950, repaired} per-string counts.
 */
function csvToDoc(file, name) {
  if (!name) {
    const b = path.basename(file).toLowerCase();
    name = b === 'io_table.csv' ? 'ioTable' : b === 'mot_table.csv' ? 'motTable' : b === 'trayform.csv' ? 'trayForm' : b === 'plateform.csv' ? 'plateForm' : '';
  }
  let text;
  try { text = fs.readFileSync(file); } catch (e) { return null; }
  const stats = { utf8: 0, cp950: 0, repaired: 0 };
  const lines = [];
  let start = 0;
  for (let i = 0; i < text.length; i++) if (text[i] === 0x0A) { lines.push(text.subarray(start, i)); start = i + 1; }
  if (start < text.length) lines.push(text.subarray(start));

  // wb_serve's path is SysFilePath() of the golden global (an ASCII path); here the file asked for.
  const doc = { path: file, kind: 'csv' };
  if (lines.length === 0) { doc.columns = []; doc.rows = []; return finish(doc, stats); }

  const colBytes = splitCsv(lines[0]);
  const cols = colBytes.map(b => sanitize(b, stats));
  doc.columns = cols;
  const keyCol = KEY_COLUMN[name] || null;
  let ki;
  if (!keyCol) ki = colBytes.length ? 0 : -1;
  else { ki = -1; const kb = Buffer.from(keyCol, 'latin1'); for (let c = 0; c < colBytes.length; c++) if (colBytes[c].equals(kb)) { ki = c; break; } }
  doc.keyColumn = ki < 0 ? '' : cols[ki];
  doc.rows = [];
  for (let li = 1; li < lines.length; li++) {
    if (lines[li].length === 0) continue;
    const f = splitCsv(lines[li]);
    if (ki < 0 || ki >= f.length || f[ki].length === 0) continue;
    const row = {};
    for (let c = 0; c < cols.length; c++) setLast(row, cols[c], c < f.length ? sanitize(f[c], stats) : '');
    doc.rows.push(row);
  }
  return finish(doc, stats);
}

function finish(doc, stats) {
  const bytes = Buffer.byteLength(JSON.stringify(doc), 'utf8');   // wb_serve escapes the same set except it never \u-escapes >= 0x80
  Object.defineProperty(doc, '_jsonBytes', { value: bytes });
  Object.defineProperty(doc, '_tooLarge', { value: bytes > 2 * 1024 * 1024 });   // wb_serve.cpp:978 kMaxDocBytes -> 413
  Object.defineProperty(doc, '_decode', { value: stats });
  return doc;
}

/**
 * nameMap(machineRoot = 'D:\\HT9045', env = {}) -> { name: { file, kind, keyColumn?, env?, note? } }
 * Every name SystemRoute answers (wb_serve.cpp:1172-1215 SysFileTable + :1908-1910 SysBinTable), path globals from
 * common.cpp (decl :155-334 and InitCommonString :393-484, which wb_serve runs at startup, wb_serve.cpp:3871).
 * machineRoot replaces the golden "D:\HT9045" prefix (e.g. a copy at D:\HP9050\HT9045). env: the W906_* seams wb_serve
 * honours (W906_GENERAL_INI_PATH, W906_TEACH_INI_PATH, W906_SETUPINF_PATH, W906_AUTH_PATH (used verbatim, then the
 * file name is APPENDED -- needs a trailing '\'), W906_IOTABLE_PATH, W906_MOTTABLE_PATH (database.cpp:1720/:1797, applied
 * by LoadMachineConfig at boot)). W906_INIDATA_ROOT makes wb_serve refuse the web API (wb_serve.cpp:3768-3770).
 * Name lookup in wb_serve is ASCII case-insensitive (StemFoldEq :922) and must match [A-Za-z0-9_.-]{1,64} (:933).
 */
function nameMap(machineRoot, env) {
  const R = String(machineRoot || 'D:\\HT9045').replace(/[\\/]+$/, '');
  env = env || {};
  const g = s => s.replace(/^[dD]:\\HT9045/, R);              // golden literal under the chosen root
  const ev = (k, lit) => (env[k] ? env[k] : g(lit));
  const SYS = g('d:\\HT9045\\system\\');
  const AUTH = env.W906_AUTH_PATH ? env.W906_AUTH_PATH : g('D:\\HT9045\\config\\');
  const INI = g('D:\\HT9045\\IniData\\');
  const ERR = g('D:\\HT9045\\Error\\');
  const PM = g('D:\\HT9045\\PMAlarm\\');
  const ini = (file, extra) => Object.assign({ file, kind: 'ini' }, extra || {});
  const csv = (file, extra) => Object.assign({ file, kind: 'csv', keyColumn: null }, extra || {});
  return {
    gerneral:      ini(ev('W906_GENERAL_INI_PATH', 'D:\\HT9045\\system\\Gerneral.ini'), { env: 'W906_GENERAL_INI_PATH', note: '--dry never redirects the web read (gRealGeneralPath)' }),
    teach:         ini(ev('W906_TEACH_INI_PATH', 'D:\\HT9045\\system\\teach.ini'), { env: 'W906_TEACH_INI_PATH' }),
    motTable:      csv(ev('W906_MOTTABLE_PATH', 'D:\\HT9045\\System\\Mot_Table.csv'), { env: 'W906_MOTTABLE_PATH', keyColumn: 'Motorname' }),
    ioTable:       csv(ev('W906_IOTABLE_PATH', 'D:\\HT9045\\System\\IO_Table.csv'), { env: 'W906_IOTABLE_PATH', keyColumn: 'Alias' }),
    config:        ini(AUTH + 'config.ini', { env: 'W906_AUTH_PATH' }),
    dio:           ini(null, { note: 'computed: config.ini [Tester] bI16TTLSaveInSetupFile ? <recipe>\\<TypeName>.ini : IniData\\DioCfg\\<TypeName>.ini, TypeName from <recipe>\\Tester.Data [DIO] (wb_serve.cpp:1810-1844)' }),
    lastSet:       ini(AUTH + 'LastSet.ini', { env: 'W906_AUTH_PATH' }),
    errNote:       ini(g('D:\\HT9045\\system\\SpecialErrNote.ini')),
    description:   ini(g('D:\\HT9045\\config\\Description.ini')),
    setupInf:      ini(ev('W906_SETUPINF_PATH', 'D:\\HT9045\\SetUp.inf'), { env: 'W906_SETUPINF_PATH' }),
    trayForm:      csv(g('D:\\HT9045\\System\\TrayForm.csv'), { note: 'W906_TRAYFORMCSV_PATH only moves FileRW/CfgTrayPlate.cpp, not this route' }),
    plateForm:     csv(g('D:\\HT9045\\System\\PlateForm.csv'), { note: 'W906_PLATEFORMCSV_PATH likewise not used here' }),
    trayStepSpeed: ini(g('D:\\HT9045\\system\\TrayStepSpeed.ini')),
    machineLife:   ini(g('D:\\HT9045\\system\\MachineLife.ini')),
    arms:          ini(g('D:\\HT9045\\system\\ARMS.ini')),
    secsGem:       ini(g('D:\\HT9045\\SECS\\SYSTEM\\Gerneral.ini'), { note: 'InitCommonString value (common.cpp:422); the declaration says SECS\\SECS\\SYSTEM' }),
    contactInfo:   ini(SYS + 'ContactInfo.ini'),
    autoTemp:      ini(SYS + 'AutoTemperature.ini'),
    atcSystem:     ini(SYS + 'ATC.ini'),
    barcode:       ini(SYS + 'Barcode.ini'),
    padInterface:  ini(SYS + 'PadInterfacePara.ini'),
    eventLogLevel: ini(SYS + 'EvenLogLevel.ini'),
    socketCount:   ini(INI + 'SocketCount.ini'),
    motorTest:     ini(SYS + 'MotorTest.ini'),
    colorSensor:   ini(SYS + 'ColorSensorType.ini'),
    mvData:        ini(SYS + 'MVData.ini'),
    rpDefault:     ini(INI + 'RPDefault.ini'),
    alarmDesc:     ini(ERR + 'AlarmDescription.ini'),
    alarmCodeList: ini(ERR + 'AlarmCodeList.txt', { note: 'a .txt served through the INI parser' }),
    securityNew:   ini(AUTH + 'Security_new.def', { env: 'W906_AUTH_PATH' }),
    criticalPara:  ini(AUTH + 'CriticalParaControl.ini', { env: 'W906_AUTH_PATH' }),
    esdConfig:     ini(AUTH + 'ESDconfig.ini', { env: 'W906_AUTH_PATH' }),
    atcConfig:     ini(AUTH + 'ATC.ini', { env: 'W906_AUTH_PATH' }),
    pmMonth:       ini(PM + 'PM_Month.ini'),
    pmQuarter:     ini(PM + 'PM_Quarter.ini'),
    pmYear:        ini(PM + 'PM_Year.ini'),
    pmTemperature: ini(PM + 'PM_Temperature.ini'),
    pmEsd:         ini(PM + 'PM_ESD.ini'),
    pmIonFan:      ini(PM + 'PM_IonFan.ini'),
    pmSetting:     ini(PM + 'PM_Setting.ini'),
    levelset:      { file: SYS + 'levelset.dat', kind: 'i32', note: '256 little-endian int32, size must be exactly 1024 bytes (wb_serve.cpp:1946-1975)' },
  };
}

module.exports = { csvToDoc, nameMap, sanitize, splitCsv };
