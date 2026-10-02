---
name: ht9045-outarm-flow
description: >
  HT9045 IC Test Handler OutArm 流程知識庫。當使用者詢問 OutArm、Output Arm、
  Shuttle 取料、Unloader Tray 放料、Auto Tray 放料、Fix Tray 放料、Bin 分類、
  Magazine、吸嘴（Sucker）、真空吸取（Vacuum Suck）、Destroy 吹氣、掉料（IC Fall Down）、
  Pick Error、Retry/Skip/Home、AOI、Rotator、Fix AI CCD、Clean Out 判斷、BinBox 等
  相關問題時，應先載入此技能以理解 OutArm 完整處理流程。
  關鍵字：DoOutArm, DoOutArm_9045, OutArmSuck, OutArmTask, iInArmType, Shuttle,
  Unloader, Auto Tray, Fix Tray, Magazine, DoPickFromShuttle, DoOutArmPlaceToAuto,
  DoOutArmAfterPlaceToAuto, CheckOutArmCleanOut, SearchTrayToPlace, DoSortingBinTray,
  DoOutArmAdditionalFunction, iPlaceToAutoTask, iDoOutArmAfterPlaceToAutoTask,
  DoOutArmRotateKIT, DoAOIFunction, DoFix2AICCDFunction, Destroy吹氣, Fix3FullTray,
  JAM0217, Yield計數, P27, iBinBoxSelect, iAMRFixTraySelect, 吸嘴幾何配置,
  MoveOutArmToShuttleIncludeZ, SwapShuttleDataToOutArm, iPickFromShuttle1Task,
  iPickFromShuttle2Task, OutArmPickShuttleAlarm, OldPos, 32Site模式。
  另含 Scanner AOI 拍照位置偏移（X-Pitch 出界 + 吸嘴欄位映射）：
  AOI 第二顆拍不到, AOI 拍照位置偏移, 第一顆對第二顆錯, DoMoveXY_ScannerAOI,
  iScannerAOI_X, Tech.M_ScannerAOI_X, OutArm XPitch out of range, ScannerAOI pitch probe,
  iMovePitchX 21000, GetOutArmPitchX_9045, GetOutArmPitch_9045, clamp, MOutArmPitch,
  開迴路步進失步, 失步不報警, XPitchIsStand, iXpitchMaxX3, IN_OUT_ARM_X_PITCH_MAX,
  iMyCol, CopyInitSuck, SetInOutArmParameter_1x2_2, iPickStep, e9045_1x2_2_14,
  e9045_1x2_2_13, 吸嘴交換, OutOfsScannerAOI, Position Offset.Data, GetOffsetPath,
  bE45_AllSetupFileUseOneFile, bE59GroupOffsetFile, 重新teach 影響, dummy run 驗不到相機,
  iRealDummy REALLY, 偉測 HHT-532。
applyTo: "**/*"
---

# HT9045 OutArm Flow Knowledge

## 適用場景

以下問題請載入此技能查閱對應 case 值與流程細節：
- OutArm / Output Arm 的動作流程、狀態機、case 值意義
- Shuttle 取料（Pick from Shuttle 1 / Shuttle 2）
- Unloader Tray 放料（Place to Auto Tray / Fix Tray）
- Bin 分類與 Tray 條件（SearchTrayToPlace, CheckBin）
- Magazine 放料處理（Magazine Tray / Buffer）
- 附加功能（Rotator / AOI / Fix AI CCD）
- 吸取異常處理（Retry / Skip / Home）
- IC 掉料（Fall Down）與 Destroy 吹氣
- Clean Out 判斷與 Auto Sorting BinTray
- OutArm sucker 幾何配置（iInArmType）
- Flag 邊界（`bSingleUseOtherSuck` vs `bSingleInArmUseOtherSuck`）— 詳見 [SingleSiteOtherSuck-Flag-Boundary.md](references/SingleSiteOtherSuck-Flag-Boundary.md)
- **Fix3 滿盤氣缸 `JAM1940`/`JAM1941` 誤報**（緊跟在別的 alarm 恢復後 1~2 秒內跳）— 詳見 [fix3-cylinder-jam1940-false-alarm.md](references/fix3-cylinder-jam1940-false-alarm.md)

## 吸嘴物理排列（8-Sucker 模式）

```
Row 0:  A   C   E   G      ←  OutArmSuck.Item[0][0] ~ Item[0][3]
Row 1:  B   D   F   H      ←  OutArmSuck.Item[1][0] ~ Item[1][3]
        j=0 j=1 j=2 j=3
```

| 吸嘴 | Item 索引 | 說明 |
|------|-----------|------|
| A | `Item[0][0]` / `Suck[0][0]` | Row0 第1吸嘴 |
| C | `Item[0][1]` / `Suck[0][1]` | Row0 第2吸嘴 |
| E | `Item[0][2]` / `Suck[0][2]` | Row0 第3吸嘴 |
| G | `Item[0][3]` / `Suck[0][3]` | Row0 第4吸嘴 |
| B | `Item[1][0]` / `Suck[1][0]` | Row1 第1吸嘴 |
| D | `Item[1][1]` / `Suck[1][1]` | Row1 第2吸嘴 |
| F | `Item[1][2]` / `Suck[1][2]` | Row1 第3吸嘴 |
| H | `Item[1][3]` / `Suck[1][3]` | Row1 第4吸嘴 |

> 吸嘴排列與 InArm 相同。OutArm 的 Variable Pitch 機構同樣支援 1-3 (A-E) 與 1-4 (A-G) 模式。

## OutArm 基準軸（與 InArm 鏡像對稱）

**基準軸 = `OutArmSuck.Suck[iOutArmYBase][iOutArmXBase]`** — 是 OutArm 所有 Teach 位置（`Tech.iOutArm*`）的參考原點。其他 7 顆吸嘴的座標由基準軸 + 個別 Z 高差（`Tech.iOutArmZHeightSub`）+ Offset 推算。

### 依 Y-Pitch 模式對照

| `USE_IN_OUT_ARM_Y_PITCH` 模式 | `iOutArmYBase` | `iOutArmXBase` | OutArm 基準吸嘴 | 對應 InArm 基準 | 對稱關係 |
|---|---|---|---|---|---|
| `iXPitch60` (Fixed 60mm) | 0 | 2 | **E** [0,2] | E [0,2] | 兩臂同位置 |
| `iXPitchManual635` (60/63.5) | 0 | 2 | **E** [0,2] | E [0,2] | 兩臂同位置 |
| `iXYPitchVariable` (可變 X+Y) | **1** | **1** | **D** [1,1] | F [1,2] | **鏡像對稱**（col 1 ↔ col 2）|
| `iXYPitchRowA` (Row A 可變) | 0 | **1** | **C** [0,1] | E [0,2] | **鏡像對稱**（col 1 ↔ col 2）|

