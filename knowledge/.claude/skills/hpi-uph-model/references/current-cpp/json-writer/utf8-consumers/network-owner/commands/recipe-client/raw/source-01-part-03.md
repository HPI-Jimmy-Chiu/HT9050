# Recipe Client完整原文（1／3）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `web/page/ht9045_recipe_client.js`。
整檔909行含全部metadata、原註解、API objects及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
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

<!-- preserved-content:end -->
```
