# SECS 程式開發指南

> 作者：Chrischen   
> 更新日期：2026-03-27  
> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260323`

## 目錄

- [概述](#概述)
- [SEMI 標準層級架構](#semi-標準層級架構)
- [核心術語與識別碼定義](#核心術語與識別碼定義)
- [HSMS 通訊層協議](#hsms-通訊層協議-semi-e37)
- [SECS-II 訊息格式](#secs-ii-訊息格式-semi-e5)
- [Stream & Function 指令總表](#stream--function-指令總表)
- [GEM 設備狀態模型](#gem-設備狀態模型-semi-e30)
- [資料型別](#資料型別)
- [HT9045 實作](#ht9045-實作)
  - [功能支援總覽](#功能支援總覽)
  - [功能支援詳細說明](#功能支援詳細說明)
    - [Recipe 管理](#2-recipe-管理-s7fx)
    - [時間同步](#9-時間同步-s2f31f32)
    - [Carrier 管理與 Port 事件](#11-carrier-管理與-port-事件)
    - [Material ID/Count 報告](#12-material-idcount-報告)
    - [SEMI E84 介面交握](#14-semi-e84-介面交握)
    - [GEM300 Auto Run](#10-gem300-auto-run-scenario)
  - [Stream/Function 實作詳細](#streamfunction-實作詳細)
  - [資料型態對照](#資料型態對照)
  - [關鍵整合點](#關鍵整合點)
  - [主動上報時機點](#主動上報時機點)
- [SEMI E30 Remote Control 合規性分析](#semi-e30-remote-control-合規性分析)
- [參考資料](#參考資料)

---

## 概述

本文件說明 SECS (SEMI Equipment Communications Standard) 程式開發的基本概念與實作方式。SECS/GEM 是半導體業界標準的設備通訊協定，用於主機 (Host) 與設備 (Equipment) 之間的數據交換與控制。

---

## SEMI 標準層級架構

```
+-------------------+
|   SEMI E30 (GEM)  |  ← 設備行為模型與狀態機
+-------------------+
|   SEMI E5 (SECS-II)|  ← 訊息語法與語義 (SxFy)
+-------------------+
|   SEMI E37 (HSMS) |  ← TCP/IP 通訊協定
+-------------------+
|   TCP/IP          |  ← 網路層
+-------------------+
```

### 標準說明

| 標準 | 名稱 | 用途 |
|------|------|------|
| SEMI E30 | GEM (Generic Model) | 定義設備行為模型、狀態機、配方管理、數據採集機制 |
| SEMI E5 | SECS-II | 定義訊息的語法與語義，採用 SxFy 結構 |
| SEMI E37 | HSMS | 基於 TCP/IP 的高速通訊協定，取代舊式 RS-232 (SECS-I) |

### HSMS vs SECS-I

| 項目 | HSMS (E37) | SECS-I (E4) |
|------|------------|-------------|
| 傳輸介質 | TCP/IP | RS-232 |
| 速度 | 高速 | 慢 (9600 bps) |
| 連線數 | 多連線 | 單一連線 |
| 現代化 | 主流標準 | 舊式 (淘汰中) |

---

## 核心術語與識別碼定義

在 SECS/GEM 通訊中，識別碼用於定義數據的邏輯歸類：

| 識別碼 | 全稱 | 說明 |
|:------:|------|------|
| **VID** | Variable ID | 變量識別碼總稱，包含 SVID 與 DVV (數據變量) |
| **SVID** | Status Variable ID | 狀態變量，讀取設備當前動態數值（溫度、壓力、ControlMode） |
| **ECID** | Equipment Constant ID | 設備常數，控制設備行為的參數（逾時設定、重試次數） |
| **CEID** | Collection Event ID | 收集事件，代表設備發生的特定動作（製程開始、完成） |
| **RPTID** | Report ID | 報告識別碼，主機定義一組 VID 組合成報告 |
| **ALID** | Alarm ID | 警報識別碼，代表特定的警報類型 |

### 動態數據監控連結機制

```
VID 定義 (設備預埋數據點)
    ↓
RPTID 定義 (S2F33: 將 VID 打包成報告)
    ↓
CEID 連結 (S2F35: 將 RPTID 連結至 CEID)
    ↓
事件觸發 (S6F11: CEID 發生時自動上報 RPTID 內容)
```

---

## HSMS 通訊層協議 (SEMI E37)

HSMS 在 TCP/IP 之上建立邏輯 Session 管理機制。大多數晶圓廠採用 **HSMS-SS (Single Selected-Session Mode)**。

### HSMS 狀態轉換

```
┌─────────────────┐
│  NOT CONNECTED  │ ← 初始狀態，等待 TCP 連線
└────────┬────────┘
         │ TCP 連線成功
         ↓
┌─────────────────┐
│  NOT SELECTED   │ ← TCP 已連線，等待 Select
└────────┬────────┘
         │ Select.req / Select.rsp
         ↓
┌─────────────────┐
│    SELECTED     │ ← 可收發 SxFy 資料訊息
└─────────────────┘
```

### 關鍵逾時參數

| 參數 | 名稱 | HT9045 預設值 | 說明 |
|:----:|------|:-------------:|------|
| T3 | Reply Timeout | 120 秒 | 等待回覆的最大時間 |
| T5 | Connect Separation Timeout | 10 秒 | TCP 斷線後重新連線的等待時間 |
| T6 | Control Transaction Timeout | 5 秒 | Select/Linktest 控制訊息等待時間 |
| T7 | Connection Select Timeout | 10 秒 | Select.req 等待 Select.rsp 的時限 |
| T8 | Intercharacter Timeout | 5 秒 | 封包字符間最大時間間隔 |

#### HT9045 逾時參數設定 (uHGemEquipment.h)

```cpp
// UI 元件對應
TEdit *edtT3TimeOut;  // Reply Timeout
TEdit *edtT5TimeOut;  // Connect Separation
TEdit *edtT6TimeOut;  // Control Transaction
TEdit *edtT7TimeOut;  // Connection Select
TEdit *edtT8TimeOut;  // Intercharacter
```

**設定檔儲存位置**: `system\GEM\GemSys.ini`

### HSMS 控制訊息

| 訊息 | 功能 | 狀態影響 |
|------|------|----------|
| Select.req/rsp | 請求建立邏輯 Session | 進入 SELECTED |
| Linktest.req/rsp | 心跳檢測 (Keep-alive) | 維持 SELECTED |
| Separate.req | 宣告通訊終止 | 回歸 NOT SELECTED |
| Reject.req | 拒絕不支援的訊息 | 告知處理失敗 |

---

## SECS-II 訊息格式 (SEMI E5)

### Stream & Function 命名規則

- **Stream (S)**：訊息類別（0-127）
- **Function (F)**：訊息功能
  - 奇數 = 請求 (Request)，常帶 `W` 表示等待回覆
  - 偶數 = 回應 (Response)

### Stream 功能總覽

| Stream | 用途 | 方向 |
|:------:|------|:----:|
| S1 | 系統管理 (Equipment Status) | H?E |
| S2 | 遠端控制與設備設定 (Remote Control & Constants) | H→E |
| S5 | 報警管理 (Alarm Management) | E→H |
| S6 | 數據收集與事件報告 (Data Collection) | E→H |
| S7 | 配方管理 (Process Program) | H?E |
| S9 | 系統錯誤 (System Errors) | E→H |
| S10 | 終端服務 (Terminal Services) | H→E |

---

## Stream & Function 指令總表

### Stream 1: 系統管理 (System Management)

負責建立連線與查詢基本設備狀態。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S1F1/F2 | Are You There | H?E | 詢問設備是否在線 |
| S1F3/F4 | Selected Equipment Status | H→E | 根據 SVID 請求狀態數據 |
| S1F11/F12 | Status Variable Namelist | H→E | 請求 SVID 名稱列表 |
| S1F13/F14 | Establish Communications | H?E | 建立通訊（邏輯交握） |
| S1F15/F16 | Request Offline | H→E | 請求切換至 Offline |
| S1F17/F18 | Request Online | H→E | 請求切換至 Online |
| S1F23/F24 | Collection Event Namelist | H→E | 請求 CEID 名稱列表 |

#### S1F1 (Are You There Request)

```
S1F1 W
.
```

#### S1F2 (On Line Data)

```
S1F2
<L [2]
    <A "Equipment Model">      ← MDLN
    <A "Software Version">     ← SOFTREV
>.
```

#### S1F3 (Selected Equipment Status Request)

```
S1F3 W
<L [n]
    <U4 SVID1>
    <U4 SVID2>
    ...
>.
```

#### S1F4 (Selected Equipment Status Data)

```
S1F4
<L [n]
    <value1>
    <value2>
    ...
>.
```

#### S1F13 (Establish Communications Request)

```
S1F13 W
<L [2]
    <A "WET.01">              ← MDLN
    <A "REV.01">              ← SOFTREV
>.
```

#### S1F17 (Request Online)

```
S1F17 W
.
```

#### S1F18 (On Line Acknowledge)

```
S1F18
<B 0x00>                      ← ONLACK: 0=Accepted
.
```

---

### Stream 2: 遠端控制與設備設定 (Remote Control & Constants)

負責配置 ECID 以及下達遠端執行命令。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S2F13/F14 | Equipment Constant Request | H→E | 查詢 ECID 值 |
| S2F15/F16 | New Equipment Constant | H→E | 修改 ECID 值 |
| S2F17/F18 | Date and Time Request | H→E | 請求設備時間 |
| S2F29/F30 | Equipment Constant Namelist | H→E | 請求 ECID 列表 |
| S2F31/F32 | Date and Time Set | H→E | 設定設備時間 |
| S2F33/F34 | Define Report | H→E | 定義報告 (RPTID + VID) |
| S2F35/F36 | Link Event Report | H→E | 連結 CEID 與 RPTID |
| S2F37/F38 | Enable/Disable Event Report | H→E | 啟用/停用事件報告 |
| S2F41/F42 | Host Command Send | H→E | 遠端控制指令 |

#### S2F31 (Date and Time Set Request)

```
S2F31 W
<A "2026032714000000">        ← CCYYMMDDHHmmsscc
.
```

#### S2F32 (Date and Time Set Acknowledge)

```
S2F32
<B 0x00>                      ← TIACK: 0=OK, 1=Error
.
```

#### S2F33 (Define Report)

```
S2F33 W
<L [2]
    <U4 62>                   ← DATAID
    <L [1]
        <L [2]
            <U4 9002>         ← RPTID
            <L [2]
                <U4 810>      ← VID 1
                <U4 800>      ← VID 2
            >
        >
    >
>.
```

#### S2F35 (Link Event Report)

```
S2F35 W
<L [2]
    <U4 63>                   ← DATAID
    <L [1]
        <L [2]
            <U4 4050>         ← CEID
            <L [1]
                <U4 9002>     ← RPTID
            >
        >
    >