> **JerryYang 20251218 補充**：`USE_OUT_ARM_Y_PITCH` 獨立於 `USE_IN_OUT_ARM_Y_PITCH`（`database.cpp` L933-940），若 OutArm 設為 `iXPitch60` / `iXPitchManual635` 會覆蓋為 `iOutArmXBase=2, iOutArmYBase=0`，使 OutArm 回到固定模式基準（E）。

### 物理意義

InArm 與 OutArm **面朝相反方向**安裝（InArm 朝 Loader、OutArm 朝 Unloader），可變 X+Y Pitch 機構也是鏡像安裝。從機台「全域座標」看，InArm 基準軸在 col 2 → 鏡射到 OutArm 落在 col 1。

```
可變 X+Y Pitch 模式：

InArm（朝 Loader）：                  OutArm（朝 Unloader，鏡像）：
  A   C   E   G   ← Row 0           A   C   E   G   ← Row 0
  B   D  ★F★  H   ← Row 1           B  ★D★  F   H   ← Row 1
  ↑       ↑                          ↑   ↑
  XP1     YP                         XP1 YP

★ 表示基準軸吸嘴。
```

### OutArm 取放料公式（與 InArm 公式平行）

```cpp
// Shuttle 取料（aoutarm9045 系列）
iXPos = Prod.XOutArm_Shuttle_Pick[iOutArmYBase][iOutArmXBase]   // 基準軸 Teach
      + iCol * SHT.XPitch                                        // Shuttle Site 跨距
      + iOutArmXBase * dOutArmXPitch_1Step;                      // 基準軸→第 0 吸嘴 Offset

iYPos = Prod.YOutArm_Shuttle_Pick[iOutArmYBase][iOutArmXBase]
      - iRow * SHT.YPitch;

// 最後加上個別吸嘴 Offset：
iXPos += OutArmOffSet[OutOfsSht1]->GetArmX(i, j);
iYPos += OutArmOffSet[OutOfsSht1]->GetArmY(i, j);
```

**X-Pitch 馬達單步距離**：

```cpp
dOutArmXPitch_1Step    = OutArmClose_PitchX / 3.0;   // 4 吸嘴模式
dOutArmXPitch_MovePitch = dOutArmXPitch_1Step * 3.0;
```

對應馬達編號：

| InArm | OutArm |
|---|---|
| `MInArmX/Y/Pitch/ZA~ZH` | `MOutArmX/Y/Pitch/ZA~ZH` |
| `MInArmPitchY` (YP) | `MOutArmPitchY` |
| `MInArmPitchX2` (XP2，可變模式) | `MOutArmPitchX2` |

