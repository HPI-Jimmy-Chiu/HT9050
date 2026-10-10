# 原文：ForwardShowErrorMessage（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 180行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static int ForwardShowErrorMessage(const char* code, int kcode, int pos)
{
    const DWORD w906AlmT0 = ::GetTickCount();  if (!g_w906MotorNoteMsg) { extern void W906_AlarmStopLikeGolden(const char*); W906_AlarmStopLikeGolden(code); }  const DWORD w906AlmT1 = ::GetTickCount();  /* AI(W906-ALARMTIME) 20261007: EastSun 1007「出現異常的速度慢」-- T0 entry / T1 after the stop / T2 after the record, printed with the post (below) */  if (!g_w906MotorNoteMsg) /* AI(W906-JAM-STOP) 20260930: both skipped only for golden ShowMotorErrorMessage's note (EOF ForwardShowMotorErrorMessage) -- golden note.cpp:795-868 are ShowErrorMessage's, the motor body did its own stop + record (golden :1054-1092) */ { extern bool W906_ShowErrorMessageRecordLikeGolden(AnsiString, bool, AnsiString); extern AnsiString W906_ShowErrorMessage_LastErrPart; extern bool W906_ShowErrorMessage_LastDuplicate; W906_ShowErrorMessageRecordLikeGolden(AnsiString(code ? code : ""), W906_ShowErrorMessage_LastDuplicate, W906_ShowErrorMessage_LastErrPart); }  const DWORD w906AlmT2 = ::GetTickCount();  if (!g_modalServer || !g_pumpQueue) return 0;   // unattended -> sim answer  // AI(W906-ALARMSTOP) 20260924: 先照 golden note.cpp:795-801 停機（使用者 20260924：kcode≠0 與 kcode==0 都停；kcode==0 仍依 0923 不阻塞）—— 本體在檔尾  //AI(W906-SHOWERR) 20260929: then golden ShowErrorMessage's alarm record (note.cpp:538 / :794 / :802-818 / :839-868 -- MyDBIEvent -> EventTracker / HANDLER LOG csv, ProductionLog, ErrShowToForm -> fNote->edErrorCode / Edit3 / edUnitName / AlarmType; forms/fNote_ShowError.cpp) with the errPart / bDuplicateErr canary_support.cpp recorded; before the unattended return because golden records every alarm (RULINGS_20260929 section 5 item 7)
    const unsigned long long qid = g_nextQid++;
    char qidStr[24];
    std::snprintf(qidStr, sizeof(qidStr), "%llu", qid);  { extern void W906_NoteAuthArm(const char*, const char*, int, bool); W906_NoteAuthArm(qidStr, code, kcode, g_w906MotorNoteMsg != 0); }   //AI(W906-D026) 20261001 St01: golden TfNote::FormShow's password block (note.cpp:1496-1502 / :1818-1941) for this note, after the record and before the posts (WebLogin.cpp EOF). Same line, no line moves

    // AI(W906-Q30-8) 20260922: ⚠⚠ **kCode 守門，放在最前面。**
    //   畫面只接得住 9 個 K（dialog-page.js:25-28 的 ALARM_BTNS）。
    //   若 kcode 只含 K_FIX(0x100) / K_PAUSE(0x400) / K_START(0x800)，
    //   框會跳出來但**一顆可選鍵都沒有**，而 dialog-page.js:61 的
    //   `if (!selected) return;` 讓 Start/Pause 都變 no-op
    //   ⇒ **關不掉、答不了、機台永久卡住**，而且 log 裡什麼都沒有。
    //   ⇒ 寧可在這裡就大聲講，也不要讓操作員對著一個沒有按鈕的框。
    const int kShown = kcode & (K_RETRY | K_SKIP | K_CLEAN_OUT | K_TRAY_FEED |
                                K_TRAY_END | K_RESET | K_HOME | K_TRAIN | K_ONECYCLE);
    if (kcode != 0 && kShown == 0) {
        std::printf("  ⚠⚠ kcode=%d 不含任何畫面接得住的 K —— 框會沒有按鈕。\n"
                    "     畫面支援的是 golden note.cpp:1235 KeyComp[] 那 9 個；\n"
                    "     K_FIX/K_PAUSE/K_START 不在其中（golden 自己註解掉 K_FIX）。\n"
                    "     這一則改走 WebSocket，不寫信箱。\n", kcode);
    }

    // AI(W906-Q30-KZERO) 20260923: kcode==0 是**通知**，不是問題 —— 不進等待迴圈。
    //
    //   下面那個迴圈唯一的出口是
    //   `if (k != 0 && (k & kcode) != 0)`。kcode==0 時 `(k & 0)` 恆為 0
    //   ⇒ **沒有任何出口，永遠等下去**。
    //
    //   而 wb_serve 是單執行緒：這一支卡住，主迴圈的 `if (pumpBeat) ht9045::PumpTick();`
    //   連帶停擺 ⇒ MainProc 不再被呼叫、DoAllProcess 不再跑，而且**其他網頁
    //   命令拿不到 ack** —— start.run 分支的 `CompleteCommand`
    //   要等 `StartFromWeb()` 返回才發。
    //   （AI(W906-Q34-7-L1) 20260923 夜間：這裡原本寫 `:408`／`:2979`／`:3632` 三個行號，
    //    rebase Q34-7 之後全部漂掉（獨立審查 L1）；改成用程式碼本身當錨點，不再換一組會再漂的數字。）三個症狀、一個根因，而且看起來像
    //   三個不相干的 bug（斷點不中／流程不跑／網頁 no ack within 15000ms）。
    //   光是 StartFromWeb() 裡就有約 13 個 `ShowErrorMessage(code, 0, ...)`
    //   （WebStart.cpp:1486/1914/2064/2418/2638/3000/3336…），任一被打到就整台停。
    //
    //   ## 為什麼回 0 是對的
    //   0 ＝「沒有任何鍵被按」，與本函式最上面 :283 的無人值守出口
    //   `if (!g_modalServer || !g_pumpQueue) return 0;` 同值。而且 kcode==0 的
    //   呼叫點**全部丟棄回傳值**（20260923 逐處實測 WebStart.cpp 那 13 個）。
    //
    //   ## 為什麼只寫信箱、不 PostQuery
    //   PostQuery 會在 g_modalServer 留一個待答查詢，而畫面對 kcode==0 生不出
    //   任何按鈕（上面那段 kShown 警告講的就是這件事）⇒ 操作員關不掉，又沒有
    //   人會去 :417 的 ClearQuery() ⇒ 下一個連上來的瀏覽器收到一個關不掉
    //   的框，它的回應還會被判成 `no query pending`。
    //   信箱是**單槽檔**，而單執行緒下不可能在別人阻塞時輪到我們覆蓋它
    //   （阻塞中的那一支正握著這條執行緒），所以只寫信箱是安全的。
    //
    //   ⚠ 使用者 20260923 裁決：「先這樣做，未來再回頭修改，這些通知雖然忠於
    //     翻譯，但不是最急著處理的」。**這是刻意偏離 golden** —— golden 的
    //     kcode==0 是一則要操作員按鍵消掉的 note，它會擋住機台。要回頭補的是
    //     「通知型對話框的非阻塞確認通道」，不是把這裡改回去等。   [AI(W906-J5-ACK) 20260930: that channel exists now -- WS dialog.notifyAck, tag = this qid (the mailbox requestId): W906_NoticeAckCommand (EOF) retires the notice and applies golden's close, BtnPauseClick's KeyCode==0 arm + FormClose (forms/fNote_ShowError.cpp EOF).  This branch still returns at once]
    if (kcode == 0) {
        const bool noteOnly = DialogMailboxPostAlarm(qidStr, code, kcode, pos);  { extern void W906_NoteNoticeCapture(const char*, bool); W906_NoteNoticeCapture(qidStr, g_w906MotorNoteMsg != 0); }   //AI(W906-J5-ACK) 20260930: keep what golden FormClose reads from fNote (code / AlarmType / iDuplicateError / iEventID) for the ack -- forms/fNote_ShowError.cpp EOF
        // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 事件通道留一筆 =====
        //   ⚠ blocking=false，而且**馬上補一筆 clear** —— 這一則不進等待迴圈
        //   （AI(W906-Q30-KZERO)），沒有人會回答它，留著 raise 會讓
        //   alarm.active 永遠遞增。退役者是 C++ 自己，所以 action 是空字串。
        //   ⚠ EmitAlarm/ClearAlarm 都是純記憶體 append，不阻塞。
        ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowErrorMessage,
                                 code ? code : "", kcode, pos, qidStr, false);
        ht9045::sjson::ClearAlarm(qidStr, "", "", 0);
        // ===== AI(W906-SJSON-S10) 20260923 END =====
        std::printf("query qid=%s code=%s kcode=0 -- NOTE, not blocking (mailbox=%s)\n",
                    qidStr, code ? code : "", noteOnly ? "yes" : "no");
        std::fflush(stdout);
        return 0;
    }

    g_modalServer->PostQuery(qid, code ? code : "", kcode);
    // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 阻塞式警報的 raise =====
    //   放在 PostQuery 之後、等待迴圈之前：這一刻「警報已經送出去了」是事實。
    //   放在迴圈裡會每 100 ms 記一筆，放在函式最後就只剩解除沒有發生。
    ht9045::sjson::EmitAlarm(ht9045::sjson::kSrcShowErrorMessage,
                             code ? code : "", kcode, pos, qidStr, true);
    // ===== AI(W906-SJSON-S10) 20260923 END =====

    // AI(W906-Q30-8) 20260922: 同一則警報**同時**走兩條路 ——
    //   信箱（同事那套現成的 HMI，會在瀏覽器重開後再彈）與
    //   WebSocket（我們自己的，Q30 第 3 題做的重連補發）。
    //   ⚠ 先到的算數；下面的等待迴圈兩種回應都收。
    const bool posted = (kShown != 0)
                      ? DialogMailboxPostAlarm(qidStr, code, kcode, pos)
                      : false;
    if (!posted && !g_dialogMailboxDir.empty() && (kShown != 0))
        std::printf("  ⚠ dialog mailbox 寫入失敗 —— 只剩 WebSocket 那條路\n");

    std::printf("query qid=%s code=%s kcode=%d -- waiting (mailbox=%s, ws=yes)\n",
                qidStr, code ? code : "", kcode, posted ? "yes" : "no");
    { const DWORD t3 = ::GetTickCount(); char w906b[192]; std::snprintf(w906b, sizeof(w906b), "%s stop %lu ms, record %lu ms, auth+post %lu ms, total %lu ms before the box was posted", code ? code : "", (unsigned long)(w906AlmT1 - w906AlmT0), (unsigned long)(w906AlmT2 - w906AlmT1), (unsigned long)(t3 - w906AlmT2), (unsigned long)(t3 - w906AlmT0)); std::printf("[ALARMTIME] %s\n", w906b); std::fflush(stdout); OpLine("ALARMTIME", w906b); }   /* OpLine: the anonymous-namespace op log (declared on ForwardShowErrorMessage's line) */   //AI(W906-ALARMTIME) 20261007: its own line (the "query qid=" line above is parsed by tools/webprobe); replaces a blank line
    WdMark2("modal wait (blocking, needs a browser answer): ", code ? code : "");  // AI(W906-WD) 20260923
    std::vector<webbridge::WebCommand> local;  W906ModalWaitScope wakeScope(0, code);  w906dlg::WaitTagScope s17Wait(qidStr);   /*AI(W906-S17) 20261003 St01: this wait holds qidStr (tools/wb_dialog_mailbox.h WaitingTags)*/   // AI(W906-MODAL-WAKE) 20260926: golden TfNote::FormShow／FormClose 的 fShow、蜂鳴器、Alarm Reset 燈（檔尾）
    for (;;) {
        if (g_carry.empty()) { extern bool W906_FastClockModalWait(webbridge::CommandQueue&, unsigned long); W906_FastClockModalWait(*g_pumpQueue, 100); }  /* AI(W906-FASTCLK-MODAL) 20261003: was g_pumpQueue->waitForPush(100) -- the same wait (a command wakes it, 100 ms at most), sliced at the fast clock's due jobs so the heater / panels keep their period while the box is up (E-FT1-001; FastClockWbServe.cpp) */  { extern void DoAvoidIndexMotorFallDown(); DoAvoidIndexMotorFallDown(); }  W906_ModalWaitTick(0, kcode);  { extern void W906_TesterCommPoll(); W906_TesterCommPoll(); }  /* AI(W906-GB-P3) 20260926: H5 -- golden ShowModal keeps handling WM_COPYDATA from the bridge */   // AI(W906-IOWEB-P25c) 20260925: was Sleep(100) -- an answer or a motor.stop now wakes the wait at once (still bounded to 100 ms; laptop review Q3-3)  //AI(W906-MODAL-SAFETY) 20260925: golden 在 ShowModal 期間 MyMessageBox::Timer1Timer（mymessbox.cpp:538-550／:650）照跑 DoAvoidIndexMotorFallDown（EMG／斷電／Index Z servo off ⇒ 鎖 Index 煞車並停機）；移植樹等待期間 tick 停住，這裡補跑（R-YESNO 審查 high；本體不跳框、SOFT_SIMULTE 下是 no-op）  //AI(W906-MERGE-56bbf785) 20260926: machine P25c wanted the event wait instead of Sleep(100), laptop MODAL-SAFETY wanted DoAvoidIndexMotorFallDown on every pass -- both kept on this one line. The check now also runs on each early wake; golden's Timer1Timer runs it more often than every 100 ms anyway. ⚠ On HT9050 the IO it reads comes from the 1203 monitor's samples, and Poll() does not run inside this wait, so it sees the state from before the wait (same for W906_AlarmIoAnswer below)
        local.clear();
        W906_TakeCarry(local);  g_pumpQueue->drain(local);   // AI(W906-IOWEB-P25) 20260925: carried commands first (a dialog answer the output service moved into g_carry must still reach this wait)
        for (size_t i = 0; i < local.size(); ++i) {
            const webbridge::WebCommand& wc = local[i];
            // AI(W906-Q30-8) 20260922: 兩種回應指令都收。
            //   `modal.answer`    -- 我們自己的 WebSocket 路（20260921）
            //   `dialog.response` -- 同事那套 HMI 經 window.HTDialogHost 送回來的
            //   兩者的 `tag` 都是 qid/requestId，`value` 是動作名。
            //
            //   ⚠ `dialog.response` 的 value 允許 `"<ACTION>:<pressedButton>"`，
            //     例如 `"RETRY:BtnStart"`。理由是**契約層面的**：
            //     `selectedAction.code` **分不出 Start 還是 Pause**
            //     —— 那個 code 由先點的選擇鍵決定，Start/Pause 只是「送出」
            //     （dialog-bridge.js:297 / dialog-page.js:75-76）。
            //     golden 的 fNote 靠 pressedButton 決定後續 SoftStart / SoftStop，
            //     所以這個資訊不能丟。冒號後面沒東西就是 null，與契約一致。
            if ((wc.cmd == "modal.answer" || wc.cmd == "dialog.response")
                && wc.hasTag && wc.tag == qidStr) {
                std::string ans = (wc.hasValue && wc.value.isString())
                                  ? wc.value.asString() : std::string();
                std::string pressed;
                const std::string::size_type colon = ans.find(':');
                if (colon != std::string::npos) {
                    pressed = ans.substr(colon + 1);
                    ans     = ans.substr(0, colon);
                }
                // AI(W906-Q30-KMAP) 20260921: **補滿 9 個** —— 與
                //   `WebBridge/WebBridgeServer.cpp` 的 `PostQuery` 對稱。
                //   兩邊必須同時補：只補送出側，瀏覽器送回一個合法選項會在
                //   這裡被判成 0 而落到下面的 `not an offered option`，
                //   迴圈繼續 —— 症狀與「沒補」一模一樣，但更難查。
                //
                //   清單與 golden `note.cpp:1235` 的 `KeyComp[]` 逐項相同。
                //   ⚠ 不含 `K_FIX`（golden 註解「kevin 20130722 cancel K_FIX」）、
                //     也不含 `K_PAUSE` / `K_START`（不在 `KeyComp[]` 裡）。
                //   ⚠ 這裡用具名常數（`cmydef` 的 `K_*`）而不是字面值 ——
                //     這支 TU 拿得到它們，不必像 WebBridgeServer 那樣硬寫數值。
                int k = 0;
                if      (ans == "SKIP")      k = K_SKIP;
                else if (ans == "RETRY")     k = K_RETRY;
                else if (ans == "TRAY_FEED") k = K_TRAY_FEED;
                else if (ans == "TRAY_END")  k = K_TRAY_END;
                else if (ans == "CLEAN_OUT") k = K_CLEAN_OUT;
                else if (ans == "RESET")     k = K_RESET;
                else if (ans == "HOME")      k = K_HOME;
                else if (ans == "TRAIN")     k = K_TRAIN;
                else if (ans == "ONECYCLE")  k = K_ONECYCLE;  if (k != 0 && (k & kcode) != 0 && pressed == "BtnStart" && !W906_NoteScreenStartActs()) { const std::string ssWhy = "note-start-ignored: golden 這台畫面上的 START 不做事（note.cpp:3806-3824 BtnStartClick 只有 SIGURD_PeiXing／模擬組態會 Start）—— 選好鍵請按 PAUSE，或按面板的 START 鍵"; g_modalServer->CompleteCommand((unsigned long long)wc.id, false, ssWhy); std::printf("query qid=%s: %s:BtnStart refused (golden BtnStartClick does nothing on this machine)\n", qidStr, ans.c_str()); continue; }   /*AI(W906-NOTE-SCREENSTART) 20261002: EastSun「Bcb上怎做你就怎做」-- before the key gate: golden BtnStartClick never reaches the key checks*/  if (k != 0 && (k & kcode) != 0 && k != K_RESET) { std::string kgWhy; if (!W906_NoteWebKeyGate(g_w906NoteChamberDoorPtr ? *g_w906NoteChamberDoorPtr : false, &kgWhy)) { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, kgWhy); std::printf("query qid=%s: %s refused (%s)\n", qidStr, ans.c_str(), kgWhy.c_str()); continue; } }   /*AI(W906-NOTE-KEYGATE) 20261002: golden TfNote::BtnSkipClick note.cpp:2843-2870 + UpdateButtonStatus :2764-2873 -- a key chosen on the screen is ignored while an IC fell into the test site / contact over / auto-clean door / clean pad / safe lock / index jam door / auto-retest door, as the physical keys already are (W906_AlarmIoAnswer). BEFORE the D-026 password gate on the next line (golden: a locked key cannot be chosen, so DoPassword never runs for it -- and the gate there uses up the one-shot pass). RESET (BtnResetClick) not gated; refused = the wait goes on*/
                if (k != 0 && (k & kcode) != 0) {  { extern bool W906_NoteAuthAnswerGate(const char*, int, const std::string&, std::string*); std::string noteAuthWhy; if (!W906_NoteAuthAnswerGate(qidStr, k, pressed, &noteAuthWhy)) { g_modalServer->CompleteCommand((unsigned long long)wc.id, false, noteAuthWhy); continue; } }   /*AI(W906-D026) 20261001 St01: golden TfNote::Start() :3599-3607 / BtnPauseClick() :3921-3929 -- DoUnlockPassword / DoPassword must have returned true (the dialog.auth pass) before the note closes; otherwise refused and the wait goes on (golden: return). Same line, no line moves*/
                    g_modalServer->CompleteCommand((unsigned long long)wc.id, true, std::string());
                    // AI(W906-Q30-REPLAY) 20260921: 問題已經被回答 ⇒ 清掉待答狀態。
                    //   ⚠ 一定要在 `return` 之前。不清的話，下一個連上來的瀏覽器
                    //     會收到這個早就答完的警報框，而且它回的 `modal.answer`
                    //     會被判成 `no query pending` —— 一個關不掉的框。
                    //   ⚠ 這是這個函式**唯一**的正常出口（另一個是最上面
                    //     `!g_modalServer || !g_pumpQueue` 那個 early return，
                    //     而那條路根本沒有 PostQuery 過，沒有東西要清）。
                    g_modalServer->ClearQuery(qid);
                    // AI(W906-Q30-8) 20260922: ⚠⚠ 信箱那一側也要退役。
                    //   不做的話：操作員答完之後按 F5，dialog-bridge 的
                    //   lastSeq 歸 0、request 還是 pending ⇒ **同一個警報再彈一次**。
                    //   而且只有重整才看得到，平常測不出來。
                    DialogMailboxRetire();
                    // ===== AI(W906-SJSON-S10) 20260923 BEGIN -- 解除那一筆 =====
                    //   requestId 與 raise 同一個 qidStr，兩筆才配得起來。
                    //   pressed 可能是空字串（modal.answer 那條路沒有它）——
                    //   空 = 不知道按的是 Start 還是 Pause，序列化成 null。
                    ht9045::sjson::ClearAlarm(qidStr, ans, pressed, k);
                    // ===== AI(W906-SJSON-S10) 20260923 END =====
                    std::printf("query qid=%s answered %s (K=%d) via %s%s%s\n",
                                qidStr, ans.c_str(), k, wc.cmd.c_str(),
                                pressed.empty() ? "" : " pressed=",
                                pressed.c_str());
                    // AI(W906-DLG-PAUSE) 20260923 夜間：告警框上的 Pause 照 golden 讓機台暫停。
                    //   golden note.cpp:3980-3982 TfNote::BtnPauseClick：
                    //       ReturnCode=KeyComp[i]; SoftStop=true; SoftStart=false;
                    //   對照按 START（BtnStartClick → TfNote::Start()，:3665）只做 ReturnCode=KeyComp[i]  [⚠ 20260924 更正：這句不完整 —— golden :3669 接著還有 fMain->Start("fNote::Start 1") 與 :3677 SendCommand_ESD(ESD_SYSTEM_START)，現在照做，見下面 return k; 那行]
                    //   —— SoftStart／SoftStop 那兩行在 golden 本身就被註解掉（:3667-3668），所以 START 這邊不加任何東西。
                    //   在這之前 pressed 只印進 log、沒有人消費（8e7809d 的訊息自己也寫「沒人消費它」），
                    //   於是告警框按 Pause 的效果等於按 START。20260923 夜間 T8 從畫面實測（無頭 Edge 點 iframe 裡的
                    //   #BtnRetry 再點 #BtnPause）：回答送到了（log「answered RETRY ... pressed=BtnPause」、無 [object Object]），
                    //   但按後 3 秒／10 秒 SystemStart 仍是 1。
                    //   ⚠ 只認 "BtnPause"（dialog-page.js 的 fNote 路徑送的就是這個字）；

<!-- preserved-content:end -->
```
