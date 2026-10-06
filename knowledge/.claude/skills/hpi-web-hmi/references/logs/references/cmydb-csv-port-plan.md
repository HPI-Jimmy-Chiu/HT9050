> 保存來源：`.claude/skills/ht9045-mydb/references/cmydb-csv-port-plan.md`，main `b8ea3a511`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# cMyDB CSV 版移植計畫（Handler.db3 與 sqlite3 退役，只保留 `CosFunction.bUseMDB==false` 路徑）

> 本檔為 `ht9045-mydb` 的 reference。工作副本同步放在 V906 樹
> `docs/CMYDB_20260926_CSV_PORT_PLAN.md`；兩邊內容相同，改動時兩邊一起改。

- 日期：2026-09-26
- 來源
  - golden：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cMyDB.cpp`（2,069 行）／`cMyDB.h`（83 行）；`Public/MyStringList.cpp`（1,006 行）
  - AlarmCode 靜態目錄：`D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp`（2,666 行；`CreateTableAlarmList()` 32 個 Unit；與 Rev900 逐行相同）＋ agent `D:\MDB Updater\.github\agents\MDBUpdater.agent.md`
- 目標樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`
- 使用者裁決
  - 20260923（已寫在 V906 `CosFunction.cpp:4060-4086`）：「SQLite 資料庫已經不再使用，只使用單純的 csv 存檔方式」「目前機台就是 `CosFunction.bUseMDB=false`」「C++ 實作也強制為 false」；當時**保留** cMyDB.cpp 與 sqlite3 在建置內。
  - **20260926（取代上一條的保留部分）**：「handler.db 可以不用了，sqlite3 也退役了，只做 csv 版本即可」「只需移植 `CosFunction.bUseMDB==false` 的部分」。
- 狀態：計畫，尚未動工。V906 已有 cMyDB.cpp 翻譯（2,386 行，基準 golden 906，33 個 gate），本計畫是**刪 DB 分支、補齊 CSV 分支、接上開機與畫面**。

---

> **順序調整（使用者 20260926）**：現在只做 P2（D:\MDB Updater 的轉換，已完成，見帳本）；P1、P3～P6 排到測試通訊（GPIB）完成之後；TraceDB（Tray/Plate.DB → CSV）往後排。

## 0. 一句話

`bUseMDB==false` 時 cMyDB 做的事只有四件：(1) 開機把 `AlarmCodeList.txt` 讀成 `AlarmCodeMap`，再補動態碼與補丁碼寫回；
(2) 告警發生時用 Code 查 Map 得 Type／Unit／Message，寫 `HANDLER LOG_*.csv` 與 `EventTracker.csv`；
(3) 流程／按鍵／生產／時間統計寫 `EventLogTxt` 等 `TMyStringList` CSV；(4) 讀 `AlarmCodeList.txt` 給畫面列表。
sqlite 的 19 個呼叫點全部到不了，退役後直接刪；`Handler.db3`、`SQLiteUpdater.exe`、`EventLogSaver.exe` 都不再是 C++ 線的依賴。

---

## 1. 現況盤點

### 1.1 golden（V912）在 `bUseMDB==false` 下每個本體做什麼

