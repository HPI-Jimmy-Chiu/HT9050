# 和椿 MC88X1P 系列軸卡 API 參考

> 對應原始碼：`Motor/HTMC88X1Motor.cpp`、`Motor/HTMC88X1Motor.h`  
> 手冊來源：MC88系列使用手冊v27.pdf（和椿科技）  
> DLL 標頭檔：`Motor/Mc88x1p_DLL.h`

---

## 概述

MC88X1P 系列是和椿科技出品的 PCI 介面軸卡，支援 MC8841P（4軸）、MC8881P（8軸）、MC8882P（8軸+進階功能）。HT9045 使用 HTMC88X1Motor 類別封裝該軸卡的控制功能。

---

## 軸卡型號比較

| 型號 | 軸數 | 補間功能 | 位置記錄(Latch) | 觸發輸出 |
|------|------|----------|-----------------|----------|
| MC8841P | 4 | 線性/圓弧 | 基本 | 等距觸發 |
| MC8881P | 8 | 線性/圓弧 | 基本 | 等距觸發 |
| MC8882P | 8 | 線性/圓弧 | 進階(FIFO) | 位置觸發 |

---

## 系統命令

| API 函數 | 說明 | 參數 |
|----------|------|------|
| `MC88X1PMotDevOpen(byBoard_ID)` | 開啟軸卡並初始化資源 | byBoard_ID: 0~15 |
| `MC88X1PMotDevClose(byBoard_ID)` | 關閉軸卡並釋放資源 | byBoard_ID: 0~15 |
| `MC88X1PMotGetBoardInfo(byBoard_ID, pInfo)` | 讀取軸卡資訊（型號、DLL版本、韌體版本） | pInfo: BoardInfo* |
| `MC88X1PMotGetTotalAxis(byBoard_ID, *ret)` | 讀取軸卡最大軸數 | *ret: WORD* |
| `MC88X1PMotReset(byBoard_ID)` | 重置軸卡為初始狀態 | - |

---

## 數位 I/O 命令

| API 函數 | 說明 |
|----------|------|
| `MC88X1PMotDI(byBoard_ID, byAxis, *lpReturnValue)` | 讀取指定軸的 DI 狀態 |
| `MC88X1PMotDO(byBoard_ID, byAxis, lpWriteValue)` | 設定指定軸的 DO 輸出（OUT4~OUT7） |
| `MC88X1PGetOutput(byBoard_ID, Axis, *Ret)` | 讀取目前 DO 輸出狀態 |

### DI 位元定義

| 位元 | 訊號 | 說明 |
|------|------|------|
| 0 | IN0 | 通用輸入 0 |
| 1 | IN1 | 通用輸入 1 |
| 2 | IN2 | 通用輸入 2 |
| 3 | IN3 | 原點感測器（Home） |
| 4 | EXPP | 正方向外部驅動控制輸入 |
| 5 | EXPM | 負方向外部驅動控制輸入 |
| 6 | InPos | 到位訊號 |
| 7 | Alarm | 伺服異常警報 |

---

## 運動參數設定

| API 函數 | 說明 |
|----------|------|
| `MC88X1PMotAxisParaSet(...)` | 各軸運動參數設定（加減速曲線） |
| `MC88X1PMotAxisParaSet2(...)` | 各軸運動參數設定（時間型加減速） |
| `MC88X1PMotChgDV(...)` | 動態改變驅動速度 |
| `MC88X1PSetPulseMode(byBoard_ID, bySetAxis, Value)` | 設定脈波輸出型式（單脈波/雙脈波/兩相） |

---

## 狀態讀取

| API 函數 | 說明 |
|----------|------|
| `MC88X1PGetError(...)` | 取得軸的錯誤狀態 |
| `MC88X1PGetMotionInput(...)` | 監測指定軸的外部錯誤訊號 |
| `MC88X1PGetMotionState(...)` | 取得軸運動停止的狀態 |
| `MC88X1PGetDriveStatus(...)` | 取得軸目前的驅動狀態 |
| `MC88X1PMotAxisBusy(...)` | 取得軸的忙碌狀態 |
| `MC88X1PGetSpeed(...)` | 取得目前運動速度 |

---

## 原點搜尋命令

