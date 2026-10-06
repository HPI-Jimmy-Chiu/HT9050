# HotPlate 放料路徑知識庫

按需要選取以下章節，原文依順序保留。

- [HotPlate 放料路徑知識庫](HP_Knowledgebase/00.md)
- [HotPlate 放料路徑知識庫](HP_Knowledgebase/01.md)
- [適用場景](HP_Knowledgebase/02.md)
- [原始碼位置](HP_Knowledgebase/03.md)
- [0. InArm → HotPlate 座標配置](HP_Knowledgebase/04.md)
- [1. HotPlate.Data 檔案結構](HP_Knowledgebase/05.md)
- [2. Tray 資料狀態定義](HP_Knowledgebase/06.md)
- [3. 關鍵全域變數](HP_Knowledgebase/07.md)
- [3.5 iInArmType vs 吸嘴選擇函式 — 路由陷阱](HP_Knowledgebase/08.md)
- [4. DoPlaceToHotPlate_9045() — 放料 Dispatcher](HP_Knowledgebase/09.md)
- [5. 放料狀態機通用流程](HP_Knowledgebase/10.md)
- [6. SearchPlateToPlace() — 空格搜尋入口](HP_Knowledgebase/11.md)
- [7. SearchPlacePlateXItem3_1x2Suck() — 3-Col HP 搜尋（1x2 Mode）](HP_Knowledgebase/12.md)
- [8. CheckHasSpaceToPlace_9045() — 空格門檻計算](HP_Knowledgebase/13.md)
- [9. GetHotPlateColStep() / GetPlaceToHotPlateSuckCol() / GetPlaceToHotPlateCol()](HP_Knowledgebase/14.md)
- [10. GetHotPlateYHalfPos() — iYHalf 計算](HP_Knowledgebase/15.md)
- [11. DoPlaceToHPSwapData() — 放料資料交換](HP_Knowledgebase/16.md)
- [12. Row2CanPutHP() — 是否可放第二排吸嘴](HP_Knowledgebase/17.md)
- [13. 已知問題與修改提案](HP_Knowledgebase/18.md)
- [14. HotPlate HangUp 防護機制](HP_Knowledgebase/19.md)
- [15. 工作檔範例模擬（Polaris 3x5 HP + 1x2_14）](HP_Knowledgebase/20.md)
- [16. `DoInArmPickFromHotPlate_9045()` — 取料狀態機](HP_Knowledgebase/21.md)
- [17. `HotplateDataConversion()` — 取料資料交換](HP_Knowledgebase/22.md)
- [18. `SearchPlateToPick()` — 取料格位搜尋](HP_Knowledgebase/23.md)
- [19. `HasHotReadyIC_9045()` — 檢查 HP 是否有 IC 可取](HP_Knowledgebase/24.md)
- [20. `PorcessJAM0109HotPlatePickUpErrorSkip()` — HP 取料 Skip 處理](HP_Knowledgebase/25.md)
- [21. `GetHeaterWaitTime()` — 取得剩餘加熱等待時間](HP_Knowledgebase/26.md)
- [22. `iHotCount` 計數器與批次識別](HP_Knowledgebase/27.md)
- [附錄A：InArm 上下eContext](HP_Knowledgebase/28.md)
- [References — 歷次除錯與修改紀錄](HP_Knowledgebase/29.md)

# HotPlate 放料路徑知識庫

