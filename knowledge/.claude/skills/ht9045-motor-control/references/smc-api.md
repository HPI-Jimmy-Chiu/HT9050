# 康泰克 CONTEC SMC 系列軸卡 API 參考

> 對應原始碼：`Motor/mySMCmotor.cpp`、`Motor/mySMCmotor.h`  
> 手冊來源：SMC8DF2_LYTJ34_160426.pdf（CONTEC CO., LTD.）  
> 客製版手冊：120329_HonTech殿向け_SMC-8DF-PCI-C01_設計仕様書_Rev 1 0.doc  
> DLL 標頭檔：`Motor/CSmc.h`、`Motor/CSmcdef.h`

---

## 概述

SMC 系列是康泰克（CONTEC）出品的高速運動控制 PCI 介面軸卡，支援步進馬達與脈波輸入型伺服馬達。HT9045 使用 TMySMCMotor 類別封裝該軸卡的控制功能。

---

## 軸卡型號比較

| 型號 | 軸數 | 最大脈波輸出 | 補間功能 | Frame 序列 | 備註 |
|------|------|--------------|----------|-----------|------|
| SMC-4DF2-PCI | 4 | 6.5Mpps | 線性/圓弧 | 1024 frames/軸 | 標準版 |
| SMC-8DF2-PCI | 8 | 6.5Mpps | 線性/圓弧 | 1024 frames/軸 | 標準版 |
| **SMC-8DF-PCI-C01** | 8 | 6.5Mpps | 線性/圓弧 | 1024 frames/軸 | **鴻勁客製版**，含 FIFO Latch |

---

## 主要功能

- **PTP 定位運動**：點到點定位移動
- **JOG 運動**：連續移動操作
- **ORG 原點復歸**：原點搜尋功能
- **線性補間**：多軸直線插補
- **圓弧補間**：圓弧插補
- **S 曲線加減速**：平滑加減速控制
- **Frame 序列運動**：最多 1024 個 frame 連續執行
- **同步控制**：多軸/多卡同步啟動/停止

---

## I/O 訊號定義

### 極限輸入訊號

| 訊號 | 說明 |
|------|------|
| +LIM | 正方向硬體極限 |
| -LIM | 負方向硬體極限 |
| ORG | 原點感測器 |

### 通用輸入訊號（每軸 7 點）

| 訊號 | 預設功能 | 可選功能 |
|------|---------|----------|
| IN1 | 通用輸入 1 | ALM（伺服異常警報） |
| IN2 | 通用輸入 2 | INP（定位完成） |
| IN3 | 通用輸入 3 | SD（減速輸入） |
| IN4 | 通用輸入 4 | LTC（計數器閂鎖） |
| IN5 | 通用輸入 5 | PCS（定位開始） |
| IN6 | 通用輸入 6 | CLR（計數器清除） |
| IN7 | 通用輸入 7 | - |

### 編碼器輸入

| 訊號 | 說明 |
|------|------|
| A+/A- | 編碼器 A 相差動輸入 |
| B+/B- | 編碼器 B 相差動輸入 |
| Z+/Z- | 編碼器 Z 相（原點）差動輸入 |

### 脈波輸出

| 訊號 | 說明 |
|------|------|
| OUT+/CW+ | 脈波/CW 輸出正端 |
| OUT-/CW- | 脈波/CW 輸出負端 |
| DIR+/CCW+ | 方向/CCW 輸出正端 |
| DIR-/CCW- | 方向/CCW 輸出負端 |

### 通用輸出訊號（每軸 3 點）

| 訊號 | 說明 |
|------|------|
| OUT1 | 通用輸出 1（可用於 ERC） |
| OUT2 | 通用輸出 2 |
| OUT3 | 通用輸出 3 |

---

## API 常數定義（CSmc.h / mySMCmotor.cpp）

### 控制輸入訊號類型 (SmcWSetCtrlTypeIn)

```cpp
const int SMC_CtrlIn_ALM = 0x01;   // 伺服異常警報
const int SMC_CtrlIn_INP = 0x02;   // 定位完成
const int SMC_CtrlIn_SD  = 0x04;   // 減速訊號
const int SMC_CtrlIn_LTC = 0x08;   // 閂鎖訊號
const int SMC_CtrlIn_CTR = 0x10;   // 計數器訊號
const int SMC_CtrlIn_CLR = 0x20;   // 清除訊號
```

### 控制輸出訊號類型 (SmcWSetCtrlTypeOut)

