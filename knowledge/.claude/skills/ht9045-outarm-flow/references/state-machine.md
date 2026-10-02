# OutArm 狀態機與放料細節

## 主狀態機 — DoOutArm_9045_2x8_8() (代表所有 XxY_Z 版本)

> Source: `aoutarm9045_2x8_8.cpp` line ~1766  
> Task variable: `int &Task = OutArmTask`

### Pre-Switch Guard

- `bCarryControlOutarm1 || bCarryControlOutarm2` → return（Carry 控制中）

### 主流程

```
case 1   → case 5    → case 10   → case 50
(Reset)    (Z Safe)    (Init      (Z Safe +
                        Check)     Choose Shuttle)
                                       |
                    ┌──────────────────┘
                    ↓
              case 100 (Decide Shuttle)
              ┌─ OldPos==1 ──→ case 1000 (Shuttle 1 Flow)
              └─ OldPos==0 ──→ case 2000 (Shuttle 2 Flow)
```

### Shuttle 1 Pick Flow (case 1000 ~ 1235)

```
case 1000 → case 1010~1030 (Y Pitch Home, optional)
          → case 1140 (Check: CleanOut / Has IC / Shuttle Ready)
          → case 1150 (Move XY to Shuttle 1, Sub Speed)
          → case 1100 (Wait Shuttle Ready, Check Tray Need)
          → case 1200 (DoPickFromShuttle_9045_2x8_8(0))
               ├─ Still has IC on same side → loop 1200
               ├─ STM/TW153 mode → switch kit side → loop 1200
               ├─ No IC picked → case 50
               └─ Pick done → case 3000
```

### Shuttle 2 Pick Flow (case 2000 ~ 2235)
結構與 Shuttle 1 完全對稱，case 編號 +1000。

### Place to Tray Flow (case 3000 ~ 3500)

```
case 3000 (Z Safe + IC Fall Down Check + Release Shuttle)
     ├─ Need Additional Function → case 7000
     └─ No Additional → case 3010

case 3010 (SearchTrayToPlace_9045 + VerifyFixTrayLink)
     ├─ Fix Tray no tray → case 4000
     ├─ Magazine busy → wait
     └─ Ready → case 3100

case 3100 → case 3300 (Verify Fix Tray Sensor)
          → case 3305 (Auto Alignment mode, optional)
          → case 3310: DoOutArmPlaceToAuto_9045()
               Done → DoOutArmAfterPlaceToAuto(init)
               → case 3500

case 3500: DoOutArmAfterPlaceToAuto()
     ├─ return 3010 → 吸嘴還有 IC, 繼續放
     ├─ return 100  → 全部放完, 回去拿下一批
     ├─ return 11100 → Magazine Buffer 處理
     └─ return 5000 → Clean Out 整盤
```

### Special Cases

| case | Name | Description |
|---|---|---|
| **200** | Yield Position | Z Safe 後移到讓位位置 |
| **300** | Yield Move | 32Site: 移到 Shuttle 2 上等待 |
| **310/311** | Auto Z Teach | OutArm Z 軸自動校正高度 |
| **4000** | Fix Tray Missing | 移到讓位位置 |
| **4100** | Fix Tray Full Alarm | `DoFixTrayFullAlarm()` |
| **7000** | Additional Function | Rotator / AOI / Fix AI CCD |
| **10000** | Magazine Tray | `CheckPlaceToMagazineTray()` |
| **11100** | Magazine Buffer Pick | `DoPickFromMagazineBuffer()` |
| **12000** | Magazine Place | 等待 Magazine 就位 |

### State Descriptions

| case | Name | Description |
|---|---|---|
| **1** | Reset | 重置 SortingAllBinTrayFinish 旗標 |
| **5** | Z Safe | `MoveOutArmToAutoSafe()` |
| **10** | Init Check | `CheckOutArmInitState()` |
| **50** | Shuttle Decision | Z Safe → 釋放 Shuttle → 判斷有 IC 否 |
| **100** | Shuttle Select | 決定先取 Shuttle 1 還是 2 |
| **1000-1030** | SHT1 Y Pitch Home | Y Pitch 馬達歸位 |
| **1140** | SHT1 Pre-Check | CleanOut / Has IC / Shuttle 就位 |
| **1150** | SHT1 Move XY | 移到 Shuttle 1 上方 |
| **1100** | SHT1 Wait Ready | 等 Shuttle 就位 |
| **1200** | SHT1 Pick IC | `DoPickFromShuttle_9045_2x8_8(0)` |
| **2000-2200** | SHT2 完整流程 | 與 SHT1 對稱 |
| **3000** | Post-Pick | Z Safe → IC Fall Down → 釋放 Shuttle |
| **3010** | Search Tray | `SearchTrayToPlace_9045()` |
| **3100-3310** | Place to Auto | 驗證 → 放料 |
| **3500** | After Place | 後處理 |

