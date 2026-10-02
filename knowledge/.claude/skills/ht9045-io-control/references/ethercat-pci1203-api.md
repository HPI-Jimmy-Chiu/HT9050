# EtherCAT PCI-1203 API 參考

> 本文件根據研華 PCI-1203/PCIE-1203 用戶手冊整理，適用於 HT9045 專案中 `ePCI1203` ISABase 類型。

---

## 系統限制

| 參數 | 限制值 |
|------|--------|
| 最大軸數 | 64（0~63） |
| 群組數 | 6 組，每組最多 8 軸插補 |
| 最大 DI/DO 通道數 | 依從站配置，透過 `FT_DaqDiMaxChan`/`FT_DaqDoMaxChan` 取得 |
| 運動環循環時間 | 250μs（最低） |
| IO 環循環時間 | 200μs |
| 主站模式 | 支援多主站同時使用不同 EtherCAT 端口 |

---

## EtherCAT 架構

```
┌──────────────────────────────────────────────────────────────────┐
│                          PCI Bus                                  │
└──────────────────────────────────────────────────────────────────┘
                              │
          ┌───────────────────┴───────────────────┐
          │         PCI-1203 / PCIE-1203          │
          │    ┌─────────────┬─────────────┐      │
          │    │  Motion     │    I/O      │      │
          │    │  Master     │   Master    │      │
          │    │ (250μs)     │  (200μs)    │      │
          │    └─────────────┴─────────────┘      │
          │        │ EtherCAT     │ EtherCAT      │
          └────────┼──────────────┼───────────────┘
                   ↓              ↓
     ┌─────────────┴──────────────┴──────────────────┐
     │              EtherCAT Ring                     │
     │  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐          │
     │  │Servo│→ │Servo│→ │ I/O │→ │ I/O │→ ...    │
     │  │Drive│  │Drive│  │Module│ │Module│          │
     │  └─────┘  └─────┘  └─────┘  └─────┘          │
     └───────────────────────────────────────────────┘
```

### 特性

- **多主站模式**：運動控制與 I/O 可分離至不同主站
- **分布式時鐘（DC）**：從站間同步抖動 < 1μs
- **拓撲結構**：支援總線形、樹形、環形、星形
- **電纜長度**：兩節點間最大 100m（100BASE-TX）
- **最大從站數**：65,535 台

---

## 初始化流程

```
Acm_GetAvailableDevs() → 取得可用設備列表
         ↓
Acm_DevOpen() → 開啟設備，取得 DeviceHandle
         ↓
Acm_DevLoadMapFile() → 下載映射檔（可選）
         ↓
Acm_AxOpen() / Acm_GpOpen() → 開啟軸/群組
         ↓
     ... 運動/IO 控制 ...
         ↓
Acm_AxClose() / Acm_GpClose() → 關閉軸/群組
         ↓
Acm_DevClose() → 關閉設備
```

---

## 核心 API 分類

### 設備管理

| 函式 | 說明 |
|------|------|
| `Acm_GetAvailableDevs(DevList, MaxEntryCount, &OutAvailableCount)` | 取得可用設備列表 |
| `Acm_DevOpen(DevNo, &DevHandle, InitFlags)` | 開啟設備 |
| `Acm_DevClose(&DevHandle)` | 關閉設備 |
| `Acm_DevLoadMapFile(DevHandle, FilePath)` | 載入 I/O 映射檔 |
| `Acm_DevResetAllError(DevHandle)` | 重置所有錯誤 |

### 屬性存取

| 函式 | 說明 |
|------|------|
| `Acm_GetU32Property(Handle, PropertyID, &Value)` | 取得 U32 屬性 |
| `Acm_SetU32Property(Handle, PropertyID, Value)` | 設定 U32 屬性 |
| `Acm_GetF64Property(Handle, PropertyID, &Value)` | 取得 F64 屬性 |
| `Acm_SetF64Property(Handle, PropertyID, Value)` | 設定 F64 屬性 |
| `Acm_GetChannelProperty(Handle, Channel, PropertyID, &Value)` | 取得通道屬性 |
| `Acm_SetChannelProperty(Handle, Channel, PropertyID, Value)` | 設定通道屬性 |

---

## DI/DO 控制

### DI 讀取

| 函式 | 說明 |
|------|------|
| `Acm_DaqDiGetBit(DevHandle, DiChannel, &BitData)` | 讀取單一 DI 位元 |
| `Acm_DaqDiGetByte(DevHandle, StartChannel, &ByteData)` | 讀取 8-bit DI |
| `Acm_DaqDiGetWord(DevHandle, StartChannel, &WordData)` | 讀取 16-bit DI |
| `Acm_DaqDiGetAllChannels(DevHandle, &DataBuffer, BufferLength)` | 讀取所有 DI |

### DO 輸出

