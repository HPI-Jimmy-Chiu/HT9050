# DoOutArm_9045() and DoOutArm_9045_XxY_Z() FlowChart

> Source: `aoutarm9045.cpp` / `aoutarm9045_XxY_Z.cpp`  
> Project: HT9011UC_Code_V3.33.897.0_20260306

---

## 1. DoOutArm_9045() - Dispatch Summary

`DoOutArm_9045()` is a dispatch function that routes to specific sub-functions based on `iInArmType`.

### Pre-Dispatch Guard Checks

1. **TestingNeedStopAllMotor** — `bEnableTestingNeedStopAllMotor && bI24TestingNeedStopAllMotor && bTestingStopAllMotor` → return
2. **bAlarmNeedServoOff && bMyServoOffOutArm** — servo off → return
3. **CheckOutArmDestroyActive()** — 確認 Destroy 吹氣完成（ASE_KaohSiung 客戶需開 bDevicConfirm）→ if any active → return
4. **QA Mode** — `bQAMode && rsmQAMode && bQAModeUseUnloadCnt` → `Check_QA_ModeUnloadCount()`
5. **iPauseBackUp != -1** — if paused & pick/destroy finish → return
6. **bResetOutArmTask** — call `InitOutArmTask()` → return
7. **Auto Alignment CCD** — `MACHINE_HAS_AUTO_ALIGNMENT_CCD && bEnableAutoAlignment`:
   - `bAutoNeedTrayMustFinish`:
     - CleanOut 且無 IC → 清旗標 → return
     - Fix Tray 必須有 Tray（Fix1~Fix6 依設定檢查）
     - `WhichAutoNeedTray()==0` → 執行 Auto Alignment → return
     - 否則 `IsCatchTrayReadySupplyNewTray()` → return
   - `bRunOutArmAutoAlignment` → return（正在 Alignment 中）

### iInArmType Dispatch Table

| iInArmType | Sub-function | Picker Config |
|---|---|---|
| `ep1Picker` | `DoOutArm_9045_All_1Picker()` | Single picker |
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoOutArm_9045_1x1_1()` | 1x1, 1 sucker |
| `e9045_1x2_2_14` / `e9045_1x2_2_13` | `DoOutArm_9045_1x2_2()` | 1x2, 2 suckers |
| `e9045_1x2_4_Hot` | `DoOutArm_9045_1x2_4()` | 1x2, 4 suckers |
| `e9045_1x3_2_14` | `DoOutArm_9045_1x3_2_14()` | 1x3, 2 suckers |
| `e9045_1x3_4` | `DoOutArm_9045_1x3_4()` | 1x3, 4 suckers |
| `e9045_1x4_2_14` | `DoOutArm_9045_1x4_2()` | 1x4, 2 suckers |
| `e9045_1x4_4_13` | `DoOutArm_9045_1x4_4S()` | 1x4, 4 suckers (S) |
| `e9045_1x4_4_Back` | `DoOutArm_9045_1x4_4()` | 1x4, 4 suckers (Back) |
| `e9045_1x4_4` | `DoOutArm_9045_1x4_4()` | 1x4, 4 suckers |
| `e9045_1x4_8_Hot` | `DoOutArm_9045_1x4_8()` | 1x4, 8 suckers |
| `e9045_2x1_2_13` | `DoOutArm_9045_2x1_2()` | 2x1, 2 suckers |
| `e9045_2x2_4_12/13/14` | `DoOutArm_9045_2x2_4()` | 2x2, 4 suckers |
| `e9045_2x2_8_Hot` | `DoOutArm_9045_2x2_8()` | 2x2, 8 suckers |
| `e9045_2x3_6_14` | `DoOutArm_9045_2x3_6_14()` | 2x3, 6 suckers |
| `e9045_2x3_6` | `DoOutArm_9045_2x3_6()` | 2x3, 6 suckers |
| `e9045_2x4_4_13/14` | `DoOutArm_9045_2x4_4()` | 2x4, 4 suckers |
| `e9045_2x4_8` | `DoOutArm_9045_2x4_8()` | 2x4, 8 suckers |
| `e9045_2x5_8` | `DoOutArm_9045_2x5_8()` | 2x5, 8 suckers |
| `e9045_2x6_8` | `DoOutArm_9045_2x6_8()` | 2x6, 8 suckers |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoOutArm_9045_2x8_8()` | 2x8, 8/32 suckers |

---

