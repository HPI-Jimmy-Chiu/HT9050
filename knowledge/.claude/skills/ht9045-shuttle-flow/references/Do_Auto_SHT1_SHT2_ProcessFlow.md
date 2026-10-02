# Do_Auto_SHT1 / Do_Auto_SHT2 Process Flow

> Source: `acarry.cpp`
> Functions: `Do_Auto_SHT1()`, `Do_Auto_SHT2()`
> Project: `HT9011UC_Code_V3.33.897.0_20260306`

---

## 1. Call Hierarchy

```text
Do_Auto_SHT1() / Do_Auto_SHT2()                          [acarry.cpp]
  |- Left/Right shuttle move state machine (switch Task)
  |- Sensor / Null IC / Floating / Barcode checks
  |- Error retry / alarm / skip-home path
  |
  |- (next-layer, no switch)
  |   |- CheckNullICShuttle1_9045 / CheckNullICShuttle2_9045
  |   |- CheckCFixTrayFullPlace
  |   |- DoStepShuttleCheck
  |   |- DoCheckShuttle1EmptyIC / DoCheckShuttle2EmptyIC
  |   |- CheckShtFloating
  |   `- DoInOutARM_SHT_MoveSafe
  |
  `- Related child functions with switch
      |- CheckShuttleOutputHasICError(iSelSHT, X, Y)     [switch(TestIF.iTestMode)]
      |- CheckShuttleSensorBroken_1(...)                 [switch(TestIF.iTestMode)]
      `- CheckShuttleSensorBroken_2(...)                 [switch(TestIF.iTestMode)]
```

---

## 2. Main State Machine: Do_Auto_SHT1()

- **Task variable**: `int &Task = AutoSHT1Task`
- **Core purpose**: 控制 Shuttle 1 左移/右移、置偏檢查、2D Barcode、Floating、重試與告警。

### 2.1 Case List (switch Task)

`1, 10, 105, 100, 130, 124, 135, 125, 120, 115, 110, 140, 400, 500, 202, 200, 201, 220, 225, 230, 235, 3000, 210, 211, 212, 300, 5000, 5100, 2300, 2500, 2400, 2600`

### 2.2 Phase Map

```text
Init
  1 -> 10

Left-move branch
  100 -> (130->124->135->125 optional rotate check) -> 120 -> 1
  105 / 115 / 110 / 140 are side paths for null-IC, empty-IC, retry delay, step-check

Right-move branch
  202 -> 200 -> 201
  201 -> (220->225 / 230->235 optional rotate check) -> 210 -> 1

Special branch
  201 -> 2300 (floating)
  201 -> 2400 / 2500 (2D barcode)
  2400 -> 2600 (HTTP retry delay) -> 201
  3000 -> 201

Error branch
  400 -> 500 -> 110 -> 100 (retry loop)
  210 -> 211 -> 212 (error bin record)
```

### 2.3 Key Transitions

- `case 1`: 確認可動作後進入 `10`。
- `case 10`: 依 one-cycle / clean-out 與前後吸嘴狀態，決定左移流程或右移流程。
- `case 100`: 若啟用 Rotate Shuttle 檢查，走 `130`; 否則直接走 `120`。
- `case 120`: 左移到位後做 sensor/IC 驗證，完成回 `1`。
- `case 200/201`: 右移前決策點，分流到 floating、barcode 或直接右移。
- `case 210`: 右移到位後驗證，完成回 `1`。
- `case 2300`: Floating check 完成後回 `201`。
- `case 2400/2500`: 2DID 掃描完成回 `210` 或 `201`。
- `case 2600`: HTTP 延遲重試完成回 `201`。
- `case 400/500/110`: 錯誤告警與 retry 節點。

---

## 3. Main State Machine: Do_Auto_SHT2()

- **Task variable**: `int &Task = AutoSHT2Task`
- **Core purpose**: 與 SHT1 對稱，控制 Shuttle 2 左移/右移、檢查與錯誤處理。

### 3.1 Case List (switch Task)

`1, 10, 105, 100, 130, 124, 135, 125, 120, 115, 110, 140, 400, 500, 202, 200, 201, 220, 225, 230, 235, 210, 211, 212, 300, 5000, 5100, 2300, 2500, 2400, 3000, 2600`

### 3.2 Phase Map

```text
Init
  1 -> 10

Left-move branch
  100 -> (130->124->135->125 optional rotate check) -> 120 -> 1

Right-move branch
  202 -> 200 -> 201
  201 -> (220->225 / 230->235 optional rotate check) -> 210 -> 1

Special branch
  201 -> 2300 (floating)
  201 -> 2400 / 2500 (2D barcode)
  2400 -> 2600 (HTTP retry delay) -> 201
  3000 -> 201

