# SearchPlacePlateXItem 全函式放料路徑圖與適用表

按需要選取以下章節，原文依順序保留。

- [SearchPlacePlateXItem 全函式放料路徑圖與適用表](HP_SearchPlacePlate_AllFunctions/00.md)
- [目錄](HP_SearchPlacePlate_AllFunctions/01.md)
- [§0 SearchPlateToPlace() Dispatch 總覽](HP_SearchPlacePlate_AllFunctions/02.md)
- [§1 _1Suck](HP_SearchPlacePlate_AllFunctions/03.md)
- [§2 _1x2Suck](HP_SearchPlacePlate_AllFunctions/04.md)
- [§3 _2x2Suck](HP_SearchPlacePlate_AllFunctions/05.md)
- [§4 XItem3_2Suck_3Site](HP_SearchPlacePlate_AllFunctions/06.md)
- [§5 XItem3_1x2Suck](HP_SearchPlacePlate_AllFunctions/07.md)
- [§6 XItem3_2x2Suck](HP_SearchPlacePlate_AllFunctions/08.md)
- [§7 XItem4_8Suck](HP_SearchPlacePlate_AllFunctions/09.md)
- [§8 XItem6_2x2_8Suck](HP_SearchPlacePlate_AllFunctions/10.md)
- [§9 XItem6_3Suck_ACEx](HP_SearchPlacePlate_AllFunctions/11.md)
- [§10 XItem6_8Suck](HP_SearchPlacePlate_AllFunctions/12.md)
- [§11 XItem6_8Suck_NotStandY](HP_SearchPlacePlate_AllFunctions/13.md)
- [§12 XItem8_8Suck](HP_SearchPlacePlate_AllFunctions/14.md)
- [§13 XItem10_8Suck](HP_SearchPlacePlate_AllFunctions/15.md)
- [§14 XItem12_2x6](HP_SearchPlacePlate_AllFunctions/16.md)
- [§15 XItem12_8Suck](HP_SearchPlacePlate_AllFunctions/17.md)
- [§16 XItem16_8Suck](HP_SearchPlacePlate_AllFunctions/18.md)
- [§17 全函式適用矩陣總表](HP_SearchPlacePlate_AllFunctions/19.md)
- [§18 關鍵變數速查](HP_SearchPlacePlate_AllFunctions/20.md)
- [§19 HP 配置規格表](HP_SearchPlacePlate_AllFunctions/21.md)
- [相關文件](HP_SearchPlacePlate_AllFunctions/22.md)

# SearchPlacePlateXItem 全函式放料路徑圖與適用表

