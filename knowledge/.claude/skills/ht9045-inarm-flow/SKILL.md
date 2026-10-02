---
name: ht9045-inarm-flow
description: HT9045 IC Test Handler InArm 流程知識庫。當使用者詢問 InArm、Input Arm、Loader 取料、HotPlate 放料/取料、Shuttle 放料、吸嘴（Sucker）、真空吸取（Vacuum Suck）、Destroy 吹氣、掉料（IC Fall Down）、黏貨（Sticky）、Floating 浮起、Auto Clean、Tray End、Clean Out、Pick Error、Retry/Skip/Home、Auto Site Mapping、Precisor、Rotator、Die Clean、Bottom 2DID 等相關問題時，應先載入此技能以理解 InArm 完整處理流程。關鍵字：DoInArm, DoInArm_9045, InArmSuck, iArmTask, iInArmType, Shuttle, HotPlate, Loader, AutoClean, PickFromHPList, PickHPRec.json, DataForwardAndNextTeam, bTripASMHPBorrow, HasHotReadyIC_9045, 幽靈帳, 幽靈團, 帳沒銷, case 350, bDutOnOffNeedASM, 暫停中關站切模式。 另含 **熱盤落點幾何鐵律**：CheckHasSpaceToPlace_9045, PlactCT, SearchPlacePlateXItem8_8Suck, CheckHotPlateHasSpace_9045_8_New_V, 2x4 落點, 熱盤碎片化, 熱盤孤島, HAS_NULL_IC 占位, WAR0150, 熱盤接近滿, iYHalf, bPitchOver12000, Task=500 逃生出口。 分工提示（repo）— 吸取判定細節（真空／HAS_NULL_IC／HAS_TRY_SUCK_IC／TMySucker::Suck／HotPlate Data Swap／座標偏差檢測 E74／16-site 吸嘴映射）另有 ht9045-inarm-suck-logic 可查。
---

<!-- AI(W906-BA-SKILL) 20260915：正文換成網頁同事 20260915 那版（較新／較完整）。
     frontmatter 的路由描述保留我們的（★ description 已收斂到流程層，與 ht9045-inarm-suck-logic 分工）。
     我們原有但他沒有的段落，另存 references/ours-kept-20260915.md。 -->


# HT9045 InArm Flow Knowledge

## 適用場景

- InArm / Input Arm 的動作流程、狀態機、case 值意義
- Loader Tray 吸取 / HotPlate 放料取料 / Shuttle 放料
- 附加功能（Precisor / Rotator / Die Clean / Bottom 2DID）
- 吸取異常處理（Retry / Skip / Home / Tray End / Clean Out）
- IC 掉料、黏貨、浮起檢查
- InArm sucker 幾何配置（iInArmType）

## 吸嘴物理排列（8-Sucker 模式）

```
Row 0:  A   C   E   G      ←  InArmSuck.Item[0][0] ~ Item[0][3]
Row 1:  B   D   F   H      ←  InArmSuck.Item[1][0] ~ Item[1][3]
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

### Variable Pitch 模式

| 模式名稱 | 使用吸嘴 | HP 放料間距 | 最大跨距 |
|----------|----------|------------|----------|
| 1-3 (A-E) | A,B,E,F (j=0,j=2) | j*2 (每隔1格) | XPitch×4 |
| 1-4 (A-G) | A,B,G,H (j=0,j=3) | j*3 (每隔2格) 或 j*1 (連續) | XPitch×6 或 XPitch×3 |

> **注意**：「1-3」「1-4」是吸嘴物理編號（1-based），非陣列索引。
> 2x2 模式每次取放 4 顆 IC（Row0+Row1 各 2 顆），不是 8 顆。

### iInArmType → 吸嘴選擇路由（重要）

修改 HP 搜尋/放料邏輯前，**必須先確認目標機台的 iInArmType**。
同一 TestMode (如 2x4) 可能對應不同 iInArmType（如 e9045_2x2_4_13），
取決於 SitePitch、HP XDiv 等參數。詳見 [HP_Knowledgebase.md §3.5](references/HP_Knowledgebase.md)。

| 函式 | 吸嘴 | jStep | 代表 iInArmType |
|------|------|-------|-----------------|
| `bUseAxExPicker()` | A,E | 2 | _13 系列 |
| `bUseAxxGPicker()` | A,G | 3 | _14 系列 |
| `bUseACEGPicker()` | A,C,E,G | 1 | 8-picker 系列（含 2x2_8_Hot） |

> **教訓**：切換吸嘴模式（如 AE→AG）影響 jStep、Loader 取料座標、Shuttle 映射表等全鏈路。
> 必須從 `iInArmType` 決策入口（§4 DoInArm_9045_Type）直接判斷，下游才會自動正確。

### 2x2 模式 iInArmType 決策條件（QualSite2X2 Hot 路徑）

| 優先序 | 條件 | iInArmType | 吸嘴 | 說明 |
|--------|------|------------|------|------|
| 1 | `dSiteXPitch > iXpitchMaxX2_MM` | _14 | A,G | Site Pitch 超過 80mm |
| 2 | `HP XPitch > iXpitchMaxX2/3+1` 且 `HP XItem==6` | _14 | A,G | 6 格 HP Pitch>26.66mm（20250827 RogerYang）|
| 3 | `HP XItem==12` 且 `HP XPitch*6 > iXpitchMaxX2` | _14 | A,G | **12 格 HP 跨距 >80mm（20260512 RogerYang）** |
| 4 | 以上皆不符 | _13 | A,E | 預設 AxEx |

> **案例**：偉測 PPLD 12×20 HP（XP=1700, YP=1800）→ XP×6=10200>8000 → 走 _14（AxxG）→ VP=10200 安全 ✓
> 若錯走 _13（AxEx）→ VP=XP×9=15300 → bPitchOver12000=true → 搜尋退化 → WAR0150

### InArm 動作方向

- **取料來源**：Loader Tray、HotPlate（HP 模式回取）
- **放料目標**：HotPlate（HP 模式）、Shuttle（最終放料）
- 正常生產模式下 InArm **不會**從 Shuttle 取料（Shuttle 是放料端）
- **例外**：Auto Site Mapping 模式、AutoClean 模式時 InArm 會從 Shuttle 取料

## InArm 基準軸（取放料計算原點）

**基準軸 = `InArmSuck.Suck[iInArmYBase][iInArmXBase]`** — 所有 Teach 點位（`Tech.iInArmLoadStageX/Y/Z2`, `iInArmPlate1X/Y`...）儲存的是「基準軸吸嘴」的馬達座標。其他 7 顆吸嘴透過基準軸 + Pitch 公式 + `iInArmZHeightSub` 高差陣列推算。

### 依 `USE_IN_OUT_ARM_Y_PITCH` 切換

| Y-Pitch 模式 | 值 | `iInArmYBase` | `iInArmXBase` | InArm 基準吸嘴 |
|---|---|---|---|---|
| `iXPitch60` (Fixed 60mm) | 0 | 0 | 2 | **E** [0,2] |
| `iXPitchManual635` (60/63.5mm) | 1 | 0 | 2 | **E** [0,2] |
| `iXYPitchVariable` (可變 X+Y) | 2 | **1** | 2 | **F** [1,2] |
| `iXPitchManual360` (60/36mm) | 3 | 0 | 2 | **E** [0,2] |
| `iXYPitchRowA` (Row A 可變) | 4 | 0 | 2 | **E** [0,2] |

> 可變模式選 F[1,2] 是因為 YP 馬達固定端在 Row 1，基準必須選在 YP 不會帶動的那一側。
> OutArm 基準軸為鏡像對稱版（可變模式 `iOutArmXBase=1` → D[1,1]），詳見 [ht9045-outarm-flow](file:///d:\HT9045\.github\skills\ht9045-outarm-flow\SKILL.md)。
> 完整推導：[ht9045-sucker-architecture §4.3-4.5](file:///d:\HT9045\.github\skills\ht9045-sucker-architecture\SKILL.md)、[references/InArm_TMyKitSuck_and_TypeDecision_20260513.md §11-§17](references/InArm_TMyKitSuck_and_TypeDecision_20260513.md)。

### Loader 取料公式（多吸嘴版，`ainarm9045.cpp` L4660）

```cpp
iXPos = Prod.XInArm_Tray_Pick[iInArmYBase][iInArmXBase]  //基準軸 Teach 位置
      + iCol * Prod.LoadForm.iXPitch                      //Tray IC 跨距
      + iInArmXBase * dInArmXPitch_1Step;                 //基準軸→第0吸嘴的 Pitch 偏移
iYPos = Prod.YInArm_Tray_Pick[iInArmYBase][iInArmXBase]
      - iRow * Prod.LoadForm.iYPitch;
iXPos += InArmOffSet[InOfsLoader]->GetArmX(i, j);         //個別吸嘴 Offset（Eastsun 20251231）
```

### X-Pitch 單步距離（`ainarm9045.cpp` L4476-4610）

```cpp
InArmClose_PitchX     = UserDefForm[Ld].XPitch * iInArmXStep;  //對齊 Tray
dInArmXPitch_1Step    = InArmClose_PitchX / 3.0;               //4 吸嘴模式
dInArmXPitch_MovePitch = dInArmXPitch_1Step * 3.0;             //= InArmClose_PitchX
```

## 關鍵原始檔

| 檔案 | 主要函式 | Task 變數 |
|------|---------|-----------|
| `ainarm2.cpp` | `DoInArm()` 最上層入口 | — |
| `ainarm9045.cpp` | `DoInArm_9045()` Dispatch；`DoInArmPickFromLoadStage_9045()`；`DoInArmAdditionalFunction()`；`DoInArm_9045_Type()`；`CheckPickerMode()` | `iPickFromLoadStageTask`、`iInArmAdditionalFunctionTask` |
| `ainarm9045_2x8_8.cpp` | `DoInArm_9045_2x8_8()`；`DoPlaceToHotPlate_9045_2x8_8()`；`DoInArmPlaceToShuttle_9045_2x8_8()` | `iArmTask`、`iInArmPlaceToHotPlateTask`、`iInArmPlaceToShuttleTask` |
| `ainarm_SearchPlacePlate.cpp` | HP 放料搜尋全部核心函式（`SearchPlateToPlace`、`CheckHasSpaceToPlace_9045`、`DoPlaceToHotPlate_9045` dispatcher 等）| — |
| `ainarm_SearchPickPlate.cpp` | `DoInArmPickFromHotPlate_9045()`；`HasHotReadyIC_9045()`；`SearchPlateToPick()`；`HotplateDataConversion()` | `iInArmPickFromHotPlateTask` |
| `mykitsuck.h / .cpp` | `TMyKitSuck` class；`SetPickerCount()` | — |

> 其他 `ainarm9045_*.cpp` 結構與 `_2x8_8` 相同，僅 sucker 幾何不同。

## 參考文件

| 文件 | 內容簡述 |
|------|---------|
| [DoInArm_FlowChart.md](references/DoInArm_FlowChart.md) | InArm 入口層流程 |
| [DoInArm_9045_FlowChart.md](references/DoInArm_9045_FlowChart.md) | 完整 case-by-case 狀態機（所有 iInArmType） |
| [InArm_TMyKitSuck_and_TypeDecision.md](references/InArm_TMyKitSuck_and_TypeDecision.md) | **TMyKitSuck 成員、SetPickerCount、iInArmType 決策樹、CheckPickerMode、iXStep、iWhichSht/Kit** |
| [HP_Knowledgebase.md](references/HP_Knowledgebase.md) | **HotPlate 完整知識庫**：SearchPlateToPlace、CheckHasSpaceToPlace、DoPlaceToHotPlate dispatcher、PickFromHPList、iHotCount、WAR0150/0151、JAM0109/0159、奇數 YDiv 幽靈格位 Bug |
| [HP_SearchPlacePlate_AllFunctions.md](references/HP_SearchPlacePlate_AllFunctions.md) | **全 17 個 SearchPlacePlateXItem 函式 + _1Suck 詳細分析 + HP 配置規格表**：路徑圖、掃描邏輯、馬達座標公式、適用 XDiv、XDiv=1 可達性、生產模式×XDiv 矩陣、關鍵變數速查（原 HP_1Suck_PathChart.md 已整合） |
| [HP_VariablePitch_Reference.md](references/HP_VariablePitch_Reference.md) | **HP Variable Pitch 完整參考表**：三函式 Pitch 公式對照（AxEx/AxxG/ACEG）、bPitchOver12000 判定、GetPlaceToHotPlateCol ix 計算、SearchPlacePlate spacX/spacY 計算、常見 XDiv×XPitch 速查表、修改前交叉驗證清單 |
| [SearchPlacePlate_Route_Proposal.md](references/SearchPlacePlate_Route_Proposal.md) | **XDiv=2/3/4/6/8/10/12/16 全路徑圖總表**：各 iInArmType 在不同 XDiv 下的吸嘴落點視覺化（JerryYang 提案，2026-04-07） |
| [HP_PlateForm.xlsx](references/HP_PlateForm.xlsx) | HP 格板規格表（Excel）：各機型 XDiv/YDiv 配置、吸嘴覆蓋範圍 |
| HP_REF-001 ~ REF-005 | 歷次 HP 除錯修改紀錄（Race Condition、HasIC Fix、Debug Log） |
| [HP_Item_NULL_IC_RaceCondition.md](references/HP_Item_NULL_IC_RaceCondition.md) | **HP `Item[][]` Race Condition 家族整合檔**：整合原 HP_REF-001/002/003/004，含根本原因、2 處修正案例、Debug Log 提案、同類風險點清單（2026-05-25 整合） |
| [HP_REF-005_Step400_HasIC_Fix.md](references/HP_REF-005_Step400_HasIC_Fix.md) | Step 400 `GetPlaceToHotPlateSuckCol` 對 Col2 回傳重複 j2 → IC 靜默丟失（獨立 Bug，與 Race Condition 家族不同） |
| [AutoClean_OOB_Bug.md](references/AutoClean_OOB_Bug.md) | Auto Clean `bUseTestSocket` 陣列越界案例（V3.33.889→904.0） |
| [SingleSiteOtherSuck-Flag-Semantics.md](references/SingleSiteOtherSuck-Flag-Semantics.md) | `bSingleUseOtherSuck` vs `bSingleInArmUseOtherSuck` Flag 語意 + AutoClean 一致性修正（2026-04-13） |
| [AutoClean_OOB_Bug.md](references/AutoClean_OOB_Bug.md) | **Auto Clean bInArmSuckActive Array OOB Bug**：`bUse8Picker=false` 時 A 排 Z 軸異常下降、記憶體佈局、修正方式、核心函式速查、bUse8Picker 判斷規則、EventLog 診斷（P260428-ATC-H9-01） |
| [SingleSiteOtherSuck-Flag-Semantics.md](references/SingleSiteOtherSuck-Flag-Semantics.md) | **Flag 語意**：`bSingleUseOtherSuck` vs `bSingleInArmUseOtherSuck` 差異 + §12 AutoClean 單站吸嘴一致性修正（2026-04-13） |
| [InArm_Debug_Methodology.md](references/InArm_Debug_Methodology.md) | **除錯方法論**：8-Picker 模式不生效根因（配置標誌 gate）、ChangeUseSuckMode 降級追蹤、VTEST 客戶函數完整性檢查清單（案例 2026-03-27） |
| [AutoClean_TwoArmSimultaneous.md](../ht9045-index-flow/references/AutoClean_TwoArmSimultaneous.md) | **NN 模式 Auto Clean 兩臂同動（ACSim）**：Index 側逐臂取料 + 同時下壓 + 平行放回；InArm 依序補 FL/BLCarryKit，ACSim 在 case 4010/4015 逐座鎖 Shuttle（60s timeout 降級）（隸屬 ht9045-index-flow） |

---

## 1. 呼叫階層

```
DoInArm()                           ainarm2.cpp — Guard checks → DoInArm_9045()
  └─ DoInArm_9045()                 ainarm9045.cpp — Dispatch by iInArmType
       └─ DoInArm_9045_XxY_Z()      ainarm9045_XxY_Z.cpp — 主狀態機 iArmTask
            ├─ case 100:  DoInArmPickFromLoadStage_9045()
            ├─ case 400:  DoInArmAdditionalFunction()
            ├─ case 1100: DoPlaceToHotPlate_9045()       → Hot 模式放料
            ├─ case 1500: DoInArmPickFromHotPlate_9045() → Hot 模式取料
            └─ case 2000: DoInArmPlaceToShuttle_9045()
