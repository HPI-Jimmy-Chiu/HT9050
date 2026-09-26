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

### P1 log 物件與 CSV 骨幹（20260926 晚，St02）

- **LogObjects.cpp／.h**（ht9045_db，新檔）：
  - `W906_CreateLogObjects()` 照 golden 912 TfMain 建構子 main.cpp:1550-1724 的 log 那一段。
  - `W906_DestroyLogObjects()` 照 FormDestroy :12508-12550；golden 從不 delete `slEventLog`，這裡也照做。
  - 路徑、檔名、表頭、SaveType 都是 golden 原文，並逐字比對過 golden 字串常值。
- **範圍**：使用者裁決 #6（全建 27 個，還是只建 cMyDB 用到的 4 個）還沒回，**暫照建議：只建 4 個**：`slEventLog`、`slJamAlarmLog`、`slTimeData`、`slProdRecordLog`。
  - 其餘 golden 物件依 golden 順序寫成註解。
  - `forms/fMain.h` 類別尾一次宣告全部 golden 成員（nullptr）。之後要全建，只要在 LogObjects.cpp 補建構就好。
- `slAutoSiteMapLog` 仍是門面替身 `TfMainSiteMapLog`（FormWidgets.h:219），這次不動。
- **golden `SaveEventLog()`** 定義在 LogObjects.cpp，不是去解 cmydef.cpp:129-147 的 gate。
  - 原因：cmydef.cpp 在 ht9045_globals，有些窄的 ctest 只連 globals、沒連 TMyStringList 所在的 ht9045_sm，在那裡解會斷連結。
  - 呼叫端（cMyDB.cpp）都在 ht9045_db。
- **cMyDB.cpp**：include `Public/MyStringList.h`，解開 8 個 gate。
  - :371／:427／:475／:772 的 `slEventLog` 寫入。
  - :658 `fMain->slTimeData`、:736 `fMain->slProdRecordLog`：各加一個 V906 的 NULL 守衛。golden 在建構子就建，V906 開機才建，在那之前以及 ctest 裡都是 nullptr。
  - :2061／:2176 兩段 CSV 本體（SaveEventTracker 的 ASE log、SaveEventLogInfo 的 HANDLER LOG）：`as9045LogPath`／`asSaveEventLogPath` 其實在 common.h:86／:151，gate 是過期的。
- **ctest**：
  - 新增 `MyDB_CSV_EventLog`（tests/test_mydb_csv_eventlog.cpp，完整 RESCAN 群組）。呼叫前先把 `slEventLog` 與兩個路徑全域都指到 %TEMP%，**不寫 D:\HT9045_Log**。
    驗 `MyDBITotalLoader(7)` 之後三件事：EventLogTxt 多一行、HANDLER LOG csv 有寫、`RunInfo.slEventLogFile` 有記。
  - `GA1_cMyDB`（單獨編 cMyDB.cpp）補 4 個 TMyStringList 方法與 2 個路徑全域的測試內替身，讓連結維持綠。
- **接線（wb_serve 兩行）另一個 commit**：開機在 :4160（開機的 DoReadLastData 之後）、關機在 H6 那一行。
  - 偏離 golden：golden 在 DoReadLastData 之前建。所以這裡 ProductionRecordLog 的表頭用的是讀進來的托盤種類，golden 用的是建構子預設值（:1414-1430）。
  - 原因：V906 的 IniConfig（bSPILFunction）在 DoReadLastData 那條鏈裡才讀。
- ⚠ 未編譯（這台沒有 MinGW）。

### P3 MyDBUpdateDB：開機的 AlarmCode 快取（20260926 晚，St02；共用檔經 github-59 GO）

- 背景：
  - golden `TfMain::FormShow`（main.cpp:10655）開機跑一次 `MyDBUpdateDB`：
    - 從 `D:\HT9045\Error\AlarmCodeList.txt` 建 `fMain->AlarmCodeList`／`AlarmCodeMap`，從 `AlarmUnit[]` 建 `UnitNameMap`；
    - 補 Cylinder（JAM31nnn）、Unit 15 溫度等約 100 個開機碼（DoInsertAlarmCode），缺的 `English\JAM31nnn.dat` 寫出來。
  - `MyDBIEvent` 用 AlarmCodeMap 查碼：有的照 JAM／WAR／其他給 Type 1／2／3、訊息用表裡的；沒有的是 "Unknown Alarm Code"、Type 0。
  - V906 之前這些都在 `#if 0`（fMain 門面沒有這四個成員），CSV 版的 MyDBIEvent 一律回 "Unknown Alarm Code"、Type 0，也沒人呼叫 MyDBUpdateDB。
