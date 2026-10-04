---
name: ht9045-motor-control
description: HT9045 Handler 馬達控制層模式。適用於馬達移動、回原點、JOG 點動、伺服控制或底層馬達卡操作。涵蓋 HTMotor、TMyMotor、TTrayMotor、TMySYNTEKMotor、TMyMN200Motor、TMyEtherCatMotor、TMyGALILMotor、TMySMCMotor、Hontech_M4 及 Galil DMC 整合。用於馬達修改、馬達問題除錯、新增馬達定義或理解馬達控制流程。觸發關鍵字：motor, 馬達, Galil, MN200, SYNTEK, PCI-L132, 回原點, home, JOG, servo, PISO-MN200, MotionNet, 泓格, ICP-DAS, mn_fix_move, mn_velocity_move, mn_home_start, EtherCAT, PCI1203, Advantech, SMC, CONTEC, Hontech, M2X4, SortArm
---

<!-- AI(W906-BA-SKILL) 20260915：這支 skill 來自網頁同事 20260915 交付的
     D:\HT9050\HT9045_skills_20260915\skills\ht9045-motor-control，內容未改。
     ⚠ 資料通道以使用者 20260915 裁決為準：wb_serve HTTP 直讀 .Data +
       recipe.doc.put 寫。本 skill 若描述 JSON-only 或 C# Simulator 通道，
       那是同事那邊的舊架構，不是本專案的正式路徑。 -->

# HT9045 馬達控制層

Handler 機台馬達控制模式，涵蓋步進/伺服馬達驅動、多種運動控制卡整合。

## 相關技能與代理

- **技能**: `ht9045-io-control` - IO 控制層（氣缸/感測器/開關）
- **技能**: `bcb_build` - BCB6 編譯
- **技能**: `pre-release-check` - 程式碼風險掃描
- **AGENTS.md**: `D:\HT9045\AGENTS.md` - 專案概覽

## 快速參考

| 類別 | 說明 | 標頭檔 |
|------|------|--------|
| `HTMotor` | 馬達基底類別（抽象介面） | `HTMotor.h` |
| `TMyMotor` | 馬達控制封裝（含 Galil 整合） | `mymotor.h` |
| `TTrayMotor` | Tray 馬達（繼承 TMyMotor） | `mymotor.h` |
| `TMySYNTEKMotor` | SYN-TEK MotionNet 馬達 | `mySYNTEKmotor.h` |
| `TMyMN200Motor` | ICP-DAS MN200 馬達 | `myMN200motor.h` |
| `TMyMC88X1Motor` | MC88X1 馬達控制 | `HTMC88X1Motor.h` |
| `TMyEtherCatMotor` | Advantech EtherCAT 馬達 | `myEthercatmotor.h` |
| `TMyGALILMotor` | Galil DMC 馬達（獨立實作） | `myGALILmotor.h` |
| `TMySMCMotor` | CONTEC SMC 馬達 | `mySMCmotor.h` |
| `Hontech_M4` | 泓格 MotionNet M2X4 C API | `Hontech_M4.h` |

## 馬達類型常數

```cpp
const int Step_Motor         = 0;   // 步進馬達
const int Servo_Motor        = 1;   // 伺服馬達
const int Rotate_Motor       = 2;   // 旋轉馬達
const int YASKAWA_Servo_Motor= 3;   // Yaskawa 伺服馬達
const int YASKAWA_Liner_Motor= 4;   // Yaskawa 線性馬達
const int Step_Motor_Oriental= 5;   // Oriental 步進馬達
```

## 控制卡整合

| 控制卡 | 供應商 | 用途 | API 參考 |
|--------|--------|------|----------|
| **Galil DMC** | Galil | Index Arm (Test Y/Z) | [galil-api.md](references/galil-api.md) |
| **PCI-L132** | SYN-TEK | MotionNet 主控卡 | [motionnet-api.md](../ht9045-io-control/references/motionnet-api.md) |
| **106-M2x4** | SYN-TEK | MotionNet 4軸 Slave | [syntek-motion-slave-api.md](references/syntek-motion-slave-api.md) |
| **PISO-MN200** | 泓格 (ICP-DAS) | MotionNet 馬達 | [mn200-api.md](references/mn200-api.md) |
| **MC88X1P** | 和椿 | 4/8軸 PCI 軸卡 | [mc88x1-api.md](references/mc88x1-api.md) |
| **SMC-xDF2-PCI** | CONTEC | 4/8軸 PCI 軸卡 | [smc-api.md](references/smc-api.md) |
| **PCI-1203** | Advantech | EtherCAT 主控卡 | [ethercat-api.md](references/ethercat-api.md) |
| **Hontech M2X4/M1X4** | 泓格 (ICP-DAS) | MotionNet 4軸馬達驅動 | `Hontech_M4.h` |

