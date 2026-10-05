---
name: hpi-gpib
description: >
  HT-9xxx Handler 與 Tester／ATC 的通訊主題 skill（GPIB 為主）。併入 11 支舊 skill：gpib-command-list、
  gpib-program-manual、gpib-93k-art、gpib-hana、gpib-qrovo、gpib-rs232-merge、gpib-ht9045-sync、
  ht9045-gpib-bridge、ht9045-art-flow、ht9045-atc、ht9045-atc-interface（舊內容整份搬進 references，未刪）。
  涵蓋 GPIB 指令與 BIN／SRQ／2DID 格式、MSG_CMD 與 VM／MV 結構、H9046_32GPIB 橋接與 V906 TesterComm 併入
  （通訊執行緒、SyncMailbox、P8 上機）、Handler TCP 指令伺服器 7016／7017、SETTEMP? 回覆格式（Settemp +25.0）、
  ART（93K／Flex／Hana、SRQKIND、SetLotState、請結批報表、bDummyART）、Qorvo、GPIB／RS232／TTL 三介面、
  CC_／MSG_CMD／eTestMode 三邊同步、ATC TCP 協定（1001～1137、iATC_MODE_TYPE）；另有 HT9045／HT9046LS／
  HT9050（9050GPIB 解碼成 Type_HT9050）機型檔與客戶分流表。
  關鍵字：GPIB, H9046_32GPIB, GPIB9045, TesterComm, MSG_CMD, WM_COPYDATA, SRQ, FULLSITES, BINON, ECHOOK,
  SETTEMP, SETTEMP?, bI38SETTEMPRespondSetTemp, ART, Auto Retest, SCK_ART, SRQKIND, SetLotState,
  DoART_AfterCleanOut, bDummyART, 93K, Hana, Qorvo, SIGURD, g_iTestType, eTestMode, CC_, ATC,
  iATC_MODE_TYPE, @1003, Dynamic PID, HulkMode, MTK ASIF, 7016, HTSET, 9050GPIB, Type_HT9050,
  Type_HT9046_LS, MachineTypeChoice。
---

# hpi-gpib：Handler ⇄ Tester／ATC 通訊（GPIB 主題）

## 這是什麼

Handler 跟外部測試設備講話的那一層：Tester 經 GPIB 送字串指令給 `H9046_32GPIB.exe`（BCB 線）或 V906 行程內的 `TesterComm` GPIB 引擎（C++ 線），轉成 `MSG_CMD_*` 交給 Handler，Handler 的回覆原樣寫回 Tester；ART（自動重測）、Qorvo、Hana 等是建在這條線上的協定；ATC 溫控設備則是 Handler 直接走 TCP 的另一條線。2026-10-05 起 11 支舊 skill 併成這一支（ST02-C22），舊檔整份放在 `references/<舊名>/<舊名>.md`，只改了指向舊位置的連結。先讀 [references/common.md](references/common.md)（不分機型的行為），遇到「依機型分流」再看機型檔，遇到客戶專屬功能看 [references/customers.md](references/customers.md)。

## 路由表

