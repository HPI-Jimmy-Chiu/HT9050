---
name: ht9045-index-flow
description: HT9045 IC Test Handler Index Arm（下壓模組）流程知識庫。當使用者詢問 Index Arm、Test Head、DoTestHeadMotor、DoTestY、DoTestYFront、DoTestYRear、Index 測試流程、Socket 下壓、RTC FullView、AutoSiteMap、PlaceToShuttleFirst、IndexEveryTimeCheckEP、32-site 兩臂測試、Front/Rear 測試切換、Index Torque、Socket Sensor、EP 充氣壓力、Index Cycle Time / Test Time / Index Time 三欄定義與記錄機制、Index Cycle Time 包含哪些時間、cycle time 變長排查、[D70] Index Cycle Time Record 記錄檔、INDEXCYCLETIME? 查詢、Test Information → Test Time 頁欄位說明、右側無標題欄（InArm 放料到 Shuttle 的循環時間 / InArm pick 到下一次 pick）、UPH 後台記錄檔案與統計口徑、[P11] Record UPH 的 CSV 格式等相關問題時，應先載入此技能以理解 Index 完整處理流程。關鍵字：DoTestHeadMotor, DoTestY, DoTestYFront, DoTestYRear, DoTestY_TwoArm32Site, IndexEveryTimeCheckEP, DoFrontTestDestroyIC, DoRearTestDestroyIC, FTestSuck, BTestSuck, MTestY, MTestZ, Index, TestHead, Index Cycle Time, IndexCycleTime, Test Time, Index Time, RecordStartTestTime, RecordEndTestTime, RecordTimeInfo, ShowIndexTime, tIndexTimer, GetIndexTime_flag, TimeInfoGrid, Time Info, tsTestTime, sgTimeData, Motion Part, DropContactTimer, Drop Contact 三段, bD70IndexCycleTimeRecord, RecordIndexCycle, IndexCycleTimeRecord, INDEXCYCLETIME?, MSG_CMD_IndexCycleTime, RunInfo.IndexCycleTime, bEnableIndexCycleTimeMonitoring, bIndexTimeSet, SOT, EOT, cycle time 變長, TimeInfoGrid_InArm, RecordInArmTime, fRecordInArmTime, tRecordInArmTimer, InArm 循環時間, InArm pick to pick, 無標題欄位, UPH, UPH 記錄, UPH log, UPH csv, CalculateUPH, CaculateUPH, MyDBIUPH, RecordUPH, LotRecordUPH, RecordLotUPH_For_FOREHOPE_NINGBO, bP11RecordUPH, P11 Record UPH, as9045UPH, HT9045_Log\UPH, EventLogTxt, bRecordUPH, iUPH_LoaderCount, tUPH_PauseTime, RunInfo.iUPH, RunInfo.iAvgUPH, iNetUPH, iGrossUPH, Avg UPH, Net UPH, Gross UPH, UPH_StringGrid, Tab_UPH, bShowUPH, bG10ShowImmediateUPH, CountMTBF, MTBA, MUBA, UPHRecordEnd。另含 **Auto Clean 後 RTC 不回應 @ARM2+ 導致 WAR0335「RTC arm 1 error!」** 完整案例：WAR0335, RTC arm 1 error, RTC Arm1 Error, SnRealTimeCCDIndexArm, @ARM2+, rtArmIndex2, iRealCCDSendArmCT, OpenRTCComPortAgain, WAR0335 auto retry com port, bFullTestBeforeAutoClean, Full View before Auto Clean, DoFullViewCheck, DoReleaseAndInspEnd, DoReleaseAndInspEndByFlag, InitRealTimeCCDPara, bSendRealCCDSendStart, rtInspStart, @START+, @RELEASE+, @END+, bIndexCheckCanTurnOff, iD71IndexCheckOnOffMode, bAfterAutoCleanNoIndexCheck, iD69IndexCheckModeForAutoClean, 升版後才發生, 降版就正常, SCK RTC, RTC 1.0。另含**下壓後 hangup / 跨臂 Z 安全位互鎖卡死**完整案例：DoInterFaceErrorStep, TESTZ1UP, TESTZ2UP, iDoInterFaceErrorStepTask, Prod.TestZ1_Safe, Prod.TestZ2_Safe, 跨臂互鎖, 兩臂互相等待, Front Rear 互鎖, 下壓後hangup, 下壓後卡死, Index下壓hangup, PAUSE後卡死, PAUSE中斷座標移動, bATCHasAlarmBinNeedToError, RearTestSuckTestICTask, FrontTestSuckICTask, iBTestSuckTestICTask, iFTestSuckTestICTask, GetTesterResult, bEcho, MTestZ1, MTestZ2, Galil VS0 急停, 綠燈亮不動作。
---

