# Index Cycle Time / Test Time 記錄機制 Reference

> 適用：HT9045 / HT9046LS / HT9011UC 主線（Observer「Time Info」頁、SECS SVID、`INDEXCYCLETIME?`、Config `[D70]`）
> 驗證版本：`HT9046LS_Code_V3.32.745.9_Beta_20250503`、主線 `HT9011UC_Code_V3.33.910.0_20260716`（兩版結構相同，僅行號位移）
> **行號僅供定位，實作前一律重新 grep 函式名。**

---

## 1. 一句話結論

**`Index Cycle Time` 記的不是 index 手臂的動作時間，而是「上一顆測試結束(EOT) → 這一顆測試開始(SOT)」的整段空檔。**
兩次測試之間 Handler 做的所有事情（含等待、retry、停機）全部被算進去；唯一不含的是測試時間本身。

想看純 index 換手動作，要看隔壁那一欄 `Index Time`。

---

## 2. Observer「Time Info」三欄不可混用（＋右側獨立第 4 欄）

畫面位置：Observer → `tsTestInfo` → `pgcTestInfo` → **Test Time** 頁（同層另有 `TimeData` / `LoadInfo` 兩頁）。
該頁其實有**兩個** StringGrid：左邊 `TimeInfoGrid`（6 欄）＋右邊 `TimeInfoGrid_InArm`（Left=744，2 欄、**無標題**）。

| 欄位 | 標題 | 內容 | 資料來源 |
|---|---|---|---|
| Col1 | `Start Time` | SOT 時間戳（`分:秒.毫秒`） | `TestTimeInfoRecord[0][i].iStartMin/Sec/MSec` |
| Col2 | `End   Time` | EOT 時間戳 | `TestTimeInfoRecord[0][i].iEndMin/Sec/MSec` |
| Col3 | `Test Time` | SOT → EOT，純測試時間 | `End − Start`（同一筆） |
| **Col4** | **`Index Cycle Time`** | **本次 SOT − 上次 EOT** | `TestTimeInfoRecord[0][i].Start − TestTimeInfoRecord[0][i-1].End` |
| Col5 | `Index Time` | 只量 index 換手動作 | `tIndexTimer`，由 `ShowIndexTime()` 計算 |
| **右側 grid** | **（無標題）** | **InArm 放料到 Shuttle 的循環時間** | `fRecordInArmTime[]`，由 `RecordInArmTime()` 計算 → 見 §2-1 |

- Row 2~11 = `Last 9` ~ `Now`，Row 14 = `Average`。
- **Col4 的 Average 是畫面上最近 9 筆的平均**（`IndexTime/IndexTimeCT` 是 `RecordTimeInfo()` 內的區域變數，每次呼叫歸零），不是開機以來的累計平均。注意它與全域 `double IndexTime`（`cmydef.cpp`，被 `ShowIndexTime()` 使用）同名但被區域變數遮蔽，兩者互不影響。
- Col5 的 Average = `fObserver->fRecordIndexTime[11]`（最近 10 筆平均）。

### 對外輸出全部取 Col4「Now」

| 用途 | 程式位置 |
|---|---|
| SECS SVID | `RunInfo.IndexCycleTime = TimeInfoGrid->Cells[4][11]` |
| Remote Command `INDEXCYCLETIME?`（Novatek，Sam 20220408） | `TfMain::IndexCycleTimeStrings()` → `MSG_CMD_IndexCycleTime` |
| `[D70]` 記錄檔 | `TfObserver::RecordIndexCycle()` |
| 送 ASE（kevin 20170210） | `dSend_ASEData[1]`（Col4）、`dSend_ASEData[2]`（Col3） |
| OEE | `dIndexCycleTime[i] = TimeInfoGrid->Cells[4][i+3]` |
| SPIL Index Cycle Time Monitoring | `dTotoalIndexCycleTime = iT` |

---

## 2-1. 右側無標題欄 = InArm 放料到 Shuttle 的循環時間

