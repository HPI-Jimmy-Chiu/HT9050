# MotionNet API 參考（ICP-DAS / 泓格 PISO-MN200）

ICP-DAS（泓格）MotionNet 系統 API 參考文件，適用於 **PISO-MN200** Master 卡。

> **供應商**: ICP-DAS（泓格）  
> **原廠手冊**: `PISO-MN200_Function_Reference_1.0TC_20200827.pdf`（位於 `u:\共用區\HT9045W_相關料件技術文件\IO\泓格\MotionNet\`）  
> **另見**: [motionnet-api.md](motionnet-api.md)（SYN-TEK / 先達）

---

## 目錄

1. [系統限制](#系統限制)
2. [系統架構](#系統架構)
3. [初始化 API](#初始化-api)
4. [DIO 操作 API](#dio-操作-api)
5. [運動控制 API](#運動控制-api)
6. [診斷 API](#診斷-api)
7. [錯誤碼](#錯誤碼)

---

## 系統限制

| 項目 | 限制 | 說明 |
|------|------|------|
| **每張卡片通訊線數** | 2 | 每張 PISO-MN200 有 2 條 Motionnet 通訊線 |
| **每通訊線裝置數** | 64 | DevNo 範圍 0~63（由 DIP Switch 設定） |
| **每裝置 Port 數** | 4 | PortNo 範圍 0~3 |
| **每 Port Bit 數** | 8 | BitNo 範圍 0~7 |
| **通訊速度選項** | 4 種 | 2.5M / 5M / 10M / 20M bps |

### 定址容量計算

```
每張卡片最大 IO 點數 = 2 Line × 64 Dev × 4 Port × 8 Bit = 4096 點
```

---

## 系統架構

```
┌──────────────────────────────────────────────────────────┐
│                       PISO-MN200                          │
│                    (PCI 介面卡)                           │
│                                                           │
│   ┌───────────────┐           ┌───────────────┐          │
│   │   Line 0      │           │   Line 1      │          │
│   │  (Card ID×2)  │           │ (Card ID×2+1) │          │
│   └───────┬───────┘           └───────┬───────┘          │
└───────────┼───────────────────────────┼──────────────────┘
            │                           │
            ▼                           ▼
     ┌──────────────────────────────────────────────┐
     │              Motionnet Bus                    │
     │  ┌─────┐  ┌─────┐  ┌───────┐  ┌───────┐     │
     │  │Servo│→ │Servo│→ │32DI/DO│→ │16DI/DO│→... │
     │  │Dev0 │  │Dev1 │  │  Dev2 │  │  Dev3 │     │
     │  └─────┘  └─────┘  └───────┘  └───────┘     │
     └──────────────────────────────────────────────┘
```

### Card ID 與通訊線編號對應

| Card ID (DIP Switch) | Line 0 | Line 1 |
|---------------------|--------|--------|
| 0 | No.0 | No.1 |
| 1 | No.2 | No.3 |
| 15 | No.30 | No.31 |

---

## 初始化 API

### 開啟與關閉

| 函式 | 說明 |
|------|------|
| `mn_open_all(&pNumLine)` | 掃描所有可用的 Motionnet 板卡，回傳通訊線數量 |
| `mn_close_all()` | 關閉所有板卡使用權，釋放資源 |

### 取得板卡資訊

| 函式 | 說明 |
|------|------|
| `mn200_get_lineinfo(bScannedIndex, &pLineNo)` | 依掃描索引取得通訊線編號 |
| `mn200_get_cardinfo(bScannedIndex, &pCardID)` | 依掃描索引取得 Card ID |

### 通訊線控制

| 函式 | 說明 |
|------|------|
| `mn_reset(bLineNo)` | 重置通訊線，暫存器回預設值 |
| `mn_set_comm_speed(bLineNo, bCommSpeed)` | 設定傳輸速度 |
| `mn_start_line(bLineNo, &pNumDev)` | 開始通訊，回傳連接裝置數 |
| `mn_stop_line(bLineNo)` | 停止通訊 |

### 通訊速度常數

| 常數 | 說明 |
|------|------|
| `COMMSPEED_2_5M` | 2.5 Mbps |
| `COMMSPEED_5M` | 5 Mbps |
| `COMMSPEED_10M` | 10 Mbps |
| `COMMSPEED_20M` | 20 Mbps |

### 初始化流程

```
mn_open_all()                    // 掃描板卡
        ↓
