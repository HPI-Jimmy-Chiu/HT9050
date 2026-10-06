> 保存來源：`.claude/skills/ht9045-general-ini/references/cross-reference.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# 三模組交叉比對 — Gerneral.ini Key 讀寫分佈

## 目錄

- [模組角色摘要](cross-reference.md#模組角色摘要)
- [三模組重疊 Key](cross-reference.md#三模組重疊-key)
- [database + HandlerSys 重疊（main 不參與）](cross-reference.md#database--handlersys-重疊main-不參與)
- [database + main 重疊（HandlerSys 不參與）](cross-reference.md#database--main-重疊handlersys-不參與)
- [HandlerSys 獨有 Key](cross-reference.md#handlersys-獨有-key)
- [database 獨有 Key](cross-reference.md#database-獨有-key)
- [main 獨有 Key](cross-reference.md#main-獨有-key)
- [跨模組寫入衝突風險](cross-reference.md#跨模組寫入衝突風險)
- [資料流圖](cross-reference.md#資料流圖)

---

## 模組角色摘要

| 模組 | 讀取 Key 數 | 寫入 Key 數 | 角色定位 |
|------|------------|------------|----------|
| database.cpp `ReadGeneralIni()` | ~400 | ~10 (條件回寫) | 啟動載入，全域變數初始化 |
| main.cpp `TfMain()`/`FormShow()` | ~30 | ~40 (含 AOA 30) | 遷移補寫 + AOA 校正寫入 |
| HandlerSys.cpp `Load/SaveSystemSet()` | ~170 | ~170 | UI ↔ INI 雙向（使用者操作觸發） |

---

## 三模組重疊 Key

以下 Key 在三個模組中皆有存取：

| Section | Key | database | main | HandlerSys |
|---------|-----|----------|------|------------|
| System | EP_Install | 讀 | 遷移+讀 | 讀+寫 |
| System | INOUT_ARM_PICKER_USE_MOTOR | 讀 | 遷移+讀+修正 | 讀+寫 |
| System | SUPPORT_2_EMPTY_EMPTY | 讀 | 遷移+讀 | (條件寫) |
| System | ION_FAN_TYPE | 讀 | 遷移+讀 | 讀+寫 |
| System | INDEX_SUCKER_TYPE | 讀 | 遷移+讀 | 讀+寫 |
| System | EP_MAXKPA | 讀 | 讀 | 讀+寫 |
| System | EP_MAXA | 讀 | 讀 | 讀+寫 |
| System | EP_MINMPA | 讀 | 讀 | 讀+寫 |
| System | EP_MINA_FeedBack | 讀 | 讀 | 讀+寫 |
| System | CUSTOMER_CODE | 讀 | 讀 | 讀+寫 |
| Version | Ver | 讀 | 寫 (ATC) | 讀+寫 |

---

## database + HandlerSys 重疊（main 不參與）

大部分硬體選配旗標屬於此類（約 150+ Key），包含：

| 分類 | 代表 Key |
|------|---------|
| Tray Z 馬達 | LOAD/EMPTY/COLOR/AUTO1-6_Z_USE_MOTOR |
| Cassette 模式 | LOAD/EMPTY/COLOR/AUTO1-6_USE_Cassette |
| ART 配置 | UNLOADER_AUTO1-6_ART |
| 溫控 | HEATER_CTRL_TYPE, COM_PORT, COM_PORT_OMRON |
| ATC | USE_ATC_MODE, ATC1-4_COM_PORT, ATC_SYSTEM_IP/PORT |
| 2D 條碼 | BAR_CODE_INSTALL, BOTTOM_2DID, BarCode1-4_COM_PORT |
| Shuttle | SHUTTLE_SENSOR_TYPE, SHUTTLE_Z_TYPE, ShuttleVibration |
| Pitch | USE_IN_OUT_ARM_Y/X_PITCH, MIN/MAX 值 |
| Motion/IO | MOTION_CARD_TYPE, IO_CARD_TYPE, TTL_CARD_TYPE |
| Index | INDEX_PRESS_TYPE, INDEX_MOTION_CARD |
| OCR | INSTALL_OCR, OCR_COM_PORT |
| RotateKit | USE_ROTATE_KIT, RotateKit_Type |
| ESD/ION | USE_ESD_Monior, USE_NOVX3360, USE_KASUGA |
| 溫度限制 | HighTempLimit, HighTemperatureSet150/155/175 |
| GroundMan | USE_GROUND_MAN, COM_PORT, ScanPoint, AlarmOhm |
| 三溫 | Tri_Temp_Machine, MaxDegree, MinDegree 等 |
| ... | （完整清單見各 mapping 文件） |

---

## database + main 重疊（HandlerSys 不參與）

| Section | Key | database 角色 | main 角色 |
|---------|-----|--------------|-----------|
| System | bHasEnteredPEModel | 讀 | 寫 |
| System | AOA_InArm_* (10 Key) | 讀 | 寫 |
| System | AOA_OutArm_* (20 Key) | 讀 | 寫 |

> AOA 系列 Key 的資料流：main.cpp (CCD校正) → 寫入 INI → database.cpp (啟動讀取) → 全域變數

---

## HandlerSys 獨有 Key

以下 Key **僅在 HandlerSys.cpp 中讀寫**：

| Section | Key | 說明 |
|---------|-----|------|
| MachineDefine | Tri_Temp_Machine | 三溫機器（System 有同名 Key，但 MachineDefine 為額外寫入） |
| EM Aware | EM_AWARE_PORT1-4 | ESD 監控埠（4 個） |
| EM Aware | EM_AWARE_USE_4_COM | ESD 使用 4 COM |
| EM Aware | NOVX_3360_PORT | Simco ION 風扇埠 |
| 外部 INI | COMPort/CommName (RS232Standard) | Tester COM |
| 外部 INI | COMPort_TTL/CommName (RS232Standard) | TTL COM |
| 外部 INI | COMPort_TTL_2/CommName (RS232Standard) | TTL2 COM |

---

## database 獨有 Key

以下 Key **僅在 database.cpp 中讀取**：

| 分類 | Key |
|------|-----|
| 溫度詳細 | AMBIENT_TEMP_CHECK01-71, SHUTTLE_COOLING, VORTEX_COOLING, SOCKET_OFFSET, Socket |
| 溫控進階 | TEMPCTRL_NEED_UNDER_20A, UseHotGunCheck, UseHotGunFlowCheck, TEMPCTRL_HOTPLATE_TOGTHER, bTEMPCTRL_Shuttle_TOGTHER |
| 通訊參數 | 2D_BarCode BaudRate/ByteSize/StopBit/Parity, RFID BaudRate/ByteSize/StopBit/Parity |
| Fine Pitch | COM_PORT, COM_PORT_Adjustment |
| 馬達 | MotorDriver/Type |
| 網路裝置 | Fix_AI_CCD 全部 IP/Port, Tray_Mapping 全部 IP/Port, Auto_Alignment 全部 IP/Port |
| 自動教導 | AutoTeach 全部 Key |
| ATC 進階 | USE_ATC_SELFTEST |
| 旋轉 | ROTATE_KIT/SpecialSequence |
| Index | IndexDriver/USE_ReadIndex_TOQUE |
| 速度/時間 | T_MODE_SPEED, FixTrayDataCleanTime |
| 旗標 | bHT9045S_USE2x4, bBarCodeRules, OTDRecord, USE_BARCODE_AS_KEYBOARD, USE_ARM_PROTECTION |
| 查詢 | iDBQueryDays, iMagazineCheckZPos |
| 客戶 | JCET_FOR_EVAN, iAutoFormSize |
| 系統 | OFFLINE_ALARM, CHECK_EP_SETTING, USE_InPlacement, USE_ATC_RS232_Check |
| 面板 | NUMBER_PANEL_DELAY |

---

## main 獨有 Key

以下 Key **僅在 main.cpp 中處理**：

| Section | Key | 說明 |
|---------|-----|------|
| System | EPDual_MAXKPA / EPDual_MAXAFB | 雙 EP 壓力上限 |
| System | EPDual_MINMPA / EPDual_MinAFB | 雙 EP 壓力下限 |
| System | bContaceTorque | TSMC 接觸力矩 |
| System | bUse_NewAutoCleanForm | 新清潔表單 |
| System | bUseNewCleanModeKit | 新清潔 Kit |
| System | SetupFileCheckList | Setup 檔案名 |
| System | bInitialCleanCount | TSMC TAINAN 清潔計數 |
| System | IndexTimeSet | Index 時間設定 |
| In Arm | ZSafePos | Z 安全位置 |
| VENDER | ASEK15UsePW / ASEK15PassWord | ASEK15 密碼 |
| VENDER | HONTECH / HONPREC | 鴻勁密碼 |
| Record | Program Close | 程式啟閉旗標 |
| Password | Change | 密碼變更旗標 |

---

## 跨模組寫入衝突風險

| Key | 寫入模組 | 風險描述 |
|-----|----------|----------|
| EP_Install | main (遷移), HandlerSys (UI) | main 僅在 Key 不存在時寫入，無衝突 |
| INOUT_ARM_PICKER_USE_MOTOR | main (修正), HandlerSys (UI) | main 讀到 0 時強制寫 1，會覆寫 HandlerSys 設定 |
| Version/Ver | main (ATC關閉), HandlerSys (UI) | 兩者皆寫不同時機，以最後寫入為準 |
| bHasEnteredPEModel | database (讀), main (寫) | 正常流程：main 寫 → 下次啟動 database 讀 |
| AOA_* | database (讀), main (寫) | 正常流程：main 校正寫 → 下次啟動 database 讀 |
| USE_46_SUCKER/SENSOR_DB | database (條件回寫), HandlerSys (UI) | database 在 9046/1032 機型強制歸零，可能覆寫 UI 設定 |

---

## 資料流圖

```
┌──────────────────────────────────────────────────────────────────┐
│                    Gerneral.ini (300+ Key)                       │
│                    D:\HT9045\system\                             │
└───────┬──────────────────┬──────────────────┬────────────────────┘
        │                  │                  │
        ▼                  ▼                  ▼
