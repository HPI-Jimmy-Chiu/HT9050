# 同題機型、版本與未查範圍

| 分流 | 本層證據 | 仍未查證 |
| --- | --- | --- |
| V906共用 | 正式Init factory、tracked InjectDriver文字普查、兩Sim fixture與IPC替身、建置宣告 | Init的前置callee、實際TestType選擇／外部注入、例外／重入／跨thread壽命、完整建置配置 |
| HT9045／客戶910 | 兩fixture寫9045GPIB／HT-9045／910，正常路徑有shutdown／清注入 | 實際客戶版本、runtime參數／通訊與完整global還原 |
| HT9050／其他Handler | TestEndToEnd局部9050 enum切換只含連線查找斷言；正式Init選讀body無機型factory分支 | 9050 Model／site／容量、完整流程與其他機型部署矩陣；沿 [機型樹](../../../../machines/index.md) 續查 |
| V912 BCB6 | 同題 [封包版本](../../versions.md) 保留差異 | 本單元未查V912對應factory／driver／測試鏈 |
| NI／Sim | [NI／Sim實作](../implementations/index.md)、[Start選擇](../lifecycle/selection.md) 與測試注入銜接 | NI DLL ABI／硬體卡／Tester收件、實機UPH／校正與完整producer／consumer |

來源註解的歷史判斷、CHECK中的預期結果與本次靜態查證分開標示；沒有執行程式或改runtime，不能因測試存在就宣稱已通過。

本單元只縮小factory與已選測試caller的缺口，不宣告整體UPH或S8已完成。回 [本層索引](index.md)、[driver索引](../index.md)。
