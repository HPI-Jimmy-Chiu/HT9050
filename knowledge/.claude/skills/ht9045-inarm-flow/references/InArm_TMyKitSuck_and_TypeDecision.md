# InArm 流程 — TMyKitSuck 吸嘴結構與 iInArmType 決策

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision.md)。

## 1. 機台模式起點：TestIF_File.iTestMode

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/01.md#1-機台模式起點testif_fileitestmode)

## 2. TMyKitSuck 吸嘴結構說明

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/02.md#2-tmykitsuck-吸嘴結構說明)

### 2.1 關鍵成員變數

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/02.md#21-關鍵成員變數)

### 2.1.1 X 軸吸嘴間距硬體設定

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/02.md#211-x-軸吸嘴間距硬體設定)

### 2.2 SetPickerCount 完整版

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/02.md#22-setpickercount-完整版)

### 2.3 全域 TMyKitSuck 實例

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/02.md#23-全域-tmykitsuck-實例)

## 3. iPickRow / iPickCol 與 iShtRow / iShtCol 的三種關係

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#3-ipickrow--ipickcol-與-ishtrow--ishtcol-的三種關係)

### 情況 A：Pick 與 Shuttle 放料尺寸相同（1:1）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#情況-apick-與-shuttle-放料尺寸相同11)

### 情況 B：Pick 行數是 Shuttle 的 2 倍（Hot 模式雙行吸嘴）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#情況-bpick-行數是-shuttle-的-2-倍hot-模式雙行吸嘴)

#### B-1：`e9045_1x2_4_Hot`（DualSite 2-Site Hot 模式）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#b-1e9045_1x2_4_hotdualsite-2-site-hot-模式)

#### B-2：`e9045_1x4_8_Hot`（QualSite1X4 Hot 模式）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#b-2e9045_1x4_8_hotqualsite1x4-hot-模式)

#### B-3：`e9045_2x2_8_Hot`（QualSite2X2 Hot 模式）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#b-3e9045_2x2_8_hotqualsite2x2-hot-模式)

### 情況 C：吸嘴 Pick 幾何＜Shuttle 格位（需多次 X 移動放完）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#情況-c吸嘴-pick-幾何shuttle-格位需多次-x-移動放完)

### 特殊情況

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#特殊情況)

#### `e9045_1x4_1_Ac`（1×4 機台僅開啟 Ac Site）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#e9045_1x4_1_ac14-機台僅開啟-ac-site)

#### `USE_PICKER_COUNT == ep1Picker`（1 Picker 模式）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/03.md#use_picker_count--ep1picker1-picker-模式)

## 4. DoInArm_9045_Type() — iInArmType 決策流程

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/04.md#4-doinarm_9045_type--iinarmtype-決策流程)

### 決策邏輯（精簡版）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/04.md#決策邏輯精簡版)

### iInArmType 決策輔助函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/04.md#iinarmtype-決策輔助函式)

### iXStep — Shuttle X-Pitch 補償次數

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/04.md#ixstep--shuttle-x-pitch-補償次數)

### Shuttle 路由：iWhichSht / iWhichKit

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/04.md#shuttle-路由iwhichsht--iwhichkit)

## 5. 關鍵：Shuttle 端如何使用 InArmSuck 參數

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/05.md#5-關鍵shuttle-端如何使用-inarmsuck-參數)

## 6. InArm 整體流程概覽

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/06.md#6-inarm-整體流程概覽)

### 6.1 常溫模式（`LastSet.iTemperature != Tempture_Hot`）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/06.md#61-常溫模式lastsetitemperature--tempture_hot)

### 6.2 加熱模式（`LastSet.iTemperature == Tempture_Hot`）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/06.md#62-加熱模式lastsetitemperature--tempture_hot)

### 6.3 main loop 關係

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/06.md#63-main-loop-關係)

## 7. 加熱模式詳細流程

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#7-加熱模式詳細流程)

### 7.1 放到 HotPlate（case 1100）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#71-放到-hotplatecase-1100)

### 7.2 PickFromHPList 資料結構

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#72-pickfromhplist-資料結構)

### 7.3 從 HotPlate 取料（case 1500）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#73-從-hotplate-取料case-1500)

### 7.4 加熱模式下 Shuttle 放料

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#74-加熱模式下-shuttle-放料)

### 7.5 Hot Mode 的 HP 空位判斷

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/07.md#75-hot-mode-的-hp-空位判斷)

## 8. iInArmType 與 SetPickerCount 對照表（常見型號）

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/08.md#8-iinarmtype-與-setpickercount-對照表常見型號)

## 9. Item 狀態碼快查

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/09.md#9-item-狀態碼快查)

## 10. 常見問題索引

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/10.md#10-常見問題索引)

## 相關知識庫連結

[讀取此節](../../hpi-inarm-flow/references/flow/references/InArm_TMyKitSuck_and_TypeDecision/11.md#相關知識庫連結)
