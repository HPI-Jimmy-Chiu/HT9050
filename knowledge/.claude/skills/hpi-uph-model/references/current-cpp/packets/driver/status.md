# Wrapper 返回與 ibsta／iberr／ibcnt

以下是選定的 V906 cpp body 靜態判讀，完整文字與 hash 在 [manifest](source-manifest.json)。namespace 為 `gpibbridge`；`NoCard`／`Refresh` 在其匿名 namespace。

| 定位 | 選定 body 的行為 |
| --- | --- |
| 全域 ibsta／iberr／ibcnt 初值 | 三者以 0 定義；不代表實際呼叫後仍為 0 |
| NoCard | `ibsta=ERR`、`iberr=0`、`ibcnt=0`，返回 `ERR`；ERR 宣告為 `1<<15` |
| Refresh(ret) | 依序從 `g_driver->Status()`／`Error()`／`Count()` 複製三個全域狀態，返回傳入的 ret |
| gpibbridge::ibwrt(ud,buf,cnt)，g_driver 為空 | 直接返回 NoCard 的結果 |
| gpibbridge::ibwrt(ud,buf,cnt)，g_driver 非空 | 呼叫 driver 的 ibwrt，再以其返回值呼叫 Refresh |

`IGpibDriver::Status()`／`Error()` 返回 int，`Count()` 返回 long；該介面是 pure virtual。wrapper 的 ret 與複製後的 ibsta 是不同來源，不能由 ret 名稱或型別認定資料完整送達／已獲 ACK。`ibcnt` 的實際意義還要核對所選 driver 的實作。

`Refresh` 本身沒有空指標檢查；本次選讀的 ibwrt 在呼叫前有 g_driver 檢查。所有 Refresh caller、driver 替換時序與執行緒條件尚未閉合，不能由這個 caller 當作全域安全證明。

這層沒有核對 VM／MV 字串截斷、driver 的 NI／Sim 返回路徑或機台通訊。回 [driver 索引](index.md)、[指標](pointer.md)、[界線](limits.md)。
