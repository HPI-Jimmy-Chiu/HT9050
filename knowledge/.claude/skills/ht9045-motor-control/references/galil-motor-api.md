# TMyGALILMotor — Galil DMC 馬達 API 參考

> 對應原始碼：`Motor/myGALILmotor.cpp`、`Motor/myGALILmotor.h`  
> 硬體：Galil DMC 控制器  
> 另見：[galil-api.md](galil-api.md)（TMyMotor 高階 Galil 封裝）

---

## 概述

`TMyGALILMotor` 繼承 `HTMotor`，獨立封裝 Galil DMC 控制器介面，可直接掛載至 `MOT[idx].Motor`。

> **與 TMyMotor 的區別**：`TMyMotor` 中的 `Gali_MotMove()`、`Gali_MotHome()` 等是高階封裝方法，內部呼叫 Galil DMC 命令字串。`TMyGALILMotor` 則是 HTMotor 子類別，實作 HTMotor 的虛擬函式介面，兩者不要混淆。

---

## 類別資訊

| 項目 | 說明 |
|------|------|
| 類別名稱 | `TMyGALILMotor` |
| 繼承自 | `HTMotor` |
| 標頭檔 | `Motor/myGALILmotor.h` |
| 實作檔 | `Motor/myGALILmotor.cpp` |
| 硬體 | Galil DMC 控制器 |
| 預設型別 | `Servo_Motor` |

---

## 核心方法

```cpp
// 初始化
Motor->InitMotor();
Motor->SetSpeed(speed);
Motor->SetInitSpeed(initSpeed);

// 位置移動
Motor->MoveToPos(targetPos);

// 回原點
Motor->HomeObject();

// JOG 點動
Motor->JogP();
Motor->JogN();

// 位置讀取
Motor->ReadRealPos();
Motor->ReadEnCoderRealPos();

// 狀態
Motor->MotionDone();
Motor->ResetPos();
```
