# MDB Updater（SQLiteUpdater.exe）參考

> 來源：舊的 VS Code agent `D:\MDB Updater\.github\agents\MDBUpdater.agent.md`（原檔保留在原處）。
> 20260926 併進本 skill（St02，使用者指示「併進既有 skill，不另開 skill」）。只搬知識；agent 外殼
> （frontmatter 的 tools、「你是…專家」、argument-hint、個人路徑的 skill 路由表）沒有搬。
> BCB 線的程序照舊有效；V906（C++）的對應作法標在「V906」段落。

---

## 1. 這支程式是什麼

`SQLiteUpdater.exe`（MDB Updater）重建機台的 AlarmCode 靜態清單，並輸出 `AlarmCodeList.txt`。它和 HT9045 機台端 `cMyDB.cpp` 的 `MyDBUpdateDB()` 高度耦合：兩邊合起來才是一台機台看到的完整 AlarmCode。

| 項目 | 路徑 |
|---|---|
| 版本目錄 | `D:\MDB Updater\MDB_Updater_Rev<Rev>.<Minor>_<YYYYMMDD>\`。20260926 磁碟上有 `Rev900.0_20260331` 與 `Rev902.0_20260410`；agent 原本寫的「最新」是 900.0，V906 產生器讀的是 **902.0** |
| 備份目錄 | `D:\MDB Updater\backup\` |
| 輸出 DB（BCB 線） | `D:\HT9045\MDB\Handler.db3` |
| AlarmCode 清單 | `D:\HT9045\Error\AlarmCodeList.txt` |
| 更新 Log | `D:\HT9045_Log\MDB_UpdateLog\` |

### 核心檔案

| 檔案 | 說明 |
|---|---|
| `CreatDatabase.cpp` | DB Schema 建立與所有靜態 AlarmCode 定義（主要修改檔） |
| `CreatDatabase.h` | 函式宣告 |
| `Main.cpp` | `UpdateAlarmList()`、`TraceDB()`、`DoInsertAlarmCode()`、`DoUpdate()` |
| `Main.h` | Form 類別與外部宣告 |
| `cMyDef.cpp/h` | 版本查詢工具 `VerInfo` |
| `SQLiteUpdater.bpr` | BCB6 專案檔 |

### 執行流程

```
SQLiteUpdater.exe 啟動
  └─ Timer1Timer()                         [Main.cpp]
       ├─ Task=100~120: TraceDB() × 2      ← 同步 Tray.DB / Plate.DB → CSV
       ├─ Task=200~220: UpdateAlarmList()  ← DB 主更新流程
       │     ├─ 開啟 Handler.db3
       │     ├─ DROP TABLE AlarmList       ← 全量重建
       │     ├─ CreateTable()              ← 建立所有 Table + CreateTableAlarmList()
       │     ├─ CreateView()               ← 建立 View（EventLogView 等）
       │     ├─ DoSaveEventLog()           ← 備份並刪除一年前 Log
       │     ├─ MyDBCloseDB()
       │     └─ SaveUpdateLog()            ← 寫入 MDB_UpdateLog
       └─ Task=300: Close()
