# DoInArm_9045() and DoInArm_9045_XxY_Z() FlowChart

> Source: `ainarm9045.cpp` / `ainarm9045_XxY_Z.cpp`  
> Project: HT9011UC_Code_V3.33.897.0_20260306

---

## 1. DoInArm_9045() - Dispatch Summary

`DoInArm_9045()` is a dispatch function that routes to specific sub-functions based on `iInArmType`.

### Pre-Dispatch Guard Checks

1. **AUTO_ALIGNMENT_CCD** — if auto-alignment running → return
2. **AutoSiteMappingUseHotPlate** — set `bRunAutoSiteMapping` flag
3. **TestingNeedStopAllMotor** — call `StopAllMotor()` → return
4. **bAlarmNeedServoOff && bMyServoOffInArm** — servo off → return
5. **Init ASET_StartTimeNAME** — initialize start time if empty
6. **CheckInArmDestroyActive()** — if any destroy still active → return
7. **iPauseBackUp != -1** — if paused & pick/destroy finish → return
8. **bResetInArmTask** — call `InitInArmTask()` → return

### iInArmType Dispatch Table

| iInArmType | Sub-function | Picker Config |
|---|---|---|
| `ep1Picker` | `DoInArm_9045_All_1Pick()` | Single picker |
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoInArm_9045_1x1_1()` | 1x1, 1 sucker |
| `e9045_1x2_2_13` | `DoInArm_9045_1x2_2()` | 1x2, 2 suckers |
| `e9045_1x2_2_14` | `DoInArm_9045_1x2_2_14()` | 1x2, 2 suckers (1-4) |
| `e9045_1x2_4_Hot` | `DoInArm_9045_1x2_4_Hot()` | 1x2, 4 suckers (Hot) |
| `e9045_1x3_2_14` | `DoInArm_9045_1x3_2_14()` | 1x3, 2 suckers (1-4) |
| `e9045_1x3_4` | `DoInArm_9045_1x3_4()` | 1x3, 4 suckers |
| `e9045_1x4_4_13` | `DoInArm_9045S_1x4_4()` | 1x4, 4 suckers (S) |
| `e9045_1x4_2_14` | `DoInArm_9045_1x4_2()` | 1x4, 2 suckers |
| `e9045_1x4_4_Back` | `DoInArm_9045_1x4_4_Back()` | 1x4, 4 suckers (Back) |
| `e9045_1x4_4` | `DoInArm_9045_1x4_4()` | 1x4, 4 suckers |
| `e9045_1x4_8_Hot` | `DoInArm_9045_1x4_8_Hot()` | 1x4, 8 suckers (Hot) |
| `e9045_2x1_2_13` | `DoInArm_9045_2x1_2()` | 2x1, 2 suckers |
| `e9045_2x2_4_12` | `DoInArm_9045_2x2_4_12()` | 2x2, 4 suckers (1-2) |
| `e9045_2x2_4_13` | `DoInArm_9045_2x2_4()` | 2x2, 4 suckers |
| `e9045_2x2_4_14` | `DoInArm_9045_2x2_4_14()` | 2x2, 4 suckers (1-4) |
| `e9045_2x2_8_Hot` | `DoInArm_9045_2x2_8_Hot()` | 2x2, 8 suckers (Hot) |
| `e9045_2x3_6_14` | `DoInArm_9045_2x3_6_14()` | 2x3, 6 suckers (1-4) |
| `e9045_2x3_6` | `DoInArm_9045_2x3_6()` | 2x3, 6 suckers |
| `e9045_2x4_4_13` | `DoInArm_9045_2x4_4_13()` | 2x4, 4 suckers (1-3) |
| `e9045_2x4_4_14` | `DoInArm_9045_2x4_4()` | 2x4, 4 suckers |
| `e9045_2x4_8` | `DoInArm_9045_2x4_8()` | 2x4, 8 suckers |
| `e9045_2x5_8` | `DoInArm_9045_2x5_8()` | 2x5, 8 suckers |
| `e9045_2x6_8` | `DoInArm_9045_2x6_8()` | 2x6, 8 suckers |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoInArm_9045_2x8_8()` | 2x8, 8/32 suckers |

