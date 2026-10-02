# Tester 通訊併入 V906 轉換計畫（GPIB／DIO／RS232／TCPIP 四種介面，獨立執行緒 + 瀏覽器畫面）

> 本檔為 `ht9045-gpib-bridge` 的 reference。工作副本同步放在 V906 樹
> `docs/GPIB_20260926_INTEGRATION_PROPOSAL.md`；兩邊內容相同，改動時兩邊一起改。

- 日期：2026-09-26（上午 GPIB 提案；下午依使用者裁決擴成四種介面）
- 來源
  - GPIB：`D:\GPIB9045\GPIB_Code_32Site_V12.13.905.0_20260525`（H9046_32GPIB.exe，BCB6，Big5，Main.cpp 8,380 行）
  - RS232／DIO(TTL 板)：`D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`（RS232Standard.exe，BCB6，Big5，MainForm.cpp 3,746 行）
  - TCPIP：golden Handler 內建 `Interface/TesterTCP.cpp`（1,118 行）＋ `TesterTCP.h`
  - DIO：**新機台一律走 RS232Standard 的 TTL 板**（使用者 20260926）；golden `atester.cpp` 內 `TTL_MODE && TTL_CARD_TYPE<2`「直接讀 DIO 卡」分支已不適用，不移植
  - 設定：golden `cTesterIF.cpp`（1,570 行）／`cTesterIF.h`（TFTestIF，Tester I/F 設定畫面）
- 目標樹：`D:\HT9045\HT9011UC_Cpp_V3.33.906.0`（wb_serve.exe，C++17，MinGW 6.3，單一執行檔）
- 使用者裁決（20260926，全文 §7）：GPIB 必須是獨立執行緒；四種介面一併整合；設定統一在 cTesterIF；TMyDutPanel 與畫面共用。
- 狀態：計畫，尚未動工。

---

## 0. 一句話

四種 Tester 介面在 golden 是「三支程式＋一個內建模組」各自存讀設定、各自畫面、用同一套 `MessageDef.h` 封包經 `WM_COPYDATA` 跟 Handler 講話。
併入 V906 後變成 **一個 `TesterComm/` 層**：**一條 TesterComm 執行緒**（生產中只會用一種通訊，GPIB／RS232／TCPIP 共用，依 `iTestType` 只啟動一個引擎）、同一個同步信箱、同一個 Handler 端 `OnMyCopyMsg`、
設定只在 `TestIF_File`（cTesterIF）一處存讀，畫面是 `testercomm.html` 一頁多分頁，**32 站面板四種模式共用同一個 `makeDutPanel`**。

---

## 1. 現況盤點

### 1.1 四種介面在 golden 的形狀（先看這張）

| 介面 | `TestIF_File.iTestType` | 誰在跑通訊 | Handler 怎麼接 | 設定存在哪 | 畫面 |
|---|---|---|---|---|---|
| GPIB | `GPIB_MODE`=1 | 獨立 exe `H9046_32GPIB.exe`（NI gpib-32） | `WM_COPYDATA` ⇄ `TfMain::OnMyCopyMsg`；`bFind` 靠 `FindWindow("TfMain",…)` | `D:\GPIB9045\system\general.ini`（`LAST_GENERAL_SET` 56 欄）＋ Handler 的 `Tester.Data` | exe 自己的 `TSerialPoll` 表單 |
| RS232 | `RS232_MODE`=2 | 獨立 exe `RS232Standard.exe`（SPComm `TComm`） | 同上 | `D:\RS232Standard\System\Setup.ini`（COM 參數）＋ Handler `Tester.Data`；golden `cTesterIF.cpp:496 CheckRs232StandardIni()` 把 Handler 值**回寫** Setup.ini 再關掉小程式重開 | exe 自己的 `TfRS232Main` 表單 |
| DIO／TTL 板 | `TTL_MODE`=0 且 `TTL_CARD_TYPE`=2/3 | **同一支 `RS232Standard.exe`**，模式 `iUseRS232Mode>=InterfaceType_TTL(1000)`，接 1～2 塊 TTL 板（`@WSOTS…`／`@WINIT…`／`RBIN` CRC 協定） | 同上，多 `MSG_CMD_State_TTL`／`MSG_CMD_Command_TTL` | Setup.ini `[COMPort_TTL]`／`[COMPort_TTL_2]`＋ `Gerneral.ini [System] TTL_CARD_TYPE/TTL_CARD_USE_ADDRESS` | 同上（TTL 分頁） |
| DIO／直接讀卡（**不移植**） | `TTL_MODE`=0 且 `TTL_CARD_TYPE`<2 | golden 由 Handler 自己在 `atester.cpp` 讀 DIO 線；**新機台不再有這種接法**（使用者 20260926），C++ 線只認 TTL 板 | — | — | — |
| TCPIP | `TCP_IP_MODE`=3 | **Handler 內建** `TfTesterTCP`（`TClientSocket`），`LastSet.iTester==ON_LINE` 才連 | 無 IPC，直接呼叫 | `Tester.Data`（`asTester_Address`／`iTester_Port`） | Handler 的 `TesterTCP.dfm`（2,955 行：32 站面板、Log、OS 報表） |

三個要點：
1. **GPIB 與 RS232 兩支 exe 的 `MessageDef.h` 逐行相同**（diff 0 行），`MessageDef.cpp` 只差版本字串（GPIB `12.13.905.0`，RS232 `12.13.884.0`）；`MyDutPanel.cpp` 只差 Bin 下拉項目。
2. **TCPIP 本來就在 Handler 行程內**，沒有 IPC 要模擬；要做的是解閘與接泵。**DIO 在 C++ 線就是 RS232Standard 的 TTL 板模式**（`TTL_CARD_TYPE` 2/3），與 RS232 共用同一個引擎；`atester.cpp` 的直接讀卡分支不翻。
3. **設定分散是歷史包袱**：三處存讀＋一段回寫同步碼。使用者 20260926 裁決：**統一在 cTesterIF 處理**（§7 裁決 6）。

### 1.2 GPIB 橋接程式（V12.13.905.0）

