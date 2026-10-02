# 先達 SYNTEK 106-M2x4 Motion Slave API 參考

> 對應原始碼：`Motor/mySYNTEKmotor.cpp`、`Motor/mySYNTEKmotor.h`  
> 手冊來源：SYNTEK_106-M204_4Axis motion slave_UserGuide_CHA_20101206.pdf  
> DLL 標頭檔：`Motor/PCI_L132.H`（主控卡）  
> 關聯文件：[motionnet-api.md](../../ht9045-io-control/references/motionnet-api.md)（PCI-L132 主控卡 API）

---

## 概述

106-M2x4（M204/M224/M234）是先達科技（SYNTEK）出品的 **Motion.NET 4 軸遠端運動控制模組**。它是 Motion.NET 架構中的 Slave 模組，需搭配 **PCI-L132** 等主控卡使用。HT9045 使用 TMySYNTEKMotor 類別封裝該模組的控制功能。

---

## 產品型號

| 型號 | 說明 |
|------|------|
| 106-M204 | 基本 4 軸運動控制模組 |
| 106-M224 | 4 軸運動控制模組（含驅動器介面） |
| 106-M234 | 4 軸運動控制模組（三菱 J3S 介面） |

---

## Motion.NET 架構

```
┌──────────────────────────────────────────────────────────────┐
│ PC (Windows)                                                 │
│   └── PCI-L132 (Motion.NET Master Card)                      │
│           │                                                  │
│           │ RS-485 (CAT5 UTP/STP)                            │
│           ▼                                                  │
│   ┌───────────────┐   ┌───────────────┐   ┌───────────────┐  │
│   │ 106-M2x4      │──▶│ 106-M2x4      │──▶│ DIO Slaves    │  │
│   │ Motion Slave  │   │ Motion Slave  │   │               │  │
│   │ (4 軸)        │   │ (4 軸)        │   │               │  │
│   └───────────────┘   └───────────────┘   └───────────────┘  │
│                                                              │
│   最多可串接 64 個擴充模組                                     │
└──────────────────────────────────────────────────────────────┘
```

---

## 主要功能

- **脈波輸出控制**：±OUT/DIR 或 ±CW/CCW 輸出
- **脈波輸出速率**：Max 6.5Mpps / Min 0.05pps
- **脈波計數範圍**：28 bits (±134,217,728 pulses)
- **歸零模式**：13 種應用模式
- **速率曲線控制**：T-curve / S-curve 加減速
- **補間模式**：線性補間、圓弧補間、連續補間
- **位置閂鎖（Latch）**：LTC × 4
- **位置比較輸出**：CMP × 4
- **增量式編碼器**：±EA × 4、±EB × 4、±EZ × 4
- **機械接點**：PEL × 4、MEL × 4、ORG × 4、SLD × 4
- **伺服介面**：ALM × 4、RDY × 4、SVON × 4、INP × 4、ERC × 4
- **同步控制**：STA（同步啟動）、STP（同步停止）

---

## Motion.NET 通訊規格

| 項目 | 規格 |
|------|------|
| 串列控制介面 | Half duplex RS-485（隔離變壓器） |
| 線材型式 | CAT5 UTP/STP 網路線 |
| 突波保護 | 10KV |
| 傳送速度 | 2.5Mbps / 5Mbps / 10Mbps / 20Mbps |
| 通訊距離 | 最大 100m |
| 串接數目 | 最大 64 個擴充模組 |
| 電源電壓 | +24V DC |
| 工作溫度 | 0 ~ 60°C |

---

## 連接器定義

### CN1/CN2 - Motion.NET 通訊連接埠（RJ45）

| Pin | 訊號 | 說明 |
|-----|------|------|
| 3 | RS485+ | RS-485 差動訊號（+） |
| 6 | RS485- | RS-485 差動訊號（-） |

### CN3 - 外部電源輸入

| Pin | 訊號 | 說明 |
|-----|------|------|
| 1 | FG | 設備接地 |
| 2 | GND | GND 輸入 |
| 3 | 24V | +24V 電源輸入 |

