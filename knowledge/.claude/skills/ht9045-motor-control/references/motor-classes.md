# 馬達類別成員參考

完整的 HT9045 馬達控制類別成員參考。

---

## HTMotor（基底類別）

### 公開屬性

| 屬性 | 型別 | 說明 |
|------|------|------|
| `Address` | unsigned int | I/O 位址 |
| `iBoardID` | unsigned int | 板卡 ID |
| `iPortID` | unsigned int | Port ID |
| `PHomeHighSpeed` | unsigned int | 回原點高速 |
| `PHomeLowSpeed` | unsigned int | 回原點低速 |
| `PJogHighSpeed` | unsigned int | JOG 高速 |
| `PJogLowSpeed` | unsigned int | JOG 低速 |
| `InitSpeed` | unsigned int | 起始速度 |
| `PServoAlarmOn` | bool | 伺服 Alarm 啟用 |
| `Enable` | bool | 馬達啟用 |
| `Direction` | bool | 運動方向 |
| `HomeDirection` | bool | 回原點方向 |
| `MotorType` | int | 馬達類型（見下方常數） |
| `bSensorType` | bool | 感測器類型 |
| `bLimitLogic` | bool | 極限邏輯 |
| `bIn1Logic` | bool | IN1 邏輯 |
| `GearRatio` | double | 齒輪比 |
| `PSoftLimitP` | int | 正向軟體極限 |
| `PSoftLimitN` | int | 負向軟體極限 |
| `LastHomePos` | int | 最後回原點位置 |
| `EncoderType` | int | 編碼器類型 |
| `iHomePitch` | int | 回原點螺距 |
| `ErrorString[256]` | char[] | 錯誤訊息 |

### 馬達類型常數

```cpp
const int Step_Motor          = 0;   // 步進馬達
const int Servo_Motor         = 1;   // 伺服馬達
const int Rotate_Motor        = 2;   // 旋轉馬達
const int YASKAWA_Servo_Motor = 3;   // Yaskawa 伺服馬達
const int YASKAWA_Liner_Motor = 4;   // Yaskawa 線性馬達
const int Step_Motor_Oriental = 5;   // Oriental 步進馬達
```

### LED 索引常數

```cpp
enum {
    iCwLed          = 0,   // CW 方向
    iHomeLed        = 1,   // Home 感測器
    iCcwLed         = 2,   // CCW 方向
    iEmgLed         = 3,   // EMG 緊急停止
    iAlarmLed       = 4,   // Alarm
    iSoftcwLed      = 5,   // 軟體 CW 極限
    iSoftccwLed     = 6,   // 軟體 CCW 極限
    iServoalarmLed  = 7,   // 伺服 Alarm
    iInposLed       = 8,   // 到位
    iServoOn        = 9    // 伺服 ON
};
```

### 虛擬方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Stop()` | void | 停止馬達 |
| `JogP()` | bool | 正向 JOG |
| `JogN()` | bool | 負向 JOG |
| `ReadPos()` | int | 讀取命令位置 |
| `MoveToPos(int Tar)` | bool | 移動至目標位置 |
| `MoveToPosShortDistance(int Tar)` | bool | 短距離移動 |
| `HomeObject()` | bool | 回原點物件 |
| `HomeFlag()` | bool | 回原點旗標 |
| `GetAlarm()` | bool | 取得 Alarm 狀態 |
| `SetSpeed(unsigned int x)` | void | 設定速度 |
| `SetInitSpeed(unsigned int x)` | void | 設定起始速度 |
| `SetServoAlarmOn(bool Value)` | void | 設定伺服 Alarm |
| `InitMotor(int IoAddress)` | int | 初始化馬達 |
| `SetRange(unsigned int a)` | void | 設定範圍 |
| `SetRate(unsigned int a)` | void | 設定速率 |
| `ResetPos(int Pulse)` | bool | 重置位置 |
| `SoftLimitEnable(bool bFlag)` | void | 軟體極限啟用 |
| `ServerOnOff(bool bStatus)` | void | 伺服開關 |
| `MotOutputOn(int iOutPort)` | void | 輸出 ON |
| `MotOutputOff(int iOutPort)` | void | 輸出 OFF |
| `MotInputStatus(bool *bInputPort)` | void | 讀取輸入狀態 |
| `ScanMotorStatus(bool *Led)` | void | 掃描馬達狀態 |
| `DecStop()` | void | 減速停止 |
| `MotionDone()` | bool | 運動完成 |
| `ReadRealPos()` | int | 讀取實際位置 |
| `ReadEnCoderRealPos()` | int | 讀取編碼器實際位置 |
| `SetCommand(int p)` | int | 設定命令位置 |
| `SetPosition(int p)` | int | 設定位置 |
| `SetServoOn(bool IsOn)` | void | 設定伺服 ON/OFF |
| `SetSoftLimit(int iPLimit, int iNLimit)` | void | 設定軟體極限 |
| `SetAcc(double a)` | void | 設定加速度 |
| `SetDec(double a)` | void | 設定減速度 |

