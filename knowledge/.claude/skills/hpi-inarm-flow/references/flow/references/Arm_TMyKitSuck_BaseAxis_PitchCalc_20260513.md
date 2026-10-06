# InArm / OutArm — TMyKitSuck 吸嘴結構、類型決策、基準軸與 Pitch 計算

按需要選取以下章節，原文依順序保留。

- [InArm / OutArm — TMyKitSuck 吸嘴結構、類型決策、基準軸與 Pitch 計算](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/00.md)
- [v2→v3 重大補充摘要](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/01.md)
- [1. 機台模式起點：TestIF_File.iTestMode](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/02.md)
- [2. TMyKitSuck 吸嘴結構說明](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md)
- [3. iPickRow / iPickCol 與 iShtRow / iShtCol 的三種關係](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md)
- [4. DoInArm_9045_Type() — iInArmType 決策流程](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md)
- [5. 關鍵：Shuttle 端如何使用 InArmSuck 參數](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/06.md)
- [6. InArm 整體流程概覽](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/07.md)
- [7. 加熱模式詳細流程](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md)
- [8. iInArmType 與 SetPickerCount 對照表（常見型號）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/09.md)
- [9. Item 狀態碼快查](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/10.md)
- [10. 常見問題索引](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/11.md)
- [11. Y-Pitch Modular 模式定義（`USE_IN_OUT_ARM_Y_PITCH`）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/12.md)
- [12. 基準軸 — 「全部 Teach 點位的參考原點」](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md)
- [13. Teach 點位結構（`LastSet.h` Tech 結構 → `Prod` 全域）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/14.md)
- [14. Loader / HP / Shuttle 取放料 X/Y 公式](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/15.md)
- [15. HotPlate 縮 Pitch 數學模型（PPT Slide 23-26）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/16.md)
- [16. X-Pitch 馬達與「單步距離」（`ainarm9045.cpp` L4476-4610）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/17.md)
- [17. OutArm 對稱版（鏡像） — 取放料公式參考](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/18.md)
- [18. v1 → v2 修正記錄（缺漏與錯誤補充，v2 版保留）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/19.md)
- [19. OutArm 類型路由 — `DoOutArm_9045()` Dispatch（`aoutarm9045.cpp` L540）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md)
- [20. InArm / OutArm SetPickerCount 完整對照表](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/21.md)
- [21. 本次技能更新摘要（2026-05-13）](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md)
- [相關知識庫連結](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/23.md)

# InArm / OutArm — TMyKitSuck 吸嘴結構、類型決策、基準軸與 Pitch 計算

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/00.md#inarm--outarm--tmykitsuck-吸嘴結構類型決策基準軸與-pitch-計算)

## v2→v3 重大補充摘要

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/01.md#v2v3-重大補充摘要)

## 1. 機台模式起點：TestIF_File.iTestMode

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/02.md#1-機台模式起點testif_fileitestmode)

## 2. TMyKitSuck 吸嘴結構說明

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md#2-tmykitsuck-吸嘴結構說明)

### 2.1 關鍵成員變數

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md#21-關鍵成員變數)

### 2.1.1 X 軸吸嘴間距硬體設定

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md#211-x-軸吸嘴間距硬體設定)

### 2.2 SetPickerCount 完整版

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md#22-setpickercount-完整版)

### 2.3 全域 TMyKitSuck 實例

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/03.md#23-全域-tmykitsuck-實例)

## 3. iPickRow / iPickCol 與 iShtRow / iShtCol 的三種關係

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#3-ipickrow--ipickcol-與-ishtrow--ishtcol-的三種關係)

### 情況 A：Pick 與 Shuttle 放料尺寸相同（1:1）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#情況-apick-與-shuttle-放料尺寸相同11)

### 情況 B：Pick 行數是 Shuttle 的 2 倍（Hot 模式雙行吸嘴）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#情況-bpick-行數是-shuttle-的-2-倍hot-模式雙行吸嘴)

#### B-1：`e9045_1x2_4_Hot`（DualSite 2-Site Hot 模式）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#b-1e9045_1x2_4_hotdualsite-2-site-hot-模式)

