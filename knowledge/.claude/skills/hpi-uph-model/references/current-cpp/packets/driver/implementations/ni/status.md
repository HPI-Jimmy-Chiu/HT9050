# NI 狀態是另一次函式呼叫

定位 `NiGpibDriver::Status`／`Error`／`Count`、F_sta／F_err／F_cnt、FnV；[manifest](../source-manifest.json) 保存完整文字。

| 定位 | fn_ 有值 | fn_ 為空 |
| --- | --- | --- |
| Status | 呼叫 ThreadIbsta | 返回 lastSta_ |
| Error | 呼叫 ThreadIberr | 返回0 |
| Count | 用 FnV（int 返回）呼叫 ThreadIbcnt，再轉long | 返回0 |

wrapper 的 [Refresh](../../status.md) 依序取這三個值，最後仍返回原 ret。DLL 呼叫返回、ibsta、iberr、ibcnt 是分開的來源，這層沒有建立送達／ACK 契約。

header 註解稱這些 NI 狀態為 per-thread，並稱所有呼叫在同一 TesterComm thread；本輪只保留註解，沒有盤完所有 caller 或查證外部 NI DLL 行為。回 [NI 索引](index.md)、[機型／版本](../versions.md)、[界線](../limits.md)。
