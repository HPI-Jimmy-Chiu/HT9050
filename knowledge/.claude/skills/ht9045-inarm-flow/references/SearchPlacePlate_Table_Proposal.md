# SearchPlacePlate 查表法改寫方案

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal.md)。

## 目錄

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/01.md#目錄)

## §0 現狀對照

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/02.md#0-現狀對照)

### 0.1 舊版: 16 個硬編碼函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/02.md#01-舊版-16-個硬編碼函式)

### 0.2 新版: 1 張表 + 1 個引擎

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/02.md#02-新版-1-張表--1-個引擎)

## §1 設計原則

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/03.md#1-設計原則)

## §2 資料結構

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/04.md#2-資料結構)

### 2.1 TPlaceSegment — 掃描段

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/04.md#21-tplacesegment--掃描段)

### 2.2 TPlaceEntry — 完整放置規則（表的一行）

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/04.md#22-tplaceentry--完整放置規則表的一行)

### 2.3 TStepHint — 放置偏移

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/04.md#23-tstephint--放置偏移)

## §3 完整查表 — PLACE_TABLE

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#3-完整查表--place_table)

### 3.1 armCategory 分類

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#31-armcategory-分類)

### 3.2 condition 分類

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#32-condition-分類)

### 3.3 完整查表 — 50 條規則

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#33-完整查表--50-條規則)

#### XDiv=2 (4 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv2-4-條)

#### XDiv=3 (5 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv3-5-條)

#### XDiv=4 (9 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv4-9-條)

#### XDiv=6 標準 HP (8 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv6-標準-hp-8-條)

#### XDiv=6 WideHP (4 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv6-widehp-4-條)

#### XDiv=8 (8 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv8-8-條)

#### XDiv=10 (2 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv10-2-條)

#### XDiv=12 (4 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv12-4-條)

#### XDiv=16 (2 條)

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#xdiv16-2-條)

### 3.4 表格總計

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/05.md#34-表格總計)

## §4 統一放置引擎

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/06.md#4-統一放置引擎)

### 4.1 入口函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/06.md#41-入口函式)

### 4.2 ExecutePlaceEntry — 統一掃描引擎

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/06.md#42-executeplaceentry--統一掃描引擎)

### 4.3 Col-first 掃描（XDiv=3 專用）

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/06.md#43-col-first-掃描xdiv3-專用)

## §5 StepHint 統一計算

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/07.md#5-stephint-統一計算)

### 5.1 stepHintRule 定義

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/07.md#51-stephintrule-定義)

### 5.2 擬碼

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/07.md#52-擬碼)

## §6 PickFromHPList 整合

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/08.md#6-pickfromhplist-整合)

### 6.1 設計要點

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/08.md#61-設計要點)

### 6.2 呼叫時機

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/08.md#62-呼叫時機)

### 6.3 Pick 端讀取

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/08.md#63-pick-端讀取)

## §7 Edge Case 統一處理

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#7-edge-case-統一處理)

### 7.1 以表項區分的 Edge Case

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#71-以表項區分的-edge-case)

### 7.2 b6x20HP 的處理

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#72-b6x20hp-的處理)

### 7.3 AutoClean 統一

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#73-autoclean-統一)

### 7.4 奇數 YDiv 末格保護

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#74-奇數-ydiv-末格保護)

### 7.5 PlateSelect 統一

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/09.md#75-plateselect-統一)

## §8 查表索引函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/10.md#8-查表索引函式)

### 8.1 armCategory 分類函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/10.md#81-armcategory-分類函式)

### 8.2 condition 分類函式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/10.md#82-condition-分類函式)

### 8.3 LookupPlaceEntry

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/10.md#83-lookupplaceentry)

## §9 檔案結構與行數預估

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/11.md#9-檔案結構與行數預估)

### 9.1 新增檔案

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/11.md#91-新增檔案)

### 9.2 行數預估

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/11.md#92-行數預估)

### 9.3 保留不動的檔案

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/11.md#93-保留不動的檔案)

## §10 相容性與風險

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/12.md#10-相容性與風險)

### 10.1 全域變數相容

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/12.md#101-全域變數相容)

### 10.2 PickFromHPList 相容

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/12.md#102-pickfromhplist-相容)

### 10.3 潛在行為差異

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/12.md#103-潛在行為差異)

### 10.4 風險評估

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/12.md#104-風險評估)

## §11 驗證方案

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/13.md#11-驗證方案)

### 11.1 Python 模擬驗證

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/13.md#111-python-模擬驗證)

### 11.2 驗證覆蓋率

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/13.md#112-驗證覆蓋率)

### 11.3 差異報告格式

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/13.md#113-差異報告格式)

## 附錄 A: PLACE_TABLE C++ 靜態陣列（完整版）

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/14.md#附錄-a-place_table-c-靜態陣列完整版)

## 附錄 B: 不支援的組合

[讀取此節](../../hpi-inarm-flow/references/flow/references/SearchPlacePlate_Table_Proposal/15.md#附錄-b-不支援的組合)
