---
name: ht9045-recipe
description: >
  HT9045 IC Test Handler 工作檔（Recipe）結構與程式碼對應。涵蓋 11 個核心 .Data 檔
  （TestMode.Data、HotPlate.Data、HandlerCondition.Data、Contact.Data、Temperature.Data、
  Binasgn.Data、ArmCondition.Data、Tray.Data、Tester.Data、Rotate.Data、UdUld.Data）、
  7 個 Binasgn 變體、工作檔切換流程（ChangeRecipe/DataPath）、全域變數對應、
  configByRecipe.ini 覆蓋邏輯、Big5 編碼限制。
  觸發關鍵字：Recipe, 工作檔, DataPath, TestMode.Data, HotPlate.Data, Contact.Data,
  Temperature.Data, ArmCondition.Data, Binasgn.Data, Tray.Data, UdUld.Data,
  HandlerCondition.Data, Tester.Data, Rotate.Data, configByRecipe.ini,
  ReadTestMode, ReadContactFile, ReadTrayFile, ReadArmCondition, ChangeRecipe,
  GetLastOpenFN, BinSelect, SYSTEM_TEST_MODE, SYSTEM_BIN_SELECT。
  AOI.Data, TFrmAOI, fAOI_ReadFile, AOISetup, ScannerAOIIF, tAOISetup, TfOCR, fOCR_ReadFile,
  OCR SETTING, StartDelayTimeScanAOIView, StartDelayTimeTopScanAOIView, TimeOutScanTopAOIView,
  TimeOutTopScanAOIView, Q31, R71。
applyTo: "**/*"
---

# ht9045-recipe 相容入口

同主題已整合到 [hpi-lotinfo-recipe](../hpi-lotinfo-recipe/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-lotinfo-recipe/references/recipe/original-entry.md)

## 概述

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#概述)

## 工作檔清單與 Reference 對照

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#工作檔清單與-reference-對照)

### 核心工作檔（11 個）

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#核心工作檔11-個)

### 複製工作檔（7 個 Binasgn 變體）

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#複製工作檔7-個-binasgn-變體)

### 輔助檔案（5 個）

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#輔助檔案5-個)

### 其他工作檔（20260927 補）

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#其他工作檔20260927-補)

## 工作檔載入流程

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#工作檔載入流程)

### 1. 程式啟動時讀取工作檔名

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#1-程式啟動時讀取工作檔名)

### 2. 各模組分別讀取對應工作檔

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#2-各模組分別讀取對應工作檔)

### 3. 工作檔切換流程

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#3-工作檔切換流程)

## 全域變數對應表

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#全域變數對應表)

## Web API 這一側：配方文件是 glob 出來的，不是寫死清單（20260914 實測）

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#web-api-這一側配方文件是-glob-出來的不是寫死清單20260914-實測)

### 鍵的存在性：分母要算「擁有該文件的配方」，不是全部配方

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#鍵的存在性分母要算擁有該文件的配方不是全部配方)

## 常見問題排查

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#常見問題排查)

## 重要提示

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#重要提示)

## 相關技能與知識庫

[讀取此節](../hpi-lotinfo-recipe/references/recipe/original-entry.md#相關技能與知識庫)