```

---

## 2. DoInArm() — 入口 Guard Checks (ainarm2.cpp ~1588)

1. `bInitialStartIndexCheckDone == false` → return
2. `iHPHangUpCount != 0` → WAR0150 → return
3. `bF16CheckShuttleSensorBroken && bDoingF16` → return
4. **QA Mode** — `Check_QA_ModeCount()` → 設定 `HAS_NULL_IC`
5. **Auto Alignment CCD** — Loader 需求檢查
6. **IndexJam** — `bIndexJam` 檢查
7. 通過 → 呼叫 `DoInArm_9045()`

---

## 3. DoInArm_9045() — Dispatch (ainarm9045.cpp ~3907)

**Pre-Dispatch Guard（依序）**:
1. AUTO_ALIGNMENT_CCD running → return
2. AutoSiteMappingUseHotPlate → set `bRunAutoSiteMapping`
3. TestingNeedStopAllMotor → `StopAllMotor()` → return
4. bAlarmNeedServoOff && bMyServoOffInArm → return
5. CheckInArmDestroyActive() active → return
6. iPauseBackUp != -1 → return
7. bResetInArmTask → `InitInArmTask()` → return

**iInArmType Dispatch Table**（詳見 [DoInArm_9045_FlowChart.md](references/DoInArm_9045_FlowChart.md)）:

| iInArmType | Sub-function | Picker Config |
|---|---|---|
| `ep1Picker` | `DoInArm_9045_All_1Pick()` | 1 sucker |
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoInArm_9045_1x1_1()` | 1×1 |
| `e9045_1x2_2_13` | `DoInArm_9045_1x2_2()` | 1×2 (13排) |
| `e9045_1x2_2_14` | `DoInArm_9045_1x2_2_14()` | 1×2 (14排) |
| `e9045_1x2_4_Hot` | `DoInArm_9045_1x2_4_Hot()` | 1×2, 4 suckers (Hot) |
| `e9045_1x3_2_14` | `DoInArm_9045_1x3_2_14()` | 1×3, 2 suckers |
| `e9045_1x3_4` | `DoInArm_9045_1x3_4()` | 1×3, 4 suckers |
| `e9045_1x4_4_13` | `DoInArm_9045S_1x4_4()` | 1×4 (S, 13排) |
| `e9045_1x4_2_14` | `DoInArm_9045_1x4_2()` | 1×4, 2 suckers |
| `e9045_1x4_4_Back` | `DoInArm_9045_1x4_4_Back()` | 1×4 (Back) |
| `e9045_1x4_4` | `DoInArm_9045_1x4_4()` | 1×4 |
| `e9045_1x4_8_Hot` | `DoInArm_9045_1x4_8_Hot()` | 1×4, 8 suckers (Hot) |
| `e9045_2x1_2_13` | `DoInArm_9045_2x1_2()` | 2×1 |
| `e9045_2x2_4_12` | `DoInArm_9045_2x2_4_12()` | 2×2 (12排) |
| `e9045_2x2_4_13` | `DoInArm_9045_2x2_4()` | 2×2 (13排) |
| `e9045_2x2_4_14` | `DoInArm_9045_2x2_4_14()` | 2×2 (14排) |
| `e9045_2x2_8_Hot` | `DoInArm_9045_2x2_8_Hot()` | 2×2, 8 suckers (Hot) |
| `e9045_2x3_6_14` | `DoInArm_9045_2x3_6_14()` | 2×3 (14排) |
| `e9045_2x3_6` | `DoInArm_9045_2x3_6()` | 2×3 |
| `e9045_2x4_4_13` | `DoInArm_9045_2x4_4_13()` | 2×4 (13排) |
| `e9045_2x4_4_14` | `DoInArm_9045_2x4_4()` | 2×4 (14排) |
| `e9045_2x4_8` | `DoInArm_9045_2x4_8()` | 2×4, 8 suckers |
| `e9045_2x5_8` | `DoInArm_9045_2x5_8()` | 2×5 |
| `e9045_2x6_8` | `DoInArm_9045_2x6_8()` | 2×6 |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoInArm_9045_2x8_8()` | 2×8, 8/32 suckers |

---

## 4. 主狀態機 (DoInArm_9045_XxY_Z, Task = iArmTask)

**Pre-Switch Guard**: `DoInArmAutoSiteMapping()` || `bIndexAlarmInArmAway` → return

### Ambient Mode (常溫: Loader → Shuttle)

```
1(Wait) → 10(Decision) → 50(Prepare) → 75(HP Pre-Check) → 100(Pick Loader)
                                                                   ↓
                                           200(Close Check) → 400(Additional)
                                                                   ↓
                                                            2000(Place Shuttle)
```

### Hot Mode (高溫: Loader → HotPlate → Shuttle)

```
1 → 10 → 50 → 75 → 100(Pick Loader) → 200 → 400(Additional)
                                                   ↓
              1500/1550(Pick HP) ← 1100(Place HP) ← 1000(Pre-HP)
                    ↓
              2000(Place Shuttle) → 50 (loop)
```

**HP Verification Flow**: `case 75 → case 15000 (TryPick HP Verify)` → case 2000 (if IC) / case 100 (if empty)

### State Descriptions

| case | 名稱 | 說明 |
|------|------|------|
| **1** | Wait | 移動到等待位置；OneCycle/CleanOut 檢查 |
| **10** | Decision Hub | 依 Sucker IC 狀態路由 |
| **50** | Prepare Pick | Heater/Tray/FIFO/CleanOut 預備 |
| **75** | HP Pre-Check | TrySuck 驗證判斷 |
| **100** | Pick Loader | `DoInArmPickFromLoadStage_9045()` |
| **200** | Close Site Check | 確認關閉 Site 無 IC |
| **300** | Close Site Error | JAM0114 處理 |
| **400** | Additional | `DoInArmAdditionalFunction()` |
| **500** | HP Wait | 等待 HP 有 Ready IC |
| **600** | Z Safe | Z 軸安全上升 |
| **1000** | Pre-Place HP | 判斷是否跳過 HP 直接放 Shuttle |
| **1100** | Place HP | `DoPlaceToHotPlate_9045()` → 詳見 [HP_Knowledgebase.md §4-15](references/HP_Knowledgebase.md) |
| **1500/1550** | Pick HP | `DoInArmPickFromHotPlate_9045()` → 詳見 [HP_Knowledgebase.md §16-22](references/HP_Knowledgebase.md) |
| **1600** | Post-HP Pick Error | HP 取料後 Close Site 異常 |
| **2000** | Place Shuttle | `DoInArmPlaceToShuttle_9045()` |
| **15000** | TryPick HP | HP 驗證 |

---

## 5. DoInArmPickFromLoadStage_9045() (Task = iPickFromLoadStageTask)

從 Loader Tray 吸取 IC：確認 Z safe → Tray 有 IC → 搜尋 XY → Z 下降吸取 → 真空驗證 → 異常處理

| case | 動作 |
|------|------|
| 1 | Tray arm safe 確認、Z safe、OneCycle/DoPickLoaderOK |
| 10 | Loader Tray 有 IC? → 有: case 12, 無: case 15 |
| 12 | `SearchAndMoveInArmXYToLoad_9045()` → case 200 |
| 15 | 等待 Tray sensor clear → `DoAutoSkipCheck()` → return true |
| 200 | `MoveInArmZToLoaderPick(iRetryCT)` → case 1000 |
| 1000 | 逐一 `Suck()` → 全完成: case 2000, 異常: case 1010 |
| 1010 | Retry（Z safe → 重新吸） |
| 1050 | 超過 Retry → case 1100 |
| 1100 | `ProcessMES0101InArmPickLoaderError` → K_TRAY_END / K_CLEAN_OUT / K_SKIP / K_HOME / K_RETRY |
| 2000 | Z safe + IC Fall Down 檢查 → case 2100 |
| 2100 | `ArmFinishForLoader()` → return true |

---

## 6. DoInArmAdditionalFunction() (Task = iInArmAdditionalFunctionTask)

依序執行四種附加功能，每個完成後回 case 100 檢查下一個：

```
case 1 → case 100 → Die Clean (10000) → Precisor (20000) → Bottom 2DID (30000) → Rotator (40000→41000) → return true
```

---

## 7. DoPlaceToHotPlate_9045() (Task = iInArmPlaceToHotPlateTask)

> **完整說明**：[HP_Knowledgebase.md §4-15](references/HP_Knowledgebase.md)

依 iInArmType dispatch 到各 `DoPlaceToHotPlate_9045_NxM_N()` 狀態機。

| case | 動作 |
|------|------|
| 1 | `SearchPlateToPlace()` → case 100 |
| 100 | `MoveInArmXYToHotPlatePlace` → case 200；掉料 → case 110 |
| 200 | 偏移補正 / `UpdateHPSuckGroup()` → case 340/350 |
| 350 | Destroy 吹氣放料（loop rows × cols）→ `DoPlaceToHPSwapData()` → case 400 |
| 400 | Z safe；`HasIC()==true` → case 1（繼續），`HasIC()==false` → case 500 |
| 500 | `CheckInArmDestroyICFail()`（黏貨）→ case 501 |
| 501 | `AdjustShuttleWhichKitOrder()`；`AddHPSuckGroup()`；→ return true |

---

## 8. DoInArmPickFromHotPlate_9045() (Task = iInArmPickFromHotPlateTask)

> **完整說明**：[HP_Knowledgebase.md §16-22](references/HP_Knowledgebase.md)

| case | 動作 |
|------|------|
| 1 | `SearchPlateToPick()` → `HasHotReadyIC_9045()` → case 50 |
| 50 | Z safe → case 100 |
| 100 | `GetHPFirstTeamMotUse`（Team 分組）→ case 110 |
| 110 | `MoveInArmXYPickHotPlate_9045` → case 150 |
| 150 | 等待 Shuttle Load-Free → case 190/200 |
| 200 | `MoveInArmZToHotPlatePick` → case 300 |
| 300 | `HotplateDataConversion()`（資料交換）→ 異常: case 320, OK: case 340 |
| 330 | JAM0109 → K_SKIP/K_HOME/K_RETRY |
| 350 | `DataForwardAndNextTeam()` → 還有: case 1, 全完: case 400 |
| 400 | `bPickFormHotplatePartOK = true` → return true |

---

## 9. DoInArmPlaceToShuttle_9045() (Task = iInArmPlaceToShuttleTask)

兩個 Shuttle 邏輯對稱（SHT1: case 900~1610；SHT2: case 1900~2610）：

| case | 動作 |
|------|------|
| 1 | `SetInArmUseSuckToHasNullIC`、HasRealIC 確認 → case 100 |
| 100 | 選擇 SHT1 (case 900) 或 SHT2 (case 1900) |
| 900 | 等待 `InSHT1InLF` → speed adjust → case 930/950 |
| 1000 | 確認 carrier kit / `bCanFreeShuttle` → case 1050 or 900 |
| 1100 | `MoveInArmZToShuttlePlace_9045` + Offset → case 1200 |
| 1200 | Destroy 吹氣（loop 2×4）→ case 1300 |
| 1300 | `InitDoInArmCheckShtFloatTask` → case 1400 |
| 1400 | `DoInArmCheckShuttleFloating()`（浮起檢查）→ case 1500 |
| 1500 | `AdjustShtOrderWhenPlaceToSht` → return true |

---

## 10. 吸嘴配置 (iInArmType)

**吸嘴矩陣（最大）**: Row0: Aa~Ah, Row1: Ba~Bh（2×8）

**iInArmType 命名規則**: `e9045_{PickRow}x{PickCol}_{ShtCnt}[_suffix]`

- 後綴 `_Hot`：高溫模式，iPickRow = iShtRow × 2
- 後綴 `_13`：吸嘴使用 1/3 跳格（PickStep=2）
- 後綴 `_14`：吸嘴使用 1/4 跳格（PickStep=3）
- 後綴 `_Back`：從背面取料

**決策入口**: `DoInArm_9045_Type()` — 詳見 [InArm_TMyKitSuck_and_TypeDecision.md §4](references/InArm_TMyKitSuck_and_TypeDecision.md)

### General.ini 吸嘴相關參數

| INI Key | 說明 |
|---------|------|
| `USE_PICKER_COUNT` (`rgPickerCount`) | `ep4Picker`=0 / `ep8Picker`=1(預設) / `ep2Picker`=2 / `ep16Picker`=3 / `ep1Picker`=4 |
| `USE_IN_OUT_ARM_X_PITCH` | 影響 `CheckPickerMode()` → `iXStep`（1/2/3/4）；iXStep 決定 Shuttle X-Pitch 補正次數 |
| `USE_IN_OUT_ARM_Y_PITCH` | 影響 `CheckInArmYStep()` → `iYStep`（1=一次全放, 2=分兩次） |
| `IN_OUT_ARM_X_PITCH_MIN/MAX` | X Pitch 閾值；CheckPickerMode 使用 |
| `IN_OUT_ARM_Y_PITCH_MIN/MAX` | Y Pitch 閾值；CheckInArmYStep 使用 |
| `BASE_X_TO_HP` | InArm 基準 X 到 HP 距離（通常=6800, mm×100）；`ArmXCanSuck4IC_9045()` 等使用 |

### TMyKitSuck 核心成員速查

| 成員 | 說明 |
|------|------|
| `iPickRow/iPickCol` | 從 Loader/HP 吸取時的矩陣大小 |
| `iShtRow/iShtCol` | 放到 Shuttle 時的矩陣大小 |
| `iXStep` | X 方向放料移動次數（CheckPickerMode 動態設定） |
| `iYStep` | Y 方向放料次數（CheckInArmYStep 設定） |
| `iWhichSht` | 目標 Shuttle 編號（0=SHT1, 1=SHT2） |
| `iWhichKit` | 目標 Kit（0=左, 1=右） |
| `Item[R][C]` | 各 sucker 狀態：NULL_IC/HAS_IC/HAS_HOT_IC/測試結果 |

> 完整說明 → [InArm_TMyKitSuck_and_TypeDecision.md §2-3](references/InArm_TMyKitSuck_and_TypeDecision.md)

### 軟體架構深入（HP 縮 Pitch、Teaching 推導、機型拓樸）

詳見獨立技能 [`ht9045-sucker-architecture`](../ht9045-sucker-architecture/SKILL.md)（Steven Chou 2023.07 軟體重構脈絡）：

- `TMySucker` / `TMyKitSuck` 四層欄位差異（`iMotRow` / `iMaxRow` / `iPickRow` / `iShtRow`）
- HP 縮 Pitch 數學模型（`iBaseXToHP` / `iHPBasePos` / `iPickStep1Pos`）
- Loader 共用 Function 對照（`MoveArmXYToLoaderStage` / `GetInArmToLoaderPosition` / `_Single`）
- InArm ↔ OutArm 基準軸鏡像對稱關係
- Y-Pitch Modular 5 種模式（`iXPitch60` / `iXPitchManual635` / `iXYPitchVariable` / `iXPitchManual360` / `iXYPitchRowA`）

---

## 10.5 InArm Pitch 模式 — `ArmSpeed[InArm].bVariModeFIX`

InArm Picker 的開合（Pitch）有兩種模式，以 `ArmSpeed[InArm].bVariModeFIX`
（位於 `cprod.h:2818` `TArmSpeed`）切換。**請勿與 OutArm 的同名變數混淆。**

### 10.5.1 語意

| 值 | UI label (`rgInArmPitch`) | 含意 | SECS ECID 8564 描述 |
|----|---------------------------|------|---------------------|
| `false` (0) | "Open/Close" | **變動 Pitch**：依 IC 距離自動把 Picker 撐開 → 一次可吸 4/8 顆 | "0: Open/Close" |
| `true` (1) | "Fixed" | **固定 Close Pitch**：Picker 維持最近距離 → 一次只吸 1~2 顆（對 IC 較大、對位精度高的場景）| "1: Fixed" |

> ⚠️ **命名陷阱**：cSpeed.cpp 讀取的 INI key 叫 `"One by one"`
> （`Input Arm` section），歷史命名沿用，**並非真的「一顆一顆」**。
> 在 32-Site/16-Site 模式下，Fix 模式仍可能一次吸 2~4 顆。

### 10.5.2 三層變數結構

```
ArmSpeed_File[InArm].bVariModeFIX   ← 工作檔暫存 (cSpeed.cpp UI/Read/Write)
        │  ApplyToProd / ProcessLastSetIni
        ▼
