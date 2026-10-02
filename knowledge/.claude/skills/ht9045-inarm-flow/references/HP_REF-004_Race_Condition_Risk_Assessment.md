# REF-004：同類條件風險提示

## 目的

REF-001 與 REF-002 針對 `Item[0][0]==NULL_IC` Race Condition 進行修正，但原始碼中仍存在其他類似的吸嘴實時狀態判斷。本文件列舉已識別到的風險點，為未來版本的驗證與維護提供參考。

## 重點資訊

| 項目 | 內容 |
|------|------|
| **狀態** | 待驗證：已識別風險但未確認在相同機台配置下是否觸發 |
| **排查** | 需使用 REF-003 提議的 Debug Log 進行實機驗證 |
| **優先等級** | 中：預防性維護 |

## 識別到的風險點

### 1. `GetPlaceToHotPlateCol()` 中 `bUseAxExPicker` 區段

| 欄位 | 內容 |
|------|------|
| **檔案** | `ainarm_SearchPlacePlate.cpp` |
| **位置** | ~line 940 |
| **區段** | `bUseAxExPicker` 條件判斷 |
| **適用機台** | `e9045_1x2_2_13` 配置 |
| **潛在問題** | 若同樣使用 `InArmSuck.Item[0][0]==NULL_IC` 判斷，可能在 1x2_13 Picker 上也出現 Race Condition |

### 2. `SearchPlacePlateXItem3_2x2Suck()` 中的條件判斷

| 欄位 | 內容 |
|------|------|
| **檔案** | `ainarm_SearchPlacePlate.cpp` |
| **位置** | ~line 3115 |
| **函式** | `SearchPlacePlateXItem3_2x2Suck()` |
| **區段** | `bUseAxxGPicker` 條件判斷 |
| **適用機台** | 2x2 Picker mode（XDiv=3） |
| **潛在問題** | 2x2 版本搜尋邏輯與 1x2 版本相似，若採用相同判斷方式，可能遭遇相同 Race Condition |

## 驗證建議

### 短期（現有版本）

1. **部署 REF-003 Debug Log**：在上述兩處函式新增 EventLog 記錄
2. **實機測試**：在對應機台配置下執行高溫工作檔，記錄 EventLog
3. **對比分析**：檢查 `Item[][]` 狀態在短時間內是否出現異常波動

### 長期（下一版本）

1. 統一改用設計層面的邏輯條件（如 Row 奇偶性、Picker 索引等），替代所有吸嘴實時狀態判斷
2. 開發 Code Review CheckList，防止其他新增都遺漏此類陷阱
3. 加強多執行緒單元測試框架（模擬 Race Condition 場景）

## 附參考

- [REF-001_iForPlaceHPX3Step_RaceCondition.md](REF-001_iForPlaceHPX3Step_RaceCondition.md)
- [REF-002_GetPlaceToHotPlateCol_RaceCondition.md](REF-002_GetPlaceToHotPlateCol_RaceCondition.md)
- [REF-003_Debug_Log_Addition.md](REF-003_Debug_Log_Addition.md)
