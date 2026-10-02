# PISO-MN200 MotionNet API 參考

泓格 PISO-MN200 運動控制卡 API 參考文件，適用於 HT9045 步進/伺服馬達控制。

---

## 系統架構

### 通訊線拓撲

```
PISO-MN200 (主卡)
    │
    ├── Line 0 ─┬── Dev 0  (軸 0)
    │           ├── Dev 1  (軸 1)
    │           ├── ...
    │           └── Dev 63 (軸 63)
    │
    └── Line 1 ─┬── Dev 0
                └── ...
```

### 全域變數（HT9045 使用）

```cpp
BYTE NumLine;                    // 通訊線數量
DWORD MN_200_ErrorTable[4];      // 錯誤表
bool bResetMNet;                 // 重置旗標
SPEED_PAR MN200SpeedPar;         // 速度參數結構
```

---

## 速度參數結構

```cpp
typedef struct _SPEED_PAR
{
    MaxSpeed    Max_Speed;           // 速度倍率選擇
    double      Start_Speed;         // 起始/停止速度 (PPS)
    double      Drive_Speed;         // 運轉速度 (PPS)
    double      Correction_Speed;    // 修正速度 (PPS)
    double      Acc;                 // 加速度 (s 或 PPS/S)
    double      Dec;                 // 減速度 (s 或 PPS/S)
    BYTE        AccDec_Mode;         // ADC_MODE_RATE / ADC_MODE_TIME
    BYTE        SCurve_Enable;       // S 曲線啟用
    double      SCurveAcc_Sect;      // S 曲線加速區段
    double      SCurveDec_Sect;      // S 曲線減速區段
} SPEED_PAR;
```

### MaxSpeed 列舉

| 列舉值 | 最大速度 |
|--------|----------|
| `MAXSPEED_10K` | 10,000 PPS |
| `MAXSPEED_20K` | 20,000 PPS |
| `MAXSPEED_50K` | 50,000 PPS |
| `MAXSPEED_100K` | 100,000 PPS |
| `MAXSPEED_200K` | 200,000 PPS |
| `MAXSPEED_500K` | 500,000 PPS |
| `MAXSPEED_1M` | 1,000,000 PPS |
| `MAXSPEED_2M` | 2,000,000 PPS |
| `MAXSPEED_5M` | 5,000,000 PPS |
| `MAXSPEED_6M` | 6,000,000 PPS |

### AccDec_Mode 模式

| 模式 | 值 | 說明 |
|------|-----|------|
| `ADC_MODE_RATE` | 0 | 加減速以脈波數/秒² 指定 |
| `ADC_MODE_TIME` | 1 | 加減速以秒指定 |
| `ADC_MODE_RATE_NOACC` | 2 | 不加速 |
| `ADC_MODE_TIME_NOACC` | 3 | 不加速（時間模式）|
| `ADC_MODE_RATE_NODEC` | 4 | 不減速 |
| `ADC_MODE_TIME_NODEC` | 5 | 不減速（時間模式）|

---

## 系統初始化函式

### mn_open_all

```cpp
short mn_open_all(BYTE* pNumLine);
```

**功能**：開啟所有 MotionNet 卡片

| 參數 | 說明 |
|------|------|
| `pNumLine` | 回傳找到的通訊線數量 |
| **回傳** | 0=成功，負值=錯誤碼 |

### mn_close_all

```cpp
short mn_close_all();
```

**功能**：關閉所有 MotionNet 卡片

### mn_start_line

```cpp
short mn_start_line(BYTE bLineNo, BYTE* pNumDev);
```

**功能**：啟動指定通訊線

| 參數 | 說明 |
|------|------|
| `bLineNo` | 通訊線編號 |
| `pNumDev` | 回傳該線的裝置數量 |

### mn_stop_line

```cpp
short mn_stop_line(BYTE bLineNo);
```

**功能**：停止指定通訊線

### mn_reset

```cpp
short mn_reset(BYTE bLineNo);
```

**功能**：重置指定通訊線

### mn_set_comm_speed

```cpp
short mn_set_comm_speed(BYTE bLineNo, BYTE bCommSpeed);
```

**功能**：設定通訊速度

