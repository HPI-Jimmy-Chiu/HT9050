# Dialog bridge完整原文（3／5）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`，來源 `web/page/dialog-bridge.js`。
全文835行保留metadata、原裁決註解、API object及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->
  function enqueue(kind, request, displayKind) {
    if (alreadySeen(request && request.requestId)) return false;
    var item = { kind: kind, displayKind: displayKind, request: request, at: Date.now() };
    if (isNonStop(displayKind)) {
      queueNS.push(item);
      while (queueNS.length > NS_QUEUE_MAX) {
        var dropped = queueNS.shift();
        console.warn('[Dialog] 不停機佇列滿了（上限 ' + NS_QUEUE_MAX + '），丟棄最舊的一則：'
                     + ((dropped.request.arguments || {}).code || dropped.request.requestId || '?'));
      }
    } else {
      queueStop.push(item);
      /* 永不丟棄，但要出聲 —— 堆起來代表有人沒把前一則關掉，
       * 或 C++ 在阻塞等回應的同時又送了新的，兩者都該被看見。 */
      if (queueStop.length >= STOP_DEPTH_WARN) {
        console.error('[Dialog] **停機告警佇列深度 ' + queueStop.length + '** —— '
                      + '這些都不會被丟棄，但堆積本身是異常。目前排隊中：'
                      + queueStop.map(function (q) {
                          return (q.request.arguments || {}).code || q.request.requestId || '?';
                        }).join(', '));
      }
    }
    return true;
  }

  /* 出貨。放在輪詢迴圈裡跑，不掛在每一個關閉點上 ——
   * 關閉點有五處（complete 兩處、inspectClose、completeNonStop、raiseNonStop），
   * 漏掉任何一處都會讓佇列靜默卡住。輪詢延遲 100ms，換到的是漏不掉。 */
  function drain() {
    if (!active && queueStop.length) {
      var a = queueStop.shift();
      render(a.kind, a.request, a.displayKind);
    }
    if (!activeNS && queueNS.length) {
      var b = queueNS.shift();
      renderNonStop(b.kind, b.request, b.displayKind);
    }
  }

  function routeAndRender(kind, request) {
    resolveDisplay(kind, request).then(function (r) {
      if (r.info) request.nonStopInfo = Object.assign({}, r.info, request.nonStopInfo || {});
      if (isNonStop(r.kind)) renderNonStop(kind, request, r.kind);
      else render(kind, request, r.kind);
    });
  }

  /* web 端自行發起（不經過 C++）。StopAllMotor() 連被呼叫的機會都沒有，
   * 所以這一條是唯一真的能保證「機台沒停」的路徑。 */
  function raiseNonStop(msg) {
    /* ⚠ 這裡**刻意沒有** `if (active) return` ——
     * 初版有，而那是個退步：機台停著、停機告警開著的時候，權限不足的提示會被
     * 直接丟掉（只留一行 console.warn），而那正是它最該出現的時候。 */
    var kind = (msg && msg.kind === 'message') ? 'message' : 'alarm';
    var dk = NONSTOP_OF[kind];
    if (!frames[dk]) {
      console.warn('[NonStop] ' + dk + ' 的頁面沒有部署，raise 無法顯示：', msg && msg.request);
      return;
    }
    /* 同時只顯示一個不停機的窗，但**排隊**而不是覆蓋。
     * golden 的 ShowUnloaderTrayMessage 是先 MyMessageBox->Close() 再 Show()
     * （等於覆蓋），這裡刻意做得寬一點：佇列上限 8，滿了才丟最舊的，
     * 所以「最新的一定看得到」這個 golden 語意保留，中間的也不會全丟。 */
    enqueue(kind, msg.request || {}, dk);
    drain();
  }

  function closeView() {
    cancelAuth();
    var view = document.getElementById('dialogBridge');
    view.className = '';
    document.body.classList.remove('dialogOpen');
    view.querySelectorAll('.dbWindow').forEach(function (w) { w.classList.remove('show'); });
  }

  // ---- 權限／密碼：BCB6 fNote::DoPassword()/DoUnlockPassword()/PanSpecialNoteClick、MyMessageBox::DoPassword_MBox()
  //      HTML 只收集 User ID/Password 並寫 Dialog-auth-verify；C++ 比對 LevelSet/密碼檔後寫 Dialog-auth-result；通過才送 normal response 與關畫面 ----
  function needsAuth() {
    var a = active && active.request.auth;
    return !!(a && a.required && !active.authVerified);
  }
  function authMsg(text) {
    var el = document.querySelector('#dialogAuth .daMsg');
    el.textContent = text || ''; el.className = 'daMsg' + (text ? ' show' : '');
  }
  function postAuthPage(msg) {
    var f = frames.auth;
    if (f && f.ready) { try { f.el.contentWindow.postMessage(msg, '*'); } catch (e) {} }
  }
  function sendAuthRequest(message) {
    if (!auth || !active) return;
    postAuthPage({ type: 'HT_DIALOG_AUTH_REQUEST', auth: active.request.auth || {}, message: message || null });
  }

  function onAuthPageMessage(m) {
    if (m.type === 'HT_DIALOG_READY') { frames.auth.ready = true; if (auth) sendAuthRequest(); return; }
    if (!auth) return;
    if (m.type === 'HT_DIALOG_INPUT') {                 // 鍵盤必須在頂層彈出（609×305 的登入頁容不下 HTQwerty）
      if (!global.HTQwerty) return;
      var proxy = document.createElement('input'); proxy.value = m.current || '';
      global.HTQwerty.show(proxy, m.flags || 0, {
        onCommit: function (v) { postAuthPage({ type: 'HT_DIALOG_INPUT_RESULT', field: m.field, value: v, commit: true }); }
      });
    } else if (m.type === 'HT_DIALOG_AUTH_SUBMIT') {
      submitAuth(m.userId, m.password);
    } else if (m.type === 'HT_DIALOG_AUTH_CANCEL') {
      cancelAuth();                                       // spbCancel：只關登入層，Alert 保持開啟，不送 JSON
    }
  }

  function openAuth(definition) {
    var a = active.request.auth || {};
    auth = { authId: 'auth-' + Date.now() + '-' + Math.random().toString(16).slice(2), definition: definition, timer: null, submitting: false };
    document.querySelector('#dialogAuth .daBar').textContent = a.title || 'Password';
    authMsg('');
    document.getElementById('dialogAuth').className = 'open';
    sendAuthRequest();
    global.dispatchEvent(new CustomEvent('ht-dialog-auth', { detail: { state: 'prompt', authId: auth.authId } }));
  }

  function cancelAuth() {
    if (!auth) return;
    if (auth.timer) clearTimeout(auth.timer);
    auth = null;
    document.getElementById('dialogAuth').className = '';
  }

  function submitAuth(userId, password) {
    if (!auth || !active || auth.submitting) return;
    var a = active.request.auth || {};
    var verify = {
      schemaVersion: '1.0.0', channel: 'dialog-auth', seq: Date.now(), authId: auth.authId, state: 'pending',
      requestedAt: new Date().toISOString(),
      target: { channel: active.request.channel, requestId: active.request.requestId, requestSeq: active.request.seq },
      kind: a.kind || 'access-level', level: a.level == null ? null : a.level,
      pendingAction: { name: auth.definition.name, code: auth.definition.code == null ? null : Number(auth.definition.code) || 0,
                       pressedButton: auth.definition.pressedButton || null },
      credentials: { userId: userId || null, password: password || '' }
    };
    auth.submitting = true;
    authMsg('Verifying...');
    var p = (global.HTDialogHost && typeof global.HTDialogHost.verifyAuth === 'function')
      ? Promise.resolve(global.HTDialogHost.verifyAuth(verify))
      : submit(verify, authChannel.verify).then(function () { return waitAuthResult(verify.authId); });
    p.then(function (result) {
      if (!auth || auth.authId !== verify.authId) return;
      if (result && result.accepted) {
        active.authVerified = { authId: verify.authId, accessLevel: result.accessLevel == null ? null : result.accessLevel, userId: result.userId || verify.credentials.userId };
        var def = auth.definition;
        cancelAuth();
        complete(def);
      } else {
        // cpp：密碼錯誤/權限不足 → fPassword Label3/Label4，Alert 保持開啟可重試
        auth.submitting = false;
        authMsg('');
        postAuthPage({ type: 'HT_DIALOG_AUTH_RESULT', accepted: false, message: (result && result.message) || null });
      }
    }).catch(function (err) {
      if (!auth) return;
      auth.submitting = false;
      authMsg(err.message);
    });
  }

  function waitAuthResult(authId, timeoutMs) {
    var deadline = Date.now() + (timeoutMs || 15000);
    return new Promise(function (resolve, reject) {
      (function poll() {
        if (!auth || auth.authId !== authId) return reject(new Error('auth cancelled'));
        loadFresh(authChannel.result).then(function (r) {
          if (r && r.authId === authId && r.state !== 'pending' && r.state !== 'idle') return resolve(r);
          if (Date.now() > deadline) return reject(new Error('C++ auth result timeout'));
          auth.timer = setTimeout(poll, 200);
        }).catch(function () {
          if (Date.now() > deadline) return reject(new Error('C++ auth result timeout'));
          auth.timer = setTimeout(poll, 200);
        });
      })();
    });
  }

<!-- preserved-content:end -->
```
