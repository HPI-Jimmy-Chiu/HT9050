---
name: ht9045-mydb
description: >
  HT9045 cMyDB 記錄層知識庫（cMyDB.cpp／cMyDB.h）：告警事件（MyDBIEvent）、流程／按鍵／生產／時間統計
  （MyDBIProcess／MyDBIProcessNew／NewRecordProcess／RecordProcess／RecordChangeLogProcess／MyDBITotalLoader／
  MyDBITimeData／MyDBIUPH／MyDBIProductionData／RecordTimeData）、AlarmCode 目錄（MyDBUpdateDB／DoInsertAlarmCode／
  AlarmCodeList.txt／AlarmCodeMap／UnitNameMap／AlarmUnit[32]）、CSV 落點（SaveEventLogInfo 的 HANDLER LOG csv、
  SaveEventTracker、slEventLog／TMyStringList 的 EventLogTxt）。使用者裁決（20260923、20260926）：SQLite／Handler.db3／
  sqlite3 退役，C++ 只做 CSV 版，只移植 CosFunction.bUseMDB==false 的路徑。含 MDB Updater（SQLiteUpdater.exe、
  CreatDatabase.cpp 32 個 Unit 的靜態 AlarmCode）與 EventlogAnalyzer.exe 的關係、AlarmCode 格式與唯一性規則、
  V906 現況（P0～P4＋W7 已落地；P4／W7 是 20260927 St02 本機 commit、只編譯未上機：五個借住替身歸位、2 參數 MyDBIProcess 改成
  轉接器、golden log 物件全建 24 名 58 個、ProductionLog（D2）、SaveMessageHistroy（D4）、ctest MyDB_P4_Containment）與 CSV 版移植計畫。
  Use when：告警沒寫進 log、Unknown Alarm Code、AlarmCodeList.txt 缺碼或重複、新增 JAM/WAR/MES 碼、EventLogTxt／
  HANDLER LOG csv 欄位、SPIL 格式 event log、bUseMDB、cMyDB 移植、MDB Updater 升版、Event Log Analyzer。
  關鍵字：cMyDB, MyDBIEvent, MyDBIProcess, MyDBIProcessNew, NewRecordProcess, RecordProcess, RecordChangeLogProcess,
  MyDBUpdateDB, DoInsertAlarmCode, AlarmCodeList.txt, AlarmCodeMap, UnitNameMap, AlarmUnit, GetMyDBIMessage,
  GetAlarmCodeList, GetJameCodeOfAxis, SaveEventLogInfo, SaveEventTracker, HANDLER LOG, EventTracker, slEventLog,
  TMyStringList, EventLogTxt, SaveEventLog, bUseMDB, Handler.db3, sqlite3 退役, MDB Updater, SQLiteUpdater,
  CreatDatabase.cpp, CreateTableAlarmList, EventlogAnalyzer, EventLogSaver, JAM, WAR, MES, AlarmID 9 碼,
  Unknown Alarm Code, bSPILFunction, ht9045_db, homecoming, AlarmCodeCatalog, CSV 版, P4, W7, LogObjects,
  W906_CreateLogObjects, ProductionLog, MemoProductionLog, SaveMessageHistroy, W906_HeaterLogHook, W906_RMS_ROOT,
  W906_PRODINFO_ROOT, MyDB_P4_Containment, 轉接器。
  CSV 版移植計畫全文 → references/cmydb-csv-port-plan.md
---

# HT9045 cMyDB 記錄層（CSV 版）知識庫

## 1. 這個模組是什麼

- 檔案：golden `cMyDB.cpp`（V912：2,069 行）／`cMyDB.h`（83 行）。名字裡的 DB 是歷史：原本是 SQLite（`D:\HT9045\MDB\Handler.db3`）的封裝，
  2021 起部分客戶關閉 MDB（`CosFunction.bUseMDB`），**現行機台一律 `bUseMDB=false`，只走 CSV**。
- 兩個裁決：
  - 20260923：「SQLite 資料庫已經不再使用，只使用單純的 csv 存檔方式」「C++ 實作也強制為 false」（V906 `CosFunction.cpp:4060-4086`）。
  - **20260926：「handler.db 可以不用了，sqlite3 也退役了，只做 csv 版本即可」「只需移植 `CosFunction.bUseMDB==false` 的部分」**。
