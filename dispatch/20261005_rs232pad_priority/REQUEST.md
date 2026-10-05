# 派工 2026-10-05（優先）：先行移植 RS-232 實體操作面板（golden uPadInterface）—— 機台要測試

> 給：Jimmy（筆電開發樹）　來自：EastSun（機台端，1203 層作者）　整理：機台端 Claude，2026-10-05 晚
> 機台樹：`D:\HT9045\_integ_ioweb\HT9011UC_Cpp_V3.33.906.0`（10-05 起實體在 SSD `C:\HT9045_ssd\_integ_ioweb`，D: 是 junction），HEAD `9affb8e`（＝包 149）；web `abe25b4`。
> golden = `D:\HT9045\HT9011UC_Code_V3.33.906.0_20260618`（Big5，唯讀）。行號會漂，用之前 grep 名稱。

## 0. EastSun 原話

「較jimmy 先行轉 RS232 要測試了」（＝請 Jimmy 先行轉 RS-232，要測試了）。
前文：電控說實體按鈕的 COM「有接上去了 是照舊的」「應該是RS232」，「跟一般 9 系列一樣」。
包 149 README 寫 RS-232 面板排給 Ifor01 —— EastSun 要求**優先**，請 Jimmy 決定由誰做、盡快出包。

## 1. golden 的路（機台端查證）

- **唯一的 RS-232 面板路徑是 `uPadInterface.cpp/.h`（約 960 行，`TfPadInterface`＋`TPadRS232Thread`）**。IO_Table 的 SnFK*／SnRK* 列（Lane 1、ModuleType 1、IP 5、ISABase 0）是 **MotionNet** 遠端 DIO，不是這條路；機台上 26 列全 Enable=0，**不要打開**。
- **開關**：`iControlPanelMode = [System] ControlPanelMode`（database.cpp:1498）；**COM 埠**：`HSys.TrayStepMotor_ComPort = [TrayY] COM PORT`（database.cpp:522，預設 COM18）——面板與 Tray 步進馬達**共用同一個埠**（uPadInterface.cpp:288、:381；`dmTrayMotor->comTrayStepMotor`）。
- **開埠**：開機時 `LoaderUnload_StepMotor || iControlPanelMode || USE_VibrationCommunication` 就 `dmTrayMotor->RS232Init(TrayStepMotor_ComPort)`（cinitial.cpp:5847-5851）；埠檢查與「Control Panel : COMx port error」訊息 rs232.cpp:221-234。
- **序列設定**：115200、8N1、無流量控制、DTR／RTS 開（TrayStepMotor.cpp:69-71；TrayStepMotor.dfm comTrayStepMotor）。`PadInterfacePara.ini`（COM 1／9600）與 rs232.dfm 的 PadComm 9600 是死碼（從不讀／不開）。
- **協定**（ASCII hex，每幀以 `\r` 結尾）：
  - 收：`comTrayStepMotorReceiveData` 把 `t05` 開頭的給面板、`t07`／`t08` 給步進馬達／振動（TrayStepMotor.cpp:410、:507-513）。幀＝`t05`＋位址字元（0 前面板、1 後面板、2 第三面板）＋DLC＋2 字元功能碼＋6 hex 按鍵遮罩（uPadInterface.cpp:715-748）。功能 `00`＝按鍵（DoScanPanelLed 解碼）、`90`＝燈號回應、`20`＝版本回覆。
  - 按鍵位元（uPadInterface.h:27-43）：PowerOff 0x1、PowerOn 0x2、PanelEnable 0x4、Reset 0x8、Pause 0x10、Home 0x20、Start 0x40、OneCycle 0x80、Retry 0x100、Skip 0x200、CleanOut 0x400、TrayFeed 0x800、TrayEnd 0x1000、AlarmReset 0x2000、SafeLock 0x4000、Step 0x8000、TStart 0x10000。
  - 送：開機一次 `t051400000000`（問按鍵狀態，:887）；每 10 秒 `t051120`（問版本，RequestPadVersion），1 秒沒回就 ResetComm（:899-937）；初始化燈 `t050490000000`／`t050491000000`／`t051490…`／`t052490…`（:165-170）；燈號幀 `t05{addr}49{0|1}{6-hex}`（:751-847）。`SendCommand` 寫 `Length()+1` 個位元組＝資料＋`\r`（:491-510）。
  - `TPadRS232Thread` 約每 1 ms `Synchronize(Main232)`（:42-49；main.cpp:22478 建立、:10141 啟動）。
- **位元落點**：DoScanPanelLed 設 31 個 `PadItem[i].mlEvent->Value`（SnFK*／SnRK*／SnRearPadActive／SnRKSafeLock／SnRKManualStep…，:211-242、:545-665）；`TMySensor::Status/IsOn` 在 `iControlPanelMode==1 && IsPadKey(Name)` 時回 `fPadInterface->ProcessScanKey(Name)`（golden mysensor.cpp:40-43、:81-83）→ `ScanPannelKey` 的 `Sen[SnFK*]` 就讀到面板。ControlPanelMode==1 時 golden 也把 IO 型的面板 Sen／SW 列停用（cinitial.cpp:1537、:2727）。

