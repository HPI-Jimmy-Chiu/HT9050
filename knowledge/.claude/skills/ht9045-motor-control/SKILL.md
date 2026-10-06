---
name: ht9045-motor-control
description: HT9045 Handler 馬達控制層模式。適用於馬達移動、回原點、JOG 點動、伺服控制或底層馬達卡操作。涵蓋 HTMotor、TMyMotor、TTrayMotor、TMySYNTEKMotor、TMyMN200Motor、TMyEtherCatMotor、TMyGALILMotor、TMySMCMotor、Hontech_M4 及 Galil DMC 整合。用於馬達修改、馬達問題除錯、新增馬達定義或理解馬達控制流程。觸發關鍵字：motor, 馬達, Galil, MN200, SYNTEK, PCI-L132, 回原點, home, JOG, servo, PISO-MN200, MotionNet, 泓格, ICP-DAS, mn_fix_move, mn_velocity_move, mn_home_start, EtherCAT, PCI1203, Advantech, SMC, CONTEC, Hontech, M2X4, SortArm；馬達驅動器手冊參照：Panasonic MINAS A4／A5 RS232（rs232.cpp，Index Z 扭力）、Panasonic MINAS A6BN EtherCAT、安川 Σ-X EtherCAT（HT9050，6077h／警報與重置）
---

# ht9045-motor-control 相容入口

同主題已整合到 [hpi-motor-control](../hpi-motor-control/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-motor-control/references/control/original-entry.md)

## 相關技能與代理

[讀取此節](../hpi-motor-control/references/control/original-entry/01.md#相關技能與代理)

## 快速參考

[讀取此節](../hpi-motor-control/references/control/original-entry/02.md#快速參考)

## 馬達類型常數

[讀取此節](../hpi-motor-control/references/control/original-entry/03.md#馬達類型常數)

## 控制卡整合

[讀取此節](../hpi-motor-control/references/control/original-entry/04.md#控制卡整合)

## 類別繼承架構

[讀取此節](../hpi-motor-control/references/control/original-entry/05.md#類別繼承架構)

## 全域馬達陣列

[讀取此節](../hpi-motor-control/references/control/original-entry/06.md#全域馬達陣列)

## 核心使用模式

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#核心使用模式)

### 馬達移動

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#馬達移動)

### 回原點

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#回原點)

### JOG 點動

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#jog-點動)

### 伺服控制

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#伺服控制)

### 位置讀取

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#位置讀取)

### 雙 Z 軸協調（Index Arm）

[讀取此節](../hpi-motor-control/references/control/original-entry/07.md#雙-z-軸協調index-arm)

## Galil DMC 控制卡

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#galil-dmc-控制卡)

### 系統配置

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#系統配置)

### Galil 命令介面

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#galil-命令介面)

### 常用 Galil 命令

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#常用-galil-命令)

### Galil 軸名稱對應

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#galil-軸名稱對應)

### 開啟/關閉 Galil 卡片

[讀取此節](../hpi-motor-control/references/control/original-entry/08.md#開啟關閉-galil-卡片)

## 安全機制

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#安全機制)

### 安全門檢查

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#安全門檢查)

### Z 軸安全位置

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#z-軸安全位置)

### 軟體極限

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#軟體極限)

### 編碼器容許誤差

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#編碼器容許誤差)

### 常設規則：閘與防護看「馬達類別的狀態」，不看機型、軸卡或「表格要不要 1203」（Steven 1005 23:2x）

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#常設規則閘與防護看馬達類別的狀態不看機型軸卡或表格要不要-1203steven-1005-232x)

### 常設規則：Index 軸卡別（Steven 1006 Q130）

[讀取此節](../hpi-motor-control/references/control/original-entry/09.md#常設規則index-軸卡別steven-1006-q130)

## 馬達移動回傳值

[讀取此節](../hpi-motor-control/references/control/original-entry/10.md#馬達移動回傳值)

## TTrayMotor 額外功能

[讀取此節](../hpi-motor-control/references/control/original-entry/11.md#ttraymotor-額外功能)

## 詳細參考

[讀取此節](../hpi-motor-control/references/control/original-entry/12.md#詳細參考)
