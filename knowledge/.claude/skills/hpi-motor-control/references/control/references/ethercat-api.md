> 保存來源：`.claude/skills/ht9045-motor-control/references/ethercat-api.md`，main `eb7f47e7f`。原文機型、版本與日期維持原標註；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# Advantech EtherCAT 馬達 API 參考

> 對應原始碼：`Motor/myEthercatmotor.cpp`、`Motor/myEthercatmotor.h`  
> 硬體：Advantech PCI-1203 EtherCAT 主控卡  
> API 標頭檔：`EtherCAT/AdvMotApi.h`、`EtherCAT/AdvMotDrv.h`

---

## 概述

`TMyEtherCatMotor` 繼承 `HTMotor`，使用 Advantech PCI-1203 EtherCAT 主控卡驅動馬達。透過 AdvMotDrv API 進行軸控制，支援 Latch 位置觸發鎖存、直線多軸同步移動及 EtherCAT 網路分佈式 IO。

---

## 類別資訊

| 項目 | 說明 |
|------|------|
| 類別名稱 | `TMyEtherCatMotor` |
| 繼承自 | `HTMotor` |
| 標頭檔 | `Motor/myEthercatmotor.h` |
| 實作檔 | `Motor/myEthercatmotor.cpp` |
| 硬體 | Advantech PCI-1203 EtherCAT 控制卡 |
| API | AdvMotDrv（AdvMotApi.h） |

---

## 核心方法

### 初始化

```cpp
Motor->InitMotor(IoAddress);
```

### 位置移動

```cpp
Motor->MoveTo(targetPos);
Motor->MoveToPos(targetPos);
```

### 回原點

```cpp
Motor->HomeObject();
Motor->HomeFlag();      // 回原點完成旗標
```

### JOG 點動

```cpp
Motor->JogP();
Motor->JogN();
```

### 多軸路徑規劃

```cpp
Motor->AddAxis(axis);
Motor->AddPath(axis);
Motor->RunPath();
```

### 直線多軸同步移動

```cpp
Motor->LinearAxisMoveTo();
```

### Latch 機制（位置觸發鎖存）

```cpp
Motor->EnableTrigger();
Motor->GetLatchBuffer();
```

### EtherCAT 分佈式 IO 控制

```cpp
Motor->ethercat_set_output_bit(ring, ip, port, bitNo, data);
Motor->ethercat_set_output_byte(ring, ip, port, data);
```

---

## 特性

- **Latch 位置觸發鎖存**：支援在特定位置自動鎖存編碼器數值
- **直線多軸同步移動**：多軸協調直線插補運動
- **EtherCAT 網路分佈式 IO**：透過 EtherCAT 網路直接控制遠端 DO 輸出

<!-- preserved-content:end -->