<!-- AI(W906-BA-SKILL) 20260915：正文換成網頁同事 20260915 那版（較新／較完整）。
     frontmatter 的路由描述保留我們的。
     我們原有但他沒有的段落，另存 references/ours-kept-20260915.md。 -->


# HT9045 Index (Test Head Motor) Flow Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：
- Index Arm / Test Head 的動作流程、狀態機、case 數值意義
- DoTestHeadMotor 總控流程
- DoTestY / DoTestYFront / DoTestYRear / DoTestY_TwoArm32Site 測試循環
- Socket 下壓測試（Z 軸 torque / vacuum check）
- Front / Rear 測試頭切換邏輯
- AutoSiteMap 測試驗證
- PlaceToShuttleFirst 清空測試頭 IC 流程
- RTC FullView / ROI Learning
- **WAR0335「RTC arm 1 error!」**：Auto Clean 之後 RTC 不回應 `@ARM2+`、升版後才發生／降版就正常（見 [rtc-autoclean-war0335.md](references/rtc-autoclean-war0335.md)）
- IndexEveryTimeCheckEP 充氣壓力檢查
- 32-site 兩臂測試模式
- Index 掉料 / Destroy / Suck 處理
- CCD TCPIP 檢測、F16 shuttle sensor broken check
- Auto Clean 下壓清潔流程（DoIndexAutoClean）、NN 模式兩臂同動 Auto Clean（ACSim）
- Index Cycle Time / Test Time / Index Time 的定義、包含範圍與判讀陷阱（客戶問 cycle time 記了哪些時間、為何變長）
- Test Information → Test Time 頁的欄位說明，含**右側無標題那一欄**（＝ InArm 放料到 Shuttle 的循環時間，客戶常稱「InArm pick → 下一次 pick」）
- **UPH**：後台有哪些檔案記錄 UPH、統計區間口徑、`[P11] Record UPH` 開啟後的 CSV 格式與客戶碼白名單

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
- **呼叫方式**: 函式回傳 `bool`，`true` = 動作完成，`false` = 尚未完成（主迴圈反覆呼叫）
## 參考文件

