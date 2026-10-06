> 保存來源：`.claude/skills/ht9045-inarm-flow/references/HP_REF-001_iForPlaceHPX3Step_RaceCondition.md`，main `372b91908`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# REF-001：`iForPlaceHPX3Step` Race Condition（3x5 HP 1x2 Mode）

## 摘要

在 3x5 HP 1x2 Mode 配置中，`SearchPlacePlateXItem3_1x2Suck()` 函式因為使用吸嘴實時狀態判斷來決定 `iForPlaceHPX3Step`，導致多執行緒環境下出現 Race Condition。同一時刻，取料執行緒（InArmPickFromLoadTask）與放料執行緒（InArmPlaceToHotPlateTask）會競爭 `InArmSuck.Item[0][0]` 的狀態，造成 Col2 放料路徑錯誤。

## 詳細資訊

| 項目 | 內容 |
|------|------|
| **日期** | 2026-04-02 |
| **版本** | V3.33.900.0 |
| **機台配置** | 3x5 HP，1x2 Picker（Sucker Aa, Ab） |
| **症狀** | 模擬 log（`D:\HT9045_StateRecord\2026-04-02 11_22_15\`）出現 `InArm Suck[0,3] Pos(Y=..., X=...) Place to Hot Plate 1 [1,2]` 位置偏差警告 |

## 根本原因

`SearchPlacePlateXItem3_1x2Suck()` 第一次呼叫時使用下列條件判斷 Col2 是否需要使用 Ad 吸嘴：

```cpp
if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
{
    iForPlaceHPX3Step = 1;  // Col2 單顆放，使用 Ad（Sucker 3）
}
```

**Race Condition 時序：**

1. `InArmPickFromLoadTask` step 1 @ 11:22:08.249 → 完成取料，`Item[0][0]` 被新 IC 填入 `HAS_IC`
2. `InArmPlaceToHotPlateTask` step 1 @ 11:22:08.510（差距 261ms）→`SearchPlateToPlace()` 執行
3. 搜尋 Col2 時，檢查 `InArmSuck.Item[0][0]==NULL_IC` 判斷失敗
4. `iForPlaceHPX3Step` 錯誤維持以前的值（0）
5. 後續 `GetPlaceToHotPlateSuckCol()` 依 0 值取得 Aa 吸嘴，但實際應使用 Ad

**結果：** 位置偏差，ColX/Y 對應錯誤。

## 修改方案

**修改位置：** `ainarm_SearchPlacePlate.cpp`，函式 `SearchPlacePlateXItem3_1x2Suck()`，約 line 3079

**修改前：**
```cpp
if(x==2 && InArmSuck.Item[0][0]==NULL_IC)
{
    iForPlaceHPX3Step = 1;
}
```

**修改後：**
```cpp
if(x==2 && y%2==1)
{
    iForPlaceHPX3Step = 1;  // Col2 奇數 Row 永遠需要 Ad 放料
}
```

## 修改原理

根據 HT9045 HotPlate 設計，3x5 HP 的 Col2（第三欄）放料邏輯為：
- **偶數 Row**（0, 2, 4）：使用 Aa（Sucker 0）
- **奇數 Row**（1, 3）：使用 Ad（Sucker 3）

此設計**完全基於 Row 奇偶性**，與吸嘴手上是否有 IC 無關。因此改用 `y%2==1` 作為穩定條件，避免多執行緒污染。

## 備份

原檔案已備份至：`ainarm_SearchPlacePlate.cpp.bak_20260402`

## 受影響範圍

- **檔案**：`ainarm_SearchPlacePlate.cpp`
- **函式**：`SearchPlacePlateXItem3_1x2Suck()`
- **機台**：所有 3x5 HP + 1x2 Picker 配置

<!-- preserved-content:end -->
