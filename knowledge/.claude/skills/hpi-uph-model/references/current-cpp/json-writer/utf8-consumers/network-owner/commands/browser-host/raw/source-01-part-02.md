# dialog host完整原文（1／2）

[上層](../index.md)／[manifest](../source-manifest.json)。固定pin `06c430c157a3e39c14d33a5b29219925b2879854`，來源 `web/page/ht9045_dialog_host.js`。
整檔原文380行含所有metadata、註解與IIFE／anonymous callbacks；本頁payload 180行，context credit0。



```javascript
<!-- preserved-content:start -->

      /* AI(W906-S17C) 20261003 (St01)：pendingQuery 只有一格（最近一個 query 訊框），不一定是**這個框**的。
       *   告警通道 requestId ＝ WS query 的 qid（tools/wb_serve.cpp qidStr，同一個十進位字串）⇒ 用它判斷 pendingQuery 是不是這個框的。
       *   不是 ⇒ 不拿它的 qid／options（以前會把答案送給別的告警、或被 pickOption 拒掉，框留著）。沒有 requestId ⇒ 照舊用 pendingQuery。 */
      var rqid = (response && response.requestId != null && response.requestId !== '') ? String(response.requestId) : '';
      var ownQuery = !!(pendingQuery && (!rqid || String(pendingQuery.qid) === rqid));

      /* AI(W906-J5-ACK) 20260930 St01（INBOX 119，Jerry J-5）：kCode==0 的通知型告警框（ForwardShowErrorMessage 的 kcode==0 那一支、
       *   golden ShowMotorErrorMessage 的每一則 note）背後沒有等待迴圈 ⇒ 沒有 pendingQuery、dialog.response 會被拒，
       *   以前這裡一律 reject ⇒ 框關不掉、重新整理又跳、只能重開 wb_serve。契約（tools/wb_serve.cpp「THE NOTICE CONTRACT」、
       *   檔尾 W906_NoticeAckCommand，筆電 INBOX 119）：送 {"cmd":"dialog.notifyAck","tag":"<requestId>"}；
       *   ok:true ⇒ C++ 已退役，關框；ok:false「no-pending-notice」⇒ 早就關了，照樣關框；其他 ok:false ⇒ 框留著，操作員可以再按。
       *   頁面分不出是不是通知（closePolicy／buttons 兩種都一樣）⇒ 只要是「確認」（ACKNOWLEDGE／code 0）而且沒有 pendingQuery 就送，
       *   由 C++ 核對這個 id 是不是 kCode==0 的通知（不是就回 not-a-notice、什麼都不做，跟以前一樣框留著）。 */
      /* AI(W906-S17C) 20261003 (St01)：pendingQuery 是**別的**告警的（qid≠requestId）也走這條 —— 以前那種情形會把確認送成
       *   modal.answer 給另一個 qid（答錯框）或被拒，通知框關不掉。 */
      if (!ownQuery && rqid && isAcknowledge(response) && typeof R.rawCmd === 'function') {
        var nid = String(response.requestId);
        var tryAck = function (again) {
          return R.rawCmd('dialog.notifyAck', { tag: nid }).then(function () { return 'notice-retired'; }, function (e) {
            var m = String((e && e.message) || '');
            if (/^no-pending-notice/.test(m)) return 'notice-already-closed';
            if (/^not-operator/.test(m) && !again && typeof R.keepAlive === 'function') {
              return R.keepAlive().then(function () { return tryAck(true); });
            }
            throw new Error('dialog.notifyAck 被拒：' + (m || '(沒有 error 欄位)') + '（框留著，可以再按一次）。requestId=' + nid);
          });
        };
        return tryAck(false);
      }

      if (!ownQuery && !rqid) {   // AI(W906-S17C) 20261003 (St01)：有 requestId 就照下面用它當 qid 答，C++ 等待迴圈自己比 tag、自己驗選項
        return Promise.reject(new Error(
          '沒有待答的 query —— 這個畫面的來源不是 wb_serve 的 modal ' +
          '（例如檔案信箱送來的，或本地 raise 的不停機告警）。file=' + file));
      }

      var want = optionOf(response);
      if (!want) {
        return Promise.reject(new Error('response 裡沒有 selectedAction，無法對應到選項。file=' + file));
      }

      var qid = ownQuery ? pendingQuery.qid : rqid;   // AI(W906-S17C) 20261003 (St01)：不是這個框的 pendingQuery 不用
      if (answered[qid]) {
        return Promise.reject(new Error('qid ' + qid + ' 已經答過了（重複送會回 no query pending）'));
      }

      var opt = ownQuery ? pickOption(want, pendingQuery.options) : want;   // AI(W906-S17C)：別人的 options 不拿來比，C++ 自己驗
      if (opt === null) {
        return Promise.reject(new Error(
          '選項 "' + want + '" 不在伺服器提供的 options ' +
          JSON.stringify(pendingQuery.options) + ' 裡（qid=' + qid + '，code=' +
          pendingQuery.code + '，kcode=' + pendingQuery.kcode + '）。' +
          '多半是 kCode 位元遮罩與畫面上的按鈕對不起來。'));
      }

      /* AI(W906-DLGOBJ) 20260923: 把 pressedButton 一起送出去。
       * wb_serve 的等待迴圈接受 "<ACTION>:<pressedButton>" 形式並自己拆冒號
       * （tools/wb_serve.cpp 的 modal.answer / dialog.response 分支）。
       *
       * 為什麼要帶：操作員的「選 RETRY 但按 PAUSE」與「選 RETRY 按 START」是
       * 兩件不同的事 —— 契約寫著 BtnStart -> fMain->Start(SoftStart)、
       * BtnPause -> BtnPauseClick(SoftStop)，而 selectedAction.code 分不出這兩者。
       *
       * ⚠ 誠實地講清楚現況：wb_serve 目前**只把 pressed 印進 log**
       * （tools/wb_serve.cpp 的 `pressed=%s`），還沒有人消費它。
       * 所以這一行不會改變今天的行為，它是把資訊保住 —— 不帶的話，等未來
       * 接上消費端時，這個資訊已經在瀏覽器端被丟掉了，而且沒有人會發現。
       */
      var pressed = response.pressedButton;
      var payload = (pressed ? (opt + ':' + String(pressed)) : opt);

      /* AI(W906-S17C) 20261003 (St01)：「no query pending」（含 C++ 新的 "no query pending:superseded-by=<id>"）＝ C++ 已經不在等這個 id
       *   （別處答了、IO 關了、被新的取代、或 wb_serve 重開過）⇒ 當成已關，讓 dialog-bridge 收框；是 pendingQuery 自己那一題就順便清掉。 */
      var gone = function (m) {
        if (!/no query pending/i.test(String(m || ''))) return false;
        if (pendingQuery && String(pendingQuery.qid) === String(qid)) pendingQuery = null;
        return true;
      };
      answered[qid] = true;
      return R.modalAnswer(qid, payload).then(function (ack) {
        if (ack && ack.ok === false) {
          delete answered[qid];                       // 沒成功就讓它可以再試
          if (gone(ack.error)) return 'alarm-already-closed';   // AI(W906-S17C)
          throw new Error('modal.answer 被拒：' + (ack.error || '(沒有 error 欄位)'));
        }
        if (pendingQuery && pendingQuery.qid === qid) pendingQuery = null;
        return 'modal-answered';
      }, function (e) {
        delete answered[qid];
        if (gone(e && e.message)) return 'alarm-already-closed';   // AI(W906-S17C)
        throw e;
      });
    } catch (e) {
      /* 同步例外在這裡就地轉成 rejected Promise —— 這正是檔頭那個坑的解法 */
      return Promise.reject(e);
    }
  }

  /* 訂閱 query 訊框。HT9045Tags.onEvent 收的是 alarm / modal / query 三種原始訊框。
   * 這裡只認 query（那是「等答案」的那一種）。 */
  function watch() {
    var T = global.HT9045Tags;
    if (!T || typeof T.onEvent !== 'function') {
      log('err', 'HT9045Tags.onEvent 不存在，無法追蹤 query 訊框 —— 警報將無法回答。');
      return;
    }
    T.onEvent(function (m) {
      if (!m || m.type !== 'query') return;
      pendingQuery = m;
      delete answered[m.qid];
      /* 只記錄，不自己叫畫面。要不要彈框、彈哪一頁是 dialog-bridge 的職責
       * （它有 AlarmNonStop 路由與權限閘）。這裡多做一次會變成兩個框。 */
    });
    /* onEvent 只在連線之後才有東西；connect() 是冪等的。
     * 失敗不要吵 —— 沒有伺服器的場合（純靜態預覽）本來就不該報錯。 */
    if (typeof T.connect === 'function') T.connect().catch(function () {});
  }

  /* AI(W906-D026) 20261001 St01（todo D-026）：告警框的密碼層 —— golden TfNote::DoPassword／DoUnlockPassword
   *   （V912 note.cpp:5277／:5431，由 Start :3599-3604、BtnPauseClick :3921-3926、KeyCode==0 :3724／:4092 呼叫）。
   *
   * dialog-bridge.js（不改它）在 request.auth.required 時開登入層（Alert.Password.html），操作員按 OK 後叫
   *   HTDialogHost.verifyAuth(verify)，verify＝Dialog-auth-verify 物件（target.requestId、pendingAction、credentials）。
   *   這裡把它原封轉給 C++（WS dialog.auth，tag＝authId、value＝JSON 字串；比對只在 C++，同主畫面 auth.login 只走本機 127.0.0.1）。
   *   C++ 的回覆（accepted／accessLevel／message；沒有密碼）照 bridge 要的形狀回去：accepted ⇒ bridge 送正常回答
   *   （上面 submitResponse 的 modal.answer／dialog.notifyAck），C++ 用掉 dialog.auth 留的一次性通行才關框；
   *   沒過 ⇒ 登入框留著、顯示 Label3／Label4（golden fPassword2），可以再輸入。
   * 取消（Q45-4＝A）：golden 的登入框按取消＝空白帳密＝錯（有密碼本時變 Operator）。bridge 取消時什麼都不送，
   *   所以這裡聽登入頁的 HT_DIALOG_AUTH_CANCEL，補送一筆 {"cancelled":true}（帶這一則與按的鍵，沒有密碼）。
   * 密碼：不 console、不存 storage、送出後把 verify.credentials.password 清成空字串；錯誤訊息不帶輸入內容。
   * 一定回 Promise、絕不同步 throw（檔頭那個坑）。 */
  var lastAlarmAction = null;       // 告警頁最後一次 HT_DIALOG_ACTION（requestId、action、pressedButton）
  var authPrompt = null;            // bridge 開登入層時（ht-dialog-auth prompt）記下 {authId, action}
  function verifyAuth(verify) {
    try {
      var R = global.HT9045Recipe;
      if (!R || typeof R.dialogAuth !== 'function') {
        return Promise.reject(new Error('HT9045Recipe.dialogAuth 不存在 —— ht9045_recipe_client.js 沒載到或太舊'));
      }
      if (!verify || !verify.authId || !verify.target || !verify.target.requestId) {
        return Promise.reject(new Error('Dialog-auth-verify 缺 authId／target.requestId'));
      }
      var payload = JSON.stringify(verify);
      if (verify.credentials) verify.credentials.password = '';          // bridge 之後不再讀它
      if (authPrompt && authPrompt.authId === verify.authId) authPrompt.submitted = true;
      return R.dialogAuth(verify.authId, payload).then(function (ack) {
        payload = null;
        var r = ack || {};
        if (typeof r.value === 'string') { try { var j = JSON.parse(r.value); if (j && typeof j === 'object') r = j; } catch (e) {} }
        if (r.accepted === true && authPrompt && authPrompt.authId === verify.authId) authPrompt = null;   // bridge 接著關登入層（不是取消）
        return { schemaVersion: '1.0.0', channel: 'dialog-auth', authId: verify.authId, state: 'completed',
                 accepted: r.accepted === true, accessLevel: (r.accessLevel == null ? null : r.accessLevel),
                 userId: null, message: r.message || null, stage: r.stage || null, reason: r.reason || null };
      }, function (e) {
        payload = null;
        throw new Error('dialog.auth 被拒：' + String((e && e.message) || '(沒有 error 欄位)'));
      });
    } catch (e) {
      return Promise.reject(e);
    }
  }
  function sendAuthCancel() {
    var p = authPrompt;
    authPrompt = null;
    if (!p || !p.action || !p.action.requestId) return;
    var R = global.HT9045Recipe;
    if (!R || typeof R.dialogAuth !== 'function') return;
    var msg = { schemaVersion: '1.0.0', channel: 'dialog-auth', seq: Date.now(), authId: p.authId, state: 'pending', cancelled: true,
                target: { channel: 'show-error-message', requestId: String(p.action.requestId) },
                pendingAction: { name: (p.action.action && p.action.action.name) || 'NONE',
                                 code: (p.action.action && p.action.action.code != null) ? Number(p.action.action.code) || 0 : null,
                                 pressedButton: p.action.pressedButton || null } };
    try { R.dialogAuth(p.authId, JSON.stringify(msg)).catch(function (e) { log('warn', 'dialog.auth（取消）被拒：' + ((e && e.message) || '')); }); }
    catch (e) { log('warn', 'dialog.auth（取消）送不出去'); }
  }
  global.addEventListener('message', function (e) {
    var m = e && e.data; if (!m || !m.type) return;
    if (m.type === 'HT_DIALOG_ACTION') {
      lastAlarmAction = (m.kind === 'alarm') ? { requestId: m.requestId, action: m.action || null, pressedButton: m.pressedButton || null } : null;

<!-- preserved-content:end -->
```