## 2. DoOutArm_9045_XxY_Z() - State Machine Summary

> Source: `aoutarm9045_XxY_Z.cpp`  
> Task variable: `int &Task = OutArmTask`  
> Return: void (state machine runs in main loop)

### Pre-Switch Guard

- `bCarryControlOutarm1 || bCarryControlOutarm2` → return（Carry 臂正在控制中）

### Main Process Flow

```
case 1  →  case 5    →  case 10   →  case 50
(Reset)    (Z Safe)     (Init Check) (Z Safe + Choose Shuttle)
                                          │
                                          ↓
                                     case 100 (Shuttle Select)
                           ┌── OldPos==1 ──┤── OldPos==0 ──┐
                           ↓                                ↓
                      case 1000                        case 2000
                      (Shuttle 1 Flow)                 (Shuttle 2 Flow)
                           │                                │
                     case 1140→1150→1100→1200         case 2040→2050→2100→2200
                     (Check→Move→Wait→Pick)           (Check→Move→Wait→Pick)
                           │                                │
                           └────────────┬───────────────────┘
                                        ↓
                                   case 3000 (Z Safe + IC Fall Down + Release Shuttle)
                                        │
                              ┌── Need Additional? ──┐
                              ↓                      ↓
                         case 7000              case 3010
                         (Rotator/AOI/FixAI)    (Search Tray)
                              │                      │
                              └──────┬───────────────┘
                                     ↓
                                case 3100→3300→3310
                                (Verify Tray → Place to Auto)
                                     │
                                case 3500
                                (After Place: Fix3/Magazine/CleanOut)
                                     │
                     ┌───── return ───┼───── return ───┐
                     ↓               ↓                 ↓
                 case 100         case 3010         case 5000
                 (next cycle)     (more IC)         (CleanOut Sort)
```

### State Descriptions

