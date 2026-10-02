# HT9045 OLP 通訊協定與模組結構

## 目錄
- [封包格式](#封包格式)
- [TfAutomation 模組結構](#tfautomation-模組結構)
- [TCP 連線架構](#tcp-連線架構)
- [ProcessBuffer 指令分派邏輯](#processbuffer-指令分派邏輯)
- [狀態查詢指令（INQUIRE 系列）](#狀態查詢指令inquire-系列)
- [重要全域變數](#重要全域變數)
- [機台狀態回傳](#機台狀態回傳)
- [DoDLRequest Setup File 解壓流程](#dodlrequest-setup-file-解壓流程)

---

## 封包格式

HT9045 使用自定義 ASCII 封包，控制字元：

| 字元 | 值 | 用途 |
|------|-----|------|
| STX | 0x02 | 封包開始 |
| SOH | 0x01 | 欄位分隔符 |
| ETX | 0x03 | 封包結束 |

**封包結構：**
```
STX + Command + SOH + V_Total + [SOH + Data[0]] + [SOH + Data[1]] + ... + ETX
```

其中：
- `Command`：命令字串（如 `START_REQUEST`, `PAUSE_REQUEST`）
- `V_Total`：後續資料欄位數量（字串形式）
- `Data[i]`：第 i 個資料欄位

**回傳封包（CommandProcess 產生）：**
```cpp
void CommandProcess(AnsiString CMD, AnsiString V_TOTAL, AnsiString *Data, bool Standard, int iHandle)
// Standard=true → 送 OLPServer（Client 端連線）
// Standard=false → 送 OLPClient（Server 端連線）
```

---

## TfAutomation 模組結構

主檔：`automation.cpp`，表單：`TfAutomation`

| 成員 | 類型 | 用途 |
|------|------|------|
| `OLPServer` | TServerSocket | Handler 作為 Server，接收 Agent 指令 |
| `OLPClient` | TClientSocket | Handler 作為 Client，主動連接外部 MES/OLP |
| `CommandBuffer` | TList* | 待發送的事件/報告佇列 |
| `CommandClientReplyBuffer` | TStringList* | Log/Debug 用 |
| `bOneCycle` | bool | One Cycle 模式旗標 |
| `bCleanOut` | bool | Clean Out 旗標 |
| `TestMode` | int | Tester 模式（`SetTesterMode` 設定） |
| `iSocketHandle` | int | 當前連線 Socket Handle |

**外部關鍵變數（extern）：**
```cpp
extern bool bLockByServer;   // Safety Interlock Lock 狀態
extern bool SoftStart;       // 軟體啟動旗標（目前已 comment out）
extern bool SoftStop;        // 軟體停止旗標
extern bool SystemStart;     // 機台運行狀態
extern bool InitialOK;       // 初始化完成旗標
```

---

## TCP 連線架構

```
┌──────────────────────────────────────────────┐
│              HT9045 Handler PC               │
│                                              │
│  ┌─────────────────────────────────────┐     │
│  │          TfAutomation               │     │
│  │                                     │     │
│  │  OLPServer (TServerSocket)          │◄────┼──── Agent (connects to Handler)
│  │  → 接收 Agent 下達的控制指令        │     │
│  │                                     │     │
│  │  OLPClient (TClientSocket)          │────►├──── MES / 外部 OLP Server
│  │  → 主動連接 (CC_MTI, CC_PTI 使用)   │     │     (如 HTML Monitor, MTI, PTI)
│  └─────────────────────────────────────┘     │
└──────────────────────────────────────────────┘
```

**OLPServer 特性：**
- 每次收到資料觸發 `OLPServerClientRead()`
- 解析封包，累積至 `ReceiveString`，找到 STX~ETX 完整封包後呼叫 `ProcessBuffer()`
- 若 `ReceiveString.Length() > 1024` 則強制清空（防止長封包死鎖）

**OLPClient 特性：**
- 用於 CC_MTI / CC_PTI / CC_HTML_Monitor 等客戶的雙向模式
- 連線來自 `btnConnectClick()`
- 重連計數器：`iClientReConnect`

---

## ProcessBuffer 指令分派邏輯

`automation.cpp ProcessBuffer()` 是 TCP 指令的總入口，採用 if-else 鏈分派：

```
ProcessBuffer(Buffer, iHandle)
  │
  ├── if Command.Pos("INQUIRE") != 0 → 狀態查詢分支（L989~L1493）
  │
  ├── ON_LINE_REQUEST → DoOnLineReply() → ON_LINE_REPLY
  ├── INITIATE_REQUEST → 設定時間 → INITIATE_REPLY
  ├── PAUSE_REQUEST → bLockByServer=true; SoftStop=true → PAUSE_REPLY
  ├── RESUME_REQUEST → bLockByServer=false → RESUME_REPLY
  ├── START_REQUEST → fMain->Start() (HALT才有效) → START_REPLY
  ├── CLEAR_REPORT_REQUEST → DoClearReportRequest() → CLEAR_REPORT_REPLY
  ├── PP_UL_REQUEST → DoULRequest() → PP_UL_REPLY
  ├── PP_DL_REQUEST → ProcessBuffer1() → DoDLRequest()
  ├── CATEGORY_REQUEST → SetCategory() → CATEGORY_REPLY
  ├── BINDEFINE_REQUEST → SetBinDefine() → BINDEFINE_REPLY
  ├── MAPPING_REQUEST → SetMapping() → MAPPING_REPLY
  ├── TEMPERATURE_REQUEST → SetTemperature() → TEMPERATURE_REPLY
  ├── LotInfo_REQUEST → SetLotInfo() → LotInfo_REPLY
  ├── StartMode_REQUEST → SetStartMode() → StartMode_REPLY
  └── [其他 Request 指令...]

  #ifdef DEBUG_DUTONOFF only:
  ├── CLEANOUT_REQUEST → bCleanOut=true
  ├── HOMEANDSTART_REQUEST → DoHomeAndStart()
  └── ONECYCLE_REQUEST → DoOneCycle(); bOneCycle=true
```

---

## 狀態查詢指令（INQUIRE 系列）

當 Command 包含 "INQUIRE" 字串時進入查詢分支，格式：
```
{CommandName}_INQUIRE → Get{CommandName}() → {CommandName}_REPLY
```

相關查詢指令（部分）：

| 查詢指令 | 回傳函式 | 回傳內容 |
|----------|---------|---------|
| `LotInfo_INQUIRE` | `GetLotInfo()` | `LotID, OperatorID, FT/RT` |
| `StartMode_INQUIRE` | `GetStartMode()` | `LastSet.iRunStartMode` |
| `MAIN_STATUS_INQUIRE` | `GetMainStatus()` | `RUN/PAUSE/HALT/LOCK` |
| `SETUP_FILE_INQUIRE` | `GetSetUpFileName()` | 目前 Setup File 名稱 |

---

## 重要全域變數

| 變數 | 類型 | 位置 | 說明 |
|------|------|------|------|
| `bLockByServer` | bool | automation.cpp extern | Safety Lock 狀態（PAUSE/RESUME 控制） |
| `SoftStop` | bool | extern | 軟體停止旗標 |
| `SystemStart` | bool | extern | 機台運行中旗標 |
| `InitialOK` | bool | extern | 初始化完成旗標 |
| `bOneCycle` | bool | TfAutomation member | One Cycle 模式旗標 |
| `bCleanOut` | bool | TfAutomation member | Clean Out 旗標 |
| `bFTPDownloadSetupFile` | bool | auto9045.cpp | Setup File 已 FTP 下載旗標 |

---

## 機台狀態回傳

`GetMainStatus()` (`auto9045.cpp`)：

```cpp
AnsiString GetMainStatus()
{
    AnsiString S = fMain->palMainStatus->Caption.UpperCase();
    if(S=="RUNNING") S="RUN";
    if(S=="RUNNING" || S=="PAUSE" || S=="HALT" || S=="LOCK")
        return S;
    else
        return "RUN";  // 未知狀態預設回傳 RUN
}
```

**機台狀態表：**

| palMainStatus | GetMainStatus() 回傳 | 說明 |
|---------------|---------------------|------|
| RUNNING | RUN | 機台運行中 |
| PAUSE | PAUSE | 暫停中 |
| HALT | HALT | 停止（可接受 Auto Start） |
| LOCK | LOCK | 鎖定中（bLockByServer=true 時） |
| 其他 | RUN | 預設 |

---

## DoDLRequest Setup File 解壓流程

`auto9045.cpp DoDLRequest(sDLFileName)` 執行步驟：

```
1. 建立目錄 DataPath + sDLFileName (若不存在)
2. 嘗試備份現有保護參數（下列項目依 Security_new.def 判斷是否保留）
   - Temperature User Offset (CH1~CH10)
   - Index Heating Mode
   - Contact 高度（Test Arm1/2 Pick Up/Contact/Drop/Place）
   - Torque Control（Pin Number, Force Per Pin）
   - Contact Mode 各項參數
   - HotPlate.Data
   - UdUld.Data（Loader/Unloader 設定）
   - ArmCondition.Data
   - HandlerCondition.Data（Shuttle Mode）
3. 執行 7z.exe 解壓 DataPath + sDLFileName.zip → DataPath + sDLFileName\
4. 刪除 .zip 暫存
5. 依 Security_new.def 判斷是否還原各保護項目
6. bFTPDownloadSetupFile = true

回傳：1 (成功), 0 (失敗)
失敗訊息：ShowErrorMessage("WAR1686") → "UnZip {filename}.zip Fail."
前提：d:\HT9045\7z.exe 必須存在（若不存在會從 Program Files\7-Zip 複製）
```

**Security_new.def 保護項目（路徑：AuthPath + "Security_new.def"）：**

| Section | Key | 預設 | 說明 |
|---------|-----|------|------|
| Network | Temp OffSet | true | 不覆寫溫度 Offset |
| Network | Index Heat Mode | true | 不覆寫 Index 加熱模式 |
| Network | Test Arm | false | **覆寫** Contact 高度 |
| Network | Torque Control | true | 不覆寫扭力控制 |
| Network | Contact Mode | true | 不覆寫 Contact 模式 |
| Network | HotPlate | true | 不覆寫 HotPlate.Data |
| Network | Load Unload | true | 不覆寫 UdUld.Data |
| Network | ArmCondition | true | 不覆寫 ArmCondition.Data |
| Network | Shuttle Mode | true | 不覆寫 Shuttle 模式 |
