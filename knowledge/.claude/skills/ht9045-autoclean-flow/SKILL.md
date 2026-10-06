---
name: ht9045-autoclean-flow
description: HT9045 IC Test Handler Auto Clean（自動清潔 Socket）流程知識庫。當使用者詢問 AutoClean、DoAutoCleanKit、DoAutoCleanPickfromCleanKit、DoAutoCleanPlaceToCleanKit、DoShuttle1AutoClean、DoShuttle2AutoClean、DoIndexAutoClean、Clean Kit、Clean Pad、CleanAir、bRunAutoClean、EnableAutoclean、IntervalContact、Smart Auto Clean、iAutoClean_Function、iAutoClean_ContactCount、iAutoClean_ContactMode、WAR16102、WAR16103、JAM0110、JAM0312、JAM0314、bChangeCleanPad、bAutoCleanCheckOpenDoor、eCKPos、HAS_CLEAN_IC、HAS_NULL_CLEAN_IC、Auto Clean Hang Up、Clean Pad 更換、低良率清潔、PopAutoClean、CleanAir 模式等相關問題時，應先載入此技能以理解 Auto Clean 完整流程。關鍵字：DoAutoCleanKit, AutoClean, Clean Kit, Clean Pad, bRunAutoClean, iAutoClean_Function, IntervalContact, SmartAutoClean, DoIndexAutoClean, DoShuttle1AutoClean, EnableAutoclean, CleanAir, eCKPos, HAS_CLEAN_IC, HAS_NULL_CLEAN_IC, WAR16102, JAM0110, JAM0312, JAM0314。
---

# ht9045-autoclean-flow 相容入口

同主題已整合到 [hpi-autostart-autoclean](../hpi-autostart-autoclean/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-autostart-autoclean/references/autoclean/original-entry.md)

## 適用場景

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/01.md#適用場景)

## 專案資訊

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/02.md#專案資訊)

## 關鍵原始檔

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/03.md#關鍵原始檔)

## 1. Auto Clean 概述

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/04.md#1-auto-clean-概述)

### 清潔模式 (eCKPos)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/04.md#清潔模式-eckpos)

### IC 狀態常數（AutoClean 專用）

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/04.md#ic-狀態常數autoclean-專用)

## 2. 觸發機制

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#2-觸發機制)

### 觸發模式位元 (iAutoClean_Mode)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#觸發模式位元-iautoclean_mode)

### 啟用條件（全部需滿足）

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#啟用條件全部需滿足)

### 手動觸發（M_MANUAL）：Cleaning 頁「Clean」鈕

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#手動觸發m_manualcleaning-頁clean鈕)

### Interval 觸發（最常用）

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#interval-觸發最常用)

### Smart Auto Clean（自適應）

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/05.md#smart-auto-clean自適應)

## 3. 呼叫層級總覽

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/06.md#3-呼叫層級總覽)

## 4. DoAutoCleanKit 主流程摘要

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/07.md#4-doautocleankit-主流程摘要)

## 5. Index AutoClean 接觸循環

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#5-index-autoclean-接觸循環)

### Phase 1: Setup (Task 1-200)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#phase-1-setup-task-1-200)

### Phase 2: Pick from Shuttle (Task 300-700)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#phase-2-pick-from-shuttle-task-300-700)

### Phase 3: Contact 接觸循環 (Task 800-1400)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#phase-3-contact-接觸循環-task-800-1400)

### Phase 4: Release (Task 1500-1900)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#phase-4-release-task-1500-1900)

### Phase 5: Arm2 (Task 2100-3900)

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/08.md#phase-5-arm2-task-2100-3900)

## 6. Shuttle AutoClean 定位

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/09.md#6-shuttle-autoclean-定位)

## 7. Clean Pad 管理

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/10.md#7-clean-pad-管理)

### 取料位置類型

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/10.md#取料位置類型)

### 計數追蹤

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/10.md#計數追蹤)

### Clean Pad 更換

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/10.md#clean-pad-更換)

## 8. InArm 與 AutoClean 互動

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/11.md#8-inarm-與-autoclean-互動)

### 吸嘴資料映射

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/11.md#吸嘴資料映射)

### 放料至 Shuttle 座標偏移

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/11.md#放料至-shuttle-座標偏移)

### bAutoCleanPlaceToSht 邏輯

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/11.md#bautocleanplacetosht-邏輯)

## 9. 設定參數一覽

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/12.md#9-設定參數一覽)

## 10. CosFunction 旗標

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/13.md#10-cosfunction-旗標)

## 11. Alarm 與 Error Code

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/14.md#11-alarm-與-error-code)

## 12. 與 OneCycle 互動

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/15.md#12-與-onecycle-互動)

## 13. SECS/GEM 事件

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/16.md#13-secsgem-事件)

## 深入參考

[讀取此節](../hpi-autostart-autoclean/references/autoclean/original-entry/17.md#深入參考)
