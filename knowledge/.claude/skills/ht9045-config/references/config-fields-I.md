# HT9045_CONFIG 欄位速查：群組 [I] Tester（測試介面 / 良率）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Ixx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 92 個欄位，分屬 73 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| I01 | Enable tester finish then homing | `cbI01` | `bI01TesterFinishThenHome` | bool | `bI01TesterFinishThenHome` | 35500 | ECBool | Enable tester finish then homing |  |
| I02 | After home,set socket IC to error bin | `cbI02` | `bI02HomeSetSocketICToErrBin` | bool | `bI02_bHomeSetSocketICToErrBin` | 35516 | ECBool | After home,set socket IC to error bin | JerryYang 20151026 歸零時把當下在測試的IC當ErrorBin |
| I03 | Enable temperature control function | `cbI03` | `bI03AmbientTempControl` | bool | `bI03AmbientTempControl` | 35517 | ECBool | Enable temperature control function | kevin 20140918 常溫恆溫控制 |
| I04 | Enable change bin during pause | `cbI04` | `bI04EnableChangeBinDuringTesting` | bool | `bI04EnableChangeBinDuringTesting` | 35501 | ECBool | Enable change bin during pause |  |
| I05 | Low yield alarm forced one cycle | `cbI05` | `bI05LowYieldForcedOneCycle` | bool | `bI05LowYieldForcedOneCycle` |  | ECBool | Low yield alarm forced one cycle | JerryYang 20190709 黃剛要求low yield不要強制one cycle |
| I06 | Turn on the [I01] function after Home is complete | `cbI06` | `bI06TurnOnI01AfterHome` | bool | `bI06TurnOnI01AfterHome` |  | ECBool | Turn on the  function after Home is complete | Sam 20220216 : Home 完成後強制開啟 I01 Function |
| I06 | Turn on the [I01] function after Home is complete | — | `bI06_Active` | bool | — |  | — | Turn on the  function after Home is complete | Sam 20220527 : for 矽格北興 -- I06打勾 |
| I06 | Turn on the [I01] function after Home is complete | — | `bI06_Enable` | bool | — |  | — | Turn on the  function after Home is complete | Sam 20220527 : for 矽格北興 -- I06 Enable |
| I07 | Reset GPIB after one cycle or clean out | `cbI07` | `bI07ResetGPIBAfterOneCycleCleanOut` | bool | `bI07ResetGPIBAfterOneCycleCleanOut` | 35502 | ECBool | Reset GPIB after one cycle or clean out |  |
| I08 | Check 2DID function when initial start. | `cbI08` | `bI08Check2DIDEnableWhenInitialStart` | bool | `bI08Check2DIDEnableWhenInitialStart` | 35518 | ECBool | Check 2DID function when initial start. | Steven 20190412 : Initial Start時檢查有沒有開啟2DID |
| I09 | Yield alarm no need  clean shuttle. | `cbI09` | `bI09LowYieldOneCycleDontCleanShuttle` | bool | `bI09LowYieldOneCycleDontCleanShuttle` |  | ECBool | Yield alarm no need  clean shuttle. | JerryYang 20220923 : yield alarm時觸發half one cycle(shuttle保留IC不測試跳ONE CYCLE FINISH) |
| I12 | Tester time out ,don'#39't send START signal again | `cbI12` | `bI12TesterTimerOutNotNeedReTest` | bool | `bI12TesterTimerOutNotNeedReTest` | 35503 | ECBool | Tester time out ,don't send START signal again |  |
| I13 |  | `cbI13` | `bI13InitStartDelayHasFTandRT` | bool | `bI13InitStartDelayHasFTandRT` | 35519 | ECBool | Initial start delay fuinction setting different when FT and RT. | Steven 20190313 : Initial Start Delay use different setting in FT and RT |
| I16 | TTL setting save to setup file. | `cbI16` | `bI16TTLSaveInSetupFile` | bool | `bI16TTLSaveInSetupFile` | 35520 | ECBool | TTL setting save to setup file. | Steven 20180626 (wei) : TTL設定存到工作檔裡面 |
| I18 | Can receive ECHOSTOP | `cbI18` | `bI18CanReceiveEchoStop` | bool | `bCanReceiveEchoStop` | 35505 35521 | ECBool | Can receive ECHOSTOP | ChungHung 20120326 add for ASE_KR |
| I19 | Auto site map pause change SIMULATE test bin data | `cbI19` | `bI19AuToSitMapPauseWaitBin` | bool | `bAuToSitMapPauseWaitBin` | 35522 | ECBool | Auto site map pause change SIMULATE test bin data | kevin 20150122 AutoSiteMAP 模式 用gpib模擬器 暫停等待改bin別 |
| I20 | The alphabet of the error bin. | — | `iI20ErrorBinAlphabet` | int | — | 35506 | — | The alphabet of the error bin. |  |
| I21 | Auto Site Mapping | `edI21` | `fI21UseSameSoakTime` | double | `Use Same Soak Time Sec` | -- | -- | Auto Site Mapping |  |
| I21-1 | Enable auto site mapping function | `cbI21` | `bI21EnableASM` | bool | `Enable Auto Site Mapping` | 35507 | ECBool | Enable auto site mapping function | 是否啟用Auto Site Mapping |
| I21-1 | Enable auto site mapping function | `edI21_1` | `iI21FailRetryCount` | int | `iI21FailRetryCount` | 35507 | ECBool | Enable auto site mapping function | KenHsieh 20251002 : add auto site mapping fail retrey Func. |
| I21-2 | Skip soak time | `cbI21_SkipSoakTime` | `bI21SkipSoakTime` | bool | `Skip Soak Time` | 35508 | ECBool | Skip soak time | [I21] Auto Site Mapping Soak Time------------------- |
| I21-3 | Use same soak time | `cbI21_SameSoakTime` | `bI21UseSameSoakTime` | bool | `Use Same Soak Time` | 35509 | ECBool | Use same soak time |  |
| I21-4 | Check every dut should be open. | `cbI21_CheckOpen` | `bI21ASMNeedCheckEachSiteOpen` | bool | `bASMNeedCheckEachSiteOpen` | 35511 | ECBool | Check every dut should be open. | Steven 20120726 : AutoSiteMapping, 當確認Open Bin時,同時也要檢查是不是所有Dut都Open |
| I21-5 | Remove loader tray manually. | `cbI21_5` | `bI21ASMRemoveLTrayManually` | bool | `bASMRemoveLTrayManually` | 35512 | ECBool | Remove loader tray manually. | Steven 20120830 : AutoSiteMapping, 手動移除Loader Tray |
| I21-6 | Run time check | `cbI21_6` | `bI21ASMRunTimeCHeck` | bool | `bASMRunTimeCHeck` | 35523 | ECBool | Run time check | Steven 20140729 : AutoSiteMapping, 邊生產邊做 |
| I21-6 | Run time check | `edI21_6` | `iI21AutoSiteMappingErrCT` | int | `iI21AutoSiteMappingErrCT` | 35523 | ECBool | Run time check | jou 20200707 : VTEST auto site mapping |
| I21-7 | Bin IC combine place to Fix 2 | `cbI21_7` | `bASMAutoSiteMapBinComBine` | bool | `bASMAutoSiteMapBinComBine` | 35524 | ECBool | Bin IC combine place to Fix 2 | kevin 20150115 Auto Site map 所有bin 別 放在fix 2 |
| I21-8 | Auto Site Mapping Use Hotplate | `cbI21_8` | `bI21AutoSiteMappingUseHotplate` | bool | `bI21AutoSiteMappingUseHotplate` | 35525 | ECBool | Auto Site Mapping Use Hotplate | Ifor 20170919 (Steven) : add Auto Site Mapping Hotplate Mode |
| I21-9 | Enable Site Mapping Fail Bin setting | `cbI21_9` | `bI21AutoSiteMappingFailBinSetting` | bool | `bI21AutoSiteMappingFailBinSetting` | 35526 | ECBool | Enable Site Mapping Fail Bin setting | Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting |
| I21-9 | Enable Site Mapping Fail Bin setting | `edI21_9` | `iI21UseFailBinSetting` | int | `iI21UseFailBinSetting` | 35526 | ECBool | Enable Site Mapping Fail Bin setting | Ifor 20171128  (Steven) : add Auto Site Mapping Fail Bin Setting |
| I21-10 | RT Mode Don'#39't Run Site Mapping | `cbI21_10` | `bI21RTmodeDonotRunSiteMapping` | bool | `bI21RTmodeDonotRunSiteMapping` |  | ECBool | RT Mode Don't Run Site Mapping | Richard 20230427 : RT mode不跑sitemapping |
| I22 | Home delay time               sec. | `cbI22` | `bI22TimeOutCanSkip` | bool | `TestTimeOutCanSkip` | 35513 | ECBool | Enable test time out option: | Steven 20111220 : 測試TimeOut可以Skip |
| I22 | Home delay time               sec. | `rgI22` | `iI22TestTimeOutOption` | int | `iI22TestTimeOutOption` | 35513 | ECBool | Enable test time out option: | Steven 20181226 : 測試Time out可以按的按鈕 |
| I22-1 |  | `edI22_1` | `fI22HomeDelay` | double | `fI22HomeDelay` |  |  |  |  |
| I23 | Hot test waiting mode | `cbI23` | `bI23HotTestWaitingMode` | bool | `HotTestWaitingMode` | 35514 | ECBool | Hot test waiting mode | ChungHung 20111230 Hot Test Waiting Mode |
| I24 | Stop all motor while testing | `cbI24` | `bI24TestingNeedStopAllMotor` | bool | `bTestingNeedStopAllMotor` | 35515 | ECBool | Stop all motor while testing | jou 2013-09-25 Testing Need Stop All Motor |
| I25 | Format for get handler testing arm temperature. | — | `iI25UseGPIBFormat` | int | — | 35528 | — | Format for get handler testing arm temperature. | kevin 20130705 台積電通訊規格  0:HT    1:NS   //Steven 20160301 : 改為海思格式 |
| I26 | Close site have bin data need manual remove device. | `cbI26` | `bI26TestCloseSiteHaveBin` | bool | `bTestCloseSiteHaveBin` | 35529 | ECBool | Close site have bin data need manual remove device. | kevin 20150202 測試時沒有 ic 出現bin資料或bin別沒設定需取出ic |
| I27 | Manual sort mode | `cbI27` | `bI27_ManualSortMode` | bool | `bI27_ManualSortMode` | 35530 | ECBool | Manual sort mode | Steven 20150915 : For TSMC 手動整盤功能 |
| I28 | On off sites on the fly | `cbI28` | `bI28_OnOffSiteOnTheFly` | bool | `bI28_OnOffSiteOnTheFly` | 35531 | ECBool | On off sites on the fly | Steven 20150924 : 隨時開關Site功能 |
| I29 | Enable yield record                         Sec | `cbI29` | `bI29EnableYieldRecord` | bool | `bEnableYieldRecord` | 35532 | ECBool | Enable yield record                         Sec | Ifor 20151221 :新增 Yield Record Function |
| I29 | Enable yield record                         Sec | `edI29` | `fI29YieldRecordInterval` | double | `fYieldRecordInterval` | 35532 | ECBool | Enable yield record                         Sec | Ifor 20151221 :新增 Yield Record Interval Time |
| I29-1 | Save yield data by socket by bin | `chkI29_1` | `bI29_1SaveYieldBySocketByBin` | bool | `bI29_1SaveYieldBySocketByBin` | 35534 | ECBool | Save yield data by socket by bin | Steven 20171108 (wei) : By Socket By Bin存檔 |
| I29-3 |  | `cbI29_3` | `bI29YieldRecordIntervalIC` | bool | `bI29YieldRecordIntervalIC` |  | ECBool | Enable yield record                         ea (By Total Yield) | Sam 20231106 : 紀錄 Total yield |
| I29-3 |  | `edI29_3` | `iI29YieldRecordIntervalIC` | int | `iI29YieldRecordIntervalIC` |  | ECBool | Enable yield record                         ea (By Total Yield) | Sam 20231106 : 紀錄 Total yield |
| I30 | Reset bin fail count number over. | `cbI30` | `bI30ContFailBin` | bool | `I30_bContFailBin` | 35535 | ECBool | Reset bin fail count number over. | kevin 20160407 Fail bin 超過數量發警告訊息 |
| I31-1 | GPIB Lot End Command | `cbI31_1` | `bI31_1GPIBLotEnd` | bool | `I31_GPIBLotEnd` | 35536 | ECBool | GPIB Lot End Command | wei 20160726 TSMC GPIB Lot End |
| I31-2 | GPIB Lot Start Command | `cbI31_2` | `bI31_2GPIBLotStart` | bool | `bI31_2GPIBLotStart` | 35537 | ECBool | GPIB Lot Start Command | wei 20170607 GPIB Lot Start Command |
| I31-3 | GPIB Reset Command | `cbI31_3` | `bI31_3GPIBReset` | bool | `bI31_3GPIBReset` | 35538 | ECBool | GPIB Reset Command | wei 20170918 Reset Command |
| I32 |  | `cbI32` | `bI32CanCelErrorBin` | bool | `bI32CanCelErrorBin` | 35539 | ECBool | Disable error bin setting. Error devices should take out manually. | kevin 20160802 取消error bin 要手到在 outshuttle 取出 |
| I33 |  | `cbI33` | `bI33ErrorBinBox` | bool | `bI33ErrorBinBox` | 35540 | ECBool | Enable error bin box setting. Error devices put to bin box. | kevin 20160819 error bin 要放到 Bin Box |
| I34 |  | `cbI34` | `bI34AllSiteAreSameFailBinShowAlarm` | bool | `bI34AllSiteAreSameFailBinShowAlarm` | 35541 | ECBool | In the test relust all site are specific fail bin, show alarm. | JerryYang 20160913 矽品要求當測試結果中所有site的bin都是所設定一樣的fail bin要跳alarm |
| I35 | Use Third Test Site (Engineer Access) | `cbI35` | `bI35UseThirdSiteControlByEngineer` | bool | `bI35UseThirdSiteControlByEngineer` | 35542 | ECBool | Use Third Test Site (Engineer Access) | Alick 20160926 add for 第三組工程師用開關SITE |
| I36 | Tester timer out, manual take out on arm device. | `cbI36` | `bI36TestTimeOut` | bool | `bI36TestTimeOut` | 35543 | ECBool | Tester timer out, manual take out on arm device. | kevin 20161105 沒有收到測試資料需手動取出ic |
| I37 | FIFO Mode | — | `iI37_LockLoaderDirection` | int | — | -- | — | FIFO Mode | Steven 20170302 (wei) : FIFO MODE |
| I37-1 | Enable  FIFO mode. | `cbI37_1` | `bI37_EnableFIFOMode` | bool | `bI37_EnableFIFOMode` | 35544 | ECBool | Enable  FIFO mode. | Steven 20170302 (wei) : FIFO MODE |
| I37-2 | Enable site order link. | `cbI37_2` | `bI37_EnableFIFOSiteOrder` | bool | `bI37_EnableFIFOSiteOrder` | 35545 | ECBool | Enable site order link. | Steven 20170302 (wei) : FIFO MODE |
| I37-3 | Lock loader sort direction | `cbI37_3` | `bI37_LockLoaderDirection` | bool | `bI37_LockLoaderDirection` | 35546 | ECBool | Lock loader sort direction | Steven 20170302 (wei) : FIFO MODE |
| I38 | Format of SETTEMP? | `rgI38` | `iI38SETTEMPRespondSetTemp` | bool | `bI38SETTEMPRespondSetTemp` | 35548 | ECBool | SETTEMP? Respond Settemp +25.0.. Data. | kevin 20180308 Settemp? 回傳需要 Settemp +25.0. |
| I39 | Enabled Spirox Lot End and Full lot end command. | `cbI39` | `bI39SpiroxTesterLotEnd` | bool | `bI39SpiroxTesterLotEnd` | 35549 | ECBool | Enabled Spirox Lot End and Full lot end command. | JerryYang 20170515 (wei) JCET通知tester lot end command |
| I40 | Operator mode ->ON line | `cbI40` | `bI40_bStartProductOnLine` | bool | `bI40_bStartProductOnLine` | 35550 | ECBool | Operator mode ->ON line | kevin 20180517 生產前OP OFF_LINE 強制 On line |
| I41 | Enable Empty Socket Check | `cbI41` | `bI41EnableEmptySocketCheck` | bool | `bI41EnableEmptySocketCheck` | -- | -- | Empty Socket Check Function |  |
| I41 | Enable Empty Socket Check | `cbbI41` | `iI41_BinOfESC` | int | `iI44_BinOfESC` | -- | -- | Empty Socket Check Function | Steven 20220817 : Bin of ESC function //Steven 20230308 : I44 --> I41 |
| I41-1 |  | `cbI41_1` | `bI41_1_StartOfLot` | bool | `bI41_1_StartOfLot` |  |  |  |  |
| I41-2 |  | `cbI41_2` | `bI41_2_OpenChamberDoor` | bool | `bI41_2_OpenChamberDoor` |  |  |  |  |
| I41-3 |  | `cbI41_3` | `bI41_3_AfterContactorTeminated` | bool | `bI41_3_AfterContactorTeminated` |  |  |  |  |
| I41-4 |  | `cbI41_4` | `bI41_4_AfterContactorJam` | bool | `bI41_4_AfterContactorJam` |  |  |  |  |
| I41-5 |  | `cbI41_5` | `bI41_5_RegularExecutionCycle` | bool | `bI41_5_RegularExecutionCycle` |  |  |  |  |
| I41-5 |  | `edtI41_5` | `iI41_5_RegularExecutionCycleCount` | int | `iI41_5_RegularExecutionCycleCount` |  |  |  |  |
| I41-6 |  | `cbI41_6` | `bI41_6_Manual` | bool | `bI41_6_Manual` |  |  |  |  |
| I42 | Enable Barcode Flow Error Check | `cbI42` | `bI42_bEnableBarcodeFlowErr` | bool | `bI42_bEnableBarcodeFlowErr` |  | ECBool | Enable Barcode Flow Error Check | Ifor 20211116 add: Use Barcode Flow Err Check |
| I43 | Reset GPIB after tray feed finish. | `cbI43` | `bI43ResetGPIBAfterTrayFeedFinish` | bool | `bI06ResetGPIBAfterTrayFeedFinish` |  | ECBool | Reset GPIB after tray feed finish. | Sam 20211107 : 新增 Tray Feed Finish Reset GPIB |
| I44 |  | `chkI44` | `bI44_LowYieldAlarmIntervalTimeBySetting` | bool | `bI44_LowYieldAlarmIntervalTimeBySetting` |  | ECBool | Low Yield Alarm Interval Time By Setting(1~300)                           sec | JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間 |
| I44 |  | `edI44` | `iI44_LowYieldAlarmIntervalTime` | int | `iI44_LowYieldAlarmIntervalTime` |  | ECBool | Low Yield Alarm Interval Time By Setting(1~300)                           sec | JimmyChiu 20220601 : 修改low yield報警時間邏輯，報警間隔時間固定1min改為可以自行設定報警間隔時間 |
| I45 | Use 2DID Sorting | `cbI45` | `bI45_Use2DIDSort` | bool | `bI45_Use2DIDSort` |  | ECBool | Use 2DID Sorting | KenHsieh 20230607 : ASEKH add 2D SORT |
| I46 | Action when GPIB flow error (WAR07327 and WAR07328) | `rgI46` | `iI46_ActionWhenGpibFlowErr` | int | `iI46_ActionWhenGpibFlowErr` |  | ECInteger | Action when GPIB flow error (WAR07327 and WAR07328) | Steven 20231017 : GPIB flow error need alarm |
| I49 |  | `cbI49` | `bI49_TesterTimeOutResetAllIC` | bool | `bI49_TesterTimeOutResetAllIC` |  |  | Enable All unit to error bin for tester clean unit. If drop contact auto change direct contact auto above  socket(mm) | Sam 20240215 : Tester time out show reset all ic |
| I49 |  | `edI49` | `fI49_ChangeAboveSocket` | double | `fI49_ChangeAboveSocket` |  |  | Enable All unit to error bin for tester clean unit. If drop contact auto change direct contact auto above  socket(mm) | Sam 20240215 : Tester time out show reset all ic |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_EnableASM_Trigger` | `bI50_EnableAutoSiteMappingTrigger` | bool | `bI50_EnableAutoSiteMappingTrigger` | -- | -- | Auto Site Mapping Trigger |  |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_InitialStart` | `bI50_InitialStart` | bool | `bI50_InitialStart` | -- | -- | Auto Site Mapping Trigger |  |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_OnyCycle` | `bI50_OnyCycle` | bool | `bI50_OnyCycle` | -- | -- | Auto Site Mapping Trigger |  |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_Pause` | `bI50_Pause` | bool | `bI50_Pause` | -- | -- | Auto Site Mapping Trigger |  |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_RT` | `bI50_RT` | bool | `bI50_RT` | -- | -- | Auto Site Mapping Trigger | Steven 20231031 : RT不做ASM |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_StartLot` | `bI50_StartLot` | bool | `bI50_StartLot` | -- | -- | Auto Site Mapping Trigger |  |
| I50 | Enable Auto Site Mapping Trigger | `cbI50_TrayFeed` | `bI50_TrayFeed` | bool | `bI50_TrayFeed` | -- | -- | Auto Site Mapping Trigger |  |
| I51 | -[I60] | `cbI51` | `bI51_bNotSetErrBinForInput` | bool | `bI51_bNotSetErrBinForInput` |  | ECBool | Reset not set error bin for device on input shuttle and input arm. |  |
| I52 | Use AQL Sort Mode | — | `bI52_bAQLSortMode` | bool | — |  | — | Use AQL Sort Mode |  |
| I53 |  | — | `bI53_bKLTInitial` | bool | — |  | — | When stops for more than                       Sec, execute Initial Start Delay |  |
| I53 |  | — | `fI53KLTInitialInterval` | double | — |  | — | When stops for more than                       Sec, execute Initial Start Delay | Ifor 20230210 add:新增 KLT 要求Onecycle Finish未超過設定時間不執行KL initial delay |
| I54 | Enable | `cbI54_Enable` | `bI54_Enable` | bool | `bI54_Enable` | -- | -- | Check the temperature during index arm testing | Jimmychiu 20240916 : Check the temperature during index arm testing |
| I54-1 | All ICs on the arm set error bin when temperature error | `cbI54_1` | `bI54_1_AllICErr` | bool | `bI54_1_AllICErr` |  | ECBool | All ICs on the arm set error bin when temperature error | Jimmychiu 20240916 : Check the temperature during index arm testing |
| I54-2 | Only abnormal IC set error bin when temperature error | `cbI54_2` | `bI54_2_AbnormalICErr` | bool | `bI54_2_AbnormalICErr` |  | ECBool | Only abnormal IC set error bin when temperature error | Jimmychiu 20240916 : Check the temperature during index arm testing |

---


## 未綁定 UI 元件的欄位

_無。所有 `I##` 開頭的欄位均已在主表中列出。_
