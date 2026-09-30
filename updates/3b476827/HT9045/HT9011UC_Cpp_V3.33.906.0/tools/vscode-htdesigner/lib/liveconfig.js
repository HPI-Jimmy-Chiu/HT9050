'use strict';
// AI(W906-HTDESIGNER) 20260930: 機種與機台設定（唯讀）-- so a page looks in the designer the way it does on the machine.
//
// Pages that differ by machine (HW.IoSetView's Above9050 tab, Main/Alert.MotionView9050) decide it from
//   1. the machine the HMI was opened for (background.html?machine=HT9050 -> every window's launch options), and
//   2. the machine's settings, GET /api/system/gerneral (and config) from wb_serve.
// The designer has no network, so both were missing and those pages showed their "no server" face
// (EastSun 20260930: "這IO頁面 怎感覺有缺? 跟我現在使用的有少東西").
// Both come from what F5 starts wb_serve with (.vscode/launch.json): W906_HMI_URL's machine=, and
// W906_GENERAL_INI_PATH / W906_AUTH_PATH (+ config.ini). The files are only READ, here, never written.
// iniToDoc() gives the same shape as wb_serve (WebBridgeRecipeDoc.cpp RecipeDocToJson):
//   { path, available, sections: { sec: { key: { value, type: int|float|string, raw, bcb } } } }
// Plain Node (no vscode).
const fs = require('fs');
const path = require('path');

/** JSON with // and /* comments and trailing commas (launch.json) -> JSON. */
function stripJsonc(t) {
  let out = '', i = 0, inStr = false;
  const s = String(t || '');
  while (i < s.length) {
    const c = s[i];
    if (inStr) {
      out += c;
      if (c === '\\') { out += s[i + 1] || ''; i += 2; continue; }
      if (c === '"') inStr = false;
      i++;
      continue;
    }
    if (c === '"') { inStr = true; out += c; i++; continue; }
    if (c === '/' && s[i + 1] === '/') { while (i < s.length && s[i] !== '\n') i++; continue; }
    if (c === '/' && s[i + 1] === '*') { const e = s.indexOf('*/', i + 2); i = e < 0 ? s.length : e + 2; continue; }
    out += c;
    i++;
  }
  return out.replace(/,(\s*[\]}])/g, '$1');
}

/**
 * What F5 starts wb_serve with, from `.vscode/launch.json` of each folder:
 *   { machine, machines: { id: count }, generalIni, configIni, from }   (null fields: not found)
 * The machine most configurations name wins; the first settings paths found are used.
 */
function launchHints(dirs) {
  const out = { machine: null, machines: {}, generalIni: null, configIni: null, ioTable: null, from: null };
  for (const dir of dirs || []) {
    if (!dir) continue;
    const f = path.join(dir, '.vscode', 'launch.json');
    let j;
    try { j = JSON.parse(stripJsonc(fs.readFileSync(f, 'utf8'))); } catch (e) { continue; }
    for (const c of (j && j.configurations) || []) {
      const env = {};
      for (const e of Array.isArray(c.environment) ? c.environment : []) if (e && e.name) env[e.name] = String(e.value || '');
      if (c.env && typeof c.env === 'object') for (const k of Object.keys(c.env)) env[k] = String(c.env[k] || '');
      const sub = v => path.normalize(String(v).replace(/\$\{workspaceFolder\}/g, dir));
      const m = /[?&]machine=([A-Za-z0-9_-]+)/.exec(env.W906_HMI_URL || '');
      if (m) { out.machines[m[1]] = (out.machines[m[1]] || 0) + 1; if (!out.from) out.from = f; }
      if (env.W906_GENERAL_INI_PATH && !out.generalIni) { out.generalIni = sub(env.W906_GENERAL_INI_PATH); out.from = out.from || f; }
      if (env.W906_AUTH_PATH && !out.configIni) out.configIni = path.join(sub(env.W906_AUTH_PATH), 'config.ini');
      // (the IO table: its Alias column is what an IO page's component names, lib/aliasedit.js)
      if (env.W906_IOTABLE_PATH && !out.ioTable) out.ioTable = sub(env.W906_IOTABLE_PATH);
    }
  }
  const ids = Object.keys(out.machines).sort((a, b) => out.machines[b] - out.machines[a]);
  out.machine = ids[0] || null;
  return out;
}

/** The machines a web folder knows (JSON/Machine-profile.json): { ids: [...], label: { id: text }, def }. */
function machineProfiles(webRoot) {
  try {
    const j = JSON.parse(fs.readFileSync(path.join(webRoot, 'JSON', 'Machine-profile.json'), 'utf8'));
    const p = (j && j.profiles) || {};
    const label = {};
    for (const id of Object.keys(p)) label[id] = (p[id] && p[id].label) || id;
    return { ids: Object.keys(p), label, def: (j && j.default) || null };
  } catch (e) {
    return { ids: [], label: {}, def: null };
  }
}

// WebBridgeRecipeBcb.h RecipeBcbValue: what BCB6's TIniFile::ReadString (GetPrivateProfileStringA) reads
function bcbValue(raw) {
  let b = 0, e = raw.length;
  const blank = ch => { const c = ch.charCodeAt(0); return c >= 1 && c <= 0x20; };
  while (b < e && blank(raw[b])) b++;
  while (e > b && blank(raw[e - 1])) e--;
  if (e - b >= 2 && raw[b] === raw[e - 1] && (raw[b] === '"' || raw[b] === "'")) { b++; e--; }
  return raw.slice(b, e).slice(0, 2047);
}

// WebBridgeRecipeDoc.cpp ClassifyRecipeField: "1" int, "2.00" float, anything else a string
function typed(bcb) {
  const t = bcb.trim();
  if (/^[+-]?\d+$/.test(t)) return { value: parseInt(t, 10), type: 'int' };
  if (/^[+-]?(\d+\.?\d*|\.\d+)$/.test(t)) return { value: parseFloat(t), type: 'float' };
  return { value: bcb, type: 'string' };
}

function decode(buf) {
  try { return new TextDecoder('utf-8', { fatal: true }).decode(buf); } catch (e) { return new TextDecoder('big5').decode(buf); }
}

/** One ini file, READ ONLY, in wb_serve's /api/system/<name> shape (vclcompat TIniStore grammar). */
function iniToDoc(file) {
  let buf;
  try { buf = fs.readFileSync(file); } catch (e) { return { path: file, available: false, sections: {} }; }
  const sections = {};
  let cur = null;
  for (const line of decode(buf).split(/\r\n|\r|\n/)) {
    const t = line.replace(/^[\x00-\x20]+/, '');
    if (!t || t[0] === ';' || t[0] === '#') continue;
    if (t[0] === '[') {
      const close = t.lastIndexOf(']');
      const name = (close > 0 ? t.slice(1, close) : t.slice(1)).trim();
      // the FIRST section of a name, the FIRST key in it (like kernel32): a repeated section is not read
      cur = Object.prototype.hasOwnProperty.call(sections, name) ? {} : (sections[name] = {});
      continue;
    }
    const eq = t.indexOf('=');
    if (eq < 0 || !cur) continue;
    const key = t.slice(0, eq).trim();
    if (!key || Object.prototype.hasOwnProperty.call(cur, key)) continue;
    const raw = t.slice(eq + 1);
    const bcb = bcbValue(raw);
    const v = typed(bcb);
    cur[key] = { value: v.value, type: v.type, raw, bcb };
  }
  return { path: file, available: true, sections };
}

module.exports = { stripJsonc, launchHints, machineProfiles, iniToDoc, bcbValue };