| State (case) | Name | Description |
|---|---|---|
| **1** | Reset | `bSortingAllBinTrayFinish = false` → fall through to case 5 |
| **5** | Z Safe | `MoveOutArmToAutoSafe()` → case 10 |
| **10** | Init Check | `CheckOutArmInitState()` → case 50 |
| **50** | Shuttle Decide | `FirstEnter = true`; `IsCatchTrayReadySupplyNewTray()`; Z Safe; 釋放 Shuttle 可回; 有 IC → case 3000, 無 → case 100 |
| **100** | Shuttle Select | 決定先取 SHT1 或 SHT2（32Site: SHT2 優先; 一般: SHT1 優先; ShuttleMode==1: 由設定決定）; STM/TW153: OldPos 交替 → case 1000 或 2000 |
| **200** | Yield Position | Z Safe → case 300（讓位等待 Shuttle 就位） |
| **300** | Yield Move | 32Site: 移到 SHT2 上; 否則: `MoveOutArmXY_ToFix_Tray_Full()` → `CheckOutArmCleanOut()` |
| **310** | Auto Z Teach | `AutoTeachLoadTrayZ()` → case 311 |
| **311** | Auto Z Move | `MoveOutArmXY_ToFix_Tray_Full()` → case 1 |
| **1000** | SHT1 Y Home | Y Pitch Home 判斷 → case 1140 或 1010-1030 |
| **1010-1030** | SHT1 Y Home Exec | `MoveOutArmToAutoSafe()` → Y Pitch Home → SetMotorSpeed → case 1140 |
| **1140** | SHT1 Pre-Check | `CheckOutArmToTask50()` → CleanOut check → 確認 `FRCarryKit` 有 IC → Shuttle 1 就位 → case 1200; 無 IC → case 1150 |
| **1150** | SHT1 Move XY | `OutArmSubSpeed()`; `MoveOutArmToShuttleIncludeZ_9045_2x8_8(SHT1)` → case 1100 |
| **1100** | SHT1 Wait Ready | `IsCatchTrayReadySupplyNewTray()`; `WhichAutoNeedTray()` 需換盤 → case 1160（讓位）; SHT1 有 IC → case 1200; SHT2 有 IC → case 50 |
| **1160** | SHT1 Yield | `MoveOutArmXY_ToFix_Tray_Full()` → case 1100 |
| **1200** | SHT1 Pick IC | `OutArmAddSpeed()`; `DoPickFromShuttle_9045_2x8_8(0)`; 同側還有 IC → loop; STM 模式 → 切 Kit → loop; 全吸完 → case 3000; 無 IC → case 50 |
| **1235** | SHT1 Manual Step | `bOutArmManualStepPress = false` → Z Safe → case 1150 |
| **2000** | SHT2 Y Home | Y Pitch Home 判斷 → case 2040 或 2010-2030 |
| **2010-2030** | SHT2 Y Home Exec | `MoveOutArmToAutoSafe()` → Y Pitch Home → SetMotorSpeed → case 2040 |
| **2040** | SHT2 Pre-Check | `CheckOutArmToTask50()` → CleanOut check → 確認 `BRCarryKit` 有 IC → Shuttle 2 就位 → case 2200; 無 IC → case 2050 |
| **2050** | SHT2 Move XY | `OutArmSubSpeed()`; `MoveOutArmToShuttleIncludeZ_9045_2x8_8(SHT2)` → case 2100 |
| **2100** | SHT2 Wait Ready | SHT2 有 IC → case 2200; SHT1 有 IC → case 50 |
| **2200** | SHT2 Pick IC | `OutArmAddSpeed()`; `DoPickFromShuttle_9045_2x8_8(1)`; 同側還有 IC → loop; STM 模式 → 切 Kit → loop; 全吸完 → case 3000; 無 IC → case 50 |
| **2235** | SHT2 Manual Step | `bOutArmManualStepPress = false` → Z Safe → case 2050 |
| **3000** | Post-Pick | `IsCatchTrayReadySupplyNewTray()`; Z Safe; `CheckOutArmSuckICFallDown()`; 釋放 Shuttle; Need Additional → case 7000; else → case 3010 |
| **3010** | Search Tray | `SearchTrayToPlace_9045()`; Magazine 忙碌 → 讓位等待; `VerifyFixTrayLink()` → 4000/10000; Tray arm 忙碌 → 讓位; `InitPlaceToAutoTask()` → case 3100 |
| **3020** | Tray Wait | `MoveOutArmXY_ToFix_Tray_Full()` → `fHasTray` → case 3100 |
| **3050** | Tray ReCheck | `AutoTrayReCheck()` → case 3100 |
| **3100** | Verify Tray | `VerifyTrayStatus()` → case 3300 |
| **3300** | Fix Sensor | Fix Tray sensor 偵測 → 無 Tray → case 4000; AutoTeach → case 3305; else → case 3310 |
| **3305** | Auto Alignment | `DoPlaceToAutoForAutoTeachOffset_9045_2x8_8()` → finish → case 100/3310 |
| **3310** | Place to Auto | `DoOutArmPlaceToAuto_9045()` → `DoOutArmAfterPlaceToAuto(init)` → case 3500 |
| **3500** | After Place | `DoOutArmAfterPlaceToAuto()` → return Task (3010/100/5000/11100/1) |
| **4000** | Fix Missing | `MoveOutArmXY_ToFix_Tray_Full()` → case 4100 |
| **4100** | Fix Full Alarm | `DoFixTrayFullAlarm()` → case 3010 |
| **7000** | Additional Func | `DoOutArmAdditionalFunction()` → case 3010 |
| **10000** | Magazine Check | `CheckPlaceToMagazineTray()` → case 12000 |
| **11100** | Magazine Buffer | `DoPickFromMagazineBuffer()` → `SearchTrayToPlace_9045()` → case 12000 |
| **12000** | Magazine Place | 等待 Magazine 就位 → `InitPlaceToAutoTask()` → case 3100 |

### Key Functions Called

| Function | Purpose |
|---|---|
| `MoveOutArmToAutoSafe()` | Z 軸上升到安全高度 |
| `CheckOutArmInitState()` | 檢查 OutArm 初始狀態 |
| `IsCatchTrayReadySupplyNewTray()` | 通知 Catch Tray Arm 是否可以補 Tray |
| `MoveOutArmToShuttleIncludeZ_9045_2x8_8()` | 移動 OutArm XY/Z 到 Shuttle 位置 |
| `DoPickFromShuttle_9045_2x8_8()` | 從 Shuttle 吸取 IC |
| `CheckOutArmSuckICFallDown()` | 掉料檢查（Z Safe 後） |
| `OutArmAddSpeed() / OutArmSubSpeed()` | Auto Speed 調整 |
| `SearchTrayToPlace_9045()` | 搜尋可放料的 Tray（Auto/Fix） |
| `VerifyFixTrayLink()` | Fix Tray 連動驗證 |
| `DoOutArmPlaceToAuto_9045()` | 放料到 Tray 的控制流程 |
| `DoOutArmAfterPlaceToAuto()` | 放料後處理（Fix3 整盤 / Magazine / CleanOut） |
| `DoOutArmAdditionalFunction()` | 附加功能（Rotator / AOI / Fix AI CCD） |
| `CheckOutArmCleanOut()` | CleanOut 判斷（整盤 / Magazine / Z Teach） |
| `CheckOutArmToTask50()` | 檢查是否需要回 case 50 重新判斷 |
| `MoveOutArmXY_ToFix_Tray_Full()` | 移到讓位點（X 軸極限 + SafeY） |