[讀取此節](HP_Knowledgebase/00.md#hotplate-放料路徑知識庫)

## HotPlate 放料路徑知識庫

[讀取此節](HP_Knowledgebase/01.md#hotplate-放料路徑知識庫)

## 適用場景

[讀取此節](HP_Knowledgebase/02.md#適用場景)

## 原始碼位置

[讀取此節](HP_Knowledgebase/03.md#原始碼位置)

## 0. InArm → HotPlate 座標配置

[讀取此節](HP_Knowledgebase/04.md#0-inarm--hotplate-座標配置)

### 教示值（Tech）→ 運行值（Prod）流程

[讀取此節](HP_Knowledgebase/04.md#教示值tech-運行值prod流程)

### 座標限制

[讀取此節](HP_Knowledgebase/04.md#座標限制)

### 變數對照

[讀取此節](HP_Knowledgebase/04.md#變數對照)

## 1. HotPlate.Data 檔案結構

[讀取此節](HP_Knowledgebase/05.md#1-hotplatedata-檔案結構)

### HotPlate 座標映射

[讀取此節](HP_Knowledgebase/05.md#hotplate-座標映射)

## 2. Tray 資料狀態定義

[讀取此節](HP_Knowledgebase/06.md#2-tray-資料狀態定義)

## 3. 關鍵全域變數

[讀取此節](HP_Knowledgebase/07.md#3-關鍵全域變數)

## 3.5 iInArmType vs 吸嘴選擇函式 — 路由陷阱

[讀取此節](HP_Knowledgebase/08.md#35-iinarmtype-vs-吸嘴選擇函式--路由陷阱)

### 核心規則

[讀取此節](HP_Knowledgebase/08.md#核心規則)

### 吸嘴選擇函式路由表

[讀取此節](HP_Knowledgebase/08.md#吸嘴選擇函式路由表)

### 教訓（2026-05-12 偉測 WAR0150 案例）

[讀取此節](HP_Knowledgebase/08.md#教訓2026-05-12-偉測-war0150-案例)

## 4. DoPlaceToHotPlate_9045() — 放料 Dispatcher

[讀取此節](HP_Knowledgebase/09.md#4-doplacetohotplate_9045--放料-dispatcher)

### 功能

[讀取此節](HP_Knowledgebase/09.md#功能)

### iInArmType → 放料函式對照表

[讀取此節](HP_Knowledgebase/09.md#iinarmtype--放料函式對照表)

## 5. 放料狀態機通用流程

[讀取此節](HP_Knowledgebase/10.md#5-放料狀態機通用流程)

## 6. SearchPlateToPlace() — 空格搜尋入口

[讀取此節](HP_Knowledgebase/11.md#6-searchplatetoplace--空格搜尋入口)

### 搜尋方向

[讀取此節](HP_Knowledgebase/11.md#搜尋方向)

## 7. SearchPlacePlateXItem3_1x2Suck() — 3-Col HP 搜尋（1x2 Mode）

[讀取此節](HP_Knowledgebase/12.md#7-searchplaceplatexitem3_1x2suck--3-col-hp-搜尋1x2-mode)

### 放料格位分組

[讀取此節](HP_Knowledgebase/12.md#放料格位分組)

### 奇數 YDivision 保護（3x5 HP）

[讀取此節](HP_Knowledgebase/12.md#奇數-ydivision-保護3x5-hp)

## 8. CheckHasSpaceToPlace_9045() — 空格門檻計算

[讀取此節](HP_Knowledgebase/13.md#8-checkhasspacetoplace_9045--空格門檻計算)

### PlactCT 門檻對照

[讀取此節](HP_Knowledgebase/13.md#plactct-門檻對照)

### 8.1 ⚠ 單排吸嘴（`iPickRow==1`）落單：`PlactCT=8` 會多留一趟（SCK 20260727 案）

[讀取此節](HP_Knowledgebase/13.md#81--單排吸嘴ipickrow1落單plactct8-會多留一趟sck-20260727-案)

### 8.2 為什麼要卡 `XDivision==iPickCol`——「單排 ≠ 一次放完」

[讀取此節](HP_Knowledgebase/13.md#82-為什麼要卡-xdivisionipickcol單排--一次放完)

### 8.3 日後要放寬到 XDiv=8/12/16 的驗證 Checklist

[讀取此節](HP_Knowledgebase/13.md#83-日後要放寬到-xdiv81216-的驗證-checklist)

## 9. GetHotPlateColStep() / GetPlaceToHotPlateSuckCol() / GetPlaceToHotPlateCol()

[讀取此節](HP_Knowledgebase/14.md#9-gethotplatecolstep--getplacetohotplatesuckcol--getplacetohotplatecol)

### GetHotPlateColStep(iAction)

[讀取此節](HP_Knowledgebase/14.md#gethotplatecolstepiaction)

### GetPlaceToHotPlateSuckCol(j)

[讀取此節](HP_Knowledgebase/14.md#getplacetohotplatesuckcolj)

### GetPlaceToHotPlateCol(j)

[讀取此節](HP_Knowledgebase/14.md#getplacetohotplatecolj)

## 10. GetHotPlateYHalfPos() — iYHalf 計算

[讀取此節](HP_Knowledgebase/15.md#10-gethotplateyhalfpos--iyhalf-計算)

## 11. DoPlaceToHPSwapData() — 放料資料交換

[讀取此節](HP_Knowledgebase/16.md#11-doplacetohpswapdata--放料資料交換)

## 12. Row2CanPutHP() — 是否可放第二排吸嘴

[讀取此節](HP_Knowledgebase/17.md#12-row2canputhp--是否可放第二排吸嘴)

## 13. 已知問題與修改提案

[讀取此節](HP_Knowledgebase/18.md#13-已知問題與修改提案)

### 問題：3xN HP（奇數 YDivision）幽靈空位導致放料路徑不一致

[讀取此節](HP_Knowledgebase/18.md#問題3xn-hp奇數-ydivision幽靈空位導致放料路徑不一致)

## 14. HotPlate HangUp 防護機制

[讀取此節](HP_Knowledgebase/19.md#14-hotplate-hangup-防護機制)

## 15. 工作檔範例模擬（Polaris 3x5 HP + 1x2_14）

[讀取此節](HP_Knowledgebase/20.md#15-工作檔範例模擬polaris-3x5-hp--1x2_14)

## 16. `DoInArmPickFromHotPlate_9045()` — 取料狀態機

[讀取此節](HP_Knowledgebase/21.md#16-doinarmpickfromhotplate_9045--取料狀態機)

## 17. `HotplateDataConversion()` — 取料資料交換

[讀取此節](HP_Knowledgebase/22.md#17-hotplatedataconversion--取料資料交換)

### 執行步驟

[讀取此節](HP_Knowledgebase/22.md#執行步驟)

### 關鍵注意事項

[讀取此節](HP_Knowledgebase/22.md#關鍵注意事項)

## 18. `SearchPlateToPick()` — 取料格位搜尋

[讀取此節](HP_Knowledgebase/23.md#18-searchplatetopick--取料格位搜尋)

### ⚠️ `GetHPFirstTeamPlate()` 的 `iLimitcount` 永久 latch Bug（2026-06-13 偉測 KLD019 案例）

[讀取此節](HP_Knowledgebase/23.md#-gethpfirstteamplate-的-ilimitcount-永久-latch-bug2026-06-13-偉測-kld019-案例)

## 19. `HasHotReadyIC_9045()` — 檢查 HP 是否有 IC 可取

[讀取此節](HP_Knowledgebase/24.md#19-hashotreadyic_9045--檢查-hp-是否有-ic-可取)

### 判斷邏輯

[讀取此節](HP_Knowledgebase/24.md#判斷邏輯)

## 20. `PorcessJAM0109HotPlatePickUpErrorSkip()` — HP 取料 Skip 處理

[讀取此節](HP_Knowledgebase/25.md#20-porcessjam0109hotplatepickuperrorskip--hp-取料-skip-處理)

### 執行動作

[讀取此節](HP_Knowledgebase/25.md#執行動作)

## 21. `GetHeaterWaitTime()` — 取得剩餘加熱等待時間

[讀取此節](HP_Knowledgebase/26.md#21-getheaterwaittime--取得剩餘加熱等待時間)

## 22. `iHotCount` 計數器與批次識別

[讀取此節](HP_Knowledgebase/27.md#22-ihotcount-計數器與批次識別)

### 放料端（`DoPlaceToHPSwapData`）

[讀取此節](HP_Knowledgebase/27.md#放料端doplacetohpswapdata)

### 取料端（`HotplateDataConversion`）

[讀取此節](HP_Knowledgebase/27.md#取料端hotplatedataconversion)

### OutArm 使用

[讀取此節](HP_Knowledgebase/27.md#outarm-使用)

### 批次計數特性

[讀取此節](HP_Knowledgebase/27.md#批次計數特性)

## 附錄A：InArm 上下eContext

[讀取此節](HP_Knowledgebase/28.md#附錄ainarm-上下econtext)

## References — 歷次除錯與修改紀錄

[讀取此節](HP_Knowledgebase/29.md#references--歷次除錯與修改紀錄)

### REF-001：`iForPlaceHPX3Step` Race Condition（3x5 HP 1x2 Mode）

[讀取此節](HP_Knowledgebase/29.md#ref-001iforplacehpx3step-race-condition3x5-hp-1x2-mode)

### REF-002：`GetPlaceToHotPlateCol()` Race Condition（3x5 HP 1x2 Mode）

[讀取此節](HP_Knowledgebase/29.md#ref-002getplacetohotplatecol-race-condition3x5-hp-1x2-mode)

### REF-003：新增 Debug Log（Proposal A + B）

[讀取此節](HP_Knowledgebase/29.md#ref-003新增-debug-logproposal-a--b)

### REF-004：同類條件風險提示

[讀取此節](HP_Knowledgebase/29.md#ref-004同類條件風險提示)

### REF-005：Step 400 HasIC() 統一修正（3x5 HP Col2 IC 遺失）

[讀取此節](HP_Knowledgebase/29.md#ref-005step-400-hasic-統一修正3x5-hp-col2-ic-遺失)