- 職責（`bUseMDB==false` 下）：
  1. 開機建 AlarmCode 目錄：`MyDBUpdateDB()` 讀 `D:\HT9045\Error\AlarmCodeList.txt` → `AlarmCodeMap`；`UnitNameMap` 來自 `AlarmUnit[32]`；再加動態碼（Cylinder `JAM31xxx`、Temperature `WAR15xx`）與 105 個補丁 `DoInsertAlarmCode()`，寫回 txt。
  2. 告警事件：`MyDBIEvent(Code, MotorID, …)` 用 Map 解出 Type／Unit／Message／UnitName → `SaveEventLogInfo()`（HANDLER LOG csv）＋ `SaveEventTracker()`。
  3. 流程／按鍵／生產／時間統計：`MyDBIProcess` 一族 → `slEventLog`（`TMyStringList`，`D:\HT9045_Log\EventLogTxt`）＋ `ProductionLog()` ＋ `SaveEventLogInfo(…22)`。
  4. 畫面：`GetAlarmCodeList()`／`GetJameCodeOfAxis()` 讀 txt。

## 2. 檔案與 CSV 落點

| 檔 | 產生者 | 格式 |
|---|---|---|
| `D:\HT9045\Error\AlarmCodeList.txt` | `MyDBUpdateDB`／`DoInsertAlarmCode` | `CODE=Message` 一行一筆；web `Alarm-description.json` 的來源（見 `ht9045-alarm-dismissal`） |
| `D:\HT9045_Log\EventLogTxt\…` | `slEventLog`（golden `main.cpp:1552` 建） | 8 欄 UnitName／AlarmCode／OccurDateTime／Recovery／StopedTime／Duplicate／Message／ErrPart；`IniConfig.bSPILFunction` 決定 SPIL 格式（`AddTextWithLineNo`）或一般（`AddTextWithDateTime`） |
| `D:\HT9045_Log\SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv` | `SaveEventLogInfo` | 16 欄；`iType` 0=ERROR（暫存等 recover）、10 START、11 END、12 INPUT、13 PASS、14 FAIL、15 RATE、16 IDLE、17 PRODUCTION、18 DOWN、19 SOCKET、20 WARNING、21 CHANGE LOG、其他 PROCESS |
| `D:\HT9045_Log\ASE log\yyyy\mm\dd\<ID>@…_EventTracker.csv` | `SaveEventTracker` | 12 欄 |
| `D:\HT9045_Log\TimeData`／`Production_Loader`／ProdRecord | `slTimeData`／`RecordTimeData`／`slProdRecordLog` | 時間統計（MUBA／MTBA 每 12 小時）、生產記錄 |

## 3. AlarmCode 規則（與 MDB Updater agent 一致）

- 格式 `{JAM|WAR|MES}{Unit:02d}{Code:04d}`，例 `JAM0101`、`WAR16446`、`MES2820`；Type：JAM=1（停機）、WAR=2、MES=3。
- 9 碼 AlarmID = `Type×10^8 + Unit×10^7 + Code`（`DoInsertAlarmCode` 算；`MyDBIEvent` 組成 `%d%02d%06d` 給 `SaveEventLogInfo`）。
- Unit 01~31 對照：01 Input Arm、02 Output Arm、03 Index、04 In Shuttle、05 Out Shuttle、06 Tray Arm、07 Tester I/F、08 Scanner、09 Loader、10 Empty、11~13 Auto1~3、14 Color、15 Temperature（動態）、16 System、17~19 Fix1~3、20 ESD、21 Process Log、22 Motion Log、23 Cassette、24 Motor（`MyDBIEvent` 強制 UnitName=Motor）、25~27 Auto4~6、28~30 Fix4~6、31 Cylinder（動態，`CreateTableAlarmList_031` 保持空）。
- 唯一性：同碼只能出現一次（靜態＋補丁合計）；**同數字段不可跨 TYPE**（`WAR16119` 撞 `MES16119` 的教訓 → 改 `WAR16126`）。
- 靜態碼來源（BCB 線）：`D:\MDB Updater\…\CreatDatabase.cpp` 32 個 `CreateTableAlarmList_XXX_()`；補丁碼放 `MyDBUpdateDB()` 末段；定期把補丁升格為靜態。C++ 線（CSV 版）改成單一來源 `AlarmCodeCatalog.cpp`（計畫 P2）。