```cpp
const int SMC_CtrlOut_General        = 0;  // 一般輸出
const int SMC_CtrlOut_AlarmClear     = 1;  // 警報清除
const int SMC_CtrlOut_ERC            = 2;  // 編碼器清除
const int SMC_CtrlOut_OutPulseSignal = 3;  // 脈波訊號
const int SMC_CtrlOut_EncoderSignal  = 4;  // 編碼器訊號
const int SMC_CtrlOut_HoldOffSignal  = 5;  // 保持關閉訊號
```

### 脈波輸出模式 (SmcWSetPulseType)

```cpp
const int SMC_Pulse_OutNeg_DirHigh   = 0;  // 脈波負緣，方向高電位
const int SMC_Pulse_OutPos_DirHigh   = 1;  // 脈波正緣，方向高電位
const int SMC_Pulse_OutNeg_DirLow    = 2;  // 脈波負緣，方向低電位
const int SMC_Pulse_OutPos_DirLow    = 3;  // 脈波正緣，方向低電位
const int SMC_Pulse_2Pulse_Neg       = 4;  // 雙脈波，負緣
const int SMC_Pulse_2Pulse_Pos       = 5;  // 雙脈波，正緣
const int SMC_Pulse_PhaseDiff        = 6;  // 兩相 90° 差
const int SMC_Pulse_PhaseDiffDelay   = 7;  // 兩相 90° 差（延遲）
```

### 原點搜尋模式 (SmcWSetOrgMode)

```cpp
// LimitTurn - 極限反向
const int SMC_OrgMode_LimitTurn_Off  = 0;  // 不反向
const int SMC_OrgMode_LimitTurn_On   = 1;  // 自動反向

// OrgType - Z 相使用
const int SMC_OrgMode_OrgType_UnUseZ = 0;  // 不使用 Z 相
const int SMC_OrgMode_OrgType_UseZ   = 1;  // 使用 Z 相

// EndDir - 歸零結束方向
const int SMC_OrgMode_EndDir_UnSpec  = 0;  // 不指定
const int SMC_OrgMode_EndDir_PosCW   = 1;  // 正方向（CW）
const int SMC_OrgMode_EndDir_NegCCW  = 2;  // 負方向（CCW）
```

### 原點邏輯設定 (SmcWSetOrgLog)

```cpp
const int SMC_OrgLog_ORG_Negative    = 0x00;  // ORG 負邏輯
const int SMC_OrgLog_ORG_Positive    = 0x01;  // ORG 正邏輯
const int SMC_OrgLog_Z_RisingEdge    = 0x02;  // Z 上升緣觸發
```

### 計數器模式 (SmcWSetCounterMode)

```cpp
// ClearCntLtc - LTC 訊號觸發時清除
const int SMC_ClearCntLtc_Off       = 0;  // 不清除
const int SMC_ClearCntLtc_OutPulse  = 1;  // 清除脈波計數器
const int SMC_ClearCntLtc_Encoder   = 2;  // 清除編碼器計數器
const int SMC_ClearCntLtc_Both      = 3;  // 清除兩者

// LtcMode - 閂鎖模式
const int SMC_LtcMode_Off           = 0;  // 關閉
const int SMC_LtcMode_OutPulse      = 1;  // 閂鎖脈波計數器
const int SMC_LtcMode_Encoder       = 2;  // 閂鎖編碼器計數器
const int SMC_LtcMode_Both          = 3;  // 閂鎖兩者
```

### 運動類型 (SmcWSetReady)

```cpp
const int SMC_MotionType_NoMotion    = 0;  // 無運動
const int SMC_MotionType_PTPMotion   = 1;  // PTP 定位
const int SMC_MotionType_JogMotion   = 2;  // JOG 連續
const int SMC_MotionType_OrgMotion   = 3;  // 原點復歸
const int SMC_MotionType_ZPhaMotion  = 6;  // Z 相運動
const int SMC_MotionType_NoDecMotion = 7;  // 無減速運動
```

### 停止位置模式 (SmcWSetStopPosition)

```cpp
const int SMC_StopPosition_Absolute  = 0;  // 絕對座標
const int SMC_StopPosition_Relative  = 1;  // 相對座標
```

### I/O 狀態位元 (SmcWGetCtrlInOutStatus)