- [DoTestHeadMotor_ProcessFlow.md](references/DoTestHeadMotor_ProcessFlow.md) — Index Arm 完整狀態機 Case 說明
- [indium-contact-count-reference.md](references/indium-contact-count-reference.md) — 銦片計數（Indium LifeTime）完整 Reference（旗標、存取路徑、UI 權限、告警、客戶差異）
- [index-cycle-time-record.md](references/index-cycle-time-record.md) — Index Cycle Time / Test Time / Index Time 記錄機制完整 Reference（SOT/EOT 打點、包含範圍、**§2-1 右側無標題欄＝InArm 放料循環時間**、Drop Contact 三段、[D70] 記錄檔、7 個判讀陷阱）
- [rtc-autoclean-war0335.md](references/rtc-autoclean-war0335.md) — **WAR0335「RTC arm 1 error!」完整案例**（症狀簽章、`@ARM2+`／`SnRealTimeCCDIndexArm` 握手、根因 r780 全域打開 Full View、發作三條件、`OpenRTCComPortAgain` 的 2 靜默+1 報警節奏、908.x 必須用 `DoReleaseAndInspEndByFlag()` 的版本差異、驗證判準）
- [Z1UpZ2Down_MotionSequence.md](references/Z1UpZ2Down_MotionSequence.md) — Z1UpZ2Down1 / Z1DownZ2Up1 向量運動序列、3 段式 LI 安全結構、運動中保護檢查
- [AutoClean_TwoArmSimultaneous.md](references/AutoClean_TwoArmSimultaneous.md) — NN 模式 Auto Clean 兩臂同動（ACSim）：逐臂取料（Arm1 提早開吸）+ 同時下壓 + 平行放回狀態機（case 4000~4750）、進入條件 11 項、降級路徑 A/C/D、真空壓降限制、交叉 guard 與 shuttle window、v1~v5 迭代教訓
- [NN_2DID_PreScan_DuringIndexDown.md](../ht9045-shuttle-flow/references/NN_2DID_PreScan_DuringIndexDown.md) — NN 模式 2DID 下壓期預掃（2026-07-28，跨 skill 共用）：Index 端 `IsIndexZ1Z2DownStableForPreScan()` 8 重穩態判斷、case240 穩定延遲 timer、case110 busy guard（MotorMove 假成功地雷）
- [uph-record.md](references/uph-record.md) — UPH 記錄機制完整 Reference（by-tray 統計口徑、Counter→UPH 頁欄位、EventLog 預設落地、**[P11] 三種 CSV 格式與客戶碼白名單**、SVID 1021/1028/1038/1039、10 個判讀陷阱）
- [case-crossarm-z-safe-interlock-deadlock.md](references/case-crossarm-z-safe-interlock-deadlock.md) — **「下壓後 hangup」跨臂 Z 安全位互鎖卡死案例**（`DoInterFaceErrorStep()` 用嚴格 `==` 比對對向臂是否精確在 `Prod.TestZ{1,2}_Safe`、不成立時只等待不主動補位；PAUSE 的 Galil `VS0` 急停會讓閒置臂留下無法自行消除的殘差；`RearTestSuckTestICTask`(iBTestSuckTestICTask)/`FrontTestSuckICTask` 卡 2500 + `TestTask` 卡 60；對應 `ht9045-staterecord-analysis` 的 Pattern #27）
- [autoheight-contact-test-ht9045.md](references/autoheight-contact-test-ht9045.md) — **自動測高／Contact Test 動作流程（golden 906 0618，照 Steven 的 7 步：Arm 讓開 → In Shuttle 浮料檢查／2D／往右 → Index 吸料 → Index1、Index2 量高度 → 放回 → In Shuttle 回左；只用函式＋Task 參照、不寫行號）**：Contact 頁按鈕 → `DoTestContactFunction` 各 case、`DoZ1PickFromShuttle`、`Do_Z1/Z2_AutoGetHeight`（扭力上限、COM2 0x52 讀值 k/20＝%、停止條件 `TorqueData>=kg`／`fIndexDownPos`、case 555 沒有逾時）、`Do_LoadCellAutoHigh`、`DoZPlaceToShuttle`、存檔位置、V912 差異（20261005）
- [autoheight-contact-test-ht9050-current.md](references/autoheight-contact-test-ht9050-current.md) — **HT9050 自動測高今天走到哪、卡在哪**：port main `MainProc` 呼叫 `DoTestContactFunction` 仍 `#if 0`、7 步對照（Index 只有 Z1、第 5 步不存在、放回前 In Shuttle 沒人移回右邊）、branch `v906/st01-e042` B1-B3（B3b 未 push）、P4/P5/P7/P8/P9/W-44、逐步狀態（等 E-10、EP 硬體、工單設定、Jimmy E-050/E-051）（20261005）
- [ht9050-index-fp-flow.md](references/ht9050-index-fp-flow.md) — **HT9050 的 Index 流程＝910 `DoTestHeadMotorFP`**（只有 Z1、沒有 Index Y，飛梭開到 Index 下面讓 Z1 吸料／下壓／放料）：呼叫階層與 case、FinePitch 專用 vs 需要的拆分表、[F14] FP 與一般流程混用會卡住、F1（MTestZ1 只有 `Gali_*` 到得了 1203）、W-44 新定義（安全 X 座標，Steven 1005 23:1x）與 St01／Frank 分工、ST01-C 進度（20261005）；⛔ §9（20261006）：W-44 改成禁止飛梭進 socket 區、一切行為以 Index 需求為優先（Q133）、Index 軸卡別（Q130）、[I01] 定義（Q132）

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `atester.cpp` | `DoTestHeadMotor()` — Index 測試總控 state machine | `iTestHeadMotorTask` |
| `atester.cpp` | `DoTestY()` — 測試循環分派（Front/Rear/32Site） | `iTestYTask` |
| `atester.cpp` | `IndexEveryTimeCheckEP()` — 每次 Index 前 EP 充氣壓力確認 | `iIndexEveryTimeCheckEPTask` |
| `aTester_Front.cpp` | `DoTestYFront()` — Front 測試頭完整測試流程 | `iTestYFrontTask` |
| `aTester_Front.cpp` | `DoFrontTestDestroyIC(false)` — Front 殘 IC 清除 | `iFrontTestDestroyICTask` |
| `aTester_Front.cpp` | `DoFTestSuckTestIC()` — Front AutoSiteMap 測試驗證 | `iFTestSuckTestICTask` |
| `aTester_Rear.cpp` | `DoTestYRear()` — Rear 測試頭完整測試流程 | `iTestYRearTask` |
| `aTester_Rear.cpp` | `DoRearTestDestroyIC(false)` — Rear 殘 IC 清除 | `iRearTestDestroyICTask` |
| `aTester_Rear.cpp` | `DoBTestSuckTestIC()` — Rear AutoSiteMap 測試驗證 | `iBTestSuckTestICTask` |
| `atester_32Site.cpp` | `DoTestY_TwoArm32Site()` — 32-site 兩臂測試流程 | `iTestTwoArm32SiteTask` |
| `atester_32Site.cpp` | `DoTestSuckTestIC_TwoArm32Site()` — 32-site AutoSiteMap 測試驗證 | `iTestSuckTestIC_TwoArm32Site_Task` |
| `cObserver.cpp` | `RecordStartTestTime()` / `RecordEndTestTime(iArm)` / `RecordTimeInfo()` / `RecordIndexCycle()` — Test Time 與 Index Cycle Time 記錄 | （非 state machine） |
| `csystem.cpp` | `ShowIndexTime(int Item)` — Index Time（換手動作時間）計算與顯示 | （非 state machine） |
| `cObserver.cpp` | `RecordInArmTime()` — Test Time 頁右側無標題欄（InArm 放料到 Shuttle 的循環時間） | （非 state machine） |
| `ainarm9045.cpp` | `CalculateUPH(bool)` / `RecordUPH()` / `LotRecordUPH()` / `RecordLotUPH_For_FOREHOPE_NINGBO()` — UPH 計算與 CSV 落地 | （非 state machine） |
| `cMyDB.cpp` | `MyDBIUPH(int)` — UPH 寫入 EventLog（預設就記） | （非 state machine） |
| `AutoClean\AutoClean.cpp` | `DoIndexAutoClean()` — Auto Clean 下壓清潔（原生序列 case 100~3900 + ACSim 兩臂同動 case 4000~4750） | `iDoIndexAutoCleanTask` |
| `AutoClean\AutoClean.cpp` | `RunAutoCleanTwoArmSimultaneous()` — ACSim 進入條件判斷（11 項） | — |

