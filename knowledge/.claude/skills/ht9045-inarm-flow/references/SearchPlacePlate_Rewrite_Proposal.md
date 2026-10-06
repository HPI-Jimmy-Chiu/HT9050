# ainarm_SearchPlacePlate_new.cpp 重寫方案

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal.md)。

## 0. 現狀問題分析

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/01.md#0-現狀問題分析)

### 0.1 舊版架構

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/01.md#01-舊版架構)

### 0.2 主要痛點

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/01.md#02-主要痛點)

## 1. 新版架構總覽

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/02.md#1-新版架構總覽)

### 1.1 設計原則

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/02.md#11-設計原則)

## 2. 資料結構定義

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/03.md#2-資料結構定義)

### 2.1 TScanSegment — 掃描段

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/03.md#21-tscansegment--掃描段)

### 2.2 TScanPlan — 完整掃描計畫

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/03.md#22-tscanplan--完整掃描計畫)

### 2.3 TStepHint — 放置偏移提示（取代 iForPlaceHPXnStep）

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/03.md#23-tstephint--放置偏移提示取代-iforplacehpxnstep)

## 3. 掃描計畫查表 — 完整 §19 × iInArmType 映射

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#3-掃描計畫查表--完整-19--iinarmtype-映射)

### 3.1 查表函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#31-查表函式)

### 3.2 掃描參數表

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#32-掃描參數表)

#### 3.2.1 XDiv=2 (2×6, 2×7 HP)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#321-xdiv2-26-27-hp)

#### 3.2.2 XDiv=3 (3×5, 3×7 HP — 目前無 PlateForm，需新增)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#322-xdiv3-35-37-hp--目前無-plateform需新增)

#### 3.2.3 XDiv=4 (4×8 HP, Pitch 40×40)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#323-xdiv4-48-hp-pitch-4040)

#### 3.2.4 XDiv=6 標準 HP (6×11, Pitch 26.67×30)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#324-xdiv6-標準-hp-611-pitch-266730)

#### 3.2.5 XDiv=6 WideHP (6×11/6×16, Pitch 26.67×30/20)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#325-xdiv6-widehp-611616-pitch-26673020)

#### 3.2.6 XDiv=8 (8×12/8×16/8×18/8×24 HP)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#326-xdiv8-812816818824-hp)

#### 3.2.7 XDiv=10 (10×16 HP, ATK 專用)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#327-xdiv10-1016-hp-atk-專用)

#### 3.2.8 XDiv=12 (12×18/12×24 HP, Type B 大板)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#328-xdiv12-12181224-hp-type-b-大板)

#### 3.2.9 XDiv=16 (16×24 HP, 32-Site 專用)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/04.md#329-xdiv16-1624-hp-32-site-專用)

## 4. 統一掃描引擎 — 核心演算法

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/05.md#4-統一掃描引擎--核心演算法)

### 4.1 擬碼 (Pseudocode)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/05.md#41-擬碼-pseudocode)

### 4.2 ExecuteScanPlan — 統一掃描引擎

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/05.md#42-executescanplan--統一掃描引擎)

### 4.3 ComputeStepHint — 統一偏移計算

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/05.md#43-computestephint--統一偏移計算)

## 5. BuildScanPlan 查表實作

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/06.md#5-buildscanplan-查表實作)

### 5.1 查表結構

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/06.md#51-查表結構)

### 5.2 Picker Mode 分類函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/06.md#52-picker-mode-分類函式)

## 6. Edge Case 統一處理

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/07.md#6-edge-case-統一處理)

### 6.1 統一到 BuildScanPlan 的修正

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/07.md#61-統一到-buildscanplan-的修正)

### 6.2 AutoClean 統一

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/07.md#62-autoclean-統一)

### 6.3 PlateSelect 統一

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/07.md#63-plateselect-統一)

## 7. 檔案結構規劃

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/08.md#7-檔案結構規劃)

### 7.1 新檔案行數預估

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/08.md#71-新檔案行數預估)

## 8. 相容性與風險

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/09.md#8-相容性與風險)

### 8.1 iForPlaceHPXnStep 相容

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/09.md#81-iforplacehpxnstep-相容)

### 8.2 MoveInArmXYToHotPlatePlace 不需修改

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/09.md#82-moveinarmxytohotplateplace-不需修改)

### 8.3 潛在行為差異

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/09.md#83-潛在行為差異)

### 8.4 風險評估

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/09.md#84-風險評估)

## 9. 驗證方案

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/10.md#9-驗證方案)

### 9.1 Python 模擬驗證

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/10.md#91-python-模擬驗證)

### 9.2 比對矩陣 (162 組合)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/10.md#92-比對矩陣-162-組合)

## 10. 實作步驟

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/11.md#10-實作步驟)

## 11. 開放問題（需與你確認）

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Rewrite_Proposal/12.md#11-開放問題需與你確認)
