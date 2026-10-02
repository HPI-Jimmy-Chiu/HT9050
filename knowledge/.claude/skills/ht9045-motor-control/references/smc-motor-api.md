# TMySMCMotor — CONTEC SMC 馬達 API 參考

> 對應原始碼：`Motor/mySMCmotor.cpp`、`Motor/mySMCmotor.h`  
> 硬體：CONTEC SMC-4DF2 / SMC-8DF2 PCI 軸卡  
> 另見：[smc-api.md](smc-api.md)（SMC 底層 API）

---

## 概述

`TMySMCMotor` 繼承 `HTMotor`，驅動 CONTEC SMC-xDF2-PCI 4/8 軸卡。支援多速度移動模式（標準、短距離、短距離慢速）及 Latch 機制。

---

## 類別資訊

| 項目 | 說明 |
|------|------|
| 類別名稱 | `TMySMCMotor` |
| 繼承自 | `HTMotor` |
| 標頭檔 | `Motor/mySMCmotor.h` |
| 實作檔 | `Motor/mySMCmotor.cpp` |
| 硬體 | CONTEC SMC-xDF2-PCI 4/8 軸卡 |

---

## 核心方法

```cpp
// 速度設定（多模式）
Motor->SetSpeed(speed);
Motor->SetSpeedShortDistance(speed);        // 短距離模式
Motor->SetSpeedShortDisSlowSP(speed);       // 短距離慢速模式

// 位置移動（對應不同速度模式）
Motor->MoveToPos(targetPos);
Motor->MoveToPosShortDistance(targetPos);
Motor->MoveToPosShortDisSlowSP(targetPos);

// 加減速
Motor->SetAcc(acceleration);
Motor->SetDec(deceleration);

// Latch 與軟體極限
Motor->GetLatchBuffer();
Motor->SetFIFOLatchSrc();
Motor->SetSoftLimit(positive, negative);
```

---

## 速度模式對應

| 速度設定方法 | 搭配移動方法 | 適用情境 |
|-------------|-------------|---------|
| `SetSpeed()` | `MoveToPos()` | 一般長距離移動 |
| `SetSpeedShortDistance()` | `MoveToPosShortDistance()` | 短距離移動（低速） |
| `SetSpeedShortDisSlowSP()` | `MoveToPosShortDisSlowSP()` | 短距離精密定位 |
