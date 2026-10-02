---
name: ht9045-shuttle-flow
description: HT9045 IC Test Handler Shuttle（InShuttle/OutShuttle）流程知識庫。當使用者詢問 Shuttle 移動、Do_Auto_SHT1、Do_Auto_SHT2、Shuttle 左右移動、Shuttle sensor broken、Shuttle 浮料（Floating）、Shuttle 殘料（Null IC / Lose IC）、Shuttle 2D Barcode、Rotate Shuttle 檢查、Shuttle retry/skip/home、Fix3 氣缸保護、Shuttle 與 Index/InArm/OutArm 安全互鎖等相關問題時，應先載入此技能以理解 Shuttle 完整處理流程。關鍵字：Do_Auto_SHT1, Do_Auto_SHT2, AutoSHT1Task, AutoSHT2Task, CheckShuttleOutputHasICError, CheckShuttleSensorBroken_1, CheckShuttleSensorBroken_2, CheckNullICShuttle1_9045, CheckNullICShuttle2_9045, InSHT1, InSHT2, Shuttle。
---

<!-- AI(W906-BA-SKILL) 20260915：正文換成網頁同事 20260915 那版（較新／較完整）。
     frontmatter 的路由描述保留我們的。
     我們原有但他沒有的段落，另存 references/ours-kept-20260915.md。 -->


# HT9045 Shuttle Flow Knowledge

## 適用場景

當使用者詢問以下主題時，載入此技能：
- Shuttle 1 / Shuttle 2 的動作流程、狀態機、case 數值意義
- `Do_Auto_SHT1()` / `Do_Auto_SHT2()` 主流程
- Shuttle 左移/右移流程與前置安全互鎖
- Shuttle 置偏、殘料、飛料、Floating 偵測
- Shuttle 2D Barcode/CCD 掃描流程
- Rotate Shuttle 檢查流程（left/right rotate check）
- `CheckShuttleOutputHasICError()` 模式化 (`TestMode`) 判斷
- `CheckShuttleSensorBroken_1/_2()` 斷線檢查流程
- Retry / Skip / Home / Alarm 路徑
- Shuttle 與 InArm/OutArm/Index 的安全保護 (`DoInOutARM_SHT_MoveSafe`)
- **三站雙 Kit 幾何**：站A/站B/站C、In-Kit / Out-Kit、`FLCarryKit` / `FRCarryKit` / `BLCarryKit` / `BRCarryKit` 命名語意、`iLeft` / `iRight` 停點、為何「Index 不動、Kit 在動」、每批應該只移動 2 次（見 §6）

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
- **呼叫方式**: 函式回傳 `bool` 或 `void`，由主循環反覆呼叫
## 參考文件

- [Do_Auto_SHT1_SHT2_ProcessFlow.md](references/Do_Auto_SHT1_SHT2_ProcessFlow.md) — Shuttle 1/2 完整狀態機 Case 說明
- [Shuttle_OCR_2DID_BarcodeFlow.md](references/Shuttle_OCR_2DID_BarcodeFlow.md) — Shuttle OCR 2DID Barcode 掃碼流程、CCD 對應、已知問題
- [Shuttle-Arm-Safety-Interlock.md](references/Shuttle-Arm-Safety-Interlock.md) — Shuttle ↔ InArm/OutArm 三層安全互鎖機制（fCanMoveR / DoInOutARM_SHT_MoveSafe / OutSHTxInRT）
- [NN_2DID_PreScan_DuringIndexDown.md](references/NN_2DID_PreScan_DuringIndexDown.md) — NN 模式 2DID 下壓期預掃（2026-07-28 新功能）：`bNNShuttlePreScan2DID` 開關、`IsNNPreScan2DIDCanStart()`、case10 改道 / case210 hold、Index 端 8 重穩態判斷與 case110 busy guard

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `acarry.cpp` | `Do_Auto_SHT1()` — Shuttle 1 主狀態機 | `AutoSHT1Task` |
| `acarry.cpp` | `Do_Auto_SHT2()` — Shuttle 2 主狀態機 | `AutoSHT2Task` |
| `acarry.cpp` | `CheckShuttleOutputHasICError()` — 依測試模式檢查 Shuttle 殘料/飛料 | （switch `TestIF.iTestMode`） |
| `acarry.cpp` | `CheckShuttleSensorBroken_1()` — SHT1 sensor broken 檢查 | （switch `TestIF.iTestMode`） |
| `acarry.cpp` | `CheckShuttleSensorBroken_2()` — SHT2 sensor broken 檢查 | （switch `TestIF.iTestMode`） |
| `acarry.cpp` | `CheckNullICShuttle1_9045()` / `CheckNullICShuttle2_9045()` | （無 switch） |
| `acarry.cpp` | `DoInOutARM_SHT_MoveSafe()` — InArm/OutArm/Shuttle 移動安全互鎖 | （無 switch） |
| `acarry.cpp` | `IsNNPreScan2DIDCanStart()` — NN 2DID 預掃 Shuttle 端前置條件（2026-07-28） | （無 switch） |
| `atester_32Site.cpp` | `IsIndexZ1Z2DownStableForPreScan()` — NN 2DID 預掃 Index 端穩態判斷（2026-07-28） | `iTestTwoArm32SiteTask` |

