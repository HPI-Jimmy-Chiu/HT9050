# Dialog bridge完整原文（5／5）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`，來源 `web/page/dialog-bridge.js`。
全文835行保留metadata、原裁決註解、API object及callbacks；本頁payload 115行，context credit0。



```javascript
<!-- preserved-content:start -->
  function sameTarget(target, req) {
    return !!(target && req) && target.channel === req.channel && target.requestId === req.requestId &&
           Number(target.requestSeq) === Number(req.seq);
  }
  function dropQueued(target) {        // 排隊中（還沒顯示）的框：直接拿掉，永遠不畫
    var lists = [queueStop, queueNS];
    for (var l = 0; l < lists.length; l++) {
      for (var i = 0; i < lists[l].length; i++) {
        if (sameTarget(target, lists[l][i].request)) return lists[l].splice(i, 1)[0];
      }
    }
    return null;
  }

  function closeOne(request) {
      var target = request.target || {};
      /* Steven 20260922：實體 IO 按鈕**四種都能解除**（使用者說明），
       * 所以這條路也要認得不停機那一層，不能只看 active。
       * 先比對不停機的那一個；沒中再走原本的停機路徑。 */
      if (activeNS && activeNS.request && sameTarget(target, activeNS.request)) {
        closeNonStop();
        return submitCloseResponse(closeResponseFor(null, request, true, null)).catch(function () {});
      }
      if (active && sameTarget(target, active.request)) {
        var action = request.resolvedAction || {};
        if (!action.name || action.name === 'NONE') return rejectClose(request, 'C++ did not provide a resolved action');
        complete({ name: action.name, label: action.name, code: action.code }, request);
        return;
      }
      // AI(W906-S17D)：佇列裡的拿掉是同步的，不等 complete()（它是非同步的）
      if (dropQueued(target)) return submitCloseResponse(closeResponseFor(null, request, false, null)).catch(function () {});   // never shown: dialogWasOpen false
      if (!active) return rejectClose(request, 'No dialog is open');
      return rejectClose(request, 'Target does not match the active dialog');
  }

  function inspectClose() {
    if (closeChannel.loading) return;
    closeChannel.loading = true;
    loadFresh(closeChannel.request).then(function (request) {
      closeChannel.loading = false;
      var seq = Number(request && request.seq) || 0;
      if (!request || request.state !== 'pending' || !request.closeRequestId || seq <= closeChannel.lastSeq) return;
      // AI(W906-S17D)：recent[] 裡 seq > lastSeq 的每一筆依序處理；lastSeq 逐筆推進（中途出錯也不會重做已處理的）
      var recent = (request.recent && request.recent.length) ? request.recent.slice() : [request];
      if (!recent.some(function (e) { return e && Number(e.seq) === seq; })) recent.push(request);
      recent.sort(function (a, b) { return (Number(a && a.seq) || 0) - (Number(b && b.seq) || 0); });
      recent.forEach(function (entry) {
        var s = Number(entry && entry.seq) || 0;
        if (!entry || !entry.closeRequestId || s <= closeChannel.lastSeq) return;
        closeChannel.lastSeq = s;
        try { closeOne(entry); } catch (e) { console.warn('[dialog-bridge] close ' + entry.closeRequestId + ' 失敗：' + ((e && e.message) || e)); }
      });
    }).catch(function () { closeChannel.loading = false; });
  }

  function inspect(kind) {
    var channel = channels[kind];
    if (channel.loading) return;
    channel.loading = true;
    loadFresh(channel.request).then(function (request) {
      channel.loading = false;
      var seq = Number(request && request.seq) || 0;
      if (request && request.state === 'pending' && request.recent && request.recent.length > 1 && !channel.routing) { inspectRecent(kind, channel, request); return; }   /* AI(W906-W178) 20261008 (St02-E): contract 1.3.2 */  if (request && request.state === 'pending' && request.requestId && seq > channel.lastSeq) {
        /* Steven 20260922：先解析要開哪一頁，再決定擋不擋。
         * 舊版是 `if (!active)` 一律擋 —— 那會讓不停機的訊息排在停機告警後面，
         * 而不停機的本意就是「不要等」。停機類維持一次一個（golden 的 modal 語意）。 */
        if (channel.routing) return;
        channel.routing = true;
        resolveDisplay(kind, request).then(function (r) {
          channel.routing = false;
          if (r.info) request.nonStopInfo = Object.assign({}, r.info, request.nonStopInfo || {});
          /* ⚠ lastSeq 一定要在這裡推進，**不可以**因為畫面忙就跳過。
           * 舊版跳過它，等下一輪再讀同一個檔 —— 但那個檔只有一格，
           * C++ 覆寫它的瞬間舊的那一則就永久消失了。
           * 入列之後由 drain() 負責出貨，畫面忙不忙與收不收件無關。 */
          channel.lastSeq = seq;
          enqueue(kind, request, r.kind);
          drain();
        }, function () { channel.routing = false; });
      }
    }).catch(function () { channel.loading = false; });
  }
  function inspectRecent(kind, channel, request) { /* AI(W906-W178) 20261008 (St02-E): contract 1.3.2 -- Alarm-dialog-request.recent[] (2+ outstanding, e.g. the W-175 low-yield notices of one One Cycle finish): every entry newer than lastSeq, oldest first, through the same resolveDisplay -> lastSeq -> enqueue -> drain as inspect(); channel.routing is held for the whole run so the poll does not interleave; enqueue's requestId de-dup skips what was already queued */ var list = request.recent.slice(); if (!list.some(function (e) { return e && Number(e.seq) === Number(request.seq); })) list.push(request); list = list.filter(function (e) { return e && e.state === 'pending' && e.requestId && Number(e.seq) > channel.lastSeq; }).sort(function (a, b) { return Number(a.seq) - Number(b.seq); }); if (!list.length) return; channel.routing = true; (function next(i) { if (i >= list.length) { channel.routing = false; return; } var e = list[i]; resolveDisplay(kind, e).then(function (r) { if (r.info) e.nonStopInfo = Object.assign({}, r.info, e.nonStopInfo || {}); channel.lastSeq = Math.max(channel.lastSeq, Number(e.seq)); enqueue(kind, e, r.kind); drain(); next(i + 1); }, function () { channel.routing = false; }); })(0); }
  installView();
  setInterval(function () {
    if (pendingCloseResponse && !closeResponseSubmitting) submitCloseResponse(pendingCloseResponse).catch(function () {});
    else if (!pendingCloseResponse && !closeResponseSubmitting && closeResponseBacklog.length) submitCloseResponse(closeResponseBacklog.shift()).catch(function () {});   // AI(W906-S17D) 20261003 (St01)
    inspectClose();
    drain();                     // 前一則關掉之後把佇列裡的下一則叫出來
    inspect('alarm');
    inspect('message');
  }, POLL_MS);
  global.HTDialogBridge = {
    /* 版本標記：`file:` 下 Edge 會快取 .js，改了沒生效時第一個要確認的就是它。
     * console 打 HTDialogBridge.build 對不上就是吃到舊快取 -> Ctrl+F5。 */
    build: '20261003-s17-close-recent',   // AI(W906-S17D) 20261003 (St01)：was '20260924b-dialoghost-falls-through'
    inspect: inspect, inspectClose: inspectClose, render: render,
    routeAndRender: routeAndRender, raiseNonStop: raiseNonStop,
    active: function () { return active; }, auth: function () { return auth; },
    activeNS: function () { return activeNS; },
    /* 佇列可觀測性。現場若懷疑「有 alarm 沒跳出來」，先看這裡的深度。 */
    queues: function () {
      function brief(q) {
        return q.map(function (i) {
          return { code: (i.request.arguments || {}).code || null,
                   requestId: i.request.requestId || null,
                   displayKind: i.displayKind, waitedMs: Date.now() - i.at };
        });
      }
      return { stop: brief(queueStop), nonStop: brief(queueNS),
               stopDepth: queueStop.length, nonStopDepth: queueNS.length };
    },
    enqueue: enqueue, drain: drain
  };
})(window);
<!-- preserved-content:end -->
```
