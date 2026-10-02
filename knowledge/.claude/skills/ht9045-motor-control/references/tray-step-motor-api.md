# TdmTrayMotor — Tray 軌道步進馬達 API 參考

> 對應原始碼：`Motor/TrayStepMotor.cpp`、`Motor/TrayStepMotor.h`、`Motor/TrayStepMotor.dfm`

---

## 概述

DataModule 元件，透過 RS-232 序列通訊控制 Tray 軌道步進馬達。與 HTMotor 體系無繼承關係，獨立管理 10 條軌道與 2 個震動馬達。

---

## 模組資訊

| 項目 | 說明 |
|------|------|
| 類別名稱 | `TdmTrayMotor` |
| 類型 | Delphi DataModule |
| 標頭檔 | `Motor/TrayStepMotor.h` |
| 實作檔 | `Motor/TrayStepMotor.cpp` |
| 表單檔 | `Motor/TrayStepMotor.dfm` |
| 通訊方式 | RS-232 serial protocol |

---

## 軌道索引

| 索引 | 軌道 |
|------|------|
| 0 | Load |
| 1 | Auto1 |
| 2 | Auto2 |
| 3 | Auto3 |
| 4 | Auto4 |
| 5 | Auto5 |
| 6 | Empty |
| 7 | Color |
| 8 | VibrationMotor1 |
| 9 | VibrationMotor2 |

---

## 核心方法

```cpp
// 初始化 COM port
dmTrayMotor->RS232Init(ComPort);

// 設定各軌道速度（百分比）
dmTrayMotor->iStepMotorSpeed[trackIndex] = speedPercent;

// 送出速度設定
dmTrayMotor->StartSetSpeed();
```
