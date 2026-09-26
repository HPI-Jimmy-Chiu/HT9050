# cMyDB 移植帳本（CSV 版）

計畫：`docs/CMYDB_20260926_CSV_PORT_PLAN.md`（skill `ht9045-mydb`）。本檔記錄基準版本、每一階段做了什麼、刻意偏離與待辦。

## 基準

| 來源 | 版本 | 用途 |
|---|---|---|
| golden Handler | `D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp`（2,069 行） | 翻譯基準（20260926 起；之前是 906_20260618） |
| MDB Updater | `D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp` | 靜態 AlarmCode 目錄來源（P2） |

## 裁決（使用者）

| 日期 | 內容 |
|---|---|
| 20260923 | SQLite 不再使用，只用 CSV；`CosFunction.bUseMDB` 強制 false（`CosFunction.cpp:4086`） |
| 20260926 | handler.db 與 sqlite3 退役，只做 CSV 版，只移植 `bUseMDB==false` 的部分 |
| 20260926 | DB 分支先 `#if 0`，不刪 |
| 20260926 | 靜態 AlarmCode 目錄：C++ 靜態表與出貨 `AlarmCodeList.txt` 兩個都要 |
| 20260926 | Summary Report XLS 區塊：可以翻，但往後排 |
| 20260926 | 20 個 `TMyStringList`：未回覆，暫照 golden 全建 |

## 階段紀錄

### P0 裁決落地（20260926）

- `cMyDB.cpp`：新增開關 `W906_CMYDB_SQLITE`（預設 0）。以下全部改成 `#if W906_CMYDB_SQLITE … #else <bUseMDB==false 的結果> #endif`，golden 原文保留：
  - `#include "third_party/sqlite3/sqlite3.h"`、`dbReadOnly`／`dbReadWrite` 兩個 handle
  - `MyDBCloseDB`／`MyDBOpenDB`／`MyDBExecSQL`（#else = golden 的 `bUseMDB==false` 早退）
  - `MyDBULotEndTime`（早退）、`MyDBQLotData`（不加任何列）、`MyDBQMotMess`／`GetMyDBIMessage`（""）、`MyDBQClearDT`（"NULL"）、`MyDBQTotalLoader`（"0"）、`MyDBQAlarmCodeList`（false）
  - `MyDBQTimeData`、`MyDBVEventFreq`（0）、`MyDBVProcess`（0）、`MyDBVProcessFilter`（-1）：只在 cObserver 的 MDB 查詢分頁用，golden `cObserver.cpp:379` 在 `bUseMDB==false` 時把該分頁隱藏；#else 顯示 "No Record!!"
  - `MyDBIEvent` 裡 golden :614 的 `if(CosFunction.bUseMDB){…}` 整段
  - 驗證：以 `#if` 堆疊掃描，全檔沒有任何 `sqlite3_`／`SQLITE_`／`dbReadOnly`／`dbReadWrite` 參照落在活的區段
- `MyDBUpdateDB` 補丁區對到 V912（906→912 的 23 行差異）：新增 `MES0103`、`WAR0359`、`JAM1014`、`WAR04217`、`WAR16330`、`WAR0179`、`WAR16340`、`WAR16341`、`WAR0495`、`WAR0496`、`WAR2213`～`WAR2216`、`WAR2371` 與 AMR 迴圈 `MES1125/1225/1325`；`WAR0354`（"Cotact" 錯字，與 RTC 的 `WAR0354` 撞號）改為 `WAR0357`。結果：99 個碼，順序與 golden 912 完全相同。
- `tests/CMakeLists.txt`：`test_ga1_cmydb` 自編一份 cMyDB.cpp，加 `W906_CMYDB_SQLITE=1`，繼續驗證保留的 DB 程式碼。
- `CosFunction.cpp:4086` 的說明補上 20260926 裁決。
- CMake 的 `ht9045_db` 仍連 `sqlite3`（沒有活的參照，無害；拔除等 sqlite 程式碼真的刪除時一起做）。
- ⚠ **未編譯**：STEVEN-NB3 沒有 MinGW（`C:\MinGW\bin\g++.exe`）與 CMake，`build.bat gate` 無法執行。需在有工具鏈的機器上跑 gate，比對 ctest 失敗清單（常駐 5 個）。

### P0 修正：連結失敗（20260926，2528799c）