| 函式 | 說明 |
|------|------|
| `Acm_DaqDoSetBit(DevHandle, DoChannel, BitData)` | 寫入單一 DO 位元 |
| `Acm_DaqDoSetByte(DevHandle, StartChannel, ByteData)` | 寫入 8-bit DO |
| `Acm_DaqDoSetWord(DevHandle, StartChannel, WordData)` | 寫入 16-bit DO |
| `Acm_DaqDoSetAllChannels(DevHandle, &DataBuffer, BufferLength)` | 寫入所有 DO |
| `Acm_DaqDoGetBit(DevHandle, DoChannel, &BitData)` | 讀回 DO 狀態 |
| `Acm_DaqDoGetByte(DevHandle, StartChannel, &ByteData)` | 讀回 8-bit DO 狀態 |

### DI/DO 屬性

| 屬性 ID | 名稱 | 說明 |
|---------|------|------|
| 50 | `FT_DaqDiMaxChan` | DI 通道最大數量（唯讀） |
| 51 | `FT_DaqDoMaxChan` | DO 通道最大數量（唯讀） |
| 1500 | `CFG_CH_DaqDiInvertEnable` | DI 反相（0=不反相, 1=反相） |
| 1501 | `CFG_CH_DaqDiLowFilter` | DI 濾波下限（0~65535） |
| 1502 | `CFG_CH_DaqDiHighFilter` | DI 濾波上限 |

---

## AI/AO 控制

### AI 讀取

| 函式 | 說明 |
|------|------|
| `Acm_DaqAiGetRawData(DevHandle, Channel, &RawData)` | 讀取 AI 原始值 |
| `Acm_DaqAiGetVoltage(DevHandle, Channel, &Voltage)` | 讀取 AI 電壓值 |
| `Acm_DaqAiGetCurrent(DevHandle, Channel, &Current)` | 讀取 AI 電流值 |

### AO 輸出

| 函式 | 說明 |
|------|------|
| `Acm_DaqAoSetRawData(DevHandle, Channel, RawData)` | 寫入 AO 原始值 |
| `Acm_DaqAoSetVoltage(DevHandle, Channel, Voltage)` | 寫入 AO 電壓值 |
| `Acm_DaqAoCurrent(DevHandle, Channel, Current)` | 寫入 AO 電流值 |

### AI/AO 屬性

| 屬性 ID | 名稱 | 說明 |
|---------|------|------|
| 52 | `FT_DaqAiRangeMap` | AI 支援範圍（位元遮罩） |
| 53 | `FT_DaqAoRangeMap` | AO 支援範圍（位元遮罩） |
| 54 | `FT_DaqAiMaxSingleChan` | 單端 AI 最大通道數 |
| 55 | `FT_DaqAiMaxDiffChan` | 差分 AI 最大通道數 |
| 57 | `FT_DaqAoMaxChan` | AO 最大通道數 |

### AI 範圍定義（位元遮罩）

| 位元 | 宏定義 | 範圍 |
|------|--------|------|
| 0 | `DAQ_AI_NEG_10V_TO_10V` | ±10V |
| 1 | `DAQ_AI_NEG_5V_TO_5V` | ±5V |
| 2 | `DAQ_AI_NEG_2500MV_TO_2500MV` | ±2.5V |
| 8 | `DAQ_AI_NEG_0_TO_10V` | 0~10V |
| 16 | `DAQ_AI_0MA_TO_20MA` | 0~20mA |
| 17 | `DAQ_AI_4MA_TO_20MA` | 4~20mA |

---

## 軸運動控制

### 軸管理

| 函式 | 說明 |
|------|------|
| `Acm_AxOpen(DevHandle, AxisNo, &AxisHandle)` | 開啟軸 |
| `Acm_AxClose(&AxisHandle)` | 關閉軸 |
| `Acm_AxResetError(AxisHandle)` | 重置軸錯誤 |
| `Acm_AxGetState(AxisHandle, &State)` | 取得軸狀態 |
| `Acm_AxGetMotionStatus(AxisHandle, &Status)` | 取得運動狀態 |

### 單軸運動

| 函式 | 說明 |
|------|------|
| `Acm_AxMoveRel(AxisHandle, Distance)` | 相對移動 |
| `Acm_AxMoveAbs(AxisHandle, Position)` | 絕對移動 |
| `Acm_AxMoveVel(AxisHandle, Direction)` | 速度運動（JOG） |
| `Acm_AxStopDec(AxisHandle)` | 減速停止 |
| `Acm_AxStopEmg(AxisHandle)` | 緊急停止 |
| `Acm_AxHome(AxisHandle, Mode)` | 原點復歸 |

### 軸狀態定義

| 值 | 宏定義 | 說明 |
|----|--------|------|
| 0 | `STA_AX_DISABLE` | 軸禁用 |
| 1 | `STA_AX_READY` | 軸就緒 |
| 2 | `STA_AX_STOPPING` | 停止中 |
| 3 | `STA_AX_ERROR_STOP` | 錯誤停止 |
| 4 | `STA_AX_DISCRETE_MOTION` | 離散運動 |
| 5 | `STA_AX_CONTINUOUS_MOTION` | 連續運動 |
| 6 | `STA_AX_SYNCHRONIZED_MOTION` | 同步運動 |
| 7 | `STA_AX_HOMING` | 回原點中 |