| 項目 | 事實 | 位置 |
|---|---|---|
| 進入點 | WinMain 建 4 個表單，`CreateMutex("MyMutexGPIB")` 防多開 | `H9046_32GPIB.cpp` |
| **原本就有獨立執行緒** | `TMyThread::Execute`：每 1 ms `Synchronize(Process)` → `ProcessAddress()` + `ProcessMessage()` | `Unit2.cpp:29-70` |
| 輪詢核心 | `ProcessMessage()` 狀態機 → `TestGPIB()`（Task 1/100/7000…，`ibwait(…,0)` 非阻塞） | `Main.cpp:3950`, `1455` |
| 指令派發 | `ProcessStatusString()` 1,300 行 if-else ＋ RCMD／RFMD／Delta Castle 三分支 | `Main.cpp:5133`, `5117`, `5054`, `7091` |
| 回寫 Tester | `MyGPIBWrite()`：等 TACS，最多 20 次 × 100 ms（**最長阻塞 2 秒**） | `Main.cpp:4631` |
| 送 Handler | `SendMSG_CMD()` → `SendMessage(HMountWnd, WM_COPYDATA)`（**同步**） | `Main.cpp:4908` |
| 收 Handler | `OnMyCopyMsg()` 780 行 | `Main.cpp:3136-3915` |
| NI 驅動 | 只用 9 個函式 `ibfind ibrsc ibpad ibtmo ibwait ibrd ibrsv ibwrt ibstop`；`BorlandC_gpib-32.obj` 是 OMF，MinGW 不能連 | `Decl-32.h` |
| 機型清單 | 9045GPIB／9046GPIB／9046_32GPIB／9045GPIB_12Site／2601GPIB／9055GPIB／502GPIB／7080GPIB／1032GPIB／**9050GPIB（20260926 加）**；`MachineType` 0/1/2 = GpibString 補位 2/4/8 hex | `Main.cpp:630`, `4455`, `3091`, `4780` |
| 畫面 | 13 顆 ibsta LED、狀態列、32 站 `TMyDutPanel`、手動 Start、Timeout、SCK ART 面板、RS232(AMD) 分頁、Version | `Main.dfm` |
| **內建的 RS232 線（不是 Tester 測試通訊）** | `RS232.cpp`（665 行，`TfRS232Main`，與 RS232Standard 的主表單**同名但不同物**）＋ `Main.cpp` 的 `CommAMD`／`CommAMD2`：AMD／ATC 溫控與 HT-505 的 STX/LRC/ETX 指令（`TSW000n`、`RDY0000`、`1038,4,…`；`SendMode`／`QueryRDY`／`btnSendTemp`），走 `MSG_CMD_AMDRS232Connect`（CMD 92）。使用者 20260926 提醒：**它做的事跟 RS232Standard（測試工作：SOT／BIN）不一樣**，翻譯時留在 `GpibEngine` 內當 GPIB 的附屬通道，不併進 `Rs232Engine` | `RS232.h`、`Main.cpp:6714-6830` |

### 1.3 RS232／DIO(TTL 板) 橋接程式（Rev12.13.902.0）

| 項目 | 事實 | 位置 |
|---|---|---|
| 進入點 | WinMain 建 2 個表單，`CreateMutex("MyRS232Standard")` | `RS232Standard.cpp` |
| 執行緒 | **沒有**專屬執行緒；全靠 `Timer1`（300 ms）＋ `TComm` 的 `OnReceiveData` 事件（SPComm 的讀執行緒回呼） | `MainForm.cpp:1707`, `1388`, `1407`, `2914` |
| 找 Handler | `ProcessHandlerConnect()` 每秒 `FindWindow`，清單與 GPIB 端同一份（**尚未加 HT-9050**） | `MainForm.cpp:580-653` |
| 三種模式 | `iUseRS232Mode`：`InterfaceType_Standard`=0、`InterfaceType_SLT`=1、`InterfaceType_TTL`=1000+`iDioMode`；由 Handler 經 `MSG_CMD_TesterMode`（`GPIBBin` 欄）指定 | `MainForm.cpp:23-25`, `1165-1207` |
| Standard 協定 | ASCII 控制碼：`_ENQ_`(05)/`_ACK_`(06)/`_STX_`(02)…`_ETX_`(03)；指令 `CF`／`CE`（開測，回 site 清單）／`BA`（Bin 結果 `site,order,bin;`）／`ST`／`AB`／`CD`／`CN`／`CB`／`BARCODE?`／`GET2DID?`／`CZ id?`／`CZ which?`／`CZ status?`／`CZ testerbin?`／`CZ all masstemp?`／`CZ sitemap?`／`CZ jam?`／`CZ soaktime?`／`CZ doublecontact?` | `GetAnalysisString` 3159、`DoRevCommand` 3227-3725 |
| TTL 板協定 | `@[00|01]WINIT<47 碼>CRC#`（初始化：TS+5V、Bin 模式、SOT/Data/EOT/DUT 邏輯、寬度、Bin timeout）、`@[00]WSOTS<8 位 site>CRC#`（開測）、`@[00]WCSOT`（清 SOT）、回 `RBIN`（16+站號 bytes）；1 塊板 8 站／2 塊板各 4 站；`Err SOF/CRC/EOF/CMD/SOT/DAT/SIT/BIN` 錯誤碼；3 秒無回應報 `MSG_CMD_Version` 帶錯誤字串 | `OnMyCopyMsg` 902-941、1208-1376；`CommTester_TTLReceiveData` 1407-1646；`Timer1Timer` 1736-1785 |
| 送 Handler | `SendMSG_CMD()`／`SendResultFinish()`（`MSG_CMD_NONE` 帶 `Result[32]`；`BA without CE` → `MSG_CMD_BinonWithoutFullsite`；關 site 有 bin → `MSG_CMD_CloseSiteHaveBin`，全 999） | `MainForm.cpp:655-766` |
| 收 Handler | `OnMyCopyMsg()` 620 行：`MSG_CMD_NONE`（開測：`Site[]`→`iStart[]`、barcode→`labOcr`）、`ChangeGpib`、`TesterMode`、`State_TTL`、`Command_TTL`、`MachineState`/`TesterBin`/`SoakTime`/`JamCode`/`SiteMap`/`AllMassTemp`/`DoubleContactCount`（給 `CZ *?` 查詢用的快取字串）、神盾雙臂 `SwitchArm`… | `MainForm.cpp:768-1386` |
| TCP 模擬 | `#ifdef SOFT_SIMULTE` 用 `uSocketServer`（`uSocketServerClient.cpp` 417 行）把 RS232 資料改走 TCP，給模擬器接 | `MainForm.cpp:379-383`, `2168`, `3727` |
| 設定 | `D:\RS232Standard\System\Setup.ini`：`[COMPort]` CommName/BaudRate/ByteSize/StopBits/Parity、`[Detail Settng] ReadIntervalTimeout`、`[SystemSetup] iTesterMode/iRevCycleClear/bCheckClosedSiteHasBin`、`[COMPort_TTL]`/`[COMPort_TTL_2]`；另讀 Handler `Gerneral.ini [System] TTL_CARD_TYPE/CUSTOMER_CODE`、`[Version] Model/Machine ID` | `MainForm.cpp:36-37`, `2496-2562`, `419-423` |
| Log | `TMyStringList`（`D:\RS232Log\LOG`，每 2 小時一檔，`Date,Time,Action,Message,Hex`）＋ `D:\RS232Log\BinData` | `MainForm.cpp:361`, `2607` |
| 畫面 | Standard（COM Log／Bin Log／Setup／Simulate）、AVAGO、SPRD（4 site 各自 COM）、BinCode、TTL（Bin Log／Setup／Simulate）、Log、Version；32 站 `TMyDutPanel` | `MainForm.dfm` |
| 版本 | `RS232Version`／`GPIBVersion` 在 `MessageDef.cpp` 為 `12.13.884.0`（比 GPIB 端 905 舊；Handler `bGpibRS232Error` 只比對主次版 12.13） | `MessageDef.cpp` |

