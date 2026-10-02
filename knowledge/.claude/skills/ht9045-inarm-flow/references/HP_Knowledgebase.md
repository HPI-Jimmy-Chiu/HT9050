# HotPlate 放料路徑知識庫

> 此檔案從 ht9045-inarm-flow\SKILL.md 中分離出來，為獨立 reference。
> 對應 InArm 主狀態機 case 1100 (放料) / case 1500 (取料)。

## HotPlate 放料路徑知識庫


## 適用場景

當使用者詢問以下主題時，載入此技能：
- HotPlate 放料路徑、格位擺放順序模擬
- `SearchPlateToPlace()` / `SearchPlacePlateXItemN_*Suck()` 演算法邏輯
- `CheckHasSpaceToPlace_9045()` 空格計算與 `PlactCT` 門檻
- `DoPlaceToHotPlate_9045()` dispatcher 與狀態機 Task 流程
- 奇數 `YDivision`（如 3x5、4x11）造成 HangUp 或幽靈空位問題
- `iYHalf`、`iPlaceHPOrder`、`iForPlaceHPX3Step` 等放料輔助變數
- `bZFlgToHP[][]`、`bZFlgToHPUsage[][]` Z 軸旗標
- HotPlate HangUp（WAR0150、WAR0151）根因分析
- `SetTraySingleData` 時機問題
- `Use Wide Hotplate` / `i8PickerHPMode` 寬版盤模式
- `iHotCount` 計數與 OutArm Soak 等待聯動
- **InArm 執行層面**：HotPlate 是 InArm 的目標設備，詳見 ht9045-inarm-flow SKILL 中的 **case 1100 / case 1500 (Pick from HotPlate)** 說明

## 原始碼位置

| 檔案 | 主要內容 |
|------|---------|
| `ainarm_SearchPlacePlate.cpp` | HotPlate **放料**搜尋、放料所有核心函式（~5000 行） |
| `ainarm_SearchPlacePlate.h` | 放料側所有函式 extern 宣告 |
| `ainarm_SearchPickPlate.cpp` | HotPlate **取料**核心函式：`DoInArmPickFromHotPlate_9045`、`HasHotReadyIC_9045`、`SearchPlateToPick`、`HotplateDataConversion` 等 |
| `ainarm_SearchPickPlate.h` | 取料側所有函式 extern 宣告 |
| `ainarm9045_*.cpp` | 各 iInArmType 的 `DoPlaceToHotPlate_9045_NxM_N()` 狀態機（由 InArm case 1100 呼叫） |
| `ainarm2.h` | `iPlacePlate[]`、`iPickPlate[]`、`iHotCount`、`iHotWhichShuttle`、`iHotWhichKit` 宣告 |
| `cmydef.h / cmydef.cpp` | `iForPlaceHPX3Step`、`iPlaceHPOrder`、`i8PickerHPMode`、`iHPWideHP`、`HotTime`、`bPickHPDuplicateErr` |

> **呼叫鏈**（參考 ht9045-inarm-flow SKILL）：  
> InArm 主狀態機 → case 1100 → `DoPlaceToHotPlate_9045()` → 本 SKILL Section 4 分派  
> InArm 主狀態機 → case 1500 → `DoInArmPickFromHotPlate_9045()` → 本 SKILL Section 16

---

## 0. InArm → HotPlate 座標配置

### 教示值（Tech）→ 運行值（Prod）流程

InArm 移動到 HotPlate 的目標位置，由以下流程確定：

```
Tech.iInArmPlate1X / Tech.iInArmPlate1Y   ← HP0（左盤）教示座標
Tech.iInArmPlate2X / Tech.iInArmPlate2Y   ← HP1（右盤）教示座標
        │
        ▼ void SetTechDataToProd_InArm()
        │
Prod.XInArm_Plate1_Pick / Prod.YInArm_Plate1_Pick   ← 實際移動目標（HP0）
Prod.XInArm_Plate2_Pick / Prod.YInArm_Plate2_Pick   ← 實際移動目標（HP1）
```

`SetTechDataToProd_InArm()` 將 Tech 教示值轉換並寫入 Prod，供 `MoveInArmXYToHotPlatePlace()` 等函式直接使用。

### 座標限制

| 限制 | 說明 |
|------|------|
| **基準軸 X 不得小於 `BASE_X_TO_HP`** | 防止 InArm 在 X 方向未到達 HotPlate 區域就執行放/取料動作；`BASE_X_TO_HP` 定義於 General.ini（標準值 **6800**，單位 mm×100） |

> **注意**：`BASE_X_TO_HP` 同時用於 `ArmXCanSuck4IC_9045()` 等函式判斷目前 X 位置是否在 HP 允許操作範圍內。

### 變數對照

| 類別 | 變數 | 說明 |
|------|------|------|
| 教示值 | `Tech.iInArmPlate1X` | HP0 InArm 到位 X 教示值 |
| 教示值 | `Tech.iInArmPlate1Y` | HP0 InArm 到位 Y 教示值 |
| 教示值 | `Tech.iInArmPlate2X` | HP1 InArm 到位 X 教示值 |
| 教示值 | `Tech.iInArmPlate2Y` | HP1 InArm 到位 Y 教示值 |
| 運行值 | `Prod.XInArm_Plate1_Pick` | HP0 實際移動目標 X（由 `SetTechDataToProd_InArm()` 寫入） |
| 運行值 | `Prod.YInArm_Plate1_Pick` | HP0 實際移動目標 Y |
| 運行值 | `Prod.XInArm_Plate2_Pick` | HP1 實際移動目標 X |
| 運行值 | `Prod.YInArm_Plate2_Pick` | HP1 實際移動目標 Y |
| 限制值 | `BASE_X_TO_HP` | General.ini，基準軸 X 最低門檻（mm×100），InArm X 不得小於此值 |

---

## 1. HotPlate.Data 檔案結構

路徑：`D:\HT9045\IniData\Data\[工作檔名]\HotPlate.Data`

```ini
[Hotplate Form]
   Name=Polaris
   X Start=45.000         ; 第一格 X 座標（mm）
   Y Start=50.000         ; 第一格 Y 座標（mm）
   X Pitch=65.000         ; 欄間距（mm），乘以 100 存到 HotPlateForm.XPitch
   Y Pitch=70.000         ; 列間距（mm），同上
   X Division=3           ; 欄數（Col）
   Y Division=5           ; 列數（Row）
   Using Flag=3           ; 使用哪個盤：1=只用HP0, 2=只用HP1, 3=兩盤都用
Use Wide Hotplate=1       ; 1=Wide HP 模式（i8PickerHPMode=iHPWideHP）
```

### HotPlate 座標映射

```
iPlacePlate[0] = HP 編號（0=左盤, 1=右盤）
iPlacePlateX[0] = Col 索引（0 ~ XDivision-1）
iPlacePlateY[0] = Row 索引（0 ~ YDivision-1）

實際 X 座標 = X Start + iPlacePlateX * X Pitch
實際 Y 座標 = Y Start + iPlacePlateY * Y Pitch
```

---

## 2. Tray 資料狀態定義

`MOT[MMPlate1+iPlate].Tray.Data[Col][Row]` 存放各格位狀態：

| 值 | 常數 | 意義 |
|----|------|------|
| 0 | `NULL_IC` | 空格，可供放料 |
| 1 | `HAS_IC` | 有 IC，不可放料 |
| 2 | `HAS_HOT_IC` | 有 IC 且已加熱（等待 Soak 完成） |
| 3 | `HAS_NULL_IC` | 標記為不可用（Close Site 造成 NULL IC / 奇數格保護） |

