'use strict';
// AI(W906-HTDESIGNER) 20261008 (machine; EastSun「外掛可以自動偵測編譯記憶體 不要超過最大值嗎?」「可以自動偵測該電腦記憶體 編譯自動配置?」):
// how many compilers a build may run at once, from THIS PC's memory at the moment the build starts -- not from its CPU count.
// Every build path used min(6, cores - 2): 6 on the machine (28 cores, 7.7 GB) whatever was free; a -j 4 full rebuild there
// crashed the PC 20261008 20:02 (Kernel-Power 41). Measured on the machine (one g++ -g at a time, peak private bytes):
// csystem.cpp / cinitial.cpp / Command.cpp 300 MB, wb_serve.cpp 356 MB, ainarm9045.cpp / aTester_Front.cpp 185 MB; a link of
// a ~100 MB Debug exe needs more -- hence 700 MB per job by default, and memory kept back for VS Code and the machine program.
const os = require('os');

const DEFAULTS = { perJobMB: 700, reserveMB: 1200, maxJobs: 6 };

/** -> { jobs, availMB, totalMB, byMem, byCpu, max, perJobMB, reserveMB, why }. Every input can be given (tests); missing = this PC. */
function plan(o) {
  o = o || {};
  const num = (v, d) => (typeof v === 'number' && isFinite(v) && v > 0 ? v : d);
  const availMB = Math.round(num(o.availMB, os.freemem() / 1048576));
  const totalMB = Math.round(num(o.totalMB, os.totalmem() / 1048576));
  const cores = Math.round(num(o.cores, (os.cpus() || []).length || 1));
  const perJobMB = num(o.perJobMB, DEFAULTS.perJobMB);
  const reserveMB = typeof o.reserveMB === 'number' && o.reserveMB >= 0 ? o.reserveMB : DEFAULTS.reserveMB;
  const max = Math.max(1, Math.round(num(o.maxJobs, DEFAULTS.maxJobs)));
  const byMem = Math.floor((availMB - reserveMB) / perJobMB);
  const byCpu = Math.max(1, cores - 2);
  const jobs = Math.max(1, Math.min(byMem, byCpu, max));
  const limit = jobs === max ? '上限' : jobs === byCpu && byCpu <= byMem ? 'CPU' : '記憶體';
  const why = '可用記憶體 ' + availMB + ' MB（共 ' + totalMB + ' MB），保留 ' + reserveMB + ' MB、每個 ' + perJobMB + ' MB → 同時 ' + jobs + ' 個編譯（受' + limit + '限制）';
  return { jobs, availMB, totalMB, byMem, byCpu, max, perJobMB, reserveMB, why };
}

module.exports = { plan, DEFAULTS };
