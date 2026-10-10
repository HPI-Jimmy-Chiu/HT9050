# Recipe Client完整原文（1／4）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `2f2872da7fce4b7ae6eb6685885de626885ccd92`，來源 `web/page/ht9045_recipe_client.js`。
整檔909行含全部metadata、原註解、API objects及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
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
      return {connected: !!(sock && sock.readyState === 1), holdsToken: haveToken,
              live: !!(sock && sock.readyState === 1 && sock.linkState !== 'reconnecting')};   // AI(W906-HMICLOSE) 20261006 (NB2-2, Jimmy 1006 10:5x「網頁如果已經發生斷線，應該要能夠關閉的」): connected stays OPEN while the frame's hub reconnects (ht9045_link.js AI(W906-J11-RECONNECT-A) 20261005 keeps readyState OPEN, the real state is linkState) -- live = really connected now, for "is wb_serve still there" questions (background.html HT9045ShellCloseRequest, ht9045_main_close.js)
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


<!-- preserved-content:end -->
```
