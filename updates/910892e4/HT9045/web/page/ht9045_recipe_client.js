//Steven 20260916
// ----------------------------------------------------------------------
// 兩件事：
// (1) HT9045Tags —— 執行期 tag 串流。onmessage 原本只有 ack 一個分支、沒有
//     else，伺服器每 500ms 送的 snapshot／patch 與 alarm／modal／query 全部
//     被靜默丟棄。C++ 端早就完整，缺口一直只在這一段。
// (2) HT9045System.levels —— 二進位定長陣列的個別元素寫入（system.levels.put），
//     給 Status.Security 的 179 組 radio（system\levelset.dat）用。
// 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
// ----------------------------------------------------------------------

//Steven 20260915
// ----------------------------------------------------------------------
// 修正 dryRun 被靜默丟棄的 bug：WebCommand 只認 {id,cmd,tag,value,connId}，
// dryRun 當兄弟欄位會被丟掉，導致 preview() 在操作員確認前就真的寫下去。
// 已改為放進 value 裡面。另新增 HT9045System 全域（list/read/write/preview）。
// 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
// ----------------------------------------------------------------------

// ht9045_recipe_client.js -- the browser side of the HT9045 recipe read/write
// pipe, in the shape the hand-built HMI already uses.
//
// AI(W906-BA-C1CLIENT) 20260911.
//
// ---------------------------------------------------------------------------
// WHY THIS FILE EXISTS, AND WHY IT IS NOT AN INVISIBLE ADAPTER
// ---------------------------------------------------------------------------
// The HMI has its own write seam (HTJsonWriter / HTDialogHost) and it was
// tempting to override that so pages would "just work" with no edits. Measured
// 20260911, that would connect nothing: the documents the HMI writes through
// HTJsonWriter are dialog responses, motor-access / state-record (both guarded
// by cfg.offline), and a Config / General-config EXPORT whose own status
// message says 需工程師轉回 .ini. None of them is a recipe document, and the
// recipe pipe is the only write path C++ currently accepts. An override would
// have produced something that looked wired and was not.
//
// So this is an explicit client instead. It is small, it hides nothing, and it
// matches the agreed workflow: the pipe is built and proven first, then the
// pages are adapted to call it.
//
// ---------------------------------------------------------------------------
// WHAT THE SERVER OFFERS  (tools/wb_serve.cpp)
// ---------------------------------------------------------------------------
//   GET  /api/recipe/          -> {recipe, path, documents:[...]}
//   GET  /api/recipe/<doc>     -> {path, available, sections:{<sec>:{<key>:
//                                  {value, type, raw}}}}
//   ws://<host>/ht9045         -> {"type":"cmd","id":N,"cmd":"control.acquire"}
//                                 {"type":"cmd","id":N,"cmd":"recipe.doc.put",
//                                  "tag":"<doc>","value":"<json string>"}
//
// <doc> is the file stem with the first letter lowercased -- handlerCondition,
// testMode, contact, udUld ... i.e. exactly the keys Setup-current.json already
// uses under documents.*, so existing page code needs no renaming.
//
// THE PER-FIELD SHAPE IS ALREADY COMPATIBLE. settings.js:154 unwraps a value
// only when BOTH 'value' and 'raw' are present, and the server emits value,
// type and raw. Had it emitted only {raw,type}, that guard would have failed
// and the page would have received the whole object instead of the scalar --
// silently, rendering [object Object]. Do not "simplify" the payload.
//
// ---------------------------------------------------------------------------
// THREE CONSTRAINTS THAT ARE NOT NEGOTIABLE
// ---------------------------------------------------------------------------
// 1. WRITES GO OVER THE WEBSOCKET, NOT HTTP. The HTTP side answers GET and HEAD
//    only and returns 405 otherwise, because the server's request parser stops
//    at the end of the headers -- there is no request-body buffering anywhere in
//    the connection state machine. A POST would need that machinery, not just a
//    route.
// 2. value MUST BE A STRING. The command channel carries null/bool/number/string
//    and nothing else, so the document is sent as a JSON string, not an object.
//    write() does that for you.
// 3. ONE WRITER AT A TIME. control.acquire takes a single-operator token; a
//    second connection is refused while it is held. acquire() is called for you
//    on first write and the token is released on disconnect.
//
// ---------------------------------------------------------------------------
// USAGE
// ---------------------------------------------------------------------------
//   HT9045Recipe.list().then(function (r) { console.log(r.recipe, r.documents); });
//
//   HT9045Recipe.read('contact').then(function (doc) {
//     console.log(doc.sections['Test Arm1']['Pick Up'].value);   // 1.25
//   });
//
//   // Always preview first. dryRun reports what WOULD change and writes nothing.
//   HT9045Recipe.write('contact', {'Test Arm1': {'Pick Up': '1.25'}}, {dryRun: true})
//     .then(function (res) { console.log(res.changed, res.identical, res.notFound); });
//
// The server rewrites ONLY the value text after the first '=' on the lines you
// name. Everything else in the file is copied byte for byte, so comments,
// ordering, blank lines and even orphaned keys survive. That is deliberate:
// re-serialising from a parsed store would silently drop the 23 orphaned
// parameters in the one corrupted recipe document on this machine.
(function (global) {
  'use strict';

  var cfg = {
    base: '',                 // same-origin by default
    wsPath: '/ht9045',
    timeoutMs: 15000
  };

  function httpBase() {
    return cfg.base || '';
  }

  function getJson(url) {
    // XMLHttpRequest rather than fetch, to match the HMI's own loader and keep
    // this usable from the same places settings.js runs.
    return new Promise(function (resolve, reject) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        if (x.status < 200 || x.status >= 300) {
          reject(new Error('GET ' + url + ' -> HTTP ' + x.status));
          return;
        }
        try { resolve(JSON.parse(x.responseText)); }
        catch (e) { reject(new Error('GET ' + url + ' -> bad JSON: ' + e.message)); }
      };
      x.onerror = function () { reject(new Error('GET ' + url + ' -> network error')); };
      x.send();
    });
  }

  // --- the command socket -------------------------------------------------
  var sock = null, nextId = 1, pending = {}, opening = null, haveToken = false;  var tokenIdleMs = 30000, tokenTimer = null, tokenInflight = 0, tokenHolds = [];   // AI(W906-TOKEN-IDLE) 20260926: 閒置自動還權杖，見 global.HT9045Recipe 上方那段

  // Steven 20260918 (W906-FW-WINREG)：ui.windows.put 這個指令**伺服器還不接受**。
  //   null = 還沒問過（或剛重連）、true = 送得進去、false = 被拒過。
  // 斷線時要歸回 null —— 換一個伺服器版本上來就該重新問一次，
  // 而不是因為上一代不支援就永遠不再嘗試。
  var winSupported = null, winRefusal = '';  var linkWait = (function () { try { if (global.HT9045Link || global.parent === global || !document.currentScript || !document.currentScript.src) return null; var el = document.createElement('script'); el.src = document.currentScript.src.replace(/[^\/?#]*([?#].*)?$/, 'ht9045_link.js'); return new Promise(function (res) { var t = setTimeout(res, 2000); el.onload = el.onerror = function () { clearTimeout(t); res(); }; (document.head || document.documentElement).appendChild(el); }); } catch (e) { return null; } })();   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：一個瀏覽器分頁只開一條 WebSocket —— 在外框（background.html）的 iframe 裡，這一頁的連線經外框的 hub（同資料夾的 ht9045_link.js）；頁面沒載它就在這裡載一次，connect() 等它（最多 2 秒），載不到／外框沒有 hub ＝照舊直接開 WebSocket

  function wsUrl() {
    if (cfg.base) return cfg.base.replace(/^http/, 'ws') + cfg.wsPath;
    return (location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + cfg.wsPath;
  }

  function connect() {
    if (sock && sock.readyState === 1) return Promise.resolve(sock);
    if (opening) return opening;
    opening = (linkWait || Promise.resolve()).then(function () { return new Promise(function (resolve, reject) {   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：先等上面的 linkWait
      var s;
      try { s = global.HT9045Link ? global.HT9045Link.open(wsUrl()) : new WebSocket(wsUrl()); }   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：經外框的 hub（ht9045_link.js；Jimmy 20260929 機台 23 條撞 16 條上限）；沒有 HT9045Link 時跟以前一樣
      catch (e) { opening = null; reject(e); return; }
      s.onopen = function () { sock = s; opening = null; resolve(s); };
      s.onerror = function () { opening = null; reject(new Error('cannot open ' + wsUrl())); };
      s.onclose = function () {
        sock = null; haveToken = false; if (tokenTimer) { clearTimeout(tokenTimer); tokenTimer = null; }   // AI(W906-TOKEN-IDLE) 20260926
        winSupported = null; winRefusal = '';   // Steven 20260918：重連後重新試一次
        //Steven 20260916 (W906-FW-TAGSUB): 斷線要說出來。留著 connected=true
        // 會讓看板頁面把上一代的殘值當成活的資料繼續顯示。
        tagStat.connected = false;
        // Fail everything still in flight rather than leaving callers hanging.
        Object.keys(pending).forEach(function (k) {
          pending[k].reject(new Error('socket closed before ack'));
          delete pending[k];
        });
      };
      s.onmessage = function (ev) {
        var m;
        try { m = JSON.parse(ev.data); } catch (e) { return; }
        if (!m) return;  if (m.type === 'link.token') { if (m.owner === false) { haveToken = false; if (tokenTimer) { clearTimeout(tokenTimer); tokenTimer = null; } } return; }   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：hub 看到 not-operator（別的瀏覽器接管、或伺服器收回）會通知每一頁；清掉 haveToken，下一個自動動作重新 acquire（同 :170 的 not-operator）
        if (m.type === 'ack') {
          // AI(W906-MT-FIX1) 20260925：伺服器說「你不是操作員」＝權杖已經不在我們手上（閒置 10 分鐘被收回
          //   WebBridgeServer.cpp:1566-1569、或被別的連線拿走）。原本 haveToken 一直留 true，之後每一條
          //   motor.access／存檔都不再 acquire、全部被拒，直到重新整理 HMI。清掉它，下一條指令會重新 acquire。
          if (!m.ok && m.error === 'not-operator') haveToken = false;
          if (pending[m.id]) {
            var p = pending[m.id]; delete pending[m.id];
            if (m.ok) p.resolve(m); else p.reject(new Error(m.error || 'command refused'));
          }
          return;
        }
        //Steven 20260916 (W906-FW-TAGSUB)
        // 這裡原本只有 ack 一個分支，沒有 else —— 伺服器每 500ms 送來的
        // snapshot／patch，以及 alarm／modal／query，全部靜默丟棄。
        // C++ 端的串流早就完整（WebBridgeServer.cpp:1125 snapshot、:1443 patch、
        // :1534 alarm、:1558 modal、:1582 query；wb_serve.cpp 的 500ms tick 重發），
        // 缺口一直只在這一段。
        tagFrame(m);
      };
    }); });   // AI(W906-WSLINK) 20260929 [W906] ST01-E3：linkWait.then 的收尾
    return opening;
  }

  function cmd(name, extra) { return tokenTrack(name, extra, cmd0(name, extra), false); }   // AI(W906-TOKEN-IDLE) 20260926: 權杖記帳包一層，原本體改名 cmd0
  function cmd0(name, extra) {
    return connect().then(function (s) {
      return new Promise(function (resolve, reject) {
        var id = nextId++;
        var msg = {type: 'cmd', id: id, cmd: name};
        if (extra) Object.keys(extra).forEach(function (k) { msg[k] = extra[k]; });
        pending[id] = {resolve: resolve, reject: reject};
        var timer = setTimeout(function () {
          if (pending[id]) {
            delete pending[id];
            reject(new Error(name + ': no ack within ' + cfg.timeoutMs + 'ms'));
          }
        }, cfg.timeoutMs);
        var done = function () { clearTimeout(timer); };
        var p = pending[id];
        pending[id] = {
          resolve: function (v) { done(); p.resolve(v); },
          reject:  function (e) { done(); p.reject(e); }
        };
        s.send(JSON.stringify(msg));
      });
    });
  }

  // Steven 20260924：伺服器把回傳 JSON 的欄位併進 ack 物件；舊寫法可能放在 value 字串裡 —— 兩種都收
  function unwrapAck(m) {
    if (m && typeof m.value === 'string') { try { var j = JSON.parse(m.value); if (j && typeof j === 'object') return j; } catch (e) {} }
    return m;
  }

  function acquire() {
    if (haveToken) { armTokenIdle(); return Promise.resolve(); }   // AI(W906-TOKEN-IDLE) 20260926: 要用權杖就把閒置計時往後推
    return cmd('control.acquire').then(function () { haveToken = true; armTokenIdle(); });
  }
  // AI(W906-TAKEOVER) 20260926：EastSun「我在哪一頁按按鈕，那一頁就把控制權拿回來」（實測：IO 頁接管後回 Motor Test 按
  //   btnServoOff 得到 control-held）。操作員親手按的（馬達按鈕、存檔）每次都送 control.takeover —— 不管權杖在哪一頁，
  //   也順便修掉「以為自己還拿著、其實已被別頁接管」的過期 haveToken；自動送的（開頁／關頁同步、開頁讀資料、續約）照舊 acquire()，不搶。
  function takeover() {
    return cmd('control.takeover').then(function () { haveToken = true; armTokenIdle(); });   // AI(W906-MERGE-0929) 20260929: arm the idle timer like acquire() does (AI(W906-TOKEN-IDLE) 20260926); otherwise a token taken over is never released
  }

  // ---------------------------------------------------------------------------
  //  HT9045Tags -- 執行期 tag 串流（snapshot / patch）
  //  AI(W906-FW-TAGSUB) 20260916
  // ---------------------------------------------------------------------------
  //  伺服器每 500ms 重新 publish 一次，變動的部分以 patch 送出，新連線先收一個
  //  完整 snapshot。這裡只做兩件事：維護本地那份 tag 表、通知訂閱者。
  //  它刻意不碰 DOM —— 哪個 tag 對到哪個 widget 是各頁接線檔的事，混在這裡會
  //  變成第二張隱形的對照表。
  //
  //  ⚠ null 是有意義的值，不是「沒收到」。
  //    伺服器契約（WebBridge/TagSnapshot.h section 4 rule 3）：值不可知時送
  //    null，而且 patch 裡的 null 代表「這個 tag 消失了 / 現在不可知」。
  //    所以這裡把 null 原樣存起來而不是 delete —— 訂閱者要能分辨「掉回未知」
  //    和「從來沒出現過」。畫面上 null 就是 "---"，那是實話不是壞掉。
  //
  //  用法：
  //    HT9045Tags.connect();                       // 只看資料、不需要寫入權杖
  //    HT9045Tags.on('machine.state', function (v) { ... });
  //    HT9045Tags.subscribe(function (changed, all) { ... });
  //    HT9045Tags.get('clock.text');
  //    HT9045Tags.status();   // {connected, generation, frames, count, lastAt}
  var tagState = {};            // tag -> value（含 null）
  var tagSubs = [];             // 全域訂閱者 fn(changed, all)
  var tagOne = {};              // tag -> [fn, ...]
  var evtSubs = [];             // alarm / modal / query
  var tagStat = {connected: false, generation: 0, frames: 0, lastAt: 0, unknown: {},
                 // Steven 20260916：訊框層 metadata（見 .github/specs/web-bridge-json-contract.md）
                 // 這些欄位 producer 目前**還沒送**。這裡先實作消費端，等對方補上就直接生效，
                 // 不必再改客戶端；沒有 seq 的訊框一律照舊處理（不會因為缺欄位就拒收）。
                 seq: null, at: '', trigger: '', stale: false, gaps: 0};
  var RESYNC_SENT = false;

  // 缺一幀 patch 會讓畫面上某幾格永遠停在舊值，而且看起來完全正常 ——
  // 沒有紅字、沒有 "---"。這是會誤導操作員的那種壞法，所以偵測到就要停止信任畫面。
  function seqCheck(m) {
    if (typeof m.seq !== 'number') return true;        // producer 還沒送 seq -> 照舊
    if (m.type === 'snapshot') {                        // snapshot 無條件重設基準
      tagStat.seq = m.seq; tagStat.stale = false; RESYNC_SENT = false;
      return true;
    }
    if (tagStat.seq === null) { tagStat.seq = m.seq; return true; }
    if (m.seq === tagStat.seq + 1) { tagStat.seq = m.seq; return true; }
    // 跳號或倒退（倒退＝伺服器重啟）：丟棄這一幀並要求重送完整狀態。
    tagStat.gaps++;
    tagStat.stale = true;
    if (!RESYNC_SENT) {
      RESYNC_SENT = true;
      cmd('stream.resync').then(function () { /* 等 snapshot 到達時解除 stale */ },
          function () {
            // 伺服器還沒實作 stream.resync：stale 維持著，讓畫面說實話，
            // 不要假裝沒事繼續套用可能已經不連續的資料。
            if (global.console && console.warn)
              console.warn('[HT9045Tags] seq 跳號且伺服器沒有 stream.resync，資料標記為 stale');
          });
    }
    return false;
  }

  function tagNotify(changed) {
    var keys = Object.keys(changed);
    if (!keys.length) return;
    keys.forEach(function (k) {
      var fns = tagOne[k];
      if (!fns) return;
      fns.forEach(function (f) {
        try { f(changed[k], k); } catch (e) { /* 一個訂閱者壞掉不能拖垮其他人 */ }
      });
    });
    tagSubs.forEach(function (f) {
      try { f(changed, tagState); } catch (e) { /* 同上 */ }
    });
  }

  function tagFrame(m) {
    tagStat.frames++;
    tagStat.lastAt = Date.now();
    if (m.type === 'snapshot' || m.type === 'patch') {
      //Steven 20260916：訊框層 metadata 先收下來（producer 補上就生效）
      if (typeof m.at === 'string') tagStat.at = m.at;
      if (typeof m.trigger === 'string') tagStat.trigger = m.trigger;
      if (!seqCheck(m)) return;          // seq 跳號 -> 丟棄這一幀，已標記 stale 並要求 resync
      var data = m.data || {}, changed = {};
      if (m.type === 'snapshot') {
        // 整份取代：snapshot 之後本地表就是伺服器那一代的全部內容。
        // 先把不在新 snapshot 裡的舊 tag 標成消失，否則斷線重連後畫面會留著
        // 上一代的殘值，而那看起來完全像是活的。
        Object.keys(tagState).forEach(function (k) {
          if (!(k in data)) { changed[k] = null; tagState[k] = null; }
        });
      }
      Object.keys(data).forEach(function (k) {
        if (!(k in tagState) || tagState[k] !== data[k]) {
          tagState[k] = data[k];
          changed[k] = data[k];
        }
      });
      tagStat.connected = true;
      if (typeof m.generation === 'number') tagStat.generation = m.generation;
      tagNotify(changed);
      return;
    }
    if (m.type === 'alarm' || m.type === 'modal' || m.type === 'query') {
      evtSubs.forEach(function (f) { try { f(m); } catch (e) { /* 同上 */ } });
      return;
    }
    // 沒見過的訊框型別只記一次。靜默丟棄正是這次要修的毛病，不要再引入一個。
    if (!tagStat.unknown[m.type]) {
      tagStat.unknown[m.type] = true;
      if (global.console && console.warn) console.warn('[HT9045Tags] 未處理的訊框型別: ' + m.type);
    }
  }

  var tagsApi = {
    // 打開 socket 但不取寫入權杖：純看板頁面不該佔用單一操作員 token。
    connect: function () { return connect().then(function () { return true; }); },
    get: function (tag) { return (tag in tagState) ? tagState[tag] : null; },
    has: function (tag) { return tag in tagState; },
    all: function () {
      var o = {};
      Object.keys(tagState).forEach(function (k) { o[k] = tagState[k]; });
      return o;
    },
    // fn(changed, all)；回傳取消訂閱的函式
    subscribe: function (fn) {
      tagSubs.push(fn);
      // 已經有資料就立刻補送一次，訂閱者才不必自己處理「我來晚了」。
      if (Object.keys(tagState).length) {
        try { fn(tagsApi.all(), tagState); } catch (e) { /* 同上 */ }
      }
      return function () {
        var i = tagSubs.indexOf(fn);
        if (i >= 0) tagSubs.splice(i, 1);
      };
    },
    // fn(value, tag)
    on: function (tag, fn) {
      (tagOne[tag] = tagOne[tag] || []).push(fn);
      if (tag in tagState) { try { fn(tagState[tag], tag); } catch (e) { /* 同上 */ } }
      return function () {
        var a = tagOne[tag] || [], i = a.indexOf(fn);
        if (i >= 0) a.splice(i, 1);
      };
    },
    // alarm / modal / query 的原始訊框
    onEvent: function (fn) {
      evtSubs.push(fn);
      return function () {
        var i = evtSubs.indexOf(fn);
        if (i >= 0) evtSubs.splice(i, 1);
      };
    },
    status: function () {
      return {connected: tagStat.connected, generation: tagStat.generation,
              frames: tagStat.frames, lastAt: tagStat.lastAt,
              count: Object.keys(tagState).length,
              unknownFrameTypes: Object.keys(tagStat.unknown),
              // Steven 20260916：訊框層 metadata。seq 為 null 表示 producer 還沒送。
              // stale=true 表示偵測到漏幀，畫面上的值不可信任到下一次 snapshot 為止。
              seq: tagStat.seq, at: tagStat.at, trigger: tagStat.trigger,
              stale: tagStat.stale, gaps: tagStat.gaps};
    }
  };
  global.HT9045Tags = tagsApi;

  // --- public API ---------------------------------------------------------
  var api = {
    configure: function (o) {
      if (!o) return cfg;
      Object.keys(o).forEach(function (k) { cfg[k] = o[k]; });
      return cfg;
    },

    // {recipe, path, documents:[...]}
    list: function () { return getJson(httpBase() + '/api/recipe/'); },

    // {path, available, sections:{<sec>:{<key>:{value,type,raw,bcb}}}}   // AI(W906-RECIPE-BCB-R13) 20260926: bcb＝BCB6 讀到的字，只供顯示；寫入仍送 {raw}
    read: function (doc) { return getJson(httpBase() + '/api/recipe/' + encodeURIComponent(doc)); },

    // Steven 20260924 (S12)：C++ 跑 TfXxx::DoIniDataToForm() 之後的表單狀態。
    // {page, form, available, widgets:{<id>:{text|checked|itemIndex|caption|position|down|visible|enabled}}}
    // 還沒接 C++ 表單的頁面回 404（getJson 會 reject，訊息含 "HTTP 404"）。
    form: function (page) { return getJson(httpBase() + '/api/form/' + encodeURIComponent(page)); },

    // Steven 20260924：C 路（golden 表單橋）。struct = FileRW 的結構名（目前只有 "IniConfig"）。
    // editlistGet：golden 開頁 FormShow，只在開頁／重讀時呼叫一次，不要輪詢（會把 LastSet 回到
    // lastdata.dat 的值，golden 開頁亦同）。必須走 WS（主迴圈）；HTTP GET 刻意回 405。
    // 回應：{lists:{<清單>:{entries:[{id,group,key,...,text|checked|itemIndex}]}}, proxies:{id:{visible,enabled,…值}}, mustSend:[…]}
    editlistGet: function (struct) {
      return acquire().then(function () {
        return cmd('editlist.get', { tag: struct }).then(unwrapAck);
      });
    },
    // widgets = {id:{text|checked|itemIndex|position|dateTime|cells|tag}}；answers = {"<golden 英文題目>":1|2}
    // ack：{saved, applied, ignored:[…], kept:[…], unknown:[…], session:{messages, asked, todo, trace}}
    // extra（Steven 20260925）：頁面自己的資料併進 value（例 BinSelect 的 {bin, actions}）；widgets／answers 不可被蓋掉
    editlistSave: function (struct, widgets, answers, extra) {
      var v = {};
      if (extra) Object.keys(extra).forEach(function (k) { v[k] = extra[k]; });
      v.widgets = widgets; v.answers = answers || {};
      return takeover().then(function () {                 // AI(W906-TAKEOVER) 20260926：存檔＝操作員按的
        return cmd('editlist.save', { tag: struct, value: JSON.stringify(v) })
          .then(unwrapAck);
      });
    },

    // AI(W906-W4-MOTOR) 20260925：HW.MotorTest／HW.teach 的馬達按鈕 → C++（WebMotorAccess.h）。
    // req = motor-access.js 組好的 request；成功時 resolve 併好的 ack（{state, result, message, layer, ...}），
    // C++ 拒絕時 reject(Error(理由)) —— 還沒接上的動作也是拒絕，不回假成功。
    // AI(W906-MT-FIX1) 20260925：**線上順序＝呼叫順序**（審查 high：jog 的放開可能比 jog 先到 C++）。
    //   原本沒有權杖時要先等 control.acquire 的 ack 才送 motor.access，而 motorStop 是立刻送 ——
    //   手指一點就放開時，motor.stop 先上線、C++ 停一根沒在動的軸，接著才收到 jogP 開始 Acm_AxJog，
    //   而頁面已經 finish('aborted')、不再理那個 ack：軸一路 jog 到極限，狀態列還顯示綠色 stopped。
    //   現在 acquire 與 motor.access 背靠背送出（同一條 WS 依序處理；acquire 在 socket 執行緒當場回答，
    //   WebBridgeServer.cpp:1386-1396，所以 motor.access 到的時候權杖已經在了 —— 同 LAT-1 的 ht9045_io_do.js）。
    //   之後的 motorStop 一定排在 motor.access 後面上線；C++ 的輸出優先柵欄（wb_serve.cpp:5453-5457）
    //   保證停止不會超車已經到達的 jog。acquire 被拒（control-held）時，回報 acquire 的理由（那才是原因）。
    motorAccess: function (req) {
      var tag = (req && req.motors && req.motors[0]) || '';
      // AI(W906-TAKEOVER) 20260926：formShow／formClose 是視窗狀態的自動同步（HW.MotorTest.html syncCppForm）→ 照舊 acquire，不搶；
      //   其他動作都是操作員按的按鈕 → 每次都 takeover（背靠背送出的順序規則不變）。
      var auto = !!(req && (req.action === 'formShow' || req.action === 'formClose' || (req.params && req.params.query === true)));   //AI(W906-MERGE-0929) 20260929: review TK-2: Teach's page-load query (HW.teach.html:469/:576 teachSet {query:true}) is automatic too -> acquire, never take over
      var a = (auto && haveToken) ? null : cmd(auto ? 'control.acquire' : 'control.takeover').then(function () { haveToken = true; });
      var m = cmd('motor.access', { tag: tag, value: JSON.stringify(req) }).then(unwrapAck);
      if (!a) return m;
      a.catch(function () {});                       // 結果在下面合併，這裡只防 unhandled rejection
      // 兩個都有結果才回傳：haveToken 由 acquire 的回覆設定，先回傳的話下一條指令可能又送一次 acquire，
      //   或晚到的 acquire 回覆把 not-operator 剛清掉的 haveToken 又設回 true（測試架 harness5 量到）。
      return m.then(function (ack) { return a.then(function () { return ack; }, function () { return ack; }); },
        function (e) { return a.then(function () { throw e; }, function (ae) { throw ae; }); });
    },
    // AI(W906-MT-FIX1) 20260925：有工作在跑（Loop Move／HOME／Light Scale）而且 Motor Test 開著時，定期續一次權杖。
    //   權杖閒置 10 分鐘會被收回（WebBridgeServer.cpp:1566-1569），接著 MotorAccessTick 以「操作員連線消失」
    //   取消工作 —— golden 的 Loop Move 會一直跑到按停止。持有人再 acquire 一次只會重設閒置計時（:1388-1390）；
    //   別人持有時回 control-held，什麼都不改（呼叫端吞掉）。
    keepAlive: function () {
      return cmd('control.acquire').then(function () { haveToken = true; armTokenIdle(); });   // AI(W906-TOKEN-IDLE) 20260926: 續回來的權杖也要掛閒置計時，否則工作結束後永遠不還
    },
    // 停止刻意不 acquire：伺服器讓 motor.stop 免操作權杖（權杖可能在別的分頁，停機不能被擋），
    // 而 C++ 端把 motor.stop 鎖死在 action=="stop"。同 modalAnswer「警報一定要答得掉」的理由。
    motorStop: function (req) {
      return cmd('motor.stop', { tag: (req && req.motors && req.motors[0]) || '', value: JSON.stringify(req) })
        .then(unwrapAck);
    },

    // Steven 20260924 (S12 第二型)：走 golden 存檔鈕（例：TFTestIF::spbSaveClick）由 golden 原檔產生的 bridge。
    // widgets = {<id>: {text?, itemIndex?, checked?}}，要涵蓋 /api/form 回應的 saveReads 全部，
    // 少一個伺服器就整筆拒寫（golden SaveSetupFile 會把整頁寫回檔案）。
    // ack：{page, closed, messages:[{en,zh}], todo:[...]}
    formSave: function (page, widgets) {
      return takeover().then(function () {                 // AI(W906-TAKEOVER) 20260926：存檔＝操作員按的
        return cmd('form.save', { tag: page, value: JSON.stringify({ widgets: widgets }) });
      });
    },

    // edits: { '<section>': { '<key>': '<raw text>' } }
    //
    // The value you supply is written VERBATIM after the '='. It is not
    // reformatted, so pass the text exactly as the machine should store it --
    // '1.25', not 1.25. Numbers are accepted and stringified, but a float that
    // round-trips differently is your problem, not the server's.
    write: function (doc, edits, opts) {
      opts = opts || {};
      var sections = {};
      Object.keys(edits).forEach(function (sec) {
        sections[sec] = {};
        Object.keys(edits[sec]).forEach(function (key) {
          sections[sec][key] = {raw: String(edits[sec][key])};
        });
      });
      // AI(W906-TAKEOVER) 20260926：真的寫（存檔）＝操作員按的 → takeover；preview（dryRun）不寫檔 → 照舊 acquire，不搶
      return (opts.dryRun ? acquire() : takeover()).then(function () {
        // AI(W906-FW-DRYRUN) 20260915 FIX: dryRun MUST travel INSIDE `value`.
        // It used to be a sibling of `value` on the message, and the server
        // drops it there: WebCommand carries only {id, cmd, tag, value}
        // (WebBridge/CommandQueue.h:70) and the string "dryRun" appears ZERO
        // times in WebBridgeServer.cpp. wb_serve.cpp parses it out of the
        // PARSED `value` object, so the old shape meant preview() performed a
        // REAL WRITE -- silently, and before the operator's confirm dialog.
        return cmd('recipe.doc.put', {
          tag: doc,
          value: JSON.stringify({sections: sections, dryRun: !!opts.dryRun})
        });
      });
    },

    // Preview helper -- identical to write({dryRun:true}), named so call sites
    // read as the safe thing they are.
    preview: function (doc, edits) { return api.write(doc, edits, {dryRun: true}); },

    release: function () {
      if (!haveToken) return Promise.resolve();
      haveToken = false; if (tokenTimer) { clearTimeout(tokenTimer); tokenTimer = null; }   // AI(W906-TOKEN-IDLE) 20260926: 先在本地放掉，之後的指令會先重送 control.acquire（同一條 WS 依序處理，不會跟這個 release 搶）
      return cmd('control.release');
    },

    // -----------------------------------------------------------------------
    //  Steven 20260922：對話框與登入的低階指令
    // -----------------------------------------------------------------------
    //  這三個的參數形狀**不是猜的**，是 20260922 對活的 wb_serve
    //  （server\wb_serve.exe，0917 build，--dry --allow-cmd）逐一探測出來的。
    //  探測腳本與完整輸出見 CHANGES_20260922_Steven.md §20。
    //
    //  ⚠ 這個 build 的 dispatch 只有七個指令：
    //      sys.ping, sys.echoModal, sys.echoErrorModal,
    //      auth.login, auth.logout, counter.clear, modal.answer
    //    **沒有 start.run / lot.start / pause** —— Jimmy 那邊的 build 比較新，
    //    那三個等拿到新的 wb_serve.exe 再補，不要先寫上去假裝有。

    //  modal 掛著的時候，**其他每一個指令都會回 modal-pending**（實測連
    //  sys.ping 都不行）—— 佇列真的被擋住，只有 modal.answer 進得去。
    //  所以警報沒答完之前不要送別的指令，送了也只是白費往返。
    //
    //  qid 在 query 訊框裡是**數字**，但 modal.answer 的 tag 要**字串**。
    //    query 訊框  {"type":"query","qid":1,"code":"WAR0000","kcode":3,
    //                 "options":["RETRY","SKIP"],"at":"..."}
    //    答覆        cmd('modal.answer', {tag:"1", value:"RETRY"})
    //  option 必須是 query 訊框 options[] 裡逐字存在的字串，
    //  否則伺服器回 "not an offered option"。
    modalAnswer: function (qid, option) {
      return cmd('modal.answer', {tag: String(qid), value: String(option)});
    },

    //  auth.login：tag = 帳號、value = 密碼。**驗證一律由伺服器做**，
    //  HTML 只負責把使用者打的字送過去（與 Alert.Password.html 的分工一致）。
    //  帳密錯誤回 {ok:false, error:"bad credentials"}，那是正常回應不是例外。
    // AI(W906-MERGE-56bbf785) 20260926: the four auth.* calls no longer acquire() the single-operator token. The server exempts
    //   auth.* from the token (WebBridgeServer.cpp's "auth." exemption next to the not-operator check), so acquire() here only
    //   GRABBED the token -- main.html calls authMode() on load and never gave it back, locking the IO page / Motor Test / saves out
    //   for up to 10 minutes (review of the 56bbf785 merge). haveToken is left as it was.
    authLogin: function (user, password) {
      return cmd('auth.login', {tag: String(user), value: String(password)});
    },
    authLogout: function () { return cmd('auth.logout').then(unwrapAck); },

    // Steven 20260924：主畫面登入（golden TfMain::cbUserSelectChange／stOperatorClick／btLoginClick，WebLogin.cpp）
    // authMode → {mode:'book'|'select', level, itemIndex, levelName, items:[4], btLogin:'Login'|'Logout', systemStart}
    authMode: function () { return cmd('auth.mode').then(unwrapAck); },
    // 下拉選單：itemIndex 0..3；需要密碼時回 {needPassword:true,…}，帶 password 再送一次
    authSelect: function (itemIndex, password) {
      var extra = { tag: String(itemIndex) };
      if (password !== undefined && password !== null) extra.value = String(password);
      return cmd('auth.select', extra).then(unwrapAck);
    },

    //  給 ht9045_dialog_host.js 之類的掛鉤用 —— 需要送 dispatch 裡其他指令時，
    //  不必為了拿 cmd() 而去改這個檔。名字用 raw 是要提醒呼叫端：
    //  這裡沒有任何參數檢查，形狀錯了只會拿到伺服器的 error 字串。
    rawCmd: function (name, extra) { return cmd(name, extra); },

    status: function () {
      return {connected: !!(sock && sock.readyState === 1), holdsToken: haveToken};
    },

    // =======================================================================
    //  AI(W906-MERGE-20260923) 三方合併還原：以下五個方法在 a016aa0 的版本裡
    //  不存在，不是被誰刪掉，是 Steven 20260922 刻意沒寫 —— 他的原話就在上面
    //  那段註解裡：「**沒有 start.run / lot.start / pause** —— Jimmy 那邊的
    //  build 比較新，那三個等拿到新的 wb_serve.exe 再補，不要先寫上去假裝有」。
    //  他探測的是 0917 build，我們的 wb_serve 已經有這三個 dispatch，所以現在
    //  補回來，並保留他 20260922 新增的 modalAnswer / authLogin / authLogout。
    // =======================================================================

    start: function (who) {
      return takeover().then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的
        return cmd('start.run', {value: String(who || 'web')});
      });
    },

    // AI(W906-T3-PAUSE) 20260918: START 的鏡像。
    //   ack = {accepted, softStop, systemStart}。
    //   ⚠ systemStart 回來時**通常還是 true** —— ckernel 要到下一個 pump tick
    //   才把它清掉。要看機台真的停了，看 machine.state tag，不要看這個欄位。
    pause: function (who) {
      return takeover().then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的
        return cmd('pause.run', {value: String(who || 'web')});
      });
    },

    // AI(W906-Q27) 20260921: lot.start -- START 的前置，不是 START 本身。
    //
    //   為什麼需要：多數客戶組態下 `StartFromWeb` 的第一個檢查就是
    //   「LotID / Operator ID 是不是空的」（golden main.cpp:5212-5217）。
    //   那兩個欄位全樹**唯一的寫入點**是 wb_serve 的 `lot.start`
    //   （tools/wb_serve.cpp:3119），而 20260921 逐檔量過 D:\HT9045\web 的
    //   665 個檔，`lot.start` 出現 **0 次** —— 瀏覽器端根本沒有這一步。
    //
    //   訊框形狀照 wb_serve.cpp:3111-3114 的註解：`tag` 放 LotID、
    //   `value` 放 OperatorID，兩個都是 WebCommand 本來就有的欄位。
    //
    //   ⚠ 與 start/pause 一樣要先 acquire：`lot.start` 不在
    //   WebBridgeServer.cpp:1377-1379 的豁免名單裡（只有 auth.* 與
    //   ui.windows.put 豁免），沒有權杖會被回 `not-operator`。
    //
    //   ⚠ 會寫真實檔：`SetLotStart` -> `ReadWriteLotInfo(false)` 會寫
    //   `D:\HT9045\config\config.ini` 的 `[Lot Info]`。
    //
    //   ⛔ 這是它**不會**做的事：它不啟動機台。按完 Lot Start 還是要按 START。
    lotStart: function (lotId, operatorId) {
      return takeover().then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的
        return cmd('lot.start', {tag:   String(lotId || ''),
                                 value: String(operatorId || '')});
      });
    },

    // AI(W906-Q30-8) 20260922: 警報對話框的回應通道。
    //
    //   ⚠ 刻意做成**兩個窄方法**而不是一個通用的 `cmd(name, extra)`。
    //     Q27 當時就記過「這個 client 沒有通用 cmd」，那是刻意的 ——
    //     開一個通用逃生口等於讓任何頁面送任意指令給機台。
    //
    //   ⚠⚠ **不呼叫 acquire()**：警報一定要答得掉。
    //     操作員面前那一頁不見得握著單一操作權杖（權杖可能在辦公室那台
    //     改配方的分頁上），若這裡要求權杖，就會變成
    //     「框跳出來但按不了」= 使用者明講要避免的 hang up。
    //     C++ 端的等待迴圈只接受**與當前 qid 相符**的回應，
    //     所以這條路送不出別的東西。
    //
    //   ⚠ 20260923 合併後的未決事項：本方法（`dialog.response`）與上面
    //     Steven 的 `modalAnswer`（`modal.answer`）是**兩條並存的應答傳輸**，
    //     正是 INBOX Q30 第 8 題「甲/乙」still 未裁決的那兩條。
    //     目前實際有消費者的是 `modalAnswer`（ht9045_dialog_host.js）；
    //     `dialogResponse` 呼叫者 0。裁決前兩條都先留著，不要自行擇一刪除。
    dialogResponse: function (requestId, actionAndPressed) {
      return cmd('dialog.response', {tag:   String(requestId || ''),
                                     value: String(actionAndPressed || '')});
    },

    // 密碼：契約 dialogAuth.verifier 明文「C++ only，HTML never compares」。
    // 這裡只把使用者輸入原封轉送，不做任何判斷。
    dialogAuth: function (authId, payloadJson) {
      return cmd('dialog.auth', {tag:   String(authId || ''),
                                 value: String(payloadJson || '')});
    }
  };

  // ---------------------------------------------------------------------------
  //  AI(W906-TOKEN-IDLE) 20260926: 閒置自動還權杖。
  //  機台 0926 實測：Motor Test 拿到權杖後從來不 release（視窗按 X 只是把 iframe 藏起來，連線還在），
  //  之後 10 分鐘（WebBridgeServer.cpp PumpLiveness 的 controlIdleTimeoutMs）IO 頁按 Output 都被擋。
  //  使用者 0926：「MotorTest 優先，IO 畫面現在被卡權杖問題」。
  //    * 要權杖的指令做完後 tokenIdleMs（30 秒，比照機台 IO 頁的做法）沒有下一個，就送 control.release。
  //    * 還有指令在路上、或頁面登記的 hold 函式回 true（例：HOME／Loop Move 進行中 —— 停掉它們的
  //      start:false 要權杖）時不還，30 秒後再看。motor.stop／modal.answer 本來就免權杖，不靠這裡。
  //    * 伺服器回 not-operator 而本頁送出時以為自己拿著（伺服器 10 分鐘收回、或剛好同一刻還掉）：
  //      清掉 haveToken、重拿、重送一次。別頁拿著時重拿會回 control-held，照原樣報給呼叫端。
  //    * 伺服器的單一操作員設計（control.acquire）不動：同一時刻仍只有一條連線能寫。
  // ---------------------------------------------------------------------------
  function tokenHeld() {
    for (var i = 0; i < tokenHolds.length; i++) {
      try { if (tokenHolds[i]()) return true; } catch (e) { return true; }   // 判斷壞掉就保守地不還
    }
    return false;
  }
  function armTokenIdle() {
    if (tokenTimer) clearTimeout(tokenTimer);
    tokenTimer = setTimeout(function () {
      tokenTimer = null;
      if (!haveToken) return;
      if (tokenInflight > 0 || tokenHeld()) { armTokenIdle(); return; }
      api.release().catch(function () {});
    }, tokenIdleMs);
  }
  function tokenTrack(name, extra, p, retried) {
    if (name === 'control.acquire' || name === 'control.release') return p;
    var sentWithToken = haveToken;
    tokenInflight++;
    return p.then(function (v) {
      tokenInflight--; if (haveToken) armTokenIdle(); return v;
    }, function (e) {
      tokenInflight--;
      if (!retried && sentWithToken && e && e.message === 'not-operator' && name !== 'motor.access') {   // AI(W906-TOKEN-IDLE) 20260926: NB2 R68／R69 B2 —— 運動指令不自動重送：motor.stop 免權杖、會先排進佇列，重送的 jog 可能落在 stop 之後（放開後軸還在跑）。被拒時 :170 已把 haveToken 清掉，下一次按鍵會先重拿權杖
        haveToken = false;
        return acquire().then(function () { return tokenTrack(name, extra, cmd0(name, extra), true); });
      }
      if (haveToken) armTokenIdle();
      throw e;
    });
  }
  // 頁面登記「現在不能還權杖」的判斷（可以登記多個，任何一個回 true 就不還）。
  api.setTokenHold = function (fn) { if (typeof fn === 'function') tokenHolds.push(fn); };
  // 讀／改閒置秒數（毫秒）；測試與除錯用。
  api.tokenIdleMs = function (ms) { if (ms > 0) tokenIdleMs = ms; return tokenIdleMs; };

  global.HT9045Recipe = api;

  // ---------------------------------------------------------------------------
  //  HT9045System -- 五個機台設定檔（system\ 與 config\），wb_serve 20260915 新增
  // ---------------------------------------------------------------------------
  //  重用同一條 WebSocket 與同一個單一操作員 token，所以兩套 API 不會互搶。
  //
  //      list()                 -> {files:[{name,path,kind,available,bytes}]}
  //      read(name)             -> ini: {path,available,sections:{...}}（與配方同形）
  //                                csv: {path,kind:'csv',columns,keyColumn,rows}
  //      write(name, edits, o)  -> ini edits: {sec:{key:'raw'}}
  //                                csv edits: {rowKey:{column:'raw'}}
  //      preview(name, edits)   -> write(..., {dryRun:true})
  //
  //  ⚠ 寫入除了 --allow-cmd 還需要伺服器帶 --allow-system-write，否則回
  //    "system writes need --allow-system-write"。這是刻意的：system\ 是共用
  //    量產設定，不是配方資料。
  //
  //  ⚠ dryRun 一定要放在 value 裡面，不能當成訊息的兄弟欄位 —— WebCommand 只
  //    帶 {id,cmd,tag,value}，放外面會被靜默丟棄而變成真寫。見上面 write() 的
  //    AI(W906-FW-DRYRUN) 註解。
  var sysApi = {
    list: function () { return getJson(httpBase() + '/api/system/'); },

    read: function (name) {
      return getJson(httpBase() + '/api/system/' + encodeURIComponent(name));
    },

    // ini: edits = { '<section>': { '<key>': '<raw>' } }
    // csv: edits = { '<rowKey>': { '<column>': '<raw>' } }
    write: function (name, edits, opts) {
      opts = opts || {};
      var csv = !!opts.csv, body = {dryRun: !!opts.dryRun};
      var outer = {};
      Object.keys(edits).forEach(function (a) {
        outer[a] = {};
        Object.keys(edits[a]).forEach(function (b) {
          outer[a][b] = csv ? String(edits[a][b]) : {raw: String(edits[a][b])};
        });
      });
      if (csv) body.rows = outer; else body.sections = outer;
      return (opts.dryRun ? acquire() : takeover()).then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的  //AI(W906-MERGE-0929) 20260929: the machine rule (see editlist dryRun above) -- a dryRun preview writes nothing -> acquire, only a real write takes over
        return cmd('system.file.put', {tag: name, value: JSON.stringify(body)});
      });
    },

    preview: function (name, edits, opts) {
      opts = opts || {};
      return sysApi.write(name, edits, {dryRun: true, csv: !!opts.csv});
    },

    // Steven 20260916
    // csv 整列新增 / 刪除：system.csv.rows（wb_serve 20260916）。
    //   ops = { add: [ {<col>: '<raw>', ...}, ... ],  del: [ '<key>', ... ] }
    //   -> ack {ok, added, deleted, notFound}
    // 新增列的識別欄（motTable=Motorname、ioTable=Alias）必填且不可與現有列重複；
    // 刪除以識別欄定位，命中不是剛好一列就 notFound。與 write() 同一個 dryRun 契約、
    // 同一個 --allow-system-write 閘門、同樣先備份再原子置換。
    rows: function (name, ops, opts) {
      opts = opts || {}; ops = ops || {};
      var body = {dryRun: !!opts.dryRun, add: ops.add || [], 'delete': ops.del || ops['delete'] || []};
      return (opts.dryRun ? acquire() : takeover()).then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的  //AI(W906-MERGE-0929) 20260929: the machine rule (see editlist dryRun above) -- a dryRun preview writes nothing -> acquire, only a real write takes over
        return cmd('system.csv.rows', {tag: name, value: JSON.stringify(body)});
      });
    },

    //Steven 20260916 (W906-FW-LEVELSET)
    // 二進位定長陣列的個別元素寫入：system.levels.put（wb_serve 20260916）。
    //   edits = { <index>: <int>, ... }   稀疏，只有列出來的索引會被碰
    //   -> ack {ok, changed, identical, notFound, backup}
    //
    // 目前唯一的目標是 'levelset'（system\levelset.dat，256 個小端 int32，
    // 就是 Status.Security 那 179 組 radio 的 AccessLevel[]）。
    // read() 走同一支 /api/system/<name>，回 {kind:'i32', count, min, max, values:[...]}。
    //
    // ⚠ 值域外或索引越界一律整批拒寫（ok:false / changed:0），不會只寫進去
    //   一半，也不會把非法值夾回合法範圍。dryRun 同樣會回 ok:false —— 預覽
    //   就該說出正式送出時會被拒。
    levels: function (name, edits, opts) {
      opts = opts || {};
      var vals = {};
      Object.keys(edits || {}).forEach(function (k) { vals[String(k)] = Number(edits[k]); });
      var body = {dryRun: !!opts.dryRun, values: vals};
      return (opts.dryRun ? acquire() : takeover()).then(function () {   // AI(W906-TAKEOVER) 20260926：操作員按的  //AI(W906-MERGE-0929) 20260929: the machine rule (see editlist dryRun above) -- a dryRun preview writes nothing -> acquire, only a real write takes over
        return cmd('system.levels.put', {tag: name, value: JSON.stringify(body)});
      });
    }
  };

  global.HT9045System = sysApi;

  /* -------------------------------------------------------------------------
   * HT9045Text —— 機台產生的純文字記錄（唯讀）
   * -------------------------------------------------------------------------
   * Steven 20260918 (W906-FW-TEXT)
   *
   *   GET /api/text/                 -> {roots:[{name,path,kind,available}]}
   *   GET /api/text/<root>           kind==='file' -> {path,bytes,text}
   *                                  kind==='dir'  -> {path,root,subPath,entries,truncated}
   *   GET /api/text/<root>/<sub...>  同上，往下一層
   *
   * ⚠ **沒有寫入指令，這裡也刻意不提供 write()。**
   *   wb_serve 的指令表（11 個）裡沒有任何 text 寫入。這些是機台自己產生的
   *   記錄，不是人設定的東西 —— 契約上就該是唯讀的。
   *
   * ⚠ `available:false` 不是錯誤。六個 root 裡有兩個（jamCount、indexCycleTime）
   *   在這台開發機上不存在，上了機台才有。呼叫端要把它顯示成「此環境沒有」，
   *   不是「讀取失敗」，更不可以顯示成空的記錄 —— 空的記錄看起來像「機台很乾淨」。
   *
   * 路徑組法：每一段各自 encodeURIComponent，但**不編碼分隔的斜線**，
   * 因為伺服器是靠 '/' 切 root / subPath / 檔名的（SafeDocName 另外擋 '..'）。
   */
  var textApi = {
    roots: function () {
      return getJson(httpBase() + '/api/text/');
    },

    // path 可以是 'releaseNote'，也可以是 'eventLogTxt/2026/xxx.txt'
    read: function (path) {
      var seg = String(path || '').split('/').filter(function (s) { return s !== ''; });
      return getJson(httpBase() + '/api/text/' +
                     seg.map(encodeURIComponent).join('/'));
    }
  };

  global.HT9045Text = textApi;

  /* -------------------------------------------------------------------------
   * HT9045Windows —— 視窗狀態總表的推送通道（取代 golden 的 fShow）
   * -------------------------------------------------------------------------
   * Steven 20260918 (W906-FW-WINREG)
   * 契約：D:\HT9045\docs\web-client\WINDOW_REGISTRY_CONTRACT.md §4
   *
   * 走的是**既有的**指令通道（同一條 WebSocket、同一個 {id,cmd,tag,value} 訊框），
   * 不另開連線，也不需要 control.acquire —— 這是「回報事實」，不是寫機台的檔案，
   * 不該去搶單一操作員權杖（搶了會讓真正要存檔的那一頁寫不進去）。
   *
   * ⚠ **wb_serve 今天收不下 `ui.windows.put`，而且會有兩種不同的拒絕**
   *   （20260918 以手工 WS 客戶端實測）：
   *     1. `ok:false, error:"not-operator"`  <- **今天實際會拿到的是這一個**
   *        WebBridgeServer.cpp:1362 的閘門：除了 control.acquire/release 與 auth.*，
   *        **每一條**指令都要求呼叫端持有單一操作員權杖。
   *        ⚠ 但這裡刻意**不去 acquire** —— background.html 會一路持有權杖不放，
   *          真正要存檔的那一頁就再也拿不到了。總表是「回報」，不是「寫入」，
   *          本來就不該進到那個閘門裡。已寫進 CPP_REQUESTS_20260918.md §9。
   *     2. `ok:false, error:"unknown cmd (dispatch: ...)"`
   *        wb_serve.cpp:2744 的 dispatch else。指令表只有那幾條，ui.* 一條都沒有。
   *        等 Jimmy 把權杖那一關放行之後，會換成看到這一個。
   *
   *   兩種都是**預期中的**，不是故障。處理方式一樣：
   *     - 被拒一次 -> 記下 supported=false，之後**不再送**。不這樣做的話，
   *       每動一次視窗就會往伺服器丟一條必然被拒的指令，把 log 刷滿。
   *     - 斷線重連 -> supported 歸 null，重新試一次（伺服器可能已經升級了）。
   *   呼叫端拿到的是一個 reject，**必須自己吞掉**，不可以讓它變成畫面上的錯誤。
   *
   * ⚠ 這裡刻意**不做任何互鎖判斷**。總表只報事實，放行／擋住由 C++ 端決定
   *   （契約 §9）。瀏覽器算出一個 boolean 再送過去，等於把安全判斷搬到最容易
   *   被繞過的一層 —— WS 直連完全繞得過瀏覽器。
   */
  var windowsApi = {
    // null = 還沒問過 / 剛重連；true = 伺服器收得下；false = 被拒過
    supported: function () { return winSupported; },
    // 被拒時伺服器說的理由（'not-operator' / 'unknown cmd ...'）。診斷用。
    refusal: function () { return winRefusal; },

    // frame 就是契約 §4 的那個物件。回傳 Promise；被拒時 reject。
    put: function (frame) {
      if (winSupported === false) {
        return Promise.reject(new Error('ui.windows.put: refused earlier (' + winRefusal + ')'));
      }
      return cmd('ui.windows.put', {tag: 'registry', value: JSON.stringify(frame)})
        .then(function (r) { winSupported = true; winRefusal = ''; return r; },
              function (e) {
                var msg = (e && e.message) || '';
                // 逾時／斷線不算「伺服器不支援」—— 那是連線問題，重試是對的。
                // AI(W906-MT-FIX1) 20260925：只有「伺服器不認得這條指令」才算不支援、停送（審查 high）。
                //   原本任何明確拒絕都停送：告警框等回答時 wb_serve 對幾乎所有指令回 modal-pending
                //   （wb_serve.cpp:654），第一次心跳被拒就永遠不再送；15 秒後 WebWindowRegistry 把每張表單
                //   當成 stale＝開著（WebWindowRegistry.cpp:231-232），golden MainProc 在 Motor Test／Teach
                //   開著時暫停 ⇒ 生產停住。modal-pending、command queue full、not-operator 都是暫時的，照送。
                winRefusal = msg;
                if (/unknown cmd/i.test(msg)) winSupported = false;
                throw e;
              });
    }
  };

  global.HT9045Windows = windowsApi;
})(window);
