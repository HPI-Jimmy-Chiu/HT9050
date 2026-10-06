---
name: ht9045-qamode
description: >
  HT9045 QA Mode（品保抽測模式）知識庫。涵蓋核心三階段流程、Run Type 4 種行為、
  iQAModeBin Bin Routing、已知陷阱與修正歷程。
  UI 欄位詳細對照 → references/qamode-ui-fields.md
  InArm 變體/SiteMap/程式碼座標 → references/qamode-inarm-variants.md
  階段 C 深入分析 → references/qamode-stage-c-analysis.md
  觸發關鍵字：QA Mode, QA Sampling, iQAModeLoaderCT, iQAModeCount, iQAModeRunType,
  iQAModeBin, bQAModeQuickCleanOut, bQAModeFinishCleanOut, rsmQAMode,
  Check_QA_ModeCount, QABackupStatus, Untest Bin, QA 抽測, QA 停測.
applyTo: "**/QAMode.cpp, **/QAMode.h, **/ainarm2.cpp, **/ainarm9045*.cpp, **/csystem.cpp, **/cinitial.cpp, **/main.cpp"
---

# ht9045-qamode 相容入口

同主題已整合到 [hpi-lotinfo-recipe](../hpi-lotinfo-recipe/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-lotinfo-recipe/references/qa/original-entry.md)

## 1. 模式定位

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#1-模式定位)

## 2. iQAModeRunType（做完後動作）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#2-iqamoderuntype做完後動作)

## 3. 核心三階段判斷（`Check_QA_ModeCount()` @ ainarm2.cpp:155）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#3-核心三階段判斷check_qa_modecount--ainarm2cpp155)

## 4. Bin Routing 機制（main.cpp:16677）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#4-bin-routing-機制maincpp16677)

### iQAModeBin 生效條件

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#iqamodebin-生效條件)

### OffT Guard（main.cpp:889 & 1267）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#offt-guardmaincpp889--1267)

## 5. 已知陷阱（DO NOT MISS）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#5-已知陷阱do-not-miss)

### 5.1 多 Site 模式下 `==` 判斷會被跳過（已修正 904.2）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#51-多-site-模式下--判斷會被跳過已修正-9042)

### 5.2 `iQAModeBin` 是 0-based ComboBox index

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#52-iqamodebin-是-0-based-combobox-index)

### 5.3 ART + QA 混用注意

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#53-art--qa-混用注意)

## 6. 修正歷程

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#6-修正歷程)

### 6.1 V3.33.904.2 — QA Mode 32-Site Stop Fix（2026-05-11）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#61-v3339042--qa-mode-32-site-stop-fix2026-05-11)

### 6.2 V3.33.904.3 — QA Mode CleanOut Bin Routing Fix（2026-05-14）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#62-v3339043--qa-mode-cleanout-bin-routing-fix2026-05-14)

### 6.3 V3.33.904.4 — QA Mode Tester Mode Restore Fix（2026-05-19）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#63-v3339044--qa-mode-tester-mode-restore-fix2026-05-19)

### 6.4 V3.33.904.5 — QA Mode V904.4 Regression Fix（2026-05-21）

[讀取此節](../hpi-lotinfo-recipe/references/qa/original-entry.md#64-v3339045--qa-mode-v9044-regression-fix2026-05-21)