> 客戶最常問的一欄：「Test Time 頁右邊那排數字是什麼？」
> 客戶常自行理解成「InArm pick → 下一次 pick」——**週期長度等價，但錨點在「放料完成」不是「取料」**。

| 項目 | 內容 |
|---|---|
| 元件 | `TfObserver::TimeInfoGrid_InArm`（`cObserver.h` / `cObserver.dfm`，Left=744、Width=177、ColCount=2、RowCount=15） |
| 計算函式 | `TfObserver::RecordInArmTime()`（`cObserver.cpp:2968`，Steven 20140930，原註解 `For XY-Pitch`） |
| 計時器 | `tRecordInArmTimer` — 先 `LatchCycleTime()` 取值再 `LatchCycleTime(true)` 重新起算 → **記的是兩次相鄰觸發之間的間隔（秒）** |
| 陣列 | `fRecordInArmTime[0..10]` 顯示、`[11]` 存平均但**未顯示** |
| 顯示 | `TimeInfoGrid_InArm->Cells[1][i+1] = fRecordInArmTime[i]`（i=0~10 → Row 1~11），`double` 直接轉 `AnsiString` → 小數位數不固定（`1.64` / `1.563` / `0.484`） |

### 觸發點（＝定義區間的錨點）

每個機型的 InArm 檔案（`ainarm9045*.cpp`，共 29 檔、約 92 處）都在同樣三個位置呼叫：

1. **InArm 放料到 Shuttle1 完成**（`DoPlaceToShuttle1` 尾端，例 `ainarm9045_2x4_8.cpp:1607`）
2. **InArm 放料到 Shuttle2 完成**（例 `ainarm9045_2x4_8.cpp:1980`）
3. **保護分支**：`InArmSuck.HasRealIC()==false` 時 Z 退到 Plate Safe 直接離開（例 `ainarm9045_2x4_8.cpp:1198`）

→ 所以一筆 = 「上一次放完 Shuttle → 這一次放完 Shuttle」＝**入料側一個完整取放循環**。
雙 Shuttle 機台兩邊都會打點，因此每筆約等於一個 index cycle；可用來判斷 InArm 是否為瓶頸（此欄明顯 > Col4 → Index 在等 InArm 供料）。

### 三個判讀陷阱（都會讓客戶誤讀）

| # | 陷阱 | 說明 |
|---|---|---|
| 1 | **沒有欄位標題** | 程式從未寫 `Cells[?][0]`，DFM 也沒設 → 客戶只看到一排裸數字 |
| 2 | **排序與左表相反** | 右側 Row 1（對齊左表 `Start Time` 標題列那一行）= `fRecordInArmTime[0]` = **最新**，往下越舊；左表 Col5 是 `Cells[5][2+i]=fRecordIndexTime[9-i]`，`Last 9`(最舊) → `Now`(最新) 由上往下。**同一頁兩個方向** |
| 3 | **沒有 Average、不落地** | Row 14 `Average` 永遠空白（顯示迴圈只到 index 10）；此欄不寫任何 log、不進 SECS SVID、不進 `[D70]` CSV，純畫面參考 |

> 若要改善（純 UI 層、不動流程）：補標題（如 `InArm Cycle`）、把順序改成與左表一致、把已算好的 `fRecordInArmTime[11]` 顯示到 Row 14。

---

## 3. 兩個時間戳的打點位置

| 打點 | 函式 | 觸發時機 | 呼叫點 |
|---|---|---|---|
| **SOT**（Start） | `RecordStartTestTime()` | Handler 準備送 Start Test 給 tester **之前** | GPIB `atester.cpp` case 55；TTL `atester.cpp` case 250 |
| **EOT**（End） | `RecordEndTestTime(int iArm)` | 收到 tester 結果、Double Contact 判定結束後 | `aTester_Front.cpp`(iArm=0)、`aTester_Rear.cpp`(iArm=1)、`atester_32Site.cpp`(iArm=2) |

