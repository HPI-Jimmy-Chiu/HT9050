# 原文：ForwardShowMyMessageBoxYesNo（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 92行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static int ForwardShowMyMessageBoxYesNo(const char* s1, const char* s2, const char* s3)
{
    // 沒有網頁可以問（伺服器還沒起來 —— 例如開機序列裡 cinitial.cpp 的四個呼叫 —— 或已關站）：
    //   回 0 ＝「沒有人回答」，canary_support.cpp 改回 SimReturn（預設 0，替身時代的值）。
    //   ⚠ 這條路刻意**不停機**：沒問就不該有 golden「框開著」的副作用，行為與 20260925 前逐位元相同。
    if (!g_modalServer || !g_pumpQueue) return 0;

    // golden mymessbox.cpp:1012-1015：已有 MyMessageBox 開著（且不是不停機那種）就回 3、不再開第二個。
    //   移植樹的等待迴圈佔住唯一的 tick 執行緒，迴圈裡不呼叫任何狀態機碼，所以今天不可能重入；
    //   守衛照翻，萬一日後有人在迴圈裡加了會跳框的呼叫，結果仍與 golden 同。
    //   ⓘ golden :1016-1019（不停機的 MyMessageBox 開著時先 Close 再開）[AI(W906-MODAL-WAKE) 20260926：現在照翻 —— ShowUnloaderTrayMessage 的
    //     非阻塞框佔 fShow（MbFormShow），下一行之後的 W906_YesNoPreCloseLikeGolden 先關它，會停機的那種回 3（檔尾）]
    static bool s_showing = false;
    if (s_showing) return 3;  if (W906_YesNoPreCloseLikeGolden() == 3) return 3;   // AI(W906-MODAL-WAKE) 20260926: golden :1012-1019（見上兩行）
    s_showing = true;

    { extern bool W906_MotorAccessArmCellOnPopup(const char*, const char*); W906_MotorAccessArmCellOnPopup("是／否框", s1); }  /*AI(W906-ARMCELL2) 20261002: NB2 R165 (1) -- the Teach Arm Cell job stops on this box (ArmCellLive.cpp EOF); same line*/  { extern void W906_YesNoShowLikeGolden(); W906_YesNoShowLikeGolden(); }   // golden :1020 StopAllMotor ＋ FormShow :70／:302-310／:353（:1049-1053 缺相依，見 SAFETY-GATE(W906-YESNO-FTCT)）—— 本體在檔尾

    const unsigned long long qid = g_nextQid++;
    char qidStr[24];
    std::snprintf(qidStr, sizeof(qidStr), "%llu", qid);

    std::vector<std::string> options;
    options.push_back("YES");
    options.push_back("NO");
    g_modalServer->PostQueryOptions(qid, "yes-no", s1 ? s1 : "", options);
    const bool posted = DialogMailboxPostYesNo(qidStr, s1, s2, s3);
    if (!posted && !g_dialogMailboxDir.empty())
        std::printf("  ⚠ Message 信箱寫入失敗 —— 只剩 WebSocket 那條路\n");
    std::printf("yesno qid=%s \"%s\" -- waiting (mailbox=%s, ws=yes)\n",
                qidStr, s1 ? s1 : "", posted ? "yes" : "no");
    std::fflush(stdout);

    WdMark2("yes/no wait (blocking, needs a browser answer): ", s1 ? s1 : "");
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(1);  w906dlg::WaitTagScope s17Wait(qidStr);   /*AI(W906-S17) 20261003 St01: this wait holds qidStr*/   // AI(W906-MODAL-WAKE) 20260926: golden TMyMessageBox::FormShow :336／:344、FormClose :408（檔尾）
    for (;;) {
        if (g_carry.empty()) { extern bool W906_FastClockModalWait(webbridge::CommandQueue&, unsigned long); W906_FastClockModalWait(*g_pumpQueue, 100); }  /* AI(W906-FASTCLK-MODAL) 20261003: was g_pumpQueue->waitForPush(100) -- the same wait (a command wakes it, 100 ms at most), sliced at the fast clock's due jobs so the heater / panels keep their period while the box is up (E-FT1-001; FastClockWbServe.cpp) */  { extern void DoAvoidIndexMotorFallDown(); DoAvoidIndexMotorFallDown(); }  W906_ModalWaitTick(1);  { extern void W906_TesterCommPoll(); W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   //AI(W906-MODAL-SAFETY) 20260925: golden 在 ShowModal 期間 MyMessageBox::Timer1Timer（mymessbox.cpp:538-550／:650）照跑 DoAvoidIndexMotorFallDown（EMG／斷電／Index Z servo off ⇒ 鎖 Index 煞車並停機）；移植樹等待期間 tick 停住，這裡補跑（R-YESNO 審查 high；本體不跳框、SOFT_SIMULTE 下是 no-op）  //AI(W906-MERGE-56bbf785) 20260926: laptop wrote Sleep(100) here; machine P25c replaced that exact Sleep in ForwardShowErrorMessage with this event wait (an answer / motor.stop wakes it at once). This wait is the same shape, so it gets the same line
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   //AI(W906-MERGE-56bbf785) 20260926: laptop drained only the queue; machine IOWEB-P25 requires EVERY drain (main loop and each modal wait) to take g_carry first, in arrival order -- otherwise a motor.stop the output-first service parked in g_carry (behind a barrier) would wait until this dialog is answered
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response")
                && wc.hasTag && wc.tag == qidStr) {
                std::string ans = (wc.hasValue && wc.value.isString())
                                  ? wc.value.asString() : std::string();
                std::string pressed;
                const std::string::size_type colon = ans.find(':');   // 今天送來的是純 "YES"/"NO"：dialog-bridge.js responseFor() 只替告警通道帶 pressedButton，訊息通道沒有；"YES:pnlYes" 形式照告警那支一併接受（20260925 無頭 Edge 自測實測）
                if (colon != std::string::npos) {
                    pressed = ans.substr(colon + 1);
                    ans     = ans.substr(0, colon);
                }
                const int v = YesNoValueOf(ans);
                if (v != 0) {
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                    g_modalServer->ClearQuery(qid);        // 不清：下一個連上來的瀏覽器會收到早答完的框
                    DialogMailboxRetireMessage();          // 不退役：答完按 F5 同一題再彈一次
                    if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }  { extern void W906_YesNoCloseLikeGolden(const char*, const char*, int); W906_YesNoCloseLikeGolden(s1, s3, v); }   // golden FormClose :385-442 ＋ :1056-1061 MyDBIProcess —— 本體在檔尾  //AI(W906-MERGE-56bbf785) 20260926: same fix as the ShowErrorMessage wait's answer (:627) -- local[i+1..] was dropped at the `return v;` below (never run, never acked); it goes back to the front of g_carry in arrival order, before the golden close side effects. Same line, so no line below moves
                    s_showing = false;
                    std::printf("yesno qid=%s answered %s (iValue=%d) via %s%s%s\n",
                                qidStr, ans.c_str(), v, wc.cmd.c_str(),
                                pressed.empty() ? "" : " pressed=", pressed.c_str());
                    std::fflush(stdout);
                    return v;
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                               "not an offered option");
            } else if (wc.cmd == "cfg.resync") {
                // 同 ForwardShowErrorMessage：純查詢、不碰機台，擋住反而讓重整後的頁面畫不出框。
                unsigned long long since = 0;
                if (wc.hasValue) {
                    if (wc.value.isInt())
                        since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble())
                        since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true,
                                               ht9045::sjson::ConfigResyncJson(since));
            } else if (wc.cmd == "ui.windows.put") { std::string whyW; const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW)) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}"); else g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));   //AI(W906-MERGE-56bbf785) 20260926: machine MT-FIX1 accepts ui.windows.put during the ShowErrorMessage wait (a wait over 15 s lets the window registry go stale -> fMotorTest/fTeach read "open" -> MainProc stays paused after the answer); laptop's YES/NO wait says its exceptions are "理由同上面那支", so it takes the same one. Same body as that branch, on one line
            } else if (wc.cmd == "motor.stop") {
                // 同 ForwardShowErrorMessage（AI(W906-W4D) 20260925）：STOP 永遠放行。
                extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool);
                std::string msAck;
                const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                       (long long)wc.id, msAck, true);
                g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
            } else if (wc.cmd == "act.home.abort") { W906_HomeAbortCommand(*g_modalServer, wc);   /*AI(W906-HOMEMON) 20261001: Abort Home answered during the YES/NO box too, like motor.stop above (JsonBridge/ChanHome.cpp)*/ } else if (W906_SimDiCommand(wc)) { } else if (wc.cmd == "io.btnPanelClick" && []() { extern bool W906_IoPageNoGuards(); return W906_IoPageNoGuards(); }()) { extern void W906_DispatchIoClick(webbridge::WebBridgeServer&, const webbridge::WebCommand&); W906_DispatchIoClick(*g_modalServer, wc); } else if (::W906_StateRecordDuringDialog(*g_modalServer, wc)) { } else {   /*AI(W906-IO-NOGUARD) 20260929: IO click served during the YesNo dialog too*/   // AI(W906-R70) 20260926 YN-3：是／否框也收模擬 DI（另外兩種框 :671／:6782 都收；同一行寫完，行數不變），SOFT_SIMULTE 下才驗得到第 26 條 Q3 的「面板 Alarm Reset 消音」   //AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A): State Record during this box
                { extern bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand&); if (wc.cmd == "dialog.notifyAck") g_modalServer->CompleteCommand((unsigned long long)wc.id, false, w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qidStr)); else if (!W906_MsgBoxModelessAnswer(wc)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qidStr)); }   /*AI(W906-S17) 20261003 St01: was "modal-pending" for all three -- a stale answer / a superseded notice now closes its box; the open modeless box is answered as the other two waits do*/
            }
        }
    }
}


<!-- preserved-content:end -->
```
