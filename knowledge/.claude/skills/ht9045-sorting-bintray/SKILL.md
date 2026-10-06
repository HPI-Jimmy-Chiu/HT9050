---
name: ht9045-sorting-bintray
description: "HT9045 Unloader 整盤功能 (SortingBinTray / P27) 知識庫。涵蓋 DoSortingBinTray 狀態機、DoFix3FullTray 流程、觸發條件、CosFunction/IniConfig 旗標、互鎖邏輯。Use when: 分析 Clean Out 整盤、Tray Feed 整盤、P27 設定、Fix3 FullTray、OutArm 整盤流程。關鍵字：SortingBinTray, DoSortingBinTray, bP27AutoSortingBinTrayByOutArmwhenCleanOut, bSortingBinTraywhenCleanOut, bSortingBinTrayWhenTrayFeed, DoFix3FullTray, bSortingAllBinTrayFinish, bSortingSuckMode, Clean Out, Tray Feed, Unloader 整盤, P27"
---

# ht9045-sorting-bintray 相容入口

同主題已整合到 [hpi-tray-flow](../hpi-tray-flow/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-tray-flow/references/sorting/original-entry.md)

## 功能概述

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#功能概述)

## 旗標體系

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#旗標體系)

### 啟用條件（三層控制）

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#啟用條件三層控制)

### Configuration P27 顯示邏輯

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#configuration-p27-顯示邏輯)

### CosFunction 初始化

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#cosfunction-初始化)

### 強制關閉條件

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#強制關閉條件)

### 執行時輔助旗標

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#執行時輔助旗標)

## 觸發路徑

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#觸發路徑)

### 路徑 A：Clean Out 時整盤

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#路徑-aclean-out-時整盤)

### 路徑 B：Tray Feed 前整盤

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#路徑-btray-feed-前整盤)

### OneCycleFinish 保護

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#onecyclefinish-保護)

## DoSortingBinTray 狀態機

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#dosortingbintray-狀態機)

### 初始化 (iFlag==0)

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#初始化-iflag0)

### 主流程

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#主流程)

## SortingBinTray_ToTrayPickIC 狀態機

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#sortingbintray_totraypickic-狀態機)

## SortingBinTray_ToTrayPlaceIC 狀態機

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#sortingbintray_totrayplaceic-狀態機)

## DoFix3FullTray 流程

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#dofix3fulltray-流程)

## SortingBinTray_IsBinTrayNeedToSorting 判斷邏輯

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#sortingbintray_isbintrayneedtosorting-判斷邏輯)

## 吸嘴使用

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#吸嘴使用)

## 互鎖與安全

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#互鎖與安全)

## Offset 設定

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#offset-設定)

## 相關 Alarm

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#相關-alarm)

## 客戶啟用清單 (V899)

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#客戶啟用清單-v899)

## 潛在風險點

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#潛在風險點)

## 關鍵原始檔清單

[讀取此節](../hpi-tray-flow/references/sorting/original-entry.md#關鍵原始檔清單)
