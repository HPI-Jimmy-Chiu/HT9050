# hpi-gpib 通用（不分機型）

> 本檔只放程式裡**沒有依機型分支**的 GPIB／ART／ATC／Tester 通訊行為。遇到 `MachineTypeChoice`、`Type_HT9050`、`Type_HT9046_LS` 或 Model 字串（`9050GPIB`…）分流的地方，只寫「此處依機型分流」；客戶分流只寫「此處依客戶分流，見 [customers.md](customers.md)」。
> 每段結尾的〔…〕是出處（ST02-C22 搬進來的舊 skill 檔，內容沒改）。行號以函式、Task／case 代替；要行號請看出處原文。

## 0. 出處縮寫

| 縮寫 | 檔案 | 原 skill |
|---|---|---|
| CL | [gpib-command-list.md](gpib-command-list/gpib-command-list.md) | gpib-command-list |
| STD | [standard-gpib-command.md](gpib-command-list/references/standard-gpib-command.md) | gpib-command-list |
| PM | [gpib-program-manual.md](gpib-program-manual/gpib-program-manual.md) | gpib-program-manual |
| PMD | [GPIB_Program_Manual_V12.04.md](gpib-program-manual/references/GPIB_Program_Manual_V12.04.md) | gpib-program-manual |
| 93K | [gpib-93k-art.md](gpib-93k-art/gpib-93k-art.md)、[93k-art-protocol.md](gpib-93k-art/references/93k-art-protocol.md) | gpib-93k-art |
| HANA | [gpib-hana.md](gpib-hana/gpib-hana.md)、[hana-art-protocol.md](gpib-hana/references/hana-art-protocol.md) | gpib-hana |
| QRV | [gpib-qrovo.md](gpib-qrovo/gpib-qrovo.md)、[qorvo-protocol.md](gpib-qrovo/references/qorvo-protocol.md) | gpib-qrovo |
| MRG | [gpib-rs232-merge.md](gpib-rs232-merge/gpib-rs232-merge.md)、[rs232-merge-details.md](gpib-rs232-merge/references/rs232-merge-details.md)、[bug-log.md](gpib-rs232-merge/references/bug-log.md)、[design-decisions.md](gpib-rs232-merge/references/design-decisions.md) | gpib-rs232-merge |
| SYNC | [gpib-ht9045-sync.md](gpib-ht9045-sync/gpib-ht9045-sync.md)、[sync-procedures.md](gpib-ht9045-sync/references/sync-procedures.md) | gpib-ht9045-sync |
| BR | [ht9045-gpib-bridge.md](ht9045-gpib-bridge/ht9045-gpib-bridge.md) | ht9045-gpib-bridge |
| PLAN | [gpib-v906-integration-plan.md](ht9045-gpib-bridge/references/gpib-v906-integration-plan.md) | ht9045-gpib-bridge |
| P8 | [gb-p8-bringup-plan.md](ht9045-gpib-bridge/references/gb-p8-bringup-plan.md) | ht9045-gpib-bridge |
| 7016 | [tcp-command-server-7016.md](ht9045-gpib-bridge/references/tcp-command-server-7016.md) | ht9045-gpib-bridge |
| ART | [ht9045-art-flow.md](ht9045-art-flow/ht9045-art-flow.md)、[handler-art-flow.md](ht9045-art-flow/references/handler-art-flow.md) | ht9045-art-flow |
| ATC | [ht9045-atc.md](ht9045-atc/ht9045-atc.md)、[Handler_Command_Report.md](ht9045-atc/references/Handler_Command_Report.md)、[ATC_Command_Payload.md](ht9045-atc/references/ATC_Command_Payload.md)、[MultiZone_EnableChannel_Bug.md](ht9045-atc/references/MultiZone_EnableChannel_Bug.md) | ht9045-atc |
| ATCI | [ht9045-atc-interface.md](ht9045-atc-interface/ht9045-atc-interface.md)、[atc-commands.md](ht9045-atc-interface/references/atc-commands.md) | ht9045-atc-interface |

---

## 1. 架構：Tester ⇄ 橋接 ⇄ Handler