> **Note:** 所有 `DoOutArm_9045_*` sub-functions 共享相同的狀態機結構，
> 僅在 sucker 幾何處理與 Shuttle Mode 判斷上不同。

---

## 3. DoPickFromShuttle_9045_XxY_Z() - Process Flow

> Source: `aoutarm9045_XxY_Z.cpp` (line ~1333)  
> Task variable: `int &Task = iPickFromShuttle1Task` (SHT1) / `iPickFromShuttle2Task` (SHT2)  
> Return: `true` = pick complete, `false` = still in progress

### Summary

從 Shuttle 吸取 IC。先確認 Tester Z 安全，可選的 Shuttle 上等待時間後，
移動 XY/Z 到 Shuttle 位置，逐一 Suck() 吸取所有 IC，並進行 Shuttle → OutArm 資料交換。
異常時依 Retry 次數決定自動重試或顯示 Alarm（Retry / Skip / Home）。

### Process Flow

```
              [Pre-Switch]
              ptrOutSHT = (iSht==0) ? &FRCarryKit : &BRCarryKit
              iPickKit = iOutArmiWhichKit offset
              CheckTesterZ(iSht) → if Z not safe → return false
                    ↓
case 1     [Wait on Shuttle (optional)]
           dWaitOnSH != 0 && first pick (iOutArmiWhichKit==0 && NoIC)?
           ├─ YES → MoveOutArmToShuttleIncludeZ_9045_2x8_8(iSht, ZDown=false)
           │        PickFromShuttle(iOutArmiWhichKit) → case 2
           └─ NO  → case 10
              ↓
case 2     bOutShtwaitPick() timer → case 10
              ↓
case 10    iWitchErrBin = 0 → case 200 (fall through)
              ↓
case 200   [Move XY/Z to Shuttle]
           MoveOutArmToShuttleIncludeZ_9045_2x8_8(iSht, iOutArmiWhichKit, ZDown=true)
           ├─ Done: fAutoTeach→iATOutArmWhichKit = iOutArmiWhichKit
           │        OutArmNeedCheckOffset(false, iSht)?
           │        ├─ YES → case 500 (offset)
           │        └─ NO  → OutArmSuck.ResetAll() → case iOUTARM_SUCK
           └─ Not done → wait
              ↓
case 500   [Offset Processing]
           bEnterOffset?
           ├─ YES → bOutArmManualStepPress = true → Task=1 → return false
           └─ NO  → fAutoTeach→DoNext() → OutArmSuck.ResetAll() → case iOUTARM_SUCK
              ↓
case iOUTARM_SUCK
           [Suck All Suckers]
           Loop OutArmSuck[iPickRow × iPickCol]:
             Determine iShtCol via GetShuttleCol(i, j+iPickKit)
             ptrOutSHT→Item[iShtRow][iShtCol] has IC && Suck needs active?
             ├─ Suck[i][j].Suck() completed:
             │    SwapShuttleDataToOutArm(iSht, iShtRow, iShtCol, iSuckRow, iSuckCol)
             │    ← 將 Shuttle 上的 IC 資料搬到 OutArm
             │    bOutSuckShtDupErr[i][j] = false
             ├─ Suck error → flag = false (stay)
             └─ Not yet completed → flag = false (stay)

           bHasErr → ShowOutputShuttleDataMiss()

           flag == false → break (wait for all Suck to complete)

           Check for any Suck[i][j].Error:
           ├─ Has Error → iOutShtRetryCount++ → case 2000
           └─ No Error:
                Check all NULL_IC with NeedSuck → wait
                Error BinBox detect (if enabled)
                Fix AI CCD counter update (if enabled)
                iOutShtRetryCount = 0
                Task = 1
                return true ← 吸取完成
              ↓
case 2000  [Error Handling]
           MoveOutArmToAutoSafe()
           Collect error sucker names → ErrPart
           Check bOutSuckShtDupErr → bHasDuplicateErr

           bHasErr?
           ├─ iOutShtRetryCount > ArmSpeed[OutArm].iRetryCT:
           │    MoveOutArmXY_ToShuttleAlarmArea() → if failed → return false
           │    ret = OutArmPickShuttleAlarm(iSht, bHasDuplicateErr, ErrPart)
           │    ← 顯示 Alarm 等待操作員決定
           │    Record bOutSuckShtDupErr for duplicate detection
           └─ iOutShtRetryCount ≤ limit:
                ret = K_RETRY (auto retry)

           ret == K_SKIP:
             Loop: PorcessJAM0201OutArmPickUpErrorSkip() for each error sucker
             iOutShtRetryCount = 0
             ptrOutSHT side no IC?
             ├─ YES → clear DupErr → return true (skip done)
             └─ NO  → Task = 1 (re-pick remaining)

           ret == K_HOME:
             bInOutArmCanPushHome → case 2200 (Home)
             else → case 1 (retry)

           ret == K_RETRY:
             bInOutArmCanPushHome → case 1 (retry)
             else → case 2200 (Home via retry)
              ↓
case 2200  [Home]
           SetOutArmHome()
           iOutShtRetryCount = 0
           Task = 1
           return false
```