>.
```

#### S2F41 (Host Command Send)

```
S2F41 W
<L [2]
    <A "PP-SELECT">           ← RCMD
    <L [1]
        <L [2]
            <A "PPExecName">  ← CPNAME
            <A "Recipe_001">  ← CPVAL
        >
    >
>.
```

#### S2F42 (Host Command Acknowledge)

```
S2F42
<L [2]
    <B 0x00>                  ← HCACK: 0=OK
    <L [0]>                   ← CPACK (空=無錯誤)
>.
```

**HCACK 回傳值**：

| HCACK | 說明 | HT9045 使用情境 |
|:-----:|------|----------------|
| 0 | Acknowledge (成功) | 指令執行成功 |
| 1 | Denied, invalid command | 無效指令或未啟用 |
| 2 | Denied, cannot perform now | 機台有 IC / 執行中 |
| 3 | Denied, parameter error | 參數錯誤 |
| 4 | Acknowledge, will complete later | 稍後完成 |
| **7** | FTP 控制中 (HT9045 擴充) | DOWNLOAD_RECIPE_BY_FTP 執行中 |
| **8** | LIST 結構錯誤 (HT9045 擴充) | S2F41 LIST 格式不正確 |
| **9** | 參數名稱錯誤 (HT9045 擴充) | CPNAME 非 "Setup_File" |
| **10** | LIST 類型錯誤 (HT9045 擴充) | 資料類型不符 |

> **Note**: HCACK 7-10 為 HT9045 自定義擴充碼 (Steven 20240923)

---

### Stream 5: 報警管理 (Alarm Management)

處理設備故障或警報。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S5F1/F2 | Alarm Report Send | E→H | 發送報警訊息 |
| S5F3/F4 | Enable/Disable Alarm | H→E | 啟用/禁用報警 |
| S5F5/F6 | List Alarms Request | H→E | 查詢報警列表 |
| S5F7/F8 | List Enabled Alarms | H→E | 查詢已啟用報警 |

#### S5F1 (Alarm Report Send)

```
S5F1 W
<L [3]
    <B 0x80>                  ← ALCD (0x80=Set, 0x00=Clear)
    <U4 10>                   ← ALID
    <A "ALARM: home failed!"> ← ALTX
>.
```

#### S5F3 (Enable/Disable Alarm Send)

```
S5F3 W
<L [2]
    <B 0x00>                  ← ALED (0x00=Disable, 0x80=Enable)
    <U4 6410>                 ← ALID
>.
```

---

### Stream 6: 數據收集與事件報告 (Data Collection)

回報設備實際發生的動作與關聯數據。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S6F11/F12 | Event Report Send | E→H | CEID 觸發時發送報告 |
| S6F13/F14 | Annotated Event Report | E→H | 含註釋的事件報告 |
| S6F15/F16 | Event Report Request | H→E | 主動請求事件報告 |
| S6F19/F20 | Individual Report Request | H→E | 請求個別報告 |
| S6F23/F24 | Request Spooled Data | H→E | 請求假脫機數據 |

#### S6F11 (Event Report Send)

```
S6F11 W
<L [3]
    <U4 1234>                 ← DATAID
    <U4 4050>                 ← CEID
    <L [1]                    ← Reports
        <L [2]
            <U4 9002>         ← RPTID
            <L [2]            ← VID Values
                <A "Value1">
                <I4 12345>
            >
        >
    >
>.
```

#### S6F13 (Annotated Event Report)

```
S6F13 W
<L [3]
    <U4 4060>                 ← DATAID
    <U4 4060>                 ← CEID (Run Check Waiting)
    <L [1]
        <L [2]
            <U4 9006>         ← RPTID
            <L [...]>         ← Annotated Data
        >
    >
>.
```

---

### Stream 7: 配方管理 (Process Program)

用於製程程序（Recipe）的傳送與詢問。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S7F1/F2 | Process Program Load Inquire | H→E | 詢問是否可加載配方 |
| S7F3/F4 | Process Program Send | H→E | 傳送配方內容 |
| S7F5/F6 | Process Program Request | H→E | 請求下載配方 |
| S7F17/F18 | Delete Process Program | H→E | 刪除配方 |
| S7F19/F20 | Current EPPD Request | H→E | 請求可用配方列表 |
| S7F23/F24 | Formatted PP Send | H→E | 傳送格式化配方 |
| S7F25/F26 | Formatted PP Request | H→E | 請求格式化配方 |

#### S7F3 (Process Program Send)

```
S7F3 W
<L [2]
    <A "PackageOne">          ← PPID
    <A "Recipe Body Content"> ← PPBODY
>.
```

#### S7F19 (Current EPPD Request)

```
S7F19 W
.
```

#### S7F20 (Current EPPD Data)

```
S7F20
<L [n]
    <A "Recipe_001">
    <A "Recipe_002">
    ...
>.
```

---

### Stream 9: 系統錯誤 (System Errors)

當收到無法識別的訊息時，設備的回應。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S9F1 | Unrecognized Device ID | E→H | 設備編號錯誤 |
| S9F3 | Unrecognized Stream Type | E→H | 不支援的 Stream |
| S9F5 | Unrecognized Function Type | E→H | 不支援的 Function |
| S9F7 | Illegal Data | E→H | 數據格式錯誤 |
| S9F9 | Transaction Timer Timeout | E→H | 交易逾時 |
| S9F11 | Data Too Long | E→H | 數據過長 |

---

### Stream 10: 終端服務 (Terminal Services)

在設備顯示器上顯示文字訊息。

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S10F1/F2 | Terminal Request | H→E | 請求終端訊息 |
| S10F3/F4 | Terminal Display, Single | H→E | 單行文字顯示 |
| S10F5/F6 | Terminal Display, Multi | H→E | 多行文字顯示 |

#### S10F3 (Terminal Display, Single)

```
S10F3 W
<L [2]
    <B 0x00>                  ← TID (Terminal ID)
    <A "Test for Document">   ← TEXT
>.
```

---

## GEM 設備狀態模型 (SEMI E30)

GEM 核心在於標準化的控制狀態模型，決定主機與設備之間的權限分配。

### 控制狀態 (Control State)

```
┌─────────────────────────────────────────────────────┐
│                    Equipment                         │
├─────────────────────────────────────────────────────┤
│                                                      │
│   ┌─────────┐    S1F17     ┌──────────────────┐    │
│   │ OFFLINE │ ───────────→ │     ONLINE       │    │
│   └─────────┘              │                  │    │
│        ↑                   │  ┌────────────┐  │    │
│        │      S1F15        │  │   LOCAL    │  │    │
│        └─────────────────  │  └────────────┘  │    │
│                            │        ?         │    │
│                            │  ┌────────────┐  │    │
│                            │  │   REMOTE   │  │    │
│                            │  └────────────┘  │    │
│                            └──────────────────┘    │
└─────────────────────────────────────────────────────┘
```

| 狀態 | 說明 | 主機權限 |
|------|------|----------|
| **Offline** | 設備不接受主機遠端操作 | 無控制權 |
| **Online / Local** | 本地自動化模式 | 可監控，無法遠端控制 |
| **Online / Remote** | 遠端自動化模式 (工廠正常運作) | **完整控制權** |

### 指令間的邏輯關係

```
1. 連線階段
   S1F13 (建立通訊) → S1F17 (切換 Online)

2. 配置階段
   S2F33 (定義 RPTID) → S2F35 (連結 CEID-RPTID)

3. 執行階段
   S2F41 (Host Command) → 設備運作 → S6F11 (自動回報 CEID)

4. 監控階段
   S1F3 (詢問 SVID) / S5F1 (警報通知)
```

---

## 資料型別

| 型別 | 代碼 | 說明 | 範例 |
|------|:----:|------|------|
| L | List | 列表 | `<L [2] ... >` |
| B | Binary | 二進位 | `<B 0x80>` |
| BOOLEAN | Boolean | 布林 | `<BOOLEAN TRUE>` |
| A | ASCII | 字串 | `<A "Text">` |
| U1 | Unsigned 1B | 無符號整數 | `<U1 255>` |
| U2 | Unsigned 2B | 無符號整數 | `<U2 65535>` |
| U4 | Unsigned 4B | 無符號整數 | `<U4 12345>` |
| U8 | Unsigned 8B | 無符號整數 | `<U8 ...>` |
| I1 | Signed 1B | 有符號整數 | `<I1 -128>` |
| I2 | Signed 2B | 有符號整數 | `<I2 -32768>` |
| I4 | Signed 4B | 有符號整數 | `<I4 -12345>` |
| I8 | Signed 8B | 有符號整數 | `<I8 ...>` |
| F4 | Float 4B | 單精度浮點 | `<F4 3.14>` |
| F8 | Float 8B | 雙精度浮點 | `<F8 3.14159265>` |

---

## HT9045 實作

> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260323`

### 功能支援總覽

#### GEM 基礎功能 (SEMI E5/E30/E37)

| # | 功能 | 狀態 | 實作位置 |
|:--:|------|:----:|----------|
| 1 | 建立連線及初始化流程 | ? | S1F1/F2, S1F13/F14 |
| 2 | 刪除、下載及上傳程式 | ? | S7F1/F2, S7F17/F18, S101F5-F8 |
| 3 | 選程式/切換程式 | ? | S7F19/F20, PP_SELECT (S2F41) |
| 4 | 修改參數流程 | ? | S2F13/F14, S2F15/F16 |
| 5 | 動態設定事件/報告 | ? | S2F33/F34, S2F35/F36, S2F37/F38 |
| 6 | 傳送錯誤指令/格式/連線 | ? | S9F3, S9F7 |
| 7 | 開啟/關閉警報 | ? | S5F1/F2, S5F3/F4, S5F5/F6 |
| 8 | Host 下 Remote Command | ? | S2F41/F42 |
| 9 | 時間同步 | ? | S2F31/F32 |

#### GEM300 進階功能 (SEMI E40/E84/E87/E90/E94)

| # | 功能 | 狀態 | 實作位置 |
|:--:|------|:----:|----------|
| 10 | GEM300 Auto Run Scenario | ?? | 部分支援 (缺 E94/E40) |
| 11 | Carrier 管理與 Port 事件 | ? | CEID 系列 |
| 12 | Material ID/Count 報告 | ? | S3Fx, SVID |
| 13 | 量測結果拋送 | ?? | SECS-II 格式 (非 XML) |
| 14 | SEMI E84 介面 | ? | 完整 E84 訊號 |
| 15 | ASECL Like-E87 功能 | ? | 13/14 項目 (93%) |

#### 效能追蹤與擴充功能 (SEMI E10/ASECL)

