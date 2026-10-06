---
name: ht9045-html-version
description: >
  HT9045/HT9011UC Handler 主程式（BCB6 VCL）畫面的 HTML 互動模擬專案（HTML Version）。
  涵蓋 background.html 視窗管理器、page/*.html 各 dfm 轉換頁、hwidgets 元件模板、theme.css 佈景、
  _gen_dfm_abs 產生器、ScreenShots.html 截圖索引、版面拖曳模式、i18n。
  觸發關鍵字：HTML version, HTML 畫面, 畫面模擬, background.html, release.html, debug.html,
  hwidgets, WidgetTemplates, ComponentMap, dfm 轉 HTML, _gen_dfm_abs, theme.css, SHOT_MAP,
  layoutEdit, htPropPanel, HTLAYOUT_BASE, htLoader, HT9045_Release.cmd, HT9045_Debug.cmd
---

# ht9045-html-version 相容入口

同主題已整合到 [hpi-web-hmi](../hpi-web-hmi/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-web-hmi/references/pages/original-entry.md)

## 觸發關鍵字

[讀取此節](../hpi-web-hmi/references/pages/original-entry/01.md#觸發關鍵字)

## 產出位置

[讀取此節](../hpi-web-hmi/references/pages/original-entry/02.md#產出位置)

## 核心原則

[讀取此節](../hpi-web-hmi/references/pages/original-entry/03.md#核心原則)

## scripts/（產生器與工具）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/04.md#scripts產生器與工具)

## references/（詳細規則）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/05.md#references詳細規則)

## ⛔ CPP 模式（使用者 20260915 裁決）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/06.md#-cpp-模式使用者-20260915-裁決)

## Setup 頁的配方接線與小鍵盤（20260914/15 落地）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#setup-頁的配方接線與小鍵盤2026091415-落地)

### 執行期 tag（唯讀顯示）——20260916 新增的第五種形狀

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#執行期-tag唯讀顯示20260916-新增的第五種形狀)

### 盤點「一頁到底接了沒」—— 先對現成的表，不要自己另算（20260922）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#盤點一頁到底接了沒-先對現成的表不要自己另算20260922)

#### dfm 事件盤點工具 `scratchpad/audit_dfm_events.py`

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#dfm-事件盤點工具-scratchpadaudit_dfm_eventspy)

### 第三種形狀：行為翻譯層（golden handler → 瀏覽器）—— 20260921 新增

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#第三種形狀行為翻譯層golden-handler--瀏覽器-20260921-新增)

#### 不停機告警 NonStop（20260922）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#不停機告警-nonstop20260922)

#### 權限：levelset 有管道，目前登入者的等級沒有（20260922）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/07.md#權限levelset-有管道目前登入者的等級沒有20260922)

## `/api/system/` 與 `/api/text/` —— 機台檔案的即時讀寫（20260915 實測）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/08.md#apisystem-與-apitext--機台檔案的即時讀寫20260915-實測)

### `/api/system/` 的 40 支（35 支本機實檔存在，5 支此環境未部署）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/08.md#apisystem-的-40-支35-支本機實檔存在5-支此環境未部署)

### `/api/text/` 的 6 個來源（唯讀）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/08.md#apitext-的-6-個來源唯讀)

### 五條設計約束，改這段程式前先讀

[讀取此節](../hpi-web-hmi/references/pages/original-entry/08.md#五條設計約束改這段程式前先讀)

## 輸入途徑規格（20260915 定案，不可違反）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/09.md#輸入途徑規格20260915-定案不可違反)

## 已知陷阱（速查）

[讀取此節](../hpi-web-hmi/references/pages/original-entry/10.md#已知陷阱速查)
