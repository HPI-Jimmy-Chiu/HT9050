/* ht9045_wire_livesettings.js -- View-rules 規則改對 wb_serve 即時值求值（B 路）
 * ---------------------------------------------------------------------------
 * //Steven 20260918
 * ---------------------------------------------------------------------------
 * 為什麼有這個檔
 * ---------------------------------------------------------------------------
 * settings.js（HTSettings，A 路 JSON Simulator）把兩件混在一起的東西一起送：
 *
 *   規則式   View-rules.json 的 when / any / onFalse / note
 *            -> 設計期產物，由 _gen_view_rules.py 產生，裡面沒有任何機台值
 *               （實測 top keys 只有 schemaVersion/source/pathSyntax/ops/
 *                 onFalse/windows/blocks，沒有 sections 也沒有 quick），
 *               也沒有對應的 wb_serve 端點（它是衍生規則）。留在前端是對的。
 *   被評估的值
 *            -> 來自 General-config.json 這份「產生一次就不會再動」的快照。
 *               這才是要換掉的。
 *
 * 實測（20260918，wb_serve --dry）：
 *   web/JSON/General-config.json 才是 web/page/* 實際讀到的那一份
 *   （settings.js base() 把 /page/ 解析成 '../JSON/' = web/JSON/，
 *     不是根目錄的 JSON/ —— 這兩棵樹的同名檔內容不同，很容易看錯）
 *       generatedAt      2026-09-02T15:10:29+08:00
 *       quick.model      'HT-9046AT'     <- 實機 Gerneral.ini [Version] Model = HT-9050
 *       quick.machineId  'HT9046AT-136'  <- 實機 HT9050-001
 *       quick.serialNo   'LLS062'        <- 這個是對的
 *   序號對、型號錯 —— 對錯混雜最難察覺。
 *
 * ⚠ 誠實記一筆：根目錄 JSON/General-config.json 已在 20260909 重新產生過，
 *   佈署樹 web/JSON/ 那一份沒有跟著更新，兩棵因此不同步。
 *   而 ioview / teach 規則實際引用的那 6 個鍵
 *   （USE_IN_OUT_ARM_X_PITCH / USE_IN_OUT_ARM_Y_PITCH / USE_CATCH_TRAY_MODEL /
 *     USE_LOADER_HINGE / FIX3_INSTALL / USE_OUT_SORT_ARM）
 *   在 20260918 這一刻「快照與實機相同」。也就是說這兩頁的區塊顯隱
 *   目前剛好是對的 —— 缺陷是潛在的，不是正在發作的。
 *   但同一份快照的 Model 已經錯了一整個機種，證明這個機制真的會漂；
 *   漂到哪個欄位只是時間問題，所以照樣要把來源換掉。
 *
 * ---------------------------------------------------------------------------
 * 讀不到即時值時怎麼辦（這一段是安全判斷，請連同理由一起讀）
 * ---------------------------------------------------------------------------
 * HW.IoSetView / HW.teach 是 page-access-policy.md 第 1、3 列的 SystemStart
 * 守衛頁，是操作頁不是唯讀頁；規則決定 SortArm / OutSort / Fix3 / TrayArm 這類
 * 「選配硬體」的區塊要不要出現。兩個方向的失敗不對稱：
 *
 *   把不該顯示的顯示出來  -> 操作員可以對「機上沒有的硬體」下命令：
 *                            jog 一個不存在的軸、打一個沒裝的 Fix3 氣缸。
 *                            危險，而且畫面上看不出異常。
 *   把該顯示的藏起來      -> 操作員少一個控制項。動作上是安全的，
 *                            但 display:none 之後那個控制項「從來沒出現過」，
 *                            沒有人會發現它本來該在。一樣是靜默的。
 *
 * 兩個純選項都是靜默的，所以兩個都不選。讀不到時一律：
 *
 *   rule-dim（不是 rule-hide，即使該規則的 onFalse 寫的是 hide）
 *     -> 元素還留在畫面上（操作員看得到「這裡有東西而且不對勁」）
 *        ＋ pointer-events:none（按不下去，不可能誤觸不存在的硬體）
 *   ＋ rule-unknown 外框
 *   ＋ data-rule="規則未套用：<原因>"
 *   ＋ 頁面最上方橫幅
 *   ＋ 回傳物件標 unknown:true、show:null
 *      （與「求值後判定為隱藏」的 unknown:false、show:false 分得開）
 *
 * 也就是「拒絕 ＋ 大聲」：拿到 deny 的安全性，同時拿到 show 的可見性。
 * ⛔ 絕不默默沿用上一次的值，也絕不預設為「顯示」或「隱藏」。
 *
 * ---------------------------------------------------------------------------
 * 依據
 * ---------------------------------------------------------------------------
 *   quick.* 別名表   .claude/skills/ht9045-html-version/scripts/_gen_ini_json.py:66-97
 *                    （pick() 回傳 sec[n]["value"] —— 純量，不是 {value,raw}；
 *                      鍵不存在時回 None -> JSON null，'exists' 因此為假）
 *   blocks 規則      .claude/skills/ht9045-html-version/scripts/_gen_view_rules.py:50-58
 *   實機值           D:\HT9045\system\Gerneral.ini（本樹，不是 golden912）
 *                      [System]                   :1
 *                      USE_IN_OUT_ARM_Y_PITCH=2   :24
 *                      FIX3_INSTALL=1             :38
 *                      USE_CATCH_TRAY_MODEL=3     :63
 *                      USE_LOADER_HINGE=0         :74
 *                      USE_IN_OUT_ARM_X_PITCH=0   :90
 *                      [OutSortArm]               :673
 *                      USE_OUT_SORT_ARM=0         :674
 *
 * ⚠ /api/system/gerneral 的欄位形狀與快照完全相同（實測兩邊都是
 *   {"value":1,"type":"int","raw":"1"}），所以 sections.* 這條路徑是原地替換，
 *   求值邏輯一個字都不用改。只有 quick.* 要照上面的別名表自己合成。
 *
 * 相依：ht9045_recipe_client.js（HT9045System）要先載入。
 */