> **重要**：`NULL_IC` 與 `HAS_NULL_IC` 意義完全不同。搜尋空格時只取 `NULL_IC`，`HAS_NULL_IC` 代表「此格確定不使用」，不算有效空格。

---

## 3. 關鍵全域變數

| 變數 | 宣告檔 | 說明 |
|------|--------|------|
| `iPlacePlate[2]` | ainarm2.h | 目前放料目標 HP 編號（0/1），[0]=主，[1]=備用 |
| `iPlacePlateX[2]` | ainarm2.h | 目前放料目標 Col 索引 |
| `iPlacePlateY[2]` | ainarm2.h | 目前放料目標 Row 索引 |
| `iPickPlate[2]` | ainarm2.h | 目前取料來源 HP 編號 |
| `iPickPlateX[2]` | ainarm2.h | 目前取料來源 Col 索引 |
| `iPickPlateY[2]` | ainarm2.h | 目前取料來源 Row 索引 |
| `iHotCount` | ainarm2.h | 放料計數器，用於 OutArm 區分「哪一趟放的 IC」，Soak 等待依此計數 |（詳見本 SKILL § 22；由 ht9045-inarm-flow case 1100 放料時遞增） |
| `iYHalf` | cmydef.h | 每次放料 Y 方向步距（Row 數）；由 `GetHotPlateYHalfPos()` 計算 |
| `iPlaceHPOrder` | cmydef.h | 0=先放 Row0（第一排吸嘴行），1=只放 Row1（第二排吸嘴行） |
| `iForPlaceHPX3Step` | cmydef.h | 對 XDiv=3 時的放料步驟（0=Col0+1, 1=Col2 單顆） |
| `iForPlaceHPX6Step` | cmydef.h | 對 XDiv=6 時的放料步驟（0=左半, 1=右半） |
| `bZFlgToHP[row][col]` | ainarm_SearchPlacePlate.h | 對應 Sucker 是否有 IC 且需 Z 軸下壓放料 |
| `bZFlgToHPUsage[row][col]` | ainarm_SearchPlacePlate.h | 對應 Sucker 是否需要移動至 HP（含 HAS_NULL_IC 情況） |
| `bPickFormHotplatePartOK` | ainarm_SearchPickPlate.h | 一趟取料動作已完成旗標 |
| `bPickFormHotplateRetry` | ainarm_SearchPickPlate.h | 取料發生 Retry 旗標（防 HangUp） |
| `bInArmTryPickFromHotPlateFinish` | ainarm_SearchPickPlate.h | TryPick（驗證性吸取）完成旗標 |
| `bPickHPDuplicateErr[row][col]` | cmydef.h | 各吸嘴重複取料錯誤旗標（連續 Retry 判斷用） |
| `iHotWhichShuttle[p][c][r]` | ainarm2.h | 記錄各 HP 格位 IC 放料時對應的 Shuttle 編號（OutArm 取料識別用） |
| `iHotWhichKit[p][c][r]` | ainarm2.h | 記錄各 HP 格位 IC 放料時對應的 Kit 編號 |
| `iHotPlateCount[p][c][r]` | ainarm2.h | 記錄各 HP 格位 IC 放料時的 `iHotCount` 值（OutArm 識別批次用） |
| `HotTime[p][c][r]` | cmydef.h | 各格位的加熱計時器（與 `Prod.iHotTime` 比較判斷 Soak 完成） |
| `bPickFromHotplate` | — | InArm 正在從 HP 取料的旗標（一般 = 常溫 false，高溫 true） |
| `b6x20HP` | ainarm_SearchPlacePlate.cpp | 6x20 HP 特殊模式旗標 |
| `b4x11HP` | ainarm_SearchPlacePlate.cpp | 4x11 HP 特殊模式旗標（奇數 YDiv） |
| `b4x10HP_2x2` | ainarm_SearchPlacePlate.cpp | 4x10 HP 奇數模式旗標 |
| `b12x16HP_2x6` | ainarm_SearchPlacePlate.cpp | 12x16 HP 2x6 模式旗標 |
| `iHPHangUpCount` | ainarm_SearchPlacePlate.h | HangUp 發生次數計數器 |
| `HPPlaceLog` | ainarm_SearchPlacePlate.h | 放料位置重複偵測物件（→ JAM0159） |

---

## 3.5 iInArmType vs 吸嘴選擇函式 — 路由陷阱

### 核心規則

`bUseAxExPicker()`、`bUseAxxGPicker()`、`bUseACEGPicker()` 依 **iInArmType** 決定使用哪組吸嘴。
**修改 HP 搜尋/放料邏輯時，必須先確認機台實際的 iInArmType**，而非假設。

### 吸嘴選擇函式路由表

| 函式 | 吸嘴 | jStep | 涵蓋 iInArmType |
|------|------|-------|-----------------|
| `bUseAxExPicker()` | A,E (j=0,2) | 2 | 1x2_13, 1x4_13, 2x1_13, **2x2_4_13**, 2x4_13, e2x8Run2x2_13, AutoClean:2x2_8_Hot |
| `bUseAxxGPicker()` | A,G (j=0,3) | 3 | **2x2_4_13+XDiv==12**, 1x2_14, 1x3_14, 1x4_14, 2x2_2_14, 2x2_4_14, 2x3_14, 2x4_14, e2x8Run2x2_14, AutoClean:2x2_8_Hot(wide) |
| `bUseACEGPicker()` | A,C,E,G (j=0,1,2,3) | 1 | 1x4_4, 1x4_4_Back, 1x4_8_Hot, 2x4_8, 2x8_8, 2x8_32, **2x2_8_Hot** |

### 教訓（2026-05-12 偉測 WAR0150 案例）

**問題描述**：e9045_2x2_4_13 + XDiv=12 HP 發生 WAR0150（搜尋所有格位失敗）。

**根因鏈**：
1. `bUseAxExPicker()` 第一個 if 直接包含 `e9045_2x2_4_13` → 回傳 true（AE 模式）
2. `GetVariableInHotPlateData_AxEx()` XDiv=12: XPitch×9 = 15300 > 12000 → **bPitchOver12000=true**
3. bPitchOver12000=true 時搜尋只掃 4 個位置 (0,1,4,5)，但放料用 j×6 → 只填 cols 0,1,6,7
4. 其餘 8 個 col 永遠無法被搜到 → **HP 滿 1/3 即報 WAR0150**

**修正方式**：在 SearchPlacePlateXItem_2x2Suck 的 bPitchOver12000 分支，為 XDiv=12 + bUseAxExPicker 設定 spacX=XDiv/2=6 並使用標準 ix++ 迭代。
搜尋 (ix, ix+6) 正確對齊放料 GetPlaceToHotPlateCol(j) = base+j*6。

**❌ 失敗方案（AG 模式切換）**：
曾嘗試把 e9045_2x2_4_13+XDiv=12 從 bUseAxExPicker 切到 bUseAxxGPicker（AG 模式），
但 AG 改變 jStep=3，導致 Loader 取料使用不同吸嘴（Suck[0,3]而非[0,2]），
Shuttle 映射表 XPHSuckToSht_2x2_13 也不相容，Motor 座標完全錯誤。
**結論：切換吸嘴模式影響全鏈路，不可輕易變更。**

**檢查規則**：
- 修改 HP 搜尋/放料邏輯前，**必須確認目標機台的 iInArmType**（而非假設是 e9045_2x2_8_Hot）
- `iInArmType` 在 `DoInArm_9045_Type()` 中根據 TestMode、SitePitch 等條件動態決定
- 同一台機器的 iInArmType 與 XDiv/YDiv 組合可能不直觀（例：2x4 mode 的 InArm 可能用 e9045_2x2_4_13）
- **每個 iInArmType 有獨立的 DoPlaceToHotPlate 函式**（見 §4 對照表），`ainarm9045_2x2_8_Hot.cpp` 只服務 e9045_2x2_8_Hot

