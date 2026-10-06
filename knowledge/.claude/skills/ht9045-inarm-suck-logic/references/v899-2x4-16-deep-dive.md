# HT9045 V899 16-site InArm 吸取邏輯深度解析

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive.md)。

## 1. 範圍與關鍵檔案

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/01.md#1-範圍與關鍵檔案)

## 2. 三層架構總覽

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/02.md#2-三層架構總覽)

### 第 1 層：入口保護

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/02.md#第-1-層入口保護)

### 第 2 層：16-site 主狀態機

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/02.md#第-2-層16-site-主狀態機)

### 第 3 層：單顆吸嘴真空狀態機

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/02.md#第-3-層單顆吸嘴真空狀態機)

## 3. 16-site 吸嘴映射怎麼建立

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/03.md#3-16-site-吸嘴映射怎麼建立)

## 4. Loader 取料邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#4-loader-取料邏輯)

### 4.1 流程骨架

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#41-流程骨架)

### 4.2 真正吸取發生在 case 1000

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#42-真正吸取發生在-case-1000)

### 4.3 `AddLoadingCount()` 做了什麼

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#43-addloadingcount-做了什麼)

### 4.4 Retry / Alarm / 清料路徑

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#44-retry--alarm--清料路徑)

### 4.5 為什麼成功後還會再檢查掉料

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#45-為什麼成功後還會再檢查掉料)

### 4.6 `HAS_NULL_IC` 在 Loader 路徑的意義

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/04.md#46-has_null_ic-在-loader-路徑的意義)

## 5. 初始真空一致性檢查

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/05.md#5-初始真空一致性檢查)

## 6. HotPlate 取料邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#6-hotplate-取料邏輯)

### 6.1 先決定要吸哪一 team

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#61-先決定要吸哪一-team)

### 6.2 `bZFlgToHPPick[][]` 才是本輪 HP pick 的核心旗標

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#62-bzflgtohppick-才是本輪-hp-pick-的核心旗標)

### 6.3 真正的 HP 吸取發生在 `HotplateDataConversion()`

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#63-真正的-hp-吸取發生在-hotplatedataconversion)

### 6.4 `HotplateDataConversion()` 的關鍵分支

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#64-hotplatedataconversion-的關鍵分支)

#### 分支 A：HotPlate 該格已是 `HAS_NULL_IC`

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#分支-ahotplate-該格已是-has_null_ic)

#### 分支 B：本輪應該吸 (`bZFlgToHPPick == true`) 且 `Suck()` 成功

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#分支-b本輪應該吸-bzflgtohppick--true-且-suck-成功)

#### 分支 C：本輪不該吸或 team 裡沒有資料

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#分支-c本輪不該吸或-team-裡沒有資料)

### 6.5 HP pick error 如何收斂成 `JAM0109`

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#65-hp-pick-error-如何收斂成-jam0109)

### 6.6 取完一 team 後怎麼往下走

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/06.md#66-取完一-team-後怎麼往下走)

## 7. HotPlate 放料與 Data Swap error

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/07.md#7-hotplate-放料與-data-swap-error)

### Error 1

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/07.md#error-1)

### Error 2

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/07.md#error-2)

### 延伸判讀

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/07.md#延伸判讀)

## 8. `TMySucker::Suck()` 真空狀態機解讀

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/08.md#8-tmysuckersuck-真空狀態機解讀)

### 8.1 重要欄位

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/08.md#81-重要欄位)

### 8.2 實機模式流程

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/08.md#82-實機模式流程)

### 8.3 何時 `Error=true`

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/08.md#83-何時-errortrue)

### 8.4 `GetStatus()` 與 `Sensor()`

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/08.md#84-getstatus-與-sensor)

## 9. `HAS_NULL_IC` 的真正角色

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/09.md#9-has_null_ic-的真正角色)

## 10. Debug Checklist

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/10.md#10-debug-checklist)

## 11. 一句話總結

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-2x4-16-deep-dive/11.md#11-一句話總結)
