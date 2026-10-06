---
name: ht9045-general-ini
description: >
  HT9045 規格指示書 → Gerneral.ini 同步工作流程。依規格書比對並更新 Gerneral_PQLE*.ini，
  涵蓋 CUSTOMER_CODE、Model、Serial No、Machine ID、Factory、ATC_SYSTEM_USEHEAT 等欄位校對，
  以 MachineType.h 驗證數值。
  INI 結構靜態規格（Section 定義、模組對應、資料流）已移至 Spec：d:\HT9045\.github\specs\gerneral-ini-schema.md
  觸發關鍵字：規格指示書, CUSTOMER_CODE, Serial No, Machine ID, Factory, ATC_SYSTEM_USEHEAT,
  USE_ATC_MODE, INDEX_PRESS_TYPE, Gerneral_PQLE, 機台序號, spec sync, 規格書更新,
  General.ini, Gerneral.ini, INI Key, HandlerSys, database, LoaderSystemSet, ReadGeneralIni
---

# ht9045-general-ini 相容入口

同主題已整合到 [hpi-config](../hpi-config/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-config/references/general/original-entry.md)

## 規格指示書轉 Gerneral.ini（Spec Sync 工作流程）

[讀取此節](../hpi-config/references/general/original-entry.md#規格指示書轉-gerneralinispec-sync-工作流程)

### 讀取順序

[讀取此節](../hpi-config/references/general/original-entry.md#讀取順序)

### 核心欄位對應

[讀取此節](../hpi-config/references/general/original-entry.md#核心欄位對應)

### 規格選項對應表

[讀取此節](../hpi-config/references/general/original-entry.md#規格選項對應表)

#### 001 取放裝置

[讀取此節](../hpi-config/references/general/original-entry.md#001-取放裝置)

#### 005 空 Tray 收送

[讀取此節](../hpi-config/references/general/original-entry.md#005-空-tray-收送)

#### 009 加熱模組

[讀取此節](../hpi-config/references/general/original-entry.md#009-加熱模組)

#### 014 Tray 取放機構

[讀取此節](../hpi-config/references/general/original-entry.md#014-tray-取放機構)

#### 022 Show Bin 顯示器

[讀取此節](../hpi-config/references/general/original-entry.md#022-show-bin-顯示器)

#### 037 Contact Force

[讀取此節](../hpi-config/references/general/original-entry.md#037-contact-force)

#### 059 Socket sensor 疊料偵測

[讀取此節](../hpi-config/references/general/original-entry.md#059-socket-sensor-疊料偵測)

#### 075 175度加熱

[讀取此節](../hpi-config/references/general/original-entry.md#075-175度加熱)

### 欄位驗證備查

[讀取此節](../hpi-config/references/general/original-entry.md#欄位驗證備查)

### 執行流程

[讀取此節](../hpi-config/references/general/original-entry.md#執行流程)

### Factory 寫法規則

[讀取此節](../hpi-config/references/general/original-entry.md#factory-寫法規則)

### 輸出要求

[讀取此節](../hpi-config/references/general/original-entry.md#輸出要求)
