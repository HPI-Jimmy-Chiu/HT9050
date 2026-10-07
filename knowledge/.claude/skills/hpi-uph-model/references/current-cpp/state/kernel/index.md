# Kernel 的 UPH 暫停開始與排程

本層是局部靜態證據，pin `938ebc37e95314ad496b03c7c257e1abc5fb9799`。四個 source blob／byte hash、四個 body hash、兩個 DoSystemMessage 宣告、四個 ShowRunLabel 已讀區段與兩個 SystemStart if／else 結構見 [manifest](source-manifest.json)。兩個 DoSystemMessage 排程 body 已完整讀取；兩個 ShowRunLabel 完整 UI body 尚未讀完。

| 問題 | Reference | 範圍 |
| --- | --- | --- |
| 暫停開始值何時寫入 | [開始區段](pause-start.md) | SystemStart else 與局部分支／bCalculatePauseTime；helper與全流程未閉合 |
| 每次排程是否一定記時 | [排程與刷新](scheduling.md) | 六次呼叫輪轉、刷新／安全條件；實際時間間隔與callee未驗 |
| V906／V912 差異及剩餘 | [差異與界線](limits.md) | Alarm／PLC／FTP附近差異；不稱整個UI或機台等價 |

| 固定來源 | function／變數定位 |
| --- | --- |
| [V906 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Cpp_V3.33.906.0/ckernel.cpp) | ShowRunLabel、DoSystemMessage；SystemStart、FlushFlag、tUPH_PauseStartTime、bCalculatePauseTime、iMyCounter |
| [V912 ckernel.cpp](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ckernel.cpp) | 同上；Alarm分支另有fSecsAlarm，PLC判斷使用IsSafePLCIOInstall |
| [V906 ckernel.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Cpp_V3.33.906.0/ckernel.h) | DoSystemMessage宣告；不推定所有caller |
| [V912 ckernel.h](https://gitlab.honprec.com/honprec/rd/rd5/ht9045/-/blob/938ebc37e95314ad496b03c7c257e1abc5fb9799/HT9011UC_Code_V3.33.912.0_20260908_Jimmy/ckernel.h) | DoSystemMessage宣告；與作用中機型／呼叫週期分開 |

2298個追蹤cpp／h的限定字面形狀盤點，只找到四個ShowRunLabel(形狀（兩版各定義＋呼叫）；註解／字串已mask，13個原文命中檔不是13個caller。巨集、別名、跨root與實際執行仍未核對。

沿用同一 [UPH 主題](../../../../SKILL.md) 的HT9050／其他Handler、Hot／Ambient與runtime分流；不建立機型各自的另一套Skill。