- 做法：
  - `forms/fMain.h`：golden main.h:1503-1507 的 `AlarmCodeIter`／`AlarmCodeMap`／`UnitNameMap`／`AlarmCodeList`（接在 P1 區塊後），`<map>` 放在 :73 原本的空行。
  - `cMyDB.cpp` 解三個閘，本文照 golden：
    - DoInsertAlarmCode（golden :173-184）；
    - MyDBIEvent 的 CSV 路徑（golden :679-705，不看 bUseMDB 的第二份查表）；
    - MyDBUpdateDB 的快取建立（golden :1816-1845）。
  - 加兩個 NULL 保護：DoInsertAlarmCode 的 `AlarmCodeList`、MyDBIEvent 的 `UnitNameMap`。清單在 MyDBUpdateDB 之前是 NULL；golden 在 FormShow 就建好，那時還不會有報警。
  - sqlite 區塊裡的內層閘（:806、:853、:1308）不動：CSV 版不會編，`test_ga1_cmydb` 開 sqlite 編的時候 fMain 是 NULL。
  - `tools/wb_serve.cpp:4165`：接在 `W906_CreateLogObjects()` 同一行後面呼叫 `MyDBUpdateDB()`，不增減行。
    順序同 golden：TfMain 建構子的 log 物件 → FormShow 的 InitialHandler（Cylinder[]，wb_serve :4064）→ MyDBUpdateDB。
  - `LogObjects.cpp` `W906_DestroyLogObjects`：照 golden main.cpp:12512-12513 的位置 delete AlarmCodeList／UnitNameMap。
- 會寫的檔（golden 行為）：
  - 第一次開機，有新碼時重存 `D:\HT9045\Error\AlarmCodeList.txt`（golden 每加一個新碼就整份存一次）；
  - 缺的 `D:\HT9045\Error\English\JAM31nnn.dat`。
  - 之後開機碼都在表裡，就不寫了。
  - ⚠ 這台開發機的 D:\HT9045 就是機台根目錄，第一次跑 wb_serve 會寫這兩處，跟機台上的 golden 一樣。
- ctest：不加隔離接縫。沒有 ctest 呼叫 MyDBUpdateDB（`tests/test_ga1_cmydb.cpp:179-183` 刻意不呼叫）。
  `test_ga1_cmydb` 自己編一份 cMyDB.cpp，多加 `AlarmCodeCatalog.cpp`／`AlarmCodeUpdater.cpp` 兩個標準 C++ 檔，只為連結。
- 目前的執行期影響：MyDBIEvent 在 V906 還沒有 live 呼叫端（golden 從 note.cpp ShowErrorMessage 叫，V906 那條還是 canary 替身，見待使用者裁決 D-4）。
  所以現在實際改變的只有開機維護；報警流程接上後，事件 log 的訊息／Type 才會照 golden。
  `tools/wb_serve.cpp:7514-7520` 的 `W906_AlarmTypeOfCode`（網頁用，一律當成「在表裡」）之後可以改讀 AlarmCodeMap，那是 wb_serve 的檔，沒動。
- ⚠ 未編譯（這台沒有 MinGW）。

### P4-D5 ctest 的 log 根目錄接縫（20260927，St02；筆電 TO_STEVEN 23:4x OK，github-59 GO）

- `common.cpp` 三處都在原行改寫，不增減行：
  - :240 `as9045LogPath` 靜態初值 → `W906EnvPathOr("W906_HT9045LOG_ROOT", "D:\\HT9045_Log")`；
  - :303 `asSaveEventLogPath` 靜態初值 → `W906EnvPathOr("W906_SAVEEVENTLOG_ROOT", "D:\\HT9045_Log\\SaveEventLog")`；
  - :425 `InitCommonString` 的執行期覆寫也讀同一個變數（只改一處會被另一處蓋掉）。
- 沒設變數＝golden 字面，量產不變。CTest 的 ENVIRONMENT 在行程啟動前就在，靜態初值讀得到。
- CMake 那一半：筆電的 `tests/CMakeLists.txt` 檔尾 `cmake_language(DEFER …)`＋`_ht9045_env_extra`（`260a29ca`）已進 main。
  St02 在自己的段落（ELA_Core 之後）`list(APPEND _ht9045_env_extra ...)` 兩個變數，值是 `${CMAKE_CURRENT_BINARY_DIR}/machine_log_scratch`（＝`_ht9045_log_scratch`，
  那個變數在檔案更後面才 set，所以寫它的字面）與其下的 `SaveEventLog`。
