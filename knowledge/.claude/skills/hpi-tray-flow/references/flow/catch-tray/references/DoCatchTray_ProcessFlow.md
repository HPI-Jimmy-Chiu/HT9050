> 保存來源：`.claude/skills/ht9045-catchtray-flow/references/DoCatchTray_ProcessFlow.md`，main `9d9dfa9c7`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../../common.md)。

<!-- preserved-content:start -->
# DoCatchTray Process Flow

> Source: `acatchtray.cpp`
> Function Root: `DoCatchTray()`
> Project: `HT9011UC_Code_V3.33.897.0_20260306`

---

## 1. Call Hierarchy (Switch-Oriented)

```text
DoCatchTray()                                      [acatchtray.cpp]
  |- DoCatchFromLoader()                           [switch(Task)]
  |   |- C_CatchTray_Fix_Pop()                     [switch(iTask)]
  |   `- C_CatchTray_Fix_Puch()                    [switch(iTask)]
  |
  |- DoCatchUnderTray(iWhichAuto)                  [switch(Task)]
  |- CatchNewTrayFromBuffer(iWhichAuto)            [switch(Task)]
  |- DoPlaceTrayToAuto(AutoTarget)                 [switch(Task)]
  |   `- DoSupportUnderTray(iWhichAuto)            [switch(Task)]
  |
  |- DoPlaceToBuffer()                             [switch(Task)]
  |   `- DoPlaceBufferTray(iWhichAuto)             [switch(Task)]
  |
  |- DoSlapTray(bool bInit)                        [switch(Task)]
  `- (related child) DoLoadCarRotArmReadRFID()     [switch(Task)]
```

---

## 2. Root State Machine: DoCatchTray()

- **Signature**: `void DoCatchTray()`
- **Task variable**: `int &Task = CatchTrayTask`
- **Switch variable**: `Task`

### 2.1 Case List

`1, 10, 15, 20, 30, 40, 50, 100, 140, 145, 147, 150, 160, 170, 200, 250, 260, 300, 350, 400, 500, 600, 1000, 1100, 1150, 1200, 2000, 2100, 2120, 2130, 2140, 2160, 3000, 3010, 3100, 4000, 4100, 4200, 5000, 5100, 5150, 5200, 6000, 6010, 6030, 6100`

### 2.2 Phase Map

```text
Init / Safety
  1 -> 10 -> 15 -> 20 -> 30 -> 40 -> 50

Catch Empty Tray From Loader
  100 -> 140 -> 145 -> 147 -> 150 -> 160 -> 170
  -> 200 -> 250 -> 260

Need Tray Decision / Buffer Catch
  300 -> 350 -> 400
  -> 500 / 600 (CatchNewTrayFromBuffer)

Place Tray To Auto
  1000 -> 1100 -> 1150 -> 1200

Place Tray To Buffer
  2000 -> 2100 -> 2120 -> 2130 -> 2140 -> 2160

Manual / Mapping / Clean-up
  3000 -> 3010 -> 3100
  4000 -> 4100 -> 4200
  5000 -> 5100 -> 5150 -> 5200
  6000 -> 6010 -> 6030 -> 6100
```

### 2.3 Key Routing Notes

- `DoCatchFromLoader()` 是主流程前段核心（吸空 Tray + 夾爪/上蓋/退讓位）。
- `CatchNewTrayFromBuffer()` 是中段補盤核心（Empty/Color/Auto2 路徑分流）。
- `DoPlaceTrayToAuto()` 與 `DoPlaceToBuffer()` 是後段放盤主路徑。
- `DoCatchUnderTray()` / `DoSupportUnderTray()` / `DoPlaceBufferTray()` 主要在下層 Conveyor 模式與補盤子流程中被呼叫。

---

## 3. Next Layer Switch Functions

## 3.1 DoCatchFromLoader

- **Signature**: `int DoCatchFromLoader()`
- **Task variable**: `int &Task = iCatchFromLoaderTask`
- **Cases**:

`1, 10, 50, 90, 95, 100, 110, 120, 145, 150, 155, 156, 160, 161, 162, 163, 165, 170, 230, 240, 250, 300, 301, 305, 310, 400, 410, 500, 501, 502, 503, 550, 560, 570, 600`

- **Purpose**: 從 Loader 吸空 Tray，含吸取重試、夾爪控制、拍盤、移動到目標軌道。

## 3.2 CatchNewTrayFromBuffer

- **Signature**: `int CatchNewTrayFromBuffer(int iWhichAuto)`
- **Task variable**: `int &Task = iCatchNewTrayFromBufferTask`
- **Cases**:

`1, 2100, 2101, 2102, 2103, 2149, 2150, 2160, 2165, 2180, 2185, 2190, 2191, 2199, 2200, 2250, 2260, 2300, 2310, 2340, 2350, 2351, 2352, 2353, 2355, 2360, 2400, 2450, 2500, 2550, 2600, 2700, 4000`