### Latch 相關方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `SetEnableLatch(bool a)` | void | 啟用 Latch |
| `ResetLatch()` | void | 重置 Latch |
| `GetLatchTotalLen()` | int | 取得 Latch 長度 |
| `GetLatchBuffer(...)` | int | 取得 Latch 緩衝區 |
| `GetLatchIOStatus(...)` | bool | 取得 Latch IO 狀態 |
| `SetFIFOLatchSrc(...)` | void | 設定 FIFO Latch 來源 |

### MN200 補間運動方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `SetGroup(BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[])` | int | 設定群組 |
| `LineNMove(BYTE bDevNo[], long DevPos[], BYTE bNumDev)` | int | N 軸直線補間 |

### 安全門檢查

```cpp
PF_CHECK MotorIdleSafeDoorCheck;   // 安全門檢查回呼函式
bool CheckIsSafeDoorOpen();        // 檢查安全門是否開啟
```

---

## TMyMotor（馬達控制封裝）

### 公開屬性

| 屬性 | 型別 | 說明 |
|------|------|------|
| `Mot_Name` | int | 馬達編號（索引） |
| `Alias` | AnsiString | 馬達別名 |
| `NumberAlias` | AnsiString | 馬達編號別名 |
| `Motor` | HTMotor* | 實際馬達實作指標 |
| `speed` | int | 運行速度 |
| `fCanMove` | bool | 可移動旗標 |
| `fCanMoveR` | bool | 可向右移動 |
| `fCanMoveM` | bool | 可向中間移動 |
| `fCanMoveL` | bool | 可向左移動 |
| `TargetPosition` | int | 目標位置 |
| `Position` | int | 目前位置 |
| `EncoderPosition` | int | 編碼器位置 |
| `ScreenPos` | int | 螢幕位置 |
| `HomeFlag` | int | 回原點旗標（0=未完成, 1=完成, 2=失敗） |
| `HomeTask` | int | 回原點任務狀態 |
| `Led[10]` | bool[] | LED 狀態陣列 |
| `MovFlag` | bool | 移動旗標 |

### Galil 相關屬性

| 屬性 | 型別 | 說明 |
|------|------|------|
| `GailSpeed` | int | Galil 速度 |
| `Gali_MotorAlarm` | bool | Galil 馬達 Alarm |
| `IndexPickLimit` | int | Index 取放極限 |
| `bScanFlag` | bool | 掃描旗標 |
| `GaliSofDelayCount` | int | Galil 軟體延遲計數 |
| `iGali_SingalHomeTask` | int | Galil 單軸回原點任務 |
| `iGali_FindZPhaseTask[4]` | int[] | Galil 搜尋 Z 相任務 |

### 基本方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `SetAlias(int iNo, AnsiString Name)` | void | 設定別名 |
| `SetPanel(TWinControl *PCtrl, bool b)` | void | 設定面板 |
| `SetScreenScale(int s1, int e1, int s2, int e2)` | void | 設定螢幕比例 |
| `SetSpeed(double p)` | void | 設定速度 |
| `GetSpeed()` | int | 取得速度 |
| `EnableMotorMove()` | void | 啟用馬達移動 |
| `MotorMove(int p)` | int | 馬達移動 |
| `MotorInitial()` | void | 馬達初始化 |
| `ScanMotorStatus()` | void | 掃描馬達狀態 |
| `MotorHome(bool)` | int | 馬達回原點 |
| `ReadPos()` | int | 讀取位置 |
| `ReadEncoderPos()` | int | 讀取編碼器位置 |
| `GetMotorAlarm()` | bool | 取得馬達 Alarm |
| `GetErrorIndex()` | int | 取得錯誤索引 |
| `Home()` | bool | 回原點 |
| `HomeReset()` | void | 重置回原點 |
| `IsCanMove()` | bool | 是否可移動 |