---

## 1. 呼叫階層總覽

```text
DoTestHeadMotor()                                [atester.cpp]
  |- (Init / Index Check / RTC FullView / Torque / EP check)
  |- DoTestY()                                   [atester.cpp]
  |   |- DoTestYFront()                          [aTester_Front.cpp]
  |   |- DoTestYRear()                           [aTester_Rear.cpp]
  |   `- DoTestY_TwoArm32Site()                  [atester_32Site.cpp]
  |
  |- DoFrontTestDestroyIC(false)                 [aTester_Front.cpp]
  |- DoRearTestDestroyIC(false)                  [aTester_Rear.cpp]
  |- DoFTestSuckTestIC() / DoBTestSuckTestIC()  [aTester_Front.cpp / aTester_Rear.cpp]
  |- DoTestSuckTestIC_TwoArm32Site()             [atester_32Site.cpp]
  `- IndexEveryTimeCheckEP()                     [atester.cpp]
```

---

## 2. DoTestHeadMotor() Main Flow (atester.cpp)

> Task variable: `int &Task = iTestHeadMotorTask`

### Pre-switch guards

1. 客戶碼/Offset 修正：`CC_SIGURD_HUKOU` 使用 contact 高度補償。
2. CCD TCPIP 掃描：`ScanCCDProgram()` 後可啟動 `CCDRunExec()`。
3. Pause 保護：`iPauseBackUp != -1` 且前後吸嘴/吹氣完成時直接 return。
4. Heater connect check：多種 heater 模式下 `CheckIndexConnect()`。
5. Auto clean 重置：`bIndexCheckState` 會強制 `Task=1`。
6. Auto Alignment 執行中 return。
7. OutShuttle Lose IC (`bSht1LoseICErr || bSht2LoseICErr`) 且 REAL 模式 return。

### Top-level phase map

```text
Task 1
  -> 200000~600000  (Index 真空/初始狀態自檢)
  -> 10000~10030    (F16 shuttle sensor broken check, optional)
  -> 11             (route select)
     |- 2~7         (CCD TCPIP 檢測流程)
     |- 9~21        (Index 安全位 + socket sensor 前置檢查)
     |- 30~90       (Front/Rear drop IC handling)
     |- 100~145     (Z1/Z2 torque + index vacuum initial check)
     |- 150~170     (AutoSiteMap test trigger)
     |- 1500~1720   (進入正常測試姿態)
     |- 600         (進入 DoTestY 循環)
     |- 20000~21500 (PlaceToShuttleFirst 先清 test head IC)
     |- 30000       (IndexEveryTimeCheckEP)
     |- 40200~40510 (RTC FullView / ROI learning)
     `- 50000~80010 (OneByOne index check routes)
