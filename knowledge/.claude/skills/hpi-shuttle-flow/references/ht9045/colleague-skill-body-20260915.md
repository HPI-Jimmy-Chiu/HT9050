# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


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

## 專案資訊

- **專案**: HT9045 IC Test Handler
- **原始碼根路徑（可自定義）**: `${HT9045_SOURCE_ROOT}`
- **預設值（範例）**: `d:\HT9045\HT9011UC_Code_V3.33.897.0_20260306\`
- **語言**: C++ (Borland C++ Builder 6, VCL framework, AnsiString)
- **架構**: State Machine pattern — `switch(Task)` 搭配 `int &Task` 參考變數
- **呼叫方式**: 函式回傳 `bool` 或 `void`，由主循環反覆呼叫
## 參考文件

- [Do_Auto_SHT1_SHT2_ProcessFlow.md](Do_Auto_SHT1_SHT2_ProcessFlow.md) — Shuttle 1/2 完整狀態機 Case 說明
- [Shuttle_OCR_2DID_BarcodeFlow.md](Shuttle_OCR_2DID_BarcodeFlow.md) — Shuttle OCR 2DID Barcode 掃碼流程、CCD 對應、已知問題
- [Shuttle-Arm-Safety-Interlock.md](Shuttle-Arm-Safety-Interlock.md) — Shuttle ↔ InArm/OutArm 三層安全互鎖機制（fCanMoveR / DoInOutARM_SHT_MoveSafe / OutSHTxInRT）
- [NN_2DID_PreScan_DuringIndexDown.md](NN_2DID_PreScan_DuringIndexDown.md) — NN 模式 2DID 下壓期預掃（2026-07-28 新功能）：`bNNShuttlePreScan2DID` 開關、`IsNNPreScan2DIDCanStart()`、case10 改道 / case210 hold、Index 端 8 重穩態判斷與 case110 busy guard

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

> **NN 2DID 預掃（`bNNShuttlePreScan2DID=true` 時，2026-07-28）**：case10 在 `FRCarryKit.UseSiteNoIC()`（OutShuttle 空）且 `IsNNPreScan2DIDCanStart(0)` 成立時，改道 `Task=200` 提前進掃碼鏈（與 Index 下壓測試並行）；掃完後 case210 檢查 `FTestSuck.UseSiteHasIC()`（IC 仍在測試）時強制 `Task=100` 回左邊停等（hold）。SHT2 於 case10/case210 對稱（`BLCarryKit`/`BTestSuck`）。詳見 [NN_2DID_PreScan_DuringIndexDown.md](NN_2DID_PreScan_DuringIndexDown.md)。

---

## 3. Do_Auto_SHT2() 主流程

> Task variable: `int &Task = AutoSHT2Task`

### Case 清單

`1, 10, 105, 100, 130, 124, 135, 125, 120, 115, 110, 140, 400, 500, 202, 200, 201, 220, 225, 230, 235, 210, 211, 212, 300, 5000, 5100, 2300, 2500, 2400, 3000, 2600`

### 流程分段

- 與 SHT1 對稱，主要差異為 Motor/Sensor/CarryKit 對象切換（`MInShuttle2`, `BRCarryKit`, `BLCarryKit`）。
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
6. NN 預掃不觸發 / 時序異常：先確認 `config.ini [Index] bNNShuttlePreScan2DID`，再依序核對 `IsIndexZ1Z2DownStableForPreScan()` 8 重條件與 `IsNNPreScan2DIDCanStart()` 排除清單（見 [NN_2DID_PreScan_DuringIndexDown.md](NN_2DID_PreScan_DuringIndexDown.md)）。

---

## 使用指引

回答 Shuttle 問題時建議順序：

1. 先判斷是 SHT1 還是 SHT2（對應 `AutoSHT1Task` / `AutoSHT2Task`）。
2. 再定位是左移主線、右移主線、或 barcode/floating 特殊支線。
3. 若涉及殘料/sensor 斷線，切到 mode-based switch（`TestIF.iTestMode`）解釋。
4. 若涉及卡機，補充安全互鎖（InArm/OutArm/Index safe move）條件。
