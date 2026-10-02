# Galil DMC API 參考

HT9045 使用 Galil DMC 運動控制卡控制 Index Arm 的 Y/Z 軸馬達。

---

## 系統配置

### 全域變數

```cpp
HANDLEDMC hDmc;               // Galil 卡片 Handle
bool bGali_CardInstall;       // 卡片安裝旗標
bool GaliAxisAlarm[4];        // 軸警報狀態 (W/X/Y/Z = A/B/C/D)
CRITICAL_SECTION g_cs;        // 臨界區（多執行緒保護）
```

### 速度參數

```cpp
int GailAcSpeed  = 20000000;  // 加速度
int GailDcSpeed  = 20000000;  // 減速度
int GailAcSpeed2 = 20000000;  // 加速度 2
int GailDcSpeed2 = 20000000;  // 減速度 2
```

### 軸名稱對應

| 馬達索引 | Galil 軸 | 命令字母 | 說明 |
|----------|----------|----------|------|
| MTestY1 | W（或 A）| W 或 A | Index Arm Y1 |
| MTestY2 | X（或 B）| X 或 B | Index Arm Y2 |
| MTestZ1 | Y（或 C）| Y 或 C | Index Arm Z1 |
| MTestZ2 | Z（或 D）| Z 或 D | Index Arm Z2 |

---

## DMC API 函式

### 卡片管理

| 函式 | 說明 |
|------|------|
| `DMCOpen(controller, hDmc, response)` | 開啟控制器 |
| `DMCClose(hDmc)` | 關閉控制器 |
| `DMCCommand(hDmc, cmd, response, size)` | 發送命令 |
| `DMCClear(hDmc)` | 清除緩衝區 |
| `DMCDiagnosticsOff(hDmc)` | 關閉診斷 |

### 開啟卡片

```cpp
bool Open_GaliCard()
{
    // 使用 DMCOpen 開啟 Galil 控制器
    // 回傳 true 表示成功
}

bool Close_GaliCard()
{
    // 使用 DMCClose 關閉控制器
}
```

---

## Galil 命令字串

### 基本格式

```cpp
// 發送命令並取得回應
long rc = MOT[idx].Gali_Command("命令字串", "呼叫函式名稱");

// 範例
MOT[MTestY1].Gali_Command("PA 10000", "MoveY1");
MOT[MTestZ1].Gali_Command("SP 5000", "SetSpeed");
```

### 運動命令

| 命令 | 格式 | 說明 |
|------|------|------|
| `PA` | `PA x` 或 `PA x,x,x,x` | 絕對位置移動 |
| `PR` | `PR x` 或 `PR x,x,x,x` | 相對位置移動 |
| `BG` | `BG A` 或 `BG ABCD` | 開始運動 |
| `ST` | `ST` 或 `ST A` | 停止運動 |
| `AB` | `AB` | 中止運動 |
| `HM` | `HM A` | 回原點 |
| `FI` | `FI A` | 搜尋 Index（Z 相） |

### 速度與加減速

| 命令 | 格式 | 說明 |
|------|------|------|
| `SP` | `SP x` 或 `SP x,x,x,x` | 設定速度 (counts/sec) |
| `AC` | `AC x` 或 `AC x,x,x,x` | 設定加速度 (counts/sec²) |
| `DC` | `DC x` 或 `DC x,x,x,x` | 設定減速度 (counts/sec²) |
| `VS` | `VS x` | 向量速度 |
| `VA` | `VA x` | 向量加速度 |
| `VD` | `VD x` | 向量減速度 |

### JOG 運動

| 命令 | 格式 | 說明 |
|------|------|------|
| `JG` | `JG x` 或 `JG x,x,x,x` | 設定 JOG 速度（正/負值） |
| `BG` | `BG A` | 開始 JOG |
| `ST` | `ST A` | 停止 JOG |

### 位置讀取