| # | 功能 | 狀態 | 實作位置 |
|:--:|------|:----:|----------|
| 16 | SEMI E10 / Gross UPH / Net UPH | ? | SVID 1021, 1028, 1032-1035 |
| 17 | 自動過帳功能 (Track IN/OUT) | ?? | 8/13 項目支援 |
| 18 | RFID Tag 讀取功能 | ?? | NFC 13.56MHz (需 UHF 升級) |
| 19 | Remote Control 指令 | ?? | 7/11 項目支援 |
| 20 | E142 Substrate Mapping | ? | 缺 S14F1/F2 Map Download |

**圖例**：? 完整支援 | ?? 部分支援 | ? 不支援/不適用

---

### 功能支援詳細說明

#### 2. Recipe 管理 (S7Fx)

**支援訊息**：

| S/F | 名稱 | 方向 | 說明 |
|:---:|------|:----:|------|
| S7F1/F2 | PP Load Inquire/Grant | H→E | 詢問是否可上傳程式 |
| S7F3/F4 | PP Send/Ack | H→E | 傳送程式資料 |
| S7F5/F6 | PP Request/Data | H→E | 請求下載程式 |
| S7F17/F18 | Delete PP/Ack | H→E | 刪除程式 |
| S7F19/F20 | Current EPPD Request/Data | H→E | 查詢目前程式清單 |
| S7F23/F24 | Formatted PP Send/Ack | H→E | Formatted Recipe 傳送 |
| S7F25/F26 | Formatted PP Request/Data | H→E | Formatted Recipe 查詢 |
| S101F5-F8 | Extended Upload/Download | H?E | 擴充下載/上傳 |

**S7F18 回傳值 (ACKC7)**：

| ACKC | 說明 |
|:----:|------|
| 0 | Acknowledge, 刪除成功 |
| 1 | Permission denied, 程式使用中 |
| 4 | PPID not found, 程式不存在 |

**Recipe 結構分離**：

| 參數類型 | 儲存位置 | S7F4 下載時行為 |
|----------|----------|:---------------:|
| 製程參數 | `IniData/Data/{PPID}/*.Data` | ? 覆蓋 |
| 機構參數 (Offset) | `IniData/Offset/{PPID}/` | ? 保留不覆蓋 |
| 溫度參數 | `Temperature.Data` | ? 保留不覆蓋 |
| Auto Clean 計數 | `HandlerCondition.Data` | ? 保留不覆蓋 |

**實作細節 (uHGemHT9045.cpp)**：

```cpp
// S7F18 刪除限制 - 使用中的 Recipe 不能刪除
if(asLastFileName == PPID)
{
    HGemPtr->LocalAcknowledge(7, 18, 1);  // ACKC=1: Permission denied
    return;
}

// S7F4 下載時機構參數保護
NewFile = PathCombin(OffsetPath, "Position Offset.Data");
if(!FileExists(NewFile))  // 只在不存在時複製，不覆蓋現有參數
    CopyFile(OrgFile, NewFile, false);
```

#### 9. 時間同步 (S2F31/F32)

**實作位置**：`uHGemClass.cpp` Line ~1080

**支援時間格式**：

| 長度 | 格式 | 範例 |
|:----:|------|------|
| 12 | YYMMDDHHmmss | 260327143000 |
| 14 | CCYYMMDDHHmmss | 20260327143000 |
| 16 | CCYYMMDDHHmmsscc | 2026032714300000 |
| 19 | CCYY-MM-DD HH:mm:ss | 2026-03-27 14:30:00 |
| 21 | CCYY-MM-DD HH:mm:ss.c | 2026-03-27 14:30:00.0 |
| 22 | CCYY-MM-DD HH:mm:ss.cc | 2026-03-27 14:30:00.00 |

**S2F32 回傳值 (TIACK)**：

| TIACK | 說明 |
|:-----:|------|
| 0 | Acknowledge, Time Set 成功 |
| 1 | Error, Time Set 失敗 |

> ?? **注意**：設定系統時間需要管理員權限，成功設定後觸發 CEID 1 (TimeChange)

#### 11. Carrier 管理與 Port 事件

**Port 狀態變數 (SVID)**：

| SVID | 名稱 | 說明 |
|:----:|------|------|
| 38120 | Load Port Transfer State - LOADER | Loader 傳送狀態 |
| 38121 | Load Port Transfer State - EMPTY | Empty 傳送狀態 |
| 38122 | Load Port Transfer State - COLOR | Color 傳送狀態 |
| 38123 | Load Port Transfer State - AUTO1 | Auto1 傳送狀態 |
| 38124-38128 | AUTO2-6 | Auto2-6 傳送狀態 |
| 38129-38133 | FIX1-5 | Fix1-5 傳送狀態 |

**Loader 入料事件**：

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 95 | CassetteLoadComplete | Cassette 上料完成 |
| 140 | PreLoadTray | Tray 準備入料 |
| 154 | LoadNoTray | Loader 空盤 |
| 155 | LoadFullTray | Loader 滿盤 |
| 156 | LoadOnlyOneTray | Loader 剩一盤 |
| 157/158 | Loader_ReadyToUnload/FinishUnload | 退盤事件 |
| 217 | LoadPortStatusChanged | LoadPort 狀態變更 |
| 236/237 | LoadPortBundleArrived/Read | Bundle 事件 |

**Unloader 出料事件**：

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 101 | ReadyToUnload | 準備下料 |
| 102 | UnloadComplete | 下料完成 |
| 136-138 | Auto1~3 Unloading tray | Auto 退盤中 |
| 145-147 | Auto4~6 Unloading tray | Auto 退盤中 |
| 196-201 | Auto1~6 ReadyToUnload | 開始退盤 |
| 202-207 | Auto1~6 NoTray | 空盤事件 |
| 220-231 | PortStatusChanged | Port 狀態變更 |

**Carrier ID 讀取事件**：

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 96 | CassetteIDReadComplete | Cassette ID 讀取完成 |
| 115 | TrayIDChange | Tray ID 變更 |
| 285 | UnloaderTrayIDReadOK | Unloader Tray ID 讀取成功 |
| 286 | UnloaderTrayIDReadFail | Unloader Tray ID 讀取失敗 |
| 287 | LoaderTrayIDReadFail | Loader Tray ID 讀取失敗 |

#### 12. Material ID/Count 報告

**Material 相關事件**：

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 26 | GetTestResult | 單片測試結果 |
| 129 | TestFinish | 單片測試完成 |
| 8 | DoLotEnd | 整批測試完成 |
| 130 | MaterialReceive | 物料接收 |

**Count 相關變數**：

| 變數 | 型態 | 用途 |
|------|------|------|
| `LastSet.SendCT[0]` | int | 輸入數量 |
| `LastSet.BinCT[bin]` | int[] | 各 Bin 分類數量 |
| `TastCategory.iTotalCategory[bin]` | int[] | 總分類數量 |

#### 14. SEMI E84 介面交握

**E84 Active 訊號 (設備→AMHS) - cmydef.cpp Line 1226-1234**：

| 訊號 | 常數 | 說明 |
|------|------|------|
| VALID | SnE84VALID (358) | 有效訊號 |
| CS_0 | SnE84CS0 (359) | Carrier Stage 0 |
| CS_1 | SnE84CS1 (360) | Carrier Stage 1 |
| AM_AVBL | SnE84AMAVBL (361) | AM Available |
| TR_REQ | SnE84TRREQ (362) | Transfer Request |
| BUSY | SnE84BUSY (363) | 忙碌中 |
| COMPT | SnE84COMPT (364) | Complete |
| CONT | SnE84CONT (365) | Continue |

**E84 Passive 訊號 (AMHS→設備) - cmydef.cpp Line 1362-1370**：

| 訊號 | 常數 | 說明 |
|------|------|------|
| L_REQ | SnE84LREQ (474) | Load Request |
| U_REQ | SnE84UREQ (475) | Unload Request |
| VA | SnE84VA (476) | Valid Active |
| READY | SnE84READY (477) | Ready |
| VS_0 | SnE84VS0 (478) | Valid Stage 0 |
| VS_1 | SnE84VS1 (479) | Valid Stage 1 |
| HO_AVBL | SnE84HOAVBL (480) | Host Available |
| ES | SnE84ES (481) | E-Stop |
| POWER | SnE84POWER (482) | Power |

**多 Load Port 支援** - Line 514-531：
- Load Port 1：`SnE84_1_*` (523-531)
- Load Port 2：`SnE84_2_*` (514-522)

#### 10. GEM300 Auto Run Scenario

**支援的 RCMD 指令**：

| RCMD | 程式碼行數 | 說明 |
|------|:----------:|------|
| PP_SELECT | L2133 | 選擇 Recipe (PPID) |
| DOWNLOAD_RECIPE_BY_FTP | L1731 | FTP 下載 Recipe |
| LOTSTART | L2445 | 開始 Lot |
| SET_LOT_INFO | L2556 | 設定 Lot 資訊 |
| START / REMOTE_START | L1926/L1912 | 啟動設備 |
| PAUSE / STOP | L1262 | 暫停/停止 |
| STOP_LOAD_PORT | L1134 | 停止 Load Port |
| RESTART_LOAD_PORT | L1208 | 重啟 Load Port |
| INITIAL_START | L1647 | 初始啟動 |
| AUTO_RETEST | L1695 | 自動複測 |
| CLEAN_OUT | L1825 | 清空機台 |

**未支援功能**：

| 功能 | 標準 | 說明 |
|------|------|------|
| Control Job 狀態機 | E94 | 無完整 CJ 建立/查詢/刪除 |
| Process Job 狀態機 | E40 | 無完整 PJ 建立/查詢/刪除 |
| 完整 Carrier 狀態機 | E87 | 簡化實作 |

> ?? 簡化版 GEM300 足以應付 IC Tray Handler 自動化生產需求

---

### 核心檔案架構

| 檔案 | 程式碼行數 | 用途 |
|------|:----------:|------|
| `SECSGEM/uHGemHT9045.cpp` | 5,519 | 主控類別、S2F42/S7F2/S7F4/S7F6 處理 |
| `SECSGEM/uHGemHT9045.h` | 351 | CEID 定義 (288+ 個事件) |
| `SECSGEM/uHGemHT9045_SV.cpp` | 887 | SVID 註冊 (350+ 狀態變量) |
| `SECSGEM/uHGemHT9045_EC.cpp` | 1,824 | ECID 註冊 (300+ 設備常數) |
| `SECSGEM/uHGemClass.cpp` | - | GEM 基礎類別與狀態機 |
| `SECSGEM/uHGemEquipment.cpp` | - | 設備層實作 |
| `SECSGEM/SECSGEM.cpp` | - | HSMS 通訊處理 |

### 類別繼承結構

