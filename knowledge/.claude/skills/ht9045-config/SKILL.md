---
name: ht9045-config
description: >
  HT9045 機台設定模組知識庫（HT9045_CONFIG / IniConfig）。當使用者詢問 IniConfig 欄位定義、
  Config.h 結構、cConfiguration.cpp 讀寫流程、功能字母分組（A/B/C/D/E/F/G/I/L/M/N/O/P）、
  Lock by File 機制、InitConfigEdtList 元件對應、CheckConfigurationBeforeSave 驗證邏輯，
  或新增 / 修改 IniConfig 功能開關時，應先載入此技能。
  關鍵字：IniConfig, HT9045_CONFIG, Config.h, cConfiguration, InitConfigEdtList,
  ReadLastSetIni, Lock by File, bLockByFile, HTEditList, SaveConfiguration,
  CheckConfigurationBeforeSave, ReadConfigStandard, elConfig_byRecipe, configByRecipe.ini,
  SetFontBlue, ReadConfigByRecipe, LifeTimeCount, bLifeTimeCount, HeadContactCount,
  ContactSet, bUseHeadContactCount, O12, O13, O14, 銦片, Indium, SLK, 測試頭, by arm,
  HeadContactCountHistory, SocketContactSet, SocketContactCount, iContactAlarmCount,
  sgHeadCondition, sbHeadCondition1Save, editContactCountAlarm, CheckContactOver,
  ProcessHeadContactCount, CC_XINYUN, 芯云, 銦片設定失效,
  A01, A32, B01, C01, D41, E30,
  F06, G01, I21, L11, M01, N06, N14, O06, P06, bEnable_SECS_GEM, bSPILFunction,
  bMaximFunction, bSIGURDFunction, bVTESTFunction, bKoreaFunction, bSingaporeFunction。
  另含 **teach.ini 升版陷阱**（教點資料檔）：teach.ini, tech.dat, MInShutte1, MInShuttle1,
  MInShutte2, MInShuttle2, TECH_PARA, ReadFromFile, SaveToFile, TfTeach::ReadFile,
  CheckSectionExist, CheckAndReadIniData 寫入副作用, MOT[].Alias, SetAlias, Update2,
  ReadTechData, Tech.OutSH1ZDetectPos, Tech.iInShuttle1Left, Tech.iInShuttle1Right,
  SThreadPara.base_pos, Prod.InSHT, 升版後教點跑掉, 升版後位置偏移,
  in shuttle sensor 偵測點位偏移, 教點變 0, sizeof(TECH), 降版相容, 教點 migration。
---

# ht9045-config 相容入口

同主題已整合到 [hpi-config](../hpi-config/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-config/references/config/original-entry.md)

## 1. 結構概覽

