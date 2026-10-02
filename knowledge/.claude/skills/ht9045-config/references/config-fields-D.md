# HT9045_CONFIG 欄位速查：群組 [D] Index（Index / 下壓設定）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Dxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 131 個欄位，分屬 95 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| D01 | Enable read torque  (1/per) | `cbD01` | `bD01EnableReadTorque` | bool | `bD01EnableReadTorque` | 35100 | ECBool | Enable read torque  (1/per) |  |
| D01 | Enable read torque  (1/per) | `edD01_Xilinx` | `dD01ReadTorque` | double | `dD01ReadTorque` | 35100 | ECBool | Enable read torque  (1/per) |  |
| D01 | Enable read torque  (1/per) | `edD01DelayTime_Xilinx` | `dD01ReadTorqueDelayTime` | double | `dD01ReadTorqueDelayTime` | 35100 | ECBool | Enable read torque  (1/per) |  |
| D01 | Enable read torque  (1/per) | — | `iD01ReadTorqueDelayTime` | int | — | 35100 | — | Enable read torque  (1/per) |  |
| D01 | Enable read torque  (1/per) | `edD01` | `iD01ReadTorqueTimeCount` | int | `iD01ReadTorqueTimeCount` | 35100 | ECBool | Enable read torque  (1/per) |  |
| D01-1 | Enable read and check torque | `cbD01_1` | `bD01_1EnableReadAndCheckTorque` | bool | `bD01_1EnableReadAndCheckTorque` |  | ECBool | Enable read and check torque |  |
| D02 | Off read torque function during test | `cbD02` | `bD02OffReadTorqueDuringTest` | bool | `bD02OffReadTorqueDuringTest` | 35101 | ECBool | Off read torque function during test |  |
| D04 | Min force is set by file. | `cbD04` | `bD04MinForceByFile` | bool | `bD04MinForceByFile` |  | ECBool | Min force is set by file. | Steven 20190314 : Min force is read from file |
| D04 | Min force is set by file. | `edtD04` | `dD04MinForceByFile` | double | `dD04MinForceByFile` |  | ECBool | Min force is set by file. |  |
| D04-20 |  | `edtD04_20mm` | `dD04MinForceByFile_20mm` | double | `dD04MinForceByFile_20mm` |  |  |  | Steven 20220822 : Min force for different SLK |
| D04-30 |  | `edtD04_30mm` | `dD04MinForceByFile_30mm` | double | `dD04MinForceByFile_30mm` |  |  |  |  |
| D04-40 |  | `edtD04_40mm` | `dD04MinForceByFile_40mm` | double | `dD04MinForceByFile_40mm` |  |  |  |  |
| D04-60 |  | `edtD04_60mm` | `dD04MinForceByFile_60mm` | double | `dD04MinForceByFile_60mm` |  |  |  |  |
| D04-80 |  | `edtD04_80mm` | `dD04MinForceByFile_80mm` | double | `dD04MinForceByFile_80mm` |  |  |  | Ifor 20240620 : add缸徑 80 |
| D05 | Contact Count Alarm | `cbD05` | `bD05ContactCountAlarm` | bool | `bD05ContactCountAlarm` |  | ECBool | Contact Count Alarm | wei 20170327 add |
| D05 | Contact Count Alarm | `edD05` | `iD05_ContactCountAlarm` | int | `iD05_ContactCountAlarm` |  | ECBool | Contact Count Alarm | wei 20170327 |
| D05-1 | Save socket count by machine | `cbD05_1` | `bD05_1SaveSocketCntByHandler` | bool | `bD05_1SaveSocketCntByHandler` |  |  | Save socket count by machine | Steven 20250807 : By handler save contact count |
| D06 | Contact Offset Default Value | `cbD06` | `bD06ContactOffsetDefaultValue` | bool | `bD06ContactOffsetDefaultValue` |  | ECBool | Contact Offset Default Value | JimmyChiu 20200120 add |
| D06 | Contact Offset Default Value | `edD06` | `dD06_ContactOffsetDefaultValue` | double | `dD06_ContactOffsetDefaultValue` |  | ECBool | Contact Offset Default Value | JimmyChiu 20200120 add |
| D10 | Manual height use Z1, Z2 button to UP/Down | `cbD10` | `bD10ManualHeightComptibleWithNS` | bool | `bD10ManualHeightComptibleWithNS` | 35104 | ECBool | Manual height use Z1, Z2 button to UP/Down |  |
| D11 | If shuttle no device, no need to do auto height. | `cbD11` | `bD11NoIcSkipAutoHeight` | bool | `bNoIcSkipAutoHeight` | 35105 | ECBool | If shuttle no device, no need to do auto height. | Steven 20110726 : Shuttle沒IC時,該Arm不要Auto Height |
| D12 | Shuttle auto height by setting force value | `cbD12` | `bD12UseDeviceFormPressDoShtHeight` | bool | `bUseDeviceFormPressDoShtHeight` | 35106 | ECBool | Shuttle auto height by setting force value | Steven 20140220 : 用生產ep去做蝦頭auto high |
| D13 | Check index home sensor after find home. | `cdD13` | `bD13CheckIndexHomeSensor` | bool | `bCheckIndexHomeSensor` | 35148 | ECBool | Check index home sensor after find home. | Steven 20140828 : 歸零後檢查Index位置 |
| D14 | Auto height use setting torque (For > 300KG Model) | `cbD14` | `bD14_AutoHeightUseSetTorque` | bool | `bD14_AutoHeightUseSetTorque` | 35149 | ECBool | Auto height use setting torque (For > 300KG Model) | Steven 20141105 : 使用指定的扭力進行Auto Height |
| D14 | Auto height use setting torque (For > 300KG Model) | `edD14` | `iD14_AutoHeightUseSetTorque` | int | `iD14_AutoHeightUseSetTorque` | 35149 | ECBool | Auto height use setting torque (For > 300KG Model) |  |
| D15 | Continuous auto contact test | `cbD15` | `bD15_AutoContactTest` | bool | `bD15_AutoContactTest` | 35151 | ECBool | Continuous auto contact test | Steven 20150224 : Auto Contact Test |
| D16 | Step by step contact test | `cbD16` | `bD16_StepContactTest` | bool | `bD16_StepContactTest` | 35152 | ECBool | Step by step contact test | Steven 20150811 : Step by Step Contact Test |
| D17 | Auto Height Method | `rgD17` | `iD17_UseHardwareHeightToContact` | int | `bD17_UseHardwareHeightToContact` | -- | -- | Auto Height Method | Steven 20170411 (wei) : SCK的SIP怕刮傷,所以Contact Height使用硬體高度 |
| D17-3 |  | `edtD17_3` | `dD17_3CheckEPLeakage` | double | `dD17_3CheckEPLeakage` |  |  |  | Steven 20231025 : 充氣跟不充氣都做一次auto height, 然後檢查有沒有漏氣 |
| D18 | Notice to check contact height when change recipe. | `chkD18` | `bD18_AutoHeightWhenChangeRecipe` | bool | `bD18_AutoHeightWhenChangeRecipe` |  | ECBool | Notice to check contact height when change recipe. | Steven 20221013 : ATK希望更換工作檔時, 需要提示有沒有做Auto Height. |
| D21 | Enable finish test up && wait | `cbD21` | `bD21EnableFinishTestUpWait` | bool | `bD21EnableFinishTestUpWait` | 35107 | ECBool | Enable finish test up && wait |  |
| D21 | Enable finish test up && wait | `edD21_mm` | `iD21FinishTestUpWaitHeight` | int | `iD21FinishTestUpWaitHeight` | 35107 | ECBool | Enable finish test up && wait |  |
| D21 | Enable finish test up && wait | `edD21_Sec` | `iD21FinishTestUpWaitTime` | int | `iD21FinishTestUpWaitTime` | 35107 | ECBool | Enable finish test up && wait |  |
| D22 | Double Contact Function | — | `iD22DoubleContactCount` | int | — | -- | — | Double Contact Function |  |
| D22-1 | Support multi double contact | `cbD22_1` | `bD22SupportMultiDoubleContact` | bool | `bD22SupportMultiDoubleContact` | 35110 | ECBool | Support multi double contact |  |
| D22-2 | Double contact no need re-contact | `cbD22_2` | `bD22DoubleContactNoNeedReContact` | bool | `bDoubleContactNoNeedReContact` | 35119 | ECBool | Double contact no need re-contact | Steven 20131202 : Double Contact不需要Index Arm上下動 |
| D22-3 | Double contact use diff. SRQ | `cbD22_3_` | `bD22DoubleContactUseDiffSRQ` | bool | `bD22DoubleContactUseDiffSRQ` |  | ECBool | Double contact use different SRQ | Steven 20230508 : 南茂鐘永生說要使用0x41  //JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號 |
| D22-3 | Double contact use diff. SRQ | `cbD22_3` | `bD22VerifyMode` | bool | `bD22VerifyMode` |  | ECBool | Double contact use different SRQ | Sam 20221012 : 新增 VerifyMode 功能 |
| D22-4 | Double contact can set pass bin | `cbD22_4` | `bD22_4_PassBinCanDoubleContact` | bool | `bD22_4_PassBinCanDoubleContact` |  | ECBool | Double contact can set pass bin | JerryYang 20230914 : add |
| D23 | Every device do multi contact before test. | `cbD23` | `bD23EveryDeviceDoubleContactFirstNoTesting` | bool | `bEveryDeviceDoubleContactFirstNoTestting` | 35154 | ECBool | Every device do multi contact before test. | ChungHung 20140709 add for SPIL |
| D23 | Every device do multi contact before test. | — | `iD23_DoubleContactSRQ` | int | — | 35154 | — | Every device do multi contact before test. | JerryYang 20221004 : Double contact改成可以選擇不同的測試訊號 |
| D23 | Every device do multi contact before test. | `edD23` | `iD23_MultiContactCount` | int | `iMultiContactCount` | 35154 | ECBool | Every device do multi contact before test. | Steven 20151001 : Add for TSMC |
| D24 | Enable EP check function | `cbD24` | `bD24EnableEPCheckFuntion` | bool | `EnableEPCheckFuntion` | 35112 | ECBool | Enable EP check function |  |
| D26 | Enable EP encoder range +,- | `cbD26` | `bD26EnableEPEncoderRange` | bool | `bEnableEPEncoderRange` | 35116 | ECBool | Enable EP encoder range +,- | ChungHung 20111217 |
| D26 | Enable EP encoder range +,- | `edD26` | `iD26EPEncoderRange` | int | `iEPEncoderRange` | 35116 | ECBool | Enable EP encoder range +,- | ChungHung 20111217 |
| D26-1 | Enable EP log | `cbD26_1` | `bD26EnableEPLog` | bool | `bEnableEPLog` | 35156 | ECBool | Enable EP log | Ifor 20150706 |
| D26-2 | Show EP encoder | `cbD26_2` | `bD26EnableEncodeShow` | bool | `bEnableEncodeShow` | 35157 | ECBool | Show EP encoder | Ifor 20150706 |
| D26-3 | Enable Dual EP encoder range +,-                   Kpa | `cbD26_3` | `bD26_3EnableDualEPEncoderRange` | bool | `bD26_3EnableDualEPEncoderRange` | 35117 | ECInteger | EP Encoder Range | Ifor 20221228 add: Dual Force EP Check |
| D26-3 | Enable Dual EP encoder range +,-                   Kpa | `edD26_3` | `iD26_3DualEPEncoderRange` | int | `iD26_3DualEPEncoderRange` | 35117 | ECInteger | EP Encoder Range | Ifor 20221228 add: Dual Force EP Check |
| D26-3 | Enable Dual EP encoder range +,-                   Kpa | — | `iD26_3FixValueOrPercentage` | int | — | 35117 | — | EP Encoder Range | Richard 20230428 : EP固定值或百分比 |
| D27 | Enable single site 85 kg | `cbD27` | `bD27UseSingleSite85kg` | bool | `UseSingleSite85kg` | 35118 | ECBool | Enable single site 85 kg | Dell 2012-01-03 在1X2模式下,關Site能達85kg |
| D28 | Maximum contact force limitation by diameter. | `cbD28` | `bD28MaxForceLimitByDiameter` | bool | `bD28MaxForceLimitByDiameter` |  | ECBool | Maximum contact force limitation by diameter. | Steven 20200813 : 用缸徑計算最大壓力 |
| D29 | Stable contact mode | `cbD29` | `bD29EnableIndexContactDelay` | bool | `bEnableIndexContactDelay` | 35120 | ECBool | Stable contact mode | Steven 20140519 : [D29] Index下壓後多Delay 0.4秒 (For SPIL Low Yield) |
| D30 | Enable site mode select | `cbD30` | `bD30EnableSiteModeSelect` | bool | `bD30EnableSiteModeSelect` | 35151 | ECBool | Enable site mode select |  |
| D31 | RTC change recipe need re-create RTC model | `cbD31` | `bD31RTCChangeRecipeNeedreCreateModel` | bool | `bRTCChangeRecipeNeedreCreateModel` | 35122 | ECBool | RTC change recipe need re-create RTC model | jou 2012-03-01 [D31] RTC Change Recipe Need reCreate RTC Model |
| D32 | Tray pitch > 35mm, the counter air on time must >0.5 Sec. | `cbD32` | `bD32_35TrayPitchIndexDOnMoreThen500MS` | bool | `b35TrayPitchIndexDOnMoreThen500MSec` | 35123 | ECBool | Tray pitch > 35mm, the counter air on time must >0.5 Sec. | Steven 20120727 : Tray Pitch 大於35mm的話, Index吹氣至少要0.5秒 |
| D33 | RTC initial start need verify | `cbD33` | `bD33RTCInitStartVerify` | bool | `bRTCInitStartVerify` | 35124 | ECBool | RTC initial start need verify | Handler Use Model Verify |
| D34 | Enable galil protection function. | `cbD34` | `bD34GailDMCProtection` | bool | `bGailDMCProtection` | 35125 | ECBool | Enable galil protection function. | ChungHung 20131230 add for ATK |
| D35 | RTC need check site number | `cbD35` | `bD35RTCCheckSiteMap` | bool | `bRTCCheckSiteMap` |  | ECBool | RTC need check site number | Steven 20140513 : [D35] |
| D36 | RTC auto model verify | `cbD36` | `bD36EnableRTCAutoModelVerify` | bool | `bEnableRTCAutoModelVerify` |  | ECBool | RTC auto model verify | jou 2014-06-24 RTC 自動進行Model驗證 |
| D36 | RTC auto model verify | — | `iD36_RTCAutoVerifyPickHeight` | int | — |  | — | RTC auto model verify | jou 2014-06-24 RTC 自動進行Model驗證 |
| D36 | RTC auto model verify | — | `iD36_RTCAutoVerifyReleaseHeight` | int | — |  | — | RTC auto model verify | jou 2014-06-24 RTC 自動進行Model驗證 |
| D36-1 | RTC auto model verify auto live show check | `cbD36_1` | `bD36_1EnableRTCAutoModelVerifyLive` | bool | `bD36_1EnableRTCAutoModelVerifyLive` |  | ECBool | RTC auto model verify auto live show check | jou 2014-06-24 RTC 自動進行Model驗證 |
| D36-2 | Trigger of RTC auto verification after  one cycle | `cbD36_2` | `bD36_2AfterOneCycleNeedAutoVerify` | bool | `bD36_2AfterOneCycleNeedAutoVerify` |  | ECBool | Trigger of RTC auto verification after  one cycle | JerryYang 20201116 : One cycle後要執行RTC auto verify |
| D37 | Manual process | `cbD37` | `bD37EnableManualProcess` | bool | `bEnableManualProcess` |  | ECBool | Manual process | ChungHung 20150526 add for QualComm US |
| D38 | Index release device to shuttle no wait motion. | `cbD38` | `bD38IndexPutICToShtNoWaitMotion` | bool | `bD38IndexPutICToShtNoWaitMotion` | 35158 | ECBool | Index release device to shuttle no wait motion. | Steven 20181228 : Add Index Action   //ChungHung 20171116 modify for Index Action |
| D40 | Index IC lose, need press Z1 to active skip button. | `cbD40` | `bD40IndexICFallDownMustPressFMotorDown` | bool | `bD40IndexICFallDownMustPressFMotorDown` | 35126 | ECBool | Index IC lose, need press Z1 to active skip button. |  |
| D41 | Index check IC position for socket | `cbD41` | `bD41CheckbySetup` | bool | `bD41CheckbySetup` | 35128 | ECInteger | Index check IC position for socket | kevin 20200801 by setup 功能只能用一次 |
| D41 | Index check IC position for socket | — | `bD41_Active` | bool | — | 35128 | — | Index check IC position for socket | Steven 20140627 : Add for ASE-CL -- D41 打勾 |
| D41 | Index check IC position for socket | — | `bD41_Enable` | bool | — | 35128 | — | Index check IC position for socket | Steven 20140627 : Add for ASE-CL -- D41 Enable |
| D41 | Index check IC position for socket | `edD41` | `dD41SocketInitialCheckOffset` | double | `fD41SocketInitialICCheckPositionOffset` | 35128 | ECInteger | Index check IC position for socket | Steven 20100818 :[D41]       //Steven 20230308 : [D41] LastSet 改 IniConfig |
| D41 | Index check IC position for socket | — | `dD41_Offset` | double | — | 35128 | — | Index check IC position for socket | Steven 20140627 : Add for ASE-CL -- D41 高度 |
| D41 | Index check IC position for socket | — | `iD41SocketInitialICCheckPosition` | int | — | 35128 | — | Index check IC position for socket |  |
| D41 | Index check IC position for socket | — | `iD41_Position` | int | — | 35128 | — | Index check IC position for socket | Steven 20140627 : Add for ASE-CL -- Inside/Above |
| D41-2 |  | `edD41_2` | `fIndexCheckOffset` | double | `fIndexCheckOffset` |  |  |  | ChungHung 20140807 add for ATK TestZ_Test + fIndexCheckOffset |
| D42 | Index && shuttle jam need pause | `cbD42` | `bD42IndexPickICShuttlePause` | bool | `bD42IndexPickICShuttlePause` | 35130 | ECBool | Index && shuttle jam need pause |  |
| D42 | Index && shuttle jam need pause | — | `bD42_Active` | bool | — | 35130 | — | Index && shuttle jam need pause | JerryYang 20160220 add for Amkor-Philippine -- D42打勾 |
| D42 | Index && shuttle jam need pause | — | `bD42_Enable` | bool | — | 35130 | — | Index && shuttle jam need pause | JerryYang 20160220 add for Amkor-Philippine -- D42 Enable |
| D43 | Index && shuttle jam can retry or skip | `cbD43` | `bD43IndexDropErrorCanRetryandSkip` | bool | `bIndexDropErrorCanRetryandSkip` | 35131 | ECBool | Index && shuttle jam can retry or skip | ChungHung 20120717 add Index Drop Error Can Retry and Start |
| D43-1 | Enable auto retry when index pick up error | `cbD43_1` | `bD43AutoRetryWhenIndexPickErr` | bool | `bD43AutoRetryWhenIndexPickErr` | 35159 | ECBool | Enable auto retry when index pick up error | Steven 20170105 : Index吸取異常,要退出來用Shuttle Sensor檢查後, 再進去吸一次 |
| D43-2 | Check vacuum in socket when index pick up error | `cbD43_2` | `bD43IndexPickErrCheckSocket` | bool | `bD43IndexPickErrCheckSocket` | 35160 | ECBool | Check vacuum in socket when index pick up error |  |
| D44 | Check vacuum after test head purge, (sec) | `cbD44` | `bD44CheckIndexICDestroy` | bool | `bD44CheckIndexICDestroy` | 35132 | ECBool | Check vacuum after test head purge, (sec) |  |
| D44 | Check vacuum after test head purge, (sec) | — | `bD44_Active` | bool | — | 35132 | — | Check vacuum after test head purge, (sec) | JerryYang 20160220 add for Amkor-Philippine -- D44打勾 |
| D44 | Check vacuum after test head purge, (sec) | — | `bD44_Enable` | bool | — | 35132 | — | Check vacuum after test head purge, (sec) | JerryYang 20160220 add for Amkor-Philippine -- D44 Enable |
| D44 | Check vacuum after test head purge, (sec) | `edD44_Height` | `iD44TestHeadCheckVacuumHeight` | int | `TestHeadCheckVacuumHeight` | 35132 | ECBool | Check vacuum after test head purge, (sec) | wei 20150609 延遲破壞高度 |
| D44 | Check vacuum after test head purge, (sec) | `edD44` | `iD44TestHeadCheckVacuumTime` | int | `iD44TestHeadCheckVacuumTime` | 35132 | ECBool | Check vacuum after test head purge, (sec) |  |
| D45 | Out arm Z should wait until Index Z goes up from shuttle | `cbD45` | `bD45UseOutArmCheckIndex` | bool | `bD45UseOutArmCheckIndex` | 35134 | ECBool | Out arm Z should wait until Index Z goes up from shuttle |  |
| D46 | Index destroy delay | `edD46` | `iD46WaitIndexDestroyTime` | int | `iD46WaitIndexDestroyTime` | 35135 | ECInteger | Index destroy delay |  |
| D47 | Socket clean function | `edD47_Count` | `iD47SocketPurgeCount` | int | `iD47SocketPurgeCount` | -- | -- | Socket clean function |  |
| D47 | Socket clean function | `edD47_Time` | `iD47SocketPurgeTime` | int | `iD47SocketPurgeTime` | -- | -- | Socket clean function |  |
| D47 | Socket clean function | `cbbD47` | `iD47SocketPurgeType` | int | `iD47SocketPurgeType` | -- | -- | Socket clean function | Steven 20190703 : Socket Purge include shuttle |
| D47-1 | Enable socket clean function | `cbD47` | `bD47EnableSocketPurgeFunction` | bool | `bD47EnableSocketPurgeFunction` |  | ECBool | Enable socket clean function |  |
| D47-5 | Contact offset (mm) : | `edtD47_5` | `dD47_5_ContactOffset` | double | `dD47_5_ContactOffset` |  | ECDouble | Contact offset (mm) : | Steven 20190703 : Socket Purge include shuttle |
| D47-6 | Shuttle offset (mm) : | `edtD47_6` | `dD47_6_ShuttleOffset` | double | `dD47_6_ShuttleOffset` |  | ECDouble | Shuttle offset (mm) : | Steven 20190703 : Socket Purge include shuttle |
| D48 | Disable Z1, Z2 function when power off or EMG press | `cbD48` | `bD48PowerOffEmgCanNotUseZ1Z2` | bool | `bD48DisablePowerOffEmgStopZ1Z2KeyButton` | 35139 | ECBool | Disable Z1, Z2 function when power off or EMG press |  |
| D49 | After RTC alarm, set index devices to error bin. | `cbD49` | `bD49RTCAlarmSetIndexToErrBin` | bool | `bD49RTCAlarmSetIndexToErrBin` |  | ECBool | After RTC alarm, set index devices to error bin. | JerryYang 20160712 for 力成,發生RTC Alarm時把Index上所有IC設為Errorbin |
| D50 | Enable Index Pick Error Skip Need Check Vaccum | `cbD50` | `bD50IndexPickErrSkipNeedCheckVac` | bool | `bD50IndexPickErrSkipNeedCheckVac` |  | ECBool | Enable Index Pick Error Skip Need Check Vaccum | jou 20171031 (Steven) : 增加開關,JSCC要求index pick up error 需再慢速下降吸一次 |
| D51 | After one cycle and clean out, test arm go rear position | `cbD51` | `bD51UseOnecycleCleanOutFinishTestArmAtRear` | bool | `bD51UseOnecycleCleanOutFinishTestArmAtRear` | 35140 | ECBool | After one cycle and clean out, test arm go rear position |  |
| D52 | Test head goed up then show interface error | `cbD52` | `bD52InterFaceErrHeadNeedUp` | bool | `bD52InterFaceErrHeadNeedUp` | 35141 | ECBool | Test head goed up then show interface error |  |
| D53 | Index light always on | `cbD53` | `bD53CCDLightOn` | bool | `bD53CCDLightOn` | 35142 | ECBool | Index light always on |  |
| D53 | Index light always on | `edD53` | `iD53LightOnMin` | int | `iD53LightOnMin` | 35142 | ECBool | Index light always on |  |
| D54 | Index pick && place shuttle need slow down | `cbD54` | `bD54SlowDown` | bool | `bSlowDown` | 35144 | ECBool | Index pick && place shuttle need slow down | ChungHung 20110816 add |
| D54 | Index pick && place shuttle need slow down | `edD54` | `iD54SlowDownScale` | int | `iSlowDownScale` | 35144 | ECBool | Index pick && place shuttle need slow down | ChungHung 20110816 add |
| D55 | When RTC enabled, disable index check. | `cbD55` | `bD55DisableIndexCheck` | bool | `bDisableIndexCheck` | 35146 | ECBool | When RTC enabled, disable index check. | ChungHung 20120606 Disable IndexCheck |
| D56 | Forced enable Piggy-Back function | `cbD56` | `bD56YieldPiggyBackEnable` | bool | `bYieldPiggyBackEnable` | 35147 | ECBool | Forced enable Piggy-Back function | kevin 20131009 強致 Enable Yield 裡面piggyback 功能 |
| D57 | Close site need display chanel number | `cbD57` | `bD57SiteMapCloseDisplay` | bool | `bSiteMapCloseDisplay` | 35162 | ECBool | Close site need display chanel number | kevin 20141203 關site顯示 site號碼 |
| D58 | Arm 1 for pick and place,  Arm 2 for testing | `cbD58` | `bD58UseArm1PickPlaceArm2Test` | bool | `bUseArm1PickPlaceArm2Test` | 35163 | ECBool | Arm 1 for pick and place,  Arm 2 for testing | kevin 20150127 讓setup 出現功能  Arm1 下壓 arm2 測試 |
| D59 | 32Site, Pnp devices together | `cbD59` | `bD59_32SitePnpTogether` | bool | `bD59_32SitePnpTogether` |  | ECBool | 32Site, Pnp devices together | Steven 20150910 : 32Site 雙Arm一起吸放 |
| D61 |  | `cbD61` | `bD61IndexArmVacOffErrNeedPiggyBack` | bool | `bD61IndexArmVacOffErrNeedPiggyBack` | 35164 | ECBool | encountered  Vacuum sensor OFF error, must do piggyback check. | JerryYang 20160815 Index arm 發生Vaccum off error要做piggy back |
| D62 | Pick up shuttle error, need to purge one time. | `cbD62` | `bD62PickUpErrorNeedPurge` | bool | `bD62PickUpErrorNeedPurge` | 35165 | ECBool | Pick up shuttle error, need to purge one time. | Steveb 20161024 : 吸取異常需要吹氣一次 |
| D62 | Pick up shuttle error, need to purge one time. | `edD62` | `iD62IndexBlowAirTime` | int | `iD62IndexBlowAirTime` | 35165 | ECBool | Pick up shuttle error, need to purge one time. | Frank 20171213 (Steven) : Index Pick Err In Shuttle Skip and Blow Air |
| D63 | Check Index Z Home To Z Phase Distance Over Range | `cbD63` | `bD63CheckIndexZHomeToZPhaseDistanceRange` | bool | `bD63CheckIndexZHomeToZPhaseDistanceRange` |  | ECBool | Check Index Z Home To Z Phase Distance Over Range | kevin 20170515 (wei) Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range |
| D63 | Check Index Z Home To Z Phase Distance Over Range | `edD63` | `iD63IndexZHomeToZPhaseRange` | int | `iD63IndexZHomeToZPhaseRange` |  | ECBool | Check Index Z Home To Z Phase Distance Over Range | kevin 20170515 (wei) Jeffrey 20170414 Check IndexZ Home to Z Phase Distance Range |
| D63-1 | Find Motor Phase Every Go-Home Process | `cbD63_1` | `bD63_1FindMotorPhaseEveryGoHomeProcess` | bool | `bD63_1FindMotorPhaseEveryGoHomeProcess` |  | ECBool | Find Motor Phase Every Go-Home Process | Isaac 20201110 : Index Y find motor phase |
| D64 | Pick up shuttle error, only SKIP | `cbD64` | `bD64IndexPickErrOnlySKIP` | bool | `bD64IndexPickErrOnlySKIP` | 35166 | ECBool | Pick up shuttle error, only SKIP | kevin 20171103 (wei) index pick up error only skip |
| D65 | Enable check socket sensor function | `cbD65` | `bD65EnableCheckSocketsensorFunction` | bool | `bD65EnableCheckSocketsensorFunction` | 35167 | ECBool | Enable check socket sensor function | Ifor 20171123 add |
| D66 | Initial start auto height | `cbD66` | `bD66Initialstartautoheight` | bool | `bD66Initialstartautoheight` |  | ECBool | Initial start auto height |  |
| D67 | Load Cell Measure | `cbD67` | `bD67LoadCellMeasure` | bool | `bD67LoadCellMeasure` |  | ECBool | Load Cell Measure | kevin 20190907 Arm 測區次數道量測 功能; |
| D68 |  | `edD68` | `dD68DistanceRange` | double | `dD68DistanceRange` |  | ECDouble | After complete of the Auto high compared to before, if the height more than need to Jam mm | Ifor 20190925 : add |
| D69 | Index check mode for auto clean | `rgD69` | `iD69IndexCheckModeForAutoClean` | int | `iD69IndexCheckModeForAutoClean` |  | ECInteger | Index check mode for auto clean | Steven 20191212 : 劉仁洲說Auto Clean只要作一次Index Check |
| D70 | Index Cycle Time Record | `cbD70` | `bD70IndexCycleTimeRecord` | bool | `bD70IndexCycleTimeRecord` |  | ECBool | Index Cycle Time Record | Sam 20200916 : Add Index Cycle Time Record |
| D71 | Do Index check mode | `rgD71` | `iD71IndexCheckOnOffMode` | int | `iD71IndexCheckOnOffMode` |  | ECInteger | Do Index check mode | Isaac 20211019 : 可選擇做index check的時機 |
| D72 | Shuttle 1 move after index contact for NN mode. | `cbD72` | `bD72NNModeMoveShtAfterContact` | bool | `bD72NNModeMoveShtAfterContact` |  | ECBool | Shuttle 1 move after index contact for NN mode. | Steven 20220531 : index下壓之後才能移動shuttle |
| D73 | Contact Mode fast | `cbD73` | `bD73ContactModeFast` | bool | `bD73ContactModeFast` |  | ECBool | Contact Mode fast | kevin 20220817 : Conttact mode 加速 |
| D74 | RTC Auto Tuning | `cbD74` | `bD74RTCAutoTuning` | bool | `bD74RTCAutoTuning` |  | ECBool | RTC Auto Tuning | Sam 20230419 : 新增 RTC Auto Tuning 功能 |
| D75 | One cycle finished, Alaway need to be learning RTC golden. | `cbD75` | `bD75OneCycleFinishedAlawayLearnRTCGolden` | bool | `bD75OneCycleFinishedAlawayLearnRTCGolden` |  | ECBool | One cycle finished, Alaway need to be learning RTC golden. | Sam 20240117 : OneCycle 完成做完 Full view check 後都需要做 RTC Learning golden |
| D78 | Index Check Has IC Need Purge | `cbD78` | `bD78EnableIndexCheckHasICNeedPurge` | bool | `bD78EnableIndexCheckHasICNeedPurge` |  | ECBool | Index Check Has IC Need Purge | Ifor 20200622 add:Index Check Has IC Need Purge |
| D79 | Index Pick Shuttle Err Need Purge | `cbD79` | `bD79EnableIndexPickShuttleErrNeedPurge` | bool | `bD79EnableIndexPickShuttleErrNeedPurge` |  | ECBool | Index Pick Shuttle Err Need Purge | Ifor 20200622 add:Index Pick Shuttle Err Need Purge |
| D80 | After Out Shuttle to Right Site Index Check | `cbD80` | `bD80AfterOutSHToRightSiteIndexCheck` | bool | `bD80EnableIndexPickShuttleErrNeedPurge` |  | ECBool | After Out Shuttle to Right Site Index Check | Ifor 20240415 add科園廠功能 |
| D81 | Index Check Vacuum on shuttle after input arm device drop. | `cbD81` | `bD81IndexCheckVacuumOnShuttle` | bool | `bD81IndexCheckVacuumOnShuttle` | -- | -- | Auto High check setting torque and record Log | JerryYang 20241010 : add |
| D82 | Check Index Arm has IC when indexing the product. | `cbD82` | `bD82CheckIndexHasIC` | bool | `bD82CheckIndexHasIC` |  | ECBool | If Check,RTC Disable Avtive Check,Otherwise Enable | Jimmychiu 20250826 : 每次下壓確認有IC在socket |

---


## 未綁定 UI 元件的欄位

_無。所有 `D##` 開頭的欄位均已在主表中列出。_
