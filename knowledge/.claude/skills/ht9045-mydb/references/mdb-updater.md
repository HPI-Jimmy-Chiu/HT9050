# MDB Updater（SQLiteUpdater.exe）參考

舊引用路徑保留；[讀取整理後文件](../../hpi-web-hmi/references/logs/references/mdb-updater.md)。

## 1. 這支程式是什麼

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/01.md#1-這支程式是什麼)

### 核心檔案

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/01.md#核心檔案)

### 執行流程

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/01.md#執行流程)

## 2. ⚠ 核心耦合：CreateTableAlarmList ↔ MyDBUpdateDB

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/02.md#2--核心耦合createtablealarmlist--mydbupdatedb)

## 3. AlarmCode 格式與唯一性

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/03.md#3-alarmcode-格式與唯一性)

### Unit 編號對照

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/03.md#unit-編號對照)

## 4. DB Schema（BCB 線 `Handler.db3` 主要 Table；V906 沒有 sqlite）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/04.md#4-db-schemabcb-線-handlerdb3-主要-tablev906-沒有-sqlite)

## 5. 新增 AlarmCode 作業程序（雙端同步）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#5-新增-alarmcode-作業程序雙端同步)

### Step 1：決定放在哪一端

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#step-1決定放在哪一端)

### Step 2：MDB Updater 端（`CreatDatabase.cpp`，Big5）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#step-2mdb-updater-端creatdatabasecppbig5)

### Step 3：HT9045 端（`cMyDB.cpp`，補丁模式，讓已部署的機台也有這個碼）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#step-3ht9045-端cmydbcpp補丁模式讓已部署的機台也有這個碼)

### Step 4：驗證（BCB 線）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#step-4驗證bcb-線)

### V906（C++）的對應步驟

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/05.md#v906c的對應步驟)

## 6. 常見作業情境

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#6-常見作業情境)

### 情境 A：新增一個 Tray Loader 相關的 JAM 警報

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#情境-a新增一個-tray-loader-相關的-jam-警報)

### 情境 B：修改 DB Schema（新增 Table 欄位，BCB 線）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#情境-b修改-db-schema新增-table-欄位bcb-線)

### 情境 C：新增 Motor 到 Unit 24

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#情境-c新增-motor-到-unit-24)

### 情境 D：版本升版

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#情境-d版本升版)

### 情境 E：把 MyDBUpdateDB() 的補丁碼升格為靜態碼（定期同步）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/06.md#情境-e把-mydbupdatedb-的補丁碼升格為靜態碼定期同步)

## 7. ⚠ 已知衝突碼（同編號、不同訊息）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/07.md#7--已知衝突碼同編號不同訊息)

### WAR16118 解決紀錄（20260331）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/07.md#war16118-解決紀錄20260331)

### WAR16339 解決紀錄（20260331）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/07.md#war16339-解決紀錄20260331)

## 8. 約束條件（BCB 線）

[讀取此節](../../hpi-web-hmi/references/logs/references/mdb-updater/08.md#8-約束條件bcb-線)
