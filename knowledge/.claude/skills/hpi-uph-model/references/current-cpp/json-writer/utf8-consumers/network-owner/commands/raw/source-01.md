# 原文：WebBridgeServer::Impl::HandleTextMessage（1）

[上層](../index.md)。來源`HT9011UC_Cpp_V3.33.906.0/WebBridge/WebBridgeServer.cpp`、pin`34c8e2e164014a93699f0b0a712366f5da01ebd2`；本頁payload SHA256 `ac5882e8f211f41925ab416bb8a25a7d37c3bc129ffd55252b3244bb9e509c41`。
完整函式完成數見manifest；context／helper／adapter與拆頁不另計。原comment／metadata保存，本輪未執行程式。

```cpp
<!-- preserved-content:start -->
// -----------------------------------------------------------------------------
//  One inbound JSON text message. Parsed with the vendored cJSON.
//
//  NOTHING here calls machine logic. A validated command is pushed onto the
//  CommandQueue and this function returns; the UI thread drains it later and
//  calls CompleteCommand(), which is what finally produces the ack.
// -----------------------------------------------------------------------------
void WebBridgeServer::Impl::HandleTextMessage(Conn& c, const std::string& text)
{
    cJSON* root = cJSON_Parse(text.c_str());
    if (!root) {
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        SendAck(c, 0, false, "malformed json");
        return;
    }

    const cJSON* jType = cJSON_GetObjectItemCaseSensitive(root, "type");
    const std::string type = (jType && cJSON_IsString(jType) && jType->valuestring)
                             ? std::string(jType->valuestring) : std::string();

    const cJSON* jId = cJSON_GetObjectItemCaseSensitive(root, "id");
    const bool haveId = (jId && cJSON_IsNumber(jId));
    const double id = haveId ? jId->valuedouble : 0.0;

    if (type == "ping") {
        // Liveness. Answered from this thread: it touches nothing but the socket.
        SendAck(c, id, true, std::string());
        cJSON_Delete(root);
        return;
    }

    if (type != "cmd") {
        // Forward compatibility: unknown frame types are ignored, not fatal.
        cJSON_Delete(root);
        return;
    }

    const cJSON* jCmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    const std::string cmdName = (jCmd && cJSON_IsString(jCmd) && jCmd->valuestring)
                                ? std::string(jCmd->valuestring) : std::string();
    if (g_W906OpLogHook) g_W906OpLogHook("RECV", c.id, id, true, cmdName, text);   //AI(W906-OPLOG) 20260928: every cmd frame as received (occupies a blank line, no line moves)
    const cJSON* jTag = cJSON_GetObjectItemCaseSensitive(root, "tag");
    const std::string tagName = (jTag && cJSON_IsString(jTag) && jTag->valuestring)
                                ? std::string(jTag->valuestring) : std::string();

    std::string reject;
    if (!haveId)                            reject = "missing numeric id";
    else if (cmdName.empty())               reject = "missing cmd";
    else if (!IsSaneName(cmdName, 64))      reject = "illegal cmd name";
    else if (!tagName.empty() && !IsSaneName(tagName, 128)) reject = "illegal tag name";

    TagValue value = sib::MakeNull();
    if (reject.empty()) {
        const cJSON* jVal = cJSON_GetObjectItemCaseSensitive(root, "value");
        if (!jVal || cJSON_IsNull(jVal))    value = sib::MakeNull();
        else if (cJSON_IsBool(jVal))        value = sib::MakeBool(cJSON_IsTrue(jVal) != 0);
        else if (cJSON_IsNumber(jVal))      value = sib::MakeNumber(jVal->valuedouble);
        else if (cJSON_IsString(jVal) && jVal->valuestring)
                                            value = sib::MakeString(std::string(jVal->valuestring));
        else                                reject = "unsupported value type";
    }

    // The read-only gate. Default configuration lands here for every command.
    if (reject.empty() && readOnly.load()) reject = "bridge is read-only";
    if (reject.empty() && !queue)          reject = "no command queue attached";

    // AI(W906-FW-W3) 20260819: single-operator control token (design doc
    // section 3). control.acquire/release are answered HERE, from the socket
    // thread -- they touch nothing but server state, same in-thread rule as
    // "ping". Every other command except the auth.* family requires the
    // caller to BE the holder. Sits after the read-only gate on purpose: a
    // read-only bridge stays uniformly fail-closed for every cmd.
    if (reject.empty() && (cmdName == "control.acquire" || cmdName == "control.takeover")) {   //AI(W906-TAKEOVER) 20260926: EastSun「我進入IO頁面就應該把控制權拿回來」
        const unsigned long long owner = ctrlOwner_.load();
        if (owner == 0 || owner == c.id || cmdName == "control.takeover") {   //AI(W906-TAKEOVER) 20260926: takeover moves the token to this connection whoever holds it (loopback-only bridge, one operator at the HMI); the old holder learns it on its next command (not-operator). Same lines, no line moves.
            ctrlOwner_.store(c.id);
            ctrlLastCmdMs_ = NowMs();
            SendAck(c, id, true, std::string());
        } else {
            SendAck(c, id, false, "control-held");
        }
        cJSON_Delete(root);
        return;
    }
    if (reject.empty() && cmdName == "control.release") {
        if (ctrlOwner_.load() == c.id) {
            ctrlOwner_.store(0);
            SendAck(c, id, true, std::string());
        } else {
            SendAck(c, id, false, "not-operator");
        }
        cJSON_Delete(root);
        return;
    }
    // AI(W906-P6-WINREG) 20260920: `ui.windows.put` 與 `auth.*` 同列豁免。
    //
    //   它是**回報**，不是寫入 —— 瀏覽器只是告訴 C++「哪些視窗開著」
    //   （Steven `WINDOW_REGISTRY_CONTRACT.md` §4/§9）。
    //
    //   ⚠ 為什麼一定要豁免，而不是叫 background.html 去 acquire：
    //   `background.html` 是常駐的背景頁，它一旦拿到權杖就會**一路持有不放**，
    //   於是真正要存檔的那一頁（Contact / Teach / Speed）就再也拿不到 ——
    //   整個 HMI 的存檔會失效。把「回報」放進「單一操作員」的閘門裡，
    //   本來就是把兩種不同的東西混在一起。
    //
    //   ⚠ 刻意用**完全比對**而不是 `ui.` 前綴：前綴等於預先替所有未來的
    //   `ui.*` 指令開好洞，而其中很可能會有真的會寫東西的。豁免要一條一條給。
    //
    //   ⚠ 豁免也順帶代表它**不會**去更新 `ctrlLastCmdMs_`（權杖的閒置計時）。
    //   那是對的：背景頁推總表不該把別人的權杖續命。
    // AI(W906-JSONBRIDGE-S1) 20260923: `cfg.resync` 與 `log.event` 比照
    //   `ui.windows.put` 豁免，理由與上面那三個 ⚠ 完全同源。
    //
    //   `cfg.resync` —— **純讀**，只把「這顆 exe 是什麼組態建出來的」再說一次，
    //   不碰機台任何狀態。而它非豁免不可的理由跟 `ui.windows.put` 一樣尖銳：
    //   `background.html` 在渲染任何依機型而異的元件之前就需要這份組態，
    //   但它一旦 acquire 就會一路持有不放，真正要存檔的那一頁永遠拿不到。
    //   把「開機要問的事」放進「單一操作員」的閘門裡，是把兩種東西混在一起。
    //
    //   `log.event` —— 它是**回報**不是機台寫入：瀏覽器告訴 C++「操作員在畫面上
    //   做了什麼」，落點是留痕，不是機台。擋住它會讓稽核軌跡剛好在非操作員的
    //   那些頁面上破一個洞 —— 而那正是最需要知道「誰在什麼時候看了什麼」的地方。
    //   ⚠ 代價是任何連上的瀏覽器都能寫 ring。可接受，因為 ring 有界（200 筆）、
    //     每筆都標 origin、而且 `log.dropped` 會讓灌爆這件事看得見。
    //
    //   ⚠ 一樣用**完全比對**而不是前綴。`cfg.` 與 `log.` 底下都很可能長出真的
    //     會寫東西的指令（例如未來的 `cfg.put`），豁免要一條一條給。
    //   ⚠ 一樣不更新 `ctrlLastCmdMs_`：查組態與留痕都不該把別人的權杖續命。
    if (reject.empty()
        && cmdName.compare(0, 5, "auth.") != 0
        && cmdName != "ui.windows.put"
        && cmdName != "cfg.resync"
        && cmdName != "log.event" && cmdName != "modal.answer" && cmdName != "dialog.response" && cmdName != "dialog.notifyAck" && cmdName != "dialog.auth" /*AI(W906-D026) 20261001 St01: the alarm note's password answer, exempt like dialog.response (an alarm must be answerable without the token; the wait loop only takes it for the current qid)*/ && cmdName != "motor.stop" && cmdName != "act.home.abort" && cmdName != "contactct.get" && cmdName != "counterclear.get" && cmdName != "observer.get"   //AI(W906-HOMEMON) 20261001: act.home.abort exempt with motor.stop -- the Home Monitor's Abort Home (golden sbAbortHomeClick, stop direction) must not be held by a token in another tab; wb_serve runs it only while the home sequence shows the form (JsonBridge/ChanHome.cpp)
        && cmdName != "vacuum.get" && cmdName != "vacuum.open" && cmdName != "vacuum.close" && cmdName != "pad.get" && cmdName != "pad.close" /*AI(W906-W155) 20261007 (St02-E): the Pad window's poll (= its heartbeat) and its close (golden FormClose, bShow=false) are a READ and the safe direction -- exempt like vacuum.get / .close; pad.open / button / send / bling / exit write the pad COM and need the token*/ && cmdName != "act.observerSG.state" && cmdName != "panel.key" /*AI(W906-SOFTKEY-NOTOKEN) 20261003 (machine cpp 0157 f23150a6; ST02-P2 port 20261006 St02-E, without the machine's panel.estop -- RULINGS_20261005 #17): a screen panel key is a press sent without the token step, like the physical key; golden's own gates (ScanPannelKey / TfMain::ScanKey) still decide. Human review*/) {   //AI(W906-VACUNIT-1203) 20260930: HW.VacuumUnit's live view (golden tmr1Timer, polled every 1 s) is a READ -- exempt like observer.get, so the poll never takes the operator token (Motor Test's jog/HOME watchdog follows the holder). vacuum.setSV / do / setAll / reset are presses and stay behind the token.  // AI(W906-ALARM-ANSWER-TOKEN) 20260924: 告警回答豁免權杖 —— 使用者 F5 實測：MES0920 框上按 PAUSE 回 not-operator。ht9045_recipe_client.js:532-537 的 modalAnswer 刻意不 acquire（「警報一定要答得掉」，權杖可能在別的分頁），這裡卻沒豁免 ⇒ 告警框 iframe 的連線永遠答不掉。安全性不靠權杖：wb_serve 等待迴圈只收當前 qid＋有提供的選項（tools/wb_serve.cpp 回答處理），沒有告警時回 no query pending。⚠ c3c459f 起回答 START 會重走 StartFromWeb ⇒ 任何連上的瀏覽器答 START 都能讓機台繼續（golden 也是誰在框前誰按）  AI(W906-W4-MOTOR) 20260925: `motor.stop` 同列豁免 —— 停機不能被「權杖在別的分頁」擋住（golden 任何畫面的 STOP 都停得了；EastSun 的停止也不看控制權）。安全性不靠權杖：wb_serve 的 W906_MotorAccessWire 把 motor.stop 鎖死在 action=="stop"，其他動作走 motor.access 照樣要權杖。  AI(W906-Q2-S124) 20260927 (St02): Steven S124 = B -- the three read-only queries by exact name, no prefix (the laptop 20260927 10:2x: a `*.get` prefix would also pass a future `xxx.get` that writes). contactct.get / counterclear.get read, write no file and move nothing; observer.get is exempt HERE by name, but its four Yield acts (yieldSite / yieldMax / yieldMin / yieldClear) change memory, so wb_serve checks the token for them per act (WebCmdGuard::Exempt, AI(W906-Q2-OBS) 20260927, St02-E) -- the earlier "only read" wording was wrong. editlist.get stays token-gated (it is not read-only). ctest: tests/test_wb_server.cpp section 7.   AI(W906-J5-ACK) 20260930: `dialog.notifyAck` (INBOX 119) exempt by exact name with the two alarm answers -- the operator acknowledges a kCode==0 notice from the alarm iframe, which never acquires the token (same reason as AI(W906-ALARM-ANSWER-TOKEN)); it only retires a notice whose requestId matches (tools/wb_dialog_mailbox.h NotifyAckHandle), never starts anything.  ctest: tests/test_wb_server.cpp section 7   //AI(W906-ST02-OB7F) 20261004 (St02-E, NB2 R180 low (b)): + act.observerSG.state = the Observer SG_JamCount grid read back (ObserverSGJam.cpp op "state": no log line, no golden code), read only -- a viewer without the operator token sees the table (queryNow / queryYesterday stay operator-only)
        if (ctrlOwner_.load() != c.id) reject = "not-operator";
        else                           ctrlLastCmdMs_ = NowMs();
    }

    if (!reject.empty()) {
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        SendAck(c, id, false, reject);
        cJSON_Delete(root);
        return;
    }

    const unsigned long long ticket = nextTicket_++;  { WbGuard pl(pendMx_); PendingAck pa; pa.connId = c.id; pa.browserId = id; pending_[ticket] = pa; pendingOrder_.push_back(ticket); while (pendingOrder_.size() > kMaxPendingAcks) { pending_.erase(pendingOrder_.front()); pendingOrder_.pop_front(); } }   //AI(W906-IOWEB-P25c) 20260925: the pending ack is registered BEFORE the push. The tick thread now wakes on the push (CommandQueue::waitForPush) and a DO write takes microseconds, so it could CompleteCommand before the old registration below ran -> CompleteCommand found no ticket and dropped the ack silently; the browser then reported 'no ack' for a coil that DID switch (laptop review of P25, Q3-1)
    if (!sib::QueuePush(queue, ticket, cmdName, tagName, value, c.id)) {   // AI(W906-CONNID) 20260926: 帶上這條連線的 id（同上一行 PendingAck 的 connId）
        {
            WbGuard sl(statsMx);
            ++stats.cmdRejected;
        }
        { WbGuard pl(pendMx_); pending_.erase(ticket); }  SendAck(c, id, false, "command queue full");
        cJSON_Delete(root);
        return;
    }

    if (false) {   //AI(W906-IOWEB-P25c) 20260925: the registration moved to :1461 (kept here, dead, so no line below moves)   [AI(W906-MERGE-56bbf785) 20260926: now :1463 -- the laptop's PostQueryOptions declaration (W906-YESNO) added 2 lines at :420-421]
        WbGuard pl(pendMx_);
        PendingAck pa;
        pa.connId    = c.id;
        pa.browserId = id;
        pending_[ticket] = pa;
        pendingOrder_.push_back(ticket);
        while (pendingOrder_.size() > kMaxPendingAcks) {
            pending_.erase(pendingOrder_.front());
            pendingOrder_.pop_front();
        }
    }
    {
        WbGuard sl(statsMx);
        ++stats.cmdAccepted;
    }
    // No ack yet -- ARCHITECTURE.md section 5: the ack is sent when the UI
    // thread has actually processed the command (CompleteCommand).
    cJSON_Delete(root);
}


<!-- preserved-content:end -->
```