| 本體 | `bUseMDB==false` 行為 | 分類 |
|---|---|---|
| `MyDBOpenDB`／`MyDBCloseDB`／`MyDBExecSQL`／`MyDBVACUUM` | 早退（:72／:119／:138／:1186） | **退役**（刪） |
| `DoInsertAlarmCode(Code, Msg)` | 解 `JAM/WAR/MES`＋Unit＋Code 成 9 碼 ID；`AlarmCodeMap` 查重；不重複就 `AlarmCodeList->Add("Code=Msg")` 並 `SaveToFile("D:\HT9045\Error\AlarmCodeList.txt")`；SQL 那行到不了 | **CSV 核心** |
| `MyDBUpdateDB()` | `new AlarmCodeList/UnitNameMap`；讀 `AlarmCodeList.txt`（沒有檔時走 `MyDBQAlarmCodeList()`→無 DB 回 false→**清單空**）；`UnitNameMap` 從 `AlarmUnit[32]`；Map 建索引；6 個 `ALTER`／`DELETE`／`VACUUM` 到不了；動態碼：Cylinder `JAM31xxx`（依 `Cylinder[]`）、Temperature `WAR15xx/WAR151xx`（依 `asTempCtrl[]`）；**105 個補丁 `DoInsertAlarmCode`**（V912 比 906 多 17 個＋AMR 迴圈＋`WAR0354→WAR0357`） | **CSV 核心** |
| `MyDBIEvent(Code, MotorID, …)` | 跳過 DB 區塊；用 `AlarmCodeMap` 解 `*AlarmID/*UnitNo/*Type/*Message/*UnitName`（`24`=Motor 特例，`UnitNameMap`）；`MyDBULotEndTime()`（早退）；組 9 碼 Code；`SaveEventLogInfo(Code, Msg, JAM?0:20, " ", errPart)` | **CSV 核心** |
| `MyDBIProcess(Table, S1, S2)` | ASE 高雄 `RespondASECom`；SQL 到不了；`ProductionLog(S1+S2,true)`；組 SPIL／一般兩種欄位進 `slEventLog->AddTextWithLineNo/AddTextWithDateTime`＋`SaveEventLog()`；`SaveEventLogInfo("220000000", S1, 22)` | **CSV 核心**（全樹 100+ 呼叫點） |
| `MyDBIProcessNew(Table, Code, S1, S2)`／`NewRecordProcess`／`RecordProcess`／`RecordChangeLogProcess` | 同上形狀，帶 AlarmCode 欄 | **CSV 核心** |
| `MyDBITotalLoader`／`MyDBITimeData`／`MyDBIUPH`／`MyDBIProductionData` | SQL 到不了；`slEventLog`（TimeDataTotalLoader…）／`slTimeData`／`slProdRecordLog` CSV；`SaveEventLogInfo(…22)` | **CSV 核心** |
| `RecordTimeData(iDataType)` | 每 12 小時算 MUBA／MTBA 等寫 `slTimeData`；內含 Summary Report XLS 區塊（`SgdToXLS`，`fConfiguration->edtN04_Host`，`asProduct_LoaderPath`） | CSV 核心（XLS 區塊另裁決） |
| `SaveEventLogInfo(Code, Msg, iType, Status, ErrPart)` | 先 `SaveEventTracker`；寫 `D:\HT9045_Log\SaveEventLog\HANDLER LOG_<HandlerID>_yyyy_mm_dd.csv`（16 欄，表頭固定）；`iType==0` 時只暫存到 `aBackEventLogMessage`（等 recover 再寫）；換日 `fLotInfo->UploadEventLogFile(前一檔)` | **CSV 核心** |
| `SaveEventTracker` | 寫 `D:\HT9045_Log\ASE log\yyyy\mm\dd\<HandlerID>@yyyy_mm_dd_EventTracker.csv`（12 欄） | **CSV 核心** |
| `GetAlarmCodeList(TStringGrid*)`／`GetJameCodeOfAxis(iAxis, TComboBox*)` | 讀 `AlarmCodeList.txt` 填畫面 | CSV（畫面改 JSON） |
| `GetMyDBIMessage(Code)` | 查 `AlarmCodeMap`（Sam 20230218） | CSV |
| `MyDBQ*`（LotData／MotMess／ClearDT／TotalLoader／TimeData／AlarmCodeList）、`MyDBU*`（LotEndTime／LotData／EventRecover）、`MyDBV*`（EventFreq／Process／ProcessFilter／UnitEventCount／AxleEventCount） | 純 DB：不是早退就是對 NULL handle 查（golden 在 false 時畫面 `tsMDB` 隱藏，`cObserver.cpp:379`，所以沒人叫） | **退役**（刪本體，呼叫點改） |