| 問題或機型 | 先讀 | 深入（舊 skill 原文） |
|---|---|---|
| 不分機型的架構、IPC、指令派發、ART、ATC、7016、V906 TesterComm、上機通則 | [common.md](references/common.md) | 各節標了出處 |
| GPIB 指令語法、BINON／ECHO 排列、BIN Mapping、2DID 指令、各章指令表 | common §4 | [gpib-command-list.md](references/gpib-command-list/gpib-command-list.md)、[standard-gpib-command.md](references/gpib-command-list/references/standard-gpib-command.md) |
| `MSG_CMD_*` 代碼、VM／MV 結構、函式一～九、`eTestMode` | common §2 | [gpib-program-manual.md](references/gpib-program-manual/gpib-program-manual.md)、[GPIB_Program_Manual_V12.04.md](references/gpib-program-manual/references/GPIB_Program_Manual_V12.04.md) |
| H9046_32GPIB 橋接、`SETTEMP?` 回 `Settemp +25.0`、各 Tester 廠牌差異 | common §3、§5 | [ht9045-gpib-bridge.md](references/ht9045-gpib-bridge/ht9045-gpib-bridge.md) §1～§7 |
| V906 TesterComm（一條通訊執行緒、SyncMailbox、裁決 1～11、as-built、ctest） | common §11 | [ht9045-gpib-bridge.md](references/ht9045-gpib-bridge/ht9045-gpib-bridge.md) §8、[gpib-v906-integration-plan.md](references/ht9045-gpib-bridge/references/gpib-v906-integration-plan.md) |
| 第一次接真 NI 卡／真 COM／真 TTL 板（P8） | common §12 | [gb-p8-bringup-plan.md](references/ht9045-gpib-bridge/references/gb-p8-bringup-plan.md) |
| Handler 自己的 TCP 指令伺服器 7016／7017（HTGR／HTSET） | common §9 | [tcp-command-server-7016.md](references/ht9045-gpib-bridge/references/tcp-command-server-7016.md) |
| 93K ART 的 GPIB 端（FR?、LOTCLEAR?、INPUTQTY、SRQKIND?、LORORDER） | common §7 | [gpib-93k-art.md](references/gpib-93k-art/gpib-93k-art.md)、[93k-art-protocol.md](references/gpib-93k-art/references/93k-art-protocol.md) |
| Handler 端 ART、「請結批報表」、FT lot end 收不到 SRQ 0xC0、bDummyART | common §7 | [ht9045-art-flow.md](references/ht9045-art-flow/ht9045-art-flow.md)、[handler-art-flow.md](references/ht9045-art-flow/references/handler-art-flow.md)（另有兩份 pptx） |
| Hana Micron ART | [customers.md](references/customers.md) | [gpib-hana.md](references/gpib-hana/gpib-hana.md)、[hana-art-protocol.md](references/gpib-hana/references/hana-art-protocol.md) |
| Qorvo（CONFIGURE、ESC、QRM?／QRC?） | [customers.md](references/customers.md) | [gpib-qrovo.md](references/gpib-qrovo/gpib-qrovo.md)、[qorvo-protocol.md](references/gpib-qrovo/references/qorvo-protocol.md)、[通訊記錄](references/gpib-qrovo/message_2020_07_30_10.TXT) |
| GPIB／RS232／TTL 三介面（`g_iTestType`、`iLotStatus` 複用、BUG-001～005） | common §6 | [gpib-rs232-merge.md](references/gpib-rs232-merge/gpib-rs232-merge.md)、[rs232-merge-details.md](references/gpib-rs232-merge/references/rs232-merge-details.md)、[bug-log.md](references/gpib-rs232-merge/references/bug-log.md)、[design-decisions.md](references/gpib-rs232-merge/references/design-decisions.md) |
| `CC_`／`MSG_CMD_`／`eTestMode` 在 HT9045、GPIB9045、RS232Standard 三邊同步 | common §10 | [gpib-ht9045-sync.md](references/gpib-ht9045-sync/gpib-ht9045-sync.md)、[sync-procedures.md](references/gpib-ht9045-sync/references/sync-procedures.md) |
| ATC 程式行為、`iATC_MODE_TYPE`、6.0／7.0 封鎖、已知 ACK 缺陷、新增命令 | common §8 | [ht9045-atc.md](references/ht9045-atc/ht9045-atc.md)、[Handler_Command_Report.md](references/ht9045-atc/references/Handler_Command_Report.md)、[ATC_Command_Payload.md](references/ht9045-atc/references/ATC_Command_Payload.md)、[MultiZone_EnableChannel_Bug.md](references/ht9045-atc/references/MultiZone_EnableChannel_Bug.md)、[ATC_Control_Command_Diff_Table.md](references/ht9045-atc/references/ATC_Control_Command_Diff_Table.md) |
| ATC 封包範例、137 個命令、Site／Channel 對應、通訊 log 除錯 | common §8 | [ht9045-atc-interface.md](references/ht9045-atc-interface/ht9045-atc-interface.md)、[atc-commands.md](references/ht9045-atc-interface/references/atc-commands.md)、[from-colleague-20260915.md](references/ht9045-atc-interface/references/from-colleague-20260915.md) |
| 客戶專屬（Hana、Qorvo、SIGURD、Novatek、TSMC、MTK、AMD、TESNA…） | [customers.md](references/customers.md) | 表內「906／912 來源」欄 |
| **HT9045**（9045GPIB） | [ht9045.md](references/ht9045.md) | — |
| **HT9046LS**（9046_32GPIB） | [ht9046-ls.md](references/ht9046-ls.md) | — |
| **HT9050**（9050GPIB，main 解成 LS、機台 0210 解成 Type_HT9050） | [ht9050.md](references/ht9050.md) | `ht9050-hw`、`ht9050-construction` |
| 其他機型（2601／9055／502／7080／1032） | 同 common（只有白名單與補位，見 common §1） | — |
| RS232Standard／TTL 板協定本身 | `hpi-rs232`（`.claude/skills/hpi-rs232/SKILL.md`） | — |
| SECS 對應（例 ECID 35548「[I38] Format of SETTEMP?」） | `hpi-secs`（原 ht9045-secsgem） | — |
| 溫控在 V906 的現況、`ATC.ini` 位置 | `ht9045-temperature` | — |

