> 保存來源：`.claude/skills/ht9045-inarm-flow/references/HP_Item_NULL_IC_RaceCondition.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# HP `Item[][]` NULL_IC Race Condition 家族（整合報告）

> **整合日期**：2026-05-25
> **整合範圍**：原 HP_REF-001 / 002 / 003 / 004（同一 Race Condition 家族）
> **不在本檔範圍**：HP_REF-005（屬獨立 Bug 類別，請見 [HP_REF-005_Step400_HasIC_Fix.md](HP_REF-005_Step400_HasIC_Fix.md)）

---

## 0. 整合說明

原本以 4 份獨立檔案記錄的問題，其根本原因都是同一個：

> 在多執行緒環境下，使用 `InArmSuck.Item[0][0]==NULL_IC` 等**吸嘴實時狀態**作為放料路徑判斷條件，會被取料執行緒的填充動作競爭污染，造成路徑分支錯誤。

為避免讀者跨檔追查、降低同類問題未來的維護成本，特將四份整合為一份。原始 REF 編號對照於 §1。

| 章節 | 內容 | 對應原 REF |
|------|------|-----------|
| §1 | 原始 REF 對照表 | — |
| §2 | 根本原因（Race Condition 共通機制） | REF-001 / 002 |
| §3 | 修正案例 A：`SearchPlacePlateXItem3_1x2Suck()` | REF-001 |
| §4 | 修正案例 B：`GetPlaceToHotPlateCol()` `bUseAxxGPicker` 區段 | REF-002 |
| §5 | 修正原理：為何 `y%2==1` 安全 | REF-001 / 002 |
| §6 | Debug Log 提案（A：SearchHP3 / B：GetHPCol） | REF-003 |
| §7 | 已識別但未驗證的同類風險點 | REF-004 |
| §8 | 統一驗證與長期防護建議 | REF-003 / 004 |

---

## 1. 原始 REF 對照表

| 原 REF | 標題 | 整合後章節 | 狀態 |
|--------|------|-----------|------|
| REF-001 | `iForPlaceHPX3Step` Race Condition (3x5 HP 1x2 Mode) | §3 + §5 | ✅ 已修正 |
| REF-002 | `GetPlaceToHotPlateCol()` Race Condition (3x5 HP 1x2 Mode) | §4 + §5 | ✅ 已修正 |
| REF-003 | 新增 Debug Log（Proposal A + B） | §6 | 🟡 待部署 |
| REF-004 | 同類條件風險提示 | §7 | 🟡 待驗證 |

---

## 2. 根本原因（Race Condition 共通機制）

### 2.1 反模式

```cpp
// 反模式 ❌ — 用實時吸嘴狀態判斷放料路徑
if (x==2 && InArmSuck.Item[0][0]==NULL_IC) {
    iForPlaceHPX3Step = 1;   // 或 ix = 2
}
```

`InArmSuck.Item[R][C]` 由「取料」與「放料」兩個 Task 同時讀寫：

- **InArmPickFromLoadTask**：吸取完成後將 `Item[][]` 由 `NULL_IC` 改為 `HAS_IC`
- **InArmPlaceToHotPlateTask**：搜尋空格 / 計算 Col 索引時讀取 `Item[][]`

無互斥鎖保護 → 競爭視窗只需數百毫秒即會發生。

### 2.2 通用時序

```
T+0ms     PickTask  step 1   → Suck() 完成，Item[0][0] := HAS_IC
T+30~261ms PlaceTask step 1   → SearchPlateToPlace() 讀 Item[0][0]
                              → 條件 (==NULL_IC) 判斷失敗
                              → 走錯分支
T+later   PlaceTask step 350 → 用錯誤路徑放料
                              → HotPlate Data Swap error / 位置偏差
