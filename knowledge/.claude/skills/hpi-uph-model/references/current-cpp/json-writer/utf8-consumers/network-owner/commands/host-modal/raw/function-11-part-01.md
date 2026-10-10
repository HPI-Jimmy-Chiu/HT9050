# 原文：DialogMailboxPostAlarm（1）

[上層](../index.md)／[manifest](../source-manifest.json)。來源 `HT9011UC_Cpp_V3.33.906.0/tools/wb_serve.cpp`，固定pin `b187b85fbbdc5167338ae886c8845ac775d97b37`。
以function／變數定位；offset與SHA僅固定pin保存證據。本頁payload 35行；context credit0，重疊內容不重計。

```cpp
<!-- preserved-content:start -->
static bool DialogMailboxPostAlarm(const std::string& requestId,
                                   const char* code, int kcode, int pos)
{
    if (g_dialogMailboxDir.empty()) return false;
    //AI(W906-J5-ACK) 20260930: `++g_dialogSeq`, the JSON and the slot are w906dlg::AlarmPost's now (below); g_dialogAlarmSeq follows g_alarmSlot.seq

    // AI(W906-Q30-8) 20260922: `blocking` **不是常數**。
    //   同事 20260922 早上提出第三種形狀 [P32]（`blocking:false`），
    //   我去 golden 驗過，他是對的而且三棵樹一致：
    //     V899 acatchtray.cpp:4897 / V906 :5382 / V912 :5655
    //     都是 `iUnLoaderCount=8;  // 必須不為0 Handler才不停機`
    //     然後走 `ShowUnloaderTrayMessage()` —— 本體最後是 `Show()` 不是
    //     `ShowModal()`，所以它真的不阻塞、也不 StopAllMotor。
    //   ⇒ `ShowErrorMessage` 這條路恆為 true（golden note.cpp:795-798 兩個
    //     分支都 StopAllMotor，停機不是可選項）；
    //     但 `ShowUnloaderTrayMessage` 接上來的時候必須是 false，
    //     否則 C++ 會去等一個不該等的東西 ⇒ 機台停在那裡不動，
    //     跟 [P32] 要的「不停機」正好相反。
    //   ⇒ 參數化，不要讓下一個人以為它是常數。
    const bool blocking = true;   // ShowErrorMessage 專用；NonStop 那條要傳 false
    //AI(W906-J5-ACK) 20260930: the two snprintf buffers that were here (head[256] / args[192]) became
    //   w906dlg::AlarmRequestJson (tools/wb_dialog_mailbox.h): the same text -- tests/test_notice_ack.cpp [A] compares
    //   it with a verbatim copy of them -- without their silent truncation.  w906dlg::AlarmPost also keeps g_alarmSlot
    //   (kNotice when kcode==0, kBlocking otherwise); WS dialog.notifyAck (EOF W906_NoticeAckCommand) retires a notice only.
    //   ⚠ `buttons` 是**死欄位** —— dialog-page.js 完全不讀它，
    //     按鈕是由 arguments.kCode 的位元驅動的。填了也不影響行為。
    extern std::string W906_NoteAuthRequestJson(const char*);   /*AI(W906-D026) 20261001 St01: the request's "auth" object = golden TfNote::DoPassword / DoUnlockPassword would ask for this note (WebLogin.cpp EOF; "" = today's required:false bytes). Same line, no line moves*/  extern std::string W906_AlarmMsgWithErrPart(const char*);  extern void W906_AlarmTextForPost(const char*, const char*, const std::string&, std::string&, std::string&, std::string&); std::string w906Msg, w906Desc, w906Key; W906_AlarmTextForPost(code, g_w906MotorNoteMsg, W906_AlarmMsgWithErrPart(code), w906Msg, w906Desc, w906Key);   /*AI(W906-W142) 20261007 (St02-E): golden ShowMessageEdit1 + description for this request (forms/fNote_ShowError.cpp EOF)*/  const bool posted = w906dlg::AlarmPost(g_alarmSlot, g_dialogMailboxDir, g_dialogSeq, requestId,
        code ? code : "", kcode, pos,
        g_w906MotorNoteUnit ? g_w906MotorNoteUnit : "",                        //AI(W906-JAM-STOP) 20260930: a motor note carries golden's unit alias (MOT[UnitNo].Alias); every other alarm keeps "" (unchanged)
        w906Msg,         //AI(W906-JAM-STOP) 20260930: a motor note carries golden's MyDBIEvent Message (ShowMessageEdit1); every other alarm keeps the code (unchanged)   //AI(W906-ERRPART) 20261006: + golden's " : "+errPart (note.cpp ErrShowToForm, forms/fNote_ShowError.cpp:408) -- EOF   //AI(W906-W142) 20261007 (St02-E): was `g_w906MotorNoteMsg ? std::string(g_w906MotorNoteMsg) : W906_AlarmMsgWithErrPart(code)` -- both still apply, inside W906_AlarmTextForPost
        blocking, W906_NoteAuthRequestJson(requestId.c_str()), w906Desc, w906Key);   //AI(W906-D026) 20261001 St01: + the "auth" object (see the first line of this call)   //AI(W906-W142) 20261007 (St02-E): + display.description / descriptionKey
    { extern unsigned long long g_dialogAlarmSeq; g_dialogAlarmSeq = g_alarmSlot.seq; }   // AI(W906-SMM-IO) 20260925: IO 解除時 Dialog-close-request 要對到這一則（檔尾 AI(W906-SMM-IO)）
    return posted;
}


<!-- preserved-content:end -->
```