## 4. DoPlaceToHotPlate_9045() — 放料 Dispatcher

> Source: `ainarm_SearchPlacePlate.cpp` line 4810  
> **呼叫位置**：InArm 主狀態機 case 1100（ht9045-inarm-flow SKILL §4）  
> **Task 變數**：`iInArmPlaceToHotPlateTask`

### 功能
根據 `iInArmType` 分派到對應的放料狀態機，**本身不執行任何動作**。

### iInArmType → 放料函式對照表

| iInArmType | 呼叫函式 |
|-----------|---------|
| `ep1Picker` / `e9045_1x1_1` / `e9045_1x4_1_Ac` | `DoPlaceToHotPlate_9045_1x1_1()` |
| `e9045_1x2_2_13` | `DoPlaceToHotPlate_9045_1x2_2()` |
| `e9045_1x2_2_14` | `DoPlaceToHotPlate_9045_1x2_2_14()` |
| `e9045_1x2_4_Hot` | `DoPlaceToHotPlate_9045_1x2_4_Hot()` |
| `e9045_1x3_2_14` | `DoPlaceToHotPlate_9045_1x3_2_14()` |
| `e9045_1x3_4` | `DoPlaceToHotPlate_9045_1x3_4()` |
| `e9045_1x4_4_13` | `DoPlaceToHotPlate_9045_1x4_4_13()` |
| `e9045_1x4_2_14` | `DoPlaceToHotPlate_9045_1x4_2_14()` |
| `e9045_1x4_4_Back` | `DoPlaceToHotPlate_9045_1x4_4_Back()` |
| `e9045_1x4_4` | `DoPlaceToHotPlate_9045_1x4_4()` |
| `e9045_1x4_8_Hot` | `DoPlaceToHotPlate_9045_1x4_8_Hot()` |
| `e9045_2x1_2_13` | `DoPlaceToHotPlate_9045_2x1_2()` |
| `e9045_2x2_4_12` | `DoPlaceToHotPlate_9045_2x2_4_12()` |
| `e9045_2x2_4_13` | `DoPlaceToHotPlate_9045_2x2_4_13()` |
| `e9045_2x2_4_14` | `DoPlaceToHotPlate_9045_2x2_4_14()` |
| `e9045_2x2_8_Hot` | `DoPlaceToHotPlate_9045_2x2_8_Hot()` |
| `e9045_2x3_6_14` | `DoPlaceToHotPlate_9045_2x3_6_14()` |
| `e9045_2x3_6` | `DoPlaceToHotPlate_9045_2x3_6()` |
| `e9045_2x4_4_13` | `DoPlaceToHotPlate_9045_2x4_4_13()` |
| `e9045_2x4_4_14` | `DoPlaceToHotPlate_9045_2x4_4_14()` |
| `e9045_2x4_8` | `DoPlaceToHotPlate_9045_2x4_8()` |
| `e9045_2x5_8` | `DoPlaceToHotPlate_9045_2x5_8()` |
| `e9045_2x6_8` | `DoPlaceToHotPlate_9045_2x6_8()` |
| `e9045_2x8_8` / `e9045_2x8_32` | `DoPlaceToHotPlate_9045_2x8_8()` |

---

## 5. 放料狀態機通用流程

**InArm 上下文**（ht9045-inarm-flow SKILL）：  
InArm case 1100 呼叫 `DoPlaceToHotPlate_9045()` ➔ 分派至各 iInArmType 的 `DoPlaceToHotPlate_9045_NxM_N()` 狀態機

所有 `DoPlaceToHotPlate_9045_NxM_N()` 共用相同的 Task 結構，Task 變數為 `iInArmPlaceToHotPlateTask`。

```
[Task=1]  搜尋格位
  GetVariableXInHotPlateData(iPlaceHP)
  SearchPlateToPlace()            ← 決定 iPlacePlate, iPlacePlateX, iPlacePlateY
  bPlaceToHotplatePartOK = false
  直接 fall through 到 Task=100（不 break）

[Task=100]  移動 InArm XY → 目標格位
  MoveInArmXYToHotPlatePlace(iPlaceHP, true)
  ├─ 移動中偵測到掉料 → Task=110
  │     CheckInArmSuckICFallDownToHasNullIC() → 回 Task=100
  └─ XY 到位 → Task=200

[Task=200]  HP 偏移補正 / 記錄放料位置
  ├─ InArmNeedCheckHotPlateOffset() == true → Task=210（Z 補正）
  └─ 不需補正：
       PickFromHPList->UpdateHPSuckGroup()    ← 記錄位置供 OutArm 查詢
       iEnableReleaseDelay==0 → 計時器 → Task=340
       else                               → Task=350

[Task=210/220]  HP 偏移補正
  bEnterOffset=true → MoveInArmZToPlateSafe() → 回 Task=100

[Task=340]  Release 前延遲
  InArmReleaseDelayToHot.Off() → Task=350

[Task=350]  Z 軸下壓 + 吹氣放料
  for(row 0~iPickRow, col 0~iStepHP)
    j2 = GetPlaceToHotPlateSuckCol(j)
    ix = GetPlaceToHotPlateCol(j)
    if InArmSuck.Suck[i][j2].Destroy()     ← 吹氣
      DoPlaceToHPSwapData(i, j2, ip, iy, ix)  ← 更新 Tray 資料
  bPlaceToHotplatePartOK = true
  DoCheckAutoSiteMappingPosition()
  → Task=400

[Task=400]  Z 軸上升回安全位置
  MoveInArmZToPlateSafe()
  ├─ InArmSuck.HasIC() == true（手臂還有 IC 未放完）
  │    → Task=1（iHotCount++，繼續找下一個空格）
  └─ 手臂 IC 已全部放完 → Task=500

[Task=500]  黏貨判斷
  CheckInArmDestroyICFail()
  ├─ 黏貨（Error） → return false（等待 Retry）
  └─ 通過 → Task=501

[Task=501]  放料完成收尾
  AdjustShuttleWhichKitOrder()
  iHotCount++
  若 HP 仍有空格（CheckHasSpaceToPlace_9045）→ SearchPlateToPlace() 預先搜尋
  PickFromHPList->AddHPSuckGroup()        ← 建立新 Group 供 OutArm 查詢 Soak Time
  使用雷射測距 → Task=600
  else → return true ✓

[Task=600]  雷射浮料檢查（選配）
  CheckInArmFloating() 完成 → return true ✓
```

**放滿一個 HP 的循環關鍵**：Task=400 發現 `InArmSuck.HasIC()==true` 時跳回 Task=1，反覆放料直到手臂清空為止。

---

## 6. SearchPlateToPlace() — 空格搜尋入口

> Source: `ainarm_SearchPlacePlate.cpp` line 4443  
> **InArm 呼叫鏈**\: ht9045-inarm-flow SKILL case 1100 → `DoPlaceToHotPlate_9045()` → Section 4 Dispatcher → Task 30 → `SearchPlateToPlace()`

依 `HotPlateForm.XDivision` 分派到對應的搜尋函式：

