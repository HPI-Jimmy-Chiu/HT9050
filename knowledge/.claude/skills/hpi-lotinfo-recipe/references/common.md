# LotInfo／Recipe／QA共同與差異

| 共通項 | 必須另分的條件 |
|---|---|
| Lot Start／End更新批次資料、紀錄、事件 | UI、OLP、SECS／Web caller與客戶gate不同，是否開批查RunInfo.bLotStart |
| Recipe是一組檔案與對應記憶體資料 | DataPath／作用中資料夾、選用文件、編碼、configByRecipe覆蓋與寫者分工不同 |
| QA抽測閾值與停測／CleanOut協調 | RunType、Site數、Bin表、ART、OFF_LINE與backup的時點不能混用 |
| 工單字段經讀取函式才進入變數 | Shuttle Mode／Cancel、Temperature Mode在不同文件／section，不由文字名稱或JSON推定值域 |
| 同主題涵蓋HT9050與其他機台 | 對應機台snapshot、機構與單／雙Shuttle差異在 [機型](machines/index.md)，不另建9050副本 |

[API／owner現況](runtime/recipe.md)與[QA現況](runtime/qa.md)列本次來源碼查證。原V899／V900／V904／V912文件完整保留，按其原版本解讀；讀唯讀版本是分析，不授權修改其原碼。

Lot／Recipe修改會寫入執行期資料，Skill重整不執行開批、結批、下載或存檔；動作結論Steven本人優先、其次Frank，保留原裁決與實作狀態。