```cpp
class HT9045Gem : public HTGem
{
protected:
    AnsiString EventDescription[SECS_EVENT.TotalEvent];
    
public:
    virtual void AddSV();                           // SVID 註冊
    virtual void AddEC();                           // ECID 註冊
    virtual void AddAlarmList();                    // 警報註冊
    virtual void AddCEID();                         // 事件註冊
    virtual void AddReprot();                       // 報告註冊
    virtual int  S2F15_CheckNewEquipmentConstant(); // EC 變更前檢查
    virtual int  S2F15_UpdateNewEquipmentConstant();// EC 變更後更新
    virtual int  S2F42_Host_Command_Acknowledge();  // 遠端指令處理
    virtual void S5F6_ListAlarmData();
    virtual int  S7F2_ProcessProgramLoadGrant();    // Recipe 上傳許可
    virtual void S7F4_ProcessProgramAcknowledge();  // Recipe 確認
    virtual void S7F6_ProcessProgramData();         // Recipe 資料
    virtual void S14F4_Get2DID_BinCode();          // 2D ID Bin Code
    // ...
};
```

---

### CEID 事件定義 (uHGemHT9045.h)

HT9045 定義了 288+ 個收集事件：

#### 機台操作事件（CEID 1-34）

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 1 | DoStart | Start 按下 |
| 2 | DoPause | Pause 按下 |
| 3 | DoOneCycle | OneCycle 按下 |
| 4 | DoCleanOut | CleanOut 按下 |
| 5 | DoClearCount | ClearCount 按下 |
| 6 | DoLotStart | Lot 開始 |
| 7 | DoLot | Lot 進行中 |
| 8 | DoLotEnd | Lot 結束 |
| 9-16 | Switch* | 各種模式切換 (RunMode/Tester/Production/Engineer/Temperature 等) |
| 17-23 | Enter* | 進入各頁面 (Tool/Config/Offset/Speed/IO/Message/Debug) |
| 24-26 | DoExit/DoHome/GetTestResult | 系統操作 |
| 27 | RunStatus | 機台狀態變更 |
| 28-33 | DoRetry/DoSkip/DoAlarmReset/DoTrayEnd/DoTrayFeed/DoReset | 動作控制 |
| 34 | DoAutoClean | Auto Clean 開始 |

#### Tray 狀態事件（CEID 35-70）

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 35-40 | Auto1-3Full/Fix1-3Full | 各區滿盤 |
| 41-50 | OneCycleFinish/CleanOutFinish/DownloadRecipe/SiteOnOff... | 完成事件 |
| 51-70 | SiteMappingStart/End/UPHRecord/ART 事件 | ART/Site Mapping |

#### SECS 連線狀態（CEID 91-93）

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 91 | SECSOffline | SECS/GEM Offline |
| 92 | SECSOnline | SECS/GEM Online |
| 93 | SECSOnlineRemote | SECS/GEM Online Remote |

#### OHT/Cassette 事件（CEID 94-135）

| CEID | 事件名稱 | 說明 |
|:----:|----------|------|
| 94-102 | TransferBlocked/CassetteLoad/CassetteOut... | 傳送與卸載 |
| 131-133 | SlotMapCountOK/CHECK_IN/CHECK_OUT | Slot Map 與 Check |

#### 擴充 Tray 與 Port 事件（CEID 136-288）

| CEID 範圍 | 類別 | 說明 |
|:---------:|------|------|
| 136-153 | Auto/Fix Unload & Full | 擴充區退盤與滿盤 |
| 154-211 | Loader/Empty/Color/Auto 狀態 | 詳細 Tray 狀態變更 |
| 212-213 | PowerSaving | 省電模式開始/結束 |
| 217-231 | PortStatusChanged | 各出入料區狀態變更 |
| 234-244 | SafetyDoor/Bundle/2DID | 安全門與 Bundle 事件 |
| 245-270 | BundleEnd_AutoX/FixX | 各區 BundleEnd |
| 271-288 | LoaderTrayState/AGV/TrayIDRead | Tray 狀態與 ID 讀取 |

> 完整 CEID 定義請參閱 `uHGemHT9045.h` ETypeStruct enum

---

### SVID 狀態變量註冊 (uHGemHT9045_SV.cpp)

#### 註冊函式簽章

```cpp
void HT9045Gem::AddSV()
{
    // SetSVDataPointer(SVID, Type, Name, Unit, &Variable, Description)
    HGemPtr->SetSVDataPointer(SVID, HType.ASCII_TYPE, "名稱", "單位", &變量指標, "描述");
}
```

#### SVID 編號範圍

| 範圍 | 類別 | 說明 |
|:----:|------|------|
| 1000-1099 | 基本資訊 | 機台定義、UPH、時間統計 |
| 1100-1199 | 計數器 | Loader/Unloader/Auto/Fix Count |
| 1150-1199 | 良率 | Auto1-6、Fix1-12 Yield |
| 1200-1299 | 溫度 | ATC Head、Site 溫度 |
| 1420-1470 | 測試結果 | Site 1-32 Bin Code |
| 2000-2199 | 下壓參數 | Force、Height、Contact |
| 16200-16415 | Site 統計 | Site Aa-Dh Bin Count (含 History) |
| 37000-37800 | 擴充參數 | Barcode、ION FAN、ATC 版本 |
| 38100-38300 | Port 狀態 | Load Port Transfer State |
| 65000-65095 | Head 觸碰次數 | HeadCondition1-3 Contact Count |

#### 核心 SVID 範例

```cpp
void HT9045Gem::AddSV()
{
    // === 機台基本資訊 ===
    HGemPtr->SetSVDataPointer(1000, HType.ASCII_TYPE, "Machine Define", "", 
        &RunInfo.MachineDefine, "機台定義");
    
    HGemPtr->SetSVDataPointer(1001, HType.ASCII_TYPE, "Machine Model", "", 
        &RunInfo.MachineModel, "機台型號");
    
    // === 產能統計 ===
    HGemPtr->SetSVDataPointer(1021, HType.INT_4_TYPE, "UPH", "", 
        &RunInfo.iUPH, "UPH");
    
    HGemPtr->SetSVDataPointer(1028, HType.ASCII_TYPE, "Avg UPH", "", 
        &RunInfo.iAvgUPH, "Average UPH");
    
    // === 計數器 ===
    HGemPtr->SetSVDataPointer(1101, HType.INT_4_TYPE, "Loader Count", "", 
        &LastSet.SendCT[0], "Loading Count");
    
    HGemPtr->SetSVDataPointer(1102, HType.INT_4_TYPE, "Output Total Count", "", 
        &RunInfo.iUnloadCount, "Unloading Count");
    
    HGemPtr->SetSVDataPointer(1103, HType.INT_4_TYPE, "Auto1 Count", "", 
        &LastSet.BinCT[0][e3Auto1], "Auto1 Count");
    
    // === Bin 計數 (0-15) ===
    HGemPtr->SetSVDataPointer(1164, HType.INT_4_TYPE, "Bin 0 Count", "", 
        &LastSet.iBinData32[0][0], "Bin 0 Count");
    // ... Bin 1-15
    
    // === 良率 ===
    HGemPtr->SetSVDataPointer(1151, HType.ASCII_TYPE, "Auto1 Yield", "", 
        &RunInfo.sT6AutoYield[eAuto1], "Auto 1 Yield");
    
    HGemPtr->SetSVDataPointer(1160, HType.INT_4_TYPE, "PassTotalCount", "", 
        &iSECSGEMPass, "Pass Total Count");
    
    HGemPtr->SetSVDataPointer(1161, HType.INT_4_TYPE, "FailTotalCount", "", 
        &iSECSGEMFail, "Fail Total");
    
    // === ATC 溫度 (依配置數量動態註冊) ===
    if(iATC_Use_Heat_Count <= 4)
    {
        HGemPtr->SetSVDataPointer(1351, HType.ASCII_TYPE, "ATC Arm1 Head 1", "", 
            fLotInfo->pl_ATCTempHead01, "ATC Arm1 Head 1");
        // ... Head 2-4
    }
    else if(iATC_Use_Heat_Count <= 8)
    {
        // 8 Head 配置
    }
    
    // === 測試結果 ===
    HGemPtr->SetSVDataPointer(1420, HType.ASCII_TYPE, "Site 1 to 32 Test Result", "", 
        fMain->tTestResult, "Site 1 to 32 Test Result; Bin -1 is no test; CSV Format");
    
    // === 下壓參數 ===
    HGemPtr->SetSVDataPointer(2003, HType.FT_8_TYPE, "Force Per Device KG", "", 
        fContact->edForcePerDeviceKG, "Force Per Device KG");
    
    HGemPtr->SetSVDataPointer(2018, HType.FT_8_TYPE, "Contact Air Force", "", 
        fContact->edAirKPA, "Contact Air Force");
    
    // === Port 狀態 ===
    HGemPtr->SetSVDataPointer(38120, HType.INT_4_TYPE, "Load Port Transfer State - LOADER", "", 
        &iPortStatus[ePortLoader], "Load Port Transfer State - LOADER");
    // ... EMPTY, COLOR, AUTO1-6, FIX1-6
    
    // === EC 變更追蹤 ===
    HGemPtr->SetSVDataPointer(20001, HType.ASCII_TYPE, "EC Change ID", "", 
        &SYS_ECChangeID, "機台EC改變的ID");
    HGemPtr->SetSVDataPointer(20002, HType.ASCII_TYPE, "EC Change Original Value", "", 
        &SYS_ECChangeIDOriginaValue, "機台EC改變ID原本的值");
    HGemPtr->SetSVDataPointer(20003, HType.ASCII_TYPE, "EC Change New Value", "", 
        &SYS_ECChangeIDNewValue, "機台EC改變ID新的值");
}
```

---

### ECID 設備常數註冊 (uHGemHT9045_EC.cpp)

#### 註冊函式簽章

```cpp
void HT9045Gem::AddEC()
{
    // SetECDataPointer(ECID, Type, Name, Unit, &Variable, Max, Min, Default, Description)
    HGemPtr->SetECDataPointer(ECID, HType.ASCII_TYPE, "名稱", "單位", 
        &變量指標, "最大值", "最小值", "預設值", "描述");
}
```

#### ECID 編號範圍

| 範圍 | 類別 | 說明 |
|:----:|------|------|
| 1006-1007 | Lot 資訊 | Lot ID、Operator ID |
| 1501-1520 | 配方/溫度 | Recipe、Temperature |
| 1581-1595 | SPIL Lot Info | 擴充 Lot 資訊 |
| 2266-2270 | Auto Site Off | 良率自動關站 |
| 2501-2530 | Arm 參數 | Vacuum Wait、Destroy Wait |
| 2600-2700 | 下壓參數 | Drop Height、Contact Mode |
| 2700-2800 | Tray 參數 | Tray Type、Direction |
| 3400-3600 | 測試介面 | Interface Type、GPIB、RS232 |
| 3616-3700 | Bin 設定 | Bin Tray Select、Bin Type |

