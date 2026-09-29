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

### P4 替身歸位＋W7 log 物件全建＋D2／D4（20260927，St02-E；Steven P4 D1=A／D2=B／D3=A／D4=B／W7=A，筆電同意共用檔清單）

- commit（`v906/steven-p4-wip`，只在本機；兩組態只編譯，未執行、未上機）：
  - WIP `d1dfbfa9`／`240d37c3`／`20cd39f4`（LogObjects.cpp、cMyDB.cpp；St02 的檔）；
  - `b4712e5f` W7＋HeaterLog hook（附帶把 20cd39f4 提早解開的 4 個 gate 先關回去、240d37c3 的 D2 三段先 gate，讓每個 commit 都編得過）；
  - `5c30e8cf` 選配接縫（筆電可單獨否決／revert，其他 commit 不依賴它）；
  - `ec1b034c` D2＋D4＋接縫；
  - 本 commit：P4 原子（五個替身同一個 commit 刪）＋containment ctest＋兩支測試修正＋本帳本＋skill。
- 五個替身（全部刪；共用檔都行數不變，刪掉的行改成註解／空行）：
  - `SECSGEM/uHGemEquipment.cpp:3452-3500` 的 3 參數 `__fastcall` 轉發（丟掉 S3）→ 刪；`:88-98` 改成兩個 extern（2 參數＋3 參數 `__fastcall`，**沒有**預設值）；`:5815`／`:5855` 現在接到 golden 本體。
  - `aHotPlateSubstrate.cpp:1244-1249` 的 2 參數計數空槽 → **D1 轉接器**（同一個符號）：計數器照舊，再呼叫 golden 3 參數 `MyDBIProcess(S1, S2, "")`。golden 906 沒有 2 參數版，golden 的 2 引數呼叫就是 `(asTable, S1, "")`。
  - `canary_support.cpp:114-123` RecordProcess（printf）→ 刪；`:523-543` MyDBIProcessNew＋沒人讀的記錄器 → 刪；`canary_support.h:302-307` 記錄器宣告拿掉，`:70`／`:300` 宣告留著。
  - `acatchtray_shims.cpp:152` NewRecordProcess `{}` → 刪（`.h:439` 宣告留著，預設值跟 golden 不同但本體把 "" 換成 " "，效果相同）。
  - `cMyDB.cpp` 四個 homecoming gate 解開：MyDBIProcessNew（golden 906_0625_Steven cMyDB.cpp:724-787）、MyDBIProcess（:789-855）、NewRecordProcess（:1545-1562）、RecordProcess（:1564-1573）；本體與 golden 空白正規化後相同，只有 MyDBIProcessNew 沒有 `__fastcall`（D3 = A）。
- 呼叫點：約 947 個 live 呼叫（RecordProcess 485、2 參數 MyDBIProcess 228、3 參數 32、NewRecordProcess 159、MyDBIProcessNew 28）＋ `tools/wb_serve.cpp` MyMessageBox 宿主 5 處（:6143／:6847／:6890／:6953／:7083 的 `MyDBIProcess("Message"/"Exception", …)`）現在都**真的寫檔**：
  HANDLER LOG csv（`asSaveEventLogPath`）、EventLogTxt（`slEventLog`，wb_serve 開機才建）、EventTracker（`as9045LogPath\ASE log`，只有 MyDBIProcessNew 類）、O06 開時 `asProductionLogPath`、Greatek＋N14_1 時 `sProductionInfoFilePath`。
- D2 ProductionLog：本體在 `LogObjects.cpp`（golden cpublic.cpp:594-622；cpublic.cpp:778-806 的 gate 內那份留作對照）；`forms/fMain.h:1301` `MemoProductionLog`（vclcompat::TMemo，Lines 是不限行數的 TStringList，不是 4096 上限的 TfMainMemo）；
  開機在 `W906_CreateLogObjects` 末尾照 golden main.cpp:9411-9416 載回當天檔（不載的話第一次存檔會把當天檔蓋掉）；關機 `ProductionLog("Close")`。接縫 `W906_RMS_ROOT`（common.cpp:263 與 :442 兩處）。
