---
name: ht9045-lotinfo-flow
description: "HT9045 LotInfo 批次管理流程知識庫。當使用者詢問 uLotInfo、TfLotInfo、Lot Start、Lot End、SetLotStart、SetLotID、SetLotInfo、sbSECSLotStartClick、sbSECSLotEndClick、DownloadFromServer、Recipe 下載、Recipe 上傳、PP_DL_REQUEST、LotInfo_REQUEST、DoBackupSetupFile、DoOverWriteSetupFile、Security_new.def、ATC 溫控顯示、Yield Monitor、FTP Automation、SPIL 下載、批次 ID 持久化、CyuEan LotInfo CSV 等相關問題時，應先載入此技能以理解 LotInfo 完整批次管理流程。關鍵字：LotInfo, Lot Start, Lot End, SetLotStart, SetLotID, Recipe, PP_DL_REQUEST, FTP, uLotInfo, TfLotInfo, SECS Lot, Security_new.def。"
---

# ht9045-lotinfo-flow 相容入口

同主題已整合到 [hpi-lotinfo-recipe](../hpi-lotinfo-recipe/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-lotinfo-recipe/references/lot/original-entry.md)

## 適用範圍

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#適用範圍)

## 1. 模組概述

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#1-模組概述)

### 主要職能

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#主要職能)

## 2. 關鍵函式速查

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#2-關鍵函式速查)

## 3. Lot Start 流程

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#3-lot-start-流程)

### 3.1 呼叫來源

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#31-呼叫來源)

### 3.2 SetLotStart 狀態機（V899 L1438）

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#32-setlotstart-狀態機v899-l1438)

### 3.3 Lot Start 前置條件（sbSECSLotStartClick）

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#33-lot-start-前置條件sbsecslotstartclick)

## 4. Lot End 流程

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#4-lot-end-流程)

### 4.1 sbSECSLotEndClick（L876）

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#41-sbsecslotendclickl876)

## 5. Config 持久化架構

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#5-config-持久化架構)

## 6. Recipe 下載流程概要

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#6-recipe-下載流程概要)

### 觸發方式

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#觸發方式)

### 下載引擎

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#下載引擎)

## 7. SECS/GEM 事件對應

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#7-secsgem-事件對應)

## 8. ATC 溫控顯示架構

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#8-atc-溫控顯示架構)

## 9. CyuEan 特殊 CSV Log

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#9-cyuean-特殊-csv-log)

## 9b. V906 移植樹：RTC 換檔、Change File、palSecsGem 暗門（20261002，St01，todo E-020 LI-6／LI-11／LI-13）

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#9b-v906-移植樹rtc-換檔change-filepalsecsgem-暗門20261002st01todo-e-020-li-6li-11li-13)

## 10. 已知問題 / 注意事項

[讀取此節](../hpi-lotinfo-recipe/references/lot/original-entry.md#10-已知問題--注意事項)