### 1.2 CSV 落點（照 golden 真讀真寫，路徑不改）

| 檔 | 產生者 | 內容 |
|---|---|---|
| `D:\HT9045\Error\AlarmCodeList.txt` | `MyDBUpdateDB`／`DoInsertAlarmCode` | `CODE=Message` 一行一筆；也是 web `Alarm-description.json`（`ht9045-alarm-dismissal`）的來源 |
| `D:\HT9045_Log\EventLogTxt\…` | `slEventLog`（`TMyStringList`，golden `main.cpp:1552-1558` 建） | 8 欄：UnitName／AlarmCode／OccurDateTime／Recovery／StopedTime／Duplicate／Message／ErrPart（SPIL 格式帶時間欄；一般格式由 `AddTextWithDateTime` 補時間） |
| `D:\HT9045_Log\SaveEventLog\HANDLER LOG_<ID>_yyyy_mm_dd.csv` | `SaveEventLogInfo` | 16 欄（SiteID…ERROR MESSAGE, ErrPart） |
| `D:\HT9045_Log\ASE log\yyyy\mm\dd\<ID>@…_EventTracker.csv` | `SaveEventTracker` | 12 欄 |
| `D:\HT9045_Log\TimeData\…`／`Production_Loader\…`／`ProductRecord…` | `slTimeData`／`asProduct_LoaderPath`／`slProdRecordLog` | 時間統計、生產記錄 |
| `ProductionLog(...)`（`cpublic.cpp`） | 矽品蘇州格式 Process 文字檔 | |

### 1.3 靜態 AlarmCode 目錄從哪來（sqlite 退役後的缺口）

golden 的 `AlarmCodeList.txt` **第一份**是 `SQLiteUpdater.exe` 建 `AlarmList` 表後輸出的（`CreatDatabase.cpp` 32 個 Unit 的 `DoInsertAlarmCode` 資料列）；機台端 `MyDBUpdateDB` 只補動態碼與補丁碼。
退役 sqlite 之後，C++ 線若沒有這份靜態目錄，`MyDBUpdateDB` 從空清單起步，`AlarmCodeList.txt` 只會有 105 個補丁碼 → 大部分告警變 `Unknown Alarm Code`。
⇒ 本計畫把 `CreatDatabase.cpp` 的 32 個 Unit 資料列翻成 V906 的 **C++ 靜態表 `AlarmCodeCatalog.cpp`**（用腳本從 Big5 原始檔抽 `DoInsertAlarmCode("CODE", "Msg")`，不手抄），`MyDBUpdateDB` 先灌靜態表、再動態、再補丁、再寫 txt。這就是「MDB Updater 移植」在 CSV 版裡剩下的唯一內容。

### 1.4 CSV 時代的外掛工具（BCB 線）