┌───────────────┐  ┌───────────────┐  ┌─────────────────┐
│  database.cpp │  │   main.cpp    │  │ HandlerSys.cpp  │
│ ReadGeneralIni│  │ TfMain/Show   │  │ Load/SaveSystem │
├───────────────┤  ├───────────────┤  ├─────────────────┤
│ 順序：① 最早  │  │ 順序：② 次之  │  │ 順序：③ 按需   │
│ 讀：~400 Key  │  │ 遷移：~15 Key │  │ 讀：~170 Key    │
│ 寫：~10 Key   │  │ 讀：~30 Key   │  │ 寫：~170 Key    │
│  (條件回寫)   │  │ 寫：~40 Key   │  │  (UI→INI 同步)  │
│               │  │ (含 AOA 30)   │  │                 │
├───────────────┤  ├───────────────┤  ├─────────────────┤
│      ↓        │  │      ↓        │  │      ↕          │
│ 全域變數初始化│  │ 全域變數補充   │  │ UI ↔ 全域變數   │
│ (~400 個)     │  │ + 安全修正     │  │ (~170 個)       │
└───────────────┘  └───────────────┘  └─────────────────┘
        │                  │                  │
        ▼                  ▼                  ▼
┌──────────────────────────────────────────────────────────────────┐
│                    全域變數 (程式運行時使用)                      │
│  CUSTOMER_CODE, EP_Install, MOTION_CARD_TYPE, ATC_SYSTEM, ...   │
└──────────────────────────────────────────────────────────────────┘
```

### 外部 INI 檔資料流

```
D:\GPIB9045\system\general.ini
  ├─ database.cpp 讀取 Version/Model → MachineTypeChoice
  └─ HandlerSys.cpp 寫入 Version/Model (UI 更新時同步)

D:\RS232Standard\System\Setup.ini
  └─ HandlerSys.cpp 寫入 COMPort/CommName (Tester/TTL COM Port)
```

### 新增 Key 時的 Checklist

1. ☐ `database.cpp::ReadGeneralIni()` — 加入讀取至全域變數（啟動時載入）
2. ☐ `HandlerSys.cpp::LoaderSystemSet()` — 加入 INI → UI 讀取
3. ☐ `HandlerSys.cpp::SaveSystemSet()` — 加入 UI → INI 寫入 + 全域變數同步
4. ☐ `HandlerSys.h` — 如需 UI 元件，加入 TRadioGroup/TCheckBox/TEdit 宣告
5. ☐ `HandlerSys.dfm` — 如需 UI 元件，加入表單設計
6. ☐ 考慮是否需要 main.cpp 遷移邏輯（舊機台升級相容）

<!-- preserved-content:end -->
