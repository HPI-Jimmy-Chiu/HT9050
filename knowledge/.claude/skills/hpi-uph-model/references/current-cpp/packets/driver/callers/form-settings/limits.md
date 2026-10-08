# 機型、版本與保存內容

同題整合 HT9050 與其他 Handler，共同入口沿用 [機型樹](../../../../../machines/index.md)。七個選讀 cpp body 沒有 MachineTypeChoice／CustomerCode 分支；只表示這些段落共用 constructor、映射及 transport 符號，不能推成全部機型或客戶配置相同。

- 客戶預設仍在 [Aux consumer](../tick-consumer/consumer.md)，不因本層沒有條件就刪去客戶差異。
- Windows 的 DCB 套用與非 Windows 空 body 分開；SIM、COM 探測與 stop 界線仍見 [transport](../tick-consumer/transport.md)。
- V906 UTF-8 為本層 pin；V912 Big5 的量產 counterpart 與 golden 註解並未重新驗證。
- COM13／COM3、timeout 0／1／70 是不同來源階段的初值，不是 HT9050 現場參數或所有 Handler 的共同設定。

Comm.h 原註解的 consumer 數量、只在 Start 前設欄位、906 不會 runtime new TComm 等保留為歷史敘述。本次 Aux constructor 的完整 body 明確含 `new TComm(0)`；活文件依目前來源，不將歷史註解當成全 caller 已重驗。

原文／header／metadata／裁決保留，cpp／h 行號只出現在保存的歷史註解中。摘要以檔名、function、欄位定位。完整 ownership、button／INI caller、容量／ABI、V912 矩陣、實機 UPH 與 S8 語意仍待續。