#### B-2：`e9045_1x4_8_Hot`（QualSite1X4 Hot 模式）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#b-2e9045_1x4_8_hotqualsite1x4-hot-模式)

#### B-3：`e9045_2x2_8_Hot`（QualSite2X2 Hot 模式）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#b-3e9045_2x2_8_hotqualsite2x2-hot-模式)

### 情況 C：吸嘴 Pick 幾何＜Shuttle 格位（需多次 X 移動放完）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#情況-c吸嘴-pick-幾何shuttle-格位需多次-x-移動放完)

### 特殊情況

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#特殊情況)

#### `e9045_1x4_1_Ac`（1×4 機台僅開啟 Ac Site）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#e9045_1x4_1_ac14-機台僅開啟-ac-site)

#### `USE_PICKER_COUNT == ep1Picker`（1 Picker 模式）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/04.md#use_picker_count--ep1picker1-picker-模式)

## 4. DoInArm_9045_Type() — iInArmType 決策流程

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md#4-doinarm_9045_type--iinarmtype-決策流程)

### 決策邏輯（精簡版）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md#決策邏輯精簡版)

### iInArmType 決策輔助函式

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md#iinarmtype-決策輔助函式)

### iXStep — Shuttle X-Pitch 補償次數

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md#ixstep--shuttle-x-pitch-補償次數)

### Shuttle 路由：iWhichSht / iWhichKit

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/05.md#shuttle-路由iwhichsht--iwhichkit)

## 5. 關鍵：Shuttle 端如何使用 InArmSuck 參數

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/06.md#5-關鍵shuttle-端如何使用-inarmsuck-參數)

## 6. InArm 整體流程概覽

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/07.md#6-inarm-整體流程概覽)

### 6.1 常溫模式（`LastSet.iTemperature != Tempture_Hot`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/07.md#61-常溫模式lastsetitemperature--tempture_hot)

### 6.2 加熱模式（`LastSet.iTemperature == Tempture_Hot`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/07.md#62-加熱模式lastsetitemperature--tempture_hot)

### 6.3 main loop 關係

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/07.md#63-main-loop-關係)

## 7. 加熱模式詳細流程

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#7-加熱模式詳細流程)

### 7.1 放到 HotPlate（case 1100）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#71-放到-hotplatecase-1100)

### 7.2 PickFromHPList 資料結構

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#72-pickfromhplist-資料結構)

### 7.3 從 HotPlate 取料（case 1500）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#73-從-hotplate-取料case-1500)

### 7.4 加熱模式下 Shuttle 放料

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#74-加熱模式下-shuttle-放料)

### 7.5 Hot Mode 的 HP 空位判斷

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/08.md#75-hot-mode-的-hp-空位判斷)

## 8. iInArmType 與 SetPickerCount 對照表（常見型號）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/09.md#8-iinarmtype-與-setpickercount-對照表常見型號)

## 9. Item 狀態碼快查

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/10.md#9-item-狀態碼快查)

## 10. 常見問題索引

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/11.md#10-常見問題索引)

## 11. Y-Pitch Modular 模式定義（`USE_IN_OUT_ARM_Y_PITCH`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/12.md#11-y-pitch-modular-模式定義use_in_out_arm_y_pitch)

## 12. 基準軸 — 「全部 Teach 點位的參考原點」

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md#12-基準軸--全部-teach-點位的參考原點)

### 12.1 基準軸概念

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md#121-基準軸概念)

### 12.2 InArm 基準軸（依 `USE_IN_OUT_ARM_Y_PITCH`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md#122-inarm-基準軸依-use_in_out_arm_y_pitch)

### 12.3 OutArm 基準軸 — 與 InArm 鏡像對稱

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md#123-outarm-基準軸--與-inarm-鏡像對稱)

### 12.4 各模式基準軸視覺化（HT-9xxx，2x4 = 8 吸嘴）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/13.md#124-各模式基準軸視覺化ht-9xxx2x4--8-吸嘴)

## 13. Teach 點位結構（`LastSet.h` Tech 結構 → `Prod` 全域）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/14.md#13-teach-點位結構lastseth-tech-結構--prod-全域)