---

## 4. DoOutArmAdditionalFunction() - Process Flow

> Source: `aoutarm9045.cpp` (line ~2208)  
> Task variable: `int &Task = iOutArmAdditionalFunctionTask`  
> Return: `true` = all additional functions done, `false` = still in progress

### Summary

依序檢查並執行 Rotator、AOI、Fix AI CCD 三種附加功能。
每個子功能完成後回到 case 100 檢查是否還有下一個需要執行。

### Trigger Conditions (CheekNeedToDoOutArmAdditionalFunction)

| 功能 | 觸發條件 |
|---|---|
| **Rotator** | `USE_ROTATE_KIT==1 && tRotate.ActiveRotate && iOutRotateFinish==0 or 1`; 排除 `ART_RT_NoRotate` |
| **AOI** | `tAOISetup.bEnabledAOI` or `ScannerAOIIF.iEnableScannerMode!=0` or `TopScanner!=0` |
| **Fix AI CCD** | `USE_Fix_AI_CCD && bEnableFix2BGAAICCD && fFixAICCD→NeedToGrabImage()`; 需符合 TestMode 限制 |

### Process Flow

```
case 1     CheekNeedToDoOutArmAdditionalFunction()
           → case 100 (fall through)
              ↓
case 100   [Sequential Check & Dispatch]
           MoveOutArmToAutoSafe()
           CheckOutArmSuckICFallDown() ← 掉料檢查
           MOT[MInShuttle1/2].fCanMoveR = true ← 釋放 Shuttle
           ├─ bAlreadyRotate==false && bOutRotator
           │    InitialOutArmRotateKIT(), iOutRotateFinish=1 → case 10000
           ├─ bAlreadyAOI==false && bDoAOI
           │    InitAOIFunction() → case 20000
           ├─ bAlreadyFixAI==false && bDoFixAI
           │    fFixAICCD→Fix2AICCDFunction() → case 30000
           └─ All done → return true
              ↓
case 10000 [Rotator]
           DoOutArmRotateKIT() completed?
           ├─ YES → bAlreadyRotate=true, bOutRotator=false
           │        iOutRotateFinish=2 → case 100
           └─ NO  → stay
              ↓
case 20000 [AOI Inspection]
           DoAOIFunction() completed?
           ├─ YES → bAlreadyAOI=true, bDoAOI=false → case 100
           └─ NO  → stay
              ↓
case 30000 [Fix AI CCD]
           fFixAICCD→DoFix2AICCDFunction() completed?
           ├─ YES → bAlreadyFixAI=true, bDoFixAI=false → case 100
           └─ NO  → stay
```

### 與 InArm Additional Function 比較

| InArm | OutArm |
|---|---|
| Die Clean | (無) |
| Precisor | (無) |
| Bottom 2DID Scan | (無) |
| Rotator | Rotator |
| (無) | AOI Inspection |
| (無) | Fix AI CCD |

---

## 5. DoOutArmPlaceToAuto_9045() - Process Flow

> Source: `aoutarm9045.cpp` (line ~2935)  
> Task variable: `int &Task = iPlaceToAutoTask`  
> Return: `true` = place complete, `false` = still in progress