| bCommSpeed | 速度 |
|------------|------|
| `COMMSPEED_2_5M` | 2.5 MHz |
| `COMMSPEED_5M` | 5 MHz |
| `COMMSPEED_10M` | 10 MHz |
| `COMMSPEED_20M` | 20 MHz |

---

## 硬體配置函式

### mn_set_motion_cfg

```cpp
short mn_set_motion_cfg(BYTE bLineNo, BYTE bDevNo, MotionConfig CfgItem, DWORD dwData);
```

**功能**：設定運動控制模組配置

### MotionConfig 配置項目

| 項目 | 值 | 說明 |
|------|-----|------|
| `PULSE_MODE` | 0 | 脈波輸出模式（0~7）|
| `EL_PROC` | 3 | 極限開關處理（SLOWDOWN/SUDDEN_STOP）|
| `SD_ENA` | 31 | 減速點功能啟用 |
| `SD_PROC` | 4 | 減速點處理方式 |
| `ORG_LOGIC` | 7 | 原點觸發邏輯 |
| `ALM_PROC` | 8 | Alarm 處理方式 |
| `ALM_LOGIC` | 9 | Alarm 觸發邏輯 |
| `ERC_ERR_ENA` | 10 | ERC 錯誤啟用 |
| `ERC_ORG_ENA` | 11 | ERC 原點啟用 |
| `ERC_LEN` | 12 | ERC 脈衝寬度 |
| `ERC_LOGIC` | 15 | ERC 邏輯 |
| `INP_ENA` | 18 | INP 啟用 |
| `INP_LOGIC` | 19 | INP 邏輯 |
| `ENC_MODE` | 23 | 編碼器模式 |
| `ENC_REV_ENA` | 25 | 編碼器反向 |
| `ENC_Z_LOGIC` | 26 | 編碼器 Z 相邏輯 |
| `OUTPLS_REV_ENA` | 27 | 輸出脈波反向 |

### 脈波輸出模式（PULSE_MODE）

| 模式 | 值 | 說明 |
|------|-----|------|
| `PULSE_MODE_PULSE_LOGIC_LOW_DIR_FORWARD_HIGH` | 0 | 脈波低電位有效，方向高=正向 |
| `PULSE_MODE_PULSE_LOGIC_HIGH_DIR_FORWARD_HIGH` | 1 | 脈波高電位有效，方向高=正向 |
| `PULSE_MODE_PULSE_LOGIC_LOW_DIR_FORWARD_LOW` | 2 | 脈波低電位有效，方向低=正向 |
| `PULSE_MODE_PULSE_LOGIC_HIGH_DIR_FORWARD_LOW` | 3 | 脈波高電位有效，方向低=正向 |
| `PULSE_MODE_CW_LOGIC_LOW` | 4 | CW/CCW 低電位有效 |
| `PULSE_MODE_A_LEAD_B` | 5 | A 相領先 B 相 |
| `PULSE_MODE_A_LAG_B` | 6 | A 相落後 B 相 |
| `PULSE_MODE_CW_LOGIC_HIGH` | 7 | CW/CCW 高電位有效 |

### 編碼器模式（ENC_MODE）

| 模式 | 值 | 說明 |
|------|-----|------|
| `ENCODER_MODE_AB` | 0 | AB 相正交脈波 |
| `ENCODER_MODE_AB_MULT_2` | 1 | AB 相 ×2 |
| `ENCODER_MODE_AB_MULT_4` | 2 | AB 相 ×4 |
| `ENCODER_MODE_CW_CCW` | 3 | CW/CCW 脈波 |
| `ENCODER_MODE_PULSE_DIR` | 4 | 脈波/方向 |

### mn_set_softlimit

```cpp
short mn_set_softlimit(BYTE bLineNo, BYTE bDevNo, BYTE bSWLimitEnable, 
                       BYTE bCmpSource, BYTE bStopMode, 
                       long LimitPositive, long LimitNegaitive);
```

**功能**：設定軟體極限

| 參數 | 說明 |
|------|------|
| `bSWLimitEnable` | ENABLE_FEATURE / DISABLE_FEATURE |
| `bCmpSource` | PULSE_COMMAND / ENCODER_POSITION |
| `bStopMode` | SLOWDOWN_STOP / SUDDEN_STOP |
| `LimitPositive` | 正極限位置 |
| `LimitNegaitive` | 負極限位置 |