---

## 1. 呼叫階層總覽

```text
Do_Auto_SHT1() / Do_Auto_SHT2()                        [acarry.cpp]
  |- switch(Task) 主流程（左移、右移、旋轉檢查、Barcode、Floating）
  |- CheckNullICShuttle1_9045 / CheckNullICShuttle2_9045
  |- CheckCFixTrayFullPlace
  |- CheckShtFloating
  |- DoCheckShuttle1EmptyIC / DoCheckShuttle2EmptyIC
  |- DoInOutARM_SHT_MoveSafe
  |
  `- Child switch（mode-based）
      |- CheckShuttleOutputHasICError()   -> switch(TestIF.iTestMode)
      |- CheckShuttleSensorBroken_1()     -> switch(TestIF.iTestMode)
      `- CheckShuttleSensorBroken_2()     -> switch(TestIF.iTestMode)
```

---

## 2. Do_Auto_SHT1() 主流程

> Task variable: `int &Task = AutoSHT1Task`

### Case 清單

`1, 10, 105, 100, 130, 124, 135, 125, 120, 115, 110, 140, 400, 500, 202, 200, 201, 220, 225, 230, 235, 3000, 210, 211, 212, 300, 5000, 5100, 2300, 2500, 2400, 2600`

### 流程分段

1. 初始化：`1 -> 10`
2. 左移分支：`100 -> (130->124->135->125 可選) -> 120 -> 1`
3. 右移入口：`202 -> 200 -> 201`
4. 右移分支：`201 -> (220->225 / 230->235 可選) -> 210 -> 1`
5. 特殊分支：`2300`(Floating), `2400/2500`(2DID), `2600`(HTTP retry)
6. 錯誤分支：`400 -> 500 -> 110 -> 100`（重試回圈）

> **NN 2DID 預掃（`bNNShuttlePreScan2DID=true` 時，2026-07-28）**：case10 在 `FRCarryKit.UseSiteNoIC()`（OutShuttle 空）且 `IsNNPreScan2DIDCanStart(0)` 成立時，改道 `Task=200` 提前進掃碼鏈（與 Index 下壓測試並行）；掃完後 case210 檢查 `FTestSuck.UseSiteHasIC()`（IC 仍在測試）時強制 `Task=100` 回左邊停等（hold）。SHT2 於 case10/case210 對稱（`BLCarryKit`/`BTestSuck`）。詳見 [NN_2DID_PreScan_DuringIndexDown.md](references/NN_2DID_PreScan_DuringIndexDown.md)。

---

## 3. Do_Auto_SHT2() 主流程

> Task variable: `int &Task = AutoSHT2Task`

### Case 清單