### 13.1 Setup 頁面只 Teach 基準軸

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/14.md#131-setup-頁面只-teach-基準軸)

### 13.2 個別吸嘴的 Z 高度差（`iInArmZHeightSub[2][4]`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/14.md#132-個別吸嘴的-z-高度差iinarmzheightsub24)

### 13.3 初始化時推算所有吸嘴座標（`cinitial.cpp` L8637+）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/14.md#133-初始化時推算所有吸嘴座標cinitialcpp-l8637)

## 14. Loader / HP / Shuttle 取放料 X/Y 公式

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/15.md#14-loader--hp--shuttle-取放料-xy-公式)

### 14.1 Loader 取料（多吸嘴版，`ainarm9045.cpp` L4660）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/15.md#141-loader-取料多吸嘴版ainarm9045cpp-l4660)

### 14.2 Loader 取料（單吸嘴 Fix 模式，`ainarm9045.cpp` L4704）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/15.md#142-loader-取料單吸嘴-fix-模式ainarm9045cpp-l4704)

### 14.3 個別吸嘴 X/Y Offset（Eastsun 20251231 修正後）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/15.md#143-個別吸嘴-xy-offseteastsun-20251231-修正後)

## 15. HotPlate 縮 Pitch 數學模型（PPT Slide 23-26）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/16.md#15-hotplate-縮-pitch-數學模型ppt-slide-23-26)

## 16. X-Pitch 馬達與「單步距離」（`ainarm9045.cpp` L4476-4610）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/17.md#16-x-pitch-馬達與單步距離ainarm9045cpp-l4476-4610)

### 16.1 兩個關鍵全域變數

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/17.md#161-兩個關鍵全域變數)

### 16.2 計算公式

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/17.md#162-計算公式)

### 16.3 X-Pitch 機構模式（`USE_IN_OUT_ARM_X_PITCH`）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/17.md#163-x-pitch-機構模式use_in_out_arm_x_pitch)

## 17. OutArm 對稱版（鏡像） — 取放料公式參考

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/18.md#17-outarm-對稱版鏡像--取放料公式參考)

## 18. v1 → v2 修正記錄（缺漏與錯誤補充，v2 版保留）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/19.md#18-v1--v2-修正記錄缺漏與錯誤補充v2-版保留)

## 19. OutArm 類型路由 — `DoOutArm_9045()` Dispatch（`aoutarm9045.cpp` L540）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md#19-outarm-類型路由--dooutarm_9045-dispatchaoutarm9045cpp-l540)

### 19.1 關鍵結論：OutArm 沒有獨立的 `iOutArmType`

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md#191-關鍵結論outarm-沒有獨立的-ioutarmtype)

### 19.2 `DoOutArm_9045()` 路由表

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md#192-dooutarm_9045-路由表)

### 19.3 OutArm Guard Checks（InArm 沒有的）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md#193-outarm-guard-checksinarm-沒有的)

### 19.4 InArm vs OutArm 類型決策差異

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/20.md#194-inarm-vs-outarm-類型決策差異)

## 20. InArm / OutArm SetPickerCount 完整對照表

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/21.md#20-inarm--outarm-setpickercount-完整對照表)

### 20.1 InArm → OutArm 函式名對應

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/21.md#201-inarm--outarm-函式名對應)

### 20.2 OutArm 主狀態機概覽（與 InArm 對稱）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/21.md#202-outarm-主狀態機概覽與-inarm-對稱)

## 21. 本次技能更新摘要（2026-05-13）

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md#21-本次技能更新摘要2026-05-13)

### 19.1 `ht9045-sucker-architecture` SKILL 更新

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md#191-ht9045-sucker-architecture-skill-更新)

### 19.2 `ht9045-inarm-flow` SKILL 更新

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md#192-ht9045-inarm-flow-skill-更新)

### 19.3 `ht9045-outarm-flow` SKILL 更新

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md#193-ht9045-outarm-flow-skill-更新)

### 19.4 更新前後差異總結

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/22.md#194-更新前後差異總結)

## 相關知識庫連結

[讀取此節](Arm_TMyKitSuck_BaseAxis_PitchCalc_20260513/23.md#相關知識庫連結)