---

## 2. DoInArm_9045_XxY_Z() - State Machine Summary

> Source: `ainarm9045_XxY_Z.cpp`  
> Task variable: `int &Task = iArmTask`  
> Return: `true` = cycle complete, `false` = still in progress (called repeatedly)

### Pre-Switch Guard

- `DoInArmAutoSiteMapping()` — if auto site mapping active or `bIndexAlarmInArmAway` → return
- Init local variables: `iSht`, `iKit`, `iShtHP`, `iKitHP`

### Main Process Flow (Ambient Mode: Loader → Shuttle)

```
case 1  →  case 10  →  case 50  →  case 75  →  case 100
(Wait)     (Decide)    (Prepare)   (HP Check)   (Pick from Loader)
                                                       ↓
case 2000  ←  case 400  ←  case 200  ←──────────────────
(Place to     (Additional   (Close Site
 Shuttle)      Functions)    Check)
```

### Main Process Flow (Hot Mode: Loader → HotPlate → Shuttle)

```
case 1  →  case 10  →  case 50  →  case 75  →  case 100
(Wait)     (Decide)    (Prepare)   (HP Check)   (Pick from Loader)
                                                       ↓
                                          case 200  →  case 400
                                          (Close Site) (Additional)
                                                          ↓
              case 1500/1550  ←  case 1100  ←  case 1000
              (Pick from HP)     (Place to HP)  (Pre-Place HP)
                    ↓
              case 2000
              (Place to Shuttle)  →  case 50 (loop)
```

### HotPlate Verification Flow

```
case 75  →  case 15000
(HP Check)  (TryPick from HP Verification)
                  ↓
            case 2000 (if has IC)
            case 100  (if no IC, normal)
            case 75   (if no IC, need re-check)
```

### State Descriptions

| State (case) | Name | Description |
|---|---|---|
| **1** | Wait | Move InArm XY to wait position; check OneCycle/CleanOut |
| **10** | Decision Hub | Check IC status on suckers; route to appropriate next state |
| **50** | Prepare Pick | Check Heater/Tray/FIFO/CleanOut conditions; prepare loader pick |
| **75** | HP Pre-Check | Check if HotPlate needs TrySuck verification or normal pick |
| **100** | Pick from Loader | Execute `DoInArmPickFromLoadStage_9045()` |
| **200** | Close Site Check | Verify closed sites don't hold IC |
| **300** | Close Site Error | Handle alarm JAM0114 for closed site with IC |
| **400** | Additional Functions | Execute `DoInArmAdditionalFunction()` (Precisor / Rotator / Bottom CCD / Die Clean) |
| **500** | HP Wait/Pick Ready | Check HotPlate for ready IC; dispatch to pick or wait |
| **600** | Z Safe Move | Move Z axis to plate safe position |
| **1000** | Pre-Place to HP | Check if can skip HotPlate and go directly to shuttle |
| **1100** | Place to HotPlate | Execute `DoPlaceToHotPlate_9045()` |
| **1500/1550** | Pick from HotPlate | Execute `DoInArmPickFromHotPlate_9045()` |
| **1600** | Post-HP Pick Error | Handle close site error after HotPlate pick |
| **2000** | Place to Shuttle | Execute `DoInArmPlaceToShuttle_9045()` |
| **15000** | TryPick HP Verify | Execute `DoInArmTryPickFromHotPlate_9045_2x8_8()` for HP verification |

### Key Functions Called

| Function | Purpose |
|---|---|
| `MoveInArm2XYToWait()` | Move InArm to safe wait position |
| `CheckInArmSuckInitial()` | Verify sucker initialization status |
| `DoInArmPickFromLoadStage_9045()` | Pick IC from loader tray |
| `DoInArmAdditionalFunction()` | Precisor / Rotator / Bottom CCD / Die Clean |
| `DoPlaceToHotPlate_9045()` | Place IC onto hot plate for soaking |
| `DoInArmPickFromHotPlate_9045()` | Pick soaked IC from hot plate |
| `DoInArmPlaceToShuttle_9045()` | Place IC onto shuttle for testing |
| `DoInArmTryPickFromHotPlate_9045_2x8_8()` | Verify HotPlate IC placement by try-suck |
| `DoInArmAutoSiteMapping()` | Auto site mapping process |
| `CheekNeedToDoInArmAdditionalFunction()` | Check if additional processing needed |
| `CheckPlaceToShuttle()` | Check if can bypass HP and go to shuttle |
| `HasHotReadyIC_9045()` | Check HotPlate has soak-complete IC |