- 兩者都寫入 `TestSocketTimeInfo[0..1]`（迴圈 `i<2` 兩支 arm 寫同一組值），再由 `RecordEndTestTime()` → `RecordTimeInfo()` 推入 `TestTimeInfoRecord[][9]` 並整批重算畫面。
- 因為雙臂輪替，Col4 的「上次 EOT」實際上是**另一支 arm 的測試結束**。
- `RecordEndTestTime()` 只在 `TestSocket.HasRealIC()` 為真時呼叫（Steven 20210218 修正）。

### 行號對照

| 項目 | 9046LS 745.9 | 主線 910.0 |
|---|---|---|
| `RecordStartTestTime()` 定義 | `cObserver.cpp:2182` | `cObserver.cpp:2136` |
| `RecordEndTestTime()` 定義 | `cObserver.cpp:2197` | `cObserver.cpp:2150` |
| SOT 呼叫（GPIB / TTL） | `atester.cpp:1436 / 1919` | `atester.cpp:1499 / 1983` |
| EOT 呼叫（Front / Rear / 32Site） | `aTester_Front.cpp:2603` / `aTester_Rear.cpp:2574` / `atester_32Site.cpp:2710` | `aTester_Front.cpp:2930` / `aTester_Rear.cpp:2897` / `atester_32Site.cpp:2783` |
| Col3/Col4 計算 | `cObserver.cpp:1962~2003` | `cObserver.cpp:~1930~1970` |
| 三欄標題 | `cObserver.cpp:208~210` | `cObserver.cpp:201~203` |
| `ShowIndexTime()` 定義 | `csystem.cpp:15225` | `csystem.cpp:19865` |
| `RecordIndexCycle()`（D70） | `cObserver.cpp:4979` | `cObserver.cpp:4846` |

---

## 4. Index Cycle Time 實際包含哪些動作

EOT → SOT 之間全部計入：

1. **測試後處理**：讀 bin 結果、`ProcessCount()` 分 bin 計數、ATC `SendTestEnd`/`SiteTesting`、溫度 log（`TemperatureStorageLog`）
2. **殘料處理**：Destroy 吹氣、真空關閉、IC 掉料檢查（`CheckIndexAllSuckICFallDown`）、socket sensor 檢查
3. **index 換手動作**：Z 上升 → Y 移動 → 另一支 Z 下降（`Z1UpZ2Down` / `Z2UpZ1Down`）＝ Col5 那一段
4. **壓力/力量相關**：Torque 讀取（`bEnableReadAndCheckTorque`）、EP 充氣、Soft Contact 分段下壓（`DirectContactSoftEP`/`DropContactSoftEP` + `ADAM_WriteVoltage`）
5. **視覺檢查**：RTC / CCD Full View、Half View、`rtCHECKNULL`、ROI 相關等待
6. **跨模組等待（最常被忽略）**：等 Shuttle 到位、等 InArm 放完料、`fCanMoveM` 互鎖釋放
7. **開測前置**：drop wait time、真空建立、Start Delay
8. **異常時間也算**：retry、JAM/WAR 停機、Pause、operator 處理時間 —— 只要期間沒有重新觸發 SOT，全部落在這段內

> 診斷準則：Index Cycle Time 暴增通常是「等料 / 交握 / 停機」，不是馬達變慢。要區分，就比對 Col5 `Index Time`：Col5 正常而 Col4 暴增 → 問題在等待或異常處理，不在 index 機構。

---

## 5. Drop Contact 模式的三段拆解（排查首選）

`DeviceForm.ContactMode == DropContact` 時，這段被切成三段送到 Observer「Motion Part」頁（`sgTimeData` row 18~21，JerryYang 20170503）：

| Row | 名稱 | 區間 | 計時器 |
|---|---|---|---|
| 18 | `Drop Contact 1` | 測試完成 → 另一支 arm 降到 drop 高度 | `DropContactTimer1`（EOT 處 latch，`bIndexFinish` 時讀） |
| 19 | `Drop Contact 2` | drop wait time + 剩 2mm 移到 contact 高度 | `DropContactTimer2` |
| 20 | `Drop Contact 3` | 到 contact 高度後吸真空 + start delay | `DropContactTimer3`（SOT 處讀） |
| 21 | `Total` | 三段相加 ≈ 一個 index cycle time | `AddTimeData(20)` 時計算 |