- `EventlogAnalyzer.exe`（`d:\EventlogAnalyzer\`，視窗 `TfrmELA`「Event Log Analyzer」）：golden `bUseMDB==false` 時由 `main.cpp:18611` 啟動、`InterfaceSYS.cpp:481 SendCommand_EventLog()` 用 `WM_COPYDATA` 送指令；它讀的就是上面的 CSV。
- `EventLogSaver.exe`（`bUseMDB==true` 才用）與 `SQLiteUpdater.exe`：C++ 線不再需要。
- 依「BCB 接 BCB、C++ 接 C++」原則：C++ 線用 web 頁讀 CSV 取代 `EventlogAnalyzer.exe`；BCB 線工具照舊。

### 1.5 V906 現況

| 項目 | 事實 | 位置 |
|---|---|---|
| `cMyDB.cpp` | 2,386 行，基準 golden 906；30 個 ACTIVE 本體，`bUseMDB` 守衛 13 處（比 golden 6 處多，沒掉守衛） | 檔頭 banner |
| 建置 | `ht9045_db` = `database.cpp`＋`cMyDB.cpp`，連 `sqlite3`（`third_party/sqlite3`，3.7.7.1） | `CMakeLists.txt:1218-1247` |
| `bUseMDB` | `CosFunction.cpp:4086` 於 switch 之後強制 `false`（20260923 裁決，附 26 行說明） | |
| 33 個 `#if 0` gate 中屬 CSV 路徑的 | `fMain->AlarmCodeMap/AlarmCodeList/AlarmCodeIter/UnitNameMap`（276／783／830／849／1254／2147）、`slEventLog` 型別不完整（344／400／448／745）、`fMain->slTimeData/slProdRecordLog`（631／709）、`fObserver` 三個時間（1349-1384）、`fContactCT`／`fSecurity`／`fConfiguration`（645／1615／511）、`fLotInfo->edtASECL_TesterID/cbbASECL_LoginMode/UploadEventLogFile`（1924／2042／2112）、`as9045LogPath`／`asSaveEventLogPath`（1973／2088：**V906 `common.cpp:240/303` 其實已有**，gate 過期）、借住 ×4（923／999／1802／1830）、TChart ×2（1426／1450，退役） | `cMyDB.cpp` |
| `DoInsertAlarmCode` | 現行 ACTIVE fallback 只剩 SQL（到不了）；**txt 鏡射被 gate 掉** → 現在什麼都不寫 | `cMyDB.cpp:276-303` |
| `TMyStringList` | `Public/MyStringList.cpp` 1,288 行已翻（PT-MyStringList 20260807）；但 `cmydef.h` 只有前置宣告，cMyDB 的 gate 還沒接上真型別 | `Public/MyStringList.h:152/201` |
| log 物件 | golden `TfMain` 建構子建約 20 個 `TMyStringList`（`main.cpp:1552-1690`：EventLogTxt／2DMapping／MNetLog／TTL／Heater／JamAlarm／SocketId／TimeData／IndexPos／Qty／ProdRecord／ASM／LotInfo／TriTemp／DewPoint…）；**wb_serve 只建了 3 個**（`MyBinDisp.cpp:183`、`MainClarnData.cpp:62`、`cObserver.cpp:3367`），`slEventLog` 是 NULL → CSV 路徑全部 `if(slEventLog!=NULL)` 跳過 | grep |
| 借住替身 | `MyDBIProcess`(3-arg)→`uHGemEquipment.cpp:3483` 轉發→`aHotPlateSubstrate.cpp:1264` **計數式空槽**；`MyDBIProcessNew`→`canary_support.cpp:453`；`RecordProcess`→`canary_support.cpp:102`；`NewRecordProcess`→`acatchtray_shims.cpp:132` | `cMyDB.h` HOMECOMING NOTICE；`JsonBridge/ChanAction.cpp:215-244` 的 20260923 實測 |
| 畫面 | `cObserver` 的 `btnQueryEventLogTxtClick`（讀 EventLogTxt 查詢）已 ACTIVE；`JsonBridge/EventLog.cpp` 環形緩衝＋`log.tail`；`log.db` tag 寫死 false | |
| 路徑全域 | `as9045LogPath`／`asSaveEventLogPath` 已在 `common.cpp`；`asProduct_LoaderPath` 未見 | |

---

## 2. 目標架構

```
 wb_serve 開機 ── ReadGeneralIni ─► CreateLogObjects()（golden TfMain ctor 那 20 個 TMyStringList）
                                  ─► MyDBUpdateDB()：AlarmCodeCatalog（靜態 32 Unit）→ 動態（Cylinder／Temp）→ 補丁 105 → AlarmCodeList.txt
 tick 執行緒 ── 告警 ─► MyDBIEvent ─► AlarmCodeMap ─► SaveEventLogInfo / SaveEventTracker（CSV）
            ── 流程／按鍵／生產 ─► MyDBIProcess* / Record* ─► slEventLog / slTimeData / slProdRecordLog（CSV）＋ JsonBridge EventLog ring
 瀏覽器 ── /api/log/files（列 CSV）／log.tail（環形）／AlarmCodeList → Event Log 頁、Alarm Code 頁（取代 EventlogAnalyzer.exe）
 wb_serve 關機 ── 各 TMyStringList MySaveToFile()（golden FormDestroy 行為）
```

