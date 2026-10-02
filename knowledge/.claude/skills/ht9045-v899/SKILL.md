---
name: ht9045-v899
description: "HT9045 V3.33.899.0 版本知識庫。涵蓋版本編譯旗標、模組索引、iInArmType 分派表、HotPlate DataSwap 觸發邏輯、iArmTask 狀態機。Use when: 分析 V899 版本程式碼、查 HotPlate Data Swap error、追蹤 InArm 16-site 流程、比對版本差異。關鍵字：V899, V3.33.899, HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422, HotPlate Data Swap, ainarm_SearchPlacePlate, DoPlaceToHPSwapData, iArmTask, iInArmType, 2x4_16, 2x8_32"
---

# HT9045 V3.33.899.0 版本知識庫

## 與 Weekly_AI 的分工

本 skill / agent 負責 V899 程式與異常分析：定位 alarm、比對版本差異、修改 V899 C/C++、提供 root cause / fix / validation / Weekly 備註建議。

週報、case 狀態、release note、鴻勁紅 / 鴻勁藍產物不由本 skill 產出。若使用者要求「結案」、「週報」、「release note」、「鴻勁紅」、「鴻勁藍」、「提供安裝包紀錄」，應交給 Weekly_AI / weekly-report agent 處理，正式產物路徑為 `Customer/<客戶>/<CASE>/04_release/`。

handoff 給 Weekly_AI 時至少包含：客戶、機型、版本、CASE-ID（若有）、問題現象、根因、修正檔案與函式、驗證結果、對外描述。

## 版本路徑

```
d:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\
```

## 編譯旗標

| 旗標 | 值 | 狀態 |
|------|-----|------|
| `ALIAS` | "HT9045" | 啟用 |
| `ATC_HEAD_COUNT` | 32 | 啟用 |
| `INDEX_HEAT_COUNT` | 32 | 啟用 |
| `USE_CompareCommandPos` | — | 啟用 |
| `DEBUG_SOFTWARE_EXEC_TIME` | — | 啟用 |
| `ATC_SW_VER` | 1031 | 啟用 |
| `ATC_FW_VER` | 1032 | 啟用 |
| `SOFT_SIMULTE` | — | 未啟用（模擬模式） |
| `DEBUG_OneCycleHotPlate` | — | 未啟用 |

## 模組快速索引

| 模組 | 主檔案 | 行數 | 狀態機變數 |
|------|--------|-----|-----------|
| InArm 分派 | `ainarm9045.cpp` | ~10,890 | iInArmType 分派 |
| InArm 2x4-16 | `ainarm9045_2x4_16.cpp` | ~2,877 | iArmTask |
| HP 放料搜尋 | `ainarm_SearchPlacePlate.cpp` | ~4,700 | iInArmPlaceToHotPlateTask |
| HP 取料搜尋 | `ainarm_SearchPickPlate.cpp` | ~4,700 | iInArmPickFromHotPlateTask |
| OutArm 分派 | `aoutarm9045.cpp` | — | OutArmTask |
| Index | `atester.cpp`, `atester_32Site.cpp` | — | DoTestY 系列 |
| Shuttle | `uShuttleThread.cpp` | — | AutoSHT1Task, AutoSHT2Task |
| CatchTray | `acatchtray.cpp` | — | CatchTrayTask |
| Kernel | `ckernel.cpp` | — | 主控排程 |
| Initial | `cinitial.cpp` | — | 系統初始化 |

## iInArmType 分派表

InArm 根據 `iInArmType` 呼叫對應 site 配置的子函式（`ainarm9045.cpp` L1502-1588）：

| iInArmType | 函式 | 配置 |
|------------|------|------|
| `e9045_1x1_1` | `DoInArm_9045_All_1Pick()` | 1×1, 1 picker |
| `e9045_1x2_2_13` | `DoInArm_9045_1x2_2()` | 1×2, 2 吸嘴 |
| `e9045_1x2_4_Hot` | `DoInArm_9045_1x2_4_Hot()` | 1×2, 4 吸嘴 (HotPlate) |
| `e9045_1x4_4` | `DoInArm_9045_1x4_4()` | 1×4, 4 吸嘴 |
| `e9045_1x4_8_Hot` | `DoInArm_9045_1x4_8_Hot()` | 1×4, 8 吸嘴 (HotPlate) |
| `e9045_2x2_4_13` | `DoInArm_9045_2x2_4()` | 2×2, 4 吸嘴 |
| `e9045_2x2_8_Hot` | `DoInArm_9045_2x2_8_Hot()` | 2×2, 8 吸嘴 (HotPlate) |
| `e9045_2x4_4_13` | `DoInArm_9045_2x4_4_13()` | 2×4, 4 吸嘴 |
| `e9045_2x4_8` | `DoInArm_9045_2x4_8()` | 2×4, 8 吸嘴 |
| **`e9045_2x4_16`** | **`DoInArm_9045_2x4_16()`** | **2×4, 16 吸嘴** |
| `e9045_2x8_8` | `DoInArm_9045_2x8_8()` | 2×8, 8 吸嘴 |
| `e9045_2x8_32` | `DoInArm_9045_2x8_8()` | 2×8, 32 吸嘴 |