| 命令 | 格式 | 回傳 | 說明 | HT9045 對應函式 |
|------|------|------|------|-----------------|
| `TP` | `TP A` | 位置值 | 讀取 Encoder 實際位置（主編碼器） | `Gali_ReadEncoderPos()` |
| `TD` | `TD A` | 位置值 | 讀取命令位置（輔助編碼器/步進脈衝數） | `Gali_ReadPos()` |
| `DE` | `DE A` | 位置值 | 定義輔助編碼器位置 | — |
| `DP` | `DP A` | - | 定義命令位置 | — |
| `TE` | `TE A` | 差值 | 讀取 Following Error（= command - encoder）| GaliProtectMode==2 使用 |

> **ShowIndexMotorError 顯示慣例**：CMD = `Gali_ReadPos()`(TD)，POS = `Gali_ReadEncoderPos()`(TP)。

### 狀態讀取

| 命令 | 格式 | 回傳 | 說明 |
|------|------|------|------|
| `TS` | `TS A` | 狀態位元 | 讀取開關狀態 |
| `TI` | `TI` | 輸入狀態 | 讀取數位輸入 |
| `TC` | `TC` 或 `TC1` | 錯誤碼 | 讀取錯誤碼 |
| `SC` | `SC` | 狀態 | 讀取停止碼 |
| `MG_BG` | `MG_BGx` | 0/1 | 檢查軸是否運動中 |
| `MG_MO` | `MG_MOx` | 0/1 | 檢查馬達是否 OFF |
| `MG_SC` | `MG_SCx` | 停止碼 | 取得停止碼 |

### 伺服控制

| 命令 | 格式 | 說明 |
|------|------|------|
| `SH` | `SH A` 或 `SH ABCD` | 伺服 ON |
| `MO` | `MO A` 或 `MO ABCD` | 伺服 OFF |
| `OE` | `OE 1` | 啟用 Off-on-error |

### 極限與保護

| 命令 | 格式 | 說明 |
|------|------|------|
| `BL` | `BL x` | 設定負極限 |
| `FL` | `FL x` | 設定正極限 |
| `CN` | `CN -1` | 設定極限邏輯 |
| `ER` | `ER x,x,x,x` | 設定各軸 Position Error Limit（Following Error 上限，counts）|
| `OE` | `OE n,n,n,n` | Off-on-Error：position error 超過 ER 時的行為（0=關馬達, 1=關馬達+觸發 #POSERR）|
| `TE` | `TE A` | Tell Error：讀取該軸當前 following error（= command - encoder）|

> **V3.33.904 新增 GaliProtectMode**：Mode 1 使用 ER+OE（firmware 自動保護），Mode 2 使用 TE（host polling following error）。Mode 0 維持原始 encoder polling。詳見 `ht9045-index-flow/references/Z1UpZ2Down_MotionSequence.md` Section 7。

### 線性補間

| 命令 | 格式 | 說明 |
|------|------|------|
| `LM` | `LM WXYZ` | 設定線性補間軸 |
| `LI` | `LI x,x,x,x` | 線性補間增量 |
| `LE` | `LE` | 結束線性補間 |
| `VS` | `VS x` | 向量速度 |

---

## TS 開關狀態位元

`TS` 命令回傳的狀態位元定義：

| 位元 | 說明 |
|------|------|
| 0 | Home 感測器 (active low) |
| 1 | Reverse Limit Switch |
| 2 | Forward Limit Switch |
| 3 | 第二 Home |
| 4 | Latch 輸入 |
| 5 | 運動中 |
| 6 | 錯誤 |
| 7 | 運動完成 |

### 範例

```cpp
long status = MOT[MTestY1].Gali_Command("TSW", "ReadStatus");
bool homeOn = (status & 0x01) == 0;   // Home 感測器（低電平有效）
bool moving = (status & 0x20) != 0;   // 運動中
```

---

## 錯誤碼

### DMC 函式錯誤碼

| 錯誤碼 | 常數 | 說明 |
|--------|------|------|
| 0 | SUCCESS | 成功 |
| -1 | DMCERROR_TIMEOUT | 超時 |
| -2 | DMCERROR_COMMAND | 命令錯誤 |
| -4 | DMCERROR_CONTROLLER | 控制器錯誤 |
| -5 | DMCERROR_FILE | 檔案錯誤 |
| -6 | DMCERROR_DRIVER | 驅動程式錯誤 |
| -7 | DMCERROR_HANDLE | Handle 錯誤 |
| -8 | DMCERROR_HMODULE | 模組錯誤 |
| -9 | DMCERROR_MEMORY | 記憶體錯誤 |
| -11 | DMCERROR_FIRMWARE | 韌體錯誤 |
| -14 | DMCERROR_BUSY | 忙碌 |
| -15 | DMCERROR_DEVICE_DISCONNECTED | 裝置斷線 |
| -16 | DMCERROR_TIMEING_ERROR | 時序錯誤 |