#### 核心 ECID 範例

```cpp
void HT9045Gem::AddEC()
{
    // === Lot 資訊 ===
    HGemPtr->SetECDataPointer(1006, HType.ASCII_TYPE, "Lot ID", "", 
        fLotInfo->edtSysLotID, "", "", "", "Lot ID");
    
    HGemPtr->SetECDataPointer(1007, HType.ASCII_TYPE, "Operator ID", "", 
        fLotInfo->edtSysOperatorID, "", "", "", "Operator ID");
    
    // === 配方檔案 ===
    HGemPtr->SetECDataPointer(1501, HType.ASCII_TYPE, "Setup File", "", 
        fMain->cbSetupFileName, "", "", "", "Setup File");
    
    // === 溫度設定 (含上下限與預設值) ===
    HGemPtr->SetECDataPointer(1519, HType.FT_8_TYPE, "Temperature Default", "Celsius", 
        &Temperature.fWorkTemperBase, "150", "20", "30", "Temperature Default");
    
    HGemPtr->SetECDataPointer(1520, HType.FT_8_TYPE, "Soak Time", "Second", 
        &Temperature.fSoakTime, "3600", "0", "60", "Soak Time");
    
    // === Auto Site Off 功能 ===
    HGemPtr->SetECDataPointer(2266, HType.BOOLEAN_TYPE, "ContiFail(Socket) Auto Site Off Func", "", 
        &TestIF_File.bLowYieldAutoSiteOffByContiFail, "1", "0", "0", "0: OFF; 1: ON");
    
    HGemPtr->SetECDataPointer(2269, HType.BOOLEAN_TYPE, "By Socket Compare Yield Auto Site Off Func", "", 
        &TestIF_File.bLowYieldAutoSiteOff, "1", "0", "0", "0: OFF; 1: ON");
    
    // === Arm 等待時間 ===
    HGemPtr->SetECDataPointer(2501, HType.FT_8_TYPE, "Input Arm Vacuum Wait", "Second", 
        &ArmSpeed_File[InArm].dVacuumTI, "10", "0.01", "0.01", "Input Arm Vacuum wait time");
    
    HGemPtr->SetECDataPointer(2511, HType.FT_8_TYPE, "Input Arm Destroy Wait", "Second", 
        &ArmSpeed_File[InArm].dCTAirOn, "10", "0.01", "0.01", "Input Arm Destroy wait time");
    
    // === 下壓模式 ===
    HGemPtr->SetECDataPointer(2616, HType.INT_4_TYPE, "Contact Test Mode for HT9045", "", 
        &DeviceForm_File.ContactMode, "6", "0", "0", 
        "0:Direct Contact Mode; 1:Drop Contact; 2:Direct & Soft Contact Mode; "
        "3:TMOVE; 4:TMOVE Drop; 5:Direct Soft Contact; 6:Drop Soft Contact;");
    
    HGemPtr->SetECDataPointer(2617, HType.INT_4_TYPE, "Vacuum Mode", "", 
        &DeviceForm_File.VacuumMode, "1", "0", "0", 
        "0:Vacuum ON Mode; 1:Vacuum OFF Mode;");
    
    // === Tray 設定 ===
    HGemPtr->SetECDataPointer(2701, HType.INT_4_TYPE, "Tray Type", "", 
        &TrayForm.LodareType, "1", "0", "0", "0:Same; 1:Different");
    
    HGemPtr->SetECDataPointer(2702, HType.INT_4_TYPE, "Loader Tray Type", "", 
        &TrayForm.LoaderToEmptyColor[FT], "2", "0", "0", "0:Empty; 1:Color; 2:Auto2");
    
    // === 測試介面 ===
    HGemPtr->SetECDataPointer(3400, HType.INT_4_TYPE, "Interface Type for HT9045", "", 
        &TestIF_File.iTestType, "2", "0", "1", "0: DIO; 1: GP-IB; 2: RS232;");
    
    HGemPtr->SetECDataPointer(3450, HType.ASCII_TYPE, "Test Mode for HT9045", "", 
        &TestIF_File.sTestMode, "", "", "", 
        "Single Site; 2-Site; 2-Site Busy Shuttle; In-Line 4-Site(1X4); 2-Site (2x1); "
        "Square 4-Site(2X2); Square 4-Site(2X2) Busy Shuttle; 8-Site; 12-Site; 16-Site; "
        "32-Site N Mode; 32-Site M Mode;");
    
    // === Bin 設定 ===
    HGemPtr->SetECDataPointer(3640, HType.INT_4_TYPE, "Bin 0 Type", "", 
        &Prod.iIsFailBin[0], "", "", "", "0:Pass; 1:Fail;");
    // ... Bin 1-15
    
    HGemPtr->SetECDataPointer(3656, HType.ASCII_TYPE, "Bin 0-255 Type", "", 
        fBinSel->sBinType[eBinFT], "", "", "", "0:Pass; 1:Fail; CSV Format for Bin 0 to 255");
}
```

---

### S2F42 Remote Command 處理 (uHGemHT9045.cpp)

#### 支援的 Remote Command 完整列表

**基本操作指令**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `START` | 啟動機台 | 0/1 | 需啟用 bCanRemoteStart |
| `REMOTE_START` | 遠端啟動 | 0/1 | 需風險告知確認 |
| `PAUSE` / `STOP` | 暫停機台 | 0 | |
| `HALT` | 停止運行 | 0/2 | 需 bRCMDStart |
| `HOME` | 回原點 | 0/1 | 需 bCanRemoteStart |
| `RETRY` | 重試 | 0 | 關閉對話框 |
| `TRAY_END` / `TRAY_FEED` | 補盤 | 0/2 | 機台無 IC 時可執行 |
| `CLEAN_OUT` | 清出機台 IC | 0 | |

**Recipe 與設定**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `PP_SELECT` / `PP-SELECT` | 切換 Recipe | 0 | |
| `DOWNLOAD_RECIPE_BY_FTP` | FTP 下載 Recipe | 0/7-10 | 7=FTP中, 8=LIST錯, 9=參數名錯, 10=類型錯 |
| `PP_PASSWORD` | 變更密碼 | 0/1 | |
| `PP_MUSIC` | 音樂控制 | 0/1 | |
| `PP_SIGNALTOWER` | 信號燈控制 | 0/1 | RED/GREEN/YELLOW |

**Lot 管理**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `LOT_START` | 開始 Lot | 0 | AMKOR 專用參數 |
| `LOT_END` | 結束 Lot | 0 | |
| `LOT_PRE_END` | Lot 預結束 | 0 | |
| `TRAY_CHECK` | Tray 檢查 | 0/1/2 | 含 Bind/Unbind/Skip 參數 |

**運行模式切換**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `SWITCH_TO_FT` | 切換至 FT 模式 | 0/2 | |
| `SWITCH_TO_RT` | 切換至 RT 模式 | 0/2 | |
| `INITIAL_START` | 初始啟動 | 0/2 | 機台無 IC |
| `INITIAL_START_ART` | ART 初始啟動 | 0/2 | |
| `INITIAL_START_MRT` | MRT 初始啟動 | 0/2 | |
| `AUTO_RETEST` | 自動回測 | 0/2 | TSMC ATR |
| `AUTOSITEMAP` | Auto Site Map | 0/2 | |
| `CONTINUE_START_ART` | ART 續測 | 0/2 | |
| `CONTINUE_START_MRT` | MRT 續測 | 0/2 | |
| `RETEST_MRT` | MRT 回測 | 0/2 | |

**Port 與輸出管理**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `DISCHARGE_OUTPUT_PORT` | 退出指定 Port | 0/1 | 需參數指定 Port |
| `DISCHARGE_OUTPUT_ALL_PORT` | 退出全部 Port | 0 | |

**清除與告警**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `CLEAN_AUTO_SORT_COUNT` | 清除 ART 計數 | 0/2 | |
| `AUTO_CLEAN` | Auto Clean 啟動 | 0/1 | 需啟用 AutoClean |
| `ALARM_NOTIFY` | Host 警報通知 | 0 | 顯示於機台 |

**控制狀態**：

| 指令 | 功能 | HCACK | 備註 |
|------|------|:-----:|------|
| `ONLINE_LOCAL` | 切換 Online Local | 0 | |
| `ONLINE_REMOTE` | 切換 Online Remote | 0 | |
| `REMOTE_SAVE` | 遠端儲存 | 0/4 | 機台無 IC 且未運行 |

#### HCACK 回應碼 (含 HT9045 擴充)

| 值 | 說明 | 使用情境 |
|:--:|------|----------|
| 0 | 成功 (ACK) | 指令執行成功 |
| 1 | 指令無效 / 未啟用 | START 未啟用、無效指令 |
| 2 | 機台有 IC / 無法執行 | INITIAL_START 時有殘料 |
| 3 | RunStartMode 禁用 | SWITCH_TO_FT/RT |
| 4 | 稍後完成 / 機台運行中 | REMOTE_SAVE |
| 5 | RunStartMode 禁用 (RT) | SWITCH_TO_RT |
| 6 | Tray 有 IC | SWITCH_TO_FT/RT |
| **7** | FTP 控制中 | DOWNLOAD_RECIPE_BY_FTP |
| **8** | LIST 結構錯誤 | DOWNLOAD_RECIPE_BY_FTP |
| **9** | 參數名稱錯誤 | DOWNLOAD_RECIPE_BY_FTP (非 Setup_File) |
| **10** | LIST 類型錯誤 | DOWNLOAD_RECIPE_BY_FTP |

#### 程式碼範例