ArmSpeed[InArm].bVariModeFIX        ← 執行中即時值（被 InArm 流程讀取）
```

備份用：`IniConfig.bBackUpInArmMode`（QA / SCK_ART / Loader Skip 暫存原值，
事後還原）。

### 10.5.3 主要 Pitch 判斷點（讀取 `bVariModeFIX`）

| 檔案:行 | 用途 |
|---------|------|
| `ainarm9045.cpp:6088` `AdjustInArmClosePitchCondition()` | 設 `bCanPick2ICAtOnceTime`：true=允許多顆同時吸；false=Fix 模式或計算失敗 |
| `ainarm9045.cpp:6284` `SearchLoadTrayUpDown_9045()` | 條件 `bVariModeFIX==false && iPickRow==2` → 進入「一次吸 8 顆」branch；否則走較少顆 |
| `aoutarm.cpp:2315` (OutArm) | `bVariModeFIX==false && !bUseOnebyOne` → OutArm 支援多顆同放 |
| `cOffSet.cpp:1089` | 追擊偏移功能僅對非 Fix 模式啟用 |
| 各 OutArm 變體 (1162/925/1031/...) | Fix 模式或 OneByOne → Pitch 收 close 才下放 |

### 10.5.4 設值點與時機（寫入 `bVariModeFIX`）

| 檔案:行 | 觸發條件 | 設定值 | 備註 |
|---------|---------|--------|------|
| `cSpeed.cpp:497` | 讀 ArmCondition.Data `[Input Arm] One by one` | UI 設定值 | UI 入口 |
| `cSpeed.cpp:510` | 8-Site/4-Site + IC X-dim ≥ 25mm | true (ArmSpeed_File) | 大 IC 強制 Fix |
| `cSpeed.cpp:522` | `bI37_EnableFIFOMode && rsmFIFOMode` | true | FIFO 模式 |
| `ainarm2.cpp:188` | **QA Mode 接近結束（`iQAModeLoaderCT > QACount-20 && < QACount`）**| true | 詳見 ht9045-qamode SKILL §5.4 |
| `ainarm2.cpp:177` | QA Mode CleanOut 後 | `IniConfig.bBackUpInArmMode`（還原）| - |
| `csystem.cpp:10466/10916/10967/12007/13088/14920/14951` | TrayFeed/QA結束 等多處還原 | `IniConfig.bBackUpInArmMode` | - |
| `acatchtray.cpp:5888-9` | Catchtray 流程結束 | 還原 | - |
| `aoutarm9045.cpp:3703` | OutArm 還原 | 還原 | - |
| `asendic_Loader.cpp:1209-10` | sendic Loader 還原 | 還原 | - |
| `aoutarm.cpp:2317`（呼叫處）| Loader Pick Skip + `bE78OneByOneWhenPickErrAtLoader` | true（含 ArmSpeed_File）| Loader 取錯後改 OneByOne |
| `SCK_ART.cpp:1011/1024/1213` | ART 流程：In/Out Arm 一顆一顆放 | true | `CheckInArmNeedVariModeFIX()` |
| `SCK_ART.cpp:1219/1226` | ART 結束 | `bBackUpInArmMode`（ART 自有備份）| - |
| `cinitial.cpp` (透過 ApplyToProd) | Initial Start | 套用 ArmSpeed_File | - |

### 10.5.5 SECS/GEM 整合

`uHGemHT9045_EC.cpp:898` 將 `ArmSpeed_File[InArm].bVariModeFIX` 對應為
**ECID 8564 "In Arm Pitch Function"**（INT_4，預設 1）。
OutArm 對應 ECID 8578。MES 可透過 SECS 改動，再次寫入工作檔。

### 10.5.6 與多顆吸取 (iRowCT) 的影響表（已更正 2026-05-11）

`SearchLoadTrayUpDown_9045`（ainarm9045.cpp:6227）的 pick 決策路徑：

```cpp
AdjustInArmClosePitchCondition(bCanPick2ICAtOnceTime);  // bVariModeFIX=true ⇒ bCanPick2ICAtOnceTime=false
if(bCanPick2ICAtOnceTime)
    Find_InArm_PickerMaxUseCountOnTime(...);   // 多顆 batch
else
    Find_InArm_Single(iUseSuck, iRow, iCol);    // 只挑 1 顆 (iRow/iCol 單一座標)
```

→ **bVariModeFIX=true 時實際上是 1 顆/cycle**（與先前文件表述相反）。

| InArm 模式 | bVariModeFIX=false | bVariModeFIX=true | Dispatch |
|-----------|--------------------|--------------------|----------|
| 1x1_1     | 1 顆/cycle | 1 顆/cycle | `DoInArm_9045_1x1_1` |
| 1x4_4     | 4 顆/cycle | 1 顆/cycle | `DoInArm_9045_1x4_4` |
| 2x4_8     | 8 顆/cycle | 1 顆/cycle | `DoInArm_9045_2x4_8` |
| 2x8_8 (16-Site)        | 8 顆/cycle  | 1 顆/cycle | `DoInArm_9045_2x8_8` |
| **2x8_32 (32-Site N)** | **16 顆/cycle** | **1 顆/cycle** | **同上**（_2x8_32.cpp 為死碼） |
| 1x4_8_Hot | 8 顆/cycle | 1 顆/cycle | `DoInArm_9045_1x4_8_Hot` |

> **重要更正**：`ainarm9045_2x8_32.cpp` **不在 HT9045.bpr 中**（grep 確認、無 obj），整檔為死碼。
> `e9045_2x8_32` 在 `ainarm9045.cpp:4135` 直接 dispatch 到 `DoInArm_9045_2x8_8()`，
> 與 16-Site 共用 InArm 流程，pick 決策同樣走 `SearchLoadTrayUpDown_9045`。
>
> 因此 QA Mode Stage C 觸發後（counter 105~124），理論上應該每 cycle 1 顆穩定遞增到 125 命中 Stage A `==`。
> ATK 仍失效的可能原因見 `ht9045-qamode` SKILL §5.4.3。

---

## 11. 常見問題快查

| 問題 | 查看 |
|------|------|
| InArm 卡在哪個 case？ | §4 State Descriptions 對照 `iArmTask` |
| Loader 吸料異常（Retry/Skip/Tray End）？ | §5 case 1100 MES0101 |
| HotPlate 黏貨？ | §7 case 500 `CheckInArmDestroyICFail()` |
| Shuttle 浮起？ | §9 case 1300-1400 `DoInArmCheckShuttleFloating()` |
| IC 掉料偵測？ | 各函式 `CheckInArmSuckICFallDownToHasNullIC()` |
| Ambient/Hot 流程差異？ | §4 兩張流程圖 |
| 附加功能執行順序？ | §6: Die Clean → Precisor → Bottom 2DID → Rotator |
| iInArmType 對應函式？ | §3 Dispatch Table |
| Auto Clean A 排異常下降？ | [AutoClean_OOB_Bug.md](references/AutoClean_OOB_Bug.md) |
| NN 模式 Auto Clean 兩臂為何不同時動 / ACSim 降級？ | [AutoClean_TwoArmSimultaneous.md](../ht9045-index-flow/references/AutoClean_TwoArmSimultaneous.md) |
| HP 空格搜尋演算法？ | [HP_Knowledgebase.md §6](references/HP_Knowledgebase.md) |
| **加熱盤永遠填不滿／固定留 2 排空？** | [HP_Knowledgebase.md §8.1](references/HP_Knowledgebase.md)（`PlactCT=8` 寫死，單排吸嘴變成留兩趟；SCK 20260727，908.8 已修） |
| PlactCT 門檻語意／為何要卡 `XDiv==iPickCol`？ | [HP_Knowledgebase.md §8.2](references/HP_Knowledgebase.md)（單排≠一次放完，分次放會踩 WAR0150） |
| 想把門檻放寬到 XDiv=8/12/16 要驗什麼？ | [HP_Knowledgebase.md §8.3](references/HP_Knowledgebase.md)（8 項 checklist） |
| HP spacX/Pitch/bPitchOver12000 對照？ | [HP_VariablePitch_Reference.md §1-4](references/HP_VariablePitch_Reference.md) |
| HP XDiv×XPitch 行為速查？ | [HP_VariablePitch_Reference.md §6](references/HP_VariablePitch_Reference.md) |
| HP 修改前交叉驗證？ | [HP_VariablePitch_Reference.md §7](references/HP_VariablePitch_Reference.md) |
| HP 放料狀態機 case？ | [HP_Knowledgebase.md §5](references/HP_Knowledgebase.md) |
| JAM0109 HP 取料真空失敗？ | [HP_Knowledgebase.md §20](references/HP_Knowledgebase.md) |
| WAR0150/0151 HotPlate HangUp？ | [HP_Knowledgebase.md §14](references/HP_Knowledgebase.md) |
| 奇數 YDivision 幽靈空位 Bug？ | [HP_Knowledgebase.md §13](references/HP_Knowledgebase.md) |
| PickFromHPList 放料/取料連結？ | [HP_Knowledgebase.md §18](references/HP_Knowledgebase.md) |
| iHotCount 批次識別？ | [HP_Knowledgebase.md §22](references/HP_Knowledgebase.md) |
| TMyKitSuck iPickRow vs iShtRow？ | [InArm_TMyKitSuck_and_TypeDecision.md §3](references/InArm_TMyKitSuck_and_TypeDecision.md) |
| iXStep 怎麼決定？CheckPickerMode 邏輯？ | [InArm_TMyKitSuck_and_TypeDecision.md §4](references/InArm_TMyKitSuck_and_TypeDecision.md) |
| Shuttle 路由 iWhichSht/iWhichKit？ | [InArm_TMyKitSuck_and_TypeDecision.md §4](references/InArm_TMyKitSuck_and_TypeDecision.md) |
| SetPickerCount 參數意義？ | [InArm_TMyKitSuck_and_TypeDecision.md §2-3](references/InArm_TMyKitSuck_and_TypeDecision.md) |
| 完整 case-by-case flowchart？ | [DoInArm_9045_FlowChart.md](references/DoInArm_9045_FlowChart.md) |
| WAR0152 Loader 取料 motor out of limit？（奇數寬盤 1x2 最左吸嘴吸最右欄）| §13 |
---

## 10. HP 縮 Pitch 公式與放/取料流程（Steven 2023.07 軟體重構）

> **來源**：Steven Chou《邏輯機台_軟體架構.pptx》Slide 23-32
> **詳細架構說明**：見 [ht9045-sucker-architecture](../ht9045-sucker-architecture/SKILL.md)
> **基準版本**：V3.33.904.2_20260511_RogerYang（V3.33.8XX+ 後皆適用）

### 10.1 HP 縮 Pitch 數學模型

當 HotPlate 物理 Col 數與 InArm 吸嘴模式不匹配時（1x2/2x2 vs 3-Col、1x4/2x4 vs 6-Col），縮 Pitch 計算公式：

```text
iHPBasePos     = HP.XStart - iBaseXToHP                          //假設 Step=1 時右側槽穴位置
iPickStep1Pos  = iHPBasePos + HP.XPitch       (1x2/2x2 vs 3-Col)
iPickStep1Pos  = iHPBasePos + HP.XPitch * 2   (1x4/2x4 vs 6-Col)
```

| 符號 | 含義 |
|------|------|
| `iBaseXToHP` | 基準軸針對加熱盤邊緣的距離 |
| `iHPBasePos` | 假設吸嘴對比加熱盤 Step 為 1 時的右側槽穴位置 |
| `iPickStep1Pos` | 縮 Pitch 後第一步的位置 |
| `HP.XStart` / `HP.XPitch` | HotPlate Recipe 中 X 起始座標 / X 間距 |

> ⚠️ **不要套錯機型**：1x2/2x2 是 `+HP.XPitch`，1x4/2x4 是 `+HP.XPitch*2`。

### 10.2 重構後 HP 放/取料核心函式

| 函式 | 行為 | 對應 PPT |
|------|------|----------|
| `SearchPlateToPlace()` | 工作檔載入後產生加熱盤吸取陣列，對應位置有料就往下找 → 取出**預計要放料的位置** | Slide 27 |
| `HasHotReadyIC()` | 直接檢查 `PickFromHPList` 內有無已加熱完成的 IC | Slide 30 |
| `SearchPlateToPick()` | 提取 `PickFromHPList` 第一筆 → 決定要使用的吸嘴 → 資料交換 | Slide 31 |
| `MoveInArmXYToPlate()` | **吸料的位置帶入與放料相同的 Function**（共用） | Slide 32 |
| `PlaceToHPList` / `PickFromHPList` | 待放料 / 待吸料陣列 | Slide 27-31 |

### 10.3 放料順序表（4 吸嘴範例）

```
次數    Row     Col
第0次   [0,0]   [0,1]
        [1,0]   [1,1]