### CN8 - 急停及同步控制

| Pin | 訊號 | 說明 |
|-----|------|------|
| 1-2 | STA | 同時啟動訊號輸入 |
| 3-4 | STP | 同時停止訊號輸入 |
| 5 | EMG | 緊急停止訊號輸入 |
| 6 | GND | GND |

### CN9/CN10/CN11/CN12 - 機械 I/O 介面（每軸 10Pin 牛角座）

| Pin | 訊號 | 說明 |
|-----|------|------|
| 1 | PEL | 正極限訊號輸入 |
| 2 | MEL | 負極限訊號輸入 |
| 3 | ORG | 原點位置訊號輸入 |
| 4 | SLD | 減速點訊號輸入 |
| 5 | +24V | 電源 +24V 輸出 |
| 6 | CMP | 位置比對訊號輸出 |
| 7 | LTC | 外部閂鎖訊號輸入 |
| 8 | GND | GND |
| 9 | BRK+ | 機械煞車訊號（+） |
| 10 | BRK- | 機械煞車訊號（-） |

### CN4/CN5/CN6/CN7 - 馬達驅動器介面（SCSI 50Pin）

| Pin | 訊號 | 說明 | Pin | 訊號 | 說明 |
|-----|------|------|-----|------|------|
| 3 | OUT- | 脈波訊號（-） | 29 | SVON | 伺服啟動輸出 |
| 4 | OUT+ | 脈波訊號（+） | 30 | ERC | 清除伺服錯誤 |
| 5 | DIR- | 方向訊號（-） | 31 | RALM | 重置警報 |
| 6 | DIR+ | 方向訊號（+） | 35 | RDY | 伺服備妥輸入 |
| 7 | +24V | 電源 +24V | 37 | ALM | 伺服警報輸入 |
| 8 | PEL | 正極限 | 39 | INP | 到位訊號輸入 |
| 9 | MEL | 負極限 | | | |
| 10 | BRK- | 煞車（-） | 21 | EA+ | 編碼器 A 相（+） |
| 11 | BRK+ | 煞車（+） | 22 | EA- | 編碼器 A 相（-） |
| | | | 23 | EZ+ | 編碼器 Z 相（+） |
| | | | 24 | EZ- | 編碼器 Z 相（-） |
| | | | 48 | EB+ | 編碼器 B 相（+） |
| | | | 49 | EB- | 編碼器 B 相（-） |

---

## LED 指示燈

| LED | 顏色 | 說明 |
|-----|------|------|
| RUN | 綠色 | 通訊正常運作中 |
| ERR | 紅色 | 通訊故障 |
| PG1 | 黃色 | DC +3.3V 電壓準位正常 |
| PG2 | 黃色 | DC +3.3V 電壓供應中 |
| SLD | 紅色 | 外部 SLD 訊號狀態 |
| ORG | 綠色 | 外部 ORG 訊號狀態 |
| MEL | 黃色 | 外部 MEL 訊號狀態 |
| PEL | 黃色 | 外部 PEL 訊號狀態 |

---

## DIP 開關設定

### SW1 - 位址設定開關

| 位元 | 說明 |
|------|------|
| A0~A5 | Motion.NET 模組位址（0~63） |

### SW2 - 通訊鮑率設定

| 設定 | B1 | B0 | 鮑率 |
|------|----|----|------|
| 0 | OFF | OFF | 2.5Mbps |
| 1 | OFF | ON | 5Mbps |
| 2 | ON | OFF | 10Mbps |
| 3 | ON | ON | 20Mbps |

---

## 運動控制 IC

106-M2x4 使用的運動控制 IC 為 **PLC6045BL**（Nippon Pulse Motor），提供：

- 4 軸獨立運動控制
- 線性/圓弧/連續補間
- 13 種歸零模式
- T/S 曲線加減速
- 位置閂鎖/比較功能

---