```cpp
// uHGemHT9045.cpp
int HT9045Gem::S2F42_Host_Command_Acknowledge()
{
    AnsiString S = GetRCMD();  // 取得 Remote Command
    int HCACK = 0;
    
    if(S.AnsiPos("START") == 1)
    {
        if(SystemStart == false)
        {
            fMain->Start("SECS GEM RCMD : START");
            HCACK = 0;
        }
        else
        {
            HCACK = 3;  // 已在執行中
        }
    }
    else if(S.AnsiPos("PAUSE") == 1 || S.AnsiPos("STOP") == 1)
    {
        fMain->BtnPauseClick(fMain);
        HCACK = 0;
    }
    else if(S.AnsiPos("PP_SELECT") == 1)
    {
        // 解析 Recipe 名稱
        AnsiString PPID = ParseParameter("PPID");
        fMain->cbSetupFileName->Text = PPID;
        fMain->cbSetupFileNameChange(fMain);
        HCACK = 0;
    }
    else if(CUSTOMER_CODE == CC_AMKOR_Korea && S.AnsiPos("LOT_START") == 1)
    {
        // AMKOR 專用：解析 LOT_START 參數
        // LOT_ID, DCC, OPERATION_CODE, UNIT_QTY, TRAY_ID, LOT_YIELD, HARD_BIN_INFO, OUTPUT_TRAY_QTY
        AnsiString strLotID, sDCC, sOP, sLotCnt, sTrayID, sYield, sHBin, sTrayQty;
        
        // 解析 LIST 結構中的參數
        int SVlen = 0;
        HGem->GetDataItemLenAndTypeAndDelete(SVlen, HType.LIST_TYPE);
        
        for(int i = 0; i < SVlen; i++)
        {
            // 讀取 Key-Value 對
            AnsiString Key = ReadASCIIParameter();
            AnsiString Value = ReadASCIIParameter();
            
            if(Key == "LOT_ID")       strLotID = Value;
            else if(Key == "DCC")     sDCC = Value;
            // ... 其他參數
        }
        
        fLotInfo->sbSECSLotStartClick(fLotInfo);
        RecordProcess("SECS/GEM LOTSTART!");
        HCACK = 0;
    }
    else if(S.AnsiPos("ALARM_NOTIFY") == 1)
    {
        // Host 發送警報訊息至設備顯示
        AnsiString AlarmDesc = ParseParameter("HOST_ALARM_DESCRIPTION");
        SecsAlarmMessage->Add(AlarmDesc);
        HCACK = 0;
    }
    else
    {
        HCACK = 1;  // 無效指令
    }
    
    // 回覆 S2F42
    HGemPtr->InitLocalHead(2, 42, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
    HGemPtr->SendLocalData();
    
    return HCACK;
}
```

---

### 事件觸發範例

```cpp
// 觸發 CEID 事件
void TriggerEvent(int CEID)
{
    HGemPtr->SendCEID(CEID);
}

// 範例：測試完成時觸發
void OnTestFinish()
{
    // 更新 SVID 數據
    RunInfo.iUnloadCount++;
    LastSet.iBinData32[0][BinCode]++;
    
    // 觸發測試完成事件
    HGemPtr->SendCEID(SECS_EVENT.TestFinish);  // CEID 129
}

// 範例：Port 狀態變更
void OnPortStatusChange(int PortIndex)
{
    iPortStatus[PortIndex] = NewStatus;
    
    // 觸發對應 Port 狀態變更事件
    switch(PortIndex)
    {
        case ePortLoader: HGemPtr->SendCEID(SECS_EVENT.LoadPortStatusChanged); break;
        case ePortAuto1:  HGemPtr->SendCEID(SECS_EVENT.Auto1PortStatusChanged); break;
        // ...
    }
}
```

---

### Stream/Function 實作詳細

#### 支援的 Standard SECS-II Messages

| S/F | 功能 | 實作函式 |
|:---:|------|----------|
| S1F1/F2 | Are You There | `S1F1_AreYouThereRequest()` |
| S1F3/F4 | Selected Status | `S1F4_SelectedStatusReply()` |
| S1F12 | SV Namelist | `S1F12_StatusVariableNamelistReply()` |
| S1F13/F14 | Establish Communications | `S1F13`, `S1F14_ConnectRequestAcknowledge()` |
| S1F16 | Offline Acknowledge | `S1F16_OFFLINEAcknowledge()` |
| S1F18 | Online Acknowledge | `S1F18_ONLINEAcknowledge()` |
| S1F24 | CEID Namelist | `S1F24_CollectionEventNamelist()` |
| S2F14 | EC Data | `S2F14_EquipmentConstanData()` |
| S2F15/F16 | New EC | `S2F15_CheckNewEquipmentConstant()`, `S2F15_UpdateNewEquipmentConstant()` |
| S2F18 | Date/Time | `S2F18_DateandTimeData()` |
| S2F24 | Trace Initialize | `S2F24_TraceInitializeAcknowledge()` |
| S2F30 | EC Namelist | `S2F30_EquipmentConstantNamelistReply()` |
| S2F34 | Define Report | `S2F34_DefineReportAcknowledge()` |
| S2F36 | Link Event Report | `S2F36_LinkEventReportAcknowledge()` |
| S2F38 | Enable/Disable Event | `S2F38_EnableDisableEventReportAcknowledge()` |
| S2F42 | Host Command | `S2F42_Host_Command_Acknowledge()` |
| S5F4 | Enable/Disable Alarm | `S5F4_EnableDisableAlarmAcknowledge()` |
| S5F6 | List Alarm | `S5F6_ListAlarmData()` |
| S5F8 | List Enable Alarm | `S5F8_ListEnableAlarmAcknowledge()` |
| S6F16 | Event Report Data | `S6F16_EventReportData()` |
| S6F18 | Annotated Event Report | `S6F18_AnnotatedEventReportData()` |
| S6F20 | Individual Report | `S6F20_IndividualReportData()` |
| S7F2 | PP Load Grant | `S7F2_ProcessProgramLoadGrant()` |
| S7F4 | PP Acknowledge | `S7F4_ProcessProgramAcknowledge()` |
| S7F6 | PP Data | `S7F6_ProcessProgramData()` |
| S7F18 | Delete PP | `S7F18_DeleteProcessProgramAcknowledge()` |
| S7F20 | Current EPPID | `S7F20_CurrentEPPDData()` |
| S7F24 | Formatted PP Send Ack | `S7F24_FormattedProcessProgramSendAcknowledge()` |
| S7F26 | Formatted PP Data | `S7F26_FormattedProcessProgramData()` |
| S10F4 | Terminal Display Single | `S10F4_TerminalDisplaySingleAcknowledge()` |
| S10F6 | Terminal Display Multi | `S10F6_TerminalDisplayMultiBlockAcknowledge()` |

#### 客製化 Messages

| S/F | 功能 | 客戶/用途 |
|:---:|------|----------|
| S100F4 | Report All Alarm | 通用 |
| S101F2-F8 | Host Upload File | 檔案上傳 |
| S103F12 | SV Namelist | 擴充查詢 |
| S110F5/F6 | Customer Name List | ASEM |
| S14F4 | Get 2DID BinCode | ATK |

---

### 資料型態對照

| SECS 型態 | C++ 型態 | HType 常數 |
|-----------|----------|------------|
| ASCII | `char[]` | `HType.ASCII_TYPE` |
| BOOLEAN | `char` | `HType.BOOLEAN_TYPE` |
| INT_1 | `char` | `HType.INT_1_TYPE` |
| INT_2 | `short` | `HType.INT_2_TYPE` |
| INT_4 | `int` | `HType.INT_4_TYPE` |
| INT_8 | `__int64` | `HType.INT_8_TYPE` |
| UINT_1 | `unsigned char` | `HType.UINT_1_TYPE` |
| UINT_2 | `unsigned short` | `HType.UINT_2_TYPE` |
| UINT_4 | `unsigned int` | `HType.UINT_4_TYPE` |
| UINT_8 | `unsigned __int64` | `HType.UINT_8_TYPE` |
| FT_4 | `float` | `HType.FT_4_TYPE` |
| FT_8 | `double` | `HType.FT_8_TYPE` |
| LIST | — | `HType.LIST_TYPE` |
| BINARY | `char` | `HType.BINARY_TYPE` |

---

### 關鍵整合點

#### Yield Monitoring 整合

| 觸發路徑 | 說明 |
|----------|------|
| CEID 68 (AutoCleanClearCount) | Auto Clean 後清計數 |
| S2F42 `CLEAN_AUTO_SORT_COUNT` | Host 觸發清計數 |
| → `ClearYieldCount()` | 清除良率計數 |

#### Recipe 管理整合

| Stream/Function | 功能 |
|-----------------|------|
| S7F1-F6 | Recipe 下載/上傳 |
| S7F17-F20 | Recipe 刪除/查詢 |
| S7F23-F26 | Formatted Recipe |
| ECID 1501 | 當前 Recipe 名稱 |

#### Alarm 整合

| 來源 | 說明 |
|------|------|
| `Error/AlarmCodeList.txt` | Alarm 碼定義 |
| `AddAlarmList()` | 載入 Alarm 到 SECS |
| S5Fx | Alarm 報告 |

---

### 主動上報時機點

HT9045 透過 **S6F11 (Event Report Send)** 主動向 Host 上報事件，共定義 **288 個 CEID 事件**。以下依類別整理觸發時機：

#### 1. 操作按鍵事件 (CEID 1-33)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 1 | DoStart | 按下 Start | S6F11 |
| 2 | DoPause | 按下 Pause | S6F11 |
| 3 | DoOneCycle | 按下 One Cycle | S6F11 |
| 4 | DoCleanOut | 按下 Clean Out | S6F11 |
| 5 | DoClearCount | 按下 Clear Count | S6F11 |
| 24 | DoExit | 按下 Exit | S6F11 |
| 25 | DoHome | 按下 Home | S6F11 |
| 28 | DoRetry | 按下 Retry | S6F11 |
| 29 | DoSkip | 按下 Skip | S6F11 |
| 30 | DoAlarmReset | 按下 Alarm Reset | S6F11 |
| 31 | DoTrayEnd | 按下 Tray End | S6F11 |
| 32 | DoTrayFeed | 按下 Tray Feed | S6F11 |
| 33 | DoReset | 按下 Reset | S6F11 |

#### 2. Lot 生命週期 (CEID 6-8, 108, 119-122)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 6 | DoLotStart | Lot 開始 | S6F11 |
| 7 | DoLot | Lot 進行中 | S6F11 |
| 8 | DoLotEnd | Lot 結束 | S6F11 |
| 108 | DoCSTLotStart | Cassette Lot 開始 | S6F11 |
| 119 | ART_SRQKIND2_FTLOTSTART | ART FT Lot 開始 | S6F11 |
| 120 | ART_SRQKIND4_RTLOTSTART | ART RT Lot 開始 | S6F11 |
| 121 | ART_SRQKIND8_LOTEND | ART Lot 結束 | S6F11 |
| 122 | ART_SRQKIND10_FINALLOTEND | Final Lot 結束 | S6F11 |

#### 3. 測試進度事件 (CEID 26, 66-67, 128-129)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 26 | GetTestResult | 取得測試結果 | S6F11 |
| 66 | LoadTrayFinish | 載入 Tray 完成 | S6F11 |
| 67 | TrayTestFinish | Tray 測試完成 | S6F11 |
| 128 | TestStart | 測試開始 | S6F11 |
| 129 | TestFinish | 測試結束 | S6F11 |

#### 4. Tray/Port 滿盤事件 (CEID 35-40, 148-153)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 35 | Auto1Full | Auto1 滿盤 | S6F11 |
| 36 | Auto2Full | Auto2 滿盤 | S6F11 |
| 37 | Auto3Full | Auto3 滿盤 | S6F11 |
| 38 | Fix1Full | Fix1 滿盤 | S6F11 |
| 39 | Fix2Full | Fix2 滿盤 | S6F11 |
| 40 | Fix3Full | Fix3 滿盤 | S6F11 |
| 148 | Auto4Full | Auto4 滿盤 | S6F11 |
| 149 | Auto5Full | Auto5 滿盤 | S6F11 |
| 150 | Auto6Full | Auto6 滿盤 | S6F11 |
| 151-153 | Fix4-6Full | Fix4-6 滿盤 | S6F11 |