> **Note:** All `DoInArm_9045_*` sub-functions share this same state machine structure  
> (case 1/10/50/75/100/200/300/400/500/600/1000/1100/1500/1550/1600/2000/15000),  
> differing only in sucker geometry handling and specific HP check functions.

---

## 3. DoInArmPickFromLoadStage_9045() - Process Flow

> Source: `ainarm9045.cpp` (line ~7000)  
> Task variable: `int &Task = iPickFromLoadStageTask`  
> Return: `true` = pick cycle complete, `false` = still in progress

### Summary

從 Loader Tray 吸取 IC。先確認 Z 軸安全與 Tray 是否有 IC,搜尋 XY 位置後 Z 下降吸取,
透過真空驗證吸取結果,異常時 Retry / Skip / Home / Tray End / Clean Out。

### Process Flow

```
case 1   Check Tray arm safe position (encoder < iXTraySafty → wait)
         Auto Teach Load Tray Z (ASE customer → case 5)
         MoveInArmZToPlateSafeAndCheckLoaderTray()
         OneCycle check → return true if done
         DoPickLoaderOK() → return true if already picked
         Check Rotate → case 9 (rotate to 0°) or → case 10
              ↓
case 5   AutoTeachLoadTrayZ() → when done → case 1
              ↓
case 9   MoveInRotateToDegreeAtSameTime(0) → case 10
              ↓
case 10  [Decision] Loader Tray has IC?
         ├─ YES: CheckLoaderHasTray() → case 20 (Tray fixer)
         │       OneCycle check → return true
         │       NeedPickupErrorICToRecycleBin → stay case 10
         │       → case 12 (search XY)
         └─ NO:  OneCycle → return true
                  Move to wait/shuttle2 → case 15
              ↓
case 12  AutoSiteMapping check → Tray check
         ArmFinishForLoader() → case 2100 (finish)
         SearchAndMoveInArmXYToLoad_9045() → case 200 (Z down)
              ↓
case 15  Wait for Tray sensor clear
         DoAutoSkipCheck() → SetInArmUseSuckToHasNullIC → return true
              ↓
case 20  MoveInArm2XYToWait() → case 30
              ↓
case 30  C_TrayY_Fixer Push/On → case 39/40
              ↓
case 39  Fixer delay wait → case 40
              ↓
case 40  CheckLoaderHasTray(retry) → C_LoaderEdgePush On → case 10
              ↓
case 200 [Z Down Pick]
         MoveInArmZToLoaderPick(iRetryCT)
         InArmNeedCheckOffset → case 220 (offset mode)
         Calculate Y pitch step
         → case 1000 (suck all)
              ↓
case 220 Offset enter/exit → case 200 or case 1
              ↓
case 1000 [Suck All Suckers]
          Loop all InArmSuck[row][col]:
            If NULL_IC & used → Suck[i][j].Suck()
            On error → set bSuckEnd, record error position
          All suck done?
          ├─ Has Error + Retry ≤ limit → case 1010 (retry)
          ├─ Has Error + Retry > limit → case 1050 (error handling)
          ├─ SCK ART loading count alarm → case 4000
          └─ All OK → case 2000 (Z safe & finish)
              ↓
case 1010 MoveInArmZToPlateSafeAndCheckLoaderTray()
          E56 RetryAtSamePosition → case 1020
          else → bInArmPickErrFromLoader, case 10
              ↓
case 1020 MoveInArmZToLoaderPick(retry) → case 1000 (re-suck)
              ↓
case 1050 MoveInArmZToPlateSafeAndCheckLoaderTray()
          bPickFromLoader = false → case 1100
              ↓
case 1100 [Pick Error Alarm]
          Collect error sucker names & tray positions
          MoveInArm2XYToWait (if E67 option)
          ProcessMES0101InArmPickLoaderError()
          ├─ K_TRAY_END   → DoTrayEndProcess_9045()
          ├─ K_CLEAN_OUT  → CleanOut, return true
          ├─ K_SKIP       → DoTraySkipProcess_9045() → case 1400
          ├─ K_HOME       → ResetAll → case 1200
          └─ K_RETRY      → ResetAll → case 1200 or 1400
          Normal complete → case 2000
              ↓
case 1200 MoveInArmZToPlateSafe → case 1300
              ↓
case 1300 SetInArmHome() → case 10
              ↓
case 1400 MoveInArmZToPlateSafe → case 10
              ↓
case 2000 [Post-Pick Verify]
          MoveInArmZToPlateSafeAndCheckLoaderTray()
          CheckInArmSuckFromLoaderICFallDown()
          ├─ IC dropped → case 1010 or case 1 (retry/restart)
          └─ OK → case 2100
              ↓
case 2100 [Finish]
          ArmFinishForLoader() → SetInArmUseSuckToHasNullIC → return true
          else FullPick / OneCycle check → case 10 or return true
              ↓
case 4000 ProcessSCKARTLoadingCount() → case 10
```