## 2. 移植樹現況：完全沒轉

- 沒有 `uPadInterface` 邏輯、沒有 `TdmTrayMotor`；只有 dfm2rc 產生的版面檔（tools/dfm2rc/…/uPadInterface*）與 `docs/nb2_assist/RD5軟體_NB2回覆Q2_uPadInterface預勘_20260924_222022.md`（預勘）。
- 閘住的地方：`mysensor.cpp:72／:119／:167`、`myswitch.cpp:78／:128／:178` 都是 `#if 0`（「uPadInterface 未翻 ⇒ W3 子項 4b」）；`cinitial.cpp:17139` `#if 0`（N1-G2g，RS232Init 從不呼叫）；rs232.cpp:250／:271 面板那段開機埠檢查仍閘住；tools/wb_serve.cpp:7231、FileRW/MainClose.cpp:1011-1012 也寫未移植。
- 已活著的：database.cpp:1645 讀 ControlPanelMode；cinitial.cpp:1680／:2983（=1 時停用 IO 面板列）；ckernel.cpp:3025、csystem.cpp:486（SafeLock）依它分支。
- 移植版的按鍵掃描入口已經在：`WebMainScanKey.cpp`（golden TfMain::ScanKey，主迴圈每拍）→ `ckernel.cpp ScanPannelKey` 讀 `Sen[SnFK*]`；[W906] first-scan guard 只擋「開機就 ON 的 START／HOME」。所以面板只要把 golden 的 ProcessScanKey 接回 mysensor／myswitch，ScanKey 那一半不用動。
- 建議放法：序列收發用 vclcompat TComm（收執行緒只排隊、主迴圈解碼，同 BinDisplay `BinDispBringUp_St02.cpp` 與 rs232.cpp TCOM2Shim 的做法），或併入另一張派工單 `dispatch/20261005_threading_perf` 的新執行緒架構。

## 3. 機台端 10-05 實測（wb_serve 關著；附檔腳本可重跑）

- 這台實體 COM＝**COM1～COM6**（ACPI PNP0501）＋ COM7（Intel AMT SOL，虛擬）；**沒有 USB 轉 RS-232**。
- 機台 ini（附 `Gerneral_ini_relevant.txt`）：`ControlPanelMode=0`、`[TrayY] COM PORT=COM18`（**這台沒有 COM18**）、`LoaderUnload_StepMotor=0`、`VibrationCommunication=0`、`NUMBER_PANEL_TYPE=3`（EastSun：TFT 應為 4）、`[NUMBER_PANEL] COM_PORT=COM14`（不存在）、`[NUMBER_PANEL2] COM_PORT=COM4`。wb_serve 開著時佔 COM2／COM4／COM5（COM4＝Bin 顯示 2、COM5＝Tester RS-232；COM2 原因不明）。
- `com_listen.ps1`：COM1～6 只聽 60 秒（115200、DTR/RTS 開、不送），EastSun 期間按面板 PAUSE／ALARM RESET → **全部 0 位元組**。
- `com_probe_pad.ps1`：每埠送 golden 查詢 `t051120`、`t050120`、`t051400000000`、`t050400000000`（115200；另 9600／19200／38400／57600 送問版本）→ **全部無回應**。
- `com_probe_tft.ps1`：每埠送 golden TFT `SetNoBackGround_TFT`（9600 8N1，位址 0x20～0x23）→ **全部無回應**。
- ⇒ 面板與 TFT 目前在 6 個埠上都沒有任何回應，**機台端已請電控確認接哪個 DB9、有沒有通電、RS-232／RS-485 模式、直通／交叉線**。所以移植完成後機台第一步會先用 `com_listen.ps1` 確認有 `t05…` 封包，再開 ControlPanelMode=1。

## 4. 驗收建議

- 有開關（`ControlPanelMode`），=0 時行為完全不變（今天機台就是 0）。
- ctest：假埠餵 `t050?00000040\r` 之類的幀 → `Sen[SnFKStart].IsOn()` 為 true、放開後 false；`t051…` 後面板；PanelEnable／SafeLock 決定前後面板（`bFrontPadActive`）與 IsSafeLockCheck；問版本逾時會 ResetComm；送出的幀逐位元組等於 golden。
- 安全：面板 START／HOME 直接進 ScanPannelKey 會啟動機台；first-scan guard 只擋開機就 ON 的 START／HOME，RESET／ONE CYCLE／RETRY／SKIP 照 golden 一按就觸發。SafeLock 狀態錯誤會鎖死全部鍵或打開後面板，請在測試裡釘住。
- 共用埠：`[TrayY] COM PORT` 同時給 Tray 步進馬達與振動；HT9050 兩者都 0，但程式要照 golden 共用同一個 TComm。

## 5. 附檔

| 檔名 | 內容 |
|---|---|
| `com_listen.ps1` | 只聽不送（多埠同時，115200，DTR/RTS） |
| `com_probe_pad.ps1` | 送一句 golden 面板查詢、聽回應（參數 -Ports／-Frame／-Baud） |
| `com_probe_tft.ps1` | 送 golden TFT SetNoBackGround、聽回應 |
| `Gerneral_ini_relevant.txt` | 機台 Gerneral.ini 相關行 |
