# repo 版 SKILL.md 正文（合併前原樣保留）

按需要選取以下章節，原文依順序保留。

- [repo 版 SKILL.md 正文（合併前原樣保留）](colleague-skill-body-20260915/00.md)
- [1. 結構概覽](colleague-skill-body-20260915/01.md)
- [2. 群組分類總表](colleague-skill-body-20260915/02.md)
- [3. 讀寫流程](colleague-skill-body-20260915/03.md)
- [4. 新增欄位標準流程](colleague-skill-body-20260915/04.md)
- [5. 常見查詢入口](colleague-skill-body-20260915/05.md)
- [6. 重要技術限制](colleague-skill-body-20260915/06.md)
- [7. UI 元件放置與命名慣例](colleague-skill-body-20260915/07.md)
- [8. 參考來源](colleague-skill-body-20260915/08.md)
- [12. 資料源與多語客戶手冊（YAML + i18n）](colleague-skill-body-20260915/09.md)
- [web 這側：`config.ini` 已可由瀏覽器讀寫（20260915）](colleague-skill-body-20260915/10.md)

# repo 版 SKILL.md 正文（合併前原樣保留）

[讀取此節](colleague-skill-body-20260915/00.md#repo-版-skillmd-正文合併前原樣保留)

# HT9045 機台設定模組（HT9045_CONFIG）

[讀取此節](colleague-skill-body-20260915/00.md#ht9045-機台設定模組ht9045_config)

## 1. 結構概覽

[讀取此節](colleague-skill-body-20260915/01.md#1-結構概覽)

## 2. 群組分類總表

[讀取此節](colleague-skill-body-20260915/02.md#2-群組分類總表)

## 3. 讀寫流程

[讀取此節](colleague-skill-body-20260915/03.md#3-讀寫流程)

### 3.1 初始化流程（`TfConfiguration::TfConfiguration`）

[讀取此節](colleague-skill-body-20260915/03.md#31-初始化流程tfconfigurationtfconfiguration)

### 3.2 HTEditList 元件綁定（`InitConfigEdtList_Item[A~P]`）

[讀取此節](colleague-skill-body-20260915/03.md#32-hteditlist-元件綁定initconfigedtlist_itemap)

### 3.3 Lock by File（`ReadLockByFile` / `ChangeCBListProperty`）

[讀取此節](colleague-skill-body-20260915/03.md#33-lock-by-filereadlockbyfile--changecblistproperty)

### 3.4 儲存 / 載入

[讀取此節](colleague-skill-body-20260915/03.md#34-儲存--載入)

### 3.5 新增功能開關的兩種模式比較

[讀取此節](colleague-skill-body-20260915/03.md#35-新增功能開關的兩種模式比較)

## 4. 新增欄位標準流程

[讀取此節](colleague-skill-body-20260915/04.md#4-新增欄位標準流程)

### Step 1：`Config.h` 中加欄位

[讀取此節](colleague-skill-body-20260915/04.md#step-1configh-中加欄位)

### Step 2：`cConfiguration.cpp` 的 `InitConfigEdtList_ItemN()` 中加綁定

[讀取此節](colleague-skill-body-20260915/04.md#step-2cconfigurationcpp-的-initconfigedtlist_itemn-中加綁定)

### Step 3（選用）：需要 Lock by File 時

[讀取此節](colleague-skill-body-20260915/04.md#step-3選用需要-lock-by-file-時)

### Step 4：讀取預設值

[讀取此節](colleague-skill-body-20260915/04.md#step-4讀取預設值)

## 5. 常見查詢入口

[讀取此節](colleague-skill-body-20260915/05.md#5-常見查詢入口)

### 按需求找群組

[讀取此節](colleague-skill-body-20260915/05.md#按需求找群組)

### INI Key 格式

[讀取此節](colleague-skill-body-20260915/05.md#ini-key-格式)

## 6. 重要技術限制

[讀取此節](colleague-skill-body-20260915/06.md#6-重要技術限制)

## 7. UI 元件放置與命名慣例

[讀取此節](colleague-skill-body-20260915/07.md#7-ui-元件放置與命名慣例)

## 8. 參考來源

[讀取此節](colleague-skill-body-20260915/08.md#8-參考來源)

### 外部欄位參考（references/）

[讀取此節](colleague-skill-body-20260915/08.md#外部欄位參考references)

## 12. 資料源與多語客戶手冊（YAML + i18n）

[讀取此節](colleague-skill-body-20260915/09.md#12-資料源與多語客戶手冊yaml--i18n)

### 12.1 目錄結構

[讀取此節](colleague-skill-body-20260915/09.md#121-目錄結構)

### 12.2 新增 / 修改欄位流程

[讀取此節](colleague-skill-body-20260915/09.md#122-新增--修改欄位流程)

### 12.3 Audience 過濾規則

[讀取此節](colleague-skill-body-20260915/09.md#123-audience-過濾規則)

### 12.4 支援語系

[讀取此節](colleague-skill-body-20260915/09.md#124-支援語系)

### 12.5 詳細 Schema

[讀取此節](colleague-skill-body-20260915/09.md#125-詳細-schema)

## web 這側：`config.ini` 已可由瀏覽器讀寫（20260915）

[讀取此節](colleague-skill-body-20260915/10.md#web-這側configini-已可由瀏覽器讀寫20260915)