## 絕不能漏的安全事項

1. **7016／7017 打開後，區網任何主機都能遠端啟動／暫停機台**：「啟動／暫停機台（333／334…）、用任意 LotID 開批（720）、拿 Supervisor（310，不用密碼）…防火牆只開給 MES／AMR 主機。」〔`tcp-command-server-7016.md` §6〕
2. **GPIB 遠端 START／STOP 不在 906**：`MSG_CMD_RemoteStart`(204)／`MSG_CMD_RemoteStop`(205) 是 912 才有的 Qualcomm 功能，V906 已拿掉分支，「沒寫到的指令＝不支援」。〔`ht9045-gpib-bridge.md` §8.7〕
3. **寫入類 GPIB 指令會動機台或改配方**：「第一步不要送會寫東西的指令：`SETTEMP +`／`SETTEMP_`（改機台溫度）…`DEVICETEMP`／`SETTESTOFFSET_`（W9，寫配方 Temperature.Data）」，而且 offset「會累加」。〔`gb-p8-bringup-plan.md` §3.2、§8 第 3 點〕
4. **ART lot end 漏送 SRQ 0xC0 = Tester 一直等**：「若 Handler 不呼叫 `SetLotState(8)`，tester 會一直等 FT-lot-end 的 SRQ 0xC0 → hang」；「只要 `bDummyART==true`，GPIB 就只寫 log、不送 SRQ:0xC0」；「`bDummyART`／`bSCKART_RunARTWithoutCmd` 必須在 lot start 之前就確定為生產值」。〔`ht9045-art-flow.md`〕
5. **Qorvo**：「Handler 必須等到收到 `ECHOOK` 後才能排料」；第二次 `ECHONG` 全部送 Flush Bin；`PAUSE` 之後下一道必須是 `RESUME`。〔`gpib-qrovo.md`〕
6. **Hana SRQ**：「0x59 (HD_MODE_SRQ) 需先 `ibstop()` 再 `ibrsv()`」，否則 Tester 收不到。〔`gpib-hana.md`〕
7. **VM／MV 結構只能在末端加成員**，「Handler／GPIB／RS232 三端須同步修改並同步版號」。〔`gpib-program-manual.md`〕；每個 `WM_COPYDATA` 送出點都必須設 `iLotStatus`（POD 預設 0＝TTL_MODE）。〔`gpib-rs232-merge.md`〕
8. **真卡、真 COM、真 TTL 線會驅動真設備**：「不要在客戶量產的 Tester 上做，除非客戶在場並同意」；testercomm 網頁指令沒有權限閘（B7）；V906 與 BCB 橋接不能同時跑；先備份 `Gerneral.ini`、`D:\GPIB9045\system\general.ini`、`Setup.ini`、配方。〔`gb-p8-bringup-plan.md` §2.5、§8〕
9. **HT9050 的 GPIB Model 必須是 `9050GPIB`**：開機時不是的話，機型解成 Type_HT9045，HT9050 的掛勾（ORG 規則、HOME 分支…）全部失效。〔[ht9050.md](references/ht9050.md) §2〕
10. **`SETTEMP` 上限依機型與客戶不同**：LS 一律 150 ℃；非 LS 看 `iTempLimitation` 與 `CC_SCC`（HT9050 機台的 ini 算出 135 ℃），HT9050 換成 `Type_HT9050` 解碼後 135～150 ℃ 會回 `VALUE NG`。〔[ht9050.md](references/ht9050.md) §3〕
11. **ATC 版本專屬命令一律用白名單**；ATC 6.0／7.0 不得送 1038／1067、1071、1084、1085、1087、1088、1125（呼叫端另封 1104；1105 限 Rogers TYPE_33）。〔`ht9045-atc.md`〕
12. **三邊同步永遠以 HT9045 為基準、不反向、不刪除**；Big5 檔一律 bytes 讀寫。〔`gpib-ht9045-sync.md`〕
13. **`HT9045_TESTERCOMM=0` 不可用在要跑有 IC 測試循環的 SIM 回歸**（沒有 bridge 時 GetTesterResult 每 3 秒重試、不前進）；會寫 `D:\GPIBLOG`／`D:\RS232Log` 的 ctest 要手動開。〔`ht9045-gpib-bridge.md` §8.7〕