> **詳細推導**：見 [ht9045-sucker-architecture](file:///d:\HT9045\.github\skills\ht9045-sucker-architecture\SKILL.md) §4.3-4.5 與 [ht9045-inarm-flow references/InArm_TMyKitSuck_and_TypeDecision_20260513.md](file:///d:\HT9045\.github\skills\ht9045-inarm-flow\references\InArm_TMyKitSuck_and_TypeDecision_20260513.md) §11-§17。

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|---|---|---|
| `aoutarm.cpp` | `DoOutArm()` — 最上層入口 | 外層 switch|
| `aoutarm.cpp` | `CheckOutArmCleanOut()` — Clean Out 判斷 | 外層 switch|
| `aoutarm.cpp` | `DoOutArmPlaceToAuto()` — Destroy 後置放料 | 外層函式|
| `aoutarm9045.cpp` | `DoOutArm_9045()` — Dispatch | （Dispatch）|
| `aoutarm9045.cpp` | `DoOutArmPlaceToAuto_9045()` — 放料主流程 | `iPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAfterPlaceToAuto()` — 放料後處理 | `iDoOutArmAfterPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAdditionalFunction()` — 附加功能 | `iOutArmAdditionalFunctionTask` |
| `aoutarm9045_2x8_8.cpp` | `DoOutArm_9045_2x8_8()` — 主狀態機 | `OutArmTask` |
| `aoutarm9045_2x8_8.cpp` | `DoPickFromShuttle_9045_2x8_8()` — Shuttle 取料 | `iPickFromShuttle1/2Task` |
| `aoutarm.cpp` | `MoveOutArmXY_ToFix_Tray_Full()` — Fix Full 時 OutArm 移至安全 XY（L285） | — |
| `aoutarm9045.cpp` | `VerifyFixTrayLink()` — 判斷 Fix Tray 滿/無盤，回傳 0/1/4000/10000（L1595） | — |
| `aoutarm9045.cpp` | `DoFixTrayFullAlarm()` — Fix Tray 滿盤警報 + 等待換盤完成（L1669） | — |

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

## 附加功能段 — Rotator → AOI → Fix AI CCD

> **執行順序固定：Rotator（case 10000）→ AOI（case 20000）→ Fix AI CCD（case 30000）**。
> `DoOutArmAdditionalFunction()` 在 case 100 依序分派，每做完一項回 100 再挑下一項
> （`aoutarm9045.cpp`）。整段夾在 Shuttle 取料與搜盤放料之間：
> **Shuttle 取料 → Rotator → AOI → 搜盤放料 → Sorting**。
>
> **Scanner AOI 拍照位置偏移（X-Pitch 出界 + 吸嘴欄位映射）**：
> [references/outarm-aoi-xpitch-photo-offset.md](references/outarm-aoi-xpitch-photo-offset.md)
> 關鍵字：AOI 第二顆拍不到／偏移、第一顆對第二顆錯、`DoMoveXY_ScannerAOI`、
> `iScannerAOI_X`、`OutArm XPitch out of range`、`iMovePitchX=21000`、
> `GetOutArmPitchX_9045` clamp、`MOutArmPitch` 開迴路失步不報警、
> `Suck[][].iMyCol` 被 `CopyInitSuck` 交換、`iPickStep`（13→2／14→3）、
> `Tech.M_ScannerAOI_X`（機台級⛔）vs `Position Offset.Data`（per-recipe✅）、
> dummy run 驗不到相機、偉測 HHT-532

## 放料段（§6–§9）— Place to Tray

> 詳見 case 值、Destroy 吹氣、BinCT 計數、AfterPlace 後置分發流程。
> [references/outarm-place.md](references/outarm-place.md)
>
> **Y-Pitch 縮 pitch 換行程（放料 Y 踩到 `MOutArmY` 負向軟體極限）→ 大 IC 前後排互撞 JAM0203**：
> [references/outarm-ypitch-softlimit-shrink.md](references/outarm-ypitch-softlimit-shrink.md)
> 關鍵字：Y Pitch 靠太近、吸嘴把料碰掉、JAM0203 八支 Z 全在 50、`iMovePitchY`、
> `IN_OUT_ARM_Y_PITCH_MIN`、`PSoftLimitN`、`ShrinkOutArmYPitchForSoftLimit`、
> `MOutArmPitchY` 停在 `setEditOutY15`、`AutoCalculateOutArmYClosePitch`（⛔不可改）、
> Tray 最後一排、`iOutArmPlaceOrder==0`、芯云 PPLD2924

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

> ⚠ **`JAM1940` / `JAM1941` 若緊接在另一個警報被排除後 1~2 秒內出現 = 誤報，氣缸沒壞。**
> `Fix3CylinderDelay` 的守門旗標 `bHangTimePause` 會在 modal 解除的同一毫秒被 Index case 2092 清掉。
> 判定法、機制、修法 B/A/C 見 [references/fix3-cylinder-jam1940-false-alarm.md](references/fix3-cylinder-jam1940-false-alarm.md)

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

### `MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false)` 移動目標（`aoutarm.cpp`，904.5/906.2 約 L286/L311）

OutArm 所有「讓位給換盤/TrayArm」的退讓共用此函式。**預設 `bMoveY=false`**（`aoutarm.h`）。

```cpp
int iXPos = MOT[MOutArmX].Motor->PSoftLimitN + iOutArmXBase*2000 + 100;  // X 退到負端軟極限附近（最左）
int iYPos = Prod.iOutArmSafeY;                                           // Y = 安全位（預設，bMoveY=false）

// bMoveY=true 時（一般客戶需 [E90] ON）：iYPos = Prod.iOutArmSafeY_FixFull
// CC_ASE_SG 特例：iYPos = Prod.iOutArmSafeY - 15000，且 iXPos = Tech.iOutArmAuto2X
```

| 項目 | 值 / 來源 |
|------|-----------|
| `Prod.iOutArmSafeY` | `= Tech.iOutArmShuttle1Y`（或 `+iOutArmShtYCenterPos`，`cinitial.cpp` L9816-9820）→ **退讓 Y ＝ Shuttle 1 上方** |
| `Prod.iOutArmSafeY_FixFull` | `= iOutArmSafeY + (bE90 ? -15000 : 0)`（`cinitial.cpp` L9823）；**-15000 ＝ -150mm（15cm）**，單位見 `ht9045-motor-control` |
| ⚠ 不可重 teach | `iOutArmSafeY` 取自 **Shuttle1 取料 teach 點**；改它會同時破壞 OutArm 從 Shuttle1 取料的對位 |

### `[E90] bE90_OutArmFixFullExtraY`（退讓量加大；不是補盤的解）

- **原始目的**：給 **CC_ASE_SG** 的「Fix Tray 滿盤後門取盤（back-door tray pickup）」——`Steven 20260428`。
- ASE_SG：**強制 ON 且隱藏**；其餘客戶（含 ATK 971）：**畫面可見、預設 OFF**（`cConfiguration.cpp` L2106-2110）。
- **只在 `bMoveY=true` 的路徑生效**（多數模式僅 Fix-Tray-Full 用 `(true)`，如 `aoutarm9045_2x8_8.cpp` L2886）。
- ⚠ **不影響一般補盤退讓**：補盤退讓呼叫 `MoveOutArmXY_ToFix_Tray_Full()` 為 `bMoveY=false`（無參數）。

### 補盤退讓位置（每次換盤的避讓）— 客戶常問「OutArm 退到哪？」

> ⚠ **先用 state record 的 `iInArmType` 確認模式檔**，不要從客戶口頭「1x4/1x2」猜。
> `iInArmType` 由 `DoOutArm_9045()`（`aoutarm9045.cpp`）分派至各模式檔，enum 見 `MachineType.h`。
> 例：**ATK ZEPPELIN 32SITE 案** `iInArmType=25=e9045_2x8_32` → `DoOutArm_9045_2x8_8()` →
> 檔案是 **`aoutarm9045_2x8_8.cpp`**（8 吸嘴 / 2 排，ZA~ZH），**不是** 單排 1x4。

CatchTray 要補盤時，OutArm 在 place 搜盤段呼叫 `MoveOutArmXY_ToFix_Tray_Full()`（`bMoveY=false`）退讓
（`aoutarm9045_2x8_8.cpp` L2728/L2747/L2785，Sam 20180822 註：避免與 TrayArm 補盤互卡 hang up）：

```cpp
if(MOT[MTrayX].ReadPos() > Prod.iXTrayColor)                 { MoveOutArmXY_ToFix_Tray_Full(); break; } // TrayArm 進 Auto 區
if(iCatchTrayControlManual>=2 || WhichAutoNeedTray()!=0)     { MoveOutArmXY_ToFix_Tray_Full(); break; } // CatchTray 要補盤
```

→ 退讓目標 = **X 最左（PSoftLimitN 附近）、Y = iOutArmSafeY = Shuttle1 Y（即 Shuttle1 上方）**。

**若「Shuttle1 上方」幾何上仍無法完全避開 TrayArm Z 管 → 須改程式**（teach 不能改、E90 config 救不到補盤路徑）：
- **(a) 複用 E90**：把補盤退讓呼叫改為 `MoveOutArmXY_ToFix_Tray_Full(true)` 並啟用 `bE90`，讓 -150mm 退避＋`IsTrayArmMoveAvoidOutArmCrash()` 互鎖也套用到補盤；須先確認 `MOutArmY` 的 `PSoftLimitN` 仍有 ≥150mm 行程。
- **(b) 新增獨立參數**：加一個與 Shuttle1Y 脫鉤的「補盤退讓 Y」teach/config，依實測需要的避讓量微調（通常不需整整 150mm）。

### 兩個放行 TrayArm 的安全互鎖函式 — `IsOutArmSafe()` vs `IsCatchTrayReadySupplyNewTray()`

> ⚠ 客戶問「OutArm 與 TrayArm 干涉」「補盤撞 OutArm Z 管」時必看。
> **這兩個函式都會授權 TrayArm 進入 Auto 區**，方向不同、嚴格度不同，但**錨的 base Y 相同 → 同一個盲點**。

| | `IsOutArmSafe()` | `IsCatchTrayReadySupplyNewTray()` |
|---|---|---|
| 定義 | `acatchtray.cpp` L3754 | `aoutarm.cpp` L522 |
| 本質 | **查詢**（read-only guard） | **授權**（有副作用） |
| 誰呼叫 | 想移動的一方：**TrayArm / CatchTray / TrayMapping / System**（`acatchtray.cpp` 2137/2627/2792…、`cTrayMapping.cpp` 2497、`csystem.cpp` 14904） | **OutArm 自己**，在各模式檔 state machine 幾乎每個 case（`aoutarm9045_2x8_8.cpp` 2019/2038/2297/2308/2576/2700…） |
| 問的問題 | 「OutArm 退得夠遠，換我動安全嗎？」 | 「我(OutArm)已停妥，授權你來補盤」 |
| 副作用 | 無 | **設 `iCatchTrayControlManual=1`** 交出控制權（L556） |
| 位置條件 | `iPosY >= base` 但**有容差**（依 config 分支 0 / −1900 / −3500），或 `CompareCommandPos(iOutArmSafeY,2)==1` | `iPosY >= base`（多數分支**無容差**）**且 `MOutArmX/Y.fCMD==false`（完全停妥）** |
| 嚴格度 | 較寬（容許 OutArm 快到位就提前放行） | 較嚴（要求停妥才授權） |

**何時用哪個**
- `IsOutArmSafe()`：某個會動的機構（TrayArm/系統）**進入 OutArm 可能佔據區之前先查一聲**。只查不改，可隨意呼叫；容差就是為了讓 TrayArm 不要等太久。
- `IsCatchTrayReadySupplyNewTray()`：OutArm 跑到某 state、**想趁機把補盤權限交出去**時呼叫。**有副作用，不可拿來純查詢。**

**分工鐵律（HT-9132 案時序實證，2026-07）**：
- `IsCatchTrayReadySupplyNewTray()` 的授權（`iCatchTrayControlManual=1`）只讓 TrayArm **去 Color/Empty 拿盤**（不在 Auto 走廊）——提早授權是刻意的**平行化設計**（OutArm 退讓同時 TrayArm 先備盤），**不需要也不應該**跟著收緊。
- **真正進 Auto 走廊的每一條路都必須過 `IsOutArmSafe()`**：DoPlaceTrayToAuto case 100、CatchNewTrayFromBuffer case 2101、取滿盤（acatchtray 8742/8892/9119）、cTrayMapping 2497、csystem 14904。修 `IsOutArmSafe()` 一點 = 堵住全部進入點。
- ⚠ 破功條件：若日後有新動作直接拿 `iCatchTrayControlManual` 當「可進 Auto 區」依據而不問 `IsOutArmSafe()`，防線即破。新增 CatchTray/TrayArm 進走廊動作時必查。
- 註：`IsCatchTrayReadySupplyNewTray` 第 4 分支（kevin 20180712）`iPosY>=iOutArmSafeY && fCMD==false` 本來就是正確錨點，與 KaiHuang 20200811（TrayMapping 分支錨 iOutArmSafeY）互為先例。

**`IsOutArmSafe()` 的 config 分支與容差**（`acatchtray.cpp` L3759-3788）：
```
TRAY_ARM_MODE==eUnderCoveyor                                   → return true（直接放行）
INSTALL_OCR!=eocrUninstal                                      → iPosY >= YOutArm_Shuttle1_Pick[0][2]        （容差 0）
USE_TRAY_MAPPING==1 && Y_PITCH==Variable/Bb                    → iPosY >= iOutArmSafeY                       （容差 0）
Cylinder[C_TrayCover].Enable || USE_TRAY_MAPPING==etmInstall   → iPosY >= YOutArm_Shuttle1_Pick[0][2]-1900   （容差 -1900）
else                                                           → iPosY >= YOutArm_Shuttle1_Pick[0][2]-3500   （容差 -3500，最寬）
```
> `YOutArm_Shuttle1_Pick[0][2]` 與 `iOutArmSafeY` 經 `cinitial.cpp` L9519-9530 全陣列填同值（= `Tech.iOutArmShuttle1Y`），所以改 `[2]` 索引為變數是 no-op；兩個閘門其實錨在**同一個 base Y**。

> InArm 端有對應的查詢函式（`InArmXYZSafe()` / `IsMoveInArm2XYToWait()` / `InArmZInSafe()`），但**無**「設 control flag 主動授權」的對應；詳見 `ht9045-inarm-flow` §「InArm 安全互鎖判斷函式」。

### `IsOutArmSafe()` 的 −3500 容差到底在補什麼 — 撞機根因 + 修正

> ⚠ **撞機根因不是「base Y 不夠」，而是 `−3500` 容差對雙排模式太鬆 → TrayArm 提前放行**。
> （客戶 RogerYang 2026-06 釐清；ATK HT-9046LS 2x8 雙排案）

**容差在補的是「用哪一排吸料」造成的 Y 偏移**。OutArm 實際停的 Y ＝ `iOutArmSafeY ＋ (iActiveSuckY − iOutArmYBase)×iMovePitchY`（`aoutarm9045_2x8_8.cpp` L1870）。`iOutArmSafeY` 錨在 `iOutArmYBase`，所以：

| 模式 | 判斷變數 | 實際停位 vs base | 需要的容差 |
|------|----------|------------------|------------|
| **雙排 2x?** | `OutArmSuck.iPickRow==2` | ≈ base（偏移小） | **`−500` 即可** |
| **單排 1x? + 後排 Row0** | `iPickRow==1 && iOutArmYBase==0` | kit 在中間、偏後 ≈ `−pitch/2`（pitch=6000→≈3000） | **`−3500`（須保留）** |
| **單排 1x? + 前排 Row1** | `iPickRow==1 && iOutArmYBase==1` | ≈ base | **`−500` 即可** |

→ 現行 `else` 分支對**所有**模式硬套 `−3500`；對 ATK 這種雙排是「多放行 ~30mm」→ TrayArm 在 OutArm 還沒退夠時就動 → 撞 OutArm Z 管。

**判斷變數來源**（皆既有、不需新 teach）：
- `OutArmSuck.iPickRow`：吸料用幾排（`1`單/`2`雙），由 `SetPickerCount()` 設（`mykitsuck.cpp` L206，**只存數量、不含哪一排**）。
- `iOutArmYBase`：哪一排（`0`後排/`1`前排），依 `USE_IN_OUT_ARM_Y_PITCH` 在 `database.cpp` L807-952 設定；對照表見 `ht9045-sucker-architecture` §4.3.3。

#### ✅ 採用解（已上線）：保守離散 margin，目標「不撞」

ATK 案最終採用此版（`RogerYang 20260626`，在 `IsOutArmSafe()` 的 `else` 分支；單排後排用**客戶碼鎖**只動 ATK）：

```cpp
else
{
    int iMargin;                                                            //RogerYang 20260626 : ATK 雙排還沒到位 TrayArm 就過來,調整讓位 tolerance
    if(OutArmSuck.iPickRow==2)
        iMargin = 500;                                                      // 雙排:不需多讓(ATK 落這條)
    else if(iOutArmYBase==0)
        iMargin = (CUSTOMER_CODE==CC_AMKOR_Korea)?500:3500;                // 單排後排:只 ATK 收緊,其餘維持 3500(零回歸)
    else
        iMargin = 500;                                                      // 單排前排 Row1:不需多讓
    if(iPosY>=(Prod.YOutArm_Shuttle1_Pick[0][2]-iMargin) ||
       MOT[MOutArmY].CompareCommandPos(Prod.iOutArmSafeY, 2)==1)
        return true;
}
```

> `CC_AMKOR_Korea==971`（`MachineType.h` L342），= ATK 這台 State record 的 `CUSTOMER_CODE`，gate 正確生效。
> ⚠ **不對稱**：單排後排用客戶碼鎖（只 ATK），但**雙排 `iPickRow==2→500` 仍全客戶生效**。雙排只「收緊」、hang-safe，對別家最多略降 UPH；若要對其他客戶完全零更動，雙排那條也可同樣包 `CUSTOMER_CODE==CC_AMKOR_Korea`。

**為何縮 margin 不會 hang**（縮到任意小都成立）：
- `IsOutArmSafe()=false` 時 case 2101 走 else（`acatchtray.cpp` L2637-2656）：TrayArm 退到 `iXTrayColor`/`iXTrayEmpty` 等待軌、**放行 OutArm 繼續跑**（`fCanMove=true`）、break 重試 → OutArm 不被卡。
- `iCatchTrayControlManual=2`（L2624）逼 OutArm 狀態機 `MoveOutArmXY_ToFix_Tray_Full()` 退到 `iOutArmSafeY` → 兜底條件 `||CompareCommandPos(iOutArmSafeY,2)` 成立 → 放行（與位置 margin 無關）。
- 等待期間**不觸發 alarm**：此路徑 break 在呼叫 `AvoidOutArm`（L2659，含 10s timeout）之前。
- 代價：TrayArm 少了「OutArm 還在取料位就提早搶」的機會，改等 OutArm 退到 `iOutArmSafeY` → 略降 UPH，但有界、不死等。

**判斷變數來源**（皆既有、不需新 teach）：
- `OutArmSuck.iPickRow`：吸料用幾排（`1`單/`2`雙），由 `SetPickerCount()` 設（`mykitsuck.cpp` L206，**只存數量、不含哪一排**）。
- `iOutArmYBase`：哪一排（`0`後排/`1`前排），依 `USE_IN_OUT_ARM_Y_PITCH` 在 `database.cpp` L807-952 設定；對照表見 `ht9045-sucker-architecture` §4.3.3。

**為何這版安全（達成「不撞」且不破壞別台）**：
- **不會 hang**：保留 `|| CompareCommandPos(iOutArmSafeY,2)==1` → OutArm 一被命令退到位就 true，位置窗口收緊也不死等。
- **對其他機台低風險**：只把 `else` 分支雙排**收緊**（−3500→−500），收緊只讓 TrayArm 多等零點幾秒、不會製造新撞機。
- **單排後排維持 `3500`**：那些原本沒撞的單排模式零回歸。

**⚠ 必懂的數字落差：參考點是「裸陣列」，實際淨空 = `margin + ~3000`**

`YOutArm_Shuttle1_Pick[0][2] = Tech.iOutArmShuttle1Y`（`cinitial.cpp` L9519-9530，全陣列同值、**無 center**），但 ATK 是變間距，**實際取料位 ≈ 裸陣列 + 3000**（`iMovePitchY/2` 等項，算式見下「進階」）。所以雙排 `−500` 的**實際淨空 ≈ 距真實取料位 3500**：

| 版本 | 門檻（裸陣列−margin） | 距真實取料位（≈−47000） | 結果 |
|------|----------------------|--------------------------|------|
| 舊 `−3500` | ≈−53500 | **6500 短** | 撞 ❌ |
| 採用 `−500`（雙排） | ≈−50500 | **3500 短** | 比舊緊 3000，方向對 ✅（**須實機驗證掃不到**）|

→ 若實機仍擦到：**不必改架構，直接把雙排 `iMargin` 再調小**（往 0 收，手上還有 ~3000 裕度），CompareCommandPos 兜底不會 hang。

---

#### 🔧 進階／升級選項（精準解，暫不採用）

若日後要做到「真的只放 500」或遇到別的 recipe 不夠，再走這條：**參考點不要用裸陣列或 `iOutArmSafeY`（皆是常數/不含 recipe 偏移），改錨「實際算出的 Shuttle1 取料命令 Y」**。

- 真實取料 Y 是算出來的（`CheckOutArmXYPitch_*`）：`= YOutArm_Shuttle1_Pick[base] − iMovePitchY + 7000(socket) − dSiteYOffset/2 + dSiteYPitch/2 (±row A/B…)`，**隨 recipe/排數變**；單排後排那「+3000」其實是 `iMovePitchY/2`，含在這條算式裡，**不在裸陣列**。
- 作法：在 OutArm 算完 `iSht==0` 的最終 `iYPos` 處 cache 進全域（如 `iOutArmCurShuttle1PickY`），`IsOutArmSafe()` 比 `iPosY >= iOutArmCurShuttle1PickY - 500`。
- 好處：所有偏移（center、row ±`iMovePitchY/2`、7000、dSiteYOffset）由命令值自動含入，**連 `iPickRow`/`iOutArmYBase` 分支都可省**，全模式共用小 margin。
- 代價：共用互鎖（~13 呼叫點）耦合到取料 context、需 config-gate + 跨 recipe 驗證 → 故暫不採用。

> 參考點常數定義（備查）：
> `iOutArmSafeY = USE_OUT_Y_IS_AUTO_PITCH ? Shuttle1Y + iOutArmShtYCenterPos(≈+3000) : Shuttle1Y`（`cinitial.cpp` L9818/9820）；
> `USE_OUT_Y_IS_AUTO_PITCH` 條件（`database.cpp` L718）：`USE_OUT_ARM_Y_PITCH` ∈ {Variable,16Picker,Bb,16Bd_Be,In_Bb_Out_Bc}。ATK = Variable → true。
> ⚠ `USE_PICKER_COUNT` enum 陷阱：`ep4Picker=0, ep8Picker=1, ep2Picker=2, ep16Picker=3, ep1Picker=4`（`MachineType.h` L1364）——General.ini `USE_PICKER_COUNT=1` 是 **ep8Picker**，不是 1 Picker。

#### 📌 第二案鑑識（2026-07-10，ATK HT-9132 PPLS2160）：客戶更新後仍撞 → 機上 exe 沒帶到修正

同客戶（971）**不同機台**：HT-9132、`iInArmType=3=e9045_1x2_2_14`（**1x2 單排**）、NN 2x2 site、Variable pitch。State record（`D:\!Korea\!ATK\20260711...\2026-07-10 15_25_30`）完整還原：

| 證據 | 值 |
|------|-----|
| EventLog | 15:24:50.856 TrayArm 從 Color 取盤 → **15:24:51.567 操作員按 PAUSE**（0.7 秒後）→ 15:24:55 開門 → 15:25:30 State Record |
| `TrayArmPlaceTrayToAutoTask` | **50.870 進 case 100 → 50.874 過閘到 200（4ms 一次就過）**→ 210/250 出發 |
| MTrayX teach / 凍結位 | Empty=31600, Color=63170, Auto1=95360, Auto2=113770；**凍結在 102160＝已衝過 Auto1** |
| MOutArmY | Current **−51332**、Target −47611（=iOutArmSafeY）→ 距退讓目標差 37mm；放行當下更在 ≈−53240（放料搜尋位） |
| MOutArmX | Current −48265、Target −53601（X speed=800 很慢，幾乎沒動） |

**判讀（更正版；exe 經 RogerYang 確認含 20260626 修正）**：
- teach 實值：`setEditOutSht1Y=−50611`（=bare）、`setOutPickY=−57624`（Shuttle1 取料 Y）、`setEditAuto1Y=−63950`、iOutArmSafeY=−47611（bare+center3000, Variable）。
- 時序重建：48.15 OutArm 進 3010 下退讓令（ContinuousMove 下令即回），Y 自 −57624 以 ~2720/s 爬升 → ~50.54 跨過**修正後門檻 −51111（bare−500）** → 50.874 位置窗口**合法成立、4ms 過閘**。
- **放行當下 Y≈−50226，距真正退讓目標 −47611 還差 ~26mm；X 還懸在 Auto1 正上方（Auto1X=−50842）且吸嘴掛著 2 顆 IC** → TrayArm 0.7 秒衝入 → 干涉。
- ⚠ 凍結馬達值（Y=−51332 低於目標）**不可信**：54.7 安全門已開、15:25:30 才存 record，中間操作員徒手撥臂 —— 這解釋了「Y 比目標低」的運動學矛盾。分析 state record 時**開門後的凍結位置一律存疑**。

**結論：修正照設計運作，但 bare 錨點的放行線本來就提早 3500（35mm）＝「參考點陷阱」被第二台實證**。HT-9046LS（第一台）機構裕度吃得下 35mm；HT-9132（1x2、TrayArm 升降搬盤）吃不下。margin 數值調整已到極限，**必須改錨真正退讓目標**。

**✅ 最終出貨版（V3.33.908.5，RogerYang 20260711 + JerryYang 20260717，已審核通過）**：
```cpp
bool IsOutArmSafe()
{
    int iPosY  =MOT[MOutArmY].ReadPos();                                    //命令計數器
    int iEncPosY=MOT[MOutArmY].ReadEncoderPos();                            //JerryYang 20260717 : encoder 保護
    int iMargin=500;
    int iEncGuard=(Prod.iOutArmSafeY<Prod.YOutArm_Shuttle1_Pick[0][2])?    //★min(bare,safeY):ep1Picker(center=-3000)
            Prod.iOutArmSafeY:Prod.YOutArm_Shuttle1_Pick[0][2];             //  停點在 teach 下方,guard 跟著停點走,勿擋

    // eUnderCoveyor → true
    // OCR 分支      : enc>=iEncGuard-500 && (cmd到位 || iPosY>=bare)
    // KaiHuang 分支 : 未動（TrayMapping+Variable/Bb，iPosY>=safeY，無 encoder guard）
    // TrayCover 分支: enc>=iEncGuard-500 && (iPosY>=bare-500 || cmd到位)   ←由 -1900 收緊
    // else{
    //   ATK 單排(iPickRow<=1 && CC_AMKOR_Korea):
    //        enc>=iEncGuard-500 && (iPosY>=safeY-500 || cmd到位)          ←★HT-9132 cure:cmd 窗口錨真停點
    //   其他(含 ATK 雙排):
    //        enc>=iEncGuard-500 && (iPosY>=bare-500 || cmd到位)           ←HT-9046LS 行為同 906.5
    // }
}
```
**設計要點（審核三輪的結晶）**：
1. **encoder guard 為 AND 前置**：`ReadPos()`/`CompareCommandPos` 只看命令計數器——堵轉時命令走完、實際卡走廊照樣放行（競態於 following-error alarm 前）。先例：Steven 20231214 `PCIL112_OutArmXYMove` Command→Encoder（就是 OutArm XY 軸）；InArm `IsMoveInArm2XYToWait()` encoder(±9)+command(±2) 雙確認。
2. **encoder 窗口必須留 margin（−500）**：encoder 靜止有 ±9 系統偏差（InArm 前例：Steven 20250605 gap 2→9）。審核第一輪抓到 OCR 分支零容差 `>=bare` → 固定間距機台停妥也開不了門 → hang。
3. **guard 錨必須 min(bare, safeY)**：審核第二輪抓到 guard 錨死 bare 時，ep1Picker（`USE_PICKER_COUNT==ep1Picker=4`，`iOutArmShtYCenterPos=-3000`，database.cpp L817）停點=bare−3000 在 guard 線下方 → 停在正確退讓位也永遠 false → hang（舊碼靠 `||CompareCommandPos` 活路，AND 把它堵死）。min() 讓 guard 跟著停點走，**不需枚舉危險機型、未來新配置也涵蓋**。注意：風險條件是「停點<guard 線」（center 為負+auto-pitch），**不是**單排——單排的深位置是取料位（gate 關閉是正確等待），讓位停點永遠是 iOutArmSafeY。
4. **ATK 單排 cmd 窗口錨 `safeY−500`**（放行時差 5mm）；其他分支維持 bare 錨（HT-9046LS 雙排實證可用，等效 safeY−3500）。
5. **知情的全域收緊**（無 gate，記入 release note）：TrayCover 分支 −1900→−500；其他客戶單排 Row0 3500→500（舊 margin 表註解保留於碼中）。
6. 遺留可接受項：KaiHuang 分支未加 encoder guard（不一致但無案例）；堵轉在最後 ~35mm 窗口由 following-error alarm 兜底。
6b. **模擬模式（SOFT_SIMULTE）不受影響**：卡未開（`Open_SMCCard` 被 `#ifndef` 編掉）→ `Motor->Enable=false` → `TMyMotor::ReadEncoderPos()` 走 else 路徑 `EncoderPosition=Position`（encoder 鏡射命令，mymotor.cpp L434-447，JimmyChiu 20250306 加的保護）；模擬運動引擎逐步推 `Position`（MotorMove `#else` 段 L824-859）→ encoder guard 在模擬中等價命令判斷，不 hang。先例：InArm 雙確認/`PCIL112_OutArmXYMove` CompareEncoderPos 在模擬跑多年無事。
6c. **encoder 判斷與 `#ifdef SOFT_SIMULTE` split**（對照 `InShtInLF`/`InShtInRT`，csystem.cpp L365；經 2023/2026-04/現行三版比對＋RogerYang 親歷驗證）：
   - **鏡射保護自 2023（V3.32.811）以前就存在**：`TMyMotor::ReadEncoderPos()` 在 `Motor->Enable==false` 時回 `EncoderPosition=Position`（JimmyChiu 20250306 僅補 `Motor!=NULL`）；且模擬版三種軸卡都強制 `Enable=false`（cinitial `#ifdef SOFT_SIMULTE`）→ **模擬讀 encoder 永遠拿到命令值（鏡射），不是垃圾**。
   - **實測事實**：20260410 實機側升級 `CompareEncoderPos(±9)`（當時無 split）→ **軟體模擬實測 hang** → 補上 split（模擬側走 `CompareCommandPos(±2)`）後修復。
   - **機制分析（最合理解釋，精確機制待重現定位）**：鏡射下模擬的 encoder 比對 ≡ 命令比對，±9 嚴格比 ±2 寬鬆 → 「等不到 true」在數學上不可能；唯一行為差異是**判定窗口變寬 → 假陽性**——shuttle 尚未被命令去 Left 時，模擬的 `Position`（從 0/home 起跳、與開發機 teach 值距離跟實機不同）恰落在 `iLeft±9` 內（±2 排除得掉的區間）→ `InShtInLF()` 過早回 true → 流程以為已到位、跳過移動/走錯分支 → 下游互等 hang。**split 修的實質是容差寬度（±9→±2），不是 encoder/command 之別**。
   - 定位實驗（下次重現時）：拿掉 split，hang 當下 log `iInPos / ReadEncoderPos() / iLeft`——一直 false（距離遠）＝鏡射分析有洞；不該 true 時 true＝假陽性機制確認。
   - 實務準則：**到位判斷（OR 型放行）在模擬側維持窄容差的命令比對（split）是實證安全的做法**；AND 型攔截 guard（如 `IsOutArmSafe`）在模擬中恆不擋、無假陽性放行面 → 不需 split。
7. `IsCatchTrayReadySupplyNewTray()` **不需跟改**（授權≠進走廊，見「分工鐵律」）。
- ⚠ 分析注意：Motor.xls / MotorView「Current」欄 = `ReadPos()` **命令側**（Check Encoder 勾選框可切換）——讀 state record 凍結位置勿當實際位置；**開門後的凍結值可能被徒手撥動**（HT-9132 案教訓）。
- X 條件（OutArm X 是否離開 Auto 區）留待 HT-9132 驗證仍擦到再加。

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

---

> **以下為 Roger 維護的額外章節（原始版本保留）**

## Local Preserved Notes (from previous local agent)

HT9045 IC Test Handler OutArm ???????????? OutArm?Output Arm?
  Shuttle ???Unloader Tray ???Auto Tray ???Fix Tray ???Bin ???
  Magazine????Sucker???????Vacuum Suck??Destroy ??????IC Fall Down??
  Pick Error?Retry/Skip/Home?AOI?Rotator?Fix AI CCD?Clean Out ???BinBox ?
  ???????????????? OutArm ???????
  ????DoOutArm, DoOutArm_9045, OutArmSuck, OutArmTask, iInArmType, Shuttle,
  DoOutArmRotateKIT, DoAOIFunction, DoFix2AICCDFunction, Destroy??, Fix3FullTray,
  JAM0217, Yield??, P27, iBinBoxSelect, iAMRFixTraySelect, ?????,
  iPickFromShuttle2Task, OutArmPickShuttleAlarm, OldPos, 32Site?
## ????
??????????????????
- OutArm / Output Arm ??????????case ????
- Shuttle ???Pick from Shuttle 1 / Shuttle 2?
- Unloader Tray ???Place to Auto Tray / Fix Tray?
- Bin ??? Tray ???SearchTrayToPlace, CheckBin?
- Magazine ?????Magazine Tray / Buffer?
- ?????Rotator / AOI / Fix AI CCD?
- ???????Retry / Skip / Home?
- IC ???Fall Down??Destroy ????
- Clean Out ???Auto Sorting BinTray?
- OutArm sucker ?????iInArmType?
## ?????
| ?? | ???? | Task ?? |
| `aoutarm.cpp` | `DoOutArm()` ? ????? | ?? switch?|
| `aoutarm.cpp` | `CheckOutArmCleanOut()` ? Clean Out ?? | ?? switch?|
| `aoutarm.cpp` | `DoOutArmPlaceToAuto()` ? Destroy ???? | ??????|
| `aoutarm9045.cpp` | `DoOutArm_9045()` ? Dispatch | ?Dispatch?|
| `aoutarm9045.cpp` | `DoOutArmPlaceToAuto_9045()` ? ???? | `iPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAfterPlaceToAuto()` ? ????? | `iDoOutArmAfterPlaceToAutoTask` |
| `aoutarm9045.cpp` | `DoOutArmAdditionalFunction()` ? ???? | `iOutArmAdditionalFunctionTask` |
| `aoutarm9045_2x8_8.cpp` | `DoOutArm_9045_2x8_8()` ? ???? | `OutArmTask` |
| `aoutarm9045_2x8_8.cpp` | `DoPickFromShuttle_9045_2x8_8()` ? Shuttle ?? | `iPickFromShuttle1/2Task` |
## ??????
  ?? DoOutArm_9045()                               <- aoutarm9045.cpp (Dispatch)
       ?? DoOutArm_9045_2x8_8() [??? iInArmType]
            ?? case 1200/2200: DoPickFromShuttle_9045_2x8_8(iSht)
            ?     ?? case 200:  MoveOutArmToShuttleIncludeZ
            ?     ?? case iOUTARM_SUCK: Suck() ?? IC
            ?     ?? case 2000: Error Retry/Skip/Home
            ?? case 7000: DoOutArmAdditionalFunction()
            ?     ?? DoOutArmRotateKIT()
            ?     ?? DoAOIFunction()
            ?     ?? DoFix2AICCDFunction()
            ?? case 3010: SearchTrayToPlace_9045()
            ?? case 3310: DoOutArmPlaceToAuto_9045()
            ?     ?? DoOutArmPlaceToAuto(iWhichAuto)
            ?? case 3500: DoOutArmAfterPlaceToAuto()
                  ?? case 1000: DoFix3FullTray()
                  ?? case 5000: DoSortingBinTray()
                  ?? case 11100: DoPickFromMagazineBuffer()
## ????�1?�5?? Shuttle Pick
> ????? case ??iInArmType ????Suck ???Error ??????
### ????
| ?? | ?? |
| `DoOutArm()` | ? NULL IC ???Shuttle ??????? Dispatch |
| `DoOutArm_9045()` | Guard Checks?Servo Off / Destroy Active / QA Mode?+ iInArmType ?? |
| `DoOutArm_9045_2x8_8()` | SHT1/SHT2 ???Pick Flow ????case 1000/2000? |
| `DoPickFromShuttle_9045_2x8_8()` | XY/Z ?? ? Suck ? Retry/Skip/Home |
## AOI ???????�AOI?
> ?? AOI ????4 ????Fail Bin ???RS232/TCP ????? Fail ???
> [references/outarm-aoi.md](references/outarm-aoi.md)
### ????
AOI ?? **Pick from Shuttle ???Place to Auto/Fix ??**?? `DoOutArmAdditionalFunction()` ???case 20000??

### 4 ? AOI ??

| ?? | ???? | ?? | DoAOIFunction case |
|------|---------|------|-------------------|
| Vitrox BGA View | `USE_AOI_Inspection==1` + `tBGAView.bEnabled` | IO ?? | 1000 |
| Vitrox PAD View | `USE_AOI_Inspection==1` + `tPADView.bEnabled` | IO ?? | 2000 |
| Scanner AOI (Bottom) | `Scanner_AOI==1` + `iEnableScannerMode!=0` | RS-232 | 6000 |
| Top & Bottom Inspect | `Scanner_AOI==2` + `ttbInsp->iEnable==1` | TCP/IP | 7000 |

### AOI Fail ? Bin ??

- **???** `iAOIFailBinType==0`?`iBin = ScannerIfError + 4`???? Fix?
- **???** `iAOIFailBinType==1`???? Pass/Fail ????? Fix
- ?????`OutArmSuck.iWhichAuto[iRow][iCol] = GetAOIFailBin(iRow, iCol)`
- ?? `SearchTrayToPlace_9045()` ?? `iWhichAuto` ???? Tray

### ????

| ?? | ?? |
|------|------|
| `DoAOIFunction()` | AOI ????`fAOI.cpp`? |
| `DoScanAOIFunction()` | Scanner AOI ???? |
| `DoScanAOIFunction_Inspection()` | Scanner AOI ???? + RS232 ?? |
| `TTopBottomInspect::DoTopBtmInspFunc()` | Top & Bottom TCP/IP ??? |
| `GetAOIFailBin()` | Fail Bin ?????/???? |
| `TriggerAOISystem()` | RS232 ???? |
| `PreSetOutAdditionalFlag()` | Pick ??? `bAlreadyAOI` |

### ???

- ?????`Gerneral.ini` ? `[System] AOI` / `Scanner_AOI` / `Top_Scanner_AOI`
- ????`{DataPath}{Recipe}\AOI.Data` ? `[SETTING]` section
## ????�6��9?Place to Tray
> ?? case?Destroy ???BinCT ???AfterPlace ???
> [references/outarm-place.md](references/outarm-place.md)
### ????
| ?? | ?? |
|---|---|
| `DoOutArmAdditionalFunction()` | Rotator / AOI / Fix AI CCD ???? |
| `DoOutArmPlaceToAuto_9045()` | Tray ?? ? Offset ? Destroy ?? ? ?? ? Tray ?? |
| `DoOutArmPlaceToAuto()` | ?? Destroy ?? + BinCT / ART ?? + Lot Summary |
| `DoOutArmAfterPlaceToAuto()` | ???????? 100/110/3010/5000/11100? |
## Bin ?? / CleanOut?�10��13?
> ?? CheckOutArmCleanOut ????SearchTrayToPlace ???OutArm vs InArm ????FAQ ??
### ????
| ?? | ?? |
| `CheckOutArmCleanOut()` | ?? 0/310/5000/11100 ?????? CleanOut ?? |
| `SearchTrayToPlace_9045()` | ? BIN ?????? IC ? Auto/Fix Tray ?? |
| `DoSortingBinTray()` | Clean Out ?????? Tray ? ?? ? ???|

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
- [references/colleague-DoOutArm_9045_FlowChart.md](references/colleague-DoOutArm_9045_FlowChart.md)：repo 既有參考檔（同事整理）
- [references/colleague-DoOutArm_FlowChart.md](references/colleague-DoOutArm_FlowChart.md)：repo 既有參考檔（同事整理）
- [references/colleague-SingleSiteOtherSuck-Flag-Boundary.md](references/colleague-SingleSiteOtherSuck-Flag-Boundary.md)：repo 既有參考檔（同事整理）
- [references/colleague-only-20260915.md](references/colleague-only-20260915.md)：repo 既有參考檔（同事整理）
- [references/colleague-outarm-bin.md](references/colleague-outarm-bin.md)：repo 既有參考檔（同事整理）
- [references/colleague-outarm-pick.md](references/colleague-outarm-pick.md)：repo 既有參考檔（同事整理）
- [references/colleague-outarm-place.md](references/colleague-outarm-place.md)：repo 既有參考檔（同事整理）
- [references/state-machine.md](references/state-machine.md)：repo 既有參考檔（同事整理）