`1, 10, 105, 100, 130, 124, 135, 125, 120, 115, 110, 140, 400, 500, 202, 200, 201, 220, 225, 230, 235, 210, 211, 212, 300, 5000, 5100, 2300, 2500, 2400, 3000, 2600`

### 流程分段

- 與 SHT1 對稱，主要差異為 Motor/Sensor/CarryKit 對象切換（`MInShuttle2`, `BRCarryKit`, `BLCarryKit`）。

---

## 4. GetNowSiteKitMode 與 CheckXYPitch（Steven 2023.07 軟體重構）

> **來源**：Steven Chou《邏輯機台_軟體架構.pptx》Slide 33-36
> **架構總覽**：見 [ht9045-sucker-architecture](../ht9045-sucker-architecture/SKILL.md) §7
> **基準版本**：V3.33.904.2_20260511_RogerYang（V3.33.8XX+ 後皆適用）

### 4.1 Mode 編碼規則

Shuttle 在初始化時即判斷 InArm 在 Shuttle 上面要使用的 X / Y 軸移動次數，編碼為單一 `int`：

```text
Mode = (X 倍數) * 100 + (Y 倍數)

Mode / 100  → X 軸要移動的標準 X 間距「倍數」（1 = 標準 X-Pitch）
Mode % 100  → Y 軸要移動的標準 Y 間距「倍數」（1 = 標準 Y-Pitch）
```

> ⚠️ **常見誤區**：不要把 `Mode` 直接當「吸取數量」用，它是**倍數編碼**。

### 4.2 Auto Clean 模式下的特殊處理

| 一般模式 | Auto Clean 模式 |
|---------|----------------|
| 以「**預計要放入的** Shuttle Row」為基準 | 以「**預計要吸取的** Shuttle Row」為基準 |

實作位置範例：`ainarm9045_2x4_16.h` L15 — `GetNowSiteKitMode_2x4_16(int iShuttle, bool bPlace)`

### 4.3 CheckXYPitch Teaching 計算範例（HT-9046 2x4_16 標準模式）

```text
X Teaching                       = 35038
E to Center                      = 35038 - 2000 = 33038
X-Pitch from A to G              = 8000
A to E                           = 8000 / 3 * 2 = 5333.33
A move to Center                 = 33038 + 5333.33 = 38371.33

A to site Aa (X pitch × -1.5)    = 38371.33 - (8000 × 1.5) = 26371.33
A to site Ac (X pitch ×  0.5)    = 38371.33 + (8000 / 2)   = 42371.33
```

### 4.4 公式抽象化

```text
吸嘴 Aa 移動到基準軸位置          = (N-1) 個 Arm X-Pitch          (N = 吸嘴 Col 數)
計算每隻吸嘴間的 Arm X-Pitch     = (XPitch_total) / (N - 1)
吸嘴 Aa 移動到第一個 Site 位置   = (SiteCount/2 - 0.5) 個 Site Pitch
```

### 4.5 重構後 Loader 共用 Function 對照

| 函式 | 角色 | 位置（V3.33.904.2） |
|------|------|--------------------|
| `MoveArmXYToLoaderStage()` | 對外介面 | `ainarm9045.cpp` |
| `DoMoveArmXYToLoaderStage()` | 根據模式計算 X-Pitch 與 Step | `ainarm9045.cpp` |
| `GetInArmToLoaderPosition(iSelRow, iXPos, iYPos, iRow, iCol)` | 多吸嘴 Loader 取料位置 | `ainarm9045.cpp` L4614 |
| `GetInArmToLoaderPosition_Single(...)` | 單吸嘴版本（回傳 `iMovePitchX`） | `ainarm9045.cpp` L4673 |

> **Eastsun 20251231 註**：`InArmOffSet[InOfsLoader]->GetArmX(i,j)` 必須加在 `GetInArmToLoaderPosition`（多吸嘴版），不可放 `_Single`，因 `_Single` 無法判斷吸嘴編號。

### 4.6 關鍵 Source 位置（V3.33.904.2 基準）