```cpp
const int SMC_CtrlInOutSts_PCS    = 0x01;  // 定位開始
const int SMC_CtrlInOutSts_ERC    = 0x02;  // 編碼器清除
const int SMC_CtrlInOutSts_EZ     = 0x04;  // 編碼器 Z 相
const int SMC_CtrlInOutSts_CLR    = 0x08;  // 清除
const int SMC_CtrlInOutSts_LTC    = 0x10;  // 閂鎖
const int SMC_CtrlInOutSts_SD     = 0x20;  // 減速
const int SMC_CtrlInOutSts_INP    = 0x40;  // 定位完成
const int SMC_CtrlInOutSts_DIRCCW = 0x80;  // 方向 CCW
```

### 編碼器類型 (SmcWSetEncType)

```cpp
const int SMC_MotionType_AB_1X   = 0;  // AB 相 1 倍頻
const int SMC_MotionType_AB_2X   = 1;  // AB 相 2 倍頻
const int SMC_MotionType_AB_4X   = 2;  // AB 相 4 倍頻
const int SMC_MotionType_UD      = 3;  // 上/下計數
const int SMC_MotionType_Unused  = 4;  // 不使用
```

### FIFO Latch 來源 (SmcWSetFIFOLatchSrc)

```cpp
const int SMC_FIFOLtcSrc_Axis1 = 0x01;
const int SMC_FIFOLtcSrc_Axis2 = 0x02;
const int SMC_FIFOLtcSrc_Axis3 = 0x04;
const int SMC_FIFOLtcSrc_Axis4 = 0x08;
const int SMC_FIFOLtcSrc_Axis5 = 0x10;
const int SMC_FIFOLtcSrc_Axis6 = 0x20;
const int SMC_FIFOLtcSrc_Axis7 = 0x40;
const int SMC_FIFOLtcSrc_Axis8 = 0x80;
```

---

## TMySMCMotor 類別方法

| TMySMCMotor 方法 | 功能說明 |
|-----------------|----------|
| `Open_SMCCard()` | 開啟 SMC 軸卡 |
| `Close_SMCCard()` | 關閉 SMC 軸卡 |
| `InitMotor(IoAddress)` | 初始化馬達（設定參數） |
| `MoveTo(Tar)` / `MoveToPos(Tar)` | PTP 移動至目標位置 |
| `MoveToPosShortDistance(Tar)` | 短距離移動（使用短距離速度） |
| `MoveToPosShortDisSlowSP(Tar)` | 短距離慢速移動 |
| `JogP()` / `JogN()` | 正/負方向 JOG 連續移動 |
| `Stop()` | 立即停止 |
| `DecStop()` | 減速停止 |
| `HomeObject()` | 原點復歸 |
| `SMCMotHome()` | SMC 原點復歸狀態機 |
| `MotionDone()` | 檢查運動是否完成 |
| `ReadPos()` | 讀取指令位置 |
| `ReadRealPos()` | 讀取實際位置（脈波計數器） |
| `ReadEnCoderRealPos()` | 讀取編碼器位置 |
| `SetCommand(p)` | 設定指令位置 |
| `SetPosition(p)` | 設定實際位置 |
| `SetSpeed(x)` | 設定運動速度 |
| `SetSpeedShortDistance(x)` | 設定短距離速度 |
| `SetSpeedShortDisSlowSP(x)` | 設定短距離慢速 |
| `SetAcc(a)` / `SetDec(a)` | 設定加速/減速度 |
| `SetSoftLimit(iPLimit, iNLimit)` | 設定軟體極限 |
| `SMCSoftLimitEnable(bFlag)` | 啟用/停用軟體極限 |
| `SetServoOn(IsOn)` | 伺服開/關 |
| `GetAlarm()` | 讀取警報狀態 |
| `ScanMotorStatus(*Led)` | 掃描馬達狀態 LED |
| `MotOutputOn(iOutPort)` | 輸出 ON |
| `MotOutputOff(iOutPort)` | 輸出 OFF |
| `MotInputStatus(*bInputPort)` | 讀取輸入狀態 |
| `LinearAxisMoveTo(...)` | 線性補間移動 |
| `EnableTrigger(...)` | 啟用位置觸發功能 |
| `ResetLatch()` | 重置 Latch |
| `GetLatchTotalLen()` | 取得 Latch 資料總數 |
| `GetLatchBuffer(...)` | 讀取 Latch 緩衝區 |
| `GetLatchIOStatus(...)` | 取得 Latch I/O 狀態 |
| `SetFIFOLatchSrc(...)` | 設定 FIFO Latch 來源 |

---

## 初始化流程範例