- BCB 線有兩支獨立程式：`H9046_32GPIB.exe`（GPIB 橋接，原始碼 `d:\GPIB9045`，Big5／BCB6／pre-C++11）與 `HT9045.exe`（Handler；PMD、93K、HANA 文件裡寫成 `HandlerSys.exe`，ART 文件寫 `HandlerSys / H9045.exe`）。目前分析／可寫的橋接版本是 `GPIB_Code_32Site_V12.13.905.0_20260525`（20260926 起），883 版只拿來比對；其他版本目錄唯讀。〔BR §1、PMD §1〕
- 四個方向：Tester→橋接用 GPIB `ibwrt/ibrd`，`TSerialPoll::Timer1Timer` 輪詢、`ProcessStatusString()` 派發；橋接→Tester 用 `MyGPIBWrite(str, Task)`；橋接→Handler 用 `SendMSG_CMD()` 包 `GGpib2Handler` 送 `WM_COPYDATA` 給 `HMountWnd`；Handler→橋接由 `ProcessAddress()` 收 `GHandler2Gpib`。〔BR §2〕
- **查詢類回覆是原樣轉發**：橋接在 `ProcessAddress()` 把 `GHandler2Gpib->Message` 原封不動 `MyGPIBWrite` 回 Tester，字串內容與格式由 Handler 決定，橋接不加料。〔BR §2〕
- 橋接原本就有獨立執行緒：`Unit2.cpp` 的 `TMyThread::Execute` 每 1 ms `Synchronize(Process)` → `ProcessAddress()`＋`ProcessMessage()`；`ProcessMessage()` 的狀態機呼叫 `TestGPIB()`（Task 1／100／7000…，`ibwait(…,0)` 非阻塞）。`MyGPIBWrite` 等 TACS 最多 20×100 ms＝2 秒；`ProcessHMountConnect` 有 `SleepEx(1000)`。〔PLAN §1.2、BR §8.2〕
- 只用 9 個 NI 函式（`ibfind ibrsc ibpad ibtmo ibwait ibrd ibrsv ibwrt ibstop`），宣告在 `Decl-32.h`；`CreateMutex("MyMutexGPIB")` 防多開。〔PLAN §1.2〕
- 橋接用 `FindWindow` 找 Handler 視窗，存在 `HVisionWnd`／`HMountWnd`，`bFind` 表示已連上；GPIB 端 `cmydef.h` 的 `TOTAL_SITE=32`、`MAX_SITE_COUNT=32`、`StrLength=2560`。〔PMD §1、§5〕
- **機型白名單與補位**（`D:\GPIB9045\system\general.ini [Version] Model`；`Timer1Timer` 白名單、`ReadLastDataFile` 的 `MachineType`／`Application->Title`、`ProcessHMountConnect` 的 `FindWindow` 標題清單三處要一起改；`MachineType` 0／1／2＝`GpibString` 補 2／4／8 個 hex＝8／16／32 站；不在清單會記 `"Model" read error!!` 並自己關，缺鍵時先寫回 `ModelNG`；`SVON_CLOSE()` 內的舊清單沒有呼叫者）：**此處依機型分流，見 [ht9045.md](ht9045.md)／[ht9050.md](ht9050.md)／[ht9046-ls.md](ht9046-ls.md)**。白名單另有 9046GPIB、2601GPIB、9055GPIB、502GPIB、7080GPIB、1032GPIB 等非本 skill 三機型的字串（V906 裁決 2：全部照翻）。〔BR §1、P8 §2.4、PLAN §7〕
- **Handler 依 Model 字串決定 `MachineTypeChoice`**（`SYSTEM_MODULAR::ReadGeneralIni`）：此處依機型分流，見 [ht9045.md](ht9045.md)／[ht9050.md](ht9050.md)／[ht9046-ls.md](ht9046-ls.md)。〔PLAN §1.2、P8 §2.4〕
- 版本字串：`MessageDef.cpp` 的 `GPIBVersion`（橋接 12.13.905.0、RS232Standard 12.13.884.0；Handler 的 `bGpibRS232Error` 只比主次版 12.13）。V906 P0 已把移植樹升到 905。〔BR §1、§8.2、PLAN §1.3〕

## 2. IPC 結構與 MSG_CMD

- `VM`（橋接→Handler：`iCommand`、`Result[32]`、`bError`、`bEchoStop`、`cReturn[256]`、`GpibStatus[32]`、`GpibData[256]`、`GPIBBin`、`bOneCycle`）與 `MV`（Handler→橋接：`iSendCommand`、`Site[32]`、`bSimulate`、`bSupport32Bin`、`bCloseGpib`、`bTimeOutProcess`、`GpibAddress`、`MachineISRun`、`IsTest`、`bGpibMode`、`iLotStatus`、`HandlerHwnd`、`GpibHwnd`、`GPIBBin`、`iStatus[17]`、`Message[2048]`、`UseSiteMapData[256]`、`asATC_TYPE[32]`、`MultiMessage[4096]`）。〔PMD §2〕
- **結構規則**：只能在末端加成員、不可用 VCL 型別、`sizeof` 在 Handler／GPIB／RS232 三端必須一致並同步更版號；`cReturn[256]` 與函式九 `strncpy(…,544)` 有越界風險。〔PMD §2、§5、PM〕
- 函式一～九：Handler 端 `RunTestProgram(true, flag2)`、`SendMSG_CMD(CMD)`、`SendMSG_CMD(CMD, Msg)`（填 `Message[2048]`）、`SendMSG_CMD_DeviceMapSRQ(iStatus)`（帶 `iLotStatus`）、`SendMSG_TestMode()`（`GPIBBin=TestIF_File.iGpibMode`）；橋接端 `SendCaptureFinish()`（`MSG_CMD_NONE`＋`Result[32]`）、`SendECHO`（`Result[0..3]='E','C','H','O'`）、`SendMSG_CMD(CMD)`、`SendMSG_CMD(CMD, Msg)`（填 `cReturn`）。Handler 端 `SendMSG_CMD` 在 `bFind==false` 時直接 return。〔PM、PMD §4〕
- `MSG_CMD_*` 分類（基礎測試、機台動作、取得狀態、控制 GPIB、設定參數、ART、2D Barcode、溫控、SIGURD／Novatek 客製、其他、已棄用／只 RS232 用）與每一個值見 PMD §3；客戶專屬的代碼此處依客戶分流，見 [customers.md](customers.md)。〔PMD §3〕
- 橋接的 `slCmdList` 註冊順序索引＝`MessageDef.cpp` 的 `MSG_CMD_*` 數值，兩者必須一一對應。〔BR §3〕
- `enum eTestMode`（SingleSite 0 … `_32Site4X8N` 14）在 Handler `MachineType.h` 與 GPIB 端 `MessageDef.h` 各一份，要靠 §10 的同步維持一致。〔PMD §6、SYNC〕

