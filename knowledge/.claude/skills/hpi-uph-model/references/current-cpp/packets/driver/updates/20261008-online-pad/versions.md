# 版本、機型、客戶與剩餘

這一節固定在 V906 C++ pin `37a048f1ce5a9c3128cf75d69d579b602afce918`。新 `ibonl`、LastPrimaryAddress／padSaid欄位與頁面hook的差異依當前定義查證，舊節點的source manifest、metadata、原文及body hash都保留。

| 面向 | 共用項／差異及界線 |
| --- | --- |
| V906 driver／engine | 本單元選定定義的共用wrapper、NI／Sim、解除與觀測程式條件；沒有完整caller、ABI／DLL實測或排程／UPH效能結論 |
| HT9050與其他Handler | 這些共用元件不能直接證明任一機台實際進入相同路徑；仍須核有效TestType、配置、Init與機台身分。fixture只寫9045GPIB／HT-9045 |
| 客戶 | Close函式有CC_MTI／CC_PTI的lot-status清零條件；fixture有910。其他客戶、V912分支與實際格式未窮舉 |
| golden／V912 | 註解的獨立exe釋放、CreateProcess與先前故障是歷史敘述。本單元沒有重新對照golden或V912，不主張等價、適用出貨版或已修復客訴 |
| runtime／HMI | 靜態wrapper記錄、頁面hook與fixture斷言不等於board狀態、封包送達、畫面顯示或實機UPH量測 |

下一步：BuildUiSnapshot的新位址欄位／WebBridge發布鏈與hook安裝／消費；ProcessAddress與完整Restart／Close caller、外部driver及ABI、其他客戶／機型；INI字串／TStrings、容量與實機UPH/S8。新增來源更動先比pin與function／變數，保留每一代原文。

回 [版本入口](index.md)、[舊NI／Sim版本界線](../../implementations/versions.md) 與 [handler選路](../../callers/init-selection/index.md)。