Error branch
  400 -> 500 -> 110 -> 100 (retry loop)
  210 -> 211 -> 212 (error bin record)
```

### 3.3 SHT2-Specific Notes

- 使用 `MInShuttle2` / `BRCarryKit` / `BLCarryKit`。
- case 結構與 SHT1 近乎鏡像，僅參數與 sensor index 對應不同。
- `case 10` 有 one-cycle 額外判斷（`CheckOneCycleAction` 路徑）。

---

## 4. Next Layer Functions (Called by SHT1/SHT2)

## 4.1 No-switch helper group

- `CheckNullICShuttle1_9045()` / `CheckNullICShuttle2_9045()`
- `CheckCFixTrayFullPlace()`
- `DoStepShuttleCheck()`
- `DoCheckShuttle1EmptyIC()` / `DoCheckShuttle2EmptyIC()`
- `CheckShtFloating()`
- `DoInOutARM_SHT_MoveSafe()` — 詳見 [Shuttle-Arm-Safety-Interlock.md](Shuttle-Arm-Safety-Interlock.md)

These functions are heavily used as gating/validation steps in many cases, but are not `switch(Task)` state machines in `acarry.cpp`.

**Note**: `DoInOutARM_SHT_MoveSafe()` 在所有呼叫點均被 `IniConfig.bF21InOutArmZMotorPrivate` 門控。
此函式包含 `DoINARM_SHT_MoveSafe` + `DoOutARM_SHT_MoveSafe` 兩個子判斷，各自檢查
arm XY 位置是否落在 shuttle 保護區內且 Z 軸不在 Home。觸發時強制停 shuttle 馬達並移 arm 到安全位。

## 4.2 Child switch function: CheckShuttleOutputHasICError

- **Function**: `CheckShuttleOutputHasICError(int iSelSHT, int &X, int &Y)`
- **switch variable**: `TestIF.iTestMode`
- **Purpose**: 依 site 模式判斷 Out Shuttle 各位置是否有殘料/飛料不一致。

### Cases

`SingleSite, DualSite, QualSite2X2N, QualSite1X4, _8Site1X4, _8Site2X4N, TriSite1X3, _6Site2X3N, QualSite2X2, _6Site2X3, _8Site2X4, _16Site4X4, _10Site2X5, _12Site2X6, _16Site2X8, _32Site4X8N`

---

## 4.3 Child switch function: CheckShuttleSensorBroken_1

- **Function**: `CheckShuttleSensorBroken_1(bool bRefreshCheck, bool bRight)`
- **switch variable**: `TestIF.iTestMode`
- **Purpose**: Shuttle 1 左->右->左過程中檢查 sensor 是否曾被遮斷，避免斷線漏檢。

### Cases

`DualSite2x1, SingleSite, DualSite, QualSite2X2N, QualSite2X2, TriSite1X3, _6Site2X3, _6Site2X3N, QualSite1X4, _8Site1X4, _16Site4X4, _8Site2X4, _8Site2X4N, _16Site2X8, _10Site2X5, _12Site2X6, _32Site4X8N`

---

## 4.4 Child switch function: CheckShuttleSensorBroken_2

- **Function**: `CheckShuttleSensorBroken_2(bool bRefreshCheck, bool bRight)`
- **switch variable**: `TestIF.iTestMode`
- **Purpose**: Shuttle 2 的 sensor broken 檢查，邏輯與 `_1` 對稱。

### Cases

`DualSite2x1, SingleSite, DualSite, QualSite2X2N, QualSite2X2, TriSite1X3, _6Site2X3, _6Site2X3N, QualSite1X4, _8Site1X4, _16Site4X4, _8Site2X4, _8Site2X4N, _16Site2X8, _10Site2X5, _12Site2X6, _32Site4X8N`

---

## 5. End-to-End Condensed Flow

```text
Do_Auto_SHT1 / Do_Auto_SHT2
  -> init & readiness check
  -> left move (with optional rotate-check path)
  -> right move entry
  -> floating / barcode / laser related branch (optional)
  -> right move complete
  -> sensor/IC consistency verification
  -> done (Task back to 1)
  -> if error: alarm/retry/skip/home loop
```

---

## 6. Notes

- 兩個主函式為對稱設計：SHT1/SHT2 case 編號架構一致，差異在 motor/sensor/carrier 對象。
- `AutoSHT1Task` / `AutoSHT2Task` 是主循環核心狀態變數。
- 內層大量 helper function 以 `if/else` 為主，真正 child `switch` 主要集中在 mode-dependent 檢查函式。
- 若需進一步展開 `DoCheckShuttle1EmptyIC/DoCheckShuttle2EmptyIC` 或 `CheckShuttleSensor_9045` 外部定義，可再生成第二層 flow 文件。