```

### Key state groups

#### A. Index head init & vacuum self-check

- `1`: 初始化記錄、socket clamp sensor 檢查。
- `200000`: 前後吸嘴 reset / 開吸。
- `300000`, `500000`: `ProcessIndexSuckDestroy1/2()`。
- `600000`: `CheckIndexArmInitState()`；成功後切到 `10000`(F16) 或 `11`。

#### B. F16 shuttle broken sensor check (optional)

- `10000 -> 10010 -> 10020 -> 10030`。
- Shuttle move-left/check 回圈，若仍異常回 `10000`，否則進 `11`。

#### C. CCD startup check path

- `2 -> 3 -> 4 -> 5 -> 6 -> 7`。
- 含 CCD program existence / identification / timeout retry。

#### D. Core index pre-test check (torque + vacuum + socket)

- 安全/前置：`9 -> 10 -> 15 -> 20 -> 21`。
- Drop 流：`30~90`（`bFTestSuckDrop`/`bBTestSuckDrop`）。
- Arm1 檢查：`12000 -> 121xx -> 12200 -> 123xx -> 121/122/123/124/125`。
- Arm2 檢查：`14000 -> 141xx -> 14200 -> 143xx -> 141/142/143/144/145`。
- OneByOne/4site 分支：`50000/50100/50110`, `60000/60100/60110`, `80000/80010`。

#### E. AutoSiteMap trigger and test execution

- `150`: auto site mapping 資料配置（Open BIN/dummy data）。
- `160`: `DoFTestSuckTestIC()` or `DoBTestSuckTestIC()`。
- `170`: `DoTestSuckTestIC_TwoArm32Site()`。

#### F. Move into run posture and enter test cycle

- Single/normal route: `1500 -> 1550 -> 1600 -> (1700/1710/1720 hot delay optional) -> 600`
- 2-arm 32 site route: `1500 -> 1650 -> 1660 -> 1670 -> 1675(optional) -> 1680 -> 600`

#### G. PlaceToShuttleFirst cleanup route

- `20000 -> 20100 -> 20200`
- 若 head 有 IC：
  - Front path `20300 -> 20400 -> 20500(DoFrontTestDestroyIC)`
  - Rear path `21000 -> 21400 -> 21500(DoRearTestDestroyIC)`
- 全清完後回 `1`。

#### H. RTC FullView / ROI learning route

- `40200 -> 40300 -> 40310 -> 40320 -> 40330 -> 40400 -> 40500 -> 40510`
- 主要事件：SiteMap send、FullT OK/NG、retry/skip、ROI learning (`fContact->Do_ROILearning`)。

---

## 3. Next Layer: DoTestY() (atester.cpp)

> Task variable: `int &Task = iTestYTask`

### State machine overview

```text
1 -> (20 -> 25 -> 30 -> 40/45)  [OneCycle/CleanOut/Index normalize]
  -> 50 -> 60 -> 65/70 -> 100/200 (front/rear alternate)
  -> 260 -> 300 -> 310            (TwoArm32Site route)