## 類別繼承架構

```
HTMotor (基底類別)
    │
    ├── TMySYNTEKMotor      (PCI-L132 SYN-TEK MotionNet)
    ├── TMyMN200Motor       (PISO-MN200 ICP-DAS MotionNet)
    ├── TMyMC88X1Motor      (MC88X1 和椿)
    ├── TMyEtherCatMotor    (PCI-1203 Advantech EtherCAT)
    ├── TMyGALILMotor       (Galil DMC 獨立實作)
    └── TMySMCMotor         (SMC-xDF2 CONTEC)

TMyMotor (馬達控制封裝)
    │
    ├── Motor: HTMotor*     (指向實際馬達實作)
    ├── Galil 控制整合
    └── 安全門檢查整合
        │
        └── TTrayMotor      (Tray 馬達，含 Tray 資料管理)

Hontech_M4 (C API，非 C++ 類別)
    └── MotionNet M2X4/M1X4 4軸馬達驅動（直線插補、S-curve）

TdmTrayMotor (DataModule，RS-232 序列通訊)
    └── Tray 軌道步進馬達（Load, Auto1~5, Empty, Color）
```

## 全域馬達陣列

```cpp
TTrayMotor MOT[300];   // 所有馬達實例（最大 300 個）

// 常用馬達索引定義在 cmydef.h
// MTestY1, MTestY2, MTestZ1, MTestZ2 等
```

---

## 核心使用模式

### 馬達移動

```cpp
// 基本移動（阻塞式）
int result = MOT[idx].MotorMove(targetPos);
// 回傳值: 1=成功, 0=移動中, -1=安全門開, -2/-3=超限, -4=Alarm

// Galil 馬達移動
bool ok = MOT[idx].Gali_MotMove(targetPos, speed, "FunctionName");

// Galil 非阻塞移動
bool ok = MOT[idx].Gali_MotMoveNoWait(targetPos, speed, delayTime);
```

### 回原點

```cpp
// 基本回原點
int result = MOT[idx].MotorHome(flag);
// 回傳值: 100=完成, 0~99=進行中

// Galil 回原點
MOT[idx].Gali_MotHome("A");  // 軸名稱: A, B, C, D
MOT[idx].Gali_MotHomeFindZ("A");  // 含 Z 相搜尋
```

### JOG 點動

```cpp
// 基本 JOG
MOT[idx].JogP(speed);   // 正向點動
MOT[idx].JogN(speed);   // 負向點動

// Galil JOG
MOT[idx].Gali_JogP(speed);
MOT[idx].Gali_JogN(speed);
MOT[idx].Gali_JogPAndCount(speed, pulseCount);  // 限定脈波數
```

### 伺服控制

```cpp
MOT[idx].ServoOnOff(true);   // 伺服 ON
MOT[idx].ServoOnOff(false);  // 伺服 OFF
```

### 位置讀取

```cpp
int cmdPos = MOT[idx].ReadPos();           // 命令位置
int encPos = MOT[idx].ReadEncoderPos();    // 編碼器位置

// Galil
long pos = MOT[idx].Gali_ReadPos();
long enc = MOT[idx].Gali_ReadEncoderPos();
```

### 雙 Z 軸協調（Index Arm）

```cpp
// Z1 上升 + Z2 下降
bool ok = MOT[idx].Z1UpZ2Down(speed, testMode, pickErr);

// Z1 下降 + Z2 上升
bool ok = MOT[idx].Z1DownZ2Up(speed, testMode, pickErr);

// 檢查狀態
bool z1Up = MOT[idx].ISZ1Up_Z2Down();
bool z1Dn = MOT[idx].ISZ1Down_Z2Up();
```

---

## Galil DMC 控制卡

HT9045 使用 Galil DMC 控制 Index Arm 的 Y/Z 軸馬達。

### 系統配置

```cpp
HANDLEDMC hDmc;           // Galil 卡片 Handle
bool bGali_CardInstall;   // 安裝旗標
bool GaliAxisAlarm[4];    // 軸警報狀態 (A/B/C/D)
```

### Galil 命令介面

```cpp
long rc = MOT[idx].Gali_Command("PA 10000", "MoveToPos");
// 發送 Galil 命令字串，回傳錯誤碼
```

### 常用 Galil 命令

| 命令 | 說明 |
|------|------|
| `PA x` | 絕對位置移動 |
| `PR x` | 相對位置移動 |
| `SP x` | 設定速度 |
| `AC x` | 設定加速度 |
| `DC x` | 設定減速度 |
| `BG A` | 開始軸 A 運動 |
| `ST A` | 停止軸 A |
| `HM A` | 軸 A 回原點 |
| `TP A` | 讀取軸 A 位置 |
| `TS A` | 讀取軸 A 開關狀態 |
| `SH A` | 伺服 ON |
| `MO A` | 伺服 OFF |