| API 函數 | 說明 |
|----------|------|
| `MC88X1PMotHome(...)` | 原點搜尋功能（13種歸零模式） |
| `MC88X1PMotHomeStatus(...)` | 取得原點搜尋狀態 |
| `MC88X1PMotHomeReset(...)` | 停止原點搜尋運動 |
| `MC88X1PMotWrReg(byBoard_ID, bAxisID, offset, value)` | 設定原點搜尋功能參數 |
| `MC88X1PSetHomeLogic(...)` | 設定 IN3（原點感測器）作動位準 |
| `MC88X1PSetZLogic(...)` | 設定 IN0（Z 相輸入）作動位準 |
| `MC88X1PSetInputLogic(...)` | 設定 IN1、IN2 輸入點作動位準 |

### 原點搜尋參數暫存器 (HomeType)

| 暫存器位址 | 名稱 | 說明 |
|------------|------|------|
| 0x30B | HomeType | 歸零模式（0~12，共13種） |
| 0x30C | HomeP0_Dir | 第0階段方向 |
| 0x30D | HomeP0_Speed | 第0階段速度 |
| 0x30E | HomeP1_Dir | 第1階段方向 |
| 0x30F | HomeP1_Speed | 第1階段速度 |
| 0x309 | HomeOffset | 機械原點與程式原點偏移值 |
| 0x310 | HomeP2_Dir | 第2階段方向 |
| 0x311 | HomeOffset_Speed | 移至程式原點的速度 |

---

## 一般驅動命令

| API 函數 | 說明 |
|----------|------|
| `MC88X1PMotCmove(...)` | 連續驅動（JOG） |
| `MC88X1PMotPtp(...)` | 點到點驅動 |
| `MC88X1PMotStop(...)` | 停止驅動 |

---

## 補間命令

| API 函數 | 說明 |
|----------|------|
| `MC88X1PMotArc(...)` | 圓弧補間（中心、終點、方向） |
| `MC88X1PMotArcTheta(...)` | 圓弧補間（中心、角度、方向） |
| `MC88X1PMotArcThreePoints(...)` | 圓弧補間（經過點、終點） |
| `MC88X1PMoveCircle(...)` | 全圓補間 |
| `MC88X1PMotLine(...)` | 直線補間 |
| `MC88X1PMotLines(...)` | 跨卡直線補間 |
| `MC88X1PMotEndlessLine(...)` | 射線補間 |
| `MC88X1PMotIpStatus(...)` | 取得目前補間狀態 |
| `MC88X1PMotIpReset(...)` | 停止所有補間命令 |

---

## 連續補間命令

| API 函數 | 說明 |
|----------|------|
| `MC88X1PInitialContiBuf(...)` | 配置連續補間緩衝區 |
| `MC88X1PFreeContiBuf(...)` | 釋放連續補間緩衝區 |
| `MC88X1PSetContiData(...)` | 設定連續補間資料 |
| `MC88X1PStartContiDrive(...)` | 開始連續補間 |
| `MC88X1PStartContiDriveSC(...)` | 開始變速度連續補間 |
| `MC88X1PGetCurContiNum(...)` | 取得目前執行的路徑編號 |

---

## 計數器狀態

| API 函數 | 說明 |
|----------|------|
| `MC88X1PSetTheorecticalRegister(...)` | 設定邏輯位置計數器 |
| `MC88X1PGetTheorecticalRegister(...)` | 取得邏輯位置計數器 |
| `MC88X1PSetPracticalRegister(...)` | 設定實際位置計數器（編碼器） |
| `MC88X1PGetPracticalRegister(...)` | 取得實際位置計數器（編碼器） |

---

## 極限功能設定

| API 函數 | 說明 |
|----------|------|
| `MC88X1PEnableCompLimit(...)` | 啟動/停止軟體極限功能 |
| `MC88X1PSetCompNLimit(...)` | 設定負軟體極限位置 |
| `MC88X1PSetCompPLimit(...)` | 設定正軟體極限位置 |
| `MC88X1PSetCompLimitMode(...)` | 設定軟體極限停止模式 |
| `MC88X1PSetNLimitLogic(...)` | 設定負硬體極限作動位準 |
| `MC88X1PSetPLimitLogic(...)` | 設定正硬體極限作動位準 |
| `MC88X1PSetHardLimitMode(...)` | 設定硬體極限停止模式 |

---

## 伺服相關

| API 函數 | 說明 |
|----------|------|
| `MC88X1PSetInPosition(...)` | 啟用/停用 Inposition 功能 |
| `MC88X1PSetServoAlarm(...)` | 啟用/停用伺服異常警報功能 |