```

> 若 IC 分多次放入加熱盤，需針對該 Flag++（典型用法：`iHotCount`）。

### 10.4 關鍵 Source 位置（V3.33.904.2 基準）

| 主題 | 檔案 / 行號 |
|------|-------------|
| `SearchPlateToPlace` 宣告 | `ainarm2.h` L112 |
| `PickFromHPList` (extern) | `Public/HTEditList.h` L244 |
| `GetInArmToLoaderPosition` | `ainarm9045.cpp` L4614 |
| `GetInArmToLoaderPosition_Single` | `ainarm9045.cpp` L4673（回傳 `iMovePitchX`） |

## 11. RT-Initial Start 與 Auto Tray 清除保護

> 此主題已獨立為 [`ht9045-asm-flow`](../ht9045-asm-flow/SKILL.md) 技能。
> 涵蓋：ASM 觸發條件、SetRunStartMode 決策樹、RT 跳過 ASM 機制、
> ASM 完成後模式轉換、CheckAutoHasTray 清除保護（bDoReTestStart + HasAnyICInMachine）、
> DutOnOff 觸發 ASM、相關 Config 欄位。

---

## 11b. InArm 安全互鎖判斷函式（與 OutArm 對照）

> ⚠ 對應 OutArm 端的 `IsOutArmSafe()` / `IsCatchTrayReadySupplyNewTray()`（見 `ht9045-outarm-flow`）。
> InArm 端**有查詢型互鎖、但沒有「主動授權」的對應**（不存在 InArm 版本去設 `iCatchTrayControlManual`）。
> Loader 補盤/搬空盤前由 CatchTray 側查 `InArmXYZSafe()` 決定能不能動。

| 函式 | 定義 | 角色 | 條件 |
|------|------|------|------|
| `InArmXYZSafe()` | `acatchtray.cpp` L305 | **查詢**（≈ `IsOutArmSafe()`），由 Loader/CatchTray 側呼叫（`acatchtray.cpp` 1530/2704/5923/6234/6268） | `eUnderCoveyor` → true；或 `IsMoveInArm2XYToWait()` → true；或 `iPosY >= YInArm_Shuttle1_Place[base]-300 && InArmZInSafe()` |
| `IsMoveInArm2XYToWait()` | `ainarm2.cpp` L1295 | **嚴格「已停妥於安全 XY」**（≈ OutArm 的 `fCMD==false` 條件） | `MInArmX/Y` In-Pos LED 過 **且** `CompareEncoderPos(iInArmSafeX/Y, 9)==1` **且** `CompareCommandPos(iInArmSafeX/Y, 2)==1`（encoder＋command 雙確認） |
| `InArmZInSafe()` | `acatchtray.cpp` L290 | **所有 Z 在 ZSafePos** | 逐吸嘴 `MOT[iMotNo].ReadPos()==ZSafePos`，任一不符即 false |

**與 OutArm 的差異重點**
- OutArm 容差最寬到 **−3500**；InArm `InArmXYZSafe()` 的 Y 容差只有 **−300**，且**額外要求 Z 全部在安全位**（OutArm 兩個閘門都不查 Z）→ InArm 端較嚴。
- InArm 用 **encoder＋command 雙比對**（`CompareEncoderPos` 9 + `CompareCommandPos` 2），對「command 已到、encoder 飄移」更敏感；這正是 §12.3 用消去法鎖定死結的依據。
- InArm **無**「OutArm `IsCatchTrayReadySupplyNewTray()` 那種設 flag 主動交權」的對應函式；InArm 端是純查詢，由 Loader CatchTray 主動判斷。

---

## 12. Hot Mode Loader 抽空 → 「吸嘴有料卻不放 HP」+ CatchTray 換盤死結

<!-- AI(ht9045-inarm-flow) 20260619 (RogerYang): ATK Amkor HT-9045WA 供料死結案例 -->

> ATK / Auto Retest（`USE_AUTO_RETEST=1`）+ Hot Mode 機型的供料死結。客戶現象：**「Loader 盤內無 IC，但 TrayArm 不去取盤」**。
> 機台**沒當機**（MainProc Alive=Y）、**沒 JAM**、GPIB 正常 —— 純粹是 InArm／Loader／CatchTray 三方互等。
> StateRecord 訊號簽章與完整機制見 [staterecord-analysis / deadlock-patterns.md Pattern #10](../ht9045-staterecord-analysis/references/deadlock-patterns.md)。

### 12.1 為什麼「吸嘴有料卻不去放 HotPlate」

Hot Mode 下 InArm 吸到**半盤**（Loader 最後幾顆，吸嘴未吸滿）時，會**等補滿才放 HP**，不放半盤。
關鍵在 `DoInArmPickFromLoadStage_9045()` **case 15**（`ainarm9045.cpp` ~L7777）：

```cpp
case 15:
    if(MOT[MMTrayY_Car].fHasTray || MOT[MMTrayY].fHasTray || MOT[MMTrayZ].fHasTray)
        { Task=10; return false; }                  // ← 還有盤在 → 回去等補滿，不放 HP
    else                                            // ← 只有 Car/Y/Z 全空才放行
    {
        if(DoAutoSkipCheck()) {
            if(InArmSuck.HasRealIC())
                SetInArmUseSuckToHasNullIC(iSht,iKit);  // 半盤剩餘位標 NULL → 視為收齊
            Task=1; return true;                     // → 接著去放 HP
        }
    }
```

- 只要 `MMTrayY`（空盤還在）或 `MMTrayY_Car`（新盤已備）任一 `fHasTray` 為真 → InArm 一直等「下一盤來補滿」，**不放 HP**。
- 因此「吸嘴有料卻不動」的根本不是 InArm 故障，而是它在**等補盤**；而補盤又被卡住（見 12.2）。

### 12.2 三方互等死結

| 角色 | hang 當下狀態 | 可信度 |
|------|------|------|
| InArm `iPickFromLoadStageTask` | case 15（10↔15 振盪，貫穿空窗期）= loader 空、盤還在、等被搬走才吸滿放 HP | ✅ 可靠（ring 未被沖）|
| 補盤 `DoSupplyNewICTray` case 1（`asendic_Loader.cpp`）| `LoadTask=1000`，等 `MMTrayY.fHasTray==false`（空盤先被搬走）才推 Car 盤 | ✅ 可靠 |
| CatchTray `DoCatchTray`：**為何不搬空盤** | ⚠ **hang 當下行為不明**——CSV ring 被 home burst 沖掉（見 staterecord LL-11），無法判定它凍結 or idle loop | ❌ 看不到 |

→ 可確認：空盤整段沒被搬走（EventLog 無 MES0650）、InArm 在等。**但「CatchTray 為何不搬」的 hang 當下狀態，這份 auto-record 看不到**（ring 被 home burst 沖、LED/位置被斷電污染）。要看到需 **hang 當下手動 State Record**（按任何觸發 home 的鍵之前）。

### 12.3 為什麼 CatchTray 不搬空盤：消去法鎖定 `InArmXYZSafe()==false`（取代先前 LED 推論）

CatchTray case 100 走到搬空盤（`Task=200`→case 250 `DoCatchFromLoader`=MES0650）的**唯一關卡**是 L5887 四條件 AND 閘：
`MMTrayY.Tray.HasIC()==false && MMTrayY.fHasTray==true && bPlaceToHotplate==false && InArmXYZSafe()==true`
閘過→`Task=200`（本機無 skip，後續分支一律落 B5 預設搬盤）；閘不過→`Task=300`（idle）。

12_38 record 實測 CatchTray 一直 100→300→400（值只有這三個、**無 200/250**）→ **閘每圈都失敗**。逐條用乾淨證據（不碰被污染的 LED/快照）判：

| # | 條件 | 狀態 | 乾淨依據 |
|---|---|---|---|
| ① | `Tray.HasIC()==false`（盤空）| ✅ 滿足 | InArm 卡 PickFromLoad **case 15**；底層同一個 `Tray.HasIC()` |
| ② | `fHasTray==true`（盤在）| ✅ 滿足 | `DoSupplyNewICTray` 卡 **case 1**（`LoadTask=1000`、`iSupplyNewIC=1`）只有 fHasTray==true 才會卡住不動 |
| ③ | `bPlaceToHotplate==false` | ✅ 滿足 | InArm 在 pick-loader 相，其上游 `case 10 else` 進入前提即 bPlaceToHotplate==false |
| ④ | **`InArmXYZSafe()==true`** | ❌ **失敗（消去法）** | ①②③成立、閘卻每圈失敗 → 失敗者**只能是 ④**。此結論**不依賴被污染的 In Pos/位置** |

> **這取代先前用 In Pos 燈的推論**：④ 為失敗閘，是由①②③（皆 CSV/邏輯可證）消去法得到，站得住。

**再內一層（仍乾淨）**：`InArmXYZSafe()=路2 OR 路3`。InArm 已退到最遠安全位（PickFromLoad case 10 loader 空 → `MoveInArm2XYToWait()` → case 15 振盪），該處 `iInArmSafeY=Shuttle1−9000`（`cinitial.cpp:9051`）使**路3 結構性為假**；InArmXYZSafe 既為 false ⟹ **`IsMoveInArm2XYToWait()`（路2）也為 false**。

#### In Pos 語意（RogerYang 確認）
In/Out Arm X/Y 的「In Pos」燈亮 = **作動中(moving)**，非到位；故 `Led[iInposLed]==false`（要求非作動/已停）是合理判據。**但 auto-record 當下已回 home（含斷電），In Pos/Alarm 燈與位置都被污染，不可參考**（見 staterecord LL-10）。

#### ✅ SaveTaskList 直接記了答案（20260623）—— In Pos 作廢，失敗在 X encoder
`Task_ListWithTime.csv` 檔尾的 `SaveTaskList` 區（見 staterecord SKILL LL-12）3 筆一致：
`Led X/Y=false`（In Pos 過）、`cmd X/Y ±2=1`（過）、`enc Y ±2=1`（過）、**`enc X ±2=-1`**、**`IsMoveInArm2XYToWait()=false`**。
→ `IsMoveInArm2XYToWait()` 失敗的唯一一項是 **InArm X encoder 不在安全位**（command 在安全位、encoder 不在）。**先前「In Pos 是死點」作廢**（兩軸 In Pos 皆 false=過）。方向轉為 **InArm X 定位問題**（伺服 following error / 機構 / 或斷電飄移）。

#### 仍待查（為何還不能定論）
同筆 `Motor.xls` 的 InArm X encoder=7582=`iInArmSafeX`（理應 enc±2=1），與 SaveTaskList 的 -1 **矛盾** → 擷取當下 X 在動/不穩（home/斷電）→ 此 `enc X=-1` 也可能污染。需 `[CatchStuck]` clean in-hang log（卡滿15s、回 home 前）確認：clean log 若 enc X 仍偏離安全位 → 真 X 定位問題（查 InArm X 伺服/following error/機構）；若正常 → 斷電飄移假象。

### 12.4 可信度分級 & 下一步
| 結論 | 來源 | 可信度 |
|------|------|--------|
| ~10 分鐘靜默 stall：空盤未搬走、InArm case 15 等補料、測試臂餓死、零警報 | Task CSV 時序 | 高 |
| `IsMoveInArm2XYToWait()=false`、In Pos 兩軸 false（過）、失敗項=**enc X 不在安全位** | **SaveTaskList 直接記錄（不靠推論）** | 高 |
| enc X 偏離是「真 X 定位」還是「斷電飄移假象」 | Motor.xls 與 SaveTaskList 矛盾 | 待查（需 clean in-hang log）|

**下一步**：①`[CatchStuck]` clean in-hang log（回 home 前）取乾淨 enc X / cmd X / In Pos；②正常機在「InArm 停最遠安全位」時 SaveTaskList 的 enc X 對照；③若坐實 X 偏離 → 往 **InArm X 伺服 following error / 機構 / 安全位可達性** 查（非 In Pos、非單純軟體判定）。

### 12.5「config/IO/DB 都一樣卻只壞一台」（機制待查）
**已確定且不變**：teach 點是各機台機差校正的基準，**正確且不可任意更改**（改了取放料就不準，offset 也補不回所有機差）。本案 InArm command==encoder 停在 teach 推導的安全位 → teach/定位正確、不是 teach 錯。
**「為何只壞這台」目前無定論**：先前歸因「InArm X/Y in-position(`Led[8]`) 訊號判定」已收回（見 §12.3：In Pos 語意搞反＋瞬時快照不可靠）。真因待用 §12.4 下一步查清，**勿先以「對齊／重 teach」當 workaround，也勿先改 code**。

> 案例：2026-06-19 ATK / Amkor Korea HT-9045WA（Serial ELC619, CC=971, `iInArmType=22 e9045_2x4_8`）。

---

## 13. WAR0152 Loader 取料「motor will out of limit」— 奇數寬盤 + 1x2 AxEx 單顆取料超極限

<!-- AI(ht9045-inarm-flow) 20260622 (RogerYang): 華天南京 X:7 Y:17 / Kit 1x2 案例 -->

> 客戶現象：1x2 模式、tray X 為**奇數欄**（如 X:7）、**最左 1 號吸嘴（A）去吸最右欄時報 `WAR0152`「Loader tray parameter error, motor will out of limit」**。
> EventLog 簽章：`WAR0152` / UnitName `01 Input Arm` / Function `SearchLoadTrayUpDown_9045`。機台**沒當機、沒 JAM**，純屬取料座標超出 `MInArmX` 正向軟極限。
> 案例：2026-06-22 華天南京（CUSTOMER_CODE=729）HT9045 V3.33.905.14，recipe `HK098_WILY4300-6BG256I_14.014.0`，tray X:7 Y:17、X Pitch 18.35mm、device LFBGA 14×14。

### 13.1 觸發條件
- `iInArmType = e9045_1x2_2_13`（A、E 兩吸嘴；base 軸 = E，`iInArmXBase=2`；由 `USE_IN_OUT_ARM_Y_PITCH=1`→`iXPitchManual635` 決定）。
- Tray X-Division 為**奇數**（最右欄無法成對，必定落單）。
- 落單欄由 **最左吸嘴 A** 單顆取：`GetInArmToLoaderPosition_Single()` 取「第一支被使用的 suck」= j0 = A → `iRealUseSuck=0`。

### 13.2 機制（為何偏偏最右欄才報）
單顆取料 X 目標（`ainarm9045.cpp` `GetInArmToLoaderPosition_Single`）：
```
iXPos = XInArm_Tray_Pick[base_E] + iCol*TrayXPitch + (iInArmXBase - iRealUseSuck)*dInArmXPitch_1Step
                                                      └ 用 A 時 = 2 × close-pitch，把滑台往右多推（讓最左吸嘴對到該欄）