- **無 sqlite**：`third_party/sqlite3` 從建置移除，`ht9045_db` 不再連它；`cMyDB.cpp` 的 DB 分支整段刪除（不是 gate），檔頭記「AI(W906-CSVONLY) 20260926 使用者裁決：sqlite3 退役」。golden 原文留在 golden 樹，不在 V906 留 `#if 0` 屍體（與 20260923「程式碼留著」的做法相反，依新裁決）。
- **執行緒**：全部在 tick 執行緒（golden 都在主執行緒）；`TMyStringList::MySaveToFile` 是同步檔案寫入，`AutoSave` 依 `SaveType`（TByDay／TBy2Hour）換檔；`cStateRecord` 已有的 detached 寫檔執行緒不用來寫這些 log（照 golden 同步寫）。
- **設定**：`IniConfig.bSPILFunction`（欄位格式）、`IniConfig.SocketHandlerID`（檔名）、`asEventLogAutoSavePath`；沿用既有 `IniConfig`，不新增旗標。

---

## 3. 目錄與建置

```
HT9011UC_Cpp_V3.33.906.0/
  cMyDB.cpp / cMyDB.h            ← 改成 CSV 版（刪 DB 分支、解 CSV gate、對到 V912 的 105 補丁）
  AlarmCodeCatalog.cpp/.h        ← CreatDatabase.cpp 32 個 Unit 的靜態碼（腳本產生，Big5→UTF-8）
  cmydef.h                       ← `class TMyStringList;` 改 include Public/MyStringList.h（或在 cMyDB.cpp 直接 include）
  forms/fMain.*                  ← AlarmCodeMap/AlarmCodeList/UnitNameMap 移到 cMyDB 自有全域，facade 只留存取器
  LogObjects.cpp                 ← golden main.cpp:1552-1690 的 20 個 TMyStringList 建構／解構（開機／關機各一函式）
  JsonBridge/EventLog.cpp        ← 移除 log.db；加 /api/log/files（列 EventLogTxt／SaveEventLog 的 CSV）
  tools/gen_alarmcode_catalog.py ← 從 D:\MDB Updater\...\CreatDatabase.cpp 抽 DoInsertAlarmCode → AlarmCodeCatalog.cpp
  tests/test_mydb_csv_*.cpp
  docs/CMYDB_PORT_LEDGER.md      ← golden 912 基準、MDB Updater Rev902 基準、補丁清單、刪除清單
```

- CMake：移除 `sqlite3` target 與 `ht9045_db` 對它的 link；`ht9045_db` 加 `AlarmCodeCatalog.cpp`、`LogObjects.cpp`。
- ctest 一律寫到暫存目錄（`as9045LogPath` 在測試內改指向 temp；這是既有 `W906_*_PATH` 唯讀接縫的同類做法）。

---

## 4. 分階段與驗收 gate