### Galil 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Gali_Command(AnsiString str, AnsiString sFunc)` | long | 發送 Galil 命令 |
| `Gali_MotMove(int Pos, int Speed, AnsiString _Func)` | bool | Galil 馬達移動 |
| `Gali_MotMove2(int Pos, int Speed, int SpeedDec)` | bool | Galil 兩段速移動 |
| `Gali_MotMoveNoWait(int Pos, int Speed, int iNeedDelayTime, bool bCheckZ)` | bool | Galil 非阻塞移動 |
| `Gali_MotMoveSkipEncoder(int Pos, int Speed)` | bool | Galil 移動（跳過編碼器） |
| `Gali_MovePR(int Pos, int Speed)` | bool | Galil 相對移動 |
| `Gali_ReadPos()` | long | 讀取 Galil 位置 |
| `Gali_ReadEncoderPos()` | long | 讀取 Galil 編碼器位置 |
| `Gali_MotHome(AnsiString HomeAxis)` | void | Galil 回原點 |
| `Gali_MotHomeFindZ(AnsiString HomeAxis)` | void | Galil 回原點（含 Z 相） |
| `Gali_SingalHome(bool IndexZFirstHome)` | bool | Galil 單軸回原點 |
| `Gali_FindZPhase()` | bool | Galil 搜尋 Z 相 |
| `Gali_JogP(int Speed)` | void | Galil 正向 JOG |
| `Gali_JogN(int Speed)` | void | Galil 負向 JOG |
| `Gali_JogPSetup(int Speed)` | void | Galil 正向 JOG 設定 |
| `Gali_JogNSetup(int Speed)` | void | Galil 負向 JOG 設定 |
| `Gali_JogPAndCount(int Speed, int Count)` | void | Galil 正向 JOG（限定脈波） |
| `Gali_JogNAndCount(int Speed, int Count)` | void | Galil 負向 JOG（限定脈波） |
| `Gali_ScanMotStatus()` | void | 掃描 Galil 馬達狀態 |
| `Gali_ScanMotStatusTIMO()` | void | 掃描 Galil 馬達狀態（含超時） |
| `Gali_ScanAlarmStatus()` | void | 掃描 Galil Alarm 狀態 |
| `Gali_Two_ZAxis_Move(int Pos, int Speed, AnsiString sFunc, bool bTwoPos, int Pos2)` | bool | Galil 雙 Z 軸移動 |
| `GalilTwoY_Move(int YPos, int Y2Pos, int Speed, AnsiString sFunc)` | bool | Galil 雙 Y 軸移動 |

### 雙 Z 軸協調方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Z1UpZ2Down(int Speed, bool TMode, bool bPickErr)` | bool | Z1 上升 Z2 下降 |
| `Z1DownZ2Up(int Speed, bool TMode, bool bPickErr)` | bool | Z1 下降 Z2 上升 |
| `Z1UpZ2Down1(int Speed)` | bool | Z1 上升 Z2 下降（模式 1） |
| `Z1DownZ2Up1(int Speed)` | bool | Z1 下降 Z2 上升（模式 1） |
| `Z1UpZ2Down2(int Speed, bool bPickErr)` | bool | Z1 上升 Z2 下降（模式 2） |
| `Z1DownZ2Up2(int Speed, bool bPickErr)` | bool | Z1 下降 Z2 上升（模式 2） |
| `ISZ1Up_Z2Down()` | bool | 檢查 Z1 上 Z2 下狀態 |
| `ISZ1Down_Z2Up()` | bool | 檢查 Z1 下 Z2 上狀態 |
| `ISZ1Up_Z2DownNoWait()` | bool | 非阻塞檢查 Z1 上 Z2 下 |
| `ISZ1Down_Z2UpNoWait()` | bool | 非阻塞檢查 Z1 下 Z2 上 |
| `ISNormal()` | bool | 檢查正常狀態 |

