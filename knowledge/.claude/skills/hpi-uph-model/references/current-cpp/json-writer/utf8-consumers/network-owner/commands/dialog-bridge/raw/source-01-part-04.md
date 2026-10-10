# Dialog bridge完整原文（4／5）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `f527888567ba11e5b6a2324323d750bf78de58e9`，來源 `web/page/dialog-bridge.js`。
全文835行保留metadata、原裁決註解、API object及callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->

  function responseFor(definition, closeRequest) {
    var request = active.request;
    var common = {
      schemaVersion: '1.0.0', channel: request.channel, seq: Date.now(), requestId: request.requestId,
      requestSeq: request.seq, state: 'completed', accepted: true,
      closedBy: closeRequest ? (closeRequest.closeReason || 'external-io') : (definition.closedBy || 'action-button'),
      completedAt: new Date().toISOString(), durationMs: Date.now() - active.openedAt, error: null
    };
    var reqAuth = active.request.auth || {};
    common.auth = { required: !!reqAuth.required, verified: !!active.authVerified,
                    authId: active.authVerified ? active.authVerified.authId : null,
                    accessLevel: active.authVerified ? active.authVerified.accessLevel : null,
                    bypassedByCloseRequest: !!(closeRequest && reqAuth.required && !active.authVerified) };
    if (active.kind === 'alarm') {
      common.selectedAction = { name: definition.name, code: Number(definition.code) || 0 };
      // fNote::Start()（BtnStart）與 BtnPauseClick 回同一 ReturnCode，但 C++ 需分 Start/Pause 後續動作
      common.pressedButton = closeRequest ? null : (definition.pressedButton || null);
    } else {
      common.selectedAction = definition.name;
      common.sideEffects = {
        handlerPaused: null, motorsStopped: null, servoOffInArmXY: null,
        secsGemEventReported: null, formCloseCleanupCompleted: null
      };
    }
    return common;
  }

  function closeResponseFor(response, closeRequest, wasOpen, error) {
    var target = closeRequest && closeRequest.target || {};
    return {
      schemaVersion: '1.0.0', channel: 'dialog-close', seq: Date.now(),
      closeEventId: 'close-' + Date.now() + '-' + Math.random().toString(16).slice(2),
      closeRequestId: closeRequest ? closeRequest.closeRequestId : null,
      closeRequestSeq: closeRequest ? closeRequest.seq : null,
      state: error ? 'error' : 'completed', accepted: !error,
      target: {
        channel: response ? response.channel : (target.channel || ''),
        requestId: response ? response.requestId : (target.requestId || ''),
        requestSeq: response ? response.requestSeq : (Number(target.requestSeq) || 0)
      },
      trigger: {
        source: closeRequest && closeRequest.trigger ? closeRequest.trigger.source : 'html-action',
        inputName: closeRequest && closeRequest.trigger ? (closeRequest.trigger.inputName || null) : null
      },
      selectedAction: closeRequest && closeRequest.resolvedAction
        ? closeRequest.resolvedAction
        : (response ? response.selectedAction : { name: 'NONE', code: null }),
      closedBy: response ? response.closedBy : (closeRequest && closeRequest.closeReason || null),
      dialogWasOpen: !!wasOpen, closedAt: error ? null : new Date().toISOString(),
      normalResponse: { file: response ? channels[active && active.kind || 'alarm'].response + '.json' : '', seq: response ? response.seq : 0 },
      error: error || null
    };
  }

  function submit(response, responseName) {
    /* //Steven 20260924：第 1 段（HTDialogHost）**不通時要往下掉**，不能直接 return。
     * -----------------------------------------------------------------------
     * ht9045_dialog_host.js:143 在「沒有待答的 query」時回 Promise.reject
     * （'這個畫面的來源不是 wb_serve 的 modal ...'）。檔案信箱送來的告警
     * 每一則都會踩到這條 —— 原本寫成 `return Promise.resolve(f(...))`，
     * 等於把那個 reject 直接當成 submit() 的結果，後面三段傳輸與 debug 本機關閉
     * **一次都沒機會跑**。現場症狀就是：選了 K_SKIP、按了 K_PAUSE，
     * 跳出「沒有待答的 query」然後框還在。
     *
     * ⚠ 同步 throw 也要接（見 ht9045_dialog_host.js 檔頭那個坑：例外炸穿
     *   submit() 會讓 active.submitting 卡在 true，對話框永久鎖死）。 */
    if (global.HTDialogHost && typeof global.HTDialogHost.submitResponse === 'function') {
      var first;
      try {
        first = Promise.resolve(global.HTDialogHost.submitResponse(responseName + '.json', response));
      } catch (e) {
        first = Promise.reject(e);
      }
      return first.catch(function (err) {
        if (global.console) console.warn(
          '[dialog-bridge] HTDialogHost 這條不通，改走下一段傳輸：%s',
          (err && err.message) || err);
        return submitRest(response, responseName);
      });
    }
    return submitRest(response, responseName);
  }

  function submitRest(response, responseName) {
    if (global.HTJsonWriter && global.HTJsonWriter.ready()) return global.HTJsonWriter.write(responseName, response);
    if (global.chrome && global.chrome.webview && typeof global.chrome.webview.postMessage === 'function') {
      global.chrome.webview.postMessage({ type: 'HT_DIALOG_RESPONSE', file: responseName + '.json', response: response });
      return Promise.resolve('webview-posted');
    }
    global.postMessage({ type: 'HT_DIALOG_RESPONSE', file: responseName + '.json', response: response }, '*');
    /* //Steven 20260924（使用者指定）：debug 模式的**本機**解除。
     * -----------------------------------------------------------------------
     * file: 底下沒有 wb_serve、沒有 WebView2、沒有 HTJsonWriter，上面四段傳輸
     * 全部踩空 -> 框永遠關不掉，連 golden 的主路徑
     * 「K_SKIP / K_RETRY 選鍵 + START / PAUSE 確認」都沒辦法驗。
     *
     * ⚠ 兩道閘同時成立才放行，缺一不可：
     *     mode=debug                 —— 明示是除錯
     *     location.protocol==='file:' —— 實機是用 http 連 wb_serve，永遠不是 file:
     *   只靠 mode=debug 不夠：有人在實機上帶了 ?mode=debug，框會「看起來解除了」
     *   而 C++ 完全沒收到回應 —— 機台還停著，操作員以為好了。那是會出事的。
     * 回應不丟掉，留在 __HT_DEBUG_RESPONSES__ 供檢查。 */
    if (isFileDebug()) {
      (global.__HT_DEBUG_RESPONSES__ = global.__HT_DEBUG_RESPONSES__ || [])
        .push({ at: new Date().toISOString(), file: responseName + '.json', response: response });
      if (global.console) console.warn(
        '[dialog-bridge] DEBUG 本機關閉：%s 沒有送到 C++（file: + mode=debug）。' +
        '回應留在 window.__HT_DEBUG_RESPONSES__', responseName);
      return Promise.resolve('debug-local');
    }
    return Promise.reject(new Error('C++ dialog response transport is not connected'));
  }

  function isFileDebug() {
    return location.protocol === 'file:' &&
           /[?&]mode=debug/.test(location.search);
  }

  function submitCloseResponse(value) {
    /* AI(W906-S17D) 20261003 (St01)：以前這裡直接 reject（回報就丟了）。recent[] 一輪關兩個以上時，第二份一定撞上第一份 ⇒ 改成排隊。 */
    if (closeResponseSubmitting) {
      if (closeResponseBacklog.length < 16) closeResponseBacklog.push(value);
      return Promise.resolve('close-response-queued');
    }
    pendingCloseResponse = value;
    closeResponseSubmitting = true;
    return submit(value, closeChannel.response).then(function () {
      pendingCloseResponse = null;
      closeResponseSubmitting = false;
    }).catch(function (error) {
      closeResponseSubmitting = false;
      throw error;
    });
  }

  function complete(definition, closeRequest) {
    if (!active || active.submitting) return;
    var mask = Number(active.request.arguments && active.request.arguments.kCode) || 0;
    var selectedCode = Number(definition.code) || 0;
    if (!closeRequest && active.kind === 'alarm' && ((mask === 0 && selectedCode !== 0) ||
      (mask !== 0 && (selectedCode === 0 || (mask & selectedCode) === 0)))) return;
    active.submitting = true;
    var kind = active.kind;
    var dk = active.displayKind || kind;          // 狀態列要貼在實際顯示的那個視窗上

    /* Steven 20260922：web 端自行發起的不停機告警（channel 'nonstop-local'）
     * 沒有對應的 C++ request，**不可以**寫 response 檔 ——
     * 那會讓 C++ 收到一份它從來沒有發問過的回覆。按確認就只是關掉。 */
    if (active.request && active.request.channel === 'nonstop-local') {
      closeView();
      active = null;
      return;
    }

    status(dk, 'Submitting response...');
    var response = responseFor(definition, closeRequest);
    submit(response, channels[kind].response).then(function () {
      global.dispatchEvent(new CustomEvent('ht-dialog-response', { detail: response }));
      closeView();
      var closeResponse = closeResponseFor(response, closeRequest, true, null);
      closeResponse.normalResponse.file = channels[kind].response + '.json';
      active = null;
      return submitCloseResponse(closeResponse);
    }).catch(function (error) {
      if (!active) return;
      active.submitting = false;
      status(dk, error.message);
    });
  }

  function rejectClose(closeRequest, message) {
    return submitCloseResponse(closeResponseFor(null, closeRequest, !!active, {
      code: 'CLOSE_REQUEST_REJECTED', message: message
    })).catch(function () {});
  }

  /* AI(W906-S17D) 20261003 (St01)：Dialog-close-request 是單槽檔，100 ms 內 C++ 連關兩個框時舊版只看得到最後一個；
   *   而且只認 active／activeNS，還在佇列裡（沒顯示）的框收到關閉會被拒，之後照樣跳出來、C++ 早就不等了 ⇒ 關不掉。
   *   契約 1.3.1：檔頂層照舊是最新一筆，另加 recent[]（最多 8 筆、舊到新、含最新）；沒有 recent 的舊檔當成 [request]。 */

<!-- preserved-content:end -->
```
