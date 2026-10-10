# 原文：ForwardShowErrorMessage（2）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 80行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
                    //     pnlPause 是另一條路（送 action PAUSE，不在 KeyComp 裡，這裡本來就會回 not an offered option），不動。
                    //   ⓘ 只是設旗標，跟 pause.run 一樣由 MainProc 在下一拍消化，不會卡住 tick。
                    if (pressed == "BtnPause") {
                        SoftStop  = true;                                       // golden note.cpp:3981
                        SoftStart = false;                                      // golden note.cpp:3982
                        std::printf("query qid=%s: pressed=BtnPause -> SoftStop=1 SoftStart=0 (golden BtnPauseClick)\n", qidStr);  { extern void W906_NoteBlockingPauseLikeGolden(int); W906_NoteBlockingPauseLikeGolden(k); }   //AI(W906-I37A) 20261002: INBOX 123 -- golden BtnPauseClick :3840 + SIM key records :3983-4034 + DoPause / ESD STOP :4036-4038 (forms/fNote_ShowError.cpp EOF)
                    }
                    if (i + 1 < local.size()) { g_carry.insert(g_carry.begin(), local.begin() + (i + 1), local.end()); g_carryRunnable = true; }  { extern void W906_NoteJamCountOnClose(int); W906_NoteJamCountOnClose(k); }  /* AI(W906-J2) 20260926: golden FormClose :2528 Jam 計數，在重走 START 之前（forms/fNote_JamCount.cpp） */  if (pressed.empty() || pressed == "BtnStart") { extern bool W906_AlarmAnswerStartLikeGolden(const char*); W906_AlarmAnswerStartLikeGolden(qidStr); }  { extern void W906_NoteFormCloseAlarmClear(); W906_NoteFormCloseAlarmClear(); }  return k;  // AI(W906-HALARM-CLOSE) 20260926: golden TfNote::FormClose note.cpp:2531 Alarm->Clear()（HAlarm.cpp 檔尾）  // AI(W906-ALARMSTOP) 20260924: 按 START（或沒帶按鍵）照 golden TfNote::Start（note.cpp:3665-3678）重走啟動檢查 —— 本體在檔尾  //AI(W906-MERGE-56bbf785) 20260926: the commands drained together with the answer (local[i+1..]) were dropped here -- never run, never acked (the browser saw an ack timeout). They go back to the FRONT of g_carry (machine IOWEB-P25 carry; the main loop wakes on !g_carry.empty() and drains it first), in arrival order. Put back BEFORE the START side effect: StartFromWeb can raise another alarm, and that wait must see these older commands before anything newer. Same line, so no line below moves
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                               "not an offered option");
            } else if (wc.cmd == "ui.windows.put") {
                //AI(W906-MT-FIX1) 20260926: 視窗總表在 modal 掛著時也收（同 cfg.resync 的理由：它是「回報」不碰機台狀態，
                //   WebBridgeServer.cpp:1408-1423 本來就豁免權杖）。原本這裡回 modal-pending：告警框等回答超過 15 秒，
                //   總表的每一條連線都過期、stale 算「開著」（WebWindowRegistry 契約 §6）⇒ fMotorTest／fTeach 讀成開著，
                //   MT-E3b 起 golden MainProc 在它們開著時暫停 ⇒ 答完告警之後生產仍然停著，直到下一次心跳（審查 high 的 C++ 那一半）。
                std::string whyW;
                const std::string frameW = (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string();
                if (!frameW.empty() && ht9045::WebWindowRegistryPut(wc.connId, frameW, whyW))
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, "{\"accepted\":true,\"duringModal\":true}");
                else
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, false,
                                                   "ui.windows.put: " + (frameW.empty() ? std::string("value must be the frame JSON string") : whyW));
            } else if (wc.cmd == "cfg.resync") {
                // AI(W906-JSONBRIDGE-S0) 20260923: 組態查詢在 modal 掛著時也放行。
                //
                //   它不碰機台任何狀態 —— 只把「這顆 exe 是什麼組態建出來的」
                //   再說一次。擋住它反而製造死結：瀏覽器重整之後會收到這個還沒
                //   答的警報框，而它**在渲染那個框之前**需要知道機型與
                //   SOFT_SIMULTE（要不要顯示某些按鈕）。擋住 = 框畫不出來 =
                //   答不了 = modal 永遠不會解除。
                //
                //   ⚠ 這是 modal 期間唯一的例外。要再加別的指令進來以前，先問：
                //     它會不會改到機台狀態？會的話就不該在這裡。
                unsigned long long since = 0;
                // ⚠ TagValue 的存取子是**嚴格**的：asInt() 對非 Int
                //   一律回 fallback，不做型別轉換（WebBridge/TagValue.h 的
                //   Inspection 那節）。而 JSON 的 `1` 在指令通道上會變成 Double，
                //   所以只寫 isNumber()+asInt() 的話 since 永遠是 0 ——
                //   cfg.resync 於是永遠回「有變」並夾帶完整內容，
                //   unchanged 那條捷徑等於不存在。
                //   20260923 由 tools/webprobe/s1_eventlog_probe.py 抓到，不是看出來的。
                if (wc.hasValue) {
                    if (wc.value.isInt())
                        since = (unsigned long long)wc.value.asInt(0);
                    else if (wc.value.isDouble())
                        since = (unsigned long long)wc.value.asDouble(0.0);
                }
                g_modalServer->CompleteCommand((unsigned long long)wc.id, true,
                                               ht9045::sjson::ConfigResyncJson(since));
            } else if (::W906_StateRecordDuringDialog(*g_modalServer, wc)) { } else {   //AI(W906-S24-Q93) 20261004 (St02-E; Steven Q93 = A): State Record while this box waits (tools/wb_serve.cpp end of file)
                extern bool W906_IoPageNoGuards(); extern void W906_DispatchIoClick(webbridge::WebBridgeServer&, const webbridge::WebCommand&);   /*AI(W906-IO-NOGUARD) 20260929: EastSun「IO畫面一律不要卡控」-- an IO click is served during this dialog too (JsonBridge/IoBtnPanelClick.cpp W906_IO_PAGE_NO_GUARDS)*/  if (wc.cmd == "io.btnPanelClick" && W906_IoPageNoGuards()) W906_DispatchIoClick(*g_modalServer, wc); else if (wc.cmd == "dialog.auth") { /*AI(W906-D026) 20261001 St01: the login box of THIS note (golden TfNote::DoUnlockPassword / DoPassword for the press the page names) -- WebLogin.cpp EOF; the reply has no password; nothing here is printed but the verdict*/ extern bool W906_NoteAuthVerify(const std::string&, const std::string&, const std::string&, bool, std::string*); std::string naReply; const bool naOk = W906_NoteAuthVerify(wc.hasTag ? wc.tag : std::string(), (wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), std::string(qidStr), true, &naReply); g_modalServer->CompleteCommand((unsigned long long)wc.id, naOk, naReply); } else if (wc.cmd == "dialog.notifyAck") { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, w906dlg::WaitNotifyAckReply(g_alarmSlot, wc.hasTag ? wc.tag : std::string(), qidStr)); } else if (wc.cmd == "motor.stop") { extern bool W906_MotorAccessWire(const std::string&, long long, std::string&, bool); std::string msAck; const bool msOk = W906_MotorAccessWire((wc.hasValue && wc.value.isString()) ? wc.value.asString() : std::string(), (long long)wc.id, msAck, true); g_modalServer->CompleteCommand((unsigned long long)wc.id, msOk, msAck); } else if (wc.cmd == "act.home.abort") { W906_HomeAbortCommand(*g_modalServer, wc); } else { extern bool W906_MsgBoxModelessAnswer(const webbridge::WebCommand&); extern bool W906_SimDiCommand(const webbridge::WebCommand&); if (!W906_MsgBoxModelessAnswer(wc) && !W906_SimDiCommand(wc)) g_modalServer->CompleteCommand((unsigned long long)wc.id, false, w906dlg::WaitOtherReply(wc.cmd, wc.hasTag ? wc.tag : std::string(), qidStr)); }   /*AI(W906-S17) 20261003 St01: notifyAck -> WaitNotifyAckReply (E), a stale answer -> WaitOtherReply (A); both were the box-keeping not-a-notice / modal-pending*/   /*AI(W906-HOMEMON) 20261001: act.home.abort (Home Monitor's Abort Home = golden sbAbortHomeClick, JsonBridge/ChanHome.cpp) answered under this alarm like motor.stop -- a stop is not held behind a box ([W906]; see ChanHome.h for what golden has here)*/   /*AI(W906-SMM) 20260925: 告警框開著時，網頁上不停機的 MyMessageBox 仍按得掉（dialog-bridge.js 不停機那一層在最上層、可單獨按）—— C++ 這側也要收，否則網頁關了、C++ 的 fShow／iUnLoaderCount 還留著（檔尾 W906_MsgBoxModelessAnswer）*/   /*AI(W906-W4D) 20260925: NB2 R23 §2 -- motor.stop answered even under a modal alarm: STOP never starts motion, golden keeps the physical STOP live during a modal, and for 1203 axes opened by the EastSun monitor this is the only stop there is (motor.stop is locked to action=stop)*/   /*AI(W906-J5-ACK) 20260930: dialog.notifyAck while this blocking alarm waits = not-a-notice: this alarm owns the mailbox (a notice under it, kShown==0, is retired by this alarm's DialogMailboxRetire) -- nothing retired, nothing applied, answered at once*/
            }
        }
        // ===== AI(W906-SMM-IO) 20260925 BEGIN -- 框開著時的實體面板鍵（RULINGS_20260925 第 42 條）=====
        //   golden fNote 在 ShowModal 期間由 Timer1Timer（note.cpp:3172 起）→ ScanKey（:2894-3121）掃面板：
        //   K_RETRY／K_SKIP… 先選取，再按 K_PAUSE（BtnPauseClick :3866）或 K_START（Start :3560）確認。
        //   解除時與網頁回答走同一個出口（下面幾行照抄上面 answered 分支），另寫 Dialog-close-request 讓網頁收框。
        {
            extern int W906_AlarmIoAnswer(const char*, int, std::string*, std::string*, std::string*);
            extern void W906_DialogCloseRequest(const char*, const std::string&, unsigned long long, const std::string&, int, const char*);
            extern unsigned long long g_dialogAlarmSeq;  extern bool W906_NoteAuthIoGate(const char*, int, const std::string&);   //AI(W906-D026) 20261001 St01: WebLogin.cpp EOF (same line)
            std::string ioAns, ioPressed, ioInput;
            const int kio = W906_AlarmIoAnswer(qidStr, kcode, &ioAns, &ioPressed, &ioInput);
            if (kio > 0 && (kio & kcode) != 0 && W906_NoteAuthIoGate(qidStr, kio, ioPressed)) {   //AI(W906-D026) 20261001 St01: + golden Start() / BtnPauseClick() DoUnlockPassword / DoPassword -- when golden would open a password box the panel key does not close the note ([W906] fail closed: the box is only on the screen). Same line
                g_modalServer->ClearQuery(qid);
                DialogMailboxRetire();
                W906_DialogCloseRequest("show-error-message", qidStr, g_dialogAlarmSeq, ioAns, kio, ioInput.c_str());
                ht9045::sjson::ClearAlarm(qidStr, ioAns, ioPressed, kio);
                std::printf("query qid=%s answered %s (K=%d) via panel IO %s pressed=%s\n",
                            qidStr, ioAns.c_str(), kio, ioInput.c_str(), ioPressed.c_str());
                if (ioPressed == "BtnPause") { SoftStop = true; SoftStart = false;  extern void W906_NoteBlockingPauseLikeGolden(int); W906_NoteBlockingPauseLikeGolden(kio); }        // golden note.cpp:3981-3982（同上面網頁那條）；AI(W906-I37A) 20261002：INBOX 123 同上
                { extern void W906_NoteJamCountOnClose(int); W906_NoteJamCountOnClose(kio); }  /* AI(W906-J2) 20260926: 同上面網頁那個出口 */  if (ioPressed == "BtnStart") { extern bool W906_AlarmAnswerStartLikeGolden(const char*); W906_AlarmAnswerStartLikeGolden(qidStr); }
                { extern void W906_NoteFormCloseAlarmClear(); W906_NoteFormCloseAlarmClear(); }  return kio;   // AI(W906-HALARM-CLOSE) 20260926: 面板鍵回答也是關框，同上（golden note.cpp:2531）
            }
        }
        // ===== AI(W906-SMM-IO) 20260925 END =====
    }
}


<!-- preserved-content:end -->
```