### 1.4 TCPIP（Handler 內建 TfTesterTCP）

| 項目 | golden | V906 現況 |
|---|---|---|
| 連線 | `TimerTCPIPConnectTimer`：`LastSet.iTester==ON_LINE` 才連，30 秒重試；`ClientSocket_TCPIP->Open()`，`bConnectOK` | `Interface/TesterTCP_Socket.cpp`（791 行）已翻譯，含 connect/disconnect/error/read、`SendTCPIPCommand`、log |
| 協定 | `TimerProcessTCPDataTimer`（349-552）：`WORKFILE_OK/FAIL`、`BARCODE?`→`BARCODE:…`、`ECHOCODE:`→`ECHOCODEOK/NG`、`Test Arm?`、`TempArm?`、`BINON:`→`iBin[][]`＋`ECHO:`、`ECHOOK`（`bEcho=true`）、`GETOSSETUP`；離線 `SimulateBin()` | 已翻譯（AI(W906-TesterTCPTimer) 20260720），`DESIGN_TesterTCP_TimerProcessTCPDataTimer.md` |
| OS 報表族 | `CopyOSTestResult`／`PlaceOSTestResultToTray`／`ProcessOSPrint`／`ProcessOSTrayData`／`CopyRecipeTo/FromTester`（CC_JSCC_OS 專用） | `Interface/TesterTCP.cpp` 部分翻譯；`forms/fTesterTCP.cpp` 8 個 gate（T-1～T-7：ctor 32 站 widget 鏡射、FormShow、OS 報表寫檔） |
| 啟動 | `fTesterIF.cpp:829-836` 依 `iTestType` 開關兩個 Timer；`main.cpp:4772` START 前檢查 `bConnectOK` | `forms/fTesterIF.cpp` 有同樣開關碼，但 **wb_serve 沒有任何地方驅動這兩個 Timer**（沒有 TTimer 泵） |
| 結果消費 | `atester.cpp:1426/2958/3031/3658` `iTestType==TCP_IP_MODE` 走與 GPIB 相同的 `bEcho`／`tTestResult` 路 | `GetTesterResult` 是回 false 的 stub（`atester.cpp:2541-2548`），全部 mode 一起閘 |

### 1.5 DIO（C++ 線 = RS232Standard 的 TTL 板模式）

- golden `atester.cpp:1439`：`else if(TestIF.iTestType==TTL_MODE && TTL_CARD_TYPE<2)` 是 Handler 自己驅動 SOT／等 EOT／讀 Bin 位元的「直接讀 DIO 卡」分支。**使用者 20260926 裁決：新機台已不適用，要走 RS232Standard 那邊；這條分支不移植**（V906 `atester.cpp` 解閘時保持 gate，帳本記「刻意不翻」）。
- 活的 DIO 路徑：`:1815`、`:3710`、`:3794`、`:6134` 的 `TTL_MODE && (TTL_CARD_TYPE==2||3)` → 經 `MSG_CMD_Command_TTL`／`MSG_CMD_State_TTL` 交給 RS232Standard 的 TTL 板協定（§1.3）。
- `TTL_CARD_TYPE` 來自 `Gerneral.ini [System]`（`database.cpp:1094`，2=一塊板、3=兩塊板），`TTL_CARD_USE_ADDRESS`（`:1099`）決定板子是否帶站號。這兩個是機台硬體屬性，留在 `Gerneral.ini`。

### 1.6 設定：cTesterIF（TFTestIF）與 TestIF_File

- golden `cTesterIF.h`：`rgInterfaceType`（介面選擇）＋ 分頁 `tsDio`（`grpTTL`／`grpTTLSetting`）、`tsGpib`（`grpGPIB`／`grpGPIBType`）、`tsRs232`（`pgcRS232`：`tsRS232Setting` 的 Parity/StopBit/BitLength/BaudRate、`tsSLTSetting`；`gbRs232BinCount`）、`tsTCPIP`、`tsRT`、`tsEQC`、`rg2DID_Format`、`grpTestingStopTime`、`gbInitialDelay`。
- `SYSTEM_TEST_IF`（`cprod.h`）已有：`iTestType`(1669)、`iDioMode`(1670)、`iGpibMode`(1672)、`iGpibAddress`(1673)、`iRs232Mode`(1676)、`iRs232MaxBinCount`(1677)、`Rs232_Data.{Baud_Rate,Bit_Length,Stop_Bit,Parity}`、`asTester_Address`(2403)、`iTester_Port`(2404)、`bEnableBarCode`、`bOcrFunction`、`bTTLUseASEJPMode`。
- **原本每支程式各自存讀**（使用者 20260926）：GPIB 的 `general.ini` 56 欄、RS232 的 `Setup.ini`、Handler 的 `Tester.Data`；golden 用 `CheckRs232StandardIni()` 把 Handler 的 COM 參數回寫 Setup.ini 後 `CloseGpibProgram()` 重開小程式（`cTesterIF.cpp:496-558`，`:945` 在存檔時呼叫）。GPIB 端沒有這種同步，`general.ini` 的 56 欄（timeout、`bGPIBWriteWithout_r_n`、`iMyGpibWriteRetry`…）只能在小程式畫面改。
- V906：`forms/fTesterIF.cpp` 1,760 行，20 個 gate；dfm2rc 已產出 `cTesterIF` 的 layout／ids／events；web 端有 `page/` 對應表單（FW 戰役範圍）。

### 1.7 既有先行設計與跨專案規則（來自兩個子專案的 agent／skill，使用者 20260926 指示參考）