```

**V906**：sqlite 已退場（使用者 20260926）。V906 保留的只有「產生 AlarmCode 清單並寫 `AlarmCodeList.txt`＋更新 log」：
- `AlarmCodeCatalog.cpp`：由 `tools/gen_alarmcode_catalog.py` 產生。
- `AlarmCodeUpdater.cpp`：golden `UpdateAlarmList()` 的 CSV 版。log 檔名改成 `AlarmCodeUpdateLog_…`。
- TraceDB（Tray.DB／Plate.DB → CSV 一次性轉檔）往後排。
- 細節在 `docs/CMYDB_PORT_LEDGER.md` 與本 skill 的 `cmydb-csv-port-plan.md`。

---

## 2. ⚠ 核心耦合：CreateTableAlarmList ↔ MyDBUpdateDB

**MDB Updater 端（`CreatDatabase.cpp`）**
- `CreateTableAlarmList()` 先 Insert `WAR000000`，再依序呼叫 `_001`～`_031` 共 31 個子函式（agent 原文寫「32 個」），**完整重建** AlarmList，Insert 所有靜態 AlarmCode。
- 執行後輸出 `D:\HT9045\Error\AlarmCodeList.txt`。

**HT9045 機台端（`cMyDB.cpp`）**
- `MyDBUpdateDB()` 在機台**每次開機**時執行，載入 `AlarmCodeList.txt` 到 `AlarmCodeMap`（執行期查詢用）。
- 另外呼叫 `DoInsertAlarmCode()` 插入**動態／補丁**型 AlarmCode：
  - **Unit 31（Cylinder）**：依 `Cylinder[]` 自動生成 `JAM31xxx`
  - **Unit 15（Temperature）**：依 `asTempCtrl[]` 自動生成 `WAR15xx`／`WAR151xx`
  - **補丁碼**：已部署機台缺少的碼，直接在 `MyDBUpdateDB()` 補入

| 情境 | 修改位置 |
|---|---|
| 新增靜態 AlarmCode（隨 MDB Updater 發版） | `CreatDatabase.cpp` 對應的 `CreateTableAlarmList_XXX_()` |
| 新增動態 AlarmCode（Cylinder／Temperature 自動生成） | `cMyDB.cpp` `MyDBUpdateDB()` 的生成邏輯 |
| 補丁既有機台缺少的 AlarmCode | `cMyDB.cpp` `MyDBUpdateDB()` 末段 `DoInsertAlarmCode()` |
| 修改 DB Schema（新增欄位／Table） | `CreatDatabase.cpp` `CreateTable()` 對應函式（BCB 線；V906 沒有 sqlite） |
| 修改 View | `CreatDatabase.cpp` `CreateView()`（同上） |

**V906**：`AlarmCodeCatalog.h:14` 註明產生的清單**還沒接進開機**。之後由 V906 `cMyDB.cpp:2229` `MyDBUpdateDB()` 使用，排在測試通訊之後。

---

## 3. AlarmCode 格式與唯一性

```
格式： {TYPE}{UnitNo:02d}{Code:04d}
範例： JAM0101, WAR16446, MES2820

TYPE:
  JAM → AlarmType=1 (機台停止型警報)
  WAR → AlarmType=2 (警告型，可繼續)
  MES → AlarmType=3 (訊息/操作提示)

UnitNo（2位）= Code.SubString(4,2)  [1-based]
Code（4位）  = Code.SubString(6,4)  [1-based]