---

## 4. DoInArmAdditionalFunction() - Process Flow

> Source: `ainarm9045.cpp` (line ~3182)  
> Task variable: `int &Task = iInArmAdditionalFunctionTask`  
> Return: `true` = all additional functions done, `false` = still in progress

### Summary

依序檢查並執行 Die Clean、Precisor、Bottom 2DID、Rotator 四種附加功能。
每個子功能完成後回到 case 100 檢查是否還有下一個需要執行。

### Process Flow

```
case 1     CheekNeedToDoInArmAdditionalFunction()
           → case 100 (fall through)
              ↓
case 100   [Sequential Check & Dispatch]
           ├─ bAlreadyDieClean==false && bDieClean  → InitInDieCleanTask() → case 10000
           ├─ bAlreadyPreciser==false && bPrecisor  → InitInArmDevicePosPrecise() → case 20000
           ├─ bAlready2DID==false && bBottom2DID    → InitBottom2DIDScan() → case 30000
           ├─ bAlreadyRotate==false && bInRotator   → case 40000
           └─ All done → return true
              ↓
case 10000 [Die Clean]
           DoInDieClean() completed?
           ├─ YES → bAlreadyDieClean=true, bDieClean=false → case 100
           └─ NO  → stay
              ↓
case 20000 [Precisor]
           DoInArmDevicePosPrecise() completed?
           ├─ YES → bAlreadyPreciser=true, bPrecisor=false → case 100
           └─ NO  → stay
              ↓
case 30000 [Bottom 2DID Scan]
           BOTTOM_2DID_CCD==1 → DoBottom2DID_8CCD_Scan()
           else               → DoBottom2DIDScan()
           completed?
           ├─ YES → bAlready2DID=true, bBottom2DID=false → case 100
           └─ NO  → stay
              ↓
case 40000 [Rotator - Z Safe]
           MoveInArmZToPlateSafe() → iInRotateFinish=1 → case 41000
              ↓
case 41000 [Rotator - Rotate]
           DoInArmRotateKIT() completed?
           ├─ YES → iInRotateFinish=2, bAlreadyRotate=true → case 100
           └─ NO  → stay
```

---

## 5. DoPlaceToHotPlate_9045_XxY_Z() - Process Flow

> Source: `ainarm9045_XxY_Z.cpp` (line ~1733)  
> Task variable: `int &Task = iInArmPlaceToHotPlateTask`  
> Return: `true` = place complete, `false` = still in progress

### Summary

將 InArm 上的 IC 放置到 HotPlate 進行 Soak（浸泡加熱）。搜尋可用 HP 位置後移動 XY,
依序對每個 Sucker 執行 Destroy（吹氣放料），放完後 Z 上升檢查是否還有剩餘 IC，
若有則重複放料（分段式），最後驗證是否有黏貨。

### Process Flow