| 來源 | 內容 | 對本計畫的意義 |
|---|---|---|
| `D:\GPIB9045\.github\skills\gpib-rs232-merge`（20260409） | 曾把 GPIB＋RS232 併成一支 BCB 程式 `GPIB_RS232_Code_32Site_V12.13.900.0_20260331`（樹本身目前不在磁碟上，設計留在 skill）：`g_iTestType`（TTL/GPIB/RS232 三值）取代 `bGpibMode`；Handler 用 `HHandler2Gpib.iLotStatus` 欄位**複用**傳 `iTestType`；log 統一到 `D:\GPIBLOG\{Log,RS232_Log,RS232_BinLog,RS232_BinLog_TTL}`；RS232 log 同步進 GPIB log 視窗 | **V906 直接沿用這套分流原則**：`TesterCommHub` 以 `TestIF_File.iTestType` 分流（DD-001：用 int 不用兩個 bool），`iTesterMode` 只在 GPIB 內有意義（DD-001 的職責分離）。三個 bug 要當 ctest 案例：BUG-001 `iLotStatus` 漏設→POD 預設 0 被當 TTL（行程內改成直接讀 `TestIF_File.iTestType`，不再靠封包欄位帶）；BUG-002 FormDestroy 懸空指標（引擎物件用 RAII）；BUG-003 Timer 先於 `MY_DUT_PAL` 初始化（引擎建構完成前執行緒不得啟動） |
| `D:\.github\skills\gpib-ht9045-sync` | 三組定義必須三邊一致：`CC_xxx`（MachineType.h ↔ cmydef.h）、`MSG_CMD_*`（MessageDef.h/.cpp）、`enum eTestMode`（MachineType.h ↔ GPIB 端 MessageDef.h 的獨立 enum）；同步歷史 20260401 加 `_8Site2X4N=10` 後續值 +1、23 個 CC_、24 個 ATC/AutoZ MSG_CMD（180~203） | 併入 V906 後 **這一類「三邊不一致」的 bug 結構性消失**：只有一份 `MachineType.h`／`MessageDef.h`。但 BCB 線（§7 裁決 5）仍要照 sync skill 維護；帳本（P0）要記兩線各自的版本 |
| `D:\GPIB9045\.github\agents\GPIB9045.agent.md` | Big5 原始檔禁轉 UTF-8、治理文件 UTF-8 無 BOM、BCB6 無 C++11、臨時腳本放 `D:\AI_TempFile\`；skills：`gpib-command-list`（指令規格 V12.13.884）、`gpib-program-manual`（V12.04）、`gpib-93k-art`、`gpib-hana`、`gpib-qrovo` | P1 翻譯 `ProcessStatusString*` 時以 `gpib-command-list` 的指令表當 ctest 腳本來源；93K ART／HANA／Qorvo 三個協定各一組腳本 |
| `D:\RS232Standard\.github\agents\RS232Standard.agent.md` | 絕不改 `D:\HT9045\elec\myvcl`／`elec\Component` 共用元件；skills：`rs232-standard-interface`（V12.11.843：`[STX]+Cmd+[ETX]`、BA/CE/CF/CZ/CN/CA/CB/CD/CH/CI/BARCODE?/GET2DID?、9600-7-1-Even、Handler Status bit）、`rs232-ttl-communication`（VER.07082201：`@ W/R CMD DATA CRC16-Modbus #`、INIT/MODE/SOTL/DATL/EOTL/DUTL/SOTT/DUTT/SOTS/DUTS/RBIN/PWON/OTME/CSOT/VERS、`@Runing`、Err 碼） | P4 的 Standard／TTL 兩組 ctest 腳本直接照這兩份規格寫；TTL 板 CRC16-Modbus 用 `cmydef.cpp` 的 `crc_chk` 翻譯後再對規格的範例值驗證 |

---

## 2. 目標架構

```
 [Tester]  GPIB(gpib-32.dll)   RS232 / DIO=TTL 板(COM)   TCP(ClientSocket)
               │                    │                        │
 ┌─────────────┴────────────────────┴────────────────────────┴──────────────── wb_serve.exe ───┐
 │              TesterComm 執行緒（一條；TesterCommHub 依 TestIF_File.iTestType 只啟動一個引擎）    │
 │     GpibEngine        │   Rs232Engine（含 TTL 板）   │   TcpEngine（既有 TesterTCP_Socket 的     │
 │     (TSerialPoll 去 VCL)│   (TfRS232Main 去 VCL)      │    連線／解碼工作搬進這條執行緒）          │
 │              │ SyncMailbox × 1（同一對，三種引擎共用）                                           │
 │              ▼                                                                                │
 │  ┌───────────────────────── tick 執行緒 (500 ms) ─────────────────────────────────────────────┐   │
 │  │  TfMain::OnMyCopyMsg（golden 翻譯一份，依 iSendCommand 分派，不分來源）                    │   │
 │  │  Command.cpp Write*（已翻） ／ atester GetTesterResult（解閘，四種 mode 同一個入口）        │   │
 │  │  TestIF_File（唯一設定來源，cTesterIF 讀寫） ／ PublishExtraTags → testercomm.*            │   │
 │  └────────────────────────────────────────────────────────────────────────────────────────────┘   │
 └───────────────────────────────────────────────────────────────────────────────────┬────────────────┘
                                                                                     ▼  WS/HTTP
                                                                        瀏覽器 testercomm.html（GPIB／RS232-TTL／TCPIP／DIO 分頁）
```

### 2.1 執行緒規則（回應「GPIB 必須是獨立執行緒」＋「IO 通訊即時、畫面可略延遲」＋「三種通訊可共用執行緒」）

1. **只有一條 TesterComm 執行緒**（使用者 20260926 裁決 9：機台生產中基本上只會用一種通訊方式）。`TesterCommHub` 依 `TestIF_File.iTestType` 建立並啟動**一個**引擎；換介面（cTesterIF 存檔）時先停舊引擎、再啟新引擎，照 golden `CloseGpibProgram()`→`RunTestProgram()` 的時序。
2. **GpibEngine**：golden `TMyThread` 1 ms 迴圈原樣搬：`Sleep(1); ProcessAddress(); ProcessMessage();`，不再 `Synchronize`。`MyGPIBWrite` 最長 2 秒阻塞留在此執行緒。
3. **Rs232Engine（含 TTL 板）**：golden 沒有專屬執行緒（靠 300 ms Timer＋COM 事件）。在 TesterComm 執行緒內：迴圈 = `Timer1Timer` 的工作（每 300 ms）＋ 從 vclcompat `Comm` 讀執行緒收來的 bytes（`OnReceiveData` 在讀執行緒觸發，改為丟進本執行緒的 inbox，由它跑 `GetAnalysisString/DoRevCommand`／`CommTester_TTLReceiveData`）。理由：golden 這些 handler 都在 VCL 主執行緒跑，是單執行緒語意；不能讓 COM 讀執行緒直接跑協定碼。
4. **TcpEngine**：沿用 V906 vclcompat `ClientSocket`（OnRead 在讀執行緒）；`TimerTCPIPConnectTimer`（1 s）與 `TimerProcessTCPDataTimer` 的工作搬進 TesterComm 執行緒的迴圈。golden 的解碼直接寫 `fMain->tTestResult`／`bEcho`（主執行緒狀態），這裡改成經同一個信箱交給 tick 執行緒寫入——**這是為了「一條執行緒」規則的刻意適配**，帳本記錄。
5. **DIO**：就是 Rs232Engine 的 TTL 板模式。
6. **引擎狀態只屬於 TesterComm 執行緒**；tick 執行緒只透過信箱與唯讀快照互動。tag 只由 tick publish（`TagSnapshot` 單一 publisher 契約）。
7. **與機台執行緒隔離，不能互相干擾**（使用者 20260926 裁決 11）。明文規則：
   - 兩條執行緒之間**只有三個接點**：`SyncMailbox`（雙向封包）、`TesterCommSnapshot`（`WbMutex` 保護、tick 只讀）、瀏覽器指令 inbox（tick 丟、comm 收）。除此之外**沒有任何共用可變狀態**；引擎裡的 golden 全域（`iStart[]`、`iResult[]`、`bSimulate`、`LastSet`…）全部搬成引擎成員。
   - TesterComm 執行緒**不得**呼叫 `MainProc`、IO／馬達／溫控物件、`PublishHandlerTags`、任何 `fMain->`；tick 執行緒**不得**呼叫 NI／COM／socket API。用 `static_assert`＋連結層分離守住：`ht9045_testercomm` 不連 `ht9045_io`／`ht9045_motor`。
   - tick 被擋的唯一情況是 golden 本來就有的 `SendMessage` 同步等待（Handler 送一筆等橋接處理完），上限 = 橋接端 `OnMyCopyMsg` 的處理時間（毫秒級）；`SyncMailbox` 的 5 秒 timeout 保證 comm 執行緒卡死時 tick 最多慢一拍就回來，並記 `WAR` 告警而不是停機。
   - comm 執行緒的阻塞（`MyGPIBWrite` 2 秒、`SleepEx(1000)`、COM timeout）**永遠只在它自己身上**；watchdog breadcrumb 分開記兩條執行緒的心跳，任一條停止都能在 log 看出是哪一條。
   - 驗收（P3）：`wb_serve` 跑 10 分鐘，模擬引擎故意 sleep 3 秒／throw／死迴圈三種故障，`PumpTick` 週期抖動 ≤ 1 個 tick，機台狀態機不掉步。