- 筆電代跑 310eb411 的 gate：出貨組態連結失敗，`cObserver.cpp:7333` 宣告 `extern sqlite3 *dbReadOnly;`，`:7612`／`:7636` 拿它判「有沒有 DB」，而 P0 把定義包進了 `#if W906_CMYDB_SQLITE`。
- 修法（筆電建議 ①）：`dbReadOnly`／`dbReadWrite` 的定義移到 `#if` 外、初值 NULL；CSV 版沒人開它們，恆為 NULL 就是「沒有 DB」。不動 cObserver.cpp。
- 教訓：P0 當時找外部使用者的搜尋截在前 15 筆，漏了 cObserver。這次全樹不設上限重查，正式程式碼只有 cObserver 這三處。

### P2 MDB Updater → V906（20260926）

範圍（使用者 20260926）：「先做 D:\MDB Updater 裡面項目的轉換，其他排到往後，至少在 GPIB 完成之後」；TraceDB「不重要，往後排」。

| SQLiteUpdater 的工作 | V906 |
|---|---|
| `UpdateAlarmList` 的 AlarmCode 部分（`CreateTableAlarmList` 32 段、`DoInsertAlarmCode` 查重、存 `AlarmCodeList.txt`、`SaveUpdateLog`） | ✅ `AlarmCodeCatalog.cpp`（產生）＋`AlarmCodeUpdater.cpp`（手寫） |
| sqlite：`CreateTable`／`CreateView`／AlarmList INSERT／舊表 DROP | 退役（sqlite3 退役裁決） |
| `DoSaveEventLog`（DB 內一年前 EventLog 備份＋刪除） | 退役（純 DB） |
| `TraceDB`（Tray.DB／Plate.DB → TrayForm.csv／PlateForm.csv） | 往後排（V906 已直接讀 CSV） |

- 產生器 `tools/gen_alarmcode_catalog.py`：讀 Rev902 `CreatDatabase.cpp`（Big5），照 `CreateTableAlarmList()` 順序；Unit 24 保留 164 個馬達名 × 9 個訊息的雙迴圈（`WAR24%03d%d`）；輸出 `AlarmCodeCatalog.cpp` 與出貨檔 `ship/Error/AlarmCodeList.txt`（ASCII、CRLF）。
- 數量：原始插入 2,996、去重後 2,993；重複 3 個 `WAR0131`／`MES1621`／`JAM2339`（照 golden 保留第一個）。
- 驗證（這台可做的部分）：
  - 以 2023 年 Rev780 的 `CreatDatabase.cpp` 跑產生器，對照本機真檔 `D:\HT9045\Error\AlarmCodeList.txt`（由舊版 updater 寫出）：目錄 2,530 行中 2,528 行逐字相同且順序相同，差的 2 行是該版 `WAR1532x` 一帶的訊息修改。真檔多出的 546 行是機台自己的 Cylinder／Temperature 動態碼與開機補丁。
  - 把產生出的 C++ 表格用 Python 依 `AlarmCodeCatalog_Replay` 的順序重播：與出貨檔逐位元組相同。
- ctest `MyDB_CSV_AlarmCode`（`tests/test_mydb_csv_alarmcode.cpp`，只用標準 C++）：數量、重複碼、Unit 24 拼法、C++ 輸出與出貨檔逐位元組相同、log 用 golden 的 `@@Error!!` 字樣。⚠ 這台無法編譯，未執行。
- 尚未接進開機：`MyDBUpdateDB` 使用這份目錄屬 cMyDB 後續階段，排在 GPIB 之後。

## 刻意偏離 golden（累積）

| 位置 | 偏離 | 理由 |
|---|---|---|
| 所有 sqlite 路徑 | 編譯期關掉 | 使用者裁決 sqlite3 退役 |
| MDB Updater 更新 log 檔名 | `SQLiteUpdateLog_*.logs` → `AlarmCodeUpdateLog_*.logs`；內容只剩版本行、重複碼、摘要 | 沒有 sqlite 了，golden log 主體是執行過的 SQL 文字 |
| `GetMyDBIMessage` | golden 無 `bUseMDB` 守衛，DB 關著時對 NULL handle 查詢（rows 未定義）；V906 固定回 "" | golden 在 CSV 模式的實際結果即為空字串，V906 讓它有定義 |

## 待辦（P1，排在測試通訊／GPIB 完成之後）

- 建 golden `TfMain` 建構子（`main.cpp:1550-1700`）的 `TMyStringList` log 物件（先 `slEventLog`／`slTimeData`／`slProdRecordLog`／`slJamAlarmLog`）
- `cmydef.h` 接上真 `TMyStringList`（`Public/MyStringList.h`），解 cMyDB 內 `slEventLog` 相關 gate
- `SaveEventLogInfo`／`SaveEventTracker` 的 `fLotInfo` 與路徑 gate（`as9045LogPath`／`asSaveEventLogPath` 已在 `common.cpp`，gate 過期）
