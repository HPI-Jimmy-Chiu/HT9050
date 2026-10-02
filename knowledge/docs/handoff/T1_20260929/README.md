# T1：golden 906.2 模擬紀錄 vs 移植樹重播（給 ST02，工作卡 S-11）

> 使用者 0929 11:0x 放了一份 golden BCB6 **V3.33.906.2 模擬版**（`SOFT_SIMULTE`）的 State Record：`D:\HT9045\Staterecord\2026-09-29 10_58_52`。
> 設定是兄弟機那一套（客戶碼 910、MOTION 1、IO 2）＋工單 FT005054，**沒有任何警報**。筆電 13:34 用同一套設定、同樣的操作重播移植樹的模擬版，這裡是兩邊的資料與比對結果。

## 內容

| 檔 | 是什麼 |
|---|---|
| `golden/Task_ListWithTime.csv`、`Task_ListWithTime2.csv`、`DecisionVariables.csv` | golden 那份紀錄的 task 環（每個 task 最後 500 筆）與判斷變數 |
| `golden/EventLogTxt_20260929.csv` | golden 的 EventLog（cp950）：第二次開機 10:55:25 起就是這次的操作 |
| `port/tasklist_<錨點>.csv`、`decision_<錨點>.csv` | 移植樹重播在錨點拍的快照：`home_done`（HOME 做完）、`empty_placed`（`TrayArmPlaceToBufferTask`＝3000）、`load2`（`LoadNewICTrayTask`＝500）、`paused`（按 PAUSE） |
| `port/script_result.json`、`restore.txt` | 重播的每一步與 ack、設定換入／換回紀錄（換回後逐檔驗證 CLEAN） |
| `flow_cmp_T1.txt` | `tools/flowcmp/flow_cmp.py --window T1` 的比對報告：**DIFF 44／EQUAL 7／EQUAL-mod-AL 2** |
| `FILLED_FROM_20251211.txt` | golden 紀錄的 `system` 缺 `teach.ini`、`Mot_Table.csv`、`IO_Table.csv`、`lastdata.dat`、`machinerecord.dat`、`tech.dat` 等 71 個檔，重播時從 12/11 那份對照檔補（新紀錄的 `Gerneral.ini` 跟 12/11 那份只差 906.2 開機補的預設鍵與版本號） |

## 操作順序（golden EventLog → 重播腳本 `tools/flowcmp/script_T1.json`）

開機答「不讀上次資料」→ Continuous Start → Lot Start → HOME（WAR2207～WAR2208）→ START → Color→Auto 1 → Loader 換盤→Empty → 第二次入盤 → **10:58:11 PAUSE**。
golden 之後還有「Tray Edit 把 Auto 1 填滿 → 退盤 → START → Color→Auto 1」，移植樹還沒有 Tray Edit（工作卡 S-10），所以重播停在 PAUSE。

## 要做的：44 個 DIFF 逐項分類（RULINGS_20260929 第 7、8 條）

每一項用 golden 程式碼說明為什麼兩邊不一樣，分成：
① 時間窗不同（**紀錄滿 500 筆的 8 個 task**：`AutoSHT1Task`、`AutoSHT2Task`、`CatchTrayTask`、`InArmPickFromLoadTask`、`InArmPlaceToShuttleTask`、`OutArmTask`、`RearTestDestroyICTask`、`TrayArmCatchNewTrayFromBufferTask` —— golden 留下的是 10:58:17 第二次 START 之後的資料，移植樹是第一次 PAUSE 的快照）；
② 模擬速度讓順序錯開（兩個 task 各自都對，只是交錯不同）；③ 資料不同；④ **翻譯缺陷**（同樣資料 golden 906 會走、移植樹沒走 —— 只有這類要修，寫出 golden 行號與移植樹行號）；
⑤ **Index 臂不判**：`TestHeadMotorTask`、`TestYTask`、`TestYFrontTask`、`TestYRearTask`、`IndexStatus`、`RearTest*Task`（第 8 條，等使用者的 BCB6 9050 Index 流程）。
先看的例子：`AutoSHT1Task` golden 迴圈 1→10→**200→210**、移植樹 1→10→**100→120**（Shuttle 是當標準的）。
交付：一份 `docs/handoff/ST02_T1_CLASSIFY_20260929.md`（每項一列：task、類別、golden 行號、移植樹行號、要不要修），④ 類另外列出修法。