```
case 1     GetVariableXInHotPlateData(iPlaceHP)
           SearchPlateToPlace()
           bPlaceToHotplatePartOK = false
           → case 100 (fall through)
              ↓
case 100   [Move XY to HotPlate]
           MoveInArmXYToHotPlatePlace(iPlaceHP)
           ├─ Done → case 200
           └─ IC fall down detected → case 110 (recheck all suckers)
              ↓
case 110   CheckInArmSuckICFallDownToHasNullIC() (full recheck)
           → case 100
              ↓
case 200   [Offset & Release Delay]
           InArmNeedCheckHotPlateOffset → case 210 (offset mode)
           Update HPSuckGroup record
           EnableReleaseDelay==0 → set delay timer → case 340
           else → case 350
              ↓
case 210   Offset enter → case 220 / case 100
              ↓
case 220   MoveInArmZToPlateSafe → case 100
              ↓
case 340   [Release Delay]
           InArmReleaseDelayToHot timer off → case 350
              ↓
case 350   [Destroy (Blow) IC to HotPlate]
           Loop InArmSuck[2 rows][iStepHP cols]:
             Row2 place check (iPlaceHPOrder)
             For each sucker with HAS_IC:
               Suck.Destroy() → DoPlaceToHPSwapData() (data exchange)
               HAS_NULL_IC → DoPlaceToHPBackupData()
           All destroy done → bPlaceToHotplatePartOK = true → case 400
              ↓
case 400   [Z Safe & Check Remaining]
           MoveInArmZToPlateSafe()
           InArm still HasIC()?
           ├─ YES (XDivision==6 special check)
           │   → case 1 (loop: pick next HP position & place again)
           │     iHotCount++
           └─ NO → case 500
              ↓
case 500   [Verify No Sticky IC]
           CheckInArmDestroyICFail()
           → case 501
              ↓
case 501   [Post-Place Cleanup]
           AdjustShuttleWhichKitOrder()
           iHotCount++
           SearchPlateToPlace() (if hot & has space)
           PickFromHPList->AddHPSuckGroup()
           USE_LASER_DISTANCE → case 600 (laser check)
           else → return true
              ↓
case 600   [Laser Floating Check]
           CheckInArmFloating() → return true
```

---

## 6. DoInArmPickFromHotPlate_9045() - Process Flow

> Source: `ainarm_SearchPickPlate.cpp` (line ~683)  
> Task variable: `int &Task = iInArmPickFromHotPlateTask`  
> Return: `true` = pick complete, `false` = still in progress

### Summary

從 HotPlate 吸取已完成 Soak 的 IC。先搜尋 HP 位置，取得 Team 分組(bZFlgToHP)，
移動 XY 定位後等待 Shuttle 就位，Z 下降吸取並進行 HotplateDataConversion 資料轉換。
異常時 Retry / Skip / Home。

### Process Flow