## 完整關鍵字

（11 支舊 skill 的描述與內文「關鍵字」行的聯集，去重；每個詞只列在第一次出現的舊 skill 底下。）

- **gpib-command-list**：GPIB Command、BINON、BINOFF、SRQ、BIN Mapping、FULLSITES、ECHO、BARCODE?、GET2DID?、SETSITEMAP、BINMAP、SETTEMP、PAUSE、STOP_01、CHKSTATUS、SETSTARTMODE、ART、SETTESTTEMP、OVERDRIVE、RECONTACT、GPIB 指令格式、BINON/BINOFF 排列、SRQ 流程、溫度控制指令、ART/ATC 功能、2DID/Barcode 指令、Device Map、Remote Control 指令、Qorvo/SIGURD 客製 GPIB 協定、ECHOOK、ECHONG、QRC?、GETBARCODENUMBER、SETUP_、SETSOAK、ASIF_TJ_EFUSED、ASIF_TJ_REQUEST、TEST ARM?、TEMPARM?、GETNOWALLTEMP、FORCE?、TESTARMEP?、TESTARMPOS?、ONECYCLE、AUTO_CLEAN、PICKLOAD、PLACETOLOAD、TRAYFEED、NEXTSTEP、READDIODE、READTEMP、READDAQ、SENDERROR、DEVICETEMP_、SETTESTOFFSET_、SETCOOLINGVALUE_、CHECKLIST、SGFTP_、NONDOUBLEBIN_、BINCOUNT_、SGOSBIN_、SGCONTFAIL、SETTESTERID_、BINPOS_、SETAICCD_、SVID、ECID、RCMD、*IDN?、CHKMATCH?、HANDLERID?、Version?、SETUPFILENAME_、BIN 0~254、32BIN、GS16/GS32 BIN、Advan T6577、Qorvo Protocol、SIGURD Protocol、32-site、HT9045、HT9046
- **gpib-program-manual**：GPIB Program Manual、H9046_32GPIB、GPIB 架構、GPIB 初始化、RS232 通訊、32-site GPIB、Logic Handler、SRQ flow、GPIB 軟體架構、H9046_32GPIB.exe 功能、GPIB 初始化流程、RS232 通訊程式架構、GPIB 與 Handler 通訊介面設計、GPIB 程式設計、GPIB interface、test handler GPIB program
- **gpib-93k-art**：93K、HP93K、Advantest、SCK_ART、iCurrent93KARTStep、FR?、LOTCLEAR?、INPUTQTY、SRQKIND?、LORORDER、DummyART、CheckNeedRT、bCanRunSCKART、93K ART 指令、iCurrent93KARTStep 狀態機、FT/RT 流程切換、SRQMASK 設定、CheckNeedRT 判斷、DummyART 模擬器、SCK_ART 模組、Auto Retest、SETTINGOK、SETTINGNG、LOTCLEARED、SRQKIND、SRQKIND 2、SRQKIND 4、SRQKIND 8、SRQKIND 10、LORORDER 2、LOTORDER、LOTRETESTCLEAR?、iTesterType、MSG_CMD_SCKART_LOTCLEAR、MSG_CMD_SCKART_INPUTQTY、MSG_CMD_LotStatus、MSG_CMD_SCKART_SRQMASK、iLotModeGPIB、SRQMASK、FT lot start、RT lot start、Final lot end、iFTRTCount、iNeedRT、bEndLotAutoRetestGPIB、bWaitStartLotAutoRetestGPIB
- **gpib-hana**：Hana、HanaMicron、HANA_ART、DEVON、LOTON、LOTRT、TDATA、CDATA、SDATA、JDATA、ADATA、DATACLEAR、DUMMYTEST_START、bRunHANA_ART、SMILL、FT、RT、Prime、Retest、Hana GPIB 指令、SRQ 代碼（0x55~0x79）、DEVON/LOTON 設定、TDATA/CDATA/SDATA/JDATA/ADATA 資料格式、FT/RT 切換、SMILL 模式、SRQ0x55、SRQ0x56、SRQ0x57、LOT_START、LOT_END、PRIME_START、PRIME_END、RETEST_START、RETEST_END、STANDBY_TESTMODE、bTryHANA_ART、MSG_CMD_HANA_ART、MSG_CMD_RUN_HANA_ART、CC_HANA_MICRON、InitHANA_ART、HANA_ART_SMILL、GENERAL、LOADER、HDMODE、STEPOK、PMODEOK、PRIMETESTSTARTOK、PRIMETESTEND、RMODEOK、RETESTSTARTOK、RETESTEND、LOTEND:COMP、CLEAROK、ID?、MAP?、CT?、TEMPSET?、SOAK?
- **gpib-qrovo**：Qorvo、CONFIGURE、FULLSITES?、QRM?、RESUME、CHECKEMPTY、SPE-001712、SRQ41、SRQ42、SRQ44、SOT、ESC、Empty Socket Check、CONFIGURE 流程、FULLSITES? 回應格式、ECHOOK/ECHONG 重試機制、QRM?/QRC? 2D Barcode、PAUSE/RESUME、REQUEST,CHECKEMPTY (ESC) Empty Socket Check、GPIB、CONFIGURE,SRQ、CONFIGURE,FULLSITES?、CONFIGURE,CONTACTOR、REQUEST,CHECKEMPTY、Pick & Place、IEEE488.2
- **gpib-rs232-merge**：g_iTestType、TTL_MODE、GPIB_MODE、RS232_MODE、MSG_CMD_ChangeGpib、iTestType、OpenTesterComm、CloseTesterComm、iLotStatus、SendMessageToGpibProg、ProcessHVisionConnect、eRs232Mode、g_iTestType 架構、COM port 開關、startup ini 還原、iTesterMode 職責分離、DIO TypeName log、Qorvo guard、MSG_CMD_TesterMode、LoadSetupData、g_sRecipePath、WriteLastDataFile、ReadLastDataFile、bGpibMode、SendMSG_TestMode、sLastHVisionWnd、FormDestroy、FormClose、slRS232Log、AppException、MY_DUT_PAL、double-free、dangling pointer、crash on close、DIO TypeName、eRs232Standard、eRs23232Bin、SerialPoll WriteLog、ShowCommData
- **gpib-ht9045-sync**：sync、同步、Customer Code、CC_、MSG_CMD、eTestMode、cmydef、MessageDef、GPIB 同步、HT9045 同步、RS232 同步、RS232Standard、跨專案同步
- **ht9045-gpib-bridge**：GPIB9045、SETTEMP?、MyGPIBWrite、SendMSG_CMD、SetParameter、TSerialPoll、ibwrt、ibrd、Settemp +25.0、Temperature check fail、iI38SETTEMPRespondSetTemp、Tester、ATE、pgm、MES Temp、GPIB 併入、GPIB in-process、GpibEngine、GpibThread、GpibIpc、SyncMailbox、IGpibDriver、gpib-32.dll、SimGpibDriver、gpib.html、gpib.* tag、TMyDutPanel、makeDutPanel、OnMyCopyMsg 翻譯、GB 戰役、golden TCPCommandServer／TeraTCPResultServer、HanderTcpIp、TCPCommandServerClientRead、HTGR／HTSET／HTSR 指令、V906 W10 的 CmdServerPump／TcpCmdFramer／polled TServerSocket、GPIB 併入 V906 的轉換計畫、獨立執行緒 + 瀏覽器畫面、Handler 自己的 TCP 指令伺服器 7016／7017、GPIB 指令、SETTEMP/SETTEMP?/SETSOAK/SETSITEMAP、ibwrt/ibrd、Tester 溫度比對失敗、Handler Temp=Settemp、溫度回應格式、各 Tester 廠牌（Advantest/Flex/93K/SPEA/RFMD/Qorvo/Delta Castle/DOOSAN/Novatek）指令差異、V906 轉換計畫全文、P8 機邊 bring-up 步驟書、上機前缺口 B1～B7
- **ht9045-art-flow**：A10、Enable ART、fSCKART、DoART_AfterCleanOut、SetLotState、bAutoRetestGPIBmode、bUseSCKART、bFirstTestAutoRetestGPIB、SRQ 0xC0、FT Lot End、RT Lot Start、Final Lot End、ContinuStart_ART、rsmContinuRetest_ART、Move Tray to Loader、Please Print Summary、請結批報表、TESNA、TeraTech、bART_SECSGEM_93K、iAutoRetestTCPmode、bSCKART_RunARTWithoutCmd、bDummyART、DoInitialStart、MSG_CMD_SCKART_RunDummy、Dummy ART Running Status、[A10-1] Enable ART 設定、SCK_ART 模組（fSCKART）、DoART_AfterCleanOut() FT/RT lot-end 三岔分支、SetLotState() 與 GPIB 通訊、bAutoRetestGPIBmode / bUseSCKART / iTesterType 旗標關係、FT Lot Start / FT Lot End / RT / Final Lot End 與 GPIB SRQKIND 對應、「請結批報表 / Please Print Summary」對話框來源、ContinuStart_ART 執行模式
- **ht9045-atc**：ATC、ATCInterface、ATC_Handler_Side、iATC_MODE_TYPE、SendCommand、ProcessHandlerCommand、ProcessATC_BufferCommand、ATC_RECIPE_FILE、ATC_RUN_STOP、ATC_READ_TEMP、ATC_TEST_START、ATC_LOT_START、ATC_SELFTEST、FFC、PFC、TJ Offset、Dynamic PID、HulkMode、ASIF、2DID、Chiller、水閥、溫控、ATC指令、ATC通訊、ATC 溫控流程、ATC 通訊指令（code 1001~1137）、ATC_Handler_Side.cpp 指令實作、Handler↔ATC TCP 封包格式、ATC 版本型號差異（ATC_3.5 / ATC_Rogers / ATC_3.1 / 5.0 / 6.0 / 7.0）、iATC_MODE_TYPE 判斷邏輯、新增或修改 ATC 指令 case、ProcessHandlerCommand / SendCommandToHandler 空實作 bug、ATC Self-Test 流程、Lot Start/End 通知、FFC/PFC 功能、TJ Slope/Offset 校正、Dynamic PID（TSMC）、多區 Tc Offset、Recipe 傳輸、MTK ASIF 指令、2DID 設定、水閥/水流/水警告、Chiller 控制
- **ht9045-atc-interface**：ATC Interface、ATC_SET_TEMP、ATC_SET_TOFS、ATC_SITE_ENABLED、GetData、@1001、@1002、@1003、@1004、@1016、@1105、ATC_Multi_Temperature_Control、ATC_SET_T2OFS、HANDLER_2DID、ATC Recipe、Self-Test、MTK ASIF、TSMC Hulk、137 個 ATC 命令（1001-1137）、封包格式、Handler Side 類別、溫度/Offset 傳送流程、Site Mapping、Recipe、TJ/FFC/PFC、分析 ATC 通訊 log、追 @1002/@1003/@1004 等封包內容、除錯 ATC 不回應、新增/修改 ATC 命令、解析 ATC hangup 資料夾 TXT 檔