### 編碼器檢查方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `Gali_ReadEncoderInRandge(long checkpos)` | bool | 編碼器在範圍內 |
| `Gali_ReadEncoderInRandgeNoWait(long checkpos)` | bool | 非阻塞編碼器範圍檢查 |
| `Gali_ReadEncoderInRandgeMinLimit(long checkpos)` | bool | 編碼器最小極限檢查 |
| `Gali_ReadEncoderMaxRandge(long checkpos)` | bool | 編碼器最大範圍檢查 |
| `Gali_ReadEncoderOver(long checkpos)` | bool | 編碼器超過檢查 |
| `Gali_ReadEncoderBelowCheckHeight(long checkpos)` | bool | 編碼器低於高度檢查 |

### PCI-L132 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `PCIL132_SetPos(int Pos)` | void | PCI-L132 設定位置 |
| `PCIL132_StopMotor()` | void | PCI-L132 停止馬達 |
| `PCIL132_ResetPos()` | void | PCI-L132 重置位置 |

### JOG 與伺服方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `JogP(int Speed)` | void | 正向 JOG |
| `JogN(int Speed)` | void | 負向 JOG |
| `ServoOnOff(bool IsOn)` | void | 伺服開關 |

### 鎖定機制（Lock）

| 方法 | 回傳 | 說明 |
|------|------|------|
| `GetLockCount()` | int | 取得鎖定計數 |
| `Lock(AnsiString MotorAlias, AnsiString FunctionName, int Task)` | void | 鎖定馬達 |
| `UnLock(AnsiString MotorAlias, AnsiString FunctionName)` | void | 解鎖馬達 |
| `ClearLock()` | void | 清除所有鎖定 |
| `GetLockString(int Index)` | AnsiString | 取得鎖定字串 |

### Galil PR（Program）方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `IsStartGali_Pr()` | bool | 是否啟動 Galil 程式 |
| `StartGali_Pr()` | void | 啟動 Galil 程式 |
| `EndGali_Pr()` | void | 結束 Galil 程式 |
| `GetGali_Pr_Result()` | long | 取得 Galil 程式結果 |
| `SetGali_Pr_ER(long ERA, long ERB, long ERC, long ERD)` | void | 設定 Galil 編碼器 |
| `GetGali_Pr_ER(long &ERA, long &ERB, long &ERC, long &ERD)` | void | 取得 Galil 編碼器 |

---

## TTrayMotor（Tray 馬達）

繼承 `TMyMotor`，額外管理 Tray 資料。

### 公開屬性

| 屬性 | 型別 | 說明 |
|------|------|------|
| `Tray` | HTray | Tray 資料結構 |
| `fHasTray` | bool | 是否有 Tray |
| `bIsFullIC` | bool | Tray 已滿 |
| `bIsEmptyIC` | bool | Tray 已空 |

### Tray 方法

| 方法 | 回傳 | 說明 |
|------|------|------|
| `HasIC()` | bool | 檢查是否有 IC |
| `HasRealIC()` | bool | 檢查是否有真實 IC |
| `HasCleanPad()` | bool | 檢查是否有清潔墊 |
| `InitNewTray(int data, bool bShowSiteMapFlag, AnsiString Func)` | void | 初始化新 Tray |
| `ClearTray(AnsiString Func)` | void | 清除 Tray |
| `InitEmptyTray(AnsiString Func)` | void | 初始化空 Tray |
| `SetHTrayPanel(TTMyTray *ptr)` | void | 設定 Tray 面板 |
| `SetTray(int data, AnsiString Func)` | void | 設定 Tray 資料 |
| `SetTrayBinData(int x, int y, int data, AnsiString iInfo)` | void | 設定 Tray Bin 資料 |
| `SetTraySingleData(int x, int y, int data, int iTarget)` | void | 設定單一 Tray 資料 |
| `SetTraySiteMap(int x, int y, int iSiteMap)` | void | 設定 Tray SiteMap |
| `SetNullIcToHasNullIc()` | void | 設定 NULL_IC 為 HAS_NULL_IC |
| `SetNullIcToHasIc()` | void | 設定 NULL_IC 為 HAS_IC |
| `SetTrayBufferSingleData(int x, int y, int data)` | void | 設定 Tray Buffer 資料 |
| `Refresh()` | void | 刷新 Tray 顯示 |
| `MoveTrayAllItem(TTrayMotor *Source)` | void | 移動所有 Tray 項目 |
| `HowManyDevice(int iType)` | int | 計算指定類型數量 |
| `HowManyDevice()` | int | 計算總數量 |
| `UpHalfIsFull()` | bool | 上半部已滿 |
| `DownHalfIsFull()` | bool | 下半部已滿 |
| `WhichBufferIsFull()` | int | 哪個 Buffer 已滿 |

