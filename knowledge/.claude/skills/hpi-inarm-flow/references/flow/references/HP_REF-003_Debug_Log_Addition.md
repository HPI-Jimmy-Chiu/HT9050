> 保存來源：`.claude/skills/ht9045-inarm-flow/references/HP_REF-003_Debug_Log_Addition.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# REF-003：新增 Debug Log（Proposal A + B）

## 目的

為加快未來同類 Race Condition 異常的排查速度，在 `SearchPlacePlateXItem3_1x2Suck()` 和 `GetPlaceToHotPlateCol()` 中增加診斷性 EventLog，記錄搜尋與 Col 計算的實時狀態。

## 重點資訊

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **優先等級** | 中：非關鍵功能，但助益於問題排查 |
| **觀察方式** | 機台 EventLog，查 `Message` 欄位 |

## Proposal A：`SearchPlacePlateXItem3_1x2Suck()` 搜尋結果 Log

### 目的

每次 `SearchPlacePlateXItem3_1x2Suck()` 搜尋到目標格位時立即記錄搜尋參數，以便事後確認：
- 搜尋到的格位是否正確
- `iForPlaceHPX3Step` 是否被正確設定
- Aa 吸嘴在搜尋當下的實際狀態

### 實作位置

**檔案**：`ainarm_SearchPlacePlate.cpp`  
**函式**：`SearchPlacePlateXItem3_1x2Suck()`  
**時機**：找到空格位且決定 `iForPlaceHPX3Step` 之後，`return` 前

### 實作建議

```cpp
// 在 SearchPlacePlateXItem3_1x2Suck() 的 return 前加入：
EventLog.WriteMessage(
    "SearchHP3", 
    "x=%d, y=%d, Step=%d, Aa=%d", 
    iPlacePlateX[0], 
    iPlacePlateY[0], 
    iForPlaceHPX3Step,
    (int)InArmSuck.Item[0][0]
);
return true;
```

### Log 格式

```
Message: SearchHP3:x=2,y=1,Step=1,Aa=0
```

| 參數 | 含義 |
|------|------|
| `x` | 搜尋到的 Col 索引 |
| `y` | 搜尋到的 Row 索引 |
| `Step` | 設定的 `iForPlaceHPX3Step`（0=Col0+1 同步, 1=Col2 單顆） |
| `Aa` | Aa 吸嘴狀態（0=NULL_IC, 1=HAS_IC, 2=HAS_HOT_IC, 3=HAS_NULL_IC） |

### 分析方式

- 同一筆 Set 中，若多個 SearchHP3 log 的 `Step` 相同，表示邏輯一致
- 若 Step 值在同一 Row 內波動，表示 Race Condition 已發生

## Proposal B：`GetPlaceToHotPlateCol()` Col2 路徑 Log

### 目的

每次走到 Col2（x=2）放料路徑時記錄計算參數，以便事後確認：
- Col 索引 `ix` 的計算結果是否合理
- Aa 狀態在計算當下是否已被污染

### 實作位置

**檔案**：`ainarm_SearchPlacePlate.cpp`  
**函式**：`GetPlaceToHotPlateCol()`  
**區段**：`bUseAxxGPicker` 的 `ix=2` 或計算分支

### 實作建議

```cpp
// 在 GetPlaceToHotPlateCol() 的 bUseAxxGPicker 分支中，x==2 時加入：
EventLog.WriteMessage(
    "GetHPCol",
    "x=%d, y=%d, j=%d, ix=%d, Aa=%d",
    iPlacePlateX[0],
    iPlacePlateY[0],
    j,
    ix,
    (int)InArmSuck.Item[0][0]
);
```

### Log 格式

```
Message: GetHPCol:x=2,y=1,j=1,ix=2,Aa=1
```

| 參數 | 含義 |
|------|------|
| `x` | 目標 Col（應為 2） |
| `y` | 目標 Row |
| `j` | 放料步驟索引 |
| `ix` | 最終計算得出的 Col 索引 |
| `Aa` | Aa 吸嘴當時狀態 |

### 分析方式

- 若 `ix > 2`（超界），表示計算分支走錯
- 若 Aa 從 0（NULL_IC）變成 1（HAS_IC），表示 Race Condition 已發生

## EventLog 觀察流程

1. 機台停機，複製 EventLog database 或導出 CSV 報表
2. 過濾 Message 欄位：`LIKE '%SearchHP3%'` 和 `LIKE '%GetHPCol%'`
3. 依時間排序
4. 對比 `SearchHP3` 與 `GetHPCol` 同一批次內的時間差與參數變化
5. 如見 `Step` / `ix` 異常值，即為 Race Condition 發生點

## 相關檔案

- [REF-001_iForPlaceHPX3Step_RaceCondition.md](HP_REF-001_iForPlaceHPX3Step_RaceCondition.md)
- [REF-002_GetPlaceToHotPlateCol_RaceCondition.md](HP_REF-002_GetPlaceToHotPlateCol_RaceCondition.md)

<!-- preserved-content:end -->