## 3. 橋接的指令派發規則

- `ProcessStatusString()`（1,300 行 if-else，另有 RCMD／RFMD／Delta Castle 分支）以 `Str.Pos("XXX")==1` 比對，**大小寫敏感**；呼叫前 `buffer` 已被 `strupr` 原地轉大寫，例外是 `SGSETUP_` 開頭（刻意保留小寫）。〔BR §3、PLAN §1.2〕
- `SetParameter()` 剝前綴存到 `TempStr`／`Sitemapstr`：`SETTEMP +`（刪 9）、`SETTEMP_`（8）、`SETSOAK_`／`SETSOAK`（8／7）、`SETSITEMAP_`／`SETSITEMAP `（11）。〔BR §3〕
- case 600 的 `S=buffer` 不轉大寫、也不呼叫 `SetParameter`，混合大小寫的指令（`GetFFC?`、`GetTJFunction?`、`Contact Force?`、`SetSiteOnOff:`…）只有在 0600 才比得到。〔BR §3〕
- 剛結束一個 cycle 的 8 秒內，閒置時不讀匯流排（`IsTestDelay`）。〔P8 §3.4〕
- 橋接自己回答、不經 Handler 的問句：`VERSION?`（`general.ini [SystemSetup] Version`）、`CHKMATCH?`／`CHKMACH?`（`Gerneral.ini [Version] Model`；`RETURN_GPIB_VERSION=1` 時回版本字串）、`*IDN?`（`HONTECH`）、`HOSTNAME?`、`HANDLERID?`（Handler config I25 開才回 `Machine ID`，關時不回）。`SETSOAK?`、`SITEMAP?`、`HANDLER ID?`／`ID?` 經 Handler，回覆格式待上機確認。〔P8 §3.2〕

## 4. 標準測試循環、BIN 與 2DID 格式