| XDivision | 使用函式 | 備註 |
|-----------|---------|------|
| **1** | `SearchPlacePlateXItem_1Suck()` ⚠️ | **無獨立 XDiv=1 分支**；只有 ep1Picker/SingleSite/DualSite關站 才能進入，其他多吸嘴模式呼叫 → `MES0156` 錯誤 |
| 2 | `SearchPlacePlateXItem_1x2Suck()` | |
| 3 | `SearchPlacePlateXItem3_1x2Suck()` | 1x2 mode |
| 3 | `SearchPlacePlateXItem3_2x2Suck()` | 2x2 mode |
| 3 | `SearchPlacePlateXItem3_2Suck_3Site()` | 1x3 mode |
| 4 | `SearchPlacePlateXItem4_8Suck()` | |
| 6 | `SearchPlacePlateXItem6_8Suck()` / `SearchPlacePlateXItem6_2x2_8Suck()` / `SearchPlacePlateXItem6_3Suck_ACEx()` | 依 i8PickerHPMode 選擇 |
| 8 | `SearchPlacePlateXItem8_8Suck()` | |
| 10 | `SearchPlacePlateXItem10_8Suck()` | |
| 12 | `SearchPlacePlateXItem12_8Suck()` / `SearchPlacePlateXItem12_2x6()` | 依 b12x16HP_2x6 |
| 16 | `SearchPlacePlateXItem16_8Suck()` | |

### 搜尋方向

所有搜尋函式掃描順序：**先遞增 Col（X 方向）→ 後遞增 Row（Y 方向）**。
HP 填滿後切換到另一個 HP（依 `HotPlateForm.iPlateSelect` 旗標決定優先順序）。

---

## 7. SearchPlacePlateXItem3_1x2Suck() — 3-Col HP 搜尋（1x2 Mode）

> Source: `ainarm_SearchPlacePlate.cpp` line 3023  
> 適用：`XDivision=3`（如 Polaris 熱盤 3×5）

### 放料格位分組

**Col 0+1 同步放**（2 顆）：`iForPlaceHPX3Step=0`
**Col 2 單顆放**（1 顆）：`iForPlaceHPX3Step=1`，吸嘴選擇依 Row 奇偶：
- 偶數 Row → `GetPlaceToHotPlateSuckCol(0)` = `j2=0`（Sucker Aa）
- 奇數 Row → `GetPlaceToHotPlateSuckCol(0)` = `j2=3`（Sucker Ad）

### 奇數 YDivision 保護（3x5 HP）

`YDivision=5`（奇數），最後一格 `[Col2][Row4]` 必須跳過以確保格位對稱。

**目前實作**（20250923 修改後）：使用 `continue` 跳過：
```cpp
if(HotPlateForm.YDivision%2==1 &&
   i==HotPlateForm.XDivision-1 &&
   j==HotPlateForm.YDivision-1)
{
    continue;
}
```

> ⚠️ **已知問題**：`continue` 跳過但不標記 `[2][4]=HAS_NULL_IC`，導致 `CheckHasSpaceToPlace_9045()` 仍將其計為有效空格（幽靈 +1），InArm 比正確時機提早 1 個 OutArm 取料趟次開始放料，造成**第二輪放料路徑與第一輪不同**。  
> 參見本技能第 10 節「已知問題與修改提案」。

---

## 8. CheckHasSpaceToPlace_9045() — 空格門檻計算

> Source: `ainarm_SearchPlacePlate.cpp` line 601

**功能**：計算當前 HP 空格總數 `iCT`，若 `iCT > PlactCT` 則 InArm 可繼續放料。

### PlactCT 門檻對照

| 條件 | PlactCT | 說明 |
|------|---------|------|
| `ep1Picker` | 0 | 放滿才可繼續 |
| `e9045_1x1_1` | 1 | 留 1 格緩衝（ASE_SG 例外 = 0） |
| `e9045_1x2_2_13` / `_14`（格數為偶數） | 0 | |
| `e9045_1x2_2_13` / `_14`（格數為奇數） | 4 | |
| `bUseAxExPicker`：`2x2_4_13` + `6x11` + 雙盤 | 12 | |
| `bUseAxExPicker`：其他 | `2 * iPickRow` | |
| `bUseAxxGPicker`：`XDiv=6` | 6 | |
| `bUseAxxGPicker`：`XDiv=3` + `1x3` 模式 | 3 | |
| `8/10/12/16 XDiv` | 8（或 6） | |
| `6 XDiv` + Wide HP + 1x3/2x3/2x6 模式 | `3 * iPickRow` | |
| `6 XDiv` + 雙盤 + Close Site | 8 | |
| `6 XDiv` + 雙盤 | 12 | |

> 判定是 **`if(iCT > PlactCT)`（嚴格大於）**，且 `iCT` 是**整片盤的 `NULL_IC` 總數**，**不看幾何、不看連續性**。
> 設計意圖＝「**留一次取放量**」（AxEx 分支 `PlactCT=2*iPickRow` 的 `//Sam 20250904 : 修正加熱盤沒有留空格問題。` 即此意）。

### 8.1 ⚠ 單排吸嘴（`iPickRow==1`）落單：`PlactCT=8` 會多留一趟（SCK 20260727 案）

**現象**：加熱盤永遠有 **2 排空格**填不滿（客戶說「In arm do not fill hot plate fully」），無 JAM／WAR，UPH 正常。

**機制**：`XDiv=4/8/10/12/16` 走 else 分支 → **`PlactCT=8` 是字面常數**，當年為「2x4 八吸嘴一趟 8 顆」而訂。
`e9045_1x4_4`（`SetPickerCount(1,4,1,4,…)`）一趟只有 **4 顆**，同一個 8 就變成**兩趟量**：

| 空格 E | `iCT>8`？ | 動作 | 結果 |
|---|---|---|---|
| 12 | ✓ | 取 Loader → 放 4 顆 | E=8 |
| **8** | ✗ | → `Task=500` 等 hot-ready → 取 4 顆去 shuttle | E=12 |

⇒ **放完料的峰值恆為「盤容量−8」**（SCK 實測 4×8 盤 = 24/32）。若 `PlactCT=4` 則峰值為 28（留 1 排）。

**同型比對**（同一份碼自相矛盾的直接證據）：同樣 `_8Site2X4N`、同樣一趟 4 顆 ——
4-picker 機（`USE_PICKER_COUNT==0`）走 `e9045_1x4_4_13` → AxEx 分支 `PlactCT=2*iPickRow`=**4（留 1 趟）**；
8-picker 機走 `e9045_1x4_4` → else 分支 **8（留 2 趟）**（`ainarm9045.cpp` L1774-1797）。

| iInArmType | (iPickRow,iPickCol) | 一趟顆數 | PlactCT | 留幾趟 |
|---|---|---|---|---|
| `e9045_1x4_4` / `_Back` | (1,4) | 4 | 8 | **2.0** ⚠ |
| `e9045_2x4_8` / `2x2_8_Hot` / `1x4_8_Hot` | (2,4) | 8 | 8 | 1.0 ✔ |
| `e9045_1x4_4_13` / `2x4_4_13` | (2,2) | 4 | `2*iPickRow`=4 | 1.0 ✔ |
| `e9045_1x4_2_14` | (1,2) | 2 | 4 | 2.0 ⚠（未修） |
| XDiv=2/3 落到 else | — | — | **0** | 0（完全不留）⚠ |

**修正（V3.33.908.8，20260801，不限客戶碼）** — `ainarm_SearchPlacePlate.cpp:697` 起：

```cpp
            if(b12x16HP_2x6==true)                                              //Steven 20241203 : for 12x16 HP
                PlactCT=6;
            else if(InArmSuck.iPickRow==1 &&
                    HotPlateForm.XDivision==InArmSuck.iPickCol)                 //AI(ht9045-inarm-flow) 20260801 (RogerYang) : 限單排吸嘴且一趟剛好放滿一整排(不會分次放)才縮門檻
                PlactCT=InArmSuck.iPickRow*InArmSuck.iPickCol;                  //AI(ht9045-inarm-flow) 20260801 (RogerYang) : 門檻=一次取放量, 原寫死8是2x4八吸嘴的量
            else
                PlactCT=8;                                                      //AI(ht9045-inarm-flow) 20260801 (RogerYang) : 雙排或會分次放料的HP寬度一律維持原值
```

