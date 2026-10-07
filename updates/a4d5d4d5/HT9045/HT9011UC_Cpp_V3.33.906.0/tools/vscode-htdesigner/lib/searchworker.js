'use strict';
// AI(W906-HTDESIGNER) 20261007 (1006 audit, search #5): the project search off the extension host's thread. A regex such as
// (a+)+$ on a long line runs for minutes inside one RegExp.exec -- a cancel flag is never looked at, the whole VS Code
// window froze. Here the search runs in a worker thread; cancel = the thread terminated, which stops even that.
const { Worker, isMainThread, parentPort, workerData } = require('worker_threads');

if (!isMainThread && workerData && workerData.htdSearch) {
  const projectsearch = require('./projectsearch');
  let stop = false;
  parentPort.on('message', m => { if (m && m.cancel) stop = true; });
  const { roots, source, flags, opts } = workerData;
  projectsearch.search(roots, new RegExp(source, flags), Object.assign({}, opts, {
    cancelled: () => stop,
    progress: (n, total) => parentPort.postMessage({ progress: [n, total] }),
  })).then(res => parentPort.postMessage({ res }), e => parentPort.postMessage({ error: String(e && e.message || e) }));
}

/**
 * search(roots, re, opts) as projectsearch.search, in a worker thread. opts.cancelled() is asked every 100 ms; when it
 * says so the worker is asked to stop, and terminated if it has not ended 1 s later (stuck in one regex) -- the result
 * is then { hits: [], cancelled: true, truncated: true, stuck: true }.
 */
function searchInWorker(roots, re, opts) {
  const o = opts || {};
  const plain = Object.assign({}, o);
  delete plain.cancelled; delete plain.progress;
  return new Promise((resolve, reject) => {
    let w;
    try {
      w = new Worker(__filename, { workerData: { htdSearch: true, roots, source: re.source, flags: re.flags, opts: plain }, resourceLimits: { maxOldGenerationSizeMb: 1024 } });
    } catch (e) { reject(e); return; }
    let done = false, asked = 0;
    const t0 = Date.now();
    const end = v => { if (done) return; done = true; clearInterval(iv); resolve(v); };
    const iv = setInterval(() => {
      if (done || !(o.cancelled && o.cancelled())) return;
      if (!asked) { asked = Date.now(); w.postMessage({ cancel: true }); return; }
      if (Date.now() - asked > 1000) {
        w.terminate().catch(() => null);
        end({ hits: [], files: 0, scanned: 0, total: 0, truncated: true, cancelled: true, stuck: true, ms: Date.now() - t0 });
      }
    }, 100);
    w.on('message', m => {
      if (m.progress && o.progress) o.progress(m.progress[0], m.progress[1]);
      else if (m.res) { end(m.res); w.terminate().catch(() => null); }
      else if (m.error) { done = true; clearInterval(iv); w.terminate().catch(() => null); reject(new Error(m.error)); }
    });
    w.on('error', e => { if (!done) { done = true; clearInterval(iv); reject(e); } });
    w.on('exit', () => { if (!done) end({ hits: [], files: 0, scanned: 0, total: 0, truncated: true, cancelled: true, ms: Date.now() - t0 }); });
  });
}

module.exports = { searchInWorker };