其餘變體見 [references/inarm-variants.md](references/inarm-variants.md)。

## iArmTask 狀態機 (2x4-16)

`ainarm9045_2x4_16.cpp` 的 `DoInArm_9045_2x4_16()` 主狀態機：

```
case 1   → 移至等待位；檢查 OneCycle/CleanOut
case 10  → 決策中樞 → 依 InArm 上的 IC 狀態路由
case 50  → 準備取料：檢查 Heater/Tray/FIFO
case 100 → 從 Loader 取料 (DoArmPickFromLoadStage_9045_2x4_16)
case 200 → Close Site Check
case 1000-1050 → 吸取重試 & JAM0114 錯誤處理
case 1100 → 放料至 HotPlate
case 1500 → 從 HotPlate 取料 (DoInArmPickFromHotPlate_9045_2x4_16)
case 2000 → 放料至 Shuttle (DoPlaceToShuttle_9045_2x4_16)
case 2600-2610 → Y-Pitch Home 馬達序列
```

## HotPlate Data Swap Error 觸發邏輯

**檔案**：`ainarm_SearchPlacePlate.cpp` L4615-4690

**函式**：`DoPlaceToHPSwapData(iSuckRow, iSuckCol, iP, iPlateR, iPlateC)`

**功能**：InArm 將 IC 放到 HotPlate 後，把吸嘴資料轉移到 HotPlate Tray 陣列

### Error 觸發條件

```
前置：邊界檢查 → iSuckRow<0 || iSuckRow>=2 || iSuckCol>=8 → return
資料寫入：iHotWhichShuttle / iHotWhichKit / iHotPlateCount / PickFromHPList
```

| Error | 行號 | 條件 | 含義 |
|-------|------|------|------|
| **Error 1** | L4661 | `Tray.Data[c][r] == HAS_HOT_IC/HAS_IC` **且** `InArmSuck.Item[row][col] == HAS_IC` | HotPlate 該格已有 IC，但 InArm 也帶有 IC → 重複放料衝突 |
| **Error 2** | L4668 | `Tray.Data[c][r] == HAS_HOT_IC/HAS_IC` **且** `InArmSuck.Item[row][col] == HAS_NULL_IC` | HotPlate 該格已有 IC，但 InArm 帶的是空 IC (NULL_IC) → 資料不一致 |

### 額外 Swap 檢測

| Error | 行號 | 條件 |
|-------|------|------|
| **Rotate Data Swap** | L4676 | `USE_ROTATE_KIT==1` 且旋轉角度記錄為空 |

### Debug Hook

觸發前會呼叫：`fMain->DebugOneCycleHotPlate("DoPlaceToHPSwapData1/2")`
（需啟用 `DEBUG_OneCycleHotPlate` 旗標才有完整 log）

### HotPlate 取料狀態機

`ainarm_SearchPickPlate.cpp` 的 `DoInArmPickFromHotPlate_9045()` (L683)：

| case | 動作 |
|------|------|
| 1 | SearchPlateToPick — 找 HP 上有 IC 的位置 |
| 50 | Z 移到安全高度 |
| 100-110 | 取得 FirstTeam 映射，移 XY 到 HP 取料位 |
| 150-200 | 等 Shuttle 淨空，控制下壓速度 |
| 300 | HotplateDataConversion — HP 位置轉吸嘴映射 |
| 330 | 重試計數，顯示 JAM0109 |
| 350 | DataForwardAndNextTeam — 多團次循環 |
| 400 | 完成，回傳 true |

## 共用參數檢查點

分析問題時同步檢查：

| 參數 | 路徑 | 重點欄位 |
|------|------|----------|
| 機台設定 | `system/Gerneral.ini` | IO/馬達基本配置 |
| 馬達表 | `system/Mot_Table.csv` | 馬達型號/位址 |
| IO 表 | `system/IO_Table.csv` | IO 對應 |
| 溫控 | `config/ATC.ini` | 加熱盤設定 |
| Recipe | `config/config.ini` | 運行參數 |
| 教導 | `system/teach.ini` | 位置偏移 |
| 機台設定 | `setup.inf` | 基本配置 |

## 客戶代碼影響

V899 定義 30+ 客戶代碼（`MachineType.h`），影響流程分歧：
- `CC_AMKOR_China` / `CC_QUALCOMM`：啟用 HotPlate Data Swap 檢查（QA+離線模式）
- HiSilicon / Nvidia / TSMC：各有專屬流程分支

分析報警時需確認 `CUSTOMER_CODE` 值。
