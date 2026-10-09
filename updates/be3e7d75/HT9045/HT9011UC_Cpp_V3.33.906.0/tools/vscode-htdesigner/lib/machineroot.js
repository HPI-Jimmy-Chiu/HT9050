'use strict';
// AI(W906-HTDESIGNER) 20261008 (EastSun「只要是9050 都幫我改到 D:\HP9050 這邊執行」「我現在就是要分兩個資料夾 不然參數會混淆」;
// ruling: dev PCs only, the 9050 machine itself stays on D:\HT9045): a 9050 launch (its env opens machine=HT9050) on a PC
// whose setting ht9045Designer.hp9050Root is set gets every path it hands wb_serve moved under that root, laid out like
// the machine (README_PARAMS.txt of the 9050 snapshot):
//   D:\HT9045\...        -> <root>\HT9045\...
//   D:\HT9045_Log\...    -> <root>\HT9045_Log\...
//   D:\GPIB9045\...      -> <root>\GPIB9045\...
//   <tree>\..\runcfg\... -> <root>\runcfg\...        (the machine's D:\HT9045\_integ_ioweb\runcfg)
// launch.json is not changed (it is the machine's too). Paths the code has no env seam for are not reached from here.
// Plain Node (no vscode).

const path = require('path');

/** Is this launch a 9050 one: its environment opens the HMI with machine=HT9050 (the IOWEB entries do). */
function is9050(env) {
  return Object.keys(env || {}).some(k => /machine=HT9050\b/i.test(String(env[k])));
}

/** One path value moved under root (unchanged when it is none of the four places). */
function mapPath(v, root) {
  const s = String(v);
  const r = String(root).replace(/[\\/]+$/, '');
  // (only the folders htd_machine_switch.ps1 switches: the rest of D:\HT9045 is shared by both machines)
  const m = /^([A-Za-z]:)[\\/]+(HT9045[\\/]+(system|config|IniData)|HT9045_Log|GPIB9045[\\/]+system)(?=[\\/]|$)/i.exec(s);
  if (m) {
    const parts = m[2].split(/[\\/]+/);
    const fix = { ht9045: 'HT9045', ht9045_log: 'HT9045_Log', gpib9045: 'GPIB9045', system: 'system', config: 'config', inidata: 'IniData' };
    return r + '\\' + parts.map(x => fix[x.toLowerCase()] || x).join('\\') + s.slice(m[0].length).replace(/\//g, '\\');
  }
  const rc = /^(.*?)[\\/]runcfg(?=[\\/]|$)/i.exec(s);
  // (1009 review (F5 #4): a C++ tree under D:\HT9045 (D:\HT9045\HT9045_fromMachine) too -- its runcfg (the work order, teach.ini,
  //  config.ini) was left in the shared folder; the machine itself never gets here (no hp9050Root there))
  if (rc) return r + '\\runcfg' + s.slice(rc[0].length).replace(/\//g, '\\');
  return s;
}

/**
 * env {NAME: value} -> { env, changed: [{ name, from, to }] }: every W906_* value that is a path under the four places,
 * moved. Values that are not paths (ports, URLs, switches) are left alone.
 */
function remapEnv(env, root) {
  const out = Object.assign({}, env || {});
  const changed = [];
  if (!root) return { env: out, changed };
  for (const k of Object.keys(out)) {
    if (!/^W906_/.test(k) || /_URL$/.test(k)) continue;
    const v = String(out[k]);
    if (!/^[A-Za-z]:[\\/]|[\\/]runcfg([\\/]|$)/i.test(v)) continue;
    const n = mapPath(v, root);
    if (n !== v) { out[k] = n; changed.push({ name: k, from: v, to: n }); }
  }
  return { env: out, changed };
}

/** 1008: does this launch run the machine program (so the data folders must be switched first) */
function runsMachine(cfg) {
  const p = String((cfg && cfg.program) || '');
  // (1009 review (F5 #7): the tests and probes write the machine folders too (test_automation: IniData) -- the folders
  //  switched for them as well, not left on whichever machine ran last)
  return /(^|[\\/])(wb_(serve|publish|gateway)|test_\w+|ioweb_probe|pci1203_linkprobe)(\.exe)?$/i.test(p);
}

/** 1008 (audit A7): a launch's environment as { NAME: value } -- cppdbg's environment [{ name, value }], lldb-dap's env {}
 *  or ["K=V"] -- one reader, so the machine picked and the paths moved always agree */
function envOf(cfg) {
  const env = {};
  if (!cfg) return env;
  if (Array.isArray(cfg.environment)) for (const x of cfg.environment) if (x && x.name) env[x.name] = String(x.value);
  if (cfg.env && typeof cfg.env === 'object' && !Array.isArray(cfg.env)) for (const k of Object.keys(cfg.env)) env[k] = String(cfg.env[k]);
  if (Array.isArray(cfg.env)) for (const x of cfg.env) { const i = String(x).indexOf('='); if (i > 0) env[String(x).slice(0, i)] = String(x).slice(i + 1); }
  return env;
}

module.exports = { is9050, mapPath, remapEnv, runsMachine, envOf };