## 4. 相關外掛工具（BCB 線，C++ 線不依賴）

| 工具 | 何時 | 作用 |
|---|---|---|
| `SQLiteUpdater.exe`（MDB Updater Rev902） | `bUseMDB=true` 機台 | 重建 `Handler.db3` 的 AlarmList／表／View，輸出 `AlarmCodeList.txt` |
| `EventLogSaver.exe` | `bUseMDB=true` | 事件存 DB |
| `EventlogAnalyzer.exe`（`TfrmELA`） | `bUseMDB=false` | 讀 CSV 的分析器；Handler `main.cpp:18611` 啟動、`InterfaceSYS.cpp SendCommand_EventLog()` 下指令 |

C++ 線：web Event Log 頁讀 CSV 取代 `EventlogAnalyzer.exe`（BCB 接 BCB、C++ 接 C++）。

## 5. V906 現況（20260926 盤點；**落地狀態見本節末「20260926 晚」**）

- `cMyDB.cpp` 2,386 行已翻（基準 golden 906），掛 `ht9045_db` 並連 `third_party/sqlite3` 3.7.7.1（`CMakeLists.txt:1218-1247`）；`bUseMDB` 在 `CosFunction.cpp:4086` 強制 false。
- （20260926 盤點當時；P1 之後已不成立，見本節末）33 個 `#if 0` gate：CSV 路徑被卡在 `fMain->AlarmCodeMap/AlarmCodeList/UnitNameMap`、`slEventLog` 型別不完整（`cmydef.h` 只有前置宣告，雖然 `Public/MyStringList.cpp` 1,288 行已翻）、`fObserver` 三個時間、`fLotInfo` ASECL 欄位、借住替身 ×4。
- （20260926 盤點當時；**已不成立**：現在 `LogObjects.cpp:67`／`:73` 的 `W906_CreateLogObjects` 由 wb_serve :4165 開機建 `slEventLog`，ctest 裡仍是 NULL）**wb_serve 沒建 `slEventLog`**（golden `TfMain` 建構子建約 20 個 `TMyStringList`，V906 只建 3 個）→ 所有 `if(slEventLog!=NULL)` 跳過；`DoInsertAlarmCode` 的 txt 鏡射被 gate → 目前 **什麼 CSV 都沒寫**（`JsonBridge/ChanAction.cpp:215-244` 20260923 實測）。
- （20260926 盤點當時；**P4 之後已不成立**，見本節末）借住替身：`MyDBIProcess`→`uHGemEquipment.cpp:3483`→`aHotPlateSubstrate.cpp:1264` 計數空槽；`MyDBIProcessNew`／`RecordProcess`→`canary_support.cpp`；`NewRecordProcess`→`acatchtray_shims.cpp`。
- 906→912 差異只有 `MyDBUpdateDB` 補丁區 23 行（17 個新碼＋AMR 迴圈＋`WAR0354→WAR0357`）。

**20260926 晚落地狀態**（St02，`v906/steven-gpib-widget`，都未在 St02 編譯；權威 `docs/CMYDB_PORT_LEDGER.md`）：
- P0：sqlite3 退役，`W906_CMYDB_SQLITE` 預設 0；DB 分支 `#if 0` 保留；已進 main。
- P1 log 物件：當時暫定只建 4 個（`slEventLog`、`slJamAlarmLog`、`slTimeData`（TByYear）、`slProdRecordLog`）；**W7 = A（Steven 20260927）之後 golden 全建**，見本節末。
  - 建立／刪除在 `LogObjects.cpp`（ht9045_db）：`W906_CreateLogObjects`／`W906_DestroyLogObjects`，wb_serve :4165 開機、H6 旁關機。
  - golden `SaveEventLog()` 也放在 `LogObjects.cpp`：cmydef.cpp 在 ht9045_globals，窄連結的 ctest 沒有 TMyStringList。
  - fMain.h 宣告全部 27 個成員（nullptr）。cMyDB.cpp 解 8 個閘，加 slTimeData／slProdRecordLog NULL 保護。
  - ctest `MyDB_CSV_EventLog` 把 slEventLog／as9045LogPath／asSaveEventLogPath 導到 %TEMP%。