AlarmID（9碼）= AlarmType×100000000 + UnitNo×10000000 + Code
例：JAM0101 → 1×10^8 + 01×10^7 + 0001 = 110000001
```

- **AlarmCode 唯一**：同一個 Code 只能出現一次，MDB Updater 與 `MyDBUpdateDB` 合計。重複時 golden `DoInsertAlarmCode` 保留**第一個**，並記 `@@Error!!` 與 `Alarm Code: <code> is duplicate!!`。V906 產生器與 `AlarmCodeUpdater.cpp` 照同樣行為。
- **數字段唯一**：同一個數字段（例：`16119`）不可被不同 TYPE（WAR／JAM／MES）共用。新增前**必須確認同數字段的三種 TYPE 都沒被用**。
- **Unit 31 保留給動態**：`CreateTableAlarmList_031_Cylinder()` 函式體保持空白，由機台端動態生成。

### Unit 編號對照

| Unit | 分類 | 函式 |
|---|---|---|
| 01 | Input Arm | `CreateTableAlarmList_001_InArm()` |
| 02 | Output Arm | `CreateTableAlarmList_002_OutArm()` |
| 03 | Index Unit | `CreateTableAlarmList_003_Index()` |
| 04 | Input Shuttle | `CreateTableAlarmList_004_InSht()` |
| 05 | Output Shuttle | `CreateTableAlarmList_005_OutSht()` |
| 06 | Tray Arm | `CreateTableAlarmList_006_TrayArm()` |
| 07 | Tester I/F | `CreateTableAlarmList_007_Test()` |
| 08 | Scanner | `CreateTableAlarmList_008_Scanner()` |
| 09 | Tray Loader | `CreateTableAlarmList_009_Loader()` |
| 10 | Empty Tray | `CreateTableAlarmList_010_Empty()` |
| 11 | Auto 1 | `CreateTableAlarmList_011_Auto1()` |
| 12 | Auto 2 | `CreateTableAlarmList_012_Auto2()` |
| 13 | Auto 3 | `CreateTableAlarmList_013_Auto3()` |
| 14 | Color Tray | `CreateTableAlarmList_014_Color()` |
| 15 | Temperature | `CreateTableAlarmList_015_Temperature()`＋`MyDBUpdateDB()` 動態生成 |
| 16 | System | `CreateTableAlarmList_016_System()` |
| 17 | Fix Tray 1 | `CreateTableAlarmList_017_Fix1()` |
| 18 | Fix Tray 2 | `CreateTableAlarmList_018_Fix2()` |
| 19 | Fix Tray 3 | `CreateTableAlarmList_019_Fix3()` |
| 20 | ESD | `CreateTableAlarmList_020_ESD()` |
| 21 | Process Log | `CreateTableAlarmList_021_Process()` |
| 22 | Motion Log | `CreateTableAlarmList_022_Motion()` |
| 23 | Cassette | `CreateTableAlarmList_023_Cassette()` |
| 24 | Motor | `CreateTableAlarmList_024_Motor()`（依 MotorList 自動生成：164 個馬達名 × 9 種訊息，`WAR24%03d%d`） |
| 25 | Auto 4 | `CreateTableAlarmList_025_Auto4()` |
| 26 | Auto 5 | `CreateTableAlarmList_026_Auto5()` |
| 27 | Auto 6 | `CreateTableAlarmList_027_Auto6()` |
| 28 | Fix Tray 4 | `CreateTableAlarmList_028_Fix4()` |
| 29 | Fix Tray 5 | `CreateTableAlarmList_029_Fix5()` |
| 30 | Fix Tray 6 | `CreateTableAlarmList_030_Fix6()` |
| 31 | Cylinder | `CreateTableAlarmList_031_Cylinder()` → **空函式**，由 `MyDBUpdateDB()` 動態生成 |

---

## 4. DB Schema（BCB 線 `Handler.db3` 主要 Table；V906 沒有 sqlite）

| Table | 說明 |
|---|---|
| `AlarmList` | 靜態警報清單（MDB Updater 全量重建） |
| `EventLog` | 警報事件紀錄（機台執行時寫入） |
| `Message` | 訊息紀錄 |
| `Motion` | 動作紀錄 |
| `Process` | 流程紀錄 |
| `Production` | 生產數量紀錄 |
| `TimeData` | 機台時間統計 |
| `TotalLoader` | 取料計數 |
| `UnitName` | Unit 名稱對照 |
| `MotorAlarmList` | 馬達警報對照 |
| `AxleName` | 軸名稱 |
| `LotInfo` | 批次資訊 |
| `ClearDateTime` | 清除時間紀錄 |
| `Customer` | 客戶資料 |

主要 View：`AlarmHistoryView`、`EventLogView`、`ProductionView`。

---

## 5. 新增 AlarmCode 作業程序（雙端同步）

### Step 1：決定放在哪一端
```
靜態碼？→ CreatDatabase.cpp 對應 CreateTableAlarmList_XXX_()
動態碼？→ cMyDB.cpp MyDBUpdateDB() 末段
```
先照第 3 節確認 Code 與數字段都沒被用過。

### Step 2：MDB Updater 端（`CreatDatabase.cpp`，Big5）
```cpp
// 在對應的 CreateTableAlarmList_XXX_() 函式末段加入：
DoInsertAlarmCode("JAM0XXX", "Error message here!");
DoInsertAlarmCode("WAR0XXX", "Warning message here!");
DoInsertAlarmCode("MES0XXX", "Info message here!");
```

### Step 3：HT9045 端（`cMyDB.cpp`，補丁模式，讓已部署的機台也有這個碼）
```cpp
// 在 MyDBUpdateDB() 末段加入：
DoInsertAlarmCode("JAMxxxx", "Error message here!");
```

### Step 4：驗證（BCB 線）
- 編譯 MDB Updater（BCB6 `bpr2mak`＋`make`，見 skill `bcb_build`）。
- 確認沒有重複 Code（重複會輸出 `@@Error!! ... is duplicate!!`）。
- 執行 `SQLiteUpdater.exe`，確認 Memo1 顯示 OK、沒有 `@@Error`。

### V906（C++）的對應步驟
1. 一樣改 `D:\MDB Updater\MDB_Updater_Rev902.0_20260410\CreatDatabase.cpp`（產生器的預設來源）。來源換版時用 `--src` 指定。
2. 在 `HT9011UC_Cpp_V3.33.906.0\` 下重跑 `python tools/gen_alarmcode_catalog.py`，重新產生：
   - `AlarmCodeCatalog.cpp`（UTF-8、CRLF）
   - `ship/Error/AlarmCodeList.txt`（ASCII、CRLF）

   產生器會回報重複碼。可以加 `--check <真機的 AlarmCodeList.txt>` 比對真檔。
3. 交付時帶 `ship/Error/AlarmCodeList.txt`，裝到機台 `D:\HT9045\Error\AlarmCodeList.txt`。
4. 跑 ctest `MyDB_CSV_AlarmCode`。
5. 兩個檔都要 commit。**不要**手改 `AlarmCodeCatalog.cpp`，它是產生的。

---

## 6. 常見作業情境

### 情境 A：新增一個 Tray Loader 相關的 JAM 警報
```
1. 在 CreatDatabase.cpp 的 CreateTableAlarmList_009_Loader() 末段加入：
   DoInsertAlarmCode("JAM09XX", "message");