### Summary

控制 OutArm 放料到 Unloader Tray（Auto / Fix）的主流程。搜尋可放 IC 的 Tray 位置，
IC Fall Down 全時檢查，Fix3 Cylinder 控制，計算放料位置後進行 Offset 校正，
Destroy 延遲，最後呼叫 `DoOutArmPlaceToAuto()` 執行實際放料。

### Process Flow

```
case 1     [Init]
           bOutArmXOverLimit = false
           InitialFix3CanFullTask()
           → case 10 (fall through)
              ↓
case 10    [Search Tray & Setup]
           CheckOutSuckICFallDown(false)?
           ├─ IC dropped → case 220 (Z Safe & restart)
           └─ OK → continue

           iWhichAuto = SearchTrayToPlace_9045()   ← 搜尋放料 Tray
           IfUseOnebyOne(iWhichAuto)                ← 判斷是否需要逐一放
           Magazine mode check → ct = iWhichBuff or iWhichAuto
           Magazine busy → return false

           Tray has tray && not full?
           ├─ NO → InitOutArmTask() → break (Tray 已滿或無 Tray)
           └─ YES → continue

           Fix3 Cylinder → UseFix3Cylinder(ct)
           flag = SetOutArm_9045()   ← 計算放料 XY 位置、設定 bOutArmSuckActive
           ├─ flag true:
           │    fHasTray==false || FullIC → case 30 (wait)
           │    else → case 50
           └─ flag false:
                bOverTray → SetTray(HAS_IC) → return true
              ↓
case 30    [Wait Tray Available]
           IsCatchTrayReadySupplyNewTray()
           fHasTray && !FullIC → case 10
              ↓
case 50    [IC Fall Down Full Check]
           CheckOutSuckICFallDown(false)?
           ├─ OK → iRetry = 0 → case 100
           └─ Drop → case 220 (restart)
              ↓
case 100   [Offset Check & Delay]
           OutArmNeedCheckOffset(true, iWhichAuto)?
           ├─ YES → case 200 (offset mode)
           └─ NO:
                bSuckOnDown==false && dDestroyPauseTime!=0?
                ├─ YES → start DoPlaceToAutoDelay → case 110
                └─ NO  → case 300
              ↓
case 110   [Destroy Pause Delay]
           DoPlaceToAutoDelay timer off → case 300
              ↓
case 200   [Offset Enter/Exit]
           bEnterOffset?
           ├─ NO  → case 300 (continue)
           └─ YES → bEnterOffset=false → case 220
              ↓
case 220   [Offset Abort - Z Safe Restart]
           MoveOutArmToAutoSafe() → case 1
           OutArmSuck.HasIC()==false → return true (all IC dropped)
              ↓
case 300   [Place IC to Tray]
           Auto alignment mode → fix iWhichAuto = iFixRight
           DoOutArmPlaceToAuto(iWhichAuto)?
           ├─ Done → case 400
           └─ Not done → wait
              ↓
case 400   [Post-Place Processing]
           Rotate 1 Motor 1 Picker → MoveOutArmToAutoSafe()
           bNewAutoTrayDetect && Auto Tray?
           ├─ Tray Full → ReversionEmptyPoint() → return true
           └─ Not Full → MoveOutArmToAutoSafe() → case 500
           else → ReversionEmptyPoint() → return true
              ↓
case 500   [Auto Tray Detect]
           DetectAutoTray(iWhichAuto, &iRetry)?
           ├─ Done → ReversionEmptyPoint() → return true
           └─ Not done → wait
```

---

## 6. DoOutArmPlaceToAuto() - Process Flow

> Source: `aoutarm.cpp` (line ~2483)  
> No Task variable — runs as a single-pass loop per call  
> Return: `true` = all active suckers destroyed, `false` = still destroying

### Summary

實際執行 Destroy 吹氣放料。遍歷所有 `bOutArmSuckActive[i][j]` 的吸嘴，
對每個吸嘴呼叫 `Destroy()` 放料，完成後將 IC 測試資料寫入 Tray 並記錄 Production Log。

### Process Flow

