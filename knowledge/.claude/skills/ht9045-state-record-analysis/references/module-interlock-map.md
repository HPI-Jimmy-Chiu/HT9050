# 模組間安全互鎖 / 依賴關係

舊引用路徑保留；[讀取整理後文件](../../hpi-state-analysis/references/source/references/module-interlock-map.md)。

## 符號說明

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#符號說明)

## 1. Shuttle ↔ Index（Z 軸安全互鎖）

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#1-shuttle--indexz-軸安全互鎖)

### SHT2 state 210 → TestZ2 安全高度

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#sht2-state-210--testz2-安全高度)

### SHT1 state 210 → TestZ1 安全高度

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#sht1-state-210--testz1-安全高度)

## 2. InArm → Shuttle 位置

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#2-inarm--shuttle-位置)

### InArm state 2000 → SHT2 在左側

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#inarm-state-2000--sht2-在左側)

### InArm state 2000 → SHT1 在左側（對稱）

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#inarm-state-2000--sht1-在左側對稱)

## 3. OutArm → Shuttle 位置 + IC 資料

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#3-outarm--shuttle-位置--ic-資料)

### OutArm state 1100 → SHT1 IC 就緒

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#outarm-state-1100--sht1-ic-就緒)

### OutArm pick 子任務完成 ≠ OutArm 已持料

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#outarm-pick-子任務完成--outarm-已持料)

## 4. DoTestHeadMotor → 系統旗標（門控）

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#4-dotestheadmotor--系統旗標門控)

### DoTestHeadMotor 進入條件

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#dotestheadmotor-進入條件)

### SoftStop / SoftStart 生命週期

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#softstop--softstart-生命週期)

### ShowErrorMessage 重置行為

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#showerrormessage-重置行為)

## 5. bEcho 生命週期

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#5-becho-生命週期)

## 6. bShuttle2Pause（D42 專用）

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#6-bshuttle2paused42-專用)

## 7. 依賴圖（文字版）

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#7-依賴圖文字版)

## 待補充互鎖

[讀取此節](../../hpi-state-analysis/references/source/references/module-interlock-map.md#待補充互鎖)