2. 同步在 cMyDB.cpp 的 MyDBUpdateDB() 末段加入同一筆（補既有機台）
3. 同步在 HT9045 相關流程呼叫 MyDBIEvent("JAM09XX", ...)
（V906：第 1 步之後重跑產生器，見第 5 節）
```

### 情境 B：修改 DB Schema（新增 Table 欄位，BCB 線）
```
1. 在 CreateTable() 中對應 Table 的 CREATE TABLE 語句加入欄位
2. 在 cMyDB.cpp 的 MyDBUpdateDB() 加入 ALTER TABLE ... ADD COLUMN ...
   （UPDATE 時不會重建 Table，只有 MDB Updater 才全量重建）
```

### 情境 C：新增 Motor 到 Unit 24
```
1. 在 CreateTableAlarmList_024_Motor() 的 MotorList->Add("MNewMotor") 處加入
2. 對應在 HT9045 的 motor define 確認名稱一致
（V906：產生器保留 MotorList／AlarmList 兩個清單，C++ 重播同一個雙迴圈，重跑產生器即可）
```

### 情境 D：版本升版
```
1. 複製現有版本目錄，命名為 MDB_Updater_RevXXX.X_YYYYMMDD
2. 修改 SQLiteUpdater.bpr 版本資訊
3. 版本號格式：Rev{主版本}.{次版本}（如 Rev900.0）
4. 修改前先把目錄複製到 backup\ 保存舊版本
（V906：產生器的 DEFAULT_SRC 指向 Rev902.0，升版後要更新它或用 --src）
```

### 情境 E：把 MyDBUpdateDB() 的補丁碼升格為靜態碼（定期同步）

`MyDBUpdateDB()` 末段累積多筆補丁後，應定期移進 `CreatDatabase.cpp` 對應的靜態函式，讓新部署的機台不需要依賴補丁。

- **可升格**：純靜態訊息（不是依 `Cylinder[]`／`asTempCtrl[]` 動態產生）。
- **不可升格**：`JAM31xxx`（Cylinder 動態）、`WAR15xx`／`WAR151xx`（Temperature 動態）。

```
1. 讀出 MyDBUpdateDB() 末段所有 DoInsertAlarmCode() 呼叫
2. 對每個 Code，grep 確認 CreatDatabase.cpp 是否已有同碼
3. 沒有 → 加入對應 CreateTableAlarmList_XXX_() 末段
4. 已有但訊息不同 → 記為「衝突碼」，人工裁量後再改（見第 7 節）
5. 升格後，cMyDB.cpp 的那一行可以保留（重複 Insert 會記 @@Error 但不影響運作），
   或加註 // Moved to CreatDatabase.cpp
