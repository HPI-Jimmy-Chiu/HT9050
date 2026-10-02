/* ht9045_nonstop_alarm.js -- 不停機告警：對照表、路由、以及 web 端自行發起
 * ---------------------------------------------------------------------------
 * //Steven 20260922
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260922_Steven.md
 * ---------------------------------------------------------------------------
 * NonStop 的定義（使用者 20260922 裁定）：
 *   **NonStop 模式下，C++ 那邊不呼叫 StopAllMotor()。** 就這一件事。
 *
 *   會停  note.cpp:805-808   ShowErrorMessage() 內
 *                            if(Code!="WAR1635") StopAllMotor(); else StopAllMotor(false);
 *                            -- **兩條路都停機**。StopAllMotor(false) 只是跳過
 *                            Galil MTestY1 的 VS0;SP0（myGALILmotor.cpp:4713-4719），
 *                            其餘馬達照停。所以 alarm 不需要 requestedSideEffects。
 *                            （只有 WAR1635 例外，改走 StopAllMotor(false)）
 *   不停  mymessbox.cpp:303  if(!iUnLoaderCount){ ... StopAllMotor(); }
 *                            -> iUnLoaderCount != 0 就跳過，機台繼續跑
 *
 * ---------------------------------------------------------------------------
 * 兩條路，對應使用者說的「兩邊都要有機制」
 * ---------------------------------------------------------------------------
 * A. **html 端決定**（權限不足這一類 —— 屬於操作介面自己發生的）
 *    web 比對 levelset 就知道等級不夠，直接 raise()，**完全不呼叫 C++**。
 *    StopAllMotor() 連被呼叫的機會都沒有，所以機台是真的沒停。
 *    這一條今天就能用，不需要 Jimmy 改任何東西。
 *
 * B. **C++ 端主動**（[P32] Empty/Color Tray Pre Alarm 這一類）
 *    停不停已經由 C++ 決定完了（走不走 StopAllMotor）。web 只能**照實畫**。
 *    判斷依據依序是：
 *      1. requestedSideEffects.stopAllMotor === false
 *         **既有契約就有這一格**（web\JSON\Message-dialog-request.json），
 *         不需要新欄位 —— Jimmy 只要照實填。
 *      2. request.nonStop 旗標（備用覆寫；alarm 頻道沒有 requestedSideEffects
 *         那個區塊，要標不停機才需要它）
 *      3. AlarmNonStop.json 的 codes / prefixes 查表
 *      4. 都沒有 -> 走原本會停機的那一頁（保守側）
 *
 * ⚠ **把 code 加進對照表不會讓機台不停。** StopAllMotor() 在 request 送到
 *   瀏覽器之前就跑完了。加錯只會讓操作員看到一個寫著「機台未停機」的小視窗，
 *   而馬達其實已經停住 —— 那比沒有這個功能更糟。
 *   要讓某個 code 真的不停，得在 C++ 端不呼叫 StopAllMotor，兩件事一起做。
 */