| 主題 | 檔案 / 行號 |
|------|-------------|
| `GetNowSiteKitMode_2x4_16` 宣告 | `ainarm9045_2x4_16.h` L15 |
| `CheckXYPitch_2x4_16` 宣告 | `ainarm9045_2x4_16.h` L16 |
| `GetInArmToLoaderPosition` | `ainarm9045.cpp` L4614 |
| `GetInArmToLoaderPosition_Single` | `ainarm9045.cpp` L4673 |


- 包含 one-cycle 條件分支與對稱的 retry/alarm 路徑。

---

## 4. 子函式 switch（下一層）

### 4.1 CheckShuttleOutputHasICError

- switch 變數：`TestIF.iTestMode`
- 用途：依 test mode 逐格比對 Shuttle sensor 與 CarryKit 資料，判定殘料/飛料
- 主要 case：`SingleSite`, `DualSite`, `QualSite2X2N`, `QualSite1X4`, `_8Site1X4`, `_8Site2X4N`, `TriSite1X3`, `_6Site2X3N`, `QualSite2X2`, `_6Site2X3`, `_8Site2X4`, `_16Site4X4`, `_10Site2X5`, `_12Site2X6`, `_16Site2X8`, `_32Site4X8N`

### 4.2 CheckShuttleSensorBroken_1 / _2

- switch 變數：`TestIF.iTestMode`
- 用途：Shuttle 左->右->左運動期間，確認對應 sensor 曾被遮斷，防止斷線漏檢
- 主要 case：`DualSite2x1`, `SingleSite`, `DualSite`, `QualSite2X2N`, `QualSite2X2`, `TriSite1X3`, `_6Site2X3`, `_6Site2X3N`, `QualSite1X4`, `_8Site1X4`, `_16Site4X4`, `_8Site2X4`, `_8Site2X4N`, `_16Site2X8`, `_10Site2X5`, `_12Site2X6`, `_32Site4X8N`

---

## 5. 典型除錯入口

1. 看 Shuttle 卡在哪個 case：先讀 `AutoSHT1Task` / `AutoSHT2Task` 即時值。
2. 左移異常：優先檢查 `100/120/130/135` 路徑與 Rotate 檢查條件。
3. 右移異常：看 `200/201/210`，再分流到 `2300/2400/2500/2600`。
4. 重試無法恢復：檢查 `400/500/110` 路徑是否反覆進入。
5. 殘料判斷爭議：比對 `CheckShuttleOutputHasICError()` 的 `TestMode` case。
6. NN 預掃不觸發 / 時序異常：先確認 `config.ini [Index] bNNShuttlePreScan2DID`，再依序核對 `IsIndexZ1Z2DownStableForPreScan()` 8 重條件與 `IsNNPreScan2DIDCanStart()` 排除清單（見 [NN_2DID_PreScan_DuringIndexDown.md](references/NN_2DID_PreScan_DuringIndexDown.md)）。

---

## 6. 三站雙 Kit 幾何 — 「Shuttle 在動，Index 不動」

畫動作圖／解釋料流時最常搞錯的一點：**每條 Shuttle 軌只有一顆馬達、兩個停點，
托板上卻有兩個 Kit**。所以同一次移動會讓兩個 Kit 各換一個「站」。

| 元件 | 對應 |
|---|---|
| 馬達 | `MInShutte1`（前／SHT1）、`MInShutte2`（後／SHT2）—— **整機只有這兩顆** |
| 停點 | `Prod.InSHT[n].iLeft` / `.iRight` |
| 托板上的 Kit | `FLCarryKit`＝前軌**左** Kit＝**In-Kit**（未測料）、`FRCarryKit`＝前軌**右** Kit＝**Out-Kit**（已測料）；後軌為 `BLCarryKit` / `BRCarryKit` |

> ⚠ `F`/`B` 是**前／後 Shuttle**，`L`/`R` 是**該托板上的左／右 Kit**。
> 不要讀成「前左 Shuttle／前右 Shuttle」（那會變成四條軌，實機沒有）。
> 碼證：`acarry.cpp:158` `TMyKitSuck *pKit=(iSht==0)? &FRCarryKit : &BRCarryKit;`