```
- `iCol` 越大 `iXPos` 越大；到最右欄時 `iXPos > MOT[MInArmX].PSoftLimitP` → `MoveInArmXYToLoader_9045(RealMove=false)` 回 false → `SearchLoadTrayUpDown_9045` 丟 `WAR0152`。
- 旁證：最右欄**前一欄**剛好過關、最右欄超出 → 完美解釋「偏偏最後一欄才報」。

### 13.3 內建縮 pitch 補救為何失效（關鍵）
`GetInArmToLoaderPosition_Single` 本有 `if(iXPos > PSoftLimitP-100)` 的縮 pitch 補救（`bUseAxExPicker()` 分支），用：
```cpp
iTrayXPitch = DeviceForm.XDimension * i + 200;   // 需落在 [iXpitchMinX2, iXpitchMaxX2]
```
其中 `DeviceForm.XDimension` = Contact form「X-Outer dimension」(`cContact.cpp` "Torque Control / X Dimension"，預設 0) × 100。
**若該值 = 0 或太小（華天案誤填 3mm）** → `XDimension*i+200` 撐不到下限 → `bHasMatch=false` → 回退設成 `iXpitchMax`（**最大** pitch）→ 反而把滑台推更遠 → 必超限。

### 13.4 解法
- **快解（先驗證）**：Contact「X-Outer dimension」填回裝置實際尺寸（如 14mm）→ 縮 pitch 命中合法值（如 30mm gap）→ A 吸得到最右欄。
  改前注意 `DeviceForm.XDimension` 亦用於：OutArm 縮 pitch（`aoutarm.cpp`，有 `iXpitchMin/Max` 夾住，安全）、AOI/Laser 位置、SECS Package Size 上報；14mm 都在「大 IC 切模式」門檻（25/40/50mm）以下，不會誤切模式。
- **根解（code，最穩，與 XDimension 脫鉤）**：把 `GetInArmToLoaderPosition_Single` 的 `bHasMatch==false` 回退由 `iXpitchMax` 改 `iXpitchMin`；或單顆取料（`bCanPick2ICAtOnceTime==false`）逼近 +X 極限時直接 `dInArmXPitch_1Step = iXpitchMin`（單顆取時 E 閒置，能縮就縮）。

> ⚠ 不採「最右欄改用 E 吸嘴」(`iRealUseSuck=iInArmXBase` → offset=0) 的原因：**尾盤會反過來出現 A 沒料的配對問題**，客戶通常不採此法 → 優先走「縮 pitch」。

> 完整推導（含座標數值、`DeviceForm.XDimension` 全使用點掃描）見 [ht9045-sucker-architecture §4.5.3](../ht9045-sucker-architecture/SKILL.md)。


---

## 14. HP 取料帳本 `PickFromHPList`（`uPlateInfo`）— 設計原則與 ASM 補料失步

> 2026-07-31 偉測 HHT-30「加熱盤剩一顆料吸不掉」案的完整結論。與 [staterecord-analysis Pattern #16](../ht9045-staterecord-analysis/references/deadlock-patterns.md) 互為上下游（Pattern #9 是同一條鏈的下游表現）。

### 14.0 總綱：帳本錯亂全景（先讀這節，20260829 定版）

> **一句話：正常生產遵守「放料=記帳、取料=銷帳」，帳↔料永遠 1:1；ASM 不走這兩個動詞，改用第二套「借出=暫停、復活=補回」。兩套動詞平行存在、只靠旗標對時序，任何中斷落在借與還之間，帳就永久失真——而且三種失真全部靜默。**

**理想模型（機台鐵則）**：`放料 → 記一筆帳（格位+吸嘴+Site）；取料 → 銷那筆帳`。不變式：booking ↔ 盤上的料 1:1。

**正常生產（FT）完全符合**，四步一組（跑一萬次帳都不會錯，FT-only 機台從不出這問題）：

| 步驟 | 碼位置 | 帳本動詞 |
|---|---|---|
| 放料 case 200 | `UpdateHPSuckGroup` | **記帳**（建 team）|
| 放料 swap | `DoPlaceToHPSwapData` → `SetArrPlateXY` | 補吸嘴/座標/Site |
| 放完 case 501 | `AddHPSuckGroup` | 開下一個空 group |
| 取料 case 350 | `DataForwardAndNextTeam` | **銷帳**（刪 team）|

**ASM 的第二套動詞**（設計本意：借的料會補回同一格，帳不用動）：

| 步驟 | 碼位置 | 帳本動詞 |
|---|---|---|
| 借料（取）| case 350 ASM 分支**跳過**銷帳（`ainarm_SearchPickPlate.cpp` L1324-1328）；改 `SetPlateSuck(格,false)`（L346，條件式）| **暫停**（bSuck=false、座標保留）|
| 補回（放）| case 200 ASM 分支**跳過**記帳；`DoPlaceToHPBackupData` 尾段 `SetPlateSuck(格,true)`（L4840）| **復活** |

借還成對 → 帳回原狀 → 沒事（＝這功能多年無恙的原因）。**錯亂全部發生在借還不成對或動詞錯配**：

| # | 錯亂 | 機制 | 實證 | 對策 |
|---|---|---|---|---|
| ① | **幽靈**（盤空帳在）| 借了、中斷（fail 迴圈/One Cycle/Clean Out/Home/關站）、沒還；且 ASM 結束不對帳，沒人清它 | HHT-10 20260826（4 筆，site 8/7/6/5 恰為一輪後半）| Reconcile（保險絲，908.16）；長期：ASM 中斷點要有「歸還或銷帳」收尾 |
| ② | **孤兒**（盤有帳無）| 補回時舊帳已被正常生產銷掉 → `SetPlateSuck(true)` **靜默撲空**（找不到照樣回 true，`HTEditList.cpp` L2765-2770），復活這個動詞沒有「撲空要補記帳」分支、連 log 都沒有 | HHT-30 20260731（#16）、HHT-76 20260829 批尾 | 20260731 補登記（保留）＋撲空/拒登落 log |
| ③ | **重複**（一格兩帳）| 20260731 的補登記**無條件**記新帳，而尾段 L4840 又把舊帳復活 → 常態下（舊帳還在）每輪借還 +1 筆 | HHT-80 20260828（乾淨帳本 2.5h → dup=25；三輪 fail＝同格三份 G0 T7/T13/T18）| **冪等化**：`HasBookingByCoordinate` 查無此格才登記（20260829 修法）|
| ④ | **延後死結** | dup 的第一份被銷後，第二份自動變幽靈 → 回到① | HHT-80 預測 | ③＋① 合力 |
| ⑤ | **旗標錯序** | `bAutoSiteMapHasPickHP`/`bAutoSiteMapHotplateSave`/`bRunAutoSiteMapping`/`SiteMapData` 設定與清除點各自獨立，中斷落在不同組合 → 記帳/銷帳/暫停/復活錯配（§14.4 的 guard 情境是其中一種）| §14.4 | 每個新分支都要重查四旗標當下組合 |
| ⑥ | **粒度不對稱**（機制推演，尚無單案實證）| 銷帳粒度＝整個 team（8 slot），暫停/復活粒度＝單格。team 內一格借出中、其餘格被正常生產取走 → 整 team 被銷、**連暫停中那格一起消失** → 補回撲空 → 孤兒 | — | 同②的落 log 可抓到 |

⚠ **重要態度**：ASM Must-Pass-Bin 反覆 fail、關站、operator One Cycle/Clean Out 介入——這些都是**正常操作**，不是「客戶使用型態問題」；上表每個中斷點都是本來就要補的洞（RogerYang 20260829 裁示）。

**「放料=記帳、取料=銷帳」為何支援得了 ASM（20260829 定案，RogerYang 提問後推演）**

ASM 取完料一樣放 Shuttle 續流、空格一樣留給 InArm 補新料——物料流與正常生產同構，模型本身沒有不支援的地方。當年卡住設計者的是**兩個 API 缺口**：銷帳只有 `DataForwardAndNextTeam()`（只能銷**隊首整個 team**），記帳只有 `UpdateHPSuckGroup/SetArrPlateXY`（只能記**隊尾**）；而 ASM 的取放發生在**佇列中間的單格**。設計者為了保 team 完整（一趟 8 顆、FIFO 順位、身分繼承）選了「暫停/復活」捷徑——**它是一個沒有 rollback 的兩段式交易**。
對照之下，「借=銷帳、補=記帳」的**兩個半狀態本身都是終態**：借了沒補＝空格無帳＝**合法狀態**（之後正常放料自然會放料+記帳）→ 系統自癒、不需要交易保護、不需要 Reconcile。且**若借料時真的銷了帳，20260731 的補登記就自動正確**（無舊帳可疊）——冪等檢查退化成保險。
代價盤點（全是 UPH 級小損耗，無正確性風險）：team 碎片化（借 1 格→7+1 兩趟，僅 ASM 前後短暫存在）；FIFO 順位後移（soak 由 `HotTime>=iHotTime` 把關，不會取到沒烤熟的）；目的地分組從 `iHotWhichShuttle/iHotWhichKit` 帶、單格取料幾何本就支援（§14.3）。

**修正路線（20260829 RogerYang 裁示：採中期）**

| 階段 | 內容 | 改動面 | 殘留風險 |
|---|---|---|---|
| 短期（908.16 已上機）| Reconcile 對帳（保險絲）＋ gate 逐格化 | 3 檔 | 只擦地板不關水龍頭 |
| ~~中期（採用）＝只換帳本動詞，不動管線~~ **⛔ 20260831 已回退，勿再採用，理由見下方「20260831 定案」** | ①新原語 `ConsumeBookingByCoordinate`（按座標銷單格，掃全帳含 legacy dup、空 team 刪、空 group 非隊尾才刪——原語邏輯已在 Reconcile Pass1/2 驗證過）②借料點 `SetPlateSuck(false)` → 銷帳 ③補回點改「查有帳→復活（吃掉舊版暫停殘留）；查無帳→記帳；metadata 不齊→落 log」④刪無條件復活（L4840）。**ASM 掃盤取料、強制 anchor、四支旗標、Reconcile 全部保留** | 2 檔動詞點＋HTEditList 2 個新方法；**模式檔 0 改動** | 未經 `HotplateDataConversion` 標記路徑的借料仍可能留幽靈（HHT-10 的幽靈就是 bSuck=1，證明有借料不走 L346）→ Reconcile 兜底；關站格（iWhichSite<=0）拒登仍是孤兒 → log 收證據另案 |
| 長期（尚未排程）＝廢掉第二套動詞的管線 | ASM 補回走正常 case 200 記帳（拿掉 25+ 模式檔的 skip 條件）、`DoPlaceToHPBackupData` 縮成純 metadata、`SetPlateSuck` 借還 API 與旗標的帳本角色全數清理、關站格政策定案、Reconcile 退役成純診斷 | 25+ 模式檔 | 需完整迴歸，等中期上機驗證後排程 |

#### 中期實作記錄（20260829 完成改碼，**待編譯上機驗證**）

資料夾 `D:\HT9045\RogerYang\HT9011UC_Code_V3.33.908.16_20260827_NB_AI`（各檔留 `*.bak_20260829`）。**5 處 / 4 檔，模式檔零改動**：

| 檔案 | 位置 | 動作 |
|---|---|---|
| `ainarm_SearchPickPlate.cpp` | L346（取代 1 行）| 借料 `SetPlateSuck(iP,iPlateR,iPlateC,false)` → **`ConsumeBookingByCoordinate()`＝銷帳**。L345 `bAutoSiteMapHasPickHP=true` 保留（幾何 anchor 角色）；三重條件 `rsmAutoSiteMap && bUSEJCETSiteMapMode && *flag1` 原封不動 ⇒ 正常生產碰不到 |
| `ainarm_SearchPlacePlate.cpp` | L4759-4783（13→25 行）| 補回改**記帳三分支**：查有帳→`SetPlateSuck(true)` 復活（吃升版前留在 json 的 `bSuck=false` 暫停帳）／查無帳且 metadata 齊→`UpdateHPSuckGroup`+`SetArrPlateXY` 記帳／metadata 不齊→`MyDBIProcess("Exception", "BackupData skip register : P%d C%d R%d Sht%d Kit%d Site%d")` |
| 同上 | L4848（刪 6 行→1 行註解）| 移除**無條件**復活 `SetPlateSuck(true)`（功能已被上面「查有帳」分支吸收）|
| `Public/HTEditList.cpp` | L2772-2846（+75 行）| 新增 `HasBookingByCoordinate()`（座標相符即算存在，**不看 bSuck**）＋`ConsumeBookingByCoordinate()`（**掃全帳**銷所有 match slot ⇒ legacy dup 一次消化；team 全空刪 team；group 空**且非隊尾**才刪 group；由下往上迭代）|
| `Public/HTEditList.h` | L212-213（+2 行）| 兩個宣告 |

⚠ **編輯注意**：`ainarm_SearchPlacePlate.cpp` 是 **LF 為主的混合換行檔**（改前 4951 LF / 34 CRLF，CRLF 全是歷次 AI 插入的孤島）。**不可用 `Set-Content` 寫**（會把全檔轉 CRLF、毀掉 diff），要用「cp950 讀 raw text → 依 `\n` 切成 parts（保留各行原有 `\r`）→ 只改目標 index → `\n` 接回 → `Set-Content -Encoding Byte`」。另三檔為純 CRLF，可直接 `Set-Content -Encoding Default`（本機 ACP=950＝Big5）。

驗證：全資料夾 diff 對 `_NB` **僅 4 檔不同**；四檔 no-BOM、high-byte 完好；`ainarm_SearchPlacePlate.cpp` CRLF 孤島由 34 降至 21 行（新內容用 LF 對齊基底）。

**設計要點（比對/回顧時看這三條）**：①借料點只換動詞、沒換路徑；②復活沒被拿掉、只是從「無條件」改成「查有帳才做」，升版相容靠它；③`ConsumeBookingByCoordinate` 掃全帳而非只掃隊首，所以 HHT-80 型 legacy dup 在借料當下就一次銷完，不必等 Reconcile。

---

#### ⛔ 20260831 定案：中期方案（借料改銷帳）已回退 —— **借出只能是「暫停」，不能是「銷帳」**

> 上面那條中期路線在偉測 20260831 模擬中**製造了新的批尾孤兒**，已回退。這一節是本題的最終結論，也是往後動 HP 帳本前**必讀**的通則。

**為什麼「借=銷帳」在推演上看起來對、實作上必壞**

§14.0 那段推演漏掉一個維度：**粒度**。

| | 借（一次一格）| 還（一次一格）| team 結構 |
|---|---|---|---|
| **暫停模型（原設計，正確）** | `SetPlateSuck(false)` —— 只翻 `bSuck`，**`iPlateR/C/iSite` 全部留著** | `SetPlateSuck(true)` | 原本那個 4 格 team **原封不動** |
| ~~銷帳模型（20260829，錯）~~ | `ConsumeBookingByCoordinate` —— 連座標一起清成 -1 | 逐格重建 | team **被拆成 4 筆單格帳，永遠拼不回去** |

還料端（`DoPlaceToHPBackupData`）是**一格呼叫一次**、彼此之間沒有任何「這 4 格屬於同一趟」的資訊。所以只要借料端把座標清掉，還料端就**沒有任何辦法**重組回原本那個 4 格 team。

**後果分兩層**：

1. **功能層（客戶會直接看到、不可接受）**：ASM 借走的 4 顆變成一顆一趟 Z 下取，不再是「一次吸 4 顆直接上 shuttle 測試」。RogerYang 20260831 裁示：*「這會造成客戶誤解，該放四顆就還是要放四顆」*。
2. **正確性層（更嚴重）**：重建出來的單格帳若用 `UpdateHPSuckGroup()` 併進隊尾 group，就會踩下面那條鐵則 → 批尾孤兒。

**🔑 鐵則：group / team 的物理語意，以及 `GetTeamCount()>1` 的真正代價**

| 帳本層級 | 物理意義 |
|---|---|
| `group` | **一趟 arm 取料行程** |
| `team` | 這一趟裡的**一次 Z 下**（可同時吸多格）|
| `bSuck[i][j]` + `iPlateR/C[i][j]` | 這次 Z 下用哪支吸嘴、對到哪個盤格 |

`DataForwardAndNextTeam()`（`HTEditList.cpp`）：

```cpp
if(HPGroup->GetTeamCount()>1) { HPGroup->DataForward(); return true;  }   // 同一趟還沒吸完
else                          { ...DataForwardAndDelete(...); return false; } // 這趟結束
```

呼叫端 `ainarm_SearchPickPlate.cpp` case 350：**回 true → `Task=1`，整個取料流程重跑一次**。

> **⇒ 任何把單格 team 塞進既有 group 的動作，都等於宣告「這一趟還要再下去吸一次」。**
> 若那些格子實際上已無料（或吸嘴映射對不上），取料端就會**空轉連續 pop**，把同 group 排在後面的**真 team 一起銷掉** → 盤上留下有料無帳的孤兒。
>
> 這正是 20260831 HHT-76 的實證：`grp=2` 內 `team=5..1`，前 4 個是 ASM 補回產生的單格 team，第 5 個是真 team，5 筆在 **0.5 秒內**被連續銷完（140~170 ms 一筆，遠短於實際取料動作時間）。

**⇒ 這條規則可以純靜態推導，不需要模擬。** 見 [staterecord-analysis LL-18](../ht9045-staterecord-analysis/SKILL.md)。

**20260831 最終修法（`908.16_NB_AI`，已模擬驗證通過）**

| 檔案 | 位置 | 動作 |
|---|---|---|
| `ainarm_SearchPickPlate.cpp` | L346 | 借料**還原成** `SetPlateSuck(iP,iPlateR,iPlateC,false)`（暫停）。⚠ 勿再改成銷帳 |
| `ainarm_SearchPlacePlate.cpp` | L4761-4764 | 補回走「查有帳→`SetPlateSuck(true)` 復活」，**原 4 格 team 原封不動** ← 正常流程走這條 |
| 同上 | L4765-4773 | 查無帳（異常）才走 `AppendStandaloneBooking()`；**必須自成獨立 group**，不可用 `UpdateHPSuckGroup()` 併進隊尾 |
| `Public/HTEditList.cpp` | `PurgePausedBookings()` | **新原語**：掃全帳清掉 `bSuck==false 且座標>=0` 的暫停帳（未使用吸嘴座標=-1，不會誤清）；team 全空刪 team、group 空且非隊尾刪 group |
| `main.cpp` | `ReStartAutoSiteMapping()` 入口 | 呼叫 `PurgePausedBookings()`。進出此函式＝一輪 ASM 開始或結束，**不會有 in-flight 借料**，是唯一安全的清理點。log：`ASM boundary : purge %d paused booking slot(s)` |
| `csystem.cpp` | `DoCleanOutFinishCheck()` | `ResetFile` 改為**條件式**：兩盤都 `HasRealIC()==false` 才清帳（原本無條件清，Clean Out 被提前結束時會把有料的帳清光 → 孤兒）|

**與 20260829「中期做法」的關係（常被問，先講清楚）**

中期做法有四項，被推翻的只有前兩項：

| 中期項目 | 現況 |
|---|---|
| ① 新原語 `ConsumeBookingByCoordinate`（按座標銷單格）| ⛔ **已整支移除**（全專案 0 筆呼叫） |
| ② 借料點 `SetPlateSuck(false)` → 銷帳 | ⛔ **已回退**成暫停 |
| ③ 補回改三分支（查有帳→復活／查無帳→補登／metadata 不齊→落 log）| ✅ **仍生效** |
| ④ 刪掉尾段**無條件**的 `SetPlateSuck(true)` 復活 | ✅ **仍生效** |

**③④ 與「借料是暫停還是銷帳」正交，所以不衝突**：③ 的分支 1 `HasBookingByCoordinate → SetPlateSuck(true)` 本來就是**復活**，正是暫停模型要的動作；它相對 905 的差別只是把「無條件復活」收斂成「查有帳才復活」，而 ④ 刪掉的那個無條件復活，功能已被分支 1 完整吸收。分支 2（`AppendStandaloneBooking`）在暫停模型下正常走不到，純粹是帳莫名不見時的防線。
⇒ 現在的碼＝**905 的暫停／復活模型** ＋ 中期留下的「復活條件化＋異常補登＋拒登留痕」 ＋ 20260831 的「ASM 邊界清暫停帳＋Clean Out 條件式清帳」。

⚠ **`bSuck==false 且座標>=0` 現在是「ASM 借出中」的唯一語意**，`PurgePausedBookings()` 就靠它判斷。目前很乾淨：`ReconcilePickFromHPList()` 清 `bSuck` 時**連座標一起清成 -1**（`ainarm_SearchPickPlate.cpp` L1524-1527 / L1537-1540），而且它在 `bRunAutoSiteMapping || rsmAutoSiteMap` 時直接 `return -1` 不執行，兩者不會打架。
**但這是往後的地雷**：任何人新增第三種「`SetPlateSuck(false)` 但保留座標」的用法，都會在下一個 ASM 邊界被靜默清掉。要加就得先給暫停帳一個可區分的標記。

**這組修法為什麼同時解掉幽靈與孤兒**

- **幽靈**（帳在料空）＝借了沒還。暫停帳留著 → `PurgePausedBookings()` 在 ASM 邊界清掉 → 消滅 Pattern #22。
- **孤兒**（料在帳空）＝帳被錯誤銷掉。借料不再銷帳、Clean Out 不再無條件清帳、補回不再併進隊尾 group → 三個來源同時堵住。
- **粒度不變**（§14.0 表格 ⑥）：team 從頭到尾沒被拆過。

**⇒ 動 HP 帳本前的五問（靜態檢查表，通過才動手）**

1. 這個原語**保不保留座標**？保留＝暫停（可逆）；不保留＝銷帳（不可逆、粒度資訊永久遺失）。
2. 借與還**粒度對不對稱**？逐格借 vs 整 team 銷 = 不對稱 ⇒ 必壞。
3. 新建的 team 會落在**哪個 group**？會不會被 `ExtractLastGroup()` 撈到、污染正在累積的那一趟？
4. 這樣做之後 `GetTeamCount()>1` 會不會成立？成立＝**多一次 Z 下**，那一次有沒有料可吸？
5. 中斷路徑（One Cycle / Clean Out / 關站 / Home）有沒有**對應的收尾清理**？兩段式交易沒有 rollback 就是缺陷。

### 14.1 設計原則（機台鐵則，RogerYang 20260801 確認）

1. **HotPlate 上的料必定是 InArm 從 Loader 搬來的。** 物料流固定為 `Loader → HP → Shuttle → Index → 下游`，**測試後的料不會回 HP**，Index Arm 不會把料放回 In Shuttle。
2. **放料時記錄「哪支吸嘴搬來的」，取走時只能用同一支吸嘴。**
3. 這條規則的**實際載體是 `PickFromHPList` 的 team**（`bSuck[i][j]` + `iPlateR/C[i][j]` + `iSite[i][j]`），
   **不是** `iRowOnHotPlate`（那只是顯示欄位，全程式唯一讀者是 `main.cpp` State Record 的 `HP*_Row.xls`；`(iSuckRow+1)*10+iSuckCol+1`，Aa~Ad=11~14）。
4. 推論：**InArm 要不要去 HP 取料、取哪一格，100% 由帳本決定，不是掃盤。** 盤上有料但帳本沒登記 = 那顆永遠不會被取走。

### 14.2 帳本讀寫點（`Public/HTEditList.cpp`，V3.33.908.7 行號）

| 動作 | 函式 | 位置 |
|------|------|------|
| 放料建 team | `UpdateHPSuckGroup(iP,iR,iC,iSht,iKit)` | 各模式檔 `DoPlaceToHotPlate` **case 200** |
| 放料寫位置/吸嘴/Site | `SetArrPlateXY(iSuckRow,iSuckCol,iP,iR,iC,iSite)` | `DoPlaceToHPSwapData()` `ainarm_SearchPlacePlate.cpp` |
| 放完追加空 Group | `AddHPSuckGroup()` | 各模式檔 **case 501** |
| 取料讀 anchor | `GetHPFirstTeamPlate()` | `SearchPlateToPick()` → `iPickPlate/Y/X[0]`、`iWhichShtPickFor32`、`iWhichKitPickFor32` |
| 取料讀哪支下 Z | `GetHPFirstTeamMotUse()`（依 `iSite[i][j]>0`） | `MoveInArmXYPickHotPlate_9045()` / 取料 case 100 |
| 取料讀per-sucker座標 | `GetHPFirstTeam()` → `HotplateDataConversion()` | 取料 case 300 |
| 取完前移/刪 team | `DataForwardAndNextTeam()` | 取料 **case 350** |
| 落地檔 | `SaveFile/LoadFile` → `D:\HT9045\system\PickHPRec.json`、`PickHPRecException.json` | — |

⚠ **`UpdateHPSuckGroup` 與 `SetArrPlateXY` 都作用在「最後一個 Group 的最後一個 team」。** 從放料流程以外的地方（例如 gate `HasHotReadyIC_9045`）呼叫它們，會污染正在進行中的放料回合（case 200 建了 team、case 350 還沒寫位置的空窗）→ **當場再製造一顆無帳孤兒**。要在別處插入 team，必須自建 group 再 `InsertHPGroup(0, ...)`。

### 14.3 取料幾何：anchor 與吸嘴 X 位移（重要，避免反推各模式 pitch 公式）

```
iC = team->iC (anchor);  iR = team->iR
iHPXPos = Prod.XInArm_PlateN_Pick[iInArmYBase][iInArmXBase] + Prod.HotPlateForm[0].iXPitch*iC
iHPYPos = Prod.YInArm_PlateN_Pick[iInArmYBase][iInArmXBase] - Prod.HotPlateForm[0].iYPitch*iR
```
接著 `MoveInArmXYPickHotPlate_9045()`（`ainarm_SearchPickPlate.cpp` L584-665）**依「第一支有效吸嘴」自動做 X 位移補償**，機型基準分三種：`USE_PICKER_COUNT==ep16Picker`→基準 Ad、`USE_IN_OUT_ARM_Y_PITCH==iXYPitchBb/iXYPitchIn_Bb_Out_Bc`→基準 Ab、其餘→**基準 Ac**（Aa 需 `+2×pitch/3`、Ab `+1×`、Ac `0`、Ad `-1×`）。

→ **單顆 team 的 anchor 可以直接用「該格自己」，吸嘴身分由 `bSuck` 帶即可，不必反推 `GetPlaceToHotPlateCol()` 那一堆模式條件。** ASM 的單顆取料路徑就是這麼做的（L483-494：`iR=iAutoSiteMapHPR; iC=iAutoSiteMapHPC; iPickPlateX[0]=0;`，含 RogerYang 20250709「重新計算 pitch，避免用錯 col 位置造成 Pitch 錯誤」修正）。
⚠ 但**不可**因此改用 Aa 取料：Aa 取最右欄要多跑 `+2×pitch/3`，會踩 X soft limit（同 WAR0152/0154 家族，見 §13）。**必須用原吸嘴。**

### 14.4 缺陷：ASM 期間從 Loader 補料到 HP 不寫帳本（20260731 偉測 HHT-30 根因）

> ⚠ **20260829 補充定位**：本節 20260731 的修法（無條件補建 team）後來證實**違反冪等**——常態下（原 booking 還在、只是被借出暫停）它會疊出第二筆帳，是 HHT-80「2.5 小時 dup=25」的直接源頭（見 §14.0 錯亂③）。**正確修法＝先查 `HasBookingByCoordinate` 查無此格才登記**（20260829 冪等版），本節保留作歷史脈絡與 #16 機制說明。

`DoPlaceToHPBackupData()`（`ainarm_SearchPlacePlate.cpp` L4719）**不是「把料放回 HP」**，是「**用 ASM 備份的 Sht/Kit/HotCount 資料**把 Loader 新料放到 HP」——函式名是 `Backup**Data**`，原註解也是「將 Auto Site Mapping **資料**寫回到HP」。

三個判斷用了**兩個不同旗標**，導致帳本三個寫入點全部被跳過：

| 階段 | 判斷旗標 | 結果 |
|------|---------|------|
| case 200 建 team | `rsmAutoSiteMap && bAutoSiteMapHasPickHP==true && bUSEJCETSiteMapMode`（**25 個模式檔一致**）| 跳過 `UpdateHPSuckGroup` → **無 team** |
| case 350 寫位置 | `bAutoSiteMapHotplateSave==true` → 走 `DoPlaceToHPBackupData()` | 該函式**從不呼叫 `SetArrPlateXY`** → **無位置** |
| case 501 追加 Group | 同 case 200 條件 | 跳過 `AddHPSuckGroup` |

兩旗標都在「ASM 從 HP 取料」時一起被設 true（`CopyFromTray` `mykitsuck.cpp` L1583-1594 需 `iSuckData==HAS_HOT_IC`；`bAutoSiteMapHasPickHP` 在 `ainarm_SearchPickPlate.cpp` L349 / 取料 case 300 L1019-1021），但**清除點不同**，於是 ASM 後續的 Loader 補料就掉進這個縫。

**症狀鏈**：料實體在 HP、帳本零登記 → ASM 結束切回正常模式（取料改由帳本驅動）→ 該顆永遠不被選中 → 撐到批尾、帳本最後一個 team 被吃完 → gate `HasHotReadyIC_9045` 進 dead-end（`No GetHPFirstTeamMotUse`）→ **停機、Clean Out 卡 `iCleanOutCycleTask=1001`（`MOT[MMPlate1].HasIC()` 恆真）→ 只能關程式選「不保留上次資料」**。

**修正（V3.33.908.7，20260731）**：在 `DoPlaceToHPBackupData()` 的 metadata 三行之後補建 team 並登記原吸嘴：
```cpp
    PickFromHPList->UpdateHPSuckGroup(iP, iPlateR, iPlateC,
                                      iHotWhichShuttle[iP][iPlateC][iPlateR],
                                      iHotWhichKit[iP][iPlateC][iPlateR]);
    PickFromHPList->SetArrPlateXY(iSuckRow, iSuckCol, iP, iPlateR, iPlateC,
                                  MOT[MMPlate1+iP].Tray.iWhichSite[iPlateC][iPlateR]);