### 2.2 WM_COPYDATA 的行程內替身：`SyncMailbox`（三種引擎共用同一對）

golden `SendMessage(WM_COPYDATA)` 是同步且允許雙向巢狀；橋接端 `OnMyCopyMsg` 內會再 `SendMSG_CMD` 回 Handler（GPIB 的 `MSG_CMD_TesterMode` 回覆；RS232 的 `MSG_CMD_TesterMode` 內 `SendMSG_CMD(MSG_CMD_State_TTL)`、`CZ status?` 查詢時先 `SendMSG_CMD(MSG_CMD_MachineState)` 再讀快取字串——**RS232 端的 `CZ *?` 一族是同步語意的鐵證**：`DoRevCommand` 送出查詢後立刻用 `sMachineStateDecade` 等字串組回覆，靠的是 SendMessage 回來時 Handler 已經把值填進 `GHandler2Gpib` 並被 `OnMyCopyMsg` 存起來）。

`TesterComm/SyncMailbox.h`：

| 方向 | 呼叫端 | 行為 |
|---|---|---|
| Bridge → Handler | 引擎執行緒 `SendMSG_CMD` | 複製 `GGpib2Handler` 進 `toHandler`，**等待**完成；等待期間代為處理 `toBridge` 送進來的訊息（pump-while-waiting） |
| Handler → Bridge | tick 執行緒 `TfMain::SendMSG_CMD` | 複製 `GHandler2Gpib` 進 `toBridge`，**等待**完成；等待期間代為處理 `toHandler` |

- timeout（5 s）＋ watchdog breadcrumb；逾時記 log 不永久卡死。
- `HandlerHwnd/GpibHwnd` 欄位保留（封包不改），行程內用 session token 取代 `FindWindow`；`bFind` 改成 `link.IsUp()`。
- `HGpib2Handler`/`HHandler2Gpib` 從 NULL 改指向信箱內的當前封包（解 `MessageDef.cpp:267` 踩空）。
- 一個 Handler 同時只會有一種介面在跑（`iTestType` 決定），所以 `SyncMailbox` **只有一對**，Handler 端 `SendMSG_CMD` 直接送給目前啟動的引擎；`TesterCommHub` 換引擎時先清空信箱。

### 2.3 驅動抽象

| 通訊 | 抽象 | 真實實作 | 模擬實作（SOFT_SIMULTE／ctest） |
|---|---|---|---|
| GPIB | `IGpibDriver`（9 個 NI 函式＋`ibsta/iberr/ibcnt`） | `LoadLibrary("gpib-32.dll")`＋`ThreadIbsta()` 系列（per-thread，配獨立執行緒） | `SimGpibDriver` 餵腳本（`SETTEMP?`、`SETSITEMAP_`、BINON…） |
| RS232／TTL 板 | `ISerialPort`（open/close/write/bytes-in） | vclcompat `Comm`（已有讀寫執行緒） | `SimSerialPort`：腳本回 `ENQ/ACK/STX CF ETX/BA…` 或 TTL 的 `RBIN`；也可沿用 golden 的 `uSocketServer` TCP 模擬入口 |
| TCPIP | 既有 `ClientSocket` | 同 | 既有 `SimulateBin()`（`LastSet.iTester==OFF_LINE`） |

### 2.4 設定：統一在 cTesterIF（使用者裁決 6）

> **20260928 註（St02-E）：本節下面的設計大部分被使用者裁決 C-2 取代（20260926，FROM_STEVEN `c05a32a9` 15:32），下面保留原文當歷史。**
> - C-2：St02 先逐項比對兩支橋接程式與 Handler 的設定，結論是大部分欄位 Handler 本來就有並會送給橋接；裁決 **1＝A** Tester／TTL1／TTL2 的 COM port 維持機台層級（Setup.ini、System 畫面）；
>   **2＝A** GPIB 程式自己的 serial port 跟 Handler 的設定走；**3＝A** Auto Retest 廠牌以配方為準；**4＝A** Handler ID 回覆格式只看 Handler 的 config；**5＝B** 補 RS232 缺的選項；
>   **6＝A** GPIB 的時間類設定維持機台層級（GPIB general.ini）；**7＝B** Tester COM 預設值照 golden。
> - **取代掉的**：`SYSTEM_TEST_IF` 不加新欄位（asRs232ComName、iRs232ReadIntervalTimeout、asTTLComName[2]、GPIB recipe 級欄位都不做）；一次性遷移只剩 2A 的種值；
>   `CheckRs232StandardIni` **不退場**（RS232 引擎照 golden 讀 Setup.ini，要靠它把配方的格式寫進去，V906 `HT9011UC_Cpp_V3.33.906.0/FileRW/TestIF_File_TesterIF.gen.inc:1031`）；iRevCycleClear／bCheckClosedSiteHasBin 照 golden 只在 Setup.ini。
> - **做了的**：5B、Q3(1)A、4A、3A（`7aa13ca3`）；2A＋Q2 (a)（`e36e05a0`，ctest `TesterComm_P6GpibAux`）；W1＝(b)＋W1b、W3～W6（Steven 20260927）；網頁 Q5＝W6（Q41 `e6e90401`）。細節在 `HT9011UC_Cpp_V3.33.906.0/docs/TESTERCOMM_PORT_LEDGER.md`「P6」。
> - **剩下的**：沒有程式要做；只有 Q41-W3G（格式檢查在 GPIB 模式也擋，暫照 A）待確認。