### mn_servo_on

```cpp
short mn_servo_on(BYTE bLineNo, BYTE bDevNo, BYTE bServoOn);
```

**功能**：伺服 ON/OFF

| bServoOn | 動作 |
|----------|------|
| `TURN_ON` (1) | 伺服 ON |
| `TURN_OFF` (0) | 伺服 OFF |

### mn_alarm_reset

```cpp
short mn_alarm_reset(BYTE bLineNo, BYTE bDevNo, BYTE bAlmRstOn);
```

**功能**：Alarm 重置

---

## 獨立運動函式

### mn_fix_move

```cpp
short mn_fix_move(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                  long Position, BYTE bMoveType);
```

**功能**：定點移動

| bMoveType | 值 | 說明 |
|-----------|-----|------|
| `FIX_MOVE_MODE_REL` | 0x41 | 相對移動 |
| `FIX_MOVE_MODE_ABS_BY_OUTPLS` | 0x42 | 絕對移動（脈波計數）|
| `FIX_MOVE_MODE_ABS_BY_ENC` | 0x43 | 絕對移動（編碼器）|
| `FIX_MOVE_MODE_ZERO_RETURN_BY_OUTPLS` | 0x44 | 回零（脈波）|
| `FIX_MOVE_MODE_ZERO_RETURN_BY_ENC` | 0x45 | 回零（編碼器）|

### mn_velocity_move

```cpp
short mn_velocity_move(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                       BYTE bDirection);
```

**功能**：速度移動（JOG）

| bDirection | 說明 |
|------------|------|
| `MOVE_DIRECTION_FORWARD` (1) | 正向 |
| `MOVE_DIRECTION_REVERSE` (0) | 負向 |

### mn_stop_move

```cpp
short mn_stop_move(BYTE bLineNo, BYTE bDevNo, BYTE bStopMode);
```

**功能**：停止移動

| bStopMode | 說明 |
|-----------|------|
| `SUDDEN_STOP` (0) | 急停 |
| `SLOWDOWN_STOP` (1) | 減速停止 |

### mn_change_v

```cpp
short mn_change_v(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                  BYTE bWaitCmpEnable);
```

**功能**：運動中變更速度

### mn_change_p

```cpp
short mn_change_p(BYTE bLineNo, BYTE bDevNo, long Position);
```

**功能**：運動中變更目標位置

---

## 原點搜尋函式

### mn_home_start

```cpp
short mn_home_start(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                    BYTE bDirection, BYTE bHomeMode, BYTE bEZcount);
```

**功能**：開始原點搜尋

| 參數 | 說明 |
|------|------|
| `bDirection` | 搜尋方向 |
| `bHomeMode` | 原點模式（0~12）|
| `bEZcount` | EZ 計數（0~15）|

### mn_leave_home

```cpp
short mn_leave_home(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                    BYTE bDirection, BYTE bHomeMode, BYTE bEZcount);
```

**功能**：離開原點

### mn_home_search

```cpp
short mn_home_search(BYTE bLineNo, BYTE bDevNo, SPEED_PAR* pSpeedPar, 
                     BYTE bDirection, long OrgWidth, 
                     BYTE bHomeMode, BYTE bEZcount);
```

**功能**：原點搜尋（含寬度）

---

## 補間運動函式

### mn_line2_move

```cpp
short mn_line2_move(BYTE bLineNo, BYTE bDev1No, BYTE bDev2No, 
                    SPEED_PAR* pSpeedPar, 
                    long Dev1Pos, long Dev2Pos, BYTE bCnstSpdEnable);
```

**功能**：二軸線性補間

### mn_line3_move

```cpp
short mn_line3_move(BYTE bLineNo, BYTE bDev1No, BYTE bDev2No, BYTE bDev3No,
                    SPEED_PAR* pSpeedPar, 
                    long Dev1Pos, long Dev2Pos, long Dev3Pos);
```

**功能**：三軸線性補間

### mn_linen_move

```cpp
short mn_linen_move(BYTE bLineNo, BYTE bDevNo[], SPEED_PAR* pSpeedPar, 
                    long DevPos[], BYTE bNumDev);
```

**功能**：N 軸線性補間

### mn_arc2_move

