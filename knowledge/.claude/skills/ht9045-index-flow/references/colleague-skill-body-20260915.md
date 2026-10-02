# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


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
- IndexEveryTimeCheckEP 充氣壓力檢查
- 32-site 兩臂測試模式
- Index 掉料 / Destroy / Suck 處理
- CCD TCPIP 檢測、F16 shuttle sensor broken check
- Auto Clean 下壓清潔流程（DoIndexAutoClean）、NN 模式兩臂同動 Auto Clean（ACSim）

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
- **呼叫方式**: 函式回傳 `bool`，`true` = 動作完成，`false` = 尚未完成（主迴圈反覆呼叫）
## 參考文件

- [DoTestHeadMotor_ProcessFlow.md](references/DoTestHeadMotor_ProcessFlow.md) — Index Arm 完整狀態機 Case 說明
- [Z1UpZ2Down_MotionSequence.md](references/Z1UpZ2Down_MotionSequence.md) — Z1UpZ2Down1 / Z1DownZ2Up1 向量運動序列、3 段式 LI 安全結構、運動中保護檢查
- [AutoClean_TwoArmSimultaneous.md](references/AutoClean_TwoArmSimultaneous.md) — NN 模式 Auto Clean 兩臂同動（ACSim）：逐臂取料（Arm1 提早開吸）+ 同時下壓 + 平行放回狀態機（case 4000~4750）、進入條件 11 項、降級路徑 A/C/D、真空壓降限制、交叉 guard 與 shuttle window、v1~v5 迭代教訓
- [NN_2DID_PreScan_DuringIndexDown.md](../ht9045-shuttle-flow/references/NN_2DID_PreScan_DuringIndexDown.md) — NN 模式 2DID 下壓期預掃（2026-07-28，跨 skill 共用）：Index 端 `IsIndexZ1Z2DownStableForPreScan()` 8 重穩態判斷、case240 穩定延遲 timer、case110 busy guard（MotorMove 假成功地雷）

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
