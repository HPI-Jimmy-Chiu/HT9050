# 目前 C++ 的 UPH 局部查證

來源 pin8f213da4f；五份source blob、六個指定body、四個RUN_INFO欄位宣告與Profiler符號盤點見 [manifest](source-manifest.json)。本層是20261007靜態證據，與 [HTML／歷史來源](../resources.md) 分開保存。

| 問題 | 路由 | 已查／仍待查 |
| --- | --- | --- |
| 公式、重置與平均 | [計算本體](calculate.md) | 兩版CalculateUPH；全部caller／pause writer未閉合 |
| 計數單位、site、時間 | [計數與時間](count-time.md) | 兩版AddLoadingCount、V906兩個date helper與RUN_INFO欄位；所有writer／caller待查 |
| 版本／客戶輸出差異 | [輸出分流](outputs.md) | 指定body的guard／呼叫；DB／檔案／SECS／UI結果未查 |
| Profiler舊設計是否已實作 | [限定符號盤點](profiler.md) | 六個符號在指定追蹤檔零命中；不是全功能不存在或安全證明 |
| 機型／版本與剩餘 | [機型與界線](versions.md) | 同題分HT9050／其他Handler；容量／site／校正待查 |

沒有執行C++、Profiler、Home、檔案寫入、API、build、runtime或機台；實際UPH誤差仍待查。
