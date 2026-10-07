# NI driver

`gpibbridge::NiGpibDriver` 的 ctor、dtor、九個 ib* 及 Status／Error／Count 共十四個 cpp 定義，完整文字與 hash 在 [來源紀錄](../source-manifest.json)。

- [載入與卸載](load.md)：module_、fn_[12]、kNames、Loaded。
- [呼叫與返回](calls.md)：ibfind 的特殊失敗值、NI_CALL 的 fallback。
- [狀態讀取](status.md)：ThreadIbsta／ThreadIberr／ThreadIbcnt。

這個 class 的存在不證明某客戶或機型正在使用 NI DLL；[機型／版本](../versions.md) 與 [未查範圍](../limits.md) 分開看。回 [兩實作索引](../index.md)。