```

**Rev900.0（20260331）已同步的碼：**

| Code | 加入函式 | 說明 |
|---|---|---|
| `MES0105`、`MES0106` | `001_InArm` | Device pick-up error on the tray |
| `WAR04200`～`WAR04207` | `004_InSht` | Barcode Compare／CheckSum／Pin1 Error |
| `WAR0732`、`MES0736` | `007_Test` | 2DID allow list／Tester PAUSE |
| `WAR07402`～`WAR07406` | `007_Test` | Site Mapping Detect Fail 系列 |
| `WAR07451` | `007_Test` | Barcode Pin1 Function Enabled |
| `MES16334` | `016_System` | Loader Optical Gate Abnormal |
| `WAR16500` | `016_System` | Local Recipe Switching Failed |
| `WAR1487`、`WAR14061`～`WAR14066` | `014_Color` | Tray ID／AI 視覺殘料 |
| `WAR2098` | `020_ESD` | Loader Ion Gun Alarm |
| `MES2813`、`MES2913`、`MES3013` | `028`／`029`／`030_Fix4/5/6` | NO ReTest BIN ID scan |

---

## 7. ⚠ 已知衝突碼（同編號、不同訊息）

`MyDBUpdateDB()` 的重複 Insert 因為 `AlarmCodeMap` 已經有同碼而**靜默失效**，實際顯示的是 MDB Updater 那一側的訊息。

| Code | `CreatDatabase.cpp` 的訊息 | `MyDBUpdateDB()` 的訊息 | 狀態 |
|---|---|---|---|
| `WAR16118` | `MD5 check fail!` | `Auto site mapping function is OFF!` → 已改用 `WAR16126` | ✅ 已解決（20260331） |
| `WAR16339` | `Loader Tray ID no respond Read Error` → 已改為 `Tray ID Read Duplicate Error!` | `Tray ID Read Duplicate Error!` | ✅ 已解決（20260331） |

### WAR16118 解決紀錄（20260331）
- **原因**：JerryYang 20230327 新增 ASE_CL Auto Site Mapping 檢查時，誤用了既有的 `WAR16118`（"MD5 check fail!"）。
- **修正**：較晚建立的「Auto site mapping function is OFF!」改用新碼 **`WAR16126`**。曾一度選 `WAR16119`，但 **`MES16119` 已存在**（Chiller water leakage alarm），數字段不可跨 TYPE 共用，所以改 `WAR16126`。
- **異動範圍**：
  - `CreatDatabase.cpp`：`WAR16125` 之後新增 `WAR16126`
  - `csystem.cpp` 約 :6071：`ShowErrorMessage("WAR16118"...)` → `WAR16126`（`#ifdef FOR_ASECL_L8`）
  - `cSecurity.cpp` 約 :1267：`sJamCode=="WAR16118"` → `WAR16126`（`CC_ASE_CL` 權限判斷）
  - `cMyDB.cpp` `MyDBUpdateDB()`：補丁 `DoInsertAlarmCode("WAR16118"...)` → `WAR16126`
  - `uLotInfo.cpp`：**保留 `WAR16118`**（那裡是正確的 MD5 check fail 用途）
- 以上行號是 BCB 線當時的版本。

### WAR16339 解決紀錄（20260331）
- **原因**：`WAR16339` 只在 `cMyDB.cpp` 補丁裡有 `DoInsertAlarmCode`，實際流程沒用到，訊息可以直接覆蓋。
- **修正**：`CreatDatabase.cpp` 的訊息由 `"Loader Tray ID no respond Read Error"` 改為 `"Tray ID Read Duplicate Error!"`。

---

## 8. 約束條件（BCB 線）

- **Big5**：MDB Updater 的 `.cpp`／`.h` 一律 Big5（CP950），禁止轉 UTF-8。V906 產生器讀它時用 cp950 解碼。
- **BCB6 C++98**：不可用 C++11 以上語法。
- **AlarmCode 唯一**與**數字段唯一**：見第 3 節。
- **Unit 31 保留給動態**：見第 3 節。
- **備份**：修改前先把目錄複製到 `backup\`。
