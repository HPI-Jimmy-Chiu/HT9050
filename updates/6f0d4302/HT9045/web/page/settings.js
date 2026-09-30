/* HT9045 設定檔載入器（HTSettings）——BCB6 設定/Setup → HTML 唯一資料來源（JSON）
   - JSON/General-config.json ← D:\HT9045\system\Gerneral.ini（機台選配；HandlerSys.dfm 寫入）
   - JSON/Config.json         ← D:\HT9045\config\config.ini（功能開關；cConfiguration.dfm 寫入）
   - JSON/View-rules.json     ← 視窗/畫面「是否顯示」規則（依上面兩份的值判斷）
    - JSON/Setup-index.json    ← 目前/可選工作檔與完整性
    - JSON/Setup-current.json  ← 目前工作檔 11 組 Recipe 資料
    - JSON/Production-runtime.json ← Lot Info / Arm / LotSummary / TEST_CATEGORY 生產快照
   background.html 啟動時先載入，再依規則決定各視窗是否建立/隱藏，並以 HT_SETTINGS 廣播給 iframe。
   各頁（teach / motionview / ioview / HandlerSys / cConfiguration）可直接呼叫 HTSettings.load() 取用。 */
(function (global) {
  var cache = null, loading = null, baseProduction = null, lastProductionSeq = -1;
  var MUTABLE_PRODUCTION = ['context','lotInfo','observerRecord','observerPanels','arms','lotSummary','testCategory','productionStreams','sitePanel','motionView'];

  function shimKey(u) {
    u = String(u).split('?')[0].split('#')[0];
    return u.substring(u.lastIndexOf('/') + 1).replace(/\.json$/i, '');
  }
  function base() {                       // background 在根目錄，page/* 在子目錄
    return /\/page\//.test(location.pathname) ? '../JSON/' : 'JSON/';
  }
  function viaScript(url) {
    return new Promise(function (res, rej) {
      var k = shimKey(url), st = global.__HT9045_DATA__ || {};
      if (st[k]) return res(st[k]);
      var s = document.createElement('script');
      s.src = base() + 'js/' + k + '.js';
      s.onload = function () { var d = (global.__HT9045_DATA__ || {})[k]; d ? res(d) : rej(new Error('shim empty ' + k)); };
      s.onerror = function () { rej(new Error('shim fail ' + k)); };
      document.head.appendChild(s);
    });
  }
  function viaNet(url) {
    return new Promise(function (res, rej) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        var t = x.responseText || '';
        if ((x.status === 200 || x.status === 0) && t.length > 1) { try { res(JSON.parse(t)); } catch (e) { rej(e); } }
        else rej(new Error('http ' + x.status));
      };
      x.onerror = function () { rej(new Error('xhr error')); };
      x.send();
    });
  }
  function loadJson(name) {
    var url = base() + name;
    if (location.protocol === 'file:') return viaScript(url).catch(function () { return viaNet(url); });
    return viaNet(url).catch(function () { return viaScript(url); });
  }
  function loadJsonFresh(name) {
    var url = base() + name;
    if (location.protocol !== 'file:') return viaNet(url);
    return new Promise(function (res, rej) {
      var key = shimKey(url), store = global.__HT9045_DATA__ || (global.__HT9045_DATA__ = {}), previous = store[key];
      delete store[key];
      var script = document.createElement('script');
      script.src = base() + 'js/' + key + '.js?_=' + Date.now();
      script.onload = function () { var value = store[key]; script.remove(); value ? res(value) : rej(new Error('shim empty ' + key)); };
      script.onerror = function () { if (previous) store[key] = previous; script.remove(); rej(new Error('shim fail ' + key)); };
      document.head.appendChild(script);
    });
  }
  function clone(value) {
    return value == null ? value : JSON.parse(JSON.stringify(value));
  }
  function makeArmDisplay(summary) {
    var total = Number(summary && summary.total) || 0;
    var pass = Number(summary && summary.pass) || 0;
    var fail = Number(summary && summary.fail) || 0;
    return {
      sum: pass + fail,
      passYield: total ? Number((pass * 100 / total).toFixed(4)) : 0,
      failYield: total ? Number((fail * 100 / total).toFixed(4)) : 0,
      binTotal: Object.keys((summary && summary.binCounts) || {}).reduce(function (totalBins, bin) {
        return totalBins + (Number(summary.binCounts[bin]) || 0);
      }, 0)
    };
  }
  function rebuildArmDisplays(production) {
    var canonical = production && production.arms && production.arms.canonical;
    if (!Array.isArray(canonical)) return production;
    canonical.forEach(function (arm) {
      if (arm && arm.summary) arm.display = makeArmDisplay(arm.summary);
    });
    return production;
  }
  function mergeProduction(baseValue, update) {
    var merged = Object.assign({}, baseValue || {}), state = update && update.state;
    if (state) MUTABLE_PRODUCTION.forEach(function (key) {
      if (Object.prototype.hasOwnProperty.call(state, key)) merged[key] = clone(state[key]);
    });
    return rebuildArmDisplays(merged);
  }
  function applyProductionDelta(production, update) {
    var merged = clone(production || {}), delta = update && update.delta;
    if (!delta) return null;
    if (!merged.arms || !Array.isArray(merged.arms.canonical)) merged.arms = { canonical: [] };
    var canonical = merged.arms.canonical;
    (delta.armCheckpoints || []).forEach(function (checkpoint) {
      var arm = canonical.filter(function (entry) { return entry && entry.arm === checkpoint.arm; })[0];
      if (!arm) { arm = { arm: checkpoint.arm, summary: {}, sites: [] }; canonical.push(arm); }
      arm.summary = Object.assign({}, arm.summary || {}, clone(checkpoint));
      delete arm.summary.arm;
    });
    (delta.sites || []).forEach(function (site) {
      var arm = canonical.filter(function (entry) { return entry && entry.arm === site.arm; })[0];
      if (!arm) { arm = { arm: site.arm, summary: {}, sites: [] }; canonical.push(arm); }
      if (!Array.isArray(arm.sites)) arm.sites = [];
      var existing = arm.sites.filter(function (entry) { return entry && entry.row === site.row && entry.col === site.col; })[0];
      if (existing) Object.assign(existing, clone(site));
      else arm.sites.push(clone(site));
    });
    if (delta.streamChanges) {
      merged.productionStreams = Object.assign({}, merged.productionStreams || {}, clone(delta.streamChanges));
    }
    return rebuildArmDisplays(merged);
  }
  function isDelta(update) {
    return update && update.messageType === 'test-complete-delta';
  }
  function initialProduction(update) {
    if (isDelta(update)) return rebuildArmDisplays(clone(baseProduction || {}));
    return mergeProduction(baseProduction, update);
  }
  function applyPhysicalRecipe(settings) {
    var physical = settings && settings.production && settings.production.context && settings.production.context.physicalRecipe;
    if (!physical || !physical.documents) return settings;
    settings.recipe = Object.assign({}, settings.recipe || {});
    settings.recipe.recipeName = physical.name || settings.recipe.recipeName;
    settings.recipe.documents = Object.assign({}, settings.recipe.documents || {}, clone(physical.documents));
    settings.recipe.quick = Object.assign({}, settings.recipe.quick || {}, {
      temperature: physical.temperature,
      soakTime: physical.soakTime,
      testMode: physical.runMode
    });
    return settings;
  }
  function notifyProductionResync(update, reason) {
    global.dispatchEvent(new CustomEvent('HT_PRODUCTION_RESYNC_REQUIRED', {
      detail: { reason: reason, receivedSeq: update && update.event && update.event.seq, lastSeq: lastProductionSeq }
    }));
  }

  // 路徑取值：'general.quick.useAutoRetest' / 'general.sections.System.FIX3_INSTALL' / 'config.sections.QA Mode.Enable QA Mode'
  // sections 下的 key 自動取 .value
  function get(path, dflt) {
    if (!cache) return dflt;
    var parts = String(path).split('.'), cur = cache;
    for (var i = 0; i < parts.length; i++) {
      if (cur == null) return dflt;
      cur = cur[parts[i]];
    }
    if (cur && typeof cur === 'object' && 'value' in cur && 'raw' in cur) cur = cur.value;
    return cur === undefined ? dflt : cur;
  }

  // 規則：{ when:[{path, op, value}], all:true } ；op: truthy|falsy|eq|ne|gt|lt|in|exists
  function test(cond) {
    var v = get(cond.path);
    switch (cond.op || 'truthy') {
      case 'truthy': return !!v && v !== '0';
      case 'falsy': return !v || v === '0';
      case 'eq': return v == cond.value;
      case 'ne': return v != cond.value;
      case 'gt': return Number(v) > Number(cond.value);
      case 'lt': return Number(v) < Number(cond.value);
      case 'in': return Array.isArray(cond.value) && cond.value.indexOf(v) >= 0;
      case 'exists': return v !== undefined && v !== null;
      default: return false;
    }
  }
  function evalRule(rule) {
    if (!rule || !rule.when || !rule.when.length) return true;
    var fn = rule.any ? 'some' : 'every';
    return rule.when[fn](test);
  }
  // 回傳 {show:bool, action:'hide'|'disable', reason}
  function decide(winId) {
    var rules = (cache && cache.rules && cache.rules.windows) || {};
    var r = rules[winId];
    if (!r) return { show: true, action: null, reason: 'no rule' };
    var ok = evalRule(r);
    return { show: ok, action: ok ? null : (r.onFalse || 'hide'), reason: r.note || '' };
  }
  function decideAll() {
    var out = {}, rules = (cache && cache.rules && cache.rules.windows) || {};
    for (var k in rules) out[k] = decide(k);
    return out;
  }
  // 頁內區塊：View-rules.json blocks["<prefix>:<name>"] → selectors[name]（CSS 選擇器）→ hide / dim
  function applyBlocks(prefix, selectors) {
    var blocks = (cache && cache.rules && cache.rules.blocks) || {}, out = {};
    for (var key in blocks) {
      if (key.indexOf(prefix + ':') !== 0) continue;
      var name = key.substring(prefix.length + 1), sel = selectors[name];
      if (!sel) continue;
      var ok = evalRule(blocks[key]), act = blocks[key].onFalse || 'hide', n = 0;
      var els = document.querySelectorAll(sel);
      for (var i = 0; i < els.length; i++) {
        var el = els[i]; n++;
        if (ok) { el.classList.remove('rule-hide', 'rule-dim'); el.removeAttribute('data-rule'); continue; }
        el.classList.add(act === 'dim' ? 'rule-dim' : 'rule-hide');
        el.setAttribute('data-rule', blocks[key].note || key);
      }
      out[name] = { show: ok, action: ok ? null : act, count: n };
    }
    if (!document.getElementById('htRuleStyle')) {
      var st = document.createElement('style'); st.id = 'htRuleStyle';
      st.textContent = '.rule-hide{display:none !important;}.rule-dim{opacity:.35 !important;pointer-events:none !important;filter:grayscale(1);}';
      document.head.appendChild(st);
    }
    return out;
  }

  // ---- 機種 profile（Machine-profile.json，HTML-only；BCB6 無對應，runtimeSupported:false）----
  // 解析順序：?machine= → sessionStorage 啟動選項 → Gerneral.ini [Version] Model 前綴 → localStorage → default
  // localStorage 排在 INI 之後，避免曾開過 HT9050 就永遠黏在 HT9050
  var FALLBACK_MACHINE = { id: 'HT9045', label: 'HT9045 / HT9046 系列', runtimeSupported: true, caps: {}, motionView: {}, motors: { hidden: [] }, io: { hiddenGroups: [] }, profile: null, source: 'fallback' };
  function machineFromUrl() {
    var m = /[?&]machine=([A-Za-z0-9_\-]+)/.exec(global.location && global.location.search || '');
    return m ? m[1] : '';
  }
  function machineFromLaunch() {
    try { return (JSON.parse(global.sessionStorage.getItem('ht9xxx-launch-options') || '{}') || {}).machine || ''; } catch (e) { return ''; }
  }
  function machineFromStore() {
    try { return global.localStorage.getItem('ht9xxx-machine') || ''; } catch (e) { return ''; }
  }
  function machineFromModel(profile, general) {
    var model = general && general.sections && general.sections.Version && general.sections.Version.Model;
    model = (model && model.value) || (general && general.quick && general.quick.model) || '';
    var map = (profile && profile.versionModelPrefix) || {};
    for (var prefix in map) if (String(model).indexOf(prefix) === 0) return map[prefix];
    return '';
  }
  function resolveMachine(profile, general) {
    var id = machineFromUrl() || machineFromLaunch() || machineFromModel(profile, general) ||
             machineFromStore() || (profile && profile.default) || 'HT9045';
    var p = profile && profile.profiles && profile.profiles[id];
    if (!p) return Object.assign({}, FALLBACK_MACHINE, { id: id, source: profile ? 'unknown-profile' : 'no-profile-json' });
    return {
      id: id,
      label: p.label || id,
      runtimeSupported: p.runtimeSupported !== false,
      caps: p.caps || {},
      motionView: p.motionView || {},
      motors: p.motors || { hidden: [] },
      io: p.io || { hiddenGroups: [] },
      handlerSys: p.handlerSys || null,
      layoutModules: p.layoutModules || null,
      profile: p,
      source: 'Machine-profile.json'
    };
  }

  function load(force) {
    if (cache && !force) return Promise.resolve(cache);
    if (loading && !force) return loading;
    loading = Promise.all([
      loadJson('General-config.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('Config.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('View-rules.json').catch(function (e) { return { __error: e.message, windows: {} }; }),
      loadJson('Setup-index.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('Setup-current.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('Production-runtime.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('Production-update.json').catch(function (e) { return { __error: e.message }; }),
      loadJson('Machine-profile.json').catch(function () { return null; })
    ]).then(function (r) {
      baseProduction = r[5];
      lastProductionSeq = isDelta(r[6]) ? -1 : Number(r[6] && r[6].event && r[6].event.seq);
      if (!isFinite(lastProductionSeq)) lastProductionSeq = -1;  lastProductionTag = null;   // AI(W906-SCRUBCACHE) 20261001
      cache = applyPhysicalRecipe({ general: r[0], config: r[1], rules: r[2], setup: r[3], recipe: r[4], production: initialProduction(r[6]), productionUpdate: r[6], loadedAt: new Date().toISOString() });
      cache.machine = resolveMachine(r[7], r[0]);
      if (isDelta(r[6])) notifyProductionResync(r[6], 'initial-message-must-be-full-snapshot');
      global.__HT9045_SETTINGS__ = cache;
      return cache;
    });
    return loading;
  }
  // 同步版：頁面已以 <script src="JSON/js/*.js"> 預載墊片時（background.html）可立即取得
  function loadSync() {
    var st = global.__HT9045_DATA__ || {};
    if (!st['General-config'] && !st['Config'] && !st['View-rules']) return null;
    baseProduction = st['Production-runtime'] || { __error: 'shim missing' };
    var update = st['Production-update'] || { __error: 'shim missing' };
    lastProductionSeq = isDelta(update) ? -1 : Number(update.event && update.event.seq);
    if (!isFinite(lastProductionSeq)) lastProductionSeq = -1;  lastProductionTag = null;   // AI(W906-SCRUBCACHE) 20261001
    cache = applyPhysicalRecipe({ general: st['General-config'] || { __error: 'shim missing' },
              config: st['Config'] || { __error: 'shim missing' },
              rules: st['View-rules'] || { __error: 'shim missing', windows: {} },
              setup: st['Setup-index'] || { __error: 'shim missing' },
              recipe: st['Setup-current'] || { __error: 'shim missing' },
              production: initialProduction(update), productionUpdate: update,
          loadedAt: new Date().toISOString(), sync: true });
    if (isDelta(update)) notifyProductionResync(update, 'initial-message-must-be-full-snapshot');
    cache.machine = resolveMachine(st['Machine-profile'] || null, cache.general);
    global.__HT9045_SETTINGS__ = cache;
    return cache;
  }

  // AI(W906-SCRUBCACHE) 20261001: background.html calls this every 250 ms. Production-update.json is 8.6 MB and nothing in
  //   the C++ port rewrites it, yet every call downloaded and JSON.parse'd all of it, and each GET cost wb_serve's socket
  //   thread 115-132 ms -- the thread that also carries the stop command (NB2 problem A, v906/nb2-assist
  //   docs/nb2_assist/notes_webref/P1_STOPLAT_first_load_stall.md). Over http a HEAD goes first: wb_serve gives an ETag
  //   only for a file whose stamp has settled (tools/wb_serve.cpp W906_JsonScrubRoute, WebJsonScrubCache.h), and while it
  //   equals the ETag of the last body parsed here the GET is skipped -- that body would get the same answer again
  //   (seq <= lastProductionSeq -> false). No ETag (file://, a file written < 2 s ago, an older server, the HEAD failed)
  //   -> the GET as before. A body that asked for a resync keeps no ETag, so that request repeats on every call as before;
  //   load() / loadSync() reset the ETag with lastProductionSeq.
  var lastProductionTag = null, productionResyncAsked = false;
  function headTag(url) {
    return new Promise(function (res) {
      var x = new XMLHttpRequest();
      x.open('HEAD', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () { if (x.readyState === 4) res(x.status === 200 ? (x.getResponseHeader('ETag') || null) : null); };
      x.onerror = function () { res(null); };
      x.send();
    });
  }
  function viaNetTagged(url) {             // viaNet + the response's ETag
    return new Promise(function (res, rej) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        var t = x.responseText || '';
        if ((x.status === 200 || x.status === 0) && t.length > 1) { try { res({ data: JSON.parse(t), tag: x.getResponseHeader('ETag') || null }); } catch (e) { rej(e); } }
        else rej(new Error('http ' + x.status));
      };
      x.onerror = function () { rej(new Error('xhr error')); };
      x.send();
    });
  }
  function refreshProduction() {
    var url = base() + 'Production-update.json', got;
    if (location.protocol === 'file:') got = loadJsonFresh('Production-update.json').then(function (d) { return { data: d, tag: null }; });
    else got = headTag(url).then(function (tag) { return (tag && tag === lastProductionTag) ? null : viaNetTagged(url); });
    return got.then(function (r) {
      if (!r) return false;
      productionResyncAsked = false;
      var changed = applyProductionUpdate(r.data);
      lastProductionTag = productionResyncAsked ? null : r.tag;
      return changed;
    });
  }
  function applyProductionUpdate(update) {
      var seq = Number(update && update.event && update.event.seq);
      if (!isFinite(seq) || seq <= lastProductionSeq) return false;
      if (isDelta(update)) {
        var baseSeq = Number(update.event && update.event.baseSeq);
        if (!isFinite(baseSeq) || baseSeq !== lastProductionSeq) {
          productionResyncAsked = true;
          notifyProductionResync(update, 'sequence-gap');
          return false;
        }
        cache.production = applyProductionDelta(cache.production, update);
      } else {
        cache.production = mergeProduction(baseProduction, update);
      }
      lastProductionSeq = seq;
      cache.productionUpdate = update;
      cache.loadedAt = new Date().toISOString();
      applyPhysicalRecipe(cache);
      global.__HT9045_SETTINGS__ = cache;
      if (global.HTSettings.onChange) global.HTSettings.onChange(cache, decisions);
      global.dispatchEvent(new CustomEvent('HT_SETTINGS_REFRESHED', { detail: { settings: cache, decisions: decisions } }));
      return true;
  }

  var decisions = null;

  // iframe 端：background 廣播 HT_SETTINGS 時直接採用（省一次載入）
  global.addEventListener('message', function (ev) {
    if (ev.data && ev.data.type === 'HT_SETTINGS' && ev.data.settings) {
      cache = ev.data.settings; global.__HT9045_SETTINGS__ = cache;
      decisions = ev.data.decisions || null;
      if (global.HTSettings.onChange) global.HTSettings.onChange(cache, decisions);
    }
  });

  global.HTSettings = {
    load: load, loadSync: loadSync, get: get, decide: decide, decideAll: decideAll, applyBlocks: applyBlocks,
    test: test, evalRule: evalRule,
    machine: function () { return (cache && cache.machine) || FALLBACK_MACHINE; },
    loadJson: loadJson, loadJsonFresh: loadJsonFresh, refreshProduction: refreshProduction, current: function () { return cache; }, decisions: function () { return decisions; },
    onChange: null,
    // 子頁向 background 索取（也會在 iframe load 後自動收到）
    request: function () { if (global.parent !== global) global.parent.postMessage({ getSettings: 1 }, '*'); }
  };
})(window);
