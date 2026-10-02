# repo 版 SKILL.md 正文（合併前原樣保留）

> 📥 20261001 與 RogerYang 版合併時，主 SKILL.md 以 RogerYang 版為底；repo 版中「同一內容不同寫法」的段落以 RogerYang 寫法為準，
> 為避免遺失任何字句，repo 版 SKILL.md 正文整份原樣保存在此。


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
| [SearchPlacePlate_Route_Proposal.md](references/SearchPlacePlate_Route_Proposal.md) | **XDiv=2/3/4/6/8/10/12/16 全路徑圖總表**：各 iInArmType 在不同 XDiv 下的吸嘴落點視覺化（JerryYang 提案，2026-04-07） |
| [HP_PlateForm.xlsx](references/HP_PlateForm.xlsx) | HP 格板規格表（Excel）：各機型 XDiv/YDiv 配置、吸嘴覆蓋範圍 |
| [HP_Item_NULL_IC_RaceCondition.md](references/HP_Item_NULL_IC_RaceCondition.md) | **HP `Item[][]` Race Condition 家族整合檔**：整合原 HP_REF-001/002/003/004，含根本原因、2 處修正案例、Debug Log 提案、同類風險點清單（2026-05-25 整合） |
| [HP_REF-005_Step400_HasIC_Fix.md](references/HP_REF-005_Step400_HasIC_Fix.md) | Step 400 `GetPlaceToHotPlateSuckCol` 對 Col2 回傳重複 j2 → IC 靜默丟失（獨立 Bug，與 Race Condition 家族不同） |
| [AutoClean_OOB_Bug.md](references/AutoClean_OOB_Bug.md) | Auto Clean `bUseTestSocket` 陣列越界案例（V3.33.889→904.0） |
| [SingleSiteOtherSuck-Flag-Semantics.md](references/SingleSiteOtherSuck-Flag-Semantics.md) | `bSingleUseOtherSuck` vs `bSingleInArmUseOtherSuck` Flag 語意 + AutoClean 一致性修正（2026-04-13） |
| [AutoClean_OOB_Bug.md](references/AutoClean_OOB_Bug.md) | **Auto Clean bInArmSuckActive Array OOB Bug**：`bUse8Picker=false` 時 A 排 Z 軸異常下降、記憶體佈局、修正方式、核心函式速查、bUse8Picker 判斷規則、EventLog 診斷（P260428-ATC-H9-01） |
| [SingleSiteOtherSuck-Flag-Semantics.md](references/SingleSiteOtherSuck-Flag-Semantics.md) | **Flag 語意**：`bSingleUseOtherSuck` vs `bSingleInArmUseOtherSuck` 差異 + §12 AutoClean 單站吸嘴一致性修正（2026-04-13） |
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