```

### Case summary

- `1`: 判斷 OneCycle/CleanOut/disable one cycle 條件；決定走一般測試或收尾流程。
- `20`: 是否做 `DoCheckSocketHasIC()`。
- `25`: 兩 Z 軸回安全位。
- `30/40/45`: Y 軸到 wait/back，`IndexStatus` 重置。
- `50`: 判斷 front/rear 測試優先順序。
- `60/65/70`: soak time / heater 條件 / front-rear 切換。
- `100 -> 110`: `InitTestYFrontTask()` + `DoTestYFront()`。
- `200 -> 210`: `InitTestYRearTask()` + `DoTestYRear()`。
- `260 -> 300 -> 310`: 32-site 兩臂路徑，呼叫 `DoTestY_TwoArm32Site()`。

---

## 4. Child State Machines

### 4.1 DoTestYFront() (`aTester_Front.cpp`)

> Task variable: `int &Task = iTestYFrontTask`

- 函式規模大，主 case 群：
- 前置/掉料/安全段：`1, 50, 52, 55, 56, 57, 60, 61, 62, 64, 65, 66, 67, 68, 70, 72, 73, 75, 7500, 76, 78, 79, 80`
- 中段主測試路徑：`81, 82, 84, 90, 95, 97, 100, 105, 106, 108, 109, 110, 115, 116, 130, 140, 200, 209, 2091, 2092, 210, 211, 212, 213, 215, 220, 230, 240`
- 後段處理：`300, 310, 311, 312, 313, 314, 315, 316, 320, 325, 326, 330, 340, 350`
- 擴充/特例：`1000, 1100, 1200, 1300, 2000, 12000, 12010, 14000, 16000, 16100, 17900, 18000, 18100, 18500`

### 4.2 DoTestYRear() (`aTester_Rear.cpp`)

> Task variable: `int &Task = iTestYRearTask`

- 與 Front 類似但包含 Rear 專屬高編號流程：
- 前中段：`1,2,3,50,52,55,56,57,60,61,62,64,65,66,67,68,70,72,73,75,7500,76,78,79,80,81,82,84,90,95,97,100,105,106,108,109,110,115,130,140,200,209,2091,2092,210,211,212,213,215,220,230,240,300,310,311,312,313,314,315,316,320,325,326,330,340,350`
- 延伸段（Rear 特有大分支）：`1000,1100,1200,1300,2000,10500,11000,11002,11010,11015,11016,11017,11020,11030,11035,11040,11041,11050,11060,11080,11090,11095,11096,11100,11110,12000,12010,14000,16000,16100,17900,18000,18100,18500`

### 4.3 DoTestY_TwoArm32Site() (`atester_32Site.cpp`)

> Task variable: `int &Task = iTestTwoArm32SiteTask`

- 主 case 群：
- 初始與掉料：`1, 50, 52, 55, 60, 61, 62, 64, 65, 66, 67, 68`
- 主測試段：`81, 82, 84, 85, 90, 95, 97, 100, 105, 106, 108, 110, 120, 130, 200, 209, 210, 220, 221, 225, 226, 230, 240, 241, 250`
- 後段：`300, 310, 311, 312, 313, 314, 315, 316, 320, 325, 330, 340`
- 結尾：`2000, 12000, 12010`

> **NN 2DID 預掃插入點（`bNNShuttlePreScan2DID=true` 時，2026-07-28）**：
> - case240：下壓到位後武裝 `NNPreScanStableDelay_32` 穩定延遲 timer（`iNNPreScanStableDelayMS`，預設 500ms）。
> - case241（測試等待窗）：`IsIndexZ1Z2DownStableForPreScan()` 8 重條件成立時，InShuttle 得以提前進掃碼鏈（僅 4 軸機；`Task==241` 結構性 gating 使 D41 retry / Up / Reset 自動失效）。
> - case110（Up）：任一 Shuttle 掃碼鏈進行中（`AutoSHTxTask ∈ {200,201,2300,2400}`）時原地等待，且**不鎖** `fCanMoveM`（避免 `MotorMove()` 假成功造成 2DID 錯位）。
> - 詳見 [NN_2DID_PreScan_DuringIndexDown.md](../ht9045-shuttle-flow/references/NN_2DID_PreScan_DuringIndexDown.md)。

---

## 5. Other Direct Child State Machines

### 5.1 DoFrontTestDestroyIC(false) (`aTester_Front.cpp`)

> Task variable: `int &Task = iFrontTestDestroyICTask`

- Case: `1, 100, 200, 309, 310, 320, 500, 550, 600, 650, 700, 750, 800`
- 用途：Front 測試頭殘 IC 清除（PlaceToShuttleFirst 路徑）。

### 5.2 DoRearTestDestroyIC(false) (`aTester_Rear.cpp`)

> Task variable: `int &Task = iRearTestDestroyICTask`

- Case: `1, 100, 200, 309, 310, 320, 500, 550, 600, 650, 700, 750, 800`
- 用途：Rear 測試頭殘 IC 清除。

### 5.3 DoFTestSuckTestIC() (`aTester_Front.cpp`)

> Task variable: `int &Task = iFTestSuckTestICTask`

- Case: `1, 100, 200, 2200, 2400, 2410, 2500, 2600, 2700, 3000, 3050, 3100, 5000, 5050, 5100, 5300, 5350, 5400, 5450, 5500, 6000, 6100, 6200, 6300, 6500, 6600, 7000, 7100`
- 用途：AutoSiteMap/測試驗證流程（Front）。

### 5.4 DoBTestSuckTestIC() (`aTester_Rear.cpp`)

> Task variable: `int &Task = iBTestSuckTestICTask`

- Case: `1, 100, 200, 2200, 2400, 2410, 2500, 2600, 2700, 3000, 3050, 3100, 5000, 5050, 5100, 5300, 5350, 5400, 5450, 5500, 6000, 6100, 6200, 6300, 6500, 6600, 7000, 7100`
- 用途：AutoSiteMap/測試驗證流程（Rear）。

### 5.5 DoTestSuckTestIC_TwoArm32Site() (`atester_32Site.cpp`)

> Task variable: `int &Task = iTestSuckTestIC_TwoArm32Site_Task`

- Case: `1, 2200, 2400, 2500, 2600, 2700, 3000, 3100, 5000, 5100, 5200, 5300, 5400, 5500, 6000, 6100`
- 用途：32-site 兩臂 AutoSiteMap/測試驗證流程。

### 5.6 IndexEveryTimeCheckEP() (`atester.cpp`)

> Task variable: `int &Task = iIndexEveryTimeCheckEPTask`

- Case: `1 -> 100 -> 200 -> 300`
- 用途：每次 Index 前 EP 充氣壓力確認與告警重試。

---

## 6. End-to-End Runtime Flow

```text
DoTestHeadMotor
  -> Index self check / F16 / CCD / RTC pre-check
  -> Arm1 torque+vacuum check
  -> Arm2 torque+vacuum check
  -> (optional) AutoSiteMap test path
  -> posture ready
  -> DoTestY loop
       -> DoTestYFront or DoTestYRear
       -> (32-site mode) DoTestY_TwoArm32Site
  -> loop