```
[Init]
  iOutArmYStep = AutoCalculateOutArmYClosePitch / YPitch
  Motor = iMMAuto[iWhichAuto] (or iWhichBuff if Magazine)
        ↓
[Main Loop] for all OutArmSuck[iPickRow × iPickCol]:
  bOutArmSuckActive[i][j] == true?
  ├─ NO  → skip
  └─ YES:
       Item == HAS_NULL_IC?
       ├─ YES → SetItemData(NULL_IC) → skip (no Destroy needed)
       └─ NO  → Suck[i][j].Destroy()?
            ├─ Not done → flag = false (wait)
            └─ Done:
                 ┌─ [Data Write to Tray]
                 │  MOT[Motor].Tray.iWhichSite  = OutArmSuck.iWhichSite
                 │  MOT[Motor].Tray.iWhichIndex = OutArmSuck.iWhichIndex
                 │  MOT[Motor].Tray.iBinCode    = OutArmSuck.Item
                 │  MOT[Motor].Tray.iBinData    = OutArmSuck.iBinData
                 │  MOT[Motor].Tray.iAOIResult  = OutArmSuck.iAOIResult
                 │
                 ├─ [Lot Summary]
                 │  Item >= START_TEST → LotSummary.AddCount(Site, BIN)
                 │  MultiLotCnt > 1 → AddByLotCount()
                 │
                 ├─ [2D Mapping]
                 │  SortingBy2DList → sl2DMappingLog→MyInsert2DMappingToFile()
                 │
                 ├─ [Bin Count]
                 │  iByBinTotal[bin]++
                 │  LastSet.BinCT[k][iTo3Unload]++
                 │  LastSet.iBinData32[k][bin]++
                 │  SCK ART count (if enabled)
                 │
                 ├─ [Production Log]
                 │  PordRec.AddUnloadRecord(iWhichAuto, TrayCount, Row, Col, X, Y, OCR, Pitch...)
                 │  PordRec.SaveRecord()
                 │
                 ├─ [Tray Display]
                 │  Barcode → SetTrayBinData(HAS_IC / HAS_BARCODEERROR_IC, info)
                 │  BinBox → LastSet.iBinBoxCount++
                 │  Normal → SetTrayBinData(HAS_IC, info)
                 │
                 ├─ bOutArmCheckDestroyACT[i][j] = true  ← 回吸檢測
                 └─ bOutArmSuckActive[i][j] = false
                 └─ OutArmSuck.SetItemData(i, j, NULL_IC)
        ↓
  Suck[i][j].Error?
  ├─ YES → ErrPart += sName, bHasErr = true
  └─ NO  → clear DupErr
        ↓
[Post Loop]
  bHasErr → ShowErrorMessage("JAM0217", K_RETRY)
             ← Device drop error / Vacuum sensor OFF error
             Record PordRec error log
        ↓
  Any bOutArmSuckActive still true → return false (wait)
        ↓
  BinBox check → count >= alarm → SetTray(HAS_IC)
  TraySortCntFunc → tray IC count check → 達標則退盤
  fSortCT→CheckTheYieldAfterPlaceAuto() ← Yield 檢查
        ↓
  return true ← 全部放料完成
```

---

## 7. DoOutArmAfterPlaceToAuto() - Process Flow

> Source: `aoutarm9045.cpp` (line ~3139)  
> Task variable: `int &Task = iDoOutArmAfterPlaceToAutoTask`  
> Return: `int` (跳轉目標 case, 0 = not done)  
> Special: 呼叫方式 `bInitial=true` 時僅初始化不執行

### Summary

放料後的後處理流程。判斷吸嘴是否還有 IC（繼續放 or 清理流程），
處理 Fix3 整盤、Magazine Buffer、Clean Out 時的 Auto Sorting BinTray。

### Process Flow

