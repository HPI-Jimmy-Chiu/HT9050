# Dialog bridge完整原文（2／5）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`，來源 `web/page/dialog-bridge.js`。
全文835行保留metadata、原裁決註解、API object及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
    var el = document.querySelector('#dialogBridge .dbWindow[data-kind="' + kind + '"] .dbStatus');
    if (!el) return;
    el.textContent = text || '';
    el.className = 'dbStatus' + (text ? ' show' : '');
  }

  function sendRequest(kind) {
    var f = frames[kind];
    if (!f || !f.ready) return;                        // 等 HT_DIALOG_READY 再送
    /* Steven 20260922：比對的是 displayKind 不是 channel kind ——
     * 不停機的那兩頁是同一個 channel 的另一種畫法，channel kind 必須保持
     * 'alarm'/'message'，否則 responseFor() 與 channels[] 都會查不到。
     *
     * ⚠ 20260922 實機驗證抓到的缺陷：原本這裡**只看 active**。
     *   不停機的那一層用的是 activeNS（刻意不共用，見檔頭），
     *   所以 renderNonStop() 末尾那次 sendRequest() 一定在這行被擋掉，
     *   造成兩個症狀：
     *     (1) 不停機的視窗是**空白的** —— 頁面沒收到 HT_DIALOG_REQUEST
     *     (2) 滑鼠**關不掉**它 —— 頁內 current 永遠是 null，
     *         acknowledge() 開頭就 return，HT_DIALOG_ACTION 根本沒送出
     *   而「不停機允許用滑鼠點畫面按鈕解除」正是它和停機版唯一的差別。
     *   兩組 displayKind 互斥（alarm/message vs alarmNonStop/messageNonStop），
     *   所以這裡先比 activeNS 再比 active 不會有歧義。 */
    var st = (activeNS && activeNS.displayKind === kind) ? activeNS
           : (active && active.displayKind === kind) ? active
           : null;
    if (!st) return;
    try { f.el.contentWindow.postMessage({ type: 'HT_DIALOG_REQUEST', kind: kind, request: st.request }, '*'); } catch (e) {}
  }

  function onPageMessage(e) {
    var m = e.data; if (!m || !m.type) return;
    var kind = m.kind;
    // Steven 20260922：web 端自行發起的不停機告警（HT9045NonStop.raise）。
    // 來源是桌面上任何一個 iframe，不是 dialog frame，所以要在下面那道
    // 來源過濾之前處理。
    if (m.type === 'HT_NONSTOP_RAISE') { raiseNonStop(m); return; }
    if (!frames[kind] || e.source !== frames[kind].el.contentWindow) return;
    if (kind === 'auth') return onAuthPageMessage(m);
    if (m.type === 'HT_DIALOG_READY') { frames[kind].ready = true; sendRequest(kind); return; }
    /* 不停機的窗：自己的狀態、自己的關閉路徑，不經過 active / complete()。
     * 這是**滑鼠**那條路；實體 IO 按鈕那條在 inspectClose()。兩條都要能關掉它。
     * ⚠ **刻意不做 needsAuth()**。停機告警的解除可以要密碼
     *   （golden note.cpp:5261 TfNote::DoPassword、mymessbox.cpp:458 DoPassword_MBox，
     *    契約的 request.auth.required 就是它），但不停機的不該要 ——
     *   其中一種就是「權限不足」本身，再要一次權限等於把人鎖在門外。
     *   若日後真有需要密碼才能關的不停機告警，那它就不該是不停機的。 */
    if (isNonStop(kind)) {
      if (!activeNS || activeNS.displayKind !== kind ||
          m.requestId !== (activeNS.request || {}).requestId) return;
      if (m.type === 'HT_DIALOG_ACTION') completeNonStop();
      return;
    }
    if (!active || active.displayKind !== kind || m.requestId !== active.request.requestId) return;
    if (m.type === 'HT_DIALOG_ACTION') {
      var a = m.action || {};
      var definition = { name: a.name, label: a.name, code: a.code, pressedButton: m.pressedButton || null,
                         closedBy: a.name === 'ALARM_RESET' ? 'alarm-reset' : undefined };
      if (needsAuth()) openAuth(definition); else complete(definition);
    } else if (m.type === 'HT_DIALOG_EVENT') {
      global.dispatchEvent(new CustomEvent('ht-dialog-event', { detail: { kind: kind, requestId: m.requestId, event: m.event } }));
    }
  }

  function render(kind, request, displayKind) {
    displayKind = displayKind || kind;
    active = { kind: kind, displayKind: displayKind, request: request,
               openedAt: Date.now(), submitting: false, authVerified: null };
    var view = document.getElementById('dialogBridge');
    view.className = 'open ' + displayKind;
    document.body.classList.add('dialogOpen');
    view.querySelectorAll('.dbWindow').forEach(function (w) { w.classList.toggle('show', w.dataset.kind === displayKind); });
    status(displayKind, '');
    sendRequest(displayKind);
  }

  /* Steven 20260922：查 AlarmNonStop.json 決定畫哪一頁，再 render。
   * 查表失敗一律退回原本會停機的頁 —— 保守側。把該停的畫成不該停的，
   * 比沒有這個功能更糟。 */
  function resolveDisplay(kind, request) {
    if (!global.HT9045NonStop || !global.HT9045NonStop.route) {
      return Promise.resolve({ kind: kind, info: null, why: 'no-router' });
    }
    return global.HT9045NonStop.route(kind, request).then(function (r) {
      var dk = (r && r.kind) || kind;
      if (!frames[dk]) dk = kind;                       // 頁沒部署 -> 退回會停機的那一頁
      return { kind: dk, info: r && r.info, why: r && r.why };
    }, function () { return { kind: kind, info: null, why: 'router-failed' }; });
  }

  function isNonStop(dk) { return dk === 'alarmNonStop' || dk === 'messageNonStop'; }

  /* 不停機的顯示／關閉 —— 完全不碰 active、不碰 #dialogBridge。
   * 停機那條路是機台真的在用的，這裡一個字都不動它。 */
  function renderNonStop(kind, request, dk) {
    activeNS = { kind: kind, displayKind: dk, request: request, openedAt: Date.now() };
    var v = document.getElementById('dialogNonStop');
    v.classList.add('open');
    v.querySelectorAll('.dnWindow').forEach(function (w) {
      w.classList.toggle('show', w.dataset.kind === dk);
    });
    sendRequest(dk);
  }

  function closeNonStop() {
    activeNS = null;
    var v = document.getElementById('dialogNonStop');
    v.classList.remove('open');
    v.querySelectorAll('.dnWindow').forEach(function (w) { w.classList.remove('show'); });
  }

  /* 按下螢幕上那顆確認鍵。
   * 本地發起的（channel 'nonstop-local'）沒有對應的 C++ request，只關窗；
   * C++ 發來的依契約 blocking:false，C++ 不等回應，response 只是畫面關閉的紀錄。 */
  function completeNonStop() {
    if (!activeNS) return;
    var req = activeNS.request || {};
    var ch = channels[activeNS.kind];
    var local = (req.channel === 'nonstop-local');
    if (!local && ch) {
      submit({
        schemaVersion: '1.0.0', channel: req.channel, seq: Date.now(),
        requestId: req.requestId, requestSeq: req.seq,
        state: 'completed', accepted: true, closedBy: 'action-button',
        completedAt: new Date().toISOString(),
        durationMs: Date.now() - activeNS.openedAt, error: null,
        selectedAction: 'ACKNOWLEDGE', nonStop: true
      }, ch.response).catch(function (e) {
        // 不停機的回應送不出去不該卡住畫面 —— C++ 本來就沒在等
        console.warn('[NonStop] 回應沒送出（C++ 不等待，畫面照關）：' + e.message);
      });
    }
    closeNonStop();
  }

  /* =========================================================================
   *  兩條獨立佇列（Steven 20260922 裁定）
   * =========================================================================
   *  使用者的裁定：**停機告警的「顯示」本身就是安全要件** ——
   *  它關係到人身安全與機台重大風險（ESD、安全門、溫度異常、EMG）。
   *  所以會停機的那條佇列**永不丟棄**，這是**刻意偏離 golden** 的地方。
   *
   *  golden 的做法是丟掉（note.cpp:824-828）：
   *      if(fNote->fShow)
   *      {
   *          MyDBIProcess("Exception", "Alarm at same time: "+Code, errPart);
   *          return 0;                         // <- 第二則直接消失，不排隊
   *      }
   *  在 golden 那是可接受的，因為那個 return 發生在 StopAllMotor()（:805-808）
   *  **之後** —— 機台已經停了，沒顯示只是少一筆資訊。
   *  但在 web 版，C++ 停機與瀏覽器顯示是兩個行程、兩條通道，
   *  「沒顯示」可能是操作員唯一會察覺的訊號，所以不能丟。
   *
   *  多個 alarm 會不會連續出現？**會**，golden 為此專門寫了上面那道保護。
   *  另外 TfNote::ScanKey() 在 fNote **顯示中**（Timer1Timer 開頭 if(!fShow) return）
   *  還會呼叫 ShowErrorMessage（note.cpp:3069 / :3077）—— 真正的巢狀。
   *  全樹 ShowErrorMessage 呼叫點 1950 個。
   *
   *  ⚠ 舊做法為什麼不夠：
   *    原本是「`if (active) return;` 且**不推進 lastSeq**，下一輪再讀檔案」。
   *    那不是佇列，是重讀 —— 它依賴 C++ 把 request 檔維持在 pending。
   *    C++ 只要在這期間把同一個單槽檔覆寫成新的一則，
   *    **前一則就永久消失，而且沒有任何痕跡**。
   *  ⇒ 現在改成：讀到就**立刻推進 lastSeq 並入列**，之後只從佇列出貨。
   * ========================================================================= */
  var queueStop = [];          // 會停機：FIFO，**永不丟棄**
  var queueNS = [];            // 不停機：FIFO，滿了丟**最舊**的（保留最新，符合 golden 的覆蓋語意）
  var NS_QUEUE_MAX = 8;
  var STOP_DEPTH_WARN = 3;     // 超過就在 console 出聲 —— 正常情況不該堆起來
  var seenIds = [];            // 去重用，只留最近 64 筆
  var SEEN_MAX = 64;

  function alreadySeen(id) {
    if (!id) return false;                       // 沒有 requestId 就不去重，寧可重複也不要漏
    if (seenIds.indexOf(id) >= 0) return true;
    seenIds.push(id);
    if (seenIds.length > SEEN_MAX) seenIds.shift();
    return false;
  }


<!-- preserved-content:end -->
```