### 三個站

| Shuttle 位置 | 站 A（InArm 側） | 站 B（Index／Socket 側） | 站 C（OutArm 側） |
|---|---|---|---|
| `iLeft` | In-Kit ← InArm 放**未測**料 | Out-Kit ← Index 放**已測**料 | — |
| `iRight` | — | In-Kit → Index **取**未測料 | Out-Kit → OutArm **取**已測料 |

推導依據（三條互相印證）：

1. `csystem.cpp:701` `bool OutSHT1InRT(){ return InSHT1InRT(); }`
   → In/Out Shuttle 1 是**同一個物件**，「In/Out」只是同一托板上的兩個 Kit。
2. Index **取**料要 `InSHT?InRT()`、**放**料要 `InSht?InLF()`
   （`aTester_Front.cpp` case 200/300）→ **Index 固定在站 B，是 Kit 在動**。
3. InArm／OutArm 各只有**一個** Shuttle X 教點
   （`teach.ini` `MInArmX SHT1=35115`、`MOutArmX SHT1=-47573`）
   → A、B、C 是三個不同的 X，**兩站間距 ＝ Shuttle 行程**（本機約 469 mm）。

### 對排程的意義（做動作模擬／查「為什麼一直左右跑」時最有用）

一次左右來回（`iLeft → iRight → iLeft`）就能做完四件事：
左邊「InArm 放 In-Kit ＋ Index 放 Out-Kit」，右邊「Index 取 In-Kit ＋ OutArm 取 Out-Kit」。

⇒ **每批的 Shuttle 移動次數應該是 2 次（右移一次、左移一次）**。
若量到 4 次，表示每個動作各自去要求停位、沒有把「同一停位能做的事」批次化 ——
實機不會這樣跑（左右狂移），是排程／模擬的缺陷。

視覺化與可執行的排程模型見
[ht9045-motionview-html-ui](../ht9045-motionview-html-ui/SKILL.md)。

---

## 使用指引

回答 Shuttle 問題時建議順序：

1. 先判斷是 SHT1 還是 SHT2（對應 `AutoSHT1Task` / `AutoSHT2Task`）。
2. 再定位是左移主線、右移主線、或 barcode/floating 特殊支線。
3. 若涉及殘料/sensor 斷線，切到 mode-based switch（`TestIF.iTestMode`）解釋。
4. 若涉及卡機，補充安全互鎖（InArm/OutArm/Index safe move）條件。

---

## Shuttle Move Timeout watchdog 判讀

`Shuttle%d Move Timeout, Enc=… Tar=…` **不是既有 alarm，是 2026-04-30 加的 watchdog**
（`acarry.cpp` `DoShtMoveTimeoutHandle()` L3341；timer 於 2026-05-07 由 `GetTickCount` 改為 `TQPF_Timer`）。
它本身**不是故障，是把既有的互鎖卡死現形**，看到它要往「誰壓著 shuttle 不讓它動」去查。

### 觸發條件

`Do_Auto_SHT1/2` **case 120**（往左移到等待區）的
`MOT[MInShuttleN].MotorMove(Prod.InSHT[N-1].iLeft)` **連續 30 秒回傳 ≠ 1**
（SHT1 @L4241-4248、SHT2 @L6128-6136，`SHUTTLE_MOVE_TIMEOUT_SEC = 30.0`）。

觸發後：dump latch → `ShowMyMessage` 阻塞（**不計 JAM/WAR KPI，EventLog 只留 `Message` 列**）→ 操作員按 OK 後清 latch／清方向旗標／`AutoSHTxTask=1`／`fAllMotorHome=false; iHome=1` 觸發整機 Home。
`ResetShtMoveTimeoutWatchdog()`（L3393）在 PAUSE/START 邊界重置，避免 PAUSE 期間誤計時。

### 欄位怎麼讀

