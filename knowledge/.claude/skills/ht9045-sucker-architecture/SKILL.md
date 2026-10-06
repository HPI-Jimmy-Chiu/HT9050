---
name: ht9045-sucker-architecture
description: HT-9xxx 系列（HT9045 / HT9046 / HT9011UC，InArm/OutArm 各 8 吸嘴）吸嘴模組軟體架構知識庫（Steven Chou 2023.07 軟體重構）。當使用者詢問 TMySucker / TMyKitSuck 結構成員、iPickRow / iPickCol / iShtRow / iShtCol / iPickStep / iPickKitStep / iShtKitStep / iXStep / iYStep / iModeX 意義、In/Out Suck 預先定義（A~H 對應 Item[0..1][0..3]）、實體位置 vs 迴圈位置、馬達數量與吸嘴數量為何分開指定、為什麼要全模式共用 Function、Loader 共用兩個 Function 設計（MoveArmXYToLoaderStage / DoMoveArmXYToLoaderStage / GetInArmToLoaderPosition / GetInArmToLoaderPosition_Single）、HotPlate 縮 Pitch 數學模型（iBaseXToHP / iHPBasePos / iPickStep1Pos / HP.XStart / HP.XPitch）、Shuttle 的 GetNowSiteKitMode Mode 編碼規則（Mode/100 = X、Mode%100 = Y）、CheckXYPitch Teaching 計算（X Teaching → E to Center → A to G Pitch → Site Aa/Ac 位置）、Auto Clean 在 Shuttle Mode 的特殊處理、IO_Table.csv / Mot_Table.csv 表格驅動的硬體選配、**基準軸 iInArmXBase/iInArmYBase 與 iOutArmXBase/iOutArmYBase 設定（依 USE_IN_OUT_ARM_Y_PITCH 切換）**、**InArm↔OutArm 基準軸鏡像對稱關係（固定模式同 col 2、可變模式 InArm=F[1,2]/OutArm=D[1,1]）**、**Y-Pitch Modular 5 種模式（iXPitch60 / iXPitchManual635 / iXYPitchVariable / iXPitchManual360 / iXYPitchRowA）**、**Tech 結構 Teach 點位（iInArmLoadStageX/Y/Z2、iInArmZHeightSub[2][4]）**、**Loader 取料絕對位置公式（基準位置 + iCol×Tray.iXPitch + iInArmXBase×dInArmXPitch_1Step）**、**Eastsun 20251231 修正（個別 Offset 移到 GetInArmToLoaderPosition 才加）** 等問題時，應先載入此技能。觸發關鍵字：TMySucker, TMyKitSuck, iPickRow, iPickCol, iShtRow, iShtCol, iPickStep, iPickKitStep, iShtKitStep, iXStep, iYStep, iModeX, GetNowSiteKitMode, CheckXYPitch, iBaseXToHP, iHPBasePos, iPickStep1Pos, MoveArmXYToLoaderStage, DoMoveArmXYToLoaderStage, GetInArmToLoaderPosition, GetInArmToLoaderPosition_Single, IO_Table.csv, Mot_Table.csv, 邏輯機台, Steven 軟體重構, 吸嘴拓樸, 吸嘴模組, A~H 吸嘴, 全模式共用, 縮 Pitch, Teaching 計算, iInArmXBase, iInArmYBase, iOutArmXBase, iOutArmYBase, USE_IN_OUT_ARM_Y_PITCH, USE_OUT_ARM_Y_PITCH, iXPitch60, iXYPitchVariable, iXYPitchRowA, iXPitchManual635, dInArmXPitch_1Step, dInArmXPitch_MovePitch, InArmClose_PitchX, iInArmZHeightSub, Tech.iInArmLoadStageX, XInArm_Tray_Pick, InArmOffSet, OutArmOffSet, GetArmX, GetArmY, Eastsun 20251231, 基準軸, 鏡像對稱, BASE_X_TO_HP。
---

# ht9045-sucker-architecture 相容入口

同主題已整合到 [hpi-io-control](../hpi-io-control/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-io-control/references/vacuum/original-entry.md)

## 0. 與現有技能的範圍切分（避免內容重複）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/01.md#0-與現有技能的範圍切分避免內容重複)

## 1. 適用場景

