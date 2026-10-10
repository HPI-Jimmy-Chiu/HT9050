'use strict';
// AI(W906-HTDESIGNER) 20261003 (machine, EastSun: "你要寫LOG紀錄 才能查異常" + "並且定時清理LOG"): the extension's log in files --
// one per day, htdesigner_YYYYMMDD.log, each line "YYYY-MM-DD HH:MM:SS.mmm [LEVEL] text"; old ones cleaned: older than
// `keepDays`, then the oldest while all of them are over `maxMB`. Plain Node (tests run it without VS Code). Writing never
// throws: a log that cannot be written must not stop what is being logged.
const fs = require('fs');
const path = require('path');

const NAME = /^htdesigner_(\d{8})\.log$/;
const two = n => String(n).padStart(2, '0');
const day = d => d.getFullYear() + two(d.getMonth() + 1) + two(d.getDate());
const stamp = d => d.getFullYear() + '-' + two(d.getMonth() + 1) + '-' + two(d.getDate()) + ' ' + two(d.getHours()) + ':' + two(d.getMinutes()) + ':' + two(d.getSeconds()) + '.' + String(d.getMilliseconds()).padStart(3, '0');

/**
 * create(dir, { keepDays = 14, maxMB = 50, now }) -> { dir, file(d), write(level, text), cleanup(now) -> { removed: [names] } }
 * `now`: a clock for the tests.
 */
function create(dir, opts) {
  const o = Object.assign({ keepDays: 14, maxMB: 50 }, opts || {});
  const clock = o.now || (() => new Date());
  let made = false;
  const file = d => path.join(dir, 'htdesigner_' + day(d) + '.log');
  function write(level, text) {
    try {
      if (!made) { fs.mkdirSync(dir, { recursive: true }); made = true; }
      const d = clock();
      // (one line per entry: a stack's lines indented under it)
      const body = String(text == null ? '' : text).replace(/\r?\n/g, '\n    ');
      const ln = stamp(d) + ' [' + String(level || 'INFO').toUpperCase() + '] ' + body + '\n';
      // (1009 second review: the folder deleted while VS Code runs ("開啟 LOG 資料夾" invites it) -- made again, not every later
      //  line lost)
      try { fs.appendFileSync(file(d), ln, 'utf8'); } catch (e) { if (e && e.code === 'ENOENT') { fs.mkdirSync(dir, { recursive: true }); fs.appendFileSync(file(d), ln, 'utf8'); } else throw e; }
      return true;
    } catch (e) { return false; }
  }
  function cleanup(nowArg) {
    const removed = [];
    let names = [];
    try { names = fs.readdirSync(dir).filter(n => NAME.test(n)).sort(); } catch (e) { return { removed }; }
    const now = nowArg || clock();
    const cut = new Date(now.getFullYear(), now.getMonth(), now.getDate() - o.keepDays);
    const cutDay = day(cut);
    const rm = n => { try { fs.unlinkSync(path.join(dir, n)); removed.push(n); return true; } catch (e) { return false; } };
    // older than keepDays (today's file is never removed)
    names = names.filter(n => (NAME.exec(n)[1] < cutDay ? !rm(n) : true));
    // then over the size cap: the oldest first
    const sizeOf = n => { try { return fs.statSync(path.join(dir, n)).size; } catch (e) { return 0; } };
    let total = names.reduce((s, n) => s + sizeOf(n), 0);
    const today = 'htdesigner_' + day(now) + '.log';
    for (const n of names.slice()) {
      if (total <= o.maxMB * 1024 * 1024) break;
      if (n === today) continue;
      const sz = sizeOf(n);
      if (rm(n)) total -= sz;
    }
    return { removed };
  }
  return { dir, file, write, cleanup };
}

module.exports = { create, NAME };