- `iPickRow==2` 的機型：`2*4=8`，與原值相同 ⇒ **零行為變更**（且連算式都不會執行到）。
- 目前實際受影響：**只有 `e9045_1x4_4` / `_Back` 且 HP `XDivision==4`**（SCK HT-9046 = 4×8 盤）。
- 修完峰值 24 → 28，**仍會留 1 排** —— 那是設計上的取放緩衝，不是沒修好（對客戶要講清楚）。

### 8.2 為什麼要卡 `XDivision==iPickCol`——「單排 ≠ 一次放完」

`case 400`（各模式檔）有回頭迴圈：`if(InArmSuck.HasIC()){ … Task=1; iHotCount++; }` ⇒ **手上沒放完就回 case 1 重新 `SearchPlateToPlace()` 再放一次**。
每趟放幾欄由 `GetHotPlateColStep(iPlaceHP)` 決定（§9）：

| HP XDiv（iPickCol=4 單排） | iStepHP | 會不會分次 |
|---|---|---|
| **4** | 4 | **不會**（一趟＝一整排）✔ 可套用新門檻 |
| 10 | `iPlateC==8 ? 2 : 4` | **會**（第 8 欄只放 2 顆）✘ |
| 6 | 2 / 3 / 4 | 會 ✘（且不在此分支） |
| 8 / 12 / 16 | 4 | 一排要放兩趟、落點交錯 ✘ |

`iCT` 只數總數不看幾何 ⇒ **分次放的配置若把門檻縮到 4，最後一段可能找不到合法落點 → `SearchPlacePlateXItem*` 空轉 → `DoHotPlateHangUp()` → WAR0150**。
`XDivision==iPickCol` 這個條件的意義就是「空格永遠以整排為單位」，此時 count 與幾何等價，零風險。

### 8.3 日後要放寬到 XDiv=8/12/16 的驗證 Checklist

> 前提：客戶實際回報 + 取得 **hang 當下**的 State Record（勿先改）。逐項通過才可放寬。

1. **確認 iInArmType 與 (iPickRow, iPickCol)** — `DecisionVariables.csv` 的 `iInArmType` + 對應 `SetInOutArmParameter_*()` 的 `SetPickerCount()`；只有 `iPickRow==1` 才在討論範圍。
2. **算出該 XDiv 的 `GetHotPlateColStep(iPlaceHP)`** — 是否**恆等於 iPickCol**（不因 `iPlateC` 變動）。XDiv=10 在 `iPlateC==8` 會變 2 ⇒ 直接淘汰。
3. **展開 `GetPlaceToHotPlateCol(j)` 的落點** — `ix = iPlacePlateX[0] + j*(XDivision/2)`：列出一趟會佔用哪幾欄，確認**放料落點集合 == 取料落點集合**（`GetHotPlateColStep(iPickHP)` 那一半），否則空格會交錯碎片化。
4. **模擬空格演進** — 由滿盤開始，交替「放一趟／取一趟」跑 ≥3 圈，確認空格永遠是**同一組欄位**、不會出現「總數夠但湊不出一趟」的狀態。
5. **尾盤／半盤情境** — Loader 收尾時 `SetInArmUseSuckToHasNullIC()` 會把 `HAS_NULL_IC` 佔位放進 HP；確認佔位格被取走後會回到 `NULL_IC`（否則 §14.5 的「白色格永不再填」會與新門檻疊加）。
6. **雙盤 `iPlateSelect==3`** — `CheckHasSpaceToPlace_9045()` 的 `iCT` 是兩片合計，但一趟只能放在同一片；確認不會出現「兩片各剩半趟」而閘門放行。
7. **Hang 防護回歸** — 故意把盤填到只剩一趟，跑 30 分鐘確認不觸發 `WAR0150`／`DoHotPlateHangUp()`。
8. **回歸 `iPickRow==2` 機型** — 任挑一台 `2x4_8` 或 `2x2_8_Hot`，確認峰值水位與改前一致（門檻仍是 8）。

---

## 9. GetHotPlateColStep() / GetPlaceToHotPlateSuckCol() / GetPlaceToHotPlateCol()

### GetHotPlateColStep(iAction)
> Source: `ainarm_SearchPlacePlate.cpp` line 745

回傳此次放/取料需執行幾個 Col 步驟（`iStepHP`）。  
簡記：1x1→1，2吸嘴→2（但 XDiv=3 且 Col=2 時→1），4/8吸嘴→4。

### GetPlaceToHotPlateSuckCol(j)
> Source: `ainarm_SearchPlacePlate.cpp` line 859

輸入放料步驟索引 `j`，回傳對應的**吸嘴欄 `j2`**。

| Picker 種類 | 公式 | 說明 |
|------------|------|------|
| `1x1` | `j2=0`（或 1，若 SingleUseOtherSuck） | |
| `bUseAxExPicker`（1&3 吸嘴）| `j2=j*2` | Aa, Ac |
| `bUseAxxGPicker`（1&4 吸嘴）| `j2=j*3`，XDiv=3 且 Col=2：偶 Row→0, 奇 Row→3 | |
| 其他（4 吸嘴）| 依 XDiv 與 iForPlaceHPX*Step | |

### GetPlaceToHotPlateCol(j)
> Source: `ainarm_SearchPlacePlate.cpp` line 927

輸入放料步驟索引 `j`，回傳對應的**熱盤 Col 索引 `ix`**。  
公式：`ix = iPlacePlateX[0] + j * (XDivision / 2)`（標準 4/8 吸嘴）。

---

## 10. GetHotPlateYHalfPos() — iYHalf 計算

> Source: `ainarm_SearchPlacePlate.cpp` line 89

每次放料前呼叫，計算一次 `iYHalf`（Y 方向放料步距）並設定特殊 HP 旗標：

| 條件 | iYHalf | 特殊旗標 |
|------|--------|---------|
| `iPickRow==1`（單排吸嘴） | 1 | — |
| `2 Row + XDiv=4 + 奇數 YDiv` | 1 | `b4x11HP=true` |
| `2 Row + iPickCol=2 + ARM_HP_Y_PITCH 不整除 YPitch` | 1 | `b4x10HP_2x2=true` |
| `2 Row + YDiv=20 + YPitch=12.7mm` | 5 | `b6x20HP=true` |
| `2 Row + XDiv=12 + YDiv=16` + 2x6 模式 | 2 或 3 | `b12x16HP_2x6=true` |
| 一般：| `ARM_HP_Y_PITCH / YPitch` | — |

---

## 11. DoPlaceToHPSwapData() — 放料資料交換

> Source: `ainarm_SearchPlacePlate.cpp` line ~4620

放料吹氣成功後，負責：
1. 記錄 `iHotWhichShuttle[][]`、`iHotWhichKit[][]`、`iHotPlateCount[][]`（OutArm 識別用）
2. 更新 `PickFromHPList` 供 OutArm 查詢
3. 設定 `HotTime` 計時起點（一般 = 0，QA mode offline 例外 = iHotTime）
4. 設定 `bInArmCheckDestroyACT[][]`（確認吹氣動作）
5. 記錄 `iRowOnHotPlate[][]`（`(row+1)*10 + col+1`，例：Aa=11, Ab=12）
6. 更新 `InArmSuck.PordRec` 歷史
7. 呼叫 `InArmSuck.CopyToTray()`→`MOT[MMPlate1+iP].Tray.Data[col][row] = HAS_IC`