```

---

## 7. Notes

- `DoTestHeadMotor()` 是 Index 測試總控 state machine；`Task=600` 才正式進入 `DoTestY()` 持續循環。
- 兩大例外路徑：
  - `PlaceToShuttleFirst`：先清空 test head IC（`20000~21500`）。
  - `REAL_TIME_CCD`：走 FullView / ROI learning (`40200~40510`)。
- 真空/壓力檢查分兩層：
  - 主流程中的 torque + socket sensor check（121xx / 141xx）。
  - 子流程中的 Front/Rear/TwoArm test-and-destroy state machines。
- **[I01]（`bI01TesterFinishThenHome`，Enable tester finish then homing）作用在「測試中」**（Steven 1006 08:2x，`D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md` Q132）：原話「[I01]功能本來就是生產中作用的  這邊所謂的生產中 (正確說是 測試中), 就是 GPIB的 0x41命令 一直到收到 BINON與 ECHOOK, 且把BIN資料回寫到socket為止」⇒ 從 GPIB 的 0x41 命令開始，一直到收到 BINON 與 ECHOOK、而且把 BIN 資料回寫到 socket 為止；這段期間 HOME／停機要等測試做完才歸零（MainProc 的 [I01] 路徑會繼續呼叫 `DoTestHeadMotor()` 把測試推完）。HT9050 照 910／Jimmy #14 維持一般 `DoTestHeadMotor()`，見 [references/ht9050-index-fp-flow.md](references/ht9050-index-fp-flow.md) §9.3。
- **HT9050 W-44（Steven 1006 Q133）**：禁止**飛梭**進 socket 區（入料飛梭右側、出料飛梭左側），不是禁止 Index；一切行為以 Index 的需求為優先——見 [references/ht9050-index-fp-flow.md](references/ht9050-index-fp-flow.md) §9.1。

---

## 8. Index Cycle Time / Test Time 記錄機制（摘要）

> 完整內容見 [index-cycle-time-record.md](references/index-cycle-time-record.md)

### 三欄定義（Observer「Time Info」頁，勿混用）

| 欄位 | 標題 | 內容 |
|---|---|---|
| Col3 | `Test Time` | SOT → EOT，純測試時間 |
| **Col4** | **`Index Cycle Time`** | **本次 SOT − 上次 EOT**（＝兩次測試之間的整段空檔） |
| Col5 | `Index Time` | 只量 index 換手動作（`tIndexTimer`，由 `ShowIndexTime()` 計算） |
| **右側 grid** | **（無標題）** | **`TimeInfoGrid_InArm`：InArm 放料到 Shuttle 的循環時間**（`RecordInArmTime()` / `fRecordInArmTime[]`）。客戶常稱「InArm pick → 下一次 pick」——週期等價但錨點在**放料完成**。⚠️ **Row 1 最新、往下越舊，與左表方向相反**；無 Average、不落地、不進 SECS。詳見 reference §2-1 |

SECS SVID `RunInfo.IndexCycleTime`、Remote Command `INDEXCYCLETIME?`、`[D70]` 記錄檔、OEE、送 ASE 全部取 **Col4「Now」**。

### 打點位置

- **SOT** = `RecordStartTestTime()`：Handler 送 Start Test 給 tester **之前**（`atester.cpp` GPIB case 55 / TTL case 250）。
- **EOT** = `RecordEndTestTime(iArm)`：收到 tester 結果、Double Contact 判定結束後（Front=0 / Rear=1 / 32Site=2），內部再呼叫 `RecordTimeInfo()` 重算整個畫面。
- 只有 `TestSocket.HasRealIC()` 為真才打點。雙臂輪替，故「上次 EOT」是**另一支 arm** 的測試結束。

### Index Cycle Time 包含哪些

測試後處理（讀 bin / `ProcessCount` / ATC SendTestEnd / 溫度 log）、Destroy 吹氣與掉料檢查、socket sensor 檢查、**index 換手動作**、Torque 讀取 / EP 充氣 / Soft Contact 分段、RTC-CCD Full/Half View、**等 Shuttle 到位與等 InArm 放料的交握時間**、drop wait / 真空建立 / Start Delay，以及 **retry、JAM/WAR 停機、Pause** 等異常時間。唯一不含的是測試時間本身。

> 診斷準則：Col4 暴增但 Col5 正常 → 問題在等待 / 交握 / 異常處理，不在 index 機構。
> `DropContact` 模式可直接看 Observer「Motion Part」頁 row 18~21（Drop Contact 1/2/3 + Total）做分段。

### 常見判讀陷阱

1. 空 socket / dummy cycle 不打點 → 下一筆 Col4 把整段併進來，特別大。
2. Double Contact 時 SOT 每次覆蓋、EOT 不記 → 前幾次 contact 的測試時間被算進 Col4。
3. 時間戳只有 `分:秒:毫秒`，跨分鐘僅補一次 60 分 → 停機超過 1 小時後的第一筆不可信。
4. 關站 / 單臂：依 `iShuttleMode` / `iShuttle_Sel` 過濾（Sam 20201231 修正）。
5. `bIndexTimeSet` 為真時顯示假值（`0.9 + random(25)/100`）。
6. Col4 的 Average 只是畫面最近 9 筆的平均，不是長期平均（長期趨勢用 D70 CSV）。
7. `[D70]` 每累積 **10 筆**才寫檔 → 停機當下不足 10 筆的資料會遺失。
8. **右側 InArm 欄排序與左表相反**（Row 1 最新），且無 Average、不落地 → 客戶對數字時最常誤讀這裡。

---

## 9. UPH 記錄機制（摘要）

> 完整內容見 [uph-record.md](references/uph-record.md)

### 統計口徑（與 cycle time 完全不同，不可互推）

**一筆 UPH = 一盤入料盤（Loader Tray）用完為一個區間，不是每小時一筆。**
`UPH = 3600 ÷ (區間秒數 − Pause Time) × 該區間入料顆數` → **已扣停機**（cycle time 則不扣）。

- 觸發：換入料盤時 `bRecordUPH=true`（`asendic_Loader.cpp:1259`）→ 新盤第一顆取料時 `CalculateUPH(false)`（`ainarm9045.cpp:5757` / `5444`）。
- 計數是**入料顆數** `iUPH_LoaderCount`，不是出料。
- 另有一套**即時 UPH** `CaculateUPH()`（`cShowBinSelect.cpp:2325`，注意少一個 l）用出料 Bin 增量算 → `iNetUPH`/`iGrossUPH`。狀態列 Panel 2 與 Panel 7 來源不同，數字本來就不會一樣。

### 後台記錄位置

| 層級 | 位置 | 需開關 |
|---|---|---|
| **EventLog（預設就記）** | `D:\HT9045_Log\EventLogTxt\`，UnitName=`UPH`、Message=UPH 值（`MyDBIUPH()`，`cMyDB.cpp:291`） | ❌ 不需 |
| 生產記錄 | `D:\HT9045_Log\ProductRecord\ProductionRecordLog*.csv`、DB `Production` 表 `UPH` 欄（`RunInfo.iAvgUPH`） | ❌ 不需 |
| 專用 CSV | `D:\HT9045_Log\UPH\`，三種格式見 reference §5 | ✅ `[P11] Record UPH information` |
| 畫面 | Counter → `UPH` 頁（最近 10 筆 + `Avg UPH`）、狀態列、Observer → MDB Query 區間統計（`CountMTBF`） | `bShowUPH` / `bG10ShowImmediateUPH` |
| SECS | SVID **1021** UPH、**1028** Avg UPH、**1038** Gross UPH、**1039** Net UPH；CEID `UPHRecordEnd`(54) | — |

### `[P11]` 三種 CSV 格式（依 `CUSTOMER_CODE` 分流，`ainarm9045.cpp:5535`）

| 客戶 | 檔名 | 欄位 |
|---|---|---|
| 一般白名單客戶 | `<YYYY>_<MMDDHH>_UPH.csv`（每小時一檔） | 無 header，`hh:mm:ss, UPH` |
| `CC_KYEC_LEE` | `<LotID>_UPH.csv`（每 Lot 一檔） | `Start Time, End Time, Pause Time, UPH` |
| `CC_FOREHOPE_NINGBO` | `UPH\<yyyy>\<m>\<d>\<yyyymmdd>_UPH.csv`（每天一檔） | `Start Time, End Time, Pause Time, UPH, Tray Count, Site Count` |

> ⚠️ **客戶碼白名單**（`cConfiguration.cpp:4154`）：只有 `CC_KYEC_LEE`、`CC_KYEC_XILINX`、`CC_TERAPOWER`、`CC_SIGURD_ChungXing`、`CC_UTAC_TW`、`CC_FOREHOPE_NINGBO` 看得到 `[P11]`；其餘機台**選項不顯示且強制關閉**。客戶要就得加碼升版。
> ⚠️ 一般客戶那條路徑需 `bCloseExcelfinishflag` 才寫檔（Excel 關不掉會漏記）；且該格式與 KYEC 格式**每筆之間會多一個空行**（sprintf 已含 `\n`，`WriteDataToFile()` 又補一個）。

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
- [references/child-state-machines.md](references/child-state-machines.md)：repo 既有參考檔（同事整理）