```
case 1     [Init]
           DoInArm_9045_SuckerMap()
           SearchPlateToPick()
           ├─ No HP pick list → ShowMyMessage → return true
           └─ OK → bPickFormHotplatePartOK = false → case 50
              ↓
case 50    MoveInArmZToPlateSafe() → case 100
              ↓
case 100   [Get HP Team]
           iRetryCT = 0
           GetHPFirstTeamMotUse(bZFlgToHP)    — which suckers need Z-down
           GetHPFirstTeamSuckUse(bZFlgToHPPick) — which suckers need suck
           → case 110
              ↓
case 105   MoveInArmZToPlateSafe → case 110
              ↓
case 110   [Move XY to HP Pick Position]
           MoveInArmXYPickHotPlate_9045()
           + IsCheckInArmDestroyActiveFinish()
           → case 150
              ↓
case 150   [Wait Shuttle Ready]
           Check which shuttle (iWhichShtPickFor32)
           Shuttle in load-free position?
           ├─ YES → check carrier kit full → free shuttle
           │        → case 190
           ├─ 1Picker + OneCycle + carrier full → return true
           └─ Other shuttle / TrayArm wait → return true or stay
              ↓
case 190   [Re-confirm XY]
           MoveInArmXYPickHotPlate_9045() → case 200
              ↓
case 200   [Z Down Pick from HP]
           bSuckOnDown → pre-activate vacuum
           Clear bZFlgToHP for items already having IC
           MoveInArmZToHotPlatePick(iRetryCT)
           Offset check → case 210
           InArmSuck.ResetAll() → case 300
              ↓
case 210   Offset enter → case 220 / case 200
              ↓
case 220   MoveInArmZToPlateSafe → case 110
              ↓
case 300   [Suck & Data Conversion]
           bPickFromHotplate = true
           BackupHotplatelocation()
           AutoSiteMap mode → HotplateDataConversion (single)
           Normal mode → GetHPFirstTeam + loop HotplateDataConversion
           All suck done?
           ├─ Has Error → case 320 (retry)
           └─ OK → bPickFormHotplatePartOK = true → case 340
              ↓
case 320   MoveInArmZToPlateSafe → iRetryCT++ → case 330
              ↓
case 330   [Error Handling]
           iRetryCT > limit?
           ├─ YES → ShowErrorMessage JAM0109
           │   ├─ K_SKIP → PorcessJAM0109HotPlatePickUpErrorSkip → case 340
           │   ├─ K_HOME → ResetAll → case 335
           │   └─ K_RETRY → ResetAll → case 335 or 336
           └─ NO → case 190 (re-pick)
              ↓
case 335   SetInArmHome(), iRetryCT=0 → case 190
              ↓
case 336   iRetryCT = 0 → case 190
              ↓
case 340   [Z Safe]
           MoveInArmZToPlateSafe() → case 350
              ↓
case 350   [Next Team]
           DataForwardAndNextTeam()
           ├─ More teams → case 1 (loop)
           └─ All done → case 400
              ↓
case 400   [Finish]
           Auto speed adjust (bAutoSpeed)
           Record pick count (iInArmPickPlaceCnt)
           return true
```

---

## 7. DoInArmPlaceToShuttle_9045_XxY_Z() - Process Flow

> Source: `ainarm9045_XxY_Z.cpp` 
> Task variable: `int &Task = iInArmPlaceToShuttleTask`  
> Return: `true` = place complete, `false` = still in progress

### Summary

將 InArm 上的 IC 放置到 Shuttle（蝦頭）送往測試。先判斷放 Shuttle 1 或 2，
等待 Shuttle 到達 Load-Free 定位（InSHTxInLF），速度調整後移動 XY/Z 定位，
依序 Destroy（吹氣）每個 IC 到 Shuttle，驗證有無粘黏與浮起（Floating），
最後調整 Shuttle 放料順序，Y Pitch 歸位。
兩個 Shuttle 的邏輯對稱（Shuttle 1: case 900-1610, Shuttle 2: case 1900-2610）。

### Process Flow — Shuttle 1 (case 900 ~ 1610)