| 階段 | 內容 | 驗收 |
|---|---|---|
| **P0 裁決落地＋帳本**（半天） | `CosFunction.cpp:4060-4086` 註解改成新裁決；`cMyDB.cpp` 的 DB 分支與 14 個純 DB 本體包 `#if 0 // AI(W906-CSVONLY)`（裁決 5：先 if 0）、呼叫點盤點；對到 V912（補丁區 23 行差異）；`docs/CMYDB_PORT_LEDGER.md` | `build.bat gate` 綠；cMyDB.cpp.obj 對 `sqlite3_*` 無未解析參照；ctest 失敗清單不變 |
| **P1 log 物件與 CSV 骨幹** | `LogObjects.cpp`（照 golden 20 個，路徑照抄）；`cmydef.h` 接真 `TMyStringList`；解 `slEventLog`／`slTimeData`／`slProdRecordLog` 四個 gate；`SaveEventLogInfo`／`SaveEventTracker` 解 `fLotInfo`／路徑 gate（路徑全域已在 `common.cpp`）；關機 flush | `test_mydb_csv_eventlog`：`MyDBIProcess("Process","x")` 後 `EventLogTxt` 當日檔多一行、`HANDLER LOG_*.csv` 多一行、欄位數 8／16；SPIL 格式切換正確 |
| **P2 AlarmCode 目錄** | `tools/gen_alarmcode_catalog.py` → `AlarmCodeCatalog.cpp`（32 Unit）；`MyDBUpdateDB` 改「靜態表 → 動態 → 補丁 → 寫 txt」；`DoInsertAlarmCode` 解 gate（Map 查重＋txt 鏡射）；重複碼／跨 TYPE 同數字段檢查照 MDB agent 規則 | `test_mydb_alarmcode`：產出的 `AlarmCodeList.txt` 與 SQLiteUpdater Rev902 在 BCB 機台產出的檔逐行相同（除補丁區順序）；`WAR16118/WAR16126`、`WAR16339` 兩個歷史衝突碼結果與 agent 記錄一致；無重複 |
| **P3 告警路徑** | `MyDBIEvent` 解 gate（Map／UnitNameMap／`24`=Motor 特例）；`GetMyDBIMessage`；`MyDBULotEndTime` 呼叫移除 | `test_mydb_event`：`JAM0101` → Type 1／Unit 1／"Input Arm"／訊息正確 → `HANDLER LOG` 暫存（iType 0 不立即寫）與 `EventTracker` 各一行；未知碼 → `Unknown Alarm Code`／ID 41 |
| **P4 替身歸位** | 一個 commit：刪 `uHGemEquipment.cpp:3483`／`aHotPlateSubstrate.cpp:1264`／`canary_support.cpp:102,453`／`acatchtray_shims.cpp:132` 五個替身，解開 cMyDB.cpp 四個 homecoming gate；`MyDBIProcessNew` 補 `__fastcall`；ATC／Automation／AutoClean 的 `MyDBIProcess("Exception",…)` 真的落 CSV。**20260927 做法（St02-E，本機 commit，未上機）**：2 參數那支改成轉接器（D1=A）、`__fastcall` 不補（D3=A）、ProductionLog／SaveMessageHistroy 一起翻（D2=B／D4=B）、log 物件全建（W7=A），詳見帳本「P4 替身歸位」段 | link 綠；`nm` 每個符號一個 `T`；`JsonBridge/ChanAction.cpp:215-244` 的診斷字串改成新事實（**還沒改**：清單外，筆電的檔）；ctest `MyDB_P4_Containment` |
| **P5 時間統計與生產** | `MyDBITotalLoader`／`MyDBITimeData`／`MyDBIUPH`／`MyDBIProductionData`／`RecordTimeData` 解 `fObserver` 時間、`fConfiguration->edtN04_Host` gate；Summary Report XLS 區塊（`SgdToXLS`）依裁決 4 | `test_mydb_timedata`：12 小時邊界觸發寫 `TimeData` CSV |
| **P6 畫面** | `/api/log/files`＋Event Log 頁（讀 CSV，取代 EventlogAnalyzer.exe）；Alarm Code 頁（`GetAlarmCodeList`／`GetJameCodeOfAxis` 改回傳 JSON）；`log.db` tag 移除 | 三個 API 離線用暫存 CSV 能顯示；`cObserver` 的 `tsMDB` 分頁在 web 端不存在 |

工作量：P0 主要是刪（約 -600 行）；P1 ≈400 行；P2 腳本＋約 2,000 行資料列；P3～P5 ≈500 行；P6 ≈400 行。**沒有 sqlite 之後，這個模組沒有任何外部依賴。**