### TC 錯誤碼（Galil 控制器）

使用 `TC1` 命令可取得詳細錯誤說明。

---

## TMyMotor Galil 方法實作

### Gali_Command

```cpp
long TMyMotor::Gali_Command(AnsiString Data, AnsiString sFunc)
{
    // 進入臨界區
    EnterCriticalSection(&g_cs);
    
    // 發送命令
    rc = DMCCommand(hDmc, Data.c_str(), szBuffer, sizeof(szBuffer));
    
    // 處理錯誤與重試
    // ...
    
    // 離開臨界區
    LeaveCriticalSection(&g_cs);
    return atol(szBuffer);
}
```

### Gali_MotMove

```cpp
bool TMyMotor::Gali_MotMove(int Pos, int Speed, AnsiString _Func)
{
    // 1. 設定速度 SP
    // 2. 設定加減速 AC, DC
    // 3. 設定目標位置 PA
    // 4. 開始運動 BG
    // 5. 等待運動完成
    // 6. 檢查編碼器位置
}
```

### Gali_MotHome

```cpp
void TMyMotor::Gali_MotHome(AnsiString HomeAxis)
{
    // 1. 設定回原點速度
    // 2. 發送 HM 命令
    // 3. 開始運動 BG
    // 4. 等待 Home 感測器
}
```

### Gali_MotHomeFindZ

```cpp
void TMyMotor::Gali_MotHomeFindZ(AnsiString HomeAxis)
{
    // 1. 執行基本回原點
    // 2. 發送 FI 命令搜尋 Z 相
    // 3. 精確定位
}
```

---

## 雙 Z 軸協調實作

### Z1UpZ2Down

```cpp
bool TMyMotor::Z1UpZ2Down(int Speed, bool TMode, bool bPickErr)
{
    // 使用線性補間 LM
    // 設定 Z1 上升位置、Z2 下降位置
    // 發送 LI 命令
    // 等待運動完成
}
```

### 四軸同動命令範例

```cpp
// 設定線性補間模式
Gali_Command("LM WXYZ", "Setup");

// 設定向量速度
Gali_Command("VS 10000", "SetVectorSpeed");

// 發送補間增量（W, X, Y, Z）
Gali_Command("LI 0,0,1000,-1000", "MoveZ1UpZ2Down");

// 結束補間
Gali_Command("LE", "EndInterpolation");

// 開始運動
Gali_Command("BGS", "StartVector");
```

---

## 常用命令組合範例

### 單軸絕對移動

```cpp
// 設定速度
MOT[idx].Gali_Command("SP 10000", "SetSpeed");
MOT[idx].Gali_Command("AC 50000", "SetAcc");
MOT[idx].Gali_Command("DC 50000", "SetDec");

// 設定目標位置
MOT[idx].Gali_Command("PA 25000", "SetTarget");

// 開始運動
MOT[idx].Gali_Command("BG W", "BeginMotion");
```

### 等待運動完成

```cpp
while (true) {
    long status = MOT[idx].Gali_Command("MG_BGw", "CheckMotion");
    if (status == 0) break;  // 運動完成
    MySleepEx(1, true);
}
```

### JOG 點動

```cpp
// 正向 JOG
MOT[idx].Gali_Command("JG 5000", "SetJogSpeed");
MOT[idx].Gali_Command("BG W", "StartJog");

// 停止
MOT[idx].Gali_Command("ST W", "StopJog");
```

### 伺服 ON/OFF

```cpp
// 伺服 ON
MOT[idx].Gali_Command("SH W", "ServoOn");

// 伺服 OFF
MOT[idx].Gali_Command("MO W", "ServoOff");
```

### 位置重置