[讀取此節](../hpi-config/references/config/original-entry/01.md#1-結構概覽)

## 2. 群組分類總表

[讀取此節](../hpi-config/references/config/original-entry/02.md#2-群組分類總表)

### ⚠ 高風險旗標：`[P04] bP04ColorIsEmptyUnloader`（改錯會造成無報警靜默停機）

[讀取此節](../hpi-config/references/config/original-entry/02.md#-高風險旗標p04-bp04colorisemptyunloader改錯會造成無報警靜默停機)

## 3. 讀寫流程

[讀取此節](../hpi-config/references/config/original-entry/03.md#3-讀寫流程)

### 3.1 初始化流程（`TfConfiguration::TfConfiguration`）

[讀取此節](../hpi-config/references/config/original-entry/03.md#31-初始化流程tfconfigurationtfconfiguration)

### 3.2 HTEditList 元件綁定（`InitConfigEdtList_Item[A~P]`）

[讀取此節](../hpi-config/references/config/original-entry/03.md#32-hteditlist-元件綁定initconfigedtlist_itemap)

### 3.3 elConfig_byRecipe — 跟隨 Recipe 的設定（藍色字體）

[讀取此節](../hpi-config/references/config/original-entry/03.md#33-elconfig_byrecipe--跟隨-recipe-的設定藍色字體)

#### elConfig vs elConfig_byRecipe 差異

[讀取此節](../hpi-config/references/config/original-entry/03.md#elconfig-vs-elconfig_byrecipe-差異)

#### 讀寫路徑

[讀取此節](../hpi-config/references/config/original-entry/03.md#讀寫路徑)

#### 使用方式

[讀取此節](../hpi-config/references/config/original-entry/03.md#使用方式)

#### 已使用 elConfig_byRecipe 的設定項

[讀取此節](../hpi-config/references/config/original-entry/03.md#已使用-elconfig_byrecipe-的設定項)

#### 注意事項

[讀取此節](../hpi-config/references/config/original-entry/03.md#注意事項)

### 3.4 Lock by File（`ReadLockByFile` / `ChangeCBListProperty`）

[讀取此節](../hpi-config/references/config/original-entry/03.md#34-lock-by-filereadlockbyfile--changecblistproperty)

### 3.4 儲存 / 載入

[讀取此節](../hpi-config/references/config/original-entry/03.md#34-儲存--載入)

### 3.5 新增功能開關的兩種模式比較

[讀取此節](../hpi-config/references/config/original-entry/03.md#35-新增功能開關的兩種模式比較)

## 4. 新增欄位標準流程

[讀取此節](../hpi-config/references/config/original-entry/04.md#4-新增欄位標準流程)

### Step 1：`Config.h` 中加欄位

[讀取此節](../hpi-config/references/config/original-entry/04.md#step-1configh-中加欄位)

### Step 2：`cConfiguration.cpp` 的 `InitConfigEdtList_ItemN()` 中加綁定

[讀取此節](../hpi-config/references/config/original-entry/04.md#step-2cconfigurationcpp-的-initconfigedtlist_itemn-中加綁定)

### Step 3（選用）：需要 Lock by File 時

[讀取此節](../hpi-config/references/config/original-entry/04.md#step-3選用需要-lock-by-file-時)

### Step 4：讀取預設值

[讀取此節](../hpi-config/references/config/original-entry/04.md#step-4讀取預設值)

## 5. 常見查詢入口

[讀取此節](../hpi-config/references/config/original-entry/05.md#5-常見查詢入口)

### 按需求找群組

[讀取此節](../hpi-config/references/config/original-entry/05.md#按需求找群組)

### INI Key 格式

[讀取此節](../hpi-config/references/config/original-entry/05.md#ini-key-格式)

## 6. LifeTimeCount（銦片壽命計數 O12/O13/O14）

[讀取此節](../hpi-config/references/config/original-entry/06.md#6-lifetimecount銦片壽命計數-o12o13o14)

### 6.1 概述

[讀取此節](../hpi-config/references/config/original-entry/06.md#61-概述)

### 6.2 相關變數

[讀取此節](../hpi-config/references/config/original-entry/06.md#62-相關變數)

### 6.3 客戶別差異

[讀取此節](../hpi-config/references/config/original-entry/06.md#63-客戶別差異)

### 6.3.1 客戶碼卡控安全準則（全域適用）

[讀取此節](../hpi-config/references/config/original-entry/06.md#631-客戶碼卡控安全準則全域適用)

#### ✅ 正確範例

[讀取此節](../hpi-config/references/config/original-entry/06.md#-正確範例)

#### ❌ 危險範例

[讀取此節](../hpi-config/references/config/original-entry/06.md#-危險範例)

#### 檢查清單（修改前必問）

[讀取此節](../hpi-config/references/config/original-entry/06.md#檢查清單修改前必問)

#### AccessLevel 對照表

[讀取此節](../hpi-config/references/config/original-entry/06.md#accesslevel-對照表)

### 6.4 InitConfigEdtList_ItemO 中的分支邏輯

[讀取此節](../hpi-config/references/config/original-entry/06.md#64-initconfigedtlist_itemo-中的分支邏輯)

### 6.5 VTEST 銦片 SPEC 跟隨 Recipe 的完整路徑

[讀取此節](../hpi-config/references/config/original-entry/06.md#65-vtest-銦片-spec-跟隨-recipe-的完整路徑)

### 6.6 相關告警

[讀取此節](../hpi-config/references/config/original-entry/06.md#66-相關告警)

### 6.7 Lot Start 互動（VTEST）

[讀取此節](../hpi-config/references/config/original-entry/06.md#67-lot-start-互動vtest)

### 6.8 銦片計數完整 Reference（跨客戶）

[讀取此節](../hpi-config/references/config/original-entry/06.md#68-銦片計數完整-reference跨客戶)

## 7. 重要技術限制

[讀取此節](../hpi-config/references/config/original-entry/07.md#7-重要技術限制)

## 8. UI 元件放置與命名慣例

[讀取此節](../hpi-config/references/config/original-entry/08.md#8-ui-元件放置與命名慣例)

## 9. 參考來源

[讀取此節](../hpi-config/references/config/original-entry/09.md#9-參考來源)

### 外部欄位參考（references/）

[讀取此節](../hpi-config/references/config/original-entry/09.md#外部欄位參考references)

## 12. 資料源與多語客戶手冊（YAML + i18n）

[讀取此節](../hpi-config/references/config/original-entry/10.md#12-資料源與多語客戶手冊yaml--i18n)

### 12.1 目錄結構

[讀取此節](../hpi-config/references/config/original-entry/10.md#121-目錄結構)

### 12.2 新增 / 修改欄位流程

[讀取此節](../hpi-config/references/config/original-entry/10.md#122-新增--修改欄位流程)

### 12.3 Audience 過濾規則

[讀取此節](../hpi-config/references/config/original-entry/10.md#123-audience-過濾規則)

### 12.4 支援語系

[讀取此節](../hpi-config/references/config/original-entry/10.md#124-支援語系)

### 12.5 詳細 Schema

[讀取此節](../hpi-config/references/config/original-entry/10.md#125-詳細-schema)

## web 這側：`config.ini` 已可由瀏覽器讀寫（20260915）

[讀取此節](../hpi-config/references/config/original-entry/11.md#web-這側configini-已可由瀏覽器讀寫20260915)

## 合併補充：repo 既有參考（20261001）

[讀取此節](../hpi-config/references/config/original-entry/12.md#合併補充repo-既有參考20261001)
