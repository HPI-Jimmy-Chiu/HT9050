# HP `Item[][]` NULL_IC Race Condition 家族（整合報告）

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md)。

## 0. 整合說明

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#0-整合說明)

## 1. 原始 REF 對照表

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#1-原始-ref-對照表)

## 2. 根本原因（Race Condition 共通機制）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#2-根本原因race-condition-共通機制)

### 2.1 反模式

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#21-反模式)

### 2.2 通用時序

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#22-通用時序)

### 2.3 適用機台 / 配置

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#23-適用機台--配置)

## 3. 修正案例 A：`SearchPlacePlateXItem3_1x2Suck()`

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#3-修正案例-asearchplaceplatexitem3_1x2suck)

### 修改前 ❌

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#修改前-)

### 修改後 ✅

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#修改後-)

### 備份

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#備份)

## 4. 修正案例 B：`GetPlaceToHotPlateCol()` `bUseAxxGPicker` 區段

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#4-修正案例-bgetplacetohotplatecol-buseaxxgpicker-區段)

### 失效後果鏈

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#失效後果鏈)

### 修改前 ❌

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#修改前-)

### 修改後 ✅

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#修改後-)

## 5. 修正原理：為何 `y%2==1` 安全？

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#5-修正原理為何-y21-安全)

## 6. Debug Log 提案（REF-003 移入）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#6-debug-log-提案ref-003-移入)

### 6.1 Proposal A：`SearchPlacePlateXItem3_1x2Suck()` 搜尋結果 Log

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#61-proposal-asearchplaceplatexitem3_1x2suck-搜尋結果-log)

### 6.2 Proposal B：`GetPlaceToHotPlateCol()` Col2 路徑 Log

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#62-proposal-bgetplacetohotplatecol-col2-路徑-log)

### 6.3 EventLog 觀察流程

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#63-eventlog-觀察流程)

## 7. 已識別但未驗證的同類風險點（REF-004 移入）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#7-已識別但未驗證的同類風險點ref-004-移入)

## 8. 統一驗證與長期防護建議

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#8-統一驗證與長期防護建議)

### 8.1 短期（現有版本）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#81-短期現有版本)

### 8.2 長期（下一版本）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#82-長期下一版本)

## 9. 相關檔案

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#9-相關檔案)

## 10. 變更歷史

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_Item_NULL_IC_RaceCondition.md#10-變更歷史)