計時器宣告於 `atester.cpp` / `atester.h`（`TQPF_Timer DropContactTimer1~3`）。

---

## 6. Col5 `Index Time` 的計算（`ShowIndexTime`）

`csystem.cpp` → `void ShowIndexTime(int Item)`，搭配全域 `GetIndexTime_flag`：

| 呼叫 | 行為 |
|---|---|
| `ShowIndexTime(-1)` | 若 `GetIndexTime_flag==false` → `tIndexTimer.LatchCycleTime(true)` 起算並設 flag（避免重複 latch）。位置在下壓前、EP/soft contact 準備之後 |
| `ShowIndexTime(-2)` | 只把 flag 清為 false → **下一次 -1 會重新起算**。散佈在 socket float、RTC error、IndexStatus 不符等分支（Steven 20200715「重新計算 Cycle Time」） |
| `ShowIndexTime()`（Item=0） | `IndexTime = tIndexTimer.LatchCycleTime()`，顯示到 StatusBar Panel 0、`RunInfo.IndexTime`、`fObserver->RecordIndexTime()`、`dSend_ASEData[0]`，最後清 flag |

重點差異：**Col5 因為 -2 會重新起算，所以排除 retry / 錯誤處理；Col4 不排除。**

其他行為：
- `bIndexTimeSet` 為真時顯示**假值**（`0.9 + random(25)/100`，kevin 20130321/20130812，K15 假 index time）— 客戶抱怨數字不合理時先確認這個旗標。
- SPIL 特例：`iBodySP>=100 && INDEX_PRESS_TYPE==e85KG && bSPILFunction` 時，`400 < IndexTime < 500` 一律壓成 400ms（jou 2010-11-15，SPIL 要求 index time 必須在 0.4 sec 內）。
- `ShowIndexTime()` 內順帶呼叫 OEE `CalculateOEEPlanOut()`（`bN14_1_EnableOEEFunction`）。
- ⛔ 20261003 V906 移植樹補（AI(W906-E034) 20261003，todo E-034＝筆電卡 S-21，St01；Steven 1003 14:5x「Q82. A」＝#20 例外，Steven 1003 常設規則）：`TfObserver::RecordIndexTime` 的 OEE 分支（`CosFunction.bOEEFunction`、Index Z 筆數 < 11）golden 0618 `cObserver.cpp:2897` 是沒作用的 `sTestIndexZTime=="";`（舊的 Index-Z 時間字串留著）；0625 `:2897`、V912 `:3057` 是 `=""`。移植樹 `D:\HT9045\HT9011UC_Cpp_V3.33.906.0\cObserver.cpp:2540` 已改 `= ""`。測試 ctest `ObserverCore` `Test_RecordIndexTime_OEEResetsIndexZTime`。
- 是否顯示由 `IniConfig.bShowIndexTime` 控制（`fCounterSel` 的 `cbIndexTime`）。

---

## 7. `[D70] Index Cycle Time Record` 記錄檔

- Config：`IniConfig.bD70IndexCycleTimeRecord`，群組 `"Index"`，UI `cbD70`（Sam 20200916）。
  - 由 `CosFunction.bIndexCycleTimeRecord` 決定是否顯示；`cprod.cpp` 內依機型/客戶設 true/false。
- 實作：`TfObserver::RecordIndexCycle(bool bReset)`，由 `RecordTimeInfo()` 尾端每個 cycle 呼叫一次。
- **每累積 10 筆才寫檔**（`iRecordIndexCycleTimeCnt>=10`），關閉功能或 `bReset` 時計數歸零 → **停機當下最後不足 10 筆的資料會遺失**。
- 輸出：