```

### 2.3 適用機台 / 配置

| 配置 | 函式 | XDiv |
|------|------|------|
| 3x5 HP + 1x2 Picker (`e9045_1x2_2_14`, `e9045_1x3_2_14`) | `SearchPlacePlateXItem3_1x2Suck()` | 3 |
| 同上 | `GetPlaceToHotPlateCol()` `bUseAxxGPicker` 區段 | 3 |

---

## 3. 修正案例 A：`SearchPlacePlateXItem3_1x2Suck()`

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **檔案** | `ainarm_SearchPlacePlate.cpp` |
| **行號（約）** | 3079 |
| **症狀** | 模擬 log（`D:\HT9045_StateRecord\2026-04-02 11_22_15\`）出現 `InArm Suck[0,3] Pos(Y=..., X=...) Place to Hot Plate 1 [1,2]` 位置偏差警告 |
| **時序差距** | 261ms |

### 修改前 ❌

```cpp
if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
{
    iForPlaceHPX3Step = 1;     // Col2 單顆放，使用 Ad（Sucker 3）
}
```

### 修改後 ✅

```cpp
if(x==2 && y%2==1)
{
    iForPlaceHPX3Step = 1;     // Col2 奇數 Row 永遠需要 Ad 放料
}
```

### 備份

原檔案已備份至 `ainarm_SearchPlacePlate.cpp.bak_20260402`。

---

## 4. 修正案例 B：`GetPlaceToHotPlateCol()` `bUseAxxGPicker` 區段

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **檔案** | `ainarm_SearchPlacePlate.cpp` |
| **行號（約）** | 969 |
| **症狀** | `HotPlate Data Swap error 1. Please call engineer`（14:13:54） |
| **時序差距** | 30ms |
| **等級** | 🔴 高 — 直接導致 IC 資料遺失、停機 |

### 失效後果鏈

1. 取料 Task @ 14:13:53.571 → `Item[0][0] := HAS_IC`
2. 放料 Task @ 14:13:53.601 → 條件 `==NULL_IC` 判斷失敗
3. 走正常分支：`ix = iPlacePlateX[0] + j*(XDiv/2)` → 若 `iPlacePlateX[0]=2, j=1` → `ix = 3`（**超出 XDiv=3 邊界**）
4. `Tray[3][y]` 存取越界 → Ad 吸嘴的 IC **未被寫入** HP Tray
5. 手臂上 Ad 吸嘴已吹氣空了，但 HP Tray 仍無記錄
6. 下一趟放料再次掃描相同 Col2 格位 → 發現 `Tray[2][y].Data = HAS_IC`（前一趟遺留）
7. `DoPlaceToHPSwapData()` 衝突偵測 → 觸發 `HotPlate Data Swap error 1`

### 修改前 ❌

```cpp
if(bUseAxxGPicker && x==2 && hPlateRowCount==2 && iPlaceHPOrder==0)
{
    if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
    {
        ix = 2;
    }
    else
    {
        ix = iPlacePlateX[0]+j*(XDiv/2);
    }
}
```

### 修改後 ✅

```cpp
if(bUseAxxGPicker && x==2 && hPlateRowCount==2 && iPlaceHPOrder==0)
{
    if(x==2 && y%2==1)       // 奇數 Row 使用 Ad 放
    {
        ix = 2;
    }
    else
    {
        ix = iPlacePlateX[0]+j*(XDiv/2);
    }
}
```

---

## 5. 修正原理：為何 `y%2==1` 安全？

根據 HT9045 HotPlate 設計，3x5 HP 的 Col2（第三欄）放料邏輯為：

- **偶數 Row**（0, 2, 4）→ 使用 **Aa**（Sucker 0）
- **奇數 Row**（1, 3） → 使用 **Ad**（Sucker 3）

此設計**完全基於 Row 奇偶性**，與吸嘴手上是否有 IC 無關。`y` 在進入 `GetPlaceToHotPlateCol()` 時已由上游 `SearchPlacePlateXItem3_1x2Suck()` 確定，是**穩定的設計常數**，不會被任何 Task 競爭修改。

✅ 將判斷條件從「實時狀態」改為「設計常數」即可完全規避 Race Condition。

---

## 6. Debug Log 提案（REF-003 移入）

> **優先等級**：中 — 非關鍵功能，但助益於問題排查
> **觀察方式**：機台 EventLog，查 `Message` 欄位

### 6.1 Proposal A：`SearchPlacePlateXItem3_1x2Suck()` 搜尋結果 Log

**實作位置**：`ainarm_SearchPlacePlate.cpp` / `SearchPlacePlateXItem3_1x2Suck()`，找到空格位且決定 `iForPlaceHPX3Step` 之後、`return` 前。

```cpp
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

| 參數 | 含義 |
|------|------|
| `x` | 搜尋到的 Col 索引 |
| `y` | 搜尋到的 Row 索引 |
| `Step` | `iForPlaceHPX3Step`（0=Col0+1 同步, 1=Col2 單顆） |
| `Aa` | Aa 吸嘴狀態（0=NULL_IC, 1=HAS_IC, 2=HAS_HOT_IC, 3=HAS_NULL_IC） |

**分析方式**：同一筆 Set 中，若多個 SearchHP3 log 的 `Step` 在同一 Row 內波動，表示 Race Condition 已發生。

### 6.2 Proposal B：`GetPlaceToHotPlateCol()` Col2 路徑 Log

