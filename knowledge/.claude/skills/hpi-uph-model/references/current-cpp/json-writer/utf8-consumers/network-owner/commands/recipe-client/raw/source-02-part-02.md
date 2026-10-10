# Recipe Client完整原文（2／2）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`。
整檔688行含全部metadata、原註解、API objects及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
    });
    return opening;
  }

  function cmd(name, extra) {
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

  function acquire() {
    if (haveToken) return Promise.resolve();
    return cmd('control.acquire').then(function () { haveToken = true; });
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

<!-- preserved-content:end -->
```