- D4 SaveMessageHistroy：`forms/fProductionInfo.cpp` 檔尾（golden ProductionInfo.cpp:1136-1173）、`.h:280` 宣告、`.h:129` 加 `_sOEE_DirectoryName`；接縫 `W906_PRODINFO_ROOT`（common.cpp:293，只有一處）。
- W7 = A：golden main.cpp:1500-1675 全部 24 名／58 個 log 物件（`slHanaTrayMap[eTrayCount]`、`slDewPointLog[3]`），FormDestroy :11991-12042 的刪除順序。
  - HeaterLog：`cpublic.cpp:704-706` 改成呼叫 `W906_HeaterLogHook`（定義在 cpublic.cpp 檔尾，LogObjects 開機設、關機清）；`HeaterSVLog`（:711）仍 gate。
  - `slHanaTrayMap` 定義：`cmydef.cpp:6151`（原 :6152 那行變成 `#if 0`，W6 gate 往下一行）；同一個 commit 打開 Jimmy 的 `aoutarm9045.cpp` GATE(W906-ARM3) golden :2901（外層 `IsHanaArtAvailable()` 在 V906 仍恆 false）。
  - `slAutoSiteMapLog`：衍生替換（`W906_SiteMapLogReal` 包真的 TMyStringList，關機還原門面替身）。
  - `JsonBridge/actions/MainClarnData.cpp:45`：先用 `fMain->slQtyLog`，沒有才用延遲建的那份（ctest）。
- 選配接縫（`5c30e8cf`）：LogObjects 的 21 個 `"D:\\HT9045_Log\\X"` 改 `as9045LogPath+"\\X"`；common.cpp :246／:430 `asTorqLogPath`、:266 `asProductRecordPath`、:268 `asQtyDataPath` 從 `as9045LogPath` 導出；:316 `W906_HPCARD_ROOT`、:323 `W906_GROUNDMAN_ROOT`（保留 golden 小寫 `_log`）；tests/CMakeLists.txt :3448。
- ctest：
  - 新 `MyDB_P4_Containment`（`tests/test_mydb_p4_containment.cpp`，檔尾，RUN_SERIAL）：四個根目錄（as9045LogPath／asSaveEventLogPath／asProductionLogPath／sProductionInfoFilePath）不含 machine_log_scratch 就拒跑；每個入口一個 token（含 2 參數轉接器、RecordChangeLogProcess、wb_serve 的 "Message"／"Exception" 形狀、O06 的 ProductionLog、Greatek 的 SaveMessageHistroy）；
    找得到 token 列；golden 真根目錄（SaveEventLog、ASE log、EventLogTxt、D:\RMS、ProductionInfo＋W7 的 21 個）測試期間不可新建、不可有寫入時間 ≥ 開始時間的檔。**同一台機器開著 wb_serve／HT9045.exe 會誤判失敗。**
  - `test_ga1_cmydb`：刪掉自己的 3 參數替身（:238-248 改成 fProductionInfo 替身＋%TEMP% 小工具），兩個 D5 根目錄在 RecordChangeLogProcess 前指到 `%TEMP%\ht9045_ga1_cmydb_p4`（原本是 ""，會寫到磁碟根目錄），:325-327 改成檢查 HANDLER LOG 那一列再刪檔。
  - `test_ga1_cprod`：這個 target 直接編 canary_support.cpp，:168 補一個空的 `RecordProcess`（cprod.cpp:1173 要用）。
  - `tests/CMakeLists.txt` St02 段 :3446 加 `W906_RMS_ROOT`／`W906_PRODINFO_ROOT`（`_ht9045_env_extra`，每支測試都有）。
  - ⚠ 手動跑測試 exe（沒有 ctest 的環境變數）會寫真的 `D:\HT9045_Log`：跟以前一樣，containment 只保護 ctest。
- 其他：`JsonBridge/EventLog.cpp:113` RecordProcess 那條不再標 `kSinkStdout`（替身不印了）；`TesterComm/Handler/HandlerBridgeCtl.cpp:95-99`、`cMyDB.h` 的 HOMECOMING NOTICE 與宣告註解改成新事實。
- 沒改（清單外，要的話請擁有者改）：`JsonBridge/ChanAction.cpp:210-248`（`act.main.clarnData` 的診斷字串還寫「落在計數式空槽」）、`EventLog.cpp:104` 註解、`EventLog.h:69` 註解、約 31 個檔引用舊替身行號的註解、`ht9045-json-bridge` skill 裡的同一段敘述。
- nm（驗收，唯讀）：見 commit 訊息。
- ⚠ 未執行、未上機。
- 上機要看（RULINGS_20260927 第 4 條：沒在真機驗過）：① 開機後 `D:\HT9045_Log\Heater_On_Off_LOG`、`EventLogTxt`、`SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv` 每按一個鍵（START／PAUSE／ALARM RESET）各多一行，`ASE log\yyyy\mm\dd\*_EventTracker.csv` 在 NewRecordProcess 類（MESxxxx）多一行；② O06 開：`D:\RMS\<SocketHandlerID>_yyyymmdd.logs` 當天檔重開 wb_serve 後**不被截短**（開機先載回），關機多一行 `--> Close`；D:\RMS 不存在時什麼都不寫、也不報錯（偏離：golden 丟 EFCreateError）；③ CC_Greatek（956）＋N14_1：每筆 NewRecordProcess 在 `D:\HT9045_log\ProductionInfo\<MO>\\<MO>_History.csv` 多一行（資料夾名空白，偏離）；④ 關機後 `HeaterLog("Close")` 那行有寫、之後再呼叫 HeaterLog 不當機；⑤ 2D sort 的 `D:\HT9045_Log\2DMapping`、Auto Site Map 的 `ASM\ASMLog*`、清料 `QtyData` 有檔；⑥ 事件多的機台 tick 週期沒有明顯變慢（每筆都同步寫檔，golden 同）。