原則：**`TestIF_File`（`Tester.Data`）是四種介面的唯一設定來源，cTesterIF 是唯一的讀寫畫面**；三支程式各自的 ini 不再是設定來源。

| 原本在哪 | 欄位 | 併入後 |
|---|---|---|
| RS232 `Setup.ini [COMPort]` | CommName／BaudRate／ByteSize／StopBits／Parity／ReadIntervalTimeout | `TestIF_File.Rs232_Data` 已有 Baud/Bit/Stop/Parity；**新增** `asRs232ComName`、`iRs232ReadIntervalTimeout`（golden 的 `CheckRs232StandardIni` 同步碼整段退場） |
| RS232 `Setup.ini [COMPort_TTL*]` | TTL 板 1/2 的 COM 名 | **新增** `asTTLComName[2]`；`TTL_CARD_TYPE`／`TTL_CARD_USE_ADDRESS` 維持在 `Gerneral.ini [System]`（機台硬體屬性，不是 recipe） |
| RS232 `Setup.ini [SystemSetup]` | iRevCycleClear／bCheckClosedSiteHasBin | 併入 `TestIF_File`（recipe 級）；`iTesterMode` 由 `iTestType`＋`iDioMode` 推得，不再獨立存 |
| GPIB `general.ini [SystemSetup]` 56 欄 | GpibAddress／iTimeOut／FullSiteTimeOut／bGPIBWriteWithout_r_n／iMyGpibWriteRetry／WaitMS／Threshold／bDummyART／Delta Castle 參數… | 分兩類：**recipe 級**（timeout、r/n 選項、retry 參數、FullSite）進 `TestIF_File`；**機台級**（`[Version] Model`、Log 路徑）留 `general.ini`（`database.cpp:316` 已讀 Model 決定 `W906_GpibModel`） |
| GPIB `GpibString.dat` | 上次 site 字串 | 生產狀態，非設定：留檔（golden 真讀真寫規則） |
| TCPIP | `asTester_Address`／`iTester_Port` | 已在 `TestIF_File`，不動 |
| DIO（TTL 板） | `iDioMode`、SOT/EOT/DUT 邏輯、寬度、Bin timeout（組成 `WINIT` 47 碼） | 已在 `TestIF_File`，不動；`TTL_CARD_TYPE`／`TTL_CARD_USE_ADDRESS` 留 `Gerneral.ini` |

- cTesterIF 的 web 頁（FW 戰役 `Config`／`TesterIF` 表單）加對應欄位；`/api/struct` 走 `ht9045-json-bridge` 既有的 struct→JSON 通道，不另開設定 API。
- 一次性遷移：wb_serve 第一次啟動若 `TestIF_File` 新欄位為空，從舊 `Setup.ini`／`general.ini` 讀入並存回 `Tester.Data`，log 記「migrated from …」。之後舊 ini 只剩機台級欄位。

### 2.5 畫面：`testercomm.html` 一頁多分頁

| 分頁 | 內容（golden 對應） | tag 前綴 |
|---|---|---|
| GPIB | 13 顆 ibsta LED、狀態列 6 格、32 站 `makeDutPanel`、手動 Start／Timeout／RunMode／SendTemp、選項、SCK ART 面板、Log tail、Version | `gpib.*` |
| RS232／TTL | COM 狀態（Standard／TTL1／TTL2 三個連線燈）、32 站 `makeDutPanel`（`"T"`／bin／`999`／barcode 進 OCR）、Bin Log、Simulate（手動測試、All Use／No Use）、TTL Setup（`WINIT` 參數顯示） | `rs232.*` |
| TCPIP | 連線狀態（ON-LINE／OFF-LINE）、32 站面板（`plSite` `--`/bin）、Comm Log、手動指令送出 | `tcpip.*` |

- 共用元件：`HTWidgets.makeDutPanel`（已做，20260926）**四種模式共用同一個**（使用者 20260926 裁決 10）——GPIB／RS232／TTL／TCPIP 分頁的 32 站面板都是同一個 maker、同一組 `setSite/setBin/setOn/setOcr`，只有 tag 前綴不同；LED 列用既有 `makeALed`。
- 站面板的更新只走快照 tag（≤500 ms 延遲，使用者裁決可接受）；指令 `gpib.*`／`rs232.*`／`tcpip.*` 由 tick drain 後丟進對應引擎 inbox。
- 註冊：`background.html` WINDOWS 加 `{id:'testercomm', title:'Tester Comm', src:'testercomm.html', hidden:true}`；檔案放 `web-overlay/`。

---

## 3. 目錄與建置

```
HT9011UC_Cpp_V3.33.906.0/
  TesterComm/
    MessageDefBridge.h         ← 引用 V906 MessageDef.h（唯一契約），版本字串升 12.13.905.0
    SyncMailbox.h/.cpp         ← §2.2，GPIB／RS232 共用
    TesterEngine.h             ← ITesterEngine：Start/Stop/IsUp/Snapshot/RunOnce；GPIB／RS232／TCP 三種實作
    TesterCommThread.h/.cpp    ← 唯一的 WbThread：迴圈呼叫目前引擎的 RunOnce()，處理信箱與 inbox
    TesterCommHub.h/.cpp       ← 依 TestIF_File.iTestType 建立／切換引擎；Handler 端 SendMSG_CMD 入口；換介面時清信箱
    Gpib/
      GpibDriver.h / GpibDriverNi.cpp / GpibDriverSim.cpp
      GpibSettings.h/.cpp      ← cmydef LastSet 的 recipe 級欄位改讀 TestIF_File，機台級留 general.ini
      GpibEngine.h/.cpp        ← Main.cpp 去 VCL（TestGPIB, ProcessStatusString*, SetParameter, MyGPIBWrite, OnMyCopyMsg(bridge), ProcessAddress, ProcessMessage, Save_Log）
      GpibEngineCastle.cpp / GpibEngineHanaArt.cpp
    Rs232/
      SerialPort.h / SerialPortComm.cpp / SerialPortSim.cpp
      Rs232Engine.h/.cpp       ← MainForm.cpp 去 VCL（GetAnalysisString, DoRevCommand, SendResultFinish, OnMyCopyMsg(bridge), Timer1 工作, Open/Close/SendCommandToTester*）
      Rs232EngineTtl.cpp       ← TTL 板協定（CommTester_TTLReceiveData, State_TTL/Command_TTL, CRC）
      Rs232Log.cpp             ← TMyStringList 的 2 小時分檔 log
    Tcp/
      TcpEngine.cpp            ← 把 Interface/TesterTCP_Socket 的兩個 Timer 工作接到 TesterComm 執行緒；解碼結果經信箱交 tick
    TesterCommTags.cpp         ← stage gpib.*/rs232.*/tcpip.*/dio.*（tick 執行緒）
    TesterCommCommands.cpp     ← gpib.*/rs232.*/tcpip.* 指令 → 引擎 inbox
  web-overlay/testercomm.html, web-overlay/js/testercomm.js
  tests/test_testercomm_*.cpp
```