```cpp
short mn_arc2_move(BYTE bLineNo, BYTE bDev1No, BYTE bDev2No, 
                   SPEED_PAR* pSpeedPar, BYTE bDirection,
                   long Dev1CenterPos, long Dev2CenterPos, 
                   long Dev1FinishPos, long Dev2FinishPos,
                   DWORD Low32BitDummyDevNo, DWORD High32BitDummyDevNo,
                   BYTE bCnstSpdEnable);
```

**功能**：二軸圓弧補間

---

## 群組運動函式

### mn_set_group

```cpp
short mn_set_group(BYTE bLineNo, BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[]);
```

**功能**：設定群組

### mn_group_stop_move

```cpp
short mn_group_stop_move(BYTE bLineNo, BYTE bGrpNo, BYTE bStopMode);
```

**功能**：群組停止

### mn_group_hold_move

```cpp
short mn_group_hold_move(BYTE bLineNo, BYTE bGrpNo);
```

**功能**：群組暫停

### mn_group_start_move

```cpp
short mn_group_start_move(BYTE bLineNo, BYTE bGrpNo);
```

**功能**：群組啟動（從暫停恢復）

---

## 狀態函式

### mn_motion_done

```cpp
short mn_motion_done(BYTE bLineNo, BYTE bDevNo, BYTE* pDone);
```

**功能**：檢查運動是否完成

| *pDone | 說明 |
|--------|------|
| `MOTION_DONE` (1) | 運動完成 |
| `MOTION_NOT_DONE` (0) | 運動中 |

### mn_get_cmdcounter

```cpp
short mn_get_cmdcounter(BYTE bLineNo, BYTE bDevNo, long* pData);
```

**功能**：讀取命令位置計數器

### mn_get_enccounter

```cpp
short mn_get_enccounter(BYTE bLineNo, BYTE bDevNo, long* pData);
```

**功能**：讀取編碼器位置計數器

### mn_set_cmdcounter

```cpp
short mn_set_cmdcounter(BYTE bLineNo, BYTE bDevNo, long Data);
```

**功能**：設定命令位置計數器

### mn_set_enccounter

```cpp
short mn_set_enccounter(BYTE bLineNo, BYTE bDevNo, long Data);
```

**功能**：設定編碼器位置計數器

### mn_get_speed

```cpp
short mn_get_speed(BYTE bLineNo, BYTE bDevNo, double* pData);
```

**功能**：讀取目前速度

### mn_get_mdio_status

```cpp
short mn_get_mdio_status(BYTE bLineNo, BYTE bDevNo, MOTION_IO* MotionIO);
```

**功能**：讀取運動 I/O 狀態

### MOTION_IO 結構

```cpp
typedef struct _MOTION_DEV_IO
{
    BYTE SVON;      // 伺服 ON 狀態
    BYTE RESET_ALM; // Alarm 重置狀態
    BYTE RDY;       // 就緒狀態
    BYTE ALM;       // Alarm 狀態
    BYTE PEL;       // 正極限狀態
    BYTE MEL;       // 負極限狀態
    BYTE ORG;       // 原點狀態
    BYTE SDLTC;     // SD/LTC 狀態
    BYTE SDIN;      // SD 輸入狀態
    BYTE INP;       // INP 狀態
    BYTE EMG;       // 緊急停止狀態
    BYTE EZ;        // 編碼器 Z 相狀態
    BYTE ERC;       // ERC 狀態
} MOTION_IO;
```

### mn_get_error_status

```cpp
short mn_get_error_status(BYTE bLineNo, BYTE bDevNo, DWORD* pData);
```

**功能**：讀取錯誤狀態

### 錯誤狀態位元

| 位元遮罩 | 說明 |
|----------|------|
| `ERR_STATUS_SW_PEL_STOP` (0x01) | 軟體正極限停止 |
| `ERR_STATUS_SW_MEL_STOP` (0x02) | 軟體負極限停止 |
| `ERR_STATUS_PEL_STOP` (0x08) | 硬體正極限停止 |
| `ERR_STATUS_MEL_STOP` (0x10) | 硬體負極限停止 |
| `ERR_STATUS_ALM_STOP` (0x20) | Alarm 停止 |
| `ERR_STATUS_EMG_STOP` (0x80) | 緊急停止 |
| `ERR_STATUS_SD_STOP` (0x100) | 減速停止 |

