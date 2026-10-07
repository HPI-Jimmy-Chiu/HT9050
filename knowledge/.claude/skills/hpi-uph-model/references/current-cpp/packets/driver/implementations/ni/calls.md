# NI 呼叫與 fallback

定位 `NiGpibDriver::ibfind`、其餘八個 ib*、NI_CALL、F_* 及 Fn*；完整來源在 [manifest](../source-manifest.json)。

| 呼叫 | fn_ 有值 | fn_ 為空 |
| --- | --- | --- |
| ibfind | 用 FnS 呼叫 F_find，即 export ibfindA | 返回 -1，選讀 body 不更新 lastSta_ |
| ibrsc／ibpad／ibtmo／ibwait／ibrsv | NI_CALL 以 FnII 呼叫對應 slot | 把 lastSta_ 設 ERR 並返回該值 |
| ibrd | NI_CALL 以 FnRd 呼叫 F_rd | 同上 |
| ibwrt | NI_CALL 以 FnWr 呼叫 F_wrt | 同上 |
| ibstop | NI_CALL 以 FnI 呼叫 F_stop | 同上 |

有函式指標時 NI_CALL 直接返回 DLL 函式的結果；這個分支沒有把返回值存進 lastSta_。ibwrt 沒有在這層解析 VM／MV 字串、切片或認證 ACK。

此處只確認來源的函式指標派送與 fallback。傳入 descriptor／buf／cnt 是否有效、外部 DLL 寫了多少或 Tester 收到什麼，都要另查 [狀態](status.md) 與 caller／DLL／機台。回 [NI 索引](index.md)、[界線](../limits.md)。
