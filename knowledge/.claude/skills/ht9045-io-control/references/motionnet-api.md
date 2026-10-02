# MotionNet API 參考（SYN-TEK / 先達）

SYN-TEK（先達）MotionNet 系統 API 參考文件，適用於 **PCI-L132** 與 **PCI-M114G** Master 卡。

> **供應商**: SYN-TEK（先達）  
> **原廠手冊**: `Motion.net_ProgrammingGuide_ENG_C版本.pdf`（位於 `E:\HT9045W_相關料件技術文件\IO\先達\`）  
> **另見**: [motionnet-mn200-api.md](motionnet-mn200-api.md)（ICP-DAS / 泓格 PISO-MN200）

## 目錄

1. [系統限制](#系統限制)
2. [系統架構](#系統架構)
3. [初始化 API](#初始化-api)
4. [DIO 操作 API](#dio-操作-api)
5. [診斷 API](#診斷-api)
6. [看門狗 API](#看門狗-api)
7. [DIO Slave 模組類型](#dio-slave-模組類型)
8. [錯誤碼](#錯誤碼)

---

## 系統限制

| 項目 | 限制 | 說明 |
|------|------|------|
| **每張卡片 Ring 數** | 2 | 每張 Master 卡（PCI-L132/PCI-M114G）支援 2 個 Ring |
| **每 Ring IP 數** | 64 | DeviceIP 範圍 0~63（由 DIP Switch S1 設定） |
| **每 IP Port 數** | 4 | PortNo 範圍 0~3 |
| **每 Port Bit 數** | 8 | Bit 範圍 0~7 |
| **Ring 最大長度** | 100 公尺 | 包含所有 Slave 模組的總線長度 |
| **Terminator** | 必須 | 最後一個 Slave 模組須設定終端電阻 |

### 定址容量計算

```
每張卡片最大 IO 點數 = 2 Ring × 64 IP × 4 Port × 8 Bit = 4096 點
```

---

## 系統架構

```
┌──────────────────┐
│  Master Card     │
│  (PCI-L132)      │
│                  │
│  ┌─────┐ ┌─────┐ │
│  │Ring0│ │Ring1│ │
│  └──┬──┘ └──┬──┘ │
└─────┼───────┼────┘
      │       │
      │       └──────────────────────────────────────┐
      │                                              │
      ▼                                              ▼
┌─────────┐   ┌─────────┐   ┌─────────┐      ┌─────────┐
│ Slave   │───│ Slave   │───│ Slave   │ ...  │ Slave   │[TR]
│ IP=0    │   │ IP=1    │   │ IP=2    │      │ IP=63   │
└─────────┘   └─────────┘   └─────────┘      └─────────┘
                                              (Terminator)
```

### 通訊類型

| 類型 | 說明 |
|------|------|
| Cyclic Data | DIO Slave 使用，週期性資料交換 |
| Wrapped Data | Motion Slave 使用，封包式資料交換 |

---

## 初始化 API

### _m114g_initial

初始化 PCI-M114G 硬體。

```cpp
I16 _m114g_initial(U16 &ExistCards);
```

| 參數 | 型別 | 說明 |
|------|------|------|
| ExistCards | U16& | 回傳存在的卡片數量 |

### _l132_open / _l112_open

初始化 PCI-L132/L112 硬體。

```cpp
I16 _l132_open(U16 &ExistCards);
I16 _l112_open(U16 &ExistCards);
```

### _m114g_open_mnet

開啟 MNET 介面。

```cpp
I16 _m114g_open_mnet(U16 CardNo);
```

### _mnet_start_ring

啟動 Ring 通訊。

```cpp
I16 _mnet_start_ring(U16 RingNo);
```

| 回傳 | 說明 |
|------|------|
| ERR_NoError | 成功 |
| ERR_Invalid_RingNo | Ring 編號無效 |

### _mnet_stop_ring

停止 Ring 通訊。

```cpp
I16 _mnet_stop_ring(U16 RingNo);
```

### _mnet_reset_ring

軟重置 Ring。

```cpp
I16 _mnet_reset_ring(U16 RingNo);
```

### _mnet_close

關閉 MNET 介面（僅 104-L112）。

```cpp
I16 _mnet_close();
```

### 初始化流程

```
1. _m114g_initial(&ExistCards)     // 硬體初始化
       ↓