```cpp
// 重置命令位置為 0
MOT[idx].Gali_Command("DP 0", "ResetCommandPos");

// 重置編碼器位置為 0
MOT[idx].Gali_Command("DE 0", "ResetEncoderPos");
```

---

## 標頭檔

```cpp
#include "DMCCOM.H"      // DMC 通訊 API
#include "dmcdrc.h"      // DMC 驅動程式常數
#include "DMCMLIB.H"     // DMC 函式庫
```

### 連結庫

- `Dmc32b.lib` - Galil DMC 32-bit 函式庫

---

## 詳細命令參考（官方手冊）

以下內容摘自 Galil_CommandReference_rev1.0o.pdf，適用於 DMC-1xxx 和 DMC-18x2 控制器。

### 命令分類速查

| 分類 | 命令 |
|------|------|
| **運動控制** | AC, BG, DC, IP, IT, JG, PA, PR, PT, RP, SP, ST |
| **位置/原點** | AM, AP, AR, MC, MF, MR, DE, DP, FE, FI, HM |
| **向量/線性** | AV, CA, CR, CS, ES, LE, LI, LM, TN, VA, VD, VE, VM, VP, VR, VS, VT |
| **伺服控制** | DV, FA, FV, IL, KD, KI, KP, KS, MO, NB, NF, NZ, OF, PL, SH, TE, TK, TL, TM, TT |
| **回授** | AF, AL, CE, OC, RL, TD, TP, TV |
| **極限** | BL, FL, ER, OE, SC, TC |
| **I/O** | @AN, @IN, @OUT, AI, CB, CN, CO, II, OB, OP, SB, TI, TS |
| **陣列** | DA, DM, LA, QD, QU, RA, RC, RD, [] |
| **數學** | @ABS, @ACOS, @ASIN, @ATAN, @COM, @COS, @FRAC, @INT, @RND, @SIN, @SQR, @TAN |

---

### AC（Acceleration）加速度

**功能**：設定獨立軸運動的加速度

| 項目 | 說明 |
|------|------|
| **語法** | `AC n,n,n,n,n,n,n,n` 或 `ACA=n` |
| **範圍** | 1024 ~ 67,107,840（counts/sec²）|
| **預設值** | 256000 |
| **運動中可改** | 是（JG 模式）|
| **Operand** | `_ACn` 回傳指定軸的加速度 |

**範例**：
```
AC 150000,200000,,300000   ' 設定 A/B/D 軸加速度，C 軸維持不變
AC ?,?,?                    ' 查詢 A/B/C 軸加速度
```

---

### BG（Begin）開始運動

**功能**：開始指定軸的運動

| 項目 | 說明 |
|------|------|
| **語法** | `BG nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H,S,T,N |
| **引數** | S = 協調序列，T = 第二序列，N = 機械臂 |
| **Operand** | `_BGn` 回傳運動狀態（1=運動中，0=停止）|

**範例**：
```
PR 4000,5000,6000,7000
BG                     ' 開始所有軸
BG AB                  ' 只啟動 A、B 軸
BGS                    ' 啟動協調序列
```

---

### BL（Backward Limit）反向軟體極限

**功能**：設定反向（負方向）軟體極限位置

| 項目 | 說明 |
|------|------|
| **語法** | `BL n,n,n,n,n,n,n,n` 或 `BLA=n` |
| **範圍** | -2,147,483,648 ~ 2,147,483,647 |
| **停用值** | -2,147,483,648（預設）|
| **觸發時機** | 馬達位置 ≤ n 時觸發 |
| **Operand** | `_BLn` 回傳反向極限設定值 |

**範例**：
```
BL -20000,-10000      ' A 軸極限 -20000，B 軸極限 -10000
BL -2147483648        ' 停用 A 軸反向極限
```

---

### DC（Deceleration）減速度

**功能**：設定獨立軸運動的減速度

| 項目 | 說明 |
|------|------|
| **語法** | `DC n,n,n,n,n,n,n,n` 或 `DCA=n` |
| **範圍** | 1024 ~ 67,107,840（counts/sec²）|
| **預設值** | 256000 |
| **運動中可改** | 是（JG 模式）|
| **Operand** | `_DCn` 回傳指定軸的減速度 |

---

### DE（Define Encoder）定義編碼器位置

**功能**：將輔助（雙）編碼器的位置設為指定值

| 項目 | 說明 |
|------|------|
| **語法** | `DE n,n,n,n,n,n,n,n` 或 `DEA=n` |
| **範圍** | -2,147,483,647 ~ 2,147,483,647 |
| **Operand** | `_DEn` 回傳指定軸的輔助編碼器位置 |

**備註**：此命令影響 TD 命令回傳的值

---

### DP（Define Position）定義位置

**功能**：將主編碼器的命令位置和回授位置設為指定值

| 項目 | 說明 |
|------|------|
| **語法** | `DP n,n,n,n,n,n,n,n` 或 `DPA=n` |
| **範圍** | -2,147,483,648 ~ 2,147,483,647 |
| **Operand** | `_DPn` 回傳指定軸的位置定義值 |

**備註**：
- 會同時重設 TP（命令位置）和 RP（回授位置）
- 常用於原點校正

---

### FI（Find Index）搜尋 Index

**功能**：移動馬達直到偵測到編碼器 Index 脈衝，並將該位置定義為零

| 項目 | 說明 |
|------|------|
| **語法** | `FI nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H |
| **速度設定** | 使用 JG 命令預先設定 |