---

## 12. Row2CanPutHP() — 是否可放第二排吸嘴

> Source: `ainarm_SearchPlacePlate.cpp` line 310

以下任一條件成立時，**無法**使用第二排（回傳 false）：
- `bRunAutoClean && iAutoClean_Tray == eCKPos_HP2`（AutoClean 佔用 HP2）
- `iInArmType == e9045_2x3_6_14`
- `HotPlateYPitchCanPutAll() == false`（Y-Pitch 過大，無法雙排放）
- `iYStep + iNowRow >= HotPlateForm.YDivision`（超出盤邊界）
- `iCloseSiteState > 0`（有 Close Site 狀態）

---

## 13. 已知問題與修改提案

### 問題：3xN HP（奇數 YDivision）幽靈空位導致放料路徑不一致

**條件**：`XDivision=3`，`YDivision=5`（或任何奇數），`iInArmType=e9045_1x2_2_14`

**症狀**：
- 第一輪放料正確跳過 `[Col2][Row4]`
- 但 `[Col2][Row4]` 的 `Tray.Data` 始終為 `NULL_IC`
- `CheckHasSpaceToPlace_9045()` 計算 `iCT` 時將其算入，門檻 `PlactCT=4` 提早滿足
- InArm 比預期提早 1 趟開始放料，第二輪路徑與第一輪不同

**根本原因**：  
`SearchPlacePlateXItem3_1x2Suck()` 使用 `continue` 跳過格位，但對比 `SearchPlacePlateXItem3_2x2Suck()`（2x2 版本），後者在 while 頂部呼叫 `SetTraySingleData(XDiv-1, YDiv-1, HAS_NULL_IC)` 正確標記不可用格位。  
1x2 版本的同段程式碼於 2025-09-23 因解決「3x5 HP hang up」被 Mark out，改為 `continue`。

**修改提案**（非最終定論，需確認後再實作）：  
將 `SetTraySingleData` 呼叫移至 iPlate 切換邏輯**之前**（while 迴圈底部），確認目前 HP 無空格後才執行標記：

```cpp
// 在 SearchPlacePlateXItem3_1x2Suck() 的 for-loop 結束後、iPlate 切換前加入：
if(HotPlateForm.YDivision%2==1)
    MOT[MMPlate1+iPlate].SetTraySingleData(
        HotPlateForm.XDivision-1,
        HotPlateForm.YDivision-1,
        HAS_NULL_IC);

if((HotPlateForm.iPlateSelect&0x02) && (HotPlateForm.iPlateSelect&0x01))
    iPlate++;
...
```

**為何不會重現原 HangUp**：原 HangUp 發生於「Sucker1 仍有 IC，搜尋 Col2 最後空格」時立即被標記。移至切換前執行，此時 HP 確定無空格，Sucker 也已清空，不會再搜尋該格位。

---

## 14. HotPlate HangUp 防護機制

| 警報碼 | 觸發條件 | 相關函式 |
|--------|---------|---------|
| `WAR0150` | `iHPHangUpCount != 0`（可能 HP 已滿但仍持續嘗試放料）| `ainarm2.cpp` 入口 Guard |
| `WAR0151` | `SearchPlateToPlace` 中 X/Y < 0 的非法位置 | `SearchPlacePlateXItem16_8Suck` |
| `JAM0109` | HP 取料真空失敗，超過 Retry 次數 | `DoInArmPickFromHotPlate_9045` Task=330 |
| `JAM0159` | `HPPlaceLog.bIsSamePos == true`（連續兩次放到同一格位） | `TMyHotPlatePlaceLog::SetPosition()` |

`iHangUpCount=10000`：while 迴圈保護計數器，超過 10000 次且計時器也過時才觸發 HangUp 報警。

---

## 15. 工作檔範例模擬（Polaris 3x5 HP + 1x2_14）

**工作檔**：`55.0X55.0_2768FCPBGAH_POLARIS_2P_85C`  
**HP**：XDiv=3, YDiv=5, XPitch=65mm, YPitch=70mm, 雙盤 iPlateSelect=3

有效格位：每半盤 3×5−1=**14 格**，合計 **28 顆** IC。

```
放料順序（iInArmType=e9045_1x2_2_14）：
HP0-Left：
  趟次 1: [Col0, Row0] + [Col1, Row0]   2顆同步
  趟次 2: [Col0, Row1] + [Col1, Row1]   2顆同步
  趟次 3: [Col0, Row2] + [Col1, Row2]   2顆同步
  趟次 4: [Col0, Row3] + [Col1, Row3]   2顆同步
  趟次 5: [Col0, Row4] + [Col1, Row4]   2顆同步
  趟次 6: [Col2, Row0]                  1顆 (Sucker Aa, j2=0, 偶Row)
  趟次 7: [Col2, Row1]                  1顆 (Sucker Ad, j2=3, 奇Row)
  趟次 8: [Col2, Row2]                  1顆 (Sucker Aa, j2=0, 偶Row)
  趟次 9: [Col2, Row3]                  1顆 (Sucker Ad, j2=3, 奇Row)
 [Col2, Row4] ← 跳過

HP1-Right（同樣結構，趟次 10~18）
```

---

## 16. `DoInArmPickFromHotPlate_9045()` — 取料狀態機

> Source: `ainarm_SearchPickPlate.cpp` L658–1166  
> **InArm 呼叫鏈**\: ht9045-inarm-flow SKILL case 1500 → `DoInArmPickFromHotPlate_9045()`  
> **狀態變數**: `iInArmPickFromHotPlateTask`

| Task | 動作 | 備註 |
|------|------|------|
| **1** | 呼叫 `SearchPlateToPick()` → `HasHotReadyIC_9045()` | 決定 `iPickPlate[0]`、`iPickPlateX/Y[0]`、`iWhichShtPickFor32`、`iWhichKitPickFor32` |
| **50** | Z 抬升至安全高度 | 確保 InArm 移動前不碰到 HP |
| **100** | 依 `PickFromHPList` 設定 `bZFlgToHP` 旗標陣列 | 控制哪幾個 Sucker 要下壓 |
| **110** | `MoveInArmXYPickHotPlate_9045()` — XY 移至目標格位 | 同時設定 `iHotWhichShuttle`、`iHotWhichKit` |
| **150** | 等待目標 Shuttle 槽位空出 | `CheckShuttleHasICForInArmPlaceHP()` |
| **190** | 確認 XY 到位 | `CheckXYFinish()` |
| **200** | `MoveInArmZToHotPlatePick()` — Z 下壓取料 | 依 `bZFlgToHP[]` 逐 Sucker 控制 |
| **300** | `HotplateDataConversion()` 取料資料交換 | 真空吸取、資料複製 |
| **320** | JAM0109 取料錯誤 Retry 處理 | `bPickHPDuplicateErr` 觸發 |
| **330** | JAM0109 取料錯誤 Skip 處理 | `PorcessJAM0109HotPlatePickUpErrorSkip()` |
| **340** | Z 抬升 | 取料完成後離開 HP |
| **350** | `DataForwardAndNextTeam()` — 資料轉發 | 準備下一組取料或轉至放料 |
| **400** | 計數並判斷是否完成 | `bPickFormHotplatePartOK = true` |

**回傳**: `true` 表示本次取料完成（進入後續 Shuttle 放料流程）。

---

## 17. `HotplateDataConversion()` — 取料資料交換

**位置**: `ainarm_SearchPickPlate.cpp` L158  
**作用**: 與 `DoPlaceToHPSwapData()` 對稱，逆向執行資料交換。