---

## 群組插補

### 群組管理

| 函式 | 說明 |
|------|------|
| `Acm_GpOpen(DevHandle, &AxisList, AxisCount, &GpHandle)` | 開啟群組 |
| `Acm_GpClose(&GpHandle)` | 關閉群組 |
| `Acm_GpAddAxis(&GpHandle, &AxisHandle)` | 加入軸 |
| `Acm_GpRemAxis(&GpHandle, &AxisHandle)` | 移除軸 |

### 插補運動

| 函式 | 說明 |
|------|------|
| `Acm_GpMoveLinearRel(GpHandle, DistanceArray, ArrayElements)` | 相對直線插補 |
| `Acm_GpMoveLinearAbs(GpHandle, PositionArray, ArrayElements)` | 絕對直線插補 |
| `Acm_GpMoveCircularRel(GpHandle, CenterArray, EndArray, Direction)` | 相對圓弧插補 |
| `Acm_GpMoveCircularAbs(GpHandle, CenterArray, EndArray, Direction)` | 絕對圓弧插補 |
| `Acm_GpMoveHelicalRel(...)` | 相對螺旋插補 |

### 群組狀態定義

| 值 | 宏定義 | 說明 |
|----|--------|------|
| 0 | `STA_GP_DISABLE` | 群組禁用 |
| 1 | `STA_GP_READY` | 群組就緒 |
| 2 | `STA_GP_STOPPING` | 停止中 |
| 3 | `STA_GP_ERROR_STOP` | 錯誤停止 |
| 4 | `STA_GP_MOVING` | 運動中 |
| 5 | `STA_GP_HOMING` | 回原點中 |

---

## 設備屬性

### 緊急停止

| 屬性 ID | 名稱 | 說明 |
|---------|------|------|
| 220 | `CFG_DevEmgLogic` | EMG 信號邏輯（0=Normal Open, 1=Normal Close） |
| 222 | `CFG_DevEmgFilterTime` | EMG 濾波時間（0=5μs, 1=100μs, 2=200μs, 3=500μs） |

### 錯誤處理

| 屬性 ID | 名稱 | 說明 |
|---------|------|------|
| 229 | `CFG_DevLogMsg` | 錯誤記錄（0=不記錄, 1=記錄至 .txt） |
| 230 | `CFG_DevErrorReact` | 錯誤反應（0=其他軸不反應, 1=全部軸立刻停止, 2=全部軸減速停止） |

### 循環時間

| 屬性 ID | 名稱 | 說明 |
|---------|------|------|
| 261 | `CFG_MasCycleTime` | 運動環循環時間（μs），唯讀 |
| 262 | `CFG_IoCycleTime` | IO 環循環時間（μs），唯讀 |

---

## 錯誤碼

### 常見錯誤

| 錯誤碼 | 說明 |
|--------|------|
| 0 | 成功（無錯誤） |
| 1 | 無效設備 Handle |
| 2 | 無效軸 Handle |
| 3 | 無效群組 Handle |
| 4 | 參數超出範圍 |
| 5 | 緩衝區不足 |

### EtherCAT 錯誤

| 錯誤碼 | 說明 |
|--------|------|
| 0x8001 | EtherCAT 通訊錯誤 |
| 0x8002 | 從站未連接 |
| 0x8003 | 從站狀態錯誤 |
| 0x8004 | SDO 通訊超時 |

---

## BCB6 使用注意

### 標頭檔

```cpp
#include "AdvMotDrv.h"   // 主標頭檔
#include "AdvMotApi.h"   // API 函式宣告
```

### 連結庫

- `AdvMotDrv.lib` - 靜態連結庫
- `AdvMotDrv.dll` - 動態連結庫

### 初始化範例

```cpp
HAND DevHandle = NULL;
HAND AxisHandle[8] = {NULL};
DEVLIST DevList[10];
U32 DevCount = 0;

// 取得可用設備
Acm_GetAvailableDevs(DevList, 10, &DevCount);

// 開啟設備
Acm_DevOpen(DevList[0].DevNum, &DevHandle, 0);

// 開啟軸
for (int i = 0; i < 8; i++) {
    Acm_AxOpen(DevHandle, i, &AxisHandle[i]);
}
```

### DIO 操作範例

```cpp
U8 DiData, DoData;

// 讀取 DI
Acm_DaqDiGetByte(DevHandle, 0, &DiData);

// 寫入 DO
DoData = 0x55;
Acm_DaqDoSetByte(DevHandle, 0, DoData);
```

---

## 與 TLaneIO 整合

在 HT9045 專案中，PCI-1203 透過 `TLaneIO` 封裝使用。當 `ISABase == ePCI1203` 時：

- 初始化透過 `Acm_DevOpen()` 進行
- DIO 操作透過 `Acm_DaqDi*()` / `Acm_DaqDo*()` 函式
- 關閉透過 `Acm_DevClose()`

詳見 [io-classes.md](io-classes.md) 中 `TLaneIO` 類別的 `ePCI1203` 相關方法。