- CMake：新 target `ht9045_testercomm`（依賴 `ht9045_globals` 的 MessageDef、`vclcompat`）；`wb_serve` 連結它。不需要 NI SDK（動態載入）；SOFT_SIMULTE 建置自動用 Sim 驅動。
- `MessageDef.cpp` 的 `GPIBVersion`／`RS232Version` 升到 `"12.13.905.0"`（`GPIBVersionCheck` 仍 12.13）。

---

## 4. 分階段與驗收 gate

| 階段 | 內容 | 驗收 |
|---|---|---|
| **P0 契約凍結**（1 天） | 三邊 `MessageDef.h` 逐欄 diff（已知 0 差異）；`TesterComm/` 骨架＋CMake；`docs/TESTERCOMM_PORT_LEDGER.md`（GPIB 905／RS232 902 為基準）；cTesterIF 新欄位清單定案（§2.4） | `build.bat gate` 綠；ctest 失敗清單不變（5 個已知） |
| **P1 GPIB 引擎離線翻譯** | `Main.cpp` 非 UI 部分 → `GpibEngine`；UI 存取改寫進 `GpibSnapshot`；`static` 區域變數改成員 | `test_testercomm_gpib_engine`：SimGpibDriver 餵腳本，比對回寫字串與 `MSG_CMD_*` 序列（golden 程式 bSimulate 下的 log 當 oracle） |
| **P2 Handler 端接線** | 翻譯 golden `TfMain::OnMyCopyMsg`（1,830 行）；`SendMSG_CMD`／`SetLotState`／`RunTestProgram`／`CloseGpibProgram` 從 stub 改接 `TesterCommHub`；`HGpib2Handler` 指向真封包；`atester.cpp` `GetTesterResult`／`ProcessTestResult`／`ProcessTesterTimeOut` 解閘（GPIB／RS232／TTL 板／TCP 一次解；`TTL_CARD_TYPE<2` 直接讀卡分支**不解**） | `test_testercomm_handler_dispatch`：Bridge→Handler `MSG_CMD_HandlerTemperature` → `TempDataStrings` → 回字串 = golden；`test_interfacesys` 仍綠；DIO 分支在 SOFT_SIMULTE 下走 IO 物件 stub 不當機 |
| **P3 執行緒 + 信箱** | 一條 `TesterCommThread`、`TesterCommHub` 依 `iTestType` 啟停引擎、`SyncMailbox` 雙向、timeout/watchdog、單一實例互斥（`MyMutexGPIB`／`MyRS232Standard` 名稱保留：舊 exe 在跑就拒啟動） | `test_testercomm_ipc`：雙向巢狀不死結、逾時可回收；wb_serve 跑 10 分鐘 tick 抖動不受 `MyGPIBWrite` 2 秒阻塞影響 |
| **P4 RS232／TTL 引擎翻譯** | `MainForm.cpp` → `Rs232Engine`＋`Rs232EngineTtl`；套 P3 的執行緒與信箱骨架；`uSocketServer` 模擬入口保留 | `test_testercomm_rs232_engine`：Standard 腳本（`ENQ→ACK`、`CF/CE→STX 8|sites ETX`、`BA 1,1,1;…→MSG_CMD_NONE Result[]`、`BA without CE→BinonWithoutFullsite`、關 site 有 bin→999）；TTL 腳本（`WINIT`/`WSOTS` CRC 正確、`RBIN` 長度錯→999、3 秒無回應→`MSG_CMD_Version` 錯誤字串） |
| **P5 TcpEngine** | `TcpEngine` 掛進 TesterComm 執行緒；`fTesterTCP` T-1/T-3 gate 以快照取代 widget；`atester.cpp` 解閘時 `TTL_CARD_TYPE<2` 分支保持 gate 並記帳 | 既有 `TesterTCP` ctest 仍綠；離線 `SimulateBin` 走通 `bEcho` |
| **P6 設定統一** | cTesterIF／`TestIF_File` 新欄位、`ReadIniData/WriteIniData` 對應、一次性遷移、`CheckRs232StandardIni` 退場；web 表單加欄位 | `test_testercomm_settings`：舊 `Setup.ini`＋`general.ini` → 遷移後 `Tester.Data` 值正確；存檔後重讀一致（`ht9045-json-bridge` 的存檔後重讀規則） |
| **P7 畫面** | `testercomm.html` 四分頁、tag、指令、`background.html` 註冊 | `test_testercomm_tags`；瀏覽器離線（Sim）四個分頁都能動 |
| **P8 機邊 bring-up** | 真 NI 卡／真 COM／真 Tester；HANA ART、DummyART、AMD RS232 面板補齊；`WebStart` 的 `bFind` 閘改用 `TesterCommHub::IsUp()` | 走 BU 戰役授權三層級；順序：GPIB 查詢類 → SOT/EOT 一個 cycle → 32 站 BINON；RS232 同序；TTL 板先 `WINIT` 再 `WSOTS` |

工作量（翻譯行數）：GPIB 引擎 ≈6,000 ＋ Handler `OnMyCopyMsg` 1,830 ＋ RS232 引擎 ≈3,000 ＋ atester 解閘 ≈1,700 ＋ TCP 泵／DIO 解閘 ≈300 ＋ 設定 ≈500 ≈ **13,000 行**。
建議開一個 **TC 戰役**（`tc-wave` + `tc-wave-loop`），夜間 loop 推 P1／P2／P4（純翻譯＋ctest），P3／P5～P8 白天做。

---

## 5. 裁決點（20260926 使用者已全部裁決，結論見 §7；本節保留原題目）

1. **同步語意**：§2.2 的 pump-while-waiting 信箱是忠實作法；若接受「回覆時序可略為不同」可先用純非同步佇列少寫約 200 行。建議：忠實。
2. **非 9045 機型分支**：橋接程式內有 2601/7080/1032/502/9055 的機型與指令分支。忠實翻譯全帶（死碼但零風險）或只帶 9045/9046_32/9050？建議：全帶，`W906_GpibModel` 不在白名單時引擎拒啟動（照 golden `Timer1Timer` 行為）。
   - HT9050 已於 20260926 加進橋接程式 V12.13.905.0 的 `Main.cpp`（四處：版本備忘、`Timer1Timer` 白名單 `9050GPIB`、
     `ProcessHMountConnect` 視窗標題清單 `HT-9050`、`ReadLastDataFile` 分支 `MachineType=2` 比照 9046_32GPIB，因為 F/B 各 2×8 吸嘴 = 32 站）。
     V906 端 `database.cpp:345/492` 早已有 `9050GPIB`。翻譯時這五個機型一起帶。**RS232Standard 的 `FindWindow` 清單尚未加 `HT-9050`**（BCB 線要補）。
