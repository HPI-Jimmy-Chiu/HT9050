---
name: ht9050-1203-homing
description: >-
  HT9050 / V906 PCI-1203 EtherCAT 回原點（homing）知識：回原點是「驅動器」做的（DS402 CiA402 method 24/28），
  不是卡片做的（SMC / golden MODE12）。用在修改、除錯、擴充單軸 HOME、全機 HOME、Light Scale HOME、
  新增驅動器型號、步進軸壓在原點上、回原點速度被拒、回原點判完成規則時。
  Triggers: 回原點, 歸原點, HOME, homing, Acm_AxHome, Acm_AxMoveHome, MODE12, 124, 128, CiA402, DS402,
  6098h, 6099h, 609Ah, 607Ch, 6041h, PAR_AxHomeVel, CFG_AxMaxVel, 0x80000081, 0x8000510F,
  RouteHomeStart, RouteHomeDone, StartHome1203, StartHomeCardSide, TickHomes, Pci1203DriveKind,
  Pci1203MotorRouteDriveKind, MotorAccessStepperLeaveOrigin, bW906HomeTrusted, SW3D-680, SGDXW, SGDXS,
  W906_GaliRoutedSingalHome, HOMEPOS0, TRAYSAFE, STEPLEAVE, READYDONE, MAXVEL, DUALAXIS.
---

# ht9050-1203-homing 相容入口

同主題已整合到 [hpi-motor-home](../hpi-motor-home/SKILL.md)，HT9050 與其他機型共用此入口。

- [原版詳細內容](../hpi-motor-home/references/drivers/pci1203/original-entry.md)

## 0. 一句話（EastSun，1203 層作者）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#0-一句話eastsun1203-層作者)

## 1. 兩種回原點：卡片式 vs 驅動器式

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#1-兩種回原點卡片式-vs-驅動器式)

### 1a. 卡片式（golden，SMC 與 golden EtherCAT）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#1a-卡片式goldensmc-與-golden-ethercat)

### 1b. 驅動器式（DS402 / CiA402，HT9050 全部 19 軸）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#1b-驅動器式ds402--cia402ht9050-全部-19-軸)

## 2. 樹怎麼決定走哪一條

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#2-樹怎麼決定走哪一條)

### 2a. 驅動器類型判斷（兩份一樣的規則）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#2a-驅動器類型判斷兩份一樣的規則)

### 2b. 全機 HOME（引擎路由）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#2b-全機-home引擎路由)

### 2c. 單軸 HOME（Motor Test / Teach / Light Scale）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#2c-單軸-homemotor-test--teach--light-scale)

## 3. 今天在 HT9050 上發現的驅動器差異（2026-10-03）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3-今天在-ht9050-上發現的驅動器差異2026-10-03)

### 3a. 壓在原點開關上起跑：伺服退開、步進一直往前（HOME-STEPLEAVE）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3a-壓在原點開關上起跑伺服退開步進一直往前home-stepleave)

### 3b. 快速回原點（軸已在原點，0.6 s）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3b-快速回原點軸已在原點06-s)

### 3c. 判「回完」的規則演進（全機）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3c-判回完的規則演進全機)

### 3d. 速度上限（HOME-MAXVEL / HOME-MAXVEL-SINGLE）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3d-速度上限home-maxvel--home-maxvel-single)

### 3e. 雙軸驅動器（同站號）DUALAXIS —— 目前關閉

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3e-雙軸驅動器同站號dualaxis--目前關閉)

### 3f. 其他

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#3f-其他)

## 4. golden 跟驅動器式回原點對不上的地方

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#4-golden-跟驅動器式回原點對不上的地方)

## 5. 新增驅動器型號的檢查清單

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#5-新增驅動器型號的檢查清單)

## 6. 待決定／暫時分支

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#6-待決定暫時分支)

### 6a. S-26 的 St01 判讀（20261004，項目 4／9／10；全文 `D:\AI_TempFile\st01-s26\FINDINGS-S26-items-4-9-10.md`）

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#6a-s-26-的-st01-判讀20261004項目-4910全文-dai_tempfilest01-s26findings-s26-items-4-9-10md)

## 7. 改這塊時的規矩

[讀取此節](../hpi-motor-home/references/drivers/pci1203/original-entry.md#7-改這塊時的規矩)