**備註**：
- Index 脈衝通常是每轉一次的 Z 相信號
- 搭配 HM 命令可實現精確原點定位

**範例**：
```
JG 5000            ' 設定搜尋速度
FI A               ' 開始搜尋 A 軸 Index
BG A               ' 執行
```

---

### FL（Forward Limit）正向軟體極限

**功能**：設定正向（正方向）軟體極限位置

| 項目 | 說明 |
|------|------|
| **語法** | `FL n,n,n,n,n,n,n,n` 或 `FLA=n` |
| **範圍** | -2,147,483,647 ~ 2,147,483,647 |
| **停用值** | 2,147,483,647（預設）|
| **觸發時機** | 馬達位置 ≥ n+1 時觸發 |
| **Operand** | `_FLn` 回傳正向極限設定值 |

---

### HM（Home）回原點

**功能**：執行回原點程序

| 項目 | 說明 |
|------|------|
| **語法** | `HM nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H |
| **停止碼** | 完成後 SC 回傳 10 |

**伺服馬達三階段回原點流程**：
1. **尋找 Home**：以 JG 速度朝 Home 開關移動
2. **反向接近**：以 JG 速度 1/3 反向移動，離開 Home 開關
3. **尋找 Index**：以 JG 速度 1/3 正向移動，找到編碼器 Index

**範例**：
```
JG -10000          ' 設定回原點速度（負方向）
HM A               ' 設定 A 軸回原點
BG A               ' 執行
AM A               ' 等待完成
```

---

### JG（Jog）點動

**功能**：設定各軸的點動速度（方向由正負號決定）

| 項目 | 說明 |
|------|------|
| **語法** | `JG n,n,n,n,n,n,n,n` 或 `JGA=n` |
| **伺服範圍** | 0 ~ ±12,000,000（counts/sec）|
| **步進範圍** | 0 ~ ±3,000,000（counts/sec）|
| **Operand** | `_JGn` 回傳指定軸的點動速度 |

**備註**：
- 正值 = 正向移動，負值 = 負向移動
- 運動中可即時變更速度（含方向）

**範例**：
```
JG 5000,-10000     ' A 軸正向 5000，B 軸負向 10000
BG AB              ' 開始點動
ST AB              ' 停止
```

---

### LI（Linear Interpolation Distance）線性補間距離

**功能**：指定線性補間模式下各軸的增量移動距離

| 項目 | 說明 |
|------|------|
| **語法** | `LI n,n,n,n,n,n,n,n <o >p` |
| **距離範圍** | -8,388,607 ~ 8,388,607（counts）|
| **速度 o** | 段落起始向量速度（0 ~ 12,000,000）|
| **速度 p** | 段落結束向量速度 |
| **緩衝區** | 最多 511 筆 LI 命令 |

**備註**：
- 使用前須先執行 LM 設定參與軸
- 最後一筆須以 LE 命令結束
- 使用 `LM ?` 查詢可用緩衝空間

**範例**：
```
LM ABC             ' 設定 A/B/C 軸線性補間
LI 1000,2000,3000  ' 第一增量
LI 500,500,500     ' 第二增量
LE                 ' 結束
BGS                ' 執行
```

---

### LM（Linear Interpolation Mode）線性補間模式

**功能**：設定線性補間模式並指定參與軸

| 項目 | 說明 |
|------|------|
| **語法** | `LM nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H |
| **查詢** | `LM ?` 回傳緩衝區可用空間（0~511）|
| **座標系** | 使用 CAS/CAT 選擇 S 或 T 座標系 |
| **Operand** | `_LMn` 回傳座標系 n 的緩衝空間 |

