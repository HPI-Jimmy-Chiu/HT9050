# module_ 與 fn_ 的來源分支

以 `NiGpibDriver::NiGpibDriver`、`~NiGpibDriver`、header 的 `Loaded()` 及 kNames 定位，詳見 [manifest](../source-manifest.json)。

| 來源分支 | 靜態行為 |
| --- | --- |
| 初值 | module_=0、lastSta_=ERR，十二個 fn_ 歸零 |
| LoadLibraryA 失敗 | 直接返回，module_ 留0 |
| DLL 可開、任一 export 找不到 | 十二項全數 GetProcAddress 後，FreeLibrary，fn_ 全數清0，再返回 |
| 十二項均找到 | module_=h；header Loaded() 只比較 module_ 是否非0 |
| dtor | module_ 非0時 FreeLibrary；選讀 body 沒有 SetGpibDriver(0) |

kNames：ibfindA、ibrsc、ibpad、ibtmo、ibwait、ibrd、ibrsv、ibwrt、ibstop、ThreadIbsta、ThreadIberr、ThreadIbcnt。slot 與 typedef 同列保存；本輪沒有檢查實際 DLL export 或 ABI。

需要追 ctor／dtor 使用者與 g_driver 的解除順序才可判定指標壽命；單看 FreeLibrary 或 Loaded() 不足以閉合此題。回 [NI 索引](index.md)、[呼叫](calls.md)、[界線](../limits.md)。