## 歸零模式（13 種）

| 模式 | 說明 |
|------|------|
| 0 | ORG 訊號 |
| 1 | ORG + Z 相 |
| 2 | +LIM 訊號 |
| 3 | +LIM + Z 相 |
| 4 | -LIM 訊號 |
| 5 | -LIM + Z 相 |
| 6 | ORG + SLD |
| 7 | ORG + SLD + Z 相 |
| ... | （依應用客製） |

---

## TMySYNTEKMotor 類別

### 繼承架構

```
HTMotor (基底類別)
    │
    └── TMySYNTEKMotor (先達 MotionNet Slave 實作)
```

### 主要成員變數

```cpp
// 卡件識別
int iRingNo;      // Ring 編號（對應 PCI-L132 Ring）
int iModuleNo;    // Module 編號（模組位址）
int iAxisNo;      // 軸編號（0~3）
```

### 主要方法

| 方法 | 功能說明 |
|------|----------|
| `InitMotor()` | 初始化馬達參數 |
| `MoveTo(Tar)` | 移動至目標位置 |
| `JogP()` / `JogN()` | 正/負方向 JOG |
| `Stop()` | 停止運動 |
| `DecStop()` | 減速停止 |
| `HomeObject()` | 原點復歸 |
| `MotionDone()` | 檢查運動完成 |
| `ReadPos()` | 讀取指令位置 |
| `ReadRealPos()` | 讀取實際位置 |
| `ReadEnCoderRealPos()` | 讀取編碼器位置 |
| `SetSpeed()` | 設定運動速度 |
| `SetServoOn()` | 伺服開/關 |

---

## 與 PCI-L132 主控卡協作

TMySYNTEKMotor 透過 PCI-L132 主控卡的 API 函數進行控制：

```cpp
// 位置讀取（透過 PCI-L132 API）
L132_Get_Command(iRingNo, iModuleNo, iAxisNo, &position);
L132_Get_Position(iRingNo, iModuleNo, iAxisNo, &position);

// 運動控制
L132_Set_Speed(iRingNo, iModuleNo, iAxisNo, speed);
L132_Start_Move(iRingNo, iModuleNo, iAxisNo, target);
L132_Stop(iRingNo, iModuleNo, iAxisNo);

// 狀態讀取
L132_Get_Status(iRingNo, iModuleNo, iAxisNo, &status);
```

---

## 驅動器介面版本

| 型號 | 適用驅動器 |
|------|-----------|
| 106-M224-PMA | Panasonic MINAS A 系列 |
| 106-M224-PA4/5 | Panasonic MINAS A4/A5 系列 |
| 106-M234-J3S | Mitsubishi J3S 系列 |

---

## 安裝流程

1. **硬體安裝**：依需求調整 SW1（位址）與 SW2（鮑率）
2. **供電**：由 CN3 供應 +24V DC 電源
3. **通訊連接**：以 CAT5 網路線連接 CN1/CN2 至主控卡
4. **機械 I/O**：連接 CN9~CN12 至各軸感測器
5. **驅動器介面**：連接 CN4~CN7 至伺服/步進驅動器
6. **同步控制**（選配）：連接 CN8 的 STA/STP 訊號
7. **上電測試**：確認 LED 狀態正常

---

## 問題排除

| 症狀 | 可能原因 | 處理方式 |
|------|---------|----------|
| ERR LED 亮紅色 | 通訊故障 | 檢查網路線連接、鮑率設定 |
| 馬達不動 | 無伺服啟動 | 檢查 SVON 訊號、ALM 狀態 |
| 位置錯誤 | 編碼器訊號問題 | 檢查編碼器接線、倍率設定 |
| 緊急停止後無法動作 | EMG 訊號未解除 | 解除緊急停止按鈕 |

---

## 相關參考

- [motionnet-api.md](../../ht9045-io-control/references/motionnet-api.md) - PCI-L132 主控卡 API
- [motor-classes.md](motor-classes.md) - 馬達類別參考
