---
name: ht9045-motionview-html-ui
description: 用 HTML 做出可取代實機 Motion View 的頁面（HT9045 / HT9046 Handler）。參數不寫死：頁面 runtime 直接讀機台的 Gerneral.ini / teach.ini / Tray.Data / HotPlate.Data / TestMode.Data / ArmCondition.Data，連站點與行程幾何都從 teach.ini 推導，再依真實程式碼的配位與互鎖規則跑模擬（SIM）；另有 LIVE 模式讀 StateRecord 的 MainFormSnapshot.txt / Motor.xls / HP*_*.xls，直接顯示實機當下的各軸位置、各 Task 與各處 IC 在籍。當使用者要求「取代 Motion View / 機台動作模擬畫面 / 整機跑料動畫 / 吸嘴取放料順序 / HotPlate 配位視覺化 / 看實機當下狀態 / 給客戶解釋機台動作的 HTML」，或要換機台換 recipe、加模組、排查配位與互鎖缺陷時使用。
---

# ht9045-motionview-html-ui 相容入口

同主題已整合到 [hpi-motionview](../hpi-motionview/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-motionview/references/template/original-entry.md)

## Overview

[讀取此節](../hpi-motionview/references/template/original-entry.md#overview)

## HT9045 HTML 部署規則（2026-09-08）

[讀取此節](../hpi-motionview/references/template/original-entry.md#ht9045-html-部署規則2026-09-08)

## 快速開始

[讀取此節](../hpi-motionview/references/template/original-entry.md#快速開始)

## 工作流程

[讀取此節](../hpi-motionview/references/template/original-entry.md#工作流程)

### 步驟 1 — 參數層：頁面自己讀，不要把值烤進頁面

[讀取此節](../hpi-motionview/references/template/original-entry.md#步驟-1--參數層頁面自己讀不要把值烤進頁面)

### 步驟 2 — 決定要畫哪些模組

[讀取此節](../hpi-motionview/references/template/original-entry.md#步驟-2--決定要畫哪些模組)

### 步驟 3 — 配位與取放料規則

[讀取此節](../hpi-motionview/references/template/original-entry.md#步驟-3--配位與取放料規則)

### 步驟 4 — 顯示語意要與實機一致

[讀取此節](../hpi-motionview/references/template/original-entry.md#步驟-4--顯示語意要與實機一致)

### 步驟 5 — 驗證（不可省）

[讀取此節](../hpi-motionview/references/template/original-entry.md#步驟-5--驗證不可省)

## 架構約束（改模板時務必遵守）

[讀取此節](../hpi-motionview/references/template/original-entry.md#架構約束改模板時務必遵守)

## 兩種模式

[讀取此節](../hpi-motionview/references/template/original-entry.md#兩種模式)

## 常見需求對應

[讀取此節](../hpi-motionview/references/template/original-entry.md#常見需求對應)

## Resources

[讀取此節](../hpi-motionview/references/template/original-entry.md#resources)