- 一個 cycle：Handler `SRQ 0x41` → Tester `FULLSITES?` → Handler `FULLSITES xxxxxxxx`（8 個 hex，每 4 個 channel 一個 hex digit）→ Tester `BINON:…` → Handler `ECHO: <同內容>` → Tester `ECHOOK`（相同）／`ECHONG`（不同）。〔STD〕
- `BINON` 由右至左：最右＝Site 1、最左＝Site 32；關閉的 channel 只能是 `0` 或 `A`。〔STD §4〕
- Bin 字元表：Advan Type 1（0～15，每 channel 1 byte、每 8 byte 一個逗號）、16 BIN（0～16，16＝G）、32 BIN（16～32＝G～W）、254 BIN（每 channel 3 byte、每 24 byte 一個逗號，255＝Error Bin）、GS16（每 channel 各自逗號，Reject＝A）、GS32（1～2 byte）、Advan T6577（V3.28.588 之後，Reject＝A）。配方 `[GP-IB] Type`：0 ADVAN_Type1、1 256Bin、2 16Bin、3 32Bin、4 SPEA、5 16BinGS、6 32BinGS、7 15BinT6577、8 15BinQorvo、9 Delta_Castle（8、9 此處依客戶分流，見 [customers.md](customers.md)）。〔STD §4.3、P8 §2.4〕
- golden 坑：GPIB 16BinGS 的 BINON 永遠不過、256 bin 只看第一位數；上機不要先用 16BinGS／32BinGS／256 bin。〔BR §8.6、P8 §8〕
- 2DID：`BARCODE?` **倒序**（最右＝Site 1，關閉 Site＝`0`）；`GET2DID?` 正序；`QRC?` 正序，無 IC `@`、未讀到 `$`、讀取失敗 `#`；`GETBARCODENUMBER` 回 `<3 位數長度><Site1_2D,…><checksum>`（無資料＝`NA`）。〔STD、CL〕
- 各章指令（Handler Information、Recipe、Site Map、Bin Map／Yield、Temperature & Soak、Index Arm、Remote Control：`PAUSE`／`SETHANDLERDOPAUSE`／`PAUSE_01`／`STOP_01`／`ONECYCLE`／`AUTO_CLEAN`／`OVERDRIVE`／`RECONTACT`、Device Map：`PICKLOAD`／`PLACETOLOAD`／`TRAYFEED`、ART、ATC Control、ATC 6.0、ASIF、Data Collection、Tester Control `SVID`／`ECID`／`RCMD`；`CEID` 與 S5F1 尚未支援）見 CL；原始規格 `HT9xxx GPIB_Command_FullVersion_V12.13.884.docx` 沒進版控，在 `d:\GPIB9045\.github\skills\gpib-command-list\references\`（二進位規格，不在 git：`D:\GPIB9045` 不是 git repo，只在 GPIB9045 工作區）。〔CL〕

## 5. 溫度指令與 `SETTEMP?` 回覆格式

- 指令對照（Tester → 橋接 → Handler）：`SETTEMP?`→`MSG_CMD_HandlerTemperature`(65)；`SETTEMP +<n>`／`SETTEMP_<n>`→`MSG_CMD_SetTemp`(73)；`SETTESTTEMP +<n>`→`MSG_CMD_SetTestTemp`（值兩份舊檔不一致，見 BR §3 與 PMD §3.8）；`DEVICETEMP`／`SETTESTOFFSET_`→`MSG_CMD_SetTJ`(75)；`SETSOAK?`／`SETSOAK `／`SETSOAK_`→`HandlerSoakTime`(64)／`SetSoakTime`(74)；`GETNOWALLTEMP?`→`MSG_CMD_GetNowAllTemp`(69)。〔BR §3、PMD §3.5、§3.8〕
- `SETTEMP?` 的字串由 Handler `TfMain::TempDataStrings()` 組，看 config `[Tester] bI38SETTEMPRespondSetTemp`：0（預設）`+25.0`、加熱時 `+<設定溫度>.0`；1 `Settemp +25.0`；2 整數 `25`；橋接原樣轉回再補 `" \r\n"`。鍵在 `TfConfiguration` 註冊（預設 0），實機值在 `config/config.ini`；SECS ECID 35548「[I38] Format of SETTEMP?」也能改。各值對應哪個客戶：此處依客戶分流，見 [customers.md](customers.md)。〔BR §4、P8 §3.2〕
- 「`pgm or MES Temp=25 , Handler Temp=Settemp +25.0` / `Temperature check fail`」是 Tester 端 pgm／MES 印的：Handler 旗標被設成 1、Tester 期望純數值。修設定（改 0 或 2），不是改程式；任何版本的 GPIB 程式都不會自己加 `Settemp` 前綴。`HANA_ART.cpp` 的 `Handler Temp : %f` 是另一個來源，不要混。〔BR §4、§7〕
- `SETTEMP` 可設定的上限（`TfTemp_Set::MaxTempSetting`；超過回 `VALUE NG`）：**此處依機型分流，見 [ht9045.md](ht9045.md)／[ht9050.md](ht9050.md)／[ht9046-ls.md](ht9046-ls.md)**；`CC_SCC` 例外見 [customers.md](customers.md)。〔C19 §1 第 50 列：`C:\AI_TempFile\st02e-scratch\c22_src\ST02_C19_TYPE9050_SWITCH_20261005.md`〕

## 6. 介面種類（GPIB／RS232／TTL／TCP）與 Off-Line

- 配方 `Tester.Data [Mode] Tester Type`＝`TestIF_File.iTestType`：0 TTL_MODE、1 GPIB_MODE、2 RS232_MODE、3 TCP_IP_MODE（超出範圍改寫成 1）。GPIB 與 RS232 是獨立 exe（同一套 `MessageDef.h`，`MyDutPanel.cpp` 只差 Bin 下拉）；TCP/IP 是 Handler 內建 `TfTesterTCP`（Handler 當 client，`LastSet.iTester==ON_LINE` 才連、30 秒重試）。〔P8 §2.4、PLAN §1.1、§1.4〕
- `g_iTestType`（**介面種類**，TTL／GPIB／RS232）與 `LastSet.iTesterMode`（**GPIB 子協定**，`InterfaceType_*`）職責分離；`iTesterMode` 只在 GPIB 模式有意義；禁止新增 `InterfaceType_RS232Standard` 這類混用的 define。〔MRG、DD-001〕
- Handler 用 `HHandler2Gpib.iLotStatus` 欄位複用傳 `iTestType`（只限 `MSG_CMD_TesterMode`／`MSG_CMD_ChangeGpib`，不動 `MSG_CMD_LotStatus`）；**每個 `WM_COPYDATA` 送出點都必須設 `iLotStatus`**（POD 預設 0＝TTL_MODE，漏設就是 BUG-001「Change tester interface: TTL ()」）。〔MRG、DD-005、BUG-001〕
- `ProcessHVisionConnect()` 每秒跑：用 `static HWND sLastHVisionWnd` 只在 NULL→非 NULL 時呼叫一次 `SendMessageToGpibProg()`，斷線歸零以便重連再送；`WakeupGPIBdelay.Off()` 不能當 one-shot。〔DD-004、BUG-004〕
- GPIB_RS232 合併版（BCB，`GPIB_RS232_Code_32Site_V12.13.900.0_20260331`）的設計：`g_iTestType` 從 `D:\GPIB9045\system\general.ini [SystemSetup] iTestType` 冷啟動還原、關閉時寫回；log 統一在 `D:\GPIBLOG\{Log,RS232_Log,RS232_BinLog,RS232_BinLog_TTL}`；RS232 log 同步進 GPIB log 視窗；C1～C10 修改清單；BUG-002～005（FormDestroy 懸空指標、Timer 先於 `MY_DUT_PAL` 初始化、每秒重送、模擬時「Closed Site Have Bin ERROR」）。那棵樹不在磁碟上，設計留在 skill。〔MRG、BUG、PLAN §1.7〕
- DIO：golden `atester.cpp` 的 `TTL_MODE && TTL_CARD_TYPE<2`「直接讀 DIO 卡」分支新機台不適用、不移植；C++ 線的 DIO＝RS232Standard 的 TTL 板模式（`Gerneral.ini [System] TTL_CARD_TYPE` 2＝一塊、3＝兩塊；`TTL_CARD_USE_ADDRESS`）。〔PLAN §1.5、BR §8.5 第 7 條〕
- Off-Line：Handler 送 `bSimulate=(LastSet.iTester==OFF_LINE)`；V906 的 Off-Line 一律走 GPIB 引擎的 simulate、用 GPIB 的設定（`OffLineGpibWay()`／`EffectiveBridgeTestType()`），RS232／TTL 配方 Off-Line 時不開 COM（跟 golden 不同）。Off-Line 不代表離開匯流排，閒置聽匯流排那條路不看 `bSimulate`，問句照樣會回。〔BR §8.7、P8 §2.2〕

## 7. ART（Auto Retest）

- 誰驅動：Handler 每個 lot 階段呼叫 `SetLotState(N)`（N＝2 FT Lot Start、4 RT Lot Start、8 Lot End、10 Final Lot End）→ 橋接設 `iLotStatus=N` → Tester 送 `SRQKIND?` 時回 `SRQKIND N`，並在對應時機送 `SRQ 0xC0`。`SetLotState` 進入時兩道 guard：`TestIF.iTestType==GPIB_MODE` 且 `fMain->bFind==true`，任一不成立就不送。〔ART〕
- 開關：[A10-1] Enable ART（`IniConfig.bA10_AutoReTest`）、`CosFunction.bUseSCKART`（共用預設 true）、`bAutoRetestGPIBmode`（`iTesterType==1` 時 true，動態重算）、`fSCKART->iTesterType`（1＝93K GPIB ART、0＝Flex／TCP）。Tester 送 `SRQMASK` → `MSG_CMD_SCKART_SRQMASK` → `iTesterType=1` 並重算；送 `LOTSTATUS?` → `MSG_CMD_SCKART_LOTSTATUS` → `iTesterType=0` **但不重算** → 可能出現 `iTesterType==1` 卻 `bAutoRetestGPIBmode==false`。〔ART、93K〕
- 93K GPIB 端流程：`SRQMASK` → `FR?`（5 秒等 Handler 版本字串）→ `LOTCLEAR?`（`LOTCLEARED`／機台運轉中回 `SETTINGNG`）→ `INPUTQTY NN,XX[,stepcode]`（`SETTINGOK` 後 `DoARTLotStart()`、Step 1）→ SRQ／`SRQKIND 2` → `LORORDER NN,XX`（`MSG_CMD_LotStatus`）→ FT 循環 → `SRQKIND 8` →（需要 RT）`SRQKIND 4` … →`SRQKIND 10` → `LORORDER 2` → Tray Feed、Step 12。Flex 的 `INPUTQTY XX,NN` 欄位順序相反並回 `ECHOQTY`。〔93K〕
- `iCurrent93KARTStep` 0～12 對應 UI LED（`aledInitART` … `aledMoveTrayToLoader`）；`TfSCKART::CheckNeedRT()`：`iFTRTCount > iSCKART_TryCnt`→0、`==`→2、`<` 且 `dCurrYield < dSCKART_Yield`→1、Yield 達標→2、Low Yield Alarm 選 Tray Feed→0；第一次 RT Step 5、之後 Step 10。設定在 `Automation/SCK_ART.ini`。〔93K、ART〕
- **FT Lot End 三岔**（`DoART_AfterCleanOut()`）：#1 `bAutoRetestGPIBmode==true` → 正常送 `SetLotState(8)`／`(10)`；#2 `bUseSCKART && iTesterType==0` → TCP／Flex ART；#3 其他 → SECS/GEM 結批或 `ShowMyMessage("Please Print Summary ", "請結批報表")`（單 OK 鈕），**不呼叫任何 `SetLotState`** → Tester 等不到 SRQ 0xC0 → ART Alarm。另一個 YES/NO 的「請先在 TESTER 結報表」是 Tray Feed 客製訊息，無關。〔ART〕
- **第二失效模式 `LastSet.bDummyART`**（GPIB 端）：`bDummyART==true` 時只寫 `Dummy FT ==> SRQ:0xC0`、不 `ibrsv` → 症狀相同。它不是 `general.ini [Auto Retest] ART Simulator`（只管面板可見），也不是全域 `bSimulate`。`bDummyART` 由 Handler 的 `MSG_CMD_SCKART_RunDummy` 帶的 `bSimulate` 決定（Off-Line→true；On-Line 且 `bUseSCKART && bA10_AutoReTest && bSCKART_EnableART && bSCKART_RunARTWithoutCmd`→true；其他→false），只在 lot start 與 ON/OFF Line 切換時送，是持久狀態；風險 A（現場誤開 `bSCKART_RunARTWithoutCmd`）、B（Off-Line dummy 後切 On-Line 沒重算）。〔ART、handler-art-flow §6〕
- `DoInitialStart()` case 1：純 GPIB 93K ART 每批送 `RunDummy`（自我修復）；SECS/GEM（`bART_SECSGEM_93K`）或 TCP ART（`iAutoRetestTCPmode!=0`）且 `RunWithoutCmd==false` 原本走空 if、不重算；905.x（Steven 20260612）補成「乾淨開批（`HasICUnderMachine()==false && HasAnyICInMachine()==false`）時送 `RunDummy`」。`RunWithoutCmd==true` 是合法的自走模式，不可擋。〔ART、handler-art-flow §7〕
- DummyART 模擬器 `TfDummyART`（`DummyArt.cpp`，HP93KART／FLEXART）：`SRQKIND?` 強制回 `SRQKIND 2`、`INPUTQTY` 只回 `ECHOQTY…`。Run Mode `ContinuStart_ART`＝`rsmContinuRetest_ART`。〔93K、ART〕
- V906：配方 `Tester.Data [AutoRetest] iTesterType` 是 Handler 端的值（W1＝(b)，`forms/fSCKART` 的 `AccessFile`），缺鍵＝1（偏離 golden 的 0），開機／換配方會把 1 寫進沒有這個鍵的配方；`INPUTQTY` 的廠牌跟 Handler 目前的值（3A）。〔BR §8.7 P6〕
- ART 的客戶變體（Hana、RENESAS、TESNA 案例）：此處依客戶分流，見 [customers.md](customers.md)。原始流程圖：`ht9045-art-flow/references/ART on HT-9xxx - With GPIB log.pptx`、`ART on HT-9xxx.pptx`。〔ART〕

## 8. ATC（Handler ⇄ ATC 溫控設備，TCP）

- 檔案：`ATC/ATC_Handler_Side.cpp/.h`（主模組，所有 1001～1137 的 case）、`ATCInterface`（ATC 7.0 以前的相容介面與 `TMyHonPrecATCPanel`）、`ATCSystem.h`（`HT_ATC`）、`TCPData`（底層 TCP）、`WinWaySetting`、`uTemp_Set.cpp`（Site／Channel 對應）、`TempCtrl/TriTemp.cpp`；全域物件 `ATC_InterfaceForm`。〔ATC、ATCI §2〕
- 封包：`@<Code>,<DataCount>,<D1>,…,#`（雙向同格式；`0001`＝ATC_NO_THIS_COMMAND）；溫度乘 10（`55`＝5.5 ℃）；`ProcessReceiveString_ATC()` 把結果放進 `dATC_Result[]`；延遲回傳靠 `bCommFlag[Code-1000]`＋Timer 輪詢；命名 `ATC_*`／`HANDLER_*`／`ASIF_*`。〔ATC、ATCI §1〕
- **`iATC_MODE_TYPE` 是 ATC 設備型號，不是 Handler 機型**：20／21／30／31／32／33／35／36／50／51／60／61／70，9999＝未連線；連線後送 1009 取得，存在 `D:\HT9045\system\ATC.ini [System] iATC_MODE_TYPE`（20261001 更正；`Config\ATC.ini` 只是開機就被蓋掉的預設）。ATC 3.6 硬體跟 3.5 共用軟體 V1.2.1，也回 35，屬正常。`iATC_MODE_TYPE==35` 時 `ProcessHandlerCommand()` 先丟進 `SendToATCBufferQueue`，由 `ProcessATC_BufferCommand()` 處理。〔ATC〕
- 常用 API：`Run`／`Stop`（1007）、`ChangeRecipe`（1001）、`SetAllTemp`（1002）、`SetSingleTemp`（1015）、`SetOffset`（1003）、`SetSingleOffset`（1016）、`EnablesChannel`（1004）、`StartTesting`／`TestFinish`（1012）、`LotStart`（1050，需 ≥31）、`LotEnd`（1051）、`ReadTC`、`ReadTJ`、`SendCommand`。〔ATC〕
- 版本保護：6.0／7.0 封鎖 1038／1067（`ATCCONTROLMODEMode` 末段）、1071、1084、1085、1087、1088、1125（函式開頭 early return），呼叫端另封 1087／1088（`MSG_CMD_RECODETJ`）、1104（`TfLotInfo` case 17），1105 限 Rogers（TYPE_33）；6.0 的 1076／1077／1100 會送但 ATC server 沒有 case、不回 ACK。**版本專屬命令一律白名單**（例：1127／1128 Dynamic PID 只給 35／36）。〔ATC〕
- 已知缺陷：1074 `SendCommandToHandler` 無 case；3.5 的 1076／1077 不發 ACK；Rogers 1080 無 case；case 1044 缺 `break`（fall-through 到 1045）；1085 只設 flag、等 Timer 才回。修時要補 `SendCommandToHandler` 的 case。〔ATC、Handler_Command_Report.md〕
- `SendReadTempComm()` 質數錯峰：1010 每秒；1025 `%2==1`；1024 `%9==2`；1035 `%9==5`；1067 `%30==7`；1128 `%30==19`（35／36 白名單）；1071 `%60==23`；1036 `%60==41`（取代 `bGetNowRecipeFlag` 每秒輪詢）；1033 `%300==137`；`iCount>=3600` 歸零；穩態約 1.7 個／秒。`ChangeRecipe`（1001）仍是事件觸發。〔ATC〕
- 新增命令：頂部 `#define` → `SetCommandString()` → `SendCommand()` case → `ProcessReceiveString_ATC()` case → 需要時 `bCommFlag` → 更新 `Handler_Command_Report.md`；`ATC_MAX_COMMAND`＝150（超過 1150 要改）。〔ATC〕
- Site／Channel 對應（`InitSiteToATC`，另一支 skill 說實名是 `InitialAddrToATC`，見報告矛盾）：`iSiteToATC[Arm][Row][Col]`、`iSiteToOfs[Arm][Row][Col]`、`iATCToAddr`、`iAddrToATC`、`iATCToSiteArm／Row／Col`；`dATCTempOffset[]` 32 組（Arm1 `j+i*8`、Arm2 `j+16+i*8`）；V899 的 `iSiteToOfs[1][0][0]=24` 筆誤；`tcAa1`…`tcBd2` 命名：A／B＝Row1／Row2、1／2＝Arm1／Arm2。〔ATCI §5〕
- Offset 呼叫路徑：UI → `Temperature.dATCTempOffset[0..31]` → `TfLotInfo::SetATCOffset()` → `ATC_InterfaceForm->SetOffset()` → `SendCommand(ATC_SET_TOFS)`（`@1003,…`）。除錯：統計 hangup 資料夾 TXT 的 `@1003`、對 `EventLogTxt_*.csv` 的 ChangeLog、查對應表；不回應先送 `@1048`；Recipe 用 `@1036`／`@1037`／`@1001`，新版 `@1132`＋`@1134`（`FileTransfer.cpp`）。〔ATCI §4、§6、§7〕
- Multi Zone（`cbMultiZoneFunction`，35／36 可用）：`EnablesChannel` 要把所有 Zone channel 都 Enable；SingleSite 只對到 1 個 channel 的缺陷在 r896 修。〔ATC、MultiZone_EnableChannel_Bug.md〕
- 其他機型專屬：1058／1059 標「for HT3012CT」（不在本 skill 三機型內）。客戶專屬 ATC 命令（TSMC、MTK、Qualcomm、AMD、ASE_CL、勝麗、ANST、HYGON…）：此處依客戶分流，見 [customers.md](customers.md)。〔ATCI atc-commands.md〕