- P2 MDB Updater：`AlarmCodeCatalog.cpp`（產生）＋`AlarmCodeUpdater.cpp`＋`ship/Error/AlarmCodeList.txt`，2,993 碼。
- P3 MyDBUpdateDB：
  - wb_serve :4165 同一行、在 CreateLogObjects 之後呼叫；fMain.h 有 AlarmCodeMap／AlarmCodeIter／AlarmCodeList／UnitNameMap。
  - DoInsertAlarmCode、MyDBIEvent 的 CSV 查表、快取建立照 golden 解閘。
  - 偏離：`AlarmCodeList.txt` 不在時先用 P2 C++ 目錄寫一份（golden 靠 SQLiteUpdater.exe）；兩個清單指標加 NULL 保護。
  - 第一次開機會寫 `D:\HT9045\Error`（新碼時重存清單、缺的 `English\JAM31nnn.dat`），同 golden。
  - 沒有 ctest 呼叫 MyDBUpdateDB，所以不加接縫。
- **MyDBIEvent 在 V906 還沒有 live 呼叫端**：golden 從 note.cpp ShowErrorMessage 叫，V906 是 canary 替身。事件 log 三支誰做，是待裁決 D-4。
  報警流程接上前，事件 log 的訊息／Type 不會變；wb_serve 的 `W906_AlarmTypeOfCode` 仍一律當成「在表裡」。
- **P4-D5 接縫**（20260927）：`common.cpp` 的 `as9045LogPath`（:240／:425）讀 `W906_HT9045LOG_ROOT`、`asSaveEventLogPath`（:303）讀 `W906_SAVEEVENTLOG_ROOT`，沒設＝golden。
  測試端：筆電的 `_ht9045_env_extra`（`tests/CMakeLists.txt` 檔尾 DEFER）已進 main，St02 已 APPEND 兩個變數（指向 build 目錄的 machine_log_scratch）。
  例外：`test_ga1_cmydb` 自己定義兩個全域為 ""；P4 已改成在 RecordChangeLogProcess 前指到 %TEMP%（20260927）。
- **S93 Save_SiteStatusLog**（20260927）：`handlerlog.cpp` 檔尾有 `W906_SaveSiteStatusLog()`（本體 :277 早就是活的）。
  呼叫端是 St01 分支的 C 路 TfSetup 存檔（`tools/editlist/TestIF_File_SetUp.py:287` GATE → REPLACE），St01 做。
- **P4＋W7（20260927，St02-E；`v906/steven-p4-wip` 本機 commit `b4712e5f`／`5c30e8cf`／`ec1b034c`＋P4 原子 commit；兩組態只編譯、未執行、未上機）**。權威：`D:\AI_TempFile\st02-p4\HT9011UC_Cpp_V3.33.906.0\docs\CMYDB_PORT_LEDGER.md` 的「P4 替身歸位」段。
  - 五個替身同一個 commit 刪掉：`SECSGEM/uHGemEquipment.cpp:3452-3500`（3 參數轉發）、`canary_support.cpp:114-123`（RecordProcess printf）與 `:523-543`（MyDBIProcessNew＋記錄器）、`acatchtray_shims.cpp:152`（NewRecordProcess 空的）；`cMyDB.cpp` 四個 homecoming gate（:926／:1002／:1864／:1892）解開。共用檔都是行數不變。
  - D1 = A：`aHotPlateSubstrate.cpp:1244-1249` 的 2 參數 `MyDBIProcess(S1, S2)` 是**轉接器** → golden 3 參數 `(S1, S2, "")`；`W906_MyDBIProcess_*` 計數器保留。D3 = A：MyDBIProcessNew 三處都沒有 `__fastcall`。
  - D2 = B：`ProductionLog` 在 `LogObjects.cpp`；`forms/fMain.h:1301` `MemoProductionLog`（vclcompat::TMemo）；開機載回當天檔（golden main.cpp:9411-9416）、關機 `ProductionLog("Close")`；接縫 `W906_RMS_ROOT`（common.cpp:263／:442）。
  - D4 = B：`TfProductionInfo::SaveMessageHistroy` 在 `forms/fProductionInfo.cpp` 檔尾；接縫 `W906_PRODINFO_ROOT`（common.cpp:293）。
  - W7 = A：`LogObjects.cpp` 建 golden 全部 24 名／58 個 log 物件；HeaterLog 經 `W906_HeaterLogHook`（cpublic.cpp:704-706＋檔尾）；`slHanaTrayMap` 定義在 `cmydef.cpp:6151`；`slAutoSiteMapLog` 衍生替換；`MainClarnData.cpp:45` 先用 `fMain->slQtyLog`。
  - 選配接縫（`5c30e8cf`，筆電可否決）：W7 路徑改 `as9045LogPath+"\\X"`、TorqueLog／ProductRecord／QtyData 跟著 as9045LogPath、`W906_HPCARD_ROOT`／`W906_GROUNDMAN_ROOT`。
  - ctest：`MyDB_P4_Containment`（`tests/test_mydb_p4_containment.cpp`）四個根目錄不是 machine_log_scratch 就拒跑、每個入口一個 token、真根目錄不可被寫；`test_ga1_cmydb`、`test_ga1_cprod` 跟著改。
  - **現在會真的寫檔**：約 947 個呼叫點＋wb_serve MyMessageBox 5 處 → HANDLER LOG csv、EventLogTxt（wb_serve）、EventTracker（MyDBIProcessNew 類）、O06 時 D:\RMS、Greatek＋N14_1 時 ProductionInfo 歷史 csv。手動跑測試 exe（沒有 ctest 環境變數）會寫真的 D:\HT9045_Log。
  - 偏離（帳本有表）：轉接器、匿名 3 參數摺疊、SaveToFile 靜默失敗、Close 在關機跑、slAutoSiteMapLog 衍生替換、SaveMessageHistroy 的 Append→Add 與空資料夾名。