```cpp
// TMySMCMotor::InitMotor() 摘要
int TMySMCMotor::InitMotor(int IoAddress)
{
    if (!Enable) return 0;
    if (Open_SMCCard() == false) return 0;
    
    // 設定軟體極限（預設關閉）
    SMCSoftLimitEnable(false);
    
    // 設定速度參數
    SetSpeed(PJogLowSpeed);
    
    // 設定加減速
    // 加速度 = (JogLowSpeed - InitSpeed) / Second
    // ...
    
    // 設定編碼器倍率
    SetEncodeMultiple(3);  // 4X 倍頻
    
    // 設定位置計數器
    SetPosition(0);
    
    return 1;
}
```

---

## 原點復歸流程

```cpp
bool TMySMCMotor::SMCMotHome()
{
    switch (Task)
    {
        case 1:
            // 設定原點模式參數
            // 設定歸零方向、速度
            // 啟動原點搜尋
            break;
        case 10:
            // 檢查原點搜尋狀態
            break;
        case 20:
            // 等待運動完成
            break;
        case 30:
            // 原點搜尋完成，清除計數器
            break;
        default:
            break;
    }
    return Result;
}
```

---

## 硬體規格摘要

| 項目 | 規格 |
|------|------|
| 介面 | PCI Bus |
| 脈波輸出 | 6.5Mpps（每軸） |
| 脈波格式 | 單脈波/雙脈波/90° 相差 |
| 輸出格式 | 差動/Open-Collector 可切換 |
| 編碼器輸入 | 差動/TTL/Open-Collector |
| 極限輸入 | 每軸 3 點（+LIM, -LIM, ORG） |
| 通用輸入 | 每軸 7 點 |
| 通用輸出 | 每軸 3 點 |
| 補間功能 | 線性/圓弧 |
| Frame 儲存 | 1024 frames/軸 |
| 同步控制 | 最多 16 卡（128 軸） |

---

## 運動控制 IC

本軸卡使用日本脈衝電機（Nippon Pulse Motor）的 **PCL6045 系列** 運動控制 IC。

---

## SMC-8DF-PCI-C01 客製版功能

> 此版本為康泰克與鴻勁科技共同開發的客製版，主要增加 FIFO Latch 功能。

### 功能概述

- **FIFO Latch 功能**：增加 Encoder 輸入 Counter 的 LATCH 功能
- **SENSOR 輸入**：當 SENSOR 偵測到訊號時，鎖存當下的 Encoder 計數值
- 其他功能與 SMC-8DF-PCI 標準版相同

### 系統架構

```
SENSOR 輸入 (IN4/LTC)
       │
       ▼
┌──────────────┐
│ Photocoupler │ ← 需要 +12V ~ +24V 外部電源
└──────────────┘
       │
       ▼
┌──────────────┐
│ FPGA 控制器   │
└──────────────┘
       │
       ▼
┌──────────────────────────────────┐
│ FIFO Buffer                      │
│ 容量: 8 SENSOR × 80 Line = 640   │
│ Counter: 24-bit × 8 軸           │
└──────────────────────────────────┘
```

### 腳位差異（與標準版比較）

| 腳位 | 標準版 | C01 客製版 |
|------|--------|-----------|
| **IN4/LTC** | 可設定為通用輸入或 LTC | **固定為 LTC 功能，不可更改** |
| **IN1～IN6** | 可設定為通用輸入 | **不能設為通用輸入** |
| 其他腳位 | - | 與標準版相同 |

### SENSOR 輸入規格

| 項目 | 規格 |
|------|------|
| 輸入方式 | Photocoupler 隔離 |
| 電源需求 | +12V ~ +24V 外部電源 |
| Counter 位元數 | 24-bit |
| Counter 數量 | 8 個（每軸 1 個）|
| 最大延遲時間 | 400 μsec |

### FIFO Buffer 規格

| 項目 | 規格 |
|------|------|
| Buffer 容量 | 640 筆（8 SENSOR × 80 Line）|
| 資料格式 | 24-bit Counter 值 |
| 讀取方式 | FIFO（先進先出）|

### 時序限制

以下情況會導致 Counter 值無法正確擷取：

1. **單 SENSOR 信號變化過快**
   - LOW→HIGH→LOW 或 HIGH→LOW→HIGH 之間的時間 < 400μs

2. **兩個 SENSOR 輸入間隔過短**
   - 兩個 SENSOR 輸入中間的時間 < 400μs

```
錯誤情況 1：單 SENSOR 變化過快
    ┌───┐
────┘   └──── (HIGH 時間 < 400μs)
    |<-->|
    < 400μs

錯誤情況 2：兩 SENSOR 間隔過短
SENSOR1 ─┬─────
SENSOR2 ──┬────
          |<>|
          < 400μs
```

