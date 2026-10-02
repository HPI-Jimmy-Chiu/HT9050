# OutArm — 取料段（Shuttle Pick）詳細參考

> 摘自 §1–§5 取料流程。主 Skill 見 [ht9045-outarm-flow/SKILL.md](../SKILL.md)。

## 1. 呼叫階層總覽

```
DoOutArm()                              <- aoutarm.cpp（最上層入口）
  └─ DoOutArm_9045()                    <- aoutarm9045.cpp（Dispatch）
       └─ DoOutArm_9045_2x8_8()         <- aoutarm9045_2x8_8.cpp（主狀態機）
            ├─ case 1200/2200: DoPickFromShuttle_9045_2x8_8(iSht)
            │     ├─ case 200: MoveOutArmToShuttleIncludeZ (XY/Z 定位)
            │     ├─ case iOUTARM_SUCK: 逐一 Suck() 吸取 IC
            │     └─ case 2000: Error Retry/Skip/Home
            ├─ case 7000: DoOutArmAdditionalFunction()
            ├─ case 3010: SearchTrayToPlace_9045()
            ├─ case 3310: DoOutArmPlaceToAuto_9045()
            └─ case 3500: DoOutArmAfterPlaceToAuto()
```

## 1.1 OutArm 吸嘴結構與 X 軸硬體設定

OutArm 使用的吸嘴結構 `OutArmSuck`（`TMyKitSuck` 類型）與 InArm 相同，同樣支援多種吸嘴配置。根據機台設定檔 `Gerneral.ini` 的硬體配置：

| 設定項 | 設定值 | 對應吸嘴 | 物理距離 | 備註 |
|--------|--------|---------|---------|------|
| `USE_IN_OUT_ARM_X_PITCH` | 0 (iXPitch60) | Aa → Ad（Column direction） | 4000 ~ 12000（0.1um 單位） | **固定值**，由硬體決定 |
| `IN_OUT_ARM_X_PITCH_MIN` | 4000 | — | 400mm（最小夾爪寬度） | — |
| `IN_OUT_ARM_X_PITCH_MAX` | 12000 | — | 1200mm（最大夾爪寬度） | — |