```
[Init Call] bInitial == true:
  Task = 1, InitialFix3CanFullTask() → return 0
              ↓
case 1     [Check Remaining IC]
           OutArmSuck.HasRealIC()?
           ├─ YES (still has IC):
           │    Magazine Fix mode → return 3010 (continue place)
           │    Fix3 Cylinder → case 500 (retract first)
           │    else → return 3010 (continue place)
           └─ NO (all IC placed):
                ASE Report → SendDataToASE(AseIcRecord)
                BinBoxShiftY() ← BinBox Y 偏移

                ATK AMR mode?
                ├─ YES → Fix Tray has IC → InitialDoPickFromMagazineBuffer() → return 11100
                │        else → iUnloadFixTray = eAtkTfFeedFix → return 1
                └─ NO:
                     Magazine Buffer 需清?
                     ├─ YES → InitialDoPickFromMagazineBuffer() → return 11100
                     └─ NO:
                          Fix3 Full Tray enabled?
                          ├─ YES → setup SortingBinTray → case 1000
                          └─ NO  → InitialFix3CanFullTask() → case 2000
              ↓
case 500   [Fix3 Cylinder Retract]
           UseFix3Cylinder(0)?
           ├─ Done → return 3010 (continue place)
           └─ Not done → wait
              ↓
case 1000  [Fix3 Full Tray Sort]
           DoFix3FullTray() completed?
           ├─ Done → InitialFix3CanFullTask() → case 2000
           └─ Not done → wait
              ↓
case 2000  [Fix3 Cylinder Final]
           EnableFix3UseCylinder()?
           ├─ YES → UseFix3Cylinder(0)? → case 3000
           └─ NO  → case 3000 (fall through)
              ↓
case 3000  [CleanOut Check & Speed]
           CheckOutArmCleanOut(3100) → set Task
           bAutoSpeed → bCheckSpeed = true
              ↓
case 3100  [Complete]
           Task = 1
           return 100 ← 正常完成回到主狀態機 case 100
           tRotate.ActiveRotate && eInOutArm1Motor → return 110 (with rotate offset)
              ↓
case 5000  [CleanOut Sorting]
           bP27AutoSortingBinTrayByOutArmwhenCleanOut && bSortingBinTraywhenCleanOut
           && !bSortingAllBinTrayFinish?
           ├─ YES → DoSortingBinTray(0) → case 5100
           └─ NO:
                MoveOutArmXY_ToFix_Tray_Full()
                iCleanOut==1 && (InArm/OutArm/Shuttle/Index has IC)?
                ├─ YES → return 1 (wait for others to finish)
                └─ NO  → wait
              ↓
case 5100  [BinTray Sort Execute]
           DoSortingBinTray() completed?
           ├─ Done && bSortingAllBinTrayFinish → case 5000 (re-check)
           └─ Not done → wait
```

### Return Value Reference

| Return | Meaning | Main SM Jump |
|---|---|---|
| 0 | Not done | Stay at case 3500 |
| 1 | CleanOut complete / idle | case 1 |
| 100 | Normal complete | case 100 |
| 110 | Complete + Rotate | case 100 + offset |
| 3010 | Sucker still has IC | case 3010 (continue place) |
| 5000 | CleanOut + Sort entry | case 5000 (from case 300 init) |
| 11100 | Magazine Buffer | case 11100 |

---

## 8. CheckOutArmCleanOut() - Process Flow

> Source: `aoutarm.cpp` (line ~215)  
> Return: `int` (Task 跳轉目標)

### Summary

Clean Out 判斷邏輯，在讓位等待（case 300）和放料後處理（case 3000）中被呼叫。

### Process Flow

```
[Input] Task: 預設目標 case（若不改變流程則回傳此值）
              ↓
  [Check 1: P27 Auto Sorting]
  iCleanOut==1 && bP27AutoSortingBinTrayByOutArmwhenCleanOut
  && bSortingBinTraywhenCleanOut?
  ├─ YES → InArm/OutArm/Shuttle/Index all no IC
  │        && IsInArmCleanOutFinish()?
  │        ├─ YES → return 5000 ← 進入 BinTray 整盤
  │        └─ NO  → continue
  └─ NO → continue
              ↓
  [Check 2: Magazine Buffer]
  iCleanOut==1 && AUTO3_IS_MAGAZINE && iMagFixTrayType==1?
  ├─ YES → all no IC && IsInArmCleanOutFinish()?
  │        ├─ YES → InitialDoPickFromMagazineBuffer()
  │        │        CheckMagazineBufferNeedClear()
  │        │        → return 11100
  │        └─ NO  → continue
  └─ NO → continue
              ↓
  [Check 3: ATK AMR]
  fAGV→IsATK_AMR() && eAtkTfMoveFixIC?
  ├─ YES → all no IC?
  │        SearchTrayToPick_Buffer() != -1?
  │        ├─ YES → InitialDoPickFromMagazineBuffer()
  │        │        CheckMagazineBufferNeedClear()
  │        │        → return 11100
  │        └─ NO  → continue
  └─ NO → continue
              ↓
  [Check 4: Auto Z Teach]
  bOutarmAutoHigh?
  ├─ YES → AutoTeachLoadTrayZ(true, OutArm) → return 310
  └─ NO  → return Task ← 不改變流程
```
