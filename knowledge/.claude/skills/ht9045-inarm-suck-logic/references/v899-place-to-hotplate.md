# HT9045 V899 InArm 放料到 HotPlate 深度解析

舊引用路徑保留；[讀取整理後文件](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate.md)。

## TOC

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/01.md#toc)

## 1. 範圍與關鍵檔案

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/02.md#1-範圍與關鍵檔案)

## 2. 全流程總覽

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/03.md#2-全流程總覽)

## 3. 主狀態機觸發路徑 (case 1000→1100)

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/04.md#3-主狀態機觸發路徑-case-10001100)

## 4. SearchPlateToPlace 搜尋空位

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/05.md#4-searchplatetoplace-搜尋空位)

### 路由邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/05.md#路由邏輯)

### SearchPlacePlateXItem4_8Suck 核心邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/05.md#searchplaceplatexitem4_8suck-核心邏輯)

### CheckHotPlateHasSpace_9045_8_New_V 判定邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/05.md#checkhotplatehasspace_9045_8_new_v-判定邏輯)

## 5. DoPlaceToHotPlate_9045_2x4_8 放料狀態機

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/06.md#5-doplacetohotplate_9045_2x4_8-放料狀態機)

## 6. Destroy 吹氣與 DoPlaceToHPSwapData 資料交換

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/07.md#6-destroy-吹氣與-doplacetohpswapdata-資料交換)

### case 350 核心邏輯

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/07.md#case-350-核心邏輯)

### DoPlaceToHPSwapData 做了什麼

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/07.md#doplacetohpswapdata-做了什麼)

## 7. bZFlgToHP 與 Z 軸下降控制

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/08.md#7-bzflgtohp-與-z-軸下降控制)

## 8. Shuttle / Kit 對 HotPlate 放料的影響

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/09.md#8-shuttle--kit-對-hotplate-放料的影響)

### 為什麼放料與 Shuttle 有關

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/09.md#為什麼放料與-shuttle-有關)

### Close-site 的影響

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/09.md#close-site-的影響)

### iWhichShtPickFor32

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/09.md#iwhichshtpickfor32)

## 9. 吸嘴到 HotPlate 的映射複雜性

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/10.md#9-吸嘴到-hotplate-的映射複雜性)

### GetPlaceToHotPlateSuckCol — 吸嘴 index 映射

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/10.md#getplacetohotplatesuckcol--吸嘴-index-映射)

### GetPlaceToHotPlateCol — HP 列映射

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/10.md#getplacetohotplatecol--hp-列映射)

### GetHotPlateColStep — 每輪放幾列

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/10.md#gethotplatecolstep--每輪放幾列)

## 10. HotPlate XDivision 對放料路徑的影響

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/11.md#10-hotplate-xdivision-對放料路徑的影響)

## 11. Row2CanPutHP 與 iPlaceHPOrder

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/12.md#11-row2canputhp-與-iplacehporder)

## 12. 放料後多輪迴路 (case 400)

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/13.md#12-放料後多輪迴路-case-400)

## 13. InspectInArmPosition 座標驗證工具

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/14.md#13-inspectinarmposition-座標驗證工具)

### 原理

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/14.md#原理)

### 支援的目標

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/14.md#支援的目標)

### 呼叫位置

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/14.md#呼叫位置)

### 已知限制與改進方向

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/14.md#已知限制與改進方向)

## 14. Data Swap Error 1/2 根因分析

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/15.md#14-data-swap-error-12-根因分析)

### Error 1：HP有料 + InArm也有料

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/15.md#error-1hp有料--inarm也有料)

### Error 2：HP有料 + InArm無料

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/15.md#error-2hp有料--inarm無料)

### 追蹤方向

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/15.md#追蹤方向)

## 15. Debug Checklist

[讀取此節](../../hpi-inarm-flow/references/vacuum/references/v899-place-to-hotplate/16.md#15-debug-checklist)