---

## 六軸步進模組專用函式

### mn_step6_set_home_check

```cpp
short mn_step6_set_home_check(BYTE bLineNo, BYTE bFirstDevNo, 
                              BYTE bHomeLogic, BYTE bEnableDevIndexBits);
```

**功能**：設定六軸步進模組的原點檢查

| 參數 | 說明 |
|------|------|
| `bFirstDevNo` | 模組第一軸編號（0~63）|
| `bHomeLogic` | `STEP6_HOME_NORMAL_OPEN` (0) / `STEP6_HOME_NORMAL_CLOSE` (1) |
| `bEnableDevIndexBits` | 軸致能位元（0x3F = 六軸全開）|

**致能位元對應**：

| Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|-------|-------|-------|-------|-------|-------|
| 第六軸 | 第五軸 | 第四軸 | 第三軸 | 第二軸 | 第一軸 |

### mn_step6_set_micro_step

```cpp
short mn_step6_set_micro_step(BYTE bLineNo, BYTE bFirstDevNo, BYTE bMicroStep);
```

**功能**：設定微步進模式

| bMicroStep | 值 | 說明 |
|------------|-----|------|
| `STEP6_MICRO_STEP_FULL` | 0 | 全步進 |
| `STEP6_MICRO_STEP_ONE_HALF` | 1 | 半步進 |
| `STEP6_MICRO_STEP_ONE_QUARTER` | 2 | 1/4 步進 |
| `STEP6_MICRO_STEP_ONE_EIGHTH` | 4 | 1/8 步進 |
| `STEP6_MICRO_STEP_ONE_SIXTEENTH` | 5 | 1/16 步進 |
| `STEP6_MICRO_STEP_ONE_THIRTY_SECOND` | 6 | 1/32 步進 |

### mn_step6_set_current

```cpp
short mn_step6_set_current(BYTE bLineNo, BYTE bFirstDevNo, BYTE bCurrentMode);
```

**功能**：設定驅動電流模式

| bCurrentMode | 值 | 說明 |
|--------------|-----|------|
| `STEP6_SET_CURRENT_ALL_NORMAL` | 0 | 六軸驅動電流正常 |
| `STEP6_SET_CURRENT_AXIS_4_LOW` | 1 | 第五軸低電流（0.95A）|
| `STEP6_SET_CURRENT_AXIS_4_5_LOW` | 2 | 第五、六軸低電流 |
| `STEP6_SET_CURRENT_ALL_NORMAL_` | 3 | 六軸驅動電流正常 |

**備註**：低電流模式下，運轉電流固定為 0.95A。停止後約 0.4 秒待機電流下降至設定比率（75%、50% 或 25%）。

---

## 數位 I/O 函式

### 並列 I/O（板卡端）

```cpp
short mn200_get_di(BYTE bCardID, BYTE* pData);
short mn200_set_do(BYTE bCardID, BYTE bData);
short mn200_get_do(BYTE bCardID, BYTE* pData);
```

### 串列 I/O（Bit 操作）

```cpp
short mn_get_di_bit(BYTE bLineNo, BYTE bDevNo, BYTE bBitNo, BYTE* pData);
short mn_set_do_bit(BYTE bLineNo, BYTE bDevNo, BYTE bBitNo, BYTE bData);
short mn_get_do_bit(BYTE bLineNo, BYTE bDevNo, BYTE bBitNo, BYTE* pData);
```

### 串列 I/O（Byte 操作）

```cpp
short mn_get_di_byte(BYTE bLineNo, BYTE bDevNo, BYTE bByteNo, BYTE* pData);
short mn_set_do_byte(BYTE bLineNo, BYTE bDevNo, BYTE bByteNo, BYTE bData);
short mn_get_do_byte(BYTE bLineNo, BYTE bDevNo, BYTE bByteNo, BYTE* pData);
```

### 串列 I/O（Word 操作）

```cpp
short mn_get_di_word(BYTE bLineNo, BYTE bDevNo, BYTE bWordNo, WORD* pData);
short mn_set_do_word(BYTE bLineNo, BYTE bDevNo, BYTE bWordNo, WORD wData);
short mn_get_do_word(BYTE bLineNo, BYTE bDevNo, BYTE bWordNo, WORD* pData);
```

