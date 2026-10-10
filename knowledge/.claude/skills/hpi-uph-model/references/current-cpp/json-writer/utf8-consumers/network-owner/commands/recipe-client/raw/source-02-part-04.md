# Recipe Client完整原文（2／4）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `HT9011UC_Cpp_V3.33.906.0/tools/websync/ht9045_recipe_client.js`。
整檔688行含全部metadata、原註解、API objects及callbacks；本頁payload 148行，context credit0。



```javascript
<!-- preserved-content:start -->
      });
      if (csv) body.rows = outer; else body.sections = outer;
      return acquire().then(function () {
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
      return acquire().then(function () {
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
      return acquire().then(function () {
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
                // 只有伺服器**明確回了一個拒絕**才停送。
                if (!/no ack within|socket closed/i.test(msg)) {
                  winSupported = false;
                  winRefusal = msg;
                }
                throw e;
              });
    }
  };

  global.HT9045Windows = windowsApi;
})(window);

<!-- preserved-content:end -->
```