## 9. Handler 的 TCP 指令伺服器 7016／7017

- 不是 GPIB 橋接的一部分：Handler 自己兩個 `TServerSocket`（`TCPCommandServer`／`TeraTCPResultServer`），`HanderTcpIp()` 寫死 7016／7017；只在 `CosFunction.bEnableHandlerResultServer` 開（此處依客戶分流，見 [customers.md](customers.md)）。〔7016 §1〕
- 收：`TCPCommandServerClientRead` 一次讀＝一個指令（`char EthernetBuffer[100]`，≥101 byte 蓋堆疊），最多 4 欄；`HTGR,<id>,…` 查詢 42 個＋`HTSET,<id>,…` 設定／動作 56 個；回 `HTSR,<id>,<欄位>,`（沒有 CR/LF），`HandlerTCPIPResultSendProcess` 廣播給所有 7016 client；未知指令回空字串；log 在 `D:\HT9045_Log\TCPIP_Log\`。〔7016 §1〕
- V906 W10：`vclcompat/ServerSocket` POLLED 模式（tick 執行緒上觸發、不開執行緒）＋`CmdServerPump`（第一個 pass＝golden FormShow 的 `HanderTcpIp`）＋`TcpCmdFramer`（每連線緩衝、依 id 欄數切框、2048 byte 上限整段丟、不回 NG）；`HTSET,333`／`334` 只經 `W906_RemoteRun` 到 `StartFromWeb`／`PauseFromWeb`，不碰基底 `TfMain::Start`；兩台物件就算不 listen 也一定要先存在（`W906_CmdServersEnsure()`）。還沒開：701、7017 的 501 推播；354＝A、702 例外＝A。〔7016 §2～§4〕

## 10. 三邊定義同步（HT9045 ⇄ GPIB9045 ⇄ RS232Standard）

- 三組定義：`CC_xxx`（`MachineType.h` → `cmydef.h`）、`MSG_CMD_*`（`MessageDef.h/.cpp`）、`enum eTestMode`（`MachineType.h` → 對方 `MessageDef.h` 的獨立 enum）；不一致會誤判客戶、Handler 不回應或 BIN 分錯。〔SYNC〕
- 做法：先問三個版本資料夾（RS232 可「跳過」）→ 備份 → Agent A 做 CC_（只寫 `cmydef.h`）、Agent B 依序做 MSG_CMD 再做 eTestMode（都寫 `MessageDef.h`）→ 驗證 missing=0、diff=0 → 寫同步歷史。約束：Big5 一律 bytes 讀寫、BCB6 語法、**永遠以 HT9045 為基準、不反向、不刪目標多出的定義**、`.bak_YYYYMMDD` 備份、臨時腳本放 `D:\AI_TempFile\`。〔SYNC、sync-procedures.md〕
- V906 只有一份 `MachineType.h`／`MessageDef.h`，這類不一致結構性消失；BCB 線仍要照做。GPIB 與 RS232 兩支 exe 的 `MessageDef.h` 逐行相同，`MessageDef.cpp` 只差版本字串。〔PLAN §1.1、§1.7〕

## 11. V906 TesterComm（as-built 要點）

- **一條通訊執行緒**（裁決 9）：`TesterCommThread`＋`TesterCommHub` 依 `TestIF_File.iTestType` 只起一個引擎：GPIB_MODE／TCP_IP_MODE → `gpibbridge::GpibEngine`（golden 在 TCP 模式也開 GPIB exe）；RS232_MODE／TTL_MODE → `rs232std::Rs232Engine`；引擎自己關掉時 `Restart()`（＝golden `WakeupGPIB`）。權威帳本是 V906 樹 `docs/TESTERCOMM_PORT_LEDGER.md`。〔BR §8.6〕
- 橋接程式整支照 golden 翻（`TesterComm/Gpib/`＝H9046_32GPIB V12.13.905.0、`TesterComm/Rs232/`＝RS232Standard Rev12.13.902.0，各自 namespace，vclcompat headless 元件）；行程內替身：`SendMessage(WM_COPYDATA)`→`SyncMailbox`（同步，等待中處理對方送來的，可雙向巢狀）、`FindWindow`→代號、`Close()`→旗標、每次啟動重設全域與函式內 static。〔BR §8.6、§8.2〕
- NI-488.2：`LoadLibrary("gpib-32.dll")`，12 個進入點全找到才用；沒有驅動時每個 `ib*` 回 ERR，golden 自己記 `Error Open GPIB0`；`SimGpibDriver` 只在 ctest 注入；testercomm 頁「驅動」欄 `ni-gpib-32.dll`／`none`。〔BR §8.6、P8 §2.2〕
- Handler 端 `TesterComm/Handler/THandlerTesterSide`（golden 的 case WM_GPIB_Program、`SendMSG_CMD`、`SendMSG_TestMode`、`RunTestProgram`、`CloseGpibProgram`、`ProcessHVisionConnect`、`WakeupGPIB`）；fMain 門面經 `forms/fMain.h` 檔尾的 `W906_TesterForward` 轉過去。〔BR §8.6、§8.7〕
- 執行緒隔離（裁決 11）：只有信箱／快照／指令 inbox 三個接點、雙向禁呼清單、tick 最多被擋一拍、分開的 watchdog；GPIB 執行緒只寫 `GpibSnapshot`，tick 在 `PublishExtraTags()` 複製後 publish；GPIB IO 即時、畫面可延遲（裁決 1）。〔BR §8.2、§8.5〕
- 906／912：20261003 起以 906 為準（RULINGS_20261002 第 20 條、第 23 條第 6 項），少數照 Steven 1003 常設規則保留 912（註「#20 exception」）；溫控（#20a）與 HANA（#20b）照 912。客戶專屬的 906／912 差異此處依客戶分流，見 [customers.md](customers.md)。〔BR §8.7〕
- 其他已落地：`SetTestTimeOutTimer` 照 golden 翻（修 On-Line 一送 SOT 就 WAR07352）；`ChangeTesterConnect` 本體與 D1／D2／D3／D4／D6／D7 規則（D5 不做）；網頁 `act.main.testerConnect`；`SyncBridgeSettings`；W9 遠端溫度 offset 寫配方；Q2 (a) GPIB 額外 RS232 port 跟配方走；`testercomm.html` 共用 `makeDutPanel`（150×110、8×4 排列、母表單色 `#408080`）。〔BR §8.4、§8.7〕
- 陷阱：vclcompat 的 `X->Click()` 是空的、`Strings[i]` 是代理物件、`AnsiString==0` 在 BCB6 是 `=="0"`、`AnsiString+=int` 是十進位字、`TComboBox` ItemIndex 與 Text 不連動；BCB6「數字 != AnsiString」是數值比較（NUMCMP）；匿名 namespace 裡的區域 `extern` 連不到真全域；golden RS232Standard 902 資料夾是 `#define DEBUG` 快照（翻譯預設關）。〔BR §8.6、§8.7〕
- `HT9045_TESTERCOMM=0`：整個不裝，只給「不能看到 bridge」的 SIM 回歸；有 IC 測試循環的 SIM 回歸不能用（沒有 bridge 時 `GetTesterResult` 照 golden 每 3 秒重試、不前進）。ctest：`TesterComm_IPC`／`_GPIB`／`_RS232`／`_Handler`（E2E 要 `HT9045_TESTERCOMM_E2E=1`）／`_TestTimeOutTimer`／`_P6GpibAux`／`_TcpCmdFramer`／`_TcpCmdServer`／`_TcpCmdFieldsCensus`；會寫 `D:\GPIBLOG`、`D:\RS232Log` 的完整生命週期測試要手動開。〔BR §8.7〕

