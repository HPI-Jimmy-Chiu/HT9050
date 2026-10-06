---
name: ht9045-contact-pick-interlock
description: HT9045 Contact 模式「取料資料流 + 模式互鎖」深度知識庫。當使用者詢問 Contact test 切不回、切不換模式、Please finish ONE CYCLE、機台有IC不能切換模式、DoZ1PickFromShuttle、DoZ2PickFromShuttle、case 3050 掉料防護、bContactTestICDropGuard、IndexHasIC、ShuttleHasIC、TestSocketHasIC、TestHeadHasIC、FTestSuck/BTestSuck/FLCarryKit/BLCarryKit 殘留、SetItemData vs MoveSuckData、iFTestBackItem/iBTestBackItem 還原、Contact 掉料中止、單臂/雙臂部分上料誤判掉料、HAS_IC 守護門檻、客戶碼隔離掉料防護等相關問題時，應先載入此技能。關鍵字：Contact 切不回, Please finish ONE CYCLE, 機台有IC不能切換模式, DoZ1PickFromShuttle, DoZ2PickFromShuttle, case 3050, bContactTestICDropGuard, IndexHasIC, ShuttleHasIC, TestSocketHasIC, FTestSuck, BTestSuck, FLCarryKit, BLCarryKit, SetItemData, MoveSuckData, iFTestBackItem, iBTestBackItem, HAS_IC, NULL_IC, 掉料防護, 部分上料, 客戶碼隔離, CC_GIGAS。
---

# ht9045-contact-pick-interlock 相容入口

> 同主題已整合到 [hpi-index-flow](../hpi-index-flow/SKILL.md)，先由共同流程與機型差異選路。原觸發詞與完整正文保留。

- [原版詳細內容](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md)

## 適用場景

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#適用場景)

## 核心檔案

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#核心檔案)

## 1. IC 資料容器模型（Contact 情境）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#1-ic-資料容器模型contact-情境)

### SetItemData vs MoveSuckData（關鍵差異）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#setitemdata-vs-movesuckdata關鍵差異)

## 2. 模式互鎖鏈（為什麼切不回 Contact）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#2-模式互鎖鏈為什麼切不回-contact)

### 兩個互鎖點（FACT, cContact.cpp）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#兩個互鎖點fact-ccontactcpp)

### 互鎖看的資料來源（FACT, csystem.cpp #815-895）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#互鎖看的資料來源fact-csystemcpp-815-895)

### 「想切模式被擋」反查表（從現象 → 查哪個容器）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#想切模式被擋反查表從現象--查哪個容器)

## 3. DoZ1/Z2PickFromShuttle 取料狀態機（重點 case）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#3-doz1z2pickfromshuttle-取料狀態機重點-case)

### case 3050 寫資料邏輯（FACT）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#case-3050-寫資料邏輯fact)

## 4. 掉料防護 bContactTestICDropGuard（CASE-20260608-001 全智）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#4-掉料防護-bcontacttesticdropguardcase-20260608-001-全智)

### 開關與豁免

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#開關與豁免)

### 守護門檻（V899 現行邏輯）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#守護門檻v899-現行邏輯)

### 掉料中止前的還原（問題 b 修正）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#掉料中止前的還原問題-b-修正)

## 5. 客戶碼隔離雙層閘門範式（可重用）

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#5-客戶碼隔離雙層閘門範式可重用)

## 6. 排查 SOP：Contact 掉料後切不回 Contact test

[讀取此節](../hpi-index-flow/references/history/ht9045-contact-pick-interlock/index.md#6-排查-sopcontact-掉料後切不回-contact-test)
