# V906 NI／Sim driver 局部來源

兩種實作保留在同一份 UPH Skill。pin `ef03a121441d7695d1461ab4f739c6348640e828`；兩個 UTF-8 來源、NI 14／Sim 15 個完整 cpp 函式、兩個 class 宣告及宏／export 表保存於 [manifest](source-manifest.json)。只做靜態判讀，沒有載入 DLL 或執行 driver。

| 要確認的問題 | 路由 |
| --- | --- |
| DLL 載入／失敗、fn_ 與卸載 | [NI 載入](ni/load.md) |
| NI 呼叫、返回值及狀態來源 | [NI 索引](ni/index.md) |
| Sim 佇列、讀寫、狀態保留 | [Sim 索引](sim/index.md) |
| V912、HT9050／其他 Handler 與客戶 | [共用與差異](versions.md) |
| caller、生命週期、ABI 與送達未查部分 | [查證界線](limits.md) |

來源：

- [GpibDriver.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ef03a121441d7695d1461ab4f739c6348640e828/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.h)，blob `131b757ba1d964fd8e9e1cbf485015f8da4f60fc`。
- [GpibDriver.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/ef03a121441d7695d1461ab4f739c6348640e828/HT9011UC_Cpp_V3.33.906.0/TesterComm/Gpib/GpibDriver.cpp)，blob `9ded9361c8b8569c0207ddefe73dbe4c2d4d9b83`。

回 [wrapper 指標／狀態](../index.md) 與 [封包宣告](../../index.md)。原始註解與選讀原文完整保留，註解所稱 golden／單一執行緒行為不是本輪的實機或完整 caller 驗證。