**向量速度計算**：
$$VS = \sqrt{AS^2 + BS^2 + CS^2}$$

---

### MO（Motor Off）馬達斷電

**功能**：關閉指定軸的伺服控制演算法

| 項目 | 說明 |
|------|------|
| **語法** | `MO nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H |
| **無引數** | 關閉所有軸 |
| **Operand** | `_MOn` 回傳馬達狀態（1=OFF，0=ON）|

**範例**：
```
MO                 ' 關閉所有馬達
MO A               ' 只關閉 A 軸
SH A               ' 重新啟用 A 軸伺服
```

---

### PA（Position Absolute）絕對位置移動

**功能**：設定各軸的絕對目標位置

| 項目 | 說明 |
|------|------|
| **語法** | `PA n,n,n,n,n,n,n,n` 或 `PAA=n` |
| **範圍** | -2,147,483,647 ~ 2,147,483,648（counts）|
| **運動中可改** | 否 |
| **Operand** | `_PAn` 回傳上次停止時的命令位置 |

**範例**：
```
PA 400,-600,500,200   ' 設定各軸目標
BG                    ' 開始運動
AM                    ' 等待完成
PA ?,?,?,?            ' 查詢停止位置
```

---

### PR（Position Relative）相對位置移動

**功能**：設定各軸的相對移動距離

| 項目 | 說明 |
|------|------|
| **語法** | `PR n,n,n,n,n,n,n,n` 或 `PRA=n` |
| **範圍** | -2,147,483,648 ~ 2,147,483,647（counts）|
| **Operand** | `_PRn` 回傳指定軸的增量距離 |

**範例**：
```
PR 100,200,300,400
BG                    ' A 前進 100，B 前進 200，...
```

---

### SC（Stop Code）停止碼

**功能**：查詢馬達停止原因

| 項目 | 說明 |
|------|------|
| **語法** | `SC nnnnnnnn` |
| **Operand** | `_SCn` 回傳指定軸的停止碼 |

**停止碼定義**：

| 碼 | 說明 | 碼 | 說明 |
|---|------|---|------|
| 0 | 運動中（獨立模式）| 9 | FE 找邊緣停止 |
| 1 | 到達命令位置 | 10 | HM 回原點完成 |
| 2 | 正向極限停止 | 11 | 選擇性中止輸入 |
| 3 | 反向極限停止 | 50 | 輪廓運動中 |
| 4 | ST 命令停止 | 51 | 輪廓停止 |
| 6 | Abort 輸入停止 | 99 | MC 超時 |
| 7 | AB 命令停止 | 100 | 運動中（向量序列）|
| 8 | OE 誤差停止 | 101 | 向量序列到達位置 |

---

### SH（Servo Here）伺服啟用

**功能**：將目前位置設為命令位置並啟用伺服控制

| 項目 | 說明 |
|------|------|
| **語法** | `SH nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H |
| **無引數** | 啟用所有軸 |

**備註**：
- 執行後會改變座標系統
- 之前的位置命令需重新發送

---

### SP（Speed）速度

**功能**：設定獨立軸運動的速度

| 項目 | 說明 |
|------|------|
| **語法** | `SP n,n,n,n,n,n,n,n` 或 `SPA=n` |
| **伺服範圍** | 0 ~ 12,000,000（counts/sec）|
| **步進範圍** | 0 ~ 3,000,000（counts/sec）|
| **預設值** | 25000 |
| **Operand** | `_SPn` 回傳指定軸的速度 |

