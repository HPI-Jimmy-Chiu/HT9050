# 機型、版本與完整生命期界線

同題整合 HT9050 與其他 Handler，沿用 [機型樹](../../../../../machines/index.md)。六個選讀 cpp body 沒有 MachineTypeChoice／CustomerCode 條件；只證明這些段落的共同符號，不代表全機型配置、tester protocol 或客戶 parser 相同。

- ReaderProc_ 與 Impl 的硬體資源段有 Win32 編譯分流；SIM 的直接 callback 與 reader 的呼叫場景分開。
- 客戶預設與 recipe framing 仍見 [Aux consumer](../tick-consumer/consumer.md)；port tag 及 sender 區分 AMD／AMD2／Aux，不合併成同一訊息格式。
- 本層是 V906 UTF-8 pin；V912 Big5 counterpart、所有 receiver／button event、rxQueue writer 與 ABI 尚未完整重驗。
- QueueRx 保存複本只閉合輸入 buffer 的這一段生命期；不表示 SerialPoll、mailbox、fRS232Main、CommTester 或 callback owner 的全生命期已安全。
- 呼叫成功、排入佇列、event 觸發、parser 返回與 handler／tester 已收到，是不同證據；不能換算成已量到 UPH。

完整 TIniFile 保存 callee、接收 parser、writer／reader shutdown 與錯誤路徑、容量／ABI、實機 UPH 及 S8 語意接續。原文、metadata、資源及裁決保留；活摘要以 function／欄位定位，歷史註解不升格為本輪實機驗證。