---

## 類比 I/O 函式

### mn_set_ao

```cpp
short mn_set_ao(BYTE bLineNo, BYTE bDevNo, BYTE bChannelNo, float fData);
```

**功能**：設定類比輸出（-10V ~ +10V）

### mn_get_ai

```cpp
short mn_get_ai(BYTE bLineNo, BYTE bDevNo, BYTE bChannelNo, float* pData);
```

**功能**：讀取類比輸入

### mn_get_ai_all

```cpp
short mn_get_ai_all(BYTE bLineNo, BYTE bDevNo, float fData[]);
```

**功能**：讀取所有類比輸入通道

---

## 錯誤碼參考

### 系統錯誤碼

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| 0 | `SUCCESS` | 成功 |
| -100 | `ERROR_NO_CARD_FOUND` | 找不到 MotionNet 板卡 |
| -101 | `ERROR_IOCTL_FAILED` | 驅動控制代碼錯誤 |
| -102 | `ERROR_INVALID_LINE_NO` | 無效的通訊線編號 |
| -103 | `ERROR_COMM_NOT_START` | 通訊未啟動 |
| -104 | `ERROR_INVALID_DEV_NO` | 無效的裝置編號（0~63）|
| -105 | `ERROR_NO_DEV_FOUND` | 找不到裝置 |
| -106 | `ERROR_SET_IO_DEV` | 裝置為 I/O 模組，非運動控制 |
| -107 | `ERROR_SET_MOTION_DEV` | 裝置為運動控制模組，非 I/O |

### 速度參數錯誤

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| -108 | `ERROR_START_SPEED_EXCEED_DRIVING_SPEED` | 起始速度 > 運轉速度 |
| -109 | `ERROR_INVALID_MAX_SPEED_SELECTION` | 無效的 Max_Speed |
| -110 | `ERROR_SET_START_SPEED_OUT_RANGE` | 起始速度超出範圍 |
| -111 | `ERROR_SET_DRIVING_SPEED_OUT_RANGE` | 運轉速度超出範圍 |
| -112 | `ERROR_INVALID_SCURVE_ENABLE` | 無效的 S 曲線設定 |
| -113 | `ERROR_INVALID_ADC_MODE` | 無效的加減速模式 |
| -114 | `ERROR_INVALID_ACC_DATA` | 加速度值為零 |
| -115 | `ERROR_SET_ACC_DOUBLE_DEC` | 加速度 > 2×減速度 |
| -116 | `ERROR_SET_ACC_OUT_RANGE` | 加速度超出範圍 |
| -117 | `ERROR_SET_DEC_OUT_RANGE` | 減速度超出範圍 |
| -120 | `ERROR_SET_CORRECTION_SPD_OUT_RANGE` | 修正速度超出範圍 |

### 運動錯誤

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| -124 | `ERROR_SET_DATA` | 參數值不在正確範圍 |
| -125 | `ERROR_INVALID_CONFIG_ITEM` | 無效的配置項目 |
| -130 | `ERROR_INVALID_MOVE_DIRECTION` | 無效的移動方向 |
| -131 | `ERROR_INVALID_HOME_MODE` | 無效的原點模式（0~12）|
| -132 | `ERROR_INVALID_EZ_COUNT` | 無效的 EZ 計數（0~15）|
| -133 | `ERROR_MOVE_HOLD` | 裝置處於暫停模式 |
| -134 | `ERROR_EMG_SIGNAL_ON` | EMG 訊號觸發 |
| -135 | `ERROR_ALM_SIGNAL_ON` | ALARM 訊號觸發 |
| -136 | `ERROR_MEL_SIGNAL_ON` | 負極限訊號觸發 |
| -137 | `ERROR_PEL_SIGNAL_ON` | 正極限訊號觸發 |
| -143 | `ERROR_INVALID_FIX_MOVE_MODE` | 無效的定點移動模式 |
| -145 | `ERROR_INVALID_POSITION` | 位置超出範圍（±134,217,727）|