mn200_get_lineinfo()             // 取得通訊線編號
        ↓
mn_set_comm_speed()              // 設定傳輸速度
        ↓
mn_start_line()                  // 開始通訊
        ↓
mn_set_motion_cfg()              // 設定運動參數（若有馬達控制）
        ↓
   ... 操作 IO/Motion ...
        ↓
mn_stop_line()                   // 停止通訊
        ↓
mn_close_all()                   // 關閉板卡
```

---

## DIO 操作 API

### 標準 DI/DO（含通訊狀態檢查）

| 函式 | 說明 |
|------|------|
| `mn200_get_di(bLineNo, bDevNo, &wData)` | 讀取 16-bit DI |
| `mn200_get_do(bLineNo, bDevNo, &wData)` | 讀取 16-bit DO 狀態 |
| `mn_set_do_bit(bLineNo, bDevNo, bBitNo, bData)` | 設定單一位元 DO |
| `mn_get_do_bit(bLineNo, bDevNo, bBitNo, &bData)` | 讀取單一位元 DO |
| `mn_set_do_byte(bLineNo, bDevNo, bByteNo, bData)` | 設定 8-bit DO |
| `mn_get_do_byte(bLineNo, bDevNo, bByteNo, &bData)` | 讀取 8-bit DO |
| `mn_set_do_word(bLineNo, bDevNo, bWordNo, wData)` | 設定 16-bit DO |
| `mn_get_do_word(bLineNo, bDevNo, bWordNo, &wData)` | 讀取 16-bit DO |

### 進階 DI/DO（高效能，無通訊檢查）

> **注意**: 進階函式不包含通訊狀態檢查，需搭配 `mn_get_line_status()` 與 `mn_get_slave_error_table()` 使用。

| 函式 | 說明 |
|------|------|
| `mn_get_port_bit(bLineNo, bDevNo, bPortNo, bBitNo, &bData)` | 讀取指定埠的單一位元 |
| `mn_set_port_bit(bLineNo, bDevNo, bPortNo, bBitNo, bData)` | 設定指定埠的單一位元 |
| `mn_get_port_byte(bLineNo, bDevNo, bPortNo, &bData)` | 讀取指定埠 8-bit |
| `mn_set_port_byte(bLineNo, bDevNo, bPortNo, bData)` | 設定指定埠 8-bit |

### Port 編號對應

| 裝置類型 | Port 0 | Port 1 | Port 2 | Port 3 |
|----------|--------|--------|--------|--------|
| 32 DI | DI 0~7 | DI 8~15 | DI 16~23 | DI 24~31 |
| 32 DO | DO 0~7 | DO 8~15 | DO 16~23 | DO 24~31 |
| 16DI/16DO | DI 0~7 | DI 8~15 | DO 0~7 | DO 8~15 |

---

## 運動控制 API

### 基本運動

| 函式 | 說明 |
|------|------|
| `mn_velocity_move(bLineNo, bDevNo, SpeedPar, bDirection)` | 連續速度運動（直到極限或停止） |
| `mn_fix_move(bLineNo, bDevNo, SpeedPar, Position, bMoveType)` | 固定脈波運動 |
| `mn_stop_move(bLineNo, bDevNo, bStopMode)` | 停止運動 |

### 原點復歸

| 函式 | 說明 |
|------|------|
| `mn_home_start(bLineNo, bDevNo, SpeedPar, bDirection, bHomeMode, bEZcount)` | 開始尋找原點 |
| `mn_leave_home(bLineNo, bDevNo, SpeedPar, bDirection, bHomeMode, bEZcount)` | 離開原點 |
| `mn_home_search(bLineNo, bDevNo, SpeedPar, bDirection, OrgWidth, bHomeMode, bEZcount)` | 原點搜尋 |

### 群組運動

| 函式 | 說明 |
|------|------|
| `mn_set_group(bLineNo, bGrpNo, bNumDev, bDevNo[])` | 設定群組（確保插補同步） |
| `mn_get_group(bLineNo, bGrpNo, &pNumDev, bDevNo[])` | 取得群組資訊 |
| `mn_group_stop_move(bLineNo, bGrpNo, bStopMode)` | 群組停止 |
| `mn_group_hold_move(bLineNo, bGrpNo)` | 群組暫停 |
| `mn_group_start_move(bLineNo, bGrpNo)` | 群組啟動 |

### 速度參數結構 (SPEED_PAR)

```cpp
struct SPEED_PAR {
    DWORD Start_Speed;      // 起始/停止速度 (PPS)
    DWORD Drive_Speed;      // 運動速度 (PPS)
    DWORD Correction_Speed; // 歸原點低速（模式 1,4,6,7）
    BYTE  AccDec_Mode;      // ADC_MODE_RATE 或 ADC_MODE_TIME
    DWORD Acc;              // 加速度
    DWORD Dec;              // 減速度
    BYTE  SCurve_Enable;    // ENABLE_FEATURE / DISABLE_FEATURE
    DWORD SCurveAcc_Sect;   // S 曲線加速區域
    DWORD SCurveDec_Sect;   // S 曲線減速區域
    BYTE  Max_Speed;        // 最高速度限制（MaxSpeed 列舉）
};
```

### 運動方向常數

| 常數 | 說明 |
|------|------|
| `MOVE_DIRECTION_FORWARD` | 正向 |
| `MOVE_DIRECTION_REVERSE` | 反向 |

### 停止模式常數

| 常數 | 說明 |
|------|------|
| `SLOWDOWN_STOP` | 減速停止 |
| `SUDDEN_STOP` | 立即停止 |

### 固定脈波運動模式

| 常數 | 值 | 說明 |
|------|-----|------|
| `FIX_MOVE_MODE_REL` | 0x41 | 相對脈波運動 |
| `FIX_MOVE_MODE_ABS_BY_OUTPLS` | 0x42 | 絕對運動（輸出脈波計數器） |
| `FIX_MOVE_MODE_ABS_BY_ENC` | 0x43 | 絕對運動（編碼器計數器） |
| `FIX_MOVE_MODE_ZERO_RETURN_BY_OUTPLS` | 0x44 | 歸零（輸出脈波） |
| `FIX_MOVE_MODE_ZERO_RETURN_BY_ENC` | 0x45 | 歸零（編碼器） |

---

## 診斷 API

| 函式 | 說明 |
|------|------|
| `mn_get_line_status(bLineNo, &wData)` | 取得通訊線狀態 |
| `mn_get_slave_error_table(bLineNo, ErrorTable[2])` | 取得通訊異常裝置列表 |
| `mn_clear_slave_error_flag(bLineNo, ErrorTable[2])` | 清除錯誤旗標 |
| `mn_get_dev_info(bLineNo, bDevNo, &DevInfo)` | 取得裝置資訊 |
| `mn_motion_done(bLineNo, bDevNo, &bDone)` | 檢查運動完成 |

### 通訊線狀態位元

| 位元 | 符號 | 說明 |
|------|------|------|
| 0 | CEND | 暫存器傳輸完成 |
| 1 | BRKF | 接收到 Slave 連線要求 |
| 2 | IOPC | 監控輸入埠訊號變動 |
| 3 | EIOE | I/O 裝置通訊異常 |
| 4 | EDTE | 運動控制裝置通訊異常 |
| 5 | ERAE | I/O 裝置操作異常 |
| 6 | CAER | CPU 資料存取異常 |

---

## 錯誤碼

### 通用錯誤

| 錯誤碼 | 說明 |
|--------|------|
| `SUCCESS` | 執行成功 |
| `ERROR_NO_CARD_FOUND` | 找不到 Motionnet 板卡 |
| `ERROR_INVALID_LINE_NO` | 無效的通訊線編號 |
| `ERROR_INVALID_DEV_NO` | 無效的裝置編號（超出 0~63） |
| `ERROR_NO_DEV_FOUND` | 通訊線上找不到裝置 |
| `ERROR_CARD_ID_DUPLICATED` | Card ID 重複 |

### 通訊錯誤

| 錯誤碼 | 說明 |
|--------|------|
| `ERROR_COMM_NOT_START` | 未開始通訊傳輸 |
| `ERROR_COMM_DISCONNECT` | 通訊中斷 |
| `ERROR_COMM_NOT_STOP` | 無法停止通訊 |

### IO 錯誤

| 錯誤碼 | 說明 |
|--------|------|
| `ERROR_SET_BYTENO` | PortNo 超出範圍 |
| `ERROR_INVALID_BITNO` | BitNo 超出範圍 |
| `ERROR_SET_IO_DEV` | 裝置不是 I/O 模組 |
| `ERROR_SET_MOTION_DEV` | 裝置不是運動控制模組 |

### 運動錯誤

| 錯誤碼 | 說明 |
|--------|------|
| `ERROR_INVALID_MOVE_DIRECTION` | 無效的運動方向 |
| `ERROR_INVALID_STOP_MODE` | 無效的停止模式 |
| `ERROR_INVALID_HOME_MODE` | 無效的原點搜尋模式 |
| `ERROR_INVALID_POSITION` | 位置超出範圍 (-134217728~134217727) |
| `ERROR_MOVE_HOLD` | 裝置處於暫停模式 |
| `ERROR_EMG_SIGNAL_ON` | EMG 訊號觸發 |
| `ERROR_ALM_SIGNAL_ON` | ALARM 訊號觸發 |
| `ERROR_MEL_SIGNAL_ON` | 硬體負極限觸發 |
| `ERROR_PEL_SIGNAL_ON` | 硬體正極限觸發 |
| `ERROR_REGISTER_FULL` | 連續運動暫存器已滿 |

### 速度參數錯誤

| 錯誤碼 | 說明 |
|--------|------|
| `ERROR_START_SPEED_EXCEED_DRIVING_SPEED` | 起始速度大於運動速度 |
| `ERROR_INVALID_MAX_SPEED_SELECTION` | 無效的最高速度選擇 |
| `ERROR_SET_START_SPEED_OUT_RANGE` | 起始速度超出範圍 |
| `ERROR_SET_DRIVING_SPEED_OUT_RANGE` | 運動速度超出範圍 |
| `ERROR_INVALID_ACC_DATA` | 加速度為零 |
| `ERROR_SET_ACC_OUT_RANGE` | 加速度超出範圍 |
| `ERROR_SET_DEC_OUT_RANGE` | 減速度超出範圍 |

---

## BCB6 使用方式

### 標頭檔

```cpp
#include "MN200.h"       // 主標頭檔
```

### 連結庫

- `MN200.lib` - 靜態連結庫
- `MN200.dll` - 動態連結庫

### 初始化範例

```cpp
BYTE NumLine = 0;
BYTE LineNo = 0;
BYTE NumDev = 0;

