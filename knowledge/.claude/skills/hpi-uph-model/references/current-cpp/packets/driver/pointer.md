# g_driver 設定與讀取

兩來源與五函式見 [manifest](source-manifest.json)。只描述已選讀 body，沒有延伸成全域生命週期或並行契約。

| 定位 | 已查 |
| --- | --- |
| gpibbridge 匿名 namespace 的 g_driver | `IGpibDriver* g_driver=0` 初值 |
| SetGpibDriver(d) | body 只做 `g_driver=d` |
| GetGpibDriver() | body 只返回 g_driver |
| GpibDriver.h 的 active-driver 宣告註解 | 原文稱 driver 不由此處持有；完整保存 |

選定 setter／getter body 沒有 new／delete、狀態刷新或同步操作。`SetGpibDriver(0)` 本身不更新 ibsta／iberr／ibcnt；下一個選讀的 ibwrt 空指標路徑會透過 NoCard 設值。這是來源語意推論，沒有執行測試。

誰建立／銷毀 driver、何時呼叫 setter、整個 thread 的啟停與同步還未核對；header 註解中的單一 TesterComm thread 說法保存作來源敘述，未由全部 caller 查證。禁止將未持有 ownership 的指標當作已驗證的壽命保證。

VM／MV 四全域符號與 g_driver 是不同宣告；本單元沒有證明兩者之間的所有資料傳送或指標鏈。回 [driver 索引](index.md)、[封包欄位](../fields.md)、[界線](limits.md)。
