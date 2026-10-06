---
name: ht9050-motionview-layout
description: >
  HT9050（HP-9050）Motion View HTML 版面調整與對應。當要移動／新增／刪除機構模組方塊、
  改流程箭頭順序、換出料軌、綁定馬達軸、或把畫面對回 layout PDF 與實機教點時使用。
  涵蓋 page/Main.MotionView9050.html 的資料契約、JSON/MotionView9050-layout.json 欄位、
  版面編輯模式操作、機種切換鏈路（Machine-profile.json → settings.js → background.html）、
  以及 HTML-only 邊界（BCB6 無 HT9050，runtimeSupported:false）。
  觸發關鍵字：HT9050, HP-9050, MotionView9050, Main.MotionView9050.html,
  MotionView9050-layout.json, Machine-profile.json, 版面編輯, 匯出版面 JSON,
  機種切換, ?machine=, HT9050_Debug.cmd, debug9050.html, 模組方塊, 流程箭頭, 出料軌。
applyTo: "**/Main.MotionView9050.html, **/MotionView9050-layout.json, **/Machine-profile.json"
---

# ht9050-motionview-layout 相容入口

同主題已整合到 [hpi-motionview](../hpi-motionview/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-motionview/references/layout/original-entry.md)

## 兩個版本，兩份 reference

[讀取此節](../hpi-motionview/references/layout/original-entry/01.md#兩個版本兩份-reference)

## Scripts

[讀取此節](../hpi-motionview/references/layout/original-entry/02.md#scripts)

## 檔案地圖

[讀取此節](../hpi-motionview/references/layout/original-entry/03.md#檔案地圖)

## 1. 調版面的兩種方式

[讀取此節](../hpi-motionview/references/layout/original-entry/04.md#1-調版面的兩種方式)

### A. 頁面內拖曳（快、適合對圖）

[讀取此節](../hpi-motionview/references/layout/original-entry/04.md#a-頁面內拖曳快適合對圖)

### B. 直接改 JSON（精準、可 review）

[讀取此節](../hpi-motionview/references/layout/original-entry/04.md#b-直接改-json精準可-review)

## 2. `MotionView9050-layout.json` 欄位契約

[讀取此節](../hpi-motionview/references/layout/original-entry/05.md#2-motionview9050-layoutjson-欄位契約)

### `kind` → 底色對照

[讀取此節](../hpi-motionview/references/layout/original-entry/05.md#kind--底色對照)

### 標籤自動排版規則（`putLabel()`）

[讀取此節](../hpi-motionview/references/layout/original-entry/05.md#標籤自動排版規則putlabel)

### 編號徽章

[讀取此節](../hpi-motionview/references/layout/original-entry/05.md#編號徽章)

## 3. 流程（`flow`）

[讀取此節](../hpi-motionview/references/layout/original-entry/06.md#3-流程flow)

## 4. 軸綁定（`axes.bindings`）

[讀取此節](../hpi-motionview/references/layout/original-entry/07.md#4-軸綁定axesbindings)

## 4b. 行程與互鎖範圍（畫動畫／改流程前必讀）

[讀取此節](../hpi-motionview/references/layout/original-entry/08.md#4b-行程與互鎖範圍畫動畫改流程前必讀)

### Out Shuttle

[讀取此節](../hpi-motionview/references/layout/original-entry/08.md#out-shuttle)

### Out P&P

[讀取此節](../hpi-motionview/references/layout/original-entry/08.md#out-pp)

### 共用 DUT 停位互斥（p.24 In PUT／Out PUT）

[讀取此節](../hpi-motionview/references/layout/original-entry/08.md#共用-dut-停位互斥p24-in-putout-put)

### In P&P 補料節奏（HP 不可長時間空置）

[讀取此節](../hpi-motionview/references/layout/original-entry/08.md#in-pp-補料節奏hp-不可長時間空置)

## 5. 機種切換鏈路（要動這條才需要看）

[讀取此節](../hpi-motionview/references/layout/original-entry/09.md#5-機種切換鏈路要動這條才需要看)

## 6. 驗證清單（改完必跑）

[讀取此節](../hpi-motionview/references/layout/original-entry/10.md#6-驗證清單改完必跑)

## 7. 已知坑

[讀取此節](../hpi-motionview/references/layout/original-entry/11.md#7-已知坑)

## 8. 相關資源

[讀取此節](../hpi-motionview/references/layout/original-entry/12.md#8-相關資源)