(function (g) {
  'use strict';

  /* 對照表的位置。
   * ---------------------------------------------------------------------
   * ⚠ **量產啟動器走的是 `file:` 協定**
   *   （`HT9045_Release.cmd:16` set URL=file:///D:/HT9045/background.html?mode=release）
   *   而 Edge/Chromium 在 `file:` 下封鎖 XHR/fetch 讀本地檔。
   *   原本只用 XMLHttpRequest 抓 .json 的寫法在量產模式**100% 讀不到**，
   *   而且只會留一行 console.warn —— 功能等於沒開，卻看不出來。
   *
   * 所以照 dialog-bridge.js:22-43 `loadFresh()` 的既有做法，依協定分兩條路：
   *     http(s)  -> 直接抓 .json
   *     file:    -> 注入 <script src="...js"> 墊片，讀 window.__HT9045_DATA__
   * 兩個檔的內容必須一致；墊片由 scratchpad/gen_alarm_nonstop_shim.py 從 .json 產生。
   *
   * ⚠ 這裡**不**沿用 JSON/js/ 那個目錄，因為 web\JSON\ 裡混著已退場的
   *   A 路靜態快照；對照表放在 config\ 比較不會被誤認成那一類。
   *   代價是不能直接用 dialog-bridge 的 loadFresh()，要自己做一份。
   */
  var TABLE_NAME = 'AlarmNonStop';

  var cache = null, loading = null;

  /* 相對於**本檔自己的位置**解析，不要用 location.pathname ——
   * 這支模組在 background.html（web 根）與 page/*.html（子目錄）兩邊都會載入，
   * 用文件位置算會在其中一邊算錯。 */
  function baseDir() {
    var s = document.currentScript;
    if (!s) {
      var all = document.getElementsByTagName('script');
      for (var i = all.length - 1; i >= 0; i--) {
        if (/ht9045_nonstop_alarm\.js/.test(all[i].src || '')) { s = all[i]; break; }
      }
    }
    if (!s || !s.src) return '';                       // 退而求其次：相對於文件
    var src = s.src.replace(/\\/g, '/');
    var dir = src.substring(0, src.lastIndexOf('/') + 1);
    return dir.replace(/(^|\/)page\/$/, '$1');         // page/ -> web 根
  }
  var BASE = baseDir();

  function fetchJson(url) {
    return new Promise(function (resolve, reject) {
      var x = new XMLHttpRequest();
      x.open('GET', url + (url.indexOf('?') < 0 ? '?' : '&') + '_=' + Date.now(), true);
      x.onreadystatechange = function () {
        if (x.readyState !== 4) return;
        if (x.status < 200 || x.status >= 300) return reject(new Error('HTTP ' + x.status));
        try { resolve(JSON.parse(x.responseText)); }
        catch (e) { reject(new Error('bad JSON: ' + e.message)); }
      };
      x.onerror = function () { reject(new Error('network')); };
      x.send();
    });
  }

  /* file: 專用 —— 注入 <script> 墊片。同 dialog-bridge.js:30-42。 */
  function loadShim() {
    return new Promise(function (resolve, reject) {
      var store = g.__HT9045_DATA__ || (g.__HT9045_DATA__ = {});
      delete store[TABLE_NAME];
      var el = document.createElement('script');
      el.src = BASE + 'config/' + TABLE_NAME + '.js?_=' + Date.now();
      el.onload = function () {
        var v = store[TABLE_NAME];
        el.remove();
        v ? resolve(v) : reject(new Error('墊片載入了但 __HT9045_DATA__["'
                                          + TABLE_NAME + '"] 是空的'));
      };
      el.onerror = function () { el.remove(); reject(new Error('墊片載入失敗 ' + el.src)); };
      document.head.appendChild(el);
    });
  }

  /* 讀對照表。讀不到時回一張空表並**寫 console** ——
   * 靜默回空表等於「所有 alarm 都照舊」，看起來一切正常，其實功能沒生效。 */
  function table() {
    if (cache) return Promise.resolve(cache);
    if (loading) return loading;
    /* 協定決定走哪一條，和 dialog-bridge.js 一致。
     * 兩條都失敗才進 fallback —— 例如 http 下墊片比 .json 新的情況，
     * 先試 .json、失敗再試墊片，兩邊都在就以 .json 為準。 */
    var isFile = (location.protocol === 'file:');
    var first = isFile ? loadShim : function () { return fetchJson(BASE + 'config/' + TABLE_NAME + '.json'); };
    var second = isFile ? function () { return fetchJson(BASE + 'config/' + TABLE_NAME + '.json'); } : loadShim;
    loading = first().catch(function (e1) {
      return second().catch(function (e2) { throw new Error(e1.message + ' / ' + e2.message); });
    }).then(function (t) {
      cache = t; loading = null;
      return t;
    }, function (e) {
      loading = null;
      cache = { codes: {}, prefixes: [], pages: {}, __err: String(e && e.message || e) };
      console.warn('[NonStop] 讀不到對照表（protocol=' + location.protocol
                 + '、base=' + (BASE || '(相對)') + '、' + cache.__err + '）—— '
                 + '所有告警都會走原本會停機的頁面。**這不是「沒有不停機的告警」，'
                 + '是對照表沒載到**，功能等於沒開。');
      return cache;
    });
    return loading;
  }

  /* 查表：codes 精確比對優先，其次 prefixes 由長到短。 */
  function lookup(t, code) {
    if (!code) return null;
    code = String(code);
    if (t.codes && Object.prototype.hasOwnProperty.call(t.codes, code)) return t.codes[code];
    var pre = (t.prefixes || []).slice().sort(function (a, b) {
      return String(b.prefix || '').length - String(a.prefix || '').length;
    });
    for (var i = 0; i < pre.length; i++) {
      if (pre[i].prefix && code.indexOf(pre[i].prefix) === 0) return pre[i];
    }
    return null;
  }

  /* 給 dialog-bridge.js 用：決定這一則要用哪一個顯示 kind。
   * kind 進來是 'alarm' | 'message'，出去可能變成 'alarmNonStop' | 'messageNonStop'。 */
  /* =========================================================================
   *  neverNonStop —— 公司鐵律的否決閘（Steven 20260922 裁定）
   * =========================================================================
   *  以下狀態**一定要停機**：
   *      1. ESD alarm   2. 安全門沒關   3. 溫度異常   4. EMG 被按下去
   *
   *  停不停機是 C++ 主控，HTML 端不介入 —— 但**顯示**是 HTML 的責任，
   *  而把這四類畫成「機台未停機，仍在運轉」是會害死人的錯誤資訊。
   *  所以這一道是**最高優先**：命中就一律走會停機的那一頁，
   *  連 requestedSideEffects.stopAllMotor === false 都壓不過它。
   *
   *  清單與 golden 佐證：web\config\AlarmNonStop.json 的 neverNonStop 區段
   *  規範：.github\specs\machine-stop-policy.md
   * ========================================================================= */
  function collectNever(t) {
    var n = (t && t.neverNonStop) || {};
    var codes = [], prefixes = [];
    var groups = n.codes || {};
    Object.keys(groups).forEach(function (g) {
      var e = groups[g] || {};
      (e.list || []).forEach(function (c) { codes.push(String(c).toUpperCase()); });
      /* 巢狀的子群（安全門的 _hatchway / _plc）也要收，漏掉等於漏一整類 */
      Object.keys(e).forEach(function (k) {
        var sub = e[k];
        if (sub && typeof sub === 'object' && sub.list) {
          sub.list.forEach(function (c) { codes.push(String(c).toUpperCase()); });
        }
      });
    });
    (n.prefixes || []).forEach(function (q) {
      if (q && q.prefix) prefixes.push(String(q.prefix).toUpperCase());
    });
    return { codes: codes, prefixes: prefixes };
  }

  function neverHit(t, code) {
    if (!code) return null;
    var c = String(code).toUpperCase();
    var n = collectNever(t);
    if (n.codes.indexOf(c) >= 0) return 'code';
    for (var i = 0; i < n.prefixes.length; i++) {
      if (c.indexOf(n.prefixes[i]) === 0) return 'prefix:' + n.prefixes[i];
    }
    return null;
  }

  function route(kind, request) {
    return table().then(function (t) {
      var req = request || {};
      var info = null, why = '';
      var code = (req.arguments || {}).code;

      /* 0. 鐵律否決 —— 放在所有判準之前，任何欄位都壓不過它 */
      var never = neverHit(t, code);
      if (never) {
        var se0 = req.requestedSideEffects;
        if ((se0 && se0.stopAllMotor === false) || req.nonStop === true) {
          /* C++ 說沒停、但這是一定要停的類別 —— 兩邊認知不一致，
           * 這是契約缺陷，要被看見，不是默默照畫。 */
          console.error('[NonStop] **契約衝突**：code ' + code +
            ' 屬於一定要停機的類別（' + never + '，見 AlarmNonStop.json 的 neverNonStop），' +
            '但 request 宣稱不停機（stopAllMotor=' +
            (se0 ? se0.stopAllMotor : 'n/a') + ', nonStop=' + req.nonStop + '）。' +
            '已強制走會停機的頁面。請追這個契約缺陷，不要改 neverNonStop。');
        }
        return { kind: kind, info: null, why: 'neverNonStop(' + never + ')' };
      }

      /* 0.5 AI(W906-NONSTOP-SEM) 20261002：C++ 的 ShowErrorMessage（channel show-error-message）一律是會停機的頁。
       *    golden ShowErrorMessage 兩條分支都 StopAllMotor（note.cpp:795-801／:805-808），C++ 也照做
       *    （tools/wb_serve.cpp W906_AlarmStopLikeGolden）；那條請求不帶 requestedSideEffects（下面 1. 的註解），
       *    以前就掉到 3. 查表 —— WAR1676（cSecurity.cpp:592 權限不足）／WAR1681 在表上，小窗就寫「機台未停機，
       *    仍在運轉」，其實已經停了（不停機頁稽核 noFunc_A 第 1 項）。查表只留給網頁自己發起的（channel nonstop-local）。 */
      if (req.channel === 'show-error-message' && req.origin !== 'html') {
        return { kind: kind, info: null, why: 'ShowErrorMessage always stops (note.cpp:795-808)' };
      }

      /* 1. 既有契約就有的事實：requestedSideEffects.stopAllMotor
       *    （web\JSON\Message-dialog-request.json 一開始就有這個區塊）
       *    NonStop 的定義就是「C++ 沒呼叫 StopAllMotor」，所以這一格 false
       *    就是最權威的判準 —— 不需要任何新欄位，今天的契約就講得出來。
       *    ⚠ show-error-message 那一條**沒有**這個區塊，因為 ShowErrorMessage
       *      兩條分支都 StopAllMotor（note.cpp:805-808），停機不是可選項。 */
      var se = req.requestedSideEffects;
      if (se && se.stopAllMotor === false) {
        info = lookup(t, code) || {};
        why = 'stopAllMotor:false';
      } else if (se && se.stopAllMotor === true) {
        return { kind: kind, info: null, why: 'stopAllMotor:true' };
      } else if (req.nonStop === true) {     // 2. 選用的覆寫旗標（契約沒有時的後路）
        info = lookup(t, code) || {};
        why = 'nonStop flag';
      } else if (req.nonStop === false) {
        return { kind: kind, info: null, why: 'nonStop flag(false)' };
      } else {                                // 3. 查表
        info = lookup(t, code);
        why = info ? 'table' : 'default';
      }
      if (!info) return { kind: kind, info: null, why: why };
      return { kind: kind === 'message' ? 'messageNonStop' : 'alarmNonStop',
               info: info, why: why };
    });
  }

  /* -------------------------------------------------------------------------
   * A 路：web 端自行發起（不經過 C++）
   * -------------------------------------------------------------------------
   * 任何頁面都可以呼叫。頁面通常跑在 background.html 的 iframe 裡，
   * 所以是 postMessage 給頂層，由 dialog-bridge.js 開視窗。
   * 頂層自己呼叫時直接走 bridge。
   */
  function raise(opts) {
    opts = opts || {};
    var request = {
      channel: 'nonstop-local',
      requestId: 'ns-' + Date.now() + '-' + Math.random().toString(16).slice(2),
      seq: Date.now(),
      state: 'pending',
      origin: 'html',
      nonStop: true,                          // 這一條的意思：C++ 從頭到尾沒被呼叫
      arguments: { code: opts.code || '', kCode: 0, errorPart: opts.errorPart || ' ' },
      display: {
        message: opts.message || '',
        unitName: opts.unitName || 'System',
        description: opts.description || '',
        mainMessage: opts.message || '',
        chineseMessage: opts.messageZh || ''
      },
      nonStopInfo: {
        title: opts.title || '', titleZh: opts.titleZh || '', hint: opts.hint || ''
      }
    };
    var msg = { type: 'HT_NONSTOP_RAISE', kind: opts.kind === 'message' ? 'message' : 'alarm',
                request: request };
    if (g.parent !== g) g.parent.postMessage(msg, '*');
    else if (g.HTDialogBridge && g.HTDialogBridge.raiseNonStop) g.HTDialogBridge.raiseNonStop(msg);
    else console.warn('[NonStop] 找不到 dialog-bridge，raise() 沒有作用：', request);
    return request.requestId;
  }

  /* -------------------------------------------------------------------------
   * 權限檢查 —— golden fSecurity->Insufficient(iType) 的 web 版
   * -------------------------------------------------------------------------
   * golden（cSecurity.cpp:585..596）：
   *     if(AccessLevel < LevelSet.AccessLevel[iType]) {
   *         if(bAlarm) ShowErrorMessage("WAR1676", 0, MMSystem);   // <- 這一行會 StopAllMotor
   *         return false;
   *     }
   * 這裡把那一行換成 raise() —— 同樣擋下操作、同樣告訴操作員，但不進 C++、
   * 因此 StopAllMotor() 不會被呼叫。這就是使用者要的 to-be。
   *
   * ⚠ 這是**顯示層**的擋，不是安全閘。真正的權限判斷仍然在 C++ 端；
   *   瀏覽器這一層能被 F12 繞過，所以絕不可以拿它當唯一防線。
   *   它的價值是：在操作介面上就把不該按的擋下來，不要讓機台為此停下來。
   *
   * AccessLevel（目前登入者的等級）web 這邊還沒有來源 —— 見 currentLevel()。
   */
  var levelCache = null;

  function levelSet() {
    if (levelCache) return Promise.resolve(levelCache);
    if (typeof g.HT9045System === 'undefined') {
      console.warn('[NonStop] 沒有 HT9045System，讀不到 levelset。');
      return Promise.resolve(null);
    }
    return g.HT9045System.read('levelset').then(function (r) {
      levelCache = r; return r;
    }, function (e) {
      console.warn('[NonStop] 讀 levelset 失敗：' + (e && e.message) + ' —— 權限檢查一律放行。');
      return null;
    });
  }

  /* 目前登入者的等級。
   * ⚠ golden 的 AccessLevel 是 cmydef.h:3527 的執行期全域，登入後才有值，
   *   **不在任何 ini 裡，也還沒有對應的 tag**。web 這邊目前拿不到。
   *   在拿到之前一律回 null，而 requireLevel() 對 null 是**放行並警告** ——
   *   擋錯比放行更糟：這一層本來就不是安全閘，擋錯會讓操作員做不了事，
   *   而真正該擋的 C++ 端還是會擋。 */
  /* 等級語意（使用者 20260922 確認，與 golden 一致）：
   *     **0 是最低等級，數字越大權限越高。**
   *     cSecurity.cpp:589   if(AccessLevel < LevelSet.AccessLevel[iType]) 擋
   *     ContactForce.cpp:717 AccessLevel >= iDefHonPrecLevel
   *
   * 「預設權限」指的是 **需求等級**（levelset 裡那一格的值），不是操作者的等級：
   *     debug   預設需求 0  -> 任何等級都過得去，操作者全部能操作
   *     normal  預設需求 2  -> 第三級
   * 這個預設只在 levelset 讀不到、或該 iType 沒有對應格子時才用得上；
   * levelset 讀得到就以檔案為準。 */
  var DEFAULT_REQUIRED = { debug: 0, normal: 2 };

  function isDebug() {
    try { return document.documentElement.getAttribute('data-mode') === 'debug'; }
    catch (e) { return false; }
  }

  function defaultRequired() {
    return isDebug() ? DEFAULT_REQUIRED.debug : DEFAULT_REQUIRED.normal;
  }

  /* 操作者目前的等級。優先序：執行期 tag > 人工覆寫 > 不知道。
   *
   * ⚠ golden 的 AccessLevel 是 cmydef.h:3527 的執行期全域，登入後才有值，
   *   **不在 ini、沒有 tag、wb_serve 也沒有端點**。要請 Jimmy 推成
   *   security.accessLevel。在那之前一律回 null。 */
  function currentLevel() {
    if (typeof g.HT9045Tags !== 'undefined' && g.HT9045Tags.has('security.accessLevel')) {
      return Number(g.HT9045Tags.get('security.accessLevel'));
    }
    if (typeof g.HT9045AccessLevel === 'number') return g.HT9045AccessLevel;  // 人工覆寫，測試用
    return null;
  }

  function requireLevel(iType, opts) {
    opts = opts || {};
    return levelSet().then(function (ls) {
      var mine = currentLevel();

      /* 需求等級：以 levelset 檔為準；檔讀不到、或該 iType 沒有對應格子時，
       * 才退回模式預設（debug 0 / normal 2）。
       * levelset.dat 是 int AccessLevel[256] 的定長陣列，伺服器以 kind:"i32"
       * 供應；形狀可能是 {values:[...]} 或直接是陣列，兩種都認。 */
      var arr = (ls && ls.values) || (Array.isArray(ls) ? ls : null);
      var need = null;
      if (arr && iType >= 0 && iType < arr.length) need = Number(arr[iType]);
      if (need === null || isNaN(need)) {
        need = defaultRequired();
        console.info('[NonStop] levelset 沒有 iType=' + iType + ' 的值，'
                   + '用' + (isDebug() ? 'debug' : 'normal') + '預設需求等級 ' + need + '。');
      }

      /* 需求 0 = 誰都過得去（debug 的預設就是這個：操作者全部能操作）。
       * 這一條擺在「不知道操作者等級」之前 —— 需求是 0 的時候，
       * 操作者是誰根本不影響結果。 */
      if (need <= 0) return true;

      /* 操作者等級還沒有來源時放行並警告。擋錯比放行糟：
       * 這一層是顯示層不是安全閘，真正該擋的 C++ 端還是會擋，
       * 而擋錯會讓操作員做不了事又找不到原因。 */
      if (mine === null) {
        console.warn('[NonStop] 不知道操作者的 AccessLevel（需要 Jimmy 推 '
                   + 'security.accessLevel tag）—— iType=' + iType
                   + ' 需求 ' + need + '，本層放行。C++ 端仍會檢查。');
        return true;
      }

      if (mine >= need) return true;

      raise({
        code: 'WAR1676',
        title: 'Insufficient privileges',
        titleZh: '權限不足',
        message: opts.message || ('Insufficient privileges (need level ' + need + ', current ' + mine + ')'),
        unitName: 'System',
        description: opts.description ||
          ('golden: fSecurity->Insufficient(' + iType + ')\n'
           + '需要等級 ' + need + '，目前 ' + mine + '。\n'
           + '此告警不影響生產，機台未停機。'),
        hint: '請洽有權限的人員操作；機台仍在運轉。'
      });
      return false;
    });
  }

  g.HT9045NonStop = {
    table: table, route: route, lookup: lookup, raise: raise,
    requireLevel: requireLevel, currentLevel: currentLevel,
    reload: function () { cache = null; levelCache = null; return table(); }
  };
})(window);