### Galil 軸名稱對應

| 馬達 | Galil 軸 | 說明 |
|------|----------|------|
| MTestY1 | A | Index Arm Y1 |
| MTestY2 | B | Index Arm Y2 |
| MTestZ1 | C | Index Arm Z1 |
| MTestZ2 | D | Index Arm Z2 |

### 開啟/關閉 Galil 卡片

```cpp
bool Open_GaliCard();   // 開啟 Galil 卡片
bool Close_GaliCard();  // 關閉 Galil 卡片
```

---

## 安全機制

### 安全門檢查

所有馬達移動前皆會呼叫 `CheckIsSafeDoorOpen()`：

```cpp
bool HTMotor::CheckIsSafeDoorOpen()
{
    if (MotorIdleSafeDoorCheck != NULL) {
        return MotorIdleSafeDoorCheck();  // 回呼函式
    }
    return Enable;
}
```

### Z 軸安全位置

```cpp
int ZSafePos = 20;       // Z 軸安全位置
int ZlimitPos = -3200;   // Z 軸極限位置（吸嘴）
```

### 軟體極限

```cpp
MOT[idx].Motor->PSoftLimitP = 999999;   // 正向軟體極限
MOT[idx].Motor->PSoftLimitN = -999999;  // 負向軟體極限
```

### 編碼器容許誤差

```cpp
int iEncoderTorence = 500;   // 編碼器到位容許範圍
int iCheckZ = 4000;          // 輕壓速度的放寬範圍
```

---

## 馬達移動回傳值

| 值 | 說明 |
|----|------|
| 1 | 移動成功 |
| 0 | 移動中 |
| -1 | 安全門開啟 |
| -2 | 目標超過正極限 |
| -3 | 目標超過負極限 |
| -4 | 伺服 Alarm |
| -5 | 編碼器錯誤 |
| -6 | Home 感測器錯誤 |

---

## TTrayMotor 額外功能

`TTrayMotor` 繼承 `TMyMotor`，額外管理 Tray 資料：

```cpp
class TTrayMotor : public TMyMotor {
    HTray Tray;         // Tray 資料結構
    bool fHasTray;      // 是否有 Tray
    bool HasIC();       // 檢查是否有 IC
    bool HasRealIC();   // 檢查是否有真實 IC
    void InitNewTray(int data, bool showSiteMap, AnsiString func);
    void ClearTray(AnsiString func);
    void SetTrayBinData(int x, int y, int data, AnsiString info);
    int HowManyDevice(int type);
    // ...
};
```

---

## 詳細參考

類別成員細節與配置模式請參閱：
- [motor-classes.md](references/motor-classes.md) - 完整類別成員參考
- [galil-api.md](references/galil-api.md) - Galil DMC API 參考（TMyMotor 高階封裝）
- [galil-motor-api.md](references/galil-motor-api.md) - TMyGALILMotor HTMotor 子類別 API
- [mn200-api.md](references/mn200-api.md) - 泓格 PISO-MN200 MotionNet API 參考
- [mc88x1-api.md](references/mc88x1-api.md) - 和椿 MC88X1P 系列軸卡 API
- [smc-api.md](references/smc-api.md) - CONTEC SMC 系列軸卡 API
- [smc-motor-api.md](references/smc-motor-api.md) - TMySMCMotor 多速度模式 API
- [syntek-motion-slave-api.md](references/syntek-motion-slave-api.md) - 先達 106-M2x4 Motion Slave API
- [ethercat-api.md](references/ethercat-api.md) - Advantech EtherCAT 馬達 API
- [hontech-m4-api.md](references/hontech-m4-api.md) - 泓格 MotionNet M2X4 C API
- [tray-step-motor-api.md](references/tray-step-motor-api.md) - Tray 軌道步進馬達（RS-232）API
- [index-torque-autoheight.md](references/index-torque-autoheight.md) - Index Z 扭力讀值：golden 國際牌 RS-232（rs232.cpp 0x52、k/20、負值歸 0）↔ 安川 Σ-X 6077h（0.1 %、2704h）、正負號比較規則（Steven Q87）、自動測高流程與 port 防護（20261003）
- [ht9050-1203-runtime-traps.md](references/ht9050-1203-runtime-traps.md) - HT9050（PCI-1203＋DS402）上 golden 流程的執行期陷阱：MOTION_CARD_TYPE=0 讓伺服 ON 寫 0、route 拒絕 DS402 座標寫入但 Reload／InitMotor1203 直接寫、iInposLed 永遠 false（料盤手臂防撞失效）、原點燈當 Z 安全、移動中驅動器警報不報、WAR16122 沒編又撞碼、W906_IsHT9050 只在機台樹（S-26，20261004）