## 12. 上機 bring-up 通則（P8）

- 一定用出貨組態（`-DW906_NO_SOFT_SIMULTE=ON`，`build.bat serve`）：模擬組態下感測器一律 ON、TTL 的 `InitDIOStstus` 不做、D2「機台有 IC 不准切 On/Off-Line」不擋。GPIB 驅動的選擇跟組態無關。〔P8 §2.1、§2.2〕
- 順序：GPIB 只送問句 → 一個 SOT／EOT cycle → 32 站 BINON；RS232 同順序；TTL 板先 `WINIT` 再 `WSOTS`；不要先做兩塊 TTL 板。授權照 BU 戰役三層級。〔P8 §0、§3～§7〕
- 先備份會被寫的檔：`D:\GPIB9045\system\general.ini`、`GpibString.dat`、`D:\HT9045\system\Gerneral.ini`（兩個橋接缺鍵寫回預設）、`D:\RS232Standard\System\Setup.ini`、配方 `Tester.Data`／`Temperature.Data`／`TestMode.Data`、`lastdata.dat`。〔P8 §2.5〕
- 上機前缺口 B1～B5 已在 20260928（gpib-widget `35d17d44`）做掉；B6（On/Off-Line 用哪個網頁入口）、B7（testercomm 網頁指令沒有權限閘）仍開著；待上機確認 U1～U13。〔P8 §1、§9〕
- 停止／還原：切回 Off-Line（`CloseGpibProgram` 後依新模式重起），或關 wb_serve、或以 `HT9045_TESTERCOMM=0` 重開，再放回備份。〔P8 §3.4〕