### 插補錯誤

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| -146 | `ERROR_INVALID_GROUPNO` | 無效的群組編號（0~7）|
| -147 | `ERROR_INVALID_NUM_DEV` | 無效的裝置數量 |
| -148 | `ERROR_GROUP_ALREADY_HOLD` | 群組已暫停 |
| -149 | `ERROR_SET_ARC_FINISH_POS` | 圓弧終點超出範圍 |
| -152 | `ERROR_GROUP_NOT_HOLD` | 群組未暫停 |

### 通訊錯誤

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| -122 | `ERROR_INVALID_COMM_SPEED` | 無效的通訊速度 |
| -123 | `ERROR_COMM_NOT_STOP` | 無法停止通訊 |
| -165 | `ERROR_CARD_ID_DUPLICATED` | Card ID 重複 |
| -168 | `ERROR_COMM_DISCONNECT` | 通訊中斷 |
| -171 | `ERROR_STEP_HOME_FAILED` | 原點搜尋失敗 |
| -194 | `ERROR_LINE_EDTE_FAULT` | 連續三次以上通訊異常 |
| -195 | `ERROR_LINE_ERAE_FAULT` | Slave 收到錯誤資料 |
| -196 | `ERROR_LINE_CAER_FAULT` | 主卡收到不正確命令 |

---

## HT9045 使用範例

### 初始化馬達

```cpp
int TMyMN200Motor::InitMotor(int IoAddress)
{
    int ret = 0;
    
    // 設定極限處理
    ret = mn_set_motion_cfg(iBoardID, iPortID, EL_PROC, SUDDEN_STOP);
    
    // 設定 Alarm 處理
    ret = mn_set_motion_cfg(iBoardID, iPortID, ALM_PROC, SUDDEN_STOP);
    
    // 停用減速點
    ret = mn_set_motion_cfg(iBoardID, iPortID, SD_ENA, DISABLE_FEATURE);
    
    // 設定原點邏輯
    ret = mn_set_motion_cfg(iBoardID, iPortID, ORG_LOGIC, LOGIC_ACTIVE_LOW);
    
    // 伺服馬達編碼器設定
    if (MotorType == Servo_Motor)
    {
        ret = mn_set_motion_cfg(iBoardID, iPortID, ENC_MODE, ENCODER_MODE_AB_MULT_4);
        ret = mn_set_motion_cfg(iBoardID, iPortID, PULSE_MODE, PULSE_MODE_CW_LOGIC_LOW);
    }
    
    return ret == 0;
}
```

### 絕對位置移動

```cpp
bool TMyMN200Motor::MoveTo(int Tar)
{
    if (!Enable || !MotionDone())
        return false;
    
    SetSpeed(iSpeed);
    
    int ret = mn_fix_move(iBoardID, iPortID, MN200SpeedPar, 
                          Tar, FIX_MOVE_MODE_ABS_BY_OUTPLS);
    
    GetMN200ErrorMessage(ret, "mn_fix_move");
    return ret == 0;
}
```

### JOG 移動

```cpp
bool TMyMN200Motor::JogP()
{
    if (!Enable)
        return false;
    
    BYTE direction = Direction ? MOVE_DIRECTION_REVERSE : MOVE_DIRECTION_FORWARD;
    int ret = mn_velocity_move(iBoardID, iPortID, MN200SpeedPar, direction);
    
    GetMN200ErrorMessage(ret, "mn_velocity_move");
    return ret == 0;
}
```

### 讀取狀態

```cpp
void TMyMN200Motor::ScanMotorStatus(bool *Led)
{
    MOTION_IO MotionIO;
    int ret = mn_get_mdio_status(iBoardID, iPortID, &MotionIO);
    
    Led[iCwLed]    = MotionIO.PEL;
    Led[iCcwLed]   = MotionIO.MEL;
    Led[iHomeLed]  = MotionIO.ORG;
    Led[iAlarmLed] = MotionIO.ALM;
    Led[iInposLed] = !MotionIO.INP;
    Led[iServoOn]  = MotionIO.SVON;
    Led[iEmgLed]   = !(MotionIO.EMG);
}
```

---

## 參考資源

- **手冊**：PISO-MN200_Function_Reference_240202.pdf
- **新增函式**：PISO-MN200_New_Function_Reference_1_0TC (20131129).pdf
- **標頭檔**：mn200.h
- **廠商**：泓格科技（ICP DAS）
