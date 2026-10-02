# OutArm — 放料段（Place to Tray）詳細參考

> 摘自 §6–§9 放料流程。主 Skill 見 [ht9045-outarm-flow/SKILL.md](../SKILL.md)。

## 6. DoOutArmAdditionalFunction() — 附加功能

> Source: `aoutarm9045.cpp` L~2208 | Task: `iOutArmAdditionalFunctionTask`

### 流程摘要

```
case 1 → case 100 (check flags, Z Safe, IC Fall Down Check)
           ├─ Rotator    → case 10000: DoOutArmRotateKIT()
           ├─ AOI        → case 20000: DoAOIFunction()
           ├─ Fix AI CCD → case 30000: DoFix2AICCDFunction()
           └─ All done   → return true
```

### 觸發條件（CheekNeedToDoOutArmAdditionalFunction）

| 功能 | 觸發條件 |
|---|---|
| **Rotator** | `USE_ROTATE_KIT==1 && tRotate.ActiveRotate && iOutRotateFinish==0/1` |
| **AOI** | `tAOISetup.bEnabledAOI` 或 Scanner AOI / Top Scanner AOI 啟用 |
| **Fix AI CCD** | `USE_Fix_AI_CCD && TestIF_File.bEnableFix2BGAAICCD && fFixAICCD->NeedToGrabImage()` |

> 與 InArm 的 DoInArmAdditionalFunction() 比較：OutArm 有 AOI 和 Fix AI CCD，InArm 有 Die Clean / Precisor / Bottom 2DID / Rotator。

## 7. DoOutArmPlaceToAuto_9045() — 放料到 Tray 控制

> Source: `aoutarm9045.cpp` L~2935 | Task: `iPlaceToAutoTask`

### 流程摘要

搜尋 Tray → IC Fall Down 檢查 → Fix3 Cylinder → 計算放料位置 → Offset → Destroy 延遲 → 放料 → 偵測 Tray

### 關鍵 Case

| case | 動作 |
|---|---|
| 1 | 初始化、Flag Reset → case 10 |
| 10 | IC Fall Down 檢查 → `SearchTrayToPlace_9045()` → `IfUseOnebyOne()` → `SetOutArm_9045()` 計算放料位置 → case 50 |
| 30 | 等待 Tray 有空位（fHasTray && !FullIC）|
| 50 | `CheckOutSuckICFallDown()` 全時掉料檢查 → case 100 |
| 100 | `OutArmNeedCheckOffset()` Offset 校正 → Destroy 延遲 → case 300 |
| 110 | Destroy 暫停延遲計時 → case 300 |
| 200 | Offset 完成 → case 300；Offset 要回 → case 220 |
| 220 | Z Safe → 回 case 1 重新搜尋 |
| 300 | `DoOutArmPlaceToAuto(iWhichAuto)` — 實際 Destroy 吹氣放料 → case 400 |
| 400 | Rotate 處理、Auto Tray Detect → case 500 |
| 500 | `DetectAutoTray()` 偵測 Tray → return true |

## 8. DoOutArmPlaceToAuto() — 實際放料 Destroy

> Source: `aoutarm.cpp` L~2483（不使用 switch/Task，以迴圈遍歷）

### 流程摘要

遍歷所有 `bOutArmSuckActive[i][j]` 為 true 的吸嘴：

1. **NULL IC 處理** — `HAS_NULL_IC` 直接標記為 `NULL_IC`
2. **Destroy 吹氣** — `OutArmSuck.Suck[i][j].Destroy()` 放料
3. **資料記錄**
   - `MOT[Motor].Tray.iBinCode` → BIN 代碼
   - `MOT[Motor].Tray.iWhichSite` → 測試 Site
   - `LotSummary.AddCount()` → 累計 Lot 數量
   - `PordRec.AddUnloadRecord()` / `PordRec.SaveRecord()` → Production Log
4. **Barcode/2DID 記錄**（若啟用）
5. **Bin 計數** — `iByBinTotal[]`、`LastSet.BinCT[]`、`LastSet.iBinData32[]` 累計
6. **ART 計數** — SCK ART / HANA ART 相關計數
7. **Error 處理** — `JAM0217`（Vacuum sensor OFF error）
8. **Tray 滿盤檢查** — BinBox / TraySortCntFunc
9. **Yield 檢查** — `fSortCT->CheckTheYieldAfterPlaceAuto()`

## 9. DoOutArmAfterPlaceToAuto() — 放料後處理

> Source: `aoutarm9045.cpp` L~3139 | Task: `iDoOutArmAfterPlaceToAutoTask`
> 回傳值：int（跳轉目標 case，0 = 尚未完成）

### 流程摘要

```
case 1 (判斷吸嘴是否還有 IC)
  ├─ 還有 IC:
  │   ├─ Fix3 Cylinder → case 500 → return 3010 (繼續放)
  │   └─ return 3010
  └─ 沒有 IC:
      ├─ ATK AMR Fix Tray → return 11100
      ├─ Magazine Buffer 需清 → return 11100
      ├─ Fix3 Full Tray → case 1000: DoFix3FullTray()
      └─ 正常 → case 2000

case 2000 → case 3000 → CheckOutArmCleanOut() → case 3100 → return 100
case 5000 → DoSortingBinTray() (Clean Out 整盤)
```

### 回傳值對照

| 回傳值 | 意義 | 主狀態機跳轉 |
|---|---|---|
| 0 | 尚未完成 | 維持 case 3500 |
| 100 | 正常完成 | 回 case 100（拿下一批）|
| 110 | 完成 + Rotate | 回 case 100 + Rotate offset |
| 3010 | 吸嘴還有 IC | 回 case 3010 繼續放 |
| 5000 | Clean Out + 整盤 | case 5000 |
| 11100 | Magazine Buffer 處理 | case 11100 |