- **Purpose**: 從 Empty/Color/Auto2 抓新 Tray 並處理失敗重試、警報、fallback。

## 3.3 DoPlaceTrayToAuto

- **Signature**: `bool DoPlaceTrayToAuto(int AutoTarget)`
- **Task variable**: `int &Task = iPlaceTrayToAutoTask`
- **Cases**:

`1, 100, 150, 200, 210, 220, 240, 250, 260, 300, 400, 401, 500, 510, 520, 530, 540, 600, 900, 1000, 1030, 1040, 1050, 1060, 1070, 1100, 1130, 1140, 1150, 1160, 1200, 1300`

- **Purpose**: 把 Tray 放到 Auto 軌道（定位、壓盤、Fixer、EdgePush、AutoAlignment、Mag 路徑）。

## 3.4 DoPlaceToBuffer

- **Signature**: `bool DoPlaceToBuffer()`
- **Task variable**: `int &Task = iPlaceToBufferTask`
- **Cases**:

`1, 2100, 2150, 2160, 2250, 2251, 2260, 2270, 2280, 2300, 2350, 2351, 2352, 2500, 2600, 2900, 3000`

- **Purpose**: 將 Tray 放回 Empty/Color buffer，含保護與 retry。

## 3.5 DoSlapTray

- **Signature**: `bool DoSlapTray(bool bInit)`
- **Task variable**: `int &Task = iSlapTrayTask`
- **Cases**:

`1, 5, 10, 20, 50, 100, 200, 300, 400, 450, 500`

- **Purpose**: 拍盤/整盤動作（AutoRetest 相關）。

## 3.6 DoCatchUnderTray

- **Signature**: `bool DoCatchUnderTray(int iWhichAuto)`
- **Task variable**: `int &Task = iCatchUnderTrayTask`
- **Cases**:

`1, 50, 100, 200, 300, 400, 500, 600, 700`

- **Purpose**: 下方 Conveyor 模式的夾盤流程。

## 3.7 DoSupportUnderTray

- **Signature**: `bool DoSupportUnderTray(int iWhichAuto)`
- **Task variable**: `int &Task = iSupportUnderTrayTask`
- **Cases**:

`1, 50, 100, 110, 200, 300, 400, 600`

- **Purpose**: 下方 Conveyor 模式的補盤流程。

## 3.8 DoPlaceBufferTray

- **Signature**: `bool DoPlaceBufferTray(int iWhichAuto)`
- **Task variable**: `int &Task = iPlaceBufferTrayTask`
- **Cases**:

`1, 50, 100, 200, 300, 400, 600, 1000`

- **Purpose**: 把 Tray 放到 Empty/Color buffer。

---

## 4. Child Switch Functions (Inside Next Layer)

## 4.1 C_CatchTray_Fix_Puch

- **Signature**: `bool C_CatchTray_Fix_Puch(bool bInitial)`
- **Task variable**: `int &iTask = iCatchTray_Fix_Puch`
- **Cases**: `1, 100`
- **Purpose**: CatchTray Fix 氣缸 Push 控制。

## 4.2 C_CatchTray_Fix_Pop

- **Signature**: `bool C_CatchTray_Fix_Pop(bool bInitial)`
- **Task variable**: `int &iTask = iCatchTray_Fix_Pop`
- **Cases**: `1, 100`
- **Purpose**: CatchTray Fix 氣缸 Pop 控制。

## 4.3 DoLoadCarRotArmReadRFID (Related)

- **Signature**: `bool DoLoadCarRotArmReadRFID(bool bAlarm)`
- **Task variable**: `int &Task = iCoverTrayIDTask[iKeyenceCoverTrayID_LoaderCar]`
- **Cases**: `1, 2, 3, 100, 500, 1000, 2000, 2500, 3000, 3100, 4000, 4100, 5000, 6000`
- **Purpose**: Loader Car 旋轉臂讀 RFID/NFC。

---

## 5. End-to-End Runtime Flow (Condensed)

```text
DoCatchTray
  -> Loader catch empty tray
  -> decide which auto needs tray
  -> catch new tray from buffer/auto2
  -> place tray to auto track
  -> optional place tray to buffer path
  -> mapping/check/remain cleanup
  -> loop
```

---

## 6. Notes

- `CatchTrayTask` 是整個 CatchTray 主流程控制軸。
- 子流程普遍使用 `Initial*Task()` 重置 task state，再進入對應 `Do*` state machine。
- 大量 case 含 `fall-through` 設計（例如 `case 1` 到 `case 10`），除錯時需注意。
- `TRAY_ARM_MODE`、`AUTO_EMPTY_COLOR`、`USE_AUTO_RETEST`、`AUTO3_IS_MAGAZINE` 會影響分支路徑。

<!-- preserved-content:end -->
