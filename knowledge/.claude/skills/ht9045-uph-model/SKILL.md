---
name: ht9045-uph-model
description: >
  HT9045／HT9046／HT9011UC 系列 IC Test Handler 離線 UPH 解析式計算與馬達速度校正表（UPH_MotorProfiler）知識庫。
  Use when: 估算客戶條件（Tray X×Y、Soak、Test Time、2D on/off、Speed %）下的 UPH、分析實機 UPH 低於規格、
  HotPlate Soak／Index 平行／Tray Arm 攤提、用 Mot_Table 速度反推每段 cycle time、UPH_MotorProfiler 用法與記錄檔欄位。
  關鍵字：UPH, 產能, Throughput, Cycle Time, Soak Time, Test Time, Tray Form, Motor Speed, MotorProfiler, Mot_Table, Speed 校正。
---

# ht9045-uph-model 相容入口

同主題已整合到 [hpi-uph-model](../hpi-uph-model/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-uph-model/references/legacy/original-entry.md)

## 觸發關鍵字

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#觸發關鍵字)

## 適用版本

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#適用版本)

## 1. 機台既有 UPH 計算（Per-Tray Real-Time）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#1-機台既有-uph-計算per-tray-real-time)

## 2. 三大平行模型（解析式預測 — 核心觀念）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#2-三大平行模型解析式預測--核心觀念)

### 規則 1：HotPlate Soak 與 InArm 平行

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#規則-1hotplate-soak-與-inarm-平行)

### 規則 2：InArm vs Index+Test vs OutArm 三段平行（主瓶頸）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#規則-2inarm-vs-indextest-vs-outarm-三段平行主瓶頸)

### 規則 3：Tray Arm 補退盤攤提

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#規則-3tray-arm-補退盤攤提)

### 最終 UPH

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#最終-uph)

## 3. 11 大時間項定義

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#3-11-大時間項定義)

## 4. 馬達速度校正表（UPH_MotorProfiler — 三方混合架構）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#4-馬達速度校正表uph_motorprofiler--三方混合架構)

### 目的

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#目的)

### 架構：埋點 + uMotorTest 補位 + Excel 計算

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#架構埋點--umotortest-補位--excel-計算)

### 實作優先順序（建議）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#實作優先順序建議)

### 校正策略：Teach 座標 + 機台固定尺寸導出距離（適用 Phase 1B 補位）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#校正策略teach-座標--機台固定尺寸導出距離適用-phase-1b-補位)

### Phase 1A 埋點位置（~15~25 個呼叫點）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#phase-1a-埋點位置1525-個呼叫點)

### Speed + Acc 掃描（Phase 1B 補位用）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#speed--acc-掃描phase-1b-補位用)

### 馬達清單（43 顆，依 Mot_Table.csv）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#馬達清單43-顆依-mot_tablecsv)

## 5. 編譯開關與自動歸零

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#5-編譯開關與自動歸零)

### 不需要 `#define`

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#不需要-define)

### 自動歸零（Auto Home）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#自動歸零auto-home)

### 安全門中斷 → 重新初始化

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#安全門中斷--重新初始化)

## 6. 使用流程（FAQ）

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#6-使用流程faq)

### Q1：客戶要算 UPH，從哪裡開始？

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#q1客戶要算-uph從哪裡開始)

### Q2：實機跑出來與公式差很多怎辦？

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#q2實機跑出來與公式差很多怎辦)

### Q3：MotorProfiler 怎麼啟用？

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#q3motorprofiler-怎麼啟用)

## 7. 安全注意事項

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#7-安全注意事項)

## References

[讀取此節](../hpi-uph-model/references/legacy/original-entry.md#references)