- 還沒蓋到：`test_ga1_cmydb` 自己定義這兩個全域為 ""（不經 common.cpp），P4 歸位後會把 HANDLER LOG 寫到磁碟根目錄；P4 那一步要一起改它。
- 這個接縫先於 P4：歸位後約 960 個呼叫會寫這兩個目錄（P4 盤點 `scratchpad\audit\P4_HOMECOMING_plan.md`，決策 #24～#30）。

### S93 Save_SiteStatusLog（20260927，St02；github-59 GO，S85 登記 handlerlog.cpp）

- 本體早就是活的：`handlerlog.cpp:277` `TMyLog::Save_SiteStatusLog`（只有 fSetup->Panel1 那個子分支還 gate），`myLog` 在 `cmydef.cpp:3483`。
- 缺的是呼叫端：golden `cSetUp.cpp:4129`（SaveSetupFile 內）。
  - C 路的產生檔在 St01 分支（`tools/editlist/TestIF_File_SetUp.py:287` 目前 GATE，理由「handlerlog 沒有這支」已過時；真正的問題只剩 handlerlog.h 跟 language.h 衝突）。
  - A 形狀的 `FileRW/TestIF_File.cpp:3528` 是 J.Todo，St01 正在退場這個檔，不動。
- St02：`handlerlog.cpp` 檔尾加 `void W906_SaveSiteStatusLog() { myLog.Save_SiteStatusLog(); }`，給不能 include handlerlog.h 的 TU 用。
- St01：把 .py:287 的 GATE 改成 REPLACE `{ extern void W906_SaveSiteStatusLog(); W906_SaveSiteStatusLog(); }`，再 `--only TestIF_File_SetUp` 重產。
  gen.inc 在 `TestIF_File_SetUp.cpp:30` 全域範圍 include（匿名 namespace 從 :41 才開始），所以區塊內 extern 指到的是全域那支。
- St01 那步進來前，這支沒有呼叫端（不影響任何行為）。寫的檔：每次 Setup 存檔 append `D:\HT9045_Log\ChangeLog\yyyy_mm\log_yyyy_mm_dd.ini`，只經 FileRW（wb_serve 專用），ctest 不會走到。
- ⚠ 未編譯。

## 刻意偏離 golden（累積）

| 位置 | 偏離 | 理由 |
|---|---|---|
| 所有 sqlite 路徑 | 編譯期關掉 | 使用者裁決 sqlite3 退役 |
| MDB Updater 更新 log 檔名 | `SQLiteUpdateLog_*.logs` → `AlarmCodeUpdateLog_*.logs`；內容只剩版本行、重複碼、摘要 | 沒有 sqlite 了，golden log 主體是執行過的 SQL 文字 |
| `MyDBUpdateDB`（P3） | `AlarmCodeList.txt` 不在時，先用 P2 的 C++ 靜態目錄寫一份（`AlarmCodeCatalog_UpdateList`，同時寫 `D:\HT9045_Log\MDB_UpdateLog\` 的更新 log），再照 golden 讀進來 | golden 靠另外跑的 SQLiteUpdater.exe 先寫好這個檔；CSV 版沒有 DB 可退（MyDBQAlarmCodeList 固定 false），不補的話表裡只剩開機加的約 100 個碼。使用者裁決「C++ 靜態表＋出貨 txt 兩個都要」 |
| DoInsertAlarmCode／MyDBIEvent（P3） | `fMain->AlarmCodeList`／`UnitNameMap` 加 NULL 保護 | 清單由 MyDBUpdateDB 建；golden 在 FormShow 建好之後才會有報警，V906 的呼叫順序不保證 |
| `GetMyDBIMessage` | golden 無 `bUseMDB` 守衛，DB 關著時對 NULL handle 查詢（rows 未定義）；V906 固定回 "" | golden 在 CSV 模式的實際結果即為空字串，V906 讓它有定義 |

## 待辦（P1，排在測試通訊／GPIB 完成之後）

- 建 golden `TfMain` 建構子（`main.cpp:1550-1700`）的 `TMyStringList` log 物件（先 `slEventLog`／`slTimeData`／`slProdRecordLog`／`slJamAlarmLog`）
- `cmydef.h` 接上真 `TMyStringList`（`Public/MyStringList.h`），解 cMyDB 內 `slEventLog` 相關 gate
- `SaveEventLogInfo`／`SaveEventTracker` 的 `fLotInfo` 與路徑 gate（`as9045LogPath`／`asSaveEventLogPath` 已在 `common.cpp`，gate 過期）