---

## HTMC88X1Motor 類別對應

HT9045 中 `HTMC88X1Motor` 類別封裝上述 API，主要方法對應：

| HTMC88X1Motor 方法 | MC88X1P API |
|-------------------|-------------|
| `Open_Card()` | `MC88X1PMotDevOpen()` |
| `Close_Card()` | `MC88X1PMotDevClose()` |
| `InitMotor()` | 多個 `MC88X1PMotWrReg()` + 參數設定 |
| `MoveTo(Tar)` | `MC88X1PMotPtp()` |
| `JogP()` / `JogN()` | `MC88X1PMotCmove()` |
| `Stop()` | `MC88X1PMotStop()` |
| `DecStop()` | `MC88X1PMotStop()` |
| `MC88X1MotHome()` | `MC88X1PMotHome()` + 狀態機 |
| `MotionDone()` | `MC88X1PMotAxisBusy()` |
| `ReadRealPos()` | `MC88X1PGetTheorecticalRegister()` |
| `ReadEnCoderRealPos()` | `MC88X1PGetPracticalRegister()` |
| `SetPos(p)` | `MC88X1PSetTheorecticalRegister()` |
| `SetEnCoderPos(p)` | `MC88X1PSetPracticalRegister()` |
| `SetSoftLimit(...)` | `MC88X1PSetCompPLimit()` + `MC88X1PSetCompNLimit()` |
| `MC88X1SoftLimitEnable(...)` | `MC88X1PEnableCompLimit()` |
| `SetServoOn(IsOn)` | DO 輸出控制 |

---

## 初始化流程範例

```cpp
// HTMC88X1Motor::InitMotor() 摘要
int HTMC88X1Motor::InitMotor(int IoAddress)
{
    if (!Enable) return 0;
    if (Open_Card() == false) return 0;
    
    iHomeType = 7;  // 歸零模式
    MC88X1SoftLimitEnable(false);
    
    // 設定極限/原點感測器邏輯
    MC88X1PSetNLimitLogic(iBoardID, bAxisID, bSensorType);
    MC88X1PSetPLimitLogic(iBoardID, bAxisID, bSensorType);
    MC88X1PSetHomeLogic(iBoardID, bAxisID, bSensorType);
    
    // 伺服警報設定
    if (PServoAlarmOn)
        MC88X1PSetServoAlarm(iBoardID, bAxisID, 1);
    
    // 原點搜尋參數
    MC88X1PMotWrReg(iBoardID, bAxisID, HomeType, iHomeType);
    MC88X1PMotWrReg(iBoardID, bAxisID, HomeP0_Dir, HomeDirection);
    MC88X1PMotWrReg(iBoardID, bAxisID, HomeP0_Speed, PHomeHighSpeed * Range);
    // ... 其他原點參數 ...
    
    // 編碼器設定
    MC88X1PSetEncoderDir(iBoardID, bAxisID, 1);
    MC88X1PSetEncoderMultiple(iBoardID, bAxisID, 3);
    
    SetPos(0);
    return 1;
}
```

---

## 錯誤碼

| 錯誤碼 | 說明 |
|--------|------|
| `ERROR_SUCCESS` | 函數呼叫成功 |
| `BoardNumErr` | 軸卡號碼錯誤或未開卡 |
| `AxisNumErr` | 指定軸號碼錯誤 |
| `OpenEventFail` | 開卡失敗 |
| `FirmwareCheckFail` | 韌體檢查失敗（MC8882P only） |

---

## 軸位元遮罩對應

```cpp
// bySetAxis / byReadDIAxis 位元對應
bit 0: X1 Axis
bit 1: Y1 Axis
bit 2: Z1 Axis
bit 3: U1 Axis
bit 4: X2 Axis  // MC8881P/MC8882P only
bit 5: Y2 Axis
bit 6: Z2 Axis
bit 7: U2 Axis
```

---

## 注意事項

1. **開卡必要**：使用任何 API 前必須先呼叫 `MC88X1PMotDevOpen()`
2. **軸號範圍**：MC8841P 限 4 軸（bit 0~3），MC8881P/MC8882P 支援 8 軸（bit 0~7）
3. **韌體版本**：MC8882P 需檢查韌體版本相容性
4. **編碼器倍率**：支援 1X / 2X / 4X 倍率
5. **歸零模式**：共 13 種模式（0~12），需依機構設計選擇