// 開啟所有板卡
mn_open_all(&NumLine);

// 取得第一張卡片的通訊線編號
mn200_get_lineinfo(0, &LineNo);

// 設定通訊速度
mn_set_comm_speed(LineNo, COMMSPEED_10M);

// 開始通訊
mn_start_line(LineNo, &NumDev);
```

### DIO 操作範例

```cpp
BYTE bData;
WORD wData;

// 讀取 DI（16-bit）
mn200_get_di(LineNo, DevNo, &wData);

// 設定單一位元 DO
mn_set_do_bit(LineNo, DevNo, 5, 1);  // Bit 5 = ON

// 使用進階函式（高效能）
mn_set_port_byte(LineNo, DevNo, 2, 0x55);  // Port 2 = 0x55
mn_get_port_bit(LineNo, DevNo, 0, 3, &bData);  // Port 0, Bit 3
```

---

## 與 SYN-TEK 版本差異

| 項目 | ICP-DAS (MN200) | SYN-TEK (PCI-L132/M114G) |
|------|-----------------|--------------------------|
| 定址方式 | bLineNo, bDevNo, bPortNo, bBitNo | RingNo, SlaveIP, PortNo, Bit |
| 初始化 | `mn_open_all()` → `mn_start_line()` | `_m114g_initial()` → `_mnet_start_ring()` |
| DIO 讀取 | `mn_get_port_bit()` | `_mnet_io_input()` |
| DIO 輸出 | `mn_set_port_bit()` | `_mnet_io_output()` |
| 通訊速度 | 可設定 2.5M~20M | 固定 |
| 運動控制 | 內建（MN-SERVO 系列） | 需搭配運動控制卡 |

---

## 與 TLaneIO 整合

在 HT9045 專案中，`TLaneIO` 會根據 `ISABase` 類型選擇適當的 API：

| ISABase | 供應商 | API 前綴 |
|---------|--------|----------|
| `eMotionNet` (0) | SYN-TEK 或 ICP-DAS | `_mnet_` 或 `mn_` |

詳見 [io-classes.md](io-classes.md) 中 `TLaneIO` 類別說明。
