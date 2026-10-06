# HotPlate Variable Pitch 完整參考表

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md)。

## 1. GetVariableInHotPlateData — 三函式 Pitch 公式對照

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#1-getvariableinhotplatedata--三函式-pitch-公式對照)

### 閾值常數

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#閾值常數)

### 1.1 GetVariableInHotPlateData_AxEx（bUseAxExPicker = true）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#11-getvariableinhotplatedata_axexbuseaxexpicker--true)

### 1.2 GetVariableInHotPlateData_AxxG（bUseAxxGPicker = true）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#12-getvariableinhotplatedata_axxgbuseaxxgpicker--true)

### 1.3 GetVariableInHotPlateData_ACEG（bUseACEGPicker = true）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#13-getvariableinhotplatedata_acegbuseacegpicker--true)

## 2. GetPlaceToHotPlateSuckCol — 吸嘴→陣列索引對應

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#2-getplacetohotplatesuckcol--吸嘴陣列索引對應)

## 3. GetPlaceToHotPlateCol — HP 格位→col 索引對應

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#3-getplacetohotplatecol--hp-格位col-索引對應)

### 3.1 AxEx 模式（bUseAxExPicker = true）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#31-axex-模式buseaxexpicker--true)

### 3.2 AxxG 模式（bUseAxxGPicker = true）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#32-axxg-模式buseaxxgpicker--true)

### 3.3 ACEG 模式（else, 4-picker）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#33-aceg-模式else-4-picker)

## 4. SearchPlacePlateXItem_2x2Suck — spacX / spacY 計算

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#4-searchplaceplatexitem_2x2suck--spacx--spacy-計算)

### 4.1 正常路徑（HotPlateYPitchCanPutAll = true, iPickRow=2）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#41-正常路徑hotplateypitchcanputall--true-ipickrow2)

### 4.2 bSpecialPlace 路徑（HotPlateYPitchCanPutAll = false 或 iPickRow=1）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#42-bspecialplace-路徑hotplateypitchcanputall--false-或-ipickrow1)

### 4.3 迭代邏輯（搜尋 next position）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#43-迭代邏輯搜尋-next-position)

## 5. CheckHotPlateHasSpace_9045_8_New_V — 空格檢查

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#5-checkhotplatehasspace_9045_8_new_v--空格檢查)

## 6. 常見 XDiv × XPitch 組合速查表

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#6-常見-xdiv--xpitch-組合速查表)

## 7. 交叉驗證清單（修改 HP 邏輯時必查）

[讀取此節](../../hpi-inarm-flow/references/flow/references/HP_VariablePitch_Reference.md#7-交叉驗證清單修改-hp-邏輯時必查)
