# 康泰克 CONTEC SMC 系列軸卡 API 參考

舊引用路徑保留；[讀取整理後文件](../../hpi-motor-control/references/control/references/smc-api.md)。

## 概述

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/01.md#概述)

## 軸卡型號比較

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/02.md#軸卡型號比較)

## 主要功能

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/03.md#主要功能)

## I/O 訊號定義

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#io-訊號定義)

### 極限輸入訊號

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#極限輸入訊號)

### 通用輸入訊號（每軸 7 點）

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#通用輸入訊號每軸-7-點)

### 編碼器輸入

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#編碼器輸入)

### 脈波輸出

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#脈波輸出)

### 通用輸出訊號（每軸 3 點）

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/04.md#通用輸出訊號每軸-3-點)

## API 常數定義（CSmc.h / mySMCmotor.cpp）

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#api-常數定義csmch--mysmcmotorcpp)

### 控制輸入訊號類型 (SmcWSetCtrlTypeIn)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#控制輸入訊號類型-smcwsetctrltypein)

### 控制輸出訊號類型 (SmcWSetCtrlTypeOut)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#控制輸出訊號類型-smcwsetctrltypeout)

### 脈波輸出模式 (SmcWSetPulseType)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#脈波輸出模式-smcwsetpulsetype)

### 原點搜尋模式 (SmcWSetOrgMode)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#原點搜尋模式-smcwsetorgmode)

### 原點邏輯設定 (SmcWSetOrgLog)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#原點邏輯設定-smcwsetorglog)

### 計數器模式 (SmcWSetCounterMode)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#計數器模式-smcwsetcountermode)

### 運動類型 (SmcWSetReady)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#運動類型-smcwsetready)

### 停止位置模式 (SmcWSetStopPosition)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#停止位置模式-smcwsetstopposition)

### I/O 狀態位元 (SmcWGetCtrlInOutStatus)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#io-狀態位元-smcwgetctrlinoutstatus)

### 編碼器類型 (SmcWSetEncType)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#編碼器類型-smcwsetenctype)

### FIFO Latch 來源 (SmcWSetFIFOLatchSrc)

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/05.md#fifo-latch-來源-smcwsetfifolatchsrc)

## TMySMCMotor 類別方法

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/06.md#tmysmcmotor-類別方法)

## 初始化流程範例

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/07.md#初始化流程範例)

## 原點復歸流程

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/08.md#原點復歸流程)

## 硬體規格摘要

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/09.md#硬體規格摘要)

## 運動控制 IC

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/10.md#運動控制-ic)

## SMC-8DF-PCI-C01 客製版功能

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smc-8df-pci-c01-客製版功能)

### 功能概述

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#功能概述)

### 系統架構

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#系統架構)

### 腳位差異（與標準版比較）

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#腳位差異與標準版比較)

### SENSOR 輸入規格

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#sensor-輸入規格)

### FIFO Buffer 規格

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#fifo-buffer-規格)

### 時序限制

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#時序限制)

### C01 專用 API 函式

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#c01-專用-api-函式)

### C01 專用 API 函式詳細說明

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#c01-專用-api-函式詳細說明)

#### SmcWResetLatchFIFO

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smcwresetlatchfifo)

#### SmcWSetFIFOLatchSrc

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smcwsetfifolatchsrc)

#### SmcWGetFIFOLatchSrc

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smcwgetfifolatchsrc)

#### SmcWGetLatchDataFromBuffer

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smcwgetlatchdatafrombuffer)

#### SmcWGetLatchFIFOLength

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#smcwgetlatchfifolength)

### C01 版本使用範例

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/11.md#c01-版本使用範例)

## 注意事項

[讀取此節](../../hpi-motor-control/references/control/references/smc-api/12.md#注意事項)
