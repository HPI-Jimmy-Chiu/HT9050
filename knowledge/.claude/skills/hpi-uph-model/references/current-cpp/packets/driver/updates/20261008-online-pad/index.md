# V906 1008 main：GPIB 解除、位址觀測與頁面通知

固定來源 `37a048f1ce5a9c3128cf75d69d579b602afce918`；這是舊 wrapper／實作／生命週期節點之後的新版本差異，舊節點仍按各自 pin 閱讀。
8 個 UTF-8 來源、21 個完整 cpp 定義、1 個 header inline、4 個 class 宣告與3組宣告／macro，共29片段保存於 [manifest](source-manifest.json)。只做靜態來源查證。

| 問題 | 路由 |
| --- | --- |
| board 解除、optional NI export、Sim 記錄 | [解除順序](release.md) |
| last successful ibpad、未知值、重複訊息抑制 | [位址觀測](address.md) |
| bridge transition 與 HMI open/close hook | [頁面通知](pages.md) |
| 現有測試寫了哪些斷言、哪些路徑會 skip | [fixture 界線](fixtures.md) |
| HT9050／其他 Handler、客戶、golden 與 runtime | [版本與剩餘](versions.md) |
| 18項舊 manifest 來源變動：9項片段變、9項片段不變 | [差異清冊](drift-review.md) |

## 固定來源

- [GpibDriver.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.cpp)，blob `f5c78f8327278bdf1f033e64fca0b27f399f4889`。
- [GpibEngine.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibEngine.cpp)，blob `1eac77790f876816215a8a5c9181b516513601cf`。
- [HandlerTesterSide.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerTesterSide.cpp)，blob `dbc9fba0d8ccadf8efc0c8a36771e5ffdc392cd4`。
- [HandlerBridgeCtl.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Handler/HandlerBridgeCtl.cpp)，blob `0141ec1ad8e73fe9c76e2260606a3c467765f3da`。
- [test_testercomm_gpib.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/tests/test_testercomm_gpib.cpp)，blob `0492009e9ae4b819123bd52baa348162c39e03d2`。
- [W906FormShowing.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/W906FormShowing.h)，blob `08e68c83463725ca5742ad19ce8bd3a63a0190c1`。
- [GpibDriver.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.h)，blob `ea38ba9acd491bbc8e2868b6136bf26be505970a`。
- [GpibEngine.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/37a048f1ce5a9c3128cf75d69d579b602afce918/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibEngine.h)，blob `e9d0943906cf09405793e3154187795c1d667d85`。

回 [driver 版本入口](../../index.md)。原文中的量測、golden/J-18/J-19 與註解意圖是該版本的歷史敘述，本段沒有重現量測、載入 DLL、執行 fixture 或操作機台。
