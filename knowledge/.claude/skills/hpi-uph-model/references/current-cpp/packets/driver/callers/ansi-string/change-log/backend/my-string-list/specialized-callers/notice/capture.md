# Capture 保存哪些值，哪些仍是 live

[上層](index.md)；[原文與 hash](source-manifest.json)。

`wb_serve.cpp::ForwardShowErrorMessage` 的 kcode==0 路徑先
DialogMailboxPostAlarm，再 W906_NoteNoticeCapture(qidStr, g_w906MotorNoteMsg!=0)。
本輪保留此呼叫區段；沒有保存該大型父函式整體，或證明全部入口／thread ownership。

## 單槽 snapshot

`forms/fNote_ShowError.cpp::W906NoticeNote` 與 W906_Notice 是本檔單一狀態。
Capture 設 valid=true、保存 requestId（NULL 轉空字串）、code、alarmType、
duplicateError 與 eventId；fNote 為 NULL 時 code 為空、alarmType 為 0。
motorNote 為 true 時 goldenNote／answerApplied 都為 true；
其他情況 goldenNote 讀 W906_ShowErrorMessage_Recorded，answerApplied 為 false。
它沒有先檢查 requestId 是否非空，也沒有依 noteOnly 回傳值決定是否保存。

後一次 Capture 覆寫同一 W906_Notice；Ack 用 valid 加 requestId 相等判斷是否 mine。
被覆蓋的 snapshot 不會由這三個函式再做原 notice 的 close。
mailbox 與 fNote snapshot 是兩份狀態，不能以其中一份吻合推定另一份也吻合。
其他 IniConfig、CUSTOMER_CODE、LastSet、全域 flags 在 Ack 讀取當時的值，
不是全部在 Capture 複製。選定三函式沒有鎖；完整執行緒／重入條件仍待查。

## 計時器重設不等於 PassTime 歸零

goldenNote 為 true 時 Capture 呼叫 W906_tNoteTimer.LatchCycleTimeSec(true)，
並將 Recovery 清空；它沒有對本檔 static DWORD PassTime 賦值。
這份檔案的詞法遮罩查找中，PassTime 的直接賦值在 AckLikeGolden：
`PassTime=W906_tNoteTimer.LatchCycleTimeSec()`；計時器如何計算／轉型仍需續查。
因此不能由「Capture 啟動計時」推論 post 時寫入的 PassTime 每次必為 0；
前一次 Ack 的值可能仍保存在該 static 儲存，這是靜態條件推論，未做連續通知實測。

原 banner「post 的 StopedTime 0」與歷史 golden／ctest 註記完整保留；
活文件以上述現行賦值與 caller 的讀取位置為依據。
前一段 ShowMotorErrorMessage 在 hook 後組欄位，並未等待操作員回覆；
Ack 呼叫 MyDBUEventRecover，不會在這個正文回改已寫入的 EventLogTxt 那一列。
事件 DB recovery、文字 log 與操作員停留時間必須分開核對。
