# 網頁同事 20260915 那版**獨有**的段落（AI(W906-BA-SKILL) 20260915）

這一支是**分岔不是版本差**：他 161 行 / 我們 176 行，他多 11 節、我們多 9 節。
拿比較短的那份當底會把我們那幾節從主文擠掉，所以主文保留我們的，
他獨有的收在這裡。

---

## 呼叫階層總覽

```
DoOutArm()                                         <- aoutarm.cpp
  └─ DoOutArm_9045()                               <- aoutarm9045.cpp (Dispatch)
       └─ DoOutArm_9045_2x8_8() [依 iInArmType]
            ├─ case 1200/2200: DoPickFromShuttle_9045_2x8_8(iSht)
            │     ├─ case 200:  MoveOutArmToShuttleIncludeZ
            │     ├─ case iOUTARM_SUCK: Suck() 吸取 IC
            │     └─ case 2000: Error Retry/Skip/Home
            ├─ case 7000: DoOutArmAdditionalFunction()
            │     ├─ DoOutArmRotateKIT()
            │     ├─ DoAOIFunction()
            │     └─ DoFix2AICCDFunction()
            ├─ case 3010: SearchTrayToPlace_9045()
            │     ├─ VerifyFixTrayLink() == 4000 → Task=4000  (Fix Tray 滿/無盤)
            │     └─ iHWFix_BinBox==1 && eBulkBox → DoFixTrayFullAlarm() ⚠️ 直達（無 Move 前置）
            ├─ case 3300: Sen[SnFixedTrayDetect].IsOff() → Task=4000  (感測器無盤)
            ├─ case 4000: MoveOutArmXY_ToFix_Tray_Full(true) → Task=4100
            ├─ case 4100: DoFixTrayFullAlarm() → Task=3010  (換盤完成)
            ├─ case 3310: DoOutArmPlaceToAuto_9045()
            │     └─ DoOutArmPlaceToAuto(iWhichAuto)
            └─ case 3500: DoOutArmAfterPlaceToAuto()
                  ├─ case 1000: DoFix3FullTray()
                  ├─ case 5000: DoSortingBinTray()
                  └─ case 11100: DoPickFromMagazineBuffer()
```

---

## 取料段（§1–§5）— Shuttle Pick

> 詳見取料段 case 流程、iInArmType 幾何配置、Suck 吸取時序、Error 處理與錯誤警報。
> [references/outarm-pick.md](references/outarm-pick.md)

### 重要摘要

| 函式 | 說明 |
|---|---|
| `DoOutArm()` | 入口：NULL IC 清除、Shuttle 待料判斷，再分發到 Dispatch |
| `DoOutArm_9045()` | Guard Checks（Servo Off / Destroy Active / QA Mode）+ iInArmType 分派 |
| `DoOutArm_9045_2x8_8()` | SHT1/SHT2 雙軌取料、Pick Flow 主狀態機（case 1000/2000） |
| `DoPickFromShuttle_9045_2x8_8()` | XY/Z 移動 → Suck → Retry/Skip/Home |

---

## 放料段（§6–§9）— Place to Tray

> 詳見 case 值、Destroy 吹氣、BinCT 計數、AfterPlace 後置分發流程。
> [references/outarm-place.md](references/outarm-place.md)

### 重要摘要

| 函式 | 說明 |
|---|---|
| `DoOutArmAdditionalFunction()` | Rotator / AOI / Fix AI CCD 附加功能 |
| `DoOutArmPlaceToAuto_9045()` | Tray 定位 → Offset → Destroy 吹氣 → 放料 → Tray 計數 |
| `DoOutArmPlaceToAuto()` | 執行 Destroy 吹氣 + BinCT / ART 計數 + Lot Summary |
| `DoOutArmAfterPlaceToAuto()` | 後處理分發：100/110/3010/5000/11100 |

---

## Fix Tray Full 換盤流程（§Fix）

> 適用問題：Fix Tray 滿盤時 OutArm 位置干涉、IC Drop、換盤前移動路徑。

### case 4000 的三條觸發路徑

| 路徑 | 觸發條件 | 是否有 Move 前置 |
|------|---------|----------------|
| **路徑 1**（主）| `VerifyFixTrayLink()` 回傳 `4000`：Fix Tray 區（`iWhichAuto>=iFixMin`）且滿盤或無盤，且無法 Link 到下一盤 | ✅ case 4000 保護 |
| **路徑 2**（補）| `case 3300`：`Sen[SnFixedTrayDetect].IsOff()`（感測器實體無盤）且 `iWhichAuto>=iAutoCnt` | ✅ case 4000 保護 |
| **路徑 3** ⚠️ | `case 3010` BulkBox 直達：`iHWFix_BinBox==1 && iWhichAuto==eBulkBox` → 直接呼叫 `DoFixTrayFullAlarm()`，**跳過 case 4000** | ❌ 無 Move 前置 |

### `VerifyFixTrayLink()` 回傳值說明（`aoutarm9045.cpp` L1595）

| 回傳值 | 意義 |
|--------|------|
| `4000` | Fix Tray 滿/無盤，需換盤（主要換盤觸發） |
| `10000` | Magazine 模式，跳到 Magazine 流程 |
| `1` | 可 Link 到下一個 Fix Tray（IC 指向 Fix2/Fix3），繼續放料 |
| `0` | 正常，無需換盤 |

### `MoveOutArmXY_ToFix_Tray_Full()` 移動目標（`aoutarm.cpp` L285）

```cpp
int iXPos = MOT[MOutArmX].Motor->PSoftLimitN + 6000;  // X 軸負端極限 +6000
int iYPos = Prod.iOutArmSafeY;                         // Y 軸安全位置
// 特例：CC_ASE_SG 時 iYPos = Prod.iOutArmSafeY - 15000（bMoveY=true）
//       且 iXPos = Tech.iOutArmAuto2X
```

### 換盤流程 case 序列

```
case 3010
  → VerifyFixTrayLink() == 4000
  → Task = 4000
case 4000
  → MoveOutArmXY_ToFix_Tray_Full(true)   // OutArm 移至安全 XY（等待完成）
  → Task = 4100
case 4100
  → DoFixTrayFullAlarm()                  // 顯示警報，等待 CatchTray 換盤
  → Task = 3010                           // 換盤完成，重新搜盤繼續放料
```

### 覆蓋率（V3.33.901.0）

掃描 27 個 aoutarm9045_*.cpp 模式：
- **路徑 1 / 路徑 2**（經 case 4000）：27/27 個模式均有 `MoveOutArmXY_ToFix_Tray_Full` 前置保護 ✅
- **路徑 3**（BulkBox 直達）：26 個模式存在此路徑（`aoutarm9045_2x2_4_23.cpp` 無），**均無 Move 前置** ⚠️

---

## Bin 分類 / CleanOut（§10–§13）

> 詳見 CheckOutArmCleanOut 判斷、SearchTrayToPlace 搜盤、OutArm vs InArm 計數差異、FAQ 說明。
> [references/outarm-bin.md](references/outarm-bin.md)

### 重要摘要

| 函式 | 說明 |
|---|---|
| `CheckOutArmCleanOut()` | 回傳 0/310/5000/11100 決定接下來的 CleanOut 流程 |
| `SearchTrayToPlace_9045()` | 依 BIN 結果搜尋可放 IC 的 Auto/Fix Tray 位置 |
| `DoSortingBinTray()` | Clean Out 後執行 Tray 換盤 → 回收 → 搬送|