```
case 1     [Init]
           SetInArmUseSuckToHasNullIC()
           Check HasRealIC → if no IC → Z safe → return true
           Set bCheckSpeed → case 100
              ↓
case 100   [Select Shuttle]
           iSht == 0 → D43 drop error check → case 900 (Shuttle 1)
           iSht == 1 → D43 drop error check → case 1900 (Shuttle 2)
              ↓
case 900   [Wait Shuttle 1 In Position]
           InSHT1InLF() && FL no IC?
           ├─ YES → speed add → case 950 (Z-down path)
           └─ NO  → speed sub → case 930 (wait path)
              ↓
case 930   MoveInArmXYToWaitTrayArm(ZAxisNotDown)
           + CheckInArmSuckICFallDownToHasNullIC → case 935
           Move done → case 1000
              ↓
case 935   IC FallDown full recheck → case 930
              ↓
case 950   [Direct Z-Down Path]
           SetShuttlefCanMoveL(0, false)
           MoveInArmXYToWaitTrayArm(ZAxisDown)
           + IC FallDown check → case 955
           Move done → case 1100 or other
              ↓
case 955   IC FallDown alarm → Z safe → case 950
              ↓
case 1000  [Shuttle 1 Check Load-Free]
           InSHT1InLF() + carrier kit all has IC → bCanFreeShuttle
           bPlaceToShuttle2Step → cannot free
           ├─ Cannot free → SetShuttlefCanMoveL(false), InArmZNeedDown → case 900
           ├─ NeedWaitTrayArm → OCR wait → case 1030
           └─ Can free → case 1050
              ↓
case 1030  OCRMoveInArm2XYToWait → case 1
              ↓
case 1050  MoveInArmZToPlateSafe → SetShuttlefCanMoveL(true) → case 1000
              ↓
case 1100  [Z Down to Shuttle]
           D43 drop error re-check → return false
           SetShuttlefCanMoveL(0, false)
           InShtInLF(0)==false → case 1110 (abort)
           Shuttle vibration (ASE option)
           MoveInArmZToShuttlePlace_9045(0)
           Offset check → case 1150
           InArmSuck.ResetAll()
           ReleaseDelay → case 1180 / case 1200
              ↓
case 1110  [Abort - Shuttle Left]
           MoveInArmZToPlateSafe → SetShuttlefCanMoveL(true) → case 1
              ↓
case 1150  Offset enter → case 1160 / case 1100
              ↓
case 1160  Z safe → case 1170
              ↓
case 1170  MoveInArmXYToShuttle_9045(ZAxisDown) → case 1100
              ↓
case 1180  InArmReleaseDelay off → case 1200
              ↓
case 1200  [Destroy (Blow) IC to Shuttle 1]
           Loop InArmSuck[2×4]:
             Suck.Destroy() → SetShuttleStatus_9045(sht=0)
             HAS_NULL_IC → direct set status
           All done?
           CheckInArmDestroyICFail()
           ├─ HasRealIC still → bPlaceToShuttle2Step = true → case 1250 (2nd pass)
           └─ All placed → case 1300
              ↓
case 1250  [2nd Pass - Move XY for remaining IC]
           MoveInArmXYToShuttle_9045(ZAxisDown) → case 1100
           IC FallDown → case 1255
              ↓
case 1255  IC FallDown alarm → Z safe → case 1250
              ↓
case 1300  [Floating Check Init]
           InitDoInArmCheckShtFloatTask → case 1400
              ↓
case 1400  [Floating Check Execute]
           DoInArmCheckShuttleFloating(0) → case 1500
              ↓
case 1500  [Post-Place Cleanup]
           SetClosedShtKitToHasNullIC_9045()
           AdjustShtOrderWhenPlaceToSht(3)
           Set shuttle soak time (if IndexPickupWait)
           RecordInArmTime()
           bDestoryOnSht = false
           Y Pitch Home (bE57YPitchHome) → case 1600
           else → return true
              ↓
case 1600  bCheckYPitchRunHomeSen → return true
           timeout → InitProcessSingleMotorTask → case 1610
              ↓
case 1610  bCheckYPitchHome → return true
           timeout → WAR0123 alarm → case 1600
```

### Process Flow — Shuttle 2 (case 1900 ~ 2610)

Shuttle 2 流程與 Shuttle 1 對稱，case 編號偏移 +1000：

| Shuttle 1 | Shuttle 2 | 功能 |
|---|---|---|
| case 900 | case 1900 | Wait shuttle in load-free |
| case 930 | case 1930 | XY wait (Z not down) |
| case 935 | case 1935 | IC fall-down recheck |
| case 950 | case 1950 | XY move (Z down) |
| case 955 | case 1955 | IC fall-down alarm |
| case 1000 | case 2000 | Check load-free & can-free |
| case 1030 | case 2030 | OCR wait |
| case 1050 | case 2050 | Z safe → free shuttle |
| case 1100 | case 2100 | Z down to shuttle |
| case 1110 | case 2110 | Abort (shuttle left) |
| case 1150 | case 2150 | Offset enter |
| case 1160 | case 2160 | Z safe (offset) |
| case 1170 | case 2170 | XY move (offset redo) |
| case 1180 | case 2180 | Release delay |
| case 1200 | case 2200 | Destroy IC to shuttle |
| case 1250 | case 2250 | 2nd pass XY move |
| case 1255 | case 2255 | IC fall-down alarm (2nd) |
| case 1300 | case 2300 | Floating check init |
| case 1400 | case 2400 | Floating check execute |
| case 1500 | case 2500 | Post-place cleanup |
| case 1600 | case 2600 | Y pitch home check |
| case 1610 | case 2610 | Y pitch home execute |