### 執行步驟
1. 對目標 HP 格位發出真空吸取指令（`SuckOnFor…`）
2. 從 `HPTray[p][c][r]` 複製 IC 描述資料至 `InArmSuck[k]`
3. 複製 `iHotPlateCount[p][c][r]` → `InArmSuck[k].HotCount`
4. 複製 `iHotWhichShuttle[p][c][r]` → `InArmSuck[k].iHotWhichShuttle`
5. 複製 `iHotWhichKit[p][c][r]` → `InArmSuck[k].iHotWhichKit`
6. 呼叫 `CopyFromTray(p, c, r, HAS_NULL_IC)` — 清除 HP Tray 狀態為空
7. 清除追蹤陣列: `iHotWhichShuttle[p][c][r] = -1; iHotWhichKit[p][c][r] = -1; iHotPlateCount[p][c][r] = -1`

### 關鍵注意事項
- `HotTime[p][c][r]` 在此函式中**不清零**，由外層流程在確認取料後另行清零
- `bPickHPDuplicateErr` 若 HP Tray 狀態檢查發現重複取料，於此步驟設為 `true` → 觸發 JAM0109

---

## 18. `SearchPlateToPick()` — 取料格位搜尋

**位置**: `ainarm_SearchPickPlate.cpp` L1219  
**作用**: 從 `PickFromHPList` 取出下一組需要取料的格位清單。

```cpp
void SearchPlateToPick()
{
    PickFromHPList->GetHPFirstTeamPlate(
        iPickPlate[0], iPickPlateY[0], iPickPlateX[0],
        iWhichShtPickFor32, iWhichKitPickFor32);
}
```

| 輸出變數 | 說明 |
|----------|------|
| `iPickPlate[0]` | 目標熱盤編號（0 或 1） |
| `iPickPlateY[0]` | 目標 Col（Y 方向索引） |
| `iPickPlateX[0]` | 目標 Row（X 方向索引） |
| `iWhichShtPickFor32` | 目標 Shuttle 編號（32-site 時有效） |
| `iWhichKitPickFor32` | 目標 Kit 編號（32-site 時有效） |

**PickFromHPList 結構**: 由 `DoPlaceToHPSwapData()` 在放料完成時向隊列寫入，取料端消費；採用 FIFO 順序確保取料與放料路徑一致。

### ⚠️ `GetHPFirstTeamPlate()` 的 `iLimitcount` 永久 latch Bug（2026-06-13 偉測 KLD019 案例）

> **症狀**：盤中明明有料、`PickFromHPList` 也有有效 group，卻一直跳 `DoInArmPickFromHotPlate_9045 No PickFromHPList`，**且 HOME / One Cycle / Clean Out / Auto Site Map 全部無效，只有「重啟 Handler 程式」才能恢復**。

`GetHPFirstTeamPlate()`（`Public/HTEditList.cpp`）的保護迴圈有兩個缺陷，會交互造成**永久誤報空佇列**：

```cpp
// 原始（含 bug）
int iStep = HPGroup->GetTeamCount();
while(iStep==0 && HPGroup!=NULL && iLimitcount<500){   // iLimitcount 是 member 變數！
    HPGroup->DataForward();          // = DataForwardAndDelete(HPSuckTeamList) → 只動「團的 team 清單」
    HPGroup = ExtractSuckGroup(0);   // group 沒被刪 → 還是回同一個空 group
    iStep = HPGroup->GetTeamCount(); // 還是 0 → 空轉
    iLimitcount++;
}
if(iLimitcount>=500) return false;   // ← 直接 return，沒把 iLimitcount 歸零！
iLimitcount=0;                       // ← 只有正常路徑才會執行到
```

1. **空轉**：當 `group[0]` 的 team 數為 0，`HPGroup->DataForward()` 是空操作（`DataForwardAndDelete` 在 `list->Count==0` 直接 return），group 不會被刪 → 迴圈空轉到 `iLimitcount=500`。
2. **永久 latch**：`iLimitcount` 是 `uPlateInfo` 的 **member 變數**，**只在 constructor 歸零**。一旦衝到 500，提早 `return false` 不歸零，之後**每次呼叫**（即使 `group[0]` 已有有效 team）都在 `if(iLimitcount>=500) return false` 直接吐 false。
3. `PickFromHPList` 是全域物件、整支程式只 `new` 一次（`main.cpp`），recipe reload / One Cycle / HOME / ASM 都不重建它 → latch 後**唯一解法是重啟 Handler 程式**。

**判讀重點**：`No PickFromHPList` 訊息會「假空」誤報——StateRecord 的 `PickHPRec.json` 可能顯示佇列**有效且滿**（`group[0]` 有 team），但機台仍一直報空。看到「佇列有效卻持續報空 + 操作員所有恢復動作無效」即命中此 bug。**經查至 V3.33.905.10（2026-06-10）仍未修。**

**修正建議**（`GetHPFirstTeamPlate`）：① 將 `iLimitcount` 由 member 改為函式內本地變數（移除 `HTEditList.h` 宣告與 constructor 初始化），消除跨呼叫 latch；② 迴圈內改 `DataForwardAndDelete(HPSuckGroupList)` 真正移除空 group，消除單次呼叫 500 次空轉。

> **① 與 ② 各自解決的問題不同（決定能否「不重啟自癒」）**：
> - **只做①**：移除永久 latch。對「`group[0]` 有效、純粹被 latch 卡死」（本案就是這種，快照 `group[0]` 有 team）→ 下次呼叫即自動恢復、**不需重啟**。**但**若是「空 group 卡在佇列最前面」（`group[0]` team 數=0、後面才有有效 group）→ 每次呼叫仍會空轉 500 次後回 false，且 `GetHPFirstTeamPlate`（未做②時）與 `DataForwardAndNextTeam`（`GetTeamCount()<=0` 直接 return）**都不會移除那個空 group** → 持續失敗、仍需重啟或 recipe reload（ClearGroupList）。
> - **①+②**：連「空 group 卡前面」也能不重啟自癒（②會真正刪掉空 group 往後找），並消除空轉。完整保險建議兩者一起做；②改動佇列行為，建議單獨上機驗一輪。
> - **case 1 的 `ShowMyMessage("No PickFromHPList")` 在根因（①）修好後不需改成 alarm**——它回到「佇列真的空」的正確語意（罕見），維持原樣即可。

