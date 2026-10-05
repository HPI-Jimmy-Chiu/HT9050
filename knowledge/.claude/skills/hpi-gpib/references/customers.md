# hpi-gpib 客戶分流表

| 客戶碼 | 函式／Task | 開關（INSTALL_／USE_／FUNC_／CC_…） | 行為一句話 | 906／912 來源 | 機台 |
|---|---|---|---|---|---|
| `CC_HANA_MICRON`（865） | Handler `uHANA_ART::DoCmdWhenHDStart`、`DoSetUpInfo`、`ParseLOTONStr`、`EndPrimeTest`；橋接 `TSerialPoll::InitHANA_ART`、`InitializeSRQCodeMap`、`DoSendCommand` | `General.ini [ART] bRunHANA_ART`／`bTryHANA_ART`；`MSG_CMD_RUN_HANA_ART`、`MSG_CMD_HANA_ART` | Hana 專屬 ART：DEVON 逐項驗證（0x56／0x57）、LOTON、Prime／Retest、SRQ 0x00～0x79、TDATA～ADATA 統計回報 | `HANA_ART.cpp/.h`（V3.33.900）＋GPIB `Main.cpp`（V12.13.900）；`gpib-hana.md`、`hana-art-protocol.md` | `ID?` 範例回 `HT9046,HT9046-01,HANA` |
| `CC_HANA_MICRON`（865） | 橋接 Dummy Test：`MSG_CMD_HANA_ART` 帶 0x42 → `SendCaptureFinish` | `bHanaDummyTest` | 0x42 會照一般 Fullsites 跑一次測試循環，結束回 `DUMMY_TEST_0x42_OK` 而不走 BIN 分類 | GPIB `Main.cpp`；`gpib-hana.md` | — |
| `CC_HANA_MICRON`（865） | `THandlerTesterSide::WakeupGPIB` 的 `PrepareHANARMSConnect`（G9） | — | V906 由 C10（MR !126）解閘 | 照 912（RULINGS_20261002 第 20b 條）；`ht9045-gpib-bridge.md` §8.7 | — |
| Qorvo（`[GP-IB] Type`＝8，`InterfaceType_15BinQorvo`） | 橋接 `ProcessStatusString`：`CONFIGURE,SRQ`／`FULLSITES?`／`CONTACTOR`、`FULLSITES?`、`BINON`／`BINOFF`、`ECHOOK`／`ECHONG`、`QRM?`／`QRC?`、`PAUSE`／`RESUME`、`REQUEST,CHECKEMPTY` | `LastSet.iTesterMode`、`LastSet.bConfigureSRQ`、`LastSet.bHaveContactorInfo` | SPE-001712 Rev A：GPIB 位址固定 1、ECHOOK 後才分料、二次 ECHONG 全送 Flush Bin、`A`＝縮回重壓、ESC 用 SRQ44 | `SPE-001712 Rev(A)_Gpib command.docx`（未進版控）；`gpib-qrovo.md`、`qorvo-protocol.md`、`message_2020_07_30_10.TXT` | — |
| Qorvo | GPIB_RS232 合併版 `MSG_CMD_TesterMode` handler（C7） | `g_iTestType==GPIB_MODE` guard | 非 Qorvo 時清 `bConfigureSRQ`／`bHaveContactorInfo`，Qorvo 時 `LastSet.iTesterType=1` | `rs232-merge-details.md` C7 | — |
| Qorvo | `MSG_CMD_ESC`(96)、`MSG_CMD_RESUME`(97)、`MSG_CMD_TestAlarm`(98)、`MSG_CMD_QRA`(178) | — | ESC 觸發、RESUME、測試警報（Steven 20201022）、Qorvo ART 啟用確認（Steven 20241004） | `GPIB_Program_Manual_V12.04.md` §3.1、§3.10 | — |
| Qorvo | V906 P6 3A：`INPUTQTY` 的廠牌 | — | Qorvo 固定 1、DummyART 用自己學到的值 | `ht9045-gpib-bridge.md` §8.7 | — |
| Qorvo | Tester Pause 蜂鳴 | — | V906 改回 906＝馬上響（MR !132）；912 的延遲要不要回來等 Steven | 906 `main.cpp`（引述）；`ht9045-gpib-bridge.md` §8.7 | — |
| Advantest 93K（`iTesterType=1`） | Handler `TfSCKART::DoARTLotStart`、`CheckNeedRT`、`DoART_AfterCleanOut` 分支 #1；橋接 `SRQMASK`／`FR?`／`LOTCLEAR?`／`INPUTQTY NN,XX`／`SRQKIND?`／`LORORDER` | `CosFunction.bUseSCKART`、`bAutoRetestGPIBmode`、`IniConfig.bA10_AutoReTest`（[A10-1]）、`TestIF_File.bSCKART_EnableART` | GPIB ART：SRQKIND 2／4／8／10 驅動 FT／RT／Final，SRQ 0xC0 | `SCK_ART.cpp/.h`（V3.33.900）；`gpib-93k-art.md`；`ht9045-art-flow.md`（base_r902） | — |
| Flex（`iTesterType=0`） | 橋接 `LOTSTATUS?`、`INPUTQTY XX,NN`；Handler `DoART_AfterCleanOut` 分支 #2 | — | `LOTSTATUS?` 設 0 但不重算 `bAutoRetestGPIBmode`；INPUTQTY 欄位相反、回 `ECHOQTY` | `gpib-93k-art.md`、`handler-art-flow.md` §3 | — |
| RENESAS | `uRENESAS_Server`（CMD 10／20／30／40／50／70；case 23000、1436、1578） | — | FT-CT ART，與 93K 共用 `fSCKART->iCurrent93KARTStep` | `Automation/uRENESAS_Server.cpp`；`gpib-93k-art.md` | — |
| SIGURD | 橋接 `CHECKLIST`、`SGFTP_`、`NONDOUBLEBIN_`、`BINCOUNT_`、`SGOSBIN_`、`SGCONTFAIL`、`SETTESTERID_`、`BINPOS_`、`SETAICCD_`、`SGSETUP_`；`MSG_CMD_*`(133～150、156～160、173、177、179、180) | — | SIGURD 專用指令集（Check List、FTP 自動化、Tester ID、Shuttle mode、Max Test、AI CCD、OS Bin） | `gpib-command-list.md`；`GPIB_Program_Manual_V12.04.md` §3.9 | — |
| Novatek | `MSG_CMD_GetAutClean`(152)、`MSG_CMD_ForcePerPinN`(153)、`MSG_CMD_ContactHeight`(154)、`MSG_CMD_YieldContinusFail`(155) | — | `AUTOCLEAN?`／`DEVICEFORCEPERPIN?`／`ARMCONTACTHIGHVALUE?`／`YIELDCONTINUESFAIL?` 查詢（Sam 20220408） | `GPIB_Program_Manual_V12.04.md` §3.9；`ht9045-gpib-bridge.md` §5 | — |
| TSMC | `MSG_CMD_OverDrive`(35)、`MSG_CMD_ReContact`(36) | — | `OVERDRIVE`／`RECONTACT`（Steven 20151207） | `GPIB_Program_Manual_V12.04.md` §3.5 | — |
| TSMC | 橋接 `DEVICETEMP`／`SETTESTOFFSET_` → `MSG_CMD_SetTJ`(75) | — | Tj／offset；V906 W9＝A 照 golden 寫配方 Temperature.Data，會累加，寫成功後照 912 重載 | 912（#20a 邊界案例）；`ht9045-gpib-bridge.md` §3、§8.7 | — |
| TSMC | `eTestMode` `TriSite1X3`(2) | — | 3 Site（1x3）測試模式（Frank 20160104） | `GPIB_Program_Manual_V12.04.md` §6 | — |
| TSMC | ATC `SetDynamicPID`／`ReadDynamicPID`（1127／1128）、`SendReadTempComm` | `iATC_MODE_TYPE==ATC_TYPE_35 \|\| ATC_TYPE_36`（白名單） | Dynamic PID 只送 ATC 3.5／3.6 | `ht9045-atc.md` | — |
| TSMC（TSMC_Hulk） | ATC 1137 `ATC_HulkMode` | `iATC_MODE_TYPE==35` | Hulk 模式開關（20260402） | `atc-commands.md`、`ht9045-atc.md` | — |
| TSMC | ATC 1038 `ATC_SET_TjMode`、1012 `ATC_TEST_START` | ATC 3.5 | 3.5 額外更新 TDC for TSMC；1012 帶 2DID | `Handler_Command_Report.md` | — |
| MTK | ATC 1112～1119 `ASIF_TJ_*`；GPIB `ASIF_TJ_EFUSED`／`ASIF_TJ_REQUEST`／`ASIF_TJ_FB`／`ASIF_TJ_EOT`；`MSG_CMD_ASIF_TJ_*`(174～176) | — | LVTS 校正資料與 Tj 回報（1115～1119 TBD） | `atc-commands.md`；`gpib-command-list.md` 第 12 章；`GPIB_Program_Manual_V12.04.md` §3.10 | — |
| Samsung（三星） | `MSG_CMD_SamSung_Tmp`(89)、`_Map`(90)、`_Soak`(91) | — | 三星格式溫度／Site Map／Soak（Steven 20191112）；Hana 的 `MAP?`／`SOAK?` 也回三星格式 | `GPIB_Program_Manual_V12.04.md` §3.10；`gpib-hana.md` | — |
| `CC_DoosanTesna`（843） | Handler `TfMain::TempDataStrings` | config `[Tester] bI38SETTEMPRespondSetTemp=2` | `SETTEMP?` 回整數 `25`（Steven 20250701） | `ht9045-gpib-bridge.md` §4 | — |
| `CC_DoosanTesna`（843） | 橋接 `DUTCHK?` → `MSG_CMD_DUTCHK`(181) | — | DUT Check（Steven 20250701） | `GPIB_Program_Manual_V12.04.md` §3.10；`ht9045-gpib-bridge.md` §5 | — |
| `CC_DoosanTesna`（843） | `FUNC_CC_DoosanTesna()` | `FUNC_CC_DoosanTesna`（`bKoreaFunction`、`bShowFunctionWindow`） | 不影響 ART（ART 在共用預設已開，客戶碼不是問題點） | `handler-art-flow.md` §3 | — |
| TESNA／TeraTech Korea（843） | `DoART_AfterCleanOut` 分支 #3 | `bAutoRetestGPIBmode` | FT lot end 時旗標掉成 false → 跳「請結批報表」、不送 `SetLotState(8)` → Tester hang | V3.21.902.0／GPIB V12.13.902.0 現場；`handler-art-flow.md` §5 | HT-9046LS |
| kevin 20180308 的特定客戶（舊 skill 沒寫名稱） | Handler `TfMain::TempDataStrings` | `bI38SETTEMPRespondSetTemp=1` | `SETTEMP?` 回 `Settemp +25.0`；期望純數值的 Tester 會 `Temperature check fail` | `ht9045-gpib-bridge.md` §4 | — |
| Ampere | 橋接 `GetFFC?`（只有 case 600 比得到）→ `MSG_CMD_GetFFC`(182) | — | FFC 取得（Steven 20250701） | `GPIB_Program_Manual_V12.04.md` §3.10；`ht9045-gpib-bridge.md` §3、§5 | — |
| Amlogic | `MSG_CMD_SBIN`(151) | — | SBIN 接收（Steven 20220120） | `GPIB_Program_Manual_V12.04.md` §3.10 | — |
| UTAC | `MSG_CMD_PPSELECT`(161)、`MSG_CMD_ASKPPSELECT`(162) | — | PP_SELECT 讀檔、詢問目前 Setup 檔名（Richard 20220929） | `GPIB_Program_Manual_V12.04.md` §3.10 | — |
| AMD | 橋接 `*IDN?` | `i2DIDFormat==eAMD` | 回 `1 Hontech,ROGERS,0,Rogers V12.13.905.0`（一般回 `HONTECH`） | GPIB V12.13.905.0；`gb-p8-bringup-plan.md` §3.2 | — |
| AMD | 橋接內建 RS232 `TfRS232Main`、`CommAMD`／`CommAMD2`（`SendMode`、`QueryRDY`、`btnSendTemp`） | `MSG_CMD_AMDRS232Connect`(92) | AMD／ATC 溫控與 HT-505 的 STX／LRC／ETX 輔助線，V906 留在 `GpibEngine` | `gpib-v906-integration-plan.md` §1.2；`GPIB_Program_Manual_V12.04.md` §3.10 | — |
| AMD | Handler 的 AMD 執行期條件（`HandlerBridgeCtl`） | — | V906 改回 906＝只有 Delta Castle（MR !134），912 要不要回來等 Steven | 906／912 待定；`ht9045-gpib-bridge.md` §8.7 | — |
| AMD | `MSG_CMD_EnableAMDFunction`(78)、`MSG_CMD_DisableAMDFunction`(79) | — | V3.30.649（GPIB V12.03）以後不用 | `GPIB_Program_Manual_V12.04.md` §3.11 | — |
| AMD | ATC 1038 `ATC_SET_TjMode`、`ATCCONTROLMODEMode` | 6.0／7.0 封鎖 | TC／TJ／TS 控制模式 | `atc-commands.md`；`ht9045-atc.md` | — |
| AMD-US | ATC 1120 `HANDLER_2DID` | — | 以 2DID 對 thermo profile（20240829 Eliot） | `atc-commands.md` | — |
| Delta Castle（`[GP-IB] Type`＝9） | 橋接 `SETPOINT?…ZONE`、`MASSTEMP?…ZONE` | `InterfaceType_Delta_Castle` | Delta Castle 專用溫度查詢；I38＝0 時 `SETTEMP?` 回 `25.0\n \r\n` | `ht9045-gpib-bridge.md` §3；`gb-p8-bringup-plan.md` §3.2 | — |
| RFMD | 橋接 `ProcessStatusString` 的 RFMD 分支、`QRM?`／`QRA?` | — | RFMD／Qorvo 指令子集 | `ht9045-gpib-bridge.md` §5；`gpib-v906-integration-plan.md` §1.2 | — |
| SPEA（`[GP-IB] Type`＝4） | 橋接標準派發 | — | 標準指令子集 | `ht9045-gpib-bridge.md` §5；`gb-p8-bringup-plan.md` §2.4 | — |
| Intel | 2DID 格式 | `i2DIDFormat==eIntel` | 2DID 輸出格式 | `ht9045-gpib-bridge.md` §5 | — |
| `CC_PTI`（957，力成） | `MSG_CMD_MachineState`(34)；MV `iStatus[17]` | — | 只 RS232 用的機台狀態（JerryYang 20151109） | `GPIB_Program_Manual_V12.04.md` §2、§3.11 | HT9050 機台的客戶碼（C19 §7） |
| `CC_MTI`／`CC_PTI` | `MSG_CMD_LotStatus`、`MSG_CMD_ChangeGpib` 的 `iLotStatus` | — | ChangeGpib 原本帶 0、接收端沒讀；GPIB_RS232 合併版改成帶 `iTestType`，LotStatus 不動 | `rs232-merge-details.md`「iLotStatus 欄位複用說明」；`design-decisions.md` DD-005 | — |
| OLP 聚成 | `RunTestProgram`（MV `iLotStatus`） | — | OLP 聚成專用 Lot 狀態 | `GPIB_Program_Manual_V12.04.md` §2、§3.1 | — |
| Maxim Philippine | `MSG_CMD_TesterBin`(37)、`MSG_CMD_SoakTime`(38)、`MSG_CMD_JamCode`(39)、`MSG_CMD_SiteMap`(40) | — | 只 RS232 用 | `GPIB_Program_Manual_V12.04.md` §3.11 | — |
| Qualcomm | GPIB 遠端 `MSG_CMD_RemoteStart`(204)／`MSG_CMD_RemoteStop`(205) | `W906_REMOTE_START_WIRED`（已隨 MR !130 消失） | 912 才有（JerryYang 20260828）；V906 拿掉分支，只留通訊紀錄 | 912 `main.cpp`／`MessageDef.h`（引述）；golden 906 與 `D:\GPIB9045` 都沒有；`ht9045-gpib-bridge.md` §8.7 | — |
| Qualcomm | ATC 1061 `ATC_60_SET_AIRVALVE` | `iATC_MODE_TYPE==ATC_TYPE_60` | ATC 6.0 氣閥開關 | `atc-commands.md`、`ATC_Command_Payload.md` | — |
| ASE_CL | ATC 1060 `ATC_51_SET_DEFROST` | — | ATC 5.1 除霜 | `atc-commands.md` | — |
| ASE_CL | RS232 `TfRS232Std::FormClose` 的 `sBarCode_ASE_CL` | — | 關閉時 delete 後設 NULL，防 double-free | `bug-log.md` BUG-002 | — |
| ASE_JP | TTL 板 `WINIT` 的 Bin 模式 `4` | `bTTLUseASEJPMode` | ASE_JP 模式 | `gb-p8-bringup-plan.md` §7.2；`gpib-v906-integration-plan.md` §1.6 | — |
| 勝麗 | ATC 1056 `ATC_51_FREONRECOVER` | — | ATC 5.1 冷媒回收 | `atc-commands.md` | — |
| ANST | ATC 1062 `ATC_READ_SOCKETTEMP` | — | 4 ch Socket 溫度（6.0 沒有這個 case） | `atc-commands.md`、`Handler_Command_Report.md` | — |
| HYGON | ATC 1109 `ATC_READ_HYGON_STATUS` | — | 讀 HYGON 狀態（2024/5/06 Cheng） | `atc-commands.md` | — |
| MTP（舊 skill 沒說是不是客戶碼） | ATC 1135 `ATC_SET_SINGLE_OFFSET_MTP` | — | 多 offset（20260226 victor）；Excel 先加，報告與原始碼沒列 | `atc-commands.md`、`ATC_Control_Command_Diff_Table.md` | — |
| AMKOR Korea（971，ATK） | `TfLotInfo` case 7 `SetMultiZoneTemp`、`EnablesChannel` | `cbMultiZoneFunction`、`Temperature.bMultiZoneEnable` | SingleSite 只啟用 1 個 ATC channel、其他 Zone 停用；r896 起修 | V3.21.895.2（r895）vs r896；`MultiZone_EnableChannel_Bug.md`（P260421-ATK-H9-01） | HT-9132／HT-9046LS |
| ATK／TeraTech Korea | ATC 1127／1128 守衛 | `iATC_MODE_TYPE` 白名單 35／36 | 黑名單漏 Rogers／3.3 → ACK 不回、佔頻寬 | `ht9045-atc.md`（2026-04-23） | HT-9046LS Hybrid |
| Greatek（956）、TeraPower（967）、TeraProbe（804） | `TfMain::HanderTcpIp`、`TCPCommandServerClientRead` | `CosFunction.bEnableHandlerResultServer` | 開 7016／7017 TCP 指令伺服器；`HTSET,333／334` 只在 TeraPower 或 `bRemoteLotStart` | golden 906_0625_Steven、V906 `CosFunction.cpp`；`tcp-command-server-7016.md` §1、§6 | — |
| 868（開發機） | `TfMain::HanderTcpIp` | `bEnableHandlerResultServer=false` | 不 listen；START 時 else 支把兩台 `Active=false`，物件必須先存在 | `tcp-command-server-7016.md` §2 | 開發機 |
| Murata | `GetTesterResult` 的 NonTestToRBin 紀錄 | — | V906 拿掉 912 的紀錄（MR !136），要不要回來等 Steven | 906／912 待定；`ht9045-gpib-bridge.md` §8.7 | — |
| FOREHOPE_NINGBO | `ChangeTesterConnect` 的「Silent run mode change」`RecordProcess` | — | 912 這兩行照 Steven 1003 常設規則保留（#20 exception，MR !136） | 912 保留；`ht9045-gpib-bridge.md` §8.7（CASE-FOREHOPE_NINGBO-20260920-001） | — |
| `CC_JSCC_OS` | `TfTesterTCP::CopyOSTestResult`、`PlaceOSTestResultToTray`、`ProcessOSPrint`、`ProcessOSTrayData`、`CopyRecipeTo/FromTester` | — | TCP/IP Tester 的 OS 報表族（V906 部分翻譯、8 個 gate） | `gpib-v906-integration-plan.md` §1.4 | — |
| SPIL（AMR） | Tray Feed 客製訊息 | — | YES／NO「請先在 TESTER 結報表」對話框，與 ART lot-end 三岔無關 | `ht9045-art-flow.md` | — |
| `CC_SCC` | `TfTemp_Set::MaxTempSetting` | `CUSTOMER_CODE==CC_SCC` | `SETTEMP` 上限 150 ℃（非 LS 機型也是） | C19 §1 第 50 列 | 不分 |
| 神盾（雙臂） | RS232Standard `OnMyCopyMsg` 的 `SwitchArm` | — | 雙臂交換 Index Arm（`MSG_CMD_SwitchArm`／`SwitchArmOK` 只 RS232 用） | `gpib-v906-integration-plan.md` §1.3；`GPIB_Program_Manual_V12.04.md` §3.11 | — |
| AVAGO、SPRD | RS232Standard 畫面分頁（SPRD：4 site 各自 COM） | — | RS232 客製畫面 | `gpib-v906-integration-plan.md` §1.3 | — |