#### 5. Tray 退盤事件 (CEID 136-147, 196-211)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 136 | Auto1Unloadtray | Auto1 退盤中 | S6F11 |
| 137 | Auto2Unloadtray | Auto2 退盤中 | S6F11 |
| 138 | Auto3Unloadtray | Auto3 退盤中 | S6F11 |
| 145-147 | Auto4-6Unloadtray | Auto4-6 退盤中 | S6F11 |
| 196-201 | Auto1-6_ReadyToUnload | Auto1-6 開始退盤 | S6F11 |
| 202-207 | Auto1-6NoTray | Auto1-6 空盤 | S6F11 |
| 209 | TrayEndFinish | Tray End 完成 | S6F11 |
| 210 | Empty_FinishUnload | Empty 退盤完成 | S6F11 |
| 211 | Color_FinishUnload | Color 退盤完成 | S6F11 |

#### 6. 通訊/狀態變更 (CEID 27, 91-93, 141)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 27 | RunStatus | 機台狀態變更 (Run/Idle/Down) | S6F11 |
| 91 | SECSOffline | SECS/GEM 離線 | S6F11 |
| 92 | SECSOnline | SECS/GEM 上線 | S6F11 |
| 93 | SECSOnlineRemote | SECS/GEM 上線 (Remote 模式) | S6F11 |
| 141 | GemControlStateChange | GEM 控制狀態變更 | S6F11 |

#### 7. 模式切換事件 (CEID 9-16, 112-113)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 9 | SwitchRunMode | Real/Dummy 模式切換 | S6F11 |
| 10 | SwitchTesterMode | Tester Online/Offline 切換 | S6F11 |
| 11 | SwitchProduction | 切換生產模式 | S6F11 |
| 12 | SwitchEngineer | 切換工程模式 | S6F11 |
| 15 | SwitchSetupFile | 切換 Job 檔案 | S6F11 |
| 16 | SwitchUser | 切換使用者等級 | S6F11 |
| 112 | MRRunModeChange | MR 執行模式變更 | S6F11 |
| 113 | AccessModeChange | 存取模式變更 | S6F11 |

#### 8. 週期性/定時事件 (CEID 77, 80-83)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 77 | ReadCurrentESDData | 定時上報 ESD 資料 | S6F11 |
| 80 | ReadNowHandlerData | 定時上報 Handler 資料 | S6F11 |
| 81 | ReadATCTemperature | 定時上報 ATC 溫度 | S6F11 |
| 82 | ReadATCRefTemperature | 定時上報 ATC 參考溫度 | S6F11 |
| 83 | ReadNowEPPencoder | 定時上報 EP Encoder 資料 | S6F11 |

#### 9. Carrier/Material 事件 (CEID 95-103, 115, 130, 285-287)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 95 | CassetteLoadComplete | Cassette 載入完成 | S6F11 |
| 96 | CassetteIDReadComplete | Cassette ID 讀取完成 | S6F11 |
| 97 | ReadyToProcessComplete | 準備處理完成 | S6F11 |
| 99 | CassetteOutComplete | Cassette 退出完成 | S6F11 |
| 101 | ReadyToUnload | 準備下料 | S6F11 |
| 102 | UnloadComplete | 下料完成 | S6F11 |
| 115 | TrayIDChange | Tray ID 變更 | S6F11 |
| 130 | MaterialReceive | 接收物料 | S6F11 |
| 285 | UnloaderTrayIDReadOK | Unloader Tray ID 讀取成功 | S6F11 |
| 286 | UnloaderTrayIDReadFail | Unloader Tray ID 讀取失敗 | S6F11 |
| 287 | LoaderTrayIDReadFail | Loader Tray ID 讀取失敗 | S6F11 |

#### 10. Port 狀態變更事件 (CEID 217-231)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 217 | LoadPortStatusChanged | LoadPort 狀態變更 | S6F11 |
| 218 | EmptyPortStatusChanged | EmptyPort 狀態變更 | S6F11 |
| 219 | ColorPortStatusChanged | ColorPort 狀態變更 | S6F11 |
| 220-222 | Auto1-3PortStatusChanged | Auto1-3 Port 狀態變更 | S6F11 |
| 223-225 | Fix1-3PortStatusChanged | Fix1-3 Port 狀態變更 | S6F11 |
| 226-228 | Auto4-6PortStatusChanged | Auto4-6 Port 狀態變更 | S6F11 |
| 229-231 | Fix4-6PortStatusChanged | Fix4-6 Port 狀態變更 | S6F11 |

#### 11. 品質/良率事件 (CEID 78, 86-88, 114, 274)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 78 | JamSkipICCount | Jam Skip IC 計數 | S6F11 |
| 86 | MapNoArmHasIC | Map 無 IC 但 Arm 有 IC | S6F11 |
| 87 | MapHasICArmRetry | Map 有 IC 但 Arm 錯誤重試 | S6F11 |
| 88 | MapHasICArmSkip | Map 有 IC 但 Arm 錯誤跳過 | S6F11 |
| 114 | SoftwareBin | Software Bin 事件 | S6F11 |
| 274 | SECSGEMConsecutiveFailure | 連續失敗警報 | S6F11 |

#### 12. 完成事件 (CEID 41-50)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 41 | OneCycleFinish | One Cycle 完成 | S6F11 |
| 42 | CleanOutFinish | Clean Out 完成 | S6F11 |
| 43 | DownloadRecipe | Recipe 下載完成 | S6F11 |
| 44 | SiteOnOff | Site 開關變更 | S6F11 |
| 45 | ArmOnOff | Arm 開關變更 | S6F11 |
| 46 | SwitchTempData | 溫度設定變更 | S6F11 |
| 47 | SwitchSpeed | 速度設定變更 | S6F11 |
| 48 | ChangeEC | EC 參數變更 | S6F11 |
| 49 | TrayFeedFinish | Tray Feed 完成 | S6F11 |
| 50 | AutoCleanFinish | Auto Clean 完成 | S6F11 |

#### 13. Site Mapping/UPH 事件 (CEID 51-54, 69)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 51 | SiteMappingStart | Site Mapping 開始 | S6F11 |
| 52 | SiteMappingEnd | Site Mapping 結束 | S6F11 |
| 53 | UPHRecordStart | UPH 記錄開始 | S6F11 |
| 54 | UPHRecordEnd | UPH 記錄結束 | S6F11 |
| 69 | SiteMappingStop | Site Mapping 停止 | S6F11 |

#### 14. ART 事件 (CEID 55-63)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 55 | InitialArtStart | ART 初始化開始 | S6F11 |
| 56 | TesterFT | Tester 切換至 FT | S6F11 |
| 57 | TesterRT | Tester 切換至 RT | S6F11 |
| 58 | ReadyForArt | ART 準備就緒 | S6F11 |
| 59 | ArtReceiveTrayOK | ART 接收 Tray 成功 | S6F11 |
| 60 | ArtReceiveTraySTART | ART 接收 Tray 開始 | S6F11 |
| 61 | ArtRTFinish | ART RT 測試完成 | S6F11 |
| 62 | ArtTrayFeedFinish | ART Tray Feed 完成 | S6F11 |
| 63 | ArtFTFinish | ART FT 測試完成 | S6F11 |

#### 15. Recipe/FTP 事件 (CEID 64-65, 124)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 64 | DownLoadRecipeByFTPOK | FTP 下載 Recipe 成功 | S6F11 |
| 65 | DownLoadRecipeByFTPNG | FTP 下載 Recipe 失敗 | S6F11 |
| 124 | SaveRecipe | Recipe 儲存 | S6F11 |

#### 16. OTD/Barcode 事件 (CEID 70-73)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 70 | BarcodeReaderEnter | Barcode 讀取進入 | S6F11 |
| 71 | OTDLock | OTD 鎖定 | S6F11 |
| 72 | OTDUnLock | OTD 解鎖 | S6F11 |
| 73 | MymessboxOK | 對話框確認 | S6F11 |

#### 17. 省電事件 (CEID 212-213)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 212 | PowerSavingStart | 省電模式開始 | S6F11 |
| 213 | PowerSavingEnd | 省電模式結束 | S6F11 |

#### 18. 安全門事件 (CEID 123, 234-235)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 123 | SafeDoorOnOff | 安全門開關變更 | S6F11 |
| 234 | SafetyDoorOpen | 安全門開啟 | S6F11 |
| 235 | SafetyDoorClosed | 安全門關閉 | S6F11 |

#### 19. Bundle 事件 (CEID 236-270)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 236 | LoadPortBundleArrived | Bundle 到達 LoadPort | S6F11 |
| 237 | LoadPortBundleRead | Bundle ID 讀取 | S6F11 |
| 239 | UnexpectedBundleIDRead | 非預期 Bundle ID | S6F11 |
| 241 | BundleCompleteProcessed | Bundle 處理完成 | S6F11 |
| 242 | BundleCompleteIDRead | Bundle 完成 ID 讀取 | S6F11 |
| 245-246 | BundleEnd_Auto1/IDREAD | Auto1 Bundle 結束 | S6F11 |
| 247-248 | BundleEnd_Auto2/IDREAD | Auto2 Bundle 結束 | S6F11 |
| 251-252 | BundleEnd_Auto3/IDREAD | Auto3 Bundle 結束 | S6F11 |
| 253-258 | BundleEnd_Auto4-6/IDREAD | Auto4-6 Bundle 結束 | S6F11 |
| 259-270 | BundleEnd_Fix1-6/IDREAD | Fix1-6 Bundle 結束 | S6F11 |

#### 20. AGV/Port 詳細事件 (CEID 271-284, 288)

| CEID | 事件 | 觸發時機 | 上報訊息 |
|:----:|------|----------|:--------:|
| 271 | LoaderTrayState | Loader Tray 狀態變更 | S6F11 |
| 272 | AGVSupplement | AGV 補料 | S6F11 |
| 273 | AGVLDUnLDStatus | AGV 上下料狀態 | S6F11 |
| 275 | Loader_Buffer_HasTray | Loader Buffer 有 Tray | S6F11 |
| 276 | Loader_Buffer_NoTray | Loader Buffer 無 Tray | S6F11 |
| 277-282 | OutputPort1-6BinCode | 出料 Port Bin Code | S6F11 |
| 283 | MaterialModeChange | 物料模式變更 | S6F11 |
| 284 | PortStateUpdated | Port 狀態更新 | S6F11 |
| 288 | MaximumOutputPortReport | 最大出料 Port 報告 | S6F11 |

