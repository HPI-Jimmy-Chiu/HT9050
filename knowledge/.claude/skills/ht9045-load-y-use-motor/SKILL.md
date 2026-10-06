---
name: ht9045-load-y-use-motor
description: "HT9045 Loader/Empty/Color/Auto1/Auto2/Auto3 Y 軸步進馬達 (LOAD_Y_USE_MOTOR[0]~[5]) 進出 Tray 機構知識庫。Use when: 修改 Tray Y 軸步進馬達進出料邏輯、TrayMoveIn/TrayMoveOut/TrayMoveStatus、新增 EMPTY_Y_USE_MOTOR/COLOR_Y_USE_MOTOR/AUTOn_Y_USE_MOTOR 設定、處理 INSTALL_OCR_YMot 與 LOAD_Y_USE_MOTOR 衝突、HandlerSys 設定面板 Y 軸馬達勾選、MLoaderY/MEmptyY/MColorY/MAuto1Y/MAuto2Y/MAuto3Y 步進馬達控制。關鍵字：LOAD_Y_USE_MOTOR, TrayMoveIn, TrayMoveOut, TrayMoveStatus, MLoaderY, MEmptyY, MColorY, MAuto1Y, MAuto2Y, MAuto3Y, INSTALL_OCR_YMot, LoaderUnload_StepMotor, USE_LdUldCassetteMode, chkLoaderY, grpTrayYUseMot, TrayY ini section, Loader Tray 步進"
---

# ht9045-load-y-use-motor 相容入口

同主題已整合到 [hpi-tray-flow](../hpi-tray-flow/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md)

## 1. 變數定位

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#1-變數定位)

### 1.1 宣告

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#11-宣告)

### 1.2 索引語意（與 `MAX_TRACK = 9` 軌道對齊）

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#12-索引語意與-max_track--9-軌道對齊)

## 2. 設定來源（INI）

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#2-設定來源ini)

### 2.1 [0] 載入規則

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#21-0-載入規則)

## 3. UI 面板（HandlerSys）

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#3-ui-面板handlersys)

## 4. 行為邏輯

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#4-行為邏輯)

### 4.1 進料：`TrayMoveIn(bool bMove, int iAxis, int iPos)`

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#41-進料traymoveinbool-bmove-int-iaxis-int-ipos)

### 4.2 出料：`TrayMoveOut(bool bMove, int iAxis, int iPos)`

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#42-出料traymoveoutbool-bmove-int-iaxis-int-ipos)

### 4.3 狀態判讀：`TrayMoveStatus(int iAxis, AnsiString sFun)`

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#43-狀態判讀traymovestatusint-iaxis-ansistring-sfun)

### 4.4 馬達測試列表：`uMotorTest.cpp`

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#44-馬達測試列表umotortestcpp)

## 5. 與其他相近旗標的關係

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#5-與其他相近旗標的關係)

## 6. 影響函式總覽

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#6-影響函式總覽)

## 7. 風險與注意事項

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#7-風險與注意事項)

## 8. 驗證建議

[讀取此節](../hpi-tray-flow/references/mechanisms/tray-y/original-entry.md#8-驗證建議)