| 欄位 | 意義 | 判讀 |
|---|---|---|
| `Enc` | `MOT[MInShuttleN].ReadPos()` | 對照 `system\teach.ini` `[MInShutteN]` 的 `setEditInShtNLeft` / `setEditInShtNRight` |
| `Tar` | `Prod.InSHT[N-1].iLeft`（左等待位，含 offset） | 應 ≈ `setEditInShtNLeft` |
| `RetryCnt` | `iOutShuttleNHasICErrRetryCnt` | ≠0 ⇒ 卡在殘料複檢迴圈 |
| `LtcCnt0/1/5` | `fLtcSensor->LatchDataCntX` | 全 0 ⇒ 沒有 Latch 誤觸發；≠0 要查 sensor |

**`Enc` ≈ 右位 且 30 秒沒變 ⇒ shuttle 根本沒動**（不是走到一半卡住、不是掉步）。

### 可以先排除的路徑

case 120 開頭的 `IsTestZ2NotSafeShuttle2CanNotMove()`（L5385，SHT1 對應 `…Z1…`）若成立會直接 `break`，
**MotorMove 不會被呼叫、watchdog 也不會啟動**。
⇒ **只要 watchdog 有觸發，就代表 Z 安全位這道閘門是通過的**，問題出在 `MotorMove()` 自己不動
⇒ 幾乎必然是 `MOT[MInShuttleN].fCanMove` 被別的單元壓著（Index / InArm / OutArm）。

### 實例：2026-09-07 SCK HT9046LS

```
19:34:51  JAM0302 Index arm 2 device pick-up error! "Ab" → RETRY
19:35:16  ChangeLog Test Arm2_Pick Up 21.00→20.90、Place 23.00→22.90（現場調取放高度）
19:35:28  START
19:36:00  Shuttle2 Move Timeout, Enc=47763 Tar=675, RetryCnt=0, LtcCnt0=0 LtcCnt1=0 LtcCnt5=0
          teach.ini [MInShutte2] Left=667 / Right=47730  ⇒ 停在右邊完全沒動
19:36:07  HOME CAUSE >> … fRearNeedSuck=1 … Y1 -113/-113 Y2 168/168 Z1 200/200 Z2 200/200 (cmd/enc)
19:42:25  HOME CAUSE >> … fRearNeedSuck=1 …（同上，HOME 解不開）
19:47~51  七道門連開（含 Heater Door 1）→ 人工清機 → 19:51:26 關程式 → 19:54 重開 → HOME
```

- 四軸 **cmd == enc**，機構都到位 ⇒ 不是掉步、不是沒到位
- `fRearNeedSuck=1` 是 JAM0302 RETRY 留下的未完成狀態，**兩次 HOME 都沒清掉**
- GPIB log 全程 `Arm 1 Down` × 228、`Arm 2 Down` × 0、`Test arm?` 一律回 `1` ⇒ **單臂模式**
- 前提條件與偉測 HHT-280 Pattern #27（單臂模式跨臂互鎖、HOME 解不開）相同，但本案 Z1/Z2 皆 200/200 看似健康，**可能是別的旗標**，尚未收斂

**後果**：操作員最後只能開門把機台裡 20 顆料用手清掉並重開軟體 → 入料帳永久偏差 20 顆 → 8 小時後爆成 WAR0120。
完整下游見 `ht9045-art-flow` §14。

### 復發時要抓什麼（建議擴進 `DoShtMoveTimeoutHandle()` 的唯讀 dump）

```
MOT[MInShuttleN].fCanMove / fCanMoveL / fCanMoveR / fCanMoveM
IndexZCanMove[0] / IndexZCanMove[1]
fRearNeedSuck / fRearNeedSuckIC / fFrontNeedSuck / fFrontNeedTest / fRearNeedDestroy
iTestTask / iTestHeadMotorTask / iTestYTask / AutoSHTNTask / iPickFromShuttleNTask
MTestY1 / MTestY2 / MTestZ1 / MTestZ2  的 cmd vs enc
```
只在 timeout 觸發時跑一次，頻率極低，風險低。
**同時要請客戶在看到這個 message 時先做 State Record 再按 OK**（按 OK 會觸發 Home，現場狀態就沒了）。

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
