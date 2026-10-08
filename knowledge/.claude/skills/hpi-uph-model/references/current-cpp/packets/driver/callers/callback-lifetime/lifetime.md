# TComm 與 Impl 的釋放

定位 vclcompat/Comm.cpp 的 `TComm::~TComm()` 與 `struct TComm::Impl`，全文保存在 [manifest](source-manifest.json)。這是靜態來源查證，不是物件生命期或執行緒測試。

TComm destructor 先呼叫 StopComm，再 delete pImpl_／清指標。這個 body 沒有在 delete 之前額外確認 reader／writer／callback 已退出；StopComm 的有限 wait 與未檢查結果見 [transport](../tick-consumer/transport.md)。

## Impl 初值與資源

- 共用欄位 bSim、bSimForced、bOpen 初值皆 false；simTx 是保存傳送資料的 vector。
- Win32 欄位 hFile 初值 INVALID_HANDLE_VALUE；reader／writer／event／closer handles、self 與停止旗標初值為 0。
- Impl constructor 的 Win32 body 初始化 csTx，再建立 manual-reset evStop，初值未觸發；這個 body 沒有檢查 CreateEventA 的回傳值。
- Impl destructor 的 Win32 body 先呼叫 W906_CommJoinCloser(hCloser)，evStop 非零時 CloseHandle，再 DeleteCriticalSection(csTx)。非 Win32 不編入這些操作。

Impl destructor 沒有另外 join reader／writer；其依賴 TComm 的 StopComm。closer helper 的有限 wait／結果界線已在 [transport](../tick-consumer/transport.md) 保存。原註解的「port 在物件離開前已關閉」不能取代所有 wait 結果與 callback 生命期的完整證據。

本單元補齊 Impl 宣告及兩個 inline constructor／destructor body 的原文，統計為一個完整 struct 快照；不把它重算成兩個獨立 cpp 交付函式。