```
- 走到此處必定 `InArmSuck.Item != HAS_NULL_IC`（L4726-4733 早退已濾掉）⇒ 借走時必定設過 `bAutoSiteMapHasPickHP=true` ⇒ case 200 必定跳過 ⇒ **不會重複建 team**。
- Sht/Kit 直接讀前三行剛寫入的陣列（單一資料來源，帳本與 metadata 必定一致）。
- 吸嘴用本函式參數 `iSuckRow/iSuckCol` = 實際放料的那支 ⇒ 符合 §14.1 原則。
- **不加客戶碼開關**：這是通用的 HP 取放料規則，任何開 ASM 的機台都會踩。

⚠ **必須配的 guard**：若 `bRunAutoSiteMapping` 已關但 `bAutoSiteMapHotplateSave` 尚未清（兩者清除點不同，`bRunAutoSiteMapping` 由 `ainarm9045.cpp` L4463-4475 每個 pass 依 6 個條件重算），`CopyToTray` 的 ASM 分支會被跳過 → `iAutoSiteMapHotplatePlateC/R` 是**殘值** → L4734-4736 的 metadata 寫到錯格 → 上面兩行會建出 `iSht/iKit=-1` 的 team 並**真的被執行**。故登記前要驗資料有效性：
```cpp
    if(iHotWhichShuttle[iP][iPlateC][iPlateR]>=0 &&
       iHotWhichKit[iP][iPlateC][iPlateR]>=0     &&
       MOT[MMPlate1+iP].Tray.iWhichSite[iPlateC][iPlateR]>0)
    { ...上面兩行... }
```

### 14.5 同區域其他已知陷阱

| 陷阱 | 說明 |
|------|------|
| `GetHPFirstTeamToList()` 對「有 group、0 team」回 **true** | 點位清單是空的 → 上層 `HasHotReadyIC_9045` 判 `bHasIC=false` → 進「發現多餘的組別」分支 → `GetHPFirstTeamMotUse` 回 false → 早退、**不做自我修復**，空 group 永遠留著 |
| ⚠ `GetHPFirstTeamToList()` **攤平整個 group 0 的「所有 team」**，不是只取第一個 team | `Public/HTEditList.cpp` L3087 `for(i<HPGroup->GetTeamCount())` 逐 team 把 `bSuck==true` 的格全部加進 `lsPoint2D`。**但真正取料只吃第一個 team**（`ExtractFirstTeam` / `GetHPFirstTeamMotUse` / `GetHPFirstTeamPlate`）。⇒ **gate 的判定範圍 ⊃ 動作範圍**：group 0 內只要有**任何一個** team 指到空格，整個 gate 就永遠過不了（見 §14.6） |
| `GetHPFirstTeamPlate()` 空 group 空轉 | 迴圈內 `HPGroup->DataForward()` 對空 TeamList 是 no-op → 空 group 刪不掉、空轉到 `iLimitcount=500`（Pattern #9） |
| `AddHPSuckGroup()` 的 guard | 上一個 group 為空就 `return` 不新增（RogerYang 20250814），且 caller `new` 出來的 group **洩漏** |
| `DelHPSuckGroup()` | 全程式**零呼叫點**（死碼）；`TList::Delete` 不 free 物件 |
| `lsPoint2D` 洩漏 | `HasHotReadyIC_9045` 的 `TList::Clear()` **不釋放 item**，`GetHPFirstTeamToList` 是 `new uPoint2D` → 每次呼叫全漏；`bReady==false && bHasIC==true` 早退（soak 等待期間每個 pass 都走）連 TList 也漏。**V3.33.908.7 20260801 已修**（新增 `static FreePoint2DList()`，三個出口統一呼叫） |
| State Record 會清 HotTime | Hot mode 抓 State Record 時 `ProcessICHotTime(true)` 被連呼 6 次、同秒 `P=0` → **HP 全盤 `HotTime` 歸零**、`HAS_HOT_IC` 降回 `HAS_IC`。⇒ `HP*_HotTime.xls` 永遠是 0，**不可用來判斷加熱是否完成**；且抓完 record 後 HP 上的料要重新 soak |
| `TMyTray::HasIC()` 把 `HAS_NULL_IC` 當有料 | `mytray.cpp` L183 只判 `Data[iC][iR]` 非 0；`HasRealIC()` 才排除。⇒ HP 只剩佔位符也會讓 Clean Out 卡 1001。工具：`TMyTray::ClearNullIC()` |

### 14.6 缺陷：幽靈帳（ASM 結束後帳本指到空格）→ `HasHotReadyIC_9045()` 永遠回 false（20260826 偉測 HHT-10 根因）

> 這是 §14.4 的**鏡像**：§14.4 是「盤上有料、帳本沒登記」＝**孤兒**；本節是「帳本有登記、盤上是空格」＝**幽靈**。
> 兩者同源（ASM 取放料不走帳本），但症狀、判別、修法完全不同。對應 [Pattern #22](../ht9045-staterecord-analysis/references/deadlock-patterns.md)。

#### 成因（三段，缺一不可 —— 這也是「為什麼大部分情況都沒事」的答案）

**① ASM 的 HP 取料確實不消耗帳本（碼級確認）**
`DoInArmPickFromHotPlate_9045()` **case 350** 明文跳過帳本：
```cpp
case 350:
    if(LastSet.iRunStartMode==rsmAutoSiteMap &&
       CosFunction.bUSEJCETSiteMapMode==true)                                   //Steven 20220527
    {
        Task=400;                          // ← ASM: 直接跳過, 不碰帳本
    }
    else
    {
        if(PickFromHPList->DataForwardAndNextTeam()) { Task=1; break; }          // 只有正常生產才 pop
        else                                          Task=400;
    }
