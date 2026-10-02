# REF-002：`GetPlaceToHotPlateCol()` Race Condition（3x5 HP 1x2 Mode）

## 摘要

在 3x5 HP 1x2 Mode 配置中，`GetPlaceToHotPlateCol()` 函式中 `bUseAxxGPicker` 區段同樣存在 `InArmSuck.Item[0][0]==NULL_IC` 的 Race Condition。多執行緒競爭造成 Col2 索引計算走錯分支，導致 AD 吸嘴攜帶的 IC 未被正確寫入 HotPlate Tray，後續放料試圖寫入同一格位時觸發重複 IC 偵測。

## 詳細資訊

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **機台配置** | 3x5 HP，1x2 Picker（`e9045_1x2_2_14` / `e9045_1x3_2_14`） |
| **症狀** | 模擬 log（`D:\HT9045_StateRecord\2026-04-02 14_14_05\`）出現 `HotPlate Data Swap error 1. Please call engineer`（14:13:54） |
| **等級** | 高：直接導致 IC 資料遺失、停機 |

## 根本原因

`GetPlaceToHotPlateCol()` 中 `bUseAxxGPicker` 區段在決定 Col2 放料時的 Col 索引 `ix` 時使用：

```cpp
if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
{
    ix = 2;  // Col2 直接放
}
else
{
    ix = iPlacePlateX[0]+j*(XDiv/2);  // 正常計算
}
```

**Race Condition 時序：**

1. `InArmPickFromLoadTask` step 1 @ 14:13:53.571 → 取料完成，`Item[0][0]` 填入 `HAS_IC`
2. `InArmPlaceToHotPlateTask` step 1 @ 14:13:53.601（差距 30ms）→ `SearchPlateToPlace()` 執行
3. 搜尋到 Col2，回傳 `iForPlaceHPX3Step` = 0 或 1
4. 進入 step 350（吹氣放料）@ 14:13:54.070，呼叫 `GetPlaceToHotPlateCol(j)`
5. 檢查 `InArmSuck.Item[0][0]==NULL_IC` 判斷失敗（此時已是 HAS_IC）
6. `ix` 計算走正常分支：`ix = iPlacePlateX[0]+j*(XDiv/2)`
7. 若 `iPlacePlateX[0]=2, j=1`，則 `ix = 2 + 1*1 = 3`（超出 XDiv=3 邊界）
8. Tray[3][y] 存取越界或跳過 → Ad 吸嘴的 IC **未被寫入** HP Tray

**後續後果：**

- 手臂上 Ad 吸嘴已吹氣空了，但 HP Tray 仍無此顆 IC 的記錄
- 下一趟放料再次掃描相同 Col2 格位
- 發現 `Tray[2][y].Data = HAS_IC`（前一趟遺留）
- `DoPlaceToHPSwapData()` 檢查發現衝突，觸發

  ```
  HotPlate Data Swap error 1. Please call engineer
  ```

## 修改方案

**修改位置：** `ainarm_SearchPlacePlate.cpp`，函式 `GetPlaceToHotPlateCol()`，`bUseAxxGPicker` 區段（`e9045_1x2_2_14` / `e9045_1x3_2_14`），約 line 969

**修改前：**
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

**修改後：**
```cpp
if(bUseAxxGPicker && x==2 && hPlateRowCount==2 && iPlaceHPOrder==0)
{
    if(x==2 && y%2==1)  // 奇數 Row 使用 Ad 放
    {
        ix = 2;
    }
    else
    {
        ix = iPlacePlateX[0]+j*(XDiv/2);
    }
}
```

## 修改原理

同 REF-001：移除對吸嘴實時狀態的依賴，改用設計層面的 Row 奇偶邏輯。此時 `y` 已由 `SearchPlacePlateXItem3_1x2Suck()` 確定，是穩定的設計常數。

## 受影響範圍

- **檔案**：`ainarm_SearchPlacePlate.cpp`
- **函式**：`GetPlaceToHotPlateCol()`
- **Picker 類型**：`bUseAxxGPicker`（`e9045_1x2_2_14`, `e9045_1x3_2_14`）
- **機台**：所有 3 Col HotPlate + 1x2/1x3 Picker 配置

## 相關參考

- [REF-001_iForPlaceHPX3Step_RaceCondition.md](REF-001_iForPlaceHPX3Step_RaceCondition.md)
- SKILL.md § 7：`SearchPlacePlateXItem3_1x2Suck()` — 3-Col HP 搜尋（1x2 Mode）
- SKILL.md § 9：`GetPlaceToHotPlateCol()` / `GetPlaceToHotPlateSuckCol()`