### C01 專用 API 函式

| 函式 | 說明 |
|------|------|
| `SmcWResetLatchFIFO()` | 重置 FIFO 緩衝區資料 |
| `SmcWSetFIFOLatchSrc()` | 設定 Latch 來源與啟用外部 Latch 輸入 |
| `SmcWGetFIFOLatchSrc()` | 取得 Latch 來源設定 |
| `SmcWGetLatchDataFromBuffer()` | 從緩衝區讀取 Latch 資料 |
| `SmcWGetLatchFIFOLength()` | 取得 FIFO 中目前的資料筆數 |

### C01 專用 API 函式詳細說明

#### SmcWResetLatchFIFO

```cpp
long SmcWResetLatchFIFO(short Id);
```

**功能**：重置 FIFO 緩衝區中的所有資料

| 參數 | 說明 |
|------|------|
| `Id` | 軸卡 ID |
| **回傳** | 0=成功，負值=錯誤 |

#### SmcWSetFIFOLatchSrc

```cpp
long SmcWSetFIFOLatchSrc(short Id, short AxisNo, short LatchAxisNo, short Enable);
```

**功能**：設定 Latch Counter 來源與啟用外部 Latch 輸入訊號

| 參數 | 說明 |
|------|------|
| `Id` | 軸卡 ID |
| `AxisNo` | 軸編號 |
| `LatchAxisNo` | Latch 來源軸編號 |
| `Enable` | 啟用旗標（0=停用，1=啟用）|
| **回傳** | 0=成功，負值=錯誤 |

#### SmcWGetFIFOLatchSrc

```cpp
long SmcWGetFIFOLatchSrc(short Id, short AxisNo, short *LatchAxisNo, short *Enable);
```

**功能**：取得目前的 Latch 來源設定

| 參數 | 說明 |
|------|------|
| `Id` | 軸卡 ID |
| `AxisNo` | 軸編號 |
| `*LatchAxisNo` | [輸出] Latch 來源軸編號 |
| `*Enable` | [輸出] 啟用狀態 |
| **回傳** | 0=成功，負值=錯誤 |

#### SmcWGetLatchDataFromBuffer

```cpp
long SmcWGetLatchDataFromBuffer(short Id, short BufferNo, 
                                 short *AxisCounterNo, short *LatchDataCnt, 
                                 long *LatchDataTable);
```

**功能**：從 Latch 緩衝區讀取資料

| 參數 | 說明 |
|------|------|
| `Id` | 軸卡 ID |
| `BufferNo` | 緩衝區編號 |
| `*AxisCounterNo` | [輸出] 軸 Counter 編號 |
| `*LatchDataCnt` | [輸出] 讀取的資料筆數 |
| `*LatchDataTable` | [輸出] Latch 資料陣列 |
| **回傳** | 0=成功，負值=錯誤 |

#### SmcWGetLatchFIFOLength

```cpp
long SmcWGetLatchFIFOLength(short Id, short *Length);
```

**功能**：取得 FIFO 中目前的資料筆數

| 參數 | 說明 |
|------|------|
| `Id` | 軸卡 ID |
| `*Length` | [輸出] 資料筆數 |
| **回傳** | 0=成功，負值=錯誤 |

### C01 版本使用範例

```cpp
// 1. 重置 FIFO 緩衝區
SmcWResetLatchFIFO(Id);

// 2. 設定 Latch 來源（從軸 1 的 Encoder 取得）
SmcWSetFIFOLatchSrc(Id, 1, 1, 1);

// 3. 等待資料收集...

// 4. 檢查資料筆數
short length;
SmcWGetLatchFIFOLength(Id, &length);

// 5. 讀取 Latch 資料
if (length > 0) {
    short axisNo, dataCnt;
    long latchData[640];
    SmcWGetLatchDataFromBuffer(Id, 0, &axisNo, &dataCnt, latchData);
    
    // 處理 latchData...
}
```

---

## 注意事項

1. **軸號對應**：手冊中 Axis0~Axis3 對應 API 中 Axis No.1~No.4
2. **脈波輸出格式**：需透過硬體開關（SW）設定 Open-Collector 或差動輸出
3. **終端電阻**：差動輸入需啟用終端電阻（透過 DIP 開關設定）
4. **Board ID**：多卡使用時需設定不同 ID（0~F，透過旋鈕開關）
5. **Frame 執行**：支援迴圈執行（Loop Operation）
