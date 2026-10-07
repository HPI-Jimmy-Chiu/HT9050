# 機台既有 UPH 計算方式（Per-Tray Real-Time UPH）

舊引用路徑保留；[讀取整理後文件](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md)。

## 概述

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#概述)

## 時序流程

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#時序流程)

## 關鍵變數

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#關鍵變數)

## 程式碼來源

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#程式碼來源)

### 觸發點：`asendic_Loader.cpp` line ~1190

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#觸發點asendic_loadercpp-line-1190)

### 計數點：`ainarm9045.cpp` line ~2755

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#計數點ainarm9045cpp-line-2755)

### 計算觸發：`ainarm9045.cpp` line ~5226

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#計算觸發ainarm9045cpp-line-5226)

### 核心計算：`CalculateUPH(bool bReset)` — ainarm9045.cpp line ~4912

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#核心計算calculateuphbool-breset--ainarm9045cpp-line-4912)

## UI 顯示

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#ui-顯示)

### UPH_StringGrid（位於 fShowBinSelect 表單）

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#uph_stringgrid位於-fshowbinselect-表單)

### StatusBar

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#statusbar)

## CSV 記錄

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#csv-記錄)

### 標準格式（`IniConfig.bP11RecordUPH`）

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#標準格式iniconfigbp11recorduph)

### FOREHOPE 格式（CC_FOREHOPE_NINGBO）

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#forehope-格式cc_forehope_ningbo)

### EventLog

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#eventlog)

## SECS/GEM 通知

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#secsgem-通知)

## 與解析式 UPH 公式的差異

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#與解析式-uph-公式的差異)

### 驗證互補

[讀取此節](../../hpi-uph-model/references/legacy/references/uph-existing-realtime-method.md#驗證互補)
