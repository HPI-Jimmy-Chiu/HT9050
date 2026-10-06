# REF-005：Step 400 HasIC() 統一修正（3x5 HP Col2 IC 遺失）

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md)。

## 症狀

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#症狀)

## 根因分析

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#根因分析)

### Step 400 使用 `GetPlaceToHotPlateSuckCol(j)` 迴圈檢查殘留 IC

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#step-400-使用-getplacetohotplatesuckcolj-迴圈檢查殘留-ic)

### Bug 機制

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#bug-機制)

### 時序重建

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#時序重建)

## 修正方案

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#修正方案)

### 改用 `InArmSuck.HasIC()` 取代 `GetPlaceToHotPlateSuckCol` 迴圈

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#改用-inarmsuckhasic-取代-getplacetohotplatesuckcol-迴圈)

### 選擇 `HasIC()` 的理由

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#選擇-hasic-的理由)

## 修改檔案清單

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#修改檔案清單)

## 全模式 Step 400 調查結果

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#全模式-step-400-調查結果)

## 修正工具

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#修正工具)

## 關聯項目

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_REF-005_Step400_HasIC_Fix.md#關聯項目)
