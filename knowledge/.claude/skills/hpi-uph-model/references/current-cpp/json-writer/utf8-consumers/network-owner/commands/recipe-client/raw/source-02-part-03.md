# Recipe Client完整原文（2／3）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`。
整檔688行含全部metadata、原註解、API objects及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
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

    // {path, available, sections:{<sec>:{<key>:{value,type,raw}}}}
    read: function (doc) { return getJson(httpBase() + '/api/recipe/' + encodeURIComponent(doc)); },

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
      return acquire().then(function () {
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
      return cmd('control.release').then(function () { haveToken = false; });
    },

    status: function () {
      return {connected: !!(sock && sock.readyState === 1), holdsToken: haveToken};
    },

    start: function (who) {
      return acquire().then(function () {
        return cmd('start.run', {value: String(who || 'web')});
      });
    },

    // AI(W906-T3-PAUSE) 20260918: START 的鏡像。
    //   ack = {accepted, softStop, systemStart}。
    //   ⚠ systemStart 回來時**通常還是 true** —— ckernel 要到下一個 pump tick
    //   才把它清掉。要看機台真的停了，看 machine.state tag，不要看這個欄位。
    pause: function (who) {
      return acquire().then(function () {
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
      return acquire().then(function () {
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

<!-- preserved-content:end -->
```
