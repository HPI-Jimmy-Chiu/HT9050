# GPIB Program Manual

**Handler Type**：HT-9XXX  
**版本**：V12.04  
**原始文件**：`GPIB_Program Manual_V12.04.doc`  
**目前程式版本**：V12.13.884.0（參照 `MessageDef.cpp`）  
**開發環境**：Borland C++ Builder 6（BCB6），VCL 框架

> 本手冊描述 `H9046_32GPIB.exe` 與 Handler 主程式（`HandlerSys.exe`）之間的
> IPC 通訊協定（WM_COPYDATA）、資料結構定義與函式使用說明。

---

## 目錄

1. [程式架構概述](#1-程式架構概述)
2. [資料結構說明](#2-資料結構說明)
   - [VM 結構（GPIB → Handler）](#vm-結構gpib--handler)
   - [MV 結構（Handler → GPIB）](#mv-結構handler--gpib)
3. [功能代碼說明（MSG_CMD）](#3-功能代碼說明msg_cmd)
   - [基礎 / 測試流程類](#31-基礎--測試流程類)
   - [機台動作狀態類](#32-機台動作狀態類)
   - [取得機台狀態類](#33-取得機台狀態類)
   - [控制 GPIB 程式類](#34-控制-gpib-程式類)
   - [設定機台參數類](#35-設定機台參數類)
   - [ART 專用類](#36-art-專用類)
   - [2D Barcode / 2DID 類](#37-2d-barcode--2did-類)
   - [溫控相關類](#38-溫控相關類)
   - [SIGURD / Novatek 客製類](#39-sigurd--novatek-客製類)
   - [其他 / 新增類](#310-其他--新增類)
   - [已棄用 / 僅 RS232 使用](#311-已棄用--僅-rs232-使用)
4. [使用函式說明](#4-使用函式說明)
5. [通訊機制說明](#5-通訊機制說明)
6. [eTestMode 測試模式定義](#6-etestmode-測試模式定義)
7. [版本說明](#7-版本說明)

---

## 1. 程式架構概述

```
HandlerSys.exe                      H9046_32GPIB.exe
─────────────────                   ─────────────────────
Handler 主程式                  ←→  GPIB / RS232 通訊程式
（HT9045 / HT9046 機台控制）        （轉接 GPIB 指令 ↔ Handler IPC）
        │                                   │
        │  WM_COPYDATA (MV 結構)            │
        │ ─────────────────────────────→   │  接收 Handler 狀態 / 命令
        │                                   │
        │  WM_COPYDATA (VM 結構)            │
        │ ←─────────────────────────────   │  回傳 BIN 結果 / GPIB 結果
```

### 執行檔
| 程式 | 說明 |
|------|------|
| `H9046_32GPIB.exe` | 主要 GPIB 程式，處理 GPIB / RS232 與 Handler 之間的通訊橋接 |
| `HandlerSys.exe` | Handler 主程式，負責機台控制 |

### 主要原始檔
| 檔案 | 說明 |
|------|------|
| `Main.cpp` / `Main.h` | GPIB 主視窗 `TSerialPoll`，GPIB 核心流程 |
| `MessageDef.h` / `.cpp` | MSG_CMD 常數定義、VM/MV 結構定義 |
| `RS232.cpp` / `RS232.h` | RS232 串列通訊模組 |
| `cmydef.h` | 全域常數（TOTAL_SITE=32、ALIAS="HT9045"、客戶代碼等）|
| `type_def.h` | 基本型別定義（U8/I16/U16/I32/U32/F32/F64/Boolean）|
| `MyDutPanel.cpp` | DUT 狀態顯示面板 |

### 常數定義
- `TOTAL_SITE = 32`：最大 Site 數
- `MAX_SITE_COUNT = 32`
- `StrLength = 2560`

---

## 2. 資料結構說明

> **警告**：VM 與 MV 結構不可任意新增成員（至少須加在最末端），
> 且任何更動必須 **Handler、GPIB、RS232 三隻程式同步修改**並同步更版號。
> 結構內不可使用 VCL 元件，只能使用純 C 型別。

### VM 結構（GPIB → Handler）

`H9046_32GPIB.exe` 傳給 `HandlerSys.exe` 使用的結構，
以 `WM_COPYDATA` 的 `lpData = &GGpib2Handler.iCommand` 方式傳送。

```cpp
typedef struct
{
    unsigned int iCommand;      // 功能代碼（MSG_CMD_*）
    unsigned int Result[32];    // 各 Site 的 BIN 結果，或 ECHO 字串（'E','C','H','O'）
    bool bError;                // 是否發生錯誤
    bool bEchoStop;             // ECHO 停止旗標（ChungHung 20130326）
    char cReturn[256];          // 回傳字串資料（例：版本號、機台 ID）
    char GpibStatus[32];        // GPIB 狀態字串
    char GpibData[256];         // GPIB 原始資料
    int  GPIBBin;               // GPIB Bin 數量 / 簡單資料傳輸（Steven 20151219 已改作簡單傳輸）
    bool bOneCycle;             // Tester Low Yield 要求 One Cycle（jou 2014-09-23）
} VM;
```

**全域變數**：
- `GGpib2Handler`（GPIB 端使用）
- `HGpib2Handler`（Handler 端接收，為指標）

---

### MV 結構（Handler → GPIB）

`HandlerSys.exe` 傳給 `H9046_32GPIB.exe` 使用的結構，
以 `WM_COPYDATA` 的 `lpData = &HHandler2Gpib.iSendCommand` 方式傳送。

```cpp
typedef struct
{
    unsigned int iSendCommand;      // 功能代碼（MSG_CMD_*）
    unsigned int Site[32];          // 各 Site 狀態 / 設定值
    bool bSimulate;                 // 離線模擬模式（LastSet.iTester == OFF_LINE）
    bool bSupport32Bin;             // 支援 32 Bin（TTL 模式為 false）
    bool bCloseGpib;                // 是否關閉 GPIB
    bool bTimeOutProcess;           // Test Timeout 處理旗標
    int  GpibAddress;               // GPIB 模式：GPIB 位址；RS232 模式：MaxBinCount
    bool MachineISRun;              // 機台是否在運行（SystemStart）
    bool IsTest;                    // 是否正在測試（終止測試時為 false）
    bool bGpibMode;                 // LastSet.iTester==ON_LINE && TestIF.iTestType==GPIB_MODE
    int  iLotStatus;                // OLP 聚成專用 Lot 狀態
    HWND HandlerHwnd;               // Handler 視窗 Handle（回傳用）
    HWND GpibHwnd;                  // GPIB 視窗 Handle
    int  GPIBBin;                   // GPIB Bin 模式數量（iTestBinCount）
    int  iStatus[17];               // 機台狀態陣列（JerryYang 20151109，力成）
    char Message[2048];             // Barcode / OCR 資料字串（Steven 20150713）
    char UseSiteMapData[256];       // Site Mapping 資料（Ifor 20201030）
    char asATC_TYPE[32];            // ATC TYPE 字串（Ifor 20230828）
    char MultiMessage[4096];        // 多 Site 2D Code 資料（Ifor 20241003）
} MV;
```

**全域變數**：
- `HHandler2Gpib`（Handler 端使用）
- `GHandler2Gpib`（GPIB 端接收，為指標）

---

## 3. 功能代碼說明（MSG_CMD）

### 3.1 基礎 / 測試流程類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 0 | `MSG_CMD_NONE` | 雙向 | 開始或終止測試；BIN 結果回傳 |
| 9 | `MSG_CMD_ECHONG` | H→G | 資料錯誤通知（Steven 20250915）|
| 10 | `MSG_CMD_DoubleContact` | H→G | Double Contact 動作 |
| 20 | `MSG_CMD_ContactTestAbort` | G→H | 紀錄 Contact Test 動作結束，取消測試狀態 |
| 33 | `MSG_CMD_BarCodeFlowErr` | G→H | 測試機未詢問 BARCODE? 通知 Handler |
| 42 | `MSG_CMD_BarcodeOFF` | G→H | 測試機詢問 BARCODE? 但 Handler 未開啟 Barcode 功能 |
| 51 | `MSG_CMD_TesterMode` | H→G | 送出 GPIB 模式資訊（GPIBBin = TestIF_File.iGpibMode）|
| 54 | `MSG_CMD_Auto_Clean` | H→G | Auto Clean 動作（wei 20180309）|
| 55 | `MSG_CMD_Pause` | H→G | Pause 動作（wei 20180309）|
| 96 | `MSG_CMD_ESC` | H→G | Qorvo Empty Socket Check 觸發（Steven 20201022）|
| 97 | `MSG_CMD_RESUME` | H→G | Qorvo RESUME 指令（Steven 20201022）|
| 98 | `MSG_CMD_TestAlarm` | H→G | Qorvo 測試警報（Steven 20201022）|

#### MSG_CMD_NONE（0）詳細說明

**H → G（Handler 送往 GPIB）**：

| 欄位 | 對應值 | 說明 |
|------|--------|------|
| `iSendCommand` | `MSG_CMD_NONE (0)` | 通知 GPIB 開始 / 終止測試 |
| `Site[32]` | 有要測試的 Site bits | |
| `iLotStatus` | OLP 聚成專用 | |
| `bSimulate` | `LastSet.iTester==OFF_LINE` | 離線模擬 |
| `bCloseGpib` | false | |
| `bTimeOutProcess` | `TestISTimeOut` | Test Timeout 標誌 |
| `bSupport32Bin` | TTL 模式為 false | |
| `GpibAddress` | GPIB 位址（GPIB 模式）/ MaxBinCount（RS232 模式）| |
| `MachineISRun` | `SystemStart` | |
| `IsTest` | 終止測試時為 false | |
| `bGpibMode` | `LastSet.iTester==ON_LINE && iTestType==GPIB_MODE` | |
| `GPIBBin` | `iTestBinCount` | |
| `Message[2048]` | Barcode / OCR 資料 | |

使用函式：`RunTestProgram(true, flag2);`（函式一）

**G → H（GPIB 送往 Handler）**：

| 欄位 | 對應值 | 說明 |
|------|--------|------|
| `iCommand` | `MSG_CMD_NONE (0)` | 確認可進行分 BIN |
| `Result[32]` | 各 Site 的 BIN 別 | |

使用函式：`SendCaptureFinish();`（函式六）+ `SendECHO();`（函式七）

---

### 3.2 機台動作狀態類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 14 | `MSG_CMD_HandlerHomeStart` | H→G | 通知 GPIB 機台開始歸零 |
| 15 | `MSG_CMD_HandlerHomeFinish` | H→G | 通知 GPIB 機台歸零完成 |
| 16 | `MSG_CMD_Arm1Down` | H→G | 通知 GPIB 目前下壓手臂為 Arm 1 |
| 17 | `MSG_CMD_Arm2Down` | H→G | 通知 GPIB 目前下壓手臂為 Arm 2 |
| 18 | `MSG_CMD_ContactTestArm1` | H→G | 通知 GPIB Arm 1 在執行 Contact Test |
| 19 | `MSG_CMD_ContactTestArm2` | H→G | 通知 GPIB Arm 2 在執行 Contact Test |
| 11 | `MSG_CMD_TimeOutSkip` | H→G | Test Timeout 後選擇 SKIP |
| 12 | `MSG_CMD_TimeOutRetryWait` | H→G | Test Timeout 後選擇 RETRY（等待結果）|
| 13 | `MSG_CMD_TimeOutRetrySend` | H→G | Test Timeout 後選擇 RETRY（重送 SOT）|

#### 個別說明

**MSG_CMD_Arm1Down (16)** — 函式二：`SendMSG_CMD(MSG_CMD_Arm1Down)`  
條件：`LastSet.iTester != OFF_LINE`；ART 模式：`TestIF_File.bSCKART_RunARTWithoutCmd == false`

**MSG_CMD_Arm2Down (17)** — 函式二：`SendMSG_CMD(MSG_CMD_Arm2Down)`

**MSG_CMD_ContactTestArm1 (18)** — 函式二：`SendMSG_CMD(MSG_CMD_ContactTestArm1)`

**MSG_CMD_ContactTestArm2 (19)** — 函式二：`SendMSG_CMD(MSG_CMD_ContactTestArm2)`

**MSG_CMD_TimeOutSkip (11)** — 函式二：`SendMSG_CMD(MSG_CMD_TimeOutSkip)`

**MSG_CMD_TimeOutRetryWait (12)** — 函式二：`SendMSG_CMD(MSG_CMD_TimeOutRetryWait)`

**MSG_CMD_TimeOutRetrySend (13)** — 函式二：`SendMSG_CMD(MSG_CMD_TimeOutRetrySend)`  
條件：`LastSet.iTester != OFF_LINE`；ART 模式：`TestIF_File.bSCKART_RunARTWithoutCmd == false`

**MSG_CMD_HandlerHomeStart (14)** — 函式二：`SendMSG_CMD(MSG_CMD_HandlerHomeStart)`

**MSG_CMD_HandlerHomeFinish (15)** — 函式二：`SendMSG_CMD(MSG_CMD_HandlerHomeFinish)`

---

### 3.3 取得機台狀態類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 71 | `MSG_CMD_GetTestArmPos` | 雙向 | 取得 Test Arm 位置 |
| 72 | `MSG_CMD_GetTestArmEP` | 雙向 | 取得 Test Arm EP 值 |
| 83 | `MSG_CMD_GetSiteOnOff` | 雙向 | 取得各 Site 啟用狀態 |
| 84 | `MSG_CMD_GetNumOfSites` | 雙向 | 取得 Site 總數 |
| 69 | `MSG_CMD_GetNowAllTemp` | 雙向 | 取得所有溫度當前值 |
| 85 | `MSG_CMD_DeviceMapSRQ` | H→G | Device Map SRQ 狀態（函式四）|
| 34 | `MSG_CMD_MachineState` | — | 機台狀態（僅 RS232 通訊使用）|

---

### 3.4 控制 GPIB 程式類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 21 | `MSG_CMD_CloseGpib` | H→G | 關閉 GPIB 連線（wei 20150408）|
| 23 | `MSG_CMD_ChangeGpib` | H→G | 切換 GPIB 連線（wei 20150409）|
| 31 | `MSG_CMD_EnableBarCode` | H→G | 啟用 2D Barcode 功能（Steven 20150713）|
| 32 | `MSG_CMD_DisableBarCode` | H→G | 停用 2D Barcode 功能 |
| 24 | `MSG_CMD_Version` | G→H | 通知 Handler 目前 GPIB 版本 |
| 62 | `MSG_CMD_HandlerID` | 雙向 | 詢問或回覆機台 ID |

#### MSG_CMD_Version (24)
- **G → H**：通知 Handler GPIB 版本
  - `cReturn[256]` = `GPIBVersion`（版本字串）
  - 函式九：`SendMSG_CMD(MSG_CMD_Version, GPIBVersion);`

#### MSG_CMD_HandlerID (62)
- **G → H**（詢問）：函式八：`SendMSG_CMD(MSG_CMD_HandlerID);`
- **H → G**（回覆）：`cReturn[256]` = MachineID；函式三：`SendMSG_CMD(MSG_CMD_Version, MachineID);`

---

### 3.5 設定機台參數類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 73 | `MSG_CMD_SetTemp` | G→H | 設定測試溫度 |
| 74 | `MSG_CMD_SetSoakTime` | G→H | 設定 Soak Time |
| 75 | `MSG_CMD_SetTJ` | G→H | 設定 TJ 值 |
| 76 | `MSG_CMD_SetSiteMapData` | G→H | 設定 Site Map 資料 |
| 77 | `MSG_CMD_SetAlarmSetup` | G→H | 設定警報參數 |
| 35 | `MSG_CMD_OverDrive` | G→H | OverDrive 設定（TSMC，Steven 20151207）|
| 36 | `MSG_CMD_ReContact` | G→H | ReContact 設定（TSMC，Steven 20151207）|
| 60 | `MSG_CMD_Assign` | G→H | 指派設定 |
| 61 | `MSG_CMD_StartMode` | G→H | 設定 Start Mode |
| 63 | `MSG_CMD_HandlerSiteMap` | G→H | Handler Site Map |
| 64 | `MSG_CMD_HandlerSoakTime` | G→H | Handler Soak Time |
| 65 | `MSG_CMD_HandlerTemperature` | G→H | Handler 溫度設定 |
| 66 | `MSG_CMD_Force` | G→H | Contact Force 設定 |
| 67 | `MSG_CMD_BinMap` | G→H | BIN Map 設定 |

---

### 3.6 ART 專用類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 22 | `MSG_CMD_LotStatus` | H→G | Lot 狀態通知（wei 20150409）|
| 28 | `MSG_CMD_RetestFlag` | H→G | Auto Retest 旗標（jou 2015-09-21）|
| 43 | `MSG_CMD_SCKART_LOTCLEAR` | G→H | SCK ART Lot 清除（Steven 20161025）|
| 44 | `MSG_CMD_SCKART_LOTRTCLEAR` | G→H | SCK ART Lot Retest 清除 |
| 45 | `MSG_CMD_SCKART_INPUTQTY` | G→H | SCK ART 輸入數量 |
| 46 | `MSG_CMD_SCKART_LOTSTATUS` | G→H | SCK ART Lot 狀態 |
| 47 | `MSG_CMD_SCKART_Alarm` | G→H | SCK ART 警報 |
| 48 | `MSG_CMD_SCKART_QTY` | G→H | SCK ART 數量查詢 |
| 49 | `MSG_CMD_SCKART_INITIAL` | G→H | SCK ART 初始化 |
| 50 | `MSG_CMD_SCKART_SRQMASK` | G→H | SCK ART SRQ Mask 設定 |
| 53 | `MSG_CMD_SCKART_RunDummy` | H→G | SCK ART 執行 Dummy 動作（Steven 20180824）|
| 25 | `MSG_CMD_RCMD` | 雙向 | SECS/GEM Remote Command（jou 2015-09-21）|
| 26 | `MSG_CMD_SVID` | 雙向 | SECS/GEM SVID 查詢（jou 2015-09-21）|
| 27 | `MSG_CMD_ECID` | 雙向 | SECS/GEM ECID 設定（jou 2015-09-21）|

---

### 3.7 2D Barcode / 2DID 類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 93 | `MSG_CMD_2DIDFormat` | G→H | 2DID 格式設定（JerryYang 20200422）|
| 86 | `MSG_CMD_PickLoad` | H→G | Pick from Loader（GPIB 流程）|
| 87 | `MSG_CMD_PlaceLoad` | H→G | Place to Loader（GPIB 流程）|
| 88 | `MSG_CMD_TrayFeed` | H→G | Tray Feed 動作 |
| 165 | `MSG_CMD_READYNEXTSHOT` | H→G | 要求下一個 2DID 資訊（Jimmychiu 20231011，HT9046LS）|
| 166 | `MSG_CMD_NEXT2DID` | G→H | 回傳下一個 2DID 資訊 |

---

### 3.8 溫控相關類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 56 | `MSG_CMD_TempArm` | 雙向 | 溫控手臂狀態（Steven 20190326，GPIB V9.0）|
| 57 | `MSG_CMD_TestArm` | 雙向 | 測試手臂狀態 |
| 58 | `MSG_CMD_ContactForce` | 雙向 | Contact Force 值 |
| 59 | `MSG_CMD_ActualTemp` | 雙向 | 實際溫度 |
| 68 | `MSG_CMD_TestMode` | 雙向 | 測試模式 |
| 99 | `MSG_CMD_POWERFOLLOWING` | 雙向 | Power Following 功能 |
| 132 | `MSG_CMD_SetTestTemp` | G→H | 測試溫度切換（Ifor 20210623）|

---

### 3.9 SIGURD / Novatek 客製類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 133 | `MSG_CMD_SIGURD_CHKSTATUS` | 雙向 | SIGURD `CHKSTATUS?` 指令（KaiChen 20180910）|
| 134 | `MSG_CMD_ONECYCLE` | H→G | SIGURD `ONECYCLE` 指令 |
| 135 | `MSG_CMD_ECHOOK_ONECYCLE` | G→H | SIGURD `ECHOOK:ONECYCLE`（KaiChen 20181114）|
| 136 | `MSG_CMD_GETBINCATEGORY` | 雙向 | SIGURD `GETBINCATEGORY?`（KaiChen 20180913）|
| 137 | `MSG_CMD_SETUPFILENAME` | 雙向 | SIGURD `GETSETUPFILENAME?`（KaiChen 20181022）|
| 138 | `MSG_CMD_SIGURD_HANDLERID` | 雙向 | SIGURD `HANDLERID?`（KaiChen 20200507）|
| 139 | `MSG_CMD_SGSETUP` | G→H | SIGURD `SGSETUP_`（KaiChen 20190613）|
| 140 | `MSG_CMD_SETSTARTMODE` | G→H | SIGURD `SetStartMode_`（KaiChen 20180910）|
| 141 | `MSG_CMD_CHECKLIST` | 雙向 | SIGURD `CHECKLIST?`（KaiChen 20190613）|
| 142 | `MSG_CMD_BINPOS` | G→H | SIGURD `BINPOS_`（KaiChen 20190706）|
| 143 | `MSG_CMD_GetSGFTP_STATUS` | 雙向 | SIGURD `SGFTP_STATUS`（Sam 20210329）|
| 144 | `MSG_CMD_SetSGFTP` | G→H | SIGURD `SGFTP_ON / SGFTP_OFF`（Sam 20210329）|
| 145 | `MSG_CMD_SetNONDOUBLEBIN` | G→H | SIGURD `NONDOUBLEBIN_`（Sam 20210329）|
| 146 | `MSG_CMD_SetBINCOUNT` | G→H | SIGURD `BINCOUNT_`（Sam 20210329）|
| 147 | `MSG_CMD_SetSGOSBIN` | G→H | SIGURD `SGOSBIN_`（Sam 20210406）|
| 148 | `MSG_CMD_SetSGCONTFAIL` | G→H | SIGURD `SGCONTFAIL_`（Sam 20210422）|
| 149 | `MSG_CMD_SETTESTERID` | G→H | SIGURD `SETTESTERID`（Sam 20210617）|
| 150 | `MSG_CMD_GETTESTERID` | 雙向 | SIGURD `GETTESTERID`（Sam 20210617）|
| 156 | `MSG_CMD_GETSHUTTLEMODE` | 雙向 | SIGURD `GETSHUTTLEMODE?`（Sam 20230130）|
| 157 | `MSG_CMD_SETMAXTEST` | G→H | SIGURD `SETMAXTEST_`（Sam 20230201）|
| 158 | `MSG_CMD_GETMAXTEST` | 雙向 | SIGURD `GETMAXTEST`（Sam 20230201）|
| 159 | `MSG_CMD_SETINITIALMAXTEST` | G→H | SIGURD `SETINITIALMAXTEST_`（Sam 20230201）|
| 160 | `MSG_CMD_GETINITIALMAXTEST` | 雙向 | SIGURD `GETINITIALMAXTEST`（Sam 20230201）|
| 173 | `MSG_CMD_SETAICCD` | G→H | SIGURD `SETAICCD_`（Sam 20231108）|
| 179 | `MSG_CMD_SETOSBIN` | G→H | SIGURD `SETOSBIN_`（Sam 20250115）|
| 180 | `MSG_CMD_GETOSBIN` | 雙向 | SIGURD `GETOSBIN?`（Sam 20250115）|
| 152 | `MSG_CMD_GetAutClean` | 雙向 | Novatek `AUTOCLEAN?`（Sam 20220408）|
| 153 | `MSG_CMD_ForcePerPinN` | 雙向 | Novatek `DEVICEFORCEPERPIN?`（Sam 20220408）|
| 154 | `MSG_CMD_ContactHeight` | 雙向 | Novatek `ARMCONTACTHIGHVALUE?`（Sam 20220408）|
| 155 | `MSG_CMD_YieldContinusFail` | 雙向 | Novatek `YIELDCONTINUESFAIL?`（Sam 20220408）|

---

### 3.10 其他 / 新增類

| 值 | 常數名稱 | 方向 | 說明 |
|----|---------|------|------|
| 52 | `MSG_CMD_State_Record` | H→G | 狀態記錄（wei 20170911）|
| 89 | `MSG_CMD_SamSung_Tmp` | H→G | 三星格式：溫度（Steven 20191112）|
| 90 | `MSG_CMD_SamSung_Map` | H→G | 三星格式：Site Map |
| 91 | `MSG_CMD_SamSung_Soak` | H→G | 三星格式：Soak Time |
| 92 | `MSG_CMD_AMDRS232Connect` | G→H | AMD RS232 連線錯誤警報（Ifor 20200220）|
| 94 | `MSG_CMD_State_TTL` | H→G | TTL RS232 通訊狀態（Isaac 20200903）|
| 95 | `MSG_CMD_Command_TTL` | 雙向 | TTL RS232 通訊指令 |
| 151 | `MSG_CMD_SBIN` | 雙向 | Amlogic SBIN 接收（Steven 20220120）|
| 161 | `MSG_CMD_PPSELECT` | G→H | UTAC PP_SELECT 讀檔（Richard 20220929）|
| 162 | `MSG_CMD_ASKPPSELECT` | G→H | UTAC 詢問 Handler 當前 Setup 檔名 |
| 163 | `MSG_CMD_SetBinMap` | G→H | 設定 BIN Map（Steven 20230210）|
| 174 | `MSG_CMD_ASIF_TJ_EFUSED` | 雙向 | MTK ASIF TJ Efused 資料（Steven 20240903）|
| 175 | `MSG_CMD_ASIF_TJ_REQUEST` | 雙向 | MTK ASIF TJ Request |
| 176 | `MSG_CMD_ASIF_TJ_FB` | 雙向 | MTK ASIF TJ Feedback |
| 177 | `MSG_CMD_GETAICCD` | 雙向 | SIGURD `GETAICCD?`（Sam 20240826）|
| 178 | `MSG_CMD_QRA` | H→G | Qorvo ART 啟用確認（Steven 20241004）|
| 181 | `MSG_CMD_DUTCHK` | 雙向 | DOOSAN TESNA DUT Check（Steven 20250701）|
| 182 | `MSG_CMD_GetFFC` | 雙向 | Ampere FFC 取得（Steven 20250701）|

---

### 3.11 已棄用 / 僅 RS232 使用

| 值 | 常數名稱 | 備註 |
|----|---------|------|
| 1 | `MSG_CMD_CatalystSimpleGPIB` | 無使用 |
| 2 | `MSG_CMD_SwitchArm` | RS232 通訊才使用（要求交換 Index Arm）|
| 3 | `MSG_CMD_SwitchArmOK` | RS232 通訊才使用（Index Arm 交換完成）|
| 4 | `MSG_CMD_AbortTest` | 無使用 |
| 5 | `MSG_CMD_AskArmTestMode` | RS232 通訊才使用 |
| 6 | `MSG_CMD_2ArmTestMode` | RS232 通訊才使用 |
| 7 | `MSG_CMD_1ArmTestMode` | RS232 通訊才使用 |
| 8 | `MSG_CMD_NoFullSiteRespon` | 取消使用（FullSite Test Timeout，Steven 20141016）|
| 29 | `MSG_CMD_CEIDON` | 功能未完成（Steven 20150901）|
| 30 | `MSG_CMD_CEIDOFF` | 功能未完成 |
| 37 | `MSG_CMD_TesterBin` | RS232 才使用（Maxim Philippine）|
| 38 | `MSG_CMD_SoakTime` | RS232 才使用（Maxim Philippine）|
| 39 | `MSG_CMD_JamCode` | RS232 才使用（Maxim Philippine）|
| 40 | `MSG_CMD_SiteMap` | RS232 才使用（Maxim Philippine）|
| 34 | `MSG_CMD_MachineState` | RS232 才使用（JerryYang 20151109，力成）|
| 78 | `MSG_CMD_EnableAMDFunction` | V3.30.649（GPIB V12.03）以後不使用 |
| 79 | `MSG_CMD_DisableAMDFunction` | V3.30.649（GPIB V12.03）以後不使用 |

---

## 4. 使用函式說明

以下函式定義在 Handler 主程式（`HandlerSys.exe` 的 `TfMain` 類）與 GPIB 程式（`TSerialPoll` 類）中。

### 函式一：\[H → G\] RunTestProgram(true, flag2)

Handler 啟動測試時呼叫，透過 WM_COPYDATA 傳送 `MV` 結構至 GPIB，
`iSendCommand = MSG_CMD_NONE`，`Site[32]` 填入有效 Site 旗標。

---

### 函式二：\[H → G\] SendMSG_CMD(int CMD)

```cpp
void TfMain::SendMSG_CMD(int CMD)
{
    if(fMain->bFind == false)
        return;
    HHandler2Gpib.iSendCommand = CMD;
    HHandler2Gpib.bCloseGpib  = false;
    HHandler2Gpib.HandlerHwnd  = this->Handle;
    HHandler2Gpib.GpibHwnd     = HVisionWnd;

    // 特殊處理：SCKART_RunDummy
    if(CMD == (int)MSG_CMD_SCKART_RunDummy)
    {
        if(LastSet.iTester == OFF_LINE)
            HHandler2Gpib.bSimulate = true;
        else if(CosFunction.bUseSCKART && IniConfig.bA10_AutoReTest &&
                TestIF_File.bSCKART_EnableART && TestIF_File.bSCKART_RunARTWithoutCmd)
            HHandler2Gpib.bSimulate = TestIF_File.bSCKART_RunARTWithoutCmd;
        else
            HHandler2Gpib.bSimulate = false;
    }

    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(HHandler2Gpib);
    pcp->lpData  = (unsigned char *)&HHandler2Gpib.iSendCommand;
    SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：傳送僅含代碼、無附加資料的 Handler→GPIB 指令。

---

### 函式三：\[H → G\] SendMSG_CMD(int CMD, AnsiString Message)

```cpp
void TfMain::SendMSG_CMD(int CMD, AnsiString Message)
{
    HHandler2Gpib.iSendCommand = CMD;
    memset(HHandler2Gpib.Message, '\0', sizeof(HHandler2Gpib.Message));
    strncpy(HHandler2Gpib.Message, Message.c_str(), Message.Length());
    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(HHandler2Gpib);
    pcp->lpData  = (unsigned char *)&HHandler2Gpib.iSendCommand;
    SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：傳送含字串附加資料（例：機台 ID、Setup 檔名）的 Handler→GPIB 指令。
填入 `HHandler2Gpib.Message[2048]`。

---

### 函式四：\[H → G\] SendMSG_CMD_DeviceMapSRQ(int iStatus)

```cpp
void TfMain::SendMSG_CMD_DeviceMapSRQ(int iStatus)
{
    HHandler2Gpib.iSendCommand = MSG_CMD_DeviceMapSRQ;
    HHandler2Gpib.iLotStatus   = iStatus;
    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(HHandler2Gpib);
    pcp->lpData  = (unsigned char *)&HHandler2Gpib.iSendCommand;
    SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：傳送 Device Map SRQ 狀態，附帶 `iLotStatus` 整數值。

---

### 函式五：\[H → G\] SendMSG_TestMode()

```cpp
void TfMain::SendMSG_TestMode()
{
    if(fMain->bFind == false)
        return;
    HHandler2Gpib.iSendCommand = MSG_CMD_TesterMode;
    HHandler2Gpib.bCloseGpib   = false;
    HHandler2Gpib.HandlerHwnd  = this->Handle;
    HHandler2Gpib.GPIBBin      = TestIF_File.iGpibMode;    // 傳送 GPIB 模式
    HHandler2Gpib.GpibHwnd     = HVisionWnd;
    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(HHandler2Gpib);
    pcp->lpData  = (unsigned char *)&HHandler2Gpib.iSendCommand;
    SendMessage(fMain->HVisionWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：Handler 通知 GPIB 目前測試模式，`GPIBBin` = `TestIF_File.iGpibMode`。

---

### 函式六：\[H ← G\] SendCaptureFinish()

```cpp
void __fastcall TSerialPoll::SendCaptureFinish()
{
    if(bFind)
    {
        GGpib2Handler.iCommand = MSG_CMD_NONE;              // 分 BIN 完成通知
        COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
        pcp->dwData  = 0;
        pcp->cbData  = sizeof(GGpib2Handler);
        pcp->lpData  = (unsigned char *)&GGpib2Handler.iCommand;
        SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
        // 記錄 LOG：Result[31..0] 各 Site BIN 值
        delete pcp;
    }
    iGPIPWaitCount = 17;
}
```

**用途**：GPIB 程式通知 Handler，BIN 分類完成（`iCommand = MSG_CMD_NONE`，`Result[32]` = 各 Site BIN 值）。

---

### 函式七：\[H ← G\] SendECHO

```cpp
// 傳送 ECHO 響應給 Handler
GGpib2Handler.iCommand   = MSG_CMD_NONE;
GGpib2Handler.Result[0]  = 'E';
GGpib2Handler.Result[1]  = 'C';
GGpib2Handler.Result[2]  = 'H';
GGpib2Handler.Result[3]  = 'O';
COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
pcp->dwData  = 0;
pcp->cbData  = sizeof(GGpib2Handler);
pcp->lpData  = (unsigned char *)&GGpib2Handler.iCommand;
SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
```

**用途**：GPIB 程式告知 Handler，已收到 BINON 並送出 ECHO 回應。
`Result[0..3]` = `'E','C','H','O'`，`iCommand = MSG_CMD_NONE`。

---

### 函式八：\[H ← G\] SendMSG_CMD(int CMD)

```cpp
void TSerialPoll::SendMSG_CMD(int CMD)
{
    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(GGpib2Handler);
    GGpib2Handler.iCommand = CMD;
    pcp->lpData  = (unsigned char *)&GGpib2Handler.iCommand;

    // LOG：若 CMD >= slCmdList->Count，輸出數字；否則輸出名稱字串
    SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：GPIB 程式傳送僅含代碼、無附加資料的 GPIB→Handler 通知。

---

### 函式九：\[H ← G\] SendMSG_CMD(int CMD, AnsiString Message)

```cpp
void TSerialPoll::SendMSG_CMD(int CMD, AnsiString Message)
{
    COPYDATASTRUCT *pcp = new COPYDATASTRUCT;
    pcp->dwData  = 0;
    pcp->cbData  = sizeof(GGpib2Handler);
    GGpib2Handler.iCommand = CMD;
    strncpy(GGpib2Handler.cReturn, Message, 544);   // 注意：cReturn 最大 256、需確認邊界
    pcp->lpData  = (unsigned char *)&GGpib2Handler.iCommand;
    SendMessage(HMountWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)pcp);
    delete pcp;
}
```

**用途**：GPIB 程式傳送含字串的 GPIB→Handler 通知，例如：版本號、機台 ID。
字串存入 `GGpib2Handler.cReturn[256]`。

---

## 5. 通訊機制說明

### WM_COPYDATA 傳送流程

```
傳送端                                  接收端
─────                                   ─────
COPYDATASTRUCT pcp;
pcp.dwData = 0;
pcp.cbData = sizeof(MV or VM);
pcp.lpData = &結構.iSendCommand 或 .iCommand;
SendMessage(目標視窗 HWND, WM_COPYDATA, NULL, &pcp)
                                        → OnMessage → WM_COPYDATA handler
                                        → 複製 pcp.lpData 到本地結構
```

### Handler 視窗尋找

`bFind`：Handler 程式啟動後，GPIB 程式透過 `FindWindow(ALIAS, NULL)` 等方式取得
Handler 主視窗 Handle，儲存於 `HVisionWnd / HMountWnd`。

### 重要注意事項

1. `sizeof(MV)` 和 `sizeof(VM)` 必須在 Handler、GPIB、RS232 三端保持完全一致
2. 結構成員**只可新增於末端**，不可插入中間
3. `cReturn[256]` 與 函式九的 `strncpy(..., 544)` 存在潛在越界，實際使用時需注意

---

## 6. eTestMode 測試模式定義

```cpp
enum eTestMode {
    SingleSite   = 0,   // 1 Site
    DualSite     = 1,   // 2 Site（1x2）
    TriSite1X3   = 2,   // 3 Site（1x3，TSMC，Frank 20160104）
    QualSite1X4  = 3,   // 4 Site（1x4）
    DualSite2x1  = 4,   // 2 Site（2x1）
    QualSite2X2  = 5,   // 4 Site（2x2）
    QualSite2X2N = 6,   // 4 Site（2x2NN，Frank 20200520）
    _6Site2X3    = 7,   // 6 Site（2x3，ChungHung 20140115）
    _6Site2X3N   = 8,   // 6 Site（2x3NN，Steven 20220425）
    _8Site2X4    = 9,   // 8 Site（2x4）
    _10Site2X5   = 10,  // 10 Site（wei 20190614）
    _12Site2X6   = 11,  // 12 Site（2x6）
    _16Site2X8   = 12,  // 16 Site（2x8）
    _16Site4X4   = 13,  // 16 Site（4x4，Sam 20190226）
    _32Site4X8N  = 14,  // 32 Site（雙 Arm，4x8N）
};
```

---

## 7. 版本說明

| 版本 | 說明 |
|------|------|
| V12.04 | 原始手冊版本（本文件基礎）|
| V12.13.884.0 | 目前程式版本（`MessageDef.cpp`）|

### 重要里程碑（從 MSG_CMD 加入時間判斷）
| 時間 | 負責人 | 主要新增 |
|------|--------|---------|
| 2014 | Steven | MSG_CMD_NoFullSiteRespon、bEchoStop |
| 2015 | wei, jou | MSG_CMD_CloseGpib/ChangeGpib/Version/RCMD/SVID/ECID/EnableBarCode/OverDrive/ReContact |
| 2016 | JerryYang, Steven, wei | MSG_CMD_SCKART 系列（ART 功能）|
| 2017 | wei | MSG_CMD_State_Record |
| 2018–2019 | KaiChen, Steven | MSG_CMD_SIGURD_* 系列，GPIB V9.0 新增指令 |
| 2020 | Steven, Isaac, JerryYang | MSG_CMD_ESC/RESUME（Qorvo），TTL RS232，2DIDFormat |
| 2021–2022 | Sam, Ifor, Richard | SIGURD 擴充指令集，PPSELECT，SBIN |
| 2023–2024 | Sam, Jimmy, Steven | READYNEXTSHOT/NEXT2DID，SETAICCD，ASIF_TJ |
| 2025 | Sam, Steven, Ifor | SETOSBIN，DUTCHK，GetFFC，Socket/TIM Counter |

---

*本文件由 GitHub Copilot 依 `GPIB_Program Manual_V12.04.doc` 及 `GPIB_Code_32Site_V12.13.900.0_20260331` 程式碼自動轉換與補充生成。*  
*生成日期：2026-04-01*