- **還沒做**：P5 時間統計／生產、P6 web 頁、TraceDB（往後排）、XLS（往後排）；`MyDBIEvent` 的 live 呼叫端（D-4）；`JsonBridge/ChanAction.cpp:210-248` 的診斷字串（清單外，筆電的檔）。

## 6. CSV 版移植計畫（摘要，全文 `references/cmydb-csv-port-plan.md`）

P0 裁決落地（拔 sqlite3、刪 DB 分支與 14 個純 DB 本體、對到 V912、帳本）→ P1 建 20 個 log 物件＋接真 `TMyStringList`＋`SaveEventLogInfo/EventTracker` →
P2 `AlarmCodeCatalog.cpp`（腳本從 `CreatDatabase.cpp` 抽 32 Unit）＋`MyDBUpdateDB` 改「靜態→動態→補丁→txt」→ P3 `MyDBIEvent` 告警路徑 →
P4 五個替身歸位（一個 commit，`nm` 驗） → P5 時間統計／生產 → P6 web Event Log／Alarm Code 頁。
使用者 20260926 裁決：靜態目錄 **C++ 靜態表＋出貨 txt 兩個都要**；log 物件未回覆（暫照 golden 全建）；EventlogAnalyzer 另立計畫（`ht9045-eventlog-analyzer`）；Summary Report XLS **往後排**；DB 分支**先 `#if 0`**（不刪）。

**進度**：P0／P1／P2／P3 已推；P4＋W7（log 物件全建）20260927 在 St02 本機 commit（未推、未上機），見 §5 末；P5／P6／TraceDB 未做。產生器用法：`python tools/gen_alarmcode_catalog.py [--src <CreatDatabase.cpp>] [--check <真檔 AlarmCodeList.txt>]`。

## 7. 跨技能連動

- 告警顯示與 `Alarm-description.json`／`AlarmCodeList`：`ht9045-alarm-dismissal`。
- `log.event`／`log.tail` 通道與 JSON 中介層：`ht9045-json-bridge`。
- `TMyStringList` 換檔規則（TByDay／TBy2Hour／SaveByLotID）：`Public/MyStringList.h`；RS232 橋接程式也用同一類別（`ht9045-gpib-bridge` §8）。
- MDB Updater（SQLiteUpdater.exe）的操作程序：[references/mdb-updater.md](references/mdb-updater.md)。內容包括 AlarmCode 格式與唯一性、Unit 對照、雙端新增 AlarmCode 的程序（含 V906 產生器步驟）、情境 A～E、WAR16118／WAR16339 衝突紀錄。20260926 由舊 agent `D:\MDB Updater\.github\agents\MDBUpdater.agent.md` 併入，原檔保留；BCB 線程序仍有效。