詳見 [staterecord deadlock-patterns.md Pattern #9](../../ht9045-staterecord-analysis/references/deadlock-patterns.md)。

---

## 19. `HasHotReadyIC_9045()` — 檢查 HP 是否有 IC 可取

**位置**: `ainarm_SearchPickPlate.cpp` L1248  
**呼叫位置**: InArm 狀態機決策點，判斷是否進入取料流程

### 判斷邏輯
```
HasHotReadyIC_9045()
├── SearchPlateToPick()           ← 取得第一組待取格位
├── for each point in first team:
│   ├── if HotTime[p][c][r] >= Prod.iHotTime → 可取料
│   └── if 格位狀態 = NULL_IC (幽靈格位)
│       ├── DataForwardAndNextTeam()  ← 跳過此組
│       └── 或呼叫 CopyFromTray(HAS_NULL_IC) 修正狀態
└── return true / false
```

| 條件 | 動作 |
|------|------|
| `HotTime >= Prod.iHotTime` | 回傳 `true`，可進行取料 |
| `HotTime < Prod.iHotTime` | 回傳 `false`，繼續等待 |
| 格位為 `NULL_IC`（幽靈格位） | 呼叫 `DataForwardAndNextTeam()` 跳過，重新檢查下一組 |
| `PickFromHPList` 為空 | 回傳 `false` |

**`Prod.iHotTime`**: 從工作檔 `HotPlate.Data` 的 `Soak Time` 欄位讀取（單位：秒），預設 0 表示不需等待。

---

## 20. `PorcessJAM0109HotPlatePickUpErrorSkip()` — HP 取料 Skip 處理

**位置**: `ainarm_SearchPickPlate.cpp` L112  
**觸發條件**: JAM0109（HotPlate PickUp Error）使用者按下 `K_SKIP`

### 執行動作
1. 重設 `bPickHPDuplicateErr = false`
2. 對發生錯誤的格位呼叫 `CopyFromTray(p, c, r, HAS_NULL_IC)` — 強制清除格位狀態
3. 清除該格位的追蹤陣列：
   - `HotTime[p][c][r] = 0`
   - `iHotWhichShuttle[p][c][r] = -1`
   - `iHotWhichKit[p][c][r] = -1`
   - `iHotPlateCount[p][c][r] = -1`
4. 遞增 SCK ART 計數器（記錄良率損失）
5. 重設 `iInArmPickFromHotPlateTask = 1` — 返回搜尋狀態

**對應放名**: `PorcessJAM0109HotPlatePickUpErrorSkip`（檔案中原文有 typo "Porccess"，為 Process）

---

## 21. `GetHeaterWaitTime()` — 取得剩餘加熱等待時間

**位置**: `ainarm_SearchPickPlate.cpp` L32  
**呼叫位置**: UI 顯示、InArm 等待判斷

```cpp
int GetHeaterWaitTime(int iPlate, int iCol, int iRow)
{
    int iWait = Prod.iHotTime - HotTime[iPlate][iCol][iRow];
    if (iWait < 0) iWait = 0;
    return iWait;
}
```

| 參數 | 說明 |
|------|------|
| `iPlate` | 熱盤編號 0/1 |
| `iCol` | Col 索引（X 方向） |
| `iRow` | Row 索引（Y 方向） |
| **回傳** | 剩餘等待秒數，最小為 0 |

**用途**: UI 上每格顯示剩餘加熱時間；`HasHotReadyIC_9045()` 也使用此值做門檻判斷。

---

## 22. `iHotCount` 計數器與批次識別

**作用**: 連結放料批次與取料批次，確保 OutArm 路由正確。

### 放料端（`DoPlaceToHPSwapData`）
```cpp
iHotPlateCount[p][c][r] = iHotCount;  // 記錄放入時的批次編號
iHotCount++;                           // 全域批次遞增
```

### 取料端（`HotplateDataConversion`）
```cpp
InArmSuck[k].HotCount = iHotPlateCount[p][c][r];  // 傳遞批次至 OutArm
```

### OutArm 使用
- `InArmSuck[k].HotCount` → Shuttle 資料 → OutArm 讀取
- OutArm 依 `HotCount` 查詢 `iHotWhichShuttle` / `iHotWhichKit`，送往正確的 Bin 格位

### 批次計數特性
| 特性 | 說明 |
|------|------|
| 單調遞增 | 每次放料 +1，不重置（除機台 Reset） |
| 格位獨立 | 每個 `[p][c][r]` 各自儲存對應批次號 |
| 清除時機 | `HotplateDataConversion()` 取料後設 `iHotPlateCount[p][c][r] = -1` |
| 溢位安全 | `int` 型別，正常使用下不會溢位 |

---

## 附錄A：InArm 上下eContext

- **放料區執行次數**：InArm 一個 cycle 可支援複數 HP 放料（取決於 Loader IC 正伸在 InArm Sucker 上）
- **同時執行流程**：ht9045-inarm-flow SKILL § 4, Hot Mode 顯示控佋
  - case 1100 放料到 HP
  - case 1500 取料侞 HP（待 Soak 完成）
  - case 2000 放料到 Shuttle
- **外生丢布值**：`iHotCount` (熟設目錄：ht9045-hotplate § 22)

```
DoPlaceToHPSwapData()
  └─ HPTray[p][c][r]          ← IC 描述資料
  └─ iHotPlateCount[p][c][r]  ← 批次號 iHotCount
  └─ iHotWhichShuttle[p][c][r]← 來源 Shuttle 編號
  └─ iHotWhichKit[p][c][r]    ← 來源 Kit 編號
  └─ HotTime[p][c][r] = 0     ← 開始計時
  └─ PickFromHPList.Push(p,c,r)

HasHotReadyIC_9045()
  └─ HotTime[p][c][r] >= Prod.iHotTime → 可取料

HotplateDataConversion()
  └─ InArmSuck[k] ← HPTray[p][c][r]   IC 資料
  └─ InArmSuck[k].HotCount ← iHotPlateCount[p][c][r]
  └─ InArmSuck[k].iHotWhichShuttle ← iHotWhichShuttle[p][c][r]
  └─ InArmSuck[k].iHotWhichKit    ← iHotWhichKit[p][c][r]
  └─ CopyFromTray(p,c,r, HAS_NULL_IC)  清空 HP 格位
  └─ iHotWhichShuttle/Kit/PlateCount[p][c][r] = -1

OutArm
  └─ Shuttle.HotCount → 查 iHotWhichShuttle/Kit → 正確 Bin 路由
```

---

## References — 歷次除錯與修改紀錄

本章節列舉 HotPlate 放料相關模組的已知問題與修正紀錄。詳細分析與實作指南存放於 `references/` 子資料夾。

### REF-001：`iForPlaceHPX3Step` Race Condition（3x5 HP 1x2 Mode）

**問題**：多執行緒競爭導致 Col2 放料路徑選擇錯誤  
**症狀**：位置偏差警告、吸嘴對應格位異常  
**修正**：改用 Row 奇偶性判斷替代吸嘴實時狀態  
**文件**：[REF-001_iForPlaceHPX3Step_RaceCondition.md](references/HP_REF-001_iForPlaceHPX3Step_RaceCondition.md)

### REF-002：`GetPlaceToHotPlateCol()` Race Condition（3x5 HP 1x2 Mode）

**問題**：Col 索引計算因吸嘴狀態污染走錯分支，導致 IC 寫入超界或遺失  
**症狀**：`HotPlate Data Swap error 1` 報警、重複 IC 衝突  
**修正**：改用 Row 奇偶性判斷  
**文件**：[REF-002_GetPlaceToHotPlateCol_RaceCondition.md](references/HP_REF-002_GetPlaceToHotPlateCol_RaceCondition.md)

### REF-003：新增 Debug Log（Proposal A + B）

**目的**：加速 Race Condition 類問題的排查  
**內容**：
- Proposal A：`SearchPlacePlateXItem3_1x2Suck()` 搜尋結果 log
- Proposal B：`GetPlaceToHotPlateCol()` Col2 路徑 log

**文件**：[REF-003_Debug_Log_Addition.md](references/HP_REF-003_Debug_Log_Addition.md)

### REF-004：同類條件風險提示

**範圍**：其他函式中類似的吸嘴實時狀態判斷  
**狀態**：待驗證，列為預防性維護  
**文件**：[REF-004_Race_Condition_Risk_Assessment.md](references/HP_REF-004_Race_Condition_Risk_Assessment.md)

### REF-005：Step 400 HasIC() 統一修正（3x5 HP Col2 IC 遺失）

**問題**：`GetPlaceToHotPlateSuckCol()` 迴圈無法偵測 Ad 吸嘴是否仍有 IC，致 IC 永久遺失  
**症狀**：HP Col2 Row1+ 全空、IC 資料不對稱  
**修正**：改用 `InArmSuck.HasIC()` 全臂掃描  
**影響**：4 個狀態機檔案  
**文件**：[REF-005_Step400_HasIC_Fix.md](references/HP_REF-005_Step400_HasIC_Fix.md)