### A4 RecordTimeData 的每小時 Production_Loader 檔（20260928，St02-E helper；審查 A4）

- golden：906_0625_Steven `cMyDB.cpp:339-498` RecordTimeData（912 同段逐字相同）。
  - `:393-431`＝Production_Loader：`iDataType==3 || CUSTOMER_CODE==CC_TERAPOWER` 時，依時鐘選時段與日期（`:396-418`），建 `<asProduct_LoaderPath>\<yyyy>`（`:423-424` MyForceDirectories），
    檔案 `<yyyy>\<yyyy-mm-dd>-<0800-2000|2000-0800>.txt`（`:425`），讀舊值再寫 `[Product] LoaderCount`／`ProductTime`（累加，`:427-430`），`LastSet.iLoaderCount=0`（`:431`）。
  - `:433-476`＝O19 08:00 日／週／月報表（只有 08:00:01～08:00:19 那一次）。
- V906：`cMyDB.cpp:533-571` 解開（原本 GA1-B4 的 `#if 0` 把 :393-476 整段關掉）；宣告回到 golden 的位置（:488-497＝golden :348-357），只用在 O19 的那幾個（iWeek、iDate、as1DayDate…、HostName）放進 O19 的 gate 裡。
  O19 仍 `#if 0`（:575-630，在 `if(bIsRecordSummaryReport)` 的大括號裡，if 本身是活的）＝**已知缺口**，理由寫在 gate 上。整個函式行數不變（:656 以後不動）。
- `LastSet.iLoaderCount`：`ainarm9045.cpp:6405` 每從 Loader 取一顆 +1（golden `ainarm9045.cpp:2767`）；以前從不歸零，現在每小時（iDataType 3）歸零。
- 接縫：`W906ProductLoaderDir()`（:474）＋ ctest 環境 `tests/CMakeLists.txt` :3553（St02 的行）`W906_PRODLOADER_ROOT=…/machine_log_scratch/Production_Loader`。
- ctest `ELA_TimeData`（`tests/test_ela_timedata.cpp`）擴充：containment 多查 `W906_PRODLOADER_ROOT`；`asProduct_LoaderPath` 與 `W906_PRODLOADER_ROOT` 都指到沙盒 `%TEMP%\ht9045_ela_timedata_<tick>\Production_Loader`（在 D:\HT9045* 底下就拒）；
  步驟 3：一個檔、檔名（測試自己照 golden :396-425 用真的時鐘算）、`[Product]`／`LoaderCount=9`／`ProductTime=14`、iLoaderCount 歸零；步驟 4：iDataType 2 且不是 TERAPOWER＝檔不動、iLoaderCount 留著；
  步驟 7：CC_TERAPOWER＋iDataType 2＝同一個檔累加成 13／58。預期值是照 golden 推的，沒有執行。
- golden 怪處照搬（沒修）：時段只看時鐘，**夜班那 12 小時會分到三個檔**——當天 21:00～23:00 的整點記在「明天-2000-0800」，隔天 00:00～07:00 的整點記在「(隔天＋1)-2000-0800」（`:414-418` 對 00:00～08:00 也是 +1 天），隔天 08:00 那次記在「(隔天－1)-2000-0800」。白班（09:00～20:00 的整點）都記在「今天-0800-2000」。
  golden 的讀取端（cObserver.cpp:5018-5033）把區間內每天的兩個檔加總，所以多天加總大致對，單日與區間兩端會偏。