---

## 5. 裁決（使用者 20260926 回覆）

| # | 題目 | 裁決 | 對計畫的影響 |
|---|---|---|---|
| 1 | 靜態 AlarmCode 目錄形式 | **兩個都要**：C++ 靜態表＋出貨一份 `AlarmCodeList.txt` | P2：`AlarmCodeCatalog.cpp` 是來源；建置時同時產出一份基準 `AlarmCodeList.txt`（放 `machines/common/Error/`，隨機台出貨）。開機時 `MyDBUpdateDB` 若磁碟上沒有 txt 就用靜態表建，有就讀檔再補，行為與 golden「先讀檔」一致 |
| 2 | 20 個 `TMyStringList` log 物件 | **未回覆** | 暫依建議「照 golden 全建」（忠實）；P1 先做 cMyDB 用到的 4 個，其餘 16 個在同一個 `LogObjects.cpp` 裡一次補齊。若要只建 4 個，說一聲即可撤回 |
| 3 | EventlogAnalyzer.exe 取代時機 | **另立計畫**，與 `tsEventLogTxt`／`TfObserver::GetEventLogText()` 有關 | 已立：`ht9045-eventlog-analyzer` skill ＋ `docs/ELA_20260926_WEB_CONVERSION_PLAN.md`；本計畫 P6 只做到「CSV 能被讀」，畫面交給 ELA 計畫 |
| 4 | Summary Report XLS 區塊 | **可以翻，但往後排** | P5 先 gate XLS 區塊（`#if 0`，帳本記「往後排」）；排在三項工作之後 |
| 5 | DB 分支刪或留 | **先 `#if 0` 沒關係** | P0 改成：DB 分支與 14 個純 DB 本體包 `#if 0 // AI(W906-CSVONLY)`，不刪；sqlite3 因此沒有活的呼叫點，建置上可拔可留，先留著不動建置（最小改動） |
| — | RS232Standard 的 FindWindow 加 HT-9050 | **不處理** | （測試通訊計畫的事項，記在此供對照） |
---

## 6. 風險

- **`AlarmCodeList.txt` 是 web 告警描述的來源**（`ht9045-alarm-dismissal` 的 `Alarm-description.json`）：P2 之前 C++ 機台的這個檔若是舊 BCB 機台帶來的，內容仍正確；P2 之後由 V906 產生，格式必須逐行一致，否則 web 告警描述會斷。
- **`MyDBIProcess` 一族 100+ 呼叫點在 P4 那一刻開始真的寫檔**：`slEventLog==NULL` 守衛保住不當機，但 `SaveEventLogInfo` 直接 `fopen`；`asSaveEventLogPath` 目錄不存在時 `MyForceDirectories` 會建；磁碟滿或路徑無權限時 `fopen` 回 NULL → golden 直接 return，行為一致。
- **編碼**：CSV 內容 BCB 是 Big5，V906 是 UTF-8；`EventlogAnalyzer.exe`（BCB）讀 V906 寫的 CSV 會亂碼——C++ 線不用它，所以不處理；但若同一台機台 BCB／C++ 交替執行，`EventLogTxt` 同一檔會混編碼。列入帳本，不在本計畫解。
- **`TMyStringList` 換檔規則**（`TByDay`／`TBy2Hour`／`SaveByLotID`／`SaveSameFolder`）是客戶客製的重災區（矽格湖口、北興、力成…）：P1 只接 golden 預設，客製旗標照 `IniConfig` 走，ctest 各一組。
- `cObserver` 的 MDB 分頁與 `MyDBV*` 畫面：web 端沒有對應頁，刪本體後要確認 `cObserver.cpp` 的呼叫點都在 `tsMDB` 隱藏路徑內（golden `:379`），否則會有孤兒呼叫。

<!-- preserved-content:end -->
