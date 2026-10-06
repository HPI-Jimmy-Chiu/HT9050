# HT9045 OutArm Flow Knowledge

按需要選取以下章節，原文依順序保留。

- [HT9045 OutArm Flow Knowledge](original-entry/00.md)
- [適用場景](original-entry/01.md)
- [吸嘴物理排列（8-Sucker 模式）](original-entry/02.md)
- [OutArm 基準軸（與 InArm 鏡像對稱）](original-entry/03.md)
- [關鍵原始檔](original-entry/04.md)
- [呼叫階層總覽](original-entry/05.md)
- [取料段（§1–§5）— Shuttle Pick](original-entry/06.md)
- [附加功能段 — Rotator → AOI → Fix AI CCD](original-entry/07.md)
- [放料段（§6–§9）— Place to Tray](original-entry/08.md)
- [Fix Tray Full 換盤流程（§Fix）](original-entry/09.md)
- [Bin 分類 / CleanOut（§10–§13）](original-entry/10.md)
- [Local Preserved Notes (from previous local agent)](original-entry/11.md)
- [????](original-entry/12.md)
- [?????](original-entry/13.md)
- [??????](original-entry/14.md)
- [????�1?�5?? Shuttle Pick](original-entry/15.md)
- [AOI ???????�AOI?](original-entry/16.md)
- [????�6��9?Place to Tray](original-entry/17.md)
- [Bin ?? / CleanOut?�10��13?](original-entry/18.md)
- [合併補充：repo 既有參考（20261001）](original-entry/19.md)

# HT9045 OutArm Flow Knowledge

[讀取此節](original-entry/00.md#ht9045-outarm-flow-knowledge)

## 適用場景

[讀取此節](original-entry/01.md#適用場景)

## 吸嘴物理排列（8-Sucker 模式）

[讀取此節](original-entry/02.md#吸嘴物理排列8-sucker-模式)

## OutArm 基準軸（與 InArm 鏡像對稱）

[讀取此節](original-entry/03.md#outarm-基準軸與-inarm-鏡像對稱)

### 依 Y-Pitch 模式對照

[讀取此節](original-entry/03.md#依-y-pitch-模式對照)

### 物理意義

[讀取此節](original-entry/03.md#物理意義)

### OutArm 取放料公式（與 InArm 公式平行）

[讀取此節](original-entry/03.md#outarm-取放料公式與-inarm-公式平行)

## 關鍵原始檔

[讀取此節](original-entry/04.md#關鍵原始檔)

## 呼叫階層總覽

[讀取此節](original-entry/05.md#呼叫階層總覽)

## 取料段（§1–§5）— Shuttle Pick

[讀取此節](original-entry/06.md#取料段15-shuttle-pick)

### 重要摘要

[讀取此節](original-entry/06.md#重要摘要)

## 附加功能段 — Rotator → AOI → Fix AI CCD

[讀取此節](original-entry/07.md#附加功能段--rotator--aoi--fix-ai-ccd)

## 放料段（§6–§9）— Place to Tray

[讀取此節](original-entry/08.md#放料段69-place-to-tray)

### 重要摘要

[讀取此節](original-entry/08.md#重要摘要)

## Fix Tray Full 換盤流程（§Fix）

[讀取此節](original-entry/09.md#fix-tray-full-換盤流程fix)

### case 4000 的三條觸發路徑

[讀取此節](original-entry/09.md#case-4000-的三條觸發路徑)

### `VerifyFixTrayLink()` 回傳值說明（`aoutarm9045.cpp` L1595）

[讀取此節](original-entry/09.md#verifyfixtraylink-回傳值說明aoutarm9045cpp-l1595)

### `MoveOutArmXY_ToFix_Tray_Full(bool bMoveY=false)` 移動目標（`aoutarm.cpp`，904.5/906.2 約 L286/L311）

[讀取此節](original-entry/09.md#moveoutarmxy_tofix_tray_fullbool-bmoveyfalse-移動目標aoutarmcpp90459062-約-l286l311)

### `[E90] bE90_OutArmFixFullExtraY`（退讓量加大；不是補盤的解）

[讀取此節](original-entry/09.md#e90-be90_outarmfixfullextray退讓量加大不是補盤的解)

### 補盤退讓位置（每次換盤的避讓）— 客戶常問「OutArm 退到哪？」

[讀取此節](original-entry/09.md#補盤退讓位置每次換盤的避讓-客戶常問outarm-退到哪)

### 兩個放行 TrayArm 的安全互鎖函式 — `IsOutArmSafe()` vs `IsCatchTrayReadySupplyNewTray()`

[讀取此節](original-entry/09.md#兩個放行-trayarm-的安全互鎖函式--isoutarmsafe-vs-iscatchtrayreadysupplynewtray)

### `IsOutArmSafe()` 的 −3500 容差到底在補什麼 — 撞機根因 + 修正

[讀取此節](original-entry/09.md#isoutarmsafe-的-3500-容差到底在補什麼--撞機根因--修正)

#### ✅ 採用解（已上線）：保守離散 margin，目標「不撞」

[讀取此節](original-entry/09.md#-採用解已上線保守離散-margin目標不撞)

#### 🔧 進階／升級選項（精準解，暫不採用）

[讀取此節](original-entry/09.md#-進階升級選項精準解暫不採用)

#### 📌 第二案鑑識（2026-07-10，ATK HT-9132 PPLS2160）：客戶更新後仍撞 → 機上 exe 沒帶到修正

[讀取此節](original-entry/09.md#-第二案鑑識2026-07-10atk-ht-9132-ppls2160客戶更新後仍撞--機上-exe-沒帶到修正)

### 換盤流程 case 序列

[讀取此節](original-entry/09.md#換盤流程-case-序列)

### 覆蓋率（V3.33.901.0）

[讀取此節](original-entry/09.md#覆蓋率v3339010)

## Bin 分類 / CleanOut（§10–§13）

[讀取此節](original-entry/10.md#bin-分類--cleanout1013)

### 重要摘要

[讀取此節](original-entry/10.md#重要摘要)

## Local Preserved Notes (from previous local agent)

[讀取此節](original-entry/11.md#local-preserved-notes-from-previous-local-agent)

## ????

[讀取此節](original-entry/12.md#)

## ?????

[讀取此節](original-entry/13.md#)

## ??????

[讀取此節](original-entry/14.md#)

## ????�1?�5?? Shuttle Pick

[讀取此節](original-entry/15.md#15-shuttle-pick)

### ????

[讀取此節](original-entry/15.md#)

## AOI ???????�AOI?

[讀取此節](original-entry/16.md#aoi-aoi)

### ????

[讀取此節](original-entry/16.md#)

### 4 ? AOI ??

[讀取此節](original-entry/16.md#4--aoi-)

### AOI Fail ? Bin ??

[讀取此節](original-entry/16.md#aoi-fail--bin-)

### ????

[讀取此節](original-entry/16.md#)

### ???

[讀取此節](original-entry/16.md#)

## ????�6��9?Place to Tray

[讀取此節](original-entry/17.md#69place-to-tray)

### ????

[讀取此節](original-entry/17.md#)

## Bin ?? / CleanOut?�10��13?

[讀取此節](original-entry/18.md#bin---cleanout1013)

### ????

[讀取此節](original-entry/18.md#)

## 合併補充：repo 既有參考（20261001）

[讀取此節](original-entry/19.md#合併補充repo-既有參考20261001)