```
且取哪一格是**純掃盤**（`SearchPickPlateXItem_AutoSiteMap()`：找 `iWhichSite==iAutoSiteMapSiteNo && Data==HAS_HOT_IC`），與帳本無關。

**② 但設計上「哪邊取料放回哪邊」，所以正常情況帳本仍然正確 —— 這才是它平常不出事的原因**
- 放料 anchor 被強制改成 ASM 借料的那一格：`ainarm_SearchPlacePlate.cpp` **L1801-1812**
  `if(bUSEJCETSiteMapMode && bI21AutoSiteMappingUseHotplate && bAutoSiteMapHasPickHP) { iP=iAutoSiteMapHPNo; iR=iAutoSiteMapHPR; iC=iAutoSiteMapHPC; }`
- `CheckHasSpaceToPlace_9045()` L619 `if(bRunAutoSiteMapping && SiteMapData[j][k]==1) return true;`
  註解直接寫「**Site Mapping 取料後需補回IC**」（Ifor 20210426）
- `SearchPlateToPlace()` L4504 註解「**Auto Site Mapping Hotplate 哪邊取料放回哪邊**」（Ifor 20170929）

⇒ **借走 → 從 Loader 補一顆回同一格**，那筆 booking 又變成有效（Sht/Kit 用 `iWhichShuttleBackup`/`iWhichKitBackup` 還原）。**借還相抵，不留幽靈。**
⇒ 所以這功能用了很多年沒事，**不是「每跑一次 ASM 就壞一次」**。

**③ ⚠ 已否決的說法：「ASM 收尾的 One Cycle 讓最後幾站補不回來」**

曾一度把幽靈歸因於「`AutoSiteMappignCleanOut()` 在 ASM 收尾呼叫 `BtnOneCycleClick()`（`main.cpp` L28603-28619），One Cycle 不再取新料 ⇒ 尾段借走的補不回」。**這個說法站不住，已撤回**，兩個反證：

1. **ASM 完成本來就一定觸發 One Cycle**：`DoSiteMappingCHK()` [`csystem.cpp` L20948-20952] 在 JCET 分支 `if(iDoSiteMappingStep>=CT) { fMain->BtnOneCycleClick(fMain->BtnOneCycle); bASMFinishOneCycle=true; }` —— **每一次 ASM 都跑**，不能當「為什麼只有這次壞」的差異因子。
2. **數量對不上**：ASM 每站約 60 秒，借→測→補回在下一站之前早已完成，最多只有「最後 1 格」來不及，不可能是 4 格。

**④ 幽靈的真正來源：目前未定（不要編）**

20260826 帳本實測有**兩種**殘留，機制不同：

| 殘留 | 內容 | 這些格子現在有沒有料 | 判讀 |
|---|---|---|---|
| **4 筆幽靈**（`G0/T1`、`G0/T2`）| `(c4,r0)(c5,r0)` site 8,7；`(c4,r1)(c5,r1)` site 6,5 | ❌ 空 | 卡住隊首 ⇒ 本次死結的直接原因 |
| **8 筆重複**（`G7` 的 T0~T7 與 T8~T15 完全相同）| `(c0~c3,r1)` site 1~4 + `(c0~c3,r2)` site 8,7,6,5 | ✅ 都有料 | 依 `SearchPickPlateXItem_AutoSiteMap()` 的掃描順序（`iR` 由 0 遞增、`iC` 由 0 遞增、首個 `HAS_HOT_IC` 命中即取），這正是當下 ASM 會選到的 8 格 ⇒ **借還成功，但登記了兩次** |

另一條硬證據：**熱盤 10 格空，其中 8 格恰好是「一輪 ASM 所需的 8 個 site」的完整集合**
（`row0 c0~c3`＝site 1,2,3,4；`row0 c4,c5`＝site 8,7；`row1 c4,c5`＝site 6,5）
⇒ 高度指向「**某一輪 ASM 借走 8 顆沒有補回去**」；但同一批 8 格中只有 4 格的 booking 還活著、另 4 格（`c0~c3,r0`）帳已被消掉 —— **同一批處理結果不一致，原因未明。**

**決定性證據＝ASM 動作 log**：`slAutoSiteMapLog`（`main.cpp` L1634 `new TMyStringList("D:\\HT9045_Log\\ASM", ...)`，`TByDay`）逐筆記 `P, R, C, 吸嘴名, Pick/Place`（寫入點：`ainarm_SearchPickPlate.cpp` L190/246/324、`ainarm_SearchPlacePlate.cpp` L4794、`uhome.cpp` L1324）。
⚠ **`DoStateRecord` 沒有收集這個資料夾**（擷取包只有 `EventLogTxt / Galil_LOG / GPIB9045 / GPIBLOG / HT9045 / MNetLog`）→ 追這類問題要**另外**跟客戶要 `D:\HT9045_Log\ASM\`；建議把它加進 State Record 收集清單。

**⑤ 經驗相關性（只是觀察，未證實為機制）**

20260826 同機同日三次 ASM：

| 時間 | 前置 | 結果 |
|---|---|---|
| 16:34:34 **Initial Start**（`InitialStartTask` 50→200）→ ASM#1 16:38:48~16:46:14 | 空機 | ✅ reset 16:46:30 → **16:46:31 立刻恢復取熱盤**跑到 16:54:43 |
| 18:49:36~18:50:05 **One Cycle** → ASM#2 18:51:25~18:58:51 | 機內有料 | ❌ reset 18:59:07 後**再也沒取過熱盤**（19:00~20:41 零產出、EventLog 零事件）|
| 20:41:18 **One Cycle** → ASM#3 20:42:57~20:50:24 | 機內有料（熱盤 56 顆、帳本 8 group）| ❌ 20:50:59 起 183 秒零動作＝本案 |

⚠ 這只是**相關性**：「空機跑 ASM 比較安全」目前沒有碼級機制支撐（唯一沾得上邊的是空機時幽靈較可能整團落在同一 group，因而能走 `bHasIC==false` 自我修復分支）。
⚠ ASM#2 那班的 EventLog 不在擷取包內，「ASM#2 也是同一死結」屬高度可能但未證實。

**⑥ 重複 booking 為什麼也要清**：重複帳的第一份被取走後，第二份就指到空格 ⇒ **同一死結會延後復發**。所以對帳必須連重複一起清。

**為什麼永遠解不開（三個條件缺一不可）**：

```cpp
// ainarm_SearchPickPlate.cpp  HasHotReadyIC_9045()
for(int i=0; i<lsPoint2D->Count; i++)      // ← ①清單 = group 0 的「所有 team」攤平（見 14.5）
{
    if(Data[iC][iR]!=NULL_IC) bHasIC=true; // ← ②bHasIC 是累加式(sticky)，前面有料就永遠 true
    if(bHasIC && HotTime[iP][iC][iR]>=Prod.iHotTime) { } else bReady=false;
                                           //   幽靈格 HotTime 被 ProcessICHotTime 強制歸 0 → 恆 false
}
if(bReady==false && bHasIC==true) return bReady;   // ← ③早退，跳過下面唯一的自我修復分支
if(bHasIC==false) { ...清爛帳/跳下一團... }         //   永遠進不來（②讓 bHasIC 恆 true）
```

**死結全貌**：熱盤同時滿足「`iCT <= PlactCT`＝**放不下**」（見 §15.2）與「gate 恆 false＝**取不走**」→
`iArmTask` 在 **50 → 500 → 600 → 50** 全速空轉；case 500 的 else 每個 pass 設 `bHangTimePause=true` → **看門狗被永久暫停、零 alarm、零自動存檔**；
下游 Shuttle / Index / OutArm 全部空等 → 畫面綠燈 `Running`、UPH=0。
**HOME 沒用**（不清 `Tray.Data`、不清帳本），**Clean Out 也沒用**（clean-out 分支同樣要先過 `HasHotReadyIC_9045()`），
**重開機答「機台內有料」會把爛帳原封讀回**（`cinitial.cpp` L8427）。

**現場恢復（不改碼）**：人工取走熱盤上的料 → 關程式 → 開機選**「不保留上次資料」**（走 `cinitial.cpp` L8432 `ResetFile`）；或程式關閉時直接刪 `D:\HT9045\system\PickHPRec.json`。

### 14.7 帳本三種壞法與「對帳（reconcile）」原則

| 壞法 | 定義 | 症狀 | 對策 |
|---|---|---|---|
| **孤兒** | 盤上有料、帳本無 booking | 該顆永不被取 → 批尾 `No GetHPFirstTeamMotUse` 停機、Clean Out 卡 1001 | §14.4（放料端補登記） |
| **幽靈** | 帳本有 booking、盤上是 `NULL_IC` | gate 恆 false → 50/500/600 空轉、零 alarm | §14.6（對帳刪除） |
| **重複** | 同一 `(P,col,row)` 出現在多個 team | 第一份被取走後，第二份就變成幽靈 → 同一死結延後復發 | 對帳去重（留最靠隊首那筆） |

**對帳原則：只做減法，不要「重建帳本」。**

- **零資料丟失的理由**：對帳只把不該存在的 booking 的 `bSuck` 設 false，**全程不重新推導吸嘴 slot**，存活的那筆本來就帶著原始 (row,col) ⇒ §14.1 的「哪支放的哪支取」不受影響。
- **重建會丟什麼（實測過的四項）**：
  1. 🔴 **group / team 邊界**。一個 group = 8 個吸嘴 slot，但**可以橫跨兩趟**（20260826 實例：G1 = `iHotPlateCount` 26+27、G2 = 28+29，而 G3~G6 是單趟 8 格）。`iHotPlateCount` 只能告訴你「哪些格同一趟」，**推不出「哪兩趟同一 group」**；猜錯會組出物理上一趟到不了的 group → 製造新 hang。
  2. 🔴 **metadata 不齊的格子**。`iRowOnHotPlate`（吸嘴 slot 的冗餘備份，`(iSuckRow+1)*10+iSuckCol+1`）**只有 `DoPlaceToHPSwapData()` L4697 會寫**；ASM 路徑的 `DoPlaceToHPBackupData()` **不寫** ⇒ 那些格重建時解不出吸嘴，只能丟掉（＝製造孤兒）或報警。
  3. 🟡 `PromoteTeamToFront()`（Pattern #21 逃生插隊）的優先序。
  4. 🟢 FIFO 先後（只影響 soak 順序，不會取錯吸嘴）。
- **冗餘備份可信度（20260826 實測）**：把 `PickHPRec.json` 的 68 筆 booking 對 `iRowOnHotPlate` / `Tray.iWhichSite` / `iHotWhichShuttle` / `iHotPlateCount` 逐筆比對，**56 格全部 100% 一致、零筆不符**；缺陷是**只多不少**（4 筆幽靈 + 8 筆重複，沒有任何一格漏登記）⇒ 純減法即足夠。
- 這四個陣列都會存進 `machinerecord.dat`（`cinitial.cpp` L7943-7947 寫 / L8393-8397 讀），`Tray.iWhichSite` 走 `MachRec.iWhichSite`（L7833 / L8217）⇒ **重開機不會掉**。

⚠ 對帳的執行閘門（缺一不可，否則會刪到 in-flight 的 team）：
`InArmSuck.HasIC()==false`、`bPlaceToHotplate==false`、`bPickFromHotplate==false`、`iInArmPickFromHotPlateTask==1`、`iInArmTryPickFromHotPlateTask==1`、`bRunAutoSiteMapping==false`、`LastSet.iRunStartMode!=rsmAutoSiteMap`。
**不可用 `iInArmPlaceToHotPlateTask==1` 當閘門** —— 成功路徑 `return` 不 reset，它會停在 **501**（LL-7），加了就永遠對不了帳。

---

### 14.8 缺陷：一趟取料跨越模式切換 → `case 350` 跳過銷帳（20260909 偉測 HHT-139 根因，**已修**）

`DoInArmPickFromHotPlate_9045()`（`ainarm_SearchPickPlate.cpp`）用**同一個式子**
`LastSet.iRunStartMode==rsmAutoSiteMap && CosFunction.bUSEJCETSiteMapMode==true`
在 4 個 case **各自即時判一次**「這是不是 ASM 去熱盤借料」：

| case | 這個式子決定什麼 | 908.18 行號 |
|---|---|---|
| 100 | 吸 **1 顆**（ASM 借料）還是 **整團 8 格**（`bZFlgToHP` / `bZFlgToHPPick`）| L849 |
| 300 | 資料轉換：1 顆 vs 整團 ← **資料語意真正 commit 的那一點** | L1169 |
| 330 | JAM0109 **SKIP** 要寫哪一格 | L1260 |
| 350 | **要不要銷帳**（`DataForwardAndNextTeam()`）| L1324 |

**缺陷**：`LastSet.iRunStartMode` 會在**一趟取料進行中**被合法改掉 ——
`IniConfig.bDutOnOffNeedASM=true`（偉測開著）時，**暫停期間關一個 site，程式自己**就把模式切成 Site Mapping Check
（EventLog：`Trigger Auto Site Map after site on.` → `Change Start Mode`，**無操作員按鍵**）。
而 **PAUSE 不會結束那一趟取料**，`iInArmPickFromHotPlateTask` 原地凍結、START 後從中段續跑。

於是：
```
case 100 / case 300  模式=Continuous Start -> 按「整團 8 格」處理，8 顆料真的被吸走
        [PAUSE -> 關站 -> 程式自動切 Site Mapping Check -> START]
case 330 / case 350  模式=Site Mapping Check -> 按「ASM 單顆借料」處理
                     => case 350 走 Task=400，跳過 DataForwardAndNextTeam()
```
⇒ **料走了、帳留著＝整整一團 8 格的幽靈帳**（§14.7 的「幽靈」，但來源與 §14.6 完全不同）。

**證明帳沒銷**：`uPlateInfo::DataForwardAndNextTeam()`（`Public/HTEditList.cpp`）在 `GetTeamCount()==1` 會
`DataForwardAndDelete(HPSuckGroupList)` **刪掉整個 group**。每個 group 只有 1 team、group 卻還在 ⇒ 從未被呼叫。

#### 修正（20260909，`V3.33.912.0_20260908_RogerYang_AI`，未編譯未上機）

加函式內 `static bool bTripASMHPBorrow`，**在 `case 300` 定案**，`case 330 / case 350` 沿用：

```cpp
    static bool bTripASMHPBorrow=false;   // 本次取料是否為 ASM 熱盤借料

case 300:
    ...
    bTripASMHPBorrow=(LastSet.iRunStartMode==rsmAutoSiteMap &&
                      CosFunction.bUSEJCETSiteMapMode==true);
    if(bTripASMHPBorrow)          // 原: if(LastSet.iRunStartMode==... && ...)
