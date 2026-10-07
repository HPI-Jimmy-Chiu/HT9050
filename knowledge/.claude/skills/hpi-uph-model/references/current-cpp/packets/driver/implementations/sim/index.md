# Sim driver

`gpibbridge::SimGpibDriver` 的十五個 cpp 定義與 header class 原文在 [manifest](../source-manifest.json)，包括三個 inline 狀態 getter。這是來源的腳本介面，沒有執行模擬測試。

- [狀態與控制](state.md)：Recompute、FindFails、ibwait、pad_／tmo_。
- [讀寫路徑](read-write.md)：ibrd 如何取佇列，ibwrt 如何依 talk_ 返回。
- [佇列](queues.md)：TesterWrite、TakeBridgeWrites、TakeSrqBytes、ibrsv。

現場 UPH、真實 GPIB 等待／送達不能由 Sim 的返回值推定；看 [版本／機型](../versions.md) 及 [未查界線](../limits.md)。回 [兩實作索引](../index.md)。