---

## DoPickFromShuttle_9045_2x8_8() — Shuttle 取料

> Task: `iPickFromShuttle1Task` (SHT1) / `iPickFromShuttle2Task` (SHT2)

### 關鍵 Case

| case | 動作 |
|---|---|
| 1 | 若 `dWaitOnSH != 0` 且首次 → 移到 Shuttle 等待 |
| 2 | 等待時間到 (`bOutShtwaitPick`) |
| 10 | 初始化 |
| 200 | `MoveOutArmToShuttleIncludeZ_9045_2x8_8()` XY/Z 定位 |
| 500 | Offset / AutoTeach 完成 |
| iOUTARM_SUCK | 逐一吸取：`Suck()` → `SwapShuttleDataToOutArm()` |
| 2000 | Error: Z Safe → Retry / `OutArmPickShuttleAlarm()` |
| 2200 | Home: `SetOutArmHome()` |

### Error 處理（case 2000）

- `iOutShtRetryCount <= RetryCT` → 自動 Retry
- 超過 → Alarm：`K_SKIP` / `K_HOME` / `K_RETRY`

---

## DoOutArmAdditionalFunction() — 附加功能

> Task: `iOutArmAdditionalFunctionTask`

```
case 1 → case 100 (check flags, Z Safe, IC Fall Down Check)
           ├─ Rotator   → case 10000
           ├─ AOI       → case 20000
           ├─ Fix AI CCD→ case 30000
           └─ All done  → return true
```

| 功能 | 觸發條件 |
|---|---|
| Rotator | `USE_ROTATE_KIT==1 && tRotate.ActiveRotate && iOutRotateFinish==0/1` |
| AOI | `tAOISetup.bEnabledAOI` 或 Scanner/Top Scanner AOI |
| Fix AI CCD | `USE_Fix_AI_CCD && TestIF_File.bEnableFix2BGAAICCD && fFixAICCD->NeedToGrabImage()` |

---

## DoOutArmPlaceToAuto_9045() — 放料到 Tray 控制

> Task: `iPlaceToAutoTask`

| case | 動作 |
|---|---|
| 1 | 初始化 |
| 10 | IC Fall Down → SearchTray → SetOutArm_9045() 計算位置 |
| 30 | 等待 Tray 有空位 |
| 50 | 全時掉料檢查 |
| 100 | Offset 校正 → Destroy 延遲 |
| 300 | `DoOutArmPlaceToAuto(iWhichAuto)` 實際放料 |
| 400 | Rotate + Auto Tray Detect |
| 500 | `DetectAutoTray()` |

---

## DoOutArmPlaceToAuto() — 實際放料 Destroy

> 迴圈遍歷所有 `bOutArmSuckActive[i][j]` 為 true 的吸嘴：

1. NULL IC 處理
2. `OutArmSuck.Suck[i][j].Destroy()` 放料
3. 資料記錄（BIN、Site、LotSummary、ProdRec）
4. Barcode/2DID 記錄
5. Bin 計數（`iByBinTotal[]`, `LastSet.BinCT[]`）
6. ART 計數
7. Error 處理（`JAM0217`）
8. Tray 滿盤檢查
9. Yield 檢查

---

## DoOutArmAfterPlaceToAuto() — 放料後處理

> Task: `iDoOutArmAfterPlaceToAutoTask`

### 回傳值對照

| 回傳值 | 意義 | 主狀態機跳轉 |
|---|---|---|
| 0 | 尚未完成 | 維持 case 3500 |
| 100 | 正常完成 | 回 case 100 |
| 3010 | 吸嘴還有 IC | 回 case 3010 繼續放 |
| 5000 | Clean Out + 整盤 | case 5000 |
| 11100 | Magazine Buffer | case 11100 |

---

## CheckOutArmCleanOut() — Clean Out 判斷

| 條件 | 回傳 |
|------|------|
| P27 整盤 + 所有模組無 IC | 5000（進入整盤） |
| Magazine Buffer 模式 + 無 IC | 11100 |
| ATK AMR + 有 Buffer IC | 11100 |
| Auto Z Teach | 310 |
| 以上都不符合 | 原 Task |