[讀取此節](HP_SearchPlacePlate_AllFunctions/00.md#searchplaceplatexitem-全函式放料路徑圖與適用表)

## 目錄

[讀取此節](HP_SearchPlacePlate_AllFunctions/01.md#目錄)

## §0 SearchPlateToPlace() Dispatch 總覽

[讀取此節](HP_SearchPlacePlate_AllFunctions/02.md#0-searchplatetoplace-dispatch-總覽)

## §1 _1Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#1-_1suck)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#進入條件)

### 掃描邏輯

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#掃描邏輯)

### AutoClean 保護

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#autoclean-保護)

### 馬達座標公式

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#馬達座標公式)

### 路徑範例

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#路徑範例)

### ⚠️ XDiv=1 可達性限制

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#-xdiv1-可達性限制)

### ⚠️ 3xN 奇數 YDiv 差異

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#-3xn-奇數-ydiv-差異)

### WAR0151

[讀取此節](HP_SearchPlacePlate_AllFunctions/03.md#war0151)

## §2 _1x2Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#2-_1x2suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#核心機制)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#掃描策略)

### CheckHotPlateHasSpace 公式

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#checkhotplatehasspace-公式)

### 路徑圖（XDiv=6, 標準 Pitch, AxEx, iPickRow=1）

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#路徑圖xdiv6-標準-pitch-axex-ipickrow1)

### 路徑圖（XDiv=8, bPitchOver12000, AxEx）

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#路徑圖xdiv8-bpitchover12000-axex)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/04.md#進入條件)

## §3 _2x2Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/05.md#3-_2x2suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/05.md#核心機制)

### 特殊 HP 旗標處理

[讀取此節](HP_SearchPlacePlate_AllFunctions/05.md#特殊-hp-旗標處理)

### 路徑圖（XDiv=4, YDiv=8, iYHalf=2, AxEx, 標準 Pitch）

[讀取此節](HP_SearchPlacePlate_AllFunctions/05.md#路徑圖xdiv4-ydiv8-iyhalf2-axex-標準-pitch)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/05.md#進入條件)

## §4 XItem3_2Suck_3Site

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#4-xitem3_2suck_3site)

### 掃描邏輯

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#掃描邏輯)

### 路徑圖（3x5 HP, 窄Pitch, Ab開啟）

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#路徑圖3x5-hp-窄pitch-ab開啟)

### 路徑圖（3x5 HP, 窄Pitch, Ab關站）

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#路徑圖3x5-hp-窄pitch-ab關站)

### 路徑圖（3x7 HP, 寬Pitch）

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#路徑圖3x7-hp-寬pitch)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/06.md#進入條件)

## §5 XItem3_1x2Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/07.md#5-xitem3_1x2suck)

### 掃描邏輯

[讀取此節](HP_SearchPlacePlate_AllFunctions/07.md#掃描邏輯)

### 路徑圖（3x6 HP, 偶數 YDiv）

[讀取此節](HP_SearchPlacePlate_AllFunctions/07.md#路徑圖3x6-hp-偶數-ydiv)

### 路徑圖（3x5 HP, 奇數 YDiv）

[讀取此節](HP_SearchPlacePlate_AllFunctions/07.md#路徑圖3x5-hp-奇數-ydiv)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/07.md#進入條件)

## §6 XItem3_2x2Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/08.md#6-xitem3_2x2suck)

### 掃描邏輯

[讀取此節](HP_SearchPlacePlate_AllFunctions/08.md#掃描邏輯)

### 路徑圖（3x6 HP, AxEx 2Row）

[讀取此節](HP_SearchPlacePlate_AllFunctions/08.md#路徑圖3x6-hp-axex-2row)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/08.md#進入條件)

## §7 XItem4_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/09.md#7-xitem4_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/09.md#核心機制)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/09.md#掃描策略)

### 路徑圖（4x8 HP, ACEG, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/09.md#路徑圖4x8-hp-aceg-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/09.md#進入條件)

## §8 XItem6_2x2_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#8-xitem6_2x2_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#核心機制)

### iForPlaceHPX6Step 判斷

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#iforplacehpx6step-判斷)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#掃描策略)

### 路徑圖（6x11 HP, AxEx, 2Row, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#路徑圖6x11-hp-axex-2row-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/10.md#進入條件)

## §9 XItem6_3Suck_ACEx

[讀取此節](HP_SearchPlacePlate_AllFunctions/11.md#9-xitem6_3suck_acex)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/11.md#核心機制)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/11.md#掃描策略)

### 路徑圖（6x11 HP, WideHP, 2Row, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/11.md#路徑圖6x11-hp-widehp-2row-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/11.md#進入條件)

## §10 XItem6_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/12.md#10-xitem6_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/12.md#核心機制)

### iForPlaceHPX6Step 判斷（ix==4 時）

[讀取此節](HP_SearchPlacePlate_AllFunctions/12.md#iforplacehpx6step-判斷ix4-時)

### 路徑圖（6x11 HP, WideHP, ACEG, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/12.md#路徑圖6x11-hp-widehp-aceg-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/12.md#進入條件)

## §11 XItem6_8Suck_NotStandY

[讀取此節](HP_SearchPlacePlate_AllFunctions/13.md#11-xitem6_8suck_notstandy)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/13.md#核心機制)

### iForPlaceHPX6Step 規則

[讀取此節](HP_SearchPlacePlate_AllFunctions/13.md#iforplacehpx6step-規則)

### 路徑圖（6x11 HP, 非標準Y）

[讀取此節](HP_SearchPlacePlate_AllFunctions/13.md#路徑圖6x11-hp-非標準y)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/13.md#進入條件)

## §12 XItem8_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#12-xitem8_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#核心機制)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#掃描策略)

### ⚠️ 游標推進順序：`iy` 內圈、`ix` 外圈（**同一欄組先掃完所有列**）

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#-游標推進順序iy-內圈ix-外圈同一欄組先掃完所有列)

### 路徑圖（8x16 HP, ACEG, 一般, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#路徑圖8x16-hp-aceg-一般-iyhalf2)

### ⚠️ `Row=1` 只該給「只剩一排有料」用；滿手走它會造成永遠取不滿

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#-row1-只該給只剩一排有料用滿手走它會造成永遠取不滿)

### ⚠️ 配對數上限：8×16 盤的可用容量是 **112** 不是 128

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#-配對數上限816-盤的可用容量是-112-不是-128)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/14.md#進入條件)

## §13 XItem10_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/15.md#13-xitem10_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/15.md#核心機制)

### iForPlaceHPX10Step

[讀取此節](HP_SearchPlacePlate_AllFunctions/15.md#iforplacehpx10step)

### 路徑圖（10x16 HP, ACEG, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/15.md#路徑圖10x16-hp-aceg-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/15.md#進入條件)

## §14 XItem12_2x6

[讀取此節](HP_SearchPlacePlate_AllFunctions/16.md#14-xitem12_2x6)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/16.md#核心機制)

### 掃描策略

[讀取此節](HP_SearchPlacePlate_AllFunctions/16.md#掃描策略)

### 路徑圖（12x16 HP, 2x6, iXYPitchVariable）

[讀取此節](HP_SearchPlacePlate_AllFunctions/16.md#路徑圖12x16-hp-2x6-ixypitchvariable)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/16.md#進入條件)

## §15 XItem12_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/17.md#15-xitem12_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/17.md#核心機制)

### 路徑圖（12x24 HP, ACEG, iYHalf=2）

[讀取此節](HP_SearchPlacePlate_AllFunctions/17.md#路徑圖12x24-hp-aceg-iyhalf2)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/17.md#進入條件)

## §16 XItem16_8Suck

[讀取此節](HP_SearchPlacePlate_AllFunctions/18.md#16-xitem16_8suck)

### 核心機制

[讀取此節](HP_SearchPlacePlate_AllFunctions/18.md#核心機制)

### 路徑圖（16x24 HP, 32Site）

[讀取此節](HP_SearchPlacePlate_AllFunctions/18.md#路徑圖16x24-hp-32site)

### 進入條件

[讀取此節](HP_SearchPlacePlate_AllFunctions/18.md#進入條件)

## §17 全函式適用矩陣總表

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#17-全函式適用矩陣總表)

### 17.1 搜尋函式 × XDiv 對應

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#171-搜尋函式--xdiv-對應)

### 17.2 生產模式 × XDiv × 搜尋函式

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#172-生產模式--xdiv--搜尋函式)

### 17.3 XDiv=6 子分派細節

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#173-xdiv6-子分派細節)

### 17.4 XDiv=12 子分派細節

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#174-xdiv12-子分派細節)

### 17.5 XDiv=1 可達性矩陣

[讀取此節](HP_SearchPlacePlate_AllFunctions/19.md#175-xdiv1-可達性矩陣)

## §18 關鍵變數速查

[讀取此節](HP_SearchPlacePlate_AllFunctions/20.md#18-關鍵變數速查)

## §19 HP 配置規格表

[讀取此節](HP_SearchPlacePlate_AllFunctions/21.md#19-hp-配置規格表)

### 19.1 欄位說明

[讀取此節](HP_SearchPlacePlate_AllFunctions/21.md#191-欄位說明)

### 19.2 配置表

[讀取此節](HP_SearchPlacePlate_AllFunctions/21.md#192-配置表)

### 19.3 支援 Picker 類型的 XDivision 範圍

[讀取此節](HP_SearchPlacePlate_AllFunctions/21.md#193-支援-picker-類型的-xdivision-範圍)

## 相關文件

[讀取此節](HP_SearchPlacePlate_AllFunctions/22.md#相關文件)