case 330:   if(bTripASMHPBorrow)  // 原: 同上
case 350:   if(bTripASMHPBorrow)  // 原: 同上
```

⚠ **latch 必須放 `case 300`，不能放 `case 100`**（設計級陷阱，20260909 動手前才查到）：

`MoveInArmXYPickHotPlate_9045()`（決定手臂 XY 目標與 `bZFlgToHP`）有一個**跨函式呼叫者**
`ainarm9045_2x2_8_Hot.cpp` 的 `case 700`，它不屬於這條取料流程 ⇒ **那一處的即時判別式改不了**。
latch 若從 `case 100` 起算，就會變成：`case 100` 用 latch（整團）→ `case 110/190` 的 XY 移動用即時模式（ASM 單顆座標）→ `case 300` 用 latch（整團轉換 8 格）
⇒ **手臂在 ASM 座標、卻把整團 8 格記成已取**，比原病更糟。

放 `case 300` 則：
- `case 100` — 不動，維持原行為
- `HotplateDataConversion()` 內的即時判（含 `SetPlateSuck(...,false)` 借料暫停）— 不動；它的專案內呼叫者**只有 `case 300` 那兩處**，永遠與 `case 300` 同一次執行，不會不一致
- `MoveInArmXYPickHotPlate_9045()` — 不動
- `case 300` 必定在 `320/330/340/350` 之前執行；`InitInArmPickFromHotPlateTask340()` 的續做路徑沿用同一趟的決定，也是對的

⇒ **除了 `case 330 / 350` 兩處，其他流程與原本逐位元相同。**

#### 附帶缺陷（同一個錯誤判別式）

`case 330` 的 SKIP 分支在非-ASM 趟誤走 ASM 路徑時會呼叫
`PorcessJAM0109HotPlatePickUpErrorSkip(iAutoSiteMapInArmRow, iAutoSiteMapInArmCol, iAutoSiteMapHPNo, iAutoSiteMapHPR, iAutoSiteMapHPC)`
—— 而 `iAutoSiteMapHPNo/R/C` 的**初值是 `0` 不是 `-1`**（`cmydef.cpp`），且只在 `SearchPickPlateXItem_AutoSiteMap()` 被寫入
⇒ 若這一輪 ASM 從未借過熱盤料，就會**用殘留初值把 SKIP 寫到熱盤 `P0(R0,C0)`**，而不是真正吸取失敗的那一格。
latch 修好後這條路徑不會被誤走；要不要把初值改 -1 並加防護是另案（動全域）。

#### 症狀與判讀
死結簽章、帳實對帳數字、與 §14.6 的分家方法、重現步驟，見
**[staterecord-analysis Pattern #26](../ht9045-staterecord-analysis/references/deadlock-patterns.md)**。
最快的分家：**`D:\HT9045_Log\ASM\ASMLog_*.csv` 在熱盤歸零後是否為空**（空 ⇒ 與 ASM 無關 ⇒ 走本節），
以及帳實差額是不是「**整整一團 8 格**、孤兒 0、重複 0」。

---

## 15. 加熱模式「熱盤填滿才取」的取放優先權 —— 以及 RT 小批量為何永遠不出料

> 來源：偉測 HHT-79 20260802 案（RT 投入 133 顆、產出 0 顆）。**這一節的結論是「設計如此，不是缺陷」**，寫下來是為了避免下次又把它當 bug 追。

### 15.1 case 50 的取／放決策（`ainarm9045_2x6_8.cpp`，其他模式檔對稱）

```
case 50:
    ...
    Task=75;                                      // ★ 預設：去 Loader 取新料
    ...
    if(iCleanOut || iOneCycle) { ...clean-out 專用分支（見 15.3）... }

    if(LastSet.iTemperature==Tempture_Hot &&
       CheckHasSpaceToPlace_9045()==false)         // ★ 正常生產下，唯一切換到「取熱盤」的條件
        Task=500;                                  //   → case 500 → HasHotReadyIC_9045() → Task=1500 取熱盤
```

**正常加熱生產（`iCleanOut==0 && iOneCycle==0`）下，只有「熱盤放不下了」才會開始取熱盤。**
這是「用整片熱盤當 soak buffer、填滿才開始取」的設計：填滿的過程本身遠超 soak time，取出來的都早已 soak 完，UPH 不受 soak 限制（pipeline）。

### 15.2 `CheckHasSpaceToPlace_9045()` 的判準（`ainarm_SearchPlacePlate.cpp`）

數熱盤上 `NULL_IC` 的格數 `iCT`（**只數 `==NULL_IC`；`HAS_NULL_IC` 佔位符不算空格**），與門檻 `PlactCT` 比，**`iCT > PlactCT` 才 return true（還有空間）**。

⚠ `PlactCT` **不是固定的 8**，是依 picker 型式 / `iInArmType` / `XDivision` 分支（`ainarm_SearchPlacePlate.cpp` L625~734）：

| 分支條件 | `PlactCT` |
|---|---|
| `USE_PICKER_COUNT==ep1Picker` | 0（要放滿） |
| `e9045_1x1_1` / `e9045_1x4_1_Ac` | `CC_ASE_SG` → 0；其餘 → 1 |
| `e9045_1x2_2_13` / `_2_14` | `(XDiv*YDiv)%2==0` → 0；否則 4 |
| `bUseAxExPicker()` | `2x2_4_13` 且 6×11 → `iPlateSelect==3 ? 12 : 6`；其餘 → `2*InArmSuck.iPickRow` |
| `bUseAxxGPicker()` | `XDiv==6`→6；`XDiv==3 && TriSite1X3 && XPitch<=iXpitchMaxX3/2`→3；`1x2_2_14 && XDiv==2`→2；其餘→4 |
| 其餘（多吸嘴）`XDiv ∈ {4,8,10,12,16}` | `b12x16HP_2x6`→6；`iPickRow==1 && XDiv==iPickCol`→`iPickRow*iPickCol`；其餘→**8** |
| 其餘（多吸嘴）`XDiv==6` | 寬版且 `1x3_4/2x3_6/2x6_8`→`3*iPickRow`；`iPlateSelect==3`→（`iCloseSiteState` 1/2 或 `1x4_4` ? 8 : 12）；**其餘→10** |
| 其餘 `XDivision`（如 2、5、7…） | **未賦值＝0** → 只要有 1 格空就算有空間 |

⇒ 判「熱盤到底放不放得下」時**必須先把 recipe 的 `X Division` / `Use Wide Hotplate` / `iInArmType` 代進去算 `PlactCT`**，不能一律套 8。
> 實例（20260826 偉測 HHT-10）：6×11 寬版 + `e9045_2x4_8`（8 吸嘴）→ `PlactCT=10`；當下盤上剛好 **10 格空** → `10 > 10` 為假 → **判定沒空間**，只差一格。這種「剛好卡在門檻上」的狀態一旦搭配取料端卡住，就是死結（見 §14.6）。

### 15.3 RT（複測）小批量為何永遠不出料

| 項目 | 偉測 HHT-79 實例 |
|---|---|
| 熱盤容量 | 12 × 24 = **288 格**（`Use Wide Hotplate=1`） |
| RT 投入量 | **133 顆**（一盤複測料） |
| 結果 | 空格永遠 ≫ 8 → `CheckHasSpaceToPlace_9045()` 恆 true → `Task` 恆 75 → **只放不取，永遠不開始測** |

實測佐證：09:06–09:18 `InArmPlaceToHotPlateTask` 循環 200+ 次、`InArmPickFromHotPlateTask` **0 筆**、`InArmPlaceToShuttleTask` **0 筆**；GPIB 同期 `START TEST` **0 次**。

**FT 為什麼正常**：料源源不絕，很快填滿熱盤 → 開始取 → 正常 pipeline。

### 15.4 程式已提供的兩個機制（優先用這兩個，不要改通用邏輯）

| 機制 | 位置 | 行為 |
|---|---|---|
| **CLEAN OUT** | case 50 的 clean-out 分支（`iCleanOut && iOneCycle==0 && Hot && 熱盤有料` 且 Loader/Buffer 無盤 → `Task=500`；以及「沒空間或沒新料就取熱盤」） | **設計上的正常收尾流程**：RT 料跑完按 CLEAN OUT，機台把熱盤的料測完出料 |
| **`[E40] Clear all hot IC then pick load IC`**<br>（`config.ini` `"In/Out Arm"` 區 `bClearAllHotICThenPickLoadIC`） | case 1100 尾段：`iOneCycle==0 && bE40… && CheckClearAllHotICThenPickLoadIC() && HasHotReadyIC_9045()` → `Task=1500` | **放完一組就去取熱盤**，不必等填滿 |

- 「必須手動清機」對 RT 而言**是正確操作**，不是故障。要跟現場說明：RT 料跑完按 **CLEAN OUT**（不是 ONE CYCLE —— ONE CYCLE 走 `CheckOneCycleAction()` 另一條路，不保證取熱盤）。
- `[E40]` 是 **config.ini 全域設定（非 by recipe）**，開了對 FT 也生效 → 必須做 FT 的 UPH / soak 迴歸。溫度安全性無虞（`HasHotReadyIC_9045()` 仍強制 soak 滿才取），但 buffer 深度會變。
- ⚠ **CLEAN OUT 的取熱盤路徑會被 Pattern #17 擋死** —— 若 InArm 在按下 CLEAN OUT 後卡進 RotateKit case 1160，clean-out 根本沒機會執行。追「按了 CLEAN OUT 還是不出料」時先排除 #17。

### 15.5 ⚠ 判讀陷阱：`PickHPRec.json` 的 `Site=-1` **不是**「熱盤格空著、team 破碎待補」

**規則：HP 上的料，怎麼放就怎麼取；哪隻吸嘴放料，就是哪隻吸嘴取料。** team 一建立就是最終組成，**永遠不會「湊」**。

程式落實：

| 欄位 | 語意 |
|---|---|
| `Site[i][j]` | 該熱盤位置對應哪個測試 site；`-1` = 這輪這支吸嘴**沒放料**（或該 kit 結構上不用這支吸嘴） |
| `bSuck[i][j]` | **這支吸嘴這輪是否有料** ← 真正的關鍵欄位 |

- `GetHPFirstTeamToList()`（`Public\HTEditList.cpp`）**只把 `bSuck[i][j]==true` 的格加進 `lsPoint2D`**
- `GetHPFirstTeamMotUse()` **依 `iSite[i][j] > 0` 決定哪些吸嘴 Z 軸要下降**（`//JerryYang 202050813 : fix錯誤吸嘴下降`）

⇒ `HasHotReadyIC_9045()` 的 soak 檢查迴圈裡**每一格都是有料的格**，空格從來不參與判斷。

**已知誤判（20260804）**：曾把 RT 的 `Site: [-1,4,5,6,-1,-1,11,-1]`（4 顆）解讀成「team 湊不滿、`bHasIC` 累積旗標讓空格拖累 `bReady`」並提出修法 —— **該修法是空操作，已作廢**。反證很簡單：**FT 正常時 team 也固定有 2 格 `-1`**（Kit0 用 A/B/C+E/F/G、跳過 D/H），若空格真會拖累，FT 也早就死鎖了。

⚠ **但本節結論有一個前提，20260826 被打破：`bSuck==true` 不保證「盤上真的有料」。**
上面說的「空格從來不參與判斷」只涵蓋 **`bSuck==false`**（`Site=-1`）的格；若 team 的 `bSuck==true` 卻指到 `Tray.Data==NULL_IC` 的**幽靈格**，那一格**會**參與 soak 判斷，`bHasIC` 累加式就真的會咬人。
兩者是完全不同的東西，**不要用 20260804 的反證去否定幽靈帳** —— 判別方式與案例見 **[§14.6](#146-缺陷幽靈帳asm-結束後帳本指到空格--hashotreadyic_9045-永遠回-false20260826-偉測-hht-10-根因)**。

### 15.6 InArm 旋轉站 case 1160 靜默凍結

`InArmRotateKit=1160` + `InArmAdditionalFunctionTask=41000` + `InArmTask=400` 三者同時凍結，InArm 全線靜止、只有 HOME 能解 →
完整因果鏈、判別方法、修正（V3.33.908.8 watchdog `WAR0157`）見
[deadlock-patterns.md Pattern #17](../ht9045-staterecord-analysis/references/deadlock-patterns.md)。


---

# 熱盤落點幾何鐵律（放料／取料／碎片化）

> 2026-09-14 偉測 HHT-83 推導。適用 8 吸嘴 2x4 + `bPitchOver12000==true` 的機型；其他機型換一下列數欄數，原理相同。

## 1. 放料：**8 格全部 `NULL_IC` 才成立**

`SearchPlacePlateXItem8_8Suck()` → `CheckHotPlateHasSpace_9045_8_New_V(iP, iy, ix, spacY, spacX, Row=2, Col=4, state=0)`

```cpp
else if(state==0)                                   // 放置
{
    if(MOT[MMPlate1+iP].Tray.Data[iCol+SpacC*numC][iRow+SpacR*numR]!=NULL_IC)
        bfail=true;                                 // 任一格不是 NULL_IC 就失敗
}
```
- 區塊形狀＝`(iy, iy+iYHalf) × (ix .. ix+3)`
- `bPitchOver12000==true` → `spacX=1`、`ix` 只在 `{0, XDivision/2}` 跳（左半 / 右半）
- `iYHalf` 常見為 3（HP Y-Pitch 與手臂排距的比值）
- **`HAS_NULL_IC` 占位格一樣算「不是 NULL_IC」→ 一樣擋死整個區塊**

## 2. 取料：**跟 booking 走，不要求整塊**

`PickFromHPList->GetHPFirstTeam()` 回的是 per-slot `bSuck`，team 只剩 7 格照樣取 7 顆。

## 3. 推論：空格集合恆為「整塊」的聯集

放料一次填滿一個完整區塊，取料一次取空當初放料的同一組 8 格
⇒ **空格集合永遠是若干個完整 2x4 區塊的聯集，幾何上不可能自己碎掉。**

碎片只可能來自**單格操作**：

| 來源 | 留下什麼 |
|---|---|
| **ASM 借還被打斷**（Pattern #28） | 真料孤島（`HAS_IC`，`iHotPlateCount=0`） |
| 掉料 `JAM0126` | `HAS_NULL_IC` 占位格（Pattern #24） |
| 搜尋失敗仍放料（缺 S1 的舊 build） | 殘留座標亂放 |
| ASM 借了沒還 | `HAS_NULL_IC` 占位格 |

## 4. ⚠ 容忍度只有 1 顆 —— 熱盤「接近滿」是**設計上的常態**

放料閘門 `CheckHasSpaceToPlace_9045()`：`PlactCT=8`（XDivision 4/8/10/12/16 且雙排吸嘴）、`if(iCT>PlactCT) return true;`

- 空格 ≥ 9 就會去放料 → 放完剩 ≥ 1 → 取一趟回到 ≥ 9
- **穩態空格數在 8~16 之間徘徊 ＝ 只剩 1~2 個落點區塊**

⇒ **一顆孤島吃掉一個區塊；第二顆就把落點吃光 → `WAR0150 In arm search hot plate data error` → 停機。**
⇒ 反過來說，**單獨一個 `NULL_IC` 空洞不會永久損失**：它所在區塊的其他 7 格有 booking，被取走後整塊變空即可重用（以 UPH≈1600 估，約 5~6 分鐘）。**會永久損失的是「占位格」與「真料孤島」，不是空洞。**

## 5. 已知缺陷（未修）：閘門與搜尋的判準不一致

| | `CheckHasSpaceToPlace_9045()` | `SearchPlacePlateXItem8_8Suck()` |
|---|---|---|
| 問的問題 | **散格總數** > 8 ? | 有沒有**連續 2x4 全空**區塊 ? |
| 碎片化時 | `14 > 8` → 「有空間」 | 找不到 → 3 秒 timer → `DoHotPlateHangUp()` → `WAR0150` + 強制 Home |

**兩者判斷相反 → InArm 被放行去放料 → 必然失敗 → 無限重演。**

逃生出口其實**已經寫好**（`ainarm9045_2x4_8.cpp` 高溫分支）：
```cpp
if(LastSet.iTemperature==Tempture_Hot && CheckHasSpaceToPlace_9045()==false)
{
    ...
    Task=500;        // 沒空間 → 改去「從熱盤取料」→ 空出一整塊 → 自己解開
}
```
只要閘門改問「有沒有合法落點」，碎片化就會自癒、不停機。

⚠ **改動風險**：`CheckHasSpaceToPlace_9045()` 有 **177 個呼叫點、跨 26 個會編譯的檔**（1x1 / 1x2 / 1x4 / 2x2 / 2x3 / 2x4 / 2x5 / 2x6 / 2x8 全機型）。
唯一的好消息：ASM 補回走的是該函式**最前面**的早退（`if(bRunAutoSiteMapping && SiteMapData==1) return true;`），在計數之前，所以改後段計數邏輯**不影響 ASM**。

---

## 合併補充：repo 既有參考（20261001）

- [references/colleague-skill-body-20260915.md](references/colleague-skill-body-20260915.md)：repo 版 SKILL.md 正文原樣保存（合併前）
- [references/hotplate-shuttle.md](references/hotplate-shuttle.md)：repo 既有參考檔（同事整理）