3. **與 BCB6 橋接程式的後續同步**：併入後 D:\GPIB9045、D:\RS232Standard 仍會被同事改版。建議：`docs/TESTERCOMM_PORT_LEDGER.md`，每次橋接端出新版就做一次 diff 補搬（同 V899→V912 的做法）。
4. **啟動時機**：跟著 golden `RunTestProgram()`（Tester 模式為 GPIB 時開）還是 wb_serve 開機就常駐？建議：跟 golden，避免非 GPIB 客戶多一條執行緒去摸 NI DLL。
5. **舊 exe 共存**：是否保留能外掛舊 H9046_32GPIB.exe／RS232Standard.exe 的相容路徑？建議：不保留，單一執行檔裁決（20260918）已定。

---

## 6. 風險

- Handler 端 `OnMyCopyMsg`（1,830 行）＋ `atester` 解閘（≈1,700 行）是最大的一塊未翻譯 golden 邏輯，屬 GL 戰役等級的「照 golden 不偷吃步」項目；四種 mode 的結果路徑都經過它，**解閘一次就全部上線**，要有四組 ctest 同時守。
- `ThreadIbsta()` 等 per-thread 狀態：所有 NI 呼叫必須在 GPIB 執行緒；`IGpibDriver` 只交給引擎持有。
- RS232 的 COM 事件在 golden 是主執行緒語意；V906 vclcompat `Comm` 的 OnReceiveData 在讀執行緒觸發，**必須**改成投遞到 RS232 執行緒，否則協定狀態機（`iSendStart`、`iHasCE`、`bStartSOT[]` 全是 static／全域）會被兩條執行緒同時改。
- `Main.cpp`／`MainForm.cpp` 大量 `static` 區域變數：搬進引擎成員，否則單元測試無法重置。
- 設定統一（P6）會改 `SYSTEM_TEST_IF` 結構：`ht9045-array-audit` 的邊界檢查與 `ht9045-json-bridge` 的 FieldDesc 要同步；`Tester.Data` 是 recipe，舊 recipe 沒有新欄位時走預設值＋遷移，不能報錯。
- TTL 板 CRC（`crc_chk`）與 `@00`/`@01` 站號規則只在 RS232 程式有實作，翻譯時要帶 `cmydef.cpp` 的 `crc_chk`。
- Big5 → UTF-8 轉碼：兩支橋接程式 log 字串含中文，沿用 V906 既有做法。

---

## 7. 裁決紀錄（使用者 20260926）

| # | 題目 | 裁決 | 對設計的影響 |
|---|---|---|---|
| 1 | 信箱同步語意 | **GPIB 的 IO 通訊必須是即時的；HTML 畫面可以有一點點延遲** | §2.2 的 pump-while-waiting 同步信箱定案，GPIB↔Handler 不走非同步佇列。畫面維持 §2.1 的快照＋tick publish（≤500 ms 延遲可接受），不為畫面另開即時通道 |
| 2 | 非 9045 機型分支 | **全部都要進來** | 2601/7080/1032/502/9055/9050 與 9045 系一起忠實翻譯；`W906_GpibModel` 不在白名單時引擎拒啟動 |
| 3 | 補搬帳本 | **OK** | 立 `docs/TESTERCOMM_PORT_LEDGER.md`：記 BCB 橋接版本 → V906 翻譯基準；橋接端每出新版做一次 diff 補搬 |
| 4 | 啟動時機 → 範圍擴大 | **後續要把 GPIB、DIO、RS232、TCPIP 四種通訊一併整合**；DIO 用 RS232 版本，程式碼在 `D:\RS232Standard\RS232_Code32Bin_Rev12.13.902.0_20260410`；**TMyDutPanel 可以共用，畫面也可以共用**；**`#include "TesterTCP.h"` 也是** | §1.3～§1.5 盤點、§2 四合一架構、§4 P4～P5。啟動時機本身未另裁，暫照建議（跟 golden `RunTestProgram()` 時機，由 `TesterCommHub` 依 `iTestType` 啟停） |
| 5 | 舊 exe 相容路徑 | **短期 BCB 版本會存在，但 BCB 接 BCB；C++ 版本就是串接 C++ 的 GPIB** | V906 不做外掛舊 exe 的相容路徑。BCB Handler ⇄ BCB 橋接 exe 那條線照舊維護（20260926 GPIB 已加 HT9050），與 C++ 線互不相接 |
| 6 | 設定 | **設定的部分可以統一在 `cTesterIF.h` 處理**；**原本是每個程式得單獨存檔與讀檔** | §2.4：`TestIF_File` 是唯一設定來源，cTesterIF 是唯一畫面；`Setup.ini`／`general.ini` 的 recipe 級欄位併入，機台級留原檔；`CheckRs232StandardIni` 同步碼退場；一次性遷移 |
| 7 | DIO 直接讀卡 | **`atester.cpp` 的 `TTL_MODE && TTL_CARD_TYPE<2` 分支在新機台已不適用，要走 RS232Standard 那邊** | §1.5：C++ 線的 DIO = RS232 引擎的 TTL 板模式；直接讀卡分支不翻、帳本記「刻意不翻」；§2 架構少一條線 |
| 8 | GPIB 內建 RS232 | **GPIB 裡原本就有一條 RS232，但做的事跟 RS232Standard（測試工作）不一樣** | §1.2：AMD／ATC／HT-505 的輔助通道留在 `GpibEngine`，不併進 `Rs232Engine` |
| 9 | 執行緒數 | **GPIB／RS232／TCPIP 可以共享執行緒，因為機台生產中基本上只會使用其中一種通訊方式** | §2.1：一條 `TesterCommThread`，`TesterCommHub` 依 `iTestType` 只啟動一個引擎；`SyncMailbox` 一對；TcpEngine 的解碼結果改經信箱交 tick（刻意適配，記帳） |
| 10 | 站面板 | **32 站 `makeDutPanel` 四種模式共用同一個** | §2.5：四個分頁同一個 maker，只差 tag 前綴 |
| 11 | 執行緒隔離 | **執行緒的部分要注意必須跟機台的執行緒分開，不能互相干擾** | §2.1 第 7 條：三個接點以外零共用狀態、雙向禁呼清單、tick 最多被擋一拍、分開的 watchdog 心跳、P3 故障注入驗收 |

### 7.1 對階段表的調整（已併入 §4）

- P0 加 RS232 盤點與帳本；P4 RS232 引擎；P5 TcpEngine；P6 設定統一；P7 畫面改四分頁（32 站面板共用）。
- 通知信規則（使用者 20260926）：GitLab 推送後寄給 Jimmy 的信，**標題必須是 `[HT9045 Gitlab 推送通知]`**。
