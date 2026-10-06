# 台達 Delta DTM 多迴路模組化溫控器（重點 DTME08／DTMN08）（溫控器手冊參照）

舊引用路徑保留；[讀取整理後文件](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md)。

## 0. 結論先講：DTME08 是台達的

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#0-結論先講dtme08-是台達的)

## 1. HT9045 在哪裡用

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#1-ht9045-在哪裡用)

### 1.1 設定鍵與值

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#11-設定鍵與值)

### 1.2 通訊介面與參數

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#12-通訊介面與參數)

### 1.3 站號規則

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#13-站號規則)

### 1.4 程式位置

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#14-程式位置)

### 1.5 輪詢順序（golden V912 `EJ1N/fDTME08.cpp:475-676`）

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#15-輪詢順序golden-v912-ej1nfdtme08cpp475-676)

## 2. 手冊清單

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#2-手冊清單)

## 3. 通訊重點（手冊內容）

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#3-通訊重點手冊內容)

### 3.1 系統架構與站號

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#31-系統架構與站號)

### 3.2 協定與預設值

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#32-協定與預設值)

### 3.3 通訊位址（OM p.60～64（7-1～7-5）；`Hx###` 的 **x＝內部站號**，主機 x=0；CH1～CH8 依序＋0～＋7）

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#33-通訊位址om-p60647-17-5hx-的-x內部站號主機-x0ch1ch8-依序07)

### 3.4 感測器代碼（OM p.22～23（3-2～3-3））與 PV 錯誤碼（OM p.23（3-3））

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#34-感測器代碼om-p22233-23-3與-pv-錯誤碼om-p233-3)

### 3.5 檢查碼

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#35-檢查碼)

## 4. 與程式對照（golden V912 `EJ1N\`）

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#4-與程式對照golden-v912-ej1n)

## 5. 注意事項（含 HT9050）

[讀取此節](../../../hpi-temperature/references/core/references/controllers/delta-dtm.md#5-注意事項含-ht9050)