- 連結：`W906ProductLoaderDir()`（:474）沒設環境變數時回 golden 的字面 `D:\HT9045_Log\Production_Loader`（`asProduct_LoaderPath` 唯一會有的值，common.cpp:314／:467），不直接引用那個全域 ⇒ `test_ga1_cmydb`（自己編一份 cMyDB.cpp、全域用替身）不用加替身也連得過（St02-E 20260928 改；原本 helper 版會缺 `asProduct_LoaderPath` 連結不過）。
- ⚠ 未執行、未上機。上機要看：每小時 hh:00:04 起 `D:\HT9045_Log\Production_Loader\<yyyy>\<yyyy-mm-dd>-<0800-2000|2000-0800>.txt` 更新（`[Product]` `LoaderCount`＝這一小時從 Loader 取的顆數累加、`ProductTime`＝生產秒數累加）。

## 刻意偏離 golden（累積）

| 位置 | 偏離 | 理由 |
|---|---|---|
| 所有 sqlite 路徑 | 編譯期關掉 | 使用者裁決 sqlite3 退役 |
| MDB Updater 更新 log 檔名 | `SQLiteUpdateLog_*.logs` → `AlarmCodeUpdateLog_*.logs`；內容只剩版本行、重複碼、摘要 | 沒有 sqlite 了，golden log 主體是執行過的 SQL 文字 |
| `MyDBUpdateDB`（P3） | `AlarmCodeList.txt` 不在時，先用 P2 的 C++ 靜態目錄寫一份（`AlarmCodeCatalog_UpdateList`，同時寫 `D:\HT9045_Log\MDB_UpdateLog\` 的更新 log），再照 golden 讀進來 | golden 靠另外跑的 SQLiteUpdater.exe 先寫好這個檔；CSV 版沒有 DB 可退（MyDBQAlarmCodeList 固定 false），不補的話表裡只剩開機加的約 100 個碼。使用者裁決「C++ 靜態表＋出貨 txt 兩個都要」 |
| DoInsertAlarmCode／MyDBIEvent（P3） | `fMain->AlarmCodeList`／`UnitNameMap` 加 NULL 保護 | 清單由 MyDBUpdateDB 建；golden 在 FormShow 建好之後才會有報警，V906 的呼叫順序不保證 |
| `GetMyDBIMessage` | golden 無 `bUseMDB` 守衛，DB 關著時對 NULL handle 查詢（rows 未定義）；V906 固定回 "" | golden 在 CSV 模式的實際結果即為空字串，V906 讓它有定義 |
| 2 參數 `MyDBIProcess(S1,S2)`（P4-D1） | 不是 golden（golden 906 只有 3 參數，S2=""）；留成轉接器 → golden `(S1, S2, "")`，計數器照舊 | 228 個呼叫點、18 個區域宣告；一次改完不可能（使用者裁決 A，以後再拿掉） |
| `Public/MyProductionRecord.cpp:1154`、`KYECFTP/FTPClient_Transfer.cpp:87` 匿名 3 參數 | 繼續把 (S1,S2,S3) 摺成 (S1, S2+" : "+S3) 再進轉接器；golden 是 (S1,S2,S3) | 匿名 namespace 的 TU 內替身，P4 範圍外 |
| `ProductionLog`（P4-D2） | 本體放 ht9045_db（LogObjects.cpp）；`SaveToFile` 目錄不存在時靜默失敗（golden VCL 丟 EFCreateError）；`ProductionLog("Close")`／`HeaterLog("Close")` 在關機（FormDestroy 時機）跑，不是 FormClose | globals 沒有 fMain；vclcompat 不丟例外；wb_serve 沒有 FormClose |
| `SaveMessageHistroy`（P4-D4） | `TStringList::Append` → `Add`；`_sOEE_DirectoryName` 恆為 ""（golden 從 PI 設定 ini 讀，ProductionInfo.cpp:204 沒翻）→ 路徑少一層資料夾 | vclcompat 沒有 Append；OEE 設定讀取未移植 |
| `slAutoSiteMapLog`（W7） | 門面成員型別是 `TfMainSiteMapLog*`；開機換成包真 TMyStringList 的衍生類別，關機換回 | 不動共用的 fMain.h 型別 |
| HeaterLog（W7） | golden 那行經 `W906_HeaterLogHook` 呼叫 | cpublic.cpp 在 ht9045_globals，沒有 fMain／TMyStringList |
| `MainClarnData.cpp:45`（W7） | `fMain->slQtyLog` 有就用；沒有（ctest）才用延遲建的那份 | JsonBridge 既有的 lazy 物件留給沒開機的測試 |
| W7 log 路徑（選配，`5c30e8cf`） | golden 字面 `"D:\\HT9045_Log\\X"` 寫成 `as9045LogPath+"\\X"`；TorqueLog／ProductRecord／QtyData 從 as9045LogPath 導出；HPCARD／GroundMan 各一個 env | 沒設接縫時值相同；ctest 圍堵 |
| `JsonBridge/EventLog.cpp:113` | RecordProcess 那條不再標 stdout | 替身刪了，golden 本體不印 |
| State Record 背景執行緒的 log（STATEREC-TS，20260928 St02-E，筆電 13:1x 同意） | cStateRecord.cpp 工作執行緒（RunStateRecordJob :1420／:1473／:1479／:1506）不再自己呼叫 RecordProcess，改呼叫 `W906_StateRecordLogLater` 排隊（LogObjects.cpp 檔尾，CRITICAL_SECTION）；tick 執行緒 `W906_StateRecordDrainLog`（wb_serve.cpp:4575，每拍）照順序呼叫 golden RecordProcess。cStateRecord.cpp 7 行、wb_serve.cpp 1 行都是同一行改 | P4 之後 RecordProcess 是 golden 本體（寫 ExString＋MyDBIProcess 到共用 log 物件），在工作執行緒呼叫會和 tick 執行緒搶；golden 本來就只在主執行緒呼叫。延遲最多一拍 |
| `D:\HT9045\Error\` 字面（ERRSEAM，20260928 St02-E） | cMyDB.cpp 第 283 行 `W906ErrorDir()`：環境變數 `W906_ERROR_ROOT` 有值就用它，沒設或空＝golden 的 `D:\HT9045\Error\`；第 319／320（DoInsertAlarmCode）、1752、1928（讀）、2193～2209（MyDBUpdateDB 的 AlarmCodeList.txt）、2252（English\JAM31xxx.dat）改走它；第 2203 行 P2 目錄的兩個路徑改成 `W906ErrorDir()+"AlarmCodeList.txt"`／`as9045LogPath+"\MDB_UpdateLog"`（兩個接縫都沒設時＝`kAlarmCodeListPath`／`kAlarmCodeUpdateLogDir`）。ctest 的環境（tests/CMakeLists.txt 第 3552 行，St02 的行）設成 `machine_log_scratch/Error`。每一處都是一行換一行 | 測試縫：將來的測試呼叫 MyDBUpdateDB 不會寫到真的 Error 資料夾；機台不設這個變數，行為不變 |
| RecordTimeData 的 Production_Loader（A4，20260928 St02-E helper） | ① 路徑經 cMyDB.cpp 第 474 行 `W906ProductLoaderDir()`：`W906_PRODLOADER_ROOT` 有值就用它，沒設或空＝golden 的 `asProduct_LoaderPath`（common.cpp:314／:467 仍是 golden 字面 `D:\HT9045_Log\Production_Loader`）；第 563／565 行兩處。ctest 環境 tests/CMakeLists.txt 第 3553 行設成 `machine_log_scratch/Production_Loader`。② golden `(Now()+iDecDay)` 寫成 `(Now()+TDateTime(iDecDay))`（第 560／561 行）。③ O19 08:00 報表（golden :433-476）仍 gate＝已知缺口 | ① common.cpp 不是 St02 的檔，接縫放在自己的 cMyDB.cpp（同 ERRSEAM）；每小時都會跑，沒有接縫的話任何跨整點的 ctest 會寫到真的 D:\HT9045_Log；機台不設，行為不變。② vclcompat 的 TDateTime＋int 有歧義（cpublic.cpp:461 同一個問題），值相同。③ DoProduction_Summary_Report 是空殼（cObserver.cpp:2040）、沒有 DayOfWeek、沒有一般用的 fConfiguration |

## 待辦（P1，排在測試通訊／GPIB 完成之後）

- 建 golden `TfMain` 建構子（`main.cpp:1550-1700`）的 `TMyStringList` log 物件（先 `slEventLog`／`slTimeData`／`slProdRecordLog`／`slJamAlarmLog`）
- `cmydef.h` 接上真 `TMyStringList`（`Public/MyStringList.h`），解 cMyDB 內 `slEventLog` 相關 gate
- `SaveEventLogInfo`／`SaveEventTracker` 的 `fLotInfo` 與路徑 gate（`as9045LogPath`／`asSaveEventLogPath` 已在 `common.cpp`，gate 過期）

## golden 行號與 912↔906 稽核（20260927）

- 規定是對照 golden 906_20260618；本帳本與 cMyDB.cpp 的 golden 行號多半是 912（St02 這台只有加密 7z）。全面換算等 NB2 的工具。
- 稽核（`docs/ST02_GOLDEN906_AUDIT.md`）：cMyDB.cpp 35 支裡 34 支本體 906／912 相同；`MyDBUpdateDB` 差在 912 的 AlarmCode 目錄（當時刻意同步 912，#11）。
