# 原文 02／分頁 1

[證據入口](../evidence.md)。

來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`；定位 `MbWait`；種類 `complete_cpp_functions`。
來源 commit `5d86796f2cc4a2b1c9c1f3a474dbb4127df085eb`；完整摘錄 SHA256 `e6cd1509cf11d39a42fec3dd6b47087d8b19fc1e7bd220e6d666763ca8f18397`。
原註解、裁決、gate 及歷史 test 敘述保留；歷史驗證不是本輪實測。

```cpp
<!-- preserved-content:start -->
std::string MbWait(const std::string& qid, const char* const* opts, int nOpts)
{
    WdMark2("modal wait (MyMessageBox, needs a browser answer): ", qid.c_str());
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(2);  w906dlg::WaitTagScope s17Wait(qid);   /*AI(W906-S17) 20261003 St01: this wait holds qid*/   // AI(W906-MODAL-WAKE) 20260926: ShowMyMessage 框（fShow／蜂鳴器由 MbFormShow 設）—— 只做 tick 與開瀏覽器
    std::vector<webbridge::WebCommand> w906NoticeHeld;  struct W906NoticeRelease { std::vector<webbridge::WebCommand>& held; ~W906NoticeRelease() { if (!held.empty()) { g_carry.insert(g_carry.begin(), held.begin(), held.end()); g_carryRunnable = true; } } } w906NoticeRelease = { w906NoticeHeld };   //AI(W906-NOTICE-DEFER-2) 20261004 (laptop review of machine cpp 0190): a dialog.notifyAck that arrives during this wait is HELD here and put at the front of g_carry when the wait returns (answer / IO close; the destructor runs after the answer branch put local[i+1..] there, so arrival order is kept) -- the main loop runs and answers it. 0190 pushed it into g_carry directly, but W906_TakeCarry at the top of every pass of THIS loop takes g_carry back, so the same command came round again each pass and `if (g_carry.empty()) waitForPush(100)` never waited: the loop spun (and printed) at full speed until the box closed
    for (;;) {
        if (g_carry.empty()) { ::W906_FastClockModalWait(*g_pumpQueue, 100); }  /* AI(W906-FASTCLK-MODAL) 20261003: was g_pumpQueue->waitForPush(100) -- the same wait (a command wakes it, 100 ms at most), sliced at the fast clock's due jobs so the heater / panels keep their period while the box is up (E-FT1-001; FastClockWbServe.cpp; declared at global scope :6528, this wait is in the unnamed namespace) */  DoAvoidIndexMotorFallDown();  W906_ModalWaitTick(2);  { ::W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   //AI(W906-MERGE-56bbf785) 20260926: laptop wrote Sleep(100); machine P25c's event wait, same as ForwardShowErrorMessage's (an answer / motor.stop wakes it at once, still bounded to 100 ms)  //AI(W906-MERGE-56bbf785) 20260926: + DoAvoidIndexMotorFallDown on every pass, at the same place as the YES/NO wait (:806). golden runs it from TMyMessageBox::Timer1Timer (mymessbox.cpp:650) during ShowModal, and ShowMyMessage is that same form; the port's tick is stopped during this wait, so without it EMG / power loss / Index Z servo-off would not lock the Index brake while the box is open. Declared at file scope (:6519)
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   //AI(W906-MERGE-56bbf785) 20260926: machine IOWEB-P25 -- every drain takes g_carry first (see the same line in ForwardShowErrorMessage / ForwardShowMyMessageBoxYesNo)
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response") && wc.hasTag && wc.tag == qid) {
                const std::string a = MbAnswerOf(wc);
                if (!MbIn(a, opts, nOpts)) {
                    std::printf("  [MyMessageBox] %s: 回答 %s 不是這一則的選項 -> not an offered option\n", qid.c_str(), a.c_str());
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "not an offered option");
                    continue;
                }
                if ((a == "OK" || a == "PAUSE") && bWaitSecsGemReply) {        // golden pnlPauseClick :450-453
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                                   "bWaitSecsGemReply: golden pnlPauseClick ignores the key while waiting for EAP");
                    continue;
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                WdMark("serve loop (MyMessageBox answered)");
                if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }   //AI(W906-MERGE-56bbf785) 20260926: same fix as the ShowErrorMessage / YES-NO answers (:627 / :826) -- local[i+1..] was dropped at this return (never run, never acked); back to the front of g_carry (machine IOWEB-P25 carry, drained first by the main loop), in arrival order
                return a;
            }
            if (W906_MsgBoxModelessAnswer(wc)) continue;
            if (W906_SimDiCommand(wc)) continue;                                // AI(W906-SMM-IO): 模擬 DI＝實體鍵，框開著也要收（golden 的面板鍵在 modal 期間仍有效）
            if (wc.cmd == "cfg.resync") {
                unsigned long long since = 0;
                if (wc.hasValue) {
                    if (wc.value.isInt())         since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble()) since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true, ht9045::sjson::ConfigResyncJson(since));
            } else if (wc.cmd == "ui.windows.put") { std::string whyW; const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(); if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW)) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}"); else g_modalServer->CompleteCommand((unsigned long long)wc.id, false, "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));   //AI(W906-MERGE-56bbf785) 20260926: machine MT-FIX1's modal exception (window registry must not go stale during a long wait -> fMotorTest/fTeach "open" -> MainProc paused); laptop says this pump's exceptions are "同告警 pump", so it takes this one too
            } else if (wc.cmd == "motor.stop") {
                std::string msAck;
                const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(),
                                                       (long long)wc.id, msAck, true);
                g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck);
            } else if (wc.cmd == "dialog.notifyAck") {   //AI(W906-NOTICE-DEFER) 20261003: EastSun「我歸原點後 有異常 排除後再按歸原點 就沒用了」-- 21:56:31 the PAUSE on the motor jam NOTICE came while the "Motor not home yet" box waited and was refused modal-pending: the notice stayed open (the page had closed it) and every later HOME key read "擋關：通知框開著" (golden ScanKey ignores keys while the note is up). Not refused any more: carried to the main loop, which runs it (and answers it) as soon as this box closes
                { const std::string tg = wc.hasTag ? wc.tag : std::string(); std::string nw; if (!::W906_NoteAuthNoticeGate(tg, &nw)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, nw); else { w906NoticeHeld.push_back(wc); g_w906NoticeGatePassed.insert((unsigned long long)wc.id); if (::W906_NoteNoticeAckRefusal(tg.c_str()) == 0 && w906dlg::NotifyAckDecide(g_alarmSlot, tg) == w906dlg::kNotifyAckRetire) g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"notice\":\"held\",\"requestId\":\"" + tg + "\",\"runsWhen\":\"" + qid + " closes\"}"); } }   //AI(W906-NOTICE-DEFER-4) 20261007: review of 45d4f4d -- the early ok only checked the BtnPauseClick refusal; the held run's auth gate (main loop :4915) could still refuse after the page closed the note (e.g. auto-logout while this box was up) -> the note lived on in C++ and keys read "通知框開著" again. Now the SAME gate runs here first, at the press as golden DoPassword does: refused -> answered refused (the note stays, press again after dialog.auth), not held; passed -> held, its id remembered so :4915 does not run the gate twice (it spends the one-shot pass); early ok only when the slot really holds this notice (NotifyAckDecide == retire) and BtnPauseClick would not refuse.  AI(W906-NOTICE-DEFER-3) 20261007: EastSun「這畫面我按pause 10秒後才有反應」/「我按reset 畫面很久alarm才會被清除」-- the page shows one stop box at a time and kept "Motor not home yet" queued behind the jam note until this ack was answered, while this wait held the ack until that box closed: a deadlock broken only by the page's 15 s timeout. Now the page gets ok at once (note closes, the box shows, as golden: note PAUSE -> MyMessageBox); the close work itself still runs after the box (held below; its later CompleteCommand on this id is a no-op). Not when golden BtnPauseClick would refuse (the note must stay).   //AI(W906-NOTICE-DEFER-2) 20261004: held, handed to g_carry when this wait returns (the w906NoticeHeld line at the top of MbWait); was g_carry.push_back + g_carryRunnable
                std::printf("  [MyMessageBox] %s 等待中收到通知框的 PAUSE（dialog.notifyAck）-> 框關掉後由主迴圈處理\n", qid.c_str());
            } else if (wc.cmd == "act.home.abort") { ::W906_HomeAbortCommand(*g_modalServer, wc);   /*AI(W906-HOMEMON) 20261001: Abort Home answered during the MyMessageBox wait too, like motor.stop above (JsonBridge/ChanHome.cpp; :: = the file-scope declaration at :439, this wait is in an anonymous namespace)*/ } else if (wc.cmd == "io.btnPanelClick" && ::W906_IoPageNoGuards()) { ::W906_DispatchIoClick(*g_modalServer, wc); } else if (::W906_StateRecordDuringDialog(*g_modalServer, wc)) { } else {   //AI(W906-IO-NOGUARD) 20260929: IO click served during the MyMessageBox wait too.  AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A): State Record during this box
                if (wc.cmd == "modal.answer" || wc.cmd == "dialog.response")   // 答錯對象（例如舊的一則）—— 印出來，現場查「按了沒反應」用
                    std::printf("  [MyMessageBox] %s 等待中收到給 %s 的回答 -> modal-pending\n", qid.c_str(),
                                wc.hasTag ? wc.tag.c_str() : "(no tag)");
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false, wc.cmd == "dialog.notifyAck" ? w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qid) : w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qid));   //AI(W906-S17) 20261003 St01: was "modal-pending" (see the YES/NO pump)
            }
        }
        // AI(W906-SMM-IO) 20260925: golden Timer1Timer（mymessbox.cpp:558-663）—— 面板鍵／開門等條件成立 = golden 的 Close()
        const std::string io = W906MbIoDismiss();
        if (!io.empty()) {
            std::printf("  [MyMessageBox] %s 由 IO 解除：%s（golden mymessbox.cpp Timer1Timer → Close()）\n", qid.c_str(), io.c_str());
            WdMark("serve loop (MyMessageBox closed by IO)");
            return "IO:" + io;
        }
    }
}

<!-- preserved-content:end -->
```
