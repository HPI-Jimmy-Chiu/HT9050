---
name: ht9045-autostart-flow
description: "HT9045 Auto Start 通訊流程知識庫。當使用者詢問 AutoStart、Auto Start 指令、GTK Loader Handshake、End Lot、Start Lot、Safety Interlock、SOT/EOT 信號、HTSET 700/702/333、批次自動啟動、遠端啟動 Handler、Info Mismatch 驗證、Setting OK 回傳、Agent 協調流程、One Cycle 模式等相關問題時，應先載入此技能以理解 AutoStart 完整通訊流程。關鍵字：AutoStart, Auto Start, GTK Loader, Handshake, End Lot, Start Lot, HTSET 333, HTSET 700, HTSET 702, Safety Interlock, SOT, EOT, MO, Flow, Ticket, Info Mismatch, Setting OK, Agent, Coordinator"
---

# ht9045-autostart-flow 相容入口

同主題已整合到 [hpi-autostart-autoclean](../hpi-autostart-autoclean/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-autostart-autoclean/references/autostart/original-entry.md)

## 系統角色速覽

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#系統角色速覽)

## 五階段流程總覽

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#五階段流程總覽)

### 階段一：握手（Handshake）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#階段一握手handshake)

### 階段二：清除批次（End Lot）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#階段二清除批次end-lot)

### 階段三：啟動批次（Start Lot）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#階段三啟動批次start-lot)

### 階段四：解除鎖定並啟動（Auto Start）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#階段四解除鎖定並啟動auto-start)

### 階段五：測試信號（SOT / EOT）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#階段五測試信號sot--eot)

## HT9045 OLP 實際指令對應表

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#ht9045-olp-實際指令對應表)

## Safety Interlock 實作（bLockByServer）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#safety-interlock-實作blockbyserver)

## Auto Start 條件（START_REQUEST）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#auto-start-條件start_request)

## 關鍵驗證邏輯（Info Mismatch）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#關鍵驗證邏輯info-mismatch)

## 錯誤處理決策表

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#錯誤處理決策表)

## OLP 通訊協定格式

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#olp-通訊協定格式)

## 錯誤處理速查

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#錯誤處理速查)

## 待確認事項（高優先）

[讀取此節](../hpi-autostart-autoclean/references/autostart/original-entry.md#待確認事項高優先)