2. _m114g_open_mnet(CardNo)        // 開啟 MNET
       ↓
3. _mnet_start_ring(RingNo)        // 啟動 Ring
       ↓
4. _mnet_m1_initial(RingNo, IP)    // 初始化各 Slave（Motion Slave）
```

---

## DIO 操作 API

### _mnet_io_output

輸出整個 Port（8-bit）。

```cpp
I16 _mnet_io_output(U16 RingNo, U16 SlaveIP, U8 PortNo, U8 Val);
```

| 參數 | 型別 | 範圍 | 說明 |
|------|------|------|------|
| RingNo | U16 | 0~1 | Ring 編號 |
| SlaveIP | U16 | 0~63 | Slave IP（63~0xFFFF 表示無警告） |
| PortNo | U8 | 0~3 | Port 編號 |
| Val | U8 | 0~255 | 輸出值 |

**注意**: 若 Port 類型不符（如對 Input Port 輸出），MNET 將停止輸出動作。

### _mnet_io_input

讀取整個 Port。

```cpp
I16 _mnet_io_input(U16 RingNo, U16 SlaveIP, U8 PortNo);
```

| 回傳 | 說明 |
|------|------|
| > 0 | 輸入值（0~255） |
| < 0 | 錯誤碼 |

---

## 診斷 API

### _mnet_get_slave_type

取得 Slave 模組類型。

```cpp
I16 _mnet_get_slave_type(U16 RingNo, U16 SlaveIP, U8 *Type);
```

**注意**: 須先呼叫 `_mnet_start_ring()` 才能使用。

### _mnet_get_slave_info

取得 Slave 資訊。

```cpp
I16 _mnet_get_slave_info(U16 RingNo, U16 SlaveIP);
```

| 回傳值 Bit | 說明 |
|------------|------|
| Bit 0~2 | 模組子類型 |
| Bit 3 | 0=IO Slave, 1=Motion Slave |
| Bit 7 | 0=未使用, 1=使用中 |

### _mnet_get_ring_active_table

取得連線中的 Slave 列表。

```cpp
I16 _mnet_get_ring_active_table(U16 RingNo, U32 *DevTable);
```

| 參數 | 說明 |
|------|------|
| DevTable[0] | Bit 0~31 對應 Slave IP 0~31 |
| DevTable[1] | Bit 0~31 對應 Slave IP 32~63 |

### _mnet_get_slave_error_table

取得通訊錯誤的 Slave 列表。

```cpp
I16 _mnet_get_slave_error_table(U16 RingNo, U32 *ErrorTable);
```

**說明**: 連續 3 次通訊失敗會設定對應的錯誤旗標。

### _mnet_clear_slave_error_flag

清除 Slave 錯誤旗標。

```cpp
I16 _mnet_clear_slave_error_flag(U16 RingNo, U32 *ErrorTable);
```

### _mnet_get_error_device

取得第一個錯誤的 Slave。

```cpp
I16 _mnet_get_error_device(U16 RingNo);
```

| 回傳 | 說明 |
|------|------|
| > 0 | 錯誤 Slave IP（取低 8 位元） |
| < 0 | 錯誤碼 |

### _mnet_get_ring_status

取得 Ring 狀態。

```cpp
I16 _mnet_get_ring_status(U16 RingNo);
```

---

## 看門狗 API

軟體看門狗可監控 Ring 通訊品質。

### _mnet_enable_soft_watchdog

啟用軟體看門狗。

```cpp
I16 _mnet_enable_soft_watchdog(U16 RingNo, HANDLE *User_hEvent);
```

**說明**: 啟用後會建立監控執行緒，監控事件：IOPC/EIOE/EDTE/ERAE/CAER。

### _mnet_disable_soft_watchdog

停用軟體看門狗。

```cpp
I16 _mnet_disable_soft_watchdog(U16 RingNo);
```

### _mnet_watchdog_link

連結看門狗事件回呼。

```cpp
I16 _mnet_watchdog_link(...);
```

### _mnet_get_com_status

取得通訊狀態。

```cpp
I16 _mnet_get_com_status(U16 RingNo);
```

### _mnet_set_ring_quality_param

設定通訊品質監控參數。

```cpp
I16 _mnet_set_ring_quality_param(...);
```

### _mnet_clear_ring_error

清除 Ring 錯誤狀態。

```cpp
I16 _mnet_clear_ring_error(U16 RingNo);
```

---

## DIO Slave 模組類型

### 32-bit 模組（G9002 系列）

| 常數 | 值 | 配置 |
|------|-----|------|
| `G9002_Q32` | 0xB0 | 32 OUT |
| `G9002_I8Q24` | 0xB1 | 8 IN / 24 OUT |
| `G9002_I16Q16` | 0xB2 | 16 IN / 16 OUT |
| `G9002_I24Q8` | 0xB3 | 24 IN / 8 OUT |
| `G9002_I32` | 0xB4 | 32 IN |

### 16-bit 模組（G9102 系列）

| 常數 | 值 | 配置 |
|------|-----|------|
| `G9102_I16` | 0xC0 | 16 IN |
| `G9102_I14Q2` | 0xC1 | 14 IN / 2 OUT |
| `G9102_I12Q4` | 0xC2 | 12 IN / 4 OUT |
| `G9102_I10Q6` | 0xC3 | 10 IN / 6 OUT |
| `G9102_I8Q8` | 0xC4 | 8 IN / 8 OUT |
| `G9102_I6Q10` | 0xC5 | 6 IN / 10 OUT |
| `G9102_I4Q12` | 0xC6 | 4 IN / 12 OUT |
| `G9102_Q16` | 0xC7 | 16 OUT |

### 16-bit 模組（G9205 系列）

| 常數 | 值 | 配置 |
|------|-----|------|
| `G9205_I16` | 0xC8 | 16 IN |
| `G9205_I14Q2` | 0xC9 | 14 IN / 2 OUT |
| `G9205_I12Q4` | 0xCA | 12 IN / 4 OUT |
| `G9205_I10Q6` | 0xCB | 10 IN / 6 OUT |
| `G9205_I8Q8` | 0xCC | 8 IN / 8 OUT |
| `G9205_I6Q10` | 0xCD | 6 IN / 10 OUT |
| `G9205_I4Q12` | 0xCE | 4 IN / 12 OUT |
| `G9205_Q16` | 0xCF | 16 OUT |

### Motion Slave 模組

| 常數 | 值 | 說明 |
|------|-----|------|
| `G9003_M101` | 0xA3 | 1-Axis 模組 |
| `G9004_M104` | 0xA4 | 4-Axis 模組（106-M1x4） |
| `G9004_M204` | 0xA7 | 4-Axis 模組（106-M204） |

### Analog I/O 模組

| 常數 | 值 | 說明 |
|------|-----|------|
| `G9004_A104` | 0xD0 | 4 OUT Analog |
| `G9004_A180` | 0xD1 | 8 IN Analog |

---

## 錯誤碼

| 常數 | 說明 |
|------|------|
| `ERR_NoError` | 成功 |
| `ERR_Invalid_RingNo` | Ring 編號無效（確認 Ring 是否啟用） |
| `ERR_Invalid_Slave` | Slave IP 無效 |
| `ERR_InvalidDeviceType` | 設備類型無效 |
| `ERR_Not_Initialized` | 硬體未初始化 |
| `ERR_Not_Release` | 資源未釋放 |
| `ERR_FailToOpenFile` | 無法開啟設定檔 |
| `ERR_Invalid_Event` | 事件物件無效 |
| `ERR_Invalid_TableAddr` | 變數位址無效 |
| `ERR_M1_LoadAxisDataError` | 無法載入軸配置檔 |