#### 21. 警報事件 (Alarm)

| 訊息 | 觸發時機 | 說明 |
|:----:|----------|------|
| S5F1 | 警報發生/解除 | 機台故障或異常時主動上報 |
| S5F2 | Host 確認警報 | Host 回應 |

#### 上報機制程式碼範例

```cpp
// 觸發 CEID 事件上報
HGemPtr->SendCEID(SECS_EVENT.DoStart);      // 發送 S6F11
HGemPtr->SendCEID(SECS_EVENT.TestFinish);   // 測試完成
HGemPtr->SendCEID(SECS_EVENT.TrayIDChange); // Tray ID 變更

// 警報上報 (S5F1)
HGemPtr->ReportAlarm(AlarmCode, bIsJam, iDuplicateError, SubMessage, bReleaseAlm);
```

#### 上報統計

| 類別 | CEID 數量 | 上報訊息 |
|------|:---------:|:--------:|
| 操作按鍵 | 15 | S6F11 |
| Lot 週期 | 8 | S6F11 |
| 測試進度 | 5 | S6F11 |
| Tray/Port 滿盤 | 12 | S6F11 |
| Tray 退盤 | 16 | S6F11 |
| 通訊/狀態變更 | 5 | S6F11 |
| 模式切換 | 10 | S6F11 |
| 週期性/定時 | 5 | S6F11 |
| Carrier/Material | 12 | S6F11 |
| Port 狀態變更 | 15 | S6F11 |
| 品質/良率 | 6 | S6F11 |
| 完成事件 | 10 | S6F11 |
| Site Mapping/UPH | 5 | S6F11 |
| ART 事件 | 9 | S6F11 |
| Recipe/FTP | 3 | S6F11 |
| OTD/Barcode | 4 | S6F11 |
| 省電事件 | 2 | S6F11 |
| 安全門事件 | 3 | S6F11 |
| Bundle 事件 | 20+ | S6F11 |
| AGV/Port 詳細 | 12 | S6F11 |
| 警報 | - | S5F1 |
| **合計** | **288** | - |

---

## SEMI E30 Remote Control 合規性分析

> 參考：SEMI E30 7.5 Remote Control  
> 程式碼版本：`HT9011UC_Code_V3.33.899.0_20260323`

### 需求規格

SEMI E30 要求設備支援以下 7 項基本 Remote Command：

| 指令 | 功能 | SEMI 格式要求 |
|------|------|---------------|
| **START** | Host 下達開始生產 | 無 CPNAME/CPVAL |
| **STOP** | Host 下達安全停止生產 | 無 CPNAME/CPVAL |
| **PPSELECT** | Host 下達切換 Recipe | 允許 CPNAME=PPID |
| **PAUSE** | Host 下達安全暫停 | 無 CPNAME/CPVAL |
| **RESUME** | Host 下達解除 PAUSE 狀態 | 無 CPNAME/CPVAL |
| **LOCAL** | Host 下達切換至 ONLINE-LOCAL | 無 CPNAME/CPVAL |
| **REMOTE** | Host 下達切換至 ONLINE-REMOTE | 無 CPNAME/CPVAL |

> **SEMI E30 7.5.2.1**：除 PPSELECT 外，其他 Remote Command **不得** 包含 CPNAME 及 CPVAL（多 Port 設備除外）

### HT9045 合規性狀態

| # | 需求 | 狀態 | 實作名稱 | 備註 |
|---|------|:----:|----------|------|
| 1 | ECID 由 Host 修改 | ? | S2F15 | 完整實作 |
| 2 | START | ? | `START` | 需 bCanRemoteStart 或 bRCMDStart |
| 3 | STOP | ? | `STOP` / `PAUSE` | 共用處理 |
| 4 | PPSELECT | ? | `PP_SELECT` / `PP-SELECT` | 支援 CPNAME=PPID |
| 5 | PAUSE | ? | `PAUSE` | 呼叫 BtnPauseClick |
| 6 | RESUME | ? | **未實作** | 需新增 |
| 7 | LOCAL | ?? | `ONLINE_LOCAL` | 別名不符規格 |
| 8 | REMOTE | ?? | `ONLINE_REMOTE` | 別名不符規格 |
| 9 | 錯誤回傳非0/4 | ? | HCACK 1,2,3,5,6,7-10 | 符合規範 |
| 10 | Idle PAUSE | ?? | - | 需確認行為 |

---

### S2F15 ECID 修改 (uHGemHT9045.cpp)

#### 實作位置

```cpp
int HT9045Gem::S2F15_CheckNewEquipmentConstant();   // Line ~456 驗證 ECID
int HT9045Gem::S2F15_UpdateNewEquipmentConstant();  // Line ~700 更新 ECID
```

#### 支援資料型別

| 型別 | 型別碼 |
|------|:------:|
| UINT_1, UINT_2, UINT_4, UINT_8 | ? |
| INT_1, INT_2, INT_4, INT_8 | ? |
| ASCII | ? |
| BINARY, BOOLEAN | ? |
| FLOAT_4, FLOAT_8 | ? |

#### EAC 回傳值 (S2F16)

| EAC | 說明 |
|:---:|------|
| 0 | Acknowledge (成功) |
| 1 | Denied, at least one constant does not exist |
| 2 | Denied, busy |
| 3 | Denied, at least one constant out of range |

---

### 缺失項目修改建議

#### 1. 新增 RESUME 指令

**插入位置**：`uHGemHT9045.cpp` 約 Line 1175 (PAUSE/STOP 之後)

```cpp
else if(S.AnsiPos("RESUME")==1)
{
    // 解除 PAUSE 狀態
    if(bPauseByHost==true)
    {
        bPauseByHost=false;
        fMain->BtnPauseClick(fMain);  // 恢復運行
        HCACK=0;
        RecordProcess("SECS GEM RCMD : RESUME");
    }
    else
    {
        HCACK=1;  // 非 PAUSE 狀態
    }
}
```

> **Note**: `bPauseByHost` 變數已存在於 `SECSGEM.cpp` (Line 98)

#### 2. 新增 LOCAL/REMOTE 別名

**修改位置**：`uHGemHT9045.cpp` Line ~1771

```cpp
// 修改前
else if(S.AnsiPos("ONLINE_LOCAL")==1)

// 修改後 (符合 SEMI E30)
else if(S.AnsiPos("LOCAL")==1 || S.AnsiPos("ONLINE_LOCAL")==1)
```

```cpp
// 修改前
else if(S.AnsiPos("ONLINE_REMOTE")==1)

// 修改後 (符合 SEMI E30)
else if(S.AnsiPos("REMOTE")==1 || S.AnsiPos("ONLINE_REMOTE")==1)
```

---

### HCACK 回傳值對照表 (完整)

| 情境 | HCACK | 說明 |
|------|:-----:|------|
| 成功執行 | 0 | Acknowledge |
| 無效指令 / 未啟用 | 1 | Denied, invalid command |
| 機台有 IC / 執行中 | 2 | Denied, cannot perform now |
| 參數錯誤 | 3 | Denied, parameter error |
| 機台忙碌 | 4 | Acknowledge, will complete later |
| RunStartMode 禁用 (RT) | 5 | Equipment-specific error |
| Recipe 不存在 / Tray 有 IC | 6 | Equipment-specific error |
| FTP 控制中 | 7 | Equipment-specific error |
| LIST 結構錯誤 | 8 | Equipment-specific error |
| 參數名稱錯誤 | 9 | Equipment-specific error |
| LIST 類型錯誤 | 10 | Equipment-specific error |

---

### Remote Command 測試案例

| 測試項目 | 預期結果 |
|----------|----------|
| START (Idle, bCanRemoteStart=true) | HCACK=0, 機台開始 |
| START (Running) | HCACK=1 |
| STOP (Running) | HCACK=0, 安全停止 |
| STOP (Idle) | HCACK=1 |
| PAUSE (Running) | HCACK=0, 暫停 |
| RESUME (After PAUSE) | HCACK=0, 解除暫停 |
| RESUME (Not PAUSE) | HCACK=1 |
| PP_SELECT (Valid PPID) | HCACK=0, 切換成功 |
| PP_SELECT (Invalid PPID) | HCACK=6 |
| PP_SELECT (Running/有 IC) | HCACK=4 |
| LOCAL / ONLINE_LOCAL | HCACK=0, 切換 ONLINE-LOCAL |
| REMOTE / ONLINE_REMOTE | HCACK=0, 切換 ONLINE-REMOTE |

---

## 參考資料

| 標準 | 名稱 | 說明 |
|------|------|------|
| SEMI E5 | SECS-II | 訊息內容標準 |
| SEMI E30 | GEM | 設備行為模型 |
| SEMI E37 | HSMS | 高速通訊協定 |
| SEMI E40 | Process Job | 製程作業管理 |
| SEMI E87 | Carrier Management | 載具管理 |
| SEMI E94 | Control Job | 控制作業管理 |

---

## 版本歷程

| 版本 | 日期 | 說明 |
|------|------|------|
| 1.0 | - | 初版：基本概念與實作方式 |
| 2.0 | 2026-03-27 | 大幅擴充：Stream & Function 指令總表、HSMS 協議、GEM 狀態模型 |
| 2.1 | 2026-03-27 | 補充 HT9045 實作：CEID 288 事件定義、SVID/ECID 完整註冊範例、S2F42 Remote Command 處理、AMKOR LOT_START 支援 |
| 2.2 | 2026-03-27 | 補充 HSMS 逾時參數預設值、擴充 HCACK 回應碼 (7-10)、完整 Remote Command 列表 (35+ 指令) |
| 2.3 | 2026-03-27 | 新增 SEMI E30 Remote Control 合規性分析、S2F15 ECID 修改詳情、7 項基本指令合規狀態、缺失項目修改建議 |
| 2.4 | 2026-03-27 | 整合 HT9045_SECS_GEM_CEID.md：功能支援總覽、完整 CEID 分類列表、32 項 S/F 實作表、資料型態對照、關鍵整合點 |
| 2.5 | 2026-03-27 | 整合 SECS-GEM-Feature-Support.md：Recipe 管理詳細 (S7Fx/ACKC7)、時間同步格式、SEMI E84 訊號定義、Carrier/Port 事件清單、Material 追蹤、GEM300 Auto Run 說明 |
| 2.6 | 2026-03-27 | 新增「主動上報時機點」章節：288 個 CEID 事件分類表格、12 類觸發時機、上報機制程式碼範例 |
| 2.7 | 2026-03-27 | 依專案程式碼驗證主動上報時機點：新增完成事件/Site Mapping/UPH/ART/Recipe/OTD/安全門/Bundle/AGV/省電共 10 類 CEID，總分類增至 21 類 |

