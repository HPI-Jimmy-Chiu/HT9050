# UPH 記錄機制 Reference

> 內容已分層整理，舊路徑保留供既有引用使用。

[讀取整理後文件](../../hpi-index-flow/references/flow/uph-record.md)

## 1. 一句話結論

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#1-一句話結論)

## 2. 統計區間怎麼切（最常被誤解）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#2-統計區間怎麼切最常被誤解)

### 計算主體 `CalculateUPH(bool bReset)` — `ainarm9045.cpp:5444`

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#計算主體-calculateuphbool-breset--ainarm9045cpp5444)

## 3. 畫面顯示

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#3-畫面顯示)

### UPH 頁欄位

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#uph-頁欄位)

## 4. 檔案落地（客戶問「有沒有文件」的答案）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#4-檔案落地客戶問有沒有文件的答案)

### ① EventLog —— **預設就記，不需開任何選項**

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#①-eventlog--預設就記不需開任何選項)

### ② 生產記錄 / DB

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#②-生產記錄--db)

### ③ 專用 UPH CSV —— 需 `[P11] Record UPH information`

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#③-專用-uph-csv--需-p11-record-uph-information)

## 5. `IniConfig.bP11RecordUPH` 開啟後的記錄格式

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#5-iniconfigbp11recorduph-開啟後的記錄格式)

### ⚠️ 客戶碼白名單（`cConfiguration.cpp:4154-4162`）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#-客戶碼白名單cconfigurationcpp4154-4162)

### 5-1. 一般白名單客戶（非 KYEC_LEE、非 FOREHOPE_NINGBO）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#5-1-一般白名單客戶非-kyec_lee非-forehope_ningbo)

### 5-2. `CC_KYEC_LEE`

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#5-2-cc_kyec_lee)

### 5-3. `CC_FOREHOPE_NINGBO`（華天寧波）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#5-3-cc_forehope_ningbo華天寧波)

## 6. SECS/GEM 與 Remote Command

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#6-secsgem-與-remote-command)

## 7. 判讀陷阱（客戶反饋 UPH 數字怪異時先查這裡）

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#7-判讀陷阱客戶反饋-uph-數字怪異時先查這裡)

## 8. 快速定位用 grep

[讀取此節](../../hpi-index-flow/references/flow/uph-record.md#8-快速定位用-grep)
