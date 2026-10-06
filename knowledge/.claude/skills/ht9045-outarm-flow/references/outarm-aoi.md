# OutArm — AOI（自動光學檢測）詳細參考

舊引用路徑保留；[讀取整理後文件](../../hpi-outarm-flow/references/flow/references/outarm-aoi.md)。

## 1. AOI 在 OutArm 流程中的位置

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/01.md#1-aoi-在-outarm-流程中的位置)

### 觸發條件

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/01.md#觸發條件)

## 2. 四種 AOI 模式

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/02.md#2-四種-aoi-模式)

### Gerneral.ini 硬體開關

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/02.md#gerneralini-硬體開關)

### AOI 模式 enum

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/02.md#aoi-模式-enum)

## 3. DoAOIFunction() — 核心分派

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/03.md#3-doaoifunction--核心分派)

## 4. Scanner AOI — 底部掃描（最常見模式）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#4-scanner-aoi--底部掃描最常見模式)

### 4.1 DoScanAOIFunction()

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#41-doscanaoifunction)

### 4.2 DoScanAOIFunction_Inspection() — 單顆檢測

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#42-doscanaoifunction_inspection--單顆檢測)

### 4.3 RS232 通訊協定

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#43-rs232-通訊協定)

### 4.4 抽檢邏輯（Interval Counter）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#44-抽檢邏輯interval-counter)

### 4.5 OutArm 移動到 Scanner 位置

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/04.md#45-outarm-移動到-scanner-位置)

## 5. Top & Bottom Inspect（TCP/IP 模式）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#5-top--bottom-inspecttcpip-模式)

### 5.1 啟用條件

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#51-啟用條件)

### 5.2 流程概要

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#52-流程概要)

### 5.3 檢測子流程 DoTopBtmInspFunc_Inspection()

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#53-檢測子流程-dotopbtminspfunc_inspection)

### 5.4 Retry 機制

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#54-retry-機制)

### 5.5 專用馬達

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/05.md#55-專用馬達)

## 6. Vitrox BGA/PAD View（傳統 AOI）

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/06.md#6-vitrox-bgapad-view傳統-aoi)

### 6.1 DoBGAViewFunction()

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/06.md#61-dobgaviewfunction)

### 6.2 DoPADViewFunction()

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/06.md#62-dopadviewfunction)

### 6.3 Vitrox Fail Bin 分配

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/06.md#63-vitrox-fail-bin-分配)

## 7. AOI Fail → Bin 分配邏輯

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/07.md#7-aoi-fail--bin-分配邏輯)

### 7.1 GetAOIFailBin(iRow, iCol)

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/07.md#71-getaoifailbinirow-icol)

### 7.2 Bin 編號對照

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/07.md#72-bin-編號對照)

### 7.3 Fix 上下層映射

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/07.md#73-fix-上下層映射)

### 7.4 修改出料 Tray 的方式

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/07.md#74-修改出料-tray-的方式)

## 8. AOI.Data 工作檔設定

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#8-aoidata-工作檔設定)

### [SETTING] section — Scanner AOI 相關

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#setting-section--scanner-aoi-相關)

### [SETTING] section — Vitrox 相關

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#setting-section--vitrox-相關)

### [SETTING] section — 連續 Fail 告警

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#setting-section--連續-fail-告警)

### [SETTING] section — Ball Damage 計數器

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#setting-section--ball-damage-計數器)

### [RS232] section — Scanner AOI 串口

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#rs232-section--scanner-aoi-串口)

### [DutOnOff_BGAView] / [DutOnOff_PADView] sections

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/08.md#dutonoff_bgaview--dutonoff_padview-sections)

## 9. 連續 Fail 告警機制

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/09.md#9-連續-fail-告警機制)

### 9.1 告警類型

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/09.md#91-告警類型)

### 9.2 Arm Side 判定

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/09.md#92-arm-side-判定)

## 10. 相關全域變數

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/10.md#10-相關全域變數)

## 11. AOI 相關 Alarm Code

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/11.md#11-aoi-相關-alarm-code)

## 12. 相關函式索引

[讀取此節](../../hpi-outarm-flow/references/flow/references/outarm-aoi/12.md#12-相關函式索引)