(function (global) {
  'use strict';

  /* quick.* -> Gerneral.ini [section] key
     依據 _gen_ini_json.py:71-97，逐條對照，沒有依據的不收。 */
  var QUICK = {
    customerCode:      ['System', 'CUSTOMER_CODE'],
    serialNo:          ['Version', 'Serial No'],
    machineId:         ['Version', 'Machine ID'],
    factory:           ['Version', 'Factory'],
    version:           ['Version', 'Ver'],
    useATC:            ['ATC', 'USE_ATC_MODE'],
    secsGemSystem:     ['SECS_GEM', 'SECS_GEM_SYSTEM'],
    motionCardType:    ['System', 'MOTION_CARD_TYPE'],
    indexMotionCard:   ['System', 'INDEX_MOTION_CARD'],
    ioCardType:        ['System', 'IO_CARD_TYPE'],
    indexDriverType:   ['IndexDriver', 'INDEX_DRIVER_TYPE'],
    trayArmMode:       ['System', 'TRAY_ARM_MODE'],
    useTrayMapping:    ['System', 'USE_TRAY_MAPPING'],
    useSocketSensor:   ['System', 'USE_SOCKET_SENSOR'],
    useAutoRetest:     ['System', 'USE_AUTO_RETEST'],
    realTimeCCD:       ['System', 'REAL_TIME_CCD'],
    installOCR:        ['System', 'INSTALL_OCR'],
    useInOutArmYPitch: ['System', 'USE_IN_OUT_ARM_Y_PITCH'],
    use2x4:            ['System', 'bHT9045S_USE2x4'],
    useOutShtMot:      ['System', 'USE_OUT_SHT_MOT'],
    fix3Install:       ['System', 'FIX3_INSTALL'],
    useCatchTrayModel: ['System', 'USE_CATCH_TRAY_MODEL'],
    /* settings.js resolveMachine() 的 machineFromModel 讀
       general.sections.Version.Model，實機 Gerneral.ini 裡就是這個鍵。 */
    model:             ['Version', 'Model']
  };

  var RULES_URL = 'View-rules.json';

  var cache = null;            /* {general, config, quick, loadedAt, source} */
  var rules = null;            /* View-rules.json（只有規則，沒有機台值） */
  var loadP = null;

  function rulesBase() {       /* 與 settings.js base() 同一套：page/* 在子目錄 */
    return /\/page\//.test(global.location.pathname) ? '../JSON/' : 'JSON/';
  }

  function getJson(url) {
    return new Promise(function (res, rej) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        var t = x.responseText || '';
        if ((x.status === 200 || x.status === 0) && t.length > 1) {
          try { res(JSON.parse(t)); } catch (e) { rej(e); }
        } else rej(new Error('http ' + x.status));
      };
      x.onerror = function () { rej(new Error('xhr error')); };
      x.send();
    });
  }

  function sysRead(name) {
    if (!global.HT9045System || !global.HT9045System.read) {
      return Promise.reject(new Error('沒有載入 ht9045_recipe_client.js（HT9045System 不存在）'));
    }
    return global.HT9045System.read(name).then(function (d) {
      if (!d || d.available === false) throw new Error('/api/system/' + name + ' available=false');
      if (!d.sections) throw new Error('/api/system/' + name + ' 沒有 sections');
      return d;
    });
  }

  /* 依 _gen_ini_json.py:66-70 pick()：回傳 sec[key]["value"]，找不到回 None。
     ⚠ 一定要是 null 不是 undefined —— 'exists' 判的是 !== undefined && !== null，
       在這裡兩者同結果，但只有 null 撐得過 JSON 化，別讓語意漂掉。 */
  function buildQuick(general) {
    var secs = (general && general.sections) || {}, out = {};
    Object.keys(QUICK).forEach(function (alias) {
      var sec = QUICK[alias][0], key = QUICK[alias][1];
      var ent = secs[sec] && secs[sec][key];
      out[alias] = (ent && typeof ent === 'object' && 'value' in ent) ? ent.value : null;
    });
    return out;
  }

  /* 一次把即時值抓齊。gerneral 失敗就整批 reject ——
     不做部分成功，因為「一半即時、一半沿用舊值」正是這次要消滅的東西。 */
  function load(force) {
    if (cache && !force) return Promise.resolve(cache);
    if (loadP && !force) return loadP;
    loadP = Promise.all([
      sysRead('gerneral'),
      /* config 目前沒有任何 ioview/teach 規則用到；缺了不擋，
         但也不會拿舊值頂替 —— 讀不到就是 null，路徑求值自然落空。 */
      sysRead('config').catch(function () { return null; }),
      rules ? Promise.resolve(rules) : getJson(rulesBase() + RULES_URL)
    ]).then(function (r) {
      rules = r[2] || { windows: {}, blocks: {} };
      cache = {
        general: r[0],
        config: r[1],
        quick: buildQuick(r[0]),
        loadedAt: new Date().toISOString(),
        source: '/api/system/gerneral'
      };
      global.__HT9045_LIVE__ = cache;
      return cache;
    });
    return loadP;
  }

  /* ---- 求值：與 settings.js 的 get/test/evalRule 逐行同語意 ----
     差別只有資料從哪來。刻意不共用 settings.js 的實作，
     因為共用就等於還要把 A 路整包載進來。 */
  function get(path) {
    if (!cache) return undefined;
    var parts = String(path).split('.'), cur, start;
    if (parts[0] === 'general') {
      if (parts[1] === 'quick') { cur = cache.quick; start = 2; }
      else { cur = cache.general; start = 1; }
    } else if (parts[0] === 'config') {
      cur = cache.config; start = 1;
    } else {
      return undefined;
    }
    for (var i = start; i < parts.length; i++) {
      if (cur == null) return undefined;
      cur = cur[parts[i]];
    }
    if (cur && typeof cur === 'object' && 'value' in cur && 'raw' in cur) cur = cur.value;
    return cur;
  }

  function test(cond) {
    var v = get(cond.path);
    switch (cond.op || 'truthy') {
      case 'truthy': return !!v && v !== '0';
      case 'falsy':  return !v || v === '0';
      case 'eq':     return v == cond.value;
      case 'ne':     return v != cond.value;
      case 'gt':     return Number(v) > Number(cond.value);
      case 'lt':     return Number(v) < Number(cond.value);
      case 'in':     return Array.isArray(cond.value) && cond.value.indexOf(v) >= 0;
      case 'exists': return v !== undefined && v !== null;
      default:       return false;
    }
  }

  function evalRule(rule) {
    if (!rule || !rule.when || !rule.when.length) return true;
    return rule.when[rule.any ? 'some' : 'every'](test);
  }

  function ensureStyle() {
    if (global.document.getElementById('htRuleStyle')) return;
    var st = global.document.createElement('style');
    st.id = 'htRuleStyle';
    st.textContent = '.rule-hide{display:none !important;}' +
      '.rule-dim{opacity:.35 !important;pointer-events:none !important;filter:grayscale(1);}' +
      '.rule-unknown{outline:2px dashed #c60 !important;}' +
      '#htRuleBanner{position:sticky;top:0;z-index:99999;background:#7a2b00;color:#ffd9b0;' +
      'font:12px/1.6 sans-serif;padding:4px 8px;border-bottom:1px solid #c60;}';
    global.document.head.appendChild(st);
  }

  function banner(msg) {
    var b = global.document.getElementById('htRuleBanner');
    if (!b) {
      if (!msg) return;
      b = global.document.createElement('div');
      b.id = 'htRuleBanner';
      global.document.body.insertBefore(b, global.document.body.firstChild);
    }
    b.textContent = msg || '';
    b.style.display = msg ? '' : 'none';
  }

  function clearMarks(el) {
    el.classList.remove('rule-hide', 'rule-dim', 'rule-unknown');
    el.removeAttribute('data-rule');
  }

  /* 讀不到即時值 -> 一律 dim ＋ unknown 標記，絕不 hide。理由見檔頭。 */
  function markUnknown(prefix, selectors, why) {
    ensureStyle();
    var out = {};
    Object.keys(selectors).forEach(function (name) {
      var els = global.document.querySelectorAll(selectors[name]), n = 0;
      for (var i = 0; i < els.length; i++) {
        var el = els[i]; n++;
        clearMarks(el);
        el.classList.add('rule-dim', 'rule-unknown');
        el.setAttribute('data-rule', '規則未套用：' + why);
      }
      out[name] = { show: null, action: 'dim', unknown: true, count: n, reason: why };
    });
    banner('⚠ 選配區塊規則未套用：讀不到 /api/system/gerneral（' + why + '）。' +
           '畫面上這些區塊一律變灰且不可操作，顯隱狀態不可信 —— ' +
           '不要依這個畫面判斷機上有沒有這些選配硬體。');
    return out;
  }

  /* 與 HTSettings.applyBlocks(prefix, selectors) 同介面、同回傳欄位，
     但回傳的是 Promise，且每一項多帶 unknown / source 兩個欄位（原欄位不動）。 */
  function applyBlocks(prefix, selectors) {
    return load().then(function () {
      ensureStyle();
      banner('');
      var blocks = (rules && rules.blocks) || {}, out = {};
      Object.keys(blocks).forEach(function (key) {
        if (key.indexOf(prefix + ':') !== 0) return;
        var name = key.substring(prefix.length + 1), sel = selectors[name];
        if (!sel) return;
        var rule = blocks[key], ok = evalRule(rule), act = rule.onFalse || 'hide';
        var els = global.document.querySelectorAll(sel), n = 0;
        for (var i = 0; i < els.length; i++) {
          var el = els[i]; n++;
          clearMarks(el);
          if (ok) continue;
          el.classList.add(act === 'dim' ? 'rule-dim' : 'rule-hide');
          el.setAttribute('data-rule', rule.note || key);
        }
        out[name] = { show: ok, action: ok ? null : act, count: n,
                      unknown: false, source: '/api/system/gerneral' };
      });
      return out;
    }).catch(function (e) {
      return markUnknown(prefix, selectors, (e && e.message) || String(e));
    });
  }

  /* 機種解析：與 settings.js resolveMachine() 同順序，但 Model 取即時值。
     實測 20260918：
       web/JSON 快照 model='HT-9046AT' -> 前綴 HT-9046 -> HT9045
       實機 Gerneral.ini [Version] Model='HT-9050' -> 前綴 HT-9050 -> HT9050
     也就是佈署樹上這一項「現在就是錯的」，不是潛在問題。 */
  function machineFromUrl() {
    var m = /[?&]machine=([A-Za-z0-9_\-]+)/.exec((global.location && global.location.search) || '');
    return m ? m[1] : '';
  }
  function machineFromLaunch() {
    try { return (JSON.parse(global.sessionStorage.getItem('ht9xxx-launch-options') || '{}') || {}).machine || ''; }
    catch (e) { return ''; }
  }
  function machineFromStore() {
    try { return global.localStorage.getItem('ht9xxx-machine') || ''; } catch (e) { return ''; }
  }
  function machine(profile) {
    var model = (cache && cache.quick && cache.quick.model) || '';
    var map = (profile && profile.versionModelPrefix) || {}, byModel = '';
    for (var prefix in map) {
      if (String(model).indexOf(prefix) === 0) { byModel = map[prefix]; break; }
    }
    /* localStorage 排在 INI 之後，理由同 settings.js：
       曾經開過 HT9050 不該讓機器永遠黏在 HT9050。 */
    var id = machineFromUrl() || machineFromLaunch() || byModel || machineFromStore() ||
             (profile && profile.default) || 'HT9045';
    var p = profile && profile.profiles && profile.profiles[id];
    var src = model ? ('live /api/system/gerneral [Version] Model=' + model)
                    : 'live 讀不到 [Version] Model';
    if (!p) {
      return { id: id, label: id, runtimeSupported: true, caps: {}, motionView: {},
               motors: { hidden: [] }, io: { hiddenGroups: [] }, profile: null, source: src };
    }
    return { id: id, label: p.label || id, runtimeSupported: p.runtimeSupported !== false,
             caps: p.caps || {}, motionView: p.motionView || {},
             motors: p.motors || { hidden: [] }, io: p.io || { hiddenGroups: [] },
             handlerSys: p.handlerSys || null, layoutModules: p.layoutModules || null,
             profile: p, source: src };
  }

  global.HT9045Live = {
    load: load, get: get, test: test, evalRule: evalRule,
    applyBlocks: applyBlocks, machine: machine,
    current: function () { return cache; },
    rules: function () { return rules; },
    quickMap: QUICK
  };
})(window);