---

## TMySYNTEKMotor（SYN-TEK MotionNet 馬達）

繼承 `HTMotor`，實作 SYN-TEK PCI-L132 MotionNet 馬達控制。

### 建構式

```cpp
__fastcall TMySYNTEKMotor(int Addr);
```

### 覆寫方法

| 方法 | 說明 |
|------|------|
| `InitMotor(int IoAddress)` | 初始化馬達 |
| `SetSpeed(unsigned int x)` | 設定速度 |
| `SetInitSpeed(unsigned int x)` | 設定起始速度 |
| `ReadPos()` | 讀取位置 |
| `ScanMotorStatus(bool *Led)` | 掃描馬達狀態 |
| `MoveToPos(int Tar)` | 移動至位置 |
| `Stop()` | 停止 |
| `DecStop()` | 減速停止 |
| `JogP()` / `JogN()` | JOG 點動 |
| `HomeObject()` | 回原點 |
| `HomeFlag()` | 回原點旗標 |
| `GetAlarm()` | 取得 Alarm |
| `ResetPos(int p)` | 重置位置 |
| `MotionDone()` | 運動完成 |
| `SetServoOn(bool IsOn)` | 伺服開關 |
| `SetSoftLimit(int iPLimit, int iNLimit)` | 設定軟體極限 |

---

## TMyMN200Motor（ICP-DAS MN200 馬達）

繼承 `HTMotor`，實作 ICP-DAS PISO-MN200 MotionNet 馬達控制。

### 建構式

```cpp
__fastcall TMyMN200Motor(int Addr);
```

### 常數

```cpp
const int MAXRing = 4;    // 最大 Ring 數
const int MAXIP = 64;     // 每 Ring 最大 IP 數
const int MAXPort = 4;    // 每 IP 最大 Port 數
```

### 覆寫方法

與 `TMySYNTEKMotor` 類似，另外支援：

| 方法 | 說明 |
|------|------|
| `SetGroup(BYTE bGrpNo, BYTE bNumDev, BYTE bDevNo[])` | 設定補間群組 |
| `LineNMove(BYTE bDevNo[], long DevPos[], BYTE bNumDev)` | N 軸直線補間 |
| `GetLatchBuffer(...)` | 取得 Latch 緩衝區 |
| `GetLatchIOStatus(...)` | 取得 Latch IO 狀態 |
| `SetFIFOLatchSrc(...)` | 設定 FIFO Latch 來源 |

### MN200 錯誤處理

```cpp
extern DWORD MN_200_ErrorTable[4];
extern void OpenPCI132Card(bool bfirst);
extern void ResetMNet(int iRingNo, AnsiString EngMessage, AnsiString ChtMessage, bool bShowMess);
extern bool GetMN200_Error_Code(int iRing, int iCode, AnsiString *EngStr, AnsiString *ChStr, int iIP);
```

---

## 全域函式

### Galil 卡片管理

```cpp
bool Open_GaliCard();    // 開啟 Galil 卡片
bool Close_GaliCard();   // 關閉 Galil 卡片
```

### 馬達停止

```cpp
void StopAllMotor(bool bIndexCanStop);   // 停止所有馬達
```

### MN200 管理

```cpp
void ShowMNetTree(TTreeView *TView);     // 顯示 MotionNet 樹狀結構
void OpenPCI132Card(bool bfirst);        // 開啟 PCI-L132 卡片
```

---

## 原始檔清單

| 檔案 | 用途 |
|------|------|
| `HTMotor.cpp/h` | 馬達基底類別 |
| `mymotor.cpp/h` | TMyMotor/TTrayMotor 實作 |
| `mySYNTEKmotor.cpp/h` | SYN-TEK MotionNet 馬達實作 |
| `myMN200motor.cpp/h` | ICP-DAS MN200 馬達實作 |
| `HTMC88X1Motor.cpp/h` | MC88X1 馬達實作 |
| `mySMCmotor.cpp/h` | SMC 馬達實作 |
| `TrayStepMotor.cpp/h` | Tray 步進馬達 |
