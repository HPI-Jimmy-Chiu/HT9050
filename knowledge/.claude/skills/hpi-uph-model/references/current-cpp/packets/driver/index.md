# V906 GPIB wrapper 的指標與狀態

pin `55c14cf1e07ce2ab6b8e77e78fafef7bd08a4cd7`；兩份 UTF-8 來源、五個完整 cpp 函式文字／body hash、六組宣告／初值與十一原文片段見 [manifest](source-manifest.json)。本單元只查 wrapper 與指標／狀態 helper。

- [狀態刷新與返回](status.md)：`NoCard`、`Refresh`、`gpibbridge::ibwrt`。
- [指標設定與讀取](pointer.md)：`g_driver`、`SetGpibDriver`、`GetGpibDriver`。
- [版本／機型與剩餘](limits.md)：V912、driver 實作、生命週期、送達與實機待查。

固定來源：

- [GpibDriver.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/55c14cf1e07ce2ab6b8e77e78fafef7bd08a4cd7/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.h)，blob `131b757ba1d964fd8e9e1cbf485015f8da4f60fc`。
- [GpibDriver.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/55c14cf1e07ce2ab6b8e77e78fafef7bd08a4cd7/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.cpp)，blob `9ded9361c8b8569c0207ddefe73dbe4c2d4d9b83`。

回 [封包宣告](../index.md) 與 [Command consumer](../../consumers/command/index.md)。原註解完整保存，來源的歷史敘述不當作本輪 runtime 驗證。

## 後續實作局部查證

NI／Sim的來源返回、DLL／佇列與狀態保留分流見 [實作樹](implementations/index.md)；此路由不將前述wrapper單元擴張成全部caller或實機驗證。

## Driver連接與解除

Start讀取注入driver、未注入時嘗試NI，及Teardown的本地解除順序見 [建立／解除](lifecycle/index.md)；全caller／建置與機型選擇仍待查。

## Factory與測試注入

正式Init、兩個Sim注入fixture、IPC替身與建置宣告見 [呼叫端與測試界線](callers/index.md)；測試斷言與實際執行結果分開，完整機型／ABI／runtime仍待查。

## 1008 main來源差異

上列節點均保留其固定pin。新main新增board解除、成功設定位址觀測及bridge頁面通知，續讀 [1008版本差異](updates/20261008-online-pad/index.md)；該節點另核18項舊manifest來源變動，不把歷史註解或fixture文字當本輪runtime結果。

## 1008快照與頁面消費

GPIB快照走HTTP，程式頁面開關走ui.pages tag；來源、renderer未直接使用raw欄位及入列／執行界線見 [快照／頁面樹](updates/20261008-ui-page/index.md)。舊pin／正文不改，完整caller與runtime仍待驗。