**說明**：
- 當 `USE_IN_OUT_ARM_X_PITCH=0` 時，OutArm 吸嘴的 X 方向間距採用硬體預設值 **4000 ~ 12000**（0.1um 單位）
- 此值定義了吸嘴組 (Aa/Ac/Ab/Ad) 在夾爪上的最小與最大展開範圍
- 每組吸嘴間距相差一個 Sucker Pitch，排列根據 `iOutArmXBase` 決定基準位置
- `iPickStep` 決定每次吸取時使用的吸嘴數量（若 `iPickStep=2`，則只使用 Aa、Ac；若 `iPickStep=1`，則連續使用 Aa、Ab、Ac、Ad）
- 此設定與 InArm 共用相同的硬體規格（詳見 [ht9045-inarm-flow SKILL](../../ht9045-inarm-flow/references/InArm_TMyKitSuck_and_TypeDecision.md#211-x-軸吸嘴間距硬體設定)）

## 2. DoOutArm() — 入口

> Source: `aoutarm.cpp` L1117-1135

1. `FRCarryKit.SetHasNullIcToNullIc()` / `BRCarryKit.SetHasNullIcToNullIc()` — 清理 NULL IC 標記
2. 將 Shuttle 上的 `HAS_IC` / `HAS_HOT_IC` 轉換為 `TEST_PASS + iTestBinCount`
3. `SetFixTrayMiddleDtata()` — 設定 Fix Tray 中間資料
4. 呼叫 `DoOutArm_9045()` — 進入 Dispatch

> 與 InArm 不同：DoOutArm() 沒有複雜 Guard Checks，Guard 邏輯全在 DoOutArm_9045()。

## 3. DoOutArm_9045() — Dispatch

> Source: `aoutarm9045.cpp` L~403-620

### Pre-Dispatch Guard Checks

1. `TestingNeedStopAllMotor` → return
2. `bAlarmNeedServoOff && bMyServoOffOutArm` → return
3. `CheckOutArmDestroyActive()` → 確認 Destroy 吹氣完成
4. `QA Mode` — `Check_QA_ModeUnloadCount()`
5. `iPauseBackUp != -1 && Suck/Destroy 完成` → return
6. `bResetOutArmTask` → `InitOutArmTask()` → return
7. **Auto Alignment CCD** — Tray 補料確認與自動對位

### iInArmType Dispatch Table

| iInArmType | Sub-function | Picker Config |
|---|---|---|
| `ep1Picker` | `DoOutArm_9045_All_1Picker()` | Single picker |
| `e9045_1x1_1` | `DoOutArm_9045_1x1_1()` | 1x1, 1 sucker |
| `e9045_1x2_2_14/13` | `DoOutArm_9045_1x2_2()` | 1x2, 2 suckers |
| `e9045_1x2_4_Hot` | `DoOutArm_9045_1x2_4()` | 1x2, 4 suckers |
| `e9045_1x3_2_14` | `DoOutArm_9045_1x3_2_14()` | 1x3, 2 suckers |
| `e9045_1x3_4` | `DoOutArm_9045_1x3_4()` | 1x3, 4 suckers |
| `e9045_1x4_2_14` | `DoOutArm_9045_1x4_2()` | 1x4, 2 suckers |
| `e9045_1x4_4_13` | `DoOutArm_9045_1x4_4S()` | 1x4, 4 suckers (S) |
| `e9045_1x4_4` | `DoOutArm_9045_1x4_4()` | 1x4, 4 suckers |
| `e9045_2x2_4_12/13/14` | `DoOutArm_9045_2x2_4()` | 2x2, 4 suckers |
| `e9045_2x4_8` | `DoOutArm_9045_2x4_8()` | 2x4, 8 suckers |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoOutArm_9045_2x8_8()` | 2x8, 8/32 suckers |

## 4. 主狀態機 — DoOutArm_9045_2x8_8()

> Source: `aoutarm9045_2x8_8.cpp` L~1766 | Task: `int &OutArmTask`

### Shuttle 1 Pick Flow

```
case 1000 → case 1010~1030 (Y Pitch Home)
          → case 1140 (Check CleanOut / Has IC / Shuttle Ready)
          → case 1150 (Move XY to Shuttle 1)
          → case 1100 (Wait Shuttle Ready)
          → case 1200 (DoPickFromShuttle_9045_2x8_8(0))
               ├─ Still has IC → loop 1200
               ├─ STM/TW153 mode → switch kit side
               ├─ No IC → case 50
               └─ Done → case 3000
```

### Shuttle 2 Pick Flow

```
case 2000 → case 2040 (Check)
          → case 2050 (Move XY to Shuttle 2)
          → case 2100 (Wait Ready)
          → case 2200 (DoPickFromShuttle_9045_2x8_8(1))
               ├─ Still has IC → loop 2200
               └─ Done → case 3000
```

### State Descriptions（取料相關）

| case | Name | Description |
|---|---|---|
| **1** | Reset | 重置 SortingAllBinTrayFinish 旗標 |
| **5** | Z Safe | `MoveOutArmToAutoSafe()` |
| **10** | Init Check | `CheckOutArmInitState()` |
| **50** | Shuttle Decision | Z Safe → 釋放 Shuttle → 有 IC → 3000；無 → 100 |
| **100** | Shuttle Select | 32Site: SHT2 優先；一般: SHT1 優先 |
| **1140** | SHT1 Pre-Check | CleanOut 檢查、確認 FRCarryKit 有 IC、Shuttle 1 就位 |
| **1150** | SHT1 Move XY | 移到 Shuttle 1 上方，降速 |
| **1100** | SHT1 Wait Ready | 等 Shuttle 就位 / Tray 換盤讓路 |
| **1200** | SHT1 Pick IC | `DoPickFromShuttle_9045_2x8_8(0)` |
| **2040** | SHT2 Pre-Check | CleanOut 檢查、確認 BRCarryKit 有 IC |
| **2050** | SHT2 Move XY | 移到 Shuttle 2 上方，降速 |
| **2100** | SHT2 Wait Ready | 等 Shuttle 就位 |
| **2200** | SHT2 Pick IC | `DoPickFromShuttle_9045_2x8_8(1)` |
| **200** | Yield Position | Z Safe 後移到讓位位置 |
| **300** | Yield Move | 32Site: 移到 Shuttle 2 上等待 |

## 5. DoPickFromShuttle_9045_2x8_8() — Shuttle 取料

> Source: `aoutarm9045_2x8_8.cpp` L~1333
> Task: `iPickFromShuttle1Task` (SHT1) / `iPickFromShuttle2Task` (SHT2)

### 關鍵 Case

| case | 動作 |
|---|---|
| 1 | 若設定 `dWaitOnSH != 0` 且首次吸取 → 移到 Shuttle 上等待 → case 2 |
| 2 | 等待時間到（`bOutShtwaitPick`）→ case 10 |
| 10 | 初始化 → case 200 |
| 200 | `MoveOutArmToShuttleIncludeZ_9045_2x8_8()` 移動 XY/Z → case iOUTARM_SUCK |
| iOUTARM_SUCK | 逐一吸取所有 Sucker → `SwapShuttleDataToOutArm()` → 全部完成無 Error: return true |
| 2000 | Error：Z Safe → Retry（未超限）或 `OutArmPickShuttleAlarm()` alarm |
| 2200 | Home：`SetOutArmHome()` → 重置 |

### 吸取邏輯（case iOUTARM_SUCK）

1. 遍歷所有 `OutArmSuck.Suck[i][j]`
2. 每個需要吸取的 sucker 呼叫 `Suck()`
3. 吸取成功 → `SwapShuttleDataToOutArm()` 搬 IC 資料
4. 有任一 `Error` → case 2000 Retry 流程
5. 全部完成無錯誤 → `iOutShtRetryCount = 0` → return true

### Error 處理（case 2000）

- `iOutShtRetryCount <= RetryCT` → 自動 Retry（回 case 1）
- 超限 → Alarm：
  - `K_SKIP` → `PorcessJAM0201OutArmPickUpErrorSkip()` — 跳過 Error IC
  - `K_HOME` → case 2200 → `SetOutArmHome()`
  - `K_RETRY` → 回 case 1 重試