**備註**：負值會被當作絕對值處理

---

### ST（Stop）停止

**功能**：減速停止指定軸的運動

| 項目 | 說明 |
|------|------|
| **語法** | `ST nnnnnnnn` 其中 n = A,B,C,D,E,F,G,H,N,S,T |
| **無引數** | 停止所有軸並停止程式執行 |

**範例**：
```
ST A               ' 只停止 A 軸
ST S               ' 停止協調序列
ST ABCD            ' 停止 A/B/C/D 軸
```

---

### TC（Tell Error Code）錯誤碼查詢

**功能**：回傳最後一個命令錯誤的原因碼

| 項目 | 說明 |
|------|------|
| **語法** | `TC n` |
| **引數** | n=0 只回傳碼，n=1 回傳碼與訊息 |

**常見錯誤碼**：

| 碼 | 說明 | 碼 | 說明 |
|---|------|---|------|
| 1 | 無法識別的命令 | 16 | IP 符號錯誤或強制減速 |
| 4 | 運算元錯誤 | 20 | 馬達 OFF 時無法 Begin |
| 6 | 數值超出範圍 | 21 | 運動中無法 Begin |
| 7 | 運動中不可執行 | 22 | 因極限無法 Begin |

---

### TD（Tell Dual Encoder）查詢輔助編碼器

**功能**：回傳輔助（雙）編碼器的位置

| 項目 | 說明 |
|------|------|
| **語法** | `TD nnnnnnnn` |
| **Operand** | `_TDn` 回傳輔助編碼器位置 |

**備註**：步進馬達模式下回傳已輸出的脈衝數

---

### TP（Tell Position）查詢位置

**功能**：回傳馬達的目前位置

| 項目 | 說明 |
|------|------|
| **語法** | `TP nnnnnnnn` |
| **Operand** | `_TPn` 回傳指定軸的位置 |

**範例**：
```
TP                 ' 查詢所有軸
TPA                ' 只查詢 A 軸
Position=_TPA      ' 存入變數
```

---

### TS（Tell Switches）查詢開關狀態

**功能**：回傳軸的開關與狀態位元

**位元定義**（從已有章節補充詳細說明）：

| 位元 | 值 | 說明 |
|------|-----|------|
| 0 | 0x01 | Home 感測器（active low）|
| 1 | 0x02 | 反向極限開關 |
| 2 | 0x04 | 正向極限開關 |
| 3 | 0x08 | 第二 Home/Latch |
| 4 | 0x10 | Latch 輸入狀態 |
| 5 | 0x20 | 運動中 |
| 6 | 0x40 | 發生錯誤 |
| 7 | 0x80 | 運動完成（AMn）|

---

### 向量運動命令

#### VA（Vector Acceleration）

| 項目 | 說明 |
|------|------|
| **語法** | `VA n` |
| **範圍** | 1024 ~ 67,107,840（counts/sec²）|
| **Operand** | `_VA` |

#### VD（Vector Deceleration）

| 項目 | 說明 |
|------|------|
| **語法** | `VD n` |
| **範圍** | 1024 ~ 67,107,840（counts/sec²）|
| **Operand** | `_VD` |

#### VS（Vector Speed）

| 項目 | 說明 |
|------|------|
| **語法** | `VS n` |
| **伺服範圍** | 0 ~ 12,000,000（counts/sec）|
| **步進範圍** | 0 ~ 3,000,000（counts/sec）|
| **Operand** | `_VS` |

---

## Operand 使用摘要

Galil 控制器支援以 `_` 開頭的 Operand 語法，可直接在表達式中使用：

```
' 讀取狀態
IF (_BGA = 1)          ; A 軸運動中
IF (_MOA = 1)          ; A 軸馬達 OFF
Bob = _SCA             ; 讀取停止碼存入變數

' 讀取位置
CurrentPos = _TPA      ; 讀取命令位置
DualPos = _TDA         ; 讀取輔助編碼器
Speed = _SPA           ; 讀取速度設定
```

---

## 資源連結

- **官方手冊**：Galil_CommandReference_rev1.0o.pdf（DMC-1xxx / DMC-18x2）
- **控制器系列**：Optima DMC-1xxx、DMC-18x2
