// AI(W906-SCRUBCACHE) 20261001: web/page/settings.js refreshProduction -- HEAD + ETag before the 8.6 MB GET (NB2 problem A,
//   v906/nb2-assist docs/nb2_assist/notes_webref/P1_STOPLAT_first_load_stall.md; the server half is WebJsonScrubCache).
//   Runs the real settings.js in a node vm with a fake XMLHttpRequest "server" (body + optional ETag) and pins:
//   an unchanged ETag -> HEAD only (no GET, no parse); a new ETag -> GET and the same seq / delta rules as before; no ETag or
//   a failed HEAD -> the GET every time (today's behaviour); a resync-asking delta keeps no ETag, so the resync repeats every
//   call as before; loadSync() resets the ETag; file:// sends no XHR at all (the shim path, unchanged).
//   Control: W906_SCRUBCACHE_PAGE_DIR (or argv[2]) -> a directory holding the pre-change settings.js must be red.
//   Offline: no wb_serve, no machine files. Usage: through ctest (ScrubCache_SettingsEtag).
'use strict';
const fs = require('fs');
const path = require('path');
const vm = require('vm');

const PAGE_DIR = process.env.W906_SCRUBCACHE_PAGE_DIR || process.argv[2] || path.join(__dirname, '..', '..', '..', 'web', 'page');
let passed = 0, failed = 0;
function check(name, cond, detail) {
  if (cond) { passed++; console.log('PASS ' + name); }
  else { failed++; console.log('FAIL ' + name + (detail ? ' -- ' + detail : '')); }
}
const same = (a, b) => JSON.stringify(a) === JSON.stringify(b);

const URL = 'JSON/Production-update.json';
const HEAD = 'HEAD ' + URL, GET = 'GET ' + URL;
const server = { body: null, tag: null, headFails: false, log: [] };
class FakeXHR {
  open(method, url) { this.method = method; this.url = url; this.readyState = 1; }
  getResponseHeader(h) { return String(h).toLowerCase() === 'etag' ? this.tag : null; }
  send() {
    server.log.push(this.method + ' ' + this.url.split('?')[0]);
    setImmediate(() => {
      this.readyState = 4;
      if (this.method === 'HEAD' && server.headFails) { this.status = 500; this.tag = null; this.responseText = ''; }
      else { this.status = 200; this.tag = server.tag; this.responseText = this.method === 'HEAD' ? '' : JSON.stringify(server.body); }
      if (this.onreadystatechange) this.onreadystatechange();
    });
  }
}

const full = (seq) => ({ messageType: 'full-snapshot', event: { seq }, state: { lotInfo: { seq } } });
const delta = (seq, baseSeq) => ({ messageType: 'test-complete-delta', event: { seq, baseSeq }, delta: { sites: [] } });

function makeCtx(protocol) {
  const events = [];
  const ctx = {
    location: { protocol, pathname: '/background.html', search: '' },
    XMLHttpRequest: FakeXHR,
    CustomEvent: class { constructor(type, init) { this.type = type; this.detail = init && init.detail; } },
    dispatchEvent: (e) => { events.push(e.type); return true; },
    addEventListener: () => {},
    sessionStorage: { getItem: () => null },
    localStorage: { getItem: () => null },
    document: {
      createElement: () => ({ remove() {} }),
      // file:// shim path of loadJsonFresh: the <script> "loads" the current body into __HT9045_DATA__
      head: { appendChild: (s) => setImmediate(() => { ctx.__HT9045_DATA__['Production-update'] = server.body; s.onload(); }) },
    },
    console, JSON, Promise, Date, Number, Object, Array, String, Math, isFinite, setTimeout, setImmediate,
  };
  ctx.window = ctx;
  ctx.parent = ctx;
  ctx.__events = events;
  ctx.__HT9045_DATA__ = {
    'General-config': { quick: {}, sections: {} }, 'Config': { sections: {} }, 'View-rules': { windows: {} },
    'Production-runtime': {}, 'Production-update': full(5),
  };
  vm.createContext(ctx);
  vm.runInContext(fs.readFileSync(path.join(PAGE_DIR, 'settings.js'), 'utf8'), ctx, { filename: 'settings.js' });
  return ctx;
}