```text
D:\HT9045_log\IndexCycleTimeRecord\
  <SetupFileName>_Temperature[nn]_AutoTray1Dir[n]_yyyymmdd_hhnnss.csv

Record Time,No,Index Cycle Time
2026-07-29 10:11:12,1,1.234        ← 即 Col4 的 "Now" 值
```

- 另有 SPIL 專用的 monitoring 路徑（與 D70 無關）：`RecordEndTestTime()` 內
  `bSPILFunction && bEnableIndexCycleTimeMonitoring` → `RecordMonitoringIndexCycleTime_New()`，
  否則 `bEnableIndexCycleTimeMonitoring && bResetflag` → `RecordMonitoringIndexCycleTime()`。

---

## 8. 判讀陷阱（客戶反饋數值異常時先查這裡）

| # | 陷阱 | 說明 |
|---|---|---|
| 1 | **空 socket / dummy cycle 不打點** | `RecordEndTestTime()` 只在 `HasRealIC()` 為真時呼叫。空跑的 cycle 不更新 EOT，下一筆 Index Cycle Time 會把整段併進來，數值特別大。 |
| 2 | **Double Contact 會膨脹** | 中間幾次 contact 不記 EOT，但每次 contact 都重記 SOT。最終 Col4 = 最後一次 contact 的 SOT − 上個 cycle 的 EOT，**前幾次 contact 的測試時間被算進 index cycle time**。 |
| 3 | **時間戳只有 分:秒:毫秒** | `TestSocketTimeInfo` 只存 `SystemMin/Sec/MSec`，跨分鐘僅補一次 60 分（`if(Start.iMin < PrevEnd.iMin) iE=(60+iMin)*60`）。停機超過 1 小時後的第一筆數值不可信。 |
| 4 | **關站 / 單臂模式** | `RecordEndTestTime(iArm)` 依 `TestIF_File.iShuttleMode` / `iShuttle_Sel` 過濾，只有被選中的那支 arm 才呼叫 `RecordTimeInfo()`（Sam 20201231 修正「關 Arm 後 Index Cycle Time 異常」）。32-site 走 `iArm=2` 不過濾。 |
| 5 | **假 index time** | `bIndexTimeSet` 為真時 Col5 與 Col4 的 `Now` 都會被替換成 `fIndexTime` 假值。 |
| 6 | **Average 只看最近 9 筆** | Col4 Row14 每 cycle 重算，不是長期平均；要長期趨勢請用 D70 CSV。 |
| 7 | **Col4 vs Col5 差距 = 非機構時間** | 兩者差額就是等待、交握、視覺、retry、停機。先算差額再決定往機構或往流程查。 |

---

## 9. 快速定位用 grep

```text
RecordStartTestTime          # SOT 打點
RecordEndTestTime            # EOT 打點 + RecordTimeInfo 觸發
RecordTimeInfo               # Col3/Col4 計算與畫面更新
TimeInfoGrid->Cells[4]       # Index Cycle Time 欄
ShowIndexTime                # Col5 Index Time（-1 起算 / -2 重算 / 0 顯示）
DropContactTimer1            # Drop Contact 三段拆解
RecordIndexCycle             # [D70] CSV 落地
bD70IndexCycleTimeRecord     # Config 開關
IndexCycleTimeStrings        # INDEXCYCLETIME? remote command
bEnableIndexCycleTimeMonitoring  # SPIL monitoring（另一條路徑）
RecordInArmTime              # 右側無標題欄（InArm 放料循環時間）
TimeInfoGrid_InArm           # 右側 grid 元件
fRecordInArmTime             # 右側欄陣列（[0]=最新、[11]=平均未顯示）
tRecordInArmTimer            # 右側欄計時器
```

---

## 10. 相關 Reference

- **UPH 記錄機制**（客戶常一起問「後台有沒有記 UPH 的檔案」）：見 [uph-record.md](uph-record.md)。
  注意 UPH 與本檔的 cycle time **口徑完全不同**：UPH 一筆 = 一盤入料盤，且已扣掉 Pause Time；
  cycle time 一筆 = 一顆 / 一次 index，且**不扣**停機時間。兩者不可互推。