[讀取此節](../hpi-io-control/references/vacuum/original-entry/02.md#1-適用場景)

## 2. 表格驅動的硬體選配

[讀取此節](../hpi-io-control/references/vacuum/original-entry/03.md#2-表格驅動的硬體選配)

### IO_Table.csv

[讀取此節](../hpi-io-control/references/vacuum/original-entry/03.md#io_tablecsv)

### Mot_Table.csv

[讀取此節](../hpi-io-control/references/vacuum/original-entry/03.md#mot_tablecsv)

## 3. `TMySucker` — 單一吸嘴類別

[讀取此節](../hpi-io-control/references/vacuum/original-entry/04.md#3-tmysucker--單一吸嘴類別)

## 4. `TMyKitSuck` — 一組吸嘴集合（核心）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/05.md#4-tmykitsuck--一組吸嘴集合核心)

### 4.1 為什麼要把「馬達數」和「吸嘴數」分開？

[讀取此節](../hpi-io-control/references/vacuum/original-entry/05.md#41-為什麼要把馬達數和吸嘴數分開)

### 4.2 預先定義 In/Out Suck — HT-9xxx 拓樸

[讀取此節](../hpi-io-control/references/vacuum/original-entry/05.md#42-預先定義-inout-suck--ht-9xxx-拓樸)

## 4.3 基準軸（`iInArmXBase` / `iInArmYBase`）— 取放料計算原點

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#43-基準軸iinarmxbase--iinarmybase-取放料計算原點)

### 4.3.1 Y-Pitch Modular 模式（`USE_IN_OUT_ARM_Y_PITCH`）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#431-y-pitch-modular-模式use_in_out_arm_y_pitch)

### 4.3.2 InArm 基準軸對照（依 Y-Pitch 模式）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#432-inarm-基準軸對照依-y-pitch-模式)

### 4.3.3 OutArm 基準軸 — 與 InArm 鏡像對稱

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#433-outarm-基準軸--與-inarm-鏡像對稱)

### 4.3.4 視覺對照（HT-9xxx，2x4 = 8 吸嘴）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#434-視覺對照ht-9xxx2x4--8-吸嘴)

### 4.3.5 為什麼可變模式要把基準改到 Row 1

[讀取此節](../hpi-io-control/references/vacuum/original-entry/06.md#435-為什麼可變模式要把基準改到-row-1)

## 4.4 Teach 點位結構 — Setup 頁面只 Teach 基準軸

[讀取此節](../hpi-io-control/references/vacuum/original-entry/07.md#44-teach-點位結構--setup-頁面只-teach-基準軸)

### 4.4.1 Tech 結構欄位（`LastSet.h`）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/07.md#441-tech-結構欄位lastseth)

### 4.4.2 Z 高度差陣列示意

[讀取此節](../hpi-io-control/references/vacuum/original-entry/07.md#442-z-高度差陣列示意)

## 4.5 取放料公式 — Loader / HP / Shuttle

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#45-取放料公式--loader--hp--shuttle)

### 4.5.1 初始化：推算所有吸嘴的絕對座標（`cinitial.cpp` L8637+）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#451-初始化推算所有吸嘴的絕對座標cinitialcpp-l8637)

### 4.5.2 Loader 取料 — 多吸嘴版（`ainarm9045.cpp` L4660）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#452-loader-取料--多吸嘴版ainarm9045cpp-l4660)

### 4.5.3 Loader 取料 — 單吸嘴 Fix 模式（`ainarm9045.cpp` L4704）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#453-loader-取料--單吸嘴-fix-模式ainarm9045cpp-l4704)

### 4.5.4 X-Pitch 單步距離（`ainarm9045.cpp` L4476-4610）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#454-x-pitch-單步距離ainarm9045cpp-l4476-4610)

### 4.5.5 OutArm 對稱版

[讀取此節](../hpi-io-control/references/vacuum/original-entry/08.md#455-outarm-對稱版)

## 5. 核心設計哲學 — 全模式共用 Function

[讀取此節](../hpi-io-control/references/vacuum/original-entry/09.md#5-核心設計哲學--全模式共用-function)

### 5.1 Loader 操作（`ainarm9045.cpp`）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/09.md#51-loader-操作ainarm9045cpp)

### 5.2 迴圈統一寫法

[讀取此節](../hpi-io-control/references/vacuum/original-entry/09.md#52-迴圈統一寫法)

## 6. HotPlate（加熱盤）操作

[讀取此節](../hpi-io-control/references/vacuum/original-entry/10.md#6-hotplate加熱盤操作)

### 6.1 縮 Pitch 數學模型（Slide 23–26）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/10.md#61-縮-pitch-數學模型slide-2326)

### 6.2 放料 / 吸料流程（Slide 27–32）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/10.md#62-放料--吸料流程slide-2732)

## 7. Shuttle 操作

[讀取此節](../hpi-io-control/references/vacuum/original-entry/11.md#7-shuttle-操作)

### 7.1 `GetNowSiteKitMode` — Mode 編碼規則（Slide 33–34）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/11.md#71-getnowsitekitmode--mode-編碼規則slide-3334)

### 7.2 Auto Clean 模式下的特殊處理（Slide 34）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/11.md#72-auto-clean-模式下的特殊處理slide-34)

### 7.3 `CheckXYPitch` — Teaching 計算（Slide 35–36）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/11.md#73-checkxypitch--teaching-計算slide-3536)

## 8. 關鍵 Source 位置（V3.33.904.2 基準）

[讀取此節](../hpi-io-control/references/vacuum/original-entry/12.md#8-關鍵-source-位置v3339042-基準)

## 9. 常見誤區

[讀取此節](../hpi-io-control/references/vacuum/original-entry/13.md#9-常見誤區)

## 10. 參考文件

[讀取此節](../hpi-io-control/references/vacuum/original-entry/14.md#10-參考文件)