**實作位置**：`ainarm_SearchPlacePlate.cpp` / `GetPlaceToHotPlateCol()`，`bUseAxxGPicker` 分支中 `x==2` 時。

```cpp
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

**分析方式**：
- 若 `ix > 2`（超界），表示計算分支走錯
- 若 Aa 從 0（NULL_IC）變成 1（HAS_IC），表示 Race Condition 已發生

### 6.3 EventLog 觀察流程

1. 機台停機，複製 EventLog database 或導出 CSV 報表
2. 過濾 Message 欄位：`LIKE '%SearchHP3%'` 與 `LIKE '%GetHPCol%'`
3. 依時間排序
4. 對比 `SearchHP3` 與 `GetHPCol` 同一批次內的時間差與參數變化
5. 若見 `Step` / `ix` 異常值，即為 Race Condition 發生點

---

## 7. 已識別但未驗證的同類風險點（REF-004 移入）

| 風險 # | 檔案 / 函式 / 位置 | 適用機台 | 潛在問題 |
|--------|------------------|---------|---------|
| R-1 | `ainarm_SearchPlacePlate.cpp` / `GetPlaceToHotPlateCol()` / `bUseAxExPicker` 區段（~line 940） | `e9045_1x2_2_13` | 若同樣使用 `InArmSuck.Item[0][0]==NULL_IC` 判斷，1x2_13 Picker 上也可能出現 Race Condition |
| R-2 | `ainarm_SearchPlacePlate.cpp` / `SearchPlacePlateXItem3_2x2Suck()` / `bUseAxxGPicker` 區段（~line 3115） | 2x2 Picker mode（XDiv=3） | 2x2 版本搜尋邏輯與 1x2 相似，若採用相同判斷方式，可能遭遇相同 Race Condition |

> **狀態**：已識別風險但未確認在相同機台配置下是否觸發。需使用 §6 Debug Log 進行實機驗證。

---

## 8. 統一驗證與長期防護建議

### 8.1 短期（現有版本）

1. **部署 §6 Debug Log** 至 §7 兩處風險點
2. **實機測試**：在對應機台配置下執行高溫工作檔，記錄 EventLog
3. **對比分析**：檢查 `Item[][]` 狀態在短時間內是否出現異常波動

### 8.2 長期（下一版本）

1. **設計層替代實時狀態**：統一改用 Row 奇偶性、Picker 索引等設計層面的邏輯條件，替代所有吸嘴實時狀態判斷
2. **Code Review CheckList**：建立明文規則 — 凡在放料路徑判斷中讀 `*Suck.Item[][]`，必須附上「為何沒有 Race Condition」的註解
3. **多執行緒單元測試**：開發模擬 Race Condition 場景的測試框架
4. **Grep 稽核**：定期執行下列 grep 找出仍存在的反模式：

```powershell
Get-ChildItem -Recurse -Include *.cpp -Exclude .svn |
  Select-String -Pattern 'Suck\.Item\[\d+\]\[\d+\]\s*==\s*NULL_IC' |
  Where-Object { $_.Line -notmatch '//.*safe' }
```

---

## 9. 相關檔案

- [HP_REF-005_Step400_HasIC_Fix.md](HP_REF-005_Step400_HasIC_Fix.md) — 獨立 Bug：Step 400 GetPlaceToHotPlateSuckCol 對 Col2 回傳重複 j2 → IC 靜默丟失
- [HP_Knowledgebase.md](HP_Knowledgebase.md) — HotPlate 完整知識庫
- [HP_SearchPlacePlate_AllFunctions.md](HP_SearchPlacePlate_AllFunctions.md) — 全 17 個 SearchPlacePlateXItem 函式分析
- [SearchPlacePlate_Route_Proposal.md](SearchPlacePlate_Route_Proposal.md) — XDiv=2~16 路徑圖總表（JerryYang 提案）
- SKILL.md § 7：`SearchPlacePlateXItem3_1x2Suck()` — 3-Col HP 搜尋（1x2 Mode）
- SKILL.md § 9：`GetPlaceToHotPlateCol()` / `GetPlaceToHotPlateSuckCol()`

---

## 10. 變更歷史

| 日期 | 版本 | 動作 | 作者 |
|------|------|------|------|
| 2026-04-02 | V3.33.900.0 | REF-001 / REF-002 修正並部署 | — |
| 2026-04-02 | — | REF-003 Debug Log 提案 | — |
| 2026-04-02 | — | REF-004 風險評估 | — |
| 2026-05-25 | — | **整合 REF-001~004 為本檔**；原 4 份檔案已刪除 | Steven |

<!-- preserved-content:end -->