async function main() {
  const ctx = makeCtx('http:');
  const S = ctx.HTSettings;
  check('settings.js loads, loadSync() builds the cache (seq 5)', !!(S && S.loadSync()));
  const poll = async () => {
    server.log.length = 0;
    ctx.__events.length = 0;
    const v = await S.refreshProduction();
    return { v, log: server.log.slice(), ev: ctx.__events.slice() };
  };

  server.body = full(5); server.tag = 'W/"a"';
  let r = await poll();
  check('[1] first call: HEAD, then the GET', same(r.log, [HEAD, GET]), JSON.stringify(r.log));
  check('[1] same seq as loadSync -> false', r.v === false);
  r = await poll();
  check('[2] ETag unchanged: HEAD only -- no 8.6 MB GET, no parse', same(r.log, [HEAD]), JSON.stringify(r.log));
  check('[2] -> false (what the GET would have answered)', r.v === false);

  server.body = full(6); server.tag = 'W/"b"';
  r = await poll();
  check('[3] new ETag: HEAD + GET, newer seq -> true', same(r.log, [HEAD, GET]) && r.v === true, JSON.stringify(r));
  check('[3] HT_SETTINGS_REFRESHED fired', r.ev.indexOf('HT_SETTINGS_REFRESHED') >= 0);
  check('[3] merged state is seq 6', S.current().production.lotInfo.seq === 6);
  r = await poll();
  check('[3b] then HEAD only again', same(r.log, [HEAD]) && r.v === false, JSON.stringify(r.log));

  server.tag = null;                                                    // unsettled file / older server: no ETag
  r = await poll();
  check('[4] no ETag: HEAD + GET', same(r.log, [HEAD, GET]) && r.v === false, JSON.stringify(r.log));
  r = await poll();
  check('[4b] no ETag: the GET every time (as before)', same(r.log, [HEAD, GET]), JSON.stringify(r.log));

  server.headFails = true; server.tag = 'W/"b"';
  r = await poll();
  check('[5] HEAD fails -> the GET as before', same(r.log, [HEAD, GET]) && r.v === false, JSON.stringify(r.log));
  server.headFails = false;

  server.body = delta(8, 7); server.tag = 'W/"c"';                     // gap: last applied is 6
  r = await poll();
  check('[6] gap delta: GET, false, resync requested', same(r.log, [HEAD, GET]) && r.v === false &&
        r.ev.indexOf('HT_PRODUCTION_RESYNC_REQUIRED') >= 0, JSON.stringify(r));
  r = await poll();
  check('[6b] same ETag after a resync request: GET and resync again (kept no ETag)', same(r.log, [HEAD, GET]) &&
        r.ev.indexOf('HT_PRODUCTION_RESYNC_REQUIRED') >= 0, JSON.stringify(r));

  server.body = delta(7, 6); server.tag = 'W/"d"';                     // fits: baseSeq 6 = last applied
  r = await poll();
  check('[7] fitting delta: applied -> true', same(r.log, [HEAD, GET]) && r.v === true, JSON.stringify(r));
  r = await poll();
  check('[7b] then HEAD only', same(r.log, [HEAD]) && r.v === false, JSON.stringify(r.log));

  S.loadSync();                                                         // lastProductionSeq back to 5 (the shim)
  r = await poll();
  check('[8] loadSync() resets the ETag: HEAD + GET, the delta (base 6) vs 5 asks for a resync', same(r.log, [HEAD, GET]) &&
        r.v === false && r.ev.indexOf('HT_PRODUCTION_RESYNC_REQUIRED') >= 0, JSON.stringify(r));

  const f = makeCtx('file:');
  f.HTSettings.loadSync();
  server.body = full(9); server.tag = 'W/"e"'; server.log.length = 0;
  const v = await f.HTSettings.refreshProduction();
  check('[9] file://: no XHR at all (the shim path), newer seq -> true', server.log.length === 0 && v === true,
        JSON.stringify(server.log) + ' ' + v);

  console.log('scrubcache_settings_selftest: ' + passed + ' passed, ' + failed + ' failed');
  process.exit(failed ? 1 : 0);
}
main().catch((e) => { console.log('FAIL exception: ' + (e && e.stack || e)); process.exit(1); });
